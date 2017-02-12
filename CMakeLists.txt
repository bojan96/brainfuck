cmake_minimum_required(VERSION 2.4)

set(PROJECT_NAME bf)
project(${PROJECT_NAME})

# Port to win
aux_source_directory(./src SRC_FILES)
add_definitions(-DNDEBUG)
add_compile_options(-O2 -std=c++11 -pedantic -pedantic-errors)
add_executable(${PROJECT_NAME} ${SRC_FILES})
