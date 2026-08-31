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
#include "xwav.h"
#include "../xmetadataappender.h"

XWAV::XWAV(QIODevice *pDevice) : XRiff(pDevice)
{
}

XWAV::~XWAV()
{
}

bool XWAV::isValid(PDSTRUCT *pPdStruct)
{
    if (!XRiff::isValid(pPdStruct)) return false;
    QString form = read_ansiString(8, 4);
    return (form == "WAVE");
}

bool XWAV::isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    XWAV x(pDevice);
    return x.isValid(pPdStruct);
}

QString XWAV::getFileFormatExt()
{
    return "wav";
}

QString XWAV::getFileFormatExtsString()
{
    return "WAV";
}

qint64 XWAV::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    return _calculateRawSize(pPdStruct);
}

XBinary::FT XWAV::getFileType()
{
    return FT_WAV;
}

QString XWAV::getMIMEString()
{
    return "audio/x-wav";
}

QVector<XBinary::XMETADATA_STRUCT> XWAV::getMetadataStructs()
{
    QVector<XMETADATA_STRUCT> listResult;
    if (!isValid((PDSTRUCT *)nullptr)) {
        return listResult;
    }

    qint64 nFmtOffset = -1;
    quint32 nFmtSize = 0;
    qint64 nDataOffset = -1;
    quint32 nDataSize = 0;
    const qint64 nEnd = qMin<qint64>(getSize(), 8 + (qint64)read_uint32(4, false));

    for (qint64 nOffset = 12; nOffset + 8 <= nEnd;) {
        const QString sType = read_ansiString(nOffset, 4);
        const quint32 nSize = read_uint32(nOffset + 4, false);
        if ((qint64)nSize > nEnd - nOffset - 8) {
            break;
        }
        if ((sType == QString("fmt ")) && (nSize >= 16) && (nFmtOffset == -1)) {
            nFmtOffset = nOffset + 8;
            nFmtSize = nSize;
        } else if ((sType == QString("data")) && (nDataOffset == -1)) {
            nDataOffset = nOffset + 8;
            nDataSize = nSize;
        }
        nOffset += 8 + (qint64)nSize + (nSize & 1);
    }

    if (nFmtOffset == -1) {
        return listResult;
    }

    const XMetadataAppender appendMetadata(this, &listResult);

    const quint16 nFormat = read_uint16(nFmtOffset, false);
    const quint16 nChannels = read_uint16(nFmtOffset + 2, false);
    const quint32 nSampleRate = read_uint32(nFmtOffset + 4, false);
    const quint32 nByteRate = read_uint32(nFmtOffset + 8, false);
    const quint16 nBitsPerSample = read_uint16(nFmtOffset + 14, false);
    appendMetadata(nFmtOffset, 2, XMETADATA_ID_CODEC, QString("Audio format"), QString("0x%1").arg(nFormat, 4, 16, QChar('0')));
    appendMetadata(nFmtOffset + 2, 2, XMETADATA_ID_CHANNELS, QString("Channels"), nChannels);
    appendMetadata(nFmtOffset + 4, 4, XMETADATA_ID_SAMPLE_RATE, QString("Sample rate"), nSampleRate);
    appendMetadata(nFmtOffset + 8, 4, XMETADATA_ID_BITRATE, QString("Bitrate"), (quint64)nByteRate * 8);
    appendMetadata(nFmtOffset + 14, 2, XMETADATA_ID_BIT_DEPTH, QString("Bits per sample"), nBitsPerSample);
    if ((nDataOffset != -1) && nByteRate) {
        appendMetadata(nDataOffset, nDataSize, XMETADATA_ID_DURATION, QString("Duration"), (double)nDataSize / nByteRate);
    }

    Q_UNUSED(nFmtSize)
    return listResult;
}

QList<QString> XWAV::getSearchSignatures()
{
    QList<QString> listResult;

    listResult.append("'RIFF'....'WAVE'");

    return listResult;
}

XBinary *XWAV::createInstance(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    Q_UNUSED(bIsImage)
    Q_UNUSED(nModuleAddress)

    return new XWAV(pDevice);
}

bool XWAV::handleInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XWAV> guardedThis(this);
    bool bResult = true;

    if (!isInternalInfoHandled()) {
        bResult = guardedThis->XRiff::handleInternalInfo(pPdStruct);
        if (!guardedThis || !bResult) return false;

        XRiff::INTERNAL_INFO *pInfo = static_cast<XRiff::INTERNAL_INFO *>(guardedThis->XRiff::getInternalInfo(pPdStruct));
        if (!guardedThis || !pInfo) return false;

        static_cast<XRiff::INTERNAL_INFO &>(guardedThis->m_internalInfo) = *pInfo;
        guardedThis->setIsInternalInfoHandled(true);
    }

    return guardedThis && bResult;
}

void *XWAV::getInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XWAV> guardedThis(this);
    const bool bHandled = guardedThis->handleInternalInfo(pPdStruct);
    if (!guardedThis || !bHandled) return nullptr;

    return &guardedThis->m_internalInfo;
}

void XWAV::setInternalInfo(void *pInternalInfo)
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
