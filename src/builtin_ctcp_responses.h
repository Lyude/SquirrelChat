/* Callbacks for the built-in CTCP types
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
#ifndef __BUILTIN_CTCP_RESPONSES_H__
#define __BUILTIN_CTCP_RESPONSES_H__
#include "ctcp.h"

#define BUILTIN_CTCP_RESP(func_name)                            \
    extern void func_name(struct sqchat_network * network,      \
                          char * hostmask,                      \
                          char * target,                        \
                          char * msg)                           \
    _attr_nonnull(1, 2)

BUILTIN_CTCP_RESP(sqchat_ctcp_version_resp_handler);
BUILTIN_CTCP_RESP(sqchat_ctcp_ping_resp_handler);

#undef BUILTIN_CTCP_RESP

#endif // __BUILTIN_CTCP_RESPONSES_H__

// vim: set expandtab tw=80 shiftwidth=4 softtabstop=4 cinoptions=(0,W4:
