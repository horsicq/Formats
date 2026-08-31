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
#include "xjar.h"

namespace {
const qint64 JAR_MANIFEST_LIMIT = 16LL * 1024 * 1024;

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
}  // namespace

XBinary::XCONVERT _TABLE_XJAR_STRUCTID[] = {
    {XJAR::STRUCTID_UNKNOWN, "Unknown", QObject::tr("Unknown")},
};

XJAR::XJAR(QIODevice *pDevice) : XZip(pDevice)
{
}

bool XJAR::isValid(PDSTRUCT *pPdStruct)
{
    DevicePositionGuard positionGuard(getDevice());
    XZip xzip(getDevice());

    if (xzip.isValid(pPdStruct)) {
        QList<XArchive::RECORD> listArchiveRecords = xzip.getRecords(20000, pPdStruct);
        return isValid(getDevice(), &listArchiveRecords, pPdStruct);
    }

    return false;
}

bool XJAR::isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    XJAR xjar(pDevice);

    return xjar.isValid(pPdStruct);
}

bool XJAR::isValid(QList<RECORD> *pListRecords, PDSTRUCT *pPdStruct)
{
    if (!pListRecords) return false;

    for (qint32 i = 0; (i < pListRecords->count()) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        const RECORD &record = pListRecords->at(i);
        if ((record.spInfo.sRecordName == QLatin1String("META-INF/MANIFEST.MF")) && (record.spInfo.nUncompressedSize > 0)) {
            return true;
        }
    }

    return false;
}

bool XJAR::isValid(QIODevice *pDevice, QList<RECORD> *pListRecords, PDSTRUCT *pPdStruct)
{
    if (!pDevice || !isValid(pListRecords, pPdStruct)) return false;
    DevicePositionGuard positionGuard(pDevice);

    const RECORD record = XArchive::getArchiveRecord(QStringLiteral("META-INF/MANIFEST.MF"), pListRecords, pPdStruct);
    if ((record.spInfo.sRecordName != QLatin1String("META-INF/MANIFEST.MF")) || (record.spInfo.nUncompressedSize <= 0) ||
        (record.spInfo.nUncompressedSize > JAR_MANIFEST_LIMIT) || (record.nDataSize <= 0) || (record.nDataSize > JAR_MANIFEST_LIMIT) ||
        record.mapProperties.value(XBinary::FPART_PROP_ENCRYPTED, false).toBool() || (record.spInfo.compressMethod2 != XBinary::HANDLE_METHOD_UNKNOWN)) {
        return false;
    }

    XZip zip(pDevice);
    const QByteArray baManifest = zip.decompress(&record, pPdStruct, 0, JAR_MANIFEST_LIMIT + 1);
    return baManifest.size() == record.spInfo.nUncompressedSize;
}

XBinary::FT XJAR::getFileType()
{
    return FT_JAR;
}

XBinary::FILEFORMATINFO XJAR::getFileFormatInfo(PDSTRUCT *pPdStruct)
{
    // getRecords() is called at top level here, so unlike isValid() this needs
    // its own guard to leave the caller's cursor where it was.
    DevicePositionGuard positionGuard(getDevice());

    XBinary::FILEFORMATINFO result = {};

    QList<XArchive::RECORD> listArchiveRecords = getRecords(20000, pPdStruct);

    if (isValid(getDevice(), &listArchiveRecords, pPdStruct)) {
        result.bIsValid = true;
        result.nSize = getSize();
        result.sExt = "jar";
        result.fileType = FT_JAR;

        result.osName = OSNAME_JVM;
        result.bIsVM = true;

        result.sArch = getArch();
        result.mode = getMode();
        result.sType = typeIdToString(getType());
        result.endian = getEndian();

        qint32 nNumberOfRecords = listArchiveRecords.count();

        for (qint32 i = 0; i < nNumberOfRecords; i++) {
            if (listArchiveRecords.at(i).spInfo.sRecordName.section(".", -1, -1) == "class") {
                RECORD record = listArchiveRecords.at(i);
                QByteArray baData = XArchive::decompress(&record, pPdStruct, 0, 0x100);

                if (baData.size() > 10) {
                    char *pData = baData.data();
                    if (XBinary::_read_uint32(pData, true) == 0xCAFEBABE) {
                        quint16 nMinor = XBinary::_read_uint16(pData + 4, true);
                        quint16 nMajor = XBinary::_read_uint16(pData + 6, true);

                        result.sOsVersion = XJavaClass::_getJDKVersion(nMajor, nMinor);

                        break;
                    }
                }
            }
        }

        // if (nNumberOfRecords < 20000) {
        //     result.nNumberOfRecords = nNumberOfRecords;
        // } else {
        //     result.nNumberOfRecords = getNumberOfRecords(pPdStruct);
        // }
    }

    return result;
}

QString XJAR::getFileFormatExt()
{
    return "jar";
}

XBinary::ENDIAN XJAR::getEndian()
{
    return ENDIAN_UNKNOWN;
}

XBinary::MODE XJAR::getMode()
{
    return MODE_DATA;
}

QString XJAR::getArch()
{
    return tr("Universal");
}

qint32 XJAR::getType()
{
    return TYPE_PACKAGE;
}

QString XJAR::typeIdToString(qint32 nType)
{
    QString sResult = tr("Unknown");

    switch (nType) {
        case TYPE_PACKAGE: sResult = tr("Package");
    }

    return sResult;
}

QString XJAR::structIDToString(quint32 nID)
{
    return XBinary::XCONVERT_idToTransString(nID, _TABLE_XJAR_STRUCTID, sizeof(_TABLE_XJAR_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QString XJAR::structIDToFtString(quint32 nID)
{
    return XBinary::XCONVERT_idToFtString(nID, _TABLE_XJAR_STRUCTID, sizeof(_TABLE_XJAR_STRUCTID) / sizeof(XBinary::XCONVERT));
}

quint32 XJAR::ftStringToStructID(const QString &sFtString)
{
    return XCONVERT_ftStringToId(sFtString, _TABLE_XJAR_STRUCTID, sizeof(_TABLE_XJAR_STRUCTID) / sizeof(XBinary::XCONVERT));
}

bool XJAR::handleInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XJAR> guardedThis(this);
    bool bResult = true;

    if (!isInternalInfoHandled()) {
        bResult = guardedThis->XZip::handleInternalInfo(pPdStruct);
        if (!guardedThis || !bResult) return false;
        XZip::INTERNAL_INFO *pInfo = static_cast<XZip::INTERNAL_INFO *>(guardedThis->XZip::getInternalInfo(pPdStruct));
        if (!guardedThis || !pInfo) return false;
        static_cast<XZip::INTERNAL_INFO &>(guardedThis->m_internalInfo) = *pInfo;
    }

    return guardedThis && bResult;
}

void *XJAR::getInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XJAR> guardedThis(this);
    const bool bHandled = guardedThis->handleInternalInfo(pPdStruct);
    if (!guardedThis || !bHandled) return nullptr;

    return &guardedThis->m_internalInfo;
}

void XJAR::setInternalInfo(void *pInternalInfo)
{
    if (pInternalInfo) {
        m_internalInfo = *static_cast<INTERNAL_INFO *>(pInternalInfo);
        XZip::setInternalInfo(static_cast<XZip::INTERNAL_INFO *>(&m_internalInfo));
    } else {
        m_internalInfo = INTERNAL_INFO();
        XZip::setInternalInfo(nullptr);
    }
}
