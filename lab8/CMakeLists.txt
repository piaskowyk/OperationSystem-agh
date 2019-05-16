cmake_minimum_required(VERSION 3.13)
project(lab8 C)

set(CMAKE_C_STANDARD 11)
link_libraries("pthread")
add_executable(lab8 zad1/main.c)