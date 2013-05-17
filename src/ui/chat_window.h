#ifndef CHAT_WINDOW_H
#define CHAT_WINDOW_H

#include "../irc_network.h"
#include "buffer.h"

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

    GtkWidget * buffer_pane;

    struct buffer_info * current_buffer;
};

// TODO: Remove the "create_" part in the function name
struct chat_window * create_new_chat_window(struct irc_network * network);

void change_active_buffer(struct chat_window * window,
                          struct buffer_info * new_buffer);

#endif // CHAT_WINDOW_H

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
