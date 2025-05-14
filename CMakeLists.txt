cmake_minimum_required(VERSION 3.31)
project(jank LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include_directories(
    include/
)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    src/*.cpp
)

add_executable(jank ${SRC_FILES})

if (ENABLE_COMPARISON)
    add_compile_definitions(jank PRIVATE ENABLE_COMPARISON)
endif()

# Janky solution to disable annoying warnings from raygui.h
# add_library(raygui INTERFACE)
# target_include_directories(raygui INTERFACE ${CMAKE_SOURCE_DIR}/include/raygui)

# if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
#     target_compile_options(raygui INTERFACE -w)
# elseif(MSVC)
#     target_compile_options(raygui INTERFACE /w)
# endif()

# target_link_libraries(jank PRIVATE raygui)

if (WIN32)
    # target_link_directories(jank PRIVATE lib/windows_x64)
    # target_link_libraries(jank PRIVATE raylib winmm gdi32)
elseif(UNIX)
    # target_link_directories(jank PRIVATE lib/linux_x64)
    # target_link_libraries(jank PRIVATE raylib)
endif()
