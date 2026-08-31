/* Copyright (c) 2022-2026 hors<horsicq@gmail.com>
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
#include "xdos16.h"

XBinary::XCONVERT _TABLE_XDOS16_STRUCTID[] = {
    {XDOS16::STRUCTID_UNKNOWN, "Unknown", QObject::tr("Unknown")},
    {XDOS16::STRUCTID_LOADER, "LOADER", QString("Loader")},
    {XDOS16::STRUCTID_SEGMENT, "SEGMENT", QString("Segment")},
    {XDOS16::STRUCTID_PAYLOAD, "PAYLOAD", QString("Payload")},
};

XDOS16::XDOS16(QIODevice *pDevice) : XArchive(pDevice)
{
}

XDOS16::~XDOS16()
{
}

bool XDOS16::isValid(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    bool bResult = false;

    if (getSize() > 1024) {
        if (read_uint16(0) == XMSDOS_DEF::S_IMAGE_DOS_SIGNATURE_MZ) {
            bResult = (getFileType() != FT_UNKNOWN);
        }
    }

    return bResult;
}

bool XDOS16::isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    XDOS16 xdos16(pDevice);

    return xdos16.isValid(pPdStruct);
}

quint64 XDOS16::getNumberOfRecords(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    quint64 nResult = 0;

    quint16 nCP = read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_cp));
    quint16 nCblp = read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_cblp));
    qint64 nSize = getSize();

    if (nCP > 0) {
        nResult = 1;
        qint64 nSignatureOffset = (nCP - 1) * 512 + nCblp;
        if (nSize - nSignatureOffset) {
            while (true) {
                quint16 nSignature = read_uint16(nSignatureOffset);

                if (nSignature == 0x5742) {  // BW
                    nSignatureOffset = read_uint32(nSignatureOffset + offsetof(XMSDOS_DEF::dos16m_exe_header, next_header_pos));
                    nResult++;
                } else if (nSignature == 0x464D) {  // MF - find info
                    nSignatureOffset += read_uint32(nSignatureOffset + 2);
                } else if (nSignature == 0x5A4D) {  // MZ
                    nResult++;
                    break;
                } else {
                    break;
                }
            }
        }
    }

    return nResult;
}

QString XDOS16::getOsVersion()
{
    return "3.0";
}

XBinary::OSNAME XDOS16::getOsName()
{
    return OSNAME_MSDOS;
}

QList<XArchive::RECORD> XDOS16::getRecords(qint32 nLimit, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    QList<XArchive::RECORD> listResult;

    if ((nLimit < -1) || (nLimit == 0)) {
        return listResult;
    }

    quint16 nCP = read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_cp));
    quint16 nCblp = read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_cblp));
    qint64 nSize = getSize();

    if (nCP > 0) {
        qint64 nSignatureOffset = (nCP - 1) * 512 + nCblp;
        {
            RECORD record = {};

            record.spInfo.sRecordName = tr("Loader");
            record.nDataOffset = 0;
            record.nDataSize = nSignatureOffset;
            record.spInfo.nUncompressedSize = nSignatureOffset;
            record.spInfo.compressMethod = HANDLE_METHOD_STORE;

            listResult.append(record);
        }

        if ((nLimit != -1) && (listResult.count() >= nLimit)) {
            return listResult;
        }

        if (nSize - nSignatureOffset) {
            while (true) {
                quint16 nSignature = read_uint16(nSignatureOffset);

                if (nSignature == 0x5742) {  // BW
                    qint64 nNewSignatureOffset = read_uint32(nSignatureOffset + offsetof(XMSDOS_DEF::dos16m_exe_header, next_header_pos));
                    QString sName = read_ansiString(nSignatureOffset + offsetof(XMSDOS_DEF::dos16m_exe_header, EXP_path));

                    nNewSignatureOffset = qMin(nNewSignatureOffset, nSize);

                    RECORD record = {};

                    record.spInfo.sRecordName = sName;
                    record.nDataOffset = nSignatureOffset;
                    record.nDataSize = nNewSignatureOffset - nSignatureOffset;
                    record.spInfo.nUncompressedSize = nNewSignatureOffset - nSignatureOffset;
                    record.spInfo.compressMethod = HANDLE_METHOD_STORE;

                    listResult.append(record);

                    if ((nLimit != -1) && (listResult.count() >= nLimit)) {
                        break;
                    }

                    nSignatureOffset = nNewSignatureOffset;
                } else if (nSignature == 0x464D) {  // MF - find info
                    nSignatureOffset += read_uint32(nSignatureOffset + 2);
                } else if (nSignature == 0x5A4D) {  // MZ
                    RECORD record = {};

                    record.spInfo.sRecordName = tr("Payload");
                    record.nDataOffset = nSignatureOffset;
                    record.nDataSize = nSize - nSignatureOffset;
                    record.spInfo.nUncompressedSize = nSize - nSignatureOffset;
                    record.spInfo.compressMethod = HANDLE_METHOD_STORE;

                    listResult.append(record);

                    break;
                } else {
                    break;
                }
            }
        }
    }

    return listResult;
}

bool XDOS16::initUnpack(UNPACK_STATE *pState, const QMap<UNPACK_PROP, QVariant> &mapProperties, PDSTRUCT *pPdStruct)
{
    QPointer<XDOS16> guardedThis(this);
    if (m_bUnpackOperationInProgress) {
        return false;
    }
    UNPACK_OPERATION_GUARD operationGuard(&m_bUnpackOperationInProgress);
    if (!operationGuard.isAcquired() || !pState) {
        return false;
    }

    if ((pState->pContext || !pState->baUnpackSourceToken.isEmpty()) && !guardedThis->ownsUnpackSource(pState)) {
        return false;
    }

    DOS16_UNPACK_CONTEXT *pOldContext = static_cast<DOS16_UNPACK_CONTEXT *>(pState->pContext);
    guardedThis->releaseUnpackSource(pState);
    pState->pContext = nullptr;
    *pState = UNPACK_STATE();
    delete pOldContext;

    if (!guardedThis || !XBinary::isPdStructNotCanceled(pPdStruct) || !guardedThis->bindUnpackSource(pState, pPdStruct) || !guardedThis) {
        return false;
    }

    const bool bValid = guardedThis->isValid(pPdStruct);
    if (!guardedThis || !bValid) {
        if (guardedThis) guardedThis->releaseUnpackSource(pState);
        *pState = UNPACK_STATE();
        return false;
    }

    const QList<RECORD> listRecords = guardedThis->getRecords(-1, pPdStruct);
    if (!guardedThis) {
        return false;
    }
    const qint64 nSourceSize = guardedThis->getSize();
    if (!guardedThis) {
        return false;
    }

    if (listRecords.isEmpty() || (nSourceSize < 0) || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        guardedThis->releaseUnpackSource(pState);
        *pState = UNPACK_STATE();
        return false;
    }

    DOS16_UNPACK_CONTEXT *pContext = new (std::nothrow) DOS16_UNPACK_CONTEXT;
    if (!pContext) {
        guardedThis->releaseUnpackSource(pState);
        *pState = UNPACK_STATE();
        return false;
    }

    for (const RECORD &record : listRecords) {
        if ((record.nDataOffset < 0) || (record.nDataSize < 0) || (record.nDataOffset > nSourceSize) || (record.nDataSize > (nSourceSize - record.nDataOffset))) {
            guardedThis->releaseUnpackSource(pState);
            delete pContext;
            *pState = UNPACK_STATE();
            return false;
        }
    }

    pContext->listRecords = listRecords;
    pState->pContext = pContext;
    pState->nCurrentOffset = 0;
    pState->nCurrentIndex = 0;
    pState->nNumberOfRecords = pContext->listRecords.count();
    pState->nTotalSize = nSourceSize;
    pState->mapUnpackProperties = mapProperties;

    if (!guardedThis->validateAndFinalizeUnpackSource(pState, pContext, pPdStruct)) {
        if (!guardedThis) return false;
        pState->pContext = nullptr;
        guardedThis->releaseUnpackSource(pState);
        delete pContext;
        *pState = UNPACK_STATE();
        return false;
    }

    return true;
}

XBinary::ARCHIVERECORD XDOS16::infoCurrent(UNPACK_STATE *pState, PDSTRUCT *pPdStruct)
{
    QPointer<XDOS16> guardedThis(this);
    UNPACK_OPERATION_GUARD operationGuard(&m_bUnpackOperationInProgress, &m_bNestedUnpackInfoAuthorized);
    if (!operationGuard.isAllowed() || !pState || !pState->pContext) {
        return ARCHIVERECORD();
    }

    const bool bSourceCurrent = guardedThis->isUnpackSourceCurrent(pState, pPdStruct);
    if (!guardedThis || !bSourceCurrent || (pState->nCurrentIndex < 0) || (pState->nCurrentIndex >= pState->nNumberOfRecords)) {
        return ARCHIVERECORD();
    }

    DOS16_UNPACK_CONTEXT *pContext = static_cast<DOS16_UNPACK_CONTEXT *>(pState->pContext);
    if (pState->nCurrentIndex >= pContext->listRecords.count()) {
        return ARCHIVERECORD();
    }

    const RECORD &record = pContext->listRecords.at(pState->nCurrentIndex);
    ARCHIVERECORD result = {};
    result.nStreamOffset = record.nDataOffset;
    result.nStreamSize = record.nDataSize;
    result.mapProperties = record.mapProperties;
    result.mapProperties.insert(FPART_PROP_ORIGINALNAME, record.spInfo.sRecordName);
    result.mapProperties.insert(FPART_PROP_COMPRESSEDSIZE, record.nDataSize);
    result.mapProperties.insert(FPART_PROP_UNCOMPRESSEDSIZE, record.spInfo.nUncompressedSize);
    result.mapProperties.insert(FPART_PROP_HANDLEMETHOD, static_cast<quint32>(record.spInfo.compressMethod));

    return result;
}

bool XDOS16::moveToNext(UNPACK_STATE *pState, PDSTRUCT *pPdStruct)
{
    QPointer<XDOS16> guardedThis(this);
    UNPACK_OPERATION_GUARD operationGuard(&m_bUnpackOperationInProgress);
    if (!operationGuard.isAcquired() || !pState || !pState->pContext) {
        return false;
    }

    const bool bSourceCurrent = guardedThis->isUnpackSourceCurrent(pState, pPdStruct);
    if (!guardedThis || !bSourceCurrent || (pState->nCurrentIndex < 0) || (pState->nCurrentIndex >= pState->nNumberOfRecords)) {
        return false;
    }

    DOS16_UNPACK_CONTEXT *pContext = static_cast<DOS16_UNPACK_CONTEXT *>(pState->pContext);
    if (pState->nNumberOfRecords != pContext->listRecords.count()) {
        return false;
    }

    pState->nCurrentIndex++;

    return (pState->nCurrentIndex < pState->nNumberOfRecords);
}

bool XDOS16::finishUnpack(UNPACK_STATE *pState, PDSTRUCT *pPdStruct)
{
    QPointer<XDOS16> guardedThis(this);
    UNPACK_OPERATION_GUARD operationGuard(&m_bUnpackOperationInProgress);
    if (!operationGuard.isAcquired() || !pState) {
        return false;
    }

    Q_UNUSED(pPdStruct)

    if ((pState->pContext || !pState->baUnpackSourceToken.isEmpty()) && !guardedThis->ownsUnpackSource(pState)) {
        return false;
    }

    DOS16_UNPACK_CONTEXT *pContext = static_cast<DOS16_UNPACK_CONTEXT *>(pState->pContext);
    guardedThis->releaseUnpackSource(pState);
    pState->pContext = nullptr;
    *pState = UNPACK_STATE();
    delete pContext;

    return guardedThis;
}

static bool dos16CanAppendPart(qint32 nLimit, const QList<XBinary::FPART> &listResult)
{
    return (nLimit == -1) || (listResult.size() < nLimit);
}

QList<XBinary::FPART> XDOS16::getFileParts(quint32 nFileParts, qint32 nLimit, PDSTRUCT *pPdStruct)
{
    QList<FPART> listResult;

    if ((nLimit < -1) || (nLimit == 0)) {
        return listResult;
    }

    const qint64 nFileSize = getSize();

    quint16 nCP = read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_cp));
    quint16 nCblp = read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_cblp));

    if (nCP == 0) {
        return listResult;
    }

    qint64 nSignatureOffset = (nCP - 1) * 512 + nCblp;
    nSignatureOffset = qBound<qint64>(0, nSignatureOffset, nFileSize);

    // Optional: header part (loader before first signature)
    if ((nFileParts & FILEPART_HEADER) && dos16CanAppendPart(nLimit, listResult) && (nSignatureOffset > 0)) {
        FPART header = {};
        header.filePart = FILEPART_HEADER;
        header.nFileOffset = 0;
        header.nFileSize = qMin<qint64>(nSignatureOffset, nFileSize);
        header.nVirtualAddress = XADDR_MAX;
        header.sName = tr("Header");
        listResult.append(header);
    }

    // Regions: walk BW/MF/MZ chain similar to getRecords/getMemoryMap
    qint32 nIndex = 0;
    qint64 nCur = nSignatureOffset;
    if (nFileSize > nCur) {
        while (dos16CanAppendPart(nLimit, listResult) && isPdStructNotCanceled(pPdStruct)) {
            if (!isOffsetValid(nCur + 1)) break;
            quint16 nSig = read_uint16(nCur);

            if (nSig == 0x5742) {  // BW
                qint64 nNext = read_uint32(nCur + offsetof(XMSDOS_DEF::dos16m_exe_header, next_header_pos));
                QString sName = read_ansiString(nCur + offsetof(XMSDOS_DEF::dos16m_exe_header, EXP_path));
                if ((nNext <= 0) || (nNext > nFileSize)) nNext = nFileSize;  // clamp

                if ((nFileParts & FILEPART_REGION) && dos16CanAppendPart(nLimit, listResult)) {
                    FPART part = {};
                    part.filePart = FILEPART_REGION;
                    part.nFileOffset = nCur;
                    part.nFileSize = qMax<qint64>(0, nNext - nCur);
                    part.nVirtualAddress = XADDR_MAX;
                    part.sName = sName.isEmpty() ? (tr("Segment") + QString(" %1")).arg(nIndex) : sName;
                    listResult.append(part);
                }

                nCur = nNext;
                nIndex++;
            } else if (nSig == 0x464D) {  // MF - info block, skip length at +2
                qint64 nSkip = read_uint32(nCur + 2);
                nCur += qMax<qint64>(0, nSkip);
            } else if (nSig == 0x5A4D) {  // MZ payload
                if ((nFileParts & (FILEPART_REGION | FILEPART_DATA)) && dos16CanAppendPart(nLimit, listResult)) {
                    FPART data = {};
                    data.filePart = (nFileParts & FILEPART_DATA) ? FILEPART_DATA : FILEPART_REGION;
                    data.nFileOffset = nCur;
                    data.nFileSize = qMax<qint64>(0, nFileSize - nCur);
                    data.nVirtualAddress = XADDR_MAX;
                    data.sName = (data.filePart == FILEPART_DATA) ? tr("Data") : tr("Payload");
                    listResult.append(data);
                }
                break;
            } else {
                break;
            }
        }
    }

    // Overlay: any tail not covered by the above when not using DATA-only
    if ((nFileParts & FILEPART_OVERLAY) && dos16CanAppendPart(nLimit, listResult) && !listResult.isEmpty()) {
        qint64 nCoveredEnd = 0;
        for (qint32 nI = 0; nI < listResult.size(); nI++) {
            const FPART &record = listResult.at(nI);
            if (record.filePart != FILEPART_OVERLAY) {
                nCoveredEnd = qMax(nCoveredEnd, record.nFileOffset + qMax<qint64>(0, record.nFileSize));
            }
        }
        if (nCoveredEnd < nFileSize) {
            FPART ov = {};
            ov.filePart = FILEPART_OVERLAY;
            ov.nFileOffset = nCoveredEnd;
            ov.nFileSize = nFileSize - nCoveredEnd;
            ov.nVirtualAddress = XADDR_MAX;
            ov.sName = tr("Overlay");
            listResult.append(ov);
        }
    }

    return listResult;
}

XBinary::FT XDOS16::getFileType()
{
    XBinary::FT result = FT_UNKNOWN;

    quint16 nCP = read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_cp));
    quint16 nCblp = read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_cblp));
    qint64 nSize = getSize();

    if (nCP > 0) {
        qint64 nSignatureOffset = (nCP - 1) * 512 + nCblp;
        if (nSize - nSignatureOffset) {
            bool bBW = false;
            while (true) {
                quint16 nSignature = read_uint16(nSignatureOffset);

                if (nSignature == 0x5742) {  // BW
                    bBW = true;
                    result = FT_DOS16M;
                    nSignatureOffset = read_uint32(nSignatureOffset + offsetof(XMSDOS_DEF::dos16m_exe_header, next_header_pos));
                } else if (nSignature == 0x464D) {  // MF - find info
                    nSignatureOffset += read_uint32(nSignatureOffset + 2);
                } else if (nSignature == 0x5A4D) {  // MZ
                    qint64 nSignatureOffsetOpt = read_uint32(nSignatureOffset + offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_lfanew));
                    quint16 nSignatureOpt = read_uint16(nSignatureOffsetOpt + nSignatureOffset);

                    if (nSignatureOpt == 0x454E) {  // NE
                        if (bBW) {
                            result = FT_DOS16M;
                        }
                    } else if (nSignatureOpt == 0x454C) {  // LE
                        if (bBW) {
                            result = FT_DOS4G;
                        }
                    } else if (nSignatureOpt == 0x584C) {  // LX
                        if (bBW) {
                            result = FT_DOS4G;
                        }
                    }
                    break;
                } else {
                    break;
                }
            }
        }
    }

    return result;
}

XBinary::_MEMORY_MAP XDOS16::getMemoryMap(XBinary::MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(mapMode)
    Q_UNUSED(pPdStruct)

    _MEMORY_MAP result = {};

    qint32 nIndex = 0;

    result.fileType = getFileType();

    result.sArch = QString("386");
    result.sType = typeIdToString(getType());
    result.mode = getMode();
    result.nBinarySize = getSize();
    result.nImageSize = getImageSize();
    result.nModuleAddress = getModuleAddress();
    result.endian = getEndian();

    quint16 nCP = read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_cp));
    quint16 nCblp = read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_cblp));
    qint64 nSize = getSize();

    if (nCP > 0) {
        qint64 nSignatureOffset = (nCP - 1) * 512 + nCblp;

        {
            _MEMORY_RECORD record = {};
            record.nSize = nSignatureOffset;
            record.nOffset = 0;
            record.nAddress = XADDR_MAX;
            record.filePart = FILEPART_REGION;
            record.sName = tr("Loader");
            record.nIndex = nIndex++;

            result.listRecords.append(record);
        }

        if (nSize - nSignatureOffset) {
            while (true) {
                quint16 nSignature = read_uint16(nSignatureOffset);

                if (nSignature == 0x5742) {  // BW
                    qint64 nNewSignatureOffset = read_uint32(nSignatureOffset + offsetof(XMSDOS_DEF::dos16m_exe_header, next_header_pos));
                    QString sName = read_ansiString(nSignatureOffset + offsetof(XMSDOS_DEF::dos16m_exe_header, EXP_path));

                    nNewSignatureOffset = qMin(nNewSignatureOffset, nSize);

                    _MEMORY_RECORD record = {};
                    record.nSize = nNewSignatureOffset - nSignatureOffset;
                    record.nOffset = nSignatureOffset;
                    record.nAddress = XADDR_MAX;
                    record.filePart = FILEPART_REGION;
                    record.sName = sName;
                    record.nIndex = nIndex++;

                    result.listRecords.append(record);

                    nSignatureOffset = nNewSignatureOffset;
                } else if (nSignature == 0x464D) {  // MF - find info
                    // TODO
                    nSignatureOffset += read_uint32(nSignatureOffset + 2);
                } else if (nSignature == 0x5A4D) {  // MZ
                    _MEMORY_RECORD record = {};
                    record.nSize = nSize - nSignatureOffset;
                    record.nOffset = nSignatureOffset;
                    record.nAddress = XADDR_MAX;
                    record.filePart = FILEPART_REGION;
                    record.sName = tr("Data");
                    record.nIndex = nIndex++;

                    result.listRecords.append(record);

                    break;
                } else {
                    break;
                }
            }
        }
    }

    return result;
}

XBinary::MODE XDOS16::getMode()
{
    return MODE_16;
}

QString XDOS16::getArch()
{
    return "386";
}

XBinary::ENDIAN XDOS16::getEndian()
{
    return ENDIAN_LITTLE;
}

qint32 XDOS16::getType()
{
    return TYPE_DOSEXTENDER;
}

QString XDOS16::getFileFormatExt()
{
    return "exe";
}

QString XDOS16::getFileFormatExtsString()
{
    return "DOS Extender (*.exe)";
}

QString XDOS16::getMIMEString()
{
    return "application/x-dosexec";
}

qint64 XDOS16::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    return getSize();
}

QList<XBinary::MAPMODE> XDOS16::getMapModesList()
{
    QList<MAPMODE> listResult;

    listResult.append(MAPMODE_REGIONS);
    listResult.append(MAPMODE_DATA);

    return listResult;
}

QString XDOS16::structIDToString(quint32 nID)
{
    return XBinary::XCONVERT_idToTransString(nID, _TABLE_XDOS16_STRUCTID, sizeof(_TABLE_XDOS16_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QString XDOS16::structIDToFtString(quint32 nID)
{
    return XBinary::XCONVERT_idToFtString(nID, _TABLE_XDOS16_STRUCTID, sizeof(_TABLE_XDOS16_STRUCTID) / sizeof(XBinary::XCONVERT));
}

quint32 XDOS16::ftStringToStructID(const QString &sFtString)
{
    return XCONVERT_ftStringToId(sFtString, _TABLE_XDOS16_STRUCTID, sizeof(_TABLE_XDOS16_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QList<XBinary::XFHEADER> XDOS16::getXFHeaders(const XFSTRUCT &xfStruct, PDSTRUCT *pPdStruct)
{
    QList<XBinary::XFHEADER> listResult;

    quint32 nStructID = xfStruct.nStructID;

    if (nStructID == STRUCTID_UNKNOWN) {
        XFSTRUCT _xfStruct = xfStruct;
        _xfStruct.nStructID = STRUCTID_LOADER;
        _xfStruct.xLoc = offsetToLoc(0);
        listResult.append(getXFHeaders(_xfStruct, pPdStruct));
    } else if (nStructID == STRUCTID_LOADER) {
        XLOC headerLoc = xfStruct.xLoc;
        if (headerLoc.locType == LT_UNKNOWN) {
            headerLoc = offsetToLoc(0);
        }

        quint16 nCP = read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_cp));
        quint16 nCblp = read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_cblp));
        qint64 nSignatureOffset = (qint64)(nCP - 1) * 512 + nCblp;

        XFHEADER xfHeader = {};
        xfHeader.sParentTag = xfStruct.sParent;
        xfHeader.fileType = xfStruct.fileType;
        xfHeader.structID = static_cast<XBinary::STRUCTID>(STRUCTID_LOADER);
        xfHeader.xLoc = headerLoc;
        xfHeader.nSize = sizeof(XMSDOS_DEF::IMAGE_DOS_HEADER);
        xfHeader.xfType = XFTYPE_HEADER;
        xfHeader.listFields = getXFRecords(xfStruct.fileType, STRUCTID_LOADER, headerLoc);
        xfHeader.sTag = xfHeaderToTag(xfHeader, structIDToString(STRUCTID_LOADER), xfHeader.sParentTag);
        listResult.append(xfHeader);

        if (xfStruct.bIsParent && (nCP > 0)) {
            XFSTRUCT _xfStruct = xfStruct;
            _xfStruct.sParent = xfHeader.sTag;
            _xfStruct.nStructID = STRUCTID_SEGMENT;
            _xfStruct.xLoc = offsetToLoc(nSignatureOffset);
            listResult.append(getXFHeaders(_xfStruct, pPdStruct));
        }
    } else if (nStructID == STRUCTID_SEGMENT) {
        qint64 nSignatureOffset = locToOffset(xfStruct.pMemoryMap, xfStruct.xLoc);
        qint64 nFileSize = getSize();

        if (nSignatureOffset == -1) {
            quint16 nCP = read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_cp));
            quint16 nCblp = read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_cblp));
            nSignatureOffset = (qint64)(nCP - 1) * 512 + nCblp;
        }

        if ((nSignatureOffset > 0) && (nSignatureOffset < nFileSize)) {
            XFHEADER xfHeader = {};
            xfHeader.sParentTag = xfStruct.sParent;
            xfHeader.fileType = xfStruct.fileType;
            xfHeader.structID = static_cast<XBinary::STRUCTID>(STRUCTID_SEGMENT);
            xfHeader.xLoc = offsetToLoc(nSignatureOffset);
            xfHeader.xfType = XFTYPE_TABLE;
            xfHeader.listFields = getXFRecords(xfStruct.fileType, STRUCTID_SEGMENT, xfHeader.xLoc);

            qint64 nCurrentOffset = nSignatureOffset;

            while (XBinary::isPdStructNotCanceled(pPdStruct)) {
                if ((nCurrentOffset + (qint64)sizeof(XMSDOS_DEF::dos16m_exe_header)) > nFileSize) {
                    break;
                }

                quint16 nSignature = read_uint16(nCurrentOffset);

                if (nSignature != 0x5742) {  // BW
                    break;
                }

                xfHeader.listRowLocations.append(nCurrentOffset);

                quint32 nNextHeaderPos = read_uint32(nCurrentOffset + offsetof(XMSDOS_DEF::dos16m_exe_header, next_header_pos));

                if ((nNextHeaderPos == 0) || ((qint64)nNextHeaderPos <= nCurrentOffset) || ((qint64)nNextHeaderPos >= nFileSize)) {
                    break;
                }

                nCurrentOffset = nNextHeaderPos;
            }

            if (!xfHeader.listRowLocations.isEmpty()) {
                xfHeader.sTag = xfHeaderToTag(xfHeader, structIDToString(STRUCTID_SEGMENT), xfHeader.sParentTag);
                listResult.append(xfHeader);
            }
        }
    }

    return listResult;
}

QList<XBinary::XFRECORD> XDOS16::getXFRecords(FT fileType, quint32 nStructID, const XLOC &xLoc)
{
    Q_UNUSED(fileType)
    Q_UNUSED(xLoc)

    QList<XBinary::XFRECORD> listResult;

    if (nStructID == STRUCTID_LOADER) {
        listResult.append({"e_magic", (qint32)offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_magic), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"e_cblp", (qint32)offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_cblp), 2, XFRECORD_FLAG_SIZE, VT_UINT16});
        listResult.append({"e_cp", (qint32)offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_cp), 2, XFRECORD_FLAG_COUNT, VT_UINT16});
        listResult.append({"e_crlc", (qint32)offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_crlc), 2, XFRECORD_FLAG_COUNT, VT_UINT16});
        listResult.append({"e_cparhdr", (qint32)offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_cparhdr), 2, XFRECORD_FLAG_SIZE, VT_UINT16});
        listResult.append({"e_minalloc", (qint32)offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_minalloc), 2, XFRECORD_FLAG_SIZE, VT_UINT16});
        listResult.append({"e_maxalloc", (qint32)offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_maxalloc), 2, XFRECORD_FLAG_SIZE, VT_UINT16});
        listResult.append({"e_ss", (qint32)offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_ss), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"e_sp", (qint32)offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_sp), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"e_csum", (qint32)offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_csum), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"e_ip", (qint32)offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_ip), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"e_cs", (qint32)offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_cs), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"e_lfarlc", (qint32)offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_lfarlc), 2, XFRECORD_FLAG_OFFSET, VT_UINT16});
        listResult.append({"e_ovno", (qint32)offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_ovno), 2, XFRECORD_FLAG_COUNT, VT_UINT16});
    } else if (nStructID == STRUCTID_SEGMENT) {
        listResult.append({"signature", (qint32)offsetof(XMSDOS_DEF::dos16m_exe_header, signature), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"last_page_bytes", (qint32)offsetof(XMSDOS_DEF::dos16m_exe_header, last_page_bytes), 2, XFRECORD_FLAG_SIZE, VT_UINT16});
        listResult.append({"pages_in_file", (qint32)offsetof(XMSDOS_DEF::dos16m_exe_header, pages_in_file), 2, XFRECORD_FLAG_COUNT, VT_UINT16});
        listResult.append({"reserved1", (qint32)offsetof(XMSDOS_DEF::dos16m_exe_header, reserved1), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"reserved2", (qint32)offsetof(XMSDOS_DEF::dos16m_exe_header, reserved2), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"min_alloc", (qint32)offsetof(XMSDOS_DEF::dos16m_exe_header, min_alloc), 2, XFRECORD_FLAG_SIZE, VT_UINT16});
        listResult.append({"max_alloc", (qint32)offsetof(XMSDOS_DEF::dos16m_exe_header, max_alloc), 2, XFRECORD_FLAG_SIZE, VT_UINT16});
        listResult.append({"stack_seg", (qint32)offsetof(XMSDOS_DEF::dos16m_exe_header, stack_seg), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"stack_ptr", (qint32)offsetof(XMSDOS_DEF::dos16m_exe_header, stack_ptr), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"first_reloc_sel", (qint32)offsetof(XMSDOS_DEF::dos16m_exe_header, first_reloc_sel), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"init_ip", (qint32)offsetof(XMSDOS_DEF::dos16m_exe_header, init_ip), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"code_seg", (qint32)offsetof(XMSDOS_DEF::dos16m_exe_header, code_seg), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"runtime_gdt_size", (qint32)offsetof(XMSDOS_DEF::dos16m_exe_header, runtime_gdt_size), 2, XFRECORD_FLAG_SIZE, VT_UINT16});
        listResult.append({"MAKEPM_version", (qint32)offsetof(XMSDOS_DEF::dos16m_exe_header, MAKEPM_version), 2, XFRECORD_FLAG_VERSION, VT_UINT16});
        listResult.append({"next_header_pos", (qint32)offsetof(XMSDOS_DEF::dos16m_exe_header, next_header_pos), 4, XFRECORD_FLAG_OFFSET, VT_UINT32});
        listResult.append({"cv_info_offset", (qint32)offsetof(XMSDOS_DEF::dos16m_exe_header, cv_info_offset), 4, XFRECORD_FLAG_OFFSET, VT_UINT32});
        listResult.append({"last_sel_used", (qint32)offsetof(XMSDOS_DEF::dos16m_exe_header, last_sel_used), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"pmem_alloc", (qint32)offsetof(XMSDOS_DEF::dos16m_exe_header, pmem_alloc), 2, XFRECORD_FLAG_SIZE, VT_UINT16});
        listResult.append({"alloc_incr", (qint32)offsetof(XMSDOS_DEF::dos16m_exe_header, alloc_incr), 2, XFRECORD_FLAG_SIZE, VT_UINT16});
        listResult.append({"options", (qint32)offsetof(XMSDOS_DEF::dos16m_exe_header, options), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"trans_stack_sel", (qint32)offsetof(XMSDOS_DEF::dos16m_exe_header, trans_stack_sel), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"exp_flags", (qint32)offsetof(XMSDOS_DEF::dos16m_exe_header, exp_flags), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"program_size", (qint32)offsetof(XMSDOS_DEF::dos16m_exe_header, program_size), 2, XFRECORD_FLAG_SIZE, VT_UINT16});
        listResult.append({"gdtimage_size", (qint32)offsetof(XMSDOS_DEF::dos16m_exe_header, gdtimage_size), 2, XFRECORD_FLAG_SIZE, VT_UINT16});
        listResult.append({"first_selector", (qint32)offsetof(XMSDOS_DEF::dos16m_exe_header, first_selector), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"default_mem_strategy", (qint32)offsetof(XMSDOS_DEF::dos16m_exe_header, default_mem_strategy), 1, XFRECORD_FLAG_NONE, VT_UINT8});
        listResult.append({"transfer_buffer_size", (qint32)offsetof(XMSDOS_DEF::dos16m_exe_header, transfer_buffer_size), 2, XFRECORD_FLAG_SIZE, VT_UINT16});
        listResult.append({"EXP_path", (qint32)offsetof(XMSDOS_DEF::dos16m_exe_header, EXP_path), 64, XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
    }

    return listResult;
}

QList<QString> XDOS16::getSearchSignatures()
{
    QList<QString> listResult;

    listResult.append("'MZ'");

    return listResult;
}

XBinary *XDOS16::createInstance(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    Q_UNUSED(bIsImage)
    Q_UNUSED(nModuleAddress)

    return new XDOS16(pDevice);
}

bool XDOS16::handleInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XDOS16> guardedThis(this);
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

void *XDOS16::getInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XDOS16> guardedThis(this);
    const bool bHandled = guardedThis->handleInternalInfo(pPdStruct);
    if (!guardedThis || !bHandled) return nullptr;

    return &guardedThis->m_internalInfo;
}

void XDOS16::setInternalInfo(void *pInternalInfo)
{
    if (pInternalInfo) {
        m_internalInfo = *static_cast<INTERNAL_INFO *>(pInternalInfo);
        XArchive::setInternalInfo(static_cast<XArchive::INTERNAL_INFO *>(&m_internalInfo));
    } else {
        m_internalInfo = INTERNAL_INFO();
        XArchive::setInternalInfo(nullptr);
    }
}
