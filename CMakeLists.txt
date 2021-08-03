cmake_minimum_required(VERSION 3.16)

project(jon VERSION 0.0.3 LANGUAGES CXX)

### Target
set(JON_TARGET_NAME ${PROJECT_NAME})

add_compile_options(
        # WARNINGS #
        -Wall
        -pedantic
        -pedantic-errors
        -Wextra
        -Wpedantic
        -Wcast-align
        -Wcast-qual
        -Wconversion
        -Wctor-dtor-privacy
        -Wenum-compare
        -Wfloat-equal
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Woverloaded-virtual
        -Wredundant-decls
        -Wsign-conversion
        -Wsign-promo
        -Wstrict-aliasing
        -ansi
        -Wunreachable-code
        -Wunused
        -Wunused-parameter

        -Wvariadic-macros
        -Wwrite-strings
)

### Compiler flags
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS_DEBUG "-g -Og")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS "-std=c++17")

include_directories("${PROJECT_SOURCE_DIR}/include")

add_executable(jon test.cpp include/jon/Lexer.h include/jon/utils.h include/jon/Parser.h include/jon/ast.h include/jon/Printer.h include/jon/jon.h)
