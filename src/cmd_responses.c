/* Contains functions for handling command responses
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
#include "cmd_responses.h"
#include "irc_network.h"

#include <stdlib.h>

void request_cmd_response(struct irc_network * network,
                          struct buffer_info * buffer,
                          unsigned short irc_numeric) {
    irc_response_queue ** end;
    // Find the location of the end of the queue
    for (end = &network->response_queue; *end != NULL; end = &(*end)->next);

    // Create the new request
    *end = malloc(sizeof(struct irc_response_queue));
    (*end)->buffer = buffer;
    (*end)->irc_numeric = irc_numeric;
    (*end)->next = NULL;
}
irc_response_queue ** find_cmd_response_request(struct irc_network * network,
                                                unsigned short irc_numeric) {
    irc_response_queue ** request;
    // Find the location of the pointer to the response
    if (network->response_queue == NULL)
        return NULL;
    for (request = &network->response_queue;
         (*request)->irc_numeric != irc_numeric;
         request = &(*request)->next)
        if ((*request)->next == NULL)
            return NULL;
    return request;
}

void remove_cmd_response_request(irc_response_queue ** response) {
    irc_response_queue * next_node = (*response)->next;
    free(*response);
    *response = next_node;
}

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
