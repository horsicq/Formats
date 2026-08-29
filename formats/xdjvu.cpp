// Copyright (c) 2017-2026 hors<horsicq@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#include "xdjvu.h"

XBinary::XIDSTRING _TABLE_XDJVU_ImageTypes[] = {
    {XDJVU::TYPE_UNKNOWN, "UNKNOWN"},       {XDJVU::TYPE_SINGLE_PAGE, "SINGLE_PAGE"}, {XDJVU::TYPE_MULTI_PAGE, "MULTI_PAGE"},
    {XDJVU::TYPE_MULTI_FILE, "MULTI_FILE"}, {XDJVU::TYPE_THUMBNAILS, "THUMBNAILS"},   {XDJVU::TYPE_SECURE, "SECURE"},
};

const QString XDJVU::PREFIX_ImageType = "IMAGETYPE";

XBinary::XCONVERT _TABLE_XDJVU_STRUCTID[] = {
    {XDJVU::STRUCTID_UNKNOWN, "Unknown", QObject::tr("Unknown")},
    {XDJVU::STRUCTID_HEADER, "HEADER", QString("Header")},
    {XDJVU::STRUCTID_CHUNK, "CHUNK", QString("Chunk")},
};

XDJVU::XDJVU(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress) : XBinary(pDevice, bIsImage, nModuleAddress)
{
    m_header = {};
}

XDJVU::~XDJVU()
{
}

bool XDJVU::isValid(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    bool bResult = false;

    quint64 nSize = getSize();

    if (nSize >= 16) {
        QString sSignature = read_ansiString(0, 8);

        if ((sSignature == "AT&TFORM") || (sSignature == "SDJVFORM")) {
            QString sType = read_ansiString(12, 4);

            if ((sType == "DJVU") || (sType == "DJVM") || (sType == "DJVI") || (sType.startsWith("THUM"))) {
                bResult = true;
            }
        }
    }

    return bResult;
}

QString XDJVU::getVersion()
{
    return "";
}

XBinary::FT XDJVU::getFileType()
{
    return FT_DJVU;
}

XDJVU::HEADER XDJVU::getHeader()
{
    if (m_header.sSignature == "") {
        m_header = _getHeader();
    }

    return m_header;
}

bool XDJVU::isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    XDJVU djvu(pDevice);

    return djvu.isValid(pPdStruct);
}

QString XDJVU::getFileFormatExt()
{
    return "djvu";
}

qint64 XDJVU::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    qint64 nResult = 0;

    HEADER header = getHeader();

    if (header.bIsValid) {
        nResult = header.nSize + 12;
    }

    return nResult;
}

QString XDJVU::getMIMEString()
{
    return "application/x-djvu";
}

QString XDJVU::getFileFormatExtsString()
{
    return "DjVu (*.djvu *.djv)";
}

XBinary::ENDIAN XDJVU::getEndian()
{
    return ENDIAN_BIG;
}

XBinary::MODE XDJVU::getMode()
{
    return MODE_UNKNOWN;
}

QString XDJVU::getArch()
{
    return "";
}

QList<XBinary::MAPMODE> XDJVU::getMapModesList()
{
    QList<MAPMODE> listResult;

    listResult.append(MAPMODE_REGIONS);

    return listResult;
}

XBinary::_MEMORY_MAP XDJVU::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    XBinary::_MEMORY_MAP result = {};

    if (mapMode == MAPMODE_UNKNOWN) {
        mapMode = MAPMODE_REGIONS;
    }

    if (mapMode == MAPMODE_REGIONS) {
        result = _getMemoryMap(FILEPART_HEADER | FILEPART_REGION | FILEPART_OVERLAY, pPdStruct);
    }

    return result;
}

