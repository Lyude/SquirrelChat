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
#include "ui/user_list.h"
#include "ui/network_tree.h"
#include "net_io.h"
#include "irc_macros.h"
#include "cmd_responses.h"
#include "errors.h"
#include "trie.h"
#include "ctcp.h"

#include <string.h>
#include <stdlib.h>

trie * cap_features;

#define IRC_CAP_MULTI_PREFIX    1
#define IRC_CAP_SASL            2

void init_message_types() {
    cap_features = trie_new(trie_strtolower);
    trie_set(cap_features, "multi-prefix",  (void*)IRC_CAP_MULTI_PREFIX);
    trie_set(cap_features, "sasl",          (void*)IRC_CAP_SASL);
}

#define MSG_CB(func_name)                       \
    short func_name(struct irc_network * network,\
                    char * hostmask,             \
                    short argc,                  \
                    char * argv[])

MSG_CB(cap_msg_callback) {
    return IRC_MSG_ERR_ARGS_FATAL;
    if (argc < 2)
        return IRC_MSG_ERR_ARGS_FATAL;
    
    if (strcmp(argv[1], "LS") == 0) {
        char cap_str[IRC_MSG_BUF_LEN];
        char * saveptr;
        bool features_found = false;

        memset(&cap_str, 0, IRC_MSG_LEN);

        // Build the CAP REQ string
        strcat(&cap_str[0], "CAP REQ ");
        for (char * feature = strtok_r(argv[2], " ", &saveptr);
             feature != NULL;
             feature = strtok_r(NULL, " ", &saveptr)) {
            switch ((int)trie_get(cap_features, feature)) {
                case IRC_CAP_MULTI_PREFIX:
                case IRC_CAP_SASL:
                    features_found = true;
                    strncat(&cap_str[0], " ", IRC_MSG_LEN);
                    strncat(&cap_str[0], feature, IRC_MSG_LEN);
                    break;
            }
        }
        // If we found features that we're compatible with
        if (features_found)
            send_to_network(network, "%s\r\n", (char*)&cap_str);
        else
            send_to_network(network, "CAP END\r\n");
    }

    else if (strcmp(argv[1], "ACK") == 0) {
        char * saveptr;
        for (char * feature = strtok_r(argv[2], " ", &saveptr);
             feature != NULL;
             feature = strtok_r(NULL, " ", &saveptr)) {
            switch ((int)trie_get(cap_features, feature)) {
                case IRC_CAP_MULTI_PREFIX:
                    network->multi_prefix = true;
                    break;
                case IRC_CAP_SASL:
                    network->sasl = true;
                    break;
            }
        }

        if (network->status == CAP) {
            // Finish the capability negotiation and begin registration
            print_to_buffer(network->buffer,
                            "Finished CAP negotiation.\n"
                            "Sending registration information.\n");
            send_to_network(network,
                            "CAP END\r\n"
                            "NICK %s\r\n"
                            "USER %s X X %s\r\n",
                            network->nickname, network->username,
                            network->real_name);
            network->status = CONNECTED;
        }
    }
    return 0;
}

MSG_CB(join_msg_callback) {
    if (argc < 1)
        return IRC_MSG_ERR_ARGS;

    char * nickname;
    char * address;
    split_irc_hostmask(hostmask, &nickname, &address);

    // Check if we're the user joining a channel
    if (strcmp(network->nickname, nickname) == 0)
        add_buffer_to_tree(new_buffer(argv[0], CHANNEL, network), network);
    else {
        struct buffer_info * buffer;
        GtkTreeIter new_user_row;
        if ((buffer = trie_get(network->buffers, argv[0])) == NULL) {
            print_to_buffer(network->buffer,
                            "Error parsing message: received JOIN from %s for "
                            "%s, but we're not in that channel!\n",
                            nickname, argv[0]);
            return IRC_MSG_ERR_MISC_NODUMP;
        }

        add_user_to_list(buffer, nickname, NULL, 0);

        print_to_buffer(buffer, "* %s (%s) has joined %s\n",
                        nickname, address, argv[0]);
    }
    return 0;
}

