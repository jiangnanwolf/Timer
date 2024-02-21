cmake_minimum_required(VERSION 3.15)
project(timer)

set(CMAKE_CXX_STANDARD 14)

add_executable(timer main.cpp)

target_link_libraries(timer pthread)