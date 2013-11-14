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
#include "net_io.h"
#include "message_parser.h"

#define SQCHAT_CTCP_DELIM '\001'
#define SQCHAT_CTCP_DELIM_STR "\001"

typedef void (*sqchat_ctcp_callback)(struct sqchat_network * network,
                                     char *, // hostmask
                                     char *, // target
                                     char *); // message

extern void sqchat_ctcp_init();

extern void sqchat_add_ctcp_request(const char * type, sqchat_ctcp_callback cb)
    _nonnull(1, 2);

extern void sqchat_add_ctcp_response(const char * type, sqchat_ctcp_callback cb)
    _nonnull(1, 2);

enum _ctcp_type {
    REQUEST,
    RESPONSE
};

extern void sqchat_process_ctcp(struct sqchat_network * network,
                                enum _ctcp_type type,
                                char * hostmask,
                                char * target,
                                char * msg)
    _nonnull(1, 3, 4, 5);

#define sqchat_network_send_ctcp(_network, _target, _type)                  \
    sqchat_network_send(_network,                                           \
                        "PRIVMSG %s :" SQCHAT_CTCP_DELIM_STR "%s"           \
                        SQCHAT_CTCP_DELIM_STR                               \
                        "\r\n", _target, _type)

#define sqchat_network_sendf_ctcp(_network, _target, _type, _msg, ...)      \
    sqchat_network_send(_network,                                           \
                        "PRIVMSG %s :" SQCHAT_CTCP_DELIM_STR "%s " _msg     \
                        SQCHAT_CTCP_DELIM_STR                               \
                        "\r\n", _target, _type, __VA_ARGS__)

#define sqchat_network_send_ctcp_reply(_network, _target, _type)            \
    sqchat_network_send(_network,                                           \
                        "NOTICE %s :" SQCHAT_CTCP_DELIM_STR "%s "           \
                        SQCHAT_CTCP_DELIM_STR                               \
                        "\r\n", _target, _type)

#define sqchat_network_sendf_ctcp_reply(_network, _target, _type, _msg, ...)\
    sqchat_network_send(_network,                                           \
                        "NOTICE %s :" SQCHAT_CTCP_DELIM_STR "%s " _msg      \
                        SQCHAT_CTCP_DELIM_STR                               \
                        "\r\n", _target, _type, __VA_ARGS__)

#endif // __CTCP_H__
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4:cinoptions=(0,W4