MSG_CB(part_msg_callback) {
    if (argc < 1)
        return IRC_MSG_ERR_ARGS;

    struct buffer_info * buffer;
    char * nickname;
    char * address;
    split_irc_hostmask(hostmask, &nickname, &address);

    if ((buffer = trie_get(network->buffers, argv[0])) == NULL) {
        print_to_buffer(network->buffer,
                        "Received a PART message for %s, but we're not in that "
                        "channel!\n", argv[0]);
        return IRC_MSG_ERR_MISC_NODUMP;
    }

    // Check if we're the one the part message is coming from
    if (strcmp(network->nickname, nickname) == 0) {
        // Remove the buffer from the network tree
        remove_buffer_from_tree(buffer);

        destroy_buffer(buffer);
    }
    else {
        if (remove_user_from_list(buffer, nickname) == -1) {
            print_to_buffer(buffer->network->buffer,
                            "Error parsing message: Received a PART message "
                            "from %s (%s) in %s, but the user wasn't in the "
                            "channel.\n",
                            nickname, address, argv[0]);
            return IRC_MSG_ERR_MISC_NODUMP;
        }
        if (argc < 2)
            print_to_buffer(buffer, 
                            "* %s (%s) has left %s.\n",
                            nickname, address, argv[0]);
        else
            print_to_buffer(buffer,
                            "* %s (%s) has left %s (%s).\n",
                            nickname, address, argv[0], argv[1]);
    }
    return 0;
}

MSG_CB(privmsg_msg_callback) {
    // Check if the message being sent is a CTCP
    if ((argv[1])[0] == CTCP_DELIM)
        process_ctcp(network, hostmask, argv[0], argv[1]);
    else {
        char * nickname;
        char * address;
        split_irc_hostmask(hostmask, &nickname, &address);
        
        // Check whether or not the message was meant to be sent to a channel
        if (IRC_IS_CHAN(network, argv[0]))
            print_to_buffer(trie_get(network->buffers, argv[0]),
                            "<%s> %s\n", nickname, argv[1]);
        else {
            struct buffer_info * buffer;

            if ((buffer = trie_get(network->buffers, nickname)) == NULL) {
                buffer = new_buffer(nickname, QUERY, network);
                add_buffer_to_tree(buffer, network);
            }

            print_to_buffer(buffer, "<%s> %s\n", nickname, argv[1]);
        }
    }
    return 0;
}

MSG_CB(notice_msg_callback) {
    if (argc < 2)
        return IRC_MSG_ERR_ARGS;
    
    if ((argv[1])[0] == CTCP_DELIM)
        process_ctcp(network, hostmask, argv[0], argv[1]);
    else {
        char * nickname;
        char * address;
        split_irc_hostmask(hostmask, &nickname, &address);

        if (strcmp(argv[0], "*") == 0)
            print_to_buffer(network->buffer, "* %s: %s\n", nickname, argv[1]);
        else if (strcmp(argv[0], network->nickname) == 0)
            print_to_buffer(network->window->current_buffer,
                            "-%s- %s\n", nickname, argv[1]);
        else {
            struct buffer_info * output;
            if ((output = trie_get(network->buffers, argv[0])) != NULL)
                print_to_buffer(network->window->current_buffer,
                                "-%s:%s- %s\n", nickname, argv[0], argv[1]);
        }
    }
    return 0;
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

MSG_CB(nick_msg_callback) {
    if (argc < 1)
        return IRC_MSG_ERR_ARGS;

    char * nickname;
    char * address;

    split_irc_hostmask(hostmask, &nickname, &address);

    struct announce_nick_change_param params;
    params.old_nick = nickname;
    params.new_nick = argv[0];

    // Check if we're the one whose nickname is being changed
    if (strcmp(network->nickname, nickname) == 0) {
        free(network->nickname);
        network->nickname = strdup(argv[0]);
        print_to_buffer(network->buffer, "* You are now known as %s\n",
                        argv[0]);
        trie_each(network->buffers, announce_our_nick_change, &params);

        // If the user initiated the nick change, remove their response request
        if (network->claimed_responses != NULL)
            remove_last_response_claim(network);
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
                            nickname, argv[0]);

            // Change the name of the buffer
            free(query->buffer_name);
            query->buffer_name = strdup(argv[0]);

            // Change the name of the buffer on the network tree
            network_tree_model = gtk_tree_row_reference_get_model(query->row);
            gtk_tree_model_get_iter(network_tree_model,
                                    &query_row,
                                    gtk_tree_row_reference_get_path(query->row));
            gtk_tree_store_set(GTK_TREE_STORE(network_tree_model), &query_row,
                               0, argv[0], -1);

            trie_del(network->buffers, nickname);
            trie_set(network->buffers, argv[0], query);
        }
    }
    return 0;
}

