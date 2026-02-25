cmake_minimum_required(VERSION 3.16)
project(producer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(producer main.cpp producer.cpp)

target_link_libraries(producer PRIVATE common_lib)
