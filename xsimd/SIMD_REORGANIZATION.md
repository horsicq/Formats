# XSIMD Code Organization

## Overview
The XSIMD library code has been reorganized to separate SIMD instruction set implementations into dedicated files.

## Files Created

### Header Files
1. **xsimd_sse.h** - SSE2 (128-bit SIMD) function declarations
2. **xsimd_avx.h** - AVX (256-bit SIMD) function declarations  
3. **xsimd_avx2.h** - AVX2 (256-bit enhanced SIMD) function declarations

### Implementation Files
1. **xsimd_sse.c** - Placeholder for SSE2 implementations
2. **xsimd_avx.c** - Placeholder for AVX implementations
3. **xsimd_avx2.c** - Placeholder for AVX2 implementations

## Structure

Each SIMD variant file contains declarations/placeholders for:

### Core Search Functions
- `_xsimd_find_byte_XXX` - Find single byte in buffer
- `_xsimd_find_pattern_bmh_XXX` - Boyer-Moore-Horspool pattern search
- `_xsimd_find_pattern_bmh_1byte_XXX` - Optimized 1-byte pattern
- `_xsimd_find_pattern_bmh_2byte_XXX` - Optimized 2-byte pattern
- `_xsimd_find_pattern_bmh_3byte_XXX` - Optimized 3-byte pattern

### Character Classification
- `_xsimd_find_ansi_XXX` - Find ANSI printable sequences (0x20-0x7E)
- `_xsimd_find_not_ansi_XXX` - Find non-ANSI sequences
- `_xsimd_find_notnull_XXX` - Find sequences with no null bytes
- `_xsimd_find_not_ansi_and_null_XXX` - Find non-printable excluding null
- `_xsimd_find_ansi_number_XXX` - Find digit sequences (0x30-0x39)

### Validation Functions
- `_xsimd_is_not_null_XXX` - Check buffer has no zeros
- `_xsimd_is_ansi_XXX` - Check buffer is all ANSI printable
- `_xsimd_is_not_ansi_XXX` - Check buffer has no ANSI chars
- `_xsimd_is_not_ansi_and_null_XXX` - Check buffer validation
- `_xsimd_is_ansi_number_XXX` - Check buffer is all digits

### Utility Functions
- `_xsimd_compare_memory_XXX` - Fast memory comparison
- `_xsimd_find_first_non_ansi_XXX` - Find first non-ANSI byte
- `_xsimd_find_null_byte_XXX` - Find first null byte
- `_xsimd_count_unicode_prefix_XXX` - Count valid Unicode chars (UTF-16)
- `_xsimd_count_char_XXX` - Count occurrences of byte value
- `_xsimd_create_ansi_mask_XXX` - Create ANSI character bitmask

Where XXX is: SSE2, AVX, or AVX2

## Next Steps

The placeholder files currently reference line numbers where implementations exist in xsimd.c.
To complete the migration:

1. Extract each function implementation from xsimd.c based on the line numbers
2. Move implementations to the corresponding _sse.c, _avx.c, or _avx2.c file
3. Update xsimd.c to remove the migrated functions (keep only dispatcher/public API)
4. Ensure proper compilation with all three implementation files

## Integration

The main xsimd.c file now includes:
```c
#include "xsimd_sse.h"
#include "xsimd_avx.h"
#include "xsimd_avx2.h"
```

This allows xsimd.c to dispatch to the appropriate SIMD implementation based on CPU capabilities detected at runtime.

## Benefits

1. **Better Organization** - Each SIMD instruction set in its own file
2. **Easier Maintenance** - Changes to one SIMD variant don't affect others
3. **Clearer Code** - Separation of concerns makes code easier to understand
4. **Selective Compilation** - Could enable/disable specific SIMD variants
5. **Testing** - Each SIMD variant can be tested independently
