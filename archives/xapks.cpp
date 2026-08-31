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
#include "xapks.h"
#include "subdevice.h"

#include <QBuffer>

namespace {
const qint32 APKS_APK_CANDIDATE_LIMIT = 8;
const qint64 APKS_DEFLATED_APK_LIMIT = 64LL * 1024 * 1024;
const qint64 APKS_TOC_LIMIT = 16LL * 1024 * 1024;

bool isCompleteAPK(QIODevice *pDevice, XBinary::PDSTRUCT *pPdStruct)
{
    if (!pDevice || !pDevice->isOpen() || !pDevice->isReadable()) return false;
    XZip innerZip(pDevice);
    const qint64 nECDOffset = innerZip.findECDOffset(pPdStruct);
    if (nECDOffset < 0) return false;

    QList<XArchive::RECORD> listRecords = innerZip.getRecords(20000, pPdStruct);
    return XAPK::isValid(pDevice, &listRecords, pPdStruct);
}

class DevicePositionGuard {
public:
    explicit DevicePositionGuard(QIODevice *pDevice) : m_pDevice(pDevice), m_nPosition(-1)
    {
        if (m_pDevice && !m_pDevice->isSequential()) {
            m_nPosition = m_pDevice->pos();
        }
    }

    ~DevicePositionGuard()
    {
        if (m_pDevice && (m_nPosition >= 0) && m_pDevice->isOpen()) {
            m_pDevice->seek(m_nPosition);
        }
    }

private:
    QPointer<QIODevice> m_pDevice;
    qint64 m_nPosition;
};

bool isStoredRecordCRCValid(QIODevice *pDevice, const XArchive::RECORD &record, XBinary::PDSTRUCT *pPdStruct)
{
    if (!pDevice || (record.nDataOffset < 0) || (record.nDataSize <= 0) || (record.nDataSize != record.spInfo.nUncompressedSize) ||
        !record.mapProperties.contains(XBinary::FPART_PROP_RESULTCRC) || !record.mapProperties.contains(XBinary::FPART_PROP_CRC_TYPE)) {
        return false;
    }

    SubDevice dataDevice(pDevice, record.nDataOffset, record.nDataSize);
    if (!dataDevice.open(QIODevice::ReadOnly)) return false;
    const XBinary::CRC_TYPE crcType = (XBinary::CRC_TYPE)record.mapProperties.value(XBinary::FPART_PROP_CRC_TYPE).toUInt();
    const bool bValid = XBinary::checkCRC(&dataDevice, crcType, record.mapProperties.value(XBinary::FPART_PROP_RESULTCRC), pPdStruct);
    dataDevice.close();
    return bValid;
}

bool hasValidTableOfContents(QIODevice *pDevice, XZip *pOuterZip, const QList<XArchive::RECORD> *pListRecords, XBinary::PDSTRUCT *pPdStruct)
{
    if (!pDevice || !pOuterZip || !pListRecords) return false;

    for (qint32 i = 0; (i < pListRecords->count()) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        const XArchive::RECORD &record = pListRecords->at(i);
        if ((record.spInfo.sRecordName != QLatin1String("toc.pb")) || (record.spInfo.nUncompressedSize <= 0) || (record.spInfo.nUncompressedSize > APKS_TOC_LIMIT) ||
            (record.nDataSize <= 0) || (record.nDataSize > APKS_TOC_LIMIT) || record.mapProperties.value(XBinary::FPART_PROP_ENCRYPTED, false).toBool() ||
            (record.spInfo.compressMethod2 != XBinary::HANDLE_METHOD_UNKNOWN)) {
            continue;
        }

        if (record.spInfo.compressMethod == XBinary::HANDLE_METHOD_STORE) {
            if (isStoredRecordCRCValid(pDevice, record, pPdStruct)) return true;
        } else if (record.spInfo.compressMethod == XBinary::HANDLE_METHOD_DEFLATE) {
            const QByteArray baTOC = pOuterZip->decompress(&record, pPdStruct, 0, APKS_TOC_LIMIT + 1);
            if (baTOC.size() == record.spInfo.nUncompressedSize) return true;
        }
    }

    return false;
}
}  // namespace

XAPKS::XAPKS(QIODevice *pDevice) : XAPK(pDevice)
{
}

XAPKS::~XAPKS()
{
}

bool XAPKS::isValid(PDSTRUCT *pPdStruct)
{
    DevicePositionGuard positionGuard(getDevice());
    bool bResult = false;

    XZip xzip(getDevice());

    // An APK Set is the ZIP produced by bundletool's build-apks command.  It
    // is not itself an APK, so requiring an AndroidManifest.xml/classes.dex at
    // the outer level rejects every normal .apks file.
    if (xzip.isValid(pPdStruct)) {
        QList<XArchive::RECORD> listArchiveRecords = xzip.getRecords(20000, pPdStruct);
        bResult = isValid(getDevice(), &listArchiveRecords, pPdStruct);
    }

    return bResult;
}

bool XAPKS::isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    XAPKS xapks(pDevice);

    return xapks.isValid(pPdStruct);
}

bool XAPKS::isValid(QList<RECORD> *pListRecords, PDSTRUCT *pPdStruct)
{
    if (!pListRecords) return false;

    bool bHasTableOfContents = false;
    bool bHasAPK = false;

    for (qint32 i = 0; (i < pListRecords->count()) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        const RECORD &record = pListRecords->at(i);
        const QString &sFileName = record.spInfo.sRecordName;

        if ((sFileName == QLatin1String("toc.pb")) && (record.spInfo.nUncompressedSize != 0)) {
            bHasTableOfContents = true;
        } else if (sFileName.endsWith(QLatin1String(".apk"), Qt::CaseInsensitive) && (record.spInfo.nUncompressedSize != 0)) {
            bHasAPK = true;
        }

        if (bHasTableOfContents && bHasAPK) return true;
    }

    return false;
}

bool XAPKS::isValid(QIODevice *pDevice, QList<RECORD> *pListRecords, PDSTRUCT *pPdStruct)
{
    if (!pDevice || !isValid(pListRecords, pPdStruct)) return false;
    DevicePositionGuard positionGuard(pDevice);

    XZip outerZip(pDevice);
    if (!hasValidTableOfContents(pDevice, &outerZip, pListRecords, pPdStruct)) {
        return false;
    }
    qint32 nCandidates = 0;
    for (qint32 i = 0; (i < pListRecords->count()) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        const RECORD &record = pListRecords->at(i);
        if (!record.spInfo.sRecordName.endsWith(QLatin1String(".apk"), Qt::CaseInsensitive) || (record.spInfo.nUncompressedSize <= 0) ||
            record.mapProperties.value(XBinary::FPART_PROP_ENCRYPTED, false).toBool() || (record.spInfo.compressMethod2 != XBinary::HANDLE_METHOD_UNKNOWN) ||
            ((record.spInfo.compressMethod != XBinary::HANDLE_METHOD_STORE) && (record.spInfo.compressMethod != XBinary::HANDLE_METHOD_DEFLATE))) {
            continue;
        }

        if (++nCandidates > APKS_APK_CANDIDATE_LIMIT) break;

        if (record.spInfo.compressMethod == XBinary::HANDLE_METHOD_STORE) {
            if ((record.nDataOffset < 0) || (record.nDataSize <= 0) || (record.nDataSize != record.spInfo.nUncompressedSize)) {
                continue;
            }
            if (!isStoredRecordCRCValid(pDevice, record, pPdStruct)) {
                continue;
            }
            SubDevice apkDevice(pDevice, record.nDataOffset, record.nDataSize);
            if (apkDevice.open(QIODevice::ReadOnly)) {
                const bool bValid = isCompleteAPK(&apkDevice, pPdStruct);
                apkDevice.close();
                if (bValid) return true;
            }
        } else {
            // Bundletool normally stores already-compressed APK members.  A
            // deflated outer member is supported only when both sides are
            // bounded, then decoded and CRC-authenticated in full before the
            // inner central directory and Android identity are trusted.
            if ((record.nDataSize <= 0) || (record.nDataSize > APKS_DEFLATED_APK_LIMIT) || (record.spInfo.nUncompressedSize > APKS_DEFLATED_APK_LIMIT)) {
                continue;
            }
            const QByteArray baAPK = outerZip.decompress(&record, pPdStruct);
            if (baAPK.size() != record.spInfo.nUncompressedSize) continue;
            QBuffer apkBuffer;
            apkBuffer.setData(baAPK);
            if (apkBuffer.open(QIODevice::ReadOnly)) {
                const bool bValid = isCompleteAPK(&apkBuffer, pPdStruct);
                apkBuffer.close();
                if (bValid) return true;
            }
        }
    }

    return false;
}

QString XAPKS::getFileFormatExt()
{
    return "apks";
}

QString XAPKS::getMIMEString()
{
    return "application/vnd.android.package-archive";
}

XBinary::FT XAPKS::getFileType()
{
    return FT_APKS;
}

XBinary::FILEFORMATINFO XAPKS::getFileFormatInfo(PDSTRUCT *pPdStruct)
{
    XBinary::FILEFORMATINFO result = {};

    if (isValid(pPdStruct)) {
        result.bIsValid = true;
        result.nSize = getSize();
        result.sExt = getFileFormatExt();
        result.fileType = FT_APKS;
        result.osName = OSNAME_ANDROID;
        result.sArch = getArch();
        result.mode = getMode();
        result.sType = typeIdToString(getType());
        result.endian = getEndian();
    }

    return result;
}

bool XAPKS::handleInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XAPKS> guardedThis(this);
    bool bResult = true;

    if (!isInternalInfoHandled()) {
        bResult = guardedThis->XAPK::handleInternalInfo(pPdStruct);
        if (!guardedThis || !bResult) return false;
        XAPK::INTERNAL_INFO *pInfo = static_cast<XAPK::INTERNAL_INFO *>(guardedThis->XAPK::getInternalInfo(pPdStruct));
        if (!guardedThis || !pInfo) return false;
        static_cast<XAPK::INTERNAL_INFO &>(guardedThis->m_internalInfo) = *pInfo;
    }

    return guardedThis && bResult;
}

void *XAPKS::getInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XAPKS> guardedThis(this);
    const bool bHandled = guardedThis->handleInternalInfo(pPdStruct);
    if (!guardedThis || !bHandled) return nullptr;

    return &guardedThis->m_internalInfo;
}

void XAPKS::setInternalInfo(void *pInternalInfo)
{
    if (pInternalInfo) {
        m_internalInfo = *static_cast<INTERNAL_INFO *>(pInternalInfo);
        XAPK::setInternalInfo(static_cast<XAPK::INTERNAL_INFO *>(&m_internalInfo));
    } else {
        m_internalInfo = INTERNAL_INFO();
        XAPK::setInternalInfo(nullptr);
    }
}
