cmake_minimum_required(VERSION 3.10)
project(smashedit C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set output directory to bin/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Option to use PDCurses (for Windows)
option(USE_PDCURSES "Use PDCurses instead of ncurses" OFF)

if(USE_PDCURSES)
    # Use PDCurses built from source
    set(PDCURSES_DIR "${CMAKE_SOURCE_DIR}/external/PDCurses")
    set(CURSES_INCLUDE_DIRS "${PDCURSES_DIR}")
    set(CURSES_LIBRARIES "${PDCURSES_DIR}/wincon/pdcurses.a")
    add_definitions(-DPDC_WIDE)
    add_definitions(-DPDCURSES)
    # PDCurses wincon backend needs user32 for Windows APIs
    list(APPEND CURSES_LIBRARIES user32)
    message(STATUS "Using PDCurses from: ${PDCURSES_DIR}")
else()
    # Use pkg-config to find ncurses if available
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(CURSES ncursesw)
        if(NOT CURSES_FOUND)
            pkg_check_modules(CURSES ncurses)
        endif()
    endif()

    # Find ncursesw library (fallback if pkg-config didn't work)
    if(NOT CURSES_FOUND)
        find_library(NCURSESW_LIBRARY NAMES ncursesw ncurses
            PATHS
                /usr/lib
                /usr/local/lib
                /opt/homebrew/opt/ncurses/lib
        )
        find_path(CURSES_INCLUDE_DIR ncurses.h
            PATHS
                /usr/include/ncursesw
                /usr/include/ncurses
                /usr/include
                /usr/local/include
                /opt/homebrew/opt/ncurses/include
        )
        set(CURSES_LIBRARIES ${NCURSESW_LIBRARY})
        set(CURSES_INCLUDE_DIRS ${CURSES_INCLUDE_DIR})
    endif()
endif()

# Include directories
if(CURSES_INCLUDE_DIRS)
    include_directories(${CURSES_INCLUDE_DIRS})
endif()
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)

# Link directories (needed for keg-only homebrew packages on macOS)
if(CURSES_LIBRARY_DIRS)
    link_directories(${CURSES_LIBRARY_DIRS})
endif()

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
    src/explorer.c
)

# Executable
add_executable(smashedit ${SOURCES})

# Static linking option for self-contained binaries
option(STATIC_BUILD "Build a statically linked executable" OFF)

if(STATIC_BUILD AND NOT WIN32 AND NOT MINGW)
    # Static linking for Linux and macOS only
    # Windows PDCurses doesn't support static linking easily, bundle DLLs instead
    if(APPLE)
        # macOS: use static libraries from homebrew
        find_library(CURSES_STATIC NAMES libncursesw.a libncurses.a
            PATHS /opt/homebrew/opt/ncurses/lib /usr/local/opt/ncurses/lib
            NO_DEFAULT_PATH)
        if(CURSES_STATIC)
            target_link_libraries(smashedit ${CURSES_STATIC})
        else()
            target_link_libraries(smashedit ${CURSES_LIBRARIES})
        endif()
    else()
        # Linux: link statically
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
        find_library(CURSES_STATIC NAMES libncursesw.a libncurses.a
            PATHS /usr/lib/x86_64-linux-gnu /usr/lib /usr/local/lib)
        find_library(TINFO_STATIC NAMES libtinfo.a
            PATHS /usr/lib/x86_64-linux-gnu /usr/lib /usr/local/lib)
        if(CURSES_STATIC)
            target_link_libraries(smashedit ${CURSES_STATIC})
            if(TINFO_STATIC)
                target_link_libraries(smashedit ${TINFO_STATIC})
            endif()
        else()
            target_link_libraries(smashedit ${CURSES_LIBRARIES})
        endif()
    endif()
else()
    # Dynamic linking (default, and required for Windows)
    target_link_libraries(smashedit ${CURSES_LIBRARIES})
endif()

# Add required definitions for wide character support and signals
target_compile_definitions(smashedit PRIVATE _XOPEN_SOURCE=700 _XOPEN_SOURCE_EXTENDED)

# Platform-specific definitions
if(APPLE)
    # macOS needs _DARWIN_C_SOURCE to expose SIGWINCH with _XOPEN_SOURCE
    target_compile_definitions(smashedit PRIVATE _DARWIN_C_SOURCE)
endif()

# Compiler warnings
target_compile_options(smashedit PRIVATE -Wall -Wextra -pedantic)
