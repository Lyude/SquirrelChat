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

    char * channel_name;

    if ((channel_name = trailing) == NULL && (channel_name = argv[0]) == NULL)
        print_to_buffer(network->buffer,
                        "Error parsing message: Received a JOIN from %s, but "
                        "no channel was specified\n", nickname);
    // Check if we're the user joining a channel
    else if (strcmp(network->nickname, nickname) == 0) {
        GtkTreeModel * network_tree_model;
        GtkTreeIter network_iter;
        GtkTreeIter channel_iter;
        struct buffer_info * new_channel = new_buffer(channel_name,
                                                      CHANNEL, network);

        trie_set(network->buffers, channel_name, new_channel);
        // Add a new row as a child of the network
        network_tree_model = gtk_tree_row_reference_get_model(network->row);
        gtk_tree_model_get_iter(network_tree_model, &network_iter,
                                gtk_tree_row_reference_get_path(network->row));

        gtk_tree_store_append(GTK_TREE_STORE(network_tree_model),
                               &channel_iter, &network_iter);
        gtk_tree_store_set(GTK_TREE_STORE(network_tree_model), &channel_iter,
                           0, channel_name, 1, new_channel, -1);
        
        // Store a reference to the row in the buffer
        new_channel->row = gtk_tree_row_reference_new(network_tree_model,
                gtk_tree_model_get_path(network_tree_model, &channel_iter));
    }
    else {
        struct buffer_info * buffer;
        GtkTreeIter new_user_row;
        if ((buffer = trie_get(network->buffers, channel_name)) == NULL) {
            print_to_buffer(network->buffer,
                            "Error parsing message: received JOIN from %s for "
                            "%s, but we're not in that channel!\n",
                            nickname, channel_name);
            return;
        }

        // Add the user to the user list in the channel
        gtk_list_store_append(buffer->user_list_store, &new_user_row);
        gtk_list_store_set(buffer->user_list_store, &new_user_row, 1,
                           nickname, -1);
        trie_set(buffer->users, nickname,
                 gtk_tree_row_reference_new(
                     GTK_TREE_MODEL(buffer->user_list_store),
                     gtk_tree_model_get_path(
                         GTK_TREE_MODEL(buffer->user_list_store), &new_user_row
                         )
                     )
                 );

        print_to_buffer(buffer, "* %s (%s) has joined %s\n",
                        nickname, address, channel_name);
    }
}

void part_msg_callback(struct irc_network * network,
                       char * hostmask,
                       short argc,
                       char * argv[],
                       char * trailing) {
    struct buffer_info * buffer;
    char * channel_name;
    char * nickname;
    char * address;
    split_irc_hostmask(hostmask, &nickname, &address);

    if (argc < 1) {
        if ((channel_name = trailing) == NULL) {
            print_to_buffer(network->buffer,
                            "Error parsing message: Received PART from %s "
                            "without a channel!\n", nickname);
            return;
        }
    }
    else
        channel_name = argv[0];

    if ((buffer = trie_get(network->buffers, channel_name)) == NULL) {
        print_to_buffer(network->buffer,
                        "Received a PART message for %s, but we're not in that "
                        "channel!\n", channel_name);
        return;
    }

    // Check if we're the one the part message is coming from
    if (strcmp(network->nickname, nickname) == 0) {
        // Remove the buffer from the network tree
        GtkTreeModel * network_tree_model;
        GtkTreeIter buffer_row;
        network_tree_model = gtk_tree_row_reference_get_model(buffer->row);
        gtk_tree_model_get_iter(network_tree_model,
                                &buffer_row,
                                gtk_tree_row_reference_get_path(buffer->row));

        gtk_tree_store_remove(GTK_TREE_STORE(network_tree_model), &buffer_row);

        destroy_buffer(buffer);
    }
    else {
        GtkTreeRowReference * user;
        GtkTreeIter user_row;

        if ((user = trie_get(buffer->users, nickname)) == NULL) {
            print_to_buffer(buffer->parent_network->buffer,
                            "Error parsing message: Received a PART message "
                            "from %s (%s) in %s, but the user wasn't in the "
                            "channel.\n",
                            nickname, address, channel_name);
            return;
        }

        // Find the user's row in the user list and remove it and it's reference
        gtk_tree_model_get_iter(GTK_TREE_MODEL(buffer->user_list_store),
                                &user_row,
                                gtk_tree_row_reference_get_path(user));

        // Remove the user's row and remove them from the users trie
        gtk_tree_row_reference_free(user);
        gtk_list_store_remove(buffer->user_list_store, &user_row);
        trie_del(buffer->users, nickname);

        print_to_buffer(buffer, 
                        "* %s (%s) has left the channel\n", nickname, address);
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
        buffer = new_buffer(nickname, QUERY, network);
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

        network->row = gtk_tree_row_reference_new(network_tree_model,
                gtk_tree_model_get_path(network_tree_model, &buffer_iter));
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
