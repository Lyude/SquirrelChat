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
#include <time.h>
#include <strings.h>

#include "chat.h"
#include "builtin_commands.h"
#include "irc_network.h"
#include "net_io.h"
#include "commands.h"
#include "ui/buffer.h"
#include "irc_numerics.h"
#include "cmd_responses.h"
#include "ctcp.h"

#define DEFAULT_AWAY_MSG "I am not here right now."

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
    add_irc_command("ctcp", cmd_ctcp, 2,
                    "/ctcp <target> <type> [arguments]",
                    "Sends a CTCP request of the specified type to the "
                    "target.\n");
    add_irc_command("me", cmd_me, 0,
                    "/me <action>",
                    "Sends an action to the current buffer. For example:\n"
                    "/me does the safety dance\n"
                    "Would send a message to the buffer that looks like:\n"
                    "* Foo does the safety dance\n"
                    "(replacing \"Foo\" with your nickname)\n");
    add_irc_command("whois", cmd_whois, 2,
                    "/whois [server] <user>",
                    "Queries the server for information on the specified user."
                    "The information can be queried from an alternate server "
                    "if one is specified.\n");
    add_irc_command("oper", cmd_oper, 2,
                    "/oper <name> <password>",
                    "Makes you an IRC operator, a god among men.\n"
                    "If authentication on your IRC server does not require "
                    "that you provide a plaintext password, you may replace "
                    "the password parameter with a literal '*'.\n");
    add_irc_command("whowas", cmd_whowas, 0,
                    "/whowas <nickname> *( \",\" <nickname> ) [ <count> [ <target> ] ]",
                    "Queries the server for the history of a user (nicknames "
                    "they used, addresses, etc.). If count is specified, only "
                    "that many records will be returned. If target is "
                    "specified, the results will be narrowed down to those "
                    "from said server.\n");
    add_irc_command("lusers", cmd_lusers, 2,
                    "/lusers [mask] [target]",
                    "Queries the server for information on it's size. If mask "
                    "is targeted, the results will only take into account the "
                    "users on the servers matching the mask. If target is "
                    "specified, the lusers command will be forwarded to the "
                    "target server.\n");
    add_irc_command("invite", cmd_invite, 2,
                    "/invite <user> <channel>",
                    "Sends an invitation to a user to join the channel you "
                    "specify.\n");
    add_irc_command("time", cmd_time, 1,
                    "/time [target]",
                    "Prints the current server's local time or if a target is "
                    "provided, the specified server's local time.\n");
    add_irc_command("version", cmd_version, 1,
                    "/version [target]",
                    "Prints the version of the ircd software the current "
                    "server is running or if a target is specified, the "
                    "version of the ircd that server is running.\n");
    add_irc_command("info", cmd_info, 1,
                    "/info [target]",
                    "Prints information regarding the software the current "
                    "server is running, or if a target is specified, the "
                    "information regarding the software that server is "
                    "running.\n"
                    "WARNING: On most servers, this command produces a ton of "
                    "output.\n");
    add_irc_command("away", cmd_away, 0,
                    "/away [message]",
                    "Sets your status as away with an optional message "
                    "accompanying it, so you can let people know you're not "
                    "at the computer when they send you a message.\n"
                    "If no message is specified, the default message "
                    "\"" DEFAULT_AWAY_MSG "\" will be used.\n");
    add_irc_command("back", cmd_back, 0,
                    "/back",
                    "Unsets your away status, if one is set.\n");
    add_irc_command("who", cmd_who, 2,
                    "/who <target> [o]",
                    "Gets information on a single user, or all the users in a "
                    "channel. If o is specified, the command filters it's "
                    "results to only include operators.\n");
    add_irc_command("links", cmd_links, 2,
                    "/links [remote server] [server mask]",
                    "Lists all the servers linked up the current server. If "
                    "[server mask] is specified, the results are filtered to "
                    "servers matching the mask. If [remote server] is "
                    "given along with [server mask], the command s forwarded "
                    "to the first server matching [remote server] and "
                    "filtered to only include servers matching the mask.\n");
    add_irc_command("list", cmd_list, 2,
                    "/list [channels] [server]",
                    "Shows the list of channels on the current server or the "
                    "specified server. If the channels argument is used, the "
                    "output of the command is limited to only showing the "
                    "information on those specific channels.\n");
    add_irc_command("kick", cmd_kick, 1,
                    "/kick [channel] <user> [reason]",
                    "Kicks a user from the specified channel (or the current "
                    "channel, if none is specified), with the option of "
                    "providing a reason for the kick.\n");
    add_irc_command("kill", cmd_kill, 1,
                    "/kill <user> <reason>",
                    "Forcefully disconnects a user from the server you are "
                    "connected to with the reason you specify.\n"
                    "On most networks, this will only work if you are an IRC "
                    "operator on the network.\n");
}

#define BI_CMD(func_name)                       \
    short func_name(struct buffer_info * buffer,\
                    unsigned short argc,        \
                    char * argv[],              \
                    char * trailing)

BI_CMD(cmd_help) {
    if (argc < 1)
        return 0; // FIXME: printing the syntax for help segfaults
    print_command_help(buffer, argv[0]);
    return 0;
}

// Max argc: 1
BI_CMD(cmd_nick) {
    if (argc < 1)
        return IRC_CMD_SYNTAX_ERR;
    else if (buffer->network->nickname &&
             strcmp(buffer->network->nickname, argv[0]) == 0)
        return 0;

    //TODO: Add code to check the length of the nickname
    if (buffer->network->status != DISCONNECTED) {
        send_to_network(buffer->network, "NICK %s\r\n", argv[0]);
        claim_response(buffer->network, buffer, strdup(argv[0]), free);
    }
    else {
        free(buffer->network->nickname);
        buffer->network->nickname = strdup(argv[0]);
        print_to_buffer(buffer, "* You are now known as %s.\n", argv[0]);
    }
    return 0;
}

// Max argc: 1
BI_CMD(cmd_server) {
    if (argc < 1)
        return IRC_CMD_SYNTAX_ERR;

    char * saveptr;
    char * address = strtok_r(argv[0], ":", &saveptr);
    char * port = strtok_r(NULL, ":", &saveptr);

    free(buffer->network->address);
    free(buffer->network->port);
    buffer->network->address = strdup(address);
    buffer->network->port = strdup(port ? port : "6667");

    print_to_buffer(buffer, "Server set to %s:%s\n",
                    buffer->network->address,
                    buffer->network->port);
    return 0;
}

// Max argc: 1
BI_CMD(cmd_msg) {
    if (argc < 1)
        return IRC_CMD_SYNTAX_ERR;
    else if (buffer->network->status != CONNECTED)
        print_to_buffer(buffer, "Not connected!\n");
    // TODO: Add support for sending messages > 512 chars
    else if (strlen(trailing) > IRC_MSG_LEN -
                           (strlen(buffer->network->nickname) +
                            sizeof(" :")))
        print_to_buffer(buffer, "Message too long!\n");
    else
        send_privmsg(buffer->network, argv[0], trailing);
    return 0;
}

BI_CMD(cmd_notice) {
    if (argc < 1)
        return IRC_CMD_SYNTAX_ERR;
    else if (buffer->network->status != CONNECTED)
        print_to_buffer(buffer, "Not connected!\n");
    // TODO: add support for sending notices > 512 chars
    else if (strlen(trailing) > IRC_MSG_LEN -
                                (strlen(buffer->network->nickname) +
                                sizeof(" :")))
        print_to_buffer(buffer, "Notice too long!\n");
    else
        send_to_network(buffer->network, "NOTICE %s :%s\r\n",
                        argv[0], trailing);
    return 0;
}

// Max argc: 1
/* TODO: Possibly allow passwords to be specified as the second parameter if
 * only one channel is specified
 */
BI_CMD(cmd_join) {
    if (argc < 1)
        return IRC_CMD_SYNTAX_ERR;
    else if (buffer->network->status == CONNECTED)
        send_to_network(buffer->network, "JOIN %s\r\n",
                        argv[0]);
    else
        print_to_buffer(buffer, "Not connected!\n");
    return 0;
}

// Max argc: 1
BI_CMD(cmd_part) {
    if (argc < 1) {
        if (buffer->type == CHANNEL)
            send_to_network(buffer->network, "PART %s\r\n",
                            buffer->buffer_name);
        else
            print_to_buffer(buffer, "You're not in a channel!\n");
    }
    else if (buffer->network->status != CONNECTED)
        print_to_buffer(buffer, "Not connected!\n");
    else
        send_to_network(buffer->network, "PART %s :%s\r\n",
                        argv[0], trailing ? trailing : "");
    return 0;
}

// Max argc: 0
BI_CMD(cmd_topic) {
    if (buffer->network->status != CONNECTED) {
        print_to_buffer(buffer, "Not connected!\n");
        return 0;
    }

    // Check if the user specified any parameters
    if (trailing == NULL) {
        if (buffer->type == CHANNEL) {
            send_to_network(buffer->network, "TOPIC %s\r\n",
                            buffer->buffer_name);
            claim_response(buffer->network, buffer, NULL, NULL);
        }
        else
            print_to_buffer(buffer, "You're not in a channel!\n");
    }
    else {
        char * channel;
        if (strchr(buffer->network->chantypes, trailing[0]) != NULL)
            channel = strtok_r(trailing, " ", &trailing);
        else {
            if (buffer->type != CHANNEL) {
                print_to_buffer(buffer, "You're not in a channel!\n");
                return 0;
            }
            channel = buffer->buffer_name;
        }
        if (*trailing == '\0') {
            send_to_network(buffer->network, "TOPIC %s\n",
                            channel);
            claim_response(buffer->network, buffer, NULL, NULL);
        }
        else
            send_to_network(buffer->network, "TOPIC %s :%s\r\n", channel,
                            trailing);
    }
    return 0;
}

// Max argc: 0
BI_CMD(cmd_connect) {
    if (buffer->network->status == DISCONNECTED)
        connect_irc_network(buffer->network);
    return 0;
}

// Max argc: 0
BI_CMD(cmd_quit) {
    if (buffer->network->status == DISCONNECTED)
        print_to_buffer(buffer, "Not connected!\n");
    else
        disconnect_irc_network(buffer->network, trailing);
    return 0;
}

BI_CMD(cmd_quote) {
    if (buffer->network->status == DISCONNECTED)
        print_to_buffer(buffer, "Not connected!\n");
    else if (trailing != NULL)
        send_to_network(buffer->network, "%s\r\n", trailing);
    else
        return IRC_CMD_SYNTAX_ERR;
    return 0;
}

BI_CMD(cmd_motd) {
    if (buffer->network->status != CONNECTED) {
        print_to_buffer(buffer, "Not connected!\n");
        return 0;
    }

    claim_response(buffer->network, buffer, NULL, NULL);
    send_to_network(buffer->network, "MOTD %s\r\n",
                    (argc >= 1) ? argv[0] : "");
    return 0;
}

BI_CMD(cmd_mode) {
    if (buffer->network->status != CONNECTED) {
        print_to_buffer(buffer, "Not connected!\n");
        return 0;
    }

    if (argc == 0) {
        if (buffer->type == NETWORK)
            send_to_network(buffer->network,
                            "MODE %s\r\n", buffer->network->nickname);
        else if (buffer->type == CHANNEL)
            send_to_network(buffer->network,
                        "MODE %s\r\n", buffer->buffer_name);
        else
            return IRC_CMD_SYNTAX_ERR;
    }

    // Check if the user explicitly specified a target
    else if (strchr(buffer->network->chantypes, *(argv[0])) ||
             buffer->network->casecmp(buffer->network->nickname,
                                             argv[0]) == 0)
        send_to_network(buffer->network,
                        "MODE %s %s\r\n", argv[0], trailing ? trailing : "");
    else {
        if (buffer->type == NETWORK)
            send_to_network(buffer->network,
                            "MODE %s %s %s\r\n",
                            buffer->network->nickname, argv[0],
                            trailing ? trailing : "");
        else if (buffer->type == CHANNEL)
            send_to_network(buffer->network,
                            "MODE %s %s %s\r\n", buffer->buffer_name, argv[0],
                            trailing ? trailing : "");
        else
            return IRC_CMD_SYNTAX_ERR;
    }
    claim_response(buffer->network, buffer, NULL, NULL);
    return 0;
}

BI_CMD(cmd_ctcp) {
    if (buffer->network->status != CONNECTED) {
        print_to_buffer(buffer, "Not connected!\n");
        return 0;
    }

    // TODO: Choose the target automatically for query buffers
    if (argc < 2)
        return IRC_CMD_SYNTAX_ERR;

    /* If the ctcp being sent is a PING, we need to record the time the command
     * was sent
     */
    if (strcasecmp(argv[1], "PING") == 0) {
        struct timespec current_time;
        clock_gettime(CLOCK_REALTIME, &current_time);
        sendf_ctcp(buffer->network, argv[0], "PING", "%li",
                   (current_time.tv_sec * 1000000000) + current_time.tv_nsec);
    }
    else {
        if (trailing == NULL)
            send_ctcp(buffer->network, argv[0], argv[1]);
        else
            sendf_ctcp(buffer->network, argv[0], argv[1], "%s", trailing);
    }

    print_to_buffer(buffer, "* Sent a CTCP %s to %s.\n", argv[1], argv[0]);
    return 0;
}

