#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <errno.h>
#include <string.h>

#include "ui/chat_window.h"

int main(int argc, char *argv[]) {

    gtk_init(&argc, &argv);

    struct chat_window * window = create_new_chat_window();

    gtk_main();

    return 0;
}
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
