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

void sqchat_add_builtin_commands() {
    sqchat_add_irc_command("help", sqchat_cmd_help, 1,
                           "/help <command>",
                           "Prints a description of a command\n");
    sqchat_add_irc_command("nick", sqchat_cmd_nick, 1,
                           "/nick <new_nickname>",
                           "Changes your nickname.\n");
    sqchat_add_irc_command("server", sqchat_cmd_server, 1,
                           "/server <address>[:[+]<port>] [password]",
                           "Sets the server for the current buffer.\n"
                           "A + in the port number indicates the server uses "
                           "SSL.\n");
    sqchat_add_irc_command("msg", sqchat_cmd_msg, 1,
                           "/msg <recepient>",
                           "Sends a message to a person or a channel.\n");
    sqchat_add_irc_command("join", sqchat_cmd_join, 1,
                           "/join #channel[;<password>][<,>...]",
                           "Join a list of channels. Each channel is seperate "
                           "with a comma. If the channel has a password, you "
                           "can insert it after the channel name by adding a "
                           "semi-colon and a space (\";\"), followed by the "
                           "password. For example:\n"
                           "/join #foobar <-- joins the channel #foobar\n"
                           "/join #foo,#bar,#moo <-- joins the channels #foo, "
                           "#bar, and #moo\n"
                           "/join #foo; bar <-- joins the channel #foo with "
                           "password bar\n"
                           "/join #foobar,#moo; cow <-- joins the channel "
                           "#foobar with no password and also joins the "
                           "channel #moo with the password \"cow\".");
    sqchat_add_irc_command("part", sqchat_cmd_part, 1,
                           "/part [channel[,...]] [part_message]",
                           "Parts the current channel or a comma-seperated "
                           "list of channels, optionally with a message.\n");
    sqchat_add_irc_command("connect", sqchat_cmd_connect, 0,
                           "/connect",
                           "Starts a connection with the IRC server for the "
                           "current buffer.\n");
    sqchat_add_irc_command("quit", sqchat_cmd_quit, 0,
                           "/quit [quit message]",
                           "Disconnects the current buffer, with an optional "
                           "quit message.\n");
    sqchat_add_irc_command("quote", sqchat_cmd_quote, 0,
                           "/quote <message>",
                           "Sends a raw message to the server.\n");
    sqchat_add_irc_command("motd", sqchat_cmd_motd, 0,
                           "/motd [server]",
                           "Prints the MOTD for server, or the current server "
                           "if server is omitted.\n");
    sqchat_add_irc_command("topic", sqchat_cmd_topic, 0,
                           "/topic [channel] [new topic]",
                           "Displays/sets the topic of the current channel, or "
                           "displays/sets the topic of another channel.\n");
    sqchat_add_irc_command("notice", sqchat_cmd_notice, 1,
                           "/notice <target> <notice>",
                           "Sends a notice to target.\n");
    sqchat_add_irc_command("mode", sqchat_cmd_mode, 1,
                           "/mode [target] [modes] [mode arguments]",
                           "Changes or displays the mode of a user or a "
                           "channel.\n");
    sqchat_add_irc_command("ctcp", sqchat_cmd_ctcp, 2,
                           "/ctcp <target> <type> [arguments]",
                           "Sends a CTCP request of the specified type to the "
                           "target.\n");
    sqchat_add_irc_command("me", sqchat_cmd_me, 0,
                           "/me <action>",
                           "Sends an action to the current buffer. For "
                           "example:\n"
                           "/me does the safety dance\n"
                           "Would send a message to the buffer that looks "
                           "like:\n"
                           "* Foo does the safety dance\n"
                           "(replacing \"Foo\" with your nickname)\n");
    sqchat_add_irc_command("whois", sqchat_cmd_whois, 2,
                           "/whois [server] <user>",
                           "Queries the server for information on the "
                           "specified user."
                           "The information can be queried from an alternate "
                           "server if one is specified.\n");
    sqchat_add_irc_command("oper", sqchat_cmd_oper, 2,
                           "/oper <name> <password>",
                           "Makes you an IRC operator, a god among men.\n"
                           "If authentication on your IRC server does not "
                           "require that you provide a plaintext password, you "
                           "may replace the password parameter with a literal "
                           "'*'.\n");
    sqchat_add_irc_command("whowas", sqchat_cmd_whowas, 0,
                           "/whowas <nickname> *( \",\" <nickname> ) "
                           "[ <count> [ <target> ] ]",
                           "Queries the server for the history of a user "
                           "(nicknames they used, addresses, etc.). If count "
                           "is specified, only that many records will be "
                           "returned. If target is specified, the results will "
                           "be narrowed down to those from said server.\n");
    sqchat_add_irc_command("lusers", sqchat_cmd_lusers, 2,
                           "/lusers [mask] [target]",
                           "Queries the server for information on it's size. "
                           "If a mask is targeted, the results will only take "
                           "into account the users on the servers matching the "
                           "mask. If target is specified, the lusers command "
                           "will be forwarded to the target server.\n");
    sqchat_add_irc_command("invite", sqchat_cmd_invite, 2,
                           "/invite <user> <channel>",
                           "Sends an invitation to a user to join the channel "
                           "you specify.\n");
    sqchat_add_irc_command("time", sqchat_cmd_time, 1,
                           "/time [target]",
                           "Prints the current server's local time or if a "
                           "target is provided, the target server's local "
                           "time.\n");
    sqchat_add_irc_command("version", sqchat_cmd_version, 1,
                           "/version [target]",
                           "Prints the version of the ircd software the "
                           "current server is running or if a target is "
                           "specified, the version of the ircd that server is "
                           "running.\n");
    sqchat_add_irc_command("info", sqchat_cmd_info, 1,
                           "/info [target]",
                           "Prints information regarding the software the "
                           "current server is running, or if a target is "
                           "specified, the information regarding the software "
                           "that server is running.\n"
                           "WARNING: On most servers, this command produces a "
                           "lot of output.\n");
    sqchat_add_irc_command("away", sqchat_cmd_away, 0,
                           "/away [message]",
                           "Sets your status as away with an optional message "
                           "accompanying it, so you can let people know you're "
                           "not at the computer when they send you a message.\n"
                           "If no message is specified, the default message "
                           "\"" DEFAULT_AWAY_MSG "\" will be used.\n");
    sqchat_add_irc_command("back", sqchat_cmd_back, 0,
                           "/back",
                           "Unsets your away status, if one is set.\n");
    sqchat_add_irc_command("who", sqchat_cmd_who, 2,
                           "/who <target> [o]",
                           "Gets information on a single user, or all the "
                           "users in a channel. If o is specified, the command "
                           "filters it's results to only include operators.\n");
    sqchat_add_irc_command("links", sqchat_cmd_links, 2,
                           "/links [remote server] [server mask]",
                           "Lists all the servers linked up the current "
                           "server. If [server mask] is specified, the results "
                           "are filtered to servers matching the mask. If "
                           "[remote server] is given along with [server mask], "
                           "the command s forwarded to the first server "
                           "matching [remote server] and filtered to only "
                           "include servers matching the mask.\n");
    sqchat_add_irc_command("list", sqchat_cmd_list, 2,
                           "/list [channels] [server]",
                           "Shows the list of channels on the current server "
                           "or the specified server. If the channels argument "
                           "is used, the output of the command is limited to "
                           "only showing the information on those specific "
                           "channels.\n");
    sqchat_add_irc_command("kick", sqchat_cmd_kick, 1,
                           "/kick [channel] <user> [reason]",
                           "Kicks a user from the specified channel (or the "
                           "current channel, if none is specified), with the "
                           "option of providing a reason for the kick.\n");
    sqchat_add_irc_command("kill", sqchat_cmd_kill, 1,
                           "/kill <user> <reason>",
                           "Forcefully disconnects a user from the server you "
                           "are connected to with the reason you specify.\n"
                           "On most networks, this will only work if you are "
                           "an IRC operator on the network.\n");
    sqchat_add_irc_command("wallops", sqchat_cmd_wallops, 0,
                           "/wallops <message>",
                           "Sends a message to all online IRC operators. On "
                           "most networks this requires IRC operator "
                           "privileges.\n");
    sqchat_add_irc_command("trace", sqchat_cmd_trace, 0,
                           "/trace [server]",
                           "Shows a list of all the nodes connected to a "
                           "server. This includes users, leafs, etc. The "
                           "output of this command usually depends on your "
                           "privileges on the server.\n");
    sqchat_add_irc_command("username", sqchat_cmd_username, 1,
                           "/username <username>",
                           "Changes your default username for the current "
                           "network.\n"
                           "NOTE: Your username can only be set once per "
                           "connection, so using this command mid-connection "
                           "will not affect the current connection. Some "
                           "servers do however, have a command to change your "
                           "username without having to reconnect.\n");
    sqchat_add_irc_command("realname", sqchat_cmd_realname, 1,
                           "/realname <real name>",
                           "Changes your default real name for the current "
                           "network.\n"
                           "NOTE: Your real name can only be set once per "
                           "connection, so using this command mid-connection "
                           "will not affect the current connection. Some "
                           "servers do however, have a command to change your "
                           "real name without having to reconnect.\n");
}

#define BI_CMD(func_name)                           \
    short func_name(struct sqchat_buffer * buffer,  \
                    unsigned short argc,            \
                    char * argv[],                  \
                    char * trailing)

BI_CMD(sqchat_cmd_help) {
    if (argc < 1)
        return 0; // FIXME: printing the syntax for help segfaults
    sqchat_print_command_help(buffer, argv[0]);
    return 0;
}

// Max argc: 1
BI_CMD(sqchat_cmd_nick) {
    if (argc < 1)
        return SQCHAT_CMD_SYNTAX_ERR;
    else if (buffer->network->nickname &&
             strcmp(buffer->network->nickname, argv[0]) == 0)
        return 0;

    //TODO: Add code to check the length of the nickname
    if (buffer->network->status != DISCONNECTED) {
        sqchat_network_send(buffer->network, "NICK %s\r\n", argv[0]);
        sqchat_claim_response(buffer->network, buffer, strdup(argv[0]), free);
    }
    else {
        free(buffer->network->nickname);
        buffer->network->nickname = strdup(argv[0]);
        sqchat_buffer_print(buffer, "* You are now known as %s.\n", argv[0]);
    }
    return 0;
}

// Max argc: 1
BI_CMD(sqchat_cmd_server) {
    if (argc < 1)
        return SQCHAT_CMD_SYNTAX_ERR;

    char * saveptr;
    char * address = strtok_r(argv[0], ":", &saveptr);
    char * port = strtok_r(NULL, ":", &saveptr);

    free(buffer->network->address);
    free(buffer->network->port);
    free(buffer->network->password);
    buffer->network->address = strdup(address);

    if (port != NULL) {
        if (port[0] == '+') {
            port++;
            buffer->network->ssl = true;
        }
        else
            buffer->network->ssl = false;

        buffer->network->port = strdup(port);
    }
    else {
        buffer->network->ssl = false;
        buffer->network->port = strdup("6667");
    }

    if (trailing != NULL)
        buffer->network->password = strdup(trailing);
    else
        buffer->network->password = NULL;

    sqchat_buffer_print(buffer, "Server set to %s:%s\n",
                        buffer->network->address,
                        buffer->network->port);
    return 0;
}

// Max argc: 1
BI_CMD(sqchat_cmd_msg) {
    if (argc < 1 || trailing == NULL)
        return SQCHAT_CMD_SYNTAX_ERR;
    else if (buffer->network->status != CONNECTED)
        sqchat_buffer_print(buffer, "Not connected!\n");
    // TODO: Add support for sending messages > 512 chars
    else if (strlen(trailing) > SQCHAT_IRC_MSG_LEN -
                           (strlen(buffer->network->nickname) +
                            sizeof(" :")))
        sqchat_buffer_print(buffer, "Message too long!\n");
    else
        sqchat_send_privmsg(buffer->network, argv[0], trailing);
    return 0;
}

BI_CMD(sqchat_cmd_notice) {
    if (argc < 1)
        return SQCHAT_CMD_SYNTAX_ERR;
    else if (buffer->network->status != CONNECTED)
        sqchat_buffer_print(buffer, "Not connected!\n");
    // TODO: add support for sending notices > 512 chars
    else if (strlen(trailing) > SQCHAT_IRC_MSG_LEN -
                                (strlen(buffer->network->nickname) +
                                sizeof(" :")))
        sqchat_buffer_print(buffer, "Notice too long!\n");
    else
        sqchat_network_send(buffer->network, "NOTICE %s :%s\r\n",
                            argv[0], trailing);
    return 0;
}

// Max argc: 1
/* TODO: Possibly allow passwords to be specified as the second parameter if
 * only one channel is specified
 */
BI_CMD(sqchat_cmd_join) {
    if (argc < 1)
        return SQCHAT_CMD_SYNTAX_ERR;
    else if (buffer->network->status == CONNECTED)
        sqchat_network_send(buffer->network, "JOIN %s\r\n",
                            argv[0]);
    else
        sqchat_buffer_print(buffer, "Not connected!\n");
    return 0;
}

// Max argc: 1
BI_CMD(sqchat_cmd_part) {
    if (argc < 1) {
        if (buffer->type == CHANNEL)
            sqchat_network_send(buffer->network, "PART %s\r\n",
                                buffer->buffer_name);
        else
            sqchat_buffer_print(buffer, "You're not in a channel!\n");
    }
    else if (buffer->network->status != CONNECTED)
        sqchat_buffer_print(buffer, "Not connected!\n");
    else
        sqchat_network_send(buffer->network, "PART %s :%s\r\n",
                            argv[0], trailing ? trailing : "");
    return 0;
}

// Max argc: 0
BI_CMD(sqchat_cmd_topic) {
    if (buffer->network->status != CONNECTED) {
        sqchat_buffer_print(buffer, "Not connected!\n");
        return 0;
    }

    // Check if the user specified any parameters
    if (trailing == NULL) {
        if (buffer->type == CHANNEL) {
            sqchat_network_send(buffer->network, "TOPIC %s\r\n",
                                buffer->buffer_name);
            sqchat_claim_response(buffer->network, buffer, NULL, NULL);
        }
        else
            sqchat_buffer_print(buffer, "You're not in a channel!\n");
    }
    else {
        char * channel;
        if (strchr(buffer->network->chantypes, trailing[0]) != NULL)
            channel = strtok_r(trailing, " ", &trailing);
        else {
            if (buffer->type != CHANNEL) {
                sqchat_buffer_print(buffer, "You're not in a channel!\n");
                return 0;
            }
            channel = buffer->buffer_name;
        }
        if (*trailing == '\0') {
            sqchat_network_send(buffer->network, "TOPIC %s\n",
                                channel);
            sqchat_claim_response(buffer->network, buffer, NULL, NULL);
        }
        else
            sqchat_network_send(buffer->network, "TOPIC %s :%s\r\n", channel,
                                trailing);
    }
    return 0;
}

// Max argc: 0
BI_CMD(sqchat_cmd_connect) {
    if (buffer->network->status == DISCONNECTED)
        sqchat_network_connect(buffer->network);
    return 0;
}

// Max argc: 0
BI_CMD(sqchat_cmd_quit) {
    if (buffer->network->status == DISCONNECTED)
        sqchat_buffer_print(buffer, "Not connected!\n");
    else
        sqchat_network_disconnect(buffer->network, trailing);
    return 0;
}

BI_CMD(sqchat_cmd_quote) {
    if (buffer->network->status == DISCONNECTED)
        sqchat_buffer_print(buffer, "Not connected!\n");
    else if (trailing != NULL)
        sqchat_network_send(buffer->network, "%s\r\n", trailing);
    else
        return SQCHAT_CMD_SYNTAX_ERR;
    return 0;
}

BI_CMD(sqchat_cmd_motd) {
    if (buffer->network->status != CONNECTED) {
        sqchat_buffer_print(buffer, "Not connected!\n");
        return 0;
    }

    sqchat_claim_response(buffer->network, buffer, NULL, NULL);
    sqchat_network_send(buffer->network, "MOTD %s\r\n",
                        (argc >= 1) ? argv[0] : "");
    return 0;
}

BI_CMD(sqchat_cmd_mode) {
    if (buffer->network->status != CONNECTED) {
        sqchat_buffer_print(buffer, "Not connected!\n");
        return 0;
    }

    if (argc == 0) {
        if (buffer->type == NETWORK)
            sqchat_network_send(buffer->network,
                                "MODE %s\r\n", buffer->network->nickname);
        else if (buffer->type == CHANNEL)
            sqchat_network_send(buffer->network,
                                "MODE %s\r\n", buffer->buffer_name);
        else
            return SQCHAT_CMD_SYNTAX_ERR;
    }

    // Check if the user explicitly specified a target
    else if (strchr(buffer->network->chantypes, *(argv[0])) ||
             buffer->network->casecmp(buffer->network->nickname,
                                             argv[0]) == 0)
        sqchat_network_send(buffer->network,
                            "MODE %s %s\r\n",
                            argv[0], trailing ? trailing : "");
    else {
        if (buffer->type == NETWORK)
            sqchat_network_send(buffer->network,
                                "MODE %s %s %s\r\n",
                                buffer->network->nickname, argv[0],
                                trailing ? trailing : "");
        else if (buffer->type == CHANNEL)
            sqchat_network_send(buffer->network,
                                "MODE %s %s %s\r\n", buffer->buffer_name,
                                argv[0], trailing ? trailing : "");
        else
            return SQCHAT_CMD_SYNTAX_ERR;
    }
    sqchat_claim_response(buffer->network, buffer, NULL, NULL);
    return 0;
}

BI_CMD(sqchat_cmd_ctcp) {
    if (buffer->network->status != CONNECTED) {
        sqchat_buffer_print(buffer, "Not connected!\n");
        return 0;
    }

    // TODO: Choose the target automatically for query buffers
    if (argc < 2)
        return SQCHAT_CMD_SYNTAX_ERR;

    /* If the ctcp being sent is a PING, we need to record the time the command
     * was sent
     */
    if (strcasecmp(argv[1], "PING") == 0) {
        sqchat_network_sendf_ctcp(buffer->network, argv[0], "PING", "%li",
                                  g_get_monotonic_time());
    }
    else {
        if (trailing == NULL)
            sqchat_network_send_ctcp(buffer->network, argv[0], argv[1]);
        else
            sqchat_network_sendf_ctcp(buffer->network, argv[0], argv[1], "%s",
                                      trailing);
    }

    sqchat_buffer_print(buffer, "* Sent a CTCP %s to %s.\n", argv[1], argv[0]);
    return 0;
}

BI_CMD(sqchat_cmd_me) {
    if (buffer->network->status != CONNECTED)
        sqchat_buffer_print(buffer, "Not connected!\n");
    else if (trailing == NULL)
        return SQCHAT_CMD_SYNTAX_ERR;
    else if (buffer->type == NETWORK)
        sqchat_buffer_print(buffer,
                            "You can't send messages to this buffer.\n");
    else {
        sqchat_network_sendf_ctcp(buffer->network, buffer->buffer_name,
                                  "ACTION", "%s", trailing);
        sqchat_buffer_print(buffer, "* %s %s\n", buffer->network->nickname,
                            trailing);
    }
    return 0;
}

BI_CMD(sqchat_cmd_whois) {
    if (buffer->network->status != CONNECTED)
        sqchat_buffer_print(buffer, "Not connected!\n");
    else if (argc == 0)
        return SQCHAT_CMD_SYNTAX_ERR;
    else {
        if (argc == 1)
            sqchat_network_send(buffer->network, "WHOIS %s\r\n", argv[0]);
        else
            sqchat_network_send(buffer->network, "WHOIS %s %s\r\n", argv[0], argv[1]);
        sqchat_claim_response(buffer->network, buffer, NULL, NULL);
    }
    return 0;
}

BI_CMD(sqchat_cmd_oper) {
    if (buffer->network->status != CONNECTED)
        sqchat_buffer_print(buffer, "Not connected!\n");
    else if (argc < 2)
        return SQCHAT_CMD_SYNTAX_ERR;
    else {
        sqchat_network_send(buffer->network, "OPER %s %s\r\n", argv[0], argv[1]);
        sqchat_claim_response(buffer->network, buffer, NULL, NULL);
    }
    return 0;
}

BI_CMD(sqchat_cmd_whowas) {
    if (trailing == NULL)
        return SQCHAT_CMD_SYNTAX_ERR;

    sqchat_network_send(buffer->network, "WHOWAS %s\r\n", trailing);
    sqchat_claim_response(buffer->network, buffer, NULL, NULL);
    return 0;
}

BI_CMD(sqchat_cmd_lusers) {
    if (buffer->network->status != CONNECTED) {
        sqchat_buffer_print(buffer, "Not connected!\n");
        return 0;
    }

    if (argc == 0)
        sqchat_network_send(buffer->network, "LUSERS\r\n");
    else if (argc == 1)
        sqchat_network_send(buffer->network, "LUSERS %s\r\n", argv[0]);
    else
        sqchat_network_send(buffer->network, "LUSERS %s %s\r\n", argv[0], argv[1]);
    sqchat_claim_response(buffer->network, buffer, NULL, NULL);
    return 0;
}

BI_CMD(sqchat_cmd_invite) {
    if (argc < 2)
        return SQCHAT_CMD_SYNTAX_ERR;

    if (buffer->network->status != CONNECTED) {
        sqchat_buffer_print(buffer, "Not connected!\n");
        return 0;
    }

    sqchat_network_send(buffer->network, "INVITE %s %s\r\n", argv[0], argv[1]);
    sqchat_claim_response(buffer->network, buffer, NULL, NULL);
    return 0;
}

BI_CMD(sqchat_cmd_time) {
    if (buffer->network->status != CONNECTED)
        sqchat_buffer_print(buffer, "Not connected!\n");
    else {
        if (argc == 0)
            sqchat_network_send(buffer->network, "TIME\r\n");
        else
            sqchat_network_send(buffer->network, "TIME %s\r\n", argv[0]);
        sqchat_claim_response(buffer->network, buffer, NULL, NULL);
    }
    return 0;
}

BI_CMD(sqchat_cmd_version) {
    if (buffer->network->status != CONNECTED)
        sqchat_buffer_print(buffer, "Not connected!\n");
    else {
        if (argc == 0)
            sqchat_network_send(buffer->network, "VERSION\r\n");
        else
            sqchat_network_send(buffer->network, "VERSION %s\r\n", argv[0]);
        sqchat_claim_response(buffer->network, buffer, NULL, NULL);
    }
    return 0;
}

BI_CMD(sqchat_cmd_info) {
    if (buffer->network->status != CONNECTED)
        sqchat_buffer_print(buffer, "Not connected!\n");
    else {
        if (argc == 0) {
            sqchat_network_send(buffer->network, "INFO\r\n");
            sqchat_buffer_print(buffer, "--- Showing INFO for %s ---\n",
                                buffer->network->server_name);
        }
        else {
            sqchat_network_send(buffer->network, "INFO %s\r\n", argv[0]);
            sqchat_buffer_print(buffer, "--- Showing INFO for %s ---\n", argv[0]);
        }
        sqchat_claim_response(buffer->network, buffer, NULL, NULL);
    }
    return 0;
}

BI_CMD(sqchat_cmd_away) {
    if (buffer->network->status != CONNECTED)
        sqchat_buffer_print(buffer, "Not connected!\n");
    else {
        sqchat_network_send(buffer->network, "AWAY %s\r\n",
                            trailing ? trailing : DEFAULT_AWAY_MSG);
        sqchat_claim_response(buffer->network, buffer, NULL, NULL);
    }
    return 0;
}

BI_CMD(sqchat_cmd_back) {
    if (buffer->network->status != CONNECTED)
        sqchat_buffer_print(buffer, "Not connected!\n");
    else {
        sqchat_network_send(buffer->network, "AWAY\r\n");
        sqchat_claim_response(buffer->network, buffer, NULL, NULL);
    }
    return 0;
}

BI_CMD(sqchat_cmd_who) {
    if (argc < 1)
        return SQCHAT_CMD_SYNTAX_ERR;
    if (buffer->network->status != CONNECTED)
        sqchat_buffer_print(buffer, "Not connected!\n");
    else {
        if (argc == 1)
            sqchat_network_send(buffer->network, "WHO %s\r\n", argv[0]);
        else
            sqchat_network_send(buffer->network, "WHO %s %s\r\n", argv[0], argv[1]);

        if (SQCHAT_IS_CHAN(buffer->network, argv[0]))
            sqchat_buffer_print(buffer,
                                "--- Beginning of WHO for %s ---\n", argv[0]);
        sqchat_claim_response(buffer->network, buffer, NULL, NULL);
    }
    return 0;
}

BI_CMD(sqchat_cmd_links) {
    if (buffer->network->status != CONNECTED)
        sqchat_buffer_print(buffer, "Not connected!\n");
    else {
        if (argc == 0) {
            sqchat_buffer_print(buffer, "--- Showing LINKS for %s ---\n",
                                buffer->network->server_name);
            sqchat_network_send(buffer->network, "LINKS\r\n");
        }
        else if (argc == 1) {
            sqchat_buffer_print(buffer, "--- Showing LINKS for %s ---\n",
                                buffer->network->server_name);
            sqchat_network_send(buffer->network, "LINKS %s\r\n", argv[0]);
        }
        else {
            sqchat_buffer_print(buffer, "--- Showing LINKS for %s ---\n", argv[0]);
            sqchat_network_send(buffer->network, "LINKS %s %s\r\n", argv[0], argv[1]);
        }
        sqchat_claim_response(buffer->network, buffer, NULL, NULL);
    }
    return 0;
}

// TODO: Display a physical window for the LIST command
BI_CMD(sqchat_cmd_list) {
    if (buffer->network->status != CONNECTED)
        sqchat_buffer_print(buffer, "Not connected!\n");
    else {
        if (argc == 0)
            sqchat_network_send(buffer->network, "LIST\r\n");
        else if (argc == 1)
            sqchat_network_send(buffer->network, "LIST %s\r\n", argv[0]);
        else
            sqchat_network_send(buffer->network, "LIST %s %s\r\n", argv[0],
                                argv[1]);
        sqchat_claim_response(buffer->network, buffer, NULL, NULL);
    }
    return 0;
}

BI_CMD(sqchat_cmd_kick) {
    if (buffer->network->status != CONNECTED) {
        sqchat_buffer_print(buffer, "Not connected!\n");
        return 0;
    }
    if (argc < 1)
        return SQCHAT_CMD_SYNTAX_ERR;
    
    if (trailing == NULL) {
        if (buffer->type != CHANNEL)
            return SQCHAT_CMD_SYNTAX_ERR;

        sqchat_network_send(buffer->network, "KICK %s %s\r\n",
                            buffer->buffer_name, argv[0]);
    }
    else {
        // Check if the first argument was a username or a channel
        if (SQCHAT_IS_CHAN(buffer->network, argv[0])) {
            char * saveptr;
            char * arg2 = strtok_r(trailing, " ", &saveptr);
            if (arg2 == NULL)
                sqchat_network_send(buffer->network, "KICK %s %s\r\n",
                                    argv[0], trailing);
            else
                sqchat_network_send(buffer->network, "KICK %s %s :%s\r\n",
                                    argv[0], arg2,
                                    saveptr + strspn(saveptr, " "));
        }
        else {
            if (buffer->type != CHANNEL)
                return SQCHAT_CMD_SYNTAX_ERR;

            sqchat_network_send(buffer->network, "KICK %s %s :%s\r\n",
                                buffer->buffer_name, argv[0], trailing);
        }
    }

    sqchat_claim_response(buffer->network, buffer, NULL, NULL);

    return 0;
}

BI_CMD(sqchat_cmd_kill) {
    if (argc < 1 || trailing == NULL)
        return SQCHAT_CMD_SYNTAX_ERR;
    if (buffer->network->status != CONNECTED) {
        sqchat_buffer_print(buffer, "Not connected!\n");
        return 0;
    }

    sqchat_network_send(buffer->network, "KILL %s :%s\r\n", argv[0], trailing);
    sqchat_claim_response(buffer->network, buffer, NULL, NULL);
    return 0;
}

BI_CMD(sqchat_cmd_wallops) {
    if (trailing == NULL)
        return SQCHAT_CMD_SYNTAX_ERR;
    if (buffer->network->status != CONNECTED) {
        sqchat_buffer_print(buffer, "Not connected!\n");
        return 0;
    }

    sqchat_network_send(buffer->network, "WALLOPS :%s\r\n", trailing);
    sqchat_claim_response(buffer->network, buffer, NULL, NULL);
    return 0;
}

BI_CMD(sqchat_cmd_trace) {
    if (buffer->network->status != CONNECTED) {
        sqchat_buffer_print(buffer, "Not connected!\n");
        return 0;
    }

    if (argc == 0)
        sqchat_network_send(buffer->network, "TRACE\r\n");
    else
        sqchat_network_send(buffer->network, "TRACE %s\r\n", argv[0]);

    sqchat_buffer_print(buffer, "--- Start of TRACE ---\n");

    sqchat_claim_response(buffer->network, buffer, NULL, NULL);
    return 0;
}

BI_CMD(sqchat_cmd_username) {
    if (argc < 1)
        return SQCHAT_CMD_SYNTAX_ERR;

    free(buffer->network->username);
    buffer->network->username = strdup(argv[0]);
    sqchat_buffer_print(buffer,
                        "Your default username for this network is now '%s'.\n",
                        argv[0]);

    return 0;
}

BI_CMD(sqchat_cmd_realname) {
    if (argc < 1)
        return SQCHAT_CMD_SYNTAX_ERR;

    free(buffer->network->real_name);
    buffer->network->real_name = strdup(argv[0]);
    sqchat_buffer_print(buffer,
                        "Your default username for this network is now '%s'.\n",
                        argv[0]);

    return 0;
}

// vim: set expandtab tw=80 shiftwidth=4 softtabstop=4 cinoptions=(0,W4:
