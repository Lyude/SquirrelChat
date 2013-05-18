/* Functions for creating and managing chat windows
 */

#include "chat_window.h"

#include "buffer.h"
#include "main_menu_bar.h"
#include "network_tree.h"

#include <stdlib.h>
#include <gtk/gtk.h>

struct chat_window * create_new_chat_window(struct irc_network * network) {
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

    new_window->buffer_pane = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_add2(GTK_PANED(new_window->network_tree_and_buffer_pane),
                   new_window->buffer_pane);

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

    // TODO: Destroy chat_window struct when windows are destroyed
    g_signal_connect(new_window->window, "destroy", G_CALLBACK(gtk_main_quit),
                     NULL);

    // Show the main window 
    gtk_widget_show_all(new_window->window);

    return new_window;
}

void change_active_buffer(struct chat_window * window,
                          struct buffer_info * new_buffer) {
    if (new_buffer->type == CHANNEL)
        gtk_paned_add2(GTK_PANED(window->buffer_pane),
                       new_buffer->chat_viewer_and_user_list_pane);
    else
        gtk_paned_add2(GTK_PANED(window->buffer_pane),
                       new_buffer->chat_and_command_box_container);
    gtk_widget_show_all(window->window);
    window->current_buffer = new_buffer;
}
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
