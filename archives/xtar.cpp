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
#include "xtar.h"

#include <limits>
#include <new>

XTAR::XCONVERT _TABLE_XTAR_STRUCTID[] = {{XTAR::STRUCTID_UNKNOWN, "Unknown", QObject::tr("Unknown")},
                                         {XTAR::STRUCTID_POSIX_HEADER, "posix_header", QString("posix_header")}};

namespace {
QString tarBoundedString(const char *pData, qint32 nSize)
{
    if (!pData || (nSize <= 0)) return QString();

    QByteArray baValue(pData, nSize);
    const qint32 nNullPosition = baValue.indexOf('\0');
    if (nNullPosition >= 0) baValue.truncate(nNullPosition);
    return QString::fromUtf8(baValue);
}

bool tarWriteAll(QIODevice *pDevice, const char *pData, qint64 nSize, XBinary::PDSTRUCT *pPdStruct)
{
    if (!pDevice || (nSize < 0) || ((nSize > 0) && !pData) || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    qint64 nWritten = 0;
    while ((nWritten < nSize) && XBinary::isPdStructNotCanceled(pPdStruct)) {
        const qint64 nResult = pDevice->write(pData + nWritten, nSize - nWritten);
        if ((nResult <= 0) || (nResult > (nSize - nWritten))) return false;
        nWritten += nResult;
    }

    return (nWritten == nSize) && XBinary::isPdStructNotCanceled(pPdStruct);
}

void tarRollbackWrite(QIODevice *pDevice, qint64 nStartPosition)
{
    if (pDevice && !pDevice->isSequential() && (nStartPosition >= 0)) {
        XBinary::resize(pDevice, nStartPosition);
        pDevice->seek(nStartPosition);
    }
}

const qint64 TAR_MAX_METADATA_PAYLOAD = 1024 * 1024;
const qint32 TAR_MAX_METADATA_TEXT = 64 * 1024;
const qint32 TAR_MAX_PAX_KEY = 256;

bool tarIsRecognizedPaxKey(const QByteArray &baKey)
{
    return (baKey == "path") || (baKey == "linkpath") || (baKey == "size") || (baKey == "uid") || (baKey == "gid") || (baKey == "uname") || (baKey == "gname") ||
           (baKey == "mtime") || (baKey == "atime");
}

bool tarParseUnsignedDecimal(const QByteArray &baValue, quint64 nMaximum, quint64 *pValue)
{
    if (!pValue || baValue.isEmpty()) return false;

    quint64 nValue = 0;
    for (char cValue : baValue) {
        const quint8 nByte = static_cast<quint8>(cValue);
        if ((nByte < '0') || (nByte > '9')) return false;
        const quint64 nDigit = nByte - '0';
        if (nValue > ((nMaximum - nDigit) / 10)) return false;
        nValue = (nValue * 10) + nDigit;
    }

    *pValue = nValue;
    return true;
}

bool tarParsePaxTime(const QByteArray &baValue, qint64 *pMilliseconds)
{
    if (!pMilliseconds || baValue.isEmpty()) return false;

    qint32 nIndex = 0;
    bool bNegative = false;
    if (baValue.at(0) == '-') {
        bNegative = true;
        nIndex++;
    }
    if (nIndex >= baValue.size()) return false;

    const qint32 nDot = baValue.indexOf('.', nIndex);
    const QByteArray baSeconds = (nDot == -1) ? baValue.mid(nIndex) : baValue.mid(nIndex, nDot - nIndex);
    if (baSeconds.isEmpty()) return false;

    const quint64 nMaximumSeconds = static_cast<quint64>((std::numeric_limits<qint64>::max)() / 1000);
    quint64 nSeconds = 0;
    if (!tarParseUnsignedDecimal(baSeconds, nMaximumSeconds, &nSeconds)) {
        return false;
    }

    quint64 nFractionMilliseconds = 0;
    bool bDiscardedNonZeroFraction = false;
    if (nDot != -1) {
        const QByteArray baFraction = baValue.mid(nDot + 1);
        if (baFraction.isEmpty()) return false;
        for (qint32 i = 0; i < baFraction.size(); i++) {
            const char cValue = baFraction.at(i);
            if ((cValue < '0') || (cValue > '9')) return false;
            if ((i >= 3) && (cValue != '0')) {
                bDiscardedNonZeroFraction = true;
            }
        }

        for (qint32 i = 0; i < 3; i++) {
            nFractionMilliseconds *= 10;
            if (i < baFraction.size()) {
                nFractionMilliseconds += baFraction.at(i) - '0';
            }
        }
    }

    const quint64 nMaximumPositive = static_cast<quint64>((std::numeric_limits<qint64>::max)());
    const quint64 nMaximumNegative = nMaximumPositive + 1;
    quint64 nMilliseconds = nSeconds * 1000 + nFractionMilliseconds;
    if (bNegative && bDiscardedNonZeroFraction) {
        if (nMilliseconds >= nMaximumNegative) return false;
        nMilliseconds++;
    }
    if (bNegative) {
        if (nMilliseconds > nMaximumNegative) return false;
        if (nMilliseconds == nMaximumNegative) {
            *pMilliseconds = (std::numeric_limits<qint64>::min)();
        } else {
            *pMilliseconds = -static_cast<qint64>(nMilliseconds);
        }
    } else {
        if (nMilliseconds > nMaximumPositive) return false;
        *pMilliseconds = static_cast<qint64>(nMilliseconds);
    }
    return true;
}

bool tarDecodeMetadataText(const QByteArray &baValue, QString *pValue)
{
    if (!pValue || baValue.isEmpty() || (baValue.size() > TAR_MAX_METADATA_TEXT) || baValue.contains('\0')) {
        return false;
    }

    const QString sValue = QString::fromUtf8(baValue);
    if (sValue.toUtf8() != baValue) return false;

    *pValue = sValue;
    return true;
}

bool tarIsValidPaxKey(const QByteArray &baKey)
{
    if (baKey.isEmpty() || (baKey.size() > TAR_MAX_PAX_KEY) || baKey.contains('=') || baKey.contains('\0')) {
        return false;
    }

    return QString::fromUtf8(baKey).toUtf8() == baKey;
}

bool tarValidateMetadata(const QMap<QByteArray, QByteArray> &mapMetadata)
{
    for (QMap<QByteArray, QByteArray>::const_iterator it = mapMetadata.constBegin(); it != mapMetadata.constEnd(); ++it) {
        const QByteArray &baKey = it.key();
        const QByteArray &baValue = it.value();
        // A zero-length value is a POSIX PAX tombstone.  Keep it in the
        // effective map so the corresponding raw/global field stays deleted.
        if (baValue.isEmpty()) continue;

        if ((baKey == "path") || (baKey == "linkpath")) {
            QString sPath;
            if (!tarDecodeMetadataText(baValue, &sPath)) return false;
        } else if ((baKey == "uname") || (baKey == "gname")) {
            QString sValue;
            if (!tarDecodeMetadataText(baValue, &sValue)) return false;
        } else if (baKey == "size") {
            quint64 nValue = 0;
            if (!tarParseUnsignedDecimal(baValue, static_cast<quint64>((std::numeric_limits<qint64>::max)()), &nValue)) {
                return false;
            }
        } else if ((baKey == "uid") || (baKey == "gid")) {
            quint64 nValue = 0;
            if (!tarParseUnsignedDecimal(baValue, static_cast<quint64>((std::numeric_limits<qint64>::max)()), &nValue)) {
                return false;
            }
        } else if ((baKey == "mtime") || (baKey == "atime")) {
            qint64 nMilliseconds = 0;
            if (!tarParsePaxTime(baValue, &nMilliseconds)) return false;
        }
    }

    return true;
}

bool tarValidateMetadataUpdate(const QMap<QByteArray, QByteArray> &mapMetadata)
{
    QMap<QByteArray, QByteArray> mapValues;
    for (QMap<QByteArray, QByteArray>::const_iterator it = mapMetadata.constBegin(); it != mapMetadata.constEnd(); ++it) {
        if (!it.value().isEmpty()) mapValues.insert(it.key(), it.value());
    }
    return tarValidateMetadata(mapValues);
}

bool tarParsePaxPayload(const QByteArray &baPayload, QMap<QByteArray, QByteArray> *pMetadata)
{
    if (!pMetadata || baPayload.isEmpty() || (baPayload.size() > TAR_MAX_METADATA_PAYLOAD)) {
        return false;
    }

    pMetadata->clear();
    qint32 nOffset = 0;
    while (nOffset < baPayload.size()) {
        const qint32 nSpace = baPayload.indexOf(' ', nOffset);
        if ((nSpace <= nOffset) || ((nSpace - nOffset) > 20)) return false;

        quint64 nLength = 0;
        if (!tarParseUnsignedDecimal(baPayload.mid(nOffset, nSpace - nOffset), static_cast<quint64>(baPayload.size() - nOffset), &nLength) ||
            (nLength <= static_cast<quint64>(nSpace - nOffset + 2))) {
            return false;
        }

        const qint64 nEnd64 = static_cast<qint64>(nOffset) + static_cast<qint64>(nLength);
        if ((nEnd64 > baPayload.size()) || (baPayload.at(static_cast<qint32>(nEnd64 - 1)) != '\n')) {
            return false;
        }

        const qint32 nValueStart = nSpace + 1;
        const qint32 nDataEnd = static_cast<qint32>(nEnd64 - 1);
        const qint32 nEquals = baPayload.indexOf('=', nValueStart);
        if ((nEquals <= nValueStart) || (nEquals >= nDataEnd) || ((nEquals - nValueStart) > TAR_MAX_PAX_KEY)) {
            return false;
        }

        const QByteArray baKey = baPayload.mid(nValueStart, nEquals - nValueStart);
        if (!tarIsValidPaxKey(baKey)) return false;

        const QByteArray baValue = baPayload.mid(nEquals + 1, nDataEnd - nEquals - 1);
        if (tarIsRecognizedPaxKey(baKey)) {
            pMetadata->insert(baKey, baValue);
        }

        nOffset = static_cast<qint32>(nEnd64);
    }

    return nOffset == baPayload.size();
}

void tarMergeGlobalMetadata(QMap<QByteArray, QByteArray> *pTarget, const QMap<QByteArray, QByteArray> &mapUpdate)
{
    if (!pTarget) return;
    for (QMap<QByteArray, QByteArray>::const_iterator it = mapUpdate.constBegin(); it != mapUpdate.constEnd(); ++it) {
        pTarget->insert(it.key(), it.value());
    }
}

QMap<QByteArray, QByteArray> tarEffectiveMetadata(const QMap<QByteArray, QByteArray> &mapGlobal, const QMap<QByteArray, QByteArray> &mapLocal)
{
    QMap<QByteArray, QByteArray> result = mapGlobal;
    for (QMap<QByteArray, QByteArray>::const_iterator it = mapLocal.constBegin(); it != mapLocal.constEnd(); ++it) {
        result.insert(it.key(), it.value());
    }
    return result;
}

bool tarParseGnuMetadata(const QByteArray &baPayload, QByteArray *pValue)
{
    if (!pValue || baPayload.isEmpty() || (baPayload.size() > TAR_MAX_METADATA_TEXT)) {
        return false;
    }

    const qint32 nNull = baPayload.indexOf('\0');
    if (nNull < 0) return false;
    for (qint32 i = nNull; i < baPayload.size(); i++) {
        if (baPayload.at(i) != '\0') return false;
    }

    const QByteArray baValue = baPayload.left(nNull);
    QString sValue;
    if (!tarDecodeMetadataText(baValue, &sValue)) return false;

    *pValue = baValue;
    return true;
}

bool tarGetRecordSize(qint64 nOffset, qint64 nTotalSize, qint64 nFileSize, qint64 *pRecordSize)
{
    if (!pRecordSize || (nOffset < 0) || (nTotalSize < 0) || (nOffset > nTotalSize) || (nFileSize < 0)) {
        return false;
    }

    const qint64 nMaximum = (std::numeric_limits<qint64>::max)();
    if (nFileSize > (nMaximum - 511)) return false;

    const qint64 nAlignedFileSize = ((nFileSize + 511) / 512) * 512;
    if (nAlignedFileSize > (nMaximum - 512)) return false;

    const qint64 nRecordSize = 512 + nAlignedFileSize;
    if (nRecordSize > (nTotalSize - nOffset)) return false;

    *pRecordSize = nRecordSize;
    return true;
}

}  // namespace

QString XTAR::_getRecordPath(const posix_header &header)
{
    const QString sName = tarBoundedString(header.name, (qint32)sizeof(header.name));
    const QString sPrefix = tarBoundedString(header.prefix, (qint32)sizeof(header.prefix));
    if (sPrefix.isEmpty()) return sName;
    if (sName.isEmpty()) return sPrefix;
    return sPrefix + QLatin1Char('/') + sName;
}

XTAR::XTAR(QIODevice *pDevice) : XArchive(pDevice)
{
}

bool XTAR::isValid(PDSTRUCT *pPdStruct)
{
    _MEMORY_MAP memoryMap = XBinary::getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);

