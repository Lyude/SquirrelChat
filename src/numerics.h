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

#define NUMERIC_CB(name)                            \
    extern void name(struct irc_network * network,  \
                     char * hostmask,               \
                     short argc,                    \
                     char * argv[])                 \
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
NUMERIC_CB(generic_channel_error);
NUMERIC_CB(nick_change_error);
NUMERIC_CB(err_notregistered);

#undef NUMERIC_CB
#endif
// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
