/* Copyright (c) 2020-2026 hors<horsicq@gmail.com>
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
#include "xatarist.h"

namespace {

constexpr qint64 g_nAtariSTHeaderSize = 2 + (6 * sizeof(quint32)) + sizeof(quint16);

}  // namespace

XBinary::XCONVERT _TABLE_XAtariST_STRUCTID[] = {
    {XAtariST::STRUCTID_UNKNOWN, "Unknown", QObject::tr("Unknown")},
    {XAtariST::STRUCTID_HEADER, "HEADER", QString("HEADER")},
};

XAtariST::XAtariST(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress) : XBinary(pDevice, bIsImage, nModuleAddress)
{
}

XAtariST::~XAtariST()
{
}

bool XAtariST::isValid(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    bool bResult = false;

    if (getSize() >= g_nAtariSTHeaderSize) {
        quint16 nMagic = read_uint16(0, true);  // Big-endian

        if (nMagic == XATARIST_DEF::MAGIC) {
            bResult = true;
        }
    }

    return bResult;
}

bool XAtariST::isValid(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress, PDSTRUCT *pPdStruct)
{
    XAtariST xatarist(pDevice, bIsImage, nModuleAddress);

    return xatarist.isValid(pPdStruct);
}

bool XAtariST::isExecutable()
{
    return true;
}

XBinary::MODE XAtariST::getMode(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    XAtariST xatarist(pDevice, bIsImage, nModuleAddress);

    return xatarist.getMode();
}

XATARIST_DEF::HEADER XAtariST::getHeader()
{
    XATARIST_DEF::HEADER result = {};

    result.nMagic = read_uint16(0, true);
    result.nTextSize = read_uint32(2, true);
    result.nDataSize = read_uint32(6, true);
    result.nBssSize = read_uint32(10, true);
    result.nSymbolSize = read_uint32(14, true);
    result.nReserved = read_uint32(18, true);
    result.nFlags = read_uint32(22, true);
    result.nRelocation = read_uint16(26, true);

    return result;
}

QVector<XBinary::XSYMBOL_STRUCT> XAtariST::getSymbolStructs()
{
    QVector<XSYMBOL_STRUCT> listResult;
    const XATARIST_DEF::HEADER header = getHeader();
    const qint64 nSymbolOffset = 28 + (qint64)header.nTextSize + header.nDataSize;
    const qint64 nSymbolSize = qMin<qint64>(header.nSymbolSize, 14LL * 0x20000);
    const qint64 nRecordSize = 14;

    if ((nSymbolSize <= 0) || !checkOffsetSize(nSymbolOffset, nSymbolSize)) {
        return listResult;
    }

    const quint16 SYMBOL_DEFINED = 0x8000;
    const quint16 SYMBOL_GLOBAL = 0x2000;
    const quint16 SYMBOL_EXTERNAL = 0x0800;
    const qint64 nCount = nSymbolSize / nRecordSize;
    listResult.reserve((qint32)nCount);

    for (qint64 i = 0; i < nCount; ++i) {
        const qint64 nOffset = nSymbolOffset + i * nRecordSize;
        QByteArray baName = read_array(nOffset, 8);
        const qint32 nZero = baName.indexOf('\0');
        if (nZero >= 0) {
            baName.truncate(nZero);
        }

        const quint16 nType = read_uint16(nOffset + 8, true);
        const quint32 nValue = read_uint32(nOffset + 10, true);

        XSYMBOL_STRUCT record = {};
        record.nOffset = nOffset;
        record.nSize = nRecordSize;
        record.nAddress = (nType & SYMBOL_EXTERNAL) ? (XADDR)-1 : getModuleAddress() + nValue;
        record.sName = QString::fromLatin1(baName).trimmed();

        if (nType & SYMBOL_EXTERNAL) {
            record.symbolType = SYMBOL_TYPE_IMPORT;
        } else if ((nType & SYMBOL_GLOBAL) && (nType & SYMBOL_DEFINED)) {
            record.symbolType = SYMBOL_TYPE_EXPORT;
        } else {
            record.symbolType = SYMBOL_TYPE_LABEL;
        }

        if (!record.sName.isEmpty()) {
            listResult.append(record);
        }
    }

    return listResult;
}

QVector<XBinary::XIMPORT_STRUCT> XAtariST::getImportStructs()
{
    QVector<XIMPORT_STRUCT> listResult;
    const QVector<XSYMBOL_STRUCT> listSymbols = getSymbolStructs();

    for (qint32 i = 0; i < listSymbols.count(); ++i) {
        const XSYMBOL_STRUCT &symbol = listSymbols.at(i);
        if (symbol.symbolType == SYMBOL_TYPE_IMPORT) {
            XIMPORT_STRUCT record = {};
            record.nOffset = symbol.nOffset;
            record.nSize = symbol.nSize;
            record.nAddress = symbol.nAddress;
            record.sFunction = symbol.sName;
            record.nOrdinal = -1;
            listResult.append(record);
        }
    }

    return listResult;
}

QVector<XBinary::XEXPORT_STRUCT> XAtariST::getExportStructs()
{
    QVector<XEXPORT_STRUCT> listResult;
    const QVector<XSYMBOL_STRUCT> listSymbols = getSymbolStructs();

    for (qint32 i = 0; i < listSymbols.count(); ++i) {
        const XSYMBOL_STRUCT &symbol = listSymbols.at(i);
        if (symbol.symbolType == SYMBOL_TYPE_EXPORT) {
            XEXPORT_STRUCT record = {};
            record.nOffset = symbol.nOffset;
            record.nSize = symbol.nSize;
            record.nAddress = symbol.nAddress;
            record.sFunction = symbol.sName;
            record.nOrdinal = -1;
            listResult.append(record);
        }
    }

    return listResult;
}

QList<XBinary::MAPMODE> XAtariST::getMapModesList()
{
    QList<XBinary::MAPMODE> listResult;

    listResult.append(MAPMODE_REGIONS);

    return listResult;
}

QList<XBinary::FPART> XAtariST::getFileParts(quint32 nFileParts, qint32 nLimit, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    QList<FPART> listResult;

    if ((nLimit < -1) || (nLimit == 0)) {
        return listResult;
    }

    XATARIST_DEF::HEADER header = getHeader();

    qint64 nTotalSize = getSize();
    qint64 nHeaderSize = qMin(g_nAtariSTHeaderSize, qMax((qint64)0, nTotalSize));
    qint64 nCurrentOffset = nHeaderSize;

    if (nFileParts & FILEPART_HEADER) {
        FPART partHeader = {};
        partHeader.filePart = FILEPART_HEADER;
        partHeader.nFileOffset = 0;
        partHeader.nFileSize = nHeaderSize;
        partHeader.nVirtualAddress = (XADDR)-1;
        partHeader.sName = tr("Header");
        listResult.append(partHeader);
        if ((nLimit != -1) && (listResult.count() >= nLimit)) return listResult;
    }

    if (nFileParts & FILEPART_SEGMENT) {
        // Text segment
        if ((header.nTextSize > 0) && (nCurrentOffset < nTotalSize)) {
            FPART partText = {};
            partText.filePart = FILEPART_SEGMENT;
            partText.nFileOffset = nCurrentOffset;
            partText.nFileSize = qMin((qint64)header.nTextSize, nTotalSize - nCurrentOffset);
            partText.nVirtualAddress = getModuleAddress();
            partText.sName = tr("Text");
            listResult.append(partText);
            if ((nLimit != -1) && (listResult.count() >= nLimit)) return listResult;
            nCurrentOffset += partText.nFileSize;
        }

        // Data segment
        if ((header.nDataSize > 0) && (nCurrentOffset < nTotalSize)) {
            FPART partData = {};
            partData.filePart = FILEPART_SEGMENT;
            partData.nFileOffset = nCurrentOffset;
            partData.nFileSize = qMin((qint64)header.nDataSize, nTotalSize - nCurrentOffset);
            partData.nVirtualAddress = getModuleAddress() + header.nTextSize;
            partData.sName = tr("Data");
            listResult.append(partData);
            if ((nLimit != -1) && (listResult.count() >= nLimit)) return listResult;
            nCurrentOffset += partData.nFileSize;
        }

        // BSS segment
        if (header.nBssSize > 0) {
            FPART partBss = {};
            partBss.filePart = FILEPART_SEGMENT;
            partBss.nFileOffset = -1;
            partBss.nFileSize = header.nBssSize;
            partBss.nVirtualAddress = getModuleAddress() + header.nTextSize + header.nDataSize;
            partBss.sName = QString("BSS");
            listResult.append(partBss);
            if ((nLimit != -1) && (listResult.count() >= nLimit)) return listResult;
        }
    }

    if (nFileParts & FILEPART_OVERLAY) {
        qint64 nCoveredEnd = 0;
        qint32 nNumberOfParts = listResult.count();

        for (qint32 i = 0; i < nNumberOfParts; i++) {
            const FPART &part = listResult.at(i);
            qint64 nPartEnd = -1;

            if (part.nFileOffset >= 0) {
                nPartEnd = part.nFileOffset + part.nFileSize;
            }

            if (nPartEnd > nCoveredEnd) {
                nCoveredEnd = nPartEnd;
            }
        }

        if (nCoveredEnd < nTotalSize) {
            FPART partOverlay = {};
            partOverlay.filePart = FILEPART_OVERLAY;
            partOverlay.nFileOffset = nCoveredEnd;
            partOverlay.nFileSize = nTotalSize - nCoveredEnd;
            partOverlay.nVirtualAddress = (XADDR)-1;
            partOverlay.sName = tr("Overlay");
            listResult.append(partOverlay);
            if ((nLimit != -1) && (listResult.count() >= nLimit)) return listResult;
        }
    }

    return listResult;
}

bool XAtariST::_initMemoryMap(XBinary::_MEMORY_MAP *pMemoryMap, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    XATARIST_DEF::HEADER header = getHeader();

    qint64 nTotalSize = getSize();

    pMemoryMap->nModuleAddress = getModuleAddress();
    pMemoryMap->nEntryPointAddress = pMemoryMap->nModuleAddress;
    pMemoryMap->nBinarySize = nTotalSize;
    pMemoryMap->nImageSize = header.nTextSize + header.nDataSize + header.nBssSize;
    pMemoryMap->fileType = getFileType();
    pMemoryMap->mode = getMode();
    pMemoryMap->sArch = getArch();
    pMemoryMap->endian = getEndian();
    pMemoryMap->sType = getTypeAsString();

    return true;
}

XBinary::_MEMORY_MAP XAtariST::getMemoryMap(XBinary::MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    if (mapMode == MAPMODE_UNKNOWN) {
        mapMode = MAPMODE_REGIONS;
    }

    if (mapMode == MAPMODE_REGIONS) {
        return _getMemoryMap(FILEPART_SEGMENT | FILEPART_OVERLAY, pPdStruct);
    } else {
        return _getMemoryMap(FILEPART_SEGMENT | FILEPART_OVERLAY, pPdStruct);
    }
}

QString XAtariST::getArch()
{
    return QString("68K");
}

XBinary::MODE XAtariST::getMode()
{
    return MODE_32;
}

XBinary::ENDIAN XAtariST::getEndian()
{
    return ENDIAN_BIG;
}

qint64 XAtariST::getImageSize()
{
    XATARIST_DEF::HEADER header = getHeader();

    return header.nTextSize + header.nDataSize + header.nBssSize;
}

XBinary::FT XAtariST::getFileType()
{
    return FT_ATARIST;
}

qint32 XAtariST::getType()
{
    return TYPE_EXECUTABLE;
}

XBinary::OSNAME XAtariST::getOsName()
{
    return OSNAME_ATARIST;
}

QString XAtariST::structIDToString(quint32 nID)
{
    return XBinary::XCONVERT_idToTransString(nID, _TABLE_XAtariST_STRUCTID, sizeof(_TABLE_XAtariST_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QString XAtariST::structIDToFtString(quint32 nID)
{
    return XBinary::XCONVERT_idToFtString(nID, _TABLE_XAtariST_STRUCTID, sizeof(_TABLE_XAtariST_STRUCTID) / sizeof(XBinary::XCONVERT));
}

quint32 XAtariST::ftStringToStructID(const QString &sFtString)
{
    return XCONVERT_ftStringToId(sFtString, _TABLE_XAtariST_STRUCTID, sizeof(_TABLE_XAtariST_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QList<XBinary::XFHEADER> XAtariST::getXFHeaders(const XFSTRUCT &xfStruct, PDSTRUCT *pPdStruct)
{
    QList<XBinary::XFHEADER> listResult;

    quint32 nStructID = xfStruct.nStructID;

    if (nStructID == STRUCTID_UNKNOWN) {
        XFSTRUCT _xfStruct = xfStruct;
        _xfStruct.nStructID = STRUCTID_HEADER;
        _xfStruct.xLoc = offsetToLoc(0);
        listResult.append(getXFHeaders(_xfStruct, pPdStruct));
    } else if (nStructID == STRUCTID_HEADER) {
        XLOC headerLoc = xfStruct.xLoc;
        if (headerLoc.locType == LT_UNKNOWN) {
            headerLoc = offsetToLoc(0);
        }

        XFHEADER xfHeader = {};
        xfHeader.sParentTag = xfStruct.sParent;
        xfHeader.fileType = xfStruct.fileType;
        xfHeader.structID = static_cast<XBinary::STRUCTID>(STRUCTID_HEADER);
        xfHeader.xLoc = headerLoc;
        xfHeader.nSize = qMin((qint64)sizeof(XATARIST_DEF::HEADER), getSize());
        xfHeader.xfType = XFTYPE_HEADER;
        xfHeader.listFields = getXFRecords(xfStruct.fileType, STRUCTID_HEADER, headerLoc);
        xfHeader.sTag = xfHeaderToTag(xfHeader, structIDToString(STRUCTID_HEADER), xfHeader.sParentTag);
        listResult.append(xfHeader);
    }

    return listResult;
}

QList<XBinary::XFRECORD> XAtariST::getXFRecords(FT fileType, quint32 nStructID, const XLOC &xLoc)
{
    Q_UNUSED(fileType)
    Q_UNUSED(xLoc)

    QList<XBinary::XFRECORD> listResult;

    // Big-endian; literal offsets match the on-disk 28-byte header
    if (nStructID == STRUCTID_HEADER) {
        listResult.append({"nMagic", 0, 2, XFRECORD_FLAG_BE, VT_UINT16});
        listResult.append({"nTextSize", 2, 4, XFRECORD_FLAG_BE | XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"nDataSize", 6, 4, XFRECORD_FLAG_BE | XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"nBssSize", 10, 4, XFRECORD_FLAG_BE | XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"nSymbolSize", 14, 4, XFRECORD_FLAG_BE | XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"nReserved", 18, 4, XFRECORD_FLAG_BE, VT_UINT32});
        listResult.append({"nFlags", 22, 4, XFRECORD_FLAG_BE, VT_UINT32});
        listResult.append({"nRelocation", 26, 2, XFRECORD_FLAG_BE, VT_UINT16});
    }

    return listResult;
}

QString XAtariST::typeIdToString(qint32 nType)
{
    QString sResult = tr("Unknown");

    switch (nType) {
        case TYPE_UNKNOWN: sResult = tr("Unknown"); break;
        case TYPE_EXECUTABLE: sResult = QString("Executable"); break;
    }

    return sResult;
}

QString XAtariST::getFileFormatExtsString()
{
    return QString("tos,ttp,prg");
}

bool XAtariST::handleInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XAtariST> guardedThis(this);
    bool bResult = true;

    if (!isInternalInfoHandled()) {
        bResult = guardedThis->XBinary::handleInternalInfo(pPdStruct);
        if (!guardedThis || !bResult) return false;

        XBinary::INTERNAL_INFO *pInfo =
            static_cast<XBinary::INTERNAL_INFO *>(
                guardedThis->XBinary::getInternalInfo(pPdStruct));
        if (!guardedThis || !pInfo) return false;

        static_cast<XBinary::INTERNAL_INFO &>(
            guardedThis->m_internalInfo) = *pInfo;
        guardedThis->setIsInternalInfoHandled(true);
    }

    return guardedThis && bResult;
}

void *XAtariST::getInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XAtariST> guardedThis(this);
    const bool bHandled = guardedThis->handleInternalInfo(pPdStruct);
    if (!guardedThis || !bHandled) return nullptr;

    return &guardedThis->m_internalInfo;
}

void XAtariST::setInternalInfo(void *pInternalInfo)
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
