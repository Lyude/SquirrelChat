/* Copyright (C) 2013 Stephen Chandler Paul
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

#ifndef NETWORK_TREE_H
#define NETWORK_TREE_H
#include <gtk/gtk.h>

#include "chat_window.h"

#include "buffer.h"

extern void create_network_tree(struct chat_window * window)
    _nonnull(1);
extern void connect_network_tree_signals(struct chat_window * window)
    _nonnull(1);

extern struct irc_network * get_current_network(struct chat_window * window)
    _nonnull(1);

extern void add_network(struct chat_window * window,
                        struct irc_network * network)
    _nonnull(1, 2);

#endif /* NETWORK_TREE_H */

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
