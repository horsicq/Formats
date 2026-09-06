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
#include <QBuffer>
#include <memory>
#include <new>

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
    // A concatenated gzip transport has no single raw Deflate extent. The
    // base class already extracts from the fully verified materialized TAR
    // when no outer codec coordinates are advertised.
    nOuterStreamOffset = 0;
    nOuterStreamSize = 0;
    handleMethod = HANDLE_METHOD_UNKNOWN;
    return false;
}

QIODevice *XTAR_GZ::decompressData(PDSTRUCT *pPdStruct)
{
    QPointer<XTAR_GZ> guardedThis(this);
    QPointer<QIODevice> source(getDevice());
    const PDSTRUCTLIFETIME progressLifetime = pPdStruct ? retainPdStructLifetime(pPdStruct) : PDSTRUCTLIFETIME();
    const qint64 nOutputLimit = m_nMaterializedOutputLimit;
    if (!source || (nOutputLimit < 0) || !XBinary::isPdStructNotCanceled(pPdStruct)) return nullptr;
    XGzip gzip(source.data());
    UNPACK_STATE state = {};
    QMap<UNPACK_PROP, QVariant> properties;
    properties.insert(UNPACK_PROP_MAX_OUTPUT_SIZE, nOutputLimit);
    const bool bInitialized = gzip.initUnpack(&state, properties, pPdStruct);
    if (!guardedThis || !source || !bInitialized || (pPdStruct && !isPdStructLifetimeAlive(progressLifetime))) {
        gzip.finishUnpack(&state, nullptr);
        return nullptr;
    }
    std::unique_ptr<QBuffer> result(new (std::nothrow) QBuffer());
    if (!result || !result->open(QIODevice::ReadWrite)) {
        gzip.finishUnpack(&state, nullptr);
        return nullptr;
    }
    const bool bDecoded = gzip.unpackCurrent(&state, result.get(), pPdStruct);
    const bool bFinished = gzip.finishUnpack(&state, nullptr);
    if (!guardedThis || !source || !bDecoded || !bFinished || (pPdStruct && !isPdStructLifetimeAlive(progressLifetime)) ||
        !XBinary::isPdStructNotCanceled(pPdStruct) || (result->size() <= 0) || (result->size() > nOutputLimit) || !result->seek(0)) return nullptr;
    return result.release();
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
