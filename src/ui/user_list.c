/*
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

#include "user_list.h"
#include "chat_window.h"

#include <gtk/gtk.h>

void create_user_list(struct chat_window * window) {
    GtkCellRenderer * renderer = gtk_cell_renderer_text_new();

    GtkTreeViewColumn * prefix_column = 
        gtk_tree_view_column_new_with_attributes("User prefix", renderer,
                                                 "text", 0, NULL);
    GtkTreeViewColumn * name_column =
        gtk_tree_view_column_new_with_attributes("Name", renderer,
                                                 "text", 1, NULL);
    GtkTreeViewColumn * data_column = gtk_tree_view_column_new();

    gtk_tree_view_column_set_sizing(prefix_column,
                                    GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_column_set_expand(name_column, true);

    window->user_list = gtk_tree_view_new();
    gtk_tree_view_set_headers_visible(window->user_list, FALSE);

    gtk_tree_view_append_column(GTK_TREE_VIEW(window->user_list), prefix_column);
    gtk_tree_view_append_column(GTK_TREE_VIEW(window->user_list), name_column);
    gtk_tree_view_append_column(GTK_TREE_VIEW(window->user_list), data_column);
}

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