BI_CMD(cmd_me) {
    if (buffer->network->status != CONNECTED)
        print_to_buffer(buffer, "Not connected!\n");
    else if (trailing == NULL)
        return IRC_CMD_SYNTAX_ERR;
    else if (buffer->type == NETWORK)
        print_to_buffer(buffer, "You can't send messages to this buffer.\n");
    else {
        sendf_ctcp(buffer->network, buffer->buffer_name, "ACTION", "%s",
                   trailing);
        print_to_buffer(buffer, "* %s %s\n", buffer->network->nickname,
                        trailing);
    }
    return 0;
}

BI_CMD(cmd_whois) {
    if (buffer->network->status != CONNECTED)
        print_to_buffer(buffer, "Not connected!\n");
    else if (argc == 0)
        return IRC_CMD_SYNTAX_ERR;
    else {
        if (argc == 1)
            send_to_network(buffer->network, "WHOIS %s\r\n", argv[0]);
        else
            send_to_network(buffer->network, "WHOIS %s %s\r\n", argv[0], argv[1]);
        claim_response(buffer->network, buffer, NULL, NULL);
    }
    return 0;
}

BI_CMD(cmd_oper) {
    if (buffer->network->status != CONNECTED)
        print_to_buffer(buffer, "Not connected!\n");
    else if (argc < 2)
        return IRC_CMD_SYNTAX_ERR;
    else {
        send_to_network(buffer->network, "OPER %s %s\r\n", argv[0], argv[1]);
        claim_response(buffer->network, buffer, NULL, NULL);
    }
    return 0;
}

BI_CMD(cmd_whowas) {
    if (trailing == NULL)
        return IRC_CMD_SYNTAX_ERR;

    send_to_network(buffer->network, "WHOWAS %s\r\n", trailing);
    claim_response(buffer->network, buffer, NULL, NULL);
    return 0;
}

BI_CMD(cmd_lusers) {
    if (buffer->network->status != CONNECTED) {
        print_to_buffer(buffer, "Not connected!\n");
        return 0;
    }

    if (argc == 0)
        send_to_network(buffer->network, "LUSERS\r\n");
    else if (argc == 1)
        send_to_network(buffer->network, "LUSERS %s\r\n", argv[0]);
    else
        send_to_network(buffer->network, "LUSERS %s %s\r\n", argv[0], argv[1]);
    claim_response(buffer->network, buffer, NULL, NULL);
    return 0;
}

BI_CMD(cmd_invite) {
    if (argc < 2)
        return IRC_CMD_SYNTAX_ERR;

    if (buffer->network->status != CONNECTED) {
        print_to_buffer(buffer, "Not connected!\n");
        return 0;
    }

    send_to_network(buffer->network, "INVITE %s %s\r\n", argv[0], argv[1]);
    claim_response(buffer->network, buffer, NULL, NULL);
    return 0;
}

BI_CMD(cmd_time) {
    if (buffer->network->status != CONNECTED)
        print_to_buffer(buffer, "Not connected!\n");
    else {
        if (argc == 0)
            send_to_network(buffer->network, "TIME\r\n");
        else
            send_to_network(buffer->network, "TIME %s\r\n", argv[0]);
        claim_response(buffer->network, buffer, NULL, NULL);
    }
    return 0;
}

BI_CMD(cmd_version) {
    if (buffer->network->status != CONNECTED)
        print_to_buffer(buffer, "Not connected!\n");
    else {
        if (argc == 0)
            send_to_network(buffer->network, "VERSION\r\n");
        else
            send_to_network(buffer->network, "VERSION %s\r\n", argv[0]);
        claim_response(buffer->network, buffer, NULL, NULL);
    }
    return 0;
}

BI_CMD(cmd_info) {
    if (buffer->network->status != CONNECTED)
        print_to_buffer(buffer, "Not connected!\n");
    else {
        if (argc == 0) {
            send_to_network(buffer->network, "INFO\r\n");
            print_to_buffer(buffer, "--- Showing INFO for %s ---\n",
                            buffer->network->server_name);
        }
        else {
            send_to_network(buffer->network, "INFO %s\r\n", argv[0]);
            print_to_buffer(buffer, "--- Showing INFO for %s ---\n", argv[0]);
        }
        claim_response(buffer->network, buffer, NULL, NULL);
    }
    return 0;
}

