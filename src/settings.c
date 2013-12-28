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

#include <stdbool.h>
#include <sys/stat.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/vfs.h>

#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>

GQuark main_settings_quark;

char * sqchat_config_dir;
char * sqchat_config_main_file_path;

GKeyFile * sqchat_main_settings;
char * sqchat_default_nickname;
char * sqchat_default_username;
char * sqchat_default_real_name;
char * sqchat_fallback_encoding;

static void config_file_error(const char * file, GError * error);
static void parse_settings(const char * filename, GKeyFile ** out);
static void cache_settings(const char * filename);
static void create_file(const char * filename);
static void try_to_load_setting_string(const char * filename,
                                       GKeyFile * keyfile,
                                       const char * group,
                                       const char * setting,
                                       const char * default_value,
                                       char ** out);

void sqchat_init_settings() {
    // Setup the quarks
    main_settings_quark = g_quark_from_static_string("settings.conf");

    // Figure out what the config directory should be
    sqchat_config_dir = g_strconcat(g_get_user_config_dir(), "/squirrelchat",
                                    NULL);

    // Instantiate the key file structs
    sqchat_main_settings = g_key_file_new();

    /* Check if there's already a settings folder for SquirrelChat, otherwise
     * make one
     */
    if (access(sqchat_config_dir, R_OK | W_OK) != 0) {
        g_mkdir_with_parents(sqchat_config_dir, 0755);
        create_file("settings.conf");
    }
    else
        parse_settings("settings.conf", &sqchat_main_settings);

    cache_settings("settings.conf");
}

GKeyFile * sqchat_get_keyfile_for_config(const char * filename) {
    if (g_quark_from_static_string(filename) == main_settings_quark)
        return sqchat_main_settings;
    else
        return NULL;
}

int sqchat_settings_update_file(const char * filename) {
    // Update the keyfile struct
    if (g_quark_from_static_string(filename) == main_settings_quark) {
        g_key_file_set_string(sqchat_main_settings, "main",
                              "default_nickname", sqchat_default_nickname);
        g_key_file_set_string(sqchat_main_settings, "main",
                              "default_username", sqchat_default_username);
        g_key_file_set_string(sqchat_main_settings, "main",
                              "default_real_name", sqchat_default_real_name);
    }

    char * config_data =
        g_key_file_to_data(sqchat_get_keyfile_for_config(filename), NULL, NULL);
    char * full_file_path = g_strconcat(sqchat_config_dir, "/", filename, NULL);
    FILE * file = fopen(full_file_path, "w");
    if (file == NULL) {
        g_free(config_data);
        g_free(full_file_path);
        return -1;
    }
    fputs(config_data, file);
    fclose(file);
    g_free(config_data);
    g_free(full_file_path);
    return 0;
}

/* Tries to open the configuration file and read in all it's settings into a
 * keyfile structure
 */
void parse_settings(const char * filename, GKeyFile ** out) {
    char * file_path;
    GError * error = NULL;

    if (g_key_file_load_from_dirs(*out, filename,
                                  (const char**)&sqchat_config_dir,
                                  &file_path, G_KEY_FILE_KEEP_COMMENTS, &error)
        == false) {
        if (g_error_matches(error, G_KEY_FILE_ERROR,
                            G_KEY_FILE_ERROR_NOT_FOUND))
            create_file(filename);
        else
            config_file_error(filename, error);
    }
}

/* Takes all of the settings in a keyfile structure and writes their values to
 * the global setting variables in SquirrelChat for faster access
 */
void cache_settings(const char * filename) {
    if (g_quark_from_static_string(filename) == main_settings_quark) {
        try_to_load_setting_string("settings.conf", sqchat_main_settings,
                                   "main", "default_nickname",
                                   g_get_user_name(), &sqchat_default_nickname);
        try_to_load_setting_string("settings.conf", sqchat_main_settings,
                                   "main", "default_username",
                                   g_get_user_name(), &sqchat_default_username);
        try_to_load_setting_string("settings.conf", sqchat_main_settings,
                                   "main", "default_real_name",
                                   g_get_real_name(), &sqchat_default_real_name);
        try_to_load_setting_string("settings.conf", sqchat_main_settings,
                                   "main", "fallback_encoding",
                                   "WINDOWS 1252", &sqchat_fallback_encoding);
    }
}

// Creates a default configuration file
void create_file(const char * filename) {
    GKeyFile * out;
    FILE * out_file;
    char * out_file_path;
    char * config_data;

    if (g_quark_from_static_string(filename) == main_settings_quark) {
        out = sqchat_main_settings;
        g_key_file_set_string(out, "main", "default_nickname",
                              g_get_user_name());
        g_key_file_set_string(out, "main", "default_username",
                              g_get_user_name());
        g_key_file_set_string(out, "main", "default_real_name",
                              g_get_real_name());
        g_key_file_set_string(out, "main", "fallback_encoding",
                              "WINDOWS-1252");
    }
    // placeholder, we should never reach this anyway
    else 
        out = NULL;

    // Try to save the settings to a file
    config_data = g_key_file_to_data(out, NULL, NULL);
    out_file_path = g_strconcat(sqchat_config_dir, "/", filename, NULL);
    out_file = fopen(out_file_path, "w");
    if (out_file == NULL)
        config_file_error(filename, NULL);
    fputs(config_data, out_file);
    fclose(out_file);
    g_free(config_data);
    g_free(out_file_path);

    // Cache the new settings
    cache_settings(filename);
}

/* Tries to load a string from a configuration value. If the string does not
 * exist, it assumes the string should be set to the default value.
 */
void try_to_load_setting_string(const char * filename,
                                GKeyFile * keyfile,
                                const char * group,
                                const char * setting,
                                const char * default_value,
                                char ** out) {
    GError * error;
    *out = g_key_file_get_string(keyfile, group, setting, &error);
    if (*out == NULL) {
        if (g_error_matches(error, G_KEY_FILE_ERROR,
                            G_KEY_FILE_ERROR_GROUP_NOT_FOUND) ||
            g_error_matches(error, G_KEY_FILE_ERROR,
                            G_KEY_FILE_ERROR_KEY_NOT_FOUND)) {
            g_key_file_set_string(keyfile, group, setting, default_value);
            *out = strdup(default_value);
        }
        else
            config_file_error(filename, error);
    }
}

/* Error reporting function used internally by this file, since configuration
 * file errors need to be handled differently then most of the errors in
 * SquirrelChat
 */
void config_file_error(const char * file, GError * error) {
    // Rely on errno if error was not specified
    if (error == NULL) {
        GtkWidget * dialog =
            gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_ERROR,
                                   GTK_BUTTONS_OK,
                                   _("Could not open the configuration file "
                                     "'%s' due to the following error:\n"
                                     "\"%s\"\n"
                                     "SquirrelChat cannot continue without a "
                                     "valid configuration file."),
                                   file, strerror(errno));
        gtk_dialog_run(GTK_DIALOG(dialog));
        exit(-1);
    }
    if (error->domain == G_KEY_FILE_ERROR) {
        GtkWidget * dialog =
            gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_WARNING,
                                   GTK_BUTTONS_YES_NO,
                                   _("Your settings could not be loaded due to "
                                     "an issue in the configuration file "
                                     "'%s':\n"
                                     "\"%s\"\n"
                                     "Errors like this are usually caused by "
                                     "the user incorrectly modifying one of "
                                     "the configuration files by hand, or by a "
                                     "bug in SquirrelChat. If you do believe "
                                     "this is a bug, please file a bug report "
                                     "and include this message.\n"
                                     "Would you like SquirrelChat to replace "
                                     "the broken file with a new file "
                                     "containing the default settings? If you "
                                     "do this, all of your previous settings "
                                     "will be lost.\n"),
                                     file, error->message);
        /* Delete the errorneous config file and start from scratch if approved
         * by the user
         */
        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_YES) {
            char * file_path = g_strconcat(sqchat_config_dir, "/", file, NULL);
            if (remove(file_path) == -1) {
                GtkWidget * dialog =
                    gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_ERROR,
                                           GTK_BUTTONS_OK,
                                           _("Could not delete configuration "
                                             "file:\n"
                                             "%s"),
                                           strerror(errno));
                gtk_dialog_run(GTK_DIALOG(dialog));
                exit(-1);
            }
            g_free(file_path);
            create_file(file);
            g_error_free(error);
        }
        else
            exit(-1);
        gtk_widget_destroy(dialog);
    }
    else {
        GtkWidget * dialog =
            gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_ERROR,
                                   GTK_BUTTONS_OK,
                                   _("Could not open the configuration "
                                     "file '%s' due to the following error:\n"
                                     "\"%s\"\n"
                                     "SquirrelChat cannot continue without a "
                                     "valid configuration file."),
                                   file, error->message);
        gtk_dialog_run(GTK_DIALOG(dialog));
        exit(-1);
    }
}

// vim: set expandtab tw=80 shiftwidth=4 softtabstop=4 cinoptions=(0,W4:
