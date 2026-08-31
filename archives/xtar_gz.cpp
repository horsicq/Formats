/* Copyright (c) 2026 hors<horsicq@gmail.com>
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
#include "xtar_gz.h"
#include "xgzip.h"

XTAR_GZ::XTAR_GZ(QIODevice *pDevice) : XTARCOMPRESSED(pDevice)
{
    m_compressionType = COMPRESSION_GZIP;
}

XTAR_GZ::~XTAR_GZ()
{
}

bool XTAR_GZ::isValid(PDSTRUCT *pPdStruct)
{
    return XTARCOMPRESSED::isValid(pPdStruct);
}

bool XTAR_GZ::isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    if (!pDevice) {
        return false;
    }

    if (detectCompressionType(pDevice) != COMPRESSION_GZIP) return false;
    XTAR_GZ archive(pDevice);
    return archive.XTARCOMPRESSED::isValid(pPdStruct);
}

XBinary::FT XTAR_GZ::getFileType()
{
    return FT_TAR_GZ;
}

QString XTAR_GZ::getFileFormatExt()
{
    return "tar.gz";
}

QString XTAR_GZ::getFileFormatExtsString()
{
    return "*.tar.gz;*.tgz";
}

QString XTAR_GZ::getMIMEString()
{
    return "application/gzip";
}

bool XTAR_GZ::getOuterStreamInfo(qint64 &nOuterStreamOffset, qint64 &nOuterStreamSize, HANDLE_METHOD &handleMethod)
{
    XGzip xgzip(getDevice());
    if (!xgzip.isValid()) {
        return false;
    }
    qint64 nHeaderSize = xgzip.getHeaderSize();
    qint64 nTotalSize = getSize();
    if (nTotalSize <= (nHeaderSize + 8)) {
        return false;
    }
    nOuterStreamOffset = nHeaderSize;
    nOuterStreamSize = nTotalSize - nHeaderSize - 8;
    handleMethod = HANDLE_METHOD_DEFLATE;
    return true;
}

QIODevice *XTAR_GZ::decompressData(PDSTRUCT *pPdStruct)
{
    const PDSTRUCTLIFETIME progressLifetime = pPdStruct ? retainPdStructLifetime(pPdStruct) : PDSTRUCTLIFETIME();
    XGzip xgzip(getDevice());

    if (!xgzip.isValid(pPdStruct)) {
        return nullptr;
    }

    qint64 nHeaderSize = xgzip.getHeaderSize();
    qint64 nTotalSize = getSize();

    if (nTotalSize <= (nHeaderSize + 8)) {
        return nullptr;
    }

    qint64 nCompressedOffset = nHeaderSize;
    qint64 nCompressedSize = nTotalSize - nHeaderSize - 8;  // Footer: CRC32 + ISIZE

    QIODevice *pResult = decompressByMethod(HANDLE_METHOD_DEFLATE, nCompressedOffset, nCompressedSize, pPdStruct);
    if (!pResult) {
        return nullptr;
    }

    // decompressByMethod() intentionally receives only the raw DEFLATE
    // payload.  Validate the RFC 1952 footer here; otherwise a .tar.gz with a
    // corrupted CRC32 or ISIZE can still contain a parseable TAR and be
    // accepted by the compressed-TAR detector.
    const QByteArray baFooter = read_array(nTotalSize - 8, 8);
    if (baFooter.size() != 8) {
        delete pResult;
        return nullptr;
    }

    const quint32 nExpectedCRC =
        (quint32)(quint8)baFooter.at(0) | ((quint32)(quint8)baFooter.at(1) << 8) | ((quint32)(quint8)baFooter.at(2) << 16) | ((quint32)(quint8)baFooter.at(3) << 24);
    const quint32 nExpectedSize =
        (quint32)(quint8)baFooter.at(4) | ((quint32)(quint8)baFooter.at(5) << 8) | ((quint32)(quint8)baFooter.at(6) << 16) | ((quint32)(quint8)baFooter.at(7) << 24);

    QPointer<QIODevice> guardedResult(pResult);
    const qint64 nResultSize = guardedResult->size();
    if (!guardedResult) return nullptr;
    if (pPdStruct && !isPdStructLifetimeAlive(progressLifetime)) {
        delete guardedResult.data();
        return nullptr;
    }
    const bool bCRCValid =
        ((quint32)nResultSize == nExpectedSize) && XBinary::checkCRC(guardedResult.data(), CRC_TYPE_FFFFFFFF_EDB88320_FFFFFFFFF, nExpectedCRC, pPdStruct);
    if (!guardedResult) return nullptr;
    if (pPdStruct && !isPdStructLifetimeAlive(progressLifetime)) {
        delete guardedResult.data();
        return nullptr;
    }
    const bool bFooterValid = bCRCValid && XBinary::isPdStructNotCanceled(pPdStruct);
    if (!bFooterValid) {
        delete guardedResult.data();
        return nullptr;
    }

    return guardedResult.data();
}

bool XTAR_GZ::handleInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XTAR_GZ> guardedThis(this);
    bool bResult = true;

    if (!isInternalInfoHandled()) {
        bResult = guardedThis->XTARCOMPRESSED::handleInternalInfo(pPdStruct);
        if (!guardedThis || !bResult) return false;
        XTARCOMPRESSED::INTERNAL_INFO *pInfo = static_cast<XTARCOMPRESSED::INTERNAL_INFO *>(guardedThis->XTARCOMPRESSED::getInternalInfo(pPdStruct));
        if (!guardedThis || !pInfo) return false;
        static_cast<XTARCOMPRESSED::INTERNAL_INFO &>(guardedThis->m_internalInfo) = *pInfo;
    }

    return guardedThis && bResult;
}

void *XTAR_GZ::getInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XTAR_GZ> guardedThis(this);
    const bool bHandled = guardedThis->handleInternalInfo(pPdStruct);
    if (!guardedThis || !bHandled) return nullptr;

    return &guardedThis->m_internalInfo;
}

void XTAR_GZ::setInternalInfo(void *pInternalInfo)
{
    if (pInternalInfo) {
        m_internalInfo = *static_cast<INTERNAL_INFO *>(pInternalInfo);
        XTARCOMPRESSED::setInternalInfo(static_cast<XTARCOMPRESSED::INTERNAL_INFO *>(&m_internalInfo));
    } else {
        m_internalInfo = INTERNAL_INFO();
        XTARCOMPRESSED::setInternalInfo(nullptr);
    }
}
