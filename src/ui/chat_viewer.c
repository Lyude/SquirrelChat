/* Creates a new chat viewer
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

#include "chat_viewer.h"
#include "chat_window.h"

#include <gtk/gtk.h>

void sqchat_chat_viewer_new(struct sqchat_chat_window * window) {
    window->chat_viewer = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(window->chat_viewer), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(window->chat_viewer), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(window->chat_viewer),
                                GTK_WRAP_WORD_CHAR);
}
// vim: set expandtab tw=80 shiftwidth=4 softtabstop=4 cinoptions=(0,W4:
