/*
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

#include "irc_network.h"
#include "ui/buffer.h"
#include "net_io.h"
#include "net_input_handler.h"

#include <gtk/gtk.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <fcntl.h>

#ifdef WITH_SSL
#include <gnutls/gnutls.h>
#include <gnutls/x509.h>

static int _verify_certificate_cb(gnutls_session_t session);
#endif

/* Creates a new network network and adds it to the network tree
 * NOTE: The struct is automatically sanitized by this function, so there is no
 * need to do it yourself
 */
struct irc_network * new_irc_network() {
    // Allocate and sanitize the structure
    struct irc_network * network = malloc(sizeof(struct irc_network));
    memset(network, 0, sizeof(struct irc_network));

    //XXX: Added just for testing purposes
    network->username = strdup("SquirrelChat");
    network->real_name = strdup("SquirrelChat");

    network->buffer = new_buffer(NULL, NETWORK, network);

    network->buffers = trie_new(trie_strtolower);

    return network;
}

/* TODO: Switch the tab when freeing a network to ensure all references to the
 * text buffer are removed
 */
void free_irc_network(struct irc_network * network,
                      GtkTreeStore * network_tree_store) {
    destroy_buffer(network->buffer);
    free(network->name);
    free(network->nickname);

    free(network);
}

int connect_irc_network(struct irc_network * network) {
#ifndef WITH_SSL
    if (network->ssl) {
        print_to_buffer(network->buffer,
                        "SSL support was not included in this version of "
                        "Squirrelchat, therefore SSL cannot be enabled.\n");
        return 0;
    }
#endif

    struct addrinfo hints; // For specifying the type of host we want
    struct addrinfo * results; // Stores the result list from getaddrinfo
    int func_result;

    // Make sure the server is set
    if (network->address == NULL) {
        print_to_buffer(network->buffer, "You forgot to set a server!\n");
        return -1;
    }

    // Set a default nickname when the user does not set one
    if (network->nickname == NULL)
        network->nickname = strdup(getlogin());

    print_to_buffer(network->buffer, "Attempting to connect to %s:%s...\n",
                    network->address, network->port);

    // Try to get the addrinfo for the server
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    func_result = getaddrinfo(network->address, network->port, &hints, &results);
    if (func_result != 0) {
        print_to_buffer(network->buffer, "CONNECTION ERROR: %s\n",
                        gai_strerror(func_result));
        return -1;
    }

    // Try to connect to each address structure returned by getaddrinfo
    /* FIXME: Instead of returning function, retry connection attempt and print
     * messages
     */
    struct addrinfo * rp;
    for (rp = results; rp != NULL; rp = rp->ai_next) {
        // Try to create a socket
        network->socket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (network->socket == -1)
            continue;

        // Set the socket as non-blocking
        fcntl(network->socket, F_SETFD, O_NONBLOCK);

        // Try to connect to the created socket
        if (connect(network->socket, rp->ai_addr, rp->ai_addrlen) != -1)
            break;

        // The connection failed, so close the socket
        close(network->socket);
    }

    // Check if we successfully connected, if not return an error
    if (rp == NULL) {
        print_to_buffer(network->buffer, "CONNECTION ERROR: %s\n",
                        strerror(errno));
        freeaddrinfo(results);
        return -1;
    }

#ifdef WITH_SSL
    // If the connection is meant to be SSL, we have to start the handshake
    if (network->ssl) {
        int ret;
        const char * err;

        gnutls_init(&network->ssl_session, GNUTLS_CLIENT);
        gnutls_certificate_allocate_credentials(&network->ssl_cred);

        gnutls_session_set_ptr(network->ssl_session, network);
        gnutls_server_name_set(network->ssl_session, GNUTLS_NAME_DNS,
                               network->address, strlen(network->address));

        // Setup the trusted CAS file
        gnutls_certificate_set_x509_trust_file(network->ssl_cred,
                                               CAFILE_PATH,
                                               GNUTLS_X509_FMT_PEM);
        gnutls_certificate_set_verify_function(network->ssl_cred,
                                               _verify_certificate_cb);

        gnutls_credentials_set(network->ssl_session, GNUTLS_CRD_CERTIFICATE,
                               network->ssl_cred);

        // TODO: Add stuff for client certificate here

        // we may need to set priorities here, not sure yet

        ret = gnutls_priority_set_direct(network->ssl_session, "NORMAL", &err);
        if (ret < 0) {
            print_to_buffer(network->buffer,
                            "GnuTLS error: %s\n"
                            "Connection aborted.\n", err);
            freeaddrinfo(results);
            close(network->socket);
            gnutls_deinit(network->ssl_session);
            gnutls_certificate_free_credentials(network->ssl_cred);
            return -1;
        }

        gnutls_transport_set_ptr(network->ssl_session,
                                 (gnutls_transport_ptr_t)network->socket);
        print_to_buffer(network->buffer,
                        "Performing SSL handshake...\n");
        if ((ret = gnutls_handshake(network->ssl_session)) == GNUTLS_E_SUCCESS) {
            print_to_buffer(network->buffer,
                            "Handshake complete!\n");
            begin_registration(network);
        }
        else if (ret == GNUTLS_E_INTERRUPTED || ret == GNUTLS_E_AGAIN)
            network->status = HANDSHAKE;
        else {
            print_to_buffer(network->buffer, "GnuTLS error: %s\n"
                                             "Connection aborted.\n",
                            gnutls_strerror(ret));
            freeaddrinfo(results);
            gnutls_deinit(network->ssl_session);
            gnutls_certificate_free_credentials(network->ssl_cred);
            close(network->socket);
            return -1;
        }
    }
    else {
        print_to_buffer(network->buffer, "Connection established.\n");
        begin_registration(network);
    }
#else
    print_to_buffer(network->buffer, "Connection established.\n");
    begin_registration(network);
#endif

    network->input_channel = g_io_channel_unix_new(network->socket);
    g_io_channel_set_encoding(network->input_channel, NULL, NULL);
    g_io_channel_set_buffered(network->input_channel, FALSE);

    g_io_add_watch_full(network->input_channel, G_PRIORITY_DEFAULT, G_IO_IN,
                        (GIOFunc)net_input_handler, network, NULL);

    freeaddrinfo(results);

    return 0;
}

