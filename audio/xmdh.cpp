/* Copyright (c) 2026 hors<horsicq@gmail.com>
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
#include "xmdh.h"
#include "../xmetadataappender.h"

namespace {

const qint64 kMDHControlSize = offsetof(XMDH::HEADER, records);
const qint64 kMDHRecordTableOffset = kMDHControlSize;
const qint64 kMDHRecordTableSize = XMDH::RECORD_COUNT * (qint64)sizeof(XMDH::RECORD);
const quint16 kMDHRecordCodes[XMDH::RECORD_COUNT] = {0x00F0, 0x01D2, 0x02B4, 0x0396, 0x0478, 0x0564, 0x0614, 0x070A};

void appendFilePart(QList<XBinary::FPART> *pList, quint32 nRequestedFileParts, XBinary::FILEPART filePart, qint64 nOffset, qint64 nSize, const QString &sName,
                    qint32 nLimit)
{
    if (!(nRequestedFileParts & filePart) || (nSize <= 0) || ((nLimit != -1) && (pList->count() >= nLimit))) {
        return;
    }

    XBinary::FPART record = {};
    record.filePart = filePart;
    record.nFileOffset = nOffset;
    record.nFileSize = nSize;
    record.nVirtualAddress = (XADDR)-1;
    record.sName = sName;
    pList->append(record);
}

}  // namespace

static_assert(sizeof(XMDH::RECORD) == 8, "Unexpected MDH record size");
static_assert(sizeof(XMDH::HEADER) == 74, "Unexpected MDH header size");

XMDH::XMDH(QIODevice *pDevice) : XBinary(pDevice)
{
}

XMDH::~XMDH()
{
}

bool XMDH::isValid(PDSTRUCT *pPdStruct)
{
    const qint64 nSize = getSize();
    if ((nSize < ((qint64)sizeof(HEADER) + 2)) || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    _MEMORY_MAP memoryMap = XBinary::getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);
    if (!compareSignature(&memoryMap, "'MDH'00", 0, pPdStruct)) {
        return false;
    }

    const HEADER header = _read_HEADER();
    if ((header.nReserved != 0) || (header.nTrackOffset != sizeof(HEADER)) || (header.nActiveRecordCount < 1) || (header.nActiveRecordCount > RECORD_COUNT) ||
        (header.nReserved2 != 0)) {
        return false;
    }

    for (qint32 i = 0; i < RECORD_COUNT; ++i) {
        if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
            return false;
        }

        const RECORD &record = header.records[i];
        if (i < header.nActiveRecordCount) {
            if ((record.nReserved1 != 0) || (record.nReserved2 != 0) || (record.nCode != kMDHRecordCodes[i]) || (record.anLevels[3] == 0)) {
                return false;
            }

            for (qint32 j = 0; j < 4; ++j) {
                if (record.anLevels[j] != (((quint32)record.anLevels[3] * (quint32)(j + 1)) / 4)) {
                    return false;
                }
            }
        } else {
            if ((record.nReserved1 != 0) || (record.nCode != 0) || (record.nReserved2 != 0)) {
                return false;
            }
            for (qint32 j = 0; j < 4; ++j) {
                if (record.anLevels[j] != 0) {
                    return false;
                }
            }
        }
    }

    // 0x60 0x00 is the exact end-of-program marker in every known MDH stream.
    return read_uint16(nSize - 2, false) == 0x0060;
}

bool XMDH::isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    XMDH xmdh(pDevice);
    return xmdh.isValid(pPdStruct);
}

XBinary::FT XMDH::getFileType()
{
    return FT_MDH;
}

XBinary::MODE XMDH::getMode()
{
    return MODE_8;
}

XBinary::ENDIAN XMDH::getEndian()
{
    return ENDIAN_LITTLE;
}

QString XMDH::getMIMEString()
{
    return QStringLiteral("audio/x-parsec-mdh");
}

QString XMDH::getFileFormatExt()
{
    return QStringLiteral("mdh");
}

QString XMDH::getFileFormatExtsString()
{
    return QStringLiteral("MDH (*.mdh)");
}

qint64 XMDH::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    return isValid(pPdStruct) ? getSize() : 0;
}

QList<XBinary::MAPMODE> XMDH::getMapModesList()
{
    QList<MAPMODE> listResult;
    listResult.append(MAPMODE_REGIONS);
    listResult.append(MAPMODE_DATA);
    return listResult;
}

XBinary::_MEMORY_MAP XMDH::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    if (mapMode == MAPMODE_DATA) {
        return _getMemoryMap(FILEPART_DATA | FILEPART_OVERLAY, pPdStruct);
    }

    return _getMemoryMap(FILEPART_HEADER | FILEPART_TABLE | FILEPART_REGION | FILEPART_OVERLAY, pPdStruct);
}

QVector<XBinary::XMETADATA_STRUCT> XMDH::getMetadataStructs()
{
    QVector<XMETADATA_STRUCT> listResult;
    if (!isValid((PDSTRUCT *)nullptr)) {
        return listResult;
    }

    const HEADER header = _read_HEADER();
    const XMetadataAppender appendMetadata(this, &listResult);

    appendMetadata(offsetof(HEADER, nTrackOffset), 2, tr("Track data offset"), header.nTrackOffset);
    appendMetadata(offsetof(HEADER, nActiveRecordCount), 1, tr("Active record count"), header.nActiveRecordCount);
    appendMetadata(header.nTrackOffset, getSize() - header.nTrackOffset, tr("Track data size"), getSize() - header.nTrackOffset);

    for (qint32 i = 0; i < header.nActiveRecordCount; ++i) {
        const qint64 nRecordOffset = offsetof(HEADER, records) + ((qint64)i * sizeof(RECORD));
        appendMetadata(nRecordOffset + offsetof(RECORD, nCode), 2, tr("Record %1 code").arg(i + 1), header.records[i].nCode);
        appendMetadata(nRecordOffset + offsetof(RECORD, anLevels), 4, tr("Record %1 maximum level").arg(i + 1), header.records[i].anLevels[3]);
    }

    return listResult;
}

QList<XBinary::FPART> XMDH::getFileParts(quint32 nFileParts, qint32 nLimit, PDSTRUCT *pPdStruct)
{
    QList<FPART> listResult;
    if ((nLimit < -1) || (nLimit == 0) || !isValid(pPdStruct)) {
        return listResult;
    }

    const HEADER header = _read_HEADER();
    const qint64 nSize = getSize();
    appendFilePart(&listResult, nFileParts, FILEPART_HEADER, 0, kMDHControlSize, tr("Header"), nLimit);
    appendFilePart(&listResult, nFileParts, FILEPART_TABLE, kMDHRecordTableOffset, kMDHRecordTableSize, tr("Record table"), nLimit);
    appendFilePart(&listResult, nFileParts, FILEPART_REGION, header.nTrackOffset, nSize - header.nTrackOffset, tr("Track data"), nLimit);
    appendFilePart(&listResult, nFileParts, FILEPART_DATA, 0, nSize, tr("Data"), nLimit);
    return listResult;
}

QList<QString> XMDH::getSearchSignatures()
{
    QList<QString> listResult;
    listResult.append(QStringLiteral("'MDH'00"));
    return listResult;
}

XBinary *XMDH::createInstance(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    Q_UNUSED(bIsImage)
    Q_UNUSED(nModuleAddress)
    return new XMDH(pDevice);
}

XMDH::HEADER XMDH::_read_HEADER(qint64 nOffset)
{
    HEADER result = {};
    read_array(nOffset, result.magic, sizeof(result.magic));
    result.nReserved = read_uint16(nOffset + offsetof(HEADER, nReserved), false);
    result.nTrackOffset = read_uint16(nOffset + offsetof(HEADER, nTrackOffset), false);
    result.nActiveRecordCount = read_uint8(nOffset + offsetof(HEADER, nActiveRecordCount));
    result.nReserved2 = read_uint8(nOffset + offsetof(HEADER, nReserved2));

    for (qint32 i = 0; i < RECORD_COUNT; ++i) {
        const qint64 nRecordOffset = nOffset + offsetof(HEADER, records) + ((qint64)i * sizeof(RECORD));
        result.records[i].nReserved1 = read_uint8(nRecordOffset + offsetof(RECORD, nReserved1));
        result.records[i].nCode = read_uint16(nRecordOffset + offsetof(RECORD, nCode), false);
        result.records[i].nReserved2 = read_uint8(nRecordOffset + offsetof(RECORD, nReserved2));
        read_array(nRecordOffset + offsetof(RECORD, anLevels), reinterpret_cast<char *>(result.records[i].anLevels), sizeof(result.records[i].anLevels));
    }

    return result;
}

bool XMDH::handleInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XMDH> guardedThis(this);
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

void *XMDH::getInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XMDH> guardedThis(this);
    const bool bHandled = guardedThis->handleInternalInfo(pPdStruct);
    if (!guardedThis || !bHandled) return nullptr;
    return &guardedThis->m_internalInfo;
}

void XMDH::setInternalInfo(void *pInternalInfo)
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
