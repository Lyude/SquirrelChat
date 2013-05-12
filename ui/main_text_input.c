// Code for creating and managing the main text entry widget

#include "main_text_input.h"
#include "main_text_view.h"
#include "network_panel.h"
#include "../buffers.h"

#include <gtk/gtk.h>

GtkWidget * main_text_input;

void create_main_text_input() {
    main_text_input = gtk_entry_new();
}

/* Callback for when the main text input is activated (the user hits enter while
 * the text input has focus)
 */
void main_text_input_activate_callback() {
    print_to_network_buffer(get_current_network(), "%s\n",
                            gtk_entry_get_text(GTK_ENTRY(main_text_input)));
}

void connect_main_text_input_signals() {
    g_signal_connect(main_text_input, "activate",
                     G_CALLBACK(main_text_input_activate_callback), NULL);
}

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
