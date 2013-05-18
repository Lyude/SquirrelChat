#include "commands.h"
#include "trie.h"
#include "ui/chat_window.h"
#include "ui/buffer.h"

#include <string.h>
#include <ctype.h>
#include <stdlib.h>

trie * command_trie;

// For testing, this will be removed
void test_command(struct buffer_info * buffer,
                  char * argv[],
                  char * trailing) {
    print_to_buffer(buffer,
                    "Command received!\n"
                    "argv: \"%s\"\n"
                    "trailing: \"%s\"\n",
                    argv[0], trailing);
}

// Converts a string to lowercase, used to canonize strings by the trie
void strtolower(char * s) {
    for (int i = 0; s[i] != '\0'; i++)
        s[i] = tolower(s[i]);
}

// Sets up the commands trie and adds the default client commands to said trie
void init_irc_commands() {
    command_trie = trie_new(strtolower);

    // Add default commands to command_trie
    add_irc_command("test", test_command, 1);
}

// Adds an IRC command
void add_irc_command(char * command,
                     irc_command_callback * callback,
                     unsigned short min_param_count) {
    struct irc_command_callback_info * info;
    info = malloc(sizeof(struct irc_command_callback_info));
    info->min_param_count = min_param_count;
    info->callback = callback;

    trie_set(command_trie, command, info);
}

// Removes an added IRC command
void del_irc_command(char * command) {
    struct irc_command_callback_info * info = trie_get(command_trie, command);
    free(info);
    trie_del(command_trie, command);
}


void call_command(struct buffer_info * buffer,
                  char * command,
                  char * params) {
    struct irc_command_callback_info * info = trie_get(command_trie, command);

    // Make sure the command exists
    if (info == NULL) {
        print_to_buffer(buffer, "Error: Unknown command \"%s\"\n", command);
        return;
    }

    char * argv[info->min_param_count];

    /* If no parameters were provided and the command requires some, return an
     * error
     */
    if (params == NULL && info->min_param_count != 0) {
        print_to_buffer(buffer, "Error: %s: Not enough parameters\n",
                        command);
        return;
    }

    unsigned short i;
    for (i = 0; i < info->min_param_count; i++) {
        char * param_end = strpbrk(params, " ");
        
        // Null terminate the parameter and add it to argv
        if (param_end == NULL) {
            argv[i] = params;
            params = NULL;
        }
        else {
            *param_end = '\0';
            argv[i] = params;

            // Eat everything up until the next parameter, or the end of the string
            for (params = param_end + 1; *params == ' '; ++params);
        }
    }
    info->callback(buffer, argv, params);
}

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
