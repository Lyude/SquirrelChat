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
    gtk_tree_selection_set_mode(gtk_tree_view_get_selection(
                                GTK_TREE_VIEW(window->network_tree)),
                                GTK_SELECTION_SINGLE);
}

// Adds an empty network buffer to the network tree
void add_network(struct chat_window * window,
                 struct irc_network * network) {
    gtk_tree_store_append(window->network_tree_store,
                          &network_tree_toplevel, NULL);
    gtk_tree_store_set(window->network_tree_store, &network_tree_toplevel,
                       0, "untitled", 1, network, -1);
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

// Callbacks
/* TODO: Modify this handler to also work with all types of buffers, not just
 * network buffers
 */
void cursor_changed_handler(GtkTreeSelection *treeselection,
                            struct chat_window * window) {
    struct irc_network * network;
    GtkTreeModel * network_list_model =
        gtk_tree_view_get_model(gtk_tree_selection_get_tree_view(treeselection));
    GtkTreeIter selected_row;
    
    gtk_tree_selection_get_selected(treeselection, &network_list_model,
                                    &selected_row);

    gtk_tree_model_get(network_list_model, &selected_row,
                       1, &network, -1);
    
    change_active_buffer(window, network->buffer);
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
