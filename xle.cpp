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
#include "xle.h"

XLE::XLE(QIODevice *pDevice, bool bIsImage, qint64 nImageBase) : XMSDOS(pDevice,bIsImage,nImageBase)
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
            quint16 signature=read_uint16(lfanew);

            if((signature==XLE_DEF::S_IMAGE_VXD_SIGNATURE)||(signature==XLE_DEF::S_IMAGE_LX_SIGNATURE))
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

    if(!_isOffsetValid(nResult))
    {
        nResult=-1;
    }

    return nResult;
}

qint64 XLE::getImageVxdHeaderSize()
{
    return sizeof(XLE_DEF::IMAGE_VXD_HEADER);
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
        result.e32_level=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_level));
        result.e32_cpu=read_uint16(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_cpu));
        result.e32_os=read_uint16(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_os));
        result.e32_ver=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_ver));
        result.e32_mflags=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_mflags));
        result.e32_mpages=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_mpages));
        result.e32_startobj=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_startobj));
        result.e32_eip=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_eip));
        result.e32_stackobj=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_stackobj));
        result.e32_esp=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_esp));
        result.e32_pagesize=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_pagesize));
        result.e32_lastpagesize=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_lastpagesize));
        result.e32_fixupsize=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_fixupsize));
        result.e32_fixupsum=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_fixupsum));
        result.e32_ldrsize=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_ldrsize));
        result.e32_ldrsum=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_ldrsum));
        result.e32_objtab=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_objtab));
        result.e32_objcnt=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_objcnt));
        result.e32_objmap=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_objmap));
        result.e32_itermap=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_itermap));
        result.e32_rsrctab=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_rsrctab));
        result.e32_rsrccnt=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_rsrccnt));
        result.e32_restab=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_restab));
        result.e32_enttab=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_enttab));
        result.e32_dirtab=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_dirtab));
        result.e32_dircnt=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_dircnt));
        result.e32_fpagetab=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_fpagetab));
        result.e32_frectab=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_frectab));
        result.e32_impmod=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_impmod));
        result.e32_impmodcnt=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_impmodcnt));
        result.e32_impproc=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_impproc));
        result.e32_pagesum=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_pagesum));
        result.e32_datapage=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_datapage));
        result.e32_preload=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_preload));
        result.e32_nrestab=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_nrestab));
        result.e32_cbnrestab=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_cbnrestab));
        result.e32_nressum=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_nressum));
        result.e32_autodata=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_autodata));
        result.e32_debuginfo=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_debuginfo));
        result.e32_debuglen=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_debuglen));
        result.e32_instpreload=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_instpreload));
        result.e32_instdemand=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_instdemand));
        result.e32_heapsize=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_heapsize));
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

quint32 XLE::getImageVxdHeader_level()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_level));
    }

    return nResult;
}

quint16 XLE::getImageVxdHeader_cpu()
{
    quint16 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint16(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_cpu));
    }

    return nResult;
}

quint16 XLE::getImageVxdHeader_os()
{
    quint16 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint16(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_os));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_ver()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_ver));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_mflags()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_mflags));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_mpages()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_mpages));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_startobj()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_startobj));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_eip()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_eip));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_stackobj()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_stackobj));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_esp()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_esp));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_pagesize()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_pagesize));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_lastpagesize()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_lastpagesize));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_fixupsize()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_fixupsize));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_fixupsum()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_fixupsum));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_ldrsize()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_ldrsize));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_ldrsum()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_ldrsum));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_objtab()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_objtab));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_objcnt()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_objcnt));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_objmap()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_objmap));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_itermap()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_itermap));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_rsrctab()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_rsrctab));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_rsrccnt()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_rsrccnt));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_restab()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_restab));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_enttab()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_enttab));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_dirtab()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_dirtab));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_dircnt()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_dircnt));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_fpagetab()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_fpagetab));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_frectab()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_frectab));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_impmod()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_impmod));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_impmodcnt()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_impmodcnt));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_impproc()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_impproc));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_pagesum()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_pagesum));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_datapage()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_datapage));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_preload()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_preload));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_nrestab()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_nrestab));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_cbnrestab()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_cbnrestab));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_nressum()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_nressum));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_autodata()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_autodata));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_debuginfo()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_debuginfo));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_debuglen()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_debuglen));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_instpreload()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_instpreload));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_instdemand()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_instdemand));
    }

    return nResult;
}

quint32 XLE::getImageVxdHeader_heapsize()
{
    quint32 nResult=0;

    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        nResult=read_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_heapsize));
    }

    return nResult;
}

void XLE::setImageVxdHeader_magic(quint16 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint16(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_magic),nValue);
    }
}

void XLE::setImageVxdHeader_border(quint8 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint8(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_border),nValue);
    }
}

void XLE::setImageVxdHeader_worder(quint8 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint8(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_worder),nValue);
    }
}

void XLE::setImageVxdHeader_level(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_level),nValue);
    }
}

void XLE::setImageVxdHeader_cpu(quint16 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint16(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_cpu),nValue);
    }
}

