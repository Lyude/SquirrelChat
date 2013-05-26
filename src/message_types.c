/* Contains the callbacks for all of the stock non-numeric message types
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
#include "message_parser.h"
#include "irc_network.h"
#include "ui/buffer.h"
#include "net_io.h"
#include "irc_macros.h"

#include <string.h>

void join_msg_callback(struct irc_network * network,
                       char * hostmask,
                       short argc,
                       char * argv[],
                       char * trailing) {
    char * nickname;
    char * address;
    split_irc_hostmask(hostmask, &nickname, &address);

    if (trailing == NULL)
        print_to_buffer(network->buffer,
                        "Error parsing message: Received a JOIN from %s, but "
                        "no channel was specified\n", nickname);
    // Check if we're the user joining a channel
    else if (strcmp(network->nickname, nickname) == 0) {
        GtkTreeModel * network_tree_model;
        GtkTreeIter network_iter;
        GtkTreeIter channel_iter;
        struct buffer_info * new_channel = new_buffer(CHANNEL, network);

        trie_set(network->buffers, trailing, new_channel);
        // Add a new row as a child of the network
        network_tree_model = gtk_tree_row_reference_get_model(network->row);
        gtk_tree_model_get_iter(network_tree_model, &network_iter,
                                gtk_tree_row_reference_get_path(network->row));

        gtk_tree_store_append(GTK_TREE_STORE(network_tree_model),
                               &channel_iter, &network_iter);
        gtk_tree_store_set(GTK_TREE_STORE(network_tree_model), &channel_iter,
                           0, trailing, 1, new_channel, -1);
    }
    else {
        struct buffer_info * buffer;
        if ((buffer = trie_get(network->buffers, trailing)) == NULL) {
            print_to_buffer(network->buffer,
                            "Error parsing message: received JOIN from %s for "
                            "%s, but we're not in that channel!\n",
                            nickname, trailing);
            return;
        }

        print_to_buffer(buffer, "%s (%s) has joined %s\n",
                        nickname, address, trailing);
    }
}

void privmsg_msg_callback(struct irc_network * network,
                          char * hostmask,
                          short argc,
                          char * argv[],
                          char * trailing) {
    char * nickname;
    char * address;
    split_irc_hostmask(hostmask, &nickname, &address);
    struct buffer_info * buffer;
        
    /* Since a channel buffer is always created whenever we join a channel, and
     * we cannot receive messages from channels we're not in, then the only time
     * we won't be able to find a buffer is when a new user messages us, so we
     * can just go ahead and make a new query buffer.
     */
    if ((buffer = trie_get(network->buffers, argv[0])) == NULL) {
        buffer = new_buffer(QUERY, network);
        GtkTreeModel * network_tree_model;
        GtkTreeIter network_iter;
        GtkTreeIter buffer_iter;

        network_tree_model = gtk_tree_row_reference_get_model(network->row);
        gtk_tree_model_get_iter(network_tree_model, &network_iter,
                                gtk_tree_row_reference_get_path(network->row));

        gtk_tree_store_append(GTK_TREE_STORE(network_tree_model),
                              &buffer_iter, &network_iter);
        gtk_tree_store_set(GTK_TREE_STORE(network_tree_model), &buffer_iter,
                           0, nickname, 1, buffer, -1);
    }
    
    print_to_buffer(buffer, "<%s> %s\n", nickname, trailing);
}

void ping_msg_callback(struct irc_network * network,
                       char * hostmask,
                       short argc,
                       char * argv[],
                       char * trailing) {
    send_to_network(network, "PONG %s\r\n", trailing);
}

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
