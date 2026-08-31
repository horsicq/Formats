/* Copyright (c) 2025-2026 hors<horsicq@gmail.com>
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
#include "xiso9660.h"
#include "Algos/xstoredecoder.h"
#include "../subdevice.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QPointer>
#include <QRegularExpression>

#include <cstring>
#include <limits>
#include <new>

namespace {

const qint64 CUE_MAX_SIZE = 1024 * 1024;
const qint32 ISO_LOGICAL_SECTOR_SIZE = 2048;

struct CD_SOURCE_LAYOUT {
    bool bValid;
    bool bCue;
    QString sImageFileName;
    qint64 nSourceOffset;
    qint64 nSourceSize;
    qint32 nSectorSize;
    qint32 nPayloadOffset;
    bool bAllowTerminalZeroSector;

    CD_SOURCE_LAYOUT()
        : bValid(false), bCue(false), nSourceOffset(0), nSourceSize(0), nSectorSize(0), nPayloadOffset(0), bAllowTerminalZeroSector(false)
    {
    }
};

struct CUE_TRACK {
    QString sFileName;
    QString sFileType;
    QString sMode;
    qint64 nIndex00;
    qint64 nIndex01;

    CUE_TRACK() : nIndex00(-1), nIndex01(-1)
    {
    }
};

quint16 read16le(const char *pData)
{
    const uchar *p = reinterpret_cast<const uchar *>(pData);
    return static_cast<quint16>(p[0]) | (static_cast<quint16>(p[1]) << 8);
}

quint16 read16be(const char *pData)
{
    const uchar *p = reinterpret_cast<const uchar *>(pData);
    return (static_cast<quint16>(p[0]) << 8) | static_cast<quint16>(p[1]);
}

quint32 read32le(const char *pData)
{
    const uchar *p = reinterpret_cast<const uchar *>(pData);
    return static_cast<quint32>(p[0]) | (static_cast<quint32>(p[1]) << 8) | (static_cast<quint32>(p[2]) << 16) | (static_cast<quint32>(p[3]) << 24);
}

quint32 read32be(const char *pData)
{
    const uchar *p = reinterpret_cast<const uchar *>(pData);
    return (static_cast<quint32>(p[0]) << 24) | (static_cast<quint32>(p[1]) << 16) | (static_cast<quint32>(p[2]) << 8) | static_cast<quint32>(p[3]);
}

bool readDeviceAt(QIODevice *pDevice, qint64 nOffset, char *pData, qint64 nSize)
{
    QPointer<QIODevice> guardedDevice(pDevice);
    if (!guardedDevice || (nOffset < 0) || (nSize < 0) || ((nSize > 0) && !pData)) {
        return false;
    }
    const bool bOpen = guardedDevice->isOpen();
    if (!guardedDevice || !bOpen) return false;
    const bool bReadable = guardedDevice->isReadable();
    if (!guardedDevice || !bReadable) return false;
    const bool bSequential = guardedDevice->isSequential();
    if (!guardedDevice || bSequential) return false;
    const qint64 nOldPosition = guardedDevice->pos();
    if (!guardedDevice || (nOldPosition < 0)) return false;
    const bool bInitialSeek = guardedDevice->seek(nOffset);
    if (!guardedDevice || !bInitialSeek) return false;

    qint64 nReadTotal = 0;
    while (guardedDevice && (nReadTotal < nSize)) {
        const bool bSeek = guardedDevice->seek(nOffset + nReadTotal);
        if (!guardedDevice || !bSeek) break;
        const qint64 nRead = guardedDevice->read(pData + nReadTotal, nSize - nReadTotal);
        if (!guardedDevice || (nRead <= 0) || (nRead > (nSize - nReadTotal))) {
            break;
        }
        nReadTotal += nRead;
    }

    if (!guardedDevice) return false;
    const bool bRestored = guardedDevice->seek(nOldPosition);
    return guardedDevice && bRestored && (nReadTotal == nSize);
}

QByteArray readDeviceAt(QIODevice *pDevice, qint64 nOffset, qint64 nSize)
{
    if ((nSize < 0) || (nSize > (std::numeric_limits<int>::max)())) return QByteArray();
    QByteArray baResult(static_cast<int>(nSize), 0);
    if (!readDeviceAt(pDevice, nOffset, baResult.data(), nSize)) baResult.clear();
    return baResult;
}

bool hasRawSync(const char *pSector)
{
    if (!pSector || (static_cast<uchar>(pSector[0]) != 0) || (static_cast<uchar>(pSector[11]) != 0)) {
        return false;
    }
    for (qint32 i = 1; i < 11; ++i) {
        if (static_cast<uchar>(pSector[i]) != 0xff) return false;
    }
    return true;
}

bool isAllZeroPhysicalSector(const char *pSector, qint32 nSectorSize)
{
    if (!pSector || (nSectorSize <= 0)) return false;

    for (qint32 i = 0; i < nSectorSize; ++i) {
        if (static_cast<uchar>(pSector[i]) != 0) return false;
    }

    return true;
}

quint32 updateCdRomEdc(quint32 nEdc, uchar nByte)
{
    nEdc ^= nByte;
    for (qint32 i = 0; i < 8; ++i) {
        nEdc = (nEdc >> 1) ^ ((nEdc & 1U) ? 0xd8018001U : 0U);
    }
    return nEdc;
}

bool hasRecoverableMode1SyncDamage(const char *pSector)
{
    if (!pSector || (static_cast<uchar>(pSector[15]) != 1)) return false;

    // MODE1 EDC covers bytes 0..2063.  Recompute it with only the canonical
    // sync pattern substituted.  A match proves that the address, mode and
    // 2048-byte payload are intact and that damage is confined to sync bytes.
    quint32 nEdc = 0;
    for (qint32 i = 0; i < 2064; ++i) {
        uchar nByte = static_cast<uchar>(pSector[i]);
        if ((i == 0) || (i == 11)) {
            nByte = 0;
        } else if ((i >= 1) && (i <= 10)) {
            nByte = 0xff;
        }
        nEdc = updateCdRomEdc(nEdc, nByte);
    }

    return nEdc == read32le(pSector + 2064);
}

bool isPhysicalSectorValid(const char *pSector, qint32 nSectorSize, qint32 nPayloadOffset, bool bAllowTerminalZeroSector)
{
    if (!pSector || (nSectorSize < ISO_LOGICAL_SECTOR_SIZE) || (nPayloadOffset < 0) || (nPayloadOffset > nSectorSize - ISO_LOGICAL_SECTOR_SIZE)) {
        return false;
    }

    if ((nSectorSize == 2352) || (nSectorSize == 2448)) {
        if (!hasRawSync(pSector)) {
            if (nPayloadOffset != 16) return false;
            if (bAllowTerminalZeroSector && isAllZeroPhysicalSector(pSector, nSectorSize)) return true;
            return hasRecoverableMode1SyncDamage(pSector);
        }
        const uchar nMode = static_cast<uchar>(pSector[15]);
        if (nPayloadOffset == 16) return nMode == 1;
        if (nPayloadOffset != 24 || nMode != 2) return false;

        // CD-ROM XA Form 2 sectors carry 2324 bytes and cannot be projected
        // into an ISO9660 2048-byte logical block without corrupting offsets.
        // Require both submode copies to identify Form 1.  Do not require all
        // duplicated subheader bytes to match: surviving historical images
        // can contain a damaged file/channel copy while their 2048-byte data
        // payload remains intact (for example Abe's Exoddus CD 2).
        return ((static_cast<uchar>(pSector[18]) & 0x20) == 0) && ((static_cast<uchar>(pSector[22]) & 0x20) == 0);
    }

    if ((nSectorSize == 2336) && (nPayloadOffset == 8)) {
        return ((static_cast<uchar>(pSector[2]) & 0x20) == 0) && ((static_cast<uchar>(pSector[6]) & 0x20) == 0);
    }

    return (nSectorSize == ISO_LOGICAL_SECTOR_SIZE) && (nPayloadOffset == 0);
}

qint64 finishLogicalSectorRead(const QPointer<QIODevice> &guardedSource, qint64 nOriginalSourcePosition, qint64 nResult)
{
    if (!guardedSource) return -1;
    const bool bRestored = guardedSource->seek(nOriginalSourcePosition);
    return (guardedSource && bRestored) ? nResult : -1;
}

class CDLogicalSectorDevice : public SubDevice {
public:
    CDLogicalSectorDevice(QIODevice *pDevice, qint64 nOffset, qint64 nSourceSize, qint32 nSectorSize, qint32 nPayloadOffset, bool bAllowTerminalZeroSector)
        : SubDevice(pDevice, nOffset, nSourceSize),
          m_nSourceSize(nSourceSize),
          m_nSectorSize(nSectorSize),
          m_nPayloadOffset(nPayloadOffset),
          m_nLogicalSize(0),
          m_bAllowTerminalZeroSector(bAllowTerminalZeroSector)
    {
        if ((m_nSourceSize >= 0) && (m_nSectorSize > 0) && (m_nPayloadOffset >= 0) && (m_nPayloadOffset <= m_nSectorSize - ISO_LOGICAL_SECTOR_SIZE)) {
            const qint64 nSectors = m_nSourceSize / m_nSectorSize;
            if (nSectors <= (std::numeric_limits<qint64>::max)() / ISO_LOGICAL_SECTOR_SIZE) {
                m_nLogicalSize = nSectors * ISO_LOGICAL_SECTOR_SIZE;
            }
        }
    }

    qint64 size() const override
    {
        return m_nLogicalSize;
    }

    bool open(OpenMode mode) override
    {
        QPointer<CDLogicalSectorDevice> guardedThis(this);
        QPointer<QIODevice> guardedSource(getOrigDevice());
        if (!guardedThis || !guardedSource || ((mode & QIODevice::ReadOnly) == 0) || ((mode & QIODevice::WriteOnly) != 0)) {
            return false;
        }
        const qint64 nOriginalSourcePosition = guardedSource->pos();
        if (!guardedThis || !guardedSource || (nOriginalSourcePosition < 0)) {
            return false;
        }
        const bool bOpened = SubDevice::open(mode);
        if (!guardedSource) return false;
        const bool bRestored = guardedSource->seek(nOriginalSourcePosition);
        if (!guardedThis || !guardedSource || !bRestored) {
            if (guardedThis) guardedThis->close();
            return false;
        }
        return bOpened;
    }

    bool seek(qint64 nPosition) override
    {
        if (!isOpen() || (nPosition < 0) || (nPosition > m_nLogicalSize)) {
            return false;
        }
        return XIODevice::seek(nPosition);
    }

protected:
    qint64 readData(char *pData, qint64 nMaximumSize) override
    {
        QPointer<CDLogicalSectorDevice> guardedThis(this);
        QPointer<QIODevice> guardedSource(getOrigDevice());
        if (!guardedThis || !guardedSource || (nMaximumSize < 0) || ((nMaximumSize > 0) && !pData)) {
            return -1;
        }
        const qint64 nStart = guardedThis->pos();
        if (!guardedThis || !guardedSource || (nStart < 0)) return -1;
        const bool bOpen = guardedThis->isOpen();
        if (!guardedThis || !guardedSource || !bOpen) return -1;
        const bool bReadable = guardedThis->isReadable();
        if (!guardedThis || !guardedSource || !bReadable) return -1;
        const bool bSourceOpen = guardedSource->isOpen();
        if (!guardedThis || !guardedSource || !bSourceOpen) return -1;
        const bool bSourceReadable = guardedSource->isReadable();
        if (!guardedThis || !guardedSource || !bSourceReadable) return -1;
        const bool bSourceSequential = guardedSource->isSequential();
        if (!guardedThis || !guardedSource || bSourceSequential || (nStart > guardedThis->m_nLogicalSize)) {
            return -1;
        }
        const qint64 nOriginalSourcePosition = guardedSource->pos();
        if (!guardedThis || !guardedSource || (nOriginalSourcePosition < 0)) {
            return -1;
        }
        qint64 nRemaining = qMin(nMaximumSize, guardedThis->m_nLogicalSize - nStart);
        qint64 nProduced = 0;
        const qint64 nBaseOffset = static_cast<qint64>(guardedThis->getInitLocation());
        const qint64 nPhysicalSectorCount = guardedThis->m_nSourceSize / guardedThis->m_nSectorSize;
        if (!guardedThis || !guardedSource) return finishLogicalSectorRead(guardedSource, nOriginalSourcePosition, -1);

        while (guardedThis && guardedSource && (nRemaining > 0)) {
            const qint64 nLogicalPosition = nStart + nProduced;
            const qint64 nSector = nLogicalPosition / ISO_LOGICAL_SECTOR_SIZE;
            const qint32 nInside = static_cast<qint32>(nLogicalPosition % ISO_LOGICAL_SECTOR_SIZE);

            qint64 nSectorCount = 1;
            if ((nInside == 0) && (nRemaining >= ISO_LOGICAL_SECTOR_SIZE)) {
                nSectorCount = qMin<qint64>(256, nRemaining / ISO_LOGICAL_SECTOR_SIZE);
            }
            if (nSector > ((std::numeric_limits<qint64>::max)() - nBaseOffset) / guardedThis->m_nSectorSize) {
                return finishLogicalSectorRead(guardedSource, nOriginalSourcePosition, nProduced ? nProduced : -1);
            }
            const qint64 nPhysicalOffset = nBaseOffset + nSector * guardedThis->m_nSectorSize;
            if (nSectorCount > (std::numeric_limits<int>::max)() / guardedThis->m_nSectorSize) {
                return finishLogicalSectorRead(guardedSource, nOriginalSourcePosition, nProduced ? nProduced : -1);
            }
            const qint64 nPhysicalSize = nSectorCount * guardedThis->m_nSectorSize;
            QByteArray baSectors(static_cast<int>(nPhysicalSize), 0);

            const bool bPositioned = guardedSource->seek(nPhysicalOffset);
            if (!guardedThis || !guardedSource || !bPositioned) {
                return finishLogicalSectorRead(guardedSource, nOriginalSourcePosition, nProduced ? nProduced : -1);
            }
            qint64 nReadTotal = 0;
            while (guardedThis && guardedSource && (nReadTotal < nPhysicalSize)) {
                const bool bSeek = guardedSource->seek(nPhysicalOffset + nReadTotal);
                if (!guardedThis || !guardedSource || !bSeek) {
                    return finishLogicalSectorRead(guardedSource, nOriginalSourcePosition, nProduced ? nProduced : -1);
                }
                const qint64 nRead = guardedSource->read(baSectors.data() + nReadTotal, nPhysicalSize - nReadTotal);
                if (!guardedThis || !guardedSource || (nRead <= 0) || (nRead > nPhysicalSize - nReadTotal)) {
                    return finishLogicalSectorRead(guardedSource, nOriginalSourcePosition, nProduced ? nProduced : -1);
                }
                nReadTotal += nRead;
            }

            if ((nInside != 0) || (nRemaining < ISO_LOGICAL_SECTOR_SIZE)) {
                const bool bTerminalSector = guardedThis->m_bAllowTerminalZeroSector && (nSector == (nPhysicalSectorCount - 1));
                if (!isPhysicalSectorValid(baSectors.constData(), guardedThis->m_nSectorSize, guardedThis->m_nPayloadOffset, bTerminalSector)) {
                    return finishLogicalSectorRead(guardedSource, nOriginalSourcePosition, nProduced ? nProduced : -1);
                }
                const qint64 nCopy = qMin<qint64>(nRemaining, ISO_LOGICAL_SECTOR_SIZE - nInside);
                memcpy(pData + nProduced, baSectors.constData() + guardedThis->m_nPayloadOffset + nInside, static_cast<size_t>(nCopy));
                nProduced += nCopy;
                nRemaining -= nCopy;
                continue;
            }

            for (qint64 i = 0; i < nSectorCount; ++i) {
                const char *pSector = baSectors.constData() + i * guardedThis->m_nSectorSize;
                const bool bTerminalSector = guardedThis->m_bAllowTerminalZeroSector && ((nSector + i) == (nPhysicalSectorCount - 1));
                if (!isPhysicalSectorValid(pSector, guardedThis->m_nSectorSize, guardedThis->m_nPayloadOffset, bTerminalSector)) {
                    return finishLogicalSectorRead(guardedSource, nOriginalSourcePosition, nProduced ? nProduced : -1);
                }
                memcpy(pData + nProduced, pSector + guardedThis->m_nPayloadOffset, ISO_LOGICAL_SECTOR_SIZE);
                nProduced += ISO_LOGICAL_SECTOR_SIZE;
                nRemaining -= ISO_LOGICAL_SECTOR_SIZE;
            }
        }

        return finishLogicalSectorRead(guardedSource, nOriginalSourcePosition, (guardedThis && guardedSource) ? nProduced : -1);
    }

private:
    qint64 m_nSourceSize;
    qint32 m_nSectorSize;
    qint32 m_nPayloadOffset;
    qint64 m_nLogicalSize;
    bool m_bAllowTerminalZeroSector;
};

struct ISO_DESCRIPTOR_CHOICE {
    bool bValid;
    bool bJoliet;
    qint32 nJolietLevel;
    qint64 nOffset;
    quint64 nVolumeSize;

    ISO_DESCRIPTOR_CHOICE() : bValid(false), bJoliet(false), nJolietLevel(0), nOffset(-1), nVolumeSize(0)
    {
    }
};

bool isBoundedRange(quint64 nOffset, quint64 nSize, quint64 nLimit)
{
    return (nOffset <= nLimit) && (nSize <= nLimit - nOffset);
}

bool validateSpecialDirectoryRecord(const QByteArray &baDirectory, qint32 nOffset, quint8 nIdentifier, quint64 nVolumeSize, qint32 nBlockSize)
{
    if ((nOffset < 0) || (nOffset > baDirectory.size() - 34)) {
        return false;
    }
    const char *pRecord = baDirectory.constData() + nOffset;
    const quint8 nRecordLength = static_cast<quint8>(pRecord[0]);
    const quint8 nNameLength = static_cast<quint8>(pRecord[32]);
    const qint32 nMinimumLength = 33 + nNameLength + ((nNameLength % 2) == 0 ? 1 : 0);
    if ((nRecordLength < 34) || (nRecordLength > baDirectory.size() - nOffset) || (nMinimumLength > nRecordLength) || (nNameLength != 1) ||
        (static_cast<quint8>(pRecord[33]) != nIdentifier) || ((static_cast<quint8>(pRecord[25]) & 0x02) == 0)) {
        return false;
    }

    const quint32 nExtentLe = read32le(pRecord + 2);
    const quint32 nExtentBe = read32be(pRecord + 6);
    const quint32 nDataSizeLe = read32le(pRecord + 10);
    const quint32 nDataSizeBe = read32be(pRecord + 14);
    const quint16 nSequenceLe = read16le(pRecord + 28);
    const quint16 nSequenceBe = read16be(pRecord + 30);
    if ((nExtentLe != nExtentBe) || (nDataSizeLe == 0) || (nDataSizeLe != nDataSizeBe) || (nSequenceLe == 0) || (nSequenceLe != nSequenceBe)) {
        return false;
    }

    const quint64 nDataBlock = static_cast<quint64>(nExtentLe) + static_cast<quint8>(pRecord[1]);
    const quint64 nDataOffset = nDataBlock * nBlockSize;
    return isBoundedRange(nDataOffset, nDataSizeLe, nVolumeSize);
}

bool validateVolumeDescriptor(QIODevice *pDevice, const QByteArray &baDescriptor)
{
    QPointer<QIODevice> guardedDevice(pDevice);
    if (!guardedDevice || (baDescriptor.size() != ISO_LOGICAL_SECTOR_SIZE)) {
        return false;
    }

    const quint16 nBlockSizeLe = read16le(baDescriptor.constData() + 128);
    const quint16 nBlockSizeBe = read16be(baDescriptor.constData() + 130);
    const quint32 nVolumeBlocksLe = read32le(baDescriptor.constData() + 80);
    const quint32 nVolumeBlocksBe = read32be(baDescriptor.constData() + 84);
    if ((nBlockSizeLe < 512) || (nBlockSizeLe > 8192) || (nBlockSizeLe != nBlockSizeBe) || (nVolumeBlocksLe == 0) || (nVolumeBlocksLe != nVolumeBlocksBe)) {
        return false;
    }

    const quint64 nVolumeSize = static_cast<quint64>(nVolumeBlocksLe) * nBlockSizeLe;
    const qint64 nDeviceSize = guardedDevice->size();
    if (!guardedDevice || (nDeviceSize < 0) || (nVolumeSize > static_cast<quint64>(nDeviceSize))) {
        return false;
    }

    const char *pRoot = baDescriptor.constData() + 156;
    const quint8 nRootLength = static_cast<quint8>(pRoot[0]);
    const quint8 nRootNameLength = static_cast<quint8>(pRoot[32]);
    const quint32 nRootExtentLe = read32le(pRoot + 2);
    const quint32 nRootExtentBe = read32be(pRoot + 6);
    const quint32 nRootSizeLe = read32le(pRoot + 10);
    const quint32 nRootSizeBe = read32be(pRoot + 14);
    const quint16 nRootSequenceLe = read16le(pRoot + 28);
    const quint16 nRootSequenceBe = read16be(pRoot + 30);
    if ((nRootLength < 34) || (156 + nRootLength > 2048) || (nRootNameLength != 1) || (static_cast<quint8>(pRoot[33]) != 0) ||
        ((static_cast<quint8>(pRoot[25]) & 0x02) == 0) || (nRootExtentLe != nRootExtentBe) || (nRootSizeLe == 0) || (nRootSizeLe != nRootSizeBe) ||
        (nRootSequenceLe == 0) || (nRootSequenceLe != nRootSequenceBe)) {
        return false;
    }

    const quint64 nRootBlock = static_cast<quint64>(nRootExtentLe) + static_cast<quint8>(pRoot[1]);
    const quint64 nRootOffset = nRootBlock * nBlockSizeLe;
    if (!isBoundedRange(nRootOffset, nRootSizeLe, nVolumeSize)) return false;

    const qint64 nProbeSize = qMin<qint64>(nRootSizeLe, ISO_LOGICAL_SECTOR_SIZE);
    const QByteArray baRoot = readDeviceAt(guardedDevice.data(), static_cast<qint64>(nRootOffset), nProbeSize);
    if (!guardedDevice || (baRoot.size() != nProbeSize) || !validateSpecialDirectoryRecord(baRoot, 0, 0, nVolumeSize, nBlockSizeLe)) {
        return false;
    }
    const qint32 nParentOffset = static_cast<quint8>(baRoot.at(0));
    return validateSpecialDirectoryRecord(baRoot, nParentOffset, 1, nVolumeSize, nBlockSizeLe);
}

qint32 jolietLevel(const QByteArray &baDescriptor)
{
    if ((baDescriptor.size() != ISO_LOGICAL_SECTOR_SIZE) || (static_cast<quint8>(baDescriptor.at(0)) != 2) || ((static_cast<quint8>(baDescriptor.at(7)) & 0x01) != 0)) {
        return 0;
    }
    const QByteArray baEscape = baDescriptor.mid(88, 3);
    if (baEscape == QByteArrayLiteral("%/@")) return 1;
    if (baEscape == QByteArrayLiteral("%/C")) return 2;
    if (baEscape == QByteArrayLiteral("%/E")) return 3;
    return 0;
}

bool selectIsoDescriptor(QIODevice *pDevice, ISO_DESCRIPTOR_CHOICE *pChoice)
{
    if (pChoice) *pChoice = ISO_DESCRIPTOR_CHOICE();
    QPointer<QIODevice> guardedDevice(pDevice);
    if (!guardedDevice || !pChoice) return false;
    const bool bOpen = guardedDevice->isOpen();
    if (!guardedDevice || !bOpen) return false;
    const bool bReadable = guardedDevice->isReadable();
    if (!guardedDevice || !bReadable) return false;
    const bool bSequential = guardedDevice->isSequential();
    if (!guardedDevice || bSequential) return false;
    const qint64 nDeviceSize = guardedDevice->size();
    if (!guardedDevice || (nDeviceSize < 17LL * ISO_LOGICAL_SECTOR_SIZE)) {
        return false;
    }

    ISO_DESCRIPTOR_CHOICE primaryChoice;
    ISO_DESCRIPTOR_CHOICE jolietChoice;
    bool bFoundTerminator = false;
    const qint64 nSectorCount = nDeviceSize / ISO_LOGICAL_SECTOR_SIZE;
    const qint64 nScanEnd = qMin<qint64>(nSectorCount, 16 + 256);
    for (qint64 nSector = 16; nSector < nScanEnd; ++nSector) {
        const qint64 nOffset = nSector * ISO_LOGICAL_SECTOR_SIZE;
        const QByteArray baDescriptor = readDeviceAt(guardedDevice.data(), nOffset, ISO_LOGICAL_SECTOR_SIZE);
        if (!guardedDevice) return false;
        if (baDescriptor.size() != ISO_LOGICAL_SECTOR_SIZE) return false;
        if ((baDescriptor.mid(1, 5) != QByteArrayLiteral("CD001")) || (static_cast<quint8>(baDescriptor.at(6)) != 1)) {
            break;
        }

        const quint8 nType = static_cast<quint8>(baDescriptor.at(0));
        if (nType == 255) {
            bFoundTerminator = true;
            break;
        }
        const qint32 nLevel = jolietLevel(baDescriptor);
        if ((nType != 1) && (nLevel == 0)) continue;
        if (!validateVolumeDescriptor(guardedDevice.data(), baDescriptor)) continue;
        if (!guardedDevice) return false;

        if (nLevel > 0) {
            if (!jolietChoice.bValid || (nLevel >= jolietChoice.nJolietLevel)) {
                jolietChoice.bValid = true;
                jolietChoice.bJoliet = true;
                jolietChoice.nJolietLevel = nLevel;
                jolietChoice.nOffset = nOffset;
                jolietChoice.nVolumeSize = static_cast<quint64>(read32le(baDescriptor.constData() + 80)) * read16le(baDescriptor.constData() + 128);
            }
        } else if (!primaryChoice.bValid) {
            primaryChoice.bValid = true;
            primaryChoice.nOffset = nOffset;
            primaryChoice.nVolumeSize = static_cast<quint64>(read32le(baDescriptor.constData() + 80)) * read16le(baDescriptor.constData() + 128);
        }
    }

    if (!bFoundTerminator) return false;
    *pChoice = jolietChoice.bValid ? jolietChoice : primaryChoice;
    return pChoice->bValid;
}

bool isLogicalIsoValid(QIODevice *pDevice, quint64 *pnVolumeSize = nullptr)
{
    ISO_DESCRIPTOR_CHOICE choice;
    const bool bResult = selectIsoDescriptor(pDevice, &choice);
    if (pnVolumeSize) *pnVolumeSize = bResult ? choice.nVolumeSize : 0;
    return bResult;
}

bool decodeUcs2Be(const QByteArray &baValue, QString *pResult)
{
    if (!pResult || ((baValue.size() % 2) != 0)) return false;
    pResult->clear();
    pResult->reserve(baValue.size() / 2);
    for (qint32 i = 0; i < baValue.size(); i += 2) {
        const quint16 nCodeUnit = (static_cast<quint16>(static_cast<quint8>(baValue.at(i))) << 8) | static_cast<quint8>(baValue.at(i + 1));
        if (nCodeUnit == 0) break;
        pResult->append(QChar(nCodeUnit));
    }
    return true;
}

QString decodeDescriptorText(const QByteArray &baValue, bool bJoliet)
{
    if (!bJoliet) return QString::fromLatin1(baValue).trimmed();
    QByteArray baEvenValue = baValue;
    if ((baEvenValue.size() % 2) != 0) baEvenValue.chop(1);
    QString sResult;
    if (!decodeUcs2Be(baEvenValue, &sResult)) return QString();
    return sResult.trimmed();
}

bool modeToLayout(const QString &sMode, qint32 *pnSectorSize, qint32 *pnPayloadOffset)
{
    if (!pnSectorSize || !pnPayloadOffset) return false;
    const QString sUpper = sMode.toUpper();
    if (sUpper == QStringLiteral("MODE1/2048")) {
        *pnSectorSize = 2048;
        *pnPayloadOffset = 0;
        return true;
    }
    if (sUpper == QStringLiteral("MODE1/2352")) {
        *pnSectorSize = 2352;
        *pnPayloadOffset = 16;
        return true;
    }
    if (sUpper == QStringLiteral("MODE2/2336")) {
        *pnSectorSize = 2336;
        *pnPayloadOffset = 8;
        return true;
    }
    if (sUpper == QStringLiteral("MODE2/2352")) {
        *pnSectorSize = 2352;
        *pnPayloadOffset = 24;
        return true;
    }
    return false;
}

qint64 cueFrames(const QRegularExpressionMatch &match)
{
    bool bMinute = false;
    bool bSecond = false;
    bool bFrame = false;
    const qint64 nMinute = match.captured(2).toLongLong(&bMinute);
    const qint64 nSecond = match.captured(3).toLongLong(&bSecond);
    const qint64 nFrame = match.captured(4).toLongLong(&bFrame);
    if (!bMinute || !bSecond || !bFrame || (nMinute < 0) || (nSecond < 0) || (nSecond >= 60) || (nFrame < 0) || (nFrame >= 75) ||
        (nMinute > ((std::numeric_limits<qint64>::max)() - nSecond) / 60)) {
        return -1;
    }
    const qint64 nSeconds = nMinute * 60 + nSecond;
    if (nSeconds > ((std::numeric_limits<qint64>::max)() - nFrame) / 75) {
        return -1;
    }
    return nSeconds * 75 + nFrame;
}

bool parseCue(QIODevice *pDevice, QList<CUE_TRACK> *pTracks)
{
    QPointer<QIODevice> guardedDevice(pDevice);
    if (!guardedDevice || !pTracks) return false;
    const bool bOpen = guardedDevice->isOpen();
    if (!guardedDevice || !bOpen) return false;
    const bool bReadable = guardedDevice->isReadable();
    if (!guardedDevice || !bReadable) return false;
    const bool bSequential = guardedDevice->isSequential();
    if (!guardedDevice || bSequential) return false;
    const qint64 nCueSize = guardedDevice->size();
    if (!guardedDevice || (nCueSize <= 0) || (nCueSize > CUE_MAX_SIZE)) {
        return false;
    }
    pTracks->clear();
    const QByteArray baCue = readDeviceAt(guardedDevice.data(), 0, nCueSize);
    if (!guardedDevice || baCue.isEmpty() || baCue.contains('\0')) return false;

    const QRegularExpression rxFile(QStringLiteral("^\\s*FILE\\s+(?:\\\"([^\\\"]+)\\\"|(\\S+))\\s+(\\S+)\\s*$"), QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression rxTrack(QStringLiteral("^\\s*TRACK\\s+(\\d{1,3})\\s+(\\S+)\\s*$"), QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression rxIndex(QStringLiteral("^\\s*INDEX\\s+(\\d{1,2})\\s+(\\d+):(\\d+):(\\d+)\\s*$"), QRegularExpression::CaseInsensitiveOption);

    QString sCurrentFile;
    QString sCurrentFileType;
    const QStringList listLines = QString::fromLatin1(baCue).split(QRegularExpression(QStringLiteral("[\\r\\n]+")), Qt::SkipEmptyParts);
    for (const QString &sLine : listLines) {
        QRegularExpressionMatch match = rxFile.match(sLine);
        if (match.hasMatch()) {
            sCurrentFile = match.captured(1).isEmpty() ? match.captured(2) : match.captured(1);
            sCurrentFileType = match.captured(3).toUpper();
            continue;
        }
        if (QRegularExpression(QStringLiteral("^\\s*FILE(?:\\s|$)"), QRegularExpression::CaseInsensitiveOption).match(sLine).hasMatch()) {
            return false;
        }
        match = rxTrack.match(sLine);
        if (match.hasMatch()) {
            if (sCurrentFile.isEmpty()) return false;
            CUE_TRACK track;
            track.sFileName = sCurrentFile;
            track.sFileType = sCurrentFileType;
            track.sMode = match.captured(2).toUpper();
            pTracks->append(track);
            continue;
        }
        if (QRegularExpression(QStringLiteral("^\\s*TRACK(?:\\s|$)"), QRegularExpression::CaseInsensitiveOption).match(sLine).hasMatch()) {
            return false;
        }
        match = rxIndex.match(sLine);
        if (match.hasMatch()) {
            if (pTracks->isEmpty()) return false;
            bool bIndex = false;
            const qint32 nIndex = match.captured(1).toInt(&bIndex);
            const qint64 nFrames = cueFrames(match);
            if (!bIndex || (nFrames < 0)) return false;
            if (nIndex == 0) pTracks->last().nIndex00 = nFrames;
            if (nIndex == 1) pTracks->last().nIndex01 = nFrames;
            continue;
        }
        if (QRegularExpression(QStringLiteral("^\\s*INDEX(?:\\s|$)"), QRegularExpression::CaseInsensitiveOption).match(sLine).hasMatch()) {
            return false;
        }
    }

    return !pTracks->isEmpty();
}

Qt::CaseSensitivity pathCaseSensitivity()
{
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
    return Qt::CaseInsensitive;
#else
    return Qt::CaseSensitive;
#endif
}

bool isSafeCueCandidate(const QString &sCueDirectory, const QFileInfo &candidate)
{
    if (!candidate.exists() || !candidate.isFile() || candidate.isSymLink()) {
        return false;
    }
    const QString sRoot = QFileInfo(sCueDirectory).canonicalFilePath();
    const QString sCandidate = candidate.canonicalFilePath();
    if (sRoot.isEmpty() || sCandidate.isEmpty()) return false;
    QString sPrefix = QDir::fromNativeSeparators(sRoot);
    if (!sPrefix.endsWith(QLatin1Char('/'))) sPrefix += QLatin1Char('/');
    const QString sNormalized = QDir::fromNativeSeparators(sCandidate);
    return sNormalized.startsWith(sPrefix, pathCaseSensitivity());
}

QString resolveSiblingCaseInsensitive(const QString &sDirectory, const QString &sFileName)
{
    if (sFileName.isEmpty() || sFileName.contains(QLatin1Char('/')) || sFileName.contains(QLatin1Char('\\'))) {
        return QString();
    }

    const QDir dir(sDirectory);
    const QFileInfo exact(dir.filePath(sFileName));
    if (isSafeCueCandidate(sDirectory, exact)) return exact.canonicalFilePath();

    QString sResult;
    const QFileInfoList listFiles = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
    for (const QFileInfo &fileInfo : listFiles) {
        if ((fileInfo.fileName().compare(sFileName, Qt::CaseInsensitive) == 0) && isSafeCueCandidate(sDirectory, fileInfo)) {
            if (!sResult.isEmpty() && (sResult.compare(fileInfo.canonicalFilePath(), pathCaseSensitivity()) != 0)) {
                return QString();
            }
            sResult = fileInfo.canonicalFilePath();
        }
    }
    return sResult;
}

QString resolveCueImage(const QString &sCueFileName, const QString &sReference)
{
    const QFileInfo cueInfo(sCueFileName);
    const QString sDirectory = cueInfo.absolutePath();
    QString sNormalizedReference = sReference;
    sNormalizedReference.replace(QLatin1Char('\\'), QLatin1Char('/'));

    // A genuine relative reference may name a child path, but never escape
    // the directory containing the CUE sheet.
    const bool bDriveRooted = QRegularExpression(QStringLiteral("^[A-Za-z]:/")).match(sNormalizedReference).hasMatch();
    if (!bDriveRooted && !QDir::isAbsolutePath(sNormalizedReference)) {
        const QString sClean = QDir::cleanPath(sNormalizedReference);
        if ((sClean != QStringLiteral("..")) && !sClean.startsWith(QStringLiteral("../"))) {
            const QFileInfo relativeCandidate(QDir(sDirectory).absoluteFilePath(sClean));
            if (isSafeCueCandidate(sDirectory, relativeCandidate)) return relativeCandidate.canonicalFilePath();
        }
    }

    // Old mastering tools often saved the operator's drive path. Resolve its
    // basename only beside the CUE; never follow the stale absolute location.
    const QString sBaseName = QFileInfo(sNormalizedReference).fileName();
    QString sResult = resolveSiblingCaseInsensitive(sDirectory, sBaseName);
    if (!sResult.isEmpty()) return sResult;

    // A small number of historical sheets contain a completely wrong FILE
    // token. A same-directory image matching the CUE stem is deterministic.
    const QString sStem = cueInfo.completeBaseName();
    QStringList listFallbacks;
    listFallbacks << sStem + QStringLiteral(".bin") << sStem + QStringLiteral(".raw");
    QString sFallback;
    for (const QString &sName : listFallbacks) {
        const QString sCandidate = resolveSiblingCaseInsensitive(sDirectory, sName);
        if (sCandidate.isEmpty()) continue;
        if (!sFallback.isEmpty() && (sFallback.compare(sCandidate, pathCaseSensitivity()) != 0)) {
            return QString();
        }
        sFallback = sCandidate;
    }
    return sFallback;
}

bool detectCueLayout(QIODevice *pCueDevice, CD_SOURCE_LAYOUT *pLayout)
{
    QPointer<QIODevice> guardedCueDevice(pCueDevice);
    if (!guardedCueDevice || !pLayout) return false;
    const QString sCueFileName = XBinary::getDeviceFileName(guardedCueDevice.data());
    if (!guardedCueDevice || sCueFileName.isEmpty() || (QFileInfo(sCueFileName).suffix().compare(QStringLiteral("cue"), Qt::CaseInsensitive) != 0)) {
        return false;
    }

    QList<CUE_TRACK> listTracks;
    if (!parseCue(guardedCueDevice.data(), &listTracks) || !guardedCueDevice) return false;

    for (qint32 i = 0; i < listTracks.size(); ++i) {
        const CUE_TRACK &track = listTracks.at(i);
        qint32 nSectorSize = 0;
        qint32 nPayloadOffset = 0;
        if ((track.sFileType != QStringLiteral("BINARY")) || (track.nIndex01 < 0) || !modeToLayout(track.sMode, &nSectorSize, &nPayloadOffset)) {
            continue;
        }

        const QString sImageFileName = resolveCueImage(sCueFileName, track.sFileName);
        if (sImageFileName.isEmpty()) continue;
        QFile imageFile(sImageFileName);
        if (!imageFile.open(QIODevice::ReadOnly)) continue;

        if ((track.nIndex01 > (std::numeric_limits<qint64>::max)() / nSectorSize)) {
            imageFile.close();
            continue;
        }
        const qint64 nSourceOffset = track.nIndex01 * nSectorSize;
        qint64 nSourceEnd = imageFile.size();
        bool bMixedModePregapBoundary = false;
        for (qint32 j = i + 1; j < listTracks.size(); ++j) {
            const CUE_TRACK &nextTrack = listTracks.at(j);
            if (nextTrack.sFileName.compare(track.sFileName, Qt::CaseInsensitive) != 0) {
                break;
            }
            // INDEX 00 is the following track's pregap.  Some mixed-mode
            // masters (including Excessive Speed) keep valid MODE1 sectors
            // there and declare them in the ISO volume.  Bound the current
            // track at INDEX 01, falling back only when it is absent.
            qint64 nBoundaryFrames = nextTrack.nIndex01 >= 0 ? nextTrack.nIndex01 : nextTrack.nIndex00;
            if ((nBoundaryFrames >= track.nIndex01) && (nBoundaryFrames <= (std::numeric_limits<qint64>::max)() / nSectorSize)) {
                const qint64 nBoundaryOffset = nBoundaryFrames * nSectorSize;
                nSourceEnd = qMin(nSourceEnd, nBoundaryOffset);
                bMixedModePregapBoundary = (nSectorSize == 2352) && (nPayloadOffset == 16) &&
                                           (nextTrack.sMode.compare(QStringLiteral("AUDIO"), Qt::CaseInsensitive) == 0) &&
                                           (nextTrack.nIndex00 >= track.nIndex01) && (nextTrack.nIndex01 > nextTrack.nIndex00) &&
                                           (nBoundaryFrames == nextTrack.nIndex01) && (nSourceEnd == nBoundaryOffset);
            }
            break;
        }
        if ((nSourceOffset < 0) || (nSourceOffset >= nSourceEnd) || ((nSourceEnd - nSourceOffset) < 17LL * nSectorSize)) {
            imageFile.close();
            continue;
        }

        CDLogicalSectorDevice logicalDevice(&imageFile, nSourceOffset, nSourceEnd - nSourceOffset, nSectorSize, nPayloadOffset, false);
        quint64 nVolumeSize = 0;
        const bool bValid = logicalDevice.open(QIODevice::ReadOnly) && isLogicalIsoValid(&logicalDevice, &nVolumeSize);
        const qint64 nLogicalSize = logicalDevice.size();
        logicalDevice.close();
        imageFile.close();
        if (!bValid) continue;

        pLayout->bValid = true;
        pLayout->bCue = true;
        pLayout->sImageFileName = sImageFileName;
        pLayout->nSourceOffset = nSourceOffset;
        pLayout->nSourceSize = nSourceEnd - nSourceOffset;
        pLayout->nSectorSize = nSectorSize;
        pLayout->nPayloadOffset = nPayloadOffset;
        pLayout->bAllowTerminalZeroSector = bMixedModePregapBoundary && (nLogicalSize >= 0) && (nVolumeSize == static_cast<quint64>(nLogicalSize));
        return true;
    }
    return false;
}

bool detectDirectLayout(QIODevice *pDevice, CD_SOURCE_LAYOUT *pLayout)
{
    QPointer<QIODevice> guardedDevice(pDevice);
    if (!guardedDevice || !pLayout) return false;
    const bool bOpen = guardedDevice->isOpen();
    if (!guardedDevice || !bOpen) return false;
    const bool bReadable = guardedDevice->isReadable();
    if (!guardedDevice || !bReadable) return false;
    const bool bSequential = guardedDevice->isSequential();
    if (!guardedDevice || bSequential) return false;

    const qint64 nSize = guardedDevice->size();
    if (!guardedDevice || (nSize < 17LL * ISO_LOGICAL_SECTOR_SIZE)) return false;

    // Preserve the historical cooked ISO acceptance contract. The native
    // parser performs its normal descriptor checks after construction.
    const QByteArray baCookedSignature = readDeviceAt(guardedDevice.data(), 16LL * ISO_LOGICAL_SECTOR_SIZE + 1, 5);
    if (!guardedDevice) return false;
    if (baCookedSignature == QByteArrayLiteral("CD001")) {
        pLayout->bValid = true;
        pLayout->nSourceSize = nSize;
        pLayout->nSectorSize = ISO_LOGICAL_SECTOR_SIZE;
        pLayout->nPayloadOffset = 0;
        return true;
    }

    struct RAW_LAYOUT {
        qint32 nSectorSize;
        qint32 nPayloadOffset;
    };
    const RAW_LAYOUT layouts[] = {{2352, 16}, {2352, 24}, {2336, 8}, {2448, 16}, {2448, 24}};
    for (const RAW_LAYOUT &candidate : layouts) {
        if ((nSize < 17LL * candidate.nSectorSize) || ((nSize % candidate.nSectorSize) != 0)) {
            continue;
        }
        CDLogicalSectorDevice logicalDevice(guardedDevice.data(), 0, nSize, candidate.nSectorSize, candidate.nPayloadOffset, false);
        const bool bValid = logicalDevice.open(QIODevice::ReadOnly) && isLogicalIsoValid(&logicalDevice);
        logicalDevice.close();
        if (!guardedDevice) return false;
        if (!bValid) continue;

        pLayout->bValid = true;
        pLayout->nSourceSize = nSize;
        pLayout->nSectorSize = candidate.nSectorSize;
        pLayout->nPayloadOffset = candidate.nPayloadOffset;
        return true;
    }
    return false;
}

bool detectSourceLayout(QIODevice *pDevice, CD_SOURCE_LAYOUT *pLayout)
{
    if (pLayout) *pLayout = CD_SOURCE_LAYOUT();
    QPointer<QIODevice> guardedDevice(pDevice);
    if (!guardedDevice || !pLayout) return false;
    if (detectDirectLayout(guardedDevice.data(), pLayout)) return true;
    if (!guardedDevice) return false;
    return detectCueLayout(guardedDevice.data(), pLayout);
}

}  // namespace

XBinary::XCONVERT _TABLE_XISO9660_STRUCTID[] = {{XISO9660::STRUCTID_UNKNOWN, "Unknown", QObject::tr("Unknown")},
                                                {XISO9660::STRUCTID_PVDESC, "PVDESC", QString("Primary Volume Descriptor")},
                                                {XISO9660::STRUCTID_DIR_RECORD, "DIR_RECORD", QString("Directory Record")}};

XISO9660::XISO9660(QIODevice *pDevice)
    : XArchive(pDevice),
      m_pLogicalImageDevice(nullptr),
      m_pOwnedImageFile(nullptr),
      m_nVolumeDescriptorOffset(-1),
      m_bJoliet(false),
      m_bCueSource(false),
      m_bRawSectorSource(false)
{
    QPointer<QIODevice> guardedOriginalDevice(pDevice);
    const qint64 nOriginalPosition = guardedOriginalDevice ? guardedOriginalDevice->pos() : -1;
    _configureLogicalImage(guardedOriginalDevice.data());
    _selectVolumeDescriptor();

    if (isValid()) {
        qint64 nPVDOffset = _getPrimaryVolumeDescriptorOffset();
        m_sSystemIdentifier = decodeDescriptorText(read_array(nPVDOffset + 8, 32), m_bJoliet);
        m_sVolumeIdentifier = decodeDescriptorText(read_array(nPVDOffset + 40, 32), m_bJoliet);
        m_sVolumeSetIdentifier = decodeDescriptorText(read_array(nPVDOffset + 190, 128), m_bJoliet);
        m_sPublisherIdentifier = decodeDescriptorText(read_array(nPVDOffset + 318, 128), m_bJoliet);
        m_sDataPreparerIdentifier = decodeDescriptorText(read_array(nPVDOffset + 446, 128), m_bJoliet);
        m_sApplicationIdentifier = decodeDescriptorText(read_array(nPVDOffset + 574, 128), m_bJoliet);
        m_sCopyrightFileIdentifier = decodeDescriptorText(read_array(nPVDOffset + 702, 37), m_bJoliet);
        m_sAbstractFileIdentifier = decodeDescriptorText(read_array(nPVDOffset + 739, 37), m_bJoliet);
        m_sBibliographicFileIdentifier = decodeDescriptorText(read_array(nPVDOffset + 776, 37), m_bJoliet);
        m_sCreationDateTime = QString::fromLatin1(read_array(nPVDOffset + 813, 17)).trimmed();
        m_sModificationDateTime = QString::fromLatin1(read_array(nPVDOffset + 830, 17)).trimmed();
        m_sExpirationDateTime = QString::fromLatin1(read_array(nPVDOffset + 847, 17)).trimmed();
        m_sEffectiveDateTime = QString::fromLatin1(read_array(nPVDOffset + 864, 17)).trimmed();
    }
    if (guardedOriginalDevice && (nOriginalPosition >= 0)) guardedOriginalDevice->seek(nOriginalPosition);
}

XISO9660::~XISO9660()
{
    if (m_pLogicalImageDevice) {
        QPointer<QIODevice> guardedLogical = m_pLogicalImageDevice;
        if (getDevice() == guardedLogical.data()) setDevice(nullptr);
        if (guardedLogical) guardedLogical->close();
        if (guardedLogical) delete guardedLogical.data();
        m_pLogicalImageDevice.clear();
    }
    if (m_pOwnedImageFile) {
        QPointer<QFile> guardedFile = m_pOwnedImageFile;
        guardedFile->close();
        if (guardedFile) delete guardedFile.data();
        m_pOwnedImageFile.clear();
    }
}

bool XISO9660::_configureLogicalImage(QIODevice *pDevice)
{
    QPointer<XISO9660> guardedThis(this);
    QPointer<QIODevice> guardedSource(pDevice);
    if (!guardedThis || !guardedSource) return false;
    CD_SOURCE_LAYOUT layout;
    if (!detectSourceLayout(guardedSource.data(), &layout) || !guardedThis || !guardedSource || !layout.bValid) {
        return false;
    }

    m_bCueSource = layout.bCue;
    m_bRawSectorSource = (layout.nSectorSize != ISO_LOGICAL_SECTOR_SIZE) || (layout.nPayloadOffset != 0);

    // A normal contiguous ISO already is the logical view.
    if (!layout.bCue && !m_bRawSectorSource && (layout.nSourceOffset == 0)) {
        const qint64 nDeviceSize = guardedSource->size();
        if (!guardedThis || !guardedSource) return false;
        if (layout.nSourceSize == nDeviceSize) return true;
    }

    QPointer<QIODevice> guardedImageDevice = guardedSource;
    if (layout.bCue) {
        m_pOwnedImageFile = new (std::nothrow) QFile(layout.sImageFileName);
        QPointer<QFile> guardedFile = m_pOwnedImageFile;
        if (!guardedFile) return false;
        const bool bOpened = guardedFile->open(QIODevice::ReadOnly);
        if (!guardedThis || !guardedFile || !bOpened) {
            if (guardedFile) delete guardedFile.data();
            if (guardedThis) m_pOwnedImageFile.clear();
            return false;
        }
        guardedImageDevice = guardedFile.data();
    }

    QPointer<CDLogicalSectorDevice> guardedLogicalDevice =
        new (std::nothrow) CDLogicalSectorDevice(guardedImageDevice.data(), layout.nSourceOffset, layout.nSourceSize, layout.nSectorSize, layout.nPayloadOffset,
                                                layout.bAllowTerminalZeroSector);
    if (!guardedThis || !guardedImageDevice || !guardedLogicalDevice) {
        if (guardedLogicalDevice) delete guardedLogicalDevice.data();
        if (guardedThis && m_pOwnedImageFile) {
            QPointer<QFile> guardedFile = m_pOwnedImageFile;
            guardedFile->close();
            if (guardedFile) delete guardedFile.data();
            if (guardedThis) m_pOwnedImageFile.clear();
        }
        return false;
    }
    const bool bLogicalOpened = guardedLogicalDevice->open(QIODevice::ReadOnly);
    if (!guardedThis || !guardedImageDevice || !guardedLogicalDevice || !bLogicalOpened) {
        if (guardedLogicalDevice) delete guardedLogicalDevice.data();
        if (guardedThis && m_pOwnedImageFile) {
            QPointer<QFile> guardedFile = m_pOwnedImageFile;
            guardedFile->close();
            if (guardedFile) delete guardedFile.data();
            if (guardedThis) m_pOwnedImageFile.clear();
        }
        return false;
    }

    m_pLogicalImageDevice = guardedLogicalDevice.data();
    setDevice(guardedLogicalDevice.data());
    return guardedThis && guardedLogicalDevice && (getDevice() == guardedLogicalDevice.data());
}

bool XISO9660::_selectVolumeDescriptor()
{
    QPointer<XISO9660> guardedThis(this);
    QPointer<QIODevice> guardedDevice(getDevice());
    m_nVolumeDescriptorOffset = -1;
    m_bJoliet = false;
    ISO_DESCRIPTOR_CHOICE choice;
    if (!selectIsoDescriptor(guardedDevice.data(), &choice) || !guardedThis || !guardedDevice) return false;
    m_nVolumeDescriptorOffset = choice.nOffset;
    m_bJoliet = choice.bJoliet;
    return true;
}

bool XISO9660::isValid(PDSTRUCT *pPdStruct)
{
    bool bResult = false;
    QPointer<XISO9660> guardedThis(this);
    QPointer<QIODevice> guardedDevice(getDevice());
    const qint64 nOriginalPosition = guardedDevice ? guardedDevice->pos() : -1;
    if (!guardedThis || !guardedDevice || (nOriginalPosition < 0)) return false;
    const qint64 nTotalSize = guardedThis->getSize();
    if (!guardedThis || !guardedDevice) return false;

    if (XBinary::isPdStructNotCanceled(pPdStruct) && (nTotalSize >= ISO_LOGICAL_SECTOR_SIZE) && (m_nVolumeDescriptorOffset >= 0) &&
        (m_nVolumeDescriptorOffset <= nTotalSize - ISO_LOGICAL_SECTOR_SIZE)) {
        _MEMORY_MAP memoryMap = XBinary::getSimpleMemoryMap();

        if (compareSignature(&memoryMap, "4344303031", m_nVolumeDescriptorOffset + 1, pPdStruct)) {
            bResult = true;
        }
    }

    if (!guardedThis || !guardedDevice) return false;
    const bool bRestored = guardedDevice->seek(nOriginalPosition);
    return guardedThis && guardedDevice && bRestored && bResult;
}

bool XISO9660::isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    XISO9660 xiso(pDevice);
    return xiso.isValid(pPdStruct);
}

bool XISO9660::isCueOrRawImage(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    if (!XBinary::isPdStructNotCanceled(pPdStruct)) return false;
    CD_SOURCE_LAYOUT layout;
    const bool bDetected = detectSourceLayout(pDevice, &layout);
    return bDetected && XBinary::isPdStructNotCanceled(pPdStruct) && (layout.bCue || (layout.nSectorSize != ISO_LOGICAL_SECTOR_SIZE) || (layout.nPayloadOffset != 0));
}

XISO9660::ISO9660_PVDESC XISO9660::_readPrimaryVolumeDescriptor(qint64 nOffset)
{
    ISO9660_PVDESC desc = {};
    read_array(nOffset, (char *)&desc, sizeof(ISO9660_PVDESC));
    return desc;
}

qint32 XISO9660::_getLogicalBlockSize()
{
    const qint64 nDescriptorOffset = _getPrimaryVolumeDescriptorOffset();
    if (nDescriptorOffset < 0) return 0;
    quint16 nBlockSize = read_uint16(nDescriptorOffset + 128);
    return (qint32)nBlockSize;
}

qint64 XISO9660::_getPrimaryVolumeDescriptorOffset()
{
    return m_nVolumeDescriptorOffset;
}

bool XISO9660::_isValidDescriptor(qint64 nOffset, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    char szStandard[6] = {0};
    read_array(nOffset + 1, szStandard, 5);

    return QString::fromLatin1(szStandard, 5) == "CD001";
}

QString XISO9660::getFileFormatExt()
{
    return "iso";
}

QString XISO9660::getFileFormatExtsString()
{
    return "ISO 9660 / CD image (*.iso *.img *.bin *.raw *.cue)";
}

qint64 XISO9660::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    qint64 nResult = 0;

    if (isValid()) {
        // Volume Space Size is at offset 80 in the Primary Volume Descriptor (little-endian)
        // It specifies the total number of logical blocks
        qint64 nPVDOffset = _getPrimaryVolumeDescriptorOffset();
        quint32 nVolumeSpaceSize = read_uint32(nPVDOffset + 80);
        qint32 nLogicalBlockSize = _getLogicalBlockSize();

        if (nLogicalBlockSize > 0 && nVolumeSpaceSize > 0) {
            nResult = (qint64)nVolumeSpaceSize * (qint64)nLogicalBlockSize;
        }
    }

    return nResult;
}

QString XISO9660::getMIMEString()
{
    return "application/x-iso9660-image";
}

XBinary::FT XISO9660::getFileType()
{
    return FT_ISO9660;
}

QList<XBinary::MAPMODE> XISO9660::getMapModesList()
{
    QList<MAPMODE> listResult;
    listResult.append(MAPMODE_REGIONS);
    return listResult;
}

XBinary::_MEMORY_MAP XISO9660::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(mapMode)

    _MEMORY_MAP result = {};
    result.fileType = getFileType();
    result = _getMemoryMap(FILEPART_HEADER | FILEPART_REGION | FILEPART_OVERLAY, pPdStruct);

    return result;
}

QString XISO9660::structIDToString(quint32 nID)
{
    return XBinary::XCONVERT_idToTransString(nID, _TABLE_XISO9660_STRUCTID, sizeof(_TABLE_XISO9660_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QString XISO9660::structIDToFtString(quint32 nID)
{
    return XBinary::XCONVERT_idToFtString(nID, _TABLE_XISO9660_STRUCTID, sizeof(_TABLE_XISO9660_STRUCTID) / sizeof(XBinary::XCONVERT));
}

quint32 XISO9660::ftStringToStructID(const QString &sFtString)
{
    return XCONVERT_ftStringToId(sFtString, _TABLE_XISO9660_STRUCTID, sizeof(_TABLE_XISO9660_STRUCTID) / sizeof(XBinary::XCONVERT));
}

// QList<XBinary::DATA_HEADER> XISO9660::getDataHeaders(const DATA_HEADERS_OPTIONS &dataHeadersOptions, PDSTRUCT *pPdStruct)
// {
//     QList<DATA_HEADER> listResult;

//     if (dataHeadersOptions.nID == STRUCTID_UNKNOWN) {
//         DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
//         _dataHeadersOptions.bChildren = true;
//         _dataHeadersOptions.dsID_parent = _addDefaultHeaders(&listResult, pPdStruct);
//         _dataHeadersOptions.dhMode = XBinary::DHMODE_HEADER;
//         _dataHeadersOptions.fileType = dataHeadersOptions.pMemoryMap->fileType;

//         _dataHeadersOptions.nID = STRUCTID_PVDESC;
//         _dataHeadersOptions.nLocation = _getPrimaryVolumeDescriptorOffset();
//         _dataHeadersOptions.locType = XBinary::LT_OFFSET;

//         listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//     } else {
//         qint64 nStartOffset = locationToOffset(dataHeadersOptions.pMemoryMap, dataHeadersOptions.locType, dataHeadersOptions.nLocation);

//         if (nStartOffset != -1) {
//             if (dataHeadersOptions.nID == STRUCTID_PVDESC) {
//                 DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, XISO9660::structIDToString(dataHeadersOptions.nID));
//                 dataHeader.nSize = sizeof(ISO9660_PVDESC);

//                 dataHeader.listRecords.append(getDataRecord(0, 1, "Descriptor Type", VT_UINT8, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(1, 5, "Standard Identifier", VT_CHAR_ARRAY, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(6, 1, "Version", VT_UINT8, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(7, 1, "Unused", VT_UINT8, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(8, 32, "System Identifier", VT_CHAR_ARRAY, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(40, 32, "Volume Identifier", VT_CHAR_ARRAY, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(72, 8, "Unused", VT_BYTE_ARRAY, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(80, 4, "Volume Space Size (LE)", VT_UINT32, DRF_SIZE, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(84, 4, "Volume Space Size (BE)", VT_UINT32, DRF_SIZE, ENDIAN_BIG));
//                 dataHeader.listRecords.append(getDataRecord(88, 32, "Unused", VT_BYTE_ARRAY, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(120, 2, "Volume Set Size (LE)", VT_UINT16, DRF_COUNT, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(122, 2, "Volume Set Size (BE)", VT_UINT16, DRF_COUNT, ENDIAN_BIG));
//                 dataHeader.listRecords.append(getDataRecord(124, 2, "Volume Sequence Number (LE)", VT_UINT16, DRF_COUNT, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(126, 2, "Volume Sequence Number (BE)", VT_UINT16, DRF_COUNT, ENDIAN_BIG));
//                 dataHeader.listRecords.append(getDataRecord(128, 2, "Logical Block Size (LE)", VT_UINT16, DRF_SIZE, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(130, 2, "Logical Block Size (BE)", VT_UINT16, DRF_SIZE, ENDIAN_BIG));
//                 dataHeader.listRecords.append(getDataRecord(132, 4, "Path Table Size (LE)", VT_UINT32, DRF_SIZE, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(136, 4, "Path Table Size (BE)", VT_UINT32, DRF_SIZE, ENDIAN_BIG));
//                 dataHeader.listRecords.append(getDataRecord(140, 4, "L Path Table Location", VT_UINT32, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(144, 4, "Optional L Path Table Location", VT_UINT32, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(148, 4, "M Path Table Location", VT_UINT32, DRF_UNKNOWN, ENDIAN_BIG));
//                 dataHeader.listRecords.append(getDataRecord(152, 4, "Optional M Path Table Location", VT_UINT32, DRF_UNKNOWN, ENDIAN_BIG));
//                 dataHeader.listRecords.append(getDataRecord(156, 34, "Root Directory Record", VT_BYTE_ARRAY, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(190, 128, "Volume Set Identifier", VT_CHAR_ARRAY, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(318, 128, "Publisher Identifier", VT_CHAR_ARRAY, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(446, 128, "Data Preparer Identifier", VT_CHAR_ARRAY, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(574, 128, "Application Identifier", VT_CHAR_ARRAY, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(702, 37, "Copyright File Identifier", VT_CHAR_ARRAY, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(739, 37, "Abstract File Identifier", VT_CHAR_ARRAY, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(776, 37, "Bibliographic File Identifier", VT_CHAR_ARRAY, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(813, 17, "Creation Date/Time", VT_CHAR_ARRAY, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(830, 17, "Modification Date/Time", VT_CHAR_ARRAY, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(847, 17, "Expiration Date/Time", VT_CHAR_ARRAY, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(864, 17, "Effective Date/Time", VT_CHAR_ARRAY, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(880, 1, "File Structure Version", VT_UINT8, DRF_UNKNOWN, ENDIAN_LITTLE));

//                 listResult.append(dataHeader);

//                 if (dataHeadersOptions.bChildren) {
//                     // Add root directory record as a child structure
//                     DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
//                     _dataHeadersOptions.dhMode = XBinary::DHMODE_HEADER;
//                     _dataHeadersOptions.fileType = dataHeadersOptions.pMemoryMap->fileType;
//                     _dataHeadersOptions.nID = STRUCTID_DIR_RECORD;
//                     _dataHeadersOptions.locType = LT_OFFSET;
//                     _dataHeadersOptions.nLocation = nStartOffset + 156;  // Root directory record offset within PVD
//                     _dataHeadersOptions.nSize = 34;

//                     listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//                 }
//             } else if (dataHeadersOptions.nID == STRUCTID_DIR_RECORD) {
//                 DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, XISO9660::structIDToString(dataHeadersOptions.nID));

//                 quint8 nRecordLength = read_uint8(nStartOffset);
//                 dataHeader.nSize = nRecordLength;

//                 dataHeader.listRecords.append(getDataRecord(0, 1, "Length of Directory Record", VT_UINT8, DRF_SIZE, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(1, 1, "Extended Attribute Record Length", VT_UINT8, DRF_SIZE, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(2, 4, "Location of Extent (LE)", VT_UINT32, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(6, 4, "Location of Extent (BE)", VT_UINT32, DRF_UNKNOWN, ENDIAN_BIG));
//                 dataHeader.listRecords.append(getDataRecord(10, 4, "Data Length (LE)", VT_UINT32, DRF_SIZE, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(14, 4, "Data Length (BE)", VT_UINT32, DRF_SIZE, ENDIAN_BIG));
//                 dataHeader.listRecords.append(getDataRecord(18, 1, "Years since 1900", VT_UINT8, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(19, 1, "Month", VT_UINT8, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(20, 1, "Day", VT_UINT8, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(21, 1, "Hour", VT_UINT8, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(22, 1, "Minute", VT_UINT8, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(23, 1, "Second", VT_UINT8, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(24, 1, "GMT Offset", VT_UINT8, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(25, 1, "File Flags", VT_UINT8, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(26, 1, "File Unit Size", VT_UINT8, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(27, 1, "Interleave Gap Size", VT_UINT8, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(28, 2, "Volume Sequence Number (LE)", VT_UINT16, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 dataHeader.listRecords.append(getDataRecord(30, 2, "Volume Sequence Number (BE)", VT_UINT16, DRF_UNKNOWN, ENDIAN_BIG));
//                 dataHeader.listRecords.append(getDataRecord(32, 1, "Length of File Identifier", VT_UINT8, DRF_SIZE, ENDIAN_LITTLE));

//                 // File identifier follows at offset 33
//                 quint8 nFileIdLength = read_uint8(nStartOffset + 32);
//                 if (nFileIdLength > 0 && nRecordLength >= 33 + nFileIdLength) {
//                     dataHeader.listRecords.append(getDataRecord(33, nFileIdLength, "File Identifier", VT_CHAR_ARRAY, DRF_UNKNOWN, ENDIAN_LITTLE));
//                 }

//                 listResult.append(dataHeader);
//             }
//         }
//     }

//     return listResult;
// }

QList<XBinary::XFHEADER> XISO9660::getXFHeaders(const XFSTRUCT &xfStruct, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    QList<XBinary::XFHEADER> listResult;
    quint32 nStructID = xfStruct.nStructID;

    if (nStructID == STRUCTID_UNKNOWN) {
        XFSTRUCT _xfStruct = xfStruct;
        _xfStruct.nStructID = STRUCTID_PVDESC;
        _xfStruct.xLoc = offsetToLoc(_getPrimaryVolumeDescriptorOffset());
        listResult.append(getXFHeaders(_xfStruct, pPdStruct));
    } else if ((nStructID == STRUCTID_PVDESC) || (nStructID == STRUCTID_DIR_RECORD)) {
        XLOC headerLoc = xfStruct.xLoc;
        if (headerLoc.locType == LT_UNKNOWN) {
            headerLoc = (nStructID == STRUCTID_PVDESC) ? offsetToLoc(_getPrimaryVolumeDescriptorOffset())
                                                       : offsetToLoc(_getPrimaryVolumeDescriptorOffset() + offsetof(ISO9660_PVDESC, nRootDirRecord));
        }

        qint64 nHeaderOffset = locToOffset(xfStruct.pMemoryMap, headerLoc);

        if (nHeaderOffset != -1) {
            qint64 nHeaderSize = xfStruct.nSize;

            if (nHeaderSize <= 0) {
                if (nStructID == STRUCTID_PVDESC) {
                    nHeaderSize = sizeof(ISO9660_PVDESC);
                } else {
                    nHeaderSize = read_uint8(nHeaderOffset);
                }
            }

            if ((nHeaderSize > 0) && isOffsetAndSizeValid(xfStruct.pMemoryMap, nHeaderOffset, nHeaderSize)) {
                XFHEADER xfHeader = {};
                xfHeader.sParentTag = xfStruct.sParent;
                xfHeader.fileType = xfStruct.fileType;
                xfHeader.structID = static_cast<XBinary::STRUCTID>(nStructID);
                xfHeader.xLoc = headerLoc;
                xfHeader.nSize = nHeaderSize;
                xfHeader.xfType = XFTYPE_HEADER;
                xfHeader.listFields = getXFRecords(xfStruct.fileType, nStructID, headerLoc);
                xfHeader.sTag = xfHeaderToTag(xfHeader, structIDToString(nStructID), xfHeader.sParentTag);
                listResult.append(xfHeader);

                if ((nStructID == STRUCTID_PVDESC) && xfStruct.bIsParent) {
                    XFSTRUCT dirStruct = xfStruct;
                    dirStruct.sParent = xfHeader.sTag;
                    dirStruct.nStructID = STRUCTID_DIR_RECORD;
                    dirStruct.xLoc = offsetToLoc(nHeaderOffset + offsetof(ISO9660_PVDESC, nRootDirRecord));
                    dirStruct.nSize = sizeof(ISO9660_DIR_RECORD);
                    dirStruct.bIsParent = false;
                    listResult.append(getXFHeaders(dirStruct, pPdStruct));
                }
            }
        }
    }

    return listResult;
}

QList<XBinary::XFRECORD> XISO9660::getXFRecords(FT fileType, quint32 nStructID, const XLOC &xLoc)
{
    Q_UNUSED(fileType)
    Q_UNUSED(xLoc)

    QList<XBinary::XFRECORD> listResult;

    if (nStructID == STRUCTID_PVDESC) {
        listResult.append({"nDescType", (qint32)offsetof(ISO9660_PVDESC, nDescType), 1, XFRECORD_FLAG_NONE, VT_UINT8});
        listResult.append({"szStandard", (qint32)offsetof(ISO9660_PVDESC, szStandard), 5, XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        listResult.append({"nDescVersion", (qint32)offsetof(ISO9660_PVDESC, nDescVersion), 1, XFRECORD_FLAG_VERSION, VT_UINT8});
        listResult.append({"nUnused1", (qint32)offsetof(ISO9660_PVDESC, nUnused1), 1, XFRECORD_FLAG_NONE, VT_UINT8});
        listResult.append({"szSystemId", (qint32)offsetof(ISO9660_PVDESC, szSystemId), 32, XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        listResult.append({"szVolumeId", (qint32)offsetof(ISO9660_PVDESC, szVolumeId), 32, XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        listResult.append({"nUnused2", (qint32)offsetof(ISO9660_PVDESC, nUnused2), 8, XFRECORD_FLAG_NONE, VT_BYTE_ARRAY});
        listResult.append({"nVolumeSpaceSizeLE", (qint32)offsetof(ISO9660_PVDESC, nVolumeSpaceSizeLE), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"nVolumeSpaceSizeBE", (qint32)offsetof(ISO9660_PVDESC, nVolumeSpaceSizeBE), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"nUnused3", (qint32)offsetof(ISO9660_PVDESC, nUnused3), 32, XFRECORD_FLAG_NONE, VT_BYTE_ARRAY});
        listResult.append({"nVolumeSetSizeLE", (qint32)offsetof(ISO9660_PVDESC, nVolumeSetSizeLE), 2, XFRECORD_FLAG_COUNT, VT_UINT16});
        listResult.append({"nVolumeSetSizeBE", (qint32)offsetof(ISO9660_PVDESC, nVolumeSetSizeBE), 2, XFRECORD_FLAG_COUNT, VT_UINT16});
        listResult.append({"nVolumeSeqNumLE", (qint32)offsetof(ISO9660_PVDESC, nVolumeSeqNumLE), 2, XFRECORD_FLAG_COUNT, VT_UINT16});
        listResult.append({"nVolumeSeqNumBE", (qint32)offsetof(ISO9660_PVDESC, nVolumeSeqNumBE), 2, XFRECORD_FLAG_COUNT, VT_UINT16});
        listResult.append({"nLogicalBlockSizeLE", (qint32)offsetof(ISO9660_PVDESC, nLogicalBlockSizeLE), 2, XFRECORD_FLAG_SIZE, VT_UINT16});
        listResult.append({"nLogicalBlockSizeBE", (qint32)offsetof(ISO9660_PVDESC, nLogicalBlockSizeBE), 2, XFRECORD_FLAG_SIZE, VT_UINT16});
        listResult.append({"nPathTableSizeLE", (qint32)offsetof(ISO9660_PVDESC, nPathTableSizeLE), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"nPathTableSizeBE", (qint32)offsetof(ISO9660_PVDESC, nPathTableSizeBE), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"nLPathTableLoc", (qint32)offsetof(ISO9660_PVDESC, nLPathTableLoc), 4, XFRECORD_FLAG_OFFSET, VT_UINT32});
        listResult.append({"nOptLPathTableLoc", (qint32)offsetof(ISO9660_PVDESC, nOptLPathTableLoc), 4, XFRECORD_FLAG_OFFSET, VT_UINT32});
        listResult.append({"nMPathTableLoc", (qint32)offsetof(ISO9660_PVDESC, nMPathTableLoc), 4, XFRECORD_FLAG_OFFSET, VT_UINT32});
        listResult.append({"nOptMPathTableLoc", (qint32)offsetof(ISO9660_PVDESC, nOptMPathTableLoc), 4, XFRECORD_FLAG_OFFSET, VT_UINT32});
        listResult.append({"nRootDirRecord", (qint32)offsetof(ISO9660_PVDESC, nRootDirRecord), 34, XFRECORD_FLAG_NONE, VT_BYTE_ARRAY});
        listResult.append({"szVolSetId", (qint32)offsetof(ISO9660_PVDESC, szVolSetId), 128, XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        listResult.append({"szPublisherId", (qint32)offsetof(ISO9660_PVDESC, szPublisherId), 128, XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        listResult.append({"szDataPreparerId", (qint32)offsetof(ISO9660_PVDESC, szDataPreparerId), 128, XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        listResult.append({"szApplicationId", (qint32)offsetof(ISO9660_PVDESC, szApplicationId), 128, XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        listResult.append({"szCopyrightFile", (qint32)offsetof(ISO9660_PVDESC, szCopyrightFile), 37, XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        listResult.append({"szAbstractFile", (qint32)offsetof(ISO9660_PVDESC, szAbstractFile), 37, XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        listResult.append({"szBiblioFile", (qint32)offsetof(ISO9660_PVDESC, szBiblioFile), 37, XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        listResult.append({"szCreationTime", (qint32)offsetof(ISO9660_PVDESC, szCreationTime), 17, XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        listResult.append({"szModificationTime", (qint32)offsetof(ISO9660_PVDESC, szModificationTime), 17, XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        listResult.append({"szExpirationTime", (qint32)offsetof(ISO9660_PVDESC, szExpirationTime), 17, XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        listResult.append({"szEffectiveTime", (qint32)offsetof(ISO9660_PVDESC, szEffectiveTime), 17, XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        listResult.append({"nFileStructVersion", (qint32)offsetof(ISO9660_PVDESC, nFileStructVersion), 1, XFRECORD_FLAG_VERSION, VT_UINT8});
        listResult.append({"nUnused4", (qint32)offsetof(ISO9660_PVDESC, nUnused4), 1, XFRECORD_FLAG_NONE, VT_UINT8});
        listResult.append({"nAppData", (qint32)offsetof(ISO9660_PVDESC, nAppData), 512, XFRECORD_FLAG_NONE, VT_BYTE_ARRAY});
        listResult.append({"nUnused5", (qint32)offsetof(ISO9660_PVDESC, nUnused5), 653, XFRECORD_FLAG_NONE, VT_BYTE_ARRAY});
    } else if (nStructID == STRUCTID_DIR_RECORD) {
        listResult.append({"nLength", (qint32)offsetof(ISO9660_DIR_RECORD, nLength), 1, XFRECORD_FLAG_SIZE, VT_UINT8});
        listResult.append({"nExtentAttrLength", (qint32)offsetof(ISO9660_DIR_RECORD, nExtentAttrLength), 1, XFRECORD_FLAG_SIZE, VT_UINT8});
        listResult.append({"nExtentLocationLE", (qint32)offsetof(ISO9660_DIR_RECORD, nExtentLocationLE), 4, XFRECORD_FLAG_OFFSET, VT_UINT32});
        listResult.append({"nExtentLocationBE", (qint32)offsetof(ISO9660_DIR_RECORD, nExtentLocationBE), 4, XFRECORD_FLAG_OFFSET, VT_UINT32});
        listResult.append({"nDataLengthLE", (qint32)offsetof(ISO9660_DIR_RECORD, nDataLengthLE), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"nDataLengthBE", (qint32)offsetof(ISO9660_DIR_RECORD, nDataLengthBE), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"nYear", (qint32)offsetof(ISO9660_DIR_RECORD, nYear), 1, XFRECORD_FLAG_NONE, VT_UINT8});
        listResult.append({"nMonth", (qint32)offsetof(ISO9660_DIR_RECORD, nMonth), 1, XFRECORD_FLAG_NONE, VT_UINT8});
        listResult.append({"nDay", (qint32)offsetof(ISO9660_DIR_RECORD, nDay), 1, XFRECORD_FLAG_NONE, VT_UINT8});
        listResult.append({"nHour", (qint32)offsetof(ISO9660_DIR_RECORD, nHour), 1, XFRECORD_FLAG_NONE, VT_UINT8});
        listResult.append({"nMinute", (qint32)offsetof(ISO9660_DIR_RECORD, nMinute), 1, XFRECORD_FLAG_NONE, VT_UINT8});
        listResult.append({"nSecond", (qint32)offsetof(ISO9660_DIR_RECORD, nSecond), 1, XFRECORD_FLAG_NONE, VT_UINT8});
        listResult.append({"nGMTOffset", (qint32)offsetof(ISO9660_DIR_RECORD, nGMTOffset), 1, XFRECORD_FLAG_NONE, VT_UINT8});
        listResult.append({"nFileFlags", (qint32)offsetof(ISO9660_DIR_RECORD, nFileFlags), 1, XFRECORD_FLAG_NONE, VT_UINT8});
        listResult.append({"nFileUnitSize", (qint32)offsetof(ISO9660_DIR_RECORD, nFileUnitSize), 1, XFRECORD_FLAG_SIZE, VT_UINT8});
        listResult.append({"nInterleaveGapSize", (qint32)offsetof(ISO9660_DIR_RECORD, nInterleaveGapSize), 1, XFRECORD_FLAG_SIZE, VT_UINT8});
        listResult.append({"nSequenceNumberLE", (qint32)offsetof(ISO9660_DIR_RECORD, nSequenceNumberLE), 2, XFRECORD_FLAG_COUNT, VT_UINT16});
        listResult.append({"nSequenceNumberBE", (qint32)offsetof(ISO9660_DIR_RECORD, nSequenceNumberBE), 2, XFRECORD_FLAG_COUNT, VT_UINT16});
        listResult.append({"nFileIdLength", (qint32)offsetof(ISO9660_DIR_RECORD, nFileIdLength), 1, XFRECORD_FLAG_SIZE, VT_UINT8});
    }

    return listResult;
}

static bool isoCanAppendPart(qint32 nLimit, const QList<XBinary::FPART> &listResult)
{
    return (nLimit == -1) || (listResult.size() < nLimit);
}

QList<XBinary::FPART> XISO9660::getFileParts(quint32 nFileParts, qint32 nLimit, PDSTRUCT *pPdStruct)
{
    QList<FPART> listResult;

    if ((nLimit < -1) || (nLimit == 0)) {
        return listResult;
    }

    qint64 nTotalSize = getSize();
    qint64 nFormatSize = getFileFormatSize(pPdStruct);

    if ((nFileParts & FILEPART_REGION) && isoCanAppendPart(nLimit, listResult)) {
        FPART record = {};
        record.filePart = FILEPART_REGION;
        record.nFileOffset = 0;
        record.nFileSize = _getPrimaryVolumeDescriptorOffset();
        record.nVirtualAddress = XADDR_MAX;
        record.sName = tr("Reserved");

        listResult.append(record);
    }

    if ((nFileParts & FILEPART_HEADER) && isoCanAppendPart(nLimit, listResult)) {
        FPART record = {};
        record.filePart = FILEPART_HEADER;
        record.nFileOffset = _getPrimaryVolumeDescriptorOffset();
        record.nFileSize = sizeof(ISO9660_PVDESC);
        record.nVirtualAddress = XADDR_MAX;
        record.sName = tr("Primary Volume Descriptor");

        listResult.append(record);
    }

    if ((nFileParts & FILEPART_REGION) && isoCanAppendPart(nLimit, listResult)) {
        qint64 nDataOffset = _getPrimaryVolumeDescriptorOffset() + sizeof(ISO9660_PVDESC);
        qint64 nDataSize = (nFormatSize > 0) ? (nFormatSize - nDataOffset) : (nTotalSize - nDataOffset);

        if (nDataSize > 0) {
            FPART record = {};
            record.filePart = FILEPART_REGION;
            record.nFileOffset = nDataOffset;
            record.nFileSize = nDataSize;
            record.nVirtualAddress = XADDR_MAX;
            record.sName = tr("Data");

            listResult.append(record);
        }
    }

    if ((nFileParts & FILEPART_OVERLAY) && isoCanAppendPart(nLimit, listResult)) {
        if (nFormatSize > 0 && nTotalSize > nFormatSize) {
            FPART record = {};
            record.filePart = FILEPART_OVERLAY;
            record.nFileOffset = nFormatSize;
            record.nFileSize = nTotalSize - nFormatSize;
            record.nVirtualAddress = XADDR_MAX;
            record.sName = tr("Overlay");

            listResult.append(record);
        }
    }

    return listResult;
}

QString XISO9660::_cleanFileName(const QString &sFileName)
{
    QString sResult = sFileName;

    const qint32 nSemicolon = sResult.lastIndexOf(QLatin1Char(';'));
    if ((nSemicolon >= 0) && (nSemicolon + 1 < sResult.size())) {
        bool bVersion = true;
        for (qint32 i = nSemicolon + 1; i < sResult.size(); ++i) {
            if (!sResult.at(i).isDigit()) {
                bVersion = false;
                break;
            }
        }
        if (bVersion) sResult.truncate(nSemicolon);
    }
    if (sResult.endsWith(QLatin1Char('.'))) sResult.chop(1);

    return sResult;
}

QList<XBinary::ARCHIVERECORD> XISO9660::_parseDirectoryEntries(qint64 nOffset, qint64 nSize, qint32 nBlockSize, const QString &sParentPath, PDSTRUCT *pPdStruct)
{
    QPointer<XISO9660> guardedThis(this);
    QList<ARCHIVERECORD> listResult;

    qint64 nFileSize = guardedThis->getSize();
    if (!guardedThis) return listResult;
    const qint64 nFormatSize = guardedThis->getFileFormatSize(pPdStruct);
    if (!guardedThis) return listResult;
    if (nFormatSize > 0) nFileSize = qMin(nFileSize, nFormatSize);
    if ((nBlockSize <= 0) || (nFileSize < 0) || (nOffset < 0) || (nSize < 0) || (nOffset > nFileSize) || (nSize > nFileSize - nOffset)) {
        return listResult;
    }
    const qint64 nEndOffset = nOffset + nSize;

    qint64 nCurrentOffset = nOffset;

    while ((nCurrentOffset < nEndOffset) && isPdStructNotCanceled(pPdStruct)) {
        // Align to logical block boundaries for padding detection
        const qint64 nBlockStart = nCurrentOffset;
        const qint64 nAdvance = nBlockSize - (nBlockStart % nBlockSize);
        const qint64 nNextBlockStart = (nAdvance > nEndOffset - nBlockStart) ? nEndOffset : nBlockStart + nAdvance;
        qint64 nBlockEnd = qMin(nNextBlockStart, nEndOffset);

        while (nCurrentOffset < nBlockEnd && isPdStructNotCanceled(pPdStruct)) {
            quint8 nRecordLength = guardedThis->read_uint8(nCurrentOffset);
            if (!guardedThis) return QList<ARCHIVERECORD>();

            if (nRecordLength == 0) {
                // Zero-padding to next logical block
                nCurrentOffset = nBlockEnd;
                break;
            }

            if ((nRecordLength < 34) || (nRecordLength > nBlockEnd - nCurrentOffset)) {
                return QList<ARCHIVERECORD>();
            }

            const QByteArray baRecord = guardedThis->read_array(nCurrentOffset, nRecordLength);
            if (!guardedThis || (baRecord.size() != nRecordLength)) return QList<ARCHIVERECORD>();
            const char *pRecord = baRecord.constData();
            const quint8 nExtAttrLength = static_cast<quint8>(pRecord[1]);
            const quint32 nExtentLocation = read32le(pRecord + 2);
            const quint32 nExtentLocationBe = read32be(pRecord + 6);
            const quint32 nDataLength = read32le(pRecord + 10);
            const quint32 nDataLengthBe = read32be(pRecord + 14);
            const quint8 nFileFlags = static_cast<quint8>(pRecord[25]);
            const quint16 nSequence = read16le(pRecord + 28);
            const quint16 nSequenceBe = read16be(pRecord + 30);
            const quint8 nFileNameLength = static_cast<quint8>(pRecord[32]);
            const qint32 nMinimumRecordLength = 33 + nFileNameLength + ((nFileNameLength % 2) == 0 ? 1 : 0);
            if ((nMinimumRecordLength > nRecordLength) || (nExtentLocation != nExtentLocationBe) || (nDataLength != nDataLengthBe) || (nSequence == 0) ||
                (nSequence != nSequenceBe)) {
                return QList<ARCHIVERECORD>();
            }

            const QByteArray baFileName = baRecord.mid(33, nFileNameLength);
            if ((nFileNameLength == 1) && ((static_cast<quint8>(baFileName.at(0)) == 0) || (static_cast<quint8>(baFileName.at(0)) == 1))) {
                nCurrentOffset += nRecordLength;
                continue;
            }

            QString sFileName;
            if (m_bJoliet) {
                if (!decodeUcs2Be(baFileName, &sFileName)) return QList<ARCHIVERECORD>();
            } else {
                sFileName = QString::fromLatin1(baFileName);
            }
            if (sFileName.isEmpty()) return QList<ARCHIVERECORD>();

            const qint64 nDataBlock = static_cast<qint64>(nExtentLocation) + nExtAttrLength;
            bool bPayloadAvailable = (nDataBlock >= 0) && (nDataBlock <= (std::numeric_limits<qint64>::max)() / nBlockSize);
            qint64 nStreamOffset = 0;
            if (bPayloadAvailable) {
                nStreamOffset = nDataBlock * nBlockSize;
                bPayloadAvailable =
                    (nStreamOffset >= 0) && (nStreamOffset <= nFileSize) && (static_cast<quint64>(nDataLength) <= static_cast<quint64>(nFileSize - nStreamOffset));
            }

            ARCHIVERECORD record = {};
            // Some mastered game discs deliberately contain copy-protection
            // directory entries whose advertised extents cannot exist inside
            // the selected volume.  They are still directory metadata, and a
            // bad member must not erase every valid sibling from the listing.
            // Publish an explicit empty placeholder for such an entry.  The
            // placeholder keeps extraction bounded and lets extract-all
            // continue without fabricating bytes from another part of the CD.
            record.nStreamOffset = bPayloadAvailable ? nStreamOffset : 0;
            record.nStreamSize = bPayloadAvailable ? nDataLength : 0;

            QString sCleanName = guardedThis->_cleanFileName(sFileName);
            QString sFullPath;

            if (sParentPath.isEmpty()) {
                sFullPath = sCleanName;
            } else {
                sFullPath = sParentPath + "/" + sCleanName;
            }

            bool bIsFolder = (nFileFlags & 0x02) != 0;

            record.mapProperties[FPART_PROP_ORIGINALNAME] = sFullPath;
            record.mapProperties[FPART_PROP_UNCOMPRESSEDSIZE] = bPayloadAvailable ? (qint64)nDataLength : 0;
            record.mapProperties[FPART_PROP_COMPRESSEDSIZE] = bPayloadAvailable ? (qint64)nDataLength : 0;
            record.mapProperties[FPART_PROP_HANDLEMETHOD] = HANDLE_METHOD_STORE;
            record.mapProperties[FPART_PROP_ISFOLDER] = bIsFolder;
            if (!bPayloadAvailable) {
                record.mapProperties[FPART_PROP_INFO] =
                    tr("Payload unavailable: ISO extent block %1, declared size %2 is outside the selected volume").arg(nDataBlock).arg(nDataLength);
            }

            if (bIsFolder) {
                record.mapProperties[FPART_PROP_STREAMOFFSET] = record.nStreamOffset;
                record.mapProperties[FPART_PROP_STREAMSIZE] = record.nStreamSize;
            }

            // Read recording date/time from directory record
            const quint8 nYear = static_cast<quint8>(pRecord[18]);
            const quint8 nMonth = static_cast<quint8>(pRecord[19]);
            const quint8 nDay = static_cast<quint8>(pRecord[20]);
            const quint8 nHour = static_cast<quint8>(pRecord[21]);
            const quint8 nMinute = static_cast<quint8>(pRecord[22]);
            const quint8 nSecond = static_cast<quint8>(pRecord[23]);

            if (nYear > 0 && nMonth >= 1 && nMonth <= 12 && nDay >= 1 && nDay <= 31) {
                QDateTime dt(QDate(1900 + nYear, nMonth, nDay), QTime(nHour, nMinute, nSecond));

                if (dt.isValid()) {
                    record.mapProperties[FPART_PROP_MTIME] = dt;
                }
            }

            listResult.append(record);
            nCurrentOffset += nRecordLength;
        }
    }

    return listResult;
}

QList<XBinary::ARCHIVERECORD> XISO9660::_collectAllRecords(qint64 nRootOffset, qint64 nRootSize, qint32 nBlockSize, PDSTRUCT *pPdStruct)
{
    QPointer<XISO9660> guardedThis(this);
    QList<ARCHIVERECORD> listResult;

    // BFS: queue of (dirOffset, dirSize, parentPath)
    struct DirEntry {
        qint64 nOffset;
        qint64 nSize;
        QString sPath;
    };

    QList<DirEntry> listDirQueue;
    QSet<qint64> setProcessedBlocks;

    DirEntry rootEntry;
    rootEntry.nOffset = nRootOffset;
    rootEntry.nSize = nRootSize;
    rootEntry.sPath = QString();

    listDirQueue.append(rootEntry);
    setProcessedBlocks.insert(nRootOffset / nBlockSize);

    while (!listDirQueue.isEmpty() && isPdStructNotCanceled(pPdStruct)) {
        DirEntry dirInfo = listDirQueue.takeFirst();

        QList<ARCHIVERECORD> listDirRecords = guardedThis->_parseDirectoryEntries(dirInfo.nOffset, dirInfo.nSize, nBlockSize, dirInfo.sPath, pPdStruct);
        if (!guardedThis) return QList<ARCHIVERECORD>();

        for (qint32 i = 0; i < listDirRecords.count() && isPdStructNotCanceled(pPdStruct); i++) {
            ARCHIVERECORD record = listDirRecords.at(i);
            listResult.append(record);

            // If it's a directory, enqueue it for processing
            if (record.mapProperties.value(FPART_PROP_ISFOLDER).toBool()) {
                qint64 nSubDirOffset = record.mapProperties.value(FPART_PROP_STREAMOFFSET).toLongLong();
                qint64 nSubDirSize = record.mapProperties.value(FPART_PROP_STREAMSIZE).toLongLong();
                qint64 nSubDirBlock = nSubDirOffset / nBlockSize;

                if (!setProcessedBlocks.contains(nSubDirBlock) && nSubDirOffset > 0 && nSubDirSize > 0) {
                    DirEntry subEntry;
                    subEntry.nOffset = nSubDirOffset;
                    subEntry.nSize = nSubDirSize;
                    subEntry.sPath = record.mapProperties.value(FPART_PROP_ORIGINALNAME).toString();

                    listDirQueue.append(subEntry);
                    setProcessedBlocks.insert(nSubDirBlock);
                }
            }
        }
    }

    return listResult;
}

QMap<XBinary::UNPACK_PROP, QVariant> XISO9660::getDefaultUnpackProperties()
{
    QMap<XBinary::UNPACK_PROP, QVariant> result = XArchive::getDefaultUnpackProperties();

    return result;
}

bool XISO9660::initUnpack(UNPACK_STATE *pState, const QMap<UNPACK_PROP, QVariant> &mapProperties, PDSTRUCT *pPdStruct)
{
    QPointer<XISO9660> guardedThis(this);
    if (m_bUnpackOperationInProgress) {
        return false;
    }
    UNPACK_OPERATION_GUARD operationGuard(&m_bUnpackOperationInProgress);
    if (!operationGuard.isAcquired()) return false;

    if (!pState) {
        return false;
    }

    if ((pState->pContext || !pState->baUnpackSourceToken.isEmpty()) && !guardedThis->ownsUnpackSource(pState)) {
        return false;
    }
    ISO9660_UNPACK_CONTEXT *pOldContext = static_cast<ISO9660_UNPACK_CONTEXT *>(pState->pContext);
    guardedThis->releaseUnpackSource(pState);
    pState->pContext = nullptr;
    delete pOldContext;
    if (!guardedThis) return false;
    *pState = UNPACK_STATE();
    if (!isPdStructNotCanceled(pPdStruct)) return false;
    const bool bBound = guardedThis->bindUnpackSource(pState, pPdStruct);
    if (!guardedThis || !bBound) return false;
    const bool bDescriptorSelected = guardedThis->_selectVolumeDescriptor();
    if (!guardedThis || !bDescriptorSelected) {
        if (guardedThis) guardedThis->releaseUnpackSource(pState);
        *pState = UNPACK_STATE();
        return false;
    }

    const qint64 nTotalSize = guardedThis->getSize();
    const qint32 nLogicalBlockSize = guardedThis->_getLogicalBlockSize();
    if (!guardedThis) return false;

    if (nLogicalBlockSize < 512 || nLogicalBlockSize > 8192) {
        guardedThis->releaseUnpackSource(pState);
        *pState = UNPACK_STATE();
        return false;
    }

    const qint64 nDescriptorOffset = guardedThis->_getPrimaryVolumeDescriptorOffset();
    if (!guardedThis || (nDescriptorOffset < 0)) {
        guardedThis->releaseUnpackSource(pState);
        *pState = UNPACK_STATE();
        return false;
    }
    const qint64 nRootRecordOffset = nDescriptorOffset + 156;

    if (nRootRecordOffset + 34 > nTotalSize) {
        guardedThis->releaseUnpackSource(pState);
        *pState = UNPACK_STATE();
        return false;
    }

    quint8 nRootRecordLength = guardedThis->read_uint8(nRootRecordOffset);
    if (!guardedThis) return false;

    if (nRootRecordLength < 34) {
        guardedThis->releaseUnpackSource(pState);
        *pState = UNPACK_STATE();
        return false;
    }

    const quint8 nRootExtAttrLength = guardedThis->read_uint8(nRootRecordOffset + 1);
    if (!guardedThis) return false;
    quint32 nRootExtentLocation = guardedThis->read_uint32(nRootRecordOffset + 2);
    if (!guardedThis) return false;
    const quint32 nRootExtentLocationBe = guardedThis->read_uint32(nRootRecordOffset + 6, true);
    if (!guardedThis) return false;
    quint32 nRootDataLength = guardedThis->read_uint32(nRootRecordOffset + 10);
    if (!guardedThis) return false;
    const quint32 nRootDataLengthBe = guardedThis->read_uint32(nRootRecordOffset + 14, true);
    if (!guardedThis) return false;

    if ((nRootExtentLocation != nRootExtentLocationBe) || (nRootDataLength != nRootDataLengthBe)) {
        guardedThis->releaseUnpackSource(pState);
        *pState = UNPACK_STATE();
        return false;
    }

    qint64 nRootOffset = (static_cast<qint64>(nRootExtentLocation) + nRootExtAttrLength) * nLogicalBlockSize;
    qint64 nRootSize = (qint64)nRootDataLength;

    if ((nRootOffset <= 0) || (nRootSize <= 0) || (nRootOffset >= nTotalSize) || (nRootSize > nTotalSize - nRootOffset)) {
        guardedThis->releaseUnpackSource(pState);
        *pState = UNPACK_STATE();
        return false;
    }

    // Build flat list of all records via BFS traversal
    QList<ARCHIVERECORD> listAllRecords = guardedThis->_collectAllRecords(nRootOffset, nRootSize, nLogicalBlockSize, pPdStruct);
    if (!guardedThis) return false;

    if (!isPdStructNotCanceled(pPdStruct)) {
        guardedThis->releaseUnpackSource(pState);
        *pState = UNPACK_STATE();
        return false;
    }

    ISO9660_UNPACK_CONTEXT *pContext = new (std::nothrow) ISO9660_UNPACK_CONTEXT;
    if (!pContext) {
        guardedThis->releaseUnpackSource(pState);
        *pState = UNPACK_STATE();
        return false;
    }
    pContext->nLogicalBlockSize = nLogicalBlockSize;
    pContext->listAllRecords = listAllRecords;

    pState->nNumberOfRecords = listAllRecords.count();
    pState->pContext = pContext;
    pState->nCurrentOffset = 0;
    pState->nTotalSize = nTotalSize;
    pState->nCurrentIndex = 0;
    pState->mapUnpackProperties = mapProperties;

    if (!guardedThis->validateAndFinalizeUnpackSource(pState, pContext, pPdStruct)) {
        if (!guardedThis) return false;
        pState->pContext = nullptr;
        guardedThis->releaseUnpackSource(pState);
        delete pContext;
        *pState = UNPACK_STATE();
        return false;
    }

    return true;
}

XBinary::ARCHIVERECORD XISO9660::infoCurrent(UNPACK_STATE *pState, PDSTRUCT *pPdStruct)
{
    QPointer<XISO9660> guardedThis(this);
    UNPACK_OPERATION_GUARD operationGuard(&m_bUnpackOperationInProgress, &m_bNestedUnpackInfoAuthorized);
    if (!operationGuard.isAllowed()) return XBinary::ARCHIVERECORD();

    ARCHIVERECORD record = {};

    if (!pState || !pState->pContext) return record;
    const bool bSourceCurrent = guardedThis->isUnpackSourceCurrent(pState, pPdStruct);
    if (!guardedThis || !bSourceCurrent) return record;

    ISO9660_UNPACK_CONTEXT *pContext = (ISO9660_UNPACK_CONTEXT *)pState->pContext;

    if (pState->nCurrentIndex >= 0 && pState->nCurrentIndex < pContext->listAllRecords.count()) {
        record = pContext->listAllRecords.at(pState->nCurrentIndex);
    }

    return record;
}

bool XISO9660::moveToNext(UNPACK_STATE *pState, PDSTRUCT *pPdStruct)
{
    QPointer<XISO9660> guardedThis(this);
    UNPACK_OPERATION_GUARD operationGuard(&m_bUnpackOperationInProgress);
    if (!operationGuard.isAcquired()) return false;

    if (!pState || !pState->pContext) return false;
    const bool bSourceCurrent = guardedThis->isUnpackSourceCurrent(pState, pPdStruct);
    if (!guardedThis || !bSourceCurrent || (pState->nCurrentIndex < 0) || (pState->nCurrentIndex >= pState->nNumberOfRecords)) {
        return false;
    }

    pState->nCurrentIndex++;

    ISO9660_UNPACK_CONTEXT *pContext = (ISO9660_UNPACK_CONTEXT *)pState->pContext;

    return (pState->nCurrentIndex < pContext->listAllRecords.count());
}

bool XISO9660::finishUnpack(UNPACK_STATE *pState, PDSTRUCT *pPdStruct)
{
    QPointer<XISO9660> guardedThis(this);
    UNPACK_OPERATION_GUARD operationGuard(&m_bUnpackOperationInProgress);
    if (!operationGuard.isAcquired()) return false;

    Q_UNUSED(pPdStruct)

    if (!pState) {
        return false;
    }

    if ((pState->pContext || !pState->baUnpackSourceToken.isEmpty()) && !guardedThis->ownsUnpackSource(pState)) return false;
    ISO9660_UNPACK_CONTEXT *pContext = static_cast<ISO9660_UNPACK_CONTEXT *>(pState->pContext);
    pState->pContext = nullptr;
    guardedThis->releaseUnpackSource(pState);
    if (!guardedThis) return false;
    delete pContext;
    if (!guardedThis) return false;

    pState->nCurrentOffset = 0;
    pState->nTotalSize = 0;
    pState->nCurrentIndex = 0;
    pState->nNumberOfRecords = 0;
    pState->mapUnpackProperties.clear();
    pState->mapArchiveProperties.clear();

    return true;
}

QString XISO9660::getSystemIdentifier()
{
    return m_sSystemIdentifier;
}

QString XISO9660::getVolumeIdentifier()
{
    return m_sVolumeIdentifier;
}

QString XISO9660::getVolumeSetIdentifier()
{
    return m_sVolumeSetIdentifier;
}

QString XISO9660::getPublisherIdentifier()
{
    return m_sPublisherIdentifier;
}

QString XISO9660::getDataPreparerIdentifier()
{
    return m_sDataPreparerIdentifier;
}

QString XISO9660::getApplicationIdentifier()
{
    return m_sApplicationIdentifier;
}

QString XISO9660::getCopyrightFileIdentifier()
{
    return m_sCopyrightFileIdentifier;
}

QString XISO9660::getAbstractFileIdentifier()
{
    return m_sAbstractFileIdentifier;
}

QString XISO9660::getBibliographicFileIdentifier()
{
    return m_sBibliographicFileIdentifier;
}

QString XISO9660::getCreationDateTime()
{
    return m_sCreationDateTime;
}

QString XISO9660::getModificationDateTime()
{
    return m_sModificationDateTime;
}

QString XISO9660::getExpirationDateTime()
{
    return m_sExpirationDateTime;
}

QString XISO9660::getEffectiveDateTime()
{
    return m_sEffectiveDateTime;
}

QList<QString> XISO9660::getSearchSignatures()
{
    // ISO 9660 has no fixed magic bytes at offset 0; the "CD001" signature
    // appears at offset 0x8001 (sector 16), so no start-of-file search is possible.
    return QList<QString>();
}

XBinary *XISO9660::createInstance(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    Q_UNUSED(bIsImage)
    Q_UNUSED(nModuleAddress)

    return new XISO9660(pDevice);
}

bool XISO9660::handleInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XISO9660> guardedThis(this);
    bool bResult = true;

    if (!isInternalInfoHandled()) {
        bResult = guardedThis->XArchive::handleInternalInfo(pPdStruct);
        if (!guardedThis || !bResult) return false;
        XArchive::INTERNAL_INFO *pInfo = static_cast<XArchive::INTERNAL_INFO *>(guardedThis->XArchive::getInternalInfo(pPdStruct));
        if (!guardedThis || !pInfo) return false;
        static_cast<XArchive::INTERNAL_INFO &>(guardedThis->m_internalInfo) = *pInfo;
    }

    return guardedThis && bResult;
}

void *XISO9660::getInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XISO9660> guardedThis(this);
    const bool bHandled = guardedThis->handleInternalInfo(pPdStruct);
    if (!guardedThis || !bHandled) return nullptr;

    return &guardedThis->m_internalInfo;
}

void XISO9660::setInternalInfo(void *pInternalInfo)
{
    if (pInternalInfo) {
        m_internalInfo = *static_cast<INTERNAL_INFO *>(pInternalInfo);
        XArchive::setInternalInfo(static_cast<XArchive::INTERNAL_INFO *>(&m_internalInfo));
    } else {
        m_internalInfo = INTERNAL_INFO();
        XArchive::setInternalInfo(nullptr);
    }
}
