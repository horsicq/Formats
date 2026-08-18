#include "xwebp.h"

XBinary::XCONVERT _TABLE_XWEBP_STRUCTID[] = {
    {XWEBP::STRUCTID_UNKNOWN, "Unknown", QObject::tr("Unknown")},
};

namespace {
const qint32 WEBP_MAX_CHUNK_COUNT = 65536;
}

XWEBP::XWEBP(QIODevice *pDevice) : XRiff(pDevice)
{
}
XWEBP::~XWEBP()
{
}

static bool _webpIsValidImageChunk(XWEBP *pWebp, const QByteArray &baType, qint64 nDataOffset, quint32 nDataSize, quint32 *pWidth, quint32 *pHeight, bool *pHasAlpha)
{
    if (pWidth) *pWidth = 0;
    if (pHeight) *pHeight = 0;
    if (pHasAlpha) *pHasAlpha = false;

    if (baType == QByteArrayLiteral("VP8 ")) {
        if ((nDataSize < 10) || ((pWebp->read_uint8(nDataOffset) & 1) != 0) ||
            (pWebp->read_array(nDataOffset + 3, 3) != QByteArray::fromHex("9d012a"))) {
            return false;
        }
        const quint32 nWidth = pWebp->read_uint16(nDataOffset + 6, false) & 0x3FFF;
        const quint32 nHeight = pWebp->read_uint16(nDataOffset + 8, false) & 0x3FFF;
        if ((nWidth == 0) || (nHeight == 0)) return false;
        if (pWidth) *pWidth = nWidth;
        if (pHeight) *pHeight = nHeight;
        return true;
    }
    if (baType == QByteArrayLiteral("VP8L")) {
        if ((nDataSize < 5) || (pWebp->read_uint8(nDataOffset) != 0x2F)) return false;
        const quint32 nBits = pWebp->read_uint32(nDataOffset + 1, false);
        if ((nBits >> 29) != 0) return false;
        if (pWidth) *pWidth = (nBits & 0x3FFF) + 1;
        if (pHeight) *pHeight = ((nBits >> 14) & 0x3FFF) + 1;
        if (pHasAlpha) *pHasAlpha = ((nBits >> 28) & 1) != 0;
        return true;
    }
    return false;
}

