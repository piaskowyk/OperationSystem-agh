cmake_minimum_required(VERSION 3.13)
project(lab8 C)

set(CMAKE_C_STANDARD 11)
link_libraries("pthread")
link_libraries("m")
add_executable(lab8 zad1/main.c zad1/utils.h zad1/utils.c)