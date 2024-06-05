cmake_minimum_required(VERSION 3.10)
set(CMAKE_BUILD_TYPE Debug)
project(projLibSandbox)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(Sandbox SHARED 
    Foo.cpp
)

set_target_properties(Sandbox PROPERTIES LINKER_LANGUAGE CXX)
