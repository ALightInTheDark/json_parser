cmake_minimum_required(VERSION 3.10)

project(JsonParser VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(test test.cpp json_parse.hpp)
