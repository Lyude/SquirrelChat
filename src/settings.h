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

#include <libconfig.h>

extern config_t sqchat_global_config;
extern config_setting_t * sqchat_global_config_root;
extern char * sqchat_config_dir;

extern void sqchat_init_settings();

extern config_setting_t * sqchat_default_nick;
extern config_setting_t * sqchat_default_username;
extern config_setting_t * sqchat_default_real_name;
extern config_setting_t * sqchat_default_fallback_encoding;

#endif // __SETTINGS_H__
// vim: set expandtab tw=80 shiftwidth=4 softtabstop=4 cinoptions=(0,W4:
