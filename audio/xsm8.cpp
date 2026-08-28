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
#include "xsm8.h"

namespace {

const qint64 kSM8HeaderSize = 10;
const double kPITClock = 1193182.0;

void appendFilePart(QList<XBinary::FPART> *pList, quint32 nRequestedFileParts, XBinary::FILEPART filePart, qint64 nOffset, qint64 nSize,
                    const QString &sName, qint32 nLimit)
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

static_assert(sizeof(XSM8::HEADER) == 10, "Unexpected SM8 header size");

XSM8::XSM8(QIODevice *pDevice) : XBinary(pDevice)
{
}

XSM8::~XSM8()
{
}

bool XSM8::isValid(PDSTRUCT *pPdStruct)
{
    const qint64 nSize = getSize();

    if (nSize < kSM8HeaderSize) {
        return false;
    }

    _MEMORY_MAP memoryMap = XBinary::getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);
    if (!compareSignature(&memoryMap, "'SM8'000001", 0, pPdStruct)) {
        return false;
    }

    const quint16 nPCMSize = read_uint16(6, false);
    return nSize == (kSM8HeaderSize + (qint64)nPCMSize);
}

bool XSM8::isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    XSM8 xsm8(pDevice);
    return xsm8.isValid(pPdStruct);
}

XBinary::FT XSM8::getFileType()
{
    return FT_SM8;
}

XBinary::MODE XSM8::getMode()
{
    return MODE_8;
}

XBinary::ENDIAN XSM8::getEndian()
{
    return ENDIAN_LITTLE;
}

QString XSM8::getMIMEString()
{
    return QStringLiteral("audio/x-sm8");
}

QString XSM8::getFileFormatExt()
{
    return QStringLiteral("sm8");
}

QString XSM8::getFileFormatExtsString()
{
    return QStringLiteral("SM8 (*.sm8)");
}

qint64 XSM8::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    if (!checkOffsetSize(0, kSM8HeaderSize)) {
        return 0;
    }

    return kSM8HeaderSize + (qint64)read_uint16(6, false);
}

QList<XBinary::MAPMODE> XSM8::getMapModesList()
{
    QList<MAPMODE> listResult;
    listResult.append(MAPMODE_REGIONS);
    listResult.append(MAPMODE_DATA);
    return listResult;
}

XBinary::_MEMORY_MAP XSM8::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    if (mapMode == MAPMODE_DATA) {
        return _getMemoryMap(FILEPART_DATA | FILEPART_OVERLAY, pPdStruct);
    }

    return _getMemoryMap(FILEPART_HEADER | FILEPART_REGION | FILEPART_OVERLAY, pPdStruct);
}

QVector<XBinary::XMETADATA_STRUCT> XSM8::getMetadataStructs()
{
    QVector<XMETADATA_STRUCT> listResult;
    if (!isValid((PDSTRUCT *)nullptr)) {
        return listResult;
    }

    const HEADER header = _read_HEADER();
    const double nSampleRate = header.nPITDivisor ? (kPITClock / (double)header.nPITDivisor) : 0.0;

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

    appendMetadata(0, 6, XMETADATA_ID_CODEC, tr("Codec"), QStringLiteral("Unsigned 8-bit PCM"));
    appendMetadata(0, 6, XMETADATA_ID_CHANNELS, tr("Channels"), 1);
    appendMetadata(0, 6, XMETADATA_ID_BIT_DEPTH, tr("Bits per sample"), 8);
    appendMetadata(6, 2, XMETADATA_ID_UNKNOWN, tr("PCM byte count"), header.nPCMSize);
    appendMetadata(8, 2, XMETADATA_ID_UNKNOWN, tr("PIT divisor"), header.nPITDivisor);

    if (header.nPITDivisor) {
        appendMetadata(8, 2, XMETADATA_ID_SAMPLE_RATE, tr("Sample rate"), nSampleRate);
        appendMetadata(10, header.nPCMSize, XMETADATA_ID_BITRATE, tr("Bitrate"), nSampleRate * 8.0);
        appendMetadata(10, header.nPCMSize, XMETADATA_ID_DURATION, tr("Duration"), (double)header.nPCMSize / nSampleRate);
    }

    return listResult;
}

QList<XBinary::FPART> XSM8::getFileParts(quint32 nFileParts, qint32 nLimit, PDSTRUCT *pPdStruct)
{
    QList<FPART> listResult;

    if ((nLimit < -1) || (nLimit == 0) || !isValid(pPdStruct)) {
        return listResult;
    }

    const qint64 nPCMSize = read_uint16(6, false);
    appendFilePart(&listResult, nFileParts, FILEPART_HEADER, 0, kSM8HeaderSize, tr("Header"), nLimit);
    appendFilePart(&listResult, nFileParts, FILEPART_REGION, kSM8HeaderSize, nPCMSize, tr("PCM data"), nLimit);
    appendFilePart(&listResult, nFileParts, FILEPART_DATA, 0, kSM8HeaderSize + nPCMSize, tr("Data"), nLimit);

    return listResult;
}

QList<QString> XSM8::getSearchSignatures()
{
    QList<QString> listResult;
    listResult.append(QStringLiteral("'SM8'000001"));
    return listResult;
}

XBinary *XSM8::createInstance(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    Q_UNUSED(bIsImage)
    Q_UNUSED(nModuleAddress)
    return new XSM8(pDevice);
}

XSM8::HEADER XSM8::_read_HEADER(qint64 nOffset)
{
    HEADER result = {};
    read_array(nOffset, result.magic, sizeof(result.magic));
    result.nPCMSize = read_uint16(nOffset + offsetof(HEADER, nPCMSize), false);
    result.nPITDivisor = read_uint16(nOffset + offsetof(HEADER, nPITDivisor), false);
    return result;
}

bool XSM8::handleInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XSM8> guardedThis(this);
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

void *XSM8::getInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XSM8> guardedThis(this);
    const bool bHandled = guardedThis->handleInternalInfo(pPdStruct);
    if (!guardedThis || !bHandled) return nullptr;
    return &guardedThis->m_internalInfo;
}

void XSM8::setInternalInfo(void *pInternalInfo)
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
