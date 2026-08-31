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
#include "xtarcompressed.h"

#include "xdecompress.h"
#include "xtar_compress.h"
#include "xtar_gz.h"
#ifdef USE_ARCHIVE
#include "xtar_bzip2.h"
#include "xtar_lzip.h"
#include "xtar_lzma.h"
#include "xtar_lzop.h"
#include "xtar_lz4.h"
#include "xtar_xz.h"
#include "xtar_zstd.h"
#endif

#include <QBuffer>
#include <QPointer>

namespace {

// Compressed TAR handlers materialize the complete TAR so that the streaming
// archive API can traverse it repeatedly.  Keep that unavoidable allocation
// bounded for untrusted input instead of allowing a small compressed stream to
// grow a QBuffer until the process runs out of memory.
const qint64 TARCOMPRESSED_MAX_DECOMPRESSED_SIZE = Q_INT64_C(512) * 1024 * 1024;

class BoundedTarOutputBuffer : public QBuffer {
public:
    BoundedTarOutputBuffer(QByteArray *pData, qint64 nLimit) : QBuffer(pData), m_nLimit(nLimit), m_bLimitExceeded(false)
    {
    }

    bool seek(qint64 nPosition) override
    {
        if ((nPosition < 0) || (nPosition > m_nLimit)) {
            m_bLimitExceeded = true;
            return false;
        }

        return QBuffer::seek(nPosition);
    }

    bool isLimitExceeded() const
    {
        return m_bLimitExceeded;
    }

protected:
    qint64 writeData(const char *pData, qint64 nMaximumSize) override
    {
        const qint64 nPosition = pos();
        if ((nMaximumSize < 0) || ((nMaximumSize > 0) && !pData) || (nPosition < 0) || (nPosition > m_nLimit) || (nMaximumSize > (m_nLimit - nPosition))) {
            m_bLimitExceeded = true;
            return -1;
        }

        return QBuffer::writeData(pData, nMaximumSize);
    }

private:
    qint64 m_nLimit;
    bool m_bLimitExceeded;
};

// Codec of the transport envelope, for DISPLAY only.  It is deliberately NOT
// published as FPART_PROP_HANDLEMETHOD/HANDLEMETHOD2: those are decode
// instructions, and an index-paired record must keep
// HANDLE_METHOD_ARCHIVE_STREAM - which, together with its no-extent
// coordinates, is what makes every offset-based consumer fail closed.
// A listing can still say which compressor wraps the TAR.
XBinary::HANDLE_METHOD tarcContainerCodec(XTARCOMPRESSED::COMPRESSION_TYPE compressionType)
{
    XBinary::HANDLE_METHOD result = XBinary::HANDLE_METHOD_UNKNOWN;

    if (compressionType == XTARCOMPRESSED::COMPRESSION_GZIP) {
        result = XBinary::HANDLE_METHOD_DEFLATE;
    } else if (compressionType == XTARCOMPRESSED::COMPRESSION_BZIP2) {
        result = XBinary::HANDLE_METHOD_BZIP2;
    } else if (compressionType == XTARCOMPRESSED::COMPRESSION_XZ) {
        result = XBinary::HANDLE_METHOD_XZ;
    } else if (compressionType == XTARCOMPRESSED::COMPRESSION_LZMA) {
        result = XBinary::HANDLE_METHOD_LZMA;
    } else if (compressionType == XTARCOMPRESSED::COMPRESSION_ZSTD) {
        result = XBinary::HANDLE_METHOD_ZSTD;
    } else if (compressionType == XTARCOMPRESSED::COMPRESSION_COMPRESS) {
        result = XBinary::HANDLE_METHOD_COMPRESS;
    } else if (compressionType == XTARCOMPRESSED::COMPRESSION_LZIP) {
        result = XBinary::HANDLE_METHOD_LZIP;
    } else if (compressionType == XTARCOMPRESSED::COMPRESSION_LZOP) {
        result = XBinary::HANDLE_METHOD_LZOP;
    } else if (compressionType == XTARCOMPRESSED::COMPRESSION_LZ4) {
        result = XBinary::HANDLE_METHOD_LZ4;
    }

    return result;
}

}  // namespace

XTARCOMPRESSED::XTARCOMPRESSED(QIODevice *pDevice) : XTAR(pDevice)
{
    m_pDecompressedData = nullptr;
    m_pOriginalDevice = nullptr;
    m_compressionType = COMPRESSION_UNKNOWN;
    m_nOuterStreamOffset = 0;
    m_nOuterStreamSize = 0;
    m_outerHandleMethod = HANDLE_METHOD_UNKNOWN;
    m_nMaterializedOutputLimit = TARCOMPRESSED_MAX_DECOMPRESSED_SIZE;
}

bool XTARCOMPRESSED::isSolidRecordAuthorized() const
{
    return PRIVATE_RECORD_AUTHORIZATION::isHeldBy(m_pUnpackGuardState);
}

XTARCOMPRESSED::~XTARCOMPRESSED()
{
    if (m_pDecompressedData) {
        delete m_pDecompressedData.data();
        m_pDecompressedData = nullptr;
    }
}

bool XTARCOMPRESSED::isValid(PDSTRUCT *pPdStruct)
{
    QPointer<XTARCOMPRESSED> guardedArchive(this);
    QPointer<QIODevice> guardedDevice(guardedArchive->getDevice());
    if (!guardedDevice || guardedDevice->isSequential() || !guardedArchive || !guardedDevice) {
        return false;
    }

    const qint64 nOriginalPosition = guardedDevice->pos();
    if (!guardedArchive || !guardedDevice || (nOriginalPosition < 0)) {
        return false;
    }

    UNPACK_STATE state = {};
    const QMap<UNPACK_PROP, QVariant> mapProperties;
    bool bResult = guardedArchive->initUnpack(&state, mapProperties, pPdStruct);
    if (!guardedArchive) return false;
    const bool bFinished = guardedArchive->finishUnpack(&state, pPdStruct);
    if (!guardedArchive || !guardedDevice || !bFinished) return false;

    if (!guardedDevice->seek(nOriginalPosition) || !guardedArchive || !guardedDevice) {
        bResult = false;
    }

    return bResult;
}

bool XTARCOMPRESSED::isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    QPointer<QIODevice> guardedDevice(pDevice);
    if (!guardedDevice) {
        return false;
    }

    if (guardedDevice->isSequential() || !guardedDevice) {
        return false;
    }

    const qint64 nOriginalPosition = guardedDevice->pos();
    if (!guardedDevice || (nOriginalPosition < 0)) {
        return false;
    }

    const COMPRESSION_TYPE compressionType = detectCompressionType(guardedDevice.data());
    if (!guardedDevice) return false;
    QPointer<XArchive> guardedArchive(getCompressionClassInstance(compressionType, guardedDevice.data()));

    if (!guardedArchive) {
        return false;
    }

    UNPACK_STATE state = {};
    const QMap<UNPACK_PROP, QVariant> mapProperties;
    bool bResult = guardedArchive->initUnpack(&state, mapProperties, pPdStruct);
    if (guardedArchive) {
        const bool bFinished = guardedArchive->finishUnpack(&state, pPdStruct);
        if (!guardedArchive || !bFinished) bResult = false;
    } else {
        bResult = false;
    }
    if (guardedArchive) delete guardedArchive.data();

    if (!guardedDevice || !guardedDevice->seek(nOriginalPosition) || !guardedDevice) {
        bResult = false;
    }

    return bResult;
}

XTARCOMPRESSED::COMPRESSION_TYPE XTARCOMPRESSED::detectCompressionType(QIODevice *pDevice)
{
    QPointer<QIODevice> guardedDevice(pDevice);
    if (!guardedDevice || guardedDevice->isSequential() || !guardedDevice) {
        return COMPRESSION_UNKNOWN;
    }

    qint64 nOffset = guardedDevice->pos();
    if (!guardedDevice || (nOffset < 0) || !guardedDevice->seek(0) || !guardedDevice) {
        return COMPRESSION_UNKNOWN;
    }

    QByteArray baMagic = guardedDevice->read(6);
    if (!guardedDevice) return COMPRESSION_UNKNOWN;
    bool bRead0 = (baMagic.size() >= 1);
    bool bRead1 = (baMagic.size() >= 2);
    bool bRead2 = (baMagic.size() >= 3);
    bool bRead3 = (baMagic.size() >= 4);
    bool bRead4 = (baMagic.size() >= 5);
    bool bRead5 = (baMagic.size() >= 6);

    quint8 nByte0 = bRead0 ? (quint8)(uchar)baMagic.at(0) : 0;
    quint8 nByte1 = bRead1 ? (quint8)(uchar)baMagic.at(1) : 0;
    quint8 nByte2 = bRead2 ? (quint8)(uchar)baMagic.at(2) : 0;
    quint8 nByte3 = bRead3 ? (quint8)(uchar)baMagic.at(3) : 0;
    quint8 nByte4 = bRead4 ? (quint8)(uchar)baMagic.at(4) : 0;
    quint8 nByte5 = bRead5 ? (quint8)(uchar)baMagic.at(5) : 0;

    COMPRESSION_TYPE result = COMPRESSION_UNKNOWN;

    if (bRead0 && bRead1) {
        if ((nByte0 == 0x1F) && (nByte1 == 0x8B)) {
            result = COMPRESSION_GZIP;
        } else if ((nByte0 == 0x42) && (nByte1 == 0x5A)) {
            result = COMPRESSION_BZIP2;
        } else if ((nByte0 == 0x1F) && (nByte1 == 0x9D)) {
            result = COMPRESSION_COMPRESS;
        } else if (bRead2 && (nByte0 == 0x89) && (nByte1 == 0x4C) && (nByte2 == 0x5A)) {
            result = COMPRESSION_LZOP;
        } else if ((nByte0 == 0x4C) && (nByte1 == 0x5A) && bRead2 && bRead3 && (nByte2 == 0x49) && (nByte3 == 0x50)) {
            result = COMPRESSION_LZIP;
        } else if (bRead3 && (nByte0 == 0x5D) && (nByte1 == 0x00) && (nByte2 == 0x00) && (nByte3 == 0x00)) {
            result = COMPRESSION_LZMA;
        } else if (bRead5 && (nByte0 == 0xFD) && (nByte1 == 0x37) && (nByte2 == 0x7A) && (nByte3 == 0x58) && (nByte4 == 0x5A) && (nByte5 == 0x00)) {
            result = COMPRESSION_XZ;
        } else if (bRead3 && (nByte0 == 0x28) && (nByte1 == 0xB5) && (nByte2 == 0x2F) && (nByte3 == 0xFD)) {
            result = COMPRESSION_ZSTD;
        } else if (bRead3 && (((nByte0 == 0x04) && (nByte1 == 0x22) && (nByte2 == 0x4D) && (nByte3 == 0x18)) ||
                              ((nByte0 == 0x02) && (nByte1 == 0x21) && (nByte2 == 0x4C) && (nByte3 == 0x18)) ||
                              ((nByte0 >= 0x50) && (nByte0 <= 0x5F) && (nByte1 == 0x2A) && (nByte2 == 0x4D) && (nByte3 == 0x18)))) {
            result = COMPRESSION_LZ4;
        }
    }

    if (!guardedDevice->seek(nOffset) || !guardedDevice) {
        return COMPRESSION_UNKNOWN;
    }

    return result;
}

