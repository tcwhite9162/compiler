cmake_minimum_required(VERSION 3.16)
project(Compiler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS_RELEASE "-O3")


file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "src/*.cpp"
    "include/*.hpp"
)
add_executable(main ${SRC_FILES})

target_include_directories(main
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)


target_compile_options(main PRIVATE
    -Wall
    -Wextra
)
