/* Data structures for storing information and buffers for an IRC network, and
 * functions for handling said structures
 */
#ifndef BUFFERS_H
#define BUFFERS_H 

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
    struct sockaddr ai_addr;
    socklen_t ai_addrlen;
    
    enum {
        DISCONNECTED,
        CONNECTING,
        CONNECTED
    } connection_status;
    pthread_t connection_thread;
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

extern struct network_buffer ** socket_table;

void free_chat_buffer(void * this_does_nothing);

void network_tree_cursor_changed_handler(GtkTreeSelection *treeselection,
                                         GtkTextView *chat_viewer);
#endif /* BUFFERS_H */
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
