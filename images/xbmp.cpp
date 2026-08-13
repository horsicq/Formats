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
#include "xbmp.h"

XBinary::XCONVERT _TABLE_XBMP_STRUCTID[] = {
    {XBMP::STRUCTID_UNKNOWN, "Unknown", QObject::tr("Unknown")},
    {XBMP::STRUCTID_BMPFILEHEADER, "BMPFILEHEADER", QString("BMP File Header")},
    {XBMP::STRUCTID_BMPINFOHEADER, "BMPINFOHEADER", QString("BMP Info Header")},
};

XBMP::XBMP(QIODevice *pDevice) : XBinary(pDevice)
{
}

bool XBMP::isValid(PDSTRUCT *pPdStruct)
{
    const qint64 nTotalSize = getSize();

    if ((nTotalSize < 54) || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    _MEMORY_MAP memoryMap = XBinary::getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);

    if (!compareSignature(&memoryMap, "'BM'", 0, pPdStruct) || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    const BMPFILEHEADER fileHeader = getFileHeader();
    const BMPINFOHEADER infoHeader = getInfoHeader();
    const bool bKnownInfoHeader = (infoHeader.biSize == 40) || (infoHeader.biSize == 108) || (infoHeader.biSize == 124);
    const qint64 nHeaderSize = 14 + (qint64)infoHeader.biSize;

    if (!bKnownInfoHeader || (nHeaderSize > nTotalSize) || (fileHeader.bfSize < (quint64)nHeaderSize) ||
        (fileHeader.bfSize > (quint64)nTotalSize) || (fileHeader.bfOffBits < (quint64)nHeaderSize) ||
        (fileHeader.bfOffBits > fileHeader.bfSize) || (fileHeader.bfReserved1 != 0) || (fileHeader.bfReserved2 != 0) ||
        (infoHeader.biPlanes != 1) || (infoHeader.biWidth <= 0) || (infoHeader.biHeight == 0)) {
        return false;
    }

    const qint64 nDataSize = (qint64)fileHeader.bfSize - fileHeader.bfOffBits;
    const qint64 nHeight = (infoHeader.biHeight < 0) ? -(qint64)infoHeader.biHeight : (qint64)infoHeader.biHeight;
    const bool bRgb = infoHeader.biCompression == 0;
    const bool bRle8 = (infoHeader.biCompression == 1) && (infoHeader.biBitCount == 8);
    const bool bRle4 = (infoHeader.biCompression == 2) && (infoHeader.biBitCount == 4);
    const bool bBitFields = ((infoHeader.biCompression == 3) || (infoHeader.biCompression == 6)) &&
                            ((infoHeader.biBitCount == 16) || (infoHeader.biBitCount == 32));
    const bool bEmbedded = ((infoHeader.biCompression == 4) || (infoHeader.biCompression == 5)) && (infoHeader.biBitCount == 0);
    const bool bKnownBitCount = (infoHeader.biBitCount == 1) || (infoHeader.biBitCount == 4) || (infoHeader.biBitCount == 8) ||
                                (infoHeader.biBitCount == 16) || (infoHeader.biBitCount == 24) || (infoHeader.biBitCount == 32);

    if ((!bRgb || !bKnownBitCount) && !bRle8 && !bRle4 && !bBitFields && !bEmbedded) {
        return false;
    }

    if ((infoHeader.biHeight < 0) && !bRgb && !bBitFields) {
        return false;
    }

    qint64 nRequiredBeforePixels = nHeaderSize;

    if ((infoHeader.biSize == 40) && bBitFields) {
        nRequiredBeforePixels += (infoHeader.biCompression == 6) ? 16 : 12;
    }

    if ((infoHeader.biBitCount > 0) && (infoHeader.biBitCount <= 8)) {
        const quint32 nMaximumColors = 1U << infoHeader.biBitCount;
        const quint32 nColorCount = infoHeader.biClrUsed ? infoHeader.biClrUsed : nMaximumColors;

        if (nColorCount > nMaximumColors) {
            return false;
        }

        nRequiredBeforePixels += (qint64)nColorCount * 4;
    }

    if ((nRequiredBeforePixels > fileHeader.bfOffBits) || (infoHeader.biSizeImage > (quint64)nDataSize)) {
        return false;
    }

    if (bRgb || bBitFields) {
        const qint64 nRowSize = ((((qint64)infoHeader.biWidth * infoHeader.biBitCount) + 31) / 32) * 4;

        if ((nRowSize <= 0) || (nHeight <= 0) || (nRowSize > nDataSize / nHeight)) {
            return false;
        }
    } else if ((bRle8 || bRle4 || bEmbedded) && (infoHeader.biSizeImage == 0)) {
        return false;
    }

    if (bEmbedded) {
        const qint64 nEmbeddedOffset = fileHeader.bfOffBits;
        if ((infoHeader.biCompression == 4) &&
            ((nDataSize < 2) || (read_uint16(nEmbeddedOffset, false) != 0xD8FF))) {
            return false;
        }
        if ((infoHeader.biCompression == 5) &&
            ((nDataSize < 8) || (read_uint32(nEmbeddedOffset, false) != 0x474E5089) ||
             (read_uint32(nEmbeddedOffset + 4, false) != 0x0A1A0A0D))) {
            return false;
        }
    }

    // TODO
    //    BM Windows 3.1x, 95, NT, ... etc.
    //    BA OS/2 struct bitmap array
    //    CI OS/2 struct color icon
    //    CP OS/2 const color pointer
    //    IC OS/2 struct icon
    //    PT OS/2 pointer

    return true;
}

bool XBMP::isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    XBMP xbmp(pDevice);

    return xbmp.isValid(pPdStruct);
}

QString XBMP::getMIMEString()
{
    return "image/bmp";
}

QString XBMP::getArch()
{
    return {};
}

XBinary::ENDIAN XBMP::getEndian()
{
    return XBinary::ENDIAN_LITTLE;
}

XBinary::MODE XBMP::getMode()
{
    return XBinary::MODE_DATA;
}

XBinary::FT XBMP::getFileType()
{
    return FT_BMP;
}

QString XBMP::getFileFormatExt()
{
    return "bmp";
}

QString XBMP::getFileFormatExtsString()
{
    return "BMP (*.bmp)";
}

qint64 XBMP::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    if (!isValid(pPdStruct)) {
        return 0;
    }

    return getFileHeader().bfSize;
}

XBinary::_MEMORY_MAP XBMP::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(mapMode)

    _MEMORY_MAP result = {};
    result.fileType = getFileType();
    result.mode = getMode();
    result.sArch = getArch();
    result.endian = getEndian();
    result.sType = typeIdToString(getType());
    result.nBinarySize = getSize();
    result.nImageSize = getSize();
    result.nModuleAddress = getModuleAddress();
    result.bIsImage = false;

    BMPFILEHEADER fileHeader = getFileHeader();
    if (!isValid(pPdStruct)) {
        return result;
    }

    // Add Header
    _MEMORY_RECORD headerRecord = {};
    headerRecord.nOffset = 0;
    headerRecord.nAddress = (XADDR)-1;
    headerRecord.nSize = fileHeader.bfOffBits;
    headerRecord.filePart = FILEPART_HEADER;
    headerRecord.sName = "Header";
    headerRecord.nIndex = 0;
    result.listRecords.append(headerRecord);

    // Add Bitmap Data (Object)
    _MEMORY_RECORD objectRecord = {};
    objectRecord.nOffset = fileHeader.bfOffBits;
    objectRecord.nAddress = (XADDR)-1;
    objectRecord.nSize = (qint64)fileHeader.bfSize - (qint64)fileHeader.bfOffBits;
    objectRecord.filePart = FILEPART_DATA;
    objectRecord.sName = "Bitmap Data";
    objectRecord.nIndex = 1;
    result.listRecords.append(objectRecord);

    _handleOverlay(&result);

    return result;
}

QString XBMP::getVersion()
{
    QString sResult;

    quint32 nDBISize = read_uint32(0x0E);

    if (nDBISize == 40) {
        sResult = "Windows NT, 3.1x";
    } else if (nDBISize == 108) {
        sResult = "Windows NT 4.0, 95";
    } else if (nDBISize == 124) {
        sResult = "Windows NT 5.0, 98";
    }

    return sResult;
}

QString XBMP::structIDToString(quint32 nID)
{
    return XBinary::XCONVERT_idToTransString(nID, _TABLE_XBMP_STRUCTID, sizeof(_TABLE_XBMP_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QString XBMP::structIDToFtString(quint32 nID)
{
    return XBinary::XCONVERT_idToFtString(nID, _TABLE_XBMP_STRUCTID, sizeof(_TABLE_XBMP_STRUCTID) / sizeof(XBinary::XCONVERT));
}

quint32 XBMP::ftStringToStructID(const QString &sFtString)
{
    return XCONVERT_ftStringToId(sFtString, _TABLE_XBMP_STRUCTID, sizeof(_TABLE_XBMP_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QList<XBinary::XFHEADER> XBMP::getXFHeaders(const XFSTRUCT &xfStruct, PDSTRUCT *pPdStruct)
{
    QList<XBinary::XFHEADER> listResult;

    if (!isValid(pPdStruct)) {
        return listResult;
    }

    quint32 nStructID = xfStruct.nStructID;

    if (nStructID == STRUCTID_UNKNOWN) {
        XFSTRUCT _xfStruct = xfStruct;
        _xfStruct.nStructID = STRUCTID_BMPFILEHEADER;
        _xfStruct.xLoc = offsetToLoc(0);
        listResult.append(getXFHeaders(_xfStruct, pPdStruct));
    } else if (nStructID == STRUCTID_BMPFILEHEADER) {
        XLOC headerLoc = xfStruct.xLoc;
        if (headerLoc.locType == LT_UNKNOWN) {
            headerLoc = offsetToLoc(0);
        }

        XFHEADER xfHeader = {};
        xfHeader.sParentTag = xfStruct.sParent;
        xfHeader.fileType = xfStruct.fileType;
        xfHeader.structID = static_cast<XBinary::STRUCTID>(STRUCTID_BMPFILEHEADER);
        xfHeader.xLoc = headerLoc;
        xfHeader.nSize = 14;
        xfHeader.xfType = XFTYPE_HEADER;
        xfHeader.listFields = getXFRecords(xfStruct.fileType, STRUCTID_BMPFILEHEADER, headerLoc);
        xfHeader.sTag = xfHeaderToTag(xfHeader, structIDToString(STRUCTID_BMPFILEHEADER), xfHeader.sParentTag);
        listResult.append(xfHeader);

        if (xfStruct.bIsParent) {
            XFSTRUCT _xfStruct = xfStruct;
            _xfStruct.sParent = xfHeader.sTag;
            _xfStruct.nStructID = STRUCTID_BMPINFOHEADER;
            _xfStruct.xLoc = offsetToLoc(14);
            listResult.append(getXFHeaders(_xfStruct, pPdStruct));
        }
    } else if (nStructID == STRUCTID_BMPINFOHEADER) {
        XLOC headerLoc = xfStruct.xLoc;
        if (headerLoc.locType == LT_UNKNOWN) {
            headerLoc = offsetToLoc(14);
        }

        qint64 nHeaderOffset = locToOffset(xfStruct.pMemoryMap, headerLoc);

        if ((nHeaderOffset >= 0) && (nHeaderOffset <= getSize() - 4)) {
            const quint32 nInfoHeaderSize = read_uint32(nHeaderOffset);

            if (((nInfoHeaderSize != 40) && (nInfoHeaderSize != 108) && (nInfoHeaderSize != 124)) ||
                (nInfoHeaderSize > (quint64)getSize() - nHeaderOffset)) {
                return listResult;
            }

            XFHEADER xfHeader = {};
            xfHeader.sParentTag = xfStruct.sParent;
            xfHeader.fileType = xfStruct.fileType;
            xfHeader.structID = static_cast<XBinary::STRUCTID>(STRUCTID_BMPINFOHEADER);
            xfHeader.xLoc = headerLoc;
            xfHeader.nSize = nInfoHeaderSize;
            xfHeader.xfType = XFTYPE_HEADER;
            xfHeader.listFields = getXFRecords(xfStruct.fileType, STRUCTID_BMPINFOHEADER, headerLoc);
            xfHeader.sTag = xfHeaderToTag(xfHeader, structIDToString(STRUCTID_BMPINFOHEADER), xfHeader.sParentTag);
            listResult.append(xfHeader);
        }
    }

    return listResult;
}

QList<XBinary::XFRECORD> XBMP::getXFRecords(FT fileType, quint32 nStructID, const XLOC &xLoc)
{
    Q_UNUSED(fileType)
    Q_UNUSED(xLoc)

    QList<XBinary::XFRECORD> listResult;

    // On-disk layout (packed); literal offsets
    if (nStructID == STRUCTID_BMPFILEHEADER) {
        listResult.append({"bfType", 0, 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"bfSize", 2, 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"bfReserved1", 6, 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"bfReserved2", 8, 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"bfOffBits", 10, 4, XFRECORD_FLAG_OFFSET, VT_UINT32});
    } else if (nStructID == STRUCTID_BMPINFOHEADER) {
        listResult.append({"biSize", 0, 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"biWidth", 4, 4, XFRECORD_FLAG_NONE, VT_INT32});
        listResult.append({"biHeight", 8, 4, XFRECORD_FLAG_NONE, VT_INT32});
        listResult.append({"biPlanes", 12, 2, XFRECORD_FLAG_COUNT, VT_UINT16});
        listResult.append({"biBitCount", 14, 2, XFRECORD_FLAG_COUNT, VT_UINT16});
        listResult.append({"biCompression", 16, 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"biSizeImage", 20, 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"biXPelsPerMeter", 24, 4, XFRECORD_FLAG_NONE, VT_INT32});
        listResult.append({"biYPelsPerMeter", 28, 4, XFRECORD_FLAG_NONE, VT_INT32});
        listResult.append({"biClrUsed", 32, 4, XFRECORD_FLAG_COUNT, VT_UINT32});
        listResult.append({"biClrImportant", 36, 4, XFRECORD_FLAG_COUNT, VT_UINT32});
    }

    return listResult;
}

XBMP::BMPINFOHEADER XBMP::getInfoHeader()
{
    BMPINFOHEADER info = {};
    info.biSize = read_uint32(14, false);
    info.biWidth = read_int32(18, false);
    info.biHeight = read_int32(22, false);
    info.biPlanes = read_uint16(26, false);
    info.biBitCount = read_uint16(28, false);
    info.biCompression = read_uint32(30, false);
    info.biSizeImage = read_uint32(34, false);
    info.biXPelsPerMeter = read_int32(38, false);
    info.biYPelsPerMeter = read_int32(42, false);
    info.biClrUsed = read_uint32(46, false);
    info.biClrImportant = read_uint32(50, false);
    return info;
}

XBMP::BMPFILEHEADER XBMP::getFileHeader()
{
    BMPFILEHEADER header = {};
    header.bfType = read_uint16(0, false);
    header.bfSize = read_uint32(2, false);
    header.bfReserved1 = read_uint16(6, false);
    header.bfReserved2 = read_uint16(8, false);
    header.bfOffBits = read_uint32(10, false);
    return header;
}

// QList<XBinary::DATA_HEADER> XBMP::getDataHeaders(const DATA_HEADERS_OPTIONS &dataHeadersOptions, PDSTRUCT *pPdStruct)
// {
//     QList<DATA_HEADER> listResult;

//     if (dataHeadersOptions.nID == STRUCTID_UNKNOWN) {
//         DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
//         _dataHeadersOptions.bChildren = true;
//         _dataHeadersOptions.dsID_parent = _addDefaultHeaders(&listResult, pPdStruct);
//         _dataHeadersOptions.dhMode = XBinary::DHMODE_HEADER;
//         _dataHeadersOptions.fileType = FT_BMP;

//         _dataHeadersOptions.nID = STRUCTID_BMPFILEHEADER;
//         _dataHeadersOptions.nLocation = 0;
//         _dataHeadersOptions.locType = XBinary::LT_OFFSET;
//         listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//     } else {
//         qint64 nStartOffset = locationToOffset(dataHeadersOptions.pMemoryMap, dataHeadersOptions.locType, dataHeadersOptions.nLocation);
//         if (nStartOffset != -1) {
//             if (dataHeadersOptions.nID == STRUCTID_BMPFILEHEADER) {
//                 DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, XBMP::structIDToString(dataHeadersOptions.nID));
//                 dataHeader.nSize = 14;  // BITMAPFILEHEADER
//                 dataHeader.listRecords.append(getDataRecord(0, 2, "bfType", VT_CHAR_ARRAY, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(2, 4, "bfSize", VT_UINT32, DRF_SIZE, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(6, 2, "bfReserved1", VT_UINT16, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(8, 2, "bfReserved2", VT_UINT16, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(10, 4, "bfOffBits", VT_UINT32, DRF_OFFSET, ENDIAN_LITTLE));
//                 listResult.append(dataHeader);

//                 if (dataHeadersOptions.bChildren) {
//                     DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
//                     _dataHeadersOptions.nID = STRUCTID_BMPINFOHEADER;
//                     _dataHeadersOptions.nLocation = dataHeadersOptions.nLocation + 14;
//                     _dataHeadersOptions.locType = dataHeadersOptions.locType;
//                     _dataHeadersOptions.dhMode = XBinary::DHMODE_HEADER;
//                     listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//                 }
//             } else if (dataHeadersOptions.nID == STRUCTID_BMPINFOHEADER) {
//                 // The first field contains the header size and defines the variant
//                 quint32 biSize = read_uint32(nStartOffset, false);
//                 DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, XBMP::structIDToString(dataHeadersOptions.nID));
//                 dataHeader.nSize = biSize;
//                 dataHeader.listRecords.append(getDataRecord(0, 4, "biSize", VT_UINT32, DRF_SIZE, ENDIAN_LITTLE));
//                 if (biSize >= 40) {
//                     dataHeader.listRecords.append(getDataRecord(4, 4, "biWidth", VT_INT32, DRF_UNKNOWN, ENDIAN_LITTLE));
//                     dataHeader.listRecords.append(getDataRecord(8, 4, "biHeight", VT_INT32, DRF_UNKNOWN, ENDIAN_LITTLE));
//                     dataHeader.listRecords.append(getDataRecord(12, 2, "biPlanes", VT_UINT16, DRF_UNKNOWN, ENDIAN_LITTLE));
//                     dataHeader.listRecords.append(getDataRecord(14, 2, "biBitCount", VT_UINT16, DRF_UNKNOWN, ENDIAN_LITTLE));
//                     dataHeader.listRecords.append(getDataRecord(16, 4, "biCompression", VT_UINT32, DRF_UNKNOWN, ENDIAN_LITTLE));
//                     dataHeader.listRecords.append(getDataRecord(20, 4, "biSizeImage", VT_UINT32, DRF_SIZE, ENDIAN_LITTLE));
//                     dataHeader.listRecords.append(getDataRecord(24, 4, "biXPelsPerMeter", VT_INT32, DRF_UNKNOWN, ENDIAN_LITTLE));
//                     dataHeader.listRecords.append(getDataRecord(28, 4, "biYPelsPerMeter", VT_INT32, DRF_UNKNOWN, ENDIAN_LITTLE));
//                     dataHeader.listRecords.append(getDataRecord(32, 4, "biClrUsed", VT_UINT32, DRF_UNKNOWN, ENDIAN_LITTLE));
//                     dataHeader.listRecords.append(getDataRecord(36, 4, "biClrImportant", VT_UINT32, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 }
//                 // For BI_BITFIELDS (if biSize >= 52/56/108/124), masks follow; we keep header generic here
//                 listResult.append(dataHeader);
//             }
//         }
//     }

//     return listResult;
// }

QList<XBinary::FPART> XBMP::getFileParts(quint32 nFileParts, qint32 nLimit, PDSTRUCT *pPdStruct)
{
    QList<FPART> listResult;

    if ((nLimit < -1) || (nLimit == 0)) {
        return listResult;
    }

    BMPFILEHEADER fh = getFileHeader();
    BMPINFOHEADER ih = getInfoHeader();

    qint64 nTotal = getSize();
    qint64 nHeaderSize = 14 + (qint64)ih.biSize;

    if (!isValid(pPdStruct) || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return listResult;
    }

    qint64 nMaxOffset = fh.bfOffBits;

    if (nFileParts & FILEPART_HEADER) {
        FPART rec = {};
        rec.filePart = FILEPART_HEADER;
        rec.nFileOffset = 0;
        rec.nFileSize = fh.bfOffBits;
        rec.nVirtualAddress = (XADDR)-1;
        rec.sName = tr("Header");
        listResult.append(rec);
        if ((nLimit != -1) && (listResult.count() >= nLimit)) return listResult;
    }

    if ((fh.bfOffBits >= (quint64)nHeaderSize) && (fh.bfOffBits <= fh.bfSize) && (fh.bfSize <= static_cast<quint64>(nTotal))) {
        if (nFileParts & FILEPART_DATA) {
            FPART rec = {};
            rec.filePart = FILEPART_DATA;
            rec.nFileOffset = fh.bfOffBits;
            rec.nFileSize = qMin<qint64>((qint64)fh.bfSize - (qint64)fh.bfOffBits, nTotal - (qint64)fh.bfOffBits);
            rec.nVirtualAddress = (XADDR)-1;
            rec.sName = tr("Bitmap Data");
            listResult.append(rec);
            if ((nLimit != -1) && (listResult.count() >= nLimit)) return listResult;
        }

        nMaxOffset = qMax(nMaxOffset, static_cast<qint64>(fh.bfSize));
    }

    if (nFileParts & FILEPART_OVERLAY) {
        if (nMaxOffset < nTotal) {
            FPART rec = {};
            rec.filePart = FILEPART_OVERLAY;
            rec.nFileOffset = nMaxOffset;
            rec.nFileSize = nTotal - nMaxOffset;
            rec.nVirtualAddress = (XADDR)-1;
            rec.sName = tr("Overlay");
            listResult.append(rec);
            if ((nLimit != -1) && (listResult.count() >= nLimit)) return listResult;
        }
    }

    return listResult;
}

QList<QString> XBMP::getSearchSignatures()
{
    return {"'BM'"};
}

XBinary *XBMP::createInstance(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    Q_UNUSED(bIsImage)
    Q_UNUSED(nModuleAddress)

    return new XBMP(pDevice);
}

bool XBMP::handleInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XBMP> guardedThis(this);
    bool bResult = true;

    if (!isInternalInfoHandled()) {
        bResult = guardedThis->XBinary::handleInternalInfo(pPdStruct);
        if (!guardedThis || !bResult) return false;

        XBinary::INTERNAL_INFO *pInfo =
            static_cast<XBinary::INTERNAL_INFO *>(
                guardedThis->XBinary::getInternalInfo(pPdStruct));
        if (!guardedThis || !pInfo) return false;

        static_cast<XBinary::INTERNAL_INFO &>(
            guardedThis->m_internalInfo) = *pInfo;
        guardedThis->setIsInternalInfoHandled(true);
    }

    return guardedThis && bResult;
}

void *XBMP::getInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XBMP> guardedThis(this);
    const bool bHandled = guardedThis->handleInternalInfo(pPdStruct);
    if (!guardedThis || !bHandled) return nullptr;

    return &guardedThis->m_internalInfo;
}

void XBMP::setInternalInfo(void *pInternalInfo)
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
