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
#include "xne.h"

XNE::XNE(QIODevice *__pDevice, bool bIsImage, qint64 nImageBase) : XMSDOS(__pDevice,bIsImage,nImageBase)
{

}

bool XNE::isValid()
{
    bool bResult=false;

    quint16 magic=get_magic();

    if(magic==(quint16)XMSDOS_DEF::S_IMAGE_DOS_SIGNATURE)
    {
        qint32 lfanew=get_lfanew();

        if(lfanew>0)
        {
            quint32 signature=read_uint16(lfanew);

            if(signature==XNE_DEF::S_IMAGE_OS2_SIGNATURE)
            {
                bResult=true;
            }
        }
    }

    return bResult;
}

qint64 XNE::getImageOS2HeaderOffset()
{
    qint64 nResult=get_lfanew();

    if(!isOffsetValid(nResult))
    {
        nResult=-1;
    }

    return nResult;
}

XNE_DEF::IMAGE_OS2_HEADER XNE::getImageOS2Header()
{
    XNE_DEF::IMAGE_OS2_HEADER result={};

    qint64 nOffset=getImageOS2HeaderOffset();

    if(nOffset!=-1)
    {
        result.ne_magic=read_uint16(nOffset+offsetof(XNE_DEF::IMAGE_OS2_HEADER,ne_magic));
        result.ne_ver=read_uint8(nOffset+offsetof(XNE_DEF::IMAGE_OS2_HEADER,ne_ver));
        result.ne_rev=read_uint8(nOffset+offsetof(XNE_DEF::IMAGE_OS2_HEADER,ne_rev));
    }

    return result;
}

quint16 XNE::getImageOS2Header_magic()
{
    quint16 nResult=0;

    qint64 nOffset=getImageOS2HeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint16(nOffset+offsetof(XNE_DEF::IMAGE_OS2_HEADER,ne_magic));
    }

    return nResult;
}

quint8 XNE::getImageOS2Header_ver()
{
    quint8 nResult=0;

    qint64 nOffset=getImageOS2HeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint8(nOffset+offsetof(XNE_DEF::IMAGE_OS2_HEADER,ne_ver));
    }

    return nResult;
}

quint8 XNE::getImageOS2Header_rev()
{
    quint8 nResult=0;

    qint64 nOffset=getImageOS2HeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint8(nOffset+offsetof(XNE_DEF::IMAGE_OS2_HEADER,ne_rev));
    }

    return nResult;
}

void XNE::setImageOS2Header_magic(quint16 value)
{
    qint64 nOffset=getImageOS2HeaderOffset();

    if(nOffset!=-1)
    {
        write_uint16(nOffset+offsetof(XNE_DEF::IMAGE_OS2_HEADER,ne_magic),value);
    }
}

void XNE::setImageOS2Header_ver(quint8 value)
{
    qint64 nOffset=getImageOS2HeaderOffset();

    if(nOffset!=-1)
    {
        write_uint8(nOffset+offsetof(XNE_DEF::IMAGE_OS2_HEADER,ne_ver),value);
    }
}

void XNE::setImageOS2Header_rev(quint8 value)
{
    qint64 nOffset=getImageOS2HeaderOffset();

    if(nOffset!=-1)
    {
        write_uint8(nOffset+offsetof(XNE_DEF::IMAGE_OS2_HEADER,ne_rev),value);
    }
}

QMap<quint64, QString> XNE::getImageNEMagics()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0x454E,"IMAGE_OS2_SIGNATURE");

    return mapResult;
}

QMap<quint64, QString> XNE::getImageNEMagicsS()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0x454E,"OS2_SIGNATURE");

    return mapResult;
}