bool XWEBP::isValid(PDSTRUCT *pPdStruct)
{
    const qint64 nTotalSize = getSize();

    if ((nTotalSize < 20) || !XBinary::isPdStructNotCanceled(pPdStruct) || (read_array(0, 4) != QByteArrayLiteral("RIFF")) ||
        (read_array(8, 4) != QByteArrayLiteral("WEBP"))) {
        return false;
    }

    const quint32 nRiffSize = read_uint32(4, false);
    const qint64 nDeclaredEnd = 8 + (qint64)nRiffSize;

    if ((nRiffSize < 12) || (nDeclaredEnd > nTotalSize)) {
        return false;
    }

    qint64 nOffset = 12;
    qint32 nChunkIndex = 0;
    bool bSimple = false;
    bool bExtended = false;
    bool bHasTopImage = false;
    bool bHasAlpha = false;
    bool bHasICCP = false;
    bool bHasEXIF = false;
    bool bHasXMP = false;
    bool bHasANIM = false;
    qint32 nFrameCount = 0;
    quint8 nFeatureFlags = 0;
    quint32 nCanvasWidth = 0;
    quint32 nCanvasHeight = 0;

    while ((nOffset < nDeclaredEnd) && XBinary::isPdStructNotCanceled(pPdStruct)) {
        if (nChunkIndex >= WEBP_MAX_CHUNK_COUNT) {
            return false;
        }

        if (nOffset > nDeclaredEnd - 8) {
            return false;
        }

        const QByteArray baChunkType = read_array(nOffset, 4);
        const quint32 nChunkSize = read_uint32(nOffset + 4, false);
        const qint64 nChunkExtent = (qint64)nChunkSize + (nChunkSize & 1);

        if (nChunkExtent > nDeclaredEnd - nOffset - 8) {
            return false;
        }

        const qint64 nDataOffset = nOffset + 8;

        if (nChunkIndex == 0) {
            if ((baChunkType != QByteArrayLiteral("VP8 ")) && (baChunkType != QByteArrayLiteral("VP8L")) &&
                (baChunkType != QByteArrayLiteral("VP8X"))) {
                return false;
            }
            bSimple = (baChunkType == QByteArrayLiteral("VP8 ")) || (baChunkType == QByteArrayLiteral("VP8L"));
            bExtended = baChunkType == QByteArrayLiteral("VP8X");
        }

        if ((baChunkType == QByteArrayLiteral("VP8 ")) || (baChunkType == QByteArrayLiteral("VP8L"))) {
            quint32 nImageWidth = 0;
            quint32 nImageHeight = 0;
            bool bImageAlpha = false;
            if (bHasTopImage || bHasANIM || (nFrameCount != 0) ||
                !_webpIsValidImageChunk(this, baChunkType, nDataOffset, nChunkSize, &nImageWidth, &nImageHeight, &bImageAlpha)) {
                return false;
            }
            if (bExtended && ((nImageWidth != nCanvasWidth) || (nImageHeight != nCanvasHeight) ||
                              (bHasAlpha && (baChunkType != QByteArrayLiteral("VP8 "))))) {
                return false;
            }
            bHasTopImage = true;
            bHasAlpha |= bImageAlpha;
        } else if (baChunkType == QByteArrayLiteral("VP8X")) {
            if ((nChunkIndex != 0) || (nChunkSize != 10) || ((read_uint8(nDataOffset) & 0xC1) != 0) ||
                (read_uint8(nDataOffset + 1) != 0) || (read_uint8(nDataOffset + 2) != 0) || (read_uint8(nDataOffset + 3) != 0)) {
                return false;
            }
            nFeatureFlags = read_uint8(nDataOffset);
            nCanvasWidth = read_uint24(nDataOffset + 4, false) + 1;
            nCanvasHeight = read_uint24(nDataOffset + 7, false) + 1;
        } else if (baChunkType == QByteArrayLiteral("ALPH")) {
            if (!bExtended || bHasTopImage || bHasANIM || (nFrameCount != 0) || bHasAlpha || (nChunkSize < 1) ||
                ((read_uint8(nDataOffset) & 0xE3) != 0)) {
                return false;
            }
            bHasAlpha = true;
        } else if (baChunkType == QByteArrayLiteral("ICCP")) {
            if (!bExtended || bHasICCP || bHasTopImage || bHasANIM || (nFrameCount != 0) || (nChunkSize == 0)) return false;
            bHasICCP = true;
        } else if (baChunkType == QByteArrayLiteral("EXIF")) {
            if (!bExtended || bHasEXIF || (nChunkSize == 0)) return false;
            bHasEXIF = true;
        } else if (baChunkType == QByteArrayLiteral("XMP ")) {
            if (!bExtended || bHasXMP || (nChunkSize == 0)) return false;
            bHasXMP = true;
        } else if (baChunkType == QByteArrayLiteral("ANIM")) {
            if (!bExtended || bHasANIM || bHasTopImage || bHasAlpha || (nFrameCount != 0) || (nChunkSize != 6) ||
                ((nFeatureFlags & 0x02) == 0)) {
                return false;
            }
            bHasANIM = true;
        } else if (baChunkType == QByteArrayLiteral("ANMF")) {
            if (!bExtended || !bHasANIM || bHasTopImage || (nChunkSize < 16) ||
                ((read_uint8(nDataOffset + 15) & 0xFC) != 0)) {
                return false;
            }

            const quint32 nFrameX = read_uint24(nDataOffset, false) * 2;
            const quint32 nFrameY = read_uint24(nDataOffset + 3, false) * 2;
            const quint32 nFrameWidth = read_uint24(nDataOffset + 6, false) + 1;
            const quint32 nFrameHeight = read_uint24(nDataOffset + 9, false) + 1;
            if ((nFrameX > nCanvasWidth) || (nFrameY > nCanvasHeight) ||
                (nFrameWidth > nCanvasWidth - nFrameX) || (nFrameHeight > nCanvasHeight - nFrameY)) {
                return false;
            }

            const qint64 nFrameEnd = nDataOffset + nChunkSize;
            qint64 nFrameOffset = nDataOffset + 16;
            bool bFrameImage = false;
            bool bFrameAlpha = false;
            qint32 nFrameChunkCount = 0;

            while ((nFrameOffset < nFrameEnd) && XBinary::isPdStructNotCanceled(pPdStruct)) {
                if (nFrameChunkCount++ >= WEBP_MAX_CHUNK_COUNT) {
                    return false;
                }

                if (nFrameOffset > nFrameEnd - 8) {
                    return false;
                }

                const QByteArray baFrameType = read_array(nFrameOffset, 4);
                const quint32 nFrameSize = read_uint32(nFrameOffset + 4, false);
                const qint64 nFrameExtent = (qint64)nFrameSize + (nFrameSize & 1);

                if (nFrameExtent > nFrameEnd - nFrameOffset - 8) {
                    return false;
                }

                if (baFrameType == QByteArrayLiteral("ALPH")) {
                    if (bFrameAlpha || bFrameImage || (nFrameSize < 1) ||
                        ((read_uint8(nFrameOffset + 8) & 0xE3) != 0)) {
                        return false;
                    }
                    bFrameAlpha = true;
                } else if ((baFrameType == QByteArrayLiteral("VP8 ")) || (baFrameType == QByteArrayLiteral("VP8L"))) {
                    quint32 nImageWidth = 0;
                    quint32 nImageHeight = 0;
                    bool bImageAlpha = false;
                    if (bFrameImage || !_webpIsValidImageChunk(this, baFrameType, nFrameOffset + 8, nFrameSize,
                                                               &nImageWidth, &nImageHeight, &bImageAlpha) ||
                        (nImageWidth != nFrameWidth) || (nImageHeight != nFrameHeight) ||
                        (bFrameAlpha && (baFrameType != QByteArrayLiteral("VP8 ")))) {
                        return false;
                    }
                    bFrameImage = true;
                    bFrameAlpha |= bImageAlpha;
                } else if ((baFrameType == QByteArrayLiteral("VP8X")) || (baFrameType == QByteArrayLiteral("ANIM")) ||
                           (baFrameType == QByteArrayLiteral("ANMF")) || (baFrameType == QByteArrayLiteral("ICCP")) ||
                           (baFrameType == QByteArrayLiteral("EXIF")) || (baFrameType == QByteArrayLiteral("XMP "))) {
                    return false;
                }

                nFrameOffset += 8 + nFrameExtent;
            }

            if (!XBinary::isPdStructNotCanceled(pPdStruct) || !bFrameImage) {
                return false;
            }
            bHasAlpha |= bFrameAlpha;
            nFrameCount++;
        }

        nOffset += 8 + nChunkExtent;
        nChunkIndex++;
    }

    if (!XBinary::isPdStructNotCanceled(pPdStruct) || (nOffset != nDeclaredEnd)) {
        return false;
    }

    if (bSimple) {
        return bHasTopImage && !bExtended && !bHasICCP && !bHasEXIF && !bHasXMP && !bHasANIM && (nFrameCount == 0);
    }

    const bool bAnimationFlag = (nFeatureFlags & 0x02) != 0;
    const bool bFeatureStateValid = (((nFeatureFlags & 0x20) != 0) == bHasICCP) &&
                                    (((nFeatureFlags & 0x10) != 0) == bHasAlpha) &&
                                    (((nFeatureFlags & 0x08) != 0) == bHasEXIF) &&
                                    (((nFeatureFlags & 0x04) != 0) == bHasXMP);
    if (!bExtended || !bFeatureStateValid) return false;

    if (bAnimationFlag) {
        return bHasANIM && (nFrameCount > 0) && !bHasTopImage;
    }

    return !bHasANIM && (nFrameCount == 0) && bHasTopImage;
}

