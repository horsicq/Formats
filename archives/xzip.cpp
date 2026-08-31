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
#include "xzip.h"
#include "xapk.h"
#include "xapks.h"
#include "xipa.h"
#include "xjar.h"
#include <algorithm>
#include <QSet>
#include <QTemporaryFile>
#include <QUuid>
#include <limits>
#include <new>
#include "Algos/xdeflatedecoder.h"
#include "Algos/ximplodedecoder.h"
#include "Algos/xlzmadecoder.h"
#include "Algos/xlzwdecoder.h"
#include "Algos/xbzip2decoder.h"
#include "Algos/xshrinkdecoder.h"
#include "Algos/xreducedecoder.h"
#include "Algos/xstoredecoder.h"
#include "Algos/xzipcryptodecoder.h"
#include "Algos/xaesdecoder.h"
#include "Algos/xppmddecoder.h"

XBinary::XCONVERT _TABLE_XZip_STRUCTID[] = {
    {XZip::STRUCTID_UNKNOWN, "Unknown", QObject::tr("Unknown")},
    {XZip::STRUCTID_LOCALFILEHEADER, "LocalFileHeader", QString("Local File Header")},
    {XZip::STRUCTID_CENTRALDIRECTORYFILEHEADER, "CentralDirectoryFileHeader", QString("Central Directory File Header")},
    {XZip::STRUCTID_ENDOFCENTRALDIRECTORYRECORD, "EndOfCentralDirectoryRecord", QString("End of Central Directory Record")},
};

XBinary::XIDSTRING _TABLE_XZip_CMETHOD[] = {
    {XZip::CMETHOD_STORE, "Store"},         {XZip::CMETHOD_SHRINK, "Shrink"},
    {XZip::CMETHOD_REDUCED_1, "Reduced1"},  {XZip::CMETHOD_REDUCED_2, "Reduced2"},
    {XZip::CMETHOD_REDUCED_3, "Reduced3"},  {XZip::CMETHOD_REDUCED_4, "Reduced4"},
    {XZip::CMETHOD_IMPLODED, "Imploded"},   {XZip::CMETHOD_DEFLATE, "Deflate"},
    {XZip::CMETHOD_DEFLATE64, "Deflate64"}, {XZip::CMETHOD_PKWARE_DCL_IMPLODING, "PKWareDCLImploding"},
    {XZip::CMETHOD_BZIP2, "BZip2"},         {XZip::CMETHOD_LZMA, "LZMA"},
    {XZip::CMETHOD_ZSTD, "Zstandard"},      {XZip::CMETHOD_XZ, "XZ"},
    {XZip::CMETHOD_JPEG, "JPEG"},           {XZip::CMETHOD_WAVPACK, "WavPack"},
    {XZip::CMETHOD_PPMD, "PPMd"},           {XZip::CMETHOD_AES, "AES"},
};

XBinary::XIDSTRING _TABLE_XZip_FLAGS[] = {
    {0x0001, "Encrypted"},        {0x0002, "CompressionOption1"}, {0x0004, "CompressionOption2"},
    {0x0008, "DataDescriptor"},   {0x0010, "EnhancedDeflation"},  {0x0020, "CompressedPatchedData"},
    {0x0040, "StrongEncryption"}, {0x0800, "LanguageEncoding"},   {0x2000, "MaskHeaderValues"},
};

XBinary::XIDSTRING _TABLE_XZip_OS[] = {
    {0, "MS-DOS"},    {1, "Amiga"},          {2, "OpenVMS"}, {3, "UNIX"},          {4, "VM/CMS"},  {5, "Atari ST"}, {6, "OS/2 HPFS"},
    {7, "Macintosh"}, {8, "Z-System"},       {9, "CP/M"},    {10, "Windows NTFS"}, {11, "MVS"},    {12, "VSE"},     {13, "Acorn Risc"},
    {14, "VFAT"},     {15, "alternate MVS"}, {16, "BeOS"},   {17, "Tandem"},       {18, "OS/400"}, {19, "OS X"},
};

XBinary::XIDSTRING _TABLE_XZip_HeaderSignatures[] = {
    {0x06054B50, "ECD"},
    {0x02014B50, "CFD"},
    {0x04034B50, "LFD"},
};

const QString XZip::PREFIX_SIGNATURE = "SIGNATURE";

static XBinary::PM_INFO createPMInfo(XBinary::HANDLE_METHOD hm0, XBinary::HANDLE_METHOD hm1 = XBinary::HANDLE_METHOD_UNKNOWN,
                                     XBinary::HANDLE_METHOD hm2 = XBinary::HANDLE_METHOD_UNKNOWN, XBinary::HANDLE_METHOD hm3 = XBinary::HANDLE_METHOD_UNKNOWN)
{
    XBinary::PM_INFO result = {};

    result.hm[0] = hm0;
    result.hm[1] = hm1;
    result.hm[2] = hm2;
    result.hm[3] = hm3;

    return result;
}

