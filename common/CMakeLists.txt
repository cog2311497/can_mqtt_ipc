cmake_minimum_required(VERSION 3.16)


add_library(common_lib STATIC
    can/linux/sockets/can_receiver.cpp
    can/linux/sockets/can_sender.cpp
    config/config_parser.cpp
    sensors/sensor_data.cpp
    sensors/emulated/sensor_data_source.cpp
)

target_include_directories(common_lib
    PUBLIC
        ${PROJECT_SOURCE_DIR}/common
)

target_compile_features(common_lib PUBLIC cxx_std_20)


target_compile_options(common_lib
    PRIVATE -Wall -Wextra -Werror
)
