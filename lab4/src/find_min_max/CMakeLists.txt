cmake_minimum_required(VERSION 3.8)

project(find_min_max)

add_executable(find_min_max parallel_min_max.c find_min_max.c utils.c)