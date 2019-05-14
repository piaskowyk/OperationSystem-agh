cmake_minimum_required(VERSION 3.13)
project(lab7 C)

set(CMAKE_C_STANDARD 11)

link_libraries("rt")
link_libraries("pthread")
add_executable(lab7 zad2/loader.c zad2/common.h)
