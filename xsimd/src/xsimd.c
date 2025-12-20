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
#include "xsimd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#define XSIMD_X86
#include <immintrin.h>
#endif

static int g_bInitialized = 0;
static xsimd_uint32 g_nFeatures = XSIMD_FEATURE_NONE;
static xsimd_uint32 g_nEnabledFeatures = XSIMD_FEATURE_NONE;

#ifdef XSIMD_X86
static void xsimd_detect_features(void)
{
    int nInfo[4];
    
#ifdef _MSC_VER
    /* CPUID function 1: Basic processor info */
    __cpuid(nInfo, 1);
    
    /* EDX register (bits in nInfo[3]) */
    if (nInfo[3] & (1 << 26)) g_nFeatures |= XSIMD_FEATURE_SSE2;   /* SSE2 */
    
    /* Check AVX support (requires both CPUID and OS support) */
    int bOSXSAVE = nInfo[2] & (1 << 27);  /* OS supports XSAVE (bit 27) */
    int bAVXCPUID = nInfo[2] & (1 << 28); /* CPU supports AVX (bit 28) */
    
    if (bOSXSAVE && bAVXCPUID) {
        /* Check if OS has enabled AVX state saving via XCR0 */
        unsigned long long nXCR0 = _xgetbv(0);
        
        if ((nXCR0 & 0x6) == 0x6) {  /* XMM (bit 1) and YMM (bit 2) state enabled */
            g_nFeatures |= XSIMD_FEATURE_AVX;
            
            /* Check AVX2 independently (CPUID function 7, subleaf 0, EBX bit 5) */
            /* Some virtualized environments may report AVX2 without AVX flag */
            __cpuidex(nInfo, 7, 0);
            
            if (nInfo[1] & (1 << 5)) {
                g_nFeatures |= XSIMD_FEATURE_AVX2;
            }
        } else {
            /* OS doesn't support AVX state, but check AVX2 anyway for completeness */
            /* In some rare cases (VM snapshots, CPU modes), AVX2 might be available */
            __cpuidex(nInfo, 7, 0);
            
            if (nInfo[1] & (1 << 5)) {
                /* AVX2 detected but OS doesn't support YMM state - still enable for SSE fallback paths */
                g_nFeatures |= XSIMD_FEATURE_AVX2;
            }
        }
    }
#else
    /* GCC/Clang builtin CPU detection */
    __builtin_cpu_init();
    
    if (__builtin_cpu_supports("sse2"))   g_nFeatures |= XSIMD_FEATURE_SSE2;
    if (__builtin_cpu_supports("avx"))    g_nFeatures |= XSIMD_FEATURE_AVX;
    if (__builtin_cpu_supports("avx2"))   g_nFeatures |= XSIMD_FEATURE_AVX2;
#endif
}
#endif

int xsimd_init(void)
{
    if (g_bInitialized) {
        return 0;
    }
    
#ifdef XSIMD_X86
    xsimd_detect_features();
#endif
    
    /* Enable all detected features by default */
    g_nEnabledFeatures = g_nFeatures;
    
    g_bInitialized = 1;
    return 0;
}

const char* xsimd_version(void)
{
    static char sVersion[32];
#ifdef _MSC_VER
    _snprintf(sVersion, sizeof(sVersion), "%d.%d.%d",
              XSIMD_VERSION_MAJOR,
              XSIMD_VERSION_MINOR,
              XSIMD_VERSION_PATCH);
#else
    snprintf(sVersion, sizeof(sVersion), "%d.%d.%d",
             XSIMD_VERSION_MAJOR,
             XSIMD_VERSION_MINOR,
             XSIMD_VERSION_PATCH);
#endif
    return sVersion;
}

xsimd_uint32 xsimd_get_features(void)
{
    return g_nFeatures;
}

xsimd_uint32 xsimd_get_enabled_features(void)
{
    return g_nEnabledFeatures;
}

int xsimd_enable_features(xsimd_uint32 nFeatures)
{
    /* Check if trying to enable features that aren't detected */
    if ((nFeatures & ~g_nFeatures) != 0) {
        return -1;  /* Error: trying to enable unavailable features */
    }
    
    g_nEnabledFeatures |= nFeatures;
    return 0;
}

void xsimd_disable_features(xsimd_uint32 nFeatures)
{
    g_nEnabledFeatures &= ~nFeatures;
}

void xsimd_enable_all_features(void)
{
    g_nEnabledFeatures = g_nFeatures;
}

void xsimd_disable_all_features(void)
{
    g_nEnabledFeatures = XSIMD_FEATURE_NONE;
}

int xsimd_is_sse2_enabled(void)
{
    return (g_nEnabledFeatures & XSIMD_FEATURE_SSE2) != 0;
}

int xsimd_is_avx_enabled(void)
{
    return (g_nEnabledFeatures & XSIMD_FEATURE_AVX) != 0;
}

int xsimd_is_avx2_enabled(void)
{
    return (g_nEnabledFeatures & XSIMD_FEATURE_AVX2) != 0;
}

int xsimd_is_sse2_present(void)
{
    return (g_nFeatures & XSIMD_FEATURE_SSE2) != 0;
}

int xsimd_is_avx_present(void)
{
    return (g_nFeatures & XSIMD_FEATURE_AVX) != 0;
}

int xsimd_is_avx2_present(void)
{
    return (g_nFeatures & XSIMD_FEATURE_AVX2) != 0;
}

void xsimd_set_sse2(int bState)
{
    if (bState) {
        xsimd_enable_features(XSIMD_FEATURE_SSE2);
    } else {
        xsimd_disable_features(XSIMD_FEATURE_SSE2);
    }
}

void xsimd_set_avx(int bState)
{
    if (bState) {
        xsimd_enable_features(XSIMD_FEATURE_AVX);
    } else {
        xsimd_disable_features(XSIMD_FEATURE_AVX);
    }
}

void xsimd_set_avx2(int bState)
{
    if (bState) {
        xsimd_enable_features(XSIMD_FEATURE_AVX2);
    } else {
        xsimd_disable_features(XSIMD_FEATURE_AVX2);
    }
}

xsimd_int64 xsimd_find_byte(const void* pBuffer, xsimd_int64 nSize, xsimd_uint8 nByte, xsimd_int64 nOffset)
{
    const unsigned char* pData = (const unsigned char*)pBuffer;
    xsimd_int64 i = 0;
    
#ifdef XSIMD_X86
    /* AVX2: Process 32 bytes at a time */
    if ((g_nEnabledFeatures & XSIMD_FEATURE_AVX2) && nSize >= 32) {
        __m256i vNeedle = _mm256_set1_epi8((char)nByte);
        
        /* Process 64 bytes per iteration for better throughput */
        while (i + 64 <= nSize) {
            __m256i vData0 = _mm256_loadu_si256((const __m256i*)(pData + i));
            __m256i vData1 = _mm256_loadu_si256((const __m256i*)(pData + i + 32));
            
            __m256i vCmp0 = _mm256_cmpeq_epi8(vData0, vNeedle);
            __m256i vCmp1 = _mm256_cmpeq_epi8(vData1, vNeedle);
            
            xsimd_int32 nMask0 = _mm256_movemask_epi8(vCmp0);
            xsimd_int32 nMask1 = _mm256_movemask_epi8(vCmp1);
            
            if (nMask0 != 0) {
#ifdef _MSC_VER
                unsigned long nBitPos;
                _BitScanForward(&nBitPos, (unsigned long)nMask0);
                return nOffset + i + nBitPos;
#else
                return nOffset + i + __builtin_ctz((unsigned int)nMask0);
#endif
            }
            
            if (nMask1 != 0) {
#ifdef _MSC_VER
                unsigned long nBitPos;
                _BitScanForward(&nBitPos, (unsigned long)nMask1);
                return nOffset + i + 32 + nBitPos;
#else
                return nOffset + i + 32 + __builtin_ctz((unsigned int)nMask1);
#endif
            }
            
            i += 64;
        }
        
        /* Process remaining 32-byte chunks */
        while (i + 32 <= nSize) {
            __m256i vData = _mm256_loadu_si256((const __m256i*)(pData + i));
            __m256i vCmp = _mm256_cmpeq_epi8(vData, vNeedle);
            xsimd_int32 nMask = _mm256_movemask_epi8(vCmp);
            
            if (nMask != 0) {
#ifdef _MSC_VER
                unsigned long nBitPos;
                _BitScanForward(&nBitPos, (unsigned long)nMask);
                return nOffset + i + nBitPos;
#else
                return nOffset + i + __builtin_ctz((unsigned int)nMask);
#endif
            }
            i += 32;
        }
    }
    /* AVX: Process 32 bytes at a time */
    else if ((g_nEnabledFeatures & XSIMD_FEATURE_AVX) && nSize >= 32) {
        __m256i vNeedle = _mm256_set1_epi8((char)nByte);
        
        while (i + 32 <= nSize) {
            __m256i vData = _mm256_loadu_si256((const __m256i*)(pData + i));
            __m256i vCmp = _mm256_cmpeq_epi8(vData, vNeedle);
            xsimd_int32 nMask = _mm256_movemask_epi8(vCmp);
            
            if (nMask != 0) {
#ifdef _MSC_VER
                unsigned long nBitPos;
                _BitScanForward(&nBitPos, (unsigned long)nMask);
                return nOffset + i + nBitPos;
#else
                return nOffset + i + __builtin_ctz((unsigned int)nMask);
#endif
            }
            i += 32;
        }
    }
    /* SSE2: Process 16 bytes at a time */
    else if ((g_nEnabledFeatures & XSIMD_FEATURE_SSE2) && nSize >= 16) {
        __m128i vNeedle = _mm_set1_epi8((char)nByte);
        
        /* Process 64 bytes per iteration for better throughput */
        while (i + 64 <= nSize) {
            __m128i vData0 = _mm_loadu_si128((const __m128i*)(pData + i));
            __m128i vData1 = _mm_loadu_si128((const __m128i*)(pData + i + 16));
            __m128i vData2 = _mm_loadu_si128((const __m128i*)(pData + i + 32));
            __m128i vData3 = _mm_loadu_si128((const __m128i*)(pData + i + 48));
            
            __m128i vCmp0 = _mm_cmpeq_epi8(vData0, vNeedle);
            __m128i vCmp1 = _mm_cmpeq_epi8(vData1, vNeedle);
            __m128i vCmp2 = _mm_cmpeq_epi8(vData2, vNeedle);
            __m128i vCmp3 = _mm_cmpeq_epi8(vData3, vNeedle);
            
            int nMask0 = _mm_movemask_epi8(vCmp0);
            int nMask1 = _mm_movemask_epi8(vCmp1);
            int nMask2 = _mm_movemask_epi8(vCmp2);
            int nMask3 = _mm_movemask_epi8(vCmp3);
            
            if (nMask0 != 0) {
#ifdef _MSC_VER
                unsigned long nBitPos;
                _BitScanForward(&nBitPos, (unsigned long)nMask0);
                return nOffset + i + nBitPos;
#else
                return nOffset + i + __builtin_ctz((unsigned int)nMask0);
#endif
            }
            
            if (nMask1 != 0) {
#ifdef _MSC_VER
                unsigned long nBitPos;
                _BitScanForward(&nBitPos, (unsigned long)nMask1);
                return nOffset + i + 16 + nBitPos;
#else
                return nOffset + i + 16 + __builtin_ctz((unsigned int)nMask1);
#endif
            }
            
            if (nMask2 != 0) {
#ifdef _MSC_VER
                unsigned long nBitPos;
                _BitScanForward(&nBitPos, (unsigned long)nMask2);
                return nOffset + i + 32 + nBitPos;
#else
                return nOffset + i + 32 + __builtin_ctz((unsigned int)nMask2);
#endif
            }
            
            if (nMask3 != 0) {
#ifdef _MSC_VER
                unsigned long nBitPos;
                _BitScanForward(&nBitPos, (unsigned long)nMask3);
                return nOffset + i + 48 + nBitPos;
#else
                return nOffset + i + 48 + __builtin_ctz((unsigned int)nMask3);
#endif
            }
            
            i += 64;
        }
        
        /* Process remaining 16-byte chunks */
        while (i + 16 <= nSize) {
            __m128i vData = _mm_loadu_si128((const __m128i*)(pData + i));
            __m128i vCmp = _mm_cmpeq_epi8(vData, vNeedle);
            int nMask = _mm_movemask_epi8(vCmp);
            
            if (nMask != 0) {
#ifdef _MSC_VER
                unsigned long nBitPos;
                _BitScanForward(&nBitPos, (unsigned long)nMask);
                return nOffset + i + nBitPos;
#else
                return nOffset + i + __builtin_ctz((unsigned int)nMask);
#endif
            }
            i += 16;
        }
    }
#endif
    
    /* Fallback for remaining bytes */
    const void* pFound = memchr(pData + i, nByte, (size_t)(nSize - i));
    if (pFound) {
        return nOffset + ((const char*)pFound - (const char*)pBuffer);
    }
    
    return -1;
}

xsimd_int64 xsimd_find_pattern_bmh(const void* pBuffer, xsimd_int64 nBufferSize, 
                                   const void* pPattern, xsimd_int64 nPatternSize, 
                                   xsimd_int64 nOffset)
{
    if (nPatternSize == 0 || nBufferSize < nPatternSize) {
        return -1;
    }
    
    const char* pHay = (const char*)pBuffer;
    const char* pNeedle = (const char*)pPattern;
    const xsimd_int64 nLimit = nBufferSize - nPatternSize;
    const char nLastChar = pNeedle[nPatternSize - 1];
    xsimd_int64 i = 0;
    
#ifdef XSIMD_X86
    /* Special handling for 1-byte patterns using SIMD */
    if (nPatternSize == 1 && (g_nEnabledFeatures & XSIMD_FEATURE_AVX2)) {
        __m256i vPattern = _mm256_set1_epi8(pNeedle[0]);
        
        /* Process 32 bytes per iteration */
        while (i + 31 < nBufferSize) {
            __m256i vData = _mm256_loadu_si256((const __m256i*)(pHay + i));
            __m256i vCmp = _mm256_cmpeq_epi8(vData, vPattern);
            xsimd_uint32 nMask = (xsimd_uint32)_mm256_movemask_epi8(vCmp);
            
            if (nMask != 0) {
                /* Find first match using bit scan */
#ifdef _MSC_VER
                unsigned long bit;
                _BitScanForward(&bit, nMask);
#else
                unsigned bit = __builtin_ctz(nMask);
#endif
                return nOffset + i + (xsimd_int64)bit;
            }
            
            i += 32;
        }
        
        /* Scalar fallback for remaining bytes */
        while (i < nBufferSize) {
            if (pHay[i] == pNeedle[0]) {
                return nOffset + i;
            }
            i++;
        }
        
        return -1;
    }
    
    /* Special handling for 2-byte patterns using SIMD */
    if (nPatternSize == 2 && (g_nEnabledFeatures & XSIMD_FEATURE_AVX2)) {
        xsimd_uint16 pattern16 = *(const xsimd_uint16*)pNeedle;
        __m256i vPattern = _mm256_set1_epi16((short)pattern16);
        
        /* Process 32 bytes (16 two-byte values) per iteration */
        while (i + 31 < nBufferSize) {
            __m256i vData = _mm256_loadu_si256((const __m256i*)(pHay + i));
            __m256i vCmp = _mm256_cmpeq_epi16(vData, vPattern);
            xsimd_uint32 nMask = (xsimd_uint32)_mm256_movemask_epi8(vCmp);
            
            if (nMask != 0) {
                /* Check each potential match (aligned to 2-byte boundaries) */
                xsimd_uint32 nTempMask = nMask;
                while (nTempMask != 0) {
#ifdef _MSC_VER
                    unsigned long bit;
                    _BitScanForward(&bit, nTempMask);
#else
                    unsigned bit = __builtin_ctz(nTempMask);
#endif
                    /* 16-bit comparison marks every other byte */
                    if ((bit & 1) == 0) {
                        xsimd_int64 pos = i + (xsimd_int64)bit;
                        if (pos + 1 < nBufferSize) {
                            if (*(const xsimd_uint16*)(pHay + pos) == pattern16) {
                                return nOffset + pos;
                            }
                        }
                    }
                    
                    nTempMask &= nTempMask - 1;
                }
            }
            
            i += 32;
        }
        
        /* Scalar fallback for remaining bytes */
        while (i + 1 < nBufferSize) {
            if (*(const xsimd_uint16*)(pHay + i) == pattern16) {
                return nOffset + i;
            }
            i++;
        }
        
        return -1;
    }
    
    /* Special handling for 3-byte patterns using hybrid SIMD + scalar */
    if (nPatternSize == 3 && (g_nEnabledFeatures & XSIMD_FEATURE_AVX2)) {
        __m256i vFirst = _mm256_set1_epi8(pNeedle[0]);
        const char nSecond = pNeedle[1];
        const char nThird = pNeedle[2];
        
        /* Process 32 bytes per iteration - filter by first byte */
        while (i + 31 + 2 < nBufferSize) {
            __m256i vData = _mm256_loadu_si256((const __m256i*)(pHay + i));
            __m256i vCmp = _mm256_cmpeq_epi8(vData, vFirst);
            xsimd_uint32 nMask = (xsimd_uint32)_mm256_movemask_epi8(vCmp);
            
            if (nMask != 0) {
                /* Check each candidate */
                xsimd_uint32 nTempMask = nMask;
                while (nTempMask != 0) {
#ifdef _MSC_VER
                    unsigned long bit;
                    _BitScanForward(&bit, nTempMask);
#else
                    unsigned bit = __builtin_ctz(nTempMask);
#endif
                    xsimd_int64 pos = i + (xsimd_int64)bit;
                    
                    /* Verify remaining 2 bytes */
                    if (pos + 2 < nBufferSize) {
                        if (pHay[pos + 1] == nSecond && pHay[pos + 2] == nThird) {
                            return nOffset + pos;
                        }
                    }
                    
                    nTempMask &= nTempMask - 1;
                }
            }
            
            i += 32;
        }
        
        /* Scalar fallback for remaining bytes */
        while (i + 2 < nBufferSize) {
            if (pHay[i] == pNeedle[0] && pHay[i + 1] == nSecond && pHay[i + 2] == nThird) {
                return nOffset + i;
            }
            i++;
        }
        
        return -1;
    }
    
    /* AVX2: Process 64 bytes at a time (2 × 32-byte vectors) */
    if ((g_nEnabledFeatures & XSIMD_FEATURE_AVX2) && nPatternSize >= 4) {
        __m256i vLast = _mm256_set1_epi8((char)nLastChar);
        
        /* Process 64 bytes per iteration for better throughput */
        while (i + nPatternSize + 63 <= nBufferSize) {
            const char* pLastPos0 = pHay + i + nPatternSize - 1;
            const char* pLastPos1 = pLastPos0 + 32;
            
            __m256i vData0 = _mm256_loadu_si256((const __m256i*)pLastPos0);
            __m256i vData1 = _mm256_loadu_si256((const __m256i*)pLastPos1);
            
            __m256i vCmp0 = _mm256_cmpeq_epi8(vData0, vLast);
            __m256i vCmp1 = _mm256_cmpeq_epi8(vData1, vLast);
            
            xsimd_uint32 nMask0 = (xsimd_uint32)_mm256_movemask_epi8(vCmp0);
            xsimd_uint32 nMask1 = (xsimd_uint32)_mm256_movemask_epi8(vCmp1);
            
            /* Fast path: skip if no matches in either block */
            if ((nMask0 | nMask1) == 0) {
                i += 64;
                continue;
            }
            
            /* Check first 32-byte block */
            if (nMask0 != 0) {
                xsimd_uint32 nTempMask = nMask0;
                while (nTempMask != 0) {
#ifdef _MSC_VER
                    unsigned long bit;
                    _BitScanForward(&bit, nTempMask);
#else
                    unsigned bit = __builtin_ctz(nTempMask);
#endif
                    xsimd_int64 nCheckPos = i + (xsimd_int64)bit;
                    
                    if (nCheckPos <= nLimit) {
                        if (memcmp(pHay + nCheckPos, pNeedle, (size_t)nPatternSize) == 0) {
                            return nOffset + nCheckPos;
                        }
                    }
                    
                    nTempMask &= nTempMask - 1;
                }
            }
            
            /* Check second 32-byte block */
            if (nMask1 != 0) {
                xsimd_uint32 nTempMask = nMask1;
                while (nTempMask != 0) {
#ifdef _MSC_VER
                    unsigned long bit;
                    _BitScanForward(&bit, nTempMask);
#else
                    unsigned bit = __builtin_ctz(nTempMask);
#endif
                    xsimd_int64 nCheckPos = i + 32 + (xsimd_int64)bit;
                    
                    if (nCheckPos <= nLimit) {
                        if (memcmp(pHay + nCheckPos, pNeedle, (size_t)nPatternSize) == 0) {
                            return nOffset + nCheckPos;
                        }
                    }
                    
                    nTempMask &= nTempMask - 1;
                }
            }
            
            i += 64;
        }
        
        /* Process remaining 32-byte chunks */
        while (i + nPatternSize + 31 <= nBufferSize) {
            const char* pLastPos = pHay + i + nPatternSize - 1;
            __m256i vData = _mm256_loadu_si256((const __m256i*)pLastPos);
            __m256i vCmp = _mm256_cmpeq_epi8(vData, vLast);
            xsimd_uint32 nMask = (xsimd_uint32)_mm256_movemask_epi8(vCmp);
            
            if (nMask != 0) {
                while (nMask != 0) {
#ifdef _MSC_VER
                    unsigned long bit;
                    _BitScanForward(&bit, nMask);
#else
                    unsigned bit = __builtin_ctz(nMask);
#endif
                    xsimd_int64 nCheckPos = i + (xsimd_int64)bit;
                    
                    if (nCheckPos <= nLimit) {
                        if (memcmp(pHay + nCheckPos, pNeedle, (size_t)nPatternSize) == 0) {
                            return nOffset + nCheckPos;
                        }
                    }
                    
                    nMask &= nMask - 1;
                }
            }
            
            i += 32;
        }
    }
    /* AVX: Process 32 bytes at a time */
    else if ((g_nEnabledFeatures & XSIMD_FEATURE_AVX) && nPatternSize >= 4) {
        __m256i vLast = _mm256_set1_epi8((char)nLastChar);
        
        while (i + nPatternSize + 31 <= nBufferSize) {
            const char* pLastPos = pHay + i + nPatternSize - 1;
            __m256i vData = _mm256_loadu_si256((const __m256i*)pLastPos);
            __m256i vCmp = _mm256_cmpeq_epi8(vData, vLast);
            xsimd_uint32 nMask = (xsimd_uint32)_mm256_movemask_epi8(vCmp);
            
            if (nMask != 0) {
                while (nMask != 0) {
#ifdef _MSC_VER
                    unsigned long bit;
                    _BitScanForward(&bit, nMask);
#else
                    unsigned bit = __builtin_ctz(nMask);
#endif
                    xsimd_int64 nCheckPos = i + (xsimd_int64)bit;
                    
                    if (nCheckPos <= nLimit) {
                        if (memcmp(pHay + nCheckPos, pNeedle, (size_t)nPatternSize) == 0) {
                            return nOffset + nCheckPos;
                        }
                    }
                    
                    nMask &= nMask - 1;
                }
            }
            
            i += 32;
        }
    }
    /* AVX: Special handling for 1-byte patterns */
    else if (nPatternSize == 1 && (g_nEnabledFeatures & XSIMD_FEATURE_AVX)) {
        __m256i vPattern = _mm256_set1_epi8(pNeedle[0]);
        
        /* Process 32 bytes per iteration */
        while (i + 31 < nBufferSize) {
            __m256i vData = _mm256_loadu_si256((const __m256i*)(pHay + i));
            __m256i vCmp = _mm256_cmpeq_epi8(vData, vPattern);
            xsimd_uint32 nMask = (xsimd_uint32)_mm256_movemask_epi8(vCmp);
            
            if (nMask != 0) {
                /* Find first match using bit scan */
#ifdef _MSC_VER
                unsigned long bit;
                _BitScanForward(&bit, nMask);
#else
                unsigned bit = __builtin_ctz(nMask);
#endif
                return nOffset + i + (xsimd_int64)bit;
            }
            
            i += 32;
        }
        
        /* Scalar fallback for remaining bytes */
        while (i < nBufferSize) {
            if (pHay[i] == pNeedle[0]) {
                return nOffset + i;
            }
            i++;
        }
        
        return -1;
    }
    /* SSE2: Special handling for 1-byte patterns */
    else if (nPatternSize == 1 && (g_nEnabledFeatures & XSIMD_FEATURE_SSE2)) {
        __m128i vPattern = _mm_set1_epi8(pNeedle[0]);
        
        /* Process 16 bytes per iteration */
        while (i + 15 < nBufferSize) {
            __m128i vData = _mm_loadu_si128((const __m128i*)(pHay + i));
            __m128i vCmp = _mm_cmpeq_epi8(vData, vPattern);
            int nMask = _mm_movemask_epi8(vCmp);
            
            if (nMask != 0) {
                /* Find first match using bit scan */
#ifdef _MSC_VER
                unsigned long bit;
                _BitScanForward(&bit, (unsigned long)nMask);
#else
                unsigned bit = __builtin_ctz((unsigned int)nMask);
#endif
                return nOffset + i + (xsimd_int64)bit;
            }
            
            i += 16;
        }
        
        /* Scalar fallback for remaining bytes */
        while (i < nBufferSize) {
            if (pHay[i] == pNeedle[0]) {
                return nOffset + i;
            }
            i++;
        }
        
        return -1;
    }
    /* AVX: Special handling for 2-byte patterns */
    else if (nPatternSize == 2 && (g_nEnabledFeatures & XSIMD_FEATURE_AVX)) {
        xsimd_uint16 pattern16 = *(const xsimd_uint16*)pNeedle;
        __m256i vPattern = _mm256_set1_epi16((short)pattern16);
        
        /* Process 32 bytes (16 two-byte values) per iteration */
        while (i + 31 < nBufferSize) {
            __m256i vData = _mm256_loadu_si256((const __m256i*)(pHay + i));
            __m256i vCmp = _mm256_cmpeq_epi16(vData, vPattern);
            xsimd_uint32 nMask = (xsimd_uint32)_mm256_movemask_epi8(vCmp);
            
            if (nMask != 0) {
                /* Check each potential match (aligned to 2-byte boundaries) */
                xsimd_uint32 nTempMask = nMask;
                while (nTempMask != 0) {
#ifdef _MSC_VER
                    unsigned long bit;
                    _BitScanForward(&bit, nTempMask);
#else
                    unsigned bit = __builtin_ctz(nTempMask);
#endif
                    /* 16-bit comparison marks every other byte */
                    if ((bit & 1) == 0) {
                        xsimd_int64 pos = i + (xsimd_int64)bit;
                        if (pos + 1 < nBufferSize) {
                            if (*(const xsimd_uint16*)(pHay + pos) == pattern16) {
                                return nOffset + pos;
                            }
                        }
                    }
                    
                    nTempMask &= nTempMask - 1;
                }
            }
            
            i += 32;
        }
        
        /* Scalar fallback for remaining bytes */
        while (i + 1 < nBufferSize) {
            if (*(const xsimd_uint16*)(pHay + i) == pattern16) {
                return nOffset + i;
            }
            i++;
        }
        
        return -1;
    }
    /* SSE2: Special handling for 2-byte patterns */
    else if (nPatternSize == 2 && (g_nEnabledFeatures & XSIMD_FEATURE_SSE2)) {
        xsimd_uint16 pattern16 = *(const xsimd_uint16*)pNeedle;
        __m128i vPattern = _mm_set1_epi16((short)pattern16);
        
        /* Process 16 bytes (8 two-byte values) per iteration */
        while (i + 15 < nBufferSize) {
            __m128i vData = _mm_loadu_si128((const __m128i*)(pHay + i));
            __m128i vCmp = _mm_cmpeq_epi16(vData, vPattern);
            int nMask = _mm_movemask_epi8(vCmp);
            
            if (nMask != 0) {
                /* Check each potential match (aligned to 2-byte boundaries) */
                int nTempMask = nMask;
                while (nTempMask != 0) {
#ifdef _MSC_VER
                    unsigned long bit;
                    _BitScanForward(&bit, (unsigned long)nTempMask);
#else
                    unsigned bit = __builtin_ctz((unsigned int)nTempMask);
#endif
                    /* 16-bit comparison marks every other byte */
                    if ((bit & 1) == 0) {
                        xsimd_int64 pos = i + (xsimd_int64)bit;
                        if (pos + 1 < nBufferSize) {
                            if (*(const xsimd_uint16*)(pHay + pos) == pattern16) {
                                return nOffset + pos;
                            }
                        }
                    }
                    
                    nTempMask &= nTempMask - 1;
                }
            }
            
            i += 16;
        }
        
        /* Scalar fallback for remaining bytes */
        while (i + 1 < nBufferSize) {
            if (*(const xsimd_uint16*)(pHay + i) == pattern16) {
                return nOffset + i;
            }
            i++;
        }
        
        return -1;
    }
    /* AVX: Special handling for 3-byte patterns */
    else if (nPatternSize == 3 && (g_nEnabledFeatures & XSIMD_FEATURE_AVX)) {
        __m256i vFirst = _mm256_set1_epi8(pNeedle[0]);
        const char nSecond = pNeedle[1];
        const char nThird = pNeedle[2];
        
        /* Process 32 bytes per iteration - filter by first byte */
        while (i + 31 + 2 < nBufferSize) {
            __m256i vData = _mm256_loadu_si256((const __m256i*)(pHay + i));
            __m256i vCmp = _mm256_cmpeq_epi8(vData, vFirst);
            xsimd_uint32 nMask = (xsimd_uint32)_mm256_movemask_epi8(vCmp);
            
            if (nMask != 0) {
                /* Check each candidate */
                xsimd_uint32 nTempMask = nMask;
                while (nTempMask != 0) {
#ifdef _MSC_VER
                    unsigned long bit;
                    _BitScanForward(&bit, nTempMask);
#else
                    unsigned bit = __builtin_ctz(nTempMask);
#endif
                    xsimd_int64 pos = i + (xsimd_int64)bit;
                    
                    /* Verify remaining 2 bytes */
                    if (pos + 2 < nBufferSize) {
                        if (pHay[pos + 1] == nSecond && pHay[pos + 2] == nThird) {
                            return nOffset + pos;
                        }
                    }
                    
                    nTempMask &= nTempMask - 1;
                }
            }
            
            i += 32;
        }
        
        /* Scalar fallback for remaining bytes */
        while (i + 2 < nBufferSize) {
            if (pHay[i] == pNeedle[0] && pHay[i + 1] == nSecond && pHay[i + 2] == nThird) {
                return nOffset + i;
            }
            i++;
        }
        
        return -1;
    }
    /* SSE2: Special handling for 3-byte patterns */
    else if (nPatternSize == 3 && (g_nEnabledFeatures & XSIMD_FEATURE_SSE2)) {
        __m128i vFirst = _mm_set1_epi8(pNeedle[0]);
        const char nSecond = pNeedle[1];
        const char nThird = pNeedle[2];
        
        /* Process 16 bytes per iteration - filter by first byte */
        while (i + 15 + 2 < nBufferSize) {
            __m128i vData = _mm_loadu_si128((const __m128i*)(pHay + i));
            __m128i vCmp = _mm_cmpeq_epi8(vData, vFirst);
            int nMask = _mm_movemask_epi8(vCmp);
            
            if (nMask != 0) {
                /* Check each candidate */
                int nTempMask = nMask;
                while (nTempMask != 0) {
#ifdef _MSC_VER
                    unsigned long bit;
                    _BitScanForward(&bit, (unsigned long)nTempMask);
#else
                    unsigned bit = __builtin_ctz((unsigned int)nTempMask);
#endif
                    xsimd_int64 pos = i + (xsimd_int64)bit;
                    
                    /* Verify remaining 2 bytes */
                    if (pos + 2 < nBufferSize) {
                        if (pHay[pos + 1] == nSecond && pHay[pos + 2] == nThird) {
                            return nOffset + pos;
                        }
                    }
                    
                    nTempMask &= nTempMask - 1;
                }
            }
            
            i += 16;
        }
        
        /* Scalar fallback for remaining bytes */
        while (i + 2 < nBufferSize) {
            if (pHay[i] == pNeedle[0] && pHay[i + 1] == nSecond && pHay[i + 2] == nThird) {
                return nOffset + i;
            }
            i++;
        }
        
        return -1;
    }
    /* SSE2: Process 64 bytes at a time (4 × 16-byte vectors) */
    else if ((g_nEnabledFeatures & XSIMD_FEATURE_SSE2) && nPatternSize >= 4) {
        __m128i vLast = _mm_set1_epi8((char)nLastChar);
        
        /* Process 64 bytes per iteration for better throughput */
        while (i + nPatternSize + 63 <= nBufferSize) {
            const char* pLastPos0 = pHay + i + nPatternSize - 1;
            const char* pLastPos1 = pLastPos0 + 16;
            const char* pLastPos2 = pLastPos0 + 32;
            const char* pLastPos3 = pLastPos0 + 48;
            
            __m128i vData0 = _mm_loadu_si128((const __m128i*)pLastPos0);
            __m128i vData1 = _mm_loadu_si128((const __m128i*)pLastPos1);
            __m128i vData2 = _mm_loadu_si128((const __m128i*)pLastPos2);
            __m128i vData3 = _mm_loadu_si128((const __m128i*)pLastPos3);
            
            __m128i vCmp0 = _mm_cmpeq_epi8(vData0, vLast);
            __m128i vCmp1 = _mm_cmpeq_epi8(vData1, vLast);
            __m128i vCmp2 = _mm_cmpeq_epi8(vData2, vLast);
            __m128i vCmp3 = _mm_cmpeq_epi8(vData3, vLast);
            
            int nMask0 = _mm_movemask_epi8(vCmp0);
            int nMask1 = _mm_movemask_epi8(vCmp1);
            int nMask2 = _mm_movemask_epi8(vCmp2);
            int nMask3 = _mm_movemask_epi8(vCmp3);
            
            /* Check all four 16-byte blocks */
            int nMasks[4] = {nMask0, nMask1, nMask2, nMask3};
            xsimd_int64 nOffsets[4] = {0, 16, 32, 48};
            
            for (int block = 0; block < 4; block++) {
                if (nMasks[block] != 0) {
                    int nTempMask = nMasks[block];
                    while (nTempMask != 0) {
#ifdef _MSC_VER
                        unsigned long bit;
                        _BitScanForward(&bit, (unsigned long)nTempMask);
#else
                        unsigned bit = __builtin_ctz((unsigned int)nTempMask);
#endif
                        xsimd_int64 nCheckPos = i + nOffsets[block] + (xsimd_int64)bit;
                        
                        if (nCheckPos <= nLimit) {
                            if (memcmp(pHay + nCheckPos, pNeedle, (size_t)nPatternSize) == 0) {
                                return nOffset + nCheckPos;
                            }
                        }
                        
                        nTempMask &= nTempMask - 1;
                    }
                }
            }
            
            i += 64;
        }
        
        /* Process remaining 16-byte chunks */
        while (i + nPatternSize + 15 <= nBufferSize) {
            const char* pLastPos = pHay + i + nPatternSize - 1;
            __m128i vData = _mm_loadu_si128((const __m128i*)pLastPos);
            __m128i vCmp = _mm_cmpeq_epi8(vData, vLast);
            int nMask = _mm_movemask_epi8(vCmp);
            
            if (nMask != 0) {
                while (nMask != 0) {
#ifdef _MSC_VER
                    unsigned long bit;
                    _BitScanForward(&bit, (unsigned long)nMask);
#else
                    unsigned bit = __builtin_ctz((unsigned int)nMask);
#endif
                    xsimd_int64 nCheckPos = i + (xsimd_int64)bit;
                    
                    if (nCheckPos <= nLimit) {
                        if (memcmp(pHay + nCheckPos, pNeedle, (size_t)nPatternSize) == 0) {
                            return nOffset + nCheckPos;
                        }
                    }
                    
                    nMask &= nMask - 1;
                }
            }
            
            i += 16;
        }
    }
#endif
    
    /* Fallback: simple search for remaining bytes */
    while (i <= nLimit) {
        if (pHay[i + nPatternSize - 1] == nLastChar) {
            if (memcmp(pHay + i, pNeedle, (size_t)nPatternSize) == 0) {
                return nOffset + i;
            }
        }
        i++;
    }
    
    return -1;
}

xsimd_int64 xsimd_find_ansi(const void* pBuffer, xsimd_int64 nBufferSize, 
                            xsimd_int64 nMinLength, xsimd_int64 nOffset)
{
    const unsigned char* pData = (const unsigned char*)pBuffer;
    const xsimd_int64 nLimit = nBufferSize - (nMinLength - 1);
    xsimd_int64 j = 0;
    
#ifdef XSIMD_X86
    /* AVX2: Process 32 bytes at a time */
    if ((g_nEnabledFeatures & XSIMD_FEATURE_AVX2) && nBufferSize >= 32) {
        const __m256i vLowerMinus1 = _mm256_set1_epi8(0x1F);
        const __m256i vUpperPlus1 = _mm256_set1_epi8(0x7F);
        
        /* Process 64 bytes per iteration for better throughput */
        while (j + 64 <= nBufferSize) {
            __m256i vData0 = _mm256_loadu_si256((const __m256i*)(pData + j));
            __m256i vData1 = _mm256_loadu_si256((const __m256i*)(pData + j + 32));
            
            __m256i vGe0 = _mm256_cmpgt_epi8(vData0, vLowerMinus1);
            __m256i vLe0 = _mm256_cmpgt_epi8(vUpperPlus1, vData0);
            __m256i vAnsi0 = _mm256_and_si256(vGe0, vLe0);
            
            __m256i vGe1 = _mm256_cmpgt_epi8(vData1, vLowerMinus1);
            __m256i vLe1 = _mm256_cmpgt_epi8(vUpperPlus1, vData1);
            __m256i vAnsi1 = _mm256_and_si256(vGe1, vLe1);
            
            xsimd_int32 nMask0 = _mm256_movemask_epi8(vAnsi0);
            xsimd_int32 nMask1 = _mm256_movemask_epi8(vAnsi1);
            
            if (nMask0 != 0) {
#ifdef _MSC_VER
                unsigned long nBitPos;
                _BitScanForward(&nBitPos, (unsigned long)nMask0);
                xsimd_int64 start = j + nBitPos;
#else
                xsimd_int64 start = j + __builtin_ctz((unsigned int)nMask0);
#endif
                xsimd_int64 runLen = 0;
                while ((start + runLen) < nBufferSize && pData[start + runLen] >= 0x20 && pData[start + runLen] <= 0x7E) {
                    runLen++;
                }
                
                if (runLen >= nMinLength) {
                    return nOffset + start;
                }
                
                j = start + runLen + 1;
                continue;
            }
            
            if (nMask1 != 0) {
#ifdef _MSC_VER
                unsigned long nBitPos;
                _BitScanForward(&nBitPos, (unsigned long)nMask1);
                xsimd_int64 start = j + 32 + nBitPos;
#else
                xsimd_int64 start = j + 32 + __builtin_ctz((unsigned int)nMask1);
#endif
                xsimd_int64 runLen = 0;
                while ((start + runLen) < nBufferSize && pData[start + runLen] >= 0x20 && pData[start + runLen] <= 0x7E) {
                    runLen++;
                }
                
                if (runLen >= nMinLength) {
                    return nOffset + start;
                }
                
                j = start + runLen + 1;
                continue;
            }
            
            j += 64;
        }
        
        /* Process remaining 32-byte chunks */
        while (j + 32 <= nBufferSize) {
            __m256i vData = _mm256_loadu_si256((const __m256i*)(pData + j));
            __m256i vGe = _mm256_cmpgt_epi8(vData, vLowerMinus1);
            __m256i vLe = _mm256_cmpgt_epi8(vUpperPlus1, vData);
            __m256i vAnsi = _mm256_and_si256(vGe, vLe);
            xsimd_int32 nMask = _mm256_movemask_epi8(vAnsi);
            
            if (nMask != 0) {
#ifdef _MSC_VER
                unsigned long nBitPos;
                _BitScanForward(&nBitPos, (unsigned long)nMask);
                xsimd_int64 start = j + nBitPos;
#else
                xsimd_int64 start = j + __builtin_ctz((unsigned int)nMask);
#endif
                xsimd_int64 runLen = 0;
                while ((start + runLen) < nBufferSize && pData[start + runLen] >= 0x20 && pData[start + runLen] <= 0x7E) {
                    runLen++;
                }
                
                if (runLen >= nMinLength) {
                    return nOffset + start;
                }
                
                j = start + runLen + 1;
            } else {
                j += 32;
            }
        }
    }
    /* AVX: Process 32 bytes at a time */
    else if ((g_nEnabledFeatures & XSIMD_FEATURE_AVX) && nBufferSize >= 32) {
        const __m256i vLowerMinus1 = _mm256_set1_epi8(0x1F);
        const __m256i vUpperPlus1 = _mm256_set1_epi8(0x7F);
        
        while (j + 32 <= nBufferSize) {
            __m256i vData = _mm256_loadu_si256((const __m256i*)(pData + j));
            __m256i vGe = _mm256_cmpgt_epi8(vData, vLowerMinus1);
            __m256i vLe = _mm256_cmpgt_epi8(vUpperPlus1, vData);
            __m256i vAnsi = _mm256_and_si256(vGe, vLe);
            xsimd_int32 nMask = _mm256_movemask_epi8(vAnsi);
            
            if (nMask != 0) {
#ifdef _MSC_VER
                unsigned long nBitPos;
                _BitScanForward(&nBitPos, (unsigned long)nMask);
                xsimd_int64 start = j + nBitPos;
#else
                xsimd_int64 start = j + __builtin_ctz((unsigned int)nMask);
#endif
                xsimd_int64 runLen = 0;
                while ((start + runLen) < nBufferSize && pData[start + runLen] >= 0x20 && pData[start + runLen] <= 0x7E) {
                    runLen++;
                }
                
                if (runLen >= nMinLength) {
                    return nOffset + start;
                }
                
                j = start + runLen + 1;
            } else {
                j += 32;
            }
        }
    }
    /* SSE2: Process 16 bytes at a time */
    else if ((g_nEnabledFeatures & XSIMD_FEATURE_SSE2) && nBufferSize >= 16) {
        const __m128i vLowerMinus1 = _mm_set1_epi8(0x1F);
        const __m128i vUpperPlus1 = _mm_set1_epi8(0x7F);
        
        /* Process 64 bytes per iteration for better throughput */
        while (j + 64 <= nBufferSize) {
            __m128i vData0 = _mm_loadu_si128((const __m128i*)(pData + j));
            __m128i vData1 = _mm_loadu_si128((const __m128i*)(pData + j + 16));
            __m128i vData2 = _mm_loadu_si128((const __m128i*)(pData + j + 32));
            __m128i vData3 = _mm_loadu_si128((const __m128i*)(pData + j + 48));
            
            __m128i vGe0 = _mm_cmpgt_epi8(vData0, vLowerMinus1);
            __m128i vLe0 = _mm_cmpgt_epi8(vUpperPlus1, vData0);
            __m128i vAnsi0 = _mm_and_si128(vGe0, vLe0);
            
            __m128i vGe1 = _mm_cmpgt_epi8(vData1, vLowerMinus1);
            __m128i vLe1 = _mm_cmpgt_epi8(vUpperPlus1, vData1);
            __m128i vAnsi1 = _mm_and_si128(vGe1, vLe1);
            
            __m128i vGe2 = _mm_cmpgt_epi8(vData2, vLowerMinus1);
            __m128i vLe2 = _mm_cmpgt_epi8(vUpperPlus1, vData2);
            __m128i vAnsi2 = _mm_and_si128(vGe2, vLe2);
            
            __m128i vGe3 = _mm_cmpgt_epi8(vData3, vLowerMinus1);
            __m128i vLe3 = _mm_cmpgt_epi8(vUpperPlus1, vData3);
            __m128i vAnsi3 = _mm_and_si128(vGe3, vLe3);
            
            int nMask0 = _mm_movemask_epi8(vAnsi0);
            int nMask1 = _mm_movemask_epi8(vAnsi1);
            int nMask2 = _mm_movemask_epi8(vAnsi2);
            int nMask3 = _mm_movemask_epi8(vAnsi3);
            
            if (nMask0 != 0) {
#ifdef _MSC_VER
                unsigned long nBitPos;
                _BitScanForward(&nBitPos, (unsigned long)nMask0);
                xsimd_int64 start = j + nBitPos;
#else
                xsimd_int64 start = j + __builtin_ctz((unsigned int)nMask0);
#endif
                xsimd_int64 runLen = 0;
                while ((start + runLen) < nBufferSize && pData[start + runLen] >= 0x20 && pData[start + runLen] <= 0x7E) {
                    runLen++;
                }
                
                if (runLen >= nMinLength) {
                    return nOffset + start;
                }
                
                j = start + runLen + 1;
                continue;
            }
            
            if (nMask1 != 0) {
#ifdef _MSC_VER
                unsigned long nBitPos;
                _BitScanForward(&nBitPos, (unsigned long)nMask1);
                xsimd_int64 start = j + 16 + nBitPos;
#else
                xsimd_int64 start = j + 16 + __builtin_ctz((unsigned int)nMask1);
#endif
                xsimd_int64 runLen = 0;
                while ((start + runLen) < nBufferSize && pData[start + runLen] >= 0x20 && pData[start + runLen] <= 0x7E) {
                    runLen++;
                }
                
                if (runLen >= nMinLength) {
                    return nOffset + start;
                }
                
                j = start + runLen + 1;
                continue;
            }
            
            if (nMask2 != 0) {
#ifdef _MSC_VER
                unsigned long nBitPos;
                _BitScanForward(&nBitPos, (unsigned long)nMask2);
                xsimd_int64 start = j + 32 + nBitPos;
#else
                xsimd_int64 start = j + 32 + __builtin_ctz((unsigned int)nMask2);
#endif
                xsimd_int64 runLen = 0;
                while ((start + runLen) < nBufferSize && pData[start + runLen] >= 0x20 && pData[start + runLen] <= 0x7E) {
                    runLen++;
                }
                
                if (runLen >= nMinLength) {
                    return nOffset + start;
                }
                
                j = start + runLen + 1;
                continue;
            }
            
            if (nMask3 != 0) {
#ifdef _MSC_VER
                unsigned long nBitPos;
                _BitScanForward(&nBitPos, (unsigned long)nMask3);
                xsimd_int64 start = j + 48 + nBitPos;
#else
                xsimd_int64 start = j + 48 + __builtin_ctz((unsigned int)nMask3);
#endif
                xsimd_int64 runLen = 0;
                while ((start + runLen) < nBufferSize && pData[start + runLen] >= 0x20 && pData[start + runLen] <= 0x7E) {
                    runLen++;
                }
                
                if (runLen >= nMinLength) {
                    return nOffset + start;
                }
                
                j = start + runLen + 1;
                continue;
            }
            
            j += 64;
        }
        
        /* Process remaining 16-byte chunks */
        while (j + 16 <= nBufferSize) {
            __m128i vData = _mm_loadu_si128((const __m128i*)(pData + j));
            __m128i vGe = _mm_cmpgt_epi8(vData, vLowerMinus1);
            __m128i vLe = _mm_cmpgt_epi8(vUpperPlus1, vData);
            __m128i vAnsi = _mm_and_si128(vGe, vLe);
            xsimd_int32 nMask = _mm_movemask_epi8(vAnsi);
            
            if (nMask != 0) {
#ifdef _MSC_VER
                unsigned long nBitPos;
                _BitScanForward(&nBitPos, (unsigned long)nMask);
                xsimd_int64 start = j + nBitPos;
#else
                xsimd_int64 start = j + __builtin_ctz((unsigned int)nMask);
#endif
                xsimd_int64 runLen = 0;
                while ((start + runLen) < nBufferSize && pData[start + runLen] >= 0x20 && pData[start + runLen] <= 0x7E) {
                    runLen++;
                }
                
                if (runLen >= nMinLength) {
                    return nOffset + start;
                }
                
                j = start + runLen + 1;
            } else {
                j += 16;
            }
        }
    }
#endif
    
    /* Fallback */
    while (j < nLimit) {
        while (j < nBufferSize && (pData[j] < 0x20 || pData[j] > 0x7E)) j++;
        if (j >= nLimit) break;
        
        xsimd_int64 start = j;
        while (j < nBufferSize && pData[j] >= 0x20 && pData[j] <= 0x7E) j++;
        
        if ((j - start) >= nMinLength) {
            return nOffset + start;
        }
    }
    
    return -1;
}

xsimd_int64 xsimd_find_notnull(const void* pBuffer, xsimd_int64 nBufferSize, 
                               xsimd_int64 nMinLength, xsimd_int64 nOffset)
{
    const unsigned char* pData = (const unsigned char*)pBuffer;
    xsimd_int64 j = 0;
    xsimd_int64 runStart = -1;  // Track start of current non-null run
    
    /* Fast path for very short minimum lengths (1-3 bytes) */
    /* SIMD overhead outweighs benefits for tiny searches */
    if (nMinLength <= 3) {
        /* Skip leading nulls */
        while (j < nBufferSize && pData[j] == 0) j++;
        if (j >= nBufferSize) return -1;
        
        /* For length 1, we're done - found first non-null */
        if (nMinLength == 1) return nOffset + j;
        
        /* For length 2-3, verify consecutive non-null bytes */
        runStart = j;
        j++;
        
        while (j < nBufferSize) {
            if (pData[j] == 0) {
                xsimd_int64 runLen = j - runStart;
                if (runLen >= nMinLength) return nOffset + runStart;
                
                /* Skip past null byte(s) and find next non-null */
                j++;
                while (j < nBufferSize && pData[j] == 0) j++;
                if (j >= nBufferSize) return -1;
                runStart = j;
            }
            j++;
        }
        
        /* Check final run */
        xsimd_int64 runLen = j - runStart;
        if (runLen >= nMinLength) return nOffset + runStart;
        return -1;
    }
    
#ifdef XSIMD_X86
    /* AVX2: Process 32 bytes at a time */
    if ((g_nEnabledFeatures & XSIMD_FEATURE_AVX2) && nBufferSize >= 32) {
        const __m256i vZero = _mm256_setzero_si256();
        
        /* First, skip any leading nulls */
        while (j < nBufferSize && pData[j] == 0) j++;
        if (j >= nBufferSize) return -1;
        runStart = j;
        
        /* Process 32 bytes per iteration */
        while (j + 32 <= nBufferSize) {
            __m256i vData = _mm256_loadu_si256((const __m256i*)(pData + j));
            __m256i vCmpZero = _mm256_cmpeq_epi8(vData, vZero);
            xsimd_int32 nMask = _mm256_movemask_epi8(vCmpZero);
            
            if (nMask != 0) {
                /* Found a null byte in this chunk */
#ifdef _MSC_VER
                unsigned long nBitPos;
                _BitScanForward(&nBitPos, (unsigned long)nMask);
                xsimd_int64 nullPos = j + nBitPos;
#else
                xsimd_int64 nullPos = j + __builtin_ctz((unsigned int)nMask);
#endif
                /* Check if current run is long enough */
                xsimd_int64 runLen = nullPos - runStart;
                if (runLen >= nMinLength) {
                    return nOffset + runStart;
                }
                
                /* Skip past null byte(s) and find start of next run */
                j = nullPos + 1;
                while (j < nBufferSize && pData[j] == 0) j++;
                if (j >= nBufferSize) return -1;
                runStart = j;
            } else {
                /* No nulls in this 32-byte chunk */
                j += 32;
            }
        }
    }
    /* AVX: Process 32 bytes at a time */
    else if ((g_nEnabledFeatures & XSIMD_FEATURE_AVX) && nBufferSize >= 32) {
        const __m256i vZero = _mm256_setzero_si256();
        
        /* Skip leading nulls */
        while (j < nBufferSize && pData[j] == 0) j++;
        if (j >= nBufferSize) return -1;
        runStart = j;
        
        while (j + 32 <= nBufferSize) {
            __m256i vData = _mm256_loadu_si256((const __m256i*)(pData + j));
            __m256i vCmpZero = _mm256_cmpeq_epi8(vData, vZero);
            xsimd_int32 nMask = _mm256_movemask_epi8(vCmpZero);
            
            if (nMask != 0) {
#ifdef _MSC_VER
                unsigned long nBitPos;
                _BitScanForward(&nBitPos, (unsigned long)nMask);
                xsimd_int64 nullPos = j + nBitPos;
#else
                xsimd_int64 nullPos = j + __builtin_ctz((unsigned int)nMask);
#endif
                xsimd_int64 runLen = nullPos - runStart;
                if (runLen >= nMinLength) {
                    return nOffset + runStart;
                }
                
                j = nullPos + 1;
                while (j < nBufferSize && pData[j] == 0) j++;
                if (j >= nBufferSize) return -1;
                runStart = j;
            } else {
                j += 32;
            }
        }
    }
    /* SSE2: Process 16 bytes at a time */
    else if ((g_nEnabledFeatures & XSIMD_FEATURE_SSE2) && nBufferSize >= 16) {
        const __m128i vZero = _mm_setzero_si128();
        
        /* Skip leading nulls */
        while (j < nBufferSize && pData[j] == 0) j++;
        if (j >= nBufferSize) return -1;
        runStart = j;
        
        while (j + 16 <= nBufferSize) {
            __m128i vData = _mm_loadu_si128((const __m128i*)(pData + j));
            __m128i vCmpZero = _mm_cmpeq_epi8(vData, vZero);
            xsimd_int32 nMask = _mm_movemask_epi8(vCmpZero);
            
            if (nMask != 0) {
#ifdef _MSC_VER
                unsigned long nBitPos;
                _BitScanForward(&nBitPos, (unsigned long)nMask);
                xsimd_int64 nullPos = j + nBitPos;
#else
                xsimd_int64 nullPos = j + __builtin_ctz((unsigned int)nMask);
#endif
                xsimd_int64 runLen = nullPos - runStart;
                if (runLen >= nMinLength) {
                    return nOffset + runStart;
                }
                
                j = nullPos + 1;
                while (j < nBufferSize && pData[j] == 0) j++;
                if (j >= nBufferSize) return -1;
                runStart = j;
            } else {
                j += 16;
            }
        }
    }
#endif
    
    /* Fallback: Process remaining bytes scalar */
    if (runStart == -1) {
        /* Haven't started processing yet, skip leading nulls */
        while (j < nBufferSize && pData[j] == 0) j++;
        if (j >= nBufferSize) return -1;
        runStart = j;
    }
    
    /* Check remaining bytes */
    while (j < nBufferSize) {
        if (pData[j] == 0) {
            xsimd_int64 runLen = j - runStart;
            if (runLen >= nMinLength) {
                return nOffset + runStart;
            }
            /* Skip past null byte(s) */
            j++;
            while (j < nBufferSize && pData[j] == 0) j++;
            if (j >= nBufferSize) return -1;
            runStart = j;
        } else {
            j++;
        }
    }
    
    /* Check final run */
    xsimd_int64 runLen = j - runStart;
    if (runLen >= nMinLength) {
        return nOffset + runStart;
    }
    
    return -1;
}

