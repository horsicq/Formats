/* Copyright (c) 2020-2026 hors<horsicq@gmail.com>
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
#include "xdataconvertor.h"

#include <algorithm>
#include <cstring>

namespace {

// Operation groups used by the 1:1 "map" converter. The concrete method
// (e.g. CMETHOD_XOR_WORD) is decomposed into an operation + a byte width so a
// single loop can serve every XOR/ADD/SUB/NOT/ROL/ROR/BSWAP variant.
enum MAPOP {
    MAPOP_XOR = 0,
    MAPOP_ADD,
    MAPOP_SUB,
    MAPOP_NOT,
    MAPOP_ROL,
    MAPOP_ROR,
    MAPOP_BSWAP,
    MAPOP_NEG,
    MAPOP_BITREVERSE,
    MAPOP_NIBBLESWAP,  // byte width only
    MAPOP_SHL,
    MAPOP_SHR,
    MAPOP_ROT13,      // byte width only
    MAPOP_ROT47,      // byte width only
    MAPOP_UPPERCASE,  // byte width only
    MAPOP_LOWERCASE,  // byte width only
    MAPOP_SWAPCASE,   // byte width only
    MAPOP_ATBASH,     // byte width only
    MAPOP_ROT5,       // byte width only
    MAPOP_ROT18,      // byte width only
    MAPOP_EBCDIC2ASCII,  // byte width only
    MAPOP_ASCII2EBCDIC   // byte width only
};

// IBM Code Page 037 (US/Canada) EBCDIC <-> Latin-1/ASCII, from the Unicode
// Consortium mapping (CP037.TXT). Verified a full bijection with all values <= 0xFF.
static const unsigned char EBCDIC_TO_ASCII_TABLE[256] = {
    0x00, 0x01, 0x02, 0x03, 0x9C, 0x09, 0x86, 0x7F, 0x97, 0x8D, 0x8E, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,  //
    0x10, 0x11, 0x12, 0x13, 0x9D, 0x85, 0x08, 0x87, 0x18, 0x19, 0x92, 0x8F, 0x1C, 0x1D, 0x1E, 0x1F,  //
    0x80, 0x81, 0x82, 0x83, 0x84, 0x0A, 0x17, 0x1B, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x05, 0x06, 0x07,  //
    0x90, 0x91, 0x16, 0x93, 0x94, 0x95, 0x96, 0x04, 0x98, 0x99, 0x9A, 0x9B, 0x14, 0x15, 0x9E, 0x1A,  //
    0x20, 0xA0, 0xE2, 0xE4, 0xE0, 0xE1, 0xE3, 0xE5, 0xE7, 0xF1, 0xA2, 0x2E, 0x3C, 0x28, 0x2B, 0x7C,  //
    0x26, 0xE9, 0xEA, 0xEB, 0xE8, 0xED, 0xEE, 0xEF, 0xEC, 0xDF, 0x21, 0x24, 0x2A, 0x29, 0x3B, 0xAC,  //
    0x2D, 0x2F, 0xC2, 0xC4, 0xC0, 0xC1, 0xC3, 0xC5, 0xC7, 0xD1, 0xA6, 0x2C, 0x25, 0x5F, 0x3E, 0x3F,  //
    0xF8, 0xC9, 0xCA, 0xCB, 0xC8, 0xCD, 0xCE, 0xCF, 0xCC, 0x60, 0x3A, 0x23, 0x40, 0x27, 0x3D, 0x22,  //
    0xD8, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0xAB, 0xBB, 0xF0, 0xFD, 0xFE, 0xB1,  //
    0xB0, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0xAA, 0xBA, 0xE6, 0xB8, 0xC6, 0xA4,  //
    0xB5, 0x7E, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0xA1, 0xBF, 0xD0, 0xDD, 0xDE, 0xAE,  //
    0x5E, 0xA3, 0xA5, 0xB7, 0xA9, 0xA7, 0xB6, 0xBC, 0xBD, 0xBE, 0x5B, 0x5D, 0xAF, 0xA8, 0xB4, 0xD7,  //
    0x7B, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0xAD, 0xF4, 0xF6, 0xF2, 0xF3, 0xF5,  //
    0x7D, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0xB9, 0xFB, 0xFC, 0xF9, 0xFA, 0xFF,  //
    0x5C, 0xF7, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0xB2, 0xD4, 0xD6, 0xD2, 0xD3, 0xD5,  //
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0xB3, 0xDB, 0xDC, 0xD9, 0xDA, 0x9F   //
};

static const unsigned char ASCII_TO_EBCDIC_TABLE[256] = {
    0x00, 0x01, 0x02, 0x03, 0x37, 0x2D, 0x2E, 0x2F, 0x16, 0x05, 0x25, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,  //
    0x10, 0x11, 0x12, 0x13, 0x3C, 0x3D, 0x32, 0x26, 0x18, 0x19, 0x3F, 0x27, 0x1C, 0x1D, 0x1E, 0x1F,  //
    0x40, 0x5A, 0x7F, 0x7B, 0x5B, 0x6C, 0x50, 0x7D, 0x4D, 0x5D, 0x5C, 0x4E, 0x6B, 0x60, 0x4B, 0x61,  //
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0x7A, 0x5E, 0x4C, 0x7E, 0x6E, 0x6F,  //
    0x7C, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6,  //
    0xD7, 0xD8, 0xD9, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xBA, 0xE0, 0xBB, 0xB0, 0x6D,  //
    0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,  //
    0x97, 0x98, 0x99, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xC0, 0x4F, 0xD0, 0xA1, 0x07,  //
    0x20, 0x21, 0x22, 0x23, 0x24, 0x15, 0x06, 0x17, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x09, 0x0A, 0x1B,  //
    0x30, 0x31, 0x1A, 0x33, 0x34, 0x35, 0x36, 0x08, 0x38, 0x39, 0x3A, 0x3B, 0x04, 0x14, 0x3E, 0xFF,  //
    0x41, 0xAA, 0x4A, 0xB1, 0x9F, 0xB2, 0x6A, 0xB5, 0xBD, 0xB4, 0x9A, 0x8A, 0x5F, 0xCA, 0xAF, 0xBC,  //
    0x90, 0x8F, 0xEA, 0xFA, 0xBE, 0xA0, 0xB6, 0xB3, 0x9D, 0xDA, 0x9B, 0x8B, 0xB7, 0xB8, 0xB9, 0xAB,  //
    0x64, 0x65, 0x62, 0x66, 0x63, 0x67, 0x9E, 0x68, 0x74, 0x71, 0x72, 0x73, 0x78, 0x75, 0x76, 0x77,  //
    0xAC, 0x69, 0xED, 0xEE, 0xEB, 0xEF, 0xEC, 0xBF, 0x80, 0xFD, 0xFE, 0xFB, 0xFC, 0xAD, 0xAE, 0x59,  //
    0x44, 0x45, 0x42, 0x46, 0x43, 0x47, 0x9C, 0x48, 0x54, 0x51, 0x52, 0x53, 0x58, 0x55, 0x56, 0x57,  //
    0x8C, 0x49, 0xCD, 0xCE, 0xCB, 0xCF, 0xCC, 0xE1, 0x70, 0xDD, 0xDE, 0xDB, 0xDC, 0x8D, 0x8E, 0xDF   //
};

inline quint8 upperByte(quint8 nValue)
{
    return ((nValue >= 'a') && (nValue <= 'z')) ? (quint8)(nValue - 32) : nValue;
}

inline quint8 lowerByte(quint8 nValue)
{
    return ((nValue >= 'A') && (nValue <= 'Z')) ? (quint8)(nValue + 32) : nValue;
}

inline quint8 swapCaseByte(quint8 nValue)
{
    if ((nValue >= 'A') && (nValue <= 'Z')) {
        return (quint8)(nValue + 32);
    } else if ((nValue >= 'a') && (nValue <= 'z')) {
        return (quint8)(nValue - 32);
    }
    return nValue;
}

inline quint8 atbashByte(quint8 nValue)
{
    if ((nValue >= 'A') && (nValue <= 'Z')) {
        return (quint8)('Z' - (nValue - 'A'));
    } else if ((nValue >= 'a') && (nValue <= 'z')) {
        return (quint8)('z' - (nValue - 'a'));
    }
    return nValue;
}

inline quint8 rot5Byte(quint8 nValue)
{
    if ((nValue >= '0') && (nValue <= '9')) {
        return (quint8)('0' + ((nValue - '0' + 5) % 10));
    }
    return nValue;
}

inline quint8 rot18Byte(quint8 nValue)
{
    // ROT13 on letters + ROT5 on digits (rot13Byte is defined later, so inline it).
    quint8 nRotated = nValue;
    if ((nValue >= 'A') && (nValue <= 'Z')) {
        nRotated = (quint8)('A' + ((nValue - 'A' + 13) % 26));
    } else if ((nValue >= 'a') && (nValue <= 'z')) {
        nRotated = (quint8)('a' + ((nValue - 'a' + 13) % 26));
    }
    return rot5Byte(nRotated);
}

// Width-sized access through a char buffer via memcpy. Using reinterpret casts
// (e.g. ((quint16 *)pBuffer)[j]) would be a strict-aliasing violation; memcpy is
// the portable, optimiser-friendly equivalent and compiles to the same load/store.
inline quint16 readU16(const char *pData)
{
    quint16 nValue = 0;
    memcpy(&nValue, pData, sizeof(nValue));
    return nValue;
}

inline quint32 readU32(const char *pData)
{
    quint32 nValue = 0;
    memcpy(&nValue, pData, sizeof(nValue));
    return nValue;
}

inline quint64 readU64(const char *pData)
{
    quint64 nValue = 0;
    memcpy(&nValue, pData, sizeof(nValue));
    return nValue;
}

inline void writeU16(char *pData, quint16 nValue)
{
    memcpy(pData, &nValue, sizeof(nValue));
}

inline void writeU32(char *pData, quint32 nValue)
{
    memcpy(pData, &nValue, sizeof(nValue));
}

inline void writeU64(char *pData, quint64 nValue)
{
    memcpy(pData, &nValue, sizeof(nValue));
}

// Bit rotates. The "n ? ... : v" guard avoids shift-by-width undefined behaviour
// when the rotate amount is a multiple of the type width.
inline quint8 rotl8(quint8 nValue, quint32 nCount)
{
    nCount &= 7;
    return nCount ? (quint8)((nValue << nCount) | (nValue >> (8 - nCount))) : nValue;
}

inline quint8 rotr8(quint8 nValue, quint32 nCount)
{
    nCount &= 7;
    return nCount ? (quint8)((nValue >> nCount) | (nValue << (8 - nCount))) : nValue;
}

inline quint16 rotl16(quint16 nValue, quint32 nCount)
{
    nCount &= 15;
    return nCount ? (quint16)((nValue << nCount) | (nValue >> (16 - nCount))) : nValue;
}

inline quint16 rotr16(quint16 nValue, quint32 nCount)
{
    nCount &= 15;
    return nCount ? (quint16)((nValue >> nCount) | (nValue << (16 - nCount))) : nValue;
}

inline quint32 rotl32(quint32 nValue, quint32 nCount)
{
    nCount &= 31;
    return nCount ? ((nValue << nCount) | (nValue >> (32 - nCount))) : nValue;
}

inline quint32 rotr32(quint32 nValue, quint32 nCount)
{
    nCount &= 31;
    return nCount ? ((nValue >> nCount) | (nValue << (32 - nCount))) : nValue;
}

inline quint64 rotl64(quint64 nValue, quint32 nCount)
{
    nCount &= 63;
    return nCount ? ((nValue << nCount) | (nValue >> (64 - nCount))) : nValue;
}

inline quint64 rotr64(quint64 nValue, quint32 nCount)
{
    nCount &= 63;
    return nCount ? ((nValue >> nCount) | (nValue << (64 - nCount))) : nValue;
}

// Byte-order swaps (endianness reverse of one unit). Implemented directly so the
// code does not depend on a particular Qt qbswap overload set.
inline quint16 byteSwap16(quint16 nValue)
{
    return (quint16)((nValue >> 8) | (nValue << 8));
}

inline quint32 byteSwap32(quint32 nValue)
{
    return ((nValue & 0x000000FF) << 24) | ((nValue & 0x0000FF00) << 8) | ((nValue & 0x00FF0000) >> 8) | ((nValue & 0xFF000000) >> 24);
}

inline quint64 byteSwap64(quint64 nValue)
{
    return ((quint64)byteSwap32((quint32)nValue) << 32) | (quint64)byteSwap32((quint32)(nValue >> 32));
}

// Bit reversal within a unit.
inline quint8 bitReverse8(quint8 nValue)
{
    nValue = (quint8)((nValue >> 4) | (nValue << 4));
    nValue = (quint8)(((nValue & 0xCC) >> 2) | ((nValue & 0x33) << 2));
    nValue = (quint8)(((nValue & 0xAA) >> 1) | ((nValue & 0x55) << 1));
    return nValue;
}

inline quint16 bitReverse16(quint16 nValue)
{
    nValue = (quint16)((nValue >> 8) | (nValue << 8));
    nValue = (quint16)(((nValue & 0xF0F0) >> 4) | ((nValue & 0x0F0F) << 4));
    nValue = (quint16)(((nValue & 0xCCCC) >> 2) | ((nValue & 0x3333) << 2));
    nValue = (quint16)(((nValue & 0xAAAA) >> 1) | ((nValue & 0x5555) << 1));
    return nValue;
}

inline quint32 bitReverse32(quint32 nValue)
{
    nValue = (nValue >> 16) | (nValue << 16);
    nValue = ((nValue & 0xFF00FF00) >> 8) | ((nValue & 0x00FF00FF) << 8);
    nValue = ((nValue & 0xF0F0F0F0) >> 4) | ((nValue & 0x0F0F0F0F) << 4);
    nValue = ((nValue & 0xCCCCCCCC) >> 2) | ((nValue & 0x33333333) << 2);
    nValue = ((nValue & 0xAAAAAAAA) >> 1) | ((nValue & 0x55555555) << 1);
    return nValue;
}

inline quint64 bitReverse64(quint64 nValue)
{
    // Reverse each 32-bit half and swap the halves.
    return ((quint64)bitReverse32((quint32)nValue) << 32) | (quint64)bitReverse32((quint32)(nValue >> 32));
}

// Logical shift by nCount; a shift of >= the unit width yields 0 (well-defined here,
// unlike the C shift operator which would be undefined).
inline quint64 shiftLeft(quint64 nValue, quint32 nCount, qint32 nBits)
{
    return (nCount >= (quint32)nBits) ? 0 : (nValue << nCount);
}

inline quint64 shiftRight(quint64 nValue, quint32 nCount, qint32 nBits)
{
    return (nCount >= (quint32)nBits) ? 0 : (nValue >> nCount);
}

inline quint8 rot13Byte(quint8 nValue)
{
    if ((nValue >= 'A') && (nValue <= 'Z')) {
        return (quint8)('A' + ((nValue - 'A' + 13) % 26));
    } else if ((nValue >= 'a') && (nValue <= 'z')) {
        return (quint8)('a' + ((nValue - 'a' + 13) % 26));
    }
    return nValue;
}

inline quint8 rot47Byte(quint8 nValue)
{
    if ((nValue >= '!') && (nValue <= '~')) {
        return (quint8)('!' + ((nValue - '!' + 47) % 94));
    }
    return nValue;
}

// Decompose a method into MAPOP + width. Returns false for methods that are not
// handled by the 1:1 map converter.
bool decomposeMapMethod(XDataConvertor::CMETHOD method, MAPOP *pOp, qint32 *pWidth)
{
    bool bResult = true;

    if (method == XDataConvertor::CMETHOD_XOR_BYTE) {
        *pOp = MAPOP_XOR;
        *pWidth = 1;
    } else if (method == XDataConvertor::CMETHOD_XOR_WORD) {
        *pOp = MAPOP_XOR;
        *pWidth = 2;
    } else if (method == XDataConvertor::CMETHOD_XOR_DWORD) {
        *pOp = MAPOP_XOR;
        *pWidth = 4;
    } else if (method == XDataConvertor::CMETHOD_XOR_QWORD) {
        *pOp = MAPOP_XOR;
        *pWidth = 8;
    } else if (method == XDataConvertor::CMETHOD_ADD_BYTE) {
        *pOp = MAPOP_ADD;
        *pWidth = 1;
    } else if (method == XDataConvertor::CMETHOD_ADD_WORD) {
        *pOp = MAPOP_ADD;
        *pWidth = 2;
    } else if (method == XDataConvertor::CMETHOD_ADD_DWORD) {
        *pOp = MAPOP_ADD;
        *pWidth = 4;
    } else if (method == XDataConvertor::CMETHOD_ADD_QWORD) {
        *pOp = MAPOP_ADD;
        *pWidth = 8;
    } else if (method == XDataConvertor::CMETHOD_SUB_BYTE) {
        *pOp = MAPOP_SUB;
        *pWidth = 1;
    } else if (method == XDataConvertor::CMETHOD_SUB_WORD) {
        *pOp = MAPOP_SUB;
        *pWidth = 2;
    } else if (method == XDataConvertor::CMETHOD_SUB_DWORD) {
        *pOp = MAPOP_SUB;
        *pWidth = 4;
    } else if (method == XDataConvertor::CMETHOD_SUB_QWORD) {
        *pOp = MAPOP_SUB;
        *pWidth = 8;
    } else if (method == XDataConvertor::CMETHOD_NOT) {
        *pOp = MAPOP_NOT;
        *pWidth = 1;
    } else if (method == XDataConvertor::CMETHOD_ROL_BYTE) {
        *pOp = MAPOP_ROL;
        *pWidth = 1;
    } else if (method == XDataConvertor::CMETHOD_ROL_WORD) {
        *pOp = MAPOP_ROL;
        *pWidth = 2;
    } else if (method == XDataConvertor::CMETHOD_ROL_DWORD) {
        *pOp = MAPOP_ROL;
        *pWidth = 4;
    } else if (method == XDataConvertor::CMETHOD_ROL_QWORD) {
        *pOp = MAPOP_ROL;
        *pWidth = 8;
    } else if (method == XDataConvertor::CMETHOD_ROR_BYTE) {
        *pOp = MAPOP_ROR;
        *pWidth = 1;
    } else if (method == XDataConvertor::CMETHOD_ROR_WORD) {
        *pOp = MAPOP_ROR;
        *pWidth = 2;
    } else if (method == XDataConvertor::CMETHOD_ROR_DWORD) {
        *pOp = MAPOP_ROR;
        *pWidth = 4;
    } else if (method == XDataConvertor::CMETHOD_ROR_QWORD) {
        *pOp = MAPOP_ROR;
        *pWidth = 8;
    } else if (method == XDataConvertor::CMETHOD_BSWAP_WORD) {
        *pOp = MAPOP_BSWAP;
        *pWidth = 2;
    } else if (method == XDataConvertor::CMETHOD_BSWAP_DWORD) {
        *pOp = MAPOP_BSWAP;
        *pWidth = 4;
    } else if (method == XDataConvertor::CMETHOD_BSWAP_QWORD) {
        *pOp = MAPOP_BSWAP;
        *pWidth = 8;
    } else if (method == XDataConvertor::CMETHOD_NEG_BYTE) {
        *pOp = MAPOP_NEG;
        *pWidth = 1;
    } else if (method == XDataConvertor::CMETHOD_NEG_WORD) {
        *pOp = MAPOP_NEG;
        *pWidth = 2;
    } else if (method == XDataConvertor::CMETHOD_NEG_DWORD) {
        *pOp = MAPOP_NEG;
        *pWidth = 4;
    } else if (method == XDataConvertor::CMETHOD_NEG_QWORD) {
        *pOp = MAPOP_NEG;
        *pWidth = 8;
    } else if (method == XDataConvertor::CMETHOD_BITREVERSE_BYTE) {
        *pOp = MAPOP_BITREVERSE;
        *pWidth = 1;
    } else if (method == XDataConvertor::CMETHOD_BITREVERSE_WORD) {
        *pOp = MAPOP_BITREVERSE;
        *pWidth = 2;
    } else if (method == XDataConvertor::CMETHOD_BITREVERSE_DWORD) {
        *pOp = MAPOP_BITREVERSE;
        *pWidth = 4;
    } else if (method == XDataConvertor::CMETHOD_BITREVERSE_QWORD) {
        *pOp = MAPOP_BITREVERSE;
        *pWidth = 8;
    } else if (method == XDataConvertor::CMETHOD_NIBBLESWAP) {
        *pOp = MAPOP_NIBBLESWAP;
        *pWidth = 1;
    } else if (method == XDataConvertor::CMETHOD_SHL_BYTE) {
        *pOp = MAPOP_SHL;
        *pWidth = 1;
    } else if (method == XDataConvertor::CMETHOD_SHL_WORD) {
        *pOp = MAPOP_SHL;
        *pWidth = 2;
    } else if (method == XDataConvertor::CMETHOD_SHL_DWORD) {
        *pOp = MAPOP_SHL;
        *pWidth = 4;
    } else if (method == XDataConvertor::CMETHOD_SHL_QWORD) {
        *pOp = MAPOP_SHL;
        *pWidth = 8;
    } else if (method == XDataConvertor::CMETHOD_SHR_BYTE) {
        *pOp = MAPOP_SHR;
        *pWidth = 1;
    } else if (method == XDataConvertor::CMETHOD_SHR_WORD) {
        *pOp = MAPOP_SHR;
        *pWidth = 2;
    } else if (method == XDataConvertor::CMETHOD_SHR_DWORD) {
        *pOp = MAPOP_SHR;
        *pWidth = 4;
    } else if (method == XDataConvertor::CMETHOD_SHR_QWORD) {
        *pOp = MAPOP_SHR;
        *pWidth = 8;
    } else if (method == XDataConvertor::CMETHOD_ROT13) {
        *pOp = MAPOP_ROT13;
        *pWidth = 1;
    } else if (method == XDataConvertor::CMETHOD_ROT47) {
        *pOp = MAPOP_ROT47;
        *pWidth = 1;
    } else if (method == XDataConvertor::CMETHOD_UPPERCASE) {
        *pOp = MAPOP_UPPERCASE;
        *pWidth = 1;
    } else if (method == XDataConvertor::CMETHOD_LOWERCASE) {
        *pOp = MAPOP_LOWERCASE;
        *pWidth = 1;
    } else if (method == XDataConvertor::CMETHOD_SWAPCASE) {
        *pOp = MAPOP_SWAPCASE;
        *pWidth = 1;
    } else if (method == XDataConvertor::CMETHOD_ATBASH) {
        *pOp = MAPOP_ATBASH;
        *pWidth = 1;
    } else if (method == XDataConvertor::CMETHOD_ROT5) {
        *pOp = MAPOP_ROT5;
        *pWidth = 1;
    } else if (method == XDataConvertor::CMETHOD_ROT18) {
        *pOp = MAPOP_ROT18;
        *pWidth = 1;
    } else if (method == XDataConvertor::CMETHOD_EBCDIC_TO_ASCII) {
        *pOp = MAPOP_EBCDIC2ASCII;
        *pWidth = 1;
    } else if (method == XDataConvertor::CMETHOD_ASCII_TO_EBCDIC) {
        *pOp = MAPOP_ASCII2EBCDIC;
        *pWidth = 1;
    } else {
        bResult = false;
    }

    return bResult;
}

// Apply a MAPOP over a width-aligned chunk in place. nSize is guaranteed to be a
// multiple of nWidth by the caller.
void applyMap(char *pData, qint64 nSize, MAPOP op, qint32 nWidth, quint64 nKey)
{
    if (nWidth == 1) {
        quint8 nK = (quint8)nKey;

        for (qint64 j = 0; j < nSize; j++) {
            quint8 nValue = (quint8)pData[j];

            if (op == MAPOP_XOR) {
                nValue = (quint8)(nValue ^ nK);
            } else if (op == MAPOP_ADD) {
                nValue = (quint8)(nValue + nK);
            } else if (op == MAPOP_SUB) {
                nValue = (quint8)(nValue - nK);
            } else if (op == MAPOP_NOT) {
                nValue = (quint8)(~nValue);
            } else if (op == MAPOP_NEG) {
                nValue = (quint8)(0 - nValue);
            } else if (op == MAPOP_ROL) {
                nValue = rotl8(nValue, (quint32)nKey);
            } else if (op == MAPOP_ROR) {
                nValue = rotr8(nValue, (quint32)nKey);
            } else if (op == MAPOP_BITREVERSE) {
                nValue = bitReverse8(nValue);
            } else if (op == MAPOP_NIBBLESWAP) {
                nValue = (quint8)((nValue >> 4) | (nValue << 4));
            } else if (op == MAPOP_SHL) {
                nValue = (quint8)shiftLeft(nValue, (quint32)nKey, 8);
            } else if (op == MAPOP_SHR) {
                nValue = (quint8)shiftRight(nValue, (quint32)nKey, 8);
            } else if (op == MAPOP_ROT13) {
                nValue = rot13Byte(nValue);
            } else if (op == MAPOP_ROT47) {
                nValue = rot47Byte(nValue);
            } else if (op == MAPOP_UPPERCASE) {
                nValue = upperByte(nValue);
            } else if (op == MAPOP_LOWERCASE) {
                nValue = lowerByte(nValue);
            } else if (op == MAPOP_SWAPCASE) {
                nValue = swapCaseByte(nValue);
            } else if (op == MAPOP_ATBASH) {
                nValue = atbashByte(nValue);
            } else if (op == MAPOP_ROT5) {
                nValue = rot5Byte(nValue);
            } else if (op == MAPOP_ROT18) {
                nValue = rot18Byte(nValue);
            } else if (op == MAPOP_EBCDIC2ASCII) {
                nValue = EBCDIC_TO_ASCII_TABLE[nValue];
            } else if (op == MAPOP_ASCII2EBCDIC) {
                nValue = ASCII_TO_EBCDIC_TABLE[nValue];
            }

            pData[j] = (char)nValue;
        }
    } else if (nWidth == 2) {
        quint16 nK = (quint16)nKey;

        for (qint64 j = 0; (j + 2) <= nSize; j += 2) {
            quint16 nValue = readU16(pData + j);

            if (op == MAPOP_XOR) {
                nValue = (quint16)(nValue ^ nK);
            } else if (op == MAPOP_ADD) {
                nValue = (quint16)(nValue + nK);
            } else if (op == MAPOP_SUB) {
                nValue = (quint16)(nValue - nK);
            } else if (op == MAPOP_NOT) {
                nValue = (quint16)(~nValue);
            } else if (op == MAPOP_NEG) {
                nValue = (quint16)(0 - nValue);
            } else if (op == MAPOP_ROL) {
                nValue = rotl16(nValue, (quint32)nKey);
            } else if (op == MAPOP_ROR) {
                nValue = rotr16(nValue, (quint32)nKey);
            } else if (op == MAPOP_BSWAP) {
                nValue = byteSwap16(nValue);
            } else if (op == MAPOP_BITREVERSE) {
                nValue = bitReverse16(nValue);
            } else if (op == MAPOP_SHL) {
                nValue = (quint16)shiftLeft(nValue, (quint32)nKey, 16);
            } else if (op == MAPOP_SHR) {
                nValue = (quint16)shiftRight(nValue, (quint32)nKey, 16);
            }

            writeU16(pData + j, nValue);
        }
    } else if (nWidth == 4) {
        quint32 nK = (quint32)nKey;

        for (qint64 j = 0; (j + 4) <= nSize; j += 4) {
            quint32 nValue = readU32(pData + j);

            if (op == MAPOP_XOR) {
                nValue = nValue ^ nK;
            } else if (op == MAPOP_ADD) {
                nValue = nValue + nK;
            } else if (op == MAPOP_SUB) {
                nValue = nValue - nK;
            } else if (op == MAPOP_NOT) {
                nValue = ~nValue;
            } else if (op == MAPOP_NEG) {
                nValue = (quint32)(0 - nValue);
            } else if (op == MAPOP_ROL) {
                nValue = rotl32(nValue, (quint32)nKey);
            } else if (op == MAPOP_ROR) {
                nValue = rotr32(nValue, (quint32)nKey);
            } else if (op == MAPOP_BSWAP) {
                nValue = byteSwap32(nValue);
            } else if (op == MAPOP_BITREVERSE) {
                nValue = bitReverse32(nValue);
            } else if (op == MAPOP_SHL) {
                nValue = (quint32)shiftLeft(nValue, (quint32)nKey, 32);
            } else if (op == MAPOP_SHR) {
                nValue = (quint32)shiftRight(nValue, (quint32)nKey, 32);
            }

            writeU32(pData + j, nValue);
        }
    } else if (nWidth == 8) {
        quint64 nK = nKey;

        for (qint64 j = 0; (j + 8) <= nSize; j += 8) {
            quint64 nValue = readU64(pData + j);

            if (op == MAPOP_XOR) {
                nValue = nValue ^ nK;
            } else if (op == MAPOP_ADD) {
                nValue = nValue + nK;
            } else if (op == MAPOP_SUB) {
                nValue = nValue - nK;
            } else if (op == MAPOP_NOT) {
                nValue = ~nValue;
            } else if (op == MAPOP_NEG) {
                nValue = (quint64)(0 - nValue);
            } else if (op == MAPOP_ROL) {
                nValue = rotl64(nValue, (quint32)nKey);
            } else if (op == MAPOP_ROR) {
                nValue = rotr64(nValue, (quint32)nKey);
            } else if (op == MAPOP_BSWAP) {
                nValue = byteSwap64(nValue);
            } else if (op == MAPOP_BITREVERSE) {
                nValue = bitReverse64(nValue);
            } else if (op == MAPOP_SHL) {
                nValue = shiftLeft(nValue, (quint32)nKey, 64);
            } else if (op == MAPOP_SHR) {
                nValue = shiftRight(nValue, (quint32)nKey, 64);
            }

            writeU64(pData + j, nValue);
        }
    }
}

inline qint32 hexNibble(char nChar)
{
    if ((nChar >= '0') && (nChar <= '9')) {
        return nChar - '0';
    } else if ((nChar >= 'a') && (nChar <= 'f')) {
        return nChar - 'a' + 10;
    } else if ((nChar >= 'A') && (nChar <= 'F')) {
        return nChar - 'A' + 10;
    }
    return -1;
}

// ---- Base32 (RFC 4648) ----
QByteArray base32Encode(const QByteArray &baIn)
{
    static const char *ALPHA = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
    QByteArray baOut;
    qint32 nLen = baIn.size();

    for (qint32 i = 0; i < nLen;) {
        qint32 nBlock = qMin(5, nLen - i);  // up to 5 input bytes -> 8 chars
        quint64 nBuffer = 0;

        for (qint32 k = 0; k < 5; k++) {
            nBuffer <<= 8;
            if (k < nBlock) {
                nBuffer |= (quint8)baIn.at(i + k);
            }
        }

        qint32 nChars = (nBlock * 8 + 4) / 5;  // meaningful 5-bit groups: 1,2,3,4,5 -> 2,4,5,7,8

        for (qint32 k = 0; k < 8; k++) {
            if (k < nChars) {
                baOut.append(ALPHA[(qint32)((nBuffer >> (35 - k * 5)) & 0x1F)]);
            } else {
                baOut.append('=');
            }
        }

        i += nBlock;
    }

    return baOut;
}

qint32 base32Value(char nChar)
{
    if ((nChar >= 'A') && (nChar <= 'Z')) {
        return nChar - 'A';
    } else if ((nChar >= 'a') && (nChar <= 'z')) {
        return nChar - 'a';  // tolerate lowercase
    } else if ((nChar >= '2') && (nChar <= '7')) {
        return nChar - '2' + 26;
    }
    return -1;
}

QByteArray base32Decode(const QByteArray &baIn)
{
    QByteArray baOut;
    quint64 nBuffer = 0;
    qint32 nBits = 0;

    for (qint32 i = 0; i < baIn.size(); i++) {
        qint32 nValue = base32Value(baIn.at(i));

        if (nValue < 0) {
            continue;  // padding / whitespace / invalid
        }

        nBuffer = (nBuffer << 5) | (quint32)nValue;
        nBits += 5;

        if (nBits >= 8) {
            nBits -= 8;
            baOut.append((char)((nBuffer >> nBits) & 0xFF));
        }
    }

    return baOut;
}

// ---- Ascii85 (Adobe / btoa, 'z' for a zero group) ----
QByteArray ascii85Encode(const QByteArray &baIn)
{
    QByteArray baOut;
    qint32 nLen = baIn.size();

    for (qint32 i = 0; i < nLen;) {
        qint32 nBlock = qMin(4, nLen - i);
        quint32 nValue = 0;

        for (qint32 k = 0; k < 4; k++) {
            nValue <<= 8;
            if (k < nBlock) {
                nValue |= (quint8)baIn.at(i + k);
            }
        }

        if ((nBlock == 4) && (nValue == 0)) {
            baOut.append('z');
        } else {
            char pChars[5];
            for (qint32 k = 4; k >= 0; k--) {
                pChars[k] = (char)('!' + (nValue % 85));
                nValue /= 85;
            }
            baOut.append(pChars, nBlock + 1);  // n input bytes -> n+1 output chars
        }

        i += nBlock;
    }

    return baOut;
}

QByteArray ascii85Decode(const QByteArray &baIn)
{
    QByteArray baOut;
    quint32 nTuple = 0;
    qint32 nCount = 0;

    // Strip an optional leading Adobe "<~" frame ('<' is otherwise a valid data char and
    // must NOT be skipped mid-stream). The '~' of a trailing "~>" ends the data below.
    qint32 nStart = 0;
    while ((nStart < baIn.size()) && ((baIn.at(nStart) == ' ') || (baIn.at(nStart) == '\t') || (baIn.at(nStart) == '\r') || (baIn.at(nStart) == '\n') ||
                                      (baIn.at(nStart) == '\f') || (baIn.at(nStart) == '\v'))) {
        nStart++;
    }
    if (((nStart + 1) < baIn.size()) && (baIn.at(nStart) == '<') && (baIn.at(nStart + 1) == '~')) {
        nStart += 2;
    }

    for (qint32 i = nStart; i < baIn.size(); i++) {
        char nChar = baIn.at(i);

        if (nChar == '~') {
            break;  // '~>' end marker
        } else if ((nChar == ' ') || (nChar == '\t') || (nChar == '\r') || (nChar == '\n') || (nChar == '\f') || (nChar == '\v')) {
            continue;
        } else if (nChar == 'z') {
            if (nCount == 0) {
                baOut.append((char)0);
                baOut.append((char)0);
                baOut.append((char)0);
                baOut.append((char)0);
            }
            continue;
        } else if ((nChar < '!') || (nChar > 'u')) {
            continue;  // out of alphabet
        }

        nTuple = nTuple * 85 + (quint32)(nChar - '!');
        nCount++;

        if (nCount == 5) {
            baOut.append((char)((nTuple >> 24) & 0xFF));
            baOut.append((char)((nTuple >> 16) & 0xFF));
            baOut.append((char)((nTuple >> 8) & 0xFF));
            baOut.append((char)(nTuple & 0xFF));
            nTuple = 0;
            nCount = 0;
        }
    }

    if (nCount > 0) {
        for (qint32 k = nCount; k < 5; k++) {
            nTuple = nTuple * 85 + 84;  // pad with 'u'
        }
        for (qint32 k = 0; k < (nCount - 1); k++) {
            baOut.append((char)((nTuple >> (24 - k * 8)) & 0xFF));
        }
    }

    return baOut;
}

// ---- URL percent-encoding (RFC 3986 unreserved set) ----
inline bool urlUnreserved(quint8 nByte)
{
    return ((nByte >= 'A') && (nByte <= 'Z')) || ((nByte >= 'a') && (nByte <= 'z')) || ((nByte >= '0') && (nByte <= '9')) || (nByte == '-') || (nByte == '_') ||
           (nByte == '.') || (nByte == '~');
}

QByteArray urlEncode(const QByteArray &baIn)
{
    static const char *HEX = "0123456789ABCDEF";
    QByteArray baOut;

    for (qint32 i = 0; i < baIn.size(); i++) {
        quint8 nByte = (quint8)baIn.at(i);

        if (urlUnreserved(nByte)) {
            baOut.append((char)nByte);
        } else {
            baOut.append('%');
            baOut.append(HEX[nByte >> 4]);
            baOut.append(HEX[nByte & 0x0F]);
        }
    }

    return baOut;
}

QByteArray urlDecode(const QByteArray &baIn)
{
    QByteArray baOut;
    qint32 nLen = baIn.size();

    for (qint32 i = 0; i < nLen; i++) {
        char nChar = baIn.at(i);

        if ((nChar == '%') && ((i + 2) < nLen)) {
            qint32 nHi = hexNibble(baIn.at(i + 1));
            qint32 nLo = hexNibble(baIn.at(i + 2));

            if ((nHi >= 0) && (nLo >= 0)) {
                baOut.append((char)((nHi << 4) | nLo));
                i += 2;
                continue;
            }
        }

        baOut.append(nChar);
    }

    return baOut;
}

// ---- Quoted-printable (binary-reversible: escapes everything but 33..126 except '=') ----
QByteArray qpEncode(const QByteArray &baIn)
{
    static const char *HEX = "0123456789ABCDEF";
    QByteArray baOut;
    qint32 nLineLen = 0;

    for (qint32 i = 0; i < baIn.size(); i++) {
        quint8 nByte = (quint8)baIn.at(i);
        bool bLiteral = ((nByte >= 33) && (nByte <= 126) && (nByte != '='));

        if (bLiteral) {
            if (nLineLen >= 75) {
                baOut.append('=');
                baOut.append('\r');
                baOut.append('\n');
                nLineLen = 0;
            }
            baOut.append((char)nByte);
            nLineLen += 1;
        } else {
            if (nLineLen >= 73) {
                baOut.append('=');
                baOut.append('\r');
                baOut.append('\n');
                nLineLen = 0;
            }
            baOut.append('=');
            baOut.append(HEX[nByte >> 4]);
            baOut.append(HEX[nByte & 0x0F]);
            nLineLen += 3;
        }
    }

    return baOut;
}

QByteArray qpDecode(const QByteArray &baIn)
{
    QByteArray baOut;
    qint32 nLen = baIn.size();

    for (qint32 i = 0; i < nLen; i++) {
        char nChar = baIn.at(i);

        if (nChar == '=') {
            if (((i + 1) < nLen) && ((baIn.at(i + 1) == '\r') || (baIn.at(i + 1) == '\n'))) {
                // soft line break "=\r\n" or "=\n"
                if ((baIn.at(i + 1) == '\r') && ((i + 2) < nLen) && (baIn.at(i + 2) == '\n')) {
                    i += 2;
                } else {
                    i += 1;
                }
                continue;
            }

            if ((i + 2) < nLen) {
                qint32 nHi = hexNibble(baIn.at(i + 1));
                qint32 nLo = hexNibble(baIn.at(i + 2));

                if ((nHi >= 0) && (nLo >= 0)) {
                    baOut.append((char)((nHi << 4) | nLo));
                    i += 2;
                    continue;
                }
            }

            baOut.append(nChar);  // stray '='
        } else if ((nChar == '\r') || (nChar == '\n')) {
            continue;  // hard breaks are structure, not data (encoder escapes real CR/LF)
        } else {
            baOut.append(nChar);
        }
    }

    return baOut;
}

// ---- Base58 (Bitcoin alphabet) ----
static const char *BASE58_ALPHABET = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

qint32 base58Value(char nChar)
{
    for (qint32 i = 0; i < 58; i++) {
        if (BASE58_ALPHABET[i] == nChar) {
            return i;
        }
    }
    return -1;
}

QByteArray base58Encode(const QByteArray &baIn)
{
    qint32 nZeros = 0;
    while ((nZeros < baIn.size()) && (baIn.at(nZeros) == 0)) {
        nZeros++;
    }

    QByteArray baDigits;  // little-endian base-58 digits

    for (qint32 i = 0; i < baIn.size(); i++) {
        qint32 nCarry = (quint8)baIn.at(i);

        for (qint32 j = 0; j < baDigits.size(); j++) {
            nCarry += ((qint32)(quint8)baDigits.at(j)) << 8;
            baDigits[j] = (char)(nCarry % 58);
            nCarry /= 58;
        }

        while (nCarry) {
            baDigits.append((char)(nCarry % 58));
            nCarry /= 58;
        }
    }

    QByteArray baOut;
    for (qint32 i = 0; i < nZeros; i++) {
        baOut.append('1');
    }
    for (qint32 i = baDigits.size() - 1; i >= 0; i--) {
        baOut.append(BASE58_ALPHABET[(quint8)baDigits.at(i)]);
    }

    return baOut;
}

QByteArray base58Decode(const QByteArray &baIn)
{
    qint32 nOnes = 0;
    while ((nOnes < baIn.size()) && (baIn.at(nOnes) == '1')) {
        nOnes++;
    }

    QByteArray baBytes;  // little-endian base-256

    for (qint32 i = 0; i < baIn.size(); i++) {
        qint32 nValue = base58Value(baIn.at(i));

        if (nValue < 0) {
            continue;  // ignore whitespace / invalid
        }

        qint32 nCarry = nValue;

        for (qint32 j = 0; j < baBytes.size(); j++) {
            nCarry += ((qint32)(quint8)baBytes.at(j)) * 58;
            baBytes[j] = (char)(nCarry & 0xFF);
            nCarry >>= 8;
        }

        while (nCarry) {
            baBytes.append((char)(nCarry & 0xFF));
            nCarry >>= 8;
        }
    }

    QByteArray baOut;
    for (qint32 i = 0; i < nOnes; i++) {
        baOut.append((char)0);
    }
    for (qint32 i = baBytes.size() - 1; i >= 0; i--) {
        baOut.append(baBytes.at(i));
    }

    return baOut;
}

// ---- UUencode (classic; 0 encoded as backtick to avoid trailing-space loss) ----
inline char uuEncChar(qint32 nValue)
{
    nValue &= 0x3F;
    return nValue ? (char)(0x20 + nValue) : (char)0x60;
}

inline qint32 uuDecChar(char nChar)
{
    return ((quint8)nChar - 0x20) & 0x3F;  // 0x20 and 0x60 both map to 0
}

QByteArray uuEncode(const QByteArray &baIn)
{
    QByteArray baOut;
    baOut.append("begin 644 data\n");

    qint32 nLen = baIn.size();

    for (qint32 i = 0; i < nLen;) {
        qint32 nLine = qMin(45, nLen - i);
        baOut.append((char)(0x20 + nLine));  // line length char

        for (qint32 k = 0; k < nLine; k += 3) {
            quint8 b0 = (quint8)baIn.at(i + k);
            quint8 b1 = ((k + 1) < nLine) ? (quint8)baIn.at(i + k + 1) : 0;
            quint8 b2 = ((k + 2) < nLine) ? (quint8)baIn.at(i + k + 2) : 0;

            baOut.append(uuEncChar(b0 >> 2));
            baOut.append(uuEncChar((b0 << 4) | (b1 >> 4)));
            baOut.append(uuEncChar((b1 << 2) | (b2 >> 6)));
            baOut.append(uuEncChar(b2));
        }

        baOut.append('\n');
        i += nLine;
    }

    baOut.append((char)0x60);  // zero-length terminator line
    baOut.append('\n');
    baOut.append("end\n");

    return baOut;
}

QByteArray uuDecode(const QByteArray &baIn)
{
    QByteArray baOut;
    QList<QByteArray> listLines = baIn.split('\n');

    for (qint32 li = 0; li < listLines.size(); li++) {
        QByteArray baLine = listLines.at(li);
        while (baLine.endsWith('\r')) {
            baLine.chop(1);
        }

        if (baLine.isEmpty()) {
            continue;
        }
        if (baLine.startsWith("begin")) {
            continue;
        }
        if (baLine.startsWith("end")) {
            break;
        }

        qint32 nLine = uuDecChar(baLine.at(0));
        if (nLine <= 0) {
            break;  // terminator line
        }

        qint32 nPos = 1;
        qint32 nProduced = 0;

        while ((nProduced < nLine) && ((nPos + 4) <= baLine.size())) {
            qint32 c0 = uuDecChar(baLine.at(nPos));
            qint32 c1 = uuDecChar(baLine.at(nPos + 1));
            qint32 c2 = uuDecChar(baLine.at(nPos + 2));
            qint32 c3 = uuDecChar(baLine.at(nPos + 3));
            nPos += 4;

            quint8 b0 = (quint8)((c0 << 2) | (c1 >> 4));
            quint8 b1 = (quint8)((c1 << 4) | (c2 >> 2));
            quint8 b2 = (quint8)((c2 << 6) | c3);

            if (nProduced < nLine) {
                baOut.append((char)b0);
                nProduced++;
            }
            if (nProduced < nLine) {
                baOut.append((char)b1);
                nProduced++;
            }
            if (nProduced < nLine) {
                baOut.append((char)b2);
                nProduced++;
            }
        }
    }

    return baOut;
}

}  // namespace

XDataConvertor::XDataConvertor(QObject *pParent) : XThreadObject(pParent)
{
    m_pDeviceIn = nullptr;
    m_pData = nullptr;
    m_method = CMETHOD_UNKNOWN;
    m_options = {};
    m_pPdStruct = nullptr;
}

void XDataConvertor::setData(QIODevice *pDeviceIn, DATA *pData, CMETHOD method, const OPTIONS &options, XBinary::PDSTRUCT *pPdStruct)
{
    m_pDeviceIn = pDeviceIn;
    m_pData = pData;
    m_method = method;
    m_options = options;
    m_pPdStruct = pPdStruct;
}

bool XDataConvertor::isMethodWidth(CMETHOD method)
{
    MAPOP op = MAPOP_XOR;
    qint32 nWidth = 1;

    return decomposeMapMethod(method, &op, &nWidth);
}

bool XDataConvertor::isMethodKeyed(CMETHOD method)
{
    return (method == CMETHOD_XOR_KEY) || (method == CMETHOD_ADD_KEY) || (method == CMETHOD_SUB_KEY) || (method == CMETHOD_XOR_ROLLING);
}

bool XDataConvertor::isMethodChain(CMETHOD method)
{
    return (method == CMETHOD_DELTA_ENCODE) || (method == CMETHOD_DELTA_DECODE) || (method == CMETHOD_XORPREV_ENCODE) || (method == CMETHOD_XORPREV_DECODE);
}

bool XDataConvertor::isMethodWhole(CMETHOD method)
{
    return (method == CMETHOD_BASE32_ENCODE) || (method == CMETHOD_BASE32_DECODE) || (method == CMETHOD_ASCII85_ENCODE) || (method == CMETHOD_ASCII85_DECODE) ||
           (method == CMETHOD_URL_ENCODE) || (method == CMETHOD_URL_DECODE) || (method == CMETHOD_QP_ENCODE) || (method == CMETHOD_QP_DECODE) ||
           (method == CMETHOD_BASE58_ENCODE) || (method == CMETHOD_BASE58_DECODE) || (method == CMETHOD_UU_ENCODE) || (method == CMETHOD_UU_DECODE);
}

qint32 XDataConvertor::getMethodWidth(CMETHOD method)
{
    MAPOP op = MAPOP_XOR;
    qint32 nWidth = 1;

    if (!decomposeMapMethod(method, &op, &nWidth)) {
        nWidth = 1;
    }

    return nWidth;
}

bool XDataConvertor::convertMap(QIODevice *pDeviceIn, QIODevice *pDeviceOut, XBinary::PDSTRUCT *pPdStruct, qint32 nFreeIndex)
{
    bool bResult = false;

    MAPOP op = MAPOP_XOR;
    qint32 nWidth = 1;

    if (!decomposeMapMethod(m_method, &op, &nWidth)) {
        return false;
    }

    quint64 nKey = m_options.varKey.toULongLong();
    qint64 nInSize = pDeviceIn->size();
    // Multi-byte modes process only the width-aligned prefix; the trailing sub-width
    // bytes are intentionally dropped (matching the S_ALIGN_DOWN64 output sizing).
    qint64 nConvertSize = (nWidth > 1) ? (qint64)S_ALIGN_DOWN64(nInSize, nWidth) : nInSize;

    qint32 nBufferSize = XBinary::getBufferSize(pPdStruct);
    if (nBufferSize < 8) {
        nBufferSize = 8;  // keep chunk >= max width so width alignment never yields 0
    }

    XBinary::setPdStructTotal(pPdStruct, nFreeIndex, nInSize);

    if (!pDeviceIn->seek(0) || !pDeviceOut->seek(0)) {
        XBinary::setPdStructInfoString(pPdStruct, tr("Seek error"));
        return false;
    }

    char *pBuffer = new char[nBufferSize];
    bResult = true;

    for (qint64 nOffset = 0; (nOffset < nConvertSize) && XBinary::isPdStructNotCanceled(pPdStruct);) {
        qint64 nChunkSize = qMin((qint64)nBufferSize, nConvertSize - nOffset);

        if (nWidth > 1) {
            nChunkSize -= (nChunkSize % nWidth);  // keep each chunk width-aligned (nBufferSize >= 8 >= nWidth)
        }

        if (pDeviceIn->read(pBuffer, nChunkSize) != nChunkSize) {
            XBinary::setPdStructInfoString(pPdStruct, tr("Read error"));
            bResult = false;
            break;
        }

        applyMap(pBuffer, nChunkSize, op, nWidth, nKey);

        if (pDeviceOut->write(pBuffer, nChunkSize) != nChunkSize) {
            XBinary::setPdStructInfoString(pPdStruct, tr("Write error"));
            bResult = false;
            break;
        }

        nOffset += nChunkSize;

        XBinary::setPdStructCurrent(pPdStruct, nFreeIndex, nOffset);
    }

    delete[] pBuffer;

    return bResult && (!XBinary::isPdStructStopped(pPdStruct));
}

bool XDataConvertor::convertKeyed(QIODevice *pDeviceIn, QIODevice *pDeviceOut, XBinary::PDSTRUCT *pPdStruct, qint32 nFreeIndex)
{
    QByteArray baKey = m_options.baKey;
    qint32 nKeyLen = baKey.size();

    if (nKeyLen <= 0) {
        XBinary::setPdStructInfoString(pPdStruct, tr("Empty key"));
        return false;
    }

    qint64 nInSize = pDeviceIn->size();

    qint32 nBufferSize = XBinary::getBufferSize(pPdStruct);
    if (nBufferSize < 1) {
        nBufferSize = 1;
    }

    XBinary::setPdStructTotal(pPdStruct, nFreeIndex, nInSize);

    if (!pDeviceIn->seek(0) || !pDeviceOut->seek(0)) {
        XBinary::setPdStructInfoString(pPdStruct, tr("Seek error"));
        return false;
    }

    char *pBuffer = new char[nBufferSize];
    bool bResult = true;

    for (qint64 nOffset = 0; (nOffset < nInSize) && XBinary::isPdStructNotCanceled(pPdStruct);) {
        qint64 nChunkSize = qMin((qint64)nBufferSize, nInSize - nOffset);

        if (pDeviceIn->read(pBuffer, nChunkSize) != nChunkSize) {
            XBinary::setPdStructInfoString(pPdStruct, tr("Read error"));
            bResult = false;
            break;
        }

        for (qint64 j = 0; j < nChunkSize; j++) {
            qint64 nPos = nOffset + j;                                     // global byte position
            quint8 nKeyByte = (quint8)baKey.at((qint32)(nPos % nKeyLen));  // repeating key

            if (m_method == CMETHOD_XOR_ROLLING) {
                nKeyByte = (quint8)(nKeyByte + (quint8)(nPos & 0xFF));  // position-dependent keystream
            }

            quint8 nValue = (quint8)pBuffer[j];

            if ((m_method == CMETHOD_XOR_KEY) || (m_method == CMETHOD_XOR_ROLLING)) {
                nValue = (quint8)(nValue ^ nKeyByte);
            } else if (m_method == CMETHOD_ADD_KEY) {
                nValue = (quint8)(nValue + nKeyByte);
            } else if (m_method == CMETHOD_SUB_KEY) {
                nValue = (quint8)(nValue - nKeyByte);
            }

            pBuffer[j] = (char)nValue;
        }

        if (pDeviceOut->write(pBuffer, nChunkSize) != nChunkSize) {
            XBinary::setPdStructInfoString(pPdStruct, tr("Write error"));
            bResult = false;
            break;
        }

        nOffset += nChunkSize;

        XBinary::setPdStructCurrent(pPdStruct, nFreeIndex, nOffset);
    }

    delete[] pBuffer;

    return bResult && (!XBinary::isPdStructStopped(pPdStruct));
}

bool XDataConvertor::convertRC4(QIODevice *pDeviceIn, QIODevice *pDeviceOut, XBinary::PDSTRUCT *pPdStruct, qint32 nFreeIndex)
{
    QByteArray baKey = m_options.baKey;
    qint32 nKeyLen = baKey.size();

    if (nKeyLen <= 0) {
        XBinary::setPdStructInfoString(pPdStruct, tr("Empty key"));
        return false;
    }

    // RC4 key-scheduling (KSA).
    unsigned char pState[256];
    for (qint32 i = 0; i < 256; i++) {
        pState[i] = (unsigned char)i;
    }
    qint32 nSchedJ = 0;
    for (qint32 i = 0; i < 256; i++) {
        nSchedJ = (nSchedJ + pState[i] + (quint8)baKey.at(i % nKeyLen)) & 0xFF;
        unsigned char nTmp = pState[i];
        pState[i] = pState[nSchedJ];
        pState[nSchedJ] = nTmp;
    }

    qint64 nInSize = pDeviceIn->size();

    qint32 nBufferSize = XBinary::getBufferSize(pPdStruct);
    if (nBufferSize < 1) {
        nBufferSize = 1;
    }

    XBinary::setPdStructTotal(pPdStruct, nFreeIndex, nInSize);

    if (!pDeviceIn->seek(0) || !pDeviceOut->seek(0)) {
        XBinary::setPdStructInfoString(pPdStruct, tr("Seek error"));
        return false;
    }

    char *pBuffer = new char[nBufferSize];
    bool bResult = true;

    // PRGA indices persist across chunks (single-threaded process()).
    qint32 nI = 0;
    qint32 nJ = 0;

    for (qint64 nOffset = 0; (nOffset < nInSize) && XBinary::isPdStructNotCanceled(pPdStruct);) {
        qint64 nChunkSize = qMin((qint64)nBufferSize, nInSize - nOffset);

        if (pDeviceIn->read(pBuffer, nChunkSize) != nChunkSize) {
            XBinary::setPdStructInfoString(pPdStruct, tr("Read error"));
            bResult = false;
            break;
        }

        for (qint64 k = 0; k < nChunkSize; k++) {
            nI = (nI + 1) & 0xFF;
            nJ = (nJ + pState[nI]) & 0xFF;
            unsigned char nTmp = pState[nI];
            pState[nI] = pState[nJ];
            pState[nJ] = nTmp;
            unsigned char nKeyStream = pState[(pState[nI] + pState[nJ]) & 0xFF];
            pBuffer[k] = (char)((quint8)pBuffer[k] ^ nKeyStream);
        }

        if (pDeviceOut->write(pBuffer, nChunkSize) != nChunkSize) {
            XBinary::setPdStructInfoString(pPdStruct, tr("Write error"));
            bResult = false;
            break;
        }

        nOffset += nChunkSize;

        XBinary::setPdStructCurrent(pPdStruct, nFreeIndex, nOffset);
    }

    delete[] pBuffer;

    return bResult && (!XBinary::isPdStructStopped(pPdStruct));
}

bool XDataConvertor::convertChain(QIODevice *pDeviceIn, QIODevice *pDeviceOut, XBinary::PDSTRUCT *pPdStruct, qint32 nFreeIndex)
{
    bool bEncode = (m_method == CMETHOD_DELTA_ENCODE) || (m_method == CMETHOD_XORPREV_ENCODE);
    bool bXor = (m_method == CMETHOD_XORPREV_ENCODE) || (m_method == CMETHOD_XORPREV_DECODE);

    qint64 nDistance = m_options.varKey.toLongLong();
    if (nDistance < 1) {
        nDistance = 1;
    }
    if (nDistance > 0x10000) {
        nDistance = 0x10000;  // bound the reference ring buffer
    }

    qint64 nInSize = pDeviceIn->size();

    qint32 nBufferSize = XBinary::getBufferSize(pPdStruct);
    if (nBufferSize < 1) {
        nBufferSize = 1;
    }

    XBinary::setPdStructTotal(pPdStruct, nFreeIndex, nInSize);

    if (!pDeviceIn->seek(0) || !pDeviceOut->seek(0)) {
        XBinary::setPdStructInfoString(pPdStruct, tr("Seek error"));
        return false;
    }

    char *pBuffer = new char[nBufferSize];
    unsigned char *pRing = new unsigned char[nDistance];  // last `distance` reference bytes, init 0
    memset(pRing, 0, (size_t)nDistance);

    bool bResult = true;
    qint64 nPos = 0;

    for (qint64 nOffset = 0; (nOffset < nInSize) && XBinary::isPdStructNotCanceled(pPdStruct);) {
        qint64 nChunkSize = qMin((qint64)nBufferSize, nInSize - nOffset);

        if (pDeviceIn->read(pBuffer, nChunkSize) != nChunkSize) {
            XBinary::setPdStructInfoString(pPdStruct, tr("Read error"));
            bResult = false;
            break;
        }

        for (qint64 k = 0; k < nChunkSize; k++) {
            qint64 nRingIndex = nPos % nDistance;
            unsigned char nRef = pRing[nRingIndex];
            unsigned char nInByte = (quint8)pBuffer[k];
            unsigned char nOutByte;

            if (bEncode) {
                nOutByte = bXor ? (unsigned char)(nInByte ^ nRef) : (unsigned char)(nInByte - nRef);
                pRing[nRingIndex] = nInByte;  // reference stream is the input
            } else {
                nOutByte = bXor ? (unsigned char)(nInByte ^ nRef) : (unsigned char)(nInByte + nRef);
                pRing[nRingIndex] = nOutByte;  // reference stream is the reconstructed output
            }

            pBuffer[k] = (char)nOutByte;
            nPos++;
        }

        if (pDeviceOut->write(pBuffer, nChunkSize) != nChunkSize) {
            XBinary::setPdStructInfoString(pPdStruct, tr("Write error"));
            bResult = false;
            break;
        }

        nOffset += nChunkSize;

        XBinary::setPdStructCurrent(pPdStruct, nFreeIndex, nOffset);
    }

    delete[] pRing;
    delete[] pBuffer;

    return bResult && (!XBinary::isPdStructStopped(pPdStruct));
}

bool XDataConvertor::convertReverse(QIODevice *pDeviceIn, QIODevice *pDeviceOut, XBinary::PDSTRUCT *pPdStruct, qint32 nFreeIndex)
{
    bool bResult = false;

    qint64 nInSize = pDeviceIn->size();

    qint32 nBufferSize = XBinary::getBufferSize(pPdStruct);
    if (nBufferSize < 1) {
        nBufferSize = 1;
    }

    XBinary::setPdStructTotal(pPdStruct, nFreeIndex, nInSize);

    if (!pDeviceOut->seek(0)) {
        XBinary::setPdStructInfoString(pPdStruct, tr("Seek error"));
        return false;
    }

    char *pBuffer = new char[nBufferSize];
    bResult = true;

    // Read the input from the tail towards the head, reverse each chunk, and write
    // it out head-first. The output is therefore produced sequentially.
    qint64 nRemaining = nInSize;

    while ((nRemaining > 0) && XBinary::isPdStructNotCanceled(pPdStruct)) {
        qint64 nChunkSize = qMin((qint64)nBufferSize, nRemaining);
        qint64 nReadPos = nRemaining - nChunkSize;

        if (!pDeviceIn->seek(nReadPos)) {
            XBinary::setPdStructInfoString(pPdStruct, tr("Seek error"));
            bResult = false;
            break;
        }

        if (pDeviceIn->read(pBuffer, nChunkSize) != nChunkSize) {
            XBinary::setPdStructInfoString(pPdStruct, tr("Read error"));
            bResult = false;
            break;
        }

        for (qint64 a = 0, b = nChunkSize - 1; a < b; a++, b--) {
            char nTmp = pBuffer[a];
            pBuffer[a] = pBuffer[b];
            pBuffer[b] = nTmp;
        }

        if (pDeviceOut->write(pBuffer, nChunkSize) != nChunkSize) {
            XBinary::setPdStructInfoString(pPdStruct, tr("Write error"));
            bResult = false;
            break;
        }

        nRemaining -= nChunkSize;

        XBinary::setPdStructCurrent(pPdStruct, nFreeIndex, nInSize - nRemaining);
    }

    delete[] pBuffer;

    return bResult && (!XBinary::isPdStructStopped(pPdStruct));
}

bool XDataConvertor::convertEncode(QIODevice *pDeviceIn, QIODevice *pDeviceOut, XBinary::PDSTRUCT *pPdStruct, qint32 nFreeIndex)
{
    bool bResult = false;

    bool bHex = (m_method == CMETHOD_HEX_ENCODE);
    bool bBase64Url = (m_method == CMETHOD_BASE64URL_ENCODE);
    // Base64 must be fed 3-byte groups; only the final chunk may be a partial group
    // (its padding is then the single, correct trailing padding). Hex is context-free
    // per input byte, so any chunk boundary is fine.
    qint32 nInUnit = bHex ? 1 : 3;

    qint64 nInSize = pDeviceIn->size();

    qint32 nBufferSize = XBinary::getBufferSize(pPdStruct);
    if (nBufferSize < nInUnit) {
        nBufferSize = nInUnit;
    }

    XBinary::setPdStructTotal(pPdStruct, nFreeIndex, nInSize);

    if (!pDeviceIn->seek(0) || !pDeviceOut->seek(0)) {
        XBinary::setPdStructInfoString(pPdStruct, tr("Seek error"));
        return false;
    }

    char *pBuffer = new char[nBufferSize];
    bResult = true;

    for (qint64 nOffset = 0; (nOffset < nInSize) && XBinary::isPdStructNotCanceled(pPdStruct);) {
        qint64 nChunkSize = qMin((qint64)nBufferSize, nInSize - nOffset);
        bool bLast = ((nOffset + nChunkSize) >= nInSize);

        if ((!bLast) && (nInUnit > 1)) {
            nChunkSize -= (nChunkSize % nInUnit);  // non-final chunks must be whole groups
        }

        if (pDeviceIn->read(pBuffer, nChunkSize) != nChunkSize) {
            XBinary::setPdStructInfoString(pPdStruct, tr("Read error"));
            bResult = false;
            break;
        }

        QByteArray baIn(pBuffer, (qint32)nChunkSize);
        QByteArray baOut;
        if (bHex) {
            baOut = baIn.toHex();
        } else if (bBase64Url) {
            baOut = baIn.toBase64(QByteArray::Base64UrlEncoding);
        } else {
            baOut = baIn.toBase64();
        }
        qint64 nOutSize = baOut.size();

        if (pDeviceOut->write(baOut.constData(), nOutSize) != nOutSize) {
            XBinary::setPdStructInfoString(pPdStruct, tr("Write error"));
            bResult = false;
            break;
        }

        nOffset += nChunkSize;

        XBinary::setPdStructCurrent(pPdStruct, nFreeIndex, nOffset);
    }

    delete[] pBuffer;

    return bResult && (!XBinary::isPdStructStopped(pPdStruct));
}

bool XDataConvertor::convertDecode(QIODevice *pDeviceIn, QIODevice *pDeviceOut, XBinary::PDSTRUCT *pPdStruct, qint32 nFreeIndex)
{
    bool bResult = false;

    bool bHex = (m_method == CMETHOD_HEX_DECODE);
    bool bBase64Url = (m_method == CMETHOD_BASE64URL_DECODE);
    // Decoding consumes fixed-size character groups (4 for Base64, 2 for Hex). We
    // strip whitespace (so MIME/PEM wrapped input works) and carry any partial group
    // across chunk boundaries, decoding whole groups only until the final chunk.
    qint32 nGroup = bHex ? 2 : 4;

    qint64 nInSize = pDeviceIn->size();

    qint32 nBufferSize = XBinary::getBufferSize(pPdStruct);
    if (nBufferSize < 1) {
        nBufferSize = 1;
    }

    XBinary::setPdStructTotal(pPdStruct, nFreeIndex, nInSize);

    if (!pDeviceIn->seek(0) || !pDeviceOut->seek(0)) {
        XBinary::setPdStructInfoString(pPdStruct, tr("Seek error"));
        return false;
    }

    char *pBuffer = new char[nBufferSize];
    bResult = true;

    QByteArray baCarry;

    for (qint64 nOffset = 0; (nOffset < nInSize) && XBinary::isPdStructNotCanceled(pPdStruct);) {
        qint64 nChunkSize = qMin((qint64)nBufferSize, nInSize - nOffset);

        if (pDeviceIn->read(pBuffer, nChunkSize) != nChunkSize) {
            XBinary::setPdStructInfoString(pPdStruct, tr("Read error"));
            bResult = false;
            break;
        }

        QByteArray baGroup = baCarry;
        baCarry.clear();

        for (qint64 j = 0; j < nChunkSize; j++) {
            char nChar = pBuffer[j];

            if ((nChar != ' ') && (nChar != '\t') && (nChar != '\r') && (nChar != '\n') && (nChar != '\f') && (nChar != '\v')) {
                baGroup.append(nChar);
            }
        }

        nOffset += nChunkSize;

        bool bLast = (nOffset >= nInSize);
        QByteArray baToDecode;

        if (bLast) {
            baToDecode = baGroup;
        } else {
            qint32 nUse = (baGroup.size() / nGroup) * nGroup;
            baToDecode = baGroup.left(nUse);
            baCarry = baGroup.mid(nUse);
        }

        if (!baToDecode.isEmpty()) {
            QByteArray baOut;
            if (bHex) {
                baOut = QByteArray::fromHex(baToDecode);
            } else if (bBase64Url) {
                baOut = QByteArray::fromBase64(baToDecode, QByteArray::Base64UrlEncoding);
            } else {
                baOut = QByteArray::fromBase64(baToDecode);
            }
            qint64 nOutSize = baOut.size();

            if (nOutSize > 0) {
                if (pDeviceOut->write(baOut.constData(), nOutSize) != nOutSize) {
                    XBinary::setPdStructInfoString(pPdStruct, tr("Write error"));
                    bResult = false;
                    break;
                }
            }
        }

        XBinary::setPdStructCurrent(pPdStruct, nFreeIndex, nOffset);
    }

    delete[] pBuffer;

    return bResult && (!XBinary::isPdStructStopped(pPdStruct));
}

bool XDataConvertor::convertWhole(QIODevice *pDeviceIn, QIODevice *pDeviceOut, XBinary::PDSTRUCT *pPdStruct, qint32 nFreeIndex)
{
    // Base32/Ascii85/URL/QP are not cleanly chunk-independent, so they run whole-buffer.
    qint64 nInSize = pDeviceIn->size();

    XBinary::setPdStructTotal(pPdStruct, nFreeIndex, nInSize);

    if (!pDeviceIn->seek(0)) {
        XBinary::setPdStructInfoString(pPdStruct, tr("Seek error"));
        return false;
    }

    QByteArray baIn = pDeviceIn->readAll();
    if (baIn.size() != nInSize) {
        XBinary::setPdStructInfoString(pPdStruct, tr("Read error"));
        return false;
    }

    XBinary::setPdStructCurrent(pPdStruct, nFreeIndex, nInSize);

    if (XBinary::isPdStructStopped(pPdStruct)) {
        return false;
    }

    QByteArray baOut;

    if (m_method == CMETHOD_BASE32_ENCODE) {
        baOut = base32Encode(baIn);
    } else if (m_method == CMETHOD_BASE32_DECODE) {
        baOut = base32Decode(baIn);
    } else if (m_method == CMETHOD_ASCII85_ENCODE) {
        baOut = ascii85Encode(baIn);
    } else if (m_method == CMETHOD_ASCII85_DECODE) {
        baOut = ascii85Decode(baIn);
    } else if (m_method == CMETHOD_URL_ENCODE) {
        baOut = urlEncode(baIn);
    } else if (m_method == CMETHOD_URL_DECODE) {
        baOut = urlDecode(baIn);
    } else if (m_method == CMETHOD_QP_ENCODE) {
        baOut = qpEncode(baIn);
    } else if (m_method == CMETHOD_QP_DECODE) {
        baOut = qpDecode(baIn);
    } else if (m_method == CMETHOD_BASE58_ENCODE) {
        baOut = base58Encode(baIn);
    } else if (m_method == CMETHOD_BASE58_DECODE) {
        baOut = base58Decode(baIn);
    } else if (m_method == CMETHOD_UU_ENCODE) {
        baOut = uuEncode(baIn);
    } else if (m_method == CMETHOD_UU_DECODE) {
        baOut = uuDecode(baIn);
    }

    if (!pDeviceOut->seek(0)) {
        XBinary::setPdStructInfoString(pPdStruct, tr("Seek error"));
        return false;
    }

    if (pDeviceOut->write(baOut.constData(), baOut.size()) != baOut.size()) {
        XBinary::setPdStructInfoString(pPdStruct, tr("Write error"));
        return false;
    }

    return (!XBinary::isPdStructStopped(pPdStruct));
}

void XDataConvertor::process()
{
    XBinary::PDSTRUCT pdStructEmpty = XBinary::createPdStruct();
    XBinary::PDSTRUCT *pPdStruct = m_pPdStruct;

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    qint32 _nFreeIndex = XBinary::getFreeIndex(pPdStruct);
    XBinary::setPdStructInit(pPdStruct, _nFreeIndex, 0);

    m_pData->bValid = false;
    m_pData->pTmpFile = nullptr;
    m_pData->dEntropy = 0;

    if (!m_pDeviceIn) {
        XBinary::setPdStructInfoString(pPdStruct, tr("No data"));
        XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);
        return;
    }

    if (m_method == CMETHOD_NONE) {
        m_pData->dEntropy = XBinary::getEntropy(m_pDeviceIn, pPdStruct);
        m_pData->bValid = (!pPdStruct->bIsStop);
    } else {
        m_pData->pTmpFile = new QTemporaryFile;

        if (m_pData->pTmpFile->open()) {
            bool bConvertOk = false;

            if (isMethodWidth(m_method)) {
                bConvertOk = convertMap(m_pDeviceIn, m_pData->pTmpFile, pPdStruct, _nFreeIndex);
            } else if (isMethodKeyed(m_method)) {
                bConvertOk = convertKeyed(m_pDeviceIn, m_pData->pTmpFile, pPdStruct, _nFreeIndex);
            } else if (m_method == CMETHOD_RC4) {
                bConvertOk = convertRC4(m_pDeviceIn, m_pData->pTmpFile, pPdStruct, _nFreeIndex);
            } else if (isMethodChain(m_method)) {
                bConvertOk = convertChain(m_pDeviceIn, m_pData->pTmpFile, pPdStruct, _nFreeIndex);
            } else if (m_method == CMETHOD_REVERSE) {
                bConvertOk = convertReverse(m_pDeviceIn, m_pData->pTmpFile, pPdStruct, _nFreeIndex);
            } else if ((m_method == CMETHOD_BASE64_ENCODE) || (m_method == CMETHOD_BASE64URL_ENCODE) || (m_method == CMETHOD_HEX_ENCODE)) {
                bConvertOk = convertEncode(m_pDeviceIn, m_pData->pTmpFile, pPdStruct, _nFreeIndex);
            } else if ((m_method == CMETHOD_BASE64_DECODE) || (m_method == CMETHOD_BASE64URL_DECODE) || (m_method == CMETHOD_HEX_DECODE)) {
                bConvertOk = convertDecode(m_pDeviceIn, m_pData->pTmpFile, pPdStruct, _nFreeIndex);
            } else if (isMethodWhole(m_method)) {
                bConvertOk = convertWhole(m_pDeviceIn, m_pData->pTmpFile, pPdStruct, _nFreeIndex);
            } else {
                XBinary::setPdStructInfoString(pPdStruct, tr("Unknown method"));
            }

            // The temp file is written sequentially, so its size is already exactly the
            // number of bytes produced (no trailing padding to truncate). Flush so the
            // subsequent read-back (entropy, hex view) sees the final contents.
            m_pData->pTmpFile->flush();

            m_pData->bValid = bConvertOk;
        }

        if (m_pData->bValid) {
            m_pData->dEntropy = XBinary::getEntropy(m_pData->pTmpFile, pPdStruct);
        }
    }

    XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);
}
