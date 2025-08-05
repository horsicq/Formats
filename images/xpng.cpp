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
#include "xpng.h"
#include <zlib.h>

XPNG::XPNG(QIODevice *pDevice) : XBinary(pDevice)
{
}

XPNG::~XPNG()
{
}

bool XPNG::isValid(PDSTRUCT *pPdStruct)
{
    bool bIsValid = false;

    if (getSize() >= 20) {
        _MEMORY_MAP memoryMap = XBinary::getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);

        bIsValid = compareSignature(&memoryMap, "89'PNG\r\n'1A0A", 0, pPdStruct);
    }

    return bIsValid;
}

bool XPNG::isValid(QIODevice *pDevice)
{
    XPNG xpng(pDevice);

    return xpng.isValid();
}

XBinary::FT XPNG::getFileType()
{
    return FT_PNG;
}

// QString XPNG::getFileFormatString()
// {
//     QString sResult;

//     QString sResolution = QString("%1x%2").arg(read_uint32(16, true)).arg(read_uint32(20, true));

//     sResult = QString("PNG(%1)").arg(sResolution);

//     return sResult;
// }

QString XPNG::getFileFormatExt()
{
    return "png";
}

QString XPNG::getFileFormatExtsString()
{
    return "PNG (*.png)";
}

qint64 XPNG::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)
    qint64 nResult = 0;

    qint64 nOffset = 8;

    while (true) {
        CHUNK chunk = _readChunk(nOffset);

        nOffset += (12 + chunk.nDataSize);

        if (chunk.sName == "IEND") {
            nResult = nOffset;

            break;
        }

        if (chunk.nCRC == 0) {  // mb TODO more checks mb ANSI names
            break;
        }
    }

    return nResult;
}

QString XPNG::getMIMEString()
{
    return "image/png";
}

XBinary::ENDIAN XPNG::getEndian()
{
    return ENDIAN_BIG;  // PNG is always big-endian
}

XBinary::_MEMORY_MAP XPNG::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(mapMode)

    XBinary::_MEMORY_MAP result = {};

    qint32 nIndex = 0;

    {
        _MEMORY_RECORD record = {};

        record.nIndex = nIndex++;
        record.filePart = FILEPART_HEADER;
        record.nOffset = 0;
        record.nSize = 8;
        record.nAddress = -1;
        record.sName = tr("Header");

        result.listRecords.append(record);
    }

    qint64 nOffset = 8;

    while (XBinary::isPdStructNotCanceled(pPdStruct)) {
        CHUNK chunk = _readChunk(nOffset);

        _MEMORY_RECORD record = {};

        record.nIndex = nIndex++;

        record.filePart = FILEPART_OBJECT;

        record.nOffset = nOffset;
        record.nSize = 12 + chunk.nDataSize;
        record.nAddress = -1;
        record.sName = chunk.sName;

        result.listRecords.append(record);

        nOffset += (12 + chunk.nDataSize);

        if (chunk.sName == "IEND") {
            break;
        }

        if (chunk.nCRC == 0) {  // mb TODO more checks mb ANSI names
            break;
        }
    }

    _handleOverlay(&result);

    return result;
}

XPNG::CHUNK XPNG::_readChunk(qint64 nOffset)
{
    CHUNK result = {};

    result.nDataSize = read_uint32(nOffset, true);
    result.nDataOffset = nOffset + 8;
    result.sName = read_ansiString(nOffset + 4, 4);
    result.nCRC = read_uint32(nOffset + 8 + result.nDataSize, true);

    return result;
}

bool XPNG::createPNG(QIODevice *pDevice, quint32 nWidth, quint32 nHeight, const QByteArray &baImageData, COLOR_TYPE colorType, quint8 nBitDepth)
{
    if (!pDevice || !pDevice->isWritable()) {
        return false;
    }

    // Calculate expected data size based on color type
    qint32 nBytesPerPixel = 0;
    switch (colorType) {
        case COLOR_TYPE_GRAYSCALE:
            nBytesPerPixel = (nBitDepth + 7) / 8;
            break;
        case COLOR_TYPE_RGB:
            nBytesPerPixel = 3 * ((nBitDepth + 7) / 8);
            break;
        case COLOR_TYPE_PALETTE:
            nBytesPerPixel = (nBitDepth + 7) / 8;
            break;
        case COLOR_TYPE_GRAYSCALE_ALPHA:
            nBytesPerPixel = 2 * ((nBitDepth + 7) / 8);
            break;
        case COLOR_TYPE_RGBA:
            nBytesPerPixel = 4 * ((nBitDepth + 7) / 8);
            break;
        default:
            return false;
    }

    qint64 nExpectedSize = nHeight * (nWidth * nBytesPerPixel + 1); // +1 for filter byte per scanline
    if (baImageData.size() < nExpectedSize) {
        return false;
    }

    // Write PNG signature
    const char pngSignature[8] = {'\x89', 'P', 'N', 'G', '\r', '\n', '\x1a', '\n'};
    if (pDevice->write(pngSignature, 8) != 8) {
        return false;
    }

    // Create IHDR chunk data
    QByteArray ihdrData;
    ihdrData.append((char*)&nWidth, 4);    // Width (big-endian)
    ihdrData.append((char*)&nHeight, 4);   // Height (big-endian)
    ihdrData.append(nBitDepth);            // Bit depth
    ihdrData.append((char)colorType);      // Color type
    ihdrData.append((char)0);              // Compression method (0 = deflate)
    ihdrData.append((char)0);              // Filter method (0 = adaptive)
    ihdrData.append((char)0);              // Interlace method (0 = no interlace)

    // Convert to big-endian for width and height
    quint32 nWidthBE = qToBigEndian(nWidth);
    quint32 nHeightBE = qToBigEndian(nHeight);
    ihdrData[0] = (nWidthBE >> 24) & 0xFF;
    ihdrData[1] = (nWidthBE >> 16) & 0xFF;
    ihdrData[2] = (nWidthBE >> 8) & 0xFF;
    ihdrData[3] = nWidthBE & 0xFF;
    ihdrData[4] = (nHeightBE >> 24) & 0xFF;
    ihdrData[5] = (nHeightBE >> 16) & 0xFF;
    ihdrData[6] = (nHeightBE >> 8) & 0xFF;
    ihdrData[7] = nHeightBE & 0xFF;

    // Write IHDR chunk
    if (!_writeChunk(pDevice, "IHDR", ihdrData)) {
        return false;
    }

    // Compress image data
    QByteArray compressedData = _compressData(baImageData);
    if (compressedData.isEmpty()) {
        return false;
    }

    // Write IDAT chunk
    if (!_writeChunk(pDevice, "IDAT", compressedData)) {
        return false;
    }

    // Write IEND chunk
    if (!_writeChunk(pDevice, "IEND", QByteArray())) {
        return false;
    }

    return true;
}

bool XPNG::_writeChunk(QIODevice *pDevice, const QString &sChunkType, const QByteArray &data)
{
    if (!pDevice || sChunkType.length() != 4) {
        return false;
    }

    // Write data length (big-endian)
    quint32 nDataLength = qToBigEndian((quint32)data.size());
    if (pDevice->write((char*)&nDataLength, 4) != 4) {
        return false;
    }

    // Write chunk type
    QByteArray chunkTypeBytes = sChunkType.toLatin1();
    if (pDevice->write(chunkTypeBytes) != 4) {
        return false;
    }

    // Write data
    if (!data.isEmpty() && pDevice->write(data) != data.size()) {
        return false;
    }

    // Calculate and write CRC
    QByteArray crcData = chunkTypeBytes + data;
    quint32 nCRC = qToBigEndian(_calculateCRC32(crcData));
    if (pDevice->write((char*)&nCRC, 4) != 4) {
        return false;
    }

    return true;
}

quint32 XPNG::_calculateCRC32(const QByteArray &data)
{
    // Use zlib's CRC32 function
    return crc32(0, (const Bytef*)data.constData(), data.size());
}

QByteArray XPNG::_compressData(const QByteArray &data)
{
    QByteArray result;
    
    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    
    if (deflateInit(&stream, Z_DEFAULT_COMPRESSION) != Z_OK) {
        return result;
    }
    
    stream.avail_in = data.size();
    stream.next_in = (Bytef*)data.constData();
    
    const int CHUNK_SIZE = 16384;
    char outBuffer[CHUNK_SIZE];
    
    do {
        stream.avail_out = CHUNK_SIZE;
        stream.next_out = (Bytef*)outBuffer;
        
        int ret = deflate(&stream, Z_FINISH);
        if (ret == Z_STREAM_ERROR) {
            deflateEnd(&stream);
            return QByteArray();
        }
        
        int compressed = CHUNK_SIZE - stream.avail_out;
        result.append(outBuffer, compressed);
        
    } while (stream.avail_out == 0);
    
    deflateEnd(&stream);
    return result;
}
