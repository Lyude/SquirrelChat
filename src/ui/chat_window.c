/* Functions for creating and managing chat windows
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

#include "chat_window.h"

#include "buffer.h"
#include "main_menu_bar.h"
#include "network_tree.h"
#include "command_box.h"
#include "chat_viewer.h"
#include "user_list.h"

#include <stdlib.h>
#include <gtk/gtk.h>

struct chat_window * create_new_chat_window(struct irc_network * network) {
    // Containers that are only ever referenced here
    GtkWidget * chat_and_command_box_container;

    struct chat_window * new_window = malloc(sizeof(struct chat_window));

    new_window->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(new_window->window), "SquirrelChat");
    gtk_window_set_position(GTK_WINDOW(new_window->window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(new_window->window), 640, 480);

    new_window->top_vertical_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(new_window->window),
                      new_window->top_vertical_container);
    
    create_main_menu_bar(new_window);
    gtk_box_pack_start(GTK_BOX(new_window->top_vertical_container),
                       new_window->main_menu_bar, FALSE, FALSE, 0);

    new_window->network_tree_and_buffer_pane =
        gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(new_window->top_vertical_container),
                       new_window->network_tree_and_buffer_pane,
                       TRUE, TRUE, 0);

    create_network_tree(new_window);
    gtk_paned_add1(GTK_PANED(new_window->network_tree_and_buffer_pane),
                   new_window->network_tree);

    new_window->chat_viewer_and_user_list_pane
        = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_add2(GTK_PANED(new_window->network_tree_and_buffer_pane),
                   new_window->chat_viewer_and_user_list_pane);

    chat_and_command_box_container =gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    create_chat_viewer(new_window);
    
    new_window->scrolled_window_for_chat_viewer =
        gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_kinetic_scrolling(
            GTK_SCROLLED_WINDOW(new_window->scrolled_window_for_chat_viewer),
            TRUE);
    gtk_container_add(GTK_CONTAINER(new_window->scrolled_window_for_chat_viewer),
                      new_window->chat_viewer);

    create_command_box(new_window);

    gtk_box_pack_start(GTK_BOX(chat_and_command_box_container),
                       new_window->scrolled_window_for_chat_viewer,
                       TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(chat_and_command_box_container),
                       new_window->command_box, FALSE, FALSE, 0);
    
    gtk_paned_add1(GTK_PANED(new_window->chat_viewer_and_user_list_pane),
                   chat_and_command_box_container);

    create_user_list(new_window);
    new_window->scrolled_window_for_user_list =
        gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_kinetic_scrolling(
            GTK_SCROLLED_WINDOW(new_window->scrolled_window_for_user_list),
            TRUE);
    gtk_container_add(GTK_CONTAINER(new_window->scrolled_window_for_user_list),
                      new_window->user_list);
    gtk_paned_add2(GTK_PANED(new_window->chat_viewer_and_user_list_pane),
                   new_window->scrolled_window_for_user_list);

    /* Create a new network to act as a place holder if a network wasn't
     * provided
     */
    if (network == NULL) {
        struct irc_network * placeholder = new_irc_network();
        add_network(new_window, placeholder);
        change_active_buffer(new_window, placeholder->buffer);
    }

    // Connect the signals
    connect_network_tree_signals(new_window);
    connect_main_menu_bar_signals(new_window);
    connect_command_box_signals(new_window);

    // TODO: Destroy chat_window struct when windows are destroyed
    g_signal_connect(new_window->window, "destroy", G_CALLBACK(gtk_main_quit),
                     NULL);

    // Show the main window 
    gtk_widget_show_all(new_window->window);

    return new_window;
}

void change_active_buffer(struct chat_window * window,
                          struct buffer_info * new_buffer) {
    // Record the scroll position of the current buffer
//    window->current_buffer->buffer_scroll_pos = gtk_adjustment_get_value(
//        gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(
//            window->scrolled_window_for_chat_viewer)));

    gtk_entry_set_buffer(GTK_ENTRY(window->command_box), new_buffer->command_box_buffer);
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(window->chat_viewer), new_buffer->buffer);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(window->chat_viewer),
                                 gtk_text_buffer_get_mark(new_buffer->buffer,
                                                          "insert"),
                                 0.0, false, 0.0, 0.0);
//    gtk_adjustment_set_value(gtk_scrolled_window_get_vadjustment(
//        GTK_SCROLLED_WINDOW(window->scrolled_window_for_chat_viewer)),
//            new_buffer->buffer_scroll_pos);

    if (new_buffer->type == CHANNEL) {
        gtk_tree_view_set_model(GTK_TREE_VIEW(window->user_list),
                                GTK_TREE_MODEL(new_buffer->user_list_store));
        gtk_widget_show(window->scrolled_window_for_user_list);
    }
    else
        gtk_widget_hide(window->scrolled_window_for_user_list);

    window->current_buffer = new_buffer;
}
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