XArchive *XTARCOMPRESSED::getCompressionClassInstance(COMPRESSION_TYPE compressionType, QIODevice *pDevice)
{
    XArchive *pResult = nullptr;

    if (compressionType == COMPRESSION_GZIP) {
        pResult = new XTAR_GZ(pDevice);
#ifdef USE_ARCHIVE
    } else if (compressionType == COMPRESSION_BZIP2) {
        pResult = new XTAR_BZIP2(pDevice);
    } else if (compressionType == COMPRESSION_XZ) {
        pResult = new XTAR_XZ(pDevice);
    } else if (compressionType == COMPRESSION_LZMA) {
        pResult = new XTAR_LZMA(pDevice);
    } else if (compressionType == COMPRESSION_ZSTD) {
        pResult = new XTAR_ZSTD(pDevice);
#endif
    } else if (compressionType == COMPRESSION_COMPRESS) {
        pResult = new XTAR_COMPRESS(pDevice);
#ifdef USE_ARCHIVE
    } else if (compressionType == COMPRESSION_LZIP) {
        pResult = new XTAR_LZIP(pDevice);
    } else if (compressionType == COMPRESSION_LZOP) {
        pResult = new XTAR_LZOP(pDevice);
    } else if (compressionType == COMPRESSION_LZ4) {
        pResult = new XTAR_LZ4(pDevice);
#endif
    }

    return pResult;
}

QMap<XBinary::UNPACK_PROP, QVariant> XTARCOMPRESSED::getDefaultUnpackProperties()
{
    QMap<XBinary::UNPACK_PROP, QVariant> result = XTAR::getDefaultUnpackProperties();

    return result;
}

struct TARC_INITUNPACK_FAIL_CONTEXT {
    QPointer<XTARCOMPRESSED> *pGuardedArchive;
    XBinary::UNPACK_STATE *pState;
    QPointer<QIODevice> *pDecompressedData;
    QPointer<QIODevice> *pOriginalDevice;
    qint64 *pnOuterStreamOffset;
    qint64 *pnOuterStreamSize;
    XBinary::HANDLE_METHOD *pOuterHandleMethod;
};

static bool tarcFailInitUnpack(TARC_INITUNPACK_FAIL_CONTEXT *pFailContext)
{
    if (!pFailContext->pGuardedArchive->isNull()) {
        QPointer<QIODevice> guardedDecompressed(*(pFailContext->pDecompressedData));
        *(pFailContext->pDecompressedData) = nullptr;
        *(pFailContext->pOriginalDevice) = nullptr;
        *(pFailContext->pnOuterStreamOffset) = 0;
        *(pFailContext->pnOuterStreamSize) = 0;
        *(pFailContext->pOuterHandleMethod) = XBinary::HANDLE_METHOD_UNKNOWN;
        (*(pFailContext->pGuardedArchive))->releaseUnpackSource(pFailContext->pState);
        if (guardedDecompressed) delete guardedDecompressed.data();
    }
    *(pFailContext->pState) = XBinary::UNPACK_STATE();
    return false;
}

bool XTARCOMPRESSED::initUnpack(UNPACK_STATE *pState, const QMap<UNPACK_PROP, QVariant> &mapProperties, PDSTRUCT *pPdStruct)
{
    QPointer<XTARCOMPRESSED> guardedArchive(this);
    if (!pState || m_bUnpackOperationInProgress) return false;
    if ((pState->pContext || !pState->baUnpackSourceToken.isEmpty()) && !guardedArchive->ownsUnpackSource(pState)) {
        return false;
    }
    if (!guardedArchive->finishUnpack(pState, nullptr) || !guardedArchive) return false;
    UNPACK_OPERATION_GUARD operationGuard(&m_bUnpackOperationInProgress);
    if (!operationGuard.isAcquired()) return false;

    qint64 nConfiguredOutputLimit = -1;
    if (!XBinary::getUnpackOutputLimit(mapProperties, &nConfiguredOutputLimit)) {
        return false;
    }
    guardedArchive->m_nMaterializedOutputLimit =
        (nConfiguredOutputLimit >= 0) ? qMin(TARCOMPRESSED_MAX_DECOMPRESSED_SIZE, nConfiguredOutputLimit) : TARCOMPRESSED_MAX_DECOMPRESSED_SIZE;

    QPointer<QIODevice> guardedOriginal(guardedArchive->getDevice());
    if (!guardedOriginal || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return false;
    }
    const bool bBound = guardedArchive->bindUnpackSource(pState, pPdStruct);
    if (!guardedArchive || !guardedOriginal || !bBound) return false;

    TARC_INITUNPACK_FAIL_CONTEXT failContext = {};
    failContext.pGuardedArchive = &guardedArchive;
    failContext.pState = pState;
    failContext.pDecompressedData = &m_pDecompressedData;
    failContext.pOriginalDevice = &m_pOriginalDevice;
    failContext.pnOuterStreamOffset = &m_nOuterStreamOffset;
    failContext.pnOuterStreamSize = &m_nOuterStreamSize;
    failContext.pOuterHandleMethod = &m_outerHandleMethod;

    pState->mapUnpackProperties = mapProperties;

    COMPRESSION_TYPE compressionType = guardedArchive->m_compressionType;
    if (compressionType == COMPRESSION_UNKNOWN) {
        compressionType = detectCompressionType(guardedOriginal.data());
        if (!guardedArchive || !guardedOriginal) {
            *pState = UNPACK_STATE();
            return false;
        }
        guardedArchive->m_compressionType = compressionType;
    }

    if (compressionType == COMPRESSION_UNKNOWN) {
        return tarcFailInitUnpack(&failContext);
    }

    guardedArchive->m_pOriginalDevice = guardedOriginal;
    QPointer<QIODevice> guardedDecompressed(guardedArchive->decompressData(pPdStruct));
    if (!guardedArchive) {
        if (guardedDecompressed) delete guardedDecompressed.data();
        *pState = UNPACK_STATE();
        return false;
    }
    guardedArchive->m_pDecompressedData = guardedDecompressed;

    if (!guardedDecompressed || !guardedOriginal || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return tarcFailInitUnpack(&failContext);
    }

    // Parse the materialized TAR through a separate view.  Rebinding this
    // archive, even temporarily, advances XBinary's source generation and
    // invalidates the source snapshot held by the hardened extraction path.
    XTAR materializedArchive(guardedDecompressed.data());
    UNPACK_STATE materializedState = {};
    bool bResult = materializedArchive.initUnpack(&materializedState, mapProperties, pPdStruct);

    if (!guardedArchive || !guardedDecompressed || !guardedOriginal) {
        materializedArchive.finishUnpack(&materializedState, nullptr);
        *pState = UNPACK_STATE();
        return false;
    }

    bResult = bResult && materializedState.pContext && (materializedState.nCurrentIndex == 0) && (materializedState.nNumberOfRecords > 0) &&
              (materializedState.nCurrentIndex < materializedState.nNumberOfRecords) && (materializedState.nCurrentOffset >= 0) && (materializedState.nTotalSize >= 0) &&
              (materializedState.nCurrentOffset <= materializedState.nTotalSize);

    if (bResult) {
        UNPACK_CONTEXT *pContext = static_cast<UNPACK_CONTEXT *>(materializedState.pContext);
        qint64 nOuterStreamOffset = 0;
        qint64 nOuterStreamSize = 0;
        HANDLE_METHOD outerHandleMethod = HANDLE_METHOD_UNKNOWN;
        (void)guardedArchive->getOuterStreamInfo(nOuterStreamOffset, nOuterStreamSize, outerHandleMethod);
        if (!guardedArchive || !guardedOriginal || !guardedDecompressed) {
            *pState = UNPACK_STATE();
            return false;
        }
        guardedArchive->m_nOuterStreamOffset = nOuterStreamOffset;
        guardedArchive->m_nOuterStreamSize = nOuterStreamSize;
        guardedArchive->m_outerHandleMethod = outerHandleMethod;
        pState->mapUnpackProperties = materializedState.mapUnpackProperties;
        pState->mapArchiveProperties = materializedState.mapArchiveProperties;
        pState->nCurrentOffset = materializedState.nCurrentOffset;
        pState->nTotalSize = materializedState.nTotalSize;
        pState->nCurrentIndex = materializedState.nCurrentIndex;
        pState->nNumberOfRecords = materializedState.nNumberOfRecords;
        materializedArchive.releaseUnpackSource(&materializedState);
        materializedState.pContext = nullptr;
        pState->pContext = pContext;
        bResult = guardedArchive->validateAndFinalizeUnpackSource(pState, pContext, pPdStruct);
        if (!guardedArchive) {
            return false;
        }
        if (!bResult) {
            pState->pContext = nullptr;
            delete pContext;
        }
    } else {
        materializedArchive.finishUnpack(&materializedState, nullptr);
    }

    return bResult ? true : tarcFailInitUnpack(&failContext);
}

