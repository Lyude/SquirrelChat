/* Code for the callback function used whenever there is data waiting on a
 * connected network buffer's socket. It parses the message, dispatches the
 * appropriate callbacks, etc.
 */
#include "net_input_handler.h"
#include "buffers.h"
#include "net_io.h"

#include <glib.h>
#include <errno.h>

gboolean net_input_handler(GIOChannel *source,
                           GIOCondition condition,
                           struct network_buffer * buffer) {
    GIOChannelError io_error;
    int errno_local;
    char * msg;
    // Keep reading messages off the socket until there's no data waiting
    
    while (receive_from_network(buffer, &msg)) {
        print_to_network_buffer(buffer, "%s\n", msg);
        printf("%s\n", msg);
    }

    errno_local = errno;
    return TRUE;
}

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
