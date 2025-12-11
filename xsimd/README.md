# xsimd

A simple SIMD utility library for the Formats project providing optimized memory operations with AVX2/SSE2 support.

## Overview

This is a C library following the same structure as other libraries in the Formats project (e.g., bzip2). It provides SIMD-accelerated operations for:
- Byte and pattern searching (Boyer-Moore-Horspool)
- ANSI text detection and validation
- Memory comparison
- Runtime feature enable/disable for testing and debugging

The library automatically detects CPU capabilities (AVX2, SSE2) and uses the best available implementation, with fallback to scalar code.

## Building

The library is compiled with SIMD instruction support enabled (SSE, SSE2, AVX, AVX2) when available on the target platform.

**Compiler Flags:**
- **MSVC**: `/arch:AVX2`
- **GCC/Clang**: `-msse -msse2 -mavx -mavx2`

These flags allow the compiler to generate SIMD instructions. The library performs runtime CPU detection to determine which features are actually available and falls back gracefully if the CPU doesn't support them.

### Using CMake

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Using qmake

```bash
qmake xsimd.pro
make
```

### Using Makefile directly

```bash
make
```

This will create a static library in the `libs/` directory with a platform and architecture-specific name:
- Windows: `xsimd-win-{arch}.lib`
- Linux: `libxsimd-unix-{arch}.a`
- macOS: `libxsimd-macos-{arch}.a`

## Integration

To use this library in a Qt project, add to your `.pri` file:

```qmake
include($$PWD/xsimd/xsimd.pri)
```

For CMake projects:

```cmake
add_subdirectory(${CMAKE_SOURCE_DIR}/../../_mylibs/Formats/xsimd xsimd)
target_link_libraries(your_target xsimd)
add_definitions(-DXSIMD_ENABLE)
```

## API Reference

### Initialization
```c
void xsimd_init(void);                          // Initialize library and detect CPU features
void xsimd_cleanup(void);                       // Clean up library resources
xsimd_uint32 xsimd_get_features(void);          // Get detected CPU features
```

### Feature Control
```c
xsimd_uint32 xsimd_get_enabled_features(void);  // Get currently enabled features
void xsimd_enable_features(xsimd_uint32 nFeatures);   // Enable specific features
void xsimd_disable_features(xsimd_uint32 nFeatures);  // Disable specific features
void xsimd_enable_all_features(void);           // Enable all detected features
void xsimd_disable_all_features(void);          // Disable all features (use scalar fallback)
```

**Feature Flags:**
- `XSIMD_FEATURE_NONE` - No features
- `XSIMD_FEATURE_SSE2` - SSE2 support
- `XSIMD_FEATURE_AVX2` - AVX2 support

### Search Operations
```c
xsimd_int64 xsimd_find_byte(const void *pBuffer, xsimd_int64 nSize, xsimd_uint8 cValue);
// Find first occurrence of a byte

xsimd_int64 xsimd_find_pattern_bmh(const void *pBuffer, xsimd_int64 nBufferSize, 
                                   const void *pPattern, xsimd_int64 nPatternSize);
// Find pattern using Boyer-Moore-Horspool algorithm

xsimd_int64 xsimd_find_ansi(const void *pBuffer, xsimd_int64 nBufferSize, xsimd_int64 nMinLength);
// Find ANSI printable sequence (min length)

xsimd_int64 xsimd_find_not_ansi(const void *pBuffer, xsimd_int64 nBufferSize, xsimd_int64 nMinLength);
// Find non-ANSI sequence (min length)
```

### Validation Operations
```c
xsimd_int32 xsimd_is_not_null(const void *pBuffer, xsimd_int64 nSize);
// Check if buffer contains any non-zero bytes (returns 1 if has non-zero, 0 if all zeros)

xsimd_int32 xsimd_is_ansi(const void *pBuffer, xsimd_int64 nSize);
// Check if buffer contains only ANSI printable characters (returns 1 if all ANSI, 0 otherwise)

xsimd_int32 xsimd_is_not_ansi(const void *pBuffer, xsimd_int64 nSize);
// Check if buffer contains non-ANSI characters (returns 1 if has non-ANSI, 0 if all ANSI)
```

### Memory Operations
```c
xsimd_int32 xsimd_compare_memory(const void *pBuffer1, const void *pBuffer2, xsimd_int64 nSize);
// Compare two memory buffers (returns 1 if equal, 0 if different)
```

## Usage Example

```c
#include "xsimd/src/xsimd.h"

int main(void) {
    // Initialize library (detects CPU features)
    xsimd_init();
    
    // Check what features are available
    xsimd_uint32 nFeatures = xsimd_get_features();
    if (nFeatures & XSIMD_FEATURE_AVX2) {
        printf("AVX2 available\n");
    }
    
    // Use library functions
    const char *pData = "Hello World";
    xsimd_int64 nPos = xsimd_find_byte(pData, 11, 'W');
    printf("Found 'W' at position: %lld\n", nPos);
    
    // For testing: disable AVX2 to test SSE2 path
    xsimd_disable_features(XSIMD_FEATURE_AVX2);
    
    // Re-enable all features
    xsimd_enable_all_features();
    
    // Cleanup
    xsimd_cleanup();
    return 0;
}
```

## Performance

The library provides significant speedups for large buffers:
- **Single byte search**: Up to 16x faster (SSE2) or 32x faster (AVX2) vs. scalar
- **Pattern search**: Optimized Boyer-Moore-Horspool with SIMD acceleration
- **Memory comparison**: Up to 32x faster on aligned data

For buffers smaller than SIMD register width (16 bytes for SSE2, 32 bytes for AVX2), the library automatically falls back to scalar code.

## Architecture Support

- **x86/x64**: Full SIMD support (AVX2, SSE2)
- **ARM**: Currently uses scalar fallback
- **Other architectures**: Scalar fallback

All functions work correctly on all platforms - SIMD is an optimization, not a requirement.

## License

MIT License - See src/LICENSE for details