namespace {
const quint16 ZIP_FLAG_UTF8 = 0x0800;
const quint16 ZIP_EXTRA_UNICODE_PATH = 0x7075;

quint16 readZipLE16(const char *pData)
{
    return (quint16)(quint8)pData[0] | ((quint16)(quint8)pData[1] << 8);
}

quint32 readZipLE32(const char *pData)
{
    return (quint32)(quint8)pData[0] | ((quint32)(quint8)pData[1] << 8) | ((quint32)(quint8)pData[2] << 16) | ((quint32)(quint8)pData[3] << 24);
}

bool decodeZipUtf8Strict(const QByteArray &baData, QString *pResult)
{
    if (!pResult) {
        return false;
    }

    QString sResult;
    sResult.reserve(baData.size());

    const quint8 *pData = reinterpret_cast<const quint8 *>(baData.constData());
    qint32 nOffset = 0;

    while (nOffset < baData.size()) {
        const quint8 nFirst = pData[nOffset++];
        quint32 nCodePoint = 0;
        qint32 nContinuationCount = 0;
        quint32 nMinimum = 0;

        if (nFirst < 0x80) {
            nCodePoint = nFirst;
        } else if ((nFirst & 0xE0) == 0xC0) {
            nCodePoint = nFirst & 0x1F;
            nContinuationCount = 1;
            nMinimum = 0x80;
        } else if ((nFirst & 0xF0) == 0xE0) {
            nCodePoint = nFirst & 0x0F;
            nContinuationCount = 2;
            nMinimum = 0x800;
        } else if ((nFirst & 0xF8) == 0xF0) {
            nCodePoint = nFirst & 0x07;
            nContinuationCount = 3;
            nMinimum = 0x10000;
        } else {
            return false;
        }

        if (nContinuationCount > (baData.size() - nOffset)) {
            return false;
        }

        for (qint32 i = 0; i < nContinuationCount; i++) {
            const quint8 nNext = pData[nOffset++];
            if ((nNext & 0xC0) != 0x80) {
                return false;
            }
            nCodePoint = (nCodePoint << 6) | (nNext & 0x3F);
        }

        if ((nContinuationCount != 0) && (nCodePoint < nMinimum)) {
            return false;
        }
        if ((nCodePoint > 0x10FFFF) || ((nCodePoint >= 0xD800) && (nCodePoint <= 0xDFFF))) {
            return false;
        }

        if (nCodePoint <= 0xFFFF) {
            sResult.append(QChar((quint16)nCodePoint));
        } else {
            nCodePoint -= 0x10000;
            sResult.append(QChar((quint16)(0xD800 + (nCodePoint >> 10))));
            sResult.append(QChar((quint16)(0xDC00 + (nCodePoint & 0x3FF))));
        }
    }

    *pResult = sResult;
    return true;
}

QString decodeZipCP437(const QByteArray &baData)
{
    static const quint16 g_anCP437HighBytes[128] = {
        0x00C7, 0x00FC, 0x00E9, 0x00E2, 0x00E4, 0x00E0, 0x00E5, 0x00E7, 0x00EA, 0x00EB, 0x00E8, 0x00EF, 0x00EE, 0x00EC, 0x00C4, 0x00C5, 0x00C9, 0x00E6, 0x00C6,
        0x00F4, 0x00F6, 0x00F2, 0x00FB, 0x00F9, 0x00FF, 0x00D6, 0x00DC, 0x00A2, 0x00A3, 0x00A5, 0x20A7, 0x0192, 0x00E1, 0x00ED, 0x00F3, 0x00FA, 0x00F1, 0x00D1,
        0x00AA, 0x00BA, 0x00BF, 0x2310, 0x00AC, 0x00BD, 0x00BC, 0x00A1, 0x00AB, 0x00BB, 0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556, 0x2555,
        0x2563, 0x2551, 0x2557, 0x255D, 0x255C, 0x255B, 0x2510, 0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x255E, 0x255F, 0x255A, 0x2554, 0x2569, 0x2566,
        0x2560, 0x2550, 0x256C, 0x2567, 0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256B, 0x256A, 0x2518, 0x250C, 0x2588, 0x2584, 0x258C, 0x2590,
        0x2580, 0x03B1, 0x00DF, 0x0393, 0x03C0, 0x03A3, 0x03C3, 0x03BC, 0x03C4, 0x03A6, 0x0398, 0x03A9, 0x03B4, 0x221E, 0x03C6, 0x03B5, 0x2229, 0x2261, 0x00B1,
        0x2265, 0x2264, 0x2320, 0x2321, 0x00F7, 0x2248, 0x00B0, 0x2219, 0x00B7, 0x221A, 0x207F, 0x00B2, 0x25A0, 0x00A0,
    };

    QString sResult;
    sResult.reserve(baData.size());
    for (char cValue : baData) {
        const quint8 nValue = (quint8)cValue;
        sResult.append(QChar((nValue < 0x80) ? nValue : g_anCP437HighBytes[nValue - 0x80]));
    }
    return sResult;
}

bool decodeZipFileName(const QByteArray &baRawName, quint16 nFlags, const QByteArray &baExtraField, QString *pResult)
{
    if (!pResult) {
        return false;
    }

    QString sResult;
    if (nFlags & ZIP_FLAG_UTF8) {
        if (!decodeZipUtf8Strict(baRawName, &sResult)) {
            return false;
        }
    } else {
        sResult = decodeZipCP437(baRawName);
    }

    const quint32 nRawNameCRC = XBinary::_getCRC32(baRawName, 0xFFFFFFFF, XBinary::_getCRC32Table_EDB88320()) ^ 0xFFFFFFFF;

    qint32 nOffset = 0;
    while ((nOffset + 4) <= baExtraField.size()) {
        const quint16 nHeaderID = readZipLE16(baExtraField.constData() + nOffset);
        const quint16 nDataSize = readZipLE16(baExtraField.constData() + nOffset + 2);
        const qint32 nRecordSize = 4 + (qint32)nDataSize;
        if (nRecordSize > (baExtraField.size() - nOffset)) {
            break;
        }

        if ((nHeaderID == ZIP_EXTRA_UNICODE_PATH) && (nDataSize >= 5)) {
            const char *pData = baExtraField.constData() + nOffset + 4;
            if (((quint8)pData[0] == 1) && (readZipLE32(pData + 1) == nRawNameCRC)) {
                QString sUnicodeName;
                if (decodeZipUtf8Strict(QByteArray(pData + 5, nDataSize - 5), &sUnicodeName)) {
                    sResult = sUnicodeName;
                    break;
                }
            }
        }

        nOffset += nRecordSize;
    }

    *pResult = sResult;
    return true;
}

bool zipWriteAll(QIODevice *pDevice, const char *pData, qint64 nSize, XBinary::PDSTRUCT *pPdStruct, qint64 *pnWritten = nullptr)
{
    if (pnWritten) *pnWritten = 0;
    if (!pDevice || (nSize < 0) || ((nSize > 0) && !pData) || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    qint64 nWritten = 0;
    while ((nWritten < nSize) && XBinary::isPdStructNotCanceled(pPdStruct)) {
        const qint64 nResult = pDevice->write(pData + nWritten, nSize - nWritten);
        if ((nResult <= 0) || (nResult > (nSize - nWritten))) return false;
        nWritten += nResult;
        if (pnWritten) *pnWritten = nWritten;
    }

    return (nWritten == nSize) && XBinary::isPdStructNotCanceled(pPdStruct);
}

bool zipCopyExactly(QIODevice *pSource, QIODevice *pDest, qint64 nSize, XBinary::PDSTRUCT *pPdStruct, qint64 *pnWritten = nullptr)
{
    if (pnWritten) *pnWritten = 0;
    if (!pSource || !pDest || (nSize < 0) || !XBinary::isPdStructNotCanceled(pPdStruct)) return false;

    const qint32 nBufferSize = 0x10000;
    char *pBuffer = new (std::nothrow) char[nBufferSize];
    if (!pBuffer) return false;

    qint64 nRemaining = nSize;
    bool bResult = true;
    while ((nRemaining > 0) && XBinary::isPdStructNotCanceled(pPdStruct)) {
        const qint64 nChunkSize = qMin<qint64>(nRemaining, nBufferSize);
        const qint64 nRead = pSource->read(pBuffer, nChunkSize);
        qint64 nChunkWritten = 0;
        if ((nRead <= 0) || (nRead > nChunkSize) || !zipWriteAll(pDest, pBuffer, nRead, pPdStruct, &nChunkWritten)) {
            if (pnWritten) *pnWritten += nChunkWritten;
            bResult = false;
            break;
        }
        if (pnWritten) *pnWritten += nChunkWritten;
        nRemaining -= nRead;
    }

    delete[] pBuffer;
    return bResult && (nRemaining == 0) && XBinary::isPdStructNotCanceled(pPdStruct);
}

bool zipCanAppendAt(QIODevice *pDevice, qint64 nStartPosition)
{
    if (!pDevice || !pDevice->isWritable() || (nStartPosition < 0)) return false;
    if (pDevice->isSequential()) return true;

    return XBinary::isResizeEnable(pDevice) && (pDevice->pos() == nStartPosition) && (pDevice->size() == nStartPosition);
}

bool zipRollbackWrite(QIODevice *pDevice, qint64 nStartPosition)
{
    if (!pDevice || pDevice->isSequential() || (nStartPosition < 0) || !XBinary::isResizeEnable(pDevice)) {
        return false;
    }

    return XBinary::resize(pDevice, nStartPosition) && pDevice->seek(nStartPosition);
}

void zipDeletePackContext(XZip::ZIP_PACK_CONTEXT *pContext)
{
    if (!pContext) return;
    delete pContext->pListZipFileRecords;
    delete pContext;
}

bool zipIsPackStateConsistent(const XBinary::PACK_STATE *pState, const XZip::ZIP_PACK_CONTEXT *pContext)
{
    if (!pState || !pContext || (pState->pContext != pContext) || !pContext->pListZipFileRecords || !pContext->pDevice || (pState->pDevice != pContext->pDevice) ||
        !pState->pDevice->isWritable() || (pContext->nStartOffset < 0) || (pContext->nCurrentOffset < pContext->nStartOffset) || (pContext->nNumberOfRecords < 0) ||
        (pContext->nNumberOfRecords != pContext->pListZipFileRecords->size()) || (pState->nCurrentOffset != pContext->nCurrentOffset) ||
        (pState->nNumberOfRecords != pContext->nNumberOfRecords)) {
        return false;
    }

    if (pState->pDevice->isSequential() || pContext->bFailed) return true;
    return zipCanAppendAt(pState->pDevice, pContext->nCurrentOffset);
}

void zipMarkPackWriteFailure(XBinary::PACK_STATE *pState, XZip::ZIP_PACK_CONTEXT *pContext, qint64 nStartPosition, qint64 nWritten)
{
    if (!pState || !pContext || (nStartPosition < 0) || (nWritten < 0)) return;
    if (zipRollbackWrite(pState->pDevice, nStartPosition)) return;

    if ((nWritten > 0) || (pState->pDevice && !pState->pDevice->isSequential())) {
        pContext->bFailed = true;
        if (nWritten <= ((std::numeric_limits<qint64>::max)() - nStartPosition)) {
            pContext->nCurrentOffset = nStartPosition + nWritten;
            pState->nCurrentOffset = pContext->nCurrentOffset;
        }
    }
}

bool zipRestoreSourcePosition(QIODevice *pSource, bool bSequential, qint64 nOriginalPosition)
{
    return bSequential || pSource->seek(nOriginalPosition);
}

bool zipStageSource(QIODevice *pSource, QTemporaryFile *pStage, qint64 nExpectedSize, XBinary::PDSTRUCT *pPdStruct, qint64 *pnSize, quint32 *pnCRC32)
{
    if (pnSize) *pnSize = 0;
    if (pnCRC32) *pnCRC32 = 0;
    if (!pSource || !pStage || !pnSize || !pnCRC32 || !pSource->isReadable() || (nExpectedSize < -1) || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    const bool bSequential = pSource->isSequential();
    const qint64 nOriginalPosition = bSequential ? -1 : pSource->pos();
    if (!bSequential && ((nOriginalPosition < 0) || !pSource->seek(0))) return false;

    if (!pStage->open()) {
        zipRestoreSourcePosition(pSource, bSequential, nOriginalPosition);
        return false;
    }

    qint64 nSize = nExpectedSize;
    if (!bSequential) {
        const qint64 nDeviceSize = pSource->size();
        if ((nDeviceSize < 0) || ((nExpectedSize >= 0) && (nDeviceSize != nExpectedSize))) {
            zipRestoreSourcePosition(pSource, bSequential, nOriginalPosition);
            return false;
        }
        nSize = nDeviceSize;
    }
    if ((nSize >= 0) && ((quint64)nSize >= (std::numeric_limits<quint32>::max)())) {
        zipRestoreSourcePosition(pSource, bSequential, nOriginalPosition);
        return false;
    }

    const qint32 nBufferSize = 0x10000;
    char *pBuffer = new (std::nothrow) char[nBufferSize];
    if (!pBuffer) {
        zipRestoreSourcePosition(pSource, bSequential, nOriginalPosition);
        return false;
    }

    qint64 nTotal = 0;
    quint32 nCRC = 0xFFFFFFFF;
    bool bResult = true;
    while (XBinary::isPdStructNotCanceled(pPdStruct) && ((nSize < 0) || (nTotal < nSize))) {
        const qint64 nRequest = (nSize < 0) ? (qint64)nBufferSize : qMin<qint64>(nBufferSize, nSize - nTotal);
        const qint64 nRead = pSource->read(pBuffer, nRequest);
        if (nRead < 0 || nRead > nRequest) {
            bResult = false;
            break;
        }
        if (nRead == 0) {
            if ((nSize < 0) && pSource->atEnd()) break;
            bResult = false;
            break;
        }
        if ((quint64)nRead >= ((quint64)(std::numeric_limits<quint32>::max)() - (quint64)nTotal) || !zipWriteAll(pStage, pBuffer, nRead, pPdStruct)) {
            bResult = false;
            break;
        }
        nCRC = XBinary::_getCRC32(pBuffer, (qint32)nRead, nCRC, XBinary::_getCRC32Table_EDB88320());
        nTotal += nRead;
    }

    delete[] pBuffer;
    if ((nSize >= 0) && (nTotal != nSize)) bResult = false;
    if (!XBinary::isPdStructNotCanceled(pPdStruct)) bResult = false;
    if (!pStage->flush() || !pStage->seek(0)) bResult = false;
    if (!zipRestoreSourcePosition(pSource, bSequential, nOriginalPosition)) bResult = false;

    if (!bResult) return false;
    *pnSize = nTotal;
    *pnCRC32 = nCRC ^ 0xFFFFFFFF;
    return true;
}

bool zipGetCentralDirectorySizes(const QList<XZip::ZIPFILE_RECORD> *pRecords, const QByteArray &baComment, qint64 nStartPosition, qint64 *pCentralSize,
                                 qint64 *pTotalSize)
{
    if (!pRecords || !pCentralSize || !pTotalSize || (nStartPosition < 0) || ((quint64)nStartPosition >= (std::numeric_limits<quint32>::max)()) ||
        (pRecords->size() >= (std::numeric_limits<quint16>::max)()) || (baComment.size() > (std::numeric_limits<quint16>::max)())) {
        return false;
    }

    qint64 nCentralSize = 0;
    for (const XZip::ZIPFILE_RECORD &record : *pRecords) {
        const QByteArray baFileName = record.sFileName.toUtf8();
        if (baFileName.isEmpty() || (baFileName.size() > (std::numeric_limits<quint16>::max)()) || ((qint64)record.method < 0) ||
            ((qint64)record.method > (std::numeric_limits<quint16>::max)()) || (record.nCompressedSize < 0) ||
            ((quint64)record.nCompressedSize >= (std::numeric_limits<quint32>::max)()) || (record.nUncompressedSize < 0) ||
            ((quint64)record.nUncompressedSize >= (std::numeric_limits<quint32>::max)()) || (record.nHeaderOffset < 0) ||
            ((quint64)record.nHeaderOffset >= (std::numeric_limits<quint32>::max)())) {
            return false;
        }

        const qint64 nExpectedDataOffset = record.nHeaderOffset + (qint64)sizeof(XZip::LOCALFILEHEADER) + baFileName.size();
        if ((record.nHeaderOffset >= nStartPosition) || (record.nDataOffset != nExpectedDataOffset) || (record.nDataOffset > nStartPosition) ||
            (record.nCompressedSize > (nStartPosition - record.nDataOffset)) ||
            ((record.method == XZip::CMETHOD_STORE) && !(record.nFlags & 0x0001) && (record.nCompressedSize != record.nUncompressedSize))) {
            return false;
        }

        const qint64 nRecordSize = (qint64)sizeof(XZip::CENTRALDIRECTORYFILEHEADER) + baFileName.size();
        if (nCentralSize > (std::numeric_limits<quint32>::max)() - nRecordSize) return false;
        nCentralSize += nRecordSize;
    }

    if ((quint64)nCentralSize >= (std::numeric_limits<quint32>::max)()) return false;

    const qint64 nTrailerSize = (qint64)sizeof(XZip::ENDOFCENTRALDIRECTORYRECORD) + baComment.size();
    if (nCentralSize > (std::numeric_limits<qint64>::max)() - nTrailerSize) return false;

    *pCentralSize = nCentralSize;
    *pTotalSize = nCentralSize + nTrailerSize;
    return true;
}

bool zipBuildCentralDirectory(QIODevice *pDest, const QList<XZip::ZIPFILE_RECORD> *pRecords, const QByteArray &baComment, qint64 nStartPosition,
                              XBinary::PDSTRUCT *pPdStruct, qint64 *pTotalSize)
{
    qint64 nCentralSize = 0;
    qint64 nResultSize = 0;
    if (!pDest || !zipGetCentralDirectorySizes(pRecords, baComment, nStartPosition, &nCentralSize, &nResultSize) || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    for (const XZip::ZIPFILE_RECORD &record : *pRecords) {
        XZip::CENTRALDIRECTORYFILEHEADER header = {};
        header.nSignature = XZip::SIGNATURE_CFD;
        header.nVersion = record.nVersion;
        header.nOS = record.nOS;
        header.nMinVersion = record.nMinVersion;
        header.nMinOS = record.nMinOS;
        header.nFlags = record.nFlags | ZIP_FLAG_UTF8;
        header.nMethod = (quint16)record.method;
        const QPair<quint16, quint16> dosDateTime = XBinary::qDateTimeToDosDateTime(record.dtTime);
        header.nLastModDate = dosDateTime.first;
        header.nLastModTime = dosDateTime.second;
        header.nCRC32 = record.nCRC32;
        header.nCompressedSize = (quint32)record.nCompressedSize;
        header.nUncompressedSize = (quint32)record.nUncompressedSize;
        const QByteArray baFileName = record.sFileName.toUtf8();
        header.nFileNameLength = (quint16)baFileName.size();
        header.nExternalFileAttributes = record.nExternalFileAttributes;
        header.nOffsetToLocalFileHeader = (quint32)record.nHeaderOffset;

        if (!zipWriteAll(pDest, reinterpret_cast<const char *>(&header), sizeof(header), pPdStruct) ||
            !zipWriteAll(pDest, baFileName.constData(), baFileName.size(), pPdStruct)) {
            return false;
        }
    }

    XZip::ENDOFCENTRALDIRECTORYRECORD endRecord = {};
    endRecord.nSignature = XZip::SIGNATURE_ECD;
    endRecord.nDiskNumberOfRecords = (quint16)pRecords->size();
    endRecord.nTotalNumberOfRecords = (quint16)pRecords->size();
    endRecord.nSizeOfCentralDirectory = (quint32)nCentralSize;
    endRecord.nOffsetToCentralDirectory = (quint32)nStartPosition;
    endRecord.nCommentLength = (quint16)baComment.size();

    if (!zipWriteAll(pDest, reinterpret_cast<const char *>(&endRecord), sizeof(endRecord), pPdStruct) ||
        !zipWriteAll(pDest, baComment.constData(), baComment.size(), pPdStruct)) {
        return false;
    }

    if (pTotalSize) *pTotalSize = nResultSize;
    return XBinary::isPdStructNotCanceled(pPdStruct);
}

bool zipAppendCentralDirectory(QIODevice *pDest, QList<XZip::ZIPFILE_RECORD> *pRecords, const QByteArray &baComment, qint64 nStartPosition, XBinary::PDSTRUCT *pPdStruct,
                               qint64 *pnWritten, qint64 *pnTotalSize = nullptr)
{
    if (pnWritten) *pnWritten = 0;
    if (pnTotalSize) *pnTotalSize = 0;
    if (!pDest || !pRecords || !pDest->isWritable() || !zipCanAppendAt(pDest, nStartPosition) || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    QTemporaryFile centralDirectory;
    qint64 nTotalSize = 0;
    if (!centralDirectory.open() || !zipBuildCentralDirectory(&centralDirectory, pRecords, baComment, nStartPosition, pPdStruct, &nTotalSize) ||
        !centralDirectory.flush() || (centralDirectory.size() != nTotalSize) || !centralDirectory.seek(0) || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    if (!zipCopyExactly(&centralDirectory, pDest, nTotalSize, pPdStruct, pnWritten)) return false;
    if (pnTotalSize) *pnTotalSize = nTotalSize;
    return true;
}
}  // namespace

XZip::XZip(QIODevice *pDevice) : XArchive(pDevice)
{
}

bool XZip::isValid(PDSTRUCT *pPdStruct)
{
    QPointer<XZip> guardedArchive(this);
    QPointer<QIODevice> guardedSource(getDevice());
    if (!XBinary::isPdStructNotCanceled(pPdStruct)) return false;
    if (!guardedSource) return false;
    const qint64 nSize = guardedSource->size();
    if (!guardedArchive || !guardedSource) return false;

    const qint64 nECDOffset = guardedArchive->findECDOffset(pPdStruct);
    if (!guardedArchive || !guardedSource) return false;
    if (nECDOffset != -1) return true;

    if (nSize < (qint64)sizeof(quint32)) return false;
    const QByteArray baSignature = XBinary::read_array_process(guardedSource.data(), 0, sizeof(quint32), pPdStruct);
    if (!guardedArchive || !guardedSource || (baSignature.size() != (qint64)sizeof(quint32)) ||
        (XBinary::_read_uint32(const_cast<char *>(baSignature.constData())) != SIGNATURE_LFD)) {
        return false;
    }

    // Preserve support for deliberately central-directory-less streams, but
    // require at least one complete local record instead of accepting a bare
    // four-byte PK signature.
    qint64 nRealSize = 0;
    const qint32 nNumberOfRecords = guardedArchive->_getNumberOfLocalFileHeaders(0, nSize, &nRealSize, pPdStruct);
    if (!guardedArchive || !guardedSource) return false;
    if ((nNumberOfRecords <= 0) || (nRealSize <= 0)) return false;
    const bool bECDPresent = guardedArchive->_isECDSignaturePresent(nRealSize, pPdStruct);
    return guardedArchive && guardedSource && !bECDPresent;
}

bool XZip::isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    XZip xzip(pDevice);

    return xzip.isValid(pPdStruct);
}

QString XZip::getVersion()
{
    QString sResult;

    qint64 nECDOffset = findECDOffset(nullptr);

    quint16 nVersion = 0;

    if (nECDOffset != -1) {
        qint64 nOffset = read_uint32(nECDOffset + offsetof(ENDOFCENTRALDIRECTORYRECORD, nOffsetToCentralDirectory));

        quint32 nSignature = read_uint32(nOffset + offsetof(CENTRALDIRECTORYFILEHEADER, nSignature));

        if (nSignature == SIGNATURE_CFD) {
            nVersion = read_uint8(nOffset + offsetof(CENTRALDIRECTORYFILEHEADER, nVersion));

            if (nVersion == 0) {
                nVersion = read_uint8(nOffset + offsetof(CENTRALDIRECTORYFILEHEADER, nMinVersion));
            }
        }
    }

    if (nVersion == 0) {
        // The first record
        nVersion = read_uint8(0 + offsetof(LOCALFILEHEADER, nMinVersion));
    }

    if (nVersion) {
        sResult = QString("%1").arg((double)nVersion / 10, 0, 'f', 1);
    }

    return sResult;
}

bool XZip::isEncrypted()
{
    qint64 nTotalSize = getSize();
    qint64 nECDOffset = findECDOffset(nullptr);

    if ((nECDOffset >= 0) && ((nECDOffset + (qint64)sizeof(ENDOFCENTRALDIRECTORYRECORD)) <= nTotalSize)) {
        qint64 nOffset = read_uint32(nECDOffset + offsetof(ENDOFCENTRALDIRECTORYRECORD, nOffsetToCentralDirectory));
        quint16 nNumberOfRecords = read_uint16(nECDOffset + offsetof(ENDOFCENTRALDIRECTORYRECORD, nTotalNumberOfRecords));
        bool bCentralDirectoryFound = false;

        for (quint16 i = 0; i < nNumberOfRecords; i++) {
            if ((nOffset < 0) || ((nOffset + (qint64)sizeof(CENTRALDIRECTORYFILEHEADER)) > nTotalSize) || (read_uint32(nOffset) != SIGNATURE_CFD)) {
                break;
            }

            CENTRALDIRECTORYFILEHEADER cdfh = read_CENTRALDIRECTORYFILEHEADER(nOffset, nullptr);
            bCentralDirectoryFound = true;

            if ((cdfh.nFlags & 0x0001) || (cdfh.nMethod == CMETHOD_AES)) {
                return true;
            }

            qint64 nRecordSize = sizeof(CENTRALDIRECTORYFILEHEADER) + (qint64)cdfh.nFileNameLength + (qint64)cdfh.nExtraFieldLength + (qint64)cdfh.nFileCommentLength;

            if ((nRecordSize <= 0) || (nRecordSize > (nTotalSize - nOffset))) {
                break;
            }

            nOffset += nRecordSize;
        }

        if (bCentralDirectoryFound || (nNumberOfRecords == 0)) {
            return false;
        }
    }

    qint64 nOffset = 0;

    while ((nOffset >= 0) && ((nOffset + (qint64)sizeof(LOCALFILEHEADER)) <= nTotalSize) && (read_uint32(nOffset) == SIGNATURE_LFD)) {
        LOCALFILEHEADER lfh = read_LOCALFILEHEADER(nOffset, nullptr);

        if ((lfh.nFlags & 0x0001) || (lfh.nMethod == CMETHOD_AES)) {
            return true;
        }

        qint64 nRecordSize = sizeof(LOCALFILEHEADER) + (qint64)lfh.nFileNameLength + (qint64)lfh.nExtraFieldLength + (qint64)lfh.nCompressedSize;

        if ((nRecordSize <= 0) || (nRecordSize > (nTotalSize - nOffset))) {
            break;
        }

        nOffset += nRecordSize;
    }

    return false;
}

bool XZip::isCommentPresent()
{
    bool bResult = false;

    qint64 nECDOffset = findECDOffset(nullptr);

    if (nECDOffset != -1) {
        quint16 nCommentLength = read_uint16(nECDOffset + offsetof(ENDOFCENTRALDIRECTORYRECORD, nCommentLength));
        bResult = (nCommentLength > 0);
    }

    return bResult;
}

QString XZip::getComment()
{
    QString sResult;

    qint64 nECDOffset = findECDOffset(nullptr);

    if (nECDOffset != -1) {
        quint16 nCommentLength = read_uint16(nECDOffset + offsetof(ENDOFCENTRALDIRECTORYRECORD, nCommentLength));

        if (nCommentLength > 0) {
            qint64 nCommentOffset = nECDOffset + sizeof(ENDOFCENTRALDIRECTORYRECORD);
            QByteArray baComment = read_array(nCommentOffset, nCommentLength);
            sResult = QString::fromUtf8(baComment);
        }
    }

    return sResult;
}

QString XZip::getCompressMethodString()
{
    QString sResult;

    // Use an ordered container(not QSet), the hash seed is randomized per process,
    // so a QSet would render the methods in a different order on every run.
    QList<HANDLE_METHOD> listMethods;

    qint64 nECDOffset = findECDOffset(nullptr);

    if (nECDOffset != -1) {
        qint64 nOffset = read_uint32(nECDOffset + offsetof(ENDOFCENTRALDIRECTORYRECORD, nOffsetToCentralDirectory));

        for (int i = 0; i < 20; i++) {
            quint32 nSignature = read_uint32(nOffset + offsetof(CENTRALDIRECTORYFILEHEADER, nSignature));

            if (nSignature == SIGNATURE_CFD) {
                quint16 nMethod = read_uint16(nOffset + offsetof(CENTRALDIRECTORYFILEHEADER, nMethod));
                quint16 nFlags = read_uint16(nOffset + offsetof(CENTRALDIRECTORYFILEHEADER, nFlags));
                quint32 nUncompressedSize = read_uint32(nOffset + offsetof(CENTRALDIRECTORYFILEHEADER, nUncompressedSize));

                if (nUncompressedSize > 0) {
                    HANDLE_METHOD method = zipToCompressMethod(nMethod, nFlags);

                    if (!listMethods.contains(method)) {
                        listMethods.append(method);
                    }
                }

                nOffset += (sizeof(CENTRALDIRECTORYFILEHEADER) + read_uint16(nOffset + offsetof(CENTRALDIRECTORYFILEHEADER, nFileNameLength)) +
                            read_uint16(nOffset + offsetof(CENTRALDIRECTORYFILEHEADER, nExtraFieldLength)) +
                            read_uint16(nOffset + offsetof(CENTRALDIRECTORYFILEHEADER, nFileCommentLength)));
            } else {
                break;
            }
        }
    } else {
        qint64 nOffset = 0;

        for (int i = 0; i < 20; i++) {
            quint32 nSignature = read_uint32(nOffset + offsetof(LOCALFILEHEADER, nSignature));

            if (nSignature == SIGNATURE_CFD) {
                quint16 nMethod = read_uint16(nOffset + offsetof(LOCALFILEHEADER, nMethod));
                quint16 nFlags = read_uint16(nOffset + offsetof(LOCALFILEHEADER, nFlags));
                quint32 nUncompressedSize = read_uint32(nOffset + offsetof(LOCALFILEHEADER, nUncompressedSize));

                if (nUncompressedSize > 0) {
                    HANDLE_METHOD method = zipToCompressMethod(nMethod, nFlags);

                    if (!listMethods.contains(method)) {
                        listMethods.append(method);
                    }
                }

                nOffset += (sizeof(LOCALFILEHEADER) + read_uint16(nOffset + offsetof(LOCALFILEHEADER, nFileNameLength)) +
                            read_uint16(nOffset + offsetof(LOCALFILEHEADER, nExtraFieldLength)));
            } else {
                break;
            }
        }
    }

    // Sort by the enum value to get the same string for the same set of methods,
    // no matter in which order the records are stored in the archive.
    std::sort(listMethods.begin(), listMethods.end());

    qint32 nNumberOfMethods = listMethods.count();

    for (qint32 i = 0; i < nNumberOfMethods; i++) {
        QString sMethod = handleMethodToString(listMethods.at(i));

        sResult = XBinary::appendText(sResult, sMethod, ", ");
    }

    return sResult;
}

XBinary::FT XZip::getFileType()
{
    // For extra use getFileFormatInfo
    return FT_ZIP;
}

XBinary::FT XZip::_getFileType(QIODevice *pDevice, QList<RECORD> *pListRecords, bool bDeep, PDSTRUCT *pPdStruct)
{
    FT result = FT_ZIP;

    if (!isValid(pDevice, pPdStruct)) {
        return result;
    }

    // We assume pListRecords is already populated by caller for efficiency.
    // If not, we still proceed with defaults.
    const bool hasRecords = (pListRecords && !pListRecords->isEmpty());

    // Fast single-pass classification using record names
    bool seenJarManifest = false;
    bool seenApksTableOfContents = false;
    bool seenApksPayload = false;

    if (hasRecords) {
        for (int idx = 0, n = pListRecords->count(); idx < n; ++idx) {
            const RECORD &rec = pListRecords->at(idx);
            const QString &name = rec.spInfo.sRecordName;
            if (!seenJarManifest && (name == QLatin1String("META-INF/MANIFEST.MF")) && (rec.spInfo.nUncompressedSize > 0)) seenJarManifest = true;
            if (!seenApksTableOfContents && (name == QLatin1String("toc.pb")) && (rec.spInfo.nUncompressedSize != 0)) seenApksTableOfContents = true;
            if (!seenApksPayload && name.endsWith(QLatin1String(".apk"), Qt::CaseInsensitive) && (rec.spInfo.nUncompressedSize != 0)) seenApksPayload = true;
        }
    }

    if (hasRecords) {
        if (XAPK::isValid(pDevice, pListRecords, pPdStruct)) {
            result = FT_APK;
        } else if (XIPA::isValid(pDevice, pListRecords, pPdStruct)) {
            result = FT_IPA;
        } else if (seenJarManifest && XJAR::isValid(pDevice, pListRecords, pPdStruct)) {
            result = FT_JAR;
        } else {
            result = FT_ZIP;
        }
    }

    if (bDeep && hasRecords) {
        // bundletool APK Sets contain a root toc.pb plus one or more APK
        // payloads.  toc.pb is metadata rather than an inner ZIP, and ZIP is
        // free to deflate entries, so an "all stored inner ZIPs" predicate can
        // never describe the real format.
        if ((result != FT_JAR) && (result != FT_APK) && (result != FT_IPA)) {
            if (seenApksTableOfContents && seenApksPayload && XAPKS::isValid(pDevice, pListRecords, pPdStruct)) {
                result = FT_APKS;
            }
        }
    }

    return result;
}

bool XZip::addLocalFileRecord(QIODevice *pSource, QIODevice *pDest, ZIPFILE_RECORD *pZipFileRecord, PDSTRUCT *pPdStruct)
{
    if (!pSource || !pDest || !pZipFileRecord || !pSource->isReadable() || !pDest->isWritable() || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    qint64 nStartPosition = pDest->pos();
    if (pDest->isSequential() && (nStartPosition < 0)) nStartPosition = pZipFileRecord->nHeaderOffset;
    if (!zipCanAppendAt(pDest, nStartPosition) || ((quint64)nStartPosition >= (std::numeric_limits<quint32>::max)())) {
        return false;
    }

    ZIPFILE_RECORD record = *pZipFileRecord;
    if ((record.method != CMETHOD_STORE) && (record.method != CMETHOD_DEFLATE)) return false;
    if (record.nFlags & (0x0001 | 0x0008)) return false;
    record.nFlags |= ZIP_FLAG_UTF8;
    if (record.nMinVersion == 0) record.nMinVersion = 0x14;
    if (record.nVersion == 0) record.nVersion = 0x3F;
    if (!record.dtTime.isValid()) record.dtTime = QDateTime::currentDateTime();

    const QByteArray baFileName = record.sFileName.toUtf8();
    if (baFileName.isEmpty() || (baFileName.size() > (std::numeric_limits<quint16>::max)())) return false;

    QTemporaryFile stagedSource;
    qint64 nSourceSize = 0;
    quint32 nSourceCRC32 = 0;
    const qint64 nExpectedSize = pSource->isSequential() ? -1 : pSource->size();
    if (!zipStageSource(pSource, &stagedSource, nExpectedSize, pPdStruct, &nSourceSize, &nSourceCRC32)) {
        return false;
    }
    record.nUncompressedSize = nSourceSize;
    record.nCRC32 = nSourceCRC32;

    QTemporaryFile compressedPayload;
    QIODevice *pPayload = &stagedSource;
    if (record.method == CMETHOD_DEFLATE) {
        if (!compressedPayload.open() ||
            (XArchive::_compress(XArchive::HANDLE_METHOD_DEFLATE, &stagedSource, &compressedPayload, pPdStruct) != XArchive::COMPRESS_RESULT_OK) ||
            !compressedPayload.flush() || !compressedPayload.seek(0) || !XBinary::isPdStructNotCanceled(pPdStruct)) {
            return false;
        }
        pPayload = &compressedPayload;
    } else if (!stagedSource.seek(0)) {
        return false;
    }

    record.nCompressedSize = pPayload->size();
    if ((record.nCompressedSize < 0) || ((quint64)record.nCompressedSize >= (std::numeric_limits<quint32>::max)()) || !pPayload->seek(0)) {
        return false;
    }

    const qint64 nRecordSize = (qint64)sizeof(LOCALFILEHEADER) + baFileName.size() + record.nCompressedSize;
    if ((quint64)nRecordSize >= ((quint64)(std::numeric_limits<quint32>::max)() - (quint64)nStartPosition)) return false;

    record.nHeaderOffset = nStartPosition;
    record.nDataOffset = nStartPosition + (qint64)sizeof(LOCALFILEHEADER) + baFileName.size();

    LOCALFILEHEADER header = {};
    header.nSignature = SIGNATURE_LFD;
    header.nMinVersion = record.nMinVersion;
    header.nMinOS = record.nMinOS;
    header.nFlags = record.nFlags;
    header.nMethod = record.method;
    const QPair<quint16, quint16> dosDateTime = XBinary::qDateTimeToDosDateTime(record.dtTime);
    header.nLastModDate = dosDateTime.first;
    header.nLastModTime = dosDateTime.second;
    header.nCRC32 = record.nCRC32;
    header.nCompressedSize = (quint32)record.nCompressedSize;
    header.nUncompressedSize = (quint32)record.nUncompressedSize;
    header.nFileNameLength = (quint16)baFileName.size();

    if (!zipWriteAll(pDest, reinterpret_cast<const char *>(&header), sizeof(header), pPdStruct) ||
        !zipWriteAll(pDest, baFileName.constData(), baFileName.size(), pPdStruct) || !zipCopyExactly(pPayload, pDest, record.nCompressedSize, pPdStruct)) {
        zipRollbackWrite(pDest, nStartPosition);
        return false;
    }

    *pZipFileRecord = record;
    return true;
}

bool XZip::addCentralDirectory(QIODevice *pDest, QList<XZip::ZIPFILE_RECORD> *pListZipFileRecords, const QString &sComment, PDSTRUCT *pPdStruct, qint64 nStartPosition)
{
    if (!pDest || !pListZipFileRecords || !pDest->isWritable() || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    if (nStartPosition < 0) {
        if (!pDest->isSequential()) {
            nStartPosition = pDest->pos();
        } else if (!pListZipFileRecords->isEmpty()) {
            const ZIPFILE_RECORD &lastRecord = pListZipFileRecords->last();
            nStartPosition = lastRecord.nDataOffset + lastRecord.nCompressedSize;
        } else {
            nStartPosition = 0;
        }
    }

    const QByteArray baComment = sComment.toUtf8();
    qint64 nWritten = 0;
    if (!zipAppendCentralDirectory(pDest, pListZipFileRecords, baComment, nStartPosition, pPdStruct, &nWritten)) {
        zipRollbackWrite(pDest, nStartPosition);
        return false;
    }

    return true;
}

QString XZip::getFileFormatExt()
{
    return "zip";
}

QString XZip::getFileFormatExtsString()
{
    return "zip,apk,apks,jar,ipa";  // TODO
}

qint64 XZip::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    // Fast path: use validated last End of Central Directory (ECD) offset.
    const qint64 nTotalSize = getSize();
    qint64 nResult = 0;

    const qint64 nECDOffset = findECDOffset(pPdStruct);
    if (nECDOffset != -1) {
        qint64 nEnd = nECDOffset + sizeof(ENDOFCENTRALDIRECTORYRECORD) + read_uint16(nECDOffset + offsetof(ENDOFCENTRALDIRECTORYRECORD, nCommentLength));
        // Clamp to file size for robustness
        if (nEnd > nTotalSize) nEnd = nTotalSize;
        nResult = nEnd;
    } else {
        // Fallback: compute real size from Local File Headers when ECD is missing
        qint64 nRealSize = 0;
        _getNumberOfLocalFileHeaders(0, nTotalSize, &nRealSize, pPdStruct);
        nResult = nRealSize;
    }

    return nResult;
}

QString XZip::getMIMEString()
{
    return "application/zip";
}

XZip::CENTRALDIRECTORYFILEHEADER XZip::read_CENTRALDIRECTORYFILEHEADER(qint64 nOffset, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)
    CENTRALDIRECTORYFILEHEADER result = {};
    if (read_array(nOffset, reinterpret_cast<char *>(&result), sizeof(result)) != sizeof(result)) {
        return CENTRALDIRECTORYFILEHEADER();
    }

    return result;
}

XZip::LOCALFILEHEADER XZip::read_LOCALFILEHEADER(qint64 nOffset, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)
    LOCALFILEHEADER result = {};
    if (read_array(nOffset, reinterpret_cast<char *>(&result), sizeof(result)) != sizeof(result)) {
        return LOCALFILEHEADER();
    }

    return result;
}

XZip::AES_EXTRA_FIELD XZip::read_AES_EXTRA_FIELD(qint64 nOffset, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    AES_EXTRA_FIELD result = {};
    if (read_array(nOffset, reinterpret_cast<char *>(&result), sizeof(result)) != sizeof(result)) {
        return AES_EXTRA_FIELD();
    }

    return result;
}

bool XZip::_readFileName(qint64 nFileNameOffset, qint64 nFileNameLength, quint16 nFlags, qint64 nExtraFieldOffset, qint64 nExtraFieldLength, QString *pFileName)
{
    QPointer<XZip> guardedArchive(this);
    if (!pFileName) {
        return false;
    }

    pFileName->clear();
    const qint64 nFileSize = guardedArchive->getSize();
    if (!guardedArchive) return false;
    if ((nFileNameOffset < 0) || (nFileNameLength < 0) || (nFileNameOffset > nFileSize) || (nFileNameLength > (nFileSize - nFileNameOffset)) || (nExtraFieldOffset < 0) ||
        (nExtraFieldLength < 0) || (nExtraFieldOffset > nFileSize) || (nExtraFieldLength > (nFileSize - nExtraFieldOffset)) ||
        (nFileNameLength > (qint64)(std::numeric_limits<qint32>::max)()) || (nExtraFieldLength > (qint64)(std::numeric_limits<qint32>::max)())) {
        return false;
    }

    const QByteArray baRawName = guardedArchive->read_array(nFileNameOffset, nFileNameLength);
    if (!guardedArchive) return false;
    const QByteArray baExtraField = guardedArchive->read_array(nExtraFieldOffset, nExtraFieldLength);
    if (!guardedArchive) return false;
    if ((baRawName.size() != nFileNameLength) || (baExtraField.size() != nExtraFieldLength)) {
        return false;
    }

    return decodeZipFileName(baRawName, nFlags, baExtraField, pFileName);
}

static bool zipReadExact(QPointer<XZip> *pGuardedArchive, QPointer<QIODevice> *pGuardedSource, qint64 nSize, XBinary::PDSTRUCT *pPdStruct, qint64 nOffset, qint64 nLength,
                         QByteArray *pData)
{
    if (!pData || pGuardedArchive->isNull() || pGuardedSource->isNull() || (nOffset < 0) || (nLength < 0) || (nOffset > nSize) || (nLength > (nSize - nOffset)))
        return false;
    *pData = XBinary::read_array_process(pGuardedSource->data(), nOffset, nLength, pPdStruct);
    return !pGuardedArchive->isNull() && !pGuardedSource->isNull() && (pData->size() == nLength);
}

// PKZIP for Windows writes the OEM (DOS codepage) spelling of a member name
// in the central directory and the ANSI spelling in the local header, so a
// non-ASCII name can differ byte-for-byte between the two headers. Require
// identical length and identical 7-bit ASCII bytes, and tolerate a difference
// only at positions where BOTH sides carry an extended (>= 0x80) byte — the
// only bytes a codepage remap can touch. This keeps the linkage check strict
// for every plain-ASCII name while accepting the legitimate codepage variance.
static bool zipNamesEquivalent(const QByteArray &baLocalName, const QByteArray &baCentralName)
{
    if (baLocalName.size() != baCentralName.size()) return false;
    for (qint32 i = 0; i < baLocalName.size(); ++i) {
        const quint8 nLocal = static_cast<quint8>(baLocalName.at(i));
        const quint8 nCentral = static_cast<quint8>(baCentralName.at(i));
        if (nLocal == nCentral) continue;
        if ((nLocal < 0x80) || (nCentral < 0x80)) return false;
    }
    return true;
}

static bool zipLocalRangeLessThan(const QPair<qint64, qint64> &a, const QPair<qint64, qint64> &b)
{
    return a.first < b.first;
}

// Cross-probe EOCD cache.
//
// XZip::isValid(QIODevice *) and the format probes build a throwaway XZip per
// call, so a per-instance cache never survives to a second probe and every probe
// repeats the backward EOCD scan over the last 64 KB. The cache therefore lives
// on the QIODevice, which does outlive the probe.
//
// A bare device cache would be unsafe: the device is beyond the reach of
// XBinary::setDevice()/setInternalInfo(nullptr), so a device reused for new
// contents (QBuffer::setData(), a reopened file) would keep answering with the
// previous file's EOCD offset. Every entry therefore carries a validity stamp --
// the device size plus a checksum over the first and last 32 bytes -- and is
// rejected unless both still match the device in front of us. The head window
// covers the first local header (version/flags/method/CRC32/sizes) and the tail
// window covers the EOCD the offset points at, so a reuse that keeps both the
// size and those 64 bytes identical is the only way to get a stale hit.
static const char ZIP_ECD_CACHED[] = "XZip_ECDCached";
static const char ZIP_ECD_OFFSET[] = "XZip_ECDOffset";
static const char ZIP_ECD_SIZE[] = "XZip_ECDSize";
static const char ZIP_ECD_STAMP[] = "XZip_ECDStamp";

static const qint64 N_ZIP_ECD_STAMP_WINDOW = 32;

static quint32 zipDeviceStamp(QIODevice *pDevice, qint64 nSize, XBinary::PDSTRUCT *pPdStruct)
{
    quint32 nResult = 2166136261u;  // FNV-1a offset basis

    const qint64 nWindow = qMin(nSize, N_ZIP_ECD_STAMP_WINDOW);
    if (nWindow <= 0) return nResult;

    QByteArray baHead = XBinary::read_array_process(pDevice, 0, nWindow, pPdStruct);
    QByteArray baTail = XBinary::read_array_process(pDevice, qMax((qint64)0, nSize - nWindow), nWindow, pPdStruct);

    for (qint32 i = 0; i < baHead.size(); ++i) {
        nResult = (nResult ^ (quint32)(quint8)baHead.at(i)) * 16777619u;
    }

    for (qint32 i = 0; i < baTail.size(); ++i) {
        nResult = (nResult ^ (quint32)(quint8)baTail.at(i)) * 16777619u;
    }

    return nResult;
}

qint64 XZip::findECDOffset(PDSTRUCT *pPdStruct)
{
    QPointer<XZip> guardedArchive(this);
    QPointer<QIODevice> guardedSource(getDevice());
    qint64 nResult = -1;
    if (!guardedSource) return -1;
    const qint64 nSize = guardedSource->size();
    if (!guardedArchive || !guardedSource) return -1;

    // First level: per-instance. Free, and the only level that can answer while
    // the same object is reused (xapk.cpp calls findECDOffset twice on one).
    if (m_internalInfo.bECDOffsetCached) {
        return guardedArchive && guardedSource ? m_internalInfo.nECDOffset : -1;
    }

    // Second level: on the device, so it survives the throwaway probe instances.
    // Only honoured while the stamp still describes the bytes actually there.
    const quint32 nStamp = zipDeviceStamp(guardedSource.data(), nSize, pPdStruct);
    if (!guardedArchive || !guardedSource) return -1;

    if (guardedSource->property(ZIP_ECD_CACHED).toBool() && (guardedSource->property(ZIP_ECD_SIZE).toLongLong() == nSize) &&
        (guardedSource->property(ZIP_ECD_STAMP).toUInt() == nStamp)) {
        nResult = guardedSource->property(ZIP_ECD_OFFSET).toLongLong();
        m_internalInfo.bECDOffsetCached = true;
        m_internalInfo.nECDOffset = nResult;
        return guardedArchive && guardedSource ? nResult : -1;
    }

    if (nSize >= 22)  // 22 is minimum size [0x50,0x4B,0x05,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00]
    {
        const qint64 nMaxECDSearchSize = 0xFFFF + (qint64)sizeof(ENDOFCENTRALDIRECTORYRECORD);
        const qint64 nSearchOffset = qMax((qint64)0, nSize - nMaxECDSearchSize);
        const QByteArray baSearch = XBinary::read_array_process(guardedSource.data(), nSearchOffset, nSize - nSearchOffset, pPdStruct);
        if (!guardedArchive || !guardedSource || (baSearch.size() != (nSize - nSearchOffset))) return -1;

        static const QByteArray baECDSignature("PK\x05\x06", 4);
        QList<qint32> listCandidates;
        for (qint32 nPos = 0; nPos <= baSearch.size() - (qint32)baECDSignature.size();) {
            const qint32 nFound = baSearch.indexOf(baECDSignature, nPos);
            if (nFound < 0) break;
            listCandidates.append(nFound);
            nPos = nFound + 1;
        }

        for (qint32 i = listCandidates.size() - 1; i >= 0 && XBinary::isPdStructNotCanceled(pPdStruct); --i) {
            const qint64 nCurrent = nSearchOffset + listCandidates.at(i);

            if ((nCurrent < 0) || ((nSize - nCurrent) < (qint64)sizeof(ENDOFCENTRALDIRECTORYRECORD))) {
                continue;
            }

            ENDOFCENTRALDIRECTORYRECORD ecd = {};
            memcpy(&ecd, baSearch.constData() + (nCurrent - nSearchOffset), sizeof(ecd));
            const quint16 nCommentLength = ecd.nCommentLength;
            if ((qint64)nCommentLength > (nSize - nCurrent - (qint64)sizeof(ENDOFCENTRALDIRECTORYRECORD))) {
                continue;
            }

            const quint16 nDiskNumber = ecd.nDiskNumber;
            const quint16 nStartDisk = ecd.nStartDisk;
            const quint16 nDiskRecords = ecd.nDiskNumberOfRecords;
            const quint16 nTotalRecords = ecd.nTotalNumberOfRecords;
            const quint32 nCentralDirectorySize = ecd.nSizeOfCentralDirectory;
            const qint64 nOffsetToCentralDirectory = ecd.nOffsetToCentralDirectory;

            // Multi-disk and ZIP64 archives need different record layouts and
            // are not implemented by this reader. Do not reinterpret their
            // sentinel values as ordinary 32-bit offsets/counts.
            if ((nDiskNumber != 0) || (nStartDisk != 0) || (nDiskRecords != nTotalRecords) || (nTotalRecords == 0xFFFF) || (nCentralDirectorySize == 0xFFFFFFFF) ||
                (nOffsetToCentralDirectory == 0xFFFFFFFF)) {
                continue;
            }

            if (nTotalRecords == 0) {
                if ((nCentralDirectorySize == 0) && (nOffsetToCentralDirectory == nCurrent)) {
                    nResult = nCurrent;
                    break;
                }
                continue;
            }

            if ((nOffsetToCentralDirectory < 0) || (nOffsetToCentralDirectory > nCurrent) || ((qint64)nCentralDirectorySize != (nCurrent - nOffsetToCentralDirectory))) {
                continue;
            }

            qint64 nCurrentHeaderOffset = nOffsetToCentralDirectory;
            bool bValid = true;
            QSet<qint64> setLocalHeaderOffsets;
            QList<QPair<qint64, qint64>> listLocalRanges;

            for (quint32 i = 0; i < nTotalRecords; i++) {
                if ((nCurrent - nCurrentHeaderOffset) < (qint64)sizeof(CENTRALDIRECTORYFILEHEADER)) {
                    bValid = false;
                    break;
                }

                QByteArray baCentralHeader;
                if (!zipReadExact(&guardedArchive, &guardedSource, nSize, pPdStruct, nCurrentHeaderOffset, sizeof(CENTRALDIRECTORYFILEHEADER), &baCentralHeader))
                    return -1;
                CENTRALDIRECTORYFILEHEADER cdfh = {};
                memcpy(&cdfh, baCentralHeader.constData(), sizeof(cdfh));
                if (cdfh.nSignature != SIGNATURE_CFD) {
                    bValid = false;
                    break;
                }
                qint64 nRecordSize = sizeof(CENTRALDIRECTORYFILEHEADER) + (qint64)cdfh.nFileNameLength + (qint64)cdfh.nExtraFieldLength + (qint64)cdfh.nFileCommentLength;
                // InstallShield 3's single-volume ZIP writer used one-based
                // disk numbers in central records while its EOCD used the
                // standard zero-based value. Accept exactly that legacy 1.
                if ((cdfh.nStartDisk > 1) || (cdfh.nCompressedSize == 0xFFFFFFFF) || (cdfh.nUncompressedSize == 0xFFFFFFFF) ||
                    (cdfh.nOffsetToLocalFileHeader == 0xFFFFFFFF) ||
                    ((cdfh.nMethod == CMETHOD_STORE) && !(cdfh.nFlags & 0x0001) && (cdfh.nCompressedSize != cdfh.nUncompressedSize)) ||
                    (nRecordSize > (nCurrent - nCurrentHeaderOffset))) {
                    bValid = false;
                    break;
                }

                const qint64 nCentralNameOffset = nCurrentHeaderOffset + sizeof(CENTRALDIRECTORYFILEHEADER);
                const qint64 nCentralExtraOffset = nCentralNameOffset + cdfh.nFileNameLength;
                QString sDecodedName;
                const bool bNameRead =
                    guardedArchive->_readFileName(nCentralNameOffset, cdfh.nFileNameLength, cdfh.nFlags, nCentralExtraOffset, cdfh.nExtraFieldLength, &sDecodedName);
                if (!guardedArchive || !guardedSource) return -1;
                if (!bNameRead) {
                    bValid = false;
                    break;
                }

                qint64 nLocalHeaderOffset = cdfh.nOffsetToLocalFileHeader;
                if ((nLocalHeaderOffset < 0) || ((nOffsetToCentralDirectory - nLocalHeaderOffset) < (qint64)sizeof(LOCALFILEHEADER)) ||
                    setLocalHeaderOffsets.contains(nLocalHeaderOffset)) {
                    bValid = false;
                    break;
                }
                setLocalHeaderOffsets.insert(nLocalHeaderOffset);

                QByteArray baLocalHeader;
                if (!zipReadExact(&guardedArchive, &guardedSource, nSize, pPdStruct, nLocalHeaderOffset, sizeof(LOCALFILEHEADER), &baLocalHeader)) return -1;
                LOCALFILEHEADER lfh = {};
                memcpy(&lfh, baLocalHeader.constData(), sizeof(lfh));
                qint64 nLocalDataOffset = nLocalHeaderOffset + sizeof(LOCALFILEHEADER) + (qint64)lfh.nFileNameLength + (qint64)lfh.nExtraFieldLength;

                QByteArray baLocalName;
                QByteArray baCentralName;
                const bool bNamesRead =
                    zipReadExact(&guardedArchive, &guardedSource, nSize, pPdStruct, nLocalHeaderOffset + sizeof(LOCALFILEHEADER), lfh.nFileNameLength, &baLocalName) &&
                    zipReadExact(&guardedArchive, &guardedSource, nSize, pPdStruct, nCurrentHeaderOffset + sizeof(CENTRALDIRECTORYFILEHEADER), cdfh.nFileNameLength,
                                 &baCentralName);
                if (!guardedArchive || !guardedSource) return -1;

                // Old InstallShield ZIP emitters sometimes retained Deflate's
                // compressor-option bits (1-2) only in the local header. They
                // do not affect decoding; all semantic/security flags must
                // still agree with the central directory. PKZIP 2.60 for
                // Windows additionally sets the PKWARE-reserved bit 15 only in
                // local headers, so tolerate that bit differing too.
                const bool bFlagsCompatible =
                    (lfh.nFlags == cdfh.nFlags) ||
                    ((lfh.nMethod == CMETHOD_DEFLATE) &&
                     (((lfh.nFlags ^ cdfh.nFlags) & quint16(~0x8006U)) == 0));

                if (!bNamesRead || (lfh.nSignature != SIGNATURE_LFD) || (lfh.nMinVersion != cdfh.nMinVersion) || (lfh.nMinOS != cdfh.nMinOS) ||
                    !bFlagsCompatible || (lfh.nMethod != cdfh.nMethod) || (lfh.nFileNameLength != cdfh.nFileNameLength) ||
                    (nLocalDataOffset > nOffsetToCentralDirectory) || ((qint64)cdfh.nCompressedSize > (nOffsetToCentralDirectory - nLocalDataOffset)) ||
                    !zipNamesEquivalent(baLocalName, baCentralName) ||
                    (!(lfh.nFlags & 0x0008) &&
                     ((lfh.nCRC32 != cdfh.nCRC32) || (lfh.nCompressedSize != cdfh.nCompressedSize) || (lfh.nUncompressedSize != cdfh.nUncompressedSize)))) {
                    bValid = false;
                    break;
                }

                qint64 nLocalRecordEnd = nLocalDataOffset + (qint64)cdfh.nCompressedSize;
                if (lfh.nFlags & 0x0008) {
                    // With bit 3 set, local size/CRC fields are placeholders and
                    // the descriptor is the authenticated source of those values.
                    if (((lfh.nCRC32 != 0) && (lfh.nCRC32 != cdfh.nCRC32)) || ((lfh.nCompressedSize != 0) && (lfh.nCompressedSize != cdfh.nCompressedSize)) ||
                        ((lfh.nUncompressedSize != 0) && (lfh.nUncompressedSize != cdfh.nUncompressedSize))) {
                        bValid = false;
                        break;
                    }

                    const qint64 nDescriptorOffset = nLocalRecordEnd;
                    bool bDescriptorValid = false;
                    qint64 nDescriptorSize = 0;
                    QByteArray baDescriptor;
                    const qint64 nAvailableDescriptor = nOffsetToCentralDirectory - nDescriptorOffset;
                    const qint64 nDescriptorReadSize = qMin<qint64>(16, qMax<qint64>(0, nAvailableDescriptor));
                    if ((nDescriptorReadSize >= 12) &&
                        !zipReadExact(&guardedArchive, &guardedSource, nSize, pPdStruct, nDescriptorOffset, nDescriptorReadSize, &baDescriptor))
                        return -1;
                    char *pDescriptor = baDescriptor.data();
                    if ((baDescriptor.size() >= 16) && (XBinary::_read_uint32(pDescriptor) == 0x08074B50) && (XBinary::_read_uint32(pDescriptor + 4) == cdfh.nCRC32) &&
                        (XBinary::_read_uint32(pDescriptor + 8) == cdfh.nCompressedSize) && (XBinary::_read_uint32(pDescriptor + 12) == cdfh.nUncompressedSize)) {
                        bDescriptorValid = true;
                        nDescriptorSize = 16;
                    }
                    // A signature is optional, and a legitimate CRC may itself
                    // equal 0x08074B50. Check the unsigned form independently.
                    if (!bDescriptorValid && (baDescriptor.size() >= 12) && (XBinary::_read_uint32(pDescriptor) == cdfh.nCRC32) &&
                        (XBinary::_read_uint32(pDescriptor + 4) == cdfh.nCompressedSize) && (XBinary::_read_uint32(pDescriptor + 8) == cdfh.nUncompressedSize)) {
                        bDescriptorValid = true;
                        nDescriptorSize = 12;
                    }
                    if (!bDescriptorValid) {
                        bValid = false;
                        break;
                    }
                    nLocalRecordEnd += nDescriptorSize;
                }

                listLocalRanges.append(qMakePair(nLocalHeaderOffset, nLocalRecordEnd));

                nCurrentHeaderOffset += nRecordSize;
            }

            if (bValid) {
                std::sort(listLocalRanges.begin(), listLocalRanges.end(), zipLocalRangeLessThan);
                for (qint32 i = 0; i < listLocalRanges.size(); i++) {
                    const QPair<qint64, qint64> &range = listLocalRanges.at(i);
                    if ((range.first < 0) || (range.second <= range.first) || (range.second > nOffsetToCentralDirectory) ||
                        ((i > 0) && (range.first < listLocalRanges.at(i - 1).second))) {
                        bValid = false;
                        break;
                    }
                }
            }

            // The optional central-directory digital-signature and archive-extra
            // records are not files and are not included in the EOCD entry count.
            while (bValid && (nCurrentHeaderOffset < nCurrent)) {
                if ((nCurrent - nCurrentHeaderOffset) < 6) {
                    bValid = false;
                    break;
                }

                QByteArray baOptionalHeader;
                if (!zipReadExact(&guardedArchive, &guardedSource, nSize, pPdStruct, nCurrentHeaderOffset, qMin<qint64>(8, nCurrent - nCurrentHeaderOffset),
                                  &baOptionalHeader))
                    return -1;
                const quint32 nSignature = XBinary::_read_uint32(baOptionalHeader.data());
                qint64 nRecordSize = 0;
                if (nSignature == 0x05054B50) {  // central-directory digital signature
                    nRecordSize = 6 + (qint64)XBinary::_read_uint16(baOptionalHeader.data() + 4);
                } else if (nSignature == 0x08064B50) {  // archive extra data record
                    if ((nCurrent - nCurrentHeaderOffset) < 8) {
                        bValid = false;
                        break;
                    }
                    nRecordSize = 8 + (qint64)XBinary::_read_uint32(baOptionalHeader.data() + 4);
                } else {
                    bValid = false;
                    break;
                }

                if ((nRecordSize <= 0) || (nRecordSize > (nCurrent - nCurrentHeaderOffset))) {
                    bValid = false;
                    break;
                }
                nCurrentHeaderOffset += nRecordSize;
            }

            if (bValid && (nCurrentHeaderOffset == nCurrent)) {
                nResult = nCurrent;
                break;
            }
        }
    }

    if (guardedArchive && guardedSource && XBinary::isPdStructNotCanceled(pPdStruct)) {
        m_internalInfo.bECDOffsetCached = true;
        m_internalInfo.nECDOffset = nResult;

        // A negative result is cached too: proving "not a ZIP" costs the same
        // full scan, and the stamp keeps the entry honest either way.
        guardedSource->setProperty(ZIP_ECD_CACHED, true);
        guardedSource->setProperty(ZIP_ECD_OFFSET, nResult);
        guardedSource->setProperty(ZIP_ECD_SIZE, nSize);
        guardedSource->setProperty(ZIP_ECD_STAMP, nStamp);
    }

    return guardedArchive && guardedSource && XBinary::isPdStructNotCanceled(pPdStruct) ? nResult : -1;
}

bool XZip::isAPK(qint64 nECDOffset, PDSTRUCT *pPdStruct)
{
    return _isRecordNamePresent(nECDOffset, "AndroidManifest.xml", "", pPdStruct, false, true);
}

bool XZip::isIPA(qint64 nECDOffset, PDSTRUCT *pPdStruct)
{
    return _isRecordNamePresent(nECDOffset, "Payload/", "", pPdStruct, true);
}

bool XZip::isJAR(qint64 nECDOffset, PDSTRUCT *pPdStruct)
{
    return _isRecordNamePresent(nECDOffset, "META-INF/MANIFEST.MF", "", pPdStruct, false, true);
}

QString XZip::structIDToString(quint32 nID)
{
    return XBinary::XCONVERT_idToTransString(nID, _TABLE_XZip_STRUCTID, sizeof(_TABLE_XZip_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QString XZip::structIDToFtString(quint32 nID)
{
    return XBinary::XCONVERT_idToFtString(nID, _TABLE_XZip_STRUCTID, sizeof(_TABLE_XZip_STRUCTID) / sizeof(XBinary::XCONVERT));
}

quint32 XZip::ftStringToStructID(const QString &sFtString)
{
    return XCONVERT_ftStringToId(sFtString, _TABLE_XZip_STRUCTID, sizeof(_TABLE_XZip_STRUCTID) / sizeof(XBinary::XCONVERT));
}

// qint32 XZip::readTableRow(qint32 nRow, LT locType, XADDR nLocation, const DATA_RECORDS_OPTIONS &dataRecordsOptions, QList<DATA_RECORD_ROW> *pListDataRecords,
//                           void *pUserData, PDSTRUCT *pPdStruct)
// {
//     qint32 nResult = 0;

//     if (dataRecordsOptions.dataHeaderFirst.dsID.nID == STRUCTID_LOCALFILEHEADER) {
//         nResult = XBinary::readTableRow(nRow, locType, nLocation, dataRecordsOptions, pListDataRecords, pUserData, pPdStruct);

//         qint64 nStartOffset = locationToOffset(dataRecordsOptions.pMemoryMap, locType, nLocation);
// #ifdef QT_DEBUG
//         qDebug("XZip::readTableRow nStartOffset=%llX", nStartOffset);
// #endif
//         quint32 nLocalSignature = read_uint32(nStartOffset + offsetof(LOCALFILEHEADER, nSignature));
//         quint32 nLocalFileNameSize = read_uint16(nStartOffset + offsetof(LOCALFILEHEADER, nFileNameLength));
//         quint32 nLocalExtraFieldSize = read_uint16(nStartOffset + offsetof(LOCALFILEHEADER, nExtraFieldLength));
//         quint32 nCompressedSize = read_uint32(nStartOffset + offsetof(LOCALFILEHEADER, nCompressedSize));

//         if (nLocalSignature == SIGNATURE_LFD) {
//             nResult = sizeof(LOCALFILEHEADER) + nLocalFileNameSize + nLocalExtraFieldSize + nCompressedSize;
//         }
//     } else {
//         nResult = XBinary::readTableRow(nRow, locType, nLocation, dataRecordsOptions, pListDataRecords, pUserData, pPdStruct);
//     }

//     return nResult;
// }

QMap<quint64, QString> XZip::getHeaderSignatures()
{
    return XBinary::XIDSTRING_createMapPrefix(_TABLE_XZip_HeaderSignatures, sizeof(_TABLE_XZip_HeaderSignatures) / sizeof(XBinary::XIDSTRING), PREFIX_SIGNATURE);
}

QMap<quint64, QString> XZip::getHeaderSignaturesS()
{
    return XBinary::XIDSTRING_createMap(_TABLE_XZip_HeaderSignatures, sizeof(_TABLE_XZip_HeaderSignatures) / sizeof(XBinary::XIDSTRING));
}

XBinary::_MEMORY_MAP XZip::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
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

QList<XBinary::MAPMODE> XZip::getMapModesList()
{
    QList<MAPMODE> listResult;

    listResult.append(MAPMODE_DATA);
    listResult.append(MAPMODE_REGIONS);
    listResult.append(MAPMODE_STREAMS);

    return listResult;
}

// QList<XBinary::DATA_HEADER> XZip::getDataHeaders(const DATA_HEADERS_OPTIONS &dataHeadersOptions, PDSTRUCT *pPdStruct)
// {
//     QList<XBinary::DATA_HEADER> listResult;

//     if (dataHeadersOptions.nID == STRUCTID_UNKNOWN) {
//         DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
//         _dataHeadersOptions.bChildren = true;
//         _dataHeadersOptions.dsID_parent = _addDefaultHeaders(&listResult, pPdStruct);

//         qint64 nECDOffset = findECDOffset(pPdStruct);

//         if (nECDOffset != -1) {
//             _dataHeadersOptions.dhMode = XBinary::DHMODE_HEADER;
//             _dataHeadersOptions.fileType = dataHeadersOptions.pMemoryMap->fileType;
//             _dataHeadersOptions.nID = STRUCTID_ENDOFCENTRALDIRECTORYRECORD;
//             _dataHeadersOptions.locType = LT_OFFSET;
//             _dataHeadersOptions.nLocation = nECDOffset;

//             listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//         } else {
//             qint64 nRealSize = 0;
//             qint32 nCount = _getNumberOfLocalFileHeaders(0, getSize(), &nRealSize, pPdStruct);

//             _dataHeadersOptions.dhMode = XBinary::DHMODE_TABLE;
//             _dataHeadersOptions.fileType = dataHeadersOptions.pMemoryMap->fileType;
//             _dataHeadersOptions.nID = STRUCTID_LOCALFILEHEADER;
//             _dataHeadersOptions.locType = LT_OFFSET;
//             _dataHeadersOptions.nLocation = 0;
//             _dataHeadersOptions.nCount = nCount;
//             _dataHeadersOptions.nSize = nRealSize;

//             listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//         }
//     } else {
//         qint64 nStartOffset = locationToOffset(dataHeadersOptions.pMemoryMap, dataHeadersOptions.locType, dataHeadersOptions.nLocation);

//         if (nStartOffset != -1) {
//             if (dataHeadersOptions.nID == STRUCTID_ENDOFCENTRALDIRECTORYRECORD) {
//                 XBinary::DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, structIDToString(dataHeadersOptions.nID));

//                 qint16 nCommentLength = read_uint16(nStartOffset + offsetof(ENDOFCENTRALDIRECTORYRECORD, nCommentLength));

//                 dataHeader.nSize = sizeof(ENDOFCENTRALDIRECTORYRECORD) + nCommentLength;

//                 dataHeader.listRecords.append(getDataRecordDV(offsetof(ENDOFCENTRALDIRECTORYRECORD, nSignature), sizeof(quint32), "Signature", XBinary::VT_UINT32, 0,
//                                                               XBinary::ENDIAN_LITTLE, XZip::getHeaderSignaturesS(), VL_TYPE_LIST));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(ENDOFCENTRALDIRECTORYRECORD, nDiskNumber), sizeof(quint16), "DiskNumber", XBinary::VT_UINT16,
//                                                             DRF_COUNT, XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(ENDOFCENTRALDIRECTORYRECORD, nStartDisk), sizeof(quint16), "StartDisk", XBinary::VT_UINT16,
//                                                             DRF_COUNT, XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(ENDOFCENTRALDIRECTORYRECORD, nDiskNumberOfRecords), sizeof(quint16), "DiskNumberOfRecords",
//                                                             XBinary::VT_UINT16, DRF_COUNT, XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(ENDOFCENTRALDIRECTORYRECORD, nTotalNumberOfRecords), sizeof(quint16), "TotalNumberOfRecords",
//                                                             XBinary::VT_UINT16, DRF_COUNT, XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(ENDOFCENTRALDIRECTORYRECORD, nSizeOfCentralDirectory), sizeof(quint32), "SizeOfCentralDirectory",
//                                                             XBinary::VT_UINT32, DRF_SIZE, XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(ENDOFCENTRALDIRECTORYRECORD, nOffsetToCentralDirectory), sizeof(quint32),
//                 "OffsetToCentralDirectory",
//                                                             XBinary::VT_UINT32, DRF_OFFSET, XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(ENDOFCENTRALDIRECTORYRECORD, nCommentLength), sizeof(quint16), "CommentLength",
//                 XBinary::VT_UINT16,
//                                                             DRF_COUNT, XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(
//                     getDataRecord(sizeof(ENDOFCENTRALDIRECTORYRECORD), nCommentLength, "Comment", XBinary::VT_CHAR_ARRAY, DRF_VOLATILE, XBinary::ENDIAN_LITTLE));

//                 listResult.append(dataHeader);

//                 if (dataHeadersOptions.bChildren) {
//                     quint16 nTotalNumberOfRecords = read_uint16(nStartOffset + offsetof(ENDOFCENTRALDIRECTORYRECORD, nTotalNumberOfRecords));
//                     quint32 nSizeOfCentralDirectory = read_uint32(nStartOffset + offsetof(ENDOFCENTRALDIRECTORYRECORD, nSizeOfCentralDirectory));
//                     quint32 nOffsetToCentralDirectory = read_uint32(nStartOffset + offsetof(ENDOFCENTRALDIRECTORYRECORD, nOffsetToCentralDirectory));

//                     {
//                         DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
//                         _dataHeadersOptions.nLocation = nOffsetToCentralDirectory;
//                         _dataHeadersOptions.dsID_parent = dataHeader.dsID;
//                         _dataHeadersOptions.dhMode = XBinary::DHMODE_TABLE;
//                         _dataHeadersOptions.nCount = nTotalNumberOfRecords;
//                         _dataHeadersOptions.nSize = nSizeOfCentralDirectory;
//                         _dataHeadersOptions.nID = STRUCTID_CENTRALDIRECTORYFILEHEADER;
//                         listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//                     }
//                     {
//                         DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
//                         _dataHeadersOptions.nLocation = 0;
//                         _dataHeadersOptions.dsID_parent = dataHeader.dsID;
//                         _dataHeadersOptions.dhMode = XBinary::DHMODE_TABLE;
//                         _dataHeadersOptions.nCount = nTotalNumberOfRecords;
//                         _dataHeadersOptions.nSize = nOffsetToCentralDirectory;
//                         _dataHeadersOptions.nID = STRUCTID_LOCALFILEHEADER;
//                         listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//                     }
//                 }
//             } else if (dataHeadersOptions.nID == STRUCTID_CENTRALDIRECTORYFILEHEADER) {
//                 XBinary::DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, structIDToString(dataHeadersOptions.nID));

//                 CENTRALDIRECTORYFILEHEADER cdh = read_CENTRALDIRECTORYFILEHEADER(nStartOffset, pPdStruct);

//                 dataHeader.nSize = sizeof(CENTRALDIRECTORYFILEHEADER) + cdh.nFileNameLength + cdh.nExtraFieldLength + cdh.nFileCommentLength;

//                 dataHeader.listRecords.append(getDataRecordDV(offsetof(CENTRALDIRECTORYFILEHEADER, nSignature), sizeof(quint32), "Signature", XBinary::VT_UINT32, 0,
//                                                               XBinary::ENDIAN_LITTLE, XZip::getHeaderSignaturesS(), VL_TYPE_LIST));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(CENTRALDIRECTORYFILEHEADER, nVersion), sizeof(quint8), "Version", XBinary::VT_UINT8, 0, XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(CENTRALDIRECTORYFILEHEADER, nOS), sizeof(quint8), "OS", XBinary::VT_UINT8, 0, XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(CENTRALDIRECTORYFILEHEADER, nMinVersion), sizeof(quint8), "MinVersion", XBinary::VT_UINT8, 0, XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(CENTRALDIRECTORYFILEHEADER, nMinOS), sizeof(quint8), "MinOS", XBinary::VT_UINT8, 0, XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(CENTRALDIRECTORYFILEHEADER, nFlags), sizeof(quint16), "Flags", XBinary::VT_UINT16, 0, XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(CENTRALDIRECTORYFILEHEADER, nMethod), sizeof(quint16), "Method", XBinary::VT_UINT16, 0, XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(CENTRALDIRECTORYFILEHEADER, nLastModTime), sizeof(quint16), "LastModTime", XBinary::VT_UINT16, 0, XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(CENTRALDIRECTORYFILEHEADER, nLastModDate), sizeof(quint16), "LastModDate", XBinary::VT_UINT16, 0, XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(CENTRALDIRECTORYFILEHEADER, nCRC32), sizeof(quint32), "CRC32", XBinary::VT_UINT32, 0, XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(CENTRALDIRECTORYFILEHEADER, nCompressedSize), sizeof(quint32), "CompressedSize",
//                 XBinary::VT_UINT32,
//                                                             DRF_SIZE, XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(CENTRALDIRECTORYFILEHEADER, nUncompressedSize), sizeof(quint32), "UncompressedSize",
//                                                             XBinary::VT_UINT32, DRF_SIZE, XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(CENTRALDIRECTORYFILEHEADER, nFileNameLength), sizeof(quint16), "FileNameLength",
//                 XBinary::VT_UINT16,
//                                                             DRF_COUNT, XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(CENTRALDIRECTORYFILEHEADER, nExtraFieldLength), sizeof(quint16), "ExtraFieldLength",
//                                                             XBinary::VT_UINT16, DRF_COUNT, XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(CENTRALDIRECTORYFILEHEADER, nFileCommentLength), sizeof(quint16), "FileCommentLength",
//                                                             XBinary::VT_UINT16, DRF_COUNT, XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(CENTRALDIRECTORYFILEHEADER, nStartDisk), sizeof(quint16), "StartDisk", XBinary::VT_UINT16, 0, XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(CENTRALDIRECTORYFILEHEADER, nInternalFileAttributes), sizeof(quint16), "InternalFileAttributes",
//                                                             XBinary::VT_UINT16, 0, XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(CENTRALDIRECTORYFILEHEADER, nExternalFileAttributes), sizeof(quint32), "ExternalFileAttributes",
//                                                             XBinary::VT_UINT32, 0, XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(CENTRALDIRECTORYFILEHEADER, nOffsetToLocalFileHeader), sizeof(quint32), "OffsetToLocalFileHeader",
//                                                             XBinary::VT_UINT32, DRF_OFFSET, XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(
//                     getDataRecord(sizeof(CENTRALDIRECTORYFILEHEADER), cdh.nFileNameLength, "FileName", XBinary::VT_CHAR_ARRAY, DRF_VOLATILE, XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(sizeof(CENTRALDIRECTORYFILEHEADER) + cdh.nFileNameLength, cdh.nExtraFieldLength, "ExtraField",
//                                                             XBinary::VT_BYTE_ARRAY, DRF_VOLATILE, XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(sizeof(CENTRALDIRECTORYFILEHEADER) + cdh.nFileNameLength + cdh.nExtraFieldLength, cdh.nFileCommentLength,
//                                                             "FileComment", XBinary::VT_CHAR_ARRAY, DRF_VOLATILE, XBinary::ENDIAN_LITTLE));

//                 listResult.append(dataHeader);
//             } else if (dataHeadersOptions.nID == STRUCTID_LOCALFILEHEADER) {
//                 XBinary::DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, structIDToString(dataHeadersOptions.nID));

//                 LOCALFILEHEADER lfh = read_LOCALFILEHEADER(nStartOffset, pPdStruct);

//                 dataHeader.nSize = sizeof(LOCALFILEHEADER) + lfh.nFileNameLength + lfh.nExtraFieldLength;

//                 dataHeader.listRecords.append(getDataRecordDV(offsetof(LOCALFILEHEADER, nSignature), sizeof(quint32), "Signature", XBinary::VT_UINT32, 0,
//                                                               XBinary::ENDIAN_LITTLE, XZip::getHeaderSignaturesS(), VL_TYPE_LIST));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(LOCALFILEHEADER, nMinVersion), sizeof(quint8), "MinVersion", XBinary::VT_UINT8, 0, XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(LOCALFILEHEADER, nMinOS), sizeof(quint8), "MinOS", XBinary::VT_UINT8, 0, XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(LOCALFILEHEADER, nFlags), sizeof(quint16), "Flags", XBinary::VT_UINT16, 0,
//                 XBinary::ENDIAN_LITTLE)); dataHeader.listRecords.append(
//                     getDataRecord(offsetof(LOCALFILEHEADER, nMethod), sizeof(quint16), "Method", XBinary::VT_UINT16, 0, XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(LOCALFILEHEADER, nLastModTime), sizeof(quint16), "LastModTime", XBinary::VT_UINT16, 0, XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(LOCALFILEHEADER, nLastModDate), sizeof(quint16), "LastModDate", XBinary::VT_UINT16, 0, XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(LOCALFILEHEADER, nCRC32), sizeof(quint32), "CRC32", XBinary::VT_UINT32, 0,
//                 XBinary::ENDIAN_LITTLE)); dataHeader.listRecords.append(
//                     getDataRecord(offsetof(LOCALFILEHEADER, nCompressedSize), sizeof(quint32), "CompressedSize", XBinary::VT_UINT32, DRF_SIZE,
//                     XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(LOCALFILEHEADER, nUncompressedSize), sizeof(quint32), "UncompressedSize", XBinary::VT_UINT32,
//                                                             DRF_SIZE, XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(LOCALFILEHEADER, nFileNameLength), sizeof(quint16), "FileNameLength", XBinary::VT_UINT16, DRF_COUNT,
//                     XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(LOCALFILEHEADER, nExtraFieldLength), sizeof(quint16), "ExtraFieldLength", XBinary::VT_UINT16,
//                                                             DRF_COUNT, XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(
//                     getDataRecord(sizeof(LOCALFILEHEADER), lfh.nFileNameLength, "FileName", XBinary::VT_CHAR_ARRAY, DRF_VOLATILE, XBinary::ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(sizeof(LOCALFILEHEADER) + lfh.nFileNameLength, lfh.nExtraFieldLength, "ExtraField", XBinary::VT_BYTE_ARRAY,
//                                                             DRF_VOLATILE, XBinary::ENDIAN_LITTLE));

//                 listResult.append(dataHeader);
//             }
//         }
//     }

//     return listResult;
// }

static bool zipPartsCanAppend(qint32 nLimit, const QList<XBinary::FPART> *pListResult)
{
    return (nLimit == -1) || (pListResult->size() < nLimit);
}

static bool zipIsWinZipAesAe2(XZip *pZip, XBinary::PDSTRUCT *pPdStruct, qint64 nExtraFieldOffset, qint64 nExtraFieldLength)
{
    qint64 nCurrentOffset = 0;

    while ((nCurrentOffset + 4) <= nExtraFieldLength) {
        quint16 nHeaderID = pZip->read_uint16(nExtraFieldOffset + nCurrentOffset);
        quint16 nDataSize = pZip->read_uint16(nExtraFieldOffset + nCurrentOffset + 2);
        qint64 nRecordSize = 4 + (qint64)nDataSize;

        if (nRecordSize > (nExtraFieldLength - nCurrentOffset)) {
            break;
        }

        if ((nHeaderID == XZip::ZIP_AES_EXTRA_FIELD_HEADER_ID) && (nDataSize >= XZip::ZIP_AES_EXTRA_FIELD_DATA_SIZE)) {
            XZip::AES_EXTRA_FIELD aesExtraField = pZip->read_AES_EXTRA_FIELD(nExtraFieldOffset + nCurrentOffset, pPdStruct);
            return (aesExtraField.nAESVersion == 2) && (aesExtraField.nVendorID == XZip::ZIP_AES_VENDOR_ID_AE);
        }

        nCurrentOffset += nRecordSize;
    }

    return false;
}

QList<XBinary::FPART> XZip::getFileParts(quint32 nFileParts, qint32 nLimit, PDSTRUCT *pPdStruct)
{
    QList<XBinary::FPART> listResult;

    if ((nLimit < -1) || (nLimit == 0) || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return listResult;
    }

    qint64 nECDOffset = findECDOffset(pPdStruct);
    qint64 nMaxOffset = 0;
    qint64 nTotalSize = getSize();

    if (nECDOffset != -1) {
        if ((nTotalSize >= (qint64)sizeof(ENDOFCENTRALDIRECTORYRECORD)) && (nECDOffset <= (nTotalSize - (qint64)sizeof(ENDOFCENTRALDIRECTORYRECORD)))) {
            quint16 nTotalNumberOfRecords = read_uint16(nECDOffset + offsetof(ENDOFCENTRALDIRECTORYRECORD, nTotalNumberOfRecords));
            quint32 nSizeOfCentralDirectory = read_uint32(nECDOffset + offsetof(ENDOFCENTRALDIRECTORYRECORD, nSizeOfCentralDirectory));
            quint32 nOffsetToCentralDirectory = read_uint32(nECDOffset + offsetof(ENDOFCENTRALDIRECTORYRECORD, nOffsetToCentralDirectory));
            quint16 nCommentLength = read_uint16(nECDOffset + offsetof(ENDOFCENTRALDIRECTORYRECORD, nCommentLength));

            nMaxOffset = qMin(nECDOffset + (qint64)sizeof(ENDOFCENTRALDIRECTORYRECORD) + (qint64)nCommentLength, nTotalSize);

            if ((nFileParts & FILEPART_HEADER) && zipPartsCanAppend(nLimit, &listResult)) {
                FPART record = {};

                record.filePart = FILEPART_HEADER;
                record.nFileOffset = nECDOffset;
                record.nFileSize = nMaxOffset - nECDOffset;
                record.nVirtualAddress = XADDR_MAX;
                record.sName = "End of Central Directory Record";

                listResult.append(record);
            }

            if ((nFileParts & FILEPART_HEADER) || (nFileParts & FILEPART_STREAM)) {
                if (((qint64)nOffsetToCentralDirectory < nECDOffset) && ((qint64)nSizeOfCentralDirectory <= (nECDOffset - (qint64)nOffsetToCentralDirectory))) {
                    qint64 nOffset = nOffsetToCentralDirectory;

                    for (qint32 i = 0; (i < nTotalNumberOfRecords) && zipPartsCanAppend(nLimit, &listResult) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
                        if ((nOffset >= 0) && ((nECDOffset - nOffset) >= (qint64)sizeof(CENTRALDIRECTORYFILEHEADER))) {
                            CENTRALDIRECTORYFILEHEADER cdh = read_CENTRALDIRECTORYFILEHEADER(nOffset, pPdStruct);
                            const qint64 nCentralRecordSize =
                                sizeof(CENTRALDIRECTORYFILEHEADER) + (qint64)cdh.nFileNameLength + (qint64)cdh.nExtraFieldLength + (qint64)cdh.nFileCommentLength;

                            if ((cdh.nSignature == SIGNATURE_CFD) && (nCentralRecordSize <= (nECDOffset - nOffset))) {
                                const qint64 nCentralNameOffset = nOffset + sizeof(CENTRALDIRECTORYFILEHEADER);
                                const qint64 nCentralExtraOffset = nCentralNameOffset + cdh.nFileNameLength;
                                QString sOriginalName;
                                if (!_readFileName(nCentralNameOffset, cdh.nFileNameLength, cdh.nFlags, nCentralExtraOffset, cdh.nExtraFieldLength, &sOriginalName)) {
                                    break;
                                }

                                if ((nFileParts & FILEPART_HEADER) && zipPartsCanAppend(nLimit, &listResult)) {
                                    FPART record = {};

                                    record.filePart = FILEPART_HEADER;
                                    record.nFileOffset = nOffset;
                                    record.nFileSize = (sizeof(CENTRALDIRECTORYFILEHEADER) + cdh.nFileNameLength + cdh.nExtraFieldLength + cdh.nFileCommentLength);
                                    record.nVirtualAddress = XADDR_MAX;
                                    record.sName = QString("%1 %2").arg(tr("Stream")).arg(QString::number(i));
                                    record.mapProperties.insert(FPART_PROP_ORIGINALNAME, sOriginalName);

                                    listResult.append(record);
                                }

                                qint64 nLocalOffset = cdh.nOffsetToLocalFileHeader;

                                if ((nLocalOffset >= 0) && ((qint64)nOffsetToCentralDirectory - nLocalOffset >= (qint64)sizeof(LOCALFILEHEADER))) {
                                    LOCALFILEHEADER lfh = read_LOCALFILEHEADER(nLocalOffset, pPdStruct);
                                    const qint64 nLocalDataOffset = nLocalOffset + sizeof(LOCALFILEHEADER) + (qint64)lfh.nFileNameLength + (qint64)lfh.nExtraFieldLength;

                                    if ((lfh.nSignature == SIGNATURE_LFD) && (nLocalDataOffset <= (qint64)nOffsetToCentralDirectory) &&
                                        ((qint64)cdh.nCompressedSize <= ((qint64)nOffsetToCentralDirectory - nLocalDataOffset))) {
                                        if ((nFileParts & FILEPART_HEADER) || (nFileParts & FILEPART_STREAM)) {
                                            QString sName = QString("%1 %2").arg(tr("Stream")).arg(QString::number(i));

                                            if ((nFileParts & FILEPART_HEADER) && zipPartsCanAppend(nLimit, &listResult)) {
                                                FPART record = {};

                                                record.filePart = FILEPART_HEADER;
                                                record.nFileOffset = nLocalOffset;
                                                record.nFileSize = sizeof(LOCALFILEHEADER) + lfh.nFileNameLength + lfh.nExtraFieldLength;
                                                record.nVirtualAddress = XADDR_MAX;
                                                record.sName = sName;
                                                record.mapProperties.insert(FPART_PROP_ORIGINALNAME, sOriginalName);

                                                listResult.append(record);
                                            }

                                            if ((nFileParts & FILEPART_STREAM) && zipPartsCanAppend(nLimit, &listResult)) {
                                                FPART record = {};

                                                record.filePart = FILEPART_STREAM;
                                                record.nFileOffset = nLocalDataOffset;
                                                record.nFileSize = cdh.nCompressedSize;
                                                record.nVirtualAddress = XADDR_MAX;
                                                record.sName = sName;
                                                record.mapProperties.insert(FPART_PROP_ORIGINALNAME, sOriginalName);
                                                record.mapProperties.insert(FPART_PROP_HANDLEMETHOD, zipToCompressMethod(cdh.nMethod, cdh.nFlags));
                                                record.mapProperties.insert(FPART_PROP_COMPRESSEDSIZE, cdh.nCompressedSize);
                                                record.mapProperties.insert(FPART_PROP_UNCOMPRESSEDSIZE, cdh.nUncompressedSize);

                                                qint64 nExtraFieldOffset = nOffset + sizeof(CENTRALDIRECTORYFILEHEADER) + cdh.nFileNameLength;
                                                bool bHasUsableCRC =
                                                    !((cdh.nMethod == CMETHOD_AES) && zipIsWinZipAesAe2(this, pPdStruct, nExtraFieldOffset, cdh.nExtraFieldLength));
                                                if (bHasUsableCRC) {
                                                    record.mapProperties.insert(FPART_PROP_RESULTCRC, cdh.nCRC32);
                                                    record.mapProperties.insert(FPART_PROP_CRC_TYPE, CRC_TYPE_FFFFFFFF_EDB88320_FFFFFFFFF);
                                                }

                                                if (cdh.nFlags & 0x01) record.mapProperties.insert(FPART_PROP_ENCRYPTED, true);

                                                record.mapProperties.insert(FPART_PROP_DATETIME, XBinary::dosDateTimeToQDateTime(cdh.nLastModDate, cdh.nLastModTime));

                                                listResult.append(record);
                                            }
                                        }
                                    }
                                }
                            }

                            nOffset += nCentralRecordSize;
                        }
                    }
                }
            }
        }
    } else {
        qint64 nRealSize = 0;
        qint32 nCount = _getNumberOfLocalFileHeaders(0, nTotalSize, &nRealSize, pPdStruct);

        qint64 nOffset = 0;

        for (qint32 i = 0; (i < nCount) && zipPartsCanAppend(nLimit, &listResult) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
            if ((nOffset >= 0) && ((nTotalSize - nOffset) >= (qint64)sizeof(LOCALFILEHEADER))) {
                LOCALFILEHEADER lfh = read_LOCALFILEHEADER(nOffset, pPdStruct);

                if (lfh.nSignature == SIGNATURE_LFD) {
                    if ((nFileParts & FILEPART_HEADER) || (nFileParts & FILEPART_STREAM)) {
                        const qint64 nFileNameOffset = nOffset + sizeof(LOCALFILEHEADER);
                        const qint64 nExtraFieldOffset = nFileNameOffset + lfh.nFileNameLength;
                        QString sOriginalName;
                        if (!_readFileName(nFileNameOffset, lfh.nFileNameLength, lfh.nFlags, nExtraFieldOffset, lfh.nExtraFieldLength, &sOriginalName)) {
                            break;
                        }

                        if ((nFileParts & FILEPART_HEADER) && zipPartsCanAppend(nLimit, &listResult)) {
                            FPART record = {};

                            record.filePart = FILEPART_HEADER;
                            record.nFileOffset = nOffset;
                            record.nFileSize = sizeof(LOCALFILEHEADER) + lfh.nFileNameLength + lfh.nExtraFieldLength;
                            record.nVirtualAddress = XADDR_MAX;
                            record.mapProperties.insert(FPART_PROP_ORIGINALNAME, sOriginalName);

                            listResult.append(record);
                        }
                        if ((nFileParts & FILEPART_STREAM) && zipPartsCanAppend(nLimit, &listResult)) {
                            FPART record = {};

                            record.filePart = FILEPART_STREAM;
                            record.nFileOffset = nOffset + sizeof(LOCALFILEHEADER) + lfh.nFileNameLength + lfh.nExtraFieldLength;
                            record.nFileSize = lfh.nCompressedSize;
                            record.nVirtualAddress = XADDR_MAX;
                            record.mapProperties.insert(FPART_PROP_ORIGINALNAME, sOriginalName);
                            record.mapProperties.insert(FPART_PROP_HANDLEMETHOD, zipToCompressMethod(lfh.nMethod, lfh.nFlags));
                            record.mapProperties.insert(FPART_PROP_COMPRESSEDSIZE, lfh.nCompressedSize);
                            record.mapProperties.insert(FPART_PROP_UNCOMPRESSEDSIZE, lfh.nUncompressedSize);

                            const qint64 nLocalExtraFieldOffset = nOffset + sizeof(LOCALFILEHEADER) + lfh.nFileNameLength;
                            bool bHasUsableCRC = !(lfh.nFlags & 0x0008) &&
                                                 !((lfh.nMethod == CMETHOD_AES) && zipIsWinZipAesAe2(this, pPdStruct, nLocalExtraFieldOffset, lfh.nExtraFieldLength));
                            if (bHasUsableCRC) {
                                record.mapProperties.insert(FPART_PROP_RESULTCRC, lfh.nCRC32);
                                record.mapProperties.insert(FPART_PROP_CRC_TYPE, CRC_TYPE_FFFFFFFF_EDB88320_FFFFFFFFF);
                            }
                            record.mapProperties.insert(FPART_PROP_DATETIME, XBinary::dosDateTimeToQDateTime(lfh.nLastModDate, lfh.nLastModTime));

                            listResult.append(record);
                        }
                    }
                } else {
                    break;
                }

                nOffset += (sizeof(LOCALFILEHEADER) + lfh.nFileNameLength + lfh.nExtraFieldLength + lfh.nCompressedSize);
#ifdef QT_DEBUG
                qDebug("Offset: %llX", nOffset);
#endif
            }
        }

        nMaxOffset = nRealSize;
    }

    if ((nFileParts & FILEPART_DATA) && zipPartsCanAppend(nLimit, &listResult)) {
        FPART record = {};

        record.filePart = FILEPART_DATA;
        record.nFileOffset = 0;
        record.nFileSize = nMaxOffset;
        record.nVirtualAddress = XADDR_MAX;
        record.sName = tr("Data");

        listResult.append(record);
    }

    if ((nFileParts & FILEPART_OVERLAY) && zipPartsCanAppend(nLimit, &listResult)) {
        if (nMaxOffset < getSize()) {
            FPART record = {};

            record.filePart = FILEPART_OVERLAY;
            record.nFileOffset = nMaxOffset;
            record.nFileSize = nTotalSize - nMaxOffset;
            record.nVirtualAddress = XADDR_MAX;
            record.sName = tr("Overlay");

            listResult.append(record);
        }
    }

    return XBinary::isPdStructNotCanceled(pPdStruct) ? listResult : QList<FPART>();
}

static bool zipIsRecordNameMatch(bool bStartWith, const QString &sRecordName1, const QString &sRecordName2, const QString &sRecordName)
{
    if (bStartWith) {
        return sRecordName.startsWith(sRecordName1) || (!sRecordName2.isEmpty() && sRecordName.startsWith(sRecordName2));
    }
    return (sRecordName == sRecordName1) || (!sRecordName2.isEmpty() && (sRecordName == sRecordName2));
}

bool XZip::_isRecordNamePresent(qint64 nECDOffset, QString sRecordName1, QString sRecordName2, PDSTRUCT *pPdStruct, bool bStartWith, bool bRequireNonEmpty)
{
    qint32 nLimit = 10000;  // TODO
    qint64 nTotalSize = getSize();

    if (nECDOffset != -1) {
        qint32 nNumberOfRecords = read_uint16(nECDOffset + offsetof(ENDOFCENTRALDIRECTORYRECORD, nTotalNumberOfRecords));

        if (nLimit != -1) {
            nNumberOfRecords = qMin(nNumberOfRecords, nLimit);
        }

        qint64 nOffset = read_uint32(nECDOffset + offsetof(ENDOFCENTRALDIRECTORYRECORD, nOffsetToCentralDirectory));

        for (qint32 i = 0; i < (nNumberOfRecords) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
            CENTRALDIRECTORYFILEHEADER cdh = read_CENTRALDIRECTORYFILEHEADER(nOffset, pPdStruct);

            if (cdh.nSignature != SIGNATURE_CFD) {
                break;
            }

            const qint64 nFileNameOffset = nOffset + sizeof(CENTRALDIRECTORYFILEHEADER);
            const qint64 nExtraFieldOffset = nFileNameOffset + cdh.nFileNameLength;
            QString sRecordName;
            if (!_readFileName(nFileNameOffset, cdh.nFileNameLength, cdh.nFlags, nExtraFieldOffset, cdh.nExtraFieldLength, &sRecordName)) {
                break;
            }
            if (zipIsRecordNameMatch(bStartWith, sRecordName1, sRecordName2, sRecordName) && (!bRequireNonEmpty || (cdh.nUncompressedSize > 0))) {
                return true;
            }

            nOffset += (sizeof(CENTRALDIRECTORYFILEHEADER) + cdh.nFileNameLength + cdh.nExtraFieldLength + cdh.nFileCommentLength);
        }
    } else {
        // if no ECD, only the first record
        qint32 nNumberOfRecords = nLimit;

        if (nNumberOfRecords == -1) {
            nNumberOfRecords = 0xFFFFFF;
        }

        qint64 nOffset = 0;

        for (qint32 i = 0; i < (nNumberOfRecords) && isPdStructNotCanceled(pPdStruct); i++) {
            if ((nOffset + (qint64)sizeof(LOCALFILEHEADER)) > nTotalSize) {
                break;
            }

            LOCALFILEHEADER lfh = read_LOCALFILEHEADER(nOffset, pPdStruct);

            if (lfh.nSignature != SIGNATURE_LFD) {
                break;
            }

            const qint64 nFileNameOffset = nOffset + sizeof(LOCALFILEHEADER);
            const qint64 nExtraFieldOffset = nFileNameOffset + lfh.nFileNameLength;
            QString sRecordName;
            if (!_readFileName(nFileNameOffset, lfh.nFileNameLength, lfh.nFlags, nExtraFieldOffset, lfh.nExtraFieldLength, &sRecordName)) {
                break;
            }
            if (zipIsRecordNameMatch(bStartWith, sRecordName1, sRecordName2, sRecordName) && (!bRequireNonEmpty || (lfh.nUncompressedSize > 0))) {
                return true;
            }

            nOffset += sizeof(LOCALFILEHEADER) + lfh.nFileNameLength + lfh.nExtraFieldLength + lfh.nCompressedSize;
        }
    }

    return false;
}

qint32 XZip::_getNumberOfLocalFileHeaders(qint64 nOffset, qint64 nSize, qint64 *pnRealSize, PDSTRUCT *pPdStruct)
{
    QPointer<XZip> guardedArchive(this);
    QPointer<QIODevice> guardedSource(getDevice());
    qint32 nResult = 0;
    if (pnRealSize) *pnRealSize = 0;
    if (!guardedSource) return 0;
    const qint64 nDeviceSize = guardedSource->size();
    if (!guardedArchive || !guardedSource) return 0;

    if ((nOffset >= 0) && (nSize >= 0) && (nOffset <= nDeviceSize) && (nSize <= (nDeviceSize - nOffset))) {
        qint64 nCurrentOffset = nOffset;
        qint64 nEndOffset = nOffset + nSize;

        while (XBinary::isPdStructNotCanceled(pPdStruct)) {
            if ((nCurrentOffset < nOffset) || ((nEndOffset - nCurrentOffset) < (qint64)sizeof(LOCALFILEHEADER))) {
                break;
            }

            const QByteArray baHeader = XBinary::read_array_process(guardedSource.data(), nCurrentOffset, sizeof(LOCALFILEHEADER), pPdStruct);
            if (!guardedArchive || !guardedSource || (baHeader.size() != (qint64)sizeof(LOCALFILEHEADER))) {
                return 0;
            }
            LOCALFILEHEADER lfh = {};
            memcpy(&lfh, baHeader.constData(), sizeof(lfh));
            const quint32 nLocalSignature = lfh.nSignature;
            const quint32 nLocalFileNameSize = lfh.nFileNameLength;
            const quint32 nLocalExtraFieldSize = lfh.nExtraFieldLength;
            const quint32 nCompressedSize = lfh.nCompressedSize;

            if (nLocalSignature != SIGNATURE_LFD) {
                break;
            }

            qint64 nRecordSize = sizeof(LOCALFILEHEADER) + (qint64)nLocalFileNameSize + (qint64)nLocalExtraFieldSize + (qint64)nCompressedSize;
            if ((nRecordSize <= 0) || (nRecordSize > (nEndOffset - nCurrentOffset))) {
                break;
            }

            const quint16 nFlags = lfh.nFlags;
            // Without a central directory the real sizes of a descriptor-based
            // entry are not available from the local header.
            if (nFlags & 0x0008) break;
            const qint64 nFileNameOffset = nCurrentOffset + sizeof(LOCALFILEHEADER);
            const qint64 nExtraFieldOffset = nFileNameOffset + nLocalFileNameSize;
            QString sDecodedName;
            const bool bNameRead = guardedArchive->_readFileName(nFileNameOffset, nLocalFileNameSize, nFlags, nExtraFieldOffset, nLocalExtraFieldSize, &sDecodedName);
            if (!guardedArchive || !guardedSource) return 0;
            if (!bNameRead) {
                break;
            }

            nResult++;

            nCurrentOffset += nRecordSize;
        }

        if (pnRealSize) {
            *pnRealSize = nCurrentOffset - nOffset;
        }
    }

    return nResult;
}

bool XZip::_isECDSignaturePresent(qint64 nOffset, PDSTRUCT *pPdStruct)
{
    QPointer<XZip> guardedArchive(this);
    QPointer<QIODevice> guardedSource(getDevice());
    if (!guardedSource) return false;
    const qint64 nTotalSize = guardedSource->size();
    if (!guardedArchive || !guardedSource) return false;
    if ((nOffset < 0) || (nOffset >= nTotalSize)) return false;

    XBinary sourceBinary(guardedSource.data());
    const qint64 nFound = sourceBinary.find_uint32(nOffset, nTotalSize - nOffset, SIGNATURE_ECD, false, pPdStruct);
    return guardedArchive && guardedSource && (nFound != -1);
}

XArchive::HANDLE_METHOD XZip::zipToCompressMethod(quint16 nZipMethod, quint32 nFlags)
{
    HANDLE_METHOD result = HANDLE_METHOD_UNKNOWN;

    switch (nZipMethod) {
        case CMETHOD_STORE: result = HANDLE_METHOD_STORE; break;
        case CMETHOD_SHRINK: result = HANDLE_METHOD_SHRINK; break;
        case CMETHOD_REDUCED_1: result = HANDLE_METHOD_REDUCE_1; break;
        case CMETHOD_REDUCED_2: result = HANDLE_METHOD_REDUCE_2; break;
        case CMETHOD_REDUCED_3: result = HANDLE_METHOD_REDUCE_3; break;
        case CMETHOD_REDUCED_4: result = HANDLE_METHOD_REDUCE_4; break;

        case CMETHOD_IMPLODED: {
            bool b8kdict = (nFlags & 0x02) ? true : false;
            bool b3tree = (nFlags & 0x04) ? true : false;

            if (!b8kdict && !b3tree) {
                result = HANDLE_METHOD_IMPLODED_4KDICT_2TREES;
            } else if (!b8kdict && b3tree) {
                result = HANDLE_METHOD_IMPLODED_4KDICT_3TREES;
            } else if (b8kdict && !b3tree) {
                result = HANDLE_METHOD_IMPLODED_8KDICT_2TREES;
            } else if (b8kdict && b3tree) {
                result = HANDLE_METHOD_IMPLODED_8KDICT_3TREES;
            }
        } break;
        case CMETHOD_DEFLATE: result = HANDLE_METHOD_DEFLATE; break;
        case CMETHOD_DEFLATE64: result = HANDLE_METHOD_DEFLATE64; break;  // TODO
        case CMETHOD_BZIP2: result = HANDLE_METHOD_BZIP2; break;
        case CMETHOD_LZMA: result = HANDLE_METHOD_LZMA; break;
        case CMETHOD_ZSTD: result = HANDLE_METHOD_ZSTD; break;
        case CMETHOD_XZ: result = HANDLE_METHOD_XZ; break;
        case CMETHOD_JPEG: result = HANDLE_METHOD_WINZIP_JPEG; break;
        case CMETHOD_WAVPACK: result = HANDLE_METHOD_WAVPACK; break;
        case CMETHOD_PPMD: result = HANDLE_METHOD_PPMD8; break;
        case CMETHOD_AES: result = HANDLE_METHOD_ZIP_AES; break;
    }
    // TODO more methods

    return result;
}

bool XZip::initPack(PACK_STATE *pState, QIODevice *pDevice, const QMap<PACK_PROP, QVariant> &mapProperties, PDSTRUCT *pPdStruct)
{
    if (!pState || !pDevice || !pDevice->isWritable() || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    qint64 nStartOffset = pDevice->pos();
    if (pDevice->isSequential() && (nStartOffset < 0)) nStartOffset = 0;
    if (!zipCanAppendAt(pDevice, nStartOffset) || ((quint64)nStartOffset >= (std::numeric_limits<quint32>::max)())) return false;

    ZIP_PACK_CONTEXT *pNewContext = new (std::nothrow) ZIP_PACK_CONTEXT();
    if (!pNewContext) return false;
    pNewContext->pListZipFileRecords = new (std::nothrow) QList<ZIPFILE_RECORD>();
    if (!pNewContext->pListZipFileRecords) {
        delete pNewContext;
        return false;
    }

    ZIP_PACK_CONTEXT *pOldContext = static_cast<ZIP_PACK_CONTEXT *>(pState->pContext);
    if (pOldContext) {
        if (!zipIsPackStateConsistent(pState, pOldContext)) {
            zipDeletePackContext(pNewContext);
            return false;
        }

        QIODevice *pOldDevice = pOldContext->pDevice;
        if (!pOldDevice->isSequential()) {
            if (!zipRollbackWrite(pOldDevice, pOldContext->nStartOffset)) {
                zipDeletePackContext(pNewContext);
                return false;
            }
        } else if ((pOldDevice == pDevice) &&
                   (pOldContext->bFailed || (pOldContext->nCurrentOffset != pOldContext->nStartOffset) || (pOldContext->nNumberOfRecords != 0))) {
            zipDeletePackContext(pNewContext);
            return false;
        }
        zipDeletePackContext(pOldContext);
        *pState = PACK_STATE();
    }

    nStartOffset = pDevice->pos();
    if (pDevice->isSequential() && (nStartOffset < 0)) nStartOffset = 0;
    if (!zipCanAppendAt(pDevice, nStartOffset) || ((quint64)nStartOffset >= (std::numeric_limits<quint32>::max)())) {
        zipDeletePackContext(pNewContext);
        *pState = PACK_STATE();
        return false;
    }

    pNewContext->pDevice = pDevice;
    pNewContext->nStartOffset = nStartOffset;
    pNewContext->nCurrentOffset = nStartOffset;
    pNewContext->nNumberOfRecords = 0;
    pNewContext->bFailed = false;

    pState->pDevice = pDevice;
    pState->mapProperties = mapProperties;
    pState->nCurrentOffset = nStartOffset;
    pState->nNumberOfRecords = 0;
    pState->pContext = pNewContext;
    return true;
}

bool XZip::addFile(PACK_STATE *pState, const QString &sFilePath, PDSTRUCT *pPdStruct)
{
    if (!pState || !pState->pDevice || !pState->pDevice->isWritable() || !pState->pContext || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    ZIP_PACK_CONTEXT *pContext = static_cast<ZIP_PACK_CONTEXT *>(pState->pContext);
    QList<ZIPFILE_RECORD> *pListZipFileRecords = pContext->pListZipFileRecords;
    if (pContext->bFailed || !zipIsPackStateConsistent(pState, pContext) || (pListZipFileRecords->size() >= ((std::numeric_limits<quint16>::max)() - 1)) ||
        ((quint64)pContext->nCurrentOffset >= (std::numeric_limits<quint32>::max)())) {
        return false;
    }

    QFileInfo fileInfo(sFilePath);
    if (!fileInfo.exists() || !fileInfo.isFile() || !fileInfo.isReadable()) {
        return false;
    }
    const qint64 nFileSize = fileInfo.size();
    if ((nFileSize < 0) || ((quint64)nFileSize >= (std::numeric_limits<quint32>::max)())) return false;

    QString sStoredPath;
    PATH_MODE pathMode = (PATH_MODE)pState->mapProperties.value(PACK_PROP_PATHMODE, PATH_MODE_BASENAME).toInt();
    QString sBasePath = pState->mapProperties.value(PACK_PROP_BASEPATH).toString();
    if ((pathMode != XBinary::PATH_MODE_DEFAULT) && (pathMode != XBinary::PATH_MODE_RELATIVE) && (pathMode != XBinary::PATH_MODE_BASENAME) &&
        (pathMode != XBinary::PATH_MODE_ABSOLUTE)) {
        return false;
    }
    HANDLE_METHOD compressMethod = (HANDLE_METHOD)pState->mapProperties.value(PACK_PROP_COMPRESSMETHOD, HANDLE_METHOD_DEFLATE).toInt();
    CRYPTO_METHOD cryptoMethod = (CRYPTO_METHOD)pState->mapProperties.value(PACK_PROP_ENCRYPTIONMETHOD, CRYPTO_METHOD_NONE).toInt();
    QString sPassword = pState->mapProperties.value(PACK_PROP_PASSWORD).toString();
    qint32 nCompressionLevel = pState->mapProperties.value(PACK_PROP_COMPRESSIONLEVEL, -1).toInt();

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
        case XBinary::PATH_MODE_DEFAULT:
        case XBinary::PATH_MODE_BASENAME: sStoredPath = fileInfo.fileName(); break;
        default: return false;
    }

    sStoredPath = sStoredPath.replace("\\", "/");
    const QByteArray baFileName = sStoredPath.toUtf8();
    if (baFileName.isEmpty() || (baFileName.size() > (std::numeric_limits<quint16>::max)())) return false;

    CMETHOD cmethod = CMETHOD_STORE;
    if (compressMethod == XArchive::HANDLE_METHOD_DEFLATE) cmethod = CMETHOD_DEFLATE;
    else if (compressMethod != XArchive::HANDLE_METHOD_STORE) return false;
    if ((cryptoMethod != XBinary::CRYPTO_METHOD_NONE) && (cryptoMethod != XBinary::CRYPTO_METHOD_ZIPCRYPTO)) return false;
    if (!sPassword.isEmpty() && (cryptoMethod != XBinary::CRYPTO_METHOD_ZIPCRYPTO)) return false;

    ZIPFILE_RECORD zipFileRecord = {};
    zipFileRecord.sFileName = sStoredPath;
    zipFileRecord.nVersion = 0x14;
    zipFileRecord.nOS = 0;
    zipFileRecord.nMinVersion = 0x14;
    zipFileRecord.nMinOS = 0;
    zipFileRecord.nFlags = ZIP_FLAG_UTF8;
    zipFileRecord.method = cmethod;
    zipFileRecord.dtTime = fileInfo.lastModified();
    zipFileRecord.nUncompressedSize = nFileSize;

    if (nCompressionLevel == -1) {
        nCompressionLevel = 8;
    }
    if ((nCompressionLevel < 0) || (nCompressionLevel > 9)) return false;

    QFile::Permissions permissions = fileInfo.permissions();
    zipFileRecord.nExternalFileAttributes = filePermissionsToExternalAttributes(permissions);

    QFile file(sFilePath);
    if (!file.open(QIODevice::ReadOnly)) return false;
    QTemporaryFile stagedSource;
    qint64 nStagedSize = 0;
    if (!zipStageSource(&file, &stagedSource, nFileSize, pPdStruct, &nStagedSize, &zipFileRecord.nCRC32) || (nStagedSize != nFileSize)) {
        return false;
    }
    zipFileRecord.nUncompressedSize = nStagedSize;

    QPair<quint16, quint16> dosDateTime = XBinary::qDateTimeToDosDateTime(zipFileRecord.dtTime);
    quint16 nDosDate = dosDateTime.first;
    quint16 nDosTime = dosDateTime.second;

    bool bEncrypt = !sPassword.isEmpty() && (cryptoMethod == XBinary::CRYPTO_METHOD_ZIPCRYPTO);
    if (bEncrypt) zipFileRecord.nFlags |= 0x01;

    QTemporaryFile compressedPayload;
    QTemporaryFile encryptedPayload;
    QIODevice *pPayload = &stagedSource;
    qint64 nPayloadSize = nStagedSize;

    if (zipFileRecord.method == CMETHOD_DEFLATE) {
        if (!compressedPayload.open()) return false;
        XBinary::DATAPROCESS_STATE compressState = {};
        compressState.pDeviceInput = &stagedSource;
        compressState.pDeviceOutput = &compressedPayload;
        compressState.nInputOffset = 0;
        compressState.nInputLimit = nStagedSize;
        if (!XDeflateDecoder::compress(&compressState, pPdStruct, nCompressionLevel) || !compressedPayload.flush() || !XBinary::isPdStructNotCanceled(pPdStruct)) {
            return false;
        }
        nPayloadSize = compressedPayload.size();
        if ((nPayloadSize < 0) || !compressedPayload.seek(0)) return false;
        pPayload = &compressedPayload;
    }

    if (bEncrypt) {
        if (!encryptedPayload.open() || !pPayload->seek(0)) return false;
        XBinary::DATAPROCESS_STATE encryptState = {};
        encryptState.pDeviceInput = pPayload;
        encryptState.pDeviceOutput = &encryptedPayload;
        encryptState.nInputOffset = 0;
        encryptState.nInputLimit = nPayloadSize;
        if (!XZipCryptoDecoder::encrypt(&encryptState, sPassword, zipFileRecord.nCRC32, pPdStruct) || !encryptedPayload.flush() ||
            !XBinary::isPdStructNotCanceled(pPdStruct)) {
            return false;
        }
        nPayloadSize = encryptedPayload.size();
        if ((nPayloadSize < 0) || !encryptedPayload.seek(0)) return false;
        pPayload = &encryptedPayload;
    } else if (!pPayload->seek(0)) {
        return false;
    }

    if ((quint64)nPayloadSize >= (std::numeric_limits<quint32>::max)()) return false;
    zipFileRecord.nCompressedSize = nPayloadSize;
    zipFileRecord.nHeaderOffset = pState->nCurrentOffset;
    zipFileRecord.nDataOffset = pState->nCurrentOffset + (qint64)sizeof(LOCALFILEHEADER) + baFileName.size();

    const qint64 nRecordSize = (qint64)sizeof(LOCALFILEHEADER) + baFileName.size() + nPayloadSize;
    if ((nRecordSize <= 0) || ((quint64)nRecordSize >= ((quint64)(std::numeric_limits<quint32>::max)() - (quint64)pContext->nCurrentOffset))) return false;

    LOCALFILEHEADER localFileHeader = {};
    localFileHeader.nSignature = SIGNATURE_LFD;
    localFileHeader.nMinVersion = zipFileRecord.nMinVersion;
    localFileHeader.nMinOS = zipFileRecord.nMinOS;
    localFileHeader.nFlags = zipFileRecord.nFlags;
    localFileHeader.nMethod = zipFileRecord.method;
    localFileHeader.nLastModTime = nDosTime;
    localFileHeader.nLastModDate = nDosDate;
    localFileHeader.nCRC32 = zipFileRecord.nCRC32;
    localFileHeader.nCompressedSize = (quint32)zipFileRecord.nCompressedSize;
    localFileHeader.nUncompressedSize = (quint32)zipFileRecord.nUncompressedSize;
    localFileHeader.nFileNameLength = (quint16)baFileName.size();

    if (pListZipFileRecords->size() >= (std::numeric_limits<quint16>::max)()) {
        return false;
    }
    pListZipFileRecords->reserve(pListZipFileRecords->size() + 1);
    const qint64 nStartPosition = pContext->nCurrentOffset;
    qint64 nRecordWritten = 0;
    qint64 nPartWritten = 0;
    bool bWriteOK = zipWriteAll(pState->pDevice, reinterpret_cast<const char *>(&localFileHeader), sizeof(localFileHeader), pPdStruct, &nPartWritten);
    nRecordWritten += nPartWritten;
    if (bWriteOK) {
        bWriteOK = zipWriteAll(pState->pDevice, baFileName.constData(), baFileName.size(), pPdStruct, &nPartWritten);
        nRecordWritten += nPartWritten;
    }
    if (bWriteOK) {
        bWriteOK = zipCopyExactly(pPayload, pState->pDevice, nPayloadSize, pPdStruct, &nPartWritten);
        nRecordWritten += nPartWritten;
    }
    if (!bWriteOK || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        zipMarkPackWriteFailure(pState, pContext, nStartPosition, nRecordWritten);
        return false;
    }

    pListZipFileRecords->append(zipFileRecord);
    pContext->nCurrentOffset = nStartPosition + nRecordSize;
    pContext->nNumberOfRecords = pListZipFileRecords->size();
    pState->nCurrentOffset = pContext->nCurrentOffset;
    pState->nNumberOfRecords = pContext->nNumberOfRecords;
    return true;
}

static void zipRestoreBasePath(XBinary::PACK_STATE *pState, bool bRestoreBasePath, bool bHadBasePath, const QVariant &originalBasePath)
{
    if (!bRestoreBasePath) return;
    if (bHadBasePath) pState->mapProperties.insert(XBinary::PACK_PROP_BASEPATH, originalBasePath);
    else pState->mapProperties.remove(XBinary::PACK_PROP_BASEPATH);
}

bool XZip::addFolder(PACK_STATE *pState, const QString &sDirectoryPath, PDSTRUCT *pPdStruct)
{
    if (!pState || !pState->pDevice || !pState->pDevice->isWritable() || !pState->pContext || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    ZIP_PACK_CONTEXT *pContext = static_cast<ZIP_PACK_CONTEXT *>(pState->pContext);
    if (pContext->bFailed || !zipIsPackStateConsistent(pState, pContext)) return false;

    // Check if directory exists
    if (!XBinary::isDirectoryExists(sDirectoryPath)) {
        return false;
    }

    const bool bHadBasePath = pState->mapProperties.contains(PACK_PROP_BASEPATH);
    const QVariant originalBasePath = pState->mapProperties.value(PACK_PROP_BASEPATH);
    bool bRestoreBasePath = false;
    PATH_MODE pathMode = (PATH_MODE)pState->mapProperties.value(PACK_PROP_PATHMODE, PATH_MODE_BASENAME).toInt();
    QString sBasePath = pState->mapProperties.value(PACK_PROP_BASEPATH).toString();
    if ((pathMode != XBinary::PATH_MODE_DEFAULT) && (pathMode != XBinary::PATH_MODE_RELATIVE) && (pathMode != XBinary::PATH_MODE_BASENAME) &&
        (pathMode != XBinary::PATH_MODE_ABSOLUTE)) {
        return false;
    }

    if (pathMode == XBinary::PATH_MODE_RELATIVE && sBasePath.isEmpty()) {
        sBasePath = sDirectoryPath;
        pState->mapProperties.insert(PACK_PROP_BASEPATH, sBasePath);
        bRestoreBasePath = true;
    }

    QList<QString> listFiles;
    XBinary::findFiles(sDirectoryPath, &listFiles, true, 0, pPdStruct);
    if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
        zipRestoreBasePath(pState, bRestoreBasePath, bHadBasePath, originalBasePath);
        return false;
    }

    qint32 nNumberOfFiles = listFiles.count();

    for (qint32 i = 0; (i < nNumberOfFiles) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        QString sFilePath = listFiles.at(i);
        QFileInfo fileInfo(sFilePath);

        if (fileInfo.isDir()) {
            continue;
        }

        if (!addFile(pState, sFilePath, pPdStruct)) {
            zipRestoreBasePath(pState, bRestoreBasePath, bHadBasePath, originalBasePath);
            return false;
        }
    }

    zipRestoreBasePath(pState, bRestoreBasePath, bHadBasePath, originalBasePath);
    return XBinary::isPdStructNotCanceled(pPdStruct);
}

bool XZip::finishPack(PACK_STATE *pState, PDSTRUCT *pPdStruct)
{
    if (!pState || !pState->pDevice || !pState->pDevice->isWritable() || !pState->pContext || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    ZIP_PACK_CONTEXT *pContext = static_cast<ZIP_PACK_CONTEXT *>(pState->pContext);
    QList<ZIPFILE_RECORD> *pListZipFileRecords = pContext->pListZipFileRecords;
    if (pContext->bFailed || !zipIsPackStateConsistent(pState, pContext)) return false;

    qint64 nCentralDirectorySize = 0;
    qint64 nTotalSize = 0;
    const QByteArray baComment;
    if (!zipGetCentralDirectorySizes(pListZipFileRecords, baComment, pContext->nCurrentOffset, &nCentralDirectorySize, &nTotalSize)) {
        return false;
    }
    Q_UNUSED(nCentralDirectorySize)

    const qint64 nStartPosition = pContext->nCurrentOffset;
    qint64 nWritten = 0;
    qint64 nBuiltSize = 0;
    if (!zipAppendCentralDirectory(pState->pDevice, pListZipFileRecords, baComment, nStartPosition, pPdStruct, &nWritten, &nBuiltSize) || (nBuiltSize != nTotalSize) ||
        (nWritten != nTotalSize)) {
        zipMarkPackWriteFailure(pState, pContext, nStartPosition, nWritten);
        return false;
    }

    pContext->nCurrentOffset = nStartPosition + nTotalSize;
    pState->nCurrentOffset = pContext->nCurrentOffset;
    pState->nNumberOfRecords = pContext->nNumberOfRecords;
    zipDeletePackContext(pContext);
    pState->pContext = nullptr;
    return true;
}

QList<XBinary::PM_INFO> XZip::unpackImplemented()
{
    QList<XBinary::PM_INFO> listResult;

    static const HANDLE_METHOD g_zipUnpackMethods[] = {
        HANDLE_METHOD_STORE,
        HANDLE_METHOD_SHRINK,
        HANDLE_METHOD_REDUCE_1,
        HANDLE_METHOD_REDUCE_2,
        HANDLE_METHOD_REDUCE_3,
        HANDLE_METHOD_REDUCE_4,
        HANDLE_METHOD_IMPLODED_4KDICT_2TREES,
        HANDLE_METHOD_IMPLODED_4KDICT_3TREES,
        HANDLE_METHOD_IMPLODED_8KDICT_2TREES,
        HANDLE_METHOD_IMPLODED_8KDICT_3TREES,
        HANDLE_METHOD_DEFLATE,
        HANDLE_METHOD_DEFLATE64,
        HANDLE_METHOD_BZIP2,
        HANDLE_METHOD_LZMA,
        HANDLE_METHOD_ZSTD,
        HANDLE_METHOD_XZ,
        HANDLE_METHOD_PPMD8,
        HANDLE_METHOD_WINZIP_JPEG,
        HANDLE_METHOD_WAVPACK,
    };

    static const HANDLE_METHOD g_zipUnpackCryptoMethods[] = {
        HANDLE_METHOD_ZIPCRYPTO,
        HANDLE_METHOD_ZIP_AES128,
        HANDLE_METHOD_ZIP_AES192,
        HANDLE_METHOD_ZIP_AES256,
    };

    const qint32 nNumberOfMethods = sizeof(g_zipUnpackMethods) / sizeof(g_zipUnpackMethods[0]);
    const qint32 nNumberOfCryptoMethods = sizeof(g_zipUnpackCryptoMethods) / sizeof(g_zipUnpackCryptoMethods[0]);

    for (qint32 i = 0; i < nNumberOfMethods; i++) {
        listResult.append(createPMInfo(g_zipUnpackMethods[i]));

        for (qint32 j = 0; j < nNumberOfCryptoMethods; j++) {
            listResult.append(createPMInfo(g_zipUnpackMethods[i], g_zipUnpackCryptoMethods[j]));
        }
    }

    return listResult;
}

QList<XBinary::PM_INFO> XZip::packImplemented()
{
    QList<XBinary::PM_INFO> listResult;

    listResult.append(createPMInfo(HANDLE_METHOD_STORE));
    listResult.append(createPMInfo(HANDLE_METHOD_STORE, HANDLE_METHOD_ZIPCRYPTO));
    listResult.append(createPMInfo(HANDLE_METHOD_DEFLATE));
    listResult.append(createPMInfo(HANDLE_METHOD_DEFLATE, HANDLE_METHOD_ZIPCRYPTO));

    return listResult;
}

QMap<XBinary::UNPACK_PROP, QVariant> XZip::getDefaultUnpackProperties()
{
    QMap<XBinary::UNPACK_PROP, QVariant> result = XArchive::getDefaultUnpackProperties();

    if (isEncrypted()) {
        result.insert(XBinary::UNPACK_PROP_PASSWORD, QString());
    }

    return result;
}

bool XZip::initUnpack(UNPACK_STATE *pState, const QMap<UNPACK_PROP, QVariant> &mapProperties, PDSTRUCT *pPdStruct)
{
    QPointer<XZip> guardedArchive(this);
    if (m_bUnpackOperationInProgress) {
        return false;
    }
    UNPACK_OPERATION_GUARD operationGuard(&m_bUnpackOperationInProgress);
    if (!operationGuard.isAcquired()) return false;

    if (!pState) {
        return false;
    }

    if ((pState->pContext || !pState->baUnpackSourceToken.isEmpty()) && !guardedArchive->ownsUnpackSource(pState)) {
        return false;
    }
    ZIP_UNPACK_CONTEXT *pOldContext = static_cast<ZIP_UNPACK_CONTEXT *>(pState->pContext);
    guardedArchive->releaseUnpackSource(pState);
    pState->pContext = nullptr;
    delete pOldContext;
    if (!guardedArchive) {
        *pState = UNPACK_STATE();
        return false;
    }
    *pState = UNPACK_STATE();

    bool bResult = false;

    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    // Capture before parsing: caller-controlled QIODevice implementations can
    // re-enter and mutate their backing during any read.  Final validation
    // below proves the context was built from this same immutable snapshot.
    const bool bBound = guardedArchive->bindUnpackSource(pState, pPdStruct);
    if (!guardedArchive || !bBound) return false;

    pState->nTotalSize = guardedArchive->getSize();
    if (!guardedArchive) {
        *pState = UNPACK_STATE();
        return false;
    }

    // Try to get number of records from end of central directory
    qint64 nECDOffset = guardedArchive->findECDOffset(pPdStruct);
    if (!guardedArchive) {
        *pState = UNPACK_STATE();
        return false;
    }
    bool bIsECD = false;
    qint64 nCDFHOffset = 0;

    if (nECDOffset != -1) {
        nCDFHOffset = (qint64)guardedArchive->read_uint32(nECDOffset + offsetof(ENDOFCENTRALDIRECTORYRECORD, nOffsetToCentralDirectory));
        if (!guardedArchive) {
            *pState = UNPACK_STATE();
            return false;
        }

        const quint16 nNumberOfRecords = guardedArchive->read_uint16(nECDOffset + offsetof(ENDOFCENTRALDIRECTORYRECORD, nTotalNumberOfRecords));
        if (!guardedArchive) {
            *pState = UNPACK_STATE();
            return false;
        }

        // findECDOffset() has already authenticated an empty EOCD. There is no
        // central-file header to inspect when the archive contains no records.
        if (nNumberOfRecords == 0) {
            bIsECD = true;
        } else {
            const quint32 nSignature = guardedArchive->read_uint32(nCDFHOffset);
            if (!guardedArchive) {
                *pState = UNPACK_STATE();
                return false;
            }
            if (nSignature == SIGNATURE_CFD) {
                bIsECD = true;
            }
        }
    }

    if (bIsECD) {
        pState->nCurrentOffset = nCDFHOffset;
        pState->nNumberOfRecords = guardedArchive->read_uint16(nECDOffset + offsetof(ENDOFCENTRALDIRECTORYRECORD, nTotalNumberOfRecords));
        if (!guardedArchive) {
            *pState = UNPACK_STATE();
            return false;
        }
        // A structurally authenticated EOCD with zero records is a valid empty
        // ZIP archive. Keep an unpack context so wrappers such as ZIP SFX can
        // list and finish it successfully without inventing a member.
        bResult = true;
    } else if (nECDOffset == -1) {
        // Fallback: count complete local file records only when no authenticated
        // central directory is available. If an EOCD signature follows those
        // records, a central directory was present but failed authentication;
        // falling back would silently accept the damaged archive.
        qint64 nRealSize = 0;
        pState->nNumberOfRecords = guardedArchive->_getNumberOfLocalFileHeaders(0, pState->nTotalSize, &nRealSize, pPdStruct);
        if (!guardedArchive) {
            *pState = UNPACK_STATE();
            return false;
        }
        pState->nCurrentOffset = 0;
        const bool bECDSignaturePresent = guardedArchive->_isECDSignaturePresent(nRealSize, pPdStruct);
        if (!guardedArchive) {
            *pState = UNPACK_STATE();
            return false;
        }
        bResult = (pState->nNumberOfRecords > 0) && !bECDSignaturePresent;
    }

    if (bResult) {
        ZIP_UNPACK_CONTEXT *pContext = new (std::nothrow) ZIP_UNPACK_CONTEXT();
        if (!pContext) {
            guardedArchive->releaseUnpackSource(pState);
            *pState = UNPACK_STATE();
            return false;
        }
        pContext->bIsECD = bIsECD;
        pContext->nCentralDirectoryOffset = bIsECD ? nCDFHOffset : 0;
        pContext->nCentralDirectoryEnd = bIsECD ? nECDOffset : 0;
        pState->mapUnpackProperties = mapProperties;
        pState->pContext = pContext;
        if (!guardedArchive->validateAndFinalizeUnpackSource(pState, pContext, pPdStruct)) {
            if (!guardedArchive) return false;
            pState->pContext = nullptr;
            guardedArchive->releaseUnpackSource(pState);
            delete pContext;
            *pState = UNPACK_STATE();
            return false;
        }
    } else {
        guardedArchive->releaseUnpackSource(pState);
        *pState = UNPACK_STATE();
    }

    return bResult;
}

XBinary::ARCHIVERECORD XZip::infoCurrent(UNPACK_STATE *pState, PDSTRUCT *pPdStruct)
{
    UNPACK_OPERATION_GUARD operationGuard(&m_bUnpackOperationInProgress, &m_bNestedUnpackInfoAuthorized);
    if (!operationGuard.isAllowed()) return XBinary::ARCHIVERECORD();

    QPointer<XZip> guardedArchive(this);
    QPointer<QIODevice> guardedSource(getDevice());
    XBinary::ARCHIVERECORD result = {};

    bool bSourceCurrent = false;
    if (pState && pState->pContext && guardedSource && XBinary::isPdStructNotCanceled(pPdStruct)) {
        bSourceCurrent = guardedArchive->isUnpackSourceCurrent(pState, pPdStruct);
    }
    if (guardedArchive && guardedSource && bSourceCurrent && (pState->nCurrentIndex >= 0) && (pState->nCurrentIndex < pState->nNumberOfRecords)) {
        result.mapProperties = pState->mapArchiveProperties;  // Initialize with archive-level properties
        ZIP_UNPACK_CONTEXT *pContext = (ZIP_UNPACK_CONTEXT *)pState->pContext;
        bool bIsECD = pContext->bIsECD;
        qint64 nLocalHeaderOffset = 0;
        QString sFileName;

        quint8 nVersion = 0;
        quint8 nOS = 0;
        quint8 nMinVersion = 0;
        quint8 nMinOS = 0;
        quint16 nFlags = 0;
        quint16 nMethod = 0;
        quint16 nLastModTime = 0;
        quint16 nLastModDate = 0;
        quint32 nCRC32 = 0;
        quint32 nCompressedSize = 0;
        quint32 nUncompressedSize = 0;
        quint32 nExternalFileAttributes = 0;
        // Extra field and file comment information
        qint64 nExtraFieldOffset = 0;
        qint64 nExtraFieldLength = 0;
        qint64 nFileCommentOffset = 0;
        qint64 nFileCommentLength = 0;

        if (bIsECD) {
            if ((pState->nCurrentOffset < pContext->nCentralDirectoryOffset) ||
                ((pContext->nCentralDirectoryEnd - pState->nCurrentOffset) < (qint64)sizeof(CENTRALDIRECTORYFILEHEADER)) ||
                (guardedArchive->read_uint32(pState->nCurrentOffset) != SIGNATURE_CFD) || !guardedArchive || !guardedSource) {
                return XBinary::ARCHIVERECORD();
            }

            CENTRALDIRECTORYFILEHEADER cdfh = guardedArchive->read_CENTRALDIRECTORYFILEHEADER(pState->nCurrentOffset, pPdStruct);
            if (!guardedArchive || !guardedSource) return XBinary::ARCHIVERECORD();
            qint64 nCentralRecordSize =
                sizeof(CENTRALDIRECTORYFILEHEADER) + (qint64)cdfh.nFileNameLength + (qint64)cdfh.nExtraFieldLength + (qint64)cdfh.nFileCommentLength;
            if ((nCentralRecordSize > (pContext->nCentralDirectoryEnd - pState->nCurrentOffset)) ||
                ((qint64)cdfh.nOffsetToLocalFileHeader > (pContext->nCentralDirectoryOffset - (qint64)sizeof(LOCALFILEHEADER)))) {
                return XBinary::ARCHIVERECORD();
            }

            nVersion = cdfh.nVersion;
            nOS = cdfh.nOS;
            nMinVersion = cdfh.nMinVersion;
            nMinOS = cdfh.nMinOS;
            nFlags = cdfh.nFlags;
            nMethod = cdfh.nMethod;
            nLastModTime = cdfh.nLastModTime;
            nLastModDate = cdfh.nLastModDate;
            nCRC32 = cdfh.nCRC32;
            nCompressedSize = cdfh.nCompressedSize;
            nUncompressedSize = cdfh.nUncompressedSize;

            nLocalHeaderOffset = cdfh.nOffsetToLocalFileHeader;
            nExternalFileAttributes = cdfh.nExternalFileAttributes;

            nExtraFieldOffset = pState->nCurrentOffset + sizeof(CENTRALDIRECTORYFILEHEADER) + cdfh.nFileNameLength;
            nExtraFieldLength = cdfh.nExtraFieldLength;
            nFileCommentOffset = nExtraFieldOffset + nExtraFieldLength;
            nFileCommentLength = cdfh.nFileCommentLength;
            if (!guardedArchive->_readFileName(pState->nCurrentOffset + sizeof(CENTRALDIRECTORYFILEHEADER), cdfh.nFileNameLength, cdfh.nFlags, nExtraFieldOffset,
                                               nExtraFieldLength, &sFileName) ||
                !guardedArchive || !guardedSource) {
                return XBinary::ARCHIVERECORD();
            }
        } else {
            nLocalHeaderOffset = pState->nCurrentOffset;
        }

        LOCALFILEHEADER lfh = guardedArchive->read_LOCALFILEHEADER(nLocalHeaderOffset, pPdStruct);
        if (!guardedArchive || !guardedSource) return XBinary::ARCHIVERECORD();
        qint64 nLocalLimit = bIsECD ? pContext->nCentralDirectoryOffset : pState->nTotalSize;
        if ((nLocalHeaderOffset < 0) || ((nLocalLimit - nLocalHeaderOffset) < (qint64)sizeof(LOCALFILEHEADER)) || (lfh.nSignature != SIGNATURE_LFD)) {
            return XBinary::ARCHIVERECORD();
        }

        qint64 nLocalDataOffset = nLocalHeaderOffset + sizeof(LOCALFILEHEADER) + (qint64)lfh.nFileNameLength + (qint64)lfh.nExtraFieldLength;
        quint32 nValidatedCompressedSize = bIsECD ? nCompressedSize : lfh.nCompressedSize;
        if ((nLocalDataOffset > nLocalLimit) || ((qint64)nValidatedCompressedSize > (nLocalLimit - nLocalDataOffset))) {
            return XBinary::ARCHIVERECORD();
        }

        if (!bIsECD) {
            nMinVersion = lfh.nMinVersion;
            nMinOS = lfh.nMinOS;
            nCRC32 = lfh.nCRC32;
            nFlags = lfh.nFlags;
            nMethod = lfh.nMethod;
            nLastModTime = lfh.nLastModTime;
            nLastModDate = lfh.nLastModDate;
            nCompressedSize = lfh.nCompressedSize;
            nUncompressedSize = lfh.nUncompressedSize;

            nExtraFieldOffset = nLocalHeaderOffset + sizeof(LOCALFILEHEADER) + lfh.nFileNameLength;
            nExtraFieldLength = lfh.nExtraFieldLength;
            nFileCommentOffset = 0;
            nFileCommentLength = 0;
            if (!guardedArchive->_readFileName(nLocalHeaderOffset + sizeof(LOCALFILEHEADER), lfh.nFileNameLength, lfh.nFlags, nExtraFieldOffset, nExtraFieldLength,
                                               &sFileName) ||
                !guardedArchive || !guardedSource) {
                return XBinary::ARCHIVERECORD();
            }
        }

        bool bIsFolder = sFileName.endsWith(QLatin1Char('/'));

        // Some archivers store directory entries without a trailing '/', flagging
        // them only via the external file attributes (as 7-Zip/Info-ZIP do). The
        // attributes are only trustworthy when read from the central directory.
        if (!bIsFolder && bIsECD && (nUncompressedSize == 0) && (nCompressedSize == 0)) {
            bool bDosDirBit = (nExternalFileAttributes & 0x10) != 0;                 // FAT/NTFS FILE_ATTRIBUTE_DIRECTORY
            bool bUnixDir = (((nExternalFileAttributes >> 16) & 0xF000) == 0x4000);  // Unix S_IFDIR
            if (bDosDirBit || bUnixDir) {
                bIsFolder = true;
            }
        }

        result.mapProperties.insert(XBinary::FPART_PROP_ISFOLDER, bIsFolder);

        result.nStreamSize = nCompressedSize;
        result.nStreamOffset = nLocalDataOffset;

        result.mapProperties.insert(XBinary::FPART_PROP_ORIGINALNAME, sFileName);
        result.mapProperties.insert(XBinary::FPART_PROP_STREAMOFFSET, result.nStreamOffset);
        result.mapProperties.insert(XBinary::FPART_PROP_STREAMSIZE, result.nStreamSize);

        // Compression method
        HANDLE_METHOD compressMethod = zipToCompressMethod(nMethod, nFlags);
        result.mapProperties.insert(XBinary::FPART_PROP_HANDLEMETHOD, compressMethod);
        result.mapProperties.insert(XBinary::FPART_PROP_TYPE, (quint32)nMethod);  // Raw ZIP method number

        // A zero CRC is valid. In a local header with a data descriptor, though,
        // the field is only a placeholder and cannot be used for verification.
        if (bIsECD || !(nFlags & 0x0008)) {
            result.mapProperties.insert(XBinary::FPART_PROP_RESULTCRC, nCRC32);
            result.mapProperties.insert(XBinary::FPART_PROP_CRC_TYPE, XBinary::CRC_TYPE_FFFFFFFF_EDB88320_FFFFFFFFF);
        }

        // Sizes
        result.mapProperties.insert(XBinary::FPART_PROP_COMPRESSEDSIZE, nCompressedSize);
        result.mapProperties.insert(XBinary::FPART_PROP_UNCOMPRESSEDSIZE, nUncompressedSize);

        // Date/Time
        QDateTime dateTime = dosDateTimeToQDateTime(nLastModDate, nLastModTime);
        if (dateTime.isValid()) {
            result.mapProperties.insert(XBinary::FPART_PROP_DATETIME, dateTime);
        }

        result.mapProperties.insert(XBinary::FPART_PROP_FLAGS, nFlags);

        if (nFlags & 0x01) {
            result.mapProperties.insert(XBinary::FPART_PROP_ENCRYPTED, true);
            result.mapProperties.insert(XBinary::FPART_PROP_HANDLEMETHOD2, HANDLE_METHOD_ZIPCRYPTO);
        }

        if (nMethod == CMETHOD_AES) {
            result.mapProperties.insert(XBinary::FPART_PROP_ENCRYPTED, true);
            result.mapProperties.insert(XBinary::FPART_PROP_HANDLEMETHOD, HANDLE_METHOD_STORE);
            result.mapProperties.insert(XBinary::FPART_PROP_HANDLEMETHOD2, HANDLE_METHOD_ZIP_AES);
        }

        result.mapProperties.insert(XBinary::FPART_PROP_EXTRAFIELDOFFSET, nExtraFieldOffset);
        result.mapProperties.insert(XBinary::FPART_PROP_EXTRAFIELDLENGTH, nExtraFieldLength);
        result.mapProperties.insert(XBinary::FPART_PROP_FILECOMMENTOFFSET, nFileCommentOffset);
        result.mapProperties.insert(XBinary::FPART_PROP_FILECOMMENTLENGTH, nFileCommentLength);

        nExtraFieldLength = qMin(nExtraFieldLength, (qint64)0xFFFF);
        const QByteArray baExtraField = guardedArchive->read_array(nExtraFieldOffset, nExtraFieldLength);
        if (!guardedArchive || !guardedSource || (baExtraField.size() != nExtraFieldLength)) {
            return XBinary::ARCHIVERECORD();
        }
        char *pExtra = const_cast<char *>(baExtraField.constData());
        // Parse the immutable snapshot: no member access remains across source
        // callbacks in this variable-length loop.
        for (qint32 i = 0; ((i + 4) <= nExtraFieldLength) && isPdStructNotCanceled(pPdStruct);) {
            quint16 nHeaderID = XBinary::_read_uint16(pExtra + i);
            quint16 nDataSize = XBinary::_read_uint16(pExtra + i + 2);
            if ((qint64)nDataSize > (nExtraFieldLength - i - 4)) break;

            if (nHeaderID == ZIP_AES_EXTRA_FIELD_HEADER_ID) {
                if ((nMethod == CMETHOD_AES) && (nDataSize >= ZIP_AES_EXTRA_FIELD_DATA_SIZE)) {
                    AES_EXTRA_FIELD aesExtraField = {};
                    aesExtraField.nHeaderID = XBinary::_read_uint16(pExtra + i);
                    aesExtraField.nDataSize = XBinary::_read_uint16(pExtra + i + 2);
                    aesExtraField.nAESVersion = XBinary::_read_uint16(pExtra + i + 4);
                    aesExtraField.nVendorID = XBinary::_read_uint16(pExtra + i + 6);
                    aesExtraField.nEncryptionMode = XBinary::_read_uint8(pExtra + i + 8);
                    aesExtraField.nCompressionMethod = XBinary::_read_uint16(pExtra + i + 9);

                    // WinZip AES AE-2 authenticates the payload but deliberately
                    // does not store a usable plaintext CRC32.
                    if ((aesExtraField.nAESVersion == 2) && (aesExtraField.nVendorID == ZIP_AES_VENDOR_ID_AE)) {
                        result.mapProperties.remove(XBinary::FPART_PROP_CRC_TYPE);
                        result.mapProperties.remove(XBinary::FPART_PROP_RESULTCRC);
                    }

                    // Original compression method
                    HANDLE_METHOD aesCompressMethod = zipToCompressMethod(aesExtraField.nCompressionMethod, nFlags);
                    result.mapProperties.insert(XBinary::FPART_PROP_HANDLEMETHOD, aesCompressMethod);

                    if (aesExtraField.nEncryptionMode == 1) {
                        result.mapProperties.insert(XBinary::FPART_PROP_HANDLEMETHOD2, HANDLE_METHOD_ZIP_AES128);
                    } else if (aesExtraField.nEncryptionMode == 2) {
                        result.mapProperties.insert(XBinary::FPART_PROP_HANDLEMETHOD2, HANDLE_METHOD_ZIP_AES192);
                    } else if (aesExtraField.nEncryptionMode == 3) {
                        result.mapProperties.insert(XBinary::FPART_PROP_HANDLEMETHOD2, HANDLE_METHOD_ZIP_AES256);
                    }
                }
            } else if (nHeaderID == 0x000A) {
                // NTFS extra field: 4-byte reserved + TLV attribute entries
                // Contains high-resolution MTIME, ATIME, CTIME as Windows FILETIMEs
                if (nDataSize >= (4 + 4 + 24)) {  // reserved(4) + tag(2)+size(2) + 3×FILETIME(24)
                    qint64 nBase = i + 4 + 4;     // skip headerID(2)+dataSize(2)+reserved(4)
                    qint64 nBlockEnd = i + 4 + nDataSize;
                    while (nBase + 4 <= nBlockEnd) {
                        quint16 nAttrTag = XBinary::_read_uint16(pExtra + nBase);
                        quint16 nAttrSize = XBinary::_read_uint16(pExtra + nBase + 2);
                        if (nAttrTag == 0x0001 && nAttrSize >= 24 && nBase + 4 + 24 <= nBlockEnd) {
                            quint64 nMTime = XBinary::_read_uint64(pExtra + nBase + 4);
                            quint64 nATime = XBinary::_read_uint64(pExtra + nBase + 12);
                            quint64 nCTime = XBinary::_read_uint64(pExtra + nBase + 20);
                            QDateTime dt;
                            dt = XBinary::winFileTimeToQDateTime(nMTime);
                            if (dt.isValid()) result.mapProperties.insert(XBinary::FPART_PROP_MTIME, dt);
                            dt = XBinary::winFileTimeToQDateTime(nATime);
                            if (dt.isValid()) result.mapProperties.insert(XBinary::FPART_PROP_ATIME, dt);
                            dt = XBinary::winFileTimeToQDateTime(nCTime);
                            if (dt.isValid()) result.mapProperties.insert(XBinary::FPART_PROP_CTIME, dt);
                        }
                        nBase += 4 + nAttrSize;
                    }
                }
            } else if (nHeaderID == 0x5455) {
                // Extended Unix Timestamp extra field
                // Flags byte: bit0=MTIME, bit1=ATIME, bit2=CTIME
                if (nDataSize >= 1) {
                    quint8 nTsFlags = XBinary::_read_uint8(pExtra + i + 4);
                    qint64 nTsOff = i + 4 + 1;  // first timestamp byte
                    qint64 nTsEnd = i + 4 + nDataSize;
                    if ((nTsFlags & 0x01) && (nTsOff + 4 <= nTsEnd)) {
                        quint32 nUnixMTime = XBinary::_read_uint32(pExtra + nTsOff);
                        QDateTime dt = XBinary::valueToTime((qint64)nUnixMTime, XBinary::DT_TYPE_UNIXTIME);
                        if (dt.isValid() && !result.mapProperties.contains(XBinary::FPART_PROP_MTIME)) {
                            result.mapProperties.insert(XBinary::FPART_PROP_MTIME, dt);
                        }
                        nTsOff += 4;
                    }
                    if ((nTsFlags & 0x02) && (nTsOff + 4 <= nTsEnd)) {
                        quint32 nUnixATime = XBinary::_read_uint32(pExtra + nTsOff);
                        QDateTime dt = XBinary::valueToTime((qint64)nUnixATime, XBinary::DT_TYPE_UNIXTIME);
                        if (dt.isValid() && !result.mapProperties.contains(XBinary::FPART_PROP_ATIME)) {
                            result.mapProperties.insert(XBinary::FPART_PROP_ATIME, dt);
                        }
                        nTsOff += 4;
                    }
                    if ((nTsFlags & 0x04) && (nTsOff + 4 <= nTsEnd)) {
                        quint32 nUnixCTime = XBinary::_read_uint32(pExtra + nTsOff);
                        QDateTime dt = XBinary::valueToTime((qint64)nUnixCTime, XBinary::DT_TYPE_UNIXTIME);
                        if (dt.isValid() && !result.mapProperties.contains(XBinary::FPART_PROP_CTIME)) {
                            result.mapProperties.insert(XBinary::FPART_PROP_CTIME, dt);
                        }
                    }
                }
            }

            i += (4 + nDataSize);
        }

        // Windows/DOS external file attributes (available only from Central Directory)
        // nOS == 0: MS-DOS/PKZIP  nOS == 10: Windows NTFS/VFAT
        if (bIsECD && (nOS == 0 || nOS == 10)) {
            quint16 nWinAttrib = (quint16)(nExternalFileAttributes & 0xFFFF);
            result.mapProperties.insert(XBinary::FPART_PROP_ISREADONLY, (nWinAttrib & 0x01) != 0);
            result.mapProperties.insert(XBinary::FPART_PROP_ISHIDDEN, (nWinAttrib & 0x02) != 0);
            result.mapProperties.insert(XBinary::FPART_PROP_ISSYSTEM, (nWinAttrib & 0x04) != 0);
            result.mapProperties.insert(XBinary::FPART_PROP_ISARCHIVE, (nWinAttrib & 0x20) != 0);
        }
    }

    if (!guardedArchive || !guardedSource || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return XBinary::ARCHIVERECORD();
    }
    const bool bStillCurrent = guardedArchive->isUnpackSourceCurrent(pState, pPdStruct);
    return (bStillCurrent && guardedArchive && guardedSource) ? result : XBinary::ARCHIVERECORD();
}

bool XZip::moveToNext(UNPACK_STATE *pState, PDSTRUCT *pPdStruct)
{
    UNPACK_OPERATION_GUARD operationGuard(&m_bUnpackOperationInProgress);
    if (!operationGuard.isAcquired()) return false;

    QPointer<XZip> guardedArchive(this);
    QPointer<QIODevice> guardedSource(getDevice());
    bool bResult = false;

    bool bSourceCurrent = false;
    if (pState && pState->pContext && guardedSource && XBinary::isPdStructNotCanceled(pPdStruct)) {
        bSourceCurrent = guardedArchive->isUnpackSourceCurrent(pState, pPdStruct);
    }
    if (guardedArchive && guardedSource && bSourceCurrent && (pState->nCurrentIndex >= 0) && (pState->nCurrentIndex < pState->nNumberOfRecords)) {
        ZIP_UNPACK_CONTEXT *pContext = (ZIP_UNPACK_CONTEXT *)pState->pContext;
        bool bIsECD = pContext->bIsECD;

        if (bIsECD) {
            if ((pState->nCurrentOffset < pContext->nCentralDirectoryOffset) ||
                ((pContext->nCentralDirectoryEnd - pState->nCurrentOffset) < (qint64)sizeof(CENTRALDIRECTORYFILEHEADER)) ||
                (guardedArchive->read_uint32(pState->nCurrentOffset) != SIGNATURE_CFD) || !guardedArchive || !guardedSource) {
                return false;
            }
            CENTRALDIRECTORYFILEHEADER cdfh = guardedArchive->read_CENTRALDIRECTORYFILEHEADER(pState->nCurrentOffset, pPdStruct);
            if (!guardedArchive || !guardedSource) return false;
            qint64 nRecordSize = sizeof(CENTRALDIRECTORYFILEHEADER) + (qint64)cdfh.nFileNameLength + (qint64)cdfh.nExtraFieldLength + (qint64)cdfh.nFileCommentLength;
            if (nRecordSize > (pContext->nCentralDirectoryEnd - pState->nCurrentOffset)) return false;
            pState->nCurrentOffset += nRecordSize;
        } else {
            if ((pState->nCurrentOffset < 0) || ((pState->nTotalSize - pState->nCurrentOffset) < (qint64)sizeof(LOCALFILEHEADER)) ||
                (guardedArchive->read_uint32(pState->nCurrentOffset) != SIGNATURE_LFD) || !guardedArchive || !guardedSource) {
                return false;
            }
            LOCALFILEHEADER lfh = guardedArchive->read_LOCALFILEHEADER(pState->nCurrentOffset, pPdStruct);
            if (!guardedArchive || !guardedSource) return false;
            qint64 nRecordSize = sizeof(LOCALFILEHEADER) + (qint64)lfh.nFileNameLength + (qint64)lfh.nExtraFieldLength + (qint64)lfh.nCompressedSize;
            if (nRecordSize > (pState->nTotalSize - pState->nCurrentOffset)) return false;
            pState->nCurrentOffset += nRecordSize;
        }

        pState->nCurrentIndex++;

        bResult = (pState->nCurrentIndex < pState->nNumberOfRecords);
    }

    return bResult;
}

bool XZip::finishUnpack(UNPACK_STATE *pState, PDSTRUCT *pPdStruct)
{
    UNPACK_OPERATION_GUARD operationGuard(&m_bUnpackOperationInProgress);
    if (!operationGuard.isAcquired()) return false;

    Q_UNUSED(pPdStruct)

    QPointer<XZip> guardedArchive(this);
    if (!pState) {
        return false;
    }

    if ((pState->pContext || !pState->baUnpackSourceToken.isEmpty()) && !guardedArchive->ownsUnpackSource(pState)) return false;
    ZIP_UNPACK_CONTEXT *pZipUnpackContext = static_cast<ZIP_UNPACK_CONTEXT *>(pState->pContext);
    guardedArchive->releaseUnpackSource(pState);
    pState->pContext = nullptr;
    delete pZipUnpackContext;
    if (!guardedArchive) return false;

    pState->nCurrentOffset = 0;
    pState->nTotalSize = 0;
    pState->nCurrentIndex = 0;
    pState->nNumberOfRecords = 0;
    pState->mapUnpackProperties.clear();
    pState->mapArchiveProperties.clear();

    return true;
}

QList<XBinary::FPART_PROP> XZip::getAvailableFPARTProperties()
{
    QList<XBinary::FPART_PROP> listResult;

    listResult.append(FPART_PROP_ORIGINALNAME);
    listResult.append(FPART_PROP_COMPRESSEDSIZE);
    listResult.append(FPART_PROP_UNCOMPRESSEDSIZE);
    listResult.append(FPART_PROP_HANDLEMETHOD);
    listResult.append(FPART_PROP_STREAMOFFSET);
    listResult.append(FPART_PROP_STREAMSIZE);

    return listResult;
}

QVariant XZip::calculateHash(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    if (!pDevice || !pDevice->isReadable() || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return 0;
    }

    QTemporaryFile stagedSource;
    qint64 nSize = 0;
    quint32 nCRC32 = 0;
    const qint64 nExpectedSize = pDevice->isSequential() ? -1 : pDevice->size();
    if (!zipStageSource(pDevice, &stagedSource, nExpectedSize, pPdStruct, &nSize, &nCRC32)) return 0;
    Q_UNUSED(nSize)
    return nCRC32;
}

quint32 XZip::filePermissionsToExternalAttributes(QFile::Permissions permissions)
{
    quint32 nResult = 0;

    // Unix file permissions format (stored in high 16 bits)
    // Format: 0xFFFF0000 where the high word contains Unix mode
    quint16 nUnixMode = 0;

    // File type (regular file)
    nUnixMode |= 0x8000;  // S_IFREG

    // Owner permissions
    if (permissions & QFile::ReadOwner) nUnixMode |= 0x0100;   // S_IRUSR
    if (permissions & QFile::WriteOwner) nUnixMode |= 0x0080;  // S_IWUSR
    if (permissions & QFile::ExeOwner) nUnixMode |= 0x0040;    // S_IXUSR

    // Group permissions
    if (permissions & QFile::ReadGroup) nUnixMode |= 0x0020;   // S_IRGRP
    if (permissions & QFile::WriteGroup) nUnixMode |= 0x0010;  // S_IWGRP
    if (permissions & QFile::ExeGroup) nUnixMode |= 0x0008;    // S_IXGRP

    // Other permissions
    if (permissions & QFile::ReadOther) nUnixMode |= 0x0004;   // S_IROTH
    if (permissions & QFile::WriteOther) nUnixMode |= 0x0002;  // S_IWOTH
    if (permissions & QFile::ExeOther) nUnixMode |= 0x0001;    // S_IXOTH

    // Store Unix mode in high 16 bits
    nResult = ((quint32)nUnixMode) << 16;

    // Low 16 bits: DOS attributes (optional)
    // Bit 0: Read-only
    // Bit 1: Hidden
    // Bit 2: System
    // Bit 5: Archive
    if (!(permissions & QFile::WriteOwner)) {
        nResult |= 0x01;  // Read-only
    }
    nResult |= 0x20;  // Archive bit

    return nResult;
}

QFile::Permissions XZip::externalAttributesToFilePermissions(quint32 nExternalAttributes)
{
    QFile::Permissions permissions = QFile::Permissions();

    // Extract Unix mode from high 16 bits
    quint16 nUnixMode = (quint16)((nExternalAttributes >> 16) & 0xFFFF);

    if (nUnixMode != 0) {
        // Owner permissions
        if (nUnixMode & 0x0100) permissions |= QFile::ReadOwner;   // S_IRUSR
        if (nUnixMode & 0x0080) permissions |= QFile::WriteOwner;  // S_IWUSR
        if (nUnixMode & 0x0040) permissions |= QFile::ExeOwner;    // S_IXUSR

        // Group permissions
        if (nUnixMode & 0x0020) permissions |= QFile::ReadGroup;   // S_IRGRP
        if (nUnixMode & 0x0010) permissions |= QFile::WriteGroup;  // S_IWGRP
        if (nUnixMode & 0x0008) permissions |= QFile::ExeGroup;    // S_IXGRP

        // Other permissions
        if (nUnixMode & 0x0004) permissions |= QFile::ReadOther;   // S_IROTH
        if (nUnixMode & 0x0002) permissions |= QFile::WriteOther;  // S_IWOTH
        if (nUnixMode & 0x0001) permissions |= QFile::ExeOther;    // S_IXOTH
    } else {
        // Fallback: use DOS attributes from low 16 bits
        bool bReadOnly = (nExternalAttributes & 0x01) != 0;

        if (bReadOnly) {
            permissions = QFile::ReadOwner | QFile::ReadUser | QFile::ReadGroup | QFile::ReadOther;
        } else {
            permissions = QFile::ReadOwner | QFile::ReadUser | QFile::ReadGroup | QFile::ReadOther | QFile::WriteOwner | QFile::WriteUser;
        }
    }

    return permissions;
}

QList<XBinary::XFHEADER> XZip::getXFHeaders(const XFSTRUCT &xfStruct, PDSTRUCT *pPdStruct)
{
    QList<XBinary::XFHEADER> listResult;

    quint32 nStructID = xfStruct.nStructID;

    if (nStructID == 0) {
        // Root: find ECD and return it as the main header
        qint64 nECDOffset = findECDOffset(pPdStruct);

        XFSTRUCT _xfStruct = xfStruct;
        _xfStruct.nStructID = STRUCTID_ENDOFCENTRALDIRECTORYRECORD;
        _xfStruct.xLoc = offsetToLoc(nECDOffset);

        listResult.append(getXFHeaders(_xfStruct, pPdStruct));
    } else if (nStructID == STRUCTID_ENDOFCENTRALDIRECTORYRECORD) {
        XLOC ecdLoc = xfStruct.xLoc;
        qint64 nECDOffset = locToOffset(xfStruct.pMemoryMap, ecdLoc);

        XFHEADER xfHeader = {};
        xfHeader.fileType = xfStruct.fileType;
        xfHeader.structID = static_cast<XBinary::STRUCTID>(STRUCTID_ENDOFCENTRALDIRECTORYRECORD);
        xfHeader.xLoc = ecdLoc;
        xfHeader.xfType = XFTYPE_HEADER;
        xfHeader.listFields = getXFRecords(xfStruct.fileType, STRUCTID_ENDOFCENTRALDIRECTORYRECORD, ecdLoc);
        xfHeader.listDataSt.append({0, 0, XFDATASTYPE_LIST, _TABLE_XZip_HeaderSignatures, sizeof(_TABLE_XZip_HeaderSignatures) / sizeof(XBinary::XIDSTRING)});
        xfHeader.sTag = xfHeaderToTag(xfHeader, structIDToString(STRUCTID_ENDOFCENTRALDIRECTORYRECORD), xfHeader.sParentTag);

        listResult.append(xfHeader);

        if (xfStruct.bIsParent) {
            quint16 nTotalNumberOfRecords = read_uint16(nECDOffset + offsetof(ENDOFCENTRALDIRECTORYRECORD, nTotalNumberOfRecords));
            quint32 nOffsetToCentralDirectory = read_uint32(nECDOffset + offsetof(ENDOFCENTRALDIRECTORYRECORD, nOffsetToCentralDirectory));

            XFSTRUCT _xfStruct = xfStruct;
            _xfStruct.sParent = xfHeader.sTag;
            _xfStruct.nStructID = STRUCTID_CENTRALDIRECTORYFILEHEADER;
            _xfStruct.xLoc = offsetToLoc(nOffsetToCentralDirectory);
            _xfStruct.nCount = nTotalNumberOfRecords;

            listResult.append(getXFHeaders(_xfStruct, pPdStruct));
        }
    } else if (nStructID == STRUCTID_CENTRALDIRECTORYFILEHEADER) {
        qint64 nOffsetToCentralDirectory = locToOffset(xfStruct.pMemoryMap, xfStruct.xLoc);
        qint32 nTotalNumberOfRecords = xfStruct.nCount;

        XLOC cdhLoc = xfStruct.xLoc;

        XFHEADER xfHeaderCDH = {};
        xfHeaderCDH.sParentTag = xfStruct.sParent;
        xfHeaderCDH.fileType = xfStruct.fileType;
        xfHeaderCDH.structID = static_cast<XBinary::STRUCTID>(STRUCTID_CENTRALDIRECTORYFILEHEADER);
        xfHeaderCDH.xLoc = cdhLoc;
        xfHeaderCDH.xfType = XFTYPE_TABLE;
        // xfHeaderCDH.listFields not fixed
        // CDH field indices: 0=Signature, 2=OS, 4=MinOS, 5=Flags, 6=Method
        xfHeaderCDH.listDataSt.append({0, 0, XFDATASTYPE_LIST, _TABLE_XZip_HeaderSignatures, sizeof(_TABLE_XZip_HeaderSignatures) / sizeof(XBinary::XIDSTRING)});
        xfHeaderCDH.listDataSt.append({2, 0, XFDATASTYPE_LIST, _TABLE_XZip_OS, sizeof(_TABLE_XZip_OS) / sizeof(XBinary::XIDSTRING)});
        xfHeaderCDH.listDataSt.append({4, 0, XFDATASTYPE_LIST, _TABLE_XZip_OS, sizeof(_TABLE_XZip_OS) / sizeof(XBinary::XIDSTRING)});
        xfHeaderCDH.listDataSt.append({5, 0xFFFF, XFDATASTYPE_FLAGS, _TABLE_XZip_FLAGS, sizeof(_TABLE_XZip_FLAGS) / sizeof(XBinary::XIDSTRING)});
        xfHeaderCDH.listDataSt.append({6, 0, XFDATASTYPE_LIST, _TABLE_XZip_CMETHOD, sizeof(_TABLE_XZip_CMETHOD) / sizeof(XBinary::XIDSTRING)});

        XFHEADER xfHeaderLFH = {};
        if (xfStruct.bIsParent) {
            xfHeaderLFH.fileType = xfStruct.fileType;
            xfHeaderLFH.structID = static_cast<XBinary::STRUCTID>(STRUCTID_LOCALFILEHEADER);
            xfHeaderLFH.xLoc = cdhLoc;
            xfHeaderLFH.xfType = XFTYPE_TABLE;
            xfHeaderLFH.bIsParentNeeded = true;  // Important!
            // xfHeaderLFH.listFields not fixed
            // LFH field indices: 0=Signature, 2=MinOS, 3=Flags, 4=Method
            xfHeaderLFH.listDataSt.append({0, 0, XFDATASTYPE_LIST, _TABLE_XZip_HeaderSignatures, sizeof(_TABLE_XZip_HeaderSignatures) / sizeof(XBinary::XIDSTRING)});
            xfHeaderLFH.listDataSt.append({2, 0, XFDATASTYPE_LIST, _TABLE_XZip_OS, sizeof(_TABLE_XZip_OS) / sizeof(XBinary::XIDSTRING)});
            xfHeaderLFH.listDataSt.append({3, 0xFFFF, XFDATASTYPE_FLAGS, _TABLE_XZip_FLAGS, sizeof(_TABLE_XZip_FLAGS) / sizeof(XBinary::XIDSTRING)});
            xfHeaderLFH.listDataSt.append({4, 0, XFDATASTYPE_LIST, _TABLE_XZip_CMETHOD, sizeof(_TABLE_XZip_CMETHOD) / sizeof(XBinary::XIDSTRING)});
        }

        // Enumerate locations for each CDH entry
        qint64 nCurrentOffset = nOffsetToCentralDirectory;
        qint64 nFileSize = getSize();

        for (qint32 i = 0; i < nTotalNumberOfRecords; i++) {
            if ((nCurrentOffset + (qint64)sizeof(CENTRALDIRECTORYFILEHEADER)) > nFileSize) {
                break;
            }

            xfHeaderCDH.listRowLocations.append(nCurrentOffset);

            CENTRALDIRECTORYFILEHEADER cdh = read_CENTRALDIRECTORYFILEHEADER(nCurrentOffset, pPdStruct);

            if (xfStruct.bIsParent) {
                xfHeaderLFH.listRowLocations.append(cdh.nOffsetToLocalFileHeader);
            }

            nCurrentOffset += sizeof(CENTRALDIRECTORYFILEHEADER) + cdh.nFileNameLength + cdh.nExtraFieldLength + cdh.nFileCommentLength;
        }

        xfHeaderCDH.sTag = xfHeaderToTag(xfHeaderCDH, structIDToString(STRUCTID_CENTRALDIRECTORYFILEHEADER), xfHeaderCDH.sParentTag);

        listResult.append(xfHeaderCDH);

        if (xfStruct.bIsParent) {
            xfHeaderLFH.sParentTag = xfHeaderCDH.sTag;
            xfHeaderLFH.sTag = xfHeaderToTag(xfHeaderLFH, structIDToString(STRUCTID_LOCALFILEHEADER), xfHeaderLFH.sParentTag);
            listResult.append(xfHeaderLFH);
        }
    } else if (nStructID == STRUCTID_LOCALFILEHEADER) {
        // Direct lookup: xLoc = CDH start, nCount = number of entries
        qint64 nCdhOffset = locToOffset(xfStruct.pMemoryMap, xfStruct.xLoc);
        qint32 nCount = xfStruct.nCount;
        qint64 nFileSize = getSize();

        if ((nCdhOffset != -1) && (nCount > 0)) {
            XFHEADER xfHeaderLFH = {};
            xfHeaderLFH.sParentTag = xfStruct.sParent;
            xfHeaderLFH.fileType = xfStruct.fileType;
            xfHeaderLFH.structID = static_cast<XBinary::STRUCTID>(STRUCTID_LOCALFILEHEADER);
            xfHeaderLFH.xLoc = xfStruct.xLoc;
            xfHeaderLFH.xfType = XFTYPE_TABLE;
            xfHeaderLFH.listDataSt.append({0, 0, XFDATASTYPE_LIST, _TABLE_XZip_HeaderSignatures, sizeof(_TABLE_XZip_HeaderSignatures) / sizeof(XBinary::XIDSTRING)});
            xfHeaderLFH.listDataSt.append({2, 0, XFDATASTYPE_LIST, _TABLE_XZip_OS, sizeof(_TABLE_XZip_OS) / sizeof(XBinary::XIDSTRING)});
            xfHeaderLFH.listDataSt.append({3, 0xFFFF, XFDATASTYPE_FLAGS, _TABLE_XZip_FLAGS, sizeof(_TABLE_XZip_FLAGS) / sizeof(XBinary::XIDSTRING)});
            xfHeaderLFH.listDataSt.append({4, 0, XFDATASTYPE_LIST, _TABLE_XZip_CMETHOD, sizeof(_TABLE_XZip_CMETHOD) / sizeof(XBinary::XIDSTRING)});

            qint64 nCurrentCdh = nCdhOffset;
            for (qint32 i = 0; i < nCount; i++) {
                if ((nCurrentCdh + (qint64)sizeof(CENTRALDIRECTORYFILEHEADER)) > nFileSize) {
                    break;
                }
                CENTRALDIRECTORYFILEHEADER cdh = read_CENTRALDIRECTORYFILEHEADER(nCurrentCdh, pPdStruct);
                xfHeaderLFH.listRowLocations.append(cdh.nOffsetToLocalFileHeader);
                nCurrentCdh += sizeof(CENTRALDIRECTORYFILEHEADER) + cdh.nFileNameLength + cdh.nExtraFieldLength + cdh.nFileCommentLength;
            }

            if (!xfHeaderLFH.listRowLocations.isEmpty()) {
                qint64 nFirstLfh = xfHeaderLFH.listRowLocations.first();
                xfHeaderLFH.listFields = getXFRecords(xfStruct.fileType, STRUCTID_LOCALFILEHEADER, offsetToLoc(nFirstLfh));
                xfHeaderLFH.sTag = xfHeaderToTag(xfHeaderLFH, structIDToString(STRUCTID_LOCALFILEHEADER), xfHeaderLFH.sParentTag);
                listResult.append(xfHeaderLFH);
            }
        }
    }

    return listResult;
}

QList<XBinary::XFRECORD> XZip::getXFRecords(FT fileType, quint32 nStructID, const XLOC &xLoc)
{
    Q_UNUSED(fileType)

    QList<XBinary::XFRECORD> listResult;

    if (nStructID == STRUCTID_ENDOFCENTRALDIRECTORYRECORD) {
        listResult.append({"Signature", (qint32)offsetof(ENDOFCENTRALDIRECTORYRECORD, nSignature), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"DiskNumber", (qint32)offsetof(ENDOFCENTRALDIRECTORYRECORD, nDiskNumber), 2, XFRECORD_FLAG_COUNT, VT_UINT16});
        listResult.append({"StartDisk", (qint32)offsetof(ENDOFCENTRALDIRECTORYRECORD, nStartDisk), 2, XFRECORD_FLAG_COUNT, VT_UINT16});
        listResult.append({"DiskNumberOfRecords", (qint32)offsetof(ENDOFCENTRALDIRECTORYRECORD, nDiskNumberOfRecords), 2, XFRECORD_FLAG_COUNT, VT_UINT16});
        listResult.append({"TotalNumberOfRecords", (qint32)offsetof(ENDOFCENTRALDIRECTORYRECORD, nTotalNumberOfRecords), 2, XFRECORD_FLAG_COUNT, VT_UINT16});
        listResult.append({"SizeOfCentralDirectory", (qint32)offsetof(ENDOFCENTRALDIRECTORYRECORD, nSizeOfCentralDirectory), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"OffsetToCentralDirectory", (qint32)offsetof(ENDOFCENTRALDIRECTORYRECORD, nOffsetToCentralDirectory), 4, XFRECORD_FLAG_OFFSET, VT_UINT32});
        listResult.append({"CommentLength", (qint32)offsetof(ENDOFCENTRALDIRECTORYRECORD, nCommentLength), 2, XFRECORD_FLAG_SIZE, VT_UINT16});
        // Variable-length fields
        quint16 nCommentLength = read_uint16(xLoc.nLocation + offsetof(ENDOFCENTRALDIRECTORYRECORD, nCommentLength));
        listResult.append({"Comment", (qint32)sizeof(ENDOFCENTRALDIRECTORYRECORD), (qint32)nCommentLength, XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
    } else if (nStructID == STRUCTID_CENTRALDIRECTORYFILEHEADER) {
        listResult.append({"Signature", (qint32)offsetof(CENTRALDIRECTORYFILEHEADER, nSignature), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"Version", (qint32)offsetof(CENTRALDIRECTORYFILEHEADER, nVersion), 1, XFRECORD_FLAG_VERSION_DIVMOD, VT_UINT8});
        listResult.append({"OS", (qint32)offsetof(CENTRALDIRECTORYFILEHEADER, nOS), 1, XFRECORD_FLAG_NONE, VT_UINT8});
        listResult.append({"MinVersion", (qint32)offsetof(CENTRALDIRECTORYFILEHEADER, nMinVersion), 1, XFRECORD_FLAG_VERSION_DIVMOD, VT_UINT8});
        listResult.append({"MinOS", (qint32)offsetof(CENTRALDIRECTORYFILEHEADER, nMinOS), 1, XFRECORD_FLAG_NONE, VT_UINT8});
        listResult.append({"Flags", (qint32)offsetof(CENTRALDIRECTORYFILEHEADER, nFlags), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"Method", (qint32)offsetof(CENTRALDIRECTORYFILEHEADER, nMethod), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"LastModTime", (qint32)offsetof(CENTRALDIRECTORYFILEHEADER, nLastModTime), 2, XFRECORD_FLAG_DOSTIME, VT_UINT16});
        listResult.append({"LastModDate", (qint32)offsetof(CENTRALDIRECTORYFILEHEADER, nLastModDate), 2, XFRECORD_FLAG_DOSDATE, VT_UINT16});
        listResult.append({"CRC32", (qint32)offsetof(CENTRALDIRECTORYFILEHEADER, nCRC32), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"CompressedSize", (qint32)offsetof(CENTRALDIRECTORYFILEHEADER, nCompressedSize), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"UncompressedSize", (qint32)offsetof(CENTRALDIRECTORYFILEHEADER, nUncompressedSize), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"FileNameLength", (qint32)offsetof(CENTRALDIRECTORYFILEHEADER, nFileNameLength), 2, XFRECORD_FLAG_SIZE, VT_UINT16});
        listResult.append({"ExtraFieldLength", (qint32)offsetof(CENTRALDIRECTORYFILEHEADER, nExtraFieldLength), 2, XFRECORD_FLAG_SIZE, VT_UINT16});
        listResult.append({"FileCommentLength", (qint32)offsetof(CENTRALDIRECTORYFILEHEADER, nFileCommentLength), 2, XFRECORD_FLAG_SIZE, VT_UINT16});
        listResult.append({"StartDisk", (qint32)offsetof(CENTRALDIRECTORYFILEHEADER, nStartDisk), 2, XFRECORD_FLAG_COUNT, VT_UINT16});
        listResult.append({"InternalFileAttributes", (qint32)offsetof(CENTRALDIRECTORYFILEHEADER, nInternalFileAttributes), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"ExternalFileAttributes", (qint32)offsetof(CENTRALDIRECTORYFILEHEADER, nExternalFileAttributes), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"OffsetToLocalFileHeader", (qint32)offsetof(CENTRALDIRECTORYFILEHEADER, nOffsetToLocalFileHeader), 4, XFRECORD_FLAG_OFFSET, VT_UINT32});
        // Variable-length fields
        quint16 nFileNameLength = read_uint16(xLoc.nLocation + offsetof(CENTRALDIRECTORYFILEHEADER, nFileNameLength));
        quint16 nExtraFieldLength = read_uint16(xLoc.nLocation + offsetof(CENTRALDIRECTORYFILEHEADER, nExtraFieldLength));
        quint16 nFileCommentLength = read_uint16(xLoc.nLocation + offsetof(CENTRALDIRECTORYFILEHEADER, nFileCommentLength));
        qint32 nVarOffset = (qint32)sizeof(CENTRALDIRECTORYFILEHEADER);
        listResult.append({"FileName", nVarOffset, (qint32)nFileNameLength, XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        nVarOffset += nFileNameLength;
        listResult.append({"ExtraField", nVarOffset, (qint32)nExtraFieldLength, XFRECORD_FLAG_NONE, VT_BYTE_ARRAY});
        nVarOffset += nExtraFieldLength;
        listResult.append({"FileComment", nVarOffset, (qint32)nFileCommentLength, XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
    } else if (nStructID == STRUCTID_LOCALFILEHEADER) {
        listResult.append({"Signature", (qint32)offsetof(LOCALFILEHEADER, nSignature), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"MinVersion", (qint32)offsetof(LOCALFILEHEADER, nMinVersion), 1, XFRECORD_FLAG_VERSION_DIVMOD, VT_UINT8});
        listResult.append({"MinOS", (qint32)offsetof(LOCALFILEHEADER, nMinOS), 1, XFRECORD_FLAG_NONE, VT_UINT8});
        listResult.append({"Flags", (qint32)offsetof(LOCALFILEHEADER, nFlags), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"Method", (qint32)offsetof(LOCALFILEHEADER, nMethod), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"LastModTime", (qint32)offsetof(LOCALFILEHEADER, nLastModTime), 2, XFRECORD_FLAG_DOSTIME, VT_UINT16});
        listResult.append({"LastModDate", (qint32)offsetof(LOCALFILEHEADER, nLastModDate), 2, XFRECORD_FLAG_DOSDATE, VT_UINT16});
        listResult.append({"CRC32", (qint32)offsetof(LOCALFILEHEADER, nCRC32), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"CompressedSize", (qint32)offsetof(LOCALFILEHEADER, nCompressedSize), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"UncompressedSize", (qint32)offsetof(LOCALFILEHEADER, nUncompressedSize), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"FileNameLength", (qint32)offsetof(LOCALFILEHEADER, nFileNameLength), 2, XFRECORD_FLAG_SIZE, VT_UINT16});
        listResult.append({"ExtraFieldLength", (qint32)offsetof(LOCALFILEHEADER, nExtraFieldLength), 2, XFRECORD_FLAG_SIZE, VT_UINT16});
        // Variable-length fields
        quint16 nFileNameLength = read_uint16(xLoc.nLocation + offsetof(LOCALFILEHEADER, nFileNameLength));
        quint16 nExtraFieldLength = read_uint16(xLoc.nLocation + offsetof(LOCALFILEHEADER, nExtraFieldLength));
        qint32 nVarOffset = (qint32)sizeof(LOCALFILEHEADER);
        listResult.append({"FileName", nVarOffset, (qint32)nFileNameLength, XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        nVarOffset += nFileNameLength;
        listResult.append({"ExtraField", nVarOffset, (qint32)nExtraFieldLength, XFRECORD_FLAG_NONE, VT_BYTE_ARRAY});
    }

    return listResult;
}

QList<QString> XZip::getSearchSignatures()
{
    QList<QString> listResult;

    listResult.append("'PK'0304");
    listResult.append("'PK'0506");

    return listResult;
}

XBinary *XZip::createInstance(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    Q_UNUSED(bIsImage)
    Q_UNUSED(nModuleAddress)

    return new XZip(pDevice);
}

bool XZip::handleInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XZip> guardedThis(this);
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

void *XZip::getInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XZip> guardedThis(this);
    const bool bHandled = guardedThis->handleInternalInfo(pPdStruct);
    if (!guardedThis || !bHandled) return nullptr;

    return &guardedThis->m_internalInfo;
}

void XZip::setInternalInfo(void *pInternalInfo)
{
    if (pInternalInfo) {
        m_internalInfo = *static_cast<INTERNAL_INFO *>(pInternalInfo);
        XArchive::setInternalInfo(static_cast<XArchive::INTERNAL_INFO *>(&m_internalInfo));
    } else {
        m_internalInfo = INTERNAL_INFO();
        XArchive::setInternalInfo(nullptr);
    }
}
