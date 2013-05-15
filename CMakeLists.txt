cmake_minimum_required(VERSION 2.6)
project(SquirrelChat C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

add_subdirectory(src)

# vim: expandtab:tabstop=4:shiftwidth=4:softtabstop=4:tw=80
