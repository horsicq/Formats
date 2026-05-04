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
const qint64 JPEG_DRI_SEGMENT_SIZE = 6;
const qint64 JPEG_EXIF_DATA_OFFSET = 10;
const qint32 JPEG_MAX_COMMENT_SIZE = 100;

const qint64 JFIF_ID_OFFSET = 6;
const qint32 JFIF_ID_SIZE = 5;
const qint64 JFIF_VERSION_MAJOR_OFFSET = 0x0B;
const qint64 JFIF_VERSION_MINOR_OFFSET = 0x0C;

const quint8 JPEG_MARKER_PREFIX = 0xFF;
const quint8 JPEG_MARKER_STUFFED_ZERO = 0x00;
const quint8 JPEG_MARKER_RST0 = 0xD0;
const quint8 JPEG_MARKER_RST7 = 0xD7;
const quint8 JPEG_MARKER_SOI = 0xD8;
const quint8 JPEG_MARKER_EOI = 0xD9;
const quint8 JPEG_MARKER_SOS = 0xDA;
const quint8 JPEG_MARKER_DQT = 0xDB;
const quint8 JPEG_MARKER_DRI = 0xDD;
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
    return (nId == JPEG_MARKER_SOI) || (nId == JPEG_MARKER_EOI) || isRestartMarker(nId);
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
    result.nVirtualAddress = -1;
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
    }

    return bIsValid;
}

bool XJpeg::isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    XJpeg xjpeg(pDevice);

    return xjpeg.isValid();
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

    QString sIdent = read_ansiString(JFIF_ID_OFFSET, JFIF_ID_SIZE);

    if (sIdent == "JFIF") {
        quint8 nMajor = read_uint8(JFIF_VERSION_MAJOR_OFFSET);
        quint8 nMinor = read_uint8(JFIF_VERSION_MINOR_OFFSET);

        sResult = QString("%1.%2").arg(QString::number(nMajor), QString::number(nMinor));
    }

    return sResult;
}

QList<XJpeg::CHUNK> XJpeg::getChunks(PDSTRUCT *pPdStruct)
{
    QList<CHUNK> listResult;

    qint64 nOffset = 0;

    while (XBinary::isPdStructNotCanceled(pPdStruct)) {
        if (nOffset == -1) {
            break;
        }

        CHUNK chunk = _readChunk(nOffset);

        if (!chunk.bValid) {
            break;
        }

        listResult.append(chunk);

        nOffset = chunkEndOffset(chunk);

        if (chunk.nId == JPEG_MARKER_SOS) {
            qint64 nDataOffset = nOffset;

            while (true) {
                nOffset = find_uint8(nOffset, -1, JPEG_MARKER_PREFIX, pPdStruct);

                if (nOffset == -1) {
                    break;
                }

                if (read_uint8(nOffset + 1) != JPEG_MARKER_STUFFED_ZERO) {
                    break;
                } else {
                    nOffset++;
                }
            }

            listResult.append(createEntropyCodedDataChunk(nDataOffset, nOffset));
        }

        if (chunk.nId == JPEG_MARKER_EOI) {
            break;
        }
    }

    return listResult;
}

QList<XJpeg::CHUNK> XJpeg::_getChunksById(QList<CHUNK> *pListChunks, quint8 nId)
{
    QList<XJpeg::CHUNK> listResult;

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

    QList<XJpeg::CHUNK> listComments = _getChunksById(pListChunks, JPEG_MARKER_COM);

    qint32 nNumberOfRecords = listComments.count();

    for (qint32 i = 0; (i < nNumberOfRecords) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        sResult += read_ansiString(listComments.at(i).nDataOffset + JPEG_SEGMENT_HEADER_SIZE, listComments.at(i).nDataSize - JPEG_SEGMENT_HEADER_SIZE);
    }

    if (sResult.size() > JPEG_MAX_COMMENT_SIZE) {
        sResult.resize(JPEG_MAX_COMMENT_SIZE);
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

    QList<XJpeg::CHUNK> listComments = _getChunksById(pListChunks, JPEG_MARKER_DQT);

    qint32 nNumberOfRecords = listComments.count();

    QCryptographicHash crypto(QCryptographicHash::Md5);

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        QByteArray baData = read_array(listComments.at(i).nDataOffset + JPEG_SEGMENT_HEADER_SIZE, listComments.at(i).nDataSize - JPEG_SEGMENT_HEADER_SIZE);

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

    qint32 nNumberOfRecords = pListChunks->count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (pListChunks->at(i).nId == nId) {
            bResult = true;
            break;
        }
    }

    return bResult;
}

XBinary::OFFSETSIZE XJpeg::getExif(QList<CHUNK> *pListChunks)
{
    OFFSETSIZE result = {};

    QList<CHUNK> listExif = _getChunksById(pListChunks, JPEG_MARKER_APP1);

    if (listExif.count() > 0) {
        CHUNK chunkExif = listExif.at(0);

        if (chunkExif.nDataSize > JPEG_EXIF_DATA_OFFSET) {
            if (read_ansiString(chunkExif.nDataOffset + JPEG_SEGMENT_HEADER_SIZE) == "Exif") {
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

QList<XBinary::DATA_HEADER> XJpeg::getDataHeaders(const DATA_HEADERS_OPTIONS &dataHeadersOptions, PDSTRUCT *pPdStruct)
{
    QList<DATA_HEADER> listResult;

    if (dataHeadersOptions.nID == STRUCTID_UNKNOWN) {
        DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
        _dataHeadersOptions.bChildren = true;
        _dataHeadersOptions.dsID_parent = _addDefaultHeaders(&listResult, pPdStruct);
        _dataHeadersOptions.dhMode = XBinary::DHMODE_HEADER;
        _dataHeadersOptions.fileType = getFileType();

        _dataHeadersOptions.nID = STRUCTID_SIGNATURE;
        _dataHeadersOptions.nLocation = 0;
        _dataHeadersOptions.locType = XBinary::LT_OFFSET;
        listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
    } else {
        qint64 nStartOffset = locationToOffset(dataHeadersOptions.pMemoryMap, dataHeadersOptions.locType, dataHeadersOptions.nLocation);
        if (nStartOffset != -1) {
            if (dataHeadersOptions.nID == STRUCTID_SIGNATURE) {
                DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, XJpeg::structIDToString(dataHeadersOptions.nID));
                dataHeader.nSize = JPEG_SIGNATURE_SIZE;
                dataHeader.listRecords.append(getDataRecord(0, JPEG_SIGNATURE_SIZE, "SOI", VT_UINT16, DRF_UNKNOWN, ENDIAN_BIG));
                listResult.append(dataHeader);

                if (dataHeadersOptions.bChildren) {
                    DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
                    _dataHeadersOptions.dhMode = XBinary::DHMODE_TABLE;
                    _dataHeadersOptions.nID = STRUCTID_CHUNK;
                    _dataHeadersOptions.nLocation = dataHeadersOptions.nLocation + JPEG_SIGNATURE_SIZE;
                    _dataHeadersOptions.locType = dataHeadersOptions.locType;
                    _dataHeadersOptions.nSize = getSize() - JPEG_SIGNATURE_SIZE;
                    listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
                }
            } else if (dataHeadersOptions.nID == STRUCTID_CHUNK) {
                DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, XJpeg::structIDToString(dataHeadersOptions.nID));
                listResult.append(dataHeader);
            }
        }
    }

    return listResult;
}

QList<XBinary::FPART> XJpeg::getFileParts(quint32 nFileParts, qint32 nLimit, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(nLimit)
    QList<FPART> listResult;

    qint64 nTotal = getSize();
    qint64 nMaxOffset = 0;

    if (nFileParts & FILEPART_SIGNATURE) {
        listResult.append(createFilePart(FILEPART_SIGNATURE, 0, JPEG_SIGNATURE_SIZE, tr("Signature")));
    }

    QList<CHUNK> chunks = getChunks(pPdStruct);
    for (int i = 0; i < chunks.size(); ++i) {
        const CHUNK &ch = chunks.at(i);
        if (ch.bEntropyCodedData) {
            if (nFileParts & FILEPART_REGION) {
                listResult.append(createFilePart(FILEPART_REGION, ch.nDataOffset, ch.nDataSize, tr("Data")));
            }
        } else {
            if (nFileParts & FILEPART_OBJECT) {
                listResult.append(createFilePart(FILEPART_OBJECT, ch.nDataOffset, ch.nDataSize, valueToHex(ch.nId)));
            }
        }

        nMaxOffset = qMax(nMaxOffset, chunkEndOffset(ch));
    }

    if (nFileParts & FILEPART_OVERLAY) {
        if (nMaxOffset < nTotal) {
            listResult.append(createFilePart(FILEPART_OVERLAY, nMaxOffset, nTotal - nMaxOffset, tr("Overlay")));
        }
    }

    return listResult;
}

XJpeg::CHUNK XJpeg::_readChunk(qint64 nOffset)
{
    CHUNK result = {};

    quint8 nBegin = read_uint8(nOffset);

    if (nBegin == JPEG_MARKER_PREFIX) {
        result.bValid = true;
        result.nId = read_uint8(nOffset + 1);

        result.nDataOffset = nOffset;

        if (isMarkerWithoutLength(result.nId)) {
            result.nDataSize = JPEG_SIGNATURE_SIZE;
        } else if (result.nId == JPEG_MARKER_DRI) {
            result.nDataSize = JPEG_DRI_SEGMENT_SIZE;
        } else if (result.nId != JPEG_MARKER_STUFFED_ZERO) {
            result.nDataSize = JPEG_SIGNATURE_SIZE + read_uint16(nOffset + JPEG_SIGNATURE_SIZE, true);
        } else {
            result.bValid = false;
        }
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
