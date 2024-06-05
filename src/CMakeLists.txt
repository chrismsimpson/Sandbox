cmake_minimum_required(VERSION 3.10)
set(CMAKE_BUILD_TYPE Debug)
project(projConsole)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(../lib/sandbox)

add_executable(console 
    main.cpp
)

target_link_libraries(console Sandbox)
