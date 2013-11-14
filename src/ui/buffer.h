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

#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "../irc_network.h"
#include "chat_window.h"
#include "../trie.h"

#include <pthread.h>
#include <gtk/gtk.h>

enum sqchat_buffer_type {
    NETWORK,
    CHANNEL,
    QUERY
};

struct __sqchat_channel_data {
    GtkListStore * user_list_store;
    sqchat_trie * users;
};

struct __sqchat_query_data {
    char * away_msg;
    bool received_away;
};

struct __sqchat_queued_output {
    char * msg;
    size_t msg_len;
    struct __sqchat_queued_output * next;
};

struct sqchat_buffer {
    enum sqchat_buffer_type type;
    char * buffer_name;
    GtkTreeRowReference * row;

    pthread_mutex_t output_mutex;
    struct __sqchat_queued_output * out_queue;
    struct __sqchat_queued_output * out_queue_end;
    size_t out_queue_size;

    struct sqchat_network * network;
    struct sqchat_chat_window * window;
    GtkTextBuffer * buffer;
    double buffer_scroll_pos;
    GtkEntryBuffer * command_box_buffer;

    union {
        struct __sqchat_channel_data * chan_data;
        struct __sqchat_query_data * query_data;
        void * extra_data;
    };
};

extern struct sqchat_buffer * sqchat_buffer_new(const char * buffer_name,
                                                enum sqchat_buffer_type type,
                                                struct sqchat_network * network);
extern void sqchat_buffer_destroy(struct sqchat_buffer * buffer)
    _nonnull(1);

extern void sqchat_buffer_print(struct sqchat_buffer * buffer,
                                const char * msg, ...)
    _nonnull(1, 2) _format(printf, 2, 3);

#endif /* __BUFFER_H__ */
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4:cinoptions=(0,W4
