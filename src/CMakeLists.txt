# Copyright (C) 2013 Stephen Chandler Paul
#
# This file is free software: you may copy it, redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 2 of this License or (at your option) any
# later version.
#
# This file is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
# A PARTICULAR PURPOSE. See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with
# this program. If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 2.6)

include_directories(${GTK3_INCLUDE_DIRS} ${GNUTLS_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS} ${GNUTLS_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAG_OTHER})

add_executable(squirrelchat
               main.c
               net_io.c
               net_input_handler.c
               irc_network.c
               commands.c
               builtin_commands.c
               trie.c
               chat.c
               message_parser.c
               message_types.c
               cmd_responses.c
               numerics.c
               casemap.c
               errors.c
               ctcp.c
               builtin_ctcp_requests.c
               builtin_ctcp_responses.c
               settings.c
               ssl.c
               connection_setup.c
               ui/chat_window.c
               ui/network_tree.c
               ui/buffer.c
               ui/main_menu_bar.c
               ui/command_box.c
               ui/buffer_view.c
               ui/user_list.c
               ui/settings_dialog.c)

target_link_libraries(squirrelchat ${GTK3_LIBRARIES} ${GNUTLS_LIBRARIES})

# vim: expandtab:tabstop=4:shiftwidth=4:softtabstop=4:tw=80
