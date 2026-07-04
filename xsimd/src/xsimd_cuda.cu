/* Copyright (c) 2025-2026 hors<horsicq@gmail.com>
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
#include "xsimd_cuda.h"
#include <cuda_runtime.h>
#include <string.h>

/* ── launch configuration ─────────────────────────────────────────────────── */

#define CUDA_THREADS 256
#define CUDA_MAX_BLOCKS 65535

static int cuda_blocks(long long n, int threads)
{
    long long b = (n + threads - 1) / threads;
    return (int)(b > CUDA_MAX_BLOCKS ? CUDA_MAX_BLOCKS : b);
}

/* ── RAII device buffer ───────────────────────────────────────────────────── */

struct DevBuf {
    void* p;
    DevBuf() : p(nullptr) {}
    bool alloc(size_t n) { return cudaMalloc(&p, n) == cudaSuccess; }
    bool upload(const void* host, size_t n) { return cudaMemcpy(p, host, n, cudaMemcpyHostToDevice) == cudaSuccess; }
    bool download(void* host, size_t n) { return cudaMemcpy(host, p, n, cudaMemcpyDeviceToHost) == cudaSuccess; }
    ~DevBuf() { if (p) cudaFree(p); }
};

/* ── kernels ──────────────────────────────────────────────────────────────── */

/* --- find_byte --- */
__global__ void k_find_byte(const unsigned char* d, long long n, unsigned char b, unsigned long long* res)
{
    long long i = (long long)blockIdx.x * blockDim.x + threadIdx.x;
    long long s = (long long)blockDim.x * gridDim.x;
    for (; i < n; i += s) {
        if ((unsigned long long)i >= *res) return;
        if (d[i] == b) atomicMin(res, (unsigned long long)i);
    }
}

/* --- find_pattern (generic, parallel naive search) --- */
__global__ void k_find_pattern(const char* hay, long long hayLen, const char* needle, long long needleLen,
                                unsigned long long* res)
{
    long long limit = hayLen - needleLen + 1;
    long long i = (long long)blockIdx.x * blockDim.x + threadIdx.x;
    long long s = (long long)blockDim.x * gridDim.x;
    for (; i < limit; i += s) {
        if ((unsigned long long)i >= *res) return;
        int ok = 1;
        for (long long k = 0; k < needleLen && ok; k++)
            if (hay[i + k] != needle[k]) ok = 0;
        if (ok) atomicMin(res, (unsigned long long)i);
    }
}

/* --- find_pattern_1byte --- */
__global__ void k_find_pattern_1byte(const char* hay, long long n, char b, unsigned long long* res)
{
    long long i = (long long)blockIdx.x * blockDim.x + threadIdx.x;
    long long s = (long long)blockDim.x * gridDim.x;
    for (; i < n; i += s) {
        if ((unsigned long long)i >= *res) return;
        if (hay[i] == b) atomicMin(res, (unsigned long long)i);
    }
}

/* --- find_pattern_2byte --- */
__global__ void k_find_pattern_2byte(const char* hay, long long n, unsigned short pat, unsigned long long* res)
{
    long long i = (long long)blockIdx.x * blockDim.x + threadIdx.x;
    long long s = (long long)blockDim.x * gridDim.x;
    long long limit = n - 1;
    for (; i < limit; i += s) {
        if ((unsigned long long)i >= *res) return;
        unsigned short v;
        memcpy(&v, hay + i, 2);
        if (v == pat) atomicMin(res, (unsigned long long)i);
    }
}

/* --- find_pattern_3byte --- */
__global__ void k_find_pattern_3byte(const char* hay, long long n, const char* needle, unsigned long long* res)
{
    long long i = (long long)blockIdx.x * blockDim.x + threadIdx.x;
    long long s = (long long)blockDim.x * gridDim.x;
    long long limit = n - 2;
    for (; i < limit; i += s) {
        if ((unsigned long long)i >= *res) return;
        if (hay[i] == needle[0] && hay[i+1] == needle[1] && hay[i+2] == needle[2])
            atomicMin(res, (unsigned long long)i);
    }
}

/* --- find_ansi: first run of >= m ANSI (0x20..0x7E) bytes --- */
__global__ void k_find_ansi(const unsigned char* d, long long n, long long m, unsigned long long* res)
{
    long long limit = n - m + 1;
    long long i = (long long)blockIdx.x * blockDim.x + threadIdx.x;
    long long s = (long long)blockDim.x * gridDim.x;
    for (; i < limit; i += s) {
        if ((unsigned long long)i >= *res) return;
        int ok = 1;
        for (long long k = 0; k < m && ok; k++) {
            unsigned char c = d[i + k];
            if (c < 0x20 || c > 0x7E) ok = 0;
        }
        if (ok) atomicMin(res, (unsigned long long)i);
    }
}

/* --- find_notnull: first run of >= m non-zero bytes --- */
__global__ void k_find_notnull(const unsigned char* d, long long n, long long m, unsigned long long* res)
{
    long long limit = n - m + 1;
    long long i = (long long)blockIdx.x * blockDim.x + threadIdx.x;
    long long s = (long long)blockDim.x * gridDim.x;
    for (; i < limit; i += s) {
        if ((unsigned long long)i >= *res) return;
        int ok = 1;
        for (long long k = 0; k < m && ok; k++)
            if (d[i + k] == 0) ok = 0;
        if (ok) atomicMin(res, (unsigned long long)i);
    }
}

