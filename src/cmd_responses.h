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

typedef struct cmd_response_claim cmd_response_claim;

struct cmd_response_claim {
    struct buffer_info * buffer;
    void * data;
    void (*data_free_func)(void *);
    cmd_response_claim * next;
};

extern void claim_response(struct irc_network * network,
                           struct buffer_info * buffer,
                           void * data,
                           void (*data_free_func)(void *))
    _nonnull(1, 2);
extern void remove_last_response_claim(struct irc_network * network)
    _nonnull(1);

extern struct buffer_info * route_rpl(const struct irc_network * network)
    _nonnull(1);
extern struct buffer_info * route_rpl_end(struct irc_network * network)
    _nonnull(1);

#endif // __CMD_RESPONSES_H__
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
