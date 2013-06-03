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

#include "trie.h"
#include "commands.h"
#include "casemap.h"
#include "cmd_responses.h"
#include "irc_numerics.h"

#include <string.h>
#include <stdlib.h>

trie * isupport_tokens;
#define ISUPPORT_CHANTYPES      1
#define ISUPPORT_EXCEPTS        2
#define ISUPPORT_INVEX          3
#define ISUPPORT_CHANMODES      4
#define ISUPPORT_PREFIX         5
#define ISUPPORT_NETWORK        6
#define ISUPPORT_ELIST          7
#define ISUPPORT_CALLERID       8
#define ISUPPORT_CASEMAPPING    9

void init_numerics() {
    isupport_tokens = trie_new(trie_strtoupper);
    trie_set(isupport_tokens, "CHANTYPES",  (void*)ISUPPORT_CHANTYPES);
    trie_set(isupport_tokens, "EXCEPTS",    (void*)ISUPPORT_EXCEPTS);
    trie_set(isupport_tokens, "INVEX",      (void*)ISUPPORT_INVEX);
    trie_set(isupport_tokens, "CHANMODES",  (void*)ISUPPORT_CHANMODES);
    trie_set(isupport_tokens, "PREFIX",     (void*)ISUPPORT_PREFIX);
    trie_set(isupport_tokens, "NETWORK",    (void*)ISUPPORT_NETWORK);
    trie_set(isupport_tokens, "ELIST",      (void*)ISUPPORT_ELIST);
    trie_set(isupport_tokens, "CALLERID",   (void*)ISUPPORT_CALLERID);
    trie_set(isupport_tokens, "ACCEPT",     (void*)ISUPPORT_CALLERID);
    trie_set(isupport_tokens, "CASEMAPPING",(void*)ISUPPORT_CASEMAPPING);
}

// Used for numerics that just give us a message requiring no special handling
void echo_numeric(struct irc_network * network,
                  char * hostmask,
                  short argc,
                  char * argv[],
                  char * trailing) {
    print_to_buffer(network->buffer, "%s\n", trailing);
}

void rpl_myinfo(struct irc_network * network,
                char * hostmask,
                short argc,
                char * argv[],
                char * trailing) {
    if (argc < 5) {
        print_to_buffer(network->buffer, 
                        "Error parsing message: Received invalid RPL_MYINFO: "
                        "not enough arguments provided (only given %i)\n",
                        argc);
        dump_msg_to_buffer(network->buffer, hostmask, argc, argv, trailing);
    }
    else {
        network->version = strdup(argv[2]);
        network->usermodes = strdup(argv[3]);
        network->chanmodes = strdup(argv[4]);
    }
}

void rpl_isupport(struct irc_network * network,
                  char * hostmask,          
                  short argc,
                  char * argv[],
                  char * trailing) {
    for (short i = 1; i < argc; i++) {
        char * saveptr;
        char * saveptr2;
        char * token = strtok_r(argv[i], "=", &saveptr);
        char * value = strtok_r(NULL, "=", &saveptr);
        switch ((int)trie_get(isupport_tokens, token)) {
            case ISUPPORT_CHANTYPES:
                free(network->chantypes);
                network->chantypes = strdup(value);
                break;
            case ISUPPORT_EXCEPTS:
                network->excepts = true;
                break;
            case ISUPPORT_INVEX:
                network->invex = true;
                break;
            case ISUPPORT_CHANMODES:
                network->chanmodes_a = strdup(strtok_r(value, ",", &saveptr2));
                network->chanmodes_b = strdup(strtok_r(NULL, ",", &saveptr2));
                network->chanmodes_c = strdup(strtok_r(NULL, ",", &saveptr2));
                network->chanmodes_d = strdup(strtok_r(NULL, ",", &saveptr2));
                break;
            case ISUPPORT_PREFIX:
                if (value[0] != '(') {
                    print_to_buffer(network->buffer,
                                    "Error parsing message: PREFIX parameters "
                                    "are invalid, server provided: %s\n",
                                    value);
                    continue;
                }
                // Eat the first (
                value++;
                
                network->prefix_chars = strdup(strtok_r(value, ")", &saveptr2));
                network->prefix_symbols = strdup(strtok_r(NULL, ")", &saveptr2));
                break;
            case ISUPPORT_NETWORK:
                free(network->name);
                network->name = strdup(value);
                
                // Update the network name in the network tree
                {
                    GtkTreeIter network_row;
                    GtkTreeModel * network_model = 
                        gtk_tree_row_reference_get_model(network->row);

                    gtk_tree_model_get_iter(network_model, &network_row,
                            gtk_tree_row_reference_get_path(network->row));

                    gtk_tree_store_set(GTK_TREE_STORE(network_model),
                                       &network_row, 0, network->name, -1);
                }
                //TODO: do this 
                break;
            case ISUPPORT_CALLERID:
                network->callerid = true;
                break;
            case ISUPPORT_CASEMAPPING:
                if (strcmp(value, "rfc1459") == 0) {
                    network->casemap_upper = trie_rfc1459_strtoupper;
                    network->casemap_lower = trie_rfc1459_strtolower;
                }
                else if (strcmp(value, "ascii") == 0) {
                    network->casemap_upper = trie_strtoupper;
                    network->casemap_lower = trie_strtolower;
                }
                else {
                    print_to_buffer(network->buffer,
                                    "WARNING: Unknown casemap \"%s\" specified "
                                    "by network. Defaulting to rfc1459.",
                                    value);
                    network->casemap_upper = trie_rfc1459_strtoupper;
                    network->casemap_lower = trie_rfc1459_strtolower;
                }
                break;
        }
    }
}

