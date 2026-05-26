/* Copyright (c) 2026-2026 hors<horsicq@gmail.com>
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
#include "xhandler.h"

XHandler::XHandler(QObject *pParent) : QObject(pParent)
{
}

void XHandler::addRecord_Remove(QList<RECORD> *pListRecords, const QString &sFilename)
{
    RECORD record = {};
    record.xhandler = XHANDLER_REMOVE;
    record.mapOptions.insert(RECORD_OPTION_FILENAME_SOURCE, sFilename);
    pListRecords->append(record);
}

void XHandler::addRecord_Copy(QList<RECORD> *pListRecords, const QString &sFilenameSource, const QString &sFilenameDest)
{
    RECORD record = {};
    record.xhandler = XHANDLER_COPY;
    record.mapOptions.insert(RECORD_OPTION_FILENAME_SOURCE, sFilenameSource);
    record.mapOptions.insert(RECORD_OPTION_FILENAME_DEST, sFilenameDest);
    pListRecords->append(record);
}

void XHandler::addRecord_Move(QList<RECORD> *pListRecords, const QString &sFilenameSource, const QString &sFilenameDest)
{
    RECORD record = {};
    record.xhandler = XHANDLER_MOVE;
    record.mapOptions.insert(RECORD_OPTION_FILENAME_SOURCE, sFilenameSource);
    record.mapOptions.insert(RECORD_OPTION_FILENAME_DEST, sFilenameDest);
    pListRecords->append(record);
}

void XHandler::processRecords(QList<RECORD> *pListRecords, XBinary::PDSTRUCT *pDStruct)
{
    qint32 _nFreeIndex = XBinary::getFreeIndex(pDStruct);
    XBinary::setPdStructInit(pDStruct, _nFreeIndex, pListRecords->size());

    for (qint32 i = 0; i < pListRecords->size(); i++) {
        if (!XBinary::isPdStructNotCanceled(pDStruct)) {
            break;
        }

        const RECORD &record = pListRecords->at(i);
        const QString sSource = QDir::cleanPath(record.mapOptions.value(RECORD_OPTION_FILENAME_SOURCE).toString());
        const QString sDest = QDir::cleanPath(record.mapOptions.value(RECORD_OPTION_FILENAME_DEST).toString());

        if (record.xhandler == XHANDLER_REMOVE) {
            if (!QFile::remove(sSource)) {
                emit errorMessage(QString("%1: %2").arg(tr("Cannot remove file"), sSource));
            }
        } else if (record.xhandler == XHANDLER_COPY) {
            if (QFile::exists(sDest)) {
                QFile::remove(sDest);
            }
            if (!QFile::copy(sSource, sDest)) {
                emit errorMessage(QString("%1: %2").arg(tr("Cannot copy file"), sSource));
            }
        } else if (record.xhandler == XHANDLER_MOVE) {
            if (QFile::exists(sDest)) {
                QFile::remove(sDest);
            }
            if (!QFile::rename(sSource, sDest)) {
                emit errorMessage(QString("%1: %2").arg(tr("Cannot move file"), sSource));
            }
        }

        XBinary::setPdStructCurrentIncrement(pDStruct, _nFreeIndex);
    }

    XBinary::setPdStructFinished(pDStruct, _nFreeIndex);
}
