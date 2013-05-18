#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <errno.h>
#include <string.h>

#include "commands.h"
#include "ui/chat_window.h"

int main(int argc, char *argv[]) {

    init_irc_commands();

    gtk_init(&argc, &argv);

    struct chat_window * window = create_new_chat_window(NULL);

    gtk_main();

    return 0;
}
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
