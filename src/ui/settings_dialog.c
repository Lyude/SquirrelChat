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

#include "../settings.h"
#include "chat_window.h"

#include <errno.h>
#include <gtk/gtk.h>

GtkWidget * settings_dialog;
GtkWidget * ok_cancel_button_box;
GtkWidget * ok_button;
GtkWidget * cancel_button;
GtkWidget * settings_notebook;

GtkWidget * user_info_label;
GtkWidget * user_info_page;
GtkWidget * your_info_frame;
GtkWidget * your_info_grid;
GtkWidget * default_nick_label;
GtkWidget * default_nick_entry;
GtkWidget * default_username_label;
GtkWidget * default_username_entry;
GtkWidget * default_real_name_label;
GtkWidget * default_real_name_entry;

static void settings_dialog_result_handler(GtkDialog * dialog,
                                           int response_id,
                                           struct sqchat_chat_window * parent);

void sqchat_settings_dialog_show(struct sqchat_chat_window * parent) {    
    settings_dialog = 
        gtk_dialog_new_with_buttons("SquirrelChat Settings",
                                    GTK_WINDOW(parent->window),
                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                    GTK_STOCK_OK,
                                    GTK_RESPONSE_OK,
                                    GTK_STOCK_APPLY,
                                    GTK_RESPONSE_APPLY,
                                    GTK_STOCK_CANCEL,
                                    GTK_RESPONSE_CANCEL,
                                    NULL);

    user_info_label = gtk_label_new("User Info");
    user_info_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    your_info_frame = gtk_frame_new("Your Info");
    gtk_widget_set_valign(your_info_frame, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(user_info_page), your_info_frame,
                       TRUE, TRUE, 0);

    your_info_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(your_info_grid), 5);
    gtk_container_add(GTK_CONTAINER(your_info_frame), your_info_grid);

    default_nick_label = gtk_label_new("Nickname: ");
    gtk_misc_set_alignment(GTK_MISC(default_nick_label), 0, 0.5);
    default_nick_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(default_nick_entry),
                       sqchat_default_nickname);

    default_username_label = gtk_label_new("Username: ");
    gtk_misc_set_alignment(GTK_MISC(default_username_label), 0, 0.5);
    default_username_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(default_username_entry),
                       sqchat_default_username);

    default_real_name_label = gtk_label_new("Real name: ");
    gtk_misc_set_alignment(GTK_MISC(default_real_name_label), 0, 0.5);
    default_real_name_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(default_real_name_entry),
                       sqchat_default_real_name);

    gtk_grid_insert_column(GTK_GRID(your_info_grid), 0);
    gtk_grid_insert_column(GTK_GRID(your_info_grid), 1);
    gtk_grid_insert_row(GTK_GRID(your_info_grid), 0);
    gtk_grid_insert_row(GTK_GRID(your_info_grid), 1);
    gtk_grid_insert_row(GTK_GRID(your_info_grid), 2);

    gtk_grid_attach(GTK_GRID(your_info_grid), default_nick_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(your_info_grid), default_nick_entry, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(your_info_grid), default_username_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(your_info_grid), default_username_entry, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(your_info_grid), default_real_name_label, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(your_info_grid), default_real_name_entry, 1, 2, 1, 1);

    settings_notebook = gtk_notebook_new();
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(settings_notebook),
                             GTK_POS_LEFT);
    gtk_notebook_append_page(GTK_NOTEBOOK(settings_notebook),
                             user_info_page,
                             gtk_label_new("User Info"));

    gtk_container_add(GTK_CONTAINER(
                      gtk_dialog_get_content_area(GTK_DIALOG(settings_dialog))
                      ),settings_notebook);

    g_signal_connect(settings_dialog, "response",
                     G_CALLBACK(settings_dialog_result_handler), parent);
    gtk_widget_show_all(settings_dialog);
}

static void settings_dialog_result_handler(GtkDialog * dialog,
                                           int response_id,
                                           struct sqchat_chat_window * parent) {
    if (response_id != GTK_RESPONSE_DELETE_EVENT &&
        response_id != GTK_RESPONSE_CANCEL) {

        if (response_id == GTK_RESPONSE_OK) {
            // Clear the old settings out of the memory
            g_free(sqchat_default_nickname);
            g_free(sqchat_default_username);
            g_free(sqchat_default_real_name);

            // Save the new settings in the memory
            sqchat_default_nickname =
                strdup(gtk_entry_get_text(GTK_ENTRY(default_nick_entry)));
            sqchat_default_username =
                strdup(gtk_entry_get_text(GTK_ENTRY(default_username_entry)));
            sqchat_default_real_name =
                strdup(gtk_entry_get_text(GTK_ENTRY(default_real_name_entry)));
            
            sqchat_settings_update_file("settings.conf",
                                        GTK_WINDOW(parent->window));
        }
    }
    else
        gtk_widget_destroy(settings_dialog);
}
// vim: set expandtab tw=80 shiftwidth=4 softtabstop=4 cinoptions=(0,W4:
