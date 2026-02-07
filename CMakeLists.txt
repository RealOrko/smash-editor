cmake_minimum_required(VERSION 3.10)
project(smashedit C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set output directory to bin/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Use pkg-config to find ncurses if available
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(NCURSES ncursesw)
    if(NOT NCURSES_FOUND)
        pkg_check_modules(NCURSES ncurses)
    endif()
endif()

# Find ncursesw library (fallback if pkg-config didn't work)
if(NOT NCURSES_FOUND)
    find_library(NCURSESW_LIBRARY NAMES ncursesw ncurses
        PATHS
            /usr/lib
            /usr/local/lib
            /opt/homebrew/opt/ncurses/lib
    )
    find_path(NCURSES_INCLUDE_DIR ncurses.h
        PATHS
            /usr/include/ncursesw
            /usr/include/ncurses
            /usr/include
            /usr/local/include
            /opt/homebrew/opt/ncurses/include
    )
    set(NCURSES_LIBRARIES ${NCURSESW_LIBRARY})
    set(NCURSES_INCLUDE_DIRS ${NCURSES_INCLUDE_DIR})
endif()

# Include directories
if(NCURSES_INCLUDE_DIRS)
    include_directories(${NCURSES_INCLUDE_DIRS})
endif()
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)

# Source files
set(SOURCES
    src/main.c
    src/editor.c
    src/buffer.c
    src/display.c
    src/smenu.c
    src/dialog.c
    src/input.c
    src/search.c
    src/file.c
    src/clipboard.c
    src/undo.c
)

# Executable
add_executable(smashedit ${SOURCES})

# Link ncurses
target_link_libraries(smashedit ${NCURSES_LIBRARIES})

# Add required definitions for wide character support and signals
target_compile_definitions(smashedit PRIVATE _XOPEN_SOURCE=700 _XOPEN_SOURCE_EXTENDED)

# Platform-specific definitions
if(APPLE)
    # macOS needs _DARWIN_C_SOURCE to expose SIGWINCH with _XOPEN_SOURCE
    target_compile_definitions(smashedit PRIVATE _DARWIN_C_SOURCE)
endif()

# Compiler warnings
target_compile_options(smashedit PRIVATE -Wall -Wextra -pedantic)
