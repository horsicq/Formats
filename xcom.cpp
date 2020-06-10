// copyright (c) 2020 hors<horsicq@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#include "xcom.h"

XCOM::XCOM(QIODevice *__pDevice, bool bIsImage, qint64 nImageBase)
{

}

XCOM::~XCOM()
{

}

bool XCOM::isValid()
{
    return true;
}

qint64 XCOM::getBaseAddress()
{
    return XCOM_DEF::S_ADDRESS_BEGIN;
}

XBinary::_MEMORY_MAP XCOM::getMemoryMap()
{
    _MEMORY_MAP result={};

    result.nBaseAddress=_getBaseAddress();
    result.nRawSize=getSize();
    result.nImageSize=getSize();
    result.fileType=FT_COM;
    result.mode=getMode();
    result.sArch=getArch();

    _MEMORY_RECORD record={};
    record.nAddress=_getBaseAddress();
    record.segment=ADDRESS_SEGMENT_FLAT;
    record.nOffset=0;
    record.nSize=getSize();
    record.nIndex=0;

    result.listRecords.append(record);

    return result;
}

QString XCOM::getArch()
{
    return "8086";
}

XBinary::MODE XCOM::getMode()
{
    return MODE_16;
}