/* --- find_not_ansi: first run of >= m non-ANSI bytes --- */
__global__ void k_find_not_ansi(const unsigned char* d, long long n, long long m, unsigned long long* res)
{
    long long limit = n - m + 1;
    long long i = (long long)blockIdx.x * blockDim.x + threadIdx.x;
    long long s = (long long)blockDim.x * gridDim.x;
    for (; i < limit; i += s) {
        if ((unsigned long long)i >= *res) return;
        int ok = 1;
        for (long long k = 0; k < m && ok; k++) {
            unsigned char c = d[i + k];
            if (c >= 0x20 && c <= 0x7E) ok = 0;
        }
        if (ok) atomicMin(res, (unsigned long long)i);
    }
}

/* --- is_not_null: 1 if no zero byte, 0 if zero found --- */
__global__ void k_is_not_null(const char* p, long long n, int* res)
{
    long long i = (long long)blockIdx.x * blockDim.x + threadIdx.x;
    long long s = (long long)blockDim.x * gridDim.x;
    for (; i < n; i += s) {
        if (!*res) return;
        if (p[i] == 0) { atomicExch(res, 0); return; }
    }
}

/* --- is_ansi: 1 if all bytes 0x20..0x7F, 0 otherwise --- */
__global__ void k_is_ansi(const unsigned char* p, long long n, int* res)
{
    long long i = (long long)blockIdx.x * blockDim.x + threadIdx.x;
    long long s = (long long)blockDim.x * gridDim.x;
    for (; i < n; i += s) {
        if (!*res) return;
        unsigned char c = p[i];
        if (c < 0x20 || c >= 0x80) { atomicExch(res, 0); return; }
    }
}

/* --- is_not_ansi: 1 if no byte is ANSI, 0 otherwise --- */
__global__ void k_is_not_ansi(const unsigned char* p, long long n, int* res)
{
    long long i = (long long)blockIdx.x * blockDim.x + threadIdx.x;
    long long s = (long long)blockDim.x * gridDim.x;
    for (; i < n; i += s) {
        if (!*res) return;
        unsigned char c = p[i];
        if (c >= 0x20 && c < 0x80) { atomicExch(res, 0); return; }
    }
}

/* --- compare_memory: 1 if equal, 0 if different --- */
__global__ void k_compare_memory(const char* a, const char* b, long long n, int* res)
{
    long long i = (long long)blockIdx.x * blockDim.x + threadIdx.x;
    long long s = (long long)blockDim.x * gridDim.x;
    for (; i < n; i += s) {
        if (!*res) return;
        if (a[i] != b[i]) { atomicExch(res, 0); return; }
    }
}

/* --- find_not_ansi_and_null: first run of >= m bytes that are (not ANSI) and (not 0x00) --- */
__global__ void k_find_not_ansi_and_null(const unsigned char* d, long long n, long long m, unsigned long long* res)
{
    long long limit = n - m + 1;
    long long i = (long long)blockIdx.x * blockDim.x + threadIdx.x;
    long long s = (long long)blockDim.x * gridDim.x;
    for (; i < limit; i += s) {
        if ((unsigned long long)i >= *res) return;
        int ok = 1;
        for (long long k = 0; k < m && ok; k++) {
            unsigned char c = d[i + k];
            if (c == 0x00 || (c >= 0x20 && c <= 0x7E)) ok = 0;
        }
        if (ok) atomicMin(res, (unsigned long long)i);
    }
}

/* --- is_not_ansi_and_null: 1 if all bytes are (not ANSI) and (not 0x00) --- */
__global__ void k_is_not_ansi_and_null(const unsigned char* p, long long n, int* res)
{
    long long i = (long long)blockIdx.x * blockDim.x + threadIdx.x;
    long long s = (long long)blockDim.x * gridDim.x;
    for (; i < n; i += s) {
        if (!*res) return;
        unsigned char c = p[i];
        if (c == 0x00 || (c >= 0x20 && c <= 0x7E)) { atomicExch(res, 0); return; }
    }
}

/* --- find_ansi_number: first run of >= m digit bytes (0x30..0x39) --- */
__global__ void k_find_ansi_number(const unsigned char* d, long long n, long long m, unsigned long long* res)
{
    long long limit = n - m + 1;
    long long i = (long long)blockIdx.x * blockDim.x + threadIdx.x;
    long long s = (long long)blockDim.x * gridDim.x;
    for (; i < limit; i += s) {
        if ((unsigned long long)i >= *res) return;
        int ok = 1;
        for (long long k = 0; k < m && ok; k++) {
            unsigned char c = d[i + k];
            if (c < 0x30 || c > 0x39) ok = 0;
        }
        if (ok) atomicMin(res, (unsigned long long)i);
    }
}

/* --- is_ansi_number: 1 if all bytes are digits --- */
__global__ void k_is_ansi_number(const unsigned char* p, long long n, int* res)
{
    long long i = (long long)blockIdx.x * blockDim.x + threadIdx.x;
    long long s = (long long)blockDim.x * gridDim.x;
    for (; i < n; i += s) {
        if (!*res) return;
        unsigned char c = p[i];
        if (c < 0x30 || c > 0x39) { atomicExch(res, 0); return; }
    }
}

