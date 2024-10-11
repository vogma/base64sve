cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
project(base64sve VERSION 0.1 LANGUAGES C)

ADD_COMPILE_OPTIONS(-march=armv8.4-a+sve -O3 -static -Wall -g)

# Add include directories
include_directories(${PROJECT_SOURCE_DIR}/lib/include)

# Specify source files
set(SOURCE_FILES 
    ${PROJECT_SOURCE_DIR}/lib/src/base64_encoding.c
    ${PROJECT_SOURCE_DIR}/lib/src/base64_decoding.c
    )

# Define the static library to be built with the specified source files
add_library(base64sve STATIC ${SOURCE_FILES})

#build base64 executable
add_executable(base64 ${PROJECT_SOURCE_DIR}/src/base64.c)
target_link_libraries(base64 base64sve)
