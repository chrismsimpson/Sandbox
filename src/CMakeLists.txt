cmake_minimum_required(VERSION 3.10)
set(CMAKE_BUILD_TYPE Debug)
project(projConsole)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 REQUIRED)
find_package(PNG REQUIRED)
find_package(raylib REQUIRED)

include_directories(${SDL3_INCLUDE_DIRS})
include_directories(${PNG_INCLUDE_DIRS})
include_directories(${raylib_INCLUDE_DIRS})

include_directories(../lib/sandbox)

add_executable(console 
    main.cpp
)

target_link_libraries(console ${SDL3_LIBRARIES} ${PNG_LIBRARIES} ${raylib_LIBRARIES} Sandbox)
