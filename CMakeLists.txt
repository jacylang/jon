cmake_minimum_required(VERSION 3.16)
project(jon)

set(CMAKE_CXX_STANDARD 17)

add_library(jon test.cpp include/Lexer.h include/utils.h include/Parser.h include/ast.h include/AstPrinter.h)