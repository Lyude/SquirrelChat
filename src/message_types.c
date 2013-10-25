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

sqchat_trie * cap_features;

#define IRC_CAP_MULTI_PREFIX    1
#define IRC_CAP_SASL            2

void sqchat_init_message_types() {
    cap_features = sqchat_trie_new(sqchat_trie_strtolower);
    sqchat_trie_set(cap_features, "multi-prefix",  (void*)IRC_CAP_MULTI_PREFIX);
    sqchat_trie_set(cap_features, "sasl",          (void*)IRC_CAP_SASL);
}

#define MSG_CB(func_name)                               \
    short func_name(struct sqchat_network * network,    \
                    char * hostmask,                    \
                    short argc,                         \
                    char * argv[])

MSG_CB(sqchat_cap_msg_callback) {
    if (argc < 2)
        return SQCHAT_MSG_ERR_ARGS_FATAL;

    if (strcmp(argv[1], "LS") == 0) {
        char cap_str[SQCHAT_MSG_BUF_LEN];
        char * saveptr;
        bool features_found = false;

        memset(&cap_str, 0, SQCHAT_IRC_MSG_LEN);

        // Build the CAP REQ string
        strcat(&cap_str[0], "CAP REQ ");
        for (char * feature = strtok_r(argv[2], " ", &saveptr);
             feature != NULL;
             feature = strtok_r(NULL, " ", &saveptr)) {
            switch ((int)sqchat_trie_get(cap_features, feature)) {
                case IRC_CAP_MULTI_PREFIX:
                case IRC_CAP_SASL:
                    features_found = true;
                    strncat(&cap_str[0], " ", SQCHAT_IRC_MSG_LEN);
                    strncat(&cap_str[0], feature, SQCHAT_IRC_MSG_LEN);
                    break;
            }
        }
        // If we found features that we're compatible with
        if (features_found)
            sqchat_network_send(network, "%s\r\n", (char*)&cap_str);
        else
            goto cap_end;
    }

    else if (strcmp(argv[1], "ACK") == 0) {
        char * saveptr;
        for (char * feature = strtok_r(argv[2], " ", &saveptr);
             feature != NULL;
             feature = strtok_r(NULL, " ", &saveptr)) {
            switch ((int)sqchat_trie_get(cap_features, feature)) {
                case IRC_CAP_MULTI_PREFIX:
                    network->multi_prefix = true;
                    break;
                case IRC_CAP_SASL:
                    network->sasl = true;
                    break;
            }
        }

cap_end:
        sqchat_network_send(network, "CAP END\r\n");
        sqchat_buffer_print(network->buffer,
                        "Finished negotiating capabilities.\n");
    }
    return 0;
}

MSG_CB(sqchat_join_msg_callback) {
    if (argc < 1)
        return SQCHAT_MSG_ERR_ARGS;

    char * nickname;
    char * address;
    sqchat_split_hostmask(hostmask, &nickname, &address);

    // Check if we're the user joining a channel
    if (strcmp(network->nickname, nickname) == 0)
        sqchat_network_tree_buffer_add(sqchat_buffer_new(argv[0], CHANNEL, network), network);
    else {
        struct sqchat_buffer * buffer;
        GtkTreeIter new_user_row;
        if ((buffer = sqchat_trie_get(network->buffers, argv[0])) == NULL) {
            sqchat_buffer_print(network->buffer,
                                "Error parsing message: received JOIN from %s for "
                                "%s, but we're not in that channel!\n",
                                nickname, argv[0]);
            return SQCHAT_MSG_ERR_MISC_NODUMP;
        }

        sqchat_user_list_user_add(buffer, nickname, NULL, 0);

        sqchat_buffer_print(buffer, "* %s (%s) has joined %s\n",
                            nickname, address, argv[0]);
    }
    return 0;
}

MSG_CB(sqchat_part_msg_callback) {
    if (argc < 1)
        return SQCHAT_MSG_ERR_ARGS;

    struct sqchat_buffer * buffer;
    char * nickname;
    char * address;
    sqchat_split_hostmask(hostmask, &nickname, &address);

    if ((buffer = sqchat_trie_get(network->buffers, argv[0])) == NULL) {
        sqchat_buffer_print(network->buffer,
                            "Received a PART message for %s, but we're not in that "
                            "channel!\n", argv[0]);
        return SQCHAT_MSG_ERR_MISC_NODUMP;
    }

    // Check if we're the one the part message is coming from
    if (strcmp(network->nickname, nickname) == 0) {
        // Remove the buffer from the network tree
        sqchat_network_tree_buffer_remove(buffer);

        sqchat_buffer_destroy(buffer);
    }
    else {
        if (sqchat_user_list_user_remove(buffer, nickname) == -1) {
            sqchat_buffer_print(buffer->network->buffer,
                                "Error parsing message: Received a PART message "
                                "from %s (%s) in %s, but the user wasn't in the "
                                "channel.\n",
                                nickname, address, argv[0]);
            return SQCHAT_MSG_ERR_MISC_NODUMP;
        }
        if (argc < 2)
            sqchat_buffer_print(buffer,
                                "* %s (%s) has left %s.\n",
                                nickname, address, argv[0]);
        else
            sqchat_buffer_print(buffer,
                                "* %s (%s) has left %s (%s).\n",
                                nickname, address, argv[0], argv[1]);
    }
    return 0;
}

MSG_CB(sqchat_privmsg_msg_callback) {
    // Check if the message being sent is a CTCP
    if ((argv[1])[0] == SQCHAT_CTCP_DELIM)
        sqchat_process_ctcp(network, REQUEST, hostmask, argv[0], argv[1]);
    else {
        char * nickname;
        char * address;
        sqchat_split_hostmask(hostmask, &nickname, &address);

        // Check whether or not the message was meant to be sent to a channel
        if (SQCHAT_IS_CHAN(network, argv[0]))
            sqchat_buffer_print(sqchat_trie_get(network->buffers, argv[0]),
                                "<%s> %s\n", nickname, argv[1]);
        else {
            struct sqchat_buffer * buffer;

            if ((buffer = sqchat_trie_get(network->buffers, nickname)) == NULL) {
                buffer = sqchat_buffer_new(nickname, QUERY, network);
                sqchat_network_tree_buffer_add(buffer, network);
            }

            sqchat_buffer_print(buffer, "<%s> %s\n", nickname, argv[1]);
        }
    }
    return 0;
}

MSG_CB(sqchat_notice_msg_callback) {
    if (argc < 2)
        return SQCHAT_MSG_ERR_ARGS;

    if ((argv[1])[0] == SQCHAT_CTCP_DELIM)
        sqchat_process_ctcp(network, RESPONSE, hostmask, argv[0], argv[1]);
    else {
        char * nickname;
        char * address;
        sqchat_split_hostmask(hostmask, &nickname, &address);

        if (strcmp(argv[0], "*") == 0)
            sqchat_buffer_print(network->buffer, "* %s: %s\n", nickname, argv[1]);
        else if (strcmp(argv[0], network->nickname) == 0)
            sqchat_buffer_print(network->window->current_buffer,
                                "-%s- %s\n", nickname, argv[1]);
        else {
            struct sqchat_buffer * output;
            if ((output = sqchat_trie_get(network->buffers, argv[0])) != NULL)
                sqchat_buffer_print(network->window->current_buffer,
                                    "-%s:%s- %s\n", nickname, argv[0], argv[1]);
        }
    }
    return 0;
}

struct announce_nick_change_param {
    char * old_nick;
    char * new_nick;
};

/* Used by sqchat_trie_each in sqchat_nick_msg_callback to announce the change of a user's
 * nickname in the appropriate channels
 */
void announce_nick_change(struct sqchat_buffer * buffer,
                          struct announce_nick_change_param * params) {
    if (buffer->type != CHANNEL)
        return;

    GtkTreeRowReference * user;

    // Check if the user is in the channel
    if ((user = sqchat_trie_get(buffer->chan_data->users, params->old_nick)) != NULL) {
        GtkTreeIter user_entry;

        sqchat_buffer_print(buffer, "* %s is now known as %s\n",
                            params->old_nick, params->new_nick);

        gtk_tree_model_get_iter(GTK_TREE_MODEL(buffer->chan_data->user_list_store),
                                &user_entry,
                                gtk_tree_row_reference_get_path(user));

        // Change the user's name on the user list
        gtk_list_store_set(buffer->chan_data->user_list_store, &user_entry, 1,
                           params->new_nick, -1);

        // Remove the old entry in the user sqchat_trie and add a new one
        sqchat_trie_del(buffer->chan_data->users, params->old_nick);
        sqchat_trie_set(buffer->chan_data->users, params->new_nick, user);
    }
}

void announce_our_nick_change(struct sqchat_buffer * buffer,
                              struct announce_nick_change_param * params) {
    sqchat_buffer_print(buffer, "* You are now known as %s\n", params->new_nick);
    if (buffer->type == CHANNEL) {
        GtkTreeRowReference * row_ref;
        // Check if our nickname is listed in the channel
        if ((row_ref = sqchat_trie_get(buffer->chan_data->users, params->old_nick))
            != NULL) {
            GtkTreeIter row;

            gtk_tree_model_get_iter(
                    GTK_TREE_MODEL(buffer->chan_data->user_list_store),
                    &row, gtk_tree_row_reference_get_path(row_ref));

            // Change the user's name on the user list
            gtk_list_store_set(buffer->chan_data->user_list_store, &row, 1,
                               params->new_nick, -1);

            // Remove the old entry in the user sqchat_trie and add a new one
            sqchat_trie_del(buffer->chan_data->users, params->old_nick);
            sqchat_trie_set(buffer->chan_data->users, params->new_nick, row_ref);
        }
    }
}

MSG_CB(sqchat_nick_msg_callback) {
    if (argc < 1)
        return SQCHAT_MSG_ERR_ARGS;

    char * nickname;
    char * address;

    sqchat_split_hostmask(hostmask, &nickname, &address);

    struct announce_nick_change_param params;
    params.old_nick = nickname;
    params.new_nick = argv[0];

    // Check if we're the one whose nickname is being changed
    if (strcmp(network->nickname, nickname) == 0) {
        free(network->nickname);
        network->nickname = strdup(argv[0]);
        sqchat_buffer_print(network->buffer, "* You are now known as %s\n",
                            argv[0]);
        sqchat_trie_each(network->buffers, announce_our_nick_change, &params);

        // If the user initiated the nick change, remove their response request
        if (network->claimed_responses != NULL)
            sqchat_remove_last_response_claim(network);
    }
    else {
        struct sqchat_buffer * query;
        sqchat_trie_each(network->buffers, announce_nick_change, &params);
        /* If we were in a query with the user who changed their nickname,
         * change the name of the query to match the new nickname
         */
        if ((query = sqchat_trie_get(network->buffers, nickname)) != NULL) {
            GtkTreeIter query_row;
            GtkTreeModel * network_tree_model;

            sqchat_buffer_print(query, "* %s is now known as %s\n",
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

            sqchat_trie_del(network->buffers, nickname);
            sqchat_trie_set(network->buffers, argv[0], query);
        }
    }
    return 0;
}

MSG_CB(sqchat_ping_msg_callback) {
    sqchat_network_send(network, "PONG %s\r\n", argv[0]);
    return 0;
}

MSG_CB(sqchat_topic_msg_callback) {
    if (argc < 1)
        return SQCHAT_MSG_ERR_ARGS;

    struct sqchat_buffer * channel;
    char * nickname;
    char * address;
    sqchat_split_hostmask(hostmask, &nickname, &address);

    if ((channel = sqchat_trie_get(network->buffers, argv[0])) == NULL) {
        sqchat_buffer_print(network->buffer,
                            "Error parsing message: Received TOPIC message for "
                            "%s, but we're not in that channel.",
                            argv[0]);
        return SQCHAT_MSG_ERR_MISC_NODUMP;
    }

    sqchat_buffer_print(channel, "* %s changed the topic to \"%s\"\n",
                        nickname, argv[0]);
    return 0;
}

MSG_CB(sqchat_mode_msg_callback) {
    // Check if the target is a channel
    if (strchr(network->chantypes, *(argv[0]))) {
        struct sqchat_buffer * channel;
        if ((channel = sqchat_trie_get(network->buffers, argv[0])) == NULL) {
            sqchat_buffer_print(network->buffer,
                                "Error parsing message: Received MODE message "
                                "for %s but we're not in that channel.\n",
                                argv[0]);
            return SQCHAT_MSG_ERR_MISC;
        }

        char * nickname;
        char * address;
        sqchat_split_hostmask(hostmask, &nickname, &address);

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
                                sqchat_user_list_user_prefix_add(channel,
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
                                sqchat_user_list_user_prefix_subtract(channel,
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
                        sqchat_network_send(network, "NAMES %s\r\n", argv[0]);
                        break;
                    }
                }
            }
escape_user_mode_check:
            sqchat_buffer_print(channel, "* %s sets mode %s", nickname, argv[1]);
            for (arg_pos = 2; arg_pos < argc; arg_pos++)
                sqchat_buffer_print(channel, " %s", argv[arg_pos]);
            sqchat_buffer_print(channel, "\n");
        }
        else
            sqchat_buffer_print(channel, "* %s sets mode %s\n", nickname, argv[1]);

        // If the mode response was claimed by another command, remove the claim
        if (network->claimed_responses)
            sqchat_remove_last_response_claim(network);
    }
    else {
        struct sqchat_buffer * output;
        if (network->claimed_responses) {
            output = network->claimed_responses->buffer;
            sqchat_remove_last_response_claim(network);
        }
        else
            output = network->buffer;
        sqchat_buffer_print(output, "Your mode is %s\n", argv[1]);
    }
    return 0;
}

