# CMake required
cmake_minimum_required(VERSION 3.10)

# Set project name and version
project(BSCII VERSION 1.0)

# Check if the compiler supports C++20
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++20" COMPILER_SUPPORTS_CXX20)

if (COMPILER_SUPPORTS_CXX20)
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED True)
else()
  message(FATAL_ERROR "The compiler does not support C++20.")
endif()

# Set the output directories for build artifacts
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add the header files directory
include_directories(${PROJECT_SOURCE_DIR}/include)

# Create a library from the header file (bmp_reader.hpp doesn't need compilation)
add_library(BMP_LIB INTERFACE)

# Link the header to the library
target_include_directories(BMP_LIB INTERFACE ${PROJECT_SOURCE_DIR}/include)

# Add the executable for the tests
add_executable(BSCII_tests ${PROJECT_SOURCE_DIR}/test/test.cpp)

# Link the BMP_LIB library to the test executable
target_link_libraries(BSCII_tests BMP_LIB)

# Add AddressSanitizer flags for the test executable
target_compile_options(BSCII_tests PRIVATE -fsanitize=address -Wall -Wextra -pedantic -O2)
target_link_options(BSCII_tests PRIVATE -fsanitize=address)

# Add a custom command to run the tests after building them
add_custom_command(TARGET BSCII_tests
    COMMENT "Running tests after build..."
    POST_BUILD
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/BSCII_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Add a custom target that depends on the test executable
add_custom_target(test_run ALL
    DEPENDS BSCII_tests
)
