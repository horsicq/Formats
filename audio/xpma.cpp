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
#include "xpma.h"
#include "../xmetadataappender.h"

namespace {

const qint64 kPMAHeaderSize = sizeof(XPMA::HEADER);
const qint64 kPMAControlSize = offsetof(XPMA::HEADER, anChannelOffsets);
const qint64 kPMAChannelTableOffset = kPMAControlSize;
const qint64 kPMAChannelTableSize = XPMA::CHANNEL_COUNT * (qint64)sizeof(quint16);
const qint64 kPMAInstrumentSize = 12;

bool isRangeWithinSize(qint64 nTotalSize, qint64 nOffset, qint64 nByteCount)
{
    return (nOffset >= 0) && (nByteCount >= 0) && (nOffset <= nTotalSize) && (nByteCount <= (nTotalSize - nOffset));
}

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

static_assert(sizeof(XPMA::HEADER) == 25, "Unexpected PMA header size");

XPMA::XPMA(QIODevice *pDevice) : XBinary(pDevice)
{
}

XPMA::~XPMA()
{
}

bool XPMA::isValid(PDSTRUCT *pPdStruct)
{
    const qint64 nSize = getSize();

    if ((nSize <= kPMAHeaderSize) || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    _MEMORY_MAP memoryMap = XBinary::getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);
    if (!compareSignature(&memoryMap, "'PLX'", 0, pPdStruct) || (read_uint8(offsetof(HEADER, nMode)) > 1)) {
        return false;
    }

    bool bHasActiveChannel = false;
    for (qint32 i = 0; i < CHANNEL_COUNT; ++i) {
        if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
            return false;
        }

        qint64 nOffset = read_uint16(kPMAChannelTableOffset + ((qint64)i * 2), false);
        if (nOffset == 0) {
            continue;
        }
        if ((nOffset < kPMAHeaderSize) || (nOffset >= nSize)) {
            return false;
        }

        bHasActiveChannel = true;
        bool bTerminated = false;

        while (nOffset < nSize) {
            if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
                return false;
            }

            const quint8 nFlags = read_uint8(nOffset++);
            if (nFlags == 0) {
                bTerminated = true;
                break;
            }

            if (nFlags & 0x80) {
                if ((nFlags != 0x80) || !isRangeWithinSize(nSize, nOffset, 1)) {
                    return false;
                }
                ++nOffset;
                continue;
            }

            if (nFlags & 0x01) {
                if (!isRangeWithinSize(nSize, nOffset, 2)) {
                    return false;
                }
                const qint64 nInstrumentOffset = read_uint16(nOffset, false);
                nOffset += 2;
                if ((nInstrumentOffset < kPMAHeaderSize) || !isRangeWithinSize(nSize, nInstrumentOffset, kPMAInstrumentSize)) {
                    return false;
                }
            }
            if (nFlags & 0x02) {
                if (!isRangeWithinSize(nSize, nOffset, 1)) {
                    return false;
                }
                ++nOffset;
            }
            if (nFlags & 0x08) {
                if (!isRangeWithinSize(nSize, nOffset, 1)) {
                    return false;
                }
                const quint8 nNote = read_uint8(nOffset++);
                if ((nNote >= 192) || (nNote & 1)) {
                    return false;
                }
            }
            if (nFlags & 0x10) {
                if (!isRangeWithinSize(nSize, nOffset, 2)) {
                    return false;
                }
                nOffset += 2;
            }
            if (nFlags & 0x40) {
                if (!isRangeWithinSize(nSize, nOffset, 2)) {
                    return false;
                }
                nOffset += 2;
            }

            // Every ordinary event ends with its delay byte. Bits 2 and 5 do
            // not carry operands.
            if (!isRangeWithinSize(nSize, nOffset, 1)) {
                return false;
            }
            ++nOffset;
        }

        if (!bTerminated) {
            return false;
        }
    }

    return bHasActiveChannel;
}

bool XPMA::isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    XPMA xpma(pDevice);
    return xpma.isValid(pPdStruct);
}

XBinary::FT XPMA::getFileType()
{
    return FT_PMA;
}

XBinary::MODE XPMA::getMode()
{
    return MODE_16;
}

XBinary::ENDIAN XPMA::getEndian()
{
    return ENDIAN_LITTLE;
}

QString XPMA::getMIMEString()
{
    return QStringLiteral("audio/x-palladix-pma");
}

QString XPMA::getFileFormatExt()
{
    return QStringLiteral("pma");
}

QString XPMA::getFileFormatExtsString()
{
    return QStringLiteral("PMA (*.pma)");
}

qint64 XPMA::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    return isValid(pPdStruct) ? getSize() : 0;
}

QList<XBinary::MAPMODE> XPMA::getMapModesList()
{
    QList<MAPMODE> listResult;
    listResult.append(MAPMODE_REGIONS);
    listResult.append(MAPMODE_DATA);
    return listResult;
}

XBinary::_MEMORY_MAP XPMA::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    if (mapMode == MAPMODE_DATA) {
        return _getMemoryMap(FILEPART_DATA | FILEPART_OVERLAY, pPdStruct);
    }

    return _getMemoryMap(FILEPART_HEADER | FILEPART_TABLE | FILEPART_REGION | FILEPART_OVERLAY, pPdStruct);
}

QVector<XBinary::XMETADATA_STRUCT> XPMA::getMetadataStructs()
{
    QVector<XMETADATA_STRUCT> listResult;
    if (!isValid((PDSTRUCT *)nullptr)) {
        return listResult;
    }

    const HEADER header = _read_HEADER();
    const XMetadataAppender appendMetadata(this, &listResult);

    appendMetadata(offsetof(HEADER, nMode), 1, tr("Mode"), header.nMode);
    appendMetadata(offsetof(HEADER, nSpeedScale), 1, tr("Speed scale"), header.nSpeedScale);
    appendMetadata(offsetof(HEADER, nSpeed), 2, tr("Speed"), header.nSpeed);

    qint32 nActiveChannelCount = 0;
    for (qint32 i = 0; i < CHANNEL_COUNT; ++i) {
        if (header.anChannelOffsets[i] != 0) {
            ++nActiveChannelCount;
        }
    }
    appendMetadata(kPMAChannelTableOffset, kPMAChannelTableSize, tr("Active channel count"), nActiveChannelCount);

    for (qint32 i = 0; i < CHANNEL_COUNT; ++i) {
        const qint64 nFieldOffset = kPMAChannelTableOffset + ((qint64)i * 2);
        appendMetadata(nFieldOffset, 2, tr("Channel %1 offset").arg(i + 1), header.anChannelOffsets[i]);
    }

    return listResult;
}

QList<XBinary::FPART> XPMA::getFileParts(quint32 nFileParts, qint32 nLimit, PDSTRUCT *pPdStruct)
{
    QList<FPART> listResult;

    if ((nLimit < -1) || (nLimit == 0) || !isValid(pPdStruct)) {
        return listResult;
    }

    const qint64 nSize = getSize();
    appendFilePart(&listResult, nFileParts, FILEPART_HEADER, 0, kPMAControlSize, tr("Header"), nLimit);
    appendFilePart(&listResult, nFileParts, FILEPART_TABLE, kPMAChannelTableOffset, kPMAChannelTableSize, tr("Channel table"), nLimit);
    appendFilePart(&listResult, nFileParts, FILEPART_REGION, kPMAHeaderSize, nSize - kPMAHeaderSize, tr("Program data"), nLimit);
    appendFilePart(&listResult, nFileParts, FILEPART_DATA, 0, nSize, tr("Data"), nLimit);
    return listResult;
}

QList<QString> XPMA::getSearchSignatures()
{
    QList<QString> listResult;
    listResult.append(QStringLiteral("'PLX'"));
    return listResult;
}

XBinary *XPMA::createInstance(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    Q_UNUSED(bIsImage)
    Q_UNUSED(nModuleAddress)
    return new XPMA(pDevice);
}

XPMA::HEADER XPMA::_read_HEADER(qint64 nOffset)
{
    HEADER result = {};
    read_array(nOffset, result.magic, sizeof(result.magic));
    result.nMode = read_uint8(nOffset + offsetof(HEADER, nMode));
    result.nSpeedScale = read_uint8(nOffset + offsetof(HEADER, nSpeedScale));
    result.nSpeed = read_uint16(nOffset + offsetof(HEADER, nSpeed), false);
    for (qint32 i = 0; i < CHANNEL_COUNT; ++i) {
        result.anChannelOffsets[i] = read_uint16(nOffset + offsetof(HEADER, anChannelOffsets) + ((qint64)i * 2), false);
    }
    return result;
}

bool XPMA::handleInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XPMA> guardedThis(this);
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

void *XPMA::getInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XPMA> guardedThis(this);
    const bool bHandled = guardedThis->handleInternalInfo(pPdStruct);
    if (!guardedThis || !bHandled) return nullptr;
    return &guardedThis->m_internalInfo;
}

void XPMA::setInternalInfo(void *pInternalInfo)
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
