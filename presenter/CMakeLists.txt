cmake_minimum_required(VERSION 3.16)
project(presenter LANGUAGES NONE)

find_program(POETRY_EXECUTABLE poetry REQUIRED)

add_custom_target(poetry_config ALL
    COMMAND ${POETRY_EXECUTABLE} config --local virtualenvs.path ${CMAKE_CURRENT_BINARY_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(poetry_install ALL
    COMMAND ${POETRY_EXECUTABLE} install --no-interaction --no-root
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
