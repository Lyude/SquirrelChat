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

typedef struct irc_response_queue irc_response_queue;

struct irc_response_queue {
    struct buffer_info * buffer;
    short type;
    irc_response_queue * next;
};

extern void request_cmd_response(struct irc_network * network,
                                 struct buffer_info * buffer,
                                 short type);
extern irc_response_queue ** find_cmd_response_request(struct irc_network * network,
                                                       short type);
extern void remove_cmd_response_request(irc_response_queue ** response);

#endif // __CMD_RESPONSES_H__
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
