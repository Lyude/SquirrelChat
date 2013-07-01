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

#include "message_parser.h"
#include "message_types.h"

#include "trie.h"
#include "irc_network.h"
#include "irc_numerics.h"
#include "numerics.h"
#include "ui/buffer.h"
#include "chat.h"
#include "errors.h"
#include "ctcp.h"
#include "cmd_responses.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <stdio.h>

#define MAX_POSSIBLE_PARAMS ((IRC_MSG_LEN - sizeof(":X X ")) / sizeof("X "))

trie * message_types;

irc_message_callback * numerics;

/* A function to convert strings to shorts, optimized specifically for IRC
 * numerics
 */
short numeric_to_short(char * numeric) {
    short result = 0;
    short i;
    for (i = 0; i < 3; i++) {
        if (numeric[i] < '0' || numeric[i] > '9')
            return -1;
        result = (result * 10) + (numeric[i] - '0');
    }
    if (numeric[i] != '\0')
        return -1;
    else
        return result;
}

void init_message_parser() {
    message_types = trie_new(trie_strtoupper);
    numerics = calloc(IRC_NUMERIC_MAX, sizeof(irc_message_callback*));

    // Add in the built in message types
    trie_set(message_types, "CAP", cap_msg_callback);
    trie_set(message_types, "JOIN", join_msg_callback);
    trie_set(message_types, "PART", part_msg_callback);
    trie_set(message_types, "PRIVMSG", privmsg_msg_callback);
    trie_set(message_types, "PING", ping_msg_callback);
    trie_set(message_types, "NICK", nick_msg_callback);
    trie_set(message_types, "TOPIC", topic_msg_callback);
    trie_set(message_types, "NOTICE", notice_msg_callback);
    trie_set(message_types, "MODE", mode_msg_callback);
    trie_set(message_types, "QUIT", quit_msg_callback);
    trie_set(message_types, "KICK", kick_msg_callback);
    trie_set(message_types, "INVITE", invite_msg_callback);
    trie_set(message_types, "ERROR", error_msg_callback);

    init_message_types();
    init_ctcp();

    numerics[IRC_RPL_WELCOME] = echo_argv_1;
    numerics[IRC_RPL_YOURHOST] = echo_argv_1;
    numerics[IRC_RPL_CREATED] = echo_argv_1;
    numerics[IRC_RPL_MYINFO] = rpl_myinfo;
    numerics[IRC_RPL_ISUPPORT] = rpl_isupport;
    numerics[IRC_RPL_NAMREPLY] = rpl_namreply;
    numerics[IRC_RPL_ENDOFNAMES] = rpl_endofnames;
    numerics[IRC_RPL_MOTDSTART] = rpl_motdstart;
    numerics[IRC_RPL_MOTD] = rpl_motd;
    numerics[IRC_RPL_ENDOFMOTD] = rpl_endofmotd;
    numerics[IRC_ERR_NOMOTD] = generic_error;
    numerics[IRC_ERR_NICKNAMEINUSE] = nick_change_error;
    numerics[IRC_ERR_ERRORNEUSNICKNAME] = nick_change_error;
    numerics[IRC_RPL_TOPIC] = rpl_topic;
    numerics[IRC_RPL_NOTOPIC] = rpl_notopic;
    numerics[IRC_RPL_TOPICWHOTIME] = rpl_topicwhotime;
    numerics[IRC_RPL_CHANNELMODEIS] = rpl_channelmodeis;
    numerics[IRC_RPL_CREATIONTIME] = rpl_creationtime;

    numerics[IRC_RPL_WHOISUSER] = rpl_whoisuser;
    numerics[IRC_RPL_WHOISSERVER] = rpl_whoisserver;
    numerics[IRC_RPL_WHOISOPERATOR] = rpl_whoisoperator;
    numerics[IRC_RPL_WHOISIDLE] = rpl_whoisidle;
    numerics[IRC_RPL_WHOISCHANNELS] = rpl_whoischannels;
    numerics[IRC_RPL_WHOISSECURE] = rpl_whois_generic;
    numerics[IRC_RPL_WHOISACCOUNT] = rpl_whoisaccount;
    numerics[IRC_RPL_WHOISREGNICK] = rpl_whois_generic;
    numerics[IRC_RPL_WHOISHOST] = rpl_whois_generic;
    numerics[IRC_RPL_WHOISSPECIAL] = rpl_whois_generic;
    numerics[IRC_RPL_WHOISMODES] = rpl_whois_generic;
    numerics[IRC_RPL_WHOISACTUALLY] = rpl_whoisactually;
    numerics[IRC_RPL_ENDOFWHOIS] = rpl_endofwhois;

    numerics[IRC_RPL_WHOWASUSER] = rpl_whowasuser;
    numerics[IRC_RPL_ENDOFWHOWAS] = rpl_endofwhowas;

    numerics[IRC_RPL_YOUREOPER] = generic_echo_rpl_end;

    numerics[IRC_RPL_LUSERCLIENT] = generic_echo_rpl;
    numerics[IRC_RPL_LUSEROP] = generic_lusers_rpl;
    numerics[IRC_RPL_LUSERUNKNOWN] = generic_lusers_rpl;
    numerics[IRC_RPL_LUSERCHANNELS] = generic_lusers_rpl;
    numerics[IRC_RPL_LOCALUSERS] = rpl_localglobalusers;
    numerics[IRC_RPL_GLOBALUSERS] = rpl_localglobalusers;
    numerics[IRC_RPL_STATSCONN] = generic_echo_rpl;
    numerics[IRC_RPL_LUSERME] = generic_echo_rpl_end;

    numerics[IRC_RPL_INVITING] = rpl_inviting;

    numerics[IRC_RPL_TIME] = rpl_time;

    numerics[IRC_RPL_VERSION] = rpl_version;

    numerics[IRC_RPL_INFO] = rpl_info;
    numerics[IRC_RPL_ENDOFINFO] = rpl_endofinfo;

    numerics[IRC_RPL_AWAY] = rpl_away;
    numerics[IRC_RPL_NOWAWAY] = rpl_nowaway;
    numerics[IRC_RPL_UNAWAY] = rpl_unaway;

    numerics[IRC_RPL_WHOREPLY] = rpl_whoreply;
    numerics[IRC_RPL_ENDOFWHO] = rpl_endofwho;

    numerics[IRC_RPL_LINKS] = rpl_links;
    numerics[IRC_RPL_ENDOFLINKS] = rpl_endoflinks;

    numerics[IRC_RPL_LISTSTART] = rpl_liststart;
    numerics[IRC_RPL_LIST] = rpl_list;
    numerics[IRC_RPL_LISTEND] = rpl_listend;

    numerics[IRC_RPL_HOSTHIDDEN] = rpl_hosthidden;

    numerics[IRC_ERR_NOTREGISTERED] = err_notregistered;
    numerics[IRC_ERR_NOADMININFO] = generic_error;
    numerics[IRC_ERR_NOSUCHNICK] = generic_target_error;
    numerics[IRC_ERR_NOSUCHSERVER] = generic_target_error;
    numerics[IRC_ERR_CANNOTSENDTOCHAN] = generic_channel_error;
    numerics[IRC_ERR_TOOMANYCHANNELS] = generic_channel_error;
    numerics[IRC_ERR_WASNOSUCHNICK] = generic_target_error;
    numerics[IRC_ERR_TOOMANYTARGETS] = generic_target_error;
    numerics[IRC_ERR_NOSUCHSERVICE] = generic_target_error;
    numerics[IRC_ERR_NOORIGIN] = generic_error;
    numerics[IRC_ERR_NORECEPIENT] = generic_error;
    numerics[IRC_ERR_NOTEXTTOSEND] = generic_error;
    numerics[IRC_ERR_NOTOPLEVEL] = generic_target_error;
    numerics[IRC_ERR_WILDTOPLEVEL] = generic_target_error;
    numerics[IRC_ERR_BADMASK] = generic_target_error;
    numerics[IRC_ERR_NOSUCHCHANNEL] = generic_channel_error;
    numerics[IRC_ERR_KEYSET] = generic_channel_error;
    numerics[IRC_ERR_NOCHANMODES] = generic_channel_error;
    numerics[IRC_ERR_UNKNOWNMODE] = generic_channel_error;
    numerics[IRC_ERR_BADCHANMASK] = generic_channel_error;
    numerics[IRC_ERR_BANLISTFULL] = generic_channel_error;
    numerics[IRC_ERR_CHANOPRIVSNEEDED] = generic_channel_error;
    numerics[IRC_ERR_NEEDMOREPARAMS] = generic_command_error;
    numerics[IRC_ERR_UNKNOWNCOMMAND] = generic_command_error;
    numerics[IRC_RPL_TRYAGAIN] = generic_command_error;
    numerics[IRC_ERR_FILEERROR] = generic_error;
    numerics[IRC_ERR_NONICKNAMEGIVEN] = generic_target_error;
    numerics[IRC_ERR_NICKCOLLISION] = generic_target_error;
    numerics[IRC_ERR_UNAVAILRESOURCE] = generic_target_error;
    numerics[IRC_ERR_USERNOTINCHANNEL] = generic_user_channel_error;
    numerics[IRC_ERR_NOTONCHANNEL] = generic_channel_error;
    numerics[IRC_ERR_USERONCHANNEL] = generic_user_channel_error;
    numerics[IRC_ERR_ALREADYREGISTERED] = generic_error;
    numerics[IRC_ERR_NOPERMFORHOST] = generic_network_error;
    numerics[IRC_ERR_PASSWDMISMATCH] = generic_network_error;
    numerics[IRC_ERR_YOUREBANNEDCREEP] = generic_network_error;
    numerics[IRC_ERR_YOUWILLBEBANNED] = generic_error;
    numerics[IRC_ERR_CANTKILLSERVER] = generic_error;
    numerics[IRC_ERR_RESTRICTED] = echo_argv_1;
    numerics[IRC_ERR_UNIQOPPRIVSNEEDED] = generic_error;
    numerics[IRC_ERR_NOOPERHOST] = generic_error;
    numerics[IRC_ERR_UMODEUNKNOWNFLAG] = generic_error;
    numerics[IRC_ERR_USERSDONTMATCH] = generic_error;
    numerics[IRC_ERR_NOPRIVILEGES] = generic_error;
}

