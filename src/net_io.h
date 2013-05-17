#ifndef NET_IO_H
#define NET_IO_H 

#include "irc_network.h"
#include <stdbool.h>

void send_to_network(struct irc_network * buffer,
                     char * msg, ...);
bool receive_from_network(struct irc_network * buffer, char ** output);

#endif /* NET_IO_H */
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
