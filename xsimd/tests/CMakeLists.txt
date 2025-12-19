cmake_minimum_required(VERSION 3.10)
project(xsimd_new_functions_test C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Source files
set(XSIMD_SOURCES
    ../src/xsimd.c
)

set(TEST_SOURCES
    test_new_functions.c
)

# Create test executable
add_executable(test_new_functions
    ${XSIMD_SOURCES}
    ${TEST_SOURCES}
)

# Create SIMD paths test executable
add_executable(test_simd_paths
    ${XSIMD_SOURCES}
    test_simd_paths.c
)

# Include directories
target_include_directories(test_new_functions PRIVATE
    ../src
)

target_include_directories(test_simd_paths PRIVATE
    ../src
)

# Enable optimizations for Release build
if(MSVC)
    target_compile_options(test_new_functions PRIVATE
        /W4
        /O2
        /arch:AVX2
    )
    target_compile_options(test_simd_paths PRIVATE
        /W4
        /O2
        /arch:AVX2
    )
else()
    target_compile_options(test_new_functions PRIVATE
        -Wall
        -Wextra
        -O3
        -march=native
    )
    target_compile_options(test_simd_paths PRIVATE
        -Wall
        -Wextra
        -O3
        -march=native
    )
endif()

# Set output directory
set_target_properties(test_new_functions PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

set_target_properties(test_simd_paths PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

message(STATUS "XSIMD new functions test configured")
message(STATUS "Build the test with: cmake --build . --config Release")
message(STATUS "Run the test: ./bin/test_new_functions (or .\\bin\\Release\\test_new_functions.exe on Windows)")
