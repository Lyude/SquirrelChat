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
    void func_name(struct irc_network * network,        \
                   char * hostmask,                     \
                   char * target,                       \
                   char * msg)                          \

CTCP_REQ_HANDLER(ctcp_version_req_handler) {
    char * address;
    char * nickname;
    split_irc_hostmask(hostmask, &nickname, &address);
    
    sendf_ctcp_reply(network, nickname, "VERSION", "%s", "SquirrelChat");
}

CTCP_REQ_HANDLER(ctcp_action_req_handler) {
    char * nickname;
    char * address;
    split_irc_hostmask(hostmask, &nickname, &address);

    struct buffer_info * output;
    if (IRC_IS_CHAN(network, target)) {
        // Check if we're in the channel
        if ((output = trie_get(network->buffers, target)) == NULL) {
            print_to_buffer(network->buffer,
                            "Error parsing CTCP: Received ACTION for %s from "
                            "%s, but we're not in %s.\n",
                            target, nickname, target);
            return;
        }

        print_to_buffer(output, "* %s %s\n", nickname, msg);
    }
    else {
        // Check if we have a query open with this user, if not open a new one
        if ((output = trie_get(network->buffers, target)) == NULL) {
            output = new_buffer(target, QUERY, network);
            add_buffer_to_tree(output, network);
        }
        print_to_buffer(output, "* %s %s\n", nickname, msg);
    }
}

CTCP_REQ_HANDLER(ctcp_ping_req_handler) {
    char * nickname;
    char * address;
    split_irc_hostmask(hostmask, &nickname, &address);

    if (msg == NULL)
        send_ctcp_reply(network, nickname, "PING");
    else
        sendf_ctcp_reply(network, nickname, "PING", "%s", msg);
}

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
