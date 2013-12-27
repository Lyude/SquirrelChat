/* Convenience functions for sending commands to servers and reading messages
 * from servers
 *
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

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include "irc_network.h"
#include "net_io.h"
#include "macros.h"

#include <stdbool.h>
#include <string.h>

#include <gnutls/gnutls.h>

// Sends a message to a currently connected IRC network
void sqchat_network_send(struct sqchat_network * network,
                         const char * msg, ...) {
    va_list args;
    size_t msg_len;
    char send_buffer[SQCHAT_MSG_BUF_LEN];
    sqchat_server * server = network->current_server->data;

    va_start(args, msg);
    msg_len = vsnprintf(&send_buffer[0], SQCHAT_IRC_MSG_LEN, msg, args);
    va_end(args);

#ifdef WITH_SSL
    if (server->ssl)
        gnutls_write(network->ssl_session, &send_buffer, msg_len);
    else
        send(network->socket, &send_buffer, msg_len, 0);
#else
    send(network->socket, &send_buffer, msg_len, 0);
#endif
}

// vim: set expandtab tw=80 shiftwidth=4 softtabstop=4 cinoptions=(0,W4:
