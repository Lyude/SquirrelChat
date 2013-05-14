/* Functions for creating and managing chat windows
 */

#include "chat_window.h"

#include "chat_viewer.h"
#include "main_menu_bar.h"
#include "command_box.h"
#include "network_tree.h"

#include <stdlib.h>
#include <gtk/gtk.h>

struct chat_window * create_new_chat_window() {
    struct chat_window * new_window = malloc(sizeof(struct chat_window));

    new_window->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(new_window->window), "TailChat");
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

    new_window->chat_viewer_and_user_list_pane =
        gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_add2(GTK_PANED(new_window->network_tree_and_buffer_pane),
                   new_window->chat_viewer_and_user_list_pane);

    new_window->command_and_chat_viewer_container =
        gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_paned_add1(GTK_PANED(new_window->chat_viewer_and_user_list_pane),
                   new_window->command_and_chat_viewer_container);

    create_chat_viewer(new_window);
    new_window->scrolled_window_container_for_chat_viewer =
        gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_kinetic_scrolling(GTK_SCROLLED_WINDOW(new_window->scrolled_window_container_for_chat_viewer), TRUE);
    gtk_container_add(GTK_CONTAINER(new_window->scrolled_window_container_for_chat_viewer),
                      new_window->chat_viewer);

    gtk_box_pack_start(GTK_BOX(new_window->command_and_chat_viewer_container),
                       new_window->scrolled_window_container_for_chat_viewer,
                       TRUE, TRUE, 0);

    create_command_box(new_window);
    gtk_box_pack_start(GTK_BOX(new_window->command_and_chat_viewer_container),
                       new_window->command_box, FALSE, FALSE, 0);

    // Connect the signals
    connect_network_tree_signals(new_window);
    connect_main_menu_bar_signals(new_window);
    connect_command_box_signals(new_window);

    // TODO: Destroy chat_window struct when windows are destroyed
    g_signal_connect(new_window->window, "destroy", G_CALLBACK(gtk_main_quit),
                     NULL);

    // Show the main window 
    gtk_widget_show(new_window->top_vertical_container);
    gtk_widget_show_all(new_window->main_menu_bar);
    gtk_widget_show(new_window->network_tree_and_buffer_pane);
    gtk_widget_show(new_window->network_tree);
    gtk_widget_show(new_window->chat_viewer_and_user_list_pane);
    gtk_widget_show_all(new_window->command_and_chat_viewer_container);
    gtk_widget_show(new_window->window);

    return new_window;
}

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
