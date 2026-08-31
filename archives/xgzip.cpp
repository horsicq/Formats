/* Copyright (c) 2022-2026 hors<horsicq@gmail.com>
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
#include "xgzip.h"
#include "Algos/xdeflatedecoder.h"

#include <limits>
#include <new>

namespace {
class GzipDiscardDevice : public QIODevice {
public:
    GzipDiscardDevice() : m_nCRC32(0xFFFFFFFFU), m_nSize(0), m_bError(false)
    {
    }

    quint32 crc32() const
    {
        return m_nCRC32 ^ 0xFFFFFFFFU;
    }

    qint64 outputSize() const
    {
        return m_nSize;
    }

    bool hasError() const
    {
        return m_bError;
    }

    bool isSequential() const override
    {
        return true;
    }

protected:
    qint64 readData(char *pData, qint64 nMaxSize) override
    {
        Q_UNUSED(pData)
        Q_UNUSED(nMaxSize)

        return -1;
    }

    qint64 writeData(const char *pData, qint64 nMaxSize) override
    {
        if ((nMaxSize < 0) || ((nMaxSize > 0) && !pData) || (m_nSize > ((std::numeric_limits<qint64>::max)() - nMaxSize))) {
            m_bError = true;
            return -1;
        }

        qint64 nDone = 0;
        while (nDone < nMaxSize) {
            const qint32 nChunk = (qint32)qMin<qint64>(nMaxSize - nDone, (std::numeric_limits<qint32>::max)());
            m_nCRC32 = XBinary::_getCRC32(pData + nDone, nChunk, m_nCRC32, XBinary::_getCRC32Table_EDB88320());
            nDone += nChunk;
        }
        m_nSize += nMaxSize;

        return nMaxSize;
    }

private:
    quint32 m_nCRC32;
    qint64 m_nSize;
    bool m_bError;
};
}  // namespace

XBinary::XCONVERT _TABLE_XGZIP_STRUCTID[] = {{XGzip::STRUCTID_UNKNOWN, "Unknown", QObject::tr("Unknown")},
                                             {XGzip::STRUCTID_GZIP_HEADER, "GZIP_HEADER", QString("GZIP header")},
                                             {XGzip::STRUCTID_STREAM, "STREAM", QString("Stream")}};

static XBinary::PM_INFO createPMInfo(XBinary::HANDLE_METHOD hm0, XBinary::HANDLE_METHOD hm1 = XBinary::HANDLE_METHOD_UNKNOWN,
                                     XBinary::HANDLE_METHOD hm2 = XBinary::HANDLE_METHOD_UNKNOWN, XBinary::HANDLE_METHOD hm3 = XBinary::HANDLE_METHOD_UNKNOWN)
{
    XBinary::PM_INFO result = {};

    result.hm[0] = hm0;
    result.hm[1] = hm1;
    result.hm[2] = hm2;
    result.hm[3] = hm3;

    return result;
}

XGzip::XGzip(QIODevice *pDevice) : XArchive(pDevice)
{
}

XGzip::~XGzip()
{
}

bool XGzip::isValid(PDSTRUCT *pPdStruct)
{
    qint64 nHeaderSize = 0;
    bool bResult = _getHeaderInfo(&nHeaderSize, nullptr, pPdStruct);

    return bResult;
}

bool XGzip::isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    XGzip xgzip(pDevice);

    return xgzip.isValid(pPdStruct);
}

qint64 XGzip::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    return _calculateRawSize(pPdStruct);
}

QList<XBinary::MAPMODE> XGzip::getMapModesList()
{
    QList<MAPMODE> listResult;

    listResult.append(MAPMODE_REGIONS);

    return listResult;
}

XBinary::_MEMORY_MAP XGzip::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(mapMode)

    _MEMORY_MAP result = {};

    if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
        return result;
    }

    result.fileType = getFileType();
    result.mode = getMode();
    result.sArch = getArch();
    result.endian = getEndian();
    result.sType = getTypeAsString();
    result.nBinarySize = getSize();

    _MEMORY_RECORD memoryRecordHeader = {};
    _MEMORY_RECORD memoryRecord = {};
    _MEMORY_RECORD memoryRecordFooter = {};

    GZIP_UNPACK_CONTEXT context = {};
    const bool bMemberInfo = _getFirstMemberInfo(&context, pPdStruct);
    if (!bMemberInfo || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return _MEMORY_MAP();
    }
    const qint64 nOffset = context.nHeaderSize;

    memoryRecordHeader.nOffset = 0;
    memoryRecordHeader.nAddress = XADDR_MAX;
    memoryRecordHeader.nSize = nOffset;
    memoryRecordHeader.sName = tr("Header");
    memoryRecordHeader.filePart = FILEPART_HEADER;

    result.listRecords.append(memoryRecordHeader);

    memoryRecord.nOffset = nOffset;
    memoryRecord.nAddress = XADDR_MAX;
    memoryRecord.nSize = context.nCompressedSize;
    memoryRecord.sName = tr("Stream");
    memoryRecord.filePart = FILEPART_REGION;

    result.listRecords.append(memoryRecord);

    if (context.bFooterValid) {
        memoryRecordFooter.nOffset = memoryRecord.nOffset + memoryRecord.nSize;
        memoryRecordFooter.nAddress = XADDR_MAX;
        memoryRecordFooter.nSize = 8;
        memoryRecordFooter.sName = tr("Footer");
        memoryRecordFooter.filePart = FILEPART_FOOTER;

        result.listRecords.append(memoryRecordFooter);
    }

    _handleOverlay(&result);

    return result;
}

QString XGzip::getFileFormatExt()
{
    return "gz";
}

QString XGzip::getFileFormatExtsString()
{
    return "GZIP (*.gz)";
}

QString XGzip::getMIMEString()
{
    return "application/gzip";
}

XBinary::FT XGzip::getFileType()
{
    return FT_GZIP;
}

static bool gzipCanAppend(XBinary::PDSTRUCT *pPdStruct, qint32 nLimit, const QList<XBinary::FPART> *pListResult)
{
    return XBinary::isPdStructNotCanceled(pPdStruct) && ((nLimit == -1) || (pListResult->size() < nLimit));
}

QList<XBinary::FPART> XGzip::getFileParts(quint32 nFileParts, qint32 nLimit, PDSTRUCT *pPdStruct)
{
    QList<FPART> listResult;

    if ((nLimit < -1) || (nLimit == 0) || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return listResult;
    }

    const qint64 fileSize = getSize();
    if (fileSize <= 0) return listResult;

    GZIP_UNPACK_CONTEXT context = {};
    const bool bMemberInfo = _getFirstMemberInfo(&context, pPdStruct);
    if (!bMemberInfo || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return listResult;
    }
    const qint64 headerSize = context.nHeaderSize;

    // Header
    if ((nFileParts & FILEPART_HEADER) && gzipCanAppend(pPdStruct, nLimit, &listResult)) {
        FPART header = {};
        header.filePart = FILEPART_HEADER;
        header.nFileOffset = 0;
        header.nFileSize = qBound<qint64>(0, headerSize, fileSize);
        header.nVirtualAddress = XADDR_MAX;
        header.sName = tr("Header");
        listResult.append(header);
    }

    // Region: compressed stream payload (best-effort)
    if ((nFileParts & FILEPART_REGION) && gzipCanAppend(pPdStruct, nLimit, &listResult)) {
        const qint64 payloadOffset = headerSize;
        const qint64 payloadSize = context.nCompressedSize;

        FPART region = {};
        region.filePart = FILEPART_REGION;
        region.nFileOffset = payloadOffset;
        region.nFileSize = payloadSize;
        region.nVirtualAddress = XADDR_MAX;
        region.sName = tr("Stream");
        listResult.append(region);
    }

    // Footer
    if ((nFileParts & FILEPART_FOOTER) && gzipCanAppend(pPdStruct, nLimit, &listResult)) {
        if (context.bFooterValid) {
            FPART footer = {};
            footer.filePart = FILEPART_FOOTER;
            footer.nFileOffset = context.nHeaderSize + context.nCompressedSize;
            footer.nFileSize = 8;
            footer.nVirtualAddress = XADDR_MAX;
            footer.sName = tr("Footer");
            listResult.append(footer);
        }
    }

    // Data: entire file
    if ((nFileParts & FILEPART_DATA) && gzipCanAppend(pPdStruct, nLimit, &listResult)) {
        FPART data = {};
        data.filePart = FILEPART_DATA;
        data.nFileOffset = 0;
        data.nFileSize = fileSize;
        data.nVirtualAddress = XADDR_MAX;
        data.sName = tr("Data");
        listResult.append(data);
    }

    if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
        listResult.clear();
    }
    return listResult;
}

XBinary::MODE XGzip::getMode()
{
    return MODE_DATA;
}

qint32 XGzip::getType()
{
    return TYPE_GZ;
}

XBinary::ENDIAN XGzip::getEndian()
{
    return ENDIAN_LITTLE;  // Gzip is little-endian
}

QString XGzip::typeIdToString(qint32 nType)
{
    QString sResult = tr("Unknown");

    switch (nType) {
        case TYPE_GZ: sResult = QString("GZ"); break;
    }

    return sResult;
}

XBinary::OSNAME XGzip::getOsName()
{
    return OSNAME_MULTIPLATFORM;
}

QString XGzip::structIDToString(quint32 nID)
{
    return XBinary::XCONVERT_idToTransString(nID, _TABLE_XGZIP_STRUCTID, sizeof(_TABLE_XGZIP_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QString XGzip::structIDToFtString(quint32 nID)
{
    return XBinary::XCONVERT_idToFtString(nID, _TABLE_XGZIP_STRUCTID, sizeof(_TABLE_XGZIP_STRUCTID) / sizeof(XBinary::XCONVERT));
}

quint32 XGzip::ftStringToStructID(const QString &sFtString)
{
    return XCONVERT_ftStringToId(sFtString, _TABLE_XGZIP_STRUCTID, sizeof(_TABLE_XGZIP_STRUCTID) / sizeof(XBinary::XCONVERT));
}

// QList<XBinary::DATA_HEADER> XGzip::getDataHeaders(const DATA_HEADERS_OPTIONS &dataHeadersOptions, PDSTRUCT *pPdStruct)
// {
//     QList<XBinary::DATA_HEADER> listResult;

//     if (dataHeadersOptions.nID == STRUCTID_UNKNOWN) {
//         DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
//         _dataHeadersOptions.bChildren = true;
//         _dataHeadersOptions.dsID_parent = _addDefaultHeaders(&listResult, pPdStruct);
//         _dataHeadersOptions.dhMode = XBinary::DHMODE_HEADER;
//         _dataHeadersOptions.fileType = dataHeadersOptions.pMemoryMap->fileType;
//         _dataHeadersOptions.nID = STRUCTID_GZIP_HEADER;
//         _dataHeadersOptions.nLocation = 0;
//         _dataHeadersOptions.locType = XBinary::LT_OFFSET;

//         if (isPdStructNotCanceled(pPdStruct)) {
//             listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//         }
//     } else {
//         qint64 nStartOffset = locationToOffset(dataHeadersOptions.pMemoryMap, dataHeadersOptions.locType, dataHeadersOptions.nLocation);

//         if (nStartOffset != -1) {
//             if (dataHeadersOptions.nID == STRUCTID_GZIP_HEADER) {
//                 XBinary::DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, XGzip::structIDToString(dataHeadersOptions.nID));
//                 dataHeader.nSize = sizeof(GZIP_HEADER);

//                 dataHeader.listRecords.append(getDataRecord(offsetof(GZIP_HEADER, nMagic), 2, "nMagic", VT_UINT16, DRF_UNKNOWN,
//                 dataHeadersOptions.pMemoryMap->endian)); dataHeader.listRecords.append(
//                     getDataRecord(offsetof(GZIP_HEADER, nCompressionMethod), 1, "nCompressionMethod", VT_UINT8, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(GZIP_HEADER, nFileFlags), 1, "nFileFlags", VT_UINT8, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(GZIP_HEADER, nTimeStamp), 4, "nTimeStamp", VT_UINT32, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(GZIP_HEADER, nCompressionFlags), 1, "nCompressionFlags", VT_UINT8, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(GZIP_HEADER, nOS), 1, "nOS", VT_UINT8, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));

//                 listResult.append(dataHeader);
//             }
//         }
//     }

//     return listResult;
// }

QList<XBinary::XFHEADER> XGzip::getXFHeaders(const XFSTRUCT &xfStruct, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    QList<XBinary::XFHEADER> listResult;
    quint32 nStructID = xfStruct.nStructID;

    if (nStructID == STRUCTID_UNKNOWN) {
        XFSTRUCT _xfStruct = xfStruct;
        _xfStruct.nStructID = STRUCTID_GZIP_HEADER;
        _xfStruct.xLoc = offsetToLoc(0);
        listResult.append(getXFHeaders(_xfStruct, pPdStruct));
    } else if (nStructID == STRUCTID_GZIP_HEADER) {
        XLOC headerLoc = xfStruct.xLoc;
        if (headerLoc.locType == LT_UNKNOWN) {
            headerLoc = offsetToLoc(0);
        }

        qint64 nHeaderOffset = locToOffset(xfStruct.pMemoryMap, headerLoc);

        if ((nHeaderOffset != -1) && isOffsetAndSizeValid(xfStruct.pMemoryMap, nHeaderOffset, sizeof(GZIP_HEADER))) {
            XFHEADER xfHeader = {};
            xfHeader.sParentTag = xfStruct.sParent;
            xfHeader.fileType = xfStruct.fileType;
            xfHeader.structID = static_cast<XBinary::STRUCTID>(STRUCTID_GZIP_HEADER);
            xfHeader.xLoc = headerLoc;
            xfHeader.nSize = sizeof(GZIP_HEADER);
            xfHeader.xfType = XFTYPE_HEADER;
            xfHeader.listFields = getXFRecords(xfStruct.fileType, STRUCTID_GZIP_HEADER, headerLoc);
            xfHeader.sTag = xfHeaderToTag(xfHeader, structIDToString(STRUCTID_GZIP_HEADER), xfHeader.sParentTag);
            listResult.append(xfHeader);
        }
    }

    return listResult;
}

QList<XBinary::XFRECORD> XGzip::getXFRecords(FT fileType, quint32 nStructID, const XLOC &xLoc)
{
    Q_UNUSED(fileType)
    Q_UNUSED(xLoc)

    QList<XBinary::XFRECORD> listResult;

    if (nStructID == STRUCTID_GZIP_HEADER) {
        listResult.append({"nMagic", (qint32)offsetof(GZIP_HEADER, nMagic), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"nCompressionMethod", (qint32)offsetof(GZIP_HEADER, nCompressionMethod), 1, XFRECORD_FLAG_NONE, VT_UINT8});
        listResult.append({"nFileFlags", (qint32)offsetof(GZIP_HEADER, nFileFlags), 1, XFRECORD_FLAG_NONE, VT_UINT8});
        listResult.append({"nTimeStamp", (qint32)offsetof(GZIP_HEADER, nTimeStamp), 4, XFRECORD_FLAG_UNIXTIME, VT_UINT32});
        listResult.append({"nCompressionFlags", (qint32)offsetof(GZIP_HEADER, nCompressionFlags), 1, XFRECORD_FLAG_NONE, VT_UINT8});
        listResult.append({"nOS", (qint32)offsetof(GZIP_HEADER, nOS), 1, XFRECORD_FLAG_NONE, VT_UINT8});
    }

    return listResult;
}

XGzip::GZIP_HEADER XGzip::_read_GZIP_HEADER(qint64 nOffset)
{
    GZIP_HEADER result = {};

    read_array(nOffset, (char *)&result, sizeof(GZIP_HEADER));

    return result;
}

qint64 XGzip::getHeaderSize()
{
    qint64 nResult = 0;

    _getHeaderInfo(&nResult);

    return nResult;
}

static bool gzipHasHeaderBytes(qint64 nOffset, qint64 nHeaderLimit, qint64 nSize)
{
    return (nSize >= 0) && (nOffset >= 0) && (nOffset <= nHeaderLimit) && (nSize <= (nHeaderLimit - nOffset));
}

static bool gzipReadZeroTerminatedField(QPointer<XGzip> *pGuardedThis, qint64 *pnOffset, qint64 nHeaderLimit, XBinary::PDSTRUCT *pPdStruct, QString *pValue)
{
    QByteArray baValue;
    const qint32 nMaxStoredStringSize = 0x10000;

    while ((*pnOffset < nHeaderLimit) && XBinary::isPdStructNotCanceled(pPdStruct)) {
        const qint64 nChunkSize = qMin<qint64>(0x1000, nHeaderLimit - *pnOffset);
        if (!(*pGuardedThis)) return false;
        QByteArray baChunk = (*pGuardedThis)->read_array_process(*pnOffset, nChunkSize, pPdStruct);

        if (!(*pGuardedThis) || (baChunk.size() != nChunkSize)) {
            return false;
        }

        const qint32 nTerminatorIndex = baChunk.indexOf('\0');
        const qint32 nValueSize = (nTerminatorIndex == -1) ? baChunk.size() : nTerminatorIndex;

        if (pValue && (baValue.size() < nMaxStoredStringSize)) {
            const qint32 nCopySize = qMin(nValueSize, nMaxStoredStringSize - baValue.size());

            if (nCopySize > 0) {
                baValue.append(baChunk.constData(), nCopySize);
            }
        }

        *pnOffset += nValueSize;

        if (nTerminatorIndex != -1) {
            (*pnOffset)++;  // Include the terminating zero byte.

            if (pValue) {
                *pValue = QString::fromLatin1(baValue);
            }

            return true;
        }
    }

    return false;
}

bool XGzip::_getHeaderInfo(qint64 *pHeaderSize, QString *pFileName, PDSTRUCT *pPdStruct)
{
    QPointer<XGzip> guardedThis(this);
    const qint64 nFixedHeaderSize = (qint64)sizeof(GZIP_HEADER);
    const qint64 nFooterSize = 8;
    const qint64 nFileSize = getSize();

    if (pHeaderSize) {
        *pHeaderSize = 0;
    }

    if (pFileName) {
        pFileName->clear();
    }

    if (!XBinary::isPdStructNotCanceled(pPdStruct) || (nFileSize < (nFixedHeaderSize + nFooterSize))) {
        return false;
    }

    QByteArray baHeader = guardedThis->read_array_process(0, nFixedHeaderSize, pPdStruct);

    if (!guardedThis || (baHeader.size() != nFixedHeaderSize)) {
        return false;
    }

    const quint8 nID1 = (quint8)baHeader.at(0);
    const quint8 nID2 = (quint8)baHeader.at(1);
    const quint8 nCompressionMethod = (quint8)baHeader.at(2);
    const quint8 nFlags = (quint8)baHeader.at(3);

    // Bits 5..7 are reserved by RFC 1952 and must be zero.
    if ((nID1 != 0x1f) || (nID2 != 0x8b) || (nCompressionMethod != 8) || (nFlags & 0xe0)) {
        return false;
    }

    qint64 nOffset = nFixedHeaderSize;
    const qint64 nHeaderLimit = nFileSize - nFooterSize;

    // FEXTRA: two-byte little-endian XLEN followed by XLEN bytes.
    if (nFlags & 0x04) {
        if (!gzipHasHeaderBytes(nOffset, nHeaderLimit, 2)) {
            return false;
        }

        QByteArray baLength = guardedThis->read_array_process(nOffset, 2, pPdStruct);

        if (!guardedThis || (baLength.size() != 2)) {
            return false;
        }

        const quint16 nExtraLength = (quint16)(quint8)baLength.at(0) | ((quint16)(quint8)baLength.at(1) << 8);
        nOffset += 2;

        if (!gzipHasHeaderBytes(nOffset, nHeaderLimit, nExtraLength)) {
            return false;
        }

        nOffset += nExtraLength;
    }

    // FNAME and FCOMMENT are zero-terminated ISO-8859-1 byte strings.
    if ((nFlags & 0x08) && !gzipReadZeroTerminatedField(&guardedThis, &nOffset, nHeaderLimit, pPdStruct, pFileName)) {
        return false;
    }

    if ((nFlags & 0x10) && !gzipReadZeroTerminatedField(&guardedThis, &nOffset, nHeaderLimit, pPdStruct, nullptr)) {
        return false;
    }

    // FHCRC is the low 16 bits of the CRC32 of all preceding header bytes.
    if (nFlags & 0x02) {
        if (!gzipHasHeaderBytes(nOffset, nHeaderLimit, 2)) {
            return false;
        }

        QByteArray baHeaderCRC = guardedThis->read_array_process(nOffset, 2, pPdStruct);
        if (!guardedThis || (baHeaderCRC.size() != 2)) {
            return false;
        }

        const quint16 nExpectedHeaderCRC = (quint16)(quint8)baHeaderCRC.at(0) | ((quint16)(quint8)baHeaderCRC.at(1) << 8);
        const quint32 nCalculatedHeaderCRC = guardedThis->_getCRC32(0, nOffset, 0xFFFFFFFF, XBinary::_getCRC32Table_EDB88320(), pPdStruct);
        if (!guardedThis || !XBinary::isPdStructNotCanceled(pPdStruct) || ((quint16)nCalculatedHeaderCRC != nExpectedHeaderCRC)) {
            return false;
        }

        nOffset += 2;
    }

    if (pHeaderSize) {
        *pHeaderSize = nOffset;
    }

    return XBinary::isPdStructNotCanceled(pPdStruct);
}

bool XGzip::_getFirstMemberInfo(GZIP_UNPACK_CONTEXT *pContext, PDSTRUCT *pPdStruct, const QMap<UNPACK_PROP, QVariant> *pUnpackProperties)
{
    QPointer<XGzip> guardedThis(this);
    if (!pContext) {
        return false;
    }

    *pContext = GZIP_UNPACK_CONTEXT();

    const bool bHeaderInfo = guardedThis->_getHeaderInfo(&pContext->nHeaderSize, &pContext->sFileName, pPdStruct);
    if (!guardedThis || !bHeaderInfo) {
        return false;
    }

    if (pContext->sFileName.isEmpty()) {
        pContext->sFileName = XBinary::getDeviceFileBaseName(guardedThis->getDevice());
    }

    const qint64 nFileSize = getSize();
    const qint64 nRemainingSize = nFileSize - pContext->nHeaderSize;

    if (nRemainingSize <= 0) {
        return false;
    }

    SubDevice sd(guardedThis->getDevice(), pContext->nHeaderSize, nRemainingSize);
    GzipDiscardDevice discardDevice;

    if (!sd.open(QIODevice::ReadOnly) || !guardedThis || !discardDevice.open(QIODevice::WriteOnly) || !guardedThis) {
        return false;
    }

    XBinary::DATAPROCESS_STATE state = {};
    if (pUnpackProperties) state.mapUnpackProperties = *pUnpackProperties;
    state.mapProperties.insert(XBinary::FPART_PROP_HANDLEMETHOD, HANDLE_METHOD_DEFLATE);
    state.pDeviceInput = &sd;
    state.pDeviceOutput = &discardDevice;
    state.nInputOffset = 0;
    state.nInputLimit = nRemainingSize;
    state.nProcessedOffset = 0;
    state.nProcessedLimit = -1;

    const bool bDecoded = XDeflateDecoder::decompress(&state, pPdStruct);
    if (!guardedThis) return false;

    discardDevice.close();
    sd.close();
    if (!guardedThis) return false;

    if (!bDecoded || discardDevice.hasError() || (state.nCountInput <= 0) || (state.nCountInput > nRemainingSize) || (state.nCountOutput < 0) ||
        (discardDevice.outputSize() != state.nCountOutput)) {
        return false;
    }

    pContext->nCompressedSize = state.nCountInput;
    pContext->nUncompressedSize = state.nCountOutput;

    const qint64 nFooterOffset = pContext->nHeaderSize + pContext->nCompressedSize;

    if ((nFooterOffset < pContext->nHeaderSize) || (nFooterOffset > (nFileSize - 8))) {
        return false;
    }

    QByteArray baFooter = guardedThis->read_array_process(nFooterOffset, 8, pPdStruct);
    if (!guardedThis || (baFooter.size() != 8)) {
        return false;
    }

    pContext->nCRC32 =
        (quint32)(quint8)baFooter.at(0) | ((quint32)(quint8)baFooter.at(1) << 8) | ((quint32)(quint8)baFooter.at(2) << 16) | ((quint32)(quint8)baFooter.at(3) << 24);
    const quint32 nFooterUncompressedSize =
        (quint32)(quint8)baFooter.at(4) | ((quint32)(quint8)baFooter.at(5) << 8) | ((quint32)(quint8)baFooter.at(6) << 16) | ((quint32)(quint8)baFooter.at(7) << 24);

    // RFC 1952 stores the uncompressed size modulo 2^32.
    if ((quint32)(quint64)pContext->nUncompressedSize != nFooterUncompressedSize) {
        return false;
    }

    if (discardDevice.crc32() != pContext->nCRC32) {
        return false;
    }

    pContext->bFooterValid = true;
    return XBinary::isPdStructNotCanceled(pPdStruct);
}

QList<XBinary::PM_INFO> XGzip::unpackImplemented()
{
    QList<PM_INFO> listResult;

    listResult.append(createPMInfo(HANDLE_METHOD_DEFLATE));

    return listResult;
}

QMap<XBinary::UNPACK_PROP, QVariant> XGzip::getDefaultUnpackProperties()
{
    QMap<XBinary::UNPACK_PROP, QVariant> result = XArchive::getDefaultUnpackProperties();

    return result;
}

bool XGzip::initUnpack(UNPACK_STATE *pState, const QMap<UNPACK_PROP, QVariant> &mapProperties, PDSTRUCT *pPdStruct)
{
    QPointer<XGzip> guardedThis(this);
    if (m_bUnpackOperationInProgress) {
        return false;
    }
    UNPACK_OPERATION_GUARD operationGuard(&m_bUnpackOperationInProgress);
    if (!operationGuard.isAcquired()) return false;

    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();
    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    if (!pState) {
        return false;
    }

    if ((pState->pContext || !pState->baUnpackSourceToken.isEmpty()) && !guardedThis->ownsUnpackSource(pState)) {
        return false;
    }
    GZIP_UNPACK_CONTEXT *pOldContext = static_cast<GZIP_UNPACK_CONTEXT *>(pState->pContext);
    guardedThis->releaseUnpackSource(pState);
    pState->pContext = nullptr;
    delete pOldContext;
    *pState = UNPACK_STATE();
    if (!XBinary::isPdStructNotCanceled(pPdStruct)) return false;
    const bool bBound = guardedThis->bindUnpackSource(pState, pPdStruct);
    if (!guardedThis || !bBound) {
        return false;
    }

    const bool bValid = guardedThis->isValid(pPdStruct);
    if (!guardedThis) return false;
    if (!bValid) {
        guardedThis->releaseUnpackSource(pState);
        *pState = UNPACK_STATE();
        return false;
    }

    GZIP_UNPACK_CONTEXT parsedContext = {};
    const bool bMemberInfo = guardedThis->_getFirstMemberInfo(&parsedContext, pPdStruct, &mapProperties);
    if (!guardedThis) return false;
    if (!bMemberInfo || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        guardedThis->releaseUnpackSource(pState);
        *pState = UNPACK_STATE();
        return false;
    }

    GZIP_UNPACK_CONTEXT *pContext = new (std::nothrow) GZIP_UNPACK_CONTEXT(parsedContext);
    if (!pContext) {
        guardedThis->releaseUnpackSource(pState);
        *pState = UNPACK_STATE();
        return false;
    }

    pState->nCurrentOffset = 0;
    pState->nTotalSize = getSize();
    pState->nCurrentIndex = 0;
    pState->nNumberOfRecords = 1;  // GZIP contains single compressed stream
    pState->mapUnpackProperties = mapProperties;
    pState->pContext = pContext;

    if (!guardedThis->validateAndFinalizeUnpackSource(pState, pContext, pPdStruct)) {
        if (!guardedThis) return false;
        pState->pContext = nullptr;
        guardedThis->releaseUnpackSource(pState);
        delete pContext;
        *pState = UNPACK_STATE();
        return false;
    }

    return true;
}

XBinary::ARCHIVERECORD XGzip::infoCurrent(UNPACK_STATE *pState, PDSTRUCT *pPdStruct)
{
    QPointer<XGzip> guardedThis(this);
    UNPACK_OPERATION_GUARD operationGuard(&m_bUnpackOperationInProgress, &m_bNestedUnpackInfoAuthorized);
    if (!operationGuard.isAllowed()) return XBinary::ARCHIVERECORD();

    XBinary::ARCHIVERECORD result = {};

    if (!pState || !pState->pContext) return result;
    const bool bSourceCurrent = guardedThis->isUnpackSourceCurrent(pState, pPdStruct);
    if (!guardedThis || !bSourceCurrent || (pState->nTotalSize != guardedThis->getSize())) {
        return result;
    }

    if ((pState->nCurrentIndex < 0) || (pState->nCurrentIndex >= pState->nNumberOfRecords)) {
        return result;
    }

    GZIP_UNPACK_CONTEXT *pContext = (GZIP_UNPACK_CONTEXT *)pState->pContext;

    if ((pContext->nHeaderSize < (qint64)sizeof(GZIP_HEADER)) || (pContext->nCompressedSize <= 0) || (pContext->nHeaderSize > guardedThis->getSize()) ||
        (pContext->nCompressedSize > (guardedThis->getSize() - pContext->nHeaderSize)) ||
        ((pContext->nHeaderSize + pContext->nCompressedSize) > (guardedThis->getSize() - 8))) {
        return ARCHIVERECORD();
    }

    // Fill ARCHIVERECORD
    result.nStreamOffset = pContext->nHeaderSize;
    result.nStreamSize = pContext->nCompressedSize;
    // result.nDecompressedOffset = 0;
    // result.nDecompressedSize = pContext->nUncompressedSize;

    // Set properties
    result.mapProperties.insert(FPART_PROP_ORIGINALNAME, pContext->sFileName);
    result.mapProperties.insert(FPART_PROP_COMPRESSEDSIZE, pContext->nCompressedSize);
    result.mapProperties.insert(FPART_PROP_UNCOMPRESSEDSIZE, pContext->nUncompressedSize);
    result.mapProperties.insert(FPART_PROP_HANDLEMETHOD, HANDLE_METHOD_DEFLATE);

    if (pContext->bFooterValid) {
        result.mapProperties.insert(FPART_PROP_RESULTCRC, pContext->nCRC32);
        result.mapProperties.insert(FPART_PROP_CRC_TYPE, CRC_TYPE_FFFFFFFF_EDB88320_FFFFFFFFF);
    }

    return result;
}

bool XGzip::moveToNext(UNPACK_STATE *pState, PDSTRUCT *pPdStruct)
{
    QPointer<XGzip> guardedThis(this);
    UNPACK_OPERATION_GUARD operationGuard(&m_bUnpackOperationInProgress);
    if (!operationGuard.isAcquired()) return false;

    if (!pState || !pState->pContext) return false;
    const bool bSourceCurrent = guardedThis->isUnpackSourceCurrent(pState, pPdStruct);
    if (!guardedThis || !bSourceCurrent || (pState->nCurrentIndex < 0) || (pState->nCurrentIndex >= pState->nNumberOfRecords) ||
        (pState->nTotalSize != guardedThis->getSize())) {
        return false;
    }

    pState->nCurrentIndex++;
    return pState->nCurrentIndex < pState->nNumberOfRecords;
}

bool XGzip::finishUnpack(UNPACK_STATE *pState, PDSTRUCT *pPdStruct)
{
    QPointer<XGzip> guardedThis(this);
    UNPACK_OPERATION_GUARD operationGuard(&m_bUnpackOperationInProgress);
    if (!operationGuard.isAcquired()) return false;

    Q_UNUSED(pPdStruct)

    if (!pState) {
        return false;
    }

    if ((pState->pContext || !pState->baUnpackSourceToken.isEmpty()) && !guardedThis->ownsUnpackSource(pState)) return false;
    GZIP_UNPACK_CONTEXT *pContext = static_cast<GZIP_UNPACK_CONTEXT *>(pState->pContext);
    pState->pContext = nullptr;
    guardedThis->releaseUnpackSource(pState);
    if (!guardedThis) return false;
    delete pContext;
    if (!guardedThis) return false;

    pState->nCurrentOffset = 0;
    pState->nTotalSize = 0;
    pState->nCurrentIndex = 0;
    pState->nNumberOfRecords = 0;
    pState->mapUnpackProperties.clear();
    pState->mapArchiveProperties.clear();

    return true;
}

QList<XBinary::FPART_PROP> XGzip::getAvailableFPARTProperties()
{
    QList<XBinary::FPART_PROP> listResult;

    listResult.append(FPART_PROP_ORIGINALNAME);
    listResult.append(FPART_PROP_COMPRESSEDSIZE);
    listResult.append(FPART_PROP_UNCOMPRESSEDSIZE);
    listResult.append(FPART_PROP_HANDLEMETHOD);
    listResult.append(FPART_PROP_STREAMOFFSET);
    listResult.append(FPART_PROP_STREAMSIZE);

    return listResult;
}

QList<QString> XGzip::getSearchSignatures()
{
    QList<QString> listResult;

    listResult.append("1F8B08");

    return listResult;
}

XBinary *XGzip::createInstance(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    Q_UNUSED(bIsImage)
    Q_UNUSED(nModuleAddress)

    return new XGzip(pDevice);
}

bool XGzip::handleInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XGzip> guardedThis(this);
    bool bResult = true;

    if (!isInternalInfoHandled()) {
        bResult = guardedThis->XArchive::handleInternalInfo(pPdStruct);
        if (!guardedThis || !bResult) return false;
        XArchive::INTERNAL_INFO *pInfo = static_cast<XArchive::INTERNAL_INFO *>(guardedThis->XArchive::getInternalInfo(pPdStruct));
        if (!guardedThis || !pInfo) return false;
        static_cast<XArchive::INTERNAL_INFO &>(guardedThis->m_internalInfo) = *pInfo;
    }

    return guardedThis && bResult;
}

void *XGzip::getInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XGzip> guardedThis(this);
    const bool bHandled = guardedThis->handleInternalInfo(pPdStruct);
    if (!guardedThis || !bHandled) return nullptr;

    return &guardedThis->m_internalInfo;
}

void XGzip::setInternalInfo(void *pInternalInfo)
{
    if (pInternalInfo) {
        m_internalInfo = *static_cast<INTERNAL_INFO *>(pInternalInfo);
        XArchive::setInternalInfo(static_cast<XArchive::INTERNAL_INFO *>(&m_internalInfo));
    } else {
        m_internalInfo = INTERNAL_INFO();
        XArchive::setInternalInfo(nullptr);
    }
}
