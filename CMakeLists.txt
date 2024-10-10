cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
project(base64sve VERSION 0.1 LANGUAGES C)

# include(CheckCSourceRuns)

set(CMAKE_C_COMPILER aarch64-none-linux-gnu-gcc)

ADD_COMPILE_OPTIONS(-march=armv8.4-a+sve+sve2 -O3 -static -Wall -g)

# check_c_source_runs(
#     "
#     #include <stdio.h>
#     #include <arm_sve.h>

#     int main(int argc, char **argv){
#         #ifndef __ARM_FEATURE_SVE
#             #error \"Compiler is not able to compile sve instructions\"
#         #endif
#         return 0;
#     } 
#     "
#     CAN_COMPILE_SVE
# )

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