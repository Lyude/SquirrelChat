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

#include <string.h>
#include <stdlib.h>

#include "chat.h"
#include "builtin_commands.h"
#include "irc_network.h"
#include "net_io.h"
#include "commands.h"
#include "ui/buffer.h"
#include "irc_numerics.h"
#include "cmd_responses.h"

void add_builtin_commands() {
    add_irc_command("help", cmd_help, 1,
                    "/help <command>",
                    "Prints a description of a command\n");
    add_irc_command("nick", cmd_nick, 1,
                    "/nick <new_nickname>",
                    "Changes your nickname.\n");
    add_irc_command("server", cmd_server, 1,
                    "/server <address>[:[+]<port>]",
                    "Sets the server for the current buffer.\n"
                    "A + in the port number indicates the server uses SSL.\n");
    add_irc_command("msg", cmd_msg, 1,
                    "/msg <recepient>",
                    "Sends a message to a person or a channel.\n");
    add_irc_command("join", cmd_join, 1,
                    "/join #channel[;<password>][<,>...]",
                    "Join a list of channels. Each channel is seperate with a "
                    "comma. If the channel has a password, you can insert it "
                    "after the channel name by adding a semi-colon and a "
                    "space (\"; \"), followed by the password. For example:\n"
                    "/join #foobar <-- joins the channel #foobar\n"
                    "/join #foo,#bar,#moo <-- joins the channels #foo, #bar, "
                    "and #moo\n"
                    "/join #foo; bar <-- joins the channel #foo with password bar\n"
                    "/join #foobar,#moo; cow <-- joins the channel #foobar with "
                    "no password and also joins the channel #moo with the "
                    "password \"cow\".");
    add_irc_command("part", cmd_part, 1,
                    "/part [channel[,...]] [part_message]",
                    "Parts the current channel or a comma-seperated list of "
                    "channels, optionally with a message.\n");
    add_irc_command("connect", cmd_connect, 0,
                    "/connect",
                     "Starts a connection with the IRC server for the current"
                     "buffer.\n");
    add_irc_command("quit", cmd_quit, 0,
                    "/quit [quit message]",
                     "Disconnects the current buffer, with an optional quit "
                     "message.\n");
    add_irc_command("quote", cmd_quote, 0,
                    "/quote <message>",
                    "Sends a raw message to the server.\n");
    add_irc_command("motd", cmd_motd, 0,
                    "/motd [server]",
                    "Prints the MOTD for server, or the current server if "
                    "server is omitted.\n");
    add_irc_command("topic", cmd_topic, 0,
                    "/topic [channel] [new topic]",
                    "Displays/sets the topic of the current channel, or "
                    "displays/sets the topic of another channel.\n");
    add_irc_command("notice", cmd_notice, 1,
                    "/notice <target> <notice>",
                    "Sends a notice to target.\n");
    add_irc_command("mode", cmd_mode, 1,
                    "/mode [target] [modes] [mode arguments]",
                    "Changes or displays the mode of a user or a channel.\n");
}

short cmd_help(struct buffer_info * buffer,
               unsigned short argc,
               char * argv[],
               char * trailing) {
    if (argc < 1)
        return 0; // FIXME: printing the syntax for help segfaults
    print_command_help(buffer, argv[0]);
    return 0;
}

// Max argc: 1
short cmd_nick(struct buffer_info * buffer,
               unsigned short argc,
               char * argv[],
               char * trailing) {
    if (argc < 1)
        return IRC_CMD_SYNTAX_ERR;
    else if (strcmp(buffer->parent_network->nickname, argv[0]) == 0)
        return 0;

    //TODO: Add code to check the length of the nickname
    if (buffer->parent_network->status != DISCONNECTED) {
        send_to_network(buffer->parent_network, "NICK %s\r\n", argv[0]);
        claim_response(buffer->parent_network, buffer, strdup(argv[0]), free);
    }
    else {
        free(buffer->parent_network->nickname);
        buffer->parent_network->nickname = strdup(argv[0]);
    }
    return 0;
}

// Max argc: 1
short cmd_server(struct buffer_info * buffer,
                 unsigned short argc,
                 char * argv[],
                 char * trailing) {
    if (argc < 1)
        return IRC_CMD_SYNTAX_ERR;

    char * saveptr;
    char * address = strtok_r(argv[0], ":", &saveptr);
    char * port = strtok_r(NULL, ":", &saveptr);

    free(buffer->parent_network->address);
    free(buffer->parent_network->port);
    buffer->parent_network->address = strdup(address);
    buffer->parent_network->port = strdup(port ? port : "6667");

    print_to_buffer(buffer, "Server set to %s:%s\n",
                    buffer->parent_network->address,
                    buffer->parent_network->port);
    return 0;
}

// Max argc: 1
short cmd_msg(struct buffer_info * buffer,
              unsigned short argc,
              char * argv[],
              char * trailing) {
    if (argc < 1)
        return IRC_CMD_SYNTAX_ERR;
    else if (buffer->parent_network->status != CONNECTED)
        print_to_buffer(buffer, "Not connected!\n");
    // TODO: Add support for sending messages > 512 chars
    else if (strlen(trailing) > IRC_MSG_LEN - 
                           (strlen(buffer->parent_network->nickname) + 
                            sizeof(" :")))
        print_to_buffer(buffer, "Message too long!\n");
    else
        send_privmsg(buffer->parent_network, argv[0], trailing);
    return 0;
}

short cmd_notice(struct buffer_info * buffer,
                 unsigned short argc,
                 char * argv[],
                 char * trailing) {
    if (argc < 1)
        return IRC_CMD_SYNTAX_ERR;
    else if (buffer->parent_network->status != CONNECTED)
        print_to_buffer(buffer, "Not connected!\n");
    // TODO: add support for sending notices > 512 chars
    else if (strlen(trailing) > IRC_MSG_LEN -
                                (strlen(buffer->parent_network->nickname) +
                                sizeof(" :")))
        print_to_buffer(buffer, "Notice too long!\n");
    else
        send_to_network(buffer->parent_network, "NOTICE %s :%s\r\n",
                        argv[0], trailing);
    return 0;
}

// Max argc: 1
/* TODO: Possibly allow passwords to be specified as the second parameter if
 * only one channel is specified
 */
short cmd_join(struct buffer_info * buffer,
               unsigned short argc,
               char * argv[],
               char * trailing) {
    if (argc < 1)
        return IRC_CMD_SYNTAX_ERR;
    else if (buffer->parent_network->status == CONNECTED)
        send_to_network(buffer->parent_network, "JOIN %s\r\n",
                        argv[0]);
    else
        print_to_buffer(buffer, "Not connected!\n");
    return 0;
}

// Max argc: 1
short cmd_part(struct buffer_info * buffer,
               unsigned short argc,
               char * argv[],
               char * trailing) {
    if (argc < 1) {
        if (buffer->type == CHANNEL)
            send_to_network(buffer->parent_network, "PART %s\r\n",
                            buffer->buffer_name);
        else
            print_to_buffer(buffer, "You're not in a channel!\n");
    }
    else if (buffer->parent_network->status != CONNECTED)
        print_to_buffer(buffer, "Not connected!\n");
    else
        send_to_network(buffer->parent_network, "PART %s :%s\r\n",
                        argv[0], trailing ? trailing : "");
    return 0;
}

// Max argc: 0
short cmd_topic(struct buffer_info * buffer,
                unsigned short argc,
                char * argv[],
                char * trailing) {
    if (buffer->parent_network->status != CONNECTED) {
        print_to_buffer(buffer, "Not connected!\n");
        return 0;
    }

    // Check if the user specified any parameters
    if (trailing == NULL) {
        if (buffer->type == CHANNEL) {
            send_to_network(buffer->parent_network, "TOPIC %s\r\n",
                            buffer->buffer_name);
            claim_response(buffer->parent_network, buffer, NULL, NULL);
        }
        else
            print_to_buffer(buffer, "You're not in a channel!\n");
    }
    else {
        char * channel;
        if (strchr(buffer->parent_network->chantypes, trailing[0]) != NULL)
            channel = strtok_r(trailing, " ", &trailing);
        else {
            if (buffer->type != CHANNEL) {
                print_to_buffer(buffer, "You're not in a channel!\n");
                return 0;
            }
            channel = buffer->buffer_name;
        }
        if (*trailing == '\0') {
            send_to_network(buffer->parent_network, "TOPIC %s\n",
                            channel);
            claim_response(buffer->parent_network, buffer, NULL, NULL);
        }
        else
            send_to_network(buffer->parent_network, "TOPIC %s :%s\r\n", channel,
                            trailing);
    }
    return 0;
}

// Max argc: 0
short cmd_connect(struct buffer_info * buffer,
                  unsigned short argc,
                  char * argv[],
                  char * trailing) {
    if (buffer->parent_network->status == DISCONNECTED)
        connect_irc_network(buffer->parent_network);
    return 0;
}

// Max argc: 0
short cmd_quit(struct buffer_info * buffer,
               unsigned short argc,
               char * argv[],
               char * trailing) {
    if (buffer->parent_network->status == DISCONNECTED)
        print_to_buffer(buffer, "Not connected!\n");
    else
        disconnect_irc_network(buffer->parent_network, trailing);
    return 0;
}

short cmd_quote(struct buffer_info * buffer,
                unsigned short argc,
                char * argv[],
                char * trailing) {
    if (buffer->parent_network->status == DISCONNECTED)
        print_to_buffer(buffer, "Not connected!\n");
    else if (trailing != NULL)
        send_to_network(buffer->parent_network, "%s\r\n", trailing);
    else
        return IRC_CMD_SYNTAX_ERR;
    return 0;
}

short cmd_motd(struct buffer_info * buffer,
               unsigned short argc,
               char * argv[],
               char * trailing) {
    if (buffer->parent_network->status != CONNECTED) {
        print_to_buffer(buffer, "Not connected!\n");
        return 0;
    }

    claim_response(buffer->parent_network, buffer, NULL, NULL);
    send_to_network(buffer->parent_network, "MOTD %s\r\n",
                    (argc >= 1) ? argv[0] : "");
    return 0;
}

short cmd_mode(struct buffer_info * buffer,
               unsigned short argc,
               char * argv[],
               char * trailing) {
    if (buffer->parent_network->status != CONNECTED) {
        print_to_buffer(buffer, "Not connected!\n");
        return 0;
    }

    if (argc == 0) {
        if (buffer->type == NETWORK)
            send_to_network(buffer->parent_network,
                            "MODE %s\r\n", buffer->parent_network->nickname);
        else if (buffer->type == CHANNEL)
            send_to_network(buffer->parent_network,
                        "MODE %s\r\n", buffer->buffer_name);
        else
            return IRC_CMD_SYNTAX_ERR;
    }

    // Check if the user explicitly specified a target
    else if (strchr(buffer->parent_network->chantypes, *(argv[0])) ||
             buffer->parent_network->casecmp(buffer->parent_network->nickname,
                                             argv[0]) == 0)
        send_to_network(buffer->parent_network,
                        "MODE %s %s\r\n", argv[0], trailing ? trailing : "");
    else {
        if (buffer->type == NETWORK)
            send_to_network(buffer->parent_network,
                            "MODE %s %s %s\r\n",
                            buffer->parent_network->nickname, argv[0],
                            trailing ? trailing : "");
        else if (buffer->type == CHANNEL)
            send_to_network(buffer->parent_network,
                            "MODE %s %s %s\r\n", buffer->buffer_name, argv[0],
                            trailing ? trailing : "");
        else
            return IRC_CMD_SYNTAX_ERR;
    }
    claim_response(buffer->parent_network, buffer, NULL, NULL);
    return 0;
}

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
