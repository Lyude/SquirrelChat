#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <errno.h>
#include <string.h>

#include "buffers.h"
#include "ui/network_panel.h"
#include "ui/main_menu_bar.h"
#include "ui/main_text_view.h"
#include "ui/main_text_input.h"

int main(int argc, char *argv[]) {
    // GTK stuff
    GtkWidget *main_window;
    GtkWidget *main_window_vertical;
    GtkWidget *network_and_buffer_pane;
    GtkWidget *chat_and_input_container;
    GtkWidget *chat_and_user_list_pane;

    //  pthread_t irc_thread;
   
    // Create the main window
    gtk_init(&argc, &argv);
    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), "TailChat");
    gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(main_window), 640, 480);

    // Setup the container for the main window
    main_window_vertical = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(main_window), main_window_vertical);
    
    // Setup the main menu bar and menus
    create_main_menu_bar();
    gtk_box_pack_start(GTK_BOX(main_window_vertical), main_menu_bar,
                       FALSE, FALSE, 0);

    // Setup the panes for the chat widgets
    network_and_buffer_pane = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(main_window_vertical), network_and_buffer_pane,
                       TRUE, TRUE, 0);

    // Setup the network tree panel
    create_network_panel();
    gtk_paned_add1(GTK_PANED(network_and_buffer_pane), network_tree);

    // Setup the pane for the chat and the userlist
    chat_and_user_list_pane = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_add2(GTK_PANED(network_and_buffer_pane), chat_and_user_list_pane);

    // Setup the chat and input container for the second pane
    chat_and_input_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_paned_add1(GTK_PANED(chat_and_user_list_pane), chat_and_input_container);

    // Setup the text view widget
    create_main_text_view();
    gtk_box_pack_start(GTK_BOX(chat_and_input_container),
                       main_text_view_scroll_container,
                       TRUE, TRUE, 0);

    // Setup the text input widget
    create_main_text_input();
    gtk_box_pack_start(GTK_BOX(chat_and_input_container), main_text_input,
                       FALSE, FALSE, 0);

    // Connect the signals
    connect_network_panel_signals();
    connect_main_menu_bar_signals();
    connect_main_text_input_signals();

    g_signal_connect(main_window, "destroy", G_CALLBACK(gtk_main_quit),
                     NULL);

    // Show the main window and start the GTK loop
    gtk_widget_show(main_window_vertical);
    gtk_widget_show_all(main_menu_bar);
    gtk_widget_show(network_and_buffer_pane);
    gtk_widget_show(network_tree);
    gtk_widget_show(chat_and_user_list_pane);
    gtk_widget_show_all(chat_and_input_container);
    gtk_widget_show(main_window);

    gtk_main();

    return 0;
}
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
