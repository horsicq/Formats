/* Copyright (c) 2017-2026 hors<horsicq@gmail.com>
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
#include "xmachofat.h"
#include "xdecompress.h"

namespace {
const quint32 MACHOFAT_MAX_ARCHITECTURES = 1000000;
}

static XBinary::XCONVERT _TABLE_XMACHOFAT_STRUCTID[] = {
    {XMACHOFat::STRUCTID_UNKNOWN, "Unknown", QObject::tr("Unknown")},
    {XMACHOFat::STRUCTID_HEADER, "HEADER", QObject::tr("Header")},
    {XMACHOFat::STRUCTID_ARCHITECTURE, "ARCHITECTURE", QObject::tr("Architecture")},
};

static XBinary::XIDSTRING _TABLE_XMACHOFAT_HeaderMagics[] = {
    {XMACH_DEF::S_FAT_MAGIC, "FAT_MAGIC"},
    {XMACH_DEF::S_FAT_CIGAM, "FAT_CIGAM"},
    {XMACH_DEF::S_FAT_MAGIC_64, "FAT_MAGIC_64"},
    {XMACH_DEF::S_FAT_CIGAM_64, "FAT_CIGAM_64"},
};

XMACHOFat::XMACHOFat(QIODevice *pDevice) : XArchive(pDevice)
{
}

bool XMACHOFat::isValid(PDSTRUCT *pPdStruct)
{
    return XBinary::isPdStructNotCanceled(pPdStruct) && (_getValidatedArchitectureCount(pPdStruct, true) != 0);
}

bool XMACHOFat::isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    XMACHOFat xmachofat(pDevice);

    return xmachofat.isValid(pPdStruct);
}

XBinary::ENDIAN XMACHOFat::getEndian()
{
    bool bIs64 = false;
    bool bIsBigEndian = false;

    if (!_getFatFormat(&bIs64, &bIsBigEndian)) return ENDIAN_UNKNOWN;

    Q_UNUSED(bIs64)
    return bIsBigEndian ? ENDIAN_BIG : ENDIAN_LITTLE;
}

quint64 XMACHOFat::getNumberOfRecords(PDSTRUCT *pPdStruct)
{
    return _getValidatedArchitectureCount(pPdStruct, true);
}

XBinary::OSNAME XMACHOFat::getOsName()
{
    return OSNAME_MACOS;
}

QList<XArchive::RECORD> XMACHOFat::getRecords(qint32 nLimit, PDSTRUCT *pPdStruct)
{
    QList<RECORD> listResult;

    if ((nLimit < -1) || (nLimit == 0)) {
        return listResult;
    }

    qint32 nNumberOfRecords = (qint32)getNumberOfRecords(pPdStruct);

    if (nLimit != -1) {
        nNumberOfRecords = qMin(nNumberOfRecords, nLimit);
    }

    if (nNumberOfRecords < 0) {
        return listResult;  // Invalid number of records
    }

    const qint64 nArchitectureRecordSize = _getArchitectureRecordSize();

    for (qint32 i = 0; (i < nNumberOfRecords) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        ARCHITECTURE_RECORD architecture = {};
        if (!_readArchitectureRecord(i, &architecture) || !_isArchitectureRangeValid(architecture)) break;

        const qint64 nOffset = sizeof(XMACH_DEF::fat_header) + (qint64)i * nArchitectureRecordSize;

        RECORD record = {};

        record.spInfo.sRecordName = XMACH::_getArch(architecture.cputype, architecture.cpusubtype);
        record.spInfo.nUncompressedSize = (qint64)architecture.size;
        record.spInfo.compressMethod = HANDLE_METHOD_STORE;
        record.nHeaderOffset = nOffset;
        record.nHeaderSize = nArchitectureRecordSize;
        record.nDataOffset = (qint64)architecture.offset;
        record.nDataSize = (qint64)architecture.size;

        listResult.append(record);
    }

    return listResult;
}

QString XMACHOFat::getFileFormatExt()
{
    return "";
}

QString XMACHOFat::getFileFormatExtsString()
{
    return "Universal Mach-O (fat) (*.fat)";
}

qint64 XMACHOFat::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    return _calculateRawSize(pPdStruct);
}

QList<XBinary::MAPMODE> XMACHOFat::getMapModesList()
{
    QList<MAPMODE> listResult;

    listResult.append(MAPMODE_REGIONS);

    return listResult;
}

XBinary::_MEMORY_MAP XMACHOFat::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    XBinary::_MEMORY_MAP result = {};

    if (mapMode == MAPMODE_UNKNOWN) {
        mapMode = MAPMODE_REGIONS;  // Default mode
    }

    if (mapMode == MAPMODE_REGIONS) {
        XBinary::PDSTRUCT pdStructEmpty = {};

        if (!pPdStruct) {
            pdStructEmpty = XBinary::createPdStruct();
            pPdStruct = &pdStructEmpty;
        }

        result.endian = getEndian();
        result.nBinarySize = getSize();

        qint32 nIndex = 0;

        {
            _MEMORY_RECORD record = {};

            record.nIndex = nIndex++;
            record.filePart = FILEPART_HEADER;
            record.nOffset = 0;
            record.nSize = sizeof(XMACH_DEF::fat_header);
            record.nAddress = XADDR_MAX;
            record.sName = tr("Header");

            result.listRecords.append(record);
        }

        const quint32 nNumberOfRecords = _getValidatedArchitectureCount(pPdStruct, true);

        for (quint32 i = 0; (i < nNumberOfRecords) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
            _MEMORY_RECORD record = {};

            ARCHITECTURE_RECORD architecture = {};
            if (!_readArchitectureRecord((qint32)i, &architecture)) break;

            record.sName = XMACH::_getArch(architecture.cputype, architecture.cpusubtype);
            record.nOffset = (qint64)architecture.offset;
            record.nSize = (qint64)architecture.size;
            record.nAddress = XADDR_MAX;
            record.filePart = FILEPART_SEGMENT;

            result.listRecords.append(record);
        }

        _handleOverlay(&result);
    }

    return result;
}

QString XMACHOFat::getArch()
{
    QStringList listArchs;

    if (getSize() >= sizeof(XMACH_DEF::fat_header)) {
        XBinary::PDSTRUCT pdStruct = XBinary::createPdStruct();
        const quint32 nNumberOfRecords = _getValidatedArchitectureCount(&pdStruct, true);

        for (quint32 i = 0; i < nNumberOfRecords; i++) {
            ARCHITECTURE_RECORD architecture = {};
            if (!_readArchitectureRecord((qint32)i, &architecture)) break;

            QString sArch = XMACH::_getArch(architecture.cputype, architecture.cpusubtype);
            if (!sArch.isEmpty()) listArchs.append(sArch);
        }
    }

    if (listArchs.isEmpty()) return tr("Universal");

    listArchs.removeDuplicates();
    return listArchs.join(", ");
}

qint32 XMACHOFat::getType()
{
    return TYPE_BUNDLE;
}

QString XMACHOFat::typeIdToString(qint32 nType)
{
    QString sResult = tr("Unknown");

    switch (nType) {
        case TYPE_BUNDLE: sResult = tr("Bundle");
    }

    return sResult;
}

QString XMACHOFat::getMIMEString()
{
    return "application/x-mach-binary";
    ;
}

XBinary::FT XMACHOFat::getFileType()
{
    return FT_MACHOFAT;
}

bool XMACHOFat::isArchive()
{
    return true;
}

QString XMACHOFat::structIDToString(quint32 nID)
{
    return XBinary::XCONVERT_idToTransString(nID, _TABLE_XMACHOFAT_STRUCTID, sizeof(_TABLE_XMACHOFAT_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QString XMACHOFat::structIDToFtString(quint32 nID)
{
    return XBinary::XCONVERT_idToFtString(nID, _TABLE_XMACHOFAT_STRUCTID, sizeof(_TABLE_XMACHOFAT_STRUCTID) / sizeof(XBinary::XCONVERT));
}

quint32 XMACHOFat::ftStringToStructID(const QString &sFtString)
{
    return XCONVERT_ftStringToId(sFtString, _TABLE_XMACHOFAT_STRUCTID, sizeof(_TABLE_XMACHOFAT_STRUCTID) / sizeof(XBinary::XCONVERT));
}

// qint64 XMACHOFat::getNumberOfArchiveRecords(PDSTRUCT *pPdStruct)
// {
//     return (qint64)getNumberOfRecords(pPdStruct);
// }

bool XMACHOFat::_getFatFormat(bool *pbIs64, bool *pbIsBigEndian)
{
    if (!pbIs64 || !pbIsBigEndian || (getSize() < (qint64)sizeof(XMACH_DEF::fat_header))) return false;

    const quint32 nMagic = read_uint32(0);

    if (nMagic == XMACH_DEF::S_FAT_MAGIC) {
        *pbIs64 = false;
        *pbIsBigEndian = false;
    } else if (nMagic == XMACH_DEF::S_FAT_CIGAM) {
        *pbIs64 = false;
        *pbIsBigEndian = true;
    } else if (nMagic == XMACH_DEF::S_FAT_MAGIC_64) {
        *pbIs64 = true;
        *pbIsBigEndian = false;
    } else if (nMagic == XMACH_DEF::S_FAT_CIGAM_64) {
        *pbIs64 = true;
        *pbIsBigEndian = true;
    } else {
        return false;
    }

    return true;
}

qint64 XMACHOFat::_getArchitectureRecordSize()
{
    bool bIs64 = false;
    bool bIsBigEndian = false;
    if (!_getFatFormat(&bIs64, &bIsBigEndian)) return 0;

    Q_UNUSED(bIsBigEndian)
    return bIs64 ? (qint64)sizeof(XMACH_DEF::fat_arch_64) : (qint64)sizeof(XMACH_DEF::fat_arch);
}

bool XMACHOFat::_readArchitectureRecord(qint32 nIndex, ARCHITECTURE_RECORD *pRecord)
{
    if (!pRecord || (nIndex < 0)) return false;

    bool bIs64 = false;
    bool bIsBigEndian = false;
    if (!_getFatFormat(&bIs64, &bIsBigEndian)) return false;

    const qint64 nRecordSize = bIs64 ? (qint64)sizeof(XMACH_DEF::fat_arch_64) : (qint64)sizeof(XMACH_DEF::fat_arch);
    const qint64 nFileSize = getSize();
    const qint64 nTableSpace = nFileSize - (qint64)sizeof(XMACH_DEF::fat_header);
    if ((nTableSpace < nRecordSize) || ((qint64)nIndex >= (nTableSpace / nRecordSize))) return false;

    const qint64 nOffset = sizeof(XMACH_DEF::fat_header) + (qint64)nIndex * nRecordSize;
    ARCHITECTURE_RECORD record = {};

    if (bIs64) {
        record.cputype = read_uint32(nOffset + offsetof(XMACH_DEF::fat_arch_64, cputype), bIsBigEndian);
        record.cpusubtype = read_uint32(nOffset + offsetof(XMACH_DEF::fat_arch_64, cpusubtype), bIsBigEndian);
        record.offset = read_uint64(nOffset + offsetof(XMACH_DEF::fat_arch_64, offset), bIsBigEndian);
        record.size = read_uint64(nOffset + offsetof(XMACH_DEF::fat_arch_64, size), bIsBigEndian);
        record.align = read_uint32(nOffset + offsetof(XMACH_DEF::fat_arch_64, align), bIsBigEndian);
        record.reserved = read_uint32(nOffset + offsetof(XMACH_DEF::fat_arch_64, reserved), bIsBigEndian);
    } else {
        record.cputype = read_uint32(nOffset + offsetof(XMACH_DEF::fat_arch, cputype), bIsBigEndian);
        record.cpusubtype = read_uint32(nOffset + offsetof(XMACH_DEF::fat_arch, cpusubtype), bIsBigEndian);
        record.offset = read_uint32(nOffset + offsetof(XMACH_DEF::fat_arch, offset), bIsBigEndian);
        record.size = read_uint32(nOffset + offsetof(XMACH_DEF::fat_arch, size), bIsBigEndian);
        record.align = read_uint32(nOffset + offsetof(XMACH_DEF::fat_arch, align), bIsBigEndian);
    }

    *pRecord = record;
    return true;
}

bool XMACHOFat::_isArchitectureRangeValid(const ARCHITECTURE_RECORD &record)
{
    const qint64 nFileSize = getSize();
    bool bIs64 = false;
    bool bIsBigEndian = false;
    if ((nFileSize < 0) || (record.cputype == 0) || (record.size == 0) || (record.align > 63) || !_getFatFormat(&bIs64, &bIsBigEndian) ||
        (bIs64 && (record.reserved != 0)))
        return false;

    const quint32 nNumberOfRecords = read_uint32(offsetof(XMACH_DEF::fat_header, nfat_arch), bIsBigEndian);
    const quint64 nRecordSize = bIs64 ? sizeof(XMACH_DEF::fat_arch_64) : sizeof(XMACH_DEF::fat_arch);
    const quint64 nTableEnd = sizeof(XMACH_DEF::fat_header) + (quint64)nNumberOfRecords * nRecordSize;
    const quint64 nAlignmentMask = record.align ? (((quint64)1 << record.align) - 1) : 0;

    const quint64 nUnsignedFileSize = (quint64)nFileSize;
    return (record.offset >= nTableEnd) && ((record.offset & nAlignmentMask) == 0) && (record.offset <= nUnsignedFileSize) &&
           (record.size <= (nUnsignedFileSize - record.offset));
}

quint32 XMACHOFat::_getValidatedArchitectureCount(PDSTRUCT *pPdStruct, bool bValidateRanges)
{
    if (!XBinary::isPdStructNotCanceled(pPdStruct)) return 0;

    bool bIs64 = false;
    bool bIsBigEndian = false;
    if (!_getFatFormat(&bIs64, &bIsBigEndian)) return 0;

    const qint64 nFileSize = getSize();
    const qint64 nRecordSize = bIs64 ? (qint64)sizeof(XMACH_DEF::fat_arch_64) : (qint64)sizeof(XMACH_DEF::fat_arch);
    const quint32 nNumberOfRecords = read_uint32(offsetof(XMACH_DEF::fat_header, nfat_arch), bIsBigEndian);

    if ((nNumberOfRecords == 0) || (nNumberOfRecords > MACHOFAT_MAX_ARCHITECTURES) || (nFileSize < (qint64)sizeof(XMACH_DEF::fat_header)) ||
        ((quint64)nNumberOfRecords > (quint64)((nFileSize - (qint64)sizeof(XMACH_DEF::fat_header)) / nRecordSize))) {
        return 0;
    }

    if (bValidateRanges) {
        for (quint32 i = 0; i < nNumberOfRecords; i++) {
            if (!XBinary::isPdStructNotCanceled(pPdStruct)) return 0;

            ARCHITECTURE_RECORD record = {};
            if (!_readArchitectureRecord((qint32)i, &record) || !_isArchitectureRangeValid(record)) return 0;
        }
    }

    return nNumberOfRecords;
}

XMACH_DEF::fat_header XMACHOFat::read_fat_header()
{
    XMACH_DEF::fat_header result = {};

    if (getSize() >= sizeof(XMACH_DEF::fat_header)) {
        bool bIsBigEndian = isBigEndian();

        result.magic = read_uint32(offsetof(XMACH_DEF::fat_header, magic), bIsBigEndian);
        result.nfat_arch = read_uint32(offsetof(XMACH_DEF::fat_header, nfat_arch), bIsBigEndian);
    }

    return result;
}

XMACH_DEF::fat_arch XMACHOFat::read_fat_arch(qint32 nIndex)
{
    XMACH_DEF::fat_arch result = {};

    bool bIs64 = false;
    bool bIsBigEndian = false;
    if (!_getFatFormat(&bIs64, &bIsBigEndian) || bIs64) return result;

    ARCHITECTURE_RECORD record = {};
    if (_readArchitectureRecord(nIndex, &record)) {
        result.cputype = record.cputype;
        result.cpusubtype = record.cpusubtype;
        result.offset = (quint32)record.offset;
        result.size = (quint32)record.size;
        result.align = record.align;
    }

    return result;
}

QList<XMACH_DEF::fat_arch> XMACHOFat::read_fat_arch_list(PDSTRUCT *pPdStruct)
{
    QList<XMACH_DEF::fat_arch> listResult;

    bool bIs64 = false;
    bool bIsBigEndian = false;
    if (!_getFatFormat(&bIs64, &bIsBigEndian) || bIs64) return listResult;

    qint32 nNumberOfRecords = (qint32)getNumberOfRecords(pPdStruct);

    if (nNumberOfRecords > 0) {
        for (qint32 i = 0; (i < nNumberOfRecords) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
            XMACH_DEF::fat_arch fatArch = read_fat_arch(i);
            listResult.append(fatArch);
        }
    }

    return listResult;
}

XMACH_DEF::fat_arch_64 XMACHOFat::read_fat_arch_64(qint32 nIndex)
{
    XMACH_DEF::fat_arch_64 result = {};

    bool bIs64 = false;
    bool bIsBigEndian = false;
    if (!_getFatFormat(&bIs64, &bIsBigEndian) || !bIs64) return result;

    ARCHITECTURE_RECORD record = {};
    if (_readArchitectureRecord(nIndex, &record)) {
        result.cputype = record.cputype;
        result.cpusubtype = record.cpusubtype;
        result.offset = record.offset;
        result.size = record.size;
        result.align = record.align;
        result.reserved = record.reserved;
    }

    return result;
}

QList<XMACH_DEF::fat_arch_64> XMACHOFat::read_fat_arch_64_list(PDSTRUCT *pPdStruct)
{
    QList<XMACH_DEF::fat_arch_64> listResult;

    bool bIs64 = false;
    bool bIsBigEndian = false;
    if (!_getFatFormat(&bIs64, &bIsBigEndian) || !bIs64) return listResult;

    const qint32 nNumberOfRecords = (qint32)getNumberOfRecords(pPdStruct);
    for (qint32 i = 0; (i < nNumberOfRecords) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        listResult.append(read_fat_arch_64(i));
    }

    return listResult;
}

QMap<quint64, QString> XMACHOFat::getHeaderMagics()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(XMACH_DEF::S_FAT_MAGIC, "FAT_MAGIC");
    mapResult.insert(XMACH_DEF::S_FAT_CIGAM, "FAT_CIGAM");
    mapResult.insert(XMACH_DEF::S_FAT_MAGIC_64, "FAT_MAGIC_64");
    mapResult.insert(XMACH_DEF::S_FAT_CIGAM_64, "FAT_CIGAM_64");

    return mapResult;
}

QMap<quint64, QString> XMACHOFat::getHeaderMagicsS()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(XMACH_DEF::S_FAT_MAGIC, "Universal Mach-O (Big Endian)");
    mapResult.insert(XMACH_DEF::S_FAT_CIGAM, "Universal Mach-O (Little Endian)");
    mapResult.insert(XMACH_DEF::S_FAT_MAGIC_64, "Universal Mach-O FAT64 (Big Endian)");
    mapResult.insert(XMACH_DEF::S_FAT_CIGAM_64, "Universal Mach-O FAT64 (Little Endian)");

    return mapResult;
}

QString XMACHOFat::getArchitectureString(qint32 nIndex)
{
    QString sResult;

    ARCHITECTURE_RECORD architecture = {};

    if (_readArchitectureRecord(nIndex, &architecture) && (architecture.cputype != 0)) {
        sResult = XMACH::_getArch(architecture.cputype, architecture.cpusubtype);
    }

    return sResult;
}

qint64 XMACHOFat::getArchitectureOffset(qint32 nIndex)
{
    qint64 nResult = -1;

    ARCHITECTURE_RECORD architecture = {};

    if (_readArchitectureRecord(nIndex, &architecture) && _isArchitectureRangeValid(architecture)) {
        nResult = (qint64)architecture.offset;
    }

    return nResult;
}

qint64 XMACHOFat::getArchitectureSize(qint32 nIndex)
{
    qint64 nResult = 0;

    ARCHITECTURE_RECORD architecture = {};

    if (_readArchitectureRecord(nIndex, &architecture) && _isArchitectureRangeValid(architecture)) {
        nResult = (qint64)architecture.size;
    }

    return nResult;
}

bool XMACHOFat::isArchitectureValid(qint32 nIndex)
{
    ARCHITECTURE_RECORD architecture = {};
    return _readArchitectureRecord(nIndex, &architecture) && _isArchitectureRangeValid(architecture);
}

QList<XBinary::XFHEADER> XMACHOFat::getXFHeaders(const XFSTRUCT &xfStruct, PDSTRUCT *pPdStruct)
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

        qint64 nHeaderOffset = locToOffset(xfStruct.pMemoryMap, headerLoc);
        if ((nHeaderOffset != -1) && isOffsetAndSizeValid(xfStruct.pMemoryMap, nHeaderOffset, sizeof(XMACH_DEF::fat_header))) {
            XFHEADER xfHeader = {};
            xfHeader.sParentTag = xfStruct.sParent;
            xfHeader.fileType = xfStruct.fileType;
            xfHeader.structID = static_cast<XBinary::STRUCTID>(STRUCTID_HEADER);
            xfHeader.xLoc = headerLoc;
            xfHeader.nSize = sizeof(XMACH_DEF::fat_header);
            xfHeader.xfType = XFTYPE_HEADER;
            xfHeader.listFields = getXFRecords(xfStruct.fileType, STRUCTID_HEADER, headerLoc);
            xfHeader.listDataSt.append(
                {0, 0, XFDATASTYPE_LIST, _TABLE_XMACHOFAT_HeaderMagics, (qint32)(sizeof(_TABLE_XMACHOFAT_HeaderMagics) / sizeof(XBinary::XIDSTRING))});
            xfHeader.sTag = xfHeaderToTag(xfHeader, structIDToString(STRUCTID_HEADER), xfHeader.sParentTag);
            listResult.append(xfHeader);
        }

        if (xfStruct.bIsParent) {
            XFSTRUCT _xfStructArch = xfStruct;
            _xfStructArch.nStructID = STRUCTID_ARCHITECTURE;
            _xfStructArch.xLoc = offsetToLoc(sizeof(XMACH_DEF::fat_header));
            _xfStructArch.nCount = (qint32)getNumberOfRecords(pPdStruct);
            listResult.append(getXFHeaders(_xfStructArch, pPdStruct));
        }
    } else if (nStructID == STRUCTID_ARCHITECTURE) {
        qint32 nCount = xfStruct.nCount;
        const qint32 nValidatedCount = (qint32)getNumberOfRecords(pPdStruct);

        if ((nCount <= 0) || (nCount > nValidatedCount)) nCount = nValidatedCount;

        if (nCount > 0) {
            XLOC firstLoc = offsetToLoc(sizeof(XMACH_DEF::fat_header));
            const qint64 nArchitectureRecordSize = _getArchitectureRecordSize();

            qint32 nCpuTypesSize = 0;
            XBinary::XIDSTRING *pCpuTypes = XMACH::getHeaderCpuTypesTablePtr(&nCpuTypesSize);

            XFHEADER xfHeader = {};
            xfHeader.sParentTag = xfStruct.sParent;
            xfHeader.fileType = xfStruct.fileType;
            xfHeader.structID = static_cast<XBinary::STRUCTID>(STRUCTID_ARCHITECTURE);
            xfHeader.xLoc = firstLoc;
            xfHeader.nSize = nArchitectureRecordSize;
            xfHeader.xfType = XFTYPE_TABLE;
            xfHeader.listFields = getXFRecords(xfStruct.fileType, STRUCTID_ARCHITECTURE, firstLoc);
            xfHeader.listDataSt.append({0, 0, XFDATASTYPE_LIST, pCpuTypes, nCpuTypesSize});
            // xfHeader.listDataSt.append({1, 0, XFDATASTYPE_LIST, pCpuTypes, nCpuTypesSize});

            for (qint32 i = 0; (i < nCount) && isPdStructNotCanceled(pPdStruct); i++) {
                qint64 nArchOffset = sizeof(XMACH_DEF::fat_header) + (qint64)i * nArchitectureRecordSize;
                xfHeader.listRowLocations.append((XADDR)nArchOffset);
                // xfHeader.listRowNames.append(getArchitectureString(i));
            }

            xfHeader.sTag = xfHeaderToTag(xfHeader, structIDToString(STRUCTID_ARCHITECTURE), xfHeader.sParentTag);
            listResult.append(xfHeader);
        }
    }

    return listResult;
}

QList<XBinary::XFRECORD> XMACHOFat::getXFRecords(FT fileType, quint32 nStructID, const XLOC &xLoc)
{
    Q_UNUSED(fileType)
    Q_UNUSED(xLoc)

    QList<XBinary::XFRECORD> listResult;

    if (nStructID == STRUCTID_HEADER) {
        listResult.append({"magic", (qint32)offsetof(XMACH_DEF::fat_header, magic), 4, XFRECORD_FLAG_BE, VT_UINT32});
        listResult.append({"nfat_arch", (qint32)offsetof(XMACH_DEF::fat_header, nfat_arch), 4, XFRECORD_FLAG_COUNT, VT_UINT32});
    } else if (nStructID == STRUCTID_ARCHITECTURE) {
        bool bIs64 = false;
        bool bIsBigEndian = false;
        if (!_getFatFormat(&bIs64, &bIsBigEndian)) return listResult;

        Q_UNUSED(bIsBigEndian)
        if (bIs64) {
            listResult.append({"cputype", (qint32)offsetof(XMACH_DEF::fat_arch_64, cputype), 4, XFRECORD_FLAG_NONE, VT_UINT32});
            listResult.append({"cpusubtype", (qint32)offsetof(XMACH_DEF::fat_arch_64, cpusubtype), 4, XFRECORD_FLAG_NONE, VT_UINT32});
            listResult.append({"offset", (qint32)offsetof(XMACH_DEF::fat_arch_64, offset), 8, XFRECORD_FLAG_OFFSET, VT_UINT64});
            listResult.append({"size", (qint32)offsetof(XMACH_DEF::fat_arch_64, size), 8, XFRECORD_FLAG_SIZE, VT_UINT64});
            listResult.append({"align", (qint32)offsetof(XMACH_DEF::fat_arch_64, align), 4, XFRECORD_FLAG_NONE, VT_UINT32});
            listResult.append({"reserved", (qint32)offsetof(XMACH_DEF::fat_arch_64, reserved), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        } else {
            listResult.append({"cputype", (qint32)offsetof(XMACH_DEF::fat_arch, cputype), 4, XFRECORD_FLAG_NONE, VT_UINT32});
            listResult.append({"cpusubtype", (qint32)offsetof(XMACH_DEF::fat_arch, cpusubtype), 4, XFRECORD_FLAG_NONE, VT_UINT32});
            listResult.append({"offset", (qint32)offsetof(XMACH_DEF::fat_arch, offset), 4, XFRECORD_FLAG_OFFSET, VT_UINT32});
            listResult.append({"size", (qint32)offsetof(XMACH_DEF::fat_arch, size), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
            listResult.append({"align", (qint32)offsetof(XMACH_DEF::fat_arch, align), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        }
    }

    return listResult;
}

QMap<XBinary::UNPACK_PROP, QVariant> XMACHOFat::getDefaultUnpackProperties()
{
    QMap<XBinary::UNPACK_PROP, QVariant> result = XArchive::getDefaultUnpackProperties();

    return result;
}

bool XMACHOFat::initUnpack(UNPACK_STATE *pState, const QMap<UNPACK_PROP, QVariant> &mapProperties, PDSTRUCT *pPdStruct)
{
    if (m_bUnpackOperationInProgress) {
        return false;
    }
    UNPACK_OPERATION_GUARD operationGuard(&m_bUnpackOperationInProgress);
    if (!operationGuard.isAcquired()) return false;
    QPointer<XMACHOFat> guardedArchive(this);

    if (!pState) {
        return false;
    }

    if ((pState->pContext || !pState->baUnpackSourceToken.isEmpty()) && !guardedArchive->ownsUnpackSource(pState)) return false;
    guardedArchive->releaseUnpackSource(pState);
    *pState = UNPACK_STATE();
    if (!XBinary::isPdStructNotCanceled(pPdStruct)) return false;
    const bool bBound = guardedArchive->bindUnpackSource(pState, pPdStruct);
    if (!guardedArchive || !bBound) return false;

    pState->nCurrentOffset = 0;
    pState->nCurrentIndex = 0;
    pState->nNumberOfRecords = (qint32)guardedArchive->getNumberOfRecords(pPdStruct);
    if (!guardedArchive) {
        *pState = UNPACK_STATE();
        return false;
    }
    pState->nTotalSize = guardedArchive->getSize();
    if (!guardedArchive) {
        *pState = UNPACK_STATE();
        return false;
    }
    pState->mapUnpackProperties = mapProperties;
    pState->pContext = nullptr;

    if ((pState->nNumberOfRecords <= 0) || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        guardedArchive->releaseUnpackSource(pState);
        *pState = UNPACK_STATE();
        return false;
    }

    const bool bFinalized = guardedArchive->validateAndFinalizeUnpackSource(pState, pPdStruct);
    if (!guardedArchive) {
        *pState = UNPACK_STATE();
        return false;
    }
    if (!bFinalized) {
        guardedArchive->releaseUnpackSource(pState);
        *pState = UNPACK_STATE();
        return false;
    }

    return true;
}

XBinary::ARCHIVERECORD XMACHOFat::infoCurrent(UNPACK_STATE *pState, PDSTRUCT *pPdStruct)
{
    UNPACK_OPERATION_GUARD operationGuard(&m_bUnpackOperationInProgress, &m_bNestedUnpackInfoAuthorized);
    if (!operationGuard.isAllowed()) return XBinary::ARCHIVERECORD();
    QPointer<XMACHOFat> guardedArchive(this);

    ARCHIVERECORD result = {};

    if (!pState || !guardedArchive->isUnpackSourceCurrent(pState, pPdStruct) || !guardedArchive || (pState->nCurrentIndex < 0) ||
        (pState->nCurrentIndex >= pState->nNumberOfRecords)) {
        return result;
    }

    ARCHITECTURE_RECORD architecture = {};
    if (!guardedArchive->_readArchitectureRecord(pState->nCurrentIndex, &architecture) || !guardedArchive || !guardedArchive->_isArchitectureRangeValid(architecture)) {
        return XBinary::ARCHIVERECORD();
    }

    QString sArchName = XMACH::_getArch(architecture.cputype, architecture.cpusubtype);

    result.nStreamOffset = (qint64)architecture.offset;
    result.nStreamSize = (qint64)architecture.size;
    result.mapProperties.insert(FPART_PROP_ORIGINALNAME, sArchName);
    result.mapProperties.insert(FPART_PROP_UNCOMPRESSEDSIZE, (qint64)architecture.size);
    result.mapProperties.insert(FPART_PROP_HANDLEMETHOD, (quint32)HANDLE_METHOD_STORE);

    return result;
}

bool XMACHOFat::moveToNext(UNPACK_STATE *pState, PDSTRUCT *pPdStruct)
{
    UNPACK_OPERATION_GUARD operationGuard(&m_bUnpackOperationInProgress);
    if (!operationGuard.isAcquired()) return false;
    QPointer<XMACHOFat> guardedArchive(this);

    if (!pState || !guardedArchive->isUnpackSourceCurrent(pState, pPdStruct) || !guardedArchive || (pState->nCurrentIndex < 0) ||
        (pState->nCurrentIndex >= pState->nNumberOfRecords)) {
        return false;
    }

    pState->nCurrentIndex++;

    return (pState->nCurrentIndex < pState->nNumberOfRecords);
}

bool XMACHOFat::finishUnpack(UNPACK_STATE *pState, PDSTRUCT *pPdStruct)
{
    UNPACK_OPERATION_GUARD operationGuard(&m_bUnpackOperationInProgress);
    if (!operationGuard.isAcquired()) return false;

    Q_UNUSED(pPdStruct)

    if (!pState) {
        return false;
    }

    if ((pState->pContext || !pState->baUnpackSourceToken.isEmpty()) && !ownsUnpackSource(pState)) return false;
    releaseUnpackSource(pState);
    *pState = UNPACK_STATE();

    return true;
}

QList<QString> XMACHOFat::getSearchSignatures()
{
    QList<QString> listResult;

    listResult.append("CAFEBABE");
    listResult.append("BEBAFECA");
    listResult.append("CAFEBABF");
    listResult.append("BFBAFECA");

    return listResult;
}

XBinary *XMACHOFat::createInstance(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    Q_UNUSED(bIsImage)
    Q_UNUSED(nModuleAddress)

    return new XMACHOFat(pDevice);
}

bool XMACHOFat::handleInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XMACHOFat> guardedThis(this);
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

void *XMACHOFat::getInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XMACHOFat> guardedThis(this);
    const bool bHandled = guardedThis->handleInternalInfo(pPdStruct);
    if (!guardedThis || !bHandled) return nullptr;

    return &guardedThis->m_internalInfo;
}

void XMACHOFat::setInternalInfo(void *pInternalInfo)
{
    if (pInternalInfo) {
        m_internalInfo = *static_cast<INTERNAL_INFO *>(pInternalInfo);
        XArchive::setInternalInfo(static_cast<XArchive::INTERNAL_INFO *>(&m_internalInfo));
    } else {
        m_internalInfo = INTERNAL_INFO();
        XArchive::setInternalInfo(nullptr);
    }
}
