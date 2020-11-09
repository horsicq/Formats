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
#include "xformats.h"

XFormats::XFormats(QObject *pParent) : QObject(pParent)
{

}

XBinary::_MEMORY_MAP XFormats::getMemoryMap(XBinary::FT fileType,QIODevice *pDevice,bool bIsImage,qint64 nImageBase)
{
    XBinary::_MEMORY_MAP result={};

    if(XBinary::checkFileType(XBinary::FT_BINARY,fileType))
    {
        XBinary binary(pDevice,bIsImage,nImageBase);
        result=binary.getMemoryMap();
    }
    else if(XBinary::checkFileType(XBinary::FT_COM,fileType))
    {
        XCOM com(pDevice,bIsImage,nImageBase);
        result=com.getMemoryMap();
    }
    else if(XBinary::checkFileType(XBinary::FT_MSDOS,fileType))
    {
        XMSDOS msdos(pDevice,bIsImage,nImageBase);
        result=msdos.getMemoryMap();
    }
    else if(XBinary::checkFileType(XBinary::FT_NE,fileType))
    {
        XNE ne(pDevice,bIsImage,nImageBase);
        result=ne.getMemoryMap();
    }
    else if(XBinary::checkFileType(XBinary::FT_LE,fileType))
    {
        XLE le(pDevice,bIsImage,nImageBase);
        result=le.getMemoryMap();
    }
    else if(XBinary::checkFileType(XBinary::FT_PE,fileType))
    {
        XPE pe(pDevice,bIsImage,nImageBase);
        result=pe.getMemoryMap();
    }
    else if(XBinary::checkFileType(XBinary::FT_ELF,fileType))
    {
        XELF elf(pDevice,bIsImage,nImageBase);
        result=elf.getMemoryMap();
    }
    else if(XBinary::checkFileType(XBinary::FT_MACH,fileType))
    {
        XMACH mach(pDevice,bIsImage,nImageBase);
        result=mach.getMemoryMap();
    }
#ifdef USE_DEX
    else if(XBinary::checkFileType(XBinary::FT_DEX,fileType))
    {
        XDEX dex(pDevice);
        result=dex.getMemoryMap();
    }
#endif
#ifdef USE_ARCHIVE
    else if(XBinary::checkFileType(XBinary::FT_ZIP,fileType))
    {
        XZip zip(pDevice);
        result=zip.getMemoryMap();
    }
#endif
    else
    {
        XBinary binary(pDevice,bIsImage,nImageBase);
        result=binary.getMemoryMap();
    }

    return result;
}

qint64 XFormats::getEntryPointAddress(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage, qint64 nImageBase)
{
    qint64 nResult=0;

    if(XBinary::checkFileType(XBinary::FT_BINARY,fileType))
    {
        XBinary binary(pDevice,bIsImage,nImageBase);
        nResult=binary.getEntryPointAddress();
    }
    else if(XBinary::checkFileType(XBinary::FT_COM,fileType))
    {
        XCOM com(pDevice,bIsImage,nImageBase);
        nResult=com.getEntryPointAddress();
    }
    else if(XBinary::checkFileType(XBinary::FT_MSDOS,fileType))
    {
        XMSDOS msdos(pDevice,bIsImage,nImageBase);
        nResult=msdos.getEntryPointAddress();
    }
    else if(XBinary::checkFileType(XBinary::FT_NE,fileType))
    {
        XNE ne(pDevice,bIsImage,nImageBase);
        nResult=ne.getEntryPointAddress();
    }
    else if(XBinary::checkFileType(XBinary::FT_LE,fileType))
    {
        XLE le(pDevice,bIsImage,nImageBase);
        nResult=le.getEntryPointAddress();
    }
    else if(XBinary::checkFileType(XBinary::FT_PE,fileType))
    {
        XPE pe(pDevice,bIsImage,nImageBase);
        nResult=pe.getEntryPointAddress();
    }
    else if(XBinary::checkFileType(XBinary::FT_ELF,fileType))
    {
        XELF elf(pDevice,bIsImage,nImageBase);
        nResult=elf.getEntryPointAddress();
    }
    else if(XBinary::checkFileType(XBinary::FT_MACH,fileType))
    {
        XMACH mach(pDevice,bIsImage,nImageBase);
        nResult=mach.getEntryPointAddress();
    }
    else
    {
        nResult=0;
    }

    return nResult;
}

qint64 XFormats::getEntryPointOffset(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage, qint64 nImageBase)
{
    qint64 nResult=0;

    if(XBinary::checkFileType(XBinary::FT_BINARY,fileType))
    {
        XBinary binary(pDevice,bIsImage,nImageBase);
        nResult=binary._getEntryPointOffset();
    }
    else if(XBinary::checkFileType(XBinary::FT_COM,fileType))
    {
        XCOM com(pDevice,bIsImage,nImageBase);
        nResult=com._getEntryPointOffset();
    }
    else if(XBinary::checkFileType(XBinary::FT_MSDOS,fileType))
    {
        XMSDOS msdos(pDevice,bIsImage,nImageBase);
        nResult=msdos._getEntryPointOffset();
    }
    else if(XBinary::checkFileType(XBinary::FT_NE,fileType))
    {
        XNE ne(pDevice,bIsImage,nImageBase);
        nResult=ne._getEntryPointOffset();
    }
    else if(XBinary::checkFileType(XBinary::FT_LE,fileType))
    {
        XLE le(pDevice,bIsImage,nImageBase);
        nResult=le._getEntryPointOffset();
    }
    else if(XBinary::checkFileType(XBinary::FT_PE,fileType))
    {
        XPE pe(pDevice,bIsImage,nImageBase);
        nResult=pe._getEntryPointOffset();
    }
    else if(XBinary::checkFileType(XBinary::FT_ELF,fileType))
    {
        XELF elf(pDevice,bIsImage,nImageBase);
        nResult=elf._getEntryPointOffset();
    }
    else if(XBinary::checkFileType(XBinary::FT_MACH,fileType))
    {
        XMACH mach(pDevice,bIsImage,nImageBase);
        nResult=mach._getEntryPointOffset();
    }
    else
    {
        nResult=0;
    }

    return nResult;
}
