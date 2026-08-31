/* Copyright (c) 2022-2026 hors<horsicq@gmail.com>
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
#include "../xmetadataappender.h"
#include <QBuffer>
#include <zlib.h>

#include <algorithm>
#include <limits>
#include <new>

namespace {
const qint64 PNG_MAX_ENCODE_BUFFER_SIZE = 256LL * 1024 * 1024;
const qint32 PNG_MAX_CHUNK_COUNT = 65536;

bool isValidPngColorDepth(XPNG::COLOR_TYPE colorType, quint8 nBitDepth)
{
    if (colorType == XPNG::COLOR_TYPE_GRAYSCALE) {
        return (nBitDepth == 1) || (nBitDepth == 2) || (nBitDepth == 4) || (nBitDepth == 8) || (nBitDepth == 16);
    }
    if (colorType == XPNG::COLOR_TYPE_PALETTE) {
        return (nBitDepth == 1) || (nBitDepth == 2) || (nBitDepth == 4) || (nBitDepth == 8);
    }
    if ((colorType == XPNG::COLOR_TYPE_RGB) || (colorType == XPNG::COLOR_TYPE_GRAYSCALE_ALPHA) || (colorType == XPNG::COLOR_TYPE_RGBA)) {
        return (nBitDepth == 8) || (nBitDepth == 16);
    }

    return false;
}

bool pngWriteAll(QIODevice *pDevice, const char *pData, qint64 nSize)
{
    if (!pDevice || !pDevice->isWritable() || (nSize < 0) || ((nSize > 0) && !pData)) {
        return false;
    }

    qint64 nWritten = 0;
    while (nWritten < nSize) {
        const qint64 nResult = pDevice->write(pData + nWritten, nSize - nWritten);
        if ((nResult <= 0) || (nResult > (nSize - nWritten))) {
            return false;
        }
        nWritten += nResult;
    }

    return true;
}
}  // namespace

XBinary::XCONVERT _TABLE_XPNG_STRUCTID[] = {
    {XPNG::STRUCTID_UNKNOWN, "Unknown", QObject::tr("Unknown")},
    {XPNG::STRUCTID_SIGNATURE, "Signature", QObject::tr("Signature")},
    {XPNG::STRUCTID_CHUNK, "Chunk", QObject::tr("Chunk")},
    {XPNG::STRUCTID_IHDR, "IHDR", QString("IHDR")},
    {XPNG::STRUCTID_PLTE, "PLTE", QString("PLTE")},
    {XPNG::STRUCTID_IDAT, "IDAT", QString("IDAT")},
    {XPNG::STRUCTID_IEND, "IEND", QString("IEND")},
    {XPNG::STRUCTID_cHRM, "cHRM", QString("cHRM")},
    {XPNG::STRUCTID_gAMA, "gAMA", QString("gAMA")},
    {XPNG::STRUCTID_iCCP, "iCCP", QString("iCCP")},
    {XPNG::STRUCTID_sBIT, "sBIT", QString("sBIT")},
    {XPNG::STRUCTID_sRGB, "sRGB", QString("sRGB")},
    {XPNG::STRUCTID_tEXt, "tEXt", QString("tEXt")},
    {XPNG::STRUCTID_zTXt, "zTXt", QString("zTXt")},
    {XPNG::STRUCTID_iTXt, "iTXt", QString("iTXt")},
    {XPNG::STRUCTID_bKGD, "bKGD", QString("bKGD")},
    {XPNG::STRUCTID_hIST, "hIST", QString("hIST")},
    {XPNG::STRUCTID_tRNS, "tRNS", QString("tRNS")},
    {XPNG::STRUCTID_pHYs, "pHYs", QString("pHYs")},
    {XPNG::STRUCTID_sPLT, "sPLT", QString("sPLT")},
    {XPNG::STRUCTID_tIME, "tIME", QString("tIME")},
};

XPNG::XPNG(QIODevice *pDevice) : XBinary(pDevice)
{
}

XPNG::~XPNG()
{
}

bool XPNG::isValid(PDSTRUCT *pPdStruct)
{
    return _getStructuredSize(pPdStruct) > 0;
}

qint64 XPNG::_getStructuredSize(PDSTRUCT *pPdStruct)
{
    if ((getSize() < 33) || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return 0;
    }

    _MEMORY_MAP memoryMap = XBinary::getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);

    if (!compareSignature(&memoryMap, "89'PNG\r\n'1A0A", 0, pPdStruct) || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return 0;
    }

    const CHUNK chunk = _readChunk(8);

    if (!chunk.bValid || (chunk.sName != "IHDR") || (chunk.nDataSize != 13)) {
        return 0;
    }

    const quint32 nWidth = read_uint32(chunk.nDataOffset, true);
    const quint32 nHeight = read_uint32(chunk.nDataOffset + 4, true);
    const COLOR_TYPE colorType = (COLOR_TYPE)read_uint8(chunk.nDataOffset + 9);

    if ((nWidth == 0) || (nHeight == 0) || !isValidPngColorDepth(colorType, read_uint8(chunk.nDataOffset + 8)) || (read_uint8(chunk.nDataOffset + 10) != 0) ||
        (read_uint8(chunk.nDataOffset + 11) != 0) || (read_uint8(chunk.nDataOffset + 12) > 1)) {
        return 0;
    }

    bool bHasImageData = false;
    bool bHasNonemptyImageData = false;
    bool bImageDataEnded = false;
    bool bHasPalette = false;
    qint64 nOffset = 8;
    qint32 nChunkCount = 0;

    while (XBinary::isPdStructNotCanceled(pPdStruct)) {
        if (nChunkCount++ >= PNG_MAX_CHUNK_COUNT) {
            return 0;
        }

        const CHUNK currentChunk = _readChunk(nOffset);

        if (!currentChunk.bValid) {
            return 0;
        }

        if (!_isChunkCRCValid(currentChunk, pPdStruct)) {
            return 0;
        }

        nOffset += 12 + currentChunk.nDataSize;

        if (currentChunk.sName == "IHDR") {
            if (currentChunk.nDataOffset != 16) {
                return 0;
            }
        } else if (currentChunk.sName == "PLTE") {
            const quint32 nPaletteEntries = currentChunk.nDataSize / 3;
            if (bHasPalette || bHasImageData || (currentChunk.nDataSize == 0) || ((currentChunk.nDataSize % 3) != 0) || (nPaletteEntries > 256) ||
                ((colorType == COLOR_TYPE_PALETTE) && (nPaletteEntries > (1U << read_uint8(chunk.nDataOffset + 8)))) || (colorType == COLOR_TYPE_GRAYSCALE) ||
                (colorType == COLOR_TYPE_GRAYSCALE_ALPHA)) {
                return 0;
            }
            bHasPalette = true;
        } else if (currentChunk.sName == "IDAT") {
            if (bImageDataEnded || ((colorType == COLOR_TYPE_PALETTE) && !bHasPalette)) {
                return 0;
            }
            bHasImageData = true;
            bHasNonemptyImageData |= currentChunk.nDataSize != 0;
        } else if (currentChunk.sName == "IEND") {
            return (currentChunk.nDataSize == 0) && bHasImageData && bHasNonemptyImageData ? nOffset : 0;
        } else {
            bImageDataEnded |= bHasImageData;

            // Unknown critical chunks cannot be safely interpreted.  The four
            // critical chunk types recognized by PNG are handled above (IHDR
            // is required to be first).
            if (!currentChunk.sName.isEmpty() && currentChunk.sName.at(0).isUpper()) {
                return 0;
            }
        }
    }

    return 0;
}

bool XPNG::isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    XPNG xpng(pDevice);

    return xpng.isValid(pPdStruct);
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
    return _getStructuredSize(pPdStruct);
}

QString XPNG::getMIMEString()
{
    return "image/png";
}

QString XPNG::getInfo(PDSTRUCT *pPdStruct)
{
    QString sResult;

    if (isValid(pPdStruct)) {
        IHDR ihdr = getIHDR(pPdStruct);

        if (ihdr.nWidth && ihdr.nHeight) {
            QString sSchema;

            switch (ihdr.nColorType) {
                case COLOR_TYPE_GRAYSCALE: sSchema = QString("Grayscale"); break;
                case COLOR_TYPE_RGB: sSchema = QString("RGB"); break;
                case COLOR_TYPE_PALETTE: sSchema = QString("Palette"); break;
                case COLOR_TYPE_GRAYSCALE_ALPHA: sSchema = QString("Grayscale+Alpha"); break;
                case COLOR_TYPE_RGBA: sSchema = QString("RGBA"); break;
                default: sSchema = QString("Unknown(%1)").arg(ihdr.nColorType); break;
            }

            sResult = QString("%1x%2, %3 bits, %4").arg(ihdr.nWidth).arg(ihdr.nHeight).arg(ihdr.nBitDepth).arg(sSchema);

            // Append pHYs info if available
            XPNG::pHYs phys = getpHYs(pPdStruct);
            if (phys.nPixelsPerUnitX || phys.nPixelsPerUnitY) {
                QString sUnit = (phys.nUnitSpecifier == 1) ? QString("meter") : QString("unknown");
                sResult += QString(", pHYs: %1x%2 %3").arg(phys.nPixelsPerUnitX).arg(phys.nPixelsPerUnitY).arg(sUnit);
            }

            // Append bKGD info if available
            XPNG::bKGD bkgd = getbKGD(pPdStruct);
            if (bkgd.nType == 1) {
                sResult += QString(", bKGD: gray=%1").arg(bkgd.nGray);
            } else if (bkgd.nType == 2) {
                sResult += QString(", bKGD: rgb=(%1,%2,%3)").arg(bkgd.nRed).arg(bkgd.nGreen).arg(bkgd.nBlue);
            } else if (bkgd.nType == 3) {
                sResult += QString(", bKGD: paletteIndex=%1").arg((quint32)bkgd.nPaletteIndex);
            }
        }
    }

    if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
        sResult.clear();
    }

    return sResult;
}

QVector<XBinary::XMETADATA_STRUCT> XPNG::getMetadataStructs()
{
    QVector<XMETADATA_STRUCT> listResult;
    qint64 nOffset = 8;
    qint32 nChunkCount = 0;

    const CHUNK ihdr = _readChunk(nOffset);
    if (ihdr.bValid && (ihdr.sName == QString("IHDR")) && (ihdr.nDataSize == 13) && _isChunkCRCValid(ihdr, nullptr)) {
        const XMetadataAppender appendMetadata(this, &listResult, ihdr.nDataOffset);

        appendMetadata(0, 4, XMETADATA_ID_FRAME_WIDTH, QString("Width"), read_uint32(ihdr.nDataOffset, true));
        appendMetadata(4, 4, XMETADATA_ID_FRAME_HEIGHT, QString("Height"), read_uint32(ihdr.nDataOffset + 4, true));
        appendMetadata(8, 1, XMETADATA_ID_BIT_DEPTH, QString("Bit depth"), read_uint8(ihdr.nDataOffset + 8));
        appendMetadata(9, 1, XMETADATA_ID_COLOR_TYPE, QString("Color type"), read_uint8(ihdr.nDataOffset + 9));
    }

    while ((nOffset + 12 <= getSize()) && (nChunkCount++ < PNG_MAX_CHUNK_COUNT)) {
        const CHUNK chunk = _readChunk(nOffset);
        if (!chunk.bValid) {
            break;
        }

        if ((chunk.sName == QString("tIME")) && (chunk.nDataSize == 7) && _isChunkCRCValid(chunk, nullptr)) {
            const QDate date(read_uint16(chunk.nDataOffset, true), read_uint8(chunk.nDataOffset + 2), read_uint8(chunk.nDataOffset + 3));
            const QTime time(read_uint8(chunk.nDataOffset + 4), read_uint8(chunk.nDataOffset + 5), read_uint8(chunk.nDataOffset + 6));
            const QDateTime dateTime(date, time, Qt::UTC);

            if (dateTime.isValid()) {
                XMETADATA_STRUCT record = {};
                record.nOffset = chunk.nDataOffset;
                record.nSize = chunk.nDataSize;
                record.nAddress = offsetToAddress(chunk.nDataOffset);
                record.id = XMETADATA_ID_MODIFICATED;
                record.sName = QString("Last modification time");
                record.varValue = dateTime;
                listResult.append(record);
            }
        }

        nOffset += 12 + chunk.nDataSize;
        if (chunk.sName == QString("IEND")) {
            break;
        }
    }

    return listResult;
}

XBinary::ENDIAN XPNG::getEndian()
{
    return ENDIAN_BIG;  // PNG is always big-endian
}

XBinary::_MEMORY_MAP XPNG::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    XBinary::_MEMORY_MAP result = {};

    if (mapMode == MAPMODE_UNKNOWN) {
        mapMode = MAPMODE_REGIONS;  // Default mode
    }

    if (mapMode == MAPMODE_REGIONS) {
        result = _getMemoryMap(FILEPART_HEADER | FILEPART_REGION | FILEPART_OVERLAY, pPdStruct);
    }

    return result;
}

XPNG::CHUNK XPNG::_readChunk(qint64 nOffset)
{
    CHUNK result = {};

    const qint64 nTotalSize = getSize();

    if ((nOffset < 0) || (nOffset > nTotalSize - 12)) {
        return result;
    }

    result.nDataSize = read_uint32(nOffset, true);

    if (result.nDataSize > nTotalSize - nOffset - 12) {
        return result;
    }

    result.nDataOffset = nOffset + 8;
    result.sName = read_ansiString(nOffset + 4, 4);

    if (result.sName.size() != 4) {
        return CHUNK();
    }

    for (qint32 i = 0; i < 4; i++) {
        const QChar c = result.sName.at(i);
        if (!((c >= QChar('A')) && (c <= QChar('Z'))) && !((c >= QChar('a')) && (c <= QChar('z')))) {
            return CHUNK();
        }
    }

    result.nCRC = read_uint32(nOffset + 8 + result.nDataSize, true);
    result.bValid = true;

    return result;
}

bool XPNG::_isChunkCRCValid(const CHUNK &chunk, PDSTRUCT *pPdStruct)
{
    if (!chunk.bValid || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    const qint32 nRequestedBufferSize = getBufferSize(pPdStruct);

    if (nRequestedBufferSize <= 0) {
        return false;
    }

    const qint32 nBufferSize = qBound((qint32)0x1000, nRequestedBufferSize, (qint32)0x100000);
    char *pBuffer = new (std::nothrow) char[nBufferSize];

    if (!pBuffer) {
        return false;
    }

    qint64 nOffset = chunk.nDataOffset - 4;
    qint64 nRemaining = 4 + chunk.nDataSize;
    quint32 nCRC = 0xFFFFFFFF;
    bool bReadError = false;

    while ((nRemaining > 0) && XBinary::isPdStructNotCanceled(pPdStruct)) {
        const qint32 nToRead = (qint32)qMin<qint64>(nBufferSize, nRemaining);

        if (read_array_process(nOffset, pBuffer, nToRead, pPdStruct) != nToRead) {
            bReadError = true;
            break;
        }

        nCRC = XBinary::_getCRC32(pBuffer, nToRead, nCRC, XBinary::_getCRC32Table_EDB88320());
        nOffset += nToRead;
        nRemaining -= nToRead;
    }

    delete[] pBuffer;

    return !bReadError && (nRemaining == 0) && XBinary::isPdStructNotCanceled(pPdStruct) && ((nCRC ^ 0xFFFFFFFF) == chunk.nCRC);
}

bool XPNG::createPNG(QIODevice *pDevice, quint32 nWidth, quint32 nHeight, const QByteArray &baImageData, COLOR_TYPE colorType, quint8 nBitDepth)
{
    if (!pDevice || !pDevice->isWritable() || (nWidth == 0) || (nHeight == 0) || !isValidPngColorDepth(colorType, nBitDepth)) {
        return false;
    }

    // Create IHDR structure
    IHDR ihdr = {};
    ihdr.nWidth = nWidth;
    ihdr.nHeight = nHeight;
    ihdr.nBitDepth = nBitDepth;
    ihdr.nColorType = (quint8)colorType;
    ihdr.nCompression = 0;  // Always 0 (deflate)
    ihdr.nFilter = 0;       // Always 0 (adaptive filtering)
    ihdr.nInterlace = 0;    // 0 = no interlace

    QByteArray _baImageData = _convertImageData(baImageData.data(), baImageData.size(), nWidth, nHeight, colorType, nBitDepth);

    if (_baImageData.isEmpty()) {
        return false;
    }

    // Complete all fallible preparation before touching the destination.  In
    // particular, builds without archive support cannot compress PNG data and
    // must not leave a signature/IHDR prefix behind on failure.
    QByteArray compressedData = _compressData(_baImageData);
    if (compressedData.isEmpty()) {
        return false;
    }

    // Write PNG signature
    const char pngSignature[8] = {'\x89', 'P', 'N', 'G', '\r', '\n', '\x1a', '\n'};
    if (!pngWriteAll(pDevice, pngSignature, sizeof(pngSignature))) {
        return false;
    }

    // Create IHDR chunk data using the struct
    QByteArray ihdrData;

    // Convert to big-endian and append
    quint32 nWidthBE = qToBigEndian(ihdr.nWidth);
    quint32 nHeightBE = qToBigEndian(ihdr.nHeight);

    ihdrData.append((char *)&nWidthBE, 4);   // Width (big-endian)
    ihdrData.append((char *)&nHeightBE, 4);  // Height (big-endian)
    ihdrData.append(ihdr.nBitDepth);         // Bit depth
    ihdrData.append(ihdr.nColorType);        // Color type
    ihdrData.append(ihdr.nCompression);      // Compression method
    ihdrData.append(ihdr.nFilter);           // Filter method
    ihdrData.append(ihdr.nInterlace);        // Interlace method

    // Write IHDR chunk
    if (!_writeChunk(pDevice, "IHDR", ihdrData)) {
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

bool XPNG::createPNGIndexed(QIODevice *pDevice, quint32 nWidth, quint32 nHeight, const QByteArray &baImageData, const QByteArray &baPalette, quint8 nBitDepth)
{
    if (!pDevice || !pDevice->isWritable() || (nWidth == 0) || (nHeight == 0) || !isValidPngColorDepth(COLOR_TYPE_PALETTE, nBitDepth)) {
        return false;
    }

    const qint32 nPaletteEntries = baPalette.size() / 3;
    if (baPalette.isEmpty() || ((baPalette.size() % 3) != 0) || (nPaletteEntries > 256) || (nPaletteEntries > (1 << nBitDepth))) {
        return false;
    }

    // Create IHDR structure
    IHDR ihdr = {};
    ihdr.nWidth = nWidth;
    ihdr.nHeight = nHeight;
    ihdr.nBitDepth = nBitDepth;
    ihdr.nColorType = (quint8)COLOR_TYPE_PALETTE;
    ihdr.nCompression = 0;
    ihdr.nFilter = 0;
    ihdr.nInterlace = 0;

    QByteArray _baImageData = _convertImageData(baImageData.data(), baImageData.size(), nWidth, nHeight, COLOR_TYPE_PALETTE, nBitDepth);

    if (_baImageData.isEmpty()) {
        return false;
    }

    QByteArray compressedData = _compressData(_baImageData);

    if (compressedData.isEmpty()) {
        return false;
    }

    // Write PNG signature
    const char pngSignature[8] = {'\x89', 'P', 'N', 'G', '\r', '\n', '\x1a', '\n'};

    if (!pngWriteAll(pDevice, pngSignature, sizeof(pngSignature))) {
        return false;
    }

    // Create IHDR chunk data
    QByteArray ihdrData;

    quint32 nWidthBE = qToBigEndian(ihdr.nWidth);
    quint32 nHeightBE = qToBigEndian(ihdr.nHeight);

    ihdrData.append((char *)&nWidthBE, 4);
    ihdrData.append((char *)&nHeightBE, 4);
    ihdrData.append(ihdr.nBitDepth);
    ihdrData.append(ihdr.nColorType);
    ihdrData.append(ihdr.nCompression);
    ihdrData.append(ihdr.nFilter);
    ihdrData.append(ihdr.nInterlace);

    if (!_writeChunk(pDevice, "IHDR", ihdrData)) {
        return false;
    }

    // Write PLTE chunk (palette data in RGB format, 3 bytes per entry)
    if (!_writeChunk(pDevice, "PLTE", baPalette)) {
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
    if (!pngWriteAll(pDevice, reinterpret_cast<const char *>(&nDataLength), sizeof(nDataLength))) {
        return false;
    }

    // Write chunk type
    QByteArray chunkTypeBytes = sChunkType.toLatin1();
    if (!pngWriteAll(pDevice, chunkTypeBytes.constData(), chunkTypeBytes.size())) {
        return false;
    }

    // Write data
    if (!data.isEmpty() && !pngWriteAll(pDevice, data.constData(), data.size())) {
        return false;
    }

    // Calculate and write CRC
    quint32 *pCrcTable = XBinary::_getCRC32Table_EDB88320();
    quint32 nCRCValue = _getCRC32(chunkTypeBytes, 0xFFFFFFFF, pCrcTable);
    if (!data.isEmpty()) {
        nCRCValue = _getCRC32(data, nCRCValue, pCrcTable);
    }
    quint32 nCRC = qToBigEndian(nCRCValue ^ 0xFFFFFFFF);
    if (!pngWriteAll(pDevice, reinterpret_cast<const char *>(&nCRC), sizeof(nCRC))) {
        return false;
    }

    return true;
}

QByteArray XPNG::_compressData(const QByteArray &data)
{
    QByteArray result;
    // Use XDeflateDecoder for compression

    if (data.isEmpty()) {
        return result;
    }

    // Create input and output devices
    QBuffer inputBuffer;
    inputBuffer.setData(data);
    if (!inputBuffer.open(QIODevice::ReadOnly)) {
        return result;
    }

    QBuffer outputBuffer;
    if (!outputBuffer.open(QIODevice::WriteOnly)) {
        return result;
    }

    // Setup compression state
    XBinary::DATAPROCESS_STATE compressState = {};
    compressState.pDeviceInput = &inputBuffer;
    compressState.pDeviceOutput = &outputBuffer;
    compressState.nInputOffset = 0;
    compressState.nInputLimit = data.size();
    compressState.nProcessedOffset = 0;
    compressState.nProcessedLimit = -1;
    compressState.bReadError = false;
    compressState.bWriteError = false;
    compressState.nCountInput = 0;
    compressState.nCountOutput = 0;

    // Compress using XDeflateDecoder (actually compresses despite the class name)
    bool success = XDeflateDecoder::compress_zlib(&compressState);

    inputBuffer.close();
    outputBuffer.close();

    if (success && !compressState.bReadError && !compressState.bWriteError) {
        result = outputBuffer.data();
    }

    return result;
}

QByteArray XPNG::_convertImageData(const char *pData, qint32 nDataSize, quint32 nWidth, quint32 nHeight, COLOR_TYPE colorType, quint8 nBitDepth)
{
    QByteArray baResult;

    if (!pData || (nDataSize < 0) || (nWidth == 0) || (nHeight == 0) || !isValidPngColorDepth(colorType, nBitDepth)) {
        return baResult;
    }

    // Calculate samples per pixel based on color type
    qint32 nSamplesPerPixel = 0;
    switch (colorType) {
        case COLOR_TYPE_GRAYSCALE: nSamplesPerPixel = 1; break;
        case COLOR_TYPE_RGB: nSamplesPerPixel = 3; break;
        case COLOR_TYPE_PALETTE: nSamplesPerPixel = 1; break;
        case COLOR_TYPE_GRAYSCALE_ALPHA: nSamplesPerPixel = 2; break;
        case COLOR_TYPE_RGBA: nSamplesPerPixel = 4; break;
    }

    if (nSamplesPerPixel > 0) {
        const qint64 nBitsPerRow = (qint64)nWidth * nSamplesPerPixel * nBitDepth;
        const qint64 nBytesPerRow = (nBitsPerRow + 7) / 8;
        const qint64 nFilteredRowSize = nBytesPerRow + 1;

        if ((nBytesPerRow > (std::numeric_limits<qint32>::max)()) || (nFilteredRowSize > PNG_MAX_ENCODE_BUFFER_SIZE / nHeight) ||
            (nBytesPerRow > nDataSize / (qint64)nHeight)) {
            return baResult;
        }

        const qint64 nTotalSize = nFilteredRowSize * nHeight;

        if ((nTotalSize <= 0) || (nTotalSize > PNG_MAX_ENCODE_BUFFER_SIZE) || (nTotalSize > (std::numeric_limits<int>::max)())) {
            return baResult;
        }

        baResult.resize((qint32)nTotalSize);
        baResult.fill(0);  // Initialize with zeros (filter byte 0 = None)

        for (quint32 y = 0; y < nHeight; y++) {
            const qint64 nOutputRowOffset = (qint64)y * nFilteredRowSize;
            const qint64 nInputRowOffset = (qint64)y * nBytesPerRow;

            // Filter byte is already 0 (None) from fill
            memcpy(baResult.data() + nOutputRowOffset + 1, pData + nInputRowOffset, (size_t)nBytesPerRow);
        }
    }

    return baResult;
}

XPNG::IHDR XPNG::getIHDR(PDSTRUCT *pPdStruct)
{
    IHDR result = {};

    if (isValid(pPdStruct)) {
        // IHDR is always the first chunk after the PNG signature (at offset 8)
        CHUNK ihdrChunk = _readChunk(8);

        if (ihdrChunk.bValid && ihdrChunk.sName == "IHDR" && ihdrChunk.nDataSize == 13) {
            result.nWidth = read_uint32(ihdrChunk.nDataOffset, true);       // Big-endian
            result.nHeight = read_uint32(ihdrChunk.nDataOffset + 4, true);  // Big-endian
            result.nBitDepth = read_uint8(ihdrChunk.nDataOffset + 8);
            result.nColorType = read_uint8(ihdrChunk.nDataOffset + 9);
            result.nCompression = read_uint8(ihdrChunk.nDataOffset + 10);
            result.nFilter = read_uint8(ihdrChunk.nDataOffset + 11);
            result.nInterlace = read_uint8(ihdrChunk.nDataOffset + 12);
        }
    }

    return result;
}

XPNG::pHYs XPNG::getpHYs(PDSTRUCT *pPdStruct)
{
    XPNG::pHYs result = {};

    if (!isValid(pPdStruct)) {
        return result;
    }

    qint64 nOffset = 8;  // After signature
    qint64 nTotalSize = getSize();
    qint32 nChunkCount = 0;

    while ((nOffset + 12 <= nTotalSize) && XBinary::isPdStructNotCanceled(pPdStruct)) {
        if (nChunkCount++ >= PNG_MAX_CHUNK_COUNT) {
            break;
        }

        CHUNK chunk = _readChunk(nOffset);

        if (!chunk.bValid) {
            break;
        }

        if (chunk.sName == "pHYs") {
            if (chunk.nDataSize == 9) {
                result.nPixelsPerUnitX = read_uint32(chunk.nDataOffset + 0, true);
                result.nPixelsPerUnitY = read_uint32(chunk.nDataOffset + 4, true);
                result.nUnitSpecifier = read_uint8(chunk.nDataOffset + 8);
            }
            break;
        }

        if (chunk.sName == "IEND") {
            break;
        }

        // Advance to next chunk: length(4) + type(4) + data + crc(4)
        nOffset += (12 + chunk.nDataSize);
    }

    if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
        return {};
    }

    return result;
}

XPNG::bKGD XPNG::getbKGD(PDSTRUCT *pPdStruct)
{
    XPNG::bKGD result = {};

    if (!isValid(pPdStruct)) {
        return result;
    }

    // We may use IHDR color type to infer expected layout, but chunk itself defines size
    IHDR ihdr = getIHDR(pPdStruct);

    qint64 nOffset = 8;  // After signature
    qint64 nTotalSize = getSize();
    qint32 nChunkCount = 0;

    while ((nOffset + 12 <= nTotalSize) && XBinary::isPdStructNotCanceled(pPdStruct)) {
        if (nChunkCount++ >= PNG_MAX_CHUNK_COUNT) {
            break;
        }

        CHUNK chunk = _readChunk(nOffset);

        if (!chunk.bValid) {
            break;
        }

        if (chunk.sName == "bKGD") {
            quint32 nLen = (quint32)chunk.nDataSize;
            if (nLen == 1) {
                result.nPaletteIndex = read_uint8(chunk.nDataOffset + 0);
                result.nType = 3;  // indexed
            } else if (nLen == 2) {
                result.nGray = read_uint16(chunk.nDataOffset + 0, true);
                result.nType = 1;  // grayscale
            } else if (nLen == 6) {
                result.nRed = read_uint16(chunk.nDataOffset + 0, true);
                result.nGreen = read_uint16(chunk.nDataOffset + 2, true);
                result.nBlue = read_uint16(chunk.nDataOffset + 4, true);
                result.nType = 2;  // truecolor
            } else {
                result.nType = 0;  // unknown or unsupported
            }
            break;
        }

        if (chunk.sName == "IEND") {
            break;
        }

        nOffset += (12 + chunk.nDataSize);
    }

    // Validate against color type if present
    if (result.nType != 0) {
        // If mismatch between IHDR color type and bKGD content, still return parsed values; caller can decide.
        Q_UNUSED(ihdr)
    }

    if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
        return {};
    }

    return result;
}

QString XPNG::structIDToString(quint32 nID)
{
    return XBinary::XCONVERT_idToTransString(nID, _TABLE_XPNG_STRUCTID, sizeof(_TABLE_XPNG_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QString XPNG::structIDToFtString(quint32 nID)
{
    return XBinary::XCONVERT_idToFtString(nID, _TABLE_XPNG_STRUCTID, sizeof(_TABLE_XPNG_STRUCTID) / sizeof(XBinary::XCONVERT));
}

quint32 XPNG::ftStringToStructID(const QString &sFtString)
{
    return XCONVERT_ftStringToId(sFtString, _TABLE_XPNG_STRUCTID, sizeof(_TABLE_XPNG_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QList<XBinary::XFHEADER> XPNG::getXFHeaders(const XFSTRUCT &xfStruct, PDSTRUCT *pPdStruct)
{
    QList<XBinary::XFHEADER> listResult;

    if (!isValid(pPdStruct)) {
        return listResult;
    }

    quint32 nStructID = xfStruct.nStructID;

    if (nStructID == STRUCTID_UNKNOWN) {
        XFSTRUCT _xfStruct = xfStruct;
        _xfStruct.nStructID = STRUCTID_SIGNATURE;
        _xfStruct.xLoc = offsetToLoc(0);
        listResult.append(getXFHeaders(_xfStruct, pPdStruct));
    } else if (nStructID == STRUCTID_SIGNATURE) {
        XLOC headerLoc = xfStruct.xLoc;
        if (headerLoc.locType == LT_UNKNOWN) {
            headerLoc = offsetToLoc(0);
        }

        XFHEADER xfHeader = {};
        xfHeader.sParentTag = xfStruct.sParent;
        xfHeader.fileType = xfStruct.fileType;
        xfHeader.structID = static_cast<XBinary::STRUCTID>(STRUCTID_SIGNATURE);
        xfHeader.xLoc = headerLoc;
        xfHeader.nSize = 8;
        xfHeader.xfType = XFTYPE_HEADER;
        xfHeader.listFields = getXFRecords(xfStruct.fileType, STRUCTID_SIGNATURE, headerLoc);
        xfHeader.sTag = xfHeaderToTag(xfHeader, structIDToString(STRUCTID_SIGNATURE), xfHeader.sParentTag);
        listResult.append(xfHeader);

        if (xfStruct.bIsParent) {
            XFSTRUCT _xfStruct = xfStruct;
            _xfStruct.sParent = xfHeader.sTag;

            _xfStruct.nStructID = STRUCTID_IHDR;
            _xfStruct.xLoc = offsetToLoc(16);
            listResult.append(getXFHeaders(_xfStruct, pPdStruct));

            _xfStruct.nStructID = STRUCTID_CHUNK;
            _xfStruct.xLoc = offsetToLoc(8);
            listResult.append(getXFHeaders(_xfStruct, pPdStruct));
        }
    } else if (nStructID == STRUCTID_IHDR) {
        XLOC headerLoc = xfStruct.xLoc;
        if (headerLoc.locType == LT_UNKNOWN) {
            headerLoc = offsetToLoc(16);  // Signature(8) + Length(4) + Type(4)
        }

        XFHEADER xfHeader = {};
        xfHeader.sParentTag = xfStruct.sParent;
        xfHeader.fileType = xfStruct.fileType;
        xfHeader.structID = static_cast<XBinary::STRUCTID>(STRUCTID_IHDR);
        xfHeader.xLoc = headerLoc;
        xfHeader.nSize = 13;
        xfHeader.xfType = XFTYPE_HEADER;
        xfHeader.listFields = getXFRecords(xfStruct.fileType, STRUCTID_IHDR, headerLoc);
        xfHeader.sTag = xfHeaderToTag(xfHeader, structIDToString(STRUCTID_IHDR), xfHeader.sParentTag);
        listResult.append(xfHeader);
    } else if (nStructID == STRUCTID_CHUNK) {
        qint64 nStartOffset = locToOffset(xfStruct.pMemoryMap, xfStruct.xLoc);

        if (nStartOffset == -1) {
            nStartOffset = 8;
        }

        qint64 nFileSize = getSize();

        XFHEADER xfHeader = {};
        xfHeader.sParentTag = xfStruct.sParent;
        xfHeader.fileType = xfStruct.fileType;
        xfHeader.structID = static_cast<XBinary::STRUCTID>(STRUCTID_CHUNK);
        xfHeader.xLoc = offsetToLoc(nStartOffset);
        xfHeader.xfType = XFTYPE_TABLE;
        xfHeader.listFields = getXFRecords(xfStruct.fileType, STRUCTID_CHUNK, xfHeader.xLoc);

        qint64 nCurrentOffset = nStartOffset;
        qint32 nChunkCount = 0;

        while ((nCurrentOffset >= 0) && (nCurrentOffset <= nFileSize - 12) && XBinary::isPdStructNotCanceled(pPdStruct)) {
            if (nChunkCount++ >= PNG_MAX_CHUNK_COUNT) {
                xfHeader.listRowLocations.clear();
                break;
            }

            const CHUNK chunk = _readChunk(nCurrentOffset);

            if (!chunk.bValid) {
                break;
            }

            xfHeader.listRowLocations.append(nCurrentOffset);

            nCurrentOffset += 12 + chunk.nDataSize;

            if (chunk.sName == "IEND") {
                break;
            }
        }

        if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
            return listResult;
        }

        if (!xfHeader.listRowLocations.isEmpty()) {
            xfHeader.sTag = xfHeaderToTag(xfHeader, structIDToString(STRUCTID_CHUNK), xfHeader.sParentTag);
            listResult.append(xfHeader);
        }
    }

    if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
        listResult.clear();
    }

    return listResult;
}

QList<XBinary::XFRECORD> XPNG::getXFRecords(FT fileType, quint32 nStructID, const XLOC &xLoc)
{
    Q_UNUSED(fileType)
    Q_UNUSED(xLoc)

    QList<XBinary::XFRECORD> listResult;

    // PNG integers are big-endian
    if (nStructID == STRUCTID_SIGNATURE) {
        listResult.append({"Signature", 0, 8, XFRECORD_FLAG_NONE, VT_BYTE_ARRAY});
    } else if (nStructID == STRUCTID_CHUNK) {
        listResult.append({"Length", 0, 4, XFRECORD_FLAG_BE | XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"Type", 4, 4, XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
    } else if (nStructID == STRUCTID_IHDR) {
        listResult.append({"Width", 0, 4, XFRECORD_FLAG_BE, VT_UINT32});
        listResult.append({"Height", 4, 4, XFRECORD_FLAG_BE, VT_UINT32});
        listResult.append({"BitDepth", 8, 1, XFRECORD_FLAG_NONE, VT_UINT8});
        listResult.append({"ColorType", 9, 1, XFRECORD_FLAG_NONE, VT_UINT8});
        listResult.append({"Compression", 10, 1, XFRECORD_FLAG_NONE, VT_UINT8});
        listResult.append({"Filter", 11, 1, XFRECORD_FLAG_NONE, VT_UINT8});
        listResult.append({"Interlace", 12, 1, XFRECORD_FLAG_NONE, VT_UINT8});
    } else if (nStructID == STRUCTID_pHYs) {
        listResult.append({"PixelsPerUnitX", 0, 4, XFRECORD_FLAG_BE, VT_UINT32});
        listResult.append({"PixelsPerUnitY", 4, 4, XFRECORD_FLAG_BE, VT_UINT32});
        listResult.append({"UnitSpecifier", 8, 1, XFRECORD_FLAG_NONE, VT_UINT8});
    } else if (nStructID == STRUCTID_tIME) {
        listResult.append({"Year", 0, 2, XFRECORD_FLAG_BE, VT_UINT16});
        listResult.append({"Month", 2, 1, XFRECORD_FLAG_NONE, VT_UINT8});
        listResult.append({"Day", 3, 1, XFRECORD_FLAG_NONE, VT_UINT8});
        listResult.append({"Hour", 4, 1, XFRECORD_FLAG_NONE, VT_UINT8});
        listResult.append({"Minute", 5, 1, XFRECORD_FLAG_NONE, VT_UINT8});
        listResult.append({"Second", 6, 1, XFRECORD_FLAG_NONE, VT_UINT8});
    }

    return listResult;
}

// QList<XBinary::DATA_HEADER> XPNG::getDataHeaders(const DATA_HEADERS_OPTIONS &dataHeadersOptions, PDSTRUCT *pPdStruct)
// {
//     QList<DATA_HEADER> listResult;

//     if (dataHeadersOptions.nID == STRUCTID_UNKNOWN) {
//         DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
//         _dataHeadersOptions.bChildren = true;
//         _dataHeadersOptions.dsID_parent = _addDefaultHeaders(&listResult, pPdStruct);
//         _dataHeadersOptions.dhMode = XBinary::DHMODE_HEADER;
//         _dataHeadersOptions.fileType = dataHeadersOptions.pMemoryMap->fileType;

//         _dataHeadersOptions.nID = STRUCTID_SIGNATURE;
//         _dataHeadersOptions.nLocation = 0;
//         _dataHeadersOptions.locType = XBinary::LT_OFFSET;

//         listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//     } else {
//         qint64 nStartOffset = locationToOffset(dataHeadersOptions.pMemoryMap, dataHeadersOptions.locType, dataHeadersOptions.nLocation);

//         if (nStartOffset != -1) {
//             if (dataHeadersOptions.nID == STRUCTID_SIGNATURE) {
//                 DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, XPNG::structIDToString(dataHeadersOptions.nID));
//                 dataHeader.nSize = 8;

//                 dataHeader.listRecords.append(getDataRecord(0, 8, "Signature", VT_BYTE_ARRAY, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));

//                 listResult.append(dataHeader);

//                 if (dataHeadersOptions.bChildren) {
//                     qint64 nCurrentOffset = 8;  // Start after the PNG signature
//                     qint64 nTotalSize = getSize();
//                     qint32 nNumberOfChunks = 0;

//                     while (XBinary::isPdStructNotCanceled(pPdStruct)) {
//                         qint64 nDataSize = read_uint32(nCurrentOffset, true);
//                         QString sTag = read_ansiString(nCurrentOffset + 4, 4);

//                         if (nCurrentOffset + nDataSize + 12 > nTotalSize) {
//                             break;  // Prevent reading beyond the file size
//                         }

//                         nNumberOfChunks++;

//                         // End Tag
//                         if (sTag == "IEND") {
//                             break;
//                         }

//                         nCurrentOffset += (12 + nDataSize);
//                     }

//                     DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;

//                     _dataHeadersOptions.dhMode = XBinary::DHMODE_TABLE;
//                     _dataHeadersOptions.fileType = dataHeadersOptions.pMemoryMap->fileType;
//                     _dataHeadersOptions.nID = STRUCTID_CHUNK;
//                     _dataHeadersOptions.locType = LT_OFFSET;
//                     _dataHeadersOptions.nLocation = 8;  // Start after the PNG signature;
//                     _dataHeadersOptions.nCount = nNumberOfChunks;
//                     _dataHeadersOptions.nSize = nCurrentOffset - 8;

//                     listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//                 }
//             } else if (dataHeadersOptions.nID == STRUCTID_CHUNK) {
//                 DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, XPNG::structIDToString(dataHeadersOptions.nID));

//                 quint32 nDataSize = read_uint32(nStartOffset, true);

//                 dataHeader.nSize = 12 + nDataSize;

//                 dataHeader.listRecords.append(getDataRecord(0, 4, "Length", XBinary::VT_UINT32, DRF_SIZE, XBinary::ENDIAN_BIG));
//                 dataHeader.listRecords.append(getDataRecord(4, 4, "Type", XBinary::VT_CHAR_ARRAY, DRF_UNKNOWN, XBinary::ENDIAN_BIG));
//                 dataHeader.listRecords.append(getDataRecord(8 + nDataSize, 4, "CRC", XBinary::VT_UINT32, 0, XBinary::ENDIAN_BIG));

//                 listResult.append(dataHeader);

//                 if (dataHeadersOptions.bChildren) {
//                     qint64 nCurrentOffset = nStartOffset;
//                     qint64 nLocation = dataHeadersOptions.nLocation;

//                     for (int i = 0; i < dataHeader.nCount; i++) {
//                         qint64 nDataSize = read_uint32(nCurrentOffset, true);
//                         QString sTag = read_ansiString(nCurrentOffset + 4, 4);

//                         if (sTag == "IHDR") {
//                             DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;

//                             _dataHeadersOptions.dhMode = XBinary::DHMODE_HEADER;
//                             _dataHeadersOptions.fileType = dataHeadersOptions.pMemoryMap->fileType;
//                             _dataHeadersOptions.nID = STRUCTID_IHDR;
//                             _dataHeadersOptions.locType = LT_OFFSET;
//                             _dataHeadersOptions.nLocation = nLocation;
//                             _dataHeadersOptions.nSize = nDataSize + 12;

//                             listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//                         } else if (sTag == "pHYs") {
//                             DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;

//                             _dataHeadersOptions.dhMode = XBinary::DHMODE_HEADER;
//                             _dataHeadersOptions.fileType = dataHeadersOptions.pMemoryMap->fileType;
//                             _dataHeadersOptions.nID = STRUCTID_pHYs;
//                             _dataHeadersOptions.locType = LT_OFFSET;
//                             _dataHeadersOptions.nLocation = nLocation;
//                             _dataHeadersOptions.nSize = nDataSize + 12;

//                             listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//                         } else if (sTag == "bKGD") {
//                             DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;

//                             _dataHeadersOptions.dhMode = XBinary::DHMODE_HEADER;
//                             _dataHeadersOptions.fileType = dataHeadersOptions.pMemoryMap->fileType;
//                             _dataHeadersOptions.nID = STRUCTID_bKGD;
//                             _dataHeadersOptions.locType = LT_OFFSET;
//                             _dataHeadersOptions.nLocation = nLocation;
//                             _dataHeadersOptions.nSize = nDataSize + 12;

//                             listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//                         }

//                         nCurrentOffset += (12 + nDataSize);
//                         nLocation += (12 + nDataSize);
//                     }
//                 }
//             } else if (dataHeadersOptions.nID == STRUCTID_IHDR) {
//                 DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, XPNG::structIDToString(dataHeadersOptions.nID));

//                 dataHeader.nSize = 25;  // IHDR size is always 25 bytes
//                 dataHeader.listRecords.append(getDataRecord(0, 4, "Length", XBinary::VT_UINT32, DRF_SIZE, XBinary::ENDIAN_BIG));
//                 dataHeader.listRecords.append(getDataRecord(4, 4, "Type", XBinary::VT_CHAR_ARRAY, DRF_UNKNOWN, XBinary::ENDIAN_BIG));
//                 dataHeader.listRecords.append(getDataRecord(8, 4, "Width", XBinary::VT_UINT32, DRF_COUNT, XBinary::ENDIAN_BIG));
//                 dataHeader.listRecords.append(getDataRecord(12, 4, "Height", XBinary::VT_UINT32, DRF_COUNT, XBinary::ENDIAN_BIG));
//                 dataHeader.listRecords.append(getDataRecord(16, 1, "Bit Depth", XBinary::VT_UINT8, DRF_COUNT, XBinary::ENDIAN_BIG));
//                 dataHeader.listRecords.append(getDataRecord(17, 1, "Color Type", XBinary::VT_UINT8, DRF_UNKNOWN, XBinary::ENDIAN_BIG));
//                 dataHeader.listRecords.append(getDataRecord(18, 1, "Compression", XBinary::VT_UINT8, DRF_UNKNOWN, XBinary::ENDIAN_BIG));
//                 dataHeader.listRecords.append(getDataRecord(19, 1, "Filter", XBinary::VT_UINT8, DRF_UNKNOWN, XBinary::ENDIAN_BIG));
//                 dataHeader.listRecords.append(getDataRecord(20, 1, "Interlace", XBinary::VT_UINT8, DRF_UNKNOWN, XBinary::ENDIAN_BIG));
//                 dataHeader.listRecords.append(getDataRecord(21, 4, "CRC", XBinary::VT_UINT32, DRF_UNKNOWN, XBinary::ENDIAN_BIG));

//                 listResult.append(dataHeader);
//             } else if (dataHeadersOptions.nID == STRUCTID_pHYs) {
//                 DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, XPNG::structIDToString(dataHeadersOptions.nID));

//                 dataHeader.nSize = 21;  // pHYs size is always 21 bytes (length=9)
//                 dataHeader.listRecords.append(getDataRecord(0, 4, "Length", XBinary::VT_UINT32, DRF_SIZE, XBinary::ENDIAN_BIG));
//                 dataHeader.listRecords.append(getDataRecord(4, 4, "Type", XBinary::VT_CHAR_ARRAY, DRF_UNKNOWN, XBinary::ENDIAN_BIG));
//                 dataHeader.listRecords.append(getDataRecord(8, 4, "Pixels per Unit X", XBinary::VT_UINT32, DRF_COUNT, XBinary::ENDIAN_BIG));
//                 dataHeader.listRecords.append(getDataRecord(12, 4, "Pixels per Unit Y", XBinary::VT_UINT32, DRF_COUNT, XBinary::ENDIAN_BIG));
//                 dataHeader.listRecords.append(getDataRecord(16, 1, "Unit Specifier", XBinary::VT_UINT8, DRF_UNKNOWN, XBinary::ENDIAN_BIG));
//                 dataHeader.listRecords.append(getDataRecord(17, 4, "CRC", XBinary::VT_UINT32, DRF_UNKNOWN, XBinary::ENDIAN_BIG));

//                 listResult.append(dataHeader);
//             } else if (dataHeadersOptions.nID == STRUCTID_bKGD) {
//                 DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, XPNG::structIDToString(dataHeadersOptions.nID));

//                 // Data part of bKGD can be 1 (indexed), 2 (grayscale), or 6 (truecolor) bytes
//                 quint32 nLen = read_uint32(nStartOffset, true);
//                 dataHeader.nSize = 12 + nLen;  // length + type + data + crc

//                 dataHeader.listRecords.append(getDataRecord(0, 4, "Length", XBinary::VT_UINT32, DRF_SIZE, XBinary::ENDIAN_BIG));
//                 dataHeader.listRecords.append(getDataRecord(4, 4, "Type", XBinary::VT_CHAR_ARRAY, DRF_UNKNOWN, XBinary::ENDIAN_BIG));
//                 if (nLen == 1) {
//                     dataHeader.listRecords.append(getDataRecord(8, 1, "Palette Index", XBinary::VT_UINT8, DRF_UNKNOWN, XBinary::ENDIAN_BIG));
//                     dataHeader.listRecords.append(getDataRecord(9, 4, "CRC", XBinary::VT_UINT32, DRF_UNKNOWN, XBinary::ENDIAN_BIG));
//                 } else if (nLen == 2) {
//                     dataHeader.listRecords.append(getDataRecord(8, 2, "Gray", XBinary::VT_UINT16, DRF_UNKNOWN, XBinary::ENDIAN_BIG));
//                     dataHeader.listRecords.append(getDataRecord(10, 4, "CRC", XBinary::VT_UINT32, DRF_UNKNOWN, XBinary::ENDIAN_BIG));
//                 } else if (nLen == 6) {
//                     dataHeader.listRecords.append(getDataRecord(8, 2, "Red", XBinary::VT_UINT16, DRF_UNKNOWN, XBinary::ENDIAN_BIG));
//                     dataHeader.listRecords.append(getDataRecord(10, 2, "Green", XBinary::VT_UINT16, DRF_UNKNOWN, XBinary::ENDIAN_BIG));
//                     dataHeader.listRecords.append(getDataRecord(12, 2, "Blue", XBinary::VT_UINT16, DRF_UNKNOWN, XBinary::ENDIAN_BIG));
//                     dataHeader.listRecords.append(getDataRecord(14, 4, "CRC", XBinary::VT_UINT32, DRF_UNKNOWN, XBinary::ENDIAN_BIG));
//                 } else {
//                     // Unknown size, just show raw data
//                     dataHeader.listRecords.append(getDataRecord(8, nLen, "Data", XBinary::VT_BYTE_ARRAY, DRF_UNKNOWN, XBinary::ENDIAN_BIG));
//                     dataHeader.listRecords.append(getDataRecord(8 + nLen, 4, "CRC", XBinary::VT_UINT32, DRF_UNKNOWN, XBinary::ENDIAN_BIG));
//                 }

//                 listResult.append(dataHeader);
//             } else {
//                 // mb TODO
//             }
//         }
//     }

//     return listResult;
// }

QList<XBinary::FPART> XPNG::getFileParts(quint32 nFileParts, qint32 nLimit, PDSTRUCT *pPdStruct)
{
    QList<FPART> listResult;

    if ((nLimit < -1) || (nLimit == 0)) {
        return listResult;
    }

    qint64 nTotalSize = getSize();

    if (!isValid(pPdStruct)) {
        return listResult;
    }

    if (nFileParts & FILEPART_SIGNATURE) {
        FPART record = {};

        record.filePart = FILEPART_SIGNATURE;
        record.nFileOffset = 0;
        record.nFileSize = 8;
        record.nVirtualAddress = (XADDR)-1;
        record.sName = tr("Signature");

        listResult.append(record);
        if ((nLimit != -1) && (listResult.count() >= nLimit)) return listResult;
    }
    qint64 nCurrentOffset = 8;  // Start after the PNG signature
    qint32 nChunkCount = 0;

    while ((nCurrentOffset >= 0) && (nCurrentOffset <= nTotalSize - 12) && XBinary::isPdStructNotCanceled(pPdStruct)) {
        if (nChunkCount++ >= PNG_MAX_CHUNK_COUNT) {
            listResult.clear();
            return listResult;
        }

        const CHUNK chunk = _readChunk(nCurrentOffset);

        if (!chunk.bValid) {
            break;
        }

        if (nFileParts & FILEPART_REGION) {
            FPART record = {};

            record.filePart = FILEPART_REGION;
            record.nFileOffset = nCurrentOffset;
            record.nFileSize = 12 + chunk.nDataSize;
            record.nVirtualAddress = (XADDR)-1;
            record.sName = chunk.sName;

            listResult.append(record);
            if ((nLimit != -1) && (listResult.count() >= nLimit)) return listResult;
        }

        nCurrentOffset += 12 + chunk.nDataSize;

        // End Tag
        if (chunk.sName == "IEND") {
            break;
        }
    }

    if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
        listResult.clear();
        return listResult;
    }

    if (nFileParts & FILEPART_OVERLAY) {
        if (nCurrentOffset < nTotalSize) {
            FPART record = {};

            record.filePart = FILEPART_OVERLAY;
            record.nFileOffset = nCurrentOffset;
            record.nFileSize = nTotalSize - nCurrentOffset;
            record.nVirtualAddress = (XADDR)-1;
            record.sName = tr("Overlay");

            listResult.append(record);
            if ((nLimit != -1) && (listResult.count() >= nLimit)) return listResult;
        }
    }

    return listResult;
}

// qint32 XPNG::readTableRow(qint32 nRow, LT locType, XADDR nLocation, const DATA_RECORDS_OPTIONS &dataRecordsOptions, QList<DATA_RECORD_ROW> *pListDataRecords,
//                           void *pUserData, PDSTRUCT *pPdStruct)
// {
//     Q_UNUSED(locType)
//     Q_UNUSED(nLocation)
//     Q_UNUSED(dataRecordsOptions)
//     Q_UNUSED(pUserData)

//     qint32 nResult = 0;

//     if (dataRecordsOptions.dataHeaderFirst.dsID.nID == STRUCTID_CHUNK) {
//         nResult = XBinary::readTableRow(nRow, locType, nLocation, dataRecordsOptions, pListDataRecords, pUserData, pPdStruct);

//         qint64 nStartOffset = locationToOffset(dataRecordsOptions.pMemoryMap, locType, nLocation);

//         quint32 nChunkLenght = read_uint32(nStartOffset, true);

//         nResult = nChunkLenght + 12;
//     } else {
//         nResult = XBinary::readTableRow(nRow, locType, nLocation, dataRecordsOptions, pListDataRecords, pUserData, pPdStruct);
//     }

//     return nResult;
// }

QList<QString> XPNG::getSearchSignatures()
{
    QList<QString> listResult;

    listResult.append("89504E470D0A1A0A");

    return listResult;
}

XBinary *XPNG::createInstance(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    Q_UNUSED(bIsImage)
    Q_UNUSED(nModuleAddress)

    return new XPNG(pDevice);
}

bool XPNG::handleInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XPNG> guardedThis(this);
    bool bResult = true;

    if (!isInternalInfoHandled()) {
        bResult = guardedThis->XBinary::handleInternalInfo(pPdStruct);
        if (!guardedThis || !bResult) return false;

        XBinary::INTERNAL_INFO *pInfo = static_cast<XBinary::INTERNAL_INFO *>(guardedThis->XBinary::getInternalInfo(pPdStruct));
        if (!guardedThis || !pInfo) return false;

        static_cast<XBinary::INTERNAL_INFO &>(guardedThis->m_internalInfo) = *pInfo;
        guardedThis->setIsInternalInfoHandled(true);
    }

    return guardedThis && bResult;
}

void *XPNG::getInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XPNG> guardedThis(this);
    const bool bHandled = guardedThis->handleInternalInfo(pPdStruct);
    if (!guardedThis || !bHandled) return nullptr;

    return &guardedThis->m_internalInfo;
}

void XPNG::setInternalInfo(void *pInternalInfo)
{
    if (pInternalInfo) {
        m_internalInfo = *static_cast<INTERNAL_INFO *>(pInternalInfo);
        XBinary::setInternalInfo(static_cast<XBinary::INTERNAL_INFO *>(&m_internalInfo));
        setIsInternalInfoHandled(true);
    } else {
        m_internalInfo = INTERNAL_INFO();
        XBinary::setInternalInfo(nullptr);
        setIsInternalInfoHandled(false);
    }
}