bool XWEBP::isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    XWEBP x(pDevice);
    return x.isValid(pPdStruct);
}

QString XWEBP::getFileFormatExt()
{
    return "webp";
}
QString XWEBP::getFileFormatExtsString()
{
    return "WebP";
}
qint64 XWEBP::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    return isValid(pPdStruct) ? (8 + (qint64)read_uint32(4, false)) : 0;
}
XBinary::FT XWEBP::getFileType()
{
    return FT_WEBP;
}
QString XWEBP::getMIMEString()
{
    return "image/webp";
}

QVector<XBinary::XMETADATA_STRUCT> XWEBP::getMetadataStructs()
{
    QVector<XMETADATA_STRUCT> listResult;
    if (!isValid((PDSTRUCT *)nullptr)) {
        return listResult;
    }

    auto appendMetadata = [this, &listResult](qint64 nOffset, qint64 nSize, XMETADATA_ID id, const QString &sName, const QVariant &varValue) {
        XMETADATA_STRUCT record = {};
        record.nOffset = nOffset;
        record.nSize = nSize;
        record.nAddress = offsetToAddress(nOffset);
        record.id = id;
        record.sName = sName;
        record.varValue = varValue;
        listResult.append(record);
    };

    const qint64 nDeclaredEnd = 8 + (qint64)read_uint32(4, false);
    const QByteArray baFirstType = read_array(12, 4);
    const qint64 nFirstDataOffset = 20;
    quint32 nWidth = 0;
    quint32 nHeight = 0;
    bool bAlpha = false;

    if (baFirstType == QByteArrayLiteral("VP8X")) {
        nWidth = read_uint24(nFirstDataOffset + 4, false) + 1;
        nHeight = read_uint24(nFirstDataOffset + 7, false) + 1;
        appendMetadata(nFirstDataOffset + 4, 3, XMETADATA_ID_FRAME_WIDTH, QString("Canvas width"), nWidth);
        appendMetadata(nFirstDataOffset + 7, 3, XMETADATA_ID_FRAME_HEIGHT, QString("Canvas height"), nHeight);
    } else if (_webpIsValidImageChunk(this, baFirstType, nFirstDataOffset, read_uint32(16, false), &nWidth, &nHeight, &bAlpha)) {
        const qint64 nDimensionOffset = nFirstDataOffset + ((baFirstType == QByteArrayLiteral("VP8 ")) ? 6 : 1);
        const qint64 nDimensionSize = (baFirstType == QByteArrayLiteral("VP8 ")) ? 2 : 4;
        appendMetadata(nDimensionOffset, nDimensionSize, XMETADATA_ID_FRAME_WIDTH, QString("Width"), nWidth);
        appendMetadata(nDimensionOffset, nDimensionSize, XMETADATA_ID_FRAME_HEIGHT, QString("Height"), nHeight);
    }

    qint32 nFrameCount = 0;
    qint64 nOffset = 12;
    while (nOffset + 8 <= nDeclaredEnd) {
        const QByteArray baType = read_array(nOffset, 4);
        const quint32 nChunkSize = read_uint32(nOffset + 4, false);
        if (baType == QByteArrayLiteral("ANMF")) {
            ++nFrameCount;
        }
        nOffset += 8 + (qint64)nChunkSize + (nChunkSize & 1);
    }

    appendMetadata(12, 4, XMETADATA_ID_CODEC, QString("Codec"), QString::fromLatin1(baFirstType).trimmed());
    appendMetadata(12, nDeclaredEnd - 12, XMETADATA_ID_FRAME_COUNT, QString("Frame count"), nFrameCount ? nFrameCount : 1);

    return listResult;
}

QString XWEBP::structIDToString(quint32 nID)
{
    return XBinary::XCONVERT_idToTransString(nID, _TABLE_XWEBP_STRUCTID, sizeof(_TABLE_XWEBP_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QString XWEBP::structIDToFtString(quint32 nID)
{
    return XBinary::XCONVERT_idToFtString(nID, _TABLE_XWEBP_STRUCTID, sizeof(_TABLE_XWEBP_STRUCTID) / sizeof(XBinary::XCONVERT));
}

quint32 XWEBP::ftStringToStructID(const QString &sFtString)
{
    return XCONVERT_ftStringToId(sFtString, _TABLE_XWEBP_STRUCTID, sizeof(_TABLE_XWEBP_STRUCTID) / sizeof(XBinary::XCONVERT));
}

bool XWEBP::handleInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XWEBP> guardedThis(this);
    bool bResult = true;

    if (!isInternalInfoHandled()) {
        bResult = guardedThis->XRiff::handleInternalInfo(pPdStruct);
        if (!guardedThis || !bResult) return false;

        XRiff::INTERNAL_INFO *pInfo =
            static_cast<XRiff::INTERNAL_INFO *>(
                guardedThis->XRiff::getInternalInfo(pPdStruct));
        if (!guardedThis || !pInfo) return false;

        static_cast<XRiff::INTERNAL_INFO &>(
            guardedThis->m_internalInfo) = *pInfo;
        guardedThis->setIsInternalInfoHandled(true);
    }

    return guardedThis && bResult;
}

void *XWEBP::getInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XWEBP> guardedThis(this);
    const bool bHandled = guardedThis->handleInternalInfo(pPdStruct);
    if (!guardedThis || !bHandled) return nullptr;

    return &guardedThis->m_internalInfo;
}

void XWEBP::setInternalInfo(void *pInternalInfo)
{
    if (pInternalInfo) {
        m_internalInfo = *static_cast<INTERNAL_INFO *>(pInternalInfo);
        XRiff::setInternalInfo(static_cast<XRiff::INTERNAL_INFO *>(&m_internalInfo));
        setIsInternalInfoHandled(true);
    } else {
        m_internalInfo = INTERNAL_INFO();
        XRiff::setInternalInfo(nullptr);
        setIsInternalInfoHandled(false);
    }
}