void rpl_namreply(struct irc_network * network,
                  char * hostmask,
                  short argc,
                  char * argv[],
                  char * trailing) {
    struct buffer_info * channel;
    // The first and second parameter aren't important
    
    // Check if we're in the channel
    if ((channel = trie_get(network->buffers, argv[2])) != NULL) {
        // Add every single person in the reply to the list
        char * saveptr;
        for (char * nick = strtok_r(trailing, " ", &saveptr);
             nick != NULL;
             nick = strtok_r(NULL, " ", &saveptr)) {
            char * prefix;
            GtkTreeIter new_user;

            // Check if the user has a user prefix
            if ((prefix = strchr(network->prefix_symbols, nick[0])) != NULL)
                nick++;

            // Check to see if we already have the user in the list
            if (trie_get(channel->users, nick) != NULL)
                continue;

            gtk_list_store_append(channel->user_list_store, &new_user);

            if (prefix != NULL) {
                char prefix_str[2];
                prefix_str[0] = *prefix;
                prefix_str[1] = '\0';
                gtk_list_store_set(channel->user_list_store,
                                   &new_user, 0, &prefix_str[0], -1);
            }

            gtk_list_store_set(channel->user_list_store,
                               &new_user, 1, nick, -1);

            trie_set(channel->users, nick,
                     gtk_tree_row_reference_new(
                         GTK_TREE_MODEL(channel->user_list_store),
                         gtk_tree_model_get_path(
                             GTK_TREE_MODEL(channel->user_list_store), &new_user)
                     ));
        }
    }
    // TODO: Print results to current buffer if we're not in the channel
}

void rpl_endofnames(struct irc_network * network,
                    char * hostmask,
                    short argc,
                    char * argv[],
                    char * trailing) {
    // TODO: Do something here
}

void rpl_motdstart(struct irc_network * network,
                   char * hostmask,
                   short argc,
                   char * argv[],
                   char * trailing) {
    irc_response_queue ** request;
    struct buffer_info * output_buffer;
    // Check if the motd was requested in a different window
    request = find_cmd_response_request(network, IRC_RPL_MOTD);
    output_buffer = (request == NULL) ? network->buffer : (*request)->buffer;

    print_to_buffer(output_buffer, "---Start of MOTD---\n");
}

void rpl_motd(struct irc_network * network,
              char * hostmask,
              short argc,
              char * argv[],
              char * trailing) {
    irc_response_queue ** request;
    struct buffer_info * output_buffer;
    // Check if the motd was requested in a different window
    request = find_cmd_response_request(network, IRC_RPL_MOTD);
    output_buffer = (request == NULL) ? network->buffer : (*request)->buffer;

    print_to_buffer(output_buffer, "%s\n", trailing);
}

void rpl_endofmotd(struct irc_network * network,
                  char * hostmask,
                  short argc,
                  char * argv[],
                  char * trailing) {
    irc_response_queue ** request;
    // Check if the motd was requested in a different window
    request = find_cmd_response_request(network, IRC_RPL_MOTD);
    if (request == NULL)
        print_to_buffer(network->buffer, "---End of MOTD---\n");
    else {
        print_to_buffer((*request)->buffer, "---End of MOTD---\n");
        remove_cmd_response_request(request);
    }
}

void nick_change_error(struct irc_network * network,
                       char * hostmask,
                       short argc,
                       char * argv[],
                       char * trailing) {
    irc_response_queue ** request;
    if ((request = find_cmd_response_request(network, IRC_NICK_RESPONSE)) ==
        NULL)
        return;

    print_to_buffer((*request)->buffer, "Could not change nickname to %s: %s\n",
                    (*request)->data, trailing);
    remove_cmd_response_request(request);
}

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
