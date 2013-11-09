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

#define MAX_POSSIBLE_PARAMS ((SQCHAT_IRC_MSG_LEN - sizeof(":X X ")) / sizeof("X "))

sqchat_trie * message_types;

sqchat_msg_cb * numerics;

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

void sqchat_init_msg_parser() {
    message_types = sqchat_trie_new(sqchat_trie_strtoupper);
    numerics = calloc(IRC_NUMERIC_MAX, sizeof(sqchat_msg_cb*));

    // Add in the built in message types
    sqchat_trie_set(message_types, "CAP", sqchat_cap_msg_callback);
    sqchat_trie_set(message_types, "JOIN", sqchat_join_msg_callback);
    sqchat_trie_set(message_types, "PART", sqchat_part_msg_callback);
    sqchat_trie_set(message_types, "PRIVMSG", sqchat_privmsg_msg_callback);
    sqchat_trie_set(message_types, "PING", sqchat_ping_msg_callback);
    sqchat_trie_set(message_types, "NICK", sqchat_nick_msg_callback);
    sqchat_trie_set(message_types, "TOPIC", sqchat_topic_msg_callback);
    sqchat_trie_set(message_types, "NOTICE", sqchat_notice_msg_callback);
    sqchat_trie_set(message_types, "MODE", sqchat_mode_msg_callback);
    sqchat_trie_set(message_types, "QUIT", sqchat_quit_msg_callback);
    sqchat_trie_set(message_types, "KICK", sqchat_kick_msg_callback);
    sqchat_trie_set(message_types, "INVITE", sqchat_invite_msg_callback);
    sqchat_trie_set(message_types, "ERROR", sqchat_error_msg_callback);
    sqchat_trie_set(message_types, "WALLOPS", sqchat_wallops_msg_callback);

    sqchat_init_message_types();
    sqchat_ctcp_init();

    numerics[IRC_RPL_WELCOME] = sqchat_echo_argv_1;
    numerics[IRC_RPL_YOURHOST] = sqchat_echo_argv_1;
    numerics[IRC_RPL_CREATED] = sqchat_echo_argv_1;
    numerics[IRC_RPL_MYINFO] = sqchat_rpl_myinfo;
    numerics[IRC_RPL_ISUPPORT] = sqchat_rpl_isupport;
    numerics[IRC_RPL_NAMREPLY] = sqchat_rpl_namreply;
    numerics[IRC_RPL_ENDOFNAMES] = sqchat_rpl_endofnames;
    numerics[IRC_RPL_MOTDSTART] = sqchat_rpl_motdstart;
    numerics[IRC_RPL_MOTD] = sqchat_rpl_motd;
    numerics[IRC_RPL_ENDOFMOTD] = sqchat_rpl_endofmotd;
    numerics[IRC_ERR_NOMOTD] = sqchat_generic_error;
    numerics[IRC_ERR_NICKNAMEINUSE] = sqchat_nick_change_error;
    numerics[IRC_ERR_ERRORNEUSNICKNAME] = sqchat_nick_change_error;
    numerics[IRC_RPL_TOPIC] = sqchat_rpl_topic;
    numerics[IRC_RPL_NOTOPIC] = sqchat_rpl_notopic;
    numerics[IRC_RPL_TOPICWHOTIME] = sqchat_rpl_topicwhotime;
    numerics[IRC_RPL_CHANNELMODEIS] = sqchat_rpl_channelmodeis;
    numerics[IRC_RPL_CREATIONTIME] = sqchat_rpl_creationtime;
    numerics[IRC_RPL_SNOMASK] = sqchat_rpl_snomask;

    numerics[IRC_RPL_WHOISUSER] = sqchat_rpl_whoisuser;
    numerics[IRC_RPL_WHOISSERVER] = sqchat_rpl_whoisserver;
    numerics[IRC_RPL_WHOISOPERATOR] = sqchat_rpl_whoisoperator;
    numerics[IRC_RPL_WHOISIDLE] = sqchat_rpl_whoisidle;
    numerics[IRC_RPL_WHOISCHANNELS] = sqchat_rpl_whoischannels;
    numerics[IRC_RPL_WHOISSECURE] = sqchat_rpl_whois_generic;
    numerics[IRC_RPL_WHOISACCOUNT] = sqchat_rpl_whoisaccount;
    numerics[IRC_RPL_WHOISREGNICK] = sqchat_rpl_whois_generic;
    numerics[IRC_RPL_WHOISHOST] = sqchat_rpl_whois_generic;
    numerics[IRC_RPL_WHOISSPECIAL] = sqchat_rpl_whois_generic;
    numerics[IRC_RPL_WHOISMODES] = sqchat_rpl_whois_generic;
    numerics[IRC_RPL_WHOISACTUALLY] = sqchat_rpl_whoisactually;
    numerics[IRC_RPL_ENDOFWHOIS] = sqchat_rpl_endofwhois;

    numerics[IRC_RPL_WHOWASUSER] = sqchat_rpl_whowasuser;
    numerics[IRC_RPL_ENDOFWHOWAS] = sqchat_rpl_endofwhowas;

    numerics[IRC_RPL_YOUREOPER] = sqchat_generic_echo_rpl_end;

    numerics[IRC_RPL_LUSERCLIENT] = sqchat_generic_echo_rpl;
    numerics[IRC_RPL_LUSEROP] = sqchat_generic_lusers_rpl;
    numerics[IRC_RPL_LUSERUNKNOWN] = sqchat_generic_lusers_rpl;
    numerics[IRC_RPL_LUSERCHANNELS] = sqchat_generic_lusers_rpl;
    numerics[IRC_RPL_LOCALUSERS] = sqchat_rpl_localglobalusers;
    numerics[IRC_RPL_GLOBALUSERS] = sqchat_rpl_localglobalusers;
    numerics[IRC_RPL_STATSCONN] = sqchat_generic_echo_rpl;
    numerics[IRC_RPL_LUSERME] = sqchat_generic_echo_rpl_end;

    numerics[IRC_RPL_INVITING] = sqchat_rpl_inviting;

    numerics[IRC_RPL_TIME] = sqchat_rpl_time;

    numerics[IRC_RPL_VERSION] = sqchat_rpl_version;

    numerics[IRC_RPL_INFO] = sqchat_rpl_info;
    numerics[IRC_RPL_ENDOFINFO] = sqchat_rpl_endofinfo;

    numerics[IRC_RPL_AWAY] = sqchat_rpl_away;
    numerics[IRC_RPL_NOWAWAY] = sqchat_rpl_nowaway;
    numerics[IRC_RPL_UNAWAY] = sqchat_rpl_unaway;

    numerics[IRC_RPL_WHOREPLY] = sqchat_rpl_whoreply;
    numerics[IRC_RPL_ENDOFWHO] = sqchat_rpl_endofwho;

    numerics[IRC_RPL_LINKS] = sqchat_rpl_links;
    numerics[IRC_RPL_ENDOFLINKS] = sqchat_rpl_endoflinks;

    numerics[IRC_RPL_LISTSTART] = sqchat_rpl_liststart;
    numerics[IRC_RPL_LIST] = sqchat_rpl_list;
    numerics[IRC_RPL_LISTEND] = sqchat_rpl_listend;

    numerics[IRC_RPL_HOSTHIDDEN] = sqchat_rpl_hosthidden;

    numerics[IRC_RPL_TRACELINK] = sqchat_rpl_tracelink;
    numerics[IRC_RPL_TRACECONNECTING] = sqchat_generic_rpl_trace;
    numerics[IRC_RPL_TRACEHANDSHAKE] = sqchat_generic_rpl_trace;
    numerics[IRC_RPL_TRACEUNKNOWN] = sqchat_generic_rpl_trace;
    numerics[IRC_RPL_TRACEOPERATOR] = sqchat_rpl_traceoperator;
    numerics[IRC_RPL_TRACEUSER] = sqchat_rpl_traceuser;
    numerics[IRC_RPL_TRACESERVER] = sqchat_rpl_traceserver;
    numerics[IRC_RPL_TRACESERVICE] = sqchat_rpl_traceservice;
    numerics[IRC_RPL_TRACECLASS] = sqchat_generic_rpl_trace;
    numerics[IRC_RPL_TRACEEND] = sqchat_rpl_traceend;

    numerics[IRC_ERR_NOADMININFO] = sqchat_generic_error;
    numerics[IRC_ERR_NOSUCHNICK] = sqchat_generic_target_error;
    numerics[IRC_ERR_NOSUCHSERVER] = sqchat_generic_target_error;
    numerics[IRC_ERR_CANNOTSENDTOCHAN] = sqchat_generic_channel_error;
    numerics[IRC_ERR_TOOMANYCHANNELS] = sqchat_generic_channel_error;
    numerics[IRC_ERR_WASNOSUCHNICK] = sqchat_generic_target_error;
    numerics[IRC_ERR_TOOMANYTARGETS] = sqchat_generic_target_error;
    numerics[IRC_ERR_NOSUCHSERVICE] = sqchat_generic_target_error;
    numerics[IRC_ERR_NOORIGIN] = sqchat_generic_error;
    numerics[IRC_ERR_NORECEPIENT] = sqchat_generic_error;
    numerics[IRC_ERR_NOTEXTTOSEND] = sqchat_generic_error;
    numerics[IRC_ERR_NOTOPLEVEL] = sqchat_generic_target_error;
    numerics[IRC_ERR_WILDTOPLEVEL] = sqchat_generic_target_error;
    numerics[IRC_ERR_BADMASK] = sqchat_generic_target_error;
    numerics[IRC_ERR_NOSUCHCHANNEL] = sqchat_generic_channel_error;
    numerics[IRC_ERR_KEYSET] = sqchat_generic_channel_error;
    numerics[IRC_ERR_NOCHANMODES] = sqchat_generic_channel_error;
    numerics[IRC_ERR_UNKNOWNMODE] = sqchat_generic_channel_error;
    numerics[IRC_ERR_BADCHANMASK] = sqchat_generic_channel_error;
    numerics[IRC_ERR_BANLISTFULL] = sqchat_generic_channel_error;
    numerics[IRC_ERR_CHANOPRIVSNEEDED] = sqchat_generic_channel_error;
    numerics[IRC_ERR_NEEDMOREPARAMS] = sqchat_generic_command_error;
    numerics[IRC_ERR_UNKNOWNCOMMAND] = sqchat_generic_command_error;
    numerics[IRC_RPL_TRYAGAIN] = sqchat_generic_command_error;
    numerics[IRC_ERR_FILEERROR] = sqchat_generic_error;
    numerics[IRC_ERR_NONICKNAMEGIVEN] = sqchat_generic_target_error;
    numerics[IRC_ERR_NICKCOLLISION] = sqchat_generic_target_error;
    numerics[IRC_ERR_UNAVAILRESOURCE] = sqchat_generic_target_error;
    numerics[IRC_ERR_USERNOTINCHANNEL] = sqchat_generic_user_channel_error;
    numerics[IRC_ERR_NOTONCHANNEL] = sqchat_generic_channel_error;
    numerics[IRC_ERR_USERONCHANNEL] = sqchat_generic_user_channel_error;
    numerics[IRC_ERR_ALREADYREGISTERED] = sqchat_generic_error;
    numerics[IRC_ERR_NOPERMFORHOST] = sqchat_generic_network_error;
    numerics[IRC_ERR_PASSWDMISMATCH] = sqchat_generic_network_error;
    numerics[IRC_ERR_YOUREBANNEDCREEP] = sqchat_generic_network_error;
    numerics[IRC_ERR_YOUWILLBEBANNED] = sqchat_generic_error;
    numerics[IRC_ERR_CANTKILLSERVER] = sqchat_generic_error;
    numerics[IRC_ERR_RESTRICTED] = sqchat_echo_argv_1;
    numerics[IRC_ERR_UNIQOPPRIVSNEEDED] = sqchat_generic_error;
    numerics[IRC_ERR_NOOPERHOST] = sqchat_generic_error;
    numerics[IRC_ERR_UMODEUNKNOWNFLAG] = sqchat_generic_error;
    numerics[IRC_ERR_USERSDONTMATCH] = sqchat_generic_error;
    numerics[IRC_ERR_NOPRIVILEGES] = sqchat_generic_error;
}

