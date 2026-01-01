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
#include "xsimd_avx2.h"
#include <string.h>

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#define XSIMD_X86
#include <immintrin.h>
#endif

xsimd_int64 _xsimd_find_byte_AVX2(const unsigned char* pData, xsimd_int64 nSize, xsimd_uint8 nByte, xsimd_int64 nOffset)
{
#ifdef XSIMD_X86
    __m256i vNeedle = _mm256_set1_epi8((char)nByte);
    xsimd_int64 i = 0;
    
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
            return i + nBitPos;
#else
            return i + __builtin_ctz((unsigned int)nMask0);
#endif
        }
        
        if (nMask1 != 0) {
#ifdef _MSC_VER
            unsigned long nBitPos;
            _BitScanForward(&nBitPos, (unsigned long)nMask1);
            return i + 32 + nBitPos;
#else
            return i + 32 + __builtin_ctz((unsigned int)nMask1);
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
            return i + nBitPos;
#else
            return i + __builtin_ctz((unsigned int)nMask);
#endif
        }
        i += 32;
    }
    
    return i;
#else
    return 0;
#endif
}

xsimd_int64 _xsimd_find_pattern_bmh_AVX2(const char* pHay, xsimd_int64 nBufferSize, const char* pNeedle, xsimd_int64 nPatternSize, xsimd_int64 nOffset, xsimd_int64 nLimit, char nLastChar, xsimd_int64 nStartPos)
{
#ifdef XSIMD_X86
    __m256i vLast = _mm256_set1_epi8((char)nLastChar);
    xsimd_int64 i = nStartPos;
    
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
    
    /* Return -1 to signal no match found by SIMD, fallback will continue from nStartPos */
    return -1;
#else
    return -1;
#endif
}

xsimd_int64 _xsimd_find_pattern_bmh_3byte_AVX2(const char* pHay, xsimd_int64 nBufferSize, const char* pNeedle, xsimd_int64 nOffset)
{
#ifdef XSIMD_X86
    __m256i vFirst = _mm256_set1_epi8(pNeedle[0]);
    const char nSecond = pNeedle[1];
    const char nThird = pNeedle[2];
    xsimd_int64 i = 0;
    
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
#endif
    
    return -1;
}

xsimd_int64 _xsimd_find_pattern_bmh_2byte_AVX2(const char* pHay, xsimd_int64 nBufferSize, xsimd_uint16 pattern16, xsimd_int64 nOffset)
{
#ifdef XSIMD_X86
    __m256i vPattern = _mm256_set1_epi16((short)pattern16);
    xsimd_int64 i = 0;
    
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
#endif
    
    return -1;
}

xsimd_int64 _xsimd_find_pattern_bmh_1byte_AVX2(const char* pHay, xsimd_int64 nBufferSize, char nByte, xsimd_int64 nOffset)
{
#ifdef XSIMD_X86
    __m256i vPattern = _mm256_set1_epi8(nByte);
    xsimd_int64 i = 0;
    
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
        if (pHay[i] == nByte) {
            return nOffset + i;
        }
        i++;
    }
#endif
    
    return -1;
}

xsimd_int64 _xsimd_find_ansi_AVX2(const unsigned char* pData, xsimd_int64 nBufferSize, xsimd_int64 nMinLength, xsimd_int64 nOffset)
{
#ifdef XSIMD_X86
    const __m256i vLowerMinus1 = _mm256_set1_epi8(0x1F);
    const __m256i vUpperPlus1 = _mm256_set1_epi8(0x7F);
    xsimd_int64 j = 0;
    
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
    
    return j;
#else
    return 0;
#endif
}

xsimd_int64 _xsimd_find_notnull_AVX2(const unsigned char* pData, xsimd_int64 nBufferSize, xsimd_int64 nMinLength, xsimd_int64 nOffset, xsimd_int64 j, xsimd_int64 runStart)
{
#ifdef XSIMD_X86
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
    
    return j | (runStart << 32);  /* Pack both values for return */
#else
    return j | (runStart << 32);
#endif
}

xsimd_int64 _xsimd_find_not_ansi_AVX2(const unsigned char* pData, xsimd_int64 nBufferSize, xsimd_int64 nMinLength, xsimd_int64 nOffset)
{
#ifdef XSIMD_X86
    const __m256i vLower = _mm256_set1_epi8(0x20);
    const __m256i vUpper = _mm256_set1_epi8(0x7E);
    xsimd_int64 j = 0;
    
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
    
    return j;
#else
    return 0;
#endif
}

int _xsimd_is_not_null_AVX2(const char* ptr, xsimd_int64 nSize)
{
#ifdef XSIMD_X86
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
    
    return -1;  /* Continue with fallback */
#else
    return -1;
#endif
}

int _xsimd_is_ansi_AVX2(const char* ptr, xsimd_int64 nSize)
{
#ifdef XSIMD_X86
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
    
    return -1;  /* Continue with fallback */
#else
    return -1;
#endif
}

int _xsimd_is_not_ansi_AVX2(const char* ptr, xsimd_int64 nSize)
{
#ifdef XSIMD_X86
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
    
    return -1;  /* Continue with fallback */
#else
    return -1;
#endif
}

int _xsimd_compare_memory_AVX2(const char* ptr1, const char* ptr2, xsimd_int64 nSize)
{
#ifdef XSIMD_X86
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
    
    return -1;  /* Continue with fallback */
#else
    return -1;
#endif
}