XBinary::ARCHIVERECORD XTARCOMPRESSED::infoCurrent(UNPACK_STATE *pState, PDSTRUCT *pPdStruct)
{
    UNPACK_OPERATION_GUARD operationGuard(&m_bUnpackOperationInProgress, &m_bNestedUnpackInfoAuthorized);
    if (!operationGuard.isAllowed()) return XBinary::ARCHIVERECORD();
    QPointer<XTARCOMPRESSED> guardedArchive(this);

    if (!pState || !guardedArchive->isUnpackSourceCurrent(pState, pPdStruct) || !guardedArchive) {
        return XBinary::ARCHIVERECORD{};
    }
    QPointer<QIODevice> guardedDecompressed(guardedArchive->m_pDecompressedData);
    QPointer<QIODevice> guardedOriginal(guardedArchive->m_pOriginalDevice);
    if (!guardedDecompressed || !guardedOriginal) {
        return XBinary::ARCHIVERECORD{};
    }
    XTAR materializedArchive(guardedDecompressed.data());
    UNPACK_STATE materializedState = *pState;
    materializedState.pContext = nullptr;
    materializedState.baUnpackSourceToken.clear();
    const bool bMaterializedBound = materializedArchive.bindUnpackSource(&materializedState, pPdStruct);
    if (bMaterializedBound) materializedState.pContext = pState->pContext;
    if (!bMaterializedBound || !materializedArchive.validateAndFinalizeUnpackSource(&materializedState, pPdStruct) || !guardedArchive || !guardedDecompressed ||
        !guardedOriginal) {
        materializedArchive.releaseUnpackSource(&materializedState);
        return XBinary::ARCHIVERECORD{};
    }
    XBinary::ARCHIVERECORD result = materializedArchive.infoCurrent(&materializedState, pPdStruct);
    materializedArchive.releaseUnpackSource(&materializedState);
    if (!guardedArchive || !guardedDecompressed || !guardedOriginal) {
        return XBinary::ARCHIVERECORD{};
    }
    const qint64 nDecompressedSize = guardedDecompressed->size();
    if (!guardedArchive || !guardedDecompressed || !guardedOriginal || result.mapProperties.isEmpty() || (result.nStreamOffset < 0) || (result.nStreamSize < 0) ||
        (nDecompressedSize < 0) || (result.nStreamOffset > nDecompressedSize) || (result.nStreamSize > (nDecompressedSize - result.nStreamOffset))) {
        return XBinary::ARCHIVERECORD{};
    }
    // Everything above was measured on m_pDecompressedData, a PRIVATE buffer
    // that no caller can address.  A record must never leave this function
    // carrying those coordinates while claiming the compressed file as its
    // source: a raw-offset consumer would then copy container bytes at the
    // member's exact length, which is corruption at the correct size and is
    // undetectable for a format (TAR) that publishes no payload checksum.
    const qint64 nOriginalSize = guardedOriginal->size();
    if (!guardedArchive || !guardedDecompressed || !guardedOriginal || (nOriginalSize < 0)) {
        return XBinary::ARCHIVERECORD{};
    }

    // If getOuterStreamInfo populated valid outer-stream metadata, build a solid-block
    // ARCHIVERECORD that decompressArchiveRecord can use directly with the original file.
    // FPART_PROP_SUBSTREAMOFFSET carries the TAR entry's offset in the decompressed buffer.
    //
    // That shape is only ever built for this class's own unpackCurrent().  Its
    // coordinates are the container's ENTIRE compressed stream shared by every
    // member, not this member's own bytes, so publishing it would hand any
    // caller a valid (offset,size) pair over the raw container with a single
    // mutable method field standing between it and a read - forge the method
    // and 158,632 bytes of raw container come back as "./sample_elf", forge the
    // container's own codec and the whole decoded TAR does.  Exactly the defect
    // the five index-paired families no longer have.  Public callers therefore
    // get the same index-paired archive-stream record those families get; the
    // solid decode below is unchanged and still runs for real extraction.
    if ((guardedArchive->m_nOuterStreamSize > 0) && (guardedArchive->m_outerHandleMethod != HANDLE_METHOD_UNKNOWN) && guardedArchive->isSolidRecordAuthorized()) {
        // The republished extent has to be fully addressable on the original
        // compressed device, otherwise the pairing is not established.
        if ((guardedArchive->m_nOuterStreamOffset < 0) || (guardedArchive->m_nOuterStreamOffset > nOriginalSize) ||
            (guardedArchive->m_nOuterStreamSize > (nOriginalSize - guardedArchive->m_nOuterStreamOffset))) {
            return XBinary::ARCHIVERECORD{};
        }
        result.mapProperties.insert(FPART_PROP_SUBSTREAMOFFSET, result.nStreamOffset);
        result.mapProperties.insert(FPART_PROP_HANDLEMETHOD, (quint32)guardedArchive->m_outerHandleMethod);
        result.mapProperties.insert(FPART_PROP_ISSOLID, true);
        result.mapProperties.insert(FPART_PROP_SOLIDFOLDERINDEX, (qint64)0);
        result.mapProperties.insert(FPART_PROP_STREAMUNPACKEDSIZE, nDecompressedSize);
        // Same display statement as the archive-stream branch below: the
        // republished extent is the container's single compressed stream, not
        // this member's own packed bytes.
        result.mapProperties.insert(FPART_PROP_INFO, QString("%1 (whole-archive stream)").arg(XBinary::handleMethodToString(guardedArchive->m_outerHandleMethod)));
        result.nStreamOffset = guardedArchive->m_nOuterStreamOffset;
        result.nStreamSize = guardedArchive->m_nOuterStreamSize;
    } else {
        // Every publicly visible record takes this branch, whether or not an
        // outer-stream pairing exists: a member of a compressed TAR has no
        // extent of its own on the compressed device either way.  Republish it
        // as an index-paired archive stream: the coordinates are dropped, the
        // private decoded-stream method and offsets are stripped, and the only
        // way to extract it is back through this archive's own session
        // (_unpackRecordByIndex).  Offset-based decoding of such a record is
        // refused outright.
        if (!XBinary::markArchiveStreamRecord(&result, pState->nCurrentIndex)) {
            return XBinary::ARCHIVERECORD{};
        }
        // The record now carries no extent at all - not the private decoded
        // buffer's, and not the container's either.  Republishing the
        // transport envelope here would be exactly as dangerous as
        // republishing the decoded coordinates: both are (offset,size) pairs
        // that resolve on some device a consumer can reach.  A listing has
        // nothing to misread and a forged codec has nothing to read.
        // Deliberately do NOT republish the outer-stream branch's
        // solid-block trio here: FPART_PROP_ISSOLID, FPART_PROP_SOLIDFOLDERINDEX
        // and FPART_PROP_STREAMUNPACKEDSIZE are a decode contract, not
        // decoration.  XDecompress reads them as "decode the shared stream,
        // then cut this member's slice out of it", which would hand a caller
        // that forges the container codec onto these whole-file coordinates a
        // plausible-looking prefix of the decoded TAR at exactly the member's
        // declared size - wrong bytes at the right size, the very failure this
        // record shape exists to prevent.  Only a descriptive string is safe.
        const HANDLE_METHOD containerCodec = tarcContainerCodec(guardedArchive->m_compressionType);
        if (containerCodec != HANDLE_METHOD_UNKNOWN) {
            result.mapProperties.insert(FPART_PROP_INFO, QString("%1 (whole-archive stream)").arg(XBinary::handleMethodToString(containerCodec)));
        }
    }
    if (!guardedArchive->isUnpackSourceCurrent(pState, pPdStruct) || !guardedArchive || !guardedDecompressed || !guardedOriginal) {
        return XBinary::ARCHIVERECORD{};
    }
    return result;
}

