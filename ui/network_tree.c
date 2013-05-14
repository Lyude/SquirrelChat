/* Code for the nice network tree on the side of a chat window
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
                 struct network_buffer * buffer) {
    gtk_tree_store_append(window->network_tree_store,
                          &network_tree_toplevel, NULL);
    gtk_tree_store_set(window->network_tree_store, &network_tree_toplevel,
                       0, "untitled", 1, buffer, -1);
}

// Get's the currently selected network in the network tree
struct network_buffer * get_current_network(struct chat_window * window) {
    GtkTreeIter selected_row;
    struct network_buffer * network;
    gtk_tree_selection_get_selected(
            gtk_tree_view_get_selection(GTK_TREE_VIEW(window->network_tree)),
            &window->network_tree_store, &selected_row);
    gtk_tree_model_get(GTK_TREE_MODEL(window->network_tree_store),
                       &selected_row, 1, &network, -1);
    return network;
}

// Callbacks
void cursor_changed_handler(GtkTreeSelection *treeselection,
                            struct chat_window * window) {
    struct network_buffer * network;
    GtkTreeModel * network_list_model =
        gtk_tree_view_get_model(gtk_tree_selection_get_tree_view(treeselection));
    GtkTreeIter selected_row;
    
    gtk_tree_selection_get_selected(treeselection, &network_list_model, &selected_row);

    gtk_tree_model_get(network_list_model, &selected_row,
                       1, &network, -1);

    // Change the buffer the chat viewer is using to the one of the selected row
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(window->chat_viewer),
                             network->buffer);
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
