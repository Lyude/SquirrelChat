/* Convience functions for sending commands to servers and reading messages from
 * servers
 */
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include "buffers.h"
#include "net_io.h"
#include "irc_macros.h"

#include <stdbool.h>
#include <string.h>

// Sends a message to a currently connected IRC network
void send_to_network(struct network_buffer * buffer,
                     char * msg, ...) {
    va_list args;
    size_t msg_len;
    char send_buffer[IRC_MSG_BUF_LEN];

    va_start(args, msg);
    msg_len = vsnprintf(&send_buffer, IRC_MSG_LEN, msg, args);
    va_end(args);

    send(buffer->socket, &send_buffer, msg_len, 0);
}

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
