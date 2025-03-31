# ------------------------------------------------------------------
# Project configuration
# ------------------------------------------------------------------
cmake_minimum_required(VERSION 3.10)

project(BP VERSION 0.0.0
        DESCRIPTION "BP - BMP PHOTOS PROBE LIBRARY"
        HOMEPAGE_URL "https://github.com/BirdUp9000/bp"
        LANGUAGES CXX
)

# ------------------------------------------------------------------
# Check if the compiler supports C++20
# ------------------------------------------------------------------
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++20" COMPILER_SUPPORTS_CXX20)

if (COMPILER_SUPPORTS_CXX20)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED True)
else()
    message(FATAL_ERROR "The compiler does not support C++20.")
endif()

# ------------------------------------------------------------------
# Catch2 configuration
# ------------------------------------------------------------------
set(CATCH_SRC ${CMAKE_CURRENT_SOURCE_DIR}/external/catch2/catch_amalgamated.cpp)

# ------------------------------------------------------------------
# Generate compile commands
# ------------------------------------------------------------------
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ------------------------------------------------------------------
# Default build type: If no build type is specified, use Release.
# ------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

# ------------------------------------------------------------------
# Header-only library
# ------------------------------------------------------------------
add_library(bp INTERFACE)
target_include_directories(bp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# ------------------------------------------------------------------
# Tests
# ------------------------------------------------------------------
enable_testing()

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
)

add_executable(run_tests ${TEST_SOURCES} ${CATCH_SRC})

target_include_directories(run_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/external/catch2
)

target_link_libraries(run_tests PRIVATE bp)
target_compile_definitions(run_tests PRIVATE UNIT_TEST)

target_compile_options(run_tests PRIVATE
        -O0
        -g
        -ggdb3
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        -D_GLIBCXX_DEBUG
)

target_link_options(run_tests PRIVATE
        -fsanitize=address
        -fprofile-arcs
        -ftest-coverage
)

target_compile_options(bp INTERFACE
        -O0
        -g
        -ggdb3
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        -D_GLIBCXX_DEBUG
)

target_link_options(bp INTERFACE
        -fsanitize=address
        -fprofile-arcs
        -ftest-coverage
)

add_test(NAME BP_tests COMMAND run_tests --colour-mode ansi)

add_custom_target(run_all_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS run_tests
        COMMENT "Running all tests..."
)

add_custom_command(TARGET run_tests POST_BUILD
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        COMMENT "Automatically running tests after build"
)