#include "chat_viewer.h"
#include "chat_window.h"

#include <gtk/gtk.h>

void sqchat_chat_viewer_new(struct sqchat_chat_window * window) {
    window->chat_viewer = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(window->chat_viewer), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(window->chat_viewer), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(window->chat_viewer),
                                GTK_WRAP_WORD_CHAR);
}
