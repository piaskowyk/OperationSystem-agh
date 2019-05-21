cmake_minimum_required(VERSION 3.14)
project(lab9 C)

set(CMAKE_C_STANDARD 11)
link_libraries("pthread")
add_executable(lab9 zad1/main.c)