xsimd_int64 xsimd_find_not_ansi(const void* pBuffer, xsimd_int64 nBufferSize, 
                                xsimd_int64 nMinLength, xsimd_int64 nOffset)
{
    const unsigned char* pData = (const unsigned char*)pBuffer;
    const xsimd_int64 nLimit = nBufferSize - (nMinLength - 1);
    xsimd_int64 j = 0;
    
#ifdef XSIMD_X86
    /* AVX2: Process 32 bytes at a time */
    if ((g_nEnabledFeatures & XSIMD_FEATURE_AVX2) && nBufferSize >= 32) {
        const __m256i vLower = _mm256_set1_epi8(0x20);
        const __m256i vUpper = _mm256_set1_epi8(0x7E);
        
        /* Process 64 bytes per iteration for better throughput */
        while (j + 64 <= nBufferSize) {
            __m256i vData0 = _mm256_loadu_si256((const __m256i*)(pData + j));
            __m256i vData1 = _mm256_loadu_si256((const __m256i*)(pData + j + 32));
            
            __m256i vLt0 = _mm256_cmpgt_epi8(vLower, vData0);
            __m256i vGt0 = _mm256_cmpgt_epi8(vData0, vUpper);
            __m256i vNotAnsi0 = _mm256_or_si256(vLt0, vGt0);
            
            __m256i vLt1 = _mm256_cmpgt_epi8(vLower, vData1);
            __m256i vGt1 = _mm256_cmpgt_epi8(vData1, vUpper);
            __m256i vNotAnsi1 = _mm256_or_si256(vLt1, vGt1);
            
            xsimd_int32 nMask0 = _mm256_movemask_epi8(vNotAnsi0);
            xsimd_int32 nMask1 = _mm256_movemask_epi8(vNotAnsi1);
            
            if (nMask0 != 0) {
#ifdef _MSC_VER
                unsigned long nBitPos;
                _BitScanForward(&nBitPos, (unsigned long)nMask0);
                xsimd_int64 start = j + nBitPos;
#else
                xsimd_int64 start = j + __builtin_ctz((unsigned int)nMask0);
#endif
                xsimd_int64 runLen = 0;
                while ((start + runLen) < nBufferSize && (pData[start + runLen] < 0x20 || pData[start + runLen] > 0x7E)) {
                    runLen++;
                }
                
                if (runLen >= nMinLength) {
                    return nOffset + start;
                }
                
                j = start + runLen + 1;
                continue;
            }
            
            if (nMask1 != 0) {
#ifdef _MSC_VER
                unsigned long nBitPos;
                _BitScanForward(&nBitPos, (unsigned long)nMask1);
                xsimd_int64 start = j + 32 + nBitPos;
#else
                xsimd_int64 start = j + 32 + __builtin_ctz((unsigned int)nMask1);
#endif
                xsimd_int64 runLen = 0;
                while ((start + runLen) < nBufferSize && (pData[start + runLen] < 0x20 || pData[start + runLen] > 0x7E)) {
                    runLen++;
                }
                
                if (runLen >= nMinLength) {
                    return nOffset + start;
                }
                
                j = start + runLen + 1;
                continue;
            }
            
            j += 64;
        }
        
        /* Process remaining 32-byte chunks */
        while (j + 32 <= nBufferSize) {
            __m256i vData = _mm256_loadu_si256((const __m256i*)(pData + j));
            __m256i vLt = _mm256_cmpgt_epi8(vLower, vData);
            __m256i vGt = _mm256_cmpgt_epi8(vData, vUpper);
            __m256i vNotAnsi = _mm256_or_si256(vLt, vGt);
            xsimd_int32 nMask = _mm256_movemask_epi8(vNotAnsi);
            
            if (nMask != 0) {
#ifdef _MSC_VER
                unsigned long nBitPos;
                _BitScanForward(&nBitPos, (unsigned long)nMask);
                xsimd_int64 start = j + nBitPos;
#else
                xsimd_int64 start = j + __builtin_ctz((unsigned int)nMask);
#endif
                xsimd_int64 runLen = 0;
                while ((start + runLen) < nBufferSize && (pData[start + runLen] < 0x20 || pData[start + runLen] > 0x7E)) {
                    runLen++;
                }
                
                if (runLen >= nMinLength) {
                    return nOffset + start;
                }
                
                j = start + runLen + 1;
            } else {
                j += 32;
            }
        }
    }
    /* AVX: Process 32 bytes at a time */
    else if ((g_nEnabledFeatures & XSIMD_FEATURE_AVX) && nBufferSize >= 32) {
        const __m256i vLower = _mm256_set1_epi8(0x20);
        const __m256i vUpper = _mm256_set1_epi8(0x7E);
        
        while (j + 32 <= nBufferSize) {
            __m256i vData = _mm256_loadu_si256((const __m256i*)(pData + j));
            __m256i vLt = _mm256_cmpgt_epi8(vLower, vData);
            __m256i vGt = _mm256_cmpgt_epi8(vData, vUpper);
            __m256i vNotAnsi = _mm256_or_si256(vLt, vGt);
            xsimd_int32 nMask = _mm256_movemask_epi8(vNotAnsi);
            
            if (nMask != 0) {
#ifdef _MSC_VER
                unsigned long nBitPos;
                _BitScanForward(&nBitPos, (unsigned long)nMask);
                xsimd_int64 start = j + nBitPos;
#else
                xsimd_int64 start = j + __builtin_ctz((unsigned int)nMask);
#endif
                xsimd_int64 runLen = 0;
                while ((start + runLen) < nBufferSize && (pData[start + runLen] < 0x20 || pData[start + runLen] > 0x7E)) {
                    runLen++;
                }
                
                if (runLen >= nMinLength) {
                    return nOffset + start;
                }
                
                j = start + runLen + 1;
            } else {
                j += 32;
            }
        }
    }
    /* SSE2: Process 16 bytes at a time */
    else if ((g_nEnabledFeatures & XSIMD_FEATURE_SSE2) && nBufferSize >= 16) {
        const __m128i vLower = _mm_set1_epi8(0x20);
        const __m128i vUpper = _mm_set1_epi8(0x7E);
        
        /* Process 64 bytes per iteration for better throughput */
        while (j + 64 <= nBufferSize) {
            __m128i vData0 = _mm_loadu_si128((const __m128i*)(pData + j));
            __m128i vData1 = _mm_loadu_si128((const __m128i*)(pData + j + 16));
            __m128i vData2 = _mm_loadu_si128((const __m128i*)(pData + j + 32));
            __m128i vData3 = _mm_loadu_si128((const __m128i*)(pData + j + 48));
            
            __m128i vLt0 = _mm_cmpgt_epi8(vLower, vData0);
            __m128i vGt0 = _mm_cmpgt_epi8(vData0, vUpper);
            __m128i vNotAnsi0 = _mm_or_si128(vLt0, vGt0);
            
            __m128i vLt1 = _mm_cmpgt_epi8(vLower, vData1);
            __m128i vGt1 = _mm_cmpgt_epi8(vData1, vUpper);
            __m128i vNotAnsi1 = _mm_or_si128(vLt1, vGt1);
            
            __m128i vLt2 = _mm_cmpgt_epi8(vLower, vData2);
            __m128i vGt2 = _mm_cmpgt_epi8(vData2, vUpper);
            __m128i vNotAnsi2 = _mm_or_si128(vLt2, vGt2);
            
            __m128i vLt3 = _mm_cmpgt_epi8(vLower, vData3);
            __m128i vGt3 = _mm_cmpgt_epi8(vData3, vUpper);
            __m128i vNotAnsi3 = _mm_or_si128(vLt3, vGt3);
            
            int nMask0 = _mm_movemask_epi8(vNotAnsi0);
            int nMask1 = _mm_movemask_epi8(vNotAnsi1);
            int nMask2 = _mm_movemask_epi8(vNotAnsi2);
            int nMask3 = _mm_movemask_epi8(vNotAnsi3);
            
            if (nMask0 != 0) {
#ifdef _MSC_VER
                unsigned long nBitPos;
                _BitScanForward(&nBitPos, (unsigned long)nMask0);
                xsimd_int64 start = j + nBitPos;
#else
                xsimd_int64 start = j + __builtin_ctz((unsigned int)nMask0);
#endif
                xsimd_int64 runLen = 0;
                while ((start + runLen) < nBufferSize && (pData[start + runLen] < 0x20 || pData[start + runLen] > 0x7E)) {
                    runLen++;
                }
                
                if (runLen >= nMinLength) {
                    return nOffset + start;
                }
                
                j = start + runLen + 1;
                continue;
            }
            
            if (nMask1 != 0) {
#ifdef _MSC_VER
                unsigned long nBitPos;
                _BitScanForward(&nBitPos, (unsigned long)nMask1);
                xsimd_int64 start = j + 16 + nBitPos;
#else
                xsimd_int64 start = j + 16 + __builtin_ctz((unsigned int)nMask1);
#endif
                xsimd_int64 runLen = 0;
                while ((start + runLen) < nBufferSize && (pData[start + runLen] < 0x20 || pData[start + runLen] > 0x7E)) {
                    runLen++;
                }
                
                if (runLen >= nMinLength) {
                    return nOffset + start;
                }
                
                j = start + runLen + 1;
                continue;
            }
            
            if (nMask2 != 0) {
#ifdef _MSC_VER
                unsigned long nBitPos;
                _BitScanForward(&nBitPos, (unsigned long)nMask2);
                xsimd_int64 start = j + 32 + nBitPos;
#else
                xsimd_int64 start = j + 32 + __builtin_ctz((unsigned int)nMask2);
#endif
                xsimd_int64 runLen = 0;
                while ((start + runLen) < nBufferSize && (pData[start + runLen] < 0x20 || pData[start + runLen] > 0x7E)) {
                    runLen++;
                }
                
                if (runLen >= nMinLength) {
                    return nOffset + start;
                }
                
                j = start + runLen + 1;
                continue;
            }
            
            if (nMask3 != 0) {
#ifdef _MSC_VER
                unsigned long nBitPos;
                _BitScanForward(&nBitPos, (unsigned long)nMask3);
                xsimd_int64 start = j + 48 + nBitPos;
#else
                xsimd_int64 start = j + 48 + __builtin_ctz((unsigned int)nMask3);
#endif
                xsimd_int64 runLen = 0;
                while ((start + runLen) < nBufferSize && (pData[start + runLen] < 0x20 || pData[start + runLen] > 0x7E)) {
                    runLen++;
                }
                
                if (runLen >= nMinLength) {
                    return nOffset + start;
                }
                
                j = start + runLen + 1;
                continue;
            }
            
            j += 64;
        }
        
        /* Process remaining 16-byte chunks */
        while (j + 16 <= nBufferSize) {
            __m128i vData = _mm_loadu_si128((const __m128i*)(pData + j));
            __m128i vLt = _mm_cmpgt_epi8(vLower, vData);
            __m128i vGt = _mm_cmpgt_epi8(vData, vUpper);
            __m128i vNotAnsi = _mm_or_si128(vLt, vGt);
            xsimd_int32 nMask = _mm_movemask_epi8(vNotAnsi);
            
            if (nMask != 0) {
#ifdef _MSC_VER
                unsigned long nBitPos;
                _BitScanForward(&nBitPos, (unsigned long)nMask);
                xsimd_int64 start = j + nBitPos;
#else
                xsimd_int64 start = j + __builtin_ctz((unsigned int)nMask);
#endif
                xsimd_int64 runLen = 0;
                while ((start + runLen) < nBufferSize && (pData[start + runLen] < 0x20 || pData[start + runLen] > 0x7E)) {
                    runLen++;
                }
                
                if (runLen >= nMinLength) {
                    return nOffset + start;
                }
                
                j = start + runLen + 1;
            } else {
                j += 16;
            }
        }
    }
#endif
    
    /* Fallback */
    while (j < nLimit) {
        while (j < nBufferSize && (pData[j] >= 0x20 && pData[j] <= 0x7E)) j++;
        if (j >= nLimit) break;
        
        xsimd_int64 start = j;
        while (j < nBufferSize && (pData[j] < 0x20 || pData[j] > 0x7E)) j++;
        
        if ((j - start) >= nMinLength) {
            return nOffset + start;
        }
    }
    
    return -1;
}

int xsimd_is_not_null(const void* pBuffer, xsimd_int64 nSize)
{
    const char* ptr = (const char*)pBuffer;
    
#ifdef XSIMD_X86
    /* AVX2: Process 32 bytes at a time */
    if ((g_nEnabledFeatures & XSIMD_FEATURE_AVX2) && nSize >= 32) {
        const __m256i zero = _mm256_setzero_si256();
        
        /* Process 128 bytes per iteration for better throughput */
        while (nSize >= 128) {
            __m256i chunk0 = _mm256_loadu_si256((const __m256i*)ptr);
            __m256i chunk1 = _mm256_loadu_si256((const __m256i*)(ptr + 32));
            __m256i chunk2 = _mm256_loadu_si256((const __m256i*)(ptr + 64));
            __m256i chunk3 = _mm256_loadu_si256((const __m256i*)(ptr + 96));
            
            __m256i cmp0 = _mm256_cmpeq_epi8(chunk0, zero);
            __m256i cmp1 = _mm256_cmpeq_epi8(chunk1, zero);
            __m256i cmp2 = _mm256_cmpeq_epi8(chunk2, zero);
            __m256i cmp3 = _mm256_cmpeq_epi8(chunk3, zero);
            
            xsimd_int32 mask0 = _mm256_movemask_epi8(cmp0);
            xsimd_int32 mask1 = _mm256_movemask_epi8(cmp1);
            xsimd_int32 mask2 = _mm256_movemask_epi8(cmp2);
            xsimd_int32 mask3 = _mm256_movemask_epi8(cmp3);
            
            if (mask0 != 0 || mask1 != 0 || mask2 != 0 || mask3 != 0) {
                return 0;
            }
            
            ptr += 128;
            nSize -= 128;
        }
        
        /* Process remaining 32-byte chunks */
        while (nSize >= 32) {
            __m256i chunk = _mm256_loadu_si256((const __m256i*)ptr);
            __m256i cmp = _mm256_cmpeq_epi8(chunk, zero);
            xsimd_int32 mask = _mm256_movemask_epi8(cmp);
            
            if (mask != 0) {
                return 0;
            }
            
            ptr += 32;
            nSize -= 32;
        }
    }
    /* AVX: Process 32 bytes at a time */
    else if ((g_nEnabledFeatures & XSIMD_FEATURE_AVX) && nSize >= 32) {
        const __m256i zero = _mm256_setzero_si256();
        
        while (nSize >= 32) {
            __m256i chunk = _mm256_loadu_si256((const __m256i*)ptr);
            __m256i cmp = _mm256_cmpeq_epi8(chunk, zero);
            xsimd_int32 mask = _mm256_movemask_epi8(cmp);
            
            if (mask != 0) {
                return 0;
            }
            
            ptr += 32;
            nSize -= 32;
        }
    }
    /* SSE2: Process 16 bytes at a time */
    else if ((g_nEnabledFeatures & XSIMD_FEATURE_SSE2) && nSize >= 16) {
        const __m128i zero = _mm_setzero_si128();
        
        /* Process 128 bytes per iteration for better throughput */
        while (nSize >= 128) {
            __m128i chunk0 = _mm_loadu_si128((const __m128i*)ptr);
            __m128i chunk1 = _mm_loadu_si128((const __m128i*)(ptr + 16));
            __m128i chunk2 = _mm_loadu_si128((const __m128i*)(ptr + 32));
            __m128i chunk3 = _mm_loadu_si128((const __m128i*)(ptr + 48));
            __m128i chunk4 = _mm_loadu_si128((const __m128i*)(ptr + 64));
            __m128i chunk5 = _mm_loadu_si128((const __m128i*)(ptr + 80));
            __m128i chunk6 = _mm_loadu_si128((const __m128i*)(ptr + 96));
            __m128i chunk7 = _mm_loadu_si128((const __m128i*)(ptr + 112));
            
            __m128i cmp0 = _mm_cmpeq_epi8(chunk0, zero);
            __m128i cmp1 = _mm_cmpeq_epi8(chunk1, zero);
            __m128i cmp2 = _mm_cmpeq_epi8(chunk2, zero);
            __m128i cmp3 = _mm_cmpeq_epi8(chunk3, zero);
            __m128i cmp4 = _mm_cmpeq_epi8(chunk4, zero);
            __m128i cmp5 = _mm_cmpeq_epi8(chunk5, zero);
            __m128i cmp6 = _mm_cmpeq_epi8(chunk6, zero);
            __m128i cmp7 = _mm_cmpeq_epi8(chunk7, zero);
            
            int mask0 = _mm_movemask_epi8(cmp0);
            int mask1 = _mm_movemask_epi8(cmp1);
            int mask2 = _mm_movemask_epi8(cmp2);
            int mask3 = _mm_movemask_epi8(cmp3);
            int mask4 = _mm_movemask_epi8(cmp4);
            int mask5 = _mm_movemask_epi8(cmp5);
            int mask6 = _mm_movemask_epi8(cmp6);
            int mask7 = _mm_movemask_epi8(cmp7);
            
            if (mask0 != 0 || mask1 != 0 || mask2 != 0 || mask3 != 0 ||
                mask4 != 0 || mask5 != 0 || mask6 != 0 || mask7 != 0) {
                return 0;
            }
            
            ptr += 128;
            nSize -= 128;
        }
        
        /* Process remaining 16-byte chunks */
        while (nSize >= 16) {
            __m128i chunk = _mm_loadu_si128((const __m128i*)ptr);
            __m128i cmp = _mm_cmpeq_epi8(chunk, zero);
            int mask = _mm_movemask_epi8(cmp);
            
            if (mask != 0) {
                return 0;
            }
            
            ptr += 16;
            nSize -= 16;
        }
    }
#endif
    
    /* Fallback */
    while (nSize > 0) {
        if (*ptr == 0) {
            return 0;
        }
        ptr++;
        nSize--;
    }
    
    return 1;
}

int xsimd_is_ansi(const void* pBuffer, xsimd_int64 nSize)
{
    const char* ptr = (const char*)pBuffer;
    
#ifdef XSIMD_X86
    /* AVX2: Process 32 bytes at a time */
    if ((g_nEnabledFeatures & XSIMD_FEATURE_AVX2) && nSize >= 32) {
        const __m256i low_bound = _mm256_set1_epi8(0x20 - 1);
        const __m256i high_bound = _mm256_set1_epi8(0x7F);
        
        /* Process 128 bytes per iteration for better throughput */
        while (nSize >= 128) {
            __m256i chunk0 = _mm256_loadu_si256((const __m256i*)ptr);
            __m256i chunk1 = _mm256_loadu_si256((const __m256i*)(ptr + 32));
            __m256i chunk2 = _mm256_loadu_si256((const __m256i*)(ptr + 64));
            __m256i chunk3 = _mm256_loadu_si256((const __m256i*)(ptr + 96));
            
            __m256i too_low0 = _mm256_cmpgt_epi8(low_bound, chunk0);
            __m256i too_high0 = _mm256_cmpgt_epi8(chunk0, high_bound);
            __m256i invalid0 = _mm256_or_si256(too_low0, too_high0);
            
            __m256i too_low1 = _mm256_cmpgt_epi8(low_bound, chunk1);
            __m256i too_high1 = _mm256_cmpgt_epi8(chunk1, high_bound);
            __m256i invalid1 = _mm256_or_si256(too_low1, too_high1);
            
            __m256i too_low2 = _mm256_cmpgt_epi8(low_bound, chunk2);
            __m256i too_high2 = _mm256_cmpgt_epi8(chunk2, high_bound);
            __m256i invalid2 = _mm256_or_si256(too_low2, too_high2);
            
            __m256i too_low3 = _mm256_cmpgt_epi8(low_bound, chunk3);
            __m256i too_high3 = _mm256_cmpgt_epi8(chunk3, high_bound);
            __m256i invalid3 = _mm256_or_si256(too_low3, too_high3);
            
            if (_mm256_movemask_epi8(invalid0) != 0 || _mm256_movemask_epi8(invalid1) != 0 ||
                _mm256_movemask_epi8(invalid2) != 0 || _mm256_movemask_epi8(invalid3) != 0) {
                return 0;
            }
            
            ptr += 128;
            nSize -= 128;
        }
        
        /* Process remaining 32-byte chunks */
        while (nSize >= 32) {
            __m256i chunk = _mm256_loadu_si256((const __m256i*)ptr);
            __m256i too_low = _mm256_cmpgt_epi8(low_bound, chunk);
            __m256i too_high = _mm256_cmpgt_epi8(chunk, high_bound);
            __m256i invalid = _mm256_or_si256(too_low, too_high);
            
            if (_mm256_movemask_epi8(invalid) != 0) {
                return 0;
            }
            
            ptr += 32;
            nSize -= 32;
        }
    }
    /* AVX: Process 32 bytes at a time */
    else if ((g_nEnabledFeatures & XSIMD_FEATURE_AVX) && nSize >= 32) {
        const __m256i low_bound = _mm256_set1_epi8(0x20 - 1);
        const __m256i high_bound = _mm256_set1_epi8(0x7F);
        
        while (nSize >= 32) {
            __m256i chunk = _mm256_loadu_si256((const __m256i*)ptr);
            __m256i too_low = _mm256_cmpgt_epi8(low_bound, chunk);
            __m256i too_high = _mm256_cmpgt_epi8(chunk, high_bound);
            __m256i invalid = _mm256_or_si256(too_low, too_high);
            
            if (_mm256_movemask_epi8(invalid) != 0) {
                return 0;
            }
            
            ptr += 32;
            nSize -= 32;
        }
    }
    /* SSE2: Process 16 bytes at a time */
    else if ((g_nEnabledFeatures & XSIMD_FEATURE_SSE2) && nSize >= 16) {
        const __m128i low_bound = _mm_set1_epi8(0x20 - 1);
        const __m128i high_bound = _mm_set1_epi8(0x7F);
        
        /* Process 128 bytes per iteration for better throughput */
        while (nSize >= 128) {
            __m128i chunk0 = _mm_loadu_si128((const __m128i*)ptr);
            __m128i chunk1 = _mm_loadu_si128((const __m128i*)(ptr + 16));
            __m128i chunk2 = _mm_loadu_si128((const __m128i*)(ptr + 32));
            __m128i chunk3 = _mm_loadu_si128((const __m128i*)(ptr + 48));
            __m128i chunk4 = _mm_loadu_si128((const __m128i*)(ptr + 64));
            __m128i chunk5 = _mm_loadu_si128((const __m128i*)(ptr + 80));
            __m128i chunk6 = _mm_loadu_si128((const __m128i*)(ptr + 96));
            __m128i chunk7 = _mm_loadu_si128((const __m128i*)(ptr + 112));
            
            __m128i too_low0 = _mm_cmpgt_epi8(low_bound, chunk0);
            __m128i too_high0 = _mm_cmpgt_epi8(chunk0, high_bound);
            __m128i invalid0 = _mm_or_si128(too_low0, too_high0);
            
            __m128i too_low1 = _mm_cmpgt_epi8(low_bound, chunk1);
            __m128i too_high1 = _mm_cmpgt_epi8(chunk1, high_bound);
            __m128i invalid1 = _mm_or_si128(too_low1, too_high1);
            
            __m128i too_low2 = _mm_cmpgt_epi8(low_bound, chunk2);
            __m128i too_high2 = _mm_cmpgt_epi8(chunk2, high_bound);
            __m128i invalid2 = _mm_or_si128(too_low2, too_high2);
            
            __m128i too_low3 = _mm_cmpgt_epi8(low_bound, chunk3);
            __m128i too_high3 = _mm_cmpgt_epi8(chunk3, high_bound);
            __m128i invalid3 = _mm_or_si128(too_low3, too_high3);
            
            __m128i too_low4 = _mm_cmpgt_epi8(low_bound, chunk4);
            __m128i too_high4 = _mm_cmpgt_epi8(chunk4, high_bound);
            __m128i invalid4 = _mm_or_si128(too_low4, too_high4);
            
            __m128i too_low5 = _mm_cmpgt_epi8(low_bound, chunk5);
            __m128i too_high5 = _mm_cmpgt_epi8(chunk5, high_bound);
            __m128i invalid5 = _mm_or_si128(too_low5, too_high5);
            
            __m128i too_low6 = _mm_cmpgt_epi8(low_bound, chunk6);
            __m128i too_high6 = _mm_cmpgt_epi8(chunk6, high_bound);
            __m128i invalid6 = _mm_or_si128(too_low6, too_high6);
            
            __m128i too_low7 = _mm_cmpgt_epi8(low_bound, chunk7);
            __m128i too_high7 = _mm_cmpgt_epi8(chunk7, high_bound);
            __m128i invalid7 = _mm_or_si128(too_low7, too_high7);
            
            if (_mm_movemask_epi8(invalid0) != 0 || _mm_movemask_epi8(invalid1) != 0 ||
                _mm_movemask_epi8(invalid2) != 0 || _mm_movemask_epi8(invalid3) != 0 ||
                _mm_movemask_epi8(invalid4) != 0 || _mm_movemask_epi8(invalid5) != 0 ||
                _mm_movemask_epi8(invalid6) != 0 || _mm_movemask_epi8(invalid7) != 0) {
                return 0;
            }
            
            ptr += 128;
            nSize -= 128;
        }
        
        /* Process remaining 16-byte chunks */
        while (nSize >= 16) {
            __m128i chunk = _mm_loadu_si128((const __m128i*)ptr);
            __m128i too_low = _mm_cmpgt_epi8(low_bound, chunk);
            __m128i too_high = _mm_cmpgt_epi8(chunk, high_bound);
            __m128i invalid = _mm_or_si128(too_low, too_high);
            
            if (_mm_movemask_epi8(invalid) != 0) {
                return 0;
            }
            
            ptr += 16;
            nSize -= 16;
        }
    }
#endif
    
    /* Fallback */
    const xsimd_uint8* pData = (const xsimd_uint8*)ptr;
    while (nSize > 0) {
        if (*pData < 0x20 || *pData >= 0x80) {
            return 0;
        }
        pData++;
        nSize--;
    }
    
    return 1;
}

int xsimd_is_not_ansi(const void* pBuffer, xsimd_int64 nSize)
{
    const char* ptr = (const char*)pBuffer;
    
#ifdef XSIMD_X86
    /* AVX2: Process 32 bytes at a time */
    if ((g_nEnabledFeatures & XSIMD_FEATURE_AVX2) && nSize >= 32) {
        /* Process 128 bytes per iteration for better throughput */
        while (nSize >= 128) {
            __m256i chunk0 = _mm256_loadu_si256((const __m256i*)ptr);
            __m256i chunk1 = _mm256_loadu_si256((const __m256i*)(ptr + 32));
            __m256i chunk2 = _mm256_loadu_si256((const __m256i*)(ptr + 64));
            __m256i chunk3 = _mm256_loadu_si256((const __m256i*)(ptr + 96));
            
            __m256i ge_low0 = _mm256_cmpgt_epi8(chunk0, _mm256_set1_epi8(0x1F));
            __m256i le_high0 = _mm256_cmpgt_epi8(_mm256_set1_epi8((char)0x80), chunk0);
            __m256i printable0 = _mm256_and_si256(ge_low0, le_high0);
            
            __m256i ge_low1 = _mm256_cmpgt_epi8(chunk1, _mm256_set1_epi8(0x1F));
            __m256i le_high1 = _mm256_cmpgt_epi8(_mm256_set1_epi8((char)0x80), chunk1);
            __m256i printable1 = _mm256_and_si256(ge_low1, le_high1);
            
            __m256i ge_low2 = _mm256_cmpgt_epi8(chunk2, _mm256_set1_epi8(0x1F));
            __m256i le_high2 = _mm256_cmpgt_epi8(_mm256_set1_epi8((char)0x80), chunk2);
            __m256i printable2 = _mm256_and_si256(ge_low2, le_high2);
            
            __m256i ge_low3 = _mm256_cmpgt_epi8(chunk3, _mm256_set1_epi8(0x1F));
            __m256i le_high3 = _mm256_cmpgt_epi8(_mm256_set1_epi8((char)0x80), chunk3);
            __m256i printable3 = _mm256_and_si256(ge_low3, le_high3);
            
            if (_mm256_movemask_epi8(printable0) != 0 || _mm256_movemask_epi8(printable1) != 0 ||
                _mm256_movemask_epi8(printable2) != 0 || _mm256_movemask_epi8(printable3) != 0) {
                return 0;
            }
            
            ptr += 128;
            nSize -= 128;
        }
        
        /* Process remaining 32-byte chunks */
        while (nSize >= 32) {
            __m256i chunk = _mm256_loadu_si256((const __m256i*)ptr);
            __m256i ge_low = _mm256_cmpgt_epi8(chunk, _mm256_set1_epi8(0x1F));
            __m256i le_high = _mm256_cmpgt_epi8(_mm256_set1_epi8((char)0x80), chunk);
            __m256i printable = _mm256_and_si256(ge_low, le_high);
            
            if (_mm256_movemask_epi8(printable) != 0) {
                return 0;
            }
            
            ptr += 32;
            nSize -= 32;
        }
    }
    /* AVX: Process 32 bytes at a time */
    else if ((g_nEnabledFeatures & XSIMD_FEATURE_AVX) && nSize >= 32) {
        while (nSize >= 32) {
            __m256i chunk = _mm256_loadu_si256((const __m256i*)ptr);
            __m256i ge_low = _mm256_cmpgt_epi8(chunk, _mm256_set1_epi8(0x1F));
            __m256i le_high = _mm256_cmpgt_epi8(_mm256_set1_epi8((char)0x80), chunk);
            __m256i printable = _mm256_and_si256(ge_low, le_high);
            
            if (_mm256_movemask_epi8(printable) != 0) {
                return 0;
            }
            
            ptr += 32;
            nSize -= 32;
        }
    }
    /* SSE2: Process 16 bytes at a time */
    else if ((g_nEnabledFeatures & XSIMD_FEATURE_SSE2) && nSize >= 16) {
        /* Process 128 bytes per iteration for better throughput */
        while (nSize >= 128) {
            __m128i chunk0 = _mm_loadu_si128((const __m128i*)ptr);
            __m128i chunk1 = _mm_loadu_si128((const __m128i*)(ptr + 16));
            __m128i chunk2 = _mm_loadu_si128((const __m128i*)(ptr + 32));
            __m128i chunk3 = _mm_loadu_si128((const __m128i*)(ptr + 48));
            __m128i chunk4 = _mm_loadu_si128((const __m128i*)(ptr + 64));
            __m128i chunk5 = _mm_loadu_si128((const __m128i*)(ptr + 80));
            __m128i chunk6 = _mm_loadu_si128((const __m128i*)(ptr + 96));
            __m128i chunk7 = _mm_loadu_si128((const __m128i*)(ptr + 112));
            
            __m128i ge_low0 = _mm_cmpgt_epi8(chunk0, _mm_set1_epi8(0x1F));
            __m128i le_high0 = _mm_cmpgt_epi8(_mm_set1_epi8((char)0x80), chunk0);
            __m128i printable0 = _mm_and_si128(ge_low0, le_high0);
            
            __m128i ge_low1 = _mm_cmpgt_epi8(chunk1, _mm_set1_epi8(0x1F));
            __m128i le_high1 = _mm_cmpgt_epi8(_mm_set1_epi8((char)0x80), chunk1);
            __m128i printable1 = _mm_and_si128(ge_low1, le_high1);
            
            __m128i ge_low2 = _mm_cmpgt_epi8(chunk2, _mm_set1_epi8(0x1F));
            __m128i le_high2 = _mm_cmpgt_epi8(_mm_set1_epi8((char)0x80), chunk2);
            __m128i printable2 = _mm_and_si128(ge_low2, le_high2);
            
            __m128i ge_low3 = _mm_cmpgt_epi8(chunk3, _mm_set1_epi8(0x1F));
            __m128i le_high3 = _mm_cmpgt_epi8(_mm_set1_epi8((char)0x80), chunk3);
            __m128i printable3 = _mm_and_si128(ge_low3, le_high3);
            
            __m128i ge_low4 = _mm_cmpgt_epi8(chunk4, _mm_set1_epi8(0x1F));
            __m128i le_high4 = _mm_cmpgt_epi8(_mm_set1_epi8((char)0x80), chunk4);
            __m128i printable4 = _mm_and_si128(ge_low4, le_high4);
            
            __m128i ge_low5 = _mm_cmpgt_epi8(chunk5, _mm_set1_epi8(0x1F));
            __m128i le_high5 = _mm_cmpgt_epi8(_mm_set1_epi8((char)0x80), chunk5);
            __m128i printable5 = _mm_and_si128(ge_low5, le_high5);
            
            __m128i ge_low6 = _mm_cmpgt_epi8(chunk6, _mm_set1_epi8(0x1F));
            __m128i le_high6 = _mm_cmpgt_epi8(_mm_set1_epi8((char)0x80), chunk6);
            __m128i printable6 = _mm_and_si128(ge_low6, le_high6);
            
            __m128i ge_low7 = _mm_cmpgt_epi8(chunk7, _mm_set1_epi8(0x1F));
            __m128i le_high7 = _mm_cmpgt_epi8(_mm_set1_epi8((char)0x80), chunk7);
            __m128i printable7 = _mm_and_si128(ge_low7, le_high7);
            
            if (_mm_movemask_epi8(printable0) != 0 || _mm_movemask_epi8(printable1) != 0 ||
                _mm_movemask_epi8(printable2) != 0 || _mm_movemask_epi8(printable3) != 0 ||
                _mm_movemask_epi8(printable4) != 0 || _mm_movemask_epi8(printable5) != 0 ||
                _mm_movemask_epi8(printable6) != 0 || _mm_movemask_epi8(printable7) != 0) {
                return 0;
            }
            
            ptr += 128;
            nSize -= 128;
        }
        
        /* Process remaining 16-byte chunks */
        while (nSize >= 16) {
            __m128i chunk = _mm_loadu_si128((const __m128i*)ptr);
            __m128i ge_low = _mm_cmpgt_epi8(chunk, _mm_set1_epi8(0x1F));
            __m128i le_high = _mm_cmpgt_epi8(_mm_set1_epi8((char)0x80), chunk);
            __m128i printable = _mm_and_si128(ge_low, le_high);
            
            if (_mm_movemask_epi8(printable) != 0) {
                return 0;
            }
            
            ptr += 16;
            nSize -= 16;
        }
    }
#endif
    
    /* Fallback */
    const xsimd_uint8* pData = (const xsimd_uint8*)ptr;
    while (nSize > 0) {
        if (*pData >= 0x20 && *pData < 0x80) {
            return 0;
        }
        pData++;
        nSize--;
    }
    
    return 1;
}

int xsimd_compare_memory(const void* pBuffer1, const void* pBuffer2, xsimd_int64 nSize)
{
    const char* ptr1 = (const char*)pBuffer1;
    const char* ptr2 = (const char*)pBuffer2;
    
#ifdef XSIMD_X86
    /* AVX2: Process 32 bytes at a time */
    if ((g_nEnabledFeatures & XSIMD_FEATURE_AVX2) && nSize >= 32) {
        /* Process 128 bytes per iteration for better throughput */
        while (nSize >= 128) {
            __m256i chunk1_0 = _mm256_loadu_si256((const __m256i*)ptr1);
            __m256i chunk2_0 = _mm256_loadu_si256((const __m256i*)ptr2);
            __m256i chunk1_1 = _mm256_loadu_si256((const __m256i*)(ptr1 + 32));
            __m256i chunk2_1 = _mm256_loadu_si256((const __m256i*)(ptr2 + 32));
            __m256i chunk1_2 = _mm256_loadu_si256((const __m256i*)(ptr1 + 64));
            __m256i chunk2_2 = _mm256_loadu_si256((const __m256i*)(ptr2 + 64));
            __m256i chunk1_3 = _mm256_loadu_si256((const __m256i*)(ptr1 + 96));
            __m256i chunk2_3 = _mm256_loadu_si256((const __m256i*)(ptr2 + 96));
            
            __m256i cmp0 = _mm256_cmpeq_epi8(chunk1_0, chunk2_0);
            __m256i cmp1 = _mm256_cmpeq_epi8(chunk1_1, chunk2_1);
            __m256i cmp2 = _mm256_cmpeq_epi8(chunk1_2, chunk2_2);
            __m256i cmp3 = _mm256_cmpeq_epi8(chunk1_3, chunk2_3);
            
            if (_mm256_movemask_epi8(cmp0) != 0xFFFFFFFF ||
                _mm256_movemask_epi8(cmp1) != 0xFFFFFFFF ||
                _mm256_movemask_epi8(cmp2) != 0xFFFFFFFF ||
                _mm256_movemask_epi8(cmp3) != 0xFFFFFFFF) {
                return 0;
            }
            
            ptr1 += 128;
            ptr2 += 128;
            nSize -= 128;
        }
        
        /* Process remaining 32-byte chunks */
        while (nSize >= 32) {
            __m256i chunk1 = _mm256_loadu_si256((const __m256i*)ptr1);
            __m256i chunk2 = _mm256_loadu_si256((const __m256i*)ptr2);
            __m256i cmp = _mm256_cmpeq_epi8(chunk1, chunk2);
            
            if (_mm256_movemask_epi8(cmp) != 0xFFFFFFFF) {
                return 0;
            }
            
            ptr1 += 32;
            ptr2 += 32;
            nSize -= 32;
        }
    }
    /* AVX: Process 32 bytes at a time */
    else if ((g_nEnabledFeatures & XSIMD_FEATURE_AVX) && nSize >= 32) {
        while (nSize >= 32) {
            __m256i chunk1 = _mm256_loadu_si256((const __m256i*)ptr1);
            __m256i chunk2 = _mm256_loadu_si256((const __m256i*)ptr2);
            __m256i cmp = _mm256_cmpeq_epi8(chunk1, chunk2);
            
            if (_mm256_movemask_epi8(cmp) != 0xFFFFFFFF) {
                return 0;
            }
            
            ptr1 += 32;
            ptr2 += 32;
            nSize -= 32;
        }
    }
    /* SSE2: Process 16 bytes at a time */
    else if ((g_nEnabledFeatures & XSIMD_FEATURE_SSE2) && nSize >= 16) {
        /* Process 128 bytes per iteration for better throughput */
        while (nSize >= 128) {
            __m128i chunk1_0 = _mm_loadu_si128((const __m128i*)ptr1);
            __m128i chunk2_0 = _mm_loadu_si128((const __m128i*)ptr2);
            __m128i chunk1_1 = _mm_loadu_si128((const __m128i*)(ptr1 + 16));
            __m128i chunk2_1 = _mm_loadu_si128((const __m128i*)(ptr2 + 16));
            __m128i chunk1_2 = _mm_loadu_si128((const __m128i*)(ptr1 + 32));
            __m128i chunk2_2 = _mm_loadu_si128((const __m128i*)(ptr2 + 32));
            __m128i chunk1_3 = _mm_loadu_si128((const __m128i*)(ptr1 + 48));
            __m128i chunk2_3 = _mm_loadu_si128((const __m128i*)(ptr2 + 48));
            __m128i chunk1_4 = _mm_loadu_si128((const __m128i*)(ptr1 + 64));
            __m128i chunk2_4 = _mm_loadu_si128((const __m128i*)(ptr2 + 64));
            __m128i chunk1_5 = _mm_loadu_si128((const __m128i*)(ptr1 + 80));
            __m128i chunk2_5 = _mm_loadu_si128((const __m128i*)(ptr2 + 80));
            __m128i chunk1_6 = _mm_loadu_si128((const __m128i*)(ptr1 + 96));
            __m128i chunk2_6 = _mm_loadu_si128((const __m128i*)(ptr2 + 96));
            __m128i chunk1_7 = _mm_loadu_si128((const __m128i*)(ptr1 + 112));
            __m128i chunk2_7 = _mm_loadu_si128((const __m128i*)(ptr2 + 112));
            
            __m128i cmp0 = _mm_cmpeq_epi8(chunk1_0, chunk2_0);
            __m128i cmp1 = _mm_cmpeq_epi8(chunk1_1, chunk2_1);
            __m128i cmp2 = _mm_cmpeq_epi8(chunk1_2, chunk2_2);
            __m128i cmp3 = _mm_cmpeq_epi8(chunk1_3, chunk2_3);
            __m128i cmp4 = _mm_cmpeq_epi8(chunk1_4, chunk2_4);
            __m128i cmp5 = _mm_cmpeq_epi8(chunk1_5, chunk2_5);
            __m128i cmp6 = _mm_cmpeq_epi8(chunk1_6, chunk2_6);
            __m128i cmp7 = _mm_cmpeq_epi8(chunk1_7, chunk2_7);
            
            if (_mm_movemask_epi8(cmp0) != 0xFFFF ||
                _mm_movemask_epi8(cmp1) != 0xFFFF ||
                _mm_movemask_epi8(cmp2) != 0xFFFF ||
                _mm_movemask_epi8(cmp3) != 0xFFFF ||
                _mm_movemask_epi8(cmp4) != 0xFFFF ||
                _mm_movemask_epi8(cmp5) != 0xFFFF ||
                _mm_movemask_epi8(cmp6) != 0xFFFF ||
                _mm_movemask_epi8(cmp7) != 0xFFFF) {
                return 0;
            }
            
            ptr1 += 128;
            ptr2 += 128;
            nSize -= 128;
        }
        
        /* Process remaining 16-byte chunks */
        while (nSize >= 16) {
            __m128i chunk1 = _mm_loadu_si128((const __m128i*)ptr1);
            __m128i chunk2 = _mm_loadu_si128((const __m128i*)ptr2);
            __m128i cmp = _mm_cmpeq_epi8(chunk1, chunk2);
            
            if (_mm_movemask_epi8(cmp) != 0xFFFF) {
                return 0;
            }
            
            ptr1 += 16;
            ptr2 += 16;
            nSize -= 16;
        }
    }
#endif
    
    /* Fallback: use memcmp */
    return (memcmp(ptr1, ptr2, (size_t)nSize) == 0) ? 1 : 0;
}