void begin_registration(struct irc_network * network) {
    print_to_buffer(network->buffer,
                    "Sending our registration information.\n");
    send_to_network(network, "NICK %s\r\n"
                             "USER %s * * %s\r\n",
                    network->nickname, network->username, network->real_name);
    if (network->password != NULL)
        send_to_network(network, "PASS :%s\r\n", network->password);

    print_to_buffer(network->buffer,
                    "Attempting to negotiate capabilities with server (CAP)...\n");
    send_to_network(network, "CAP LS\r\n");
    network->status = CONNECTED;
}

void disconnect_irc_network(struct irc_network * network,
                            const char * msg) {
    send_to_network(network, "QUIT :%s\r\n", msg ? msg : "");
    network->status = DISCONNECTED;

    free(network->version);
    free(network->server_name);
    free(network->chanmodes);
    free(network->usermodes);
    free(network->chantypes);
    free(network->chanmodes_a);
    free(network->chanmodes_b);
    free(network->chanmodes_c);
    free(network->chanmodes_d);
    free(network->prefix_chars);
    free(network->prefix_symbols);

    network->version = NULL;
    network->server_name = NULL;
    network->chanmodes = NULL;
    network->usermodes = NULL;
    network->chantypes = NULL;
    network->chanmodes_a = NULL;
    network->chanmodes_b = NULL;
    network->chanmodes_c = NULL;
    network->chanmodes_d = NULL;
    network->prefix_chars = NULL;
    network->prefix_symbols = NULL;
    network->casecmp = NULL;

#ifdef WITH_SSL
    if (network->ssl) {
        gnutls_certificate_free_credentials(network->ssl_cred);
        gnutls_bye(network->ssl_session, GNUTLS_SHUT_WR);

        network->ssl_cred = NULL;

        /* We leave the deinitialization of the ssl session to the net input
         * handler
         */
    }
#endif
}

