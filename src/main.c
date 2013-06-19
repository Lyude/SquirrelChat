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

#include "commands.h"
#include "message_parser.h"
#include "ui/chat_window.h"
#include "numerics.h"
#include "message_types.h"

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    struct chat_window * window = create_new_chat_window(NULL);

    gtk_main();

    return 0;
}
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
