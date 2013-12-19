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

#ifndef __MAIN_MENU_H__
#define __MAIN_MENU_H__

#include "chat_window.h"

#include <gtk/gtk.h>

extern void sqchat_main_menu_bar_setup(struct sqchat_chat_window * window)
    _attr_nonnull(1);
extern void sqchat_main_menu_bar_connect_signals(struct sqchat_chat_window * window)
    _attr_nonnull(1);

#endif /* __MAIN_MENU_H__ */

// vim: set expandtab tw=80 shiftwidth=4 softtabstop=4 cinoptions=(0,W4:
