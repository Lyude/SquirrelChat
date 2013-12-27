/* Contains functions for setting up an SSL connection
 *
 * Copyright (C) 2013 Stephen Chandler Paul
 *
 * This file is free software: you may copy it, redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 2 of this License or (at your option) any
 * later version.
 *
 * This file is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ssl.h"
#include "irc_network.h"
#include "connection_setup.h"
#include "ui/buffer.h"

#include <stdlib.h>
#include <gnutls/gnutls.h>
#include <gnutls/x509.h>

static int verify_certificate_cb(gnutls_session_t session);

void sqchat_begin_ssl_handshake(struct sqchat_network * network) {
    int ret;
    const char * err;

    int gtls = gnutls_init(&network->ssl_session, GNUTLS_CLIENT);
    gnutls_certificate_allocate_credentials(&network->ssl_cred);

    gnutls_session_set_ptr(network->ssl_session, network);
    gnutls_server_name_set(network->ssl_session, GNUTLS_NAME_DNS,
                           network->address, strlen(network->address));

    // Setup the trusted CAS file
    gnutls_certificate_set_x509_trust_file(network->ssl_cred,
                                           CAFILE_PATH,
                                           GNUTLS_X509_FMT_PEM);
    gnutls_certificate_set_verify_function(network->ssl_cred,
                                           verify_certificate_cb);

    gnutls_credentials_set(network->ssl_session, GNUTLS_CRD_CERTIFICATE,
                           network->ssl_cred);

    // TODO: Add stuff for client certificate here

    ret = gnutls_priority_set_direct(network->ssl_session, "NORMAL", &err);
    if (ret < 0) {
        sqchat_buffer_print(network->buffer,
                            "GnuTLS error: %s\n"
                            "Connection aborted.\n", err);
        goto ssl_handshake_error;
    }

    gnutls_transport_set_ptr(network->ssl_session,
                             (gnutls_transport_ptr_t)network->socket);
    sqchat_buffer_print(network->buffer,
                        "Performing SSL handshake...\n");
    if ((ret = gnutls_handshake(network->ssl_session)) == GNUTLS_E_SUCCESS) {
        sqchat_buffer_print(network->buffer,
                            "Handshake complete!\n");
        sqchat_begin_registration(network);
    }
    else if (ret == GNUTLS_E_INTERRUPTED || ret == GNUTLS_E_AGAIN)
        network->status = HANDSHAKE;
    else {
        sqchat_buffer_print(network->buffer,
                            "GnuTLS error: %s\n"
                            "Connection aborted.\n",
                            gnutls_strerror(ret));
        goto ssl_handshake_error;
    }
    return;

ssl_handshake_error:
    gnutls_deinit(network->ssl_session);
    gnutls_certificate_free_credentials(network->ssl_cred);
    close(network->socket);
    network->status = DISCONNECTED;
}

static int verify_certificate_cb(gnutls_session_t session) {
    unsigned int status;
    int ret;
    time_t expiration_time;
    struct sqchat_network * network = gnutls_session_get_ptr(session);
    unsigned int chain_size;
    const gnutls_datum_t * chain;
    gnutls_x509_crt_t * cert;

    // Retreive the peer's certificate chain and import it
    chain = gnutls_certificate_get_peers(session, &chain_size);
    cert = calloc(sizeof(*chain), chain_size);
    for (unsigned int i = 0; i < chain_size; i++) {
        gnutls_x509_crt_init(&cert[i]);
        gnutls_x509_crt_import(cert[i], &chain[i], GNUTLS_X509_FMT_DER);
    }

    // Make sure the certificate checks out
#if GNUTLS_VERSION_NUMBER >= 0x030104
    ret = gnutls_certificate_verify_peers3(session, network->address, &status);
    if (ret < 0)
        goto verification_error;
#else
    ret = gnutls_certificate_verify_peers2(session, &status);
    if (ret < 0)
        goto verification_error;

    // Pre GnuTLS version 3.1.4, hostname checking was done seperately
    // Check the subject
    ret = gnutls_x509_crt_check_hostname(cert[0], network->address);
    if (ret < 0)
        goto verification_error;
    // TODO: Check to see if we have to check the rest of the chain
#endif

    // Make sure the certificate is not expired
    expiration_time = gnutls_x509_crt_get_expiration_time(cert[0]);
    if (expiration_time <= (time_t)g_get_real_time() * 1.0e-6)
        status |= GNUTLS_CERT_EXPIRED;

verification_error:

    // Retrieve and display the certificate information
    {
        char * dn_subject_buf;
        size_t dn_subject_buf_size;
        char * dn_issuer_buf;
        size_t dn_issuer_buf_size;
        char * dn_strptr;
        time_t expiration_time;

        /* Figure out how large the buffer for holding DN information needs to
         * be
         */
        gnutls_x509_crt_get_dn(cert[0], NULL, &dn_subject_buf_size);
        dn_subject_buf = alloca(dn_subject_buf_size);
        gnutls_x509_crt_get_issuer_dn(cert[0], NULL, &dn_issuer_buf_size);
        dn_issuer_buf = alloca(dn_issuer_buf_size);

        sqchat_buffer_print(network->buffer, "--- Certificate info ---\n");

        // Print the name of the peer's certificate
        gnutls_x509_crt_get_dn(cert[0], dn_subject_buf, &dn_subject_buf_size);
        sqchat_buffer_print(network->buffer, "Subject:\n");
        for (char * c = strtok_r(dn_subject_buf, ",", &dn_strptr);
             c != NULL;
             c = strtok_r(NULL, ",", &dn_strptr))
            sqchat_buffer_print(network->buffer, "\t%s\n", c);

        // Print the issuer information
        gnutls_x509_crt_get_issuer_dn(cert[0], dn_issuer_buf,
                                      &dn_issuer_buf_size);
        sqchat_buffer_print(network->buffer, "Issuer:\n");
        for (char * c = strtok_r(dn_issuer_buf, ",", &dn_strptr);
             c != NULL;
             c = strtok_r(NULL, ",", &dn_strptr))
            sqchat_buffer_print(network->buffer, "\t%s\n", c);

        // Get the rest of the information
        expiration_time = gnutls_x509_crt_get_expiration_time(cert[0]);
        sqchat_buffer_print(network->buffer,
                            "Subject's certificate expires on %s",
                            ctime(&expiration_time));
    }

    if (status != 0) {
        bool fatal = false;
        if (status & GNUTLS_CERT_SIGNER_NOT_FOUND ||
            status & GNUTLS_CERT_SIGNER_NOT_CA) {
            gnutls_x509_crt_t subject, issuer;
            // TODO: Check if the certificate is self-signed
            sqchat_buffer_print(network->buffer,
                                "A trusted issuer was not found in the "
                                "certificate.");
            GtkWidget * dialog;
            dialog = gtk_message_dialog_new(GTK_WINDOW(network->window->window),
                                            GTK_DIALOG_DESTROY_WITH_PARENT,
                                            GTK_MESSAGE_WARNING,
                                            GTK_BUTTONS_YES_NO,
                "The certificate provided by the server was not issued by a "
                "trusted certificate authority. This means there is no way to "
                "verify the server's certificate is legitimate, and could "
                "mean the connection is insecure, or that someone is trying "
                "to impersonate the server and steal sensitive information "
                "from you. Occasionally, this can also occur because the "
                "certificate authority list on your computer is out of date, "
                "in which case it is recommended you update your computer.\n"
                "Are you sure you wish to continue connecting?");
            if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_YES)
                sqchat_buffer_print(network->buffer, " Ignoring by request of user.\n");
            else {
                fatal = true;
                sqchat_buffer_print(network->buffer, "\n");
            }
            gtk_widget_destroy(dialog);
        }
        if (status & GNUTLS_CERT_NOT_ACTIVATED) {
            if (!fatal) {
                sqchat_buffer_print(network->buffer,
                                    "The certificate has not been activated yet.");
                GtkWidget * dialog;
                dialog = gtk_message_dialog_new(GTK_WINDOW(network->window->window),
                                                GTK_DIALOG_DESTROY_WITH_PARENT,
                                                GTK_MESSAGE_WARNING,
                                                GTK_BUTTONS_YES_NO,
                    "The server provided a certificate that has not been "
                    "activated yet, meaning that the certificate will not be "
                    "valid until a later point in time. This could be someone "
                    "trying to impersonate the server with an invalid "
                    "certificate in an attempt to steal sensitive information "
                    "from you.\n"
                    "Are you sure you wish to continue connecting?");
                if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_YES)
                    sqchat_buffer_print(network->buffer,
                                        " Ignoring by request of user.\n");
                else {
                    fatal = true;
                    sqchat_buffer_print(network->buffer, "\n");
                }
                gtk_widget_destroy(dialog);
            }
            else
                sqchat_buffer_print(network->buffer,
                                    "The certificate has not been activated "
                                    "yet.\n");
        }
        else if (status & GNUTLS_CERT_EXPIRED) {
            /* If we've already determined the connection should not continue,
             * don't bother asking
             */
            if (!fatal) {
                sqchat_buffer_print(network->buffer,
                                    "The certificate is expired.");
                // Ask the user if they want to continue connecting or not
                GtkWidget * dialog;
                dialog = gtk_message_dialog_new(GTK_WINDOW(network->window->window),
                                                GTK_DIALOG_DESTROY_WITH_PARENT,
                                                GTK_MESSAGE_WARNING,
                                                GTK_BUTTONS_YES_NO,
                    "The server has provided a certificate that is expired.\n"
                    "This could be someone trying to impersonate the server "
                    "in an attempt to steal sensitive information from you.\n"
                    "Are you sure you wish to continue connecting?");
                if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_YES)
                    sqchat_buffer_print(network->buffer,
                                        " Ignoring by request of user.\n");
                else {
                    fatal = true;
                    sqchat_buffer_print(network->buffer, "\n");
                }
                gtk_widget_destroy(dialog);
            }
            else
                sqchat_buffer_print(network->buffer,
                                    "The certificate is expired.\n");
        }

        if (fatal) {
            for (unsigned int i = 0; i < chain_size; i++)
                gnutls_x509_crt_deinit(cert[i]);
            sqchat_network_disconnect(network, "SSL error");
            return GNUTLS_E_CERTIFICATE_ERROR;
        }
    }

    for (unsigned int i = 0; i < chain_size; i++)
        gnutls_x509_crt_deinit(cert[i]);
    return 0;
}

// vim: set expandtab tw=80 shiftwidth=4 softtabstop=4 cinoptions=(0,W4:
