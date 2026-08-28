add_library(ecu_core STATIC isotp.c)
target_include_directories(ecu_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
