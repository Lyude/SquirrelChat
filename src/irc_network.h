/* Data structures for storing information and buffers for an IRC network, and
 * functions for handling said structures
 *
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

#ifndef __IRC_NETWORK_H__
#define __IRC_NETWORK_H__ 

#include "irc_macros.h"

#include <gtk/gtk.h>
#include <glib.h>
#include <netdb.h>
#include <stdbool.h>

struct irc_network {
    // Network information
    char * name;
    char * address;
    char * port;
    // TODO: Add flags here

    char * nickname;
    char * username;
    char * real_name;

    int socket;
    
    bool connected;

    char recv_buffer[IRC_MSG_BUF_LEN];
    int buffer_cursor;
    size_t buffer_fill_len;
    GIOChannel * input_channel;

    struct buffer_info * buffer;
};

struct irc_network * new_irc_network();
void free_irc_network(struct irc_network * network,
                      GtkTreeStore * network_tree_store);

int connect_irc_network(struct irc_network * network);
void print_to_irc_network(struct irc_network * network,
                          char * message, ...);

void free_chat_buffer(void * this_does_nothing);

void network_tree_cursor_changed_handler(GtkTreeSelection *treeselection,
                                         GtkTextView *chat_viewer);
#endif /* BUFFERS_H */
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
