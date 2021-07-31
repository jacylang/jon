cmake_minimum_required(VERSION 3.16)
project(jon)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS_DEBUG "-g -Og")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS "-std=c++17")

include_directories("${PROJECT_SOURCE_DIR}/include")

add_executable(jon test.cpp include/Lexer.h include/utils.h include/Parser.h include/ast.h include/AstPrinter.h jon.h)
