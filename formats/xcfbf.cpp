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

    if (getSize() >= 24) {
        if (read_uint32(0, true) == 0xCAFEBABE) {
            if (read_uint32(4, true) > 10) {
                bResult = true;
            }
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
    return "JVM";
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
    Q_UNUSED(mapMode)

    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    qint64 nTotalSize = getSize();

    _MEMORY_MAP result = {};

    INFO info = getInfo();

    result.nBinarySize = nTotalSize;
    result.fileType = getFileType();
    result.mode = getMode();
    result.sArch = getArch();
    result.endian = getEndian();
    result.sType = getTypeAsString();

    qint32 nIndex = 0;

    _MEMORY_RECORD record = {};
    record.nAddress = -1;
    record.nOffset = 0;
    record.nSize = info.nSize;
    record.nIndex = 0;
    record.sName = tr("Data");
    record.nIndex = nIndex++;
    record.type = MMT_DATA;

    result.listRecords.append(record);

    if (nTotalSize > info.nSize) {
        record.nAddress = -1;
        record.nOffset = info.nSize;
        record.nSize = nTotalSize - info.nSize;
        record.nIndex = 1;
        record.sName = tr("Overlay");
        record.nIndex = nIndex++;
        record.type = MMT_OVERLAY;

        result.listRecords.append(record);
    }

    return result;
}

XBinary::FT XCFBF::getFileType()
{
    return FT_XCFBF;
}

XBinary::ENDIAN XCFBF::getEndian()
{
    return ENDIAN_LITTLE;
}

XBinary::MODE XCFBF::getMode()
{
    return MODE_32;
}
