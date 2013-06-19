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

// irc_command_callback(current buffer, argv, argc, trailing)
typedef short (*irc_command_callback)(struct buffer_info *,
                                      unsigned short,
                                      char*[],
                                      char*);

struct irc_command_info {
    unsigned short argc_max; // Specifies max size of argv
    char * syntax_msg;
    char * help_msg;
    irc_command_callback callback;
};
#define IRC_CMD_ARGC_MAX USHRT_MAX
#define IRC_CMD_SYNTAX_ERR -1

extern void add_irc_command(char * command,
                            irc_command_callback callback,
                            unsigned short argc_max,
                            char * syntax_msg,
                            char * help_msg);
extern void del_irc_command(char * command);
extern void call_command(struct buffer_info * buffer,
                         char * command,
                         char * params);
extern void print_command_syntax(struct buffer_info * buffer,
                                 char * command);
extern void print_command_help(struct buffer_info * buffer,
                               char * command);

#endif // __COMMANDS_H__
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
