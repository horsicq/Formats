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
#include "xmus.h"
#include "../xmetadataappender.h"

#include <QSet>

namespace {

const qint64 kHeaderSize = 24;
const qint64 kFooterSize = 20;
const qint64 kEntryPointOffset = 24;
const qint64 kMaximumPaddingSize = 15;
const quint16 kVersion100 = 0x0100;
const quint16 kEntryPointCount = 15;
const quint16 kInterfaceCount = 7;

const quint8 g_footer[kFooterSize] = {0x4E, 0x53, 0x4E, 0x53, 0x4E, 0x53, 0x4E, 0x53, 0x4E, 0x53, 0x4E, 0x53, 0x4E, 0x53, 0x4E, 0x53, 0x4E, 0x53, 0x4E, 0x53};

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
    record.nVirtualSize = nSize;
    record.sName = sName;
    pList->append(record);
}

}  // namespace

static_assert(sizeof(XMUS::HEADER) == kHeaderSize, "Unexpected MUS driver header size");

XMUS::XMUS(QIODevice *pDevice) : XBinary(pDevice)
{
}

XMUS::~XMUS()
{
}

bool XMUS::isValid(PDSTRUCT *pPdStruct)
{
    const qint64 nFileSize = getSize();

    if (nFileSize < kHeaderSize + kFooterSize) {
        return false;
    }

    _MEMORY_MAP memoryMap = XBinary::getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);
    if (!compareSignature(&memoryMap, "'MUS'00", 0, pPdStruct) || !compareSignature(&memoryMap, "'PMA'00", 12, pPdStruct)) {
        return false;
    }

    const HEADER header = _read_HEADER();
    if ((header.nVersion != kVersion100) || (header.nEntryPointCount != kEntryPointCount) || (header.nInterfaceCount != kInterfaceCount)) {
        return false;
    }

    for (quint32 i = 0; i < 4; i++) {
        if (header.nReserved[i] != 0) {
            return false;
        }
    }

    const qint64 nEntryPointSize = (qint64)header.nEntryPointCount * sizeof(quint16);
    const qint64 nEntryPointEnd = kEntryPointOffset + nEntryPointSize;
    const qint64 nLogicalSize = (qint64)header.nFooterOffset + kFooterSize;
    const qint64 nExpectedFileSize = (nLogicalSize + 15) & ~((qint64)15);

    if ((nEntryPointEnd >= header.nFooterOffset) || (nExpectedFileSize != nFileSize) || (nFileSize - nLogicalSize > kMaximumPaddingSize)) {
        return false;
    }

    QSet<quint16> setEntryPoints;
    for (quint32 i = 0; (i < header.nEntryPointCount) && isPdStructNotCanceled(pPdStruct); i++) {
        const quint16 nEntryPoint = read_uint16(kEntryPointOffset + (qint64)i * sizeof(quint16), false);
        if ((nEntryPoint < nEntryPointEnd) || (nEntryPoint >= header.nFooterOffset) || setEntryPoints.contains(nEntryPoint)) {
            return false;
        }
        setEntryPoints.insert(nEntryPoint);
    }

    if (!isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    char footer[kFooterSize] = {};
    if (read_array(header.nFooterOffset, footer, sizeof(footer)) != sizeof(footer)) {
        return false;
    }

    for (qint32 i = 0; i < kFooterSize; i++) {
        if ((quint8)footer[i] != g_footer[i]) {
            return false;
        }
    }

    const qint64 nPaddingSize = nFileSize - nLogicalSize;
    if (nPaddingSize > 0) {
        char padding[kMaximumPaddingSize] = {};
        if (read_array(nLogicalSize, padding, nPaddingSize) != nPaddingSize) {
            return false;
        }
        for (qint32 i = 0; i < nPaddingSize; i++) {
            if (padding[i] != 0) {
                return false;
            }
        }
    }

    return true;
}

bool XMUS::isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    XMUS xmus(pDevice);
    return xmus.isValid(pPdStruct);
}

bool XMUS::isExecutable()
{
    return true;
}

QString XMUS::getArch()
{
    return QStringLiteral("8086");
}

XBinary::MODE XMUS::getMode()
{
    return MODE_16;
}

XBinary::ENDIAN XMUS::getEndian()
{
    return ENDIAN_LITTLE;
}

XBinary::OSNAME XMUS::getOsName()
{
    return OSNAME_MSDOS;
}

XBinary::FT XMUS::getFileType()
{
    return FT_MUS;
}

qint32 XMUS::getType()
{
    return TYPE_PALLADIX_MUSIC_DRIVER;
}

QString XMUS::typeIdToString(qint32 nType)
{
    if (nType == TYPE_PALLADIX_MUSIC_DRIVER) {
        return tr("Palladix music driver");
    }

    return tr("Unknown");
}

QString XMUS::getVersion()
{
    if (!checkOffsetSize(offsetof(HEADER, nVersion), sizeof(quint16))) {
        return QString();
    }

    const quint16 nVersion = read_uint16(offsetof(HEADER, nVersion), false);
    return QStringLiteral("%1.%2").arg(nVersion >> 8).arg(nVersion & 0xFF, 2, 10, QChar('0'));
}

QString XMUS::getMIMEString()
{
    return QStringLiteral("application/x-palladix-mus-driver");
}

QString XMUS::getFileFormatExt()
{
    return QStringLiteral("mus");
}

QString XMUS::getFileFormatExtsString()
{
    return QStringLiteral("MUS (*.mus)");
}

qint64 XMUS::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    if (!checkOffsetSize(0, kHeaderSize)) {
        return 0;
    }

    return (qint64)read_uint16(offsetof(HEADER, nFooterOffset), false) + kFooterSize;
}

qint64 XMUS::getImageSize()
{
    if (!checkOffsetSize(0, kHeaderSize)) {
        return 0;
    }

    return read_uint16(offsetof(HEADER, nFooterOffset), false);
}

QList<XBinary::MAPMODE> XMUS::getMapModesList()
{
    return {MAPMODE_REGIONS, MAPMODE_DATA};
}

XBinary::_MEMORY_MAP XMUS::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    if (mapMode == MAPMODE_DATA) {
        return _getMemoryMap(FILEPART_DATA | FILEPART_OVERLAY, pPdStruct);
    }

    return _getMemoryMap(FILEPART_HEADER | FILEPART_TABLE | FILEPART_REGION | FILEPART_FOOTER | FILEPART_OVERLAY, pPdStruct);
}

QVector<XBinary::XMETADATA_STRUCT> XMUS::getMetadataStructs()
{
    QVector<XMETADATA_STRUCT> listResult;
    if (!isValid((PDSTRUCT *)nullptr)) {
        return listResult;
    }

    const HEADER header = _read_HEADER();
    const XMetadataAppender appendMetadata(this, &listResult);

    appendMetadata(offsetof(HEADER, nVersion), sizeof(header.nVersion), XMETADATA_ID_FILE_VERSION, tr("Version"), getVersion());
    appendMetadata(offsetof(HEADER, nEntryPointCount), sizeof(header.nEntryPointCount), XMETADATA_ID_UNKNOWN, tr("Entry-point count"), header.nEntryPointCount);
    appendMetadata(offsetof(HEADER, nInterfaceCount), sizeof(header.nInterfaceCount), XMETADATA_ID_UNKNOWN, tr("Interface entry count"), header.nInterfaceCount);
    appendMetadata(offsetof(HEADER, nFooterOffset), sizeof(header.nFooterOffset), XMETADATA_ID_UNKNOWN, tr("Footer offset"), header.nFooterOffset);
    appendMetadata(offsetof(HEADER, moduleMagic), sizeof(header.moduleMagic), XMETADATA_ID_INTERNAL_NAME, tr("Module type"), QStringLiteral("PMA"));

    return listResult;
}

QList<XBinary::FPART> XMUS::getFileParts(quint32 nFileParts, qint32 nLimit, PDSTRUCT *pPdStruct)
{
    QList<FPART> listResult;

    if ((nLimit < -1) || (nLimit == 0) || !isValid(pPdStruct)) {
        return listResult;
    }

    const HEADER header = _read_HEADER();
    const qint64 nEntryPointSize = (qint64)header.nEntryPointCount * sizeof(quint16);
    const qint64 nDriverOffset = kEntryPointOffset + nEntryPointSize;
    const qint64 nLogicalSize = (qint64)header.nFooterOffset + kFooterSize;

    appendFilePart(&listResult, nFileParts, FILEPART_HEADER, 0, kHeaderSize, tr("Header"), nLimit);
    appendFilePart(&listResult, nFileParts, FILEPART_TABLE, kEntryPointOffset, nEntryPointSize, tr("Entry-point table"), nLimit);
    appendFilePart(&listResult, nFileParts, FILEPART_REGION, nDriverOffset, header.nFooterOffset - nDriverOffset, tr("Driver image"), nLimit);
    appendFilePart(&listResult, nFileParts, FILEPART_FOOTER, header.nFooterOffset, kFooterSize, tr("NS footer"), nLimit);
    appendFilePart(&listResult, nFileParts, FILEPART_OVERLAY, nLogicalSize, getSize() - nLogicalSize, tr("Zero padding"), nLimit);
    appendFilePart(&listResult, nFileParts, FILEPART_DATA, 0, nLogicalSize, tr("Data"), nLimit);

    return listResult;
}

QList<QString> XMUS::getSearchSignatures()
{
    return {QStringLiteral("'MUS'00")};
}

XBinary *XMUS::createInstance(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    Q_UNUSED(bIsImage)
    Q_UNUSED(nModuleAddress)
    return new XMUS(pDevice);
}

XMUS::HEADER XMUS::_read_HEADER(qint64 nOffset)
{
    HEADER result = {};
    read_array(nOffset + offsetof(HEADER, magic), result.magic, sizeof(result.magic));
    result.nVersion = read_uint16(nOffset + offsetof(HEADER, nVersion), false);
    result.nEntryPointCount = read_uint16(nOffset + offsetof(HEADER, nEntryPointCount), false);
    result.nInterfaceCount = read_uint16(nOffset + offsetof(HEADER, nInterfaceCount), false);
    result.nFooterOffset = read_uint16(nOffset + offsetof(HEADER, nFooterOffset), false);
    read_array(nOffset + offsetof(HEADER, moduleMagic), result.moduleMagic, sizeof(result.moduleMagic));
    for (quint32 i = 0; i < 4; i++) {
        result.nReserved[i] = read_uint16(nOffset + offsetof(HEADER, nReserved) + (qint64)i * sizeof(quint16), false);
    }
    return result;
}

bool XMUS::handleInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XMUS> guardedThis(this);
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

void *XMUS::getInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XMUS> guardedThis(this);
    const bool bHandled = guardedThis->handleInternalInfo(pPdStruct);
    if (!guardedThis || !bHandled) return nullptr;
    return &guardedThis->m_internalInfo;
}

void XMUS::setInternalInfo(void *pInternalInfo)
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