xsimd_int64 _xsimd_find_not_ansi_and_null_AVX2(const unsigned char* hay, xsimd_int64 hayLen, xsimd_int64 nOffset, xsimd_int64 m, const xsimd_int64 limit, const int* ansiTable, xsimd_int64* pj)
{
#ifdef XSIMD_X86
    const xsimd_int64 vectorSize = 32;
    __m256i vMin = _mm256_set1_epi8(0x20);
    __m256i vMax = _mm256_set1_epi8(0x7E);
    __m256i vZero = _mm256_setzero_si256();
    xsimd_int64 j = *pj;
    
    while (j + vectorSize <= hayLen) {
        __m256i vData = _mm256_loadu_si256((const __m256i*)(hay + j));
        
        __m256i vLt = _mm256_cmpgt_epi8(vMin, vData);
        __m256i vNotNull = _mm256_cmpeq_epi8(vData, vZero);
        vLt = _mm256_andnot_si256(vNotNull, vLt);
        
        __m256i vGt = _mm256_cmpgt_epi8(vData, vMax);
        
        __m256i vNotAnsiAndNull = _mm256_or_si256(vLt, vGt);
        
        xsimd_uint32 mask = _mm256_movemask_epi8(vNotAnsiAndNull);
        
        if (mask != 0) {
            xsimd_int32 k;
            for (k = 0; k < vectorSize && j + k < limit; k++) {
                if (mask & (1U << k)) {
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
    
    *pj = j;
#endif
    return -1;
}

int _xsimd_is_not_ansi_and_null_AVX2(const char* ptr, xsimd_int64 nSize)
{
#ifdef XSIMD_X86
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
    
    return -1;  /* Continue with fallback */
#else
    return -1;
#endif
}

xsimd_int64 _xsimd_find_ansi_number_AVX2(const unsigned char* hay, xsimd_int64 hayLen, xsimd_int64 nOffset, xsimd_int64 m, const xsimd_int64 limit, xsimd_int64* pj)
{
#ifdef XSIMD_X86
    const xsimd_int64 nVectorSize = 32;
    __m256i vMin = _mm256_set1_epi8(0x30);
    __m256i vMax = _mm256_set1_epi8(0x39);
    __m256i vOne = _mm256_set1_epi8(1);
    xsimd_int64 j = *pj;
    
    while (j < limit) {
        int bFoundDigit = 0;
        
        for (; j + nVectorSize <= hayLen; j += nVectorSize) {
            __m256i vData = _mm256_loadu_si256((const __m256i*)(hay + j));
            
            __m256i vGe = _mm256_cmpgt_epi8(vData, _mm256_sub_epi8(vMin, vOne));
            __m256i vLe = _mm256_cmpgt_epi8(_mm256_add_epi8(vMax, vOne), vData);
            __m256i vDigit = _mm256_and_si256(vGe, vLe);
            
            xsimd_uint32 nMask = _mm256_movemask_epi8(vDigit);
            
            if (nMask != 0) {
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
    
    *pj = j;
#endif
    return -1;
}

int _xsimd_is_ansi_number_AVX2(const char* ptr, xsimd_int64 nSize)
{
#ifdef XSIMD_X86
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
    
    return -1;  /* Continue with fallback */
#else
    return -1;
#endif
}

xsimd_int64 _xsimd_find_first_non_ansi_AVX2(const xsimd_uint8* pData, xsimd_int64 i, xsimd_int64 nSize)
{
#ifdef XSIMD_X86
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
    
    return -1;  /* Not found in SIMD portion */
#else
    return -1;
#endif
}

xsimd_int64 _xsimd_find_null_byte_AVX2(const xsimd_uint8* pData, xsimd_int64 nSize, xsimd_int64* pi)
{
#ifdef XSIMD_X86
    __m256i vZero = _mm256_setzero_si256();
    xsimd_int64 i = *pi;
    
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
    
    *pi = i;
#endif
    return -1;
}

xsimd_int64 _xsimd_count_unicode_prefix_AVX2(const xsimd_uint16* pData, xsimd_int64 nChars, xsimd_int64* pi)
{
#ifdef XSIMD_X86
    __m256i vMin1 = _mm256_set1_epi16(0x0020);
    __m256i vMax1 = _mm256_set1_epi16(0x00FF);
    __m256i vMin2 = _mm256_set1_epi16(0x0400);
    __m256i vMax2 = _mm256_set1_epi16(0x04FF);
    xsimd_int64 i = *pi;
    
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
                    *pi = i + j;
                    return (i + j) * 2;
                }
            }
        }
    }
    
    *pi = i;
#endif
    return -1;
}

void _xsimd_count_char_AVX2(const xsimd_uint8* pData, xsimd_int64 nSize, xsimd_uint8 nByte, xsimd_int64* pi, xsimd_int64* pnCount)
{
#ifdef XSIMD_X86
    __m256i vNeedle = _mm256_set1_epi8(nByte);
    xsimd_int64 i = *pi;
    
    for (; i + 32 <= nSize; i += 32) {
        __m256i vData = _mm256_loadu_si256((const __m256i*)(pData + i));
        __m256i vCmp = _mm256_cmpeq_epi8(vData, vNeedle);
        xsimd_uint32 nMask = _mm256_movemask_epi8(vCmp);
        
#ifdef _MSC_VER
        *pnCount += __popcnt(nMask);
#else
        *pnCount += __builtin_popcount(nMask);
#endif
    }
    
    *pi = i;
#endif
}

void _xsimd_create_ansi_mask_AVX2(const xsimd_uint8* pData, xsimd_int64 nSize, xsimd_uint8* pMaskData, xsimd_int64* pi, xsimd_int64* pnAnsiCount)
{
#ifdef XSIMD_X86
    __m256i vMin = _mm256_set1_epi8(0x20);
    __m256i vMax = _mm256_set1_epi8(0x7F);
    xsimd_int64 i = *pi;
    
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
        *pnAnsiCount += __popcnt(nMask);
#else
        *pnAnsiCount += __builtin_popcount(nMask);
#endif
    }
    
    *pi = i;
#endif
}

