/* 
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
#ifndef __MESSAGE_PARSER_H__
#define __MESSAGE_PARSER_H__
#include "irc_network.h"

typedef void (*irc_message_callback)(struct irc_network *,
                                     char *,     // hostmask
                                     short,      // argc
                                     char*[]);    // argv

extern void init_message_parser();
extern void process_irc_message(struct irc_network * network, char * msg);
#endif
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
