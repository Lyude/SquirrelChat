#ifndef NET_IO_H
#define NET_IO_H 

#include "buffers.h"
#include <stdbool.h>

void send_to_network(struct network_buffer * buffer,
                     char * msg, ...);
bool receive_from_network(struct network_buffer * buffer, char ** output);

#endif /* NET_IO_H */
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
