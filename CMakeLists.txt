cmake_minimum_required(VERSION 3.24)

# default on
option(CMAKE_EXPORT_COMPILE_COMMANDS "Export compile_commands.json" ON)

project(aico LANGUAGES C CXX)

option(AICO_BUILD_TESTS "Build test suite" ON)
option(AICO_ENABLE_WARN "Build with compiler warnings" ON)

if(AICO_BUILD_TESTS)
    enable_testing()
endif()
# Enable warnings
if(AICO_ENABLE_WARN)
    if (MSVC)
        add_compile_options(/W4)
    else()
        add_compile_options(-Wall -Wextra -Wpedantic)
    endif()
endif()

# Set C++ standard globally
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)

# Set output directories
# For static/shared libraries
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# For executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Handle multi-config generators (Debug/Release)
foreach(OUTPUTCONFIG IN LISTS CMAKE_CONFIGURATION_TYPES)
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG_UPPER)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} ${CMAKE_BINARY_DIR}/bin)
endforeach()

add_subdirectory(common)
add_subdirectory(vendor)
add_subdirectory(src)
if(AICO_BUILD_TESTS)
    add_subdirectory(test)
endif()
