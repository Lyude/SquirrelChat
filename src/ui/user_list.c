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
#include "buffer.h"

#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>

int sqchat_user_list_user_row_find(const struct sqchat_buffer * buffer,
                                   const char * nickname,
                                   GtkTreeIter * user_row) {
    GtkTreeRowReference * user_ref;

    if ((user_ref = sqchat_trie_get(buffer->chan_data->users, nickname)) == NULL)
        return -1;

    gtk_tree_model_get_iter(GTK_TREE_MODEL(buffer->chan_data->user_list_store),
                            user_row,
                            gtk_tree_row_reference_get_path(user_ref));
    return 0;
}

char * sqchat_user_list_user_get_prefixes(const struct sqchat_buffer * buffer,
                                          GtkTreeIter * user) {
    GValue value = G_VALUE_INIT;
    char * prefixes;
    gtk_tree_model_get_value(GTK_TREE_MODEL(buffer->chan_data->user_list_store),
            user, 2, &value);
    prefixes = g_value_get_pointer(&value);
    g_value_unset(&value);
    return prefixes;
}

static inline void set_user_prefixes(struct sqchat_buffer * buffer,
                                     GtkTreeIter * user_row,
                                     const char * prefixes) {
    gtk_list_store_set(buffer->chan_data->user_list_store, user_row, 2,
                       prefixes, -1);
}

void sqchat_user_list_new(struct sqchat_chat_window * window) {
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
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(window->user_list), FALSE);

    gtk_tree_view_append_column(GTK_TREE_VIEW(window->user_list), prefix_column);
    gtk_tree_view_append_column(GTK_TREE_VIEW(window->user_list), name_column);
    gtk_tree_view_append_column(GTK_TREE_VIEW(window->user_list), data_column);
}

void sqchat_user_list_user_add(struct sqchat_buffer * buffer,
                               const char * nickname,
                               const char * prefix_str,
                               size_t prefix_len) {
    GtkTreeIter new_user_row;

    // Add the user to the user list in the channel
    gtk_list_store_append(buffer->chan_data->user_list_store, &new_user_row);

    if (prefix_str != NULL) {
        char prefix_row_str[2];

        prefix_row_str[0] = prefix_str[0];
        prefix_row_str[1] = '\0';

        gtk_list_store_set(buffer->chan_data->user_list_store, &new_user_row,
                           0, &prefix_row_str[0], -1);
    }

    if (buffer->network->multi_prefix)
        gtk_list_store_set(buffer->chan_data->user_list_store, &new_user_row,
                           1, nickname, 2,
                           prefix_str ? strndup(prefix_str, prefix_len) : NULL,
                           -1);
    else
        gtk_list_store_set(buffer->chan_data->user_list_store, &new_user_row,
                           1, nickname, -1);

    sqchat_trie_set(buffer->chan_data->users, nickname,
             gtk_tree_row_reference_new(
                 GTK_TREE_MODEL(buffer->chan_data->user_list_store),
                 gtk_tree_model_get_path(
                     GTK_TREE_MODEL(buffer->chan_data->user_list_store),
                     &new_user_row
                     )
                 )
             );
}

int sqchat_user_list_user_remove(struct sqchat_buffer * buffer,
                                 const char * nickname) {
    GtkTreeRowReference * user_ref;
    GtkTreeIter user_row;

    if ((user_ref = sqchat_trie_get(buffer->chan_data->users, nickname)) == NULL)
        return -1;

    gtk_tree_model_get_iter(GTK_TREE_MODEL(buffer->chan_data->user_list_store),
                            &user_row,
                            gtk_tree_row_reference_get_path(user_ref));

    gtk_tree_row_reference_free(user_ref);
    sqchat_trie_del(buffer->chan_data->users, nickname);
    free(sqchat_user_list_user_get_prefixes(buffer, &user_row));
    gtk_list_store_remove(buffer->chan_data->user_list_store, &user_row);
    return 0;
}

int _sqchat_user_list_user_set_visible_prefix_by_nick(struct sqchat_buffer * buffer,
                                              const char * nickname,
                                              char prefix) {
    GtkTreeIter user;

    if (sqchat_user_list_user_row_find(buffer, nickname, &user) == -1)
        return -1;
    sqchat_user_list_user_set_visible_prefix(buffer, &user, prefix);
    return 0;
}

void sqchat_user_list_user_set_visible_prefix(struct sqchat_buffer * buffer,
                                         GtkTreeIter * user,
                                         char prefix) {
    char new_prefix[2];
    new_prefix[0] = prefix;
    new_prefix[1] = '\0';
    gtk_list_store_set(buffer->chan_data->user_list_store, user, 0,
                       &new_prefix, -1);
}

int sqchat_user_list_user_prefix_add(struct sqchat_buffer * buffer,
                                     const char * nickname,
                                     const char * prefix) {
    GtkTreeIter user_row;
    char * current_prefixes;
    size_t current_prefixes_size;
    short insert_pos;

    if (sqchat_user_list_user_row_find(buffer, nickname, &user_row) == -1)
        return -1;

    current_prefixes = sqchat_user_list_user_get_prefixes(buffer, &user_row);
    if (current_prefixes == NULL) {
        current_prefixes = malloc(sizeof(char[2]));
        current_prefixes[0] = *prefix;
        current_prefixes[1] = '\0';
        set_user_prefixes(buffer, &user_row, current_prefixes);
        sqchat_user_list_user_set_visible_prefix(buffer, &user_row, *prefix);
        return 0;
    }

    current_prefixes_size = strlen(current_prefixes);

    // If the user already has the prefix applied, skip it
    if (strchr(current_prefixes, *prefix) != NULL)
        return 0;

    /* Figure out where to put the new prefix symbol (the list of prefixes a
     * user has is always sorted from greatest to least in terms of
     * privileges)
     */
    for (insert_pos = 0; current_prefixes[insert_pos] != '\0'; insert_pos++)
        if (prefix < strchr(buffer->network->prefix_symbols,
                            current_prefixes[insert_pos]))
            break;

    current_prefixes = realloc(current_prefixes, current_prefixes_size + 2);
    memmove(&current_prefixes[insert_pos + 1],
            &current_prefixes[insert_pos],
            current_prefixes_size - insert_pos + 1);
    current_prefixes[insert_pos] = *prefix;

    sqchat_user_list_user_set_visible_prefix(buffer, &user_row, current_prefixes[0]);

    gtk_list_store_set(buffer->chan_data->user_list_store, &user_row, 2,
                       current_prefixes, -1);
    return 0;
}

int sqchat_user_list_user_prefix_subtract(struct sqchat_buffer * buffer,
                                          const char * nickname,
                                          char prefix) {
    GtkTreeIter user_row;
    char * current_prefixes;
    size_t current_prefixes_size;
    char * pos;

    if (sqchat_user_list_user_row_find(buffer, nickname, &user_row) == -1)
        return -1;

    current_prefixes = sqchat_user_list_user_get_prefixes(buffer, &user_row);
    current_prefixes_size = strlen(current_prefixes) + 1;

    // Find the position of the prefix that needs to be removed
    if ((pos = strchr(current_prefixes, prefix)) == NULL)
        return 0;
    else if (current_prefixes_size == 2) { // The user only has one prefix
        free(current_prefixes);
        sqchat_user_list_user_set_visible_prefix(buffer, &user_row, '\0');
    }
    // Remove the prefix from the string
    memmove(pos, pos + 1,
            current_prefixes_size - (pos - current_prefixes));

    // Trim the string
    current_prefixes = realloc(current_prefixes, current_prefixes_size - 1);

    // Update the user's row
    set_user_prefixes(buffer, &user_row, current_prefixes);
    sqchat_user_list_user_set_visible_prefix(buffer,
                                             &user_row, current_prefixes[0]);
    gtk_list_store_set(buffer->chan_data->user_list_store, &user_row, 2,
                       current_prefixes, -1);
    return 0;
}

// vim: set expandtab tw=80 shiftwidth=4 softtabstop=4 cinoptions=(0,W4:
