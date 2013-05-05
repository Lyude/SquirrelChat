/* Code for setting up and managing the main text view widget
 */
#include "main_text_view.h"

#include <gtk/gtk.h>

GtkWidget * main_text_view_scroll_container;
GtkWidget * main_text_view;

/* Sets up the main text view and the scrolling container for it
 */
void create_main_text_view() {
    main_text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(main_text_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(main_text_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(main_text_view), GTK_WRAP_WORD_CHAR);

    main_text_view_scroll_container = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_kinetic_scrolling(
            GTK_SCROLLED_WINDOW(main_text_view_scroll_container), TRUE);
    gtk_container_add(GTK_CONTAINER(main_text_view_scroll_container),
                                    main_text_view);
}

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
