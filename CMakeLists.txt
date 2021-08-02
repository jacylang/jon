cmake_minimum_required(VERSION 3.16)

project(jon VERSION 0.0.1 LANGUAGES CXX)

### Target
set(JON_TARGET_NAME ${PROJECT_NAME})

add_library(${PROJECT_NAME}::${JON_TARGET_NAME} ALIAS ${JON_TARGET_NAME})


### Compiler flags
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS_DEBUG "-g -Og")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS "-std=c++17")

include_directories("${PROJECT_SOURCE_DIR}/include")

add_library(jon include/Lexer.h include/utils.h include/Parser.h include/ast.h include/Printer.h jon.h)
