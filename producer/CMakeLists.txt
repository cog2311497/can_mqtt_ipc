cmake_minimum_required(VERSION 3.16)
project(producer CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(EXTERNAL_SOURCES
    ../common/can/linux/sockets/can_sender.cpp
    ../common/sensors/emulated/sensor_data_source.cpp
    ../common/config/config_parser.cpp
    ../common/sensors/sensor_data.cpp
)

add_executable(producer main.cpp producer.cpp ${EXTERNAL_SOURCES})

target_include_directories(producer PRIVATE ../common)
