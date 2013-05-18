#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#include "ui/buffer.h"
#include "ui/chat_window.h"

// irc_command_callback(chat_window, argv, trailing)
typedef void (irc_command_callback)(struct buffer_info *,
                                     char*[],
                                     char*);

struct irc_command_callback_info {
    unsigned short min_param_count;
    irc_command_callback * callback;
};

void init_irc_commands();

void add_irc_command(char * command,
                     irc_command_callback callback,
                     unsigned short param_count);
void del_irc_command(char * command);
void call_command(struct buffer_info * buffer,
                  char * command,
                  char * params);

#endif // __COMMANDS_H__
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
