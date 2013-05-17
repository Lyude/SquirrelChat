/* Data structures for storing information and buffers for an IRC network, and
 * functions for handling said structures
 */
#ifndef __IRC_NETWORK_H__
#define __IRC_NETWORK_H__ 

#include "irc_macros.h"

#include <gtk/gtk.h>
#include <glib.h>
#include <netdb.h>

struct irc_network {
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

    struct buffer_info * buffer;
};

struct irc_network * new_irc_network();
void free_irc_network(struct irc_network * network,
                      GtkTreeStore * network_tree_store);

int connect_irc_network(struct irc_network * network);
void print_to_irc_network(struct irc_network * network,
                          char * message, ...);

void free_chat_buffer(void * this_does_nothing);

void network_tree_cursor_changed_handler(GtkTreeSelection *treeselection,
                                         GtkTextView *chat_viewer);
#endif /* BUFFERS_H */
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
