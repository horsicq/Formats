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
#include "xicon.h"
#include "xpng.h"

namespace {
bool isValidIconDib(XIcon *pIcon, const XIcon::ICONDIRENTRY &entry, quint32 nHeaderSize)
{
    if (!pIcon || (entry.dwBytesInRes < nHeaderSize)) {
        return false;
    }

    const bool bCoreHeader = nHeaderSize == 12;
    const qint64 nSignedWidth = bCoreHeader ? pIcon->read_uint16(entry.dwImageOffset + 4)
                                            : pIcon->read_int32(entry.dwImageOffset + 4);
    const qint64 nSignedCombinedHeight = bCoreHeader ? pIcon->read_uint16(entry.dwImageOffset + 6)
                                                     : pIcon->read_int32(entry.dwImageOffset + 8);
    const quint16 nPlanes = pIcon->read_uint16(entry.dwImageOffset + (bCoreHeader ? 8 : 12));
    const quint16 nBitCount = pIcon->read_uint16(entry.dwImageOffset + (bCoreHeader ? 10 : 14));

    if ((nSignedWidth <= 0) || (nSignedCombinedHeight == 0) || (nPlanes != 1) ||
        ((nBitCount != 1) && (nBitCount != 4) && (nBitCount != 8) && (nBitCount != 16) &&
         (nBitCount != 24) && (nBitCount != 32))) {
        return false;
    }

    const quint64 nCombinedHeight = (nSignedCombinedHeight < 0)
                                        ? (quint64)(-nSignedCombinedHeight)
                                        : (quint64)nSignedCombinedHeight;
    if ((nCombinedHeight < 2) || ((nCombinedHeight & 1) != 0)) {
        return false;
    }

    const quint64 nWidth = (quint64)nSignedWidth;
    const quint64 nHeight = nCombinedHeight / 2;
    const quint64 nEntrySize = entry.dwBytesInRes;
    quint64 nBeforePixels = nHeaderSize;
    quint32 nCompression = 0;
    quint32 nImageSize = 0;
    quint32 nColorCount = 0;

    if (!bCoreHeader) {
        nCompression = pIcon->read_uint32(entry.dwImageOffset + 16);
        nImageSize = pIcon->read_uint32(entry.dwImageOffset + 20);
        nColorCount = pIcon->read_uint32(entry.dwImageOffset + 32);

        const bool bRgb = nCompression == 0;
        const bool bRle = ((nCompression == 1) && (nBitCount == 8)) ||
                          ((nCompression == 2) && (nBitCount == 4));
        const bool bBitFields = ((nCompression == 3) || (nCompression == 6)) &&
                                ((nBitCount == 16) || (nBitCount == 32));
        if (!bRgb && !bRle && !bBitFields) {
            return false;
        }

        if ((nHeaderSize == 40) && bBitFields) {
            nBeforePixels += (nCompression == 6) ? 16 : 12;
        }
    }

    if (nBitCount <= 8) {
        const quint32 nMaximumColors = 1U << nBitCount;
        if (bCoreHeader) {
            nColorCount = nMaximumColors;
        } else if (nColorCount == 0) {
            nColorCount = nMaximumColors;
        } else if (nColorCount > nMaximumColors) {
            return false;
        }
    }

    const quint64 nPaletteEntrySize = bCoreHeader ? 3 : 4;
    const quint64 nPaletteSize = (quint64)nColorCount * nPaletteEntrySize;
    if ((nBeforePixels > nEntrySize) || (nPaletteSize > nEntrySize - nBeforePixels)) {
        return false;
    }
    nBeforePixels += nPaletteSize;

    const quint64 nXorRowSize = (((nWidth * nBitCount) + 31) / 32) * 4;
    const quint64 nAndRowSize = ((nWidth + 31) / 32) * 4;
    quint64 nXorSize = 0;
    if ((nCompression == 1) || (nCompression == 2)) {
        if (nImageSize == 0) {
            return false;
        }
        nXorSize = nImageSize;
    } else {
        const quint64 nRemaining = nEntrySize - nBeforePixels;
        if ((nXorRowSize == 0) || (nXorRowSize > nRemaining / nHeight)) {
            return false;
        }
        nXorSize = nXorRowSize * nHeight;
        if (nImageSize > nXorSize) {
            nXorSize = nImageSize;
        }
    }

    if ((nXorSize > nEntrySize - nBeforePixels) || (nAndRowSize == 0)) {
        return false;
    }
    const quint64 nAfterXor = nBeforePixels + nXorSize;
    return nAndRowSize <= (nEntrySize - nAfterXor) / nHeight;
}
}  // namespace

static XBinary::XCONVERT _TABLE_XICON_STRUCTID[] = {
    {XIcon::STRUCTID_UNKNOWN, "Unknown", QObject::tr("Unknown")},
    {XIcon::STRUCTID_ICONDIR, "ICONDIR", QString("ICONDIR")},
    {XIcon::STRUCTID_ICONDIRENTRY, "ICONDIRENTRY", QString("ICONDIRENTRY")},
};

XIcon::XIcon(QIODevice *pDevice) : XBinary(pDevice)
{
}

XIcon::~XIcon()
{
}

bool XIcon::isValid(PDSTRUCT *pPdStruct)
{
    const qint64 nTotalSize = getSize();

    if ((nTotalSize < (qint64)(sizeof(ICONDIR) + sizeof(ICONDIRENTRY))) || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    const ICONDIR iconDir = readICONDIR();

    if ((iconDir.idReserved != 0) || ((iconDir.idType != 1) && (iconDir.idType != 2)) || (iconDir.idCount == 0)) {
        return false;
    }

    const qint64 nTableSize = (qint64)iconDir.idCount * (qint64)sizeof(ICONDIRENTRY);
    const qint64 nDataOffset = (qint64)sizeof(ICONDIR) + nTableSize;

    if (nDataOffset > nTotalSize) {
        return false;
    }

    qint64 nOffset = sizeof(ICONDIR);

    for (quint32 i = 0; (i < iconDir.idCount) && XBinary::isPdStructNotCanceled(pPdStruct); i++, nOffset += sizeof(ICONDIRENTRY)) {
        const ICONDIRENTRY entry = readICONDIRENTRY(nOffset);

        if ((entry.bReserved != 0) || (entry.dwBytesInRes < 4) || (entry.dwImageOffset < (quint64)nDataOffset) ||
            (entry.dwImageOffset > (quint64)nTotalSize) || (entry.dwBytesInRes > (quint64)nTotalSize - entry.dwImageOffset)) {
            return false;
        }

        if ((iconDir.idType == 1) && (entry.wPlanes != 0) && (entry.wPlanes != 1)) {
            return false;
        }

        const quint32 nHeader = read_uint32(entry.dwImageOffset);
        const bool bDibHeader = (nHeader == 12) || (nHeader == 40) || (nHeader == 108) || (nHeader == 124);
        const bool bPngHeader = (entry.dwBytesInRes >= 8) &&
                                (read_array(entry.dwImageOffset, 8) == QByteArray::fromHex("89504e470d0a1a0a"));

        if (bPngHeader) {
            SubDevice subDevice(getDevice(), entry.dwImageOffset, entry.dwBytesInRes);
            if (!subDevice.open(QIODevice::ReadOnly)) {
                return false;
            }
            XPNG png(&subDevice);
            const qint64 nPngSize = png.getFileFormatSize(pPdStruct);
            subDevice.close();
            if (nPngSize != entry.dwBytesInRes) {
                return false;
            }
        } else if (!bDibHeader || !isValidIconDib(this, entry, nHeader)) {
            return false;
        }
    }

    return XBinary::isPdStructNotCanceled(pPdStruct);
}

bool XIcon::isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    XIcon xicon(pDevice);

    return xicon.isValid(pPdStruct);
}

XBinary::FT XIcon::getFileType()
{
    FT result = FT_ICO;

    quint32 nHeader = read_uint32(0);

    if (nHeader == 0x00010000) {
        result = FT_ICO;
    } else if (nHeader == 0x00020000) {
        result = FT_CUR;
    }

    return result;
}

QString XIcon::getFileFormatExt()
{
    QString sResult;

    if (getFileType() == FT_CUR) {
        sResult = QString("cur");
    } else {
        sResult = QString("ico");
    }

    return sResult;
}

QString XIcon::getFileFormatExtsString()
{
    return QString("Icons, cursors (*.ico *.cur)");
}

qint64 XIcon::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    return _calculateRawSize(pPdStruct);
}

QString XIcon::getMIMEString()
{
    return QString("image/x-icon");
}

XBinary::_MEMORY_MAP XIcon::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(mapMode)

    _MEMORY_MAP result = {};

    result.nBinarySize = getSize();

    if (!isValid(pPdStruct)) {
        return result;
    }

    qint32 nIndex = 0;

    {
        _MEMORY_RECORD record = {};

        record.nIndex = nIndex++;
        record.filePart = FILEPART_HEADER;
        record.nOffset = 0;
        record.nSize = sizeof(ICONDIR);
        record.nAddress = -1;
        record.sName = tr("Header");

        result.listRecords.append(record);
    }

    ICONDIR iconDir = readICONDIR();

    qint64 nDataOffset = sizeof(ICONDIR) + (iconDir.idCount * sizeof(ICONDIRENTRY));

    bool bError = false;

    qint32 nNumberOfRecords = iconDir.idCount;

    qint64 nOffset = sizeof(ICONDIR);

    for (qint32 i = 0; (i < nNumberOfRecords) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        ICONDIRENTRY iconDirectory = readICONDIRENTRY(nOffset);

        if ((iconDirectory.dwBytesInRes < 4) || (iconDirectory.dwImageOffset < (quint64)nDataOffset) ||
            (iconDirectory.dwImageOffset > (quint64)result.nBinarySize) ||
            (iconDirectory.dwBytesInRes > (quint64)result.nBinarySize - iconDirectory.dwImageOffset) || (iconDirectory.bReserved != 0)) {
            bError = true;
            break;
        }

        if (iconDir.idType == 1) {
            if ((iconDirectory.wPlanes != 0) && (iconDirectory.wPlanes != 1)) {
                bError = true;
                break;
            }
        }

        quint32 nHeader = read_uint32(iconDirectory.dwImageOffset);

        if ((nHeader != 12) && (nHeader != 40) && (nHeader != 108) && (nHeader != 124) && (nHeader != 0x474e5089)) {
            bError = true;
            break;
        }

        _MEMORY_RECORD record = {};

        record.nIndex = nIndex++;
        record.filePart = FILEPART_DATA;
        record.nOffset = iconDirectory.dwImageOffset;
        record.nSize = iconDirectory.dwBytesInRes;
        record.nAddress = -1;

        result.listRecords.append(record);

        nOffset += sizeof(ICONDIRENTRY);
    }

    if (bError) {
        result.listRecords.clear();
    }

    if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
        result.listRecords.clear();
        return result;
    }

    _handleOverlay(&result);

    return result;
}

XIcon::ICONDIR XIcon::readICONDIR()
{
    ICONDIR result = {};

    result.idReserved = read_uint16(offsetof(ICONDIR, idReserved));
    result.idType = read_uint16(offsetof(ICONDIR, idType));
    result.idCount = read_uint16(offsetof(ICONDIR, idCount));

    return result;
}

XIcon::ICONDIRENTRY XIcon::readICONDIRENTRY(qint64 nOffset)
{
    ICONDIRENTRY result = {};

    result.bWidth = read_uint8(nOffset + offsetof(ICONDIRENTRY, bWidth));
    result.bHeight = read_uint8(nOffset + offsetof(ICONDIRENTRY, bHeight));
    result.bColorCount = read_uint8(nOffset + offsetof(ICONDIRENTRY, bColorCount));
    result.bReserved = read_uint8(nOffset + offsetof(ICONDIRENTRY, bReserved));
    result.wPlanes = read_uint16(nOffset + offsetof(ICONDIRENTRY, wPlanes));
    result.wBitCount = read_uint16(nOffset + offsetof(ICONDIRENTRY, wBitCount));
    result.dwBytesInRes = read_uint32(nOffset + offsetof(ICONDIRENTRY, dwBytesInRes));
    result.dwImageOffset = read_uint32(nOffset + offsetof(ICONDIRENTRY, dwImageOffset));

    return result;
}

XIcon::GRPICONDIRENTRY XIcon::readGPRICONDIRENTRY(qint64 nOffset)
{
    GRPICONDIRENTRY result = {};

    result.bWidth = read_uint8(nOffset + offsetof(GRPICONDIRENTRY, bWidth));
    result.bHeight = read_uint8(nOffset + offsetof(GRPICONDIRENTRY, bHeight));
    result.bColorCount = read_uint8(nOffset + offsetof(GRPICONDIRENTRY, bColorCount));
    result.bReserved = read_uint8(nOffset + offsetof(GRPICONDIRENTRY, bReserved));
    result.wPlanes = read_uint16(nOffset + offsetof(GRPICONDIRENTRY, wPlanes));
    result.wBitCount = read_uint16(nOffset + offsetof(GRPICONDIRENTRY, wBitCount));
    result.dwBytesInRes = read_uint32(nOffset + offsetof(GRPICONDIRENTRY, dwBytesInRes));
    result.nID = read_uint16(nOffset + offsetof(GRPICONDIRENTRY, nID));

    return result;
}

QList<XIcon::ICONDIRENTRY> XIcon::getIconDirectories(PDSTRUCT *pPdStruct)
{
    QList<XIcon::ICONDIRENTRY> listResult;

    if (!isValid(pPdStruct)) {
        return listResult;
    }

    ICONDIR iconDir = readICONDIR();

    qint32 nNumberOfRecords = iconDir.idCount;
    const qint64 nTotalSize = getSize();
    const qint64 nTableEnd = (qint64)sizeof(ICONDIR) + (qint64)nNumberOfRecords * (qint64)sizeof(ICONDIRENTRY);

    if ((iconDir.idReserved != 0) || ((iconDir.idType != 1) && (iconDir.idType != 2)) || (nTableEnd > nTotalSize)) {
        return listResult;
    }

    qint64 nOffset = sizeof(ICONDIR);

    for (qint32 i = 0; (i < nNumberOfRecords) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        ICONDIRENTRY record = readICONDIRENTRY(nOffset);

        if ((record.bReserved != 0) || (record.dwBytesInRes == 0) || (record.dwImageOffset < (quint64)nTableEnd) ||
            (record.dwImageOffset > (quint64)nTotalSize) || (record.dwBytesInRes > (quint64)nTotalSize - record.dwImageOffset)) {
            listResult.clear();
            break;
        }

        listResult.append(record);

        nOffset += sizeof(ICONDIRENTRY);
    }

    if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
        listResult.clear();
    }

    return listResult;
}

QList<XIcon::GRPICONDIRENTRY> XIcon::getIconGPRDirectories(PDSTRUCT *pPdStruct)
{
    QList<XIcon::GRPICONDIRENTRY> listResult;

    ICONDIR iconDir = readICONDIR();

    qint32 nNumberOfRecords = iconDir.idCount;
    const qint64 nTableEnd = (qint64)sizeof(ICONDIR) + (qint64)nNumberOfRecords * (qint64)sizeof(GRPICONDIRENTRY);

    if ((getSize() < (qint64)sizeof(ICONDIR)) || (iconDir.idReserved != 0) || (iconDir.idType != 1) ||
        (iconDir.idCount == 0) || (nTableEnd > getSize()) || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return listResult;
    }

    qint64 nOffset = sizeof(ICONDIR);

    for (qint32 i = 0; (i < nNumberOfRecords) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        GRPICONDIRENTRY record = readGPRICONDIRENTRY(nOffset);

        if ((record.bReserved != 0) || ((record.wPlanes != 0) && (record.wPlanes != 1)) ||
            (record.dwBytesInRes == 0) || (record.nID == 0)) {
            listResult.clear();
            break;
        }

        listResult.append(record);

        nOffset += sizeof(GRPICONDIRENTRY);
    }

    if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
        listResult.clear();
    }

    return listResult;
}

QString XIcon::structIDToString(quint32 nID)
{
    return XBinary::XCONVERT_idToTransString(nID, _TABLE_XICON_STRUCTID, sizeof(_TABLE_XICON_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QString XIcon::structIDToFtString(quint32 nID)
{
    return XBinary::XCONVERT_idToFtString(nID, _TABLE_XICON_STRUCTID, sizeof(_TABLE_XICON_STRUCTID) / sizeof(XBinary::XCONVERT));
}

quint32 XIcon::ftStringToStructID(const QString &sFtString)
{
    return XCONVERT_ftStringToId(sFtString, _TABLE_XICON_STRUCTID, sizeof(_TABLE_XICON_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QList<XBinary::XFHEADER> XIcon::getXFHeaders(const XFSTRUCT &xfStruct, PDSTRUCT *pPdStruct)
{
    QList<XBinary::XFHEADER> listResult;

    if (!isValid(pPdStruct)) {
        return listResult;
    }

    quint32 nStructID = xfStruct.nStructID;

    if (nStructID == STRUCTID_UNKNOWN) {
        XFSTRUCT _xfStruct = xfStruct;
        _xfStruct.nStructID = STRUCTID_ICONDIR;
        _xfStruct.xLoc = offsetToLoc(0);
        listResult.append(getXFHeaders(_xfStruct, pPdStruct));
    } else if (nStructID == STRUCTID_ICONDIR) {
        XLOC headerLoc = xfStruct.xLoc;
        if (headerLoc.locType == LT_UNKNOWN) {
            headerLoc = offsetToLoc(0);
        }

        XFHEADER xfHeader = {};
        xfHeader.sParentTag = xfStruct.sParent;
        xfHeader.fileType = xfStruct.fileType;
        xfHeader.structID = static_cast<XBinary::STRUCTID>(STRUCTID_ICONDIR);
        xfHeader.xLoc = headerLoc;
        xfHeader.nSize = sizeof(ICONDIR);
        xfHeader.xfType = XFTYPE_HEADER;
        xfHeader.listFields = getXFRecords(xfStruct.fileType, STRUCTID_ICONDIR, headerLoc);
        xfHeader.sTag = xfHeaderToTag(xfHeader, structIDToString(STRUCTID_ICONDIR), xfHeader.sParentTag);
        listResult.append(xfHeader);

        if (xfStruct.bIsParent) {
            XFSTRUCT _xfStruct = xfStruct;
            _xfStruct.sParent = xfHeader.sTag;
            _xfStruct.nStructID = STRUCTID_ICONDIRENTRY;
            _xfStruct.xLoc = offsetToLoc(sizeof(ICONDIR));
            _xfStruct.nCount = readICONDIR().idCount;
            listResult.append(getXFHeaders(_xfStruct, pPdStruct));
        }
    } else if (nStructID == STRUCTID_ICONDIRENTRY) {
        qint64 nOffset = locToOffset(xfStruct.pMemoryMap, xfStruct.xLoc);
        qint32 nCount = xfStruct.nCount;
        qint64 nFileSize = getSize();

        if (nOffset == -1) {
            nOffset = sizeof(ICONDIR);
        }
        if (nCount == 0) {
            nCount = readICONDIR().idCount;
        }

        if ((nCount > 0) && (nOffset >= 0) && (nOffset <= nFileSize - (qint64)sizeof(ICONDIRENTRY))) {
            XFHEADER xfHeader = {};
            xfHeader.sParentTag = xfStruct.sParent;
            xfHeader.fileType = xfStruct.fileType;
            xfHeader.structID = static_cast<XBinary::STRUCTID>(STRUCTID_ICONDIRENTRY);
            xfHeader.xLoc = offsetToLoc(nOffset);
            xfHeader.xfType = XFTYPE_TABLE;
            xfHeader.listFields = getXFRecords(xfStruct.fileType, STRUCTID_ICONDIRENTRY, xfHeader.xLoc);

            qint64 nCurrentOffset = nOffset;
            for (qint32 i = 0; (i < nCount) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
                if ((nCurrentOffset < 0) || (nCurrentOffset > nFileSize - (qint64)sizeof(ICONDIRENTRY))) {
                    break;
                }
                xfHeader.listRowLocations.append(nCurrentOffset);
                nCurrentOffset += sizeof(ICONDIRENTRY);
            }

            xfHeader.sTag = xfHeaderToTag(xfHeader, structIDToString(STRUCTID_ICONDIRENTRY), xfHeader.sParentTag);
            listResult.append(xfHeader);
        }
    }

    if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
        listResult.clear();
    }

    return listResult;
}

QList<XBinary::XFRECORD> XIcon::getXFRecords(FT fileType, quint32 nStructID, const XLOC &xLoc)
{
    Q_UNUSED(fileType)
    Q_UNUSED(xLoc)

    QList<XBinary::XFRECORD> listResult;

    if (nStructID == STRUCTID_ICONDIR) {
        listResult.append({"idReserved", (qint32)offsetof(ICONDIR, idReserved), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"idType", (qint32)offsetof(ICONDIR, idType), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"idCount", (qint32)offsetof(ICONDIR, idCount), 2, XFRECORD_FLAG_COUNT, VT_UINT16});
    } else if (nStructID == STRUCTID_ICONDIRENTRY) {
        listResult.append({"bWidth", (qint32)offsetof(ICONDIRENTRY, bWidth), 1, XFRECORD_FLAG_NONE, VT_UINT8});
        listResult.append({"bHeight", (qint32)offsetof(ICONDIRENTRY, bHeight), 1, XFRECORD_FLAG_NONE, VT_UINT8});
        listResult.append({"bColorCount", (qint32)offsetof(ICONDIRENTRY, bColorCount), 1, XFRECORD_FLAG_COUNT, VT_UINT8});
        listResult.append({"bReserved", (qint32)offsetof(ICONDIRENTRY, bReserved), 1, XFRECORD_FLAG_NONE, VT_UINT8});
        listResult.append({"wPlanes", (qint32)offsetof(ICONDIRENTRY, wPlanes), 2, XFRECORD_FLAG_COUNT, VT_UINT16});
        listResult.append({"wBitCount", (qint32)offsetof(ICONDIRENTRY, wBitCount), 2, XFRECORD_FLAG_COUNT, VT_UINT16});
        listResult.append({"dwBytesInRes", (qint32)offsetof(ICONDIRENTRY, dwBytesInRes), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"dwImageOffset", (qint32)offsetof(ICONDIRENTRY, dwImageOffset), 4, XFRECORD_FLAG_OFFSET, VT_UINT32});
    }

    return listResult;
}

// QList<XBinary::DATA_HEADER> XIcon::getDataHeaders(const DATA_HEADERS_OPTIONS &dataHeadersOptions, PDSTRUCT *pPdStruct)
// {
//     QList<DATA_HEADER> listResult;

//     if (dataHeadersOptions.nID == STRUCTID_UNKNOWN) {
//         DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
//         _dataHeadersOptions.bChildren = true;
//         _dataHeadersOptions.dsID_parent = _addDefaultHeaders(&listResult, pPdStruct);
//         _dataHeadersOptions.dhMode = XBinary::DHMODE_HEADER;
//         _dataHeadersOptions.fileType = getFileType();

//         _dataHeadersOptions.nID = STRUCTID_ICONDIR;
//         _dataHeadersOptions.nLocation = 0;
//         _dataHeadersOptions.locType = XBinary::LT_OFFSET;
//         listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//     } else {
//         qint64 nStartOffset = locationToOffset(dataHeadersOptions.pMemoryMap, dataHeadersOptions.locType, dataHeadersOptions.nLocation);
//         if (nStartOffset != -1) {
//             if (dataHeadersOptions.nID == STRUCTID_ICONDIR) {
//                 DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, XIcon::structIDToString(dataHeadersOptions.nID));
//                 dataHeader.nSize = sizeof(ICONDIR);
//                 dataHeader.listRecords.append(getDataRecord(0, 2, "Reserved", VT_UINT16, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(2, 2, "Type", VT_UINT16, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(4, 2, "Count", VT_UINT16, DRF_COUNT, ENDIAN_LITTLE));
//                 listResult.append(dataHeader);

//                 if (dataHeadersOptions.bChildren) {
//                     ICONDIR dir = readICONDIR();
//                     DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
//                     _dataHeadersOptions.dhMode = XBinary::DHMODE_TABLE;
//                     _dataHeadersOptions.nID = STRUCTID_ICONDIRENTRY;
//                     _dataHeadersOptions.nLocation = dataHeadersOptions.nLocation + sizeof(ICONDIR);
//                     _dataHeadersOptions.locType = dataHeadersOptions.locType;
//                     _dataHeadersOptions.nCount = dir.idCount;
//                     _dataHeadersOptions.nSize = sizeof(ICONDIRENTRY) * dir.idCount;
//                     listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//                 }
//             } else if (dataHeadersOptions.nID == STRUCTID_ICONDIRENTRY) {
//                 // Describe the table of entries; row reading can be added later if needed
//                 DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, XIcon::structIDToString(dataHeadersOptions.nID));
//                 listResult.append(dataHeader);
//             }
//         }
//     }

//     return listResult;
// }

QList<XBinary::FPART> XIcon::getFileParts(quint32 nFileParts, qint32 nLimit, PDSTRUCT *pPdStruct)
{
    QList<FPART> listResult;

    if ((nLimit < -1) || (nLimit == 0)) {
        return listResult;
    }

    qint64 nTotal = getSize();
    qint64 nMax = sizeof(ICONDIR);

    if (!isValid(pPdStruct)) {
        return listResult;
    }

    if (nFileParts & FILEPART_HEADER) {
        FPART rec = {};
        rec.filePart = FILEPART_HEADER;
        rec.nFileOffset = 0;
        rec.nFileSize = sizeof(ICONDIR);
        rec.nVirtualAddress = -1;
        rec.sName = tr("Header");
        listResult.append(rec);
        if ((nLimit != -1) && (listResult.count() >= nLimit)) return listResult;
    }

    ICONDIR dir = readICONDIR();

    if (nFileParts & FILEPART_TABLE) {
        FPART rec = {};
        rec.filePart = FILEPART_TABLE;
        rec.nFileOffset = sizeof(ICONDIR);
        rec.nFileSize = sizeof(ICONDIRENTRY) * dir.idCount;
        rec.nVirtualAddress = -1;
        rec.sName = tr("Entries");
        listResult.append(rec);
        if ((nLimit != -1) && (listResult.count() >= nLimit)) return listResult;
    }

    nMax = qMax(nMax, (qint64)(sizeof(ICONDIR) + sizeof(ICONDIRENTRY) * dir.idCount));

    QList<ICONDIRENTRY> entries = getIconDirectories(pPdStruct);
    for (qint32 nI = 0; nI < entries.size(); ++nI) {
        ICONDIRENTRY e = entries.at(nI);
        if ((e.dwImageOffset < nTotal) && (e.dwBytesInRes > 0)) {
            if (nFileParts & FILEPART_OBJECT) {
                FPART rec = {};
                rec.filePart = FILEPART_OBJECT;
                rec.nFileOffset = e.dwImageOffset;
                rec.nFileSize = qMin<qint64>(e.dwBytesInRes, nTotal - e.dwImageOffset);
                rec.nVirtualAddress = -1;
                rec.sName = tr("Icon");
                listResult.append(rec);
                if ((nLimit != -1) && (listResult.count() >= nLimit)) return listResult;
            }

            nMax = qMax(nMax, (qint64)e.dwImageOffset + (qint64)e.dwBytesInRes);
        }
    }

    if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
        listResult.clear();
        return listResult;
    }

    if (nFileParts & FILEPART_OVERLAY) {
        if (nMax < nTotal) {
            FPART rec = {};
            rec.filePart = FILEPART_OVERLAY;
            rec.nFileOffset = nMax;
            rec.nFileSize = nTotal - nMax;
            rec.nVirtualAddress = -1;
            rec.sName = tr("Overlay");
            listResult.append(rec);
            if ((nLimit != -1) && (listResult.count() >= nLimit)) return listResult;
        }
    }

    return listResult;
}

QList<QString> XIcon::getSearchSignatures()
{
    QList<QString> listResult;

    return listResult;
}

XBinary *XIcon::createInstance(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    Q_UNUSED(bIsImage)
    Q_UNUSED(nModuleAddress)

    return new XIcon(pDevice);
}

bool XIcon::handleInternalInfo(PDSTRUCT *pPdStruct)
{
    bool bResult = true;

    if (!isInternalInfoHandled()) {
        bResult = XBinary::handleInternalInfo(pPdStruct);

        if (bResult) {
            static_cast<XBinary::INTERNAL_INFO &>(m_internalInfo) =
                *static_cast<XBinary::INTERNAL_INFO *>(XBinary::getInternalInfo(pPdStruct));
            setIsInternalInfoHandled(true);
        }
    }

    return bResult;
}

void *XIcon::getInternalInfo(PDSTRUCT *pPdStruct)
{
    handleInternalInfo(pPdStruct);

    return &m_internalInfo;
}

void XIcon::setInternalInfo(void *pInternalInfo)
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
