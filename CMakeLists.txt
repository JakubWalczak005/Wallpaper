cmake_minimum_required(VERSION 3.30)
project(Wallpaper)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG 11.1.4
)

FetchContent_Declare(
        SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 3.0.0
)


FetchContent_MakeAvailable(fmt SFML)
add_executable(Wallpaper main.cpp
        Folder.cpp
        Folder.h
        DropTarget.cpp
        DropTarget.h)
target_link_libraries(Wallpaper fmt::fmt SFML::Graphics shell32 Ole32)
target_compile_definitions(Wallpaper PRIVATE UNICODE _UNICODE)
