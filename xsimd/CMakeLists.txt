cmake_minimum_required(VERSION 3.14)

project(xsimd LANGUAGES C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories(${PROJECT_SOURCE_DIR}/src/)

add_library(xsimd STATIC
${PROJECT_SOURCE_DIR}/src/xsimd.c
)

set_target_properties(xsimd PROPERTIES LINKER_LANGUAGE C)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    # Enable SSE, AVX, AVX2 for MSVC
    target_compile_options(xsimd PRIVATE /arch:AVX2)
elseif(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    # Enable SSE, AVX, AVX2, and POPCNT for GCC/Clang
    target_compile_options(xsimd PRIVATE -msse -msse2 -mavx -mavx2)
endif()
