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
#include "xjpeg.h"

static XBinary::XCONVERT _TABLE_XJPEG_STRUCTID[] = {
    {XJpeg::STRUCTID_UNKNOWN, "Unknown", QObject::tr("Unknown")},
    {XJpeg::STRUCTID_SIGNATURE, "Signature", QString("Signature")},
    {XJpeg::STRUCTID_CHUNK, "CHUNK", QString("Chunk")},
};

namespace {
const qint64 JPEG_MIN_SIZE = 20;
const qint64 JPEG_SIGNATURE_SIZE = 2;
const qint64 JPEG_SEGMENT_HEADER_SIZE = 4;
const qint64 JPEG_EXIF_DATA_OFFSET = 10;
const qint32 JPEG_MAX_COMMENT_SIZE = 100;
const qint32 JPEG_MAX_CHUNK_COUNT = 65536;

const quint8 JPEG_MARKER_PREFIX = 0xFF;
const quint8 JPEG_MARKER_STUFFED_ZERO = 0x00;
const quint8 JPEG_MARKER_RST0 = 0xD0;
const quint8 JPEG_MARKER_RST7 = 0xD7;
const quint8 JPEG_MARKER_SOI = 0xD8;
const quint8 JPEG_MARKER_EOI = 0xD9;
const quint8 JPEG_MARKER_SOS = 0xDA;
const quint8 JPEG_MARKER_DQT = 0xDB;
const quint8 JPEG_MARKER_APP0 = 0xE0;
const quint8 JPEG_MARKER_APP1 = 0xE1;
const quint8 JPEG_MARKER_COM = 0xFE;

qint32 jpegStructIdCount()
{
    return sizeof(_TABLE_XJPEG_STRUCTID) / sizeof(_TABLE_XJPEG_STRUCTID[0]);
}

bool isRestartMarker(quint8 nId)
{
    return (nId >= JPEG_MARKER_RST0) && (nId <= JPEG_MARKER_RST7);
}

bool isMarkerWithoutLength(quint8 nId)
{
    return (nId == JPEG_MARKER_SOI) || (nId == JPEG_MARKER_EOI) || (nId == 0x01) || isRestartMarker(nId);
}

qint64 chunkEndOffset(const XJpeg::CHUNK &chunk)
{
    return chunk.nDataOffset + chunk.nDataSize;
}

XJpeg::CHUNK createEntropyCodedDataChunk(qint64 nDataOffset, qint64 nDataEnd)
{
    XJpeg::CHUNK result = {};
    result.bValid = true;
    result.bEntropyCodedData = true;
    result.nDataOffset = nDataOffset;
    result.nDataSize = nDataEnd - nDataOffset;

    return result;
}

XBinary::FPART createFilePart(XBinary::FILEPART filePart, qint64 nOffset, qint64 nSize, const QString &sName)
{
    XBinary::FPART result = {};
    result.filePart = filePart;
    result.nFileOffset = nOffset;
    result.nFileSize = nSize;
    result.nVirtualAddress = (XADDR)-1;
    result.sName = sName;

    return result;
}
}  // namespace

XJpeg::XJpeg(QIODevice *pDevice) : XBinary(pDevice)
{
}

XJpeg::~XJpeg()
{
}

bool XJpeg::isValid(PDSTRUCT *pPdStruct)
{
    bool bIsValid = false;

    if (getSize() >= JPEG_MIN_SIZE) {
        _MEMORY_MAP memoryMap = XBinary::getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);
        bIsValid = compareSignature(&memoryMap, "FFD8FFE0....'JFIF'00", 0, pPdStruct) || compareSignature(&memoryMap, "FFD8FFE1....'Exif'00", 0, pPdStruct) ||
                   compareSignature(&memoryMap, "FFD8FFDB", 0, pPdStruct);

        if (bIsValid && XBinary::isPdStructNotCanceled(pPdStruct)) {
            const QList<CHUNK> listChunks = getChunks(pPdStruct);
            bIsValid = !listChunks.isEmpty() && !listChunks.first().bEntropyCodedData && (listChunks.first().nId == JPEG_MARKER_SOI) &&
                       !listChunks.last().bEntropyCodedData && (listChunks.last().nId == JPEG_MARKER_EOI);
        }
    }

    return bIsValid;
}

bool XJpeg::isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    XJpeg xjpeg(pDevice);

    return xjpeg.isValid(pPdStruct);
}

XBinary::_MEMORY_MAP XJpeg::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    XBinary::_MEMORY_MAP result = {};

    if (mapMode == MAPMODE_UNKNOWN) {
        mapMode = MAPMODE_REGIONS;
    }

    if (mapMode == MAPMODE_REGIONS) {
        result = _getMemoryMap(FILEPART_HEADER | FILEPART_REGION | FILEPART_OVERLAY, pPdStruct);
    }

    return result;
}

XBinary::FT XJpeg::getFileType()
{
    return FT_JPEG;
}

QString XJpeg::getFileFormatExt()
{
    return "jpeg";
}

QString XJpeg::getFileFormatExtsString()
{
    return "JPEG (*.jpeg *.jpg)";
}

qint64 XJpeg::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    return _calculateRawSize(pPdStruct);
}

QString XJpeg::getVersion()
{
    QString sResult;
    QList<CHUNK> listChunks = getChunks();
    const OFFSETSIZE osJFIF = getJFIF(&listChunks);

    if (getDevice() && isJFIFPresent(osJFIF)) {
        SubDevice subDevice(getDevice(), osJFIF.nOffset, osJFIF.nSize);
        if (subDevice.open(QIODevice::ReadOnly)) {
            XJFIF jfif(&subDevice);
            if (jfif.isValid()) {
                sResult = jfif.getVersion();
            }
            subDevice.close();
        }
    }

    return sResult;
}

QList<XJpeg::CHUNK> XJpeg::getChunks(PDSTRUCT *pPdStruct)
{
    QList<CHUNK> listResult;
    const qint64 nTotalSize = getSize();
    bool bComplete = false;

    qint64 nOffset = 0;

    while (XBinary::isPdStructNotCanceled(pPdStruct)) {
        if (nOffset == -1) {
            break;
        }

        CHUNK chunk = _readChunk(nOffset);

        if (!chunk.bValid) {
            break;
        }

        if (listResult.size() >= JPEG_MAX_CHUNK_COUNT) {
            listResult.clear();
            return listResult;
        }

        if (listResult.isEmpty() && (chunk.nId != JPEG_MARKER_SOI)) {
            break;
        }

        listResult.append(chunk);

        nOffset = chunkEndOffset(chunk);

        if (chunk.nId == JPEG_MARKER_SOS) {
            qint64 nDataOffset = nOffset;
            qint64 nEntropyEnd = nTotalSize;
            qint64 nNextMarkerOffset = -1;

            while ((nOffset < nTotalSize) && XBinary::isPdStructNotCanceled(pPdStruct)) {
                const qint64 nPrefixOffset = find_uint8(nOffset, nTotalSize - nOffset, JPEG_MARKER_PREFIX, pPdStruct);

                if ((nPrefixOffset < 0) || (nPrefixOffset >= nTotalSize - 1)) {
                    break;
                }

                qint64 nIdOffset = nPrefixOffset + 1;

                while ((nIdOffset < nTotalSize) && (read_uint8(nIdOffset) == JPEG_MARKER_PREFIX)) {
                    nIdOffset++;
                }

                if (nIdOffset >= nTotalSize) {
                    break;
                }

                const quint8 nId = read_uint8(nIdOffset);

                if ((nId == JPEG_MARKER_STUFFED_ZERO) || isRestartMarker(nId)) {
                    nOffset = nIdOffset + 1;
                    continue;
                }

                nEntropyEnd = nPrefixOffset;
                nNextMarkerOffset = nIdOffset - 1;
                break;
            }

            if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
                listResult.clear();
                return listResult;
            }

            if (nEntropyEnd > nDataOffset) {
                if (listResult.size() >= JPEG_MAX_CHUNK_COUNT) {
                    listResult.clear();
                    return listResult;
                }
                listResult.append(createEntropyCodedDataChunk(nDataOffset, nEntropyEnd));
            }

            if (nNextMarkerOffset < 0) {
                break;
            }

            nOffset = nNextMarkerOffset;
        }

        if (chunk.nId == JPEG_MARKER_EOI) {
            bComplete = true;
            break;
        }
    }

    if (!XBinary::isPdStructNotCanceled(pPdStruct) || !bComplete) {
        listResult.clear();
    }

    return listResult;
}

QList<XJpeg::CHUNK> XJpeg::_getChunksById(QList<CHUNK> *pListChunks, quint8 nId)
{
    QList<XJpeg::CHUNK> listResult;

    if (!pListChunks) {
        return listResult;
    }

    qint32 nNumberOfRecords = pListChunks->count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (pListChunks->at(i).nId == nId) {
            listResult.append(pListChunks->at(i));
        }
    }

    return listResult;
}

QString XJpeg::getComment(QList<CHUNK> *pListChunks, PDSTRUCT *pPdStruct)
{
    XBinary::PDSTRUCT pdStructEmpty = {};

    if (!pPdStruct) {
        pdStructEmpty = XBinary::createPdStruct();
        pPdStruct = &pdStructEmpty;
    }

    QString sResult;

    if (!pListChunks) {
        return sResult;
    }

    QList<XJpeg::CHUNK> listComments = _getChunksById(pListChunks, JPEG_MARKER_COM);

    qint32 nNumberOfRecords = listComments.count();

    for (qint32 i = 0; (i < nNumberOfRecords) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        const CHUNK &chunk = listComments.at(i);
        const qint64 nRemaining = JPEG_MAX_COMMENT_SIZE - sResult.size();

        if (nRemaining <= 0) {
            break;
        }

        if ((chunk.nDataSize < JPEG_SEGMENT_HEADER_SIZE) || (chunk.nDataOffset < 0) || (chunk.nDataOffset > getSize() - chunk.nDataSize)) {
            continue;
        }

        sResult += read_ansiString(chunk.nDataOffset + JPEG_SEGMENT_HEADER_SIZE, (qint32)qMin(nRemaining, chunk.nDataSize - JPEG_SEGMENT_HEADER_SIZE));
    }

    if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
        return {};
    }

    sResult = sResult.remove("\r").remove("\n");

    return sResult;
}

QString XJpeg::getComment()
{
    QList<CHUNK> listChunks = getChunks();

    return getComment(&listChunks);
}

QString XJpeg::getDqtMD5(QList<CHUNK> *pListChunks)
{
    QString sResult;

    if (!pListChunks) {
        return sResult;
    }

    QList<XJpeg::CHUNK> listComments = _getChunksById(pListChunks, JPEG_MARKER_DQT);

    qint32 nNumberOfRecords = listComments.count();

    QCryptographicHash crypto(QCryptographicHash::Md5);

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        const CHUNK &chunk = listComments.at(i);

        if ((chunk.nDataSize < JPEG_SEGMENT_HEADER_SIZE) || (chunk.nDataOffset < 0) || (chunk.nDataOffset > getSize() - chunk.nDataSize)) {
            continue;
        }

        QByteArray baData = read_array(chunk.nDataOffset + JPEG_SEGMENT_HEADER_SIZE, chunk.nDataSize - JPEG_SEGMENT_HEADER_SIZE);

        crypto.addData(baData);
    }

    sResult = crypto.result().toHex();

    return sResult;
}

QString XJpeg::getDqtMD5()
{
    QList<CHUNK> listChunks = getChunks();

    return getDqtMD5(&listChunks);
}

bool XJpeg::isChunkPresent(QList<CHUNK> *pListChunks, quint8 nId)
{
    bool bResult = false;

    if (!pListChunks) {
        return false;
    }

    qint32 nNumberOfRecords = pListChunks->count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (pListChunks->at(i).nId == nId) {
            bResult = true;
            break;
        }
    }

    return bResult;
}

XBinary::OFFSETSIZE XJpeg::getJFIF(QList<CHUNK> *pListChunks)
{
    OFFSETSIZE result = {};

    if (!pListChunks) {
        return result;
    }

    const QList<CHUNK> listAPP0 = _getChunksById(pListChunks, JPEG_MARKER_APP0);

    for (qint32 i = 0; i < listAPP0.count(); ++i) {
        const CHUNK &chunk = listAPP0.at(i);

        if ((chunk.nDataSize >= JPEG_SEGMENT_HEADER_SIZE + 14) && (chunk.nDataOffset >= 0) && (chunk.nDataOffset <= getSize() - chunk.nDataSize) &&
            (read_array(chunk.nDataOffset + JPEG_SEGMENT_HEADER_SIZE, 5) == QByteArray("JFIF\0", 5))) {
            result.nOffset = chunk.nDataOffset + JPEG_SEGMENT_HEADER_SIZE;
            result.nSize = chunk.nDataSize - JPEG_SEGMENT_HEADER_SIZE;
            break;
        }
    }

    return result;
}

bool XJpeg::isJFIFPresent(OFFSETSIZE osJFIF)
{
    return osJFIF.nSize > 0;
}

XBinary::OFFSETSIZE XJpeg::getExif(QList<CHUNK> *pListChunks)
{
    OFFSETSIZE result = {};

    if (!pListChunks) {
        return result;
    }

    QList<CHUNK> listExif = _getChunksById(pListChunks, JPEG_MARKER_APP1);

    if (listExif.count() > 0) {
        CHUNK chunkExif = listExif.at(0);

        if ((chunkExif.nDataSize > JPEG_EXIF_DATA_OFFSET) && (chunkExif.nDataOffset >= 0) && (chunkExif.nDataOffset <= getSize() - chunkExif.nDataSize)) {
            if (read_array(chunkExif.nDataOffset + JPEG_SEGMENT_HEADER_SIZE, 6) == QByteArray("Exif\0\0", 6)) {
                result.nOffset = chunkExif.nDataOffset + JPEG_EXIF_DATA_OFFSET;
                result.nSize = chunkExif.nDataSize - JPEG_EXIF_DATA_OFFSET;
            }
        }
    }

    return result;
}

bool XJpeg::isExifPresent(OFFSETSIZE osExif)
{
    return osExif.nSize;
}

QString XJpeg::getMIMEString()
{
    return "image/jpeg";
}

QVector<XBinary::XMETADATA_STRUCT> XJpeg::getMetadataStructs()
{
    QVector<XMETADATA_STRUCT> listResult;
    QList<CHUNK> listChunks = getChunks(nullptr);

    for (qint32 i = 0; i < listChunks.count(); ++i) {
        const CHUNK &chunk = listChunks.at(i);
        const bool bStartOfFrame = ((chunk.nId >= 0xC0) && (chunk.nId <= 0xC3)) || ((chunk.nId >= 0xC5) && (chunk.nId <= 0xC7)) ||
                                   ((chunk.nId >= 0xC9) && (chunk.nId <= 0xCB)) || ((chunk.nId >= 0xCD) && (chunk.nId <= 0xCF));
        if (!bStartOfFrame || (chunk.nDataSize < 9)) {
            continue;
        }

        auto appendMetadata = [this, &listResult](qint64 nOffset, qint64 nSize, XMETADATA_ID id, const QString &sName, const QVariant &varValue) {
            XMETADATA_STRUCT record = {};
            record.nOffset = nOffset;
            record.nSize = nSize;
            record.nAddress = offsetToAddress(nOffset);
            record.id = id;
            record.sName = sName;
            record.varValue = varValue;
            listResult.append(record);
        };

        appendMetadata(chunk.nDataOffset + 5, 2, XMETADATA_ID_FRAME_HEIGHT, QString("Height"), read_uint16(chunk.nDataOffset + 5, true));
        appendMetadata(chunk.nDataOffset + 7, 2, XMETADATA_ID_FRAME_WIDTH, QString("Width"), read_uint16(chunk.nDataOffset + 7, true));
        appendMetadata(chunk.nDataOffset + 4, 1, XMETADATA_ID_BIT_DEPTH, QString("Sample precision"), read_uint8(chunk.nDataOffset + 4));
        break;
    }

    const OFFSETSIZE osJFIF = getJFIF(&listChunks);

    if (getDevice() && isJFIFPresent(osJFIF)) {
        SubDevice subDevice(getDevice(), osJFIF.nOffset, osJFIF.nSize);
        if (subDevice.open(QIODevice::ReadOnly)) {
            XJFIF jfif(&subDevice);
            if (jfif.isValid()) {
                QVector<XMETADATA_STRUCT> listJFIFMetadata = jfif.getMetadataStructs();
                for (qint32 i = 0; i < listJFIFMetadata.count(); ++i) {
                    XMETADATA_STRUCT &record = listJFIFMetadata[i];
                    record.nOffset += osJFIF.nOffset;
                    record.nAddress = offsetToAddress(record.nOffset);
                }
                listResult += listJFIFMetadata;
            }
            subDevice.close();
        }
    }

    const OFFSETSIZE osExif = getExif(&listChunks);

    if (!getDevice() || (osExif.nOffset < 0) || (osExif.nSize <= 0)) {
        return listResult;
    }

    SubDevice subDevice(getDevice(), osExif.nOffset, osExif.nSize);
    if (!subDevice.open(QIODevice::ReadOnly)) {
        return listResult;
    }

    XTiff tiff(&subDevice);
    if (tiff.isValid()) {
        QVector<XMETADATA_STRUCT> listExifMetadata = tiff.getMetadataStructs();
        for (qint32 i = 0; i < listExifMetadata.count(); ++i) {
            XMETADATA_STRUCT &record = listExifMetadata[i];
            record.nOffset += osExif.nOffset;
            record.nAddress = offsetToAddress(record.nOffset);
        }
        listResult += listExifMetadata;
    }

    subDevice.close();
    return listResult;
}

QString XJpeg::structIDToString(quint32 nID)
{
    return XBinary::XCONVERT_idToTransString(nID, _TABLE_XJPEG_STRUCTID, jpegStructIdCount());
}

QString XJpeg::structIDToFtString(quint32 nID)
{
    return XBinary::XCONVERT_idToFtString(nID, _TABLE_XJPEG_STRUCTID, jpegStructIdCount());
}

quint32 XJpeg::ftStringToStructID(const QString &sFtString)
{
    return XCONVERT_ftStringToId(sFtString, _TABLE_XJPEG_STRUCTID, jpegStructIdCount());
}

QList<XBinary::XFHEADER> XJpeg::getXFHeaders(const XFSTRUCT &xfStruct, PDSTRUCT *pPdStruct)
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
        xfHeader.nSize = 2;
        xfHeader.xfType = XFTYPE_HEADER;
        xfHeader.listFields = getXFRecords(xfStruct.fileType, STRUCTID_SIGNATURE, headerLoc);
        xfHeader.sTag = xfHeaderToTag(xfHeader, structIDToString(STRUCTID_SIGNATURE), xfHeader.sParentTag);
        listResult.append(xfHeader);

        if (xfStruct.bIsParent) {
            XFSTRUCT _xfStruct = xfStruct;
            _xfStruct.sParent = xfHeader.sTag;
            _xfStruct.nStructID = STRUCTID_CHUNK;
            _xfStruct.xLoc = offsetToLoc(2);
            listResult.append(getXFHeaders(_xfStruct, pPdStruct));
        }
    } else if (nStructID == STRUCTID_CHUNK) {
        qint64 nStartOffset = locToOffset(xfStruct.pMemoryMap, xfStruct.xLoc);

        if (nStartOffset == -1) {
            nStartOffset = 2;
        }

        qint64 nFileSize = getSize();

        XFHEADER xfHeader = {};
        xfHeader.sParentTag = xfStruct.sParent;
        xfHeader.fileType = xfStruct.fileType;
        xfHeader.structID = static_cast<XBinary::STRUCTID>(STRUCTID_CHUNK);
        xfHeader.xLoc = offsetToLoc(nStartOffset);
        xfHeader.xfType = XFTYPE_TABLE;

        qint64 nCurrentOffset = nStartOffset;

        while ((nCurrentOffset >= 0) && (nCurrentOffset <= nFileSize - 2) && XBinary::isPdStructNotCanceled(pPdStruct)) {
            const CHUNK chunk = _readChunk(nCurrentOffset);

            if (!chunk.bValid) {
                break;
            }

            const quint8 nId = chunk.nId;

            xfHeader.listRowLocations.append(nCurrentOffset);
            nCurrentOffset = chunkEndOffset(chunk);

            if ((nId == 0xDA) || (nId == 0xD9)) {  // SOS: entropy-coded data follows; EOI: end
                break;
            }
        }

        if (!xfHeader.listRowLocations.isEmpty()) {
            xfHeader.listFields = getXFRecords(xfStruct.fileType, STRUCTID_CHUNK, offsetToLoc(xfHeader.listRowLocations.first()));
            xfHeader.sTag = xfHeaderToTag(xfHeader, structIDToString(STRUCTID_CHUNK), xfHeader.sParentTag);
            listResult.append(xfHeader);
        }
    }

    if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
        listResult.clear();
    }

    return listResult;
}

QList<XBinary::XFRECORD> XJpeg::getXFRecords(FT fileType, quint32 nStructID, const XLOC &xLoc)
{
    Q_UNUSED(fileType)

    QList<XBinary::XFRECORD> listResult;

    if (nStructID == STRUCTID_SIGNATURE) {
        listResult.append({"Marker", 0, 2, XFRECORD_FLAG_BE, VT_UINT16});
    } else if (nStructID == STRUCTID_CHUNK) {
        listResult.append({"Marker", 0, 2, XFRECORD_FLAG_BE, VT_UINT16});

        quint8 nId = read_uint8(xLoc.nLocation + 1);
        bool bStandalone = ((nId == 0xD8) || (nId == 0xD9) || ((nId >= 0xD0) && (nId <= 0xD7)) || (nId == 0x01));

        if (!bStandalone) {
            listResult.append({"Length", 2, 2, XFRECORD_FLAG_BE | XFRECORD_FLAG_SIZE, VT_UINT16});
        }
    }

    return listResult;
}

// QList<XBinary::DATA_HEADER> XJpeg::getDataHeaders(const DATA_HEADERS_OPTIONS &dataHeadersOptions, PDSTRUCT *pPdStruct)
// {
//     QList<DATA_HEADER> listResult;

//     if (dataHeadersOptions.nID == STRUCTID_UNKNOWN) {
//         DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
//         _dataHeadersOptions.bChildren = true;
//         _dataHeadersOptions.dsID_parent = _addDefaultHeaders(&listResult, pPdStruct);
//         _dataHeadersOptions.dhMode = XBinary::DHMODE_HEADER;
//         _dataHeadersOptions.fileType = getFileType();

//         _dataHeadersOptions.nID = STRUCTID_SIGNATURE;
//         _dataHeadersOptions.nLocation = 0;
//         _dataHeadersOptions.locType = XBinary::LT_OFFSET;
//         listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//     } else {
//         qint64 nStartOffset = locationToOffset(dataHeadersOptions.pMemoryMap, dataHeadersOptions.locType, dataHeadersOptions.nLocation);
//         if (nStartOffset != -1) {
//             if (dataHeadersOptions.nID == STRUCTID_SIGNATURE) {
//                 DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, XJpeg::structIDToString(dataHeadersOptions.nID));
//                 dataHeader.nSize = JPEG_SIGNATURE_SIZE;
//                 dataHeader.listRecords.append(getDataRecord(0, JPEG_SIGNATURE_SIZE, "SOI", VT_UINT16, DRF_UNKNOWN, ENDIAN_BIG));
//                 listResult.append(dataHeader);

//                 if (dataHeadersOptions.bChildren) {
//                     DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
//                     _dataHeadersOptions.dhMode = XBinary::DHMODE_TABLE;
//                     _dataHeadersOptions.nID = STRUCTID_CHUNK;
//                     _dataHeadersOptions.nLocation = dataHeadersOptions.nLocation + JPEG_SIGNATURE_SIZE;
//                     _dataHeadersOptions.locType = dataHeadersOptions.locType;
//                     _dataHeadersOptions.nSize = getSize() - JPEG_SIGNATURE_SIZE;
//                     listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//                 }
//             } else if (dataHeadersOptions.nID == STRUCTID_CHUNK) {
//                 DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, XJpeg::structIDToString(dataHeadersOptions.nID));
//                 listResult.append(dataHeader);
//             }
//         }
//     }

//     return listResult;
// }

