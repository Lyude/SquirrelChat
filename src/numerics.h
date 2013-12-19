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
#ifndef __NUMERICS_H__
#define __NUMERICS_H__

#include "irc_network.h"

extern void sqchat_init_numerics();

#define NUMERIC_CB(name)                                    \
    extern short name(struct sqchat_network * network,      \
                      char * hostmask,                      \
                      short argc,                           \
                      char * argv[])                        \
    _attr_nonnull(1)

NUMERIC_CB(sqchat_echo_argv_1);
NUMERIC_CB(sqchat_rpl_myinfo);
NUMERIC_CB(sqchat_rpl_isupport);
NUMERIC_CB(sqchat_rpl_namreply);
NUMERIC_CB(sqchat_rpl_endofnames);
NUMERIC_CB(sqchat_rpl_motdstart);
NUMERIC_CB(sqchat_rpl_motd);
NUMERIC_CB(sqchat_rpl_endofmotd);
NUMERIC_CB(sqchat_rpl_topic);
NUMERIC_CB(sqchat_rpl_notopic);
NUMERIC_CB(sqchat_rpl_topicwhotime);
NUMERIC_CB(sqchat_rpl_channelmodeis);
NUMERIC_CB(sqchat_rpl_creationtime);
NUMERIC_CB(sqchat_rpl_whoisuser);
NUMERIC_CB(sqchat_rpl_whoisserver);
NUMERIC_CB(sqchat_rpl_whoisoperator);
NUMERIC_CB(sqchat_rpl_whoisidle);
NUMERIC_CB(sqchat_rpl_whoischannels);
NUMERIC_CB(sqchat_rpl_whoissecure);
NUMERIC_CB(sqchat_rpl_whoisaccount);
NUMERIC_CB(sqchat_rpl_whois_generic);
NUMERIC_CB(sqchat_rpl_whowasuser);
NUMERIC_CB(sqchat_rpl_endofwhois);
NUMERIC_CB(sqchat_rpl_whoisactually);
NUMERIC_CB(sqchat_rpl_endofwhowas);
NUMERIC_CB(sqchat_generic_lusers_rpl);
NUMERIC_CB(sqchat_rpl_localglobalusers);
NUMERIC_CB(sqchat_rpl_inviting);
NUMERIC_CB(sqchat_rpl_time);
NUMERIC_CB(sqchat_rpl_version);
NUMERIC_CB(sqchat_rpl_info);
NUMERIC_CB(sqchat_rpl_endofinfo);
NUMERIC_CB(sqchat_rpl_nowaway);
NUMERIC_CB(sqchat_rpl_unaway);
NUMERIC_CB(sqchat_rpl_away);
NUMERIC_CB(sqchat_rpl_whoreply);
NUMERIC_CB(sqchat_rpl_endofwho);
NUMERIC_CB(sqchat_rpl_links);
NUMERIC_CB(sqchat_rpl_endoflinks);
NUMERIC_CB(sqchat_rpl_liststart);
NUMERIC_CB(sqchat_rpl_list);
NUMERIC_CB(sqchat_rpl_listend);
NUMERIC_CB(sqchat_rpl_hosthidden);
NUMERIC_CB(sqchat_generic_rpl_trace);
NUMERIC_CB(sqchat_rpl_tracelink);
NUMERIC_CB(sqchat_rpl_traceserver);
NUMERIC_CB(sqchat_rpl_traceservice);
NUMERIC_CB(sqchat_rpl_traceoperator);
NUMERIC_CB(sqchat_rpl_traceuser);
NUMERIC_CB(sqchat_rpl_traceend);
NUMERIC_CB(sqchat_rpl_snomask);
NUMERIC_CB(sqchat_generic_error);
NUMERIC_CB(sqchat_generic_network_error);
NUMERIC_CB(sqchat_generic_command_error);
NUMERIC_CB(sqchat_generic_channel_error);
NUMERIC_CB(sqchat_generic_target_error);
NUMERIC_CB(sqchat_generic_user_channel_error);
NUMERIC_CB(sqchat_generic_echo_rpl);
NUMERIC_CB(sqchat_generic_echo_rpl_end);
NUMERIC_CB(sqchat_nick_change_error);

#undef NUMERIC_CB
#endif
// vim: set expandtab tw=80 shiftwidth=4 softtabstop=4 cinoptions=(0,W4:
