cmake_minimum_required(VERSION 2.6)
project(SquirrelChat C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

add_definitions("-D_POSIX_SOURCE")

# C11 is only partially supported in gcc versions 4.3 and up, and fully
# supported in versions 4.8 and up. So we need to account for this.
if(CMAKE_COMPILER_IS_GNUCC)
    execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
                    OUTPUT_VARIABLE GCC_VERSION)
    if (GCC_VERSION VERSION_LESS 4.3)
        message(FATAL_ERROR "gcc version 4.3 or greater is required")
    elseif(GCC_VERSION VERSION_LESS 4.7)
        add_definitions("-std=c1x")
    else(GCC_VERSION VERSION_LESS 4.3)
        add_definitions("-std=c11")
    endif(GCC_VERSION VERSION_LESS 4.3)
else(CMAKE_COMPILER_IS_GNUCC)
    add_definitions("-std=c11")
endif(CMAKE_COMPILER_IS_GNUCC)

add_subdirectory(src)

# vim: expandtab:tabstop=4:shiftwidth=4:softtabstop=4:tw=80
