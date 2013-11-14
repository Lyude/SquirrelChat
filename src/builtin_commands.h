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

extern void sqchat_add_builtin_commands();

#define BI_CMD(func_name)                                   \
    extern short func_name(struct sqchat_buffer * buffer,   \
                           unsigned short argc,             \
                           char * argv[],                   \
                           char * trailing)                 \
    _nonnull(1)

BI_CMD(sqchat_cmd_help);
BI_CMD(sqchat_cmd_nick);
BI_CMD(sqchat_cmd_server);
BI_CMD(sqchat_cmd_msg);
BI_CMD(sqchat_cmd_join);
BI_CMD(sqchat_cmd_part);
BI_CMD(sqchat_cmd_connect);
BI_CMD(sqchat_cmd_quit);
BI_CMD(sqchat_cmd_quote);
BI_CMD(sqchat_cmd_motd);
BI_CMD(sqchat_cmd_topic);
BI_CMD(sqchat_cmd_notice);
BI_CMD(sqchat_cmd_mode);
BI_CMD(sqchat_cmd_ctcp);
BI_CMD(sqchat_cmd_me);
BI_CMD(sqchat_cmd_whois);
BI_CMD(sqchat_cmd_oper);
BI_CMD(sqchat_cmd_whowas);
BI_CMD(sqchat_cmd_lusers);
BI_CMD(sqchat_cmd_invite);
BI_CMD(sqchat_cmd_time);
BI_CMD(sqchat_cmd_version);
BI_CMD(sqchat_cmd_info);
BI_CMD(sqchat_cmd_away);
BI_CMD(sqchat_cmd_back);
BI_CMD(sqchat_cmd_who);
BI_CMD(sqchat_cmd_links);
BI_CMD(sqchat_cmd_list);
BI_CMD(sqchat_cmd_kick);
BI_CMD(sqchat_cmd_kill);
BI_CMD(sqchat_cmd_wallops);
BI_CMD(sqchat_cmd_trace);
BI_CMD(sqchat_cmd_username);
BI_CMD(sqchat_cmd_realname);

#undef BI_CMD

#endif // __BUILTIN_COMMANDS_H__
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4:cinoptions=(0,W4
