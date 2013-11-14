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
#ifndef __MESSAGE_PARSER_H__
#define __MESSAGE_PARSER_H__
#include "irc_network.h"

typedef short (*sqchat_msg_cb)(struct sqchat_network *,
                               char *,     // hostmask
                               short,      // argc
                               char*[]);   // argv

#define SQCHAT_MSG_ERR_ARGS        1
#define SQCHAT_MSG_ERR_ARGS_FATAL  2
#define SQCHAT_MSG_ERR_MISC        3
#define SQCHAT_MSG_ERR_MISC_NODUMP 4

extern void sqchat_init_msg_parser();

extern void sqchat_process_msg(struct sqchat_network * network, char * msg)
    _nonnull(1, 2);
void sqchat_split_hostmask(char * hostmask, char ** nickname, char ** address)
    _nonnull(1, 2, 3);

#endif
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4:cinoptions=(0,W4
