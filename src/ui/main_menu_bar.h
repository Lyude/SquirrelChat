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

#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "chat_window.h"

#include <gtk/gtk.h>

extern void create_main_menu_bar(struct chat_window * window);
extern void connect_main_menu_bar_signals(struct chat_window * window);

#endif /* MAIN_MENU_H */

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