/* --- find_first_non_ansi: first byte outside 0x20..0x7F --- */
__global__ void k_find_first_non_ansi(const unsigned char* d, long long n, unsigned long long* res)
{
    long long i = (long long)blockIdx.x * blockDim.x + threadIdx.x;
    long long s = (long long)blockDim.x * gridDim.x;
    for (; i < n; i += s) {
        if ((unsigned long long)i >= *res) return;
        unsigned char c = d[i];
        if (c < 0x20 || c >= 0x80) atomicMin(res, (unsigned long long)i);
    }
}

/* --- find_null_byte: first 0x00 byte --- */
__global__ void k_find_null_byte(const unsigned char* d, long long n, unsigned long long* res)
{
    long long i = (long long)blockIdx.x * blockDim.x + threadIdx.x;
    long long s = (long long)blockDim.x * gridDim.x;
    for (; i < n; i += s) {
        if ((unsigned long long)i >= *res) return;
        if (d[i] == 0) atomicMin(res, (unsigned long long)i);
    }
}

/* --- count_unicode_prefix: first non-valid UTF-16LE character index (chars, not bytes) ---
   Valid: 0x0020-0x00FF  or  0x0400-0x04FF */
__global__ void k_count_unicode_prefix(const unsigned short* d, long long nChars, unsigned long long* res)
{
    long long i = (long long)blockIdx.x * blockDim.x + threadIdx.x;
    long long s = (long long)blockDim.x * gridDim.x;
    for (; i < nChars; i += s) {
        if ((unsigned long long)i >= *res) return;
        unsigned short c = d[i];
        int valid = ((c >= 0x0020 && c <= 0x00FF) || (c >= 0x0400 && c <= 0x04FF));
        if (!valid) atomicMin(res, (unsigned long long)i);
    }
}

/* --- count_char: count occurrences of a byte --- */
__global__ void k_count_char(const unsigned char* d, long long n, unsigned char b, unsigned long long* cnt)
{
    long long i = (long long)blockIdx.x * blockDim.x + threadIdx.x;
    long long s = (long long)blockDim.x * gridDim.x;
    unsigned long long local = 0;
    for (; i < n; i += s)
        if (d[i] == b) local++;
    atomicAdd(cnt, local);
}

/* --- create_ansi_mask: each thread owns one mask byte (8 data bytes) --- */
__global__ void k_create_ansi_mask(const unsigned char* d, long long nSize, unsigned char* mask, unsigned long long* cnt)
{
    long long maskBytes = (nSize + 7) / 8;
    long long b = (long long)blockIdx.x * blockDim.x + threadIdx.x;
    long long s = (long long)blockDim.x * gridDim.x;
    unsigned long long local = 0;
    for (; b < maskBytes; b += s) {
        unsigned char m = 0;
        long long base = b * 8;
        for (int bit = 0; bit < 8 && (base + bit) < nSize; bit++) {
            unsigned char c = d[base + bit];
            if (c >= 0x20 && c < 0x80) { m |= (unsigned char)(1u << bit); local++; }
        }
        mask[b] = m;
    }
    atomicAdd(cnt, local);
}

/* --- sigbyte match helper --- */
__device__ static int check_sigbyte(const unsigned char* sig, long long nPairs,
                                     const unsigned char* data, const unsigned char* alphaNum)
{
    for (long long i = 0; i < nPairs; i++) {
        unsigned char type  = sig[i * 2];
        unsigned char value = sig[i * 2 + 1];
        unsigned char c     = data[i];
        int ok = 1;
        switch (type) {
            case 0: ok = (c == value); break;
            case 1: break;
            case 2: ok = (c >= 0x20 && c <= 0x7E); break;
            case 3: ok = (c < 0x20 || c > 0x7E); break;
            case 4: ok = (c != 0 && (c < 0x20 || c > 0x7E)); break;
            case 5: ok = alphaNum[c]; break;
            case 6: ok = (c != 0); break;
            default: ok = 0; break;
        }
        if (!ok) return 0;
    }
    return 1;
}

/* --- compare_sigbytes: 1 = match, 0 = no match --- */
__global__ void k_compare_sigbytes(const unsigned char* sig, long long nPairs,
                                    const unsigned char* data, const unsigned char* alphaNum,
                                    int* res)
{
    long long i = (long long)blockIdx.x * blockDim.x + threadIdx.x;
    long long stride = (long long)blockDim.x * gridDim.x;
    for (; i < nPairs; i += stride) {
        if (!*res) return;
        unsigned char type  = sig[i * 2];
        unsigned char value = sig[i * 2 + 1];
        unsigned char c     = data[i];
        int ok = 1;
        switch (type) {
            case 0: ok = (c == value); break;
            case 1: break;
            case 2: ok = (c >= 0x20 && c <= 0x7E); break;
            case 3: ok = (c < 0x20 || c > 0x7E); break;
            case 4: ok = (c != 0 && (c < 0x20 || c > 0x7E)); break;
            case 5: ok = alphaNum[c]; break;
            case 6: ok = (c != 0); break;
            default: ok = 0; break;
        }
        if (!ok) atomicExch(res, 0);
    }
}

