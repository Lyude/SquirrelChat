/* Code for the nice network tree on the side of a chat window
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

#include "network_tree.h"

#include <gtk/gtk.h>

#include "chat_window.h"

GtkTreeIter network_tree_toplevel;

// Sets up the network tree
void create_network_tree(struct chat_window * window) {
    GtkCellRenderer * network_tree_renderer;

    GtkTreeViewColumn *network_tree_title_column;
    GtkTreeViewColumn *network_tree_data_column;

    window->network_tree_store = gtk_tree_store_new(2, G_TYPE_STRING,
                                                    G_TYPE_POINTER);
    window->network_tree = gtk_tree_view_new_with_model(
            GTK_TREE_MODEL(window->network_tree_store));
    network_tree_title_column = gtk_tree_view_column_new();
    network_tree_data_column = gtk_tree_view_column_new();
    gtk_tree_view_append_column(GTK_TREE_VIEW(window->network_tree),
                                network_tree_title_column);
    gtk_tree_view_append_column(GTK_TREE_VIEW(window->network_tree),
                                network_tree_data_column);

    network_tree_renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(network_tree_title_column,
                                    network_tree_renderer, TRUE);
    gtk_tree_view_column_add_attribute(network_tree_title_column,
                                       network_tree_renderer, "text", 0);
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(window->network_tree),
                                      FALSE);
    gtk_tree_view_set_show_expanders(GTK_TREE_VIEW(window->network_tree),
                                     TRUE);
    gtk_tree_selection_set_mode(gtk_tree_view_get_selection(
                                GTK_TREE_VIEW(window->network_tree)),
                                GTK_SELECTION_SINGLE);
}

// Adds an empty network buffer to the network tree
void add_network(struct chat_window * window,
                 struct irc_network * network) {
    GtkTreePath * toplevel_path;
    gtk_tree_store_append(window->network_tree_store,
                          &network_tree_toplevel, NULL);
    gtk_tree_store_set(window->network_tree_store, &network_tree_toplevel,
                       0, "untitled", 1, network->buffer, -1);

    toplevel_path = 
        gtk_tree_model_get_path(GTK_TREE_MODEL(window->network_tree_store),
                                &network_tree_toplevel);

    network->row = gtk_tree_row_reference_new(
            GTK_TREE_MODEL(window->network_tree_store),
            toplevel_path);
    network->window = window;
    network->buffer->window = window;
}

// Get's the currently selected network in the network tree
struct irc_network * get_current_network(struct chat_window * window) {
    GtkTreeIter selected_row;
    struct irc_network * network;
    gtk_tree_selection_get_selected(
            gtk_tree_view_get_selection(GTK_TREE_VIEW(window->network_tree)),
            &window->network_tree_store, &selected_row);
    gtk_tree_model_get(GTK_TREE_MODEL(window->network_tree_store),
                       &selected_row, 1, &network, -1);
    return network;
}

void add_buffer_to_tree(struct buffer_info * buffer,
                        struct irc_network * network) {
    GtkTreeIter network_row;
    GtkTreeIter buffer_row;
    GtkTreeRowReference * buffer_ref;
    GtkTreeModel * tree_model = gtk_tree_row_reference_get_model(network->row);

    gtk_tree_model_get_iter(tree_model, &network_row,
                            gtk_tree_row_reference_get_path(network->row));

    // Append a new buffer
    gtk_tree_store_append(GTK_TREE_STORE(tree_model), &buffer_row,
                          &network_row);
    gtk_tree_store_set(GTK_TREE_STORE(tree_model), &buffer_row, 0,
                       buffer->buffer_name, 1, buffer, -1);

    // Store a reference in the network's trie and in the buffer
    buffer_ref = gtk_tree_row_reference_new(tree_model,
                                            gtk_tree_model_get_path(tree_model,
                                                                    &buffer_row)
                                           );
    buffer->row = buffer_ref;
}

void remove_buffer_from_tree(struct buffer_info * buffer) {
    GtkTreeModel * network_tree_model;
    GtkTreeIter buffer_row;
    network_tree_model = gtk_tree_row_reference_get_model(buffer->row);
    gtk_tree_model_get_iter(network_tree_model,
                            &buffer_row,
                            gtk_tree_row_reference_get_path(buffer->row));

    gtk_tree_store_remove(GTK_TREE_STORE(network_tree_model), &buffer_row);
}

// Callbacks
/* TODO: Modify this handler to also work with all types of buffers, not just
 * network buffers
 */
void cursor_changed_handler(GtkTreeSelection *treeselection,
                            struct chat_window * window) {
    struct buffer_info * buffer;
    GtkTreeModel * network_list_model =
        gtk_tree_view_get_model(gtk_tree_selection_get_tree_view(treeselection));
    GtkTreeIter selected_row;
    
    gtk_tree_selection_get_selected(treeselection, &network_list_model,
                                    &selected_row);

    gtk_tree_model_get(network_list_model, &selected_row,
                       1, &buffer, -1);
    
    change_active_buffer(window, buffer);
}

/* Connects the signals for the network tree
 * (done as a seperate function because we cannot connect all of the signals
 * until all of the widgets for the window are created
 */
void connect_network_tree_signals(struct chat_window * window) {
    g_signal_connect(gtk_tree_view_get_selection(GTK_TREE_VIEW(window->network_tree)),
                     "changed", G_CALLBACK(cursor_changed_handler), window);
}

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
