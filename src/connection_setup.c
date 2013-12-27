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

#include "connection_setup.h"
#include "irc_network.h"
#include "ui/buffer.h"
#include "net_input_handler.h"
#include "net_io.h"

#ifdef WITH_SSL
#include "ssl.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <glib/gi18n.h>

static void connection_setup_thread(struct sqchat_network * network);
static gboolean connection_final_setup_phase(struct sqchat_network * network);
static void try_next_server_in_list(struct sqchat_network * network);

// Starts a thread to resolve the hostname given and connects to the given host
void sqchat_begin_connection_attempt(struct sqchat_network * network) {
    sqchat_server * server = network->current_server->data;

    network->status = ADDR_RES;
    sqchat_buffer_print(network->buffer,
                        _("Looking up \"%s\"...\n"), server->address);
    network->addr_res_thread =
        g_thread_new("Address Resolution",
                     (GThreadFunc)&connection_setup_thread, network);
}

static void connection_setup_thread(struct sqchat_network * network) {
    struct addrinfo hints;
    struct addrinfo * results;
    struct addrinfo * rp;
    int func_result;
    sqchat_server * server = network->current_server->data;
    
    // Try to get the addrinfo for the server
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    func_result = getaddrinfo(server->address, server->port, &hints,
                              &results);
    if (func_result != 0) {
        sqchat_buffer_print(network->buffer,
                            _("Failed to look up \"%s\": %s\n"),
                            server->address, gai_strerror(func_result));
        try_next_server_in_list(network);
        return;
    }

    sqchat_buffer_print(network->buffer,
                        _("Lookup successful, attempting to connect to "
                          "host...\n"));

    /* Attempt to connect to the results given by getaddrinfo(), this might
     * later be moved into the main application loop, but for the time being
     * it's simpler just to run it from this thread
     */
    for (rp = results; rp != NULL; rp = rp->ai_next) {
        network->socket = socket(rp->ai_family, rp->ai_socktype,
                                 rp->ai_protocol);
        
        if (network->socket == -1)
            continue;

        if (connect(network->socket, rp->ai_addr, rp->ai_addrlen) != -1)
            break; // Success

        close(network->socket);
    }

    freeaddrinfo(results);
    
    if (rp == NULL) {
        sqchat_buffer_print(network->buffer, _("Connection failed!\n"));
        try_next_server_in_list(network);
        return;
    }

    sqchat_buffer_print(network->buffer, _("Connection successful!\n"));

    /* We've completed the basic connection portion, now for simplicity sake we
     * pass the rest of the work for setting up the connection back to the main
     * thread
     */
    g_idle_add((GSourceFunc) connection_final_setup_phase, network);
}

void try_next_server_in_list(struct sqchat_network * network) {
    // Make sure there is another server in the list that we can try
    if (g_slist_next(network->current_server) == NULL) {
        sqchat_buffer_print(network->buffer,
                            _("No more servers left to try, giving up.\n"));
        network->current_server = NULL;
        network->status = DISCONNECTED;
        return;
    }

    sqchat_buffer_print(network->buffer,
                        _("Trying the next server in the list...\n"));
    network->current_server = g_slist_next(network->current_server);
    sqchat_begin_connection_attempt(network);
}

static gboolean connection_final_setup_phase(struct sqchat_network * network) {
    sqchat_server * server = network->current_server->data;

#ifdef WITH_SSL
    if (server->ssl)
        sqchat_begin_ssl_handshake(network);
    else
#endif
        sqchat_begin_registration(network);

    network->input_channel = g_io_channel_unix_new(network->socket);
    g_io_channel_set_encoding(network->input_channel, NULL, NULL);
    g_io_channel_set_buffered(network->input_channel, FALSE);

    g_io_add_watch_full(network->input_channel, G_PRIORITY_DEFAULT, G_IO_IN,
                        (GIOFunc)sqchat_net_input_handler, network, NULL);
    return false;
}

void sqchat_begin_registration(struct sqchat_network * network) {
    sqchat_buffer_print(network->buffer,
                        "Sending our registration information.\n");
    sqchat_network_send(network,
                        "NICK %s\r\n"
                        "USER %s * * %s\r\n",
                        network->nickname, network->username,
                        network->real_name);
    if (network->password != NULL)
        sqchat_network_send(network, "PASS :%s\r\n", network->password);

    sqchat_buffer_print(network->buffer,
                        "Attempting to negotiate capabilities with server "
                        "(CAP)...\n");
    sqchat_network_send(network, "CAP LS\r\n");
    network->status = CONNECTED;
}

// vim: set expandtab tw=80 shiftwidth=4 softtabstop=4 cinoptions=(0,W4:
