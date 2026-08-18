/* Copyright (c) 2024-2026 hors<horsicq@gmail.com>
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

static XBinary::XCONVERT _TABLE_XAmigaHunk_STRUCTID[] = {
    {XAmigaHunk::STRUCTID_UNKNOWN, "Unknown", QObject::tr("Unknown")},
    {XAmigaHunk::STRUCTID_HUNK, "HUNK", QString("HUNK")},
    {XAmigaHunk::STRUCTID_HUNK_UNIT, "HUNK_UNIT", QString("HUNK_UNIT")},
    {XAmigaHunk::STRUCTID_HUNK_NAME, "HUNK_NAME", QString("HUNK_NAME")},
    {XAmigaHunk::STRUCTID_HUNK_CODE, "HUNK_CODE", QString("HUNK_CODE")},
    {XAmigaHunk::STRUCTID_HUNK_DATA, "HUNK_DATA", QString("HUNK_DATA")},
    {XAmigaHunk::STRUCTID_HUNK_BSS, "HUNK_BSS", QString("HUNK_BSS")},
    {XAmigaHunk::STRUCTID_HUNK_RELOC32, "HUNK_RELOC32", QString("HUNK_RELOC32")},
    {XAmigaHunk::STRUCTID_HUNK_RELOC16, "HUNK_RELOC16", QString("HUNK_RELOC16")},
    {XAmigaHunk::STRUCTID_HUNK_RELOC8, "HUNK_RELOC8", QString("HUNK_RELOC8")},
    {XAmigaHunk::STRUCTID_HUNK_EXT, "HUNK_EXT", QString("HUNK_EXT")},
    {XAmigaHunk::STRUCTID_HUNK_SYMBOL, "HUNK_SYMBOL", QString("HUNK_SYMBOL")},
    {XAmigaHunk::STRUCTID_HUNK_DEBUG, "HUNK_DEBUG", QString("HUNK_DEBUG")},
    {XAmigaHunk::STRUCTID_HUNK_END, "HUNK_END", QString("HUNK_END")},
    {XAmigaHunk::STRUCTID_HUNK_HEADER, "HUNK_HEADER", QString("HUNK_HEADER")},
    {XAmigaHunk::STRUCTID_HUNK_OVERLAY, "HUNK_OVERLAY", QString("HUNK_OVERLAY")},
    {XAmigaHunk::STRUCTID_HUNK_BREAK, "HUNK_BREAK", QString("HUNK_BREAK")},
    {XAmigaHunk::STRUCTID_HUNK_DREL32, "HUNK_DREL32", QString("HUNK_DREL32")},
    {XAmigaHunk::STRUCTID_HUNK_DREL16, "HUNK_DREL16", QString("HUNK_DREL16")},
    {XAmigaHunk::STRUCTID_HUNK_DREL8, "HUNK_DREL8", QString("HUNK_DREL8")},
    {XAmigaHunk::STRUCTID_HUNK_LIB, "HUNK_LIB", QString("HUNK_LIB")},
    {XAmigaHunk::STRUCTID_HUNK_INDEX, "HUNK_INDEX", QString("HUNK_INDEX")},
    {XAmigaHunk::STRUCTID_HUNK_RELOC32SHORT, "HUNK_RELOC32SHORT", QString("HUNK_RELOC32SHORT")},
    {XAmigaHunk::STRUCTID_HUNK_RELRELOC32, "HUNK_RELRELOC32", QString("HUNK_RELRELOC32")},
    {XAmigaHunk::STRUCTID_HUNK_ABSRELOC16, "HUNK_ABSRELOC16", QString("HUNK_ABSRELOC16")},
    {XAmigaHunk::STRUCTID_HUNK_DREL32EXE, "HUNK_DREL32EXE", QString("HUNK_DREL32EXE")},
    {XAmigaHunk::STRUCTID_HUNK_PPC_CODE, "HUNK_PPC_CODE", QString("HUNK_PPC_CODE")},
    {XAmigaHunk::STRUCTID_HUNK_RELRELOC26, "HUNK_RELRELOC26", QString("HUNK_RELRELOC26")},
};

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

bool XAmigaHunk::isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    XAmigaHunk xamigahunk(pDevice);

    return xamigahunk.isValid(pPdStruct);
}

bool XAmigaHunk::_initMemoryMap(_MEMORY_MAP *pMemoryMap, PDSTRUCT *pPdStruct)
{
    qint64 nTotalSize = getSize();

    QList<HUNK> listHunks = getHunks(pPdStruct);

    pMemoryMap->nModuleAddress = getModuleAddress();
    pMemoryMap->nBinarySize = nTotalSize;
    pMemoryMap->nImageSize = nTotalSize;  // TODO Check

    pMemoryMap->fileType = getFileType();
    pMemoryMap->mode = getMode(&listHunks, pPdStruct);
    pMemoryMap->sArch = getArch(&listHunks, pPdStruct);
    pMemoryMap->endian = getEndian();
    pMemoryMap->sType = getTypeAsString();
    pMemoryMap->nEntryPointAddress = _getEntryPointAddress(&listHunks, pPdStruct);

    return true;
}

QList<XBinary::MAPMODE> XAmigaHunk::getMapModesList()
{
    QList<XBinary::MAPMODE> list;
    // Standardize to regions view; segment view can be derived later if needed
    list.append(MAPMODE_SEGMENTS);
    list.append(MAPMODE_REGIONS);

    return list;
}

XBinary::_MEMORY_MAP XAmigaHunk::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    if (mapMode == MAPMODE_UNKNOWN) {
        mapMode = MAPMODE_SEGMENTS;
    }

    if (mapMode == MAPMODE_SEGMENTS) {
        return _getMemoryMap(FILEPART_SEGMENT, pPdStruct);
    } else {
        return _getMemoryMap(FILEPART_REGION | FILEPART_OVERLAY, pPdStruct);
    }
}

XBinary::ENDIAN XAmigaHunk::getEndian()
{
    return ENDIAN_BIG;
}

QString XAmigaHunk::getArch()
{
    QList<HUNK> listHunks = getHunks();

    return getArch(&listHunks, nullptr);
}

QString XAmigaHunk::getArch(QList<HUNK> *pListHunks, PDSTRUCT *pPdStruct)
{
    QString sResult = "68K";

    if (isHunkPresent(pListHunks, XAMIGAHUNK_DEF::HUNK_PPC_CODE, pPdStruct)) {
        sResult = "PPC";
    }

    return sResult;
}

XADDR XAmigaHunk::_getEntryPointAddress(QList<HUNK> *pListHunks, PDSTRUCT *pPdStruct)
{
    XADDR nResult = 0;

    qint32 nNumberOfHunks = pListHunks->count();

    for (qint32 i = 0; (i < nNumberOfHunks) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        if ((pListHunks->at(i).nId == XAMIGAHUNK_DEF::HUNK_CODE) || (pListHunks->at(i).nId == XAMIGAHUNK_DEF::HUNK_PPC_CODE)) {
            nResult = XAMIGAHUNK_DEF::IMAGE_BASE;
            break;
        }
    }

    return nResult;
}

qint64 XAmigaHunk::_getHunkSize(qint64 nOffset, PDSTRUCT *pPdStruct)
{
    qint64 nResult = 0;
    bool bStop = false;
    qint64 nCurrentOffset = nOffset;

    quint32 nId = read_uint32(nCurrentOffset, true) & 0x3FFFFFFF;

    nCurrentOffset += 4;

    if (nId == XAMIGAHUNK_DEF::HUNK_HEADER) {
        quint32 nEndOfList = read_uint32(nCurrentOffset, true);

        if (nEndOfList) {
            // Resident library names
            // QString sName = read_ansiString(nCurrentOffset);
        } else {
            nCurrentOffset += 4;
        }
        qint32 nTableSize = read_uint32(nCurrentOffset + 0, true);
        qint32 nFirstLoaded = read_int32(nCurrentOffset + 4, true);
        qint32 nLastLoaded = read_int32(nCurrentOffset + 8, true);

        nCurrentOffset += 12;

        QList<qint64> listSizes;

        nTableSize = qMin(nTableSize, (qint32)0xFF0);

        nTableSize = qMin(nTableSize, nLastLoaded - nFirstLoaded);

        for (qint32 i = 0; i <= nTableSize; i++) {
            listSizes.append(read_uint32(nCurrentOffset, true));
            nCurrentOffset += 4;
        }
    } else if ((nId == XAMIGAHUNK_DEF::HUNK_CODE) || (nId == XAMIGAHUNK_DEF::HUNK_DATA) || (nId == XAMIGAHUNK_DEF::HUNK_PPC_CODE)) {
        quint32 nCodeSize = read_uint32(nCurrentOffset, true);
        nCurrentOffset += 4;
        nCurrentOffset += (nCodeSize * 4);
    } else if ((nId == XAMIGAHUNK_DEF::HUNK_RELOC32) || (nId == XAMIGAHUNK_DEF::HUNK_RELOC16) || (nId == XAMIGAHUNK_DEF::HUNK_RELOC8) ||
               (nId == XAMIGAHUNK_DEF::HUNK_DREL32) || (nId == XAMIGAHUNK_DEF::HUNK_DREL16) || (nId == XAMIGAHUNK_DEF::HUNK_DREL8)) {
        while (XBinary::isPdStructNotCanceled(pPdStruct)) {
            quint32 nRelocSize = read_uint32(nCurrentOffset, true);
            nCurrentOffset += 4;

            if (nRelocSize == 0) {
                break;
            }

            // quint32 nHunk = read_uint32(nCurrentOffset + 4, true);
            nCurrentOffset += 4;
            nCurrentOffset += (nRelocSize * 4);
        }
    } else if ((nId == XAMIGAHUNK_DEF::HUNK_UNIT) || (nId == XAMIGAHUNK_DEF::HUNK_NAME)) {
        const quint32 nNameSize = read_uint32(nCurrentOffset, true);
        nCurrentOffset += 4 + (qint64)nNameSize * 4;
    } else if (nId == XAMIGAHUNK_DEF::HUNK_EXT) {
        qint32 nGuard = 0;

        while (XBinary::isPdStructNotCanceled(pPdStruct) && checkOffsetSize(nCurrentOffset, 4) && (nGuard++ < 0x10000)) {
            const quint32 nTypeLength = read_uint32(nCurrentOffset, true);
            nCurrentOffset += 4;
            if (nTypeLength == 0) {
                break;
            }

            const quint8 nType = (quint8)(nTypeLength >> 24);
            const quint32 nNameLongs = nTypeLength & 0x00FFFFFF;
            const qint64 nNameSize = (qint64)nNameLongs * 4;
            if (!checkOffsetSize(nCurrentOffset, nNameSize)) {
                bStop = true;
                break;
            }
            nCurrentOffset += nNameSize;

            if (nType <= 3) {  // EXT_SYMB/DEF/ABS/RES
                if (!checkOffsetSize(nCurrentOffset, 4)) {
                    bStop = true;
                    break;
                }
                nCurrentOffset += 4;
            } else if (nType >= 0x80) {  // EXT_REF* / EXT_COMMON
                if (nType == 0x82) {     // EXT_COMMON: common size precedes the reference count
                    if (!checkOffsetSize(nCurrentOffset, 4)) {
                        bStop = true;
                        break;
                    }
                    nCurrentOffset += 4;
                }

                if (!checkOffsetSize(nCurrentOffset, 4)) {
                    bStop = true;
                    break;
                }
                const quint32 nReferences = read_uint32(nCurrentOffset, true);
                nCurrentOffset += 4;
                if ((nReferences > 0x100000) || !checkOffsetSize(nCurrentOffset, (qint64)nReferences * 4)) {
                    bStop = true;
                    break;
                }
                nCurrentOffset += (qint64)nReferences * 4;
            } else {
                bStop = true;
                break;
            }
        }
    } else if (nId == XAMIGAHUNK_DEF::HUNK_BSS) {
        nCurrentOffset += 4;
    } else if (nId == XAMIGAHUNK_DEF::HUNK_DEBUG) {
        quint32 nDebugSize = read_uint32(nCurrentOffset, true);
        nCurrentOffset += 4;
        nCurrentOffset += (nDebugSize * 4);
    } else if (nId == XAMIGAHUNK_DEF::HUNK_SYMBOL) {
        while (XBinary::isPdStructNotCanceled(pPdStruct)) {
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
    } else if (nId == XAMIGAHUNK_DEF::HUNK_END) {
    } else {
        bStop = true;
    }

    nResult = nCurrentOffset - nOffset;

    if (bStop) {
        nResult = false;
    }

    return nResult;
}

XBinary::MODE XAmigaHunk::getMode()
{
    QList<HUNK> listHunks = getHunks();

    return getMode(&listHunks, nullptr);
}

XBinary::MODE XAmigaHunk::getMode(QList<HUNK> *pListHunks, PDSTRUCT *pPdStruct)
{
    XBinary::MODE result = MODE_16;

    if (isHunkPresent(pListHunks, XAMIGAHUNK_DEF::HUNK_RELOC32, pPdStruct)) {
        result = MODE_32;
    }

    return result;
}

QList<XAmigaHunk::HUNK> XAmigaHunk::getHunks(PDSTRUCT *pPdStruct)
{
    QList<HUNK> listResult;

    qint64 nCurrentOffset = 0;
    qint64 nTotalSize = getSize();

    while (XBinary::isPdStructNotCanceled(pPdStruct)) {
        HUNK record = {};
        record.nId = read_uint32(nCurrentOffset, true) & 0x3FFFFFFF;
        record.nOffset = nCurrentOffset;
        record.nSize = _getHunkSize(nCurrentOffset, pPdStruct);

        if (record.nSize == 0) {
            break;
        }

        listResult.append(record);

        nCurrentOffset += record.nSize;

        if (nCurrentOffset >= nTotalSize) {
            break;
        }
    }

    return listResult;
}

QVector<XBinary::XSYMBOL_STRUCT> XAmigaHunk::getSymbolStructs()
{
    QVector<XSYMBOL_STRUCT> listResult;
    const QList<HUNK> listHunks = getHunks();
    XADDR nCurrentAddress = XAMIGAHUNK_DEF::IMAGE_BASE;
    XADDR nCurrentHunkAddress = nCurrentAddress;

    for (qint32 i = 0; i < listHunks.count(); ++i) {
        const HUNK &hunk = listHunks.at(i);

        if ((hunk.nId == XAMIGAHUNK_DEF::HUNK_CODE) || (hunk.nId == XAMIGAHUNK_DEF::HUNK_DATA) || (hunk.nId == XAMIGAHUNK_DEF::HUNK_BSS) ||
            (hunk.nId == XAMIGAHUNK_DEF::HUNK_PPC_CODE)) {
            nCurrentHunkAddress = nCurrentAddress;
            const quint32 nLongwords = read_uint32(hunk.nOffset + 4, true);
            nCurrentAddress += (XADDR)align_up((XADDR)nLongwords * 4, 16);
            continue;
        }

        if (hunk.nId == XAMIGAHUNK_DEF::HUNK_SYMBOL) {
            qint64 nOffset = hunk.nOffset + 4;
            const qint64 nEnd = hunk.nOffset + hunk.nSize;
            qint32 nGuard = 0;

            while ((nOffset + 4 <= nEnd) && (nGuard++ < 0x10000)) {
                const qint64 nEntryOffset = nOffset;
                const quint32 nNameLongs = read_uint32(nOffset, true);
                nOffset += 4;
                if (nNameLongs == 0) {
                    break;
                }

                const qint64 nNameSize = (qint64)nNameLongs * 4;
                if ((nNameSize > nEnd - nOffset) || (nOffset + nNameSize > nEnd - 4)) {
                    break;
                }

                QByteArray baName = read_array(nOffset, nNameSize);
                const qint32 nZero = baName.indexOf('\0');
                if (nZero >= 0) baName.truncate(nZero);
                nOffset += nNameSize;
                const quint32 nValue = read_uint32(nOffset, true);
                nOffset += 4;

                XSYMBOL_STRUCT record = {};
                record.nOffset = nEntryOffset;
                record.nSize = nOffset - nEntryOffset;
                record.nAddress = nCurrentHunkAddress + nValue;
                record.sName = QString::fromLatin1(baName);
                record.symbolType = SYMBOL_TYPE_LABEL;
                if (!record.sName.isEmpty()) listResult.append(record);
            }
        } else if (hunk.nId == XAMIGAHUNK_DEF::HUNK_EXT) {
            qint64 nOffset = hunk.nOffset + 4;
            const qint64 nEnd = hunk.nOffset + hunk.nSize;
            qint32 nGuard = 0;

            while ((nOffset + 4 <= nEnd) && (nGuard++ < 0x10000)) {
                const qint64 nEntryOffset = nOffset;
                const quint32 nTypeLength = read_uint32(nOffset, true);
                nOffset += 4;
                if (nTypeLength == 0) {
                    break;
                }

                const quint8 nType = (quint8)(nTypeLength >> 24);
                const qint64 nNameSize = (qint64)(nTypeLength & 0x00FFFFFF) * 4;
                if ((nNameSize > nEnd - nOffset) || !checkOffsetSize(nOffset, nNameSize)) {
                    break;
                }

                QByteArray baName = read_array(nOffset, nNameSize);
                const qint32 nZero = baName.indexOf('\0');
                if (nZero >= 0) baName.truncate(nZero);
                nOffset += nNameSize;

                XSYMBOL_STRUCT record = {};
                record.nOffset = nEntryOffset;
                record.nAddress = (XADDR)-1;
                record.sName = QString::fromLatin1(baName);
                record.symbolType = SYMBOL_TYPE_UNKNOWN;

                if ((nType <= 3) && (nOffset + 4 <= nEnd)) {
                    const quint32 nValue = read_uint32(nOffset, true);
                    nOffset += 4;
                    record.nAddress = (nType == 2) ? nValue : nCurrentHunkAddress + nValue;
                    record.symbolType = (nType == 0) ? SYMBOL_TYPE_LABEL : SYMBOL_TYPE_EXPORT;
                } else if ((nType >= 0x80) && (nOffset + 4 <= nEnd)) {
                    if (nType == 0x82) {
                        nOffset += 4;
                    }
                    if (nOffset + 4 > nEnd) break;
                    const quint32 nReferences = read_uint32(nOffset, true);
                    nOffset += 4;
                    if ((nReferences > 0x100000) || ((qint64)nReferences * 4 > nEnd - nOffset)) break;
                    nOffset += (qint64)nReferences * 4;
                    record.symbolType = SYMBOL_TYPE_IMPORT;
                } else {
                    break;
                }

                record.nSize = nOffset - nEntryOffset;
                if (!record.sName.isEmpty() && (record.symbolType != SYMBOL_TYPE_UNKNOWN)) listResult.append(record);
            }
        }
    }

    return listResult;
}

QVector<XBinary::XIMPORT_STRUCT> XAmigaHunk::getImportStructs()
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

QVector<XBinary::XEXPORT_STRUCT> XAmigaHunk::getExportStructs()
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

XAmigaHunk::STRUCTID XAmigaHunk::hunkTypeToStructId(quint32 nHunkType)
{
    STRUCTID result = STRUCTID_UNKNOWN;

    switch (nHunkType) {
        case XAMIGAHUNK_DEF::HUNK_UNIT: result = STRUCTID_HUNK_UNIT; break;
        case XAMIGAHUNK_DEF::HUNK_NAME: result = STRUCTID_HUNK_NAME; break;
        case XAMIGAHUNK_DEF::HUNK_CODE: result = STRUCTID_HUNK_CODE; break;
        case XAMIGAHUNK_DEF::HUNK_DATA: result = STRUCTID_HUNK_DATA; break;
        case XAMIGAHUNK_DEF::HUNK_BSS: result = STRUCTID_HUNK_BSS; break;
        case XAMIGAHUNK_DEF::HUNK_RELOC32: result = STRUCTID_HUNK_RELOC32; break;
        case XAMIGAHUNK_DEF::HUNK_RELOC16: result = STRUCTID_HUNK_RELOC16; break;
        case XAMIGAHUNK_DEF::HUNK_RELOC8: result = STRUCTID_HUNK_RELOC8; break;
        case XAMIGAHUNK_DEF::HUNK_EXT: result = STRUCTID_HUNK_EXT; break;
        case XAMIGAHUNK_DEF::HUNK_SYMBOL: result = STRUCTID_HUNK_SYMBOL; break;
        case XAMIGAHUNK_DEF::HUNK_DEBUG: result = STRUCTID_HUNK_DEBUG; break;
        case XAMIGAHUNK_DEF::HUNK_END: result = STRUCTID_HUNK_END; break;
        case XAMIGAHUNK_DEF::HUNK_HEADER: result = STRUCTID_HUNK_HEADER; break;
        case XAMIGAHUNK_DEF::HUNK_OVERLAY: result = STRUCTID_HUNK_OVERLAY; break;
        case XAMIGAHUNK_DEF::HUNK_BREAK: result = STRUCTID_HUNK_BREAK; break;
        case XAMIGAHUNK_DEF::HUNK_DREL32: result = STRUCTID_HUNK_DREL32; break;
        case XAMIGAHUNK_DEF::HUNK_DREL16: result = STRUCTID_HUNK_DREL16; break;
        case XAMIGAHUNK_DEF::HUNK_DREL8: result = STRUCTID_HUNK_DREL8; break;
        case XAMIGAHUNK_DEF::HUNK_LIB: result = STRUCTID_HUNK_LIB; break;
        case XAMIGAHUNK_DEF::HUNK_INDEX: result = STRUCTID_HUNK_INDEX; break;
        case XAMIGAHUNK_DEF::HUNK_RELOC32SHORT: result = STRUCTID_HUNK_RELOC32SHORT; break;
        case XAMIGAHUNK_DEF::HUNK_RELRELOC32: result = STRUCTID_HUNK_RELRELOC32; break;
        case XAMIGAHUNK_DEF::HUNK_ABSRELOC16: result = STRUCTID_HUNK_ABSRELOC16; break;
        case XAMIGAHUNK_DEF::HUNK_DREL32EXE: result = STRUCTID_HUNK_DREL32EXE; break;
        case XAMIGAHUNK_DEF::HUNK_PPC_CODE: result = STRUCTID_HUNK_PPC_CODE; break;
        case XAMIGAHUNK_DEF::HUNK_RELRELOC26: result = STRUCTID_HUNK_RELRELOC26; break;
        default: result = STRUCTID_UNKNOWN;
    }

    return result;
}

bool XAmigaHunk::isHunkPresent(QList<HUNK> *pListHunks, quint32 nHunkType, PDSTRUCT *pPdStruct)
{
    bool bResult = false;

    qint32 nNumberOfHunks = pListHunks->count();

    for (qint32 i = 0; (i < nNumberOfHunks) && isPdStructNotCanceled(pPdStruct); i++) {
        if (pListHunks->at(i).nId == nHunkType) {
            bResult = true;
            break;
        }
    }

    return bResult;
}

qint64 XAmigaHunk::getHunksSize(QList<HUNK> *pListHunks, PDSTRUCT *pPdStruct)
{
    qint64 nResult = 0;

    qint32 nNumberOfHunks = pListHunks->count();

    for (qint32 i = 0; (i < nNumberOfHunks) && isPdStructNotCanceled(pPdStruct); i++) {
        nResult = qMax(nResult, pListHunks->at(i).nOffset + pListHunks->at(i).nSize);
    }

    return nResult;
}

QList<XAmigaHunk::HUNK> XAmigaHunk::_getHunksByType(QList<HUNK> *pListHunks, quint32 nHunkType, PDSTRUCT *pPdStruct)
{
    QList<HUNK> listResult;

    qint32 nNumberOfHunks = pListHunks->count();

    for (qint32 i = 0; (i < nNumberOfHunks) && isPdStructNotCanceled(pPdStruct); i++) {
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

QString XAmigaHunk::getFileFormatExt()
{
    return "";
}

QString XAmigaHunk::getFileFormatExtsString()
{
    return "Amiga Hunk Files (*.hunk *.o *.library *.a)";
}

qint64 XAmigaHunk::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    qint64 nResult = 0;
    QList<HUNK> listHunks = getHunks(pPdStruct);

    QList<HUNK> listEndHunks = _getHunksByType(&listHunks, XAMIGAHUNK_DEF::HUNK_END, pPdStruct);

    qint32 nNumberOfHunks = listEndHunks.count();

    for (qint32 i = 0; (i < nNumberOfHunks) && isPdStructNotCanceled(pPdStruct); i++) {
        nResult = qMax(nResult, listEndHunks.at(i).nOffset + listEndHunks.at(i).nSize);
    }

    return nResult;
}

XBinary::FILEFORMATINFO XAmigaHunk::getFileFormatInfo(PDSTRUCT *pPdStruct)
{
    FILEFORMATINFO result = {};

    result.bIsValid = isValid(pPdStruct);

    if (result.bIsValid) {
        result.nSize = getSize();
        result.fileType = getFileType();
        result.sExt = getFileFormatExt();
        result.sVersion = getVersion();
        result.sInfo = getInfo();

        QList<HUNK> listHunks = getHunks();

        result.osName = OSNAME_AMIGA;
        // result.sOsVersion = "";
        result.sArch = getArch(&listHunks, pPdStruct);
        result.mode = getMode(&listHunks, pPdStruct);
        result.sType = getTypeAsString();
        result.endian = getEndian();
        result.sMIME = getMIMEString();

        if (result.nSize == 0) {
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

QString XAmigaHunk::getMIMEString()
{
    return "application/x-amiga-hunk";
}

QString XAmigaHunk::structIDToString(quint32 nID)
{
    return XBinary::XCONVERT_idToTransString(nID, _TABLE_XAmigaHunk_STRUCTID, sizeof(_TABLE_XAmigaHunk_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QString XAmigaHunk::structIDToFtString(quint32 nID)
{
    return XBinary::XCONVERT_idToFtString(nID, _TABLE_XAmigaHunk_STRUCTID, sizeof(_TABLE_XAmigaHunk_STRUCTID) / sizeof(XBinary::XCONVERT));
}

quint32 XAmigaHunk::ftStringToStructID(const QString &sFtString)
{
    return XCONVERT_ftStringToId(sFtString, _TABLE_XAmigaHunk_STRUCTID, sizeof(_TABLE_XAmigaHunk_STRUCTID) / sizeof(XBinary::XCONVERT));
}

static XBinary::XIDSTRING _TABLE_XAmigaHunk_HunkIds[] = {{XAMIGAHUNK_DEF::HUNK_UNIT, "HUNK_UNIT"},
                                                         {XAMIGAHUNK_DEF::HUNK_NAME, "HUNK_NAME"},
                                                         {XAMIGAHUNK_DEF::HUNK_CODE, "HUNK_CODE"},
                                                         {XAMIGAHUNK_DEF::HUNK_DATA, "HUNK_DATA"},
                                                         {XAMIGAHUNK_DEF::HUNK_BSS, "HUNK_BSS"},
                                                         {XAMIGAHUNK_DEF::HUNK_RELOC32, "HUNK_RELOC32"},
                                                         {XAMIGAHUNK_DEF::HUNK_RELOC16, "HUNK_RELOC16"},
                                                         {XAMIGAHUNK_DEF::HUNK_RELOC8, "HUNK_RELOC8"},
                                                         {XAMIGAHUNK_DEF::HUNK_EXT, "HUNK_EXT"},
                                                         {XAMIGAHUNK_DEF::HUNK_SYMBOL, "HUNK_SYMBOL"},
                                                         {XAMIGAHUNK_DEF::HUNK_DEBUG, "HUNK_DEBUG"},
                                                         {XAMIGAHUNK_DEF::HUNK_END, "HUNK_END"},
                                                         {XAMIGAHUNK_DEF::HUNK_HEADER, "HUNK_HEADER"},
                                                         {XAMIGAHUNK_DEF::HUNK_OVERLAY, "HUNK_OVERLAY"},
                                                         {XAMIGAHUNK_DEF::HUNK_BREAK, "HUNK_BREAK"},
                                                         {XAMIGAHUNK_DEF::HUNK_DREL32, "HUNK_DREL32"},
                                                         {XAMIGAHUNK_DEF::HUNK_DREL16, "HUNK_DREL16"},
                                                         {XAMIGAHUNK_DEF::HUNK_DREL8, "HUNK_DREL8"},
                                                         {XAMIGAHUNK_DEF::HUNK_LIB, "HUNK_LIB"},
                                                         {XAMIGAHUNK_DEF::HUNK_INDEX, "HUNK_INDEX"},
                                                         {XAMIGAHUNK_DEF::HUNK_RELOC32SHORT, "HUNK_RELOC32SHORT"},
                                                         {XAMIGAHUNK_DEF::HUNK_RELRELOC32, "HUNK_RELRELOC32"},
                                                         {XAMIGAHUNK_DEF::HUNK_ABSRELOC16, "HUNK_ABSRELOC16"},
                                                         {XAMIGAHUNK_DEF::HUNK_DREL32EXE, "HUNK_DREL32EXE"},
                                                         {XAMIGAHUNK_DEF::HUNK_PPC_CODE, "HUNK_PPC_CODE"},
                                                         {XAMIGAHUNK_DEF::HUNK_RELRELOC26, "HUNK_RELRELOC26"}};

QList<XBinary::XFHEADER> XAmigaHunk::getXFHeaders(const XFSTRUCT &xfStruct, PDSTRUCT *pPdStruct)
{
    QList<XBinary::XFHEADER> listResult;

    quint32 nStructID = xfStruct.nStructID;

    if (nStructID == STRUCTID_UNKNOWN) {
        XFSTRUCT _xfStruct = xfStruct;
        _xfStruct.nStructID = STRUCTID_HUNK;
        _xfStruct.xLoc = offsetToLoc(0);
        listResult.append(getXFHeaders(_xfStruct, pPdStruct));
    } else if (nStructID == STRUCTID_HUNK) {
        QList<HUNK> listHunks = getHunks(pPdStruct);

        if (!listHunks.isEmpty()) {
            XFHEADER xfHeader = {};
            xfHeader.sParentTag = xfStruct.sParent;
            xfHeader.fileType = xfStruct.fileType;
            xfHeader.structID = static_cast<XBinary::STRUCTID>(STRUCTID_HUNK);
            xfHeader.xLoc = offsetToLoc(listHunks.first().nOffset);
            xfHeader.xfType = XFTYPE_TABLE;
            xfHeader.listFields = getXFRecords(xfStruct.fileType, STRUCTID_HUNK, xfHeader.xLoc);
            // Field 0 = Id
            xfHeader.listDataSt.append({0, 0, XFDATASTYPE_LIST, _TABLE_XAmigaHunk_HunkIds, sizeof(_TABLE_XAmigaHunk_HunkIds) / sizeof(XBinary::XIDSTRING)});

            qint32 nNumberOfHunks = listHunks.count();

            for (qint32 i = 0; i < nNumberOfHunks; i++) {
                xfHeader.listRowLocations.append(listHunks.at(i).nOffset);
            }

            xfHeader.sTag = xfHeaderToTag(xfHeader, structIDToString(STRUCTID_HUNK), xfHeader.sParentTag);
            listResult.append(xfHeader);
        }
    }

    return listResult;
}

QList<XBinary::XFRECORD> XAmigaHunk::getXFRecords(FT fileType, quint32 nStructID, const XLOC &xLoc)
{
    Q_UNUSED(fileType)
    Q_UNUSED(xLoc)

    QList<XBinary::XFRECORD> listResult;

    // Amiga Hunk files are big-endian
    if (nStructID == STRUCTID_HUNK) {
        listResult.append({"Id", 0, 4, XFRECORD_FLAG_BE, VT_UINT32});
    } else if ((nStructID == STRUCTID_HUNK_CODE) || (nStructID == STRUCTID_HUNK_DATA) || (nStructID == STRUCTID_HUNK_BSS) || (nStructID == STRUCTID_HUNK_PPC_CODE)) {
        listResult.append({"Id", 0, 4, XFRECORD_FLAG_BE, VT_UINT32});
        listResult.append({"NumLongs", 4, 4, XFRECORD_FLAG_BE | XFRECORD_FLAG_SIZE, VT_UINT32});
    } else if (nStructID == STRUCTID_HUNK_HEADER) {
        listResult.append({"Id", 0, 4, XFRECORD_FLAG_BE, VT_UINT32});
        listResult.append({"StringsSize", 4, 4, XFRECORD_FLAG_BE | XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"TableSize", 8, 4, XFRECORD_FLAG_BE | XFRECORD_FLAG_COUNT, VT_UINT32});
        listResult.append({"FirstHunk", 12, 4, XFRECORD_FLAG_BE, VT_UINT32});
        listResult.append({"LastHunk", 16, 4, XFRECORD_FLAG_BE, VT_UINT32});
    }

    return listResult;
}

bool XAmigaHunk::isExecutable()
{
    return true;
}

// qint32 XAmigaHunk::readTableRow(qint32 nRow, LT locType, XADDR nLocation, const DATA_RECORDS_OPTIONS &dataRecordsOptions, QList<DATA_RECORD_ROW> *pListDataRecords,
//                                 void *pUserData, PDSTRUCT *pPdStruct)
// {
//     qint32 nResult = 0;

//     if (dataRecordsOptions.dataHeaderFirst.dsID.nID == STRUCTID_HUNK) {
//         nResult = XBinary::readTableRow(nRow, locType, nLocation, dataRecordsOptions, pListDataRecords, pUserData, pPdStruct);

//         qint64 nStartOffset = locationToOffset(dataRecordsOptions.pMemoryMap, locType, nLocation);

//         qint64 nHunkSize = _getHunkSize(nStartOffset, pPdStruct);

//         if (nHunkSize > 0) {
//             nResult = (qint32)nHunkSize;
//         }
//     } else {
//         nResult = XBinary::readTableRow(nRow, locType, nLocation, dataRecordsOptions, pListDataRecords, pUserData, pPdStruct);
//     }

//     return nResult;
// }

// QList<XBinary::DATA_HEADER> XAmigaHunk::getDataHeaders(const DATA_HEADERS_OPTIONS &dataHeadersOptions, PDSTRUCT *pPdStruct)
// {
//     QList<XBinary::DATA_HEADER> listResult;

//     if (dataHeadersOptions.nID == STRUCTID_UNKNOWN) {
//         DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
//         _dataHeadersOptions.bChildren = true;
//         _dataHeadersOptions.dsID_parent = _addDefaultHeaders(&listResult, pPdStruct);
//         _dataHeadersOptions.dhMode = XBinary::DHMODE_TABLE;
//         _dataHeadersOptions.fileType = dataHeadersOptions.pMemoryMap->fileType;
//         _dataHeadersOptions.nID = STRUCTID_HUNK;
//         _dataHeadersOptions.nLocation = 0;
//         _dataHeadersOptions.locType = XBinary::LT_OFFSET;

//         QList<XAmigaHunk::HUNK> listHunks = getHunks(pPdStruct);

//         _dataHeadersOptions.nCount = listHunks.count();

//         listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//     } else {
//         qint64 nStartOffset = locationToOffset(dataHeadersOptions.pMemoryMap, dataHeadersOptions.locType, dataHeadersOptions.nLocation);

//         if (nStartOffset != -1) {
//             if (dataHeadersOptions.nID == STRUCTID_HUNK) {
//                 XBinary::DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, XAmigaHunk::structIDToString(dataHeadersOptions.nID));
//                 dataHeader.nSize = 4;
//                 dataHeader.listRecords.append(getDataRecord(0, 4, "hunk_id", VT_UINT32, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));

//                 listResult.append(dataHeader);
//             }
//         }
//     }

//     return listResult;
// }

QList<XBinary::FPART> XAmigaHunk::getFileParts(quint32 nFileParts, qint32 nLimit, PDSTRUCT *pPdStruct)
{
    QList<FPART> list;

    if ((nLimit < -1) || (nLimit == 0)) {
        return list;
    }

    QList<HUNK> hunks = getHunks(pPdStruct);
    if (hunks.isEmpty()) return list;

    qint64 nMaxOffset = 0;
    // Sequential VA assignment for loadable hunks starting at IMAGE_BASE
    XADDR currentVA = XAMIGAHUNK_DEF::IMAGE_BASE;

    for (int i = 0; i < hunks.size(); ++i) {
        const HUNK &h = hunks.at(i);

        if ((nFileParts & FILEPART_TABLE) || (nFileParts & FILEPART_SEGMENT)) {
            // Assign sequential VAs to loadable hunks with 4-byte alignment
            if ((h.nId == XAMIGAHUNK_DEF::HUNK_CODE) || (h.nId == XAMIGAHUNK_DEF::HUNK_DATA) || (h.nId == XAMIGAHUNK_DEF::HUNK_BSS) ||
                (h.nId == XAMIGAHUNK_DEF::HUNK_PPC_CODE)) {
                if (nFileParts & FILEPART_TABLE) {
                    FPART r = {};
                    r.filePart = FILEPART_TABLE;
                    r.nFileOffset = h.nOffset;
                    r.nFileSize = 8;
                    r.nVirtualAddress = (XADDR)-1;
                    r.sName = tr("Table");
                    list.append(r);
                    if ((nLimit != -1) && (list.count() >= nLimit)) return list;
                }

                quint32 nLongwords = read_uint32(h.nOffset + 4, true);
                XADDR nMemSize = (XADDR)XBinary::align_up((XADDR)nLongwords * 4, 16);

                if (nFileParts & FILEPART_SEGMENT) {
                    QString sName;

                    if (h.nId == XAMIGAHUNK_DEF::HUNK_CODE) {
                        sName = QString("CODE");
                    } else if (h.nId == XAMIGAHUNK_DEF::HUNK_DATA) {
                        sName = QString("DATA");
                    } else if (h.nId == XAMIGAHUNK_DEF::HUNK_BSS) {
                        sName = QString("BSS");
                    } else if (h.nId == XAMIGAHUNK_DEF::HUNK_PPC_CODE) {
                        sName = QString("PPC_CODE");
                    }

                    FPART r = {};
                    r.filePart = FILEPART_SEGMENT;
                    r.nFileOffset = h.nOffset + 8;
                    r.nFileSize = h.nSize - 8;
                    r.nVirtualAddress = currentVA;
                    r.nVirtualSize = nMemSize;
                    r.sName = sName;
                    list.append(r);
                    if ((nLimit != -1) && (list.count() >= nLimit)) return list;
                }

                currentVA = currentVA + nMemSize;
            }
        }

        if (nFileParts & FILEPART_REGION) {
            FPART r = {};
            r.filePart = FILEPART_REGION;
            r.nFileOffset = h.nOffset;
            r.nFileSize = h.nSize;
            r.nVirtualAddress = (XADDR)-1;
            r.sName = structIDToString(hunkTypeToStructId(h.nId));
            list.append(r);
            if ((nLimit != -1) && (list.count() >= nLimit)) return list;
        }

        nMaxOffset = qMax(nMaxOffset, h.nOffset + h.nSize);
    }

    if (nFileParts & FILEPART_OVERLAY) {
        if (nMaxOffset < getSize()) {
            FPART ov = {};
            ov.filePart = FILEPART_OVERLAY;
            ov.nFileOffset = nMaxOffset;
            ov.nFileSize = getSize() - nMaxOffset;
            ov.nVirtualAddress = (XADDR)-1;
            ov.sName = tr("Overlay");
            list.append(ov);
            if ((nLimit != -1) && (list.count() >= nLimit)) return list;
        }
    }

    return list;
}

QList<QString> XAmigaHunk::getSearchSignatures()
{
    QList<QString> listResult;

    listResult.append("000003F3");
    listResult.append("000003E7");

    return listResult;
}

XBinary *XAmigaHunk::createInstance(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    Q_UNUSED(bIsImage)
    Q_UNUSED(nModuleAddress)

    return new XAmigaHunk(pDevice);
}

bool XAmigaHunk::handleInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XAmigaHunk> guardedThis(this);
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

void *XAmigaHunk::getInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XAmigaHunk> guardedThis(this);
    const bool bHandled = guardedThis->handleInternalInfo(pPdStruct);
    if (!guardedThis || !bHandled) return nullptr;

    return &guardedThis->m_internalInfo;
}

void XAmigaHunk::setInternalInfo(void *pInternalInfo)
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
