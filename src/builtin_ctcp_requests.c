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

#include "ctcp.h"
#include "irc_network.h"
#include "builtin_ctcp_requests.h"
#include "ui/buffer.h"
#include "ui/network_tree.h"

#define CTCP_REQ_HANDLER(func_name)                     \
    void func_name(struct sqchat_network * network,     \
                   char * hostmask,                     \
                   char * target,                       \
                   char * msg)                          \

CTCP_REQ_HANDLER(sqchat_ctcp_version_req_handler) {
    char * address;
    char * nickname;
    sqchat_split_hostmask(hostmask, &nickname, &address);
    
    sqchat_network_sendf_ctcp_reply(network, nickname, "VERSION", "%s",
                                    "SquirrelChat");
}

CTCP_REQ_HANDLER(sqchat_ctcp_action_req_handler) {
    char * nickname;
    char * address;
    sqchat_split_hostmask(hostmask, &nickname, &address);

    struct sqchat_buffer * output;
    if (SQCHAT_IS_CHAN(network, target)) {
        // Check if we're in the channel
        if ((output = sqchat_trie_get(network->buffers, target)) == NULL) {
            sqchat_buffer_print(network->buffer,
                                "Error parsing CTCP: Received ACTION for %s "
                                "from %s, but we're not in %s.\n",
                                target, nickname, target);
            return;
        }

        sqchat_buffer_print(output, "* %s %s\n", nickname, msg);
    }
    else {
        // Check if we have a query open with this user, if not open a new one
        if ((output = sqchat_trie_get(network->buffers, target)) == NULL) {
            output = sqchat_buffer_new(target, QUERY, network);
            sqchat_network_tree_buffer_add(output, network);
        }
        sqchat_buffer_print(output, "* %s %s\n", nickname, msg);
    }
}

CTCP_REQ_HANDLER(sqchat_ctcp_ping_req_handler) {
    char * nickname;
    char * address;
    sqchat_split_hostmask(hostmask, &nickname, &address);

    if (msg == NULL)
        sqchat_network_send_ctcp_reply(network, nickname, "PING");
    else
        sqchat_network_sendf_ctcp_reply(network, nickname, "PING", "%s", msg);
}

// vim: set expandtab tw=80 shiftwidth=4 softtabstop=4 cinoptions=(0,W4:
