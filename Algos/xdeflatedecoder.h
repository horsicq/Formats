/* Copyright (c) 2025-2026 hors<horsicq@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef XDEFLATEDECODER_H
#define XDEFLATEDECODER_H

#include "xbinary.h"
#include "xalgo_local.h"

class XDeflateDecoder : public QObject {
    Q_OBJECT
public:
    explicit XDeflateDecoder(QObject *parent = nullptr);
    static bool decompress(XBinary::DATAPROCESS_STATE *pDecompressState,
                           XBinary::PDSTRUCT *pPdStruct = nullptr,
                           bool bAcceptExactOutputAtEOF = false);
    static bool decompress64(XBinary::DATAPROCESS_STATE *pDecompressState, XBinary::PDSTRUCT *pPdStruct = nullptr);
    static bool decompress_zlib(XBinary::DATAPROCESS_STATE *pDecompressState, XBinary::PDSTRUCT *pPdStruct = nullptr);
    static bool compress(XBinary::DATAPROCESS_STATE *pCompressState, XBinary::PDSTRUCT *pPdStruct = nullptr, int nCompressionLevel = Z_DEFAULT_COMPRESSION);
    static bool compress_zlib(XBinary::DATAPROCESS_STATE *pCompressState, XBinary::PDSTRUCT *pPdStruct = nullptr, int nCompressionLevel = Z_DEFAULT_COMPRESSION);

    // ---- stream structure ----
    //
    // A deflate stream records the encoder's own decisions -- where to end a
    // block, which block type to use, how large to make the Huffman alphabets.
    // Those decisions come from the encoder's buffer sizes and match strategy,
    // not from the data, so the block layout identifies which encoder produced
    // a stream even when the compressed bytes cannot be reproduced.

    enum BLOCKTYPE {
        BLOCKTYPE_STORED = 0,
        BLOCKTYPE_FIXED = 1,
        BLOCKTYPE_DYNAMIC = 2,
        BLOCKTYPE_RESERVED = 3
    };

    struct BLOCK {
        BLOCKTYPE blockType;
        bool bFinal;
        qint64 nBitOffset;         // where the block header starts
        qint64 nBitSize;           // block size in bits, header included
        qint32 nNumberOfSymbols;   // literal/length symbols, end-of-block included
        qint64 nStoredSize;        // BLOCKTYPE_STORED only
        qint32 nHLIT;              // BLOCKTYPE_DYNAMIC only
        qint32 nHDIST;             // BLOCKTYPE_DYNAMIC only
        qint32 nHCLEN;             // BLOCKTYPE_DYNAMIC only
    };

    enum ENCODER {
        ENCODER_UNKNOWN = 0,
        ENCODER_ZLIB,             // 1 << (memLevel + 6) symbols per block
        ENCODER_ZLIB_OR_INFOZIP,  // 32768: zlib at memLevel 9 and Info-ZIP agree here
        ENCODER_SEVENZIP,         // 11265/11266 = 11264 symbols plus end-of-block
        ENCODER_NOT_ZLIB          // framing zlib cannot produce, producer unnamed
    };

    struct ENCODERINFO {
        ENCODER encoder;
        qint32 nSymbolsPerBlock;   // the repeating count, -1 when it does not repeat
        qint32 nMemLevel;          // implied zlib memLevel, -1 when not applicable
        qint32 nNumberOfBlocks;
        // A stream that never fills one block made no decision worth reading:
        // every encoder produces the same layout, so the verdict means nothing.
        bool bReliable;
    };

    // Walk the block headers. Decodes only enough Huffman data to find block
    // boundaries -- the payload is never reconstructed. nMaxBlocks limits the
    // walk (-1 for the whole stream); returns an empty list on a malformed
    // stream.
    static QList<BLOCK> getDeflateBlocks(const QByteArray &baData, qint32 nMaxBlocks = -1);
    static ENCODERINFO identifyEncoder(const QByteArray &baData);
    static QString encoderIdToString(ENCODER encoder);

signals:
};

#endif  // XDEFLATEDECODER_H
