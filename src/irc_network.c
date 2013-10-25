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

#include "irc_network.h"
#include "ui/buffer.h"
#include "net_io.h"
#include "net_input_handler.h"
#include "settings.h"
#include "connection_setup.h"

#include <gtk/gtk.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <fcntl.h>

#ifdef WITH_SSL
#include "ssl.h"
#endif

/* Creates a new network network and adds it to the network tree
 * NOTE: The struct is automatically sanitized by this function, so there is no
 * need to do it yourself
 */
struct sqchat_network * sqchat_new_irc_network() {
    // Allocate and sanitize the structure
    struct sqchat_network * network = malloc(sizeof(struct sqchat_network));
    memset(network, 0, sizeof(struct sqchat_network));

    network->nickname = strdup(config_setting_get_string(sqchat_default_nick));
    network->username = strdup(config_setting_get_string(sqchat_default_username));
    network->real_name = strdup(config_setting_get_string(sqchat_default_real_name));

    network->buffer = sqchat_buffer_new(NULL, NETWORK, network);

    network->buffers = sqchat_trie_new(sqchat_trie_strtolower);

    return network;
}

/* TODO: Switch the tab when freeing a network to ensure all references to the
 * text buffer are removed
 */
void sqchat_free_network(struct sqchat_network * network,
                         GtkTreeStore * network_tree_store) {
    sqchat_buffer_destroy(network->buffer);
    free(network->name);
    free(network->nickname);

    free(network);
}

void sqchat_connect_network(struct sqchat_network * network) {
#ifndef WITH_SSL
    if (network->ssl) {
        sqchat_buffer_print(network->buffer,
                            "SSL support was not included in this version of "
                            "Squirrelchat, therefore SSL cannot be enabled.\n");
        return;
    }
#endif

    // Make sure the server is set
    if (network->address == NULL) {
        sqchat_buffer_print(network->buffer, "You forgot to set a server!\n");
        return;
    }

    sqchat_buffer_print(network->buffer, "Attempting to connect to %s:%s...\n",
                        network->address, network->port); 
    sqchat_begin_connection(network);
}

void sqchat_disconnect_network(struct sqchat_network * network,
                            const char * msg) {
    sqchat_network_send(network, "QUIT :%s\r\n", msg ? msg : "");
    network->status = DISCONNECTED;

    free(network->version);
    free(network->server_name);
    free(network->chanmodes);
    free(network->usermodes);
    free(network->chantypes);
    free(network->chanmodes_a);
    free(network->chanmodes_b);
    free(network->chanmodes_c);
    free(network->chanmodes_d);
    free(network->prefix_chars);
    free(network->prefix_symbols);

    network->version = NULL;
    network->server_name = NULL;
    network->chanmodes = NULL;
    network->usermodes = NULL;
    network->chantypes = NULL;
    network->chanmodes_a = NULL;
    network->chanmodes_b = NULL;
    network->chanmodes_c = NULL;
    network->chanmodes_d = NULL;
    network->prefix_chars = NULL;
    network->prefix_symbols = NULL;
    network->casecmp = NULL;

#ifdef WITH_SSL
    if (network->ssl) {
        gnutls_bye(network->ssl_session, GNUTLS_SHUT_WR);

        network->ssl_cred = NULL;

        /* We leave the deinitialization of the ssl session to the net input
         * handler
         */
    }
#endif
}

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
