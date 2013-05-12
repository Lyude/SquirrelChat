/* Programming for the main menu */
#include <pthread.h>
#include <gtk/gtk.h>
#include "../buffers.h"
#include "network_panel.h"
#include "main_menu_bar.h"

GtkWidget * main_menu_bar;
GtkWidget * main_menu;

// Items for howlIRC menu
GtkWidget * main_menu_item;
GtkWidget * connect_menu_item;
GtkWidget * new_server_buffer_menu_item;
GtkWidget * exit_menu_item;

void create_main_menu_bar() {
    main_menu_bar = gtk_menu_bar_new();
    main_menu = gtk_menu_new();

    main_menu_item = gtk_menu_item_new_with_label("TailChat");
    connect_menu_item = gtk_menu_item_new_with_label("Connect");
    new_server_buffer_menu_item = gtk_menu_item_new_with_label("New server");
    exit_menu_item = gtk_menu_item_new_with_label("Exit");

    // Build the menu structure
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(main_menu_item), main_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(main_menu), connect_menu_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(main_menu), new_server_buffer_menu_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(main_menu), exit_menu_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(main_menu_bar), main_menu_item);
}

void new_server_buffer_menu_item_callback() {
    add_network(new_network_buffer());
}

/* Callback used by the "Connect" menu item. Begins a new thread (so that the
 * connection attempt does not lock up the whole client) to attempt to connect
 * the currently selected network in the network tree
 */
void connect_current_network() {
    GtkTreeIter selected_row;
    struct network_buffer * buffer;

    // Find which row is selected
    gtk_tree_selection_get_selected(gtk_tree_view_get_selection(
                                        GTK_TREE_VIEW(network_tree)),
                                    &network_tree_store,
                                    &selected_row);

    // Get the network buffer for the selected row
    gtk_tree_model_get(GTK_TREE_MODEL(network_tree_store), &selected_row, 1,
                       &buffer, -1);

    // Begin the new thread
    connect_network_buffer(buffer); 
}

// Connects all the signals for the items in the menu bar
void connect_main_menu_bar_signals() {
    g_signal_connect(connect_menu_item, "activate",
                     G_CALLBACK(connect_current_network), NULL);
    g_signal_connect(new_server_buffer_menu_item, "activate",
                     G_CALLBACK(new_server_buffer_menu_item_callback), NULL);
    g_signal_connect(exit_menu_item, "activate", G_CALLBACK(gtk_main_quit),
                     NULL);
}

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
