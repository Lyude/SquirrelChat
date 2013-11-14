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

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <errno.h>
#include <string.h>

#ifdef WITH_SSL
#include <gnutls/gnutls.h>
#endif

#include "commands.h"
#include "message_parser.h"
#include "ui/chat_window.h"
#include "numerics.h"
#include "errors.h"
#include "settings.h"

int main(int argc, char *argv[]) {
    sqchat_init_irc_commands();
    sqchat_init_msg_parser();
    sqchat_init_numerics();
#ifdef WITH_SSL
    gnutls_global_init();

#if GNUTLS_DEBUG_LEVEL > 0
    gnutls_global_set_log_level(GNUTLS_DEBUG_LEVEL);
    gnutls_global_set_log_function(_sqchat_gnutls_debug_log);
#endif

#endif

    gtk_init(&argc, &argv);
    sqchat_init_settings();

    struct sqchat_chat_window * window = sqchat_chat_window_new(NULL);

    gtk_main();

    return 0;
}
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4:cinoptions=(0,W4