QString XDJVU::structIDToString(quint32 nID)
{
    return XBinary::XCONVERT_idToTransString(nID, _TABLE_XDJVU_STRUCTID, sizeof(_TABLE_XDJVU_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QString XDJVU::structIDToFtString(quint32 nID)
{
    return XBinary::XCONVERT_idToFtString(nID, _TABLE_XDJVU_STRUCTID, sizeof(_TABLE_XDJVU_STRUCTID) / sizeof(XBinary::XCONVERT));
}

quint32 XDJVU::ftStringToStructID(const QString &sFtString)
{
    return XCONVERT_ftStringToId(sFtString, _TABLE_XDJVU_STRUCTID, sizeof(_TABLE_XDJVU_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QList<XBinary::XFHEADER> XDJVU::getXFHeaders(const XFSTRUCT &xfStruct, PDSTRUCT *pPdStruct)
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
        xfHeader.nSize = 16;
        xfHeader.xfType = XFTYPE_HEADER;
        xfHeader.listFields = getXFRecords(xfStruct.fileType, STRUCTID_HEADER, headerLoc);
        xfHeader.sTag = xfHeaderToTag(xfHeader, structIDToString(STRUCTID_HEADER), xfHeader.sParentTag);
        listResult.append(xfHeader);

        if (xfStruct.bIsParent) {
            XFSTRUCT _xfStruct = xfStruct;
            _xfStruct.sParent = xfHeader.sTag;
            _xfStruct.nStructID = STRUCTID_CHUNK;
            _xfStruct.xLoc = offsetToLoc(16);
            listResult.append(getXFHeaders(_xfStruct, pPdStruct));
        }
    } else if (nStructID == STRUCTID_CHUNK) {
        QList<CHUNK_RECORD> listChunks = _getChunkRecords(pPdStruct);

        if (!listChunks.isEmpty()) {
            XFHEADER xfHeader = {};
            xfHeader.sParentTag = xfStruct.sParent;
            xfHeader.fileType = xfStruct.fileType;
            xfHeader.structID = static_cast<XBinary::STRUCTID>(STRUCTID_CHUNK);
            xfHeader.xLoc = offsetToLoc(listChunks.first().nOffset);
            xfHeader.xfType = XFTYPE_TABLE;
            xfHeader.listFields = getXFRecords(xfStruct.fileType, STRUCTID_CHUNK, xfHeader.xLoc);

            qint32 nNumberOfChunks = listChunks.count();

            for (qint32 i = 0; i < nNumberOfChunks; i++) {
                xfHeader.listRowLocations.append(listChunks.at(i).nOffset);
            }

            xfHeader.sTag = xfHeaderToTag(xfHeader, structIDToString(STRUCTID_CHUNK), xfHeader.sParentTag);
            listResult.append(xfHeader);
        }
    }

    return listResult;
}

QList<XBinary::XFRECORD> XDJVU::getXFRecords(FT fileType, quint32 nStructID, const XLOC &xLoc)
{
    Q_UNUSED(fileType)
    Q_UNUSED(xLoc)

    QList<XBinary::XFRECORD> listResult;

    // DjVu (IFF85) chunk sizes are big-endian
    if (nStructID == STRUCTID_HEADER) {
        listResult.append({"Signature", 0, 8, XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        listResult.append({"FormSize", 8, 4, XFRECORD_FLAG_BE | XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"FormType", 12, 4, XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
    } else if (nStructID == STRUCTID_CHUNK) {
        listResult.append({"ID", 0, 4, XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        listResult.append({"Size", 4, 4, XFRECORD_FLAG_BE | XFRECORD_FLAG_SIZE, VT_UINT32});
    }

    return listResult;
}

// QList<XBinary::DATA_HEADER> XDJVU::getDataHeaders(const DATA_HEADERS_OPTIONS &dataHeadersOptions, PDSTRUCT *pPdStruct)
// {
//     QList<DATA_HEADER> listResult;

//     if (dataHeadersOptions.nID == STRUCTID_UNKNOWN) {
//         DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
//         _dataHeadersOptions.bChildren = true;
//         _dataHeadersOptions.dsID_parent = _addDefaultHeaders(&listResult, pPdStruct);
//         _dataHeadersOptions.dhMode = XBinary::DHMODE_HEADER;
//         _dataHeadersOptions.fileType = dataHeadersOptions.pMemoryMap->fileType;

//         _dataHeadersOptions.nID = STRUCTID_HEADER;
//         _dataHeadersOptions.nLocation = 0;
//         _dataHeadersOptions.locType = XBinary::LT_OFFSET;

//         listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//     } else {
//         qint64 nStartOffset = locationToOffset(dataHeadersOptions.pMemoryMap, dataHeadersOptions.locType, dataHeadersOptions.nLocation);

//         if (nStartOffset != -1) {
//             if (dataHeadersOptions.nID == STRUCTID_HEADER) {
//                 DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, XDJVU::structIDToString(dataHeadersOptions.nID));
//                 dataHeader.nSize = 16;

//                 dataHeader.listRecords.append(getDataRecord(0, 8, "Signature", VT_ANSI, DRF_UNKNOWN, XBinary::ENDIAN_BIG));
//                 dataHeader.listRecords.append(getDataRecord(8, 4, "Size", VT_UINT32, DRF_SIZE, XBinary::ENDIAN_BIG));
//                 dataHeader.listRecords.append(getDataRecord(12, 4, "Type", VT_ANSI, DRF_UNKNOWN, XBinary::ENDIAN_BIG));

//                 listResult.append(dataHeader);

//                 if (dataHeadersOptions.bChildren) {
//                     HEADER header = getHeader();

//                     if (header.bIsValid && !header.bIsSecure) {
//                         qint64 nCurrentOffset = 16;
//                         qint64 nEndOffset = qMin((qint64)(header.nSize + 12), getSize());
//                         qint32 nNumberOfChunks = 0;

//                         while (nCurrentOffset < nEndOffset) {
//                             if (nCurrentOffset + 8 > nEndOffset) break;

//                             qint64 nChunkSize = read_uint32(nCurrentOffset + 4, true);
//                             QString sChunkName = read_ansiString(nCurrentOffset, 4);

//                             if (!_isChunkValid(sChunkName)) {
//                                 break;
//                             }

//                             if (nCurrentOffset + 8 + nChunkSize > nEndOffset) {
//                                 break;
//                             }

//                             nNumberOfChunks++;
//                             nCurrentOffset += (8 + nChunkSize);

//                             if (nCurrentOffset & 1) {
//                                 nCurrentOffset++;
//                             }

//                             if (XBinary::isPdStructNotCanceled(pPdStruct) == false) {
//                                 break;
//                             }
//                         }

//                         DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
//                         _dataHeadersOptions.dhMode = XBinary::DHMODE_TABLE;
//                         _dataHeadersOptions.nID = STRUCTID_CHUNK;
//                         _dataHeadersOptions.nLocation = 16;
//                         _dataHeadersOptions.nCount = nNumberOfChunks;
//                         _dataHeadersOptions.nSize = nCurrentOffset - 16;

//                         listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//                     }
//                 }
//             } else if (dataHeadersOptions.nID == STRUCTID_CHUNK) {
//                 DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, XDJVU::structIDToString(dataHeadersOptions.nID));

//                 quint32 nChunkSize = read_uint32(nStartOffset + 4, true);
//                 dataHeader.nSize = 8 + nChunkSize;

//                 dataHeader.listRecords.append(getDataRecord(0, 4, "Name", VT_ANSI, DRF_UNKNOWN, XBinary::ENDIAN_BIG));
//                 dataHeader.listRecords.append(getDataRecord(4, 4, "Size", VT_UINT32, DRF_SIZE, XBinary::ENDIAN_BIG));
//                 dataHeader.listRecords.append(getDataRecord(8, nChunkSize, "Data", VT_BYTE_ARRAY, DRF_UNKNOWN, XBinary::ENDIAN_BIG));

//                 listResult.append(dataHeader);
//             }
//         }
//     }

//     return listResult;
// }

QList<XBinary::FPART> XDJVU::getFileParts(quint32 nFileParts, qint32 nLimit, PDSTRUCT *pPdStruct)
{
    QList<FPART> listResult;

    if ((nLimit < -1) || (nLimit == 0)) {
        return listResult;
    }

    if (nFileParts & FILEPART_HEADER) {
        FPART record = {};

        record.filePart = FILEPART_HEADER;
        record.nFileOffset = 0;
        record.nFileSize = 16;
        record.nVirtualAddress = (XADDR)-1;
        record.sName = tr("Header");

        listResult.append(record);
        if ((nLimit != -1) && (listResult.count() >= nLimit)) return listResult;
    }

    HEADER header = getHeader();

    if (header.bIsValid && !header.bIsSecure) {
        qint64 nOffset = 16;
        qint64 nEndOffset = qMin((qint64)(header.nSize + 12), getSize());

        while (nOffset < nEndOffset) {
            if (nOffset + 8 > nEndOffset) break;

            qint64 nChunkSize = read_uint32(nOffset + 4, true);
            QString sChunkName = read_ansiString(nOffset, 4);

            if (!_isChunkValid(sChunkName)) {
                break;
            }

            if (nOffset + 8 + nChunkSize > nEndOffset) {
                break;
            }

            if (nFileParts & FILEPART_REGION) {
                FPART record = {};

                record.filePart = FILEPART_REGION;
                record.nFileOffset = nOffset;
                record.nFileSize = 8 + nChunkSize;
                record.nVirtualAddress = (XADDR)-1;
                record.sName = sChunkName;

                listResult.append(record);
                if ((nLimit != -1) && (listResult.count() >= nLimit)) return listResult;
            }

            nOffset += (8 + nChunkSize);

            if (nOffset & 1) {
                nOffset++;
            }

            if (XBinary::isPdStructNotCanceled(pPdStruct) == false) {
                break;
            }
        }
    }

    qint64 nFileFormatSize = getFileFormatSize();
    qint64 nTotalSize = getSize();

    if (nFileParts & FILEPART_OVERLAY) {
        if (nFileFormatSize < nTotalSize) {
            FPART record = {};

            record.filePart = FILEPART_OVERLAY;
            record.nFileOffset = nFileFormatSize;
            record.nFileSize = nTotalSize - nFileFormatSize;
            record.nVirtualAddress = (XADDR)-1;
            record.sName = tr("Overlay");

            listResult.append(record);
            if ((nLimit != -1) && (listResult.count() >= nLimit)) return listResult;
        }
    }

    return listResult;
}

bool XDJVU::isSecure()
{
    return getHeader().bIsSecure;
}

quint32 XDJVU::getDocumentSize()
{
    return getHeader().nSize;
}

XDJVU::INFO_RECORD XDJVU::getInfoRecord(qint64 nOffset, PDSTRUCT *pPdStruct)
{
    return _getInfoRecord(nOffset, pPdStruct);
}

QString XDJVU::getDocumentInfo(PDSTRUCT *pPdStruct)
{
    QString sResult;

    HEADER header = getHeader();

    if (header.bIsValid) {
        sResult += QString("Type: %1\n").arg(getImageTypes().value(header.type));
        sResult += QString("Size: %1 bytes\n").arg(header.nSize);

        if (header.bIsSecure) {
            sResult += "Secure: Yes\n";
        } else {
            sResult += "Secure: No\n";

            QList<CHUNK_RECORD> listChunks = _getChunkRecords(pPdStruct);

            sResult += QString("Chunks: %1\n").arg(listChunks.count());

            for (qint32 i = 0; i < listChunks.count(); i++) {
                CHUNK_RECORD chunk = listChunks.at(i);

                sResult += QString("  [%1] %2 - Offset: 0x%3, Size: %4 bytes\n").arg(i).arg(chunk.sName).arg(chunk.nOffset, 0, 16).arg(chunk.nSize);

                if (chunk.sName == "INFO") {
                    INFO_RECORD infoRecord = _getInfoRecord(chunk.nDataOffset, pPdStruct);

                    sResult += QString("      Width: %1, Height: %2\n").arg(infoRecord.nWidth).arg(infoRecord.nHeight);
                    sResult += QString("      Version: %1.%2\n").arg(infoRecord.nMajorVersion).arg(infoRecord.nMinorVersion);
                    sResult += QString("      DPI: %1\n").arg(infoRecord.nDPI);
                    sResult += QString("      Gamma: %1\n").arg(infoRecord.nGamma);
                }

                if (XBinary::isPdStructNotCanceled(pPdStruct) == false) {
                    break;
                }
            }
        }
    }

    return sResult;
}

QMap<quint64, QString> XDJVU::getImageTypes()
{
    return XBinary::XIDSTRING_createMapPrefix(_TABLE_XDJVU_ImageTypes, sizeof(_TABLE_XDJVU_ImageTypes) / sizeof(XBinary::XIDSTRING), PREFIX_ImageType);
}

QMap<quint64, QString> XDJVU::getImageTypesS()
{
    return XBinary::XIDSTRING_createMap(_TABLE_XDJVU_ImageTypes, sizeof(_TABLE_XDJVU_ImageTypes) / sizeof(XBinary::XIDSTRING));
}

XDJVU::HEADER XDJVU::_getHeader()
{
    HEADER result = {};

    quint64 nSize = getSize();

    if (nSize >= 12) {
        result.sSignature = read_ansiString(0, 8);

        if (result.sSignature == "AT&TFORM") {
            result.bIsSecure = false;
            result.nSize = read_uint32(8, true);  // Big endian

            QString sType = read_ansiString(12, 4);

            if (sType == "DJVU") {
                result.type = TYPE_SINGLE_PAGE;
                result.bIsValid = true;
            } else if (sType == "DJVM") {
                result.type = TYPE_MULTI_PAGE;
                result.bIsValid = true;
            } else if (sType == "DJVI") {
                result.type = TYPE_MULTI_FILE;
                result.bIsValid = true;
            } else if (sType.startsWith("THUM")) {
                result.type = TYPE_THUMBNAILS;
                result.bIsValid = true;
            }
        } else if (result.sSignature == "SDJVFORM") {
            result.bIsSecure = true;
            result.type = TYPE_SECURE;
            result.nSize = read_uint32(8, true);  // Big endian
            result.bIsValid = true;
        }
    }

    return result;
}

QList<XDJVU::CHUNK_RECORD> XDJVU::_getChunkRecords(PDSTRUCT *pPdStruct)
{
    QList<CHUNK_RECORD> listResult;

    HEADER header = getHeader();

    if (header.bIsValid && !header.bIsSecure) {
        qint64 nOffset = 16;  // Start after main header
        qint64 nEndOffset = qMin((qint64)(header.nSize + 12), getSize());

        while (nOffset < nEndOffset) {
            if (nOffset + 8 > nEndOffset) break;

            CHUNK_RECORD record = {};
            record.nOffset = nOffset;
            record.sName = read_ansiString(nOffset, 4);
            record.nSize = read_uint32(nOffset + 4, true);  // Big endian
            record.nDataOffset = nOffset + 8;
            record.nDataSize = record.nSize;
            record.nHeaderSize = 8;

            if (!_isChunkValid(record.sName)) {
                break;
            }

            if (record.nDataOffset + record.nDataSize > nEndOffset) {
                break;
            }

            listResult.append(record);

            nOffset = record.nDataOffset + record.nDataSize;

            // Chunks are aligned on even boundaries
            if (nOffset & 1) {
                nOffset++;
            }

            if (pPdStruct) {
                if (pPdStruct->bIsStop) {
                    break;
                }
            }
        }
    }

    return listResult;
}

XDJVU::INFO_RECORD XDJVU::_getInfoRecord(qint64 nOffset, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    INFO_RECORD result = {};

    if (nOffset + 10 <= getSize()) {
        result.nWidth = read_uint16(nOffset, true);       // Big endian
        result.nHeight = read_uint16(nOffset + 2, true);  // Big endian
        result.nMinorVersion = read_uint8(nOffset + 4);
        result.nMajorVersion = read_uint8(nOffset + 5);
        result.nDPI = read_uint16(nOffset + 6, false);  // Little endian (yes, it's mixed!)
        result.nGamma = read_uint8(nOffset + 8);
        result.nFlags = read_uint8(nOffset + 9);

        if (nOffset + 12 <= getSize()) {
            result.nRotation = read_uint8(nOffset + 10);
            result.nReserved = read_uint8(nOffset + 11);
        }
    }

    return result;
}

bool XDJVU::_isChunkValid(const QString &sChunkName)
{
    static const QStringList listValidChunks = {"INFO", "Sjbz", "FG44", "BG44", "FGbz", "BGjp", "BGbz", "TXTa", "TXTz", "ANTa",
                                                "ANTz", "DJBZ", "FORM", "DIRM", "NAVM", "Smmr", "INCL", "CIDa", "LTAn", "KTAn"};

    return listValidChunks.contains(sChunkName);
}

QList<QString> XDJVU::getSearchSignatures()
{
    QList<QString> listResult;

    listResult.append("'AT&TFORM'");
    listResult.append("'SDJVFORM'");

    return listResult;
}

XBinary *XDJVU::createInstance(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    Q_UNUSED(bIsImage)
    Q_UNUSED(nModuleAddress)

    return new XDJVU(pDevice);
}

bool XDJVU::handleInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XDJVU> guardedThis(this);
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

void *XDJVU::getInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XDJVU> guardedThis(this);
    const bool bHandled = guardedThis->handleInternalInfo(pPdStruct);
    if (!guardedThis || !bHandled) return nullptr;

    return &guardedThis->m_internalInfo;
}

void XDJVU::setInternalInfo(void *pInternalInfo)
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
