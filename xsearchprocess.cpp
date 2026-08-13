/* Copyright (c) 2026 hors<horsicq@gmail.com>
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
#include "xsearchprocess.h"

#include "xformats.h"

#include <limits>
#include <QtGlobal>

namespace {
bool getMSRecordOffset(const XBinary::_MEMORY_MAP &memoryMap, const XBinary::MS_RECORD &record, qint64 nDeviceSize, qint64 *pnOffset)
{
    if (!pnOffset || (nDeviceSize < 0) || (record.nRelOffset > (quint64)(std::numeric_limits<qint64>::max)())) {
        return false;
    }

    const qint64 nRelOffset = (qint64)record.nRelOffset;
    const qint64 nRecordSize = (qint64)record.nSize;

    if (record.nRegionIndex == -1) {
        if ((nRelOffset > nDeviceSize) || (nRecordSize > (nDeviceSize - nRelOffset))) {
            return false;
        }
        *pnOffset = nRelOffset;
        return true;
    }

    const qint32 nRegionIndex = record.nRegionIndex;

    if ((nRegionIndex < 0) || (nRegionIndex >= memoryMap.listRecords.count())) {
        return false;
    }

    const XBinary::_MEMORY_RECORD &memoryRecord = memoryMap.listRecords.at(nRegionIndex);
    const qint64 nRegionOffset = memoryRecord.nOffset;
    const qint64 nRegionSize = memoryRecord.nSize;

    if (memoryRecord.bIsVirtual || (nRegionOffset < 0) || (nRegionSize < 0) || (nRelOffset > nRegionSize) ||
        (nRecordSize > (nRegionSize - nRelOffset)) || (nRelOffset > (std::numeric_limits<qint64>::max)() - nRegionOffset)) {
        return false;
    }

    *pnOffset = nRegionOffset + nRelOffset;
    return (*pnOffset <= nDeviceSize) && (nRecordSize <= (nDeviceSize - *pnOffset));
}

bool isValidMemoryMap(const XBinary::_MEMORY_MAP &memoryMap, qint64 nDeviceSize)
{
    if (nDeviceSize < 0) {
        return false;
    }

    for (const XBinary::_MEMORY_RECORD &record : memoryMap.listRecords) {
        if (!record.bIsVirtual && ((record.nOffset < 0) || (record.nSize < 0) || (record.nOffset > nDeviceSize) ||
                                   (record.nSize > (nDeviceSize - record.nOffset)))) {
            return false;
        }
    }

    return true;
}
}  // namespace

XSearchProcess::XSearchProcess(QObject *pParent) : XThreadObject(pParent)
{
    m_inData = {};
    m_location = XBinary::offsetToLoc(0);
    m_nSize = -1;
    m_ssOptions = {};
    m_pMemoryMap = nullptr;
    m_pListRecords = nullptr;
    m_pPdStruct = nullptr;
}

XSearchProcess::~XSearchProcess()
{
    clearPdStructCallback();
}

void XSearchProcess::setData(const XBinary::INDATA &inData, XBinary::XLOC location, qint64 nSize, const XBinary::XFSS_OPTIONS &ssOptions,
                             XBinary::_MEMORY_MAP *pMemoryMap, QVector<XBinary::MS_RECORD> *pListRecords, XBinary::PDSTRUCT *pPdStruct)
{
    m_inData = inData;
    m_location = location;
    m_nSize = nSize;
    m_ssOptions = ssOptions;
    m_pMemoryMap = pMemoryMap;
    m_pListRecords = pListRecords;
    m_pPdStruct = pPdStruct;
}

void XSearchProcess::process()
{
    QPointer<XSearchProcess> guardedThis(this);
    clearPdStructCallback();

    // Copy every member needed by the synchronous operation before installing
    // the progress callback. A direct progressChanged connection is allowed to
    // destroy this worker, in which case only these local values remain valid.
    const XBinary::INDATA inData = m_inData;
    const XBinary::XLOC location = m_location;
    const qint64 nRequestedSize = m_nSize;
    const XBinary::XFSS_OPTIONS ssOptions = m_ssOptions;
    XBinary::_MEMORY_MAP *pMemoryMap = m_pMemoryMap;
    QVector<XBinary::MS_RECORD> *pListRecords = m_pListRecords;
    XBinary::PDSTRUCT *pRequestedPdStruct = m_pPdStruct;

    if (pListRecords) {
        pListRecords->clear();
    }

    if (!pMemoryMap || !pListRecords) {
        clearPdStructCallback();
        return;
    }

    XBinary::_MEMORY_MAP memoryMap = *pMemoryMap;
    XBinary::PDSTRUCT pdStructEmpty = XBinary::createPdStruct();
    XBinary::PDSTRUCT *pPdStruct = pRequestedPdStruct ? pRequestedPdStruct : &pdStructEmpty;
    const XBinary::PDSTRUCTLIFETIME progressLifetime = XBinary::retainPdStructLifetime(pPdStruct);
    if (!XBinary::isPdStructLifetimeAlive(progressLifetime) ||
        !XBinary::isPdStructNotCanceled(pPdStruct) ||
        (nRequestedSize < -1)) {
        clearPdStructCallback();
        return;
    }

    // Subscribe independently for the synchronous processing interval.  This
    // leaves an arbitrary caller-supplied legacy callback untouched and gives
    // destruction a drainable in-flight registration rather than a raw chain.
    if (pRequestedPdStruct) {
        m_pdCallbackSubscription = XBinary::subscribePdStructCallback(pRequestedPdStruct, XSearchProcess::pdStructCallback, this);
    }

    QPointer<QIODevice> guardedDevice(XFormats::createDevice(inData));
    QPointer<XBinary> guardedBinary;
    qint64 nOriginalPosition = -1;

    // This cleanup never dereferences the worker. If a device callback or a
    // progress slot deleted it, its destructor already removed the callback
    // subscription while the local device/class still need deterministic
    // cleanup.
    const auto cleanup = [&]() {
        XBinary *pBinary = guardedBinary.data();
        if (pBinary) delete pBinary;

        if (guardedDevice && (nOriginalPosition >= 0)) {
            guardedDevice->seek(nOriginalPosition);
        }
        XFormats::removeDevice(guardedDevice.data(), inData);

        if (guardedThis) guardedThis->clearPdStructCallback();
    };

    const auto progressOwnerAlive = [&]() -> bool {
        return guardedThis && guardedDevice &&
               XBinary::isPdStructLifetimeAlive(progressLifetime);
    };

    if (!progressOwnerAlive() || !guardedDevice->isOpen() ||
        !guardedDevice->isReadable()) {
        cleanup();
        return;
    }
    const bool bSequential = guardedDevice->isSequential();
    if (!progressOwnerAlive() || bSequential) {
        cleanup();
        return;
    }

    nOriginalPosition = guardedDevice->pos();
    if (!progressOwnerAlive()) {
        cleanup();
        return;
    }
    const qint64 nDeviceSize = guardedDevice->size();
    if (!progressOwnerAlive() || (nDeviceSize < 0)) {
        cleanup();
        return;
    }

    guardedBinary = XFormats::createClass(inData.fileType, guardedDevice.data(),
                                          inData.bIsImage,
                                          inData.nModuleAddress);
    if (!progressOwnerAlive() || !guardedBinary) {
        cleanup();
        return;
    }

    connect(guardedBinary.data(), SIGNAL(errorMessage(QString)),
            guardedThis.data(), SIGNAL(errorMessage(QString)));

    if (memoryMap.listRecords.isEmpty()) {
        memoryMap = guardedBinary->getMemoryMap(XBinary::MAPMODE_UNKNOWN,
                                                pPdStruct);
        if (!progressOwnerAlive() || !guardedBinary) {
            cleanup();
            return;
        }
    }

    const qint64 nOffset = XBinary::locToOffset(&memoryMap, location);
    const bool bRangeValid =
        XBinary::isPdStructNotCanceled(pPdStruct) &&
        isValidMemoryMap(memoryMap, nDeviceSize) &&
        (nOffset >= 0) && (nOffset <= nDeviceSize) &&
        ((nRequestedSize == -1) ||
         (nRequestedSize <= (nDeviceSize - nOffset)));
    QVector<XBinary::MS_RECORD> listRecords;

    if (bRangeValid) {
        listRecords = guardedBinary->multiSearch_strings(
            &memoryMap, nOffset, nRequestedSize, ssOptions, pPdStruct);
        if (!progressOwnerAlive() || !guardedBinary) {
            cleanup();
            return;
        }
    }

    bool bRecordsValid = bRangeValid;
    for (qint32 i = 0; bRecordsValid && (i < listRecords.count()); i++) {
        if (!progressOwnerAlive() || !guardedBinary ||
            !XBinary::isPdStructNotCanceled(pPdStruct)) {
            bRecordsValid = false;
            break;
        }

        XBinary::MS_RECORD *pRecord = &(listRecords[i]);
        if (pRecord->sValue.isEmpty()) {
            qint64 nRecordOffset = -1;
            if (getMSRecordOffset(memoryMap, *pRecord, nDeviceSize,
                                  &nRecordOffset)) {
                const QString sValue = guardedBinary->read_msRecordString(
                    *pRecord, nRecordOffset);
                if (!progressOwnerAlive() || !guardedBinary) {
                    bRecordsValid = false;
                    break;
                }
                pRecord->sValue = sValue;
            } else {
                bRecordsValid = false;
            }
        }
    }

    if (bRecordsValid && progressOwnerAlive() && guardedBinary &&
        XBinary::isPdStructNotCanceled(pPdStruct)) {
        *pMemoryMap = memoryMap;
        *pListRecords = listRecords;
    }

    cleanup();
}

QString XSearchProcess::getTitle()
{
    return tr("Search strings");
}

void XSearchProcess::pdStructCallback(void *pUserData, XBinary::PDSTRUCT *pPdStruct)
{
    XSearchProcess *pSearchProcess = static_cast<XSearchProcess *>(pUserData);
    QPointer<XSearchProcess> guardedSearchProcess(pSearchProcess);

    if (!guardedSearchProcess || !pPdStruct) {
        return;
    }

    qint64 nCurrent = 0;
    qint64 nTotal = 0;
    QString sStatus;
    XBinary::PDSTRUCT snapshot = XBinary::getPdStructSnapshot(pPdStruct);

    for (qint32 i = 0; i < XBinary::N_NUMBER_PDRECORDS; i++) {
        const XBinary::PDRECORD *pRecord = &(snapshot._pdRecord[i]);

        if (pRecord->bIsValid && pRecord->nTotal) {
            nCurrent = pRecord->nCurrent;
            nTotal = pRecord->nTotal;
            sStatus = pRecord->sStatus;
            break;
        }
    }

    qint32 nValue = XBinary::getPdStructPercentage(&snapshot);
    nValue = qBound(0, nValue, 100);

    if (guardedSearchProcess) {
        // A direct receiver may delete the worker synchronously. Nothing in
        // this callback touches it after the signal returns.
        emit guardedSearchProcess->progressChanged(nValue, nCurrent, nTotal,
                                                   sStatus);
    }
}

void XSearchProcess::clearPdStructCallback()
{
    XBinary::unsubscribePdStructCallback(&m_pdCallbackSubscription);
}