QIODevice *XTARCOMPRESSED::getRecordStreamDevice(UNPACK_STATE *pState)
{
    Q_UNUSED(pState)

    // The TAR structure is parsed out of the private decompressed buffer.  The
    // outer-stream record shape - the only one whose coordinates are measured
    // on the original compressed file - is now built exclusively for this
    // class's own decode (see infoCurrent), so this answer is about that
    // internal shape; no published record carries an extent on either device.
    if ((m_nOuterStreamSize > 0) && (m_outerHandleMethod != HANDLE_METHOD_UNKNOWN)) {
        return m_pOriginalDevice.data();
    }

    return m_pDecompressedData.data();
}

bool XTARCOMPRESSED::getOuterStreamInfo(qint64 &nOuterStreamOffset, qint64 &nOuterStreamSize, HANDLE_METHOD &handleMethod)
{
    Q_UNUSED(nOuterStreamOffset)
    Q_UNUSED(nOuterStreamSize)
    Q_UNUSED(handleMethod)
    return false;
}

bool XTARCOMPRESSED::moveToNext(UNPACK_STATE *pState, PDSTRUCT *pPdStruct)
{
    UNPACK_OPERATION_GUARD operationGuard(&m_bUnpackOperationInProgress);
    if (!operationGuard.isAcquired()) return false;
    QPointer<XTARCOMPRESSED> guardedArchive(this);

    if (!pState || !guardedArchive->isUnpackSourceCurrent(pState, pPdStruct) || !guardedArchive) {
        return false;
    }
    QPointer<QIODevice> guardedDecompressed(guardedArchive->m_pDecompressedData);
    QPointer<QIODevice> guardedOriginal(guardedArchive->m_pOriginalDevice);
    if (!guardedDecompressed || !guardedOriginal) return false;
    XTAR materializedArchive(guardedDecompressed.data());
    UNPACK_STATE materializedState = *pState;
    materializedState.pContext = nullptr;
    materializedState.baUnpackSourceToken.clear();
    const bool bMaterializedBound = materializedArchive.bindUnpackSource(&materializedState, pPdStruct);
    if (bMaterializedBound) materializedState.pContext = pState->pContext;
    if (!bMaterializedBound || !materializedArchive.validateAndFinalizeUnpackSource(&materializedState, pPdStruct) || !guardedArchive || !guardedDecompressed ||
        !guardedOriginal) {
        materializedArchive.releaseUnpackSource(&materializedState);
        return false;
    }
    const bool bResult = materializedArchive.moveToNext(&materializedState, pPdStruct);
    materializedArchive.releaseUnpackSource(&materializedState);
    if (!guardedArchive || !guardedDecompressed || !guardedOriginal || !guardedArchive->isUnpackSourceCurrent(pState, pPdStruct) || !guardedArchive ||
        (materializedState.nCurrentIndex < 0) || (materializedState.nNumberOfRecords <= 0) || (materializedState.nCurrentIndex > materializedState.nNumberOfRecords) ||
        (materializedState.nCurrentOffset < 0) || (materializedState.nTotalSize < 0) || (materializedState.nCurrentOffset > materializedState.nTotalSize))
        return false;
    pState->nCurrentOffset = materializedState.nCurrentOffset;
    pState->nCurrentIndex = materializedState.nCurrentIndex;
    return bResult;
}

bool XTARCOMPRESSED::unpackCurrent(UNPACK_STATE *pState, QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    QPointer<XTARCOMPRESSED> guardedArchive(this);
    QPointer<QIODevice> guardedOutput(pDevice);
    if (!pState || !guardedOutput || !guardedArchive->isUnpackSourceCurrent(pState, pPdStruct) || !guardedArchive) {
        return false;
    }
    QPointer<QIODevice> guardedOriginal(guardedArchive->m_pOriginalDevice);
    QPointer<QIODevice> guardedDecompressed(guardedArchive->m_pDecompressedData);
    if (!guardedOriginal || !guardedDecompressed || !guardedOutput->isOpen() || !guardedArchive || !guardedOutput || !guardedOutput->isWritable() || !guardedArchive ||
        !guardedOutput || guardedOutput->isSequential() || !guardedArchive || !guardedOutput) {
        return false;
    }
    const QIODevice::OpenMode outputMode = guardedOutput->openMode();
    if (!guardedArchive || !guardedOutput || (outputMode & (QIODevice::Append | QIODevice::Text)) || !XBinary::isPdStructNotCanceled(pPdStruct) || !guardedOriginal ||
        !guardedDecompressed)
        return false;
    const bool bAliases = XBinary::devicesAlias(guardedOriginal.data(), guardedOutput.data());
    if (!guardedArchive || !guardedOutput || !guardedOriginal || !guardedDecompressed || bAliases) return false;

    const bool bHasOuterStream = (guardedArchive->m_nOuterStreamSize > 0) && (guardedArchive->m_outerHandleMethod != HANDLE_METHOD_UNKNOWN);
    bool bResult = false;

    if (bHasOuterStream) {
        // infoCurrent() parses the materialized TAR through an independent
        // view.  Enter the generic archive path with the original compressed
        // source so its snapshot, alias check and decoder input all refer to
        // one stable device.
        if (guardedArchive->getDevice() != guardedOriginal.data()) return false;
        // Authorize the solid-block record shape for the duration of this
        // decode only.  XArchive::unpackCurrent() calls infoCurrent() itself;
        // this is the one caller entitled to the container-stream coordinates,
        // because it is the code that actually decodes that stream and cuts
        // this member out of it.  Nothing escapes: the record never leaves
        // XArchive::unpackCurrent().
        PRIVATE_RECORD_AUTHORIZATION solidAuthorization(guardedArchive->m_pUnpackGuardState);
        if (!solidAuthorization.isAuthorized()) return false;
        bResult = guardedArchive->XTAR::unpackCurrent(pState, guardedOutput.data(), pPdStruct);
        if (!guardedArchive || !guardedOutput || !guardedOriginal || !guardedDecompressed) return false;
    } else {
        // The local TAR view owns its own source token.  Keeping the outer
        // archive bound to the compressed source avoids generation changes
        // and lets reentrant finish/setDevice attempts fail at this guard.
        UNPACK_OPERATION_GUARD operationGuard(&m_bUnpackOperationInProgress);
        if (!operationGuard.isAcquired() || !guardedArchive->isUnpackSourceCurrent(pState, pPdStruct) || !guardedArchive || !guardedDecompressed || !guardedOriginal) {
            return false;
        }

        XTAR materializedArchive(guardedDecompressed.data());
        UNPACK_STATE materializedState = *pState;
        materializedState.pContext = nullptr;
        materializedState.baUnpackSourceToken.clear();
        const bool bMaterializedBound = materializedArchive.bindUnpackSource(&materializedState, pPdStruct);
        if (bMaterializedBound) materializedState.pContext = pState->pContext;
        if (!bMaterializedBound || !materializedArchive.validateAndFinalizeUnpackSource(&materializedState, pPdStruct) || !guardedArchive || !guardedOutput ||
            !guardedOriginal || !guardedDecompressed) {
            materializedArchive.releaseUnpackSource(&materializedState);
            return false;
        }
        bResult = materializedArchive.unpackCurrent(&materializedState, guardedOutput.data(), pPdStruct);
        materializedArchive.releaseUnpackSource(&materializedState);

        if (guardedArchive && guardedOutput && guardedOriginal && guardedDecompressed && bResult && guardedArchive->isUnpackSourceCurrent(pState, pPdStruct) &&
            guardedArchive) {
            pState->nCurrentOffset = materializedState.nCurrentOffset;
        } else {
            if (guardedOutput) {
                XBinary::resize(guardedOutput.data(), 0);
                if (guardedOutput) guardedOutput->seek(0);
            }
            bResult = false;
        }
    }

    return bResult && guardedArchive && guardedOutput && guardedOriginal && guardedDecompressed && guardedArchive->isUnpackSourceCurrent(pState, pPdStruct) &&
           guardedArchive && guardedOutput && guardedOriginal && guardedDecompressed;
}

bool XTARCOMPRESSED::finishUnpack(UNPACK_STATE *pState, PDSTRUCT *pPdStruct)
{
    UNPACK_OPERATION_GUARD operationGuard(&m_bUnpackOperationInProgress);
    if (!operationGuard.isAcquired()) return false;
    QPointer<XTARCOMPRESSED> guardedArchive(this);

    Q_UNUSED(pPdStruct)

    if (!pState) return false;
    if ((pState->pContext || !pState->baUnpackSourceToken.isEmpty()) && !guardedArchive->ownsUnpackSource(pState)) {
        return false;
    }
    UNPACK_CONTEXT *pContext = static_cast<UNPACK_CONTEXT *>(pState->pContext);
    guardedArchive->releaseUnpackSource(pState);
    pState->pContext = nullptr;
    delete pContext;

    if (guardedArchive->m_pDecompressedData) {
        QPointer<QIODevice> guardedDecompressed(guardedArchive->m_pDecompressedData);
        guardedArchive->m_pDecompressedData = nullptr;
        if (guardedDecompressed) delete guardedDecompressed.data();
        if (!guardedArchive) {
            *pState = UNPACK_STATE();
            return false;
        }
    }

    guardedArchive->m_pOriginalDevice = nullptr;
    guardedArchive->m_nOuterStreamOffset = 0;
    guardedArchive->m_nOuterStreamSize = 0;
    guardedArchive->m_outerHandleMethod = HANDLE_METHOD_UNKNOWN;

    pState->mapUnpackProperties.clear();
    pState->mapArchiveProperties.clear();
    pState->nCurrentOffset = 0;
    pState->nTotalSize = 0;
    pState->nCurrentIndex = 0;
    pState->nNumberOfRecords = 0;

    return true;
}

