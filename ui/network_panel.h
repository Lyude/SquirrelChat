#ifndef NETWORK_PANEL_H
#define NETWORK_PANEL_H
#include <gtk/gtk.h>

#include "../buffers.h"

extern GtkWidget * network_tree;
extern GtkTreeStore * network_tree_store;

void create_network_panel();
void connect_network_panel_signals();

struct network_buffer * get_current_network();

void add_network(struct network_buffer * network);

#endif /* NETWORK_PANEL_H */

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
