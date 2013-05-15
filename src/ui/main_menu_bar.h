#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "chat_window.h"

#include <gtk/gtk.h>

void create_main_menu_bar(struct chat_window * window);
void connect_main_menu_bar_signals(struct chat_window * window);

#endif /* MAIN_MENU_H */

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