xsimd_int64 xsimd_find_not_ansi_and_null(const void* pBuffer, xsimd_int64 nBufferSize, 
                                         xsimd_int64 nMinLength, xsimd_int64 nOffset)
{
    if (!pBuffer || nBufferSize <= 0 || nMinLength <= 0) {
        return -1;
    }
    
    const unsigned char* hay = (const unsigned char*)pBuffer;
    xsimd_int64 hayLen = nBufferSize;
    xsimd_int64 m = nMinLength;
    const xsimd_int64 limit = hayLen - (m - 1);
    xsimd_int64 j = 0;
    
    /* ANSI table for quick lookups */
    int ansiTable[256];
    int i;
    for (i = 0; i < 256; i++) {
        ansiTable[i] = (i >= 0x20 && i <= 0x7E) ? 1 : 0;
    }
    
#ifdef XSIMD_X86
    /* AVX2-optimized search for NOT-ANSI-AND-NULL (32 bytes at a time) */
    if ((g_nEnabledFeatures & XSIMD_FEATURE_AVX2) && hayLen >= 32) {
        const xsimd_int64 vectorSize = 32;
        __m256i vMin = _mm256_set1_epi8(0x20);   /* Min printable ASCII */
        __m256i vMax = _mm256_set1_epi8(0x7E);   /* Max printable ASCII */
        __m256i vZero = _mm256_setzero_si256();  /* For null check */
        
        while (j + vectorSize <= hayLen) {
            __m256i vData = _mm256_loadu_si256((const __m256i*)(hay + j));
            
            /* Check for bytes < 0x20 (but not 0x00) */
            __m256i vLt = _mm256_cmpgt_epi8(vMin, vData);        /* vData < 0x20 */
            __m256i vNotNull = _mm256_cmpeq_epi8(vData, vZero);  /* vData == 0x00 */
            vLt = _mm256_andnot_si256(vNotNull, vLt);            /* vData < 0x20 AND vData != 0x00 */
            
            /* Check for bytes > 0x7E */
            __m256i vGt = _mm256_cmpgt_epi8(vData, vMax);  /* vData > 0x7E */
            
            /* Combine: NOT-ANSI-AND-NULL = (< 0x20 AND != 0x00) OR (> 0x7E) */
            __m256i vNotAnsiAndNull = _mm256_or_si256(vLt, vGt);
            
            xsimd_uint32 mask = _mm256_movemask_epi8(vNotAnsiAndNull);
            
            if (mask != 0) {
                /* Found some NOT-ANSI-AND-NULL bytes, check for runs of length m */
                xsimd_int32 k;
                for (k = 0; k < vectorSize && j + k < limit; k++) {
                    if (mask & (1U << k)) {
                        /* Potential start of a run */
                        xsimd_int64 start = j + k;
                        xsimd_int64 runLen = 0;
                        xsimd_int64 r;
                        for (r = start; r < hayLen; r++) {
                            unsigned char c = hay[r];
                            if ((ansiTable[c]) || (c == 0)) break;
                            runLen++;
                            if (runLen >= m) {
                                return nOffset + start;
                            }
                        }
                    }
                }
            }
            j += vectorSize;
        }
    }
    
    /* AVX-optimized search for NOT-ANSI-AND-NULL (32 bytes at a time) */
    if ((g_nEnabledFeatures & XSIMD_FEATURE_AVX) && hayLen >= 32) {
        const xsimd_int64 vectorSize = 32;
        __m256i vMin = _mm256_set1_epi8(0x20);   /* Min printable ASCII */
        __m256i vMax = _mm256_set1_epi8(0x7E);   /* Max printable ASCII */
        __m256i vZero = _mm256_setzero_si256();  /* For null check */
        
        while (j + vectorSize <= hayLen) {
            __m256i vData = _mm256_loadu_si256((const __m256i*)(hay + j));
            
            /* Check for bytes < 0x20 (but not 0x00) using AVX */
            __m256i vLt = _mm256_cmpgt_epi8(vMin, vData);        /* vData < 0x20 */
            __m256i vNotNull = _mm256_cmpeq_epi8(vData, vZero);  /* vData == 0x00 */
            vLt = _mm256_andnot_si256(vNotNull, vLt);            /* vData < 0x20 AND vData != 0x00 */
            
            /* Check for bytes > 0x7E */
            __m256i vGt = _mm256_cmpgt_epi8(vData, vMax);  /* vData > 0x7E */
            
            /* Combine: NOT-ANSI-AND-NULL = (< 0x20 AND != 0x00) OR (> 0x7E) */
            __m256i vNotAnsiAndNull = _mm256_or_si256(vLt, vGt);
            
            xsimd_uint32 mask = _mm256_movemask_epi8(vNotAnsiAndNull);
            
            if (mask != 0) {
                /* Found some NOT-ANSI-AND-NULL bytes, check for runs of length m */
                xsimd_int32 k;
                for (k = 0; k < vectorSize && j + k < limit; k++) {
                    if (mask & (1U << k)) {
                        /* Potential start of a run */
                        xsimd_int64 start = j + k;
                        xsimd_int64 runLen = 0;
                        xsimd_int64 r;
                        for (r = start; r < hayLen; r++) {
                            unsigned char c = hay[r];
                            if ((ansiTable[c]) || (c == 0)) break;
                            runLen++;
                            if (runLen >= m) {
                                return nOffset + start;
                            }
                        }
                    }
                }
            }
            j += vectorSize;
        }
    }
    
    /* SSE2-optimized search for NOT-ANSI-AND-NULL (16 bytes at a time) */
    if ((g_nEnabledFeatures & XSIMD_FEATURE_SSE2) && hayLen >= 16) {
        const xsimd_int64 vectorSize = 16;
        __m128i vMin = _mm_set1_epi8(0x20);   /* Min printable ASCII */
        __m128i vMax = _mm_set1_epi8(0x7E);   /* Max printable ASCII */
        __m128i vZero = _mm_setzero_si128();  /* For null check */
        
        while (j + vectorSize <= hayLen) {
            __m128i vData = _mm_loadu_si128((const __m128i*)(hay + j));
            
            /* Check for bytes < 0x20 (but not 0x00) */
            __m128i vLt = _mm_cmpgt_epi8(vMin, vData);        /* vData < 0x20 */
            __m128i vNotNull = _mm_cmpeq_epi8(vData, vZero);  /* vData == 0x00 */
            vLt = _mm_andnot_si128(vNotNull, vLt);            /* vData < 0x20 AND vData != 0x00 */
            
            /* Check for bytes > 0x7E */
            __m128i vGt = _mm_cmpgt_epi8(vData, vMax);  /* vData > 0x7E */
            
            /* Combine: NOT-ANSI-AND-NULL = (< 0x20 AND != 0x00) OR (> 0x7E) */
            __m128i vNotAnsiAndNull = _mm_or_si128(vLt, vGt);
            
            xsimd_uint32 mask = _mm_movemask_epi8(vNotAnsiAndNull);
            
            if (mask != 0) {
                /* Found some NOT-ANSI-AND-NULL bytes, check for runs of length m */
                xsimd_int32 k;
                for (k = 0; k < vectorSize && j + k < limit; k++) {
                    if (mask & (1U << k)) {
                        /* Potential start of a run */
                        xsimd_int64 start = j + k;
                        xsimd_int64 runLen = 0;
                        xsimd_int64 r;
                        for (r = start; r < hayLen; r++) {
                            unsigned char c = hay[r];
                            if ((ansiTable[c]) || (c == 0)) break;
                            runLen++;
                            if (runLen >= m) {
                                return nOffset + start;
                            }
                        }
                    }
                }
            }
            j += vectorSize;
        }
    }
#endif
    
    /* Fallback scalar loop */
    while (j < limit) {
        /* Skip bytes that are ANSI or zero to the start of a desired run */
        while (j < hayLen) {
            unsigned char c = hay[j];
            if ((!ansiTable[c]) && (c != 0)) break;
            j++;
        }
        if (j >= limit) break;
        xsimd_int64 start = j;
        /* Extend run of non-ANSI and non-zero bytes */
        while (j < hayLen) {
            unsigned char c = hay[j];
            if ((ansiTable[c]) || (c == 0)) break;
            j++;
        }
        xsimd_int64 runLen = j - start;
        if (runLen >= m) {
            return nOffset + start;
        }
    }
    
    return -1;
}

int xsimd_is_not_ansi_and_null(const void* pBuffer, xsimd_int64 nSize)
{
    if (!pBuffer || nSize <= 0) {
        return 0;
    }
    
    const char* ptr = (const char*)pBuffer;
    
#ifdef XSIMD_X86
    /* AVX2 optimization: process 32 bytes at a time */
    if ((g_nEnabledFeatures & XSIMD_FEATURE_AVX2) && nSize >= 32) {
        const __m256i zero = _mm256_setzero_si256();
        
        while (nSize >= 32) {
            __m256i chunk = _mm256_loadu_si256((const __m256i*)ptr);
            
            /* Check for zero bytes */
            __m256i is_zero = _mm256_cmpeq_epi8(chunk, zero);
            
            /* Check for printable ASCII (0x20-0x7E) */
            __m256i ge_low = _mm256_cmpgt_epi8(chunk, _mm256_set1_epi8(0x1F));
            __m256i le_high = _mm256_cmpgt_epi8(_mm256_set1_epi8((char)0x80), chunk);
            __m256i is_printable = _mm256_and_si256(ge_low, le_high);
            
            /* Invalid if zero OR printable */
            __m256i invalid = _mm256_or_si256(is_zero, is_printable);
            
            if (_mm256_movemask_epi8(invalid) != 0) {
                return 0;
            }
            
            ptr += 32;
            nSize -= 32;
        }
    }
    
    /* AVX optimization: process 32 bytes at a time */
    if ((g_nEnabledFeatures & XSIMD_FEATURE_AVX) && nSize >= 32) {
        const __m256i zero = _mm256_setzero_si256();
        
        while (nSize >= 32) {
            __m256i chunk = _mm256_loadu_si256((const __m256i*)ptr);
            
            /* Check for zero bytes */
            __m256i is_zero = _mm256_cmpeq_epi8(chunk, zero);
            
            /* Check for printable ASCII (0x20-0x7E) */
            __m256i ge_low = _mm256_cmpgt_epi8(chunk, _mm256_set1_epi8(0x1F));
            __m256i le_high = _mm256_cmpgt_epi8(_mm256_set1_epi8((char)0x80), chunk);
            __m256i is_printable = _mm256_and_si256(ge_low, le_high);
            
            /* Invalid if zero OR printable */
            __m256i invalid = _mm256_or_si256(is_zero, is_printable);
            
            if (_mm256_movemask_epi8(invalid) != 0) {
                return 0;
            }
            
            ptr += 32;
            nSize -= 32;
        }
    }
    
    /* SSE2 optimization: process 16 bytes at a time */
    if ((g_nEnabledFeatures & XSIMD_FEATURE_SSE2) && nSize >= 16) {
        const __m128i zero = _mm_setzero_si128();
        
        while (nSize >= 16) {
            __m128i chunk = _mm_loadu_si128((const __m128i*)ptr);
            
            /* Check for zero bytes */
            __m128i is_zero = _mm_cmpeq_epi8(chunk, zero);
            
            /* Check for printable ASCII */
            __m128i ge_low = _mm_cmpgt_epi8(chunk, _mm_set1_epi8(0x1F));
            __m128i le_high = _mm_cmpgt_epi8(_mm_set1_epi8((char)0x80), chunk);
            __m128i is_printable = _mm_and_si128(ge_low, le_high);
            
            __m128i invalid = _mm_or_si128(is_zero, is_printable);
            
            if (_mm_movemask_epi8(invalid) != 0) {
                return 0;
            }
            
            ptr += 16;
            nSize -= 16;
        }
    }
#endif
    
    /* Process remaining bytes */
    const unsigned char* pRemaining = (const unsigned char*)ptr;
    while (nSize > 0) {
        if (*pRemaining == 0 || (*pRemaining >= 0x20 && *pRemaining < 0x80)) {
            return 0;
        }
        pRemaining++;
        nSize--;
    }
    
    return 1;
}

xsimd_int64 xsimd_find_ansi_number(const void* pBuffer, xsimd_int64 nBufferSize, 
                                   xsimd_int64 nMinLength, xsimd_int64 nOffset)
{
    if (!pBuffer || nBufferSize <= 0 || nMinLength <= 0) {
        return -1;
    }
    
    const unsigned char* hay = (const unsigned char*)pBuffer;
    xsimd_int64 hayLen = nBufferSize;
    xsimd_int64 m = nMinLength;
    const xsimd_int64 limit = hayLen - (m - 1);
    xsimd_int64 j = 0;
    
#ifdef XSIMD_X86
    if ((g_nEnabledFeatures & XSIMD_FEATURE_AVX2) && hayLen >= 32) {
        const xsimd_int64 nVectorSize = 32;  /* AVX2 processes 32 bytes at once */
        
        /* AVX2 constants for digit range checking (0x30-0x39) */
        __m256i vMin = _mm256_set1_epi8(0x30);  /* '0' */
        __m256i vMax = _mm256_set1_epi8(0x39);  /* '9' */
        __m256i vOne = _mm256_set1_epi8(1);
        
        while (j < limit) {
            /* AVX2 optimized search for digit bytes */
            int bFoundDigit = 0;
            
            /* Process 32 bytes at a time with AVX2 */
            for (; j + nVectorSize <= hayLen; j += nVectorSize) {
                __m256i vData = _mm256_loadu_si256((const __m256i*)(hay + j));
                
                /* Check for bytes >= 0x30 */
                __m256i vGe = _mm256_cmpgt_epi8(vData, _mm256_sub_epi8(vMin, vOne));
                
                /* Check for bytes <= 0x39 */
                __m256i vLe = _mm256_cmpgt_epi8(_mm256_add_epi8(vMax, vOne), vData);
                
                /* Combine: digit = (>= 0x30 AND <= 0x39) */
                __m256i vDigit = _mm256_and_si256(vGe, vLe);
                
                xsimd_uint32 nMask = _mm256_movemask_epi8(vDigit);
                
                if (nMask != 0) {
                    /* Found some digit bytes, scan for start of run */
                    xsimd_int32 i;
                    for (i = 0; i < nVectorSize && j + i < hayLen; i++) {
                        if (nMask & (1U << i)) {
                            j += i;
                            bFoundDigit = 1;
                            break;
                        }
                    }
                    if (bFoundDigit) break;
                }
            }
            
            /* Fallback scalar search for remaining bytes */
            if (!bFoundDigit) {
                while (j < hayLen) {
                    unsigned char c = hay[j];
                    if ((c >= 0x30) && (c <= 0x39)) {
                        bFoundDigit = 1;
                        break;
                    }
                    j++;
                }
            }
            
            if (!bFoundDigit || j >= limit) break;
            
            xsimd_int64 start = j;
            
            /* Extend run of digit bytes with scalar code */
            while (j < hayLen) {
                unsigned char c = hay[j];
                if ((c < 0x30) || (c > 0x39)) break;
                j++;
            }
            
            xsimd_int64 runLen = j - start;
            if (runLen >= m) {
                return nOffset + start;
            }
        }
        
        return -1;
    }
    
    if ((g_nEnabledFeatures & XSIMD_FEATURE_AVX) && hayLen >= 32) {
        const xsimd_int64 nVectorSize = 32;  /* AVX processes 32 bytes at once */
        
        /* AVX constants for digit range checking (0x30-0x39) */
        __m256i vMin = _mm256_set1_epi8(0x30);  /* '0' */
        __m256i vMax = _mm256_set1_epi8(0x39);  /* '9' */
        __m256i vOne = _mm256_set1_epi8(1);
        
        while (j < limit) {
            /* AVX optimized search for digit bytes */
            int bFoundDigit = 0;
            
            /* Process 32 bytes at a time with AVX */
            for (; j + nVectorSize <= hayLen; j += nVectorSize) {
                __m256i vData = _mm256_loadu_si256((const __m256i*)(hay + j));
                
                /* Check for bytes >= 0x30 */
                __m256i vGe = _mm256_cmpgt_epi8(vData, _mm256_sub_epi8(vMin, vOne));
                
                /* Check for bytes <= 0x39 */
                __m256i vLe = _mm256_cmpgt_epi8(_mm256_add_epi8(vMax, vOne), vData);
                
                /* Combine: digit = (>= 0x30 AND <= 0x39) */
                __m256i vDigit = _mm256_and_si256(vGe, vLe);
                
                xsimd_uint32 nMask = _mm256_movemask_epi8(vDigit);
                
                if (nMask != 0) {
                    /* Found some digit bytes, scan for start of run */
                    xsimd_int32 i;
                    for (i = 0; i < nVectorSize && j + i < hayLen; i++) {
                        if (nMask & (1U << i)) {
                            j += i;
                            bFoundDigit = 1;
                            break;
                        }
                    }
                    if (bFoundDigit) break;
                }
            }
            
            /* Fallback scalar search for remaining bytes */
            if (!bFoundDigit) {
                while (j < hayLen) {
                    unsigned char c = hay[j];
                    if ((c >= 0x30) && (c <= 0x39)) {
                        bFoundDigit = 1;
                        break;
                    }
                    j++;
                }
            }
            
            if (!bFoundDigit || j >= limit) break;
            
            xsimd_int64 start = j;
            
            /* Extend run of digit bytes with scalar code */
            while (j < hayLen) {
                unsigned char c = hay[j];
                if ((c < 0x30) || (c > 0x39)) break;
                j++;
            }
            
            xsimd_int64 runLen = j - start;
            if (runLen >= m) {
                return nOffset + start;
            }
        }
        
        return -1;
    }
    
    if ((g_nEnabledFeatures & XSIMD_FEATURE_SSE2) && hayLen >= 16) {
        const xsimd_int64 nVectorSize = 16;  /* SSE2 processes 16 bytes at once */
        
        /* SSE2 constants for digit range checking (0x30-0x39) */
        __m128i vMin = _mm_set1_epi8(0x30);  /* '0' */
        __m128i vMax = _mm_set1_epi8(0x39);  /* '9' */
        __m128i vOne = _mm_set1_epi8(1);
        
        while (j < limit) {
            /* SSE2 optimized search for digit bytes */
            int bFoundDigit = 0;
            
            /* Process 16 bytes at a time with SSE2 */
            for (; j + nVectorSize <= hayLen; j += nVectorSize) {
                __m128i vData = _mm_loadu_si128((const __m128i*)(hay + j));
                
                /* Check for bytes >= 0x30 */
                __m128i vGe = _mm_cmpgt_epi8(vData, _mm_sub_epi8(vMin, vOne));
                
                /* Check for bytes <= 0x39 */
                __m128i vLe = _mm_cmpgt_epi8(_mm_add_epi8(vMax, vOne), vData);
                
                /* Combine: digit = (>= 0x30 AND <= 0x39) */
                __m128i vDigit = _mm_and_si128(vGe, vLe);
                
                xsimd_uint32 nMask = _mm_movemask_epi8(vDigit);
                
                if (nMask != 0) {
                    /* Found some digit bytes, scan for start of run */
                    xsimd_int32 i;
                    for (i = 0; i < nVectorSize && j + i < hayLen; i++) {
                        if (nMask & (1U << i)) {
                            j += i;
                            bFoundDigit = 1;
                            break;
                        }
                    }
                    if (bFoundDigit) break;
                }
            }
            
            /* Fallback scalar search for remaining bytes */
            if (!bFoundDigit) {
                while (j < hayLen) {
                    unsigned char c = hay[j];
                    if ((c >= 0x30) && (c <= 0x39)) {
                        bFoundDigit = 1;
                        break;
                    }
                    j++;
                }
            }
            
            if (!bFoundDigit || j >= limit) break;
            
            xsimd_int64 start = j;
            
            /* Extend run of digit bytes with scalar code */
            while (j < hayLen) {
                unsigned char c = hay[j];
                if ((c < 0x30) || (c > 0x39)) break;
                j++;
            }
            
            xsimd_int64 runLen = j - start;
            if (runLen >= m) {
                return nOffset + start;
            }
        }
        
        return -1;
    }
#endif
    
    /* Fallback scalar implementation */
    while (j < limit) {
        /* Skip bytes that are not digits to the start of a desired run */
        while (j < hayLen) {
            unsigned char c = hay[j];
            if ((c >= 0x30) && (c <= 0x39)) break;
            j++;
        }
        if (j >= limit) break;
        xsimd_int64 start = j;
        /* Extend run of digit bytes */
        while (j < hayLen) {
            unsigned char c = hay[j];
            if ((c < 0x30) || (c > 0x39)) break;
            j++;
        }
        xsimd_int64 runLen = j - start;
        if (runLen >= m) {
            return nOffset + start;
        }
    }
    
    return -1;
}

