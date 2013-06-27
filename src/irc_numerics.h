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

#ifndef __IRC_NUMERICS_H__
#define __IRC_NUMERICS_H__

#define IRC_NUMERIC_MAX 999

#define IRC_RPL_WELCOME             001
#define IRC_RPL_YOURHOST            002
#define IRC_RPL_CREATED             003
#define IRC_RPL_MYINFO              004
#define IRC_RPL_ISUPPORT            005
#define IRC_RPL_USERHOST            302
#define IRC_RPL_ISON                303

#define IRC_RPL_AWAY                301
#define IRC_RPL_UNAWAY              305
#define IRC_RPL_NOWAWAY             306

#define IRC_RPL_WHOISUSER           311
#define IRC_RPL_WHOISSERVER         312
#define IRC_RPL_WHOISOPERATOR       313
#define IRC_RPL_WHOISIDLE           317
#define IRC_RPL_ENDOFWHOIS          318
#define IRC_RPL_WHOISCHANNELS       319
#define IRC_RPL_WHOISACCOUNT        330
#define IRC_RPL_WHOISREGNICK        307
#define IRC_RPL_WHOISSECURE         671
#define IRC_RPL_WHOISHOST           378
#define IRC_RPL_WHOISSPECIAL        320
#define IRC_RPL_WHOISMODES          379

#define IRC_RPL_WHOWASUSER          314
#define IRC_RPL_ENDOFWHOWAS         369

#define IRC_RPL_LIST                322
#define IRC_RPL_LISTEND             323

#define IRC_RPL_UNIQOPIS            325

#define IRC_RPL_CHANNELMODEIS       324

#define IRC_RPL_NOTOPIC             331
#define IRC_RPL_TOPIC               332
#define IRC_RPL_TOPICWHOTIME        333
#define IRC_RPL_CREATIONTIME        329

#define IRC_RPL_INVITING            341

#define IRC_RPL_INVITELIST          346
#define IRC_RPL_ENDOFINVITELIST     347

#define IRC_RPL_EXCEPTLIST          348
#define IRC_RPL_ENDOFEXCEPTLIST     349

#define IRC_RPL_VERSION             351

#define IRC_RPL_WHOREPLY            352
#define IRC_RPL_ENDOFWHO            315

#define IRC_RPL_NAMREPLY            353
#define IRC_RPL_ENDOFNAMES          366

#define IRC_RPL_LINKS               364
#define IRC_RPL_ENDOFLINKS          365

#define IRC_RPL_BANLIST             367
#define IRC_RPL_ENDOFBANLIST        368

#define IRC_RPL_INFO                371
#define IRC_RPL_ENDOFINFO           374

#define IRC_RPL_MOTDSTART           375
#define IRC_RPL_MOTD                372
#define IRC_RPL_ENDOFMOTD           376

#define IRC_RPL_YOUREOPER           381

#define IRC_RPL_REHASHING           382

#define IRC_RPL_TIME                391

#define IRC_RPL_TRACELINK           200
#define IRC_RPL_TRACECONNECTING     201
#define IRC_RPL_TRACEHANDSHAKE      202
#define IRC_RPL_TRACEUNKNOWN        203
#define IRC_RPL_TRACEOPERATOR       204
#define IRC_RPL_TRACEUSER           205
#define IRC_RPL_TRACESERVER         206
#define IRC_RPL_TRACESERVICE        207
#define IRC_RPL_TRACENEWTYPE        208
#define IRC_RPL_TRACECLASS          209
#define IRC_RPL_TRACELOG            261
#define IRC_RPL_TRACEEND            262

#define IRC_RPL_STATSLINKINFO       211
#define IRC_RPL_STATSCOMMANDS       212
#define IRC_RPL_STATSUPTIME         242
#define IRC_RPL_STATSOLINE          243
#define IRC_RPL_ENDOFSTATS          219

#define IRC_RPL_UMODEIS             221

#define IRC_RPL_SERVLIST            234
#define IRC_RPL_SERVLISTEND         235

#define IRC_RPL_LUSERCLIENT         251
#define IRC_RPL_LUSEROP             252
#define IRC_RPL_LUSERUNKNOWN        253
#define IRC_RPL_LUSERCHANNELS       254
#define IRC_RPL_LUSERME             255

#define IRC_RPL_ADMINME             256
#define IRC_RPL_ADMINLOC1           257
#define IRC_RPL_ADMINLOC2           258
#define IRC_RPL_ADMINEMAIL          259

#define IRC_RPL_TRYAGAIN            263

#define IRC_ERR_NOSUCHNICK          401
#define IRC_ERR_NOSUCHSERVER        402
#define IRC_ERR_NOSUCHCHANNEL       403
#define IRC_ERR_CANNOTSENDTOCHAN    404
#define IRC_ERR_TOOMANYCHANNELS     405
#define IRC_ERR_WASNOSUCHNICK       406
#define IRC_ERR_TOOMANYTARGETS      407
#define IRC_ERR_NOSUCHSERVICE       408
#define IRC_ERR_NOORIGIN            409
#define IRC_ERR_NORECEPIENT         411
#define IRC_ERR_NOTEXTTOSEND        412
#define IRC_ERR_NOTOPLEVEL          413
#define IRC_ERR_WILDTOPLEVEL        414
#define IRC_ERR_BADMASK             415
#define IRC_ERR_UNKNOWNCOMMAND      421
#define IRC_ERR_NOMOTD              422
#define IRC_ERR_NOADMININFO         423
#define IRC_ERR_FILEERROR           424
#define IRC_ERR_NONICKNAMEGIVEN     431
#define IRC_ERR_ERRORNEUSNICKNAME   432
#define IRC_ERR_NICKNAMEINUSE       433
#define IRC_ERR_NICKCOLLISION       436
#define IRC_ERR_UNAVAILRESOURCE     437
#define IRC_ERR_USERNOTINCHANNEL    441
#define IRC_ERR_NOTONCHANNEL        442
#define IRC_ERR_USERONCHANNEL       443
#define IRC_ERR_NOLOGIN             444
#define IRC_ERR_USERSDISABLED       446
#define IRC_ERR_NOTREGISTERED       451
#define IRC_ERR_NEEDMOREPARAMS      461
#define IRC_ERR_ALREADYREGISTERED   462
#define IRC_ERR_NOPERMFORHOST       463
#define IRC_ERR_PASSWDMISMATCH      464
#define IRC_ERR_YOUREBANNEDCREEP    465
#define IRC_ERR_YOUWILLBEBANNED     466
#define IRC_ERR_KEYSET              467
#define IRC_ERR_CHANNELISFULL       471
#define IRC_ERR_UNKNOWNMODE         472
#define IRC_ERR_INVITEONLYCHAN      473
#define IRC_ERR_BANNEDFROMCHAN      474
#define IRC_ERR_BADCHANNELKEY       475
#define IRC_ERR_BADCHANMASK         476
#define IRC_ERR_NOCHANMODES         477
#define IRC_ERR_BANLISTFULL         478
#define IRC_ERR_NOPRIVILEGES        481
#define IRC_ERR_CHANOPRIVSNEEDED    482
#define IRC_ERR_CANTKILLSERVER      483
#define IRC_ERR_RESTRICTED          484
#define IRC_ERR_UNIQOPPRIVSNEEDED   485
#define IRC_ERR_NOOPERHOST          491
#define IRC_ERR_UMODEUNKNOWNFLAG    501
#define IRC_ERR_USERSDONTMATCH      502

#endif // __IRC_NUMERICS_H__

// vim: expandtab:tw=80:tabstop=4:shiftwidth=4:softtabstop=4
