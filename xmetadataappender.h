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

#ifndef XMETADATAAPPENDER_H
#define XMETADATAAPPENDER_H

#include "xbinary.h"

class XMetadataAppender
{
public:
    XMetadataAppender(XBinary *pBinary, QVector<XBinary::XMETADATA_STRUCT> *pListResult, qint64 nOffsetBias = 0, const QString &sNamePrefix = QString())
        : m_pBinary(pBinary), m_pListResult(pListResult), m_nOffsetBias(nOffsetBias), m_sNamePrefix(sNamePrefix)
    {
    }

    void operator()(qint64 nOffset, qint64 nSize, XBinary::XMETADATA_ID id, const QString &sName, const QVariant &varValue) const
    {
        XBinary::XMETADATA_STRUCT record = {};
        record.nOffset = m_nOffsetBias + nOffset;
        record.nSize = nSize;
        record.nAddress = m_pBinary->offsetToAddress(record.nOffset);
        record.id = id;
        record.sName = m_sNamePrefix + sName;
        record.varValue = varValue;
        m_pListResult->append(record);
    }

    void operator()(qint64 nOffset, qint64 nSize, const QString &sName, const QVariant &varValue) const
    {
        operator()(nOffset, nSize, XBinary::XMETADATA_ID_UNKNOWN, sName, varValue);
    }

private:
    XBinary *m_pBinary;
    QVector<XBinary::XMETADATA_STRUCT> *m_pListResult;
    qint64 m_nOffsetBias;
    QString m_sNamePrefix;
};

#endif  // XMETADATAAPPENDER_H