void XLE::setImageVxdHeader_os(quint16 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint16(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_os),nValue);
    }
}

void XLE::setImageVxdHeader_ver(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_ver),nValue);
    }
}

void XLE::setImageVxdHeader_mflags(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_mflags),nValue);
    }
}

void XLE::setImageVxdHeader_mpages(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_mpages),nValue);
    }
}

void XLE::setImageVxdHeader_startobj(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_startobj),nValue);
    }
}

void XLE::setImageVxdHeader_eip(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_eip),nValue);
    }
}

void XLE::setImageVxdHeader_stackobj(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_stackobj),nValue);
    }
}

void XLE::setImageVxdHeader_esp(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_esp),nValue);
    }
}

void XLE::setImageVxdHeader_pagesize(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_pagesize),nValue);
    }
}

void XLE::setImageVxdHeader_lastpagesize(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_lastpagesize),nValue);
    }
}

void XLE::setImageVxdHeader_fixupsize(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_fixupsize),nValue);
    }
}

void XLE::setImageVxdHeader_fixupsum(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_fixupsum),nValue);
    }
}

void XLE::setImageVxdHeader_ldrsize(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_ldrsize),nValue);
    }
}

void XLE::setImageVxdHeader_ldrsum(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_ldrsum),nValue);
    }
}

void XLE::setImageVxdHeader_objtab(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_objtab),nValue);
    }
}

void XLE::setImageVxdHeader_objcnt(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_objcnt),nValue);
    }
}

void XLE::setImageVxdHeader_objmap(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_objmap),nValue);
    }
}

void XLE::setImageVxdHeader_itermap(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_itermap),nValue);
    }
}

void XLE::setImageVxdHeader_rsrctab(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_rsrctab),nValue);
    }
}

void XLE::setImageVxdHeader_rsrccnt(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_rsrccnt),nValue);
    }
}

void XLE::setImageVxdHeader_restab(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_restab),nValue);
    }
}

void XLE::setImageVxdHeader_enttab(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_enttab),nValue);
    }
}

void XLE::setImageVxdHeader_dirtab(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_dirtab),nValue);
    }
}

void XLE::setImageVxdHeader_dircnt(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_dircnt),nValue);
    }
}

void XLE::setImageVxdHeader_fpagetab(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_fpagetab),nValue);
    }
}

void XLE::setImageVxdHeader_frectab(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_frectab),nValue);
    }
}

void XLE::setImageVxdHeader_impmod(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_impmod),nValue);
    }
}

void XLE::setImageVxdHeader_impmodcnt(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_impmodcnt),nValue);
    }
}

void XLE::setImageVxdHeader_impproc(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_impproc),nValue);
    }
}

void XLE::setImageVxdHeader_pagesum(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_pagesum),nValue);
    }
}

void XLE::setImageVxdHeader_datapage(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_datapage),nValue);
    }
}

void XLE::setImageVxdHeader_preload(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_preload),nValue);
    }
}

void XLE::setImageVxdHeader_nrestab(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_nrestab),nValue);
    }
}

void XLE::setImageVxdHeader_cbnrestab(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_cbnrestab),nValue);
    }
}

void XLE::setImageVxdHeader_nressum(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_nressum),nValue);
    }
}

void XLE::setImageVxdHeader_autodata(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_autodata),nValue);
    }
}

void XLE::setImageVxdHeader_debuginfo(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_debuginfo),nValue);
    }
}

void XLE::setImageVxdHeader_debuglen(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_debuglen),nValue);
    }
}

void XLE::setImageVxdHeader_instpreload(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_instpreload),nValue);
    }
}

void XLE::setImageVxdHeader_instdemand(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_instdemand),nValue);
    }
}

void XLE::setImageVxdHeader_heapsize(quint32 nValue)
{
    qint64 nOffset=getImageVxdHeaderOffset();

    if(nOffset!=-1)
    {
        write_uint32(nOffset+offsetof(XLE_DEF::IMAGE_VXD_HEADER,e32_heapsize),nValue);
    }
}

XLE_DEF::o32_obj XLE::_read_o32_obj(qint64 nOffset)
{
    XLE_DEF::o32_obj result={};

    result.o32_size=read_uint32(nOffset+offsetof(XLE_DEF::o32_obj,o32_size));
    result.o32_base=read_uint32(nOffset+offsetof(XLE_DEF::o32_obj,o32_base));
    result.o32_flags=read_uint32(nOffset+offsetof(XLE_DEF::o32_obj,o32_flags));
    result.o32_pagemap=read_uint32(nOffset+offsetof(XLE_DEF::o32_obj,o32_pagemap));
    result.o32_mapsize=read_uint32(nOffset+offsetof(XLE_DEF::o32_obj,o32_mapsize));
    result.o32_reserved=read_uint32(nOffset+offsetof(XLE_DEF::o32_obj,o32_reserved));

    return result;
}

