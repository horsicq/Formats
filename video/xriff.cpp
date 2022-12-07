/* Copyright (c) 2022 hors<horsicq@gmail.com>
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
#include "xriff.h"

XRiff::XRiff(QIODevice *pDevice) : XBinary(pDevice)
{
}

XRiff::~XRiff()
{
}

bool XRiff::isValid()
{
    bool bResult = false;

    if (getSize() > 0x20) {
        _MEMORY_MAP memoryMap = XBinary::getMemoryMap();

        if (compareSignature(&memoryMap, "000000..'ftyp'", 0)) {
            // TODO more checks
            bResult = true;
        }
    }

    return bResult;
}

bool XRiff::isValid(QIODevice *pDevice)
{
    XRiff xriff(pDevice);

    return xriff.isValid();
}

QString XRiff::getFileFormatString()
{
    QString sResult;

    sResult = "AVI";

    return sResult;
}

QString XRiff::getFileFormatExt()
{
    return "avi";
}

qint64 XRiff::getFileFormatSize()
{
    return _calculateRawSize();
}

XBinary::_MEMORY_MAP XRiff::getMemoryMap(PDSTRUCT *pPdStruct)
{
    PDSTRUCT ppStructEmpty = {};

    if (!pPdStruct) {
        pPdStruct = &ppStructEmpty;
    }

    XBinary::_MEMORY_MAP result = {};

    result.nRawSize = getSize();

    qint32 nIndex = 0;

    qint64 nOffset = 0;

    while (!(pPdStruct->bIsStop)) {
        quint32 nChunkSize = read_uint32(nOffset,true);
        QString sTag = read_ansiString(nOffset + 4,4);

        {
            _MEMORY_RECORD record = {};

            record.nIndex = nIndex++;
            record.type = MMT_FILESEGMENT;
            record.nOffset = nOffset;
            record.nSize = nChunkSize;
            record.nAddress = -1;
            record.sName = sTag;

            result.listRecords.append(record);
        }

        nOffset += nChunkSize;
    }

    return result;
}

XBinary::FT XRiff::getFileType()
{
    return FT_AVI;
}

