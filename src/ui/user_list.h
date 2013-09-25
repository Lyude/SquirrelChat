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

#ifndef __USER_LIST_H__
#define __USER_LIST_H__

#include "chat_window.h"

#ifndef __clang__
#include <p99_generic.h>
#endif

extern void create_user_list(struct chat_window * window)
    _nonnull(1);

extern void add_user_to_list(struct buffer_info * buffer,
                             const char * nickname,
                             const char * prefix_str,
                             size_t prefix_len)
    _nonnull(1, 2);
extern int remove_user_from_list(struct buffer_info * buffer,
                                 const char * nickname)
    _nonnull(1, 2);
extern void _set_user_prefix(struct buffer_info * buffer,
                             GtkTreeIter * user_row,
                             char prefix)
    _nonnull(1, 2);
extern int _set_user_prefix_by_nick(struct buffer_info * buffer,
                                    const char * nickname,
                                    char prefix)
    _nonnull(1, 2);
extern int add_prefix_to_user(struct buffer_info * buffer,
                              const char * nickname,
                              const char * prefix)
    _nonnull(1, 2, 3);
extern int remove_prefix_from_user(struct buffer_info * buffer,
                                   const char * nickname,
                                   char prefix)
    _nonnull(1, 2);

extern int get_user_row(const struct buffer_info * buffer,
                        const char * nickname,
                        GtkTreeIter * user_row)
    _nonnull(1, 2, 3);
extern char * get_user_prefixes(const struct buffer_info * buffer,
                                GtkTreeIter * user)
    _nonnull(1, 2);

/* P99 doesn't seem to work with clang for some reason, so let's avoid it's use
 * here
 */
#ifdef __clang__
#define set_user_prefix(_buffer, _user, _prefix)                    \
        _Generic((_user), default: _set_user_prefix_by_nick,        \
                          GtkTreeIter*: _set_user_prefix_by_nick)
#else
#define set_user_prefix(_buffer, _user, _prefix)                        \
        P99_GENERIC((_user), _set_user_prefix_by_nick,                  \
                             (GtkTreeIter*, _set_user_prefix_by_nick))
#endif

#endif // __USER_LIST_H__
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
