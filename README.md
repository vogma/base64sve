# Base64 Vectorized Implementation for ARM's SVE

This repository contains a vectorized implementation of the Base64 encoding and decoding algorithm, optimized for ARM's SVE (Scalable Vector Extension). The project uses ARM's SVE to accelerate the Base64 algorithm by processing data in parallel.

## Requirements

- **ARM architecture with SVE support**: This implementation is designed to take advantage of ARM's SVE extension.
- **CMake**: Version 3.20 or higher.
- **ARM-compatible compiler**: For example, `aarch64-none-linux-gnu-gcc` for cross-compiling.

## Build Instructions

### Native Build

To build the project on a native ARM machine that supports SVE, follow these steps:

   ```bash
   git clone https://github.com/vogma/base64sve.git
   cd base64-sve
   mkdir build && cd build
   cmake .. && make
   ```

5. The executable should now be available in the `build` directory.

### Cross-Compiling for ARM

If you are cross-compiling for an ARM architecture from a different platform, you will need to specify the correct cross-compiler.

   ```bash
   mkdir build && cd build
   CC=aarch64-none-linux-gnu-gcc cmake ..
   cd build
   make
   ```
--- 
