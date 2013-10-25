/* Contains various functions for handling errors
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

#include "errors.h"
#include "ui/buffer.h"

void sqchat_dump_msg_to_buffer(struct sqchat_buffer * buffer,
                               char * hostmask,
                               short argc,
                               char * argv[]) {
    sqchat_buffer_print(buffer,
                        "Received from: \"%s\"\n"
                        "Args: [ ",
                        hostmask);
    for (short i = 0; i < argc; i++)
        sqchat_buffer_print(buffer, "\"%s\", ", argv[i]);
    sqchat_buffer_print(buffer, " ]\n");
}

#if GNUTLS_DEBUG_LEVEL > 0
void _sqchat_gnutls_debug_log(int level, const char * msg) {
    printf("*** GnuTLS Debug (%i): %s",
           level, msg);
}
#endif

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
