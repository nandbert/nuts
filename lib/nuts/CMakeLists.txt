#
#  CONTENT
#    make nuts available in CMake projects by adding
#    "add_subdirectory(.../path/to/here nuts)"
#
#  AUTHOR
#    Norbert Stoeffler
#
#  DATE
#    2022-10-26
#
#  LANGUAGE
#    CMake
#
cmake_minimum_required(VERSION 3.10)
include_guard(GLOBAL)

project(nuts)
add_library(nuts debug.c pic.c strmem.c list.c win.c)
target_include_directories(nuts PUBLIC ..)
target_link_libraries(nuts X11)

