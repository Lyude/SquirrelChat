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
#include "../net_io.h"
#include "../cmd_responses.h"

GtkTreeIter network_tree_toplevel;

// Sets up the network tree
void sqchat_network_tree_setup(struct sqchat_chat_window * window) {
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
    gtk_tree_view_set_enable_search(GTK_TREE_VIEW(window->network_tree),
                                    FALSE); // Temporary fix for segfault
}

static void buffer_close_item_callback(GtkMenuItem * menuitem, struct sqchat_buffer * buffer);

static void show_right_click_menu(GtkWidget * widget,
                                  GdkEventButton * event,
                                  struct sqchat_buffer * buffer) {
    int button;
    int event_time;
    GtkWidget * menu = gtk_menu_new();
    GtkWidget * close_item = gtk_menu_item_new_with_label("Close");

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), close_item);

    if (event != NULL) {
        button = event->button;
        event_time = event->time;
    }
    else {
        button = 0;
        event_time = gtk_get_current_event_time();
    }

    g_signal_connect(close_item, "activate",
                 G_CALLBACK(buffer_close_item_callback), buffer);

    gtk_widget_show_all(menu);

    gtk_menu_attach_to_widget(GTK_MENU(menu), widget, NULL);
    gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, button, event_time);
}

static void buffer_close_item_callback(GtkMenuItem * menuitem,
                                       struct sqchat_buffer * buffer) {
    if (buffer->type == NETWORK) {
        if (buffer->network->status != DISCONNECTED) {
            GtkWidget * dialog;

            dialog =
                gtk_message_dialog_new(GTK_WINDOW(buffer->window->window),
                                       GTK_DIALOG_MODAL |
                                           GTK_DIALOG_DESTROY_WITH_PARENT,
                                       GTK_MESSAGE_WARNING,
                                       GTK_BUTTONS_YES_NO,
                                       "This network is still connected, are "
                                       "you sure you would like to close it?");
            if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_YES) {
                sqchat_network_tree_network_remove(buffer->network);
                sqchat_network_destroy(buffer->network);
            }
            gtk_widget_destroy(dialog);
        }
        else {
            sqchat_network_tree_network_remove(buffer->network);
            sqchat_network_destroy(buffer->network);
        }
    }
    else if (buffer->type == CHANNEL) {
        if (buffer->network->status == CONNECTED) {
            sqchat_network_send(buffer->network,
                                "PART %s\r\n",
                                buffer->buffer_name);
        }
        else {
            sqchat_network_tree_buffer_remove(buffer);
            sqchat_buffer_destroy(buffer);
        }
    }
    else {
        sqchat_network_tree_buffer_remove(buffer);
        sqchat_buffer_destroy(buffer);
    }
}

// Adds an empty network buffer to the network tree
void sqchat_network_tree_network_add(struct sqchat_chat_window * window,
                                     struct sqchat_network * network) {
    GtkTreePath * toplevel_path;
    gtk_tree_store_append(window->network_tree_store,
                          &network_tree_toplevel, NULL);
    gtk_tree_store_set(window->network_tree_store, &network_tree_toplevel,
                       0, "untitled", 1, network->buffer, -1);

    toplevel_path =
        gtk_tree_model_get_path(GTK_TREE_MODEL(window->network_tree_store),
                                &network_tree_toplevel);

    network->buffer->row = gtk_tree_row_reference_new(
            GTK_TREE_MODEL(window->network_tree_store),
            toplevel_path);
    network->window = window;
    network->buffer->window = window;
}

void sqchat_network_tree_network_remove(struct sqchat_network * network) {
    /* Check to make sure there are other buffers we can switch to
     * so that we can ensure that all references to the buffer are
     * destroyed. If there isn't, just create a generic untitled
     * buffer and switch to that.
     */
    if (gtk_tree_model_iter_n_children(
                GTK_TREE_MODEL(network->window->network_tree_store), NULL) == 1) {
        struct sqchat_network * placeholder = sqchat_network_new();

        sqchat_network_tree_network_add(network->window, placeholder);
        sqchat_chat_window_change_active_buffer(network->window,
                                                placeholder->buffer);
    }

    if (network->status != DISCONNECTED)
        sqchat_network_disconnect(network, NULL);

    // Close all of the buffers belonging to the network
    sqchat_trie_each(network->buffers, sqchat_network_tree_buffer_remove, NULL);
    sqchat_network_tree_buffer_remove(network->buffer);
}

// Get's the currently selected network in the network tree
struct sqchat_network * sqchat_network_tree_get_current(struct sqchat_chat_window * window) {
    GtkTreeIter selected_row;
    struct sqchat_network * network;
    gtk_tree_selection_get_selected(
            gtk_tree_view_get_selection(GTK_TREE_VIEW(window->network_tree)),
            &window->network_tree_store, &selected_row);
    gtk_tree_model_get(GTK_TREE_MODEL(window->network_tree_store),
                       &selected_row, 1, &network, -1);
    return network;
}

void sqchat_network_tree_buffer_add(struct sqchat_buffer * buffer,
                                    struct sqchat_network * network) {
    GtkTreeIter network_row;
    GtkTreeIter buffer_row;
    GtkTreeRowReference * buffer_ref;
    GtkTreeModel * tree_model =
        gtk_tree_row_reference_get_model(network->buffer->row);

    gtk_tree_model_get_iter(tree_model, &network_row,
                            gtk_tree_row_reference_get_path(
                                network->buffer->row
                            ));

    // Append a new buffer
    gtk_tree_store_append(GTK_TREE_STORE(tree_model), &buffer_row,
                          &network_row);
    gtk_tree_store_set(GTK_TREE_STORE(tree_model), &buffer_row, 0,
                       buffer->buffer_name, 1, buffer, -1);

    // Store a reference in the network's sqchat_trie and in the buffer
    buffer_ref = gtk_tree_row_reference_new(tree_model,
                                            gtk_tree_model_get_path(tree_model,
                                                                    &buffer_row)
                                           );
    buffer->row = buffer_ref;
    buffer->window = network->window;
}

void sqchat_network_tree_buffer_remove(struct sqchat_buffer * buffer) {
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
                            struct sqchat_chat_window * window) {
    struct sqchat_buffer * buffer;
    GtkTreeModel * network_list_model =
        gtk_tree_view_get_model(gtk_tree_selection_get_tree_view(treeselection));
    GtkTreeIter selected_row;

    gtk_tree_selection_get_selected(treeselection, &network_list_model,
                                    &selected_row);

    gtk_tree_model_get(network_list_model, &selected_row,
                       1, &buffer, -1);

    // If the active buffer hasn't been changed already, change it
    if (window->current_buffer != buffer)
        sqchat_chat_window_change_active_buffer(window, buffer);
}

static bool button_event_handler(GtkWidget * widget,
                                 GdkEventButton * event,
                                 struct sqchat_chat_window * window) {
    if (gdk_event_triggers_context_menu((GdkEvent *)event) &&
        event->type == GDK_BUTTON_PRESS) {
        GtkTreePath * path;
        GtkTreeIter row;
        struct sqchat_buffer * buffer;

        // Figure out what row the cursor is over, if any
        if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(window->network_tree),
                                          event->x, event->y, &path, NULL, NULL,
                                          NULL) == false)
            return true;
        gtk_tree_model_get_iter(GTK_TREE_MODEL(window->network_tree_store),
                                &row, path);
        gtk_tree_path_free(path);

        // Extract the buffer pointer from the row
        gtk_tree_model_get(GTK_TREE_MODEL(window->network_tree_store), &row,
                           1, &buffer, -1);

        show_right_click_menu(widget, event, buffer);
        return true;
    }

    return false;
}

static bool popup_menu_handler(GtkWidget * widget,
                               struct sqchat_chat_window * window) {
    GtkTreePath * path;
    GtkTreeIter row;
    struct sqchat_buffer * buffer;

    // Figure out what row is selected and get it's buffer pointer
    gtk_tree_view_get_cursor(GTK_TREE_VIEW(window->network_tree), &path, NULL);
    gtk_tree_model_get_iter(GTK_TREE_MODEL(window->network_tree_store), &row,
                            path);
    gtk_tree_path_free(path);
    gtk_tree_model_get(GTK_TREE_MODEL(window->network_tree_store), &row,
                       1, &buffer, -1);

    show_right_click_menu(widget, NULL, buffer);
    return true;
}



/* Connects the signals for the network tree
 * (done as a seperate function because we cannot connect all of the signals
 * until all of the widgets for the window are created
 */
void sqchat_network_tree_connect_signals(struct sqchat_chat_window * window) {
    g_signal_connect(gtk_tree_view_get_selection(GTK_TREE_VIEW(window->network_tree)),
                     "changed", G_CALLBACK(cursor_changed_handler), window);
    g_signal_connect(window->network_tree, "button-press-event",
                     G_CALLBACK(button_event_handler), window);
    g_signal_connect(window->network_tree, "popup-menu",
                     G_CALLBACK(popup_menu_handler), window);
}

// vim: set expandtab tw=80 shiftwidth=4 softtabstop=4 cinoptions=(0,W4:
