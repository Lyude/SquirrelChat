/* Data structures for storing information and buffers for an IRC network, and
 * functions for handling said structures
 */
#ifndef BUFFERS_H
#define BUFFERS_H 

#include "irc_macros.h"

#include <gtk/gtk.h>
#include <glib.h>
#include <netdb.h>

struct network_buffer {
    // Network information
    char * name;
    char * address;
    char * port;
    // TODO: Add flags here

    char * nickname;
    char * username;
    char * real_name;

    int socket;
    
    enum {
        DISCONNECTED,
        CONNECTING,
        CONNECTED
    } connection_status;

    char recv_buffer[IRC_MSG_BUF_LEN];
    int buffer_cursor;
    size_t buffer_fill_len;
    GIOChannel * input_channel;

    GtkTextBuffer * buffer;
    GHashTable * chat_buffers;
};

struct network_buffer * new_network_buffer();
void free_network_buffer(struct network_buffer * buffer,
                         GtkTreeStore * network_tree_store);

int connect_network_buffer(struct network_buffer * buffer);
void print_to_network_buffer(struct network_buffer * buffer,
                             char * message, ...);

void free_chat_buffer(void * this_does_nothing);

void network_tree_cursor_changed_handler(GtkTreeSelection *treeselection,
                                         GtkTextView *chat_viewer);
#endif /* BUFFERS_H */
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
