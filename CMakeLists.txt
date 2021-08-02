cmake_minimum_required(VERSION 3.16)

project(jon VERSION 0.0.2 LANGUAGES CXX)

### Target
set(JON_TARGET_NAME ${PROJECT_NAME})


### Compiler flags
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS_DEBUG "-g -Og")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS "-std=c++17")

include_directories("${PROJECT_SOURCE_DIR}/include")

add_library(jon include/jon/Lexer.h include/jon/utils.h include/jon/Parser.h include/jon/ast.h include/jon/Printer.h include/jon/jon.h)
