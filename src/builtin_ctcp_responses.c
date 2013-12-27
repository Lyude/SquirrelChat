/* Callbacks for the built-in CTCP types
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

#include "ctcp.h"
#include "builtin_ctcp_responses.h"
#include "irc_network.h"
#include "cmd_responses.h"
#include "message_parser.h"
#include "net_io.h"
#include "ui/buffer.h"
#include "ui/network_tree.h"

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#define BUILTIN_CTCP_RESP(func_name)                    \
    void func_name(struct sqchat_network * network,     \
                   char * hostmask,                     \
                   char * target,                       \
                   char * msg)

BUILTIN_CTCP_RESP(sqchat_ctcp_version_resp_handler) {
    char * nickname;
    char * address;
    sqchat_split_hostmask(hostmask, &nickname, &address);

    sqchat_buffer_print(network->window->current_buffer,
                        "[%s VERSION] %s\n", nickname, msg);
}

BUILTIN_CTCP_RESP(sqchat_ctcp_ping_resp_handler) {
    char * nickname;
    char * address;
    sqchat_split_hostmask(hostmask, &nickname, &address);
    long response_time;

    errno = 0;
    response_time = strtol(msg, NULL, 10);

    if (errno == 0)
        sqchat_buffer_print(network->window->current_buffer,
                            "* Received PING from %s, response time: %.2lfms.\n",
                            nickname,
                            (g_get_monotonic_time() - response_time) * 1.0e-3);
}
// vim: set expandtab tw=80 shiftwidth=4 softtabstop=4 cinoptions=(0,W4:
