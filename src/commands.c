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
#include "commands.h"
#include "builtin_commands.h"
#include "trie.h"
#include "ui/chat_window.h"
#include "ui/buffer.h"
#include "net_io.h"

#include <string.h>
#include <ctype.h>
#include <stdlib.h>

sqchat_trie * sqchat_command_trie;

/* Sets up the commands sqchat_trie and adds the default client commands to 
 * said trie
 */
void sqchat_init_irc_commands() {
    sqchat_command_trie = sqchat_trie_new(sqchat_trie_strtolower);
    sqchat_add_builtin_commands();
}

// Adds an IRC command
void sqchat_add_irc_command(char * command,
                            sqchat_command_callback callback,
                            unsigned short argc_max,
                            char * syntax_msg,
                            char * help_msg) {
    struct sqchat_command_info * info;
    info = malloc(sizeof(struct sqchat_command_info));
    info->argc_max = argc_max;
    info->syntax_msg = syntax_msg;
    info->help_msg = help_msg;
    info->callback = callback;

    sqchat_trie_set(sqchat_command_trie, command, info);
}

// Removes an added IRC command
void sqchat_del_irc_command(char * command) {
    struct sqchat_command_info * info =
        sqchat_trie_get(sqchat_command_trie, command);
    free(info);
    sqchat_trie_del(sqchat_command_trie, command);
}

void sqchat_call_command(struct sqchat_buffer * buffer,
                         char * command,
                         char * params) {
    struct sqchat_command_info * info =
        sqchat_trie_get(sqchat_command_trie, command);
    unsigned short argc;

    // Make sure the command exists
    if (info == NULL) {
        // If it doesn't exist, send the command to the server if possible
        if (buffer->network->status != DISCONNECTED) {
            if (params == NULL)
                sqchat_network_send(buffer->network, "%s\r\n", command);
            else
                sqchat_network_send(buffer->network, "%s %s\r\n", command, params);
        }
        else
            sqchat_buffer_print(buffer, "Error: Unknown command \"%s\"\n", command);
        return;
    }

    char * argv[info->argc_max];
    if (params != NULL) {
        for (argc = 0; argc < info->argc_max; argc++) {
            char * param_end = strpbrk(params, " ");

            // Null terminate the parameter and add it to argv
            if (param_end == NULL) {
                argv[argc++] = params;
                params = NULL;
                break;
            }
            else {
                *param_end = '\0';
                argv[argc] = params;

                /* Eat everything up until the next parameter, or the end of 
                 * the string
                 */
                for (params = param_end + 1; *params == ' '; ++params);
            }
        }
    }
    else
        argc = 0;
    // When a command returns -1, it the parameters passed were incorrect
    if (info->callback(buffer, argc, argv, params) == -1)
        sqchat_print_command_syntax(buffer, command);
}

void sqchat_print_command_syntax(struct sqchat_buffer * buffer,
                                 char * command) {
    /* We don't need to check if command is valid, since this cannot be
     * explicitly called by the user in any way
     */
    sqchat_buffer_print(buffer, "Syntax: %s\n",
        ((struct sqchat_command_info *)sqchat_trie_get(sqchat_command_trie, command))->syntax_msg);
}

void sqchat_print_command_help(struct sqchat_buffer * buffer,
                               char * command) {
    struct sqchat_command_info * info;
    if ((info = sqchat_trie_get(sqchat_command_trie, command)) == NULL)
        sqchat_buffer_print(buffer, "Unknown command: %s\n", command);
    else {
        sqchat_print_command_syntax(buffer, command);
        sqchat_buffer_print(buffer, "%s", info->help_msg);
    }
}

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