QIODevice *XTARCOMPRESSED::decompressByMethod(HANDLE_METHOD handleMethod, qint64 nOffset, qint64 nSize, PDSTRUCT *pPdStruct)
{
    QPointer<XTARCOMPRESSED> guardedArchive(this);
    QPointer<QIODevice> guardedDevice(guardedArchive->getDevice());

    if (!guardedDevice) {
        return nullptr;
    }

    qint64 nInputSize = nSize;

    if (nInputSize == -1) {
        const qint64 nSizeFromDevice = guardedDevice->size();
        if (!guardedArchive || !guardedDevice) return nullptr;
        nInputSize = nSizeFromDevice - nOffset;
    }

    const qint64 nDeviceSize = guardedDevice->size();
    if (!guardedArchive || !guardedDevice) return nullptr;
    if ((nOffset < 0) || (nInputSize <= 0) || (nDeviceSize < 0) || (nOffset > nDeviceSize) || (nInputSize > (nDeviceSize - nOffset))) {
        return nullptr;
    }

    QByteArray baData;
    const qint64 nOutputLimit = guardedArchive->m_nMaterializedOutputLimit;
    if (nOutputLimit < 0) return nullptr;
    BoundedTarOutputBuffer output(&baData, nOutputLimit);
    if (!output.open(QIODevice::ReadWrite)) {
        return nullptr;
    }

    XBinary::DATAPROCESS_STATE state = {};
    state.pDeviceInput = guardedDevice.data();
    state.pDeviceOutput = &output;
    state.nInputOffset = nOffset;
    state.nInputLimit = nInputSize;
    state.nProcessedOffset = 0;
    state.nProcessedLimit = -1;
    state.mapUnpackProperties.insert(UNPACK_PROP_MAX_OUTPUT_SIZE, nOutputLimit);
    state.mapProperties.insert(FPART_PROP_HANDLEMETHOD, handleMethod);

    XDecompress decompress;
    const bool bDecoded = decompress.multiDecompress(&state, pPdStruct);
    output.close();

    if (!guardedArchive || !guardedDevice) {
        baData.clear();
        return nullptr;
    }

    // decomressToByteArray() historically returned bytes even when the decoder
    // reported failure.  Do not admit a parseable TAR prefix from a truncated or
    // checksum-invalid compressed stream, and require the exact bounded source
    // extent to have been consumed.
    const bool bComplete = bDecoded && !state.bReadError && !state.bWriteError && !output.isLimitExceeded() && XBinary::isPdStructNotCanceled(pPdStruct) &&
                           (state.nCountInput == nInputSize) && (state.nCountOutput == baData.size()) && (state.nCountOutput > 0) && (state.nCountOutput <= nOutputLimit);
    if (!bComplete) {
        baData.clear();
        return nullptr;
    }

    return XTARCOMPRESSED::createMemoryBuffer(baData);
}

QIODevice *XTARCOMPRESSED::createMemoryBuffer(const QByteArray &baData)
{
    if (baData.isEmpty()) {
        return nullptr;
    }

    QBuffer *pBuffer = new QBuffer();
    pBuffer->setData(baData);

    if (!pBuffer->open(QIODevice::ReadOnly)) {
        delete pBuffer;
        return nullptr;
    }

    return pBuffer;
}

bool XTARCOMPRESSED::handleInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XTARCOMPRESSED> guardedThis(this);
    bool bResult = true;

    if (!isInternalInfoHandled()) {
        bResult = guardedThis->XTAR::handleInternalInfo(pPdStruct);
        if (!guardedThis || !bResult) return false;
        XTAR::INTERNAL_INFO *pInfo = static_cast<XTAR::INTERNAL_INFO *>(guardedThis->XTAR::getInternalInfo(pPdStruct));
        if (!guardedThis || !pInfo) return false;
        static_cast<XTAR::INTERNAL_INFO &>(guardedThis->m_internalInfo) = *pInfo;
    }

    return guardedThis && bResult;
}

void *XTARCOMPRESSED::getInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XTARCOMPRESSED> guardedThis(this);
    const bool bHandled = guardedThis->handleInternalInfo(pPdStruct);
    if (!guardedThis || !bHandled) return nullptr;

    return &guardedThis->m_internalInfo;
}

void XTARCOMPRESSED::setInternalInfo(void *pInternalInfo)
{
    if (pInternalInfo) {
        m_internalInfo = *static_cast<INTERNAL_INFO *>(pInternalInfo);
        XTAR::setInternalInfo(static_cast<XTAR::INTERNAL_INFO *>(&m_internalInfo));
    } else {
        m_internalInfo = INTERNAL_INFO();
        XTAR::setInternalInfo(nullptr);
    }
}
