cmake_minimum_required(VERSION 3.15)
project(sweep)

set(CMAKE_CXX_STANDARD 17)
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

include_directories(lib)

add_executable(sweep src/main.cpp)