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
#include "builtin_ctcp.h"
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

#define BUILTIN_CTCP(func_name)                 \
    void func_name(struct irc_network * network,\
                   char * hostmask,             \
                   char * target,               \
                   char * msg)

BUILTIN_CTCP(ctcp_cb_action) {
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

BUILTIN_CTCP(ctcp_cb_version) {
    char * nickname;
    char * address;
    split_irc_hostmask(hostmask, &nickname, &address);

    if (*msg == '\0')
        sendf_ctcp(network, nickname, "VERSION", "%s", "SquirrelChat");
    else {
        print_to_buffer(network->window->current_buffer,
                        "[%s VERSION] %s\n", nickname, msg);
    }
}

BUILTIN_CTCP(ctcp_cb_ping) {
    char * nickname;
    char * address;
    split_irc_hostmask(hostmask, &nickname, &address);
    struct timespec current_time;
    long response_time;

    clock_gettime(CLOCK_REALTIME, &current_time);

    errno = 0;
    response_time = strtol(msg, NULL, 10);

    if (errno == 0)
            print_to_buffer(network->window->current_buffer,
                            "* Received PING from %s, response time: %.2lfms.\n",
                            nickname,
                            ((current_time.tv_sec * 1000000000 +
                              current_time.tv_nsec) - response_time) * 1.0e-6);
    else
        sendf_ctcp(network, nickname, "PING", "%s", msg);
}
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
