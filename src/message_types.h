/* Contains the callbacks for all of the stock non-numeric message types
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
#ifndef __MESSAGE_TYPES_H__
#define __MESSAGE_TYPES_H__
#include "irc_network.h"

extern void init_message_types();

#define MSG_CB(func_name)                               \
    extern short func_name(struct irc_network * network, \
                           char * hostmask,              \
                           short argc,                   \
                           char * argv[])                \
    _nonnull(1)

MSG_CB(cap_msg_callback);
MSG_CB(join_msg_callback);
MSG_CB(part_msg_callback);
MSG_CB(privmsg_msg_callback);
MSG_CB(notice_msg_callback);
MSG_CB(ping_msg_callback);
MSG_CB(nick_msg_callback);
MSG_CB(topic_msg_callback);
MSG_CB(mode_msg_callback);
MSG_CB(quit_msg_callback);
MSG_CB(kick_msg_callback);
MSG_CB(invite_msg_callback);
MSG_CB(error_msg_callback);
MSG_CB(wallops_msg_callback);

#undef MSG_CB

#endif // __MESSAGE_TYPES_H__

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
