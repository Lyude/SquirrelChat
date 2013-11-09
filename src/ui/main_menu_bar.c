/* Functions for setting up the main menu bar in a chat window
 *
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

#include <gtk/gtk.h>
#include "chat_window.h"
#include "buffer.h"
#include "../irc_network.h"
#include "network_tree.h"
#include "main_menu_bar.h"
#include "settings_dialog.h"

void sqchat_main_menu_bar_new(struct sqchat_chat_window * window) {
    /* Menu items and submenus that don't need to be referenced outside of this
     * scope
     */

    window->main_menu_bar = gtk_menu_bar_new();
    window->main_menu = gtk_menu_new();

    window->main_menu_item = gtk_menu_item_new_with_label("SquirrelChat");
    window->connect_menu_item = gtk_menu_item_new_with_label("Connect");
    window->new_server_buffer_menu_item =
        gtk_menu_item_new_with_label("New Network Tab");
    window->exit_menu_item = gtk_menu_item_new_with_label("Exit");

    window->tools_menu = gtk_menu_new();
    window->tools_menu_item = gtk_menu_item_new_with_label("Tools");
    window->preferences_menu_item = gtk_menu_item_new_with_label("Preferences");

    window->help_menu = gtk_menu_new();
    window->help_menu_item = gtk_menu_item_new_with_label("Help");
    window->about_menu_item = gtk_menu_item_new_with_label("About SquirrelChat");

    // Build the menu structure
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(window->main_menu_item),
                              window->main_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(window->main_menu),
                          window->connect_menu_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(window->main_menu),
                          window->new_server_buffer_menu_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(window->main_menu),
                          window->exit_menu_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(window->main_menu_bar),
                          window->main_menu_item);

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(window->tools_menu_item),
                              window->tools_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(window->tools_menu),
                          window->preferences_menu_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(window->main_menu_bar),
                          window->tools_menu_item);

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(window->help_menu_item),
                              window->help_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(window->help_menu),
                          window->about_menu_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(window->main_menu_bar),
                          window->help_menu_item);
}

void new_network_menu_item_callback(GtkMenuItem * menuitem,
                                    struct sqchat_chat_window * window) {
    struct sqchat_network * new_network = sqchat_network_new();

    sqchat_network_tree_network_add(window, new_network);
    sqchat_chat_window_change_active_buffer(window, new_network->buffer);
}

void about_menu_item_callback(GtkMenuItem * menuitem,
                              struct sqchat_chat_window * window) {
    char * authors[] = { "Stephen Chandler Paul",
                         "Quora Dodrill",
                         NULL };
    char * copyright = "Copyright ©2013 Stephen Chandler Paul\n"
                       "Copyright ©2013 SquirrelChat Developers\n"
                       "Some components ©2013 Alex Iadicicco";

    gtk_show_about_dialog(GTK_WINDOW(window->window),
                          "license-type", GTK_LICENSE_GPL_2_0,
                          "wrap-license", TRUE,
                          "authors", authors,
                          "copyright", copyright,
                          NULL);
}

// Callback used by the "Connect" menu item
void connect_current_network(GtkMenuItem * menuitem,
                             struct sqchat_chat_window * window) {
    GtkTreeIter selected_row;
    struct sqchat_network * network;

    sqchat_network_connect(window->current_buffer->network);
}

static void preferences_menu_item_cb(GtkMenuItem * menuitem,
                                     struct sqchat_chat_window * parent) {
    sqchat_settings_dialog_show(parent);
}

// Connects all the signals for the items in the menu bar
void sqchat_main_menu_bar_connect_signals(struct sqchat_chat_window * window) {
    g_signal_connect(window->connect_menu_item, "activate",
                     G_CALLBACK(connect_current_network), window);
    g_signal_connect(window->new_server_buffer_menu_item, "activate",
                     G_CALLBACK(new_network_menu_item_callback), window);
    g_signal_connect(window->exit_menu_item, "activate", G_CALLBACK(gtk_main_quit),
                     NULL);

    g_signal_connect(window->preferences_menu_item, "activate",
                     G_CALLBACK(preferences_menu_item_cb), window);

    g_signal_connect(window->about_menu_item, "activate",
                     G_CALLBACK(about_menu_item_callback), window);
}

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
