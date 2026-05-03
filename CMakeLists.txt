cmake_minimum_required(VERSION 3.20)

project(game LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_PREFIX_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/SDL3-3.4.4/build"
    "${CMAKE_CURRENT_SOURCE_DIR}/SDL_image/build"
)

find_package(SDL3 REQUIRED CONFIG)
find_package(SDL3_image REQUIRED CONFIG)

add_executable(game
    scr/main.cpp
    scr/HUD.cpp
    scr/animation.cpp
    scr/player.cpp
    scr/slime.cpp
)

target_link_libraries(game PRIVATE
    SDL3::SDL3
    SDL3_image::SDL3_image
)

set_target_properties(game PROPERTIES
    BUILD_RPATH "${CMAKE_CURRENT_SOURCE_DIR}/SDL3-3.4.4/build;${CMAKE_CURRENT_SOURCE_DIR}/SDL_image/build"
)
