// Code for creating and managing the main text entry widget

#include "chat_window.h"
#include "network_tree.h"
#include "../buffers.h"

#include <gtk/gtk.h>

void create_command_box(struct chat_window * window) {
    window->command_box = gtk_entry_new();
}

/* Callback for when the main text input is activated (the user hits enter while
 * the text input has focus)
 */
void command_box_activate_callback(struct chat_window *window) {
    print_to_network_buffer(get_current_network(window), "%s\n",
                            gtk_entry_get_text(GTK_ENTRY(window->command_box)));
}

void connect_command_box_signals(struct chat_window * window) {
    g_signal_connect(window->command_box, "activate",
                     G_CALLBACK(command_box_activate_callback), window);
}

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
