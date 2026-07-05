cmake_minimum_required(VERSION 3.18)

project(xsimd LANGUAGES C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(XSIMD_ENABLE_CUDA "Enable CUDA GPU acceleration" ON)

include_directories(${PROJECT_SOURCE_DIR}/src/)

string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" XSIMD_SYSTEM_PROCESSOR)
set(XSIMD_X86 OFF)
if(XSIMD_SYSTEM_PROCESSOR MATCHES "^(x86|i[3-6]86|x86_64|amd64)$")
    set(XSIMD_X86 ON)
endif()

# ── SSE2 library ──────────────────────────────────────────────────────────────
if(XSIMD_X86)
    add_library(xsimd_sse2 STATIC
        ${PROJECT_SOURCE_DIR}/src/xsimd_sse2.c
    )
    set_target_properties(xsimd_sse2 PROPERTIES LINKER_LANGUAGE C)
    target_compile_definitions(xsimd_sse2 PRIVATE USE_SSE2)
endif()

# ── AVX2 library ──────────────────────────────────────────────────────────────
if(XSIMD_X86)
    add_library(xsimd_avx2 STATIC
        ${PROJECT_SOURCE_DIR}/src/xsimd_avx2.c
    )
    set_target_properties(xsimd_avx2 PROPERTIES LINKER_LANGUAGE C)
    target_compile_definitions(xsimd_avx2 PRIVATE USE_AVX2)
endif()

# ── main library ──────────────────────────────────────────────────────────────
add_library(xsimd STATIC
    ${PROJECT_SOURCE_DIR}/src/xsimd.c
)
set_target_properties(xsimd PROPERTIES LINKER_LANGUAGE C)

if(XSIMD_X86)
    target_link_libraries(xsimd PUBLIC xsimd_sse2 xsimd_avx2)
endif()

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

if(XSIMD_X86)
    if(MSVC)
        add_definitions(-D_CRT_SECURE_NO_WARNINGS)
        target_compile_options(xsimd_sse2 PRIVATE /arch:SSE2)
        target_compile_options(xsimd_avx2 PRIVATE /arch:AVX2)
    elseif(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(xsimd_sse2 PRIVATE -msse -msse2)
        target_compile_options(xsimd_avx2 PRIVATE -msse -msse2 -mavx -mavx2)
    endif()
endif()

# ── optional CUDA library ─────────────────────────────────────────────────────
if(XSIMD_ENABLE_CUDA)
    include(CheckLanguage)
    check_language(CUDA)
    if(CMAKE_CUDA_COMPILER)
        enable_language(CUDA)
        find_package(CUDAToolkit REQUIRED)

        add_library(xsimd_cuda STATIC
            ${PROJECT_SOURCE_DIR}/src/xsimd_cuda.cu
        )
        target_include_directories(xsimd_cuda PRIVATE ${PROJECT_SOURCE_DIR}/src/)
        set_target_properties(xsimd_cuda PROPERTIES
            CUDA_STANDARD          11
            CUDA_STANDARD_REQUIRED ON
            CUDA_ARCHITECTURES     "50;61;75;86;89"
        )
        target_link_libraries(xsimd_cuda PUBLIC CUDA::cudart)

        target_compile_definitions(xsimd PUBLIC XSIMD_CUDA)
        target_link_libraries(xsimd PUBLIC xsimd_cuda)

        message(STATUS "xsimd: CUDA ${CUDAToolkit_VERSION} found — xsimd_cuda enabled")
    else()
        message(STATUS "xsimd: CUDA compiler not found — xsimd_cuda disabled")
    endif()
endif()
