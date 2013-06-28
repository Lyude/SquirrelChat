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
#include "errors.h"
#include "message_parser.h"
#include "net_io.h"
#include "ui/user_list.h"

#include <errno.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <time.h>

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

/* These define the two different output behaviors many response handlers
 * follow; one for responses that check for response claims but can't satisfy
 * them, and one for responses that do the same but can satisfy claims.
 */
struct buffer_info * route_rpl(const struct irc_network * network) {
    if (network->claimed_responses != NULL)
        return network->claimed_responses->buffer;
    else
        return network->window->current_buffer;
}

struct buffer_info * route_rpl_end(struct irc_network * network) {
    if (network->claimed_responses != NULL) {
        struct buffer_info * output = network->claimed_responses->buffer;
        remove_last_response_claim(network);
        return output;
    }
    else
        return network->window->current_buffer;
}


#define NUMERIC_CB(func_name)                       \
    short func_name(struct irc_network * network,   \
                    char * hostmask,                \
                    short argc,                     \
                    char * argv[])

// Used for numerics that just give us a message requiring no special handling
NUMERIC_CB(echo_argv_1) {
    print_to_buffer(network->buffer, "%s\n", argv[1]);
    return 0;
}

NUMERIC_CB(rpl_myinfo) {
    if (argc < 5)
        return IRC_MSG_ERR_ARGS;
    else {
        network->server_name = strdup(argv[1]);
        network->version = strdup(argv[2]);
        network->usermodes = strdup(argv[3]);
        network->chanmodes = strdup(argv[4]);
        return 0;
    }
}

NUMERIC_CB(rpl_isupport) {
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
                    network->casecmp = rfc1459_strcasecmp;
                }
                else if (strcmp(value, "ascii") == 0) {
                    network->casemap_upper = trie_strtoupper;
                    network->casemap_lower = trie_strtolower;
                    network->casecmp = strcasecmp;
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
    return 0;
}

NUMERIC_CB(rpl_namreply) {
    struct buffer_info * channel;
    // The first and second parameter aren't important

    // Check if we're in the channel
    if ((channel = trie_get(network->buffers, argv[2])) != NULL) {
        // Add every single person in the reply to the list
        char * saveptr;
        for (char * nick = strtok_r(argv[3], " ", &saveptr);
             nick != NULL;
             nick = strtok_r(NULL, " ", &saveptr)) {
            char * prefix;
            GtkTreeIter user;

            // Check if the user has a user prefix
            if (strchr(network->prefix_symbols, nick[0]) != NULL) {
                prefix = nick;
                if (network->multi_prefix)
                    for (nick++;
                         strchr(network->prefix_symbols, nick[0]) != NULL;
                         nick++);
                else
                    nick++;
            }

            // Check to see if we already have the user in the list
            if (get_user_row(channel, nick, &user) != -1)
                /* Since this will really only happen on non-multi-prefix
                 * networks, we don't need to worry about updating the user's
                 * full prefix string
                 */
                set_user_prefix(channel, &user, prefix ? *prefix : '\0');
            else
                add_user_to_list(channel, nick, prefix,
                                 (size_t)(nick - prefix));
        }
    }
    return 0;
    // TODO: Print results to current buffer if we're not in the channel
}

NUMERIC_CB(rpl_endofnames) {
    // TODO: Do something here
    return 0;
}

NUMERIC_CB(rpl_motdstart) {
    struct buffer_info * output_buffer;
    // Check if the motd was requested in a different window

    print_to_buffer((network->claimed_responses) ?
                        network->claimed_responses->buffer :
                        network->buffer,
                    "---Start of MOTD---\n");
    return 0;
}

NUMERIC_CB(rpl_motd) {
    print_to_buffer((network->claimed_responses) ?
                        network->claimed_responses->buffer :
                        network->buffer,
                    "%s\n", argv[1]);
    return 0;
}

NUMERIC_CB(rpl_endofmotd) {
    if (network->claimed_responses == NULL)
        print_to_buffer(network->buffer, "---End of MOTD---\n");
    else {
        print_to_buffer(network->claimed_responses->buffer,
                        "---End of MOTD---\n");
        remove_last_response_claim(network);
    }
    return 0;
}

NUMERIC_CB(rpl_topic) {
    if (argc < 3)
        return IRC_MSG_ERR_ARGS;

    struct buffer_info * output;

    // Check if the topic was requested in a different window
    if (network->claimed_responses != NULL)
        output = network->claimed_responses->buffer;
    else if ((output = trie_get(network->buffers, argv[1])) == NULL)
        output = network->buffer;

    print_to_buffer(output, "* Topic for %s is \"%s\"\n", argv[1], argv[2]);
    return 0;
}

NUMERIC_CB(rpl_notopic) {
    if (argc < 2)
        return IRC_MSG_ERR_ARGS;

    struct buffer_info * output;
    if (network->claimed_responses != NULL) {
        output = network->claimed_responses->buffer;
        remove_last_response_claim(network);
    }
    else if ((output = trie_get(network->buffers, argv[1])) == NULL)
        output = network->buffer;

    print_to_buffer(output, "* No topic set for %s\n", argv[1]);
    return 0;
}

NUMERIC_CB(rpl_topicwhotime) {
    if (argc < 3)
        return IRC_MSG_ERR_ARGS;

    struct buffer_info * output;
    char * nickname;
    char * address;

    // Check if the response was requested in another buffer
    if (network->claimed_responses != NULL) {
        // RPL_TOPICWHOTIME is the last response for /topic
        output = network->claimed_responses->buffer;
        remove_last_response_claim(network);
    }
    else if ((output = trie_get(network->buffers, argv[1])) == NULL)
        output = network->buffer;

    split_irc_hostmask(argv[2], &nickname, &address);

    print_to_buffer(output, "* Set by %s (%s)\n", nickname, address);
    return 0;
}

NUMERIC_CB(rpl_channelmodeis) {
    if (argc < 3)
        return IRC_MSG_ERR_ARGS;

    struct buffer_info * output;
    /* Check if the response was requested in another channel
     * We don't remove the claimed response since most networks will follow up
     * with a RPL_CREATIONTIME response
     */
    if (network->claimed_responses)
        output = network->claimed_responses->buffer;
    else if ((output = trie_get(network->buffers, argv[1])) == NULL)
        output = network->buffer;

    print_to_buffer(output, "The modes for %s are: %s\r\n",
                    argv[1], argv[2]);
    return 0;
}

NUMERIC_CB(rpl_creationtime) {
    if (argc < 3)
        return IRC_MSG_ERR_ARGS;

    struct buffer_info * output;
    unsigned long epoch_time;

    if (network->claimed_responses) {
        output = network->claimed_responses->buffer;
        remove_last_response_claim(network);
    }
    else if ((output = trie_get(network->buffers, argv[1])) == NULL)
        output = network->buffer;

    errno = 0;
    if ((epoch_time = strtoul(argv[2], NULL, 10)) == 0 &&
        errno != 0) {
        print_to_buffer(network->buffer,
                        "Error parsing message: Received invalid EPOCH time in "
                        "RPL_CREATIONTIME.\n"
                        "strtoul() returned the following error: %s\n",
                        strerror(errno));
        return IRC_MSG_ERR_MISC;
    }

    print_to_buffer(output, "* Channel created on %s",
                    ctime((const long *)&epoch_time));
    return 0;
}

NUMERIC_CB(rpl_whoisuser) {
    if (argc < 6)
        return IRC_MSG_ERR_ARGS;

    print_to_buffer(route_rpl(network), "[%s] address is: %s@%s\n"
                                        "[%s] real name is: %s\n",
                    argv[1], argv[2], argv[3], argv[1], argv[5]);
    return 0;
}

NUMERIC_CB(rpl_whoisserver) {
    if (argc < 3)
        return IRC_MSG_ERR_ARGS;

    struct buffer_info * output = route_rpl(network);

    if (argc == 3)
        print_to_buffer(output, "[%s] connected to %s\n", argv[1],
                        argv[2]);
    else
        print_to_buffer(output, "[%s] connected to %s: %s\n", argv[1],
                        argv[2], argv[3]);
    return 0;
}

NUMERIC_CB(rpl_whoisoperator) {
    if (argc < 2)
        return IRC_MSG_ERR_ARGS;

    print_to_buffer(route_rpl(network), "[%s] %s\n", argv[1], argv[2]);
    return 0;
}

NUMERIC_CB(rpl_whoisidle) {
    if (argc < 2)
        return IRC_MSG_ERR_ARGS;

    print_to_buffer(route_rpl(network),
                    "[%s] has been idle for %s seconds\n", argv[1], argv[2]);
    return 0;
}

NUMERIC_CB(rpl_whoischannels) {
    if (argc < 3)
        return IRC_MSG_ERR_ARGS;

    print_to_buffer(route_rpl(network),
                    "[%s] channels: %s\n", argv[1], argv[2]);
    return 0;
}

NUMERIC_CB(rpl_whoissecure) {
    print_to_buffer(route_rpl(network),
                    "[%s] %s\n", argv[1], argv[2]);
    return 0;
}

NUMERIC_CB(rpl_whoisaccount) {
    if (argc < 4)
        return IRC_MSG_ERR_ARGS;

    print_to_buffer(route_rpl(network), "[%s] %s %s\n",
                    argv[1], argv[3], argv[2]);
    return 0;
}

NUMERIC_CB(rpl_whoisactually) {
    if (argc < 4)
        return IRC_MSG_ERR_ARGS;

    print_to_buffer(route_rpl(network), "[%s] %s %s\n",
                    argv[1], argv[3], argv[2]);
    return 0;
}

// Used for all whois replies that pretty much just echo back their arguments
NUMERIC_CB(rpl_whois_generic) {
    if (argc < 3)
        return IRC_MSG_ERR_ARGS;

    print_to_buffer(route_rpl(network), "[%s] %s\n",
                    argv[1], argv[2]);
    return 0;
}

NUMERIC_CB(rpl_endofwhois) {
    if (argc < 2)
        return IRC_MSG_ERR_ARGS;

    print_to_buffer(route_rpl_end(network),
                    "[%s] End of WHOIS.\n", argv[1]);
    return 0;
}

NUMERIC_CB(rpl_whowasuser) {
    if (argc < 6)
        return IRC_MSG_ERR_ARGS;

    print_to_buffer(route_rpl(network), "[%s] address was: %s@%s\n"
                                        "[%s] real name was: %s\n",
                    argv[1], argv[2], argv[3], argv[1], argv[5]);
    return 0;
}

NUMERIC_CB(rpl_endofwhowas) {
    if (argc < 2)
        return IRC_MSG_ERR_ARGS;

    print_to_buffer(route_rpl_end(network),
                    "[%s] End of WHOWAS.\n", argv[1]);
    return 0;
}

// Used for generic errors with only an error message
NUMERIC_CB(generic_error) {
    struct buffer_info * output = route_rpl_end(network);

    print_to_buffer(output, "Error: %s\n", argv[0]);
    return 0;
}

// Used for errors that can potentially affect the status of the connection
NUMERIC_CB(generic_network_error) {
    print_to_buffer(network->buffer, "Error: %s\n", argv[0]);
    return 0;
}

// Used for generic errors that come with a channel argument
NUMERIC_CB(generic_channel_error) {
    if (argc < 3)
        return IRC_MSG_ERR_ARGS;

    struct buffer_info * output;
    if (network->claimed_responses) {
        output = network->claimed_responses->buffer;
        remove_last_response_claim(network);
    }
    else if ((output = trie_get(network->buffers, argv[1])) == NULL)
        output = network->buffer;

    print_to_buffer(output, "Error: %s: %s\n", argv[1], argv[2]);
    return 0;
}

// Used for generic errors that come with a command argument
NUMERIC_CB(generic_command_error) {
    if (argc < 3)
        return IRC_MSG_ERR_ARGS;

    struct buffer_info * output = route_rpl_end(network);

    print_to_buffer(output, "Error: %s: %s\n", argv[1], argv[2]);
    return 0;
}

// Used for errors with a single non-channel argument
NUMERIC_CB(generic_target_error) {
    if (argc < 3)
        return IRC_MSG_ERR_ARGS;

    struct buffer_info * output = route_rpl_end(network);

    print_to_buffer(output, "Error: %s: %s\n", argv[1], argv[2]);
    return 0;
}

// Used for errors with a user argument and a channel argument
NUMERIC_CB(generic_user_channel_error) {
    if (argc < 4)
        return IRC_MSG_ERR_ARGS;

    struct buffer_info * output = route_rpl_end(network);

    print_to_buffer(output, "Error: %s with %s: %s\n",
                    argv[1], argv[0], argv[2]);
    return 0;
}

NUMERIC_CB(generic_lusers_rpl) {
    if (argc < 3)
        return IRC_MSG_ERR_ARGS;

    print_to_buffer(route_rpl(network), "%s %s\n", argv[1], argv[2]);
    return 0;
}

NUMERIC_CB(rpl_localglobalusers) {
    /* There's no standard for which argument contains the string we need, so we
     * have to just assume it's the last argument (which it should be)
     */
    print_to_buffer(route_rpl(network), "%s\n", argv[argc-1]);
    return 0;
}

NUMERIC_CB(rpl_inviting) {
    if (argc < 3)
        return IRC_MSG_ERR_ARGS;

    print_to_buffer(route_rpl_end(network),
                    "* Invitation for %s to join %s was successfully sent.\n",
                    argv[1], argv[2]);
    return 0;
}

NUMERIC_CB(generic_echo_rpl) {
    if (argc < 2)
        return IRC_MSG_ERR_ARGS;

    print_to_buffer(route_rpl(network), "%s\n", argv[1]);
    return 0;
}

NUMERIC_CB(generic_echo_rpl_end) {
    if (argc < 2)
        return IRC_MSG_ERR_ARGS;

    print_to_buffer(route_rpl_end(network), "%s\n", argv[1]);
    return 0;
}

NUMERIC_CB(nick_change_error) {
    if (network->claimed_responses == NULL)
        return 0;

    print_to_buffer(network->claimed_responses->buffer,
                    "Could not change nickname to %s: %s\n",
                    network->claimed_responses->data, argv[2]);
    remove_last_response_claim(network);
    return 0;
}

NUMERIC_CB(err_notregistered) {
    /* The only time our client could ever get this is during the CAP
     * negotiation, which means that the server does not support IRCv3 and
     * inherently does not support CAP
     */
    print_to_buffer(network->buffer,
                    "CAP negotiation failed: Server does not support CAP.\n"
                    "Sending registration information.\n");
    send_to_network(network,
                    "NICK %s\r\n"
                    "USER %s X X %s\r\n",
                    network->nickname, network->username, network->real_name);
    network->status = CONNECTED;
    return 0;
}
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
