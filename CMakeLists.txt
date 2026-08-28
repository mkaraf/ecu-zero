cmake_minimum_required(VERSION 3.16)
project(ecu_zero C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Wconversion -Werror)

add_library(unity STATIC vendor/Unity/src/unity.c)
target_include_directories(unity PUBLIC vendor/Unity/src)

enable_testing()
add_subdirectory(src)
add_subdirectory(tests)
