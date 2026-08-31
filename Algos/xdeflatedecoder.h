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

signals:
};

#endif  // XDEFLATEDECODER_H
