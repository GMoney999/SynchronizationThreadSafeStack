cmake_minimum_required(VERSION 3.28)
project(SynchronizationThreadSafeStack C)

set(CMAKE_C_STANDARD 11)

add_executable(SynchronizationThreadSafeStack stack_ptr.c)
