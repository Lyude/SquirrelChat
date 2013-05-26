/*
 * Copyright (C) 2013 Stephen Chandler Paul
 *
 * This file is free software: you may copy it, redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 2 of this License or (at your option) any
 * later version.
 *
 * This file is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "buffer.h"
#include "../irc_network.h"
#include "chat_window.h"
#include "../commands.h"

#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>

struct buffer_info * new_buffer(char * buffer_name,
                                enum buffer_type type,
                                struct irc_network * network) {
    struct buffer_info * buffer = malloc(sizeof(struct buffer_info));
    buffer->type = type;
    buffer->buffer_name = (type != NETWORK) ? strdup(buffer_name) : NULL;
    buffer->row = NULL;
    buffer->parent_network = network;
    buffer->buffer_scroll_pos = 0;
    buffer->buffer = gtk_text_buffer_new(NULL);
    buffer->command_box_buffer = gtk_entry_buffer_new(NULL, -1);

    // Add a userlist if the buffer is a channel buffer
    if (type == CHANNEL) {
        buffer->user_list_store =
            gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
    }
    return buffer;
}

void destroy_buffer(struct buffer_info * buffer) {
    if (buffer->type == CHANNEL)
        g_object_unref(buffer->user_list_store);
    g_object_unref(buffer->buffer);
    g_object_unref(buffer->command_box_buffer);

    free(buffer->buffer_name);
    gtk_tree_row_reference_free(buffer->row);
    free(buffer);
}

void print_to_buffer(struct buffer_info * buffer,
                     char * message, ...) {
    va_list args;
    char * parsed_message;
    size_t parsed_message_len;
    GtkTextIter end_of_buffer;

    // Parse the message passed to this function
    va_start(args, message);
    parsed_message_len = vsnprintf(NULL, 0, message, args);

    va_start(args, message);
    parsed_message = alloca(parsed_message_len);
    vsnprintf(parsed_message, parsed_message_len + 1, message, args);
    va_end(args);
    
    // Figure out where the end of the buffer is
    gtk_text_buffer_get_end_iter(buffer->buffer, &end_of_buffer);

    /* TODO: Add in code to maintain the line limit for the buffer whenever
     * anything is printed to the buffer
     */

    // Print the message
    gtk_text_buffer_insert(buffer->buffer, &end_of_buffer, parsed_message,
                           parsed_message_len);
}

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