XLE_DEF::o32_map XLE::_read_o32_map(qint64 nOffset)
{
    XLE_DEF::o32_map result={};

    result.o32_pagedataoffset=read_uint32(nOffset+offsetof(XLE_DEF::o32_map,o32_pagedataoffset));
    result.o32_pagesize=read_uint32(nOffset+offsetof(XLE_DEF::o32_map,o32_pagesize));
    result.o32_pageflags=read_uint32(nOffset+offsetof(XLE_DEF::o32_map,o32_pageflags));

    return result;
}

QList<XLE_DEF::o32_obj> XLE::getObjects()
{
    QList<XLE_DEF::o32_obj> listResult;

    qint64 nObjOffset=getImageVxdHeaderOffset()+getImageVxdHeader_objtab();
    quint32 nNumberOfObjects=getImageVxdHeader_objcnt();

    for(quint32 i=0;i<nNumberOfObjects;i++)
    {
        XLE_DEF::o32_obj record=_read_o32_obj(nObjOffset);

        listResult.append(record);

        nObjOffset+=sizeof(XLE_DEF::o32_obj);
    }

    return listResult;
}

QList<XLE_DEF::o32_map> XLE::getMaps()
{
    QList<XLE_DEF::o32_map> listResult;

    qint64 nMapOffset=getImageVxdHeaderOffset()+getImageVxdHeader_objmap();
    quint32 nMapCount=getImageVxdHeader_itermap();

    for(quint32 i=0;i<nMapCount;i++)
    {
        XLE_DEF::o32_map record=_read_o32_map(nMapOffset);

        listResult.append(record);

        nMapOffset+=sizeof(XLE_DEF::o32_map);
    }

    return listResult;
}

XBinary::_MEMORY_MAP XLE::getMemoryMap()
{
    XBinary::_MEMORY_MAP result={};

    result.sArch=getArch();
    result.mode=getMode();
    result.bIsBigEndian=isBigEndian();
    result.sType=getTypeAsString();

    if(result.mode==MODE_16SEG)
    {
        result.fileType=FT_LE;
    }
    else if(result.mode==MODE_32)
    {
        result.fileType=FT_LX;
    }

    result.nRawSize=getSize();
    // TODO Image size CONST
    // TODO Image Base
//    result.nImageSize=0xFFFF;

    QList<XLE_DEF::o32_map> listMaps=XLE::getMaps();

    QList<XLE_DEF::o32_obj> listObjects=XLE::getObjects();

    int nCount=listObjects.count();

    for(int i=0;i<nCount;i++)
    {
        // TODO
    }

    return result;
}

XBinary::MODE XLE::getMode()
{
    MODE result=MODE_16SEG;

    qint32 lfanew=get_lfanew();
    quint16 signature=read_uint16(lfanew);

    if(signature==XLE_DEF::S_IMAGE_VXD_SIGNATURE)
    {
        result=MODE_16SEG;
    }
    else if(signature==XLE_DEF::S_IMAGE_LX_SIGNATURE)
    {
        result=MODE_32;
    }

    return result;
}

QString XLE::getArch()
{
    return getImageLECpusS().value(getImageVxdHeader_cpu(),QString("UNKNOWN"));
}

bool XLE::isBigEndian()
{
    return false;
}

qint64 XLE::getEntryPointOffset(XBinary::_MEMORY_MAP *pMemoryMap)
{
    Q_UNUSED(pMemoryMap)
    // TODO
    // startobj:eip
    return 0;
}

int XLE::getType()
{
    return TYPE_EXE; // TODO
}

QString XLE::typeIdToString(int nType)
{
    QString sResult="Unknown"; // mb TODO translate

    switch(nType)
    {
        case TYPE_UNKNOWN:      sResult=QString("Unknown");     break; // mb TODO translate
        case TYPE_EXE:          sResult=QString("EXE");         break;
    }

    return sResult;
}

QMap<quint64, QString> XLE::getImageLEMagics()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0x454C,"IMAGE_VXD_SIGNATURE");
    mapResult.insert(0x584C,"IMAGE_LX_SIGNATURE");

    return mapResult;
}

QMap<quint64, QString> XLE::getImageLEMagicsS()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0x454C,"VXD_SIGNATURE");
    mapResult.insert(0x584C,"LX_SIGNATURE");

    return mapResult;
}

QMap<quint64, QString> XLE::getImageLECpusS()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0x01,"80286");
    mapResult.insert(0x02,"80386");
    mapResult.insert(0x03,"80486");
    mapResult.insert(0x04,"80586");
    mapResult.insert(0x20,"i860");
    mapResult.insert(0x21,"N11");
    mapResult.insert(0x40,"R2000");
    mapResult.insert(0x41,"R6000");
    mapResult.insert(0x42,"R4000");

    return mapResult;
}

QMap<quint64, QString> XLE::getImageLEOssS()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0x00,"Unknown (any new-format OS)");
    mapResult.insert(0x01,"OS/2 (default)");
    mapResult.insert(0x02,"Windows");
    mapResult.insert(0x03,"DOS 4.x");
    mapResult.insert(0x04,"Windows 386");

    return mapResult;
}

QMap<quint64, QString> XLE::getImageLEMflagsS()
{
    QMap<quint64, QString> mapResult;

    // TODO

    return mapResult;
}