int xsimd_is_ansi_number(const void* pBuffer, xsimd_int64 nSize)
{
    if (!pBuffer || nSize <= 0) {
        return 0;
    }
    
    const char* ptr = (const char*)pBuffer;
    
#ifdef XSIMD_X86
    /* AVX2 optimization: process 32 bytes at a time */
    if ((g_nEnabledFeatures & XSIMD_FEATURE_AVX2) && nSize >= 32) {
        const __m256i digit_low = _mm256_set1_epi8(0x30);   /* '0' */
        const __m256i digit_high = _mm256_set1_epi8(0x39);  /* '9' */
        
        while (nSize >= 32) {
            __m256i chunk = _mm256_loadu_si256((const __m256i*)ptr);
            
            /* Check if any byte < '0' or > '9' */
            __m256i too_low = _mm256_cmpgt_epi8(digit_low, chunk);
            __m256i too_high = _mm256_cmpgt_epi8(chunk, digit_high);
            __m256i invalid = _mm256_or_si256(too_low, too_high);
            
            if (_mm256_movemask_epi8(invalid) != 0) {
                return 0;
            }
            
            ptr += 32;
            nSize -= 32;
        }
    }
    
    /* AVX optimization: process 32 bytes at a time */
    if ((g_nEnabledFeatures & XSIMD_FEATURE_AVX) && nSize >= 32) {
        const __m256i digit_low = _mm256_set1_epi8(0x30);   /* '0' */
        const __m256i digit_high = _mm256_set1_epi8(0x39);  /* '9' */
        
        while (nSize >= 32) {
            __m256i chunk = _mm256_loadu_si256((const __m256i*)ptr);
            
            /* Check if any byte < '0' or > '9' */
            __m256i too_low = _mm256_cmpgt_epi8(digit_low, chunk);
            __m256i too_high = _mm256_cmpgt_epi8(chunk, digit_high);
            __m256i invalid = _mm256_or_si256(too_low, too_high);
            
            if (_mm256_movemask_epi8(invalid) != 0) {
                return 0;
            }
            
            ptr += 32;
            nSize -= 32;
        }
    }
    
    /* SSE2 optimization: process 16 bytes at a time */
    if ((g_nEnabledFeatures & XSIMD_FEATURE_SSE2) && nSize >= 16) {
        const __m128i digit_low = _mm_set1_epi8(0x30);   /* '0' */
        const __m128i digit_high = _mm_set1_epi8(0x39);  /* '9' */
        
        while (nSize >= 16) {
            __m128i chunk = _mm_loadu_si128((const __m128i*)ptr);
            
            /* Check if any byte < '0' or > '9' */
            __m128i too_low = _mm_cmplt_epi8(chunk, digit_low);
            __m128i too_high = _mm_cmpgt_epi8(chunk, digit_high);
            __m128i invalid = _mm_or_si128(too_low, too_high);
            
            if (_mm_movemask_epi8(invalid) != 0) {
                return 0;
            }
            
            ptr += 16;
            nSize -= 16;
        }
    }
#endif
    
    /* Process remaining bytes */
    const unsigned char* pRemaining = (const unsigned char*)ptr;
    while (nSize > 0) {
        if (*pRemaining < 0x30 || *pRemaining > 0x39) {
            return 0;
        }
        pRemaining++;
        nSize--;
    }
    
    return 1;
}

xsimd_int64 xsimd_find_ansi_string_i(const void* pBuffer, xsimd_int64 nBufferSize,
                                     const void* pString, xsimd_int64 nStringSize,
                                     xsimd_int64 nOffset)
{
    if (!pBuffer || !pString || nBufferSize <= 0 || nStringSize <= 0 || nStringSize > nBufferSize) {
        return -1;
    }
    
    /* Prepare upper and lower case versions of search string */
    unsigned char* pUpperData = (unsigned char*)malloc((size_t)nStringSize);
    unsigned char* pLowerData = (unsigned char*)malloc((size_t)nStringSize);
    if (!pUpperData || !pLowerData) {
        if (pUpperData) free(pUpperData);
        if (pLowerData) free(pLowerData);
        return -1;
    }
    
    const unsigned char* pStr = (const unsigned char*)pString;
    xsimd_int64 i;
    for (i = 0; i < nStringSize; i++) {
        unsigned char c = pStr[i];
        pUpperData[i] = (c >= 'a' && c <= 'z') ? (c - 32) : c;
        pLowerData[i] = (c >= 'A' && c <= 'Z') ? (c + 32) : c;
    }
    
    const unsigned char* pData = (const unsigned char*)pBuffer;
    xsimd_int64 nSearchEnd = nBufferSize - (nStringSize - 1);
    
#ifdef XSIMD_X86
    /* AVX2 optimized scan for first character (32 bytes at a time), then verify full match */
    if ((g_nEnabledFeatures & XSIMD_FEATURE_AVX2) && nSearchEnd >= 32) {
        const xsimd_int64 nVectorSize = 32;
        __m256i vFirstUpper = _mm256_set1_epi8(pUpperData[0]);
        __m256i vFirstLower = _mm256_set1_epi8(pLowerData[0]);
        
        for (i = 0; i < nSearchEnd;) {
            /* Quick SIMD scan for first character match */
            int bFoundCandidate = 0;
            xsimd_int64 nCandidatePos = i;
            
            /* Process vectors until we find a candidate or reach the end */
            for (; i + nVectorSize <= nSearchEnd; i += nVectorSize) {
                __m256i vData = _mm256_loadu_si256((const __m256i*)(pData + i));
                __m256i vCmpUpper = _mm256_cmpeq_epi8(vData, vFirstUpper);
                __m256i vCmpLower = _mm256_cmpeq_epi8(vData, vFirstLower);
                __m256i vMatch = _mm256_or_si256(vCmpUpper, vCmpLower);
                
                xsimd_uint32 nMask = _mm256_movemask_epi8(vMatch);
                
                if (nMask != 0) {
                    /* Find first set bit */
                    xsimd_int32 j = 0;
                    while (j < nVectorSize && !(nMask & (1U << j))) j++;
                    
                    if (i + j < nSearchEnd) {
                        nCandidatePos = i + j;
                        bFoundCandidate = 1;
                        break;
                    }
                }
            }
            
            /* Check remaining bytes with scalar */
            if (!bFoundCandidate) {
                for (; i < nSearchEnd; i++) {
                    if (pData[i] == pUpperData[0] || pData[i] == pLowerData[0]) {
                        nCandidatePos = i;
                        bFoundCandidate = 1;
                        break;
                    }
                }
            }
            
            if (!bFoundCandidate) break;
            
            /* Verify full string match at candidate position */
            int bFullMatch = 1;
            const unsigned char* pCurrent = pData + nCandidatePos;
            const unsigned char* pUpper = pUpperData;
            const unsigned char* pLower = pLowerData;
            xsimd_int64 nRemaining = nStringSize;
            
            while (nRemaining > 0) {
                if ((*pCurrent != *pUpper) && (*pCurrent != *pLower)) {
                    bFullMatch = 0;
                    break;
                }
                pCurrent++;
                pUpper++;
                pLower++;
                nRemaining--;
            }
            
            if (bFullMatch) {
                free(pUpperData);
                free(pLowerData);
                return nOffset + nCandidatePos;
            }
            
            i = nCandidatePos + 1;
        }
        
        free(pUpperData);
        free(pLowerData);
        return -1;
    }
    
    /* AVX optimized scan for first character (32 bytes at a time), then verify full match */
    if ((g_nEnabledFeatures & XSIMD_FEATURE_AVX) && nSearchEnd >= 32) {
        const xsimd_int64 nVectorSize = 32;
        __m256i vFirstUpper = _mm256_set1_epi8(pUpperData[0]);
        __m256i vFirstLower = _mm256_set1_epi8(pLowerData[0]);
        
        for (i = 0; i < nSearchEnd;) {
            /* Quick SIMD scan for first character match */
            int bFoundCandidate = 0;
            xsimd_int64 nCandidatePos = i;
            
            /* Process vectors until we find a candidate or reach the end */
            for (; i + nVectorSize <= nSearchEnd; i += nVectorSize) {
                __m256i vData = _mm256_loadu_si256((const __m256i*)(pData + i));
                __m256i vCmpUpper = _mm256_cmpeq_epi8(vData, vFirstUpper);
                __m256i vCmpLower = _mm256_cmpeq_epi8(vData, vFirstLower);
                __m256i vMatch = _mm256_or_si256(vCmpUpper, vCmpLower);
                
                xsimd_uint32 nMask = _mm256_movemask_epi8(vMatch);
                
                if (nMask != 0) {
                    /* Find first set bit */
                    xsimd_int32 j = 0;
                    while (j < nVectorSize && !(nMask & (1U << j))) j++;
                    
                    if (i + j < nSearchEnd) {
                        nCandidatePos = i + j;
                        bFoundCandidate = 1;
                        break;
                    }
                }
            }
            
            /* Check remaining bytes with scalar */
            if (!bFoundCandidate) {
                for (; i < nSearchEnd; i++) {
                    if (pData[i] == pUpperData[0] || pData[i] == pLowerData[0]) {
                        nCandidatePos = i;
                        bFoundCandidate = 1;
                        break;
                    }
                }
            }
            
            if (!bFoundCandidate) break;
            
            /* Verify full string match at candidate position */
            int bFullMatch = 1;
            const unsigned char* pCurrent = pData + nCandidatePos;
            const unsigned char* pUpper = pUpperData;
            const unsigned char* pLower = pLowerData;
            xsimd_int64 nRemaining = nStringSize;
            
            while (nRemaining > 0) {
                if ((*pCurrent != *pUpper) && (*pCurrent != *pLower)) {
                    bFullMatch = 0;
                    break;
                }
                pCurrent++;
                pUpper++;
                pLower++;
                nRemaining--;
            }
            
            if (bFullMatch) {
                free(pUpperData);
                free(pLowerData);
                return nOffset + nCandidatePos;
            }
            
            i = nCandidatePos + 1;
        }
        
        free(pUpperData);
        free(pLowerData);
        return -1;
    }
    
    /* SSE2 optimized scan for first character (16 bytes at a time), then verify full match */
    if ((g_nEnabledFeatures & XSIMD_FEATURE_SSE2) && nSearchEnd >= 16) {
        const xsimd_int64 nVectorSize = 16;
        __m128i vFirstUpper = _mm_set1_epi8(pUpperData[0]);
        __m128i vFirstLower = _mm_set1_epi8(pLowerData[0]);
        
        for (i = 0; i < nSearchEnd;) {
            /* Quick SIMD scan for first character match */
            int bFoundCandidate = 0;
            xsimd_int64 nCandidatePos = i;
            
            /* Process vectors until we find a candidate or reach the end */
            for (; i + nVectorSize <= nSearchEnd; i += nVectorSize) {
                __m128i vData = _mm_loadu_si128((const __m128i*)(pData + i));
                __m128i vCmpUpper = _mm_cmpeq_epi8(vData, vFirstUpper);
                __m128i vCmpLower = _mm_cmpeq_epi8(vData, vFirstLower);
                __m128i vMatch = _mm_or_si128(vCmpUpper, vCmpLower);
                
                xsimd_uint32 nMask = _mm_movemask_epi8(vMatch);
                
                if (nMask != 0) {
                    /* Find first set bit */
                    xsimd_int32 j = 0;
                    while (j < nVectorSize && !(nMask & (1U << j))) j++;
                    
                    if (i + j < nSearchEnd) {
                        nCandidatePos = i + j;
                        bFoundCandidate = 1;
                        break;
                    }
                }
            }
            
            /* Check remaining bytes with scalar */
            if (!bFoundCandidate) {
                for (; i < nSearchEnd; i++) {
                    if (pData[i] == pUpperData[0] || pData[i] == pLowerData[0]) {
                        nCandidatePos = i;
                        bFoundCandidate = 1;
                        break;
                    }
                }
            }
            
            if (!bFoundCandidate) break;
            
            /* Verify full string match at candidate position */
            int bFullMatch = 1;
            const unsigned char* pCurrent = pData + nCandidatePos;
            const unsigned char* pUpper = pUpperData;
            const unsigned char* pLower = pLowerData;
            xsimd_int64 nRemaining = nStringSize;
            
            while (nRemaining > 0) {
                if ((*pCurrent != *pUpper) && (*pCurrent != *pLower)) {
                    bFullMatch = 0;
                    break;
                }
                pCurrent++;
                pUpper++;
                pLower++;
                nRemaining--;
            }
            
            if (bFullMatch) {
                free(pUpperData);
                free(pLowerData);
                return nOffset + nCandidatePos;
            }
            
            i = nCandidatePos + 1;
        }
        
        free(pUpperData);
        free(pLowerData);
        return -1;
    }
#endif
    
    /* Fallback scalar implementation */
    for (i = 0; i < nSearchEnd; i++) {
        int bMatch = 1;
        xsimd_int64 j;
        for (j = 0; j < nStringSize; j++) {
            if ((pData[i + j] != pUpperData[j]) && (pData[i + j] != pLowerData[j])) {
                bMatch = 0;
                break;
            }
        }
        if (bMatch) {
            free(pUpperData);
            free(pLowerData);
            return nOffset + i;
        }
    }
    
    free(pUpperData);
    free(pLowerData);
    return -1;
}

xsimd_int64 xsimd_find_first_non_ansi(const void* pBuffer, xsimd_int64 nSize)
{
    const xsimd_uint8* pData = (const xsimd_uint8*)pBuffer;
    xsimd_int64 i = 0;
    
    if (!g_bInitialized) {
        xsimd_init();
    }
    
#ifdef XSIMD_X86
    if (g_nEnabledFeatures & XSIMD_FEATURE_AVX2) {
        __m256i vMin = _mm256_set1_epi8(0x20);  /* Minimum ANSI (space) */
        __m256i vMax = _mm256_set1_epi8(0x7F);  /* Maximum ANSI (DEL-1) */
        
        /* Process 32 bytes at a time */
        for (; i + 32 <= nSize; i += 32) {
            __m256i vData = _mm256_loadu_si256((const __m256i*)(pData + i));
            
            /* Check if any byte < 0x20 OR >= 0x80 */
            __m256i vLessThanMin = _mm256_cmpgt_epi8(vMin, vData);
            __m256i vGreaterThanMax = _mm256_cmpgt_epi8(vData, vMax);
            __m256i vNonAnsi = _mm256_or_si256(vLessThanMin, vGreaterThanMax);
            
            int nMask = _mm256_movemask_epi8(vNonAnsi);
            
            if (nMask != 0) {
                /* Find first set bit */
                int nPos = 0;
                while (((nMask >> nPos) & 1) == 0) nPos++;
                return i + nPos;
            }
        }
    } else if (g_nEnabledFeatures & XSIMD_FEATURE_SSE2) {
        __m128i vMin = _mm_set1_epi8(0x20);
        __m128i vMax = _mm_set1_epi8(0x7F);
        
        /* Process 16 bytes at a time */
        for (; i + 16 <= nSize; i += 16) {
            __m128i vData = _mm_loadu_si128((const __m128i*)(pData + i));
            
            __m128i vLessThanMin = _mm_cmplt_epi8(vData, vMin);
            __m128i vGreaterThanMax = _mm_cmpgt_epi8(vData, vMax);
            __m128i vNonAnsi = _mm_or_si128(vLessThanMin, vGreaterThanMax);
            
            int nMask = _mm_movemask_epi8(vNonAnsi);
            
            if (nMask != 0) {
                int nPos = 0;
                while (((nMask >> nPos) & 1) == 0) nPos++;
                return i + nPos;
            }
        }
    }
#endif
    
    /* Scalar fallback for remaining bytes */
    for (; i < nSize; i++) {
        xsimd_uint8 nByte = pData[i];
        if (nByte < 0x20 || nByte >= 0x80) {
            return i;
        }
    }
    
    return -1;
}

