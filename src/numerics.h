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

extern void init_numerics();

#define NUMERIC_CB(name)                            \
    extern short name(struct irc_network * network, \
                      char * hostmask,              \
                      short argc,                   \
                      char * argv[])                \
    _nonnull(1)

NUMERIC_CB(echo_argv_1);
NUMERIC_CB(rpl_myinfo);
NUMERIC_CB(rpl_isupport);
NUMERIC_CB(rpl_namreply);
NUMERIC_CB(rpl_endofnames);
NUMERIC_CB(rpl_motdstart);
NUMERIC_CB(rpl_motd);
NUMERIC_CB(rpl_endofmotd);
NUMERIC_CB(rpl_topic);
NUMERIC_CB(rpl_notopic);
NUMERIC_CB(rpl_topicwhotime);
NUMERIC_CB(rpl_channelmodeis);
NUMERIC_CB(rpl_creationtime);
NUMERIC_CB(rpl_whoisuser);
NUMERIC_CB(rpl_whoisserver);
NUMERIC_CB(rpl_whoisoperator);
NUMERIC_CB(rpl_whoisidle);
NUMERIC_CB(rpl_whoischannels);
NUMERIC_CB(rpl_whoissecure);
NUMERIC_CB(rpl_whoisaccount);
NUMERIC_CB(rpl_whois_generic);
NUMERIC_CB(rpl_whowasuser);
NUMERIC_CB(rpl_endofwhois);
NUMERIC_CB(rpl_whoisactually);
NUMERIC_CB(rpl_endofwhowas);
NUMERIC_CB(generic_lusers_rpl);
NUMERIC_CB(rpl_localglobalusers);
NUMERIC_CB(rpl_inviting);
NUMERIC_CB(rpl_time);
NUMERIC_CB(rpl_version);
NUMERIC_CB(rpl_info);
NUMERIC_CB(rpl_endofinfo);
NUMERIC_CB(rpl_nowaway);
NUMERIC_CB(rpl_unaway);
NUMERIC_CB(rpl_away);
NUMERIC_CB(rpl_whoreply);
NUMERIC_CB(rpl_endofwho);
NUMERIC_CB(rpl_links);
NUMERIC_CB(rpl_endoflinks);
NUMERIC_CB(rpl_liststart);
NUMERIC_CB(rpl_list);
NUMERIC_CB(rpl_listend);
NUMERIC_CB(rpl_hosthidden);
NUMERIC_CB(generic_rpl_trace);
NUMERIC_CB(rpl_tracelink);
NUMERIC_CB(rpl_traceserver);
NUMERIC_CB(rpl_traceservice);
NUMERIC_CB(rpl_traceoperator);
NUMERIC_CB(rpl_traceuser);
NUMERIC_CB(rpl_traceend);
NUMERIC_CB(rpl_snomask);
NUMERIC_CB(generic_error);
NUMERIC_CB(generic_network_error);
NUMERIC_CB(generic_command_error);
NUMERIC_CB(generic_channel_error);
NUMERIC_CB(generic_target_error);
NUMERIC_CB(generic_user_channel_error);
NUMERIC_CB(generic_echo_rpl);
NUMERIC_CB(generic_echo_rpl_end);
NUMERIC_CB(nick_change_error);
NUMERIC_CB(err_notregistered);

#undef NUMERIC_CB
#endif
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
