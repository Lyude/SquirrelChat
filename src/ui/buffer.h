#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "../irc_network.h"
#include "chat_window.h"

#include <gtk/gtk.h>

/* TODO: Make a typeless buffer object, and make children buffer objects for
 * all other types of buffers
 */
enum buffer_type {
    NETWORK,
    CHANNEL,
    QUERY
};

/* Contains all the widgets for a buffer, some of these are not always used
 * depending on the type of buffer
 */
struct buffer_info {
    enum buffer_type type;
    struct irc_network * parent_network;
    GtkWidget * chat_viewer;
    GtkTextBuffer * buffer;
    GtkWidget * scrolled_window_for_chat_viewer;
    GtkWidget * command_box;
    GtkWidget * chat_and_command_box_container;

    // Only used for channel buffers
    GtkWidget * user_list;
    GtkListStore * user_list_store;
    GtkWidget * chat_viewer_and_user_list_pane;
};

struct buffer_info * new_buffer(enum buffer_type type,
                                struct irc_network * network);
void destroy_buffer(struct buffer_info * buffer);

void print_to_buffer(struct buffer_info * buffer,
                     char * message, ...);

#endif /* __BUFFER_H__ */    
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4