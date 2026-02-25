cmake_minimum_required(VERSION 3.16)
project(bridge LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PahoMqttCpp REQUIRED)

add_executable(bridge main.cpp bridge.cpp)

target_link_libraries(bridge PRIVATE PahoMqttCpp::paho-mqttpp3 common_lib)