#ifdef WITH_SSL
static int _verify_certificate_cb(gnutls_session_t session) {
    unsigned int status;
    int ret;
    time_t expiration_time;
    struct timespec current_time;
    struct irc_network * network = gnutls_session_get_ptr(session);
    unsigned int chain_size;
    const gnutls_datum_t * chain;
    gnutls_x509_crt_t * cert;

    // Retreive the peer's certificate chain and import it
    chain = gnutls_certificate_get_peers(session, &chain_size);
    cert = calloc(sizeof(*chain), chain_size);
    for (int i = 0; i < chain_size; i++) {
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
    clock_gettime(CLOCK_REALTIME, &current_time);
    if (expiration_time <= current_time.tv_sec)
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

        print_to_buffer(network->buffer, "--- Certificate info ---\n");

        // Print the name of the peer's certificate
        gnutls_x509_crt_get_dn(cert[0], dn_subject_buf, &dn_subject_buf_size);
        print_to_buffer(network->buffer, "Subject:\n");
        for (char * c = strtok_r(dn_subject_buf, ",", &dn_strptr);
             c != NULL;
             c = strtok_r(NULL, ",", &dn_strptr))
            print_to_buffer(network->buffer, "\t%s\n", c);

        // Print the issuer information
        gnutls_x509_crt_get_issuer_dn(cert[0], dn_issuer_buf,
                                      &dn_issuer_buf_size);
        print_to_buffer(network->buffer, "Issuer:\n");
        for (char * c = strtok_r(dn_issuer_buf, ",", &dn_strptr);
             c != NULL;
             c = strtok_r(NULL, ",", &dn_strptr))
            print_to_buffer(network->buffer, "\t%s\n", c);

        // Get the rest of the information
        expiration_time = gnutls_x509_crt_get_expiration_time(cert[0]);
        print_to_buffer(network->buffer,
                        "Subject's certificate expires on %s",
                        ctime(&expiration_time));
    }

    if (status != 0) {
        bool fatal = false;
        if (status & GNUTLS_CERT_SIGNER_NOT_FOUND ||
            status & GNUTLS_CERT_SIGNER_NOT_CA) {
            gnutls_x509_crt_t subject, issuer;
            // TODO: Check if the certificate is self-signed
            print_to_buffer(network->buffer,
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
                print_to_buffer(network->buffer, " Ignoring by request of user.\n");
            else {
                fatal = true;
                print_to_buffer(network->buffer, "\n");
            }
            gtk_widget_destroy(dialog);
        }
        if (status & GNUTLS_CERT_NOT_ACTIVATED) {
            if (!fatal) {
                print_to_buffer(network->buffer,
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
                    print_to_buffer(network->buffer,
                                    " Ignoring by request of user.\n");
                else {
                    fatal = true;
                    print_to_buffer(network->buffer, "\n");
                }
                gtk_widget_destroy(dialog);
            }
            else
                print_to_buffer(network->buffer,
                                "The certificate has not been activated "
                                "yet.\n");
        }
        else if (status & GNUTLS_CERT_EXPIRED) {
            /* If we've already determined the connection should not continue,
             * don't bother asking
             */
            if (!fatal) {
                print_to_buffer(network->buffer,
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
                    print_to_buffer(network->buffer,
                                    " Ignoring by request of user.\n");
                else {
                    fatal = true;
                    print_to_buffer(network->buffer, "\n");
                }
                gtk_widget_destroy(dialog);
            }
            else
                print_to_buffer(network->buffer,
                                "The certificate is expired.\n");
        }

        if (fatal) {
            for (int i = 0; i < chain_size; i++)
                gnutls_x509_crt_deinit(cert[i]);
            disconnect_irc_network(network, "SSL error");
            return GNUTLS_E_CERTIFICATE_ERROR;
        }
    }

    for (int i = 0; i < chain_size; i++)
        gnutls_x509_crt_deinit(cert[i]);
    return 0;
}
#endif // WITH_SSL

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
