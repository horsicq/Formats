/* Copyright (c) 2017-2026 hors<horsicq@gmail.com>
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
#include "xcliassembly.h"

XBinary::XCONVERT _TABLE_XCLIASSEMBLY_STRUCTID[] = {
    {XCLIAssembly::STRUCTID_UNKNOWN, "Unknown", QObject::tr("Unknown")},
    {XCLIAssembly::STRUCTID_MZDOSHEADER, "MZDOSHEADER", QString("MZ DOS Header")},
    {XCLIAssembly::STRUCTID_NTHEADER, "NTHEADER", QString("NT Header")},
    {XCLIAssembly::STRUCTID_CLRHEADER, "CLRHEADER", QString("CLR Header")},
};

XCLIAssembly::XCLIAssembly(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress) : XBinary(pDevice, bIsImage, nModuleAddress)
{
    g_nNetHeaderOffset = -1;
    g_nNetMetaDataOffset = -1;
    g_bPeParsed = false;
    g_bPeValid = false;
    g_bPeIs64 = false;
    g_nPeCharacteristics = 0;
    g_nCliRva = 0;
    g_nCliSize = 0;
}

bool XCLIAssembly::_parseHeaders()
{
    // Minimal MZ/PE header walk: locate the COM(CLR) descriptor and the section
    // table so the class can work standalone, without XPE::initCLIAssembly.
    if (g_bPeParsed) {
        return g_bPeValid;
    }

    g_bPeParsed = true;

    if (read_uint16(0) != 0x5A4D) {  // 'MZ'
        return false;
    }

    qint64 nPeOffset = read_uint32(0x3C);

    if ((nPeOffset <= 0) || (nPeOffset >= getSize())) {
        return false;
    }

    if (read_uint32(nPeOffset) != 0x00004550) {  // 'PE\0\0'
        return false;
    }

    qint32 nNumberOfSections = read_uint16(nPeOffset + 6);
    quint16 nSizeOfOptionalHeader = read_uint16(nPeOffset + 20);
    g_nPeCharacteristics = read_uint16(nPeOffset + 22);

    qint64 nOptionalHeaderOffset = nPeOffset + 24;
    quint16 nMagic = read_uint16(nOptionalHeaderOffset);

    quint32 nNumberOfRvaAndSizes = 0;
    qint64 nDataDirectoryOffset = -1;

    if (nMagic == 0x10B) {  // PE32
        g_bPeIs64 = false;
        nNumberOfRvaAndSizes = read_uint32(nOptionalHeaderOffset + 92);
        nDataDirectoryOffset = nOptionalHeaderOffset + 96;
    } else if (nMagic == 0x20B) {  // PE32+
        g_bPeIs64 = true;
        nNumberOfRvaAndSizes = read_uint32(nOptionalHeaderOffset + 108);
        nDataDirectoryOffset = nOptionalHeaderOffset + 112;
    } else {
        return false;
    }

    if (nNumberOfRvaAndSizes > 14) {  // IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR
        g_nCliRva = read_uint32(nDataDirectoryOffset + 14 * 8);
        g_nCliSize = read_uint32(nDataDirectoryOffset + 14 * 8 + 4);
    }

    qint64 nSectionOffset = nOptionalHeaderOffset + nSizeOfOptionalHeader;

    for (qint32 i = 0; i < nNumberOfSections; i++) {
        PE_SECTION_REGION region = {};

        region.nVirtualSize = read_uint32(nSectionOffset + 8);
        region.nVirtualAddress = read_uint32(nSectionOffset + 12);
        region.nSizeOfRawData = read_uint32(nSectionOffset + 16);
        region.nPointerToRawData = read_uint32(nSectionOffset + 20);

        g_listSectionRegions.append(region);

        nSectionOffset += 40;
    }

    g_bPeValid = true;

    return true;
}

qint64 XCLIAssembly::_rvaToOffset(quint32 nRVA)
{
    qint64 nResult = -1;

    if (isImage()) {
        return nRVA;  // Loaded image: RVA maps directly
    }

    if (_parseHeaders()) {
        qint32 nNumberOfRegions = g_listSectionRegions.count();

        if (nNumberOfRegions == 0) {
            nResult = nRVA;
        }

        for (qint32 i = 0; i < nNumberOfRegions; i++) {
            const PE_SECTION_REGION &region = g_listSectionRegions.at(i);

            quint32 nRegionSize = qMax(region.nVirtualSize, region.nSizeOfRawData);

            if ((nRVA >= region.nVirtualAddress) && (nRVA < region.nVirtualAddress + nRegionSize)) {
                if (nRVA - region.nVirtualAddress < region.nSizeOfRawData) {
                    nResult = region.nPointerToRawData + (nRVA - region.nVirtualAddress);
                }

                break;
            }
        }

        if ((nResult == -1) && nNumberOfRegions && (nRVA < g_listSectionRegions.at(0).nVirtualAddress)) {
            nResult = nRVA;  // Header region
        }
    }

    return nResult;
}

qint64 XCLIAssembly::_getNetHeaderOffset()
{
    qint64 nResult = g_nNetHeaderOffset;

    if (nResult == -1) {
        if (_parseHeaders() && g_nCliRva && g_nCliSize) {
            nResult = _rvaToOffset(g_nCliRva);
        }
    }

    return nResult;
}

qint64 XCLIAssembly::_getNetMetaDataOffset()
{
    qint64 nResult = g_nNetMetaDataOffset;

    if (nResult == -1) {
        qint64 nHeaderOffset = _getNetHeaderOffset();

        if (nHeaderOffset != -1) {
            XCLIASSEMBLY_DEF::IMAGE_COR20_HEADER header = _read_IMAGE_COR20_HEADER(nHeaderOffset);

            if ((header.cb == 0x48) && header.MetaData.VirtualAddress && header.MetaData.Size) {
                nResult = _rvaToOffset(header.MetaData.VirtualAddress);
            }
        }
    }

    return nResult;
}

XCLIAssembly::~XCLIAssembly()
{
}

bool XCLIAssembly::isValid(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    bool bResult = false;

    qint64 nHeaderOffset = _getNetHeaderOffset();

    if (nHeaderOffset != -1) {
        XCLIASSEMBLY_DEF::IMAGE_COR20_HEADER header = _read_IMAGE_COR20_HEADER(nHeaderOffset);

        if ((header.cb == 0x48) && header.MetaData.VirtualAddress && header.MetaData.Size) {
            qint64 nMetaDataOffset = _getNetMetaDataOffset();

            if (nMetaDataOffset != -1) {
                bResult = (read_uint32(nMetaDataOffset) == 0x424A5342);  // 'BSJB'
            }
        }
    }

    return bResult;
}

bool XCLIAssembly::isValid(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress, PDSTRUCT *pPdStruct)
{
    XCLIAssembly xcliassembly(pDevice, bIsImage, nModuleAddress);

    return xcliassembly.isValid(pPdStruct);
}

XBinary::MODE XCLIAssembly::getMode(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    XCLIAssembly xcliassembly(pDevice, bIsImage, nModuleAddress);

    return xcliassembly.getMode();
}

bool XCLIAssembly::isExecutable()
{
    return true;
}

QList<XBinary::MAPMODE> XCLIAssembly::getMapModesList()
{
    QList<MAPMODE> listResult;

    listResult.append(MAPMODE_REGIONS);

    return listResult;
}

XBinary::_MEMORY_MAP XCLIAssembly::getMemoryMap(XBinary::MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(mapMode)

    XBinary::PDSTRUCT pdStructEmpty = {};

    if (!pPdStruct) {
        pdStructEmpty = XBinary::createPdStruct();
        pPdStruct = &pdStructEmpty;
    }

    _MEMORY_MAP result = {};

    qint64 nTotalSize = getSize();

    result.nModuleAddress = getModuleAddress();
    result.nEntryPointAddress = result.nModuleAddress;
    result.nBinarySize = nTotalSize;
    result.nImageSize = getImageSize();
    result.fileType = getFileType();
    result.mode = getMode();
    result.sArch = getArch();
    result.endian = getEndian();
    result.sType = getTypeAsString();

    _MEMORY_RECORD record = {};
    record.nAddress = result.nModuleAddress;
    record.nOffset = 0;
    record.nSize = nTotalSize;
    record.nIndex = 0;

    result.listRecords.append(record);

    _handleOverlay(&result);

    return result;
}

QString XCLIAssembly::getArch()
{
    QString sResult = QString("MSIL");

    return sResult;
}

XBinary::MODE XCLIAssembly::getMode()
{
    MODE modeResult = MODE_32;

    if (_parseHeaders() && g_bPeIs64) {
        modeResult = MODE_64;
    }

    return modeResult;
}

XBinary::ENDIAN XCLIAssembly::getEndian()
{
    return ENDIAN_LITTLE;
}

qint64 XCLIAssembly::getImageSize()
{
    return getSize();
}

XBinary::FT XCLIAssembly::getFileType()
{
    return FT_CLI_ASSEMBLY;
}

QString XCLIAssembly::getVersion()
{
    QString sResult = g_sVersion;

    if (sResult.isEmpty()) {
        qint64 nMetaDataOffset = _getNetMetaDataOffset();

        if (nMetaDataOffset != -1) {
            CLI_METADATA_HEADER header = _read_MetadataHeader(nMetaDataOffset);

            if (header.nSignature == 0x424A5342) {
                sResult = header.sVersion;
            }
        }
    }

    return sResult;
}

void XCLIAssembly::setNetHeaderOffset(qint64 nOffset)
{
    g_nNetHeaderOffset = nOffset;
}

void XCLIAssembly::setNetMetaDataOffset(qint64 nOffset)
{
    g_nNetMetaDataOffset = nOffset;
}

void XCLIAssembly::setVersion(const QString &sVersion)
{
    g_sVersion = sVersion;
}

qint32 XCLIAssembly::getType()
{
    qint32 nResult = TYPE_EXE;

    if (_parseHeaders() && (g_nPeCharacteristics & XCLIASSEMBLY_DEF::S_IMAGE_FILE_DLL)) {
        nResult = TYPE_DLL;
    }

    return nResult;
}

XBinary::OSNAME XCLIAssembly::getOsName()
{
    return OSNAME_WINDOWS;
}

QString XCLIAssembly::typeIdToString(qint32 nType)
{
    QString sResult = tr("Unknown");

    switch (nType) {
        case TYPE_UNKNOWN: sResult = tr("Unknown"); break;
        case TYPE_EXE: sResult = QString("EXE"); break;
        case TYPE_DLL: sResult = QString("DLL"); break;
    }

    return sResult;
}

QString XCLIAssembly::getMIMEString()
{
    return "application/x-msdownload";
}

QString XCLIAssembly::getFileFormatExt()
{
    if (getType() == TYPE_DLL) {
        return "dll";
    }

    return "exe";
}

QString XCLIAssembly::getFileFormatExtsString()
{
    return "CLI Assembly files (*.exe *.dll);;";
}

qint64 XCLIAssembly::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    return getSize();
}

QString XCLIAssembly::structIDToString(quint32 nID)
{
    return XBinary::XCONVERT_idToTransString(nID, _TABLE_XCLIASSEMBLY_STRUCTID, sizeof(_TABLE_XCLIASSEMBLY_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QString XCLIAssembly::structIDToFtString(quint32 nID)
{
    return XBinary::XCONVERT_idToFtString(nID, _TABLE_XCLIASSEMBLY_STRUCTID, sizeof(_TABLE_XCLIASSEMBLY_STRUCTID) / sizeof(XBinary::XCONVERT));
}

quint32 XCLIAssembly::ftStringToStructID(const QString &sFtString)
{
    return XCONVERT_ftStringToId(sFtString, _TABLE_XCLIASSEMBLY_STRUCTID, sizeof(_TABLE_XCLIASSEMBLY_STRUCTID) / sizeof(XBinary::XCONVERT));
}

// QList<XBinary::DATA_HEADER> XCLIAssembly::getDataHeaders(const DATA_HEADERS_OPTIONS &dataHeadersOptions, PDSTRUCT *pPdStruct)
// {
//     QList<DATA_HEADER> listResult;

//     // TODO

//     return listResult;
// }

QList<XBinary::FPART> XCLIAssembly::getFileParts(quint32 nFileParts, qint32 nLimit, PDSTRUCT *pPdStruct)
{
    QList<FPART> listResult;

    if ((nLimit < -1) || (nLimit == 0)) {
        return listResult;
    }

    CLI_INFO cliInfo = getCliInfo(false, pPdStruct);

    if (nFileParts & FILEPART_HEADER) {
        qint64 nHeaderOffset = (cliInfo.nHeaderOffset > 0) ? cliInfo.nHeaderOffset : 0;
        qint64 nHeaderSize = cliInfo.bValid ? 0x48 : getSize();

        FPART record = {};

        record.filePart = FILEPART_HEADER;
        record.nFileOffset = nHeaderOffset;
        record.nFileSize = nHeaderSize;
        record.nVirtualAddress = (XADDR)-1;
        record.sName = tr("Header");

        listResult.append(record);
        if ((nLimit != -1) && (listResult.count() >= nLimit)) return listResult;
    }

    if (nFileParts & FILEPART_REGION) {
        if (cliInfo.bValid) {
            qint32 nNumberOfStreams = cliInfo.metaData.listStreams.count();

            for (qint32 i = 0; (i < nNumberOfStreams) && isPdStructNotCanceled(pPdStruct); i++) {
                FPART record = {};

                record.filePart = FILEPART_REGION;
                record.nFileOffset = cliInfo.metaData.listStreams.at(i).nOffset;
                record.nFileSize = cliInfo.metaData.listStreams.at(i).nSize;
                record.nVirtualAddress = (XADDR)-1;
                record.sName = cliInfo.metaData.listStreams.at(i).sName;

                listResult.append(record);
                if ((nLimit != -1) && (listResult.count() >= nLimit)) return listResult;
            }
        }
    }

    return listResult;
}

// ---------------------------------------------------------------------------
// .NET / CLI metadata parsing (moved from XPE)
// ---------------------------------------------------------------------------

XCLIASSEMBLY_DEF::IMAGE_COR20_HEADER XCLIAssembly::_read_IMAGE_COR20_HEADER(qint64 nOffset)
{
    XCLIASSEMBLY_DEF::IMAGE_COR20_HEADER result = {};

    result.cb = read_uint32(nOffset + 0);
    result.MajorRuntimeVersion = read_uint16(nOffset + 4);
    result.MinorRuntimeVersion = read_uint16(nOffset + 6);
    result.MetaData.VirtualAddress = read_uint32(nOffset + 8);
    result.MetaData.Size = read_uint32(nOffset + 12);
    result.Flags = read_uint32(nOffset + 16);
    result.EntryPointRVA = read_uint32(nOffset + 20);
    result.Resources.VirtualAddress = read_uint32(nOffset + 24);
    result.Resources.Size = read_uint32(nOffset + 28);
    result.StrongNameSignature.VirtualAddress = read_uint32(nOffset + 32);
    result.StrongNameSignature.Size = read_uint32(nOffset + 36);
    result.CodeManagerTable.VirtualAddress = read_uint32(nOffset + 40);
    result.CodeManagerTable.Size = read_uint32(nOffset + 44);
    result.VTableFixups.VirtualAddress = read_uint32(nOffset + 48);
    result.VTableFixups.Size = read_uint32(nOffset + 52);
    result.ExportAddressTableJumps.VirtualAddress = read_uint32(nOffset + 56);
    result.ExportAddressTableJumps.Size = read_uint32(nOffset + 60);
    result.ManagedNativeHeader.VirtualAddress = read_uint32(nOffset + 64);
    result.ManagedNativeHeader.Size = read_uint32(nOffset + 68);

    return result;
}

XCLIAssembly::CLI_INFO XCLIAssembly::getCliInfo(bool bFindHidden, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    // https://www.codeproject.com/Articles/12585/The-NET-File-Format
    // The CLI header offset and the metadata offset are either provided by the parent
    // (XPE::initCLIAssembly via setNetHeaderOffset/setNetMetaDataOffset) or resolved
    // here from the PE headers.
    CLI_INFO result = {};

    qint64 nCLIHeaderOffset = _getNetHeaderOffset();

    bool bHiddenGuess = false;

    if ((nCLIHeaderOffset == -1) && bFindHidden) {
        // TODO Check! Some protectors strip the COM descriptor data directory entry.
        nCLIHeaderOffset = _rvaToOffset(0x2008);
        bHiddenGuess = true;
    }

    if (nCLIHeaderOffset != -1) {
        result.nHeaderOffset = nCLIHeaderOffset;

        result.header = _read_IMAGE_COR20_HEADER(result.nHeaderOffset);

        if ((result.header.cb == 0x48) && result.header.MetaData.VirtualAddress && result.header.MetaData.Size) {
            result.bValid = true;
            // Only report "hidden" when the guessed header actually validated
            result.bHidden = bHiddenGuess;

            result.metaData.nEntryPointSize = 0;
            result.metaData.nEntryPoint = result.header.EntryPointRVA;

            result.nMetaDataOffset = g_nNetMetaDataOffset;

            if (result.nMetaDataOffset == -1) {
                result.nMetaDataOffset = _rvaToOffset(result.header.MetaData.VirtualAddress);
            }

            if ((result.nMetaDataOffset != -1) && isOffsetValid(result.nMetaDataOffset)) {
                result.metaData.header = _read_MetadataHeader(result.nMetaDataOffset);

                if (result.metaData.header.nSignature == 0x424a5342) {
                    // result.bInit=true;
                    qint64 nTotalSize = getSize();

                    qint64 nOffset = result.nMetaDataOffset + 20 + result.metaData.header.nVersionStringLength;

                    // The stream headers live inside the metadata region; do not walk past it
                    qint64 nStreamHeadersEnd = result.nMetaDataOffset + result.header.MetaData.Size;

                    if ((nStreamHeadersEnd <= result.nMetaDataOffset) || (nStreamHeadersEnd > nTotalSize)) {
                        nStreamHeadersEnd = nTotalSize;
                    }

                    bool bStrings = false;
                    bool bUS = false;

                    qint32 nNumberOfStreams = result.metaData.header.nStreams;

                    for (qint32 i = 0; (i < nNumberOfStreams) && isPdStructNotCanceled(pPdStruct); i++) {
                        if ((nOffset < 0) || ((nOffset + 8) > nStreamHeadersEnd)) {
                            break;
                        }

                        CLI_METADATA_STREAM stream = {};

                        stream.nOffset = read_uint32(nOffset + 0);
                        stream.nSize = read_uint32(nOffset + 4);
                        stream.sName = read_ansiString(nOffset + 8, 32);  // Stream names are at most 32 bytes

                        if (stream.sName.isEmpty()) {
                            break;
                        }

                        stream.nOffset += result.nMetaDataOffset;

                        // A malformed file can point a stream outside the image: clamp it so the
                        // read_array calls below cannot allocate/read out of bounds.
                        if ((stream.nOffset < 0) || (stream.nOffset > nTotalSize)) {
                            stream.nOffset = 0;
                            stream.nSize = 0;
                        } else if ((stream.nSize < 0) || ((stream.nOffset + stream.nSize) > nTotalSize)) {
                            stream.nSize = nTotalSize - stream.nOffset;
                        }

                        result.metaData.listStreams.append(stream);

                        if ((result.metaData.listStreams.at(i).sName == "#~") || (result.metaData.listStreams.at(i).sName == "#-")) {
                            result.metaData.osMetadata.nOffset = result.metaData.listStreams.at(i).nOffset;
                            result.metaData.osMetadata.nSize = result.metaData.listStreams.at(i).nSize;

                            result.metaData.baMetadata = read_array(result.metaData.osMetadata.nOffset, result.metaData.osMetadata.nSize);
                        } else if ((result.metaData.listStreams.at(i).sName == "#Strings") && (!bStrings)) {
                            result.metaData.osStrings.nOffset = result.metaData.listStreams.at(i).nOffset;
                            result.metaData.osStrings.nSize = result.metaData.listStreams.at(i).nSize;

                            result.metaData.baStrings = read_array(result.metaData.osStrings.nOffset, result.metaData.osStrings.nSize);
                            bStrings = true;
                        } else if ((result.metaData.listStreams.at(i).sName == "#US") && (!bUS)) {
                            result.metaData.osUS.nOffset = result.metaData.listStreams.at(i).nOffset;
                            result.metaData.osUS.nSize = result.metaData.listStreams.at(i).nSize;

                            result.metaData.baUS = read_array(result.metaData.osUS.nOffset, result.metaData.osUS.nSize);
                            bUS = true;
                        } else if (result.metaData.listStreams.at(i).sName == "#Blob") {
                            result.metaData.osBlob.nOffset = result.metaData.listStreams.at(i).nOffset;
                            result.metaData.osBlob.nSize = result.metaData.listStreams.at(i).nSize;
                        } else if (result.metaData.listStreams.at(i).sName == "#GUID") {
                            result.metaData.osGUID.nOffset = result.metaData.listStreams.at(i).nOffset;
                            result.metaData.osGUID.nSize = result.metaData.listStreams.at(i).nSize;
                        }

                        nOffset += 8;
                        nOffset += S_ALIGN_UP((result.metaData.listStreams.at(i).sName.length() + 1), 4);
                    }

                    if ((result.metaData.osMetadata.nOffset > 0) && (!result.metaData.baMetadata.isEmpty())) {
                        qint32 _nOffset = 0;
                        char *pBuffer = result.metaData.baMetadata.data();
                        qint32 nBufferOffset = result.metaData.baMetadata.size();

                        result.metaData.nTables_Reserved1 = _read_uint32_safe(pBuffer, nBufferOffset, _nOffset + 0);
                        result.metaData.cTables_MajorVersion = _read_uint8_safe(pBuffer, nBufferOffset, _nOffset + 4);
                        result.metaData.cTables_MinorVersion = _read_uint8_safe(pBuffer, nBufferOffset, _nOffset + 5);
                        result.metaData.cTables_HeapOffsetSizes = _read_uint8_safe(pBuffer, nBufferOffset, _nOffset + 6);
                        result.metaData.cTables_Reserved2 = _read_uint8_safe(pBuffer, nBufferOffset, _nOffset + 7);
                        result.metaData.nTables_Valid = _read_uint64_safe(pBuffer, nBufferOffset, _nOffset + 8);
                        result.metaData.nTables_Sorted = _read_uint64_safe(pBuffer, nBufferOffset, _nOffset + 16);

                        quint64 nValid = result.metaData.nTables_Valid;

                        quint32 nTemp = 0;

                        for (nTemp = 0; nValid; nTemp++) {
                            nValid &= (nValid - 1);
                        }

                        result.metaData.nTables_Valid_NumberOfRows = nTemp;

                        nOffset = result.metaData.osMetadata.nOffset + 24;
                        _nOffset = 24;

                        for (qint32 i = 0; i < 64; i++) {
                            if (result.metaData.nTables_Valid & ((unsigned long long)1 << i)) {
                                result.metaData.Tables_TablesNumberOfIndexes[i] = _read_uint32_safe(pBuffer, nBufferOffset, _nOffset);
                                nOffset += 4;
                                _nOffset += 4;
                            } else {
                                result.metaData.Tables_TablesNumberOfIndexes[i] = 0;
                            }
                        }

                        result.metaData.nStringIndexSize = 2;
                        result.metaData.nGUIDIndexSize = 2;
                        result.metaData.nBLOBIndexSize = 2;
                        result.metaData.nResolutionScopeSize = 2;
                        result.metaData.nTypeDefOrRefSize = 2;
                        result.metaData.nMemberRefParentSize = 2;
                        result.metaData.nHasConstantSize = 2;
                        result.metaData.nHasCustomAttributeSize = 2;
                        result.metaData.nCustomAttributeTypeSize = 2;
                        result.metaData.nHasFieldMarshalSize = 2;
                        result.metaData.nHasDeclSecuritySize = 2;
                        result.metaData.nHasSemanticsSize = 2;
                        result.metaData.nMethodDefOrRefSize = 2;
                        result.metaData.nMemberForwardedSize = 2;
                        result.metaData.nImplementationSize = 2;
                        result.metaData.nTypeOrMethodDefSize = 2;
                        result.metaData.nHasCustomDebugInformationSize = 2;

                        quint8 cHeapOffsetSizes = result.metaData.cTables_HeapOffsetSizes;

                        if (cHeapOffsetSizes & 0x01) {
                            result.metaData.nStringIndexSize = 4;
                        }

                        if (cHeapOffsetSizes & 0x02) {
                            result.metaData.nGUIDIndexSize = 4;
                        }

                        if (cHeapOffsetSizes & 0x04) {
                            result.metaData.nBLOBIndexSize = 4;
                        }

                        // TODO !!!

                        if (result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_Module] > 0x3FFF ||
                            result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_ModuleRef] > 0x3FFF ||
                            result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_AssemblyRef] > 0x3FFF ||
                            result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_TypeRef] > 0x3FFF) {
                            result.metaData.nResolutionScopeSize = 4;
                        }
                        if (result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_ModuleRef] > 0x3FFF ||
                            result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_TypeDef] > 0x3FFF ||
                            result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_TypeSpec] > 0x3FFF) {
                            result.metaData.nTypeDefOrRefSize = 4;
                        }
                        if (result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_TypeDef] > 0x1FFF ||
                            result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_TypeRef] > 0x1FFF ||
                            result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_ModuleRef] > 0x1FFF ||
                            result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_MethodDef] > 0x1FFF ||
                            result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_TypeSpec] > 0x1FFF) {
                            result.metaData.nMemberRefParentSize = 4;
                        }
                        if (result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_Field] > 0x3FFF ||
                            result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_Param] > 0x3FFF ||
                            result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_Property] > 0x3FFF) {
                            result.metaData.nHasConstantSize = 4;
                        }
                        if (result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_MethodDef] > 0x7FF ||
                            result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_Field] > 0x7FF ||
                            result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_TypeRef] > 0x7FF ||
                            result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_TypeDef] > 0x7FF ||
                            result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_Param] > 0x7FF ||
                            result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_InterfaceImpl] > 0x7FF ||
                            result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_MemberRef] > 0x7FF ||
                            result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_Module] > 0x7FF ||
                            result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_Property] > 0x7FF ||
                            result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_Event] > 0x7FF ||
                            result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_StandAloneSig] > 0x7FF ||
                            result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_ModuleRef] > 0x7FF ||
                            result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_TypeSpec] > 0x7FF ||
                            result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_Assembly] > 0x7FF) {
                            result.metaData.nHasCustomAttributeSize = 4;
                        }

                        if (result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_MethodDef] > 0x1FFF ||
                            result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_MemberRef] > 0x1FFF) {
                            result.metaData.nCustomAttributeTypeSize = 4;
                        }

                        if (result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_MethodDef] > 0x7FFF ||
                            result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_MemberRef] > 0x7FFF) {
                            result.metaData.nMethodDefOrRefSize = 4;
                        }
                        if (result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_Field] > 0x7FFF ||
                            result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_Param] > 0x7FFF) {
                            result.metaData.nHasFieldMarshalSize = 4;
                        }
                        if (result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_TypeDef] > 0x3FFF ||
                            result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_MethodDef] > 0x3FFF ||
                            result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_Assembly] > 0x3FFF) {
                            result.metaData.nHasDeclSecuritySize = 4;
                        }
                        if (result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_Event] > 0x7FFF ||
                            result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_Property] > 0x7FFF) {
                            result.metaData.nHasSemanticsSize = 4;
                        }
                        if (result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_Field] > 0x7FFF ||
                            result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_MethodDef] > 0x7FFF) {
                            result.metaData.nMemberForwardedSize = 4;
                        }
                        // Implementation: File | AssemblyRef | ExportedType (2 tag bits)
                        if (result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_File] > 0x3FFF ||
                            result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_AssemblyRef] > 0x3FFF ||
                            result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_ExportedType] > 0x3FFF) {
                            result.metaData.nImplementationSize = 4;
                        }
                        // TypeOrMethodDef: TypeDef | MethodDef (1 tag bit)
                        if (result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_TypeDef] > 0x7FFF ||
                            result.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_MethodDef] > 0x7FFF) {
                            result.metaData.nTypeOrMethodDefSize = 4;
                        }
                        // HasCustomDebugInformation: 27 tables (5 tag bits)
                        {
                            static const quint32 arrHasCustomDebugInformation[] = {
                                XCLIASSEMBLY_DEF::metadata_MethodDef,        XCLIASSEMBLY_DEF::metadata_Field,         XCLIASSEMBLY_DEF::metadata_TypeRef,
                                XCLIASSEMBLY_DEF::metadata_TypeDef,          XCLIASSEMBLY_DEF::metadata_Param,         XCLIASSEMBLY_DEF::metadata_InterfaceImpl,
                                XCLIASSEMBLY_DEF::metadata_MemberRef,        XCLIASSEMBLY_DEF::metadata_Module,        XCLIASSEMBLY_DEF::metadata_DeclSecurity,
                                XCLIASSEMBLY_DEF::metadata_Property,         XCLIASSEMBLY_DEF::metadata_Event,         XCLIASSEMBLY_DEF::metadata_StandAloneSig,
                                XCLIASSEMBLY_DEF::metadata_ModuleRef,        XCLIASSEMBLY_DEF::metadata_TypeSpec,      XCLIASSEMBLY_DEF::metadata_Assembly,
                                XCLIASSEMBLY_DEF::metadata_AssemblyRef,      XCLIASSEMBLY_DEF::metadata_File,          XCLIASSEMBLY_DEF::metadata_ExportedType,
                                XCLIASSEMBLY_DEF::metadata_ManifestResource, XCLIASSEMBLY_DEF::metadata_GenericParam,  XCLIASSEMBLY_DEF::metadata_GenericParamConstraint,
                                XCLIASSEMBLY_DEF::metadata_MethodSpec,       XCLIASSEMBLY_DEF::metadata_Document,      XCLIASSEMBLY_DEF::metadata_LocalScope,
                                XCLIASSEMBLY_DEF::metadata_LocalVariable,    XCLIASSEMBLY_DEF::metadata_LocalConstant, XCLIASSEMBLY_DEF::metadata_ImportScope,
                            };

                            const qint32 nNumberOfHcdi = sizeof(arrHasCustomDebugInformation) / sizeof(arrHasCustomDebugInformation[0]);

                            for (qint32 i = 0; i < nNumberOfHcdi; i++) {
                                if (result.metaData.Tables_TablesNumberOfIndexes[arrHasCustomDebugInformation[i]] > 0x7FF) {
                                    result.metaData.nHasCustomDebugInformationSize = 4;
                                    break;
                                }
                            }
                        }

                        for (qint32 i = 0; i < 64; i++) {
                            if (result.metaData.Tables_TablesNumberOfIndexes[i] > 0xFFFF) {
                                result.metaData.indexSize[i] = 4;
                            } else {
                                result.metaData.indexSize[i] = 2;
                            }
                        }

                        // Module
                        {
                            qint32 nSize = 0;
                            nSize += 2;
                            nSize += result.metaData.nStringIndexSize;
                            nSize += result.metaData.nGUIDIndexSize;
                            nSize += result.metaData.nGUIDIndexSize;
                            nSize += result.metaData.nGUIDIndexSize;
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_Module] = nSize;
                        }
                        // TypeRef
                        {
                            qint32 nSize = 0;
                            nSize += result.metaData.nResolutionScopeSize;
                            nSize += result.metaData.nStringIndexSize;
                            nSize += result.metaData.nStringIndexSize;
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_TypeRef] = nSize;
                        }
                        // TypeDef
                        {
                            qint32 nSize = 0;
                            nSize += 4;
                            nSize += result.metaData.nStringIndexSize;
                            nSize += result.metaData.nStringIndexSize;
                            nSize += result.metaData.nTypeDefOrRefSize;
                            nSize += result.metaData.indexSize[XCLIASSEMBLY_DEF::metadata_Field];
                            nSize += result.metaData.indexSize[XCLIASSEMBLY_DEF::metadata_MethodDef];
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_TypeDef] = nSize;
                        }
                        // Field
                        {
                            qint32 nSize = 0;
                            nSize += 2;
                            nSize += result.metaData.nStringIndexSize;
                            nSize += result.metaData.nBLOBIndexSize;
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_Field] = nSize;
                        }
                        // MethodPtr
                        {
                            qint32 nSize = 0;
                            nSize += result.metaData.indexSize[XCLIASSEMBLY_DEF::metadata_MethodDef];
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_MethodPtr] = nSize;
                        }
                        // MethodDef
                        {
                            qint32 nSize = 0;
                            nSize += 4;
                            nSize += 2;
                            nSize += 2;
                            nSize += result.metaData.nStringIndexSize;
                            nSize += result.metaData.nBLOBIndexSize;
                            nSize += result.metaData.indexSize[XCLIASSEMBLY_DEF::metadata_Param];
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_MethodDef] = nSize;
                        }
                        // ParamPtr
                        {
                            qint32 nSize = 0;
                            nSize += result.metaData.indexSize[XCLIASSEMBLY_DEF::metadata_Param];
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_ParamPtr] = nSize;
                        }
                        // Param
                        {
                            qint32 nSize = 0;
                            nSize += 2;
                            nSize += 2;
                            nSize += result.metaData.nStringIndexSize;
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_Param] = nSize;
                        }
                        // InterfaceImpl
                        {
                            qint32 nSize = 0;
                            nSize += result.metaData.indexSize[XCLIASSEMBLY_DEF::metadata_TypeDef];
                            nSize += result.metaData.nTypeDefOrRefSize;
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_InterfaceImpl] = nSize;
                        }
                        // MemberRef
                        {
                            qint32 nSize = 0;
                            nSize += result.metaData.nMemberRefParentSize;
                            nSize += result.metaData.nStringIndexSize;
                            nSize += result.metaData.nBLOBIndexSize;
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_MemberRef] = nSize;
                        }
                        // Constant
                        {
                            qint32 nSize = 0;
                            nSize += 2;
                            nSize += result.metaData.nHasConstantSize;
                            nSize += result.metaData.nBLOBIndexSize;
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_Constant] = nSize;
                        }
                        // CustomAttribute
                        {
                            qint32 nSize = 0;
                            nSize += result.metaData.nHasCustomAttributeSize;
                            nSize += result.metaData.nCustomAttributeTypeSize;
                            nSize += result.metaData.nBLOBIndexSize;
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_CustomAttribute] = nSize;
                        }
                        // FieldMarshal
                        {
                            qint32 nSize = 0;
                            nSize += result.metaData.nHasFieldMarshalSize;
                            nSize += result.metaData.nBLOBIndexSize;
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_FieldMarshal] = nSize;
                        }
                        // DeclSecurity
                        {
                            qint32 nSize = 0;
                            nSize += 2;
                            nSize += result.metaData.nHasDeclSecuritySize;
                            nSize += result.metaData.nBLOBIndexSize;
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_DeclSecurity] = nSize;
                        }
                        // ClassLayout
                        {
                            qint32 nSize = 0;
                            nSize += 2;
                            nSize += 4;
                            nSize += result.metaData.indexSize[XCLIASSEMBLY_DEF::metadata_TypeDef];
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_ClassLayout] = nSize;
                        }
                        // FieldLayout
                        {
                            qint32 nSize = 0;
                            nSize += 4;
                            nSize += result.metaData.indexSize[XCLIASSEMBLY_DEF::metadata_Field];
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_FieldLayout] = nSize;
                        }
                        // StandAloneSig
                        {
                            qint32 nSize = 0;
                            nSize += result.metaData.nBLOBIndexSize;
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_StandAloneSig] = nSize;
                        }
                        // EventMap
                        {
                            qint32 nSize = 0;
                            nSize += result.metaData.indexSize[XCLIASSEMBLY_DEF::metadata_TypeDef];
                            nSize += result.metaData.indexSize[XCLIASSEMBLY_DEF::metadata_Event];
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_EventMap] = nSize;
                        }
                        // EventPtr
                        {
                            qint32 nSize = 0;
                            nSize += result.metaData.indexSize[XCLIASSEMBLY_DEF::metadata_Event];
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_EventPtr] = nSize;
                        }
                        // Event
                        {
                            qint32 nSize = 0;
                            nSize += 2;
                            nSize += result.metaData.nStringIndexSize;
                            nSize += result.metaData.nTypeDefOrRefSize;
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_Event] = nSize;
                        }
                        // PropertyMap
                        {
                            qint32 nSize = 0;
                            nSize += result.metaData.indexSize[XCLIASSEMBLY_DEF::metadata_TypeDef];
                            nSize += result.metaData.indexSize[XCLIASSEMBLY_DEF::metadata_Property];
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_PropertyMap] = nSize;
                        }
                        // PropertyPtr
                        {
                            // Single column: index into the Property table (like MethodPtr/ParamPtr/EventPtr)
                            qint32 nSize = 0;
                            nSize += result.metaData.indexSize[XCLIASSEMBLY_DEF::metadata_Property];
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_PropertyPtr] = nSize;
                        }
                        // Property
                        {
                            qint32 nSize = 0;
                            nSize += 2;
                            nSize += result.metaData.nStringIndexSize;
                            nSize += result.metaData.nBLOBIndexSize;
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_Property] = nSize;
                        }
                        // MethodSemantics
                        {
                            qint32 nSize = 0;
                            nSize += 2;
                            nSize += result.metaData.indexSize[XCLIASSEMBLY_DEF::metadata_MethodDef];
                            nSize += result.metaData.nHasSemanticsSize;
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_MethodSemantics] = nSize;
                        }
                        // MethodImpl
                        {
                            qint32 nSize = 0;
                            nSize += result.metaData.indexSize[XCLIASSEMBLY_DEF::metadata_TypeDef];
                            nSize += result.metaData.nMethodDefOrRefSize;
                            nSize += result.metaData.nMethodDefOrRefSize;
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_MethodImpl] = nSize;
                        }
                        // ModuleRef
                        {
                            qint32 nSize = 0;
                            nSize += result.metaData.nStringIndexSize;
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_ModuleRef] = nSize;
                        }
                        // TypeSpec
                        {
                            qint32 nSize = 0;
                            nSize += result.metaData.nBLOBIndexSize;
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_TypeSpec] = nSize;
                        }
                        // ImplMap
                        {
                            qint32 nSize = 0;
                            nSize += 2;
                            nSize += result.metaData.nMemberForwardedSize;
                            nSize += result.metaData.nStringIndexSize;
                            nSize += result.metaData.indexSize[XCLIASSEMBLY_DEF::metadata_ModuleRef];
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_ImplMap] = nSize;
                        }
                        // FieldRVA
                        {
                            qint32 nSize = 0;
                            nSize += 4;
                            nSize += result.metaData.indexSize[XCLIASSEMBLY_DEF::metadata_Field];
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_FieldRVA] = nSize;
                        }
                        // EncLog
                        {
                            qint32 nSize = 0;
                            nSize += 4;
                            nSize += result.metaData.indexSize[XCLIASSEMBLY_DEF::metadata_MethodDef];
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_ENCLog] = nSize;
                        }
                        // EncMap
                        {
                            qint32 nSize = 0;
                            nSize += 4;
                            nSize += result.metaData.indexSize[XCLIASSEMBLY_DEF::metadata_MethodDef];
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_ENCMap] = nSize;
                        }

                        // Assembly
                        {
                            qint32 nSize = 0;
                            nSize += 4;
                            nSize += 2;
                            nSize += 2;
                            nSize += 2;
                            nSize += 2;
                            nSize += 4;
                            nSize += result.metaData.nBLOBIndexSize;
                            nSize += result.metaData.nStringIndexSize;
                            nSize += result.metaData.nStringIndexSize;
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_Assembly] = nSize;  // Checked
                        }
                        // AssemblyProcessor
                        {
                            qint32 nSize = 0;
                            nSize += 4;
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_AssemblyProcessor] = nSize;  // Checked
                        }
                        // AssemblyOS
                        {
                            qint32 nSize = 0;
                            nSize += 4;
                            nSize += 4;
                            nSize += 4;
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_AssemblyOS] = nSize;  // Checked
                        }
                        // AssemblyRef
                        {
                            // MajorVersion, MinorVersion, BuildNumber, RevisionNumber, Flags,
                            // PublicKeyOrToken, Name, Culture, HashValue.
                            // Has no HashAlgId (that is the Assembly table) but does have a trailing HashValue blob.
                            qint32 nSize = 0;
                            nSize += 2;
                            nSize += 2;
                            nSize += 2;
                            nSize += 2;
                            nSize += 4;
                            nSize += result.metaData.nBLOBIndexSize;
                            nSize += result.metaData.nStringIndexSize;
                            nSize += result.metaData.nStringIndexSize;
                            nSize += result.metaData.nBLOBIndexSize;
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_AssemblyRef] = nSize;
                        }
                        // AssemblyRefProcessor
                        {
                            qint32 nSize = 0;
                            nSize += 4;
                            nSize += result.metaData.indexSize[XCLIASSEMBLY_DEF::metadata_AssemblyRef];
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_AssemblyRefProcessor] = nSize;  // Checked
                        }
                        // AssemblyRefOS
                        {
                            qint32 nSize = 0;
                            nSize += 4;
                            nSize += 4;
                            nSize += 4;
                            nSize += result.metaData.indexSize[XCLIASSEMBLY_DEF::metadata_AssemblyRef];
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_AssemblyRefOS] = nSize;  // Checked
                        }
                        // File
                        {
                            qint32 nSize = 0;
                            nSize += 4;
                            nSize += result.metaData.nStringIndexSize;
                            nSize += result.metaData.nBLOBIndexSize;
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_File] = nSize;  // Checked
                        }
                        // ExportedType
                        {
                            // Implementation is a coded index (File|AssemblyRef|ExportedType), not a File index
                            qint32 nSize = 0;
                            nSize += 4;
                            nSize += 4;
                            nSize += result.metaData.nStringIndexSize;
                            nSize += result.metaData.nStringIndexSize;
                            nSize += result.metaData.nImplementationSize;
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_ExportedType] = nSize;
                        }
                        // ManifestResource
                        {
                            // Implementation is a coded index (File|AssemblyRef|ExportedType), not a File index
                            qint32 nSize = 0;
                            nSize += 4;
                            nSize += 4;
                            nSize += result.metaData.nStringIndexSize;
                            nSize += result.metaData.nImplementationSize;
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_ManifestResource] = nSize;
                        }
                        // NestedClass
                        {
                            qint32 nSize = 0;
                            nSize += result.metaData.indexSize[XCLIASSEMBLY_DEF::metadata_TypeDef];
                            nSize += result.metaData.indexSize[XCLIASSEMBLY_DEF::metadata_TypeDef];
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_NestedClass] = nSize;
                        }
                        // GenericParam
                        {
                            // Owner is a TypeOrMethodDef coded index (TypeDef|MethodDef), not TypeDefOrRef
                            qint32 nSize = 0;
                            nSize += 2;
                            nSize += 2;
                            nSize += result.metaData.nTypeOrMethodDefSize;
                            nSize += result.metaData.nStringIndexSize;
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_GenericParam] = nSize;
                        }
                        // MethodSpec
                        {
                            qint32 nSize = 0;
                            nSize += result.metaData.nMethodDefOrRefSize;
                            nSize += result.metaData.nBLOBIndexSize;
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_MethodSpec] = nSize;
                        }
                        // GenericParamConstraint
                        {
                            // Owner: index into GenericParam (was: the row count, not the index size)
                            qint32 nSize = 0;
                            nSize += result.metaData.indexSize[XCLIASSEMBLY_DEF::metadata_GenericParam];
                            nSize += result.metaData.nTypeDefOrRefSize;
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_GenericParamConstraint] = nSize;
                        }
                        // Portable PDB tables (0x30..0x37)
                        // Document
                        {
                            // Name (Blob), HashAlgorithm (GUID), Hash (Blob), Language (GUID)
                            qint32 nSize = 0;
                            nSize += result.metaData.nBLOBIndexSize;
                            nSize += result.metaData.nGUIDIndexSize;
                            nSize += result.metaData.nBLOBIndexSize;
                            nSize += result.metaData.nGUIDIndexSize;
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_Document] = nSize;
                        }
                        // MethodDebugInformation
                        {
                            // Document (Document index), SequencePoints (Blob)
                            qint32 nSize = 0;
                            nSize += result.metaData.indexSize[XCLIASSEMBLY_DEF::metadata_Document];
                            nSize += result.metaData.nBLOBIndexSize;
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_MethodDebugInformation] = nSize;
                        }
                        // LocalScope
                        {
                            // Method (MethodDef), ImportScope (ImportScope), VariableList (LocalVariable),
                            // ConstantList (LocalConstant), StartOffset (4), Length (4)
                            qint32 nSize = 0;
                            nSize += result.metaData.indexSize[XCLIASSEMBLY_DEF::metadata_MethodDef];
                            nSize += result.metaData.indexSize[XCLIASSEMBLY_DEF::metadata_ImportScope];
                            nSize += result.metaData.indexSize[XCLIASSEMBLY_DEF::metadata_LocalVariable];
                            nSize += result.metaData.indexSize[XCLIASSEMBLY_DEF::metadata_LocalConstant];
                            nSize += 4;
                            nSize += 4;
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_LocalScope] = nSize;
                        }
                        // LocalVariable
                        {
                            // Attributes (2), Index (2), Name (String)
                            qint32 nSize = 0;
                            nSize += 2;
                            nSize += 2;
                            nSize += result.metaData.nStringIndexSize;
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_LocalVariable] = nSize;
                        }
                        // LocalConstant
                        {
                            // Name (String), Signature (Blob)
                            qint32 nSize = 0;
                            nSize += result.metaData.nStringIndexSize;
                            nSize += result.metaData.nBLOBIndexSize;
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_LocalConstant] = nSize;
                        }
                        // ImportScope
                        {
                            // Parent (ImportScope index), Imports (Blob)
                            qint32 nSize = 0;
                            nSize += result.metaData.indexSize[XCLIASSEMBLY_DEF::metadata_ImportScope];
                            nSize += result.metaData.nBLOBIndexSize;
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_ImportScope] = nSize;
                        }
                        // StateMachineMethod
                        {
                            // MoveNextMethod (MethodDef), KickoffMethod (MethodDef)
                            qint32 nSize = 0;
                            nSize += result.metaData.indexSize[XCLIASSEMBLY_DEF::metadata_MethodDef];
                            nSize += result.metaData.indexSize[XCLIASSEMBLY_DEF::metadata_MethodDef];
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_StateMachineMethod] = nSize;
                        }
                        // CustomDebugInformation
                        {
                            // Parent (HasCustomDebugInformation), Kind (GUID), Value (Blob)
                            qint32 nSize = 0;
                            nSize += result.metaData.nHasCustomDebugInformationSize;
                            nSize += result.metaData.nGUIDIndexSize;
                            nSize += result.metaData.nBLOBIndexSize;
                            result.metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_CustomDebugInformation] = nSize;
                        }

                        if (result.metaData.cTables_HeapOffsetSizes & 0x40) {
                            nOffset += 4;
                        }

                        for (qint32 i = 0; i < 64; i++) {
                            if (result.metaData.Tables_TablesNumberOfIndexes[i]) {
                                if (result.metaData.Tables_TableElementSizes[i] <= 0) {
                                    // Row size unknown (e.g. a Portable PDB table): every following
                                    // table offset would be wrong, so stop rather than report bad offsets.
                                    break;
                                }

                                result.metaData.Tables_TablesOffsets[i] = nOffset;
                                nOffset += result.metaData.Tables_TableElementSizes[i] * result.metaData.Tables_TablesNumberOfIndexes[i];
                            }
                        }
                    }
                }
            }
        }
    }

    //    emit appendError(".NET is not present");
    return result;
}

QList<XCLIAssembly::CLI_METADATA_RECORD> XCLIAssembly::getCliMetadataRecords(CLI_INFO *pCliInfo, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    QList<XCLIAssembly::CLI_METADATA_RECORD> listResult;

    if (!pCliInfo || !isPdStructNotCanceled(pPdStruct)) {
        return listResult;
    }

    for (qint32 i = 0; (i < 64) && isPdStructNotCanceled(pPdStruct); i++) {
        if (pCliInfo->metaData.nTables_Valid & ((unsigned long long)1 << i)) {
            CLI_METADATA_RECORD record = {};

            record.nNumber = i;
            record.bIsSorted = pCliInfo->metaData.nTables_Sorted & ((unsigned long long)1 << i);
            record.sId = mdtIdToString(i);
            record.nTableOffset = pCliInfo->metaData.Tables_TablesOffsets[i];
            record.nTableSize = pCliInfo->metaData.Tables_TablesNumberOfIndexes[i] * pCliInfo->metaData.Tables_TableElementSizes[i];
            record.nCount = pCliInfo->metaData.Tables_TablesNumberOfIndexes[i];

            listResult.append(record);
        }
    }

    return listResult;
}

QList<QString> XCLIAssembly::getAnsiStrings(CLI_INFO *pCliInfo, PDSTRUCT *pPdStruct)
{
    QList<QString> listResult;

    if (!pCliInfo || !isPdStructNotCanceled(pPdStruct)) {
        return listResult;
    }

    char *_pOffset = pCliInfo->metaData.baStrings.data();
    qint32 _nSize = pCliInfo->metaData.baStrings.size();

    // TODO UTF8
    for (qint32 i = 1; (i < _nSize) && isPdStructNotCanceled(pPdStruct); i++) {
        _pOffset++;
        QString sTemp = _pOffset;
        listResult.append(sTemp);

        _pOffset += sTemp.size();
        i += sTemp.size();
    }

    return listResult;
}

QList<QString> XCLIAssembly::getUnicodeStrings(CLI_INFO *pCliInfo, PDSTRUCT *pPdStruct)
{
    QList<QString> listResult;

    if (!pCliInfo || !isPdStructNotCanceled(pPdStruct)) {
        return listResult;
    }

    char *pStringOffset = pCliInfo->metaData.baUS.data();
    char *pStringCurrentOffsetOffset = pStringOffset;
    qint32 _nSize = pCliInfo->metaData.baUS.size();

    if (_nSize <= 1) return listResult;

    pStringCurrentOffsetOffset++;

    for (qint32 i = 1; (i < _nSize) && isPdStructNotCanceled(pPdStruct); i++) {
        qint32 nStringSize = (*((unsigned char *)pStringCurrentOffsetOffset));

        if (nStringSize == 0x80) {
            nStringSize = 0;
        }

        if (nStringSize > _nSize - i) {
            break;
        }

        pStringCurrentOffsetOffset++;

        if (pStringCurrentOffsetOffset > pStringOffset + _nSize) {
            break;
        }

        QString sTemp = QString::fromUtf16(reinterpret_cast<const char16_t *>(pStringCurrentOffsetOffset), nStringSize / 2);

        listResult.append(sTemp);

        pStringCurrentOffsetOffset += nStringSize;
        i += nStringSize;
    }

    return listResult;
}

QVector<XBinary::XIMPORT_STRUCT> XCLIAssembly::getImportStructs()
{
    QVector<XIMPORT_STRUCT> listResult;
    CLI_INFO cliInfo = getCliInfo();

    if (!cliInfo.bValid) {
        return listResult;
    }

    const qint32 nTable = XCLIASSEMBLY_DEF::metadata_MemberRef;
    const qint32 nCount = qMin<qint32>(cliInfo.metaData.Tables_TablesNumberOfIndexes[nTable], 0x20000);
    const qint64 nRowSize = cliInfo.metaData.Tables_TableElementSizes[nTable];

    for (qint32 i = 0; (i < nCount) && (nRowSize > 0); ++i) {
        const XCLIASSEMBLY_DEF::S_METADATA_MEMBERREF memberRef = getMetadataMemberRef(&cliInfo, i);
        const quint32 nParentTag = memberRef.nClass & 0x07;

        // TypeRef, ModuleRef and TypeSpec parents denote references outside the
        // current type definitions. TypeDef/MethodDef parents are local refs.
        if ((nParentTag != 1) && (nParentTag != 2) && (nParentTag != 4)) {
            continue;
        }

        XIMPORT_STRUCT record = {};
        record.nOffset = cliInfo.metaData.Tables_TablesOffsets[nTable] + (qint64)i * nRowSize;
        record.nSize = nRowSize;
        record.nAddress = (XADDR)-1;
        record.sLibrary = getMetadataMemberRefParentName(&cliInfo, memberRef);
        record.sFunction = _read_ansiString_safe(cliInfo.metaData.baStrings.data(), cliInfo.metaData.baStrings.size(), memberRef.nName);
        record.nOrdinal = -1;

        if (!record.sFunction.isEmpty()) {
            listResult.append(record);
        }
    }

    return listResult;
}

QVector<XBinary::XSYMBOL_STRUCT> XCLIAssembly::getSymbolStructs()
{
    QVector<XSYMBOL_STRUCT> listResult;
    CLI_INFO cliInfo = getCliInfo();

    if (!cliInfo.bValid) {
        return listResult;
    }

    const qint32 nMethodTable = XCLIASSEMBLY_DEF::metadata_MethodDef;
    const qint32 nMethodCount = qMin<qint32>(cliInfo.metaData.Tables_TablesNumberOfIndexes[nMethodTable], 0x20000);
    const qint64 nMethodRowSize = cliInfo.metaData.Tables_TableElementSizes[nMethodTable];
    QVector<QString> listTypeNames(nMethodCount);

    const qint32 nTypeCount = qMin<qint32>(cliInfo.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_TypeDef], 0x10000);
    for (qint32 i = 0; i < nTypeCount; ++i) {
        const XCLIASSEMBLY_DEF::S_METADATA_TYPEDEF typeDef = getMetadataTypeDef(&cliInfo, i);
        const quint32 nFirst = typeDef.nMethodList ? typeDef.nMethodList - 1 : 0;
        quint32 nLast = nMethodCount;
        if (i + 1 < nTypeCount) {
            const XCLIASSEMBLY_DEF::S_METADATA_TYPEDEF nextTypeDef = getMetadataTypeDef(&cliInfo, i + 1);
            nLast = nextTypeDef.nMethodList ? nextTypeDef.nMethodList - 1 : nLast;
        }

        const QString sNamespace = _read_ansiString_safe(cliInfo.metaData.baStrings.data(), cliInfo.metaData.baStrings.size(), typeDef.nTypeNamespace);
        QString sTypeName = _read_ansiString_safe(cliInfo.metaData.baStrings.data(), cliInfo.metaData.baStrings.size(), typeDef.nTypeName);
        if (!sNamespace.isEmpty()) {
            sTypeName = sNamespace + QChar('.') + sTypeName;
        }

        for (quint32 j = nFirst; (j < nLast) && (j < (quint32)nMethodCount); ++j) {
            listTypeNames[(qint32)j] = sTypeName;
        }
    }

    const quint32 METHOD_ACCESS_MASK = 0x0007;
    const quint32 METHOD_FAMILY = 0x0004;
    const quint32 METHOD_FAM_OR_ASSEM = 0x0005;
    const quint32 METHOD_PUBLIC = 0x0006;

    for (qint32 i = 0; (i < nMethodCount) && (nMethodRowSize > 0); ++i) {
        const XCLIASSEMBLY_DEF::S_METADATA_METHODDEF methodDef = getMetadataMethodDef(&cliInfo, i);
        const quint32 nAccess = methodDef.nFlags & METHOD_ACCESS_MASK;
        QString sName = _read_ansiString_safe(cliInfo.metaData.baStrings.data(), cliInfo.metaData.baStrings.size(), methodDef.nName);

        if (!listTypeNames.at(i).isEmpty()) {
            sName = listTypeNames.at(i) + QString("::") + sName;
        }

        XSYMBOL_STRUCT record = {};
        record.nOffset = cliInfo.metaData.Tables_TablesOffsets[nMethodTable] + (qint64)i * nMethodRowSize;
        record.nSize = nMethodRowSize;
        record.nAddress = (XADDR)-1;
        record.sName = sName;
        record.symbolType = ((nAccess == METHOD_PUBLIC) || (nAccess == METHOD_FAMILY) || (nAccess == METHOD_FAM_OR_ASSEM)) ? SYMBOL_TYPE_EXPORT : SYMBOL_TYPE_LABEL;

        if (methodDef.nRVA != 0) {
            const qint64 nCodeOffset = _rvaToOffset(methodDef.nRVA);
            if (nCodeOffset >= 0) {
                record.nAddress = offsetToAddress(nCodeOffset);
            }
        }

        if (!record.sName.isEmpty()) {
            listResult.append(record);
        }
    }

    return listResult;
}

QVector<XBinary::XEXPORT_STRUCT> XCLIAssembly::getExportStructs()
{
    QVector<XEXPORT_STRUCT> listResult;
    const QVector<XSYMBOL_STRUCT> listSymbols = getSymbolStructs();

    for (qint32 i = 0; i < listSymbols.count(); ++i) {
        const XSYMBOL_STRUCT &symbol = listSymbols.at(i);
        if (symbol.symbolType != SYMBOL_TYPE_EXPORT) {
            continue;
        }

        XEXPORT_STRUCT record = {};
        record.nOffset = symbol.nOffset;
        record.nSize = symbol.nSize;
        record.nAddress = symbol.nAddress;
        record.sFunction = symbol.sName;
        record.nOrdinal = i + 1;
        listResult.append(record);
    }

    return listResult;
}

QVector<XBinary::XRESOURCE_STRUCT> XCLIAssembly::getResourceStructs()
{
    QVector<XRESOURCE_STRUCT> listResult;
    CLI_INFO cliInfo = getCliInfo();

    if (!cliInfo.bValid) {
        return listResult;
    }

    const qint32 nTable = XCLIASSEMBLY_DEF::metadata_ManifestResource;
    const qint32 nCount = qMin<qint32>(cliInfo.metaData.Tables_TablesNumberOfIndexes[nTable], 0x10000);
    const qint64 nRowSize = cliInfo.metaData.Tables_TableElementSizes[nTable];
    const qint64 nResourceBase = cliInfo.header.Resources.VirtualAddress ? _rvaToOffset(cliInfo.header.Resources.VirtualAddress) : -1;

    for (qint32 i = 0; (i < nCount) && (nRowSize > 0); ++i) {
        const qint64 nRowOffset = cliInfo.metaData.Tables_TablesOffsets[nTable] + (qint64)i * nRowSize;
        if (!checkOffsetSize(nRowOffset, nRowSize)) {
            break;
        }

        qint64 nFieldOffset = nRowOffset;
        const quint32 nRelativeOffset = read_uint32(nFieldOffset);
        nFieldOffset += 8;  // Offset + Flags
        const quint32 nNameIndex = (cliInfo.metaData.nStringIndexSize == 4) ? read_uint32(nFieldOffset) : read_uint16(nFieldOffset);
        nFieldOffset += cliInfo.metaData.nStringIndexSize;
        const quint32 nImplementation = (cliInfo.metaData.nImplementationSize == 4) ? read_uint32(nFieldOffset) : read_uint16(nFieldOffset);

        XRESOURCE_STRUCT record = {};
        record.nOffset = nRowOffset;
        record.nSize = nRowSize;
        record.nAddress = (XADDR)-1;
        record.sName = _read_ansiString_safe(cliInfo.metaData.baStrings.data(), cliInfo.metaData.baStrings.size(), nNameIndex);
        record.nType = nImplementation & 0x03;
        record.nID = i + 1;

        // Implementation == 0 means the resource bytes are embedded in the
        // CLI resource directory and prefixed with a four-byte length.
        if ((nImplementation == 0) && (nResourceBase >= 0)) {
            const qint64 nLengthOffset = nResourceBase + nRelativeOffset;
            if (checkOffsetSize(nLengthOffset, 4)) {
                const quint32 nDataSize = read_uint32(nLengthOffset);
                if (checkOffsetSize(nLengthOffset + 4, nDataSize)) {
                    record.nOffset = nLengthOffset + 4;
                    record.nSize = nDataSize;
                    record.nAddress = offsetToAddress(record.nOffset);
                }
            }
        }

        listResult.append(record);
    }

    return listResult;
}

QVector<XBinary::XMETADATA_STRUCT> XCLIAssembly::getMetadataStructs()
{
    QVector<XMETADATA_STRUCT> listResult;
    CLI_INFO cliInfo = getCliInfo();

    if (!cliInfo.bValid) {
        return listResult;
    }

    auto appendMetadata = [&listResult, &cliInfo](XMETADATA_ID id, qint64 nOffset, qint64 nSize, const QString &sName, const QVariant &varValue) {
        if (!varValue.isValid() || varValue.toString().isEmpty()) {
            return;
        }

        XMETADATA_STRUCT record = {};
        record.nOffset = nOffset;
        record.nSize = nSize;
        record.nAddress = (XADDR)-1;
        record.id = id;
        record.sName = sName;
        record.varValue = varValue;
        listResult.append(record);
    };

    appendMetadata(XMETADATA_ID_UNKNOWN, cliInfo.nMetaDataOffset, cliInfo.metaData.osMetadata.nSize, QString("Runtime version"), cliInfo.metaData.header.sVersion);
    if (cliInfo.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_Module] > 0) {
        appendMetadata(XMETADATA_ID_UNKNOWN, cliInfo.nMetaDataOffset, cliInfo.metaData.osMetadata.nSize, QString("Module"), getMetadataModuleName(&cliInfo, 0));

        const XCLIASSEMBLY_DEF::S_METADATA_MODULE module = getMetadataModule(&cliInfo, 0);
        if (module.nMvid > 0) {
            const qint64 nGuidOffset = cliInfo.metaData.osGUID.nOffset + ((qint64)module.nMvid - 1) * 16;
            if ((nGuidOffset >= cliInfo.metaData.osGUID.nOffset) && (nGuidOffset + 16 <= cliInfo.metaData.osGUID.nOffset + cliInfo.metaData.osGUID.nSize) &&
                checkOffsetSize(nGuidOffset, 16)) {
                appendMetadata(XMETADATA_ID_UUID, nGuidOffset, 16, QString("Module version ID"), read_UUID(nGuidOffset));
            }
        }
    }
    if (cliInfo.metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_Assembly] > 0) {
        appendMetadata(XMETADATA_ID_UNKNOWN, cliInfo.nMetaDataOffset, cliInfo.metaData.osMetadata.nSize, QString("Assembly"), getMetadataAssemblyName(&cliInfo, 0));
    }

    return listResult;
}

bool XCLIAssembly::isNetGlobalCctorPresent(CLI_INFO *pCliInfo, PDSTRUCT *pPdStruct)
{
    return isNetMethodPresent(pCliInfo, "", "<Module>", ".cctor", pPdStruct);
}

bool XCLIAssembly::isNetTypePresent(CLI_INFO *pCliInfo, const QString &sTypeNamespace, const QString &sTypeName, PDSTRUCT *pPdStruct)
{
    bool bResult = false;

    if (pCliInfo && isPdStructNotCanceled(pPdStruct) && pCliInfo->bValid) {
        char *pBuffer = pCliInfo->metaData.baStrings.data();
        qint32 nBufferSize = pCliInfo->metaData.baStrings.size();

        qint32 nNumberOfRecords = pCliInfo->metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_TypeDef];

        for (qint32 i = 0; (i < nNumberOfRecords) && isPdStructNotCanceled(pPdStruct); i++) {
            XCLIASSEMBLY_DEF::S_METADATA_TYPEDEF record = getMetadataTypeDef(pCliInfo, i);

            QString _sTypeName;
            QString _sTypeNamespace;

            if (sTypeName != "") {
                _sTypeName = _read_ansiString_safe(pBuffer, nBufferSize, record.nTypeName);
            }

            if (sTypeNamespace != "") {
                _sTypeNamespace = _read_ansiString_safe(pBuffer, nBufferSize, record.nTypeNamespace);
            }

            if ((sTypeNamespace == _sTypeNamespace) && (sTypeName == _sTypeName)) {
                bResult = true;
                break;
            }
        }
    }

    return bResult;
}

bool XCLIAssembly::isNetMethodPresent(CLI_INFO *pCliInfo, QString sTypeNamespace, QString sTypeName, QString sMethodName, PDSTRUCT *pPdStruct)
{
    bool bResult = false;

    if (pCliInfo && isPdStructNotCanceled(pPdStruct) && pCliInfo->bValid) {
        char *pBuffer = pCliInfo->metaData.baStrings.data();
        qint32 nBufferSize = pCliInfo->metaData.baStrings.size();

        qint32 nNumberOfRecords = pCliInfo->metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_TypeDef];

        bool bProcess = true;

        if (nNumberOfRecords > 0xFFFF) {
            bProcess = false;
        }

        if (bProcess) {
            for (qint32 i = 0; (i < nNumberOfRecords) && isPdStructNotCanceled(pPdStruct); i++) {
                XCLIASSEMBLY_DEF::S_METADATA_TYPEDEF record = getMetadataTypeDef(pCliInfo, i);

                QString _sTypeName;
                QString _sTypeNamespace;

                if (sTypeName != "") {
                    _sTypeName = _read_ansiString_safe(pBuffer, nBufferSize, record.nTypeName);
                }

                if (sTypeNamespace != "") {
                    _sTypeNamespace = _read_ansiString_safe(pBuffer, nBufferSize, record.nTypeNamespace);
                }

                if ((sTypeNamespace == _sTypeNamespace) && (sTypeName == _sTypeName)) {
                    qint32 nNumberOfMethodsPtrRecords = pCliInfo->metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_MethodPtr];
                    qint32 nNumberOfMethodsDefRecords = pCliInfo->metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_MethodDef];

                    qint32 nMethodsCount = 0;
                    if (i < (nNumberOfRecords - 1)) {
                        XCLIASSEMBLY_DEF::S_METADATA_TYPEDEF recordNext = getMetadataTypeDef(pCliInfo, i + 1);
                        nMethodsCount = recordNext.nMethodList - record.nMethodList;
                    } else {
                        nMethodsCount = nNumberOfMethodsPtrRecords - record.nMethodList;
                    }

                    for (qint32 j = 0; (j < nMethodsCount) && isPdStructNotCanceled(pPdStruct); j++) {
                        if (record.nMethodList) {
                            QString _sMethodName;

                            if (nNumberOfMethodsPtrRecords) {
                                XCLIASSEMBLY_DEF::S_METADATA_METHODPTR methodPtr = getMetadataMethodPtr(pCliInfo, record.nMethodList + j - 1);

                                if (methodPtr.nMethod) {
                                    if (methodPtr.nMethod <= (quint32)nNumberOfMethodsDefRecords) {
                                        XCLIASSEMBLY_DEF::S_METADATA_METHODDEF methodDef = getMetadataMethodDef(pCliInfo, methodPtr.nMethod - 1);
                                        _sMethodName = _read_ansiString_safe(pBuffer, nBufferSize, methodDef.nName);
                                    }
                                }
                            } else {
                                XCLIASSEMBLY_DEF::S_METADATA_METHODDEF methodDef = getMetadataMethodDef(pCliInfo, record.nMethodList + j - 1);
                                _sMethodName = _read_ansiString_safe(pBuffer, nBufferSize, methodDef.nName);
                            }

                            // qDebug("_sMethodName: %s", _sMethodName.toLatin1().data());

                            if (sMethodName == _sMethodName) {
                                bResult = true;
                                break;
                            }
                        }
                    }

                    break;
                }

                // qDebug("%s %s", sTypeName.toLatin1().data(), sTypeNamespace.toLatin1().data());
            }
        }
    }

    return bResult;
}

bool XCLIAssembly::isNetFieldPresent(CLI_INFO *pCliInfo, QString sTypeNamespace, QString sTypeName, QString sFieldName, PDSTRUCT *pPdStruct)
{
    bool bResult = false;

    if (pCliInfo && isPdStructNotCanceled(pPdStruct) && pCliInfo->bValid) {
        char *pBuffer = pCliInfo->metaData.baStrings.data();
        qint32 nBufferSize = pCliInfo->metaData.baStrings.size();

        qint32 nNumberOfRecords = pCliInfo->metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_TypeDef];

        for (qint32 i = 0; (i < nNumberOfRecords) && isPdStructNotCanceled(pPdStruct); i++) {
            XCLIASSEMBLY_DEF::S_METADATA_TYPEDEF record = getMetadataTypeDef(pCliInfo, i);

            QString _sTypeName;
            QString _sTypeNamespace;

            if (sTypeName != "") {
                _sTypeName = _read_ansiString_safe(pBuffer, nBufferSize, record.nTypeName);
            }

            if (sTypeNamespace != "") {
                _sTypeNamespace = _read_ansiString_safe(pBuffer, nBufferSize, record.nTypeNamespace);
            }

            if ((sTypeNamespace == _sTypeNamespace) && (sTypeName == _sTypeName)) {
                qint32 nNumberOfFieldsRecords = pCliInfo->metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_Field];
                qint32 nFieldsCount = 0;
                if (i < (nNumberOfRecords - 1)) {
                    XCLIASSEMBLY_DEF::S_METADATA_TYPEDEF recordNext = getMetadataTypeDef(pCliInfo, i + 1);
                    nFieldsCount = recordNext.nFieldList - record.nFieldList;
                } else {
                    nFieldsCount = nNumberOfFieldsRecords - record.nFieldList;
                }

                for (qint32 j = 0; (j < nFieldsCount) && isPdStructNotCanceled(pPdStruct); j++) {
                    XCLIASSEMBLY_DEF::S_METADATA_FIELD field = getMetadataField(pCliInfo, record.nFieldList + j - 1);

                    QString _sFieldName = _read_ansiString_safe(pBuffer, nBufferSize, field.nName);

                    if (sFieldName == _sFieldName) {
                        bResult = true;
                    }
                }

                break;
            }

            // qDebug("%s %s", sTypeName.toLatin1().data(), sTypeNamespace.toLatin1().data());
        }
    }

    return bResult;
}

XCLIASSEMBLY_DEF::S_METADATA_MODULE XCLIAssembly::getMetadataModule(CLI_INFO *pCliInfo, qint32 nNumber)
{
    XCLIASSEMBLY_DEF::S_METADATA_MODULE result = {};

    if (pCliInfo->bValid) {
        qint32 nNumberOfRecords = pCliInfo->metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_Module];
        char *pBuffer = pCliInfo->metaData.baMetadata.data();
        qint32 nBufferSize = pCliInfo->metaData.baMetadata.size();

        if (nNumber < nNumberOfRecords) {
            qint64 nOffset = pCliInfo->metaData.Tables_TablesOffsets[XCLIASSEMBLY_DEF::metadata_Module] +
                             pCliInfo->metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_Module] * nNumber - pCliInfo->metaData.osMetadata.nOffset;

            result.nGeneration = _read_uint16_safe(pBuffer, nBufferSize, nOffset);
            nOffset += 2;
            result.nName = pCliInfo->metaData.nStringIndexSize == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset) : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
            nOffset += pCliInfo->metaData.nStringIndexSize;
            result.nMvid = pCliInfo->metaData.nGUIDIndexSize == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset) : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
            nOffset += pCliInfo->metaData.nGUIDIndexSize;
            result.nEncId = pCliInfo->metaData.nGUIDIndexSize == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset) : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
            nOffset += pCliInfo->metaData.nGUIDIndexSize;
            result.nEncBaseId =
                pCliInfo->metaData.nGUIDIndexSize == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset) : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
        }
    }

    return result;
}

XCLIASSEMBLY_DEF::S_METADATA_MEMBERREF XCLIAssembly::getMetadataMemberRef(CLI_INFO *pCliInfo, qint32 nNumber)
{
    XCLIASSEMBLY_DEF::S_METADATA_MEMBERREF result = {};

    if (pCliInfo->bValid) {
        qint32 nNumberOfRecords = pCliInfo->metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_MemberRef];
        char *pBuffer = pCliInfo->metaData.baMetadata.data();
        qint32 nBufferSize = pCliInfo->metaData.baMetadata.size();

        if (nNumber < nNumberOfRecords) {
            qint64 nOffset = pCliInfo->metaData.Tables_TablesOffsets[XCLIASSEMBLY_DEF::metadata_MemberRef] +
                             pCliInfo->metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_MemberRef] * nNumber - pCliInfo->metaData.osMetadata.nOffset;

            result.nClass =
                pCliInfo->metaData.nMemberRefParentSize == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset) : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
            nOffset += pCliInfo->metaData.nMemberRefParentSize;
            result.nName = pCliInfo->metaData.nStringIndexSize == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset) : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
            nOffset += pCliInfo->metaData.nStringIndexSize;
            result.nSignature =
                pCliInfo->metaData.nBLOBIndexSize == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset) : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
        }
    }

    return result;
}

XCLIASSEMBLY_DEF::S_METADATA_TYPEDEF XCLIAssembly::getMetadataTypeDef(CLI_INFO *pCliInfo, qint32 nNumber)
{
    XCLIASSEMBLY_DEF::S_METADATA_TYPEDEF result = {};

    if (pCliInfo->bValid) {
        qint32 nNumberOfRecords = pCliInfo->metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_TypeDef];
        char *pBuffer = pCliInfo->metaData.baMetadata.data();
        qint32 nBufferSize = pCliInfo->metaData.baMetadata.size();

        if (nNumber < nNumberOfRecords) {
            qint64 nOffset = pCliInfo->metaData.Tables_TablesOffsets[XCLIASSEMBLY_DEF::metadata_TypeDef] +
                             pCliInfo->metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_TypeDef] * nNumber - pCliInfo->metaData.osMetadata.nOffset;

            result.nFlags = _read_uint32_safe(pBuffer, nBufferSize, nOffset);
            nOffset += 4;
            result.nTypeName =
                pCliInfo->metaData.nStringIndexSize == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset) : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
            nOffset += pCliInfo->metaData.nStringIndexSize;
            result.nTypeNamespace =
                pCliInfo->metaData.nStringIndexSize == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset) : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
            nOffset += pCliInfo->metaData.nStringIndexSize;
            result.nExtends =
                pCliInfo->metaData.nTypeDefOrRefSize == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset) : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
            nOffset += pCliInfo->metaData.nTypeDefOrRefSize;
            result.nFieldList = pCliInfo->metaData.indexSize[XCLIASSEMBLY_DEF::metadata_Field] == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset)
                                                                                                    : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
            nOffset += pCliInfo->metaData.indexSize[XCLIASSEMBLY_DEF::metadata_Field];
            result.nMethodList = pCliInfo->metaData.indexSize[XCLIASSEMBLY_DEF::metadata_MethodDef] == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset)
                                                                                                         : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
        }
    }

    return result;
}

XCLIASSEMBLY_DEF::S_METADATA_TYPEREF XCLIAssembly::getMetadataTypeRef(CLI_INFO *pCliInfo, qint32 nNumber)
{
    XCLIASSEMBLY_DEF::S_METADATA_TYPEREF result = {};

    if (pCliInfo->bValid) {
        qint32 nNumberOfRecords = pCliInfo->metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_TypeRef];
        char *pBuffer = pCliInfo->metaData.baMetadata.data();
        qint32 nBufferSize = pCliInfo->metaData.baMetadata.size();

        if (nNumber < nNumberOfRecords) {
            qint64 nOffset = pCliInfo->metaData.Tables_TablesOffsets[XCLIASSEMBLY_DEF::metadata_TypeRef] +
                             pCliInfo->metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_TypeRef] * nNumber - pCliInfo->metaData.osMetadata.nOffset;

            result.nResolutionScope =
                pCliInfo->metaData.nResolutionScopeSize == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset) : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
            nOffset += pCliInfo->metaData.nResolutionScopeSize;
            result.nTypeName =
                pCliInfo->metaData.nStringIndexSize == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset) : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
            nOffset += pCliInfo->metaData.nStringIndexSize;
            result.nTypeNamespace =
                pCliInfo->metaData.nStringIndexSize == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset) : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
        }
    }

    return result;
}

XCLIASSEMBLY_DEF::S_METADATA_MODULEREF XCLIAssembly::getMetadataModuleRef(CLI_INFO *pCliInfo, qint32 nNumber)
{
    XCLIASSEMBLY_DEF::S_METADATA_MODULEREF result = {};

    if (pCliInfo->bValid) {
        qint32 nNumberOfRecords = pCliInfo->metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_ModuleRef];
        char *pBuffer = pCliInfo->metaData.baMetadata.data();
        qint32 nBufferSize = pCliInfo->metaData.baMetadata.size();

        if (nNumber < nNumberOfRecords) {
            qint64 nOffset = pCliInfo->metaData.Tables_TablesOffsets[XCLIASSEMBLY_DEF::metadata_ModuleRef] +
                             pCliInfo->metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_ModuleRef] * nNumber - pCliInfo->metaData.osMetadata.nOffset;

            result.nName = pCliInfo->metaData.nStringIndexSize == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset) : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
        }
    }

    return result;
}

XCLIASSEMBLY_DEF::S_METADATA_METHODDEF XCLIAssembly::getMetadataMethodDef(CLI_INFO *pCliInfo, qint32 nNumber)
{
    XCLIASSEMBLY_DEF::S_METADATA_METHODDEF result = {};

    if (pCliInfo->bValid) {
        qint32 nNumberOfRecords = pCliInfo->metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_MethodDef];
        char *pBuffer = pCliInfo->metaData.baMetadata.data();
        qint32 nBufferSize = pCliInfo->metaData.baMetadata.size();

        if (nNumber < nNumberOfRecords) {
            qint64 nOffset = pCliInfo->metaData.Tables_TablesOffsets[XCLIASSEMBLY_DEF::metadata_MethodDef] +
                             pCliInfo->metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_MethodDef] * nNumber - pCliInfo->metaData.osMetadata.nOffset;

            result.nRVA = _read_uint32_safe(pBuffer, nBufferSize, nOffset);
            nOffset += 4;
            result.nImplFlags = _read_uint16_safe(pBuffer, nBufferSize, nOffset);
            nOffset += 2;
            result.nFlags = _read_uint16_safe(pBuffer, nBufferSize, nOffset);
            nOffset += 2;
            result.nName = pCliInfo->metaData.nStringIndexSize == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset) : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
            nOffset += pCliInfo->metaData.nStringIndexSize;
            result.nSignature =
                pCliInfo->metaData.nBLOBIndexSize == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset) : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
            nOffset += pCliInfo->metaData.nBLOBIndexSize;
            result.nParamList = pCliInfo->metaData.indexSize[XCLIASSEMBLY_DEF::metadata_Param] == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset)
                                                                                                    : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
        }
    }

    return result;
}

XCLIASSEMBLY_DEF::S_METADATA_METHODPTR XCLIAssembly::getMetadataMethodPtr(CLI_INFO *pCliInfo, qint32 nNumber)
{
    XCLIASSEMBLY_DEF::S_METADATA_METHODPTR result = {};

    if (pCliInfo->bValid) {
        qint32 nNumberOfRecords = pCliInfo->metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_MethodPtr];
        char *pBuffer = pCliInfo->metaData.baMetadata.data();
        qint32 nBufferSize = pCliInfo->metaData.baMetadata.size();

        if (nNumber < nNumberOfRecords) {
            qint64 nOffset = pCliInfo->metaData.Tables_TablesOffsets[XCLIASSEMBLY_DEF::metadata_MethodPtr] +
                             pCliInfo->metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_MethodPtr] * nNumber - pCliInfo->metaData.osMetadata.nOffset;
            result.nMethod = pCliInfo->metaData.indexSize[XCLIASSEMBLY_DEF::metadata_MethodDef] == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset)
                                                                                                     : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
        }
    }

    return result;
}

XCLIASSEMBLY_DEF::S_METADATA_PARAM XCLIAssembly::getMetadataParam(CLI_INFO *pCliInfo, qint32 nNumber)
{
    XCLIASSEMBLY_DEF::S_METADATA_PARAM result = {};

    if (pCliInfo->bValid) {
        qint32 nNumberOfRecords = pCliInfo->metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_Param];
        char *pBuffer = pCliInfo->metaData.baMetadata.data();
        qint32 nBufferSize = pCliInfo->metaData.baMetadata.size();

        if (nNumber < nNumberOfRecords) {
            qint64 nOffset = pCliInfo->metaData.Tables_TablesOffsets[XCLIASSEMBLY_DEF::metadata_Param] +
                             pCliInfo->metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_Param] * nNumber - pCliInfo->metaData.osMetadata.nOffset;

            result.nFlags = _read_uint16_safe(pBuffer, nBufferSize, nOffset);
            nOffset += 2;
            result.nSequence = _read_uint16_safe(pBuffer, nBufferSize, nOffset);
            nOffset += 2;
            result.nName = pCliInfo->metaData.nStringIndexSize == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset) : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
        }
    }

    return result;
}

XCLIASSEMBLY_DEF::S_METADATA_TYPESPEC XCLIAssembly::getMetadataTypeSpec(CLI_INFO *pCliInfo, qint32 nNumber)
{
    XCLIASSEMBLY_DEF::S_METADATA_TYPESPEC result = {};

    if (pCliInfo->bValid) {
        qint32 nNumberOfRecords = pCliInfo->metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_TypeSpec];
        char *pBuffer = pCliInfo->metaData.baMetadata.data();
        qint32 nBufferSize = pCliInfo->metaData.baMetadata.size();

        if (nNumber < nNumberOfRecords) {
            qint64 nOffset = pCliInfo->metaData.Tables_TablesOffsets[XCLIASSEMBLY_DEF::metadata_TypeSpec] +
                             pCliInfo->metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_TypeSpec] * nNumber - pCliInfo->metaData.osMetadata.nOffset;

            result.nSignature =
                pCliInfo->metaData.nBLOBIndexSize == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset) : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
        }
    }

    return result;
}

XCLIASSEMBLY_DEF::S_METADATA_FIELD XCLIAssembly::getMetadataField(CLI_INFO *pCliInfo, qint32 nNumber)
{
    XCLIASSEMBLY_DEF::S_METADATA_FIELD result = {};

    if (pCliInfo->bValid) {
        qint32 nNumberOfRecords = pCliInfo->metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_Field];
        char *pBuffer = pCliInfo->metaData.baMetadata.data();
        qint32 nBufferSize = pCliInfo->metaData.baMetadata.size();

        if (nNumber < nNumberOfRecords) {
            qint64 nOffset = pCliInfo->metaData.Tables_TablesOffsets[XCLIASSEMBLY_DEF::metadata_Field] +
                             pCliInfo->metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_Field] * nNumber - pCliInfo->metaData.osMetadata.nOffset;

            result.nFlags = _read_uint16_safe(pBuffer, nBufferSize, nOffset);
            nOffset += 2;
            result.nName = pCliInfo->metaData.nStringIndexSize == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset) : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
            nOffset += pCliInfo->metaData.nStringIndexSize;
            result.nSignature =
                pCliInfo->metaData.nBLOBIndexSize == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset) : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
        }
    }

    return result;
}

XCLIASSEMBLY_DEF::S_METADATA_METHODIMPL XCLIAssembly::getMetadataMethodImpl(CLI_INFO *pCliInfo, qint32 nNumber)
{
    XCLIASSEMBLY_DEF::S_METADATA_METHODIMPL result = {};

    if (pCliInfo->bValid) {
        qint32 nNumberOfRecords = pCliInfo->metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_MethodImpl];
        char *pBuffer = pCliInfo->metaData.baMetadata.data();
        qint32 nBufferSize = pCliInfo->metaData.baMetadata.size();

        if (nNumber < nNumberOfRecords) {
            qint64 nOffset = pCliInfo->metaData.Tables_TablesOffsets[XCLIASSEMBLY_DEF::metadata_MethodImpl] +
                             pCliInfo->metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_MethodImpl] * nNumber - pCliInfo->metaData.osMetadata.nOffset;

            result.nClass =
                pCliInfo->metaData.nTypeDefOrRefSize == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset) : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
            nOffset += pCliInfo->metaData.nTypeDefOrRefSize;
            result.nMethodBody =
                pCliInfo->metaData.nMethodDefOrRefSize == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset) : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
            nOffset += pCliInfo->metaData.nMethodDefOrRefSize;
            result.nMethodDeclaration =
                pCliInfo->metaData.nMethodDefOrRefSize == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset) : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
        }
    }

    return result;
}

XCLIASSEMBLY_DEF::S_METADATA_ASSEMBLY XCLIAssembly::getMetadataAssembly(CLI_INFO *pCliInfo, qint32 nNumber)
{
    XCLIASSEMBLY_DEF::S_METADATA_ASSEMBLY result = {};

    if (pCliInfo->bValid) {
        qint32 nNumberOfRecords = pCliInfo->metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_Assembly];
        char *pBuffer = pCliInfo->metaData.baMetadata.data();
        qint32 nBufferSize = pCliInfo->metaData.baMetadata.size();

        if (nNumber < nNumberOfRecords) {
            qint64 nOffset = pCliInfo->metaData.Tables_TablesOffsets[XCLIASSEMBLY_DEF::metadata_Assembly] +
                             pCliInfo->metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_Assembly] * nNumber - pCliInfo->metaData.osMetadata.nOffset;

            result.nHashAlgId = _read_uint32_safe(pBuffer, nBufferSize, nOffset);
            nOffset += 4;
            result.nMajorVersion = _read_uint16_safe(pBuffer, nBufferSize, nOffset);
            nOffset += 2;
            result.nMinorVersion = _read_uint16_safe(pBuffer, nBufferSize, nOffset);
            nOffset += 2;
            result.nBuildNumber = _read_uint16_safe(pBuffer, nBufferSize, nOffset);
            nOffset += 2;
            result.nRevisionNumber = _read_uint16_safe(pBuffer, nBufferSize, nOffset);
            nOffset += 2;
            result.nFlags = _read_uint32_safe(pBuffer, nBufferSize, nOffset);
            nOffset += 4;
            result.nPublicKeyOrToken =
                pCliInfo->metaData.nBLOBIndexSize == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset) : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
            nOffset += pCliInfo->metaData.nBLOBIndexSize;
            result.nName = pCliInfo->metaData.nStringIndexSize == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset) : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
            nOffset += pCliInfo->metaData.nStringIndexSize;
            result.nCulture =
                pCliInfo->metaData.nStringIndexSize == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset) : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
        }
    }

    return result;
}

XCLIASSEMBLY_DEF::S_METADATA_CONSTANT XCLIAssembly::getMetadataConstant(CLI_INFO *pCliInfo, qint32 nNumber)
{
    XCLIASSEMBLY_DEF::S_METADATA_CONSTANT result = {};

    if (pCliInfo->bValid) {
        qint32 nNumberOfRecords = pCliInfo->metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_Constant];
        char *pBuffer = pCliInfo->metaData.baMetadata.data();
        qint32 nBufferSize = pCliInfo->metaData.baMetadata.size();

        if (nNumber < nNumberOfRecords) {
            qint64 nOffset = pCliInfo->metaData.Tables_TablesOffsets[XCLIASSEMBLY_DEF::metadata_Constant] +
                             pCliInfo->metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_Constant] * nNumber - pCliInfo->metaData.osMetadata.nOffset;

            result.nType = _read_uint16_safe(pBuffer, nBufferSize, nOffset);
            nOffset += 2;
            result.nParent =
                pCliInfo->metaData.nHasConstantSize == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset) : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
            nOffset += pCliInfo->metaData.nHasConstantSize;
            result.nValue = pCliInfo->metaData.nBLOBIndexSize == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset) : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
        }
    }

    return result;
}

XCLIASSEMBLY_DEF::S_METADATA_CUSTOMATTRIBUTE XCLIAssembly::getMetadataCustomAttribute(CLI_INFO *pCliInfo, qint32 nNumber)
{
    XCLIASSEMBLY_DEF::S_METADATA_CUSTOMATTRIBUTE result = {};

    if (pCliInfo->bValid) {
        qint32 nNumberOfRecords = pCliInfo->metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_CustomAttribute];
        char *pBuffer = pCliInfo->metaData.baMetadata.data();
        qint32 nBufferSize = pCliInfo->metaData.baMetadata.size();

        if (nNumber < nNumberOfRecords) {
            qint64 nOffset = pCliInfo->metaData.Tables_TablesOffsets[XCLIASSEMBLY_DEF::metadata_CustomAttribute] +
                             pCliInfo->metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_CustomAttribute] * nNumber - pCliInfo->metaData.osMetadata.nOffset;

            result.nParent =
                pCliInfo->metaData.nHasCustomAttributeSize == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset) : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
            nOffset += pCliInfo->metaData.nHasCustomAttributeSize;
            result.nType =
                pCliInfo->metaData.nCustomAttributeTypeSize == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset) : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
            nOffset += pCliInfo->metaData.nCustomAttributeTypeSize;
            result.nValue = pCliInfo->metaData.nBLOBIndexSize == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset) : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
        }
    }

    return result;
}

XCLIASSEMBLY_DEF::S_METADATA_FIELDMARSHAL XCLIAssembly::getMetadataFieldMarshal(CLI_INFO *pCliInfo, qint32 nNumber)
{
    XCLIASSEMBLY_DEF::S_METADATA_FIELDMARSHAL result = {};

    if (pCliInfo->bValid) {
        qint32 nNumberOfRecords = pCliInfo->metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_FieldMarshal];
        char *pBuffer = pCliInfo->metaData.baMetadata.data();
        qint32 nBufferSize = pCliInfo->metaData.baMetadata.size();

        if (nNumber < nNumberOfRecords) {
            qint64 nOffset = pCliInfo->metaData.Tables_TablesOffsets[XCLIASSEMBLY_DEF::metadata_FieldMarshal] +
                             pCliInfo->metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_FieldMarshal] * nNumber - pCliInfo->metaData.osMetadata.nOffset;

            result.nParent =
                pCliInfo->metaData.nHasFieldMarshalSize == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset) : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
            nOffset += pCliInfo->metaData.nHasFieldMarshalSize;
            result.nNativeType =
                pCliInfo->metaData.nBLOBIndexSize == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset) : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
        }
    }

    return result;
}

XCLIASSEMBLY_DEF::S_METADATA_DECLSECURITY XCLIAssembly::getMetadataDeclSecurity(CLI_INFO *pCliInfo, qint32 nNumber)
{
    XCLIASSEMBLY_DEF::S_METADATA_DECLSECURITY result = {};

    if (pCliInfo->bValid) {
        qint32 nNumberOfRecords = pCliInfo->metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_DeclSecurity];
        char *pBuffer = pCliInfo->metaData.baMetadata.data();
        qint32 nBufferSize = pCliInfo->metaData.baMetadata.size();

        if (nNumber < nNumberOfRecords) {
            qint64 nOffset = pCliInfo->metaData.Tables_TablesOffsets[XCLIASSEMBLY_DEF::metadata_DeclSecurity] +
                             pCliInfo->metaData.Tables_TableElementSizes[XCLIASSEMBLY_DEF::metadata_DeclSecurity] * nNumber - pCliInfo->metaData.osMetadata.nOffset;

            result.nAction = _read_uint16_safe(pBuffer, nBufferSize, nOffset);
            nOffset += 2;
            result.nParent =
                pCliInfo->metaData.nHasDeclSecuritySize == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset) : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
            nOffset += pCliInfo->metaData.nHasDeclSecuritySize;
            result.nPermissionSet =
                pCliInfo->metaData.nBLOBIndexSize == 4 ? _read_uint32_safe(pBuffer, nBufferSize, nOffset) : _read_uint16_safe(pBuffer, nBufferSize, nOffset);
        }
    }

    return result;
}

QString XCLIAssembly::getMetadataModuleName(CLI_INFO *pCliInfo, qint32 nNumber)
{
    return _read_ansiString_safe(pCliInfo->metaData.baStrings.data(), pCliInfo->metaData.baStrings.size(), getMetadataModule(pCliInfo, nNumber).nName);
}

QString XCLIAssembly::getMetadataAssemblyName(CLI_INFO *pCliInfo, qint32 nNumber)
{
    return _read_ansiString_safe(pCliInfo->metaData.baStrings.data(), pCliInfo->metaData.baStrings.size(), getMetadataAssembly(pCliInfo, nNumber).nName);
}

XCLIASSEMBLY_DEF::S_METADATA_METHODDEFORREF XCLIAssembly::getMetadataMethodDefOrRef(CLI_INFO *pCliInfo, quint32 nValue)
{
    XCLIASSEMBLY_DEF::S_METADATA_METHODDEFORREF result = {};

    if (pCliInfo->bValid) {
        result.nTag = nValue & 0x1;
        result.nIndex = nValue >> 1;

        if (result.nTag == XCLIASSEMBLY_DEF::S_METADATA_METHODDEFORREF_METHODDEF) {
            result.record.methoddef = getMetadataMethodDef(pCliInfo, result.nIndex);
        } else if (result.nTag == XCLIASSEMBLY_DEF::S_METADATA_METHODDEFORREF_MEMBERREF) {
            result.record.memberref = getMetadataMemberRef(pCliInfo, result.nIndex);
        }
    }

    return result;
}

QString XCLIAssembly::getMetadataMemberRefParentName(CLI_INFO *pCliInfo, const XCLIASSEMBLY_DEF::S_METADATA_MEMBERREF &memberRef)
{
    QString sResult;

    quint32 nIndex = (memberRef.nClass >> 3) - 1;
    quint32 nTag = (memberRef.nClass & 0x7);

    if (nTag == 0) {
        if (nIndex < pCliInfo->metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_TypeDef]) {
            XCLIASSEMBLY_DEF::S_METADATA_TYPEDEF typeDef = getMetadataTypeDef(pCliInfo, nIndex);

            sResult = _read_ansiString_safe(pCliInfo->metaData.baStrings.data(), pCliInfo->metaData.baStrings.size(), typeDef.nTypeName);
        }
    } else if (nTag == 1) {
        if (nIndex < pCliInfo->metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_TypeRef]) {
            XCLIASSEMBLY_DEF::S_METADATA_TYPEREF typeRef = getMetadataTypeRef(pCliInfo, nIndex);

            sResult = _read_ansiString_safe(pCliInfo->metaData.baStrings.data(), pCliInfo->metaData.baStrings.size(), typeRef.nTypeName);
        }
    } else if (nTag == 2) {
        if (nIndex < pCliInfo->metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_ModuleRef]) {
            XCLIASSEMBLY_DEF::S_METADATA_MODULEREF moduleRef = getMetadataModuleRef(pCliInfo, nIndex);

            sResult = _read_ansiString_safe(pCliInfo->metaData.baStrings.data(), pCliInfo->metaData.baStrings.size(), moduleRef.nName);
        }
    } else if (nTag == 3) {
        if (nIndex < pCliInfo->metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_MethodDef]) {
            XCLIASSEMBLY_DEF::S_METADATA_METHODDEF methodDef = getMetadataMethodDef(pCliInfo, nIndex);

            sResult = _read_ansiString_safe(pCliInfo->metaData.baStrings.data(), pCliInfo->metaData.baStrings.size(), methodDef.nName);
        }
    } else if (nTag == 4) {
        if (nIndex < pCliInfo->metaData.Tables_TablesNumberOfIndexes[XCLIASSEMBLY_DEF::metadata_TypeSpec]) {
            XCLIASSEMBLY_DEF::S_METADATA_TYPESPEC typeSpec = getMetadataTypeSpec(pCliInfo, nIndex);

            sResult = QString("BLOB[%1]").arg(typeSpec.nSignature);
        }
    } else {
        sResult = tr("Unknown");
    }

    return sResult;
}

QString XCLIAssembly::mdtIdToString(quint32 nID)
{
    QString sResult;

    switch (nID) {
        case 0x00: sResult = "Module"; break;
        case 0x01: sResult = "TypeRef"; break;
        case 0x02: sResult = "TypeDef"; break;
        case 0x04: sResult = "Field"; break;
        case 0x05: sResult = "MethodPtr"; break;
        case 0x06: sResult = "MethodDef"; break;
        case 0x07: sResult = "ParamPtr"; break;
        case 0x08: sResult = "Param"; break;
        case 0x09: sResult = "InterfaceImpl"; break;
        case 0x0A: sResult = "MemberRef"; break;
        case 0x0B: sResult = "Constant"; break;
        case 0x0C: sResult = "CustomAttribute"; break;
        case 0x0D: sResult = "FieldMarshal"; break;
        case 0x0E: sResult = "DeclSecurity"; break;
        case 0x0F: sResult = "ClassLayout"; break;
        case 0x10: sResult = "FieldLayout"; break;
        case 0x11: sResult = "StandAloneSig"; break;
        case 0x12: sResult = "EventMap"; break;
        case 0x13: sResult = "EventPtr"; break;
        case 0x14: sResult = "Event"; break;
        case 0x15: sResult = "PropertyMap"; break;
        case 0x16: sResult = "PropertyPtr"; break;
        case 0x17: sResult = "Property"; break;
        case 0x18: sResult = "MethodSemantics"; break;
        case 0x19: sResult = "MethodImpl"; break;
        case 0x1A: sResult = "ModuleRef"; break;
        case 0x1B: sResult = "TypeSpec"; break;
        case 0x1C: sResult = "ImplMap"; break;
        case 0x1D: sResult = "FieldRVA"; break;
        case 0x1E: sResult = "ENCLog"; break;
        case 0x1F: sResult = "ENCMap"; break;
        case 0x20: sResult = "Assembly"; break;
        case 0x21: sResult = "AssemblyProcessor"; break;
        case 0x22: sResult = "AssemblyOS"; break;
        case 0x23: sResult = "AssemblyRef"; break;
        case 0x24: sResult = "AssemblyRefProcessor"; break;
        case 0x25: sResult = "AssemblyRefOS"; break;
        case 0x26: sResult = "File"; break;
        case 0x27: sResult = "ExportedType"; break;
        case 0x28: sResult = "ManifestResource"; break;
        case 0x29: sResult = "NestedClass"; break;
        case 0x2A: sResult = "GenericParam"; break;
        case 0x2B: sResult = "MethodSpec"; break;
        case 0x2C: sResult = "GenericParamConstraint"; break;
        case 0x2D: sResult = "Reserved 2D"; break;
        case 0x2E: sResult = "Reserved 2E"; break;
        case 0x2F: sResult = "Reserved 2F"; break;
        case 0x30: sResult = "Document"; break;
        case 0x31: sResult = "MethodDebugInformation"; break;
        case 0x32: sResult = "LocalScope"; break;
        case 0x33: sResult = "LocalVariable"; break;
        case 0x34: sResult = "LocalConstant"; break;
        case 0x35: sResult = "ImportScope"; break;
        case 0x36: sResult = "StateMachineMethod"; break;
        case 0x37: sResult = "CustomDebugInformation"; break;
        case 0x38: sResult = "Reserved 38"; break;
        case 0x39: sResult = "Reserved 39"; break;
        case 0x3A: sResult = "Reserved 3A"; break;
        case 0x3B: sResult = "Reserved 3B"; break;
        case 0x3C: sResult = "Reserved 3C"; break;
        case 0x3D: sResult = "Reserved 3D"; break;
        case 0x3E: sResult = "Reserved 3E"; break;
        case 0x3F: sResult = "Reserved 3F"; break;

        default: sResult = tr("Unknown"); break;
    }

    return sResult;
}

XCLIAssembly::CLI_METADATA_HEADER XCLIAssembly::_read_MetadataHeader(qint64 nOffset)
{
    CLI_METADATA_HEADER result = {};

    result.nSignature = read_uint32(nOffset);
    result.nMajorVersion = read_uint16(nOffset + 4);
    result.nMinorVersion = read_uint16(nOffset + 6);
    result.nReserved = read_uint32(nOffset + 8);
    result.nVersionStringLength = read_uint32(nOffset + 12);
    result.sVersion = read_ansiString(nOffset + 16, result.nVersionStringLength);
    result.nFlags = read_uint16(nOffset + 16 + result.nVersionStringLength);
    result.nStreams = read_uint16(nOffset + 16 + result.nVersionStringLength + 2);

    if (result.sVersion.size() > 20) {
        result.sVersion = "";
    }

    return result;
}

qint64 XCLIAssembly::findSignatureInBlob_NET(const QString &sSignature, PDSTRUCT *pPdStruct)
{
    _MEMORY_MAP memoryMap = getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);
    CLI_INFO clinfo = getCliInfo(true, pPdStruct);

    return find_signature(&memoryMap, clinfo.metaData.osBlob.nOffset, clinfo.metaData.osBlob.nSize, sSignature, nullptr, pPdStruct);
}

bool XCLIAssembly::isSignatureInBlobPresent_NET(const QString &sSignature, PDSTRUCT *pPdStruct)
{
    return (findSignatureInBlob_NET(sSignature, pPdStruct) != -1);
}

bool XCLIAssembly::handleInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XCLIAssembly> guardedThis(this);
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

void *XCLIAssembly::getInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XCLIAssembly> guardedThis(this);
    const bool bHandled = guardedThis->handleInternalInfo(pPdStruct);
    if (!guardedThis || !bHandled) return nullptr;

    return &guardedThis->m_internalInfo;
}

void XCLIAssembly::setInternalInfo(void *pInternalInfo)
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
