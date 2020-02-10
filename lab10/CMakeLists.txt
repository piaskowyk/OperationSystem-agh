cmake_minimum_required(VERSION 3.14)
project(lab10 C)

set(CMAKE_C_STANDARD 11)
link_libraries("pthread")
#add_executable(lab10 zad1/client.c zad1/server.c zad1/utils.c)
#add_executable(lab10 zad1/client.c zad1/utils.c zad1/list.h)
#add_executable(lab10_ zad1/server.c zad1/utils.c)

add_executable(lab10_client zad2/client.c zad2/utils.c zad2/list.h)
add_executable(lab10_server zad2/server.c zad2/utils.c)