/* Functions for setting up the main menu bar in a chat window
 */
#include <gtk/gtk.h>
#include "chat_window.h"
#include "../buffers.h"
#include "network_tree.h"
#include "main_menu_bar.h"

void create_main_menu_bar(struct chat_window * window) {
    window->main_menu_bar = gtk_menu_bar_new();
    window->main_menu = gtk_menu_new();

    window->main_menu_item = gtk_menu_item_new_with_label("SquirrelChat");
    window->connect_menu_item = gtk_menu_item_new_with_label("Connect");
    window->new_server_buffer_menu_item =
        gtk_menu_item_new_with_label("New server");
    window->exit_menu_item = gtk_menu_item_new_with_label("Exit");

    // Build the menu structure
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(window->main_menu_item),
                              window->main_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(window->main_menu),
                          window->connect_menu_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(window->main_menu),
                          window->new_server_buffer_menu_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(window->main_menu),
                          window->exit_menu_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(window->main_menu_bar),
                          window->main_menu_item);
}

void new_server_buffer_menu_item_callback(GtkMenuItem * menuitem,
                                          struct chat_window * window) {
    add_network(window, new_network_buffer());
}

// Callback used by the "Connect" menu item
void connect_current_network(GtkMenuItem * menuitem,
                             struct chat_window * window) {
    GtkTreeIter selected_row;
    struct network_buffer * buffer;

    // Find which row is selected
    gtk_tree_selection_get_selected(gtk_tree_view_get_selection(
                                    GTK_TREE_VIEW(window->network_tree)),
                                    &window->network_tree_store,
                                    &selected_row);

    // Get the network buffer for the selected row
    gtk_tree_model_get(GTK_TREE_MODEL(window->network_tree_store),
                       &selected_row, 1, &buffer, -1);

    connect_network_buffer(buffer); 
}

// Connects all the signals for the items in the menu bar
void connect_main_menu_bar_signals(struct chat_window * window) {
    g_signal_connect(window->connect_menu_item, "activate",
                     G_CALLBACK(connect_current_network), window);
    g_signal_connect(window->new_server_buffer_menu_item, "activate",
                     G_CALLBACK(new_server_buffer_menu_item_callback), window);
    g_signal_connect(window->exit_menu_item, "activate", G_CALLBACK(gtk_main_quit),
                     NULL);
}

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
