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
#include "xipa.h"

namespace {
const qint64 IPA_INFO_LIMIT = 16LL * 1024 * 1024;

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

bool isInfoPlistRecord(const XArchive::RECORD &record)
{
    QString sName = record.spInfo.sRecordName;
    sName.replace(QLatin1Char('\\'), QLatin1Char('/'));

    const QString sPrefix = QStringLiteral("Payload/");
    const QString sSuffix = QStringLiteral("/Info.plist");
    if (!sName.startsWith(sPrefix) || !sName.endsWith(sSuffix) || (record.spInfo.nUncompressedSize <= 0)) {
        return false;
    }

    const QString sApplication = sName.mid(sPrefix.size(), sName.size() - sPrefix.size() - sSuffix.size());
    return !sApplication.contains(QLatin1Char('/')) && (sApplication.size() > 4) && sApplication.endsWith(QLatin1String(".app"));
}
}  // namespace

XIPA::XIPA(QIODevice *pDevice) : XJAR(pDevice)
{
}

bool XIPA::isValid(PDSTRUCT *pPdStruct)
{
    DevicePositionGuard positionGuard(getDevice());
    bool bResult = false;

    XZip xzip(getDevice());
    if (xzip.isValid(pPdStruct)) {
        QList<XArchive::RECORD> listArchiveRecords = xzip.getRecords(20000, pPdStruct);
        bResult = isValid(getDevice(), &listArchiveRecords, pPdStruct);
    }

    return bResult;
}

bool XIPA::isValid(QList<RECORD> *pListRecords, PDSTRUCT *pPdStruct)
{
    if (!pListRecords) return false;

    for (qint32 i = 0; (i < pListRecords->count()) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        const RECORD &record = pListRecords->at(i);
        if (isInfoPlistRecord(record)) return true;
    }

    return false;
}

// QString XIPA::getMIMEString()
// {

// }

bool XIPA::isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    XIPA xipa(pDevice);

    return xipa.isValid(pPdStruct);
}

bool XIPA::isValid(QIODevice *pDevice, QList<RECORD> *pListRecords, PDSTRUCT *pPdStruct)
{
    if (!pDevice || !pListRecords) return false;
    DevicePositionGuard positionGuard(pDevice);
    XZip zip(pDevice);

    for (qint32 i = 0; (i < pListRecords->count()) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        const RECORD &record = pListRecords->at(i);
        if (!isInfoPlistRecord(record)) continue;
        if ((record.spInfo.nUncompressedSize > IPA_INFO_LIMIT) || (record.nDataSize <= 0) || (record.nDataSize > IPA_INFO_LIMIT) ||
            record.mapProperties.value(XBinary::FPART_PROP_ENCRYPTED, false).toBool() || (record.spInfo.compressMethod2 != XBinary::HANDLE_METHOD_UNKNOWN)) {
            continue;
        }

        const QByteArray baInfo = zip.decompress(&record, pPdStruct, 0, IPA_INFO_LIMIT + 1);
        if (baInfo.size() == record.spInfo.nUncompressedSize) return true;
    }

    return false;
}

XBinary::FT XIPA::getFileType()
{
    return FT_IPA;
}

XBinary::FILEFORMATINFO XIPA::getFileFormatInfo(PDSTRUCT *pPdStruct)
{
    // getRecords() is called at top level here, so unlike isValid() this needs
    // its own guard to leave the caller's cursor where it was.
    DevicePositionGuard positionGuard(getDevice());

    XBinary::FILEFORMATINFO result = {};

    QList<XArchive::RECORD> listArchiveRecords = getRecords(20000, pPdStruct);
    if (isValid(getDevice(), &listArchiveRecords, pPdStruct)) {
        result.bIsValid = true;
        result.nSize = getSize();
        result.sExt = getFileFormatExt();
        result.fileType = FT_IPA;
        result.osName = OSNAME_IOS;
        result.sArch = getArch();
        result.mode = getMode();
        result.sType = typeIdToString(getType());
        result.endian = getEndian();
    }

    return result;
}

QString XIPA::getFileFormatExt()
{
    return QStringLiteral("ipa");
}

bool XIPA::handleInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XIPA> guardedThis(this);
    bool bResult = true;

    if (!isInternalInfoHandled()) {
        bResult = guardedThis->XJAR::handleInternalInfo(pPdStruct);
        if (!guardedThis || !bResult) return false;
        XJAR::INTERNAL_INFO *pInfo = static_cast<XJAR::INTERNAL_INFO *>(guardedThis->XJAR::getInternalInfo(pPdStruct));
        if (!guardedThis || !pInfo) return false;
        static_cast<XJAR::INTERNAL_INFO &>(guardedThis->m_internalInfo) = *pInfo;
    }

    return guardedThis && bResult;
}

void *XIPA::getInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XIPA> guardedThis(this);
    const bool bHandled = guardedThis->handleInternalInfo(pPdStruct);
    if (!guardedThis || !bHandled) return nullptr;

    return &guardedThis->m_internalInfo;
}

void XIPA::setInternalInfo(void *pInternalInfo)
{
    if (pInternalInfo) {
        m_internalInfo = *static_cast<INTERNAL_INFO *>(pInternalInfo);
        XJAR::setInternalInfo(static_cast<XJAR::INTERNAL_INFO *>(&m_internalInfo));
    } else {
        m_internalInfo = INTERNAL_INFO();
        XJAR::setInternalInfo(nullptr);
    }
}
