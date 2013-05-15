#ifndef COMMAND_BOX_H
#define COMMAND_BOX_H

#include <gtk/gtk.h>

void create_command_box(struct chat_window * window);
void connect_command_box_signals(struct chat_window * window);

#endif /* COMMAND_BOX_H */
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
