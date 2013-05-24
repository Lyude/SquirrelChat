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

#include <string.h>
#include <ctype.h>
#include <stdlib.h>

trie * command_trie;

// Sets up the commands trie and adds the default client commands to said trie
void init_irc_commands() {
    command_trie = trie_new(trie_strtolower);
    add_builtin_commands();
}

// Adds an IRC command
void add_irc_command(char * command,
                     irc_command_callback * callback,
                     unsigned short argc_max,
                     char * syntax_msg,
                     char * help_msg) {
    struct irc_command_info * info;
    info = malloc(sizeof(struct irc_command_info));
    info->argc_max = argc_max;
    info->syntax_msg = syntax_msg;
    info->help_msg = help_msg;
    info->callback = callback;

    trie_set(command_trie, command, info);
}

// Removes an added IRC command
void del_irc_command(char * command) {
    struct irc_command_info * info = trie_get(command_trie, command);
    free(info);
    trie_del(command_trie, command);
}

void call_command(struct buffer_info * buffer,
                  char * command,
                  char * params) {
    struct irc_command_info * info = trie_get(command_trie, command);
    unsigned short argc;

    // Make sure the command exists
    if (info == NULL) {
        print_to_buffer(buffer, "Error: Unknown command \"%s\"\n", command);
        return;
    }

    char * argv[info->argc_max];
    if (params != NULL) {
        for (argc = 0; argc < info->argc_max; argc++) {
            char * param_end = strpbrk(params, " ");
            
            // Null terminate the parameter and add it to argv
            if (param_end == NULL) {
                argv[argc] = params;
                params = NULL;
            }
            else {
                *param_end = '\0';
                argv[argc] = params;

                // Eat everything up until the next parameter, or the end of the string
                for (params = param_end + 1; *params == ' '; ++params);
            }
        }
    }
    else
        argc = 0;
    // When a command returns -1, it the parameters passed were incorrect
    if (info->callback(buffer, argc, argv, params) == -1)
        print_command_syntax(buffer, command);
}

void print_command_syntax(struct buffer_info * buffer,
                          char * command) {
    /* We don't need to check if command is valid, since this cannot be
     * explicitly called by the user in any way
     */
    print_to_buffer(buffer, "Syntax: %s\n",
        ((struct irc_command_info *)trie_get(command_trie, command))->syntax_msg);
}

void print_command_help(struct buffer_info * buffer,
                        char * command) {
    struct irc_command_info * info;
    if ((info = trie_get(command_trie, command)) == NULL)
        print_to_buffer(buffer, "Unknown command: %s\n", command);
    else {
        print_command_syntax(buffer, command);
        print_to_buffer(buffer, info->help_msg);
    }
}

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
