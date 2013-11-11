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

#ifndef __NETWORK_TREE_H__
#define __NETWORK_TREE_H__
#include <gtk/gtk.h>

#include "chat_window.h"

#include "buffer.h"

extern void sqchat_network_tree_new(struct sqchat_chat_window * window)
    _nonnull(1);
extern void sqchat_network_tree_connect_signals(struct sqchat_chat_window * window)
    _nonnull(1);

extern struct sqchat_network * sqchat_network_tree_get_current(struct sqchat_chat_window * window)
    _nonnull(1);

extern void sqchat_network_tree_network_add(struct sqchat_chat_window * window,
                                            struct sqchat_network * network)
    _nonnull(1, 2);
extern void sqchat_network_tree_network_remove(struct sqchat_network * network);

extern void sqchat_network_tree_buffer_add(struct sqchat_buffer * buffer,
                                           struct sqchat_network * network)
    _nonnull(1, 2);
extern void sqchat_network_tree_buffer_remove(struct sqchat_buffer * buffer)
    _nonnull(1);

#endif /* __NETWORK_TREE_H__ */

// vim: set expandtab tw=80 shiftwidth=4 softtabstop=4 cinoptions=(0,W4:
