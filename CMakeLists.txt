cmake_minimum_required(VERSION 2.6)
project(TailChat C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAG_OTHER})

add_executable(TailChat
               main.c
               buffers.c
               net_io.c
               net_input_handler.c
               ui/chat_window.c
               ui/network_tree.c
               ui/command_box.c
               ui/chat_viewer.c
               ui/main_menu_bar.c)

target_link_libraries(TailChat ${GTK3_LIBRARIES} pthread)

# vim: expandtab:tabstop=4:shiftwidth=4:softtabstop=4:tw=80
