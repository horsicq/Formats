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
#include "xapk.h"

namespace {
const qint64 APK_MANIFEST_LIMIT = 16LL * 1024 * 1024;

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

XAPK::XAPK(QIODevice *pDevice) : XJAR(pDevice)
{
}

bool XAPK::isValid(PDSTRUCT *pPdStruct)
{
    DevicePositionGuard positionGuard(getDevice());
    XZip xzip(getDevice());
    if (xzip.isValid(pPdStruct)) {
        QList<XArchive::RECORD> listArchiveRecords = xzip.getRecords(20000, pPdStruct);
        return isValid(getDevice(), &listArchiveRecords, pPdStruct);
    }
    return false;
}

bool XAPK::isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    XAPK xapk(pDevice);

    return xapk.isValid(pPdStruct);
}

bool XAPK::isValid(QList<RECORD> *pListRecords, PDSTRUCT *pPdStruct)
{
    if (!pListRecords) return false;

    // AndroidManifest.xml is mandatory for an APK, including resource-only
    // packages that legitimately have no classes.dex.  A filename alone is
    // not enough: an empty manifest is a common ZIP lookalike.
    for (qint32 i = 0; (i < pListRecords->count()) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        const RECORD &record = pListRecords->at(i);
        if ((record.spInfo.sRecordName == QLatin1String("AndroidManifest.xml")) && (record.spInfo.nUncompressedSize > 0)) {
            return true;
        }
    }

    return false;
}

bool XAPK::isValid(QIODevice *pDevice, QList<RECORD> *pListRecords, PDSTRUCT *pPdStruct)
{
    if (!pDevice || !isValid(pListRecords, pPdStruct)) return false;
    DevicePositionGuard positionGuard(pDevice);

    const RECORD record = XArchive::getArchiveRecord(QStringLiteral("AndroidManifest.xml"), pListRecords, pPdStruct);
    if ((record.spInfo.sRecordName != QLatin1String("AndroidManifest.xml")) || (record.spInfo.nUncompressedSize <= 0) ||
        (record.spInfo.nUncompressedSize > APK_MANIFEST_LIMIT) || (record.nDataSize <= 0) || (record.nDataSize > APK_MANIFEST_LIMIT) ||
        record.mapProperties.value(XBinary::FPART_PROP_ENCRYPTED, false).toBool() || (record.spInfo.compressMethod2 != XBinary::HANDLE_METHOD_UNKNOWN)) {
        return false;
    }

    XZip zip(pDevice);
    const QByteArray baManifest = zip.decompress(&record, pPdStruct, 0, APK_MANIFEST_LIMIT + 1);
    return baManifest.size() == record.spInfo.nUncompressedSize;
}

XBinary::FT XAPK::getFileType()
{
    return FT_APK;
}

XBinary::FILEFORMATINFO XAPK::getFileFormatInfo(PDSTRUCT *pPdStruct)
{
    // getRecords() is called at top level here, so unlike isValid() this needs
    // its own guard to leave the caller's cursor where it was.
    DevicePositionGuard positionGuard(getDevice());

    XBinary::FILEFORMATINFO result = {};

    QList<XArchive::RECORD> listArchiveRecords = getRecords(20000, pPdStruct);

    if (isValid(getDevice(), &listArchiveRecords, pPdStruct)) {
        result.bIsValid = true;
        result.nSize = getSize();
        result.sExt = "apk";
        result.fileType = FT_APK;

        result.osName = OSNAME_ANDROID;

        result.sArch = getArch();
        result.mode = getMode();
        result.sType = typeIdToString(getType());
        result.endian = getEndian();
        QByteArray baAndroidManifest = decompress(&listArchiveRecords, "AndroidManifest.xml", pPdStruct);

        if (baAndroidManifest.size() > 0) {
            QString sAndroidManifest = XAndroidBinary::getDecoded(&baAndroidManifest, pPdStruct);

            QString sCompileSdkVersion = XBinary::regExp("android:compileSdkVersion=\"(.*?)\"", sAndroidManifest, 1);
            QString sCompileSdkVersionCodename = XBinary::regExp("android:compileSdkVersionCodename=\"(.*?)\"", sAndroidManifest, 1);
            QString sPlatformBuildVersionCode = XBinary::regExp("platformBuildVersionCode=\"(.*?)\"", sAndroidManifest, 1);
            QString sPlatformBuildVersionName = XBinary::regExp("platformBuildVersionName=\"(.*?)\"", sAndroidManifest, 1);
            QString sTargetSdkVersion = XBinary::regExp("android:targetSdkVersion=\"(.*?)\"", sAndroidManifest, 1);
            QString sMinSdkVersion = XBinary::regExp("android:minSdkVersion=\"(.*?)\"", sAndroidManifest, 1);

            // Check
            if (!XBinary::checkStringNumber(sCompileSdkVersion, 1, 40)) sCompileSdkVersion = "";
            if (!XBinary::checkStringNumber(sPlatformBuildVersionCode, 1, 40)) sPlatformBuildVersionCode = "";
            if (!XBinary::checkStringNumber(sTargetSdkVersion, 1, 40)) sTargetSdkVersion = "";
            if (!XBinary::checkStringNumber(sMinSdkVersion, 1, 40)) sMinSdkVersion = "";

            if (!XBinary::checkStringNumber(sCompileSdkVersionCodename.section(".", 0, 0), 1, 15)) sCompileSdkVersionCodename = "";
            if (!XBinary::checkStringNumber(sPlatformBuildVersionName.section(".", 0, 0), 1, 15)) sPlatformBuildVersionName = "";

            if ((sCompileSdkVersion != "") || (sCompileSdkVersionCodename != "") || (sPlatformBuildVersionCode != "") || (sPlatformBuildVersionName != "") ||
                (sTargetSdkVersion != "") || (sMinSdkVersion != "")) {
                QString _sVersion;
                QString _sAndroidVersion;

                if (_sVersion == "") _sVersion = sTargetSdkVersion;
                if (_sVersion == "") _sVersion = sMinSdkVersion;
                if (_sVersion == "") _sVersion = sCompileSdkVersion;
                if (_sVersion == "") _sVersion = sPlatformBuildVersionCode;

                if (_sAndroidVersion == "") _sAndroidVersion = sCompileSdkVersionCodename;
                if (_sAndroidVersion == "") _sAndroidVersion = sPlatformBuildVersionName;

                if (_sAndroidVersion == "") {
                    _sAndroidVersion = XBinary::getAndroidVersionFromApi(_sVersion.toUInt());
                }

                result.sOsVersion = _sAndroidVersion;
                result.sOsBuild = sPlatformBuildVersionCode;
            }
        }
        // qint32 nNumberOfRecords = listArchiveRecords.count();

        // if (nNumberOfRecords < 20000) {
        //     result.nNumberOfRecords = nNumberOfRecords;
        // } else {
        //     result.nNumberOfRecords = getNumberOfRecords(pPdStruct);
        // }
    }

    return result;
}

QString XAPK::getFileFormatExt()
{
    return "apk";
}

XBinary::MODE XAPK::getMode()
{
    return MODE_DATA;
}

QString XAPK::getArch()
{
    return tr("Universal");
}

qint32 XAPK::getType()
{
    return TYPE_PACKAGE;
}

QString XAPK::typeIdToString(qint32 nType)
{
    QString sResult = tr("Unknown");

    switch (nType) {
        case TYPE_PACKAGE: sResult = tr("Package");
    }

    return sResult;
}

bool XAPK::isSigned()
{
    // TODO Check more !!!
    return isAPKSignBlockPresent();
}

XBinary::OFFSETSIZE XAPK::getSignOffsetSize()
{
    OFFSETSIZE osResult = {};

    // TODO optimize

    qint64 nOffset = findAPKSignBlockOffset();

    quint64 nBlockSize1 = read_uint64(nOffset - 8);
    quint64 nBlockSize2 = read_uint64(nOffset - nBlockSize1 + 8);

    if ((nBlockSize1) && (nBlockSize1 == nBlockSize2)) {
        nOffset = nOffset - nBlockSize1 + 16;

        qint64 nCentralDirectoryOffset = findECDOffset(nullptr);
        nCentralDirectoryOffset = read_uint32(nCentralDirectoryOffset + offsetof(ENDOFCENTRALDIRECTORYRECORD, nOffsetToCentralDirectory));

        osResult.nOffset = nOffset;
        osResult.nSize = qMax((qint64)0, nCentralDirectoryOffset - nOffset);
    }

    return osResult;
}

bool XAPK::isAPKSignBlockPresent()
{
    return (findAPKSignBlockOffset() != -1);
}

QList<XAPK::APK_SIG_BLOCK_RECORD> XAPK::getAPKSignaturesBlockRecordsList()
{
    QList<XAPK::APK_SIG_BLOCK_RECORD> listResult;

    qint64 nOffset = findAPKSignBlockOffset();

    if (nOffset != -1) {
        quint64 nBlockSize1 = read_uint64(nOffset - 8);
        quint64 nBlockSize2 = read_uint64(nOffset - nBlockSize1 + 8);

        if ((nBlockSize1) && (nBlockSize1 == nBlockSize2)) {
            qint64 nEndOffset = nOffset - 8;
            nOffset = nOffset - nBlockSize1 + 16;

            while (nOffset < nEndOffset) {
                APK_SIG_BLOCK_RECORD record = {};
                record.nID = read_uint32(nOffset);
                nOffset += 4;

                record.nDataOffset = nOffset + 4;
                record.nDataSize = read_uint32(nOffset);

                listResult.append(record);

                nOffset += 4;
                nOffset += record.nDataSize;

                if (record.nID == 0x42726577)  // End TODO CONST
                {
                    break;
                }

                if (record.nID == 0) {
                    break;
                }
            }
        }
    }

    return listResult;
}

bool XAPK::isAPKSignatureBlockRecordPresent(QList<APK_SIG_BLOCK_RECORD> *pList, quint32 nID)
{
    return (getAPKSignatureBlockRecord(pList, nID).nID == nID);
}

XAPK::APK_SIG_BLOCK_RECORD XAPK::getAPKSignatureBlockRecord(QList<APK_SIG_BLOCK_RECORD> *pList, quint32 nID)
{
    XAPK::APK_SIG_BLOCK_RECORD result = {};

    qint32 nNumberOfRecords = pList->count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (pList->at(i).nID == nID) {
            result = pList->at(i);

            break;
        }
    }

    return result;
}

qint64 XAPK::findAPKSignBlockOffset(PDSTRUCT *pPdStruct)
{
    qint64 nResult = -1;

    qint64 nOffset = findECDOffset(pPdStruct);
    nOffset = read_uint32(nOffset + offsetof(ENDOFCENTRALDIRECTORYRECORD, nOffsetToCentralDirectory));

    nOffset = qMax((qint64)0, nOffset - 0x100);  // TODO const

    while (true) {
        qint64 nCurrent = find_ansiString(nOffset, -1, "APK Sig Block 42", pPdStruct);

        if (nCurrent == -1) {
            break;
        }

        nResult = nCurrent;
        nOffset = nCurrent + 8;  // Get the last
    }

    return nResult;
}

bool XAPK::handleInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XAPK> guardedThis(this);
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

void *XAPK::getInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XAPK> guardedThis(this);
    const bool bHandled = guardedThis->handleInternalInfo(pPdStruct);
    if (!guardedThis || !bHandled) return nullptr;

    return &guardedThis->m_internalInfo;
}

void XAPK::setInternalInfo(void *pInternalInfo)
{
    if (pInternalInfo) {
        m_internalInfo = *static_cast<INTERNAL_INFO *>(pInternalInfo);
        XJAR::setInternalInfo(static_cast<XJAR::INTERNAL_INFO *>(&m_internalInfo));
    } else {
        m_internalInfo = INTERNAL_INFO();
        XJAR::setInternalInfo(nullptr);
    }
}
