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

#ifndef __CONNECTION_SETUP_H__
#define __CONNECTION_SETUP_H__

#include "irc_network.h"

extern void begin_connection(struct irc_network * network)
    _nonnull(1);

extern void begin_registration(struct irc_network * network)
    _nonnull(1);

#endif // __CONNECTION_SETUP_H__
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
