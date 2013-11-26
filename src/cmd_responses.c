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

void sqchat_claim_response(struct sqchat_network * network,
                           struct sqchat_buffer * buffer,
                           void * data,
                           void (*data_free_func)(void *)) {
    sqchat_cmd_response_claim ** end;
    // Find the last response claim
    for (end = &network->claimed_responses; *end != NULL; end = &(*end)->next);

    *end = malloc(sizeof(sqchat_cmd_response_claim));
    (*end)->buffer = buffer;
    (*end)->data = data;
    (*end)->data_free_func = data_free_func;
    (*end)->next = NULL;
}

void sqchat_remove_last_response_claim(struct sqchat_network * network) {
    sqchat_cmd_response_claim * next;

    next = network->claimed_responses->next;

    if (network->claimed_responses->data != NULL)
        network->claimed_responses->data_free_func(
                network->claimed_responses->data);

    free(network->claimed_responses);
    network->claimed_responses = next;
}

/* These define the two different output behaviors many response handlers
 * follow; one for responses that check for response claims but can't satisfy
 * them, and one for responses that do the same but can satisfy claims.
 */
struct sqchat_buffer * sqchat_route_rpl(const struct sqchat_network * network) {
    if (network->claimed_responses != NULL)
        return network->claimed_responses->buffer;
    else
        return network->window->current_buffer;
}

struct sqchat_buffer * sqchat_route_rpl_end(struct sqchat_network * network) {
    if (network->claimed_responses != NULL) {
        struct sqchat_buffer * output = network->claimed_responses->buffer;
        sqchat_remove_last_response_claim(network);
        return output;
    }
    else
        return network->window->current_buffer;
}

// vim: set expandtab tw=80 shiftwidth=4 softtabstop=4 cinoptions=(0,W4:
