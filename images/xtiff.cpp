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
#include "xtiff.h"

#include <QSet>

namespace {
const qint32 XTIFF_MAX_IFD_TABLES = 4096;
const quint64 XTIFF_MAX_IFD_ENTRIES = 256 * 1024;

void appendUnsignedTagMetadata(XTiff *pTiff, QVector<XBinary::XMETADATA_STRUCT> *pListResult, QList<XTiff::CHUNK> *pListChunks, bool bIsBigEndian,
                               quint16 nTag, XBinary::XMETADATA_ID id, const QString &sName, bool bForceUInt16)
{
    const QList<XTiff::CHUNK> listValues = XTiff::_getChunksByTag(pListChunks, nTag);
    for (qint32 i = 0; i < listValues.count(); ++i) {
        const XTiff::CHUNK &chunk = listValues.at(i);
        if (chunk.nSize < 2) {
            continue;
        }

        const quint32 nValue = (!bForceUInt16 && (chunk.nSize == 4)) ? pTiff->read_uint32(chunk.nOffset, bIsBigEndian)
                                                                    : pTiff->read_uint16(chunk.nOffset, bIsBigEndian);
        XBinary::XMETADATA_STRUCT record = {};
        record.nOffset = chunk.nOffset;
        record.nSize = bForceUInt16 || (chunk.nSize != 4) ? 2 : 4;
        record.nAddress = pTiff->offsetToAddress(chunk.nOffset);
        record.id = id;
        record.sName = (listValues.count() > 1) ? QString("Image %1: %2").arg(i + 1).arg(sName) : sName;
        record.varValue = nValue;
        pListResult->append(record);
    }
}

void appendDateTagMetadata(XTiff *pTiff, QVector<XBinary::XMETADATA_STRUCT> *pListResult, QList<XTiff::CHUNK> *pListChunks, quint16 nTag,
                           XBinary::XMETADATA_ID id, const QString &sName)
{
    const QList<XTiff::CHUNK> listDates = XTiff::_getChunksByTag(pListChunks, nTag);
    for (qint32 i = 0; i < listDates.count(); ++i) {
        const XTiff::CHUNK &chunk = listDates.at(i);
        QString sValue = pTiff->read_ansiString(chunk.nOffset, chunk.nSize).trimmed();
        QDateTime dateTime = QDateTime::fromString(sValue, QString("yyyy:MM:dd HH:mm:ss"));
        if (!dateTime.isValid()) {
            dateTime = QDateTime::fromString(sValue, Qt::ISODate);
        }
        if (!dateTime.isValid()) {
            continue;
        }

        XBinary::XMETADATA_STRUCT record = {};
        record.nOffset = chunk.nOffset;
        record.nSize = chunk.nSize;
        record.nAddress = pTiff->offsetToAddress(chunk.nOffset);
        record.id = id;
        record.sName = sName;
        record.varValue = dateTime;
        pListResult->append(record);
    }
}
}  // namespace

static XBinary::XCONVERT _TABLE_XTIFF_STRUCTID[] = {
    {XTiff::STRUCTID_UNKNOWN, "Unknown", QObject::tr("Unknown")},
    {XTiff::STRUCTID_SIGNATURE, "Signature", QString("Signature")},
    {XTiff::STRUCTID_IFD_TABLE, "IFD_TABLE", QString("IFD Table")},
    {XTiff::STRUCTID_IFD_ENTRY, "IFD_ENTRY", QString("IFD Entry")},
};

XTiff::XTiff(QIODevice *pDevice) : XBinary(pDevice)
{
}

XTiff::~XTiff()
{
}

bool XTiff::isValid(PDSTRUCT *pPdStruct)
{
    if (!isPdStructNotCanceled(pPdStruct) || (getSize() < 14) || (getEndian() == ENDIAN_UNKNOWN)) return false;
    if (!isPdStructNotCanceled(pPdStruct)) return false;

    QList<IFD_INFO> listInfo;
    if (!getIFDChain(&listInfo, pPdStruct) || !isPdStructNotCanceled(pPdStruct) || listInfo.isEmpty()) return false;

    return true;
}

bool XTiff::isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    if (!pDevice || !isPdStructNotCanceled(pPdStruct)) return false;

    XTiff xtiff(pDevice);

    return xtiff.isValid(pPdStruct);
}

XBinary::_MEMORY_MAP XTiff::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(mapMode)

    XBinary::PDSTRUCT pdStructEmpty = {};

    if (!pPdStruct) {
        pdStructEmpty = XBinary::createPdStruct();
        pPdStruct = &pdStructEmpty;
    }

    _MEMORY_MAP result = {};

    qint64 nTotalSize = qMax<qint64>(0, getSize());

    result.nBinarySize = nTotalSize;
    result.endian = ENDIAN_UNKNOWN;
    if (!isPdStructNotCanceled(pPdStruct)) return result;
    result.endian = getEndian();

    if (!isPdStructNotCanceled(pPdStruct) || (nTotalSize < 8) || (result.endian == ENDIAN_UNKNOWN)) return result;

    QList<IFD_INFO> listInfo;
    if (!getIFDChain(&listInfo, pPdStruct) || !isPdStructNotCanceled(pPdStruct)) return result;

    qint32 nIndex = 0;
    _MEMORY_RECORD header = {};
    header.nIndex = nIndex++;
    header.filePart = FILEPART_HEADER;
    header.nOffset = 0;
    header.nSize = 8;
    header.nAddress = (XADDR)-1;
    header.sName = tr("Header");
    result.listRecords.append(header);

    const bool bIsBigEndian = (result.endian == ENDIAN_BIG);
    for (const IFD_INFO &info : listInfo) {
        if (!isPdStructNotCanceled(pPdStruct)) {
            result.listRecords.clear();
            return result;
        }

        _MEMORY_RECORD table = {};
        table.nIndex = nIndex++;
        table.filePart = FILEPART_TABLE;
        table.nOffset = info.nOffset;
        table.nSize = info.nSize;
        table.nAddress = (XADDR)-1;
        table.sName = tr("Table");
        result.listRecords.append(table);

        qint64 nEntryOffset = info.nOffset + (qint64)sizeof(quint16);
        for (quint32 i = 0; i < info.nCount; i++, nEntryOffset += (qint64)sizeof(IFD_ENTRY)) {
            CHUNK chunk = {};
            quint16 nType = 0;
            if (getIFDChunk(nEntryOffset, bIsBigEndian, nTotalSize, &chunk, &nType, pPdStruct) && (chunk.nSize > 4)) {
                _MEMORY_RECORD region = {};
                region.nIndex = nIndex++;
                region.filePart = FILEPART_REGION;
                region.nOffset = chunk.nOffset;
                region.nSize = chunk.nSize;
                region.nAddress = (XADDR)-1;
                region.sName = QString("%1-%2").arg(XBinary::valueToHex(chunk.nTag)).arg(XBinary::valueToHex(nType));
                result.listRecords.append(region);
            }
            if (!isPdStructNotCanceled(pPdStruct)) {
                result.listRecords.clear();
                return result;
            }
        }
    }

    _handleOverlay(&result);

    return result;
}

XBinary::FT XTiff::getFileType()
{
    return FT_TIFF;
}

QString XTiff::getFileFormatExt()
{
    return "tiff";
}

QString XTiff::getFileFormatExtsString()
{
    return "TIFF (*.tiff *.tif)";
}

qint64 XTiff::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    return _calculateRawSize(pPdStruct);
}

XBinary::ENDIAN XTiff::getEndian()
{
    ENDIAN result = ENDIAN_UNKNOWN;

    if (getSize() < 4) return result;

    char signature[4] = {};
    if (read_array(0, signature, (qint64)sizeof(signature)) != (qint64)sizeof(signature)) return result;

    if ((signature[0] == 'I') && (signature[1] == 'I') && ((quint8)signature[2] == 0x2A) && (signature[3] == 0)) {
        result = ENDIAN_LITTLE;
    } else if ((signature[0] == 'M') && (signature[1] == 'M') && (signature[2] == 0) && ((quint8)signature[3] == 0x2A)) {
        result = ENDIAN_BIG;
    }

    return result;
}

