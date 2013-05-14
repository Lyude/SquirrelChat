/* Code for setting up and managing the main text view widget
 */
#include "chat_viewer.h"
#include "chat_window.h"

#include <gtk/gtk.h>

/* Sets up the main text view and the scrolling container for it
 */
void create_chat_viewer(struct chat_window * window) {
    window->chat_viewer = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(window->chat_viewer), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(window->chat_viewer), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(window->chat_viewer),
                                GTK_WRAP_WORD_CHAR);
}

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
