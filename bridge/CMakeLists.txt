cmake_minimum_required(VERSION 3.16)
project(bridge CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PahoMqttCpp REQUIRED)

set(EXTERNAL_SOURCES
    ../common/can/linux/sockets/can_receiver.cpp
    ../common/config/config_parser.cpp
    ../common/sensors/sensor_data.cpp
)

add_executable(bridge main.cpp ${EXTERNAL_SOURCES})

target_include_directories(bridge PRIVATE ../common)

target_link_libraries(bridge PahoMqttCpp::paho-mqttpp3)
