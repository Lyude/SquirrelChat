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

#define BI_CMD(func_name)                               \
    extern short func_name(struct buffer_info * buffer, \
                           unsigned short argc,         \
                           char * argv[],               \
                           char * trailing)             \
    __attribute__((nonnull(1)))

BI_CMD(cmd_help);
BI_CMD(cmd_nick);
BI_CMD(cmd_server);
BI_CMD(cmd_msg);
BI_CMD(cmd_join);
BI_CMD(cmd_part);
BI_CMD(cmd_connect);
BI_CMD(cmd_quit);
BI_CMD(cmd_quote);
BI_CMD(cmd_motd);
BI_CMD(cmd_topic);
BI_CMD(cmd_notice);
BI_CMD(cmd_mode);

#undef BI_CMD

#endif // __BUILTIN_COMMANDS_H__
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
