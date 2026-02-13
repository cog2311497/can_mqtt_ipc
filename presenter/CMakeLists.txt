cmake_minimum_required(VERSION 3.16)
project(presenter)

add_custom_target(install_python_deps
    COMMAND ${POETRY_EXECUTABLE} install
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Installing Python dependencies with Poetry"
)