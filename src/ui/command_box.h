#ifndef __COMMAND_BOX_H__
#define __COMMAND_BOX_H__

#include "chat_window.h"

void create_command_box(struct chat_window * window);
void connect_command_box_signals(struct chat_window * window);

#endif // __COMMAND_BOX_H__
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
