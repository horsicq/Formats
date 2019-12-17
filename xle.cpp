// copyright (c) 2019 hors<horsicq@gmail.com>
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
#include "xle.h"

XLE::XLE(QIODevice *__pDevice, bool bIsImage, qint64 nImageBase) : XMSDOS(__pDevice,bIsImage,nImageBase)
{

}

bool XLE::isValid()
{
    bool bResult=false;

    quint16 magic=get_magic();

    if(magic==(quint16)XMSDOS_DEF::S_IMAGE_DOS_SIGNATURE)
    {
        qint32 lfanew=get_lfanew();

        if(lfanew>0)
        {
            quint32 signature=read_uint32(lfanew);

            if(signature==XLE_DEF::S_IMAGE_VXD_SIGNATURE)
            {
                bResult=true;
            }
        }
    }

    return bResult;
}

qint64 XLE::getImageVxdHeaderOffset()
{
    qint64 nResult=get_lfanew();

    if(!isOffsetValid(nResult))
    {
        nResult=-1;
    }

    return nResult;
}

XLE_DEF::IMAGE_VXD_HEADER XLE::getImageVxdHeader()
{
    XLE_DEF::IMAGE_VXD_HEADER result={};

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        result.e32_magic=read_uint16(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_magic));
        result.e32_border=read_uint8(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_border));
        result.e32_worder=read_uint8(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_worder));
    }

    return result;
}

quint16 XLE::getImageVxdHeader_magic()
{
    quint16 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint16(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_magic));
    }

    return nResult;
}

quint8 XLE::getImageVxdHeader_border()
{
    quint8 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint8(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_border));
    }

    return nResult;
}

quint8 XLE::getImageVxdHeader_worder()
{
    quint8 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint8(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_worder));
    }

    return nResult;
}

void XLE::setImageVxdHeader_magic(quint16 value)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint16(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_magic),value);
    }
}

void XLE::setImageVxdHeader_border(quint8 value)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint8(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_border),value);
    }
}

void XLE::setImageVxdHeader_worder(quint8 value)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint8(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_worder),value);
    }
}

QMap<quint64, QString> XLE::getImageLEMagics()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0x454C,"IMAGE_OS2_SIGNATURE_LE");

    return mapResult;
}

QMap<quint64, QString> XLE::getImageLEMagicsS()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0x454C,"OS2_SIGNATURE_LE");

    return mapResult;
}
