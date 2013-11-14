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
#include <pthread.h>

static void connection_setup_thread(struct sqchat_network * network);
static gboolean connection_final_setup_phase(struct sqchat_network * network);

// Starts a thread to resolve the hostname given and connects to the given host
void sqchat_begin_connection(struct sqchat_network * network) {
    network->status = ADDR_RES;
    sqchat_buffer_print(network->buffer,
                        "Looking up \"%s\"...\n", network->address);
    pthread_create(&network->addr_res_thread, NULL,
                   (void*(*)(void*))&connection_setup_thread,
                   network);
}

static void connection_setup_thread(struct sqchat_network * network) {
    struct addrinfo hints;
    struct addrinfo * results;
    struct addrinfo * rp;
    int func_result;
    
    // Try to get the addrinfo for the server
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    func_result = getaddrinfo(network->address, network->port, &hints,
                              &results);
    if (func_result != 0) {
        sqchat_buffer_print(network->buffer,
                            "Failed to look up \"%s\": %s\n",
                            network->address, gai_strerror(func_result));
        network->status = DISCONNECTED;
        return;
    }

    sqchat_buffer_print(network->buffer,
                        "Lookup successful, attempting to connect to "
                        "host...\n");

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
        sqchat_buffer_print(network->buffer, "Connection failed!\n");
        network->status = DISCONNECTED;
        return;
    }

    sqchat_buffer_print(network->buffer, "Connection successful!\n");

    /* We've completed the basic connection portion, now for simplicity sake we
     * pass the rest of the work for setting up the connection back to the main
     * thread
     */
    g_idle_add((GSourceFunc) connection_final_setup_phase, network);
}

static gboolean connection_final_setup_phase(struct sqchat_network * network) {
#ifdef WITH_SSL
    if (network->ssl)
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

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4:cinoptions=(0,W4
