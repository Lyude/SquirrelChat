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

/* Creates a new network network and adds it to the network tree
 * NOTE: The struct is automatically sanitized by this function, so there is no
 * need to do it yourself
 */
struct irc_network * new_irc_network() {
    // Allocate and sanitize the structure
    struct irc_network * network = malloc(sizeof(struct irc_network));
    memset(network, 0, sizeof(struct irc_network));

    // Set the network as disconnected
    network->connection_status = DISCONNECTED;

    //XXX: Added just for testing purposes
    network->address = "rainbowdash.ponychat.net";
    network->port = "6667";
    network->nickname = "LyudeTEST";
    network->username = "LyudeTEST";
    network->real_name = "LyudeTEST";

    network->buffer = new_buffer(NETWORK, network);

    return network;
}

/* TODO: Switch the tab when freeing a network to ensure all references to the
 * text buffer are removed
 */
void free_irc_network(struct irc_network * network,
                      GtkTreeStore * network_tree_store) {
    destroy_buffer(network->buffer);
    // TODO: Add in code to free all of the strings in the structure
    free(network);
}

// Connects an IRC network
int connect_irc_network(struct irc_network * network) {
    struct addrinfo hints; // For specifying the type of host we want
    struct addrinfo * results; // Stores the result list from getaddrinfo
    int func_result;

    network->connection_status = CONNECTING;
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
        network->connection_status = DISCONNECTED;
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
        network->connection_status = DISCONNECTED;
        freeaddrinfo(results);
        return -1;
    }

    print_to_buffer(network->buffer, "Connected! Sending info...\n");
    send_to_network(network, "NICK %s\r\n", network->nickname);
    send_to_network(network, "USER %s X X :%s\r\n",
                    network->username, network->real_name);

    network->connection_status = CONNECTED;
    network->input_channel = g_io_channel_unix_new(network->socket);
    g_io_channel_set_encoding(network->input_channel, NULL, NULL);
    g_io_channel_set_buffered(network->input_channel, FALSE);

    g_io_add_watch_full(network->input_channel, G_PRIORITY_DEFAULT, G_IO_IN,
                        (GIOFunc)net_input_handler, network, NULL);

    freeaddrinfo(results);
    return 0;
}

void free_chat_buffer(void * this_does_nothing) {
    // TODO: Add stuff
}

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
