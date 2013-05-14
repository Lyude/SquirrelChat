/* Code for the callback function used whenever there is data waiting on a
 * connected network buffer's socket. It parses the message, dispatches the
 * appropriate callbacks, etc.
 */
#include "net_input_handler.h"
#include "buffers.h"
#include "net_io.h"

#include <glib.h>
#include <string.h>
#include <errno.h>

/* Checks for messages waiting in a network's buffer
 * If there is still unparsed data waiting in the buffer and check_for_messages
 * needs to be called again, returns true, otherwise false.
 */
bool check_for_messages(struct network_buffer * buffer,
                        char ** output) {
    char * next_terminator = strstr(&buffer->recv_buffer[buffer->buffer_cursor],
                                    "\r\n");

    // If a message was found, remove the terminator, set output variable
    if (next_terminator != NULL) {
        *next_terminator = '\0';
        *(next_terminator + 1) = '\0';
        *output = &buffer->recv_buffer[buffer->buffer_cursor];
        buffer->buffer_cursor = (int)(next_terminator -
                                      &buffer->recv_buffer[0]) + 2;

        // Check if there's potentially another message in the buffer
        if (buffer->buffer_cursor + 1 < buffer->buffer_fill_len) {
            return true;
        }
        else {
            buffer->buffer_cursor = 0;
            buffer->buffer_fill_len = 0;
            return false;
        }
    }
    else {
        /* If no message was found but we're not at the end of the buffer, that
         * means there's probably a only partially received message waiting
         * after the position of the cursor. Make sure there is space for the
         * rest of the message to be received
         */
        if (buffer->buffer_cursor < buffer->buffer_fill_len) {
            buffer->buffer_fill_len -= buffer->buffer_cursor;
            memmove(&buffer->recv_buffer[0],
                    &buffer->recv_buffer[buffer->buffer_cursor],
                    buffer->buffer_fill_len);
        }

        // If the message is longer then the max allowed length, report an error
        else if (buffer->buffer_cursor == 0 &&
                 buffer->buffer_fill_len >= IRC_MSG_LEN)
            errno = EMSGSIZE;

        buffer->buffer_cursor = 0;
        return false;
    }
}


gboolean net_input_handler(GIOChannel *source,
                           GIOCondition condition,
                           struct network_buffer * buffer) {
    char * msg;
    int recv_result;

    recv_result = recv(buffer->socket,
                       &buffer->recv_buffer[buffer->buffer_fill_len],
                       IRC_MSG_LEN - buffer->buffer_fill_len, 0);

    buffer->buffer_fill_len += recv_result;

    // Temporary, just for testing
    while (check_for_messages(buffer, &msg)) {
        print_to_network_buffer(buffer, "%s\n", msg);
        printf("%s\n", msg);
    }

    return TRUE;
}

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
