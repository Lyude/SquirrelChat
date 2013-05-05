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

/* Reads a single message from the server, if there are more messages waiting
 * returns true, otherwise false
 */
bool receive_from_network(struct network_buffer * buffer, char ** output) {
    static char recv_buffer[IRC_MSG_BUF_LEN];
    static size_t buffer_real_len;
    static int buffer_cursor;
    char * next_terminator = NULL;

    // Find the next message
    while ((next_terminator = strstr(&recv_buffer[buffer_cursor], "\r\n"))
            == NULL) {
        if (buffer_real_len >= IRC_MSG_LEN) {
            /* If the cursor's at the beginning that means the message was
             * longer then 512 bytes, so we abort
             */
            if (buffer_cursor == 0) {
                *output = NULL;
                errno = EMSGSIZE;
                return false;
            }
            /* Move the segment of the next message to the beginning of the
             * buffer
             */
            memmove(&recv_buffer[0], &recv_buffer[buffer_cursor],
                    (buffer_real_len = IRC_MSG_BUF_LEN - buffer_cursor - 1));
            buffer_cursor = 0;
        }
        buffer_real_len += recvfrom(buffer->socket,
                                    &recv_buffer[buffer_real_len],
                                    IRC_MSG_LEN - buffer_real_len, 0,
                                    &buffer->ai_addr, &buffer->ai_addrlen);
    }

    // Remove the terminators and set the output pointer
    *next_terminator = '\0';
    *(next_terminator + 1) = '\0';
    *output = &recv_buffer[buffer_cursor];
    buffer_cursor = (int)(next_terminator - &recv_buffer[0]) + 2;

    // Check whether or not there is more data waiting and return
    if (buffer_cursor < buffer_real_len)
        return true;
    else {
        buffer_cursor = 0;
        return false;
    }
}

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
