/* Copyright (c) 2025 hors<horsicq@gmail.com>
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
#include "xcfbf.h"

XCFBF::XCFBF(QIODevice *pDevice) : XBinary(pDevice)
{
}

XCFBF::~XCFBF()
{
}

bool XCFBF::isValid(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    bool bResult = false;

    if (getSize() >= 8) {
        _MEMORY_MAP memoryMap = XBinary::getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);

        if (compareSignature(&memoryMap, "D0CF11E0A1B11AE1", 0, pPdStruct)) {
            bResult = true;
        }
    }

    return bResult;
}

bool XCFBF::isValid(QIODevice *pDevice)
{
    XCFBF xcfbf(pDevice);

    return xcfbf.isValid();
}

QString XCFBF::getArch()
{
    return ""; // TODO
}

QString XCFBF::getVersion()
{
    QString sResult;

    return sResult;
}

QString XCFBF::getFileFormatExt()
{
    return "";
}

XBinary::_MEMORY_MAP XCFBF::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    return XBinary::getMemoryMap(mapMode, pPdStruct);
}

XBinary::FT XCFBF::getFileType()
{
    return FT_CFBF;
}

XBinary::ENDIAN XCFBF::getEndian()
{
    return ENDIAN_LITTLE;
}

XBinary::MODE XCFBF::getMode()
{
    return MODE_32;
}
