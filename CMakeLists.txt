cmake_minimum_required(VERSION 3.28)
project(ex3b_naamaevenoz C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(ex3b_naamaevenoz
        linked_list.c
        linked_list.h
        markov_chain.c
        markov_chain.h
        snakes_and_ladders.c)
