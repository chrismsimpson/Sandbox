cmake_minimum_required(VERSION 3.10)
set(CMAKE_BUILD_TYPE Debug)
project(MyProject)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(lib/sandbox)
add_subdirectory(src)