struct announce_quit_params {
    char * nickname;
    char * quit_msg;
};

static void announce_quit(struct sqchat_buffer * buffer,
                          struct announce_quit_params * params) {
    if (buffer->type == CHANNEL) {
        // Check if the user is in the channel
        if (sqchat_user_list_user_remove(buffer, params->nickname) != -1) {
            if (params->quit_msg == NULL)
                sqchat_buffer_print(buffer, "* %s has quit.\n", params->nickname);
            else
                sqchat_buffer_print(buffer, "* %s has quit (%s).\n",
                                    params->nickname, params->quit_msg);
        }
    }
    else {
        if (buffer->network->casecmp(buffer->network->nickname,
                                            params->nickname) == 0) {
            if (params->quit_msg == NULL)
                sqchat_buffer_print(buffer, "* %s has quit.\n", params->nickname);
            else
                sqchat_buffer_print(buffer, "* %s has quit (%s).\n",
                                    params->nickname, params->quit_msg);
        }
    }
} _nonnull(1, 2)

MSG_CB(sqchat_quit_msg_callback) {
    char * nickname;
    char * address;
    struct announce_quit_params params;
    sqchat_split_hostmask(hostmask, &nickname, &address);

    params.nickname = nickname;
    params.quit_msg = (argc >= 1) ? argv[0] : NULL;

    sqchat_trie_each(network->buffers, announce_quit, &params);
    if (network->claimed_responses)
        sqchat_remove_last_response_claim(network);
    return 0;
}

MSG_CB(sqchat_kick_msg_callback) {
    if (argc < 2)
        return SQCHAT_MSG_ERR_ARGS;

    struct sqchat_buffer * channel;
    char * nickname;
    char * address;
    sqchat_split_hostmask(hostmask, &nickname, &address);

    // Attempt to look up the channel
    if ((channel = sqchat_trie_get(network->buffers, argv[0])) == NULL) {
        sqchat_buffer_print(network->buffer,
                            "Error parsing message: Received KICK for %s, but "
                            "we're not in that channel.\n",
                            argv[0]);
        sqchat_dump_msg_to_buffer(network->buffer, hostmask, argc, argv);
        return SQCHAT_MSG_ERR_MISC_NODUMP;
    }

    // If we're the one being kicked, leave the channel
    if (network->casecmp(argv[1], network->nickname) == 0) {
        // TODO: Make this behavior a bit better
        sqchat_network_tree_buffer_remove(channel);
        sqchat_buffer_destroy(channel);
        if (argc < 3)
            sqchat_buffer_print(network->buffer,
                                "* You were kicked from %s by %s.\n",
                                argv[0], nickname);
        else
            sqchat_buffer_print(network->buffer,
                                "* You were kicked from %s by %s (%s).\n",
                                argv[0], nickname, argv[2]);
    }
    else {
        sqchat_user_list_user_remove(channel, argv[1]);
        if (argc < 3)
            sqchat_buffer_print(channel, "* %s has kicked %s from %s.\n",
                                nickname, argv[1], argv[0]);
        else
            sqchat_buffer_print(channel, "* %s has kicked %s from %s (%s).\n",
                                nickname, argv[1], argv[0], argv[2]);
        if (network->claimed_responses)
            sqchat_remove_last_response_claim(network);
    }
    return 0;
}

MSG_CB(sqchat_invite_msg_callback) {
    if (argc < 2)
        return SQCHAT_MSG_ERR_ARGS;

    char * nickname;
    char * address;
    sqchat_split_hostmask(hostmask, &nickname, &address);

    sqchat_buffer_print(network->window->current_buffer,
                        "* You have been invited to %s by %s.\n",
                        argv[1], nickname);
    return 0;
}

MSG_CB(sqchat_error_msg_callback) {
    if (argc < 1)
        return SQCHAT_MSG_ERR_ARGS;

    sqchat_buffer_print(network->buffer, "Connection error: %s\n", argv[0]);
    return 0;
}

MSG_CB(sqchat_wallops_msg_callback) {
    if (argc < 1)
        return SQCHAT_MSG_ERR_ARGS;
    
    char * nickname;
    char * address;
    sqchat_split_hostmask(hostmask, &nickname, &address);

    sqchat_buffer_print(sqchat_route_rpl_end(network), "-%s/WALLOPS- %s\n",
                        nickname, argv[0]);
    return 0;
}

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
