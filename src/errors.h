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
#ifndef __SQUIRRELCHAT_ERRORS_H__
#define __SQUIRRELCHAT_ERRORS_H__

#include "ui/buffer.h"

extern void sqchat_dump_msg_to_buffer(struct sqchat_buffer * buffer,
                                      char * hostmask,
                                      short argc,
                                      char * argv[])
    _attr_nonnull(1);

#ifdef WITH_SSL
#if GNUTLS_DEBUG_LEVEL > 0
extern void _sqchat_gnutls_debug_log(int level, const char * msg)
    _attr_nonnull(2);
#endif
#endif

#endif // __SQUIRRELCHAT_ERRORS_H__
// vim: set expandtab tw=80 shiftwidth=4 softtabstop=4 cinoptions=(0,W4:
