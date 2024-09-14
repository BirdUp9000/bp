# Cmake required
cmake_minimum_required(VERSION 3.10)

# Set project name and version
project(BSCII VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set the output directories for build artifacts
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add the header files directory
include_directories(${PROJECT_SOURCE_DIR}/include)

# Add source files
file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

# Add the executable
add_executable(BSCII ${SOURCES})

# Add AddressSanitizer flags to the executable target
target_compile_options(BSCII PRIVATE -fsanitize=address -Wall -Wextra -pedantic -O2)

# Add a custom command to run the executable after building it
add_custom_command(TARGET BSCII
    COMMENT "Running BSCII after build..."
    COMMENT "___________________________________________________________"
    POST_BUILD
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/BSCII
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Add a custom target that depends on the main executable
add_custom_target(run ALL
    DEPENDS BSCII
)

