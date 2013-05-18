cmake_minimum_required(VERSION 2.6)

include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAG_OTHER})

add_executable(squirrelchat
               main.c
               net_io.c
               net_input_handler.c
               irc_network.c
               commands.c
               trie.c
               ui/chat_window.c
               ui/network_tree.c
               ui/buffer.c
               ui/main_menu_bar.c)

target_link_libraries(squirrelchat ${GTK3_LIBRARIES})

# vim: expandtab:tabstop=4:shiftwidth=4:softtabstop=4:tw=80
