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

trie * ctcp_request_types;
trie * ctcp_response_types;

void init_ctcp() {
    ctcp_request_types = trie_new(trie_strtoupper);
    ctcp_response_types = trie_new(trie_strtoupper);

    // Add builtin CTCP types
    add_ctcp_request("ACTION", ctcp_action_req_handler);
    add_ctcp_request("VERSION", ctcp_version_req_handler);
    add_ctcp_request("PING", ctcp_ping_req_handler);
    add_ctcp_response("PING", ctcp_ping_resp_handler);
    add_ctcp_response("VERSION", ctcp_version_resp_handler);
}

void add_ctcp_request(const char * type, ctcp_callback cb) {
    trie_set(ctcp_request_types, type, cb);
}

void add_ctcp_response(const char * type, ctcp_callback cb) {
    trie_set(ctcp_response_types, type, cb);
}

void process_ctcp(struct irc_network * network,
                  enum _ctcp_type ctcp_type,
                  char * hostmask,
                  char * target,
                  char * msg) {
    char * saveptr;
    char * type;
    ctcp_callback cb;
    trie * trie = (ctcp_type == REQUEST) ? ctcp_request_types
                                         : ctcp_response_types;

    // Extract the CTCP info from the message
    msg = strtok_r(msg, CTCP_DELIM_STR, &saveptr);
    type = strtok_r(msg, " ", &saveptr);
    msg = saveptr;

    // Check if we have a callback for this CTCP
    if ((cb = trie_get(trie, type)) != NULL)
        cb(network, hostmask, target, msg);
    else
        print_to_buffer(network->buffer,
                        "Unknown CTCP \"%s\" received from %s with target "
                        "%s: %s\n",
                        type, hostmask, target, msg);
}

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
