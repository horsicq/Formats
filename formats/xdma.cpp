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
#include "xdma.h"

#include <QSet>
#include <cstring>

namespace {

const quint16 kDMAInterfaceVersion = 0x0102;
const quint16 kDMAEntryPointCount = 25;
const quint16 kDMAFormatRevision = 4;
const quint16 kDMAFlags = 0xffff;
const qint64 kFooterMarkerSize = 20;
const QByteArray kFooterMarker = QByteArrayLiteral("NSNSNSNSNSNSNSNSNSNS");

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

QString versionToString(quint16 nVersion)
{
    return QStringLiteral("%1.%2").arg(nVersion >> 8).arg(nVersion & 0xff);
}

}  // namespace

static_assert(sizeof(XDMA::HEADER) == 24, "Unexpected DMA header size");

XDMA::XDMA(QIODevice *pDevice) : XBinary(pDevice)
{
}

XDMA::~XDMA()
{
}

bool XDMA::_parseLayout(LAYOUT *pLayout, PDSTRUCT *pPdStruct)
{
    if (!pLayout || !XBinary::isPdStructNotCanceled(pPdStruct) || (getSize() < (qint64)sizeof(HEADER))) {
        return false;
    }

    *pLayout = LAYOUT();
    pLayout->header = _read_HEADER();

    if ((std::memcmp(pLayout->header.magic, "DMA\0", 4) != 0) || (std::memcmp(pLayout->header.embeddedMagic, "DMA\0", 4) != 0) ||
        (pLayout->header.nInterfaceVersion != kDMAInterfaceVersion) || (pLayout->header.nEntryPointCount != kDMAEntryPointCount) ||
        (pLayout->header.nFormatRevision != kDMAFormatRevision) || (pLayout->header.nFlags != kDMAFlags)) {
        return false;
    }

    for (quint32 i = 0; i < sizeof(pLayout->header.nReserved); i++) {
        if (pLayout->header.nReserved[i] != 0) {
            return false;
        }
    }

    pLayout->nEntryTableOffset = sizeof(HEADER);
    pLayout->nEntryTableSize = (qint64)pLayout->header.nEntryPointCount * 2;
    pLayout->nModuleOffset = pLayout->nEntryTableOffset + pLayout->nEntryTableSize;
    pLayout->nFooterOffset = pLayout->header.nFooterOffset;

    if ((pLayout->nFooterOffset < pLayout->nModuleOffset) || !checkOffsetSize(pLayout->nEntryTableOffset, pLayout->nEntryTableSize) ||
        !checkOffsetSize(pLayout->nFooterOffset, kFooterMarkerSize)) {
        return false;
    }

    const qint64 nUnalignedFileSize = pLayout->nFooterOffset + kFooterMarkerSize;
    pLayout->nFileSize = (nUnalignedFileSize + 15) & ~((qint64)15);
    pLayout->nModuleSize = pLayout->nFooterOffset - pLayout->nModuleOffset;
    pLayout->nFooterSize = pLayout->nFileSize - pLayout->nFooterOffset;

    if ((pLayout->nFileSize != getSize()) || (read_array(pLayout->nFooterOffset, kFooterMarkerSize) != kFooterMarker)) {
        return false;
    }

    QSet<quint16> setEntryPoints;
    for (quint16 i = 0; (i < pLayout->header.nEntryPointCount) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        const quint16 nEntryPoint = read_uint16(pLayout->nEntryTableOffset + (qint64)i * 2, false);
        if ((nEntryPoint < pLayout->nModuleOffset) || (nEntryPoint >= pLayout->nFooterOffset) || setEntryPoints.contains(nEntryPoint)) {
            return false;
        }
        setEntryPoints.insert(nEntryPoint);
        pLayout->listEntryPoints.append(nEntryPoint);
    }

    if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    for (qint64 i = nUnalignedFileSize; i < pLayout->nFileSize; i++) {
        if (read_uint8(i) != 0) {
            return false;
        }
    }

    return true;
}

bool XDMA::isValid(PDSTRUCT *pPdStruct)
{
    LAYOUT layout = {};
    return _parseLayout(&layout, pPdStruct);
}

bool XDMA::isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    XDMA xdma(pDevice);
    return xdma.isValid(pPdStruct);
}

bool XDMA::isExecutable()
{
    return true;
}

QString XDMA::getArch()
{
    return QStringLiteral("8086");
}

XBinary::MODE XDMA::getMode()
{
    return MODE_16;
}

XBinary::ENDIAN XDMA::getEndian()
{
    return ENDIAN_LITTLE;
}

XBinary::FT XDMA::getFileType()
{
    return FT_DMA;
}

QString XDMA::getVersion()
{
    LAYOUT layout = {};
    return _parseLayout(&layout, nullptr) ? versionToString(layout.header.nInterfaceVersion) : QString();
}

XBinary::OSNAME XDMA::getOsName()
{
    return OSNAME_MSDOS;
}

QString XDMA::getMIMEString()
{
    return QStringLiteral("application/x-parsec-dma");
}

QString XDMA::getFileFormatExt()
{
    return QStringLiteral("dma");
}

QString XDMA::getFileFormatExtsString()
{
    return QStringLiteral("Parsec DMA driver modules (*.dma)");
}

qint64 XDMA::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    LAYOUT layout = {};
    return _parseLayout(&layout, pPdStruct) ? layout.nFileSize : 0;
}

qint64 XDMA::getImageSize()
{
    LAYOUT layout = {};
    return _parseLayout(&layout, nullptr) ? layout.nFooterOffset : 0;
}

QList<XBinary::MAPMODE> XDMA::getMapModesList()
{
    return {MAPMODE_REGIONS, MAPMODE_DATA};
}

XBinary::_MEMORY_MAP XDMA::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    if (mapMode == MAPMODE_DATA) {
        return _getMemoryMap(FILEPART_DATA | FILEPART_OVERLAY, pPdStruct);
    }

    return _getMemoryMap(FILEPART_HEADER | FILEPART_TABLE | FILEPART_REGION | FILEPART_FOOTER | FILEPART_OVERLAY, pPdStruct);
}

QVector<XBinary::XMETADATA_STRUCT> XDMA::getMetadataStructs()
{
    QVector<XMETADATA_STRUCT> listResult;
    LAYOUT layout = {};
    if (!_parseLayout(&layout, nullptr)) {
        return listResult;
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

    appendMetadata(4, 2, XMETADATA_ID_FILE_VERSION, tr("Interface version"), versionToString(layout.header.nInterfaceVersion));
    appendMetadata(6, 2, XMETADATA_ID_UNKNOWN, tr("Entry-point count"), layout.header.nEntryPointCount);
    appendMetadata(8, 2, XMETADATA_ID_UNKNOWN, tr("Format revision"), layout.header.nFormatRevision);
    appendMetadata(10, 2, XMETADATA_ID_UNKNOWN, tr("Footer offset"), layout.header.nFooterOffset);
    appendMetadata(16, 2, XMETADATA_ID_UNKNOWN, tr("Flags"), QStringLiteral("0x%1").arg(layout.header.nFlags, 4, 16, QLatin1Char('0')));

    for (qint32 i = 0; i < layout.listEntryPoints.count(); i++) {
        appendMetadata(layout.nEntryTableOffset + (qint64)i * 2, 2, XMETADATA_ID_UNKNOWN, tr("Entry point %1").arg(i + 1),
                       QStringLiteral("0x%1").arg(layout.listEntryPoints.at(i), 4, 16, QLatin1Char('0')));
    }

    return listResult;
}

QList<XBinary::FPART> XDMA::getFileParts(quint32 nFileParts, qint32 nLimit, PDSTRUCT *pPdStruct)
{
    QList<FPART> listResult;
    if ((nLimit < -1) || (nLimit == 0)) {
        return listResult;
    }

    LAYOUT layout = {};
    if (!_parseLayout(&layout, pPdStruct)) {
        return listResult;
    }

    appendFilePart(&listResult, nFileParts, FILEPART_HEADER, 0, sizeof(HEADER), tr("Header"), nLimit);
    appendFilePart(&listResult, nFileParts, FILEPART_TABLE, layout.nEntryTableOffset, layout.nEntryTableSize, tr("Entry-point table"), nLimit);
    appendFilePart(&listResult, nFileParts, FILEPART_REGION, layout.nModuleOffset, layout.nModuleSize, tr("Module image"), nLimit);
    appendFilePart(&listResult, nFileParts, FILEPART_FOOTER, layout.nFooterOffset, layout.nFooterSize, tr("Footer"), nLimit);
    appendFilePart(&listResult, nFileParts, FILEPART_DATA, 0, layout.nFileSize, tr("Data"), nLimit);

    return listResult;
}

QList<QString> XDMA::getSearchSignatures()
{
    return {QStringLiteral("'DMA'00")};
}

XBinary *XDMA::createInstance(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    Q_UNUSED(bIsImage)
    Q_UNUSED(nModuleAddress)
    return new XDMA(pDevice);
}

XDMA::HEADER XDMA::_read_HEADER(qint64 nOffset)
{
    HEADER result = {};
    read_array(nOffset + offsetof(HEADER, magic), result.magic, sizeof(result.magic));
    result.nInterfaceVersion = read_uint16(nOffset + offsetof(HEADER, nInterfaceVersion), false);
    result.nEntryPointCount = read_uint16(nOffset + offsetof(HEADER, nEntryPointCount), false);
    result.nFormatRevision = read_uint16(nOffset + offsetof(HEADER, nFormatRevision), false);
    result.nFooterOffset = read_uint16(nOffset + offsetof(HEADER, nFooterOffset), false);
    read_array(nOffset + offsetof(HEADER, embeddedMagic), result.embeddedMagic, sizeof(result.embeddedMagic));
    result.nFlags = read_uint16(nOffset + offsetof(HEADER, nFlags), false);
    read_array(nOffset + offsetof(HEADER, nReserved), reinterpret_cast<char *>(result.nReserved), sizeof(result.nReserved));
    return result;
}

QList<quint16> XDMA::getEntryPoints(PDSTRUCT *pPdStruct)
{
    LAYOUT layout = {};
    return _parseLayout(&layout, pPdStruct) ? layout.listEntryPoints : QList<quint16>();
}

bool XDMA::handleInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XDMA> guardedThis(this);
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

void *XDMA::getInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XDMA> guardedThis(this);
    const bool bHandled = guardedThis->handleInternalInfo(pPdStruct);
    if (!guardedThis || !bHandled) return nullptr;
    return &guardedThis->m_internalInfo;
}

void XDMA::setInternalInfo(void *pInternalInfo)
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
