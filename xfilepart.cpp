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
#include "xfilepart.h"

XFilePart::XFilePart(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress) : XBinary(pDevice, bIsImage, nModuleAddress)
{
    m_parentFileType = FT_UNKNOWN;
    m_filePart = FILEPART_DATA;
    m_nFilePartOffset = 0;
    m_nFilePartSize = -1;
    m_nFilePartVirtualAddress = (XADDR)-1;
    m_nFilePartVirtualSize = -1;
}

XFilePart::~XFilePart()
{
}

bool XFilePart::isFilePart() const
{
    return true;
}

void XFilePart::setParentFileType(XBinary::FT fileType)
{
    m_parentFileType = fileType;
}

XBinary::FT XFilePart::getParentFileType() const
{
    return m_parentFileType;
}

void XFilePart::setFilePart(XBinary::FILEPART filePart)
{
    m_filePart = filePart;
}

XBinary::FILEPART XFilePart::getFilePart() const
{
    return m_filePart;
}

void XFilePart::setFilePartName(const QString &sFilePartName)
{
    m_sFilePartName = sFilePartName;
}

QString XFilePart::getFilePartName() const
{
    return m_sFilePartName;
}

void XFilePart::setFilePartOffset(qint64 nFilePartOffset)
{
    m_nFilePartOffset = nFilePartOffset;
}

qint64 XFilePart::getFilePartOffset() const
{
    return m_nFilePartOffset;
}

void XFilePart::setFilePartSize(qint64 nFilePartSize)
{
    m_nFilePartSize = nFilePartSize;
}

qint64 XFilePart::getFilePartSize() const
{
    return m_nFilePartSize;
}

void XFilePart::setFilePartVirtualAddress(XADDR nFilePartVirtualAddress)
{
    m_nFilePartVirtualAddress = nFilePartVirtualAddress;
}

XADDR XFilePart::getFilePartVirtualAddress() const
{
    return m_nFilePartVirtualAddress;
}

void XFilePart::setFilePartVirtualSize(qint64 nFilePartVirtualSize)
{
    m_nFilePartVirtualSize = nFilePartVirtualSize;
}

qint64 XFilePart::getFilePartVirtualSize() const
{
    return m_nFilePartVirtualSize;
}

qint64 XFilePart::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    qint64 nResult = m_nFilePartSize;

    if (nResult == -1) {
        nResult = getSize();
    }

    return nResult;
}

QList<XBinary::FPART> XFilePart::getFileParts(quint32 nFileParts, qint32 nLimit, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    QList<FPART> listResult;

    if ((nLimit != 0) && (nFileParts & m_filePart)) {
        qint64 nFilePartSize = getFileFormatSize();
        qint64 nFilePartVirtualSize = m_nFilePartVirtualSize;

        if (nFilePartVirtualSize == -1) {
            nFilePartVirtualSize = nFilePartSize;
        }

        listResult.append(getFPART(m_filePart, m_sFilePartName, m_nFilePartOffset, nFilePartSize, m_nFilePartVirtualAddress, nFilePartVirtualSize));
    }

    return listResult;
}
