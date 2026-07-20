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

#include <QtGlobal>

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

    if (m_pPdStruct) {
        m_pPdStruct->pCallback = XSearchProcess::pdStructCallback;
        m_pPdStruct->pCallbackUserData = this;
    }
}

void XSearchProcess::process()
{
    if (m_pListRecords) {
        m_pListRecords->clear();
    }

    if (!m_pMemoryMap || !m_pListRecords) {
        clearPdStructCallback();
        return;
    }

    XBinary::PDSTRUCT pdStructEmpty = XBinary::createPdStruct();
    XBinary::PDSTRUCT *pPdStruct = m_pPdStruct;

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    QIODevice *pDevice = XFormats::createDevice(m_inData);

    if (!pDevice) {
        clearPdStructCallback();
        return;
    }

    XBinary *pBinary = XFormats::createClass(m_inData.fileType, pDevice, m_inData.bIsImage, m_inData.nModuleAddress);

    if (pBinary) {
        connect(pBinary, SIGNAL(errorMessage(QString)), this, SIGNAL(errorMessage(QString)));

        if (m_pMemoryMap->listRecords.isEmpty()) {
            *m_pMemoryMap = pBinary->getMemoryMap(XBinary::MAPMODE_UNKNOWN, pPdStruct);
        }

        qint64 nOffset = XBinary::locToOffset(m_pMemoryMap, m_location);
        *m_pListRecords = pBinary->multiSearch_strings(m_pMemoryMap, nOffset, m_nSize, m_ssOptions, pPdStruct);

        // Prefetch the string values. Honor cancellation: on a big result set this loop
        // would otherwise keep the worker busy long after Cancel, so the dialog cannot close.
        // Records left with an empty sValue are read back lazily by the model when displayed.
        for (qint32 i = 0; (i < m_pListRecords->count()) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
            XBinary::MS_RECORD *pRecord = &((*m_pListRecords)[i]);

            if (pRecord->sValue.isEmpty()) {
                qint64 nRecordOffset = -1;

                if (pRecord->nRegionIndex != -1) {
                    if (m_pMemoryMap->listRecords.at(pRecord->nRegionIndex).nOffset != -1) {
                        nRecordOffset = m_pMemoryMap->listRecords.at(pRecord->nRegionIndex).nOffset + pRecord->nRelOffset;
                    }
                } else {
                    nRecordOffset = pRecord->nRelOffset;
                }

                if (nRecordOffset != -1) {
                    bool bBigEndian = (m_ssOptions.endian == XBinary::ENDIAN_BIG);

                    if (pRecord->nValueType == XBinary::VT_U) {
                        if (pRecord->nInfo == XBinary::ENDIAN_BIG) {
                            bBigEndian = true;
                        } else if (pRecord->nInfo == XBinary::ENDIAN_LITTLE) {
                            bBigEndian = false;
                        }
                    }

                    pRecord->sValue = pBinary->read_value((XBinary::VT)(pRecord->nValueType), nRecordOffset, pRecord->nSize, bBigEndian).toString();
                }
            }
        }

        delete pBinary;
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

    for (qint32 i = 0; i < XBinary::N_NUMBER_PDRECORDS; i++) {
        const XBinary::PDRECORD *pRecord = &(pPdStruct->_pdRecord[i]);

        if (pRecord->bIsValid && pRecord->nTotal) {
            nCurrent = pRecord->nCurrent;
            nTotal = pRecord->nTotal;
            sStatus = pRecord->sStatus;
            break;
        }
    }

    qint32 nValue = XBinary::getPdStructPercentage(pPdStruct);
    nValue = qBound(0, nValue, 100);

    emit pSearchProcess->progressChanged(nValue, nCurrent, nTotal, sStatus);
}

void XSearchProcess::clearPdStructCallback()
{
    if (m_pPdStruct && (m_pPdStruct->pCallbackUserData == this)) {
        m_pPdStruct->pCallback = nullptr;
        m_pPdStruct->pCallbackUserData = nullptr;
    }
}
