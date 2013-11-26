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

#include "settings.h"

#include <sys/stat.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/vfs.h>

#include <libconfig.h>
#include <gtk/gtk.h>

config_t sqchat_global_config;
config_setting_t * sqchat_global_config_root;

char * sqchat_config_dir;

config_setting_t * sqchat_default_nick;
config_setting_t * sqchat_default_username;
config_setting_t * sqchat_default_real_name;
config_setting_t * sqchat_default_fallback_encoding;

static void create_main_settings_file();
static void set_default_settings();

void sqchat_init_settings() {
    struct statfs buf;
    int ret;
    char * filename_buf;
    const char * home_dir = getenv("HOME");

    config_init(&sqchat_global_config);
    sqchat_global_config_root = config_root_setting(&sqchat_global_config);

    // Figure out the maximum length of a filename on this filesystem
    statfs(home_dir, &buf);
    filename_buf = alloca(buf.f_namelen);

    sqchat_config_dir = malloc(buf.f_namelen);
    sqchat_config_dir = realloc(sqchat_config_dir, snprintf(sqchat_config_dir, buf.f_namelen,
                                              "%s/.config/squirrelchat",
                                              home_dir) + 1);

    // Check if we have a configuration folder in the user's home directory
    /* TODO: when i'm not drunk figure out if we should let the group read it or
     * not
     */
    snprintf(filename_buf, buf.f_namelen, "%s/.config", home_dir);
    mkdir(filename_buf, S_IRWXU | S_IRWXG | S_IRWXO);

    // If the function was sucessful, we need to make a default settings file
    if (mkdir(sqchat_config_dir, S_IRWXU | S_IRGRP | S_IROTH) == 0) {
        // TODO: Display first time run wizard
        umask(S_IRWXG | S_IRWXO);
        create_main_settings_file();
    }
    else if (errno == EEXIST) {
        FILE * config_file;
        snprintf(filename_buf, buf.f_namelen, "%s/squirrelchat.conf", sqchat_config_dir);

        /* Check to make sure all the required configuration files exist and are
         * readable
         */
        if ((config_file = fopen(filename_buf, "r")) == NULL) {
            int error = errno;
            set_default_settings();
            if (error == ENOENT)
                create_main_settings_file();
            else {
                GtkWidget * dialog;
                dialog = gtk_message_dialog_new(
                    NULL,
                    GTK_DIALOG_MODAL,
                    GTK_MESSAGE_WARNING,
                    GTK_BUTTONS_OK,
                    "Squirrelchat could not open your user profile: %s.\n"
                    "As a result, none of your settings can be loaded.",
                    strerror(error));
                gtk_window_set_title(GTK_WINDOW(dialog), "SquirrelChat Error");
                gtk_dialog_run(GTK_DIALOG(dialog));
                gtk_widget_destroy(dialog);
            }
        }

        // Try to read the settings from the user's profile
        else if (config_read(&sqchat_global_config, config_file) == CONFIG_FALSE) {
            char * err_msg = strdup(config_error_text(&sqchat_global_config));
            config_error_t err_type = config_error_type(&sqchat_global_config);
            char * err_file = (config_error_file(&sqchat_global_config) ?
                               strdup(config_error_file(&sqchat_global_config)) :
                               NULL);
            int err_line = config_error_line(&sqchat_global_config);

            /* We can't trust the config structure since the configuration file
             * most likely has bad syntax, and implementing something to check
             * for any possible missing settings along with a sane configuration
             * structure would take way too long for me to implement right now.
             * Maybe sometime in the future, but for right now we're just going
             * to destroy the config in the memory and create a new one
             */
            config_destroy(&sqchat_global_config);
            config_init(&sqchat_global_config);

            set_default_settings();

            GtkWidget * dialog;
            dialog = (err_type == CONFIG_ERR_PARSE) ? 
                gtk_message_dialog_new(
                    NULL,
                    GTK_DIALOG_MODAL,
                    GTK_MESSAGE_WARNING,
                    GTK_BUTTONS_OK,
                    "SquirrelChat could not use your settings due to a syntax "
                    "error in squirrelchat.conf at line %i.",
                    err_line) :
                gtk_message_dialog_new(
                    NULL,
                    GTK_DIALOG_MODAL,
                    GTK_MESSAGE_WARNING,
                    GTK_BUTTONS_OK,
                    "Squirrelchat could not read your user profile: %s\n"
                    "As a result, none of your settings can be loaded.",
                    err_msg);

            gtk_window_set_title(GTK_WINDOW(dialog), "SquirrelChat Error");
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            fclose(config_file);
            free(err_msg);
            free(err_file);
        }
        else
            fclose(config_file);

        // Cache settings to their global variables
        sqchat_default_nick = config_setting_get_member(sqchat_global_config_root,
                                                    "default_nick");
        sqchat_default_username = config_setting_get_member(sqchat_global_config_root,
                                                        "default_username");
        sqchat_default_real_name = config_setting_get_member(sqchat_global_config_root,
                                                         "default_real_name");
        sqchat_default_fallback_encoding =
            config_setting_get_member(sqchat_global_config_root,
                                      "default_fallback_encoding");
    }
}

static void set_default_settings() {
    sqchat_default_nick = config_setting_add(sqchat_global_config_root, "default_nick",
                                         CONFIG_TYPE_STRING);
    sqchat_default_username =
        config_setting_add(sqchat_global_config_root, "default_username",
                           CONFIG_TYPE_STRING);
    sqchat_default_real_name =
        config_setting_add(sqchat_global_config_root, "default_real_name",
                           CONFIG_TYPE_STRING);
    sqchat_default_fallback_encoding =
        config_setting_add(sqchat_global_config_root, "default_fallback_encoding",
                           CONFIG_TYPE_STRING);

    config_setting_set_string(sqchat_default_nick, getlogin());
    config_setting_set_string(sqchat_default_username, getlogin());
    config_setting_set_string(sqchat_default_real_name, getlogin());
    config_setting_set_string(sqchat_default_fallback_encoding, "WINDOWS-1252");
}

static void create_main_settings_file() {
    set_default_settings();

    // Write all the settings to a new file
    char filename_buf[strlen(sqchat_config_dir) + sizeof("/squirrelchat.conf")];
    sprintf(&filename_buf[0], "%s/squirrelchat.conf", sqchat_config_dir);
    if (config_write_file(&sqchat_global_config, filename_buf) == CONFIG_FALSE) {
        // Alert the user
        GtkWidget * dialog;
        dialog = gtk_message_dialog_new(
            NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_OK,
            "SquirrelChat did not find a user profile, but failed to create "
            "one for you (%s).\n"
            "SquirrelChat can continue, but will not be able to save any "
            "settings you change from their defaults.",
            strerror(errno));
        gtk_window_set_title(GTK_WINDOW(dialog), "SquirrelChat Error");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
}

// vim: set expandtab tw=80 shiftwidth=4 softtabstop=4 cinoptions=(0,W4:
