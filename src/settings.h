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
#ifndef __SQUIRRELCHAT_SETTINGS_H__
#define __SQUIRRELCHAT_SETTINGS_H__

#include <libconfig.h>

extern config_t global_config;
extern config_setting_t * global_config_root;
extern char * sq_config_dir;

extern void init_settings();

extern config_setting_t * sq_default_nick;
extern config_setting_t * sq_default_username;
extern config_setting_t * sq_default_real_name;

#endif
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
