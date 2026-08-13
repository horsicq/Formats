/* Copyright (c) 2017-2026 hors<horsicq@gmail.com>
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
#include "xbinary.h"
#include "subdevice.h"
#include <algorithm>
#include <cstring>
#include <limits>
#include <QDebug>
#include <QFileDevice>
#include <QPointer>
#include <QSaveFile>
#include <QWaitCondition>
#ifdef Q_OS_WIN
#include <io.h>
#include <windows.h>
#elif defined(Q_OS_UNIX)
#include <sys/stat.h>
#ifdef Q_OS_MACOS
#include <fcntl.h>
#include <limits.h>
#endif
#endif

struct XBinary::PDSTRUCT_CALLBACK_STATE {
    struct ENTRY {
        PDSTRUCT_CALLBACK pCallback = nullptr;
        void *pUserData = nullptr;
        qint32 nInFlight = 0;
        bool bActive = true;
    };

    QMutex mutex;
    QWaitCondition drained;
    QMap<quint64, QSharedPointer<ENTRY>> mapSubscribers;
    QSharedPointer<ENTRY> pLegacyEntry;
    quint64 nNextToken = 1;
    qint32 nInvocations = 0;
    bool bInvoking = false;
    bool bDestroying = false;
};

namespace {
thread_local QList<const XBinary::PDSTRUCT_CALLBACK_STATE *> g_listActivePdCallbackStates;
thread_local QList<const XBinary::PDSTRUCT_CALLBACK_STATE::ENTRY *> g_listActivePdCallbackEntries;

bool isCurrentPdCallbackState(const XBinary::PDSTRUCT_CALLBACK_STATE *pState)
{
    return g_listActivePdCallbackStates.contains(pState);
}

bool isCurrentPdCallbackEntry(const XBinary::PDSTRUCT_CALLBACK_STATE::ENTRY *pEntry)
{
    return g_listActivePdCallbackEntries.contains(pEntry);
}

bool isPdCallbackOwnerAlive(const QSharedPointer<XBinary::PDSTRUCT_CALLBACK_STATE> &pState)
{
    if (pState.isNull()) return false;
    QMutexLocker locker(&pState->mutex);
    return !pState->bDestroying;
}

class PD_CALLBACK_INVOCATION_GUARD {
public:
    explicit PD_CALLBACK_INVOCATION_GUARD(const QSharedPointer<XBinary::PDSTRUCT_CALLBACK_STATE> &pState) : m_pState(pState)
    {
        g_listActivePdCallbackStates.append(m_pState.data());
    }

    ~PD_CALLBACK_INVOCATION_GUARD()
    {
        const qint32 nIndex = g_listActivePdCallbackStates.lastIndexOf(m_pState.data());
        if (nIndex >= 0) g_listActivePdCallbackStates.removeAt(nIndex);

        QMutexLocker locker(&m_pState->mutex);
        if (m_pState->nInvocations > 0) m_pState->nInvocations--;
        m_pState->bInvoking = false;
        m_pState->drained.wakeAll();
    }

private:
    QSharedPointer<XBinary::PDSTRUCT_CALLBACK_STATE> m_pState;
};

class PD_CALLBACK_ENTRY_GUARD {
public:
    PD_CALLBACK_ENTRY_GUARD(const QSharedPointer<XBinary::PDSTRUCT_CALLBACK_STATE> &pState,
                            const QSharedPointer<XBinary::PDSTRUCT_CALLBACK_STATE::ENTRY> &pEntry)
        : m_pState(pState), m_pEntry(pEntry)
    {
        g_listActivePdCallbackEntries.append(m_pEntry.data());
    }

    ~PD_CALLBACK_ENTRY_GUARD()
    {
        const qint32 nIndex = g_listActivePdCallbackEntries.lastIndexOf(m_pEntry.data());
        if (nIndex >= 0) g_listActivePdCallbackEntries.removeAt(nIndex);

        QMutexLocker locker(&m_pState->mutex);
        if (m_pEntry->nInFlight > 0) m_pEntry->nInFlight--;
        m_pState->drained.wakeAll();
    }

private:
    QSharedPointer<XBinary::PDSTRUCT_CALLBACK_STATE> m_pState;
    QSharedPointer<XBinary::PDSTRUCT_CALLBACK_STATE::ENTRY> m_pEntry;
};
}  // namespace

bool compareMemoryMapRecord(const XBinary::_MEMORY_RECORD &a, const XBinary::_MEMORY_RECORD &b)
{
    if (a.nAddress != b.nAddress) {
        return a.nAddress < b.nAddress;
    } else {
        return a.nOffset < b.nOffset;
    }
}

bool compareFileParts(const XBinary::FPART &a, const XBinary::FPART &b)
{
    if ((a.nVirtualAddress != -1) && (b.nVirtualAddress != -1)) {
        return a.nVirtualAddress < b.nVirtualAddress;
    } else {
        return a.nFileOffset < b.nFileOffset;
    }
}

bool compareMS_RECORD(const XBinary::MS_RECORD &a, const XBinary::MS_RECORD &b)
{
    if (a.nRegionIndex != b.nRegionIndex) {
        return a.nRegionIndex < b.nRegionIndex;
    } else if (a.nRelOffset != b.nRelOffset) {
        return a.nRelOffset < b.nRelOffset;
    } else if (a.nValueType != b.nValueType) {
        return a.nValueType < b.nValueType;
    } else if (a.nInfo != b.nInfo) {
        return a.nInfo < b.nInfo;
    } else {
        return a.nSize < b.nSize;
    }
}

static bool writeAllToDevice(QIODevice *pDevice, const char *pData, qint64 nSize)
{
    if (!pDevice || (nSize < 0) || ((nSize > 0) && !pData)) {
        return false;
    }

    QPointer<QIODevice> guardedDevice(pDevice);
    if (!guardedDevice || !guardedDevice->isWritable() || !guardedDevice) {
        return false;
    }
    const bool bSeekable = !guardedDevice->isSequential();
    if (!guardedDevice) return false;
    const qint64 nStart = bSeekable ? guardedDevice->pos() : -1;
    const qint64 nMax = (std::numeric_limits<qint64>::max)();
    if (!guardedDevice || (bSeekable && (nStart < 0))) return false;

    qint64 nWrittenTotal = 0;
    while (nWrittenTotal < nSize) {
        if (!guardedDevice ||
            (bSeekable && (nWrittenTotal > nMax - nStart))) return false;
        if (bSeekable) {
            const bool bSeeked = guardedDevice->seek(
                nStart + nWrittenTotal);
            if (!guardedDevice || !bSeeked) return false;
        }
        const qint64 nWritten = guardedDevice->write(
            pData + nWrittenTotal, nSize - nWrittenTotal);
        if (!guardedDevice) return false;
        if ((nWritten <= 0) || (nWritten > (nSize - nWrittenTotal))) {
            return false;
        }
        nWrittenTotal += nWritten;
    }

    if (!guardedDevice) return false;
    if (!bSeekable) return true;
    if (nSize > nMax - nStart) return false;
    const bool bSeeked = guardedDevice->seek(nStart + nSize);
    return guardedDevice && bSeeked;
}

static Qt::CaseSensitivity fileSystemPathCaseSensitivity()
{
#ifdef Q_OS_WIN
    return Qt::CaseInsensitive;
#else
    return Qt::CaseSensitive;
#endif
}

static bool isDirectCanonicalPath(const QFileInfo &fileInfo)
{
    if (!fileInfo.exists()) return false;

    const QString sAbsolutePath = QDir::fromNativeSeparators(QDir::cleanPath(fileInfo.absoluteFilePath()));
    const QString sCanonicalPath = QDir::fromNativeSeparators(QDir::cleanPath(fileInfo.canonicalFilePath()));

    return !sCanonicalPath.isEmpty() &&
           (sAbsolutePath.compare(sCanonicalPath, fileSystemPathCaseSensitivity()) == 0);
}

enum FILE_IDENTITY_RESULT {
    FILE_IDENTITY_UNKNOWN = 0,
    FILE_IDENTITY_SAME,
    FILE_IDENTITY_DISTINCT
};

static QString getOpenQFilePath(const QFileDevice *pFile)
{
    if (!pFile || !pFile->isOpen() || (pFile->handle() < 0)) return QString();

#ifdef Q_OS_WIN
    const intptr_t nHandle = _get_osfhandle(pFile->handle());
    if (nHandle == -1) return QString();

    const HANDLE hFile = reinterpret_cast<HANDLE>(nHandle);
    const DWORD nLength = GetFinalPathNameByHandleW(hFile, nullptr, 0, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
    if (!nLength || (nLength > (DWORD)(std::numeric_limits<qint32>::max)() - 1)) return QString();

    QVector<wchar_t> buffer((qint32)nLength + 1);
    const DWORD nWritten = GetFinalPathNameByHandleW(hFile, buffer.data(), (DWORD)buffer.size(), FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
    if (!nWritten || (nWritten >= (DWORD)buffer.size())) return QString();

    QString sResult = QString::fromWCharArray(buffer.constData(), (qint32)nWritten);
    if (sResult.startsWith(QLatin1String("\\\\?\\UNC\\"), Qt::CaseInsensitive)) {
        sResult = QLatin1String("\\\\") + sResult.mid(8);
    } else if (sResult.startsWith(QLatin1String("\\\\?\\"), Qt::CaseInsensitive)) {
        sResult.remove(0, 4);
    }

    return QDir::cleanPath(QDir::fromNativeSeparators(sResult));
#elif defined(Q_OS_LINUX)
    const QString sResult = QFileInfo(QStringLiteral("/proc/self/fd/%1").arg(pFile->handle())).symLinkTarget();
    return sResult.endsWith(QLatin1String(" (deleted)")) ? QString() : QDir::cleanPath(sResult);
#elif defined(Q_OS_MACOS)
    char szPath[PATH_MAX] = {};
    if (fcntl(pFile->handle(), F_GETPATH, szPath) == -1) return QString();
    return QDir::cleanPath(QFile::decodeName(szPath));
#else
    return QString();
#endif
}

static FILE_IDENTITY_RESULT compareOpenFileIdentity(const QFileDevice *pFirst, const QFileDevice *pSecond)
{
    if (pFirst == pSecond) return pFirst ? FILE_IDENTITY_SAME : FILE_IDENTITY_UNKNOWN;
    if (!pFirst || !pSecond || !pFirst->isOpen() || !pSecond->isOpen() ||
        (pFirst->handle() < 0) || (pSecond->handle() < 0)) {
        return FILE_IDENTITY_UNKNOWN;
    }

#ifdef Q_OS_WIN
    const intptr_t nFirstHandle = _get_osfhandle(pFirst->handle());
    const intptr_t nSecondHandle = _get_osfhandle(pSecond->handle());
    if ((nFirstHandle == -1) || (nSecondHandle == -1)) return FILE_IDENTITY_UNKNOWN;

    BY_HANDLE_FILE_INFORMATION firstInformation = {};
    BY_HANDLE_FILE_INFORMATION secondInformation = {};
    if (!GetFileInformationByHandle(reinterpret_cast<HANDLE>(nFirstHandle), &firstInformation) ||
        !GetFileInformationByHandle(reinterpret_cast<HANDLE>(nSecondHandle), &secondInformation)) {
        return FILE_IDENTITY_UNKNOWN;
    }

    return ((firstInformation.dwVolumeSerialNumber == secondInformation.dwVolumeSerialNumber) &&
            (firstInformation.nFileIndexHigh == secondInformation.nFileIndexHigh) &&
            (firstInformation.nFileIndexLow == secondInformation.nFileIndexLow))
               ? FILE_IDENTITY_SAME
               : FILE_IDENTITY_DISTINCT;
#elif defined(Q_OS_UNIX)
    struct stat firstStatus = {};
    struct stat secondStatus = {};
    if ((fstat(pFirst->handle(), &firstStatus) != 0) || (fstat(pSecond->handle(), &secondStatus) != 0)) return FILE_IDENTITY_UNKNOWN;

    return ((firstStatus.st_dev == secondStatus.st_dev) && (firstStatus.st_ino == secondStatus.st_ino))
               ? FILE_IDENTITY_SAME
               : FILE_IDENTITY_DISTINCT;
#else
    return FILE_IDENTITY_UNKNOWN;
#endif
}

static FILE_IDENTITY_RESULT compareNamedFileIdentity(const QString &sFirstFileName, const QString &sSecondFileName)
{
    if (sFirstFileName.isEmpty() || sSecondFileName.isEmpty()) return FILE_IDENTITY_UNKNOWN;

    QFile firstFile(sFirstFileName);
    QFile secondFile(sSecondFileName);
    if (!firstFile.open(QIODevice::ReadOnly) || !secondFile.open(QIODevice::ReadOnly)) return FILE_IDENTITY_UNKNOWN;

    return compareOpenFileIdentity(&firstFile, &secondFile);
}

static FILE_IDENTITY_RESULT compareFileDeviceIdentity(const QFileDevice *pFirst, const QFileDevice *pSecond)
{
    FILE_IDENTITY_RESULT result = compareOpenFileIdentity(pFirst, pSecond);
    if (result != FILE_IDENTITY_UNKNOWN) return result;
    if (!pFirst || !pSecond) return FILE_IDENTITY_UNKNOWN;

    const QString sFirstOpenPath = getOpenQFilePath(pFirst);
    const QString sSecondOpenPath = getOpenQFilePath(pSecond);
    const QString sFirstFileName = sFirstOpenPath.isEmpty() ? pFirst->fileName() : sFirstOpenPath;
    const QString sSecondFileName = sSecondOpenPath.isEmpty() ? pSecond->fileName() : sSecondOpenPath;
    const QString sFirstCanonicalPath = QDir::fromNativeSeparators(QFileInfo(sFirstFileName).canonicalFilePath());
    const QString sSecondCanonicalPath = QDir::fromNativeSeparators(QFileInfo(sSecondFileName).canonicalFilePath());

    if (!sFirstCanonicalPath.isEmpty() && !sSecondCanonicalPath.isEmpty() &&
        (QDir::cleanPath(sFirstCanonicalPath).compare(QDir::cleanPath(sSecondCanonicalPath), fileSystemPathCaseSensitivity()) == 0)) {
        return FILE_IDENTITY_SAME;
    }

    result = compareNamedFileIdentity(sFirstFileName, sSecondFileName);
    return result;
}

struct DEVICE_ROOT_VIEW {
    QIODevice *pRoot;
    qint64 nBaseOffset;
    bool bValid;
};

static DEVICE_ROOT_VIEW getDeviceRootView(QIODevice *pDevice)
{
    DEVICE_ROOT_VIEW result = {};
    result.pRoot = pDevice;
    QSet<QIODevice *> stVisited;
    const qint64 nMax = (std::numeric_limits<qint64>::max)();

    while (result.pRoot) {
        if (stVisited.contains(result.pRoot)) return DEVICE_ROOT_VIEW();
        stVisited.insert(result.pRoot);

        SubDevice *pSubDevice = dynamic_cast<SubDevice *>(result.pRoot);
        if (!pSubDevice) {
            result.bValid = true;
            return result;
        }

        const quint64 nInitLocation = pSubDevice->getInitLocation();
        if ((nInitLocation > (quint64)nMax) ||
            (result.nBaseOffset > nMax - (qint64)nInitLocation)) {
            return DEVICE_ROOT_VIEW();
        }
        result.nBaseOffset += (qint64)nInitLocation;
        result.pRoot = pSubDevice->getOrigDevice();
    }

    return DEVICE_ROOT_VIEW();
}

static bool areDistinctOpenFiles(QFile *pFirst, QFile *pSecond)
{
    return compareOpenFileIdentity(pFirst, pSecond) == FILE_IDENTITY_DISTINCT;
}

static bool areDistinctRegularFiles(const QString &sFirstFileName, const QString &sSecondFileName)
{
    QFile firstFile(sFirstFileName);
    QFile secondFile(sSecondFileName);
    if (!firstFile.open(QIODevice::ReadOnly) || !secondFile.open(QIODevice::ReadOnly)) return false;

    return areDistinctOpenFiles(&firstFile, &secondFile);
}

static bool _retainBestMSRecords(QVector<XBinary::MS_RECORD> *pListRecords, qint32 nLimit)
{
    if ((!pListRecords) || (nLimit <= 0)) {
        return false;
    }

    const bool bLimitReached = pListRecords->size() > nLimit;
    const qint64 nTrimThreshold =
        qMin((qint64)(std::numeric_limits<qint32>::max)(), (qint64)nLimit * 2);

    if ((pListRecords->size() > nLimit) && (pListRecords->size() >= nTrimThreshold)) {
        std::sort(pListRecords->begin(), pListRecords->end(), compareMS_RECORD);
        pListRecords->resize(nLimit);
    }

    return bLimitReached;
}

static void _finalizeBestMSRecords(QVector<XBinary::MS_RECORD> *pListRecords, qint32 nLimit)
{
    if (!pListRecords) {
        return;
    }

    if (pListRecords->size() > 1) {
        std::sort(pListRecords->begin(), pListRecords->end(), compareMS_RECORD);
    }

    if ((nLimit > 0) && (pListRecords->size() > nLimit)) {
        pListRecords->resize(nLimit);
    }
}

static void _setMSRecordLocation(XBinary::MS_RECORD *pRecord, XBinary::_MEMORY_MAP *pMemoryMap, qint64 nOffset)
{
    pRecord->nRegionIndex = -1;
    pRecord->nRelOffset = nOffset;

    if (!pMemoryMap) {
        return;
    }

    const qint32 nRegionIndex = XBinary::getMemoryIndexByOffset(pMemoryMap, nOffset);

    if ((nRegionIndex >= 0) && (nRegionIndex <= (std::numeric_limits<qint16>::max)())) {
        pRecord->nRegionIndex = (qint16)nRegionIndex;
        pRecord->nRelOffset = nOffset - pMemoryMap->listRecords.at(nRegionIndex).nOffset;
    }
}

static bool _setMSRecordSize(XBinary::MS_RECORD *pRecord, qint64 nSize, XBinary::VT valueType, bool bAllowTruncate)
{
    qint32 nUnitSize = 1;

    if ((valueType == XBinary::VT_U) || (valueType == XBinary::VT_U_I) || (valueType == XBinary::VT_WORD_ARRAY)) {
        nUnitSize = 2;
    } else if ((valueType == XBinary::VT_UTF32) || (valueType == XBinary::VT_DWORD_ARRAY)) {
        nUnitSize = 4;
    }

    const qint64 nMaximumSize = (0xFFFF / nUnitSize) * nUnitSize;

    if ((nSize <= 0) || ((!bAllowTruncate) && (nSize > nMaximumSize))) {
        return false;
    }

    nSize = qMin(nSize, nMaximumSize);
    nSize -= nSize % nUnitSize;

    if (nSize <= 0) {
        return false;
    }

    pRecord->nSize = (quint16)nSize;

    return true;
}

static bool _containsMemoryOffset(const XBinary::_MEMORY_RECORD &record, qint64 nOffset)
{
    return (record.nSize > 0) && (record.nOffset >= 0) && (nOffset >= record.nOffset) && ((nOffset - record.nOffset) < record.nSize);
}

static bool _containsMemoryAddress(const XBinary::_MEMORY_RECORD &record, XADDR nAddress)
{
    return (record.nSize > 0) && (record.nAddress != (XADDR)-1) && (nAddress != (XADDR)-1) && (nAddress >= record.nAddress) &&
           ((nAddress - record.nAddress) < (quint64)record.nSize);
}

static bool _addRelativeAddress(XADDR nBaseAddress, qint64 nRelativeAddress, XADDR *pnAddress)
{
    if (!pnAddress || (nBaseAddress == (XADDR)-1) || (nRelativeAddress < 0)) {
        return false;
    }

    const quint64 nRelative = (quint64)nRelativeAddress;
    const quint64 nAvailable = (std::numeric_limits<XADDR>::max)() - nBaseAddress;

    if (nRelative >= nAvailable) {
        return false;
    }

    *pnAddress = nBaseAddress + nRelative;
    return true;
}

static bool _addRelativeOffset(qint64 nBaseOffset, quint64 nRelativeOffset, qint64 *pnOffset)
{
    if (!pnOffset || (nBaseOffset < 0) || (nRelativeOffset > (quint64)((std::numeric_limits<qint64>::max)() - nBaseOffset))) {
        return false;
    }

    *pnOffset = nBaseOffset + (qint64)nRelativeOffset;
    return true;
}

static bool _addSignedOffset(qint64 nBaseOffset, qint64 nDelta, qint64 *pnOffset)
{
    if (!pnOffset) {
        return false;
    }

    if ((nDelta > 0) && (nBaseOffset > (std::numeric_limits<qint64>::max)() - nDelta)) {
        return false;
    }
    if ((nDelta < 0) && (nBaseOffset < (std::numeric_limits<qint64>::min)() - nDelta)) {
        return false;
    }

    *pnOffset = nBaseOffset + nDelta;
    return true;
}

static bool _addSignedAddress(XADDR nBaseAddress, qint64 nDelta, XADDR *pnAddress)
{
    if (!pnAddress || (nBaseAddress == (XADDR)-1)) {
        return false;
    }

    if (nDelta >= 0) {
        return _addRelativeAddress(nBaseAddress, nDelta, pnAddress);
    }

    const quint64 nMagnitude = (quint64)(-(nDelta + 1)) + 1;
    if (nMagnitude > nBaseAddress) {
        return false;
    }

    *pnAddress = nBaseAddress - nMagnitude;
    return true;
}

const quint32 _crc32_EDB88320_tab[] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b,
    0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856, 0x646ba8c0,
    0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd,
    0xa50ab56b, 0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
    0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190,
    0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433, 0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
    0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea,
    0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65, 0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
    0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525,
    0x206f85b3, 0xb966d409, 0xce61e49f, 0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6,
    0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27,
    0x7d079eb1, 0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b, 0xd80d2bda,
    0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703,
    0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c,
    0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
    0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777, 0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff,
    0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9, 0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729,
    0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d};

const quint16 _crc16_tab[] = {
    0x0000, 0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241, 0xc601, 0x06c0, 0x0780, 0xc741, 0x0500, 0xc5c1, 0xc481, 0x0440, 0xcc01, 0x0cc0, 0x0d80, 0xcd41,
    0x0f00, 0xcfc1, 0xce81, 0x0e40, 0x0a00, 0xcac1, 0xcb81, 0x0b40, 0xc901, 0x09c0, 0x0880, 0xc841, 0xd801, 0x18c0, 0x1980, 0xd941, 0x1b00, 0xdbc1, 0xda81, 0x1a40,
    0x1e00, 0xdec1, 0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41, 0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641, 0xd201, 0x12c0, 0x1380, 0xd341,
    0x1100, 0xd1c1, 0xd081, 0x1040, 0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240, 0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441,
    0x3c00, 0xfcc1, 0xfd81, 0x3d40, 0xff01, 0x3fc0, 0x3e80, 0xfe41, 0xfa01, 0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840, 0x2800, 0xe8c1, 0xe981, 0x2940,
    0xeb01, 0x2bc0, 0x2a80, 0xea41, 0xee01, 0x2ec0, 0x2f80, 0xef41, 0x2d00, 0xedc1, 0xec81, 0x2c40, 0xe401, 0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640,
    0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0, 0x2080, 0xe041, 0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281, 0x6240, 0x6600, 0xa6c1, 0xa781, 0x6740,
    0xa501, 0x65c0, 0x6480, 0xa441, 0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41, 0xaa01, 0x6ac0, 0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840,
    0x7800, 0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41, 0xbe01, 0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40, 0xb401, 0x74c0, 0x7580, 0xb541,
    0x7700, 0xb7c1, 0xb681, 0x7640, 0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041, 0x5000, 0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0, 0x5280, 0x9241,
    0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481, 0x5440, 0x9c01, 0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40, 0x5a00, 0x9ac1, 0x9b81, 0x5b40,
    0x9901, 0x59c0, 0x5880, 0x9841, 0x8801, 0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40, 0x4e00, 0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41,
    0x4400, 0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641, 0x8201, 0x42c0, 0x4380, 0x8341, 0x4100, 0x81c1, 0x8081, 0x4040};

// Alphanumeric lookup table for signature matching (0-9, A-Z, a-z)
const bool g_alphaNumTable[256] = {
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  false, false, false, false, false, false,  // 0-9
    false, true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,                               // A-O
    true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  false, false, false, false, false,                              // P-Z
    false, true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,                               // a-o
    true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  false, false, false, false, false,                              // p-z
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};

XBinary::XCONVERT _TABLE_XBINARY_STRUCTID[] = {
    {XBinary::STRUCTID_UNKNOWN, "Unknown", QObject::tr("Unknown")},
    {XBinary::STRUCTID_NFDSCAN, "nfd", QString("Nauz File Detector")},
    {XBinary::STRUCTID_DIESCAN, "die", QString("Detect It Easy")},
    {XBinary::STRUCTID_YARASCAN, "yara", QString("Yara")},
    {XBinary::STRUCTID_VIRUSTOTALSCAN, "virustotal", QString("VirusTotal")},
    {XBinary::STRUCTID_VISUALIZATION, "visualization", QObject::tr("Visualization")},
    {XBinary::STRUCTID_HEX, "hex", QObject::tr("Hex")},
    {XBinary::STRUCTID_DISASM, "disasm", QObject::tr("Disasm")},
    {XBinary::STRUCTID_ARCHIVE, "archive", QObject::tr("Archive")},
    {XBinary::STRUCTID_HASH, "hash", QObject::tr("Hash")},
    {XBinary::STRUCTID_STRINGS, "strings", QObject::tr("Strings")},
    {XBinary::STRUCTID_SIGNATURES, "signatures", QObject::tr("Signatures")},
    {XBinary::STRUCTID_REGIONS, "regions", QObject::tr("Regions")},
    {XBinary::STRUCTID_MEMORYMAP, "memorymap", QObject::tr("Memory map")},
    {XBinary::STRUCTID_SYMBOLS, "symbols", QObject::tr("Symbols")},
    {XBinary::STRUCTID_IMPORT, "import", QObject::tr("Import")},
    {XBinary::STRUCTID_EXPORT, "export", QObject::tr("Export")},
    {XBinary::STRUCTID_RESOURCES, "resources", QObject::tr("Resources")},
    {XBinary::STRUCTID_ENTROPY, "entropy", QObject::tr("Entropy")},
    {XBinary::STRUCTID_EXTRACTOR, "extractor", QObject::tr("Extractor")},
    {XBinary::STRUCTID_SEARCH, "search", QObject::tr("Search")},
    {XBinary::STRUCTID_OVERLAY, "overlay", QObject::tr("Overlay")},
    {XBinary::STRUCTID_INFO, "info", QObject::tr("Info")},
    {XBinary::STRUCTID_TOOLS, "tools", QObject::tr("Tools")},
    {XBinary::STRUCTID_DEMANGLE, "demangle", QObject::tr("Demangle")},
};

XBinary::XCONVERT _TABLE_XBINARY_HANDLE_METHOD[] = {
    {XBinary::HANDLE_METHOD_UNKNOWN, "Unknown", QObject::tr("Unknown")},
    {XBinary::HANDLE_METHOD_STORE, "Store", QString("Store")},
    {XBinary::HANDLE_METHOD_PDF_IMAGEDATA, "PDF_IMAGEDATA", QString("PDF Image data")},
    {XBinary::HANDLE_METHOD_PDF_CCITTIMAGE, "PDF_CCITTIMAGE", QString("PDF CCITT Image")},
    {XBinary::HANDLE_METHOD_PDF_PALETTE, "PDF_PALETTE", QString("PDF Palette")},
    {XBinary::HANDLE_METHOD_ANDROID_XML, "Android_XML", QString("Android XML")},
    {XBinary::HANDLE_METHOD_FILE, "File", QObject::tr("File")},  // TODO Check
    {XBinary::HANDLE_METHOD_DEFLATE, "Deflate", QString("Deflate")},
    {XBinary::HANDLE_METHOD_DEFLATE64, "Deflate64", QString("Deflate64")},
    {XBinary::HANDLE_METHOD_BZIP2, "Bzip2", QString("Bzip2")},
    {XBinary::HANDLE_METHOD_LZMA, "LZMA", QString("LZMA")},
    {XBinary::HANDLE_METHOD_LZMA2, "LZMA2", QString("LZMA2")},
    {XBinary::HANDLE_METHOD_XZ, "XZ", QString("XZ")},
    {XBinary::HANDLE_METHOD_LZW_PDF, "LZW_PDF", QString("LZW PDF")},
    {XBinary::HANDLE_METHOD_ASCII85, "ASCII85", QString("ASCII85 PDF")},
    {XBinary::HANDLE_METHOD_PPMD7, "PPMD7", QString("PPMD7")},
    {XBinary::HANDLE_METHOD_PPMD8, "PPMD8", QString("PPMD8")},  // TODO
    {XBinary::HANDLE_METHOD_LZH5, "LZH5", QString("LZH5")},
    {XBinary::HANDLE_METHOD_LZH6, "LZH6", QString("LZH6")},
    {XBinary::HANDLE_METHOD_LZH7, "LZH7", QString("LZH7")},
    {XBinary::HANDLE_METHOD_RAR_15, "RAR_15", QString("RAR 1.5")},
    {XBinary::HANDLE_METHOD_RAR_20, "RAR_20", QString("RAR 2.0")},
    {XBinary::HANDLE_METHOD_RAR_29, "RAR_29", QString("RAR 2.9")},
    {XBinary::HANDLE_METHOD_RAR_50, "RAR_50", QString("RAR 5.0")},
    {XBinary::HANDLE_METHOD_RAR_70, "RAR_70", QString("RAR 7.0")},
    {XBinary::HANDLE_METHOD_LZSS_SZDD, "LZSS_SZDD", QString("LZSS SZDD")},
    {XBinary::HANDLE_METHOD_IT214_8, "IT214_8", QString("IT214 8-bit")},
    {XBinary::HANDLE_METHOD_IT214_16, "IT214_16", QString("IT214 16-bit")},
    {XBinary::HANDLE_METHOD_IT215_8, "IT215_8", QString("IT215 8-bit")},
    {XBinary::HANDLE_METHOD_IT215_16, "IT215_16", QString("IT215 16-bit")},
    {XBinary::HANDLE_METHOD_IMPLODED_4KDICT_2TREES, "Imploded_4K_2", QString("Imploded 4K/2")},
    {XBinary::HANDLE_METHOD_IMPLODED_4KDICT_3TREES, "Imploded_4K_3", QString("Imploded 4K/3")},
    {XBinary::HANDLE_METHOD_IMPLODED_8KDICT_2TREES, "Imploded_8K_2", QString("Imploded 8K/2")},
    {XBinary::HANDLE_METHOD_IMPLODED_8KDICT_3TREES, "Imploded_8K_3", QString("Imploded 8K/3")},
    {XBinary::HANDLE_METHOD_SHRINK, "Shrink", QString("Shrink")},
    {XBinary::HANDLE_METHOD_REDUCE_1, "Reduce_1", QString("Reduce 1")},
    {XBinary::HANDLE_METHOD_REDUCE_2, "Reduce_2", QString("Reduce 2")},
    {XBinary::HANDLE_METHOD_REDUCE_3, "Reduce_3", QString("Reduce 3")},
    {XBinary::HANDLE_METHOD_REDUCE_4, "Reduce_4", QString("Reduce 4")},
    {XBinary::HANDLE_METHOD_ZIP_AES, "ZIP_AES", QString("ZIP AES")},
    {XBinary::HANDLE_METHOD_ZIP_AES128, "ZIP_AES128", QString("ZIP AES128")},
    {XBinary::HANDLE_METHOD_ZIP_AES192, "ZIP_AES192", QString("ZIP AES192")},
    {XBinary::HANDLE_METHOD_ZIP_AES256, "ZIP_AES256", QString("ZIP AES256")},
    {XBinary::HANDLE_METHOD_7Z_AES, "7Z_AES", QString("7Z AES")},
    {XBinary::HANDLE_METHOD_RAR5_AES, "RAR5_AES", QString("RAR5 AES")},
    {XBinary::HANDLE_METHOD_ZIPCRYPTO, "ZIP_Crypto", QString("ZIP Crypto")},
    {XBinary::HANDLE_METHOD_ZLIB, "ZLIB", QString("ZLIB")},
    {XBinary::HANDLE_METHOD_MSZIP_CAB, "MSZIP_CAB", QString("MSZIP CAB")},
    {XBinary::HANDLE_METHOD_STORE_CAB, "STORE_CAB", QString("STORE CAB")},
    {XBinary::HANDLE_METHOD_LZX_CAB, "LZX_CAB", QString("LZX CAB")},
    {XBinary::HANDLE_METHOD_BCJ, "BCJ", QString("BCJ")},
    {XBinary::HANDLE_METHOD_BCJ2, "BCJ2", QString("BCJ2")},
    {XBinary::HANDLE_METHOD_ARM64_BCJ, "ARM64_BCJ", QString("ARM64 BCJ")},
    {XBinary::HANDLE_METHOD_ZSTD, "ZSTD", QString("Zstandard")},
    {XBinary::HANDLE_METHOD_LZIP, "LZIP", QString("LZIP")},
    {XBinary::HANDLE_METHOD_LZOP, "LZOP", QString("LZOP")},
    {XBinary::HANDLE_METHOD_COMPRESS, "COMPRESS", QString("Compress (LZW)")},
    {XBinary::HANDLE_METHOD_ARJ, "ARJ", QString("ARJ")},
    {XBinary::HANDLE_METHOD_ARJ_FASTEST, "ARJ_FASTEST", QString("ARJ Fastest")},
    {XBinary::HANDLE_METHOD_BROTLI, "BROTLI", QString("Brotli")},
    {XBinary::HANDLE_METHOD_ACE, "ACE", QString("ACE")},
    {XBinary::HANDLE_METHOD_ACE_DELTA, "ACE_BLOCKED_1", QString("ACE BLOCKED_1 (unsupported)")},
    {XBinary::HANDLE_METHOD_DELTA, "DELTA", QString("Delta")},
    {XBinary::HANDLE_METHOD_ARM_BCJ, "ARM_BCJ", QString("ARM BCJ")},
    {XBinary::HANDLE_METHOD_ARMT_BCJ, "ARMT_BCJ", QString("ARMT BCJ")},
    {XBinary::HANDLE_METHOD_PPC_BCJ, "PPC_BCJ", QString("PPC BCJ")},
    {XBinary::HANDLE_METHOD_SPARC_BCJ, "SPARC_BCJ", QString("SPARC BCJ")},
    {XBinary::HANDLE_METHOD_IA64_BCJ, "IA64_BCJ", QString("IA64 BCJ")},
    {XBinary::HANDLE_METHOD_LZX, "LZX", QString("LZX")},
    {XBinary::HANDLE_METHOD_XPRESS, "XPRESS", QString("XPRESS")},
    {XBinary::HANDLE_METHOD_XPRESS_HUFF, "XPRESS_HUFF", QString("XPRESS Huffman")},
    {XBinary::HANDLE_METHOD_KWAJ_XOR, "KWAJ_XOR", QString("KWAJ XOR")},
    {XBinary::HANDLE_METHOD_KWAJ_LZSS, "KWAJ_LZSS", QString("KWAJ LZSS")},
    {XBinary::HANDLE_METHOD_KWAJ_LZH, "KWAJ_LZH", QString("KWAJ LZH")},
    {XBinary::HANDLE_METHOD_ZOO_LZD, "ZOO_LZD", QString("ZOO LZD")},
    {XBinary::HANDLE_METHOD_ZOO_LZH, "ZOO_LZH", QString("ZOO LZH")},
};

XBinary::XCONVERT _TABLE_XBinary_FILEPART[] = {
    {XBinary::FILEPART_UNKNOWN, "Unknown", QObject::tr("Unknown")}, {XBinary::FILEPART_HEADER, "Header", QObject::tr("Header")},
    {XBinary::FILEPART_OVERLAY, "Overlay", QObject::tr("Overlay")}, {XBinary::FILEPART_RESOURCE, "Resource", QObject::tr("Resource")},
    {XBinary::FILEPART_REGION, "Region", QObject::tr("Region")},    {XBinary::FILEPART_DEBUGDATA, "DebugData", QObject::tr("Debug data")},
    {XBinary::FILEPART_STREAM, "Stream", QObject::tr("Stream")},    {XBinary::FILEPART_SIGNATURE, "Signature", QObject::tr("Signature")},
    {XBinary::FILEPART_SECTION, "Section", QObject::tr("Section")}, {XBinary::FILEPART_DATA, "Data", QObject::tr("Data")},
    {XBinary::FILEPART_SEGMENT, "Segment", QObject::tr("Segment")}, {XBinary::FILEPART_FOOTER, "Footer", QObject::tr("Footer")},
    {XBinary::FILEPART_OBJECT, "Object", QObject::tr("Object")},    {XBinary::FILEPART_TABLE, "Table", QObject::tr("Table")},
    {XBinary::FILEPART_VALUE, "Value", QObject::tr("Value")},
};

XBinary::XCONVERT _TABLE_XBinary_FT[] = {
    {XBinary::FT_UNKNOWN, "Unknown", QObject::tr("Unknown")},
    {XBinary::FT_DATA, "Data", QObject::tr("Data")},
    {XBinary::FT_REGION, "Region", QObject::tr("Region")},
    {XBinary::FT_PROCESS, "Process", QObject::tr("Process")},
    {XBinary::FT_BINARY, "Binary", QString("Binary")},
    {XBinary::FT_BINARY16, "Binary16", QString("Binary16")},
    {XBinary::FT_BINARY32, "Binary32", QString("Binary32")},
    {XBinary::FT_BINARY64, "Binary64", QString("Binary64")},
    {XBinary::FT_COM, "COM", QString("COM")},
    {XBinary::FT_MSDOS, "MSDOS", QString("MSDOS")},
    {XBinary::FT_DOS16M, "DOS16M", QString("DOS/16M")},
    {XBinary::FT_DOS4G, "DOS4G", QString("DOS/4G")},
    {XBinary::FT_NE, "NE", QString("NE")},
    {XBinary::FT_LE, "LE", QString("LE")},
    {XBinary::FT_LX, "LX", QString("LX")},
    {XBinary::FT_PE, "PE", QString("PE")},
    {XBinary::FT_PE32, "PE32", QString("PE32")},
    {XBinary::FT_PE64, "PE64", QString("PE64")},
    {XBinary::FT_ELF, "ELF", QString("ELF")},
    {XBinary::FT_ELF32, "ELF32", QString("ELF32")},
    {XBinary::FT_ELF64, "ELF64", QString("ELF64")},
    {XBinary::FT_MACHO, "Mach-O", QString("Mach-O")},
    {XBinary::FT_MACHO32, "Mach-O32", QString("Mach-O32")},
    {XBinary::FT_MACHO64, "Mach-O64", QString("Mach-O64")},
    {XBinary::FT_AMIGAHUNK, "Amiga Hunk", QString("Amiga Hunk")},
    {XBinary::FT_ATARIST, "Atari ST", QString("Atari ST")},
    {XBinary::FT_CLI_ASSEMBLY, ".NET", QString(".NET")},
    // Extra
    {XBinary::FT_7Z, "7-Zip", QString("7-Zip")},
    {XBinary::FT_ANDROIDASRC, "AndroidASRC", QString("Android ASRC")},
    {XBinary::FT_ANDROIDXML, "AndroidXML", QString("Android XML")},
    {XBinary::FT_APK, "APK", QString("APK")},
    {XBinary::FT_APKS, "APKS", QString("APKS")},
    {XBinary::FT_AR, "ar", QString("ar")},
    {XBinary::FT_ARCHIVE, "Archive", QObject::tr("Archive")},
    {XBinary::FT_AUDIO, "Audio", QObject::tr("Audio")},
    {XBinary::FT_AVI, "AVI", QString("AVI")},
    {XBinary::FT_BMP, "BMP", QString("BMP")},
    {XBinary::FT_CAB, "CAB", QString("CAB")},
    {XBinary::FT_CUR, "CUR", QString("CUR")},
    {XBinary::FT_DEX, "DEX", QString("DEX")},
    {XBinary::FT_DOCUMENT, "Document", QObject::tr("Document")},
    {XBinary::FT_GIF, "GIF", QString("GIF")},
    {XBinary::FT_GZIP, "GZIP", QString("GZIP")},
    {XBinary::FT_ICC, "ICC", QString("ICC")},
    {XBinary::FT_ICO, "ICO", QString("ICO")},
    {XBinary::FT_IMAGE, "Image", QObject::tr("Image")},
    {XBinary::FT_ISO9660, "ISO 9660", QString("ISO 9660")},
    {XBinary::FT_UDF, "UDF", QString("UDF")},
    {XBinary::FT_IPA, "IPA", QString("IPA")},
    {XBinary::FT_JAR, "JAR", QString("JAR")},
    {XBinary::FT_JPEG, "JPEG", QString("JPEG")},
    {XBinary::FT_LHA, "LHA", QString("LHA")},
    {XBinary::FT_MACHOFAT, "Mach-O FAT", QString("Mach-O FAT")},
    {XBinary::FT_MP3, "MP3", QString("MP3")},
    {XBinary::FT_MP4, "MP4", QString("MP4")},
    {XBinary::FT_XM, "XM", QString("XM")},
    {XBinary::FT_NPM, "NPM", QString("NPM")},
    {XBinary::FT_PDF, "PDF", QString("PDF")},
    {XBinary::FT_DER, "DER", QString("DER")},
    {XBinary::FT_PLAINTEXT, "PlainText", QObject::tr("Plain Text")},
    {XBinary::FT_PNG, "PNG", QString("PNG")},
    {XBinary::FT_RAR, "RAR", QString("RAR")},
    {XBinary::FT_RIFF, "RIFF", QString("RIFF")},
    {XBinary::FT_SIGNATURE, "Signature", QObject::tr("Signature")},
    {XBinary::FT_TAR, "tar", QString("tar")},
    {XBinary::FT_TAR_GZ, "tar.gz (typed)", QString("tar.gz")},
    {XBinary::FT_TAR_BZIP2, "tar.bz2", QString("tar.bz2")},
    {XBinary::FT_TAR_LZIP, "tar.lz", QString("tar.lz")},
    {XBinary::FT_TAR_LZMA, "tar.lzma", QString("tar.lzma")},
    {XBinary::FT_TAR_LZOP, "tar.lzo", QString("tar.lzo")},
    {XBinary::FT_TAR_XZ, "tar.xz", QString("tar.xz")},
    {XBinary::FT_TAR_Z, "tar.Z", QString("tar.Z")},
    {XBinary::FT_TAR_ZSTD, "tar.zst", QString("tar.zst")},
    {XBinary::FT_TEXT, "Text", QObject::tr("Text")},
    {XBinary::FT_TIFF, "TIFF", QString("TIFF")},
    {XBinary::FT_PAL, "PAL", QString("PAL")},
    {XBinary::FT_UNICODE, "Unicode", QString("Unicode")},
    {XBinary::FT_UNICODE_BE, "UnicodeBE", QString("Unicode BE")},
    {XBinary::FT_UNICODE_LE, "UnicodeLE", QString("Unicode LE")},
    {XBinary::FT_XML, "XML", QString("XML")},
    {XBinary::FT_UTF8, "UTF8", QString("UTF8")},
    {XBinary::FT_VIDEO, "Video", QObject::tr("Video")},
    {XBinary::FT_WEBP, "WebP", QString("WebP")},
    {XBinary::FT_WAV, "WAV", QString("WAV")},
    {XBinary::FT_ZIP, "ZIP", QString("ZIP")},
    {XBinary::FT_ZLIB, "zlib", QString("zlib")},
    {XBinary::FT_DEB, "deb", QString("deb")},
    {XBinary::FT_BWDOS16M, "BW DOS16M", QString("BW DOS16M")},
    {XBinary::FT_JAVACLASS, "Java Class", QString("Java Class")},
    {XBinary::FT_PYC, "Python Bytecode", QString("Python Bytecode")},
    {XBinary::FT_TTF, "TTF", QString("TTF")},
    {XBinary::FT_DJVU, "DjVu", QString("DjVu")},
    {XBinary::FT_CFBF, "CFBF", QString("CFBF")},
    {XBinary::FT_SZDD, "SZDD", QString("SZDD")},
    {XBinary::FT_BZIP2, "BZip2", QString("BZip2")},
    {XBinary::FT_ZSTD, "Zstandard", QString("Zstandard")},
    {XBinary::FT_XZ, "XZ", QString("XZ")},
    {XBinary::FT_LZIP, "LZIP", QString("Lzip (LZMA)")},
    {XBinary::FT_LZO, "LZO", QString("LZO (lzop)")},
    {XBinary::FT_COMPRESS, "Compress", QString("Unix compress (.Z)")},
    {XBinary::FT_CPIO, "CPIO", QString("CPIO")},
    {XBinary::FT_MINIDUMP, "MiniDump", QString("Windows MiniDump")},
    {XBinary::FT_DMG, "DMG", QString("Apple Disk Image")},
    {XBinary::FT_ARC, "ARC", QString("ARC")},
    {XBinary::FT_FREEARC, "FreeARC", QString("FreeARC")},
    {XBinary::FT_ARJ, "ARJ", QString("ARJ")},
    {XBinary::FT_ACE, "ACE", QString("ACE")},
    {XBinary::FT_BROTLI, "Brotli", QString("Brotli")},
    {XBinary::FT_LZ4, "LZ4", QString("LZ4")},
    {XBinary::FT_LZMA, "LZMA", QString("LZMA")},
    {XBinary::FT_WIM, "WIM", QString("Windows Imaging Format")},
    {XBinary::FT_RPM, "RPM", QString("RPM Package")},
    {XBinary::FT_KWAJ, "KWAJ", QString("KWAJ")},
    {XBinary::FT_ASAR, "ASAR", QString("Electron ASAR")},
    {XBinary::FT_XAR, "XAR", QString("XAR")},
    {XBinary::FT_ZOO, "ZOO", QString("ZOO")},
    {XBinary::FT_PDB, "PDB", QString("Program Database")},
};

XBinary::XIDSTRING _TABLE_XBinary_VT[] = {
    {XBinary::VT_UNKNOWN, "Unknown"},
    {XBinary::VT_STRING, "String"},
    {XBinary::VT_HEX, "Hex"},
    {XBinary::VT_DATETIME, "DateTime"},
    {XBinary::VT_A, "A"},
    {XBinary::VT_A_I, "A"},
    {XBinary::VT_U, "U"},
    {XBinary::VT_U_I, "U"},
    {XBinary::VT_UTF8, "UTF8"},
    {XBinary::VT_UTF8_I, "UTF"},
    {XBinary::VT_SIGNATURE, "Signature"},
    {XBinary::VT_VALUE, "Value"},
    {XBinary::VT_BIT, "bit"},
    {XBinary::VT_BYTE, "byte"},
    {XBinary::VT_WORD, "word"},
    {XBinary::VT_DWORD, "dword"},
    {XBinary::VT_QWORD, "qword"},
    {XBinary::VT_128, "U128"},
    {XBinary::VT_256, "U256"},
    {XBinary::VT_FPEG, "FPEG"},
    {XBinary::VT_CHAR, "char"},
    {XBinary::VT_UCHAR, "uchar"},
    {XBinary::VT_SHORT, "short"},
    {XBinary::VT_USHORT, "ushort"},
    {XBinary::VT_INT, "int"},
    {XBinary::VT_UINT, "uint"},
    {XBinary::VT_INT64, "int64"},
    {XBinary::VT_UINT8, "uint8"},
    {XBinary::VT_UINT16, "uint16"},
    {XBinary::VT_UINT32, "uint32"},
    {XBinary::VT_UINT64, "uint64"},
    {XBinary::VT_DOUBLE, "double"},
    {XBinary::VT_FLOAT, "float"},
    {XBinary::VT_PACKEDNUMBER, "PackedNumber"},
    {XBinary::VT_ULEB128, "ULEB128"},
    {XBinary::VT_UTF32, "UTF32"},
};

XBinary::XIDSTRING _TABLE_XBinary_CODEPAGE[] = {
    {XBinary::CODEPAGE_ASCII, "ASCII"},
    {XBinary::CODEPAGE_WINDOWS_874, "Windows-874"},
    {XBinary::CODEPAGE_WINDOWS_1250, "Windows-1250"},
    {XBinary::CODEPAGE_WINDOWS_1251, "Windows-1251"},
    {XBinary::CODEPAGE_WINDOWS_1252, "Windows-1252"},
    {XBinary::CODEPAGE_WINDOWS_1253, "Windows-1253"},
    {XBinary::CODEPAGE_WINDOWS_1254, "Windows-1254"},
    {XBinary::CODEPAGE_WINDOWS_1255, "Windows-1255"},
    {XBinary::CODEPAGE_WINDOWS_1256, "Windows-1256"},
    {XBinary::CODEPAGE_WINDOWS_1257, "Windows-1257"},
    {XBinary::CODEPAGE_WINDOWS_1258, "Windows-1258"},
};

XBinary::XCONVERT _TABLE_XBinary_XFTYPE[] = {
    {XBinary::XFTYPE_UNKNOWN, "Unknown", QObject::tr("Unknown")},
    {XBinary::XFTYPE_HEADER, "Header", QObject::tr("Header")},
    {XBinary::XFTYPE_TABLE, "Table", QObject::tr("Table")},
    {XBinary::XFTYPE_COMMAND, "Command", QObject::tr("Command")},
};

const double XBinary::D_ENTROPY_THRESHOLD = 6.5;

QString XBinary::XCONVERT_idToTransString(quint32 nID, XCONVERT *pRecords, qint32 nRecordsSize)
{
    QString sResult;

    for (qint32 i = 0; i < nRecordsSize; i++) {
        if (pRecords[i].nID == nID) {
            sResult = pRecords[i].sTransString;
            break;
        }
    }

    if (sResult == "") {
        sResult = tr("Unknown");
#ifdef QT_DEBUG
        qDebug() << "Unknown XBinary::XCONVERT_idToTransString" << nID;
#endif
    }

    return sResult;
}

QString XBinary::XCONVERT_idToSetString(quint32 nID, XCONVERT *pRecords, qint32 nRecordsSize)
{
    QString sResult;

    for (qint32 i = 0; i < nRecordsSize; i++) {
        if (pRecords[i].nID == nID) {
            sResult = pRecords[i].sSetString;
            break;
        }
    }

    if (sResult == "") {
        sResult = "Unknown";
#ifdef QT_DEBUG
        qDebug() << "Unknown XBinary::XCONVERT_idToSetString" << nID;
#endif
    }

    return sResult;
}

QString XBinary::XCONVERT_idToFtString(quint32 nID, XCONVERT *pRecords, qint32 nRecordsSize)
{
    return XCONVERT_idToSetString(nID, pRecords, nRecordsSize).toUpper().remove(" ").remove("-");
}

quint32 XBinary::XCONVERT_ftStringToId(const QString &sString, XCONVERT *pRecords, qint32 nRecordsSize)
{
    quint32 nResult = 0;

    for (qint32 i = 0; i < nRecordsSize; i++) {
        if (pRecords[i].sSetString.toUpper().remove(" ").remove("-") == sString.toUpper()) {
            nResult = pRecords[i].nID;
            break;
        }
    }

#ifdef QT_DEBUG
    if (nResult == 0) {
        qDebug() << "XCONVERT_ftStringToId: Not found" << sString;
#ifdef QT_DEBUG
        qDebug() << "Unknown XBinary::XCONVERT_ftStringToId" << sString;
#endif
    }
#endif

    return nResult;
}

QString XBinary::XCONVERT_translate(const QString &sString, XCONVERT *pRecords, qint32 nRecordsSize)
{
    QString sResult;

    if (sString != "") {
        QString _sString = sString.toLower();

        for (qint32 i = 0; i < nRecordsSize; i++) {
            if (pRecords[i].sSetString.toLower() == _sString) {
                sResult = pRecords[i].sTransString;
                break;
            }
        }

        if (sResult == "") {
            sResult = sString;
        } else {
            if (sString.at(0).isUpper()) {
                sResult[0] = sResult.at(0).toUpper();
            } else {
                sResult[0] = sResult.at(0).toLower();
            }
        }
    }

    return sResult;
}

QString XBinary::XIDSTRING_idToString(quint64 nID, XIDSTRING *pRecords, qint32 nRecordsSize)
{
    QString sResult;

    for (qint32 i = 0; i < nRecordsSize; i++) {
        if (pRecords[i].nID == nID) {
            sResult = pRecords[i].sString;
            break;
        }
    }

    if (sResult == "") {
        sResult = "Unknown";
#ifdef QT_DEBUG
        qDebug() << "Unknown XBinary::XIDSTRING_idToString" << nID;
#endif
    }

    return sResult;
}

quint64 XBinary::XIDSTRING_ftStringToId(const QString &sString, XIDSTRING *pRecords, qint32 nRecordsSize)
{
    quint64 nResult = 0;

    for (qint32 i = 0; i < nRecordsSize; i++) {
        if (pRecords[i].sString.toUpper().remove(" ").remove("-").remove("/").remove("\\") == sString.toUpper()) {
            nResult = pRecords[i].nID;
            break;
        }
    }

#ifdef QT_DEBUG
    if (nResult == 0) {
        qDebug() << "XIDSTRING_ftStringToId: Not found" << sString;
    }
#endif

    return nResult;
}

QMap<quint64, QString> XBinary::XIDSTRING_createMap(XIDSTRING *pRecords, qint32 nRecordsSize)
{
    QMap<quint64, QString> mapResult;

    for (qint32 i = 0; i < nRecordsSize; i++) {
        mapResult.insert(pRecords[i].nID, pRecords[i].sString);
    }

    return mapResult;
}

QMap<quint64, QString> XBinary::XIDSTRING_createMapPrefix(XIDSTRING *pRecords, qint32 nRecordsSize, const QString &sPrefix)
{
    QMap<quint64, QString> mapResult;

    for (qint32 i = 0; i < nRecordsSize; i++) {
        mapResult.insert(pRecords[i].nID, sPrefix + pRecords[i].sString);
    }

    return mapResult;
}

QMap<XBinary::UNPACK_PROP, QVariant> XBinary::getDefaultUnpackProperties()
{
    QPointer<XBinary> guardedThis(this);
    QMap<XBinary::UNPACK_PROP, QVariant> result;

    result.insert(UNPACK_PROP_FIXFILENAMES, true);
    result.insert(UNPACK_PROP_OVERWRITEFILES, true);

    PDSTRUCT pdStruct = createPdStruct();
    UNPACK_STATE state = {};
    QMap<UNPACK_PROP, QVariant> mapProperties;

    const bool bInitialized = guardedThis &&
        guardedThis->initUnpack(&state, mapProperties, &pdStruct);
    if (guardedThis && bInitialized) {
        const qint32 nNumberOfRecords = state.nNumberOfRecords;
        bool bEnumerationValid = (state.nCurrentIndex == 0) &&
                                 (nNumberOfRecords >= 0) &&
                                 (state.nCurrentIndex <= nNumberOfRecords);
        QMap<UNPACK_PROP, QVariant> mapDiscoveredProperties;

        for (qint32 i = 0; guardedThis && bEnumerationValid &&
             (i < nNumberOfRecords) &&
             isPdStructNotCanceled(&pdStruct); i++) {
            const qint32 nExpectedIndex = state.nCurrentIndex;
            ARCHIVERECORD record = guardedThis->infoCurrent(
                &state, &pdStruct);
            if (!guardedThis || !isPdStructNotCanceled(&pdStruct) ||
                record.mapProperties.isEmpty() ||
                (record.nStreamOffset < 0) ||
                (record.nStreamSize < 0) ||
                (state.nCurrentIndex < 0) ||
                (state.nCurrentIndex >= state.nNumberOfRecords) ||
                (state.nCurrentIndex != nExpectedIndex) ||
                (state.nNumberOfRecords != nNumberOfRecords)) {
                bEnumerationValid = false;
                break;
            }
            CRC_TYPE crcType = (CRC_TYPE)record.mapProperties.value(FPART_PROP_CRC_TYPE, CRC_TYPE_UNKNOWN).toUInt();

            if ((crcType != CRC_TYPE_UNKNOWN) && record.mapProperties.contains(FPART_PROP_RESULTCRC)) {
                UNPACK_PROP unpackProperty = getUnpackCRCProperty(crcType);

                // Expose the applicable checksum control, not a redundant
                // global checkbox plus its typed checkbox. CHECKCRC remains
                // the default-true compatibility override for old callers.
                if (unpackProperty == UNPACK_PROP_UNKNOWN) {
                    unpackProperty = UNPACK_PROP_CHECKCRC;
                }

                mapDiscoveredProperties.insert(unpackProperty, true);
            }

            if (i + 1 < nNumberOfRecords) {
                const qint32 nPreviousIndex = state.nCurrentIndex;
                const bool bMoved = guardedThis->moveToNext(
                    &state, &pdStruct);
                if (!guardedThis || !bMoved ||
                    !isPdStructNotCanceled(&pdStruct) ||
                    (state.nCurrentIndex != (nPreviousIndex + 1)) ||
                    (state.nCurrentIndex >= nNumberOfRecords) ||
                    (state.nNumberOfRecords != nNumberOfRecords)) {
                    bEnumerationValid = false;
                    break;
                }
            }
        }

        const bool bFinished = guardedThis &&
            guardedThis->finishUnpack(&state, nullptr);
        if (guardedThis && bEnumerationValid && bFinished &&
            isPdStructNotCanceled(&pdStruct)) {
            for (auto it = mapDiscoveredProperties.constBegin();
                 it != mapDiscoveredProperties.constEnd(); ++it) {
                result.insert(it.key(), it.value());
            }
        }
    }

    return result;
}

bool XBinary::hasUnpackCRC(PDSTRUCT *pPdStruct)
{
    QPointer<XBinary> guardedThis(this);
    bool bFound = false;
    PDSTRUCT pdStructEmpty = createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    UNPACK_STATE state = {};
    QMap<UNPACK_PROP, QVariant> mapProperties;

    const bool bInitialized = guardedThis &&
        guardedThis->initUnpack(&state, mapProperties, pPdStruct);
    if (guardedThis && bInitialized) {
        const qint32 nNumberOfRecords = state.nNumberOfRecords;
        bool bEnumerationValid = (state.nCurrentIndex == 0) &&
                                 (nNumberOfRecords >= 0) &&
                                 (state.nCurrentIndex <= nNumberOfRecords);

        for (qint32 i = 0; guardedThis && bEnumerationValid &&
             (i < nNumberOfRecords) &&
             isPdStructNotCanceled(pPdStruct); i++) {
            const qint32 nExpectedIndex = state.nCurrentIndex;
            ARCHIVERECORD record = guardedThis->infoCurrent(
                &state, pPdStruct);
            if (!guardedThis || !isPdStructNotCanceled(pPdStruct) ||
                record.mapProperties.isEmpty() ||
                (record.nStreamOffset < 0) ||
                (record.nStreamSize < 0) ||
                (state.nCurrentIndex < 0) ||
                (state.nCurrentIndex >= state.nNumberOfRecords) ||
                (state.nCurrentIndex != nExpectedIndex) ||
                (state.nNumberOfRecords != nNumberOfRecords)) {
                bEnumerationValid = false;
                break;
            }
            CRC_TYPE crcType = (CRC_TYPE)record.mapProperties.value(FPART_PROP_CRC_TYPE, CRC_TYPE_UNKNOWN).toUInt();

            if ((crcType != CRC_TYPE_UNKNOWN) && record.mapProperties.contains(FPART_PROP_RESULTCRC)) {
                bFound = true;
                break;
            }

            if (i + 1 < nNumberOfRecords) {
                const qint32 nPreviousIndex = state.nCurrentIndex;
                const bool bMoved = guardedThis->moveToNext(
                    &state, pPdStruct);
                if (!guardedThis || !bMoved ||
                    !isPdStructNotCanceled(pPdStruct) ||
                    (state.nCurrentIndex != (nPreviousIndex + 1)) ||
                    (state.nCurrentIndex >= nNumberOfRecords) ||
                    (state.nNumberOfRecords != nNumberOfRecords)) {
                    bEnumerationValid = false;
                    break;
                }
            }
        }

        const bool bFinished = guardedThis &&
            guardedThis->finishUnpack(&state, nullptr);
        return guardedThis && bEnumerationValid && bFinished && bFound &&
               isPdStructNotCanceled(pPdStruct);
    }

    return false;
}

XBinary::UNPACK_PROP XBinary::getUnpackCRCProperty(CRC_TYPE crcType)
{
    UNPACK_PROP result = UNPACK_PROP_UNKNOWN;

    if ((crcType == CRC_TYPE_EDB88320) || (crcType == CRC_TYPE_FFFFFFFF_EDB88320_00000000) ||
        (crcType == CRC_TYPE_FFFFFFFF_EDB88320_FFFFFFFFF)) {
        result = UNPACK_PROP_CHECKCRC32;
    } else if (crcType == CRC_TYPE_RAR14) {
        result = UNPACK_PROP_CHECKRAR14;
    } else if (crcType == CRC_TYPE_CRC16) {
        result = UNPACK_PROP_CHECKCRC16;
    } else if (crcType == CRC_TYPE_CRC16ARC) {
        result = UNPACK_PROP_CHECKCRC16ARC;
    } else if (crcType == CRC_TYPE_ADLER32) {
        result = UNPACK_PROP_CHECKADLER32;
    }

    return result;
}

bool XBinary::isUnpackCRCProperty(UNPACK_PROP unpackProperty)
{
    return (unpackProperty == UNPACK_PROP_CHECKCRC) || (unpackProperty == UNPACK_PROP_CHECKCRC32) || (unpackProperty == UNPACK_PROP_CHECKCRC16) ||
           (unpackProperty == UNPACK_PROP_CHECKCRC16ARC) || (unpackProperty == UNPACK_PROP_CHECKADLER32) || (unpackProperty == UNPACK_PROP_CHECKRAR14);
}

bool XBinary::isUnpackCRCEnabled(const QMap<UNPACK_PROP, QVariant> &mapProperties)
{
    bool bResult = mapProperties.value(UNPACK_PROP_CHECKCRC, true).toBool();

    // Keep source compatibility for callers which still use the old inverse
    // property. A true NOCRC value always disables checking.
    if (mapProperties.value(UNPACK_PROP_NOCRC, false).toBool()) {
        bResult = false;
    }

    return bResult;
}

bool XBinary::isUnpackCRCEnabled(const QMap<UNPACK_PROP, QVariant> &mapProperties, CRC_TYPE crcType)
{
    bool bResult = isUnpackCRCEnabled(mapProperties);

    if (bResult) {
        UNPACK_PROP unpackProperty = getUnpackCRCProperty(crcType);

        if (unpackProperty != UNPACK_PROP_UNKNOWN) {
            bResult = mapProperties.value(unpackProperty, true).toBool();
        }
    }

    return bResult;
}

qint64 XBinary::getNumberOfArchiveRecords(PDSTRUCT *pPdStruct)
{
    QPointer<XBinary> guardedThis(this);
    qint64 nResult = 0;

    if (!isPdStructNotCanceled(pPdStruct)) return 0;

    UNPACK_STATE state = {};
    QMap<UNPACK_PROP, QVariant> mapProperties;

    // Initialize the streaming archive state.
    const bool bInitialized = guardedThis &&
        guardedThis->initUnpack(&state, mapProperties, pPdStruct);
    if (guardedThis && bInitialized) {
        const bool bStateValid = (state.nCurrentIndex == 0) &&
                                 (state.nNumberOfRecords >= 0) &&
                                 (state.nCurrentIndex <=
                                  state.nNumberOfRecords);
        nResult = bStateValid ? state.nNumberOfRecords : 0;

        // A successful initUnpack() owns its context until finishUnpack(),
        // even when the caller was canceled while initialization completed.
        const bool bFinished = guardedThis->finishUnpack(&state, nullptr);
        if (!guardedThis || !bFinished || !bStateValid ||
            !isPdStructNotCanceled(pPdStruct)) {
            nResult = 0;
        }
    }

    return nResult;
}

QList<XBinary::ARCHIVERECORD> XBinary::getArchiveRecords(qint32 nLimit, PDSTRUCT *pPdStruct)
{
    QPointer<XBinary> guardedThis(this);
    QList<XBinary::ARCHIVERECORD> listResult;

    if ((nLimit == 0) || (nLimit < -1)) return listResult;

    PDSTRUCT pdStructEmpty = createPdStruct();
    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    if (!isPdStructNotCanceled(pPdStruct)) {
        return listResult;
    }

    // Initialize unpack state
    UNPACK_STATE state = {};
    QMap<UNPACK_PROP, QVariant> mapProperties;

    const bool bInitialized = guardedThis &&
        guardedThis->initUnpack(&state, mapProperties, pPdStruct);
    if (guardedThis && bInitialized) {
        const qint32 nNumberOfRecords = state.nNumberOfRecords;
        qint32 nCount = 0;
        bool bEnumerationValid = (state.nCurrentIndex == 0) &&
                                 (nNumberOfRecords >= 0) &&
                                 (state.nCurrentIndex <= nNumberOfRecords);

        // Iterate through records using streaming API
        while (guardedThis && bEnumerationValid &&
               isPdStructNotCanceled(pPdStruct) &&
               (state.nCurrentIndex < nNumberOfRecords)) {
            // Get current record info
            const qint32 nExpectedIndex = state.nCurrentIndex;
            ARCHIVERECORD record = guardedThis->infoCurrent(
                &state, pPdStruct);

            if (!guardedThis || !isPdStructNotCanceled(pPdStruct) ||
                record.mapProperties.isEmpty() ||
                (record.nStreamOffset < 0) ||
                (record.nStreamSize < 0) ||
                (state.nCurrentIndex < 0) ||
                (state.nCurrentIndex != nExpectedIndex) ||
                (state.nNumberOfRecords != nNumberOfRecords) ||
                (state.nCurrentIndex >= nNumberOfRecords)) {
                bEnumerationValid = false;
                break;
            }

            listResult.append(record);
            nCount++;

            // Check limit
            if (nLimit != -1 && nCount >= nLimit) {
                break;
            }

            // Move to next record
            const qint32 nPreviousIndex = state.nCurrentIndex;
            const bool bMoved = guardedThis->moveToNext(
                &state, pPdStruct);
            if (!guardedThis || !isPdStructNotCanceled(pPdStruct) ||
                (state.nNumberOfRecords != nNumberOfRecords)) {
                bEnumerationValid = false;
                break;
            }
            if (!bMoved) {
                if (((nPreviousIndex + 1) != nNumberOfRecords) ||
                    ((state.nCurrentIndex != nPreviousIndex) &&
                     (state.nCurrentIndex != nNumberOfRecords))) {
                    bEnumerationValid = false;
                }
                break;
            }
            if ((state.nCurrentIndex != (nPreviousIndex + 1)) ||
                (state.nCurrentIndex >= nNumberOfRecords)) {
                bEnumerationValid = false;
                break;
            }
        }

        // Cleanup must not inherit a canceled enumeration token.
        const bool bFinished = guardedThis &&
            guardedThis->finishUnpack(&state, nullptr);
        if (!guardedThis || !bEnumerationValid || !bFinished ||
            !isPdStructNotCanceled(pPdStruct)) {
            listResult.clear();
        }
    }

    return listResult;
}

bool XBinary::packFolderToDevice(QIODevice *pDevice, const QMap<PACK_PROP, QVariant> &mapProperties, const QString &sFolderName, PDSTRUCT *pPdStruct)
{
    QPointer<XBinary> guardedThis(this);
    QPointer<QIODevice> guardedOutput(pDevice);
    bool bResult = false;

    PDSTRUCT pdStructEmpty = createPdStruct();
    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    // Validate input
    if (!isDirectoryExists(sFolderName)) {
        return false;
    }

    PACK_STATE state = {};

    QMap<PACK_PROP, QVariant> _mapProperties = mapProperties;

    if (!_mapProperties.contains(PACK_PROP_PATHMODE)) {
        _mapProperties.insert(PACK_PROP_PATHMODE, PATH_MODE_RELATIVE);
    }

    if (!_mapProperties.contains(PACK_PROP_BASEPATH)) {
        _mapProperties.insert(PACK_PROP_BASEPATH, sFolderName);
    }

    // Initialize packing (this writes signature/header)
    if (!guardedThis || !guardedOutput ||
        !guardedThis->initPack(&state, guardedOutput.data(),
                               _mapProperties, pPdStruct) ||
        !guardedThis || !guardedOutput) {
        return false;
    }

    // Add entire folder contents
    bResult = guardedThis->addFolder(&state, sFolderName, pPdStruct);
    if (!guardedThis || !guardedOutput) return false;

    // Finish packing (writes end marker and cleans up)
    // Always call finishPack() even if addFolder() failed, to ensure cleanup
    bool bFinish = guardedThis->finishPack(&state, pPdStruct);
    if (!guardedThis || !guardedOutput) return false;

    // Return true only if both operations succeeded
    return bResult && bFinish;
}

QList<QString> XBinary::getListOfArchiveRecordNames(const QMap<UNPACK_PROP, QVariant> &mapProperties, PDSTRUCT *pPdStruct)
{
    QPointer<XBinary> guardedThis(this);
    QList<QString> listResult;

    PDSTRUCT pdStructEmpty = createPdStruct();
    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    if (!isPdStructNotCanceled(pPdStruct)) {
        return listResult;
    }

    // Initialize unpack state
    UNPACK_STATE state = {};

    const bool bInitialized = guardedThis &&
        guardedThis->initUnpack(&state, mapProperties, pPdStruct);
    if (guardedThis && bInitialized) {
        const qint32 nNumberOfRecords = state.nNumberOfRecords;
        bool bEnumerationValid = (state.nCurrentIndex == 0) &&
                                 (nNumberOfRecords >= 0) &&
                                 (state.nCurrentIndex <= nNumberOfRecords);

        // Iterate through records using streaming API
        while (guardedThis && bEnumerationValid &&
               isPdStructNotCanceled(pPdStruct) &&
               (state.nCurrentIndex < nNumberOfRecords)) {
            // Get current record info
            const qint32 nExpectedIndex = state.nCurrentIndex;
            ARCHIVERECORD record = guardedThis->infoCurrent(
                &state, pPdStruct);

            if (!guardedThis || !isPdStructNotCanceled(pPdStruct) ||
                record.mapProperties.isEmpty() ||
                (record.nStreamOffset < 0) ||
                (record.nStreamSize < 0) ||
                (state.nCurrentIndex < 0) ||
                (state.nCurrentIndex != nExpectedIndex) ||
                (state.nNumberOfRecords != nNumberOfRecords) ||
                (state.nCurrentIndex >= nNumberOfRecords)) {
                bEnumerationValid = false;
                break;
            }

            // Extract the record name from properties
            if (record.mapProperties.contains(FPART_PROP_ORIGINALNAME)) {
                QString sRecordName = record.mapProperties.value(FPART_PROP_ORIGINALNAME).toString();
                if (!sRecordName.isEmpty()) {
                    listResult.append(sRecordName);
                }
            }

            // Move to next record
            const qint32 nPreviousIndex = state.nCurrentIndex;
            const bool bMoved = guardedThis->moveToNext(
                &state, pPdStruct);
            if (!guardedThis || !isPdStructNotCanceled(pPdStruct) ||
                (state.nNumberOfRecords != nNumberOfRecords)) {
                bEnumerationValid = false;
                break;
            }
            if (!bMoved) {
                if (((nPreviousIndex + 1) != nNumberOfRecords) ||
                    ((state.nCurrentIndex != nPreviousIndex) &&
                     (state.nCurrentIndex != nNumberOfRecords))) {
                    bEnumerationValid = false;
                }
                break;
            }
            if ((state.nCurrentIndex != (nPreviousIndex + 1)) ||
                (state.nCurrentIndex >= nNumberOfRecords)) {
                bEnumerationValid = false;
                break;
            }
        }

        // Cleanup must not inherit a canceled enumeration token.
        const bool bFinished = guardedThis &&
            guardedThis->finishUnpack(&state, nullptr);
        if (!guardedThis || !bEnumerationValid || !bFinished ||
            !isPdStructNotCanceled(pPdStruct)) {
            listResult.clear();
        }
    }

    return listResult;
}

static qint64 readDeviceWithBoundedProgress(QIODevice *pDevice, char *pBuffer, qint64 nSize)
{
    if (!pDevice || (nSize < 0) || ((nSize > 0) && !pBuffer)) return -1;

    QPointer<QIODevice> guardedDevice(pDevice);
    if (!guardedDevice) return -1;
    const bool bSeekable = !guardedDevice->isSequential();
    if (!guardedDevice) return -1;
    const qint64 nStart = bSeekable ? guardedDevice->pos() : -1;
    const qint64 nMax = (std::numeric_limits<qint64>::max)();
    if (!guardedDevice || (bSeekable && (nStart < 0))) return -1;

    // Sequential/nonblocking QIODevice implementations may transiently return
    // zero without being at EOF.  Give them a small bounded opportunity to
    // become readable; a real EOF or persistent no-progress condition is still
    // returned to the decoder promptly.
    for (qint32 i = 0; i < 3; i++) {
        if (!guardedDevice) return -1;
        if (bSeekable) {
            const bool bSeeked = guardedDevice->seek(nStart);
            if (!guardedDevice || !bSeeked) return -1;
        }
        const qint64 nResult = guardedDevice->read(pBuffer, nSize);
        if (!guardedDevice || (nResult < 0) || (nResult > nSize)) return -1;
        bool bAtEnd = false;
        if ((nResult == 0) && (nSize != 0)) {
            bAtEnd = guardedDevice->atEnd();
            if (!guardedDevice) return -1;
        }
        if ((nResult != 0) || (nSize == 0) || bAtEnd) {
            if (bSeekable) {
                if ((nResult > nMax - nStart) || !guardedDevice) return -1;
                const bool bSeeked = guardedDevice->seek(nStart + nResult);
                if (!guardedDevice || !bSeeked) return -1;
            }
            return nResult;
        }
        if (i != 2) {
            guardedDevice->waitForReadyRead(10);
            if (!guardedDevice) return -1;
        }
    }

    return -1;
}

qint32 XBinary::_readDevice(char *pBuffer, qint32 nBufferSize, DATAPROCESS_STATE *pState)
{
    if (!pState || !pState->pDeviceInput || (nBufferSize < 0) || ((nBufferSize > 0) && !pBuffer) || (pState->nCountInput < 0) ||
        (pState->nInputLimit < -1) || ((pState->nInputLimit != -1) && (pState->nCountInput > pState->nInputLimit))) {
        if (pState) {
            pState->bReadError = true;
        }
        return -1;
    }

    qint32 nReadSize = nBufferSize;
    if (pState->nInputLimit != -1) {
        nReadSize = (qint32)(std::min)((qint64)nReadSize, pState->nInputLimit - pState->nCountInput);
    }
    QPointer<QIODevice> guardedInput(pState->pDeviceInput);
    const qint64 nReadResult = readDeviceWithBoundedProgress(
        guardedInput.data(), pBuffer, nReadSize);

    if (!guardedInput || (nReadResult < 0) ||
        (nReadResult > (qint64)nReadSize) ||
        ((nReadResult > 0) && (pState->nCountInput > ((std::numeric_limits<qint64>::max)() - nReadResult)))) {
        pState->bReadError = true;
        return -1;
    }

    const qint32 nRead = (qint32)nReadResult;
    if (nRead > 0) {
        pState->nCountInput += nRead;
    } else if ((nReadSize > 0) &&
               ((pState->nInputLimit != -1) &&
                (pState->nCountInput < pState->nInputLimit))) {
        // QIODevice is allowed to return a positive short read.  Treat only an
        // actual error/EOF before the requested bounded input is complete as a
        // read failure; callers will request the remaining bytes on the next
        // iteration.
        pState->bReadError = true;
    } else if ((nReadSize > 0) && (pState->nInputLimit == -1)) {
        if (!guardedInput) {
            pState->bReadError = true;
        } else {
            const bool bAtEnd = guardedInput->atEnd();
            if (!guardedInput || !bAtEnd) pState->bReadError = true;
        }
    }

    return nRead;
}

qint32 XBinary::_readDevice(DATAPROCESS_STATE *pState)
{
    if (!pState || !pState->pDeviceInput || (pState->nInputBufferSize < 0) || ((pState->nInputBufferSize > 0) && !pState->pInputBuffer) ||
        (pState->nCountInput < 0) || (pState->nInputLimit < -1) ||
        ((pState->nInputLimit != -1) && (pState->nCountInput > pState->nInputLimit))) {
        if (pState) {
            pState->bReadError = true;
        }
        return -1;
    }

    qint32 nReadSize = pState->nInputBufferSize;
    if (pState->nInputLimit != -1) {
        nReadSize = (qint32)(std::min)((qint64)nReadSize, pState->nInputLimit - pState->nCountInput);
    }
    QPointer<QIODevice> guardedInput(pState->pDeviceInput);
    const qint64 nReadResult = readDeviceWithBoundedProgress(
        guardedInput.data(), pState->pInputBuffer, nReadSize);

    if (!guardedInput || (nReadResult < 0) ||
        (nReadResult > (qint64)nReadSize) ||
        ((nReadResult > 0) && (pState->nCountInput > ((std::numeric_limits<qint64>::max)() - nReadResult)))) {
        pState->bReadError = true;
        return -1;
    }

    const qint32 nRead = (qint32)nReadResult;
    if (nRead > 0) {
        pState->nCountInput += nRead;
    } else if ((nReadSize > 0) &&
               ((pState->nInputLimit != -1) &&
                (pState->nCountInput < pState->nInputLimit))) {
        pState->bReadError = true;
    } else if ((nReadSize > 0) && (pState->nInputLimit == -1)) {
        if (!guardedInput) {
            pState->bReadError = true;
        } else {
            const bool bAtEnd = guardedInput->atEnd();
            if (!guardedInput || !bAtEnd) pState->bReadError = true;
        }
    }

    return nRead;
}

qint32 XBinary::_writeDevice(const char *pBuffer, qint32 nBufferSize, DATAPROCESS_STATE *pState)
{
    if (!pState || (nBufferSize < 0) || ((nBufferSize > 0) && !pBuffer)) {
        if (pState) {
            pState->bWriteError = true;
        }
        return 0;
    }

    const qint64 nWindowOffset = pState->nProcessedOffset;
    const qint64 nWindowSize = pState->nProcessedLimit;
    const qint64 nChunkStart = pState->nCountOutput;
    const qint64 nMax = (std::numeric_limits<qint64>::max)();

    // nProcessedLimit is the number of bytes requested after
    // nProcessedOffset, not an absolute end position.  Validate the state
    // before doing any arithmetic so a malformed window cannot wrap around.
    if ((nWindowOffset < 0) || (nWindowSize < -1) || (nChunkStart < 0) ||
        (nChunkStart > (nMax - nBufferSize)) || ((nWindowSize != -1) && (nWindowOffset > (nMax - nWindowSize)))) {
        pState->bWriteError = true;
        return 0;
    }

    const qint64 nChunkEnd = nChunkStart + nBufferSize;
    const qint64 nWindowEnd = (nWindowSize == -1) ? nMax : nWindowOffset + nWindowSize;
    const qint64 nWriteStart = (std::max)(nChunkStart, nWindowOffset);
    const qint64 nWriteEnd = (std::min)(nChunkEnd, nWindowEnd);
    const qint64 nSkip = (nWriteEnd > nWriteStart) ? (nWriteStart - nChunkStart) : 0;
    const qint64 nRealSize = (nWriteEnd > nWriteStart) ? (nWriteEnd - nWriteStart) : 0;

    bool bWriteOK = true;

    if (nRealSize > 0) {
        if (!pState->pDeviceOutput) {
            pState->bWriteError = true;
            bWriteOK = false;
        } else {
            qint64 nWrittenTotal = 0;
            QPointer<QIODevice> guardedOutput(pState->pDeviceOutput);
            if (!guardedOutput) {
                pState->bWriteError = true;
                bWriteOK = false;
            }
            if (!bWriteOK) {
                pState->nCountOutput = nChunkEnd;
                return 0;
            }
            const bool bSeekableOutput = !guardedOutput->isSequential();
            if (!guardedOutput) {
                pState->bWriteError = true;
                bWriteOK = false;
            }
            qint64 nOutputStart = -1;
            if (bWriteOK && bSeekableOutput) {
                nOutputStart = guardedOutput->pos();
                if (!guardedOutput) {
                    pState->bWriteError = true;
                    bWriteOK = false;
                }
            }

            if (!guardedOutput || (bSeekableOutput && (nOutputStart < 0))) {
                pState->bWriteError = true;
                bWriteOK = false;
            }

            // Positive short writes are legal for QIODevice (especially
            // sequential/custom devices).  Drain the complete chunk and fail
            // only when the device reports an error or makes no progress.  A
            // write callback may re-enter code that moves the same seekable
            // cursor, so restore the absolute continuation point before every
            // retry.
            while (bWriteOK && (nWrittenTotal < nRealSize)) {
                if (!guardedOutput ||
                    (bSeekableOutput &&
                     (nWrittenTotal > (nMax - nOutputStart)))) {
                    pState->bWriteError = true;
                    bWriteOK = false;
                    break;
                }
                if (bSeekableOutput) {
                    const bool bSeeked = guardedOutput->seek(
                        nOutputStart + nWrittenTotal);
                    if (!guardedOutput || !bSeeked) {
                        pState->bWriteError = true;
                        bWriteOK = false;
                        break;
                    }
                }

                qint64 nWritten = guardedOutput->write(
                    pBuffer + nSkip + nWrittenTotal,
                    nRealSize - nWrittenTotal);

                if (!guardedOutput || (nWritten <= 0) ||
                    (nWritten > (nRealSize - nWrittenTotal))) {
                    pState->bWriteError = true;
                    bWriteOK = false;
                    break;
                }

                nWrittenTotal += nWritten;
            }

            if (bWriteOK && bSeekableOutput &&
                (!guardedOutput ||
                 (nRealSize > (nMax - nOutputStart)))) {
                // A full final write can still run a re-entrant callback that
                // displaces the cursor.  Reassert the absolute continuation
                // point before reporting this produced chunk as successful.
                pState->bWriteError = true;
                bWriteOK = false;
            }
            if (bWriteOK && bSeekableOutput) {
                const bool bSeeked = guardedOutput->seek(
                    nOutputStart + nRealSize);
                if (!guardedOutput || !bSeeked) {
                    pState->bWriteError = true;
                    bWriteOK = false;
                }
            }
        }
    }

    pState->nCountOutput = nChunkEnd;

    // Preserve the historical return contract (the number of produced bytes,
    // including bytes skipped by a processed-output window) while making zero
    // unambiguously signal a write failure to decoder call sites.
    return bWriteOK ? nBufferSize : 0;
}

quint32 XBinary::getFPART_crc32(const FPART &fpart)
{
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);

    // Serialize basic types
    stream << fpart.nFileOffset;
    stream << fpart.nFileSize;
    stream << fpart.nVirtualAddress;
    stream << fpart.nVirtualSize;
    stream << fpart.sName;
    stream << static_cast<qint32>(fpart.filePart);

    // Serialize mapProperties
    stream << fpart.mapProperties;

    quint32 crc = _getCRC32(buffer.constData(), buffer.size(), 0xFFFFFFFF, XBinary::_getCRC32Table_EDB88320());

    return crc;
}

// XBinary::DATA_HEADER XBinary::_searchDataHeaderById(FT fileType, quint32 nID, const QList<DATA_HEADER> &listDataHeaders)
// {
//     XBinary::DATA_HEADER result = {};

//     qint32 nNumberOfRecords = listDataHeaders.count();

//     for (qint32 i = 0; i < nNumberOfRecords; i++) {
//         if ((listDataHeaders.at(i).dsID.fileType == fileType) && (listDataHeaders.at(i).dsID.nID == nID)) {
//             result = listDataHeaders.at(i);
//             break;
//         }
//     }

//     return result;
// }

// XBinary::DATA_HEADER XBinary::_searchDataHeaderByGuid(const QString &sGUID, const QList<DATA_HEADER> &listDataHeaders)
// {
//     XBinary::DATA_HEADER result = {};

//     qint32 nNumberOfRecords = listDataHeaders.count();

//     for (qint32 i = 0; i < nNumberOfRecords; i++) {
//         if (listDataHeaders.at(i).dsID.sGUID == sGUID) {
//             result = listDataHeaders.at(i);
//             break;
//         }
//     }

//     return result;
// }

XBinary::FPART XBinary::findParentFPart(const QList<FPART> &listHRegions, const FPART &hRegion)
{
    FPART result = {};

    qint32 nNumberOfRegions = listHRegions.size();

    for (qint32 i = 0; i < nNumberOfRegions; i++) {
        // if hRegion inside parent hRegion
        if ((listHRegions[i].nVirtualAddress <= hRegion.nVirtualAddress) &&
            (listHRegions[i].nVirtualAddress + listHRegions[i].nVirtualSize >= hRegion.nVirtualAddress + hRegion.nVirtualSize)) {
            result = listHRegions[i];
            break;
        }
    }

    return result;
}

XBinary::DATA_RECORD XBinary::getDataRecordDV(qint64 nRelOffset, qint64 nSize, const QString &sName, VT valType, quint32 nFlags, ENDIAN endian,
                                              QMap<quint64, QString> mapValues, VL_TYPE vlType)
{
    XBinary::DATA_RECORD result = getDataRecord(nRelOffset, nSize, sName, valType, nFlags, endian);

    DATAVALUESET dataValueSet;
    dataValueSet.mapValues = mapValues;
    dataValueSet.vlType = vlType;
    dataValueSet.nMask = 0xFFFFFFFFFFFFFFFF;

    result.listDataValueSets.append(dataValueSet);

    return result;
}

QString XBinary::structIDToString(quint32 nID)
{
    return XBinary::XCONVERT_idToTransString(nID, _TABLE_XBINARY_STRUCTID, sizeof(_TABLE_XBINARY_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QString XBinary::structIDToFtString(quint32 nID)
{
    return XBinary::XCONVERT_idToFtString(nID, _TABLE_XBINARY_STRUCTID, sizeof(_TABLE_XBINARY_STRUCTID) / sizeof(XBinary::XCONVERT));
}

quint32 XBinary::ftStringToStructID(const QString &sFtString)
{
    return XCONVERT_ftStringToId(sFtString, _TABLE_XBINARY_STRUCTID, sizeof(_TABLE_XBINARY_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QList<XBinary::XFHEADER> XBinary::_getXFHeaders(PDSTRUCT *pPdStruct)
{
    _MEMORY_MAP memoryMap = getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);

    XFSTRUCT xfStruct = {};
    xfStruct.bIsParent = true;
    xfStruct.fileType = getFileType();
    xfStruct.nStructID = 0;
    xfStruct.pMemoryMap = &memoryMap;

    return getXFHeaders(xfStruct, pPdStruct);
}

// XBinary::DATA_HEADER XBinary::_dataHeaderHex(const DATA_HEADERS_OPTIONS &dataHeadersOptions, const QString &sName, const DSID &dsID_parent, quint32 nID, qint64
// nOffset,
//                                              qint64 nSize)
// {
//     XBinary::DATA_HEADER result = _initDataHeader(dataHeadersOptions, sName);
//     result.dsID_parent = dsID_parent;
//     result.dsID.nID = nID;
//     result.nLocation = nOffset;
//     result.nSize = nSize;
//     result.locType = LT_OFFSET;
//     result.dhMode = DHMODE_HEX;
//     result.nCount = 1;  // Hex view is always one record

//     return result;
// }

// XBinary::DATA_HEADER XBinary::_initDataHeader(const DATA_HEADERS_OPTIONS &dataHeadersOptions, const QString &sName)
// {
//     DATA_HEADER result = {};

//     result.dsID_parent = dataHeadersOptions.dsID_parent;
//     result.dsID.sGUID = generateUUID();
//     result.dsID.fileType = dataHeadersOptions.fileType;
//     result.dsID.nID = dataHeadersOptions.nID;
//     result.locType = dataHeadersOptions.locType;
//     result.nLocation = dataHeadersOptions.nLocation;
//     result.sName = sName;
//     result.dhMode = dataHeadersOptions.dhMode;
//     result.nSize = dataHeadersOptions.nSize;
//     result.nCount = dataHeadersOptions.nCount;

//     if (result.nCount == 0) {
//         result.nCount = 1;
//     }

//     if (result.dsID.fileType == FT_UNKNOWN) {
//         result.dsID.fileType = dataHeadersOptions.pMemoryMap->fileType;
//     }

//     return result;
// }

// XBinary::DSID XBinary::_addDefaultHeaders(QList<DATA_HEADER> *pListHeaders, PDSTRUCT *pPdStruct)
// {
//     DATA_HEADER dhInfo = {};
//     dhInfo.dsID_parent.fileType = FT_UNKNOWN;
//     dhInfo.dsID_parent.nID = 0;
//     dhInfo.dsID_parent.sGUID = generateUUID();
//     dhInfo.dsID.fileType = getFileType();
//     dhInfo.dsID.nID = 0;
//     dhInfo.dsID.sGUID = generateUUID();
//     dhInfo.sName = QObject::tr("Info");
//     dhInfo.locType = LT_UNKNOWN;
//     dhInfo.nLocation = 0;
//     dhInfo.nSize = -1;

//     pListHeaders->append(dhInfo);

//     if (XBinary::isPdStructNotCanceled(pPdStruct)) {
//         DATA_HEADER dhGeneric = {};
//         dhGeneric.dsID_parent = dhInfo.dsID;
//         dhGeneric.dsID.fileType = FT_BINARY;
//         dhGeneric.locType = LT_UNKNOWN;
//         dhGeneric.nLocation = 0;
//         dhGeneric.nSize = -1;

//         if (XBinary::isPdStructNotCanceled(pPdStruct)) {
//             dhGeneric.dsID.nID = STRUCTID_NFDSCAN;
//             dhGeneric.dsID.sGUID = generateUUID();
//             dhGeneric.sName = XBinary::structIDToString(dhGeneric.dsID.nID);
//             pListHeaders->append(dhGeneric);
//         }
//         if (XBinary::isPdStructNotCanceled(pPdStruct)) {
//             dhGeneric.dsID.nID = STRUCTID_DIESCAN;
//             dhGeneric.dsID.sGUID = generateUUID();
//             dhGeneric.sName = XBinary::structIDToString(dhGeneric.dsID.nID);
//             pListHeaders->append(dhGeneric);
//         }
//         if (XBinary::isPdStructNotCanceled(pPdStruct)) {
//             dhGeneric.dsID.nID = STRUCTID_YARASCAN;
//             dhGeneric.dsID.sGUID = generateUUID();
//             dhGeneric.sName = XBinary::structIDToString(dhGeneric.dsID.nID);
//             pListHeaders->append(dhGeneric);
//         }
//         if (XBinary::isPdStructNotCanceled(pPdStruct)) {
//             dhGeneric.dsID.nID = STRUCTID_VIRUSTOTALSCAN;
//             dhGeneric.dsID.sGUID = generateUUID();
//             dhGeneric.sName = XBinary::structIDToString(dhGeneric.dsID.nID);
//             pListHeaders->append(dhGeneric);
//         }
//         if (XBinary::isPdStructNotCanceled(pPdStruct)) {
//             dhGeneric.dsID.nID = STRUCTID_VISUALIZATION;
//             dhGeneric.dsID.sGUID = generateUUID();
//             dhGeneric.sName = XBinary::structIDToString(dhGeneric.dsID.nID);
//             pListHeaders->append(dhGeneric);
//         }
//         if (XBinary::isPdStructNotCanceled(pPdStruct)) {
//             dhGeneric.dsID.nID = STRUCTID_HEX;
//             dhGeneric.dsID.sGUID = generateUUID();
//             dhGeneric.sName = XBinary::structIDToString(dhGeneric.dsID.nID);
//             pListHeaders->append(dhGeneric);
//         }
//         if (isExecutable() && XBinary::isPdStructNotCanceled(pPdStruct)) {
//             dhGeneric.dsID.nID = STRUCTID_DISASM;
//             dhGeneric.dsID.sGUID = generateUUID();
//             dhGeneric.sName = XBinary::structIDToString(dhGeneric.dsID.nID);
//             pListHeaders->append(dhGeneric);
//         }
//         if (isArchive() && XBinary::isPdStructNotCanceled(pPdStruct)) {
//             dhGeneric.dsID.nID = STRUCTID_ARCHIVE;
//             dhGeneric.dsID.sGUID = generateUUID();
//             dhGeneric.sName = XBinary::structIDToString(dhGeneric.dsID.nID);
//             pListHeaders->append(dhGeneric);
//         }
//         if (XBinary::isPdStructNotCanceled(pPdStruct)) {
//             dhGeneric.dsID.nID = STRUCTID_HASH;
//             dhGeneric.dsID.sGUID = generateUUID();
//             dhGeneric.sName = XBinary::structIDToString(dhGeneric.dsID.nID);
//             pListHeaders->append(dhGeneric);
//         }
//         if (XBinary::isPdStructNotCanceled(pPdStruct)) {
//             dhGeneric.dsID.nID = STRUCTID_STRINGS;
//             dhGeneric.dsID.sGUID = generateUUID();
//             dhGeneric.sName = XBinary::structIDToString(dhGeneric.dsID.nID);
//             pListHeaders->append(dhGeneric);
//         }
//         if (XBinary::isPdStructNotCanceled(pPdStruct)) {
//             dhGeneric.dsID.nID = STRUCTID_SIGNATURES;
//             dhGeneric.dsID.sGUID = generateUUID();
//             dhGeneric.sName = XBinary::structIDToString(dhGeneric.dsID.nID);
//             pListHeaders->append(dhGeneric);
//         }
//         if (XBinary::isPdStructNotCanceled(pPdStruct)) {
//             dhGeneric.dsID.nID = STRUCTID_REGIONS;
//             dhGeneric.dsID.sGUID = generateUUID();
//             dhGeneric.sName = XBinary::structIDToString(dhGeneric.dsID.nID);
//             pListHeaders->append(dhGeneric);
//         }
//         if (XBinary::isPdStructNotCanceled(pPdStruct)) {
//             dhGeneric.dsID.nID = STRUCTID_MEMORYMAP;
//             dhGeneric.dsID.sGUID = generateUUID();
//             dhGeneric.sName = XBinary::structIDToString(dhGeneric.dsID.nID);
//             pListHeaders->append(dhGeneric);
//         }
//         if (XBinary::isPdStructNotCanceled(pPdStruct)) {
//             dhGeneric.dsID.nID = STRUCTID_SYMBOLS;
//             dhGeneric.dsID.sGUID = generateUUID();
//             dhGeneric.sName = XBinary::structIDToString(dhGeneric.dsID.nID);
//             pListHeaders->append(dhGeneric);
//         }
//         if (XBinary::isPdStructNotCanceled(pPdStruct)) {
//             dhGeneric.dsID.nID = STRUCTID_ENTROPY;
//             dhGeneric.dsID.sGUID = generateUUID();
//             dhGeneric.sName = XBinary::structIDToString(dhGeneric.dsID.nID);
//             pListHeaders->append(dhGeneric);
//         }
//         if (XBinary::isPdStructNotCanceled(pPdStruct)) {
//             dhGeneric.dsID.nID = STRUCTID_EXTRACTOR;
//             dhGeneric.dsID.sGUID = generateUUID();
//             dhGeneric.sName = XBinary::structIDToString(dhGeneric.dsID.nID);
//             pListHeaders->append(dhGeneric);
//         }
//         if (XBinary::isPdStructNotCanceled(pPdStruct)) {
//             dhGeneric.dsID.nID = STRUCTID_SEARCH;
//             dhGeneric.dsID.sGUID = generateUUID();
//             dhGeneric.sName = XBinary::structIDToString(dhGeneric.dsID.nID);
//             pListHeaders->append(dhGeneric);
//         }
//         if (getFileParts(FILEPART_OVERLAY, 1, pPdStruct).count()) {
//             dhGeneric.dsID.nID = STRUCTID_OVERLAY;
//             dhGeneric.dsID.sGUID = generateUUID();
//             dhGeneric.sName = XBinary::structIDToString(dhGeneric.dsID.nID);
//             pListHeaders->append(dhGeneric);
//         }
//     }

//     return dhInfo.dsID;
// }

// qint32 XBinary::getDataRecordValues(const DATA_RECORDS_OPTIONS &dataRecordsOptions, QList<DATA_RECORD_ROW> *pListDataRecords, QList<QString> *pListTitles,
//                                     PDSTRUCT *pPdStruct)
// {
//     qint32 nResult = 0;

//     if (pListTitles) {
//         pListTitles->clear();
//     }

//     if (dataRecordsOptions.dataHeaderFirst.dhMode == DHMODE_HEADER) {
//         DATA_RECORD_ROW dataRecordRow = {};
//         dataRecordRow.locType = dataRecordsOptions.dataHeaderFirst.locType;
//         dataRecordRow.nLocation = dataRecordsOptions.dataHeaderFirst.nLocation;

//         DATA_HEADER dataHeader = {};

//         if (_isFlagPresentInRecords(&(dataRecordsOptions.dataHeaderFirst.listRecords), DRF_VOLATILE)) {
//             DATA_HEADERS_OPTIONS dataHeadersOptions = {};
//             dataHeadersOptions.pMemoryMap = dataRecordsOptions.pMemoryMap;
//             dataHeadersOptions.dsID_parent = dataRecordsOptions.dataHeaderFirst.dsID_parent;
//             dataHeadersOptions.fileType = dataRecordsOptions.dataHeaderFirst.dsID.fileType;
//             dataHeadersOptions.nID = dataRecordsOptions.dataHeaderFirst.dsID.nID;
//             dataHeadersOptions.locType = dataRecordsOptions.dataHeaderFirst.locType;
//             dataHeadersOptions.nLocation = dataRecordsOptions.dataHeaderFirst.nLocation;
//             dataHeadersOptions.bChildren = false;
//             dataHeadersOptions.dhMode = DHMODE_HEADER;

//             QList<DATA_HEADER> listDataHeaders = getDataHeaders(dataHeadersOptions, pPdStruct);

//             if (listDataHeaders.count()) {
//                 dataHeader = listDataHeaders.at(0);
//             }
//         } else {
//             dataHeader = dataRecordsOptions.dataHeaderFirst;
//         }

//         qint64 nStartOffset = locationToOffset(dataRecordsOptions.pMemoryMap, dataHeader.locType, dataHeader.nLocation);
//         qint32 nNumberOfRecords = dataHeader.listRecords.count();

//         qint32 nMax = 0;

//         for (qint32 j = 0; (j < nNumberOfRecords) && XBinary::isPdStructNotCanceled(pPdStruct); j++) {
//             DATA_RECORD dataRecord = dataHeader.listRecords.at(j);

//             QVariant variant =
//                 read_value(dataRecord.valType, nStartOffset + dataRecord.nRelOffset, dataRecord.nSize, dataRecord.endian == XBinary::ENDIAN_BIG, pPdStruct);

//             dataRecordRow.listValues.append(variant);

//             if (pListTitles) {
//                 pListTitles->append(dataRecord.sName);
//             }

//             if (dataRecord.nRelOffset + dataRecord.nSize > nMax) {
//                 nMax = dataRecord.nRelOffset + dataRecord.nSize;
//             }
//         }

//         dataRecordRow.nSize = nMax;

//         pListDataRecords->append(dataRecordRow);

//         nResult = nMax;
//     } else if (dataRecordsOptions.dataHeaderFirst.dhMode == DHMODE_TABLE) {
//         qint32 nCount = dataRecordsOptions.dataHeaderFirst.nCount;
//         XADDR nLocation = dataRecordsOptions.dataHeaderFirst.nLocation;

//         if (pListTitles) {
//             pListTitles->clear();
//             pListTitles->append(getTableTitles(dataRecordsOptions));
//         }

//         void *pUserData = nullptr;
//         if (readTableInit(dataRecordsOptions, &pUserData, pPdStruct)) {
//             for (qint32 i = 0; (i < nCount) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
//                 QList<DATA_RECORD_ROW> listDataRecordRows;

//                 qint32 nResultRead =
//                     readTableRow(i, dataRecordsOptions.dataHeaderFirst.locType, nLocation, dataRecordsOptions, &listDataRecordRows, pUserData, pPdStruct);

//                 pListDataRecords->append(listDataRecordRows);

//                 nLocation += nResultRead;
//             }

//             readTableFinalize(dataRecordsOptions, pUserData, pPdStruct);
//         }

//         nResult = nLocation - dataRecordsOptions.dataHeaderFirst.nLocation;
//     }

//     return nResult;
// }

// QList<QString> XBinary::getDataRecordComments(const DATA_RECORDS_OPTIONS &dataRecordsOptions, const DATA_RECORD_ROW &dataRecordRow, PDSTRUCT *pPdStruct)
// {
//     QList<QString> listComments;

//     qint32 nNumberOfRecords = dataRecordsOptions.dataHeaderFirst.listRecords.count();

//     for (qint32 j = 0; (j < nNumberOfRecords) && XBinary::isPdStructNotCanceled(pPdStruct); j++) {
//         DATA_RECORD dataRecord = dataRecordsOptions.dataHeaderFirst.listRecords.at(j);

//         QString sComment;

//         if (dataRecord.nFlags & DRF_SIZE) {
//             sComment = appendText(bytesCountToString(dataRecordRow.listValues.at(j).toULongLong(), 1024), sComment, ", ");
//         }

//         if (dataRecord.nFlags & DRF_COUNT) {
//             sComment = appendText(QString::number(dataRecordRow.listValues.at(j).toULongLong()), sComment, ", ");
//         }

//         QString sFlags;
//         if (dataRecord.listDataValueSets.count()) {
//             qint32 nNumberOfDataValueSets = dataRecord.listDataValueSets.count();

//             for (qint32 k = 0; (k < nNumberOfDataValueSets) && XBinary::isPdStructNotCanceled(pPdStruct); k++) {
//                 DATAVALUESET dataValueSet = dataRecord.listDataValueSets.at(k);

//                 sFlags = appendText(
//                     sFlags, valueToFlagsString(dataRecordRow.listValues.at(j).toULongLong() & dataValueSet.nMask, dataValueSet.mapValues, dataValueSet.vlType), "|");
//             }

//             sComment = appendText(sComment, sFlags, ", ");
//         }

//         listComments.append(sComment);
//     }

//     return listComments;
// }

// QList<QString> XBinary::getTableTitles(const DATA_RECORDS_OPTIONS &dataRecordsOptions)
// {
//     QList<QString> listTitles;

//     qint32 nNumberOfRecords = dataRecordsOptions.dataHeaderFirst.listRecords.count();

//     for (qint32 j = 0; j < nNumberOfRecords; j++) {
//         DATA_RECORD dataRecord = dataRecordsOptions.dataHeaderFirst.listRecords.at(j);

//         listTitles.append(dataRecord.sName);
//     }

//     return listTitles;
// }

// bool XBinary::readTableInit(const DATA_RECORDS_OPTIONS &dataRecordsOptions, void **ppUserData, PDSTRUCT *pPdStruct)
// {
//     Q_UNUSED(dataRecordsOptions)
//     Q_UNUSED(ppUserData)
//     Q_UNUSED(pPdStruct)

//     // Default implementation does nothing
//     return true;
// }

// qint32 XBinary::readTableRow(qint32 nRow, LT locType, XADDR nLocation, const DATA_RECORDS_OPTIONS &dataRecordsOptions, QList<DATA_RECORD_ROW> *pListDataRecords,
//                              void *pUserData, PDSTRUCT *pPdStruct)
// {
//     Q_UNUSED(nRow)
//     Q_UNUSED(pUserData)

//     DATA_RECORDS_OPTIONS _dataRecordsOptions = dataRecordsOptions;
//     _dataRecordsOptions.dataHeaderFirst.locType = locType;
//     _dataRecordsOptions.dataHeaderFirst.nLocation = nLocation;
//     _dataRecordsOptions.dataHeaderFirst.dhMode = DHMODE_HEADER;

//     return getDataRecordValues(_dataRecordsOptions, pListDataRecords, nullptr, pPdStruct);
// }

// void XBinary::readTableFinalize(const DATA_RECORDS_OPTIONS &dataRecordsOptions, void *pUserData, PDSTRUCT *pPdStruct)
// {
//     Q_UNUSED(dataRecordsOptions)
//     Q_UNUSED(pUserData)
//     Q_UNUSED(pPdStruct)

//     // Default implementation does nothing
// }

bool XBinary::_isFlagPresentInRecords(const QList<DATA_RECORD> *pListRecords, quint32 nFlag)
{
    bool bResult = false;

    if (!pListRecords) {
        return bResult;
    }

    qint32 nNumberOfRecords = pListRecords->count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (pListRecords->at(i).nFlags & nFlag) {
            bResult = true;
            break;
        }
    }

    return bResult;
}

QString XBinary::getCompressMethodString()
{
    return "";
}

QString XBinary::handleMethodToString(HANDLE_METHOD handleMethod)
{
    return XBinary::XCONVERT_idToTransString(handleMethod, _TABLE_XBINARY_HANDLE_METHOD, sizeof(_TABLE_XBINARY_HANDLE_METHOD) / sizeof(XBinary::XCONVERT));
}

QString XBinary::handleMethodToFtString(HANDLE_METHOD handleMethod)
{
    return XBinary::XCONVERT_idToFtString(handleMethod, _TABLE_XBINARY_HANDLE_METHOD, sizeof(_TABLE_XBINARY_HANDLE_METHOD) / sizeof(XBinary::XCONVERT));
}

XBinary::HANDLE_METHOD XBinary::ftStringToHandleMethod(const QString &sString)
{
    return (HANDLE_METHOD)XBinary::XCONVERT_ftStringToId(sString, _TABLE_XBINARY_HANDLE_METHOD, sizeof(_TABLE_XBINARY_HANDLE_METHOD) / sizeof(XBinary::XCONVERT));
}

XBinary::DATA_RECORD XBinary::getDataRecord(qint64 nRelOffset, qint64 nSize, const QString &sName, VT valType, quint32 nFlags, ENDIAN endian)
{
    XBinary::DATA_RECORD dataRecord;

    dataRecord.nRelOffset = nRelOffset;
    dataRecord.nSize = nSize;
    dataRecord.sName = sName;
    dataRecord.valType = valType;
    dataRecord.nFlags = nFlags;
    dataRecord.endian = endian;

    return dataRecord;
}

// QList<XBinary::DATA_HEADER> XBinary::getDataHeaders(const DATA_HEADERS_OPTIONS &dataHeadersOptions, PDSTRUCT *pPdStruct)
// {
//     QList<XBinary::DATA_HEADER> listResult;

//     if (dataHeadersOptions.nID == 0) {
//         _addDefaultHeaders(&listResult, pPdStruct);
//     }

//     return listResult;
// }

QList<XBinary::XFHEADER> XBinary::getXFHeaders(const XFSTRUCT &xfStruct, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(xfStruct)
    Q_UNUSED(pPdStruct)

    QList<XBinary::XFHEADER> listResult;

    return listResult;
}

QList<XBinary::XFRECORD> XBinary::getXFRecords(FT fileType, quint32 nStructID, const XLOC &xLoc)
{
    Q_UNUSED(fileType)
    Q_UNUSED(nStructID)
    Q_UNUSED(xLoc)

    QList<XBinary::XFRECORD> listResult;

    return listResult;
}

QList<XBinary::XFRECORD> XBinary::XFIXEDFIELD_toXFRecords(const XFIXEDFIELD *pRecords, qint32 nRecordsSize)
{
    QList<XBinary::XFRECORD> listResult;

    if (pRecords && (nRecordsSize > 0)) {
        for (qint32 i = 0; i < nRecordsSize; i++) {
            const XFIXEDFIELD &record = pRecords[i];

            XBinary::XFRECORD xfRecord = {};
            xfRecord.sName = (record.pszName) ? QString::fromLatin1(record.pszName) : QString();
            xfRecord.nOffset = record.nOffset;
            xfRecord.nSize = record.nSize;
            xfRecord.nFlags = record.nFlags;
            xfRecord.valueType = record.valueType;

            listResult.append(xfRecord);
        }
    }

    return listResult;
}

QList<QVariant> XBinary::getXFRecordValues(const QList<XFRECORD> &listXFRecords, const XLOC &xLoc)
{
    QList<QVariant> listResult;

    _MEMORY_MAP memoryMap = getMemoryMap();
    qint64 nBaseOffset = locToOffset(&memoryMap, xLoc);
    qint32 nCount = listXFRecords.count();
    bool bIsBigEndian = isBigEndian();

    if ((nBaseOffset < 0) || (nBaseOffset > getSize())) {
        return listResult;
    }

    for (qint32 i = 0; i < nCount; i++) {
        XFRECORD xfRecord = listXFRecords.at(i);
        const qint64 nRelativeOffset = (qint64)xfRecord.nOffset;
        if (((nRelativeOffset > 0) && (nBaseOffset > (std::numeric_limits<qint64>::max)() - nRelativeOffset)) ||
            ((nRelativeOffset < 0) && (nBaseOffset < (std::numeric_limits<qint64>::min)() - nRelativeOffset))) {
            listResult.append(QVariant());
            continue;
        }

        qint64 nOffset = nBaseOffset + nRelativeOffset;
        QVariant varValue;

        bool _bIsBigEndian = bIsBigEndian;

        if (xfRecord.nFlags & XFRECORD_FLAG_LE) {
            _bIsBigEndian = false;
        } else if (xfRecord.nFlags & XFRECORD_FLAG_BE) {
            _bIsBigEndian = true;
        }

        qint64 nRequiredSize = xfRecord.nSize;
        if ((xfRecord.valueType == VT_UINT8) || (xfRecord.valueType == VT_INT8)) {
            nRequiredSize = 1;
        } else if ((xfRecord.valueType == VT_UINT16) || (xfRecord.valueType == VT_INT16)) {
            nRequiredSize = 2;
        } else if ((xfRecord.valueType == VT_UINT32) || (xfRecord.valueType == VT_INT32) || (xfRecord.valueType == VT_FLOAT)) {
            nRequiredSize = 4;
        } else if ((xfRecord.valueType == VT_UINT64) || (xfRecord.valueType == VT_INT64) || (xfRecord.valueType == VT_DOUBLE)) {
            nRequiredSize = 8;
        }

        const qint64 nDeviceSize = getSize();
        if ((nOffset < 0) || (nRequiredSize < 0) || (nOffset > nDeviceSize) ||
            (nRequiredSize > (nDeviceSize - nOffset))) {
            listResult.append(QVariant());
            continue;
        }

        if (xfRecord.valueType == VT_UINT8) {
            varValue = read_uint8(nOffset);
        } else if (xfRecord.valueType == VT_INT8) {
            varValue = read_int8(nOffset);
        } else if (xfRecord.valueType == VT_UINT16) {
            varValue = read_uint16(nOffset, _bIsBigEndian);
        } else if (xfRecord.valueType == VT_INT16) {
            varValue = read_int16(nOffset, _bIsBigEndian);
        } else if (xfRecord.valueType == VT_UINT32) {
            varValue = read_uint32(nOffset, _bIsBigEndian);
        } else if (xfRecord.valueType == VT_INT32) {
            varValue = read_int32(nOffset, _bIsBigEndian);
        } else if (xfRecord.valueType == VT_UINT64) {
            varValue = read_uint64(nOffset, _bIsBigEndian);
        } else if (xfRecord.valueType == VT_INT64) {
            varValue = read_int64(nOffset, _bIsBigEndian);
        } else if (xfRecord.valueType == VT_FLOAT) {
            varValue = read_float(nOffset, _bIsBigEndian);
        } else if (xfRecord.valueType == VT_DOUBLE) {
            varValue = read_double(nOffset, _bIsBigEndian);
        } else if (xfRecord.valueType == VT_BYTE_ARRAY) {
            varValue = read_array(nOffset, xfRecord.nSize);
        } else if (xfRecord.valueType == VT_CHAR_ARRAY) {
            varValue = read_ansiString(nOffset, xfRecord.nSize);
        } else {
            varValue = read_array(nOffset, xfRecord.nSize);
        }

        listResult.append(varValue);
    }

    return listResult;
}

QString XBinary::xfHeaderToTag(const XFHEADER &xfHeader, const QString &sStructName, const QString &sParentTag)
{
    QString sOffset = valueToHexEx(xfHeader.xLoc.nLocation);
    QString sFileTypeFt = fileTypeIdToFtString(xfHeader.fileType);
    QString sStructFt = QString(sStructName).toUpper().remove(" ").remove("-");

    QString sTypeName;

    if (xfHeader.xfType == XFTYPE_HEADER) {
        sTypeName = "HEADER";
    } else if (xfHeader.xfType == XFTYPE_TABLE) {
        sTypeName = "TABLE";
    } else if (xfHeader.xfType == XFTYPE_COMMAND) {
        sTypeName = "COMMAND";
    } else {
        sTypeName = "UNKNOWN";
    }

    QString sResult = sOffset + "::" + sFileTypeFt + "::" + sStructFt + "::" + sTypeName;

    if (xfHeader.xfType == XFTYPE_TABLE) {
        sResult += "::" + QString::number(xfHeader.listRowLocations.count());
    }

    if (xfHeader.bIsParentNeeded && (sParentTag != "")) {
        sResult = sParentTag + "#" + sResult;
    }

    return sResult;
}

QString XBinary::xfHeaderToString(const XFHEADER &xfHeader, const QString &sStructName, const QString &sParentString)
{
    QString sResult;

    if (xfHeader.fileType != FT_UNKNOWN) {
        sResult += QString("%1::").arg(fileTypeIdToFtString(xfHeader.fileType));
    }

    sResult += QString(sStructName).toUpper().remove(" ").remove("-");

    QList<QString> listParams;

    if (xfHeader.xfType != XFTYPE_UNKNOWN) {
        listParams.append(QString("type=%1").arg(xfTypeIdToFtString(xfHeader.xfType)));
    }

    if (xfHeader.xLoc.nLocation > 0) {
        listParams.append(QString("offset=0x%1").arg(valueToHexEx(xfHeader.xLoc.nLocation)));
    }

    qint64 nSize = xfHeader.nSize;

    if (nSize <= 0) {
        qint32 nNumberOfFields = xfHeader.listFields.count();

        for (qint32 i = 0; i < nNumberOfFields; i++) {
            const XFRECORD &record = xfHeader.listFields.at(i);
            if ((record.nOffset >= 0) && (record.nSize >= 0)) {
                nSize = qMax(nSize, (qint64)record.nOffset + (qint64)record.nSize);
            }
        }
    }

    qint32 nNumberOfRows = xfHeader.listRowLocations.count();

    if ((xfHeader.xfType == XFTYPE_TABLE) && (nSize > 0) && (nNumberOfRows > 0)) {
        if (nSize > (std::numeric_limits<qint64>::max)() / nNumberOfRows) {
            nSize = (std::numeric_limits<qint64>::max)();
        } else {
            nSize *= nNumberOfRows;
        }
    }

    if (nSize > 0) {
        listParams.append(QString("size=0x%1").arg(valueToHexEx(nSize)));
    }

    if (nNumberOfRows > 0) {
        listParams.append(QString("rows=0x%1").arg(valueToHexEx(nNumberOfRows)));
    }

    if (xfHeader.bIsParentNeeded && (sParentString != "")) {
        sResult = sParentString + "#" + sResult;
    }

    if (listParams.count()) {
        sResult += "?" + listParams.join("&");
    }

    return sResult;
}

XBinary::XBinary(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
    : m_nDeviceGeneration(0)
{
    setData(pDevice, bIsImage, nModuleAddress);
}

XBinary::XBinary(const QString &sFileName)
    : m_nDeviceGeneration(0)
{
    QFile *pFile = new QFile(sFileName);

    tryToOpen(pFile);

    setData(pFile);
    m_sFileName = sFileName;
    m_pFile = pFile;
}

XBinary::~XBinary()
{
    QFile *pOwnedFile = m_pFile.data();
    m_pFile.clear();
    if (pOwnedFile) {
        pOwnedFile->close();
        delete pOwnedFile;
    }
}

void XBinary::setData(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    QPointer<XBinary> guardedThis(this);
    if (!isDeviceReplacementAllowed() || !guardedThis) return;

    m_pReadWriteMutex = nullptr;
    m_nSize = 0;
    m_nFileFormatSize = 0;
    m_pConstMemory = nullptr;

    const quint64 nGenerationBefore = m_nDeviceGeneration;
    setDevice(pDevice);
    if (!guardedThis) return;
    quint64 nExpectedGeneration = nGenerationBefore + 1;
    if (nExpectedGeneration == 0) nExpectedGeneration = 1;
    if ((m_nDeviceGeneration != nExpectedGeneration) ||
        (m_pDevice.data() != pDevice)) {
        // setDevice() invoked a caller-controlled size() implementation.  A
        // re-entrant setData()/setDevice() won ownership of the object; never
        // overwrite its freshly established metadata with the outer call.
        return;
    }
    setIsImage(bIsImage);
    XBinary::setBaseAddress(0);
    setModuleAddress(nModuleAddress);
    setEndian(ENDIAN_UNKNOWN);
    XBinary::setEntryPointOffset(0);
    setMode(MODE_UNKNOWN);
    setFileType(FT_BINARY);
    setArch("NOEXEC");
    setVersion("");
    setType(TYPE_UNKNOWN);
    setOsType(OSNAME_UNKNOWN);
    setOsVersion("");
    setMultiSearchCallbackState(false);
    setIsExecutable(false);
    setIsArchive(false);
    setIsInternalInfoHandled(false);

    if (pDevice && (pDevice == m_pDevice.data())) {
        // qDebug("%s",XBinary::valueToHex((quint64)pDevice).toLatin1().data());
        const quint64 nGeneration = m_nDeviceGeneration;
        const qint64 nDeviceSize = pDevice->size();
        if ((nGeneration == m_nDeviceGeneration) &&
            (pDevice == m_pDevice.data())) {
            setFileFormatSize(nDeviceSize);
        }
    }
}

void XBinary::setDevice(QIODevice *pDevice)
{
    QPointer<XBinary> guardedThis(this);
    if (!isDeviceReplacementAllowed() || !guardedThis) return;
    ++m_nDeviceGeneration;
    if (m_nDeviceGeneration == 0) ++m_nDeviceGeneration;
    m_pDevice = pDevice;
    m_pConstMemory = nullptr;
    m_nSize = 0;
    m_nFileFormatSize = 0;

    // A parsed internal-info object is tied to the previous device contents.
    // Invalidate through the virtual setter so format-specific caches cannot
    // survive an explicit device replacement (including replacement by null).
    setInternalInfo(nullptr);
    if (!guardedThis) return;

    QPointer<QIODevice> guardedDevice = m_pDevice;
    if (guardedDevice) {
        // Do not cache pointers obtained from QBuffer::buffer().  Even a buffer
        // opened read-only can have its backing QByteArray replaced or detached
        // by its owner without going through this XBinary instance.  Device
        // reads preserve the same semantics without retaining borrowed storage.
        // qDebug("%s",XBinary::valueToHex((quint64)m_pDevice).toLatin1().data());
        const quint64 nGeneration = m_nDeviceGeneration;
        const qint64 nDeviceSize = guardedDevice->size();
        if (!guardedThis || !guardedDevice ||
            (nGeneration != guardedThis->m_nDeviceGeneration) ||
            (guardedDevice.data() != guardedThis->m_pDevice.data())) return;

        QMutex *pReadWriteMutex = guardedThis->m_pReadWriteMutex;
        QMutexLocker locker(pReadWriteMutex);
        if (guardedThis &&
            (nGeneration == guardedThis->m_nDeviceGeneration) &&
            guardedDevice &&
            (guardedDevice.data() == guardedThis->m_pDevice.data())) {
            guardedThis->m_nSize = nDeviceSize;
            guardedThis->m_nFileFormatSize = nDeviceSize;
        }
    }
}

void XBinary::setReadWriteMutex(QMutex *pReadWriteMutex)
{
    m_pReadWriteMutex = pReadWriteMutex;
}

void XBinary::setFileName(const QString &sFileName)
{
    m_sFileName = sFileName;
}

qint64 XBinary::safeReadData(QIODevice *pDevice, qint64 nPos, char *pData, qint64 nMaxLen, PDSTRUCT *pPdStruct)
{
    qint64 nResult = 0;
    QPointer<XBinary> guardedThis(this);

    if (!pDevice || (nPos < 0) || (nMaxLen < 0) ||
        ((nMaxLen > 0) && !pData) ||
        !isPdStructNotCanceled(pPdStruct)) {
        return nResult;
    }

    QPointer<QIODevice> guardedDevice(pDevice);
    const bool bOpen = guardedDevice->isOpen();
    if (!guardedThis || !guardedDevice || !bOpen) return nResult;
    const bool bReadable = guardedDevice->isReadable();
    if (!guardedThis || !guardedDevice || !bReadable) return nResult;
    const bool bSequential = guardedDevice->isSequential();
    if (!guardedThis || !guardedDevice || bSequential) return nResult;
    const QIODevice::OpenMode openMode = guardedDevice->openMode();
    if (!guardedThis || !guardedDevice ||
        openMode.testFlag(QIODevice::Text)) return nResult;

    const qint32 nRequestedBufferSize = getBufferSize(pPdStruct);

    if (nRequestedBufferSize <= 0) {
        return nResult;
    }

    const qint64 nBufferSize = qBound((qint32)0x1000, nRequestedBufferSize, (qint32)0x100000);
    QMutex *pReadWriteMutex = guardedThis->m_pReadWriteMutex;
    QMutexLocker locker(pReadWriteMutex);
    bool bPositionEstablished = false;

    const qint64 nDeviceSize = guardedDevice ? guardedDevice->size() : -1;
    if (!guardedThis || !guardedDevice) return 0;
    if ((nMaxLen == 0) || (nDeviceSize > nPos)) {
        if (nMaxLen == 0) {
            const bool bSeeked = guardedDevice->seek(nPos);
            bPositionEstablished = guardedDevice && bSeeked;
        } else {
            while ((nMaxLen > 0) && isPdStructNotCanceled(pPdStruct)) {
                const qint64 nRequestedSize = qMin(nMaxLen, nBufferSize);
                // A custom device may synchronously re-enter public read APIs
                // and move this same cursor while returning a short read.
                // Reassert the absolute position before every drained chunk.
                if (nResult > (std::numeric_limits<qint64>::max)() - nPos)
                    break;
                if (!guardedThis || !guardedDevice) break;
                const bool bSeeked = guardedDevice->seek(nPos + nResult);
                if (!guardedThis || !guardedDevice || !bSeeked) break;
                bPositionEstablished = true;
                const qint64 nCurrentSize = guardedDevice->read(pData, nRequestedSize);

                if (!guardedThis || !guardedDevice || (nCurrentSize <= 0) ||
                    (nCurrentSize > nRequestedSize)) {
                    break;
                }

                nMaxLen -= nCurrentSize;
                pData += nCurrentSize;
                nResult += nCurrentSize;
            }
        }

    } else {
#ifdef QT_DEBUG
        qDebug("Invalid pos: %llX Size: %llX", nPos, getSize());
#endif
    }

    if (bPositionEstablished) {
        if (!guardedThis || !guardedDevice ||
            (nResult > (std::numeric_limits<qint64>::max)() - nPos)) {
            nResult = 0;
        } else {
            const bool bSeeked = guardedDevice->seek(nPos + nResult);
            if (!guardedThis || !guardedDevice || !bSeeked) nResult = 0;
        }
    }

    return guardedThis ? nResult : 0;
}

qint64 XBinary::safeWriteData(QIODevice *pDevice, qint64 nPos, const char *pData, qint64 nLen, PDSTRUCT *pPdStruct)
{
    qint64 nResult = 0;
    QPointer<XBinary> guardedThis(this);

    if (!pDevice || (nPos < 0) || (nLen < 0) ||
        ((nLen > 0) && !pData) ||
        !isPdStructNotCanceled(pPdStruct)) {
        return nResult;
    }

    QPointer<QIODevice> guardedDevice(pDevice);
    const bool bOpen = guardedDevice->isOpen();
    if (!guardedThis || !guardedDevice || !bOpen) return nResult;
    const bool bWritable = guardedDevice->isWritable();
    if (!guardedThis || !guardedDevice || !bWritable) return nResult;
    const bool bSequential = guardedDevice->isSequential();
    if (!guardedThis || !guardedDevice || bSequential) return nResult;
    const QIODevice::OpenMode openMode = guardedDevice->openMode();
    if (!guardedThis || !guardedDevice ||
        (openMode & (QIODevice::Append | QIODevice::Text))) return nResult;

    const qint32 nRequestedBufferSize = getBufferSize(pPdStruct);

    if (nRequestedBufferSize <= 0) {
        return nResult;
    }

    const qint64 nBufferSize = qBound((qint32)0x1000, nRequestedBufferSize, (qint32)0x100000);

    QMutex *pReadWriteMutex = guardedThis->m_pReadWriteMutex;
    QMutexLocker locker(pReadWriteMutex);

    const qint64 nDeviceSize = guardedDevice ? guardedDevice->size() : -1;

    if (!guardedThis || !guardedDevice || (nDeviceSize < 0) || (nPos > nDeviceSize) ||
        (nLen > nDeviceSize - nPos)) {
        return nResult;
    }

    while ((nLen > 0) && isPdStructNotCanceled(pPdStruct)) {
        if (nResult > (std::numeric_limits<qint64>::max)() - nPos)
            break;
        if (!guardedThis || !guardedDevice) break;
        const bool bSeeked = guardedDevice->seek(nPos + nResult);
        if (!guardedThis || !guardedDevice || !bSeeked) break;
        const qint64 nRequestedSize = qMin(nLen, nBufferSize);
        const qint64 nCurrentSize = guardedDevice->write(pData, nRequestedSize);

        if (!guardedThis || !guardedDevice || (nCurrentSize <= 0) ||
            (nCurrentSize > nRequestedSize)) {
            setPdStructErrorString(pPdStruct, tr("Write error"));
            break;
        }

        nLen -= nCurrentSize;
        pData += nCurrentSize;
        nResult += nCurrentSize;
    }

    if (!guardedThis || !guardedDevice ||
        (nResult > (std::numeric_limits<qint64>::max)() - nPos)) {
        nResult = 0;
    } else {
        const bool bSeeked = guardedDevice->seek(nPos + nResult);
        if (!guardedThis || !guardedDevice || !bSeeked) nResult = 0;
    }

    return nResult;
}

qint64 XBinary::_readDataSimple(QIODevice *pDevice, qint64 nPos, char *pData, qint64 nMaxLen)
{
    // qDebug("%X %X pos: %X maxlen: %X", this, pDevice, nPos, nMaxLen);
    qint64 nResult = 0;
    QPointer<XBinary> guardedThis(this);

    if (!pDevice || (nPos < 0) ||
        (nMaxLen < 0) || ((nMaxLen > 0) && !pData) ||
        (nMaxLen == 0)) {
        return nResult;
    }

    QMutex *pReadWriteMutex = guardedThis ? guardedThis->m_pReadWriteMutex : nullptr;
    QMutexLocker locker(pReadWriteMutex);
    QPointer<QIODevice> guardedDevice(pDevice);
    const bool bReadable = guardedDevice->isReadable();
    if (!guardedThis || !guardedDevice || !bReadable) return nResult;

    const qint64 nDeviceSize = guardedDevice ? guardedDevice->size() : -1;
    if (guardedThis && guardedDevice && (nDeviceSize > nPos) && (nPos >= 0)) {
        const bool bSeeked = guardedDevice->seek(nPos);
        if (guardedThis && guardedDevice && bSeeked) {
            nResult = guardedDevice->read(pData, nMaxLen);
            if (!guardedThis || !guardedDevice || (nResult < 0) || (nResult > nMaxLen) ||
                (nResult > (std::numeric_limits<qint64>::max)() - nPos)) {
                nResult = 0;
            } else {
                const bool bFinalSeeked =
                    guardedDevice->seek(nPos + nResult);
                if (!guardedThis || !guardedDevice || !bFinalSeeked) nResult = 0;
            }
        } else {
#ifdef QT_DEBUG
            qDebug("Cannot seek");
#endif
        }
    } else {
#ifdef QT_DEBUG
            if (guardedThis)
                qDebug("Invalid pos: %llX Size: %llX", nPos,
                       guardedThis->getSize());
#endif
    }

    return guardedThis ? nResult : 0;
}

static bool resizeByteArrayForRead(QByteArray *pData, qint64 nSize)
{
    if (!pData || (nSize <= 0) || (nSize > (std::numeric_limits<qint32>::max)())) {
        return false;
    }

    try {
        pData->resize((qint32)nSize);
    } catch (const std::bad_alloc &) {
        pData->clear();
        return false;
    }

    return pData->size() == nSize;
}

QByteArray XBinary::_readDataSimple(QIODevice *pDevice, qint64 nPos, qint64 nSize)
{
    QByteArray baResult;

    // This overload explicitly accepts a device and must derive its bounds from
    // that device, not from the XBinary instance's possibly unrelated device.
    // QByteArray is qint32-sized in Qt 5, so reject a wider request before
    // narrowing the allocation while still passing a qint64 length to read().
    XBinary::OFFSETSIZE osRegion = XBinary::convertOffsetAndSize(pDevice, nPos, nSize);

    if ((osRegion.nOffset != -1) && resizeByteArrayForRead(&baResult, osRegion.nSize)) {
        const qint64 nBytes = _readDataSimple(pDevice, osRegion.nOffset, baResult.data(), osRegion.nSize);

        if ((nBytes < 0) || (nBytes > osRegion.nSize)) {
            baResult.clear();
        } else if (osRegion.nSize != nBytes) {
            baResult.resize((qint32)nBytes);
        }
    }

    return baResult;
}

qint64 XBinary::_writeDataSimple(QIODevice *pDevice, qint64 nPos, const char *pData, qint64 nLen)
{
    qint64 nResult = 0;
    QPointer<XBinary> guardedThis(this);

    if (!pDevice || (nPos < 0) || (nLen < 0) ||
        ((nLen > 0) && !pData)) {
        return nResult;
    }

    QMutex *pReadWriteMutex = guardedThis ? guardedThis->m_pReadWriteMutex : nullptr;
    QMutexLocker locker(pReadWriteMutex);
    QPointer<QIODevice> guardedDevice(pDevice);
    const bool bWritable = guardedDevice->isWritable();
    if (!guardedThis || !guardedDevice || !bWritable) return nResult;

    const qint64 nDeviceSize = guardedDevice ? guardedDevice->size() : -1;

    if (guardedThis && guardedDevice && (nDeviceSize >= 0) && (nPos <= nDeviceSize) &&
        (nLen <= (nDeviceSize - nPos))) {
        while (nResult < nLen) {
            if (nResult > (std::numeric_limits<qint64>::max)() - nPos)
                break;
            if (!guardedThis || !guardedDevice) break;
            const bool bSeeked = guardedDevice->seek(nPos + nResult);
            if (!guardedThis || !guardedDevice || !bSeeked) break;
            const qint64 nWritten = guardedDevice->write(
                pData + nResult, nLen - nResult);

            if (!guardedThis || !guardedDevice || (nWritten <= 0) ||
                (nWritten > (nLen - nResult))) {
                break;
            }

            nResult += nWritten;
        }
        if (!guardedThis || !guardedDevice ||
            (nResult > (std::numeric_limits<qint64>::max)() - nPos)) {
            nResult = 0;
        } else {
            const bool bFinalSeeked =
                guardedDevice->seek(nPos + nResult);
            if (!guardedThis || !guardedDevice || !bFinalSeeked) nResult = 0;
        }
    } else {
#ifdef QT_DEBUG
        if (guardedThis)
            qDebug("Invalid pos: %llX Size: %llX", nPos,
                   guardedThis->getSize());
#endif
    }

    return guardedThis ? nResult : 0;
}

qint64 XBinary::getSize()
{
    return m_nSize;
}

qint64 XBinary::getSize(QIODevice *pDevice)
{
    XBinary binary(pDevice);

    return binary.getSize();
}

qint64 XBinary::getSize(const QString &sFileName)
{
    qint64 nResult = 0;

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        nResult = getSize(&file);

        file.close();
    }

    return nResult;
}

void XBinary::setMode(XBinary::MODE mode)
{
    m_mode = mode;
}

XBinary::MODE XBinary::getMode()
{
    return m_mode;
}

void XBinary::setType(qint32 nType)
{
    m_nType = nType;
}

qint32 XBinary::getType()
{
    return m_nType;
}

QString XBinary::typeIdToString(qint32 nType)
{
    QString sResult = tr("Unknown");

    switch (nType) {
        case TYPE_UNKNOWN: sResult = tr("Unknown");
    }

    return sResult;
}

QString XBinary::getTypeAsString()
{
    return typeIdToString(getType());
}

void XBinary::setFileType(XBinary::FT fileType)
{
    m_fileType = fileType;
}

XBinary::FT XBinary::getFileType()
{
    return m_fileType;
}

QString XBinary::modeIdToString(XBinary::MODE mode)
{
    QString sResult = tr("Unknown");

    switch (mode) {
        case MODE_UNKNOWN: sResult = tr("Unknown"); break;
        case MODE_DATA: sResult = QString("Data"); break;
        case MODE_BIT: sResult = QString("BIT"); break;
        case MODE_8: sResult = tr("8-bit"); break;
        case MODE_16: sResult = tr("16-bit"); break;
        case MODE_16SEG: sResult = QString("16SEG"); break;
        case MODE_32: sResult = tr("32-bit"); break;
        case MODE_64: sResult = tr("64-bit"); break;
        case MODE_128: sResult = tr("128-bit"); break;
        case MODE_256: sResult = tr("256-bit"); break;
        case MODE_FREG: sResult = QString("freg"); break;
    }

    return sResult;
}

QString XBinary::endianToString(ENDIAN endian)
{
    QString sResult;

    if (endian == ENDIAN_UNKNOWN) {
        sResult = tr("Unknown");
    } else if (endian == ENDIAN_BIG) {
        sResult = "BE";
    } else if (endian == ENDIAN_LITTLE) {
        sResult = "LE";
    }

    return sResult;
}

QString XBinary::codepageIdToString(quint32 nCodepage)
{
    return XIDSTRING_idToString(nCodepage, _TABLE_XBinary_CODEPAGE, sizeof(_TABLE_XBinary_CODEPAGE) / sizeof(XBinary::XIDSTRING));
}

QList<XBinary::CODEPAGE> XBinary::getCodepagesList()
{
    QList<XBinary::CODEPAGE> listResult;

    qint32 nNumberOfRecords = sizeof(_TABLE_XBinary_CODEPAGE) / sizeof(XBinary::XIDSTRING);

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        listResult.append((XBinary::CODEPAGE)_TABLE_XBinary_CODEPAGE[i].nID);
    }

    return listResult;
}

void XBinary::setArch(const QString &sArch)
{
    m_sArch = sArch;
}

QString XBinary::getArch()
{
    return m_sArch;
}

QString XBinary::getFileFormatString(const FILEFORMATINFO *pFileFormatInfo)
{
    if (!pFileFormatInfo) {
        return QString();
    }

    QString sResult = fileTypeIdToString(pFileFormatInfo->fileType);

    if (pFileFormatInfo->sVersion != "") {
        sResult += QString("(%1)").arg(pFileFormatInfo->sVersion);
    }

    QString sInfo = getFileFormatInfoString(pFileFormatInfo);

    if (sInfo != "") {
        sResult += QString("[%1]").arg(sInfo);
    }

    return sResult;
}

QString XBinary::getFileFormatInfoString(const FILEFORMATINFO *pFileFormatInfo)
{
    QString sResult;

    if (!pFileFormatInfo) {
        return sResult;
    }

    if (pFileFormatInfo->bIsEncrypted) {
        sResult = appendText(sResult, QObject::tr("Encrypted"), ", ");
    }

    if (pFileFormatInfo->sCompresionMethod != "") {
        sResult = appendText(sResult, pFileFormatInfo->sCompresionMethod, ", ");
    }

    if (pFileFormatInfo->sInfo != "") {
        sResult = appendText(sResult, pFileFormatInfo->sInfo, ", ");
    }

    // TODO

    return sResult;
}

QString XBinary::getOperationSystemInfoString(const FILEFORMATINFO *pFileFormatInfo)
{
    return pFileFormatInfo ? QString("%1, %2").arg(pFileFormatInfo->sArch).arg(modeIdToString(pFileFormatInfo->mode)) : QString();
}

XBinary::OSNAME XBinary::getOsName(const FILEFORMATINFO *pFileFormatInfo)
{
    return pFileFormatInfo ? pFileFormatInfo->osName : OSNAME_UNKNOWN;
}

void XBinary::setFileFormatExt(const QString &sFileFormatExt)
{
    m_sFileFormatExt = sFileFormatExt;
}

QString XBinary::getFileFormatExt()
{
#ifdef QT_DEBUG
    // qDebug("TODO: XBinary::getFileFormatExt()");
#endif
    return m_sFileFormatExt;
}

void XBinary::setFileFormatExtsString(const QString &sFileFormatExts)
{
    m_sFileFormatExts = sFileFormatExts;
}

QString XBinary::getFileFormatExtsString()
{
    if (m_sFileFormatExts == "") {
#ifdef QT_DEBUG
        qDebug("TODO: XBinary::getFileFormatExtsString()");
#endif
    }

    return m_sFileFormatExts;
}

void XBinary::setFileFormatSize(qint64 nFileFormatSize)
{
    m_nFileFormatSize = nFileFormatSize;
}

bool XBinary::setFileDateTime(const QString &sFileName, const QDateTime &dateTime)
{
    // Try to set both modification and access times via Qt API (member function)
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    QFile file;
    file.setFileName(sFileName);
    bool bMod = file.setFileTime(dateTime, QFileDevice::FileModificationTime);
    bool bAccess = file.setFileTime(dateTime, QFileDevice::FileAccessTime);
    return (bMod || bAccess);
#else
    Q_UNUSED(sFileName)
    Q_UNUSED(dateTime)
    return false;
#endif
}

QDateTime XBinary::winFileTimeToQDateTime(quint64 nWinFileTime)
{
    // Windows FILETIME: 100-nanosecond intervals since January 1, 1601 UTC.
    // Offset to Unix epoch (January 1, 1970): 116444736000000000 * 100ns.
    if (nWinFileTime == 0) {
        return QDateTime();
    }

    const quint64 nEpochDelta = Q_UINT64_C(116444736000000000);

    if (nWinFileTime < nEpochDelta) {
        return QDateTime();
    }

    quint64 nMsecsSinceEpoch = (nWinFileTime - nEpochDelta) / Q_UINT64_C(10000);

    return QDateTime::fromMSecsSinceEpoch((qint64)nMsecsSinceEpoch, Qt::UTC);
}

bool XBinary::setFileProperties(const QMap<FPART_PROP, QVariant> &mapProperties, const QString &sFileName)
{
    bool bResult = false;

    // --- Timestamps ---
    // QFile::setFileTime requires the file to be open; use ReadWrite so the
    // OS handle has FILE_WRITE_ATTRIBUTES rights on Windows.
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    {
        QFile file(sFileName);
        if (file.open(QIODevice::ReadWrite)) {
            if (mapProperties.contains(FPART_PROP_MTIME)) {
                QDateTime dt = mapProperties.value(FPART_PROP_MTIME).toDateTime();
                if (dt.isValid()) {
                    if (file.setFileTime(dt, QFileDevice::FileModificationTime)) {
                        bResult = true;
                    }
                }
            }
            if (mapProperties.contains(FPART_PROP_ATIME)) {
                QDateTime dt = mapProperties.value(FPART_PROP_ATIME).toDateTime();
                if (dt.isValid()) {
                    file.setFileTime(dt, QFileDevice::FileAccessTime);
                }
            }
            if (mapProperties.contains(FPART_PROP_CTIME)) {
                QDateTime dt = mapProperties.value(FPART_PROP_CTIME).toDateTime();
                if (dt.isValid()) {
                    // FileBirthTime = creation time on Windows; inode change time on Unix
                    file.setFileTime(dt, QFileDevice::FileBirthTime);
                }
            }
            file.close();
        }
    }
#endif

    // --- File attributes ---
#ifdef Q_OS_WIN
    bool bHasAttrib = mapProperties.contains(FPART_PROP_ISREADONLY) || mapProperties.contains(FPART_PROP_ISHIDDEN) || mapProperties.contains(FPART_PROP_ISSYSTEM) ||
                      mapProperties.contains(FPART_PROP_ISARCHIVE);

    if (bHasAttrib) {
        DWORD nAttribs = GetFileAttributesW((LPCWSTR)sFileName.utf16());

        if (nAttribs != INVALID_FILE_ATTRIBUTES) {
            if (mapProperties.contains(FPART_PROP_ISREADONLY)) {
                if (mapProperties.value(FPART_PROP_ISREADONLY).toBool()) {
                    nAttribs |= FILE_ATTRIBUTE_READONLY;
                } else {
                    nAttribs &= ~(DWORD)FILE_ATTRIBUTE_READONLY;
                }
            }

            if (mapProperties.contains(FPART_PROP_ISHIDDEN)) {
                if (mapProperties.value(FPART_PROP_ISHIDDEN).toBool()) {
                    nAttribs |= FILE_ATTRIBUTE_HIDDEN;
                } else {
                    nAttribs &= ~(DWORD)FILE_ATTRIBUTE_HIDDEN;
                }
            }

            if (mapProperties.contains(FPART_PROP_ISSYSTEM)) {
                if (mapProperties.value(FPART_PROP_ISSYSTEM).toBool()) {
                    nAttribs |= FILE_ATTRIBUTE_SYSTEM;
                } else {
                    nAttribs &= ~(DWORD)FILE_ATTRIBUTE_SYSTEM;
                }
            }

            if (mapProperties.contains(FPART_PROP_ISARCHIVE)) {
                if (mapProperties.value(FPART_PROP_ISARCHIVE).toBool()) {
                    nAttribs |= FILE_ATTRIBUTE_ARCHIVE;
                } else {
                    nAttribs &= ~(DWORD)FILE_ATTRIBUTE_ARCHIVE;
                }
            }

            if (SetFileAttributesW((LPCWSTR)sFileName.utf16(), nAttribs)) {
                bResult = true;
            }
        }
    }
#else
    // Unix: map ISREADONLY to write-permission bits
    if (mapProperties.contains(FPART_PROP_ISREADONLY)) {
        QFile::Permissions perms = QFile::permissions(sFileName);

        if (mapProperties.value(FPART_PROP_ISREADONLY).toBool()) {
            perms &= ~(QFile::WriteOwner | QFile::WriteUser | QFile::WriteGroup | QFile::WriteOther);
        } else {
            perms |= QFile::WriteOwner | QFile::WriteUser;
        }

        if (QFile::setPermissions(sFileName, perms)) {
            bResult = true;
        }
    }
#endif

    return bResult;
}

qint64 XBinary::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    return m_nFileFormatSize;
}

bool XBinary::isSigned()
{
    return false;
}

XBinary::OFFSETSIZE XBinary::getSignOffsetSize()
{
    OFFSETSIZE osResult = {};

    return osResult;
}

void XBinary::setOsType(OSNAME osName)
{
    m_osName = osName;
}

XBinary::OSNAME XBinary::getOsName()
{
    return m_osName;
}

void XBinary::setOsVersion(const QString &sOsVersion)
{
    m_sOsVersion = sOsVersion;
}

QString XBinary::getOsVersion()
{
    return m_sOsVersion;
}

XBinary::FILEFORMATINFO XBinary::getFileFormatInfo(PDSTRUCT *pPdStruct)
{
    // TODO userData
    FILEFORMATINFO result = {};

    result.bIsValid = isValid(pPdStruct);

    if (result.bIsValid) {
        result.nSize = getSize();
        result.fileType = getFileType();
        result.sExt = getFileFormatExt();
        result.sVersion = getVersion();
        result.sInfo = getInfo(pPdStruct);
        result.osName = getOsName();
        result.sOsVersion = getOsVersion();
        result.sArch = getArch();
        result.mode = getMode();
        result.sType = typeIdToString(getType());
        result.endian = getEndian();
        result.sMIME = getMIMEString();
        result.sCompresionMethod = getCompressMethodString();
        result.bIsEncrypted = isEncrypted();

        if (result.nSize == 0) {
            result.bIsValid = false;
        }
    }

    return result;
}

void XBinary::setEndian(ENDIAN endian)
{
    m_endian = endian;
}

void XBinary::setIsExecutable(bool bIsExecutable)
{
    m_bIsExecutable = bIsExecutable;
}

void XBinary::setIsArchive(bool bIsArchive)
{
    m_bIsArchive = bIsArchive;
}

XBinary::ENDIAN XBinary::getEndian()
{
    return m_endian;
}

bool XBinary::isPacked(double dEntropy)
{
    return (dEntropy >= D_ENTROPY_THRESHOLD);  // TODO Check
}

bool XBinary::isExecutable()
{
    return m_bIsExecutable;
}

bool XBinary::isArchive()
{
    return m_bIsArchive;
}

quint8 XBinary::random8()
{
    return (quint8)random16();
}

quint16 XBinary::random16()
{
    quint16 nResult = 0;

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    nResult = (quint16)(QRandomGenerator::global()->generate());
#elif (QT_VERSION_MAJOR >= 6)
    nResult = (quint16)(QRandomGenerator::global()->generate());
#else
    static quint32 nSeed = 0;

    if (!nSeed) {
        quint32 nRValue = QDateTime::currentMSecsSinceEpoch() & 0xFFFFFFFF;

        nSeed ^= nRValue;
        qsrand(nSeed);
    }
    nResult = (quint16)qrand();
#endif

    return nResult;
}

quint32 XBinary::random32()
{
    quint16 nValue1 = random16();
    quint16 nValue2 = random16();

    return (nValue1 << 16) + nValue2;
}

quint64 XBinary::random64()
{
    quint64 nVal1 = random32();
    quint64 nVal2 = random32();

    nVal1 = nVal1 << 32;

    return nVal1 + nVal2;
}

quint64 XBinary::random(quint64 nLimit)
{
    quint64 nResult = 0;

    if (nLimit) {
        nResult = (random64()) % nLimit;
    }

    return nResult;
}

QString XBinary::randomString(qint32 nSize)
{
    QString sResult;
    sResult.reserve(nSize);

    for (int i = 0; i < nSize; i++) {
        // sResult += QChar('A' + (char)random(26 * 2));
        sResult += QChar('A' + (char)random(26));
    }

    return sResult;
}

QString XBinary::fileTypeIdToString(XBinary::FT fileType)
{
    return XCONVERT_idToTransString(fileType, _TABLE_XBinary_FT, sizeof(_TABLE_XBinary_FT) / sizeof(XBinary::XCONVERT));
}

QString XBinary::xfTypeIdToString(XBinary::XFTYPE xfType)
{
    return XCONVERT_idToTransString(xfType, _TABLE_XBinary_XFTYPE, sizeof(_TABLE_XBinary_XFTYPE) / sizeof(XBinary::XCONVERT));
}

QString XBinary::xfTypeIdToFtString(XBinary::XFTYPE xfType)
{
    return XCONVERT_idToFtString(xfType, _TABLE_XBinary_XFTYPE, sizeof(_TABLE_XBinary_XFTYPE) / sizeof(XBinary::XCONVERT));
}

// QString XBinary::fileTypeIdToExts(FT fileType)
// {
//     QString sResult = tr("Unknown");

//     switch (fileType) {
//         case FT_ZIP: sResult = QString("ZIP(zip, jar, apk, ipa, docx)"); break;
//         case FT_RAR: sResult = QString("RAR"); break;
//         case FT_PDF: sResult = QString("PDF"); break;
//         case FT_7Z: sResult = QString("7-Zip"); break;
//         case FT_PNG: sResult = QString("PNG"); break;
//         case FT_JPEG: sResult = QString("JPEG"); break;
//         case FT_CAB: sResult = QString("CAB"); break;
//         case FT_ICO: sResult = QString("ICO"); break;
//         case FT_CUR: sResult = QString("CUR"); break;
//         case FT_TIFF: sResult = QString("TIFF"); break;
//         case FT_DEX: sResult = QString("DEX"); break;
//         case FT_MACHOFAT: sResult = QString("MACHOFAT"); break;
//         case FT_MACHO: sResult = QString("MACHO"); break;
//         case FT_BMP: sResult = QString("BMP"); break;
//         case FT_GIF: sResult = QString("GIF"); break;
//         case FT_MP3: sResult = QString("MP3"); break;
//         case FT_MP4: sResult = QString("MP4"); break;
//         case FT_XM: sResult = QString("XM"); break;
//         case FT_RIFF: sResult = QString("RIFF(avi, webp)"); break;
//         case FT_ZLIB: sResult = QString("zlib"); break;
//         case FT_GZIP: sResult = QString("GZIP(gz, tgz, tar.gz)"); break;
//         case FT_LE: sResult = QString("LE(le, lx)"); break;
//         case FT_NE: sResult = QString("NE"); break;
//         case FT_AMIGAHUNK: sResult = QString("AmigaHunk"); break;
//         case FT_JAVACLASS: sResult = QString("class"); break;
//         case FT_CFBF: sResult = QString("CFBF"); break;
//         case FT_TTF: sResult = QString("TTF"); break;
//         case FT_SIGNATURE: sResult = tr("Signatures"); break;
//         default: sResult = tr("Unknown");
//     }

//     return sResult;
// }

XBinary::FT XBinary::ftStringToFileTypeId(QString sFileType)
{
    return (XBinary::FT)XCONVERT_ftStringToId(sFileType, _TABLE_XBinary_FT, sizeof(_TABLE_XBinary_FT) / sizeof(XBinary::XCONVERT));
}

QString XBinary::fileTypeIdToFtString(FT fileType)
{
    return XCONVERT_idToFtString(fileType, _TABLE_XBinary_FT, sizeof(_TABLE_XBinary_FT) / sizeof(XBinary::XCONVERT));
}

QString XBinary::fileTypesToString(const QSet<XBinary::FT> &stResult)
{
    QString sResult;

    qint32 nNumberOfRecords = sizeof(_TABLE_XBinary_FT) / sizeof(XBinary::XCONVERT);

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        XBinary::FT fileType = (XBinary::FT)_TABLE_XBinary_FT[i].nID;

        if (stResult.contains(fileType)) {
            if (!sResult.isEmpty()) {
                sResult += QString("|");
            }

            sResult += fileTypeIdToFtString(fileType);
        }
    }

    return sResult;
}

QSet<XBinary::FT> XBinary::stringToFileTypes(const QString &sString)
{
    QSet<XBinary::FT> stResult;

    QString sCurrent;
    qint32 nSize = sString.size();

    for (qint32 i = 0; i < nSize; i++) {
        const QChar c = sString.at(i);

        if (c == '|') {
            QString sToken = sCurrent.trimmed();

            if (!sToken.isEmpty()) {
                XBinary::FT fileType = ftStringToFileTypeId(sToken);

                if (fileType == XBinary::FT_UNKNOWN) {
                    fileType = ftStringToFileTypeId(sToken.toUpper());
                }

                if (fileType == XBinary::FT_UNKNOWN) {
                    fileType = ftStringToFileTypeId(sToken.toLower());
                }

                if (fileType != XBinary::FT_UNKNOWN) {
                    stResult.insert(fileType);
                }
            }

            sCurrent.clear();
        } else {
            sCurrent += c;
        }
    }

    QString sToken = sCurrent.trimmed();

    if (!sToken.isEmpty()) {
        XBinary::FT fileType = ftStringToFileTypeId(sToken);

        if (fileType == XBinary::FT_UNKNOWN) {
            fileType = ftStringToFileTypeId(sToken.toUpper());
        }

        if (fileType == XBinary::FT_UNKNOWN) {
            fileType = ftStringToFileTypeId(sToken.toLower());
        }

        if (fileType != XBinary::FT_UNKNOWN) {
            stResult.insert(fileType);
        }
    }

    return stResult;
}

QString XBinary::convertFileName(const QString &sFileName)
{
    QString sResult = sFileName;

    QFileInfo fiLink(sFileName);

    if (fiLink.isSymLink()) {
        sResult = fiLink.symLinkTarget();
    }

    return sResult;
}

// QString XBinary::convertPathName(const QString &sPathName)
// {
//     QString sResult = sPathName;

//     // TODO more
//     if (sPathName.contains("$app")) {
//         sResult.replace("$app", QCoreApplication::applicationDirPath());
//         sResult.replace("/", QDir::separator());
//     }

//     if (sPathName.contains("$data")) {
// #ifdef Q_OS_MAC
//         sResult.replace("$data", QCoreApplication::applicationDirPath() + "/../Resources");
// #else
//         sResult.replace("$data", QCoreApplication::applicationDirPath());
// #endif
//         sResult.replace("/", QDir::separator());
//     }

//     return sResult;
// }

XBinary::OS_STRING XBinary::getOsAnsiString(qint64 nOffset, qint64 nSize)
{
    OS_STRING result = {};

    result.nOffset = nOffset;
    result.nSize = nSize;
    result.sString = read_ansiString(nOffset, nSize);

    return result;
}

void XBinary::findFiles(const QString &sDirectoryName, XBinary::FFOPTIONS *pFFOption, qint32 nLevel)
{
    static const qint32 MAX_DIRECTORY_RECURSION = 256;
    if (!pFFOption || !pFFOption->pnNumberOfFiles || !pFFOption->pListFileNames || !pFFOption->pbIsStop ||
        (nLevel < 0) || (nLevel > MAX_DIRECTORY_RECURSION)) {
        return;
    }

    *(pFFOption->pnNumberOfFiles) = pFFOption->pListFileNames->count();

    if (!(*pFFOption->pbIsStop)) {
        QFileInfo fi(sDirectoryName);

        if (!isDirectCanonicalPath(fi)) return;

        if (!fi.isSymLink() && fi.isFile()) {
            pFFOption->pListFileNames->append(fi.absoluteFilePath());
        // Do not follow symlinks. Besides creating directory cycles, following
        // file or directory links can silently include content outside the root
        // selected by the caller.
        } else if (fi.isDir() && !fi.isSymLink() && ((pFFOption->bSubdirectories) || (nLevel == 0))) {
            QDir dir(sDirectoryName);

            QFileInfoList eil = dir.entryInfoList();

            qint32 nNumberOfFiles = eil.count();

            for (qint32 i = 0; (i < nNumberOfFiles) && (!(*(pFFOption->pbIsStop))); i++) {
                QString sFN = eil.at(i).fileName();

                if ((sFN != ".") && (sFN != "..") && (nLevel < MAX_DIRECTORY_RECURSION)) {
                    findFiles(eil.at(i).absoluteFilePath(), pFFOption, nLevel + 1);
                }
            }
        }
    }

    *(pFFOption->pnNumberOfFiles) = pFFOption->pListFileNames->count();
}

void XBinary::findFiles(const QString &sDirectoryName, QList<QString> *pListFileNames, PDSTRUCT *pPdStruct)
{
    findFiles(sDirectoryName, pListFileNames, true, 0, pPdStruct);
}

void XBinary::findFiles(const QString &sDirectoryName, QList<QString> *pListFileNames, bool bSubDirectories, qint32 nLevel, PDSTRUCT *pPdStruct)
{
    static const qint32 MAX_DIRECTORY_RECURSION = 256;
    if (!pListFileNames || (nLevel < 0) || (nLevel > MAX_DIRECTORY_RECURSION)) {
        return;
    }

    PDSTRUCT pdStructEmpty = createPdStruct();
    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    qint32 _nFreeIndex = 0;

    if (nLevel == 0) {
        _nFreeIndex = XBinary::reservePdStructRecord(pPdStruct, 0);
    }

    if (isPdStructNotCanceled(pPdStruct)) {
        QFileInfo fi(sDirectoryName);

        if (!isDirectCanonicalPath(fi)) {
            if (nLevel == 0) XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);
            return;
        }

        if (!fi.isSymLink() && fi.isFile()) {
            pListFileNames->append(fi.absoluteFilePath());
        } else if (fi.isDir() && !fi.isSymLink() && ((bSubDirectories) || (nLevel == 0))) {
            QDir dir(sDirectoryName);

            QFileInfoList eil = dir.entryInfoList();

            qint32 nNumberOfFiles = eil.count();

            for (qint32 i = 0; (i < nNumberOfFiles) && isPdStructNotCanceled(pPdStruct); i++) {
                QString sFN = eil.at(i).fileName();

                if ((sFN != ".") && (sFN != "..")) {
                    if (nLevel < MAX_DIRECTORY_RECURSION) {
                        findFiles(eil.at(i).absoluteFilePath(), pListFileNames, bSubDirectories, nLevel + 1, pPdStruct);
                    }
                }
            }
        }
    }

    if (nLevel == 0) {
        XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);
    }
}

qint32 XBinary::getNumberOfFiles(const QString &sDirectoryName, bool bSubDirectories, qint32 nLevel, PDSTRUCT *pPdStruct)
{
    static const qint32 MAX_DIRECTORY_RECURSION = 256;
    if ((nLevel < 0) || (nLevel > MAX_DIRECTORY_RECURSION)) {
        return 0;
    }

    PDSTRUCT pdStructEmpty = createPdStruct();
    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    qint32 nResult = 0;

    qint32 _nFreeIndex = 0;

    if (nLevel == 0) {
        _nFreeIndex = XBinary::reservePdStructRecord(pPdStruct, 0);
    }

    if (isPdStructNotCanceled(pPdStruct)) {
        QFileInfo fi(sDirectoryName);

        if (!isDirectCanonicalPath(fi)) {
            if (nLevel == 0) XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);
            return 0;
        }

        if (!fi.isSymLink() && fi.isFile()) {
            nResult = 1;
        } else if (fi.isDir() && !fi.isSymLink() && ((bSubDirectories) || (nLevel == 0))) {
            QDir dir(sDirectoryName);

            QFileInfoList eil = dir.entryInfoList();

            qint32 nNumberOfFiles = eil.count();

            for (qint32 i = 0; (i < nNumberOfFiles) && isPdStructNotCanceled(pPdStruct); i++) {
                QString sFN = eil.at(i).fileName();

                if ((sFN != ".") && (sFN != "..") && (nLevel < MAX_DIRECTORY_RECURSION)) {
                    const qint32 nChildCount = getNumberOfFiles(eil.at(i).absoluteFilePath(), bSubDirectories, nLevel + 1, pPdStruct);
                    if (nChildCount > ((std::numeric_limits<qint32>::max)() - nResult)) {
                        nResult = (std::numeric_limits<qint32>::max)();
                        break;
                    }
                    nResult += nChildCount;
                }
            }
        }
    }

    if (nLevel == 0) {
        XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);
    }

    return nResult;
}

QString XBinary::regExp(const QString &sRegExp, const QString &sString, qint32 nIndex)
{
    QString sResult;
#if (QT_VERSION_MAJOR < 5)
    QRegExp rxString(sRegExp);
    rxString.indexIn(sString);

    QStringList list = rxString.capturedTexts();

    if (list.count() > nIndex) {
        sResult = list.at(nIndex);
    }
#else
    QRegularExpression rxString(sRegExp);
    QRegularExpressionMatch matchString = rxString.match(sString);

    if (matchString.hasMatch()) {
        sResult = matchString.captured(nIndex);
    }
#endif

    return sResult;
}

bool XBinary::isRegExpPresent(const QString &sRegExp, const QString &sString)
{
    return (regExp(sRegExp, sString, 0) != "");
}

qint32 XBinary::getRegExpCount(const QString &sRegExp, const QString &sString)
{
    quint32 nResult = 0;
#if (QT_VERSION_MAJOR < 5)
    QRegExp rxString(sRegExp);
    rxString.indexIn(sString);
    nResult = rxString.capturedTexts().count();
#else
    QRegularExpression rxString(sRegExp);
    QRegularExpressionMatchIterator i = rxString.globalMatch(sString);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        if (match.hasMatch()) {
            nResult++;
        }
    }
#endif

    return nResult;
}

QString XBinary::getRegExpSection(const QString &sRegExp, const QString &sString, qint32 nStart, qint32 nEnd)
{
#if (QT_VERSION_MAJOR < 5)
    return sString.section(QRegExp(sRegExp), nStart, nEnd);
#else
    return sString.section(QRegularExpression(sRegExp), nStart, nEnd);
#endif
}

bool XBinary::isRegExpValid(const QString &sRegExp)
{
#if (QT_VERSION_MAJOR < 5)
    QRegExp rxString(sRegExp);
    return rxString.isValid();
#else
    QRegularExpression rxString(sRegExp);
    return rxString.isValid();
#endif
}

qint64 XBinary::read_array_process(qint64 nOffset, char *pBuffer, qint64 nMaxSize, PDSTRUCT *pPdStruct)
{
    qint64 nResult = 0;

    if (!m_pDevice) {
        return 0;
    }

    nResult = safeReadData(m_pDevice, nOffset, pBuffer, nMaxSize, pPdStruct);  // Check for read large files

    return nResult;
}

qint64 XBinary::read_array(qint64 nOffset, char *pBuffer, qint64 nMaxSize)
{
    qint64 nResult = 0;

    if (!m_pDevice) {
        return 0;
    }

    nResult = _readDataSimple(m_pDevice, nOffset, pBuffer, nMaxSize);  // Check for read large files

    return nResult;
}

QByteArray XBinary::read_array(qint64 nOffset, qint64 nSize)
{
    QByteArray baResult;

    XBinary::OFFSETSIZE osRegion = convertOffsetAndSize(nOffset, nSize);

    if ((osRegion.nOffset != -1) && resizeByteArrayForRead(&baResult, osRegion.nSize)) {
        const qint64 nBytes = read_array(osRegion.nOffset, baResult.data(), osRegion.nSize);

        if ((nBytes < 0) || (nBytes > osRegion.nSize)) {
            baResult.clear();
        } else if (osRegion.nSize != nBytes) {
            baResult.resize((qint32)nBytes);
        }
    }

    return baResult;
}

QByteArray XBinary::read_array_process(qint64 nOffset, qint64 nSize, PDSTRUCT *pPdStruct)
{
    QByteArray baResult;

    XBinary::OFFSETSIZE osRegion = convertOffsetAndSize(nOffset, nSize);

    if ((osRegion.nOffset != -1) && resizeByteArrayForRead(&baResult, osRegion.nSize)) {
        const qint64 nBytes = read_array_process(osRegion.nOffset, baResult.data(), osRegion.nSize, pPdStruct);

        if ((nBytes < 0) || (nBytes > osRegion.nSize)) {
            baResult.clear();
        } else if (osRegion.nSize != nBytes) {
            baResult.resize((qint32)nBytes);
        }
    }

    return baResult;
}

qint64 XBinary::write_array_process(qint64 nOffset, const char *pBuffer, qint64 nSize, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    qint64 nResult = 0;

    const qint64 nTotalSize = getSize();

    if (m_pDevice && (nOffset >= 0) && (nSize >= 0) && (nOffset <= nTotalSize) && (nSize <= nTotalSize - nOffset)) {
        nResult = safeWriteData(m_pDevice, nOffset, pBuffer, nSize, pPdStruct);
    }

    return nResult;
}

qint64 XBinary::write_array_process(qint64 nOffset, const QByteArray &baData, PDSTRUCT *pPdStruct)
{
    return write_array_process(nOffset, baData.data(), baData.size(), pPdStruct);
}

qint64 XBinary::write_array(qint64 nOffset, const char *pBuffer, qint64 nSize)
{
    qint64 nResult = 0;

    const qint64 nTotalSize = getSize();

    if (m_pDevice && (nOffset >= 0) && (nSize >= 0) && (nOffset <= nTotalSize) && (nSize <= nTotalSize - nOffset)) {
        nResult = _writeDataSimple(m_pDevice, nOffset, pBuffer, nSize);
    }

    return nResult;
}

qint64 XBinary::write_array(qint64 nOffset, const QByteArray &baData)
{
    return write_array(nOffset, baData.data(), baData.size());
}

QByteArray XBinary::read_array_process(QIODevice *pDevice, qint64 nOffset, qint64 nSize, PDSTRUCT *pPdStruct)
{
    XBinary binary(pDevice);

    return binary.read_array_process(nOffset, nSize, pPdStruct);
}

qint64 XBinary::read_array_process(QIODevice *pDevice, qint64 nOffset, char *pBuffer, qint64 nSize, PDSTRUCT *pPdStruct)
{
    XBinary binary(pDevice);

    return binary.read_array_process(nOffset, pBuffer, nSize, pPdStruct);
}

qint64 XBinary::write_array_process(QIODevice *pDevice, qint64 nOffset, char *pBuffer, qint64 nSize, PDSTRUCT *pPdStruct)
{
    XBinary binary(pDevice);

    return binary.write_array_process(nOffset, pBuffer, nSize, pPdStruct);
}

qint64 XBinary::write_array_process(QIODevice *pDevice, qint64 nOffset, const QByteArray &baData, PDSTRUCT *pPdStruct)
{
    XBinary binary(pDevice);

    return binary.write_array_process(nOffset, baData, pPdStruct);
}

QByteArray XBinary::read_array(QIODevice *pDevice, qint64 nOffset, qint64 nSize)
{
    XBinary binary(pDevice);

    return binary.read_array(nOffset, nSize);
}

qint64 XBinary::write_array(QIODevice *pDevice, qint64 nOffset, const char *pBuffer, qint64 nSize)
{
    XBinary binary(pDevice);

    return binary.write_array(nOffset, pBuffer, nSize);
}

qint64 XBinary::write_array(QIODevice *pDevice, qint64 nOffset, const QByteArray &baData)
{
    XBinary binary(pDevice);

    return binary.write_array(nOffset, baData);
}

quint8 XBinary::read_uint8(qint64 nOffset)
{
    quint8 result = 0;

    read_array(nOffset, (char *)(&result), 1);

    return result;
}

qint8 XBinary::read_int8(qint64 nOffset)
{
    quint8 result = 0;

    read_array(nOffset, (char *)(&result), 1);

    return (qint8)result;
}

quint16 XBinary::read_uint16(qint64 nOffset, bool bIsBigEndian)
{
    quint16 result = 0;

    read_array(nOffset, (char *)(&result), 2);

    if (bIsBigEndian) {
        result = qFromBigEndian(result);
    } else {
        result = qFromLittleEndian(result);
    }

    return result;
}

qint16 XBinary::read_int16(qint64 nOffset, bool bIsBigEndian)
{
    quint16 result = 0;

    read_array(nOffset, (char *)(&result), 2);

    if (bIsBigEndian) {
        result = qFromBigEndian(result);
    } else {
        result = qFromLittleEndian(result);
    }

    return (qint16)result;
}

quint32 XBinary::read_uint32(qint64 nOffset, bool bIsBigEndian)
{
    quint32 result = 0;

    read_array(nOffset, (char *)(&result), 4);

    if (bIsBigEndian) {
        result = qFromBigEndian(result);
    } else {
        result = qFromLittleEndian(result);
    }

    return result;
}

qint32 XBinary::read_int32(qint64 nOffset, bool bIsBigEndian)
{
    quint32 result = 0;

    read_array(nOffset, (char *)(&result), 4);

    if (bIsBigEndian) {
        result = qFromBigEndian(result);
    } else {
        result = qFromLittleEndian(result);
    }

    return (qint32)result;
}

quint64 XBinary::read_uint64(qint64 nOffset, bool bIsBigEndian)
{
    quint64 result = 0;

    read_array(nOffset, (char *)(&result), 8);

    if (bIsBigEndian) {
        result = qFromBigEndian(result);
    } else {
        result = qFromLittleEndian(result);
    }

    return result;
}

qint64 XBinary::read_int64(qint64 nOffset, bool bIsBigEndian)
{
    qint64 result = 0;

    read_array(nOffset, (char *)(&result), 8);

    if (bIsBigEndian) {
        result = qFromBigEndian(result);
    } else {
        result = qFromLittleEndian(result);
    }

    return (qint64)result;
}

float XBinary::read_float16(qint64 nOffset, bool bIsBigEndian)
{
    // TODO Check
    quint16 float16_value = read_uint16(nOffset, bIsBigEndian);

    // MSB -> LSB
    // float16=1bit: sign, 5bit: exponent, 10bit: fraction
    // float32=1bit: sign, 8bit: exponent, 23bit: fraction
    // for normal exponent(1 to 0x1e): value=2**(exponent-15)*(1.fraction)
    // for denormalized exponent(0): value=2**-14*(0.fraction)
    quint32 sign = float16_value >> 15;
    quint32 exponent = (float16_value >> 10) & 0x1F;
    quint32 fraction = (float16_value & 0x3FF);
    quint32 float32_value = 0;
    if (exponent == 0) {
        if (fraction == 0) {
            // zero
            float32_value = (sign << 31);
        } else {
            // can be represented as ordinary value in float32
            // 2 ** -14 * 0.0101
            // => 2 ** -16 * 1.0100
            // qint32 int_exponent = -14;
            exponent = 127 - 14;
            while ((fraction & (1 << 10)) == 0) {
                // int_exponent--;
                exponent--;
                fraction <<= 1;
            }
            fraction &= 0x3FF;
            // int_exponent += 127;
            float32_value = (sign << 31) | (exponent << 23) | (fraction << 13);
        }
    } else if (exponent == 0x1F) {
        /* Inf or NaN */
        float32_value = (sign << 31) | (0xFF << 23) | (fraction << 13);
    } else {
        /* ordinary number */
        float32_value = (sign << 31) | ((exponent + (127 - 15)) << 23) | (fraction << 13);
    }

    float result = 0;
    static_assert(sizeof(result) == sizeof(float32_value), "float32 storage size mismatch");
    memcpy(&result, &float32_value, sizeof(result));

    return result;
}

float XBinary::read_float(qint64 nOffset, bool bIsBigEndian)
{
    float result = 0;

    read_array(nOffset, (char *)(&result), 4);

    endian_float(&result, bIsBigEndian);

    return result;
}

double XBinary::read_double(qint64 nOffset, bool bIsBigEndian)
{
    double result = 0;

    read_array(nOffset, (char *)(&result), 8);

    endian_double(&result, bIsBigEndian);

    return result;
}

quint32 XBinary::read_uint24(qint64 nOffset, bool bIsBigEndian)
{
    quint32 result = 0;

    if (bIsBigEndian) {
        read_array(nOffset, (char *)(&result) + 1, 3);
        result = qFromBigEndian(result);
    } else {
        read_array(nOffset, (char *)(&result) + 0, 3);
        result = qFromLittleEndian(result);
    }

    return (result & (0xFFFFFF));
}

qint32 XBinary::read_int24(qint64 nOffset, bool bIsBigEndian)
{
    quint32 nValue = read_uint24(nOffset, bIsBigEndian);

    if (nValue & 0x800000) {
        nValue |= 0xFF000000;
    }

    return (qint32)nValue;
}

XBinary::PACKED_UINT XBinary::read_packedNumber(qint64 nOffset, qint64 nSize)
{
    PACKED_UINT result = {};

    if (nSize <= 0) {
        return result;
    }

    nSize = qMin((qint64)9, nSize);

    QByteArray baData = read_array(nOffset, nSize);

    return XBinary::_read_packedNumber(baData.data(), baData.size());
}

qint64 XBinary::write_ansiString(qint64 nOffset, const QString &sString, qint64 nMaxSize)
{
    if ((nOffset < 0) || (nMaxSize < -1)) {
        return 0;
    }

    const QByteArray baString = sString.toLatin1();
    const qint64 nTerminatedSize = (qint64)baString.size() + 1;
    const qint64 nWriteSize = (nMaxSize == -1) ? nTerminatedSize : qMin(nMaxSize, nTerminatedSize);

    return write_array_process(nOffset, baString.constData(), nWriteSize);
}

void XBinary::write_ansiStringFix(qint64 nOffset, qint64 nSize, const QString &sString)
{
    if ((nOffset < 0) || (nSize <= 0)) {
        return;
    }

    const QByteArray baString = sString.toLatin1();

    if (!zeroFill(nOffset, nSize)) {
        return;
    }

    const qint64 nStringSize = qMin(nSize, (qint64)baString.size());

    if (nStringSize > 0) {
        write_array_process(nOffset, baString.constData(), nStringSize);
    }
}

qint64 XBinary::write_unicodeString(qint64 nOffset, const QString &sString, qint64 nMaxSize, bool bIsBigEndian)
{
    qint64 nResult = 0;

    if ((nMaxSize > 0) && (nMaxSize < 0x10000)) {
        qint64 nSize = qMin((qint64)sString.size(), nMaxSize);

        quint16 *pUtf16 = (quint16 *)sString.utf16();

        for (qint32 i = 0; i < nSize; i++) {
            write_uint16(nOffset + 2 * i, *(pUtf16 + i), bIsBigEndian);
            nResult++;
        }
    }

    return nResult;
}

QString XBinary::read_ansiString(qint64 nOffset, qint64 nMaxSize)
{
    QString sResult;

    if (nMaxSize > 0x10000) {
        nMaxSize = 0x10000;
    }

    if (nMaxSize > 0) {
        QByteArray baBuffer = read_array(nOffset, nMaxSize);
        qint32 nActualSize = (qint32)baBuffer.size();
        qint32 nLen = 0;

        while ((nLen < nActualSize) && (baBuffer.at(nLen) != 0)) {
            nLen++;
        }

        sResult = QString::fromLatin1(baBuffer.constData(), nLen);
    }

    return sResult;
}

QString XBinary::read_unicodeString(qint64 nOffset, qint64 nMaxSize, bool bIsBigEndian)
{
    QString sResult;

    if ((nMaxSize > 0) && (nMaxSize < 0x10000)) {
        QByteArray baBuffer = read_array(nOffset, nMaxSize * 2);
        qint32 nActualWords = (qint32)(baBuffer.size() / 2);
        const char *pData = baBuffer.constData();
        sResult.reserve(nActualWords);

        for (qint32 i = 0; i < nActualWords; i++) {
            quint16 nWord;
            memcpy(&nWord, pData + i * 2, 2);
            if (bIsBigEndian) {
                nWord = (quint16)(((nWord & 0xFF) << 8) | ((nWord >> 8) & 0xFF));
            }
            if (nWord == 0) {
                break;
            }
            sResult.append(QChar(nWord));
        }
    }

    return sResult;
}

QString XBinary::read_utf32String(qint64 nOffset, qint64 nMaxByteSize, bool bIsBigEndian)
{
    QString sResult;

    if (nMaxByteSize > 0x40000) {
        nMaxByteSize = 0x40000;
    }

    nMaxByteSize -= nMaxByteSize % 4;

    if ((nOffset < 0) || (nMaxByteSize <= 0)) {
        return sResult;
    }

    const QByteArray baBuffer = read_array(nOffset, nMaxByteSize);
    const qint32 nActualDwords = baBuffer.size() / 4;
    QVector<uint> listCodepoints;
    listCodepoints.reserve(nActualDwords);

    for (qint32 i = 0; i < nActualDwords; i++) {
        const uchar *pData = reinterpret_cast<const uchar *>(baBuffer.constData() + i * 4);
        quint32 nCode = 0;

        if (bIsBigEndian) {
            nCode = ((quint32)pData[0] << 24) | ((quint32)pData[1] << 16) | ((quint32)pData[2] << 8) | pData[3];
        } else {
            nCode = ((quint32)pData[3] << 24) | ((quint32)pData[2] << 16) | ((quint32)pData[1] << 8) | pData[0];
        }

        if (nCode == 0) {
            break;
        }

        if ((nCode > 0x10FFFF) || ((nCode >= 0xD800) && (nCode <= 0xDFFF))) {
            nCode = 0xFFFD;
        }

        listCodepoints.append((uint)nCode);
    }

    if (!listCodepoints.isEmpty()) {
        sResult = QString::fromUcs4(listCodepoints.constData(), listCodepoints.size());
    }

    return sResult;
}

QString XBinary::read_ucsdString(qint64 nOffset)
{
    QString sResult;

    qint32 nSize = read_uint8(nOffset);

    if (nSize > 0) {
        QByteArray baData = read_array(nOffset + 1, nSize);

        if (baData.size() != nSize) {
            return sResult;
        }

        for (qint32 i = 0; i < nSize; i++) {
            if (baData[i] == '\0') {
                baData[i] = 0x20;
            }
        }

        sResult = QString::fromLatin1(baData.constData(), baData.size());
    }

    return sResult;
}

QString XBinary::read_utf8String(qint64 nOffset, qint64 nMaxSize)
{
    QString sResult;

    if (nMaxSize > 0x10000) {
        nMaxSize = 0x10000;
    }

    if (nMaxSize > 0) {
        QByteArray baBuffer = read_array(nOffset, nMaxSize);
        qint32 nActualSize = (qint32)baBuffer.size();
        qint32 nLen = 0;

        while ((nLen < nActualSize) && (baBuffer.at(nLen) != 0)) {
            nLen++;
        }

        sResult = QString::fromUtf8(baBuffer.constData(), nLen);
    }

    return sResult;
}

QString XBinary::_read_utf8String(qint64 nOffset, qint64 nMaxSize)
{
    QString sResult;

    const qint64 nDeviceSize = getSize();
    if ((nOffset < 0) || (nMaxSize <= 0) || (nOffset >= nDeviceSize)) {
        return sResult;
    }

    nMaxSize = qMin(nMaxSize, nDeviceSize - nOffset);
    PACKED_UINT ulebSize = read_uleb128(nOffset, nMaxSize);

    if (!ulebSize.bIsValid || (ulebSize.nByteSize > (quint64)nMaxSize) ||
        (ulebSize.nValue > (quint64)(nMaxSize - (qint64)ulebSize.nByteSize)) ||
        (ulebSize.nValue > (quint64)(std::numeric_limits<qint32>::max)())) {
        return sResult;
    }

    const qint64 nStringOffset = nOffset + (qint64)ulebSize.nByteSize;
    const QByteArray baString = read_array(nStringOffset, (qint64)ulebSize.nValue);

    if (baString.size() == (qint64)ulebSize.nValue) {
        sResult = QString::fromUtf8(baString.constData(), baString.size());  // TODO mutf8
    }

    return sResult;
}

QString XBinary::_read_utf8String(char *pData, qint64 nMaxSize)
{
    QString sResult;

    if (!pData || (nMaxSize <= 0)) {
        return sResult;
    }

    PACKED_UINT ulebSize = _read_uleb128(pData, nMaxSize);

    if (!ulebSize.bIsValid || (ulebSize.nByteSize > (quint64)nMaxSize) ||
        (ulebSize.nValue > (quint64)(nMaxSize - (qint64)ulebSize.nByteSize)) ||
        (ulebSize.nValue > (quint64)(std::numeric_limits<qint32>::max)())) {
        return sResult;
    }

    const qint32 nStringSize = (qint32)ulebSize.nValue;

    sResult = QString::fromUtf8(pData + ulebSize.nByteSize, nStringSize);

    return sResult;
}

QString XBinary::_read_utf8String(qint64 nOffset, char *pData, qint32 nDataSize, qint32 nDataOffset)
{
    QString sResult;

    if (!pData || (nDataSize <= 0) || (nDataOffset < 0)) {
        return sResult;
    }

    if (nOffset < (qint64)nDataOffset) {
        return sResult;
    }

    const qint64 nRelativeOffset = nOffset - (qint64)nDataOffset;

    if (nRelativeOffset < (qint64)nDataSize) {
        char *pStringData = pData + nRelativeOffset;
        qint32 nStringSize = nDataSize - (qint32)nRelativeOffset;
        sResult = XBinary::_read_utf8String(pStringData, nStringSize);
    }

    return sResult;
}

static QString decodeWindows1250(const QByteArray &baData)
{
    // QTextCodec availability is build-dependent even in Qt 5. Keep the
    // Windows-1250 contract used by persisted MS_RECORD metadata deterministic
    // on every supported host instead of falling back to locale/Latin-1.
    static const quint16 g_anWindows1250HighBytes[128] = {
        0x20AC, 0xFFFD, 0x201A, 0xFFFD, 0x201E, 0x2026, 0x2020, 0x2021, 0xFFFD, 0x2030, 0x0160, 0x2039, 0x015A, 0x0164, 0x017D, 0x0179,
        0xFFFD, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0xFFFD, 0x2122, 0x0161, 0x203A, 0x015B, 0x0165, 0x017E, 0x017A,
        0x00A0, 0x02C7, 0x02D8, 0x0141, 0x00A4, 0x0104, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x015E, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x017B,
        0x00B0, 0x00B1, 0x02DB, 0x0142, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x0105, 0x015F, 0x00BB, 0x013D, 0x02DD, 0x013E, 0x017C,
        0x0154, 0x00C1, 0x00C2, 0x0102, 0x00C4, 0x0139, 0x0106, 0x00C7, 0x010C, 0x00C9, 0x0118, 0x00CB, 0x011A, 0x00CD, 0x00CE, 0x010E,
        0x0110, 0x0143, 0x0147, 0x00D3, 0x00D4, 0x0150, 0x00D6, 0x00D7, 0x0158, 0x016E, 0x00DA, 0x0170, 0x00DC, 0x00DD, 0x0162, 0x00DF,
        0x0155, 0x00E1, 0x00E2, 0x0103, 0x00E4, 0x013A, 0x0107, 0x00E7, 0x010D, 0x00E9, 0x0119, 0x00EB, 0x011B, 0x00ED, 0x00EE, 0x010F,
        0x0111, 0x0144, 0x0148, 0x00F3, 0x00F4, 0x0151, 0x00F6, 0x00F7, 0x0159, 0x016F, 0x00FA, 0x0171, 0x00FC, 0x00FD, 0x0163, 0x02D9,
    };

    QString sResult;
    sResult.reserve(baData.size());

    for (char cValue : baData) {
        const quint8 nValue = (quint8)cValue;
        const quint16 nUnicode = (nValue < 0x80) ? (quint16)nValue : g_anWindows1250HighBytes[nValue - 0x80];
        sResult.append(QChar(nUnicode));
    }

    return sResult;
}

QString XBinary::read_codePageString(qint64 nOffset, qint64 nMaxByteSize, const QString &sCodePage)
{
    QString sResult;
    QByteArray baData = read_array(nOffset, nMaxByteSize);

    if (sCodePage.compare(QStringLiteral("Windows-1250"), Qt::CaseInsensitive) == 0) {
        sResult = decodeWindows1250(baData);
    } else {

#if (QT_VERSION_MAJOR < 6) || defined(QT_CORE5COMPAT_LIB)
        QTextCodec *pCodec = QTextCodec::codecForName(sCodePage.toLatin1().data());

        if (pCodec) {
            sResult = pCodec->toUnicode(baData);
        }
#endif
    }

    sResult = sResult.section(QChar(0xFFFD), 0, 0);
    sResult = sResult.section(QChar(0), 0, 0);

    return sResult;
}

QString XBinary::read_msRecordString(const MS_RECORD &record, qint64 nOffset)
{
    if ((nOffset < 0) || (record.nSize == 0)) {
        return QString();
    }

    const VT valueType = (VT)record.nValueType;

    if ((valueType == VT_A) || (valueType == VT_A_I)) {
        if (record.nInfo != CODEPAGE_ASCII) {
            const QString sCodepage = codepageIdToString(record.nInfo);

            if (!sCodepage.isEmpty()) {
                return read_codePageString(nOffset, record.nSize, sCodepage);
            }
        }

        return read_ansiString(nOffset, record.nSize);
    }

    if ((valueType == VT_U) || (valueType == VT_U_I)) {
        if ((record.nSize % 2) != 0) {
            return QString();
        }

        return read_unicodeString(nOffset, record.nSize / 2, record.nInfo == ENDIAN_BIG);
    }

    if (valueType == VT_UTF32) {
        if ((record.nSize % 4) != 0) {
            return QString();
        }

        return read_utf32String(nOffset, record.nSize, record.nInfo == ENDIAN_BIG);
    }

    if ((valueType == VT_UTF8) || (valueType == VT_UTF8_I)) {
        return read_utf8String(nOffset, record.nSize);
    }

    return read_value(valueType, nOffset, record.nSize, record.nInfo == ENDIAN_BIG).toString();
}

bool XBinary::isUnicodeStringLatin(qint64 nOffset, qint64 nMaxSize, bool bIsBigEndian)
{
    bool bResult = true;

    if ((nMaxSize > 0) && (nMaxSize < 0x10000)) {
        for (qint32 i = 0; i < nMaxSize; i++) {
            quint16 nWord = read_uint16(nOffset + 2 * i, bIsBigEndian);

            if (nWord == 0) {
                break;
            } else if (nWord > 0x7F) {
                bResult = false;
                break;
            }
        }
    }

    return bResult;
}

void XBinary::write_uint8(qint64 nOffset, quint8 nValue)
{
    write_array(nOffset, (char *)(&nValue), 1);
}

void XBinary::write_int8(qint64 nOffset, qint8 nValue)
{
    quint8 _value = (quint8)nValue;
    write_array(nOffset, (char *)(&_value), 1);
}

void XBinary::write_uint16(qint64 nOffset, quint16 nValue, bool bIsBigEndian)
{
    if (bIsBigEndian) {
        nValue = qFromBigEndian(nValue);
    } else {
        nValue = qFromLittleEndian(nValue);
    }

    write_array(nOffset, (char *)(&nValue), 2);
}

void XBinary::write_int16(qint64 nOffset, qint16 nValue, bool bIsBigEndian)
{
    quint16 _value = (quint16)nValue;

    if (bIsBigEndian) {
        _value = qFromBigEndian(_value);
    } else {
        _value = qFromLittleEndian(_value);
    }

    write_array(nOffset, (char *)(&_value), 2);
}

void XBinary::write_uint32(qint64 nOffset, quint32 nValue, bool bIsBigEndian)
{
    if (bIsBigEndian) {
        nValue = qFromBigEndian(nValue);
    } else {
        nValue = qFromLittleEndian(nValue);
    }

    write_array(nOffset, (char *)(&nValue), 4);
}

void XBinary::write_int32(qint64 nOffset, qint32 nValue, bool bIsBigEndian)
{
    quint32 _value = (quint32)nValue;

    if (bIsBigEndian) {
        _value = qFromBigEndian(_value);
    } else {
        _value = qFromLittleEndian(_value);
    }

    write_array(nOffset, (char *)(&_value), 4);
}

void XBinary::write_uint64(qint64 nOffset, quint64 nValue, bool bIsBigEndian)
{
    if (bIsBigEndian) {
        nValue = qFromBigEndian(nValue);
    } else {
        nValue = qFromLittleEndian(nValue);
    }

    write_array(nOffset, (char *)(&nValue), 8);
}

void XBinary::write_int64(qint64 nOffset, qint64 nValue, bool bIsBigEndian)
{
    quint64 _value = (quint64)nValue;

    if (bIsBigEndian) {
        _value = qFromBigEndian(_value);
    } else {
        _value = qFromLittleEndian(_value);
    }

    write_array(nOffset, (char *)(&_value), 8);
}

void XBinary::write_float16(qint64 nOffset, float fValue, bool bIsBigEndian)
{
    // TODO Check
    quint32 fltInt32 = *(quint32 *)(&fValue);
    quint16 fltInt16 = 0;

    fltInt16 = (fltInt32 >> 31) << 5;
    quint16 tmp = (fltInt32 >> 23) & 0xff;
    tmp = (tmp - 0x70) & ((quint32)((qint32)(0x70 - tmp) >> 4) >> 27);
    fltInt16 = (fltInt16 | tmp) << 10;
    fltInt16 |= (fltInt32 >> 13) & 0x3ff;

    if (bIsBigEndian) {
        fltInt16 = qFromBigEndian(fltInt16);
    } else {
        fltInt16 = qFromLittleEndian(fltInt16);
    }

    write_array(nOffset, (char *)(&fltInt16), 2);
}

void XBinary::write_float(qint64 nOffset, float fValue, bool bIsBigEndian)
{
    endian_float(&fValue, bIsBigEndian);

    write_array(nOffset, (char *)(&fValue), 4);
}

void XBinary::write_double(qint64 nOffset, double dValue, bool bIsBigEndian)
{
    endian_double(&dValue, bIsBigEndian);

    write_array(nOffset, (char *)(&dValue), 8);
}

QString XBinary::read_UUID_bytes(qint64 nOffset)
{
    // TODO check!
    // TODO Check Endian
    QString sResult = QString("%1-%2-%3-%4-%5")
                          .arg(read_array(nOffset + 0, 4).toHex().data(), read_array(nOffset + 4, 2).toHex().data(), read_array(nOffset + 6, 2).toHex().data(),
                               read_array(nOffset + 8, 2).toHex().data(), read_array(nOffset + 10, 6).toHex().data());

    return sResult;
}

void XBinary::write_UUID_bytes(qint64 nOffset, const QString &sValue)
{
    QString _sValue = sValue;
    // TODO Check
    _sValue = _sValue.remove("-");

    QByteArray baUUID = QByteArray::fromHex(_sValue.toLatin1().data());

    write_array(nOffset, baUUID.data(), 16);
}

QString XBinary::read_UUID(qint64 nOffset, bool bIsBigEndian)
{
    QString sResult = QString("%1-%2-%3-%4-%5")
                          .arg(valueToHex(read_uint32(nOffset + 0, bIsBigEndian), bIsBigEndian), valueToHex(read_uint16(nOffset + 4, bIsBigEndian), bIsBigEndian),
                               valueToHex(read_uint16(nOffset + 6, bIsBigEndian), bIsBigEndian), valueToHex(read_uint16(nOffset + 8, bIsBigEndian), bIsBigEndian),
                               read_array(nOffset + 10, 6).toHex().data());

    return sResult;
}

quint8 XBinary::_read_uint8(const char *pData)
{
    return *(quint8 *)pData;
}

qint8 XBinary::_read_int8(const char *pData)
{
    return *(qint8 *)pData;
}

quint16 XBinary::_read_uint16(const char *pData, bool bIsBigEndian)
{
    quint16 result = *(quint16 *)pData;

    if (bIsBigEndian) {
        result = qFromBigEndian(result);
    } else {
        result = qFromLittleEndian(result);
    }

    return result;
}

qint16 XBinary::_read_int16(const char *pData, bool bIsBigEndian)
{
    qint16 result = *(qint16 *)pData;

    if (bIsBigEndian) {
        result = qFromBigEndian(result);
    } else {
        result = qFromLittleEndian(result);
    }

    return result;
}

quint32 XBinary::_read_uint24(const char *pData, bool bIsBigEndian)
{
    quint32 result = 0;

    if (bIsBigEndian) {
        _copyMemory(((char *)(&result)) + 1, pData, 3);
        result = qFromBigEndian(result);
    } else {
        _copyMemory(((char *)(&result)) + 0, pData, 3);
        result = qFromLittleEndian(result);
    }

    return (result & (0xFFFFFF));
}

quint32 XBinary::_read_uint32(const char *pData, bool bIsBigEndian)
{
    quint32 result = *(quint32 *)pData;

    if (bIsBigEndian) {
        result = qFromBigEndian(result);
    } else {
        result = qFromLittleEndian(result);
    }

    return result;
}

qint32 XBinary::_read_int32(const char *pData, bool bIsBigEndian)
{
    qint32 result = *(qint32 *)pData;

    if (bIsBigEndian) {
        result = qFromBigEndian(result);
    } else {
        result = qFromLittleEndian(result);
    }

    return result;
}

quint64 XBinary::_read_uint64(const char *pData, bool bIsBigEndian)
{
    quint64 result = *(quint64 *)pData;

    if (bIsBigEndian) {
        result = qFromBigEndian(result);
    } else {
        result = qFromLittleEndian(result);
    }

    return result;
}

qint64 XBinary::_read_int64(const char *pData, bool bIsBigEndian)
{
    qint64 result = *(qint64 *)pData;

    if (bIsBigEndian) {
        result = qFromBigEndian(result);
    } else {
        result = qFromLittleEndian(result);
    }

    return result;
}

QString XBinary::_read_ansiString(const char *pData, qint32 nMaxSize)
{
    QString sResult;

    QByteArray baData(pData, nMaxSize);
    sResult.append(baData.data());

    return sResult;
}

QByteArray XBinary::_read_byteArray(const char *pData, qint32 nSize)
{
    return QByteArray(pData, nSize);
}

float XBinary::_read_float(const char *pData, bool bIsBigEndian)
{
    float result = *(float *)pData;

    endian_float(&result, bIsBigEndian);

    return result;
}

double XBinary::_read_double(const char *pData, bool bIsBigEndian)
{
    double result = *(double *)pData;

    endian_double(&result, bIsBigEndian);

    return result;
}

quint64 XBinary::_read_value(MODE mode, const char *pData, bool bIsBigEndian)
{
    quint64 nResult = 0;

    if (mode == MODE::MODE_8) {
        nResult = _read_uint8(pData);
    } else if (mode == MODE::MODE_16) {
        nResult = _read_uint16(pData, bIsBigEndian);
    } else if (mode == MODE::MODE_32) {
        nResult = _read_uint32(pData, bIsBigEndian);
    } else if (mode == MODE::MODE_64) {
        nResult = _read_uint64(pData, bIsBigEndian);
    }

    return nResult;
}

quint8 XBinary::_read_uint8_safe(char *pBuffer, qint32 nBufferSize, qint32 nOffset)
{
    quint8 result = 0;

    if ((nOffset >= 0) && (nOffset < nBufferSize)) {
        result = _read_uint8(pBuffer + nOffset);
    }

    return result;
}

quint16 XBinary::_read_uint16_safe(char *pBuffer, qint32 nBufferSize, qint32 nOffset, bool bIsBigEndian)
{
    quint16 result = 0;

    if ((nOffset >= 0) && (nOffset + 1 < nBufferSize)) {
        result = _read_uint16(pBuffer + nOffset, bIsBigEndian);
    }

    return result;
}

quint32 XBinary::_read_uint32_safe(char *pBuffer, qint32 nBufferSize, qint32 nOffset, bool bIsBigEndian)
{
    quint32 result = 0;

    if ((nOffset >= 0) && (nOffset + 3 < nBufferSize)) {
        result = _read_uint32(pBuffer + nOffset, bIsBigEndian);
    }

    return result;
}

quint64 XBinary::_read_uint64_safe(char *pBuffer, qint32 nBufferSize, qint32 nOffset, bool bIsBigEndian)
{
    quint64 result = 0;

    if ((nOffset >= 0) && (nOffset + 7 < nBufferSize)) {
        result = _read_uint64(pBuffer + nOffset, bIsBigEndian);
    }

    return result;
}

QString XBinary::_read_ansiString_safe(char *pBuffer, qint32 nBufferSize, qint32 nOffset, qint32 nMaxSize)
{
    QString sResult;

    if (nOffset + nMaxSize > nBufferSize) {
        nMaxSize = nBufferSize - nOffset;
    }

    if ((nOffset >= 0) && (nOffset < nBufferSize)) {
        sResult = _read_ansiString(pBuffer + nOffset, nMaxSize);
    }

    return sResult;
}

bool XBinary::_read_bool_safe(char *pBuffer, qint32 nBufferSize, qint32 nBitPosition)
{
    bool bResult = false;

    if (nBitPosition < nBufferSize * 8) {
        qint32 _nIndex = nBitPosition / 8;
        qint32 _nPosition = nBitPosition % 8;
        bResult = (quint8)(pBuffer[_nIndex]) & (0x1 << _nPosition);
    }

    return bResult;
}

bool XBinary::_read_bool_safe_rev(char *pBuffer, qint32 nBufferSize, qint32 nBitPosition)
{
    bool bResult = false;

    if (nBitPosition < nBufferSize * 8) {
        qint32 _nIndex = nBitPosition / 8;
        qint32 _nPosition = nBitPosition % 8;
        bResult = (quint8)(pBuffer[_nIndex]) & (0x80 >> _nPosition);
    }

    return bResult;
}

quint32 XBinary::_getBitCount_safe(char *pBuffer, qint32 nBufferSize)
{
    quint32 nResult = 0;

    if (pBuffer && (nBufferSize > 0)) {
        for (qint32 i = 0; i < nBufferSize; i++) {
            unsigned char byte = static_cast<unsigned char>(pBuffer[i]);

            // Count set bits in the byte
            while (byte) {
                nResult += byte & 1;
                byte >>= 1;
            }
        }
    }

    return nResult;
}

void XBinary::_write_uint8(char *pData, quint8 nValue)
{
    *(quint8 *)pData = nValue;
}

void XBinary::_write_int8(char *pData, qint8 nValue)
{
    *(qint8 *)pData = nValue;
}

void XBinary::_write_uint16(char *pData, quint16 nValue, bool bIsBigEndian)
{
    if (bIsBigEndian) {
        nValue = qToBigEndian(nValue);
    } else {
        nValue = qToLittleEndian(nValue);
    }

    *(quint16 *)pData = nValue;
}

void XBinary::_write_int16(char *pData, qint16 nValue, bool bIsBigEndian)
{
    if (bIsBigEndian) {
        nValue = qToBigEndian(nValue);
    } else {
        nValue = qToLittleEndian(nValue);
    }

    *(qint16 *)pData = nValue;
}

void XBinary::_write_uint32(char *pData, quint32 nValue, bool bIsBigEndian)
{
    if (bIsBigEndian) {
        nValue = qToBigEndian(nValue);
    } else {
        nValue = qToLittleEndian(nValue);
    }

    *(quint32 *)pData = nValue;
}

void XBinary::_write_int32(char *pData, qint32 nValue, bool bIsBigEndian)
{
    if (bIsBigEndian) {
        nValue = qToBigEndian(nValue);
    } else {
        nValue = qToLittleEndian(nValue);
    }

    *(qint32 *)pData = nValue;
}

void XBinary::_write_uint64(char *pData, quint64 nValue, bool bIsBigEndian)
{
    if (bIsBigEndian) {
        nValue = qToBigEndian(nValue);
    } else {
        nValue = qToLittleEndian(nValue);
    }

    *(quint64 *)pData = nValue;
}

void XBinary::_write_int64(char *pData, qint64 nValue, bool bIsBigEndian)
{
    if (bIsBigEndian) {
        nValue = qToBigEndian(nValue);
    } else {
        nValue = qToLittleEndian(nValue);
    }

    *(qint64 *)pData = nValue;
}

void XBinary::_write_float(char *pData, float fValue, bool bIsBigEndian)
{
    endian_float(&fValue, bIsBigEndian);

    *(float *)pData = fValue;
}

void XBinary::_write_double(char *pData, double dValue, bool bIsBigEndian)
{
    endian_double(&dValue, bIsBigEndian);

    *(double *)pData = dValue;
}

void XBinary::_write_value(MODE mode, char *pData, quint64 nValue, bool bIsBigEndian)
{
    if (mode == MODE::MODE_8) {
        _write_uint8(pData, nValue);
    } else if (mode == MODE::MODE_16) {
        _write_uint16(pData, nValue, bIsBigEndian);
    } else if (mode == MODE::MODE_32) {
        _write_uint32(pData, nValue, bIsBigEndian);
    } else if (mode == MODE::MODE_64) {
        _write_uint64(pData, nValue, bIsBigEndian);
    }
}

quint8 XBinary::read_bcd_uint8(qint64 nOffset)
{
    quint8 nResult = 0;

    nResult = _bcd_decimal(read_uint8(nOffset));

    return nResult;
}

quint16 XBinary::read_bcd_uint16(qint64 nOffset, bool bIsBigEndian)
{
    quint16 nResult = 0;

    quint16 nValue = read_uint16(nOffset, bIsBigEndian);

    nResult = _bcd_decimal(0xFF & nValue) + _bcd_decimal(0xFF & (nValue >> 8)) * 10;

    return nResult;
}

quint16 XBinary::read_bcd_uint32(qint64 nOffset, bool bIsBigEndian)
{
    quint32 nResult = 0;

    quint32 nValue = read_uint32(nOffset, bIsBigEndian);

    nResult =
        _bcd_decimal(0xFF & nValue) + _bcd_decimal(0xFF & (nValue >> 8)) * 10 + _bcd_decimal(0xFF & (nValue >> 16)) * 100 + _bcd_decimal(0xFF & (nValue >> 24)) * 1000;

    return nResult;
}

quint16 XBinary::read_bcd_uint64(qint64 nOffset, bool bIsBigEndian)
{
    quint64 nResult = 0;

    quint64 nValue = read_uint64(nOffset, bIsBigEndian);

    nResult = _bcd_decimal(0xFF & nValue) + _bcd_decimal(0xFF & (nValue >> 8)) * 10 + _bcd_decimal(0xFF & (nValue >> 16)) * 100 +
              _bcd_decimal(0xFF & (nValue >> 24)) * 1000 + _bcd_decimal(0xFF & (nValue >> 32)) * 10000 + _bcd_decimal(0xFF & (nValue >> 40)) * 100000 +
              _bcd_decimal(0xFF & (nValue >> 48)) * 1000000 + _bcd_decimal(0xFF & (nValue >> 56)) * 10000000;

    return nResult;
}

quint8 XBinary::_bcd_decimal(quint8 nValue)
{
    quint8 nResult = 0;

    nResult = ((nValue & 0xF0) >> 4) * 10 + (nValue & 0x0F);

    return nResult;
}

static qint64 _x_findbyte(char *pBuffer, qint64 nTemp, const char *pArray, qint64 nOffset)
{
    qint64 nResult = -1;

#ifdef USE_XSIMD
    nResult = xsimd_find_byte(pBuffer, nTemp, (unsigned char)pArray[0], nOffset);
    if (nResult != -1) return nResult;
#else
    // Fallback to standard memchr
    const void *p = memchr(pBuffer, (unsigned char)pArray[0], (size_t)nTemp);
    if (p) {
        nResult = nOffset + ((const char *)p - pBuffer);
    }
#endif

    return nResult;
}

static qint64 _x_findpattern_bmh(char *pBuffer, qint64 nTemp, const char *pArray, qint64 nArraySize, qint64 nOffset, const qint32 *bmhShift, char nLastSearchChar)
{
    qint64 nResult = -1;

#ifdef USE_XSIMD
    nResult = xsimd_find_pattern_bmh(pBuffer, nTemp, pArray, nArraySize, nOffset);
    if (nResult != -1) return nResult;
#else
    // Fallback: scalar BMH search
    const char *hay = pBuffer;
    qint64 hayLen = nTemp;
    qint64 m = nArraySize;
    qint64 i = 0;
    const qint64 limit = hayLen - m;

    while (i <= limit) {
        unsigned char c = (unsigned char)hay[i + m - 1];

        if (c == (unsigned char)nLastSearchChar) {
            if (memcmp(hay + i, pArray, (size_t)m) == 0) {
                nResult = nOffset + i;
                break;
            }
        }
        i += (qint64)bmhShift[c];
    }
#endif

    return nResult;
}

static qint64 _x_find_notnull(char *pBuffer, qint64 nTemp, qint64 nArraySize, qint64 nOffset)
{
    qint64 nResult = -1;

#ifdef USE_XSIMD
    nResult = xsimd_find_notnull(pBuffer, nTemp, nArraySize, nOffset);
    if (nResult != -1) return nResult;
#else
    // Find first window of length nArraySize with no zero bytes using memchr to skip over zero-containing regions.
    const char *hay = pBuffer;
    qint64 hayLen = nTemp;
    qint64 m = nArraySize;
    const qint64 limit = hayLen - (m - 1);
    qint64 j = 0;
    while (j < limit) {
        const void *pz = memchr(hay + j, 0, (size_t)(hayLen - j));
        qint64 runLen = (pz ? ((const char *)pz - (hay + j)) : (hayLen - j));
        if (runLen >= m) {
            nResult = nOffset + j;
            break;
        }
        // Skip to just after the zero byte
        j += runLen + 1;
    }
#endif

    return nResult;
}

static qint64 _x_find_ansi(char *pBuffer, qint64 nTemp, qint64 nArraySize, qint64 nOffset, const bool *ansiTable)
{
    qint64 nResult = -1;

#ifdef USE_XSIMD
    nResult = xsimd_find_ansi(pBuffer, nTemp, nArraySize, nOffset);
    if (nResult != -1) return nResult;
#else
    // Fallback: scalar loop
    const unsigned char *hay = (const unsigned char *)pBuffer;
    qint64 hayLen = nTemp;
    qint64 m = nArraySize;
    const qint64 limit = hayLen - (m - 1);
    qint64 j = 0;

    while (j < limit) {
        // Skip non-ANSI bytes to the start of an ANSI run
        while (j < hayLen && !ansiTable[hay[j]]) j++;
        if (j >= limit) break;
        qint64 start = j;
        // Extend ANSI run
        while (j < hayLen && ansiTable[hay[j]]) j++;
        qint64 runLen = j - start;
        if (runLen >= m) {
            nResult = nOffset + start;
            break;
        }
    }
#endif

    return nResult;
}

static qint64 _x_find_notansi(char *pBuffer, qint64 nTemp, qint64 nArraySize, qint64 nOffset, const bool *ansiTable)
{
    qint64 nResult = -1;

#ifdef USE_XSIMD
    nResult = xsimd_find_not_ansi(pBuffer, nTemp, nArraySize, nOffset);
    if (nResult != -1) return nResult;
#else
    // Fallback: scalar loop
    const unsigned char *hay = (const unsigned char *)pBuffer;
    qint64 hayLen = nTemp;
    qint64 m = nArraySize;
    const qint64 limit = hayLen - (m - 1);
    qint64 j = 0;

    while (j < limit) {
        // Skip ANSI bytes to the start of a non-ANSI run
        while (j < hayLen && ansiTable[hay[j]]) j++;
        if (j >= limit) break;
        qint64 start = j;
        // Extend non-ANSI run
        while (j < hayLen && !ansiTable[hay[j]]) j++;
        qint64 runLen = j - start;
        if (runLen >= m) {
            nResult = nOffset + start;
            break;
        }
    }
#endif

    return nResult;
}

static qint64 _x_find_notansiandnull(char *pBuffer, qint64 nTemp, qint64 nArraySize, qint64 nOffset, const bool *ansiTable)
{
    qint64 nResult = -1;

#ifdef USE_XSIMD
    nResult = xsimd_find_not_ansi_and_null(pBuffer, nTemp, nArraySize, nOffset);
    if (nResult != -1) return nResult;
#else
    const unsigned char *hay = (const unsigned char *)pBuffer;
    qint64 hayLen = nTemp;
    qint64 m = nArraySize;
    const qint64 limit = hayLen - (m - 1);
    qint64 j = 0;

    while (j < limit) {
        // Skip bytes that are ANSI or zero to the start of a desired run
        while (j < hayLen) {
            unsigned char c = hay[j];
            if ((!ansiTable[c]) && (c != 0)) break;
            j++;
        }
        if (j >= limit) break;
        qint64 start = j;
        // Extend run of non-ANSI and non-zero bytes
        while (j < hayLen) {
            unsigned char c = hay[j];
            if ((ansiTable[c]) || (c == 0)) break;
            j++;
        }
        qint64 runLen = j - start;
        if (runLen >= m) {
            nResult = nOffset + start;
            break;
        }
    }
#endif

    return nResult;
}

static qint64 _x_find_ansinumber(char *pBuffer, qint64 nTemp, qint64 nArraySize, qint64 nOffset)
{
    qint64 nResult = -1;

#ifdef USE_XSIMD
    nResult = xsimd_find_ansi_number(pBuffer, nTemp, nArraySize, nOffset);
    if (nResult != -1) return nResult;
#else
    const unsigned char *hay = (const unsigned char *)pBuffer;
    qint64 hayLen = nTemp;
    qint64 m = nArraySize;
    const qint64 limit = hayLen - (m - 1);
    qint64 j = 0;
    while (j < limit) {
        // Skip bytes that are not digits to the start of a desired run
        while (j < hayLen) {
            unsigned char c = hay[j];
            if ((c >= 0x30) && (c <= 0x39)) break;
            j++;
        }
        if (j >= limit) break;
        qint64 start = j;
        // Extend run of digit bytes
        while (j < hayLen) {
            unsigned char c = hay[j];
            if ((c < 0x30) || (c > 0x39)) break;
            j++;
        }
        qint64 runLen = j - start;
        if (runLen >= m) {
            nResult = nOffset + start;
            break;
        }
    }
#endif

    return nResult;
}

qint64 XBinary::_find_array(ST st, qint64 nOffset, qint64 nSize, const char *pArray, qint64 nArraySize, PDSTRUCT *pPdStruct)
{
    qint64 nResult = -1;

    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }
    // Validate input
    // For ST_COMPAREBYTES, pArray must be provided
    // For other ST modes (NOTNULL, ANSI, etc.), pArray can be nullptr
    if ((st == ST_COMPAREBYTES) && (!pArray)) {
        return -1;
    }

    if ((nArraySize <= 0) || (nArraySize > 0x100000) || !isPdStructNotCanceled(pPdStruct)) {
        return -1;
    }

    const qint64 _nSize = getSize();

    if ((nOffset < 0) || (nOffset > _nSize)) {
        return -1;
    }

    if (nSize == -1) {
        nSize = _nSize - nOffset;
    }

    if ((nSize <= 0) || (nSize > _nSize - nOffset)) {
        return -1;
    }

    if (nArraySize > nSize) {
        return -1;
    }

    qint64 nTemp = 0;

    qint32 _nFreeIndex = XBinary::reservePdStructRecord(pPdStruct, nSize);

    qint64 nStartOffset = nOffset;

    char *pBuffer = nullptr;

    const qint32 nRequestedBufferSize = getBufferSize(pPdStruct);

    if (nRequestedBufferSize <= 0) {
        setPdStructFinished(pPdStruct, _nFreeIndex);
        return -1;
    }

    const qint32 nBufferSize = qMax((qint32)nArraySize, qBound((qint32)0x1000, nRequestedBufferSize, (qint32)0x100000));

    if (!m_pConstMemory) {
        pBuffer = new (std::nothrow) char[nBufferSize];

        if (!pBuffer) {
            setPdStructFinished(pPdStruct, _nFreeIndex);
            return -1;
        }
    }

    // Precompute BMH shift table for byte-compare mode
    qint32 bmhShift[256];
    bool bUseBMH = (st == ST_COMPAREBYTES) && (nArraySize >= 2);
    if (bUseBMH) {
        qint32 m = (qint32)qMin<qint64>(nArraySize, (qint64)0x7fffffff);
        // Initialize all entries to m
        for (int i = 0; i < 256; ++i) {
            bmhShift[i] = m;
        }
        // Set shifts for all but last character
        for (int i = 0; i < m - 1; ++i) bmhShift[(quint8)pArray[i]] = m - 1 - i;
    }

    // Precompute ANSI classification table if needed
    bool needAnsiTable = (st == ST_ANSI) || (st == ST_NOTANSI) || (st == ST_NOTANSIANDNULL) || (st == ST_ANSINUMBER);
    bool ansiTable[256];
    if (needAnsiTable) {
        for (int i = 0; i < 256; ++i) {
            ansiTable[i] = isAnsiSymbol((quint8)i);
        }
    }

    const char nLastSearchChar = (st == ST_COMPAREBYTES) ? pArray[nArraySize - 1] : 0;

    while ((nSize >= nArraySize) && isPdStructNotCanceled(pPdStruct)) {
        nTemp = (nSize < nBufferSize) ? nSize : nBufferSize;

        if (m_pConstMemory) {
            pBuffer = (char *)m_pConstMemory + nOffset;
        } else {
            qint64 nBytesRead = read_array_process(nOffset, pBuffer, nTemp, pPdStruct);

            if (nBytesRead != nTemp) {
                setPdStructErrorString(pPdStruct, tr("Read error"));
                break;
            }
        }

        if (st == ST_COMPAREBYTES) {
            // Fast path: single-byte needle with SIMD optimization
            if (nArraySize == 1) {
                nResult = _x_findbyte(pBuffer, nTemp, pArray, nOffset);
                if (nResult != -1) break;
            } else if (bUseBMH) {
                // Boyer–Moore–Horspool pattern search
                nResult = _x_findpattern_bmh(pBuffer, nTemp, pArray, nArraySize, nOffset, bmhShift, nLastSearchChar);
                if (nResult != -1) break;
            } else {
                // Fallback naive scan
                const qint64 limit = nTemp - (nArraySize - 1);
                for (qint64 i = 0; i < limit; ++i) {
                    if (compareMemory(pBuffer + i, pArray, nArraySize)) {
                        nResult = nOffset + i;
                        break;
                    }
                }

                if (nResult != -1) break;
            }
        } else if (st == ST_NOTNULL) {
            nResult = _x_find_notnull(pBuffer, nTemp, nArraySize, nOffset);
            if (nResult != -1) break;
        } else if (st == ST_ANSI) {
            nResult = _x_find_ansi(pBuffer, nTemp, nArraySize, nOffset, ansiTable);
            if (nResult != -1) break;
        } else if (st == ST_NOTANSI) {
            nResult = _x_find_notansi(pBuffer, nTemp, nArraySize, nOffset, ansiTable);
            if (nResult != -1) break;
        } else if (st == ST_NOTANSIANDNULL) {
            nResult = _x_find_notansiandnull(pBuffer, nTemp, nArraySize, nOffset, ansiTable);
            if (nResult != -1) break;
        } else if (st == ST_ANSINUMBER) {
            nResult = _x_find_ansinumber(pBuffer, nTemp, nArraySize, nOffset);
            if (nResult != -1) break;
        }

        if (nResult != -1) {
            break;
        }

        nSize -= nTemp - (nArraySize - 1);
        nOffset += nTemp - (nArraySize - 1);

        XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, nOffset - nStartOffset);
    }

    if (!m_pConstMemory) {
        delete[] pBuffer;
    }

    setPdStructFinished(pPdStruct, _nFreeIndex);

    if (!isPdStructNotCanceled(pPdStruct)) {
        nResult = -1;
    }

    return nResult;
}

qint64 XBinary::find_array(qint64 nOffset, qint64 nSize, const char *pArray, qint64 nArraySize, PDSTRUCT *pPdStruct)
{
    return _find_array(ST_COMPAREBYTES, nOffset, nSize, pArray, nArraySize, pPdStruct);
}

qint64 XBinary::find_byteArray(qint64 nOffset, qint64 nSize, const QByteArray &baData, PDSTRUCT *pPdStruct)
{
    return _find_array(ST_COMPAREBYTES, nOffset, nSize, baData.data(), baData.size(), pPdStruct);
}

qint64 XBinary::find_uint8(qint64 nOffset, qint64 nSize, quint8 nValue, PDSTRUCT *pPdStruct)
{
    quint8 baValue[1];
    baValue[0] = nValue;
    return find_array(nOffset, nSize, (char *)baValue, 1, pPdStruct);
}

qint64 XBinary::find_int8(qint64 nOffset, qint64 nSize, qint8 nValue, PDSTRUCT *pPdStruct)
{
    quint8 baValue[1];
    baValue[0] = (quint8)nValue;
    return find_array(nOffset, nSize, (char *)baValue, 1, pPdStruct);
}

qint64 XBinary::find_uint16(qint64 nOffset, qint64 nSize, quint16 nValue, bool bIsBigEndian, PDSTRUCT *pPdStruct)
{
    quint8 baValue[2];

    if (bIsBigEndian) {
        baValue[0] = (quint8)(nValue >> 8);
        baValue[1] = (quint8)nValue;
    } else {
        baValue[0] = (quint8)nValue;
        baValue[1] = (quint8)(nValue >> 8);
    }

    return find_array(nOffset, nSize, (char *)baValue, 2, pPdStruct);
}

qint64 XBinary::find_int16(qint64 nOffset, qint64 nSize, qint16 nValue, bool bIsBigEndian, PDSTRUCT *pPdStruct)
{
    quint16 _value = (quint16)nValue;
    quint8 baValue[2];

    if (bIsBigEndian) {
        baValue[0] = (quint8)(_value >> 8);
        baValue[1] = (quint8)_value;
    } else {
        baValue[0] = (quint8)_value;
        baValue[1] = (quint8)(_value >> 8);
    }

    return find_array(nOffset, nSize, (char *)baValue, 2, pPdStruct);
}

qint64 XBinary::find_uint32(qint64 nOffset, qint64 nSize, quint32 nValue, bool bIsBigEndian, PDSTRUCT *pPdStruct)
{
    // // DEBUG: Print for ECD signature search (0x06054B50)
    // if (nValue == 0x06054B50) {
    //     qDebug() << "find_uint32: Searching for ECD signature 0x06054B50 at offset" << nOffset << "nSize=" << nSize;
    // }

    quint8 baValue[4];

    if (bIsBigEndian) {
        baValue[0] = (quint8)(nValue >> 24);
        baValue[1] = (quint8)(nValue >> 16);
        baValue[2] = (quint8)(nValue >> 8);
        baValue[3] = (quint8)nValue;
    } else {
        baValue[0] = (quint8)nValue;
        baValue[1] = (quint8)(nValue >> 8);
        baValue[2] = (quint8)(nValue >> 16);
        baValue[3] = (quint8)(nValue >> 24);
    }

    qint64 nResult = find_array(nOffset, nSize, (char *)baValue, 4, pPdStruct);

    // if (nValue == 0x06054B50) {
    //     qDebug() << "find_uint32: ECD search result:" << nResult;
    // }

    return nResult;
}

qint64 XBinary::find_int32(qint64 nOffset, qint64 nSize, qint32 nValue, bool bIsBigEndian, PDSTRUCT *pPdStruct)
{
    quint32 _value = (quint32)nValue;
    quint8 baValue[4];

    if (bIsBigEndian) {
        baValue[0] = (quint8)(_value >> 24);
        baValue[1] = (quint8)(_value >> 16);
        baValue[2] = (quint8)(_value >> 8);
        baValue[3] = (quint8)_value;
    } else {
        baValue[0] = (quint8)_value;
        baValue[1] = (quint8)(_value >> 8);
        baValue[2] = (quint8)(_value >> 16);
        baValue[3] = (quint8)(_value >> 24);
    }

    return find_array(nOffset, nSize, (char *)baValue, 4, pPdStruct);
}

qint64 XBinary::find_uint64(qint64 nOffset, qint64 nSize, quint64 nValue, bool bIsBigEndian, PDSTRUCT *pPdStruct)
{
    quint8 baValue[8];

    if (bIsBigEndian) {
        baValue[0] = (quint8)(nValue >> 56);
        baValue[1] = (quint8)(nValue >> 48);
        baValue[2] = (quint8)(nValue >> 40);
        baValue[3] = (quint8)(nValue >> 32);
        baValue[4] = (quint8)(nValue >> 24);
        baValue[5] = (quint8)(nValue >> 16);
        baValue[6] = (quint8)(nValue >> 8);
        baValue[7] = (quint8)nValue;
    } else {
        baValue[0] = (quint8)nValue;
        baValue[1] = (quint8)(nValue >> 8);
        baValue[2] = (quint8)(nValue >> 16);
        baValue[3] = (quint8)(nValue >> 24);
        baValue[4] = (quint8)(nValue >> 32);
        baValue[5] = (quint8)(nValue >> 40);
        baValue[6] = (quint8)(nValue >> 48);
        baValue[7] = (quint8)(nValue >> 56);
    }

    return find_array(nOffset, nSize, (char *)baValue, 8, pPdStruct);
}

qint64 XBinary::find_int64(qint64 nOffset, qint64 nSize, qint64 nValue, bool bIsBigEndian, PDSTRUCT *pPdStruct)
{
    quint64 _value = (quint64)nValue;
    quint8 baValue[8];

    if (bIsBigEndian) {
        baValue[0] = (quint8)(_value >> 56);
        baValue[1] = (quint8)(_value >> 48);
        baValue[2] = (quint8)(_value >> 40);
        baValue[3] = (quint8)(_value >> 32);
        baValue[4] = (quint8)(_value >> 24);
        baValue[5] = (quint8)(_value >> 16);
        baValue[6] = (quint8)(_value >> 8);
        baValue[7] = (quint8)_value;
    } else {
        baValue[0] = (quint8)_value;
        baValue[1] = (quint8)(_value >> 8);
        baValue[2] = (quint8)(_value >> 16);
        baValue[3] = (quint8)(_value >> 24);
        baValue[4] = (quint8)(_value >> 32);
        baValue[5] = (quint8)(_value >> 40);
        baValue[6] = (quint8)(_value >> 48);
        baValue[7] = (quint8)(_value >> 56);
    }

    return find_array(nOffset, nSize, (char *)baValue, 8, pPdStruct);
}

qint64 XBinary::find_float(qint64 nOffset, qint64 nSize, float fValue, bool bIsBigEndian, PDSTRUCT *pPdStruct)
{
    float _value = fValue;

    endian_float(&_value, bIsBigEndian);

    return find_array(nOffset, nSize, (char *)&_value, 4, pPdStruct);
}

qint64 XBinary::find_double(qint64 nOffset, qint64 nSize, double dValue, bool bIsBigEndian, PDSTRUCT *pPdStruct)
{
    double _value = dValue;

    endian_double(&_value, bIsBigEndian);

    return find_array(nOffset, nSize, (char *)&_value, 8, pPdStruct);
}

void XBinary::endian_float(float *pValue, bool bIsBigEndian)
{
    bool bReverse = false;

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
    bReverse = !bIsBigEndian;
#else
    bReverse = bIsBigEndian;
#endif

    if (bReverse) {
        std::swap(((quint8 *)pValue)[0], ((quint8 *)pValue)[3]);
        std::swap(((quint8 *)pValue)[1], ((quint8 *)pValue)[2]);
    }
}

void XBinary::endian_double(double *pValue, bool bIsBigEndian)
{
    bool bReverse = false;

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
    bReverse = !bIsBigEndian;
#else
    bReverse = bIsBigEndian;
#endif

    if (bReverse) {
        std::swap(((quint8 *)pValue)[0], ((quint8 *)pValue)[7]);
        std::swap(((quint8 *)pValue)[1], ((quint8 *)pValue)[6]);
        std::swap(((quint8 *)pValue)[2], ((quint8 *)pValue)[5]);
        std::swap(((quint8 *)pValue)[3], ((quint8 *)pValue)[4]);
    }
}

qint64 XBinary::find_ansiString(qint64 nOffset, qint64 nSize, const QString &sString, PDSTRUCT *pPdStruct)
{
    QByteArray baString = sString.toLatin1();

    return find_array(nOffset, nSize, baString.data(), baString.size(), pPdStruct);
}

qint64 XBinary::find_unicodeString(qint64 nOffset, qint64 nSize, const QString &sString, bool bIsBigEndian, PDSTRUCT *pPdStruct)
{
    QByteArray baString = getUnicodeString(sString, bIsBigEndian);

    return find_array(nOffset, nSize, baString.data(), baString.size(), pPdStruct);
}

qint64 XBinary::find_utf8String(qint64 nOffset, qint64 nSize, const QString &sString, PDSTRUCT *pPdStruct)
{
    QByteArray baData = sString.toUtf8();

    return find_array(nOffset, nSize, (char *)baData.data(), baData.size(), pPdStruct);
}

QByteArray XBinary::_signatureToSigBytes(const QString &sSignature, PDSTRUCT *pPdStruct)
{
    QByteArray baResult;

    if (!isSignatureValid(sSignature, pPdStruct)) {
        return baResult;
    }

    QString _sSignature = convertSignature(sSignature);
    if (_sSignature.contains(QChar('$')) || _sSignature.contains(QChar('#')) || _sSignature.contains(QChar('+')) ||
        _sSignature.contains(QChar('[')) || _sSignature.contains(QChar(']'))) {
        return baResult;
    }
    qint32 nSignatureSize = _sSignature.size();

    for (qint32 i = 0; (i < nSignatureSize) && isPdStructNotCanceled(pPdStruct);) {
        QChar cSymbol = _sSignature.at(i);
        QChar cSymbol2;

        if ((i + 1) < nSignatureSize) {
            cSymbol2 = _sSignature.at(i + 1);
        }

        if ((cSymbol == QChar('.')) && (cSymbol2 == QChar('.'))) {
            // Wildcard
            baResult.append((char)SIGBYTETYPE_WILDCARD);
            baResult.append((char)0x00);
            i += 2;
        } else if ((cSymbol == QChar('*')) && (cSymbol2 == QChar('*'))) {
            // Not null byte
            baResult.append((char)SIGBYTE_NOT_NULL);
            baResult.append((char)0x00);
            i += 2;
        } else if ((cSymbol == QChar('%')) && (cSymbol2 == QChar('%'))) {
            // ANSI character
            baResult.append((char)SIGBYTETYPE_ANSI);
            baResult.append((char)0x00);
            i += 2;
        } else if ((cSymbol == QChar('%')) && (cSymbol2 == QChar('&'))) {
            // ANSI alphanumeric
            baResult.append((char)SIGBYTETYPE_ANSI_ALPHANUMERIC);
            baResult.append((char)0x00);
            i += 2;
        } else if ((cSymbol == QChar('!')) && (cSymbol2 == QChar('%'))) {
            // Not ANSI
            baResult.append((char)SIGBYTETYPE_NOT_ANSI);
            baResult.append((char)0x00);
            i += 2;
        } else if ((cSymbol == QChar('_')) && (cSymbol2 == QChar('%'))) {
            // Not ANSI and not null
            baResult.append((char)SIGBYTETYPE_NOT_ANSI_AND_NOT_NULL);
            baResult.append((char)0x00);
            i += 2;
        } else if ((cSymbol == QChar('$')) || (cSymbol == QChar('#')) || (cSymbol == QChar('+')) || (cSymbol == QChar('['))) {
            // Skip relative offset, absolute address, find pattern, and address base markers
            i++;
        } else if (cSymbol == QChar(']')) {
            // Skip address base end marker
            i++;
        } else if ((i + 1) < nSignatureSize) {
            // Hex byte
            QString sHexByte = _sSignature.mid(i, 2);
            bool bOk = false;
            quint8 nValue = (quint8)sHexByte.toUInt(&bOk, 16);

            if (bOk) {
                baResult.append((char)SIGBYTETYPE_HEX);
                baResult.append((char)nValue);
                i += 2;
            } else {
                // Invalid hex, skip
                i++;
            }
        } else {
            // End of signature
            break;
        }
    }

    return baResult;
}

bool XBinary::_compareSigBytes(const QByteArray &baSigBytes, const QByteArray &baData, PDSTRUCT *pPdStruct)
{
    return _compareSigBytes(baSigBytes.constData(), baSigBytes.size(), baData.constData(), baData.size(), pPdStruct);
}

bool XBinary::_compareSigBytes(const char *pSigBytes, qint64 nSigBytesSize, const char *pData, qint64 nDataSize, PDSTRUCT *pPdStruct)
{
    // SigBytes format: pairs of {type:quint8, value:quint8}
    // So nSigBytesSize must be even and represent nSigBytesSize/2 bytes to match
    if (!pSigBytes || !pData || (nSigBytesSize <= 0) || (nDataSize < 0) || ((nSigBytesSize % 2) != 0) ||
        !isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    qint64 nPatternLength = nSigBytesSize / 2;

    // Data must be at least as long as the pattern
    if (nDataSize < nPatternLength) {
        return false;
    }

#ifdef USE_XSIMD
    // Try SIMD path first for better performance
    qint32 nResult = xsimd_compare_sigbytes((const quint8 *)pSigBytes, (qint64)nSigBytesSize, (const quint8 *)pData, (qint64)nDataSize, (const quint8 *)g_alphaNumTable);

    if (nResult >= 0) {
        // SIMD was able to process it
        return (nResult == 1);
    }
    // If nResult < 0, fall back to scalar implementation below
#endif

    const quint8 *pSig = (const quint8 *)pSigBytes;
    const quint8 *pSigEnd = pSig + nSigBytesSize;
    const quint8 *pDat = (const quint8 *)pData;

    // Check cancellation less frequently for better performance
    qint64 nCheckStride = 64;
    qint64 nNextCheck = nCheckStride;

    while (pSig < pSigEnd) {
        quint8 nType = *pSig++;
        quint8 nValue = *pSig++;
        quint8 nDataByte = *pDat++;

        // Most common cases first (HEX and WILDCARD)
        if (nType == SIGBYTETYPE_HEX) {
            if (nDataByte != nValue) {
                return false;
            }
        } else if (nType == SIGBYTETYPE_WILDCARD) {
            // Matches any byte, continue
        } else if (nType == SIGBYTE_NOT_NULL) {
            if (nDataByte == 0x00) {
                return false;
            }
        } else if (nType == SIGBYTETYPE_ANSI) {
            // Printable ASCII: 0x20 (space) to 0x7E (~)
            // Optimized: single comparison using unsigned arithmetic
            if ((quint8)(nDataByte - 0x20) >= 0x5F) {
                return false;
            }
        } else if (nType == SIGBYTETYPE_NOT_ANSI) {
            // Not printable ASCII
            if ((quint8)(nDataByte - 0x20) < 0x5F) {
                return false;
            }
        } else if (nType == SIGBYTETYPE_ANSI_ALPHANUMERIC) {
            // Use lookup table for fast classification
            if (!g_alphaNumTable[nDataByte]) {
                return false;
            }
        } else if (nType == SIGBYTETYPE_NOT_ANSI_AND_NOT_NULL) {
            // Not printable ASCII and not null
            // Optimized: combine checks
            if (nDataByte == 0x00 || (quint8)(nDataByte - 0x20) < 0x5F) {
                return false;
            }
        } else {
            // Unknown type, fail
            return false;
        }

        // Check for cancellation periodically
        if ((pSig >= pSigEnd) || ((pSig - (const quint8 *)pSigBytes) >= nNextCheck)) {
            if (!isPdStructNotCanceled(pPdStruct)) {
                return false;
            }
            nNextCheck += nCheckStride;
        }
    }

    return true;
}

qint64 XBinary::_findSigBytes(qint64 nOffset, qint64 nSize, const char *pSigBytes, qint64 nSigBytesSize, PDSTRUCT *pPdStruct)
{
    // Validate inputs
    if (!pSigBytes || (nSigBytesSize <= 0) || (nSigBytesSize > 0x200000)) {
        return -1;
    }

    // SigBytes format uses 2 bytes per pattern byte (type + value)
    if ((nSigBytesSize % 2) != 0) {
        return -1;
    }

    qint64 nPatternLength = nSigBytesSize / 2;

    // Validate region
    qint64 nFileSize = getSize();

    if ((nFileSize < 0) || (nOffset < 0) || (nOffset > nFileSize) || (nSize < -1) || !isPdStructNotCanceled(pPdStruct)) {
        return -1;
    }

    if (nSize == -1) {
        nSize = nFileSize - nOffset;
    }

    if ((nSize <= 0) || (nSize > (nFileSize - nOffset))) {
        return -1;
    }

    if (nPatternLength > nSize) {
        return -1;
    }

    // Search through the region
    qint64 nSearchEnd = nOffset + nSize - nPatternLength + 1;

    if (m_pConstMemory) {
        // Fast path: constant memory access
        const char *pFileData = (const char *)m_pConstMemory;

#ifdef USE_XSIMD
        // Try SIMD path first for better performance
        qint64 nResult = xsimd_find_sigbytes((const quint8 *)(pFileData + nOffset), nSize, (const quint8 *)pSigBytes, nSigBytesSize, (const quint8 *)g_alphaNumTable);

        if (nResult >= 0) {
            // SIMD found it, adjust offset
            return nOffset + nResult;
        }
        // If nResult < 0, fall back to scalar implementation below
#endif

        for (qint64 i = nOffset; (i < nSearchEnd) && isPdStructNotCanceled(pPdStruct); i++) {
            if (_compareSigBytes(pSigBytes, nSigBytesSize, pFileData + i, nPatternLength, pPdStruct)) {
                return i;
            }
        }
    } else {
        // Buffered read path
        const qint32 nBufferSize = qMax((qint32)0x10000, (qint32)nPatternLength);
        QByteArray baBuffer;

        for (qint64 i = nOffset; (i < nSearchEnd) && isPdStructNotCanceled(pPdStruct);) {
            // Read buffer at current position
            qint64 nRemaining = nSearchEnd - i;
            qint64 nReadSize = qMin<qint64>(nBufferSize, nRemaining + nPatternLength - 1);

            baBuffer = read_array(i, nReadSize);

            if (baBuffer.isEmpty()) {
                break;
            }

            // Search within buffer
            qint64 nBufferSearchEnd = baBuffer.size() - nPatternLength + 1;

            for (qint64 j = 0; (j < nBufferSearchEnd) && isPdStructNotCanceled(pPdStruct); j++) {
                if (_compareSigBytes(pSigBytes, nSigBytesSize, baBuffer.constData() + j, nPatternLength, pPdStruct)) {
                    return i + j;
                }
            }

            // Move to next buffer position
            // Overlap by (nPatternLength - 1) to catch patterns spanning buffer boundaries
            qint64 nAdvance = baBuffer.size() - (nPatternLength - 1);

            if (nAdvance <= 0) {
                nAdvance = 1;
            }

            i += nAdvance;
        }
    }

    return -1;
}

qint64 XBinary::find_signature(qint64 nOffset, qint64 nSize, const QString &sSignature, qint64 *pnResultSize, PDSTRUCT *pPdStruct)
{
    _MEMORY_MAP memoryMap = XBinary::getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);

    return find_signature(&memoryMap, nOffset, nSize, sSignature, pnResultSize, pPdStruct);
}

qint64 XBinary::find_signature(_MEMORY_MAP *pMemoryMap, qint64 nOffset, qint64 nSize, const QString &sSignature, qint64 *pnResultSize, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    //    bool bDisableSignals=true;

    //    if(pPdStruct->bIsDisable) // If we call find_signature in another
    //    search function
    //    {
    //        bDisableSignals=false; // TODO Check !!!
    //    }

    // TODO CheckSize function
    qint64 _nSize = getSize();

    qint64 nResultSize = 0;

    if (pnResultSize == 0) {
        pnResultSize = &nResultSize;
    }

    *pnResultSize = 0;

    if (!pMemoryMap || (_nSize < 0) || (nOffset < 0) || (nOffset > _nSize) ||
        (nSize < -1) || !isPdStructNotCanceled(pPdStruct)) {
        return -1;
    }

    if (nSize == -1) {
        nSize = _nSize - nOffset;
    }

    if (nSize > (_nSize - nOffset)) {
        nSize = _nSize - nOffset;
    }

    if (nSize <= 0) {
        return -1;
    }

    QString _sSignature = convertSignature(sSignature);

    if (_sSignature.isEmpty() || (_sSignature.size() > 0x200000) || !isSignatureValid(_sSignature, pPdStruct)) {
        return -1;
    }

    if (_sSignature.contains(QChar('$')) || _sSignature.contains(QChar('#')) || _sSignature.contains(QChar('+'))) {
        *pnResultSize = 1;
    } else {
        // Fix size
        *pnResultSize = _sSignature.size() / 2;
    }

    qint64 nResult = -1;

    if (_sSignature.contains(QChar('$')) || _sSignature.contains(QChar('#')) || _sSignature.contains(QChar('+'))) {
        bool bIsValid = true;

        QList<SIGNATURE_RECORD> listSignatureRecords = getSignatureRecords(_sSignature, &bIsValid, pPdStruct);

        if (listSignatureRecords.count()) {
            qint32 _nFreeIndex = XBinary::reservePdStructRecord(pPdStruct, 0);

            qint32 nSearchFirstIndex = 0;
            qint64 nDelta = 0;
            ST stIndex = ST_COMPAREBYTES;
            qint64 nSearchMaxSize = 0;

            if (!_sSignature.contains(QChar('+'))) {
                qint32 nNumberOfRecords = listSignatureRecords.count();
                qint64 nCurrentDelta = 0;
                bool bNewAlgo = false;

                for (qint32 i = 0; i < nNumberOfRecords; i++) {
                    if ((listSignatureRecords.at(i).st == ST_ADDRESS) || (listSignatureRecords.at(i).st == ST_RELOFFSET)) {
                        break;
                    } else if ((listSignatureRecords.at(i).nWindowSize > nSearchMaxSize) &&
                               ((listSignatureRecords.at(i).st == ST_COMPAREBYTES) || (listSignatureRecords.at(i).st == ST_FINDBYTES))) {
                        nSearchMaxSize = listSignatureRecords.at(i).nWindowSize;
                        stIndex = listSignatureRecords.at(i).st;
                        nDelta = nCurrentDelta;
                        nSearchFirstIndex = i;
                    } else if (bNewAlgo && (listSignatureRecords.at(i).nWindowSize > nSearchMaxSize) &&
                               ((listSignatureRecords.at(i).st == ST_NOTNULL) || (listSignatureRecords.at(i).st == ST_ANSI) ||
                                (listSignatureRecords.at(i).st == ST_NOTANSI) || (listSignatureRecords.at(i).st == ST_NOTANSIANDNULL) ||
                                (listSignatureRecords.at(i).st == ST_ANSINUMBER))) {
                        nSearchMaxSize = listSignatureRecords.at(i).nWindowSize;
                        stIndex = listSignatureRecords.at(i).st;
                        nDelta = nCurrentDelta;
                        nSearchFirstIndex = i;
                    }
                    nCurrentDelta += listSignatureRecords.at(i).nWindowSize;
                }
            }

            if (nSearchFirstIndex > 0) {
                if ((nDelta < 0) || (nDelta > nSize) || (nDelta > (_nSize - nOffset))) {
                    XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);
                    return -1;
                }
                qint64 nTmpOffset = nOffset + nDelta;
                qint64 nTmpSize = nSize - nDelta;

                XBinary::setPdStructInit(pPdStruct, _nFreeIndex, nTmpSize);

                QByteArray baData = listSignatureRecords.at(nSearchFirstIndex).baData;

                char *pData = baData.data();
                qint32 nDataSize = baData.size();

                for (qint64 i = 0; (i < nTmpSize) && (!(pPdStruct->bIsStop));) {
                    qint64 nCurrentOffset = -1;

                    if ((stIndex == ST_COMPAREBYTES) || (stIndex == ST_FINDBYTES)) {
                        nCurrentOffset = _find_array(stIndex, nTmpOffset + i, nTmpSize - i, pData, nDataSize, pPdStruct);
                    } else {
                        nCurrentOffset = _find_array(stIndex, nTmpOffset + i, nTmpSize - i, 0, nSearchMaxSize, pPdStruct);
                    }

                    if (nCurrentOffset != -1) {
                        if (_compareSignature(pMemoryMap, &listSignatureRecords, nCurrentOffset - nDelta, pPdStruct)) {
                            nResult = nCurrentOffset - nDelta;

                            break;
                        }
                    } else {
                        break;
                    }

                    i = nCurrentOffset + 1 - nTmpOffset;

                    XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, i);
                }
            } else if ((listSignatureRecords.at(0).st == ST_COMPAREBYTES) || (listSignatureRecords.at(0).st == ST_FINDBYTES) ||
                       (listSignatureRecords.at(0).st == ST_NOTNULL) || (listSignatureRecords.at(0).st == ST_ANSI) || (listSignatureRecords.at(0).st == ST_NOTANSI) ||
                       (listSignatureRecords.at(0).st == ST_NOTANSIANDNULL) || (listSignatureRecords.at(0).st == ST_ANSINUMBER)) {
                ST _st = listSignatureRecords.at(0).st;

                if (listSignatureRecords.at(0).st == ST_FINDBYTES) {
                    _st = ST_COMPAREBYTES;
                }

                XBinary::setPdStructInit(pPdStruct, _nFreeIndex, nSize);

                QByteArray baFirst = listSignatureRecords.at(0).baData;

                char *pData = baFirst.data();
                // For ST_COMPAREBYTES (including remapped ST_FINDBYTES), use the actual baFirst size.
                // For other ST_* modes, use the recorded window size.
                qint32 nDataSize = (_st == ST_COMPAREBYTES) ? baFirst.size() : listSignatureRecords.at(0).nWindowSize;

                for (qint64 i = 0; (i < nSize) && (!(pPdStruct->bIsStop));) {
                    qint64 nTempOffset = _find_array(_st, nOffset + i, nSize - i, pData, nDataSize, pPdStruct);

                    if (nTempOffset != -1) {
                        if (_compareSignature(pMemoryMap, &listSignatureRecords, nTempOffset, pPdStruct)) {
                            nResult = nTempOffset;

                            break;
                        }
                    } else {
                        break;
                    }

                    i = nTempOffset + 1 - nOffset;

                    XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, i);
                }
            } else {
                for (qint64 i = 0; (i < nSize) && (!(pPdStruct->bIsStop)); i++) {
                    if (_compareSignature(pMemoryMap, &listSignatureRecords, nOffset + i, pPdStruct)) {
                        nResult = nOffset + i;
                        break;
                    }

                    XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, i);
                }
            }

            XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);
        }
    } else if (_sSignature.contains(QChar('.')) || _sSignature.contains(QChar('*')) || _sSignature.contains(QChar('%')) || _sSignature.contains(QChar('!')) ||
               _sSignature.contains(QChar('_')) || _sSignature.contains(QChar('&'))) {
        QByteArray baSigBytes = _signatureToSigBytes(_sSignature, pPdStruct);
        if (baSigBytes.size()) {
            // Optimization: Check for leading non-HEX bytes
            // Sigbytes format: each entry is 2 bytes [type, value]
            qint32 nLeadingNonHex = 0;
            qint32 nSigBytesSize = baSigBytes.size();
            const quint8 *pSigBytes = (const quint8 *)baSigBytes.constData();

            // Count leading non-HEX bytes (each entry is 2 bytes: [SIGBYTETYPE, value])
            for (qint32 i = 0; i < nSigBytesSize; i += 2) {
                quint8 nType = pSigBytes[i];
                if (nType != SIGBYTETYPE_HEX) {
                    nLeadingNonHex++;
                } else {
                    break;
                }
            }

            // If we have >=3 leading non-HEX bytes, extract first fixed pattern and search for it
            if (nLeadingNonHex >= 3) {
                // Find first fixed hex byte sequence (at least 3 bytes)
                qint32 nFixedStartIndex = nLeadingNonHex * 2;  // Byte offset in sigbytes
                qint32 nFixedLength = 0;

                for (qint32 i = nFixedStartIndex; i < nSigBytesSize; i += 2) {
                    quint8 nType = pSigBytes[i];
                    if (nType == SIGBYTETYPE_HEX) {
                        nFixedLength++;
                    } else {
                        break;
                    }
                }

                // If fixed pattern is >=3 bytes, use optimized search
                if (nFixedLength >= 3) {
                    // Extract actual hex values (second byte of each [type, value] pair)
                    QByteArray baFixedPattern;
                    baFixedPattern.reserve(nFixedLength);
                    for (qint32 i = 0; i < nFixedLength; i++) {
                        baFixedPattern.append((char)pSigBytes[nFixedStartIndex + (i * 2) + 1]);
                    }

                    qint64 nCurrentOffset = nOffset;
                    qint64 nRemainingSize = nSize;

                    while (isPdStructNotCanceled(pPdStruct)) {
                        // Search for fixed pattern
                        qint64 nFoundOffset = find_array(nCurrentOffset, nRemainingSize, baFixedPattern.constData(), baFixedPattern.size(), pPdStruct);

                        if (nFoundOffset == -1) {
                            break;
                        }

                        // Check if full signature matches at position - nLeadingNonHex
                        qint64 nCheckOffset = nFoundOffset - nLeadingNonHex;

                        if (nCheckOffset >= nOffset) {
                            // Verify the full signature at this position
                            if (compareSignature(pMemoryMap, _sSignature, nCheckOffset, pPdStruct)) {
                                nResult = nCheckOffset;
                                break;
                            }
                        }

                        // Continue searching after this match
                        nCurrentOffset = nFoundOffset + 1;
                        if (nCurrentOffset >= nOffset + nSize) {
                            break;
                        }
                        nRemainingSize = nSize - (nCurrentOffset - nOffset);
                    }
                } else {
                    // Fixed pattern too short, use normal search
                    nResult = _findSigBytes(nOffset, nSize, baSigBytes.constData(), baSigBytes.size(), pPdStruct);
                }
            } else {
                // No leading non-HEX bytes or too few, use normal search
                nResult = _findSigBytes(nOffset, nSize, baSigBytes.constData(), baSigBytes.size(), pPdStruct);
            }
        }

    } else {
        const QByteArray baData = QByteArray::fromHex(_sSignature.toLatin1());

        if (baData.size()) {
            *pnResultSize = baData.size();
            nResult = find_array(nOffset, nSize, baData.data(), baData.size(), pPdStruct);
        }
    }

    return nResult;
}

qint64 XBinary::find_ansiStringI(qint64 nOffset, qint64 nSize, const QString &sString, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    const qint64 nTotalSize = getSize();

    // Enforce the public search-pattern ceiling before duplicating the input.
    if (!isPdStructNotCanceled(pPdStruct) || (nOffset < 0) || (nOffset > nTotalSize) || sString.isEmpty() || (sString.size() > 0x100000)) {
        return -1;
    }

    QByteArray baNeedle;
    try {
        baNeedle = sString.toLatin1();
    } catch (const std::bad_alloc &) {
        return -1;
    }
    const qint32 nStringSize = baNeedle.size();

    if (nSize == -1) {
        nSize = nTotalSize - nOffset;
    }

    if ((nSize < nStringSize) || (nSize > nTotalSize - nOffset)) {
        return -1;
    }

    const qint32 nRequestedBufferSize = getBufferSize(pPdStruct);

    if (nRequestedBufferSize <= 0) {
        return -1;
    }

    qint32 nBufferSize = nRequestedBufferSize;
    if (nBufferSize < 0x1000) {
        nBufferSize = 0x1000;
    } else if (nBufferSize > 0x100000) {
        nBufferSize = 0x100000;
    }

    char *pBuffer = new (std::nothrow) char[nBufferSize];
    if (!pBuffer) {
        return -1;
    }

    qint32 *pFailure = new (std::nothrow) qint32[nStringSize];
    if (!pFailure) {
        delete[] pBuffer;
        return -1;
    }

    const auto foldByte = [](quint8 nValue) -> quint8 {
        const char cFolded = QChar(nValue).toLower().toLatin1();
        return cFolded ? (quint8)cFolded : nValue;
    };

    for (qint32 i = 0; i < nStringSize; i++) {
        baNeedle[i] = (char)foldByte((quint8)baNeedle.at(i));
    }

    pFailure[0] = 0;
    for (qint32 i = 1, j = 0; (i < nStringSize) && isPdStructNotCanceled(pPdStruct); i++) {
        while ((j > 0) && (baNeedle.at(i) != baNeedle.at(j))) {
            j = pFailure[j - 1];
        }
        if (baNeedle.at(i) == baNeedle.at(j)) {
            j++;
        }
        pFailure[i] = j;
    }

    if (!isPdStructNotCanceled(pPdStruct)) {
        delete[] pBuffer;
        delete[] pFailure;
        return -1;
    }

    qint64 nResult = -1;
    const qint64 nStartOffset = nOffset;
    const qint64 nTotalSearchSize = nSize;
    const qint32 nFreeIndex = XBinary::reservePdStructRecord(pPdStruct, nSize);
    qint32 nMatched = 0;

    while ((nSize > 0) && isPdStructNotCanceled(pPdStruct)) {
        qint32 nTemp = nBufferSize;
        if (nSize < nTemp) {
            nTemp = (qint32)nSize;
        }

        if (read_array_process(nOffset, pBuffer, nTemp, pPdStruct) != nTemp) {
            if (isPdStructNotCanceled(pPdStruct)) {
                setPdStructErrorString(pPdStruct, tr("Read error"));
            }
            break;
        }

        for (qint32 i = 0; (i < nTemp) && isPdStructNotCanceled(pPdStruct); i++) {
            const char cValue = (char)foldByte((quint8)pBuffer[i]);

            while ((nMatched > 0) && (cValue != baNeedle.at(nMatched))) {
                nMatched = pFailure[nMatched - 1];
            }
            if (cValue == baNeedle.at(nMatched)) {
                nMatched++;
            }
            if (nMatched == nStringSize) {
                nResult = nOffset + i - nStringSize + 1;
                break;
            }
        }

        if (nResult != -1) {
            break;
        }

        nSize -= nTemp;
        nOffset += nTemp;
        XBinary::setPdStructCurrent(pPdStruct, nFreeIndex, nOffset - nStartOffset);
    }

    XBinary::setPdStructFinished(pPdStruct, nFreeIndex);
    delete[] pBuffer;
    delete[] pFailure;

    if (!isPdStructNotCanceled(pPdStruct) || ((nResult == -1) && (nSize != 0) && ((nOffset - nStartOffset) < nTotalSearchSize))) {
        nResult = -1;
    }

    return nResult;
}

qint64 XBinary::find_unicodeStringI(qint64 nOffset, qint64 nSize, const QString &sString, bool bIsBigEndian, PDSTRUCT *pPdStruct)
{
    qint64 nResult = -1;
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }
    const qint64 nTotalSize = getSize();

    if (!isPdStructNotCanceled(pPdStruct) || (nOffset < 0) || (nOffset > nTotalSize) || sString.isEmpty() || (sString.size() > 0x80000)) {
        return -1;
    }

    if (nSize == -1) {
        nSize = nTotalSize - nOffset;
    }

    if ((nSize < 2) || (nSize > nTotalSize - nOffset)) {
        return -1;
    }

    const qint32 nRequestedBufferSize = getBufferSize(pPdStruct);
    if (nRequestedBufferSize <= 0) {
        return -1;
    }

    qint32 nBufferSize = nRequestedBufferSize;
    if (nBufferSize < 0x1000) {
        nBufferSize = 0x1000;
    } else if (nBufferSize > 0x100000) {
        nBufferSize = 0x100000;
    }

    const qint32 nPatternCapacity = sString.size() * 2;
    char *pBuffer = new (std::nothrow) char[nBufferSize];
    if (!pBuffer) {
        return -1;
    }

    quint16 *pPattern = new (std::nothrow) quint16[nPatternCapacity];
    if (!pPattern) {
        delete[] pBuffer;
        return -1;
    }

    qint32 *pFailure = new (std::nothrow) qint32[nPatternCapacity];
    if (!pFailure) {
        delete[] pBuffer;
        delete[] pPattern;
        return -1;
    }

    qint32 nPatternSize = 0;
    for (qint32 i = 0; (i < sString.size()) && isPdStructNotCanceled(pPdStruct); i++) {
        const quint16 nFirstUnit = sString.at(i).unicode();
        uint nCodePoint = nFirstUnit;

        if (QChar::isHighSurrogate(nFirstUnit) && ((i + 1) < sString.size()) && QChar::isLowSurrogate(sString.at(i + 1).unicode())) {
            nCodePoint = QChar::surrogateToUcs4(nFirstUnit, sString.at(++i).unicode());
        }

        uint nFoldedCodePoint = QChar::toCaseFolded(nCodePoint);
        if (nFoldedCodePoint > 0x10FFFF) {
            nFoldedCodePoint = nCodePoint;
        }

        if (nFoldedCodePoint > 0xFFFF) {
            pPattern[nPatternSize++] = QChar::highSurrogate(nFoldedCodePoint);
            pPattern[nPatternSize++] = QChar::lowSurrogate(nFoldedCodePoint);
        } else {
            pPattern[nPatternSize++] = (quint16)nFoldedCodePoint;
        }
    }

    pFailure[0] = 0;
    for (qint32 i = 1, j = 0; (i < nPatternSize) && isPdStructNotCanceled(pPdStruct); i++) {
        while ((j > 0) && (pPattern[i] != pPattern[j])) {
            j = pFailure[j - 1];
        }
        if (pPattern[i] == pPattern[j]) {
            j++;
        }
        pFailure[i] = j;
    }

    if (!isPdStructNotCanceled(pPdStruct)) {
        delete[] pBuffer;
        delete[] pPattern;
        delete[] pFailure;
        return -1;
    }

    qint64 *pSourceOffsets = new (std::nothrow) qint64[(size_t)nPatternSize * 2];
    if (!pSourceOffsets) {
        delete[] pBuffer;
        delete[] pPattern;
        delete[] pFailure;
        return -1;
    }

    const qint64 nStartOffset = nOffset;
    const qint32 nFreeIndex = XBinary::reservePdStructRecord(pPdStruct, nSize);
    qint32 nMatched[2] = {0, 0};
    qint32 nRingPosition[2] = {0, 0};
    bool bPendingHighSurrogate[2] = {false, false};
    quint16 nPendingHighSurrogate[2] = {0, 0};
    qint64 nPendingHighOffset[2] = {0, 0};
    bool bHavePreviousByte = false;
    quint8 nPreviousByte = 0;

    const auto feedFoldedUnit = [&](qint32 nLane, quint16 nFoldedUnit, qint64 nSourceOffset) -> bool {
        pSourceOffsets[(qint64)nLane * nPatternSize + nRingPosition[nLane]] = nSourceOffset;
        nRingPosition[nLane]++;
        if (nRingPosition[nLane] == nPatternSize) {
            nRingPosition[nLane] = 0;
        }

        while ((nMatched[nLane] > 0) && (nFoldedUnit != pPattern[nMatched[nLane]])) {
            nMatched[nLane] = pFailure[nMatched[nLane] - 1];
        }
        if (nFoldedUnit == pPattern[nMatched[nLane]]) {
            nMatched[nLane]++;
        }
        if (nMatched[nLane] == nPatternSize) {
            nResult = pSourceOffsets[(qint64)nLane * nPatternSize + nRingPosition[nLane]];
            return true;
        }

        return false;
    };

    const auto feedCodePoint = [&](qint32 nLane, uint nCodePoint, qint64 nSourceOffset) -> bool {
        uint nFoldedCodePoint = QChar::toCaseFolded(nCodePoint);
        if (nFoldedCodePoint > 0x10FFFF) {
            nFoldedCodePoint = nCodePoint;
        }

        if (nFoldedCodePoint > 0xFFFF) {
            return feedFoldedUnit(nLane, QChar::highSurrogate(nFoldedCodePoint), nSourceOffset) ||
                   feedFoldedUnit(nLane, QChar::lowSurrogate(nFoldedCodePoint), nSourceOffset);
        }

        return feedFoldedUnit(nLane, (quint16)nFoldedCodePoint, nSourceOffset);
    };

    while ((nSize > 0) && isPdStructNotCanceled(pPdStruct)) {
        qint32 nTemp = nBufferSize;
        if (nSize < nTemp) {
            nTemp = (qint32)nSize;
        }

        if (read_array_process(nOffset, pBuffer, nTemp, pPdStruct) != nTemp) {
            if (isPdStructNotCanceled(pPdStruct)) {
                setPdStructErrorString(pPdStruct, tr("Read error"));
            }
            break;
        }

        for (qint32 i = 0; (i < nTemp) && isPdStructNotCanceled(pPdStruct); i++) {
            const quint8 nCurrentByte = (quint8)pBuffer[i];

            if (bHavePreviousByte) {
                const qint64 nPairOffset = nOffset + i - 1;
                const qint32 nLane = (qint32)((nPairOffset - nStartOffset) & 1);
                const quint16 nCode = bIsBigEndian ? (quint16)(((quint16)nPreviousByte << 8) | nCurrentByte)
                                                   : (quint16)(nPreviousByte | ((quint16)nCurrentByte << 8));

                if (bPendingHighSurrogate[nLane]) {
                    if (QChar::isLowSurrogate(nCode)) {
                        const uint nCodePoint = QChar::surrogateToUcs4(nPendingHighSurrogate[nLane], nCode);
                        bPendingHighSurrogate[nLane] = false;
                        if (feedCodePoint(nLane, nCodePoint, nPendingHighOffset[nLane])) {
                            break;
                        }
                        nPreviousByte = nCurrentByte;
                        bHavePreviousByte = true;
                        continue;
                    }

                    bPendingHighSurrogate[nLane] = false;
                    if (feedCodePoint(nLane, nPendingHighSurrogate[nLane], nPendingHighOffset[nLane])) {
                        break;
                    }
                }

                if (QChar::isHighSurrogate(nCode)) {
                    bPendingHighSurrogate[nLane] = true;
                    nPendingHighSurrogate[nLane] = nCode;
                    nPendingHighOffset[nLane] = nPairOffset;
                } else if (feedCodePoint(nLane, nCode, nPairOffset)) {
                    break;
                }
            }

            nPreviousByte = nCurrentByte;
            bHavePreviousByte = true;
        }

        if (nResult != -1) {
            break;
        }

        nSize -= nTemp;
        nOffset += nTemp;

        XBinary::setPdStructCurrent(pPdStruct, nFreeIndex, nOffset - nStartOffset);
    }

    if ((nResult == -1) && (nSize == 0) && isPdStructNotCanceled(pPdStruct)) {
        qint64 nEarliestPendingResult = -1;

        for (qint32 nLane = 0; nLane < 2; nLane++) {
            if (bPendingHighSurrogate[nLane]) {
                feedCodePoint(nLane, nPendingHighSurrogate[nLane], nPendingHighOffset[nLane]);

                if ((nResult != -1) && ((nEarliestPendingResult == -1) || (nResult < nEarliestPendingResult))) {
                    nEarliestPendingResult = nResult;
                }
                nResult = -1;
            }
        }

        nResult = nEarliestPendingResult;
    }

    XBinary::setPdStructFinished(pPdStruct, nFreeIndex);

    delete[] pBuffer;
    delete[] pPattern;
    delete[] pFailure;
    delete[] pSourceOffsets;

    if (!isPdStructNotCanceled(pPdStruct)) {
        nResult = -1;
    }

    return nResult;
}

qint64 XBinary::find_utf8StringI(qint64 nOffset, qint64 nSize, const QString &sString, PDSTRUCT *pPdStruct)
{
    // TODO !!!
    // Double
    return find_utf8String(nOffset, nSize, sString, pPdStruct);
}

quint8 XBinary::getBits_uint8(quint8 nValue, qint32 nBitOffset, qint32 nBitSize)
{
    if (nBitSize == 0) return nValue;
    if ((nBitOffset < 0) || (nBitSize < 0) || (nBitOffset >= 8)) return 0;
    nBitSize = qMin(nBitSize, 8 - nBitOffset);
    const quint8 nMask = (nBitSize == 8) ? 0xFF : (quint8)((quint16(1) << nBitSize) - 1);
    return (quint8)((nValue >> nBitOffset) & nMask);
}

quint16 XBinary::getBits_uint16(quint16 nValue, qint32 nBitOffset, qint32 nBitSize)
{
    if (nBitSize == 0) return nValue;
    if ((nBitOffset < 0) || (nBitSize < 0) || (nBitOffset >= 16)) return 0;
    nBitSize = qMin(nBitSize, 16 - nBitOffset);
    const quint16 nMask = (nBitSize == 16) ? 0xFFFF : (quint16)((quint32(1) << nBitSize) - 1);
    return (quint16)((nValue >> nBitOffset) & nMask);
}

quint32 XBinary::getBits_uint32(quint32 nValue, qint32 nBitOffset, qint32 nBitSize)
{
    if (nBitSize == 0) return nValue;
    if ((nBitOffset < 0) || (nBitSize < 0) || (nBitOffset >= 32)) return 0;
    nBitSize = qMin(nBitSize, 32 - nBitOffset);
    const quint32 nMask = (nBitSize == 32) ? 0xFFFFFFFFU : (quint32)((quint64(1) << nBitSize) - 1);
    return (nValue >> nBitOffset) & nMask;
}

quint64 XBinary::getBits_uint64(quint64 nValue, qint32 nBitOffset, qint32 nBitSize)
{
    if (nBitSize == 0) return nValue;
    if ((nBitOffset < 0) || (nBitSize < 0) || (nBitOffset >= 64)) return 0;
    nBitSize = qMin(nBitSize, 64 - nBitOffset);
    const quint64 nMask = (nBitSize == 64) ? (std::numeric_limits<quint64>::max)() : ((quint64(1) << nBitSize) - 1);
    return (nValue >> nBitOffset) & nMask;
}

bool XBinary::_addMultiSearchStringRecord(QVector<MS_RECORD> *pList, MS_RECORD *pRecord, QString sString, STRINGSEARCH_OPTIONS *pSsOptions)
{
    bool bResult = false;

    if (!pList || !pRecord || !pSsOptions) {
        return bResult;
    }

    bool bAdd = true;

    if (pSsOptions->bLinks) {
        bAdd = sString.contains(QLatin1String("http://"), Qt::CaseInsensitive) ||
               sString.contains(QLatin1String("https://"), Qt::CaseInsensitive) ||
               sString.contains(QLatin1String("www."), Qt::CaseInsensitive) ||
               sString.contains(QLatin1String("mailto:"), Qt::CaseInsensitive);
    }

    if (bAdd && (pSsOptions->sMask != "")) {
        bAdd = isRegExpPresent(pSsOptions->sMask, sString);
    }

    if (bAdd) {
        pRecord->sValue = sString;
        pList->append(*pRecord);

        bResult = true;
    }

    return bResult;
}

// Optimized version that accepts pre-compiled regex to avoid creating QRegularExpression repeatedly
bool XBinary::_addMultiSearchStringRecordOptimized(QVector<MS_RECORD> *pList, MS_RECORD *pRecord, const QString &sString, STRINGSEARCH_OPTIONS *pSsOptions,
                                                    QRegularExpression *pRegex)
{
    if (!pList || !pRecord || !pSsOptions) {
        return false;
    }

    bool bAdd = true;

    if (pSsOptions->bLinks) {
        bAdd = sString.contains(QLatin1String("http://"), Qt::CaseInsensitive) ||
               sString.contains(QLatin1String("https://"), Qt::CaseInsensitive) ||
               sString.contains(QLatin1String("www."), Qt::CaseInsensitive) ||
               sString.contains(QLatin1String("mailto:"), Qt::CaseInsensitive);
    }

    if (bAdd && pRegex) {
        // Use pre-compiled regex - MUCH faster than creating new QRegularExpression each time
        QRegularExpressionMatch match = pRegex->match(sString);
        bAdd = match.hasMatch();
    }

    if (bAdd) {
        pRecord->sValue = sString;
        pList->append(*pRecord);
        return true;
    }

    return false;
}

QVector<XBinary::MS_RECORD> XBinary::multiSearch_allStrings(_MEMORY_MAP *pMemoryMap, qint64 nOffset, qint64 nSize, STRINGSEARCH_OPTIONS ssOptions, PDSTRUCT *pPdStruct)
{
    // Keep one canonical implementation of combined string searching.  The
    // specialized scanners have independent ANSI/UTF-16 size bounds and the
    // union preserves identical filtering and record-value semantics.
    return multiSearch_allStrings2(pMemoryMap, nOffset, nSize, ssOptions, pPdStruct);
#if 0
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    OFFSETSIZE osRegion = convertOffsetAndSize(nOffset, nSize);

    nOffset = osRegion.nOffset;
    nSize = osRegion.nSize;

    QVector<XBinary::MS_RECORD> listResult;

    if (ssOptions.nMinLenght == 0) {
        ssOptions.nMinLenght = 1;
    }

    if (ssOptions.nMaxLenght == 0) {
        ssOptions.nMaxLenght = 128;  // TODO Check
    }

    if ((nOffset == -1) || (nSize <= 0) || !isPdStructNotCanceled(pPdStruct)) {
        return listResult;
    }

    const qint64 nMaximumChars = ssOptions.bUnicode ? (qint64)(0xFFFF / 2) : (qint64)0xFFFF;
    ssOptions.nMaxLenght = qMin(ssOptions.nMaxLenght, nMaximumChars);

    if ((ssOptions.nMinLenght < 1) || (ssOptions.nMinLenght > ssOptions.nMaxLenght)) {
        return listResult;
    }

    if (ssOptions.nLimit <= 0) {
        ssOptions.nLimit = (std::numeric_limits<qint32>::max)();
    }

    if (ssOptions.bUnicode && (!ssOptions.bAnsi)) {
        return multiSearch_unicodeStrings(pMemoryMap, nOffset, nSize, ssOptions, pPdStruct);
    }

    qint64 _nSize = nSize;
    qint64 _nOffset = nOffset;
    qint64 _nRawOffset = 0;

    bool bReadError = false;

    const qint32 nRequestedBufferSize = getBufferSize(pPdStruct);

    if ((nRequestedBufferSize <= 0) || !isPdStructNotCanceled(pPdStruct)) {
        return listResult;
    }

    qint32 nBufferSize = nRequestedBufferSize;
    if (nBufferSize < 0x1000) {
        nBufferSize = 0x1000;
    } else if (nBufferSize > 0x100000) {
        nBufferSize = 0x100000;
    }

    const qint32 nStringCapacity = (qint32)ssOptions.nMaxLenght + 1;
    char *pBuffer = new (std::nothrow) char[nBufferSize];
    if (!pBuffer) {
        return listResult;
    }

    char *pAnsiBuffer = new (std::nothrow) char[nStringCapacity];
    if (!pAnsiBuffer) {
        delete[] pBuffer;
        return listResult;
    }

    quint16 *pUnicodeBuffer[2] = {nullptr, nullptr};
    pUnicodeBuffer[0] = new (std::nothrow) quint16[nStringCapacity];
    if (!pUnicodeBuffer[0]) {
        delete[] pBuffer;
        delete[] pAnsiBuffer;
        return listResult;
    }

    pUnicodeBuffer[1] = new (std::nothrow) quint16[nStringCapacity];
    if (!pUnicodeBuffer[1]) {
        delete[] pBuffer;
        delete[] pAnsiBuffer;
        delete[] pUnicodeBuffer[0];
        return listResult;
    }
    qint64 nCurrentUnicodeSize[2] = {0, 0};
    qint64 nCurrentUnicodeOffset[2] = {0, 0};

    qint64 nCurrentAnsiSize = 0;
    qint64 nCurrentAnsiOffset = 0;

    bool bIsStart = true;
    char cPrevSymbol = 0;

    qint32 _nFreeIndex = XBinary::reservePdStructRecord(pPdStruct, nSize);

    bool bLimitReached = false;

    while ((_nSize > 0) && (!(pPdStruct->bIsStop))) {
        qint64 nCurrentSize = _nSize;

        nCurrentSize = qMin((qint64)nBufferSize, nCurrentSize);

        if (read_array_process(_nOffset, pBuffer, nCurrentSize, pPdStruct) != nCurrentSize) {
            bReadError = true;
            break;
        }

        for (qint64 i = 0; (i < nCurrentSize) && isPdStructNotCanceled(pPdStruct); i++) {
            bool bIsEnd = ((i == (nCurrentSize - 1)) && (_nSize == nCurrentSize));
            qint32 nParity = (_nOffset + i) % 2;

            char cSymbol = *(pBuffer + i);

            bool bIsAnsiSymbol = false;

            if (ssOptions.bAnsi) {
                bIsAnsiSymbol = isAnsiSymbol((quint8)cSymbol);
            }

            if (bIsAnsiSymbol) {
                if (nCurrentAnsiSize == 0) {
                    nCurrentAnsiOffset = _nOffset + i;
                }

                if (nCurrentAnsiSize < ssOptions.nMaxLenght) {
                    *(pAnsiBuffer + nCurrentAnsiSize) = cSymbol;
                    nCurrentAnsiSize++;
                }
            }

            if (((!bIsAnsiSymbol) || bIsEnd) && (nCurrentAnsiSize > 0)) {
                if (nCurrentAnsiSize >= ssOptions.nMinLenght) {
                    pAnsiBuffer[nCurrentAnsiSize] = 0;

                    if (ssOptions.bAnsi) {
                        const QString sString = QString::fromLatin1(pAnsiBuffer, (qint32)nCurrentAnsiSize);
                        const bool bAdd = !ssOptions.bNullTerminated || ((!bIsAnsiSymbol) && (cSymbol == 0));

                        if (bAdd) {
                            MS_RECORD record = {};
                            record.nValueType = VT_A;
                            _setMSRecordSize(&record, nCurrentAnsiSize, VT_A, true);
                            _setMSRecordLocation(&record, pMemoryMap, nCurrentAnsiOffset);

                            if (_addMultiSearchStringRecord(&listResult, &record, sString, &ssOptions)) {
                                bLimitReached = _retainBestMSRecords(&listResult, ssOptions.nLimit) || bLimitReached;
                            }
                        }
                    }
                }

                nCurrentAnsiSize = 0;
            }

            if (!bIsStart) {
                quint16 nCode = (quint8)cPrevSymbol | ((quint16)(quint8)cSymbol << 8);  // TODO BE/LE

                const bool bIsUnicodeSymbol = ssOptions.bUnicode && isUnicodeSymbol(nCode, true);

                if (bIsUnicodeSymbol) {
                    if (nCurrentUnicodeSize[nParity] == 0) {
                        nCurrentUnicodeOffset[nParity] = _nOffset - 1 + i;
                    }

                    if (nCurrentUnicodeSize[nParity] < ssOptions.nMaxLenght) {
                        *(pUnicodeBuffer[nParity] + nCurrentUnicodeSize[nParity]) = nCode;
                        nCurrentUnicodeSize[nParity]++;
                    }
                }

                if (((!bIsUnicodeSymbol) || bIsEnd) && (nCurrentUnicodeSize[nParity] > 0)) {
                    if (nCurrentUnicodeSize[nParity] >= ssOptions.nMinLenght) {
                        pUnicodeBuffer[nParity][nCurrentUnicodeSize[nParity]] = 0;

                        if (ssOptions.bUnicode) {
                            const QString sString = QString::fromUtf16(pUnicodeBuffer[nParity], (qint32)nCurrentUnicodeSize[nParity]);  // TODO Check Qt6
                            const bool bAdd = !ssOptions.bNullTerminated || ((!bIsUnicodeSymbol) && (nCode == 0));

                            if (bAdd) {
                                MS_RECORD record = {};
                                record.nValueType = VT_U;
                                _setMSRecordSize(&record, nCurrentUnicodeSize[nParity] * 2, VT_U, true);
                                _setMSRecordLocation(&record, pMemoryMap, nCurrentUnicodeOffset[nParity]);

                                if (_addMultiSearchStringRecord(&listResult, &record, sString, &ssOptions)) {
                                    bLimitReached = _retainBestMSRecords(&listResult, ssOptions.nLimit) || bLimitReached;
                                }
                            }
                        }
                    }

                    nCurrentUnicodeSize[nParity] = 0;
                }

                if (bIsEnd) {
                    const qint32 nO = (nParity == 1) ? 0 : 1;

                    if ((!ssOptions.bNullTerminated) && (nCurrentUnicodeSize[nO] >= ssOptions.nMinLenght)) {
                        pUnicodeBuffer[nO][nCurrentUnicodeSize[nO]] = 0;

                        if (ssOptions.bUnicode) {
                            const QString sString = QString::fromUtf16(pUnicodeBuffer[nO], (qint32)nCurrentUnicodeSize[nO]);  // TODO Check Qt6

                            MS_RECORD record = {};
                            record.nValueType = VT_U;
                            _setMSRecordSize(&record, nCurrentUnicodeSize[nO] * 2, VT_U, true);
                            _setMSRecordLocation(&record, pMemoryMap, nCurrentUnicodeOffset[nO]);

                            if (_addMultiSearchStringRecord(&listResult, &record, sString, &ssOptions)) {
                                bLimitReached = _retainBestMSRecords(&listResult, ssOptions.nLimit) || bLimitReached;
                            }
                        }
                    }

                    nCurrentUnicodeSize[nO] = 0;
                }
            }

            cPrevSymbol = cSymbol;

            if (bIsStart) {
                bIsStart = false;
            }
        }

        _nSize -= nCurrentSize;
        _nOffset += nCurrentSize;
        _nRawOffset += nCurrentSize;

        XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, _nOffset - nOffset);

    }

    if (bReadError) {
        setPdStructErrorString(pPdStruct, tr("Read error"));
    }

    delete[] pBuffer;
    delete[] pAnsiBuffer;

    delete[] pUnicodeBuffer[0];
    delete[] pUnicodeBuffer[1];

    _finalizeBestMSRecords(&listResult, ssOptions.nLimit);

    if (bLimitReached && !bReadError) {
        setPdStructErrorString(pPdStruct, QString("%1: %2").arg(tr("Maximum")).arg(QString::number(ssOptions.nLimit)));
    }

    XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);

    if (bReadError || !isPdStructNotCanceled(pPdStruct)) {
        listResult.clear();
    }

    return listResult;
#endif
}

QVector<XBinary::MS_RECORD> XBinary::multiSearch_allStrings2(_MEMORY_MAP *pMemoryMap, qint64 nOffset, qint64 nSize, STRINGSEARCH_OPTIONS ssOptions, PDSTRUCT *pPdStruct)
{
    // Optimized implementation: Use specialized functions and combine results
    QVector<XBinary::MS_RECORD> listResult;

    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }
    const QSharedPointer<PDSTRUCT_CALLBACK_STATE> pProgressLifetime = pPdStruct->_pdCallbackState;

    const qint32 nEffectiveLimit =
        (ssOptions.nLimit > 0) ? ssOptions.nLimit : (std::numeric_limits<qint32>::max)();

    const QString sInitialError = getPdStructErrorString(pPdStruct);
    bool bLimitReached = false;

    // Search for ANSI strings if flag is set
    if (ssOptions.bAnsi) {
        // A caller may reuse a PDSTRUCT whose error text already equals the
        // error produced by this pass.  Comparing the text before/after would
        // then mistake a fresh read failure for stale state and publish the
        // other pass's partial results.  Give every pass an isolated status
        // slot and restore the caller's original status only after the whole
        // combined operation succeeds.
        clearPdStructErrorString(pPdStruct);

        STRINGSEARCH_OPTIONS ansiOptions = ssOptions;
        ansiOptions.bUnicode = false;  // ANSI only
        ansiOptions.bAnsi = true;
        ansiOptions.nLimit = nEffectiveLimit;

        QVector<XBinary::MS_RECORD> listAnsi = multiSearch_ansiStrings(pMemoryMap, nOffset, nSize, ansiOptions, pPdStruct);
        if (!isPdCallbackOwnerAlive(pProgressLifetime)) {
            listResult.clear();
            return listResult;
        }
        listResult.append(listAnsi);

        const QString sAnsiError = getPdStructErrorString(pPdStruct);
        const bool bAnsiLimitReached = sAnsiError.startsWith(tr("Maximum"));
        if (!isPdStructNotCanceled(pPdStruct) || ((!sAnsiError.isEmpty()) && !bAnsiLimitReached)) {
            listResult.clear();
            return listResult;
        }

        bLimitReached = bAnsiLimitReached;
    }

    // Each enabled encoding gets the full limit. The best N results of the
    // union must be present in the best N results of each individual search.
    if (ssOptions.bUnicode) {
        clearPdStructErrorString(pPdStruct);

        STRINGSEARCH_OPTIONS unicodeOptions = ssOptions;
        unicodeOptions.bAnsi = false;  // Unicode only
        unicodeOptions.bUnicode = true;
        unicodeOptions.nLimit = nEffectiveLimit;

        QVector<XBinary::MS_RECORD> listUnicode = multiSearch_unicodeStrings(pMemoryMap, nOffset, nSize, unicodeOptions, pPdStruct);
        if (!isPdCallbackOwnerAlive(pProgressLifetime)) {
            listResult.clear();
            return listResult;
        }
        listResult.append(listUnicode);

        const QString sUnicodeError = getPdStructErrorString(pPdStruct);
        const bool bUnicodeLimitReached = sUnicodeError.startsWith(tr("Maximum"));
        if (!isPdStructNotCanceled(pPdStruct) || ((!sUnicodeError.isEmpty()) && !bUnicodeLimitReached)) {
            listResult.clear();
            return listResult;
        }


        bLimitReached = bUnicodeLimitReached || bLimitReached;
    }

    // Sort results by region index first, then by relative offset within region
    if (listResult.size() > 1) {
        std::sort(listResult.begin(), listResult.end(), compareMS_RECORD);
    }

    // Enforce final limit if needed
    if (listResult.size() > nEffectiveLimit) {
        listResult.resize(nEffectiveLimit);
        bLimitReached = true;
    }

    if (bLimitReached) {
        setPdStructErrorString(pPdStruct, QString("%1: %2").arg(tr("Maximum")).arg(QString::number(nEffectiveLimit)));
    } else {
        setPdStructErrorString(pPdStruct, sInitialError);
    }

    return listResult;
}

QVector<XBinary::MS_RECORD> XBinary::multiSearch_ansiStrings(_MEMORY_MAP *pMemoryMap, qint64 nOffset, qint64 nSize, STRINGSEARCH_OPTIONS ssOptions, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }
    const QSharedPointer<PDSTRUCT_CALLBACK_STATE> pProgressLifetime = pPdStruct->_pdCallbackState;

    OFFSETSIZE osRegion = convertOffsetAndSize(nOffset, nSize);

    nOffset = osRegion.nOffset;
    nSize = osRegion.nSize;

    QVector<MS_RECORD> listResult;

    if (ssOptions.nMinLenght == 0) {
        ssOptions.nMinLenght = 1;
    }

    if (ssOptions.nMaxLenght == 0) {
        ssOptions.nMaxLenght = 128;
    }

    if ((nOffset == -1) || (nSize <= 0)) {
        return listResult;
    }

    ssOptions.nMaxLenght = qMin(ssOptions.nMaxLenght, (qint64)0xFFFF);

    if ((ssOptions.nMinLenght < 1) || (ssOptions.nMinLenght > ssOptions.nMaxLenght)) {
        return listResult;
    }

    if (ssOptions.nLimit <= 0) {
        ssOptions.nLimit = (std::numeric_limits<qint32>::max)();
    }

    qint64 _nOffset = nOffset;
    qint64 _nSize = nSize;

    bool bReadError = false;
    bool bProgressOwnerAlive = true;

    const qint32 BUFFER_SIZE = 0x10000;  // 64KB chunks for efficient processing

    if (!isPdStructNotCanceled(pPdStruct)) {
        return listResult;
    }

    char *pBuffer = new (std::nothrow) char[BUFFER_SIZE];
    if (!pBuffer) {
        setPdStructErrorString(pPdStruct, tr("Allocation error"));
        return listResult;
    }

    const qint32 nStringCapacity = (qint32)ssOptions.nMaxLenght + 1;
    char *pAnsiBuffer = new (std::nothrow) char[nStringCapacity];
    if (!pAnsiBuffer) {
        delete[] pBuffer;
        setPdStructErrorString(pPdStruct, tr("Allocation error"));
        return listResult;
    }

    qint64 nCurrentAnsiSize = 0;
    qint64 nCurrentAnsiOffset = 0;

    qint32 _nFreeIndex = XBinary::reservePdStructRecord(pPdStruct, nSize);

    bool bLimitReached = false;

    // OPTIMIZATION: Pre-compile regex once instead of creating it for every string!
    QRegularExpression regex;
    QRegularExpression *pRegex = nullptr;
    if (ssOptions.sMask != "") {
        regex = QRegularExpression(ssOptions.sMask);
        pRegex = &regex;
        // Optimize regex for repeated matching
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        pRegex->optimize();
#endif
    }

    const bool bNeedStringFilter = ssOptions.bLinks || (pRegex != nullptr);

    while ((_nSize > 0) && (!(pPdStruct->bIsStop))) {
        qint64 nCurrentSize = qMin((qint64)BUFFER_SIZE, _nSize);

        if (read_array_process(_nOffset, pBuffer, nCurrentSize, pPdStruct) != nCurrentSize) {
            bReadError = true;
            break;
        }

#if 0  // The bounded scalar state machine below is used for identical semantics in every build.
        // SIMD-optimized: aggressively use xsimd for maximum performance
        // Process entire buffer with SIMD acceleration
        qint64 i = 0;

        while ((i < nCurrentSize) && isPdStructNotCanceled(pPdStruct)) {
            bool bIsEnd = ((i == (nCurrentSize - 1)) && (_nSize == nCurrentSize));
            char cSymbol = *(pBuffer + i);
            bool bIsAnsiSymbol = isAnsiSymbol((quint8)cSymbol);

            if (bIsAnsiSymbol) {
                if (nCurrentAnsiSize == 0) {
                    nCurrentAnsiOffset = _nOffset + i;

                    // Use SIMD to find the end of the ANSI string quickly
                    qint64 nRemaining = nCurrentSize - i;
                    qint64 nMaxScan = qMin(nRemaining, ssOptions.nMaxLenght);

                    if (nMaxScan >= nSimdThreshold) {
                        // Use xsimd to count consecutive ANSI characters
                        qint64 nAnsiRun = xsimd_count_ansi_prefix(pBuffer + i, nMaxScan);

                        if (nAnsiRun >= nSimdThreshold) {
                            // Found substantial ANSI run - bulk copy with memcpy
                            qint64 nCopySize = qMin(nAnsiRun, ssOptions.nMaxLenght);
                            memcpy(pAnsiBuffer, pBuffer + i, nCopySize);
                            nCurrentAnsiSize = nCopySize;
                            i += nCopySize;

                            // Check if string actually ended or just reached buffer boundary
                            bool bStringEnded = false;
                            if (i < nCurrentSize) {
                                // We're still within buffer - check if next char is non-ANSI
                                if (!isAnsiSymbol((quint8)pBuffer[i])) {
                                    bStringEnded = true;
                                }
                            } else {
                                // Hit buffer boundary - string may continue in next buffer
                                // Only end string if this is the absolute end of all data
                                if (_nSize == nCurrentSize) {
                                    bStringEnded = true;
                                }
                                // Otherwise string continues in next buffer - preserve state
                            }

                            if (bStringEnded) {
                                // String ended - process it
                                bool bLongString = (nCurrentAnsiSize >= ssOptions.nMaxLenght);

                                if (nCurrentAnsiSize >= ssOptions.nMinLenght) {
                                    pAnsiBuffer[nCurrentAnsiSize] = 0;
                                    bool bAdd = true;

                                    if (ssOptions.bNullTerminated && i < nCurrentSize && pBuffer[i] && (!bLongString)) {
                                        bAdd = false;
                                    }

                                    if (bAdd) {
                                        MS_RECORD record = {};
                                        record.nValueType = VT_A;
                                        _setMSRecordSize(&record, nCurrentAnsiSize, VT_A, true);
                                        _setMSRecordLocation(&record, pMemoryMap, nCurrentAnsiOffset);

                                        if (!bNeedStringFilter) {
                                            listResult.append(record);
                                            bLimitReached = _retainBestMSRecords(&listResult, ssOptions.nLimit) || bLimitReached;
                                        } else {
                                            QString sString = pAnsiBuffer;
                                            if (_addMultiSearchStringRecordOptimized(&listResult, &record, sString, &ssOptions, pRegex)) {
                                                bLimitReached = _retainBestMSRecords(&listResult, ssOptions.nLimit) || bLimitReached;
                                            }
                                        }
                                    }
                                }

                                nCurrentAnsiSize = 0;
                                continue;
                            }

                            // String continues beyond SIMD scan - continue byte-by-byte
                            continue;
                        }
                    }
                }

                // Single character handling (short strings or continuation)
                if (nCurrentAnsiSize < ssOptions.nMaxLenght) {
                    *(pAnsiBuffer + nCurrentAnsiSize) = cSymbol;
                    nCurrentAnsiSize++;
                    i++;
                } else {
                    // String too long
                    if (nCurrentAnsiSize >= ssOptions.nMinLenght) {
                        pAnsiBuffer[ssOptions.nMaxLenght] = 0;

                        MS_RECORD record = {};
                        record.nValueType = VT_A;
                        _setMSRecordSize(&record, ssOptions.nMaxLenght, VT_A, true);
                        _setMSRecordLocation(&record, pMemoryMap, nCurrentAnsiOffset);

                        if (!bNeedStringFilter) {
                            record.sValue = QString::fromLatin1(
                                pAnsiBuffer, (qint32)nCurrentAnsiSize);
                            listResult.append(record);
                            bLimitReached = _retainBestMSRecords(&listResult, ssOptions.nLimit) || bLimitReached;
                        } else {
                            QString sString = pAnsiBuffer;
                            if (_addMultiSearchStringRecordOptimized(&listResult, &record, sString, &ssOptions, pRegex)) {
                                bLimitReached = _retainBestMSRecords(&listResult, ssOptions.nLimit) || bLimitReached;
                            }
                        }
                    }

                    nCurrentAnsiSize = 0;
                    i++;
                }
            } else {
                // Non-ANSI character - terminate current string if any
                if (nCurrentAnsiSize >= ssOptions.nMinLenght) {
                    pAnsiBuffer[nCurrentAnsiSize] = 0;
                    bool bAdd = true;

                    if (ssOptions.bNullTerminated && cSymbol) {
                        bAdd = false;
                    }

                    if (bAdd) {
                        MS_RECORD record = {};
                        record.nValueType = VT_A;
                        _setMSRecordSize(&record, nCurrentAnsiSize, VT_A, true);
                        _setMSRecordLocation(&record, pMemoryMap, nCurrentAnsiOffset);

                        if (!bNeedStringFilter) {
                            record.sValue = QString::fromLatin1(pAnsiBuffer, (qint32)nCurrentAnsiSize);
                            listResult.append(record);
                            bLimitReached = _retainBestMSRecords(&listResult, ssOptions.nLimit) || bLimitReached;
                        } else {
                            QString sString = pAnsiBuffer;
                            if (_addMultiSearchStringRecordOptimized(&listResult, &record, sString, &ssOptions, pRegex)) {
                                bLimitReached = _retainBestMSRecords(&listResult, ssOptions.nLimit) || bLimitReached;
                            }
                        }
                    }
                }

                nCurrentAnsiSize = 0;
                i++;
            }

            // Handle end of buffer - DON'T terminate partial strings at buffer boundary
            // Only save strings if we're at the actual end of data OR if string is complete
            if (bIsEnd && nCurrentAnsiSize >= ssOptions.nMinLenght) {
                // Only save if this is truly the end of all data (not just buffer boundary)
                if (_nSize == nCurrentSize) {
                    pAnsiBuffer[nCurrentAnsiSize] = 0;

                    MS_RECORD record = {};
                    record.nValueType = VT_A;
                    _setMSRecordSize(&record, nCurrentAnsiSize, VT_A, true);
                    _setMSRecordLocation(&record, pMemoryMap, nCurrentAnsiOffset);

                    if (!bNeedStringFilter) {
                        listResult.append(record);
                        bLimitReached = _retainBestMSRecords(&listResult, ssOptions.nLimit) || bLimitReached;
                    } else {
                        QString sString = pAnsiBuffer;
                        if (_addMultiSearchStringRecordOptimized(&listResult, &record, sString, &ssOptions, pRegex)) {
                            bLimitReached = _retainBestMSRecords(&listResult, ssOptions.nLimit) || bLimitReached;
                        }
                    }

                    nCurrentAnsiSize = 0;
                }
                // Otherwise, partial string continues in next buffer - preserve state
            }
        }
#else
        // Keep one capped record for an overlong run and consume the rest of
        // that run before finalizing it. This prevents split records and lost
        // bytes while keeping MS_RECORD::nSize within its configured maximum.
        for (qint64 i = 0; (i < nCurrentSize) && isPdStructNotCanceled(pPdStruct); i++) {
            const bool bIsEnd = ((i == (nCurrentSize - 1)) && (_nSize == nCurrentSize));
            const char cSymbol = *(pBuffer + i);
            const bool bIsAnsiSymbol = isAnsiSymbol((quint8)cSymbol);

            if (bIsAnsiSymbol) {
                if (nCurrentAnsiSize == 0) {
                    nCurrentAnsiOffset = _nOffset + i;
                }

                if (nCurrentAnsiSize < ssOptions.nMaxLenght) {
                    *(pAnsiBuffer + nCurrentAnsiSize) = cSymbol;
                    nCurrentAnsiSize++;
                }
            }

            if (((!bIsAnsiSymbol) || bIsEnd) && (nCurrentAnsiSize > 0)) {
                if (nCurrentAnsiSize >= ssOptions.nMinLenght) {
                    pAnsiBuffer[nCurrentAnsiSize] = 0;
                    const bool bAdd = !ssOptions.bNullTerminated || ((!bIsAnsiSymbol) && (cSymbol == 0));

                    if (bAdd) {
                        MS_RECORD record = {};
                        record.nValueType = VT_A;
                        _setMSRecordSize(&record, nCurrentAnsiSize, VT_A, true);
                        _setMSRecordLocation(&record, pMemoryMap, nCurrentAnsiOffset);

                        if (!bNeedStringFilter) {
                            record.sValue = QString::fromLatin1(
                                pAnsiBuffer, (qint32)nCurrentAnsiSize);
                            listResult.append(record);
                            bLimitReached = _retainBestMSRecords(&listResult, ssOptions.nLimit) || bLimitReached;
                        } else {
                            const QString sString = QString::fromLatin1(pAnsiBuffer, (qint32)nCurrentAnsiSize);
                            if (_addMultiSearchStringRecordOptimized(&listResult, &record, sString, &ssOptions, pRegex)) {
                                bLimitReached = _retainBestMSRecords(&listResult, ssOptions.nLimit) || bLimitReached;
                            }
                        }
                    }
                }

                nCurrentAnsiSize = 0;
            }
        }
#endif

        _nSize -= nCurrentSize;
        _nOffset += nCurrentSize;

        XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, _nOffset - nOffset);
        if (!isPdCallbackOwnerAlive(pProgressLifetime)) {
            bProgressOwnerAlive = false;
            break;
        }

    }

    if (bProgressOwnerAlive && bReadError) {
        setPdStructErrorString(pPdStruct, tr("Read error"));
    }

    delete[] pBuffer;
    delete[] pAnsiBuffer;

    if (!bProgressOwnerAlive) {
        listResult.clear();
        return listResult;
    }

    _finalizeBestMSRecords(&listResult, ssOptions.nLimit);

    if (bLimitReached && !bReadError) {
        setPdStructErrorString(pPdStruct, QString("%1: %2").arg(tr("Maximum")).arg(QString::number(ssOptions.nLimit)));
    }

    XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);

    if (bReadError || !isPdStructNotCanceled(pPdStruct)) {
        listResult.clear();
    }

    return listResult;
}

QVector<XBinary::MS_RECORD> XBinary::multiSearch_unicodeStrings(_MEMORY_MAP *pMemoryMap, qint64 nOffset, qint64 nSize, STRINGSEARCH_OPTIONS ssOptions,
                                                                PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }
    const QSharedPointer<PDSTRUCT_CALLBACK_STATE> pProgressLifetime = pPdStruct->_pdCallbackState;

    OFFSETSIZE osRegion = convertOffsetAndSize(nOffset, nSize);

    nOffset = osRegion.nOffset;
    nSize = osRegion.nSize;

    QVector<MS_RECORD> listResult;

    if (ssOptions.nMinLenght == 0) {
        ssOptions.nMinLenght = 1;
    }

    if (ssOptions.nMaxLenght == 0) {
        ssOptions.nMaxLenght = 128;
    }

    if ((nOffset == -1) || (nSize <= 0)) {
        return listResult;
    }

    ssOptions.nMaxLenght = qMin(ssOptions.nMaxLenght, (qint64)(0xFFFF / 2));

    if ((ssOptions.nMinLenght < 1) || (ssOptions.nMinLenght > ssOptions.nMaxLenght)) {
        return listResult;
    }

    if (ssOptions.nLimit <= 0) {
        ssOptions.nLimit = (std::numeric_limits<qint32>::max)();
    }

    qint64 _nOffset = nOffset;
    qint64 _nSize = nSize;

    bool bReadError = false;
    bool bProgressOwnerAlive = true;

    const qint32 BUFFER_SIZE = 0x10000;  // 64KB chunks for efficient processing

    if (!isPdStructNotCanceled(pPdStruct)) {
        return listResult;
    }

    char *pBuffer = new (std::nothrow) char[BUFFER_SIZE];
    quint16 *pUnicodeBuffer[2];  // Two buffers for even/odd parity
    pUnicodeBuffer[0] = new (std::nothrow) quint16[ssOptions.nMaxLenght + 1];
    pUnicodeBuffer[1] = new (std::nothrow) quint16[ssOptions.nMaxLenght + 1];

    if (!pBuffer || !pUnicodeBuffer[0] || !pUnicodeBuffer[1]) {
        delete[] pBuffer;
        delete[] pUnicodeBuffer[0];
        delete[] pUnicodeBuffer[1];
        setPdStructErrorString(pPdStruct, tr("Allocation error"));
        return listResult;
    }

    qint64 nCurrentUnicodeSize[2] = {0, 0};  // Track both parities
    qint64 nCurrentUnicodeOffset[2] = {0, 0};
    char cPrevSymbol = 0;  // For building 16-bit Unicode characters

    qint32 _nFreeIndex = XBinary::reservePdStructRecord(pPdStruct, nSize);

    bool bLimitReached = false;

    // OPTIMIZATION: Pre-compile regex once instead of creating it for every string!
    QRegularExpression regex;
    QRegularExpression *pRegex = nullptr;
    if (ssOptions.sMask != "") {
        regex = QRegularExpression(ssOptions.sMask);
        pRegex = &regex;
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        pRegex->optimize();
#endif
    }

    const bool bNeedStringFilter = ssOptions.bLinks || (pRegex != nullptr);
    bool bIsStart = true;  // Track if we're at the start of processing

    while ((_nSize > 0) && (!(pPdStruct->bIsStop))) {
        qint64 nCurrentSize = qMin((qint64)BUFFER_SIZE, _nSize);

        if (read_array_process(_nOffset, pBuffer, nCurrentSize, pPdStruct) != nCurrentSize) {
            bReadError = true;
            break;
        }

        // Process buffer byte-by-byte to build Unicode (UTF-16) characters
        // Unicode strings are 16-bit (2 bytes per character)
        for (qint64 i = 0; (i < nCurrentSize) && isPdStructNotCanceled(pPdStruct); i++) {
            char cSymbol = *(pBuffer + i);
            bool bIsEnd = ((i == (nCurrentSize - 1)) && (_nSize == nCurrentSize));
            qint32 nParity = (_nOffset + i) % 2;  // Track even/odd byte position

            if (!bIsStart) {
                // Build 16-bit Unicode character (little-endian by default)
                quint16 nCode = (quint8)cPrevSymbol + ((quint8)cSymbol << 8);

                // Check if this is a valid Unicode character.
                const bool bIsUnicodeSymbol = isUnicodeSymbol(nCode, true);

                if (bIsUnicodeSymbol) {
                    if (nCurrentUnicodeSize[nParity] == 0) {
                        // Start of new Unicode string (offset points to first byte of first char)
                        nCurrentUnicodeOffset[nParity] = _nOffset + i - 1;
                    }

                    if (nCurrentUnicodeSize[nParity] < ssOptions.nMaxLenght) {
                        *(pUnicodeBuffer[nParity] + nCurrentUnicodeSize[nParity]) = nCode;
                        nCurrentUnicodeSize[nParity]++;
                    }
                }

                // Check if string ended
                if (((!bIsUnicodeSymbol) || bIsEnd) && (nCurrentUnicodeSize[nParity] > 0)) {
                    if (nCurrentUnicodeSize[nParity] >= ssOptions.nMinLenght) {
                        pUnicodeBuffer[nParity][nCurrentUnicodeSize[nParity]] = 0;
                        const bool bAdd = !ssOptions.bNullTerminated || ((!bIsUnicodeSymbol) && (nCode == 0));

                        if (bAdd) {
                            MS_RECORD record = {};
                            record.nValueType = VT_U;
                            _setMSRecordSize(&record, nCurrentUnicodeSize[nParity] * 2, VT_U, true);
                            _setMSRecordLocation(&record, pMemoryMap, nCurrentUnicodeOffset[nParity]);

                            if (!bNeedStringFilter) {
                                record.sValue = QString::fromUtf16(
                                    pUnicodeBuffer[nParity], (qint32)nCurrentUnicodeSize[nParity]);
                                listResult.append(record);
                                bLimitReached = _retainBestMSRecords(&listResult, ssOptions.nLimit) || bLimitReached;
                            } else {
                                const QString sString = QString::fromUtf16(pUnicodeBuffer[nParity], (qint32)nCurrentUnicodeSize[nParity]);
                                if (_addMultiSearchStringRecordOptimized(&listResult, &record, sString, &ssOptions, pRegex)) {
                                    bLimitReached = _retainBestMSRecords(&listResult, ssOptions.nLimit) || bLimitReached;
                                }
                            }
                        }
                    }

                    nCurrentUnicodeSize[nParity] = 0;
                }

                // Handle the other parity at end of file. It cannot be
                // considered null-terminated because no complete delimiter
                // code unit was observed for that lane.
                if (bIsEnd) {
                    const qint32 nOtherParity = (nParity == 1) ? 0 : 1;

                    if ((!ssOptions.bNullTerminated) && (nCurrentUnicodeSize[nOtherParity] >= ssOptions.nMinLenght)) {
                        pUnicodeBuffer[nOtherParity][nCurrentUnicodeSize[nOtherParity]] = 0;

                        MS_RECORD record = {};
                        record.nValueType = VT_U;
                        _setMSRecordSize(&record, nCurrentUnicodeSize[nOtherParity] * 2, VT_U, true);
                        _setMSRecordLocation(&record, pMemoryMap, nCurrentUnicodeOffset[nOtherParity]);

                        if (!bNeedStringFilter) {
                            record.sValue = QString::fromUtf16(
                                pUnicodeBuffer[nOtherParity], (qint32)nCurrentUnicodeSize[nOtherParity]);
                            listResult.append(record);
                            bLimitReached = _retainBestMSRecords(&listResult, ssOptions.nLimit) || bLimitReached;
                        } else {
                            const QString sString =
                                QString::fromUtf16(pUnicodeBuffer[nOtherParity], (qint32)nCurrentUnicodeSize[nOtherParity]);
                            if (_addMultiSearchStringRecordOptimized(&listResult, &record, sString, &ssOptions, pRegex)) {
                                bLimitReached = _retainBestMSRecords(&listResult, ssOptions.nLimit) || bLimitReached;
                            }
                        }
                    }

                    nCurrentUnicodeSize[nOtherParity] = 0;
                }
            }

            cPrevSymbol = cSymbol;
            bIsStart = false;
        }

        _nSize -= nCurrentSize;
        _nOffset += nCurrentSize;

        XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, _nOffset - nOffset);
        if (!isPdCallbackOwnerAlive(pProgressLifetime)) {
            bProgressOwnerAlive = false;
            break;
        }

    }

    if (bProgressOwnerAlive && bReadError) {
        setPdStructErrorString(pPdStruct, tr("Read error"));
    }

    delete[] pBuffer;
    delete[] pUnicodeBuffer[0];
    delete[] pUnicodeBuffer[1];

    if (!bProgressOwnerAlive) {
        listResult.clear();
        return listResult;
    }

    _finalizeBestMSRecords(&listResult, ssOptions.nLimit);

    if (bLimitReached && !bReadError) {
        setPdStructErrorString(pPdStruct, QString("%1: %2").arg(tr("Maximum")).arg(QString::number(ssOptions.nLimit)));
    }

    XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);

    if (bReadError || !isPdStructNotCanceled(pPdStruct)) {
        listResult.clear();
    }

    return listResult;
}

QVector<XBinary::MS_RECORD> XBinary::multiSearch_signature(qint64 nOffset, qint64 nSize, qint32 nLimit, const QString &sSignature, quint32 nInfo, PDSTRUCT *pPdStruct)
{
    _MEMORY_MAP memoryMap = getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);

    return multiSearch_signature(&memoryMap, nOffset, nSize, nLimit, sSignature, nInfo, pPdStruct);
}

QVector<XBinary::MS_RECORD> XBinary::multiSearch_signature(_MEMORY_MAP *pMemoryMap, qint64 nOffset, qint64 nSize, qint32 nLimit, const QString &sSignature, quint32 nInfo,
                                                           PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    QVector<XBinary::MS_RECORD> listResult;

    const qint64 nFileSize = getSize();
    if (!pMemoryMap || !isPdStructNotCanceled(pPdStruct) || (nFileSize < 0) || (nOffset < 0) || (nOffset > nFileSize) || (nSize < -1)) {
        return listResult;
    }
    if (nSize == -1) {
        nSize = nFileSize - nOffset;
    }
    if ((nSize <= 0) || (nSize > (nFileSize - nOffset))) {
        return listResult;
    }

    const qint64 nEndOffset = nOffset + nSize;

    qint64 _nSize = nSize;
    qint64 _nOffset = nOffset;

    qint32 _nFreeIndex = XBinary::reservePdStructRecord(pPdStruct, nSize);

    qint32 nCurrentRecords = 0;

    while ((_nSize > 0) && isPdStructNotCanceled(pPdStruct)) {
        qint64 nSignatureSize = 0;
        qint64 nSignatureOffset = find_signature(pMemoryMap, _nOffset, _nSize, sSignature, &nSignatureSize, pPdStruct);

        if (nSignatureOffset == -1) {
            break;
        }
        if ((nSignatureOffset < _nOffset) || (nSignatureOffset > nEndOffset) || (nSignatureSize <= 0) ||
            (nSignatureSize > (nEndOffset - nSignatureOffset))) {
            setPdStructErrorString(pPdStruct, tr("Invalid search result"));
            break;
        }

        MS_RECORD record = {};
        record.nValueType = VT_SIGNATURE;
        record.nInfo = nInfo;

        if (!_setMSRecordSize(&record, nSignatureSize, VT_SIGNATURE, false)) {
            setPdStructErrorString(pPdStruct, tr("Unrepresentable result size"));
            break;
        }

        _setMSRecordLocation(&record, pMemoryMap, nSignatureOffset);

        listResult.append(record);

        nCurrentRecords++;

        if ((nLimit > 0) && (nCurrentRecords >= nLimit)) {
            setPdStructErrorString(pPdStruct, QString("%1: %2").arg(tr("Maximum")).arg(QString::number(nCurrentRecords)));

            break;
        }

        _nOffset = nSignatureOffset + nSignatureSize;
        _nSize = nSize - (_nOffset - nOffset);

        XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, _nOffset - nOffset);
    }

    XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);

    if (!isPdStructNotCanceled(pPdStruct)) {
        listResult.clear();
    }

    return listResult;
}

QVector<XBinary::MS_RECORD> XBinary::multiSearch_value(qint64 nOffset, qint64 nSize, qint32 nLimit, QVariant varValue, VT valueType, bool bIsBigEndian,
                                                       PDSTRUCT *pPdStruct)
{
    _MEMORY_MAP memoryMap = getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);

    return multiSearch_value(&memoryMap, nOffset, nSize, nLimit, varValue, valueType, bIsBigEndian, pPdStruct);
}

QVector<XBinary::MS_RECORD> XBinary::multiSearch_value(_MEMORY_MAP *pMemoryMap, qint64 nOffset, qint64 nSize, qint32 nLimit, QVariant varValue, VT valueType,
                                                       bool bIsBigEndian, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    // QString sValuePrefix = valueTypeToString(valueType);
    // QString sValue = getValueString(varValue, valueType);
    qint64 nValSize = getValueSize(varValue, valueType);

    QVector<XBinary::MS_RECORD> listResult;

    const qint64 nFileSize = getSize();
    if (!pMemoryMap || !isPdStructNotCanceled(pPdStruct) || (nFileSize < 0) || (nOffset < 0) || (nOffset > nFileSize) || (nSize < -1) || (nValSize <= 0)) {
        return listResult;
    }
    if (nSize == -1) {
        nSize = nFileSize - nOffset;
    }
    if ((nSize <= 0) || (nSize > (nFileSize - nOffset))) {
        return listResult;
    }

    const qint64 nEndOffset = nOffset + nSize;

    qint64 _nSize = nSize;
    qint64 _nOffset = nOffset;

    qint32 _nFreeIndex = XBinary::reservePdStructRecord(pPdStruct, nSize);

    qint32 nCurrentRecords = 0;

    while ((_nSize > 0) && isPdStructNotCanceled(pPdStruct)) {
        qint64 nValOffset = find_value(pMemoryMap, _nOffset, _nSize, varValue, valueType, bIsBigEndian, &nValSize, pPdStruct);

        if (nValOffset == -1) {
            break;
        }
        if ((nValOffset < _nOffset) || (nValOffset > nEndOffset) || (nValSize <= 0) || (nValSize > (nEndOffset - nValOffset))) {
            setPdStructErrorString(pPdStruct, tr("Invalid search result"));
            break;
        }

        // QString _sValue;

        MS_RECORD record = {};
        record.nValueType = valueType;

        if (!_setMSRecordSize(&record, nValSize, valueType, false)) {
            setPdStructErrorString(pPdStruct, tr("Unrepresentable result size"));
            break;
        }

        _setMSRecordLocation(&record, pMemoryMap, nValOffset);

        // if (valueType == VT_ANSISTRING_I) {
        //     _sValue = read_ansiString(nValOffset, nValSize);
        // } else if (valueType == VT_UNICODESTRING_I) {
        //     _sValue = read_unicodeString(nValOffset, nValSize / 2, bIsBigEndian);
        // } else if (valueType == VT_UTF8STRING_I) {
        //     _sValue = read_unicodeString(nValOffset, nValSize, bIsBigEndian);
        // } else if (valueType == VT_SIGNATURE) {
        //     _sValue = getSignature(nValOffset, nValSize);
        // } else {
        //     _sValue = sValue;
        // }

        // record.sString = QString("%1: %2").arg(sValuePrefix, _sValue);

        listResult.append(record);

        nCurrentRecords++;

        if ((nLimit > 0) && (nCurrentRecords >= nLimit)) {
            setPdStructErrorString(pPdStruct, QString("%1: %2").arg(tr("Maximum")).arg(QString::number(nCurrentRecords)));

            break;
        }

        _nOffset = nValOffset + nValSize;
        _nSize = nSize - (_nOffset - nOffset);

        XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, _nOffset - nOffset);
    }

    XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);

    if (!isPdStructNotCanceled(pPdStruct)) {
        listResult.clear();
    }

    return listResult;
}

qint64 XBinary::find_value(_MEMORY_MAP *pMemoryMap, qint64 nOffset, qint64 nSize, QVariant varValue, VT valueType, bool bIsBigEndian, qint64 *pnResultSize,
                           PDSTRUCT *pPdStruct)
{
    qint64 nResult = -1;

    // TODO more mb pascal strings
    if (valueType == XBinary::VT_A) {
        nResult = find_ansiString(nOffset, nSize, varValue.toString(), pPdStruct);
    } else if (valueType == XBinary::VT_A_I) {
        nResult = find_ansiStringI(nOffset, nSize, varValue.toString(), pPdStruct);
    } else if (valueType == XBinary::VT_U) {
        nResult = find_unicodeString(nOffset, nSize, varValue.toString(), bIsBigEndian, pPdStruct);
    } else if (valueType == XBinary::VT_U_I) {
        nResult = find_unicodeStringI(nOffset, nSize, varValue.toString(), bIsBigEndian, pPdStruct);
    } else if (valueType == XBinary::VT_UTF8) {
        nResult = find_utf8String(nOffset, nSize, varValue.toString(), pPdStruct);
    } else if (valueType == XBinary::VT_UTF8_I) {
        nResult = find_utf8StringI(nOffset, nSize, varValue.toString(), pPdStruct);
    } else if (valueType == XBinary::VT_SIGNATURE) {
        nResult = find_signature(pMemoryMap, nOffset, nSize, varValue.toString(), pnResultSize, pPdStruct);
    } else if (valueType == XBinary::VT_BYTE) {
        nResult = find_uint8(nOffset, nSize, (quint8)(varValue.toULongLong()), pPdStruct);
    } else if (valueType == XBinary::VT_WORD) {
        nResult = find_uint16(nOffset, nSize, (quint16)(varValue.toULongLong()), bIsBigEndian, pPdStruct);
    } else if (valueType == XBinary::VT_DWORD) {
        nResult = find_uint32(nOffset, nSize, (quint32)(varValue.toULongLong()), bIsBigEndian, pPdStruct);
    } else if (valueType == XBinary::VT_QWORD) {
        nResult = find_uint64(nOffset, nSize, (quint64)(varValue.toULongLong()), bIsBigEndian, pPdStruct);
    } else if (valueType == XBinary::VT_CHAR) {
        nResult = find_int8(nOffset, nSize, (qint8)(varValue.toLongLong()), pPdStruct);
    } else if (valueType == XBinary::VT_UCHAR) {
        nResult = find_uint8(nOffset, nSize, (quint8)(varValue.toULongLong()), pPdStruct);
    } else if (valueType == XBinary::VT_SHORT) {
        nResult = find_int16(nOffset, nSize, (qint16)(varValue.toLongLong()), bIsBigEndian, pPdStruct);
    } else if ((valueType == XBinary::VT_USHORT) || (valueType == XBinary::VT_UINT16)) {
        nResult = find_uint16(nOffset, nSize, (quint16)(varValue.toULongLong()), bIsBigEndian, pPdStruct);
    } else if (valueType == XBinary::VT_INT) {
        nResult = find_int32(nOffset, nSize, (qint32)(varValue.toLongLong()), bIsBigEndian, pPdStruct);
    } else if ((valueType == XBinary::VT_UINT) || (valueType == XBinary::VT_UINT32)) {
        nResult = find_uint32(nOffset, nSize, (quint32)(varValue.toULongLong()), bIsBigEndian, pPdStruct);
    } else if (valueType == XBinary::VT_INT64) {
        nResult = find_int64(nOffset, nSize, (qint64)(varValue.toLongLong()), bIsBigEndian, pPdStruct);
    } else if (valueType == XBinary::VT_UINT64) {
        nResult = find_uint64(nOffset, nSize, (quint64)(varValue.toULongLong()), bIsBigEndian, pPdStruct);
    } else if (valueType == XBinary::VT_FLOAT) {
        nResult = find_float(nOffset, nSize, (float)(varValue.toFloat()), bIsBigEndian, pPdStruct);
    } else if (valueType == XBinary::VT_DOUBLE) {
        nResult = find_double(nOffset, nSize, (double)(varValue.toDouble()), bIsBigEndian, pPdStruct);
    } else {
#ifdef QT_DEBUG
        qDebug() << "Unknown valueType" << valueTypeToString(valueType, 1);
#endif
    }

    return nResult;
}

QVariant XBinary::read_value(VT valueType, qint64 nOffset, qint64 nSize, bool bIsBigEndian, PDSTRUCT *pPdStruct)
{
    QVariant varResult;

    nSize = qMin(nSize, qint64(128));

    if ((valueType == XBinary::VT_UINT8) || (valueType == XBinary::VT_BYTE)) {
        varResult = read_uint8(nOffset);
    } else if (valueType == XBinary::VT_INT8) {
        varResult = read_int8(nOffset);
    } else if ((valueType == XBinary::VT_UINT16) || (valueType == XBinary::VT_WORD) || (valueType == XBinary::VT_USHORT)) {
        varResult = read_uint16(nOffset, bIsBigEndian);
    } else if ((valueType == XBinary::VT_INT16) || (valueType == XBinary::VT_SHORT)) {
        varResult = read_int16(nOffset, bIsBigEndian);
    } else if ((valueType == XBinary::VT_UINT32) || (valueType == XBinary::VT_UINT) || (valueType == XBinary::VT_DWORD)) {
        varResult = read_uint32(nOffset, bIsBigEndian);
    } else if ((valueType == XBinary::VT_INT32) || (valueType == XBinary::VT_INT)) {
        varResult = read_int32(nOffset, bIsBigEndian);
    } else if ((valueType == XBinary::VT_UINT64) || (valueType == XBinary::VT_QWORD)) {
        varResult = read_uint64(nOffset, bIsBigEndian);
    } else if (valueType == XBinary::VT_INT64) {
        varResult = read_int64(nOffset, bIsBigEndian);
    } else if ((valueType == XBinary::VT_A) || (valueType == XBinary::VT_A_I)) {
        varResult = read_ansiString(nOffset, nSize);
    } else if ((valueType == XBinary::VT_UTF8) || (valueType == XBinary::VT_UTF8_I)) {
        varResult = read_utf8String(nOffset, nSize);
    } else if (valueType == XBinary::VT_UTF32) {
        varResult = read_utf32String(nOffset, nSize, bIsBigEndian);
    } else if ((valueType == XBinary::VT_U) || (valueType == XBinary::VT_U_I)) {
        varResult = read_unicodeString(nOffset, nSize, bIsBigEndian);
    } else if (valueType == XBinary::VT_PACKEDNUMBER) {
        varResult = read_packedNumber(nOffset, nSize).nValue;
    } else if (valueType == XBinary::VT_ULEB128) {
        varResult = read_uleb128(nOffset, nSize).nValue;
    } else if (valueType == XBinary::VT_BYTE_ARRAY) {
        if (nSize <= 256) {
            varResult = read_array_process(nOffset, nSize, pPdStruct);
        }
    } else if (valueType == XBinary::VT_WORD_ARRAY) {
        if (nSize <= 256) {
            varResult = read_array_process(nOffset, nSize, pPdStruct);
        }
    } else if (valueType == XBinary::VT_DWORD_ARRAY) {
        if (nSize <= 256) {
            varResult = read_array_process(nOffset, nSize, pPdStruct);  // TODO
        }
    } else if (valueType == XBinary::VT_CHAR_ARRAY) {
        if (nSize <= 256) {
            varResult = read_ansiString(nOffset, nSize);
        }
    } else {
#ifdef QT_DEBUG
        qDebug() << "Unknown valueType" << valueTypeToString(valueType, nSize);
#endif
    }

    // #ifdef QT_DEBUG
    //         qDebug() << "varResult" << varResult.toString();
    // #endif

    return varResult;
}

QString XBinary::valueTypeToString(VT valueType, qint32 nSize)
{
    QString sResult;
    VT _valueType = valueType;
    bool bArray = false;

    if ((valueType == VT_CHAR_ARRAY) || (valueType == VT_BYTE_ARRAY) || (valueType == VT_WORD_ARRAY) || (valueType == VT_DWORD_ARRAY)) {
        bArray = true;

        if (valueType == VT_CHAR_ARRAY) {
            _valueType = VT_CHAR;
        } else if (valueType == VT_BYTE_ARRAY) {
            _valueType = VT_BYTE;
        } else if (valueType == VT_WORD_ARRAY) {
            _valueType = VT_WORD;
        } else if (valueType == VT_DWORD_ARRAY) {
            _valueType = VT_DWORD;
        }
    }

    sResult = XIDSTRING_idToString((quint32)_valueType, _TABLE_XBinary_VT, sizeof(_TABLE_XBinary_VT) / sizeof(XBinary::XIDSTRING));

    if (bArray) {
        qint32 nCount = nSize / getBaseValueSize(_valueType);

        sResult += QString("[%1]").arg(nCount);
    }

    return sResult;
}

QString XBinary::getValueString(QVariant varValue, VT valueType, bool bTypesAsHex)
{
    QString sResult;

    if (valueType == XBinary::VT_A) {
        sResult = varValue.toString();
    } else if (valueType == XBinary::VT_A_I) {
        sResult = varValue.toString();
    } else if (valueType == XBinary::VT_U) {
        sResult = varValue.toString();
    } else if (valueType == XBinary::VT_U_I) {
        sResult = varValue.toString();
    } else if (valueType == XBinary::VT_UTF8) {
        sResult = varValue.toString();
    } else if (valueType == XBinary::VT_UTF8_I) {
        sResult = varValue.toString();
    } else if (valueType == XBinary::VT_UTF32) {
        sResult = varValue.toString();
    } else if (valueType == XBinary::VT_SIGNATURE) {
        sResult = varValue.toString();
    } else if (valueType == XBinary::VT_BYTE) {
        sResult = valueToHex((quint8)(varValue.toULongLong()));
    } else if (valueType == XBinary::VT_WORD) {
        sResult = valueToHex((quint16)(varValue.toULongLong()));
    } else if (valueType == XBinary::VT_DWORD) {
        sResult = valueToHex((quint32)(varValue.toULongLong()));
    } else if (valueType == XBinary::VT_QWORD) {
        sResult = valueToHex((quint64)(varValue.toULongLong()));
    } else if ((valueType == XBinary::VT_CHAR) || (valueType == XBinary::VT_INT8)) {
        sResult = QString("%1").arg((qint8)(varValue.toLongLong()));
    } else if ((valueType == XBinary::VT_UCHAR) || (valueType == XBinary::VT_UINT8)) {
        sResult = QString("%1").arg((quint8)(varValue.toULongLong()));
    } else if ((valueType == XBinary::VT_SHORT) || (valueType == XBinary::VT_INT16)) {
        sResult = QString("%1").arg((qint16)(varValue.toLongLong()));
    } else if ((valueType == XBinary::VT_USHORT) || (valueType == XBinary::VT_UINT16)) {
        if (bTypesAsHex) {
            sResult = valueToHex((quint16)(varValue.toULongLong()));
        } else {
            sResult = QString("%1").arg((quint16)(varValue.toULongLong()));
        }
    } else if ((valueType == XBinary::VT_INT) || (valueType == XBinary::VT_INT32)) {
        sResult = QString("%1").arg((qint32)(varValue.toLongLong()));
    } else if ((valueType == XBinary::VT_UINT) || (valueType == XBinary::VT_UINT32)) {
        if (bTypesAsHex) {
            sResult = valueToHex((quint32)(varValue.toULongLong()));
        } else {
            sResult = QString("%1").arg((quint32)(varValue.toULongLong()));
        }
    } else if (valueType == XBinary::VT_INT64) {
        sResult = QString("%1").arg((qint64)(varValue.toLongLong()));
    } else if (valueType == XBinary::VT_UINT64) {
        sResult = QString("%1").arg((quint64)(varValue.toULongLong()));
    } else if (valueType == XBinary::VT_PACKEDNUMBER) {
        sResult = QString("%1").arg((quint64)(varValue.toULongLong()));
    } else if (valueType == XBinary::VT_ULEB128) {
        sResult = QString("%1").arg((quint64)(varValue.toULongLong()));
    } else if (valueType == XBinary::VT_FLOAT) {
        sResult = QString("%1").arg(varValue.toFloat());
    } else if (valueType == XBinary::VT_DOUBLE) {
        sResult = QString("%1").arg(varValue.toDouble());
    } else if (valueType == XBinary::VT_CHAR_ARRAY) {
        sResult = varValue.toString();
    } else if (valueType == XBinary::VT_BYTE_ARRAY) {
        sResult = varValue.toByteArray().toHex();
    } else {
#ifdef QT_DEBUG
        qDebug() << "Unknown valueType" << valueTypeToString(valueType, 1);
#endif
    }

    return sResult;
}

qint32 XBinary::getValueSize(QVariant varValue, VT valueType)
{
    qint32 nResult = 1;

    if (valueType == XBinary::VT_A) {
        nResult = varValue.toString().size();
    } else if (valueType == XBinary::VT_A_I) {
        nResult = varValue.toString().size();
    } else if (valueType == XBinary::VT_U) {
        nResult = varValue.toString().size() * 2;
    } else if (valueType == XBinary::VT_U_I) {
        nResult = varValue.toString().size() * 2;
    } else if (valueType == XBinary::VT_UTF8) {
        nResult = varValue.toString().toUtf8().size();
    } else if (valueType == XBinary::VT_UTF8_I) {
        nResult = varValue.toString().toUtf8().size();
    } else if (valueType == XBinary::VT_UTF32) {
        const qint64 nByteSize = (qint64)varValue.toString().toUcs4().size() * 4;
        nResult = (nByteSize > (std::numeric_limits<qint32>::max)()) ? (std::numeric_limits<qint32>::max)() : (qint32)nByteSize;
    } else if (valueType == XBinary::VT_SIGNATURE) {
        QString sSignature = convertSignature(varValue.toString());
        nResult = sSignature.size() / 2;
    } else {
        nResult = getBaseValueSize(valueType);
    }

    return nResult;
}

qint32 XBinary::getBaseValueSize(VT valueType)
{
    qint32 nResult = 1;

    if ((valueType == XBinary::VT_BYTE) || (valueType == XBinary::VT_INT8) || (valueType == XBinary::VT_UINT8)) {
        nResult = 1;
    } else if (valueType == XBinary::VT_WORD) {
        nResult = 2;
    } else if (valueType == XBinary::VT_DWORD) {
        nResult = 4;
    } else if (valueType == XBinary::VT_QWORD) {
        nResult = 8;
    } else if (valueType == XBinary::VT_CHAR) {
        nResult = 1;
    } else if (valueType == XBinary::VT_UCHAR) {
        nResult = 1;
    } else if (valueType == XBinary::VT_SHORT) {
        nResult = 2;
    } else if (valueType == XBinary::VT_USHORT) {
        nResult = 2;
    } else if ((valueType == XBinary::VT_INT16) || (valueType == XBinary::VT_UINT16)) {
        nResult = 2;
    } else if (valueType == XBinary::VT_INT) {
        nResult = 4;
    } else if ((valueType == XBinary::VT_INT32) || (valueType == XBinary::VT_UINT) || (valueType == XBinary::VT_UINT32)) {
        nResult = 4;
    } else if ((valueType == XBinary::VT_INT64) || (valueType == XBinary::VT_UINT64)) {
        nResult = 8;
    } else if (valueType == XBinary::VT_FLOAT) {
        nResult = 4;
    } else if (valueType == XBinary::VT_DOUBLE) {
        nResult = 8;
    } else if (valueType == XBinary::VT_UTF32) {
        nResult = 4;
    }

    return nResult;
}

qint32 XBinary::getValueSymbolSize(VT valueType)
{
    qint32 nResult = 1;

    if ((valueType == XBinary::VT_BYTE) || (valueType == XBinary::VT_INT8) || (valueType == XBinary::VT_UINT8) || (valueType == XBinary::VT_CHAR) ||
        (valueType == XBinary::VT_UCHAR)) {
        nResult = 2;
    } else if ((valueType == XBinary::VT_WORD) || (valueType == XBinary::VT_INT16) || (valueType == XBinary::VT_UINT16) || (valueType == XBinary::VT_SHORT) ||
               (valueType == XBinary::VT_USHORT)) {
        nResult = 4;
    } else if ((valueType == XBinary::VT_DWORD) || (valueType == XBinary::VT_INT32) || (valueType == XBinary::VT_UINT32) || (valueType == XBinary::VT_INT) ||
               (valueType == XBinary::VT_UINT)) {
        nResult = 8;
    } else if ((valueType == XBinary::VT_QWORD) || (valueType == XBinary::VT_INT64) || (valueType == XBinary::VT_UINT64)) {
        nResult = 16;
    }

    return nResult;
}

bool XBinary::isIntegerType(VT valueType)
{
    bool bResult = false;

    if ((valueType == XBinary::VT_BYTE) || (valueType == XBinary::VT_WORD) || (valueType == XBinary::VT_DWORD) || (valueType == XBinary::VT_QWORD) ||
        (valueType == XBinary::VT_CHAR) || (valueType == XBinary::VT_SHORT) || (valueType == XBinary::VT_UCHAR) || (valueType == XBinary::VT_USHORT) ||
        (valueType == XBinary::VT_INT) || (valueType == XBinary::VT_INT8) || (valueType == XBinary::VT_INT16) || (valueType == XBinary::VT_INT32) ||
        (valueType == XBinary::VT_INT64) || (valueType == XBinary::VT_UINT) || (valueType == XBinary::VT_UINT8) || (valueType == XBinary::VT_UINT16) ||
        (valueType == XBinary::VT_UINT32) || (valueType == XBinary::VT_UINT64)) {
        bResult = true;
    }

    return bResult;
}

XBinary::VT XBinary::getValueType(quint64 nValue)
{
    XBinary::VT result = VT_QWORD;

    if (nValue <= 0xFF) {
        result = VT_BYTE;
    } else if (nValue <= 0xFFFF) {
        result = VT_WORD;
    } else if (nValue <= 0xFFFFFFFF) {
        result = VT_DWORD;
    }

    return result;
}

QByteArray XBinary::getUnicodeString(const QString &sString, bool bIsBigEndian)
{
    QByteArray baResult;

    if (sString.size() > (std::numeric_limits<qint32>::max)() / 2) {
        return baResult;
    }

    const qint32 nSize = sString.size() * 2;

    try {
        baResult.resize(nSize);
    } catch (const std::bad_alloc &) {
        baResult.clear();
        return baResult;
    }

    char *pData = baResult.data();

    for (qint32 i = 0; i < sString.size(); i++) {
        const quint16 nValue = sString.at(i).unicode();

        if (bIsBigEndian) {
            pData[2 * i] = (char)(nValue >> 8);
            pData[2 * i + 1] = (char)(nValue & 0xFF);
        } else {
            pData[2 * i] = (char)(nValue & 0xFF);
            pData[2 * i + 1] = (char)(nValue >> 8);
        }
    }

    return baResult;
}

QByteArray XBinary::getStringData(VT valueType, const QString &sString, bool bAddNull)
{
    QByteArray baResult;

    qint32 nSize = sString.size();

    char buffer[4] = {};

    if (valueType == VT_A) {
        baResult = sString.toLatin1();

        if (bAddNull) {
            baResult.append(buffer, 1);
        }
    } else if (valueType == VT_U) {
        baResult.resize(nSize * 2);

        baResult.fill(0);

        QByteArray baString = getUnicodeString(sString, false);  // mb TODO Endian

        _copyMemory(baResult.data(), baString.data(), baString.size());

        if (bAddNull) {
            baResult.append(buffer, 2);
        }
    } else if (valueType == VT_UTF8) {
        baResult = sString.toUtf8();

        if (bAddNull) {
            baResult.append(buffer, 1);
        }
    }

    return baResult;
}

bool XBinary::isSignaturePresent(_MEMORY_MAP *pMemoryMap, qint64 nOffset, qint64 nSize, const QString &sSignature, PDSTRUCT *pPdStruct)
{
    qint64 nResultSize = 0;

    return (find_signature(pMemoryMap, nOffset, nSize, sSignature, &nResultSize, pPdStruct) != -1);
}

bool XBinary::isSignatureValid(const QString &sSignature, PDSTRUCT *pPdStruct)
{
    bool bResult = false;

    if (sSignature.size()) {
        QString _sSignature = convertSignature(sSignature);
        if (!_sSignature.isEmpty()) {
            bResult = true;
            const QList<SIGNATURE_RECORD> listSignatureRecords = getSignatureRecords(_sSignature, &bResult, pPdStruct);
            bResult = bResult && !listSignatureRecords.isEmpty();
        }
    }

    return bResult;
}

bool XBinary::createFile(const QString &sFileName, qint64 nFileSize)
{
    if (sFileName.trimmed().isEmpty() || (nFileSize < 0)) {
        return false;
    }

    QSaveFile file(sFileName);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    if (!file.resize(nFileSize)) {
        file.cancelWriting();
        return false;
    }

    return file.commit();
}

bool XBinary::isFileExists(const QString &sFileName, bool bTryToOpen)
{
    bool bResult = false;

    QFileInfo fi(sFileName);

    bResult = ((fi.exists() && fi.isFile()));

    if (bResult && bTryToOpen) {
        QFile file(sFileName);

        bResult = tryToOpen(&file);

        file.close();
    }

    return bResult;
}

bool XBinary::removeFile(const QString &sFileName)
{
    if (sFileName.trimmed().isEmpty()) {
        return false;
    }

#ifdef Q_OS_WIN
    const QFileInfo targetInfo(sFileName);
    const QFile::Permissions originalPermissions = QFile::permissions(sFileName);
    bool bPermissionsChanged = false;

    const QFile::Permissions writePermissions = QFileDevice::WriteOwner | QFileDevice::WriteUser;
    if (targetInfo.exists() && targetInfo.isFile() && !targetInfo.isSymLink() && !(originalPermissions & writePermissions)) {
        bPermissionsChanged = QFile::setPermissions(sFileName, originalPermissions | writePermissions);
    }
#endif

    const bool bResult = QFile::remove(sFileName);

#ifdef Q_OS_WIN
    if (!bResult && bPermissionsChanged) {
        QFile::setPermissions(sFileName, originalPermissions);
    }
#endif

    return bResult;
}

bool XBinary::copyFile(const QString &sSrcFileName, const QString &sDestFileName)
{
    // mb TODO remove first
    return QFile::copy(sSrcFileName, sDestFileName);
}

bool XBinary::moveFile(const QString &sSrcFileName, const QString &sDestFileName)
{
    bool bResult = false;

    if (copyFile(sSrcFileName, sDestFileName)) {
        bResult = removeFile(sSrcFileName);

        if (!bResult) {
            removeFile(sDestFileName);
        }
    }

    return bResult;
}

bool XBinary::moveFileToDirectory(const QString &sSrcFileName, const QString &sDestDirectory)
{
    const QFileInfo sourceInfo(sSrcFileName);
    const QFileInfo destinationInfo(sDestDirectory);
    if (sSrcFileName.trimmed().isEmpty() || sDestDirectory.trimmed().isEmpty() ||
        !sourceInfo.exists() || !sourceInfo.isFile() || sourceInfo.isSymLink() || !destinationInfo.exists() ||
        !destinationInfo.isDir() || destinationInfo.isSymLink() || !isDirectCanonicalPath(sourceInfo) ||
        !isDirectCanonicalPath(destinationInfo)) {
        return false;
    }

    const QString sDestinationAbsolute = QDir::cleanPath(destinationInfo.absoluteFilePath());
    const QString sDestinationCanonical = QDir::cleanPath(destinationInfo.canonicalFilePath());
    const Qt::CaseSensitivity pathCaseSensitivity = fileSystemPathCaseSensitivity();
    if (sDestinationCanonical.isEmpty() || (sDestinationAbsolute.compare(sDestinationCanonical, pathCaseSensitivity) != 0)) {
        return false;
    }

    return moveFile(sSrcFileName, QDir(sDestinationCanonical).filePath(sourceInfo.fileName()));
}

QString XBinary::convertFileNameSymbols(const QString &sFileName, const QString &sDefaultString)
{
    QString sResult = sFileName;

    sResult = sResult.replace("/", "_");
    sResult = sResult.replace("\\", "_");
    sResult = sResult.replace("?", "_");
    sResult = sResult.replace("*", "_");
    sResult = sResult.replace("\"", "_");
    sResult = sResult.replace("<", "_");
    sResult = sResult.replace(">", "_");
    sResult = sResult.replace("|", "_");
    sResult = sResult.replace(":", "_");
    sResult = sResult.replace("\n", "_");
    sResult = sResult.replace("\r", "_");
    sResult = sResult.replace("{", "_");
    sResult = sResult.replace("}", "_");

    if (sResult.isEmpty()) {
        sResult = sDefaultString;
    }

    return sResult;
}

QString XBinary::fixFileName(const QString &sFileName)
{
    QString sNormalized = sFileName;
    sNormalized.replace(QLatin1Char('\\'), QLatin1Char('/'));

    QStringList listSourceParts = sNormalized.split(QLatin1Char('/'));
    QStringList listResultParts;

    for (qint32 i = 0; i < listSourceParts.count(); i++) {
        QString sPart = listSourceParts.at(i);

        // Empty and current-directory components only describe an absolute or
        // redundant path. Archive output names must always remain relative.
        if (sPart.isEmpty() || (sPart == QLatin1String("."))) {
            continue;
        }

        if (sPart == QLatin1String("..")) {
            listResultParts.append(QLatin1String("_"));
            continue;
        }

        QString sFixedPart;

        for (qint32 j = 0; j < sPart.length(); j++) {
            QChar ch = sPart.at(j);
            ushort nCode = ch.unicode();

            // Preserve valid UTF-16 surrogate pairs so readable supplementary
            // characters (for example emoji) are not replaced.
            if ((nCode >= 0xD800) && (nCode <= 0xDBFF) && ((j + 1) < sPart.length())) {
                ushort nNextCode = sPart.at(j + 1).unicode();

                if ((nNextCode >= 0xDC00) && (nNextCode <= 0xDFFF)) {
                    sFixedPart.append(ch);
                    sFixedPart.append(sPart.at(++j));
                    continue;
                }
            }

            bool bInvalid = (nCode < 0x20) || ((nCode >= 0xD800) && (nCode <= 0xDFFF)) || (ch == QLatin1Char('<')) || (ch == QLatin1Char('>')) ||
                            (ch == QLatin1Char(':')) || (ch == QLatin1Char('"')) || (ch == QLatin1Char('|')) || (ch == QLatin1Char('?')) ||
                            (ch == QLatin1Char('*'));

            sFixedPart.append(bInvalid ? QLatin1Char('_') : ch);
        }

        // These endings are not representable on Windows and are confusing on
        // other platforms, so use the same portable result everywhere.
        while (sFixedPart.endsWith(QLatin1Char('.')) || sFixedPart.endsWith(QLatin1Char(' '))) {
            sFixedPart.chop(1);
        }

        if (sFixedPart.isEmpty()) {
            sFixedPart = QLatin1String("_");
        }

        QString sUpperBase = sFixedPart.section(QLatin1Char('.'), 0, 0).toUpper();

        while (sUpperBase.endsWith(QLatin1Char(' ')) || sUpperBase.endsWith(QLatin1Char('.'))) {
            sUpperBase.chop(1);
        }

        bool bReserved = (sUpperBase == QLatin1String("CON")) || (sUpperBase == QLatin1String("PRN")) || (sUpperBase == QLatin1String("AUX")) ||
                         (sUpperBase == QLatin1String("NUL")) || (sUpperBase == QLatin1String("CLOCK$")) ||
                         (sUpperBase == QLatin1String("CONIN$")) || (sUpperBase == QLatin1String("CONOUT$"));

        if (!bReserved && (sUpperBase.length() == 4)) {
            QChar chIndex = sUpperBase.at(3);
            bool bDeviceIndex = ((chIndex >= QLatin1Char('1')) && (chIndex <= QLatin1Char('9'))) || (chIndex.unicode() == 0x00B9) ||
                                (chIndex.unicode() == 0x00B2) || (chIndex.unicode() == 0x00B3);
            bReserved = (sUpperBase.startsWith(QLatin1String("COM")) || sUpperBase.startsWith(QLatin1String("LPT"))) && bDeviceIndex;
        }

        if (bReserved) {
            sFixedPart.prepend(QLatin1Char('_'));
        }

        sFixedPart = sFixedPart.normalized(QString::NormalizationForm_C);

        // Keep room for a collision suffix while remaining below common
        // filesystem component limits. Preserve a short extension when possible.
        const qint32 nMaxComponentLength = 240;

        const qint32 nMaxComponentUtf8Bytes = 240;

        if ((sFixedPart.length() > nMaxComponentLength) ||
            (sFixedPart.toUtf8().size() > nMaxComponentUtf8Bytes)) {
            qint32 nDotPosition = sFixedPart.lastIndexOf(QLatin1Char('.'));
            QString sExtension;

            if ((nDotPosition > 0) && ((sFixedPart.length() - nDotPosition) <= 32)) {
                sExtension = sFixedPart.mid(nDotPosition);
            }

            const qint32 nBaseLength = nMaxComponentLength - sExtension.length();
            const qint32 nBaseUtf8Bytes = nMaxComponentUtf8Bytes - sExtension.toUtf8().size();
            QString sBase = sFixedPart.left(qMax(0, nBaseLength));

            while ((sBase.toUtf8().size() > qMax(0, nBaseUtf8Bytes)) ||
                   (sBase.length() > nBaseLength)) {
                if (sBase.isEmpty()) break;
                const qint32 nLast = sBase.length() - 1;
                if (sBase.at(nLast).isLowSurrogate() && (nLast > 0) &&
                    sBase.at(nLast - 1).isHighSurrogate()) {
                    sBase.chop(2);
                } else {
                    sBase.chop(1);
                }
            }

            sFixedPart = sBase + sExtension;
        }

        listResultParts.append(sFixedPart);
    }

    QString sResult = listResultParts.join(QLatin1Char('/'));

    if (sResult.isEmpty() && !sFileName.isEmpty()) {
        sResult = QLatin1String("_");
    }

    return sResult;
}

QString XBinary::getBaseFileName(const QString &sFileName)
{
    QFileInfo fi(sFileName);

    return fi.baseName();
}

bool XBinary::createDirectory(const QString &sDirectoryName)
{
    return !sDirectoryName.trimmed().isEmpty() && QDir().mkpath(sDirectoryName);
}

bool XBinary::isDirectoryExists(const QString &sDirectoryName)
{
    QFileInfo fi(sDirectoryName);

    return (fi.exists() && fi.isDir());
}

bool XBinary::removeDirectory(const QString &sDirectoryName)
{
    if (sDirectoryName.trimmed().isEmpty()) {
        return false;
    }

    QFileInfo fileInfo(sDirectoryName);
    if (!fileInfo.exists() || !fileInfo.isDir() || fileInfo.isSymLink()) {
        return false;
    }

    const QString sCanonicalPath = QDir::cleanPath(fileInfo.canonicalFilePath());
    const QString sCurrentPath = QDir::cleanPath(QDir::current().canonicalPath());
    const QString sRootPath = QDir::cleanPath(QDir(fileInfo.absolutePath()).rootPath());
    const Qt::CaseSensitivity pathCaseSensitivity = fileSystemPathCaseSensitivity();
    if (!isDirectCanonicalPath(fileInfo) ||
        (sCanonicalPath.compare(sRootPath, pathCaseSensitivity) == 0)) {
        return false;
    }

    const QString sCurrentRelative = QDir::fromNativeSeparators(QDir(sCanonicalPath).relativeFilePath(sCurrentPath));
    const bool bDifferentVolume = QDir::isAbsolutePath(sCurrentRelative);
    if (!bDifferentVolume && ((sCurrentRelative == QLatin1String(".")) ||
        (!sCurrentRelative.startsWith(QLatin1String("../")) && (sCurrentRelative != QLatin1String(".."))))) {
        return false;
    }

    QDir dir(sCanonicalPath);

#if (QT_VERSION_MAJOR > 4)
    return dir.removeRecursively();
#else
    return dir.remove(sCanonicalPath);
#endif
}

bool XBinary::isDirectoryEmpty(const QString &sDirectoryName)
{
    return !sDirectoryName.trimmed().isEmpty() && isDirectoryExists(sDirectoryName) &&
           (QDir(sDirectoryName).entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries).count() == 0);
}

QDateTime XBinary::getDirectoryLatestModificationDate(const QString &sDirectoryName)
{
    const QFileInfo rootInfo(sDirectoryName);
    if (sDirectoryName.trimmed().isEmpty() || !rootInfo.exists() || !rootInfo.isDir() || rootInfo.isSymLink() ||
        !isDirectCanonicalPath(rootInfo)) {
        return QDateTime();
    }

    QDateTime latestMod = rootInfo.lastModified();
    QDir dir(sDirectoryName);

    QFileInfoList entries = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    qint32 nNumberOfEntries = entries.size();

    for (qint32 i = 0; i < nNumberOfEntries; i++) {
        const QFileInfo &entry = entries.at(i);
        const QDateTime entryModification = entry.lastModified();
        if (entryModification.isValid() && entryModification > latestMod) latestMod = entryModification;
        if (entry.isDir()) {
            QDateTime subDirMod = getDirectoryLatestModificationDate(entry.absoluteFilePath());
            if (subDirMod.isValid() && subDirMod > latestMod) latestMod = subDirMod;
        } else if (entry.isFile()) {
            QDateTime fileMod = entry.lastModified();
            if (fileMod > latestMod) latestMod = fileMod;
        }
    }
    return latestMod;
}

bool XBinary::createEmptyFile(const QString &sFileName)
{
    return createFile(sFileName, 0);
}

QByteArray XBinary::readFile(const QString &sFileName, PDSTRUCT *pPdStruct)
{
    QByteArray baResult;

    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();
    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }
    if (!isPdStructNotCanceled(pPdStruct)) {
        return baResult;
    }

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        const qint64 nExpectedSize = file.size();
        const qint32 nRequestedBufferSize = getBufferSize(pPdStruct);

        if ((nExpectedSize < 0) || (nExpectedSize > (std::numeric_limits<qint32>::max)()) ||
            (nRequestedBufferSize <= 0)) {
            return baResult;
        }

        try {
            baResult.resize((qint32)nExpectedSize);
        } catch (const std::bad_alloc &) {
            baResult.clear();
            return baResult;
        }
        if (baResult.size() != nExpectedSize) {
            baResult.clear();
            return baResult;
        }

        const qint32 nBufferSize = qBound((qint32)0x1000, nRequestedBufferSize, (qint32)0x100000);
        const qint32 nFreeIndex = XBinary::reservePdStructRecord(pPdStruct, nExpectedSize);

        qint64 nOffset = 0;
        while ((nOffset < nExpectedSize) && isPdStructNotCanceled(pPdStruct)) {
            const qint64 nToRead = qMin((qint64)nBufferSize, nExpectedSize - nOffset);
            const qint64 nRead = file.read(baResult.data() + nOffset, nToRead);
            if ((nRead <= 0) || (nRead > nToRead)) {
                break;
            }
            nOffset += nRead;
            XBinary::setPdStructCurrent(pPdStruct, nFreeIndex, nOffset);
        }

        XBinary::setPdStructFinished(pPdStruct, nFreeIndex);

        if ((nOffset != nExpectedSize) || (file.size() != nExpectedSize) || !isPdStructNotCanceled(pPdStruct)) {
            baResult.clear();
        }

        file.close();
    }

    return baResult;
}

bool XBinary::readFile(const QString &sFileName, char *pBuffer, qint64 nSize, PDSTRUCT *pPdStruct)
{
    bool bResult = false;

    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    if ((nSize < 0) || ((nSize > 0) && !pBuffer) || !isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        const qint32 nRequestedBufferSize = getBufferSize(pPdStruct);
        if ((nRequestedBufferSize <= 0) || (file.size() < nSize)) {
            return false;
        }

        qint32 _nFreeIndex = XBinary::reservePdStructRecord(pPdStruct, nSize);
        qint64 nOffset = 0;

        bResult = true;
        const qint32 nBufferSize = qBound((qint32)0x1000, nRequestedBufferSize, (qint32)0x100000);

        while ((nSize > 0) && isPdStructNotCanceled(pPdStruct)) {
            qint64 nCurrentSize = qMin(nSize, (qint64)nBufferSize);

            const qint64 nRead = file.read(pBuffer, nCurrentSize);
            if ((nRead <= 0) || (nRead > nCurrentSize)) {
                bResult = false;
                break;
            }

            nSize -= nRead;
            pBuffer += nRead;
            nOffset += nRead;

            XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, nOffset);
        }

        XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);

        if (!isPdStructNotCanceled(pPdStruct) || (nSize != 0)) {
            bResult = false;
        }

        file.close();
    }

    return bResult;
}

void XBinary::_copyMemory(char *pDest, const char *pSource, qint64 nSize)
{
    if (pDest && pSource && (nSize > 0)) {
        memmove(pDest, pSource, (size_t)nSize);
    }
}

void XBinary::_zeroMemory(char *pDest, qint64 nSize)
{
    if (pDest && (nSize > 0)) {
        memset(pDest, 0, (size_t)nSize);
    }
}

bool XBinary::_isMemoryZeroFilled(char *pSource, qint64 nSize)
{
    if (nSize < 0) return false;
    if (nSize == 0) return true;
    if (!pSource) return false;

    // Check in 8-byte chunks for better performance
    while (nSize >= 8) {
        quint64 nValue = 0;
        memcpy(&nValue, pSource, sizeof(nValue));

        if (nValue != 0) {
            return false;
        }
        pSource += 8;
        nSize -= 8;
    }

    // Check remaining bytes one by one
    while (nSize) {
        if (*pSource) {
            return false;
        }
        pSource++;
        nSize--;
    }

    return true;
}

bool XBinary::_isMemoryNotNull(char *pSource, qint64 nSize)
{
    if (nSize < 0) return false;
    if (nSize == 0) return true;
    if (!pSource) return false;

#ifdef USE_XSIMD
    return xsimd_is_not_null(pSource, nSize) != 0;
#else
    const quint8 *pCurrent = reinterpret_cast<const quint8 *>(pSource);
    while (nSize-- > 0) {
        if (*pCurrent++ == 0) {
            return false;
        }
    }

    return true;
#endif
}

bool XBinary::_isMemoryAnsi(char *pSource, qint64 nSize)
{
    if (nSize < 0) return false;
    if (nSize == 0) return true;
    if (!pSource) return false;

#ifdef USE_XSIMD
    return xsimd_is_ansi(pSource, nSize) != 0;
#else
    const quint8 *pCurrent = reinterpret_cast<const quint8 *>(pSource);
    while (nSize-- > 0) {
        if ((*pCurrent < 0x20) || (*pCurrent >= 0x80)) {
            return false;
        }
        pCurrent++;
    }

    return true;
#endif
}

bool XBinary::_isMemoryNotAnsi(char *pSource, qint64 nSize)
{
    if (nSize < 0) return false;
    if (nSize == 0) return true;
    if (!pSource) return false;

#ifdef USE_XSIMD
    return xsimd_is_not_ansi(pSource, nSize) != 0;
#else
    const quint8 *pCurrent = reinterpret_cast<const quint8 *>(pSource);
    while (nSize-- > 0) {
        if ((*pCurrent >= 0x20) && (*pCurrent < 0x80)) {
            return false;
        }
        pCurrent++;
    }

    return true;
#endif
}

bool XBinary::_isMemoryNotAnsiAndNull(char *pSource, qint64 nSize)
{
    if (nSize < 0) return false;
    if (nSize == 0) return true;
    if (!pSource) return false;

#ifdef USE_XSIMD
    return xsimd_is_not_ansi_and_null(pSource, nSize) ? true : false;
#else
    const quint8 *pCurrent = reinterpret_cast<const quint8 *>(pSource);
    while (nSize-- > 0) {
        if ((*pCurrent == 0) || ((*pCurrent >= 0x20) && (*pCurrent < 0x80))) {
            return false;
        }
        pCurrent++;
    }

    return true;
#endif
}

bool XBinary::_isMemoryAnsiNumber(char *pSource, qint64 nSize)
{
    if (nSize < 0) return false;
    if (nSize == 0) return true;
    if (!pSource) return false;

#ifdef USE_XSIMD
    return xsimd_is_ansi_number(pSource, nSize) ? true : false;
#else
    const quint8 *pCurrent = reinterpret_cast<const quint8 *>(pSource);
    while (nSize-- > 0) {
        if ((*pCurrent < 0x30) || (*pCurrent > 0x39)) {
            return false;
        }
        pCurrent++;
    }

    return true;
#endif
}

bool XBinary::devicesAlias(QIODevice *pFirstDevice, QIODevice *pSecondDevice)
{
    if (!pFirstDevice || !pSecondDevice) return false;

    const DEVICE_ROOT_VIEW firstView = getDeviceRootView(pFirstDevice);
    const DEVICE_ROOT_VIEW secondView = getDeviceRootView(pSecondDevice);

    // A broken/cyclic SubDevice chain cannot safely be proven independent.
    if (!firstView.bValid || !secondView.bValid) return true;
    if (firstView.pRoot == secondView.pRoot) return true;

    QBuffer *pFirstBuffer = qobject_cast<QBuffer *>(firstView.pRoot);
    QBuffer *pSecondBuffer = qobject_cast<QBuffer *>(secondView.pRoot);
    if (pFirstBuffer && pSecondBuffer &&
        (&pFirstBuffer->buffer() == &pSecondBuffer->buffer())) {
        return true;
    }

    QFileDevice *pFirstFile = qobject_cast<QFileDevice *>(firstView.pRoot);
    QFileDevice *pSecondFile = qobject_cast<QFileDevice *>(secondView.pRoot);
    if (pFirstFile && pSecondFile) {
        // Unknown file identity is intentionally fail-closed: hard links and
        // platform-specific aliases must not pass a destructive preflight.
        return compareFileDeviceIdentity(pFirstFile, pSecondFile) != FILE_IDENTITY_DISTINCT;
    }

    return false;
}

bool XBinary::copyDeviceMemory(QIODevice *pSourceDevice, qint64 nSourceOffset, QIODevice *pDestDevice, qint64 nDestOffset, qint64 nSize, PDSTRUCT *pPdStruct)
{
    const qint64 nMax = (std::numeric_limits<qint64>::max)();
    QPointer<QIODevice> guardedSource(pSourceDevice);
    QPointer<QIODevice> guardedDestination(pDestDevice);
    if (!guardedSource || !guardedDestination ||
        !guardedSource->isReadable() || !guardedSource ||
        !guardedDestination->isWritable() || !guardedDestination ||
        (nSourceOffset < 0) || (nDestOffset < 0) || (nSize < 0) ||
        (nSourceOffset > (nMax - nSize)) || (nDestOffset > (nMax - nSize)) ||
        !isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    const qint64 nSourceSize = guardedSource->size();
    if (!guardedSource || (nSourceSize < 0) ||
        (nSourceOffset > nSourceSize) ||
        (nSize > (nSourceSize - nSourceOffset))) {
        return false;
    }

    if (nSize == 0 || ((guardedSource.data() == guardedDestination.data()) &&
                       (nSourceOffset == nDestOffset))) {
        return guardedSource && guardedDestination;
    }

    const qint32 nRequestedBufferSize = getBufferSize(pPdStruct);
    if (nRequestedBufferSize <= 0) {
        return false;
    }
    const qint32 nBufferSize = qBound((qint32)0x1000, nRequestedBufferSize, (qint32)0x100000);

    char *pBuffer = new (std::nothrow) char[nBufferSize];
    if (!pBuffer) {
        return false;
    }

    const DEVICE_ROOT_VIEW sourceView = getDeviceRootView(guardedSource.data());
    if (!guardedSource || !guardedDestination) {
        delete[] pBuffer;
        return false;
    }
    const DEVICE_ROOT_VIEW destinationView = getDeviceRootView(
        guardedDestination.data());
    if (!guardedSource || !guardedDestination || !sourceView.bValid ||
        !destinationView.bValid ||
        (sourceView.nBaseOffset > nMax - nSourceOffset) ||
        (destinationView.nBaseOffset > nMax - nDestOffset)) {
        delete[] pBuffer;
        return false;
    }

    const qint64 nAbsoluteSourceOffset = sourceView.nBaseOffset + nSourceOffset;
    const qint64 nAbsoluteDestOffset = destinationView.nBaseOffset + nDestOffset;
    if ((nAbsoluteSourceOffset > nMax - nSize) ||
        (nAbsoluteDestOffset > nMax - nSize)) {
        delete[] pBuffer;
        return false;
    }

    const bool bPotentialAlias = XBinary::devicesAlias(
        guardedSource.data(), guardedDestination.data());
    if (!guardedSource || !guardedDestination) {
        delete[] pBuffer;
        return false;
    }

    const bool bOverlappingAlias =
        bPotentialAlias &&
        (nAbsoluteSourceOffset < nAbsoluteDestOffset + nSize) &&
        (nAbsoluteDestOffset < nAbsoluteSourceOffset + nSize);

    const auto readExactAt = [nMax, pPdStruct](QIODevice *pDevice, qint64 nOffset,
                                               char *pData, qint64 nLength) -> bool {
        if (!pDevice || (nOffset < 0) || (nLength < 0) ||
            ((nLength > 0) && !pData) || (nLength > nMax - nOffset)) {
            return false;
        }
        QPointer<QIODevice> guardedDevice(pDevice);
        qint64 nDone = 0;
        while ((nDone < nLength) && XBinary::isPdStructNotCanceled(pPdStruct)) {
            if (!guardedDevice || (nDone > nMax - nOffset)) return false;
            const bool bSeeked = guardedDevice->seek(nOffset + nDone);
            if (!guardedDevice || !bSeeked) return false;
            const qint64 nRead = guardedDevice->read(
                pData + nDone, nLength - nDone);
            if (!guardedDevice) return false;
            if ((nRead <= 0) || (nRead > nLength - nDone)) return false;
            nDone += nRead;
        }
        if (!guardedDevice || (nDone != nLength) ||
            !XBinary::isPdStructNotCanceled(pPdStruct)) return false;
        const bool bSeeked = guardedDevice->seek(nOffset + nLength);
        return guardedDevice && bSeeked;
    };

    const auto writeExactAt = [nMax, pPdStruct](QIODevice *pDevice, qint64 nOffset,
                                                const char *pData, qint64 nLength) -> bool {
        if (!pDevice || (nOffset < 0) || (nLength < 0) ||
            ((nLength > 0) && !pData) || (nLength > nMax - nOffset)) {
            return false;
        }
        QPointer<QIODevice> guardedDevice(pDevice);
        qint64 nDone = 0;
        while ((nDone < nLength) && XBinary::isPdStructNotCanceled(pPdStruct)) {
            if (!guardedDevice || (nDone > nMax - nOffset)) return false;
            const bool bSeeked = guardedDevice->seek(nOffset + nDone);
            if (!guardedDevice || !bSeeked) return false;
            const qint64 nWritten = guardedDevice->write(
                pData + nDone, nLength - nDone);
            if (!guardedDevice) return false;
            if ((nWritten <= 0) || (nWritten > nLength - nDone)) return false;
            nDone += nWritten;
        }
        if (!guardedDevice || (nDone != nLength) ||
            !XBinary::isPdStructNotCanceled(pPdStruct)) return false;
        const bool bSeeked = guardedDevice->seek(nOffset + nLength);
        return guardedDevice && bSeeked;
    };

    QIODevice *pStagingDevice = nullptr;
    QIODevice *pEffectiveSourceDevice = pSourceDevice;
    qint64 nEffectiveSourceOffset = nSourceOffset;

    if (bOverlappingAlias) {
        pStagingDevice = XBinary::createFileBuffer(nSize, pPdStruct);
        if (!pStagingDevice) {
            delete[] pBuffer;
            return false;
        }

        qint64 nStaged = 0;
        bool bStaged = true;
        while ((nStaged < nSize) && XBinary::isPdStructNotCanceled(pPdStruct)) {
            const qint64 nChunkSize = qMin(nSize - nStaged, (qint64)nBufferSize);
            if (!guardedSource || !guardedDestination ||
                !readExactAt(guardedSource.data(), nSourceOffset + nStaged,
                             pBuffer, nChunkSize) ||
                !writeExactAt(pStagingDevice, nStaged, pBuffer, nChunkSize)) {
                bStaged = false;
                break;
            }
            nStaged += nChunkSize;
        }

        if (!bStaged || (nStaged != nSize) ||
            !XBinary::isPdStructNotCanceled(pPdStruct)) {
            XBinary::freeFileBuffer(&pStagingDevice);
            delete[] pBuffer;
            return false;
        }

        pEffectiveSourceDevice = pStagingDevice;
        nEffectiveSourceOffset = 0;
    }

    const qint32 nFreeIndex = XBinary::reservePdStructRecord(pPdStruct, nSize);

    qint64 nRemaining = nSize;
    qint64 nProcessed = 0;
    bool bResult = true;

    while ((nRemaining > 0) && isPdStructNotCanceled(pPdStruct)) {
        const qint64 nChunkSize = qMin(nRemaining, (qint64)nBufferSize);
        const qint64 nChunkSourceOffset = nEffectiveSourceOffset + nProcessed;
        const qint64 nChunkDestOffset = nDestOffset + nProcessed;

        if (!guardedSource || !guardedDestination ||
            !readExactAt(pEffectiveSourceDevice, nChunkSourceOffset,
                         pBuffer, nChunkSize) ||
            !writeExactAt(guardedDestination.data(), nChunkDestOffset,
                          pBuffer, nChunkSize)) {
            bResult = false;
            break;
        }

        nRemaining -= nChunkSize;
        nProcessed += nChunkSize;
        XBinary::setPdStructCurrent(pPdStruct, nFreeIndex, nProcessed);
    }

    XBinary::freeFileBuffer(&pStagingDevice);
    delete[] pBuffer;
    XBinary::setPdStructFinished(pPdStruct, nFreeIndex);

    return bResult && guardedSource && guardedDestination &&
           (nRemaining == 0) && isPdStructNotCanceled(pPdStruct);
}

bool XBinary::copyMemory(qint64 nSourceOffset, qint64 nDestOffset, qint64 nSize, quint32 nBufferSize, bool bReverse)
{
    bool bResult = false;

    // TODO optimize
    if (nBufferSize == 0) {
        return false;
    }

    if (nBufferSize >= 0x1000) {
        nBufferSize = 0x1000;
    }

    if ((nSourceOffset < 0) || (nDestOffset < 0) || (nSize < 0)) {
        return false;
    }

    const qint64 nMaxSize = getSize();

    if ((nDestOffset > nMaxSize) || (nSourceOffset > nMaxSize) || (nSize > (nMaxSize - nDestOffset)) || (nSize > (nMaxSize - nSourceOffset))) {
        return false;
    }

    if ((nSize == 0) || (nDestOffset == nSourceOffset)) {
        return true;
    }

    // TODO
    char *pBuffer = new (std::nothrow) char[nBufferSize];
    if (!pBuffer) {
        return false;
    }

    if (bReverse) {
        nSourceOffset += nSize;
        nDestOffset += nSize;
    }

    bResult = true;
    PDSTRUCT pdStruct = XBinary::createPdStruct();
    pdStruct.nBufferSize = (qint32)nBufferSize;

    while (nSize > 0) {
        qint64 nTempSize = qMin(nSize, (qint64)nBufferSize);

        if (bReverse) {
            nSourceOffset -= nTempSize;
            nDestOffset -= nTempSize;
        }

        if ((read_array_process(nSourceOffset, pBuffer, nTempSize, &pdStruct) != nTempSize) ||
            (write_array_process(nDestOffset, pBuffer, nTempSize, &pdStruct) != nTempSize)) {
            bResult = false;
            break;
        }

        if (!bReverse) {
            nSourceOffset += nTempSize;
            nDestOffset += nTempSize;
        }

        nSize -= nTempSize;
    }

    delete[] pBuffer;

    return bResult;
}

bool XBinary::zeroFill(qint64 nOffset, qint64 nSize, PDSTRUCT *pPdStruct)
{
    if (nSize == 0) {
        return false;
    }

    if ((nOffset < 0) || (nSize < 0) || !isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    qint64 nMaxSize = getSize();

    // Prevent overflow and out-of-bounds
    if ((nOffset > nMaxSize) || (nSize > (nMaxSize - nOffset))) {
        return false;
    }

    const qint32 nRequestedBufferSize = getBufferSize(pPdStruct);
    if (nRequestedBufferSize <= 0) {
        return false;
    }
    const qint32 nBufferSize = qBound((qint32)0x1000, nRequestedBufferSize, (qint32)0x100000);
    char *pZero = new (std::nothrow) char[nBufferSize];
    if (!pZero) {
        return false;
    }
    memset(pZero, 0, (size_t)nBufferSize);

    qint32 _nFreeIndex = XBinary::reservePdStructRecord(pPdStruct, nSize);

    qint64 nProcessed = 0;
    bool bSuccess = true;

    while ((nSize > 0) && isPdStructNotCanceled(pPdStruct)) {
        qint64 nWrite = qMin(nSize, (qint64)nBufferSize);

        if (write_array_process(nOffset, pZero, nWrite, pPdStruct) != nWrite) {
            bSuccess = false;
            break;
        }

        nOffset += nWrite;
        nSize -= nWrite;
        nProcessed += nWrite;

        XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, nProcessed);
    }

    XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);

    delete[] pZero;

    return bSuccess && (nSize == 0) && isPdStructNotCanceled(pPdStruct);
}

bool XBinary::compareMemory(const char *pMemory1, const char *pMemory2, qint64 nSize)
{
    if ((nSize < 0) || ((nSize > 0) && (!pMemory1 || !pMemory2))) {
        return false;
    }

    if (nSize == 0) {
        return true;
    }

#ifdef USE_XSIMD
    return xsimd_compare_memory(const_cast<char *>(pMemory1), pMemory2, nSize) != 0;
#else
    return memcmp(pMemory1, pMemory2, (size_t)nSize) == 0;
#endif
}

bool XBinary::compareMemoryByteI(quint8 *pMemory, const quint8 *pMemoryU, const quint8 *pMemoryL, qint64 nSize)
{
    if ((nSize < 0) || ((nSize > 0) && (!pMemory || !pMemoryU || !pMemoryL))) {
        return false;
    }

    bool bResult = true;

    while (nSize > 0) {
        if ((*(pMemory) != *(pMemoryU)) && (*(pMemory) != *(pMemoryL))) {
            bResult = false;
            break;
        }

        pMemory++;
        pMemoryU++;
        pMemoryL++;
        nSize--;
    }

    return bResult;
}

bool XBinary::compareMemoryWordI(quint16 *pMemory, const quint16 *pMemoryU, const quint16 *pMemoryL, qint64 nSize)
{
    if ((nSize < 0) || ((nSize > 0) && (!pMemory || !pMemoryU || !pMemoryL))) {
        return false;
    }

    bool bResult = true;

    while (nSize > 0) {
        if ((*(pMemory) != *(pMemoryU)) && (*(pMemory) != *(pMemoryL))) {
            bResult = false;
            break;
        }

        pMemory++;
        pMemoryU++;
        pMemoryL++;
        nSize--;
    }

    return bResult;
}

bool XBinary::isOffsetValid(qint64 nOffset, qint64 nTotalSize)
{
    return (nOffset >= 0) && (nOffset < nTotalSize);
}

bool XBinary::isOffsetValid(qint64 nOffset)
{
    return isOffsetValid(nOffset, getSize());
}

bool XBinary::isAddressValid(XADDR nAddress)
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return isAddressValid(&memoryMap, nAddress);
}

bool XBinary::isRelAddressValid(qint64 nRelAddress)
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return isRelAddressValid(&memoryMap, nRelAddress);
}

XADDR XBinary::offsetToAddress(qint64 nOffset)
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return offsetToAddress(&memoryMap, nOffset);
}

qint64 XBinary::addressToOffset(quint64 nAddress)
{
    _MEMORY_MAP memoryMap = getMemoryMap();
    return addressToOffset(&memoryMap, nAddress);
}

XADDR XBinary::offsetToRelAddress(qint64 nOffset)
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return offsetToRelAddress(&memoryMap, nOffset);
}

qint64 XBinary::relAddressToOffset(qint64 nRelAddress)
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return relAddressToOffset(&memoryMap, nRelAddress);
}

bool XBinary::isEOD(_MEMORY_MAP *pMemoryMap, qint64 nOffset)
{
    return pMemoryMap && (pMemoryMap->nBinarySize == nOffset);
}

bool XBinary::isOffsetValid(XBinary::_MEMORY_MAP *pMemoryMap, qint64 nOffset)
{
    if (!pMemoryMap) {
        return false;
    }

    bool bResult = false;

    if (pMemoryMap->nBinarySize > 0) {
        bResult = ((nOffset >= 0) && (nOffset < pMemoryMap->nBinarySize));
    } else {
        qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

        for (qint32 i = 0; i < nNumberOfRecords; i++) {
            if (_containsMemoryOffset(pMemoryMap->listRecords.at(i), nOffset)) {
                bResult = true;
                break;
            }
        }
    }

    return bResult;
}

bool XBinary::isOffsetAndSizeValid(XBinary::_MEMORY_MAP *pMemoryMap, XBinary::OFFSETSIZE *pOsRegion)
{
    return pOsRegion && isOffsetAndSizeValid(pMemoryMap, pOsRegion->nOffset, pOsRegion->nSize);
}

bool XBinary::isOffsetAndSizeValid(qint64 nOffset, qint64 nSize)
{
    XBinary::_MEMORY_MAP memoryMap = getMemoryMap();

    return isOffsetAndSizeValid(&memoryMap, nOffset, nSize);
}

bool XBinary::isOffsetAndSizeValid(XBinary::_MEMORY_MAP *pMemoryMap, qint64 nOffset, qint64 nSize)
{
    if (!pMemoryMap || (nOffset < 0) || (nSize <= 0) || (nSize > (std::numeric_limits<qint64>::max)() - nOffset)) {
        return false;
    }

    if (pMemoryMap->nBinarySize > 0) {
        return (nOffset < pMemoryMap->nBinarySize) && (nSize <= (pMemoryMap->nBinarySize - nOffset));
    }

    const qint64 nEndOffset = nOffset + nSize;
    qint64 nCoveredOffset = nOffset;

    while (nCoveredOffset < nEndOffset) {
        qint64 nNextCoveredOffset = nCoveredOffset;
        const qint32 nNumberOfRecords = pMemoryMap->listRecords.count();
        for (qint32 i = 0; i < nNumberOfRecords; ++i) {
            const _MEMORY_RECORD &record = pMemoryMap->listRecords.at(i);
            if ((record.nOffset < 0) || (record.nSize <= 0) || (record.nOffset > nCoveredOffset) ||
                (record.nSize > (std::numeric_limits<qint64>::max)() - record.nOffset)) {
                continue;
            }

            const qint64 nRecordEnd = record.nOffset + record.nSize;
            if ((nCoveredOffset < nRecordEnd) && (nRecordEnd > nNextCoveredOffset)) {
                nNextCoveredOffset = qMin(nRecordEnd, nEndOffset);
            }
        }

        if (nNextCoveredOffset <= nCoveredOffset) {
            return false;
        }
        nCoveredOffset = nNextCoveredOffset;
    }

    return true;
}

bool XBinary::isOffsetAndSizeValid(QIODevice *pDevice, qint64 nOffset, qint64 nSize)
{
    XBinary binary(pDevice);

    _MEMORY_MAP memoryMap = binary.getMemoryMap();

    return isOffsetAndSizeValid(&memoryMap, nOffset, nSize);
}

bool XBinary::isAddressValid(XBinary::_MEMORY_MAP *pMemoryMap, XADDR nAddress)
{
    if (!pMemoryMap || (nAddress == (XADDR)-1)) {
        return false;
    }

    bool bResult = false;

    if (pMemoryMap->nImageSize > 0) {
        bResult = (pMemoryMap->nModuleAddress != (XADDR)-1) && (nAddress >= pMemoryMap->nModuleAddress) &&
                  ((nAddress - pMemoryMap->nModuleAddress) < (quint64)pMemoryMap->nImageSize);
    } else {
        qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

        for (qint32 i = 0; i < nNumberOfRecords; i++) {
            if (_containsMemoryAddress(pMemoryMap->listRecords.at(i), nAddress)) {
                bResult = true;
                break;
            }
        }
    }

    return bResult;
}

bool XBinary::isRelAddressValid(XBinary::_MEMORY_MAP *pMemoryMap, qint64 nRelAddress)
{
    if (!pMemoryMap) {
        return false;
    }

    XADDR nAddress = (XADDR)-1;

    return _addRelativeAddress(pMemoryMap->nModuleAddress, nRelAddress, &nAddress) && isAddressValid(pMemoryMap, nAddress);
}

bool XBinary::isAddressPhysical(XBinary::_MEMORY_MAP *pMemoryMap, XADDR nAddress)
{
    qint64 nOffset = addressToOffset(pMemoryMap, nAddress);

    return (nOffset != -1);
}

bool XBinary::isRelAddressPhysical(_MEMORY_MAP *pMemoryMap, XADDR nAddress)
{
    if (nAddress > (XADDR)(std::numeric_limits<qint64>::max)()) {
        return false;
    }

    qint64 nOffset = relAddressToOffset(pMemoryMap, (qint64)nAddress);

    return (nOffset != -1);
}

XADDR XBinary::offsetToAddress(XBinary::_MEMORY_MAP *pMemoryMap, qint64 nOffset)
{
    XADDR nResult = -1;

    if (!pMemoryMap) {
        return nResult;
    }

    qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

    //    for (qint32 i = 0; i < nNumberOfRecords; i++) {
    //        if (pMemoryMap->listRecords.at(i).nSize && (pMemoryMap->listRecords.at(i).nOffset != -1) && (pMemoryMap->listRecords.at(i).nAddress != -1)) {
    //            if ((pMemoryMap->listRecords.at(i).nOffset <= nOffset) && (nOffset < pMemoryMap->listRecords.at(i).nOffset + pMemoryMap->listRecords.at(i).nSize)) {
    //                nResult = (nOffset - pMemoryMap->listRecords.at(i).nOffset) + pMemoryMap->listRecords.at(i).nAddress;
    //                break;
    //            }
    //        }
    //    }

    // From the last to the fist

    for (qint32 i = nNumberOfRecords - 1; i >= 0; i--) {
        const _MEMORY_RECORD &record = pMemoryMap->listRecords.at(i);

        if (_containsMemoryOffset(record, nOffset) && _addRelativeAddress(record.nAddress, nOffset - record.nOffset, &nResult)) {
            break;
        }
    }

    return nResult;
}

qint64 XBinary::addressToOffset(XBinary::_MEMORY_MAP *pMemoryMap, XADDR nAddress)
{
    qint64 nResult = -1;

    if (!pMemoryMap || (nAddress == (XADDR)-1)) {
        return nResult;
    }

    //    if(pMemoryMap->mode==MODE_16) // Check COM Check 16SEG
    //    {
    //        if(nAddress>0xFFFF)
    //        {
    //            nAddress=((nAddress>>16)&0xFFFF)*16+(nAddress&0xFFFF);
    //        }
    //    }

    // if (pMemoryMap->fileType == FT_MSDOS) {
    //     qint64 _nResult = ((nAddress >> 16) & 0xFFFF) * 16 + (nAddress & 0xFFFF);

    //     if (_nResult >= 0x10000000) {
    //         _nResult -= 0x10000000;
    //     }

    //     if (_nResult == 0x100000) {
    //         _nResult = 0;
    //     }

    //     nResult = _nResult + pMemoryMap->nSegmentBase;

    //     if (nResult > pMemoryMap->nBinarySize) {
    //         nResult = -1;
    //     }
    // } else {
    //     qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

    //     for (qint32 i = 0; i < nNumberOfRecords; i++) {
    //         if (pMemoryMap->listRecords.at(i).nSize && (pMemoryMap->listRecords.at(i).nAddress != (XADDR)-1) && (pMemoryMap->listRecords.at(i).nOffset != -1)) {
    //             if ((pMemoryMap->listRecords.at(i).nAddress <= nAddress) && (nAddress < pMemoryMap->listRecords.at(i).nAddress + pMemoryMap->listRecords.at(i).nSize))
    //             {
    //                 nResult = (nAddress - pMemoryMap->listRecords.at(i).nAddress) + pMemoryMap->listRecords.at(i).nOffset;
    //                 break;
    //             }
    //         }
    //     }
    // }

    qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

    for (qint32 i = nNumberOfRecords - 1; i >= 0; i--) {
        const _MEMORY_RECORD &record = pMemoryMap->listRecords.at(i);

        if (!record.bIsVirtual && _containsMemoryAddress(record, nAddress) && _addRelativeOffset(record.nOffset, nAddress - record.nAddress, &nResult)) {
            break;
        }
    }

    return nResult;
}

XADDR XBinary::offsetToRelAddress(XBinary::_MEMORY_MAP *pMemoryMap, qint64 nOffset)
{
    XADDR nResult = offsetToAddress(pMemoryMap, nOffset);

    if (pMemoryMap && (nResult != (XADDR)-1) && (pMemoryMap->nModuleAddress != (XADDR)-1) && (nResult >= pMemoryMap->nModuleAddress)) {
        nResult -= pMemoryMap->nModuleAddress;
    } else {
        nResult = (XADDR)-1;
    }

    return nResult;
}

qint64 XBinary::relAddressToOffset(XBinary::_MEMORY_MAP *pMemoryMap, qint64 nRelAddress)
{
    if (!pMemoryMap) {
        return -1;
    }

    XADDR nAddress = (XADDR)-1;

    return _addRelativeAddress(pMemoryMap->nModuleAddress, nRelAddress, &nAddress) ? addressToOffset(pMemoryMap, nAddress) : -1;
}

XADDR XBinary::relAddressToAddress(XBinary::_MEMORY_MAP *pMemoryMap, qint64 nRelAddress)
{
    XADDR nResult = -1;

    if (pMemoryMap && _addRelativeAddress(pMemoryMap->nModuleAddress, nRelAddress, &nResult) && !isAddressValid(pMemoryMap, nResult)) {
        nResult = (XADDR)-1;
    }

    return nResult;
}

qint64 XBinary::addressToRelAddress(XBinary::_MEMORY_MAP *pMemoryMap, XADDR nAddress)
{
    qint64 nResult = -1;

    if (pMemoryMap && isAddressValid(pMemoryMap, nAddress) && (pMemoryMap->nModuleAddress != (XADDR)-1) && (nAddress >= pMemoryMap->nModuleAddress)) {
        const quint64 nRelativeAddress = nAddress - pMemoryMap->nModuleAddress;

        if (nRelativeAddress <= (quint64)(std::numeric_limits<qint64>::max)()) {
            nResult = (qint64)nRelativeAddress;
        }
    }

    return nResult;
}

XADDR XBinary::segmentRelOffsetToAddress(_MEMORY_MAP *pMemoryMap, quint16 nSegment, XADDR nRelOffset)
{
    XADDR nResult = -1;

    if (pMemoryMap && (nSegment < pMemoryMap->listRecords.count()) && (nRelOffset <= (XADDR)(std::numeric_limits<qint64>::max)())) {
        _addRelativeAddress(pMemoryMap->listRecords.at(nSegment).nAddress, (qint64)nRelOffset, &nResult);
    }

    return nResult;
}

qint64 XBinary::locationToOffset(_MEMORY_MAP *pMemoryMap, LT locType, XADDR nLocation)
{
    qint64 nResult = -1;

    if (locType == LT_ADDRESS) {
        nResult = addressToOffset(pMemoryMap, nLocation);
    } else if ((locType == LT_RELADDRESS) && (nLocation <= (XADDR)(std::numeric_limits<qint64>::max)())) {
        nResult = relAddressToOffset(pMemoryMap, (qint64)nLocation);
    } else if ((locType == LT_OFFSET) && (nLocation <= (XADDR)(std::numeric_limits<qint64>::max)())) {
        nResult = (qint64)nLocation;
    }

    return nResult;
}

qint64 XBinary::locToOffset(_MEMORY_MAP *pMemoryMap, const XLOC &xLoc)
{
    return locationToOffset(pMemoryMap, xLoc.locType, xLoc.nLocation);
}

XBinary::XLOC XBinary::offsetToLoc(qint64 nOffset)
{
    XLOC result = {};
    result.locType = LT_OFFSET;
    result.nLocation = nOffset;

    return result;
}

XADDR XBinary::getSegmentAddress(quint16 nSegment, quint16 nAddress)
{
    XADDR nResult = nSegment * 16 + nAddress;

    if (nResult >= 0x100000) {
        nResult -= 0x100000;
    }

    return nResult;
}

XBinary::_MEMORY_RECORD XBinary::getMemoryRecordByIndex(_MEMORY_MAP *pMemoryMap, qint32 nIndex)
{
    _MEMORY_RECORD result = {};

    if (pMemoryMap && (nIndex >= 0) && (nIndex < pMemoryMap->listRecords.count())) {
        result = pMemoryMap->listRecords.at(nIndex);
    }

    return result;
}

qint32 XBinary::getMemoryIndexByOffset(_MEMORY_MAP *pMemoryMap, qint64 nOffset)
{
    qint32 nResult = -1;

    if (!pMemoryMap) {
        return nResult;
    }

    qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

    for (qint32 i = nNumberOfRecords - 1; i >= 0; i--) {
        const _MEMORY_RECORD &record = pMemoryMap->listRecords.at(i);

        if (_containsMemoryOffset(record, nOffset)) {
            nResult = i;
            break;
        }
    }

    return nResult;
}

XBinary::_MEMORY_RECORD XBinary::getMemoryRecordByOffset(XBinary::_MEMORY_MAP *pMemoryMap, qint64 nOffset)
{
    _MEMORY_RECORD result = {};

    if (!pMemoryMap) {
        return result;
    }

    qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

    for (qint32 i = nNumberOfRecords - 1; i >= 0; i--) {
        const _MEMORY_RECORD &record = pMemoryMap->listRecords.at(i);

        if (_containsMemoryOffset(record, nOffset)) {
            result = record;
            break;
        }
    }

    return result;
}

XBinary::_MEMORY_RECORD XBinary::getMemoryRecordByAddress(XBinary::_MEMORY_MAP *pMemoryMap, XADDR nAddress)
{
    _MEMORY_RECORD result = {};

    if (!pMemoryMap) {
        return result;
    }

    qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

    for (qint32 i = nNumberOfRecords - 1; i >= 0; i--) {
        if (_containsMemoryAddress(pMemoryMap->listRecords.at(i), nAddress)) {
            result = pMemoryMap->listRecords.at(i);
            break;
        }
    }

    return result;
}

XBinary::_MEMORY_RECORD XBinary::getMemoryRecordByRelAddress(XBinary::_MEMORY_MAP *pMemoryMap, qint64 nRelAddress)
{
    _MEMORY_RECORD result = {};

    XADDR nAddress = relAddressToAddress(pMemoryMap, nRelAddress);

    if (nAddress != (XADDR)-1) {
        result = getMemoryRecordByAddress(pMemoryMap, nAddress);
    }

    return result;
}

qint32 XBinary::addressToFileTypeNumber(_MEMORY_MAP *pMemoryMap, XADDR nAddress)
{
    _MEMORY_RECORD mm = getMemoryRecordByAddress(pMemoryMap, nAddress);

    return mm.nFilePartNumber;
    ;
}

qint32 XBinary::relAddressToFileTypeNumber(_MEMORY_MAP *pMemoryMap, qint64 nRelAddress)
{
    _MEMORY_RECORD mm = getMemoryRecordByRelAddress(pMemoryMap, nRelAddress);

    return mm.nFilePartNumber;
}

bool XBinary::isAddressInHeader(_MEMORY_MAP *pMemoryMap, XADDR nAddress)
{
    bool bResult = false;

    _MEMORY_RECORD mm = getMemoryRecordByAddress(pMemoryMap, nAddress);

    if (mm.filePart == FILEPART_HEADER) {
        bResult = true;
    }

    return bResult;
}

bool XBinary::isRelAddressInHeader(_MEMORY_MAP *pMemoryMap, qint64 nRelAddress)
{
    bool bResult = false;

    _MEMORY_RECORD mm = getMemoryRecordByRelAddress(pMemoryMap, nRelAddress);

    if (mm.filePart == FILEPART_HEADER) {
        bResult = true;
    }

    return bResult;
}

QString XBinary::getLoadSectionNameByOffset(_MEMORY_MAP *pMemoryMap, qint64 nOffset)
{
    QString sResult;

    if (!pMemoryMap) {
        return sResult;
    }

    qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        const _MEMORY_RECORD &record = pMemoryMap->listRecords.at(i);

        if ((record.nAddress != (XADDR)-1) && _containsMemoryOffset(record, nOffset)) {
            sResult = record.sName;
            break;
        }
    }

    return sResult;
}

bool XBinary::isSolidAddressRange(XBinary::_MEMORY_MAP *pMemoryMap, quint64 nAddress, qint64 nSize)
{
    if (!pMemoryMap || nSize <= 0
        || nAddress
               > (std::numeric_limits<quint64>::max)()
                     - static_cast<quint64>(nSize)) {
        return false;
    }

    const XADDR nEnd =
        nAddress + static_cast<quint64>(nSize);
    const qint32 nRecordCount =
        pMemoryMap->listRecords.count();
    for (qint32 i = 0; i < nRecordCount; ++i) {
        const _MEMORY_RECORD &record =
            pMemoryMap->listRecords.at(i);
        if (record.nSize <= 0
            || record.nAddress == (XADDR)-1
            || record.nAddress
                   > (std::numeric_limits<quint64>::max)()
                         - static_cast<quint64>(
                             record.nSize)) {
            continue;
        }
        const XADDR nRecordEnd =
            record.nAddress
            + static_cast<quint64>(record.nSize);
        if (nAddress >= record.nAddress
            && nEnd <= nRecordEnd) {
            return true;
        }
    }

    return false;
}

bool XBinary::isPhysicalAddressRange(
    XBinary::_MEMORY_MAP *pMemoryMap, XADDR nAddress,
    qint64 nSize)
{
    if (!pMemoryMap || nSize <= 0
        || nAddress
               > (std::numeric_limits<quint64>::max)()
                     - static_cast<quint64>(nSize)) {
        return false;
    }

    const XADDR nEnd =
        nAddress + static_cast<quint64>(nSize);
    const qint32 nRecordCount =
        pMemoryMap->listRecords.count();

    // Match addressToOffset(): in overlapping maps, the last physical
    // record has precedence.
    for (qint32 i = nRecordCount - 1; i >= 0; --i) {
        const _MEMORY_RECORD &record =
            pMemoryMap->listRecords.at(i);
        if (record.nSize <= 0 || record.bIsVirtual
            || record.nOffset < 0
            || record.nAddress == (XADDR)-1
            || record.nAddress
                   > (std::numeric_limits<quint64>::max)()
                         - static_cast<quint64>(
                             record.nSize)) {
            continue;
        }
        const XADDR nRecordEnd =
            record.nAddress
            + static_cast<quint64>(record.nSize);
        if (nAddress < record.nAddress
            || nAddress >= nRecordEnd) {
            continue;
        }
        if (nEnd > nRecordEnd) {
            return false;
        }

        const quint64 nDelta =
            nAddress - record.nAddress;
        if (nDelta
                > static_cast<quint64>(
                    (std::numeric_limits<qint64>::max)())
            || record.nOffset
                   > (std::numeric_limits<qint64>::max)()
                         - static_cast<qint64>(nDelta)) {
            return false;
        }
        const qint64 nStartOffset =
            record.nOffset + static_cast<qint64>(nDelta);
        if (nStartOffset < 0
            || nStartOffset > pMemoryMap->nBinarySize
            || nSize > pMemoryMap->nBinarySize
                           - nStartOffset) {
            return false;
        }

        // A later record that begins inside the requested span would
        // take precedence partway through it. Reject that ambiguous
        // overlap instead of treating the endpoint mapping as solid.
        for (qint32 j = i + 1; j < nRecordCount; ++j) {
            const _MEMORY_RECORD &later =
                pMemoryMap->listRecords.at(j);
            if (later.nSize <= 0 || later.bIsVirtual
                || later.nOffset < 0
                || later.nAddress == (XADDR)-1
                || later.nAddress
                       > (std::numeric_limits<quint64>::max)()
                             - static_cast<quint64>(
                                 later.nSize)) {
                continue;
            }
            const XADDR nLaterEnd =
                later.nAddress
                + static_cast<quint64>(later.nSize);
            if (nAddress < nLaterEnd
                && later.nAddress < nEnd) {
                return false;
            }
        }
        return true;
    }

    return false;
}

QString XBinary::getMemoryRecordInfoByOffset(qint64 nOffset)
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return getMemoryRecordInfoByOffset(&memoryMap, nOffset);
}

QString XBinary::getMemoryRecordInfoByAddress(XADDR nAddress)
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return getMemoryRecordInfoByAddress(&memoryMap, nAddress);
}

QString XBinary::getMemoryRecordInfoByRelAddress(qint64 nRelAddress)
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return getMemoryRecordInfoByRelAddress(&memoryMap, nRelAddress);
}

QString XBinary::getMemoryRecordInfoByOffset(XBinary::_MEMORY_MAP *pMemoryMap, qint64 nOffset)
{
    XBinary::_MEMORY_RECORD memoryRecord = getMemoryRecordByOffset(pMemoryMap, nOffset);

    return getMemoryRecordName(&memoryRecord);
}

QString XBinary::getMemoryRecordInfoByAddress(XBinary::_MEMORY_MAP *pMemoryMap, XADDR nAddress)
{
    XBinary::_MEMORY_RECORD memoryRecord = getMemoryRecordByAddress(pMemoryMap, nAddress);

    return getMemoryRecordName(&memoryRecord);
}

QString XBinary::getMemoryRecordInfoByRelAddress(XBinary::_MEMORY_MAP *pMemoryMap, qint64 nRelAddress)
{
    XBinary::_MEMORY_RECORD memoryRecord = getMemoryRecordByRelAddress(pMemoryMap, nRelAddress);

    return getMemoryRecordName(&memoryRecord);
}

QString XBinary::getMemoryRecordName(XBinary::_MEMORY_RECORD *pMemoryRecord)
{
    QString sRecord;

    if (pMemoryRecord) {
        sRecord = pMemoryRecord->sName;  // TODO
    }

    return sRecord;
}

QString XBinary::mapModeToString(MAPMODE mapMode)
{
    QString sResult = tr("Unknown");

    switch (mapMode) {
        case MAPMODE_UNKNOWN: sResult = tr("Unknown"); break;
        case MAPMODE_REGIONS: sResult = tr("Regions"); break;
        case MAPMODE_SEGMENTS: sResult = tr("Segments"); break;
        case MAPMODE_SECTIONS: sResult = tr("Sections"); break;
        case MAPMODE_OBJECTS: sResult = tr("Objects"); break;
        case MAPMODE_MAPS: sResult = tr("Maps"); break;
        case MAPMODE_DATA: sResult = tr("Data"); break;
        case MAPMODE_STREAMS: sResult = tr("Streams"); break;
    }

    return sResult;
}

bool XBinary::_initMemoryMap(_MEMORY_MAP *pMemoryMap, PDSTRUCT *pPdStruct)
{
    if (!pMemoryMap || !isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    qint64 nTotalSize = getSize();

    if (nTotalSize < 0) {
        return false;
    }

    pMemoryMap->nModuleAddress = getModuleAddress();
    pMemoryMap->bIsImage = isImage();
    pMemoryMap->nBinarySize = nTotalSize;
    pMemoryMap->nImageSize = getImageSize();
    pMemoryMap->fileType = getFileType();
    pMemoryMap->mode = getMode();
    pMemoryMap->sArch = getArch();
    pMemoryMap->endian = getEndian();
    pMemoryMap->sType = getTypeAsString();
    pMemoryMap->nEntryPointAddress = _getEntryPointAddress();

    return true;
}

void XBinary::_processMemoryMap(_MEMORY_MAP *pMemoryMap, QList<FPART> *pListFParts, PDSTRUCT *pPdStruct)
{
    if (!pMemoryMap || !pListFParts || !isPdStructNotCanceled(pPdStruct)) {
        return;
    }

    std::sort(pListFParts->begin(), pListFParts->end(), compareFileParts);

    XADDR nMaxAddress = 0;
    XADDR nMinAddress = -1;
    qint32 nNumberOfParts = pListFParts->count();
    qint32 nIndex = 0;

    for (qint32 i = 0; i < nNumberOfParts && isPdStructNotCanceled(pPdStruct); i++) {
        FPART fpart = pListFParts->at(i);

        if ((fpart.nFileSize < 0) || (fpart.nVirtualSize < 0) || (fpart.nFileOffset < -1)) {
            continue;
        }

        quint64 nVirtualEnd = 0;
        if (fpart.nVirtualAddress != (XADDR)-1) {
            if ((quint64)fpart.nVirtualSize > (std::numeric_limits<XADDR>::max)() - fpart.nVirtualAddress) {
                continue;
            }
            nVirtualEnd = fpart.nVirtualAddress + (quint64)fpart.nVirtualSize;
        }

        _MEMORY_RECORD record = {};
        record.nAddress = fpart.nVirtualAddress;
        record.nOffset = fpart.nFileOffset;
        record.nSize = fpart.nFileSize;
        record.nIndex = nIndex++;
        record.sName = fpart.sName;
        record.bIsVirtual = false;
        record.filePart = fpart.filePart;
        record.nFilePartNumber = i;

        pMemoryMap->listRecords.append(record);

        if ((fpart.nVirtualAddress != (XADDR)-1) && (fpart.nVirtualSize > fpart.nFileSize)) {
            // Add virtual size
            _MEMORY_RECORD virtualRecord = {};
            virtualRecord.nAddress = fpart.nVirtualAddress + (quint64)fpart.nFileSize;
            virtualRecord.nOffset = -1;
            virtualRecord.nSize = fpart.nVirtualSize - fpart.nFileSize;
            virtualRecord.nIndex = nIndex++;
            virtualRecord.sName = fpart.sName + " (virtual)";
            virtualRecord.bIsVirtual = true;
            virtualRecord.filePart = fpart.filePart;
            virtualRecord.nFilePartNumber = i;

            pMemoryMap->listRecords.append(virtualRecord);
        }

        if (fpart.nVirtualAddress != (XADDR)-1) {
            if (nMinAddress == -1) {
                nMinAddress = fpart.nVirtualAddress;
            }

            nMinAddress = qMin(nMinAddress, fpart.nVirtualAddress);
            nMaxAddress = qMax(nMaxAddress, (XADDR)nVirtualEnd);
        }
    }
    pMemoryMap->nModuleAddress = nMinAddress;
    if ((nMinAddress != (XADDR)-1) && (nMaxAddress >= nMinAddress) &&
        ((nMaxAddress - nMinAddress) <= (XADDR)(std::numeric_limits<qint64>::max)())) {
        pMemoryMap->nImageSize = (qint64)(nMaxAddress - nMinAddress);
    } else {
        pMemoryMap->nImageSize = 0;
    }
}

XBinary::_MEMORY_MAP XBinary::_getSimpleMemoryMap(quint32 nFileParts, PDSTRUCT *pPdStruct)
{
    _MEMORY_MAP result = {};

    QList<FPART> listParts = getFileParts(nFileParts, 1000, pPdStruct);

    _processMemoryMap(&result, &listParts, pPdStruct);

    return result;
}

XBinary::_MEMORY_MAP XBinary::getSimpleMemoryMap()
{
    _MEMORY_MAP result = {};

    qint64 nTotalSize = getSize();

    result.nBinarySize = nTotalSize;

    _MEMORY_RECORD record = {};
    record.nAddress = 0;
    record.nOffset = 0;
    record.nSize = nTotalSize;
    record.nIndex = 0;

    result.listRecords.append(record);

    return result;
}

XBinary::_MEMORY_MAP XBinary::_getMemoryMap(QList<FPART> *pListFParts, PDSTRUCT *pPdStruct)
{
    // TODO isImage
    _MEMORY_MAP result = {};

    if (_initMemoryMap(&result, pPdStruct)) {
        _processMemoryMap(&result, pListFParts, pPdStruct);
    }

    return result;
}

XBinary::_MEMORY_MAP XBinary::_getMemoryMap(quint32 nFileParts, PDSTRUCT *pPdStruct)
{
    QList<FPART> listParts = getFileParts(nFileParts, 1000, pPdStruct);

    return _getMemoryMap(&listParts, pPdStruct);
}

QList<XBinary::FPART> XBinary::getHData(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct);

    QList<XBinary::FPART> listResult;

    return listResult;
}

QList<XBinary::FPART> XBinary::getNativeRegions(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct);

    QList<XBinary::FPART> listResult;

    return listResult;
}

QList<XBinary::FPART> XBinary::getNativeSubRegions(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct);

    QList<XBinary::FPART> listResult;

    return listResult;
}

QList<XBinary::MAPMODE> XBinary::getMapModesList()
{
    QList<MAPMODE> listResult;

    listResult.append(MAPMODE_REGIONS);

    return listResult;
}

XBinary::_MEMORY_MAP XBinary::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(mapMode)
    Q_UNUSED(pPdStruct)

    _MEMORY_MAP result = {};

    qint64 nTotalSize = getSize();

    result.nModuleAddress = getModuleAddress();
    result.nBinarySize = nTotalSize;
    result.nImageSize = getImageSize();
    result.fileType = getFileType();
    result.mode = getMode();
    result.sArch = getArch();
    result.endian = getEndian();
    result.sType = getTypeAsString();

    _MEMORY_RECORD record = {};
    record.nAddress = result.nModuleAddress;

    record.nOffset = 0;
    record.nSize = nTotalSize;
    record.nIndex = 0;
    record.sName = tr("Data");

    result.listRecords.append(record);

    return result;
}

qint32 XBinary::getNumberOfPhysicalRecords(XBinary::_MEMORY_MAP *pMemoryMap)
{
    qint32 nResult = 0;

    if (!pMemoryMap) {
        return nResult;
    }

    qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (!pMemoryMap->listRecords.at(i).bIsVirtual) {
            nResult++;
        }
    }

    return nResult;
}

qint32 XBinary::getNumberOfVirtualRecords(_MEMORY_MAP *pMemoryMap)
{
    qint32 nResult = 0;

    if (!pMemoryMap) {
        return nResult;
    }

    qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (pMemoryMap->listRecords.at(i).bIsVirtual) {
            nResult++;
        }
    }

    return nResult;
}

qint32 XBinary::getNumberOfMemoryMapFileParts(_MEMORY_MAP *pMemoryMap, FILEPART filePart)
{
    qint32 nResult = 0;

    if (!pMemoryMap) {
        return nResult;
    }

    qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if ((pMemoryMap->listRecords.at(i).filePart == filePart) && (!pMemoryMap->listRecords.at(i).bIsVirtual)) {
            nResult++;
        }
    }

    return nResult;
}

qint64 XBinary::getRecordsTotalRowSize(_MEMORY_MAP *pMemoryMap)
{
    qint64 nResult = 0;

    if (!pMemoryMap) {
        return nResult;
    }

    qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        const _MEMORY_RECORD &record = pMemoryMap->listRecords.at(i);
        if (!record.bIsVirtual && (record.nSize >= 0)) {
            if (record.nSize > (std::numeric_limits<qint64>::max)() - nResult) {
                return (std::numeric_limits<qint64>::max)();
            }
            nResult += record.nSize;
        }
    }

    return nResult;
}

XADDR XBinary::getBaseAddress()
{
    return this->m_nBaseAddress;
}

void XBinary::setBaseAddress(XADDR nBaseAddress)
{
    this->m_nBaseAddress = nBaseAddress;
}

qint64 XBinary::getImageSize()
{
    return getSize();
}

bool XBinary::isImage()
{
    return m_bIsImage;
}

void XBinary::setIsImage(bool bValue)
{
    m_bIsImage = bValue;
}

void XBinary::setMultiSearchCallbackState(bool bState)
{
    m_bMultiSearchCallback = bState;
}

bool XBinary::compareSignature(const QString &sSignature, qint64 nOffset)
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return compareSignature(&memoryMap, sSignature, nOffset);
}

bool XBinary::compareSignature(_MEMORY_MAP *pMemoryMap, const QString &sSignature, qint64 nOffset, PDSTRUCT *pPdStruct)
{
    bool bResult = false;

    QString sOrigin = sSignature;

    QString _sSignature = convertSignature(sSignature);

    bool bValid = true;

    QList<SIGNATURE_RECORD> listSignatureRecords = getSignatureRecords(_sSignature, &bValid, pPdStruct);

    if (bValid && listSignatureRecords.count()) {
        bResult = _compareSignature(pMemoryMap, &listSignatureRecords, nOffset, pPdStruct);
    } else {
        setPdStructErrorString(pPdStruct, QString("%1: %2").arg(tr("Invalid signature")).arg(sOrigin));
    }

    return bResult;
}

bool XBinary::_compareByteArrayWithSignature(const QByteArray &baData, const QString &sSignature)
{
    bool bResult = false;

    const QString sHex = QString::fromLatin1(baData.toHex());
    const QString sNormalizedSignature = convertSignature(sSignature);

    if (!sNormalizedSignature.isEmpty() && isSignatureValid(sNormalizedSignature) && (sHex.size() == sNormalizedSignature.size())) {
        bResult = true;

        qint32 nNumberOfSymbols = sNormalizedSignature.size();

        for (qint32 i = 0; i < nNumberOfSymbols; i++) {
            if (sNormalizedSignature.at(i) != QChar('.')) {
                if (sNormalizedSignature.at(i) != sHex.at(i)) {
                    bResult = false;
                    break;
                }
            }
        }
    }

    return bResult;
}

QString XBinary::_createSignature(const QString &sSignature1, const QString &sSignature2)
{
    QString sResult;

    const QString sNormalizedSignature1 = convertSignature(sSignature1);
    const QString sNormalizedSignature2 = convertSignature(sSignature2);
    const qint32 nSize = sNormalizedSignature1.size();

    if (!sNormalizedSignature1.isEmpty() && (nSize == sNormalizedSignature2.size()) && ((nSize & 1) == 0) &&
        isSignatureValid(sNormalizedSignature1) && isSignatureValid(sNormalizedSignature2)) {
        for (qint32 i = 0; i < nSize; i += 2) {
            if (sNormalizedSignature1.mid(i, 2) == sNormalizedSignature2.mid(i, 2)) {
                sResult.append(sNormalizedSignature1.mid(i, 2));
            } else {
                sResult.append("..");
            }
        }
    }

    return sResult;
}

bool XBinary::compareSignatureOnAddress(const QString &sSignature, XADDR nAddress)
{
    XBinary::_MEMORY_MAP memoryMap = getMemoryMap();

    return compareSignatureOnAddress(&memoryMap, sSignature, nAddress);
}

bool XBinary::compareSignatureOnAddress(XBinary::_MEMORY_MAP *pMemoryMap, const QString &sSignature, XADDR nAddress)
{
    bool bResult = false;

    qint64 nOffset = addressToOffset(pMemoryMap, nAddress);

    if (nOffset != -1) {
        bResult = compareSignature(pMemoryMap, sSignature, nOffset);
    }

    return bResult;
}

qint64 XBinary::_getEntryPointOffset()
{
    XBinary::_MEMORY_MAP memoryMap = getMemoryMap();

    return getEntryPointOffset(&memoryMap);
}

qint64 XBinary::getEntryPointOffset(_MEMORY_MAP *pMemoryMap)
{
    return pMemoryMap ? addressToOffset(pMemoryMap, pMemoryMap->nEntryPointAddress) : -1;
}

void XBinary::setEntryPointOffset(qint64 nEntryPointOffset)
{
    this->m_nEntryPointOffset = nEntryPointOffset;
}

XADDR XBinary::getEntryPointAddress()
{
    XBinary::_MEMORY_MAP memoryMap = getMemoryMap();

    return getEntryPointAddress(&memoryMap);
}

XADDR XBinary::getEntryPointAddress(XBinary::_MEMORY_MAP *pMemoryMap)
{
    return pMemoryMap ? pMemoryMap->nEntryPointAddress : (XADDR)-1;
}

XADDR XBinary::_getEntryPointAddress()
{
    return 0;
}

qint64 XBinary::getEntryPointRVA()
{
    XBinary::_MEMORY_MAP memoryMap = getMemoryMap();

    return getEntryPointRVA(&memoryMap);
}

qint64 XBinary::getEntryPointRVA(_MEMORY_MAP *pMemoryMap)
{
    if (!pMemoryMap || (pMemoryMap->nEntryPointAddress == (XADDR)-1) ||
        (pMemoryMap->nModuleAddress == (XADDR)-1) ||
        (pMemoryMap->nEntryPointAddress < pMemoryMap->nModuleAddress) ||
        ((pMemoryMap->nEntryPointAddress - pMemoryMap->nModuleAddress) > (XADDR)(std::numeric_limits<qint64>::max)())) {
        return -1;
    }

    return (qint64)(pMemoryMap->nEntryPointAddress - pMemoryMap->nModuleAddress);
}

XADDR XBinary::getLowestAddress(XBinary::_MEMORY_MAP *pMemoryMap)
{
    XADDR nResult = -1;

    if (!pMemoryMap) {
        return nResult;
    }

    qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (pMemoryMap->listRecords.at(i).nAddress != (XADDR)-1) {
            if (nResult == (XADDR)-1) {
                nResult = pMemoryMap->listRecords.at(i).nAddress;
            }

            nResult = qMin(pMemoryMap->listRecords.at(i).nAddress, nResult);
        }
    }

    return nResult;
}

qint64 XBinary::getTotalVirtualSize(XBinary::_MEMORY_MAP *pMemoryMap)
{
    qint64 nResult = 0;

    if (!pMemoryMap) {
        return nResult;
    }

    qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        const _MEMORY_RECORD &record = pMemoryMap->listRecords.at(i);
        if ((record.filePart != FILEPART_OVERLAY) && (record.nSize >= 0)) {  // TODO Check ELF, MachO -1
            if (record.nSize > (std::numeric_limits<qint64>::max)() - nResult) {
                return (std::numeric_limits<qint64>::max)();
            }
            nResult += record.nSize;
        }
    }

    return nResult;
}

quint64 XBinary::positionToVirtualAddress(_MEMORY_MAP *pMemoryMap, qint64 nPosition)
{
    XADDR nResult = -1;

    if (!pMemoryMap || (nPosition < 0)) {
        return nResult;
    }

    qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

    qint64 _nSize = 0;

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        const _MEMORY_RECORD &record = pMemoryMap->listRecords.at(i);
        if (record.nSize < 0) {
            return (XADDR)-1;
        }

        if ((_nSize <= nPosition) && ((nPosition - _nSize) < record.nSize)) {
            if (_addRelativeAddress(record.nAddress, nPosition - _nSize, &nResult)) {
                return nResult;
            }
            return (XADDR)-1;
        }

        if (record.nSize > (std::numeric_limits<qint64>::max)() - _nSize) {
            return (XADDR)-1;
        }
        _nSize += record.nSize;
    }

    return nResult;
}

void XBinary::setModuleAddress(quint64 nValue)
{
    this->m_nModuleAddress = nValue;
}

XADDR XBinary::getModuleAddress()
{
    XADDR nResult = 0;

    if (m_nModuleAddress != (XADDR)-1) {
        nResult = m_nModuleAddress;
    } else {
        nResult = getBaseAddress();
    }

    return nResult;
}

bool XBinary::compareEntryPoint(const QString &sSignature, qint64 nOffset)
{
    XBinary::_MEMORY_MAP memoryMap = getMemoryMap();

    return compareEntryPoint(&memoryMap, sSignature, nOffset);
}

bool XBinary::compareEntryPoint(XBinary::_MEMORY_MAP *pMemoryMap, const QString &sSignature, qint64 nOffset)
{
    if (!pMemoryMap) {
        return false;
    }

    const qint64 nBaseOffset = getEntryPointOffset(pMemoryMap);
    if ((nBaseOffset < 0) || (nOffset < 0) || (nBaseOffset > (std::numeric_limits<qint64>::max)() - nOffset)) {
        return false;
    }

    const qint64 nEPOffset = nBaseOffset + nOffset;

    return compareSignature(pMemoryMap, sSignature, nEPOffset);
}

bool XBinary::moveMemory(qint64 nSourceOffset, qint64 nDestOffset, qint64 nSize)
{
    const qint64 nMaxSize = getSize();

    if (!m_pDevice || !m_pDevice->isWritable() || (nMaxSize < 0) ||
        (nSourceOffset < 0) || (nDestOffset < 0) || (nSize < 0) ||
        (nSourceOffset > nMaxSize) || (nDestOffset > nMaxSize) ||
        (nSize > (nMaxSize - nSourceOffset)) || (nSize > (nMaxSize - nDestOffset))) {
        return false;
    }

    if ((nSize == 0) || (nDestOffset == nSourceOffset)) {
        return true;
    }

    const qint64 nDelta = nDestOffset - nSourceOffset;
    const quint32 nBufferSize = (quint32)qMin<qint64>((nDelta > 0) ? nDelta : -nDelta, 0x1000);
    bool bResult = false;

    if (nDelta > 0) {
        bResult = copyMemory(nSourceOffset, nDestOffset, nSize, nBufferSize, true);
        if (bResult) {
            bResult = zeroFill(nSourceOffset, nDelta);
        }
    } else {
        bResult = copyMemory(nSourceOffset, nDestOffset, nSize, nBufferSize, false);
        if (bResult) {
            bResult = zeroFill(nDestOffset + nSize, -nDelta);
        }
    }

    return bResult;
}

bool XBinary::moveMemory(QIODevice *pDevice, qint64 nSourceOffset, qint64 nDestOffset, qint64 nSize)
{
    XBinary binary(pDevice);

    return binary.moveMemory(nSourceOffset, nDestOffset, nSize);
}

XBinary::REMOVE_MEMORY_RESULT XBinary::removeMemoryEx(qint64 nOffset, qint64 nSize)
{
    REMOVE_MEMORY_RESULT result = REMOVE_MEMORY_RESULT_FAILED;
    QIODevice *pOriginalDevice = m_pDevice.data();
    QIODevice *pOriginalData = nullptr;
    QPointer<QBuffer> pMemoryPropertyBuffer;
    bool bRefreshDevice = false;

    {
        QMutexLocker locker(m_pReadWriteMutex);

        if (!m_pDevice || !m_pDevice->isWritable() || m_pDevice->isSequential() || !isResizeEnable(m_pDevice) ||
            (m_pDevice->openMode() & (QIODevice::Append | QIODevice::Text))) {
            return result;
        }

        const qint64 nMaxSize = m_pDevice->size();
        if ((nMaxSize < 0) || (nOffset < 0) || (nSize <= 0) || (nOffset >= nMaxSize) ||
            (nSize > (nMaxSize - nOffset))) {
            return result;
        }

        const qint64 nTailSize = nMaxSize - nOffset - nSize;
        if (!m_pDevice->isReadable()) {
            return result;
        }

        const qint64 nOriginalPosition = m_pDevice->pos();
        if (nOriginalPosition < 0) {
            return result;
        }

        const qint64 nNewSize = nMaxSize - nSize;
        qint64 nNewPosition = nOriginalPosition;
        if (nOriginalPosition > nOffset) {
            nNewPosition = (nOriginalPosition < (nOffset + nSize)) ? nOffset : (nOriginalPosition - nSize);
        }
        nNewPosition = qBound((qint64)0, nNewPosition, nNewSize);

        if (QBuffer *pBuffer = qobject_cast<QBuffer *>(m_pDevice.data())) {
            if ((nMaxSize > (std::numeric_limits<qint32>::max)()) ||
                (nOffset > (std::numeric_limits<qint32>::max)()) ||
                (nSize > (std::numeric_limits<qint32>::max)())) {
                return result;
            }

            QByteArray baCandidate;
            try {
                baCandidate = pBuffer->buffer();
                baCandidate.remove((qint32)nOffset, (qint32)nSize);
            } catch (const std::bad_alloc &) {
                return result;
            }

            if (baCandidate.size() != nNewSize) {
                return result;
            }

            // Commit the detached candidate in one swap. If restoring the
            // mapped position unexpectedly fails, the second swap restores
            // the exact original QByteArray without another allocation.
            pBuffer->buffer().swap(baCandidate);
            if (m_pDevice->seek(nNewPosition)) {
                result = REMOVE_MEMORY_RESULT_OK;
                bRefreshDevice = true;
                if (pBuffer->property("Memory").isValid()) {
                    pMemoryPropertyBuffer = pBuffer;
                }
            } else {
                pBuffer->buffer().swap(baCandidate);
                const bool bPositionRestored = m_pDevice->seek(nOriginalPosition);
                result = bPositionRestored ? REMOVE_MEMORY_RESULT_FAILED_RESTORED
                                           : REMOVE_MEMORY_RESULT_FAILED_CHANGED;
                bRefreshDevice = !bPositionRestored;
            }
        } else {
            QFileDevice *pFileDevice = qobject_cast<QFileDevice *>(m_pDevice.data());
            if (!pFileDevice) {
                return result;
            }

            if (pFileDevice->flush()) {
                const qint64 nAffectedSize = nMaxSize - nOffset;
                pOriginalData = createFileBuffer(nAffectedSize, nullptr);

                bool bStaged = pOriginalData &&
                               copyDeviceMemory(m_pDevice, nOffset, pOriginalData, 0, nAffectedSize, nullptr) &&
                               (pOriginalData->size() == nAffectedSize);
                if (bStaged) {
                    if (QFileDevice *pJournalFile = qobject_cast<QFileDevice *>(pOriginalData)) {
                        bStaged = pJournalFile->flush();
                    }
                }

                if (bStaged) {
                    const bool bShifted = (nTailSize == 0) ||
                                          copyDeviceMemory(m_pDevice, nOffset + nSize, m_pDevice, nOffset, nTailSize, nullptr);
                    const bool bResized = bShifted && pFileDevice->resize(nNewSize);
                    const bool bFlushed = bResized && pFileDevice->flush();
                    const bool bPositionSet = bFlushed && m_pDevice->seek(nNewPosition);

                    if (bPositionSet) {
                        result = REMOVE_MEMORY_RESULT_OK;
                        bRefreshDevice = true;
                    } else {
                        // The journal contains every byte that may have moved
                        // or been truncated. Restore it without honoring a
                        // canceled/failed commit operation.
                        const bool bSizeRestored = pFileDevice->resize(nMaxSize);
                        const bool bDataRestored = bSizeRestored &&
                                                   copyDeviceMemory(pOriginalData, 0, m_pDevice, nOffset, nAffectedSize, nullptr);
                        const bool bRollbackFlushed = bDataRestored && pFileDevice->flush();
                        const bool bPositionRestored = bRollbackFlushed && m_pDevice->seek(nOriginalPosition);

                        result = bPositionRestored ? REMOVE_MEMORY_RESULT_FAILED_RESTORED
                                                   : REMOVE_MEMORY_RESULT_FAILED_CHANGED;
                        bRefreshDevice = !bPositionRestored;
                    }
                } else {
                    if (!m_pDevice->seek(nOriginalPosition)) {
                        result = REMOVE_MEMORY_RESULT_FAILED_CHANGED;
                        bRefreshDevice = true;
                    }
                }
            }
        }
    }

    freeFileBuffer(&pOriginalData);

    if (bRefreshDevice) {
        setDevice(pOriginalDevice);
    }

    if (pMemoryPropertyBuffer) {
        // A swap invalidates any raw QByteArray pointer published through the
        // optional fast path. Dispatch the property event only after releasing
        // the non-recursive device mutex.
        pMemoryPropertyBuffer->setProperty("Memory", QVariant());
    }

    return result;
}

XBinary::REMOVE_MEMORY_RESULT XBinary::removeMemoryEx(QIODevice *pDevice, qint64 nOffset, qint64 nSize)
{
    XBinary binary(pDevice);

    return binary.removeMemoryEx(nOffset, nSize);
}

bool XBinary::removeMemory(qint64 nOffset, qint64 nSize)
{
    return removeMemoryEx(nOffset, nSize) == REMOVE_MEMORY_RESULT_OK;
}

bool XBinary::removeMemory(QIODevice *pDevice, qint64 nOffset, qint64 nSize)
{
    return removeMemoryEx(pDevice, nOffset, nSize) == REMOVE_MEMORY_RESULT_OK;
}

bool XBinary::dumpToFile(const QString &sFileName, const char *pData, qint64 nDataSize)
{
    if ((nDataSize < 0) || ((nDataSize > 0) && !pData)) {
        return false;
    }

    QSaveFile file(sFileName);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    if (!writeAllToDevice(&file, pData, nDataSize)) {
        file.cancelWriting();
        return false;
    }

    return file.commit();
}

bool XBinary::dumpToFile(const QString &sFileName, qint64 nDataOffset, qint64 nDataSize, PDSTRUCT *pPdStruct)
{
    bool bResult = false;

    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    const qint64 nSourceSize = getSize();
    if (!m_pDevice || (nSourceSize < 0) || (nDataOffset < 0) || (nDataOffset > nSourceSize) ||
        (nDataSize < -1) || !isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    if (nDataSize == -1) {
        nDataSize = nSourceSize - nDataOffset;
    }
    if ((nDataSize < 0) || (nDataSize > (nSourceSize - nDataOffset))) {
        return false;
    }

    const qint32 nRequestedBufferSize = getBufferSize(pPdStruct);
    const qint32 nBufferSize = (nRequestedBufferSize > 0)
                                   ? qBound((qint32)0x1000, nRequestedBufferSize, (qint32)0x100000)
                                   : 0;
    if ((nDataSize > 0) && (nBufferSize <= 0)) return false;

    char *pBuffer = nullptr;
    if (nDataSize > 0) {
        pBuffer = new (std::nothrow) char[nBufferSize];
        if (!pBuffer) return false;
    }

    QSaveFile file(sFileName);

    if (file.open(QIODevice::WriteOnly)) {
        qint64 nSourceOffset = nDataOffset;
        qint64 nRemaining = nDataSize;
        qint64 nProcessed = 0;

        qint32 _nFreeIndex = XBinary::reservePdStructRecord(pPdStruct, nDataSize);

        bResult = true;

        while ((nRemaining > 0) && isPdStructNotCanceled(pPdStruct)) {
            const qint64 nTempSize = qMin(nRemaining, (qint64)nBufferSize);

            if (read_array_process(nSourceOffset, pBuffer, nTempSize, pPdStruct) != nTempSize) {
                setPdStructErrorString(pPdStruct, tr("Read error"));
                bResult = false;
                break;
            }

            if (!writeAllToDevice(&file, pBuffer, nTempSize)) {
                setPdStructErrorString(pPdStruct, tr("Write error"));
                bResult = false;
                break;
            }

            nSourceOffset += nTempSize;
            nProcessed += nTempSize;
            nRemaining -= nTempSize;

            XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, nProcessed);
        }

        XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);

        bResult = bResult && (nRemaining == 0) && isPdStructNotCanceled(pPdStruct);
        if (bResult) {
            bResult = file.commit();
        } else {
            file.cancelWriting();
        }
    } else {
        _errorMessage(QString("%1: %2").arg(QObject::tr("Cannot open file")).arg(sFileName));
    }

    delete[] pBuffer;

    return bResult;
}

bool XBinary::dumpToFile(const QString &sFileName, QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    QPointer<QIODevice> guardedDevice(pDevice);
    if (!guardedDevice) {
        return false;
    }
    const qint64 nDeviceSize = guardedDevice->size();
    if (!guardedDevice || (nDeviceSize < 0)) return false;

    XBinary binary(guardedDevice.data());
    if (!guardedDevice) return false;

    const bool bResult = binary.dumpToFile(
        sFileName, 0, nDeviceSize, pPdStruct);
    return guardedDevice && bResult;
}

bool XBinary::patchFromFile(const QString &sFileName, qint64 nDataOffset, qint64 nDataSize, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    if (!m_pDevice || !m_pDevice->isWritable() || (nDataOffset < 0) || !isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    // A second QFile object hides same-device overlap from copyDeviceMemory.
    // Reject aliases up front so a forward patch cannot overwrite source data
    // that a later chunk still has to read.
    const QFile *pDestinationFile = dynamic_cast<const QFile *>(m_pDevice.data());

    if (pDestinationFile) {
        const QFileInfo destinationInfo(pDestinationFile->fileName());
        const QFileInfo sourceInfo(sFileName);
        QString sDestinationPath = destinationInfo.canonicalFilePath();
        QString sSourcePath = sourceInfo.canonicalFilePath();

        if (sDestinationPath.isEmpty()) {
            sDestinationPath = QDir::cleanPath(destinationInfo.absoluteFilePath());
        }
        if (sSourcePath.isEmpty()) {
            sSourcePath = QDir::cleanPath(sourceInfo.absoluteFilePath());
        }

#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
        const Qt::CaseSensitivity caseSensitivity = Qt::CaseInsensitive;
#else
        const Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive;
#endif

        if (!sDestinationPath.isEmpty() && !sSourcePath.isEmpty() &&
            (QString::compare(QDir::fromNativeSeparators(sDestinationPath), QDir::fromNativeSeparators(sSourcePath), caseSensitivity) == 0)) {
            return false;
        }
    }

    if (nDataSize == -1) {
        const qint64 nDestinationSize = getSize();

        if ((nDestinationSize < 0) || (nDataOffset > nDestinationSize)) {
            return false;
        }

        nDataSize = nDestinationSize - nDataOffset;
    }

    const qint64 nDestinationSize = getSize();

    if ((nDataSize < 0) || (nDestinationSize < 0) || (nDataOffset > nDestinationSize) ||
        (nDataSize > nDestinationSize - nDataOffset)) {
        return false;
    }

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        bool bSameOpenFile = false;

        if (pDestinationFile && (pDestinationFile->handle() >= 0) && (file.handle() >= 0)) {
#ifdef Q_OS_WIN
            const intptr_t nDestinationHandle = _get_osfhandle(pDestinationFile->handle());
            const intptr_t nSourceHandle = _get_osfhandle(file.handle());

            if ((nDestinationHandle != -1) && (nSourceHandle != -1)) {
                BY_HANDLE_FILE_INFORMATION destinationFileInformation = {};
                BY_HANDLE_FILE_INFORMATION sourceFileInformation = {};

                if (GetFileInformationByHandle(reinterpret_cast<HANDLE>(nDestinationHandle), &destinationFileInformation) &&
                    GetFileInformationByHandle(reinterpret_cast<HANDLE>(nSourceHandle), &sourceFileInformation)) {
                    bSameOpenFile = (destinationFileInformation.dwVolumeSerialNumber == sourceFileInformation.dwVolumeSerialNumber) &&
                                    (destinationFileInformation.nFileIndexHigh == sourceFileInformation.nFileIndexHigh) &&
                                    (destinationFileInformation.nFileIndexLow == sourceFileInformation.nFileIndexLow);
                }
            }
#elif defined(Q_OS_UNIX)
            struct stat destinationFileStatus = {};
            struct stat sourceFileStatus = {};

            if ((fstat(pDestinationFile->handle(), &destinationFileStatus) == 0) && (fstat(file.handle(), &sourceFileStatus) == 0)) {
                bSameOpenFile = (destinationFileStatus.st_dev == sourceFileStatus.st_dev) &&
                                (destinationFileStatus.st_ino == sourceFileStatus.st_ino);
            }
#endif
        }

        if (bSameOpenFile) {
            file.close();
            return false;
        }

        const bool bResult = (file.size() >= nDataSize) && copyDeviceMemory(&file, 0, m_pDevice, nDataOffset, nDataSize, pPdStruct);
        file.close();
        return bResult;
    }

    _errorMessage(QString("%1: %2").arg(QObject::tr("Cannot open file")).arg(sFileName));

    return false;
}

QSet<XBinary::FT> XBinary::getFileTypes(bool bExtra)
{
    QSet<XBinary::FT> stResult;

    stResult.insert(FT_BINARY);

    QByteArray baHeader;
    QByteArray baNewHeader;
    baHeader = read_array(0, qMin(getSize(), (qint64)0x200));  // TODO const
    char *pOffset = baHeader.data();
    qint64 nSize = getSize();
    bool bAllFound = false;

    if (nSize >= (qint64)sizeof(XMSDOS_DEF::IMAGE_DOS_HEADEREX)) {
        if ((_read_uint16(pOffset) == XMSDOS_DEF::S_IMAGE_DOS_SIGNATURE_MZ) || (_read_uint16(pOffset) == XMSDOS_DEF::S_IMAGE_DOS_SIGNATURE_ZM)) {
            stResult.insert(FT_MSDOS);
            // TODO rewrite for NE, LE
            quint32 nLfanew = _read_uint32(pOffset + offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_lfanew));
            quint32 nHeaderSize = (quint32)baHeader.size() - sizeof(XPE_DEF::IMAGE_NT_HEADERS32);

            bool bIsNewHeaderValid = false;

            if ((nLfanew < nHeaderSize) && ((quint32)baHeader.size() > (nLfanew + sizeof(XPE_DEF::IMAGE_NT_HEADERS32))))  // TODO do not use
                                                                                                                          // IMAGE_NT_HEADERS32
            {
                pOffset += nLfanew;
                bIsNewHeaderValid = true;
            } else {
                qint64 nNtHeadersSize = 4 + sizeof(XPE_DEF::IMAGE_FILE_HEADER);

                baNewHeader = read_array(nLfanew, nNtHeadersSize);

                nHeaderSize = baNewHeader.size();

                if (nHeaderSize == nNtHeadersSize) {
                    pOffset = baNewHeader.data();
                    bIsNewHeaderValid = true;
                }
            }

            if (bIsNewHeaderValid) {
                bIsNewHeaderValid = false;

                if (_read_uint32(pOffset) == XPE_DEF::S_IMAGE_NT_SIGNATURE) {
                    stResult.insert(FT_PE);

                    quint16 nMachine = _read_uint32(pOffset + 4 + offsetof(XPE_DEF::IMAGE_FILE_HEADER, Machine));

                    // TODO more
                    if ((nMachine == XPE_DEF::S_IMAGE_FILE_MACHINE_AMD64) || (nMachine == XPE_DEF::S_IMAGE_FILE_MACHINE_IA64) ||
                        (nMachine == XPE_DEF::S_IMAGE_FILE_MACHINE_ARM64) || (nMachine == XPE_DEF::S_IMAGE_FILE_MACHINE_ALPHA64) ||
                        (nMachine == XPE_DEF::S_IMAGE_FILE_MACHINE_RISCV64) || (nMachine == XPE_DEF::S_IMAGE_FILE_MACHINE_LOONGARCH64)) {
                        stResult.insert(FT_PE64);
                    } else {
                        stResult.insert(FT_PE32);
                    }

                    // .NET / CLI assembly: check the COM(CLR) descriptor data directory
                    {
                        quint16 nOptMagic = read_uint16(nLfanew + 24);

                        qint64 nDataDirectoryOffset = -1;
                        quint32 nNumberOfRvaAndSizes = 0;

                        if (nOptMagic == 0x10B) {  // PE32
                            nNumberOfRvaAndSizes = read_uint32(nLfanew + 24 + 92);
                            nDataDirectoryOffset = nLfanew + 24 + 96;
                        } else if (nOptMagic == 0x20B) {  // PE32+
                            nNumberOfRvaAndSizes = read_uint32(nLfanew + 24 + 108);
                            nDataDirectoryOffset = nLfanew + 24 + 112;
                        }

                        if ((nDataDirectoryOffset != -1) && (nNumberOfRvaAndSizes > 14)) {  // IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR
                            quint32 nCliRva = read_uint32(nDataDirectoryOffset + 14 * 8);
                            quint32 nCliSize = read_uint32(nDataDirectoryOffset + 14 * 8 + 4);

                            if (nCliRva && nCliSize) {
                                stResult.insert(FT_CLI_ASSEMBLY);
                            }
                        }
                    }

                    bIsNewHeaderValid = true;
                } else if (_read_uint16(pOffset) == XNE_DEF::S_IMAGE_OS2_SIGNATURE) {
                    stResult.insert(FT_NE);
                    bIsNewHeaderValid = true;
                } else if (_read_uint32(pOffset) == XLE_DEF::S_IMAGE_VXD_SIGNATURE) {
                    stResult.insert(FT_LE);
                    bIsNewHeaderValid = true;
                } else if (_read_uint32(pOffset) == XLE_DEF::S_IMAGE_LX_SIGNATURE) {
                    stResult.insert(FT_LX);
                    bIsNewHeaderValid = true;
                }
            }

            if (!bIsNewHeaderValid) {
                quint16 nCP = read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_cp));
                quint16 nCblp = read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_cblp));

                if (nCP > 0) {
                    qint64 nSignatureOffset = (nCP - 1) * 512 + nCblp;
                    if (nSize - nSignatureOffset > 0) {
                        bool bBW = false;
                        bool b16M = false;
                        bool b4G = false;
                        while (true) {
                            quint16 nSignature = read_uint16(nSignatureOffset);

                            if (nSignature == 0x5742) {  // BW
                                bBW = true;
                                b16M = true;
                                nSignatureOffset = read_uint32(nSignatureOffset + offsetof(XMSDOS_DEF::dos16m_exe_header, next_header_pos));
                            } else if (nSignature == 0x464D) {  // MF - find info
                                nSignatureOffset += read_uint32(nSignatureOffset + 2);
                            } else if (nSignature == 0x5A4D) {  // MZ
                                qint64 nSignatureOffsetOpt = read_uint32(nSignatureOffset + offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_lfanew));
                                quint16 nSignatureOpt = read_uint16(nSignatureOffsetOpt + nSignatureOffset);

                                if (nSignatureOpt == 0x454E) {  // NE
                                    b16M = true;
                                } else if (nSignatureOpt == 0x454C) {  // LE
                                    b4G = true;
                                } else if (nSignatureOpt == 0x584C) {  // LX
                                    b4G = true;
                                }
                                break;
                            } else {
                                break;
                            }
                        }

                        if (bBW && b4G) {
                            stResult.insert(FT_ARCHIVE);
                            stResult.insert(FT_DOS4G);
                        } else if (bBW && b16M) {
                            stResult.insert(FT_ARCHIVE);
                            stResult.insert(FT_DOS16M);
                        }
                    }
                }
            }

            bAllFound = true;
        }
    }

    if ((!bAllFound) && (nSize >= (qint64)sizeof(XELF_DEF::Elf32_Ehdr))) {
        if ((((XELF_DEF::Elf32_Ehdr *)pOffset)->e_ident[0] == 0x7f) && (((XELF_DEF::Elf32_Ehdr *)pOffset)->e_ident[1] == 'E') &&
            (((XELF_DEF::Elf32_Ehdr *)pOffset)->e_ident[2] == 'L') && (((XELF_DEF::Elf32_Ehdr *)pOffset)->e_ident[3] == 'F')) {
            stResult.insert(FT_ELF);

            if (((XELF_DEF::Elf32_Ehdr *)pOffset)->e_ident[4] == 1) {
                stResult.insert(FT_ELF32);
            } else if (((XELF_DEF::Elf32_Ehdr *)pOffset)->e_ident[4] == 2) {
                stResult.insert(FT_ELF64);
            }
            // mb TODO another e_ident[4]
            bAllFound = true;
        }
    }

    if ((!bAllFound) && (nSize >= (qint64)sizeof(XMACH_DEF::mach_header))) {
        bool bMach = false;
        bool bBE = false;
        bool b64 = false;
        if (_read_uint32(pOffset) == XMACH_DEF::S_MH_MAGIC) {
            bMach = true;
            bBE = false;
            b64 = false;
        } else if (_read_uint32(pOffset) == XMACH_DEF::S_MH_CIGAM) {
            bMach = true;
            bBE = true;
            b64 = false;
        } else if (_read_uint32(pOffset) == XMACH_DEF::S_MH_MAGIC_64) {
            bMach = true;
            bBE = false;
            b64 = true;
        } else if (_read_uint32(pOffset) == XMACH_DEF::S_MH_CIGAM_64) {
            bMach = true;
            bBE = true;
            b64 = true;
        }

        if (bMach) {
            if (_read_uint32(pOffset + 0x0C, bBE) < 20) {
                stResult.insert(FT_MACHO);
                if (b64) {
                    stResult.insert(FT_MACHO64);
                } else {
                    stResult.insert(FT_MACHO32);
                }
                bAllFound = true;
            }
        }
    }

    if ((!bAllFound) && (nSize >= 8)) {
        quint32 nMagic = _read_uint32(pOffset);
        if ((nMagic == 0xf3030000) || (nMagic == 0xe7030000)) {
            stResult.insert(FT_AMIGAHUNK);

            bAllFound = true;
        }
    }

    if ((!bAllFound) && (nSize >= 28)) {
        quint16 nMagic = _read_uint16(pOffset, true);  // Big-endian
        if (nMagic == 0x601A) {
            stResult.insert(FT_ATARIST);

            bAllFound = true;
        }
    }

    if ((!bAllFound) && bExtra) {
        _MEMORY_MAP memoryMap = XBinary::getSimpleMemoryMap();
        UNICODE_TYPE unicodeType = getUnicodeType(&baHeader);

        bAllFound = true;

        if (compareSignature(&memoryMap, "'PK'0304", 0) || compareSignature(&memoryMap, "'PK'0506", 0))  // TODO baHeader
        {
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_ZIP);
            // TODO Check APK, JAR
            // TODO basic ZIP
        } else if (compareSignature(&memoryMap, "1F8B08")) {
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_GZIP);
        } else if (compareSignature(&memoryMap, "7801") || compareSignature(&memoryMap, "785E") || compareSignature(&memoryMap, "789C") ||
                   compareSignature(&memoryMap, "78DA")) {
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_ZLIB);
        } else if (compareSignature(&memoryMap, "....'-lh'..2d") || compareSignature(&memoryMap, "....'-lz'..2d") || compareSignature(&memoryMap, "....'-pm'..2d")) {
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_LHA);
        } else if (compareSignature(&memoryMap, "'!<arch>'0a")) {
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_AR);
            // TODO DEB
        } else if ((memoryMap.nBinarySize >= 0x200) && compareSignature(&memoryMap, "00'ustar'", 0x100)) {
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_TAR);
        } else if (compareSignature(&memoryMap, "'RE~^'") || compareSignature(&memoryMap, "'Rar!'1A07")) {
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_RAR);
        } else if (compareSignature(&memoryMap, "'MSCF'00000000", 0)) {
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_CAB);
        } else if (compareSignature(&memoryMap, "'7z'BCAF271C", 0)) {
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_7Z);
        } else if (compareSignature(&memoryMap, "'MSWIM'000000", 0)) {
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_WIM);
        } else if (compareSignature(&memoryMap, "'LZIP'", 0)) {
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_LZIP);
        } else if (compareSignature(&memoryMap, "894C5A4F000D0A1A0A", 0)) {
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_LZO);
        } else if (compareSignature(&memoryMap, "04224D18", 0)) {
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_LZ4);
        } else if (compareSignature(&memoryMap, "5D000000", 0)) {
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_LZMA);
        } else if (compareSignature(&memoryMap, "1F9D", 0)) {
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_COMPRESS);
        } else if (compareSignature(&memoryMap, "303730373031", 0) || compareSignature(&memoryMap, "303730373032", 0) ||
                   compareSignature(&memoryMap, "303730373037", 0) ||
                   ((nSize >= 26) && (((read_uint16(0) == 0x71C7) && (read_uint16(20) > 0) && (read_uint16(20) < 0x1000)) ||
                                      ((read_uint16(0, true) == 0x71C7) && (read_uint16(20, true) > 0) && (read_uint16(20, true) < 0x1000))))) {
            // CPIO formats: 070701, 070702, 070707, binary little-endian, binary big-endian
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_CPIO);
        } else if (compareSignature(&memoryMap, "'MDMP'", 0)) {
            // Windows MiniDump format
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_MINIDUMP);
        } else if (compareSignature(&memoryMap, "'koly'", nSize - 512) ||
                   ((nSize > 512) && compareSignature(&memoryMap, "'koly'0000000400000200", 0))) {
            // Apple Disk Image format (usual terminal KOLY or rare legacy front KOLY)
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_DMG);
        } else if (compareSignature(&memoryMap, "60EA", 0) && nSize >= 34) {
            // ARJ format: 0x60 0xEA marker, basic_header_size >= 30, first_hdr_size >= 30
            // Note: ARJ 2.50+ uses first_hdr_size=34; older versions use 30
            quint16 _nArjHdrSize = read_uint16(2, false);
            quint8 _nArjFirstHdr = read_uint8(4);

            if ((_nArjHdrSize >= 30) && (_nArjHdrSize <= 2600) && (_nArjFirstHdr >= 30)) {
                stResult.insert(FT_ARCHIVE);
                stResult.insert(FT_ARJ);
            }
        } else if (nSize >= 14) {
            // ACE format: head_type=0 at offset 4, magic "**ACE**" at offset 7
            quint8 _nAceHeadType = read_uint8(4);
            quint16 _nAceHeadSize = read_uint16(2, false);

            if ((_nAceHeadType == 0) && (_nAceHeadSize >= 10)) {
                if (compareSignature(&memoryMap, "'**ACE**'", 7)) {
                    stResult.insert(FT_ARCHIVE);
                    stResult.insert(FT_ACE);
                }
            }
        } else if (nSize >= 29) {
            // ARC format: 0x1A + method(1-9) + filename(13 bytes null-terminated, first char printable ASCII 0x21-0x7E)
            quint8 _nArcMarker = read_uint8(0);
            quint8 _nArcMethod = read_uint8(1);
            quint8 _nArcFirstChar = read_uint8(2);

            if ((_nArcMarker == 0x1A) && (_nArcMethod >= 1) && (_nArcMethod <= 9) && (_nArcFirstChar >= 0x21) && (_nArcFirstChar <= 0x7E)) {
                stResult.insert(FT_ARCHIVE);
                stResult.insert(FT_ARC);
            }
        }

        // FreeARC format: "ArC\x01" signature at offset 0, second "ArC\x01" at offset 8
        if (compareSignature(&memoryMap, "'ArC'01", 0) && (nSize >= 12)) {
            quint32 _nFreeArcBlock = read_uint32(8, false);

            if (_nFreeArcBlock == 0x01437241) {
                stResult.insert(FT_ARCHIVE);
                stResult.insert(FT_FREEARC);
            }
        }

        if (compareSignature(&memoryMap, "89'PNG\r\n'1A0A", 0)) {
            stResult.insert(FT_IMAGE);
            stResult.insert(FT_PNG);
        } else if (compareSignature(&memoryMap, "FFD8FFE0....'JFIF'00", 0) || compareSignature(&memoryMap, "FFD8FFE1....'Exif'00", 0) ||
                   compareSignature(&memoryMap, "FFD8FFDB", 0)) {
            stResult.insert(FT_IMAGE);
            stResult.insert(FT_JPEG);
        } else if (compareSignature(&memoryMap, "'GIF87a'", 0) || compareSignature(&memoryMap, "'GIF89a'", 0)) {
            stResult.insert(FT_IMAGE);
            stResult.insert(FT_GIF);
        } else if (compareSignature(&memoryMap, "'BM'..................000000", 0)) {
            stResult.insert(FT_IMAGE);
            stResult.insert(FT_BMP);
        } else if (compareSignature(&memoryMap, "'MM'002A", 0) || compareSignature(&memoryMap, "'II'2A00", 0)) {
            stResult.insert(FT_IMAGE);
            stResult.insert(FT_TIFF);
        } else if (compareSignature(&memoryMap, "00000100", 0)) {
            stResult.insert(FT_IMAGE);
            stResult.insert(FT_ICO);
        } else if (compareSignature(&memoryMap, "00000200", 0)) {
            stResult.insert(FT_IMAGE);
            stResult.insert(FT_CUR);
        } else if (compareSignature(&memoryMap, "........................'mntr'", 0)) {
            stResult.insert(FT_IMAGE);
            stResult.insert(FT_ICC);
        }

        if (compareSignature(&memoryMap, "4344303031", 0x8001, 0)) {  // "CD001" at offset 0x8001
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_ISO9660);
        } else if (compareSignature(&memoryMap, "0002", 256 * 2048, 0)) {  // UDF Anchor at sector 256
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_UDF);
        } else if (compareSignature(&memoryMap, "'ID3'..00", 0)) {
            stResult.insert(FT_AUDIO);
            stResult.insert(FT_MP3);
        } else if (compareSignature(&memoryMap, "000000..'ftyp'", 0)) {
            stResult.insert(FT_VIDEO);
            stResult.insert(FT_MP4);
        } else if (compareSignature(&memoryMap, "'Extended Module'", 0)) {
            stResult.insert(FT_AUDIO);
            stResult.insert(FT_XM);
        } else if (compareSignature(&memoryMap, "'dex\n'......00")) {
            stResult.insert(FT_DEX);
        } else if (compareSignature(&memoryMap, "00000800........0100") || compareSignature(&memoryMap, "03000800........0100")) {
            stResult.insert(FT_ANDROIDXML);
        } else if (compareSignature(&memoryMap, "02000C00........0100")) {
            stResult.insert(FT_ANDROIDASRC);
        } else if (compareSignature(&memoryMap, "'%PDF'", 0)) {
            stResult.insert(FT_DOCUMENT);
            stResult.insert(FT_PDF);
        } else if (compareSignature(&memoryMap, "'Microsoft C/C++ MSF 7.00'0D0A1A'DS'000000", 0) ||
                   compareSignature(&memoryMap, "'Microsoft C/C++ program database 2.00'0D0A1A'JG'0000", 0) ||
                   compareSignature(&memoryMap, "'BSJB'............'PDB v1.0'", 0)) {
            // The last one is a portable PDB: ECMA-335 metadata carrying debug information
            stResult.insert(FT_PDB);
        } else if ((compareSignature(&memoryMap, "30", 0)) && (nSize >= 4)) {
            // Minimal DER/ASN.1 check: first byte is a tag, second is definite length short form (<0x80)
            // or long form (>=0x80) followed by that many length bytes; ensure it fits into the file.
            // quint8 nTag = _read_uint8(pOffset);
            PACKED_UINT packedLen = _read_acn1_integer(pOffset + 1, nSize - 1);

            if ((packedLen.bIsValid) && (packedLen.nByteSize > 0) && (1 + packedLen.nByteSize + packedLen.nValue <= (quint64)nSize)) {
                bool bDer = false;

                if (_read_uint8(pOffset + 1 + packedLen.nByteSize) == 0x06) {
                    // OID
                    bDer = true;
                }

                if (bDer) {
                    stResult.insert(FT_DOCUMENT);
                    stResult.insert(FT_DER);
                } else {
                    bAllFound = false;
                }
            }
        } else if (compareSignature(&memoryMap, "'RIFF'", 0) || compareSignature(&memoryMap, "'RIFX'", 0)) {
            // TODO AIFF
            stResult.insert(FT_RIFF);
            if (compareSignature(&memoryMap, "'RIFF'........'AVI '", 0)) {
                stResult.insert(FT_VIDEO);
                stResult.insert(FT_AVI);
            } else if (compareSignature(&memoryMap, "'RIFF'........'WEBPVP8'", 0)) {
                stResult.insert(FT_IMAGE);
                stResult.insert(FT_WEBP);
            } else if (compareSignature(&memoryMap, "'RIFF'........'WAVE'", 0)) {
                stResult.insert(FT_AUDIO);
                stResult.insert(FT_WAV);
            } else if (compareSignature(&memoryMap, "'RIFF'........'PAL '", 0)) {
                stResult.insert(FT_IMAGE);
                stResult.insert(FT_PAL);
            } /*else if (compareSignature(&memoryMap, "'RIFF'........'ACON'", 0)) {
                stResult.insert(FT_IMAGE);
                stResult.insert(FT_ANI);
            }*/
        } else if (compareSignature(&memoryMap, "'BW'....00..00000000", 0)) {
            stResult.insert(FT_BWDOS16M);
        } else if (compareSignature(&memoryMap, "D0CF11E0A1B11AE1", 0)) {
            stResult.insert(FT_CFBF);
        } else if (compareSignature(&memoryMap, "'OTTO'00", 0) || compareSignature(&memoryMap, "0001000000", 0)) {
            stResult.insert(FT_TTF);
        } else if (compareSignature(&memoryMap, "'AT&TFORM'", 0) || compareSignature(&memoryMap, "'SDJVFORM'", 0)) {
            stResult.insert(FT_DJVU);
        } else if (compareSignature(&memoryMap, "'SZDD'88F027'3A'", 0)) {
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_SZDD);
        } else if (compareSignature(&memoryMap, "'BZh'..314159265359", 0) || compareSignature(&memoryMap, "'BZh'..17724538509000000000")) {
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_BZIP2);
        } else if (compareSignature(&memoryMap, "04224D18", 0)) {
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_LZ4);
        } else if (compareSignature(&memoryMap, "5D000000", 0)) {
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_LZMA);
        } else if (compareSignature(&memoryMap, "28B52FFD", 0)) {
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_ZSTD);
        } else if (compareSignature(&memoryMap, "FD'7zXZ'00", 0)) {
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_XZ);
        } else {
            bAllFound = false;
        }

        if (!bAllFound) {
            if (nSize >= (qint64)sizeof(XMACH_DEF::fat_header) + (qint64)sizeof(XMACH_DEF::fat_arch)) {
                if (read_uint32(0, true) == XMACH_DEF::S_FAT_MAGIC) {
                    if (read_uint32(4, true) < 10) {
                        stResult.insert(FT_ARCHIVE);
                        stResult.insert(FT_MACHOFAT);
                        bAllFound = true;
                    }
                } else if (read_uint32(0, false) == XMACH_DEF::S_FAT_MAGIC) {
                    if (read_uint32(4, false) < 10) {
                        stResult.insert(FT_ARCHIVE);
                        stResult.insert(FT_MACHOFAT);
                        bAllFound = true;
                    }
                }
            }
        }

        if (!bAllFound) {
            if (nSize >= 24) {
                if (read_uint32(0, true) == 0xCAFEBABE) {
                    if (read_uint32(4, true) > 10) {
                        stResult.insert(FT_JAVACLASS);
                        bAllFound = true;
                    }
                }
            }
        }

        if (!bAllFound) {
            if (nSize >= 12) {
                if (read_uint16(2) == 0x0A0D) {
                    // XPYC validation check
                    quint16 nMagic = read_uint16(0);
                    quint16 nCRLF = read_uint16(2);

                    // Python bytecode magic numbers (partial list, see XPYC for complete list)
                    // Format: first 2 bytes = magic, next 2 bytes = 0x0A0D (CRLF)
                    bool bIsPYC = false;
                    if (nCRLF == 0x0A0D) {
                        // Check for common Python magic numbers
                        if ((nMagic >= 0x0A0D) && (nMagic <= 0xFFFF)) {
                            // Basic validation: magic number present and CRLF marker
                            bIsPYC = true;
                        }
                    }

                    if (bIsPYC) {
                        stResult.insert(FT_PYC);
                        bAllFound = true;
                    }
                }
            }
        }

        if (isPlainTextType(&baHeader)) {
            stResult.insert(FT_TEXT);
            stResult.insert(FT_PLAINTEXT);
        } else if (isUTF8TextType(&baHeader)) {
            stResult.insert(FT_TEXT);
            stResult.insert(FT_UTF8);
        } else if (unicodeType != UNICODE_TYPE_NONE) {
            stResult.insert(FT_TEXT);
            stResult.insert(FT_UNICODE);

            if (unicodeType == UNICODE_TYPE_LE) {
                stResult.insert(FT_UNICODE_LE);
            } else {
                stResult.insert(FT_UNICODE_BE);
            }
        }
        // TODO more
        // TODO MIME

        // Fix
        if (stResult.contains(FT_GIF) && stResult.contains(FT_TEXT)) {
            stResult.remove(FT_GIF);
        }
    }

    if ((stResult.count() <= 1) || (stResult.contains(FT_PLAINTEXT))) {
        if ((nSize >= 0) && (nSize <= (0x10000 - 0x100))) {
            if (getDeviceFileSuffix(getDevice()).toUpper() == "COM") {
                stResult.insert(FT_COM);
            }
        }
    }

    return stResult;
}

QSet<XBinary::FT> XBinary::getFileTypes(QIODevice *pDevice, bool bExtra)
{
    XBinary _binary(pDevice);

    return _binary.getFileTypes(bExtra);
}

QSet<XBinary::FT> XBinary::getFileTypes(const QString &sFileName, bool bExtra)
{
    QSet<XBinary::FT> result;

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        XBinary _binary(&file);

        result = _binary.getFileTypes(bExtra);

        file.close();
    }

    return result;
}

QSet<XBinary::FT> XBinary::getFileTypes(QByteArray *pbaData, bool bExtra)
{
    QSet<XBinary::FT> result;

    QBuffer buffer;

    buffer.setBuffer(pbaData);

    if (buffer.open(QIODevice::ReadOnly)) {
        XBinary _binary(&buffer);

        result = _binary.getFileTypes(bExtra);

        buffer.close();
    }

    return result;
}

XBinary::FT XBinary::_getPrefFileType(const QSet<FT> *pStFileTypes)
{
    if (!pStFileTypes) {
        return FT_UNKNOWN;
    }

    if (!pStFileTypes) {
        return FT_UNKNOWN;
    }

    static const XBinary::FT g_arrPrefFileTypeOrder[] = {
        // Executables
        FT_PE64,
        FT_PE32,
        FT_MACHOFAT,
        FT_MACHO64,
        FT_MACHO32,
        FT_ELF64,
        FT_ELF32,
        FT_LE,
        FT_LX,
        FT_NE,
        FT_AMIGAHUNK,
        FT_ATARIST,
        FT_BWDOS16M,
        FT_DOS16M,
        FT_DOS4G,
        FT_MSDOS,

        // Android/Java ecosystems and archives
        FT_APKS,
        FT_APK,
        FT_IPA,
        FT_JAR,
        FT_ZIP,
        FT_NPM,
        FT_TAR_GZ,
        FT_TAR_BZIP2,
        FT_TAR_LZIP,
        FT_TAR_LZMA,
        FT_TAR_LZOP,
        FT_TAR_XZ,
        FT_TAR_Z,
        FT_TAR_ZSTD,
        FT_TAR,
        FT_GZIP,
        FT_ZLIB,
        FT_7Z,
        FT_RAR,
        FT_LHA,
        FT_ARJ,
        FT_ACE,
        FT_BROTLI,
        FT_LZ4,
        FT_LZMA,
        FT_ARC,
        FT_FREEARC,
        FT_DEB,
        FT_AR,
        FT_CAB,
        FT_CPIO,
        FT_ISO9660,
        FT_MINIDUMP,
        FT_DMG,
        FT_WIM,
        FT_RPM,
        FT_XAR,
        FT_ZOO,
        FT_KWAJ,
        FT_ASAR,

        // Android resources and bytecode
        FT_ANDROIDXML,
        FT_ANDROIDASRC,
        FT_DEX,
        FT_JAVACLASS,
        FT_PYC,

        // Documents and container formats
        FT_PDF,
        FT_DER,
        FT_CFBF,
        FT_PDB,

        // Compressed/pack formats
        FT_SZDD,
        FT_BZIP2,
        FT_BROTLI,
        FT_LZ4,
        FT_LZMA,
        FT_ZSTD,
        FT_XZ,
        FT_LZIP,
        FT_LZO,
        FT_COMPRESS,

        // Fonts and images/media
        FT_TTF,
        FT_PNG,
        FT_JPEG,
        FT_WEBP,
        FT_BMP,
        FT_GIF,
        FT_TIFF,
        FT_ICO,
        FT_CUR,
        FT_ICC,
        FT_PAL,
        FT_MP4,
        FT_AVI,
        FT_MP3,
        FT_WAV,
        FT_XM,
        FT_RIFF,

        // Special
        FT_SIGNATURE,
        FT_DJVU,
        FT_COM,

        // Text/encoding
        FT_UNICODE,
        FT_UTF8,
        FT_TEXT,

        // Generic
        FT_DATA,
        FT_BINARY,
    };

    const qint32 nNumberOfTypes = sizeof(g_arrPrefFileTypeOrder) / sizeof(XBinary::FT);

    for (qint32 i = 0; i < nNumberOfTypes; i++) {
        if (pStFileTypes->contains(g_arrPrefFileTypeOrder[i])) {
            return g_arrPrefFileTypeOrder[i];
        }
    }

    return FT_UNKNOWN;
}

QList<XBinary::FT> XBinary::_getFileTypeListFromSet(const QSet<FT> &stFileTypes, TL_OPTION tlOption)
{
    QList<XBinary::FT> listResult;

    // TODO optimize !
    if ((tlOption == TL_OPTION_DEFAULT) || (tlOption == TL_OPTION_EXECUTABLE) || (tlOption == TL_OPTION_ALL)) {
        if (stFileTypes.contains(FT_REGION)) listResult.append(FT_REGION);
        if (stFileTypes.contains(FT_DATA)) listResult.append(FT_DATA);
    }

    if ((tlOption == TL_OPTION_DEFAULT) || (tlOption == TL_OPTION_ALL)) {
        if (stFileTypes.contains(FT_BINARY)) listResult.append(FT_BINARY);
    }

    if ((tlOption == TL_OPTION_DEFAULT) || (tlOption == TL_OPTION_EXECUTABLE) || (tlOption == TL_OPTION_ALL)) {
        if (stFileTypes.contains(FT_COM)) listResult.append(FT_COM);
    }

    if (tlOption == TL_OPTION_ALL) {
        if (stFileTypes.contains(FT_ARCHIVE)) listResult.append(FT_ARCHIVE);
    }

    if ((tlOption == TL_OPTION_DEFAULT) || (tlOption == TL_OPTION_ALL)) {
        if (stFileTypes.contains(FT_BINARY16)) listResult.append(FT_BINARY16);
        if (stFileTypes.contains(FT_BINARY32)) listResult.append(FT_BINARY32);
        if (stFileTypes.contains(FT_BINARY64)) listResult.append(FT_BINARY64);
        if (stFileTypes.contains(FT_ZIP)) listResult.append(FT_ZIP);
        if (stFileTypes.contains(FT_GZIP)) listResult.append(FT_GZIP);
        if (stFileTypes.contains(FT_ZLIB)) listResult.append(FT_ZLIB);
        if (stFileTypes.contains(FT_LHA)) listResult.append(FT_LHA);
        if (stFileTypes.contains(FT_ARJ)) listResult.append(FT_ARJ);
        if (stFileTypes.contains(FT_ACE)) listResult.append(FT_ACE);
        if (stFileTypes.contains(FT_ARC)) listResult.append(FT_ARC);
        if (stFileTypes.contains(FT_FREEARC)) listResult.append(FT_FREEARC);
        if (stFileTypes.contains(FT_RAR)) listResult.append(FT_RAR);
        if (stFileTypes.contains(FT_CAB)) listResult.append(FT_CAB);
        if (stFileTypes.contains(FT_JAR)) listResult.append(FT_JAR);
        if (stFileTypes.contains(FT_APK)) listResult.append(FT_APK);
        if (stFileTypes.contains(FT_IPA)) listResult.append(FT_IPA);
        if (stFileTypes.contains(FT_7Z)) listResult.append(FT_7Z);
        if (stFileTypes.contains(FT_DEX)) listResult.append(FT_DEX);
        if (stFileTypes.contains(FT_PDF)) listResult.append(FT_PDF);
        if (stFileTypes.contains(FT_PDB)) listResult.append(FT_PDB);
        if (stFileTypes.contains(FT_DER)) listResult.append(FT_DER);
        if (stFileTypes.contains(FT_PNG)) listResult.append(FT_PNG);
        if (stFileTypes.contains(FT_ICO)) listResult.append(FT_ICO);
        if (stFileTypes.contains(FT_ICC)) listResult.append(FT_ICC);
        if (stFileTypes.contains(FT_JPEG)) listResult.append(FT_JPEG);
        if (stFileTypes.contains(FT_BMP)) listResult.append(FT_BMP);
        if (stFileTypes.contains(FT_GIF)) listResult.append(FT_GIF);
        if (stFileTypes.contains(FT_TIFF)) listResult.append(FT_TIFF);
        if (stFileTypes.contains(FT_PAL)) listResult.append(FT_PAL);
        if (stFileTypes.contains(FT_MP3)) listResult.append(FT_MP3);
        if (stFileTypes.contains(FT_MP4)) listResult.append(FT_MP4);
        if (stFileTypes.contains(FT_XM)) listResult.append(FT_XM);
        if (stFileTypes.contains(FT_AVI)) listResult.append(FT_AVI);
        if (stFileTypes.contains(FT_WEBP)) listResult.append(FT_WEBP);
        if (stFileTypes.contains(FT_WAV)) listResult.append(FT_WAV);
        if (stFileTypes.contains(FT_RIFF)) listResult.append(FT_RIFF);
        if (stFileTypes.contains(FT_SIGNATURE)) listResult.append(FT_SIGNATURE);
        if (stFileTypes.contains(FT_TAR)) listResult.append(FT_TAR);
        if (stFileTypes.contains(FT_TAR_GZ)) listResult.append(FT_TAR_GZ);
        if (stFileTypes.contains(FT_TAR_BZIP2)) listResult.append(FT_TAR_BZIP2);
        if (stFileTypes.contains(FT_TAR_LZIP)) listResult.append(FT_TAR_LZIP);
        if (stFileTypes.contains(FT_TAR_LZMA)) listResult.append(FT_TAR_LZMA);
        if (stFileTypes.contains(FT_TAR_LZOP)) listResult.append(FT_TAR_LZOP);
        if (stFileTypes.contains(FT_TAR_XZ)) listResult.append(FT_TAR_XZ);
        if (stFileTypes.contains(FT_TAR_Z)) listResult.append(FT_TAR_Z);
        if (stFileTypes.contains(FT_TAR_ZSTD)) listResult.append(FT_TAR_ZSTD);
        if (stFileTypes.contains(FT_NPM)) listResult.append(FT_NPM);
        if (stFileTypes.contains(FT_MACHOFAT)) listResult.append(FT_MACHOFAT);
        if (stFileTypes.contains(FT_AR)) listResult.append(FT_AR);
        if (stFileTypes.contains(FT_DEB)) listResult.append(FT_DEB);
        if (stFileTypes.contains(FT_JAVACLASS)) listResult.append(FT_JAVACLASS);
        if (stFileTypes.contains(FT_PYC)) listResult.append(FT_PYC);
        if (stFileTypes.contains(FT_TTF)) listResult.append(FT_TTF);
        if (stFileTypes.contains(FT_DJVU)) listResult.append(FT_DJVU);
        if (stFileTypes.contains(FT_TEXT)) listResult.append(FT_TEXT);
        if (stFileTypes.contains(FT_UTF8)) listResult.append(FT_UTF8);
        if (stFileTypes.contains(FT_UNICODE)) listResult.append(FT_UNICODE);
        if (stFileTypes.contains(FT_CFBF)) listResult.append(FT_CFBF);
        if (stFileTypes.contains(FT_SZDD)) listResult.append(FT_SZDD);
        if (stFileTypes.contains(FT_BZIP2)) listResult.append(FT_BZIP2);
        if (stFileTypes.contains(FT_BROTLI)) listResult.append(FT_BROTLI);
        if (stFileTypes.contains(FT_LZ4)) listResult.append(FT_LZ4);
        if (stFileTypes.contains(FT_LZMA)) listResult.append(FT_LZMA);
        if (stFileTypes.contains(FT_ZSTD)) listResult.append(FT_ZSTD);
        if (stFileTypes.contains(FT_XZ)) listResult.append(FT_XZ);
        if (stFileTypes.contains(FT_LZIP)) listResult.append(FT_LZIP);
        if (stFileTypes.contains(FT_LZO)) listResult.append(FT_LZO);
        if (stFileTypes.contains(FT_COMPRESS)) listResult.append(FT_COMPRESS);
        if (stFileTypes.contains(FT_CPIO)) listResult.append(FT_CPIO);
        if (stFileTypes.contains(FT_ISO9660)) listResult.append(FT_ISO9660);
        if (stFileTypes.contains(FT_UDF)) listResult.append(FT_UDF);
        if (stFileTypes.contains(FT_MINIDUMP)) listResult.append(FT_MINIDUMP);
        if (stFileTypes.contains(FT_DMG)) listResult.append(FT_DMG);
        if (stFileTypes.contains(FT_WIM)) listResult.append(FT_WIM);
        if (stFileTypes.contains(FT_RPM)) listResult.append(FT_RPM);
        if (stFileTypes.contains(FT_XAR)) listResult.append(FT_XAR);
        if (stFileTypes.contains(FT_ZOO)) listResult.append(FT_ZOO);
        if (stFileTypes.contains(FT_KWAJ)) listResult.append(FT_KWAJ);
        if (stFileTypes.contains(FT_ASAR)) listResult.append(FT_ASAR);
        if (stFileTypes.contains(FT_STK)) listResult.append(FT_STK);
    }

    if ((tlOption == TL_OPTION_DEFAULT) || (tlOption == TL_OPTION_EXECUTABLE) || (tlOption == TL_OPTION_ALL)) {
        if (stFileTypes.contains(FT_MSDOS)) listResult.append(FT_MSDOS);
        if (stFileTypes.contains(FT_NE)) listResult.append(FT_NE);
        if (stFileTypes.contains(FT_LE)) listResult.append(FT_LE);
        if (stFileTypes.contains(FT_LX)) listResult.append(FT_LX);
        if (stFileTypes.contains(FT_PE32)) listResult.append(FT_PE32);
        if (stFileTypes.contains(FT_PE64)) listResult.append(FT_PE64);
        if (stFileTypes.contains(FT_ELF32)) listResult.append(FT_ELF32);
        if (stFileTypes.contains(FT_ELF64)) listResult.append(FT_ELF64);
        if (stFileTypes.contains(FT_MACHO32)) listResult.append(FT_MACHO32);
        if (stFileTypes.contains(FT_MACHO64)) listResult.append(FT_MACHO64);
        if (stFileTypes.contains(FT_BWDOS16M)) listResult.append(FT_BWDOS16M);
        if (stFileTypes.contains(FT_AMIGAHUNK)) listResult.append(FT_AMIGAHUNK);
        if (stFileTypes.contains(FT_ATARIST)) listResult.append(FT_ATARIST);
    }

    if ((tlOption == TL_OPTION_DEFAULT) || (tlOption == TL_OPTION_ALL)) {
        if (stFileTypes.contains(FT_DOS16M)) listResult.append(FT_DOS16M);
        if (stFileTypes.contains(FT_DOS4G)) listResult.append(FT_DOS4G);
    }

    if ((listResult.count() == 0) && (tlOption == TL_OPTION_EXECUTABLE)) {
        listResult.append(FT_BINARY);
    }

    if (tlOption == TL_OPTION_SYMBOLS) {
        if (stFileTypes.contains(FT_PE32)) listResult.append(FT_PE32);
        if (stFileTypes.contains(FT_PE64)) listResult.append(FT_PE64);
        if (stFileTypes.contains(FT_MACHO32)) listResult.append(FT_MACHO32);
        if (stFileTypes.contains(FT_MACHO64)) listResult.append(FT_MACHO64);
    }

    return listResult;
}

QString XBinary::valueToHex(quint8 nValue)
{
    return QString("%1").arg(nValue, 2, 16, QChar('0'));
}

QString XBinary::valueToHex(qint8 nValue)
{
    return valueToHex((quint8)nValue);
}

QString XBinary::valueToHex(quint16 nValue, bool bIsBigEndian)
{
    if (bIsBigEndian) {
        nValue = qFromBigEndian(nValue);
    } else {
        nValue = qFromLittleEndian(nValue);
    }

    return QString("%1").arg(nValue, 4, 16, QChar('0'));
}

QString XBinary::valueToHex(qint16 nValue, bool bIsBigEndian)
{
    if (bIsBigEndian) {
        nValue = qFromBigEndian(nValue);
    } else {
        nValue = qFromLittleEndian(nValue);
    }

    return valueToHex((quint16)nValue);
}

QString XBinary::valueToHex(quint32 nValue, bool bIsBigEndian)
{
    if (bIsBigEndian) {
        nValue = qFromBigEndian(nValue);
    } else {
        nValue = qFromLittleEndian(nValue);
    }

    return QString("%1").arg(nValue, 8, 16, QChar('0'));
}

QString XBinary::valueToHex(qint32 nValue, bool bIsBigEndian)
{
    if (bIsBigEndian) {
        nValue = qFromBigEndian(nValue);
    } else {
        nValue = qFromLittleEndian(nValue);
    }

    return valueToHex((quint32)nValue);
}

QString XBinary::valueToHex(quint64 nValue, bool bIsBigEndian)
{
    if (bIsBigEndian) {
        nValue = qFromBigEndian(nValue);
    } else {
        nValue = qFromLittleEndian(nValue);
    }

    return QString("%1").arg(nValue, 16, 16, QChar('0'));
}

QString XBinary::valueToHex(qint64 nValue, bool bIsBigEndian)
{
    if (bIsBigEndian) {
        nValue = qFromBigEndian(nValue);
    } else {
        nValue = qFromLittleEndian(nValue);
    }

    return valueToHex((quint64)nValue);
}

QString XBinary::valueToHex(float fValue, bool bIsBigEndian)
{
    float _value = fValue;

    endian_float(&_value, bIsBigEndian);

    quint32 _nValue = 0;

    _copyMemory((char *)&_nValue, (char *)&_value, 4);

    return QString("%1").arg(_nValue, 8, 16, QChar('0'));
}

QString XBinary::valueToHex(double dValue, bool bIsBigEndian)
{
    double _value = dValue;

    endian_double(&_value, bIsBigEndian);

    quint64 _nValue = 0;

    _copyMemory((char *)&_nValue, (char *)&_value, 8);

    return QString("%1").arg(_nValue, 16, 16, QChar('0'));
}

QString XBinary::valueToHex(XBinary::MODE mode, quint64 nValue, bool bIsBigEndian)
{
    QString sResult;

    if (mode == MODE_UNKNOWN) {
        mode = getWidthModeFromSize(nValue);
    }

    if (mode == MODE_BIT) {
        if (nValue) {
            sResult = "1";
        } else {
            sResult = "0";
        }
    } else if (mode == MODE_8) {
        sResult = valueToHex((quint8)nValue);
    } else if (mode == MODE_16) {
        sResult = valueToHex((quint16)nValue, bIsBigEndian);
    } else if (mode == MODE_32) {
        sResult = valueToHex((quint32)nValue, bIsBigEndian);
    } else if (mode == MODE_64) {
        sResult = valueToHex((quint64)nValue, bIsBigEndian);
    }

    return sResult;
}

QString XBinary::valueToHexEx(quint64 nValue, bool bIsBigEndian)
{
    XBinary::MODE mode = getWidthModeFromSize(nValue);

    return valueToHex(mode, nValue, bIsBigEndian);
}

QString XBinary::valueToHexOS(quint64 nValue, bool bIsBigEndian)
{
    MODE mode = MODE_32;

    if (sizeof(void *) == 8) {
        mode = MODE_64;
    }

    return valueToHex(mode, nValue, bIsBigEndian);
}

QString XBinary::valueToHexColon(MODE mode, quint64 nValue, bool bIsBigEndian)
{
    QString sResult;

    if (mode == MODE_64) {
        quint32 nHigh = (quint32)(nValue >> 32);
        quint32 nLow = (quint32)(nValue);
        sResult = QString("%1:%2").arg(valueToHex(nHigh)).arg(valueToHex(nLow));
    } else if (mode == MODE_32) {
        quint16 nHigh = (quint16)(nValue >> 16);
        quint16 nLow = (quint16)(nValue);
        sResult = QString("%1:%2").arg(valueToHex(nHigh)).arg(valueToHex(nLow));
    } else {
        sResult = valueToHex(mode, nValue, bIsBigEndian);
    }

    return sResult;
}

QString XBinary::xVariantToHex(XVARIANT value)
{
    QString sResult;

    if (value.varType == VT_BIT) {
        if (value.var.toBool()) {
            sResult = "1";
        } else {
            sResult = "0";
        }
    } else if (value.varType == VT_BYTE) {
        sResult = valueToHex((quint8)value.var.toULongLong());
    } else if (value.varType == VT_WORD) {
        sResult = valueToHex((quint16)value.var.toULongLong(), value.bIsBigEndian);
    } else if (value.varType == VT_DWORD) {
        sResult = valueToHex((quint32)value.var.toULongLong(), value.bIsBigEndian);
    } else if (value.varType == VT_QWORD) {
        sResult = valueToHex((quint64)value.var.toULongLong(), value.bIsBigEndian);
    } else if (value.varType == VT_128) {
        // QString sLow = valueToHex(value.var.v_uint128[0], value.bIsBigEndian);
        // QString sHigh = valueToHex(value.var.v_uint128[1], value.bIsBigEndian);

        // if (value.bIsBigEndian) {
        //     sResult = sLow + sHigh;
        // } else {
        //     sResult = sHigh + sLow;
        // }
    } else if (value.varType == VT_256) {
        // QString s0 = valueToHex(value.var.v_uint256[0], value.bIsBigEndian);
        // QString s1 = valueToHex(value.var.v_uint256[1], value.bIsBigEndian);
        // QString s2 = valueToHex(value.var.v_uint256[2], value.bIsBigEndian);
        // QString s3 = valueToHex(value.var.v_uint256[3], value.bIsBigEndian);

        // if (value.bIsBigEndian) {
        //     sResult = s0 + s1 + s2 + s3;
        // } else {
        //     sResult = s3 + s2 + s1 + s0;
        // }
    } else if (value.varType == VT_FPEG) {
        // for (qint32 i = 0; i < 10; i++) {
        //     sResult += valueToHex(value.var.v_freg[i]);
        // }
    }

    return sResult;
}

QString XBinary::thisToString(qint64 nDelta, qint32 nBase)
{
    QString sResult;

    if (nDelta == 0) {
        sResult = "$ ==>";
    } else if (nDelta > 0) {
        sResult = QString("$+%1").arg(nDelta, 0, nBase);
    } else if (nDelta < 0) {
        sResult = QString("$-%1").arg(-nDelta, 0, nBase);
    }

    return sResult;
}

bool XBinary::checkString_byte(const QString &sValue)
{
    bool bResult = false;

    // TODO Check

    quint16 nValue = sValue.toUShort(&bResult, 16);

    if (bResult) {
        bResult = (nValue <= 255);
    }

    return bResult;
}

bool XBinary::checkString_word(const QString &sValue)
{
    bool bResult = false;

    sValue.toUShort(&bResult, 16);

    return bResult;
}

bool XBinary::checkString_dword(const QString &sValue)
{
    bool bResult = false;

    sValue.toUInt(&bResult, 16);

    return bResult;
}

bool XBinary::checkString_qword(const QString &sValue)
{
    bool bResult = false;

    sValue.toULongLong(&bResult, 16);

    return bResult;
}

bool XBinary::checkString_uint8(const QString &sValue)
{
    bool bResult = false;

    // TODO Check

    quint16 nValue = sValue.toUShort(&bResult);

    if (bResult) {
        bResult = (nValue <= 255);
    }

    return bResult;
}

bool XBinary::checkString_int8(const QString &sValue)
{
    bool bResult = false;

    qint16 nValue = sValue.toShort(&bResult);

    if (bResult) {
        bResult = ((-128 <= nValue) && (nValue <= 127));
    }

    return bResult;
}

bool XBinary::checkString_uint16(const QString &sValue)
{
    bool bResult = false;

    sValue.toUShort(&bResult);

    return bResult;
}

bool XBinary::checkString_int16(const QString &sValue)
{
    bool bResult = false;

    sValue.toShort(&bResult);

    return bResult;
}

bool XBinary::checkString_uint32(const QString &sValue)
{
    bool bResult = false;

    sValue.toUInt(&bResult);

    return bResult;
}

bool XBinary::checkString_int32(const QString &sValue)
{
    bool bResult = false;

    sValue.toInt(&bResult);

    return bResult;
}

bool XBinary::checkString_uint64(const QString &sValue)
{
    bool bResult = false;

    sValue.toULongLong(&bResult);

    return bResult;
}

bool XBinary::checkString_int64(const QString &sValue)
{
    bool bResult = false;

    sValue.toLongLong(&bResult);

    return bResult;
}

bool XBinary::checkString_float(const QString &sValue)
{
    bool bResult = false;

    sValue.toFloat(&bResult);

    return bResult;
}

bool XBinary::checkString_double(const QString &sValue)
{
    bool bResult = false;

    sValue.toDouble(&bResult);

    return bResult;
}

QString XBinary::boolToString(bool bValue)
{
    QString sResult;

    if (bValue) {
        sResult = tr("true");
    } else {
        sResult = tr("false");
    }

    return sResult;
}

QString XBinary::getSpaces(qint32 nNumberOfSpaces)
{
    QString sResult;

    sResult = sResult.rightJustified(nNumberOfSpaces, QChar(' '));

    return sResult;
}

QString XBinary::getUnpackedFileName(QIODevice *pDevice, bool bShort)
{
    QString sResult = "unpacked";

    QFile *pFile = dynamic_cast<QFile *>(pDevice);

    if (pFile) {
        QString sFileName = pFile->fileName();

        if (sFileName != "") {
            sResult = getUnpackedFileName(sFileName);
        }

        if (bShort) {
            QFileInfo fi(sResult);
            sResult = fi.completeBaseName();
            if (!fi.suffix().isEmpty()) {
                sResult += QLatin1Char('.') + fi.suffix();
            }
        }
    }

    return sResult;
}

QString XBinary::getUnpackedFileName(const QString &sFileName)
{
    QFileInfo fileInfo(sFileName);
    QString sResult = fileInfo.absolutePath() + QDir::separator() + fileInfo.completeBaseName() + ".unp";
    if (!fileInfo.suffix().isEmpty()) {
        sResult += QLatin1Char('.') + fileInfo.suffix();
    }
    //            sResult=fi.absolutePath()+QDir::separator()+fi.baseName()+".unp."+fi.completeSuffix();

    return sResult;
}

QString XBinary::getDumpFileName(QIODevice *pDevice)
{
    QString sResult = "dump";

    QFile *pFile = dynamic_cast<QFile *>(pDevice);

    if (pFile) {
        QString sFileName = pFile->fileName();

        if (sFileName != "") {
            sResult = getDumpFileName(sFileName);
        }
    }

    return sResult;
}

QString XBinary::getDumpFileName(const QString &sFileName)
{
    QFileInfo fileInfo(sFileName);
    QString sResult = fileInfo.absolutePath() + QDir::separator() + fileInfo.completeBaseName() + ".dump";
    if (!fileInfo.suffix().isEmpty()) {
        sResult += QLatin1Char('.') + fileInfo.suffix();
    }

    return sResult;
}

QString XBinary::getBackupFileName(QIODevice *pDevice)
{
    const QString sFileName = getDeviceFileName(pDevice);
    return sFileName.isEmpty() ? QString() : getBackupFileName(sFileName);
}

QString XBinary::getBackupFileName(const QString &sFileName)
{
    if (sFileName.trimmed().isEmpty()) {
        return QString();
    }

    QFileInfo fi(sFileName);
    QString sResult;

    sResult += fi.absolutePath() + QDir::separator() + fi.completeBaseName();

    QString sSuffix = fi.suffix();

    if (sSuffix != "") {
        sResult += "." + sSuffix;
    }

    sResult += QString("_%1.BAK").arg(getCurrentBackupDate());

    return sResult;
}

QString XBinary::getResultFileName(QIODevice *pDevice, const QString &sAppendix)
{
    QString sResult = sAppendix;

    QFile *pFile = dynamic_cast<QFile *>(pDevice);

    if (pFile) {
        QString sFileName = pFile->fileName();

        if (sFileName != "") {
            sResult = getResultFileName(sFileName, sAppendix);
        }
    }

    return sResult;
}

QString XBinary::getResultFileName(const QString &sFileName, const QString &sAppendix)
{
    QString sResult;
    // mb TODO if file exists write other .1 .2 ...
    QFileInfo fileInfo(sFileName);

    QString sSuffix = fileInfo.suffix();

    sResult += fileInfo.absolutePath() + QDir::separator() + fileInfo.completeBaseName() + ".";

    if (sSuffix != "") {
        sResult += sSuffix + ".";
    }

    sResult += sAppendix;

    return sResult;
}

QString XBinary::getDeviceFileName(QIODevice *pDevice)
{
    if (!pDevice) {
        return QString();
    }

    QFile *pFile = dynamic_cast<QFile *>(pDevice);
    if (pFile) {
        QString sResult = getOpenQFilePath(pFile);
        if (sResult.isEmpty() && !pFile->fileName().isEmpty()) {
            sResult = QFileInfo(pFile->fileName()).absoluteFilePath();
        }
        return sResult;
    }

    return pDevice->property("FileName").toString();
}

QString XBinary::getDeviceFilePath(QIODevice *pDevice)
{
    QString sResult;

    const QString sFileName = getDeviceFileName(pDevice);
    if (!sFileName.isEmpty()) {
        sResult = QFileInfo(sFileName).absoluteFilePath();
    }

    return sResult;
}

QString XBinary::getDeviceDirectory(QIODevice *pDevice)
{
    QString sResult;

    QString sFileName = getDeviceFileName(pDevice);

    if (sFileName != "") {
        QFileInfo fi(sFileName);

        sResult = fi.absolutePath();
    }

    return sResult;
}

QString XBinary::getDeviceFileBaseName(QIODevice *pDevice)
{
    QString sResult;

    QString sFileName = getDeviceFileName(pDevice);

    if (sFileName != "") {
        QFileInfo fi(sFileName);

        sResult = fi.baseName();
    }

    return sResult;
}

QString XBinary::getDeviceFileCompleteSuffix(QIODevice *pDevice)
{
    QString sResult;

    QString sFileName = getDeviceFileName(pDevice);

    if (sFileName != "") {
        QFileInfo fi(sFileName);

        sResult = fi.completeSuffix();
    }

    return sResult;
}

QString XBinary::getDeviceFileSuffix(QIODevice *pDevice)
{
    QString sResult;

    QString sFileName = getDeviceFileName(pDevice);

    if (sFileName != "") {
        QFileInfo fi(sFileName);

        sResult = fi.suffix();
    }

    return sResult;
}

QString XBinary::getFileDirectory(const QString &sFileName)
{
    QFileInfo fi(sFileName);

    return fi.absolutePath();
}

QIODevice *XBinary::getBackupDevice(QIODevice *pDevice)
{
    QSet<QIODevice *> visitedDevices;

    while (pDevice) {
        if (visitedDevices.contains(pDevice)) {
            return nullptr;
        }
        visitedDevices.insert(pDevice);

        SubDevice *pSubDevice = dynamic_cast<SubDevice *>(pDevice);
        if (!pSubDevice) {
            return pDevice;
        }

        pDevice = pSubDevice->getOrigDevice();
    }

    return nullptr;
}

bool XBinary::isBackupPresent(QIODevice *pDevice)
{
    if (!pDevice) {
        return false;
    }

    const QString sSourceFileName = XBinary::getDeviceFileName(pDevice);
    const QString sBackupFileName = XBinary::getBackupFileName(pDevice);
    const QFileInfo sourceInfo(sSourceFileName);
    const QFileInfo backupInfo(sBackupFileName);

    return !sSourceFileName.isEmpty() && !sBackupFileName.isEmpty() && sourceInfo.exists() && sourceInfo.isFile() &&
           !sourceInfo.isSymLink() && isDirectCanonicalPath(sourceInfo) && backupInfo.exists() && backupInfo.isFile() &&
           !backupInfo.isSymLink() && isDirectCanonicalPath(backupInfo) &&
           areDistinctRegularFiles(sSourceFileName, sBackupFileName);
}

bool XBinary::saveBackup(QIODevice *pDevice)
{
    if (!pDevice) {
        return false;
    }

    const QString sSourceFileName = XBinary::getDeviceFileName(pDevice);
    const QString sBackupFileName = XBinary::getBackupFileName(pDevice);
    const QFileInfo sourceInfo(sSourceFileName);

    if (sSourceFileName.isEmpty() || sBackupFileName.isEmpty() || !sourceInfo.exists() || !sourceInfo.isFile() ||
        sourceInfo.isSymLink() || !isDirectCanonicalPath(sourceInfo)) {
        return false;
    }

    const QFileInfo backupInfo(sBackupFileName);
    if (backupInfo.exists() || backupInfo.isSymLink()) {
        return backupInfo.exists() && backupInfo.isFile() && !backupInfo.isSymLink() &&
               isDirectCanonicalPath(backupInfo) && areDistinctRegularFiles(sSourceFileName, sBackupFileName);
    }

    if (!QFile::copy(sSourceFileName, sBackupFileName)) return false;

    const QFileInfo createdBackupInfo(sBackupFileName);
    return createdBackupInfo.exists() && createdBackupInfo.isFile() && !createdBackupInfo.isSymLink() &&
           isDirectCanonicalPath(createdBackupInfo) && areDistinctRegularFiles(sSourceFileName, sBackupFileName);
}

QString XBinary::getCurrentBackupDate()
{
    QString sResult;

    sResult = QDate::currentDate().toString("yyyy-MM-dd");

    return sResult;
}

QList<qint64> XBinary::getFixupList(QIODevice *pDevice1, QIODevice *pDevice2, qint64 nDelta, PDSTRUCT *pPdStruct)
{
    // Preserve the historical API and its PE32/HIGHLOW semantics.  PE32+
    // callers must opt in explicitly so existing users never start receiving
    // a different set of heuristic candidates.
    return getFixupList(pDevice1, pDevice2, nDelta, FIXUP_WIDTH_32, pPdStruct);
}

QList<qint64> XBinary::getFixupList(QIODevice *pDevice1, QIODevice *pDevice2, qint64 nDelta, FIXUP_WIDTH fixupWidth,
                                    PDSTRUCT *pPdStruct)
{
    QList<qint64> listResult;

    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }
    const QSharedPointer<PDSTRUCT_CALLBACK_STATE> pProgressLifetime = pPdStruct->_pdCallbackState;

    QPointer<QIODevice> guardedDevice1(pDevice1);
    QPointer<QIODevice> guardedDevice2(pDevice2);
    if (!guardedDevice1 || !guardedDevice2 ||
        !guardedDevice1->isReadable() || !guardedDevice1 ||
        !guardedDevice2->isReadable() || !guardedDevice2 ||
        guardedDevice1->isSequential() || !guardedDevice1 ||
        guardedDevice2->isSequential() || !guardedDevice2 ||
        ((fixupWidth != FIXUP_WIDTH_32) && (fixupWidth != FIXUP_WIDTH_64)) || !isPdStructNotCanceled(pPdStruct)) {
        return listResult;
    }

    const qint32 nValueSize = (qint32)fixupWidth;

    const qint64 nOriginalPosition1 = guardedDevice1->pos();
    if (!guardedDevice1 || !guardedDevice2) return listResult;
    const qint64 nOriginalPosition2 = guardedDevice2->pos();
    if (!guardedDevice1 || !guardedDevice2 ||
        (nOriginalPosition1 < 0) || (nOriginalPosition2 < 0)) return listResult;

    const qint64 nSize1 = guardedDevice1->size();
    if (!guardedDevice1 || !guardedDevice2) return listResult;
    const qint64 nSize2 = guardedDevice2->size();
    if (!guardedDevice1 || !guardedDevice2) return listResult;

    if ((nSize1 >= 0) && (nSize1 == nSize2)) {
        qint64 nSize = nSize1;
        const qint32 nRequestedBufferSize = getBufferSize(pPdStruct);

        if (nRequestedBufferSize <= 0) {
            return listResult;
        }

        qint32 nBufferSize = nRequestedBufferSize;

        if (nBufferSize < 0x1000) {
            nBufferSize = 0x1000;
        } else if (nBufferSize > 0x100000) {
            nBufferSize = 0x100000;
        }

        char *pBuffer1 = new (std::nothrow) char[nBufferSize];

        if (!pBuffer1) {
            return listResult;
        }

        char *pBuffer2 = new (std::nothrow) char[nBufferSize];

        if (!pBuffer2) {
            delete[] pBuffer1;
            return listResult;
        }

        qint64 nOffset = 0;
        bool bReadError = false;
        bool bProgressOwnerAlive = true;
        XBinary binary;
        const qint32 nFreeIndex = XBinary::reservePdStructRecord(pPdStruct, nSize);

        while ((nSize >= nValueSize) && isPdStructNotCanceled(pPdStruct)) {
            qint32 nTemp = nBufferSize;

            if (nSize < nBufferSize) {
                nTemp = (qint32)nSize;
            }

            if (!guardedDevice1 || !guardedDevice2 ||
                (binary.safeReadData(guardedDevice1.data(), nOffset,
                                     pBuffer1, nTemp, pPdStruct) != nTemp) ||
                !guardedDevice1 || !guardedDevice2 ||
                (binary.safeReadData(guardedDevice2.data(), nOffset,
                                     pBuffer2, nTemp, pPdStruct) != nTemp) ||
                !guardedDevice1 || !guardedDevice2) {
                bReadError = true;
                break;
            }

            for (qint32 i = 0; (i <= nTemp - nValueSize) && isPdStructNotCanceled(pPdStruct); i++) {
                bool bIsFixup = false;

                if (fixupWidth == FIXUP_WIDTH_32) {
                    quint32 nValue1 = 0;
                    quint32 nValue2 = 0;
                    memcpy(&nValue1, pBuffer1 + i, sizeof(nValue1));
                    memcpy(&nValue2, pBuffer2 + i, sizeof(nValue2));
                    nValue1 = qFromLittleEndian(nValue1);
                    nValue2 = qFromLittleEndian(nValue2);

                    // PE HIGHLOW fixups use modulo-2^32 arithmetic.  Widening
                    // signed values before subtracting misses relocations that
                    // cross the 0x7fffffff/0x80000000 boundary.
                    bIsFixup = ((quint32)(nValue2 - nValue1) == (quint32)nDelta);
                } else {
                    quint64 nValue1 = 0;
                    quint64 nValue2 = 0;
                    memcpy(&nValue1, pBuffer1 + i, sizeof(nValue1));
                    memcpy(&nValue2, pBuffer2 + i, sizeof(nValue2));
                    nValue1 = qFromLittleEndian(nValue1);
                    nValue2 = qFromLittleEndian(nValue2);

                    // DIR64 candidates must match the complete pointer.  A
                    // low-32-bit comparison systematically turns unrelated
                    // values that differ by 2^32 into false DIR64 entries.
                    bIsFixup = ((quint64)(nValue2 - nValue1) == (quint64)nDelta);
                }

                if (bIsFixup) {
                    listResult.append(nOffset + i);
                }
            }

            const qint32 nStep = nTemp - (nValueSize - 1);
            nSize -= nStep;
            nOffset += nStep;
            XBinary::setPdStructCurrent(pPdStruct, nFreeIndex, nOffset);
            if (!guardedDevice1 || !guardedDevice2 ||
                !isPdCallbackOwnerAlive(pProgressLifetime)) {
                bProgressOwnerAlive = false;
                break;
            }
        }

        delete[] pBuffer1;
        delete[] pBuffer2;
        if (!bProgressOwnerAlive) {
            listResult.clear();
        } else {
            XBinary::setPdStructFinished(pPdStruct, nFreeIndex);
            if (bReadError || !isPdStructNotCanceled(pPdStruct)) {
                listResult.clear();
            }
        }
    }

    if (!guardedDevice1 || !guardedDevice2) {
        listResult.clear();
        return listResult;
    }
    const bool bPosition1Restored = guardedDevice1->seek(nOriginalPosition1);
    if (!guardedDevice1 || !guardedDevice2) {
        listResult.clear();
        return listResult;
    }
    const bool bPosition2Restored = guardedDevice2->seek(nOriginalPosition2);
    if (!guardedDevice1 || !guardedDevice2 ||
        !bPosition1Restored || !bPosition2Restored) {
        listResult.clear();
    }
    return listResult;
}

QString XBinary::getHash(XBinary::HASH hash, const QString &sFileName, PDSTRUCT *pPdStruct)
{
    QString sResult;

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        sResult = XBinary::getHash(hash, &file, pPdStruct);

        file.close();
    }

    return sResult;
}

QString XBinary::getHash(XBinary::HASH hash, QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    QString sResult;

    QPointer<QIODevice> guardedDevice(pDevice);
    if (!guardedDevice || !guardedDevice->isReadable() || !guardedDevice ||
        guardedDevice->isSequential() || !guardedDevice) {
        return sResult;
    }

    const qint64 nOriginalPosition = guardedDevice->pos();

    if (!guardedDevice || (nOriginalPosition < 0)) {
        return sResult;
    }

    XBinary binary(guardedDevice.data());

    sResult = binary.getHash(hash, 0, -1, pPdStruct);

    if (!guardedDevice) return QString();
    const bool bRestored = guardedDevice->seek(nOriginalPosition);
    if (!guardedDevice || !bRestored) {
        sResult.clear();
    }

    return sResult;
}

QString XBinary::getHash(XBinary::HASH hash, qint64 nOffset, qint64 nSize, PDSTRUCT *pPdStruct)
{
    QString sResult;

    const qint64 nTotalSize = getSize();

    if ((nOffset < 0) || (nOffset > nTotalSize) || (nSize < -1)) {
        return sResult;
    }

    if (nSize == -1) {
        nSize = nTotalSize - nOffset;
    }

    if (nSize > nTotalSize - nOffset) {
        return sResult;
    }

    QList<OFFSETSIZE> listOS;
    OFFSETSIZE osRegion = {};
    osRegion.nOffset = nOffset;
    osRegion.nSize = nSize;
    listOS.append(osRegion);

    sResult = getHash(hash, &listOS, pPdStruct);

    return sResult;
}

QString XBinary::getHash(HASH hash, QList<OFFSETSIZE> *pListOS, PDSTRUCT *pPdStruct)
{
    QString sResult;

    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    if (!pListOS || !isPdStructNotCanceled(pPdStruct)) {
        return sResult;
    }

    QCryptographicHash::Algorithm algorithm = QCryptographicHash::Md4;
    bool bAlgorithmValid = true;

    switch (hash)  // TODO Check new versions of Qt
    {
        case HASH_MD4: algorithm = QCryptographicHash::Md4; break;
        case HASH_MD5: algorithm = QCryptographicHash::Md5; break;
        case HASH_SHA1: algorithm = QCryptographicHash::Sha1; break;
#ifndef QT_CRYPTOGRAPHICHASH_ONLY_SHA1
#if (QT_VERSION_MAJOR > 4)
        case HASH_SHA224: algorithm = QCryptographicHash::Sha224; break;  // Keccak_224 ?
        case HASH_SHA256: algorithm = QCryptographicHash::Sha256; break;
        case HASH_SHA384: algorithm = QCryptographicHash::Sha384; break;
        case HASH_SHA512: algorithm = QCryptographicHash::Sha512; break;
#endif
#endif
        default: bAlgorithmValid = false; break;
    }

    if (!bAlgorithmValid) {
        return sResult;
    }

    const qint64 nDeviceSize = getSize();
    qint64 nTotalSize = 0;

    for (qint32 i = 0; i < pListOS->count(); i++) {
        const OFFSETSIZE &osRegion = pListOS->at(i);

        if ((osRegion.nOffset < 0) || (osRegion.nSize < 0) || (osRegion.nOffset > nDeviceSize) || (osRegion.nSize > nDeviceSize - osRegion.nOffset) ||
            (nTotalSize > (std::numeric_limits<qint64>::max)() - osRegion.nSize)) {
            return sResult;
        }

        nTotalSize += osRegion.nSize;
    }

    const qint32 nRequestedBufferSize = getBufferSize(pPdStruct);

    if ((nTotalSize > 0) && (nRequestedBufferSize <= 0)) {
        return sResult;
    }

    const qint32 nBufferSize = qBound((qint32)0x1000, qMax((qint32)1, nRequestedBufferSize), (qint32)0x100000);
    char *pBuffer = (nTotalSize > 0) ? new (std::nothrow) char[nBufferSize] : nullptr;

    if ((nTotalSize > 0) && !pBuffer) {
        return sResult;
    }

    QCryptographicHash crypto(algorithm);

    qint64 nCurrentSize = 0;
    bool bReadError = false;
    const qint32 nFreeIndex = (nTotalSize > 0) ? XBinary::reservePdStructRecord(pPdStruct, nTotalSize) : -1;

    for (qint32 i = 0; (i < pListOS->count()) && isPdStructNotCanceled(pPdStruct); i++) {
        qint64 nOffset = pListOS->at(i).nOffset;
        qint64 nSize = pListOS->at(i).nSize;

        while ((nSize > 0) && isPdStructNotCanceled(pPdStruct)) {
            const qint64 nTemp = qMin((qint64)nBufferSize, nSize);

            if (read_array_process(nOffset, pBuffer, nTemp, pPdStruct) != nTemp) {
                setPdStructErrorString(pPdStruct, tr("Read error"));
                bReadError = true;
                break;
            }

            crypto.addData(pBuffer, (int)nTemp);

            nSize -= nTemp;
            nOffset += nTemp;
            nCurrentSize += nTemp;

            XBinary::setPdStructCurrent(pPdStruct, nFreeIndex, nCurrentSize);
        }

        if (bReadError) {
            break;
        }
    }

    delete[] pBuffer;

    if (nTotalSize > 0) {
        XBinary::setPdStructFinished(pPdStruct, nFreeIndex);
    }

    if (!bReadError && isPdStructNotCanceled(pPdStruct)) {
        sResult = crypto.result().toHex();
    }

    return sResult;
}

QSet<XBinary::HASH> XBinary::getHashMethods()
{
    QSet<XBinary::HASH> stResult;

    stResult.insert(HASH_MD4);
    stResult.insert(HASH_MD5);
    stResult.insert(HASH_SHA1);
#ifndef QT_CRYPTOGRAPHICHASH_ONLY_SHA1
#if (QT_VERSION_MAJOR > 4)
    stResult.insert(HASH_SHA224);
    stResult.insert(HASH_SHA256);
    stResult.insert(HASH_SHA384);
    stResult.insert(HASH_SHA512);
#endif
#endif

    return stResult;
}

QList<XBinary::HASH> XBinary::getHashMethodsAsList()
{
    QList<XBinary::HASH> listResult;

    listResult.append(HASH_MD4);
    listResult.append(HASH_MD5);
    listResult.append(HASH_SHA1);
#ifndef QT_CRYPTOGRAPHICHASH_ONLY_SHA1
#if (QT_VERSION_MAJOR > 4)
    listResult.append(HASH_SHA224);
    listResult.append(HASH_SHA256);
    listResult.append(HASH_SHA384);
    listResult.append(HASH_SHA512);
#endif
#endif

    return listResult;
}

QString XBinary::hashIdToString(XBinary::HASH hash)
{
    QString sResult = tr("Unknown");

    switch (hash) {
        case HASH_MD4: sResult = QString("MD4"); break;
        case HASH_MD5: sResult = QString("MD5"); break;
        case HASH_SHA1: sResult = QString("SHA1"); break;
#ifndef QT_CRYPTOGRAPHICHASH_ONLY_SHA1
#if (QT_VERSION_MAJOR > 4)
        case HASH_SHA224: sResult = QString("SHA224"); break;
        case HASH_SHA256: sResult = QString("SHA256"); break;
        case HASH_SHA384: sResult = QString("SHA384"); break;
        case HASH_SHA512: sResult = QString("SHA512"); break;
#endif
#endif
    }

    return sResult;
}

bool XBinary::isFileHashValid(XBinary::HASH hash, const QString &sFileName, const QString &sHash)
{
    bool bResult = false;

    if (isFileExists(sFileName)) {
        bResult = (getHash(hash, sFileName).toUpper() == sHash.toUpper());
    }

    return bResult;
}

quint32 XBinary::getAdler32(const QString &sFileName)
{
    quint32 nResult = 0;

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        nResult = XBinary::getAdler32(&file);

        file.close();
    }

    return nResult;
}

quint32 XBinary::getAdler32(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    quint32 nResult = 0;
    QPointer<QIODevice> guardedDevice(pDevice);

    if (!guardedDevice) {
        return nResult;
    }

    guardedDevice->reset();
    if (!guardedDevice) return 0;

    XBinary binary(guardedDevice.data());

    nResult = binary.getAdler32(0, -1, pPdStruct);
    if (!guardedDevice) return 0;

    guardedDevice->reset();
    if (!guardedDevice) return 0;

    return nResult;
}

quint32 XBinary::getAdler32(qint64 nOffset, qint64 nSize, PDSTRUCT *pPdStruct)
{
    // TODO Check crash
    // TODO optimize!!!
    // TODO Progress bar
    quint32 nResult = 0;

    if (!isPdStructNotCanceled(pPdStruct) || (nOffset < 0) || (nSize < -1)) {
        return nResult;
    }

    // Adler-32 starts at one, so a valid empty region hashes to 1.  The
    // generic range normalizer intentionally represents empty regions as an
    // invalid offset, which previously made empty zlib members impossible to
    // authenticate.
    const qint64 nTotalSize = getSize();
    if ((nOffset <= nTotalSize) &&
        ((nSize == 0) || ((nSize == -1) && (nOffset == nTotalSize)))) {
        return 1;
    }

    OFFSETSIZE osRegion = convertOffsetAndSize(nOffset, nSize);

    nOffset = osRegion.nOffset;
    nSize = osRegion.nSize;

    const quint32 MOD_ADLER = 65521;

    if (nOffset != -1) {
        qint32 nBufferSize = getBufferSize(pPdStruct);
        if (nBufferSize <= 0) {
            return 0;
        }
        char *pBuffer = new (std::nothrow) char[nBufferSize];
        if (!pBuffer) {
            return 0;
        }

        quint32 a = 1;
        quint32 b = 0;

        qint32 _nFreeIndex = XBinary::reservePdStructRecord(pPdStruct, nSize);

        while ((nSize > 0) && isPdStructNotCanceled(pPdStruct)) {
            const qint32 nChunkSize = (nSize < (qint64)nBufferSize) ? (qint32)nSize : nBufferSize;

            if (read_array_process(nOffset, pBuffer, nChunkSize, pPdStruct) != nChunkSize) {
                setPdStructErrorString(pPdStruct, tr("Read error"));
                XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);
                delete[] pBuffer;
                return 0;
            }

            for (qint32 i = 0; (i < nChunkSize) && isPdStructNotCanceled(pPdStruct); i++) {
                a = (a + (quint8)(pBuffer[i])) % MOD_ADLER;
                b = (b + a) % MOD_ADLER;
            }

            nSize -= nChunkSize;
            nOffset += nChunkSize;

            XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, nOffset);
        }

        XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);

        delete[] pBuffer;

        if (!XBinary::isPdStructStopped(pPdStruct)) {
            nResult = (b << 16) | a;
        }
    }

    return nResult;
}

void XBinary::_createCRC32Table(quint32 *pCRCTable, quint32 nPoly)
{
    for (qint32 i = 0; i < 256; i++) {
        quint32 crc = i;

        for (qint32 j = 0; j < 8; j++) {
            crc = (crc & 1) ? ((crc >> 1) ^ nPoly) : (crc >> 1);
        }

        *(pCRCTable + i) = crc;
    }
}

quint32 *XBinary::_getCRC32Table_EDB88320()
{
    return (quint32 *)_crc32_EDB88320_tab;
}

quint16 *XBinary::_getCRC16Table()
{
    return (quint16 *)_crc16_tab;
}

quint32 XBinary::_getCRC32(const QString &sFileName, PDSTRUCT *pPdStruct)
{
    quint32 nResult = 0;

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        nResult = XBinary::_getCRC32(&file, pPdStruct);

        file.close();
    }

    return nResult;
}

quint32 XBinary::_getCRC32(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    quint32 nResult = 0;
    QPointer<QIODevice> guardedDevice(pDevice);

    if (!guardedDevice) {
        return nResult;
    }

    XBinary binary(guardedDevice.data());

    nResult = binary._getCRC32(0, -1, 0xFFFFFFFF, _getCRC32Table_EDB88320(), pPdStruct);
    if (!guardedDevice) return 0;

    guardedDevice->reset();
    if (!guardedDevice) return 0;

    //    quint32 nResult2 = binary._getCRC32_2(0, -1);

    return nResult;
}

quint32 XBinary::_getCRC32(QIODevice *pDevice, quint32 nInit, quint32 *pCRCTable, PDSTRUCT *pPdStruct)
{
    quint32 nResult = nInit;
    QPointer<QIODevice> guardedDevice(pDevice);

    if (!guardedDevice || !pCRCTable) {
        return nResult;
    }

    XBinary binary(guardedDevice.data());

    OFFSETSIZE osRegion = binary.convertOffsetAndSize(0, -1);
    if (!guardedDevice) return 0;
    qint64 nOffset = osRegion.nOffset;
    qint64 nSize = osRegion.nSize;

    qint32 _nFreeIndex = -1;

    if ((nOffset != -1) && (XBinary::isPdStructNotCanceled(pPdStruct))) {
        _nFreeIndex = XBinary::reservePdStructRecord(pPdStruct, nSize);

        qint64 nTemp = 0;
        const qint32 nRequestedBufferSize = binary.getBufferSize(pPdStruct);
        if (nRequestedBufferSize <= 0) {
            XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);
            if (guardedDevice) guardedDevice->reset();
            return 0;
        }
        const qint32 nBufferSize = qBound((qint32)0x1000, nRequestedBufferSize, (qint32)0x100000);
        char *pBuffer = new (std::nothrow) char[nBufferSize];
        if (!pBuffer) {
            XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);
            if (guardedDevice) guardedDevice->reset();
            return 0;
        }

        while ((nSize > 0) && isPdStructNotCanceled(pPdStruct)) {
            nTemp = qMin((qint64)nBufferSize, nSize);

            if (binary.read_array_process(nOffset, pBuffer, nTemp, pPdStruct) != nTemp) {
                XBinary::setPdStructInfoString(pPdStruct, QObject::tr("Read error"));
                nResult = 0;
                break;
            }
            if (!guardedDevice) {
                nResult = 0;
                break;
            }

            nResult = _getCRC32(pBuffer, (qint32)nTemp, nResult, pCRCTable);

            nSize -= nTemp;
            nOffset += nTemp;

            XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, nOffset);
        }

        delete[] pBuffer;
    }

    if (!guardedDevice) return 0;
    guardedDevice->reset();
    if (!guardedDevice) return 0;

    XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);

    if (XBinary::isPdStructStopped(pPdStruct)) {
        nResult = 0;
    }

    return (quint32)nResult;
}

quint32 XBinary::_getCRC32(const char *pData, qint32 nDataSize, quint32 nInit, quint32 *pCRCTable)
{
    quint32 nResult = nInit;

    // This is an incremental primitive: an empty or rejected chunk leaves the
    // caller's accumulator unchanged.  Never dereference an invalid raw input
    // or table supplied through the public API.
    if ((nDataSize <= 0) || !pData || !pCRCTable) {
        return nResult;
    }

    while (nDataSize > 0) {
        quint8 nIndex = (nResult ^ (((quint8)(*pData)) & 0xFF));
        nResult = (*(pCRCTable + nIndex)) ^ (nResult >> 8);

        nDataSize--;
        pData++;
    }

    return nResult;
}

quint16 XBinary::_getCRC16(const char *pData, qint32 nDataSize, quint16 nInit, quint16 *pCRCTable)
{
    quint16 nResult = nInit;

    // Match the CRC32 incremental contract for invalid/empty chunks.
    if ((nDataSize <= 0) || !pData || !pCRCTable) {
        return nResult;
    }

    while (nDataSize > 0) {
        quint8 nIndex = (nResult ^ (((quint8)(*pData)) & 0xFF));
        nResult = (*(pCRCTable + nIndex)) ^ (nResult >> 8);

        nDataSize--;
        pData++;
    }

    return nResult;
}

quint32 XBinary::_getCRC32(const QByteArray &baData, quint32 nInit, quint32 *pCRCTable)
{
    return _getCRC32(baData.data(), baData.size(), nInit, pCRCTable);
}

quint16 XBinary::_getCRC16(const QByteArray &baData, quint16 nInit, quint16 *pCRCTable)
{
    return _getCRC16(baData.data(), baData.size(), nInit, pCRCTable);
}

bool XBinary::checkCRC(QIODevice *pDevice, CRC_TYPE crcType, QVariant value, PDSTRUCT *pPdStruct)
{
    bool bExpectedCRCValid = false;
    const quint32 nExpectedCRC = value.toUInt(&bExpectedCRCValid);
    const bool bSupportedType = (crcType == CRC_TYPE_EDB88320) || (crcType == CRC_TYPE_ADLER32) ||
                                (crcType == CRC_TYPE_FFFFFFFF_EDB88320_00000000) ||
                                (crcType == CRC_TYPE_FFFFFFFF_EDB88320_FFFFFFFFF) ||
                                (crcType == CRC_TYPE_CRC16) || (crcType == CRC_TYPE_CRC16ARC) ||
                                (crcType == CRC_TYPE_RAR14);

    QPointer<QIODevice> guardedDevice(pDevice);
    if (!guardedDevice || !guardedDevice->isReadable() || !guardedDevice ||
        !bExpectedCRCValid || !bSupportedType ||
        !isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    // A checksum value of zero is valid for several algorithms (and for an
    // empty input).  Keep completion state separate from the numeric result so
    // that a read error or cancellation can never authenticate as CRC zero.
    const qint64 nExpectedSize = guardedDevice->size();
    const qint32 nRequestedBufferSize = getBufferSize(pPdStruct);
    if (!guardedDevice || (nExpectedSize < 0) ||
        (nRequestedBufferSize <= 0)) {
        return false;
    }
    const bool bInitialSeek = guardedDevice->seek(0);
    if (!guardedDevice || !bInitialSeek) return false;

    qint32 nBufferSize = nRequestedBufferSize;

    if (nBufferSize < 0x1000) {
        nBufferSize = 0x1000;
    } else if (nBufferSize > 0x100000) {
        nBufferSize = 0x100000;
    }

    QByteArray baBuffer;
    try {
        baBuffer.resize(nBufferSize);
    } catch (const std::bad_alloc &) {
        return false;
    }
    if (baBuffer.size() != nBufferSize) {
        return false;
    }

    quint32 nCRC32 = 0xFFFFFFFF;
    quint16 nCRC16 = 0;
    quint32 nAdlerA = 1;
    quint32 nAdlerB = 0;
    const quint32 nAdlerMod = 65521;
    qint64 nProcessedSize = 0;
    bool bCRCComputed = true;

    const qint32 nFreeIndex = XBinary::reservePdStructRecord(pPdStruct, nExpectedSize);
    if (!guardedDevice) return false;

    while ((nProcessedSize < nExpectedSize) && isPdStructNotCanceled(pPdStruct)) {
        const qint64 nToRead = qMin((qint64)nBufferSize, nExpectedSize - nProcessedSize);
        if (!guardedDevice) {
            bCRCComputed = false;
            break;
        }
        const bool bPositioned = guardedDevice->seek(nProcessedSize);
        if (!guardedDevice || !bPositioned) {
            bCRCComputed = false;
            break;
        }
        const qint64 nRead = guardedDevice->read(baBuffer.data(), nToRead);

        // Positive short reads are legal; zero before the declared extent,
        // negative reads, and oversized results are incomplete computations.
        if (!guardedDevice || (nRead <= 0) || (nRead > nToRead)) {
            setPdStructErrorString(pPdStruct, tr("Read error"));
            bCRCComputed = false;
            break;
        }

        if (crcType == CRC_TYPE_ADLER32) {
            for (qint64 i = 0; (i < nRead) && isPdStructNotCanceled(pPdStruct); i++) {
                nAdlerA = (nAdlerA + (quint8)baBuffer.at((qint32)i)) % nAdlerMod;
                nAdlerB = (nAdlerB + nAdlerA) % nAdlerMod;
            }
        } else if ((crcType == CRC_TYPE_CRC16) || (crcType == CRC_TYPE_CRC16ARC)) {
            nCRC16 = _getCRC16(baBuffer.constData(), (qint32)nRead, nCRC16, _getCRC16Table());
        } else if (crcType == CRC_TYPE_RAR14) {
            for (qint64 i = 0; (i < nRead) && isPdStructNotCanceled(pPdStruct); i++) {
                nCRC16 = (quint16)(nCRC16 + (quint8)baBuffer.at((qint32)i));
                nCRC16 = (quint16)((nCRC16 << 1) | (nCRC16 >> 15));
            }
        } else {
            nCRC32 = _getCRC32(baBuffer.constData(), (qint32)nRead, nCRC32, _getCRC32Table_EDB88320());
        }

        nProcessedSize += nRead;
        XBinary::setPdStructCurrent(pPdStruct, nFreeIndex, nProcessedSize);
        if (!guardedDevice) {
            bCRCComputed = false;
            break;
        }
    }

    qint64 nFinalSize = -1;
    if (guardedDevice) nFinalSize = guardedDevice->size();
    if (!guardedDevice || !isPdStructNotCanceled(pPdStruct) ||
        (nProcessedSize != nExpectedSize) ||
        (nFinalSize != nExpectedSize)) {
        bCRCComputed = false;
    }

    XBinary::setPdStructFinished(pPdStruct, nFreeIndex);
    if (!guardedDevice) return false;
    const bool bReset = guardedDevice->reset();
    if (!guardedDevice || !bReset) return false;

    if (!bCRCComputed) {
        return false;
    }

    quint32 nCalculatedCRC = 0;
    if (crcType == CRC_TYPE_ADLER32) {
        nCalculatedCRC = (nAdlerB << 16) | nAdlerA;
    } else if ((crcType == CRC_TYPE_CRC16) || (crcType == CRC_TYPE_CRC16ARC) || (crcType == CRC_TYPE_RAR14)) {
        nCalculatedCRC = nCRC16;
    } else if ((crcType == CRC_TYPE_EDB88320) || (crcType == CRC_TYPE_FFFFFFFF_EDB88320_FFFFFFFFF)) {
        nCalculatedCRC = nCRC32 ^ 0xFFFFFFFF;
    } else {
        nCalculatedCRC = nCRC32;
    }

    return nCalculatedCRC == nExpectedCRC;
}

quint32 XBinary::_getCRC32(qint64 nOffset, qint64 nSize, quint32 nInit, quint32 *pCRCTable, PDSTRUCT *pPdStruct)
{
    if (!pCRCTable || !isPdStructNotCanceled(pPdStruct) || (nOffset < 0) || (nSize < -1)) {
        return 0;
    }

    OFFSETSIZE osRegion = convertOffsetAndSize(nOffset, nSize);

    nOffset = osRegion.nOffset;
    nSize = osRegion.nSize;

    if ((nOffset == -1) || (nSize <= 0)) {
        return 0;
    }

    const qint32 nRequestedBufferSize = getBufferSize(pPdStruct);
    if (nRequestedBufferSize <= 0) {
        return 0;
    }
    const qint32 nBufferSize = qBound((qint32)0x1000, nRequestedBufferSize, (qint32)0x100000);
    char *pBuffer = new (std::nothrow) char[nBufferSize];
    if (!pBuffer) {
        return 0;
    }

    quint32 nResult = nInit;
    bool bReadError = false;

    qint32 _nFreeIndex = XBinary::reservePdStructRecord(pPdStruct, nSize);

    if (isPdStructNotCanceled(pPdStruct)) {
        qint64 nTemp = 0;

        while ((nSize > 0) && isPdStructNotCanceled(pPdStruct)) {
            nTemp = qMin((qint64)nBufferSize, nSize);

            if (read_array_process(nOffset, pBuffer, nTemp, pPdStruct) != nTemp) {
                setPdStructInfoString(pPdStruct, tr("Read error"));
                bReadError = true;
                break;
            }

            nResult = _getCRC32(pBuffer, nTemp, nResult, pCRCTable);

            nSize -= nTemp;
            nOffset += nTemp;

            XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, nOffset);
        }
    }

    delete[] pBuffer;
    XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);

    if (bReadError || !isPdStructNotCanceled(pPdStruct) || (nSize != 0)) {
        return 0;
    }

    // Preserve this API's historical final-XOR contract for valid ranges.
    return nResult ^ 0xFFFFFFFF;
}

quint16 XBinary::_getCRC16(qint64 nOffset, qint64 nSize, quint16 nInit, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    if (!isPdStructNotCanceled(pPdStruct) || (nOffset < 0) || (nSize < -1)) {
        return 0;
    }

    OFFSETSIZE osRegion = convertOffsetAndSize(nOffset, nSize);

    nOffset = osRegion.nOffset;
    nSize = osRegion.nSize;

    // Invalid and empty regions are not successful CRC operations in this
    // range-oriented API.  In particular, do not return init^0xFFFF for a
    // range that was never accepted.
    if ((nOffset == -1) || (nSize <= 0)) {
        return 0;
    }

    const qint32 nRequestedBufferSize = getBufferSize(pPdStruct);
    if (nRequestedBufferSize <= 0) {
        return 0;
    }
    const qint32 nBufferSize = qBound((qint32)0x1000, nRequestedBufferSize, (qint32)0x100000);
    char *pBuffer = new (std::nothrow) char[nBufferSize];
    if (!pBuffer) {
        return 0;
    }

    quint16 nResult = nInit;
    bool bReadError = false;

    qint32 _nFreeIndex = XBinary::reservePdStructRecord(pPdStruct, nSize);

    if (isPdStructNotCanceled(pPdStruct)) {
        qint64 nTemp = 0;

        while ((nSize > 0) && isPdStructNotCanceled(pPdStruct)) {
            nTemp = qMin((qint64)nBufferSize, nSize);

            if (read_array_process(nOffset, pBuffer, nTemp, pPdStruct) != nTemp) {
                setPdStructErrorString(pPdStruct, tr("Read error"));
                bReadError = true;
                break;
            }

            nResult = _getCRC16(pBuffer, nTemp, nResult, XBinary::_getCRC16Table());

            nSize -= nTemp;
            nOffset += nTemp;

            XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, nOffset);
        }
    }

    delete[] pBuffer;
    XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);

    if (bReadError || !isPdStructNotCanceled(pPdStruct) || (nSize != 0)) {
        if (!bReadError && (nSize != 0)) {
            setPdStructErrorString(pPdStruct, tr("Incomplete CRC16 read"));
        }
        return 0;
    }

    return nResult ^ 0xFFFF;
}

quint32 XBinary::_getCRC32ByFileContent(const QString &sFileName)
{
    return _getCRC32(readFile(sFileName), 0xFFFFFFFF, _getCRC32Table_EDB88320());
}

quint32 XBinary::_getCRC32ByDirectory(const QString &sDirectoryName, bool bRecursive, quint32 nInit)
{
    quint32 nResult = nInit;

    QDirIterator it(sDirectoryName);

    while (it.hasNext()) {
        QString sRecord = it.next();

        QFileInfo fi(sRecord);

        if (fi.isDir() && bRecursive) {
            nResult = _getCRC32ByDirectory(fi.absoluteFilePath(), bRecursive, nResult);
        }

        QByteArray baFileName = fi.baseName().toUtf8();
        nResult = _getCRC32(baFileName, nResult, _getCRC32Table_EDB88320());
    }

    return nResult;
}

double XBinary::getEntropy(const QString &sFileName, PDSTRUCT *pPdStruct)
{
    double dResult = 0;

    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        dResult = XBinary::getEntropy(&file, pPdStruct);

        file.close();
    }

    return dResult;
}

double XBinary::getEntropy(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    double dResult = 0;
    QPointer<QIODevice> guardedDevice(pDevice);

    if (!guardedDevice) {
        return dResult;
    }

    XBinary binary(guardedDevice.data());

    dResult = binary.getBinaryStatus(BSTATUS_ENTROPY, 0, -1, pPdStruct);
    if (!guardedDevice) return 0;

    guardedDevice->reset();
    if (!guardedDevice) return 0;

    return dResult;
}

double XBinary::getBinaryStatus(BSTATUS bstatus, qint64 nOffset, qint64 nSize, PDSTRUCT *pPdStruct)
{
    return getBinaryStatus(bstatus, nOffset, nSize, pPdStruct, 0);
}

double XBinary::getEntropy(QIODevice *pDevice, qint32 nBufferSize, PDSTRUCT *pPdStruct)
{
    double dResult = 0;
    QPointer<QIODevice> guardedDevice(pDevice);

    if (!guardedDevice || (nBufferSize <= 0)) return dResult;

    XBinary binary(guardedDevice.data());
    dResult = binary.getBinaryStatus(BSTATUS_ENTROPY, 0, -1, pPdStruct, nBufferSize);
    if (!guardedDevice) return 0;
    guardedDevice->reset();
    if (!guardedDevice) return 0;

    return dResult;
}

double XBinary::getBinaryStatus(BSTATUS bstatus, qint64 nOffset, qint64 nSize, PDSTRUCT *pPdStruct, qint32 nExplicitBufferSize)
{
    double dResult = 0;

    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    OFFSETSIZE osRegion = convertOffsetAndSize(nOffset, nSize);

    nOffset = osRegion.nOffset;
    nSize = osRegion.nSize;

    if (nSize == 0) {
        return 0;
    }

    bool bReadError = false;

    qint32 _nFreeIndex = -1;

    if ((nOffset != -1) && (!(pPdStruct->bIsStop))) {
        _nFreeIndex = XBinary::reservePdStructRecord(pPdStruct, nSize);

        // Use integer histogram for entropy for better performance
        quint64 counts[256] = {0};
        qint64 nSymbolCount = 0;  // reused for ZEROS/TEXT
        quint64 nSum = 0;         // for GRADIENT

        const qint32 nBufferSize = (nExplicitBufferSize > 0) ? nExplicitBufferSize : getBufferSize(pPdStruct);
        if (nBufferSize <= 0) {
            XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);
            return 0;
        }
        char *pBuffer = new (std::nothrow) char[nBufferSize];
        if (!pBuffer) {
            XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);
            return 0;
        }

        while ((nSize > 0) && (!(pPdStruct->bIsStop))) {
            const qint32 nChunkSize = (nSize < (qint64)nBufferSize) ? (qint32)nSize : nBufferSize;

            if (read_array(nOffset, pBuffer, nChunkSize) != nChunkSize) {
                setPdStructErrorString(pPdStruct, tr("Read error"));
                bReadError = true;

                break;
            }

            if (bstatus == BSTATUS_ENTROPY) {
                const unsigned char *ptr = reinterpret_cast<const unsigned char *>(pBuffer);
                qint32 i = 0;

                // Simple loop - let compiler optimize (unrolling can hurt cache performance)
                for (; i < nChunkSize; i++) {
                    counts[ptr[i]]++;
                }
            } else if (bstatus == BSTATUS_ZEROS) {
                const unsigned char *ptr = reinterpret_cast<const unsigned char *>(pBuffer);
                for (qint32 i = 0; i < nChunkSize; i++) {
                    nSymbolCount += (ptr[i] == 0);
                }
            } else if (bstatus == BSTATUS_GRADIENT) {
                const unsigned char *ptr = reinterpret_cast<const unsigned char *>(pBuffer);
                for (qint32 i = 0; i < nChunkSize; i++) {
                    nSum += ptr[i];
                }
            } else if (bstatus == BSTATUS_TEXT) {
                const unsigned char *ptr = reinterpret_cast<const unsigned char *>(pBuffer);

                // Simple loop - compiler optimizes better than manual unrolling for this
                for (qint32 i = 0; i < nChunkSize; i++) {
                    unsigned char c = ptr[i];
                    // ASCII printable range [32..126] plus 8(BS),10(LF),13(CR)
                    if ((c >= 32 && c <= 126) || c == 8 || c == 10 || c == 13) {
                        nSymbolCount++;
                    }
                }
            }

            nSize -= nChunkSize;
            nOffset += nChunkSize;

            XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, nOffset - osRegion.nOffset);
        }

        delete[] pBuffer;

        if ((!(pPdStruct->bIsStop)) && (!bReadError)) {
            if (bstatus == BSTATUS_ENTROPY) {
                // H = -sum(p * log2 p)
                const double invLog2 = 1.4426950408889634073599246810023;  // 1/ln(2)
                const double N = (double)osRegion.nSize;
                for (qint32 j = 0; j < 256; j++) {
                    if (counts[j]) {
                        double p = (double)counts[j] / N;
                        dResult += -p * (log(p) * invLog2);
                    }
                }
            } else if (bstatus == BSTATUS_ZEROS) {
                dResult = (double)nSymbolCount / (double)(osRegion.nSize);
            } else if (bstatus == BSTATUS_GRADIENT) {
                dResult = (double)nSum / ((double)(osRegion.nSize) * (double)0xFF);
            } else if (bstatus == BSTATUS_TEXT) {
                dResult = (double)nSymbolCount / (double)(osRegion.nSize);
            }
        }
    }

    XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);

    if (pPdStruct->bIsStop) {
        dResult = 0;
    }

    return dResult;
}

bool XBinary::isZeroFilled(qint64 nOffset, qint64 nSize, PDSTRUCT *pPdStruct)
{
    if (nSize <= 0) {
        return false;
    }

    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    if (!isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    OFFSETSIZE osRegion = convertOffsetAndSize(nOffset, nSize);
    nOffset = osRegion.nOffset;
    nSize = osRegion.nSize;

    if ((nOffset == -1) || (nSize <= 0)) {
        return false;
    }

    bool bResult = true;
    const qint32 nRequestedBufferSize = getBufferSize(pPdStruct);
    if (nRequestedBufferSize <= 0) {
        return false;
    }
    const qint32 nBufferSize = qBound((qint32)0x1000, nRequestedBufferSize, (qint32)0x100000);
    char *pBuffer = new (std::nothrow) char[nBufferSize];

    if (!pBuffer) {
        return false;
    }

    const qint64 nStartOffset = nOffset;
    const qint64 nTotalSize = nSize;
    const qint32 nFreeIndex = XBinary::reservePdStructRecord(pPdStruct, nTotalSize);

    while ((nSize > 0) && isPdStructNotCanceled(pPdStruct)) {
        qint64 nTemp = qMin((qint64)nBufferSize, nSize);

        if (read_array_process(nOffset, pBuffer, nTemp, pPdStruct) != nTemp) {
            bResult = false;
            break;
        }

        if (!_isMemoryZeroFilled(pBuffer, nTemp)) {
            bResult = false;
            break;
        }

        nSize -= nTemp;
        nOffset += nTemp;
        XBinary::setPdStructCurrent(pPdStruct, nFreeIndex, nOffset - nStartOffset);
    }

    delete[] pBuffer;
    XBinary::setPdStructFinished(pPdStruct, nFreeIndex);

    if (!isPdStructNotCanceled(pPdStruct) || (nSize != 0)) {
        bResult = false;
    }

    return bResult;
}

XBinary::BYTE_COUNTS XBinary::getByteCounts(qint64 nOffset, qint64 nSize, PDSTRUCT *pPdStruct)
{
    BYTE_COUNTS result = {};

    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    OFFSETSIZE osRegion = convertOffsetAndSize(nOffset, nSize);

    nOffset = osRegion.nOffset;
    nSize = osRegion.nSize;

    const qint32 nBufferSize = getBufferSize(pPdStruct);
    if (nBufferSize <= 0) {
        return result;
    }

    bool bReadError = false;

    qint32 _nFreeIndex = -1;

    if ((nOffset != -1) && (!(pPdStruct->bIsStop))) {
        result.nSize = nSize;

        _nFreeIndex = XBinary::reservePdStructRecord(pPdStruct, nSize);

        char *pBuffer = new (std::nothrow) char[nBufferSize];
        if (!pBuffer) {
            XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);
            return BYTE_COUNTS();
        }

        while ((nSize > 0) && (!(pPdStruct->bIsStop))) {
            const qint32 nChunkSize = (nSize < (qint64)nBufferSize) ? (qint32)nSize : nBufferSize;

            if (read_array(nOffset, pBuffer, nChunkSize) != nChunkSize) {
                _errorMessage(tr("Read error"));

                bReadError = true;

                break;
            }

            for (qint32 i = 0; i < nChunkSize; i++) {
                result.nCount[(unsigned char)pBuffer[i]] += 1;
            }

            nSize -= nChunkSize;
            nOffset += nChunkSize;

            XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, nOffset - osRegion.nOffset);
        }

        delete[] pBuffer;
    }

    XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);

    if ((pPdStruct->bIsStop) || (bReadError)) {
        result = BYTE_COUNTS();
    }

    return result;
}

void XBinary::_xor(quint8 nXorValue, qint64 nOffset, qint64 nSize, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    if (!m_pDevice || !m_pDevice->isReadable() || !m_pDevice->isWritable() || m_pDevice->isSequential() ||
        (m_pDevice->openMode() & (QIODevice::Append | QIODevice::Text)) ||
        !isPdStructNotCanceled(pPdStruct)) {
        return;
    }

    OFFSETSIZE osRegion = convertOffsetAndSize(nOffset, nSize);

    nOffset = osRegion.nOffset;
    nSize = osRegion.nSize;

    if ((nOffset == -1) || (nSize <= 0) || (nXorValue == 0)) {
        return;
    }

    const qint32 nRequestedBufferSize = getBufferSize(pPdStruct);

    if (nRequestedBufferSize <= 0) {
        return;
    }

    const qint32 nBufferSize = qBound((qint32)0x1000, nRequestedBufferSize, (qint32)0x100000);
    char *pBuffer = new (std::nothrow) char[nBufferSize];

    if (!pBuffer) {
        setPdStructErrorString(pPdStruct, tr("Allocation error"));
        return;
    }

    // Stage the complete original range before the first live write.  This
    // makes read/allocation/cancellation failures non-mutating and gives us
    // the exact bytes required to undo a partial commit.
    QIODevice *pOriginalData = createFileBuffer(nSize, pPdStruct);

    if (!pOriginalData) {
        if (isPdStructNotCanceled(pPdStruct)) {
            setPdStructErrorString(pPdStruct, tr("Cannot create temporary buffer"));
        }
        delete[] pBuffer;
        return;
    }

    const qint64 nTotalSize = nSize;
    const qint32 nFreeIndex = XBinary::reservePdStructRecord(pPdStruct, nTotalSize);
    QString sFailure;
    bool bSuccess = false;
    bool bRollbackSuccess = true;
    bool bPositionRestored = true;
    QPointer<QBuffer> pMemoryPropertyBuffer;

    // These helpers deliberately do not use safeReadData/safeWriteData: the
    // operation holds m_pReadWriteMutex for its complete transaction and the
    // public helpers would try to lock the same non-recursive mutex again.
    const auto readExactAt = [](QIODevice *pDevice, qint64 nPosition, char *pData, qint64 nLength,
                                PDSTRUCT *pProgress, bool bHonorCancellation, bool *pbValid) -> qint64 {
        qint64 nResult = 0;
        *pbValid = false;

        if (!pDevice || !pData || (nPosition < 0) || (nLength < 0)) {
            return nResult;
        }

        if ((nLength == 0) && !pDevice->seek(nPosition)) return nResult;

        while (nResult < nLength) {
            if (bHonorCancellation && !XBinary::isPdStructNotCanceled(pProgress)) {
                return nResult;
            }

            const qint64 nRemaining = nLength - nResult;
            if ((nResult > (std::numeric_limits<qint64>::max)() - nPosition) ||
                !pDevice->seek(nPosition + nResult)) {
                return nResult;
            }
            const qint64 nCurrent = pDevice->read(pData + nResult, nRemaining);

            if ((nCurrent <= 0) || (nCurrent > nRemaining)) {
                return nResult;
            }

            nResult += nCurrent;
        }

        *pbValid = !bHonorCancellation || XBinary::isPdStructNotCanceled(pProgress);
        return nResult;
    };

    const auto writeExactAt = [](QIODevice *pDevice, qint64 nPosition, const char *pData, qint64 nLength,
                                 PDSTRUCT *pProgress, bool bHonorCancellation, bool *pbValid) -> qint64 {
        qint64 nResult = 0;
        *pbValid = false;

        if (!pDevice || !pData || (nPosition < 0) || (nLength < 0)) {
            return nResult;
        }

        if ((nLength == 0) && !pDevice->seek(nPosition)) return nResult;

        while (nResult < nLength) {
            if (bHonorCancellation && !XBinary::isPdStructNotCanceled(pProgress)) {
                return nResult;
            }

            const qint64 nRemaining = nLength - nResult;
            if ((nResult > (std::numeric_limits<qint64>::max)() - nPosition) ||
                !pDevice->seek(nPosition + nResult)) {
                return nResult;
            }
            const qint64 nCurrent = pDevice->write(pData + nResult, nRemaining);

            if (nCurrent > nRemaining) {
                // A backend that violates QIODevice's write contract may have
                // touched the complete request.  Return that extent so the
                // caller restores all possibly modified bytes.
                nResult = nLength;
                return nResult;
            }

            if (nCurrent <= 0) {
                return nResult;
            }

            nResult += nCurrent;
        }

        *pbValid = !bHonorCancellation || XBinary::isPdStructNotCanceled(pProgress);
        return nResult;
    };

    {
        QMutexLocker locker(m_pReadWriteMutex);
        const qint64 nOriginalPosition = m_pDevice->pos();
        QBuffer *pQBuffer = qobject_cast<QBuffer *>(m_pDevice.data());
        QFileDevice *pFileDevice = qobject_cast<QFileDevice *>(m_pDevice.data());
        QByteArray baQBufferOriginal;
        bool bHaveQBufferSnapshot = false;
        qint64 nCommitted = 0;

        if (nOriginalPosition < 0) {
            sFailure = tr("Cannot determine device position");
        } else {
            const qint64 nDeviceSize = m_pDevice->size();

            if ((nDeviceSize < 0) || (nOffset < 0) || (nOffset > nDeviceSize) || (nTotalSize > (nDeviceSize - nOffset))) {
                sFailure = tr("Invalid XOR range");
            } else {
                if (pQBuffer) {
                    // QByteArray's implicit sharing keeps this snapshot cheap
                    // until the first live write detaches the device buffer.
                    baQBufferOriginal = pQBuffer->buffer();
                    bHaveQBufferSnapshot = true;
                }

                if (pFileDevice && !pFileDevice->flush()) {
                    sFailure = tr("Flush error");
                }
            }

            qint64 nStaged = 0;

            while (sFailure.isEmpty() && (nStaged < nTotalSize) && isPdStructNotCanceled(pPdStruct)) {
                const qint32 nTemp = (qint32)qMin((qint64)nBufferSize, nTotalSize - nStaged);
                bool bReadValid = false;
                const qint64 nRead = readExactAt(m_pDevice, nOffset + nStaged, pBuffer, nTemp, pPdStruct, true, &bReadValid);

                if (!bReadValid || (nRead != nTemp)) {
                    if (isPdStructNotCanceled(pPdStruct)) sFailure = tr("Read error");
                    break;
                }

                bool bWriteValid = false;
                const qint64 nWritten = writeExactAt(pOriginalData, nStaged, pBuffer, nTemp, pPdStruct, true, &bWriteValid);

                if (!bWriteValid || (nWritten != nTemp)) {
                    if (isPdStructNotCanceled(pPdStruct)) sFailure = tr("Temporary buffer write error");
                    break;
                }

                nStaged += nTemp;
            }

            if (sFailure.isEmpty() && (nStaged == nTotalSize) && isPdStructNotCanceled(pPdStruct)) {
                while ((nCommitted < nTotalSize) && isPdStructNotCanceled(pPdStruct)) {
                    const qint32 nTemp = (qint32)qMin((qint64)nBufferSize, nTotalSize - nCommitted);
                    bool bReadValid = false;
                    const qint64 nRead = readExactAt(pOriginalData, nCommitted, pBuffer, nTemp, pPdStruct, true, &bReadValid);

                    if (!bReadValid || (nRead != nTemp)) {
                        if (isPdStructNotCanceled(pPdStruct)) sFailure = tr("Temporary buffer read error");
                        break;
                    }

                    for (qint32 i = 0; i < nTemp; i++) {
                        pBuffer[i] ^= nXorValue;
                    }

                    bool bWriteValid = false;
                    const qint64 nWritten = writeExactAt(m_pDevice, nOffset + nCommitted, pBuffer, nTemp, pPdStruct, true, &bWriteValid);
                    nCommitted += nWritten;

                    if (!bWriteValid || (nWritten != nTemp)) {
                        if (isPdStructNotCanceled(pPdStruct)) sFailure = tr("Write error");
                        break;
                    }

                }

                if ((nCommitted == nTotalSize) && isPdStructNotCanceled(pPdStruct)) {
                    if (!pFileDevice || pFileDevice->flush()) {
                        bSuccess = true;
                    } else {
                        sFailure = tr("Flush error");
                    }
                }
            }

            if (!bSuccess && (nCommitted > 0)) {
                if (bHaveQBufferSnapshot) {
                    // Restore the complete QByteArray in one noexcept swap;
                    // this remains reliable even when an overridden QBuffer
                    // writeData() is the operation that stalled.
                    pQBuffer->buffer().swap(baQBufferOriginal);
                } else {
                    qint64 nRestored = 0;

                    while (nRestored < nCommitted) {
                        const qint32 nTemp = (qint32)qMin((qint64)nBufferSize, nCommitted - nRestored);
                        bool bReadValid = false;
                        const qint64 nRead = readExactAt(pOriginalData, nRestored, pBuffer, nTemp, nullptr, false, &bReadValid);

                        if (!bReadValid || (nRead != nTemp)) {
                            bRollbackSuccess = false;
                            break;
                        }

                        bool bWriteValid = false;
                        const qint64 nWritten = writeExactAt(m_pDevice, nOffset + nRestored, pBuffer, nTemp, nullptr, false, &bWriteValid);

                        if (!bWriteValid || (nWritten != nTemp)) {
                            bRollbackSuccess = false;
                            break;
                        }

                        nRestored += nTemp;
                    }

                    if (pFileDevice && !pFileDevice->flush()) {
                        bRollbackSuccess = false;
                    }
                }
            }

            // QObject::setProperty synchronously dispatches a dynamic-property
            // event.  Cache the refresh here, but emit that reentrant event
            // only after the non-recursive device mutex has been released and
            // every device operation is complete.
            if ((nCommitted > 0) && pQBuffer && pQBuffer->property("Memory").isValid()) {
                pMemoryPropertyBuffer = pQBuffer;
            }

            bPositionRestored = m_pDevice->seek(nOriginalPosition);
        }
    }

    if (!bSuccess && !bRollbackSuccess) {
        sFailure = sFailure.isEmpty() ? tr("XOR rollback failed; device may be partially modified")
                                      : QString("%1; %2").arg(sFailure, tr("rollback failed; device may be partially modified"));
    }

    if (!bPositionRestored) {
        sFailure = sFailure.isEmpty() ? tr("Cannot restore device position")
                                      : QString("%1; %2").arg(sFailure, tr("cannot restore device position"));
    }

    if (!sFailure.isEmpty()) {
        setPdStructErrorString(pPdStruct, sFailure);
        _errorMessage(sFailure);
    }

    XBinary::setPdStructFinished(pPdStruct, nFreeIndex);
    freeFileBuffer(&pOriginalData);
    delete[] pBuffer;

    if (pMemoryPropertyBuffer) {
        // A raw constData pointer cannot be republished safely after unlocking:
        // another writer may detach the QByteArray first.  Remove the optional
        // fast-path marker instead; normal QIODevice reads remain authoritative.
        // Keep this reentrant QObject notification as the final operation.
        pMemoryPropertyBuffer->setProperty("Memory", QVariant());
    }
}

// quint32 XBinary::_ror32(quint32 nValue, quint32 nShift)
//{
//     // TODO Check
//     nShift&=(31);
//     return (nValue>>nShift)|(nValue<<((-nShift)&31));
// }

// quint32 XBinary::_rol32(quint32 nValue, quint32 nShift)
//{
//     // TODO Check
//     nShift&=(31);
//     return (nValue<<nShift)|(nValue>>((-nShift)&31));
// }

quint32 XBinary::getStringCustomCRC32(const QString &sString)
{
    quint32 nResult = 0;  // not ~0 !!! if ~0 (0xFFFFFFFF) it will be a CRC32C

    qint32 nSize = sString.size();
    QByteArray baString = sString.toUtf8();

    for (qint32 i = 0; i < nSize; i++) {
        unsigned char _char = (unsigned char)baString.data()[i];
        //        unsigned char _char1=(unsigned char)sString.at(i).toLatin1();

        //        if(_char!=_char1)
        //        {
        //            qFatal("Error"); // TODO remove
        //        }

        nResult ^= _char;

        for (qint32 k = 0; k < 8; k++) {
            nResult = (nResult & 1) ? ((nResult >> 1) ^ 0x82f63b78) : (nResult >> 1);
        }
    }

    nResult = ~nResult;

    return nResult;
}

QIODevice *XBinary::getDevice()
{
    return m_pDevice.data();
}

quint64 XBinary::getDeviceGeneration() const
{
    return m_nDeviceGeneration;
}

bool XBinary::isDeviceReplacementAllowed() const
{
    return true;
}

bool XBinary::isValid(PDSTRUCT *pPdStruct)
{
    return isPdStructNotCanceled(pPdStruct);
}

bool XBinary::isValid(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress, PDSTRUCT *pPdStruct)
{
    XBinary xbinary(pDevice, bIsImage, nModuleAddress);

    return xbinary.isValid(pPdStruct);
}

XBinary::MODE XBinary::getMode(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    XBinary xbinary(pDevice, bIsImage, nModuleAddress);

    return xbinary.getMode();
}

bool XBinary::isBigEndian()
{
    return (getEndian() == ENDIAN_BIG);
}

bool XBinary::isLittleEndian()
{
    return (getEndian() == ENDIAN_LITTLE);
}

bool XBinary::is8()
{
    MODE mode = getMode();

    return ((mode == MODE_8));
}

bool XBinary::is16()
{
    MODE mode = getMode();

    return ((mode == MODE_16) || (mode == MODE_16SEG));
}

bool XBinary::is32()
{
    MODE mode = getMode();

    return (mode == MODE_32);
}

bool XBinary::is64()
{
    MODE mode = getMode();

    return (mode == MODE_64);
}

bool XBinary::isBigEndian(XBinary::_MEMORY_MAP *pMemoryMap)
{
    return pMemoryMap && (pMemoryMap->endian == ENDIAN_BIG);
}

bool XBinary::isLittleEndian(_MEMORY_MAP *pMemoryMap)
{
    return pMemoryMap && (pMemoryMap->endian == ENDIAN_LITTLE);
}

bool XBinary::is8(XBinary::_MEMORY_MAP *pMemoryMap)
{
    return pMemoryMap && (pMemoryMap->mode == MODE_8);
}

bool XBinary::is16(XBinary::_MEMORY_MAP *pMemoryMap)
{
    return pMemoryMap && ((pMemoryMap->mode == MODE_16) || (pMemoryMap->mode == MODE_16SEG));
}

bool XBinary::is32(XBinary::_MEMORY_MAP *pMemoryMap)
{
    return pMemoryMap && (pMemoryMap->mode == MODE_32);
}

bool XBinary::is64(XBinary::_MEMORY_MAP *pMemoryMap)
{
    return pMemoryMap && (pMemoryMap->mode == MODE_64);
}

void XBinary::setVersion(const QString &sVersion)
{
    m_sVersion = sVersion;
}

void XBinary::setOptions(const QString &sOptions)
{
    m_sOptions = sOptions;
}

QString XBinary::getVersion()
{
    return m_sVersion;
}

QString XBinary::getInfo(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    return m_sOptions;
}

bool XBinary::isEncrypted()
{
    return false;
}

bool XBinary::isCommentPresent()
{
    return false;
}

QString XBinary::getComment()
{
    return QString();
}

bool XBinary::isExportPresent()
{
    return false;
}

bool XBinary::isImportPresent()
{
    return false;
}

bool XBinary::isResourcesPresent()
{
    return false;
}

bool XBinary::isSymbolsPresent()
{
    return false;
}

QVector<XBinary::XIMPORT_STRUCT> XBinary::getImportStructs()
{
    return QVector<XIMPORT_STRUCT>();
}

QVector<XBinary::XEXPORT_STRUCT> XBinary::getExportStructs()
{
    return QVector<XEXPORT_STRUCT>();
}

QVector<XBinary::XSYMBOL_STRUCT> XBinary::getSymbolStructs()
{
    return QVector<XSYMBOL_STRUCT>();
}

QVector<XBinary::XRESOURCE_STRUCT> XBinary::getResourceStructs()
{
    return QVector<XRESOURCE_STRUCT>();
}

QString XBinary::getSignature(QIODevice *pDevice, qint64 nOffset, qint64 nSize)
{
    XBinary binary(pDevice);

    return binary.getSignature(nOffset, nSize);
}

QString XBinary::getSignature(qint64 nOffset, qint64 nSize)
{
    QString sResult;

    if (nOffset != -1) {
        OFFSETSIZE osRegion = convertOffsetAndSize(nOffset, -1);

        nSize = qMin(osRegion.nSize, nSize);

        sResult = read_array(nOffset, nSize).toHex().toUpper();
    }

    return sResult;
}

XBinary::OFFSETSIZE XBinary::convertOffsetAndSize(qint64 nOffset, qint64 nSize)
{
    OFFSETSIZE osResult = {};

    osResult.nOffset = -1;
    osResult.nSize = 0;

    const qint64 nTotalSize = getSize();

    if ((nTotalSize <= 0) || (nOffset < 0) || (nOffset >= nTotalSize) || (nSize < -1) || (nSize == 0)) {
        return osResult;
    }

    const qint64 nAvailableSize = nTotalSize - nOffset;

    if ((nSize == -1) || (nSize > nAvailableSize)) {
        nSize = nAvailableSize;
    }

    if (nSize > 0) {
        osResult.nOffset = nOffset;
        osResult.nSize = nSize;
    }

    return osResult;
}

XBinary::OFFSETSIZE XBinary::convertOffsetAndSize(QIODevice *pDevice, qint64 nOffset, qint64 nSize)
{
    XBinary binary(pDevice);

    return binary.convertOffsetAndSize(nOffset, nSize);
}

bool XBinary::compareSignatureStrings(const QString &sBaseSignature, const QString &sOptSignature)
{
    bool bResult = false;
    // TODO optimize
    // TODO check
    QString _sBaseSignature = convertSignature(sBaseSignature);
    QString _sOptSignature = convertSignature(sOptSignature);

    qint32 nSize = qMin(_sBaseSignature.size(), _sOptSignature.size());

    if ((nSize) && (_sBaseSignature.size() >= _sOptSignature.size())) {
        bResult = true;

        for (qint32 i = 0; i < nSize; i++) {
            QChar _qchar1 = _sBaseSignature.at(i);
            QChar _qchar2 = _sOptSignature.at(i);

            if ((_qchar1 != QChar('.')) && (_qchar2 != QChar('.'))) {
                if (_qchar1 != _qchar2) {
                    bResult = false;

                    break;
                }
            }
        }
    }

    return bResult;
}

void XBinary::_errorMessage(const QString &sErrorMessage, PDSTRUCT *pPdStruct)
{
#ifdef QT_DEBUG
    QFile *pFile = dynamic_cast<QFile *>(m_pDevice.data());

    if (pFile) {
        qDebug("Filename: %s", pFile->fileName().toUtf8().data());
    }

    qDebug("Error: %s", sErrorMessage.toLatin1().data());
#endif
    setPdStructInfoString(pPdStruct, sErrorMessage);
    emit errorMessage(sErrorMessage);
}

void XBinary::_infoMessage(const QString &sInfoMessage, PDSTRUCT *pPdStruct)
{
    setPdStructInfoString(pPdStruct, sInfoMessage);
#ifdef QT_DEBUG
    qDebug("Info: %s", sInfoMessage.toLatin1().data());
#endif
    emit infoMessage(sInfoMessage);
}

qint64 XBinary::_calculateRawSize(PDSTRUCT *pPdStruct)
{
    _MEMORY_MAP memoryMap = getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);

    return _calculateRawSize(&memoryMap, pPdStruct);
}

qint64 XBinary::_calculateRawSize(XBinary::_MEMORY_MAP *pMemoryMap, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    if (!pMemoryMap || (pMemoryMap->nBinarySize < 0) || !isPdStructNotCanceled(pPdStruct)) {
        return 0;
    }

    qint64 nResult = 0;

    qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

    qint64 _nOverlayOffset = -1;

    for (qint32 i = 0; (i < nNumberOfRecords) && isPdStructNotCanceled(pPdStruct); i++) {
        const _MEMORY_RECORD &record = pMemoryMap->listRecords.at(i);

        if (record.filePart == FILEPART_OVERLAY) {
            if ((record.nOffset < 0) || (record.nOffset > pMemoryMap->nBinarySize)) {
                return 0;
            }
            _nOverlayOffset = (_nOverlayOffset == -1) ? record.nOffset : qMin(_nOverlayOffset, record.nOffset);
            continue;
        }

        if (record.nOffset == -1) {
            continue;
        }

        if ((record.nOffset < 0) || (record.nSize < 0) ||
            (record.nOffset > pMemoryMap->nBinarySize) ||
            (record.nSize > (pMemoryMap->nBinarySize - record.nOffset))) {
            return 0;
        }

        nResult = qMax(nResult, record.nOffset + record.nSize);
    }

    if (!isPdStructNotCanceled(pPdStruct)) {
        return 0;
    }

    if (_nOverlayOffset != -1) {
        nResult = qMin(nResult, _nOverlayOffset);
    }

    return nResult;
}

QString XBinary::convertSignature(const QString &sSignature)
{
    QString _sSignature;
    // 'AnsiString'
    // TODO more defs

    qint32 nSignatureSize = sSignature.size();
    bool bHasQuote = false;

    // First pass: detect what conversions are needed and pre-allocate
    for (qint32 i = 0; i < nSignatureSize; i++) {
        QChar c = sSignature.at(i);
        if (c == QChar(39)) {
            bHasQuote = true;
            break;
        }
    }

    // Reserve appropriate capacity
    if (bHasQuote) {
        if (nSignatureSize > ((std::numeric_limits<qint32>::max)() / 2)) {
            return QString();
        }
        _sSignature.reserve(nSignatureSize * 2);  // Worst case: all chars become hex
    } else {
        _sSignature.reserve(nSignatureSize);
    }

    if (bHasQuote) {
        bool bAnsiString = false;

        for (qint32 i = 0; i < nSignatureSize; i++) {
            QChar c = sSignature.at(i);

            if (c == QChar(39)) {
                bAnsiString = !bAnsiString;
            } else if (bAnsiString) {
                // Inline hex conversion for better performance
                const char cLatin1 = c.toLatin1();
                if (!cLatin1 && !c.isNull()) {
                    return QString();
                }
                quint8 nValue = (quint8)cLatin1;
                quint8 nHigh = (nValue >> 4) & 0x0F;
                quint8 nLow = nValue & 0x0F;
                _sSignature.append(QChar(nHigh + (nHigh < 10 ? '0' : 'a' - 10)));
                _sSignature.append(QChar(nLow + (nLow < 10 ? '0' : 'a' - 10)));
            } else {
                if (c != QChar(' ')) {  // Skip spaces while we're at it
                    if (c == QChar('?')) {
                        _sSignature.append(QChar('.'));
                    } else {
                        _sSignature.append(c.toLower());
                    }
                }
            }
        }

        if (bAnsiString) {
            return QString();
        }
    } else {
        // Simpler path when no quotes present
        for (qint32 i = 0; i < nSignatureSize; i++) {
            QChar c = sSignature.at(i);

            if (c != QChar(' ')) {  // Skip spaces
                if (c == QChar('?')) {
                    _sSignature.append(QChar('.'));
                } else {
                    _sSignature.append(c.toLower());
                }
            }
        }
    }

    return _sSignature;
}

bool XBinary::isDebugBuild()
{
    return false;
}

bool XBinary::isReleaseBuild()
{
    return true;
}

QList<QString> XBinary::getFileFormatMessages(const QList<FMT_MSG> *pListFmtMsg)
{
    QList<QString> listResult;

    if (pListFmtMsg) {
        qint32 nNumberOfRecords = pListFmtMsg->count();

        for (qint32 i = 0; i < nNumberOfRecords; i++) {
            QString sRecord;

            if (pListFmtMsg->at(i).type == FMT_MSG_TYPE_INFO) sRecord += QString("[%1]").arg(tr("Info"));
            else if (pListFmtMsg->at(i).type == FMT_MSG_TYPE_WARNING) sRecord += QString("[%1]").arg(tr("Warning"));
            else if (pListFmtMsg->at(i).type == FMT_MSG_TYPE_ERROR) sRecord += QString("[%1]").arg(tr("Error"));

            sRecord += QString("(%1) ").arg(pListFmtMsg->at(i).code, 4, 16, QChar('0'));

            sRecord += pListFmtMsg->at(i).sString;
            listResult.append(sRecord);
        }
    }

    return listResult;
}

bool XBinary::isFmtMsgCodePresent(const QList<FMT_MSG> *pListFmtMsgs, FMT_MSG_CODE code, FMT_MSG_TYPE type, PDSTRUCT *pPdStruct)
{
    bool bResult = false;

    if (pListFmtMsgs) {
        qint32 nNumberOfRecords = pListFmtMsgs->count();

        for (qint32 i = 0; (i < nNumberOfRecords) && isPdStructNotCanceled(pPdStruct); i++) {
            if ((pListFmtMsgs->at(i).code == code) && (pListFmtMsgs->at(i).type == type)) {
                bResult = true;
                break;
            }
        }
    }

    return bResult;
}

bool XBinary::_addCheckFormatTest(QList<FMT_MSG> *pListFmtMsgs, bool *pbContinue, FMT_MSG_CODE code, FMT_MSG_TYPE type, const QString &sString, QVariant value,
                                  QString sInfo, bool bFailCase)
{
    if (!pListFmtMsgs || !pbContinue) {
        return false;
    }

    bool bResult = !bFailCase;

    if (*pbContinue) {
        if (bFailCase) {
            FMT_MSG record = {};
            record.type = type;
            record.code = code;
            record.sString += QString("%1: %2: %3").arg(sString).arg(tr("Corrupted data")).arg(sInfo);
            record.value = value;

            pListFmtMsgs->append(record);

            if (type == FMT_MSG_TYPE_ERROR) {
                *pbContinue = false;
            }
        }
    }

    return bResult;
}

void XBinary::dumpMemoryMap()
{
#ifdef QT_DEBUG
    _MEMORY_MAP memoryMap = getMemoryMap(MAPMODE_UNKNOWN);

    qDebug("%s", memoryMap.bIsImage ? "Image" : "File");
    qDebug("Binary Size: %s", valueToHex(memoryMap.nBinarySize).toLatin1().data());
    qDebug("Image Size: %s", valueToHex(memoryMap.nImageSize).toLatin1().data());
    qDebug("Module Address: %s", valueToHex(memoryMap.nModuleAddress).toLatin1().data());
    qDebug("EntryPoint Address: %s", valueToHex(memoryMap.nEntryPointAddress).toLatin1().data());
    qDebug("File Type: %s", fileTypeIdToString(memoryMap.fileType).toLatin1().data());
    qDebug("Mode: %s", modeIdToString(memoryMap.mode).toLatin1().data());
    qDebug("Endian: %s", endianToString(memoryMap.endian).toLatin1().data());
    qDebug("Arch: %s", memoryMap.sArch.toLatin1().data());
    qDebug("Type: %s", memoryMap.sType.toLatin1().data());

    qint32 nNumberOfRecords = memoryMap.listRecords.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        qDebug("--------------------------------------------------");
        qDebug("Record: %d", i);
        qDebug("Index: %d", memoryMap.listRecords.at(i).nIndex);
        qDebug("Offset: %lld", memoryMap.listRecords.at(i).nOffset);
        qDebug("Address: %lld", memoryMap.listRecords.at(i).nAddress);
        // qDebug("Segment: %s", addressSegmentToString(memoryMap.listRecords.at(i).segment).toLatin1().data());
        qDebug("Size: %lld", memoryMap.listRecords.at(i).nSize);
        // qDebug("Type: %s", mmtToString(memoryMap.listRecords.at(i).type).toLatin1().data());
        qDebug("LoadSectionNumber: %d", memoryMap.listRecords.at(i).nFilePartNumber);
        qDebug("Name: %s", memoryMap.listRecords.at(i).sName.toLatin1().data());
        qDebug("IsVirtual: %s", memoryMap.listRecords.at(i).bIsVirtual ? "true" : "false");
        qDebug("IsInvisible: %s", memoryMap.listRecords.at(i).bIsInvisible ? "true" : "false");
        // qDebug("ID: %lld", memoryMap.listRecords.at(i).nID);
        qDebug("--------------------------------------------------");
    }

#endif
}

// void XBinary::dumpHeaders()
// {
// #ifdef QT_DEBUG
//     XBinary::_MEMORY_MAP memoryMap = getMemoryMap();

//     XBinary::DATA_HEADERS_OPTIONS dataHeaderOptions = {};
//     dataHeaderOptions.locType = XBinary::LT_OFFSET;
//     dataHeaderOptions.nLocation = 0;
//     dataHeaderOptions.pMemoryMap = &memoryMap;
//     dataHeaderOptions.nID = 0;

//     QList<XBinary::DATA_HEADER> listHeaders = getDataHeaders(dataHeaderOptions, nullptr);

//     qint32 nNumberOfHeaders = listHeaders.count();

//     for (qint32 i = 0; i < nNumberOfHeaders; i++) {
//         XBinary::DATA_HEADER dataHeader = listHeaders.at(i);

//         if (dataHeader.dsID.fileType == getFileType()) {
//             qDebug("%s: %X", structIDToString(dataHeader.dsID.nID).toLatin1().data(), locationToOffset(&memoryMap, dataHeader.locType, dataHeader.nLocation));

//             QList<XBinary::DATA_RECORD_ROW> listDataRecordsRow;

//             XBinary::DATA_RECORDS_OPTIONS dataRecordsOptions = {};
//             dataRecordsOptions.pMemoryMap = &memoryMap;
//             dataRecordsOptions.dataHeaderFirst = dataHeader;

//             getDataRecordValues(dataRecordsOptions, &listDataRecordsRow, nullptr, nullptr);
//             QList<QString> listComments;

//             if (listDataRecordsRow.count() > 0) {
//                 XBinary::DATA_RECORD_ROW dataRecordRow = listDataRecordsRow.at(0);
//                 listComments = getDataRecordComments(dataRecordsOptions, dataRecordRow, nullptr);
//             }

//             qint32 nNumberOfRecords = dataHeader.listRecords.count();

//             for (qint32 j = 0; j < nNumberOfRecords; j++) {
//                 XBinary::DATA_RECORD dataRecord = dataHeader.listRecords.at(j);

//                 qDebug("%X: %X %s %s %s %s", dataRecord.nRelOffset, dataRecord.nSize, XBinary::valueTypeToString(dataRecord.valType,
//                 dataRecord.nSize).toLatin1().data(),
//                        dataRecord.sName.toLatin1().data(), XBinary::getValueString(listDataRecordsRow.at(0).listValues.at(j), dataRecord.valType,
//                        true).toLatin1().data(), listComments.at(j).toLatin1().data());
//             }
//         } else if (dataHeader.dsID.fileType == FT_BINARY) {
//             qDebug("%s: %X", XBinary::structIDToString(dataHeader.dsID.nID).toLatin1().data(), 0);
//         }
//     }
// #endif
// }

XBinary::FPART XBinary::getFPART(FILEPART filePart, const QString &sOriginalName, qint64 nFileOffset, qint64 nFileSize, XADDR nVirtualAddress, qint64 nVirtualSize)
{
    XBinary::FPART fpart = {};

    fpart.filePart = filePart;
    fpart.sName = sOriginalName;
    fpart.nFileOffset = nFileOffset;
    fpart.nFileSize = nFileSize;
    fpart.nVirtualAddress = nVirtualAddress;
    fpart.nVirtualSize = nVirtualSize;

    return fpart;
}

QList<XBinary::FPART> XBinary::getFileParts(quint32 nFileParts, qint32 nLimit, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    QList<XBinary::FPART> listResult;

    if ((nLimit < -1) || (nLimit == 0)) {
        return listResult;
    }

    if (nFileParts & FILEPART_REGION) {
        XBinary::FPART fpart = {};
        fpart.nFileOffset = 0;
        fpart.nFileSize = getSize();
        fpart.sName = tr("Data");
        fpart.filePart = FILEPART_REGION;

        listResult.append(fpart);
    }

    return listResult;
}

QString XBinary::getMIMEString()
{
    return "application/octet-stream";
}

QList<XBinary::FMT_MSG> XBinary::checkFileFormat(bool bDeep, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(bDeep)
    Q_UNUSED(pPdStruct)

    QList<XBinary::FMT_MSG> listResult;

    return listResult;
}

bool XBinary::isFileFormatValid(bool bDeep, PDSTRUCT *pPdStruct)
{
    if (!isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    bool bResult = true;

    QList<FMT_MSG> list = checkFileFormat(bDeep, pPdStruct);

    qint32 nNumberOfRecords = list.count();

    for (qint32 i = 0; (i < nNumberOfRecords) && isPdStructNotCanceled(pPdStruct); i++) {
        if (list.at(i).type == FMT_MSG_TYPE_ERROR) {
            bResult = false;
            break;
        }
    }

    return bResult && isPdStructNotCanceled(pPdStruct);
}

QList<XBinary::STRINGTABLE_RECORD> XBinary::getStringTable_ANSI(qint64 nOffset, qint64 nSize, PDSTRUCT *pPdStruct)
{
    QList<XBinary::STRINGTABLE_RECORD> listResult;

    QByteArray baStringTable = read_array_process(nOffset, nSize, pPdStruct);

    char *_pOffset = baStringTable.data();
    qint32 _nSize = baStringTable.size();

    // TODO UTF8
    for (qint32 i = 0; i < _nSize; i++) {
        XBinary::STRINGTABLE_RECORD record = {};

        record.nOffsetFromStart = i;
        record.sString = _pOffset;  // TODO
        record.nSizeInBytes = record.sString.size();

        listResult.append(record);

        _pOffset += (record.nSizeInBytes + 1);
        i += record.nSizeInBytes;
    }

    return listResult;
}

QString XBinary::qcharToHex(QChar cSymbol)
{
    // TODO mb
    return QString("%1").arg((quint8)(cSymbol.toLatin1()), 2, 16, QChar('0'));
}

QString XBinary::stringToHex(const QString &sString)
{
    return QString::fromLatin1(sString.toLatin1().toHex());
}

QString XBinary::hexToString(const QString &sHex)
{
    QString sResult;

    sResult = QByteArray::fromHex(sHex.toLatin1().data());

    return sResult;
}

QString XBinary::floatToString(float fValue, qint32 nPrec)
{
    return QString("%1").arg(fValue, 0, 'f', nPrec);
}

QString XBinary::doubleToString(double dValue, qint32 nPrec)
{
    return QString("%1").arg(dValue, 0, 'f', nPrec);
}

quint8 XBinary::hexToUint8(const QString &sHex)
{
    QString _sHex = sHex;
    quint8 nResult = 0;

    if ((quint32)_sHex.length() >= sizeof(quint8)) {
        _sHex = _sHex.mid(0, 2 * sizeof(quint8));
        bool bStatus = false;
        nResult = (quint8)(_sHex.toInt(&bStatus, 16));
    }

    return nResult;
}

qint8 XBinary::hexToInt8(const QString &sHex)
{
    QString _sHex = sHex;
    quint8 nResult = 0;

    if ((quint32)_sHex.length() >= sizeof(qint8)) {
        _sHex = _sHex.mid(0, 2 * sizeof(qint8));
        bool bStatus = false;
        nResult = (qint8)(_sHex.toInt(&bStatus, 16));
    }

    return nResult;
}

quint16 XBinary::hexToUint16(const QString &sHex, bool bIsBigEndian)
{
    QString _sHex = sHex;
    quint16 nResult = 0;

    if ((quint32)_sHex.length() >= sizeof(quint16)) {
        if (!bIsBigEndian) {
            _sHex = invertHexByteString(_sHex.mid(0, 2 * sizeof(quint16)));
        }

        bool bStatus = false;
        nResult = _sHex.toUShort(&bStatus, 16);
    }

    return nResult;
}

qint16 XBinary::hexToInt16(const QString &sHex, bool bIsBigEndian)
{
    QString _sHex = sHex;
    qint16 nResult = 0;

    if ((quint32)_sHex.length() >= sizeof(qint16)) {
        if (!bIsBigEndian) {
            _sHex = invertHexByteString(_sHex.mid(0, 2 * sizeof(qint16)));
        }

        bool bStatus = false;
        nResult = _sHex.toShort(&bStatus, 16);
    }

    return nResult;
}

quint32 XBinary::hexToUint32(const QString &sHex, bool bIsBigEndian)
{
    QString _sHex = sHex;
    quint32 nResult = 0;

    if ((quint32)_sHex.length() >= sizeof(quint32)) {
        if (!bIsBigEndian) {
            _sHex = invertHexByteString(_sHex.mid(0, 2 * sizeof(quint32)));
        }

        bool bStatus = false;
        nResult = _sHex.toUInt(&bStatus, 16);
    }

    return nResult;
}

qint32 XBinary::hexToInt32(const QString &sHex, bool bIsBigEndian)
{
    QString _sHex = sHex;
    qint32 nResult = 0;

    if ((quint32)_sHex.length() >= sizeof(qint32)) {
        if (!bIsBigEndian) {
            _sHex = invertHexByteString(_sHex.mid(0, 2 * sizeof(qint32)));
        }

        bool bStatus = false;
        nResult = _sHex.toInt(&bStatus, 16);
    }

    return nResult;
}

quint64 XBinary::hexToUint64(const QString &sHex, bool bIsBigEndian)
{
    QString _sHex = sHex;
    quint64 nResult = 0;

    if ((quint32)_sHex.length() >= sizeof(quint64)) {
        if (!bIsBigEndian) {
            _sHex = invertHexByteString(_sHex.mid(0, 2 * sizeof(quint64)));
        }

        bool bStatus = false;
        nResult = _sHex.toULongLong(&bStatus, 16);
    }

    return nResult;
}

qint64 XBinary::hexToInt64(const QString &sHex, bool bIsBigEndian)
{
    QString _sHex = sHex;
    qint64 nResult = 0;

    if ((quint32)_sHex.length() >= sizeof(qint64)) {
        if (!bIsBigEndian) {
            _sHex = invertHexByteString(_sHex.mid(0, 2 * sizeof(qint64)));
        }

        bool bStatus = false;
        nResult = _sHex.toLongLong(&bStatus, 16);
    }

    return nResult;
}

QString XBinary::invertHexByteString(const QString &sHex)
{
    QString sResult;

    for (qint32 i = sHex.length() - 2; i >= 0; i -= 2) {
        sResult += sHex.mid(i, 2);
    }

    return sResult;
}

void XBinary::_swapBytes(char *pSource, qint32 nSize)
{
    for (qint32 i = 0; i < (nSize / 2); i++) {
        char cTemp = pSource[i];
        pSource[i] = pSource[(nSize - 1) - i];
        pSource[(nSize - 1) - i] = cTemp;
    }
}

quint16 XBinary::swapBytes(quint16 nValue)
{
    _swapBytes((char *)&nValue, 2);

    return nValue;
}

quint32 XBinary::swapBytes(quint32 nValue)
{
    _swapBytes((char *)&nValue, 4);

    return nValue;
}

quint64 XBinary::swapBytes(quint64 nValue)
{
    _swapBytes((char *)&nValue, 8);

    return nValue;
}

bool XBinary::isPlainTextType()
{
    // Read a larger sample for better text detection accuracy
    QByteArray baData = read_array(0, qMin(getSize(), (qint64)0x8000));  // Increased from 0x2000 to 0x8000

    return isPlainTextType(&baData);
}

bool XBinary::isPlainTextType(QByteArray *pbaData)
{
    if (!pbaData || pbaData->isEmpty()) {
        return false;
    }

    qint32 nDataSize = pbaData->size();
    const unsigned char *pDataOffset = (const unsigned char *)(pbaData->constData());

    // Check for BOM markers - if present, not ANSI
    if (nDataSize >= 3 && pDataOffset[0] == 0xEF && pDataOffset[1] == 0xBB && pDataOffset[2] == 0xBF) {
        return false;  // UTF-8 BOM
    }

    // Check for UTF-16 BOM
    if (nDataSize >= 2) {
        if ((pDataOffset[0] == 0xFF && pDataOffset[1] == 0xFE) || (pDataOffset[0] == 0xFE && pDataOffset[1] == 0xFF)) {
            return false;  // UTF-16 BOM
        }
    }

    qint32 nNullCount = 0;
    qint32 nControlCount = 0;
    qint32 nPrintableCount = 0;
    qint32 nExtendedCount = 0;

    // Analyze character distribution for ANSI compatibility
    for (qint32 i = 0; i < nDataSize; i++) {
        unsigned char byte = pDataOffset[i];

        if (byte == 0) {
            nNullCount++;
            // Any null bytes indicate binary data
            if (nNullCount > 0) {
                return false;
            }
        } else if (byte < 0x09) {
            // Control characters (excluding null)
            nControlCount++;
        } else if (byte == 0x09 || byte == 0x0A || byte == 0x0D) {
            // Tab, LF, CR - common whitespace
            nPrintableCount++;
        } else if (byte >= 0x20 && byte <= 0x7E) {
            // Standard ASCII printable characters
            nPrintableCount++;
        } else if (byte >= 0x80 && byte <= 0xFF) {
            // Extended ASCII (ANSI code page characters)
            nExtendedCount++;
        } else {
            // Other control characters (0x0B, 0x0C, 0x0E-0x1F)
            nControlCount++;
        }
    }

    // Calculate ratios for ANSI text detection
    double printableRatio = (double)(nPrintableCount + nExtendedCount) / nDataSize;
    double controlRatio = (double)nControlCount / nDataSize;
    double extendedRatio = (double)nExtendedCount / nDataSize;

    // ANSI text should have:
    // - High printable ratio (including extended ASCII)
    // - Low control character ratio
    // - Extended ASCII allowed but not dominant
    return (printableRatio >= 0.85 && controlRatio <= 0.05 && extendedRatio <= 0.50);
}

bool XBinary::isUTF8TextType()
{
    QByteArray baData = read_array(0, qMin(getSize(), (qint64)0x2000));  // Larger sample for better detection

    return isUTF8TextType(&baData);
}

bool XBinary::isUTF8TextType(QByteArray *pbaData)
{
    if (!pbaData || pbaData->isEmpty()) {
        return false;
    }

    const unsigned char *pDataOffset = (const unsigned char *)(pbaData->constData());
    qint32 nDataSize = pbaData->size();

    // Check for UTF-8 BOM
    bool bHasBOM = false;
    qint32 nStartOffset = 0;

    if (nDataSize >= 3 && pDataOffset[0] == 0xEF && pDataOffset[1] == 0xBB && pDataOffset[2] == 0xBF) {
        bHasBOM = true;
        nStartOffset = 3;
    }

    // Validate UTF-8 encoding from start offset
    qint32 nValidChars = 0;
    qint32 nMultiByteChars = 0;
    qint32 nPrintableChars = 0;
    qint32 nNullBytes = 0;

    for (qint32 i = nStartOffset; i < nDataSize;) {
        unsigned char byte = pDataOffset[i];

        if (byte == 0) {
            nNullBytes++;
            // UTF-8 text shouldn't contain null bytes
            if (nNullBytes > 0) {
                return false;
            }
            i++;
        } else if (byte < 0x80) {
            // ASCII character (0x00-0x7F)
            if (byte >= 0x20 || byte == 0x09 || byte == 0x0A || byte == 0x0D) {
                nPrintableChars++;
            }
            nValidChars++;
            i++;
        } else if ((byte & 0xE0) == 0xC0) {
            // 2-byte UTF-8 sequence (110xxxxx 10xxxxxx)
            if (i + 1 >= nDataSize || (pDataOffset[i + 1] & 0xC0) != 0x80) {
                return false;  // Invalid UTF-8 sequence
            }
            // Check for overlong encoding
            if (byte < 0xC2) {
                return false;  // Overlong 2-byte sequence
            }
            nMultiByteChars++;
            nValidChars++;
            i += 2;
        } else if ((byte & 0xF0) == 0xE0) {
            // 3-byte UTF-8 sequence (1110xxxx 10xxxxxx 10xxxxxx)
            if (i + 2 >= nDataSize || (pDataOffset[i + 1] & 0xC0) != 0x80 || (pDataOffset[i + 2] & 0xC0) != 0x80) {
                return false;  // Invalid UTF-8 sequence
            }
            // Check for overlong encoding
            if (byte == 0xE0 && pDataOffset[i + 1] < 0xA0) {
                return false;  // Overlong 3-byte sequence
            }
            nMultiByteChars++;
            nValidChars++;
            i += 3;
        } else if ((byte & 0xF8) == 0xF0) {
            // 4-byte UTF-8 sequence (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx)
            if (i + 3 >= nDataSize || (pDataOffset[i + 1] & 0xC0) != 0x80 || (pDataOffset[i + 2] & 0xC0) != 0x80 || (pDataOffset[i + 3] & 0xC0) != 0x80) {
                return false;  // Invalid UTF-8 sequence
            }
            // Check for overlong encoding and valid Unicode range
            if (byte == 0xF0 && pDataOffset[i + 1] < 0x90) {
                return false;  // Overlong 4-byte sequence
            }
            if (byte > 0xF4 || (byte == 0xF4 && pDataOffset[i + 1] > 0x8F)) {
                return false;  // Beyond valid Unicode range
            }
            nMultiByteChars++;
            nValidChars++;
            i += 4;
        } else {
            // Invalid UTF-8 start byte
            return false;
        }
    }

    // For UTF-8 detection, we need either:
    // 1. UTF-8 BOM present, or
    // 2. Valid UTF-8 sequences with some multi-byte characters and high printable ratio
    if (bHasBOM) {
        return nValidChars > 0;  // BOM present and valid UTF-8 content
    } else if (nValidChars > 0) {
        double printableRatio = (double)nPrintableChars / nValidChars;
        double multiByteRatio = (double)nMultiByteChars / nValidChars;

        // Require some multi-byte characters and high printable ratio for UTF-8 without BOM
        return (multiByteRatio > 0.05 && printableRatio >= 0.70);
    }

    return false;
}

bool XBinary::isPlainTextType(QIODevice *pDevice)
{
    XBinary binary(pDevice);

    return binary.isPlainTextType();
}

XBinary::UNICODE_TYPE XBinary::getUnicodeType()
{
    QByteArray baData = read_array(0, qMin(getSize(), (qint64)0x1000));  // Larger sample for better detection

    return getUnicodeType(&baData);
}

XBinary::UNICODE_TYPE XBinary::getUnicodeType(QByteArray *pbaData)
{
    if (!pbaData || pbaData->isEmpty()) {
        return XBinary::UNICODE_TYPE_NONE;
    }

    const unsigned char *pDataOffset = (const unsigned char *)(pbaData->constData());
    qint32 nDataSize = pbaData->size();

    // Check for BOM first (most reliable method)
    if (nDataSize >= 2) {
        quint16 nSymbol = qFromLittleEndian(*((quint16 *)(pDataOffset)));

        if (nSymbol == 0xFFFE) {
            return UNICODE_TYPE_BE;  // UTF-16 Big Endian BOM
        } else if (nSymbol == 0xFEFF) {
            return UNICODE_TYPE_LE;  // UTF-16 Little Endian BOM
        }
    }

    // If no BOM, try to detect by content analysis
    if (nDataSize >= 4) {
        qint32 nNullCount = 0;
        qint32 nEvenNulls = 0;  // Null bytes at even positions
        qint32 nOddNulls = 0;   // Null bytes at odd positions
        qint32 nPrintableCount = 0;
        qint32 nValidChars = 0;

        // Analyze first part of data for patterns
        qint32 nSampleSize = qMin(nDataSize, 512);
        for (qint32 i = 0; i < nSampleSize; i++) {
            unsigned char byte = pDataOffset[i];

            if (byte == 0) {
                nNullCount++;
                if (i % 2 == 0) {
                    nEvenNulls++;
                } else {
                    nOddNulls++;
                }
            } else if (byte >= 0x20 && byte <= 0x7E) {
                nPrintableCount++;
            }

            nValidChars++;
        }

        // UTF-16 typically has null bytes in alternating positions
        if (nNullCount > 0 && nValidChars > 4) {
            double nullRatio = (double)nNullCount / nValidChars;
            double printableRatio = (double)nPrintableCount / nValidChars;

            // For UTF-16, expect significant null presence and reasonable printable content
            if (nullRatio >= 0.30 && printableRatio >= 0.30) {
                // Determine endianness by null byte pattern
                if (nEvenNulls > nOddNulls * 2) {
                    // More nulls at even positions suggests UTF-16 LE (ASCII chars have null high byte)
                    return UNICODE_TYPE_LE;
                } else if (nOddNulls > nEvenNulls * 2) {
                    // More nulls at odd positions suggests UTF-16 BE
                    return UNICODE_TYPE_BE;
                }

                // If pattern is unclear, try byte pair analysis
                qint32 nLELikelyPairs = 0;
                qint32 nBELikelyPairs = 0;

                for (qint32 i = 0; i < nSampleSize - 1; i += 2) {
                    quint16 wChar = qFromLittleEndian(*((quint16 *)(pDataOffset + i)));

                    // Check if it's a likely ASCII character in LE format
                    if ((wChar & 0xFF00) == 0 && (wChar & 0xFF) >= 0x20 && (wChar & 0xFF) <= 0x7E) {
                        nLELikelyPairs++;
                    }

                    // Check if it's a likely ASCII character in BE format
                    wChar = qFromBigEndian(*((quint16 *)(pDataOffset + i)));
                    if ((wChar & 0xFF00) == 0 && (wChar & 0xFF) >= 0x20 && (wChar & 0xFF) <= 0x7E) {
                        nBELikelyPairs++;
                    }
                }

                if (nLELikelyPairs > nBELikelyPairs) {
                    return UNICODE_TYPE_LE;
                } else if (nBELikelyPairs > nLELikelyPairs) {
                    return UNICODE_TYPE_BE;
                }
            }
        }
    }

    return XBinary::UNICODE_TYPE_NONE;
}

bool XBinary::tryToOpen(QIODevice *pDevice)
{
    QPointer<QIODevice> guardedDevice(pDevice);
    if (!guardedDevice) {
        return false;
    }

    bool bResult = false;
    bool bCheck = false;

    QFile *pFile = dynamic_cast<QFile *>(guardedDevice.data());

    if (pFile) {
        bCheck = (pFile->fileName() != "");
    } else {
        bCheck = true;
    }

    if (bCheck) {
        bResult = guardedDevice->open(QIODevice::ReadWrite);

        if (!guardedDevice) return false;
        if (!bResult) {
            bResult = guardedDevice->open(QIODevice::ReadOnly);
        }
    }

    return guardedDevice && bResult;
}

bool XBinary::checkOffsetSize(XBinary::OFFSETSIZE osRegion)
{
    qint64 nTotalSize = getSize();

    return (nTotalSize >= 0) && (osRegion.nOffset >= 0) && (osRegion.nSize > 0) &&
           (osRegion.nOffset < nTotalSize) && (osRegion.nSize <= (nTotalSize - osRegion.nOffset));
}

bool XBinary::checkOffsetSize(qint64 nOffset, qint64 nSize)
{
    XBinary::OFFSETSIZE os = {};
    os.nOffset = nOffset;
    os.nSize = nSize;

    return checkOffsetSize(os);
}

QString XBinary::get_uint8_full_version(quint8 nValue)
{
    return QString("%1").arg(QString::number((nValue) & 0xFF));
}

QString XBinary::get_uint16_full_version(quint16 nValue)
{
    return QString("%1.%2").arg(QString::number((nValue >> 8) & 0xFF)).arg(QString::number((nValue) & 0xFF));
}

QString XBinary::get_uint32_full_version(quint32 nValue)
{
    return QString("%1.%2.%3").arg(QString::number((nValue >> 16) & 0xFFFF)).arg(QString::number((nValue >> 8) & 0xFF)).arg(QString::number((nValue) & 0xFF));
}

QString XBinary::get_uint64_full_version(quint64 nValue)
{
    QString sResult;

    quint32 nValue1 = (nValue >> 32) & 0xFFFFFFFF;
    quint32 nValue2 = nValue & 0xFFFFFFFF;

    sResult = QString("%1.%2").arg(get_uint32_full_version(nValue1)).arg(get_uint32_full_version(nValue2));

    return sResult;
}

QString XBinary::get_uint16_version(quint16 nValue)
{
    return QString("%1").arg(QString::number((nValue) & 0xFFFF));
}

QString XBinary::get_uint32_version(quint32 nValue)
{
    return QString("%1.%2").arg(QString::number((nValue >> 16) & 0xFFFF)).arg(QString::number((nValue) & 0xFFFF));
}

bool XBinary::isResizeEnable(QIODevice *pDevice)
{
    return pDevice && (qobject_cast<QBuffer *>(pDevice) || qobject_cast<QFileDevice *>(pDevice));
}

bool XBinary::resize(QIODevice *pDevice, qint64 nSize)
{
    QPointer<QIODevice> guardedDevice(pDevice);
    if (!guardedDevice || (nSize < 0)) {
        return false;
    }

    if (QBuffer *pBuffer = qobject_cast<QBuffer *>(guardedDevice.data())) {
        if (nSize > (std::numeric_limits<qint32>::max)()) {
            return false;
        }

        pBuffer->buffer().resize((qint32)nSize);
        return guardedDevice;
    }

    if (QFileDevice *pFileDevice =
            qobject_cast<QFileDevice *>(guardedDevice.data())) {
        const bool bResult = pFileDevice->resize(nSize);
        return guardedDevice && bResult;
    }

    return false;
}

XBinary::PACKED_UINT XBinary::read_uleb128(qint64 nOffset, qint64 nSize)
{
    PACKED_UINT result = {};

    const qint64 nDeviceSize = getSize();
    if ((nOffset < 0) || (nSize <= 0) || (nOffset >= nDeviceSize)) {
        return result;
    }

    const qint64 nAvailable = qMin(nSize, nDeviceSize - nOffset);
    const qint32 nEncodedLimit = (qint32)qMin<qint64>(nAvailable, 10);
    quint32 nShift = 0;

    for (qint32 i = 0; i < nEncodedLimit; i++) {
        quint8 nByte = read_uint8(nOffset + i);
        result.nByteSize++;

        // A uint64 ULEB128 is at most ten bytes. The tenth byte contributes
        // only bit 63; any other payload bit or continuation is overflow.
        if ((i == 9) && (((nByte & 0x7F) > 1) || (nByte & 0x80))) {
            return result;
        }

        result.nValue |= (quint64(nByte & 0x7F) << nShift);
        nShift += 7;

        if ((nByte & 0x80) == 0) {
            result.bIsValid = true;
            break;
        }
    }

    return result;
}

XBinary::PACKED_UINT XBinary::_read_uleb128(const char *pData, qint64 nSize)
{
    PACKED_UINT result = {};

    if (!pData || (nSize <= 0)) {
        return result;
    }

    const qint32 nEncodedLimit = (qint32)qMin<qint64>(nSize, 10);
    quint32 nShift = 0;

    for (qint32 i = 0; i < nEncodedLimit; i++) {
        quint8 nByte = (quint8)(*(pData + i));
        result.nByteSize++;

        if ((i == 9) && (((nByte & 0x7F) > 1) || (nByte & 0x80))) {
            return result;
        }

        result.nValue |= (quint64(nByte & 0x7F) << nShift);
        nShift += 7;

        if ((nByte & 0x80) == 0) {
            result.bIsValid = true;
            break;
        }
    }

    return result;
}

XBinary::PACKED_UINT XBinary::read_acn1_integer(qint64 nOffset, qint64 nSize)
{
    PACKED_UINT result = {};

    if (nSize > 0) {
        quint8 nByte = read_uint8(nOffset);

        if ((nByte & 0x80) == 0) {
            result.bIsValid = true;
            result.nByteSize = 1;
            result.nValue = nByte;
        } else {
            quint8 _nSize = (nByte & 0x7F);

            // Indefinite form (_nSize == 0) is not allowed here
            // Ensure we have at least 1 + _nSize bytes available and cap to 4 bytes
            if ((_nSize != 0) && (_nSize <= 4) && (1 + (qint64)_nSize <= nSize)) {
                result.bIsValid = true;
                result.nByteSize = 1 + _nSize;

                for (qint32 i = 0; i < _nSize; i++) {
                    result.nValue <<= 8;
                    result.nValue |= read_uint8(nOffset + 1 + i);
                }
            }
        }
    }

    return result;
}

XBinary::PACKED_UINT XBinary::_read_acn1_integer(char *pData, qint64 nSize)
{
    PACKED_UINT result = {};

    if ((pData != nullptr) && (nSize > 0)) {
        quint8 nByte = (quint8)(*pData);

        if ((nByte & 0x80) == 0) {
            result.bIsValid = true;
            result.nByteSize = 1;
            result.nValue = nByte;
        } else {
            quint8 _nSize = (nByte & 0x7F);

            if ((_nSize != 0) && (_nSize <= 4) && (1 + (qint64)_nSize <= nSize)) {
                result.bIsValid = true;
                result.nByteSize = 1 + _nSize;

                quint64 nVal = 0;
                for (qint32 i = 0; i < _nSize; i++) {
                    nVal <<= 8;
                    nVal |= (quint8)(*(pData + 1 + i));
                }
                result.nValue = nVal;
            }
        }
    }

    return result;
}

QString XBinary::read_ASN_OIDString(qint64 nOffset, qint64 nSize)
{
    QString sResult;

    if ((nOffset < 0) || (nSize <= 0) || (nSize > (std::numeric_limits<qint32>::max)())) {
        return sResult;
    }

    const QByteArray baData = read_array(nOffset, nSize);
    if (baData.size() != nSize) {
        return sResult;
    }

    QList<quint64> listSubIdentifiers;
    qint32 nIndex = 0;

    while (nIndex < baData.size()) {
        quint64 nValue = 0;
        bool bTerminated = false;
        bool bFirstByte = true;

        while (nIndex < baData.size()) {
            const quint8 nByte = (quint8)baData.at(nIndex++);
            const quint8 nPayload = nByte & 0x7F;

            // DER requires the shortest base-128 form.  A leading 0x80 would
            // otherwise permit arbitrarily many redundant bytes per arc.
            if (bFirstByte && (nByte == 0x80)) {
                return QString();
            }
            bFirstByte = false;

            if (nValue > (((std::numeric_limits<quint64>::max)() - nPayload) >> 7)) {
                return QString();
            }
            nValue = (nValue << 7) | nPayload;

            if ((nByte & 0x80) == 0) {
                bTerminated = true;
                break;
            }
        }

        if (!bTerminated) {
            return QString();
        }

        listSubIdentifiers.append(nValue);
    }

    if (listSubIdentifiers.isEmpty()) {
        return sResult;
    }

    const quint64 nFirstCombined = listSubIdentifiers.takeFirst();
    const quint64 nFirstArc = (nFirstCombined < 40) ? 0 : ((nFirstCombined < 80) ? 1 : 2);
    const quint64 nSecondArc = nFirstCombined - (nFirstArc * 40);
    sResult = QString("%1.%2").arg(QString::number(nFirstArc)).arg(QString::number(nSecondArc));

    for (quint64 nValue : listSubIdentifiers) {
        sResult += QString(".%1").arg(QString::number(nValue));
    }

    return sResult;
}

qint64 XBinary::read_ASN_Integer(qint64 nOffset, qint64 nSize)
{
    qint64 nResult = 0;

    PACKED_UINT packedInt = read_acn1_integer(nOffset, nSize);

    if (packedInt.bIsValid) {
        nResult = packedInt.nValue;
    }

    return nResult;
}

bool XBinary::read_ASN_Bool(qint64 nOffset, qint64 nSize)
{
    if (nSize != 1) return false;
    char cValue = 0;
    return (read_array(nOffset, &cValue, 1) == 1) && (cValue != 0);  // per BER, any non-zero is TRUE
}

QDateTime XBinary::read_ASN_DateTime(qint64 nOffset, qint64 nSize)
{
    // Try UTCTime (YYMMDDHHMMSSZ) then GeneralizedTime (YYYYMMDDHHMMSSZ)
    if ((nSize != 11) && (nSize != 13) && (nSize != 15)) {
        return QDateTime();
    }

    QByteArray bytes = read_array(nOffset, nSize);
    if (bytes.size() != nSize) {
        return QDateTime();
    }

    QString s = QString::fromLatin1(bytes.constData(), bytes.size());
    QDateTime dt;
    // Ensure ends with 'Z' for UTC; handle missing seconds too
    static const QStringList fmts = {"yyMMddHHmmss'Z'", "yyMMddHHmm'Z'", "yyyyMMddHHmmss'Z'", "yyyyMMddHHmm'Z'"};
    for (const QString &f : fmts) {
        dt = QDateTime::fromString(s, f);
        if (dt.isValid()) {
            dt.setTimeSpec(Qt::UTC);
            return dt;
        }
    }
    return QDateTime();
}

QString XBinary::read_ASN_AnsiString(qint64 nOffset, qint64 nSize)
{
    if ((nSize <= 0) || (nSize > (std::numeric_limits<qint32>::max)())) return QString();
    QByteArray bytes = read_array(nOffset, nSize);
    if (bytes.size() != nSize) return QString();
    // Trim potential trailing NULs
    int trim = bytes.size();
    while (trim > 0 && bytes.at(trim - 1) == '\0') trim--;
    return QString::fromLatin1(bytes.constData(), trim);
}

XBinary::PACKED_UINT XBinary::_read_packedNumber(char *pData, qint64 nSize)
{
    PACKED_UINT result = {};

    if (!pData || (nSize <= 0)) {
        return result;
    }

    quint8 nFirstByte = (quint8)(*(pData));

    result.nByteSize = 0;

    // if ((nFirstByte & 0x80) == 0) {  // 0xxxxxxx
    //     result.nValue = nFirstByte & 0x7F;
    //     result.nByteSize = 1;
    // } else if ((nFirstByte & 0xC0) == 0x80) {  // 10xxxxxx
    //     result.nValue = nFirstByte & 0x3F;
    //     result.nByteSize = 2;
    // } else if ((nFirstByte & 0xE0) == 0xC0) {  // 110xxxxx
    //     result.nValue = nFirstByte & 0x1F;
    //     result.nByteSize = 3;
    // } else if ((nFirstByte & 0xF0) == 0xE0) {  // 1110xxxx
    //     result.nValue = nFirstByte & 0x0F;
    //     result.nByteSize = 4;
    // } else if ((nFirstByte & 0xF8) == 0xF0) {  // 11110xxx
    //     result.nValue = nFirstByte & 0x07;
    //     result.nByteSize = 5;
    // } else if ((nFirstByte & 0xFC) == 0xF8) {  // 111110xx
    //     result.nValue = nFirstByte & 0x03;
    //     result.nByteSize = 6;
    // } else if ((nFirstByte & 0xFE) == 0xFC) {  // 1111110x
    //     result.nValue = nFirstByte & 0x01;
    //     result.nByteSize = 7;
    // } else if (nFirstByte == 0xFE) {  // 11111110
    //     result.nValue = 0;
    //     result.nByteSize = 8;
    // } else if (nFirstByte == 0xFF) {  // 11111111
    //     result.nValue = 0;
    //     result.nByteSize = 9;
    // }

    // if (result.nByteSize <= nSize) {
    //     result.bIsValid = true;

    //     for (qint32 i = 1; i < result.nByteSize; ++i) {
    //         quint8 _nByte = (quint8)(*(pData + i));
    //         result.nValue |= static_cast<quint32>(_nByte) << (8 * (i - 1));
    //     }
    // }

    result.nByteSize = 1;

    if (nFirstByte < 0x80) {
        // Single byte
        result.nValue = nFirstByte;
        result.bIsValid = true;
    } else {
        // Multi-byte encoding
        quint8 nMask = 0x80;
        result.nByteSize = 1;

        for (qint32 i = 0; i < 8; i++) {
            if (nFirstByte & nMask) {
                if (result.nByteSize < nSize) {
                    result.nValue |= ((quint64)(quint8)pData[result.nByteSize] << (8 * i));
                    result.nByteSize++;
                } else {
                    break;
                }
            } else {
                result.nValue |= ((quint64)(nFirstByte & (nMask - 1)) << (8 * i));
                result.bIsValid = true;
                break;
            }
            nMask >>= 1;
        }

        // 0xFF is the legal nine-byte form: the first byte carries no value
        // bits and the following eight bytes are the complete little-endian
        // UInt64. There is no zero marker bit in this one case.
        if ((nFirstByte == 0xFF) && (result.nByteSize == 9)) {
            result.bIsValid = true;
        }
    }

    return result;
}

QList<QString> XBinary::getListFromFile(const QString &sFileName)
{
    QList<QString> listResult;

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString sLine = in.readLine().trimmed();
            if (sLine != "") {
                listResult.append(sLine);
            }
        }

        file.close();
    }

    return listResult;
}

bool XBinary::_handleOverlay(_MEMORY_MAP *pMemoryMap)
{
    bool bResult = false;

    if (!pMemoryMap || (pMemoryMap->nBinarySize < 0)) {
        return false;
    }

    qint64 nTotalSize = pMemoryMap->nBinarySize;

    // get maximal offset
    qint64 nMaxOffset = 0;
    qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        const _MEMORY_RECORD &record = pMemoryMap->listRecords.at(i);

        if (record.filePart == FILEPART_OVERLAY) {
            return false;
        }

        if (record.nOffset == -1) {
            continue;
        }

        if ((record.nOffset < 0) || (record.nSize < 0) ||
            (record.nOffset > nTotalSize) || (record.nSize > (nTotalSize - record.nOffset))) {
            return false;
        }

        nMaxOffset = qMax(nMaxOffset, record.nOffset + record.nSize);
    }

    if (nMaxOffset < nTotalSize) {
        // overlay present
        qint64 nOverlayOffset = nMaxOffset;
        qint64 nOverlaySize = nTotalSize - nOverlayOffset;

        _MEMORY_RECORD record = {};
        record.nOffset = nOverlayOffset;
        record.nSize = nOverlaySize;
        record.sName = tr("Overlay");
        record.filePart = FILEPART_OVERLAY;
        record.nAddress = -1;  // TODO
        // record.nID = 0;        // TODO
        record.bIsVirtual = false;
        record.bIsInvisible = false;
        record.nIndex = pMemoryMap->listRecords.count();

        pMemoryMap->listRecords.append(record);

        bResult = true;
    }

    return bResult;
}

qint64 XBinary::getOverlaySize(PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    _MEMORY_MAP memoryMap = getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);

    return getOverlaySize(&memoryMap, pPdStruct);
}

qint64 XBinary::getOverlaySize(XBinary::_MEMORY_MAP *pMemoryMap, PDSTRUCT *pPdStruct)
{
    if (!pMemoryMap || (pMemoryMap->nBinarySize < 0)) {
        return 0;
    }

    qint64 nSize = pMemoryMap->nBinarySize;
    qint64 nOverlayOffset = getOverlayOffset(pMemoryMap, pPdStruct);

    if ((nOverlayOffset < 0) || (nOverlayOffset > nSize)) {
        return 0;
    }

    return nSize - nOverlayOffset;
}

qint64 XBinary::getOverlayOffset(PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    _MEMORY_MAP memoryMap = getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);

    return getOverlayOffset(&memoryMap, pPdStruct);
}

qint64 XBinary::getOverlayOffset(XBinary::_MEMORY_MAP *pMemoryMap, PDSTRUCT *pPdStruct)
{
    if (!pMemoryMap) {
        return -1;
    }

    qint64 nResult = -1;
    qint64 nRawSize = _calculateRawSize(pMemoryMap, pPdStruct);

    if ((nRawSize > 0) && (nRawSize <= pMemoryMap->nBinarySize)) {
        nResult = nRawSize;
    }

    return nResult;
}

bool XBinary::isOverlayPresent(PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    _MEMORY_MAP memoryMap = getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);

    return isOverlayPresent(&memoryMap, pPdStruct);
}

bool XBinary::isOverlayPresent(XBinary::_MEMORY_MAP *pMemoryMap, PDSTRUCT *pPdStruct)
{
    return (getOverlaySize(pMemoryMap, pPdStruct) > 0);
}

bool XBinary::compareOverlay(const QString &sSignature, qint64 nOffset, PDSTRUCT *pPdStruct)
{
    _MEMORY_MAP memoryMap = getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);

    return compareOverlay(&memoryMap, sSignature, nOffset, pPdStruct);
}

bool XBinary::compareOverlay(XBinary::_MEMORY_MAP *pMemoryMap, const QString &sSignature, qint64 nOffset, PDSTRUCT *pPdStruct)
{
    if (!pMemoryMap || (nOffset < 0)) {
        return false;
    }

    const qint64 nOverlaySize = getOverlaySize(pMemoryMap, pPdStruct);
    const qint64 nBaseOffset = getOverlayOffset(pMemoryMap, pPdStruct);

    if ((nOverlaySize <= 0) || (nBaseOffset < 0) || (nOffset >= nOverlaySize) ||
        (nBaseOffset > (std::numeric_limits<qint64>::max)() - nOffset)) {
        return false;
    }

    return compareSignature(pMemoryMap, sSignature, nBaseOffset + nOffset, pPdStruct);
}

bool XBinary::addOverlay(char *pData, qint64 nDataSize, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();
    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    if (!m_pDevice || !m_pDevice->isWritable() || (nDataSize < 0) ||
        ((nDataSize > 0) && !pData) || !isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    const qint64 nRawSize = getOverlayOffset(pPdStruct);
    const qint64 nOldSize = m_pDevice->size();

    if ((nRawSize < 0) || (nOldSize < 0) || (nRawSize > nOldSize) ||
        (nDataSize > (std::numeric_limits<qint64>::max)() - nRawSize)) {
        return false;
    }

    const qint64 nNewSize = nRawSize + nDataSize;
    if (!resize(m_pDevice, nNewSize)) {
        return false;
    }

    bool bResult = (nDataSize == 0) ||
                   (safeWriteData(m_pDevice, nRawSize, pData, nDataSize, pPdStruct) == nDataSize);

    if (!bResult) {
        resize(m_pDevice, nOldSize);
    }

    // resize() operates on the device directly; refresh the cached extent and
    // invalidate any format-specific parse state before returning to callers.
    setDevice(m_pDevice.data());

    return bResult;
}

bool XBinary::addOverlay(const QString &sFileName, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    if (!m_pDevice || !m_pDevice->isWritable() || !isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        const QFile *pDestinationFile = dynamic_cast<const QFile *>(m_pDevice.data());
        if (pDestinationFile) {
            QFileInfo destinationInfo(pDestinationFile->fileName());
            QFileInfo sourceInfo(file.fileName());
            QString sDestinationPath = destinationInfo.canonicalFilePath();
            QString sSourcePath = sourceInfo.canonicalFilePath();
            if (sDestinationPath.isEmpty()) sDestinationPath = QDir::cleanPath(destinationInfo.absoluteFilePath());
            if (sSourcePath.isEmpty()) sSourcePath = QDir::cleanPath(sourceInfo.absoluteFilePath());
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
            const Qt::CaseSensitivity caseSensitivity = Qt::CaseInsensitive;
#else
            const Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive;
#endif
            if (!sDestinationPath.isEmpty() && !sSourcePath.isEmpty() &&
                (QString::compare(QDir::fromNativeSeparators(sDestinationPath),
                                  QDir::fromNativeSeparators(sSourcePath), caseSensitivity) == 0)) {
                file.close();
                return false;
            }
        }

        const qint64 nRawSize = getOverlayOffset(pPdStruct);
        const qint64 nDataSize = file.size();
        const qint64 nOldSize = m_pDevice->size();

        if ((nRawSize < 0) || (nDataSize < 0) || (nOldSize < 0) || (nRawSize > nOldSize) ||
            (nDataSize > (std::numeric_limits<qint64>::max)() - nRawSize) ||
            !resize(m_pDevice, nRawSize + nDataSize)) {
            file.close();
            return false;
        }

        bool bResult = (nDataSize == 0) || copyDeviceMemory(&file, 0, m_pDevice, nRawSize, nDataSize, pPdStruct);
        if (!bResult) {
            resize(m_pDevice, nOldSize);
        }

        setDevice(m_pDevice.data());
        file.close();
        return bResult;
    }

    return false;
}

bool XBinary::removeOverlay()
{
    return addOverlay(0, 0);
}

bool XBinary::isSignatureInFilePartPresent(qint32 nFilePartNumber, const QString &sSignature)
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return isSignatureInFilePartPresent(&memoryMap, nFilePartNumber, sSignature);
}

bool XBinary::isSignatureInFilePartPresent(XBinary::_MEMORY_MAP *pMemoryMap, qint32 nFilePartNumber, const QString &sSignature, PDSTRUCT *pPdStruct)
{
    bool bResult = false;

    if (!pMemoryMap) {
        return false;
    }

    qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

    for (qint32 i = 0; (i < nNumberOfRecords) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        if (pMemoryMap->listRecords.at(i).nFilePartNumber == nFilePartNumber) {
            if (pMemoryMap->listRecords.at(i).nOffset != -1) {
                bResult = isSignaturePresent(pMemoryMap, pMemoryMap->listRecords.at(i).nOffset, pMemoryMap->listRecords.at(i).nSize, sSignature, pPdStruct);

                break;
            }
        }
    }

    return bResult;
}

QString XBinary::getStringCollision(const QList<QString> *pListStrings, const QString &sString1, const QString &sString2)
{
    // TODO Check&optimize
    QString sResult;

    if (!pListStrings) {
        return sResult;
    }

    qint32 nNumberOfStrings = pListStrings->count();

    QString sRoot1;
    QString sRoot2;

    for (qint32 i = 0; i < nNumberOfStrings; i++) {
        QString sCurrentString = pListStrings->at(i);

        if (sCurrentString.contains(sString1)) {
            sRoot1 = sCurrentString.section(sString1, 0, 0);
        }

        if ((sRoot1 != "") && sCurrentString.contains(sString2)) {
            sRoot2 = sCurrentString.section(sString2, 0, 0);

            break;
        }
    }

    if ((sRoot1 != "") && (sRoot1 == sRoot2)) {
        sResult = sRoot1;
    }

    return sResult;
}

bool XBinary::writeToFile(const QString &sFileName, const QByteArray &baData)
{
    return dumpToFile(sFileName, baData.constData(), baData.size());
}

bool XBinary::writeToFile(const QString &sFileName, QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    return dumpToFile(sFileName, pDevice, pPdStruct);
}

bool XBinary::appendToFile(const QString &sFileName, const QString &sString)
{
    QString _sString = sString;
    bool bResult = false;

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadWrite | QIODevice::Append)) {
        _sString += "\r\n";  // TODO Linux
        QByteArray baData = _sString.toUtf8();
        bResult = writeAllToDevice(&file, baData.constData(), baData.size());
        file.close();
    }

    return bResult;
}

bool XBinary::clearFile(const QString &sFileName)
{
    if (sFileName.trimmed().isEmpty()) {
        return false;
    }

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadWrite)) {
        const bool bResult = file.resize(0);
        file.close();
        return bResult;
    }

    return false;
}

qint32 XBinary::getStringNumberFromList(const QList<QString> *pListStrings, const QString &sString, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    qint32 nResult = -1;

    if (!pListStrings) {
        return nResult;
    }

    qint32 nNumberOfRecords = pListStrings->count();

    for (qint32 i = 0; (i < nNumberOfRecords) && (!(pPdStruct->bIsStop)); i++) {
        if (pListStrings->at(i) == sString) {
            nResult = i;

            break;
        }
    }

    return nResult;
}

qint32 XBinary::getStringNumberFromListExp(const QList<QString> *pListStrings, const QString &sString, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    qint32 nResult = -1;

    if (!pListStrings) {
        return nResult;
    }

    qint32 nNumberOfRecords = pListStrings->count();

    for (qint32 i = 0; (i < nNumberOfRecords) && (!(pPdStruct->bIsStop)); i++) {
        if (isRegExpPresent(sString, pListStrings->at(i))) {
            nResult = i;

            break;
        }
    }

    return nResult;
}

bool XBinary::isStringInListPresent(const QList<QString> *pListStrings, const QString &sString, PDSTRUCT *pPdStruct)
{
    return (getStringNumberFromList(pListStrings, sString, pPdStruct) != -1);
}

bool XBinary::isStringInListPresentExp(const QList<QString> *pListStrings, const QString &sString, PDSTRUCT *pPdStruct)
{
    return (getStringNumberFromListExp(pListStrings, sString, pPdStruct) != -1);
}

QString XBinary::getStringByIndex(const QList<QString> *pListStrings, qint32 nIndex, qint32 nNumberOfStrings)
{
    QString sResult;

    if (!pListStrings) {
        return sResult;
    }

    if (nNumberOfStrings == -1) {
        nNumberOfStrings = pListStrings->count();
    }

    nNumberOfStrings = qMin(nNumberOfStrings, pListStrings->count());

    if ((nIndex >= 0) && (nIndex < nNumberOfStrings)) {
        sResult = pListStrings->at(nIndex);
    }

    return sResult;
}

bool XBinary::isStringUnicode(const QString &sString, qint32 nMaxCheckSize)
{
    // TODO Optimize
    // TODO Check
    bool bResult = false;

    if (nMaxCheckSize == -1) {
        nMaxCheckSize = sString.size();
    } else {
        nMaxCheckSize = qMin(sString.size(), nMaxCheckSize);
    }

    for (qint32 i = 0; i < nMaxCheckSize; i++) {
        unsigned char cChar = sString.at(i).toLatin1();
        if ((cChar > 127) || (cChar < 27)) {
            bResult = true;
            break;
        }
    }

    return bResult;
}

quint32 XBinary::elfHash(const quint8 *pData)
{
    quint32 nResult = 0;

    while (*pData) {
        nResult = (nResult << 4) + (*pData);

        quint32 nHigh = nResult & 0xF0000000;

        if (nHigh) {
            nResult ^= (nResult >> 24);
        }

        nResult &= (~nHigh);

        pData++;
    }

    return nResult;
}

QString XBinary::getVersionString(const QString &sString)
{
    QString sResult;

    qint32 nSize = sString.size();

    for (qint32 i = 0; i < nSize; i++) {
        QChar c = sString.at(i);

        if (((QChar('9') >= c) && (c >= QChar('0'))) || (c == QChar('.'))) {
            sResult.append(c);
        } else {
            break;
        }
    }

    return sResult;
}

qint64 XBinary::getVersionIntValue(const QString &sString)
{
    qint64 nResult = 0;

    qint32 nNumberOfDots = sString.count(QChar('.'));

    for (qint32 i = 0; i < (nNumberOfDots + 1); i++) {
        nResult *= 10;

        nResult += sString.section(QChar('.'), i, i).toLongLong();
    }

    return nResult;
}

bool XBinary::checkStringNumber(const QString &sString, quint32 nMin, quint32 nMax)
{
    bool bResult = false;

    quint32 nValue = sString.toUInt();

    bResult = ((nValue >= nMin) && (nValue <= nMax));

    return bResult;
}

QDateTime XBinary::valueToTime(quint64 nValue, DT_TYPE type)
{
    QDateTime result;

    if (type == DT_TYPE_POSIX) {
        result.setMSecsSinceEpoch(nValue * 1000);
    } else if (type == DT_TYPE_UNIXTIME) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
        result = QDateTime::fromSecsSinceEpoch((quint32)nValue, Qt::UTC);
#else
        result = QDateTime::fromMSecsSinceEpoch((quint32)nValue * 1000, Qt::UTC);
#endif
    } else if (type == DT_TYPE_DOSTIME) {
        // MS-DOS time is always a 16-bit packed value; mask higher bits explicitly.
        quint16 nDosTime = (quint16)(nValue & 0xFFFF);
        // Use a valid dummy date (1980-01-01 = 0x0021) since we only need the time component
        result = dosDateTimeToQDateTime(0x0021, nDosTime);
    } else if (type == DT_TYPE_DOSDATE) {
        // MS-DOS date is always a 16-bit packed value; mask higher bits explicitly.
        quint16 nDosDate = (quint16)(nValue & 0xFFFF);
        result = dosDateTimeToQDateTime(nDosDate, 0);
    } else if (type == DT_TYPE_FILETIME) {
        result = winFileTimeToQDateTime(nValue);
    }

    return result;
}

QString XBinary::valueToTimeString(quint64 nValue, XBinary::DT_TYPE type)
{
    QString sResult;

    if (type == DT_TYPE_DOSTIME) {
        QDateTime dateTime = valueToTime(nValue, type);
        if (dateTime.isValid()) {
            sResult = dateTime.time().toString("HH:mm:ss");
        }
    } else if (type == DT_TYPE_DOSDATE) {
        QDateTime dateTime = valueToTime(nValue, type);
        if (dateTime.isValid()) {
            sResult = dateTime.date().toString("yyyy-MM-dd");
        }
    } else {
        QDateTime dateTime = valueToTime(nValue, type);
        if (dateTime.isValid()) {
            sResult = dateTime.toString("yyyy-MM-dd hh:mm:ss");
        }
    }

    return sResult;
}

QString XBinary::msecToDate(quint64 nValue)
{
    QString sResult;

    quint64 _nTmp = nValue / 1000;

    quint64 nSec = _nTmp % 60;
    _nTmp /= 60;
    quint64 nMin = _nTmp % 60;
    _nTmp /= 60;
    quint64 nHour = _nTmp % 24;
    _nTmp /= 24;
    quint64 nDay = _nTmp;

    if (nDay > 0) {
        sResult += QString("%1:").arg(nDay, 2, 10, QChar('0'));
    }

    sResult += QString("%1:").arg(nHour, 2, 10, QChar('0'));
    sResult += QString("%1:").arg(nMin, 2, 10, QChar('0'));
    sResult += QString("%1").arg(nSec, 2, 10, QChar('0'));

    return sResult;
}

QDateTime XBinary::dosDateTimeToQDateTime(quint16 nDosDate, quint16 nDosTime)
{
    if (!isValidDosDateTime(nDosDate, nDosTime)) {
        return QDateTime();
    }

    // MS-DOS date format: bits 0-4 day (1-31), 5-8 month (1-12), 9-15 years from 1980
    // MS-DOS time format: bits 0-4 seconds/2 (0-29 => 0-58), 5-10 minutes (0-59), 11-15 hours (0-23)
    qint32 nDay = (nDosDate & 0x1F);
    qint32 nMonth = (nDosDate >> 5) & 0x0F;
    qint32 nYear = ((nDosDate >> 9) & 0x7F) + 1980;

    qint32 nSecond = (nDosTime & 0x1F) * 2;
    qint32 nMinute = (nDosTime >> 5) & 0x3F;
    qint32 nHour = (nDosTime >> 11) & 0x1F;

    QDate date(nYear, nMonth, nDay);
    QTime time(nHour, nMinute, nSecond);

    QDateTime result;
    if (date.isValid() && time.isValid()) {
        result = QDateTime(date, time);
    }

    return result;
}

bool XBinary::isValidDosDateTime(quint16 nDosDate, quint16 nDosTime)
{
    return isValidDosDate(nDosDate) && isValidDosTime(nDosTime);
}

bool XBinary::isValidDosDate(quint16 nDosDate)
{
    const qint32 nDay = (nDosDate & 0x1F);
    const qint32 nMonth = (nDosDate >> 5) & 0x0F;
    const qint32 nYear = ((nDosDate >> 9) & 0x7F) + 1980;

    if ((nDay < 1) || (nDay > 31) || (nMonth < 1) || (nMonth > 12)) {
        return false;
    }

    if ((nYear < 1980) || (nYear > 2107)) {
        return false;
    }

    return QDate(nYear, nMonth, nDay).isValid();
}

bool XBinary::isValidDosTime(quint16 nDosTime)
{
    const qint32 nSecond = (nDosTime & 0x1F) * 2;
    const qint32 nMinute = (nDosTime >> 5) & 0x3F;
    const qint32 nHour = (nDosTime >> 11) & 0x1F;

    if ((nSecond > 58) || (nMinute > 59) || (nHour > 23)) {
        return false;
    }

    return true;
}

QPair<quint16, quint16> XBinary::qDateTimeToDosDateTime(const QDateTime &dateTime)
{
    QPair<quint16, quint16> result = {0, 0};

    if (dateTime.isValid()) {
        QDate date = dateTime.date();
        QTime time = dateTime.time();

        quint16 nDosDate = 0;
        quint16 nDosTime = 0;

        if (date.isValid()) {
            qint32 nYear = date.year();
            if (nYear >= 1980 && nYear <= 2107) {
                nDosDate = (date.day() & 0x1F) | ((date.month() & 0x0F) << 5) | (((nYear - 1980) & 0x7F) << 9);
            }
        }

        if (time.isValid()) {
            nDosTime = ((time.second() / 2) & 0x1F) | ((time.minute() & 0x3F) << 5) | ((time.hour() & 0x1F) << 11);
        }

        if (isValidDosDateTime(nDosDate, nDosTime)) {
            result = {nDosDate, nDosTime};
        }
    }

    return result;
}

QString XBinary::valueToFlagsString(quint64 nValue, const QMap<quint64, QString> &mapFlags, VL_TYPE vlType)
{
    QString sResult;

    if (vlType == VL_TYPE_LIST) {
        sResult = mapFlags.value(nValue);
    } else if (vlType == VL_TYPE_FLAGS) {
        QMapIterator<quint64, QString> iter(mapFlags);

        while (iter.hasNext()) {
            iter.next();

            quint64 nFlag = iter.key();

            if (nValue & nFlag) {
                if (sResult != "") {
                    sResult += "|";
                }

                sResult += iter.value();
            }
        }
    }

    return sResult;
}

bool XBinary::isX86asm(const QString &sArch)
{
    // TODO remove, use getDisasmMode
    bool bResult = false;

    QString _sArch = sArch.toUpper();

    // TODO Check
    if ((_sArch == "8086") || (_sArch == "80286") || (_sArch == "80386") || (_sArch == "80486") || (_sArch == "80586") || (_sArch == "386") || (_sArch == "I386") ||
        (_sArch == "AMD64") || (_sArch == "X86_64")) {
        bResult = true;
    }

    return bResult;
}

QString XBinary::disasmIdToString(XBinary::DM disasmMode)
{
    QString sResult = tr("Unknown");

    switch (disasmMode) {
        case DM_DATA: sResult = tr("Data"); break;
        case DM_8086: sResult = QString("8086 mode"); break;
        case DM_X86_32: sResult = QString("x86 32-bit mode"); break;
        case DM_X86_64: sResult = QString("x86 64-bit mode"); break;
        case DM_ARM_LE: sResult = QString("ARM"); break;
        case DM_ARM_BE: sResult = QString("ARM, big endian"); break;
        case DM_AARCH64_LE: sResult = QString("AArch64"); break;
        case DM_AARCH64_BE: sResult = QString("AArch64, big endian"); break;
        case DM_CORTEXM: sResult = QString("CORTEXM"); break;
        case DM_THUMB_LE: sResult = QString("THUMB"); break;
        case DM_THUMB_BE: sResult = QString("THUMB BE"); break;
        case DM_MIPS_LE: sResult = QString("MIPS"); break;
        case DM_MIPS_BE: sResult = QString("MIPS BE"); break;
        case DM_MIPS64_LE: sResult = QString("MIPS64"); break;
        case DM_MIPS64_BE: sResult = QString("MIPS64 BE"); break;
        case DM_PPC_LE: sResult = QString("PPC"); break;
        case DM_PPC_BE: sResult = QString("PPC BE"); break;
        case DM_PPC64_LE: sResult = QString("PPC64"); break;
        case DM_PPC64_BE: sResult = QString("PPC64 BE"); break;
        case DM_SPARC: sResult = QString("Sparc"); break;
        case DM_SPARCV9: sResult = QString("Sparc V9"); break;
        case DM_S390X: sResult = QString("S390X"); break;
        case DM_XCORE: sResult = QString("XCORE"); break;
        case DM_M68K: sResult = QString("M68K"); break;
        case DM_M68K00: sResult = QString("M68K00"); break;
        case DM_M68K10: sResult = QString("M68K10"); break;
        case DM_M68K20: sResult = QString("M68K20"); break;
        case DM_M68K30: sResult = QString("M68K30"); break;
        case DM_M68K40: sResult = QString("M68K40"); break;
        case DM_M68K60: sResult = QString("M68K60"); break;
        case DM_TMS320C64X: sResult = QString("TMS320C64X"); break;
        case DM_M6800: sResult = QString("M6800"); break;
        case DM_M6801: sResult = QString("M6801"); break;
        case DM_M6805: sResult = QString("M6805"); break;
        case DM_M6808: sResult = QString("M6808"); break;
        case DM_M6809: sResult = QString("M6809"); break;
        case DM_M6811: sResult = QString("M6811"); break;
        case DM_CPU12: sResult = QString("CPU12"); break;
        case DM_HD6301: sResult = QString("HD6301"); break;
        case DM_HD6309: sResult = QString("HD6309"); break;
        case DM_HCS08: sResult = QString("HCS08"); break;
        case DM_EVM: sResult = QString("EVM"); break;
        case DM_RISKV32: sResult = QString("RISKV32"); break;
        case DM_RISKV64: sResult = QString("RISKV64"); break;
        case DM_RISKVC: sResult = QString("RISKVC"); break;
        case DM_MOS65XX: sResult = QString("MOS65XX"); break;
        case DM_WASM: sResult = QString("WASM"); break;
        case DM_BPF_LE: sResult = QString("BPF LE"); break;
        case DM_BPF_BE: sResult = QString("BPF BE"); break;
        case DM_CUSTOM: sResult = tr("Custom"); break;
        case DM_CUSTOM_MACH_REBASE: sResult = QString("MACH REBASE"); break;
        case DM_CUSTOM_MACH_BIND: sResult = QString("MACH BIND"); break;
        case DM_CUSTOM_MACH_WEAK: sResult = QString("MACH WEAK"); break;
        case DM_CUSTOM_MACH_EXPORT: sResult = QString("MACH EXPORT"); break;
        case DM_CUSTOM_7ZIP_PROPERTIES: sResult = QString("7ZIP PROPERTIES"); break;
        default: sResult = tr("Unknown");
    }

    return sResult;
}

QString XBinary::disasmIdToArch(DM disasmMode)
{
    QString sResult = QString("data");

    switch (disasmMode) {
        case DM_DATA: sResult = QString("data"); break;
        case DM_8086:
        case DM_X86_32:
        case DM_X86_64: sResult = QString("x86"); break;
        case DM_ARM_LE:
        case DM_ARM_BE:
        case DM_CORTEXM:
        case DM_THUMB_LE:
        case DM_THUMB_BE: sResult = QString("ARM"); break;
        case DM_AARCH64_LE:
        case DM_AARCH64_BE: sResult = QString("AArch64"); break;
        case DM_MIPS_LE:
        case DM_MIPS_BE:
        case DM_MIPS64_LE:
        case DM_MIPS64_BE: sResult = QString("MIPS"); break;
        case DM_PPC_LE:
        case DM_PPC_BE:
        case DM_PPC64_LE:
        case DM_PPC64_BE: sResult = QString("PowerPC"); break;
        case DM_SPARC:
        case DM_SPARCV9: sResult = QString("Sparc"); break;
        case DM_S390X: sResult = QString("S390X"); break;
        case DM_XCORE: sResult = QString("XCORE"); break;
        case DM_M68K:
        case DM_M68K00:
        case DM_M68K10:
        case DM_M68K20:
        case DM_M68K30:
        case DM_M68K40:
        case DM_M68K60: sResult = QString("M68K"); break;
        case DM_TMS320C64X: sResult = QString("TMS320C64X"); break;
        case DM_M6800:
        case DM_M6801:
        case DM_M6805:
        case DM_M6808:
        case DM_M6809:
        case DM_M6811:
        case DM_CPU12:
        case DM_HD6301:
        case DM_HD6309:
        case DM_HCS08: sResult = QString("Motorola"); break;
        case DM_EVM: sResult = QString("EVM"); break;
        case DM_RISKV32:
        case DM_RISKV64:
        case DM_RISKVC: sResult = QString("RISC-V"); break;
        case DM_MOS65XX: sResult = QString("MOS"); break;
        case DM_WASM: sResult = QString("WebAssembly"); break;
        case DM_BPF_LE:
        case DM_BPF_BE: sResult = QString("BPF"); break;
        default: sResult = tr("Unknown");
    }

    return sResult;
}

QString XBinary::syntaxIdToString(SYNTAX syntax)
{
    QString sResult = tr("Unknown");

    switch (syntax) {
        case SYNTAX_DEFAULT: sResult = tr("Default"); break;
        case SYNTAX_ARM: sResult = QString("ARM"); break;
        case SYNTAX_ATT: sResult = QString("ATT"); break;
        case SYNTAX_AMD: sResult = QString("AMD"); break;
        case SYNTAX_INTEL: sResult = QString("INTEL"); break;
        case SYNTAX_MASM: sResult = QString("MASM"); break;
        case SYNTAX_MOTOROLA: sResult = QString("MOTOROLA"); break;
        case SYNTAX_MIPS: sResult = QString("MIPS"); break;
    }

    return sResult;
}

XBinary::SYNTAX XBinary::stringToSyntaxId(const QString &sString)
{
    SYNTAX result = SYNTAX_DEFAULT;

    if (sString == "ATT") result = SYNTAX_ATT;
    else if (sString == "ARM") result = SYNTAX_ARM;
    else if (sString == "AMD") result = SYNTAX_AMD;
    else if (sString == "INTEL") result = SYNTAX_INTEL;
    else if (sString == "MASM") result = SYNTAX_MASM;
    else if (sString == "MOTOROLA") result = SYNTAX_MOTOROLA;
    else if (sString == "MIPS") result = SYNTAX_MIPS;

    return result;
}

QString XBinary::osNameIdToString(OSNAME osName)
{
    QString sResult = tr("Unknown");

    switch (osName) {
        case OSNAME_MULTIPLATFORM: sResult = tr("Multiplatform"); break;
        case OSNAME_AIX: sResult = QString("AIX"); break;
        case OSNAME_ALPINELINUX: sResult = QString("Alpine Linux"); break;
        case OSNAME_AMIGA: sResult = QString("Amiga"); break;
        case OSNAME_ANDROID: sResult = QString("Android"); break;
        case OSNAME_AROS: sResult = QString("Amiga Research OS"); break;
        case OSNAME_ASPLINUX: sResult = QString("ASPLinux"); break;
        case OSNAME_ATARIST: sResult = QString("Atari ST"); break;
        case OSNAME_BORLANDOSSERVICES: sResult = QString("Borland OS Services"); break;
        case OSNAME_BRIDGEOS: sResult = QString("bridgeOS"); break;
        case OSNAME_DEBIANLINUX: sResult = QString("Debian Linux"); break;
        case OSNAME_FENIXOS: sResult = QString("FenixOS"); break;
        case OSNAME_FREEBSD: sResult = QString("FreeBSD"); break;
        case OSNAME_GENTOOLINUX: sResult = QString("Gentoo Linux"); break;
        case OSNAME_HANCOMLINUX: sResult = QString("Hancom Linux"); break;
        case OSNAME_HPUX: sResult = QString("Hewlett-Packard HP-UX"); break;
        case OSNAME_IOS: sResult = QString("iOS"); break;
        case OSNAME_IPADOS: sResult = QString("iPadOS"); break;
        case OSNAME_IPHONEOS: sResult = QString("iPhone OS"); break;
        case OSNAME_IRIX: sResult = QString("IRIX"); break;
        case OSNAME_LINUX: sResult = QString("Linux"); break;
        case OSNAME_MACOS: sResult = QString("macOS"); break;
        case OSNAME_MAC_OS: sResult = QString("Mac OS"); break;
        case OSNAME_MAC_OS_X: sResult = QString("Mac OS X"); break;
        case OSNAME_MACCATALYST: sResult = QString("Mac Catalyst"); break;
        case OSNAME_MANDRAKELINUX: sResult = QString("Mandrake Linux"); break;
        case OSNAME_MACDRIVERKIT: sResult = QString("Mac DriverKit"); break;
        case OSNAME_MACFIRMWARE: sResult = QString("Mac Firmware"); break;
        case OSNAME_MCLINUX: sResult = QString("mClinux"); break;
        case OSNAME_MINIX: sResult = QString("Minix"); break;
        case OSNAME_MODESTO: sResult = QString("Novell Modesto"); break;
        case OSNAME_MSDOS: sResult = QString("MS-DOS"); break;
        case OSNAME_FREEDOS: sResult = QString("FreeDOS"); break;
        case OSNAME_NETBSD: sResult = QString("NetBSD"); break;
        case OSNAME_NSK: sResult = QString("Hewlett-Packard Non-Stop Kernel"); break;
        case OSNAME_OPENBSD: sResult = QString("OpenBSD"); break;
        case OSNAME_OPENVMS: sResult = QString("Open VMS"); break;
        case OSNAME_OPENVOS: sResult = QString("Open VOS"); break;
        case OSNAME_OS2: sResult = QString("OS/2"); break;
        case OSNAME_OS_X: sResult = QString("OS X"); break;
        case OSNAME_POSIX: sResult = QString("Posix"); break;
        case OSNAME_QNX: sResult = QString("QNX"); break;
        case OSNAME_REDHATLINUX: sResult = QString("Red Hat Linux"); break;
        case OSNAME_SEPOS: sResult = QString("sepOS"); break;
        case OSNAME_SOLARIS: sResult = QString("Sun Solaris"); break;
        case OSNAME_STARTOSLINUX: sResult = QString("StartOS Linux"); break;
        case OSNAME_SUNOS: sResult = QString("SunOS"); break;
        case OSNAME_SUSELINUX: sResult = QString("SUSE Linux"); break;
        case OSNAME_SYLLABLE: sResult = QString("Syllable"); break;
        case OSNAME_TRU64: sResult = QString("Compaq TRU64 UNIX"); break;
        case OSNAME_TURBOLINUX: sResult = QString("Turbolinux"); break;
        case OSNAME_TVOS: sResult = QString("tvOS"); break;
        case OSNAME_UBUNTULINUX: sResult = QString("Ubuntu Linux"); break;
        case OSNAME_UEFI: sResult = QString("UEFI"); break;
        case OSNAME_UNIX: sResult = QString("Unix"); break;
        case OSNAME_VINELINUX: sResult = QString("Vine Linux"); break;
        case OSNAME_WATCHOS: sResult = QString("watchOS"); break;
        case OSNAME_REACTOS: sResult = QString("ReactOS"); break;
        case OSNAME_WINDOWS: sResult = QString("Windows"); break;
        case OSNAME_WINDOWSCE: sResult = QString("Windows CE"); break;
        case OSNAME_WINDOWSPHONE: sResult = QString("Windows Phone"); break;
        case OSNAME_WINDRIVERLINUX: sResult = QString("Wind River Linux"); break;
        case OSNAME_XBOX: sResult = QString("XBOX"); break;
        case OSNAME_PLAYSTATION: sResult = QString("PlayStation"); break;
        case OSNAME_JVM: sResult = QString("JVM"); break;
        case OSNAME_CHROMEOS: sResult = QString("ChromeOS"); break;
        case OSNAME_NINTENDOSWITCH: sResult = QString("Nintendo Switch"); break;
        case OSNAME_WEBOS: sResult = QString("webOS"); break;
        case OSNAME_HARMONYOS: sResult = QString("Harmony OS"); break;
        case OSNAME_ARCHLINUX: sResult = QString("Arch Linux"); break;

        default: sResult = tr("Unknown");
    }

    return sResult;
}

XBinary::DM XBinary::getDisasmMode()
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return getDisasmMode(&memoryMap);
}

XBinary::DM XBinary::getDisasmMode(XBinary::_MEMORY_MAP *pMemoryMap)
{
    return pMemoryMap ? getDisasmMode(pMemoryMap->sArch, (pMemoryMap->endian == ENDIAN_BIG), pMemoryMap->mode) : DM_UNKNOWN;
}

XBinary::DM XBinary::getDisasmMode(const QString &sArch, bool bIsBigEndian, MODE mode)
{
    XBinary::DM dmResult = DM_UNKNOWN;

    QString _sArch = sArch.toUpper();

    if (_sArch == "PPC") {
        if (bIsBigEndian) {
            dmResult = DM_PPC_BE;
        } else {
            dmResult = DM_PPC_LE;
        }
    } else if (_sArch == "PPC64") {
        if (bIsBigEndian) {
            dmResult = DM_PPC64_BE;
        } else {
            dmResult = DM_PPC64_LE;
        }
    } else if ((_sArch == "MIPS") || (_sArch == "R3000") || (_sArch == "R4000") || (_sArch == "R10000") || (_sArch == "WCEMIPSV2")) {
        if (mode == MODE_64) {
            if (bIsBigEndian) {
                dmResult = DM_MIPS64_BE;
            } else {
                dmResult = DM_MIPS64_LE;
            }
        } else {
            if (bIsBigEndian) {
                dmResult = DM_MIPS_BE;
            } else {
                dmResult = DM_MIPS_LE;
            }
        }
    } else if ((_sArch == "ARM") || (_sArch == "ARMNT") || (_sArch == "THUMB") || (_sArch == "ARM_V6") || (_sArch == "ARM_V7") || (_sArch == "ARM_V7S") ||
               (_sArch == "ARM64_32")) {
        if (bIsBigEndian) {
            dmResult = DM_ARM_BE;
        } else {
            dmResult = DM_ARM_LE;
        }
    } else if ((_sArch == "AARCH64") || (_sArch == "ARM64") || (_sArch == "ARM64E") || (_sArch == "ARM_A500")) {
        if (bIsBigEndian) {
            dmResult = DM_AARCH64_BE;
        } else {
            dmResult = DM_AARCH64_LE;
        }
    } else if ((_sArch == "8086") || (_sArch == "286")) {
        dmResult = DM_8086;
    } else if ((_sArch == "386") || (_sArch == "80386") || (_sArch == "80486") || (_sArch == "80586") || (_sArch == "I386") || (_sArch == "486") || (_sArch == "X86")) {
        if ((mode == MODE_16) || (mode == MODE_16SEG)) {
            dmResult = DM_8086;
        } else {
            dmResult = DM_X86_32;
        }
    } else if ((_sArch == "AMD64") || (_sArch == "X86_64") || (_sArch == "X64")) {
        dmResult = DM_X86_64;
    } else if ((_sArch == "68K") || (_sArch == "MC680x0")) {
        dmResult = DM_M68K;
    } else if ((_sArch == "MC68030") || (_sArch == "MC68030_ONLY")) {
        dmResult = DM_M68K30;
    } else if (_sArch == "MC68040") {
        dmResult = DM_M68K40;
    } else if (_sArch == "POWERPC") {
        if (bIsBigEndian) {
            dmResult = DM_PPC_BE;
        } else {
            dmResult = DM_PPC_LE;
        }
    } else if (_sArch == "POWERPC_BE") {
        dmResult = DM_PPC_BE;
    } else if (_sArch == "SPARC") {
        dmResult = DM_SPARC;
    } else if (_sArch == "SPARCV9") {
        dmResult = DM_SPARCV9;
    } else if (_sArch == "S390") {
        dmResult = DM_S390X;
    } else if ((_sArch == "RISC_V") || (_sArch == "RISCV32") || (_sArch == "RISCV64")) {
        if (mode == MODE_64) {
            dmResult = DM_RISKV64;
        } else {
            dmResult = DM_RISKV32;
        }
    }
    // else if (_sArch == "MACH_REBASE") {
    //     dmResult = DM_CUSTOM_MACH_REBASE;
    // }
    // TODO SH
    // TODO more

    return dmResult;
}

XBinary::DM XBinary::getDisasmMode(FILEFORMATINFO *pFileFormatInfo)
{
    return pFileFormatInfo ? getDisasmMode(pFileFormatInfo->sArch, (pFileFormatInfo->endian == ENDIAN_BIG), pFileFormatInfo->mode) : DM_UNKNOWN;
}

XBinary::ARCH XBinary::fileFormatInfoToArch(const XBinary::FILEFORMATINFO *pFileFormatInfo)
{
    XBinary::ARCH result = XBinary::ARCH_UNKNOWN;

    if (pFileFormatInfo) {
        XBinary::DM disasmMode = XBinary::getDisasmMode(pFileFormatInfo->sArch, (pFileFormatInfo->endian == XBinary::ENDIAN_BIG), pFileFormatInfo->mode);

        switch (disasmMode) {
            case XBinary::DM_8086: result = XBinary::ARCH_8086; break;
            case XBinary::DM_X86_32: result = XBinary::ARCH_X86_32; break;
            case XBinary::DM_X86_64: result = XBinary::ARCH_X86_64; break;
            default: break;
        }
    }

    return result;
}

XBinary::DMFAMILY XBinary::getDisasmFamily(XBinary::DM disasmMode)
{
    DMFAMILY result = DMFAMILY_UNKNOWN;

    if ((disasmMode == DM_8086) || (disasmMode == DM_X86_32) || (disasmMode == DM_X86_64)) {
        result = DMFAMILY_X86;
    } else if ((disasmMode == DM_ARM_BE) || (disasmMode == DM_ARM_LE)) {
        result = DMFAMILY_ARM;
    } else if ((disasmMode == DM_AARCH64_BE) || (disasmMode == DM_AARCH64_LE)) {
        result = DMFAMILY_ARM64;
    } else if ((disasmMode == DM_MIPS64_BE) || (disasmMode == DM_MIPS64_LE) || (disasmMode == DM_MIPS_BE) || (disasmMode == DM_MIPS_LE)) {
        result = DMFAMILY_MIPS;
    } else if ((disasmMode == DM_PPC64_BE) || (disasmMode == DM_PPC64_LE) || (disasmMode == DM_PPC_BE) || (disasmMode == DM_PPC_LE)) {
        result = DMFAMILY_PPC;
    } else if (disasmMode == DM_SPARC) {
        result = DMFAMILY_SPARC;
    } else if (disasmMode == DM_S390X) {
        result = DMFAMILY_SYSZ;
    } else if (disasmMode == DM_XCORE) {
        result = DMFAMILY_XCORE;
    } else if ((disasmMode == DM_M68K) || (disasmMode == DM_M68K00) || (disasmMode == DM_M68K10) || (disasmMode == DM_M68K20) || (disasmMode == DM_M68K30) ||
               (disasmMode == DM_M68K40) || (disasmMode == DM_M68K60)) {
        result = DMFAMILY_M68K;
    } else if ((disasmMode == DM_M6800) || (disasmMode == DM_M6801) || (disasmMode == DM_M6805) || (disasmMode == DM_M6808) || (disasmMode == DM_M6809) ||
               (disasmMode == DM_M6811) || (disasmMode == DM_CPU12) || (disasmMode == DM_HD6301) || (disasmMode == DM_HD6309) || (disasmMode == DM_HCS08)) {
        result = DMFAMILY_M68OK;
    } else if ((disasmMode == DM_RISKV32) || (disasmMode == DM_RISKV64) || (disasmMode == DM_RISKVC)) {
        result = DMFAMILY_RISCV;
    } else if (disasmMode == DM_EVM) {
        result = DMFAMILY_EVM;
    } else if (disasmMode == DM_MOS65XX) {
        result = DMFAMILY_MOS65XX;
    } else if (disasmMode == DM_WASM) {
        result = DMFAMILY_WASM;
    } else if ((disasmMode == DM_BPF_LE) || (disasmMode == DM_BPF_BE)) {
        result = DMFAMILY_BPF;
    } else if (disasmMode == DM_CUSTOM_MACH_REBASE) {
        result = DMFAMILY_CUSTOM_MACH_REBASE;
    } else if ((disasmMode == DM_CUSTOM_MACH_BIND) || (disasmMode == DM_CUSTOM_MACH_WEAK)) {
        result = DMFAMILY_CUSTOM_MACH_BIND;
    } else if (disasmMode == DM_CUSTOM_MACH_EXPORT) {
        result = DMFAMILY_CUSTOM_MACH_EXPORT;
    } else if (disasmMode == DM_CUSTOM_7ZIP_PROPERTIES) {
        result = DMFAMILY_CUSTOM_7ZIP;
    }

    return result;
}

XBinary::DMFAMILY XBinary::getDisasmFamily(XBinary::_MEMORY_MAP *pMemoryMap)
{
    return getDisasmFamily(getDisasmMode(pMemoryMap));
}

QList<XBinary::SYNTAX> XBinary::getDisasmSyntax(DM disasmMode)
{
    QList<SYNTAX> listResult;

    listResult.append(SYNTAX_DEFAULT);

    if (getDisasmFamily(disasmMode) == DMFAMILY_X86) {
        listResult.append(SYNTAX_ATT);
        listResult.append(SYNTAX_AMD);
        listResult.append(SYNTAX_INTEL);
        listResult.append(SYNTAX_MASM);
    }

    if (getDisasmFamily(disasmMode) == DMFAMILY_M68K) {
        listResult.append(SYNTAX_MOTOROLA);
    }

    return listResult;
}

XBinary::MODE XBinary::getModeFromDisasmMode(DM disasmMode)
{
    MODE result = MODE_32;

    if (disasmMode == DM_8086) {
        result = MODE_16;
    } else if (disasmMode == DM_X86_32) {
        result = MODE_32;
    } else if (disasmMode == DM_X86_64) {
        result = MODE_64;
    } else if ((disasmMode == DM_ARM_BE) || (disasmMode == DM_ARM_LE)) {
        result = MODE_32;
    } else if ((disasmMode == DM_AARCH64_BE) || (disasmMode == DM_AARCH64_LE)) {
        result = MODE_64;
    } else if ((disasmMode == DM_MIPS_BE) || (disasmMode == DM_MIPS_LE)) {
        result = MODE_32;
    } else if ((disasmMode == DM_MIPS64_BE) || (disasmMode == DM_MIPS64_LE)) {
        result = MODE_64;
    }

    return result;
}

bool XBinary::checkFileType(XBinary::FT fileTypeMain, XBinary::FT fileTypeOptional)
{
    bool bResult = false;

    if ((fileTypeMain == FT_PE) && ((fileTypeOptional == FT_PE) || (fileTypeOptional == FT_PE32) || (fileTypeOptional == FT_PE64))) {
        bResult = true;
    } else if ((fileTypeMain == FT_ELF) && ((fileTypeOptional == FT_ELF) || (fileTypeOptional == FT_ELF32) || (fileTypeOptional == FT_ELF64))) {
        bResult = true;
    } else if ((fileTypeMain == FT_MACHO) && ((fileTypeOptional == FT_MACHO) || (fileTypeOptional == FT_MACHO32) || (fileTypeOptional == FT_MACHO64))) {
        bResult = true;
    } else if ((fileTypeMain == FT_ICO) && ((fileTypeOptional == FT_ICO) || (fileTypeOptional == FT_CUR))) {
        bResult = true;
    } else if ((fileTypeMain == FT_ICC) && (fileTypeOptional == FT_ICC)) {
        bResult = true;
    } else if ((fileTypeMain == FT_TEXT) && ((fileTypeOptional == FT_TEXT) || (fileTypeOptional == FT_UTF8) || (fileTypeOptional == FT_UNICODE))) {
        bResult = true;
    } else if ((fileTypeMain == FT_TAR_GZ) &&
               ((fileTypeOptional == FT_TAR_GZ) || (fileTypeOptional == FT_TAR_BZIP2) || (fileTypeOptional == FT_TAR_LZIP) || (fileTypeOptional == FT_TAR_LZMA) ||
                (fileTypeOptional == FT_TAR_LZOP) || (fileTypeOptional == FT_TAR_XZ) || (fileTypeOptional == FT_TAR_Z) || (fileTypeOptional == FT_TAR_ZSTD))) {
        bResult = true;
    } else if (fileTypeMain == fileTypeOptional) {
        bResult = true;
    }

    return bResult;
}

void XBinary::filterFileTypes(QSet<XBinary::FT> *pStFileTypes)
{
    // TODO Check!
    // TODO optimize! Add new Types!
    if (!pStFileTypes) {
        return;
    }

    if (pStFileTypes->contains(XBinary::FT_MSDOS) || pStFileTypes->contains(XBinary::FT_DOS16M) || pStFileTypes->contains(XBinary::FT_DOS4G) ||
        pStFileTypes->contains(XBinary::FT_NE) || pStFileTypes->contains(XBinary::FT_LE) || pStFileTypes->contains(XBinary::FT_LX) ||
        pStFileTypes->contains(XBinary::FT_PE) || pStFileTypes->contains(XBinary::FT_PE32) || pStFileTypes->contains(XBinary::FT_PE64) ||
        pStFileTypes->contains(XBinary::FT_ELF) || pStFileTypes->contains(XBinary::FT_ELF32) || pStFileTypes->contains(XBinary::FT_ELF64) ||
        pStFileTypes->contains(XBinary::FT_MACHO) || pStFileTypes->contains(XBinary::FT_MACHO32) || pStFileTypes->contains(XBinary::FT_MACHO64) ||
        pStFileTypes->contains(XBinary::FT_DEX) || pStFileTypes->contains(XBinary::FT_ZIP) || pStFileTypes->contains(XBinary::FT_GZIP) ||
        pStFileTypes->contains(XBinary::FT_ZLIB) || pStFileTypes->contains(XBinary::FT_LHA) || pStFileTypes->contains(XBinary::FT_ARJ) ||
        pStFileTypes->contains(XBinary::FT_LZ4) || pStFileTypes->contains(XBinary::FT_LZMA) || pStFileTypes->contains(XBinary::FT_AMIGAHUNK) ||
        pStFileTypes->contains(XBinary::FT_ATARIST)) {
        XBinary::removeFileTypes(pStFileTypes);
    }
}

void XBinary::removeFileTypes(QSet<XBinary::FT> *pStFileTypes)
{
    if (pStFileTypes) {
        pStFileTypes->remove(XBinary::FT_BINARY);
    }
}

void XBinary::filterFileTypes(QSet<XBinary::FT> *pStFileTypes, XBinary::FT fileType)
{
    if (pStFileTypes && (fileType != XBinary::FT_UNKNOWN)) {
        QSet<XBinary::FT> stFileTypesNew;

        if (fileType == XBinary::FT_PE) {
            if (pStFileTypes->contains(XBinary::FT_PE)) stFileTypesNew.insert(XBinary::FT_PE);
            if (pStFileTypes->contains(XBinary::FT_PE32)) stFileTypesNew.insert(XBinary::FT_PE32);
            if (pStFileTypes->contains(XBinary::FT_PE64)) stFileTypesNew.insert(XBinary::FT_PE64);
        } else if (fileType == XBinary::FT_ELF) {
            if (pStFileTypes->contains(XBinary::FT_ELF)) stFileTypesNew.insert(XBinary::FT_ELF);
            if (pStFileTypes->contains(XBinary::FT_ELF32)) stFileTypesNew.insert(XBinary::FT_ELF32);
            if (pStFileTypes->contains(XBinary::FT_ELF64)) stFileTypesNew.insert(XBinary::FT_ELF64);
        } else if (fileType == XBinary::FT_MACHO) {
            if (pStFileTypes->contains(XBinary::FT_MACHO)) stFileTypesNew.insert(XBinary::FT_MACHO);
            if (pStFileTypes->contains(XBinary::FT_MACHO32)) stFileTypesNew.insert(XBinary::FT_MACHO32);
            if (pStFileTypes->contains(XBinary::FT_MACHO64)) stFileTypesNew.insert(XBinary::FT_MACHO64);
        } else {
            if (pStFileTypes->contains(fileType)) stFileTypesNew.insert(fileType);
        }

        *pStFileTypes = stFileTypesNew;
    }
}

bool XBinary::isFileTypePresent(QSet<XBinary::FT> *pStFileTypes, QSet<XBinary::FT> *pStAvailableFileTypes)
{
    bool bResult = false;

    if (!pStFileTypes || !pStAvailableFileTypes) {
        return bResult;
    }

    QSet<XBinary::FT>::iterator i = pStFileTypes->begin();

    while (i != pStFileTypes->end()) {
        if (pStAvailableFileTypes->contains(*i)) {
            bResult = true;

            break;
        }

        i++;
    }

    return bResult;
}

XBinary::PERCENTAGE XBinary::percentageInit(qint64 nMaxValue, bool bTimer)
{
    PERCENTAGE result = {};
    result.bTimer = bTimer;

    result.nMaxValue = nMaxValue;

    if (!(result.bTimer)) {
        result.nMaxPercentage = 1;

        if (result.nMaxValue > 0x100000000) {
            result.nMaxPercentage = 100;
        } else if (result.nMaxValue > 0x100000) {
            result.nMaxPercentage = 10;
        } else if (result.nMaxValue > 0x1000) {
            result.nMaxPercentage = 5;
        }
    } else {
        result.timer.start();
        result.nMaxPercentage = 100;
    }

    return result;
}

bool XBinary::percentageSetCurrentValue(XBinary::PERCENTAGE *pPercentage, qint64 nCurrentValue)
{
    bool bResult = false;

    if (!pPercentage || (pPercentage->nMaxValue <= 0) || (pPercentage->nMaxPercentage <= 0) ||
        (pPercentage->nCurrentPercentage < 0) || (pPercentage->nCurrentPercentage >= pPercentage->nMaxPercentage)) {
        return bResult;
    }

    pPercentage->nCurrentValue = qBound((qint64)0, nCurrentValue, pPercentage->nMaxValue);

    if (!(pPercentage->bTimer)) {
        const qint64 nNext = (qint64)pPercentage->nCurrentPercentage + 1;
        const qint64 nMaxPercentage = pPercentage->nMaxPercentage;
        const qint64 nQuotient = pPercentage->nMaxValue / nMaxPercentage;
        const qint64 nRemainder = pPercentage->nMaxValue % nMaxPercentage;
        const qint64 nThreshold = nQuotient * nNext + (nRemainder * nNext + nMaxPercentage - 1) / nMaxPercentage;
        if (pPercentage->nCurrentValue >= nThreshold) {
            pPercentage->nCurrentPercentage++;
            bResult = true;
        }
    } else {
        if (pPercentage->timer.elapsed() >= 1000)  // TODO Check speed
        {
            pPercentage->timer.restart();

            const long double fCurrent = ((long double)pPercentage->nCurrentValue * pPercentage->nMaxPercentage) / pPercentage->nMaxValue;
            qint32 _nCurrent = (qint32)qBound((qint64)0, (qint64)fCurrent, (qint64)pPercentage->nMaxPercentage);

            pPercentage->nCurrentPercentage = _nCurrent;
            bResult = true;
        }
    }

    return bResult;
}

qint64 XBinary::getTotalOSSize(QList<OFFSETSIZE> *pListOffsetSize)
{
    qint64 nResult = 0;

    if (!pListOffsetSize) {
        return nResult;
    }

    qint32 nNumberOfRecords = pListOffsetSize->count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        const qint64 nSize = pListOffsetSize->at(i).nSize;
        if ((nSize < 0) || (nSize > (std::numeric_limits<qint64>::max)() - nResult)) {
            return -1;
        }
        nResult += nSize;
    }

    return nResult;
}

XBinary::MODE XBinary::getWidthModeFromSize_32_64(quint64 nSize)
{
    MODE result = MODE_32;

    if (((quint64)nSize) >= 0xFFFFFFFF) {
        result = MODE_64;
    }

    return result;
}

quint32 XBinary::getByteSizeFromWidthMode(MODE mode)
{
    quint32 nResult = 0;

    switch (mode) {
        case MODE_8: nResult = 1; break;
        case MODE_16: nResult = 2; break;
        case MODE_32: nResult = 4; break;
        case MODE_64: nResult = 8; break;
        default: nResult = 0;
    }

    return nResult;
}

XBinary::MODE XBinary::getWidthModeFromSize(quint64 nSize)
{
    MODE result = MODE_32;

    if (((quint64)nSize) >= 0xFFFFFFFF) {
        result = MODE_64;
    } else if (((quint64)nSize) >= 0xFFFF) {
        result = MODE_32;
    } else if (((quint64)nSize) >= 0xFF) {
        result = MODE_16;
    } else {
        result = MODE_8;
    }

    return result;
}

XBinary::MODE XBinary::getWidthModeFromMemoryMap(XBinary::_MEMORY_MAP *pMemoryMap)
{
    MODE result = MODE_32;

    if (!pMemoryMap) {
        return result;
    }

    quint64 nImageEnd = 0;
    if ((pMemoryMap->nModuleAddress != (XADDR)-1) && (pMemoryMap->nImageSize > 0)) {
        const quint64 nImageSize = (quint64)pMemoryMap->nImageSize;
        nImageEnd = (nImageSize > (std::numeric_limits<XADDR>::max)() - pMemoryMap->nModuleAddress)
                        ? (std::numeric_limits<XADDR>::max)()
                        : pMemoryMap->nModuleAddress + nImageSize;
    }

    const quint64 nBinarySize = (pMemoryMap->nBinarySize > 0) ? (quint64)pMemoryMap->nBinarySize : 0;
    const quint64 nMax = qMax(nImageEnd, nBinarySize);

    result = getWidthModeFromSize(nMax);

    return result;
}

XBinary::MODE XBinary::getWidthModeFromByteSize(quint32 nByteSize)
{
    MODE result = MODE_32;

    if (nByteSize == 2) {
        result = MODE_8;
    } else if (nByteSize == 4) {
        result = MODE_16;
    } else if (nByteSize == 8) {
        result = MODE_32;
    } else if (nByteSize == 16) {
        result = MODE_64;
    }

    return result;
}

bool XBinary::isAnsiSymbol(quint8 cCode, bool bExtra)
{
    bool bResult = false;

    if (!bExtra) {
        if ((cCode >= 0x20) && (cCode < 0x80)) {
            bResult = true;
        }
    } else {
        if (cCode >= 0x20) {
            bResult = true;
        }
    }

    return bResult;
}

bool XBinary::isUTF8Symbol(quint8 cCode, qint32 *pnWidth)
{
    if (!pnWidth) {
        return false;
    }

    *pnWidth = 0;
    if ((cCode >= 0x20) && (cCode <= 0x7F)) {
        *pnWidth = 1;
        return true;
    }
    if ((cCode >= 0xC2) && (cCode <= 0xDF)) {
        *pnWidth = 2;
        return true;
    }
    if ((cCode >= 0xE0) && (cCode <= 0xEF)) {
        *pnWidth = 3;
        return true;
    }
    if ((cCode >= 0xF0) && (cCode <= 0xF4)) {
        *pnWidth = 4;
        return true;
    }
    if ((cCode >= 0x80) && (cCode <= 0xBF)) {
        return true;
    }

    return false;
}

bool XBinary::isUnicodeSymbol(quint16 nCode, bool bExtra)
{
    bool bResult = false;

    if (!bExtra) {
        if ((nCode >= 0x20) && (nCode < 0x80)) {
            bResult = true;
        }
    } else {
        if ((nCode >= 0x20) && (nCode <= 0xFF)) {
            bResult = true;
        } else if ((nCode >= 0x0400) && (nCode <= 0x04FF))  // Cyrillic
        {
            bResult = true;
        }
    }

    return bResult;
}

QString XBinary::getStringFromIndex(qint64 nOffset, qint64 nSize, qint32 nIndex)
{
    QString sResult;

    const qint64 nFileSize = getSize();
    if ((nOffset >= 0) && (nSize > 0) && (nIndex >= 0) && ((qint64)nIndex < nSize) &&
        (nOffset <= nFileSize) && (nSize <= (nFileSize - nOffset))) {
        sResult = read_ansiString(nOffset + nIndex, nSize - nIndex);
    }

    return sResult;
}

QList<QString> XBinary::getAllFilesFromDirectory(const QString &sDirectory, const QString &sExtension)
{
    QDir directory(sDirectory);
    QStringList fileList = directory.entryList(QStringList() << sExtension, QDir::Files);
    QList<QString> listFileNames;

    qint32 nNumberOfFiles = fileList.size();

    for (int i = 0; i < nNumberOfFiles; ++i) {
        listFileNames.append(directory.absoluteFilePath(fileList.at(i)));
    }

    return listFileNames;
}

QList<XBinary::OPCODE> XBinary::getOpcodes(qint64 nOffset, XADDR nStartAddress, qint64 nSize, quint32 nType, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    QList<OPCODE> listResult;

    OFFSETSIZE osRegion = convertOffsetAndSize(nOffset, nSize);

    nOffset = osRegion.nOffset;
    nSize = osRegion.nSize;

    if ((nOffset != -1) && (nSize > 0) && isPdStructNotCanceled(pPdStruct) &&
        (nStartAddress <= (std::numeric_limits<XADDR>::max)() - (XADDR)nSize)) {
        const qint32 nRequestedBufferSize = getBufferSize(pPdStruct);

        if (nRequestedBufferSize <= 0) {
            return listResult;
        }

        const qint32 nBufferSize = qBound((qint32)0x1000, nRequestedBufferSize, (qint32)0x100000);
        char *pBuffer = new (std::nothrow) char[nBufferSize];

        if (!pBuffer) {
            return listResult;
        }

        const qint64 nTotalSize = nSize;
        qint64 nProcessed = 0;
        bool bReadError = false;
        const qint32 nFreeIndex = XBinary::reservePdStructRecord(pPdStruct, nTotalSize);

        while ((nSize > 0) && isPdStructNotCanceled(pPdStruct)) {
            qint64 nTempSize = qMin((qint64)nBufferSize, nSize);

            if (read_array_process(nOffset, pBuffer, nTempSize, pPdStruct) != nTempSize) {
                setPdStructErrorString(pPdStruct, tr("Read error"));
                bReadError = true;
                break;
            }

            qint64 nOpcodesSize = 0;
            OPCODE_STATUS opcodeStatus = OPCODE_STATUS_SUCCESS;

            for (qint32 i = 0; (i < nTempSize) && isPdStructNotCanceled(pPdStruct);) {
                const XADDR nDecodedSize = readOpcodes(nType, pBuffer + i, nStartAddress + (XADDR)i, nTempSize - i, &listResult, &opcodeStatus);

                if ((nDecodedSize == 0) || (nDecodedSize > (XADDR)(nTempSize - i)) || (opcodeStatus == OPCODE_STATUS_END)) {
                    if ((nDecodedSize > (XADDR)(nTempSize - i)) && (opcodeStatus != OPCODE_STATUS_END)) {
                        bReadError = true;
                    }
                    break;
                }

                i += (qint32)nDecodedSize;
                nOpcodesSize += (qint64)nDecodedSize;
            }

            if (bReadError || (nOpcodesSize == 0) || (opcodeStatus == OPCODE_STATUS_END)) {
                break;
            }

            nSize -= nOpcodesSize;
            nOffset += nOpcodesSize;
            nStartAddress += nOpcodesSize;
            nProcessed += nOpcodesSize;
            XBinary::setPdStructCurrent(pPdStruct, nFreeIndex, nProcessed);
        }

        delete[] pBuffer;
        XBinary::setPdStructFinished(pPdStruct, nFreeIndex);

        if (bReadError || !isPdStructNotCanceled(pPdStruct)) {
            listResult.clear();
        }
    }

    return listResult;
}

XADDR XBinary::readOpcodes(quint32 nType, char *pData, XADDR nStartAddress, qint64 nSize, QList<OPCODE> *pListOpcodes, OPCODE_STATUS *pOpcodeStatus)
{
    Q_UNUSED(nType)
    Q_UNUSED(pData)
    Q_UNUSED(nStartAddress)
    Q_UNUSED(nSize)
    Q_UNUSED(pListOpcodes)

    if (pOpcodeStatus) {
        *pOpcodeStatus = OPCODE_STATUS_END;
    }

    return false;
}

bool XBinary::_read_opcode_uleb128(OPCODE *pOpcode, char **ppData, qint64 *pnSize, XADDR *pnAddress, XADDR *pnResult, const QString &sPrefix)
{
    bool bResult = false;

    if (pOpcode && ppData && *ppData && pnSize && pnAddress && pnResult && (*pnSize > 0)) {
        PACKED_UINT uleb128 = _read_uleb128(*ppData, *pnSize);

        if (uleb128.bIsValid && (uleb128.nByteSize > 0) && (uleb128.nByteSize <= *pnSize) &&
            ((XADDR)uleb128.nByteSize <= (std::numeric_limits<XADDR>::max)() - *pnAddress) &&
            ((XADDR)uleb128.nByteSize <= (std::numeric_limits<XADDR>::max)() - *pnResult)) {
            pOpcode->nAddress = *pnAddress;
            pOpcode->nSize = uleb128.nByteSize;
            pOpcode->sName = QString("%1(%2)").arg(sPrefix).arg(QString::number(uleb128.nValue));

            *pnSize -= pOpcode->nSize;
            *ppData += pOpcode->nSize;
            *pnResult += pOpcode->nSize;
            *pnAddress += pOpcode->nSize;

            bResult = true;
        }
    }

    return bResult;
}

bool XBinary::_read_opcode_ansiString(XBinary::OPCODE *pOpcode, char **ppData, qint64 *pnSize, XADDR *pnAddress, XADDR *pnResult, const QString &sPrefix)
{
    bool bResult = false;

    if (pOpcode && ppData && *ppData && pnSize && pnAddress && pnResult && (*pnSize > 0)) {
        const char *pStringEnd = (const char *)memchr(*ppData, '\0', (size_t)(*pnSize));

        if (pStringEnd) {
            qint64 nStringSize = pStringEnd - *ppData;
            const qint64 nConsumedSize = nStringSize + 1;
            if ((nStringSize <= 0x7fffffff) && ((XADDR)nConsumedSize <= (std::numeric_limits<XADDR>::max)() - *pnAddress) &&
                ((XADDR)nConsumedSize <= (std::numeric_limits<XADDR>::max)() - *pnResult)) {
                QString sString = QString::fromLatin1(*ppData, (int)nStringSize);
                pOpcode->nAddress = *pnAddress;
                pOpcode->nSize = nStringSize + 1;
                pOpcode->sName = QString("%1(\"%2\")").arg(sPrefix).arg(sString);

                *pnSize -= pOpcode->nSize;
                *ppData += pOpcode->nSize;
                *pnResult += pOpcode->nSize;
                *pnAddress += pOpcode->nSize;

                bResult = true;
            }
        }
    }

    return bResult;
}

QList<quint32> XBinary::get_uint32_list(qint64 nOffset, qint32 nNumberOfRecords, bool bIsBigEndian)
{
    QList<quint32> listResult;

    const qint64 nFileSize = getSize();
    if ((nOffset < 0) || (nNumberOfRecords <= 0) || (nNumberOfRecords > 0x100000) || (nOffset > nFileSize) ||
        ((qint64)nNumberOfRecords > (nFileSize - nOffset) / (qint64)sizeof(quint32))) {
        return listResult;
    }

    listResult.reserve(nNumberOfRecords);

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        quint32 nRecord = read_uint32(nOffset + i * sizeof(quint32), bIsBigEndian);

        listResult.append(nRecord);
    }

    return listResult;
}

QList<quint64> XBinary::get_uint64_list(qint64 nOffset, qint32 nNumberOfRecords, bool bIsBigEndian)
{
    QList<quint64> listResult;

    const qint64 nFileSize = getSize();
    if ((nOffset < 0) || (nNumberOfRecords <= 0) || (nNumberOfRecords > 0x100000) || (nOffset > nFileSize) ||
        ((qint64)nNumberOfRecords > (nFileSize - nOffset) / (qint64)sizeof(quint64))) {
        return listResult;
    }

    listResult.reserve(nNumberOfRecords);

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        quint64 nRecord = read_uint64(nOffset + i * sizeof(quint64), bIsBigEndian);

        listResult.append(nRecord);
    }

    return listResult;
}

bool XBinary::_isOffsetsCrossed(qint64 nOffset1, qint64 nSize1, qint64 nOffset2, qint64 nSize2)
{
    if ((nOffset1 < 0) || (nOffset2 < 0) || (nSize1 <= 0) || (nSize2 <= 0) ||
        (nSize1 > (std::numeric_limits<qint64>::max)() - nOffset1) ||
        (nSize2 > (std::numeric_limits<qint64>::max)() - nOffset2)) {
        return false;
    }

    return (nOffset1 < (nOffset2 + nSize2)) && (nOffset2 < (nOffset1 + nSize1));
}

bool XBinary::_isAddressCrossed(XADDR nAddress1, qint64 nSize1, XADDR nAddress2, qint64 nSize2)
{
    if ((nAddress1 == (XADDR)-1) || (nAddress2 == (XADDR)-1) || (nSize1 <= 0) || (nSize2 <= 0) ||
        ((quint64)nSize1 > (std::numeric_limits<XADDR>::max)() - nAddress1) ||
        ((quint64)nSize2 > (std::numeric_limits<XADDR>::max)() - nAddress2)) {
        return false;
    }

    return (nAddress1 < (nAddress2 + (quint64)nSize2)) && (nAddress2 < (nAddress1 + (quint64)nSize1));
}

bool XBinary::_isReplaced(qint64 nOffset, qint64 nSize, QList<XBinary::MEMORY_REPLACE> *pListMemoryReplace)
{
    bool bResult = false;

    if (pListMemoryReplace && (nSize > 0)) {
        qint32 nNumberOfRecords = pListMemoryReplace->count();

        for (qint32 i = 0; i < nNumberOfRecords; i++) {
            if (_isOffsetsCrossed(nOffset, nSize, pListMemoryReplace->at(i).nOffset, pListMemoryReplace->at(i).nSize)) {
                bResult = true;
                break;
            }
        }
    }

    return bResult;
}

bool XBinary::_replaceMemory(qint64 nDataOffset, char *pData, qint64 nDataSize, QList<XBinary::MEMORY_REPLACE> *pListMemoryReplace)
{
    bool bResult = false;

    if (!pListMemoryReplace || !pData || (nDataOffset < 0) || (nDataSize <= 0) ||
        (nDataSize > (std::numeric_limits<qint64>::max)() - nDataOffset)) {
        return bResult;
    }

    const qint64 nDataEnd = nDataOffset + nDataSize;
    qint32 nNumberOfRecords = pListMemoryReplace->count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        const MEMORY_REPLACE &memoryReplace = pListMemoryReplace->at(i);
        if ((memoryReplace.nOffset < 0) || (memoryReplace.nSize <= 0) ||
            (memoryReplace.nSize > (std::numeric_limits<qint64>::max)() - memoryReplace.nOffset) ||
            (memoryReplace.nSize > memoryReplace.baOriginal.size())) {
            continue;
        }

        const qint64 nReplaceEnd = memoryReplace.nOffset + memoryReplace.nSize;
        const qint64 nOverlapStart = qMax(nDataOffset, memoryReplace.nOffset);
        const qint64 nOverlapEnd = qMin(nDataEnd, nReplaceEnd);
        if (nOverlapStart < nOverlapEnd) {
            const qint64 nCopySize = nOverlapEnd - nOverlapStart;
            memcpy(pData + (nOverlapStart - nDataOffset),
                   memoryReplace.baOriginal.constData() + (nOverlapStart - memoryReplace.nOffset),
                   (size_t)nCopySize);
            bResult = true;
        }
    }

    return bResult;
}

bool XBinary::_updateReplaces(qint64 nDataOffset, char *pData, qint64 nDataSize, QList<MEMORY_REPLACE> *pListMemoryReplace)
{
    bool bResult = false;

    if (!pListMemoryReplace || !pData || (nDataOffset < 0) || (nDataSize <= 0) ||
        (nDataSize > (std::numeric_limits<qint64>::max)() - nDataOffset)) {
        return bResult;
    }

    const qint64 nDataEnd = nDataOffset + nDataSize;
    qint32 nNumberOfRecords = pListMemoryReplace->count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        MEMORY_REPLACE &memoryReplace = (*pListMemoryReplace)[i];
        if ((memoryReplace.nOffset < 0) || (memoryReplace.nSize <= 0) ||
            (memoryReplace.nSize > (std::numeric_limits<qint64>::max)() - memoryReplace.nOffset) ||
            (memoryReplace.nSize > memoryReplace.baOriginal.size())) {
            continue;
        }

        const qint64 nReplaceEnd = memoryReplace.nOffset + memoryReplace.nSize;
        const qint64 nOverlapStart = qMax(nDataOffset, memoryReplace.nOffset);
        const qint64 nOverlapEnd = qMin(nDataEnd, nReplaceEnd);
        if (nOverlapStart < nOverlapEnd) {
            const qint64 nCopySize = nOverlapEnd - nOverlapStart;
            memcpy(memoryReplace.baOriginal.data() + (nOverlapStart - memoryReplace.nOffset),
                   pData + (nOverlapStart - nDataOffset), (size_t)nCopySize);
            bResult = true;
        }
    }

    return bResult;
}

QList<XBinary::SYMBOL_RECORD> XBinary::getSymbolRecords(XBinary::_MEMORY_MAP *pMemoryMap, SYMBOL_TYPE symbolType)
{
    Q_UNUSED(pMemoryMap)
    Q_UNUSED(symbolType)

    QList<XBinary::SYMBOL_RECORD> listResult;

    // TODO

    return listResult;
}

XBinary::SYMBOL_RECORD XBinary::findSymbolByAddress(QList<SYMBOL_RECORD> *pListSymbolRecords, XADDR nAddress)
{
    SYMBOL_RECORD result = {};

    if (!pListSymbolRecords) {
        return result;
    }

    qint32 nNumberOfRecords = pListSymbolRecords->count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (pListSymbolRecords->at(i).nAddress == nAddress) {
            result = pListSymbolRecords->at(i);

            break;
        }
    }

    return result;
}

XBinary::SYMBOL_RECORD XBinary::findSymbolByName(QList<SYMBOL_RECORD> *pListSymbolRecords, const QString &sName)
{
    SYMBOL_RECORD result = {};

    if (!pListSymbolRecords) {
        return result;
    }

    qint32 nNumberOfRecords = pListSymbolRecords->count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (pListSymbolRecords->at(i).sName == sName) {
            result = pListSymbolRecords->at(i);

            break;
        }
    }

    return result;
}

XBinary::SYMBOL_RECORD XBinary::findSymbolByOrdinal(QList<SYMBOL_RECORD> *pListSymbolRecords, qint32 nOrdinal)
{
    SYMBOL_RECORD result = {};

    if (!pListSymbolRecords) {
        return result;
    }

    qint32 nNumberOfRecords = pListSymbolRecords->count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (pListSymbolRecords->at(i).nOrdinal == nOrdinal) {
            result = pListSymbolRecords->at(i);

            break;
        }
    }

    return result;
}

QString XBinary::generateUUID()
{
    return QUuid::createUuid().toString().remove("{").remove("}");
}

QString XBinary::appendText(const QString &sResult, const QString &sString, const QString &sSeparate)
{
    QString _sResult = sResult;

    if (sString != "") {
        if (_sResult != "") _sResult += sSeparate;
        _sResult += sString;
    }

    return _sResult;
}

QString XBinary::appendComma(const QString &sResult, const QString &sString)
{
    return appendText(sResult, sString, ", ");
}

QString XBinary::bytesCountToString(quint64 nValue, quint64 nBase)
{
    QString sResult;

    if (nBase != 1024) {
        nBase = 1000;
    }

    QString sValue;
    QString sUnit;

    if (nValue < nBase) {
        sValue = QString::number(nValue);
        sUnit = tr("Bytes");
    } else if (nValue < (nBase * nBase)) {
        sValue = QString::number((double)nValue / nBase, 'f', 2);

        if (nBase == 1024) sUnit = tr("KiB");
        else sUnit = tr("kB");
    } else if (nValue < (nBase * nBase * nBase)) {
        sValue = QString::number((double)nValue / (nBase * nBase), 'f', 2);

        if (nBase == 1024) sUnit = tr("MiB");
        else sUnit = tr("MB");
    } else if (nValue < (nBase * nBase * nBase * nBase)) {
        sValue = QString::number((double)nValue / (nBase * nBase * nBase), 'f', 2);

        if (nBase == 1024) sUnit = tr("GiB");
        else sUnit = tr("GB");
    } else {
        sValue = QString::number((double)nValue / (nBase * nBase * nBase * nBase), 'f', 2);

        if (nBase == 1024) sUnit = tr("TiB");
        else sUnit = tr("TB");
    }

    sResult = QString("%1 %2").arg(sValue).arg(sUnit);

    return sResult;
}

QString XBinary::numberToString(quint64 nValue)
{
    QString sResult;

    sResult = QString::number(nValue);

    return sResult;
}

QString XBinary::fullVersionByteToString(quint8 nValue)
{
    return QString("\"%1\"").arg(get_uint8_full_version(nValue));
}

QString XBinary::fullVersionWordToString(quint16 nValue)
{
    return QString("\"%1\"").arg(get_uint16_full_version(nValue));
}

QString XBinary::fullVersionDwordToString(quint32 nValue)
{
    return QString("\"%1\"").arg(get_uint32_full_version(nValue));
}

QString XBinary::fullVersionQwordToString(quint64 nValue)
{
    return QString("\"%1\"").arg(get_uint64_full_version(nValue));
}

QString XBinary::versionWordToString(quint16 nValue)
{
    return QString("\"%1\"").arg(get_uint16_version(nValue));
}

QString XBinary::versionDwordToString(quint32 nValue)
{
    return QString("\"%1\"").arg(get_uint32_version(nValue));
}

QString XBinary::versionDivModByteToString(quint8 nValue)
{
    return QString("\"%1.%2\"").arg(nValue / 10).arg(nValue % 10);
}

QString XBinary::versionDivModWordToString(quint16 nValue)
{
    quint8 nHigh = (nValue >> 8) & 0xFF;
    quint8 nLow = nValue & 0xFF;
    return QString("\"%1.%2.%3.%4\"").arg(nHigh / 10).arg(nHigh % 10).arg(nLow / 10).arg(nLow % 10);
}

QString XBinary::formatXML(const QString &sXML)
{
    QString sResult;

    QXmlStreamReader reader(sXML);
    QXmlStreamWriter writer(&sResult);
    writer.setAutoFormatting(true);

    while (!reader.atEnd()) {
        reader.readNext();
        if (!reader.isWhitespace()) {
            writer.writeCurrentToken(reader);
        }
    }

    if (reader.hasError() || writer.hasError()) {
        sResult = sXML;
    }

    return sResult;
}

quint32 XBinary::make_dword(XBinary::XDWORD xdword)
{
    quint32 nResult = 0;

    nResult += (xdword.nValue1) << 16;
    nResult += xdword.nValue2;

    return nResult;
}

quint32 XBinary::make_dword(quint16 nValue1, quint16 nValue2)
{
    XDWORD xdword = {};
    xdword.nValue1 = nValue1;
    xdword.nValue2 = nValue2;

    return make_dword(xdword);
}

XBinary::XDWORD XBinary::make_xdword(quint32 nValue)
{
    XDWORD result = {};

    result.nValue1 = (quint16)(nValue >> 16);
    result.nValue2 = (quint16)(nValue & 0xFFFF);

    return result;
}

QString XBinary::recordFilePartIdToString(FILEPART id)
{
    return XCONVERT_idToTransString(id, _TABLE_XBinary_FILEPART, sizeof(_TABLE_XBinary_FILEPART) / sizeof(XBinary::XCONVERT));
}

QString XBinary::recordFilePartIdToFtString(FILEPART id)
{
    return XCONVERT_idToFtString(id, _TABLE_XBinary_FILEPART, sizeof(_TABLE_XBinary_FILEPART) / sizeof(XBinary::XCONVERT));
}

bool XBinary::checkVersionString(const QString &sVersion)
{
    bool bResult = false;

    if (sVersion.trimmed() != "") {
        bResult = true;

        qint32 nStringSize = sVersion.size();

        // TODO
        for (qint32 i = 0; i < nStringSize; i++) {
            QChar _char = sVersion.at(i);

            if ((_char >= QChar('0')) && (_char <= QChar('9'))) {
            } else if (_char == QChar('.')) {
            } else {
                bResult = false;
                break;
            }
        }
    }

    return bResult;
}

QString XBinary::cleanString(const QString &sString)
{
    QString sResult;

    qint32 nNumberOfChars = sString.size();

    for (qint32 i = 0; i < nNumberOfChars; i++) {
        QChar _char = sString.at(i);

        bool bAdd = false;

        if (_char.isLetterOrNumber() || _char.isPunct()) {
            bAdd = true;
        }

        if (bAdd) {
            sResult.append(_char);
        }
    }

    return sResult;
}

XBinary::XVARIANT XBinary::getXVariant(bool bValue)
{
    XVARIANT result = {};

    result.varType = VT_BIT;
    result.var = bValue;

    return result;
}

XBinary::XVARIANT XBinary::getXVariant(quint8 nValue)
{
    XVARIANT result = {};

    result.varType = VT_BYTE;
    result.var = nValue;

    return result;
}

XBinary::XVARIANT XBinary::getXVariant(quint16 nValue, bool bIsBigEndian)
{
    XVARIANT result = {};

    result.bIsBigEndian = bIsBigEndian;
    result.varType = VT_WORD;
    result.var = nValue;

    return result;
}

XBinary::XVARIANT XBinary::getXVariant(quint32 nValue, bool bIsBigEndian)
{
    XVARIANT result = {};

    result.bIsBigEndian = bIsBigEndian;
    result.varType = VT_DWORD;
    result.var = nValue;

    return result;
}

XBinary::XVARIANT XBinary::getXVariant(quint64 nValue, bool bIsBigEndian)
{
    XVARIANT result = {};

    result.bIsBigEndian = bIsBigEndian;
    result.varType = VT_QWORD;
    result.var = nValue;

    return result;
}

XBinary::XVARIANT XBinary::getXVariant(quint64 nValue[2], bool bIsBigEndian)
{
    XVARIANT result = {};

    result.bIsBigEndian = bIsBigEndian;
    result.varType = VT_128;
    if (nValue) {
        result.var = QByteArray(reinterpret_cast<const char *>(nValue), 2 * (qint32)sizeof(quint64));
    }

    return result;
}

XBinary::XVARIANT XBinary::getXVariant(quint64 nLow, quint64 nHigh, bool bIsBigEndian)
{
    XVARIANT result = {};

    result.bIsBigEndian = bIsBigEndian;
    result.varType = VT_128;
    const quint64 values[2] = {nLow, nHigh};
    result.var = QByteArray(reinterpret_cast<const char *>(values), (qint32)sizeof(values));

    return result;
}

XBinary::XVARIANT XBinary::getXVariant(quint64 nLow1, quint64 nLow2, quint64 nHigh1, quint64 nHigh2, bool bIsBigEndian)
{
    XVARIANT result = {};

    result.bIsBigEndian = bIsBigEndian;
    result.varType = VT_256;
    const quint64 values[4] = {nLow1, nLow2, nHigh1, nHigh2};
    result.var = QByteArray(reinterpret_cast<const char *>(values), (qint32)sizeof(values));

    return result;
}

XBinary::XVARIANT XBinary::getXVariant(quint8 nValue[10], bool bIsBigEndian)
{
    XVARIANT result = {};

    result.bIsBigEndian = bIsBigEndian;
    result.varType = VT_FPEG;
    if (nValue) {
        result.var = QByteArray(reinterpret_cast<const char *>(nValue), 10);
    }

    return result;
}

quint64 XBinary::xVariantToQword(const XVARIANT &xvariant)
{
    quint64 nResult = 0;

    if ((xvariant.varType == VT_BYTE) || (xvariant.varType == VT_WORD) || (xvariant.varType == VT_DWORD) || (xvariant.varType == VT_QWORD)) {
        nResult = xvariant.var.toULongLong();
    }

    return nResult;
}

quint32 XBinary::getDwordFromQword(quint64 nValue, qint32 nIndex)
{
    quint32 nResult = 0;

    if ((nIndex >= 0) && (nIndex < 2)) {
        nValue = nValue >> (nIndex * 32);

        nResult = nValue & 0xFFFFFFFF;
    }

    return nResult;
}

quint16 XBinary::getWordFromQword(quint64 nValue, qint32 nIndex)
{
    quint16 nResult = 0;

    if ((nIndex >= 0) && (nIndex < 4)) {
        nValue = nValue >> (nIndex * 16);

        nResult = nValue & 0xFFFF;
    }

    return nResult;
}

quint8 XBinary::getByteFromQword(quint64 nValue, qint32 nIndex)
{
    quint8 nResult = 0;

    if ((nIndex >= 0) && (nIndex < 8)) {
        nValue = nValue >> (nIndex * 8);

        nResult = nValue & 0xFF;
    }

    return nResult;
}

quint16 XBinary::getWordFromDword(quint32 nValue, qint32 nIndex)
{
    quint16 nResult = 0;

    if ((nIndex >= 0) && (nIndex < 2)) {
        nValue = nValue >> (nIndex * 16);

        nResult = nValue & 0xFFFF;
    }

    return nResult;
}

quint8 XBinary::getByteFromDword(quint32 nValue, qint32 nIndex)
{
    quint8 nResult = 0;

    if ((nIndex >= 0) && (nIndex < 4)) {
        nValue = nValue >> (nIndex * 8);

        nResult = nValue & 0xFF;
    }

    return nResult;
}

quint8 XBinary::getByteFromWord(quint16 nValue, qint32 nIndex)
{
    quint8 nResult = 0;

    if ((nIndex >= 0) && (nIndex < 2)) {
        nValue = nValue >> (nIndex * 8);

        nResult = nValue & 0xFF;
    }

    return nResult;
}

bool XBinary::getBitFromByte(quint8 nValue, qint32 nIndex)
{
    bool bResult = false;

    if ((nIndex >= 0) && (nIndex < 8)) {
        bResult = (nValue >> nIndex) & 0x1;
    }

    return bResult;
}

bool XBinary::getBitFromWord(quint16 nValue, qint32 nIndex)
{
    bool bResult = false;

    if ((nIndex >= 0) && (nIndex < 16)) {
        bResult = (nValue >> nIndex) & 0x1;
    }

    return bResult;
}

bool XBinary::getBitFromDword(quint32 nValue, qint32 nIndex)
{
    bool bResult = false;

    if ((nIndex >= 0) && (nIndex < 32)) {
        bResult = (nValue >> nIndex) & 0x1;
    }

    return bResult;
}

bool XBinary::getBitFromQword(quint64 nValue, qint32 nIndex)
{
    bool bResult = false;

    if ((nIndex >= 0) && (nIndex < 64)) {
        bResult = (nValue >> nIndex) & 0x1;
    }

    return bResult;
}

quint64 XBinary::setDwordToQword(quint64 nInit, quint32 nValue, qint32 nIndex)
{
    quint64 nResult = nInit;

    if ((nIndex >= 0) && (nIndex < 2)) {
        quint64 nFF = 0xFFFFFFFF;
        quint64 _nValue = nValue;

        nFF = nFF << (nIndex * 32);
        _nValue = _nValue << (nIndex * 32);

        nResult = nResult & (~nFF);
        nResult = nResult | _nValue;
    }

    return nResult;
}

quint64 XBinary::setWordToQword(quint64 nInit, quint16 nValue, qint32 nIndex)
{
    quint64 nResult = nInit;

    if ((nIndex >= 0) && (nIndex < 4)) {
        quint64 nFF = 0xFFFF;
        quint64 _nValue = nValue;

        nFF = nFF << (nIndex * 16);
        _nValue = _nValue << (nIndex * 16);

        nResult = nResult & (~nFF);
        nResult = nResult | _nValue;
    }

    return nResult;
}

quint64 XBinary::setByteToQword(quint64 nInit, quint8 nValue, qint32 nIndex)
{
    quint64 nResult = nInit;

    if ((nIndex >= 0) && (nIndex < 8)) {
        quint64 nFF = 0xFF;
        quint64 _nValue = nValue;

        nFF = nFF << (nIndex * 8);
        _nValue = _nValue << (nIndex * 8);

        nResult = nResult & (~nFF);
        nResult = nResult | _nValue;
    }

    return nResult;
}

quint32 XBinary::setWordToDword(quint32 nInit, quint16 nValue, qint32 nIndex)
{
    quint32 nResult = nInit;

    if ((nIndex >= 0) && (nIndex < 2)) {
        quint32 nFF = 0xFFFF;
        quint32 _nValue = nValue;

        nFF = nFF << (nIndex * 16);
        _nValue = _nValue << (nIndex * 16);

        nResult = nResult & (~nFF);
        nResult = nResult | _nValue;
    }

    return nResult;
}

quint32 XBinary::setByteToDword(quint32 nInit, quint8 nValue, qint32 nIndex)
{
    quint32 nResult = nInit;

    if ((nIndex >= 0) && (nIndex < 4)) {
        quint32 nFF = 0xFF;
        quint32 _nValue = nValue;

        nFF = nFF << (nIndex * 8);
        _nValue = _nValue << (nIndex * 8);

        nResult = nResult & (~nFF);
        nResult = nResult | _nValue;
    }

    return nResult;
}

quint16 XBinary::setByteToWord(quint16 nInit, quint8 nValue, qint32 nIndex)
{
    quint16 nResult = nInit;

    if ((nIndex >= 0) && (nIndex < 2)) {
        quint16 nFF = 0xFF;
        quint16 _nValue = nValue;

        nFF = nFF << (nIndex * 8);
        _nValue = _nValue << (nIndex * 8);

        nResult = nResult & (~nFF);
        nResult = nResult | _nValue;
    }

    return nResult;
}

quint8 XBinary::setBitToByte(quint8 nInit, bool bValue, qint32 nIndex)
{
    quint8 nResult = nInit;

    if ((nIndex >= 0) && (nIndex < 8)) {
        quint8 nFF = 0x01;
        quint8 _nValue = (quint8)bValue;

        nFF = nFF << nIndex;
        _nValue = _nValue << nIndex;

        nResult = nResult & (~nFF);
        nResult = nResult | _nValue;
    }

    return nResult;
}

quint16 XBinary::setBitToWord(quint16 nInit, bool bValue, qint32 nIndex)
{
    quint16 nResult = nInit;

    if ((nIndex >= 0) && (nIndex < 16)) {
        quint16 nFF = 0x01;
        quint16 _nValue = (quint16)bValue;

        nFF = nFF << nIndex;
        _nValue = _nValue << nIndex;

        nResult = nResult & (~nFF);
        nResult = nResult | _nValue;
    }

    return nResult;
}

quint32 XBinary::setBitToDword(quint32 nInit, bool bValue, qint32 nIndex)
{
    quint32 nResult = nInit;

    if ((nIndex >= 0) && (nIndex < 32)) {
        quint32 nFF = 0x01;
        quint32 _nValue = (quint32)bValue;

        nFF = nFF << nIndex;
        _nValue = _nValue << nIndex;

        nResult = nResult & (~nFF);
        nResult = nResult | _nValue;
    }

    return nResult;
}

quint64 XBinary::setBitToQword(quint64 nInit, bool bValue, qint32 nIndex)
{
    quint64 nResult = nInit;

    if ((nIndex >= 0) && (nIndex < 64)) {
        quint64 nFF = 0x01;
        quint64 _nValue = (quint64)bValue;

        nFF = nFF << nIndex;
        _nValue = _nValue << nIndex;

        nResult = nResult & (~nFF);
        nResult = nResult | _nValue;
    }

    return nResult;
}

bool XBinary::isXVariantEqual(XVARIANT value1, XVARIANT value2)
{
    return (value1.varType == value2.varType) && (value1.bIsBigEndian == value2.bIsBigEndian) && (value1.var == value2.var);
}

void XBinary::clearXVariant(XVARIANT *pVar)
{
    if (pVar) {
        pVar->var.clear();
    }
}

XBinary::MODE XBinary::getModeOS()
{
    XBinary::MODE modeResult = XBinary::MODE_UNKNOWN;

    if (sizeof(void *) == 8) {
        modeResult = XBinary::MODE_64;
    } else {
        modeResult = XBinary::MODE_32;
    }

    return modeResult;
}

XBinary::PDSTRUCT::PDSTRUCT()
    : bIsStop(false),
      nFinished(0),
      bForceStop(false),
      nBufferSize(0),
      nFileBufferSize(0),
      pCallback(nullptr),
      pCallbackUserData(nullptr),
      nLastCallbackTime(0),
      _pdCallbackState(new PDSTRUCT_CALLBACK_STATE)
{
    for (qint32 i = 0; i < N_NUMBER_PDRECORDS; i++) {
        _pdRecord[i].nCurrent.storeRelease(0);
        _pdRecord[i].nTotal.storeRelease(0);
        _pdRecord[i].sStatus.clear();
        _pdRecord[i].bIsValid.storeRelease(false);
    }
}

XBinary::PDSTRUCT::PDSTRUCT(const PDSTRUCT &other) : PDSTRUCT()
{
    QMutexLocker locker(&other._pdMutex);

    for (qint32 i = 0; i < N_NUMBER_PDRECORDS; i++) {
        _pdRecord[i].nCurrent.storeRelease(other._pdRecord[i].nCurrent.loadAcquire());
        _pdRecord[i].nTotal.storeRelease(other._pdRecord[i].nTotal.loadAcquire());
        _pdRecord[i].sStatus = other._pdRecord[i].sStatus;
        _pdRecord[i].bIsValid.storeRelease(other._pdRecord[i].bIsValid.loadAcquire());
    }

    bIsStop.storeRelease(other.bIsStop.loadAcquire());
    nFinished.storeRelease(other.nFinished.loadAcquire());
    sInfoString = other.sInfoString;
    sErrorString = other.sErrorString;
    bForceStop.storeRelease(other.bForceStop.loadAcquire());
    nBufferSize.storeRelease(other.nBufferSize.loadAcquire());
    nFileBufferSize.storeRelease(other.nFileBufferSize.loadAcquire());
    // A value snapshot must not inherit executable registrations or raw
    // callback userdata owned by the source object.
    pCallback = nullptr;
    pCallbackUserData = nullptr;
    nLastCallbackTime.storeRelease(0);
}

XBinary::PDSTRUCT &XBinary::PDSTRUCT::operator=(const PDSTRUCT &other)
{
    if (this == &other) return *this;

    // Serialize multi-lock assignments so reciprocal copies cannot deadlock.
    static QMutex assignmentMutex;
    QMutexLocker assignmentLocker(&assignmentMutex);
    const QSharedPointer<PDSTRUCT_CALLBACK_STATE> pState = _pdCallbackState;
    QMutexLocker stateLocker(&pState->mutex);
    QMutexLocker sourceLocker(&other._pdMutex);
    QMutexLocker destinationLocker(&_pdMutex);

    QSharedPointer<PDSTRUCT_CALLBACK_STATE::ENTRY> pPreviousEntry = pState->pLegacyEntry;
    if (!pPreviousEntry.isNull()) pPreviousEntry->bActive = false;
    pState->pLegacyEntry.clear();

    for (qint32 i = 0; i < N_NUMBER_PDRECORDS; i++) {
        _pdRecord[i].nCurrent.storeRelease(other._pdRecord[i].nCurrent.loadAcquire());
        _pdRecord[i].nTotal.storeRelease(other._pdRecord[i].nTotal.loadAcquire());
        _pdRecord[i].sStatus = other._pdRecord[i].sStatus;
        _pdRecord[i].bIsValid.storeRelease(other._pdRecord[i].bIsValid.loadAcquire());
    }

    bIsStop.storeRelease(other.bIsStop.loadAcquire());
    nFinished.storeRelease(other.nFinished.loadAcquire());
    sInfoString = other.sInfoString;
    sErrorString = other.sErrorString;
    bForceStop.storeRelease(other.bForceStop.loadAcquire());
    nBufferSize.storeRelease(other.nBufferSize.loadAcquire());
    nFileBufferSize.storeRelease(other.nFileBufferSize.loadAcquire());
    pCallback = nullptr;
    pCallbackUserData = nullptr;
    nLastCallbackTime.storeRelease(0);

    destinationLocker.unlock();
    sourceLocker.unlock();
    assignmentLocker.unlock();

    if (!pPreviousEntry.isNull() && !isCurrentPdCallbackEntry(pPreviousEntry.data())) {
        while (pPreviousEntry->nInFlight > 0) {
            pState->drained.wait(&pState->mutex);
        }
    }

    return *this;
}

XBinary::PDSTRUCT::~PDSTRUCT()
{
    const QSharedPointer<PDSTRUCT_CALLBACK_STATE> pState = _pdCallbackState;
    if (pState.isNull()) return;

    {
        QMutexLocker locker(&pState->mutex);
        pState->bDestroying = true;

        if (!pState->pLegacyEntry.isNull()) pState->pLegacyEntry->bActive = false;
        for (auto it = pState->mapSubscribers.begin(); it != pState->mapSubscribers.end(); ++it) {
            it.value()->bActive = false;
        }

        // A callback may deliberately destroy its owning PDSTRUCT.  In that
        // case the invocation guard owns pState and will complete without
        // touching this object again.  Other threads must drain before the
        // mutexes and QString fields in this object disappear.
        if (!isCurrentPdCallbackState(pState.data())) {
            while (pState->nInvocations > 0) {
                pState->drained.wait(&pState->mutex);
            }
        }
    }

    _pdCallbackState.clear();
}

XBinary::PDSTRUCT XBinary::createPdStruct()
{
    return PDSTRUCT();
}

XBinary::PDSTRUCT XBinary::getPdStructSnapshot(const PDSTRUCT *pPdStruct)
{
    return pPdStruct ? PDSTRUCT(*pPdStruct) : PDSTRUCT();
}

void XBinary::setPdStructInit(PDSTRUCT *pPdStruct, qint32 nIndex, qint64 nTotal)
{
    if (pPdStruct && (nIndex >= 0) && (nIndex < N_NUMBER_PDRECORDS)) {
        QMutexLocker locker(&pPdStruct->_pdMutex);

        pPdStruct->_pdRecord[nIndex].nCurrent = 0;
        pPdStruct->_pdRecord[nIndex].nTotal = nTotal;
        pPdStruct->_pdRecord[nIndex].sStatus.clear();
        pPdStruct->_pdRecord[nIndex].bIsValid = true;
    }

    // qDebug("setPdStructInit: %d", nIndex);
}

void XBinary::setPdStructTotal(PDSTRUCT *pPdStruct, qint32 nIndex, qint64 nValue)
{
    if (pPdStruct && (nIndex >= 0) && (nIndex < N_NUMBER_PDRECORDS)) {
        QMutexLocker locker(&pPdStruct->_pdMutex);
        pPdStruct->_pdRecord[nIndex].nTotal = nValue;
    }
}

void XBinary::setPdStructCurrent(PDSTRUCT *pPdStruct, qint32 nIndex, qint64 nValue)
{
    bool bNotify = false;

    if (pPdStruct && (nIndex >= 0) && (nIndex < N_NUMBER_PDRECORDS)) {
        {
            QMutexLocker locker(&pPdStruct->_pdMutex);
            pPdStruct->_pdRecord[nIndex].nCurrent = nValue;
            bNotify = true;
        }
    }

    if (bNotify) invokePdStructCallback(pPdStruct);
}

void XBinary::setPdStructCurrentIncrement(PDSTRUCT *pPdStruct, qint32 nIndex)
{
    bool bNotify = false;

    if (pPdStruct && (nIndex >= 0) && (nIndex < N_NUMBER_PDRECORDS)) {
        {
            QMutexLocker locker(&pPdStruct->_pdMutex);
            const qint64 nCurrent = pPdStruct->_pdRecord[nIndex].nCurrent.loadAcquire();
            if (nCurrent < (std::numeric_limits<qint64>::max)()) {
                pPdStruct->_pdRecord[nIndex].nCurrent.storeRelease(nCurrent + 1);
                bNotify = true;
            }
        }
    }

    if (bNotify) invokePdStructCallback(pPdStruct);
}

void XBinary::setPdStructStatus(PDSTRUCT *pPdStruct, qint32 nIndex, const QString &sStatus)
{
    if (pPdStruct && (nIndex >= 0) && (nIndex < N_NUMBER_PDRECORDS)) {
        QMutexLocker locker(&pPdStruct->_pdMutex);
        pPdStruct->_pdRecord[nIndex].sStatus = sStatus;
    }
}

void XBinary::setPdStructFinished(PDSTRUCT *pPdStruct, qint32 nIndex)
{
    if (pPdStruct && (nIndex >= 0) && (nIndex < N_NUMBER_PDRECORDS)) {
        QMutexLocker locker(&pPdStruct->_pdMutex);

        if (pPdStruct->_pdRecord[nIndex].bIsValid.loadAcquire()) {
            pPdStruct->_pdRecord[nIndex].bIsValid = false;
            pPdStruct->_pdRecord[nIndex].nCurrent = 0;
            pPdStruct->_pdRecord[nIndex].nTotal = 0;
            pPdStruct->_pdRecord[nIndex].sStatus.clear();

            const quint64 nFinished = pPdStruct->nFinished.loadAcquire();
            if (nFinished < (std::numeric_limits<quint64>::max)()) {
                pPdStruct->nFinished.storeRelease(nFinished + 1);
            }
        }
    }

    // qDebug("setPdStructFinished: %d", nIndex);
}

void XBinary::setPdStructInfoString(PDSTRUCT *pPdStruct, const QString &sInfoString)
{
    if (pPdStruct) {
        QMutexLocker locker(&pPdStruct->_pdMutex);
        pPdStruct->sInfoString = sInfoString;
    }
}

void XBinary::setPdStructErrorString(PDSTRUCT *pPdStruct, const QString &sErrorString)
{
    if (pPdStruct) {
        QMutexLocker locker(&pPdStruct->_pdMutex);
        pPdStruct->sErrorString = sErrorString;
    }
}

void XBinary::clearPdStructInfoString(PDSTRUCT *pPdStruct)
{
    if (pPdStruct) {
        QMutexLocker locker(&pPdStruct->_pdMutex);
        pPdStruct->sInfoString.clear();
    }
}

void XBinary::clearPdStructErrorString(PDSTRUCT *pPdStruct)
{
    if (pPdStruct) {
        QMutexLocker locker(&pPdStruct->_pdMutex);
        pPdStruct->sErrorString.clear();
    }
}

QString XBinary::getPdStructInfoString(PDSTRUCT *pPdStruct)
{
    QString sResult;

    if (pPdStruct) {
        QMutexLocker locker(&pPdStruct->_pdMutex);
        sResult = pPdStruct->sInfoString;
    }

    return sResult;
}

QString XBinary::getPdStructErrorString(PDSTRUCT *pPdStruct)
{
    QString sResult;

    if (pPdStruct) {
        QMutexLocker locker(&pPdStruct->_pdMutex);
        sResult = pPdStruct->sErrorString;
    }

    return sResult;
}

qint32 XBinary::reservePdStructRecord(PDSTRUCT *pPdStruct, qint64 nTotal, const QString &sStatus)
{
    if (!pPdStruct) return -1;

    QMutexLocker locker(&pPdStruct->_pdMutex);
    for (qint32 i = 0; i < N_NUMBER_PDRECORDS; i++) {
        PDRECORD &record = pPdStruct->_pdRecord[i];
        if (!record.bIsValid.loadAcquire()) {
            record.nCurrent.storeRelease(0);
            record.nTotal.storeRelease(nTotal);
            record.sStatus = sStatus;
            record.bIsValid.storeRelease(true);
            return i;
        }
    }

#ifdef QT_DEBUG
    qDebug("Cannot reserve a PDSTRUCT record !!!");
#endif
    return -1;
}

qint32 XBinary::getFreeIndex(PDSTRUCT *pPdStruct)
{
    return reservePdStructRecord(pPdStruct, 0);
}

bool XBinary::isPdStructFinished(PDSTRUCT *pPdStruct)
{
    if (!pPdStruct) return false;

    QMutexLocker locker(&pPdStruct->_pdMutex);
    bool bResult = true;

    for (qint32 i = 0; i < N_NUMBER_PDRECORDS; i++) {
        if (pPdStruct->_pdRecord[i].bIsValid.loadAcquire()) {
            bResult = false;

            break;
        }
    }

    if (pPdStruct->nFinished.loadAcquire() == 0) {
        bResult = false;
    }

    return bResult;
}

bool XBinary::isPdStructNotCanceled(PDSTRUCT *pPdStruct)
{
    return !pPdStruct || !pPdStruct->bIsStop.loadAcquire();
}

bool XBinary::isPdStructSuccess(PDSTRUCT *pPdStruct)
{
    if (!pPdStruct) return false;

    QMutexLocker locker(&pPdStruct->_pdMutex);
    bool bFinished = pPdStruct->nFinished.loadAcquire() > 0;
    for (qint32 i = 0; i < N_NUMBER_PDRECORDS; i++) {
        if (pPdStruct->_pdRecord[i].bIsValid.loadAcquire()) {
            bFinished = false;
            break;
        }
    }

    return bFinished && !pPdStruct->bIsStop.loadAcquire() && !pPdStruct->bForceStop.loadAcquire();
}

bool XBinary::isPdStructStopped(PDSTRUCT *pPdStruct)
{
    return pPdStruct && pPdStruct->bIsStop.loadAcquire();
}

void XBinary::setPdStructStopped(PDSTRUCT *pPdStruct)
{
    if (pPdStruct) {
        pPdStruct->bIsStop.storeRelease(true);
    }
}

qint32 XBinary::getPdStructPercentage(PDSTRUCT *pPdStruct)
{
    qint32 nResult = 0;

    if (!pPdStruct) return nResult;

    QMutexLocker locker(&pPdStruct->_pdMutex);
    for (qint32 i = 0; i < N_NUMBER_PDRECORDS; i++) {
        const qint64 nTotal = pPdStruct->_pdRecord[i].nTotal.loadAcquire();
        if (pPdStruct->_pdRecord[i].bIsValid.loadAcquire() && (nTotal > 0)) {
            const qint64 nCurrent = qBound((qint64)0, pPdStruct->_pdRecord[i].nCurrent.loadAcquire(), nTotal);
            nResult = (qint32)(((long double)nCurrent * 100.0L) / (long double)nTotal);
            nResult = qBound((qint32)0, nResult, (qint32)100);

            break;
        }
    }

    return nResult;
}

void XBinary::setPdStructCallback(PDSTRUCT *pPdStruct, PDSTRUCT_CALLBACK pCallback, void *pCallbackUserData,
                                  PDSTRUCT_CALLBACK *pPreviousCallback, void **pPreviousCallbackUserData)
{
    if (!pPdStruct) return;

    const QSharedPointer<PDSTRUCT_CALLBACK_STATE> pState = pPdStruct->_pdCallbackState;
    if (pState.isNull()) return;

    QMutexLocker stateLocker(&pState->mutex);
    if (pState->bDestroying) return;

    QSharedPointer<PDSTRUCT_CALLBACK_STATE::ENTRY> pPreviousEntry;
    {
        QMutexLocker pdLocker(&pPdStruct->_pdMutex);
        if (pPreviousCallback) *pPreviousCallback = pPdStruct->pCallback;
        if (pPreviousCallbackUserData) *pPreviousCallbackUserData = pPdStruct->pCallbackUserData;

        if ((pPdStruct->pCallback == pCallback) && (pPdStruct->pCallbackUserData == pCallbackUserData) &&
            !pState->pLegacyEntry.isNull() && (pState->pLegacyEntry->pCallback == pCallback) &&
            (pState->pLegacyEntry->pUserData == pCallbackUserData)) {
            return;
        }

        pPreviousEntry = pState->pLegacyEntry;
        if (!pPreviousEntry.isNull()) pPreviousEntry->bActive = false;

        pPdStruct->pCallback = pCallback;
        pPdStruct->pCallbackUserData = pCallbackUserData;

        if (pCallback) {
            QSharedPointer<PDSTRUCT_CALLBACK_STATE::ENTRY> pEntry(new PDSTRUCT_CALLBACK_STATE::ENTRY);
            pEntry->pCallback = pCallback;
            pEntry->pUserData = pCallbackUserData;
            pState->pLegacyEntry = pEntry;
        } else {
            pState->pLegacyEntry.clear();
        }
    }

    if (!pPreviousEntry.isNull() && !isCurrentPdCallbackEntry(pPreviousEntry.data())) {
        while (pPreviousEntry->nInFlight > 0) {
            pState->drained.wait(&pState->mutex);
        }
    }
}

bool XBinary::compareAndSetPdStructCallback(PDSTRUCT *pPdStruct, PDSTRUCT_CALLBACK pExpectedCallback, void *pExpectedCallbackUserData,
                                            PDSTRUCT_CALLBACK pCallback, void *pCallbackUserData)
{
    if (!pPdStruct) return false;

    const QSharedPointer<PDSTRUCT_CALLBACK_STATE> pState = pPdStruct->_pdCallbackState;
    if (pState.isNull()) return false;

    QMutexLocker stateLocker(&pState->mutex);
    if (pState->bDestroying) return false;

    QSharedPointer<PDSTRUCT_CALLBACK_STATE::ENTRY> pPreviousEntry;
    {
        QMutexLocker pdLocker(&pPdStruct->_pdMutex);
        if ((pPdStruct->pCallback != pExpectedCallback) || (pPdStruct->pCallbackUserData != pExpectedCallbackUserData)) return false;

        pPreviousEntry = pState->pLegacyEntry;
        if (!pPreviousEntry.isNull()) pPreviousEntry->bActive = false;

        pPdStruct->pCallback = pCallback;
        pPdStruct->pCallbackUserData = pCallbackUserData;

        if (pCallback) {
            QSharedPointer<PDSTRUCT_CALLBACK_STATE::ENTRY> pEntry(new PDSTRUCT_CALLBACK_STATE::ENTRY);
            pEntry->pCallback = pCallback;
            pEntry->pUserData = pCallbackUserData;
            pState->pLegacyEntry = pEntry;
        } else {
            pState->pLegacyEntry.clear();
        }
    }

    if (!pPreviousEntry.isNull() && !isCurrentPdCallbackEntry(pPreviousEntry.data())) {
        while (pPreviousEntry->nInFlight > 0) {
            pState->drained.wait(&pState->mutex);
        }
    }

    return true;
}

XBinary::PDCALLBACKSUBSCRIPTION XBinary::subscribePdStructCallback(PDSTRUCT *pPdStruct, PDSTRUCT_CALLBACK pCallback, void *pCallbackUserData)
{
    PDCALLBACKSUBSCRIPTION result;
    if (!pPdStruct || !pCallback) return result;

    const QSharedPointer<PDSTRUCT_CALLBACK_STATE> pState = pPdStruct->_pdCallbackState;
    if (pState.isNull()) return result;

    QMutexLocker locker(&pState->mutex);
    if (pState->bDestroying) return result;

    quint64 nToken = pState->nNextToken++;
    while ((nToken == 0) || pState->mapSubscribers.contains(nToken)) {
        nToken = pState->nNextToken++;
    }

    QSharedPointer<PDSTRUCT_CALLBACK_STATE::ENTRY> pEntry(new PDSTRUCT_CALLBACK_STATE::ENTRY);
    pEntry->pCallback = pCallback;
    pEntry->pUserData = pCallbackUserData;
    pState->mapSubscribers.insert(nToken, pEntry);

    result._state = pState;
    result._token = nToken;
    return result;
}

bool XBinary::unsubscribePdStructCallback(PDCALLBACKSUBSCRIPTION *pSubscription)
{
    if (!pSubscription || !pSubscription->isValid()) return false;

    const QSharedPointer<PDSTRUCT_CALLBACK_STATE> pState = pSubscription->_state;
    const quint64 nToken = pSubscription->_token;
    pSubscription->_state.clear();
    pSubscription->_token = 0;

    QMutexLocker locker(&pState->mutex);
    const auto it = pState->mapSubscribers.find(nToken);
    if (it == pState->mapSubscribers.end()) return false;

    const QSharedPointer<PDSTRUCT_CALLBACK_STATE::ENTRY> pEntry = it.value();
    pEntry->bActive = false;
    pState->mapSubscribers.erase(it);

    if (!isCurrentPdCallbackEntry(pEntry.data())) {
        while (pEntry->nInFlight > 0) {
            pState->drained.wait(&pState->mutex);
        }
    }

    return true;
}

void XBinary::invokePdStructCallback(PDSTRUCT *pPdStruct, qint32 nMinIntervalMs)
{
    if (!pPdStruct) return;

    const QSharedPointer<PDSTRUCT_CALLBACK_STATE> pState = pPdStruct->_pdCallbackState;
    if (pState.isNull()) return;

    QSharedPointer<PDSTRUCT_CALLBACK_STATE::ENTRY> pLegacyEntry;
    QList<QSharedPointer<PDSTRUCT_CALLBACK_STATE::ENTRY>> listSubscribers;
    const qint64 nCurrentTime = QDateTime::currentMSecsSinceEpoch();
    const qint64 nInterval = qMax((qint64)0, (qint64)nMinIntervalMs);

    {
        QMutexLocker stateLocker(&pState->mutex);
        if (pState->bDestroying || pState->bInvoking) return;

        {
            // State is locked before the public-field mutex everywhere the
            // legacy registration is synchronized.
            QMutexLocker pdLocker(&pPdStruct->_pdMutex);
            if (pPdStruct->pCallback) {
                if (pState->pLegacyEntry.isNull() ||
                    (pState->pLegacyEntry->pCallback != pPdStruct->pCallback) ||
                    (pState->pLegacyEntry->pUserData != pPdStruct->pCallbackUserData)) {
                    if (!pState->pLegacyEntry.isNull()) pState->pLegacyEntry->bActive = false;
                    QSharedPointer<PDSTRUCT_CALLBACK_STATE::ENTRY> pEntry(new PDSTRUCT_CALLBACK_STATE::ENTRY);
                    pEntry->pCallback = pPdStruct->pCallback;
                    pEntry->pUserData = pPdStruct->pCallbackUserData;
                    pState->pLegacyEntry = pEntry;
                }
            } else {
                if (!pState->pLegacyEntry.isNull()) pState->pLegacyEntry->bActive = false;
                pState->pLegacyEntry.clear();
            }
            pLegacyEntry = pState->pLegacyEntry;
        }

        listSubscribers = pState->mapSubscribers.values();
        if (pLegacyEntry.isNull() && listSubscribers.isEmpty()) return;

        const qint64 nLastTime = pPdStruct->nLastCallbackTime.loadAcquire();
        const bool bDue = (nLastTime <= 0) || (nLastTime > nCurrentTime) ||
                          ((nLastTime != nCurrentTime) && (nInterval <= nCurrentTime) &&
                           (nLastTime <= (nCurrentTime - nInterval)));
        if (!bDue) return;

        pPdStruct->nLastCallbackTime.storeRelease(nCurrentTime);
        pState->bInvoking = true;
        pState->nInvocations++;
    }

    PD_CALLBACK_INVOCATION_GUARD invocationGuard(pState);

    const auto invokeEntry = [&](const QSharedPointer<PDSTRUCT_CALLBACK_STATE::ENTRY> &pEntry) -> bool {
        if (pEntry.isNull()) return true;

        {
            QMutexLocker locker(&pState->mutex);
            if (pState->bDestroying || !pEntry->bActive || !pEntry->pCallback) return !pState->bDestroying;
            pEntry->nInFlight++;
        }

        {
            PD_CALLBACK_ENTRY_GUARD entryGuard(pState, pEntry);
            pEntry->pCallback(pEntry->pUserData, pPdStruct);
        }

        QMutexLocker locker(&pState->mutex);
        return !pState->bDestroying;
    };

    if (!invokeEntry(pLegacyEntry)) return;
    for (const QSharedPointer<PDSTRUCT_CALLBACK_STATE::ENTRY> &pEntry : qAsConst(listSubscribers)) {
        if (!invokeEntry(pEntry)) return;
    }
}

XBinary::REGION_FILL XBinary::getRegionFill(qint64 nOffset, qint64 nSize, qint32 nAlignment)
{
    REGION_FILL result = {};

    if ((!m_pDevice) || (nOffset < 0) || (nSize <= 0) ||
        (nAlignment <= 0) || (nOffset >= m_pDevice->size())) {
        return result;
    }

    const qint64 nAvailable = m_pDevice->size() - nOffset;
    const qint64 nScanSize = qMin(nSize, nAvailable);
    if (nScanSize <= 0) {
        return result;
    }

    result.nByte = read_uint8(nOffset);
    if (nScanSize < nAlignment) {
        return result;
    }

    bool bError = false;
    const qint64 nChunkCapacity =
        qMin<qint64>(nAlignment, 64 * 1024);
    QByteArray baData(static_cast<int>(nChunkCapacity), '\0');

    for (qint64 nCurrentOffset = nOffset;
         nCurrentOffset <= nOffset + nScanSize - nAlignment;
         nCurrentOffset += nAlignment) {
        qint64 nBlockOffset = 0;
        while (nBlockOffset < nAlignment) {
            const qint64 nDataSize =
                qMin(nChunkCapacity,
                     static_cast<qint64>(nAlignment) - nBlockOffset);
            if (read_array(nCurrentOffset + nBlockOffset,
                           baData.data(), nDataSize) != nDataSize) {
                bError = true;
                break;
            }
            for (qint64 i = 0; i < nDataSize; i++) {
                if (static_cast<quint8>(
                        baData.at(static_cast<int>(i))) != result.nByte) {
                    bError = true;
                    break;
                }
            }
            if (bError) {
                break;
            }
            nBlockOffset += nDataSize;
        }

        if (bError) {
            break;
        }

        result.nSize += nAlignment;
    }

    return result;
}

QString XBinary::getDataString(char *pData, qint32 nDataSize, const QString &sBaseType, bool bIsBigEndian)
{
    // TODO optimize
    QString sResult;

    qint32 nElementSize = 0;
    if (sBaseType == "db") {
        nElementSize = 1;
    } else if (sBaseType == "dw") {
        nElementSize = 2;
    } else if (sBaseType == "dd") {
        nElementSize = 4;
    } else if (sBaseType == "dq") {
        nElementSize = 8;
    }

    if (pData && (nDataSize > 0) && (nElementSize > 0)) {
        for (qint32 i = 0; i <= nDataSize - nElementSize;
             i += nElementSize) {
            if (i > 0) {
                sResult.append(", ");
            }

            QString sString;

            if (sBaseType == "db") {
                sString = valueToHex(static_cast<quint8>(pData[i]));
            } else if (sBaseType == "dw") {
                quint16 nValue = 0;
                memcpy(&nValue, pData + i, sizeof(nValue));
                sString = valueToHex(nValue, bIsBigEndian);
            } else if (sBaseType == "dd") {
                quint32 nValue = 0;
                memcpy(&nValue, pData + i, sizeof(nValue));
                sString = valueToHex(nValue, bIsBigEndian);
            } else if (sBaseType == "dq") {
                quint64 nValue = 0;
                memcpy(&nValue, pData + i, sizeof(nValue));
                sString = valueToHex(nValue, bIsBigEndian);
            }

            sResult.append("0x" + sString);
        }
    }

    return sResult;
}

quint32 XBinary::hlTypeToFParts(HLTYPE hlType)
{
    Q_UNUSED(hlType)

    return 0;
}

QList<XBinary::FPART> XBinary::getHighlights(HLTYPE hlType, PDSTRUCT *pPdStruct)
{
    QList<XBinary::FPART> listResult;

    if (hlType == HLTYPE_TOTAL) {
        if (!m_pDevice || !m_pDevice->isOpen()) {
            return listResult;
        }
        FPART region = {};
        region.filePart = FILEPART_DATA;
        region.sName = tr("Total");
        region.nVirtualAddress = getBaseAddress();
        region.nVirtualSize = getImageSize();
        region.nFileOffset = 0;
        region.nFileSize = m_pDevice->size();
        listResult.append(region);
    } else {
        listResult = getFileParts(hlTypeToFParts(hlType), -1, pPdStruct);
    }

    return listResult;
}

qint64 XBinary::align_up(qint64 nValue, qint64 nAlignment)
{
    if ((nValue < 0) || (nAlignment < 0)) {
        return -1;
    }
    if (nAlignment == 0) {
        return nValue;
    }

    const qint64 nRemainder = nValue % nAlignment;
    if (nRemainder == 0) {
        return nValue;
    }
    const qint64 nDelta = nAlignment - nRemainder;
    return (nValue <= (std::numeric_limits<qint64>::max)() - nDelta) ? (nValue + nDelta) : -1;
}

qint64 XBinary::align_down(qint64 nValue, qint64 nAlignment)
{
    if ((nValue < 0) || (nAlignment < 0)) {
        return -1;
    }
    if (nAlignment == 0) {
        return nValue;
    }

    return nValue - (nValue % nAlignment);
}

char *XBinary::strCopy(char *dest, const char *src, size_t destSize)
{
    if (!dest || !src || destSize == 0) return dest;

    size_t i = 0;
    while (i + 1 < destSize && src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }

    dest[i] = '\0';
    return dest;
}

QString XBinary::getAndroidVersionFromApi(quint32 nAPI)
{
    QString sResult = tr("Unknown");

    // https://developer.android.com/tools/releases/platforms
    if (nAPI == 1) sResult = QString("1.0");
    else if (nAPI == 2) sResult = QString("1.1");
    else if (nAPI == 3) sResult = QString("1.5");
    else if (nAPI == 4) sResult = QString("1.6");
    else if (nAPI == 5) sResult = QString("2.0");
    else if (nAPI == 6) sResult = QString("2.0.1");
    else if (nAPI == 7) sResult = QString("2.1");
    else if (nAPI == 8) sResult = QString("2.2.X");
    else if (nAPI == 9) sResult = QString("2.3-2.3.2");
    else if (nAPI == 10) sResult = QString("2.3.3-2.3.7");
    else if (nAPI == 11) sResult = QString("3.0");
    else if (nAPI == 12) sResult = QString("3.1");
    else if (nAPI == 13) sResult = QString("3.2.X");
    else if (nAPI == 14) sResult = QString("4.0.1-4.0.2");
    else if (nAPI == 15) sResult = QString("4.0.3-4.0.4");
    else if (nAPI == 16) sResult = QString("4.1.X");
    else if (nAPI == 17) sResult = QString("4.2.X");
    else if (nAPI == 18) sResult = QString("4.3.X");
    else if (nAPI == 19) sResult = QString("4.4-4.4.4");
    else if (nAPI == 20) sResult = QString("4.4W");
    else if (nAPI == 21) sResult = QString("5.0");
    else if (nAPI == 22) sResult = QString("5.1");
    else if (nAPI == 23) sResult = QString("6.0");
    else if (nAPI == 24) sResult = QString("7.0");
    else if (nAPI == 25) sResult = QString("7.1");
    else if (nAPI == 26) sResult = QString("8.0");
    else if (nAPI == 27) sResult = QString("8.1");
    else if (nAPI == 28) sResult = QString("9.0");
    else if (nAPI == 29) sResult = QString("10.0");
    else if (nAPI == 30) sResult = QString("11.0");
    else if (nAPI == 31) sResult = QString("12.0");
    else if (nAPI == 32) sResult = QString("12.1");
    else if (nAPI == 33) sResult = QString("13.0");
    else if (nAPI == 34) sResult = QString("14.0");
    else if (nAPI == 35) sResult = QString("15.0");
    else if (nAPI == 36) sResult = QString("16.0");

    return sResult;
}

QString XBinary::getiOSVersionFromDarwin(quint32 nDarwin)
{
    QString sResult = tr("Unknown");

    if (nDarwin == 9) sResult = QString("1.X - 2.X");
    else if (nDarwin == 10) sResult = QString("3.X - 4.X");
    else if (nDarwin == 11) sResult = QString("5.X - 6.X");
    else if (nDarwin == 13) sResult = QString("7.X");
    else if (nDarwin == 14) sResult = QString("8.X");
    else if (nDarwin == 15) sResult = QString("9.X");
    else if (nDarwin == 16) sResult = QString("10.X");
    else if (nDarwin == 17) sResult = QString("11.X");
    else if (nDarwin == 18) sResult = QString("12.X");
    else if (nDarwin == 19) sResult = QString("13.X");
    else if (nDarwin == 20) sResult = QString("14.X");
    else if (nDarwin == 21) sResult = QString("15.X");
    else if (nDarwin == 22) sResult = QString("16.X");
    else if (nDarwin == 23) sResult = QString("17.X");
    else if (nDarwin == 24) sResult = QString("18.X");

    return sResult;
}

QString XBinary::_fromWCharArray(const wchar_t *pWString, qint32 size)
{
    QString sResult;

    if (!pWString || (size < -1)) {
        return sResult;
    }

#if (QT_VERSION_MAJOR < 5)  // TODO Check
    sResult = QString::fromWCharArray(pWString, size);
    // set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:wchar_t-")
    // set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zc:wchar_t-")
    // set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zc:wchar_t-")
    // set (CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /Zc:wchar_t-")
#else
    sResult = QString::fromWCharArray(pWString, size);
#endif

    return sResult;
}

qint32 XBinary::_toWCharArray(const QString &sString, wchar_t *pWString)
{
    // The legacy signature has no destination capacity and therefore cannot
    // perform a bounded write. Keep it as a source-compatible, fail-closed shim;
    // callers must use the capacity overload below.
    Q_UNUSED(sString)
    Q_UNUSED(pWString)
    return 0;
}

qint32 XBinary::_toWCharArray(const QString &sString, wchar_t *pWString, qint32 nCapacity)
{
    if (!pWString || (nCapacity <= 0) || (sString.size() >= nCapacity)) {
        return 0;
    }

    const qint32 nResult = sString.toWCharArray(pWString);
    if ((nResult < 0) || (nResult >= nCapacity)) {
        pWString[0] = 0;
        return 0;
    }

    pWString[nResult] = 0;
    return nResult;
}

QString XBinary::dataToString(const QByteArray &baData, DSMODE dsmode)
{
    QString sResult;

    qint32 nSize = baData.size();
    sResult.reserve(nSize);

    for (qint32 i = 0; i < nSize; i++) {
        QChar _char = QChar(baData.at(i));
        if (dsmode == DSMODE_NONE) {
            sResult += _char;
        } else if (dsmode == DSMODE_NOPRINT_TO_DOT) {
            if (_char.isPrint()) {
                sResult += _char;
            } else {
                sResult += QChar('.');
            }
        }
    }

    return sResult;
}

QList<XBinary::SIGNATURE_RECORD> XBinary::getSignatureRecords(const QString &sSignature, bool *pbValid, PDSTRUCT *pPdStruct)
{
    QList<SIGNATURE_RECORD> listResult;

    bool bValidLocal = true;
    if (!pbValid) {
        pbValid = &bValidLocal;
    }
    *pbValid = true;

    if (!isPdStructNotCanceled(pPdStruct)) {
        *pbValid = false;
        return listResult;
    }

    qint32 nSignatureSize = sSignature.size();
    qint32 i = 0;

    for (; (i < nSignatureSize) && isPdStructNotCanceled(pPdStruct);) {
        const qint32 nPreviousIndex = i;
        const qint32 nPreviousRecordCount = listResult.count();
        QChar cSymbol = sSignature.at(i);
        QChar cSymbol2;

        if ((cSymbol == QChar('%')) || (cSymbol == QChar('!')) || (cSymbol == QChar('_'))) {
            if ((i + 1) < (nSignatureSize)) {
                cSymbol2 = sSignature.at(i + 1);
            }
        }

        if (cSymbol == QChar('.')) {
            i += _getSignatureSkip(&listResult, sSignature, i);
        } else if (cSymbol == QChar('*')) {
            i += _getSignatureNotNull(&listResult, sSignature, i);
        } else if ((cSymbol == QChar('%')) && (cSymbol2 == QChar('%'))) {
            i += _getSignatureANSI(&listResult, sSignature, i);
        } else if ((cSymbol == QChar('%')) && (cSymbol2 == QChar('&'))) {
            i += _getSignatureANSIAndNumber(&listResult, sSignature, i);
        } else if ((cSymbol == QChar('!')) && (cSymbol2 == QChar('%'))) {
            i += _getSignatureNotANSI(&listResult, sSignature, i);
        } else if ((cSymbol == QChar('_')) && (cSymbol2 == QChar('%'))) {
            i += _getSignatureNotANSIAndNull(&listResult, sSignature, i);
        } else if (cSymbol == QChar('+')) {
            i += _getSignatureDelta(&listResult, sSignature, i, pbValid, pPdStruct);
        } else if (cSymbol == QChar('$')) {
            i += _getSignatureRelOffset(&listResult, sSignature, i);
        } else if (cSymbol == QChar('#')) {  // TODO Check []
            i += _getSignatureAddress(&listResult, sSignature, i);
        } else {
            qint32 nBytes = _getSignatureBytes(&listResult, sSignature, i, pbValid, pPdStruct);

            if (nBytes) {
                i += nBytes;
            } else {
                break;
            }
        }

        if (i <= nPreviousIndex) {
            *pbValid = false;
            break;
        }

        if (*pbValid) {
            if (listResult.count() != (nPreviousRecordCount + 1)) {
                *pbValid = false;
                break;
            }

            const SIGNATURE_RECORD &record = listResult.constLast();
            const qint32 nConsumed = i - nPreviousIndex;
            if (((record.st == ST_COMPAREBYTES) && (record.baData.isEmpty() || ((nConsumed & 1) != 0))) ||
                (((record.st == ST_SKIP) || (record.st == ST_NOTNULL)) && ((record.nWindowSize <= 0) || ((nConsumed & 1) != 0))) ||
                (((record.st == ST_ANSI) || (record.st == ST_NOTANSI) || (record.st == ST_NOTANSIANDNULL) || (record.st == ST_ANSINUMBER)) &&
                 (record.nWindowSize <= 0)) ||
                ((record.st == ST_FINDBYTES) && (record.baData.isEmpty() || (record.nFindDelta <= 0))) ||
                (((record.st == ST_RELOFFSET) || (record.st == ST_ADDRESS)) &&
                 (record.nSizeOfAddr != 1) && (record.nSizeOfAddr != 2) && (record.nSizeOfAddr != 4) && (record.nSizeOfAddr != 8))) {
                *pbValid = false;
                break;
            }
        }
    }

    if (!isPdStructNotCanceled(pPdStruct) || (i != nSignatureSize) || !*pbValid) {
        *pbValid = false;
        listResult.clear();
    }

    return listResult;
}

bool XBinary::_compareSignature(_MEMORY_MAP *pMemoryMap, QList<XBinary::SIGNATURE_RECORD> *pListSignatureRecords, qint64 nOffset, PDSTRUCT *pPdStruct)
{
    const qint64 fileSize = getSize();

    if (!pMemoryMap || !pListSignatureRecords || (fileSize < 0) || (nOffset < 0) || (nOffset > fileSize) ||
        !isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    const int nNumberOfSignatures = pListSignatureRecords->count();
    for (int i = 0; (i < nNumberOfSignatures) && isPdStructNotCanceled(pPdStruct); ++i) {
        const SIGNATURE_RECORD &rec = pListSignatureRecords->at(i);

        switch (rec.st) {
            case ST_COMPAREBYTES: {
                qint32 need = rec.baData.size();
                if ((need <= 0) || (nOffset < 0) || (nOffset > fileSize) || (need > (fileSize - nOffset))) return false;

                if (m_pConstMemory) {
                    const char *src = ((const char *)m_pConstMemory) + nOffset;
                    if (memcmp(src, rec.baData.constData(), (size_t)need) != 0) return false;
                } else {
                    QByteArray ba = read_array(nOffset, need);
                    if (ba.size() != need) return false;
                    if (!compareMemory(ba.constData(), rec.baData.constData(), need)) return false;
                }
                nOffset += need;
            } break;

            case ST_NOTNULL:
            case ST_ANSI:
            case ST_NOTANSI:
            case ST_NOTANSIANDNULL:
            case ST_ANSINUMBER: {
                const int need = rec.nWindowSize;
                if ((need <= 0) || (nOffset < 0) || (nOffset > fileSize) || (need > (fileSize - nOffset))) return false;

                if (m_pConstMemory) {
                    char *ptr = ((char *)m_pConstMemory) + nOffset;
                    bool ok = true;
                    if (rec.st == ST_NOTNULL) ok = _isMemoryNotNull(ptr, need);
                    else if (rec.st == ST_ANSI) ok = _isMemoryAnsi(ptr, need);
                    else if (rec.st == ST_NOTANSI) ok = _isMemoryNotAnsi(ptr, need);
                    else if (rec.st == ST_NOTANSIANDNULL) ok = _isMemoryNotAnsiAndNull(ptr, need);
                    else if (rec.st == ST_ANSINUMBER) ok = _isMemoryAnsiNumber(ptr, need);
                    if (!ok) return false;
                } else {
                    QByteArray ba = read_array(nOffset, need);
                    if (ba.size() != need) return false;
                    bool ok = true;
                    if (rec.st == ST_NOTNULL) ok = _isMemoryNotNull(ba.data(), ba.size());
                    else if (rec.st == ST_ANSI) ok = _isMemoryAnsi(ba.data(), ba.size());
                    else if (rec.st == ST_NOTANSI) ok = _isMemoryNotAnsi(ba.data(), ba.size());
                    else if (rec.st == ST_NOTANSIANDNULL) ok = _isMemoryNotAnsiAndNull(ba.data(), ba.size());
                    else if (rec.st == ST_ANSINUMBER) ok = _isMemoryAnsiNumber(ba.data(), ba.size());
                    if (!ok) return false;
                }
                nOffset += need;
            } break;

            case ST_FINDBYTES: {
                if ((rec.nFindDelta < 0) || rec.baData.isEmpty() ||
                    (rec.nFindDelta > (std::numeric_limits<qint64>::max)() - rec.baData.size())) return false;
                const qint64 limit = rec.nFindDelta + rec.baData.size();
                if ((nOffset < 0) || (nOffset > fileSize) || (limit > (fileSize - nOffset))) return false;
                qint64 where = find_byteArray(nOffset, limit, rec.baData, pPdStruct);
                if ((where < 0) || (where > fileSize) || (rec.baData.size() > (fileSize - where))) return false;
                nOffset = where + rec.baData.size();
            } break;

            case ST_SKIP: {
                const qint64 add = rec.nWindowSize;
                if ((add < 0) || (nOffset < 0) || (nOffset > fileSize) || (add > (fileSize - nOffset))) return false;
                nOffset += add;
            } break;

            case ST_RELOFFSET: {
                qint64 nValue = 0;

                if ((rec.nSizeOfAddr != 1) && (rec.nSizeOfAddr != 2) && (rec.nSizeOfAddr != 4) && (rec.nSizeOfAddr != 8)) return false;
                if ((nOffset < 0) || (nOffset > fileSize) || ((qint64)rec.nSizeOfAddr > (fileSize - nOffset))) return false;

                if (pMemoryMap->fileType == FT_AMIGAHUNK) {
                    switch (rec.nSizeOfAddr) {
                        case 1: nValue = 1 + read_int8(nOffset); break;
                        case 2: nValue = read_uint16(nOffset, isBigEndian(pMemoryMap)); break;
                        case 4: nValue = read_int32(nOffset, isBigEndian(pMemoryMap)); break;
                        case 8: nValue = read_int64(nOffset, isBigEndian(pMemoryMap)); break;
                        default: return false;
                    }
                } else {
                    switch (rec.nSizeOfAddr) {
                        case 1: nValue = 1 + read_int8(nOffset); break;
                        case 2: {
                            if (!_addSignedOffset(read_int16(nOffset, isBigEndian(pMemoryMap)), 2, &nValue)) return false;
                        } break;
                        case 4: {
                            if (!_addSignedOffset(read_int32(nOffset, isBigEndian(pMemoryMap)), 4, &nValue)) return false;
                        } break;
                        case 8: {
                            if (!_addSignedOffset(read_int64(nOffset, isBigEndian(pMemoryMap)), 8, &nValue)) return false;
                        } break;
                        default: return false;
                    }
                }

                if ((pMemoryMap->fileType == FT_COM) || (pMemoryMap->fileType == FT_MSDOS)) {
                    const qint64 _nOffset = nOffset & 0xFFFF0000;
                    const qint64 _nDelta = nOffset & 0xFFFF;
                    nOffset = _nOffset + (quint16)((quint16)_nDelta + (quint16)nValue);
                } else {
                    const XADDR nBaseAddress = offsetToAddress(pMemoryMap, nOffset);
                    XADDR nTargetAddress = (XADDR)-1;
                    if (!_addSignedAddress(nBaseAddress, nValue, &nTargetAddress)) return false;
                    nOffset = addressToOffset(pMemoryMap, nTargetAddress);
                }
            } break;

            case ST_ADDRESS: {
                XADDR _nAddress = 0;
                if ((rec.nSizeOfAddr != 1) && (rec.nSizeOfAddr != 2) && (rec.nSizeOfAddr != 4) && (rec.nSizeOfAddr != 8)) return false;
                if ((nOffset < 0) || (nOffset > fileSize) || ((qint64)rec.nSizeOfAddr > (fileSize - nOffset))) return false;
                switch (rec.nSizeOfAddr) {
                    case 1: _nAddress = read_uint8(nOffset); break;
                    case 2: _nAddress = read_uint16(nOffset, isBigEndian(pMemoryMap)); break;
                    case 4: _nAddress = read_uint32(nOffset, isBigEndian(pMemoryMap)); break;
                    case 8: _nAddress = read_uint64(nOffset, isBigEndian(pMemoryMap)); break;
                    default: return false;
                }

                if (pMemoryMap->fileType == FT_MSDOS) {
                    if (rec.nSizeOfAddr == 2) {
                        XADDR nTargetAddress = (XADDR)-1;
                        if (!_addSignedAddress(_nAddress, pMemoryMap->nCodeBase, &nTargetAddress)) return false;
                        nOffset = addressToOffset(pMemoryMap, nTargetAddress);
                    } else if (rec.nSizeOfAddr == 4) {
                        quint16 nLow = (quint16)_nAddress;
                        quint16 nHigh = (quint16)(_nAddress >> 16);
                        if ((pMemoryMap->nStartLoadOffset < 0) ||
                            !_addSignedOffset(pMemoryMap->nStartLoadOffset, (qint64)getSegmentAddress(nHigh, nLow), &nOffset)) return false;
                    }
                } else {
                    nOffset = addressToOffset(pMemoryMap, _nAddress);
                }
            } break;
        }

        if ((!isOffsetValid(pMemoryMap, nOffset)) && (!isEOD(pMemoryMap, nOffset))) {
            return false;
        }
    }

    return isPdStructNotCanceled(pPdStruct);
}

qint32 XBinary::_getSignatureSkip(QList<XBinary::SIGNATURE_RECORD> *pListSignatureRecords, const QString &sSignature, qint32 nStartIndex)
{
    qint32 nResult = 0;
    qint32 nSignatureSize = sSignature.size();

    for (qint32 i = nStartIndex; i < nSignatureSize; i++) {
        if (sSignature.at(i) == QChar('.')) {
            nResult++;
        } else {
            break;
        }
    }

    if (nResult && ((nResult & 1) == 0)) {
        SIGNATURE_RECORD record = {};

        record.st = XBinary::ST_SKIP;
        record.nSizeOfAddr = 0;
        record.nWindowSize = nResult / 2;

        pListSignatureRecords->append(record);
    }

    return nResult;
}

qint32 XBinary::_getSignatureNotNull(QList<SIGNATURE_RECORD> *pListSignatureRecords, const QString &sSignature, qint32 nStartIndex)
{
    qint32 nResult = 0;
    qint32 nSignatureSize = sSignature.size();

    for (qint32 i = nStartIndex; i < nSignatureSize; i++) {
        if (sSignature.at(i) == QChar('*')) {
            nResult++;
        } else {
            break;
        }
    }

    if (nResult && ((nResult & 1) == 0)) {
        SIGNATURE_RECORD record = {};

        record.st = XBinary::ST_NOTNULL;
        record.nSizeOfAddr = 0;
        record.nWindowSize = nResult / 2;

        pListSignatureRecords->append(record);
    }

    return nResult;
}

qint32 XBinary::_getSignatureANSI(QList<SIGNATURE_RECORD> *pListSignatureRecords, const QString &sSignature, qint32 nStartIndex)
{
    qint32 nResult = 0;
    qint32 nSignatureSize = sSignature.size();

    for (qint32 i = nStartIndex; i < nSignatureSize; i += 2) {
        if (sSignature.mid(i, 2) == "%%") {
            nResult += 2;
        } else {
            break;
        }
    }

    if (nResult) {
        SIGNATURE_RECORD record = {};

        record.st = XBinary::ST_ANSI;
        record.nSizeOfAddr = 0;
        record.nWindowSize = nResult / 2;

        pListSignatureRecords->append(record);
    }

    return nResult;
}

qint32 XBinary::_getSignatureNotANSI(QList<SIGNATURE_RECORD> *pListSignatureRecords, const QString &sSignature, qint32 nStartIndex)
{
    qint32 nResult = 0;
    qint32 nSignatureSize = sSignature.size();

    for (qint32 i = nStartIndex; i < nSignatureSize; i += 2) {
        if (sSignature.mid(i, 2) == "!%") {
            nResult += 2;
        } else {
            break;
        }
    }

    if (nResult) {
        SIGNATURE_RECORD record = {};

        record.st = XBinary::ST_NOTANSI;
        record.nSizeOfAddr = 0;
        record.nWindowSize = nResult / 2;

        pListSignatureRecords->append(record);
    }

    return nResult;
}

qint32 XBinary::_getSignatureNotANSIAndNull(QList<SIGNATURE_RECORD> *pListSignatureRecords, const QString &sSignature, qint32 nStartIndex)
{
    qint32 nResult = 0;
    qint32 nSignatureSize = sSignature.size();

    for (qint32 i = nStartIndex; i < nSignatureSize; i += 2) {
        if (sSignature.mid(i, 2) == "_%") {
            nResult += 2;
        } else {
            break;
        }
    }

    if (nResult) {
        SIGNATURE_RECORD record = {};

        record.st = XBinary::ST_NOTANSIANDNULL;
        record.nSizeOfAddr = 0;
        record.nWindowSize = nResult / 2;

        pListSignatureRecords->append(record);
    }

    return nResult;
}

qint32 XBinary::_getSignatureANSIAndNumber(QList<SIGNATURE_RECORD> *pListSignatureRecords, const QString &sSignature, qint32 nStartIndex)
{
    qint32 nResult = 0;
    qint32 nSignatureSize = sSignature.size();

    for (qint32 i = nStartIndex; i < nSignatureSize; i += 2) {
        if (sSignature.mid(i, 2) == "%&") {
            nResult += 2;
        } else {
            break;
        }
    }

    if (nResult) {
        SIGNATURE_RECORD record = {};

        record.st = XBinary::ST_ANSINUMBER;
        record.nSizeOfAddr = 0;
        record.nWindowSize = nResult / 2;

        pListSignatureRecords->append(record);
    }

    return nResult;
}

qint32 XBinary::_getSignatureDelta(QList<XBinary::SIGNATURE_RECORD> *pListSignatureRecords, const QString &sSignature, qint32 nStartIndex, bool *pbValid,
                                   PDSTRUCT *pPdStruct)
{
    // TODO Check!!!
    qint32 nResult = 0;
    qint32 nSignatureSize = sSignature.size();

    for (qint32 i = nStartIndex; i < nSignatureSize; i++) {
        if (sSignature.at(i) == QChar('+')) {
            nResult++;
        } else {
            break;
        }
    }

    if (nResult) {
        QList<XBinary::SIGNATURE_RECORD> _listSignatureRecords;

        qint32 nTemp = _getSignatureBytes(&_listSignatureRecords, sSignature, nStartIndex + nResult, pbValid, pPdStruct);

        if (_listSignatureRecords.count()) {
            SIGNATURE_RECORD record = {};

            record.st = XBinary::ST_FINDBYTES;
            record.nSizeOfAddr = 0;
            record.nWindowSize = 0;
            record.baData = _listSignatureRecords.at(0).baData;
            record.nFindDelta = (qint64)32 * nResult;

            pListSignatureRecords->append(record);

            nResult += nTemp;
        }
    }

    return nResult;
}

qint32 XBinary::_getSignatureRelOffset(QList<XBinary::SIGNATURE_RECORD> *pListSignatureRecords, const QString &sSignature, qint32 nStartIndex)
{
    qint32 nResult = 0;

    qint32 nSignatureSize = sSignature.size();

    for (qint32 i = nStartIndex; i < nSignatureSize; i++) {
        if (sSignature.at(i) == QChar('$')) {
            nResult++;
        } else {
            break;
        }
    }

    const qint32 nSizeOfAddr = nResult / 2;
    if (nResult && ((nResult & 1) == 0) &&
        ((nSizeOfAddr == 1) || (nSizeOfAddr == 2) || (nSizeOfAddr == 4) || (nSizeOfAddr == 8))) {
        SIGNATURE_RECORD record = {};

        record.st = XBinary::ST_RELOFFSET;
        record.nSizeOfAddr = nSizeOfAddr;
        record.nWindowSize = 0;

        pListSignatureRecords->append(record);
    }

    return nResult;
}

qint32 XBinary::_getSignatureAddress(QList<XBinary::SIGNATURE_RECORD> *pListSignatureRecords, const QString &sSignature, qint32 nStartIndex)
{
    const qint32 nSignatureSize = sSignature.size();
    qint32 i = nStartIndex;
    while ((i < nSignatureSize) && (sSignature.at(i) == QChar('#'))) {
        ++i;
    }

    const qint32 nNumberOfAddressSymbols = i - nStartIndex;
    QString sBaseAddress;
    bool bBaseAddressValid = true;
    if ((i < nSignatureSize) && (sSignature.at(i) == QChar('['))) {
        const qint32 nBaseStart = ++i;
        while ((i < nSignatureSize) && (sSignature.at(i) != QChar(']'))) {
            const QChar c = sSignature.at(i);
            if (!((c >= QChar('0')) && (c <= QChar('9'))) && !((c >= QChar('a')) && (c <= QChar('f')))) {
                bBaseAddressValid = false;
            }
            ++i;
        }
        if ((i >= nSignatureSize) || (i == nBaseStart)) {
            bBaseAddressValid = false;
        } else {
            sBaseAddress = sSignature.mid(nBaseStart, i - nBaseStart);
            ++i;
        }
    }

    const qint32 nResult = i - nStartIndex;
    const qint32 nSizeOfAddress = nNumberOfAddressSymbols / 2;
    bool bBaseConversionValid = true;
    const XADDR nBaseAddress = sBaseAddress.isEmpty() ? 0 : sBaseAddress.toULongLong(&bBaseConversionValid, 16);

    if (nResult && bBaseAddressValid && bBaseConversionValid && ((nNumberOfAddressSymbols & 1) == 0) &&
        ((nSizeOfAddress == 1) || (nSizeOfAddress == 2) || (nSizeOfAddress == 4) || (nSizeOfAddress == 8))) {
        SIGNATURE_RECORD record = {};

        record.st = XBinary::ST_ADDRESS;
        record.nSizeOfAddr = nSizeOfAddress;
        record.nBaseAddress = nBaseAddress;

        pListSignatureRecords->append(record);
    }

    return nResult;
}

qint32 XBinary::_getSignatureBytes(QList<XBinary::SIGNATURE_RECORD> *pListSignatureRecords, const QString &sSignature, qint32 nStartIndex, bool *pbValid,
                                   PDSTRUCT *pPdStruct)
{
    qint32 nResult = 0;

    qint32 nSignatureSize = sSignature.size();
    QString sBytes;

    for (qint32 i = nStartIndex; i < nSignatureSize; i++) {
        if (((sSignature.at(i) >= QChar('a')) && (sSignature.at(i) <= QChar('f'))) || ((sSignature.at(i) >= QChar('0')) && (sSignature.at(i) <= QChar('9')))) {
            nResult++;
            sBytes.append(sSignature.at(i));
        } else if ((sSignature.at(i) == '.') || (sSignature.at(i) == '$') || (sSignature.at(i) == '#') || (sSignature.at(i) == '*') || (sSignature.at(i) == '!') ||
                   (sSignature.at(i) == '_') || (sSignature.at(i) == '%') || (sSignature.at(i) == '+')) {
            break;
        } else {
            *pbValid = false;
            break;
        }
    }

    if (nResult && ((nResult & 1) == 0)) {
        SIGNATURE_RECORD record = {};

        record.st = XBinary::ST_COMPAREBYTES;
        record.nSizeOfAddr = 0;
        record.baData = QByteArray::fromHex(sBytes.toUtf8());  // TODO Check
        record.nWindowSize = record.baData.size();

        pListSignatureRecords->append(record);
    } else if (nResult) {
        *pbValid = false;
    }

    if (!(*pbValid)) {
        setPdStructErrorString(pPdStruct, QString("%1: %2").arg(tr("Invalid signature")).arg(sSignature));
    }

    return nResult;
}

qint64 XBinary::getPhysSize(char *pBuffer, qint64 nSize)
{
    if ((nSize <= 0) || (pBuffer == nullptr)) {
        return 0;
    }

    const qint64 wordSize = (qint64)sizeof(quint64);

    // Scan in word-sized chunks from the end for speed. Use memcpy to avoid
    // alignment UB on platforms that care.
    while (nSize >= wordSize) {
        const char *pWord = pBuffer + nSize - wordSize;
        quint64 val = 0;
        memcpy(&val, pWord, sizeof(val));

        if (val != 0) {
            // Found a non-zero word, search inside it from the end
            for (qint64 i = wordSize - 1; i >= 0; --i) {
                if (pWord[i] != 0) {
                    return (nSize - wordSize) + i + 1;
                }
            }
            // Shouldn't reach here since val != 0, but keep safe behavior
        }

        nSize -= wordSize;
    }

    // Tail: scan remaining bytes one by one
    while (nSize > 0) {
        if (pBuffer[nSize - 1] != 0) {
            break;
        }
        --nSize;
    }

    return nSize;
}

bool XBinary::isEmptyData(char *pBuffer, qint64 nSize)  // TODO dwords
{
    if (nSize < 0) {
        return false;
    }
    if (nSize == 0) {
        return true;
    }
    if (!pBuffer) {
        return false;
    }

    bool bResult = true;

    for (qint64 i = 0; i < nSize; i++) {
        char *pOffset = (pBuffer + i);

        if (*pOffset) {
            bResult = false;
            break;
        }
    }

    return bResult;
}

bool XBinary::_isOffsetValid(qint64 nOffset)
{
    qint64 nFileSize = getSize();

    return (nOffset >= 0) && (nOffset < nFileSize);
}

bool XBinary::isAddressPhysical(XADDR nAddress)
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return isAddressPhysical(&memoryMap, nAddress);
}

bool XBinary::initUnpack(UNPACK_STATE *pState, const QMap<UNPACK_PROP, QVariant> &mapProperties, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    if (pState) {
        pState->mapUnpackProperties = mapProperties;
    }

    return false;
}

bool XBinary::unpack(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pDevice)
    Q_UNUSED(pPdStruct)

    return false;
}

QList<XBinary::PM_INFO> XBinary::unpackImplemented()
{
    QList<XBinary::PM_INFO> listResult;

    return listResult;
}

QList<XBinary::PM_INFO> XBinary::packImplemented()
{
    QList<XBinary::PM_INFO> listResult;

    return listResult;
}

XBinary::ARCHIVERECORD XBinary::infoCurrent(UNPACK_STATE *pState, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pState)
    Q_UNUSED(pPdStruct)

    ARCHIVERECORD result = {};

    return result;
}

bool XBinary::unpackCurrent(UNPACK_STATE *pState, QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    QPointer<QIODevice> guardedDevice(pDevice);
    if ((!pState) || !guardedDevice) {
        return false;
    }

    if ((pState->nCurrentIndex < 0) || (pState->nCurrentIndex >= pState->nNumberOfRecords)) {
        return false;
    }

    const bool bResult = unpack(guardedDevice.data(), pPdStruct);
    return guardedDevice && bResult;
}

bool XBinary::writeUnpackData(UNPACK_STATE *pState, QIODevice *pDevice, const char *pData, qint64 nSize, PDSTRUCT *pPdStruct)
{
    if (!pState) {
        return false;
    }

    // The offset describes this invocation, including preflight failures.
    // Never leave a successful preceding record's byte count visible.
    pState->nCurrentOffset = 0;

    if (!pDevice ||
        (nSize < 0) || ((nSize > 0) && !pData) ||
        !isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    QPointer<QIODevice> guardedOutput(pDevice);
    const bool bWritable = guardedOutput->isWritable();
    if (!guardedOutput || !bWritable) return false;
    const bool bSequential = guardedOutput->isSequential();
    if (!guardedOutput || bSequential) return false;
    const QIODevice::OpenMode openMode = guardedOutput->openMode();
    if (!guardedOutput ||
        (openMode & (QIODevice::Append | QIODevice::Text))) return false;
    const bool bSeekableOutput = true;

    // Exact replacement and rollback both require truncation.  A custom
    // random-access QIODevice may implement seek()/write() but provide no
    // resize operation; reject it before the first byte instead of promising
    // rollback and leaving a partial result on failure.
    if (!isResizeEnable(pDevice)) {
        return false;
    }
    auto failOutput = [&guardedOutput, bSeekableOutput, pState]() -> bool {
        if (guardedOutput && bSeekableOutput) {
            resize(guardedOutput.data(), 0);
            if (guardedOutput) guardedOutput->seek(0);
        }
        pState->nCurrentOffset = 0;
        return false;
    };

    if (!guardedOutput) return false;
    const bool bInitialSeek = guardedOutput->seek(0);
    if (!guardedOutput || !bInitialSeek) return false;

    // A reused seekable output must not retain a tail from the preceding
    // record.  Refuse devices that cannot provide the required truncate
    // operation instead of reporting a corrupt output as successful.
    const qint64 nOldSize = guardedOutput->size();
    if (!guardedOutput) return false;
    if (nOldSize != 0) {
        if (!resize(guardedOutput.data(), 0) || !guardedOutput) return false;
        const bool bSeeked = guardedOutput->seek(0);
        if (!guardedOutput || !bSeeked) return false;
    }

    qint64 nWrittenTotal = 0;

    while (nWrittenTotal < nSize) {
        if (!isPdStructNotCanceled(pPdStruct)) {
            return failOutput();
        }

        const qint64 nChunkSize = qMin<qint64>(1ll << 20, nSize - nWrittenTotal);
        if (!guardedOutput) return failOutput();
        const bool bSeeked = guardedOutput->seek(nWrittenTotal);
        if (!guardedOutput || !bSeeked) return failOutput();
        const qint64 nWritten = guardedOutput->write(
            pData + nWrittenTotal, nChunkSize);

        // Positive short writes are legal for QIODevice.  Keep draining the
        // requested data and reject only errors, stalls, or impossible counts.
        if (!guardedOutput || (nWritten <= 0) ||
            (nWritten > nChunkSize)) {
            return failOutput();
        }

        nWrittenTotal += nWritten;
        pState->nCurrentOffset = nWrittenTotal;
    }

    if (!isPdStructNotCanceled(pPdStruct)) {
        return failOutput();
    }

    if (!guardedOutput) return failOutput();
    const bool bFinalSeek = guardedOutput->seek(nWrittenTotal);
    if (!guardedOutput || !bFinalSeek) return failOutput();

    return true;
}

bool XBinary::writeUnpackData(UNPACK_STATE *pState, QIODevice *pDevice, const QByteArray &baData, PDSTRUCT *pPdStruct)
{
    return writeUnpackData(pState, pDevice, baData.constData(), baData.size(), pPdStruct);
}

bool XBinary::moveToNext(UNPACK_STATE *pState, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pState)
    Q_UNUSED(pPdStruct)

    return false;
}

bool XBinary::finishUnpack(UNPACK_STATE *pState, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pState)
    Q_UNUSED(pPdStruct)

    return false;
}

QList<XBinary::FPART_PROP> XBinary::getAvailableFPARTProperties()
{
    QList<XBinary::FPART_PROP> listResult;

    listResult.append(FPART_PROP_ORIGINALNAME);
    listResult.append(FPART_PROP_COMPRESSEDSIZE);
    listResult.append(FPART_PROP_UNCOMPRESSEDSIZE);
    listResult.append(FPART_PROP_STREAMOFFSET);
    listResult.append(FPART_PROP_STREAMSIZE);
    listResult.append(FPART_PROP_HANDLEMETHOD);

    return listResult;
}

bool XBinary::initPack(PACK_STATE *pState, QIODevice *pDevice, const QMap<PACK_PROP, QVariant> &mapProperties, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pState)
    Q_UNUSED(pDevice)
    Q_UNUSED(mapProperties)
    Q_UNUSED(pPdStruct)

    return false;
}

bool XBinary::addDevice(PACK_STATE *pState, QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pState)
    Q_UNUSED(pDevice)
    Q_UNUSED(pPdStruct)

    return false;
}

bool XBinary::addFile(PACK_STATE *pState, const QString &sFileName, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pState)
    Q_UNUSED(sFileName)
    Q_UNUSED(pPdStruct)

    return false;
}

bool XBinary::addFolder(PACK_STATE *pState, const QString &sDirectoryPath, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pState)
    Q_UNUSED(sDirectoryPath)
    Q_UNUSED(pPdStruct)

    return false;
}

bool XBinary::finishPack(PACK_STATE *pState, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pState)
    Q_UNUSED(pPdStruct)

    return false;
}

QVariant XBinary::calculateHash(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pDevice)
    Q_UNUSED(pPdStruct)

    return 0;
}

QVariant XBinary::calculateHash(const QString &sFileName, PDSTRUCT *pPdStruct)
{
    return XBinary::_getCRC32(sFileName, pPdStruct);
}

bool XBinary::unpackSingleStream(QIODevice *pOutDevice, const QMap<UNPACK_PROP, QVariant> &mapProperties, PDSTRUCT *pPdStruct)
{
    QPointer<XBinary> guardedThis(this);
    QPointer<QIODevice> guardedOutput(pOutDevice);
    QPointer<QIODevice> guardedSource(getDevice());
    const auto isOutputUsable = [&guardedOutput]() -> bool {
        if (!guardedOutput) return false;
        const bool bOpen = guardedOutput->isOpen();
        if (!guardedOutput || !bOpen) return false;
        const bool bWritable = guardedOutput->isWritable();
        if (!guardedOutput || !bWritable) return false;
        const bool bSequential = guardedOutput->isSequential();
        if (!guardedOutput || bSequential) return false;
        const QIODevice::OpenMode openMode = guardedOutput->openMode();
        return guardedOutput &&
               !(openMode & (QIODevice::Append | QIODevice::Text));
    };

    // Publishing a verified stream is an exact-replacement transaction.  A
    // sequential or non-truncatable destination cannot be rolled back, and an
    // aliased destination could overwrite the archive before decoding ends.
    if (!guardedThis || !isOutputUsable() || !guardedThis ||
        !isResizeEnable(guardedOutput.data()) || !guardedThis ||
        !guardedOutput ||
        (guardedSource &&
         devicesAlias(guardedSource.data(), guardedOutput.data())) ||
        !guardedThis || !guardedOutput) {
        return false;
    }

    PDSTRUCT pdStructEmpty = createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    if (!isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    UNPACK_STATE state = {};
    const bool bInitialized = guardedThis &&
        guardedThis->initUnpack(&state, mapProperties, pPdStruct);
    if (!guardedThis || !bInitialized) return false;

    bool bResult = true;
    state.mapUnpackProperties = mapProperties;
    const qint32 nInitialIndex = state.nCurrentIndex;
    const qint32 nNumberOfRecords = state.nNumberOfRecords;
    if ((nInitialIndex != 0) || (nNumberOfRecords <= 0) ||
        (state.nCurrentIndex >= state.nNumberOfRecords) ||
        !isOutputUsable() || !isPdStructNotCanceled(pPdStruct)) {
        bResult = false;
    }

    ARCHIVERECORD record = {};
    if (bResult) {
        record = guardedThis->infoCurrent(&state, pPdStruct);
        if (!guardedThis) bResult = false;
    }
    if (bResult && (!guardedThis || !isOutputUsable() || !guardedThis ||
                    record.mapProperties.isEmpty() ||
                    (record.nStreamOffset < 0) ||
                    (record.nStreamSize < 0) ||
                    (state.nCurrentIndex != nInitialIndex) ||
                    (state.nNumberOfRecords != nNumberOfRecords) ||
                    !isPdStructNotCanceled(pPdStruct))) {
        bResult = false;
    }

    const CRC_TYPE crcType = (CRC_TYPE)record.mapProperties
        .value(FPART_PROP_CRC_TYPE, CRC_TYPE_UNKNOWN).toUInt();
    const bool bCheckCRC =
        isUnpackCRCEnabled(mapProperties, crcType) &&
        (crcType != CRC_TYPE_UNKNOWN) &&
        record.mapProperties.contains(FPART_PROP_RESULTCRC);
    const bool bHasExpectedSize = record.mapProperties.contains(
        FPART_PROP_UNCOMPRESSEDSIZE);
    const qint64 nExpectedSize = bHasExpectedSize
        ? record.mapProperties.value(FPART_PROP_UNCOMPRESSEDSIZE).toLongLong()
        : -1;
    if (bResult && bHasExpectedSize && (nExpectedSize < 0)) bResult = false;
    QIODevice *pWorkDevice = nullptr;
    if (bResult && (nExpectedSize >= 0)) {
        pWorkDevice = createFileBuffer(nExpectedSize, pPdStruct);
        if (!guardedThis) bResult = false;
    } else if (bResult) {
        // Unknown-size streams (notably XZ) must not be forced into a QBuffer
        // merely because their size is absent.  A growable temporary file
        // retains large-stream support while keeping publication private.
        QTemporaryFile *pTemporaryFile =
            new (std::nothrow) QTemporaryFile();
        if (pTemporaryFile && pTemporaryFile->open() &&
            isPdStructNotCanceled(pPdStruct)) {
            pWorkDevice = pTemporaryFile;
        } else {
            delete pTemporaryFile;
        }
        if (!guardedThis) bResult = false;
    }
    if (bResult && !pWorkDevice) bResult = false;
    QPointer<QIODevice> guardedWorkDevice(pWorkDevice);

    // Always decode to private storage.  CRC failure, finish failure, source
    // mutation, and cancellation therefore happen before the caller-owned
    // destination is changed.
    if (bResult) {
        bResult = guardedThis->unpackCurrent(
            &state, guardedWorkDevice.data(), pPdStruct);
        if (!guardedThis || !guardedWorkDevice ||
            (state.nCurrentIndex != nInitialIndex) ||
            (state.nNumberOfRecords != nNumberOfRecords)) {
            bResult = false;
        }
    }
    if (bResult && bCheckCRC) {
        const bool bReadable = guardedWorkDevice->isReadable();
        if (!guardedThis || !guardedWorkDevice || !bReadable) {
            bResult = false;
            setPdStructErrorString(
                pPdStruct, tr("CRC check requires a readable output device"));
        } else {
            const bool bSeeked = guardedWorkDevice->seek(0);
            if (!guardedThis || !guardedWorkDevice || !bSeeked) {
                bResult = false;
                setPdStructErrorString(
                    pPdStruct,
                    tr("CRC check requires a readable output device"));
            } else if (!checkCRC(
                           guardedWorkDevice.data(), crcType,
                           record.mapProperties.value(FPART_PROP_RESULTCRC),
                           pPdStruct) || !guardedThis ||
                       !guardedWorkDevice) {
                bResult = false;
                setPdStructErrorString(pPdStruct, tr("Invalid CRC"));
            }
        }
    }

    bool bFinished = false;
    if (guardedThis) {
        bFinished = guardedThis->finishUnpack(&state, nullptr);
        if (!guardedThis) bFinished = false;
    }
    bResult = bResult && bFinished && guardedThis && guardedWorkDevice &&
              isOutputUsable() && guardedThis &&
              isPdStructNotCanceled(pPdStruct);

    const auto rollbackOutput = [&guardedOutput]() {
        if (!guardedOutput) return;
        const bool bOpen = guardedOutput->isOpen();
        if (!guardedOutput || !bOpen) return;
        const bool bSequential = guardedOutput->isSequential();
        if (!guardedOutput || bSequential) return;
        XBinary::resize(guardedOutput.data(), 0);
        if (guardedOutput) guardedOutput->seek(0);
    };

    if (bResult) {
        const bool bWorkSeeked = guardedWorkDevice->seek(0);
        bResult = guardedThis && guardedWorkDevice && bWorkSeeked &&
                  resize(guardedOutput.data(), 0) && guardedThis &&
                  guardedOutput;
        if (bResult) {
            const bool bOutputSeeked = guardedOutput->seek(0);
            bResult = guardedThis && guardedOutput && bOutputSeeked;
        }

        const qint32 nRequestedBufferSize = getBufferSize(pPdStruct);
        QByteArray baBuffer;
        if (bResult && (nRequestedBufferSize > 0)) {
            const qint32 nBufferSize = qBound(
                (qint32)0x1000, nRequestedBufferSize,
                (qint32)0x100000);
            try {
                baBuffer.resize(nBufferSize);
            } catch (const std::bad_alloc &) {
                bResult = false;
            }
            if (baBuffer.size() != nBufferSize) bResult = false;
        } else {
            bResult = false;
        }

        qint64 nWorkSize = -1;
        if (bResult) {
            nWorkSize = guardedWorkDevice->size();
            if (!guardedThis || !guardedWorkDevice || (nWorkSize < 0))
                bResult = false;
        }
        qint64 nPublished = 0;
        while (bResult && (nPublished < nWorkSize) &&
               isPdStructNotCanceled(pPdStruct)) {
            const qint64 nChunkSize = qMin(
                nWorkSize - nPublished, (qint64)baBuffer.size());
            const bool bWorkChunkSeeked = guardedWorkDevice->seek(nPublished);
            if (!guardedThis || !guardedWorkDevice || !bWorkChunkSeeked) {
                bResult = false;
                break;
            }
            const qint64 nRead = guardedWorkDevice->read(
                baBuffer.data(), nChunkSize);
            if (!guardedThis || !guardedWorkDevice || (nRead <= 0) ||
                (nRead > nChunkSize)) {
                bResult = false;
                break;
            }

            qint64 nChunkWritten = 0;
            while (bResult && (nChunkWritten < nRead) &&
                   isPdStructNotCanceled(pPdStruct)) {
                if (!guardedThis || !isOutputUsable() || !guardedThis) {
                    bResult = false;
                    break;
                }
                const bool bSought = guardedOutput->seek(
                    nPublished + nChunkWritten);
                if (!guardedThis || !guardedOutput || !bSought) {
                    bResult = false;
                    break;
                }
                const qint64 nWritten = guardedOutput->write(
                    baBuffer.constData() + nChunkWritten,
                    nRead - nChunkWritten);
                if (!guardedThis || !guardedOutput || (nWritten <= 0) ||
                    (nWritten > nRead - nChunkWritten)) {
                    bResult = false;
                    break;
                }
                nChunkWritten += nWritten;
            }
            if (nChunkWritten != nRead) bResult = false;
            nPublished += nChunkWritten;
        }

        bResult = bResult && guardedThis && guardedWorkDevice &&
                  (nPublished == nWorkSize) && isOutputUsable() &&
                  guardedThis;
        qint64 nPublishedSize = -1;
        if (bResult) {
            nPublishedSize = guardedOutput->size();
            bResult = guardedThis && guardedOutput &&
                      (nPublishedSize == nWorkSize);
        }
        if (bResult) {
            const bool bSought = guardedOutput->seek(nWorkSize);
            bResult = guardedThis && guardedOutput && bSought;
        }
        bResult = bResult && guardedThis &&
                  isPdStructNotCanceled(pPdStruct);
        if (!bResult) {
            rollbackOutput();
            setPdStructErrorString(
                pPdStruct, tr("Cannot write unpacked output"));
        }
    }

    if (!guardedWorkDevice) pWorkDevice = nullptr;
    freeFileBuffer(&pWorkDevice);
    return bResult;
}

bool XBinary::unpackToFolder(const QString &sFolderName, const QMap<UNPACK_PROP, QVariant> &mapProperties, PDSTRUCT *pPdStruct)
{
    QPointer<XBinary> guardedThis(this);
    bool bResult = false;

    PDSTRUCT pdStructEmpty = createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    if (!isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    if (!sFolderName.isEmpty()) {
        QDir dir;

        if (!dir.exists(sFolderName)) {
            if (!dir.mkpath(sFolderName)) {
                return false;
            }
        }

        QMap<UNPACK_PROP, QVariant> mapEffectiveProperties = mapProperties;

        if (!mapEffectiveProperties.contains(UNPACK_PROP_FIXFILENAMES)) {
            mapEffectiveProperties.insert(UNPACK_PROP_FIXFILENAMES, true);
        }

        if (!mapEffectiveProperties.contains(UNPACK_PROP_OVERWRITEFILES)) {
            mapEffectiveProperties.insert(UNPACK_PROP_OVERWRITEFILES, true);
        }

        if (!mapEffectiveProperties.contains(UNPACK_PROP_CHECKCRC)) {
            mapEffectiveProperties.insert(UNPACK_PROP_CHECKCRC, true);
        }

        bool bFixFileNames = mapEffectiveProperties.value(UNPACK_PROP_FIXFILENAMES).toBool();
        bool bOverwriteFiles = mapEffectiveProperties.value(UNPACK_PROP_OVERWRITEFILES).toBool();

        QString sRootPath = QDir::fromNativeSeparators(QDir(sFolderName).absolutePath());
        QString sCanonicalRoot = QDir::fromNativeSeparators(QFileInfo(sRootPath).canonicalFilePath());

        if (sCanonicalRoot.isEmpty()) {
            sCanonicalRoot = sRootPath;
        }

#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
        const Qt::CaseSensitivity pathCaseSensitivity = Qt::CaseInsensitive;
#else
        const Qt::CaseSensitivity pathCaseSensitivity = Qt::CaseSensitive;
#endif

        auto normalizePath = [](const QString &sPath) -> QString {
            return QDir::fromNativeSeparators(QDir::cleanPath(QFileInfo(sPath).absoluteFilePath()));
        };

        auto pathKey = [pathCaseSensitivity, &normalizePath](const QString &sPath) -> QString {
            QString sResult = normalizePath(sPath);

            if (pathCaseSensitivity == Qt::CaseInsensitive) {
                sResult = sResult.toCaseFolded();
            }

            return sResult;
        };

        auto isSameOrChildPath = [pathCaseSensitivity, &normalizePath](const QString &sPath, const QString &sParentPath) -> bool {
            QString sCleanPath = normalizePath(sPath);
            QString sCleanParent = normalizePath(sParentPath);

            if (sCleanPath.compare(sCleanParent, pathCaseSensitivity) == 0) {
                return true;
            }

            if (!sCleanParent.endsWith(QLatin1Char('/'))) {
                sCleanParent.append(QLatin1Char('/'));
            }

            return sCleanPath.startsWith(sCleanParent, pathCaseSensitivity);
        };

        auto isSafeOutputPath = [pathCaseSensitivity, &normalizePath, &isSameOrChildPath, &sRootPath, &sCanonicalRoot](const QString &sPath) -> bool {
            if (!isSameOrChildPath(sPath, sRootPath) || (normalizePath(sPath).compare(normalizePath(sRootPath), pathCaseSensitivity) == 0)) {
                return false;
            }

            QFileInfo parentInfo(QFileInfo(sPath).absolutePath());

            while (!parentInfo.exists()) {
                QDir parentDir(parentInfo.absoluteFilePath());

                if (!parentDir.cdUp()) {
                    return false;
                }

                parentInfo.setFile(parentDir.absolutePath());
            }

            if (!parentInfo.isDir()) {
                return false;
            }

            QString sCanonicalParent = parentInfo.canonicalFilePath();

            return !sCanonicalParent.isEmpty() && isSameOrChildPath(sCanonicalParent, sCanonicalRoot);
        };

        auto appendDuplicateSuffix = [](const QString &sName, qint32 nSuffix) -> QString {
            qint32 nLastSlash = sName.lastIndexOf(QLatin1Char('/'));
            QString sDirectory;
            QString sBaseName = sName;

            if (nLastSlash >= 0) {
                sDirectory = sName.left(nLastSlash + 1);
                sBaseName = sName.mid(nLastSlash + 1);
            }

            qint32 nDotPosition = sBaseName.lastIndexOf(QLatin1Char('.'));
            QString sSuffix = QStringLiteral("_%1").arg(nSuffix);

            if (nDotPosition > 0) {
                return sDirectory + sBaseName.left(nDotPosition) + sSuffix + sBaseName.mid(nDotPosition);
            }

            return sDirectory + sBaseName + sSuffix;
        };

        auto relativePathKey = [](const QString &sPath) -> QString {
            QString sResult = QDir::fromNativeSeparators(QDir::cleanPath(sPath));

            if (sResult == QLatin1String(".")) {
                sResult.clear();
            }

            return sResult;
        };

        QSet<QString> setUsedPaths;
        QSet<QString> setUsedDirectories;
        QMap<QString, QString> mapResolvedDirectories;
        QMap<QString, QString> mapResolvedDirectoryOwners;

        auto resolveDirectoryName = [&](const QString &sDirectoryName, QString *pResolvedName) -> bool {
            if (!pResolvedName) {
                return false;
            }

            pResolvedName->clear();

            QString sCleanDirectoryName = QDir::fromNativeSeparators(QDir::cleanPath(sDirectoryName));

            if (sCleanDirectoryName.isEmpty() || (sCleanDirectoryName == QLatin1String("."))) {
                return true;
            }

            QStringList listParts = sCleanDirectoryName.split(QLatin1Char('/'), Qt::SkipEmptyParts);
            QString sLogicalDirectory;
            QString sResolvedDirectory;

            for (qint32 i = 0; i < listParts.count(); i++) {
                QString sPart = listParts.at(i);
                sLogicalDirectory = sLogicalDirectory.isEmpty() ? sPart : (sLogicalDirectory + QLatin1Char('/') + sPart);

                QString sLogicalKey = relativePathKey(sLogicalDirectory);

                if (mapResolvedDirectories.contains(sLogicalKey)) {
                    sResolvedDirectory = mapResolvedDirectories.value(sLogicalKey);
                    continue;
                }

                QString sBaseCandidate = sResolvedDirectory.isEmpty() ? sPart : (sResolvedDirectory + QLatin1Char('/') + sPart);
                QString sSelectedDirectory;
                qint32 nSuffix = 1;

                while (nSuffix < 1000000) {
                    QString sCandidateName = (nSuffix == 1) ? sBaseCandidate : appendDuplicateSuffix(sBaseCandidate, nSuffix);
                    QString sCandidatePath = QDir::cleanPath(QDir(sRootPath).absoluteFilePath(sCandidateName));

                    if (!isSafeOutputPath(sCandidatePath)) {
                        return false;
                    }

                    QString sCandidateKey = pathKey(sCandidatePath);
                    QFileInfo candidateInfo(sCandidatePath);
                    bool bDiskEntryExists = candidateInfo.exists() || candidateInfo.isSymLink();
                    bool bUsedByArchive = setUsedPaths.contains(sCandidateKey);
                    bool bOwnedByAnotherDirectory =
                        bUsedByArchive && (mapResolvedDirectoryOwners.value(sCandidateKey) != sLogicalKey);
                    bool bIncompatibleEntry = (bUsedByArchive && (!setUsedDirectories.contains(sCandidateKey) || bOwnedByAnotherDirectory)) ||
                                              (bDiskEntryExists && (!candidateInfo.isDir() || candidateInfo.isSymLink()));

                    if (!bIncompatibleEntry) {
                        sSelectedDirectory = sCandidateName;
                        setUsedPaths.insert(sCandidateKey);
                        setUsedDirectories.insert(sCandidateKey);
                        mapResolvedDirectoryOwners.insert(sCandidateKey, sLogicalKey);
                        break;
                    }

                    nSuffix++;
                }

                if (sSelectedDirectory.isEmpty()) {
                    return false;
                }

                mapResolvedDirectories.insert(sLogicalKey, sSelectedDirectory);
                sResolvedDirectory = sSelectedDirectory;
            }

            *pResolvedName = sResolvedDirectory;

            return true;
        };

        UNPACK_STATE state = {};

        const bool bInitialized = guardedThis &&
            guardedThis->initUnpack(&state, mapEffectiveProperties,
                                    pPdStruct);
        if (guardedThis && bInitialized) {
            const qint32 nNumberOfRecords = state.nNumberOfRecords;
            state.mapUnpackProperties = mapEffectiveProperties;

            if ((state.nCurrentIndex != 0) || (nNumberOfRecords < 0) ||
                (state.nCurrentIndex > nNumberOfRecords)) {
                if (guardedThis)
                    guardedThis->finishUnpack(&state, nullptr);
                return false;
            }
            if (state.nCurrentIndex == nNumberOfRecords) {
                const bool bFinished = guardedThis->finishUnpack(
                    &state, nullptr);
                return guardedThis && bFinished &&
                       isPdStructNotCanceled(pPdStruct);
            }

            bResult = true;
            bool bContinue = false;

            do {
                if (guardedThis && isPdStructNotCanceled(pPdStruct)) {
                    const qint32 nExpectedIndex = state.nCurrentIndex;
                    ARCHIVERECORD record = guardedThis->infoCurrent(
                        &state, pPdStruct);

                    if (!guardedThis ||
                        !isPdStructNotCanceled(pPdStruct) ||
                        record.mapProperties.isEmpty() ||
                        (record.nStreamOffset < 0) ||
                        (record.nStreamSize < 0) ||
                        (state.nCurrentIndex < 0) ||
                        (state.nCurrentIndex != nExpectedIndex) ||
                        (state.nNumberOfRecords != nNumberOfRecords) ||
                        (state.nCurrentIndex >= nNumberOfRecords)) {
                        bResult = false;
                        break;
                    }

                    QString sFileName = record.mapProperties.value(FPART_PROP_ORIGINALNAME).toString();

                    // Check if this is a directory entry (ends with '/' and has zero size)
                    bool bIsDirectory = sFileName.endsWith(QLatin1Char('/')) || record.mapProperties.value(FPART_PROP_ISFOLDER).toBool();

                    if (bFixFileNames) {
                        sFileName = fixFileName(sFileName);

                        QString sLinkName = record.mapProperties.value(FPART_PROP_LINKNAME).toString();
                        if (!sLinkName.isEmpty()) {
                            bIsDirectory = true;
                        }

                        if (sFileName.isEmpty() && !bIsDirectory) {
                            sFileName = QLatin1String("file");
                        }
                    }

                    bool bPathResolved = true;

                    if (bFixFileNames && !sFileName.isEmpty()) {
                        sFileName = QDir::fromNativeSeparators(sFileName);

                        qint32 nLastSlash = sFileName.lastIndexOf(QLatin1Char('/'));
                        QString sLogicalDirectoryName = bIsDirectory ? sFileName : ((nLastSlash >= 0) ? sFileName.left(nLastSlash) : QString());
                        QString sResolvedDirectoryName;

                        bPathResolved = resolveDirectoryName(sLogicalDirectoryName, &sResolvedDirectoryName);

                        if (bPathResolved) {
                            if (bIsDirectory) {
                                sFileName = sResolvedDirectoryName;
                            } else {
                                QString sLeafName = (nLastSlash >= 0) ? sFileName.mid(nLastSlash + 1) : sFileName;
                                sFileName = sResolvedDirectoryName.isEmpty() ? sLeafName : (sResolvedDirectoryName + QLatin1Char('/') + sLeafName);
                            }
                        }
                    }

                    if (!bPathResolved) {
                        bResult = false;
                    } else if (!sFileName.isEmpty()) {
                        sFileName = QDir::fromNativeSeparators(sFileName);
                        QString sBaseFileName = sFileName;
                        QString sFilePath;
                        bool bSkipFile = false;
                        bool bOutputSelected = false;
                        qint32 nSuffix = 1;

                        while (!bOutputSelected) {
                            QString sCandidateName = (nSuffix == 1) ? sBaseFileName : appendDuplicateSuffix(sBaseFileName, nSuffix);
                            QString sCandidatePath = QDir::cleanPath(QDir(sRootPath).absoluteFilePath(sCandidateName));

                            if (!isSafeOutputPath(sCandidatePath)) {
                                break;
                            }

                            QString sCandidateKey = pathKey(sCandidatePath);
                            QFileInfo outputInfo(sCandidatePath);
                            bool bDiskEntryExists = outputInfo.exists() || outputInfo.isSymLink();
                            bool bUsedByArchive = setUsedPaths.contains(sCandidateKey);

                            if (bIsDirectory) {
                                if (bUsedByArchive) {
                                    if (setUsedDirectories.contains(sCandidateKey)) {
                                        sFilePath = sCandidatePath;
                                        bOutputSelected = true;
                                        break;
                                    }

                                    if (bFixFileNames && (nSuffix < 1000000)) {
                                        nSuffix++;
                                        continue;
                                    }

                                    break;
                                }

                                if (bDiskEntryExists && (!outputInfo.isDir() || outputInfo.isSymLink())) {
                                    if (bFixFileNames && (nSuffix < 1000000)) {
                                        nSuffix++;
                                        continue;
                                    }

                                    break;
                                }

                                sFilePath = sCandidatePath;
                                setUsedPaths.insert(sCandidateKey);
                                setUsedDirectories.insert(sCandidateKey);
                                bOutputSelected = true;
                                break;
                            }

                            bool bIncompatibleDiskEntry = bDiskEntryExists && (!outputInfo.isFile() || outputInfo.isSymLink());
                            bool bNeedsDifferentName = bUsedByArchive || bIncompatibleDiskEntry || (bDiskEntryExists && !bOverwriteFiles);

                            if (bFixFileNames && bNeedsDifferentName) {
                                if (nSuffix >= 1000000) {
                                    break;
                                }

                                nSuffix++;
                                continue;
                            }

                            if (bIncompatibleDiskEntry) {
                                break;
                            }

                            if (!bFixFileNames && !bOverwriteFiles && (bUsedByArchive || bDiskEntryExists)) {
                                bSkipFile = true;
                            }

                            sFilePath = sCandidatePath;
                            setUsedPaths.insert(sCandidateKey);
                            bOutputSelected = true;
                        }

                        if (!bOutputSelected) {
                            bResult = false;
                        } else if (bSkipFile) {
                            bResult = true;
                        } else if (bIsDirectory) {
                            if (!dir.exists(sFilePath)) {
                                bResult = dir.mkpath(sFilePath);
                            } else {
                                bResult = XBinary::isDirectoryExists(sFilePath);
                            }
                        } else {
                            QFileInfo fileInfo(sFilePath);
                            QString sDirectoryPath = fileInfo.absolutePath();

                            if (!dir.exists(sDirectoryPath) && !dir.mkpath(sDirectoryPath)) {
                                bResult = false;
                            } else {
                                QTemporaryFile temporaryFile(QDir(sDirectoryPath).filePath(QLatin1String(".xunpack-XXXXXX")));

                                if (temporaryFile.open()) {
                                    const bool bUnpacked = guardedThis &&
                                        guardedThis->unpackCurrent(
                                            &state, &temporaryFile,
                                            pPdStruct);
                                    if (!guardedThis || !bUnpacked ||
                                        (state.nCurrentIndex !=
                                         nExpectedIndex) ||
                                        (state.nNumberOfRecords !=
                                         nNumberOfRecords)) {
#ifdef QT_DEBUG
                                        qDebug() << "Cannot unpack" << sFilePath;
#endif
                                        bResult = false;
                                    } else {
                                        bResult = true;
                                    }
                                } else {
#ifdef QT_DEBUG
                                    qDebug() << "Cannot create temporary output for:" << sFilePath;
#endif
                                    setPdStructErrorString(pPdStruct, QString("%1: %2").arg(tr("Cannot create temporary file")).arg(sFilePath));
                                    bResult = false;
                                }

                                if (bResult) {
                                    XBinary::CRC_TYPE crcType =
                                        (XBinary::CRC_TYPE)record.mapProperties.value(XBinary::FPART_PROP_CRC_TYPE, XBinary::CRC_TYPE_UNKNOWN).toUInt();

                                    if (isUnpackCRCEnabled(mapEffectiveProperties, crcType) && (crcType != XBinary::CRC_TYPE_UNKNOWN) &&
                                        record.mapProperties.contains(XBinary::FPART_PROP_RESULTCRC)) {
                                        if (temporaryFile.seek(0)) {
                                            QVariant varCRC = record.mapProperties.value(XBinary::FPART_PROP_RESULTCRC, 0);
                                            bool bCRCOk = checkCRC(&temporaryFile, crcType, varCRC, pPdStruct);
                                            bResult = guardedThis && bCRCOk;
#ifdef QT_DEBUG
                                            if (!bCRCOk) {
                                                qDebug() << "CRC is false for" << sFilePath << ": stored=" << QString::number(varCRC.toUInt(), 16);
                                            }
#endif
                                            if (!bCRCOk) {
                                                setPdStructErrorString(pPdStruct, QString("%1: %2").arg(tr("CRC check failed")).arg(sFilePath));
                                            }
                                        } else {
                                            bResult = false;
                                        }
                                    }
                                }

                                if (bResult && !bOverwriteFiles) {
                                    // The verified temporary file is already in the destination
                                    // directory. QFile::rename() does not replace an existing
                                    // target, so a file created after the precheck wins the race
                                    // without being overwritten (and the completed output never
                                    // becomes visible as a partial file).
                                    QString sTemporaryFilePath = temporaryFile.fileName();
                                    bResult = temporaryFile.flush();
                                    temporaryFile.close();

                                    if (bResult) {
                                        bResult = QFile::rename(sTemporaryFilePath, sFilePath);
                                    }

                                    if (!bResult) {
                                        setPdStructErrorString(pPdStruct, QString("%1: %2").arg(tr("Cannot write file")).arg(sFilePath));
                                    }
                                } else if (bResult) {
                                    bResult = temporaryFile.seek(0);

                                    if (bResult) {
                                        QSaveFile outputFile(sFilePath);

                                        if (outputFile.open(QIODevice::WriteOnly)) {
                                            const qint32 nRequestedBufferSize = getBufferSize(pPdStruct);
                                            QByteArray baBuffer;
                                            if (nRequestedBufferSize <= 0) {
                                                bResult = false;
                                            } else {
                                                const qint32 nBufferSize = qBound((qint32)0x1000, nRequestedBufferSize, (qint32)0x100000);
                                                try {
                                                    baBuffer.resize(nBufferSize);
                                                } catch (const std::bad_alloc &) {
                                                    bResult = false;
                                                }
                                                if (baBuffer.size() != nBufferSize) {
                                                    bResult = false;
                                                }
                                            }

                                            const qint64 nVerifiedSize = temporaryFile.size();
                                            qint64 nRemaining = nVerifiedSize;
                                            if (nRemaining < 0) {
                                                bResult = false;
                                            }

                                            while (bResult && (nRemaining > 0) && isPdStructNotCanceled(pPdStruct)) {
                                                const qint64 nToRead = qMin(nRemaining, (qint64)baBuffer.size());
                                                const qint64 nRead = temporaryFile.read(baBuffer.data(), nToRead);

                                                if ((nRead <= 0) || (nRead > nToRead)) {
                                                    bResult = false;
                                                    break;
                                                }

                                                qint64 nWrittenTotal = 0;
                                                while (bResult && (nWrittenTotal < nRead) && isPdStructNotCanceled(pPdStruct)) {
                                                    const qint64 nWritten = outputFile.write(baBuffer.constData() + nWrittenTotal, nRead - nWrittenTotal);
                                                    if ((nWritten <= 0) || (nWritten > (nRead - nWrittenTotal))) {
                                                        bResult = false;
                                                        break;
                                                    }
                                                    nWrittenTotal += nWritten;
                                                }

                                                if (nWrittenTotal != nRead) {
                                                    bResult = false;
                                                    break;
                                                }

                                                nRemaining -= nRead;
                                            }

                                            if (bResult && (nRemaining == 0) && (temporaryFile.size() == nVerifiedSize) &&
                                                isPdStructNotCanceled(pPdStruct)) {
                                                bResult = outputFile.commit();
                                            } else {
                                                outputFile.cancelWriting();
                                                bResult = false;
                                            }
                                        } else {
                                            bResult = false;
                                        }
                                    }

                                    if (!bResult) {
                                        setPdStructErrorString(pPdStruct, QString("%1: %2").arg(tr("Cannot write file")).arg(sFilePath));
                                    }
                                }
                            }
                        }

                        // Set file datetime if provided by the archive record
                        if (bResult && !bSkipFile) {
                            QVariant vDateTime = record.mapProperties.value(XBinary::FPART_PROP_DATETIME);
                            if (vDateTime.isValid() && !vDateTime.isNull()) {
                                QDateTime dt;
                                if (vDateTime.canConvert<QDateTime>()) {
                                    dt = vDateTime.toDateTime();
                                } else if (vDateTime.canConvert<quint64>()) {
                                    quint64 t = vDateTime.toULongLong();
#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
                                    dt = QDateTime::fromSecsSinceEpoch((qint64)t);
#else
                                    dt = QDateTime::fromMSecsSinceEpoch((qint64)t * 1000);
#endif
                                } else if (vDateTime.canConvert<qint64>()) {
                                    qint64 t = vDateTime.toLongLong();
#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
                                    dt = QDateTime::fromSecsSinceEpoch(t);
#else
                                    dt = QDateTime::fromMSecsSinceEpoch(t * 1000);
#endif
                                }

                                if (dt.isValid()) {
                                    XBinary::setFileDateTime(sFilePath, dt);
                                }
                            }
                        }
                    }
                } else {
                    bResult = false;
                }

                if (!bResult) {
                    break;
                }

                const qint32 nPreviousIndex = state.nCurrentIndex;
                bContinue = guardedThis &&
                    guardedThis->moveToNext(&state, pPdStruct);

                if (!guardedThis || !isPdStructNotCanceled(pPdStruct) ||
                    (state.nCurrentIndex < 0) ||
                    (state.nNumberOfRecords != nNumberOfRecords) ||
                    (state.nCurrentIndex > nNumberOfRecords)) {
                    bResult = false;
                    break;
                }
                if (bContinue) {
                    if ((state.nCurrentIndex != (nPreviousIndex + 1)) ||
                        (state.nCurrentIndex >= nNumberOfRecords)) {
                        bResult = false;
                        break;
                    }
                } else if (((nPreviousIndex + 1) != nNumberOfRecords) ||
                           ((state.nCurrentIndex != nPreviousIndex) &&
                            (state.nCurrentIndex != nNumberOfRecords))) {
                    bResult = false;
                    break;
                }

            } while (bContinue);

            const bool bFinished = guardedThis &&
                guardedThis->finishUnpack(&state, nullptr);
            bResult = bResult && guardedThis && bFinished &&
                      isPdStructNotCanceled(pPdStruct);
        }
    }

    return guardedThis && bResult;
}

bool XBinary::initFFSearch(FFSEARCH_STATE *pState, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    // Initialize search state
    pState->nCurrentOffset = pState->nStartOffset;
    pState->pContext = nullptr;

    return true;
}

XBinary::FFSEARCH_INFO XBinary::searchFFNext(FFSEARCH_STATE *pState, PDSTRUCT *pPdStruct)
{
    FFSEARCH_INFO result = {};

    if (!pState) {
        return result;
    }

    QList<QString> listSignatures = getSearchSignatures();

    if (listSignatures.isEmpty()) {
        return result;
    }

    QIODevice *pDevice = pState->pDevice;

    if (!pDevice) {
        pDevice = getDevice();
    }

    if (!pDevice) {
        return result;
    }

    QPointer<QIODevice> guardedDevice(pDevice);
    qint64 nTotalSize = guardedDevice->size();
    if (!guardedDevice || (nTotalSize < 0)) return result;
    qint64 nStartOffset = pState->nCurrentOffset;
    qint64 nSearchSize = -1;

    if (pState->nSize > 0) {
        qint64 nSearchEnd = qMin(nTotalSize, pState->nStartOffset + pState->nSize);
        nSearchSize = nSearchEnd - nStartOffset;
    }

    while (XBinary::isPdStructNotCanceled(pPdStruct)) {
        qint64 nFoundOffset = -1;

        qint32 nCount = listSignatures.count();

        for (qint32 i = 0; i < nCount; i++) {
            QString sSignature = listSignatures.at(i);

            if (!sSignature.isEmpty()) {
                qint64 nCandidate = find_signature(nullptr, nStartOffset, nSearchSize, sSignature, nullptr, pPdStruct);
                if (!guardedDevice) return result;

                if ((nCandidate != -1) && ((nFoundOffset == -1) || (nCandidate < nFoundOffset))) {
                    nFoundOffset = nCandidate;
                }
            }
        }

        if (nFoundOffset == -1) {
            break;
        }

        qint64 nEmbeddedSize = 0;
        FT fileType = FT_UNKNOWN;

        {
            SubDevice subdevice(guardedDevice.data(), nFoundOffset, -1);

            if (subdevice.open(QIODevice::ReadOnly)) {
                XBinary *pInstance = createInstance(&subdevice);

                if (pInstance) {
                    if (pInstance->isValid(pPdStruct)) {
                        nEmbeddedSize = pInstance->getFileFormatSize(pPdStruct);
                        fileType = pInstance->getFileType();
                    }

                    delete pInstance;
                    pInstance = nullptr;
                }

                subdevice.close();
            }
        }
        if (!guardedDevice) return result;

        if (nEmbeddedSize > 0) {
            if ((pState->nSize > 0) && ((nFoundOffset + nEmbeddedSize) > (pState->nStartOffset + pState->nSize))) {
                nStartOffset = nFoundOffset + 1;

                if (pState->nSize > 0) {
                    qint64 nSearchEnd = qMin(nTotalSize, pState->nStartOffset + pState->nSize);
                    nSearchSize = nSearchEnd - nStartOffset;
                }

                continue;
            }

            SubDevice subdevice(guardedDevice.data(), nFoundOffset, nEmbeddedSize);

            if (subdevice.open(QIODevice::ReadOnly)) {
                XBinary *pInstance = createInstance(&subdevice);

                if (pInstance) {
                    XBinary::FILEFORMATINFO formatInfo = pInstance->getFileFormatInfo(pPdStruct);

                    delete pInstance;
                    pInstance = nullptr;

                    if (guardedDevice && formatInfo.bIsValid) {
                        result.bIsValid = true;
                        result.fileTYPE = (fileType != FT_UNKNOWN) ? fileType : formatInfo.fileType;
                        result.nOffset = nFoundOffset;
                        result.nSize = nEmbeddedSize;
                        result.sExt = formatInfo.sExt;
                        result.sString = XBinary::getFileFormatString(&formatInfo);

                        pState->nCurrentOffset = nFoundOffset + nEmbeddedSize;

                        subdevice.close();
                        if (guardedDevice) return result;
                        return FFSEARCH_INFO();
                    }
                }

                subdevice.close();
            }
        }
        if (!guardedDevice) return FFSEARCH_INFO();

        nStartOffset = nFoundOffset + 1;

        if (pState->nSize > 0) {
            qint64 nSearchEnd = qMin(nTotalSize, pState->nStartOffset + pState->nSize);
            nSearchSize = nSearchEnd - nStartOffset;
        }
    }

    return result;
}

QList<QString> XBinary::getSearchSignatures()
{
    QList<QString> listResult;

    FT fileType = getFileType();

    if (XBinary::checkFileType(FT_PE, fileType)) {
        listResult.append("'MZ'");
    } else if (XBinary::checkFileType(FT_ELF, fileType)) {
        listResult.append("7F'ELF'");
    } else if (XBinary::checkFileType(FT_MACHOFAT, fileType)) {
        listResult.append("CAFEBABE");
        listResult.append("BEBAFECA");
    } else if (XBinary::checkFileType(FT_MACHO, fileType)) {
        listResult.append("FEEDFACE");
        listResult.append("CEFAEDFE");
        listResult.append("FEEDFACF");
        listResult.append("CFFAEDFE");
    } else if (XBinary::checkFileType(FT_PDF, fileType)) {
        listResult.append("'%PDF'");
    } else if (XBinary::checkFileType(FT_PDB, fileType)) {
        listResult.append("'Microsoft C/C++ MSF 7.00'0D0A1A'DS'000000");
        listResult.append("'Microsoft C/C++ program database 2.00'0D0A1A'JG'0000");
        listResult.append("'BSJB'............'PDB v1.0'");
    } else if (XBinary::checkFileType(FT_PNG, fileType)) {
        listResult.append("89'PNG\r\n'1A0A");
    } else if (XBinary::checkFileType(FT_JPEG, fileType)) {
        listResult.append("FFD8FF");
    } else if (XBinary::checkFileType(FT_TIFF, fileType)) {
        listResult.append("'MM'002A");
        listResult.append("'II'2A00");
    } else if (XBinary::checkFileType(FT_BMP, fileType)) {
        listResult.append("'BM'");
    } else if (XBinary::checkFileType(FT_GIF, fileType)) {
        listResult.append("'GIF8'");
    } else if (XBinary::checkFileType(FT_ICO, fileType)) {
        listResult.append("00000100");
    } else if (XBinary::checkFileType(FT_DEX, fileType)) {
        listResult.append("'dex\n'");
    } else if (XBinary::checkFileType(FT_ZIP, fileType)) {
        listResult.append("'PK'0304");
    } else if (XBinary::checkFileType(FT_RAR, fileType)) {
        listResult.append("'Rar!'1A07");
    } else if (XBinary::checkFileType(FT_GZIP, fileType)) {
        listResult.append("1F8B08");
    } else if (XBinary::checkFileType(FT_ZLIB, fileType)) {
        listResult.append("785E");
        listResult.append("789C");
        listResult.append("78DA");
    } else if (XBinary::checkFileType(FT_7Z, fileType)) {
        listResult.append("'7z'BCAF271C");
    } else if (XBinary::checkFileType(FT_CAB, fileType)) {
        listResult.append("'MSCF'");
    } else if (XBinary::checkFileType(FT_MP3, fileType)) {
        listResult.append("'ID3'");
    } else if (XBinary::checkFileType(FT_MP4, fileType)) {
        listResult.append("'ftyp'");
    } else if (XBinary::checkFileType(FT_RIFF, fileType)) {
        listResult.append("'RIFF'");
    } else if (XBinary::checkFileType(FT_LE, fileType)) {
        listResult.append("'MZ'");
    } else if (XBinary::checkFileType(FT_NE, fileType)) {
        listResult.append("'MZ'");
    } else if (XBinary::checkFileType(FT_AMIGAHUNK, fileType)) {
        listResult.append("000003F3");
        listResult.append("000003E7");
    } else if (XBinary::checkFileType(FT_JAVACLASS, fileType)) {
        listResult.append("CAFEBABE");
    } else if (XBinary::checkFileType(FT_SZDD, fileType)) {
        listResult.append("'SZDD'88F027'3A'");
    } else if (XBinary::checkFileType(FT_BZIP2, fileType)) {
        listResult.append("314159265359");
        listResult.append("17724538509000000000");
    } else if (XBinary::checkFileType(FT_LZ4, fileType)) {
        listResult.append("04224D18");
    } else if (XBinary::checkFileType(FT_LZMA, fileType)) {
        listResult.append("5D000000");
    } else if (XBinary::checkFileType(FT_LHA, fileType)) {
        listResult.append("'-lh'..2d");
        listResult.append("'-lz'..2d");
        listResult.append("'-pm'..2d");
    } else if (XBinary::checkFileType(FT_DJVU, fileType)) {
        listResult.append("'AT&TFORM'");
        listResult.append("'SDJVFORM'");
    }

    return listResult;
}

XBinary *XBinary::createInstance(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    FT fileType = getFileType();

    if (fileType != FT_BINARY) {
        return nullptr;
    }

    return new XBinary(pDevice, bIsImage, nModuleAddress);
}

bool XBinary::finishFFSearch(FFSEARCH_STATE *pState, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    if (!pState) {
        return false;
    }

    // Clean up search state
    pState->pDevice = nullptr;
    pState->nCurrentOffset = pState->nStartOffset;

    if (pState->pContext) {
        delete[] (char *)pState->pContext;
        pState->pContext = nullptr;
    }

    return true;
}

qint32 XBinary::getBufferSize(PDSTRUCT *pPdStruct)
{
    qint32 nResult = 0x4000;

    if (pPdStruct) {
        if (pPdStruct->nBufferSize) {
            nResult = pPdStruct->nBufferSize;
        }
    }

#ifdef QT_DEBUG
    if (!pPdStruct) {
        qDebug("Empty PDSTRUCT!!!");
    }
#endif

    return nResult;
}

qint32 XBinary::getFileBufferSize(PDSTRUCT *pPdStruct)
{
    qint32 nResult = 0x1000000;

    if (pPdStruct) {
        if (pPdStruct->nFileBufferSize > 0) {
            nResult = pPdStruct->nFileBufferSize;
        }
    }

#ifdef QT_DEBUG
    if (!pPdStruct) {
        qDebug("Empty PDSTRUCT!!!");
    }
#endif

    return nResult;
}

QIODevice *XBinary::createFileBuffer(qint64 nSize, PDSTRUCT *pPdStruct)
{
    QIODevice *pResult = nullptr;

    if ((nSize < 0) || !isPdStructNotCanceled(pPdStruct)) return pResult;

    qint32 nFileBufferSize = pPdStruct ? pPdStruct->nFileBufferSize.loadAcquire() : 0;
    if (nFileBufferSize <= 0) nFileBufferSize = 0x1000000;  // 16 MB

    if (nSize < nFileBufferSize) {
        QBuffer *pBuffer = new (std::nothrow) QBuffer();
        if (!pBuffer) return nullptr;

        if (pBuffer->open(QIODevice::ReadWrite)) {
            try {
                const QByteArray ba((qint32)nSize, '\0');
                if ((ba.size() == nSize) && (pBuffer->write(ba) == nSize) &&
                    pBuffer->seek(0) && isPdStructNotCanceled(pPdStruct)) {
                    pResult = pBuffer;
                    pResult->setProperty("Memory", (quint64)pBuffer->buffer().constData());
                }
            } catch (const std::bad_alloc &) {
                pResult = nullptr;
            }
        }
        if (!pResult) delete pBuffer;
    } else {
        QTemporaryFile *pTempFile = new (std::nothrow) QTemporaryFile();
        if (!pTempFile) return nullptr;
        if (pTempFile->open() && pTempFile->resize(nSize) &&
            (pTempFile->size() == nSize) && pTempFile->seek(0) &&
            isPdStructNotCanceled(pPdStruct)) {
            pResult = pTempFile;
        }
        if (!pResult) delete pTempFile;
    }

    return pResult;
}

void XBinary::freeFileBuffer(QIODevice **ppBuffer)
{
    if (ppBuffer && *ppBuffer) {
        delete *ppBuffer;
        *ppBuffer = nullptr;
    }
}

QString XBinary::getArchiveRecordComment(const ARCHIVERECORD &record)
{
    QString sResult;

    if (record.mapProperties.value(FPART_PROP_ISCOMMENTPRESENT).toBool()) {
        qint64 nCommentOffset = record.mapProperties.value(FPART_PROP_FILECOMMENTOFFSET).toLongLong();
        qint64 nCommentLength = record.mapProperties.value(FPART_PROP_FILECOMMENTLENGTH).toLongLong();

        Q_UNUSED(nCommentOffset)
        Q_UNUSED(nCommentLength)
        // Comment data must be read from the archive device at nCommentOffset with nCommentLength
        // This static method returns the comment text if stored directly in a property,
        // otherwise the caller must read from the device.
    }

    return sResult;
}

// 7-Zip prints dictionary/size values compactly: an exact power of two 2^n is
// shown as the exponent n, otherwise the value with a k/m/g suffix.
static QString _sizeValueToString(quint32 nVal)
{
    for (quint32 i = 0; i <= 31; i++) {
        if (((quint32)1 << i) == nVal) {
            return QString::number(i);
        }
    }

    QChar cSuffix;

    if ((nVal % (1u << 30)) == 0) {
        cSuffix = QChar('g');
        nVal >>= 30;
    } else if ((nVal % (1u << 20)) == 0) {
        cSuffix = QChar('m');
        nVal >>= 20;
    } else if ((nVal % (1u << 10)) == 0) {
        cSuffix = QChar('k');
        nVal >>= 10;
    }

    QString sResult = QString::number(nVal);

    if (!cSuffix.isNull()) {
        sResult += cSuffix;
    }

    return sResult;
}

// RAR/WinRAR print the dictionary with a byte-size suffix (e.g. "128k", "4m"),
// never as a bare power-of-two exponent. Used for window-size based coders.
static QString _sizeSuffixString(quint64 nVal)
{
    QChar cSuffix;

    if ((nVal >= (Q_UINT64_C(1) << 30)) && ((nVal % (Q_UINT64_C(1) << 30)) == 0)) {
        cSuffix = QChar('g');
        nVal >>= 30;
    } else if ((nVal >= (Q_UINT64_C(1) << 20)) && ((nVal % (Q_UINT64_C(1) << 20)) == 0)) {
        cSuffix = QChar('m');
        nVal >>= 20;
    } else if ((nVal >= (Q_UINT64_C(1) << 10)) && ((nVal % (Q_UINT64_C(1) << 10)) == 0)) {
        cSuffix = QChar('k');
        nVal >>= 10;
    }

    QString sResult = QString::number(nVal);

    if (!cSuffix.isNull()) {
        sResult += cSuffix;
    }

    return sResult;
}

static bool _isWindowDictMethod(XBinary::HANDLE_METHOD handleMethod)
{
    return (handleMethod == XBinary::HANDLE_METHOD_RAR_15) || (handleMethod == XBinary::HANDLE_METHOD_RAR_20) ||
           (handleMethod == XBinary::HANDLE_METHOD_RAR_29) || (handleMethod == XBinary::HANDLE_METHOD_RAR_50) ||
           (handleMethod == XBinary::HANDLE_METHOD_RAR_70);
}

QString XBinary::getCoderParamsString(HANDLE_METHOD handleMethod, const QByteArray &baProperties)
{
    QString sResult;

    if (handleMethod == HANDLE_METHOD_LZMA2) {
        // LZMA2 dictionary size is one property byte:
        //   b > 40 invalid; b == 40 -> 4 GiB-1; else (2 | (b & 1)) << (b/2 + 11)
        if (baProperties.size() >= 1) {
            quint8 nByte = (quint8)baProperties.at(0);

            if (nByte < 40) {
                quint32 nDict = (2u | (nByte & 1u)) << ((nByte / 2) + 11);
                sResult = _sizeValueToString(nDict);
            } else if (nByte == 40) {
                sResult = _sizeValueToString(0xFFFFFFFFu);
            }
        }
    } else if (handleMethod == HANDLE_METHOD_LZMA) {
        // LZMA properties: byte 0 packs lc/lp/pb, bytes 1..4 are the LE dict size.
        if (baProperties.size() >= 5) {
            quint32 nDict = (quint8)baProperties.at(1) | ((quint32)(quint8)baProperties.at(2) << 8) | ((quint32)(quint8)baProperties.at(3) << 16) |
                            ((quint32)(quint8)baProperties.at(4) << 24);
            sResult = _sizeValueToString(nDict);

            quint32 nEncoded = (quint8)baProperties.at(0);
            quint32 nLc = nEncoded % 9;
            nEncoded /= 9;
            quint32 nLp = nEncoded % 5;
            quint32 nPb = nEncoded / 5;

            if ((nLc != 3) || (nLp != 0) || (nPb != 2)) {
                sResult += QString(":lc%1:lp%2:pb%3").arg(nLc).arg(nLp).arg(nPb);
            }
        }
    }

    return sResult;
}

QString XBinary::getHandleMethods(const QMap<FPART_PROP, QVariant> &mapProperties)
{
    QString sResult;

    HANDLE_METHOD handleMethod = (HANDLE_METHOD)(mapProperties.value(XBinary::FPART_PROP_HANDLEMETHOD).toULongLong());
    HANDLE_METHOD handleMethod2 = (HANDLE_METHOD)(mapProperties.value(XBinary::FPART_PROP_HANDLEMETHOD2).toULongLong());

    // QString sHandleMethod3 = handleMethodToString(mapProperties.value(XBinary::FPART_PROP_HANDLEMETHOD3).toULongLong());

    if (handleMethod2 != HANDLE_METHOD_UNKNOWN) {
        QString sMethod2 = handleMethodToString(handleMethod2);
        QString sParams2 = getCoderParamsString(handleMethod2, mapProperties.value(XBinary::FPART_PROP_COMPRESSPROPERTIES2).toByteArray());

        if (!sParams2.isEmpty()) {
            sMethod2 += QString(":") + sParams2;
        }

        sResult = appendText(sResult, sMethod2, "/");
    }

    if (handleMethod != HANDLE_METHOD_UNKNOWN) {
        QString sMethod = handleMethodToString(handleMethod);
        QString sParams = getCoderParamsString(handleMethod, mapProperties.value(XBinary::FPART_PROP_COMPRESSPROPERTIES).toByteArray());

        // RAR (and other window-based coders) store the dictionary as a window
        // size rather than coder property bytes (UnRAR shows this as -md=128k).
        if (sParams.isEmpty() && _isWindowDictMethod(handleMethod)) {
            quint64 nWindowSize = mapProperties.value(XBinary::FPART_PROP_WINDOWSIZE).toULongLong();

            if (nWindowSize > 0) {
                sParams = _sizeSuffixString(nWindowSize);
            }
        }

        if (!sParams.isEmpty()) {
            sMethod += QString(":") + sParams;
        }

        sResult = appendText(sResult, sMethod, "/");
    }

    return sResult;
}

bool XBinary::handleInternalInfo(PDSTRUCT *pPdStruct)
{
    bool bResult = true;

    if (!isInternalInfoHandled()) {
        m_internalInfo = XBinary::INTERNAL_INFO();
        // getMemoryMap() is virtual and may re-enter getInternalInfo() through
        // format-specific helpers. Mark the cache as being handled first so
        // that such calls observe the stable object-owned cache.
        setIsInternalInfoHandled(true);
        m_internalInfo.memoryMap = getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);
    }

    return bResult;
}

void *XBinary::getInternalInfo(PDSTRUCT *pPdStruct)
{
    handleInternalInfo(pPdStruct);

    return &m_internalInfo;
}

void XBinary::setInternalInfo(void *pInternalInfo)
{
    if (pInternalInfo) {
        m_internalInfo = *(XBinary::INTERNAL_INFO *)pInternalInfo;
        setIsInternalInfoHandled(true);
    } else {
        m_internalInfo = XBinary::INTERNAL_INFO();
        setIsInternalInfoHandled(false);
    }
}

bool XBinary::isInternalInfoHandled()
{
    return m_bIsInternalInfoHandled;
}

void XBinary::setIsInternalInfoHandled(bool bState)
{
    m_bIsInternalInfoHandled = bState;
}

struct _XFStringRun {
    qint64 nRunBytes;
    qint64 nRecordBytes;
    qint32 nChars;
    qint32 nRecordChars;
};

static bool _isXFPrintableByte(quint8 nByte)
{
    return ((nByte >= 0x20) && (nByte < 0x80));
}

static bool _isXFAnsiByte(quint8 nByte)
{
    return (nByte >= 0x20);
}

struct _XFAnsiCodepageMask {
    quint32 nCodepage;
    quint32 anMask[4];
};

static const _XFAnsiCodepageMask g_XFAnsiCodepageMasks[] = {
    {XBinary::CODEPAGE_WINDOWS_874, {0x00FE0021, 0xFFFFFFFF, 0x87FFFFFF, 0x0FFFFFFF}},
    {XBinary::CODEPAGE_WINDOWS_1250, {0xFEFEFEF5, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},
    {XBinary::CODEPAGE_WINDOWS_1251, {0xFEFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},
    {XBinary::CODEPAGE_WINDOWS_1252, {0xDFFE5FFD, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},
    {XBinary::CODEPAGE_WINDOWS_1253, {0x0AFE0AFD, 0xFFFFFBFF, 0xFFFBFFFF, 0x7FFFFFFF}},
    {XBinary::CODEPAGE_WINDOWS_1254, {0x9FFE1FFD, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},
    {XBinary::CODEPAGE_WINDOWS_1255, {0x0BFE0BFD, 0xFFFFFFFF, 0x01FFFBFF, 0x67FFFFFF}},
    {XBinary::CODEPAGE_WINDOWS_1256, {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},
    {XBinary::CODEPAGE_WINDOWS_1257, {0x6AFEEAF5, 0xFFFFFFDD, 0xFFFFFFFF, 0xFFFFFFFF}},
    {XBinary::CODEPAGE_WINDOWS_1258, {0x9BFE1BFD, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},
};

static const _XFAnsiCodepageMask *_getXFAnsiCodepageMask(quint32 nCodepage)
{
    for (qint32 i = 0; i < (qint32)(sizeof(g_XFAnsiCodepageMasks) / sizeof(g_XFAnsiCodepageMasks[0])); i++) {
        if (g_XFAnsiCodepageMasks[i].nCodepage == nCodepage) {
            return &(g_XFAnsiCodepageMasks[i]);
        }
    }

    return nullptr;
}

static bool _isXFAnsiCodepageByte(quint8 nByte, const _XFAnsiCodepageMask *pCodepageMask, bool bFallbackAnsi)
{
    if (nByte < 0x80) {
        return _isXFPrintableByte(nByte);
    }

    if (pCodepageMask) {
        qint32 nBit = nByte - 0x80;

        return (pCodepageMask->anMask[nBit / 32] & ((quint32)1 << (nBit & 31))) != 0;
    }

    return bFallbackAnsi && _isXFAnsiByte(nByte);
}

static bool _isXFPrintableUnicode(quint32 nCode)
{
    if ((nCode < 0x20) || (nCode > 0x10FFFF)) {
        return false;
    }

    if ((nCode >= 0xD800) && (nCode <= 0xDFFF)) {
        return false;
    }

    if (((nCode & 0xFFFF) == 0xFFFE) || ((nCode & 0xFFFF) == 0xFFFF)) {
        return false;
    }

    return true;
}

static quint16 _readXFUInt16(const char *pData, bool bIsBigEndian)
{
    const quint8 *p = (const quint8 *)pData;

    if (bIsBigEndian) {
        return (quint16)(((quint16)p[0] << 8) | p[1]);
    }

    return (quint16)(((quint16)p[1] << 8) | p[0]);
}

static quint32 _readXFUInt32(const char *pData, bool bIsBigEndian)
{
    const quint8 *p = (const quint8 *)pData;

    if (bIsBigEndian) {
        return ((quint32)p[0] << 24) | ((quint32)p[1] << 16) | ((quint32)p[2] << 8) | p[3];
    }

    return ((quint32)p[3] << 24) | ((quint32)p[2] << 16) | ((quint32)p[1] << 8) | p[0];
}

static qint32 _getXFUtf8Width(const char *pData, qint64 nSize)
{
    const quint8 *p = (const quint8 *)pData;
    quint32 nCode = 0;

    if (nSize <= 0) {
        return 0;
    }

    if (p[0] < 0x80) {
        return _isXFPrintableUnicode(p[0]) ? 1 : 0;
    }

    if (((p[0] & 0xE0) == 0xC0) && (nSize >= 2) && ((p[1] & 0xC0) == 0x80)) {
        nCode = ((quint32)(p[0] & 0x1F) << 6) | (p[1] & 0x3F);

        return ((nCode >= 0x80) && _isXFPrintableUnicode(nCode)) ? 2 : 0;
    }

    if (((p[0] & 0xF0) == 0xE0) && (nSize >= 3) && ((p[1] & 0xC0) == 0x80) && ((p[2] & 0xC0) == 0x80)) {
        nCode = ((quint32)(p[0] & 0x0F) << 12) | ((quint32)(p[1] & 0x3F) << 6) | (p[2] & 0x3F);

        return ((nCode >= 0x800) && _isXFPrintableUnicode(nCode)) ? 3 : 0;
    }

    if (((p[0] & 0xF8) == 0xF0) && (nSize >= 4) && ((p[1] & 0xC0) == 0x80) && ((p[2] & 0xC0) == 0x80) && ((p[3] & 0xC0) == 0x80)) {
        nCode = ((quint32)(p[0] & 0x07) << 18) | ((quint32)(p[1] & 0x3F) << 12) | ((quint32)(p[2] & 0x3F) << 6) | (p[3] & 0x3F);

        return ((nCode >= 0x10000) && _isXFPrintableUnicode(nCode)) ? 4 : 0;
    }

    return 0;
}

static _XFStringRun _countXFAsciiRun(const char *pData, qint64 nSize, qint32 nMaxChars)
{
    _XFStringRun result = {};

#ifdef USE_XSIMD
    qint64 nScalarLimit = qMin(nSize, qMax((qint64)nMaxChars + 1, (qint64)64));

    while ((result.nRunBytes < nScalarLimit) && _isXFPrintableByte((quint8)pData[result.nRunBytes])) {
        result.nRunBytes++;
    }

    if ((result.nRunBytes == nScalarLimit) && (result.nRunBytes < nSize) && _isXFPrintableByte((quint8)pData[result.nRunBytes])) {
        result.nRunBytes += xsimd_count_ansi_prefix(pData + result.nRunBytes, nSize - result.nRunBytes);
    }
#else
    while ((result.nRunBytes < nSize) && _isXFPrintableByte((quint8)pData[result.nRunBytes])) {
        result.nRunBytes++;
    }
#endif

    result.nChars = (result.nRunBytes > 0x7FFFFFFF) ? 0x7FFFFFFF : (qint32)result.nRunBytes;
    result.nRecordBytes = qMin(qMin(result.nRunBytes, (qint64)nMaxChars), (qint64)0xFFFF);
    result.nRecordChars = (qint32)result.nRecordBytes;

    return result;
}

static _XFStringRun _countXFAnsiRun(const char *pData, qint64 nSize, qint32 nMaxChars, const _XFAnsiCodepageMask *pCodepageMask, bool bFallbackAnsi)
{
    _XFStringRun result = {};

    while (result.nRunBytes < nSize) {
        quint8 nByte = (quint8)pData[result.nRunBytes];

        if (_isXFPrintableByte(nByte)) {
#ifdef USE_XSIMD
            qint64 nAsciiRun = xsimd_count_ansi_prefix(pData + result.nRunBytes, nSize - result.nRunBytes);

            if (nAsciiRun > 0) {
                result.nRunBytes += nAsciiRun;
                continue;
            }
#endif
            result.nRunBytes++;
        } else if (_isXFAnsiCodepageByte(nByte, pCodepageMask, bFallbackAnsi)) {
            result.nRunBytes++;
        } else {
            break;
        }
    }

    result.nChars = (result.nRunBytes > 0x7FFFFFFF) ? 0x7FFFFFFF : (qint32)result.nRunBytes;
    result.nRecordBytes = qMin(qMin(result.nRunBytes, (qint64)nMaxChars), (qint64)0xFFFF);
    result.nRecordChars = (qint32)result.nRecordBytes;

    return result;
}

static _XFStringRun _countXFUtf8Run(const char *pData, qint64 nSize, qint32 nMaxChars)
{
    _XFStringRun result = {};
    qint32 nRecordChars = 0;

    while (result.nRunBytes < nSize) {
        qint32 nWidth = _getXFUtf8Width(pData + result.nRunBytes, nSize - result.nRunBytes);

        if (nWidth == 0) {
            break;
        }

        if ((nRecordChars < nMaxChars) && (result.nRecordBytes + nWidth <= 0xFFFF)) {
            result.nRecordBytes += nWidth;
            nRecordChars++;
            result.nRecordChars = nRecordChars;
        }

        result.nRunBytes += nWidth;
        if (result.nChars < 0x7FFFFFFF) {
            result.nChars++;
        }
    }

    return result;
}

static _XFStringRun _countXFUtf16Run(const char *pData, qint64 nSize, qint32 nMaxChars, bool bIsBigEndian)
{
    _XFStringRun result = {};
    qint64 nEvenSize = (nSize / 2) * 2;
    const qint32 nRecordCharLimit = qMin(nMaxChars, (qint32)(0xFFFF / 2));

    while (result.nRunBytes + 1 < nEvenSize) {
#ifdef USE_XSIMD
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
        if (!bIsBigEndian) {
            const char *pCurrentData = pData + result.nRunBytes;
            qint64 nSimdBytes = 0;

            if ((((quintptr)pCurrentData) & (alignof(quint16) - 1)) == 0) {
                nSimdBytes = xsimd_count_unicode_prefix(pCurrentData, nEvenSize - result.nRunBytes);
            }

            if (nSimdBytes > 0) {
                qint64 nSimdChars = nSimdBytes / 2;

                if ((result.nRecordBytes == 0) && (result.nChars < nRecordCharLimit) && (result.nChars + nSimdChars >= nRecordCharLimit)) {
                    result.nRecordBytes = result.nRunBytes + ((qint64)(nRecordCharLimit - result.nChars) * 2);
                }

                result.nRunBytes += nSimdBytes;
                result.nChars = (result.nChars + nSimdChars > 0x7FFFFFFF) ? 0x7FFFFFFF : (qint32)(result.nChars + nSimdChars);
                continue;
            }
        }
#endif
#endif

        quint16 nCode = _readXFUInt16(pData + result.nRunBytes, bIsBigEndian);

        if (!XBinary::isUnicodeSymbol(nCode, true)) {
            break;
        }

        if ((result.nRecordBytes == 0) && (result.nChars == (nRecordCharLimit - 1))) {
            result.nRecordBytes = result.nRunBytes + 2;
        }

        result.nRunBytes += 2;
        result.nChars++;
    }

    if (result.nRecordBytes == 0) {
        result.nRecordBytes = result.nRunBytes;
    }
    result.nRecordChars = (qint32)(result.nRecordBytes / 2);

    return result;
}

static _XFStringRun _countXFUtf32Run(const char *pData, qint64 nSize, qint32 nMaxChars, bool bIsBigEndian)
{
    _XFStringRun result = {};
    qint64 nEvenSize = (nSize / 4) * 4;
    const qint32 nRecordCharLimit = qMin(nMaxChars, (qint32)(0xFFFF / 4));

    while (result.nRunBytes + 3 < nEvenSize) {
        quint32 nCode = _readXFUInt32(pData + result.nRunBytes, bIsBigEndian);

        if (!_isXFPrintableUnicode(nCode)) {
            break;
        }

        if ((result.nRecordBytes == 0) && (result.nChars == (nRecordCharLimit - 1))) {
            result.nRecordBytes = result.nRunBytes + 4;
        }

        result.nRunBytes += 4;
        result.nChars++;
    }

    if (result.nRecordBytes == 0) {
        result.nRecordBytes = result.nRunBytes;
    }
    result.nRecordChars = (qint32)(result.nRecordBytes / 4);

    return result;
}

static void _appendXFStringRecord(QVector<XBinary::MS_RECORD> *pListResult, XBinary::_MEMORY_MAP *pMemoryMap, qint64 nStringOffset, qint64 nStringSize,
                                  XBinary::VT valueType, quint16 nInfo, qint32 nCodeUnitSize)
{
    if ((nStringSize <= 0) || (nCodeUnitSize <= 0)) {
        return;
    }

    const qint64 nMaximumAlignedSize = (0xFFFF / nCodeUnitSize) * nCodeUnitSize;
    nStringSize = qMin(nStringSize, nMaximumAlignedSize);
    nStringSize -= nStringSize % nCodeUnitSize;

    if (nStringSize <= 0) {
        return;
    }

    XBinary::MS_RECORD record = {};
    record.nValueType = valueType;
    record.nSize = (quint16)nStringSize;
    record.nInfo = nInfo;
    _setMSRecordLocation(&record, pMemoryMap, nStringOffset);

    pListResult->append(record);
}

static void _trimXFStringRecords(QVector<XBinary::MS_RECORD> *pListResult, qint32 nLimit)
{
    _retainBestMSRecords(pListResult, nLimit);
}

static void _scanXFByteStrings(QVector<XBinary::MS_RECORD> *pListResult, XBinary::_MEMORY_MAP *pMemoryMap, const char *pBuffer, qint64 nBufferOffset,
                               qint64 nBufferSize, qint64 nEmitStart, qint64 nEmitEnd, qint32 nMinChars, qint32 nMaxChars, qint32 nLimit, XBinary::VT valueType,
                               quint32 nCodepage)
{
    qint64 i = 0;
    quint16 nInfo = (nCodepage <= 0xFFFF) ? (quint16)nCodepage : 0;
    const _XFAnsiCodepageMask *pCodepageMask = _getXFAnsiCodepageMask(nCodepage);
    bool bFallbackAnsi = nCodepage && (!pCodepageMask);

    while (i < nBufferSize) {
        if (nCodepage) {
            while ((i < nBufferSize) && (!_isXFAnsiCodepageByte((quint8)pBuffer[i], pCodepageMask, bFallbackAnsi))) {
                i++;
            }
        } else {
            while ((i < nBufferSize) && (!_isXFPrintableByte((quint8)pBuffer[i]))) {
                i++;
            }
        }

        if (i >= nBufferSize) {
            break;
        }

        _XFStringRun run = nCodepage ? _countXFAnsiRun(pBuffer + i, nBufferSize - i, nMaxChars, pCodepageMask, bFallbackAnsi)
                                     : _countXFAsciiRun(pBuffer + i, nBufferSize - i, nMaxChars);

        if (run.nRunBytes == 0) {
            i++;
            continue;
        }

        qint64 nStringOffset = nBufferOffset + i;

        if ((run.nRecordChars >= nMinChars) && (nStringOffset >= nEmitStart) && (nStringOffset < nEmitEnd)) {
            _appendXFStringRecord(pListResult, pMemoryMap, nStringOffset, run.nRecordBytes, valueType, nInfo, 1);
            _trimXFStringRecords(pListResult, nLimit);
        }

        i += run.nRunBytes;
    }
}

static void _scanXFUtf8Strings(QVector<XBinary::MS_RECORD> *pListResult, XBinary::_MEMORY_MAP *pMemoryMap, const char *pBuffer, qint64 nBufferOffset,
                               qint64 nBufferSize, qint64 nEmitStart, qint64 nEmitEnd, qint32 nMinChars, qint32 nMaxChars, qint32 nLimit)
{
    qint64 i = 0;

    while (i < nBufferSize) {
        _XFStringRun run = _countXFUtf8Run(pBuffer + i, nBufferSize - i, nMaxChars);

        if (run.nRunBytes == 0) {
            i++;
            continue;
        }

        qint64 nStringOffset = nBufferOffset + i;

        if ((run.nRecordChars >= nMinChars) && (nStringOffset >= nEmitStart) && (nStringOffset < nEmitEnd)) {
            _appendXFStringRecord(pListResult, pMemoryMap, nStringOffset, run.nRecordBytes, XBinary::VT_UTF8, 0, 1);
            _trimXFStringRecords(pListResult, nLimit);
        }

        i += run.nRunBytes;
    }
}

static void _scanXFUtf16Strings(QVector<XBinary::MS_RECORD> *pListResult, XBinary::_MEMORY_MAP *pMemoryMap, const char *pBuffer, qint64 nBufferOffset,
                                qint64 nBufferSize, qint64 nEmitStart, qint64 nEmitEnd, qint32 nMinChars, qint32 nMaxChars, qint32 nLimit, bool bIsBigEndian)
{
    QVector<XBinary::MS_RECORD> listAllLanes;

    for (qint64 nLane = 0; nLane < 2; nLane++) {
        QVector<XBinary::MS_RECORD> listLaneRecords;
        qint64 i = nLane;

        while (i + 1 < nBufferSize) {
            _XFStringRun run = _countXFUtf16Run(pBuffer + i, nBufferSize - i, nMaxChars, bIsBigEndian);

            if (run.nRunBytes == 0) {
                i += 2;
                continue;
            }

            qint64 nStringOffset = nBufferOffset + i;

            if ((run.nRecordChars >= nMinChars) && (nStringOffset >= nEmitStart) && (nStringOffset < nEmitEnd)) {
                _appendXFStringRecord(&listLaneRecords, pMemoryMap, nStringOffset, run.nRecordBytes, XBinary::VT_U,
                                      (quint16)(bIsBigEndian ? XBinary::ENDIAN_BIG : XBinary::ENDIAN_LITTLE), 2);
                _trimXFStringRecords(&listLaneRecords, nLimit);
            }

            i += run.nRunBytes;
        }

        _finalizeBestMSRecords(&listLaneRecords, nLimit);
        listAllLanes += listLaneRecords;
        _finalizeBestMSRecords(&listAllLanes, nLimit);
    }

    *pListResult += listAllLanes;
    _finalizeBestMSRecords(pListResult, nLimit);
}

static void _scanXFUtf32Strings(QVector<XBinary::MS_RECORD> *pListResult, XBinary::_MEMORY_MAP *pMemoryMap, const char *pBuffer, qint64 nBufferOffset,
                                qint64 nBufferSize, qint64 nEmitStart, qint64 nEmitEnd, qint32 nMinChars, qint32 nMaxChars, qint32 nLimit, bool bIsBigEndian)
{
    QVector<XBinary::MS_RECORD> listAllLanes;

    for (qint64 nLane = 0; nLane < 4; nLane++) {
        QVector<XBinary::MS_RECORD> listLaneRecords;
        qint64 i = nLane;

        while (i + 3 < nBufferSize) {
            _XFStringRun run = _countXFUtf32Run(pBuffer + i, nBufferSize - i, nMaxChars, bIsBigEndian);

            if (run.nRunBytes == 0) {
                i += 4;
                continue;
            }

            qint64 nStringOffset = nBufferOffset + i;

            if ((run.nRecordChars >= nMinChars) && (nStringOffset >= nEmitStart) && (nStringOffset < nEmitEnd)) {
                _appendXFStringRecord(&listLaneRecords, pMemoryMap, nStringOffset, run.nRecordBytes, XBinary::VT_UTF32,
                                      (quint16)(bIsBigEndian ? XBinary::ENDIAN_BIG : XBinary::ENDIAN_LITTLE), 4);
                _trimXFStringRecords(&listLaneRecords, nLimit);
            }

            i += run.nRunBytes;
        }

        _finalizeBestMSRecords(&listLaneRecords, nLimit);
        listAllLanes += listLaneRecords;
        _finalizeBestMSRecords(&listAllLanes, nLimit);
    }

    *pListResult += listAllLanes;
    _finalizeBestMSRecords(pListResult, nLimit);
}

QVector<XBinary::MS_RECORD> XBinary::multiSearch_strings(_MEMORY_MAP *pMemoryMap, qint64 nOffset, qint64 nSize, const XFSS_OPTIONS &ssOptions, PDSTRUCT *pPdStruct)
{
    QVector<XBinary::MS_RECORD> listResult;

    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    OFFSETSIZE osRegion = convertOffsetAndSize(nOffset, nSize);

    nOffset = osRegion.nOffset;
    nSize = osRegion.nSize;

    if ((nOffset == -1) || (nSize <= 0) || !isPdStructNotCanceled(pPdStruct)) {
        return listResult;
    }

    _MEMORY_MAP memoryMap = {};

    if (!pMemoryMap) {
        memoryMap = getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);
        pMemoryMap = &memoryMap;
    }

    qint32 nMinChars = ssOptions.nMinLenght;
    qint32 nMaxChars = ssOptions.nMaxLenght;
    qint32 nLimit = ssOptions.nLimit;

    if (nMinChars <= 0) {
        nMinChars = 1;
    }

    if (nMaxChars <= 0) {
        nMaxChars = 128;
    }

    if (nMaxChars < nMinChars) {
        nMaxChars = nMinChars;
    }

    if (nLimit <= 0) {
        nLimit = 0x7FFFFFFF;
    }

    // Keep one extra candidate internally so exact-N results can be
    // distinguished from real truncation after per-encoding/lane trimming.
    const qint32 nScanLimit = (nLimit < (std::numeric_limits<qint32>::max)()) ? (nLimit + 1) : nLimit;

    const bool bSearchAnsi = ssOptions.bANSI;
    const bool bSearchUtf8 = ssOptions.bUTF8;
    const bool bSearchUtf16 = ssOptions.bUTF16;
    const bool bSearchUtf32 = ssOptions.bUTF32;

    if (!(bSearchAnsi || bSearchUtf8 || bSearchUtf16 || bSearchUtf32)) {
        return listResult;
    }

    const qint32 nRequestedBufferSize = getBufferSize(pPdStruct);

    if (nRequestedBufferSize <= 0) {
        return listResult;
    }

    const qint32 nBufferSize = qBound((qint32)0x10000, nRequestedBufferSize, (qint32)0x100000);
    qint64 nLookBehind = 0;

    if (bSearchAnsi) {
        nLookBehind = qMax(nLookBehind, (qint64)1);
    }

    if (bSearchUtf8) {
        nLookBehind = qMax(nLookBehind, (qint64)4);
    }

    if (bSearchUtf16) {
        nLookBehind = qMax(nLookBehind, (qint64)2);
    }

    if (bSearchUtf32) {
        nLookBehind = qMax(nLookBehind, (qint64)4);
    }

    qint64 nLookAhead = qMin((qint64)nMaxChars * 4 + 4, (qint64)0x40000);

    if (nLookAhead < 0) {
        nLookAhead = 0x10000;
    }

    if (nSize > (std::numeric_limits<qint64>::max)() - nOffset) {
        return listResult;
    }

    const qint64 nAllocationSize = (qint64)nBufferSize + nLookAhead + nLookBehind;
    char *pBuffer = new (std::nothrow) char[(size_t)nAllocationSize];

    if (!pBuffer) {
        return listResult;
    }

    bool bReadError = false;
    bool bLimitReached = false;
    qint32 nFreeIndex = XBinary::reservePdStructRecord(pPdStruct, nSize);

    qint64 nCurrentOffset = nOffset;
    qint64 nRemainingSize = nSize;
    const qint64 nEndOffset = nOffset + nSize;

    while ((nRemainingSize > 0) && isPdStructNotCanceled(pPdStruct)) {
        qint64 nMainSize = qMin((qint64)nBufferSize, nRemainingSize);
        qint64 nReadOffset = qMax(nOffset, nCurrentOffset - nLookBehind);
        qint64 nPrefixSize = nCurrentOffset - nReadOffset;
        const qint64 nMainEndOffset = nCurrentOffset + nMainSize;
        qint64 nReadSize = nPrefixSize + nMainSize + qMin(nLookAhead, nEndOffset - nMainEndOffset);

        if (read_array_process(nReadOffset, pBuffer, nReadSize, pPdStruct) != nReadSize) {
            bReadError = true;
            break;
        }

        qint64 nEmitStart = nCurrentOffset;
        qint64 nEmitEnd = nMainEndOffset;

        QVector<MS_RECORD> listChunkRecords;

        const auto mergeEncodingRecords = [&](QVector<MS_RECORD> *pEncodingRecords) {
            listChunkRecords += *pEncodingRecords;
            bLimitReached = (listChunkRecords.size() > nLimit) || bLimitReached;
            _finalizeBestMSRecords(&listChunkRecords, nLimit);
        };

        if (bSearchAnsi) {
            QVector<MS_RECORD> listEncodingRecords;
            _scanXFByteStrings(&listEncodingRecords, pMemoryMap, pBuffer, nReadOffset, nReadSize, nEmitStart, nEmitEnd, nMinChars, nMaxChars, nScanLimit, VT_A,
                               ssOptions.nCodepage);
            mergeEncodingRecords(&listEncodingRecords);
        }

        if (bSearchUtf8) {
            QVector<MS_RECORD> listEncodingRecords;
            _scanXFUtf8Strings(&listEncodingRecords, pMemoryMap, pBuffer, nReadOffset, nReadSize, nEmitStart, nEmitEnd, nMinChars, nMaxChars, nScanLimit);
            mergeEncodingRecords(&listEncodingRecords);
        }

        const auto scanUtf16Encoding = [&](bool bIsBigEndian) {
            QVector<MS_RECORD> listEncodingRecords;
            _scanXFUtf16Strings(&listEncodingRecords, pMemoryMap, pBuffer, nReadOffset, nReadSize, nEmitStart, nEmitEnd, nMinChars, nMaxChars, nScanLimit,
                                bIsBigEndian);
            mergeEncodingRecords(&listEncodingRecords);
        };

        if (bSearchUtf16) {
            if (ssOptions.endian == ENDIAN_BIG) {
                scanUtf16Encoding(true);
            } else if (ssOptions.endian == ENDIAN_LITTLE) {
                scanUtf16Encoding(false);
            } else {
                scanUtf16Encoding(false);
                scanUtf16Encoding(true);
            }
        }

        const auto scanUtf32Encoding = [&](bool bIsBigEndian) {
            QVector<MS_RECORD> listEncodingRecords;
            _scanXFUtf32Strings(&listEncodingRecords, pMemoryMap, pBuffer, nReadOffset, nReadSize, nEmitStart, nEmitEnd, nMinChars, nMaxChars, nScanLimit,
                                bIsBigEndian);
            mergeEncodingRecords(&listEncodingRecords);
        };

        if (bSearchUtf32) {
            if (ssOptions.endian == ENDIAN_BIG) {
                scanUtf32Encoding(true);
            } else if (ssOptions.endian == ENDIAN_LITTLE) {
                scanUtf32Encoding(false);
            } else {
                scanUtf32Encoding(false);
                scanUtf32Encoding(true);
            }
        }

        listResult += listChunkRecords;
        bLimitReached = (listResult.size() > nLimit) || bLimitReached;
        _finalizeBestMSRecords(&listResult, nLimit);

        nCurrentOffset += nMainSize;
        nRemainingSize -= nMainSize;

        XBinary::setPdStructCurrent(pPdStruct, nFreeIndex, nCurrentOffset - nOffset);
    }

    delete[] pBuffer;

    if (bReadError) {
        setPdStructErrorString(pPdStruct, tr("Read error"));
    } else if (bLimitReached) {
        setPdStructErrorString(pPdStruct, QString("%1: %2").arg(tr("Maximum")).arg(QString::number(nLimit)));
    }

    _finalizeBestMSRecords(&listResult, nLimit);

    XBinary::setPdStructFinished(pPdStruct, nFreeIndex);

    if (bReadError || !isPdStructNotCanceled(pPdStruct)) {
        listResult.clear();
    }

    return listResult;
}
