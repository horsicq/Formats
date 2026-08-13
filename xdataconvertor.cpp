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
#include <limits>
#include <new>

namespace {

constexpr qint32 MAX_CONVERT_BUFFER_SIZE = 0x100000;
constexpr qint64 MAX_WHOLE_INPUT_SIZE = 64LL * 1024 * 1024;
constexpr qint64 MAX_BASE58_ENCODE_INPUT_SIZE = 0x1000;
constexpr qint64 MAX_BASE58_DECODE_INPUT_SIZE = 0x1800;
constexpr qint32 MAX_ZERO_PROGRESS_ATTEMPTS = 3;

qint32 getConvertBufferSize(XBinary::PDSTRUCT *pPdStruct, qint32 nMinimumSize = 1)
{
    nMinimumSize = qMax<qint32>(1, nMinimumSize);

    return qBound(nMinimumSize, XBinary::getBufferSize(pPdStruct), qMax(nMinimumSize, MAX_CONVERT_BUFFER_SIZE));
}

char *allocateConvertBuffer(qint32 nSize, XBinary::PDSTRUCT *pPdStruct)
{
    char *pResult = new (std::nothrow) char[nSize];

    if (!pResult) {
        XBinary::setPdStructInfoString(pPdStruct, QObject::tr("Memory allocation error"));
    }

    return pResult;
}

bool isAsciiWhitespace(char nChar)
{
    return (nChar == ' ') || (nChar == '\t') || (nChar == '\r') || (nChar == '\n') || (nChar == '\f') || (nChar == '\v');
}

bool isWholeConversionCanceled(XBinary::PDSTRUCT *pPdStruct, qint32 nPosition)
{
    return ((nPosition & 0x3FFF) == 0) && !XBinary::isPdStructNotCanceled(pPdStruct);
}

bool readExact(QIODevice *pDevice, char *pData, qint64 nSize, XBinary::PDSTRUCT *pPdStruct)
{
    qint64 nDone = 0;
    qint32 nZeroAttempts = 0;

    while (nDone < nSize) {
        if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
            return false;
        }

        const qint64 nRead = pDevice->read(pData + nDone, nSize - nDone);
        if (nRead < 0) {
            XBinary::setPdStructInfoString(pPdStruct, QObject::tr("Read error"));
            return false;
        }
        if (nRead == 0) {
            if (++nZeroAttempts >= MAX_ZERO_PROGRESS_ATTEMPTS) {
                XBinary::setPdStructInfoString(pPdStruct, QObject::tr("Read error"));
                return false;
            }
            continue;
        }

        nDone += nRead;
        nZeroAttempts = 0;
    }

    return true;
}

bool writeExact(QIODevice *pDevice, const char *pData, qint64 nSize, XBinary::PDSTRUCT *pPdStruct)
{
    qint64 nDone = 0;
    qint32 nZeroAttempts = 0;

    while (nDone < nSize) {
        if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
            return false;
        }

        const qint64 nWritten = pDevice->write(pData + nDone, nSize - nDone);
        if (nWritten < 0) {
            XBinary::setPdStructInfoString(pPdStruct, QObject::tr("Write error"));
            return false;
        }
        if (nWritten == 0) {
            if (++nZeroAttempts >= MAX_ZERO_PROGRESS_ATTEMPTS) {
                XBinary::setPdStructInfoString(pPdStruct, QObject::tr("Write error"));
                return false;
            }
            continue;
        }

        nDone += nWritten;
        nZeroAttempts = 0;
    }

    return true;
}

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
    MAPOP_ROT13,         // byte width only
    MAPOP_ROT47,         // byte width only
    MAPOP_UPPERCASE,     // byte width only
    MAPOP_LOWERCASE,     // byte width only
    MAPOP_SWAPCASE,      // byte width only
    MAPOP_ATBASH,        // byte width only
    MAPOP_ROT5,          // byte width only
    MAPOP_ROT18,         // byte width only
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

qint32 base64Value(char nChar, bool bUrl)
{
    if ((nChar >= 'A') && (nChar <= 'Z')) {
        return nChar - 'A';
    }
    if ((nChar >= 'a') && (nChar <= 'z')) {
        return nChar - 'a' + 26;
    }
    if ((nChar >= '0') && (nChar <= '9')) {
        return nChar - '0' + 52;
    }
    if (nChar == (bUrl ? '-' : '+')) {
        return 62;
    }
    if (nChar == (bUrl ? '_' : '/')) {
        return 63;
    }

    return -1;
}

// ---- Base32 (RFC 4648) ----
QByteArray base32Encode(const QByteArray &baIn, XBinary::PDSTRUCT *pPdStruct)
{
    static const char *ALPHA = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
    QByteArray baOut;
    qint32 nLen = baIn.size();

    for (qint32 i = 0; i < nLen;) {
        if (isWholeConversionCanceled(pPdStruct, i)) {
            return QByteArray();
        }
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

QByteArray base32Decode(const QByteArray &baIn, bool *pOk, XBinary::PDSTRUCT *pPdStruct)
{
    QByteArray baOut;
    quint64 nBuffer = 0;
    qint32 nBits = 0;
    qint32 nDataChars = 0;
    qint32 nPadding = 0;
    bool bPadding = false;

    *pOk = false;

    for (qint32 i = 0; i < baIn.size(); i++) {
        if (isWholeConversionCanceled(pPdStruct, i)) {
            return QByteArray();
        }
        const char nChar = baIn.at(i);
        if (isAsciiWhitespace(nChar)) {
            continue;
        }
        if (nChar == '=') {
            bPadding = true;
            nPadding++;
            continue;
        }
        if (bPadding) {
            return QByteArray();
        }

        const qint32 nValue = base32Value(nChar);

        if (nValue < 0) {
            return QByteArray();
        }

        nBuffer = (nBuffer << 5) | (quint32)nValue;
        nBits += 5;
        nDataChars++;

        if (nBits >= 8) {
            nBits -= 8;
            baOut.append((char)((nBuffer >> nBits) & 0xFF));
        }
    }

    const qint32 nRemainder = nDataChars % 8;
    if ((nRemainder != 0) && (nRemainder != 2) && (nRemainder != 4) && (nRemainder != 5) && (nRemainder != 7)) {
        return QByteArray();
    }
    if (nPadding && ((nDataChars == 0) || (nPadding != ((8 - nRemainder) % 8)))) {
        return QByteArray();
    }
    if (nBits && ((nBuffer & (((quint64)1 << nBits) - 1)) != 0)) {
        return QByteArray();
    }

    *pOk = true;
    return baOut;
}

// ---- Ascii85 (Adobe / btoa, 'z' for a zero group) ----
QByteArray ascii85Encode(const QByteArray &baIn, XBinary::PDSTRUCT *pPdStruct)
{
    QByteArray baOut;
    qint32 nLen = baIn.size();

    for (qint32 i = 0; i < nLen;) {
        if (isWholeConversionCanceled(pPdStruct, i)) {
            return QByteArray();
        }
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

QByteArray ascii85Decode(const QByteArray &baIn, bool *pOk, XBinary::PDSTRUCT *pPdStruct)
{
    QByteArray baOut;
    quint64 nTuple = 0;
    qint32 nCount = 0;
    bool bFramed = false;
    bool bEnded = false;

    *pOk = false;

    // Strip an optional leading Adobe "<~" frame ('<' is otherwise a valid data char and
    // must NOT be skipped mid-stream). The '~' of a trailing "~>" ends the data below.
    qint32 nStart = 0;
    while ((nStart < baIn.size()) && ((baIn.at(nStart) == ' ') || (baIn.at(nStart) == '\t') || (baIn.at(nStart) == '\r') || (baIn.at(nStart) == '\n') ||
                                      (baIn.at(nStart) == '\f') || (baIn.at(nStart) == '\v'))) {
        nStart++;
    }
    if (((nStart + 1) < baIn.size()) && (baIn.at(nStart) == '<') && (baIn.at(nStart + 1) == '~')) {
        bFramed = true;
        nStart += 2;
    }

    for (qint32 i = nStart; i < baIn.size(); i++) {
        if (isWholeConversionCanceled(pPdStruct, i)) {
            return QByteArray();
        }
        char nChar = baIn.at(i);

        if (nChar == '~') {
            if (!bFramed || ((i + 1) >= baIn.size()) || (baIn.at(i + 1) != '>')) {
                return QByteArray();
            }
            bEnded = true;
            i++;
            while (++i < baIn.size()) {
                if (!isAsciiWhitespace(baIn.at(i))) {
                    return QByteArray();
                }
            }
            break;
        } else if (isAsciiWhitespace(nChar)) {
            continue;
        } else if (nChar == 'z') {
            if (nCount != 0) {
                return QByteArray();
            }
            baOut.append(4, (char)0);
            continue;
        } else if ((nChar < '!') || (nChar > 'u')) {
            return QByteArray();
        }

        nTuple = nTuple * 85 + (quint32)(nChar - '!');
        nCount++;

        if (nCount == 5) {
            if (nTuple > 0xFFFFFFFFULL) {
                return QByteArray();
            }
            baOut.append((char)((nTuple >> 24) & 0xFF));
            baOut.append((char)((nTuple >> 16) & 0xFF));
            baOut.append((char)((nTuple >> 8) & 0xFF));
            baOut.append((char)(nTuple & 0xFF));
            nTuple = 0;
            nCount = 0;
        }
    }

    if (bFramed && !bEnded) {
        return QByteArray();
    }

    if (nCount == 1) {
        return QByteArray();
    }
    if (nCount > 1) {
        for (qint32 k = nCount; k < 5; k++) {
            nTuple = nTuple * 85 + 84;  // pad with 'u'
        }
        if (nTuple > 0xFFFFFFFFULL) {
            return QByteArray();
        }
        for (qint32 k = 0; k < (nCount - 1); k++) {
            baOut.append((char)((nTuple >> (24 - k * 8)) & 0xFF));
        }
    }

    *pOk = true;
    return baOut;
}

// ---- URL percent-encoding (RFC 3986 unreserved set) ----
inline bool urlUnreserved(quint8 nByte)
{
    return ((nByte >= 'A') && (nByte <= 'Z')) || ((nByte >= 'a') && (nByte <= 'z')) || ((nByte >= '0') && (nByte <= '9')) || (nByte == '-') || (nByte == '_') ||
           (nByte == '.') || (nByte == '~');
}

QByteArray urlEncode(const QByteArray &baIn, XBinary::PDSTRUCT *pPdStruct)
{
    static const char *HEX = "0123456789ABCDEF";
    QByteArray baOut;

    for (qint32 i = 0; i < baIn.size(); i++) {
        if (isWholeConversionCanceled(pPdStruct, i)) {
            return QByteArray();
        }
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

QByteArray urlDecode(const QByteArray &baIn, bool *pOk, XBinary::PDSTRUCT *pPdStruct)
{
    QByteArray baOut;
    qint32 nLen = baIn.size();

    *pOk = false;

    for (qint32 i = 0; i < nLen; i++) {
        if (isWholeConversionCanceled(pPdStruct, i)) {
            return QByteArray();
        }
        char nChar = baIn.at(i);

        if (nChar == '%') {
            if ((i + 2) >= nLen) {
                return QByteArray();
            }
            qint32 nHi = hexNibble(baIn.at(i + 1));
            qint32 nLo = hexNibble(baIn.at(i + 2));

            if ((nHi < 0) || (nLo < 0)) {
                return QByteArray();
            }
            baOut.append((char)((nHi << 4) | nLo));
            i += 2;
            continue;
        }

        baOut.append(nChar);
    }

    *pOk = true;
    return baOut;
}

// ---- Quoted-printable (binary-reversible: escapes everything but 33..126 except '=') ----
QByteArray qpEncode(const QByteArray &baIn, XBinary::PDSTRUCT *pPdStruct)
{
    static const char *HEX = "0123456789ABCDEF";
    QByteArray baOut;
    qint32 nLineLen = 0;

    for (qint32 i = 0; i < baIn.size(); i++) {
        if (isWholeConversionCanceled(pPdStruct, i)) {
            return QByteArray();
        }
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

QByteArray qpDecode(const QByteArray &baIn, bool *pOk, XBinary::PDSTRUCT *pPdStruct)
{
    QByteArray baOut;
    qint32 nLen = baIn.size();

    *pOk = false;

    for (qint32 i = 0; i < nLen; i++) {
        if (isWholeConversionCanceled(pPdStruct, i)) {
            return QByteArray();
        }
        char nChar = baIn.at(i);

        if (nChar == '=') {
            if (((i + 1) < nLen) && (baIn.at(i + 1) == '\n')) {
                i += 1;  // tolerate a Unix soft line break
                continue;
            }
            if (((i + 2) < nLen) && (baIn.at(i + 1) == '\r') && (baIn.at(i + 2) == '\n')) {
                i += 2;
                continue;  // RFC 2045 soft line break
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

            return QByteArray();
        } else {
            baOut.append(nChar);
        }
    }

    *pOk = true;
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

QByteArray base58Encode(const QByteArray &baIn, XBinary::PDSTRUCT *pPdStruct)
{
    qint32 nZeros = 0;
    while ((nZeros < baIn.size()) && (baIn.at(nZeros) == 0)) {
        nZeros++;
    }

    QByteArray baDigits;  // little-endian base-58 digits

    for (qint32 i = 0; i < baIn.size(); i++) {
        if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
            return QByteArray();
        }
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

QByteArray base58Decode(const QByteArray &baIn, bool *pOk, XBinary::PDSTRUCT *pPdStruct)
{
    *pOk = false;
    QByteArray baNormalized;
    baNormalized.reserve(baIn.size());
    for (qint32 i = 0; i < baIn.size(); i++) {
        if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
            return QByteArray();
        }
        if (isAsciiWhitespace(baIn.at(i))) {
            continue;
        }
        if (base58Value(baIn.at(i)) < 0) {
            return QByteArray();
        }
        baNormalized.append(baIn.at(i));
    }

    qint32 nOnes = 0;
    while ((nOnes < baNormalized.size()) && (baNormalized.at(nOnes) == '1')) {
        nOnes++;
    }

    QByteArray baBytes;  // little-endian base-256

    for (qint32 i = nOnes; i < baNormalized.size(); i++) {
        if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
            return QByteArray();
        }
        qint32 nValue = base58Value(baNormalized.at(i));

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

    *pOk = true;
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

QByteArray uuEncode(const QByteArray &baIn, XBinary::PDSTRUCT *pPdStruct)
{
    QByteArray baOut;
    baOut.append("begin 644 data\n");

    qint32 nLen = baIn.size();

    for (qint32 i = 0; i < nLen;) {
        if (isWholeConversionCanceled(pPdStruct, i)) {
            return QByteArray();
        }
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

QByteArray uuDecode(const QByteArray &baIn, bool *pOk, XBinary::PDSTRUCT *pPdStruct)
{
    QByteArray baOut;
    QList<QByteArray> listLines = baIn.split('\n');
    bool bHeader = false;
    bool bTerminated = false;
    bool bEnded = false;
    bool bSawData = false;

    *pOk = false;

    for (qint32 li = 0; li < listLines.size(); li++) {
        if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
            return QByteArray();
        }
        QByteArray baLine = listLines.at(li);
        while (baLine.endsWith('\r')) {
            baLine.chop(1);
        }

        if (baLine.isEmpty()) {
            continue;
        }
        if (!bHeader && !bSawData && !bTerminated && baLine.startsWith("begin ")) {
            const QList<QByteArray> listHeader = baLine.split(' ');
            if ((listHeader.size() < 3) || (listHeader.at(1).size() != 3) || listHeader.at(2).isEmpty()) {
                return QByteArray();
            }
            for (qint32 i = 0; i < 3; i++) {
                if ((listHeader.at(1).at(i) < '0') || (listHeader.at(1).at(i) > '7')) {
                    return QByteArray();
                }
            }
            bHeader = true;
            continue;
        }
        if (baLine == "end") {
            if (!bTerminated) {
                return QByteArray();
            }
            bEnded = true;
            for (qint32 i = li + 1; i < listLines.size(); i++) {
                if (!listLines.at(i).trimmed().isEmpty()) {
                    return QByteArray();
                }
            }
            break;
        }
        if (bTerminated) {
            return QByteArray();
        }

        const quint8 nLengthChar = (quint8)baLine.at(0);
        if ((nLengthChar < 0x20) || (nLengthChar > 0x60)) {
            return QByteArray();
        }
        qint32 nLine = uuDecChar((char)nLengthChar);
        if (nLine == 0) {
            if (baLine.size() != 1) {
                return QByteArray();
            }
            bTerminated = true;
            continue;
        }
        bSawData = true;

        const qint32 nEncodedSize = 4 * ((nLine + 2) / 3);
        if (baLine.size() != (1 + nEncodedSize)) {
            return QByteArray();
        }

        qint32 nPos = 1;
        qint32 nProduced = 0;

        while ((nProduced < nLine) && ((nPos + 4) <= baLine.size())) {
            for (qint32 i = 0; i < 4; i++) {
                const quint8 nEncodedChar = (quint8)baLine.at(nPos + i);
                if ((nEncodedChar < 0x20) || (nEncodedChar > 0x60)) {
                    return QByteArray();
                }
            }
            qint32 c0 = uuDecChar(baLine.at(nPos));
            qint32 c1 = uuDecChar(baLine.at(nPos + 1));
            qint32 c2 = uuDecChar(baLine.at(nPos + 2));
            qint32 c3 = uuDecChar(baLine.at(nPos + 3));
            nPos += 4;

            const qint32 nRemaining = nLine - nProduced;
            if (((nRemaining == 1) && (((c1 & 0x0F) != 0) || (c2 != 0) || (c3 != 0))) ||
                ((nRemaining == 2) && (((c2 & 0x03) != 0) || (c3 != 0)))) {
                return QByteArray();
            }

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

    if (!bTerminated || (bHeader && !bEnded)) {
        return QByteArray();
    }

    *pOk = true;
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
    const XBinary::PDSTRUCTLIFETIME progressLifetime = XBinary::retainPdStructLifetime(pPdStruct);
    bool bProgressOwnerAlive = progressLifetime.isValid();
    if (!bProgressOwnerAlive) return false;

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

    qint32 nBufferSize = getConvertBufferSize(pPdStruct, 8);  // keep chunk >= max width so width alignment never yields 0

    XBinary::setPdStructTotal(pPdStruct, nFreeIndex, nInSize);

    if (!pDeviceIn->seek(0) || !pDeviceOut->seek(0)) {
        XBinary::setPdStructInfoString(pPdStruct, tr("Seek error"));
        return false;
    }

    char *pBuffer = allocateConvertBuffer(nBufferSize, pPdStruct);
    if (!pBuffer) {
        return false;
    }
    bResult = true;

    for (qint64 nOffset = 0; bProgressOwnerAlive && (nOffset < nConvertSize) && XBinary::isPdStructNotCanceled(pPdStruct);) {
        qint64 nChunkSize = qMin((qint64)nBufferSize, nConvertSize - nOffset);

        if (nWidth > 1) {
            nChunkSize -= (nChunkSize % nWidth);  // keep each chunk width-aligned (nBufferSize >= 8 >= nWidth)
        }

        if (!readExact(pDeviceIn, pBuffer, nChunkSize, pPdStruct)) {
            bResult = false;
            break;
        }

        applyMap(pBuffer, nChunkSize, op, nWidth, nKey);

        if (!writeExact(pDeviceOut, pBuffer, nChunkSize, pPdStruct)) {
            bResult = false;
            break;
        }

        nOffset += nChunkSize;

        bProgressOwnerAlive = XBinary::setPdStructCurrentChecked(pPdStruct, nFreeIndex, nOffset, progressLifetime);
    }

    delete[] pBuffer;

    return bProgressOwnerAlive && bResult && (!XBinary::isPdStructStopped(pPdStruct));
}

bool XDataConvertor::convertKeyed(QIODevice *pDeviceIn, QIODevice *pDeviceOut, XBinary::PDSTRUCT *pPdStruct, qint32 nFreeIndex)
{
    const XBinary::PDSTRUCTLIFETIME progressLifetime = XBinary::retainPdStructLifetime(pPdStruct);
    bool bProgressOwnerAlive = progressLifetime.isValid();
    if (!bProgressOwnerAlive) return false;

    QByteArray baKey = m_options.baKey;
    qint32 nKeyLen = baKey.size();

    if (nKeyLen <= 0) {
        XBinary::setPdStructInfoString(pPdStruct, tr("Empty key"));
        return false;
    }

    qint64 nInSize = pDeviceIn->size();

    qint32 nBufferSize = getConvertBufferSize(pPdStruct);

    XBinary::setPdStructTotal(pPdStruct, nFreeIndex, nInSize);

    if (!pDeviceIn->seek(0) || !pDeviceOut->seek(0)) {
        XBinary::setPdStructInfoString(pPdStruct, tr("Seek error"));
        return false;
    }

    char *pBuffer = allocateConvertBuffer(nBufferSize, pPdStruct);
    if (!pBuffer) {
        return false;
    }
    bool bResult = true;

    for (qint64 nOffset = 0; bProgressOwnerAlive && (nOffset < nInSize) && XBinary::isPdStructNotCanceled(pPdStruct);) {
        qint64 nChunkSize = qMin((qint64)nBufferSize, nInSize - nOffset);

        if (!readExact(pDeviceIn, pBuffer, nChunkSize, pPdStruct)) {
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

        if (!writeExact(pDeviceOut, pBuffer, nChunkSize, pPdStruct)) {
            bResult = false;
            break;
        }

        nOffset += nChunkSize;

        bProgressOwnerAlive = XBinary::setPdStructCurrentChecked(pPdStruct, nFreeIndex, nOffset, progressLifetime);
    }

    delete[] pBuffer;

    return bProgressOwnerAlive && bResult && (!XBinary::isPdStructStopped(pPdStruct));
}

bool XDataConvertor::convertRC4(QIODevice *pDeviceIn, QIODevice *pDeviceOut, XBinary::PDSTRUCT *pPdStruct, qint32 nFreeIndex)
{
    const XBinary::PDSTRUCTLIFETIME progressLifetime = XBinary::retainPdStructLifetime(pPdStruct);
    bool bProgressOwnerAlive = progressLifetime.isValid();
    if (!bProgressOwnerAlive) return false;

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

    qint32 nBufferSize = getConvertBufferSize(pPdStruct);

    XBinary::setPdStructTotal(pPdStruct, nFreeIndex, nInSize);

    if (!pDeviceIn->seek(0) || !pDeviceOut->seek(0)) {
        XBinary::setPdStructInfoString(pPdStruct, tr("Seek error"));
        return false;
    }

    char *pBuffer = allocateConvertBuffer(nBufferSize, pPdStruct);
    if (!pBuffer) {
        return false;
    }
    bool bResult = true;

    // PRGA indices persist across chunks (single-threaded process()).
    qint32 nI = 0;
    qint32 nJ = 0;

    for (qint64 nOffset = 0; bProgressOwnerAlive && (nOffset < nInSize) && XBinary::isPdStructNotCanceled(pPdStruct);) {
        qint64 nChunkSize = qMin((qint64)nBufferSize, nInSize - nOffset);

        if (!readExact(pDeviceIn, pBuffer, nChunkSize, pPdStruct)) {
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

        if (!writeExact(pDeviceOut, pBuffer, nChunkSize, pPdStruct)) {
            bResult = false;
            break;
        }

        nOffset += nChunkSize;

        bProgressOwnerAlive = XBinary::setPdStructCurrentChecked(pPdStruct, nFreeIndex, nOffset, progressLifetime);
    }

    delete[] pBuffer;

    return bProgressOwnerAlive && bResult && (!XBinary::isPdStructStopped(pPdStruct));
}

bool XDataConvertor::convertChain(QIODevice *pDeviceIn, QIODevice *pDeviceOut, XBinary::PDSTRUCT *pPdStruct, qint32 nFreeIndex)
{
    const XBinary::PDSTRUCTLIFETIME progressLifetime = XBinary::retainPdStructLifetime(pPdStruct);
    bool bProgressOwnerAlive = progressLifetime.isValid();
    if (!bProgressOwnerAlive) return false;

    bool bEncode = (m_method == CMETHOD_DELTA_ENCODE) || (m_method == CMETHOD_XORPREV_ENCODE);
    bool bXor = (m_method == CMETHOD_XORPREV_ENCODE) || (m_method == CMETHOD_XORPREV_DECODE);

    const qint64 nRequestedDistance = m_options.varKey.toLongLong();
    const qint32 nDistance = (nRequestedDistance < 1) ? 1 : ((nRequestedDistance > 0x10000) ? 0x10000 : (qint32)nRequestedDistance);

    qint64 nInSize = pDeviceIn->size();

    qint32 nBufferSize = getConvertBufferSize(pPdStruct);

    XBinary::setPdStructTotal(pPdStruct, nFreeIndex, nInSize);

    if (!pDeviceIn->seek(0) || !pDeviceOut->seek(0)) {
        XBinary::setPdStructInfoString(pPdStruct, tr("Seek error"));
        return false;
    }

    char *pBuffer = allocateConvertBuffer(nBufferSize, pPdStruct);
    if (!pBuffer) {
        return false;
    }

    QByteArray baRing;
    try {
        baRing = QByteArray(nDistance, '\0');  // last `distance` reference bytes
    } catch (const std::bad_alloc &) {
        delete[] pBuffer;
        XBinary::setPdStructInfoString(pPdStruct, tr("Memory allocation error"));
        return false;
    }
    if (baRing.size() != nDistance) {
        delete[] pBuffer;
        XBinary::setPdStructInfoString(pPdStruct, tr("Memory allocation error"));
        return false;
    }

    bool bResult = true;
    qint32 nRingIndex = 0;

    for (qint64 nOffset = 0; bProgressOwnerAlive && (nOffset < nInSize) && XBinary::isPdStructNotCanceled(pPdStruct);) {
        qint64 nChunkSize = qMin((qint64)nBufferSize, nInSize - nOffset);

        if (!readExact(pDeviceIn, pBuffer, nChunkSize, pPdStruct)) {
            bResult = false;
            break;
        }

        for (qint64 k = 0; k < nChunkSize; k++) {
            if ((nRingIndex < 0) || (nRingIndex >= baRing.size())) {
                XBinary::setPdStructInfoString(pPdStruct, tr("Internal state error"));
                bResult = false;
                break;
            }

            unsigned char nRef = (quint8)baRing.at(nRingIndex);
            unsigned char nInByte = (quint8)pBuffer[k];
            unsigned char nOutByte;

            if (bEncode) {
                nOutByte = bXor ? (unsigned char)(nInByte ^ nRef) : (unsigned char)(nInByte - nRef);
                baRing[nRingIndex] = (char)nInByte;  // reference stream is the input
            } else {
                nOutByte = bXor ? (unsigned char)(nInByte ^ nRef) : (unsigned char)(nInByte + nRef);
                baRing[nRingIndex] = (char)nOutByte;  // reference stream is the reconstructed output
            }

            pBuffer[k] = (char)nOutByte;
            nRingIndex++;
            if (nRingIndex == nDistance) {
                nRingIndex = 0;
            }
        }

        if (!bResult) {
            break;
        }

        if (!writeExact(pDeviceOut, pBuffer, nChunkSize, pPdStruct)) {
            bResult = false;
            break;
        }

        nOffset += nChunkSize;

        bProgressOwnerAlive = XBinary::setPdStructCurrentChecked(pPdStruct, nFreeIndex, nOffset, progressLifetime);
    }

    delete[] pBuffer;

    return bProgressOwnerAlive && bResult && (!XBinary::isPdStructStopped(pPdStruct));
}

bool XDataConvertor::convertReverse(QIODevice *pDeviceIn, QIODevice *pDeviceOut, XBinary::PDSTRUCT *pPdStruct, qint32 nFreeIndex)
{
    bool bResult = false;
    const XBinary::PDSTRUCTLIFETIME progressLifetime = XBinary::retainPdStructLifetime(pPdStruct);
    bool bProgressOwnerAlive = progressLifetime.isValid();
    if (!bProgressOwnerAlive) return false;

    qint64 nInSize = pDeviceIn->size();

    qint32 nBufferSize = getConvertBufferSize(pPdStruct);

    XBinary::setPdStructTotal(pPdStruct, nFreeIndex, nInSize);

    if (!pDeviceOut->seek(0)) {
        XBinary::setPdStructInfoString(pPdStruct, tr("Seek error"));
        return false;
    }

    char *pBuffer = allocateConvertBuffer(nBufferSize, pPdStruct);
    if (!pBuffer) {
        return false;
    }
    bResult = true;

    // Read the input from the tail towards the head, reverse each chunk, and write
    // it out head-first. The output is therefore produced sequentially.
    qint64 nRemaining = nInSize;

    while (bProgressOwnerAlive && (nRemaining > 0) && XBinary::isPdStructNotCanceled(pPdStruct)) {
        qint64 nChunkSize = qMin((qint64)nBufferSize, nRemaining);
        qint64 nReadPos = nRemaining - nChunkSize;

        if (!pDeviceIn->seek(nReadPos)) {
            XBinary::setPdStructInfoString(pPdStruct, tr("Seek error"));
            bResult = false;
            break;
        }

        if (!readExact(pDeviceIn, pBuffer, nChunkSize, pPdStruct)) {
            bResult = false;
            break;
        }

        for (qint64 a = 0, b = nChunkSize - 1; a < b; a++, b--) {
            char nTmp = pBuffer[a];
            pBuffer[a] = pBuffer[b];
            pBuffer[b] = nTmp;
        }

        if (!writeExact(pDeviceOut, pBuffer, nChunkSize, pPdStruct)) {
            bResult = false;
            break;
        }

        nRemaining -= nChunkSize;

        bProgressOwnerAlive = XBinary::setPdStructCurrentChecked(pPdStruct, nFreeIndex, nInSize - nRemaining, progressLifetime);
    }

    delete[] pBuffer;

    return bProgressOwnerAlive && bResult && (!XBinary::isPdStructStopped(pPdStruct));
}

bool XDataConvertor::convertEncode(QIODevice *pDeviceIn, QIODevice *pDeviceOut, XBinary::PDSTRUCT *pPdStruct, qint32 nFreeIndex)
{
    bool bResult = false;
    const XBinary::PDSTRUCTLIFETIME progressLifetime = XBinary::retainPdStructLifetime(pPdStruct);
    bool bProgressOwnerAlive = progressLifetime.isValid();
    if (!bProgressOwnerAlive) return false;

    bool bHex = (m_method == CMETHOD_HEX_ENCODE);
    bool bBase64Url = (m_method == CMETHOD_BASE64URL_ENCODE);
    // Base64 must be fed 3-byte groups; only the final chunk may be a partial group
    // (its padding is then the single, correct trailing padding). Hex is context-free
    // per input byte, so any chunk boundary is fine.
    qint32 nInUnit = bHex ? 1 : 3;

    qint64 nInSize = pDeviceIn->size();

    qint32 nBufferSize = getConvertBufferSize(pPdStruct, nInUnit);

    XBinary::setPdStructTotal(pPdStruct, nFreeIndex, nInSize);

    if (!pDeviceIn->seek(0) || !pDeviceOut->seek(0)) {
        XBinary::setPdStructInfoString(pPdStruct, tr("Seek error"));
        return false;
    }

    char *pBuffer = allocateConvertBuffer(nBufferSize, pPdStruct);
    if (!pBuffer) {
        return false;
    }
    bResult = true;

    for (qint64 nOffset = 0; bProgressOwnerAlive && (nOffset < nInSize) && XBinary::isPdStructNotCanceled(pPdStruct);) {
        qint64 nChunkSize = qMin((qint64)nBufferSize, nInSize - nOffset);
        bool bLast = ((nOffset + nChunkSize) >= nInSize);

        if ((!bLast) && (nInUnit > 1)) {
            nChunkSize -= (nChunkSize % nInUnit);  // non-final chunks must be whole groups
        }

        if (!readExact(pDeviceIn, pBuffer, nChunkSize, pPdStruct)) {
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

        if (!writeExact(pDeviceOut, baOut.constData(), nOutSize, pPdStruct)) {
            bResult = false;
            break;
        }

        nOffset += nChunkSize;

        bProgressOwnerAlive = XBinary::setPdStructCurrentChecked(pPdStruct, nFreeIndex, nOffset, progressLifetime);
    }

    delete[] pBuffer;

    return bProgressOwnerAlive && bResult && (!XBinary::isPdStructStopped(pPdStruct));
}

bool XDataConvertor::convertDecode(QIODevice *pDeviceIn, QIODevice *pDeviceOut, XBinary::PDSTRUCT *pPdStruct, qint32 nFreeIndex)
{
    const XBinary::PDSTRUCTLIFETIME progressLifetime = XBinary::retainPdStructLifetime(pPdStruct);
    bool bProgressOwnerAlive = progressLifetime.isValid();
    if (!bProgressOwnerAlive) return false;

    const bool bHex = (m_method == CMETHOD_HEX_DECODE);
    const bool bBase64Url = (m_method == CMETHOD_BASE64URL_DECODE);
    const qint64 nInSize = pDeviceIn->size();
    const qint32 nBufferSize = getConvertBufferSize(pPdStruct);

    XBinary::setPdStructTotal(pPdStruct, nFreeIndex, nInSize);

    if (!pDeviceIn->seek(0) || !pDeviceOut->seek(0)) {
        XBinary::setPdStructInfoString(pPdStruct, tr("Seek error"));
        return false;
    }

    char *pBuffer = allocateConvertBuffer(nBufferSize, pPdStruct);
    if (!pBuffer) {
        return false;
    }

    bool bResult = true;
    qint32 nHexNibble = -1;
    qint32 nQuartet[4] = {0, 0, 0, 0};
    qint32 nQuartetSize = 0;
    bool bBase64Finished = false;

    const auto flushBase64Quartet = [&](QByteArray *pOutput) -> bool {
        if ((nQuartet[0] < 0) || (nQuartet[1] < 0)) {
            return false;
        }

        pOutput->append((char)((nQuartet[0] << 2) | (nQuartet[1] >> 4)));
        if (nQuartet[2] == -2) {
            if ((nQuartet[3] != -2) || (nQuartet[1] & 0x0F)) {
                return false;
            }
            bBase64Finished = true;
        } else {
            pOutput->append((char)((nQuartet[1] << 4) | (nQuartet[2] >> 2)));
            if (nQuartet[3] == -2) {
                if (nQuartet[2] & 0x03) {
                    return false;
                }
                bBase64Finished = true;
            } else {
                if (nQuartet[3] < 0) {
                    return false;
                }
                pOutput->append((char)((nQuartet[2] << 6) | nQuartet[3]));
            }
        }

        nQuartetSize = 0;
        return true;
    };

    for (qint64 nOffset = 0; bProgressOwnerAlive && (nOffset < nInSize) && XBinary::isPdStructNotCanceled(pPdStruct);) {
        const qint64 nChunkSize = qMin((qint64)nBufferSize, nInSize - nOffset);

        if (!readExact(pDeviceIn, pBuffer, nChunkSize, pPdStruct)) {
            bResult = false;
            break;
        }

        QByteArray baOut;
        baOut.reserve((qint32)nChunkSize);

        for (qint64 j = 0; j < nChunkSize; j++) {
            const char nChar = pBuffer[j];
            if (isAsciiWhitespace(nChar)) {
                continue;
            }

            if (bHex) {
                const qint32 nValue = hexNibble(nChar);
                if (nValue < 0) {
                    bResult = false;
                    break;
                }
                if (nHexNibble < 0) {
                    nHexNibble = nValue;
                } else {
                    baOut.append((char)((nHexNibble << 4) | nValue));
                    nHexNibble = -1;
                }
            } else {
                if (bBase64Finished) {
                    bResult = false;
                    break;
                }
                const qint32 nValue = (nChar == '=') ? -2 : base64Value(nChar, bBase64Url);
                if (nValue == -1) {
                    bResult = false;
                    break;
                }
                if (nQuartetSize == 0) {
                    nQuartet[0] = nValue;
                } else if (nQuartetSize == 1) {
                    nQuartet[1] = nValue;
                } else if (nQuartetSize == 2) {
                    nQuartet[2] = nValue;
                } else if (nQuartetSize == 3) {
                    nQuartet[3] = nValue;
                } else {
                    bResult = false;
                    break;
                }
                nQuartetSize++;
                if ((nQuartetSize == 4) && !flushBase64Quartet(&baOut)) {
                    bResult = false;
                    break;
                }
            }
        }

        nOffset += nChunkSize;
        if (!bResult) {
            XBinary::setPdStructInfoString(pPdStruct, tr("Invalid encoding"));
            break;
        }
        if (!baOut.isEmpty() && !writeExact(pDeviceOut, baOut.constData(), baOut.size(), pPdStruct)) {
            bResult = false;
            break;
        }

        bProgressOwnerAlive = XBinary::setPdStructCurrentChecked(pPdStruct, nFreeIndex, nOffset, progressLifetime);
    }

    if (bProgressOwnerAlive && bResult && XBinary::isPdStructNotCanceled(pPdStruct)) {
        QByteArray baTail;
        if (bHex) {
            bResult = (nHexNibble < 0);
        } else if (nQuartetSize == 1) {
            bResult = false;
        } else if (nQuartetSize == 2) {
            bResult = (nQuartet[0] >= 0) && (nQuartet[1] >= 0) && ((nQuartet[1] & 0x0F) == 0);
            if (bResult) {
                baTail.append((char)((nQuartet[0] << 2) | (nQuartet[1] >> 4)));
            }
        } else if (nQuartetSize == 3) {
            bResult = (nQuartet[0] >= 0) && (nQuartet[1] >= 0) && (nQuartet[2] >= 0) && ((nQuartet[2] & 0x03) == 0);
            if (bResult) {
                baTail.append((char)((nQuartet[0] << 2) | (nQuartet[1] >> 4)));
                baTail.append((char)((nQuartet[1] << 4) | (nQuartet[2] >> 2)));
            }
        }

        if (bResult && !baTail.isEmpty()) {
            bResult = writeExact(pDeviceOut, baTail.constData(), baTail.size(), pPdStruct);
        }
        if (!bResult && XBinary::isPdStructNotCanceled(pPdStruct) && XBinary::getPdStructInfoString(pPdStruct).isEmpty()) {
            XBinary::setPdStructInfoString(pPdStruct, tr("Invalid encoding"));
        }
    }

    delete[] pBuffer;

    return bProgressOwnerAlive && bResult && (!XBinary::isPdStructStopped(pPdStruct));
}

bool XDataConvertor::convertWhole(QIODevice *pDeviceIn, QIODevice *pDeviceOut, XBinary::PDSTRUCT *pPdStruct, qint32 nFreeIndex)
{
    const XBinary::PDSTRUCTLIFETIME progressLifetime = XBinary::retainPdStructLifetime(pPdStruct);
    bool bProgressOwnerAlive = progressLifetime.isValid();
    if (!bProgressOwnerAlive) return false;

    // These formats are not cleanly chunk-independent. Bound their in-memory
    // representation (and the quadratic Base58 implementation more tightly).
    const qint64 nInSize = pDeviceIn->size();
    qint64 nMaximumSize = MAX_WHOLE_INPUT_SIZE;
    if (m_method == CMETHOD_BASE58_ENCODE) {
        nMaximumSize = MAX_BASE58_ENCODE_INPUT_SIZE;
    } else if (m_method == CMETHOD_BASE58_DECODE) {
        nMaximumSize = MAX_BASE58_DECODE_INPUT_SIZE;
    }

    XBinary::setPdStructTotal(pPdStruct, nFreeIndex, nInSize);

    if ((nInSize < 0) || (nInSize > nMaximumSize) || (nInSize > (std::numeric_limits<qint32>::max)())) {
        XBinary::setPdStructInfoString(pPdStruct, tr("Input is too large"));
        return false;
    }

    if (!pDeviceIn->seek(0) || !pDeviceOut->seek(0)) {
        XBinary::setPdStructInfoString(pPdStruct, tr("Seek error"));
        return false;
    }

    QByteArray baIn;
    QByteArray baOut;
    bool bDecodeOk = true;

    try {
        baIn.resize((qint32)nInSize);
        const qint32 nBufferSize = getConvertBufferSize(pPdStruct);
        for (qint64 nOffset = 0; nOffset < nInSize;) {
            const qint64 nChunkSize = qMin((qint64)nBufferSize, nInSize - nOffset);
            if (!readExact(pDeviceIn, baIn.data() + nOffset, nChunkSize, pPdStruct)) {
                return false;
            }
            nOffset += nChunkSize;
            bProgressOwnerAlive = XBinary::setPdStructCurrentChecked(pPdStruct, nFreeIndex, nOffset, progressLifetime);
            if (!bProgressOwnerAlive) return false;
        }

        if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
            return false;
        }

        if (m_method == CMETHOD_BASE32_ENCODE) {
            baOut = base32Encode(baIn, pPdStruct);
        } else if (m_method == CMETHOD_BASE32_DECODE) {
            baOut = base32Decode(baIn, &bDecodeOk, pPdStruct);
        } else if (m_method == CMETHOD_ASCII85_ENCODE) {
            baOut = ascii85Encode(baIn, pPdStruct);
        } else if (m_method == CMETHOD_ASCII85_DECODE) {
            baOut = ascii85Decode(baIn, &bDecodeOk, pPdStruct);
        } else if (m_method == CMETHOD_URL_ENCODE) {
            baOut = urlEncode(baIn, pPdStruct);
        } else if (m_method == CMETHOD_URL_DECODE) {
            baOut = urlDecode(baIn, &bDecodeOk, pPdStruct);
        } else if (m_method == CMETHOD_QP_ENCODE) {
            baOut = qpEncode(baIn, pPdStruct);
        } else if (m_method == CMETHOD_QP_DECODE) {
            baOut = qpDecode(baIn, &bDecodeOk, pPdStruct);
        } else if (m_method == CMETHOD_BASE58_ENCODE) {
            baOut = base58Encode(baIn, pPdStruct);
        } else if (m_method == CMETHOD_BASE58_DECODE) {
            baOut = base58Decode(baIn, &bDecodeOk, pPdStruct);
        } else if (m_method == CMETHOD_UU_ENCODE) {
            baOut = uuEncode(baIn, pPdStruct);
        } else if (m_method == CMETHOD_UU_DECODE) {
            baOut = uuDecode(baIn, &bDecodeOk, pPdStruct);
        }
    } catch (const std::bad_alloc &) {
        XBinary::setPdStructInfoString(pPdStruct, tr("Memory allocation error"));
        return false;
    }

    if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
        return false;
    }
    if (!bDecodeOk) {
        XBinary::setPdStructInfoString(pPdStruct, tr("Invalid encoding"));
        return false;
    }

    if (!writeExact(pDeviceOut, baOut.constData(), baOut.size(), pPdStruct)) {
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
    const XBinary::PDSTRUCTLIFETIME progressLifetime = XBinary::retainPdStructLifetime(pPdStruct);
    bool bProgressOwnerAlive = progressLifetime.isValid();
    if (!bProgressOwnerAlive) return;

    qint32 _nFreeIndex = XBinary::reservePdStructRecord(pPdStruct, 0);
    XBinary::clearPdStructInfoString(pPdStruct);
    XBinary::clearPdStructErrorString(pPdStruct);

    if (!m_pData) {
        XBinary::setPdStructInfoString(pPdStruct, tr("No result data"));
        XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);
        return;
    }

    QTemporaryFile *pPreviousTmpFile = m_pData->pTmpFile;
    const bool bPreviousIsInput = pPreviousTmpFile && (pPreviousTmpFile == m_pDeviceIn);
    m_pData->bValid = false;
    m_pData->pTmpFile = nullptr;
    m_pData->dEntropy = 0;
    QTemporaryFile *pNewTmpFile = nullptr;
    qint64 nOriginalPosition = -1;
    bool bRestorePosition = false;
    bool bOperationOk = false;

    const auto calculateEntropy = [&](QIODevice *pDevice, double *pEntropy) -> bool {
        *pEntropy = XBinary::getEntropy(pDevice, getConvertBufferSize(pPdStruct), pPdStruct);
        bProgressOwnerAlive = XBinary::isPdStructLifetimeAlive(progressLifetime);
        if (!bProgressOwnerAlive) return false;
        return XBinary::isPdStructNotCanceled(pPdStruct) && XBinary::getPdStructErrorString(pPdStruct).isEmpty();
    };

    do {
        if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
            XBinary::setPdStructInfoString(pPdStruct, tr("Canceled"));
            break;
        }

        if (!m_pDeviceIn || !m_pDeviceIn->isOpen() || !m_pDeviceIn->isReadable() || m_pDeviceIn->isSequential() || (m_pDeviceIn->size() < 0)) {
            XBinary::setPdStructInfoString(pPdStruct, tr("No data"));
            break;
        }

        nOriginalPosition = m_pDeviceIn->pos();
        if (nOriginalPosition < 0) {
            XBinary::setPdStructInfoString(pPdStruct, tr("Seek error"));
            break;
        }
        bRestorePosition = true;
        if (!m_pDeviceIn->seek(0) || !m_pDeviceIn->seek(nOriginalPosition)) {
            XBinary::setPdStructInfoString(pPdStruct, tr("Seek error"));
            break;
        }

        if (m_method == CMETHOD_NONE) {
            bOperationOk = calculateEntropy(m_pDeviceIn, &m_pData->dEntropy);
            break;
        }

        const bool bSupported = isMethodWidth(m_method) || isMethodKeyed(m_method) || (m_method == CMETHOD_RC4) || isMethodChain(m_method) ||
                                (m_method == CMETHOD_REVERSE) || (m_method == CMETHOD_BASE64_ENCODE) || (m_method == CMETHOD_BASE64URL_ENCODE) ||
                                (m_method == CMETHOD_HEX_ENCODE) || (m_method == CMETHOD_BASE64_DECODE) || (m_method == CMETHOD_BASE64URL_DECODE) ||
                                (m_method == CMETHOD_HEX_DECODE) || isMethodWhole(m_method);
        if (!bSupported) {
            XBinary::setPdStructInfoString(pPdStruct, tr("Unknown method"));
            break;
        }

        try {
            pNewTmpFile = new (std::nothrow) QTemporaryFile;
        } catch (const std::bad_alloc &) {
            pNewTmpFile = nullptr;
        }
        if (!pNewTmpFile) {
            XBinary::setPdStructInfoString(pPdStruct, tr("Memory allocation error"));
            break;
        }
        if (!pNewTmpFile->open()) {
            XBinary::setPdStructInfoString(pPdStruct, tr("Open error"));
            break;
        }

        try {
            if (isMethodWidth(m_method)) {
                bOperationOk = convertMap(m_pDeviceIn, pNewTmpFile, pPdStruct, _nFreeIndex);
            } else if (isMethodKeyed(m_method)) {
                bOperationOk = convertKeyed(m_pDeviceIn, pNewTmpFile, pPdStruct, _nFreeIndex);
            } else if (m_method == CMETHOD_RC4) {
                bOperationOk = convertRC4(m_pDeviceIn, pNewTmpFile, pPdStruct, _nFreeIndex);
            } else if (isMethodChain(m_method)) {
                bOperationOk = convertChain(m_pDeviceIn, pNewTmpFile, pPdStruct, _nFreeIndex);
            } else if (m_method == CMETHOD_REVERSE) {
                bOperationOk = convertReverse(m_pDeviceIn, pNewTmpFile, pPdStruct, _nFreeIndex);
            } else if ((m_method == CMETHOD_BASE64_ENCODE) || (m_method == CMETHOD_BASE64URL_ENCODE) || (m_method == CMETHOD_HEX_ENCODE)) {
                bOperationOk = convertEncode(m_pDeviceIn, pNewTmpFile, pPdStruct, _nFreeIndex);
            } else if ((m_method == CMETHOD_BASE64_DECODE) || (m_method == CMETHOD_BASE64URL_DECODE) || (m_method == CMETHOD_HEX_DECODE)) {
                bOperationOk = convertDecode(m_pDeviceIn, pNewTmpFile, pPdStruct, _nFreeIndex);
            } else {
                bOperationOk = convertWhole(m_pDeviceIn, pNewTmpFile, pPdStruct, _nFreeIndex);
            }
            bProgressOwnerAlive = XBinary::isPdStructLifetimeAlive(progressLifetime);
        } catch (const std::bad_alloc &) {
            bProgressOwnerAlive = XBinary::isPdStructLifetimeAlive(progressLifetime);
            if (bProgressOwnerAlive) {
                XBinary::setPdStructInfoString(pPdStruct, tr("Memory allocation error"));
            }
            bOperationOk = false;
        }

        if (!bProgressOwnerAlive || !bOperationOk) {
            bOperationOk = false;
            break;
        }
        if (!pNewTmpFile->flush()) {
            XBinary::setPdStructInfoString(pPdStruct, tr("Write error"));
            bOperationOk = false;
            break;
        }
        if (!calculateEntropy(pNewTmpFile, &m_pData->dEntropy)) {
            bOperationOk = false;
            break;
        }
        if (!pNewTmpFile->seek(0)) {
            XBinary::setPdStructInfoString(pPdStruct, tr("Seek error"));
            bOperationOk = false;
            break;
        }

        m_pData->pTmpFile = pNewTmpFile;
        pNewTmpFile = nullptr;
    } while (false);

    if (bRestorePosition && m_pDeviceIn && !m_pDeviceIn->seek(nOriginalPosition)) {
        if (bProgressOwnerAlive) {
            XBinary::setPdStructInfoString(pPdStruct, tr("Seek error"));
        }
        bOperationOk = false;
    }

    if (bProgressOwnerAlive && !bOperationOk && XBinary::isPdStructStopped(pPdStruct) && XBinary::getPdStructInfoString(pPdStruct).isEmpty()) {
        XBinary::setPdStructInfoString(pPdStruct, tr("Canceled"));
    }

    if (!bOperationOk) {
        delete pNewTmpFile;
        pNewTmpFile = nullptr;
        if (m_pData->pTmpFile) {
            delete m_pData->pTmpFile;
            m_pData->pTmpFile = nullptr;
        }
        if (bPreviousIsInput) {
            // A caller may chain a conversion by reusing DATA's prior temp file as
            // the next input. A failed replacement must not destroy that source.
            m_pData->pTmpFile = pPreviousTmpFile;
            pPreviousTmpFile = nullptr;
        }
        m_pData->dEntropy = 0;
    }

    m_pData->bValid = bOperationOk;

    if (pPreviousTmpFile && (pPreviousTmpFile != m_pData->pTmpFile)) {
        if (pPreviousTmpFile == m_pDeviceIn) {
            m_pDeviceIn = nullptr;
        }
        delete pPreviousTmpFile;
    }

    if (bProgressOwnerAlive) {
        XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);
    }
}
