cmake_minimum_required(VERSION 3.10)
project(CHIP-8)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB SOURCES "src/*.cpp")

add_executable(chip8 ${SOURCES})
target_link_libraries(chip8 SDL2)