cmake_minimum_required(VERSION 3.15)
project(locktest)

set(CMAKE_CXX_STANDARD 14)

link_libraries(pthread)

add_executable(locktest main.cpp)

add_executable(spin_rw spin_rw.cpp)