QList<XTiff::CHUNK> XTiff::getChunks(PDSTRUCT *pPdStruct)
{
    // Image
    // 0fe NewSubfileType LONG 1
    // 100 ImageWidth SHORT or LONG
    // 101 ImageLength SHORT or LONG
    // 102 BitsPerSample  SHORT 4 or 8
    // 103 Compression  SHORT 1, 2 or 32773
    // 106 PhotometricInterpretation  SHORT 0 or 1
    // 111 StripOffsets SHORT or LONG
    // 115 SamplesPerPixel SHORT
    // 116 RowsPerStrip  SHORT or LONG
    // 117 StripByteCounts LONG or SHORT
    // 11a XResolution RATIONAL
    // 11b YResolution RATIONAL
    // 11c PlanarConfiguration SHORT
    // 128 ResolutionUnit  SHORT 1, 2 or 3
    // 13d Predictor SHORT
    // 152 ExtraSamples SHORT 1

    // Exif
    // 10f Make ASCII
    // 110 Model ASCII
    // 112 Orientation SHORT 1
    // 131 Software ASCII
    // 132 DateTime ASCII
    // 213 YCbCrPositioning 1
    // 8769
    // 8825
    // 201 JPEGInterchangeFormat LONG 1
    // 202 JPEGInterchangeFormatLngth LONG 1
    // 213 YCbCrPositioning 1

    QList<XTiff::CHUNK> listResult;
    if (!isPdStructNotCanceled(pPdStruct) || (getEndian() == ENDIAN_UNKNOWN)) return listResult;

    QList<IFD_INFO> listInfo;
    if (!getIFDChain(&listInfo, pPdStruct) || !isPdStructNotCanceled(pPdStruct)) return listResult;

    const bool bIsBigEndian = (getEndian() == ENDIAN_BIG);
    const qint64 nTotalSize = getSize();
    for (const IFD_INFO &info : listInfo) {
        qint64 nEntryOffset = info.nOffset + (qint64)sizeof(quint16);
        for (quint32 i = 0; i < info.nCount; i++, nEntryOffset += (qint64)sizeof(IFD_ENTRY)) {
            CHUNK chunk = {};
            if (getIFDChunk(nEntryOffset, bIsBigEndian, nTotalSize, &chunk, nullptr, pPdStruct)) listResult.append(chunk);
            if (!isPdStructNotCanceled(pPdStruct)) {
                listResult.clear();
                return listResult;
            }
        }
    }

    return listResult;
}

QList<XTiff::CHUNK> XTiff::_getChunksByTag(QList<CHUNK> *pListChunks, quint16 nTag)
{
    QList<XTiff::CHUNK> listResult;

    if (!pListChunks) return listResult;

    qint32 nNumberOfRecords = pListChunks->count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (pListChunks->at(i).nTag == nTag) {
            listResult.append(pListChunks->at(i));
        }
    }

    return listResult;
}

QVector<XBinary::XMETADATA_STRUCT> XTiff::getMetadataStructs()
{
    QVector<XMETADATA_STRUCT> listResult;
    QList<CHUNK> listChunks = getChunks(nullptr);
    const bool bIsBigEndian = (getEndian() == ENDIAN_BIG);

    // DateTimeOriginal and DateTimeDigitized normally live in the Exif sub-IFD.
    const QList<CHUNK> listExifPointers = _getChunksByTag(&listChunks, 0x8769);
    for (qint32 i = 0; i < listExifPointers.count(); ++i) {
        const CHUNK &pointer = listExifPointers.at(i);
        if (!checkOffsetSize(pointer.nOffset, sizeof(quint32))) {
            continue;
        }

        const quint32 nExifIfdOffset = read_uint32(pointer.nOffset, bIsBigEndian);
        IFD_INFO info = {};
        if (!getIFDInfo(nExifIfdOffset, bIsBigEndian, getSize(), &info, nullptr)) {
            continue;
        }

        qint64 nEntryOffset = info.nOffset + (qint64)sizeof(quint16);
        for (quint32 j = 0; j < info.nCount; ++j, nEntryOffset += (qint64)sizeof(IFD_ENTRY)) {
            CHUNK chunk = {};
            if (getIFDChunk(nEntryOffset, bIsBigEndian, getSize(), &chunk, nullptr, nullptr)) {
                listChunks.append(chunk);
            }
        }
    }

    appendUnsignedTagMetadata(this, &listResult, &listChunks, bIsBigEndian, 0x0100, XMETADATA_ID_FRAME_WIDTH, QString("Width"), false);
    appendUnsignedTagMetadata(this, &listResult, &listChunks, bIsBigEndian, 0x0101, XMETADATA_ID_FRAME_HEIGHT, QString("Height"), false);
    appendUnsignedTagMetadata(this, &listResult, &listChunks, bIsBigEndian, 0x0102, XMETADATA_ID_BIT_DEPTH, QString("Bits per sample"), true);
    appendUnsignedTagMetadata(this, &listResult, &listChunks, bIsBigEndian, 0x0106, XMETADATA_ID_COLOR_TYPE, QString("Photometric interpretation"), true);

    appendDateTagMetadata(this, &listResult, &listChunks, 0x0132, XMETADATA_ID_MODIFICATED, QString("Date/time"));
    appendDateTagMetadata(this, &listResult, &listChunks, 0x9003, XMETADATA_ID_DATETIME_CREATED, QString("Date/time original"));
    appendDateTagMetadata(this, &listResult, &listChunks, 0x9004, XMETADATA_ID_DATETIME_CREATED, QString("Date/time digitized"));

    return listResult;
}

QString XTiff::getExifCameraName(QIODevice *pDevice, OFFSETSIZE osExif, QList<CHUNK> *pListChunks)
{
    QString sResult;

    if (pDevice && pListChunks && (osExif.nOffset >= 0) && (osExif.nSize > 0)) {
        SubDevice sd(pDevice, osExif.nOffset, osExif.nSize);

        if (sd.open(QIODevice::ReadOnly)) {
            XTiff tiff(&sd);

            if (tiff.isValid()) {
                QList<CHUNK> listMake = XTiff::_getChunksByTag(pListChunks, 0x10f);
                QList<CHUNK> listModel = XTiff::_getChunksByTag(pListChunks, 0x110);

                QString sMake;
                QString sModel;

                if (listMake.count()) {
                    sMake = tiff.read_ansiString(listMake.at(0).nOffset, listMake.at(0).nSize);
                }

                if (listModel.count()) {
                    sModel = tiff.read_ansiString(listModel.at(0).nOffset, listModel.at(0).nSize);
                }

                if ((sMake != "") || (sModel != "")) {
                    sResult = QString("%1(%2)").arg(sMake).arg(sModel);
                }
            }
        }
    }

    return sResult;
}

QList<XTiff::CHUNK> XTiff::getExifChunks(QIODevice *pDevice, OFFSETSIZE osExif, PDSTRUCT *pPdStruct)
{
    QList<XTiff::CHUNK> listResult;

    if (pDevice && (osExif.nOffset >= 0) && (osExif.nSize > 0) && isPdStructNotCanceled(pPdStruct)) {
        SubDevice sd(pDevice, osExif.nOffset, osExif.nSize);

        if (sd.open(QIODevice::ReadOnly)) {
            XTiff tiff(&sd);

            if (tiff.isValid()) {
                listResult = tiff.getChunks(pPdStruct);
            }

            sd.close();
        }
    }

    return listResult;
}

QString XTiff::getMIMEString()
{
    return "image/tiff-exif";
}

