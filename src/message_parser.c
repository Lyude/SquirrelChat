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

#include "message_parser.h"
#include "message_types.h"

#include "trie.h"
#include "irc_network.h"
#include "irc_numerics.h"
#include "numerics.h"
#include "ui/buffer.h"
#include "chat.h"
#include "errors.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <stdio.h>

#define MAX_POSSIBLE_PARAMS ((IRC_MSG_LEN - sizeof(":X X ")) / sizeof("X "))

trie * message_types;

irc_message_callback * numerics;

/* A function to convert strings to shorts, optimized specifically for IRC
 * numerics
 */
short numeric_to_short(char * numeric) {
    short result = 0;
    short i;
    for (i = 0; i < 3; i++) {
        if (numeric[i] < '0' || numeric[i] > '9')
            return -1;
        result = (result * 10) + (numeric[i] - '0');
    }
    if (numeric[i] != '\0')
        return -1;
    else
        return result;
}

void init_message_parser() {
    message_types = trie_new(trie_strtoupper);
    numerics = calloc(IRC_NUMERIC_MAX, sizeof(irc_message_callback*));

    // Add in the built in message types
    trie_set(message_types, "JOIN", join_msg_callback);
    trie_set(message_types, "PART", part_msg_callback);
    trie_set(message_types, "PRIVMSG", privmsg_msg_callback);
    trie_set(message_types, "PING", ping_msg_callback);

    numerics[IRC_RPL_WELCOME] = echo_numeric;
    numerics[IRC_RPL_YOURHOST] = echo_numeric;
    numerics[IRC_RPL_CREATED] = echo_numeric;
    numerics[IRC_RPL_MYINFO] = rpl_myinfo;
    numerics[IRC_RPL_ISUPPORT] = rpl_isupport;
    numerics[IRC_RPL_NAMREPLY] = rpl_namreply;
    numerics[IRC_RPL_ENDOFNAMES] = rpl_endofnames;
}

void process_irc_message(struct irc_network * network, char * msg) {
    char * cursor;
    char * hostmask;
    char * command;
    char * params;
    short numeric;
    irc_message_callback callback;

    /* TODO: Maybe figure out a better behavior for when bad messages are
     * received...
     */
    // Check to see if the message has a sender
    if (msg[0] == ':') {
        if ((hostmask = strtok_r(msg+1, " ", &cursor)) == NULL)
            return;
        if ((command = strtok_r(NULL, " ", &cursor)) == NULL)
            return;
    }
    else {
        hostmask = NULL;
        if ((command = strtok_r(msg, " ", &cursor)) == NULL)
            return;
    }

    char * argv[MAX_POSSIBLE_PARAMS];
    short argc;
    char * trailing = NULL;
    for (argc = 0; ; argc++) {
        char * param_end;
        
        /* If there's a ':' at the beginning of the message we need to stop
         * processing spaces
         */
        if (*cursor == ':') {
            trailing = cursor + 1;
            break;
        }

        else if ((param_end = strchr(cursor, ' ')) == NULL) {
            argv[argc] = cursor;
            if (argc == 0)
                argc++;
            break;
        }
        *param_end = '\0';
        argv[argc] = cursor;
        for (cursor = param_end + 1; *cursor == ' '; cursor++);
    }

    if ((numeric = numeric_to_short(command)) != -1) {
        if (numeric > 0 && numeric <= IRC_NUMERIC_MAX && numerics[numeric] != NULL)
            numerics[numeric](network, hostmask, argc, argv, trailing);
        else {
            print_to_buffer(network->buffer,
                            "Error parsing message: unknown numeric %i\n",
                            numeric);
            dump_msg_to_buffer(network->buffer, hostmask, argc, argv, trailing);
        }
    }
    // Attempt to look up the command
    else if ((callback = trie_get(message_types, command)) != NULL)
        callback(network, hostmask, argc, argv, trailing);
    else {
        print_to_buffer(network->buffer,
                        "Error parsing message: unknown message type: \"%s\"\n",
                        command);
        dump_msg_to_buffer(network->buffer, hostmask, argc, argv, trailing); 
    }
}

void split_irc_hostmask(char * hostmask, char ** nickname, char ** address) {
    char * saveptr;
    *nickname = strtok_r(hostmask, "!", &saveptr);
    *address = strtok_r(NULL, "!", &saveptr);
}
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
