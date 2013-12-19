/* Contains functions for handling command responses
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
#ifndef __CMD_RESPONSES_H__
#define __CMD_RESPONSES_H__

#include "commands.h"
#include "ui/buffer.h"

#include "irc_network.h"

typedef struct sqchat_cmd_response_claim sqchat_cmd_response_claim;

struct sqchat_cmd_response_claim {
    struct sqchat_buffer * buffer;
    void * data;
    void (*data_free_func)(void *);
    sqchat_cmd_response_claim * next;
};

extern void sqchat_claim_response(struct sqchat_network * network,
                                  struct sqchat_buffer * buffer,
                                  void * data,
                                  void (*data_free_func)(void *))
    _attr_nonnull(1, 2);
extern void sqchat_remove_last_response_claim(struct sqchat_network * network)
    _attr_nonnull(1);

extern struct sqchat_buffer * sqchat_route_rpl(const struct sqchat_network * network)
    _attr_nonnull(1);
extern struct sqchat_buffer * sqchat_route_rpl_end(struct sqchat_network * network)
    _attr_nonnull(1);

#endif // __CMD_RESPONSES_H__
// vim: set expandtab tw=80 shiftwidth=4 softtabstop=4 cinoptions=(0,W4:
