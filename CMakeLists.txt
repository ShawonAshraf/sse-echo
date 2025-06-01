cmake_minimum_required(VERSION 3.16)

# Project name and version
project(sse-echo VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set compiler flags for better debugging and warnings
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Default to Release build if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Add source files
set(SOURCES
    sse-echo/main.cpp
    sse-echo/Exceptions.cpp
    sse-echo/SocketGuard.cpp
)

# Add header files (for IDEs that show headers in project)
set(HEADERS
    sse-echo/Exceptions.hpp
    sse-echo/SocketGuard.hpp
    sse-echo/Utils.hpp
)

# Create the executable
add_executable(sse-echo ${SOURCES} ${HEADERS})

# Link against pthread library (required for std::thread on Unix systems)
find_package(Threads REQUIRED)
target_link_libraries(sse-echo PRIVATE Threads::Threads)

# Set target properties
set_target_properties(sse-echo PROPERTIES
    OUTPUT_NAME "sse-echo"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Include directories (so the .cpp files can find the .hpp files)
target_include_directories(sse-echo PRIVATE sse-echo)

# Platform-specific settings
if(UNIX AND NOT APPLE)
    # Linux-specific settings if needed
    target_compile_definitions(sse-echo PRIVATE LINUX)
elseif(APPLE)
    # macOS-specific settings if needed
    target_compile_definitions(sse-echo PRIVATE MACOS)
endif()

# Optional: Enable extra warnings and treat warnings as errors in Debug mode
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(sse-echo PRIVATE -Werror)
    endif()
endif()

# Print build information
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}") 