QList<XBinary::FPART> XJpeg::getFileParts(quint32 nFileParts, qint32 nLimit, PDSTRUCT *pPdStruct)
{
    QList<FPART> listResult;

    if ((nLimit < -1) || (nLimit == 0)) {
        return listResult;
    }

    qint64 nTotal = getSize();
    qint64 nMaxOffset = 0;

    if ((nTotal < JPEG_SIGNATURE_SIZE) || !isValid(pPdStruct)) {
        return listResult;
    }

    if (nFileParts & FILEPART_SIGNATURE) {
        listResult.append(createFilePart(FILEPART_SIGNATURE, 0, JPEG_SIGNATURE_SIZE, tr("Signature")));
        if ((nLimit != -1) && (listResult.count() >= nLimit)) return listResult;
    }

    QList<CHUNK> chunks = getChunks(pPdStruct);

    if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
        listResult.clear();
        return listResult;
    }

    for (int i = 0; i < chunks.size(); ++i) {
        const CHUNK &ch = chunks.at(i);
        if (ch.bEntropyCodedData) {
            if (nFileParts & FILEPART_REGION) {
                listResult.append(createFilePart(FILEPART_REGION, ch.nDataOffset, ch.nDataSize, tr("Data")));
                if ((nLimit != -1) && (listResult.count() >= nLimit)) return listResult;
            }
        } else {
            if (nFileParts & FILEPART_OBJECT) {
                listResult.append(createFilePart(FILEPART_OBJECT, ch.nDataOffset, ch.nDataSize, valueToHex(ch.nId)));
                if ((nLimit != -1) && (listResult.count() >= nLimit)) return listResult;
            }
        }

        nMaxOffset = qMax(nMaxOffset, chunkEndOffset(ch));
    }

    if (nFileParts & FILEPART_OVERLAY) {
        if (nMaxOffset < nTotal) {
            listResult.append(createFilePart(FILEPART_OVERLAY, nMaxOffset, nTotal - nMaxOffset, tr("Overlay")));
            if ((nLimit != -1) && (listResult.count() >= nLimit)) return listResult;
        }
    }

    return listResult;
}

XJpeg::CHUNK XJpeg::_readChunk(qint64 nOffset)
{
    CHUNK result = {};
    const qint64 nTotalSize = getSize();

    if ((nOffset < 0) || (nOffset > nTotalSize - JPEG_SIGNATURE_SIZE)) {
        return result;
    }

    quint8 nBegin = read_uint8(nOffset);

    if (nBegin == JPEG_MARKER_PREFIX) {
        result.nId = read_uint8(nOffset + 1);

        result.nDataOffset = nOffset;

        if (isMarkerWithoutLength(result.nId)) {
            result.nDataSize = JPEG_SIGNATURE_SIZE;
        } else if ((result.nId != JPEG_MARKER_STUFFED_ZERO) && (result.nId != JPEG_MARKER_PREFIX)) {
            if (nOffset > nTotalSize - JPEG_SEGMENT_HEADER_SIZE) {
                return CHUNK();
            }

            const quint16 nLength = read_uint16(nOffset + JPEG_SIGNATURE_SIZE, true);

            if (nLength < 2) {
                return CHUNK();
            }

            result.nDataSize = JPEG_SIGNATURE_SIZE + nLength;
        } else {
            return CHUNK();
        }

        if (result.nDataSize > nTotalSize - nOffset) {
            return CHUNK();
        }

        result.bValid = true;
    }

    return result;
}

QList<QString> XJpeg::getSearchSignatures()
{
    QList<QString> listResult;

    listResult.append("FFD8FF");

    return listResult;
}

XBinary *XJpeg::createInstance(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    Q_UNUSED(bIsImage)
    Q_UNUSED(nModuleAddress)

    return new XJpeg(pDevice);
}

bool XJpeg::handleInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XJpeg> guardedThis(this);
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

void *XJpeg::getInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XJpeg> guardedThis(this);
    const bool bHandled = guardedThis->handleInternalInfo(pPdStruct);
    if (!guardedThis || !bHandled) return nullptr;

    return &guardedThis->m_internalInfo;
}

void XJpeg::setInternalInfo(void *pInternalInfo)
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
