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
#include "xgif.h"
namespace {
const qint32 GIF_MAX_BLOCK_COUNT = 65536;
}
static XBinary::XCONVERT _TABLE_XGIF_STRUCTID[] = {
    {XGif::STRUCTID_UNKNOWN, "Unknown", QObject::tr("Unknown")},
    {XGif::STRUCTID_SIGNATURE, "Signature", QString("Signature")},
    {XGif::STRUCTID_LOGICAL_SCREEN_DESCRIPTOR, "LSD", QString("Logical Screen Descriptor")},
    {XGif::STRUCTID_GLOBAL_COLOR_TABLE, "GCT", QString("Global Color Table")},
    {XGif::STRUCTID_BLOCK, "BLOCK", QString("Block")},
    {XGif::STRUCTID_IMAGE_DESCRIPTOR, "ID", QString("Image Descriptor")},
    {XGif::STRUCTID_GRAPHIC_CONTROL_EXTENSION, "GCE", QString("Graphic Control Extension")},
    {XGif::STRUCTID_COMMENT_EXTENSION, "COM", QString("Comment Extension")},
    {XGif::STRUCTID_APPLICATION_EXTENSION, "APP", QString("Application Extension")},
    {XGif::STRUCTID_PLAIN_TEXT_EXTENSION, "PTX", QString("Plain Text Extension")},
    {XGif::STRUCTID_TRAILER, "TRAILER", QString("Trailer")},
};

XGif::XGif(QIODevice *pDevice) : XBinary(pDevice)
{
}

XGif::~XGif()
{
}

bool XGif::isValid(PDSTRUCT *pPdStruct)
{
    return _getStructuredSize(pPdStruct) > 0;
}

bool XGif::isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    XGif xgif(pDevice);

    return xgif.isValid(pPdStruct);
}

QString XGif::getFileFormatExt()
{
    return "gif";
}

QString XGif::getFileFormatExtsString()
{
    return "gif";
}

qint64 XGif::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    return _getStructuredSize(pPdStruct);
}

qint64 XGif::_getStructuredSize(PDSTRUCT *pPdStruct)
{
    const qint64 nTotalSize = getSize();

    if ((nTotalSize < 14) || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return 0;
    }

    const QByteArray baSignature = read_array(0, 6);

    if ((baSignature != QByteArrayLiteral("GIF87a")) && (baSignature != QByteArrayLiteral("GIF89a"))) {
        return 0;
    }

    if ((read_uint16(6) == 0) || (read_uint16(8) == 0)) {
        return 0;
    }

    const quint8 nPacked = read_uint8(10);
    qint64 nOffset = 13;
    qint32 nBlockCount = 0;

    if (nPacked & 0x80) {
        const qint64 nColorTableSize = 3 * ((qint64)1 << ((nPacked & 7) + 1));

        if (nColorTableSize > nTotalSize - nOffset) {
            return 0;
        }

        nOffset += nColorTableSize;
    }

    while ((nOffset < nTotalSize) && XBinary::isPdStructNotCanceled(pPdStruct)) {
        if (nBlockCount++ >= GIF_MAX_BLOCK_COUNT) {
            return 0;
        }

        const quint8 nMarker = read_uint8(nOffset);

        if (nMarker == 0x3B) {
            return XBinary::isPdStructNotCanceled(pPdStruct) ? nOffset + 1 : 0;
        }

        if (nMarker == 0x2C) {
            if (nOffset > nTotalSize - 10) {
                return 0;
            }

            if ((read_uint16(nOffset + 5) == 0) || (read_uint16(nOffset + 7) == 0)) {
                return 0;
            }

            const quint8 nImagePacked = read_uint8(nOffset + 9);
            nOffset += 10;

            if (nImagePacked & 0x80) {
                const qint64 nLocalTableSize = 3 * ((qint64)1 << ((nImagePacked & 7) + 1));

                if (nLocalTableSize > nTotalSize - nOffset) {
                    return 0;
                }

                nOffset += nLocalTableSize;
            }

            if (nOffset >= nTotalSize) {
                return 0;
            }

            const quint8 nMinimumCodeSize = read_uint8(nOffset++);

            if ((nMinimumCodeSize < 2) || (nMinimumCodeSize > 8)) {
                return 0;
            }
        } else if (nMarker == 0x21) {
            if (nOffset > nTotalSize - 2) {
                return 0;
            }

            nOffset += 2;
        } else {
            return 0;
        }

        bool bTerminated = false;

        while ((nOffset < nTotalSize) && XBinary::isPdStructNotCanceled(pPdStruct)) {
            if (nBlockCount++ >= GIF_MAX_BLOCK_COUNT) {
                return 0;
            }

            const quint8 nBlockSize = read_uint8(nOffset++);

            if (nBlockSize == 0) {
                bTerminated = true;
                break;
            }

            if (nBlockSize > nTotalSize - nOffset) {
                return 0;
            }

            nOffset += nBlockSize;
        }

        if (!bTerminated) {
            return 0;
        }
    }

    return 0;
}

QString XGif::getMIMEString()
{
    return "image/gif";
}

QVector<XBinary::XMETADATA_STRUCT> XGif::getMetadataStructs()
{
    QVector<XMETADATA_STRUCT> listResult;
    if (!isValid((PDSTRUCT *)nullptr) || !checkOffsetSize(6, 5)) {
        return listResult;
    }

    auto appendMetadata = [this, &listResult](qint64 nOffset, XMETADATA_ID id, const QString &sName, const QVariant &varValue) {
        XMETADATA_STRUCT record = {};
        record.nOffset = nOffset;
        record.nSize = (id == XMETADATA_ID_BIT_DEPTH) ? 1 : 2;
        record.nAddress = offsetToAddress(nOffset);
        record.id = id;
        record.sName = sName;
        record.varValue = varValue;
        listResult.append(record);
    };

    appendMetadata(6, XMETADATA_ID_FRAME_WIDTH, QString("Logical screen width"), read_uint16(6, false));
    appendMetadata(8, XMETADATA_ID_FRAME_HEIGHT, QString("Logical screen height"), read_uint16(8, false));
    appendMetadata(10, XMETADATA_ID_BIT_DEPTH, QString("Color resolution"), ((read_uint8(10) >> 4) & 7) + 1);

    return listResult;
}

QString XGif::structIDToString(quint32 nID)
{
    return XBinary::XCONVERT_idToTransString(nID, _TABLE_XGIF_STRUCTID, sizeof(_TABLE_XGIF_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QString XGif::structIDToFtString(quint32 nID)
{
    return XBinary::XCONVERT_idToFtString(nID, _TABLE_XGIF_STRUCTID, sizeof(_TABLE_XGIF_STRUCTID) / sizeof(XBinary::XCONVERT));
}

quint32 XGif::ftStringToStructID(const QString &sFtString)
{
    return XCONVERT_ftStringToId(sFtString, _TABLE_XGIF_STRUCTID, sizeof(_TABLE_XGIF_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QList<XBinary::XFHEADER> XGif::getXFHeaders(const XFSTRUCT &xfStruct, PDSTRUCT *pPdStruct)
{
    QList<XBinary::XFHEADER> listResult;

    if (!isValid(pPdStruct)) {
        return listResult;
    }

    quint32 nStructID = xfStruct.nStructID;

    if (nStructID == STRUCTID_UNKNOWN) {
        XFSTRUCT _xfStruct = xfStruct;
        _xfStruct.nStructID = STRUCTID_SIGNATURE;
        _xfStruct.xLoc = offsetToLoc(0);
        listResult.append(getXFHeaders(_xfStruct, pPdStruct));
    } else if (nStructID == STRUCTID_SIGNATURE) {
        XLOC headerLoc = xfStruct.xLoc;
        if (headerLoc.locType == LT_UNKNOWN) {
            headerLoc = offsetToLoc(0);
        }

        XFHEADER xfHeader = {};
        xfHeader.sParentTag = xfStruct.sParent;
        xfHeader.fileType = xfStruct.fileType;
        xfHeader.structID = static_cast<XBinary::STRUCTID>(STRUCTID_SIGNATURE);
        xfHeader.xLoc = headerLoc;
        xfHeader.nSize = 6;
        xfHeader.xfType = XFTYPE_HEADER;
        xfHeader.listFields = getXFRecords(xfStruct.fileType, STRUCTID_SIGNATURE, headerLoc);
        xfHeader.sTag = xfHeaderToTag(xfHeader, structIDToString(STRUCTID_SIGNATURE), xfHeader.sParentTag);
        listResult.append(xfHeader);

        if (xfStruct.bIsParent) {
            XFSTRUCT _xfStruct = xfStruct;
            _xfStruct.sParent = xfHeader.sTag;
            _xfStruct.nStructID = STRUCTID_LOGICAL_SCREEN_DESCRIPTOR;
            _xfStruct.xLoc = offsetToLoc(6);
            listResult.append(getXFHeaders(_xfStruct, pPdStruct));
        }
    } else if (nStructID == STRUCTID_LOGICAL_SCREEN_DESCRIPTOR) {
        XLOC headerLoc = xfStruct.xLoc;
        if (headerLoc.locType == LT_UNKNOWN) {
            headerLoc = offsetToLoc(6);
        }

        XFHEADER xfHeader = {};
        xfHeader.sParentTag = xfStruct.sParent;
        xfHeader.fileType = xfStruct.fileType;
        xfHeader.structID = static_cast<XBinary::STRUCTID>(STRUCTID_LOGICAL_SCREEN_DESCRIPTOR);
        xfHeader.xLoc = headerLoc;
        xfHeader.nSize = 7;
        xfHeader.xfType = XFTYPE_HEADER;
        xfHeader.listFields = getXFRecords(xfStruct.fileType, STRUCTID_LOGICAL_SCREEN_DESCRIPTOR, headerLoc);
        xfHeader.sTag = xfHeaderToTag(xfHeader, structIDToString(STRUCTID_LOGICAL_SCREEN_DESCRIPTOR), xfHeader.sParentTag);
        listResult.append(xfHeader);
    }

    return listResult;
}

QList<XBinary::XFRECORD> XGif::getXFRecords(FT fileType, quint32 nStructID, const XLOC &xLoc)
{
    Q_UNUSED(fileType)
    Q_UNUSED(xLoc)

    QList<XBinary::XFRECORD> listResult;

    if (nStructID == STRUCTID_SIGNATURE) {
        listResult.append({"Signature", 0, 3, XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        listResult.append({"Version", 3, 3, XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
    } else if (nStructID == STRUCTID_LOGICAL_SCREEN_DESCRIPTOR) {
        listResult.append({"Width", 0, 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"Height", 2, 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"Packed", 4, 1, XFRECORD_FLAG_NONE, VT_UINT8});
        listResult.append({"BackgroundColorIndex", 5, 1, XFRECORD_FLAG_NONE, VT_UINT8});
        listResult.append({"PixelAspectRatio", 6, 1, XFRECORD_FLAG_NONE, VT_UINT8});
    } else if (nStructID == STRUCTID_IMAGE_DESCRIPTOR) {
        listResult.append({"Separator", 0, 1, XFRECORD_FLAG_NONE, VT_UINT8});
        listResult.append({"Left", 1, 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"Top", 3, 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"Width", 5, 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"Height", 7, 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"Packed", 9, 1, XFRECORD_FLAG_NONE, VT_UINT8});
    }

    return listResult;
}

// QList<XBinary::DATA_HEADER> XGif::getDataHeaders(const DATA_HEADERS_OPTIONS &dataHeadersOptions, PDSTRUCT *pPdStruct)
// {
//     QList<DATA_HEADER> listResult;

//     if (dataHeadersOptions.nID == STRUCTID_UNKNOWN) {
//         DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
//         _dataHeadersOptions.bChildren = true;
//         _dataHeadersOptions.dsID_parent = _addDefaultHeaders(&listResult, pPdStruct);
//         _dataHeadersOptions.dhMode = XBinary::DHMODE_HEADER;
//         _dataHeadersOptions.fileType = dataHeadersOptions.pMemoryMap->fileType;

//         _dataHeadersOptions.nID = STRUCTID_SIGNATURE;
//         _dataHeadersOptions.nLocation = 0;
//         _dataHeadersOptions.locType = XBinary::LT_OFFSET;

//         listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//     } else {
//         qint64 nStartOffset = locationToOffset(dataHeadersOptions.pMemoryMap, dataHeadersOptions.locType, dataHeadersOptions.nLocation);

//         if (nStartOffset != -1) {
//             if (dataHeadersOptions.nID == STRUCTID_SIGNATURE) {
//                 DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, XGif::structIDToString(dataHeadersOptions.nID));
//                 dataHeader.nSize = 6;  // GIF87a/GIF89a
//                 dataHeader.listRecords.append(getDataRecord(0, 6, "Signature", VT_CHAR_ARRAY, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 listResult.append(dataHeader);

//                 if (dataHeadersOptions.bChildren) {
//                     // Logical Screen Descriptor follows at offset 6
//                     DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
//                     _dataHeadersOptions.dhMode = XBinary::DHMODE_HEADER;
//                     _dataHeadersOptions.nID = STRUCTID_LOGICAL_SCREEN_DESCRIPTOR;
//                     _dataHeadersOptions.nLocation = dataHeadersOptions.nLocation + 6;
//                     _dataHeadersOptions.locType = dataHeadersOptions.locType;
//                     listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//                 }
//             } else if (dataHeadersOptions.nID == STRUCTID_LOGICAL_SCREEN_DESCRIPTOR) {
//                 DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, XGif::structIDToString(dataHeadersOptions.nID));
//                 dataHeader.nSize = 7;  // Width(2), Height(2), Packed(1), BgColor(1), PixelAspect(1)
//                 dataHeader.listRecords.append(getDataRecord(0, 2, "Width", VT_UINT16, DRF_COUNT, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(2, 2, "Height", VT_UINT16, DRF_COUNT, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(4, 1, "Packed", VT_UINT8, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(5, 1, "Background Color Index", VT_UINT8, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(6, 1, "Pixel Aspect Ratio", VT_UINT8, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 listResult.append(dataHeader);

//                 if (dataHeadersOptions.bChildren) {
//                     quint8 nPacked = read_uint8(nStartOffset + 4);
//                     bool bGCT = (nPacked & 0x80) != 0;  // Global Color Table Flag
//                     quint8 nGCTSizeCode = (nPacked & 0x07);
//                     qint64 nNextOffset = nStartOffset + 7;

//                     if (bGCT) {
//                         // Size is 3 * 2^(n+1)
//                         qint64 nGCTSize = 3 * ((qint64)1 << (nGCTSizeCode + 1));
//                         DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
//                         _dataHeadersOptions.dhMode = XBinary::DHMODE_HEADER;
//                         _dataHeadersOptions.nID = STRUCTID_GLOBAL_COLOR_TABLE;
//                         _dataHeadersOptions.nLocation = dataHeadersOptions.nLocation + 7;
//                         _dataHeadersOptions.nSize = nGCTSize;
//                         _dataHeadersOptions.locType = dataHeadersOptions.locType;
//                         listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//                         nNextOffset += nGCTSize;
//                     }

//                     // Add a generic block table starting at next offset
//                     // We don’t expand children here; FileParts can present detailed blocks
//                     DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
//                     _dataHeadersOptions.dhMode = XBinary::DHMODE_TABLE;
//                     _dataHeadersOptions.nID = STRUCTID_BLOCK;
//                     _dataHeadersOptions.nLocation = nNextOffset;
//                     _dataHeadersOptions.locType = LT_OFFSET;
//                     // Count unknown; leave nCount=0, nSize=remaining bytes
//                     _dataHeadersOptions.nSize = getSize() - nNextOffset;
//                     listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//                 }
//             } else if (dataHeadersOptions.nID == STRUCTID_GLOBAL_COLOR_TABLE) {
//                 DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, XGif::structIDToString(dataHeadersOptions.nID));
//                 dataHeader.nSize = dataHeadersOptions.nSize;
//                 listResult.append(dataHeader);
//             } else if (dataHeadersOptions.nID == STRUCTID_BLOCK) {
//                 // Represent the sequence of blocks as a table; each entry spans one complete block
//                 DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, XGif::structIDToString(dataHeadersOptions.nID));

//                 // We’ll fill nCount by walking blocks once to compute extent
//                 qint64 nCurrentOffset = nStartOffset;
//                 qint64 nEndOffset = getSize();
//                 qint32 nCount = 0;

//                 while ((nCurrentOffset < nEndOffset) && XBinary::isPdStructNotCanceled(pPdStruct)) {
//                     quint8 marker = read_uint8(nCurrentOffset);
//                     if (marker == 0x3B) {  // Trailer
//                         nCount++;
//                         nCurrentOffset += 1;
//                         break;
//                     } else if (marker == 0x2C) {  // Image Descriptor
//                         // Skip Image Descriptor (10 bytes) + optional Local Color Table + Image Data (LZW blocks)
//                         qint64 nBlockStart = nCurrentOffset;
//                         nCurrentOffset += 10;  // descriptor
//                         quint8 packed = read_uint8(nBlockStart + 9);
//                         if (packed & 0x80) {  // Local Color Table
//                             quint8 sizeCode = (packed & 0x07);
//                             nCurrentOffset += 3 * ((qint64)1 << (sizeCode + 1));
//                         }
//                         // LZW minimum code size + sub-blocks
//                         nCurrentOffset += 1;
//                         while (XBinary::isPdStructNotCanceled(pPdStruct)) {
//                             quint8 sub = read_uint8(nCurrentOffset++);
//                             if (sub == 0) break;
//                             nCurrentOffset += sub;
//                             if (nCurrentOffset > nEndOffset) break;
//                         }
//                         nCount++;
//                     } else if (marker == 0x21) {  // Extension
//                         quint8 label = read_uint8(nCurrentOffset + 1);
//                         qint64 nBlockStart = nCurrentOffset;
//                         nCurrentOffset += 2;  // skip introducer + label
//                         // Sub-blocks until size 0
//                         while (XBinary::isPdStructNotCanceled(pPdStruct)) {
//                             quint8 sub = read_uint8(nCurrentOffset++);
//                             if (sub == 0) break;
//                             nCurrentOffset += sub;
//                             if (nCurrentOffset > nEndOffset) break;
//                         }
//                         Q_UNUSED(label)
//                         nCount++;
//                     } else {
//                         break;  // Unknown marker
//                     }
//                 }

//                 dataHeader.nCount = nCount;
//                 dataHeader.nSize = qMax<qint64>(0, nCurrentOffset - nStartOffset);
//                 listResult.append(dataHeader);
//             }
//         }
//     }

//     return listResult;
// }

QList<XBinary::FPART> XGif::getFileParts(quint32 nFileParts, qint32 nLimit, PDSTRUCT *pPdStruct)
{
    QList<XBinary::FPART> listResult;

    if ((nLimit < -1) || (nLimit == 0)) {
        return listResult;
    }

    qint64 nTotalSize = getSize();

    if (nTotalSize <= 0) {
        return listResult;
    }

    if (_getStructuredSize(pPdStruct) == 0) {
        return listResult;
    }

    if (nFileParts & FILEPART_SIGNATURE) {
        FPART rec = {};
        rec.filePart = FILEPART_SIGNATURE;
        rec.nFileOffset = 0;
        rec.nFileSize = qMin<qint64>(6, nTotalSize);
        rec.nVirtualAddress = (XADDR)-1;
        rec.sName = tr("Signature");
        listResult.append(rec);
        if ((nLimit != -1) && (listResult.count() >= nLimit)) return listResult;
    }

    // Logical Screen Descriptor and optional GCT
    qint64 nOffset = 6;
    if ((nFileParts & FILEPART_HEADER) && (nOffset < nTotalSize)) {
        FPART rec = {};
        rec.filePart = FILEPART_HEADER;
        rec.nFileOffset = nOffset;
        rec.nFileSize = qMin<qint64>(7, nTotalSize - nOffset);
        rec.nVirtualAddress = (XADDR)-1;
        rec.sName = tr("Logical Screen Descriptor");
        listResult.append(rec);
        if ((nLimit != -1) && (listResult.count() >= nLimit)) return listResult;
    }

    if ((nOffset < 0) || (nOffset > nTotalSize - 7)) {
        return listResult;
    }

    quint8 packed = read_uint8(nOffset + 4);
    bool bGCT = (packed & 0x80) != 0;
    quint8 gctSizeCode = (packed & 0x07);
    nOffset += 7;

    if (bGCT) {
        qint64 gctSize = 3 * ((qint64)1 << (gctSizeCode + 1));

        if (gctSize > nTotalSize - nOffset) {
            return listResult;
        }

        if (nFileParts & FILEPART_TABLE) {
            FPART rec = {};
            rec.filePart = FILEPART_TABLE;
            rec.nFileOffset = nOffset;
            rec.nFileSize = gctSize;
            rec.nVirtualAddress = (XADDR)-1;
            rec.sName = tr("Global Color Table");
            listResult.append(rec);
            if ((nLimit != -1) && (listResult.count() >= nLimit)) return listResult;
        }

        nOffset += gctSize;
    }

    // Blocks loop (sections/regions)
    qint32 nBlockCount = 0;
    while ((nOffset < nTotalSize) && XBinary::isPdStructNotCanceled(pPdStruct)) {
        if (nBlockCount++ >= GIF_MAX_BLOCK_COUNT) {
            listResult.clear();
            return listResult;
        }

        quint8 marker = read_uint8(nOffset);
        if (marker == 0x3B) {  // Trailer
            if (nFileParts & FILEPART_FOOTER) {
                FPART rec = {};
                rec.filePart = FILEPART_FOOTER;
                rec.nFileOffset = nOffset;
                rec.nFileSize = 1;
                rec.nVirtualAddress = (XADDR)-1;
                rec.sName = tr("Trailer");
                listResult.append(rec);
                if ((nLimit != -1) && (listResult.count() >= nLimit)) return listResult;
            }
            nOffset += 1;
            break;
        } else if (marker == 0x2C) {  // Image Descriptor
            qint64 start = nOffset;

            if (start > nTotalSize - 10) {
                break;
            }

            nOffset += 10;
            quint8 ipacked = read_uint8(start + 9);
            if (ipacked & 0x80) {  // Local Color Table
                quint8 sizeCode = (ipacked & 0x07);
                qint64 nLocalTableSize = 3 * ((qint64)1 << (sizeCode + 1));

                if (nLocalTableSize > nTotalSize - nOffset) {
                    nOffset = start;
                    break;
                }

                nOffset += nLocalTableSize;
            }

            // LZW
            if (nOffset >= nTotalSize) {
                nOffset = start;
                break;
            }

            nOffset += 1;  // LZW min code size

            bool bComplete = false;
            while ((nOffset < nTotalSize) && XBinary::isPdStructNotCanceled(pPdStruct)) {
                if (nBlockCount++ >= GIF_MAX_BLOCK_COUNT) {
                    listResult.clear();
                    return listResult;
                }

                quint8 sub = read_uint8(nOffset++);

                if (sub == 0) {
                    bComplete = true;
                    break;
                }

                if (sub > nTotalSize - nOffset) {
                    break;
                }

                nOffset += sub;
            }

            if (!bComplete) {
                nOffset = start;
                break;
            }

            if (nFileParts & FILEPART_REGION) {
                FPART rec = {};
                rec.filePart = FILEPART_REGION;
                rec.nFileOffset = start;
                rec.nFileSize = nOffset - start;
                rec.nVirtualAddress = (XADDR)-1;
                rec.sName = tr("Image");
                listResult.append(rec);
                if ((nLimit != -1) && (listResult.count() >= nLimit)) return listResult;
            }
        } else if (marker == 0x21) {  // Extension
            if (nOffset > nTotalSize - 2) {
                break;
            }

            quint8 label = read_uint8(nOffset + 1);
            qint64 start = nOffset;
            nOffset += 2;

            bool bComplete = false;
            while ((nOffset < nTotalSize) && XBinary::isPdStructNotCanceled(pPdStruct)) {
                if (nBlockCount++ >= GIF_MAX_BLOCK_COUNT) {
                    listResult.clear();
                    return listResult;
                }

                quint8 sub = read_uint8(nOffset++);

                if (sub == 0) {
                    bComplete = true;
                    break;
                }

                if (sub > nTotalSize - nOffset) {
                    break;
                }

                nOffset += sub;
            }

            if (!bComplete) {
                nOffset = start;
                break;
            }

            if (nFileParts & FILEPART_SECTION) {
                FPART rec = {};
                rec.filePart = FILEPART_SECTION;
                rec.nFileOffset = start;
                rec.nFileSize = nOffset - start;
                rec.nVirtualAddress = (XADDR)-1;
                switch (label) {
                    case 0xF9: rec.sName = tr("Graphic Control Extension"); break;
                    case 0xFE: rec.sName = tr("Comment Extension"); break;
                    case 0xFF: rec.sName = tr("Application Extension"); break;
                    case 0x01: rec.sName = tr("Plain Text Extension"); break;
                    default: rec.sName = tr("Extension"); break;
                }
                listResult.append(rec);
                if ((nLimit != -1) && (listResult.count() >= nLimit)) return listResult;
            }
        } else {
            break;  // Unknown marker
        }
    }

    if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
        listResult.clear();
        return listResult;
    }

    if (nFileParts & FILEPART_OVERLAY) {
        if (nOffset < nTotalSize) {
            FPART rec = {};
            rec.filePart = FILEPART_OVERLAY;
            rec.nFileOffset = nOffset;
            rec.nFileSize = nTotalSize - nOffset;
            rec.nVirtualAddress = (XADDR)-1;
            rec.sName = tr("Overlay");
            listResult.append(rec);
            if ((nLimit != -1) && (listResult.count() >= nLimit)) return listResult;
        }
    }

    return listResult;
}

XBinary::_MEMORY_MAP XGif::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    XBinary::PDSTRUCT pdStructEmpty = {};

    if (!pPdStruct) {
        pdStructEmpty = XBinary::createPdStruct();
        pPdStruct = &pdStructEmpty;
    }

    return XBinary::getMemoryMap(mapMode, pPdStruct);
}

XBinary::FT XGif::getFileType()
{
    return FT_GIF;
}

QList<QString> XGif::getSearchSignatures()
{
    QList<QString> listResult;

    listResult.append("'GIF8'");

    return listResult;
}

XBinary *XGif::createInstance(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    Q_UNUSED(bIsImage)
    Q_UNUSED(nModuleAddress)

    return new XGif(pDevice);
}

bool XGif::handleInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XGif> guardedThis(this);
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

void *XGif::getInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XGif> guardedThis(this);
    const bool bHandled = guardedThis->handleInternalInfo(pPdStruct);
    if (!guardedThis || !bHandled) return nullptr;

    return &guardedThis->m_internalInfo;
}

void XGif::setInternalInfo(void *pInternalInfo)
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
