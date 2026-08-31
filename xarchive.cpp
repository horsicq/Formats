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
#include "xarchive.h"
#include "xdecompress.h"
#include "Algos/xkwajlzssdecoder.h"
#include "Algos/xkwajlzhdecoder.h"
#include "Algos/xppmddecoder.h"
#include "Algos/xcoktellzdecoder.h"
#include "Algos/xwinzipjpegdecoder.h"
#include "Algos/xwavpackdecoder.h"
#include "Algos/xamigalzxdecoder.h"
#include "Algos/xmi10decoder.h"
#include "Algos/xftcompdecoder.h"
#include "Algos/xdndecoder.h"
#include "Algos/xsqzdecoder.h"
#include "Algos/xflsdecoder.h"
#include "Algos/xpakdecoder.h"
#include "Algos/xssmdecoder.h"

#include <algorithm>
#include <limits>
#include <memory>
#include <new>
#include <cstdio>
#include <QBuffer>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QDebug>
#include <QSaveFile>
#include <QSet>
#include <QTemporaryFile>
#include <QUuid>

#ifdef Q_OS_WIN
#include <io.h>
#include <windows.h>
#elif defined(Q_OS_UNIX)
#include <sys/stat.h>
#endif

namespace {
class ArchiveProgressAlive {
public:
    ArchiveProgressAlive(XBinary::PDSTRUCT *const &pPdStruct, const XBinary::PDSTRUCTLIFETIME &progressLifetime)
        : m_pPdStruct(pPdStruct), m_progressLifetime(progressLifetime)
    {
    }

    bool operator()() const
    {
        return !m_pPdStruct || XBinary::isPdStructLifetimeAlive(m_progressLifetime);
    }

private:
    XBinary::PDSTRUCT *const &m_pPdStruct;
    const XBinary::PDSTRUCTLIFETIME &m_progressLifetime;
};

class ArchiveContextAlive {
public:
    ArchiveContextAlive(const QPointer<QIODevice> &guardedSource, const QPointer<QIODevice> &guardedDestination, XBinary::PDSTRUCT *const &pPdStruct,
                        const XBinary::PDSTRUCTLIFETIME &progressLifetime)
        : m_guardedSource(guardedSource),
          m_guardedDestination(guardedDestination),
          m_pPdStruct(pPdStruct),
          m_progressLifetime(progressLifetime)
    {
    }

    bool operator()() const
    {
        return m_guardedSource && m_guardedDestination && (!m_pPdStruct || XBinary::isPdStructLifetimeAlive(m_progressLifetime));
    }

private:
    const QPointer<QIODevice> &m_guardedSource;
    const QPointer<QIODevice> &m_guardedDestination;
    XBinary::PDSTRUCT *const &m_pPdStruct;
    const XBinary::PDSTRUCTLIFETIME &m_progressLifetime;
};

class ArchiveDateTimeAppender {
public:
    ArchiveDateTimeAppender(QVector<XBinary::XMETADATA_STRUCT> &listResult, const XBinary::ARCHIVERECORD &archiveRecord, const QString &sRecordName)
        : m_listResult(listResult), m_archiveRecord(archiveRecord), m_sRecordName(sRecordName)
    {
    }

    void operator()(XBinary::FPART_PROP property, XBinary::XMETADATA_ID id, const QString &sLabel) const
    {
        const QVariant varValue = m_archiveRecord.mapProperties.value(property);
        if (!varValue.canConvert<QDateTime>()) {
            return;
        }

        const QDateTime dateTime = varValue.toDateTime();
        if (!dateTime.isValid()) {
            return;
        }

        XBinary::XMETADATA_STRUCT record = {};
        record.nOffset = m_archiveRecord.nStreamOffset;
        record.nSize = m_archiveRecord.nStreamSize;
        record.nAddress = (XADDR)-1;
        record.id = id;
        record.sName = QString("%1: %2").arg(m_sRecordName, sLabel);
        record.varValue = dateTime;
        m_listResult.append(record);
    }

private:
    QVector<XBinary::XMETADATA_STRUCT> &m_listResult;
    const XBinary::ARCHIVERECORD &m_archiveRecord;
    const QString &m_sRecordName;
};

struct ArchiveRarProgressBridge {
    XBinary::PDSTRUCT *pOriginal;
    XBinary::PDSTRUCTLIFETIME lifetime;
};

static void archiveRarProgressCallback(void *pUserData, XBinary::PDSTRUCT *pLocalProgress)
{
    ArchiveRarProgressBridge *pBridge = static_cast<ArchiveRarProgressBridge *>(pUserData);
    if (!pBridge || !pLocalProgress) return;
    if (!XBinary::isPdStructLifetimeAlive(pBridge->lifetime) || !XBinary::isPdStructNotCanceled(pBridge->pOriginal)) {
        XBinary::setPdStructStopped(pLocalProgress);
    }
}

class ArchiveTransactionError {
public:
    explicit ArchiveTransactionError(const XBinary::UNPACK_FOLDER_TRANSACTION &transaction) : m_transaction(transaction)
    {
    }

    QString operator()() const
    {
        QString sError = m_transaction.errorString();
        if (sError.isEmpty()) {
            sError = XArchive::tr("Extraction transaction failed");
        }
        const QString sRecoveryPath = m_transaction.recoveryPath();
        if (!sRecoveryPath.isEmpty()) {
            sError += QString(". %1: %2").arg(XArchive::tr("Recovery path"), sRecoveryPath);
        }
        return sError;
    }

private:
    const XBinary::UNPACK_FOLDER_TRANSACTION &m_transaction;
};

class ArchiveTransactionErrorReporter {
public:
    ArchiveTransactionErrorReporter(const ArchiveTransactionError &transactionError, const ArchiveProgressAlive &isProgressAlive,
                                    XBinary::PDSTRUCT *const &pPdStruct, QPointer<XArchive> &guardedArchive)
        : m_transactionError(transactionError), m_isProgressAlive(isProgressAlive), m_pPdStruct(pPdStruct), m_guardedArchive(guardedArchive)
    {
    }

    void operator()() const
    {
        const QString sError = m_transactionError();
        if (m_isProgressAlive()) {
            XBinary::setPdStructErrorString(m_pPdStruct, sError);
        }
        if (m_guardedArchive) {
            emit m_guardedArchive->errorMessage(sError);
        }
    }

private:
    const ArchiveTransactionError &m_transactionError;
    const ArchiveProgressAlive &m_isProgressAlive;
    XBinary::PDSTRUCT *const &m_pPdStruct;
    QPointer<XArchive> &m_guardedArchive;
};

class ArchiveTransactionRollback {
public:
    ArchiveTransactionRollback(XBinary::UNPACK_FOLDER_TRANSACTION &transaction, const ArchiveTransactionErrorReporter &reportTransactionError)
        : m_transaction(transaction), m_reportTransactionError(reportTransactionError)
    {
    }

    void operator()() const
    {
        if (!m_transaction.rollback()) {
            m_reportTransactionError();
        }
    }

private:
    XBinary::UNPACK_FOLDER_TRANSACTION &m_transaction;
    const ArchiveTransactionErrorReporter &m_reportTransactionError;
};

static QString archiveNormalizedFilePath(QFile *pFile)
{
    QPointer<QFile> guardedFile(pFile);
    if (!guardedFile) return QString();
    const QString sFileName = guardedFile->fileName();
    if (!guardedFile || sFileName.isEmpty()) return QString();

    const QFileInfo fileInfo(sFileName);
    QString sPath = fileInfo.canonicalFilePath();
    if (sPath.isEmpty()) sPath = QDir::cleanPath(fileInfo.absoluteFilePath());
    sPath = QDir::fromNativeSeparators(sPath);
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
    sPath = sPath.toCaseFolded();
#endif
    return sPath;
}

static QByteArray archiveFilePhysicalIdentity(QFile *pFile)
{
    QByteArray result;
    QPointer<QFile> guardedFile(pFile);
    if (!guardedFile) return result;
    const qintptr nFileHandle = guardedFile->handle();
    if (!guardedFile || (nFileHandle < 0)) return result;

#ifdef Q_OS_WIN
    const intptr_t nHandle = _get_osfhandle((int)nFileHandle);
    if (nHandle == -1) return result;
    BY_HANDLE_FILE_INFORMATION fileInformation = {};
    if (!GetFileInformationByHandle(reinterpret_cast<HANDLE>(nHandle), &fileInformation)) {
        return result;
    }
    result = QByteArray::number((qulonglong)fileInformation.dwVolumeSerialNumber, 16) + ':' + QByteArray::number((qulonglong)fileInformation.nFileIndexHigh, 16) + ':' +
             QByteArray::number((qulonglong)fileInformation.nFileIndexLow, 16);
#elif defined(Q_OS_UNIX)
    struct stat status = {};
    if (fstat((int)nFileHandle, &status) != 0) return result;
    result = QByteArray::number((qulonglong)status.st_dev, 16) + ':' + QByteArray::number((qulonglong)status.st_ino, 16);
#endif
    return result;
}

static QByteArray archiveFileMutationIdentity(QFile *pFile)
{
    QByteArray result;
    QPointer<QFile> guardedFile(pFile);
    if (!guardedFile) return result;
    const qintptr nFileHandle = guardedFile->handle();
    if (!guardedFile || (nFileHandle < 0)) return result;

#ifdef Q_OS_WIN
    const intptr_t nHandle = _get_osfhandle((int)nFileHandle);
    if (nHandle == -1) return result;
    FILE_BASIC_INFO basicInformation = {};
    FILE_STANDARD_INFO standardInformation = {};
    if (!GetFileInformationByHandleEx(reinterpret_cast<HANDLE>(nHandle), FileBasicInfo, &basicInformation, sizeof(basicInformation)) ||
        !GetFileInformationByHandleEx(reinterpret_cast<HANDLE>(nHandle), FileStandardInfo, &standardInformation, sizeof(standardInformation))) {
        return result;
    }
    // ChangeTime is deliberately excluded: NTFS bumps it for metadata-only
    // events a mere reader triggers or cannot prevent (the deferred
    // last-access-time update after any read, AV/indexer touches), so a
    // session validated against it dies nondeterministically on a file
    // nobody modified. Reads never move LastWriteTime or EndOfFile.
    result = QByteArray::number((qlonglong)basicInformation.LastWriteTime.QuadPart, 16) + ':' + QByteArray::number((qlonglong)standardInformation.EndOfFile.QuadPart, 16);
#elif defined(Q_OS_UNIX)
    struct stat status = {};
    if (fstat((int)nFileHandle, &status) != 0) return result;
#if defined(Q_OS_MAC)
    result = QByteArray::number((qlonglong)status.st_mtimespec.tv_sec, 16) + ':' + QByteArray::number((qlonglong)status.st_mtimespec.tv_nsec, 16) + ':' +
             QByteArray::number((qlonglong)status.st_ctimespec.tv_sec, 16) + ':' + QByteArray::number((qlonglong)status.st_ctimespec.tv_nsec, 16) + ':' +
             QByteArray::number((qlonglong)status.st_size, 16);
#else
    result = QByteArray::number((qlonglong)status.st_mtim.tv_sec, 16) + ':' + QByteArray::number((qlonglong)status.st_mtim.tv_nsec, 16) + ':' +
             QByteArray::number((qlonglong)status.st_ctim.tv_sec, 16) + ':' + QByteArray::number((qlonglong)status.st_ctim.tv_nsec, 16) + ':' +
             QByteArray::number((qlonglong)status.st_size, 16);
#endif
#endif
    return result;
}

static qint64 archiveReadWithBoundedProgress(QIODevice *pDevice, char *pBuffer, qint64 nSize);

class ArchiveBoundedReadDevice : public QIODevice {
public:
    ArchiveBoundedReadDevice(QIODevice *pSource, qint64 nLimit)
        : m_pSource(pSource), m_nLimit(nLimit), m_nRead(0), m_bSourceSeekable(false), m_nSourceStart(-1), m_bError(false)
    {
        if (m_pSource) {
            const bool bSequential = m_pSource->isSequential();
            if (m_pSource && !bSequential) {
                const qint64 nPosition = m_pSource->pos();
                if (m_pSource && (nPosition >= 0)) {
                    m_bSourceSeekable = true;
                    m_nSourceStart = nPosition;
                }
            }
        }
    }

    bool isSequential() const override
    {
        return true;
    }
    qint64 consumed() const
    {
        return m_nRead;
    }
    bool hasError() const
    {
        return m_bError;
    }

protected:
    qint64 readData(char *pData, qint64 nMaximumSize) override
    {
        if (!m_pSource || (nMaximumSize < 0) || ((nMaximumSize > 0) && !pData) || (m_nRead < 0) || (m_nRead > m_nLimit)) {
            m_bError = true;
            return -1;
        }

        const qint64 nRemaining = m_nLimit - m_nRead;
        if ((nMaximumSize == 0) || (nRemaining == 0)) {
            return 0;
        }

        const qint64 nMax = (std::numeric_limits<qint64>::max)();
        if (!m_pSource || (m_bSourceSeekable && ((m_nSourceStart < 0) || (m_nRead > nMax - m_nSourceStart)))) {
            m_bError = true;
            return -1;
        }
        if (m_bSourceSeekable) {
            const bool bSeeked = m_pSource->seek(m_nSourceStart + m_nRead);
            if (!m_pSource || !bSeeked) {
                m_bError = true;
                return -1;
            }
        }

        const qint64 nRequest = (std::min)(nMaximumSize, nRemaining);
        const qint64 nResult = archiveReadWithBoundedProgress(m_pSource, pData, nRequest);
        if ((nResult < 0) || (nResult > nRequest)) {
            m_bError = true;
            return -1;
        }
        if (nResult == 0) {
            m_bError = true;  // EOF before the declared compressed extent.
            return 0;
        }

        m_nRead += nResult;
        return nResult;
    }

    qint64 writeData(const char *, qint64) override
    {
        return -1;
    }

private:
    QPointer<QIODevice> m_pSource;
    qint64 m_nLimit;
    qint64 m_nRead;
    bool m_bSourceSeekable;
    qint64 m_nSourceStart;
    bool m_bError;
};

class ArchiveWindowWriteDevice : public QIODevice {
public:
    ArchiveWindowWriteDevice(QIODevice *pDestination, qint64 nOffset, qint64 nSize)
        : m_pDestination(pDestination), m_nOffset(nOffset), m_nSize(nSize), m_nProduced(0), m_nWritten(0), m_nDestinationStart(-1), m_bError(false)
    {
        if (m_pDestination) {
            const bool bSequential = m_pDestination->isSequential();
            if (m_pDestination && !bSequential) {
                const qint64 nPosition = m_pDestination->pos();
                if (m_pDestination && (nPosition >= 0)) {
                    m_nDestinationStart = nPosition;
                }
            }
        }
    }

    bool isSequential() const override
    {
        return true;
    }
    qint64 produced() const
    {
        return m_nProduced;
    }
    qint64 written() const
    {
        return m_nWritten;
    }
    bool hasError() const
    {
        return m_bError;
    }

protected:
    qint64 readData(char *, qint64) override
    {
        return -1;
    }

    qint64 writeData(const char *pData, qint64 nSize) override
    {
        const qint64 nMax = (std::numeric_limits<qint64>::max)();
        if (!m_pDestination || !pData || (nSize < 0) || (m_nProduced > (nMax - nSize))) {
            m_bError = true;
            return -1;
        }

        const qint64 nChunkStart = m_nProduced;
        const qint64 nChunkEnd = nChunkStart + nSize;
        const qint64 nWindowEnd = (m_nSize == -1) ? nMax : m_nOffset + m_nSize;
        const qint64 nWriteStart = (std::max)(nChunkStart, m_nOffset);
        const qint64 nWriteEnd = (std::min)(nChunkEnd, nWindowEnd);

        if (nWriteEnd > nWriteStart) {
            const qint64 nSkip = nWriteStart - nChunkStart;
            const qint64 nWriteSize = nWriteEnd - nWriteStart;
            qint64 nDone = 0;
            while (nDone < nWriteSize) {
                if (!m_pDestination) {
                    m_bError = true;
                    return -1;
                }
                const bool bSequential = m_pDestination->isSequential();
                if (!m_pDestination) {
                    m_bError = true;
                    return -1;
                }
                if (!bSequential) {
                    const qint64 nPositionMax = (std::numeric_limits<qint64>::max)();
                    if ((m_nDestinationStart < 0) || (m_nWritten > nPositionMax - m_nDestinationStart) || (nDone > nPositionMax - m_nDestinationStart - m_nWritten)) {
                        m_bError = true;
                        return -1;
                    }
                    const bool bSeeked = m_pDestination->seek(m_nDestinationStart + m_nWritten + nDone);
                    if (!m_pDestination || !bSeeked) {
                        m_bError = true;
                        return -1;
                    }
                }
                const qint64 nResult = m_pDestination->write(pData + nSkip + nDone, nWriteSize - nDone);
                if (!m_pDestination || (nResult <= 0) || (nResult > (nWriteSize - nDone))) {
                    m_bError = true;
                    return -1;
                }
                nDone += nResult;
            }
            if (!m_pDestination) {
                m_bError = true;
                return -1;
            }
            const bool bSequential = m_pDestination->isSequential();
            if (!m_pDestination) {
                m_bError = true;
                return -1;
            }
            if (!bSequential && ((m_nWritten > nMax - m_nDestinationStart) || (nDone > nMax - m_nDestinationStart - m_nWritten))) {
                // writeData() may synchronously re-enter code that moves the
                // destination even when it accepted the complete request.
                // Restore the final absolute continuation point before the
                // wrapper reports success to its caller.
                m_bError = true;
                return -1;
            }
            if (!bSequential) {
                const bool bSeeked = m_pDestination->seek(m_nDestinationStart + m_nWritten + nDone);
                if (!m_pDestination || !bSeeked) {
                    m_bError = true;
                    return -1;
                }
            }
            m_nWritten += nDone;
        }

        m_nProduced = nChunkEnd;
        return nSize;
    }

private:
    QPointer<QIODevice> m_pDestination;
    qint64 m_nOffset;
    qint64 m_nSize;
    qint64 m_nProduced;
    qint64 m_nWritten;
    qint64 m_nDestinationStart;
    bool m_bError;
};

static qint64 archiveReadWithBoundedProgress(QIODevice *pDevice, char *pBuffer, qint64 nSize)
{
    if (!pDevice || (nSize < 0) || ((nSize > 0) && !pBuffer)) return -1;

    QPointer<QIODevice> guardedDevice(pDevice);
    if (!guardedDevice) return -1;
    const bool bSequential = guardedDevice->isSequential();
    if (!guardedDevice) return -1;
    const bool bSeekable = !bSequential;
    const qint64 nStart = bSeekable ? guardedDevice->pos() : -1;
    const qint64 nMax = (std::numeric_limits<qint64>::max)();
    if (!guardedDevice || (bSeekable && (nStart < 0))) return -1;

    for (qint32 i = 0; i < 3; i++) {
        if (!guardedDevice) return -1;
        if (bSeekable) {
            const bool bSeeked = guardedDevice->seek(nStart);
            if (!guardedDevice || !bSeeked) return -1;
        }
        const qint64 nRead = guardedDevice->read(pBuffer, nSize);
        if (!guardedDevice) return -1;
        if ((nRead < 0) || (nRead > nSize)) return -1;
        bool bAtEnd = false;
        if ((nRead == 0) && (nSize != 0)) {
            bAtEnd = guardedDevice->atEnd();
            if (!guardedDevice) return -1;
        }
        if ((nRead != 0) || (nSize == 0) || bAtEnd) {
            if (bSeekable) {
                if (nRead > nMax - nStart) return -1;
                const bool bSeeked = guardedDevice->seek(nStart + nRead);
                if (!guardedDevice || !bSeeked) return -1;
            }
            return nRead;
        }
        if (i != 2) {
            guardedDevice->waitForReadyRead(10);
            if (!guardedDevice) return -1;
        }
    }

    if (!guardedDevice) return -1;
    const bool bAtEnd = guardedDevice->atEnd();
    if (!guardedDevice || !bAtEnd) return -1;
    if (bSeekable) {
        const bool bSeeked = guardedDevice->seek(nStart);
        if (!guardedDevice || !bSeeked) return -1;
    }
    return 0;
}

static bool archiveWriteAll(QIODevice *pDevice, const char *pData, qint64 nSize, XBinary::PDSTRUCT *pPdStruct)
{
    if (!pDevice || (nSize < 0) || ((nSize > 0) && !pData) || !XBinary::isPdStructNotCanceled(pPdStruct)) return false;

    QPointer<QIODevice> guardedDevice(pDevice);
    const XBinary::PDSTRUCTLIFETIME progressLifetime = pPdStruct ? XBinary::retainPdStructLifetime(pPdStruct) : XBinary::PDSTRUCTLIFETIME();
    const ArchiveProgressAlive isProgressAlive(pPdStruct, progressLifetime);
    if (!guardedDevice) return false;
    const bool bSequential = guardedDevice->isSequential();
    if (!guardedDevice || !isProgressAlive()) return false;
    const bool bSeekable = !bSequential;
    const qint64 nStart = bSeekable ? guardedDevice->pos() : -1;
    const qint64 nMax = (std::numeric_limits<qint64>::max)();
    if (!guardedDevice || !isProgressAlive() || (bSeekable && (nStart < 0))) return false;

    qint64 nWrittenTotal = 0;
    while ((nWrittenTotal < nSize) && isProgressAlive() && XBinary::isPdStructNotCanceled(pPdStruct)) {
        if (!guardedDevice) return false;
        if (bSeekable) {
            if (nWrittenTotal > nMax - nStart) return false;
            const bool bSeeked = guardedDevice->seek(nStart + nWrittenTotal);
            if (!guardedDevice || !isProgressAlive() || !bSeeked) return false;
        }
        const qint64 nWritten = guardedDevice->write(pData + nWrittenTotal, nSize - nWrittenTotal);
        if (!guardedDevice || !isProgressAlive()) return false;
        if ((nWritten <= 0) || (nWritten > (nSize - nWrittenTotal))) return false;
        nWrittenTotal += nWritten;
    }

    if (!guardedDevice || (nWrittenTotal != nSize) || !isProgressAlive() || !XBinary::isPdStructNotCanceled(pPdStruct)) return false;
    if (bSeekable) {
        if (nSize > nMax - nStart) return false;
        const bool bSeeked = guardedDevice->seek(nStart + nSize);
        if (!guardedDevice || !isProgressAlive() || !bSeeked) return false;
    }
    return true;
}

static XBinary::ARCHIVERECORD archiveRecordFromLegacy(const XArchive::RECORD &record)
{
    XBinary::ARCHIVERECORD result = {};
    result.nStreamOffset = record.nDataOffset;
    result.nStreamSize = record.nDataSize;
    result.mapProperties = record.mapProperties;
    return result;
}

// Normalizes an archive record path for extraction.
//
// Only genuinely dangerous shapes are refused: ".." traversal, and anything
// XBinary::fixFileName() would have to alter (control characters, NTFS ADS
// colons, trailing dots/spaces, Windows device aliases).  Shapes that are
// merely non-canonical are normalized away rather than rejected, because they
// are entirely ordinary: GNU tar stores "." and "./name", cpio and rpm store
// absolute paths such as "/home/user/file", and DOS-era archivers store
// "C:\name".  Refusing those loses ordinary data; 7-Zip, bsdtar and unzip all
// strip the prefix and extract relative to the destination.
//
// *pNormalizedPath may come back empty, meaning the record names the output
// root itself.  Callers must skip such records without treating it as failure.
static bool archiveGetSafeRelativePath(const QString &sPath, QString *pNormalizedPath)
{
    if (!pNormalizedPath) return false;

    QString sNormalized = QDir::fromNativeSeparators(sPath);

    // Drop a drive-letter prefix ("C:" / "C:/") and any leading separators.
    if ((sNormalized.size() >= 2) && sNormalized.at(0).isLetter() && (sNormalized.at(1) == QLatin1Char(':'))) {
        sNormalized = sNormalized.mid(2);
    }
    while (sNormalized.startsWith(QLatin1Char('/'))) {
        sNormalized.remove(0, 1);
    }

    QStringList listSafe;
    const QStringList listParts = sNormalized.split(QLatin1Char('/'), Qt::KeepEmptyParts);
    for (const QString &sPart : listParts) {
        // Empty components come from duplicated separators; "." is a no-op.
        if (sPart.isEmpty() || (sPart == QLatin1String("."))) continue;
        // Traversal is never normalized away -- it is refused.
        if (sPart == QLatin1String("..")) return false;
        listSafe.append(sPart);
    }

    sNormalized = listSafe.join(QLatin1Char('/')).normalized(QString::NormalizationForm_C);

    // fixFileName preserves safe path components and changes every portable
    // filesystem hazard: traversal components, control characters, NTFS ADS
    // colons, trailing dots/spaces, and Windows device aliases.
    if (!sNormalized.isEmpty() && (XBinary::fixFileName(sNormalized) != sNormalized)) return false;

    *pNormalizedPath = sNormalized;
    return true;
}

static bool archivePathHasUnsafeLink(const QString &sCanonicalRoot, const QString &sRelativePath)
{
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
    const Qt::CaseSensitivity pathCaseSensitivity = Qt::CaseInsensitive;
#else
    const Qt::CaseSensitivity pathCaseSensitivity = Qt::CaseSensitive;
#endif

    QString sRoot = QDir::fromNativeSeparators(QDir::cleanPath(sCanonicalRoot));
    QString sRootPrefix = sRoot;
    if (!sRootPrefix.endsWith(QLatin1Char('/'))) sRootPrefix.append(QLatin1Char('/'));

    QString sCurrent = sRoot;
    const QStringList listParts = sRelativePath.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    for (const QString &sPart : listParts) {
        sCurrent = QDir(sCurrent).filePath(sPart);
        QFileInfo fileInfo(sCurrent);

        // isSymLink() also detects a broken link, for which exists() is false.
        if (fileInfo.isSymLink()) return true;

#ifdef Q_OS_WIN
        // Qt's isSymLink() does not reliably flag every NTFS reparse point
        // (junctions and mount points in particular), so check the reparse
        // attribute directly. Only an existing component that IS a reparse
        // point is unsafe; a not-yet-
        // created component reads INVALID_FILE_ATTRIBUTES and is fine.
        {
            const QString sNative = XBinary::winExtendedNativePath(fileInfo.absoluteFilePath());
            const DWORD nReparseAttr = GetFileAttributesW(reinterpret_cast<LPCWSTR>(sNative.utf16()));
            if ((nReparseAttr != INVALID_FILE_ATTRIBUTES) && (nReparseAttr & FILE_ATTRIBUTE_REPARSE_POINT)) {
                return true;
            }
        }
#endif

        if (fileInfo.exists()) {
            const QString sCanonical = QDir::fromNativeSeparators(fileInfo.canonicalFilePath());
            if (sCanonical.isEmpty() || ((sCanonical.compare(sRoot, pathCaseSensitivity) != 0) && !sCanonical.startsWith(sRootPrefix, pathCaseSensitivity))) {
                return true;
            }
        }
    }

    return false;
}

class ArchiveSourceSessionRegistry : public QObject {
public:
    struct SESSION {
        XArchive::SOURCE_DEVICE_SNAPSHOT snapshot;
        void *pContext;
        const XBinary::UNPACK_STATE *pOwnerState;
        bool bFinalized;
    };

    ArchiveSourceSessionRegistry() : QObject(nullptr)
    {
    }

    QHash<QByteArray, SESSION> mapSessions;
};

static bool archiveRestoreChainPositions(const XArchive::SOURCE_DEVICE_SNAPSHOT &snapshot, const QList<qint64> &listPositions)
{
    bool bRestored = true;
    // Restore the outer view first and the physical root last.  Seeking a
    // SubDevice also seeks its backing device, so this order preserves the
    // independent logical positions of the complete chain.
    for (qint32 i = 0; i < snapshot.listChain.size(); ++i) {
        QPointer<QIODevice> guardedDevice(snapshot.listChain.at(i).pDevice);
        if (!guardedDevice) {
            bRestored = false;
            continue;
        }
        const bool bSeeked = guardedDevice->seek(listPositions.at(i));
        if (!guardedDevice || !bSeeked) bRestored = false;
    }
    return bRestored;
}

static bool archiveFingerprintSource(XArchive *pOwner, const XArchive::SOURCE_DEVICE_SNAPSHOT &snapshot, QByteArray *pFingerprint, XBinary::PDSTRUCT *pPdStruct)
{
    if (pFingerprint) pFingerprint->clear();
    if (!pOwner || !pFingerprint || !snapshot.pSourceDevice || snapshot.listChain.isEmpty() || (snapshot.listChain.constFirst().nSize < 0)) {
        return false;
    }

    // Establish the owner guard before the first caller-controlled device
    // callback.  Constructing a QPointer from pOwner after isOpen(), pos(), or
    // another virtual call has deleted the archive would itself dereference a
    // dangling QObject address.
    QPointer<XArchive> guardedOwner(pOwner);
    const XBinary::PDSTRUCTLIFETIME progressLifetime = pPdStruct ? XBinary::retainPdStructLifetime(pPdStruct) : XBinary::PDSTRUCTLIFETIME();
    const ArchiveProgressAlive isProgressAlive(pPdStruct, progressLifetime);

    QList<qint64> listPositions;
    listPositions.reserve(snapshot.listChain.size());
    for (const XArchive::SOURCE_DEVICE_CHAIN_ITEM &item : snapshot.listChain) {
        QPointer<QIODevice> guardedDevice(item.pDevice);
        if (!guardedOwner || !guardedDevice || !isProgressAlive()) return false;
        const bool bOpen = guardedDevice->isOpen();
        if (!guardedOwner || !guardedDevice || !isProgressAlive() || !bOpen) return false;
        const bool bReadable = guardedDevice->isReadable();
        if (!guardedOwner || !guardedDevice || !isProgressAlive() || !bReadable) return false;
        const bool bSequential = guardedDevice->isSequential();
        if (!guardedOwner || !guardedDevice || !isProgressAlive() || bSequential) return false;
        const QIODevice::OpenMode openMode = guardedDevice->openMode();
        if (!guardedOwner || !guardedDevice || !isProgressAlive() || openMode.testFlag(QIODevice::Text)) return false;
        const qint64 nPosition = guardedDevice->pos();
        if (!guardedOwner || !guardedDevice || !isProgressAlive() || (nPosition < 0)) return false;
        listPositions.append(nPosition);
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    QByteArray baBuffer(0x100000, 0);
    QPointer<QIODevice> guardedSource(snapshot.pSourceDevice.data());
    qint64 nOffset = 0;
    qint64 nRemaining = snapshot.listChain.constFirst().nSize;
    bool bResult = true;

    while (bResult && (nRemaining > 0) && isProgressAlive() && XBinary::isPdStructNotCanceled(pPdStruct)) {
        const qint64 nChunkSize = qMin(nRemaining, (qint64)baBuffer.size());
        if (!guardedSource || !guardedOwner) {
            bResult = false;
            break;
        }
        const qint64 nRead = guardedOwner->safeReadData(guardedSource.data(), nOffset, baBuffer.data(), nChunkSize, pPdStruct);
        if (!guardedSource || !guardedOwner || !isProgressAlive() || (nRead != nChunkSize)) {
            bResult = false;
            break;
        }
        hash.addData(baBuffer.constData(), (int)nRead);
        nOffset += nRead;
        nRemaining -= nRead;
    }

    const bool bRestored = archiveRestoreChainPositions(snapshot, listPositions);
    if (!bResult || !bRestored || !guardedSource || !guardedOwner || !isProgressAlive() || !XBinary::isPdStructNotCanceled(pPdStruct)) return false;
    *pFingerprint = hash.result();
    return true;
}
}  // namespace

QObject *XArchive::getArchiveSourceSessionRegistry(bool bCreate) const
{
    if (!m_pArchiveSourceSessionRegistry && bCreate) {
        m_pArchiveSourceSessionRegistry = new (std::nothrow) ArchiveSourceSessionRegistry;
    }
    return m_pArchiveSourceSessionRegistry;
}

#if defined(_MSC_VER)
#if _MSC_VER > 1800                                   // TODO Check !!!
#pragma comment(lib, "legacy_stdio_definitions.lib")  // bzip2.lib(compress.obj) __imp__fprintf

FILE _iob[] = {*stdin, *stdout, *stderr};  // bzip2.lib(compress.obj) _iob_func

extern "C" FILE *__cdecl __iob_func(void)
{
    return _iob;
}
#endif
#endif

static void *SzAlloc(ISzAllocPtr, size_t size)
{
    return malloc(size);
}

static void SzFree(ISzAllocPtr, void *address)
{
    free(address);
}

static ISzAlloc g_Alloc = {SzAlloc, SzFree};

bool XArchive::captureSourceDeviceSnapshot(QIODevice *pDevice, SOURCE_DEVICE_SNAPSHOT *pSnapshot)
{
    if (pSnapshot) *pSnapshot = SOURCE_DEVICE_SNAPSHOT();
    QPointer<XArchive> guardedArchive(this);
    const quint64 nDeviceGeneration = getDeviceGeneration();
    if (!guardedArchive || !pDevice || (pDevice != guardedArchive->getDevice()) || !pSnapshot) {
        return false;
    }

    SOURCE_DEVICE_SNAPSHOT snapshot = {};
    snapshot.pSourceDevice = pDevice;
    snapshot.rootKind = SOURCE_DEVICE_ROOT_UNKNOWN;
    snapshot.nRootSize = -1;
    snapshot.nBufferBackingIdentity = 0;
    snapshot.bContentFingerprintRequired = false;
    snapshot.nOwnerDeviceGeneration = nDeviceGeneration;

    QSet<QIODevice *> setVisited;
    QIODevice *pCurrent = pDevice;
    while (pCurrent) {
        QPointer<QIODevice> guardedCurrent(pCurrent);
        if (!guardedCurrent || setVisited.contains(pCurrent)) return false;
        const bool bOpen = guardedCurrent->isOpen();
        if (!guardedArchive || !guardedCurrent || !bOpen) return false;
        const bool bReadable = guardedCurrent->isReadable();
        if (!guardedArchive || !guardedCurrent || !bReadable) return false;
        const QIODevice::OpenMode openMode = guardedCurrent->openMode();
        if (!guardedArchive || !guardedCurrent || openMode.testFlag(QIODevice::Text)) return false;
        const bool bSequential = guardedCurrent->isSequential();
        if (!guardedArchive || !guardedCurrent || bSequential) return false;
        const qint64 nCurrentSize = guardedCurrent->size();
        if (!guardedArchive || !guardedCurrent || (nCurrentSize < 0)) return false;
        const bool bStillSequential = guardedCurrent->isSequential();
        if (!guardedArchive || !guardedCurrent || bStillSequential) return false;
        const bool bStillOpen = guardedCurrent->isOpen();
        if (!guardedArchive || !guardedCurrent || !bStillOpen) return false;
        const bool bStillReadable = guardedCurrent->isReadable();
        if (!guardedArchive || !guardedCurrent || !bStillReadable) return false;
        const QIODevice::OpenMode finalOpenMode = guardedCurrent->openMode();
        if (!guardedArchive || !guardedCurrent || finalOpenMode.testFlag(QIODevice::Text)) return false;
        setVisited.insert(guardedCurrent.data());

        SOURCE_DEVICE_CHAIN_ITEM item = {};
        item.pDevice = guardedCurrent;
        item.nSize = nCurrentSize;

        SubDevice *pSubDevice = dynamic_cast<SubDevice *>(guardedCurrent.data());
        item.bIsSubDevice = (pSubDevice != nullptr);
        if (pSubDevice) {
            item.nInitLocation = pSubDevice->getInitLocation();
            if (!guardedArchive || !guardedCurrent) return false;
            snapshot.listChain.append(item);
            pCurrent = pSubDevice->getOrigDevice();
            if (!guardedArchive || !guardedCurrent || !pCurrent) return false;
        } else {
            snapshot.listChain.append(item);
            snapshot.pRootDevice = guardedCurrent;
            break;
        }
    }

    QIODevice *pRootDevice = snapshot.pRootDevice.data();
    if (!pRootDevice || snapshot.listChain.isEmpty()) return false;
    // XBinary caches the logical source size when setDevice() binds a device.
    // Do not combine a freshly observed snapshot with stale bounds from an
    // earlier binding generation; rebinding the same device refreshes both.
    if (!guardedArchive || (snapshot.listChain.constFirst().nSize != guardedArchive->getSize())) return false;
    snapshot.nRootSize = snapshot.listChain.constLast().nSize;

    if (QBuffer *pBuffer = dynamic_cast<QBuffer *>(pRootDevice)) {
        QPointer<QBuffer> guardedBuffer(pBuffer);
        if (!guardedArchive || !guardedBuffer) return false;
        snapshot.rootKind = SOURCE_DEVICE_ROOT_BUFFER;
        snapshot.nBufferBackingIdentity = reinterpret_cast<quintptr>(&guardedBuffer->buffer());
        if (!guardedArchive || !guardedBuffer) return false;
        // Keep the cheap implicit-shared value here. bindUnpackSource() turns
        // the retained baseline into an independent byte copy exactly once;
        // subsequent candidates can then be compared byte-for-byte without
        // repeatedly applying a cryptographic hash to a large memory buffer.
        snapshot.baBufferSnapshot = guardedBuffer->buffer();
        if (!guardedArchive || !guardedBuffer) return false;
        snapshot.bContentFingerprintRequired = false;
    } else if (QFile *pFile = dynamic_cast<QFile *>(pRootDevice)) {
        QPointer<QFile> guardedFile(pFile);
        if (!guardedArchive || !guardedFile) return false;
        snapshot.rootKind = SOURCE_DEVICE_ROOT_FILE;
        snapshot.sFilePath = archiveNormalizedFilePath(guardedFile.data());
        if (!guardedArchive || !guardedFile) return false;
        snapshot.baFilePhysicalIdentity = archiveFilePhysicalIdentity(guardedFile.data());
        if (!guardedArchive || !guardedFile) return false;
        snapshot.baFileMutationIdentity = archiveFileMutationIdentity(guardedFile.data());
        if (!guardedArchive || !guardedFile) return false;
        // QFile::open(fd, ...) has no path.  Preserve that supported use case
        // whenever the platform can identify the opened file physically.
        if ((snapshot.sFilePath.isEmpty() && snapshot.baFilePhysicalIdentity.isEmpty()) || snapshot.baFileMutationIdentity.isEmpty()) {
            return false;
        }
    } else {
        // Generic seekable devices have no backing-store identity or mutation
        // generation.  Their complete logical contents are therefore hashed
        // by bindUnpackSource() and on every retained-state validation.
        snapshot.rootKind = SOURCE_DEVICE_ROOT_GENERIC;
        snapshot.bContentFingerprintRequired = true;
    }

    if (!guardedArchive || (guardedArchive->getDeviceGeneration() != nDeviceGeneration) || (guardedArchive->getDevice() != pDevice)) {
        return false;
    }
    for (const SOURCE_DEVICE_CHAIN_ITEM &item : snapshot.listChain) {
        QPointer<QIODevice> guardedChainDevice(item.pDevice);
        if (!guardedChainDevice) return false;
        const bool bOpen = guardedChainDevice->isOpen();
        if (!guardedArchive || !guardedChainDevice || !bOpen) return false;
        const bool bReadable = guardedChainDevice->isReadable();
        if (!guardedArchive || !guardedChainDevice || !bReadable) return false;
        const QIODevice::OpenMode openMode = guardedChainDevice->openMode();
        if (!guardedArchive || !guardedChainDevice || openMode.testFlag(QIODevice::Text)) return false;
    }

    *pSnapshot = snapshot;
    return true;
}

static bool archiveSnapshotStructureMatches(const XArchive::SOURCE_DEVICE_SNAPSHOT &snapshot, const XArchive::SOURCE_DEVICE_SNAPSHOT &candidate)
{
    if ((candidate.pRootDevice.data() != snapshot.pRootDevice.data()) || (candidate.listChain.size() != snapshot.listChain.size()) ||
        (candidate.rootKind != snapshot.rootKind) || (candidate.nRootSize != snapshot.nRootSize) ||
        (candidate.nBufferBackingIdentity != snapshot.nBufferBackingIdentity) || (candidate.baBufferSnapshot != snapshot.baBufferSnapshot) ||
        (candidate.sFilePath != snapshot.sFilePath) || (candidate.baFilePhysicalIdentity != snapshot.baFilePhysicalIdentity) ||
        (candidate.baFileMutationIdentity != snapshot.baFileMutationIdentity) || (candidate.bContentFingerprintRequired != snapshot.bContentFingerprintRequired) ||
        (candidate.nOwnerDeviceGeneration != snapshot.nOwnerDeviceGeneration)) {
        return false;
    }

    for (qint32 i = 0; i < snapshot.listChain.size(); ++i) {
        const XArchive::SOURCE_DEVICE_CHAIN_ITEM &expected = snapshot.listChain.at(i);
        const XArchive::SOURCE_DEVICE_CHAIN_ITEM &actual = candidate.listChain.at(i);
        if (!expected.pDevice || (actual.pDevice.data() != expected.pDevice.data()) || (actual.bIsSubDevice != expected.bIsSubDevice) ||
            (actual.nInitLocation != expected.nInitLocation) || (actual.nSize != expected.nSize)) {
            return false;
        }
    }

    return true;
}

bool XArchive::isSourceDeviceSnapshotCurrent(const SOURCE_DEVICE_SNAPSHOT &snapshot, QIODevice *pCurrentDevice, PDSTRUCT *pPdStruct)
{
    QPointer<XArchive> guardedArchive(this);
    // Check the guarded QPointer before inspecting pCurrentDevice: XBinary used
    // to retain the same now-dangling address after an external QObject died.
    if (!guardedArchive || (snapshot.nOwnerDeviceGeneration != guardedArchive->getDeviceGeneration()) || !snapshot.pSourceDevice ||
        (pCurrentDevice != snapshot.pSourceDevice.data())) {
        return false;
    }

    SOURCE_DEVICE_SNAPSHOT current = {};
    if (!guardedArchive || !guardedArchive->captureSourceDeviceSnapshot(pCurrentDevice, &current) || !guardedArchive ||
        !archiveSnapshotStructureMatches(snapshot, current)) {
        return false;
    }

    if (snapshot.bContentFingerprintRequired) {
        QByteArray baCurrentFingerprint;
        if (!archiveFingerprintSource(guardedArchive.data(), current, &baCurrentFingerprint, pPdStruct) || !guardedArchive ||
            (baCurrentFingerprint != snapshot.baContentFingerprint)) {
            return false;
        }

        // Hashing calls untrusted QIODevice virtual methods.  Re-capture the
        // complete chain afterwards so a callback cannot move/resize/rebind a
        // SubDevice after its bytes have already been authenticated.
        SOURCE_DEVICE_SNAPSHOT afterFingerprint = {};
        if (!guardedArchive || !guardedArchive->captureSourceDeviceSnapshot(pCurrentDevice, &afterFingerprint) || !guardedArchive ||
            !archiveSnapshotStructureMatches(snapshot, afterFingerprint)) {
            return false;
        }
    }

    return true;
}

bool XArchive::bindUnpackSource(UNPACK_STATE *pState, PDSTRUCT *pPdStruct)
{
    if (!pState || !XBinary::isPdStructNotCanceled(pPdStruct)) return false;
    QPointer<XArchive> guardedArchive(this);

    if ((pState->pContext || !pState->baUnpackSourceToken.isEmpty()) && !guardedArchive->ownsUnpackSource(pState)) return false;

    releaseUnpackSource(pState);
    SOURCE_DEVICE_SNAPSHOT snapshot = {};
    if (!guardedArchive || !guardedArchive->captureSourceDeviceSnapshot(guardedArchive->getDevice(), &snapshot) || !guardedArchive) {
        return false;
    }

    if (snapshot.rootKind == SOURCE_DEVICE_ROOT_BUFFER) {
        // Establish a non-typed lifetime guard before doing any type query.
        // qobject_cast() calls virtual metaObject() and is therefore not safe
        // on an unguarded caller-controlled QBuffer subclass. dynamic_cast has
        // no device callback; create the typed guard immediately afterwards.
        QPointer<QIODevice> guardedRoot(snapshot.pRootDevice.data());
        if (!guardedArchive || !guardedRoot) return false;
        QBuffer *pBuffer = dynamic_cast<QBuffer *>(guardedRoot.data());
        QPointer<QBuffer> guardedBuffer(pBuffer);
        if (!guardedArchive || !guardedRoot || !guardedBuffer) return false;

        // QByteArray's ordinary copy is implicit-shared and a writable pointer
        // acquired before initialization could mutate both aliases.  Retain an
        // independent baseline instead.  Candidate snapshots remain shallow,
        // so QByteArray equality performs an exact byte comparison against
        // this baseline while avoiding an allocation on every state check.
        QByteArray baIndependentBaseline;
        try {
            const QByteArray &baCurrent = guardedBuffer->buffer();
            baIndependentBaseline = QByteArray(baCurrent.constData(), baCurrent.size());
        } catch (const std::bad_alloc &) {
            return false;
        }
        if (!guardedArchive || !guardedBuffer || (guardedArchive->getDeviceGeneration() != snapshot.nOwnerDeviceGeneration) ||
            (guardedArchive->getDevice() != snapshot.pSourceDevice.data())) {
            return false;
        }
        snapshot.baBufferSnapshot.swap(baIndependentBaseline);
    }

    // Backings without a trustworthy mutation generation get a complete
    // content fingerprint. QFile uses its physical identity plus kernel
    // last-write/change metadata. QBuffer uses the independent exact-byte
    // baseline above, including for same-backing raw-pointer writes.
    QByteArray baFingerprint;
    if (snapshot.bContentFingerprintRequired &&
        (!archiveFingerprintSource(guardedArchive.data(), snapshot, &baFingerprint, pPdStruct) || !guardedArchive || baFingerprint.isEmpty())) {
        return false;
    }
    snapshot.baContentFingerprint = baFingerprint;

    // Assign the expected digest before validation. Generic devices are hashed
    // once more so no read callback can change them between baseline capture
    // and publication. QBuffer validation compares the independent byte
    // baseline, and QFile validation is metadata-only.
    if (!guardedArchive->isSourceDeviceSnapshotCurrent(snapshot, guardedArchive->getDevice(), pPdStruct) || !guardedArchive ||
        !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    ArchiveSourceSessionRegistry *pRegistry = static_cast<ArchiveSourceSessionRegistry *>(guardedArchive->getArchiveSourceSessionRegistry(true));
    if (!pRegistry) return false;

    QByteArray baToken;
    do {
        baToken = QUuid::createUuid().toRfc4122();
    } while (baToken.isEmpty() || pRegistry->mapSessions.contains(baToken));

    ArchiveSourceSessionRegistry::SESSION session = {};
    session.snapshot = snapshot;
    session.pContext = nullptr;
    session.pOwnerState = pState;
    session.bFinalized = false;
    pRegistry->mapSessions.insert(baToken, session);
    pState->baUnpackSourceToken = baToken;
    return true;
}

bool XArchive::isUnpackSourceCurrent(const UNPACK_STATE *pState, PDSTRUCT *pPdStruct)
{
    if (!pState || !XBinary::isPdStructNotCanceled(pPdStruct)) return false;
    const QByteArray baToken = pState->baUnpackSourceToken;
    const void *pExpectedContext = pState->pContext;
    QPointer<XArchive> guardedArchive(this);
    if (baToken.isEmpty()) return false;

    ArchiveSourceSessionRegistry *pRegistry = static_cast<ArchiveSourceSessionRegistry *>(guardedArchive->getArchiveSourceSessionRegistry(false));
    if (!pRegistry) return false;
    const QHash<QByteArray, ArchiveSourceSessionRegistry::SESSION>::const_iterator it = pRegistry->mapSessions.constFind(baToken);
    if (it == pRegistry->mapSessions.constEnd() || (it->pOwnerState != pState) || (it->bFinalized && (it->pContext != pExpectedContext))) return false;

    // Never retain a QHash value reference across source callbacks.  A direct
    // caller need not hold an operation guard, and a re-entrant finish can
    // remove the registry entry while snapshot validation is in progress.
    const ArchiveSourceSessionRegistry::SESSION session = *it;
    QPointer<ArchiveSourceSessionRegistry> guardedRegistry(pRegistry);
    const bool bSnapshotCurrent = guardedArchive && guardedArchive->isSourceDeviceSnapshotCurrent(session.snapshot, guardedArchive->getDevice(), pPdStruct);
    if (!bSnapshotCurrent || !guardedArchive || !guardedRegistry || !pState || (pState->baUnpackSourceToken != baToken) || (pState->pContext != pExpectedContext) ||
        !XBinary::isPdStructNotCanceled(pPdStruct))
        return false;

    const QHash<QByteArray, ArchiveSourceSessionRegistry::SESSION>::const_iterator currentIt = guardedRegistry->mapSessions.constFind(baToken);
    return (currentIt != guardedRegistry->mapSessions.constEnd()) && (currentIt->pOwnerState == pState) && (currentIt->bFinalized == session.bFinalized) &&
           (currentIt->pContext == session.pContext) && (!currentIt->bFinalized || (currentIt->pContext == pExpectedContext));
}

bool XArchive::getBoundUnpackSourceSnapshot(const UNPACK_STATE *pState, SOURCE_DEVICE_SNAPSHOT *pSnapshot) const
{
    if (pSnapshot) *pSnapshot = SOURCE_DEVICE_SNAPSHOT();
    if (!pState || !pSnapshot || pState->baUnpackSourceToken.isEmpty()) {
        return false;
    }

    ArchiveSourceSessionRegistry *pRegistry = static_cast<ArchiveSourceSessionRegistry *>(getArchiveSourceSessionRegistry(false));
    if (!pRegistry) return false;
    const QHash<QByteArray, ArchiveSourceSessionRegistry::SESSION>::const_iterator it = pRegistry->mapSessions.constFind(pState->baUnpackSourceToken);
    if ((it == pRegistry->mapSessions.constEnd()) || (it->pOwnerState != pState) || (it->bFinalized && (it->pContext != pState->pContext))) {
        return false;
    }

    *pSnapshot = it->snapshot;
    return true;
}

bool XArchive::validateAndFinalizeUnpackSource(UNPACK_STATE *pState, PDSTRUCT *pPdStruct)
{
    QPointer<XArchive> guardedArchive(this);
    if (!guardedArchive || !guardedArchive->isUnpackSourceCurrent(pState, pPdStruct) || !guardedArchive) return false;
    const QByteArray baToken = pState->baUnpackSourceToken;
    ArchiveSourceSessionRegistry *pRegistry = static_cast<ArchiveSourceSessionRegistry *>(guardedArchive->getArchiveSourceSessionRegistry(false));
    if (!pRegistry) return false;
    QHash<QByteArray, ArchiveSourceSessionRegistry::SESSION>::iterator it = pRegistry->mapSessions.find(baToken);
    if (it == pRegistry->mapSessions.end() || (it->pOwnerState != pState) || it->bFinalized) return false;

    // isUnpackSourceCurrent() above performs the complete structural/content
    // validation after parsing.  No caller-controlled operation occurs before
    // the context identity is committed below, so a redundant second/third
    // full-file hash would add cost without closing another race boundary.
    it->pContext = pState->pContext;
    it->bFinalized = true;
    return true;
}

bool XArchive::registerUnpackContextCleanup(UNPACK_STATE *pState, void *pContext, UNPACK_GUARD_STATE::CONTEXT_DELETER pDeleter)
{
    if (!pState || !pContext || !pDeleter || (pState->pContext != pContext) || !m_pUnpackGuardState || !m_pUnpackGuardState->bOwnerAlive) {
        return false;
    }

    ArchiveSourceSessionRegistry *pRegistry = static_cast<ArchiveSourceSessionRegistry *>(getArchiveSourceSessionRegistry(false));
    if (!pRegistry || pState->baUnpackSourceToken.isEmpty()) return false;
    QHash<QByteArray, ArchiveSourceSessionRegistry::SESSION>::iterator sessionIt = pRegistry->mapSessions.find(pState->baUnpackSourceToken);
    if ((sessionIt == pRegistry->mapSessions.end()) || (sessionIt->pOwnerState != pState) || sessionIt->bFinalized) {
        return false;
    }

    for (const UNPACK_GUARD_STATE::CONTEXT_CLEANUP &cleanup : m_pUnpackGuardState->listContexts) {
        if (cleanup.pContext == pContext) {
            if (cleanup.pDeleter != pDeleter) return false;
            sessionIt->pContext = pContext;
            return true;
        }
    }

    UNPACK_GUARD_STATE::CONTEXT_CLEANUP cleanup = {};
    cleanup.pContext = pContext;
    cleanup.pDeleter = pDeleter;
    // Bound the deferred-cleanup list instead of guarding the append
    // allocation: sessions register one context each, so this stays tiny.
    if (m_pUnpackGuardState->listContexts.size() >= 0x10000) return false;
    m_pUnpackGuardState->listContexts.append(cleanup);
    // Retain the provisional context identity in the private session before
    // final source validation.  If validation fails while the owner survives,
    // releaseUnpackSource() can then unregister the deferred deleter even when
    // the format has already cleared the public state's pContext field.
    sessionIt->pContext = pContext;
    return true;
}

void XArchive::unregisterUnpackContextCleanup(void *pContext)
{
    if (!pContext || !m_pUnpackGuardState) return;
    for (qint32 i = m_pUnpackGuardState->listContexts.size() - 1; i >= 0; --i) {
        if (m_pUnpackGuardState->listContexts.at(i).pContext == pContext) {
            m_pUnpackGuardState->listContexts.removeAt(i);
        }
    }
}

bool XArchive::ownsUnpackSource(const UNPACK_STATE *pState)
{
    if (!pState || pState->baUnpackSourceToken.isEmpty()) return false;
    ArchiveSourceSessionRegistry *pRegistry = static_cast<ArchiveSourceSessionRegistry *>(getArchiveSourceSessionRegistry(false));
    if (!pRegistry) return false;
    const QHash<QByteArray, ArchiveSourceSessionRegistry::SESSION>::const_iterator it = pRegistry->mapSessions.constFind(pState->baUnpackSourceToken);
    return (it != pRegistry->mapSessions.constEnd()) && it->bFinalized && (it->pOwnerState == pState) && (it->pContext == pState->pContext);
}

void XArchive::releaseUnpackSource(UNPACK_STATE *pState)
{
    if (!pState) return;
    const QByteArray baToken = pState->baUnpackSourceToken;
    ArchiveSourceSessionRegistry *pRegistry = static_cast<ArchiveSourceSessionRegistry *>(getArchiveSourceSessionRegistry(false));
    if (pRegistry && !baToken.isEmpty()) {
        const QHash<QByteArray, ArchiveSourceSessionRegistry::SESSION>::const_iterator it = pRegistry->mapSessions.constFind(baToken);
        // A copied public state must not remove the real owner's session or
        // make its copied token look legitimately finalized/cleared.
        if ((it == pRegistry->mapSessions.constEnd()) || (it->pOwnerState != pState)) return;
        unregisterUnpackContextCleanup(it->pContext);
        pRegistry->mapSessions.remove(baToken);
    }
    pState->baUnpackSourceToken.clear();
}

bool XArchive::transferUnpackSourceOwnership(UNPACK_STATE *pFromState, UNPACK_STATE *pToState)
{
    if (!pFromState || !pToState || (pFromState == pToState) || pFromState->baUnpackSourceToken.isEmpty() || !pToState->baUnpackSourceToken.isEmpty() ||
        pToState->pContext) {
        return false;
    }

    ArchiveSourceSessionRegistry *pRegistry = static_cast<ArchiveSourceSessionRegistry *>(getArchiveSourceSessionRegistry(false));
    if (!pRegistry) return false;
    QHash<QByteArray, ArchiveSourceSessionRegistry::SESSION>::iterator it = pRegistry->mapSessions.find(pFromState->baUnpackSourceToken);
    if ((it == pRegistry->mapSessions.end()) || (it->pOwnerState != pFromState) || (it->bFinalized && (it->pContext != pFromState->pContext))) {
        return false;
    }

    // Preserve every streaming field while changing only the exact owner
    // address.  The destination precondition makes this a genuine move, not
    // authorization of a pre-populated public copy.
    UNPACK_STATE movedState = *pFromState;
    *pToState = movedState;
    it->pOwnerState = pToState;
    *pFromState = UNPACK_STATE();
    return true;
}

XArchive::XArchive(QIODevice *pDevice)
    : XBinary(pDevice),
      m_pArchiveSourceSessionRegistry(nullptr),
      m_pUnpackGuardState(new UNPACK_GUARD_STATE),
      m_bUnpackOperationInProgress(m_pUnpackGuardState, false),
      m_bNestedUnpackInfoAuthorized(m_pUnpackGuardState, true)
{
}

XArchive::~XArchive()
{
    if (m_pUnpackGuardState) {
        m_pUnpackGuardState->bOwnerAlive = false;
        if (!m_pUnpackGuardState->bOperationInProgress) {
            m_pUnpackGuardState->cleanupContexts();
        }
    }
    QObject *pRegistry = m_pArchiveSourceSessionRegistry;
    m_pArchiveSourceSessionRegistry = nullptr;
    delete pRegistry;
}

quint64 XArchive::getNumberOfRecords(PDSTRUCT *pPdStruct)
{
    QPointer<XArchive> guardedArchive(this);
    if (!guardedArchive) return 0;
    const quint64 nResult = guardedArchive->getNumberOfArchiveRecords(pPdStruct);
    return guardedArchive ? nResult : 0;
}

QList<XArchive::RECORD> XArchive::getRecords(qint32 nLimit, PDSTRUCT *pPdStruct)
{
    QList<RECORD> listResult;
    QPointer<XArchive> guardedArchive(this);

    // -1 is the only unbounded sentinel.  Avoid initializing an archive when
    // the requested result is necessarily empty or the limit is invalid.
    if (nLimit < -1 || nLimit == 0) {
        return listResult;
    }

    XBinary::PDSTRUCT pdStructEmpty = {};

    if (!pPdStruct) {
        pdStructEmpty = XBinary::createPdStruct();
        pPdStruct = &pdStructEmpty;
    }
    const PDSTRUCTLIFETIME progressLifetime = retainPdStructLifetime(pPdStruct);
    const ArchiveProgressAlive isProgressAlive(pPdStruct, progressLifetime);

    if (!isProgressAlive() || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return listResult;
    }

    QMap<UNPACK_PROP, QVariant> mapProperties;

    // Initialize unpacking state
    UNPACK_STATE state = {};

    if (!guardedArchive || !guardedArchive->initUnpack(&state, mapProperties, pPdStruct) || !guardedArchive || !isProgressAlive()) {
        return listResult;
    }

    const qint32 nNumberOfRecords = state.nNumberOfRecords;

    // Iterate through records using streaming API.  A successful initializer
    // must expose a sane cursor; otherwise calling infoCurrent() with a
    // negative index is undefined for most format implementations.
    qint32 nIndex = 0;
    bool bEnumerationValid = (state.nCurrentIndex == 0) && (nNumberOfRecords >= 0) && (state.nCurrentIndex <= nNumberOfRecords);

    while (bEnumerationValid && (state.nCurrentIndex < nNumberOfRecords) && isProgressAlive() && XBinary::isPdStructNotCanceled(pPdStruct)) {
        // Get current record info
        const qint32 nExpectedIndex = state.nCurrentIndex;
        ARCHIVERECORD archiveRecord = guardedArchive->infoCurrent(&state, pPdStruct);

        // An index-paired ARCHIVE_STREAM record deliberately carries no extent
        // (see XBinary::ARCHIVE_STREAM_NO_EXTENT); every other record must
        // carry a non-negative one measured on this archive's own device.
        qint32 nArchiveStreamIndex = -1;
        const bool bIsArchiveStreamRecord = XBinary::getArchiveStreamRecordIndex(archiveRecord, &nArchiveStreamIndex);

        // A callback or parser is allowed to cancel during infoCurrent().  Do
        // not publish the record that was being assembled when that happened.
        if (!guardedArchive || !isProgressAlive() || !XBinary::isPdStructNotCanceled(pPdStruct) || archiveRecord.mapProperties.isEmpty() ||
            !XBinary::isArchiveRecordExtentValid(archiveRecord) || (state.nCurrentIndex < 0) || (state.nNumberOfRecords < 0) || (state.nCurrentIndex != nExpectedIndex) ||
            (state.nNumberOfRecords != nNumberOfRecords) || (state.nCurrentIndex >= nNumberOfRecords)) {
            bEnumerationValid = false;
            break;
        }

        // A legacy RECORD is nothing but a raw (offset,size) pair, and every
        // consumer of one resolves it against getDevice().  An index-paired
        // ARCHIVE_STREAM record carries no device-relative coordinates and is
        // extracted through the archive session instead, so it is exempt.
        // Anything else must have been measured on the very device the legacy
        // path will read; if the session says otherwise the pairing cannot be
        // established and the enumeration fails cleanly.  Publishing such a
        // record instead would let a STORE-method member be satisfied by a raw
        // copy of unrelated container bytes at exactly the right length.
        //
        // The test is pure pointer identity, deliberately including the
        // both-null case.  getRecordStreamDevice()'s default implementation
        // *is* getDevice() (xbinary.cpp), so a format that keeps the default
        // contract answers with the very same pointer whatever that pointer
        // happens to be; two nulls are that identity for a device-less
        // archive, not a disagreement, and a consumer resolving offsets
        // against a null device reads nothing at all - there are no unrelated
        // container bytes for it to be handed.  Exactly one side being null,
        // on the other hand, can only come from an override that disagrees
        // with getDevice(): either the session disclaims the coordinates the
        // legacy path will nevertheless resolve against a real device (the
        // silent-corruption shape itself), or the record was measured on a
        // private buffer the legacy path cannot see.  Both stay rejected.
        if (!bIsArchiveStreamRecord) {
            QIODevice *pRecordDevice = guardedArchive->getRecordStreamDevice(&state);
            QIODevice *pPublicDevice = guardedArchive ? guardedArchive->getDevice() : nullptr;

            if (!guardedArchive || (pRecordDevice != pPublicDevice)) {
                XBinary::setPdStructErrorString(pPdStruct, tr("Archive record does not belong to the archive device"));
                bEnumerationValid = false;
                break;
            }
        }

        // Convert ARCHIVERECORD to legacy RECORD structure
        RECORD record = {};

        record.nDataOffset = archiveRecord.nStreamOffset;
        record.nDataSize = archiveRecord.nStreamSize;
        record.mapProperties = archiveRecord.mapProperties;
        record.spInfo.nUncompressedSize = archiveRecord.mapProperties.value(FPART_PROP_UNCOMPRESSEDSIZE).toLongLong();

        // Extract common properties from mapProperties
        if (archiveRecord.mapProperties.contains(FPART_PROP_ORIGINALNAME)) {
            record.spInfo.sRecordName = archiveRecord.mapProperties.value(FPART_PROP_ORIGINALNAME).toString();
        }

        if (archiveRecord.mapProperties.contains(FPART_PROP_HANDLEMETHOD)) {
            record.spInfo.compressMethod = (HANDLE_METHOD)archiveRecord.mapProperties.value(FPART_PROP_HANDLEMETHOD).toInt();
        } else {
            record.spInfo.compressMethod = HANDLE_METHOD_UNKNOWN;
        }
        record.spInfo.compressMethod2 = HANDLE_METHOD_UNKNOWN;

        if (archiveRecord.mapProperties.contains(FPART_PROP_RESULTCRC)) {
            record.spInfo.nCRC32 = archiveRecord.mapProperties.value(FPART_PROP_RESULTCRC).toUInt();
        }

        if (archiveRecord.mapProperties.contains(FPART_PROP_WINDOWSIZE)) {
            record.spInfo.nWindowSize = archiveRecord.mapProperties.value(FPART_PROP_WINDOWSIZE).toULongLong();
        }

        if (archiveRecord.mapProperties.contains(FPART_PROP_ISSOLID)) {
            record.spInfo.bIsSolid = archiveRecord.mapProperties.value(FPART_PROP_ISSOLID).toBool();
        }

        if (archiveRecord.mapProperties.contains(FPART_PROP_HEADER_OFFSET)) {
            record.nHeaderOffset = archiveRecord.mapProperties.value(FPART_PROP_HEADER_OFFSET).toLongLong();
        }

        if (archiveRecord.mapProperties.contains(FPART_PROP_HEADER_SIZE)) {
            record.nHeaderSize = archiveRecord.mapProperties.value(FPART_PROP_HEADER_SIZE).toLongLong();
        }

        if (archiveRecord.mapProperties.contains(FPART_PROP_OPTHEADER_OFFSET)) {
            record.nOptHeaderOffset = archiveRecord.mapProperties.value(FPART_PROP_OPTHEADER_OFFSET).toLongLong();
        }

        if (archiveRecord.mapProperties.contains(FPART_PROP_OPTHEADER_SIZE)) {
            record.nOptHeaderSize = archiveRecord.mapProperties.value(FPART_PROP_OPTHEADER_SIZE).toLongLong();
        }

        record.sUUID = generateUUID();
        if (archiveRecord.mapProperties.contains(FPART_PROP_HANDLEMETHOD2)) {
            record.spInfo.compressMethod2 = (HANDLE_METHOD)archiveRecord.mapProperties.value(FPART_PROP_HANDLEMETHOD2).toInt();
        }

        listResult.append(record);

        nIndex++;

        // Check limit
        if ((nLimit != -1) && (nIndex >= nLimit)) {
            break;
        }

        // Move to next record
        const qint32 nPreviousIndex = state.nCurrentIndex;
        const bool bMoved = guardedArchive->moveToNext(&state, pPdStruct);
        if (!guardedArchive || !isProgressAlive() || !XBinary::isPdStructNotCanceled(pPdStruct) || (state.nCurrentIndex < 0) || (state.nNumberOfRecords < 0) ||
            (state.nNumberOfRecords != nNumberOfRecords) || (state.nCurrentIndex > nNumberOfRecords)) {
            bEnumerationValid = false;
            break;
        }
        if (!bMoved) {
            // Returning false is normal after the last declared record.  It is
            // corruption when records are still outstanding, and a partial
            // prefix must not be published as a complete enumeration.
            if ((nPreviousIndex + 1) < nNumberOfRecords) {
                bEnumerationValid = false;
            } else if ((state.nCurrentIndex != nPreviousIndex) && (state.nCurrentIndex != nNumberOfRecords)) {
                bEnumerationValid = false;
            }
            break;
        }
        if ((state.nCurrentIndex != (nPreviousIndex + 1)) || (state.nCurrentIndex >= nNumberOfRecords)) {
            bEnumerationValid = false;
            break;
        }
    }

    // Clean up unpacking state
    // Cleanup must not inherit a canceled enumeration token.
    const bool bFinished = guardedArchive && guardedArchive->finishUnpack(&state, nullptr) && guardedArchive;
    if (!bEnumerationValid || !bFinished || !isProgressAlive() || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        listResult.clear();
    }

    return listResult;
}

bool XArchive::isResourcesPresent()
{
    return getNumberOfArchiveRecords(nullptr) > 0;
}

QVector<XBinary::XRESOURCE_STRUCT> XArchive::getResourceStructs()
{
    QVector<XRESOURCE_STRUCT> listResult;
    const QList<ARCHIVERECORD> listRecords = getArchiveRecords(-1, nullptr);
    const qint32 nNumberOfRecords = listRecords.count();

    listResult.reserve(nNumberOfRecords);

    for (qint32 i = 0; i < nNumberOfRecords; ++i) {
        const ARCHIVERECORD &archiveRecord = listRecords.at(i);

        XRESOURCE_STRUCT record = {};
        record.nOffset = archiveRecord.nStreamOffset;
        record.nSize = archiveRecord.nStreamSize;
        record.nAddress = offsetToAddress(archiveRecord.nStreamOffset);
        record.sName = archiveRecord.mapProperties.value(FPART_PROP_ORIGINALNAME).toString();
        record.nType = archiveRecord.mapProperties.value(FPART_PROP_FILETYPE).toUInt();
        record.nID = static_cast<quint32>(i);

        listResult.append(record);
    }

    return listResult;
}

QVector<XBinary::XMETADATA_STRUCT> XArchive::getMetadataStructs()
{
    QVector<XMETADATA_STRUCT> listResult;
    const QList<ARCHIVERECORD> listRecords = getArchiveRecords(-1, nullptr);

    for (qint32 i = 0; i < listRecords.count(); ++i) {
        const ARCHIVERECORD &archiveRecord = listRecords.at(i);
        QString sRecordName = archiveRecord.mapProperties.value(FPART_PROP_ORIGINALNAME).toString();
        if (sRecordName.isEmpty()) {
            sRecordName = QString("Record %1").arg(i);
        }

        const QString sUuid = archiveRecord.mapProperties.value(FPART_PROP_UUID).toString();
        if (!sUuid.isEmpty()) {
            XMETADATA_STRUCT record = {};
            record.nOffset = -1;
            record.nSize = 16;
            record.nAddress = (XADDR)-1;
            record.id = XMETADATA_ID_UUID;
            record.sName = QString("%1: UUID").arg(sRecordName);
            record.varValue = sUuid;
            listResult.append(record);
        }

        const ArchiveDateTimeAppender appendDateTime(listResult, archiveRecord, sRecordName);

        if (archiveRecord.mapProperties.contains(FPART_PROP_MTIME)) {
            appendDateTime(FPART_PROP_MTIME, XMETADATA_ID_MODIFICATED, QString("Modification time"));
        } else {
            appendDateTime(FPART_PROP_DATETIME, XMETADATA_ID_MODIFICATED, QString("Modification time"));
        }
        appendDateTime(FPART_PROP_CTIME, XMETADATA_ID_DATETIME_CREATED, QString("Creation time"));
        appendDateTime(FPART_PROP_ATIME, XMETADATA_ID_DATETIME_ACCESSED, QString("Access time"));
    }

    return listResult;
}

XArchive::COMPRESS_RESULT XArchive::_decompress(DECOMPRESSSTRUCT *pDecompressStruct, PDSTRUCT *pPdStruct)
{
    if (!pDecompressStruct) {
        return COMPRESS_RESULT_DATAERROR;
    }

    // Decoder/progress callbacks may destroy the caller-owned argument.  Keep
    // all work in an invocation-local value and publish only while the
    // retained progress owner is still alive.
    DECOMPRESSSTRUCT *pCallerDecompressStruct = pDecompressStruct;
    DECOMPRESSSTRUCT localDecompressStruct = *pDecompressStruct;
    pDecompressStruct = &localDecompressStruct;
    pCallerDecompressStruct->nOutSize = 0;
    pCallerDecompressStruct->nDecompressedWrote = 0;
    pCallerDecompressStruct->bLimit = false;

    // Result fields belong to this invocation even when argument validation
    // fails; never expose counters or limit state left by an earlier call.
    pDecompressStruct->nOutSize = 0;
    pDecompressStruct->nDecompressedWrote = 0;
    pDecompressStruct->bLimit = false;

    if (!pDecompressStruct->pSourceDevice || !pDecompressStruct->pDestDevice || (pDecompressStruct->nInSize < 0) || (pDecompressStruct->nDecompressedOffset < 0) ||
        (pDecompressStruct->nDecompressedLimit < -1) ||
        ((pDecompressStruct->nDecompressedLimit != -1) &&
         (pDecompressStruct->nDecompressedOffset > ((std::numeric_limits<qint64>::max)() - pDecompressStruct->nDecompressedLimit)))) {
        return COMPRESS_RESULT_DATAERROR;
    }

    const qint64 nWindowEnd = (pDecompressStruct->nDecompressedLimit == -1) ? (std::numeric_limits<qint64>::max)()
                                                                            : pDecompressStruct->nDecompressedOffset + pDecompressStruct->nDecompressedLimit;

    XBinary::PDSTRUCT pdStructEmpty = {};

    if (!pPdStruct) {
        pdStructEmpty = XBinary::createPdStruct();
        pPdStruct = &pdStructEmpty;
    }
    const PDSTRUCTLIFETIME progressLifetime = retainPdStructLifetime(pPdStruct);
    struct DecompressResultPublisher {
        DECOMPRESSSTRUCT *pCaller;
        const DECOMPRESSSTRUCT *pLocal;
        PDSTRUCTLIFETIME lifetime;
        ~DecompressResultPublisher()
        {
            if (!pCaller || !pLocal || !XBinary::isPdStructLifetimeAlive(lifetime)) return;
            pCaller->nInSize = pLocal->nInSize;
            pCaller->nOutSize = pLocal->nOutSize;
            pCaller->nDecompressedWrote = pLocal->nDecompressedWrote;
            pCaller->bLimit = pLocal->bLimit;
        }
    } resultPublisher = {pCallerDecompressStruct, pDecompressStruct, progressLifetime};
    QPointer<QIODevice> guardedSource(pDecompressStruct->pSourceDevice);
    QPointer<QIODevice> guardedDestination(pDecompressStruct->pDestDevice);
    if (!guardedSource || !guardedDestination || !isPdStructLifetimeAlive(progressLifetime)) {
        return COMPRESS_RESULT_DATAERROR;
    }
    qint64 nDefaultInputLimit = pDecompressStruct->nInSize;
    if (nDefaultInputLimit == 0) {
        nDefaultInputLimit = guardedSource->size();
        if (!guardedSource || !guardedDestination || !isPdStructLifetimeAlive(progressLifetime) || (nDefaultInputLimit < 0)) {
            return COMPRESS_RESULT_READERROR;
        }
    }

    COMPRESS_RESULT result = COMPRESS_RESULT_UNKNOWN;

    if (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_STORE) {
        XBinary::DATAPROCESS_STATE decompressState = {};
        decompressState.mapProperties.insert(XBinary::FPART_PROP_HANDLEMETHOD, HANDLE_METHOD_STORE);
        decompressState.mapProperties.insert(XBinary::FPART_PROP_UNCOMPRESSEDSIZE, pDecompressStruct->spInfo.nUncompressedSize);
        decompressState.pDeviceInput = pDecompressStruct->pSourceDevice;
        decompressState.pDeviceOutput = pDecompressStruct->pDestDevice;
        decompressState.nInputOffset = 0;
        decompressState.nInputLimit = nDefaultInputLimit;
        decompressState.nProcessedOffset = pDecompressStruct->nDecompressedOffset;
        decompressState.nProcessedLimit = pDecompressStruct->nDecompressedLimit;

        if (XStoreDecoder::decompress(&decompressState, pPdStruct)) {
            pDecompressStruct->nInSize = decompressState.nCountInput;
            pDecompressStruct->nOutSize = decompressState.nCountOutput;
            pDecompressStruct->bLimit = (pDecompressStruct->nDecompressedLimit != -1) && (decompressState.nCountOutput >= nWindowEnd);
            result = COMPRESS_RESULT_OK;
        } else {
            if (decompressState.bReadError) {
                result = COMPRESS_RESULT_READERROR;
            } else if (decompressState.bWriteError) {
                result = COMPRESS_RESULT_WRITEERROR;
            } else {
                result = COMPRESS_RESULT_DATAERROR;
            }
        }
    } else if (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_AMIGA_LZX) {
        XBinary::DATAPROCESS_STATE decompressState = {};
        decompressState.mapProperties.insert(XBinary::FPART_PROP_HANDLEMETHOD, HANDLE_METHOD_AMIGA_LZX);
        decompressState.mapProperties.insert(XBinary::FPART_PROP_UNCOMPRESSEDSIZE, pDecompressStruct->spInfo.nUncompressedSize);
        decompressState.pDeviceInput = pDecompressStruct->pSourceDevice;
        decompressState.pDeviceOutput = pDecompressStruct->pDestDevice;
        decompressState.nInputOffset = 0;
        decompressState.nInputLimit = nDefaultInputLimit;
        decompressState.nProcessedOffset = pDecompressStruct->nDecompressedOffset;
        decompressState.nProcessedLimit = pDecompressStruct->nDecompressedLimit;

        if (XAmigaLZXDecoder::decompress(&decompressState, pPdStruct)) {
            pDecompressStruct->nInSize = decompressState.nCountInput;
            pDecompressStruct->nOutSize = decompressState.nCountOutput;
            pDecompressStruct->bLimit = (pDecompressStruct->nDecompressedLimit != -1) && (decompressState.nCountOutput >= nWindowEnd);
            result = COMPRESS_RESULT_OK;
        } else if (decompressState.bReadError) {
            result = COMPRESS_RESULT_READERROR;
        } else if (decompressState.bWriteError) {
            result = COMPRESS_RESULT_WRITEERROR;
        } else {
            result = COMPRESS_RESULT_DATAERROR;
        }
    } else if (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_MI10) {
        XBinary::DATAPROCESS_STATE decompressState = {};
        decompressState.mapProperties.insert(XBinary::FPART_PROP_HANDLEMETHOD, HANDLE_METHOD_MI10);
        decompressState.mapProperties.insert(XBinary::FPART_PROP_UNCOMPRESSEDSIZE, pDecompressStruct->spInfo.nUncompressedSize);
        decompressState.pDeviceInput = pDecompressStruct->pSourceDevice;
        decompressState.pDeviceOutput = pDecompressStruct->pDestDevice;
        decompressState.nInputOffset = 0;
        decompressState.nInputLimit = nDefaultInputLimit;
        decompressState.nProcessedOffset = pDecompressStruct->nDecompressedOffset;
        decompressState.nProcessedLimit = pDecompressStruct->nDecompressedLimit;

        if (XMI10Decoder::decompress(&decompressState, pPdStruct)) {
            pDecompressStruct->nInSize = decompressState.nCountInput;
            pDecompressStruct->nOutSize = decompressState.nCountOutput;
            pDecompressStruct->bLimit = (pDecompressStruct->nDecompressedLimit != -1) && (decompressState.nCountOutput >= nWindowEnd);
            result = COMPRESS_RESULT_OK;
        } else if (decompressState.bReadError) {
            result = COMPRESS_RESULT_READERROR;
        } else if (decompressState.bWriteError) {
            result = COMPRESS_RESULT_WRITEERROR;
        } else {
            result = COMPRESS_RESULT_DATAERROR;
        }
    } else if ((pDecompressStruct->spInfo.compressMethod >= HANDLE_METHOD_SQZ1) &&
               (pDecompressStruct->spInfo.compressMethod <= HANDLE_METHOD_SQZ4)) {
        const HANDLE_METHOD handleMethod =
            pDecompressStruct->spInfo.compressMethod;
        const qint32 nMethod =
            static_cast<qint32>(handleMethod) -
            static_cast<qint32>(HANDLE_METHOD_SQZ1) + 1;
        XBinary::DATAPROCESS_STATE decompressState = {};
        decompressState.mapProperties.insert(XBinary::FPART_PROP_HANDLEMETHOD, handleMethod);
        decompressState.mapProperties.insert(XBinary::FPART_PROP_UNCOMPRESSEDSIZE, pDecompressStruct->spInfo.nUncompressedSize);
        decompressState.pDeviceInput = pDecompressStruct->pSourceDevice;
        decompressState.pDeviceOutput = pDecompressStruct->pDestDevice;
        decompressState.nInputOffset = 0;
        decompressState.nInputLimit = nDefaultInputLimit;
        decompressState.nProcessedOffset = pDecompressStruct->nDecompressedOffset;
        decompressState.nProcessedLimit = pDecompressStruct->nDecompressedLimit;

        if (XSQZDecoder::decompress(&decompressState, nMethod, pPdStruct)) {
            pDecompressStruct->nInSize = decompressState.nCountInput;
            pDecompressStruct->nOutSize = decompressState.nCountOutput;
            pDecompressStruct->bLimit = (pDecompressStruct->nDecompressedLimit != -1) && (decompressState.nCountOutput >= nWindowEnd);
            result = COMPRESS_RESULT_OK;
        } else if (decompressState.bReadError) {
            result = COMPRESS_RESULT_READERROR;
        } else if (decompressState.bWriteError) {
            result = COMPRESS_RESULT_WRITEERROR;
        } else {
            result = COMPRESS_RESULT_DATAERROR;
        }
    } else if (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_FTCOMP_FT19) {
        XBinary::DATAPROCESS_STATE decompressState = {};
        decompressState.mapProperties.insert(XBinary::FPART_PROP_HANDLEMETHOD, HANDLE_METHOD_FTCOMP_FT19);
        decompressState.mapProperties.insert(XBinary::FPART_PROP_UNCOMPRESSEDSIZE, pDecompressStruct->spInfo.nUncompressedSize);
        decompressState.pDeviceInput = pDecompressStruct->pSourceDevice;
        decompressState.pDeviceOutput = pDecompressStruct->pDestDevice;
        decompressState.nInputOffset = 0;
        decompressState.nInputLimit = nDefaultInputLimit;
        decompressState.nProcessedOffset = pDecompressStruct->nDecompressedOffset;
        decompressState.nProcessedLimit = pDecompressStruct->nDecompressedLimit;

        if (XFtcompDecoder::decompress(&decompressState, pPdStruct)) {
            pDecompressStruct->nInSize = decompressState.nCountInput;
            pDecompressStruct->nOutSize = decompressState.nCountOutput;
            pDecompressStruct->bLimit = (pDecompressStruct->nDecompressedLimit != -1) && (decompressState.nCountOutput >= nWindowEnd);
            result = COMPRESS_RESULT_OK;
        } else if (decompressState.bReadError) {
            result = COMPRESS_RESULT_READERROR;
        } else if (decompressState.bWriteError) {
            result = COMPRESS_RESULT_WRITEERROR;
        } else {
            result = COMPRESS_RESULT_DATAERROR;
        }
    } else if (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_DN_COMPRESSED) {
        XBinary::DATAPROCESS_STATE decompressState = {};
        decompressState.mapProperties.insert(XBinary::FPART_PROP_HANDLEMETHOD, HANDLE_METHOD_DN_COMPRESSED);
        decompressState.mapProperties.insert(XBinary::FPART_PROP_UNCOMPRESSEDSIZE, pDecompressStruct->spInfo.nUncompressedSize);
        decompressState.pDeviceInput = pDecompressStruct->pSourceDevice;
        decompressState.pDeviceOutput = pDecompressStruct->pDestDevice;
        decompressState.nInputOffset = 0;
        decompressState.nInputLimit = nDefaultInputLimit;
        decompressState.nProcessedOffset = pDecompressStruct->nDecompressedOffset;
        decompressState.nProcessedLimit = pDecompressStruct->nDecompressedLimit;

        if (XDNDecoder::decompress(&decompressState, pPdStruct)) {
            pDecompressStruct->nInSize = decompressState.nCountInput;
            pDecompressStruct->nOutSize = decompressState.nCountOutput;
            pDecompressStruct->bLimit = (pDecompressStruct->nDecompressedLimit != -1) && (decompressState.nCountOutput >= nWindowEnd);
            result = COMPRESS_RESULT_OK;
        } else if (decompressState.bReadError) {
            result = COMPRESS_RESULT_READERROR;
        } else if (decompressState.bWriteError) {
            result = COMPRESS_RESULT_WRITEERROR;
        } else {
            result = COMPRESS_RESULT_DATAERROR;
        }
    } else if (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_FLS_LZ) {
        XBinary::DATAPROCESS_STATE decompressState = {};
        decompressState.mapProperties.insert(XBinary::FPART_PROP_HANDLEMETHOD, HANDLE_METHOD_FLS_LZ);
        decompressState.mapProperties.insert(XBinary::FPART_PROP_UNCOMPRESSEDSIZE, pDecompressStruct->spInfo.nUncompressedSize);
        decompressState.pDeviceInput = pDecompressStruct->pSourceDevice;
        decompressState.pDeviceOutput = pDecompressStruct->pDestDevice;
        decompressState.nInputOffset = 0;
        decompressState.nInputLimit = nDefaultInputLimit;
        decompressState.nProcessedOffset = pDecompressStruct->nDecompressedOffset;
        decompressState.nProcessedLimit = pDecompressStruct->nDecompressedLimit;

        if (XFLSDecoder::decompress(&decompressState, pPdStruct)) {
            pDecompressStruct->nInSize = decompressState.nCountInput;
            pDecompressStruct->nOutSize = decompressState.nCountOutput;
            pDecompressStruct->bLimit = (pDecompressStruct->nDecompressedLimit != -1) && (decompressState.nCountOutput >= nWindowEnd);
            result = COMPRESS_RESULT_OK;
        } else if (decompressState.bReadError) {
            result = COMPRESS_RESULT_READERROR;
        } else if (decompressState.bWriteError) {
            result = COMPRESS_RESULT_WRITEERROR;
        } else {
            result = COMPRESS_RESULT_DATAERROR;
        }
    } else if ((pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_PAK_CRUSHED) ||
               (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_PAK_DISTILLED)) {
        const HANDLE_METHOD handleMethod = pDecompressStruct->spInfo.compressMethod;
        const qint32 nMethod = (handleMethod == HANDLE_METHOD_PAK_CRUSHED) ? 10 : 11;
        XBinary::DATAPROCESS_STATE decompressState = {};
        decompressState.mapProperties.insert(XBinary::FPART_PROP_HANDLEMETHOD, handleMethod);
        decompressState.mapProperties.insert(XBinary::FPART_PROP_UNCOMPRESSEDSIZE, pDecompressStruct->spInfo.nUncompressedSize);
        decompressState.pDeviceInput = pDecompressStruct->pSourceDevice;
        decompressState.pDeviceOutput = pDecompressStruct->pDestDevice;
        decompressState.nInputOffset = 0;
        decompressState.nInputLimit = nDefaultInputLimit;
        decompressState.nProcessedOffset = pDecompressStruct->nDecompressedOffset;
        decompressState.nProcessedLimit = pDecompressStruct->nDecompressedLimit;

        if (XPakDecoder::decompress(&decompressState, nMethod, pPdStruct)) {
            pDecompressStruct->nInSize = decompressState.nCountInput;
            pDecompressStruct->nOutSize = decompressState.nCountOutput;
            pDecompressStruct->bLimit = (pDecompressStruct->nDecompressedLimit != -1) && (decompressState.nCountOutput >= nWindowEnd);
            result = COMPRESS_RESULT_OK;
        } else if (decompressState.bReadError) {
            result = COMPRESS_RESULT_READERROR;
        } else if (decompressState.bWriteError) {
            result = COMPRESS_RESULT_WRITEERROR;
        } else {
            result = COMPRESS_RESULT_DATAERROR;
        }
    } else if ((pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_SSM_PICTOOLS) ||
               (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_SSM_PICTOOLS5)) {
        const HANDLE_METHOD handleMethod = pDecompressStruct->spInfo.compressMethod;
        const qint32 nMethod = (handleMethod == HANDLE_METHOD_SSM_PICTOOLS) ? 3 : 5;
        XBinary::DATAPROCESS_STATE decompressState = {};
        decompressState.mapProperties.insert(XBinary::FPART_PROP_HANDLEMETHOD, handleMethod);
        decompressState.mapProperties.insert(XBinary::FPART_PROP_UNCOMPRESSEDSIZE, pDecompressStruct->spInfo.nUncompressedSize);
        decompressState.pDeviceInput = pDecompressStruct->pSourceDevice;
        decompressState.pDeviceOutput = pDecompressStruct->pDestDevice;
        decompressState.nInputOffset = 0;
        decompressState.nInputLimit = nDefaultInputLimit;
        decompressState.nProcessedOffset = pDecompressStruct->nDecompressedOffset;
        decompressState.nProcessedLimit = pDecompressStruct->nDecompressedLimit;

        if (XSSMDecoder::decompress(&decompressState, nMethod, pPdStruct)) {
            pDecompressStruct->nInSize = decompressState.nCountInput;
            pDecompressStruct->nOutSize = decompressState.nCountOutput;
            pDecompressStruct->bLimit = (pDecompressStruct->nDecompressedLimit != -1) && (decompressState.nCountOutput >= nWindowEnd);
            result = COMPRESS_RESULT_OK;
        } else if (decompressState.bReadError) {
            result = COMPRESS_RESULT_READERROR;
        } else if (decompressState.bWriteError) {
            result = COMPRESS_RESULT_WRITEERROR;
        } else {
            result = COMPRESS_RESULT_DATAERROR;
        }
    } else if (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_PPMD8) {
        XBinary::DATAPROCESS_STATE decompressState = {};
        decompressState.mapProperties.insert(XBinary::FPART_PROP_HANDLEMETHOD, HANDLE_METHOD_PPMD8);
        decompressState.mapProperties.insert(XBinary::FPART_PROP_UNCOMPRESSEDSIZE, pDecompressStruct->spInfo.nUncompressedSize);
        decompressState.pDeviceInput = pDecompressStruct->pSourceDevice;
        decompressState.pDeviceOutput = pDecompressStruct->pDestDevice;
        decompressState.nInputOffset = 0;
        decompressState.nInputLimit = nDefaultInputLimit;
        decompressState.nProcessedOffset = pDecompressStruct->nDecompressedOffset;
        decompressState.nProcessedLimit = pDecompressStruct->nDecompressedLimit;

        if (XPPMdDecoder::decompressPPMD8(&decompressState, pPdStruct)) {
            pDecompressStruct->nInSize = decompressState.nCountInput;
            pDecompressStruct->nOutSize = decompressState.nCountOutput;
            pDecompressStruct->bLimit = (pDecompressStruct->nDecompressedLimit != -1) && (decompressState.nCountOutput >= nWindowEnd);
            result = COMPRESS_RESULT_OK;
        } else {
            if (decompressState.bReadError) {
                result = COMPRESS_RESULT_READERROR;
            } else if (decompressState.bWriteError) {
                result = COMPRESS_RESULT_WRITEERROR;
            } else {
                result = COMPRESS_RESULT_DATAERROR;
            }
        }
    } else if ((pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_DEFLATE) ||
               (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_WISE_DEFLATE)) {
        XBinary::DATAPROCESS_STATE decompressState = {};
        decompressState.mapProperties.insert(XBinary::FPART_PROP_HANDLEMETHOD, HANDLE_METHOD_DEFLATE);
        decompressState.mapProperties.insert(XBinary::FPART_PROP_UNCOMPRESSEDSIZE, pDecompressStruct->spInfo.nUncompressedSize);
        decompressState.pDeviceInput = pDecompressStruct->pSourceDevice;
        decompressState.pDeviceOutput = pDecompressStruct->pDestDevice;
        decompressState.nInputOffset = 0;
        decompressState.nInputLimit = nDefaultInputLimit;
        decompressState.nProcessedOffset = pDecompressStruct->nDecompressedOffset;
        decompressState.nProcessedLimit = pDecompressStruct->nDecompressedLimit;

        if (XDeflateDecoder::decompress(
                &decompressState, pPdStruct,
                pDecompressStruct->spInfo.compressMethod ==
                    HANDLE_METHOD_WISE_DEFLATE)) {
            pDecompressStruct->nInSize = decompressState.nCountInput;
            pDecompressStruct->nOutSize = decompressState.nCountOutput;
            pDecompressStruct->bLimit = (pDecompressStruct->nDecompressedLimit != -1) && (decompressState.nCountOutput >= nWindowEnd);
            result = COMPRESS_RESULT_OK;
        } else {
            if (decompressState.bReadError) {
                result = COMPRESS_RESULT_READERROR;
            } else if (decompressState.bWriteError) {
                result = COMPRESS_RESULT_WRITEERROR;
            } else {
                result = COMPRESS_RESULT_DATAERROR;
            }
        }
    } else if (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_BZIP2) {
        XBinary::DATAPROCESS_STATE decompressState = {};
        decompressState.mapProperties.insert(XBinary::FPART_PROP_HANDLEMETHOD, HANDLE_METHOD_BZIP2);
        decompressState.mapProperties.insert(XBinary::FPART_PROP_UNCOMPRESSEDSIZE, pDecompressStruct->spInfo.nUncompressedSize);
        decompressState.pDeviceInput = pDecompressStruct->pSourceDevice;
        decompressState.pDeviceOutput = pDecompressStruct->pDestDevice;
        decompressState.nInputOffset = 0;
        decompressState.nInputLimit = nDefaultInputLimit;
        decompressState.nProcessedOffset = pDecompressStruct->nDecompressedOffset;
        decompressState.nProcessedLimit = pDecompressStruct->nDecompressedLimit;

        if (XBZIP2Decoder::decompress(&decompressState, pPdStruct)) {
            pDecompressStruct->nInSize = decompressState.nCountInput;
            pDecompressStruct->nOutSize = decompressState.nCountOutput;
            pDecompressStruct->bLimit = (pDecompressStruct->nDecompressedLimit != -1) && (decompressState.nCountOutput >= nWindowEnd);
            result = COMPRESS_RESULT_OK;
        } else {
            if (decompressState.bReadError) {
                result = COMPRESS_RESULT_READERROR;
            } else if (decompressState.bWriteError) {
                result = COMPRESS_RESULT_WRITEERROR;
            } else {
                result = COMPRESS_RESULT_DATAERROR;
            }
        }
    } else if (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_LZMA) {
        XBinary::DATAPROCESS_STATE decompressState = {};
        decompressState.mapProperties.insert(XBinary::FPART_PROP_HANDLEMETHOD, HANDLE_METHOD_LZMA);
        decompressState.mapProperties.insert(XBinary::FPART_PROP_UNCOMPRESSEDSIZE, pDecompressStruct->spInfo.nUncompressedSize);
        decompressState.pDeviceInput = pDecompressStruct->pSourceDevice;
        decompressState.pDeviceOutput = pDecompressStruct->pDestDevice;
        decompressState.nInputOffset = 0;
        decompressState.nInputLimit = nDefaultInputLimit;
        decompressState.nProcessedOffset = pDecompressStruct->nDecompressedOffset;
        decompressState.nProcessedLimit = pDecompressStruct->nDecompressedLimit;

        if (XLZMADecoder::decompress(&decompressState, pPdStruct)) {
            pDecompressStruct->nInSize = decompressState.nCountInput;
            pDecompressStruct->nOutSize = decompressState.nCountOutput;
            pDecompressStruct->bLimit = (pDecompressStruct->nDecompressedLimit != -1) && (decompressState.nCountOutput >= nWindowEnd);
            result = COMPRESS_RESULT_OK;
        } else {
            if (decompressState.bReadError) {
                result = COMPRESS_RESULT_READERROR;
            } else if (decompressState.bWriteError) {
                result = COMPRESS_RESULT_WRITEERROR;
            } else {
                result = COMPRESS_RESULT_DATAERROR;
            }
        }
    } else if (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_ZOO_LZD) {
        XBinary::DATAPROCESS_STATE decompressState = {};
        decompressState.mapProperties.insert(XBinary::FPART_PROP_HANDLEMETHOD, HANDLE_METHOD_ZOO_LZD);
        decompressState.mapProperties.insert(XBinary::FPART_PROP_UNCOMPRESSEDSIZE, pDecompressStruct->spInfo.nUncompressedSize);
        decompressState.pDeviceInput = pDecompressStruct->pSourceDevice;
        decompressState.pDeviceOutput = pDecompressStruct->pDestDevice;
        decompressState.nInputOffset = 0;
        decompressState.nInputLimit = nDefaultInputLimit;
        decompressState.nProcessedOffset = pDecompressStruct->nDecompressedOffset;
        decompressState.nProcessedLimit = pDecompressStruct->nDecompressedLimit;

        if (XLZWDecoder::decompress_zoo(&decompressState, pPdStruct)) {
            pDecompressStruct->nInSize = decompressState.nCountInput;
            pDecompressStruct->nOutSize = decompressState.nCountOutput;
            pDecompressStruct->bLimit = (pDecompressStruct->nDecompressedLimit != -1) && (decompressState.nCountOutput >= nWindowEnd);
            result = COMPRESS_RESULT_OK;
        } else {
            if (decompressState.bReadError) {
                result = COMPRESS_RESULT_READERROR;
            } else if (decompressState.bWriteError) {
                result = COMPRESS_RESULT_WRITEERROR;
            } else {
                result = COMPRESS_RESULT_DATAERROR;
            }
        }
    } else if (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_KWAJ_LZSS) {
        XBinary::DATAPROCESS_STATE decompressState = {};
        decompressState.mapProperties.insert(XBinary::FPART_PROP_HANDLEMETHOD, HANDLE_METHOD_KWAJ_LZSS);
        decompressState.mapProperties.insert(XBinary::FPART_PROP_UNCOMPRESSEDSIZE, pDecompressStruct->spInfo.nUncompressedSize);
        decompressState.pDeviceInput = pDecompressStruct->pSourceDevice;
        decompressState.pDeviceOutput = pDecompressStruct->pDestDevice;
        decompressState.nInputOffset = 0;
        decompressState.nInputLimit = nDefaultInputLimit;
        decompressState.nProcessedOffset = pDecompressStruct->nDecompressedOffset;
        decompressState.nProcessedLimit = pDecompressStruct->nDecompressedLimit;

        if (XKWAJLZSSDecoder::decompress(&decompressState, pPdStruct)) {
            pDecompressStruct->nInSize = decompressState.nCountInput;
            pDecompressStruct->nOutSize = decompressState.nCountOutput;
            pDecompressStruct->bLimit = (pDecompressStruct->nDecompressedLimit != -1) && (decompressState.nCountOutput >= nWindowEnd);
            result = COMPRESS_RESULT_OK;
        } else if (decompressState.bReadError) {
            result = COMPRESS_RESULT_READERROR;
        } else if (decompressState.bWriteError) {
            result = COMPRESS_RESULT_WRITEERROR;
        } else {
            result = COMPRESS_RESULT_DATAERROR;
        }
    } else if (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_KWAJ_LZH) {
        XBinary::DATAPROCESS_STATE decompressState = {};
        decompressState.mapProperties.insert(XBinary::FPART_PROP_HANDLEMETHOD, HANDLE_METHOD_KWAJ_LZH);
        decompressState.mapProperties.insert(XBinary::FPART_PROP_UNCOMPRESSEDSIZE, pDecompressStruct->spInfo.nUncompressedSize);
        decompressState.pDeviceInput = pDecompressStruct->pSourceDevice;
        decompressState.pDeviceOutput = pDecompressStruct->pDestDevice;
        decompressState.nInputOffset = 0;
        decompressState.nInputLimit = nDefaultInputLimit;
        decompressState.nProcessedOffset = pDecompressStruct->nDecompressedOffset;
        decompressState.nProcessedLimit = pDecompressStruct->nDecompressedLimit;

        if (XKWAJLZHDecoder::decompress(&decompressState, pPdStruct)) {
            pDecompressStruct->nInSize = decompressState.nCountInput;
            pDecompressStruct->nOutSize = decompressState.nCountOutput;
            pDecompressStruct->bLimit = (pDecompressStruct->nDecompressedLimit != -1) && (decompressState.nCountOutput >= nWindowEnd);
            result = COMPRESS_RESULT_OK;
        } else if (decompressState.bReadError) {
            result = COMPRESS_RESULT_READERROR;
        } else if (decompressState.bWriteError) {
            result = COMPRESS_RESULT_WRITEERROR;
        } else {
            result = COMPRESS_RESULT_DATAERROR;
        }
    } else if (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_KWAJ_MSZIP) {
        XBinary::DATAPROCESS_STATE decompressState = {};
        decompressState.mapProperties.insert(XBinary::FPART_PROP_HANDLEMETHOD, HANDLE_METHOD_KWAJ_MSZIP);
        decompressState.mapProperties.insert(XBinary::FPART_PROP_UNCOMPRESSEDSIZE, pDecompressStruct->spInfo.nUncompressedSize);
        decompressState.pDeviceInput = pDecompressStruct->pSourceDevice;
        decompressState.pDeviceOutput = pDecompressStruct->pDestDevice;
        decompressState.nInputOffset = 0;
        decompressState.nInputLimit = nDefaultInputLimit;
        decompressState.nProcessedOffset = pDecompressStruct->nDecompressedOffset;
        decompressState.nProcessedLimit = pDecompressStruct->nDecompressedLimit;

        XDecompress decompressor;
        if (decompressor.decompress(&decompressState, pPdStruct)) {
            pDecompressStruct->nInSize = decompressState.nCountInput;
            pDecompressStruct->nOutSize = decompressState.nCountOutput;
            pDecompressStruct->bLimit = (pDecompressStruct->nDecompressedLimit != -1) && (decompressState.nCountOutput >= nWindowEnd);
            result = COMPRESS_RESULT_OK;
        } else if (decompressState.bReadError) {
            result = COMPRESS_RESULT_READERROR;
        } else if (decompressState.bWriteError) {
            result = COMPRESS_RESULT_WRITEERROR;
        } else {
            result = COMPRESS_RESULT_DATAERROR;
        }
    } else if ((pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_LZH4) || (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_LZH5) ||
               (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_LZH6) || (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_LZH7) ||
               (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_JASC_COMPRESSED) ||
               (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_ZOO_LZH)) {
        qint32 nMethod = 5;
        XLZHDecoder::TERMINATION_MODE terminationMode = XLZHDecoder::TERMINATION_PHYSICAL_EOF;

        if (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_LZH4) {
            nMethod = 4;
        } else if (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_LZH5) {
            nMethod = 5;
        } else if (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_LZH6) {
            nMethod = 6;
        } else if (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_LZH7) {
            nMethod = 7;
        } else if (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_JASC_COMPRESSED) {
            nMethod = 5;
        } else if (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_ZOO_LZH) {
            terminationMode = XLZHDecoder::TERMINATION_ZERO_BLOCK;
        }

        XBinary::DATAPROCESS_STATE decompressState = {};
        decompressState.mapProperties.insert(XBinary::FPART_PROP_HANDLEMETHOD, pDecompressStruct->spInfo.compressMethod);
        decompressState.mapProperties.insert(XBinary::FPART_PROP_UNCOMPRESSEDSIZE, pDecompressStruct->spInfo.nUncompressedSize);
        decompressState.pDeviceInput = pDecompressStruct->pSourceDevice;
        decompressState.pDeviceOutput = pDecompressStruct->pDestDevice;
        decompressState.nInputOffset = 0;
        decompressState.nInputLimit = nDefaultInputLimit;
        decompressState.nProcessedOffset = pDecompressStruct->nDecompressedOffset;
        decompressState.nProcessedLimit = pDecompressStruct->nDecompressedLimit;

        if (XLZHDecoder::decompress(&decompressState, nMethod, pPdStruct, terminationMode)) {
            pDecompressStruct->nInSize = decompressState.nCountInput;
            pDecompressStruct->nOutSize = decompressState.nCountOutput;
            pDecompressStruct->bLimit = (pDecompressStruct->nDecompressedLimit != -1) && (decompressState.nCountOutput >= nWindowEnd);
            result = COMPRESS_RESULT_OK;
        } else {
            if (decompressState.bReadError) {
                result = COMPRESS_RESULT_READERROR;
            } else if (decompressState.bWriteError) {
                result = COMPRESS_RESULT_WRITEERROR;
            } else {
                result = COMPRESS_RESULT_DATAERROR;
            }
        }
    } else if ((pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_RAR_15) || (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_RAR_20) ||
               (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_RAR_29) || (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_RAR_50) ||
               (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_RAR_70)) {
        if (pDecompressStruct->spInfo.nUncompressedSize < 0) {
            return COMPRESS_RESULT_DATAERROR;
        }
        bool bIsSolid = false;
        qint64 nRarInputSize = pDecompressStruct->nInSize;
        const qint64 nSourceSize = guardedSource->size();
        if (!guardedSource || !guardedDestination || !isPdStructLifetimeAlive(progressLifetime)) {
            return COMPRESS_RESULT_UNKNOWN;
        }
        if (nRarInputSize == 0) {
            if (nSourceSize < 0) {
                return COMPRESS_RESULT_DATAERROR;
            }
            nRarInputSize = nSourceSize;
        } else {
            const bool bSourceSequential = guardedSource->isSequential();
            if (!guardedSource || !guardedDestination || !isPdStructLifetimeAlive(progressLifetime)) {
                return COMPRESS_RESULT_UNKNOWN;
            }
            if (!bSourceSequential && (nSourceSize >= 0) && (nRarInputSize > nSourceSize)) {
                return COMPRESS_RESULT_READERROR;
            }
        }

        const bool bSourceSeeked = guardedSource->seek(0);
        if (!guardedSource || !guardedDestination || !isPdStructLifetimeAlive(progressLifetime)) {
            return COMPRESS_RESULT_UNKNOWN;
        }
        if (!bSourceSeeked) {
            const qint64 nSourcePosition = guardedSource->pos();
            if (!guardedSource || !guardedDestination || !isPdStructLifetimeAlive(progressLifetime)) {
                return COMPRESS_RESULT_UNKNOWN;
            }
            if (nSourcePosition != 0) return COMPRESS_RESULT_READERROR;
        }

        ArchiveBoundedReadDevice inputDevice(pDecompressStruct->pSourceDevice, nRarInputSize);
        if (!inputDevice.open(QIODevice::ReadOnly)) {
            return COMPRESS_RESULT_READERROR;
        }

        ArchiveWindowWriteDevice windowDevice(pDecompressStruct->pDestDevice, pDecompressStruct->nDecompressedOffset, pDecompressStruct->nDecompressedLimit);
        if (!windowDevice.open(QIODevice::WriteOnly)) {
            inputDevice.close();
            return COMPRESS_RESULT_WRITEERROR;
        }

        std::unique_ptr<rar_Unpack> pRarUnpack(new (std::nothrow) rar_Unpack());
        if (!pRarUnpack) {
            windowDevice.close();
            inputDevice.close();
            return COMPRESS_RESULT_MEMORYERROR;
        }

        pRarUnpack->setDevices(&inputDevice, &windowDevice);
        qint32 nInit = pRarUnpack->Init(pDecompressStruct->spInfo.nWindowSize, bIsSolid);

        if (nInit > 0) {
            pRarUnpack->SetDestSize(pDecompressStruct->spInfo.nUncompressedSize);
            ArchiveRarProgressBridge rarBridge = {pPdStruct, progressLifetime};
            PDSTRUCT rarProgress = getPdStructSnapshot(pPdStruct);
            setPdStructCallback(&rarProgress, archiveRarProgressCallback, &rarBridge);

            if (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_RAR_15) {
                pRarUnpack->Unpack15(bIsSolid, &rarProgress);
            } else if (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_RAR_20) {
                pRarUnpack->Unpack20(bIsSolid, &rarProgress);
            } else if (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_RAR_29) {
                pRarUnpack->Unpack29(bIsSolid, &rarProgress);
            } else if ((pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_RAR_50) || (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_RAR_70)) {
                pRarUnpack->Unpack5(bIsSolid, &rarProgress);
            }

            if (!isPdStructLifetimeAlive(progressLifetime) || !guardedSource || !guardedDestination) {
                windowDevice.close();
                inputDevice.close();
                return COMPRESS_RESULT_UNKNOWN;
            }

            if (windowDevice.hasError()) {
                result = COMPRESS_RESULT_WRITEERROR;
            } else if (inputDevice.hasError()) {
                result = COMPRESS_RESULT_READERROR;
            } else {
                result = (pRarUnpack->IsFileExtracted() && (windowDevice.produced() == pDecompressStruct->spInfo.nUncompressedSize) &&
                          XBinary::isPdStructNotCanceled(pPdStruct))
                             ? COMPRESS_RESULT_OK
                             : COMPRESS_RESULT_DATAERROR;
            }
        } else {
            result = COMPRESS_RESULT_MEMORYERROR;
        }

        pDecompressStruct->nOutSize = windowDevice.produced();
        pDecompressStruct->nDecompressedWrote = windowDevice.written();
        pDecompressStruct->bLimit = (pDecompressStruct->nDecompressedLimit != -1) && (windowDevice.produced() >= nWindowEnd);
        windowDevice.close();
        inputDevice.close();
        if (result == COMPRESS_RESULT_OK) {
            pDecompressStruct->nInSize = inputDevice.consumed();
        }
    } else if (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_LZSS_SZDD) {
        XBinary::DATAPROCESS_STATE decompressState = {};
        decompressState.mapProperties.insert(XBinary::FPART_PROP_HANDLEMETHOD, HANDLE_METHOD_LZSS_SZDD);
        decompressState.mapProperties.insert(XBinary::FPART_PROP_UNCOMPRESSEDSIZE, pDecompressStruct->spInfo.nUncompressedSize);
        decompressState.pDeviceInput = pDecompressStruct->pSourceDevice;
        decompressState.pDeviceOutput = pDecompressStruct->pDestDevice;
        decompressState.nInputOffset = 0;
        decompressState.nInputLimit = nDefaultInputLimit;
        decompressState.nProcessedOffset = pDecompressStruct->nDecompressedOffset;
        decompressState.nProcessedLimit = pDecompressStruct->nDecompressedLimit;

        if (XLZSSDecoder::decompress(&decompressState, pPdStruct)) {
            pDecompressStruct->nInSize = decompressState.nCountInput;
            pDecompressStruct->nOutSize = decompressState.nCountOutput;
            pDecompressStruct->bLimit = (pDecompressStruct->nDecompressedLimit != -1) && (decompressState.nCountOutput >= nWindowEnd);
            result = COMPRESS_RESULT_OK;
        } else {
            if (decompressState.bReadError) {
                result = COMPRESS_RESULT_READERROR;
            } else if (decompressState.bWriteError) {
                result = COMPRESS_RESULT_WRITEERROR;
            } else {
                result = COMPRESS_RESULT_DATAERROR;
            }
        }
    } else if (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_COKTEL_LZ) {
        XBinary::DATAPROCESS_STATE decompressState = {};
        decompressState.mapProperties.insert(XBinary::FPART_PROP_HANDLEMETHOD, HANDLE_METHOD_COKTEL_LZ);
        decompressState.mapProperties.insert(XBinary::FPART_PROP_UNCOMPRESSEDSIZE, pDecompressStruct->spInfo.nUncompressedSize);
        decompressState.pDeviceInput = pDecompressStruct->pSourceDevice;
        decompressState.pDeviceOutput = pDecompressStruct->pDestDevice;
        decompressState.nInputOffset = 0;
        decompressState.nInputLimit = nDefaultInputLimit;
        decompressState.nProcessedOffset = pDecompressStruct->nDecompressedOffset;
        decompressState.nProcessedLimit = pDecompressStruct->nDecompressedLimit;

        if (XCoktelLZDecoder::decompress(&decompressState, pPdStruct)) {
            pDecompressStruct->nInSize = decompressState.nCountInput;
            pDecompressStruct->nOutSize = decompressState.nCountOutput;
            pDecompressStruct->bLimit = (pDecompressStruct->nDecompressedLimit != -1) && (decompressState.nCountOutput >= nWindowEnd);
            result = COMPRESS_RESULT_OK;
        } else if (decompressState.bReadError) {
            result = COMPRESS_RESULT_READERROR;
        } else if (decompressState.bWriteError) {
            result = COMPRESS_RESULT_WRITEERROR;
        } else {
            result = COMPRESS_RESULT_DATAERROR;
        }
    } else if (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_WINZIP_JPEG) {
        XBinary::DATAPROCESS_STATE decompressState = {};
        decompressState.mapProperties.insert(XBinary::FPART_PROP_HANDLEMETHOD, HANDLE_METHOD_WINZIP_JPEG);
        decompressState.mapProperties.insert(XBinary::FPART_PROP_UNCOMPRESSEDSIZE, pDecompressStruct->spInfo.nUncompressedSize);
        decompressState.pDeviceInput = pDecompressStruct->pSourceDevice;
        decompressState.pDeviceOutput = pDecompressStruct->pDestDevice;
        decompressState.nInputOffset = 0;
        decompressState.nInputLimit = nDefaultInputLimit;
        decompressState.nProcessedOffset = pDecompressStruct->nDecompressedOffset;
        decompressState.nProcessedLimit = pDecompressStruct->nDecompressedLimit;

        if (XWinZipJPEGDecoder::decompress(&decompressState, pPdStruct)) {
            pDecompressStruct->nInSize = decompressState.nCountInput;
            pDecompressStruct->nOutSize = decompressState.nCountOutput;
            pDecompressStruct->bLimit = (pDecompressStruct->nDecompressedLimit != -1) && (decompressState.nCountOutput >= nWindowEnd);
            result = COMPRESS_RESULT_OK;
        } else if (decompressState.bReadError) {
            result = COMPRESS_RESULT_READERROR;
        } else if (decompressState.bWriteError) {
            result = COMPRESS_RESULT_WRITEERROR;
        } else {
            result = COMPRESS_RESULT_DATAERROR;
        }
    } else if (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_WAVPACK) {
        XBinary::DATAPROCESS_STATE decompressState = {};
        decompressState.mapProperties.insert(XBinary::FPART_PROP_HANDLEMETHOD, HANDLE_METHOD_WAVPACK);
        decompressState.mapProperties.insert(XBinary::FPART_PROP_UNCOMPRESSEDSIZE, pDecompressStruct->spInfo.nUncompressedSize);
        decompressState.pDeviceInput = pDecompressStruct->pSourceDevice;
        decompressState.pDeviceOutput = pDecompressStruct->pDestDevice;
        decompressState.nInputOffset = 0;
        decompressState.nInputLimit = nDefaultInputLimit;
        decompressState.nProcessedOffset = pDecompressStruct->nDecompressedOffset;
        decompressState.nProcessedLimit = pDecompressStruct->nDecompressedLimit;

        if (XWavPackDecoder::decompress(&decompressState, pPdStruct)) {
            pDecompressStruct->nInSize = decompressState.nCountInput;
            pDecompressStruct->nOutSize = decompressState.nCountOutput;
            pDecompressStruct->bLimit = (pDecompressStruct->nDecompressedLimit != -1) && (decompressState.nCountOutput >= nWindowEnd);
            result = COMPRESS_RESULT_OK;
        } else if (decompressState.bReadError) {
            result = COMPRESS_RESULT_READERROR;
        } else if (decompressState.bWriteError) {
            result = COMPRESS_RESULT_WRITEERROR;
        } else {
            result = COMPRESS_RESULT_DATAERROR;
        }
    } else if (pDecompressStruct->spInfo.compressMethod == HANDLE_METHOD_XZ) {
        XBinary::DATAPROCESS_STATE decompressState = {};
        decompressState.mapProperties.insert(XBinary::FPART_PROP_HANDLEMETHOD, HANDLE_METHOD_XZ);
        decompressState.mapProperties.insert(XBinary::FPART_PROP_UNCOMPRESSEDSIZE, pDecompressStruct->spInfo.nUncompressedSize);
        decompressState.pDeviceInput = pDecompressStruct->pSourceDevice;
        decompressState.pDeviceOutput = pDecompressStruct->pDestDevice;
        decompressState.nInputOffset = 0;
        decompressState.nInputLimit = nDefaultInputLimit;
        decompressState.nProcessedOffset = pDecompressStruct->nDecompressedOffset;
        decompressState.nProcessedLimit = pDecompressStruct->nDecompressedLimit;

        if (XLZMADecoder::decompressXZ(&decompressState, pPdStruct)) {
            pDecompressStruct->nInSize = decompressState.nCountInput;
            pDecompressStruct->nOutSize = decompressState.nCountOutput;
            pDecompressStruct->bLimit = (pDecompressStruct->nDecompressedLimit != -1) && (decompressState.nCountOutput >= nWindowEnd);
            result = COMPRESS_RESULT_OK;
        } else {
            if (decompressState.bReadError) {
                result = COMPRESS_RESULT_READERROR;
            } else if (decompressState.bWriteError) {
                result = COMPRESS_RESULT_WRITEERROR;
            } else {
                result = COMPRESS_RESULT_DATAERROR;
            }
        }
    }

    if (result == COMPRESS_RESULT_OK) {
        const qint64 nWriteEnd = (std::min)(pDecompressStruct->nOutSize, nWindowEnd);
        pDecompressStruct->nDecompressedWrote = (nWriteEnd > pDecompressStruct->nDecompressedOffset) ? (nWriteEnd - pDecompressStruct->nDecompressedOffset) : 0;
    }

    return result;
}

bool XArchive::_decompressRecord(const RECORD *pRecord, QIODevice *pSourceDevice, QIODevice *pDestDevice, PDSTRUCT *pPdStruct, qint64 nDecompressedOffset = 0,
                                 qint64 nDecompressedLimit = -1, const QMap<UNPACK_PROP, QVariant> &mapUnpackProperties)
{
    bool bResult = false;

    if (!pRecord || !pSourceDevice || !pDestDevice || (pRecord->nDataOffset < 0) || (pRecord->nDataSize < 0) || (nDecompressedOffset < 0) || (nDecompressedLimit < -1) ||
        ((nDecompressedLimit != -1) && (nDecompressedOffset > ((std::numeric_limits<qint64>::max)() - nDecompressedLimit)))) {
        return false;
    }
    // This static path has no owning archive session with which to resolve a
    // logical record.  Never reinterpret the outer transport bytes as the
    // member's packed stream.
    //
    // The primary barrier is that such a record has no usable coordinates at
    // all: markArchiveStreamRecord() publishes ARCHIVE_STREAM_NO_EXTENT, which
    // the (nDataOffset < 0) || (nDataSize < 0) test above already refuses, so
    // forging spInfo.compressMethod buys an attacker nothing.
    //
    // The tests below refuse such a record explicitly rather than
    // incidentally, and they hold even if a caller overwrites the extent by
    // hand.  FPART_PROP_ARCHIVE_RECORD_INDEX in particular is set by exactly
    // one function - markArchiveStreamRecord() - so its presence identifies an
    // index-paired record no matter what else has been rewritten around it.
    if ((pRecord->spInfo.compressMethod == HANDLE_METHOD_ARCHIVE_STREAM) ||
        (pRecord->mapProperties.value(XBinary::FPART_PROP_HANDLEMETHOD, HANDLE_METHOD_UNKNOWN).toInt() == HANDLE_METHOD_ARCHIVE_STREAM) ||
        pRecord->mapProperties.contains(XBinary::FPART_PROP_ARCHIVE_RECORD_INDEX) || pRecord->mapProperties.contains(XBinary::FPART_PROP_ARCHIVE_RECORD_TOKEN)) {
        return false;
    }

    const RECORD record = *pRecord;
    QPointer<QIODevice> guardedSource(pSourceDevice);
    QPointer<QIODevice> guardedDestination(pDestDevice);
    const PDSTRUCTLIFETIME progressLifetime = pPdStruct ? retainPdStructLifetime(pPdStruct) : PDSTRUCTLIFETIME();
    const ArchiveContextAlive isContextAlive(guardedSource, guardedDestination, pPdStruct, progressLifetime);
    if (!isContextAlive()) return false;

    XBinary::OUTPUT_POLICY legacyPolicy = {};
    if (!XBinary::resolveUnpackOutputPolicy(mapUnpackProperties, &legacyPolicy)) {
        setPdStructErrorString(pPdStruct, tr("Invalid unpacked-output limit"));
        return false;
    }

    const qint64 nSourceSize = guardedSource->size();
    if (!isContextAlive() || (nSourceSize < 0) || (record.nDataOffset > nSourceSize) || (record.nDataSize > (nSourceSize - record.nDataOffset))) {
        return false;
    }

    SubDevice sd(guardedSource.data(), record.nDataOffset, record.nDataSize);

    if (sd.open(QIODevice::ReadOnly) && isContextAlive()) {
        XBinary::DATAPROCESS_STATE state = {};
        state.mapProperties = record.mapProperties;
        state.mapProperties.insert(XBinary::FPART_PROP_HANDLEMETHOD, record.spInfo.compressMethod);
        if (record.spInfo.compressMethod2 != HANDLE_METHOD_UNKNOWN) {
            state.mapProperties.insert(XBinary::FPART_PROP_HANDLEMETHOD2, record.spInfo.compressMethod2);
        }
        state.mapProperties.insert(XBinary::FPART_PROP_UNCOMPRESSEDSIZE, record.spInfo.nUncompressedSize);
        state.mapProperties.insert(XBinary::FPART_PROP_WINDOWSIZE, record.spInfo.nWindowSize);
        if (record.spInfo.bIsSolid) {
            state.mapProperties.insert(XBinary::FPART_PROP_ISSOLID, true);
        }

        if ((record.spInfo.compressMethod == HANDLE_METHOD_STORE_CAB) || (record.spInfo.compressMethod == HANDLE_METHOD_MSZIP_CAB) ||
            (record.spInfo.compressMethod == HANDLE_METHOD_LZX_CAB) || (record.spInfo.compressMethod == HANDLE_METHOD_QUANTUM_CAB)) {
            state.mapProperties.insert(XBinary::FPART_PROP_SUBSTREAMOFFSET, record.nOptHeaderOffset);
            state.mapProperties.insert(XBinary::FPART_PROP_OPTHEADER_SIZE, record.nOptHeaderSize);
        }

        state.pDeviceInput = &sd;
        state.pDeviceOutput = guardedDestination.data();
        state.nInputOffset = 0;
        state.nInputLimit = record.nDataSize;
        state.nProcessedOffset = nDecompressedOffset;
        state.nProcessedLimit = nDecompressedLimit;
        // Without this the whole decoder chain resolves every output gate to
        // -1: the state was built with seven fields and this one was missing,
        // so UNPACK_PROP_MAX_OUTPUT_SIZE never reached multiDecompress or the
        // _writeDevice choke point.
        state.mapUnpackProperties = mapUnpackProperties;
        // The legacy RECORD route has no owning archive session, so mint its
        // budget here. Explicit aggregate/count policy enforces; defaults shadow.
        {
            state.spOutputBudget = QSharedPointer<XBinary::OUTPUT_BUDGET>::create();
            state.spOutputBudget->configureForProperties(legacyPolicy, mapUnpackProperties);
            if (!state.spOutputBudget->beginEntry(0, record.spInfo.sRecordName) && state.spOutputBudget->isEnforcing()) {
                setPdStructErrorString(pPdStruct, tr("Unpacked output exceeds the configured limit"));
                sd.close();
                return false;
            }
        }

        XDecompress decompressor;
        bResult = decompressor.multiDecompress(&state, pPdStruct);
        if (!isContextAlive()) return false;

        sd.close();
    }

    return bResult;
}

XArchive::COMPRESS_RESULT XArchive::_compress(XArchive::HANDLE_METHOD compressMethod, QIODevice *pSourceDevice, QIODevice *pDestDevice, PDSTRUCT *pPdStruct)
{
    XBinary::PDSTRUCT pdStructEmpty = {};

    if (!pPdStruct) {
        pdStructEmpty = XBinary::createPdStruct();
        pPdStruct = &pdStructEmpty;
    }

    if (!pSourceDevice) return COMPRESS_RESULT_READERROR;
    if (!pDestDevice) return COMPRESS_RESULT_WRITEERROR;
    const PDSTRUCTLIFETIME progressLifetime = retainPdStructLifetime(pPdStruct);
    QPointer<QIODevice> guardedSource(pSourceDevice);
    QPointer<QIODevice> guardedDestination(pDestDevice);
    const ArchiveContextAlive isContextAlive(guardedSource, guardedDestination, pPdStruct, progressLifetime);
    if (!isContextAlive()) return COMPRESS_RESULT_UNKNOWN;
    if (!XBinary::isPdStructNotCanceled(pPdStruct)) return COMPRESS_RESULT_UNKNOWN;

    COMPRESS_RESULT result = COMPRESS_RESULT_UNKNOWN;

    if (compressMethod == HANDLE_METHOD_STORE) {
        const qint32 CHUNK = COMPRESS_BUFFERSIZE;
        char *pBuffer = new (std::nothrow) char[CHUNK];
        if (!pBuffer) return COMPRESS_RESULT_MEMORYERROR;

        result = COMPRESS_RESULT_OK;

        while (isContextAlive() && XBinary::isPdStructNotCanceled(pPdStruct)) {
            const qint64 nRead = archiveReadWithBoundedProgress(guardedSource.data(), pBuffer, CHUNK);
            if (!isContextAlive()) {
                result = COMPRESS_RESULT_UNKNOWN;
                break;
            }
            if ((nRead < 0) || (nRead > CHUNK)) {
                result = COMPRESS_RESULT_READERROR;
                break;
            }
            if (nRead == 0) {
                const bool bAtEnd = guardedSource->atEnd();
                if (!isContextAlive()) {
                    result = COMPRESS_RESULT_UNKNOWN;
                    break;
                }
                if (!bAtEnd) result = COMPRESS_RESULT_READERROR;
                break;
            }
            if (!archiveWriteAll(guardedDestination.data(), pBuffer, nRead, pPdStruct)) {
                if (!isContextAlive()) {
                    result = COMPRESS_RESULT_UNKNOWN;
                    break;
                }
                result = COMPRESS_RESULT_WRITEERROR;
                break;
            }
        }

        if ((!isContextAlive() || !XBinary::isPdStructNotCanceled(pPdStruct)) && (result == COMPRESS_RESULT_OK)) {
            result = COMPRESS_RESULT_UNKNOWN;
        }
        delete[] pBuffer;
    } else if (compressMethod == HANDLE_METHOD_DEFLATE) {
        result = _compress_deflate(guardedSource.data(), guardedDestination.data(), Z_DEFAULT_COMPRESSION, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY,
                                   pPdStruct);  // -MAX_WBITS for raw data
    }

    return result;
}

XArchive::COMPRESS_RESULT XArchive::_compress_deflate(QIODevice *pSourceDevice, QIODevice *pDestDevice, qint32 nLevel, qint32 nMethod, qint32 nWindowsBits,
                                                      qint32 nMemLevel, qint32 nStrategy, PDSTRUCT *pPdStruct)
{
    XBinary::PDSTRUCT pdStructEmpty = {};

    if (!pPdStruct) {
        pdStructEmpty = XBinary::createPdStruct();
        pPdStruct = &pdStructEmpty;
    }

    if (!pSourceDevice) return COMPRESS_RESULT_READERROR;
    if (!pDestDevice) return COMPRESS_RESULT_WRITEERROR;
    const PDSTRUCTLIFETIME progressLifetime = retainPdStructLifetime(pPdStruct);
    QPointer<QIODevice> guardedSource(pSourceDevice);
    QPointer<QIODevice> guardedDestination(pDestDevice);
    const ArchiveContextAlive isContextAlive(guardedSource, guardedDestination, pPdStruct, progressLifetime);
    if (!isContextAlive()) return COMPRESS_RESULT_UNKNOWN;
    if (!XBinary::isPdStructNotCanceled(pPdStruct)) return COMPRESS_RESULT_UNKNOWN;

    COMPRESS_RESULT result = COMPRESS_RESULT_UNKNOWN;

    const qint32 CHUNK = COMPRESS_BUFFERSIZE;
    unsigned char *pIn = new (std::nothrow) unsigned char[CHUNK];
    if (!pIn) {
        return COMPRESS_RESULT_MEMORYERROR;
    }
    unsigned char *pOut = new (std::nothrow) unsigned char[CHUNK];
    if (!pOut) {
        delete[] pIn;
        return COMPRESS_RESULT_MEMORYERROR;
    }

    z_stream strm = {};

    strm.zalloc = nullptr;
    strm.zfree = nullptr;
    strm.opaque = nullptr;
    strm.avail_in = 0;
    strm.next_in = nullptr;

    qint32 ret = deflateInit2(&strm, nLevel, nMethod, nWindowsBits, nMemLevel, nStrategy);
    bool bReadError = false;
    bool bWriteError = false;

    if (ret == Z_OK) {
        bool bInputFinished = false;
        do {
            if (!isContextAlive() || !XBinary::isPdStructNotCanceled(pPdStruct)) break;

            const qint64 nRead = archiveReadWithBoundedProgress(guardedSource.data(), reinterpret_cast<char *>(pIn), CHUNK);
            if (!isContextAlive()) break;
            if ((nRead < 0) || (nRead > CHUNK)) {
                bReadError = true;
                break;
            }
            if (nRead == 0) {
                const bool bAtEnd = guardedSource->atEnd();
                if (!isContextAlive()) break;
                if (!bAtEnd) {
                    bReadError = true;
                    break;
                }
                bInputFinished = true;
            }

            strm.avail_in = static_cast<uInt>(nRead);
            strm.next_in = pIn;
            const qint32 nFlush = bInputFinished ? Z_FINISH : Z_NO_FLUSH;

            do {
                if (!isContextAlive() || !XBinary::isPdStructNotCanceled(pPdStruct)) break;
                strm.avail_out = CHUNK;
                strm.next_out = pOut;
                ret = deflate(&strm, nFlush);

                if ((ret != Z_OK) && (ret != Z_STREAM_END)) break;

                const qint32 nProduced = CHUNK - static_cast<qint32>(strm.avail_out);
                if ((nProduced > 0) && !archiveWriteAll(guardedDestination.data(), reinterpret_cast<const char *>(pOut), nProduced, pPdStruct)) {
                    if (!isContextAlive()) break;
                    bWriteError = true;
                    break;
                }
            } while ((strm.avail_out == 0) && (ret != Z_STREAM_END));

            if (bWriteError || ((ret != Z_OK) && (ret != Z_STREAM_END))) break;
        } while (ret != Z_STREAM_END);

        deflateEnd(&strm);
    }

    if (bReadError) {
        result = COMPRESS_RESULT_READERROR;
    } else if (bWriteError) {
        result = COMPRESS_RESULT_WRITEERROR;
    } else if (!isContextAlive() || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        result = COMPRESS_RESULT_UNKNOWN;
    } else if (ret == Z_STREAM_END) {
        result = COMPRESS_RESULT_OK;
    } else if (ret == Z_BUF_ERROR) {
        result = COMPRESS_RESULT_BUFFERERROR;
    } else if (ret == Z_MEM_ERROR) {
        result = COMPRESS_RESULT_MEMORYERROR;
    } else if (ret == Z_DATA_ERROR) {
        result = COMPRESS_RESULT_DATAERROR;
    } else {
        result = COMPRESS_RESULT_UNKNOWN;
    }

    delete[] pIn;
    delete[] pOut;

    return result;
}

QByteArray XArchive::decompress(const XArchive::RECORD *pRecord, PDSTRUCT *pPdStruct, qint64 nDecompressedOffset, qint64 nDecompressedLimit,
                                const QMap<UNPACK_PROP, QVariant> &mapUnpackProperties)
{
    QByteArray result;
    QPointer<XArchive> guardedArchive(this);
    QPointer<QIODevice> guardedSourceDevice(getDevice());
    const PDSTRUCTLIFETIME progressLifetime = pPdStruct ? retainPdStructLifetime(pPdStruct) : PDSTRUCTLIFETIME();
    const ArchiveProgressAlive isProgressAlive(pPdStruct, progressLifetime);

    if (!pRecord || !guardedSourceDevice || (nDecompressedOffset < 0) || (nDecompressedLimit < -1)) return result;

    const ARCHIVERECORD archiveRecord = archiveRecordFromLegacy(*pRecord);
    qint32 nArchiveStreamIndex = -1;
    if (XBinary::getArchiveStreamRecordIndex(archiveRecord, &nArchiveStreamIndex)) {
        Q_UNUSED(nArchiveStreamIndex)
        QBuffer buffer;
        buffer.setBuffer(&result);
        if (!buffer.open(QIODevice::ReadWrite) || !guardedArchive->unpackArchiveStreamRecord(archiveRecord, &buffer, mapUnpackProperties, pPdStruct) || !guardedArchive ||
            !guardedSourceDevice || !isProgressAlive() || !XBinary::isPdStructNotCanceled(pPdStruct)) {
            result.clear();
            return result;
        }
        buffer.close();

        if (nDecompressedOffset > result.size()) {
            result.clear();
            return result;
        }
        if ((nDecompressedOffset != 0) || (nDecompressedLimit != -1)) {
            const qint64 nAvailable = result.size() - nDecompressedOffset;
            const qint64 nResultSize = (nDecompressedLimit == -1) ? nAvailable : qMin(nAvailable, nDecompressedLimit);
            result = result.mid((qint32)nDecompressedOffset, (qint32)nResultSize);
        }
        return result;
    }

    QBuffer buffer;
    buffer.setBuffer(&result);

    // Readable, not write-only: the decoder rereads its complete output to
    // verify the record's stored checksum, so a write-only destination fails
    // every record that carries one.
    if (buffer.open(QIODevice::ReadWrite)) {
        const bool bDecompressed =
            _decompressRecord(pRecord, guardedSourceDevice.data(), &buffer, pPdStruct, nDecompressedOffset, nDecompressedLimit, mapUnpackProperties);
        buffer.close();

        // A QByteArray return value must never expose a valid-looking prefix
        // from a failed or canceled decode.
        if (!guardedArchive || !guardedSourceDevice || !bDecompressed || !isProgressAlive() || !XBinary::isPdStructNotCanceled(pPdStruct)) {
            result.clear();
        }
    }

    return result;
}

QByteArray XArchive::decompress(QList<XArchive::RECORD> *pListArchive, const QString &sRecordFileName, PDSTRUCT *pPdStruct)
{
    QByteArray baResult;

    XArchive::RECORD record = XArchive::getArchiveRecord(sRecordFileName, pListArchive, pPdStruct);

    if (!record.spInfo.sRecordName.isEmpty()) {
        // Empty members still have a codec/checksum contract that must be
        // validated; a declared zero size is not permission to skip decoding.
        baResult = decompress(&record, pPdStruct);
    }

    return baResult;
}

QByteArray XArchive::decompress(const QString &sRecordFileName, PDSTRUCT *pPdStruct)
{
    QPointer<XArchive> guardedArchive(this);
    const PDSTRUCTLIFETIME progressLifetime = pPdStruct ? retainPdStructLifetime(pPdStruct) : PDSTRUCTLIFETIME();
    QList<XArchive::RECORD> listArchive = guardedArchive->getRecords(-1, pPdStruct);
    if (!guardedArchive || (pPdStruct && !isPdStructLifetimeAlive(progressLifetime))) {
        return QByteArray();
    }

    return guardedArchive->decompress(&listArchive, sRecordFileName, pPdStruct);
}

bool XArchive::unpackArchiveStreamRecord(const ARCHIVERECORD &expectedRecord, QIODevice *pOutputDevice, const QMap<UNPACK_PROP, QVariant> &mapProperties,
                                         PDSTRUCT *pPdStruct)
{
    qint32 nRecordIndex = -1;
    // The only thing this route accepts is the exact contract published by
    // XBinary::markArchiveStreamRecord(): a logical record index and the
    // no-extent coordinates.  A record that carries real coordinates is an
    // offset record and does not belong here.
    if (!pOutputDevice || !XBinary::isArchiveStreamNoExtent(expectedRecord.nStreamOffset, expectedRecord.nStreamSize) ||
        !XBinary::getArchiveStreamRecordIndex(expectedRecord, &nRecordIndex)) {
        return false;
    }

    QPointer<XArchive> guardedArchive(this);
    QPointer<QIODevice> guardedSource(getDevice());
    if (!guardedArchive || !guardedSource) return false;

    return _unpackRecordByIndex(nRecordIndex, &expectedRecord, pOutputDevice, mapProperties, pPdStruct);
}

bool XArchive::decompressToFile(const XArchive::RECORD *pRecord, const QString &sResultFileName, PDSTRUCT *pPdStruct,
                                const QMap<UNPACK_PROP, QVariant> &mapUnpackProperties)
{
    QPointer<XArchive> guardedArchive(this);
    const PDSTRUCTLIFETIME progressLifetime = pPdStruct ? retainPdStructLifetime(pPdStruct) : PDSTRUCTLIFETIME();
    const ArchiveProgressAlive isProgressAlive(pPdStruct, progressLifetime);
    if (!pRecord || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    if (pRecord->mapProperties.value(XBinary::FPART_PROP_ISFOLDER, false).toBool()) {
        return XBinary::createDirectory(sResultFileName) && XBinary::isPdStructNotCanceled(pPdStruct);
    }

    QPointer<QIODevice> guardedSourceDevice(getDevice());
    if (!guardedSourceDevice || !guardedSourceDevice->isReadable()) {
        return false;
    }

    QFileInfo fi(sResultFileName);

    if (!XBinary::createDirectory(fi.absolutePath())) {
        return false;
    }

    const ARCHIVERECORD archiveRecord = archiveRecordFromLegacy(*pRecord);
    qint32 nArchiveStreamIndex = -1;
    if (XBinary::getArchiveStreamRecordIndex(archiveRecord, &nArchiveStreamIndex)) {
        Q_UNUSED(nArchiveStreamIndex)
        QSaveFile outputFile(sResultFileName);
        if (!outputFile.open(QIODevice::WriteOnly)) return false;
        const bool bUnpacked = guardedArchive && guardedArchive->unpackArchiveStreamRecord(archiveRecord, &outputFile, mapUnpackProperties, pPdStruct);
        if (!guardedArchive || !guardedSourceDevice || !bUnpacked || !isProgressAlive() || !XBinary::isPdStructNotCanceled(pPdStruct) ||
            (outputFile.error() != QFile::NoError)) {
            outputFile.cancelWriting();
            return false;
        }
        return outputFile.commit();
    }

    // QSaveFile is deliberately write-only, while the shared decompressor
    // rereads complete output for CRC/authentication.  Decode into a private
    // readable temporary first, then copy the authenticated result into the
    // atomic replacement file.
    QTemporaryFile workFile;
    if (!workFile.open()) {
        return false;
    }

    // A zero packed size is not proof of a valid empty member.  The selected
    // codec must still validate its terminator, declared output size, password,
    // and checksum contract.
    const bool bResult = _decompressRecord(pRecord, guardedSourceDevice.data(), &workFile, pPdStruct, 0, -1, mapUnpackProperties);

    if (!guardedArchive || !guardedSourceDevice || !bResult || !isProgressAlive() || !XBinary::isPdStructNotCanceled(pPdStruct) || !workFile.seek(0)) {
        return false;
    }

    QSaveFile file(sResultFileName);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QByteArray baBuffer(0x4000, 0);
    qint64 nRemaining = workFile.size();
    bool bCopyResult = nRemaining >= 0;
    while (bCopyResult && (nRemaining > 0) && isProgressAlive() && XBinary::isPdStructNotCanceled(pPdStruct)) {
        const qint64 nRead = archiveReadWithBoundedProgress(&workFile, baBuffer.data(), (std::min)(nRemaining, (qint64)baBuffer.size()));
        if ((nRead <= 0) || (nRead > nRemaining) || !archiveWriteAll(&file, baBuffer.constData(), nRead, pPdStruct)) {
            bCopyResult = false;
            break;
        }
        nRemaining -= nRead;
    }

    if (!bCopyResult || (nRemaining != 0) || !isProgressAlive() || !XBinary::isPdStructNotCanceled(pPdStruct) || (file.error() != QFile::NoError)) {
        file.cancelWriting();
        return false;
    }

    return file.commit();
}

bool XArchive::decompressToDevice(const RECORD *pRecord, QIODevice *pDestDevice, PDSTRUCT *pPdStruct, const QMap<UNPACK_PROP, QVariant> &mapUnpackProperties)
{
    QPointer<XArchive> guardedArchive(this);
    QPointer<QIODevice> guardedSourceDevice(getDevice());
    QPointer<QIODevice> guardedDestDevice(pDestDevice);
    if (!pRecord || !guardedSourceDevice || !guardedDestDevice) return false;

    const ARCHIVERECORD archiveRecord = archiveRecordFromLegacy(*pRecord);
    qint32 nArchiveStreamIndex = -1;
    if (XBinary::getArchiveStreamRecordIndex(archiveRecord, &nArchiveStreamIndex)) {
        Q_UNUSED(nArchiveStreamIndex)
        const bool bResult = guardedArchive->unpackArchiveStreamRecord(archiveRecord, guardedDestDevice.data(), mapUnpackProperties, pPdStruct);
        return guardedArchive && guardedSourceDevice && guardedDestDevice && bResult;
    }

    const bool bResult = _decompressRecord(pRecord, guardedSourceDevice.data(), guardedDestDevice.data(), pPdStruct, 0, -1, mapUnpackProperties);
    return guardedArchive && guardedSourceDevice && guardedDestDevice && bResult;
}

bool XArchive::decompressToFile(QList<XArchive::RECORD> *pListArchive, const QString &sRecordFileName, const QString &sResultFileName, PDSTRUCT *pPdStruct)
{
    bool bResult = false;
    QPointer<XArchive> guardedArchive(this);

    if (!pListArchive) return false;

    XArchive::RECORD record = getArchiveRecord(sRecordFileName, pListArchive);

    if (record.spInfo.sRecordName != "")  // TODO bIsValid
    {
        bResult = guardedArchive->decompressToFile(&record, sResultFileName, pPdStruct);
    }

    return guardedArchive && bResult;
}

bool XArchive::decompressToPath(QList<XArchive::RECORD> *pListArchive, const QString &sRecordFileName, const QString &sResultPathName, PDSTRUCT *pPdStruct)
{
    if (!pListArchive || !XBinary::isPdStructNotCanceled(pPdStruct)) return false;

    bool bResult = true;
    bool bMatched = false;
    QPointer<XArchive> guardedArchive(this);
    const PDSTRUCTLIFETIME progressLifetime = pPdStruct ? retainPdStructLifetime(pPdStruct) : PDSTRUCTLIFETIME();
    const ArchiveProgressAlive isProgressAlive(pPdStruct, progressLifetime);

    const QString sRawRecordFileName = QDir::fromNativeSeparators(sRecordFileName);
    const bool bSelectorWasProvided = !sRawRecordFileName.isEmpty();
    // Archive member names are exact identifiers.  Trimming here could turn a
    // request for " name" into a request for the distinct member "name".
    QString sNormalizedRecordFileName = sRawRecordFileName;
    while (sNormalizedRecordFileName.endsWith(QLatin1Char('/'))) {
        sNormalizedRecordFileName.chop(1);
    }

    if (bSelectorWasProvided) {
        QString sSafeSelector;
        // Do not clean traversal components here: "safe/.." must be rejected,
        // not normalized to "." and accidentally interpreted as extract-all.
        if (sNormalizedRecordFileName.isEmpty() || !archiveGetSafeRelativePath(sNormalizedRecordFileName, &sSafeSelector)) {
            return false;
        }
        sNormalizedRecordFileName = sSafeSelector;
    }

    QString sCanonicalRoot = _normalizeOutputPath(QDir(sResultPathName).absolutePath());
    if (!XBinary::createDirectory(sCanonicalRoot)) return false;
    sCanonicalRoot = QDir::fromNativeSeparators(QFileInfo(sCanonicalRoot).canonicalFilePath());
    if (sCanonicalRoot.isEmpty() || !QFileInfo(sCanonicalRoot).isDir()) return false;

    XBinary::UNPACK_FOLDER_TRANSACTION transaction(sCanonicalRoot);
    const ArchiveTransactionError transactionError(transaction);
    const ArchiveTransactionErrorReporter reportTransactionError(transactionError, isProgressAlive, pPdStruct, guardedArchive);
    const ArchiveTransactionRollback rollbackTransaction(transaction, reportTransactionError);

    if (!transaction.isValid()) {
        reportTransactionError();
        return false;
    }

    qint32 nNumberOfArchives = pListArchive->count();
    bool bExtractAll = sNormalizedRecordFileName.isEmpty();

    for (qint32 i = 0; (i < nNumberOfArchives) && isProgressAlive() && isPdStructNotCanceled(pPdStruct); i++) {
        XArchive::RECORD record = pListArchive->at(i);
        QString sRecordFileNameInArchive = QDir::fromNativeSeparators(record.spInfo.sRecordName);
        while (sRecordFileNameInArchive.endsWith(QLatin1Char('/'))) {
            sRecordFileNameInArchive.chop(1);
        }

        QString sSafeRecordPath;
        if (!archiveGetSafeRelativePath(sRecordFileNameInArchive, &sSafeRecordPath)) {
            if (bExtractAll) {
                bResult = false;
                break;
            }
            continue;
        }

        const QString sRecordFileNameForMatch = sSafeRecordPath;

        bool bNamePresent = false;
        if (bExtractAll) {
            bNamePresent = true;
        } else if (sRecordFileNameForMatch == sNormalizedRecordFileName) {
            bNamePresent = true;
        } else if (sRecordFileNameForMatch.startsWith(sNormalizedRecordFileName + QLatin1Char('/'))) {
            bNamePresent = true;
        }

        if (bNamePresent || bExtractAll) {
            bMatched = true;
            QString sFileName = sSafeRecordPath;

            if (!bExtractAll && (sFileName != sNormalizedRecordFileName)) {
                const QString sPrefix = sNormalizedRecordFileName + QLatin1Char('/');
                if (sFileName.startsWith(sPrefix)) {
                    sFileName = sFileName.mid(sPrefix.size(), -1);
                }
            }

            if (sFileName.isEmpty()) {
                continue;
            }

            QString sSafeOutputPath;
            if (!archiveGetSafeRelativePath(sFileName, &sSafeOutputPath)) {
                bResult = false;
                break;
            }

            // The record names the output root itself (tar's leading ".");
            // there is nothing to place, and it is not a failure.
            if (sSafeOutputPath.isEmpty()) {
                continue;
            }

            const QString sResultFileName = _normalizeOutputPath(QDir(sCanonicalRoot).absoluteFilePath(sSafeOutputPath));

            if (!XArchive::_isSafeChildPath(sResultFileName, sCanonicalRoot) || archivePathHasUnsafeLink(sCanonicalRoot, sSafeOutputPath)) {
                bResult = false;
                break;
            }

            const bool bIsFolder = record.mapProperties.value(XBinary::FPART_PROP_ISFOLDER, false).toBool();
            const QString sDirectoryName = bIsFolder ? sResultFileName : QFileInfo(sResultFileName).absolutePath();
            if (!transaction.ensureDirectory(sDirectoryName) || archivePathHasUnsafeLink(sCanonicalRoot, sSafeOutputPath)) {
                if (!transaction.errorString().isEmpty()) {
                    reportTransactionError();
                }
                bResult = false;
                break;
            }

            if (!bIsFolder) {
                QTemporaryFile stagedFile(QDir(sDirectoryName).filePath(QLatin1String(".xarchive-legacy-XXXXXX")));
                if (!guardedArchive || !stagedFile.open() || !guardedArchive->decompressToDevice(&record, &stagedFile, pPdStruct) || !guardedArchive ||
                    !isProgressAlive() || !isPdStructNotCanceled(pPdStruct) || !stagedFile.flush() || !stagedFile.seek(0) ||
                    archivePathHasUnsafeLink(sCanonicalRoot, sSafeOutputPath)) {
                    bResult = false;
                    break;
                }

                QSaveFile outputFile(sResultFileName);
                outputFile.setDirectWriteFallback(false);
                bool bPublished = outputFile.open(QIODevice::WriteOnly);
                QByteArray baBuffer(0x4000, 0);
                qint64 nRemaining = stagedFile.size();
                bPublished = bPublished && (nRemaining >= 0);

                while (bPublished && (nRemaining > 0) && isProgressAlive() && isPdStructNotCanceled(pPdStruct)) {
                    const qint64 nRead = archiveReadWithBoundedProgress(&stagedFile, baBuffer.data(), (std::min)(nRemaining, (qint64)baBuffer.size()));
                    if ((nRead <= 0) || (nRead > nRemaining) || !archiveWriteAll(&outputFile, baBuffer.constData(), nRead, pPdStruct)) {
                        bPublished = false;
                        break;
                    }
                    nRemaining -= nRead;
                }

                if (!bPublished || (nRemaining != 0) || !isProgressAlive() || !isPdStructNotCanceled(pPdStruct) || (outputFile.error() != QFile::NoError) ||
                    archivePathHasUnsafeLink(sCanonicalRoot, sSafeOutputPath)) {
                    outputFile.cancelWriting();
                    bResult = false;
                    break;
                }
                if (!transaction.prepareFile(sResultFileName)) {
                    reportTransactionError();
                    outputFile.cancelWriting();
                    bResult = false;
                    break;
                }
                if (!outputFile.commit()) {
                    bResult = false;
                    break;
                }
                if (!transaction.markFilePublished(sResultFileName)) {
                    reportTransactionError();
                    bResult = false;
                    break;
                }
            }
        }
    }
    // TODO emits

    bResult = guardedArchive && bResult && isProgressAlive() && XBinary::isPdStructNotCanceled(pPdStruct) && (bExtractAll || bMatched);
    if (bResult) {
        if (!transaction.commit()) {
            reportTransactionError();
            bResult = false;
            rollbackTransaction();
        } else if (!transaction.errorString().isEmpty()) {
            reportTransactionError();
        }
    } else {
        rollbackTransaction();
    }

    return bResult;
}

bool XArchive::decompressToFile(const QString &sArchiveFileName, const QString &sRecordFileName, const QString &sResultFileName, PDSTRUCT *pPdStruct)
{
    bool bResult = false;
    QPointer<XArchive> guardedArchive(this);
    const PDSTRUCTLIFETIME progressLifetime = pPdStruct ? retainPdStructLifetime(pPdStruct) : PDSTRUCTLIFETIME();
    const ArchiveProgressAlive isProgressAlive(pPdStruct, progressLifetime);

    QFile file;
    file.setFileName(sArchiveFileName);

    if (file.open(QIODevice::ReadOnly)) {
        QPointer<QIODevice> guardedOriginalDevice(guardedArchive ? guardedArchive->getDevice() : nullptr);
        if (!guardedArchive) {
            file.close();
            return false;
        }
        guardedArchive->setDevice(&file);

        if (guardedArchive && isProgressAlive() && guardedArchive->isValid(pPdStruct) && guardedArchive && isProgressAlive()) {
            QList<RECORD> listRecords = guardedArchive->getRecords(-1, pPdStruct);

            if (guardedArchive && isProgressAlive()) {
                bResult = guardedArchive->decompressToFile(&listRecords, sRecordFileName, sResultFileName, pPdStruct);
            }
        }

        // Never leave the archive pointing at this stack-local QFile.
        if (guardedArchive) {
            guardedArchive->setDevice(guardedOriginalDevice.data());
        }
        file.close();
    }

    return bResult;
}

bool XArchive::unpackToFolder(const QString &sResultPathName, PDSTRUCT *pPdStruct)
{
    QPointer<XArchive> guardedArchive(this);
    PDSTRUCT pdStructEmpty = {};
    if (!pPdStruct) {
        pdStructEmpty = XBinary::createPdStruct();
        pPdStruct = &pdStructEmpty;
    }
    const PDSTRUCTLIFETIME progressLifetime = retainPdStructLifetime(pPdStruct);
    const ArchiveProgressAlive isProgressAlive(pPdStruct, progressLifetime);
    if (!isProgressAlive() || !isPdStructNotCanceled(pPdStruct)) return false;

    QString sCanonicalRoot = _normalizeOutputPath(QDir(sResultPathName).absolutePath());
    if (!XBinary::createDirectory(sCanonicalRoot)) return false;
    sCanonicalRoot = QDir::fromNativeSeparators(QFileInfo(sCanonicalRoot).canonicalFilePath());
    if (sCanonicalRoot.isEmpty() || !QFileInfo(sCanonicalRoot).isDir()) return false;

    XBinary::UNPACK_FOLDER_TRANSACTION transaction(sCanonicalRoot);
    const ArchiveTransactionError transactionError(transaction);
    const ArchiveTransactionErrorReporter reportTransactionError(transactionError, isProgressAlive, pPdStruct, guardedArchive);
    const ArchiveTransactionRollback rollbackTransaction(transaction, reportTransactionError);

    if (!transaction.isValid()) {
        reportTransactionError();
        return false;
    }

    UNPACK_STATE state = {};
    QMap<UNPACK_PROP, QVariant> mapProperties;
    const bool bInitialized = guardedArchive && guardedArchive->initUnpack(&state, mapProperties, pPdStruct);
    if (guardedArchive && bInitialized && !isProgressAlive()) {
        guardedArchive->finishUnpack(&state, nullptr);
        rollbackTransaction();
        return false;
    }
    if (!guardedArchive || !bInitialized || !isProgressAlive()) {
        rollbackTransaction();
        return false;
    }

    // Mint one budget per folder extraction so the decode chain's debit sites
    // are live on this deprecated root too. This overload takes no property
    // map, so the resolved defaults always run shadow-metered, never enforcing.
    XBinary::OUTPUT_POLICY outputPolicy = {};
    if (XBinary::resolveUnpackOutputPolicy(mapProperties, &outputPolicy)) {
        state.spOutputBudget = QSharedPointer<XBinary::OUTPUT_BUDGET>::create();
        state.spOutputBudget->configureForProperties(outputPolicy, mapProperties);
    }

    const qint32 nNumberOfRecords = state.nNumberOfRecords;

    bool bEnumerationValid = (state.nCurrentIndex == 0) && (nNumberOfRecords >= 0) && (state.nCurrentIndex <= nNumberOfRecords);
    bool bResult = bEnumerationValid;

    while (bEnumerationValid && (state.nCurrentIndex < nNumberOfRecords) && isProgressAlive() && isPdStructNotCanceled(pPdStruct)) {
        const qint32 nExpectedIndex = state.nCurrentIndex;
        const ARCHIVERECORD archiveRecord = guardedArchive->infoCurrent(&state, pPdStruct);
        if (!guardedArchive || !isProgressAlive() || !isPdStructNotCanceled(pPdStruct) || archiveRecord.mapProperties.isEmpty() ||
            !XBinary::isArchiveRecordExtentValid(archiveRecord) || (state.nCurrentIndex < 0) || (state.nCurrentIndex != nExpectedIndex) ||
            (state.nNumberOfRecords != nNumberOfRecords) || (state.nCurrentIndex >= nNumberOfRecords)) {
            bResult = false;
            bEnumerationValid = false;
            break;
        }
        QString sRecordName = QDir::fromNativeSeparators(archiveRecord.mapProperties.value(FPART_PROP_ORIGINALNAME).toString());
        QString sSafeRecordPath;

        if (!archiveGetSafeRelativePath(sRecordName, &sSafeRecordPath)) {
            bResult = false;
        } else if (sSafeRecordPath.isEmpty()) {
            // The record names the output root itself (tar's leading ".");
            // there is nothing to place, and it is not a failure.
        } else {
            const QString sResultFileName = _normalizeOutputPath(QDir(sCanonicalRoot).absoluteFilePath(sSafeRecordPath));
            const bool bIsFolder = archiveRecord.mapProperties.value(FPART_PROP_ISFOLDER, false).toBool() || sRecordName.endsWith(QLatin1Char('/'));

            if (!_isSafeChildPath(sResultFileName, sCanonicalRoot) || archivePathHasUnsafeLink(sCanonicalRoot, sSafeRecordPath)) {
                bResult = false;
            } else if (bIsFolder) {
                if (!transaction.ensureDirectory(sResultFileName)) {
                    reportTransactionError();
                    bResult = false;
                }
            } else if (!transaction.ensureDirectory(QFileInfo(sResultFileName).absolutePath()) || archivePathHasUnsafeLink(sCanonicalRoot, sSafeRecordPath)) {
                if (!transaction.errorString().isEmpty()) {
                    reportTransactionError();
                }
                bResult = false;
            } else {
                const QString sOutputDirectory = QFileInfo(sResultFileName).absolutePath();
                QTemporaryFile stagedFile(QDir(sOutputDirectory).filePath(QLatin1String(".xarchive-XXXXXX")));
                const bool bStageOpened = stagedFile.open();
                const bool bRecordUnpacked = bStageOpened && guardedArchive->unpackCurrent(&state, &stagedFile, pPdStruct);
                const bool bStageFlushed = bRecordUnpacked && stagedFile.flush();
                const bool bStageSeeked = bStageFlushed && stagedFile.seek(0);
                const bool bUnsafeAfter = archivePathHasUnsafeLink(sCanonicalRoot, sSafeRecordPath);
                if (!bStageOpened || !bRecordUnpacked || !guardedArchive || !isProgressAlive() || (state.nCurrentIndex != nExpectedIndex) ||
                    (state.nNumberOfRecords != nNumberOfRecords) || !isPdStructNotCanceled(pPdStruct) || !bStageFlushed || !bStageSeeked || bUnsafeAfter) {
                    bResult = false;
                } else {
                    QSaveFile outputFile(sResultFileName);
                    outputFile.setDirectWriteFallback(false);
                    bool bPublished = outputFile.open(QIODevice::WriteOnly);
                    QByteArray baBuffer(0x4000, 0);
                    qint64 nRemaining = stagedFile.size();
                    bPublished = bPublished && (nRemaining >= 0);

                    while (bPublished && (nRemaining > 0) && isProgressAlive() && isPdStructNotCanceled(pPdStruct)) {
                        const qint64 nRead = archiveReadWithBoundedProgress(&stagedFile, baBuffer.data(), (std::min)(nRemaining, (qint64)baBuffer.size()));
                        if ((nRead <= 0) || (nRead > nRemaining) || !archiveWriteAll(&outputFile, baBuffer.constData(), nRead, pPdStruct)) {
                            bPublished = false;
                            break;
                        }
                        nRemaining -= nRead;
                    }

                    if (!bPublished || (nRemaining != 0) || !isProgressAlive() || !isPdStructNotCanceled(pPdStruct) || (outputFile.error() != QFile::NoError) ||
                        archivePathHasUnsafeLink(sCanonicalRoot, sSafeRecordPath)) {
                        outputFile.cancelWriting();
                        bResult = false;
                    } else if (!transaction.prepareFile(sResultFileName)) {
                        reportTransactionError();
                        outputFile.cancelWriting();
                        bResult = false;
                    } else {
                        bResult = outputFile.commit();
                        if (bResult) {
                            bResult = transaction.markFilePublished(sResultFileName);
                            if (!bResult) {
                                reportTransactionError();
                            }
                        }
                    }
                }
            }
        }

        // A later successful record must never mask an earlier extraction,
        // path, or publication failure.
        if (!bResult) {
            break;
        }

        const qint32 nPreviousIndex = state.nCurrentIndex;
        const bool bMoved = guardedArchive->moveToNext(&state, pPdStruct);
        if (!guardedArchive || !isProgressAlive() || !isPdStructNotCanceled(pPdStruct) || (state.nCurrentIndex < 0) || (state.nNumberOfRecords != nNumberOfRecords) ||
            (state.nCurrentIndex > nNumberOfRecords)) {
            bEnumerationValid = false;
            break;
        }
        if (!bMoved) {
            if ((nPreviousIndex + 1) < nNumberOfRecords) {
                bEnumerationValid = false;
            } else if ((state.nCurrentIndex != nPreviousIndex) && (state.nCurrentIndex != nNumberOfRecords)) {
                bEnumerationValid = false;
            }
            break;
        }
        if ((state.nCurrentIndex != (nPreviousIndex + 1)) || (state.nCurrentIndex >= nNumberOfRecords)) bEnumerationValid = false;
    }

    const bool bFinished = guardedArchive && guardedArchive->finishUnpack(&state, nullptr);
    bResult = guardedArchive && bResult && bEnumerationValid && bFinished && isProgressAlive() && isPdStructNotCanceled(pPdStruct);
    if (bResult) {
        if (!transaction.commit()) {
            reportTransactionError();
            bResult = false;
            rollbackTransaction();
        } else if (!transaction.errorString().isEmpty()) {
            reportTransactionError();
        }
    } else {
        rollbackTransaction();
    }

    return bResult;
}

bool XArchive::decompressToPath(const QString &sArchiveFileName, const QString &sRecordPathName, const QString &sResultPathName, PDSTRUCT *pPdStruct)
{
    bool bResult = false;
    QPointer<XArchive> guardedArchive(this);
    const PDSTRUCTLIFETIME progressLifetime = pPdStruct ? retainPdStructLifetime(pPdStruct) : PDSTRUCTLIFETIME();
    const ArchiveProgressAlive isProgressAlive(pPdStruct, progressLifetime);

    QFile file;
    file.setFileName(sArchiveFileName);

    if (file.open(QIODevice::ReadOnly)) {
        QPointer<QIODevice> guardedOriginalDevice(guardedArchive ? guardedArchive->getDevice() : nullptr);
        if (!guardedArchive) {
            file.close();
            return false;
        }
        guardedArchive->setDevice(&file);

        if (guardedArchive && isProgressAlive() && guardedArchive->isValid(pPdStruct) && guardedArchive && isProgressAlive()) {
            QList<RECORD> listRecords = guardedArchive->getRecords(-1, pPdStruct);

            if (guardedArchive && isProgressAlive()) {
                bResult = guardedArchive->decompressToPath(&listRecords, sRecordPathName, sResultPathName, pPdStruct);
            }
        }

        // Never leave the archive pointing at this stack-local QFile.
        if (guardedArchive) {
            guardedArchive->setDevice(guardedOriginalDevice.data());
        }
        file.close();
    }

    return bResult;
}

bool XArchive::dumpToFile(const XArchive::RECORD *pRecord, const QString &sFileName, PDSTRUCT *pPdStruct)
{
    if (!pRecord) return false;
    qint32 nArchiveStreamIndex = -1;
    // Same refusal as every other coordinate-based route: an index-paired
    // record has no extent, and the two properties only markArchiveStreamRecord()
    // writes identify it even if its method field has been rewritten.
    if ((pRecord->spInfo.compressMethod == HANDLE_METHOD_ARCHIVE_STREAM) || XBinary::getArchiveStreamRecordIndex(pRecord->mapProperties, &nArchiveStreamIndex) ||
        pRecord->mapProperties.contains(XBinary::FPART_PROP_ARCHIVE_RECORD_INDEX) || pRecord->mapProperties.contains(XBinary::FPART_PROP_ARCHIVE_RECORD_TOKEN) ||
        (pRecord->nDataOffset < 0) || (pRecord->nDataSize < 0)) {
        return false;
    }
    return XBinary::dumpToFile(sFileName, pRecord->nDataOffset, pRecord->nDataSize, pPdStruct);
}

XArchive::RECORD XArchive::getArchiveRecord(const QString &sRecordFileName, QList<XArchive::RECORD> *pListRecords, PDSTRUCT *pPdStruct)
{
    XBinary::PDSTRUCT pdStructEmpty = {};

    if (!pPdStruct) {
        pdStructEmpty = XBinary::createPdStruct();
        pPdStruct = &pdStructEmpty;
    }

    RECORD result = {};

    if (!pListRecords) return result;

    qint32 nNumberOfArchives = pListRecords->count();

    for (qint32 i = 0; (i < nNumberOfArchives) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        if (pListRecords->at(i).spInfo.sRecordName == sRecordFileName) {
            result = pListRecords->at(i);
            break;
        }
    }

    return result;
}

XArchive::RECORD XArchive::getArchiveRecordByUUID(const QString &sUUID, QList<RECORD> *pListRecords, PDSTRUCT *pPdStruct)
{
    RECORD result = {};

    if (!pListRecords) return result;

    qint32 nNumberOfArchives = pListRecords->count();

    for (qint32 i = 0; (i < nNumberOfArchives) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        if (pListRecords->at(i).sUUID == sUUID) {
            result = pListRecords->at(i);
            break;
        }
    }

    return result;
}

bool XArchive::isArchiveRecordPresent(const QString &sRecordFileName, PDSTRUCT *pPdStruct)
{
    QPointer<XArchive> guardedArchive(this);
    QList<XArchive::RECORD> listRecords = guardedArchive->getRecords(-1, pPdStruct);
    if (!guardedArchive) return false;

    return XArchive::isArchiveRecordPresent(sRecordFileName, &listRecords, pPdStruct);
}

bool XArchive::isArchiveRecordPresent(const QString &sRecordFileName, QList<XArchive::RECORD> *pListRecords, PDSTRUCT *pPdStruct)
{
    return (!getArchiveRecord(sRecordFileName, pListRecords, pPdStruct).spInfo.sRecordName.isEmpty());
}

bool XArchive::isArchiveRecordPresentExp(const QString &sRecordFileName, QList<RECORD> *pListRecords, PDSTRUCT *pPdStruct)
{
    bool bResult = false;

    if (!pListRecords) return false;

    qint32 nNumberOfArchives = pListRecords->count();

    for (qint32 i = 0; (i < nNumberOfArchives) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        if (isRegExpPresent(sRecordFileName, pListRecords->at(i).spInfo.sRecordName)) {
            bResult = true;
            break;
        }
    }

    return bResult;
}

quint32 XArchive::getCompressBufferSize()
{
    return COMPRESS_BUFFERSIZE;
}

quint32 XArchive::getDecompressBufferSize()
{
    return DECOMPRESS_BUFFERSIZE;
}

void XArchive::showRecords(QList<XArchive::RECORD> *pListArchive)
{
    if (!pListArchive) return;
    qint32 nNumberOfRecords = pListArchive->count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
#ifdef QT_DEBUG
        qDebug("%s", pListArchive->at(i).spInfo.sRecordName.toUtf8().data());
#endif
    }
}

QList<XBinary::FPART_PROP> XArchive::getAvailableFPARTProperties()
{
    return XBinary::getAvailableFPARTProperties();
}

XBinary::MODE XArchive::getMode()
{
    return MODE_DATA;
}

qint32 XArchive::getType()
{
    return TYPE_ARCHIVE;
}

QString XArchive::typeIdToString(qint32 nType)
{
    QString sResult = tr("Unknown");

    switch (nType) {
        case TYPE_ARCHIVE: sResult = tr("Archive"); break;
        case TYPE_DOSEXTENDER: sResult = QString("DOS %1").arg(tr("extender")); break;
    }

    return sResult;
}

bool XArchive::isArchive()
{
    return true;
}

bool XArchive::_writeToDevice(char *pBuffer, qint32 nBufferSize, DECOMPRESSSTRUCT *pDecompressStruct)
{
    bool bResult = true;

    if (pDecompressStruct->pDestDevice) {
        char *_pOffset = pBuffer;
        qint32 _nSize = nBufferSize;
        qint64 nDecompressedSize = pDecompressStruct->nDecompressedLimit;

        if (nDecompressedSize == -1) {
            nDecompressedSize = pDecompressStruct->nOutSize + nBufferSize;
        }

        if ((pDecompressStruct->nDecompressedOffset) < (pDecompressStruct->nOutSize + nBufferSize)) {
            if ((pDecompressStruct->nDecompressedOffset < (pDecompressStruct->nOutSize + nBufferSize)) &&
                (pDecompressStruct->nDecompressedOffset > pDecompressStruct->nOutSize)) {
                _pOffset += (pDecompressStruct->nDecompressedOffset - pDecompressStruct->nOutSize);
                _nSize -= (pDecompressStruct->nDecompressedOffset - pDecompressStruct->nOutSize);
            }

            if ((pDecompressStruct->nDecompressedOffset + nDecompressedSize) < (pDecompressStruct->nOutSize + nBufferSize)) {
                _nSize -= ((pDecompressStruct->nOutSize + nBufferSize) - (pDecompressStruct->nDecompressedOffset + nDecompressedSize));
            }

            if (_nSize > 0) {
                if (archiveWriteAll(pDecompressStruct->pDestDevice, _pOffset, _nSize, nullptr)) {
                    pDecompressStruct->nDecompressedWrote += _nSize;
                } else {
                    bResult = false;
                }
            }
        }
    }

    return bResult;
}

QString XArchive::_normalizeOutputPath(const QString &sPath)
{
    return QDir::fromNativeSeparators(QDir::cleanPath(QFileInfo(sPath).absoluteFilePath()));
}

bool XArchive::_isSafeChildPath(const QString &sPath, const QString &sCanonicalRoot)
{
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
    const Qt::CaseSensitivity pathCaseSensitivity = Qt::CaseInsensitive;
#else
    const Qt::CaseSensitivity pathCaseSensitivity = Qt::CaseSensitive;
#endif

    const QString sNormalizedPath = _normalizeOutputPath(sPath);
    const QString sNormalizedRoot = _normalizeOutputPath(sCanonicalRoot);

    if (sNormalizedPath.compare(sNormalizedRoot, pathCaseSensitivity) == 0) {
        return false;
    }

    QString sExpectedRoot = sNormalizedRoot;
    if (!sExpectedRoot.endsWith(QLatin1Char('/'))) {
        sExpectedRoot.append(QLatin1Char('/'));
    }

    return sNormalizedPath.startsWith(sExpectedRoot, pathCaseSensitivity);
}

// XBinary::_MEMORY_MAP XArchive::getMemoryMap()
//{
//     _MEMORY_MAP result={};

//    qint64 nTotalSize=getSize();

//    result.nBaseAddress=_getBaseAddress();
//    result.nRawSize=nTotalSize;
//    result.nImageSize=nTotalSize;
//    result.fileType=FT_ARCHIVE;
//    result.mode=getMode();
//    result.sArch=getArch();
//    result.bIsBigEndian=isBigEndian();
//    result.sType=getTypeAsString();

//    qint32 nIndex=0;

//    QList<XArchive::RECORD> listRecords=getRecords();

//    qint32 nNumberOfRecords=listRecords.count();

//    for(qint32 i=0;i<nNumberOfRecords;i++)
//    {
//        _MEMORY_RECORD record={};
//        record.nAddress=-1;
//        record.segment=ADDRESS_SEGMENT_FLAT;
//        record.nOffset=listRecords.at(i).nDataOffset;
//        record.nSize=listRecords.at(i).nDataSize;
//        record.nIndex=nIndex++;
//        record.type=MMT_FILESEGMENT;
//        record.sName=listRecords.at(i).sFileName;

//        result.listRecords.append(record);
//    }

//    return result;
//}

QList<QString> XArchive::getSearchSignatures()
{
    return XBinary::getSearchSignatures();
}

XBinary *XArchive::createInstance(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    return XBinary::createInstance(pDevice, bIsImage, nModuleAddress);
}

QMap<XBinary::UNPACK_PROP, QVariant> XArchive::getDefaultUnpackProperties()
{
    QMap<XBinary::UNPACK_PROP, QVariant> result = XBinary::getDefaultUnpackProperties();

    return result;
}

bool XArchive::isUnpackOutputSupported(QIODevice *pDevice) const
{
    QPointer<XArchive> guardedArchive(const_cast<XArchive *>(this));
    QPointer<QIODevice> guardedDevice(pDevice);
    if (!guardedDevice) return false;
    const bool bOpen = guardedDevice->isOpen();
    if (!guardedArchive || !guardedDevice || !bOpen) return false;
    const bool bWritable = guardedDevice->isWritable();
    if (!guardedArchive || !guardedDevice || !bWritable) return false;
    const bool bSequential = guardedDevice->isSequential();
    if (!guardedArchive || !guardedDevice || bSequential) return false;
    const QIODevice::OpenMode openMode = guardedDevice->openMode();
    if (!guardedArchive || !guardedDevice || (openMode & (QIODevice::Append | QIODevice::Text))) return false;
    return guardedArchive && guardedDevice && XBinary::isResizeEnable(guardedDevice.data()) && guardedDevice;
}

static bool archiveFailPublication(QPointer<QIODevice> *pGuardedOutput, bool bOutputCleared, qint64 nOriginalPosition)
{
    if ((*pGuardedOutput) && bOutputCleared) {
        XBinary::resize(pGuardedOutput->data(), 0);
        if (*pGuardedOutput) (*pGuardedOutput)->seek(0);
    } else if ((*pGuardedOutput) && (nOriginalPosition >= 0)) {
        (*pGuardedOutput)->seek(nOriginalPosition);
    }
    return false;
}

bool XArchive::publishUnpackOutput(QIODevice *pStageDevice, QIODevice *pOutputDevice, const UNPACK_STATE *pState, PDSTRUCT *pPdStruct)
{
    QPointer<XArchive> guardedArchive(this);
    QPointer<QIODevice> guardedStage(pStageDevice);
    QPointer<QIODevice> guardedOutput(pOutputDevice);
    QPointer<QIODevice> guardedSource(guardedArchive ? guardedArchive->getDevice() : nullptr);
    if (!guardedArchive || !guardedStage || !guardedOutput || !guardedSource || !pState) {
        return false;
    }
    const bool bStageOpen = guardedStage->isOpen();
    if (!guardedArchive || !guardedStage || !bStageOpen) return false;
    const bool bStageReadable = guardedStage->isReadable();
    if (!guardedArchive || !guardedStage || !bStageReadable) return false;
    const bool bStageSequential = guardedStage->isSequential();
    if (!guardedArchive || !guardedStage || bStageSequential || !guardedArchive->isUnpackOutputSupported(guardedOutput.data()) || !guardedArchive || !guardedOutput ||
        !guardedSource || XBinary::devicesAlias(guardedStage.data(), guardedOutput.data()) || !guardedStage || !guardedOutput || !guardedSource ||
        XBinary::devicesAlias(guardedSource.data(), guardedOutput.data()) || !guardedStage || !guardedOutput || !guardedSource ||
        !XBinary::isPdStructNotCanceled(pPdStruct) || !guardedArchive || !guardedArchive->isUnpackSourceCurrent(pState, pPdStruct) || !guardedArchive || !guardedStage ||
        !guardedOutput || !guardedSource)
        return false;

    const qint64 nStageSize = guardedStage->size();
    if (!guardedArchive || !guardedStage) return false;
    const qint64 nOriginalPosition = guardedOutput->pos();
    if (!guardedArchive || !guardedOutput || (nStageSize < 0) || (nOriginalPosition < 0)) {
        return false;
    }
    const bool bStageSeeked = guardedStage->seek(0);
    if (!guardedArchive || !guardedStage || !bStageSeeked) return false;

    // Allocate every fallible private resource before touching caller-owned
    // bytes.  A decoder failure therefore leaves both bytes and cursor intact.
    QByteArray baBuffer;
    baBuffer.resize(0x10000);
    if (baBuffer.size() != 0x10000) return false;

    bool bOutputCleared = false;

    const bool bOutputSeeked = guardedOutput->seek(0);
    if (!guardedArchive || !guardedOutput || !bOutputSeeked) return false;
    if (!guardedOutput || !XBinary::resize(guardedOutput.data(), 0)) {
        if (guardedOutput) guardedOutput->seek(nOriginalPosition);
        return false;
    }
    bOutputCleared = true;
    if (!guardedOutput || !XBinary::resize(guardedOutput.data(), nStageSize) || !guardedArchive || !guardedOutput || !guardedOutput->seek(0) || !guardedArchive) {
        return archiveFailPublication(&guardedOutput, bOutputCleared, nOriginalPosition);
    }

    qint64 nPublished = 0;
    while (nPublished < nStageSize) {
        if (!guardedArchive || !guardedStage || !guardedOutput || !guardedSource || !XBinary::isPdStructNotCanceled(pPdStruct)) {
            return archiveFailPublication(&guardedOutput, bOutputCleared, nOriginalPosition);
        }
        const bool bChunkStageSeeked = guardedStage->seek(nPublished);
        if (!guardedArchive || !guardedStage || !guardedOutput || !guardedSource || !bChunkStageSeeked) {
            return archiveFailPublication(&guardedOutput, bOutputCleared, nOriginalPosition);
        }
        const qint64 nRequest = qMin<qint64>(baBuffer.size(), nStageSize - nPublished);
        const qint64 nRead = archiveReadWithBoundedProgress(guardedStage.data(), baBuffer.data(), nRequest);
        if ((nRead <= 0) || (nRead > nRequest) || !guardedArchive || !guardedOutput ||
            (guardedArchive->safeWriteData(guardedOutput.data(), nPublished, baBuffer.constData(), nRead, pPdStruct) != nRead) || !guardedArchive || !guardedOutput) {
            return archiveFailPublication(&guardedOutput, bOutputCleared, nOriginalPosition);
        }
        nPublished += nRead;
    }

    if (!guardedArchive || !guardedOutput || !guardedSource || !guardedArchive->isUnpackOutputSupported(guardedOutput.data()) || !guardedArchive || !guardedOutput ||
        !guardedSource) {
        return archiveFailPublication(&guardedOutput, bOutputCleared, nOriginalPosition);
    }
    const qint64 nPublishedSize = guardedOutput->size();
    if (!guardedArchive || !guardedOutput || !guardedSource || (nPublishedSize != nPublished)) {
        return archiveFailPublication(&guardedOutput, bOutputCleared, nOriginalPosition);
    }
    const bool bFinalSeek = guardedOutput->seek(nPublished);
    if (!guardedArchive || !guardedOutput || !guardedSource || !bFinalSeek || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return archiveFailPublication(&guardedOutput, bOutputCleared, nOriginalPosition);
    }

    // Source authentication is deliberately the final untrusted operation.
    // No output callback may mutate the source after this validation and still
    // allow success to escape.
    if (!guardedArchive->isUnpackSourceCurrent(pState, pPdStruct) || !guardedArchive || !guardedOutput || !guardedSource) {
        return archiveFailPublication(&guardedOutput, bOutputCleared, nOriginalPosition);
    }

    return true;
}

bool XArchive::unpackCurrent(UNPACK_STATE *pState, QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    UNPACK_OPERATION_GUARD operationGuard(&m_bUnpackOperationInProgress);
    QPointer<XArchive> guardedArchive(this);
    if (!operationGuard.isAcquired() || !pState || !pDevice || !guardedArchive || !guardedArchive->isUnpackOutputSupported(pDevice) || !guardedArchive ||
        (pState->nCurrentIndex < 0) || (pState->nNumberOfRecords <= 0) || (pState->nCurrentIndex >= pState->nNumberOfRecords) ||
        !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    QPointer<QIODevice> guardedOutput(pDevice);
    QPointer<QIODevice> guardedSource(guardedArchive ? guardedArchive->getDevice() : nullptr);
    if (!guardedArchive || !guardedSource || !guardedArchive->isUnpackSourceCurrent(pState, pPdStruct) || !guardedArchive || !guardedOutput ||
        XBinary::devicesAlias(guardedSource.data(), guardedOutput.data())) {
        return false;
    }

    SOURCE_DEVICE_SNAPSHOT sourceSnapshot;
    if (!guardedArchive || !guardedSource || !guardedArchive->getBoundUnpackSourceSnapshot(pState, &sourceSnapshot) || !guardedArchive) {
        return false;
    }

    UNPACK_INFO_AUTHORIZATION infoAuthorization(guardedArchive->m_pUnpackGuardState);
    if (!infoAuthorization.isAuthorized()) return false;
    const XBinary::ARCHIVERECORD archiveRecord = guardedArchive->infoCurrent(pState, pPdStruct);
    if (!guardedArchive || !guardedOutput || !guardedArchive->isUnpackOutputSupported(guardedOutput.data()) || !guardedArchive ||
        !guardedArchive->isSourceDeviceSnapshotCurrent(sourceSnapshot, guardedArchive->getDevice(), pPdStruct) || !guardedArchive ||
        !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    if (archiveRecord.mapProperties.value(XBinary::FPART_PROP_ISFOLDER).toBool()) {
        QBuffer emptyStage;
        const bool bStageOpen = emptyStage.open(QIODevice::ReadWrite);
        return guardedArchive && bStageOpen && guardedOutput && guardedArchive->isSourceDeviceSnapshotCurrent(sourceSnapshot, guardedArchive->getDevice(), pPdStruct) &&
               guardedArchive && guardedArchive->publishUnpackOutput(&emptyStage, guardedOutput.data(), pState, pPdStruct);
    }

    // Decode and authenticate into private storage first.  Caller-owned
    // destinations are published only after the complete record succeeds, so
    // decoder, CRC, cancellation and source-mutation failures cannot expose a
    // partial result.
    const bool bExpectedSizeDefined = archiveRecord.mapProperties.contains(XBinary::FPART_PROP_UNCOMPRESSEDSIZE);
    bool bExpectedSizeConverted = !bExpectedSizeDefined;
    const qint64 nExpectedSize = bExpectedSizeDefined ? archiveRecord.mapProperties.value(XBinary::FPART_PROP_UNCOMPRESSEDSIZE).toLongLong(&bExpectedSizeConverted) : 0;
    qint64 nConfiguredOutputLimit = -1;
    if (!XBinary::getUnpackOutputLimit(pState->mapUnpackProperties, &nConfiguredOutputLimit) || !bExpectedSizeConverted ||
        (bExpectedSizeDefined && ((nExpectedSize < 0) || ((nConfiguredOutputLimit >= 0) && (nExpectedSize > nConfiguredOutputLimit))))) {
        XBinary::setPdStructErrorString(pPdStruct, tr("Unpacked output exceeds the configured limit"));
        return false;
    }

    // A declared size can use the normal size-aware verification buffer.  A
    // terminator-driven member has no allocation hint, so stage it in a
    // private temporary file instead of treating the missing property as a
    // declaration of zero or allowing unbounded in-memory growth.  The shared
    // decompressor still carries mapUnpackProperties, and _writeDevice enforces
    // any configured output cap while this file grows.
    QTemporaryFile unknownSizeWorkFile;
    QIODevice *pOwnedWorkDevice = nullptr;
    QIODevice *pWorkDevice = nullptr;
    if (bExpectedSizeDefined) {
        pOwnedWorkDevice = XBinary::createFileBuffer(nExpectedSize, pPdStruct);
        pWorkDevice = pOwnedWorkDevice;
    } else if (unknownSizeWorkFile.open()) {
        pWorkDevice = &unknownSizeWorkFile;
    }
    if (!pWorkDevice) {
        XBinary::setPdStructErrorString(pPdStruct, tr("Cannot create unpack verification buffer"));
        return false;
    }

    XDecompress xDecompress;
    connect(&xDecompress, &XDecompress::errorMessage, this, &XBinary::errorMessage);
    connect(&xDecompress, &XDecompress::infoMessage, this, &XBinary::infoMessage);

    // Reset the per-entry meter and count this member. Explicit entry-count
    // policy enforces; the default remains shadow-metered.
    if (pState->spOutputBudget) {
        const QString sEntryName = archiveRecord.mapProperties.value(XBinary::FPART_PROP_ORIGINALNAME).toString();
        if (!pState->spOutputBudget->beginEntry(pState->nCurrentIndex, sEntryName)) {
            if (pState->spOutputBudget->isEnforcing()) {
                XBinary::setPdStructErrorString(pPdStruct, tr("Unpacked output exceeds the configured limit"));
                return false;
            }
            XBinary::OUTPUT_BUDGET::noteShadowRefusal(pState->spOutputBudget.data());
        }
    }

    bool bResult = xDecompress.decompressArchiveRecord(archiveRecord, guardedSource.data(), pWorkDevice, pState->mapUnpackProperties, pPdStruct, pState->spOutputBudget);
    bResult = bResult && guardedArchive && guardedOutput && guardedSource;
    if (bResult) {
        const qint64 nDecodedSize = pWorkDevice->size();
        bResult = (nDecodedSize >= 0) && (!bExpectedSizeDefined || (nDecodedSize == nExpectedSize)) &&
                  XBinary::isUnpackOutputSizeAllowed(pState->mapUnpackProperties, nDecodedSize);
    }
    if (bResult) {
        bResult = guardedArchive->isSourceDeviceSnapshotCurrent(sourceSnapshot, guardedArchive->getDevice(), pPdStruct);
    }
    if (bResult) {
        bResult = guardedArchive && guardedArchive->isUnpackSourceCurrent(pState, pPdStruct) && guardedArchive && XBinary::isPdStructNotCanceled(pPdStruct);
    }

    if (bResult && guardedArchive) {
        bResult = guardedArchive->publishUnpackOutput(pWorkDevice, guardedOutput.data(), pState, pPdStruct);
    }

    XBinary::freeFileBuffer(&pOwnedWorkDevice);

    return bResult;
}

bool XArchive::handleInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XArchive> guardedThis(this);
    bool bResult = true;

    if (!guardedThis->isInternalInfoHandled()) {
        bResult = guardedThis->XBinary::handleInternalInfo(pPdStruct);
        if (!bResult || !guardedThis) {
            return false;
        }

        void *pBaseInternalInfo = guardedThis->XBinary::getInternalInfo(pPdStruct);
        if (!guardedThis || !pBaseInternalInfo) {
            return false;
        }

        const XBinary::INTERNAL_INFO baseInternalInfo = *static_cast<XBinary::INTERNAL_INFO *>(pBaseInternalInfo);
        static_cast<XBinary::INTERNAL_INFO &>(guardedThis->m_internalInfo) = baseInternalInfo;
    }

    return bResult;
}

void *XArchive::getInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XArchive> guardedThis(this);

    if (!guardedThis->handleInternalInfo(pPdStruct) || !guardedThis) {
        return nullptr;
    }

    return &guardedThis->m_internalInfo;
}

void XArchive::setInternalInfo(void *pInternalInfo)
{
    if (pInternalInfo) {
        m_internalInfo = *static_cast<INTERNAL_INFO *>(pInternalInfo);
        XBinary::setInternalInfo(static_cast<XBinary::INTERNAL_INFO *>(&m_internalInfo));
    } else {
        m_internalInfo = INTERNAL_INFO();
        XBinary::setInternalInfo(nullptr);
    }
}
