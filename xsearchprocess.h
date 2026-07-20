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
#ifndef XSEARCHPROCESS_H
#define XSEARCHPROCESS_H

#include "xbinary.h"
#include "xthreadobject.h"

class XSearchProcess : public XThreadObject {
    Q_OBJECT

public:
    explicit XSearchProcess(QObject *pParent = nullptr);

    void setData(const XBinary::INDATA &inData, XBinary::XLOC location, qint64 nSize, const XBinary::XFSS_OPTIONS &ssOptions,
                 XBinary::_MEMORY_MAP *pMemoryMap, QVector<XBinary::MS_RECORD> *pListRecords, XBinary::PDSTRUCT *pPdStruct);
    void process() override;
    QString getTitle() override;

signals:
    void progressChanged(qint32 nValue, qint64 nCurrent, qint64 nTotal, const QString &sStatus);

private:
    static void pdStructCallback(void *pUserData, XBinary::PDSTRUCT *pPdStruct);
    void clearPdStructCallback();

private:
    XBinary::INDATA m_inData;
    XBinary::XLOC m_location;
    qint64 m_nSize;
    XBinary::XFSS_OPTIONS m_ssOptions;
    XBinary::_MEMORY_MAP *m_pMemoryMap;
    QVector<XBinary::MS_RECORD> *m_pListRecords;
    XBinary::PDSTRUCT *m_pPdStruct;
};

#endif  // XSEARCHPROCESS_H
