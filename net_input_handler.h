#ifndef NET_INPUT_HANDLER_H
#define NET_INPUT_HANDLER_H

#include "buffers.h"

#include <glib.h>

gboolean net_input_handler(GIOChannel *source,
                           GIOCondition condition,
                           struct network_buffer * buffer);

#endif // NET_INPUT_HANDLER_H