void process_irc_message(struct irc_network * network, char * msg) {
    char * cursor;
    char * hostmask;
    char * command;
    char * params;
    short numeric;
    irc_message_callback callback;

    /* TODO: Maybe figure out a better behavior for when bad messages are
     * received...
     */
    // Check to see if the message has a sender
    if (msg[0] == ':') {
        if ((hostmask = strtok_r(msg+1, " ", &cursor)) == NULL)
            return;
        if ((command = strtok_r(NULL, " ", &cursor)) == NULL)
            return;
    }
    else {
        hostmask = NULL;
        if ((command = strtok_r(msg, " ", &cursor)) == NULL)
            return;
    }

    char * argv[MAX_POSSIBLE_PARAMS];
    short argc;
    for (argc = 0; ; argc++) {
        char * param_end;

        /* If there's a ':' at the beginning of the message we need to stop
         * processing spaces
         */
        if (*cursor == ':') {
            argv[argc++] = cursor + 1;
            break;
        }

        else if ((param_end = strchr(cursor, ' ')) == NULL) {
            argv[argc] = cursor;
            argc++;
            break;
        }
        *param_end = '\0';
        argv[argc] = cursor;
        for (cursor = param_end + 1; *cursor == ' '; cursor++);
    }

    if ((numeric = numeric_to_short(command)) != -1) {
        if (numeric > 0 && numeric <= IRC_NUMERIC_MAX && numerics[numeric] != NULL) {
            switch (numerics[numeric](network, hostmask, argc, argv)) {
                case IRC_MSG_ERR_ARGS:
                    print_to_buffer(network->buffer,
                                    "Error parsing numeric response: Not "
                                    "enough arguments included in the "
                                    "response.\n"
                                    "Numeric: %i\n", numeric);
                    dump_msg_to_buffer(network->buffer, hostmask, argc, argv);
                    if (network->claimed_responses)
                        remove_last_response_claim(network);
                    break;
                case IRC_MSG_ERR_ARGS_FATAL:
                    print_to_buffer(network->buffer,
                                    "Fatal: Error parsing numeric response: "
                                    "Not enough arguments included in the "
                                    "response.\n"
                                    "Numeric: %i\n", numeric);
                    dump_msg_to_buffer(network->buffer, hostmask, argc, argv);
                    disconnect_irc_network(network, "Invalid data received");
                    break;
                case IRC_MSG_ERR_MISC:
                    dump_msg_to_buffer(network->buffer, hostmask, argc, argv);
                case IRC_MSG_ERR_MISC_NODUMP:
                    if (network->claimed_responses)
                        remove_last_response_claim(network);
                    break;
            }
        }
        else {
            print_to_buffer(network->buffer,
                            "Error parsing message: unknown numeric %i\n",
                            numeric);
            dump_msg_to_buffer(network->buffer, hostmask, argc, argv);
        }
    }
    // Attempt to look up the command
    else if ((callback = trie_get(message_types, command)) != NULL) {
        switch (callback(network, hostmask, argc, argv)) {
            case IRC_MSG_ERR_ARGS:
                print_to_buffer(network->buffer,
                                "Error parsing message: Not enough arguments "
                                "included in the message.\n"
                                "Type: %s\n", command);
                dump_msg_to_buffer(network->buffer, hostmask, argc, argv);
                if (network->claimed_responses)
                    remove_last_response_claim(network);
                break;
            case IRC_MSG_ERR_ARGS_FATAL:
                print_to_buffer(network->buffer,
                                "Fatal: Error parsing message: Not enough "
                                "arguments included in the message.\n"
                                "Type: %s\n", command);
                dump_msg_to_buffer(network->buffer, hostmask, argc, argv);
                disconnect_irc_network(network, "Invalid data received");
                break;
            case IRC_MSG_ERR_MISC:
                dump_msg_to_buffer(network->buffer, hostmask, argc, argv);
            case IRC_MSG_ERR_MISC_NODUMP:
                if (network->claimed_responses)
                    remove_last_response_claim(network);
                break;
        }
    }
    else {
        print_to_buffer(network->buffer,
                        "Error parsing message: unknown message type: \"%s\"\n",
                        command);
        dump_msg_to_buffer(network->buffer, hostmask, argc, argv);
    }
}

void split_irc_hostmask(char * hostmask, char ** nickname, char ** address) {
    char * saveptr;
    *nickname = strtok_r(hostmask, "!", &saveptr);
    *address = strtok_r(NULL, "!", &saveptr);
}
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