BI_CMD(cmd_away) {
    if (buffer->network->status != CONNECTED)
        print_to_buffer(buffer, "Not connected!\n");
    else {
        send_to_network(buffer->network, "AWAY %s\r\n",
                        trailing ? trailing : DEFAULT_AWAY_MSG);
        claim_response(buffer->network, buffer, NULL, NULL);
    }
    return 0;
}

BI_CMD(cmd_back) {
    if (buffer->network->status != CONNECTED)
        print_to_buffer(buffer, "Not connected!\n");
    else {
        send_to_network(buffer->network, "AWAY\r\n");
        claim_response(buffer->network, buffer, NULL, NULL);
    }
    return 0;
}

BI_CMD(cmd_who) {
    if (argc < 1)
        return IRC_CMD_SYNTAX_ERR;
    if (buffer->network->status != CONNECTED)
        print_to_buffer(buffer, "Not connected!\n");
    else {
        if (argc == 1)
            send_to_network(buffer->network, "WHO %s\r\n", argv[0]);
        else
            send_to_network(buffer->network, "WHO %s %s\r\n", argv[0], argv[1]);

        if (IRC_IS_CHAN(buffer->network, argv[0]))
            print_to_buffer(buffer,
                            "--- Beginning of WHO for %s ---\n", argv[0]);
        claim_response(buffer->network, buffer, NULL, NULL);
    }
    return 0;
}

BI_CMD(cmd_links) {
    if (buffer->network->status != CONNECTED)
        print_to_buffer(buffer, "Not connected!\n");
    else {
        if (argc == 0) {
            print_to_buffer(buffer, "--- Showing LINKS for %s ---\n",
                            buffer->network->server_name);
            send_to_network(buffer->network, "LINKS\r\n");
        }
        else if (argc == 1) {
            print_to_buffer(buffer, "--- Showing LINKS for %s ---\n",
                            buffer->network->server_name);
            send_to_network(buffer->network, "LINKS %s\r\n", argv[0]);
        }
        else {
            print_to_buffer(buffer, "--- Showing LINKS for %s ---\n", argv[0]);
            send_to_network(buffer->network, "LINKS %s %s\r\n", argv[0], argv[1]);
        }
        claim_response(buffer->network, buffer, NULL, NULL);
    }
    return 0;
}

// TODO: Display a physical window for the LIST command
BI_CMD(cmd_list) {
    if (buffer->network->status != CONNECTED)
        print_to_buffer(buffer, "Not connected!\n");
    else {
        if (argc == 0)
            send_to_network(buffer->network, "LIST\r\n");
        else if (argc == 1)
            send_to_network(buffer->network, "LIST %s\r\n", argv[0]);
        else
            send_to_network(buffer->network, "LIST %s %s\r\n",
                            argv[0], argv[1]);
        claim_response(buffer->network, buffer, NULL, NULL);
    }
    return 0;
}

BI_CMD(cmd_kick) {
    if (buffer->network->status != CONNECTED) {
        print_to_buffer(buffer, "Not connected!\n");
        return 0;
    }
    if (argc < 1)
        return IRC_CMD_SYNTAX_ERR;
    
    if (trailing == NULL) {
        if (buffer->type != CHANNEL)
            return IRC_CMD_SYNTAX_ERR;

        send_to_network(buffer->network, "KICK %s %s\r\n",
                        buffer->buffer_name, argv[0]);
    }
    else {
        // Check if the first argument was a username or a channel
        if (IRC_IS_CHAN(buffer->network, argv[0])) {
            char * saveptr;
            char * arg2 = strtok_r(trailing, " ", &saveptr);
            if (arg2 == NULL)
                send_to_network(buffer->network, "KICK %s %s\r\n",
                                argv[0], trailing);
            else
                send_to_network(buffer->network, "KICK %s %s :%s\r\n",
                                argv[0], arg2, saveptr + strspn(saveptr, " "));
        }
        else {
            if (buffer->type != CHANNEL)
                return IRC_CMD_SYNTAX_ERR;

            send_to_network(buffer->network, "KICK %s %s :%s\r\n",
                            buffer->buffer_name, argv[0], trailing);
        }
    }

    claim_response(buffer->network, buffer, NULL, NULL);

    return 0;
}

BI_CMD(cmd_kill) {
    if (argc < 1 || trailing == NULL)
        return IRC_CMD_SYNTAX_ERR;
    if (buffer->network->status != CONNECTED) {
        print_to_buffer(buffer, "Not connected!\n");
        return 0;
    }

    send_to_network(buffer->network, "KILL %s :%s\r\n", argv[0], trailing);
    claim_response(buffer->network, buffer, NULL, NULL);
    return 0;
}

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
