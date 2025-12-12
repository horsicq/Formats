/* Copyright (c) 2025 hors<horsicq@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef XSIMD_H
#define XSIMD_H

#ifdef __cplusplus
extern "C" {
#endif

/* Library version */
#define XSIMD_VERSION_MAJOR 0
#define XSIMD_VERSION_MINOR 1
#define XSIMD_VERSION_PATCH 0

/* SIMD feature detection result codes */
#define XSIMD_FEATURE_NONE  0x00
#define XSIMD_FEATURE_SSE2  0x01
#define XSIMD_FEATURE_AVX   0x02
#define XSIMD_FEATURE_AVX2  0x04

/* Search result type */
typedef long long xsimd_int64;
typedef unsigned long long xsimd_uint64;
typedef int xsimd_int32;
typedef unsigned int xsimd_uint32;
typedef unsigned short xsimd_uint16;
typedef unsigned char xsimd_uint8;

/**
 * Initialize the xsimd library
 * Detects available SIMD features on the current CPU
 * @return 0 on success, non-zero on error
 */
int xsimd_init(void);

/**
 * Get library version string
 * @return Version string in format "major.minor.patch"
 */
const char* xsimd_version(void);

/**
 * Get detected SIMD features
 * @return Bitmask of XSIMD_FEATURE_* flags
 */
xsimd_uint32 xsimd_get_features(void);

/**
 * Get enabled SIMD features (may be less than detected if disabled)
 * @return Bitmask of XSIMD_FEATURE_* flags currently enabled
 */
xsimd_uint32 xsimd_get_enabled_features(void);

/**
 * Enable specific SIMD features
 * @param nFeatures Bitmask of XSIMD_FEATURE_* flags to enable
 * @return 0 on success, non-zero if trying to enable unavailable features
 */
int xsimd_enable_features(xsimd_uint32 nFeatures);

/**
 * Disable specific SIMD features
 * @param nFeatures Bitmask of XSIMD_FEATURE_* flags to disable
 */
void xsimd_disable_features(xsimd_uint32 nFeatures);

/**
 * Enable all detected SIMD features
 */
void xsimd_enable_all_features(void);

/**
 * Disable all SIMD features (use scalar fallbacks only)
 */
void xsimd_disable_all_features(void);

/**
 * Find first occurrence of a single byte in buffer (optimized with SIMD)
 * @param pBuffer Buffer to search in
 * @param nSize Size of buffer
 * @param nByte Byte value to search for
 * @param nOffset Base offset (added to result)
 * @return Offset of first occurrence, or -1 if not found
 */
xsimd_int64 xsimd_find_byte(const void* pBuffer, xsimd_int64 nSize, xsimd_uint8 nByte, xsimd_int64 nOffset);

/**
 * Find pattern using Boyer-Moore-Horspool with SIMD optimization
 * @param pBuffer Haystack buffer
 * @param nBufferSize Haystack size
 * @param pPattern Needle pattern
 * @param nPatternSize Needle size
 * @param nOffset Base offset (added to result)
 * @return Offset of first occurrence, or -1 if not found
 */
xsimd_int64 xsimd_find_pattern_bmh(const void* pBuffer, xsimd_int64 nBufferSize, 
                                   const void* pPattern, xsimd_int64 nPatternSize, 
                                   xsimd_int64 nOffset);

/**
 * Find first occurrence of ANSI printable sequence (0x20-0x7E)
 * @param pBuffer Buffer to search in
 * @param nBufferSize Buffer size
 * @param nMinLength Minimum sequence length
 * @param nOffset Base offset (added to result)
 * @return Offset of first sequence, or -1 if not found
 */
xsimd_int64 xsimd_find_ansi(const void* pBuffer, xsimd_int64 nBufferSize, 
                            xsimd_int64 nMinLength, xsimd_int64 nOffset);

/**
 * Find first occurrence of non-null sequence (no 0x00 bytes)
 * @param pBuffer Buffer to search in
 * @param nBufferSize Buffer size
 * @param nMinLength Minimum sequence length (consecutive non-null bytes)
 * @param nOffset Base offset (added to result)
 * @return Offset of first sequence, or -1 if not found
 */
xsimd_int64 xsimd_find_notnull(const void* pBuffer, xsimd_int64 nBufferSize, 
                               xsimd_int64 nMinLength, xsimd_int64 nOffset);

/**
 * Find first occurrence of non-ANSI sequence (NOT in 0x20-0x7E)
 * @param pBuffer Buffer to search in
 * @param nBufferSize Buffer size
 * @param nMinLength Minimum sequence length
 * @param nOffset Base offset (added to result)
 * @return Offset of first sequence, or -1 if not found
 */
xsimd_int64 xsimd_find_not_ansi(const void* pBuffer, xsimd_int64 nBufferSize, 
                                xsimd_int64 nMinLength, xsimd_int64 nOffset);

/**
 * Check if memory contains no zero bytes
 * @param pBuffer Buffer to check
 * @param nSize Size of buffer
 * @return 1 if no zeros found, 0 if zero byte exists
 */
int xsimd_is_not_null(const void* pBuffer, xsimd_int64 nSize);

/**
 * Check if entire memory region is ANSI printable (0x20-0x7F)
 * @param pBuffer Buffer to check
 * @param nSize Size of buffer
 * @return 1 if all ANSI, 0 otherwise
 */
int xsimd_is_ansi(const void* pBuffer, xsimd_int64 nSize);

/**
 * Check if entire memory region is NOT ANSI printable
 * @param pBuffer Buffer to check
 * @param nSize Size of buffer
 * @return 1 if no ANSI chars, 0 otherwise
 */
int xsimd_is_not_ansi(const void* pBuffer, xsimd_int64 nSize);

/**
 * Find first occurrence of NOT-ANSI-AND-NULL sequence (non-printable excluding 0x00)
 * @param pBuffer Buffer to search in
 * @param nBufferSize Buffer size
 * @param nMinLength Minimum sequence length
 * @param nOffset Base offset (added to result)
 * @return Offset of first sequence, or -1 if not found
 */
xsimd_int64 xsimd_find_not_ansi_and_null(const void* pBuffer, xsimd_int64 nBufferSize, 
                                         xsimd_int64 nMinLength, xsimd_int64 nOffset);

/**
 * Check if entire memory region is NOT-ANSI-AND-NULL (non-printable excluding null)
 * @param pBuffer Buffer to check
 * @param nSize Size of buffer
 * @return 1 if all NOT-ANSI-AND-NULL, 0 otherwise
 */
int xsimd_is_not_ansi_and_null(const void* pBuffer, xsimd_int64 nSize);

/**
 * Find first occurrence of digit sequence (0x30-0x39)
 * @param pBuffer Buffer to search in
 * @param nBufferSize Buffer size
 * @param nMinLength Minimum sequence length
 * @param nOffset Base offset (added to result)
 * @return Offset of first sequence, or -1 if not found
 */
xsimd_int64 xsimd_find_ansi_number(const void* pBuffer, xsimd_int64 nBufferSize, 
                                   xsimd_int64 nMinLength, xsimd_int64 nOffset);

/**
 * Check if entire memory region is all digits (0x30-0x39)
 * @param pBuffer Buffer to check
 * @param nSize Size of buffer
 * @return 1 if all digits, 0 otherwise
 */
int xsimd_is_ansi_number(const void* pBuffer, xsimd_int64 nSize);

/**
 * Find ANSI string case-insensitive (optimized with SIMD)
 * @param pBuffer Haystack buffer
 * @param nBufferSize Haystack size
 * @param pString Needle string
 * @param nStringSize Needle size
 * @param nOffset Base offset (added to result)
 * @return Offset of first occurrence, or -1 if not found
 */
xsimd_int64 xsimd_find_ansi_string_i(const void* pBuffer, xsimd_int64 nBufferSize,
                                     const void* pString, xsimd_int64 nStringSize,
                                     xsimd_int64 nOffset);

/**
 * Compare two memory regions for equality
 * @param pBuffer1 First buffer
 * @param pBuffer2 Second buffer
 * @param nSize Size to compare
 * @return 1 if equal, 0 if different
 */
int xsimd_compare_memory(const void* pBuffer1, const void* pBuffer2, xsimd_int64 nSize);

/**
 * Cleanup library resources
 */
void xsimd_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif /* XSIMD_H */
