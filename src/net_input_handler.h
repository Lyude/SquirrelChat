#ifndef NET_INPUT_HANDLER_H
#define NET_INPUT_HANDLER_H

#include "irc_network.h"

#include <glib.h>

gboolean net_input_handler(GIOChannel *source,
                           GIOCondition condition,
                           struct irc_network * buffer);

#endif // NET_INPUT_HANDLER_H
