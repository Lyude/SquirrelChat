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
#include <string.h>
#include <stdlib.h>

static inline int get_user_row(const struct buffer_info * buffer,
                               const char * nickname,
                               GtkTreeIter * user_row) {
    GtkTreeRowReference * user_ref;

    if ((user_ref = trie_get(buffer->users, nickname)) == NULL)
        return -1;

    gtk_tree_model_get_iter(GTK_TREE_MODEL(buffer->user_list_store),
                            user_row,
                            gtk_tree_row_reference_get_path(user_ref));
    return 0;
}

static inline char * get_user_prefixes(const struct buffer_info * buffer,
                                       GtkTreeIter * user) {
    GValue value;
    char * prefixes;
    gtk_tree_model_get_value(GTK_TREE_MODEL(buffer->user_list_store),
                             user, 2, &value);
    prefixes = g_value_get_pointer(&value);
    g_value_unset(&value);
    return prefixes;
}

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

void add_user_to_list(struct buffer_info * buffer,
                      const char * nickname,
                      const char * prefix_str) {
    GtkTreeIter new_user_row;

    // Add the user to the user list in the channel
    gtk_list_store_append(buffer->user_list_store, &new_user_row);

    if (prefix_str != NULL) {
        char prefix_row_str[2];

        prefix_row_str[0] = prefix_str[0];
        prefix_row_str[1] = '\0';

        gtk_list_store_set(buffer->user_list_store, &new_user_row, 0,
                           &prefix_row_str[0], -1);
    }

    if (buffer->parent_network->multi_prefix)
        gtk_list_store_set(buffer->user_list_store, &new_user_row, 1,
                           nickname, 2, prefix_str ? strdup(prefix_str) : NULL,
                           -1);
    else
        gtk_list_store_set(buffer->user_list_store, &new_user_row, 1,
                           nickname, -1);

    trie_set(buffer->users, nickname,
             gtk_tree_row_reference_new(
                 GTK_TREE_MODEL(buffer->user_list_store),
                 gtk_tree_model_get_path(
                     GTK_TREE_MODEL(buffer->user_list_store), &new_user_row
                     )
                 )
             );
}

int remove_user_from_list(struct buffer_info * buffer,
                          const char * nickname) {
    GtkTreeRowReference * user_ref;
    GtkTreeIter user_row;

    if ((user_ref = trie_get(buffer->users, nickname)) == NULL)
        return -1;

    gtk_tree_model_get_iter(GTK_TREE_MODEL(buffer->user_list_store),
                            &user_row,
                            gtk_tree_row_reference_get_path(user_ref));

    gtk_tree_row_reference_free(user_ref);
    trie_del(buffer->users, nickname);
    gtk_list_store_remove(buffer->user_list_store, &user_row);
    return 0;
}

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