MSG_CB(ping_msg_callback) {
    send_to_network(network, "PONG %s\r\n", argv[0]);
    return 0;
}

MSG_CB(topic_msg_callback) {
    if (argc < 1)
        return IRC_MSG_ERR_ARGS;
    
    struct buffer_info * channel;
    char * nickname;
    char * address;
    split_irc_hostmask(hostmask, &nickname, &address);

    if ((channel = trie_get(network->buffers, argv[0])) == NULL) {
        print_to_buffer(network->buffer,
                        "Error parsing message: Received TOPIC message for %s, "
                        "but we're not in that channel.",
                        argv[0]);
        return IRC_MSG_ERR_MISC_NODUMP;
    }

    print_to_buffer(channel, "* %s changed the topic to \"%s\"\n",
                    nickname, argv[0]);
    return 0;
}

MSG_CB(mode_msg_callback) {
    // Check if the target is a channel
    if (strchr(network->chantypes, *(argv[0]))) {
        struct buffer_info * channel;
        if ((channel = trie_get(network->buffers, argv[0])) == NULL) {
            print_to_buffer(network->buffer,
                            "Error parsing message: Received MODE message for "
                            "%s but we're not in that channel.\n",
                            argv[0]);
            return IRC_MSG_ERR_MISC;
        }

        char * nickname;
        char * address;
        split_irc_hostmask(hostmask, &nickname, &address);

        if (argc > 2) {
            short arg_pos = 2;
            if (network->multi_prefix) {
                for (char * pos = argv[1]; ; pos++) {
                    // Check if the mode or modes are being unset
                    if (*pos == '+') {
                        for (pos++; *pos != '+' && *pos != '-'; pos++) {
                            /* If we've reached the end of the string, escape
                             * this loop
                             */
                            if (*pos == '\0')
                                goto escape_user_mode_check;

                            char * mode;
                            // Check if the mode is a user mode
                            if ((mode = strchr(network->prefix_chars, *pos))) {
                                add_prefix_to_user(channel,
                                    argv[arg_pos++], (char*)(
                                    (long)network->prefix_symbols +
                                    ((long)mode - 
                                    (long)network->prefix_chars)));
                            }
                        }
                    }
                    else {
                        for (pos++; *pos != '+' && *pos != '-'; pos++) {
                            /* If we've reached the end of the string, escape
                             * this loop
                             */
                            if (*pos == '\0')
                                goto escape_user_mode_check;

                            char * mode;
                            if ((mode = strchr(network->prefix_chars, *pos)))
                                remove_prefix_from_user(channel,
                                    argv[arg_pos++], (*(char*)(
                                    (long)network->prefix_symbols +
                                    ((long) mode -
                                    (long)network->prefix_chars))));
                        }
                    }
                }
            }
            else {
                for (char * pos = argv[1]; *pos != '\0'; pos++) {
                    if (*pos != '+' && *pos != '-' &&
                        strchr(network->prefix_chars, *pos) != NULL) {
                        send_to_network(network, "NAMES %s\r\n", argv[0]);
                        break;
                    }
                }
            }
escape_user_mode_check:
            print_to_buffer(channel, "* %s sets mode %s", nickname, argv[1]);
            for (arg_pos = 2; arg_pos < argc; arg_pos++)
                print_to_buffer(channel, " %s", argv[arg_pos]);
            print_to_buffer(channel, "\n");
        }
        else
            print_to_buffer(channel, "* %s sets mode %s\n", nickname, argv[1]);

        // If the mode response was claimed by another command, remove the claim
        if (network->claimed_responses)
            remove_last_response_claim(network);
    }
    else {
        struct buffer_info * output;
        if (network->claimed_responses) {
            output = network->claimed_responses->buffer;
            remove_last_response_claim(network);
        }
        else
            output = network->buffer;
        print_to_buffer(output, "Your mode is %s\n", argv[1]);
    }
    return 0;
}

