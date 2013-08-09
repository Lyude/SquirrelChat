/* Callbacks for handling CTCP requests
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
#ifndef __BUILTIN_CTCP_REQUESTS_H__
#define __BUILTIN_CTCP_REQUESTS_H__

#include "irc_network.h"

#define CTCP_REQ_HANDLER(func_name)                     \
    extern void func_name(struct irc_network * network, \
                          char * hostmask,              \
                          char * target,                \
                          char * msg)                   \
    _nonnull(1, 2)

CTCP_REQ_HANDLER(ctcp_version_req_handler);
CTCP_REQ_HANDLER(ctcp_action_req_handler);
CTCP_REQ_HANDLER(ctcp_ping_req_handler);

#undef CTCP_REQ_HANDLER
#endif
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