void sqchat_process_msg(struct sqchat_network * network, char * msg) {
    char * cursor;
    char * hostmask;
    char * command;
    char * params;
    short numeric;
    sqchat_msg_cb callback;

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
        if (numeric > 0 &&
            numeric <= IRC_NUMERIC_MAX && numerics[numeric] != NULL) {
            switch (numerics[numeric](network, hostmask, argc, argv)) {
                case SQCHAT_MSG_ERR_ARGS:
                    sqchat_buffer_print(network->buffer,
                                        "Error parsing numeric response: Not "
                                        "enough arguments included in the "
                                        "response.\n"
                                        "Numeric: %i\n", numeric);
                    sqchat_dump_msg_to_buffer(network->buffer, hostmask, argc,
                                              argv);
                    if (network->claimed_responses)
                        sqchat_remove_last_response_claim(network);
                    break;
                case SQCHAT_MSG_ERR_ARGS_FATAL:
                    sqchat_buffer_print(network->buffer,
                                        "Fatal: Error parsing numeric "
                                        "response: Not enough arguments "
                                        "included in the response from the "
                                        "server.\n"
                                        "Numeric: %i\n",
                                        numeric);
                    sqchat_dump_msg_to_buffer(network->buffer, hostmask, argc,
                                              argv);
                    sqchat_network_disconnect(network, "Invalid data received");
                    break;
                case SQCHAT_MSG_ERR_MISC:
                    sqchat_dump_msg_to_buffer(network->buffer, hostmask, argc,
                                              argv);
                case SQCHAT_MSG_ERR_MISC_NODUMP:
                    if (network->claimed_responses)
                        sqchat_remove_last_response_claim(network);
                    break;
            }
        }
        else {
            sqchat_buffer_print(network->buffer,
                                "Error parsing message: unknown numeric %i\n",
                                numeric);
            sqchat_dump_msg_to_buffer(network->buffer, hostmask, argc, argv);
        }
    }
    // Attempt to look up the command
    else if ((callback = sqchat_trie_get(message_types, command)) != NULL) {
        switch (callback(network, hostmask, argc, argv)) {
            case SQCHAT_MSG_ERR_ARGS:
                sqchat_buffer_print(network->buffer,
                                    "Error parsing message: Not enough arguments "
                                    "included in the message.\n"
                                    "Type: %s\n", command);
                sqchat_dump_msg_to_buffer(network->buffer, hostmask, argc,
                                          argv);
                if (network->claimed_responses)
                    sqchat_remove_last_response_claim(network);
                break;
            case SQCHAT_MSG_ERR_ARGS_FATAL:
                sqchat_buffer_print(network->buffer,
                                    "Fatal: Error parsing message: Not enough "
                                    "arguments included in the message.\n"
                                    "Type: %s\n", command);
                sqchat_dump_msg_to_buffer(network->buffer, hostmask, argc,
                                          argv);
                sqchat_network_disconnect(network, "Invalid data received");
                break;
            case SQCHAT_MSG_ERR_MISC:
                sqchat_dump_msg_to_buffer(network->buffer, hostmask, argc,
                                          argv);
            case SQCHAT_MSG_ERR_MISC_NODUMP:
                if (network->claimed_responses)
                    sqchat_remove_last_response_claim(network);
                break;
        }
    }
    else {
        sqchat_buffer_print(network->buffer,
                            "Error parsing message: unknown message type: "
                            "\"%s\"\n",
                            command);
        sqchat_dump_msg_to_buffer(network->buffer, hostmask, argc, argv);
    }
}

void sqchat_split_hostmask(char * hostmask, char ** nickname, char ** address) {
    char * saveptr;
    *nickname = strtok_r(hostmask, "!", &saveptr);
    *address = strtok_r(NULL, "!", &saveptr);
}
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
