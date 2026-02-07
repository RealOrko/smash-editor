cmake_minimum_required(VERSION 3.10)
project(smashedit C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set output directory to bin/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Find ncursesw (wide character support)
find_library(NCURSESW_LIBRARY ncursesw)
if(NOT NCURSESW_LIBRARY)
    find_library(NCURSESW_LIBRARY ncurses)
endif()

# Include directories
include_directories(/usr/include/ncursesw)
include_directories(/usr/include/ncurses)
include_directories(/usr/include)
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

# Link ncursesw
target_link_libraries(smashedit ${NCURSESW_LIBRARY})

# Add NCURSES_WIDECHAR definition
target_compile_definitions(smashedit PRIVATE _XOPEN_SOURCE=700 _XOPEN_SOURCE_EXTENDED)

# Compiler warnings
target_compile_options(smashedit PRIVATE -Wall -Wextra -pedantic)
