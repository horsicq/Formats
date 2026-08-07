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
#ifndef XDATACONVERTOR_H
#define XDATACONVERTOR_H

#include "xbinary.h"
#include "xthreadobject.h"

class XDataConvertor : public XThreadObject {
    Q_OBJECT

public:
    enum CMETHOD {
        CMETHOD_UNKNOWN = 0,
        CMETHOD_NONE,
        CMETHOD_XOR_BYTE,
        CMETHOD_XOR_WORD,
        CMETHOD_XOR_DWORD,
        CMETHOD_XOR_QWORD,
        CMETHOD_ADD_BYTE,
        CMETHOD_ADD_WORD,
        CMETHOD_ADD_DWORD,
        CMETHOD_ADD_QWORD,
        CMETHOD_SUB_BYTE,
        CMETHOD_SUB_WORD,
        CMETHOD_SUB_DWORD,
        CMETHOD_SUB_QWORD,
        CMETHOD_BASE64_ENCODE,
        CMETHOD_BASE64_DECODE,
        CMETHOD_NOT,
        CMETHOD_ROL_BYTE,
        CMETHOD_ROL_WORD,
        CMETHOD_ROL_DWORD,
        CMETHOD_ROL_QWORD,
        CMETHOD_ROR_BYTE,
        CMETHOD_ROR_WORD,
        CMETHOD_ROR_DWORD,
        CMETHOD_ROR_QWORD,
        CMETHOD_BSWAP_WORD,
        CMETHOD_BSWAP_DWORD,
        CMETHOD_BSWAP_QWORD,
        CMETHOD_REVERSE,
        CMETHOD_HEX_ENCODE,
        CMETHOD_HEX_DECODE,
        // Bit-level 1:1 (varKey = shift count for SHL/SHR)
        CMETHOD_NEG_BYTE,
        CMETHOD_NEG_WORD,
        CMETHOD_NEG_DWORD,
        CMETHOD_NEG_QWORD,
        CMETHOD_BITREVERSE_BYTE,
        CMETHOD_BITREVERSE_WORD,
        CMETHOD_BITREVERSE_DWORD,
        CMETHOD_BITREVERSE_QWORD,
        CMETHOD_NIBBLESWAP,
        CMETHOD_SHL_BYTE,
        CMETHOD_SHL_WORD,
        CMETHOD_SHL_DWORD,
        CMETHOD_SHL_QWORD,
        CMETHOD_SHR_BYTE,
        CMETHOD_SHR_WORD,
        CMETHOD_SHR_DWORD,
        CMETHOD_SHR_QWORD,
        // Text 1:1
        CMETHOD_ROT13,
        CMETHOD_ROT47,
        // Keyed 1:1 (baKey = repeating byte key)
        CMETHOD_XOR_KEY,
        CMETHOD_ADD_KEY,
        CMETHOD_SUB_KEY,
        CMETHOD_XOR_ROLLING,
        // Encodings (expand/shrink)
        CMETHOD_BASE64URL_ENCODE,
        CMETHOD_BASE64URL_DECODE,
        CMETHOD_BASE32_ENCODE,
        CMETHOD_BASE32_DECODE,
        CMETHOD_ASCII85_ENCODE,
        CMETHOD_ASCII85_DECODE,
        CMETHOD_URL_ENCODE,
        CMETHOD_URL_DECODE,
        CMETHOD_QP_ENCODE,
        CMETHOD_QP_DECODE,
        // Chained filters (varKey = distance, default 1)
        CMETHOD_DELTA_ENCODE,
        CMETHOD_DELTA_DECODE,
        CMETHOD_XORPREV_ENCODE,
        CMETHOD_XORPREV_DECODE,
        // Stream cipher (baKey)
        CMETHOD_RC4,
        // Charset / text 1:1
        CMETHOD_UPPERCASE,
        CMETHOD_LOWERCASE,
        CMETHOD_SWAPCASE,
        CMETHOD_ATBASH,
        CMETHOD_ROT5,
        CMETHOD_ROT18,
        CMETHOD_EBCDIC_TO_ASCII,
        CMETHOD_ASCII_TO_EBCDIC,
        // Encodings (whole-buffer)
        CMETHOD_BASE58_ENCODE,
        CMETHOD_BASE58_DECODE,
        CMETHOD_UU_ENCODE,
        CMETHOD_UU_DECODE
    };

    struct DATA {
        bool bValid;
        QTemporaryFile *pTmpFile;
        double dEntropy;
    };

    struct OPTIONS {
        QVariant varKey;   // scalar key, rotate count or shift count
        QByteArray baKey;  // repeating byte-key stream (keyed XOR/ADD/SUB, rolling)
    };

    explicit XDataConvertor(QObject *pParent = nullptr);
    void setData(QIODevice *pDeviceIn, DATA *pData, CMETHOD method, const OPTIONS &options, XBinary::PDSTRUCT *pPdStruct);
    void process();

    static bool isMethodWidth(CMETHOD method);     // 1:1 map methods (bit/byte/word ops + charset/text)
    static bool isMethodKeyed(CMETHOD method);     // XOR/ADD/SUB_KEY, XOR_ROLLING (use OPTIONS.baKey)
    static bool isMethodChain(CMETHOD method);     // DELTA/XORPREV encode/decode (varKey = distance)
    static bool isMethodWhole(CMETHOD method);     // Base32/Ascii85/URL/QP/Base58/UU (whole-buffer)
    static qint32 getMethodWidth(CMETHOD method);  // 1/2/4/8 (byte width the method operates on)

private:
    // Streaming converters. Each reads pDeviceIn sequentially and writes pDeviceOut
    // sequentially (output offset is never assumed equal to the input offset), reports
    // progress on nFreeIndex and honours cooperative cancellation.
    bool convertMap(QIODevice *pDeviceIn, QIODevice *pDeviceOut, XBinary::PDSTRUCT *pPdStruct, qint32 nFreeIndex);      // 1:1 bit/byte/word ops
    bool convertKeyed(QIODevice *pDeviceIn, QIODevice *pDeviceOut, XBinary::PDSTRUCT *pPdStruct, qint32 nFreeIndex);    // 1:1 repeating-key / rolling XOR-ADD-SUB
    bool convertRC4(QIODevice *pDeviceIn, QIODevice *pDeviceOut, XBinary::PDSTRUCT *pPdStruct, qint32 nFreeIndex);      // 1:1 RC4 stream cipher (baKey)
    bool convertChain(QIODevice *pDeviceIn, QIODevice *pDeviceOut, XBinary::PDSTRUCT *pPdStruct, qint32 nFreeIndex);    // 1:1 delta / xor-with-previous
    bool convertReverse(QIODevice *pDeviceIn, QIODevice *pDeviceOut, XBinary::PDSTRUCT *pPdStruct, qint32 nFreeIndex);  // 1:1 whole-stream reverse
    bool convertEncode(QIODevice *pDeviceIn, QIODevice *pDeviceOut, XBinary::PDSTRUCT *pPdStruct, qint32 nFreeIndex);   // expanding Base64/Base64Url/Hex encode
    bool convertDecode(QIODevice *pDeviceIn, QIODevice *pDeviceOut, XBinary::PDSTRUCT *pPdStruct, qint32 nFreeIndex);   // shrinking Base64/Base64Url/Hex decode
    bool convertWhole(QIODevice *pDeviceIn, QIODevice *pDeviceOut, XBinary::PDSTRUCT *pPdStruct, qint32 nFreeIndex);    // whole-buffer Base32/Ascii85/URL/QP

    QIODevice *m_pDeviceIn;
    DATA *m_pData;
    CMETHOD m_method;
    OPTIONS m_options;
    XBinary::PDSTRUCT *m_pPdStruct;
};

#endif  // XDATACONVERTOR_H