QString XTiff::structIDToString(quint32 nID)
{
    return XBinary::XCONVERT_idToTransString(nID, _TABLE_XTIFF_STRUCTID, sizeof(_TABLE_XTIFF_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QString XTiff::structIDToFtString(quint32 nID)
{
    return XBinary::XCONVERT_idToFtString(nID, _TABLE_XTIFF_STRUCTID, sizeof(_TABLE_XTIFF_STRUCTID) / sizeof(XBinary::XCONVERT));
}

quint32 XTiff::ftStringToStructID(const QString &sFtString)
{
    return XCONVERT_ftStringToId(sFtString, _TABLE_XTIFF_STRUCTID, sizeof(_TABLE_XTIFF_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QList<XBinary::XFHEADER> XTiff::getXFHeaders(const XFSTRUCT &xfStruct, PDSTRUCT *pPdStruct)
{
    QList<XBinary::XFHEADER> listResult;

    if (!isPdStructNotCanceled(pPdStruct) || (getEndian() == ENDIAN_UNKNOWN)) return listResult;

    quint32 nStructID = xfStruct.nStructID;

    bool bIsBigEndian = (getEndian() == ENDIAN_BIG);

    if (nStructID == STRUCTID_UNKNOWN) {
        XFSTRUCT _xfStruct = xfStruct;
        _xfStruct.nStructID = STRUCTID_SIGNATURE;
        _xfStruct.xLoc = offsetToLoc(0);
        listResult.append(getXFHeaders(_xfStruct, pPdStruct));
    } else if (nStructID == STRUCTID_SIGNATURE) {
        if (getSize() < 8) return listResult;

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
            _xfStruct.nStructID = STRUCTID_IFD_TABLE;
            quint32 nIfdOffset = 0;
            if (!readUInt32Exact(4, bIsBigEndian, &nIfdOffset, pPdStruct)) return QList<XBinary::XFHEADER>();
            _xfStruct.xLoc = offsetToLoc(nIfdOffset);
            listResult.append(getXFHeaders(_xfStruct, pPdStruct));
        }
    } else if (nStructID == STRUCTID_IFD_TABLE) {
        qint64 nIfdOffset = locToOffset(xfStruct.pMemoryMap, xfStruct.xLoc);

        if (nIfdOffset == -1) {
            quint32 nIfdOffset32 = 0;
            if (!readUInt32Exact(4, bIsBigEndian, &nIfdOffset32, pPdStruct)) return listResult;
            nIfdOffset = nIfdOffset32;
        }

        IFD_INFO info = {};
        if (getIFDInfo(nIfdOffset, bIsBigEndian, getSize(), &info, pPdStruct)) {
            XFHEADER xfHeader = {};
            xfHeader.sParentTag = xfStruct.sParent;
            xfHeader.fileType = xfStruct.fileType;
            xfHeader.structID = static_cast<XBinary::STRUCTID>(STRUCTID_IFD_TABLE);
            xfHeader.xLoc = offsetToLoc(nIfdOffset + 2);
            xfHeader.nSize = (qint64)sizeof(IFD_ENTRY) * info.nCount;
            xfHeader.xfType = XFTYPE_TABLE;
            xfHeader.listFields = getXFRecords(xfStruct.fileType, STRUCTID_IFD_ENTRY, xfHeader.xLoc);

            qint64 nCurrentOffset = nIfdOffset + 2;
            for (quint32 i = 0; i < info.nCount; i++) {
                if (!isPdStructNotCanceled(pPdStruct)) return QList<XBinary::XFHEADER>();
                xfHeader.listRowLocations.append(nCurrentOffset);
                nCurrentOffset += sizeof(IFD_ENTRY);
            }

            xfHeader.sTag = xfHeaderToTag(xfHeader, structIDToString(STRUCTID_IFD_TABLE), xfHeader.sParentTag);
            listResult.append(xfHeader);
        }
    }

    return listResult;
}

QList<XBinary::XFRECORD> XTiff::getXFRecords(FT fileType, quint32 nStructID, const XLOC &xLoc)
{
    Q_UNUSED(fileType)
    Q_UNUSED(xLoc)

    QList<XBinary::XFRECORD> listResult;

    quint64 nEndianFlag = (getEndian() == ENDIAN_BIG) ? XFRECORD_FLAG_BE : XFRECORD_FLAG_NONE;

    if (nStructID == STRUCTID_SIGNATURE) {
        listResult.append({"ByteOrder", 0, 2, XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        listResult.append({"Magic", 2, 2, nEndianFlag, VT_UINT16});
        listResult.append({"IFDOffset", 4, 4, nEndianFlag | XFRECORD_FLAG_OFFSET, VT_UINT32});
    } else if ((nStructID == STRUCTID_IFD_TABLE) || (nStructID == STRUCTID_IFD_ENTRY)) {
        listResult.append({"Tag", (qint32)offsetof(IFD_ENTRY, nTag), 2, nEndianFlag, VT_UINT16});
        listResult.append({"Type", (qint32)offsetof(IFD_ENTRY, nType), 2, nEndianFlag, VT_UINT16});
        listResult.append({"Count", (qint32)offsetof(IFD_ENTRY, nCount), 4, nEndianFlag | XFRECORD_FLAG_COUNT, VT_UINT32});
        listResult.append({"ValueOffset", (qint32)offsetof(IFD_ENTRY, nOffset), 4, nEndianFlag | XFRECORD_FLAG_OFFSET, VT_UINT32});
    }

    return listResult;
}

// QList<XBinary::DATA_HEADER> XTiff::getDataHeaders(const DATA_HEADERS_OPTIONS &dataHeadersOptions, PDSTRUCT *pPdStruct)
// {
//     QList<DATA_HEADER> listResult;

//     if (dataHeadersOptions.nID == STRUCTID_UNKNOWN) {
//         DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
//         _dataHeadersOptions.bChildren = true;
//         _dataHeadersOptions.dsID_parent = _addDefaultHeaders(&listResult, pPdStruct);
//         _dataHeadersOptions.fileType = getFileType();

//         _dataHeadersOptions.dhMode = XBinary::DHMODE_HEADER;
//         _dataHeadersOptions.nID = STRUCTID_SIGNATURE;
//         _dataHeadersOptions.nLocation = 0;
//         _dataHeadersOptions.locType = XBinary::LT_OFFSET;
//         listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//     } else {
//         qint64 nStartOffset = locationToOffset(dataHeadersOptions.pMemoryMap, dataHeadersOptions.locType, dataHeadersOptions.nLocation);
//         if (nStartOffset != -1) {
//             if (dataHeadersOptions.nID == STRUCTID_SIGNATURE) {
//                 DATA_HEADER dh = _initDataHeader(dataHeadersOptions, structIDToString(dataHeadersOptions.nID));
//                 dh.nSize = 8;
//                 dh.listRecords.append(getDataRecord(0, 2, "Byte order", VT_CHAR_ARRAY, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dh.listRecords.append(getDataRecord(2, 2, "Magic", VT_UINT16, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dh.listRecords.append(getDataRecord(4, 4, "IFD0 offset", VT_UINT32, DRF_OFFSET, isBigEndian() ? ENDIAN_BIG : ENDIAN_LITTLE));
//                 listResult.append(dh);

//                 if (dataHeadersOptions.bChildren) {
//                     DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
//                     _dataHeadersOptions.dhMode = XBinary::DHMODE_TABLE;
//                     _dataHeadersOptions.nID = STRUCTID_IFD_TABLE;
//                     _dataHeadersOptions.nLocation = read_uint32(4, isBigEndian());
//                     _dataHeadersOptions.locType = XBinary::LT_OFFSET;
//                     listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//                 }
//             } else if (dataHeadersOptions.nID == STRUCTID_IFD_TABLE) {
//                 DATA_HEADER dh = _initDataHeader(dataHeadersOptions, structIDToString(dataHeadersOptions.nID));
//                 listResult.append(dh);
//             }
//         }
//     }

//     return listResult;
// }

static bool _tiffCanAppend(qint32 nLimit, const QList<XBinary::FPART> &listResult)
{
    return (nLimit == -1) || (listResult.count() < nLimit);
}

QList<XBinary::FPART> XTiff::getFileParts(quint32 nFileParts, qint32 nLimit, PDSTRUCT *pPdStruct)
{
    QList<FPART> listResult;

    if ((nLimit < -1) || (nLimit == 0) || !isPdStructNotCanceled(pPdStruct)) return listResult;

    const qint64 nTotal = getSize();
    const ENDIAN endian = getEndian();
    if ((nTotal < 8) || (endian == ENDIAN_UNKNOWN) || !isPdStructNotCanceled(pPdStruct)) return listResult;

    QList<IFD_INFO> listInfo;
    if (!getIFDChain(&listInfo, pPdStruct) || !isPdStructNotCanceled(pPdStruct)) return listResult;

    const bool bIsBigEndian = (endian == ENDIAN_BIG);
    qint64 nParsedEnd = 8;

    if ((nFileParts & FILEPART_HEADER) && _tiffCanAppend(nLimit, listResult)) {
        FPART record = {};
        record.filePart = FILEPART_HEADER;
        record.nFileOffset = 0;
        record.nFileSize = 8;
        record.nVirtualAddress = (XADDR)-1;
        record.sName = tr("Header");
        listResult.append(record);
    }

    for (const IFD_INFO &info : listInfo) {
        nParsedEnd = qMax(nParsedEnd, info.nOffset + info.nSize);

        if ((nFileParts & FILEPART_TABLE) && _tiffCanAppend(nLimit, listResult)) {
            FPART record = {};
            record.filePart = FILEPART_TABLE;
            record.nFileOffset = info.nOffset;
            record.nFileSize = info.nSize;
            record.nVirtualAddress = (XADDR)-1;
            record.sName = tr("IFD table");
            listResult.append(record);
        }

        qint64 nEntryOffset = info.nOffset + (qint64)sizeof(quint16);
        for (quint32 i = 0; i < info.nCount; i++, nEntryOffset += (qint64)sizeof(IFD_ENTRY)) {
            CHUNK chunk = {};
            quint16 nType = 0;
            if (getIFDChunk(nEntryOffset, bIsBigEndian, nTotal, &chunk, &nType, pPdStruct) && (chunk.nSize > 4)) {
                // Referenced values may intentionally alias.  Keep each tag's extent, but use a maximum end
                // rather than summing extents so overlap cannot inflate overlay accounting.
                nParsedEnd = qMax(nParsedEnd, chunk.nOffset + chunk.nSize);
                if ((nFileParts & FILEPART_REGION) && _tiffCanAppend(nLimit, listResult)) {
                    FPART record = {};
                    record.filePart = FILEPART_REGION;
                    record.nFileOffset = chunk.nOffset;
                    record.nFileSize = chunk.nSize;
                    record.nVirtualAddress = (XADDR)-1;
                    record.sName = QString("%1-%2").arg(XBinary::valueToHex(chunk.nTag)).arg(XBinary::valueToHex(nType));
                    listResult.append(record);
                }
            }
            if (!isPdStructNotCanceled(pPdStruct)) {
                listResult.clear();
                return listResult;
            }
        }
    }

    if ((nFileParts & FILEPART_OVERLAY) && (nParsedEnd < nTotal) && _tiffCanAppend(nLimit, listResult)) {
        FPART record = {};
        record.filePart = FILEPART_OVERLAY;
        record.nFileOffset = nParsedEnd;
        record.nFileSize = nTotal - nParsedEnd;
        record.nVirtualAddress = (XADDR)-1;
        record.sName = tr("Overlay");
        listResult.append(record);
    }

    if (!isPdStructNotCanceled(pPdStruct)) listResult.clear();
    return listResult;
}

bool XTiff::getIFDInfo(qint64 nOffset, bool bIsBigEndian, qint64 nTotalSize, IFD_INFO *pInfo, PDSTRUCT *pPdStruct)
{
    if (!pInfo) return false;
    *pInfo = IFD_INFO();

    if (!isPdStructNotCanceled(pPdStruct) || (nOffset < 8) || (nTotalSize < 14) || (nOffset > nTotalSize - 6)) return false;

    quint16 nCount = 0;
    if (!readUInt16Exact(nOffset, bIsBigEndian, &nCount, pPdStruct)) return false;

    const qint64 nTableSize = (qint64)sizeof(quint16) + (qint64)sizeof(IFD_ENTRY) * nCount + (qint64)sizeof(quint32);
    if (nOffset > nTotalSize - nTableSize) return false;

    const qint64 nNextOffsetField = nOffset + (qint64)sizeof(quint16) + (qint64)sizeof(IFD_ENTRY) * nCount;
    quint32 nNextOffset = 0;
    if (!readUInt32Exact(nNextOffsetField, bIsBigEndian, &nNextOffset, pPdStruct)) return false;

    pInfo->nOffset = nOffset;
    pInfo->nSize = nTableSize;
    pInfo->nCount = nCount;
    pInfo->nNextOffset = nNextOffset;
    return true;
}

bool XTiff::getIFDChain(QList<IFD_INFO> *pListInfo, PDSTRUCT *pPdStruct)
{
    if (!pListInfo) return false;
    pListInfo->clear();
    if (!isPdStructNotCanceled(pPdStruct)) return false;

    const qint64 nTotalSize = getSize();
    const ENDIAN endian = getEndian();
    if ((nTotalSize < 14) || (endian == ENDIAN_UNKNOWN)) return true;
    const bool bIsBigEndian = (endian == ENDIAN_BIG);

    quint32 nTableOffset = 0;
    if (!readUInt32Exact(4, bIsBigEndian, &nTableOffset, pPdStruct)) return false;

    QSet<quint32> setVisited;
    quint64 nTotalEntries = 0;
    while ((nTableOffset != 0) && (pListInfo->count() < XTIFF_MAX_IFD_TABLES)) {
        if (!isPdStructNotCanceled(pPdStruct)) {
            pListInfo->clear();
            return false;
        }
        if (setVisited.contains(nTableOffset)) break;

        IFD_INFO info = {};
        if (!getIFDInfo(nTableOffset, bIsBigEndian, nTotalSize, &info, pPdStruct)) {
            if (!isPdStructNotCanceled(pPdStruct)) {
                pListInfo->clear();
                return false;
            }
            break;
        }
        if (nTotalEntries > XTIFF_MAX_IFD_ENTRIES - info.nCount) break;

        bool bOverlapsTable = false;
        for (const IFD_INFO &previousInfo : *pListInfo) {
            if ((info.nOffset < previousInfo.nOffset + previousInfo.nSize) && (previousInfo.nOffset < info.nOffset + info.nSize)) {
                bOverlapsTable = true;
                break;
            }
        }
        // Distinct IFDs cannot share structural bytes.  Reject an overlapping link while still allowing
        // a legitimate backward link to an earlier, disjoint table.
        if (bOverlapsTable) break;

        setVisited.insert(nTableOffset);
        pListInfo->append(info);
        nTotalEntries += info.nCount;
        nTableOffset = info.nNextOffset;
    }

    return isPdStructNotCanceled(pPdStruct);
}

bool XTiff::getIFDChunk(qint64 nEntryOffset, bool bIsBigEndian, qint64 nTotalSize, CHUNK *pChunk, quint16 *pType, PDSTRUCT *pPdStruct)
{
    if (!pChunk) return false;
    *pChunk = CHUNK();
    if (pType) *pType = 0;

    if (!isPdStructNotCanceled(pPdStruct) || (nEntryOffset < 0) || (nTotalSize < (qint64)sizeof(IFD_ENTRY)) || (nEntryOffset > nTotalSize - (qint64)sizeof(IFD_ENTRY))) {
        return false;
    }

    quint16 nTag = 0;
    quint16 nType = 0;
    quint32 nCount = 0;
    if (!readUInt16Exact(nEntryOffset + offsetof(IFD_ENTRY, nTag), bIsBigEndian, &nTag, pPdStruct) ||
        !readUInt16Exact(nEntryOffset + offsetof(IFD_ENTRY, nType), bIsBigEndian, &nType, pPdStruct) ||
        !readUInt32Exact(nEntryOffset + offsetof(IFD_ENTRY, nCount), bIsBigEndian, &nCount, pPdStruct)) {
        return false;
    }

    const qint32 nBaseTypeSize = getBaseTypeSize(nType);
    if ((nBaseTypeSize <= 0) || (nCount == 0)) return false;
    const qint64 nDataSize = (qint64)nBaseTypeSize * (qint64)nCount;

    qint64 nDataOffset = nEntryOffset + offsetof(IFD_ENTRY, nOffset);
    if (nDataSize > 4) {
        quint32 nExternalOffset = 0;
        if (!readUInt32Exact(nDataOffset, bIsBigEndian, &nExternalOffset, pPdStruct)) return false;
        nDataOffset = nExternalOffset;
        if ((nDataOffset < 0) || (nDataSize > nTotalSize) || (nDataOffset > nTotalSize - nDataSize)) return false;
    }

    pChunk->nTag = nTag;
    pChunk->nOffset = nDataOffset;
    pChunk->nSize = nDataSize;
    if (pType) *pType = nType;
    return true;
}

bool XTiff::readUInt16Exact(qint64 nOffset, bool bIsBigEndian, quint16 *pValue, PDSTRUCT *pPdStruct)
{
    if (!pValue || !isPdStructNotCanceled(pPdStruct)) return false;
    *pValue = 0;

    quint8 data[2] = {};
    if ((read_array_process(nOffset, (char *)data, (qint64)sizeof(data), pPdStruct) != (qint64)sizeof(data)) || !isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    if (bIsBigEndian) {
        *pValue = ((quint16)data[0] << 8) | data[1];
    } else {
        *pValue = data[0] | ((quint16)data[1] << 8);
    }
    return true;
}

bool XTiff::readUInt32Exact(qint64 nOffset, bool bIsBigEndian, quint32 *pValue, PDSTRUCT *pPdStruct)
{
    if (!pValue || !isPdStructNotCanceled(pPdStruct)) return false;
    *pValue = 0;

    quint8 data[4] = {};
    if ((read_array_process(nOffset, (char *)data, (qint64)sizeof(data), pPdStruct) != (qint64)sizeof(data)) || !isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    if (bIsBigEndian) {
        *pValue = ((quint32)data[0] << 24) | ((quint32)data[1] << 16) | ((quint32)data[2] << 8) | data[3];
    } else {
        *pValue = data[0] | ((quint32)data[1] << 8) | ((quint32)data[2] << 16) | ((quint32)data[3] << 24);
    }
    return true;
}

qint32 XTiff::getBaseTypeSize(quint16 nType)
{
    // 1 = BYTE 8-bit unsigned integer.
    // 2 = ASCII 8-bit byte that contains a 7-bit ASCII code; the last byte must be NUL (binary zero).
    // 3 = SHORT 16-bit (2-byte) unsigned integer.
    // 4 = LONG 32-bit (4-byte) unsigned integer.
    // 5 = RATIONAL Two LONGs: the first represents the numerator of a fraction; the second, the denominator
    // 6 = SBYTE An 8-bit signed (twos-complement) integer.
    // 7 = UNDEFINED An 8-bit byte that may contain anything, depending on the definition of the field.
    // 8 = SSHORT A 16-bit (2-byte) signed (twos-complement) integer.
    // 9 = SLONG A 32-bit (4-byte) signed (twos-complement) integer.
    // 10 = SRATIONAL Two SLONG’s: the first represents the numerator of a fraction, the second the denominator.
    // 11 = FLOAT Single precision (4-byte) IEEE format.
    // 12 = DOUBLE Double precision (8-byte) IEEE format.

    qint32 nResult = 0;

    switch (nType) {
        case 1: nResult = 1; break;
        case 2: nResult = 1; break;
        case 3: nResult = 2; break;
        case 4: nResult = 4; break;
        case 5: nResult = 8; break;
        case 6: nResult = 1; break;
        case 7: nResult = 1; break;
        case 8: nResult = 2; break;
        case 9: nResult = 4; break;
        case 10: nResult = 8; break;
        case 11: nResult = 4; break;
        case 12: nResult = 8; break;
        default: nResult = 0;
    }

    return nResult;
}

QList<QString> XTiff::getSearchSignatures()
{
    QList<QString> listResult;

    listResult.append("'II'..");
    listResult.append("'MM'..");

    return listResult;
}

XBinary *XTiff::createInstance(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    Q_UNUSED(bIsImage)
    Q_UNUSED(nModuleAddress)

    return new XTiff(pDevice);
}

bool XTiff::handleInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XTiff> guardedThis(this);
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

void *XTiff::getInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XTiff> guardedThis(this);
    const bool bHandled = guardedThis->handleInternalInfo(pPdStruct);
    if (!guardedThis || !bHandled) return nullptr;

    return &guardedThis->m_internalInfo;
}

void XTiff::setInternalInfo(void *pInternalInfo)
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
