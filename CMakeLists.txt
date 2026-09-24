cmake_minimum_required(VERSION 3.16)
project(Platformer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(game_logic src/game.cpp)
target_include_directories(game_logic PUBLIC src)
if(MSVC)
    target_compile_options(game_logic PRIVATE /W4)
else()
    target_compile_options(game_logic PRIVATE -Wall -Wextra -Wpedantic)
endif()

