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

XFormats::XFormats(QObject *parent) : QObject(parent)
{

}

XBinary::_MEMORY_MAP XFormats::getMemoryMap(QIODevice *pDevice, XBinary::FT fileType)
{
    XBinary::_MEMORY_MAP result={};

    if(fileType==XBinary::FT_BINARY)
    {
        XBinary binary(pDevice);
        result=binary.getMemoryMap();
    }
    else if(fileType==XBinary::FT_MSDOS)
    {
        XMSDOS msdos(pDevice);
        result=msdos.getMemoryMap();
    }
    else if(fileType==XBinary::FT_NE)
    {
        XNE ne(pDevice);
        result=ne.getMemoryMap();
    }
    else if((fileType==XBinary::FT_LE)||(fileType==XBinary::FT_LX))
    {
        XLE le(pDevice);
        result=le.getMemoryMap();
    }
    else if((fileType==XBinary::FT_PE)||(fileType==XBinary::FT_PE32)||(fileType==XBinary::FT_PE64))
    {
        XPE pe(pDevice);
        result=pe.getMemoryMap();
    }
    else if((fileType==XBinary::FT_ELF)||(fileType==XBinary::FT_ELF32)||(fileType==XBinary::FT_ELF64))
    {
        XELF elf(pDevice);
        result=elf.getMemoryMap();
    }
    else if((fileType==XBinary::FT_MACH)||(fileType==XBinary::FT_MACH32)||(fileType==XBinary::FT_MACH64))
    {
        XMACH mach(pDevice);
        result=mach.getMemoryMap();
    }

    return result;
}

bool XFormats::isOverlayPresent(QIODevice *pDevice, XBinary::FT fileType)
{
    bool bResult=false;

    if(fileType==XBinary::FT_BINARY)
    {
        XBinary binary(pDevice);
        bResult=binary.isOverlayPresent();
    }
    else if(fileType==XBinary::FT_MSDOS)
    {
        XMSDOS msdos(pDevice);
        bResult=msdos.isOverlayPresent();
    }
    else if(fileType==XBinary::FT_NE)
    {
        XNE ne(pDevice);
        bResult=ne.isOverlayPresent();
    }
    else if((fileType==XBinary::FT_LE)||(fileType==XBinary::FT_LX))
    {
        XLE le(pDevice);
        bResult=le.isOverlayPresent();
    }
    else if((fileType==XBinary::FT_PE)||(fileType==XBinary::FT_PE32)||(fileType==XBinary::FT_PE64))
    {
        XPE pe(pDevice);
        bResult=pe.isOverlayPresent();
    }
    else if((fileType==XBinary::FT_ELF)||(fileType==XBinary::FT_ELF32)||(fileType==XBinary::FT_ELF64))
    {
        XELF elf(pDevice);
        bResult=elf.isOverlayPresent();
    }
    else if((fileType==XBinary::FT_MACH)||(fileType==XBinary::FT_MACH32)||(fileType==XBinary::FT_MACH64))
    {
        XMACH mach(pDevice);
        bResult=mach.isOverlayPresent();
    }

    return bResult;
}

qint64 XFormats::getOverlayOffset(QIODevice *pDevice, XBinary::FT fileType)
{
    qint64 nResult=-1;

    if(fileType==XBinary::FT_BINARY)
    {
        XBinary binary(pDevice);
        nResult=binary.getOverlayOffset();
    }
    else if(fileType==XBinary::FT_MSDOS)
    {
        XMSDOS msdos(pDevice);
        nResult=msdos.getOverlayOffset();
    }
    else if(fileType==XBinary::FT_NE)
    {
        XNE ne(pDevice);
        nResult=ne.getOverlayOffset();
    }
    else if((fileType==XBinary::FT_LE)||(fileType==XBinary::FT_LX))
    {
        XLE le(pDevice);
        nResult=le.getOverlayOffset();
    }
    else if((fileType==XBinary::FT_PE)||(fileType==XBinary::FT_PE32)||(fileType==XBinary::FT_PE64))
    {
        XPE pe(pDevice);
        nResult=pe.getOverlayOffset();
    }
    else if((fileType==XBinary::FT_ELF)||(fileType==XBinary::FT_ELF32)||(fileType==XBinary::FT_ELF64))
    {
        XELF elf(pDevice);
        nResult=elf.getOverlayOffset();
    }
    else if((fileType==XBinary::FT_MACH)||(fileType==XBinary::FT_MACH32)||(fileType==XBinary::FT_MACH64))
    {
        XMACH mach(pDevice);
        nResult=mach.getOverlayOffset();
    }

    return nResult;
}

void XFormats::filterFileTypes(QSet<XBinary::FT> *pStFileTypes, XBinary::FT fileType)
{
    if(fileType==XBinary::FT_BINARY)
    {
        pStFileTypes->remove(XBinary::FT_MSDOS);
        pStFileTypes->remove(XBinary::FT_NE);
        pStFileTypes->remove(XBinary::FT_LE);
        pStFileTypes->remove(XBinary::FT_LX);
        pStFileTypes->remove(XBinary::FT_PE);
        pStFileTypes->remove(XBinary::FT_PE32);
        pStFileTypes->remove(XBinary::FT_PE64);
        pStFileTypes->remove(XBinary::FT_ELF);
        pStFileTypes->remove(XBinary::FT_ELF32);
        pStFileTypes->remove(XBinary::FT_ELF64);
        pStFileTypes->remove(XBinary::FT_MACH);
        pStFileTypes->remove(XBinary::FT_MACH32);
        pStFileTypes->remove(XBinary::FT_MACH64);
    }
    else if(fileType==XBinary::FT_MSDOS)
    {
        pStFileTypes->remove(XBinary::FT_BINARY);
        pStFileTypes->remove(XBinary::FT_TEXT);
        pStFileTypes->remove(XBinary::FT_COM);
        pStFileTypes->remove(XBinary::FT_NE);
        pStFileTypes->remove(XBinary::FT_LE);
        pStFileTypes->remove(XBinary::FT_LX);
        pStFileTypes->remove(XBinary::FT_PE);
        pStFileTypes->remove(XBinary::FT_PE32);
        pStFileTypes->remove(XBinary::FT_PE64);
        pStFileTypes->remove(XBinary::FT_ELF);
        pStFileTypes->remove(XBinary::FT_ELF32);
        pStFileTypes->remove(XBinary::FT_ELF64);
        pStFileTypes->remove(XBinary::FT_MACH);
        pStFileTypes->remove(XBinary::FT_MACH32);
        pStFileTypes->remove(XBinary::FT_MACH64);
    }
    else if(fileType==XBinary::FT_NE)
    {
        pStFileTypes->remove(XBinary::FT_BINARY);
        pStFileTypes->remove(XBinary::FT_TEXT);
        pStFileTypes->remove(XBinary::FT_COM);
        pStFileTypes->remove(XBinary::FT_MSDOS);
        pStFileTypes->remove(XBinary::FT_LE);
        pStFileTypes->remove(XBinary::FT_LX);
        pStFileTypes->remove(XBinary::FT_PE);
        pStFileTypes->remove(XBinary::FT_PE32);
        pStFileTypes->remove(XBinary::FT_PE64);
        pStFileTypes->remove(XBinary::FT_ELF);
        pStFileTypes->remove(XBinary::FT_ELF32);
        pStFileTypes->remove(XBinary::FT_ELF64);
        pStFileTypes->remove(XBinary::FT_MACH);
        pStFileTypes->remove(XBinary::FT_MACH32);
        pStFileTypes->remove(XBinary::FT_MACH64);
    }
    else if((fileType==XBinary::FT_LE)||(fileType==XBinary::FT_LX))
    {
        pStFileTypes->remove(XBinary::FT_BINARY);
        pStFileTypes->remove(XBinary::FT_TEXT);
        pStFileTypes->remove(XBinary::FT_COM);
        pStFileTypes->remove(XBinary::FT_MSDOS);
        pStFileTypes->remove(XBinary::FT_NE);
        pStFileTypes->remove(XBinary::FT_PE);
        pStFileTypes->remove(XBinary::FT_PE32);
        pStFileTypes->remove(XBinary::FT_PE64);
        pStFileTypes->remove(XBinary::FT_ELF);
        pStFileTypes->remove(XBinary::FT_ELF32);
        pStFileTypes->remove(XBinary::FT_ELF64);
        pStFileTypes->remove(XBinary::FT_MACH);
        pStFileTypes->remove(XBinary::FT_MACH32);
        pStFileTypes->remove(XBinary::FT_MACH64);
    }
    else if(fileType==XBinary::FT_PE)
    {
        pStFileTypes->remove(XBinary::FT_BINARY);
        pStFileTypes->remove(XBinary::FT_TEXT);
        pStFileTypes->remove(XBinary::FT_COM);
        pStFileTypes->remove(XBinary::FT_MSDOS);
        pStFileTypes->remove(XBinary::FT_NE);
        pStFileTypes->remove(XBinary::FT_LE);
        pStFileTypes->remove(XBinary::FT_LX);
        pStFileTypes->remove(XBinary::FT_ELF);
        pStFileTypes->remove(XBinary::FT_ELF32);
        pStFileTypes->remove(XBinary::FT_ELF64);
        pStFileTypes->remove(XBinary::FT_MACH);
        pStFileTypes->remove(XBinary::FT_MACH32);
        pStFileTypes->remove(XBinary::FT_MACH64);
    }
    else if(fileType==XBinary::FT_ELF)
    {
        pStFileTypes->remove(XBinary::FT_BINARY);
        pStFileTypes->remove(XBinary::FT_TEXT);
        pStFileTypes->remove(XBinary::FT_COM);
        pStFileTypes->remove(XBinary::FT_MSDOS);
        pStFileTypes->remove(XBinary::FT_NE);
        pStFileTypes->remove(XBinary::FT_LE);
        pStFileTypes->remove(XBinary::FT_LX);
        pStFileTypes->remove(XBinary::FT_PE);
        pStFileTypes->remove(XBinary::FT_PE32);
        pStFileTypes->remove(XBinary::FT_PE64);
        pStFileTypes->remove(XBinary::FT_MACH);
        pStFileTypes->remove(XBinary::FT_MACH32);
        pStFileTypes->remove(XBinary::FT_MACH64);
    }
    else if(fileType==XBinary::FT_MACH)
    {
        pStFileTypes->remove(XBinary::FT_BINARY);
        pStFileTypes->remove(XBinary::FT_TEXT);
        pStFileTypes->remove(XBinary::FT_COM);
        pStFileTypes->remove(XBinary::FT_MSDOS);
        pStFileTypes->remove(XBinary::FT_NE);
        pStFileTypes->remove(XBinary::FT_LE);
        pStFileTypes->remove(XBinary::FT_LX);
        pStFileTypes->remove(XBinary::FT_PE);
        pStFileTypes->remove(XBinary::FT_PE32);
        pStFileTypes->remove(XBinary::FT_PE64);
        pStFileTypes->remove(XBinary::FT_ELF);
        pStFileTypes->remove(XBinary::FT_ELF32);
        pStFileTypes->remove(XBinary::FT_ELF64);
    }
}
