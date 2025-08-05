/* Copyright (c) 2022-2025 hors<horsicq@gmail.com>
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
#ifndef XPNG_H
#define XPNG_H

#include "xbinary.h"

class XPNG : public XBinary {
    Q_OBJECT

public:
    enum COLOR_TYPE {
        COLOR_TYPE_GRAYSCALE = 0,        // Grayscale
        COLOR_TYPE_RGB = 2,              // RGB
        COLOR_TYPE_PALETTE = 3,          // Palette
        COLOR_TYPE_GRAYSCALE_ALPHA = 4,  // Grayscale with alpha
        COLOR_TYPE_RGBA = 6              // RGBA
    };

    struct CHUNK {
        QString sName;
        qint64 nDataOffset;
        qint64 nDataSize;
        quint32 nCRC;
    };

public:
    explicit XPNG(QIODevice *pDevice = nullptr);
    ~XPNG();

    virtual bool isValid(PDSTRUCT *pPdStruct = nullptr);
    static bool isValid(QIODevice *pDevice);
    virtual FT getFileType();
    virtual QString getFileFormatExt();
    virtual QString getFileFormatExtsString();
    virtual qint64 getFileFormatSize(PDSTRUCT *pPdStruct);
    virtual _MEMORY_MAP getMemoryMap(MAPMODE mapMode = MAPMODE_UNKNOWN, PDSTRUCT *pPdStruct = nullptr);
    virtual QString getMIMEString();
    virtual ENDIAN getEndian();
    
    static bool createPNG(QIODevice *pDevice, quint32 nWidth, quint32 nHeight, const QByteArray &baImageData, COLOR_TYPE colorType, quint8 nBitDepth = 8);

private:
    CHUNK _readChunk(qint64 nOffset);
    static bool _writeChunk(QIODevice *pDevice, const QString &sChunkType, const QByteArray &data);
    static quint32 _calculateCRC32(const QByteArray &data);
    static QByteArray _compressData(const QByteArray &data);
};

#endif  // XPNG_H
