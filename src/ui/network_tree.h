#ifndef NETWORK_TREE_H
#define NETWORK_TREE_H
#include <gtk/gtk.h>

#include "chat_window.h"

#include "buffer.h"

void create_network_tree(struct chat_window * window);
void connect_network_tree_signals(struct chat_window * window);

struct irc_network * get_current_network(struct chat_window * window);

void add_network(struct chat_window * window,
                 struct irc_network * network);

#endif /* NETWORK_TREE_H */

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
