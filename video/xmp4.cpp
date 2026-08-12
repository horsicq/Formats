/* Copyright (c) 2022-2026 hors<horsicq@gmail.com>
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
#include "xmp4.h"

XBinary::XCONVERT _TABLE_XMP4_STRUCTID[] = {
    {XMP4::STRUCTID_UNKNOWN, "Unknown", QObject::tr("Unknown")},
    {XMP4::STRUCTID_BOX, "BOX", QObject::tr("Box")},
    {XMP4::STRUCTID_HEADER, "HEADER", QObject::tr("Header")},
};

XMP4::XMP4(QIODevice *pDevice) : XBinary(pDevice)
{
}

XMP4::~XMP4()
{
}

bool XMP4::isValid(PDSTRUCT *pPdStruct)
{
    bool bResult = false;

    if (getSize() > 0x20) {
        _MEMORY_MAP memoryMap = XBinary::getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);

        if (compareSignature(&memoryMap, "000000..'ftyp'", 0, pPdStruct)) {
            // TODO more checks !!!
            bResult = true;
        }
    }

    return bResult;
}

bool XMP4::isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    XMP4 mp4(pDevice);

    return mp4.isValid(pPdStruct);
}

QString XMP4::getFileFormatExt()
{
    return "mp4";
}

QString XMP4::getFileFormatExtsString()
{
    return "MP4";
}

qint64 XMP4::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    return _calculateRawSize(pPdStruct);
}

QString XMP4::getMIMEString()
{
    return "video/mp4";
}

XBinary::_MEMORY_MAP XMP4::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(mapMode)
    return _getMemoryMap(FILEPART_HEADER | FILEPART_REGION | FILEPART_OVERLAY, pPdStruct);
}

XBinary::FT XMP4::getFileType()
{
    return FT_MP4;
}

bool XMP4::isTagValid(const QString &sTagName)
{
    bool bResult = false;

    if ((sTagName == "ftyp") || (sTagName == "pdin") || (sTagName == "moov") || (sTagName == "moof") || (sTagName == "mfra") || (sTagName == "mdat") ||
        (sTagName == "stts") || (sTagName == "stsc") || (sTagName == "stsz") || (sTagName == "meta") || (sTagName == "mvhd") || (sTagName == "trak") ||
        (sTagName == "udta") || (sTagName == "iods") || (sTagName == "uuid") || (sTagName == "free")) {
        bResult = true;
    } else {
#ifdef QT_DEBUG
        qDebug("%s", sTagName.toLatin1().data());
#endif
    }

    return bResult;
}

// QList<XBinary::DATA_HEADER> XMP4::getDataHeaders(const DATA_HEADERS_OPTIONS &dataHeadersOptions, PDSTRUCT *pPdStruct)
// {
//     Q_UNUSED(pPdStruct)
//     QList<DATA_HEADER> list;

//     if (!(dataHeadersOptions.nID) || (dataHeadersOptions.nID == STRUCTID_BOX)) {
//         DATA_HEADERS_OPTIONS opt = dataHeadersOptions;
//         opt.nID = STRUCTID_BOX;
//         opt.dhMode = DHMODE_TABLE;
//         DATA_HEADER t = _initDataHeader(opt, tr("Boxes"));
//         t.locType = LT_OFFSET;
//         t.nLocation = 0;
//         t.nSize = getSize();
//         t.listRecords.append(getDataRecord(0, 4, "size", VT_UINT32, DRF_SIZE, ENDIAN_BIG));
//         t.listRecords.append(getDataRecord(4, 4, "type", VT_ANSI, DRF_UNKNOWN, ENDIAN_LITTLE));
//         list.append(t);
//     }

//     return list;
// }

QList<XBinary::FPART> XMP4::getFileParts(quint32 nFileParts, qint32 nLimit, PDSTRUCT *pPdStruct)
{
    QList<FPART> list;

    if ((nLimit < -1) || (nLimit == 0) || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return list;
    }

    const qint64 nTotal = getSize();
    const auto canAppend = [&]() -> bool { return (nLimit == -1) || (list.size() < nLimit); };

    if ((nFileParts & FILEPART_HEADER) && canAppend()) {
        FPART h = {};
        h.filePart = FILEPART_HEADER;
        h.nFileOffset = 0;
        h.nFileSize = qMin<qint64>(nTotal, 8);  // size(4)+type(4) of first box
        h.nVirtualAddress = -1;
        h.sName = tr("Header");
        list.append(h);
    }

    qint64 nParsedEnd = 0;
    if ((nFileParts & (FILEPART_REGION | FILEPART_OVERLAY)) && canAppend()) {
        qint64 nOffset = 0;
        while ((nOffset <= (nTotal - 8)) && XBinary::isPdStructNotCanceled(pPdStruct)) {
            const quint32 nSize32 = read_uint32(nOffset, true);
            const QString sType = read_ansiString(nOffset + 4, 4);
            const qint64 nRemaining = nTotal - nOffset;
            quint64 nBoxSize = nSize32;
            bool bExtendsToEnd = false;

            if (!isTagValid(sType)) {
                break;
            }

            if (nSize32 == 0) {
                nBoxSize = (quint64)nRemaining;
                bExtendsToEnd = true;
            } else if (nSize32 == 1) {
                if (nRemaining < 16) break;
                nBoxSize = read_uint64(nOffset + 8, true);
                if (nBoxSize < 16) break;
            }

            if ((nBoxSize < 8) || (nBoxSize > (quint64)nRemaining)) break;

            if ((nFileParts & FILEPART_REGION) && canAppend()) {
                FPART f = {};
                f.filePart = FILEPART_REGION;
                f.nFileOffset = nOffset;
                f.nFileSize = (qint64)nBoxSize;
                f.nVirtualAddress = -1;
                f.sName = sType;
                list.append(f);
            }

            nOffset += (qint64)nBoxSize;
            nParsedEnd = nOffset;
            if (!canAppend() || bExtendsToEnd) break;
        }
    }

    if (!XBinary::isPdStructNotCanceled(pPdStruct)) return QList<FPART>();

    if ((nFileParts & FILEPART_OVERLAY) && canAppend()) {
        if (nParsedEnd < nTotal) {
            FPART ov = {};
            ov.filePart = FILEPART_OVERLAY;
            ov.nFileOffset = nParsedEnd;
            ov.nFileSize = nTotal - nParsedEnd;
            ov.nVirtualAddress = -1;
            ov.sName = tr("Overlay");
            list.append(ov);
        }
    }

    return list;
}

QList<XBinary::MAPMODE> XMP4::getMapModesList()
{
    QList<MAPMODE> listResult;

    listResult.append(MAPMODE_REGIONS);

    return listResult;
}

QString XMP4::structIDToString(quint32 nID)
{
    return XBinary::XCONVERT_idToTransString(nID, _TABLE_XMP4_STRUCTID, sizeof(_TABLE_XMP4_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QString XMP4::structIDToFtString(quint32 nID)
{
    return XBinary::XCONVERT_idToFtString(nID, _TABLE_XMP4_STRUCTID, sizeof(_TABLE_XMP4_STRUCTID) / sizeof(XBinary::XCONVERT));
}

quint32 XMP4::ftStringToStructID(const QString &sFtString)
{
    return XCONVERT_ftStringToId(sFtString, _TABLE_XMP4_STRUCTID, sizeof(_TABLE_XMP4_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QList<XBinary::XFHEADER> XMP4::getXFHeaders(const XFSTRUCT &xfStruct, PDSTRUCT *pPdStruct)
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

        if (nHeaderOffset != -1) {
            XFHEADER xfHeader = {};
            xfHeader.sParentTag = xfStruct.sParent;
            xfHeader.fileType = xfStruct.fileType;
            xfHeader.structID = static_cast<XBinary::STRUCTID>(STRUCTID_HEADER);
            xfHeader.xLoc = headerLoc;
            xfHeader.nSize = read_uint32(nHeaderOffset, true);
            xfHeader.xfType = XFTYPE_HEADER;
            xfHeader.listFields = getXFRecords(xfStruct.fileType, STRUCTID_HEADER, headerLoc);
            xfHeader.sTag = xfHeaderToTag(xfHeader, structIDToString(STRUCTID_HEADER), xfHeader.sParentTag);
            listResult.append(xfHeader);

            if (xfStruct.bIsParent) {
                XFSTRUCT _xfStruct = xfStruct;
                _xfStruct.sParent = xfHeader.sTag;
                _xfStruct.nStructID = STRUCTID_BOX;
                _xfStruct.xLoc = offsetToLoc(0);
                listResult.append(getXFHeaders(_xfStruct, pPdStruct));
            }
        }
    } else if (nStructID == STRUCTID_BOX) {
        qint64 nStartOffset = locToOffset(xfStruct.pMemoryMap, xfStruct.xLoc);

        if (nStartOffset == -1) {
            nStartOffset = 0;
        }

        qint64 nFileSize = getSize();

        XFHEADER xfHeader = {};
        xfHeader.sParentTag = xfStruct.sParent;
        xfHeader.fileType = xfStruct.fileType;
        xfHeader.structID = static_cast<XBinary::STRUCTID>(STRUCTID_BOX);
        xfHeader.xLoc = offsetToLoc(nStartOffset);
        xfHeader.xfType = XFTYPE_TABLE;
        xfHeader.listFields = getXFRecords(xfStruct.fileType, STRUCTID_BOX, xfHeader.xLoc);

        qint64 nCurrentOffset = nStartOffset;

        while (((nCurrentOffset + 8) <= nFileSize) && XBinary::isPdStructNotCanceled(pPdStruct)) {
            quint32 nBoxSize = read_uint32(nCurrentOffset, true);
            QString sType = read_ansiString(nCurrentOffset + 4, 4);

            if (!isTagValid(sType)) {
                break;
            }

            xfHeader.listRowLocations.append(nCurrentOffset);

            qint64 nRealSize = nBoxSize;

            if (nBoxSize == 0) {
                break;  // Box extends to the end of the file
            } else if (nBoxSize == 1) {
                nRealSize = read_uint64(nCurrentOffset + 8, true);  // 64-bit largesize
            }

            if (nRealSize < 8) {
                break;
            }

            nCurrentOffset += nRealSize;
        }

        if (!xfHeader.listRowLocations.isEmpty()) {
            xfHeader.sTag = xfHeaderToTag(xfHeader, structIDToString(STRUCTID_BOX), xfHeader.sParentTag);
            listResult.append(xfHeader);
        }
    }

    return listResult;
}

QList<XBinary::XFRECORD> XMP4::getXFRecords(FT fileType, quint32 nStructID, const XLOC &xLoc)
{
    Q_UNUSED(fileType)
    Q_UNUSED(xLoc)

    QList<XBinary::XFRECORD> listResult;

    // MP4/QuickTime box sizes are big-endian
    if (nStructID == STRUCTID_HEADER) {
        listResult.append({"Size", 0, 4, XFRECORD_FLAG_BE | XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"Type", 4, 4, XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        listResult.append({"MajorBrand", 8, 4, XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        listResult.append({"MinorVersion", 12, 4, XFRECORD_FLAG_BE | XFRECORD_FLAG_VERSION, VT_UINT32});
    } else if (nStructID == STRUCTID_BOX) {
        listResult.append({"Size", 0, 4, XFRECORD_FLAG_BE | XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"Type", 4, 4, XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
    }

    return listResult;
}

QList<QString> XMP4::getSearchSignatures()
{
    QList<QString> listResult;

    listResult.append("000000..'ftyp'");

    return listResult;
}

XBinary *XMP4::createInstance(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    Q_UNUSED(bIsImage)
    Q_UNUSED(nModuleAddress)

    return new XMP4(pDevice);
}

bool XMP4::handleInternalInfo(PDSTRUCT *pPdStruct)
{
    bool bResult = true;

    if (!isInternalInfoHandled()) {
        bResult = XBinary::handleInternalInfo(pPdStruct);

        if (bResult) {
            static_cast<XBinary::INTERNAL_INFO &>(m_internalInfo) =
                *static_cast<XBinary::INTERNAL_INFO *>(XBinary::getInternalInfo(pPdStruct));
            setIsInternalInfoHandled(true);
        }
    }

    return bResult;
}

void *XMP4::getInternalInfo(PDSTRUCT *pPdStruct)
{
    handleInternalInfo(pPdStruct);

    return &m_internalInfo;
}

void XMP4::setInternalInfo(void *pInternalInfo)
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
