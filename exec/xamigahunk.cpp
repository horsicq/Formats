/* Copyright (c) 2024 hors<horsicq@gmail.com>
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
#include "xamigahunk.h"

XAmigaHunk::XAmigaHunk(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress) : XBinary(pDevice, bIsImage, nModuleAddress)
{
}

XAmigaHunk::~XAmigaHunk()
{
}

bool XAmigaHunk::isValid(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    bool bResult = false;

    if (getSize() > 8) {
        quint32 nMagic = read_uint32(0, true);

        if ((nMagic == XAMIGAHUNK_DEF::HUNK_HEADER) || (nMagic == XAMIGAHUNK_DEF::HUNK_UNIT)) {
            bResult = true;
        }
    }

    return bResult;
}

QList<XBinary::MAPMODE> XAmigaHunk::getMapModesList()
{
    QList<XBinary::MAPMODE> listResult;

    listResult.append(XBinary::MAPMODE_SEGMENTS);
    listResult.append(XBinary::MAPMODE_REGIONS);

    return listResult;
}

XBinary::_MEMORY_MAP XAmigaHunk::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    XBinary::PDSTRUCT pdStructEmpty = {};

    if (!pPdStruct) {
        pdStructEmpty = XBinary::createPdStruct();
        pPdStruct = &pdStructEmpty;
    }

    if (mapMode == MAPMODE_UNKNOWN) {
        mapMode = MAPMODE_SEGMENTS;
    }

    QList<HUNK> listHunks = getHunks(pPdStruct);

    XBinary::_MEMORY_MAP result = {};
    qint64 nTotalSize = getSize();

    result.nModuleAddress = getModuleAddress();
    result.nBinarySize = nTotalSize;

    result.fileType = getFileType();
    result.mode = getMode(&listHunks);
    result.sArch = getArch(&listHunks);
    result.endian = getEndian();
    result.sType = getTypeAsString();

    qint32 nIndex = 0;

    qint32 nNumberOfHunks = listHunks.count();

    if (mapMode == XBinary::MAPMODE_REGIONS) {
        for (qint32 i = 0; (i < nNumberOfHunks) && (!(pPdStruct->bIsStop)); i++) {
            HUNK hunk = listHunks.at(i);
            _MEMORY_RECORD record = {};
            record.nIndex = nIndex++;
            record.type = MMT_FILESEGMENT;
            record.nOffset = hunk.nOffset;
            record.nSize = hunk.nSize;
            record.nAddress = hunk.nOffset;
            record.sName = hunkTypeToString(hunk.nId);

            result.listRecords.append(record);

            if ((hunk.nId == XAMIGAHUNK_DEF::HUNK_CODE) || (hunk.nId == XAMIGAHUNK_DEF::HUNK_PPC_CODE)) {
                result.nEntryPointAddress = record.nAddress + 8;
            }
        }

        result.nImageSize = nTotalSize;
    } else if (mapMode == XBinary::MAPMODE_SEGMENTS) {
        result.nModuleAddress = XAMIGAHUNK_DEF::IMAGE_BASE;
        XADDR nCurrentAddress = result.nModuleAddress;

        for (qint32 i = 0; (i < nNumberOfHunks) && (!(pPdStruct->bIsStop)); i++) {
            HUNK hunk = listHunks.at(i);

            if ((hunk.nId == XAMIGAHUNK_DEF::HUNK_CODE) || (hunk.nId == XAMIGAHUNK_DEF::HUNK_DATA) || (hunk.nId == XAMIGAHUNK_DEF::HUNK_PPC_CODE) ||
                (hunk.nId == XAMIGAHUNK_DEF::HUNK_BSS)) {
                {
                    _MEMORY_RECORD record = {};
                    record.nIndex = nIndex++;
                    record.type = MMT_NOLOADABLE;
                    record.nOffset = hunk.nOffset;
                    record.nSize = 8;
                    record.nAddress = -1;
                    record.sName = hunkTypeToString(hunk.nId);

                    result.listRecords.append(record);
                }
                {
                    _MEMORY_RECORD record = {};
                    record.nIndex = nIndex++;
                    record.type = MMT_LOADSEGMENT;
                    record.nSize = hunk.nSize - 8;
                    record.nAddress = nCurrentAddress;

                    if (record.nSize) {
                        record.nOffset = hunk.nOffset + 8;
                    } else {
                        record.bIsVirtual = true;

                        if (hunk.nId == XAMIGAHUNK_DEF::HUNK_BSS) {
                            quint32 nBSS = read_uint32(hunk.nOffset + 4, true);
                            record.nSize = nBSS * 4;
                            record.nOffset = -1;
                        }
                    }

                    record.sName = hunkTypeToString(hunk.nId);

                    result.listRecords.append(record);

                    if ((hunk.nId == XAMIGAHUNK_DEF::HUNK_CODE) || (hunk.nId == XAMIGAHUNK_DEF::HUNK_PPC_CODE)) {
                        result.nEntryPointAddress = record.nAddress;
                    }

                    nCurrentAddress += record.nSize;
                }
            } else {
                _MEMORY_RECORD record = {};
                record.nIndex = nIndex++;
                record.type = MMT_FILESEGMENT;
                record.nOffset = hunk.nOffset;
                record.nSize = hunk.nSize;
                record.nAddress = -1;
                record.sName = hunkTypeToString(hunk.nId);

                result.listRecords.append(record);
            }
        }

        result.nImageSize = nCurrentAddress - result.nModuleAddress;
    }

    return result;
}

XBinary::ENDIAN XAmigaHunk::getEndian()
{
    return ENDIAN_BIG;
}

QString XAmigaHunk::getArch()
{
    QList<HUNK> listHunks = getHunks();

    return getArch(&listHunks);
}

QString XAmigaHunk::getArch(QList<HUNK> *pListHunks)
{
    QString sResult = "68K";

    if (isHunkPresent(pListHunks, XAMIGAHUNK_DEF::HUNK_PPC_CODE)) {
        sResult = "PPC";
    }

    return sResult;
}

XBinary::MODE XAmigaHunk::getMode()
{
    QList<HUNK> listHunks = getHunks();

    return getMode(&listHunks);
}

XBinary::MODE XAmigaHunk::getMode(QList<HUNK> *pListHunks)
{
    XBinary::MODE result = MODE_16;

    if (isHunkPresent(pListHunks, XAMIGAHUNK_DEF::HUNK_RELOC32)) {
        result = MODE_32;
    }

    return result;
}

QList<XAmigaHunk::HUNK> XAmigaHunk::getHunks(PDSTRUCT *pPdStruct)
{
    XBinary::PDSTRUCT pdStructEmpty = {};

    if (!pPdStruct) {
        pdStructEmpty = XBinary::createPdStruct();
        pPdStruct = &pdStructEmpty;
    }

    QList<HUNK> listResult;

    qint64 nCurrentOffset = 0;
    qint64 nTotalSize = getSize();
    bool bStop = false;

    while (!(pPdStruct->bIsStop)) {
        HUNK record = {};
        record.nId = read_uint32(nCurrentOffset, true);
        record.nOffset = nCurrentOffset;

        nCurrentOffset += 4;

        if (record.nId == XAMIGAHUNK_DEF::HUNK_HEADER) {
            quint32 nEndOfList = read_uint32(nCurrentOffset, true);

            if (nEndOfList) {
                // Resident library names
                // QString sName = read_ansiString(nCurrentOffset);
            } else {
                nCurrentOffset += 4;
            }
            quint32 nTableSize = read_uint32(nCurrentOffset + 0, true);
            // quint32 nFirstLoaded = read_uint32(nCurrentOffset + 4, true);
            // quint32 nLastLoaded = read_uint32(nCurrentOffset + 8, true);

            nCurrentOffset += 12;

            QList<qint64> listSizes;

            nTableSize = qMin(nTableSize, (quint32)100);

            for (quint32 i = 0; i < nTableSize; i++) {
                listSizes.append(read_uint32(nCurrentOffset, true));
                nCurrentOffset += 4;
            }
        } else if ((record.nId == XAMIGAHUNK_DEF::HUNK_CODE) || (record.nId == XAMIGAHUNK_DEF::HUNK_DATA) || (record.nId == XAMIGAHUNK_DEF::HUNK_PPC_CODE)) {
            quint32 nCodeSize = read_uint32(nCurrentOffset, true);
            nCurrentOffset += 4;
            nCurrentOffset += (nCodeSize * 4);
        } else if (record.nId == XAMIGAHUNK_DEF::HUNK_RELOC32) {
            while (!(pPdStruct->bIsStop)) {
                quint32 nRelocSize = read_uint32(nCurrentOffset, true);
                nCurrentOffset += 4;

                if (nRelocSize == 0) {
                    break;
                }

                // quint32 nHunk = read_uint32(nCurrentOffset + 4, true);
                nCurrentOffset += 4;
                nCurrentOffset += (nRelocSize * 4);
            }
        } else if (record.nId == XAMIGAHUNK_DEF::HUNK_BSS) {
            nCurrentOffset += 4;
        } else if (record.nId == XAMIGAHUNK_DEF::HUNK_DEBUG) {
            quint32 nDebugSize = read_uint32(nCurrentOffset, true);
            nCurrentOffset += 4;
            nCurrentOffset += (nDebugSize * 4);
        } else if (record.nId == XAMIGAHUNK_DEF::HUNK_SYMBOL) {
            while (!(pPdStruct->bIsStop)) {
                quint32 nSymbolSize = read_uint32(nCurrentOffset, true);
                nCurrentOffset += 4;

                if (nSymbolSize == 0) {
                    break;
                }

                // QString sName = read_ansiString(nCurrentOffset);
                nCurrentOffset += nSymbolSize * 4;
                // quint32 nSymbolOffset = read_uint32(nCurrentOffset, true);
                nCurrentOffset += 4;
            }
        } else if (record.nId == XAMIGAHUNK_DEF::HUNK_END) {
        } else {
            bStop = true;
        }

        record.nSize = nCurrentOffset - record.nOffset;

        listResult.append(record);

        if (bStop) {
            break;
        }

        if (nCurrentOffset >= nTotalSize) {
            break;
        }
    }

    return listResult;
}

QString XAmigaHunk::hunkTypeToString(quint32 nHunkType)
{
    QString sResult;

    switch (nHunkType) {
        case XAMIGAHUNK_DEF::HUNK_UNIT: sResult = "HUNK_UNIT"; break;
        case XAMIGAHUNK_DEF::HUNK_NAME: sResult = "HUNK_NAME"; break;
        case XAMIGAHUNK_DEF::HUNK_CODE: sResult = "HUNK_CODE"; break;
        case XAMIGAHUNK_DEF::HUNK_DATA: sResult = "HUNK_DATA"; break;
        case XAMIGAHUNK_DEF::HUNK_BSS: sResult = "HUNK_BSS"; break;
        case XAMIGAHUNK_DEF::HUNK_RELOC32: sResult = "HUNK_RELOC32"; break;
        case XAMIGAHUNK_DEF::HUNK_RELOC16: sResult = "HUNK_RELOC16"; break;
        case XAMIGAHUNK_DEF::HUNK_RELOC8: sResult = "HUNK_RELOC8"; break;
        case XAMIGAHUNK_DEF::HUNK_EXT: sResult = "HUNK_EXT"; break;
        case XAMIGAHUNK_DEF::HUNK_SYMBOL: sResult = "HUNK_SYMBOL"; break;
        case XAMIGAHUNK_DEF::HUNK_DEBUG: sResult = "HUNK_DEBUG"; break;
        case XAMIGAHUNK_DEF::HUNK_END: sResult = "HUNK_END"; break;
        case XAMIGAHUNK_DEF::HUNK_HEADER: sResult = "HUNK_HEADER"; break;
        case XAMIGAHUNK_DEF::HUNK_OVERLAY: sResult = "HUNK_OVERLAY"; break;
        case XAMIGAHUNK_DEF::HUNK_BREAK: sResult = "HUNK_BREAK"; break;
        case XAMIGAHUNK_DEF::HUNK_DREL32: sResult = "HUNK_DREL32"; break;
        case XAMIGAHUNK_DEF::HUNK_DREL16: sResult = "HUNK_DREL16"; break;
        case XAMIGAHUNK_DEF::HUNK_DREL8: sResult = "HUNK_DREL8"; break;
        case XAMIGAHUNK_DEF::HUNK_LIB: sResult = "HUNK_LIB"; break;
        case XAMIGAHUNK_DEF::HUNK_INDEX: sResult = "HUNK_INDEX"; break;
        case XAMIGAHUNK_DEF::HUNK_RELOC32SHORT: sResult = "HUNK_RELOC32SHORT"; break;
        case XAMIGAHUNK_DEF::HUNK_RELRELOC32: sResult = "HUNK_RELRELOC32"; break;
        case XAMIGAHUNK_DEF::HUNK_ABSRELOC16: sResult = "HUNK_ABSRELOC16"; break;
        case XAMIGAHUNK_DEF::HUNK_DREL32EXE: sResult = "HUNK_DREL32EXE"; break;
        case XAMIGAHUNK_DEF::HUNK_PPC_CODE: sResult = "HUNK_PPC_CODE"; break;
        case XAMIGAHUNK_DEF::HUNK_RELRELOC26: sResult = "HUNK_RELRELOC26"; break;
        default: sResult = QString("HUNK_%1").arg(nHunkType, 0, 16); break;
    }

    return sResult;
}

bool XAmigaHunk::isHunkPresent(QList<HUNK> *pListHunks, quint32 nHunkType)
{
    bool bResult = false;

    qint32 nNumberOfHunks = pListHunks->count();

    for (int i = 0; i < nNumberOfHunks; i++) {
        if (pListHunks->at(i).nId == nHunkType) {
            bResult = true;
            break;
        }
    }

    return bResult;
}

QList<XAmigaHunk::HUNK> XAmigaHunk::_getHunksByType(QList<HUNK> *pListHunks, quint32 nHunkType)
{
    QList<HUNK> listResult;

    qint32 nNumberOfHunks = pListHunks->count();

    for (int i = 0; i < nNumberOfHunks; i++) {
        if (pListHunks->at(i).nId == nHunkType) {
            listResult.append(pListHunks->at(i));
        }
    }

    return listResult;
}

XBinary::FT XAmigaHunk::getFileType()
{
    return FT_AMIGAHUNK;
}

XBinary::OSINFO XAmigaHunk::getOsInfo()
{
    OSINFO result = {};

    QList<HUNK> listHunks = getHunks();

    result.osName = OSNAME_AMIGA;
    // result.sOsVersion = "";
    result.sArch = getArch(&listHunks);
    result.mode = getMode(&listHunks);
    result.sType = getTypeAsString();
    result.endian = getEndian();

    return result;
}

QString XAmigaHunk::getFileFormatExt()
{
    return "";
}

qint64 XAmigaHunk::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    qint64 nResult = 0;
    QList<HUNK> listHunks = getHunks(pPdStruct);

    QList<HUNK> listEndHunks = _getHunksByType(&listHunks, XAMIGAHUNK_DEF::HUNK_END);

    qint32 nNumberOfHunks = listEndHunks.count();

    for (qint32 i = 0; i < nNumberOfHunks; i++) {
        nResult = qMax(nResult, listEndHunks.at(i).nOffset + listEndHunks.at(i).nSize);
    }

    return nResult;
}

QString XAmigaHunk::getFileFormatString()
{
    QString sResult;

    sResult = QString("Amiga Hunk(%1)").arg(getArch());

    return sResult;
}

XBinary::FILEFORMATINFO XAmigaHunk::getFileFormatInfo(PDSTRUCT *pPdStruct)
{
    FILEFORMATINFO result = {};

    result.bIsValid = isValid(pPdStruct);

    if (result.bIsValid) {
        result.nSize = getFileFormatSize(pPdStruct);

        if (result.nSize > 0) {
            result.fileType = getFileType();
            result.sString = getFileFormatString();
            result.sExt = getFileFormatExt();
            result.sVersion = getVersion();
            result.sOptions = getOptions();
        } else {
            result.bIsValid = false;
        }
    }

    return result;
}

qint32 XAmigaHunk::getType()
{
    qint32 nResult = TYPE_EXECUTABLE;

    quint32 nMagic = read_uint32(0, true);

    if (nMagic == XAMIGAHUNK_DEF::HUNK_HEADER) {
        nResult = TYPE_EXECUTABLE;
    } else if (nMagic == XAMIGAHUNK_DEF::HUNK_UNIT) {
        nResult = TYPE_OBJECT;
    }

    return nResult;
}

QString XAmigaHunk::typeIdToString(qint32 nType)
{
    QString sResult = tr("Unknown");

    switch (nType) {
        case TYPE_UNKNOWN: sResult = tr("Unknown"); break;
        case TYPE_EXECUTABLE: sResult = QString("EXE"); break;
        // case TYPE_LIBRARY: sResult = tr("Library"); break;
        case TYPE_OBJECT: sResult = tr("Object"); break;
    }

    return sResult;
}
