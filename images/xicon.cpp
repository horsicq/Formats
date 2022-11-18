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
#include "xicon.h"

XIcon::XIcon(QIODevice *pDevice)
    : XBinary(pDevice)
{
}

XIcon::~XIcon()
{
}

bool XIcon::isValid()
{
    bool bResult = true;
    // TODO more checks !!!
    if (getSize() > 4) {
        quint32 nHeader = read_uint32(0);

        if ((nHeader == 0x00010000) || (nHeader == 0x00020000)) {
            bResult = true;
        }
    }

    return bResult;
}

bool XIcon::isValid(QIODevice *pDevice)
{
    XIcon xicon(pDevice);

    return xicon.isValid();
}

XBinary::FT XIcon::getFileType()
{
    FT result = FT_ICO;

    quint32 nHeader = read_uint32(0);

    if (nHeader == 0x00010000) {
        result = FT_ICO;
    } else if (nHeader == 0x00020000) {
        result = FT_CUR;
    }

    return result;
}

QString XIcon::getFileFormatString()
{
    QString sResult;

    if (getFileType() == FT_CUR) {
        sResult = QString("CUR");
    } else {
        sResult = QString("ICO");
    }

    // TODO more info

    return sResult;
}

QString XIcon::getFileFormatExt()
{
    QString sResult;

    if (getFileType() == FT_CUR) {
        sResult = QString("cur");
    } else {
        sResult = QString("ico");
    }

    return sResult;
}

qint64 XIcon::getFileFormatSize()
{
    return _calculateRawSize();
}

XBinary::_MEMORY_MAP XIcon::getMemoryMap()
{
    _MEMORY_MAP result = {};

    return result;
}

XIcon::ICONDIR XIcon::readICONDIR()
{
    ICONDIR result = {};

    result.reserved = read_uint32(offsetof(ICONDIR, reserved));
    result.type = read_uint32(offsetof(ICONDIR, type));
    result.count = read_uint32(offsetof(ICONDIR, count));

    return result;
}

XIcon::ICONDIRENTRY XIcon::readICONDIRENTRY(qint64 nOffset)
{
    ICONDIRENTRY result = {};

    result.width = read_uint32(nOffset + offsetof(ICONDIRENTRY, width));
    result.height = read_uint32(nOffset + offsetof(ICONDIRENTRY, height));
    result.numcolors = read_uint32(nOffset + offsetof(ICONDIRENTRY, numcolors));
    result.reserved = read_uint32(nOffset + offsetof(ICONDIRENTRY, reserved));
    result.cp = read_uint32(nOffset + offsetof(ICONDIRENTRY, cp));
    result.bpp = read_uint32(nOffset + offsetof(ICONDIRENTRY, bpp));
    result.size = read_uint32(nOffset + offsetof(ICONDIRENTRY, size));
    result.offset = read_uint32(nOffset + offsetof(ICONDIRENTRY, offset));

    return result;
}
