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

#ifndef __NET_IO_H__
#define __NET_IO_H__

#include "irc_network.h"
#include <stdbool.h>

extern void sqchat_network_send(struct sqchat_network * buffer,
                                const char * msg, ...)
    _nonnull(1, 2) _format(printf, 2, 3);

#endif /* __NET_IO_H__ */
// vim: set expandtab tw=80 shiftwidth=4 softtabstop=4 cinoptions=(0,W4:
