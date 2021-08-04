cmake_minimum_required(VERSION 3.16)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
include(jon)

project(jon VERSION 0.0.5 LANGUAGES CXX)

### Compiler flags
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS_DEBUG "-g -Og")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS "-std=c++17")

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
        -Wno-logical-op-parentheses
)

include(GNUInstallDirs)

set(JON_STANDALONE_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(JON_STANDALONE_PROJECT ON)
endif()

if (JON_BUILD_EXAMPLES)
    add_subdirectory(src)
endif()

add_library(jon INTERFACE)
add_library(jon::jon ALIAS jon)
add_subdirectory(include)

### Target
set(JON_TARGET_NAME ${PROJECT_NAME})
