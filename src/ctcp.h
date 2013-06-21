/* Contains functions for processing CTCP requests
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
#ifndef __CTCP_H__
#define __CTCP_H__

#include "irc_network.h"

#define CTCP_DELIM '\001'
#define CTCP_DELIM_STR "\001"

typedef void (*ctcp_callback)(struct irc_network * network,
                              char *, // hostmask
                              char *, // target
                              char *); // message

extern void init_ctcp();

extern void add_ctcp_type(const char * type, ctcp_callback cb)
    _nonnull(1, 2);

extern void process_ctcp(struct irc_network * network,
                         char * hostmask,
                         char * target,
                         char * msg)
    _nonnull(1, 2, 3, 4);

#define send_ctcp(_network, _target, _type)                                 \
    send_to_network(_network,                                               \
                    "PRIVMSG %s :" CTCP_DELIM_STR "%s" CTCP_DELIM_STR       \
                    "\r\n", _target, _type)

#define sendf_ctcp(_network, _target, _type, _msg, ...)                     \
    send_to_network(_network,                                               \
                    "PRIVMSG %s :" CTCP_DELIM_STR "%s " _msg CTCP_DELIM_STR \
                    "\r\n", _target, _type, __VA_ARGS__)

#endif // __CTCP_H__
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
