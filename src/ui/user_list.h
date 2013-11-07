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

extern void sqchat_user_list_new(struct sqchat_chat_window * window)
    _nonnull(1);

extern void sqchat_user_list_user_add(struct sqchat_buffer * buffer,
                                      const char * nickname,
                                      const char * prefix_str,
                                      size_t prefix_len)
    _nonnull(1, 2);
extern int sqchat_user_list_user_remove(struct sqchat_buffer * buffer,
                                        const char * nickname)
    _nonnull(1, 2);

extern int sqchat_user_list_user_prefix_add(struct sqchat_buffer * buffer,
                                            const char * nickname,
                                            const char * prefix)
    _nonnull(1, 2, 3);
extern int sqchat_user_list_user_prefix_subtract(struct sqchat_buffer * buffer,
                                                 const char * nickname,
                                                 char prefix)
    _nonnull(1, 2);

extern int sqchat_user_list_user_row_find(const struct sqchat_buffer * buffer,
                                          const char * nickname,
                                          GtkTreeIter * user_row)
    _nonnull(1, 2, 3);
extern char * sqchat_user_list_user_get_prefixes(const struct sqchat_buffer * buffer,
                                                 GtkTreeIter * user)
    _nonnull(1, 2);

extern void _sqchat_user_list_user_set_visible_prefix(struct sqchat_buffer * buffer,
                                                      GtkTreeIter * user_row,
                                                      char prefix)
    _nonnull(1, 2);
extern int _sqchat_user_list_user_set_visible_prefix_by_nick(struct sqchat_buffer * buffer,
                                                             const char * nickname,
                                                             char prefix)
    _nonnull(1, 2);

/* P99 doesn't seem to work with clang for some reason, so let's avoid it's use
 * here
 */
#ifdef __clang__
#define sqchat_user_list_user_set_visible_prefix(_buffer, _user, _prefix)               \
        _Generic((_user), default: _sqchat_user_list_user_set_visible_prefix_by_nick,   \
                          GtkTreeIter*: _sqchat_user_list_user_set_visible_prefix)
#else
#define sqchat_user_list_user_set_visible_prefix(_buffer, _user, _prefix)       \
        P99_GENERIC((_user), _sqchat_user_list_user_set_visible_prefix_by_nick, \
                             (GtkTreeIter*, _sqchat_user_list_user_set_visible_prefix))
#endif

#endif // __USER_LIST_H__
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
