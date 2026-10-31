cmake_minimum_required(VERSION 3.16)
project(ecu_zero C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Wconversion -Werror)

option(ECU_SANITIZE "Build host code with ASan and UBSan" OFF)
if(ECU_SANITIZE)
    set(ECU_SANITIZE_FLAGS -fsanitize=address,undefined
                           -fno-sanitize-recover=all
                           -fno-omit-frame-pointer)
    add_compile_options(${ECU_SANITIZE_FLAGS})
    add_link_options(${ECU_SANITIZE_FLAGS})
endif()

add_library(unity STATIC vendor/Unity/src/unity.c)
target_include_directories(unity PUBLIC vendor/Unity/src)

enable_testing()
add_subdirectory(src)
add_subdirectory(tests)
