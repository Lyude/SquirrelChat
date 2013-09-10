/* Code for the callback function used whenever there is data waiting on a
 * connected network buffer's socket. It parses the message, dispatches the
 * appropriate callbacks, etc.
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

#include "ui/buffer.h"
#include "net_input_handler.h"
#include "irc_network.h"
#include "net_io.h"
#include "message_parser.h"
#include "connection_setup.h"

#include <glib.h>
#include <string.h>
#include <errno.h>

#include <gnutls/gnutls.h>

/* Checks for messages waiting in a network's buffer
 * If there is still a message waiting in the network's buffer, it returns it
 * and moves the buffer cursor forward. Otherwise returns null. NULL is returned
 * and errno is set in the event of an error.
 */
char * check_for_messages(struct irc_network * network) {
    char * next_terminator = strstr(&network->recv_buffer[network->buffer_cursor],
                                    "\r\n");
    char * output;

    // If a message was found, remove the terminator, set output variable
    if (next_terminator != NULL) {
        *next_terminator = '\0';
        *(next_terminator + 1) = '\0';
        output = &network->recv_buffer[network->buffer_cursor];
        network->buffer_cursor = (int)(next_terminator -
                                      &network->recv_buffer[0]) + 2;

        // Check if there's potentially another message in the buffer
        if (network->buffer_cursor + 1 >= network->buffer_fill_len) {
            network->buffer_cursor = 0;
            network->buffer_fill_len = 0;
        }
    }
    else {
        /* If no message was found but we're not at the end of the buffer, that
         * means there's probably a only partially received message waiting
         * after the position of the cursor. Make sure there is space for the
         * rest of the message to be received
         */
        if (network->buffer_cursor < network->buffer_fill_len) {
            network->buffer_fill_len -= network->buffer_cursor;
            memmove(&network->recv_buffer[0],
                    &network->recv_buffer[network->buffer_cursor],
                    network->buffer_fill_len);
        }

        // If the message is longer then the max allowed length, report an error
        else if (network->buffer_cursor == 0 &&
                 network->buffer_fill_len >= IRC_MSG_LEN)
            errno = EMSGSIZE;

        network->buffer_cursor = 0;
        return NULL;
    }
    return output;
}

gboolean net_input_handler(GIOChannel *source,
                           GIOCondition condition,
                           struct irc_network * network) {
    char * msg;
    int result;

    errno = 0;
#ifdef WITH_SSL
    if (network->ssl) {
        if (network->status == DISCONNECTED) {
            print_to_buffer(network->buffer->window->current_buffer,
                            "* Disconnected\n");
            return false;
        }
        else if (network->status == CONNECTED) {
            do {
                // Try reading from the network
                result =
                    gnutls_read(network->ssl_session,
                                &network->recv_buffer[network->buffer_fill_len],
                                IRC_MSG_LEN - network->buffer_fill_len);
                if (result == 0) {
                    print_to_buffer(network->buffer, "Disconnected.\n");
                    gnutls_deinit(network->ssl_session);
                    close(network->socket);
                    return FALSE;
                }
                else if (result == GNUTLS_E_REHANDSHAKE) {
                    // Check if another handshake can be done securely
                    if (gnutls_safe_renegotiation_status(network->ssl_session)) {
                        print_to_buffer(network->buffer,
                                        "Server requested another SSL "
                                        "handshake, please wait...\n");
                        result = gnutls_handshake(network->ssl_session);
                        if (result == GNUTLS_E_SUCCESS)
                            print_to_buffer(network->buffer,
                                            "Handshake complete! Resuming "
                                            "normal operations.\n");
                        else if (result == GNUTLS_E_AGAIN ||
                                 result == GNUTLS_E_INTERRUPTED)
                            network->status = REHANDSHAKE;
                        else if (gnutls_error_is_fatal(result)) {
                            print_to_buffer(network->buffer,
                                            "Fatal SSL error: %s\n"
                                            "Closing connection.\n",
                                            gnutls_strerror(result));
                            disconnect_irc_network(network, "SSL error");
                            close(network->socket);
                            return FALSE;
                        }
                        else
                            print_to_buffer(network->buffer,
                                            "SSL warning: %s\n",
                                            gnutls_strerror(result));
                    }
                    else {
                        /* Something fishy may be happening, reject the
                         * additional handshake
                         */
                        print_to_buffer(network->buffer,
                                        "SSL warning: Server requested "
                                        "another handshake, but our "
                                        "connection does not support safe "
                                        "renegotiation. Denying handshake "
                                        "request.\n");
                        gnutls_alert_send_appropriate(network->ssl_session,
                                                      GNUTLS_A_NO_RENEGOTIATION);
                    }
                }
                else if (result == GNUTLS_E_INTERRUPTED ||
                         result == GNUTLS_E_AGAIN)
                    return TRUE;
                /* If the receive fails for any reason, close the connection
                 * We may eventually want to improve on this behavior
                 */
                else if (result < 0) {
                    print_to_buffer(network->buffer,
                                    "SSL error during record receive: %s\n"
                                    "Disconnected.\n",
                                    gnutls_strerror(result));
                    disconnect_irc_network(network, "SSL error");
                    gnutls_deinit(network->ssl_session);
                    close(network->socket);
                    return FALSE;
                }

                network->buffer_fill_len += result;

                while ((msg = check_for_messages(network)) != NULL)
                    process_irc_message(network, msg);
            } while (gnutls_record_check_pending(network->ssl_session));
        }
        /* If we're not in CONNECTED or CAP mode, we must be (re)initiating a
         * handshake
         */
        else {
            // Try to do a handshake
            result = gnutls_handshake(network->ssl_session);
            if (result == GNUTLS_E_SUCCESS) {
                // If this is our first handshake, continue to the CAP phase
                if (network->status == HANDSHAKE) {
                    print_to_buffer(network->buffer,
                                    "Handshake complete.\n");
                    begin_registration(network);
                }
                else {
                    print_to_buffer(network->buffer,
                                    "Handshake complete. Resuming "
                                    "connection.\n");
                    network->status = CONNECTED;
                }
            }
            else if (result != GNUTLS_E_AGAIN &&
                     result != GNUTLS_E_INTERRUPTED) {
                if (gnutls_error_is_fatal(result)) {
                    print_to_buffer(network->buffer,
                                    "SSL error during handshake: %s\n",
                                    gnutls_strerror(result));
                    disconnect_irc_network(network, "SSL error");
                }
                else
                    print_to_buffer(network->buffer,
                                    "SSL warning: %s\n",
                                    gnutls_strerror(result));
            }
        }
    }
    else {
#endif // WITH_SSL
        result = recv(network->socket,
                           &network->recv_buffer[network->buffer_fill_len],
                           IRC_MSG_LEN - network->buffer_fill_len, 0);

        if (result == 0) {
            print_to_buffer(network->buffer, "Disconnected.\n");
            close(network->socket);
            return FALSE;
        }
        else if (result == -1) {
            print_to_buffer(network->buffer,
                            "Error: %s\n"
                            "Closing connection.\n",
                            strerror(errno));
            close(network->socket);
            return FALSE;
        }

        network->buffer_fill_len += result;
        while ((msg = check_for_messages(network)) != NULL)
            process_irc_message(network, msg);
#ifdef WITH_SSL
    }
#endif

    return TRUE;
}

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