xsimd_int64 xsimd_find_null_byte(const void* pBuffer, xsimd_int64 nSize)
{
    const xsimd_uint8* pData = (const xsimd_uint8*)pBuffer;
    xsimd_int64 i = 0;
    
    if (!g_bInitialized) {
        xsimd_init();
    }
    
#ifdef XSIMD_X86
    if (g_nEnabledFeatures & XSIMD_FEATURE_AVX2) {
        __m256i vZero = _mm256_setzero_si256();
        
        for (; i + 32 <= nSize; i += 32) {
            __m256i vData = _mm256_loadu_si256((const __m256i*)(pData + i));
            __m256i vCmp = _mm256_cmpeq_epi8(vData, vZero);
            xsimd_uint32 nMask = _mm256_movemask_epi8(vCmp);
            
            if (nMask != 0) {
                /* Found null byte, find its position */
                unsigned long nBitPos;
#ifdef _MSC_VER
                _BitScanForward(&nBitPos, nMask);
#else
                nBitPos = __builtin_ctz(nMask);
#endif
                return i + nBitPos;
            }
        }
    } else if (g_nEnabledFeatures & XSIMD_FEATURE_AVX) {
        __m256i vZero = _mm256_setzero_si256();
        
        for (; i + 32 <= nSize; i += 32) {
            __m256i vData = _mm256_loadu_si256((const __m256i*)(pData + i));
            __m256i vCmp = _mm256_cmpeq_epi8(vData, vZero);
            xsimd_uint32 nMask = _mm256_movemask_epi8(vCmp);
            
            if (nMask != 0) {
                unsigned long nBitPos;
#ifdef _MSC_VER
                _BitScanForward(&nBitPos, nMask);
#else
                nBitPos = __builtin_ctz(nMask);
#endif
                return i + nBitPos;
            }
        }
    } else if (g_nEnabledFeatures & XSIMD_FEATURE_SSE2) {
        __m128i vZero = _mm_setzero_si128();
        
        for (; i + 16 <= nSize; i += 16) {
            __m128i vData = _mm_loadu_si128((const __m128i*)(pData + i));
            __m128i vCmp = _mm_cmpeq_epi8(vData, vZero);
            xsimd_uint16 nMask = _mm_movemask_epi8(vCmp);
            
            if (nMask != 0) {
                unsigned long nBitPos;
#ifdef _MSC_VER
                _BitScanForward(&nBitPos, nMask);
#else
                nBitPos = __builtin_ctz(nMask);
#endif
                return i + nBitPos;
            }
        }
    }
#endif
    
    /* Scalar fallback */
    for (; i < nSize; i++) {
        if (pData[i] == 0) {
            return i;
        }
    }
    
    return -1;
}

xsimd_int64 xsimd_count_unicode_prefix(const void* pBuffer, xsimd_int64 nSize)
{
    const xsimd_uint16* pData = (const xsimd_uint16*)pBuffer;
    xsimd_int64 nChars = nSize / 2;  /* Number of 16-bit characters */
    xsimd_int64 i = 0;
    
    if (!g_bInitialized) {
        xsimd_init();
    }
    
    /* Must be even size for 16-bit characters */
    if (nSize % 2 != 0) {
        return 0;
    }
    
#ifdef XSIMD_X86
    if (g_nEnabledFeatures & XSIMD_FEATURE_AVX2) {
        /* Process 16 characters (32 bytes) at a time */
        __m256i vMin1 = _mm256_set1_epi16(0x0020);
        __m256i vMax1 = _mm256_set1_epi16(0x00FF);
        __m256i vMin2 = _mm256_set1_epi16(0x0400);  /* Cyrillic start */
        __m256i vMax2 = _mm256_set1_epi16(0x04FF);  /* Cyrillic end */
        
        for (; i + 16 <= nChars; i += 16) {
            __m256i vData = _mm256_loadu_si256((const __m256i*)(pData + i));
            
            /* Check if in range [0x0020, 0x00FF] */
            __m256i vGe1 = _mm256_cmpgt_epi16(vData, _mm256_sub_epi16(vMin1, _mm256_set1_epi16(1)));
            __m256i vLe1 = _mm256_cmpgt_epi16(_mm256_add_epi16(vMax1, _mm256_set1_epi16(1)), vData);
            __m256i vRange1 = _mm256_and_si256(vGe1, vLe1);
            
            /* Check if in range [0x0400, 0x04FF] (Cyrillic) */
            __m256i vGe2 = _mm256_cmpgt_epi16(vData, _mm256_sub_epi16(vMin2, _mm256_set1_epi16(1)));
            __m256i vLe2 = _mm256_cmpgt_epi16(_mm256_add_epi16(vMax2, _mm256_set1_epi16(1)), vData);
            __m256i vRange2 = _mm256_and_si256(vGe2, vLe2);
            
            /* Valid if in either range */
            __m256i vValid = _mm256_or_si256(vRange1, vRange2);
            
            /* Check if all characters are valid */
            xsimd_uint32 nMask = _mm256_movemask_epi8(vValid);
            
            /* All 32 bytes should have alternating bits set (0xAAAAAAAA for 16-bit LE) */
            /* or all bits set (0xFFFFFFFF) */
            if (nMask != 0xFFFFFFFF) {
                /* Found invalid character, find first one */
                xsimd_int64 j;
                for (j = 0; j < 16; j++) {
                    xsimd_uint16 nChar = pData[i + j];
                    int bValid = ((nChar >= 0x0020 && nChar <= 0x00FF) || 
                                 (nChar >= 0x0400 && nChar <= 0x04FF));
                    if (!bValid) {
                        return (i + j) * 2;  /* Return byte offset */
                    }
                }
            }
        }
    } else if (g_nEnabledFeatures & XSIMD_FEATURE_AVX) {
        /* Process 16 characters (32 bytes) at a time with AVX */
        __m256i vMin1 = _mm256_set1_epi16(0x0020);
        __m256i vMax1 = _mm256_set1_epi16(0x00FF);
        __m256i vMin2 = _mm256_set1_epi16(0x0400);
        __m256i vMax2 = _mm256_set1_epi16(0x04FF);
        
        for (; i + 16 <= nChars; i += 16) {
            __m256i vData = _mm256_loadu_si256((const __m256i*)(pData + i));
            
            __m256i vGe1 = _mm256_cmpgt_epi16(vData, _mm256_sub_epi16(vMin1, _mm256_set1_epi16(1)));
            __m256i vLe1 = _mm256_cmpgt_epi16(_mm256_add_epi16(vMax1, _mm256_set1_epi16(1)), vData);
            __m256i vRange1 = _mm256_and_si256(vGe1, vLe1);
            
            __m256i vGe2 = _mm256_cmpgt_epi16(vData, _mm256_sub_epi16(vMin2, _mm256_set1_epi16(1)));
            __m256i vLe2 = _mm256_cmpgt_epi16(_mm256_add_epi16(vMax2, _mm256_set1_epi16(1)), vData);
            __m256i vRange2 = _mm256_and_si256(vGe2, vLe2);
            
            __m256i vValid = _mm256_or_si256(vRange1, vRange2);
            
            xsimd_uint32 nMask = _mm256_movemask_epi8(vValid);
            
            if (nMask != 0xFFFFFFFF) {
                xsimd_int64 j;
                for (j = 0; j < 16; j++) {
                    xsimd_uint16 nChar = pData[i + j];
                    int bValid = ((nChar >= 0x0020 && nChar <= 0x00FF) || 
                                 (nChar >= 0x0400 && nChar <= 0x04FF));
                    if (!bValid) {
                        return (i + j) * 2;
                    }
                }
            }
        }
    } else if (g_nEnabledFeatures & XSIMD_FEATURE_SSE2) {
        /* Process 8 characters (16 bytes) at a time */
        __m128i vMin1 = _mm_set1_epi16(0x0020);
        __m128i vMax1 = _mm_set1_epi16(0x00FF);
        __m128i vMin2 = _mm_set1_epi16(0x0400);
        __m128i vMax2 = _mm_set1_epi16(0x04FF);
        
        for (; i + 8 <= nChars; i += 8) {
            __m128i vData = _mm_loadu_si128((const __m128i*)(pData + i));
            
            __m128i vGe1 = _mm_cmpgt_epi16(vData, _mm_sub_epi16(vMin1, _mm_set1_epi16(1)));
            __m128i vLe1 = _mm_cmpgt_epi16(_mm_add_epi16(vMax1, _mm_set1_epi16(1)), vData);
            __m128i vRange1 = _mm_and_si128(vGe1, vLe1);
            
            __m128i vGe2 = _mm_cmpgt_epi16(vData, _mm_sub_epi16(vMin2, _mm_set1_epi16(1)));
            __m128i vLe2 = _mm_cmpgt_epi16(_mm_add_epi16(vMax2, _mm_set1_epi16(1)), vData);
            __m128i vRange2 = _mm_and_si128(vGe2, vLe2);
            
            __m128i vValid = _mm_or_si128(vRange1, vRange2);
            
            xsimd_uint16 nMask = _mm_movemask_epi8(vValid);
            
            if (nMask != 0xFFFF) {
                xsimd_int64 j;
                for (j = 0; j < 8; j++) {
                    xsimd_uint16 nChar = pData[i + j];
                    int bValid = ((nChar >= 0x0020 && nChar <= 0x00FF) || 
                                 (nChar >= 0x0400 && nChar <= 0x04FF));
                    if (!bValid) {
                        return (i + j) * 2;
                    }
                }
            }
        }
    }
#endif
    
    /* Scalar fallback */
    for (; i < nChars; i++) {
        xsimd_uint16 nChar = pData[i];
        int bValid = ((nChar >= 0x0020 && nChar <= 0x00FF) || 
                     (nChar >= 0x0400 && nChar <= 0x04FF));
        if (!bValid) {
            return i * 2;  /* Return byte offset */
        }
    }
    
    return nSize;  /* All characters are valid */
}

int xsimd_is_valid_unicode(const void* pBuffer, xsimd_int64 nSize)
{
    xsimd_int64 nValidBytes = xsimd_count_unicode_prefix(pBuffer, nSize);
    return (nValidBytes == nSize) ? 1 : 0;
}

xsimd_int64 xsimd_count_char(const void* pBuffer, xsimd_int64 nSize, xsimd_uint8 nByte)
{
    const xsimd_uint8* pData = (const xsimd_uint8*)pBuffer;
    xsimd_int64 nCount = 0;
    xsimd_int64 i = 0;
    
    if (!g_bInitialized) {
        xsimd_init();
    }
    
#ifdef XSIMD_X86
    if (g_nEnabledFeatures & XSIMD_FEATURE_AVX2) {
        __m256i vNeedle = _mm256_set1_epi8(nByte);
        
        for (; i + 32 <= nSize; i += 32) {
            __m256i vData = _mm256_loadu_si256((const __m256i*)(pData + i));
            __m256i vCmp = _mm256_cmpeq_epi8(vData, vNeedle);
            xsimd_uint32 nMask = _mm256_movemask_epi8(vCmp);
            
            /* Count set bits (popcount) */
#ifdef _MSC_VER
            nCount += __popcnt(nMask);
#else
            nCount += __builtin_popcount(nMask);
#endif
        }
    } else if (g_nEnabledFeatures & XSIMD_FEATURE_AVX) {
        __m256i vNeedle = _mm256_set1_epi8(nByte);
        
        for (; i + 32 <= nSize; i += 32) {
            __m256i vData = _mm256_loadu_si256((const __m256i*)(pData + i));
            __m256i vCmp = _mm256_cmpeq_epi8(vData, vNeedle);
            xsimd_uint32 nMask = _mm256_movemask_epi8(vCmp);
            
#ifdef _MSC_VER
            nCount += __popcnt(nMask);
#else
            nCount += __builtin_popcount(nMask);
#endif
        }
    } else if (g_nEnabledFeatures & XSIMD_FEATURE_SSE2) {
        __m128i vNeedle = _mm_set1_epi8(nByte);
        
        for (; i + 16 <= nSize; i += 16) {
            __m128i vData = _mm_loadu_si128((const __m128i*)(pData + i));
            __m128i vCmp = _mm_cmpeq_epi8(vData, vNeedle);
            xsimd_uint16 nMask = _mm_movemask_epi8(vCmp);
            
#ifdef _MSC_VER
            nCount += __popcnt(nMask);
#else
            nCount += __builtin_popcount(nMask);
#endif
        }
    }
#endif
    
    /* Scalar fallback */
    for (; i < nSize; i++) {
        if (pData[i] == nByte) {
            nCount++;
        }
    }
    
    return nCount;
}

xsimd_int64 xsimd_count_ansi_prefix(const void* pBuffer, xsimd_int64 nSize)
{
    xsimd_int64 nNonAnsiPos = xsimd_find_first_non_ansi(pBuffer, nSize);
    
    if (nNonAnsiPos == -1) {
        return nSize;  /* All bytes are ANSI */
    }
    
    return nNonAnsiPos;
}

xsimd_int64 xsimd_create_ansi_mask(const void* pBuffer, xsimd_int64 nSize, void* pMask)
{
    const xsimd_uint8* pData = (const xsimd_uint8*)pBuffer;
    xsimd_uint8* pMaskData = (xsimd_uint8*)pMask;
    xsimd_int64 nAnsiCount = 0;
    xsimd_int64 i = 0;
    
    if (!g_bInitialized) {
        xsimd_init();
    }
    
    /* Initialize mask to zero */
    memset(pMaskData, 0, (nSize + 7) / 8);
    
#ifdef XSIMD_X86
    if (g_nEnabledFeatures & XSIMD_FEATURE_AVX2) {
        __m256i vMin = _mm256_set1_epi8(0x20);
        __m256i vMax = _mm256_set1_epi8(0x7F);
        
        for (; i + 32 <= nSize; i += 32) {
            __m256i vData = _mm256_loadu_si256((const __m256i*)(pData + i));
            
            /* Check if byte >= 0x20 AND < 0x80 */
            __m256i vGreaterEqMin = _mm256_cmpgt_epi8(vData, _mm256_sub_epi8(vMin, _mm256_set1_epi8(1)));
            __m256i vLessThanMax = _mm256_cmpgt_epi8(vMax, vData);
            __m256i vIsAnsi = _mm256_and_si256(vGreaterEqMin, vLessThanMax);
            
            xsimd_uint32 nMask = _mm256_movemask_epi8(vIsAnsi);
            
            /* Store mask bits */
            pMaskData[i / 8] = (xsimd_uint8)(nMask & 0xFF);
            pMaskData[i / 8 + 1] = (xsimd_uint8)((nMask >> 8) & 0xFF);
            pMaskData[i / 8 + 2] = (xsimd_uint8)((nMask >> 16) & 0xFF);
            pMaskData[i / 8 + 3] = (xsimd_uint8)((nMask >> 24) & 0xFF);
            
            /* Count ANSI characters (popcount) */
#ifdef _MSC_VER
            nAnsiCount += __popcnt(nMask);
#else
            nAnsiCount += __builtin_popcount(nMask);
#endif
        }
    } else if (g_nEnabledFeatures & XSIMD_FEATURE_AVX) {
        __m256i vMin = _mm256_set1_epi8(0x20);
        __m256i vMax = _mm256_set1_epi8(0x7F);
        
        for (; i + 32 <= nSize; i += 32) {
            __m256i vData = _mm256_loadu_si256((const __m256i*)(pData + i));
            
            __m256i vGreaterEqMin = _mm256_cmpgt_epi8(vData, _mm256_sub_epi8(vMin, _mm256_set1_epi8(1)));
            __m256i vLessThanMax = _mm256_cmpgt_epi8(vMax, vData);
            __m256i vIsAnsi = _mm256_and_si256(vGreaterEqMin, vLessThanMax);
            
            xsimd_uint32 nMask = _mm256_movemask_epi8(vIsAnsi);
            
            pMaskData[i / 8] = (xsimd_uint8)(nMask & 0xFF);
            pMaskData[i / 8 + 1] = (xsimd_uint8)((nMask >> 8) & 0xFF);
            pMaskData[i / 8 + 2] = (xsimd_uint8)((nMask >> 16) & 0xFF);
            pMaskData[i / 8 + 3] = (xsimd_uint8)((nMask >> 24) & 0xFF);
            
#ifdef _MSC_VER
            nAnsiCount += __popcnt(nMask);
#else
            nAnsiCount += __builtin_popcount(nMask);
#endif
        }
    } else if (g_nEnabledFeatures & XSIMD_FEATURE_SSE2) {
        __m128i vMin = _mm_set1_epi8(0x20);
        __m128i vMax = _mm_set1_epi8(0x7F);
        
        for (; i + 16 <= nSize; i += 16) {
            __m128i vData = _mm_loadu_si128((const __m128i*)(pData + i));
            
            __m128i vGreaterEqMin = _mm_cmpgt_epi8(vData, _mm_sub_epi8(vMin, _mm_set1_epi8(1)));
            __m128i vLessThanMax = _mm_cmpgt_epi8(vMax, vData);
            __m128i vIsAnsi = _mm_and_si128(vGreaterEqMin, vLessThanMax);
            
            xsimd_uint16 nMask = _mm_movemask_epi8(vIsAnsi);
            
            pMaskData[i / 8] = (xsimd_uint8)(nMask & 0xFF);
            pMaskData[i / 8 + 1] = (xsimd_uint8)((nMask >> 8) & 0xFF);
            
            /* Count ANSI characters */
#ifdef _MSC_VER
            nAnsiCount += __popcnt(nMask);
#else
            nAnsiCount += __builtin_popcount(nMask);
#endif
        }
    }
#endif
    
    /* Scalar fallback for remaining bytes */
    for (; i < nSize; i++) {
        xsimd_uint8 nByte = pData[i];
        if (nByte >= 0x20 && nByte < 0x80) {
            pMaskData[i / 8] |= (1 << (i % 8));
            nAnsiCount++;
        }
    }
    
    return nAnsiCount;
}

void xsimd_cleanup(void)
{
    g_bInitialized = 0;
    g_nFeatures = XSIMD_FEATURE_NONE;
    g_nEnabledFeatures = XSIMD_FEATURE_NONE;
}
