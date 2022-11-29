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
#include "xtiff.h"

XTiff::XTiff(QIODevice *pDevice) : XBinary(pDevice)
{
}

XTiff::~XTiff()
{
}

bool XTiff::isValid()
{
    bool bIsValid = false;

    if (getSize() >= 8) {
        _MEMORY_MAP memoryMap = XBinary::getMemoryMap();
        if (compareSignature(&memoryMap, "'MM'002A") || compareSignature(&memoryMap, "'II'2A00")) {
            bool bIsBigEndian = isBigEndian();

            quint32 nOffset = read_uint32(4,bIsBigEndian);

            if((nOffset > 0) && (nOffset < getSize()))
            {
                bIsValid = true;
            }
        }
    }

    return bIsValid;
}

bool XTiff::isValid(QIODevice *pDevice)
{
    XTiff xtiff(pDevice);

    return xtiff.isValid();
}

XBinary::_MEMORY_MAP XTiff::getMemoryMap(PDSTRUCT *pPdStruct)
{
    return XBinary::getMemoryMap(pPdStruct);
}

XBinary::FT XTiff::getFileType()
{
    return FT_TIFF;
}

QString XTiff::getFileFormatString()
{
    QString sResult;

    sResult = QString("TIFF");

    return sResult;
}

QString XTiff::getFileFormatExt()
{
    return "tiff";
}

qint64 XTiff::getFileFormatSize()
{
    return XBinary::getFileFormatSize();
}

bool XTiff::isBigEndian()
{
    quint32 nEndian = read_uint32(0);

    return (nEndian == 0x2A004D4D);
}

