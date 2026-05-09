cmake_minimum_required(VERSION 3.16)

project(emoji_dungeon VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.10 REQUIRED COMPONENTS Core Gui Widgets Test)

include(CTest)

qt_standard_project_setup(REQUIRES 6.10)

add_subdirectory(src)