struct announce_quit_params {
    char * nickname;
    char * quit_msg;
};

static void announce_quit(struct buffer_info * buffer,
                          struct announce_quit_params * params) {
    if (buffer->type == CHANNEL) {
        // Check if the user is in the channel
        if (remove_user_from_list(buffer, params->nickname) != -1) {
            if (params->quit_msg == NULL)
                print_to_buffer(buffer, "* %s has quit.\n", params->nickname);
            else
                print_to_buffer(buffer, "* %s has quit (%s).\n",
                                params->nickname, params->quit_msg);
        }
    }
    else {
        if (buffer->network->casecmp(buffer->network->nickname,
                                            params->nickname) == 0) {
            if (params->quit_msg == NULL)
                print_to_buffer(buffer, "* %s has quit.\n", params->nickname);
            else
                print_to_buffer(buffer, "* %s has quit (%s).\n",
                                params->nickname, params->quit_msg);
        }
    }
} _nonnull(1, 2)

MSG_CB(quit_msg_callback) {
    char * nickname;
    char * address;
    struct announce_quit_params params;
    split_irc_hostmask(hostmask, &nickname, &address);

    params.nickname = nickname;
    params.quit_msg = (argc >= 1) ? argv[0] : NULL;

    trie_each(network->buffers, announce_quit, &params);
    return 0;
}

MSG_CB(kick_msg_callback) {
    if (argc < 2)
        return IRC_MSG_ERR_ARGS;

    struct buffer_info * channel;
    char * nickname;
    char * address;
    split_irc_hostmask(hostmask, &nickname, &address);

    // Attempt to look up the channel
    if ((channel = trie_get(network->buffers, argv[0])) == NULL) {
        print_to_buffer(network->buffer,
                        "Error parsing message: Received KICK for %s, but "
                        "we're not in that channel.\n",
                        argv[0]);
        dump_msg_to_buffer(network->buffer, hostmask, argc, argv);
        return IRC_MSG_ERR_MISC_NODUMP;
    }

    // If we're the one being kicked, leave the channel
    if (network->casecmp(argv[1], network->nickname) == 0) {
        // TODO: Make this behavior a bit better
        remove_buffer_from_tree(channel);
        destroy_buffer(channel);
        if (argc < 3)
            print_to_buffer(network->buffer,
                            "* You were kicked from %s by %s.\n",
                            argv[0], nickname);
        else
            print_to_buffer(network->buffer,
                            "* You were kicked from %s by %s (%s).\n",
                            argv[0], nickname, argv[2]);
    }
    else {
        remove_user_from_list(channel, argv[1]);
        if (argc < 3)
            print_to_buffer(channel, "* %s has kicked %s from %s.\n",
                            nickname, argv[1], argv[0]);
        else
            print_to_buffer(channel, "* %s has kicked %s from %s (%s).\n",
                            nickname, argv[1], argv[0], argv[2]);
    }
    return 0;
}

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
