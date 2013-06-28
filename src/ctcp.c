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
#include "builtin_ctcp.h"
#include "ui/buffer.h"
#include "net_io.h"

#include <string.h>

trie * ctcp_types;

void init_ctcp() {
    ctcp_types = trie_new(trie_strtoupper);

    // Add builtin CTCP types
    add_ctcp_type("ACTION", ctcp_cb_action);
    add_ctcp_type("VERSION", ctcp_cb_version);
    add_ctcp_type("PING", ctcp_cb_ping);
}

void add_ctcp_type(const char * type, ctcp_callback cb) {
    trie_set(ctcp_types, type, cb);
}

void process_ctcp(struct irc_network * network,
                  char * hostmask,
                  char * target,
                  char * msg) {
    char * saveptr;
    char * type;
    ctcp_callback cb;

    // Extract the CTCP info from the message
    msg = strtok_r(msg, CTCP_DELIM_STR, &saveptr);
    type = strtok_r(msg, " ", &saveptr);
    msg = saveptr;

    // Check if we have a callback for this CTCP
    if ((cb = trie_get(ctcp_types, type)) != NULL)
        cb(network, hostmask, target, msg);
    else
        print_to_buffer(network->buffer,
                        "Unknown CTCP \"%s\" received from %s with target "
                        "%s: %s\n",
                        type, hostmask, target, msg);
}

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
