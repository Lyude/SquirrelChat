#ifndef CHAT_WINDOW_H
#define CHAT_WINDOW_H

#include <gtk/gtk.h>

/* Struct containing all the important widgets for a window, passed to most
 * controlling the widgets in each window
 */
struct chat_window {
    GtkWidget * window;
    GtkWidget * top_vertical_container;
    
    GtkWidget * main_menu_bar;
    GtkWidget * main_menu;
    GtkWidget * main_menu_item; 
    GtkWidget * connect_menu_item;
    GtkWidget * new_server_buffer_menu_item;
    GtkWidget * exit_menu_item;

    GtkWidget * network_tree_and_buffer_pane;

    GtkWidget * network_tree;
    GtkTreeStore * network_tree_store;
    GtkTreeIter network_tree_toplevel;

    GtkWidget * chat_viewer_and_user_list_pane;

    GtkWidget * command_and_chat_viewer_container;
    GtkWidget * scrolled_window_container_for_chat_viewer;
    GtkWidget * chat_viewer;
    GtkWidget * command_box;
};

struct chat_window * create_new_chat_window();

#endif // CHAT_WINDOW_H

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
