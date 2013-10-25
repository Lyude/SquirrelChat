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
#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#include "ui/buffer.h"
#include "ui/chat_window.h"

// sqchat_command_callback(current buffer, argv, argc, trailing)
typedef short (*sqchat_command_callback)(struct sqchat_buffer *,
                                         unsigned short,
                                         char*[],
                                         char*);

struct sqchat_command_info {
    unsigned short argc_max; // Specifies max size of argv
    char * syntax_msg;
    char * help_msg;
    sqchat_command_callback callback;
};
#define SQCHAT_CMD_SYNTAX_ERR -1

extern void sqchat_init_irc_commands();

extern void sqchat_add_irc_command(char * command,
                                   sqchat_command_callback callback,
                                   unsigned short argc_max,
                                   char * syntax_msg,
                                   char * help_msg)
    _nonnull(1, 2, 4, 5);
extern void sqchat_del_irc_command(char * command)
    _nonnull(1);
extern void sqchat_call_command(struct sqchat_buffer * buffer,
                                char * command,
                                char * params)
    _nonnull(1, 2);
extern void sqchat_print_command_syntax(struct sqchat_buffer * buffer,
                                        char * command)
    _nonnull(1, 2);
extern void sqchat_print_command_help(struct sqchat_buffer * buffer,
                                      char * command)
    _nonnull(1, 2);

#endif // __COMMANDS_H__
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
