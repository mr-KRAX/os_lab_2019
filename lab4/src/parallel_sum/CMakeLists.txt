cmake_minimum_required(VERSION 3.8)

project(parallel_sum)


set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(parallel_sum parallel_sum.c sum.c utils.c)
target_link_libraries(${PROJECT_NAME} pthread)