#include "buffers.h"
#include "net_io.h"
#include "net_input_handler.h"

#include <gtk/gtk.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <fcntl.h>

struct network_buffer ** socket_table;

void _init_vars_buffers_c() {
    socket_table = malloc(sysconf(_SC_OPEN_MAX));
}

/* Creates a new network buffer and adds it to the network tree
 * NOTE: The struct is automatically sanitized by this function, so there is no
 * need to do it yourself
 */
struct network_buffer * new_network_buffer() {
    // Allocate and sanitize the structure
    struct network_buffer * network = malloc(sizeof(struct network_buffer));
    memset(network, 0, sizeof(struct network_buffer));

    network->buffer = gtk_text_buffer_new(NULL);
    network->chat_buffers = g_hash_table_new_full(g_str_hash, NULL, NULL,
                                                  free_chat_buffer);

    // Set the buffer as disconnected
    network->connection_status = DISCONNECTED;

    //XXX: Added just for testing purposes
    network->address = "rainbowdash.ponychat.net";
    network->port = "6667";
    network->nickname = "LyudeTEST";
    network->username = "LyudeTEST";
    network->real_name = "LyudeTEST";

    return network;
}

/* TODO: Switch the tab when freeing a network buffer to ensure all references
 * to the text buffer are removed
 */
void free_network_buffer(struct network_buffer * buffer,
                         GtkTreeStore * network_tree_store) {
//  g_hash_table_destroy(buffer->chat_buffers);
    g_object_unref(GTK_TEXT_BUFFER(buffer->buffer));
    // TODO: Add in code to free all of the strings in the structure
    free(buffer);
}

// Connects an IRC buffer
int connect_network_buffer(struct network_buffer * buffer) {
    struct addrinfo hints; // For specifying the type of host we want
    struct addrinfo * results; // Stores the result list from getaddrinfo
    int func_result;

    buffer->connection_status = CONNECTING;
    print_to_network_buffer(buffer, "Attempting to connect to %s:%s...\n",
                            buffer->address, buffer->port);

    // Try to get the addrinfo for the server
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    func_result = getaddrinfo(buffer->address, buffer->port, &hints, &results);
    if (func_result != 0) {
        print_to_network_buffer(buffer, "CONNECTION ERROR: %s\n",
                                gai_strerror(func_result));
        buffer->connection_status = DISCONNECTED;
        return -1;
    }

    // Try to connect to each address structure returned by getaddrinfo
    /* FIXME: Instead of returning function, retry connection attempt and print
     * messages
     */
    struct addrinfo * rp;
    for (rp = results; rp != NULL; rp = rp->ai_next) {
        // Try to create a socket
        buffer->socket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (buffer->socket == -1)
            continue;

        // Set the socket as non-blocking
        fcntl(buffer->socket, F_SETFD, O_NONBLOCK);

        // Try to connect to the created socket
        if (connect(buffer->socket, rp->ai_addr, rp->ai_addrlen) != -1)
            break;

        // The connection failed, so close the socket
        close(buffer->socket);
    }

    // Check if we successfully connected, if not return an error
    if (rp == NULL) {
        print_to_network_buffer(buffer, "CONNECTION ERROR: %s\n",
                                strerror(errno));
        buffer->connection_status = DISCONNECTED;
        freeaddrinfo(results);
        return -1;
    }

    print_to_network_buffer(buffer, "Connected! Sending info...\n");
    send_to_network(buffer, "NICK %s\r\n", buffer->nickname);
    send_to_network(buffer, "USER %s X X :%s\r\n",
                    buffer->username, buffer->real_name);

    buffer->connection_status = CONNECTED;
    buffer->input_channel = g_io_channel_unix_new(buffer->socket);
    g_io_channel_set_encoding(buffer->input_channel, NULL, NULL);
    g_io_channel_set_buffered(buffer->input_channel, FALSE);

    g_io_add_watch_full(buffer->input_channel, G_PRIORITY_DEFAULT, G_IO_IN,
                        (GIOFunc)net_input_handler, buffer, NULL);

    freeaddrinfo(results);
    return 0;
}

// Prints a plain text message to a text buffer
// TODO (maybe): Add a non printf version of this, potential security issue
void print_to_network_buffer(struct network_buffer * buffer,
                             char * message, ...) {
    va_list args;
    char * parsed_message;
    gchar * parsed_message_utf8;
    size_t parsed_message_len;
    size_t parsed_message_utf8_len;
    GtkTextIter end_of_buffer;

    // Parse the message passed to this function
    va_start(args, message);
    parsed_message_len = vsnprintf(NULL, 0, message, args);

    va_start(args, message);
    parsed_message = alloca(parsed_message_len);
    vsnprintf(parsed_message, parsed_message_len + 1, message, args);
    va_end(args);

    /* FIXME (maybe): String is already supposed to be in utf8, but glib doesn't
     * think so, so we convert it to glib's liking
     */
    parsed_message_utf8 = g_locale_to_utf8(parsed_message,
                                           parsed_message_len,
                                           &parsed_message_utf8_len,
                                           NULL, NULL);
    
    // Figure out where the end of the buffer is
    gtk_text_buffer_get_end_iter(buffer->buffer, &end_of_buffer);

    /* TODO: Add in code to maintain the line limit for the buffer whenever
     * anything is printed to the buffer
     */

    // Print the message
    gtk_text_buffer_insert(buffer->buffer, &end_of_buffer, parsed_message_utf8,
                           parsed_message_utf8_len);
}

void free_chat_buffer(void * this_does_nothing) {
    // TODO: Add stuff
}

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
