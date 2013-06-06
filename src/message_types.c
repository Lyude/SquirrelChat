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
#include "cmd_responses.h"
#include "errors.h"

#include <string.h>
#include <stdlib.h>

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
                        "* %s (%s) has left %s\n",
                        nickname, address, channel_name);
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
    
    // Check whether or not the message was meant to be sent to a channel
    if (strcmp(argv[0], network->nickname) == 0) {
        if ((buffer = trie_get(network->buffers, nickname)) == NULL) {
            buffer = new_buffer(nickname, QUERY, network);
            GtkTreeModel * network_tree_model;
            GtkTreeIter network_iter;
            GtkTreeIter buffer_iter;

            network_tree_model = gtk_tree_row_reference_get_model(network->row);
            gtk_tree_model_get_iter(network_tree_model, &network_iter,
                                    gtk_tree_row_reference_get_path(
                                        network->row
                                    ));

            gtk_tree_store_append(GTK_TREE_STORE(network_tree_model),
                                  &buffer_iter, &network_iter);
            gtk_tree_store_set(GTK_TREE_STORE(network_tree_model), &buffer_iter,
                               0, nickname, 1, buffer, -1);

            buffer->row = gtk_tree_row_reference_new(network_tree_model,
                    gtk_tree_model_get_path(network_tree_model, &buffer_iter));

            trie_set(network->buffers, nickname, buffer);
        }
    }
    else
        buffer = trie_get(network->buffers, argv[0]);
    
    print_to_buffer(buffer, "<%s> %s\n", nickname, trailing);
}

struct announce_nick_change_param {
    char * old_nick;
    char * new_nick;
};

/* Used by trie_each in nick_msg_callback to announce the change of a user's
 * nickname in the appropriate channels
 */
void announce_nick_change(struct buffer_info * buffer,
                          struct announce_nick_change_param * params) {
    if (buffer->type != CHANNEL)
        return;

    GtkTreeRowReference * user;

    // Check if the user is in the channel
    if ((user = trie_get(buffer->users, params->old_nick)) != NULL) {
        GtkTreeIter user_entry;

        print_to_buffer(buffer, "* %s is now known as %s\n",
                        params->old_nick, params->new_nick);

        gtk_tree_model_get_iter(GTK_TREE_MODEL(buffer->user_list_store),
                                &user_entry,
                                gtk_tree_row_reference_get_path(user));

        // Change the user's name on the user list
        gtk_list_store_set(buffer->user_list_store, &user_entry, 1,
                           params->new_nick, -1);

        // Remove the old entry in the user trie and add a new one
        trie_del(buffer->users, params->old_nick);
        trie_set(buffer->users, params->new_nick, user);
    }
}

void announce_our_nick_change(struct buffer_info * buffer,
                              struct announce_nick_change_param * params) {
    print_to_buffer(buffer, "* You are now known as %s\n", params->new_nick);
    if (buffer->type == CHANNEL) {
        GtkTreeRowReference * row_ref;
        // Check if our nickname is listed in the channel
        if ((row_ref = trie_get(buffer->users, params->old_nick)) != NULL) {
            GtkTreeIter row;

            gtk_tree_model_get_iter(GTK_TREE_MODEL(buffer->user_list_store),
                                    &row,
                                    gtk_tree_row_reference_get_path(row_ref));

            // Change the user's name on the user list
            gtk_list_store_set(buffer->user_list_store, &row, 1,
                               params->new_nick, -1);

            // Remove the old entry in the user trie and add a new one
            trie_del(buffer->users, params->old_nick);
            trie_set(buffer->users, params->new_nick, row_ref);
        }
    }
}

void nick_msg_callback(struct irc_network * network,
                       char * hostmask,
                       short argc,
                       char * argv[],
                       char * trailing) {
    char * new_nickname;
    char * nickname;
    char * address;

    if (trailing != NULL)
        new_nickname = trailing;
    else if (argc >= 1)
        new_nickname = argv[0];
    else {
        print_to_buffer(network->buffer,
                        "Received NICK message, but did not receive a "
                        "nickname?\n");
        dump_msg_to_buffer(network->buffer, hostmask, argc, argv, trailing);
        return;
    }

    split_irc_hostmask(hostmask, &nickname, &address);

    struct announce_nick_change_param params;
    params.old_nick = nickname;
    params.new_nick = new_nickname;

    // Check if we're the one whose nickname is being changed
    if (strcmp(network->nickname, nickname) == 0) {
        irc_response_queue ** request;
        free(network->nickname);
        network->nickname = strdup(new_nickname);
        trie_each(network->buffers, announce_our_nick_change, &params);

        // If the user initiated the nick change, remove their response request
        if ((request = find_cmd_response_request(network, IRC_NICK_RESPONSE)) !=
            NULL)
            remove_cmd_response_request(request);
    }
    else {
        struct buffer_info * query;
        trie_each(network->buffers, announce_nick_change, &params);
        /* If we were in a query with the user who changed their nickname,
         * change the name of the query to match the new nickname
         */
        if ((query = trie_get(network->buffers, nickname)) != NULL) {
            GtkTreeIter query_row;
            GtkTreeModel * network_tree_model;

            print_to_buffer(query, "* %s is now known as %s\n",
                            nickname, new_nickname);

            // Change the name of the buffer
            free(query->buffer_name);
            query->buffer_name = strdup(new_nickname);

            // Change the name of the buffer on the network tree
            network_tree_model = gtk_tree_row_reference_get_model(query->row);
            gtk_tree_model_get_iter(network_tree_model,
                                    &query_row,
                                    gtk_tree_row_reference_get_path(query->row));
            gtk_tree_store_set(GTK_TREE_STORE(network_tree_model), &query_row,
                               0, new_nickname, -1);

            trie_del(network->buffers, nickname);
            trie_set(network->buffers, new_nickname, query);
        }
    }
}

void ping_msg_callback(struct irc_network * network,
                       char * hostmask,
                       short argc,
                       char * argv[],
                       char * trailing) {
    send_to_network(network, "PONG %s\r\n", trailing);
}

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
