# Project Configuration File: config.cmake

# Project metadata
set(PROJECT_NAME "p101_error")
set(PROJECT_VERSION "0.0.1")
set(PROJECT_DESCRIPTION "Error handling")
set(PROJECT_LANGUAGE "C")

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Common compiler flags
set(STANDARD_FLAGS
        -D_POSIX_C_SOURCE=200809L
        -D_XOPEN_SOURCE=700
        #-D_GNU_SOURCE
        #-D_DARWIN_C_SOURCE
        #-D__BSD_VISIBLE
        -Werror
)

# Define library targets
set(LIBRARY_TARGETS p101_error)

# Source files for the library
set(p101_error_SOURCES
        src/check.c
        src/error.c
)

# Header files for installation, maintaining subdirectory structure
set(p101_error_HEADERS
        include/p101_error/check.h
        include/p101_error/error.h
)

# Linked libraries required for this project
set(p101_error_LINK_LIBRARIES "")
