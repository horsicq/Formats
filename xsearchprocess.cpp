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
    clearPdStructCallback();

    if (m_pListRecords) {
        m_pListRecords->clear();
    }

    if (!m_pMemoryMap || !m_pListRecords) {
        clearPdStructCallback();
        return;
    }

    // Subscribe independently for the synchronous processing interval.  This
    // leaves an arbitrary caller-supplied legacy callback untouched and gives
    // destruction a drainable in-flight registration rather than a raw chain.
    if (m_pPdStruct) {
        m_pdCallbackSubscription = XBinary::subscribePdStructCallback(m_pPdStruct, XSearchProcess::pdStructCallback, this);
    }

    XBinary::PDSTRUCT pdStructEmpty = XBinary::createPdStruct();
    XBinary::PDSTRUCT *pPdStruct = m_pPdStruct;

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    if (!XBinary::isPdStructNotCanceled(pPdStruct) || (m_nSize < -1)) {
        clearPdStructCallback();
        return;
    }

    QIODevice *pDevice = XFormats::createDevice(m_inData);

    if (!pDevice || !pDevice->isOpen() || !pDevice->isReadable() || pDevice->isSequential()) {
        XFormats::removeDevice(pDevice, m_inData);
        clearPdStructCallback();
        return;
    }

    const qint64 nOriginalPosition = pDevice->pos();
    const qint64 nDeviceSize = pDevice->size();

    XBinary *pBinary = XFormats::createClass(m_inData.fileType, pDevice, m_inData.bIsImage, m_inData.nModuleAddress);

    if (pBinary) {
        connect(pBinary, SIGNAL(errorMessage(QString)), this, SIGNAL(errorMessage(QString)));

        XBinary::_MEMORY_MAP memoryMap = *m_pMemoryMap;
        if (memoryMap.listRecords.isEmpty()) {
            memoryMap = pBinary->getMemoryMap(XBinary::MAPMODE_UNKNOWN, pPdStruct);
        }

        const qint64 nOffset = XBinary::locToOffset(&memoryMap, m_location);
        const bool bRangeValid = XBinary::isPdStructNotCanceled(pPdStruct) && isValidMemoryMap(memoryMap, nDeviceSize) && (nOffset >= 0) && (nOffset <= nDeviceSize) &&
                                 ((m_nSize == -1) || (m_nSize <= (nDeviceSize - nOffset)));
        QVector<XBinary::MS_RECORD> listRecords;

        if (bRangeValid) {
            listRecords = pBinary->multiSearch_strings(&memoryMap, nOffset, m_nSize, m_ssOptions, pPdStruct);
        }

        bool bRecordsValid = bRangeValid;
        for (qint32 i = 0; bRecordsValid && (i < listRecords.count()) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
            XBinary::MS_RECORD *pRecord = &(listRecords[i]);

            if (pRecord->sValue.isEmpty()) {
                qint64 nRecordOffset = -1;
                if (getMSRecordOffset(memoryMap, *pRecord, nDeviceSize, &nRecordOffset)) {
                    pRecord->sValue = pBinary->read_msRecordString(*pRecord, nRecordOffset);
                } else {
                    bRecordsValid = false;
                }
            }
        }

        if (bRecordsValid && XBinary::isPdStructNotCanceled(pPdStruct)) {
            *m_pMemoryMap = memoryMap;
            *m_pListRecords = listRecords;
        }

        delete pBinary;
    }

    if (nOriginalPosition >= 0) {
        pDevice->seek(nOriginalPosition);
    }

    XFormats::removeDevice(pDevice, m_inData);

    clearPdStructCallback();
}

QString XSearchProcess::getTitle()
{
    return tr("Search strings");
}

void XSearchProcess::pdStructCallback(void *pUserData, XBinary::PDSTRUCT *pPdStruct)
{
    XSearchProcess *pSearchProcess = static_cast<XSearchProcess *>(pUserData);

    if (!pSearchProcess || !pPdStruct) {
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

    emit pSearchProcess->progressChanged(nValue, nCurrent, nTotal, sStatus);
}

void XSearchProcess::clearPdStructCallback()
{
    XBinary::unsubscribePdStructCallback(&m_pdCallbackSubscription);
}
