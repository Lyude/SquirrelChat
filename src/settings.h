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
#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <glib.h>

extern char * sqchat_config_dir;
extern char * sqchat_config_main_file_path;

extern void sqchat_init_settings();
extern GKeyFile * sqchat_get_keyfile_for_config(const char * filename)
    _nonnull(1);
extern int sqchat_settings_update_file(const char * filename)
    _nonnull(1);

extern GKeyFile * sqchat_main_settings;
extern char * sqchat_default_nickname;
extern char * sqchat_default_username;
extern char * sqchat_default_real_name;
extern char * sqchat_fallback_encoding;

#endif // __SETTINGS_H__
// vim: set expandtab tw=80 shiftwidth=4 softtabstop=4 cinoptions=(0,W4:
