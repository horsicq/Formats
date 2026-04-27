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
#ifndef XFILEPART_H
#define XFILEPART_H

#include "xbinary.h"

class XFilePart : public XBinary {
    Q_OBJECT

public:
    explicit XFilePart(QIODevice *pDevice = nullptr, bool bIsImage = false, XADDR nModuleAddress = -1);
    ~XFilePart();

    bool isFilePart() const;

    void setParentFileType(FT fileType);
    FT getParentFileType() const;

    void setFilePart(FILEPART filePart);
    FILEPART getFilePart() const;

    void setFilePartName(const QString &sFilePartName);
    QString getFilePartName() const;

    void setFilePartOffset(qint64 nFilePartOffset);
    qint64 getFilePartOffset() const;

    void setFilePartSize(qint64 nFilePartSize);
    qint64 getFilePartSize() const;

    void setFilePartVirtualAddress(XADDR nFilePartVirtualAddress);
    XADDR getFilePartVirtualAddress() const;

    void setFilePartVirtualSize(qint64 nFilePartVirtualSize);
    qint64 getFilePartVirtualSize() const;

    virtual qint64 getFileFormatSize(PDSTRUCT *pPdStruct = nullptr) override;
    virtual QList<FPART> getFileParts(quint32 nFileParts, qint32 nLimit = -1, PDSTRUCT *pPdStruct = nullptr) override;

private:
    FT m_parentFileType;
    FILEPART m_filePart;
    QString m_sFilePartName;
    qint64 m_nFilePartOffset;
    qint64 m_nFilePartSize;
    XADDR m_nFilePartVirtualAddress;
    qint64 m_nFilePartVirtualSize;
};

#endif  // XFILEPART_H
