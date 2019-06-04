cmake_minimum_required(VERSION 3.14)
project(lab10 C)

set(CMAKE_C_STANDARD 11)
link_libraries("pthread")
#add_executable(lab10 zad1/client.c zad1/server.c zad1/utils.c)
#add_executable(lab10 zad1/client.c zad1/utils.c)
add_executable(lab10 zad1/server.c zad1/utils.c)