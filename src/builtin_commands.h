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
#ifndef __BUILTIN_COMMANDS_H__
#define __BUILTIN_COMMANDS_H__

#include "ui/buffer.h"

extern short cmd_help(struct buffer_info * buffer,
                      unsigned short argc,
                      char * argv[],
                      char * trailing);
extern short cmd_nick(struct buffer_info * buffer,
                      unsigned short argc,
                      char * argv[],
                      char * trailing);
extern short cmd_server(struct buffer_info * buffer,
                        unsigned short argc,
                        char * argv[],
                        char * trailing);
extern short cmd_msg(struct buffer_info * buffer,
                     unsigned short argc,
                     char * argv[],
                     char * trailing);
extern short cmd_join(struct buffer_info * buffer,
                      unsigned short argc,
                      char * argv[],
                      char * trailing);
extern short cmd_part(struct buffer_info * buffer,
                      unsigned short argc,
                      char * argv[],
                      char * trailing);
extern short cmd_connect(struct buffer_info * buffer,
                         unsigned short argc,
                         char * argv[],
                         char * trailing);
extern short cmd_quit(struct buffer_info * buffer,
                      unsigned short argc,
                      char * argv[],
                      char * trailing);
extern short cmd_quote(struct buffer_info * buffer,
                       unsigned short argc,
                       char * argv[],
                       char * trailing);
extern short cmd_motd(struct buffer_info * buffer,
                      unsigned short argc,
                      char * argv[],
                      char * trailing);
extern short cmd_topic(struct buffer_info * buffer,
                       unsigned short argc,
                       char * argv[],
                       char * trailing);
extern short cmd_notice(struct buffer_info * buffer,
                        unsigned short argc,
                        char * argv[],
                        char * trailing);
extern short cmd_mode(struct buffer_info * buffer,
                      unsigned short argc,
                      char * argv[],
                      char * trailing);

#endif // __BUILTIN_COMMANDS_H__
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