    return _isValid(&memoryMap, 0, pPdStruct);
}

bool XTAR::_isValid(_MEMORY_MAP *pMemoryMap, qint64 nOffset, PDSTRUCT *pPdStruct)
{
    if (!pMemoryMap || !isOffsetAndSizeValid(pMemoryMap, nOffset, 512)) {
        return false;
    }

    qint32 nNumberOfRecords = 0;
    qint64 nEndOffset = 0;

    return _scanArchive(nOffset, getSize(), &nNumberOfRecords, &nEndOffset, pPdStruct);
}

bool XTAR::isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    XTAR xtar(pDevice);

    return xtar.isValid(pPdStruct);
}

QString XTAR::getFileFormatExt()
{
    return "tar";
}

QString XTAR::getFileFormatExtsString()
{
    return "TAR (*.tar)";
}

QString XTAR::getMIMEString()
{
    return "application/x-tar";
}

QString XTAR::structIDToString(quint32 nID)
{
    return XBinary::XCONVERT_idToTransString(nID, _TABLE_XTAR_STRUCTID, sizeof(_TABLE_XTAR_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QString XTAR::structIDToFtString(quint32 nID)
{
    return XBinary::XCONVERT_idToFtString(nID, _TABLE_XTAR_STRUCTID, sizeof(_TABLE_XTAR_STRUCTID) / sizeof(XBinary::XCONVERT));
}

quint32 XTAR::ftStringToStructID(const QString &sFtString)
{
    return XCONVERT_ftStringToId(sFtString, _TABLE_XTAR_STRUCTID, sizeof(_TABLE_XTAR_STRUCTID) / sizeof(XBinary::XCONVERT));
}

// qint32 XTAR::readTableRow(qint32 nRow, LT locType, XADDR nLocation, const DATA_RECORDS_OPTIONS &dataRecordsOptions, QList<DATA_RECORD_ROW> *pListDataRecords,
//                           void *pUserData, PDSTRUCT *pPdStruct)
// {
//     qint32 nResult = 0;

//     if (dataRecordsOptions.dataHeaderFirst.dsID.nID == STRUCTID_POSIX_HEADER) {
//         nResult = XBinary::readTableRow(nRow, locType, nLocation, dataRecordsOptions, pListDataRecords, pUserData, pPdStruct);

//         qint64 nStartOffset = locationToOffset(dataRecordsOptions.pMemoryMap, locType, nLocation);

//         XTAR::posix_header header = read_posix_header(nStartOffset);

//         nResult = 0x200 + align_up(_getSize(header), 0x200);
//     } else {
//         nResult = XBinary::readTableRow(nRow, locType, nLocation, dataRecordsOptions, pListDataRecords, pUserData, pPdStruct);
//     }

//     return nResult;
// }

qint32 XTAR::_getNumberOf_posix_headers(qint64 nOffset, PDSTRUCT *pPdStruct)
{
    qint32 nResult = 0;
    qint64 nEndOffset = 0;

    if (!_scanArchive(nOffset, getSize(), &nResult, &nEndOffset, pPdStruct)) {
        nResult = 0;
    }

    return nResult;
}

// QList<XBinary::DATA_HEADER> XTAR::getDataHeaders(const DATA_HEADERS_OPTIONS &dataHeadersOptions, PDSTRUCT *pPdStruct)
// {
//     QList<XBinary::DATA_HEADER> listResult;

//     if (dataHeadersOptions.nID == STRUCTID_UNKNOWN) {
//         DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
//         _dataHeadersOptions.bChildren = true;
//         _dataHeadersOptions.dsID_parent = _addDefaultHeaders(&listResult, pPdStruct);
//         _dataHeadersOptions.dhMode = XBinary::DHMODE_TABLE;
//         _dataHeadersOptions.fileType = dataHeadersOptions.pMemoryMap->fileType;
//         _dataHeadersOptions.nCount = _getNumberOf_posix_headers(0, pPdStruct);
//         _dataHeadersOptions.nLocation = 0;
//         _dataHeadersOptions.locType = XBinary::LT_OFFSET;
//         _dataHeadersOptions.nID = STRUCTID_POSIX_HEADER;

//         listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//     } else {
//         qint64 nStartOffset = locationToOffset(dataHeadersOptions.pMemoryMap, dataHeadersOptions.locType, dataHeadersOptions.nLocation);

//         if (nStartOffset != -1) {
//             if (dataHeadersOptions.nID == STRUCTID_POSIX_HEADER) {
//                 XBinary::DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, XTAR::structIDToString(dataHeadersOptions.nID));

//                 dataHeader.listRecords.append(getDataRecord(0, 100, "Name", VT_CHAR_ARRAY, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(100, 8, "Mode", VT_CHAR_ARRAY, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(108, 8, "UID", VT_CHAR_ARRAY, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(116, 8, "GID", VT_CHAR_ARRAY, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(124, 12, "Size", VT_CHAR_ARRAY, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(136, 12, "MTime", VT_CHAR_ARRAY, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(148, 8, "Checksum", VT_CHAR_ARRAY, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(156, 1, "Typeflag", VT_CHAR_ARRAY, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(157, 100, "Linkname", VT_CHAR_ARRAY, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(257, 6, "Magic", VT_CHAR_ARRAY, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(263, 2, "Version", VT_CHAR_ARRAY, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(265, 32, "Uname", VT_CHAR_ARRAY, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(297, 32, "Gname", VT_CHAR_ARRAY, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(329, 8, "Devmajor", VT_CHAR_ARRAY, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(337, 8, "Devminor", VT_CHAR_ARRAY, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(345, 155, "Prefix", VT_CHAR_ARRAY, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.nSize = 500;  // TODO const

//                 listResult.append(dataHeader);
//             }
//         }
//     }

//     return listResult;
// }

QList<XBinary::XFHEADER> XTAR::getXFHeaders(const XFSTRUCT &xfStruct, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    QList<XBinary::XFHEADER> listResult;
    quint32 nStructID = xfStruct.nStructID;

    if (nStructID == STRUCTID_UNKNOWN) {
        XFSTRUCT _xfStruct = xfStruct;
        _xfStruct.nStructID = STRUCTID_POSIX_HEADER;
        _xfStruct.xLoc = offsetToLoc(0);
        listResult.append(getXFHeaders(_xfStruct, pPdStruct));
    } else if (nStructID == STRUCTID_POSIX_HEADER) {
        XLOC headerLoc = xfStruct.xLoc;
        if (headerLoc.locType == LT_UNKNOWN) {
            headerLoc = offsetToLoc(0);
        }

        qint64 nHeaderOffset = locToOffset(xfStruct.pMemoryMap, headerLoc);

        if ((nHeaderOffset != -1) && isOffsetAndSizeValid(xfStruct.pMemoryMap, nHeaderOffset, sizeof(posix_header))) {
            XFHEADER xfHeader = {};
            xfHeader.sParentTag = xfStruct.sParent;
            xfHeader.fileType = xfStruct.fileType;
            xfHeader.structID = static_cast<XBinary::STRUCTID>(STRUCTID_POSIX_HEADER);
            xfHeader.xLoc = headerLoc;
            xfHeader.nSize = sizeof(posix_header);
            xfHeader.xfType = XFTYPE_HEADER;
            xfHeader.listFields = getXFRecords(xfStruct.fileType, STRUCTID_POSIX_HEADER, headerLoc);
            xfHeader.sTag = xfHeaderToTag(xfHeader, structIDToString(STRUCTID_POSIX_HEADER), xfHeader.sParentTag);
            listResult.append(xfHeader);
        }
    }

    return listResult;
}

QList<XBinary::XFRECORD> XTAR::getXFRecords(FT fileType, quint32 nStructID, const XLOC &xLoc)
{
    Q_UNUSED(fileType)
    Q_UNUSED(xLoc)

    QList<XBinary::XFRECORD> listResult;

    if (nStructID == STRUCTID_POSIX_HEADER) {
        listResult.append({"Name", (qint32)offsetof(posix_header, name), (qint32)sizeof(((posix_header *)0)->name), XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        listResult.append({"Mode", (qint32)offsetof(posix_header, mode), (qint32)sizeof(((posix_header *)0)->mode), XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        listResult.append({"UID", (qint32)offsetof(posix_header, uid), (qint32)sizeof(((posix_header *)0)->uid), XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        listResult.append({"GID", (qint32)offsetof(posix_header, gid), (qint32)sizeof(((posix_header *)0)->gid), XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        listResult.append({"Size", (qint32)offsetof(posix_header, size), (qint32)sizeof(((posix_header *)0)->size), XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        listResult.append({"MTime", (qint32)offsetof(posix_header, mtime), (qint32)sizeof(((posix_header *)0)->mtime), XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        listResult.append({"Checksum", (qint32)offsetof(posix_header, chksum), (qint32)sizeof(((posix_header *)0)->chksum), XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        listResult.append({"Typeflag", (qint32)offsetof(posix_header, typeflag), (qint32)sizeof(((posix_header *)0)->typeflag), XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        listResult.append({"Linkname", (qint32)offsetof(posix_header, linkname), (qint32)sizeof(((posix_header *)0)->linkname), XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        listResult.append({"Magic", (qint32)offsetof(posix_header, magic), (qint32)sizeof(((posix_header *)0)->magic), XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        listResult.append({"Version", (qint32)offsetof(posix_header, version), (qint32)sizeof(((posix_header *)0)->version), XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        listResult.append({"Uname", (qint32)offsetof(posix_header, uname), (qint32)sizeof(((posix_header *)0)->uname), XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        listResult.append({"Gname", (qint32)offsetof(posix_header, gname), (qint32)sizeof(((posix_header *)0)->gname), XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        listResult.append({"Devmajor", (qint32)offsetof(posix_header, devmajor), (qint32)sizeof(((posix_header *)0)->devmajor), XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        listResult.append({"Devminor", (qint32)offsetof(posix_header, devminor), (qint32)sizeof(((posix_header *)0)->devminor), XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        listResult.append({"Prefix", (qint32)offsetof(posix_header, prefix), (qint32)sizeof(((posix_header *)0)->prefix), XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
    }

    return listResult;
}

static bool tarCanAppend(qint32 nLimit, const QList<XBinary::FPART> *pListResult)
{
    return (nLimit == -1) || (pListResult->size() < nLimit);
}

QList<XBinary::FPART> XTAR::getFileParts(quint32 nFileParts, qint32 nLimit, PDSTRUCT *pPdStruct)
{
    QList<XBinary::FPART> listResult;

    if ((nLimit < -1) || (nLimit == 0)) {
        return listResult;
    }

    const qint64 nTotalSize = getSize();
    QList<TAR_RECORD> listRecords;
    qint64 nArchiveEnd = 0;

    if (!_collectRecords(0, nTotalSize, &listRecords, &nArchiveEnd, pPdStruct)) {
        return listResult;
    }

    for (const TAR_RECORD &tarRecord : listRecords) {
        if (!tarCanAppend(nLimit, &listResult) || !isPdStructNotCanceled(pPdStruct)) {
            break;
        }
        if ((nFileParts & FILEPART_HEADER) && tarCanAppend(nLimit, &listResult)) {
            XBinary::FPART record = {};
            record.filePart = FILEPART_HEADER;
            record.nFileOffset = tarRecord.nHeaderOffset;
            record.nFileSize = 0x200;  // TODO const
            record.nVirtualAddress = XADDR_MAX;
            record.sName = tr("Header");
            listResult.append(record);
        }

        if ((nFileParts & FILEPART_STREAM) && tarCanAppend(nLimit, &listResult)) {
            const qint64 nAlignedSize = tarRecord.nRecordSize - 512;

            XBinary::FPART record = {};
            record.filePart = FILEPART_STREAM;
            record.nFileOffset = tarRecord.nDataOffset;
            record.nFileSize = nAlignedSize;  // Padded size in archive
            record.nVirtualAddress = XADDR_MAX;
            record.sName = tarRecord.sPath;
            record.mapProperties.insert(XBinary::FPART_PROP_HANDLEMETHOD, XBinary::HANDLE_METHOD_STORE);
            record.mapProperties.insert(XBinary::FPART_PROP_COMPRESSEDSIZE, nAlignedSize);
            record.mapProperties.insert(XBinary::FPART_PROP_UNCOMPRESSEDSIZE, tarRecord.nFileSize);  // Actual file size
            record.mapProperties.insert(XBinary::FPART_PROP_ORIGINALNAME, tarRecord.sPath);
            if (tarRecord.header.typeflag[0] == '5') record.mapProperties.insert(XBinary::FPART_PROP_ISFOLDER, true);
            // TODO Checksum
            listResult.append(record);
        }
    }

    if (!isPdStructNotCanceled(pPdStruct)) {
        return QList<XBinary::FPART>();
    }

    if ((nFileParts & FILEPART_DATA) && tarCanAppend(nLimit, &listResult)) {
        XBinary::FPART record = {};
        record.filePart = FILEPART_DATA;
        record.nFileOffset = 0;
        record.nFileSize = nArchiveEnd;
        record.nVirtualAddress = XADDR_MAX;
        record.sName = tr("Data");
        listResult.append(record);
    }

    if ((nFileParts & FILEPART_OVERLAY) && tarCanAppend(nLimit, &listResult) && (nArchiveEnd < nTotalSize)) {
        XBinary::FPART record = {};
        record.filePart = FILEPART_OVERLAY;
        record.nFileOffset = nArchiveEnd;
        record.nFileSize = nTotalSize - nArchiveEnd;
        record.nVirtualAddress = XADDR_MAX;
        record.sName = tr("Overlay");
        listResult.append(record);
    }

    return listResult;
}

XBinary::_MEMORY_MAP XTAR::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    XBinary::_MEMORY_MAP result = {};

    if (mapMode == MAPMODE_UNKNOWN) {
        mapMode = MAPMODE_DATA;  // Default mode
    }

    if (mapMode == MAPMODE_REGIONS) {
        result = _getMemoryMap(FILEPART_HEADER | FILEPART_STREAM | FILEPART_OVERLAY, pPdStruct);
    } else if (mapMode == MAPMODE_STREAMS) {
        result = _getMemoryMap(FILEPART_STREAM, pPdStruct);
    } else if (mapMode == MAPMODE_DATA) {
        result = _getMemoryMap(FILEPART_DATA | FILEPART_OVERLAY, pPdStruct);
    }

    return result;
}

QList<XBinary::MAPMODE> XTAR::getMapModesList()
{
    QList<MAPMODE> listResult;

    listResult.append(MAPMODE_REGIONS);
    listResult.append(MAPMODE_STREAMS);
    listResult.append(MAPMODE_DATA);

    return listResult;
}

XBinary::FT XTAR::getFileType()
{
    return FT_TAR;
}

XTAR::posix_header XTAR::read_posix_header(qint64 nOffset)
{
    posix_header record = {};

    read_array(nOffset, (char *)&record, sizeof(record));

    return record;
}

qint64 XTAR::_getSize(const posix_header &header)
{
    qint64 nResult = -1;
    _parseNumber(header.size, sizeof(header.size), &nResult);

    return nResult;
}

bool XTAR::_parseNumber(const char *pData, qint32 nSize, qint64 *pValue)
{
    if (!pData || !pValue || (nSize <= 0)) {
        return false;
    }

    const quint8 *pBytes = reinterpret_cast<const quint8 *>(pData);

    // GNU/POSIX base-256 extension.  File sizes are non-negative; reject the
    // signed form and values that cannot be represented by the public qint64
    // archive API.
    if (pBytes[0] & 0x80) {
        if (pBytes[0] & 0x40) {
            return false;
        }

        quint64 nValue = pBytes[0] & 0x7f;
        const quint64 nMaximum = static_cast<quint64>((std::numeric_limits<qint64>::max)());

        for (qint32 i = 1; i < nSize; i++) {
            if (nValue > ((nMaximum - pBytes[i]) / 256)) {
                return false;
            }

            nValue = (nValue * 256) + pBytes[i];
        }

        *pValue = static_cast<qint64>(nValue);
        return true;
    }

    qint32 nIndex = 0;
    while ((nIndex < nSize) && ((pBytes[nIndex] == 0) || (pBytes[nIndex] == ' '))) {
        nIndex++;
    }

    bool bHasDigit = false;
    quint64 nValue = 0;
    const quint64 nMaximum = static_cast<quint64>((std::numeric_limits<qint64>::max)());

    for (; nIndex < nSize; nIndex++) {
        const quint8 nByte = pBytes[nIndex];

        if ((nByte == 0) || (nByte == ' ')) {
            for (; nIndex < nSize; nIndex++) {
                if ((pBytes[nIndex] != 0) && (pBytes[nIndex] != ' ')) {
                    return false;
                }
            }
            break;
        }

        if ((nByte < '0') || (nByte > '7')) {
            return false;
        }

        const quint64 nDigit = nByte - '0';
        if (nValue > ((nMaximum - nDigit) / 8)) {
            return false;
        }

        nValue = (nValue * 8) + nDigit;
        bHasDigit = true;
    }

    if (!bHasDigit) {
        return false;
    }

    *pValue = static_cast<qint64>(nValue);
    return true;
}

bool XTAR::_readHeader(qint64 nOffset, qint64 nTotalSize, posix_header *pHeader, bool *pIsZeroBlock, PDSTRUCT *pPdStruct)
{
    QPointer<XTAR> guardedArchive(this);
    if (!pHeader || !pIsZeroBlock || (nOffset < 0) || (nTotalSize < 0) || (nOffset > nTotalSize) || ((nTotalSize - nOffset) < 512)) {
        return false;
    }

    QByteArray baHeader(512, 0);
    // A random-access QIODevice may legally satisfy a read in multiple short
    // chunks.  Drain the complete header through the guarded helper so TAR
    // parsing observes the same short-I/O contract as retained-source
    // fingerprinting and honors cancellation while doing so.
    const qint64 nRead = guardedArchive->read_array_process(nOffset, baHeader.data(), baHeader.size(), pPdStruct);
    if (!guardedArchive || (nRead != baHeader.size())) {
        return false;
    }

    bool bIsZeroBlock = true;
    for (qint32 i = 0; i < baHeader.size(); i++) {
        if (baHeader.at(i) != 0) {
            bIsZeroBlock = false;
            break;
        }
    }

    *pIsZeroBlock = bIsZeroBlock;
    memset(pHeader, 0, sizeof(posix_header));

    if (bIsZeroBlock) {
        return true;
    }

    memcpy(pHeader, baHeader.constData(), sizeof(posix_header));

    if ((pHeader->name[0] == 0) || (memcmp(pHeader->magic, "ustar", 5) != 0)) {
        return false;
    }

    qint64 nStoredChecksum = 0;
    if (!_parseNumber(pHeader->chksum, sizeof(pHeader->chksum), &nStoredChecksum)) {
        return false;
    }

    quint64 nUnsignedChecksum = 0;
    qint64 nSignedChecksum = 0;
    for (qint32 i = 0; i < baHeader.size(); i++) {
        const quint8 nUnsignedByte = ((i >= 148) && (i < 156)) ? static_cast<quint8>(' ') : static_cast<quint8>(baHeader.at(i));
        const qint8 nSignedByte = ((i >= 148) && (i < 156)) ? static_cast<qint8>(' ') : static_cast<qint8>(baHeader.at(i));
        nUnsignedChecksum += nUnsignedByte;
        nSignedChecksum += nSignedByte;
    }

    if ((static_cast<quint64>(nStoredChecksum) != nUnsignedChecksum) && ((nSignedChecksum < 0) || (nStoredChecksum != nSignedChecksum))) {
        return false;
    }

    return true;
}

bool XTAR::_readRecord(qint64 nOffset, qint64 nTotalSize, posix_header *pHeader, qint64 *pFileSize, qint64 *pRecordSize, bool *pIsZeroBlock, PDSTRUCT *pPdStruct,
                       qint64 nSizeOverride)
{
    if (!pHeader || !pFileSize || !pRecordSize || !pIsZeroBlock || (nSizeOverride < -1)) {
        return false;
    }

    if (!_readHeader(nOffset, nTotalSize, pHeader, pIsZeroBlock, pPdStruct)) {
        return false;
    }

    *pFileSize = 0;
    *pRecordSize = 512;
    if (*pIsZeroBlock) return true;

    qint64 nFileSize = nSizeOverride;
    if ((nFileSize == -1) && !_parseNumber(pHeader->size, sizeof(pHeader->size), &nFileSize)) {
        return false;
    }

    qint64 nCurrentRecordSize = 0;
    if (!tarGetRecordSize(nOffset, nTotalSize, nFileSize, &nCurrentRecordSize)) {
        return false;
    }

    *pFileSize = nFileSize;
    *pRecordSize = nCurrentRecordSize;

    return true;
}

bool XTAR::_collectRecords(qint64 nOffset, qint64 nTotalSize, QList<TAR_RECORD> *pListRecords, qint64 *pEndOffset, PDSTRUCT *pPdStruct)
{
    QPointer<XTAR> guardedArchive(this);
    if (!pListRecords || !pEndOffset || (nOffset < 0) || (nTotalSize < 0) || (nOffset > nTotalSize)) {
        return false;
    }

    pListRecords->clear();
    *pEndOffset = nOffset;

    const qint32 N_MAX_RECORDS = 100000;
    qint32 nPhysicalRecords = 0;
    qint64 nCurrentOffset = nOffset;
    bool bTerminated = false;
    bool bPendingPerFileExtension = false;
    QMap<QByteArray, QByteArray> mapGlobalMetadata;
    QMap<QByteArray, QByteArray> mapLocalMetadata;

    while (nCurrentOffset < nTotalSize) {
        if (!isPdStructNotCanceled(pPdStruct)) return false;

        posix_header header = {};
        bool bIsZeroBlock = false;
        if (!guardedArchive->_readHeader(nCurrentOffset, nTotalSize, &header, &bIsZeroBlock, pPdStruct) || !guardedArchive) {
            return false;
        }

        if (bIsZeroBlock) {
            bTerminated = true;
            break;
        }
        if (nPhysicalRecords >= N_MAX_RECORDS) return false;

        const char cTypeFlag = header.typeflag[0];
        const bool bPaxMetadata = (cTypeFlag == 'x') || (cTypeFlag == 'g');
        const bool bGnuMetadata = (cTypeFlag == 'L') || (cTypeFlag == 'K');
        const bool bPerFileExtension = (cTypeFlag == 'x') || (cTypeFlag == 'L') || (cTypeFlag == 'K');
        qint64 nFileSize = 0;
        qint64 nRecordSize = 0;

        if (bPaxMetadata || bGnuMetadata) {
            if (!_parseNumber(header.size, sizeof(header.size), &nFileSize) || (nFileSize <= 0) || (nFileSize > TAR_MAX_METADATA_PAYLOAD) ||
                !tarGetRecordSize(nCurrentOffset, nTotalSize, nFileSize, &nRecordSize)) {
                return false;
            }

            QByteArray baPayload(static_cast<qint32>(nFileSize), 0);
            const qint64 nRead = guardedArchive->read_array_process(nCurrentOffset + 512, baPayload.data(), baPayload.size(), pPdStruct);
            if (!guardedArchive || (nRead != baPayload.size())) return false;

            if (bPaxMetadata) {
                QMap<QByteArray, QByteArray> mapUpdate;
                if (!tarParsePaxPayload(baPayload, &mapUpdate) || !tarValidateMetadataUpdate(mapUpdate)) {
                    return false;
                }
                if (cTypeFlag == 'g') {
                    tarMergeGlobalMetadata(&mapGlobalMetadata, mapUpdate);
                    if (!tarValidateMetadata(mapGlobalMetadata)) return false;
                } else {
                    for (QMap<QByteArray, QByteArray>::const_iterator it = mapUpdate.constBegin(); it != mapUpdate.constEnd(); ++it) {
                        mapLocalMetadata.insert(it.key(), it.value());
                    }
                }
            } else {
                QByteArray baValue;
                if (!tarParseGnuMetadata(baPayload, &baValue)) return false;
                mapLocalMetadata.insert(cTypeFlag == 'L' ? "path" : "linkpath", baValue);
            }
            if (bPerFileExtension) bPendingPerFileExtension = true;
        } else {
            const QMap<QByteArray, QByteArray> mapMetadata = tarEffectiveMetadata(mapGlobalMetadata, mapLocalMetadata);
            if (!tarValidateMetadata(mapMetadata)) return false;

            qint64 nSizeOverride = -1;
            if (mapMetadata.contains("size")) {
                quint64 nValue = 0;
                if (!tarParseUnsignedDecimal(mapMetadata.value("size"), static_cast<quint64>((std::numeric_limits<qint64>::max)()), &nValue)) {
                    return false;
                }
                nSizeOverride = static_cast<qint64>(nValue);
            }

            if (nSizeOverride == -1) {
                if (!_parseNumber(header.size, sizeof(header.size), &nFileSize)) {
                    return false;
                }
            } else {
                nFileSize = nSizeOverride;
            }
            if (!tarGetRecordSize(nCurrentOffset, nTotalSize, nFileSize, &nRecordSize)) {
                return false;
            }

            TAR_RECORD record = {};
            record.header = header;
            record.nHeaderOffset = nCurrentOffset;
            record.nDataOffset = nCurrentOffset + 512;
            record.nFileSize = nFileSize;
            record.nRecordSize = nRecordSize;
            record.mapMetadata = mapMetadata;
            record.sPath = _getRecordPath(header);
            if (mapMetadata.contains("path")) {
                record.sPath = QString::fromUtf8(mapMetadata.value("path"));
            }
            record.sLinkPath = tarBoundedString(header.linkname, static_cast<qint32>(sizeof(header.linkname)));
            record.bHasLinkPath = !record.sLinkPath.isEmpty();
            if (mapMetadata.contains("linkpath")) {
                record.sLinkPath = QString::fromUtf8(mapMetadata.value("linkpath"));
                record.bHasLinkPath = !mapMetadata.value("linkpath").isEmpty();
            }
            pListRecords->append(record);
            mapLocalMetadata.clear();
            bPendingPerFileExtension = false;
        }

        nCurrentOffset += nRecordSize;
        nPhysicalRecords++;
    }

    *pEndOffset = nCurrentOffset;

    // A per-file PAX/GNU extension must be followed by a real member.  Global
    // headers may legally appear without affecting a later member, but a TAR
    // still needs at least one visible member to be useful to this reader.
    return !pListRecords->isEmpty() && !bPendingPerFileExtension && mapLocalMetadata.isEmpty() && (bTerminated || (nCurrentOffset == nTotalSize));
}

bool XTAR::_scanArchive(qint64 nOffset, qint64 nTotalSize, qint32 *pNumberOfRecords, qint64 *pEndOffset, PDSTRUCT *pPdStruct)
{
    if (!pNumberOfRecords || !pEndOffset || (nOffset < 0) || (nTotalSize < 0) || (nOffset > nTotalSize)) {
        return false;
    }

    QList<TAR_RECORD> listRecords;
    if (!_collectRecords(nOffset, nTotalSize, &listRecords, pEndOffset, pPdStruct)) {
        *pNumberOfRecords = 0;
        return false;
    }

    *pNumberOfRecords = listRecords.size();
    return true;
}

bool XTAR::createHeader(const QString &sFileName, const QString &sBasePath, qint64 nFileSize, quint32 nMode, qint64 nMTime, posix_header *pHeader)
{
    if (!pHeader || (nFileSize < 0) || (nMTime < 0)) return false;

    posix_header &header = *pHeader;

    // Zero out the entire header
    memset(&header, 0, sizeof(posix_header));

    // Build full name
    QString sFullName = sBasePath;

    if (!sFullName.isEmpty() && !sFullName.endsWith("/")) {
        sFullName += "/";
    }

    sFullName += sFileName;
    sFullName = QDir::fromNativeSeparators(sFullName);

    // POSIX ustar can represent up to 155 prefix bytes plus '/' plus 100 name
    // bytes.  Split on an actual UTF-8 slash; never truncate a path or a
    // multibyte code point silently.
    QByteArray baName = sFullName.toUtf8();
    QByteArray baPrefix;
    if (baName.isEmpty()) return false;

    if (baName.size() > (qint32)sizeof(header.name)) {
        qint32 nSlash = baName.lastIndexOf('/');
        while (nSlash > 0) {
            const qint32 nPrefixSize = nSlash;
            const qint32 nSuffixSize = baName.size() - nSlash - 1;
            if ((nPrefixSize <= (qint32)sizeof(header.prefix)) && (nSuffixSize > 0) && (nSuffixSize <= (qint32)sizeof(header.name))) {
                baPrefix = baName.left(nPrefixSize);
                baName = baName.mid(nSlash + 1);
                break;
            }
            nSlash = baName.lastIndexOf('/', nSlash - 1);
        }

        if (baPrefix.isEmpty() || (baName.size() > (qint32)sizeof(header.name))) return false;
    }

    memcpy(header.name, baName.constData(), baName.size());
    if (!baPrefix.isEmpty()) memcpy(header.prefix, baPrefix.constData(), baPrefix.size());

    // Write fields in octal format
    if (!writeOctal(header.mode, sizeof(header.mode), nMode) || !writeOctal(header.uid, sizeof(header.uid), 0) || !writeOctal(header.gid, sizeof(header.gid), 0) ||
        !writeOctal(header.size, sizeof(header.size), nFileSize) || !writeOctal(header.mtime, sizeof(header.mtime), nMTime)) {
        return false;
    }

    // Checksum field filled with spaces initially
    memset(header.chksum, ' ', 8);

    // Type flag (regular file)
    header.typeflag[0] = '0';

    // Magic and version (ustar format)
    memcpy(header.magic, "ustar", 5);
    header.magic[5] = 0;
    memcpy(header.version, "00", 2);

    // Calculate and write checksum
    quint32 nChecksum = calculateChecksum(header);

    if (!writeOctal(header.chksum, sizeof(header.chksum) - 1, nChecksum)) return false;
    header.chksum[6] = 0;
    header.chksum[7] = ' ';

    return true;
}

quint32 XTAR::calculateChecksum(const posix_header &header)
{
    quint32 nChecksum = 0;
    const unsigned char *pData = (const unsigned char *)&header;

    for (qint32 i = 0; i < (qint32)sizeof(posix_header); i++) {
        nChecksum += pData[i];
    }

    return nChecksum;
}

bool XTAR::writeOctal(char *pDest, qint32 nSize, qint64 nValue)
{
    if (!pDest || (nSize < 2) || (nValue < 0)) return false;

    QString sOctal = QString::number(nValue, 8);
    QByteArray baOctal = sOctal.toUtf8();
    const qint32 nDigits = nSize - 1;
    if (baOctal.size() > nDigits) return false;

    memset(pDest, '0', nDigits);
    pDest[nDigits] = 0;
    memcpy(pDest + nDigits - baOctal.size(), baOctal.constData(), baOctal.size());
    return true;
}

QMap<XBinary::UNPACK_PROP, QVariant> XTAR::getDefaultUnpackProperties()
{
    QMap<XBinary::UNPACK_PROP, QVariant> result = XArchive::getDefaultUnpackProperties();

    return result;
}

bool XTAR::initUnpack(UNPACK_STATE *pState, const QMap<UNPACK_PROP, QVariant> &mapProperties, PDSTRUCT *pPdStruct)
{
    QPointer<XTAR> guardedArchive(this);
    if (!pState || m_bUnpackOperationInProgress || ((pState->pContext || !pState->baUnpackSourceToken.isEmpty()) && !guardedArchive->ownsUnpackSource(pState))) {
        return false;
    }
    if (!guardedArchive->finishUnpack(pState, nullptr) || !guardedArchive) return false;
    UNPACK_OPERATION_GUARD operationGuard(&m_bUnpackOperationInProgress);
    if (!operationGuard.isAcquired()) return false;

    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    pState->mapUnpackProperties = mapProperties;
    pState->mapArchiveProperties.clear();
    pState->nCurrentOffset = 0;
    pState->nTotalSize = guardedArchive->getSize();
    if (!guardedArchive) {
        *pState = UNPACK_STATE();
        return false;
    }
    pState->nCurrentIndex = 0;
    pState->nNumberOfRecords = 0;
    pState->pContext = nullptr;

    const bool bBound = guardedArchive->bindUnpackSource(pState, pPdStruct);
    if (!guardedArchive || !bBound) {
        *pState = UNPACK_STATE();
        return false;
    }

    QList<TAR_RECORD> listRecords;
    qint64 nArchiveEnd = 0;
    const bool bScanned = guardedArchive->_collectRecords(0, pState->nTotalSize, &listRecords, &nArchiveEnd, pPdStruct);
    if (!guardedArchive) {
        *pState = UNPACK_STATE();
        return false;
    }
    if (!bScanned) {
        pState->nTotalSize = 0;
        pState->nNumberOfRecords = 0;
        guardedArchive->releaseUnpackSource(pState);
        return false;
    }

    UNPACK_CONTEXT *pContext = new (std::nothrow) UNPACK_CONTEXT;
    if (!pContext) {
        guardedArchive->releaseUnpackSource(pState);
        *pState = UNPACK_STATE();
        return false;
    }
    pContext->listRecords = listRecords;
    pContext->nArchiveEnd = nArchiveEnd;
    pState->pContext = pContext;
    pState->nNumberOfRecords = pContext->listRecords.size();
    pState->nCurrentOffset = pContext->listRecords.constFirst().nHeaderOffset;

    const bool bFinalized = guardedArchive->validateAndFinalizeUnpackSource(pState, pContext, pPdStruct);
    if (!guardedArchive) {
        return false;
    }
    if (!bFinalized) {
        pState->pContext = nullptr;
        guardedArchive->releaseUnpackSource(pState);
        delete pContext;
        *pState = UNPACK_STATE();
        return false;
    }

    return true;
}

XBinary::ARCHIVERECORD XTAR::infoCurrent(UNPACK_STATE *pState, PDSTRUCT *pPdStruct)
{
    UNPACK_OPERATION_GUARD operationGuard(&m_bUnpackOperationInProgress, &m_bNestedUnpackInfoAuthorized);
    if (!operationGuard.isAllowed()) return XBinary::ARCHIVERECORD();
    QPointer<XTAR> guardedArchive(this);

    XBinary::ARCHIVERECORD result = {};

    if (pState && pState->pContext && isPdStructNotCanceled(pPdStruct) && guardedArchive->isUnpackSourceCurrent(pState, pPdStruct) && guardedArchive &&
        (pState->nCurrentIndex >= 0)) {
        UNPACK_CONTEXT *pContext = static_cast<UNPACK_CONTEXT *>(pState->pContext);
        if (pState->nCurrentIndex >= pContext->listRecords.size()) return result;

        const TAR_RECORD &tarRecord = pContext->listRecords.at(pState->nCurrentIndex);
        const posix_header &header = tarRecord.header;
        const QMap<QByteArray, QByteArray> &mapMetadata = tarRecord.mapMetadata;

        result.nStreamOffset = tarRecord.nDataOffset;
        result.nStreamSize = tarRecord.nFileSize;
        // result.nDecompressedOffset = 0;
        // result.nDecompressedSize = tarRecord.nFileSize;

        result.mapProperties.insert(XBinary::FPART_PROP_ORIGINALNAME, tarRecord.sPath);
        result.mapProperties.insert(XBinary::FPART_PROP_HANDLEMETHOD, XBinary::HANDLE_METHOD_STORE);

        // Parse mode (octal)
        QString sMode = QString(QByteArray(header.mode, 8)).trimmed();
        quint32 nMode = sMode.toUInt(nullptr, 8);
        result.mapProperties.insert(XBinary::FPART_PROP_FILEMODE, nMode);

        // POSIX PAX gives an extended uname/gname precedence over both the
        // numeric header field and the corresponding numeric extension.
        const bool bExtendedUname = mapMetadata.contains("uname") && !mapMetadata.value("uname").isEmpty();
        const bool bExtendedGname = mapMetadata.contains("gname") && !mapMetadata.value("gname").isEmpty();

        // Parse uid/gid (octal).  Empty extended values are tombstones and do
        // not fall back to the raw ustar fields.
        if (!bExtendedUname && mapMetadata.contains("uid") && !mapMetadata.value("uid").isEmpty()) {
            quint64 nUid = 0;
            tarParseUnsignedDecimal(mapMetadata.value("uid"), static_cast<quint64>((std::numeric_limits<qint64>::max)()), &nUid);
            result.mapProperties.insert(XBinary::FPART_PROP_UID, static_cast<qint64>(nUid));
        } else if (!bExtendedUname && !mapMetadata.contains("uid")) {
            QString sUid = QString(QByteArray(header.uid, 8)).trimmed();
            result.mapProperties.insert(XBinary::FPART_PROP_UID, sUid.toUInt(nullptr, 8));
        }

        if (!bExtendedGname && mapMetadata.contains("gid") && !mapMetadata.value("gid").isEmpty()) {
            quint64 nGid = 0;
            tarParseUnsignedDecimal(mapMetadata.value("gid"), static_cast<quint64>((std::numeric_limits<qint64>::max)()), &nGid);
            result.mapProperties.insert(XBinary::FPART_PROP_GID, static_cast<qint64>(nGid));
        } else if (!bExtendedGname && !mapMetadata.contains("gid")) {
            QString sGid = QString(QByteArray(header.gid, 8)).trimmed();
            result.mapProperties.insert(XBinary::FPART_PROP_GID, sGid.toUInt(nullptr, 8));
        }

        // Size already handled
        result.mapProperties.insert(XBinary::FPART_PROP_UNCOMPRESSEDSIZE, tarRecord.nFileSize);

        if (mapMetadata.contains("mtime") && !mapMetadata.value("mtime").isEmpty()) {
            qint64 nMTimeMilliseconds = 0;
            tarParsePaxTime(mapMetadata.value("mtime"), &nMTimeMilliseconds);
            const QDateTime dateTime = QDateTime::fromMSecsSinceEpoch(nMTimeMilliseconds);
            result.mapProperties.insert(XBinary::FPART_PROP_DATETIME, dateTime);
            result.mapProperties.insert(XBinary::FPART_PROP_MTIME, dateTime);
        } else if (!mapMetadata.contains("mtime")) {
            QString sMTime = QString(QByteArray(header.mtime, 12)).trimmed();
            const qint64 nMTimeMilliseconds = sMTime.toLongLong(nullptr, 8) * 1000;
            const QDateTime dateTime = QDateTime::fromMSecsSinceEpoch(nMTimeMilliseconds);
            result.mapProperties.insert(XBinary::FPART_PROP_DATETIME, dateTime);
            result.mapProperties.insert(XBinary::FPART_PROP_MTIME, dateTime);
        }
        if (mapMetadata.contains("atime") && !mapMetadata.value("atime").isEmpty()) {
            qint64 nATimeMilliseconds = 0;
            tarParsePaxTime(mapMetadata.value("atime"), &nATimeMilliseconds);
            result.mapProperties.insert(XBinary::FPART_PROP_ATIME, QDateTime::fromMSecsSinceEpoch(nATimeMilliseconds));
        }

        // Parse checksum (octal)
        QString sChecksum = QString(QByteArray(header.chksum, 8)).trimmed();
        quint32 nChecksum = sChecksum.toUInt(nullptr, 8);
        result.mapProperties.insert(XBinary::FPART_PROP_RESULTCRC, nChecksum);
        result.mapProperties.insert(XBinary::FPART_PROP_CRC_TYPE, XBinary::CRC_TYPE_UNKNOWN);

        // Type flag
        char cTypeFlag = header.typeflag[0];
        if (cTypeFlag == '5') result.mapProperties.insert(XBinary::FPART_PROP_ISFOLDER, true);
        QString sTypeFlag;
        switch (cTypeFlag) {
            case '0': sTypeFlag = "Regular file"; break;
            case '1': sTypeFlag = "Hard link"; break;
            case '2': sTypeFlag = "Symbolic link"; break;
            case '3': sTypeFlag = "Character device"; break;
            case '4': sTypeFlag = "Block device"; break;
            case '5': sTypeFlag = "Directory"; break;
            case '6': sTypeFlag = "FIFO"; break;
            case '7': sTypeFlag = "Contiguous file"; break;
            default: sTypeFlag = QString("Unknown (%1)").arg(cTypeFlag); break;
        }
        result.mapProperties.insert(XBinary::FPART_PROP_TYPE, sTypeFlag);

        if (tarRecord.bHasLinkPath) {
            result.mapProperties.insert(XBinary::FPART_PROP_LINKNAME, tarRecord.sLinkPath);
        }

        // Uname/Gname
        QString sUname =
            mapMetadata.contains("uname") ? QString::fromUtf8(mapMetadata.value("uname")) : tarBoundedString(header.uname, static_cast<qint32>(sizeof(header.uname)));
        if (!sUname.isEmpty()) {
            result.mapProperties.insert(XBinary::FPART_PROP_USERNAME, sUname);
        }

        QString sGname =
            mapMetadata.contains("gname") ? QString::fromUtf8(mapMetadata.value("gname")) : tarBoundedString(header.gname, static_cast<qint32>(sizeof(header.gname)));
        if (!sGname.isEmpty()) {
            result.mapProperties.insert(XBinary::FPART_PROP_GROUPNAME, sGname);
        }

        // Dev major/minor (for devices)
        if (cTypeFlag == '3' || cTypeFlag == '4') {
            QString sDevMajor = QString(QByteArray(header.devmajor, 8)).trimmed();
            quint32 nDevMajor = sDevMajor.toUInt(nullptr, 8);
            QString sDevMinor = QString(QByteArray(header.devminor, 8)).trimmed();
            quint32 nDevMinor = sDevMinor.toUInt(nullptr, 8);
            QString sDevVersion = QString("%1,%2").arg(nDevMajor).arg(nDevMinor);
            result.mapProperties.insert(XBinary::FPART_PROP_DEVVERSION, sDevVersion);
        }

        // Prefix (for long names)
        QString sPrefix = tarBoundedString(header.prefix, static_cast<qint32>(sizeof(header.prefix)));
        if (!sPrefix.isEmpty()) {
            result.mapProperties.insert(XBinary::FPART_PROP_PREFIX, sPrefix);
        }
    }

    return result;
}

bool XTAR::moveToNext(UNPACK_STATE *pState, PDSTRUCT *pPdStruct)
{
    UNPACK_OPERATION_GUARD operationGuard(&m_bUnpackOperationInProgress);
    if (!operationGuard.isAcquired()) return false;
    QPointer<XTAR> guardedArchive(this);

    bool bResult = false;

    if (pState && pState->pContext && isPdStructNotCanceled(pPdStruct) && guardedArchive->isUnpackSourceCurrent(pState, pPdStruct) && guardedArchive &&
        (pState->nCurrentIndex >= 0)) {
        UNPACK_CONTEXT *pContext = static_cast<UNPACK_CONTEXT *>(pState->pContext);
        if (pState->nCurrentIndex >= pContext->listRecords.size()) return false;

        pState->nCurrentIndex++;
        bResult = (pState->nCurrentIndex < pContext->listRecords.size());
        pState->nCurrentOffset = bResult ? pContext->listRecords.at(pState->nCurrentIndex).nHeaderOffset : pContext->nArchiveEnd;
    }

    return bResult;
}

bool XTAR::finishUnpack(UNPACK_STATE *pState, PDSTRUCT *pPdStruct)
{
    UNPACK_OPERATION_GUARD operationGuard(&m_bUnpackOperationInProgress);
    if (!operationGuard.isAcquired()) return false;

    Q_UNUSED(pPdStruct)

    if (!pState) {
        return false;
    }

    if ((pState->pContext || !pState->baUnpackSourceToken.isEmpty()) && !ownsUnpackSource(pState)) return false;

    UNPACK_CONTEXT *pContext = static_cast<UNPACK_CONTEXT *>(pState->pContext);
    releaseUnpackSource(pState);
    pState->pContext = nullptr;

    pState->mapUnpackProperties.clear();
    pState->mapArchiveProperties.clear();
    pState->nCurrentOffset = 0;
    pState->nTotalSize = 0;
    pState->nCurrentIndex = 0;
    pState->nNumberOfRecords = 0;

    delete pContext;

    return true;
}

QList<XBinary::FPART_PROP> XTAR::getAvailableFPARTProperties()
{
    QList<XBinary::FPART_PROP> listResult;

    listResult.append(FPART_PROP_ORIGINALNAME);
    listResult.append(FPART_PROP_UNCOMPRESSEDSIZE);
    listResult.append(FPART_PROP_STREAMOFFSET);
    listResult.append(FPART_PROP_STREAMSIZE);

    return listResult;
}

bool XTAR::initPack(PACK_STATE *pState, QIODevice *pDevice, const QMap<PACK_PROP, QVariant> &mapProperties, PDSTRUCT *pPdStruct)
{
    if (!pState || !pDevice || !pDevice->isWritable() || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    pState->pDevice = pDevice;
    pState->mapProperties = mapProperties;

    // Initialize state
    pState->nCurrentOffset = pDevice->pos();
    pState->nNumberOfRecords = 0;
    pState->pContext = nullptr;

    if (pState->nCurrentOffset < 0) return false;

    // TAR format has no signature or header - starts directly with first file header
    return true;
}

static bool tarFailWrite(XBinary::PACK_STATE *pState, qint64 nStartPosition)
{
    tarRollbackWrite(pState->pDevice, nStartPosition);
    return false;
}

bool XTAR::addFile(PACK_STATE *pState, const QString &sFileName, PDSTRUCT *pPdStruct)
{
    if (!pState || !pState->pDevice || !pState->pDevice->isWritable() || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    const qint64 nStartPosition = pState->pDevice->pos();
    if (nStartPosition < 0) return false;

    QFileInfo fileInfo(sFileName);

    if (!fileInfo.exists() || !fileInfo.isFile()) {
        return false;
    }

    // Get file metadata
    qint64 nFileSize = fileInfo.size();
    quint32 nMode = 0;
#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
    qint64 nMTime = fileInfo.lastModified().toSecsSinceEpoch();
#else
    qint64 nMTime = fileInfo.lastModified().toMSecsSinceEpoch() / 1000;
#endif

#ifdef Q_OS_WIN
    nMode = 00644;  // Octal: owner read/write, group/others read
#else
    QFile::Permissions permissions = fileInfo.permissions();

    if (permissions & QFile::ReadOwner) nMode |= 0400;
    if (permissions & QFile::WriteOwner) nMode |= 0200;
    if (permissions & QFile::ExeOwner) nMode |= 0100;
    if (permissions & QFile::ReadGroup) nMode |= 0040;
    if (permissions & QFile::WriteGroup) nMode |= 0020;
    if (permissions & QFile::ExeGroup) nMode |= 0010;
    if (permissions & QFile::ReadOther) nMode |= 0004;
    if (permissions & QFile::WriteOther) nMode |= 0002;
    if (permissions & QFile::ExeOther) nMode |= 0001;
#endif

    // Determine file path to store in archive based on PATH_MODE
    QString sStoredPath;
    PATH_MODE pathMode = (PATH_MODE)pState->mapProperties.value(PACK_PROP_PATHMODE, PATH_MODE_BASENAME).toInt();
    QString sBasePath = pState->mapProperties.value(PACK_PROP_BASEPATH).toString();

    switch (pathMode) {
        case XBinary::PATH_MODE_ABSOLUTE: sStoredPath = fileInfo.absoluteFilePath(); break;
        case XBinary::PATH_MODE_RELATIVE:
            if (!sBasePath.isEmpty()) {
                QDir baseDir(sBasePath);
                sStoredPath = baseDir.relativeFilePath(fileInfo.absoluteFilePath());
            } else {
                sStoredPath = fileInfo.fileName();
            }
            break;
        case XBinary::PATH_MODE_BASENAME:
        default: sStoredPath = fileInfo.fileName(); break;
    }

    // Create TAR header
    posix_header header = {};
    if (!createHeader(sStoredPath, "", nFileSize, nMode, nMTime, &header)) {
        return false;
    }

    // Write header (500 bytes)
    if (!tarWriteAll(pState->pDevice, reinterpret_cast<const char *>(&header), sizeof(posix_header), pPdStruct)) {
        return tarFailWrite(pState, nStartPosition);
    }

    // Pad header to 512 bytes
    qint64 nHeaderPadding = 512 - sizeof(posix_header);
    if (nHeaderPadding > 0) {
        QByteArray baHeaderPadding(nHeaderPadding, 0);
        if (!tarWriteAll(pState->pDevice, baHeaderPadding.constData(), nHeaderPadding, pPdStruct)) {
            return tarFailWrite(pState, nStartPosition);
        }
    }

    // Open and write file data
    QFile file(sFileName);

    if (!file.open(QIODevice::ReadOnly)) {
        return tarFailWrite(pState, nStartPosition);
    }

    qint64 nBytesWritten = 0;

    while (nBytesWritten < nFileSize && XBinary::isPdStructNotCanceled(pPdStruct)) {
        QByteArray baBuffer = file.read(qMin((qint64)0x10000, nFileSize - nBytesWritten));

        if (baBuffer.isEmpty()) {
            file.close();
            return tarFailWrite(pState, nStartPosition);
        }

        if (!tarWriteAll(pState->pDevice, baBuffer.constData(), baBuffer.size(), pPdStruct)) {
            file.close();
            return tarFailWrite(pState, nStartPosition);
        }

        nBytesWritten += baBuffer.size();
    }

    file.close();

    if ((nBytesWritten != nFileSize) || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return tarFailWrite(pState, nStartPosition);
    }

    // Pad file data to 512-byte boundary
    qint64 nPadding = (512 - (nFileSize % 512)) % 512;

    if (nPadding > 0) {
        QByteArray baPadding(nPadding, 0);

        if (!tarWriteAll(pState->pDevice, baPadding.constData(), nPadding, pPdStruct)) {
            return tarFailWrite(pState, nStartPosition);
        }
    }

    // Update state
    pState->nCurrentOffset += 512 + nBytesWritten + nPadding;
    pState->nNumberOfRecords++;

    return true;
}

static void tarRestoreBasePath(XBinary::PACK_STATE *pState, bool bRestoreBasePath, bool bHadBasePath, const QVariant &originalBasePath)
{
    if (!bRestoreBasePath) return;
    if (bHadBasePath) {
        pState->mapProperties.insert(XBinary::PACK_PROP_BASEPATH, originalBasePath);
    } else {
        pState->mapProperties.remove(XBinary::PACK_PROP_BASEPATH);
    }
}

bool XTAR::addFolder(PACK_STATE *pState, const QString &sDirectoryPath, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();
    if (!pPdStruct) pPdStruct = &pdStructEmpty;

    if (!pState || !pState->pDevice || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    // Check if directory exists
    if (!XBinary::isDirectoryExists(sDirectoryPath)) {
        return false;
    }

    // Set base path for relative path calculation if not already set
    const bool bHadBasePath = pState->mapProperties.contains(PACK_PROP_BASEPATH);
    const QVariant originalBasePath = pState->mapProperties.value(PACK_PROP_BASEPATH);
    bool bRestoreBasePath = false;

    PATH_MODE pathMode = (PATH_MODE)pState->mapProperties.value(PACK_PROP_PATHMODE, PATH_MODE_BASENAME).toInt();
    QString sBasePath = pState->mapProperties.value(PACK_PROP_BASEPATH).toString();

    if (pathMode == XBinary::PATH_MODE_RELATIVE && sBasePath.isEmpty()) {
        sBasePath = sDirectoryPath;
        pState->mapProperties.insert(PACK_PROP_BASEPATH, sBasePath);
        bRestoreBasePath = true;
    }

    // Enumerate all files in directory
    QList<QString> listFiles;
    XBinary::findFiles(sDirectoryPath, &listFiles, true, 0, pPdStruct);

    if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
        tarRestoreBasePath(pState, bRestoreBasePath, bHadBasePath, originalBasePath);
        return false;
    }

    qint32 nNumberOfFiles = listFiles.count();

    // Add each file
    for (qint32 i = 0; (i < nNumberOfFiles) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        QString sFilePath = listFiles.at(i);
        QFileInfo fileInfo(sFilePath);

        // Skip directories (TAR stores files only)
        if (fileInfo.isDir()) {
            continue;
        }

        // Add file to archive
        if (!addFile(pState, sFilePath, pPdStruct)) {
            tarRestoreBasePath(pState, bRestoreBasePath, bHadBasePath, originalBasePath);
            return false;
        }
    }

    // Restore original base path if we changed it
    tarRestoreBasePath(pState, bRestoreBasePath, bHadBasePath, originalBasePath);

    return XBinary::isPdStructNotCanceled(pPdStruct);
}

bool XTAR::finishPack(PACK_STATE *pState, PDSTRUCT *pPdStruct)
{
    if (!pState || !pState->pDevice || !pState->pDevice->isWritable() || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    const qint64 nStartPosition = pState->pDevice->pos();
    if (nStartPosition < 0) return false;

    // TAR archives end with two 512-byte blocks of zeros
    QByteArray baZeros(1024, 0);

    if (!tarWriteAll(pState->pDevice, baZeros.constData(), baZeros.size(), pPdStruct)) {
        tarRollbackWrite(pState->pDevice, nStartPosition);
        return false;
    }

    pState->nCurrentOffset += baZeros.size();
    return true;
}

QList<QString> XTAR::getSearchSignatures()
{
    QList<QString> listResult;

    listResult.append("00'ustar'");

    return listResult;
}

XBinary *XTAR::createInstance(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    Q_UNUSED(bIsImage)
    Q_UNUSED(nModuleAddress)

    return new XTAR(pDevice);
}

bool XTAR::handleInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XTAR> guardedThis(this);
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

void *XTAR::getInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XTAR> guardedThis(this);
    const bool bHandled = guardedThis->handleInternalInfo(pPdStruct);
    if (!guardedThis || !bHandled) return nullptr;

    return &guardedThis->m_internalInfo;
}

void XTAR::setInternalInfo(void *pInternalInfo)
{
    if (pInternalInfo) {
        m_internalInfo = *static_cast<INTERNAL_INFO *>(pInternalInfo);
        XArchive::setInternalInfo(static_cast<XArchive::INTERNAL_INFO *>(&m_internalInfo));
    } else {
        m_internalInfo = INTERNAL_INFO();
        XArchive::setInternalInfo(nullptr);
    }
}