/* --- find_sigbytes: first position where signature matches --- */
__global__ void k_find_sigbytes(const unsigned char* data, long long dataSize,
                                 const unsigned char* sig, long long nPairs,
                                 const unsigned char* alphaNum, unsigned long long* res)
{
    long long limit = dataSize - nPairs + 1;
    long long i = (long long)blockIdx.x * blockDim.x + threadIdx.x;
    long long s = (long long)blockDim.x * gridDim.x;
    for (; i < limit; i += s) {
        if ((unsigned long long)i >= *res) return;
        if (check_sigbyte(sig, nPairs, data + i, alphaNum))
            atomicMin(res, (unsigned long long)i);
    }
}

/* ── C-callable wrappers ──────────────────────────────────────────────────── */

extern "C" {

int _xsimd_cuda_detect(void)
{
    int n = 0;
    return (cudaGetDeviceCount(&n) == cudaSuccess && n > 0) ? 1 : 0;
}

/* ── find_byte ───────────────────────────────────────────────────────────── */
xsimd_int64 _xsimd_find_byte_CUDA(const unsigned char* pData, xsimd_int64 nSize,
                                   xsimd_uint8 nByte, xsimd_int64 nOffset)
{
    DevBuf dData, dRes;
    if (!dData.alloc((size_t)nSize)) return XSIMD_CUDA_FAIL;
    if (!dRes.alloc(sizeof(unsigned long long))) return XSIMD_CUDA_FAIL;
    unsigned long long init = (unsigned long long)nSize;
    dData.upload(pData, (size_t)nSize);
    dRes.upload(&init, sizeof(init));
    k_find_byte<<<cuda_blocks(nSize, CUDA_THREADS), CUDA_THREADS>>>(
        (const unsigned char*)dData.p, nSize, nByte, (unsigned long long*)dRes.p);
    unsigned long long result;
    dRes.download(&result, sizeof(result));
    return (result >= (unsigned long long)nSize) ? (xsimd_int64)-1 : nOffset + (xsimd_int64)result;
}

/* ── find_pattern_bmh (generic) ──────────────────────────────────────────── */
xsimd_int64 _xsimd_find_pattern_bmh_CUDA(const char* pHay, xsimd_int64 nBufferSize,
                                          const char* pNeedle, xsimd_int64 nPatternSize,
                                          xsimd_int64 nOffset, xsimd_int64 nLimit,
                                          char nLastChar, xsimd_int64* pPosition)
{
    (void)nLimit; (void)nLastChar; (void)pPosition;
    DevBuf dHay, dNeedle, dRes;
    if (!dHay.alloc((size_t)nBufferSize)) return XSIMD_CUDA_FAIL;
    if (!dNeedle.alloc((size_t)nPatternSize)) return XSIMD_CUDA_FAIL;
    if (!dRes.alloc(sizeof(unsigned long long))) return XSIMD_CUDA_FAIL;
    unsigned long long init = (unsigned long long)nBufferSize;
    dHay.upload(pHay, (size_t)nBufferSize);
    dNeedle.upload(pNeedle, (size_t)nPatternSize);
    dRes.upload(&init, sizeof(init));
    k_find_pattern<<<cuda_blocks(nBufferSize, CUDA_THREADS), CUDA_THREADS>>>(
        (const char*)dHay.p, nBufferSize, (const char*)dNeedle.p, nPatternSize,
        (unsigned long long*)dRes.p);
    unsigned long long result;
    dRes.download(&result, sizeof(result));
    if (result >= (unsigned long long)nBufferSize) return -1;
    *pPosition = (xsimd_int64)result;
    return nOffset + (xsimd_int64)result;
}

/* ── find_pattern 1/2/3-byte ─────────────────────────────────────────────── */
xsimd_int64 _xsimd_find_pattern_bmh_1byte_CUDA(const char* pHay, xsimd_int64 nBufferSize,
                                                char nByte, xsimd_int64 nOffset)
{
    DevBuf dHay, dRes;
    if (!dHay.alloc((size_t)nBufferSize)) return XSIMD_CUDA_FAIL;
    if (!dRes.alloc(sizeof(unsigned long long))) return XSIMD_CUDA_FAIL;
    unsigned long long init = (unsigned long long)nBufferSize;
    dHay.upload(pHay, (size_t)nBufferSize);
    dRes.upload(&init, sizeof(init));
    k_find_pattern_1byte<<<cuda_blocks(nBufferSize, CUDA_THREADS), CUDA_THREADS>>>(
        (const char*)dHay.p, nBufferSize, nByte, (unsigned long long*)dRes.p);
    unsigned long long result;
    dRes.download(&result, sizeof(result));
    return (result >= (unsigned long long)nBufferSize) ? (xsimd_int64)-1 : nOffset + (xsimd_int64)result;
}

xsimd_int64 _xsimd_find_pattern_bmh_2byte_CUDA(const char* pHay, xsimd_int64 nBufferSize,
                                                xsimd_uint16 pattern16, xsimd_int64 nOffset)
{
    if (nBufferSize < 2) return -1;
    DevBuf dHay, dRes;
    if (!dHay.alloc((size_t)nBufferSize)) return XSIMD_CUDA_FAIL;
    if (!dRes.alloc(sizeof(unsigned long long))) return XSIMD_CUDA_FAIL;
    unsigned long long init = (unsigned long long)nBufferSize;
    dHay.upload(pHay, (size_t)nBufferSize);
    dRes.upload(&init, sizeof(init));
    k_find_pattern_2byte<<<cuda_blocks(nBufferSize - 1, CUDA_THREADS), CUDA_THREADS>>>(
        (const char*)dHay.p, nBufferSize, (unsigned short)pattern16, (unsigned long long*)dRes.p);
    unsigned long long result;
    dRes.download(&result, sizeof(result));
    return (result >= (unsigned long long)nBufferSize) ? (xsimd_int64)-1 : nOffset + (xsimd_int64)result;
}

xsimd_int64 _xsimd_find_pattern_bmh_3byte_CUDA(const char* pHay, xsimd_int64 nBufferSize,
                                                const char* pNeedle, xsimd_int64 nOffset)
{
    if (nBufferSize < 3) return -1;
    DevBuf dHay, dNeedle, dRes;
    if (!dHay.alloc((size_t)nBufferSize)) return XSIMD_CUDA_FAIL;
    if (!dNeedle.alloc(3)) return XSIMD_CUDA_FAIL;
    if (!dRes.alloc(sizeof(unsigned long long))) return XSIMD_CUDA_FAIL;
    unsigned long long init = (unsigned long long)nBufferSize;
    dHay.upload(pHay, (size_t)nBufferSize);
    dNeedle.upload(pNeedle, 3);
    dRes.upload(&init, sizeof(init));
    k_find_pattern_3byte<<<cuda_blocks(nBufferSize - 2, CUDA_THREADS), CUDA_THREADS>>>(
        (const char*)dHay.p, nBufferSize, (const char*)dNeedle.p, (unsigned long long*)dRes.p);
    unsigned long long result;
    dRes.download(&result, sizeof(result));
    return (result >= (unsigned long long)nBufferSize) ? (xsimd_int64)-1 : nOffset + (xsimd_int64)result;
}

/* ── find_ansi ───────────────────────────────────────────────────────────── */
xsimd_int64 _xsimd_find_ansi_CUDA(const unsigned char* pData, xsimd_int64 nBufferSize,
                                   xsimd_int64 nMinLength, xsimd_int64 nOffset)
{
    if (nBufferSize < nMinLength) return -1;
    DevBuf dData, dRes;
    if (!dData.alloc((size_t)nBufferSize)) return XSIMD_CUDA_FAIL;
    if (!dRes.alloc(sizeof(unsigned long long))) return XSIMD_CUDA_FAIL;
    unsigned long long init = (unsigned long long)nBufferSize;
    dData.upload(pData, (size_t)nBufferSize);
    dRes.upload(&init, sizeof(init));
    long long nSearch = nBufferSize - nMinLength + 1;
    k_find_ansi<<<cuda_blocks(nSearch, CUDA_THREADS), CUDA_THREADS>>>(
        (const unsigned char*)dData.p, nBufferSize, nMinLength, (unsigned long long*)dRes.p);
    unsigned long long result;
    dRes.download(&result, sizeof(result));
    return (result >= (unsigned long long)nBufferSize) ? (xsimd_int64)-1 : nOffset + (xsimd_int64)result;
}

/* ── find_notnull ────────────────────────────────────────────────────────── */
/* Returns -1 (not found) or -(nOffset+pos)-2 (found), matching AVX2 protocol */
xsimd_int64 _xsimd_find_notnull_CUDA(const unsigned char* pData, xsimd_int64 nBufferSize,
                                     xsimd_int64 nMinLength, xsimd_int64 nOffset,
                                     xsimd_int64 j, xsimd_int64 runStart)
{
    (void)j; (void)runStart;
    if (nBufferSize < nMinLength) return -1;
    DevBuf dData, dRes;
    if (!dData.alloc((size_t)nBufferSize)) return XSIMD_CUDA_FAIL;
    if (!dRes.alloc(sizeof(unsigned long long))) return XSIMD_CUDA_FAIL;
    unsigned long long init = (unsigned long long)nBufferSize;
    dData.upload(pData, (size_t)nBufferSize);
    dRes.upload(&init, sizeof(init));
    long long nSearch = nBufferSize - nMinLength + 1;
    k_find_notnull<<<cuda_blocks(nSearch, CUDA_THREADS), CUDA_THREADS>>>(
        (const unsigned char*)dData.p, nBufferSize, nMinLength, (unsigned long long*)dRes.p);
    unsigned long long result;
    dRes.download(&result, sizeof(result));
    if (result >= (unsigned long long)nBufferSize) return -1;
    return -(nOffset + (xsimd_int64)result) - 2;
}

/* ── find_not_ansi ───────────────────────────────────────────────────────── */
xsimd_int64 _xsimd_find_not_ansi_CUDA(const unsigned char* pData, xsimd_int64 nBufferSize,
                                      xsimd_int64 nMinLength, xsimd_int64 nOffset)
{
    if (nBufferSize < nMinLength) return -1;
    DevBuf dData, dRes;
    if (!dData.alloc((size_t)nBufferSize)) return XSIMD_CUDA_FAIL;
    if (!dRes.alloc(sizeof(unsigned long long))) return XSIMD_CUDA_FAIL;
    unsigned long long init = (unsigned long long)nBufferSize;
    dData.upload(pData, (size_t)nBufferSize);
    dRes.upload(&init, sizeof(init));
    long long nSearch = nBufferSize - nMinLength + 1;
    k_find_not_ansi<<<cuda_blocks(nSearch, CUDA_THREADS), CUDA_THREADS>>>(
        (const unsigned char*)dData.p, nBufferSize, nMinLength, (unsigned long long*)dRes.p);
    unsigned long long result;
    dRes.download(&result, sizeof(result));
    return (result >= (unsigned long long)nBufferSize) ? (xsimd_int64)-1 : nOffset + (xsimd_int64)result;
}

/* ── is_not_null ─────────────────────────────────────────────────────────── */
int _xsimd_is_not_null_CUDA(const char* ptr, xsimd_int64 nSize)
{
    DevBuf dData, dRes;
    if (!dData.alloc((size_t)nSize)) return -1;
    if (!dRes.alloc(sizeof(int))) return -1;
    int init = 1;
    dData.upload(ptr, (size_t)nSize);
    dRes.upload(&init, sizeof(init));
    k_is_not_null<<<cuda_blocks(nSize, CUDA_THREADS), CUDA_THREADS>>>(
        (const char*)dData.p, nSize, (int*)dRes.p);
    int result;
    dRes.download(&result, sizeof(result));
    return result;
}

/* ── is_ansi ─────────────────────────────────────────────────────────────── */
int _xsimd_is_ansi_CUDA(const char* ptr, xsimd_int64 nSize)
{
    DevBuf dData, dRes;
    if (!dData.alloc((size_t)nSize)) return -1;
    if (!dRes.alloc(sizeof(int))) return -1;
    int init = 1;
    dData.upload(ptr, (size_t)nSize);
    dRes.upload(&init, sizeof(init));
    k_is_ansi<<<cuda_blocks(nSize, CUDA_THREADS), CUDA_THREADS>>>(
        (const unsigned char*)dData.p, nSize, (int*)dRes.p);
    int result;
    dRes.download(&result, sizeof(result));
    return result;
}

/* ── is_not_ansi ─────────────────────────────────────────────────────────── */
int _xsimd_is_not_ansi_CUDA(const char* ptr, xsimd_int64 nSize)
{
    DevBuf dData, dRes;
    if (!dData.alloc((size_t)nSize)) return -1;
    if (!dRes.alloc(sizeof(int))) return -1;
    int init = 1;
    dData.upload(ptr, (size_t)nSize);
    dRes.upload(&init, sizeof(init));
    k_is_not_ansi<<<cuda_blocks(nSize, CUDA_THREADS), CUDA_THREADS>>>(
        (const unsigned char*)dData.p, nSize, (int*)dRes.p);
    int result;
    dRes.download(&result, sizeof(result));
    return result;
}

/* ── compare_memory ──────────────────────────────────────────────────────── */
int _xsimd_compare_memory_CUDA(const char* ptr1, const char* ptr2, xsimd_int64 nSize)
{
    DevBuf dA, dB, dRes;
    if (!dA.alloc((size_t)nSize)) return -1;
    if (!dB.alloc((size_t)nSize)) return -1;
    if (!dRes.alloc(sizeof(int))) return -1;
    int init = 1;
    dA.upload(ptr1, (size_t)nSize);
    dB.upload(ptr2, (size_t)nSize);
    dRes.upload(&init, sizeof(init));
    k_compare_memory<<<cuda_blocks(nSize, CUDA_THREADS), CUDA_THREADS>>>(
        (const char*)dA.p, (const char*)dB.p, nSize, (int*)dRes.p);
    int result;
    dRes.download(&result, sizeof(result));
    return result;
}

/* ── find_not_ansi_and_null ──────────────────────────────────────────────── */
xsimd_int64 _xsimd_find_not_ansi_and_null_CUDA(const unsigned char* hay, xsimd_int64 hayLen,
                                               xsimd_int64 nOffset, xsimd_int64 m,
                                               xsimd_int64 limit, const int* ansiTable,
                                               xsimd_int64* pj)
{
    (void)limit; (void)ansiTable;
    if (hayLen < m) { *pj = hayLen; return -1; }
    DevBuf dData, dRes;
    if (!dData.alloc((size_t)hayLen)) { *pj = 0; return XSIMD_CUDA_FAIL; }
    if (!dRes.alloc(sizeof(unsigned long long))) { *pj = 0; return XSIMD_CUDA_FAIL; }
    unsigned long long init = (unsigned long long)hayLen;
    dData.upload(hay, (size_t)hayLen);
    dRes.upload(&init, sizeof(init));
    long long nSearch = hayLen - m + 1;
    k_find_not_ansi_and_null<<<cuda_blocks(nSearch, CUDA_THREADS), CUDA_THREADS>>>(
        (const unsigned char*)dData.p, hayLen, m, (unsigned long long*)dRes.p);
    unsigned long long result;
    dRes.download(&result, sizeof(result));
    *pj = hayLen;
    if (result >= (unsigned long long)hayLen) return -1;
    return nOffset + (xsimd_int64)result;
}

/* ── is_not_ansi_and_null ────────────────────────────────────────────────── */
int _xsimd_is_not_ansi_and_null_CUDA(const char* ptr, xsimd_int64 nSize)
{
    DevBuf dData, dRes;
    if (!dData.alloc((size_t)nSize)) return -1;
    if (!dRes.alloc(sizeof(int))) return -1;
    int init = 1;
    dData.upload(ptr, (size_t)nSize);
    dRes.upload(&init, sizeof(init));
    k_is_not_ansi_and_null<<<cuda_blocks(nSize, CUDA_THREADS), CUDA_THREADS>>>(
        (const unsigned char*)dData.p, nSize, (int*)dRes.p);
    int result;
    dRes.download(&result, sizeof(result));
    return result;
}

/* ── find_ansi_number ────────────────────────────────────────────────────── */
xsimd_int64 _xsimd_find_ansi_number_CUDA(const unsigned char* hay, xsimd_int64 hayLen,
                                         xsimd_int64 nOffset, xsimd_int64 m,
                                         xsimd_int64 limit, xsimd_int64* pj)
{
    (void)limit;
    if (hayLen < m) { *pj = hayLen; return -1; }
    DevBuf dData, dRes;
    if (!dData.alloc((size_t)hayLen)) { *pj = 0; return XSIMD_CUDA_FAIL; }
    if (!dRes.alloc(sizeof(unsigned long long))) { *pj = 0; return XSIMD_CUDA_FAIL; }
    unsigned long long init = (unsigned long long)hayLen;
    dData.upload(hay, (size_t)hayLen);
    dRes.upload(&init, sizeof(init));
    long long nSearch = hayLen - m + 1;
    k_find_ansi_number<<<cuda_blocks(nSearch, CUDA_THREADS), CUDA_THREADS>>>(
        (const unsigned char*)dData.p, hayLen, m, (unsigned long long*)dRes.p);
    unsigned long long result;
    dRes.download(&result, sizeof(result));
    *pj = hayLen;
    if (result >= (unsigned long long)hayLen) return -1;
    return nOffset + (xsimd_int64)result;
}

/* ── is_ansi_number ──────────────────────────────────────────────────────── */
int _xsimd_is_ansi_number_CUDA(const char* ptr, xsimd_int64 nSize)
{
    DevBuf dData, dRes;
    if (!dData.alloc((size_t)nSize)) return -1;
    if (!dRes.alloc(sizeof(int))) return -1;
    int init = 1;
    dData.upload(ptr, (size_t)nSize);
    dRes.upload(&init, sizeof(init));
    k_is_ansi_number<<<cuda_blocks(nSize, CUDA_THREADS), CUDA_THREADS>>>(
        (const unsigned char*)dData.p, nSize, (int*)dRes.p);
    int result;
    dRes.download(&result, sizeof(result));
    return result;
}

/* ── find_first_non_ansi ─────────────────────────────────────────────────── */
xsimd_int64 _xsimd_find_first_non_ansi_CUDA(const xsimd_uint8* pData, xsimd_int64 startI,
                                            xsimd_int64 nSize)
{
    long long remaining = nSize - startI;
    if (remaining <= 0) return -1;
    DevBuf dData, dRes;
    if (!dData.alloc((size_t)remaining)) return XSIMD_CUDA_FAIL;
    if (!dRes.alloc(sizeof(unsigned long long))) return XSIMD_CUDA_FAIL;
    unsigned long long init = (unsigned long long)remaining;
    dData.upload(pData + startI, (size_t)remaining);
    dRes.upload(&init, sizeof(init));
    k_find_first_non_ansi<<<cuda_blocks(remaining, CUDA_THREADS), CUDA_THREADS>>>(
        (const unsigned char*)dData.p, remaining, (unsigned long long*)dRes.p);
    unsigned long long result;
    dRes.download(&result, sizeof(result));
    return (result >= (unsigned long long)remaining) ? (xsimd_int64)-1 : startI + (xsimd_int64)result;
}

/* ── find_null_byte ──────────────────────────────────────────────────────── */
xsimd_int64 _xsimd_find_null_byte_CUDA(const xsimd_uint8* pData, xsimd_int64 nSize,
                                       xsimd_int64* pi)
{
    DevBuf dData, dRes;
    if (!dData.alloc((size_t)nSize)) return XSIMD_CUDA_FAIL;
    if (!dRes.alloc(sizeof(unsigned long long))) return XSIMD_CUDA_FAIL;
    unsigned long long init = (unsigned long long)nSize;
    dData.upload(pData, (size_t)nSize);
    dRes.upload(&init, sizeof(init));
    k_find_null_byte<<<cuda_blocks(nSize, CUDA_THREADS), CUDA_THREADS>>>(
        (const unsigned char*)dData.p, nSize, (unsigned long long*)dRes.p);
    unsigned long long result;
    dRes.download(&result, sizeof(result));
    *pi = nSize;
    return (result >= (unsigned long long)nSize) ? (xsimd_int64)-1 : (xsimd_int64)result;
}

/* ── count_unicode_prefix ────────────────────────────────────────────────── */
xsimd_int64 _xsimd_count_unicode_prefix_CUDA(const xsimd_uint16* pData, xsimd_int64 nChars,
                                             xsimd_int64* pi)
{
    size_t bytes = (size_t)nChars * 2;
    DevBuf dData, dRes;
    if (!dData.alloc(bytes)) return XSIMD_CUDA_FAIL;
    if (!dRes.alloc(sizeof(unsigned long long))) return XSIMD_CUDA_FAIL;
    unsigned long long init = (unsigned long long)nChars;
    dData.upload(pData, bytes);
    dRes.upload(&init, sizeof(init));
    k_count_unicode_prefix<<<cuda_blocks(nChars, CUDA_THREADS), CUDA_THREADS>>>(
        (const unsigned short*)dData.p, nChars, (unsigned long long*)dRes.p);
    unsigned long long result;
    dRes.download(&result, sizeof(result));
    *pi = nChars;
    if (result >= (unsigned long long)nChars) return -1;  /* all valid */
    return (xsimd_int64)result * 2;  /* byte offset of first invalid char */
}

/* ── count_char ──────────────────────────────────────────────────────────── */
void _xsimd_count_char_CUDA(const xsimd_uint8* pData, xsimd_int64 nSize, xsimd_uint8 nByte,
                            xsimd_int64* pi, xsimd_int64* pnCount)
{
    DevBuf dData, dCnt;
    if (!dData.alloc((size_t)nSize) || !dCnt.alloc(sizeof(unsigned long long))) return;
    unsigned long long init = 0;
    dData.upload(pData, (size_t)nSize);
    dCnt.upload(&init, sizeof(init));
    k_count_char<<<cuda_blocks(nSize, CUDA_THREADS), CUDA_THREADS>>>(
        (const unsigned char*)dData.p, nSize, nByte, (unsigned long long*)dCnt.p);
    unsigned long long cnt;
    dCnt.download(&cnt, sizeof(cnt));
    *pi = nSize;
    *pnCount += (xsimd_int64)cnt;
}

/* ── create_ansi_mask ────────────────────────────────────────────────────── */
void _xsimd_create_ansi_mask_CUDA(const xsimd_uint8* pData, xsimd_int64 nSize,
                                  xsimd_uint8* pMaskData, xsimd_int64* pi,
                                  xsimd_int64* pnAnsiCount)
{
    long long maskBytes = (nSize + 7) / 8;
    DevBuf dData, dMask, dCnt;
    if (!dData.alloc((size_t)nSize) || !dMask.alloc((size_t)maskBytes) ||
        !dCnt.alloc(sizeof(unsigned long long))) return;
    unsigned long long init = 0;
    dData.upload(pData, (size_t)nSize);
    cudaMemset(dMask.p, 0, (size_t)maskBytes);
    dCnt.upload(&init, sizeof(init));
    k_create_ansi_mask<<<cuda_blocks(maskBytes, CUDA_THREADS), CUDA_THREADS>>>(
        (const unsigned char*)dData.p, nSize,
        (unsigned char*)dMask.p, (unsigned long long*)dCnt.p);
    dMask.download(pMaskData, (size_t)maskBytes);
    unsigned long long cnt;
    dCnt.download(&cnt, sizeof(cnt));
    *pi = nSize;
    *pnAnsiCount += (xsimd_int64)cnt;
}

/* ── compare_sigbytes ────────────────────────────────────────────────────── */
int _xsimd_compare_sigbytes_CUDA(const xsimd_uint8* pSigBytes, xsimd_int64 nSigBytesSize,
                                 const xsimd_uint8* pData, xsimd_int64 nDataSize,
                                 const xsimd_uint8* pAlphaNumTable)
{
    long long nPairs = nSigBytesSize / 2;
    DevBuf dSig, dData, dAlpha, dRes;
    if (!dSig.alloc((size_t)nSigBytesSize) || !dData.alloc((size_t)nDataSize) ||
        !dAlpha.alloc(256) || !dRes.alloc(sizeof(int))) return -1;
    int init = 1;
    dSig.upload(pSigBytes, (size_t)nSigBytesSize);
    dData.upload(pData, (size_t)nDataSize);
    dAlpha.upload(pAlphaNumTable, 256);
    dRes.upload(&init, sizeof(init));
    k_compare_sigbytes<<<cuda_blocks(nPairs, CUDA_THREADS), CUDA_THREADS>>>(
        (const unsigned char*)dSig.p, nPairs,
        (const unsigned char*)dData.p, (const unsigned char*)dAlpha.p, (int*)dRes.p);
    int result;
    dRes.download(&result, sizeof(result));
    return result;
}

/* ── find_sigbytes ───────────────────────────────────────────────────────── */
xsimd_int64 _xsimd_find_sigbytes_CUDA(const xsimd_uint8* pData, xsimd_int64 nDataSize,
                                      const xsimd_uint8* pSigBytes, xsimd_int64 nSigBytesSize,
                                      const xsimd_uint8* pAlphaNumTable)
{
    long long nPairs = nSigBytesSize / 2;
    if (nDataSize < nPairs) return -1;
    DevBuf dData, dSig, dAlpha, dRes;
    if (!dData.alloc((size_t)nDataSize) || !dSig.alloc((size_t)nSigBytesSize) ||
        !dAlpha.alloc(256) || !dRes.alloc(sizeof(unsigned long long))) return XSIMD_CUDA_FAIL;
    unsigned long long init = (unsigned long long)nDataSize;
    dData.upload(pData, (size_t)nDataSize);
    dSig.upload(pSigBytes, (size_t)nSigBytesSize);
    dAlpha.upload(pAlphaNumTable, 256);
    dRes.upload(&init, sizeof(init));
    long long nSearch = nDataSize - nPairs + 1;
    k_find_sigbytes<<<cuda_blocks(nSearch, CUDA_THREADS), CUDA_THREADS>>>(
        (const unsigned char*)dData.p, nDataSize,
        (const unsigned char*)dSig.p, nPairs,
        (const unsigned char*)dAlpha.p, (unsigned long long*)dRes.p);
    unsigned long long result;
    dRes.download(&result, sizeof(result));
    return (result >= (unsigned long long)nDataSize) ? (xsimd_int64)-1 : (xsimd_int64)result;
}

} /* extern "C" */
