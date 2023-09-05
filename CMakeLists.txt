cmake_minimum_required(VERSION 3.24)
project(slim_hashmap C)

set(CMAKE_C_STANDARD 11)

add_library(slim_hashmap SHARED slim_hashmap_types.h slim_hashmap.h slim_hashmap.c slim_hashmap_default_impl.h slim_hashmap_default_impl.c)
