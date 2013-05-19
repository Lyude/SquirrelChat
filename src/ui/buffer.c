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

void command_box_activated_handler(GtkEntry * entry,
                                   struct buffer_info * buffer) {
    char * input = gtk_entry_get_text(entry);
    guint16 input_len = gtk_entry_get_text_length(entry);
    // If the input is a blank message or a blank command, do nothing
    switch (input_len) {
        case 0:
            return;
            break;
        case 1:
            if (input[0] == '/')
                return;
            break;
        case 2:
            if (input[0] == '/' && input[1] == '/')
                return;
            break;
    }

    // Check whether or not the input is a command and act accordingly
    if (input[0] == '/' && input[1] != '/') {
        char * param_start;
        char * command_end;
        char * command;
    
        // Eat the /
        input++;

        // Extract the name of the command
        command_end = strchr(input, ' ');
        if (command_end == NULL) {
            command = input;
            param_start = NULL;
        }
        else {
            *command_end = '\0';
            command = input;
            input = command_end + 1;

            // Extract the parameters
            for (param_start = input; *param_start == ' '; ++param_start);
        }

        call_command(buffer, command, param_start);
    }

    gtk_entry_set_text(entry, "");
}

struct buffer_info * new_buffer(enum buffer_type type,
                                struct irc_network * network) {
    struct buffer_info * buffer = malloc(sizeof(struct buffer_info));
    buffer->type = type;
    buffer->parent_network = network;

    buffer->buffer = gtk_text_buffer_new(NULL);

    buffer->chat_viewer = gtk_text_view_new_with_buffer(buffer->buffer);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(buffer->chat_viewer), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(buffer->chat_viewer), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(buffer->chat_viewer),
                                GTK_WRAP_WORD_CHAR);
    
    buffer->scrolled_window_for_chat_viewer = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_kinetic_scrolling(
            GTK_SCROLLED_WINDOW(buffer->scrolled_window_for_chat_viewer), TRUE);
    gtk_container_add(GTK_CONTAINER(buffer->scrolled_window_for_chat_viewer),
                      buffer->chat_viewer);

    buffer->command_box = gtk_entry_new();

    buffer->chat_and_command_box_container =
        gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(buffer->chat_and_command_box_container),
                       buffer->scrolled_window_for_chat_viewer, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(buffer->chat_and_command_box_container),
                       buffer->command_box, FALSE, FALSE, 0);

    // Connect signals
    g_signal_connect(buffer->command_box, "activate",
                     G_CALLBACK(command_box_activated_handler), buffer);

    // Add a userlist if the buffer is a channel buffer
    if (type == CHANNEL) {
        buffer->user_list_store =
            gtk_list_store_new(2, G_TYPE_CHAR, G_TYPE_STRING);
        buffer->user_list =
            gtk_tree_view_new_with_model(GTK_TREE_MODEL(buffer->user_list_store));

        buffer->chat_viewer_and_user_list_pane =
            gtk_paned_new(GTK_ORIENTATION_VERTICAL);
        gtk_paned_add1(GTK_PANED(buffer->chat_viewer_and_user_list_pane),
                       buffer->chat_and_command_box_container);
        gtk_paned_add2(GTK_PANED(buffer->chat_viewer_and_user_list_pane),
                       buffer->user_list);
    }
    return buffer;
}

void destroy_buffer(struct buffer_info * buffer) {
    if (buffer->type == CHANNEL) {
        g_object_unref(buffer->user_list_store);
        gtk_widget_destroy(buffer->chat_viewer_and_user_list_pane);
    }
    else
        gtk_widget_destroy(buffer->chat_and_command_box_container);
    
    free(buffer);
}

void print_to_buffer(struct buffer_info * buffer,
                     char * message, ...) {
    va_list args;
    char * parsed_message;
    gchar * parsed_message_utf8;
    size_t parsed_message_len;
    size_t parsed_message_utf8_len;
    GtkTextIter end_of_buffer;
    GtkTextBuffer * text_buffer =
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(buffer->chat_viewer));

    // Parse the message passed to this function
    va_start(args, message);
    parsed_message_len = vsnprintf(NULL, 0, message, args);

    va_start(args, message);
    parsed_message = malloc(parsed_message_len);
    vsnprintf(parsed_message, parsed_message_len + 1, message, args);
    va_end(args);

    /* FIXME (maybe): String is already supposed to be in utf8, but glib doesn't
     * think so, so we convert it to glib's liking
     */
    parsed_message_utf8 = g_locale_to_utf8(parsed_message,
                                           parsed_message_len,
                                           &parsed_message_utf8_len,
                                           NULL, NULL);
    free(parsed_message);
    
    // Figure out where the end of the buffer is
    gtk_text_buffer_get_end_iter(text_buffer, &end_of_buffer);

    /* TODO: Add in code to maintain the line limit for the buffer whenever
     * anything is printed to the buffer
     */

    // Print the message
    gtk_text_buffer_insert(text_buffer, &end_of_buffer, parsed_message_utf8,
                           parsed_message_utf8_len);
    free(parsed_message_utf8);
}

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
