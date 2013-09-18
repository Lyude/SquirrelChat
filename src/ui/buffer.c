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
#include "user_list.h"

#include <pthread.h>
#include <gtk/gtk.h>
#include <iconv.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

static gboolean flush_buffer_output(struct buffer_info * buffer);

struct buffer_info * new_buffer(const char * buffer_name,
                                enum buffer_type type,
                                struct irc_network * network) {
    struct buffer_info * buffer = malloc(sizeof(struct buffer_info));
    buffer->type = type;
    buffer->buffer_name = (type != NETWORK) ? strdup(buffer_name) : NULL;
    buffer->row = NULL;
    buffer->network = network;
    buffer->window = network->window;
    buffer->buffer_scroll_pos = 0;
    buffer->buffer = gtk_text_buffer_new(NULL);
    buffer->command_box_buffer = gtk_entry_buffer_new(NULL, -1);
    buffer->out_queue_size = 0;
    buffer->out_queue = NULL;
    /* TODO: Add an option to change the encoding used per channel, or
     * per-network
     */
    buffer->locale_conv = iconv_open("UTF-8", "ISO_8859-1");
    pthread_mutex_init(&buffer->output_mutex, NULL);

    // Add a userlist if the buffer is a channel buffer
    if (type == CHANNEL) {
        buffer->chan_data = malloc(sizeof(struct __channel_data));
        buffer->chan_data->user_list_store =
            gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
        buffer->chan_data->users = trie_new(trie_strtolower);
    }
    else if (type == QUERY) {
        buffer->query_data = malloc(sizeof(struct __query_data));
        buffer->query_data->away_msg = NULL;
    }

    if (type != NETWORK)
        trie_set(network->buffers, buffer_name, buffer);

    return buffer;
}

static void destroy_users(GtkTreeRowReference * user,
                          struct buffer_info * buffer) {
    if (buffer->network->multi_prefix) {
        GtkTreeIter user_row;
        gtk_tree_model_get_iter(GTK_TREE_MODEL(buffer->chan_data->user_list_store),
                                &user_row,
                                gtk_tree_row_reference_get_path(user));
        free(get_user_prefixes(buffer, &user_row));
    }
    iconv_close(buffer->locale_conv);
    gtk_tree_row_reference_free(user);
}

/* NOTE: This should only ever be called from the main thread, behavior when
 * called in other threads is undefined. In addition, checking to see if any
 * other threads are accessing the buffer is too much of a pain, so please make
 * sure any other threads that have a chance of using the print_to_buffer()
 * function sometime during their life time have been terminated before calling
 * destroy_buffer()
 */
void destroy_buffer(struct buffer_info * buffer) {
    if (buffer->type == CHANNEL) {
        g_object_unref(buffer->chan_data->user_list_store);
        trie_free(buffer->chan_data->users, destroy_users, buffer);
    }
    g_object_unref(buffer->buffer);
    g_object_unref(buffer->command_box_buffer);

    if (buffer->type != NETWORK)
        trie_del(buffer->network->buffers, buffer->buffer_name);

    free(buffer->buffer_name);
    gtk_tree_row_reference_free(buffer->row);
    free(buffer->extra_data);

    pthread_mutex_destroy(&buffer->output_mutex);

    // If there was still data waiting to be outputted, destroy it
    if (g_idle_remove_by_data(buffer)) {
        struct __queued_output * n;
        for (struct __queued_output * c = buffer->out_queue;
             c != NULL;
             c = n) {
            free(c->msg);
            n = c->next;
            free(c);
        }
    }

    free(buffer);
}

void print_to_buffer(struct buffer_info * buffer,
                     const char * msg, ...) {
    va_list args;
    size_t parsed_msg_len;
    struct __queued_output * parsed_msg = malloc(sizeof(struct __queued_output));

    parsed_msg->next = NULL;

    // Create the string
    va_start(args, msg);
    parsed_msg_len = vsnprintf(NULL, 0, msg, args);

    va_start(args, msg);
    parsed_msg->msg = malloc((parsed_msg_len + 1));
    vsprintf(parsed_msg->msg, msg, args);
    va_end(args);

    // Add the message to the end of the queue and update the end pointer
    pthread_mutex_lock(&buffer->output_mutex);
    if (buffer->out_queue == NULL) {
        g_idle_add((GSourceFunc)flush_buffer_output, buffer);
        buffer->out_queue = parsed_msg;
        buffer->out_queue_end = parsed_msg;
    }
    else {
        buffer->out_queue_end->next = parsed_msg;
        buffer->out_queue_end = parsed_msg;
    }
    buffer->out_queue_size += parsed_msg_len;
    parsed_msg->msg_len = parsed_msg_len;
    pthread_mutex_unlock(&buffer->output_mutex);
}

static gboolean flush_buffer_output(struct buffer_info * buffer) {
    pthread_mutex_lock(&buffer->output_mutex);

    char * output_dump_utf8;
    size_t utf8_len;
    char * output_dump = alloca(buffer->out_queue_size + 1);
    char * dump_pos = output_dump;
    GtkTextIter end_of_buffer;
    GtkAdjustment * scroll_adjustment =
        gtk_scrollable_get_vadjustment(GTK_SCROLLABLE(buffer->window->chat_viewer));

    char * inbufpos;
    size_t conv_len;
    char * outbufpos;
    size_t avail;

    struct __queued_output * n;
    // Concatenate all the messages in the queue and clear it
    for (struct __queued_output * c = buffer->out_queue;
         c != NULL;
         c = n) {
        strcpy(dump_pos, c->msg);
        dump_pos += c->msg_len;
        n = c->next;
        free(c->msg);
        free(c);
    }

    // Figure out where the end of the buffer is
    gtk_text_buffer_get_end_iter(buffer->buffer, &end_of_buffer);

    // Ensure the text being printed is pure UTF-8
    output_dump_utf8 = alloca(buffer->out_queue_size * 2);
    inbufpos = output_dump;
    outbufpos = output_dump_utf8;
    conv_len = buffer->out_queue_size + 1;
    avail = buffer->out_queue_size * 2 + 1;
    
    iconv(buffer->locale_conv, &inbufpos, &conv_len, &outbufpos, &avail);

    /* If the user has manually scrolled, don't adjust the scroll position,
     * otherwise scroll to the bottom when printing the message
     */
    if (gtk_adjustment_get_value(scroll_adjustment) >=
        gtk_adjustment_get_upper(scroll_adjustment) -
        gtk_adjustment_get_page_size(scroll_adjustment) - 1e-12 &&
        buffer == buffer->window->current_buffer) {
        gtk_text_buffer_insert(buffer->buffer, &end_of_buffer, output_dump_utf8,
                               utf8_len);
        gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(buffer->window->chat_viewer),
                                     gtk_text_buffer_get_mark(buffer->buffer,
                                                              "insert"),
                                     0.0, false, 0.0, 0.0);
    }
    else
        gtk_text_buffer_insert(buffer->buffer, &end_of_buffer,
                               output_dump_utf8, -1);

    buffer->out_queue = NULL;
    buffer->out_queue_size = 0;
    pthread_mutex_unlock(&buffer->output_mutex);
    return false;
}

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
