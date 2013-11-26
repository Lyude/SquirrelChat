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

#include "trie.h"
#include "builtin_ctcp_requests.h"
#include "builtin_ctcp_responses.h"
#include "ui/buffer.h"
#include "net_io.h"

#include <string.h>

sqchat_trie * ctcp_request_types;
sqchat_trie * ctcp_response_types;

void sqchat_ctcp_init() {
    ctcp_request_types = sqchat_trie_new(sqchat_trie_strtoupper);
    ctcp_response_types = sqchat_trie_new(sqchat_trie_strtoupper);

    // Add builtin CTCP types
    sqchat_add_ctcp_request("ACTION", sqchat_ctcp_action_req_handler);
    sqchat_add_ctcp_request("VERSION", sqchat_ctcp_version_req_handler);
    sqchat_add_ctcp_request("PING", sqchat_ctcp_ping_req_handler);
    sqchat_add_ctcp_response("PING", sqchat_ctcp_ping_resp_handler);
    sqchat_add_ctcp_response("VERSION", sqchat_ctcp_version_resp_handler);
}

void sqchat_add_ctcp_request(const char * type, sqchat_ctcp_callback cb) {
    sqchat_trie_set(ctcp_request_types, type, cb);
}

void sqchat_add_ctcp_response(const char * type, sqchat_ctcp_callback cb) {
    sqchat_trie_set(ctcp_response_types, type, cb);
}

void sqchat_process_ctcp(struct sqchat_network * network,
                         enum _ctcp_type ctcp_type,
                         char * hostmask,
                         char * target,
                         char * msg) {
    char * saveptr;
    char * type;
    sqchat_ctcp_callback cb;
    sqchat_trie * sqchat_trie = (ctcp_type == REQUEST) ? ctcp_request_types
                                         : ctcp_response_types;

    // Extract the CTCP info from the message
    msg = strtok_r(msg, SQCHAT_CTCP_DELIM_STR, &saveptr);
    type = strtok_r(msg, " ", &saveptr);
    msg = saveptr;

    // Check if we have a callback for this CTCP
    if ((cb = sqchat_trie_get(sqchat_trie, type)) != NULL)
        cb(network, hostmask, target, msg);
    else
        sqchat_buffer_print(network->buffer,
                            "Unknown CTCP \"%s\" received from %s with target "
                            "%s: %s\n",
                            type, hostmask, target, msg);
}

// vim: set expandtab tw=80 shiftwidth=4 softtabstop=4 cinoptions=(0,W4:
