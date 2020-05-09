cmake_minimum_required(VERSION 2.8)
project(Tomasulo)

set(CMAKE_CXX_STANDARD 17)

# For macOS Homebrew installed boost library
include_directories("/usr/local/Cellar/boost/1.72.0_2/include/")

add_executable(Tomasulo main.cpp)