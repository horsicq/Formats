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

XBinary::_MEMORY_MAP XFormats::getMemoryMap(XBinary::FT fileType,QIODevice *pDevice,bool bIsImage,qint64 nImageBase)
{
    XBinary::_MEMORY_MAP result={};

    if(fileType==XBinary::FT_BINARY)
    {
        XBinary binary(pDevice,bIsImage,nImageBase);
        result=binary.getMemoryMap();
    }
    else if(fileType==XBinary::FT_COM)
    {
        XCOM com(pDevice,bIsImage,nImageBase);
        result=com.getMemoryMap();
    }
    else if(fileType==XBinary::FT_MSDOS)
    {
        XMSDOS msdos(pDevice,bIsImage,nImageBase);
        result=msdos.getMemoryMap();
    }
    else if(fileType==XBinary::FT_NE)
    {
        XNE ne(pDevice,bIsImage,nImageBase);
        result=ne.getMemoryMap();
    }
    else if((fileType==XBinary::FT_LE)||(fileType==XBinary::FT_LX))
    {
        XLE le(pDevice,bIsImage,nImageBase);
        result=le.getMemoryMap();
    }
    else if((fileType==XBinary::FT_PE)||(fileType==XBinary::FT_PE32)||(fileType==XBinary::FT_PE64))
    {
        XPE pe(pDevice,bIsImage,nImageBase);
        result=pe.getMemoryMap();
    }
    else if((fileType==XBinary::FT_ELF)||(fileType==XBinary::FT_ELF32)||(fileType==XBinary::FT_ELF64))
    {
        XELF elf(pDevice,bIsImage,nImageBase);
        result=elf.getMemoryMap();
    }
    else if((fileType==XBinary::FT_MACH)||(fileType==XBinary::FT_MACH32)||(fileType==XBinary::FT_MACH64))
    {
        XMACH mach(pDevice,bIsImage,nImageBase);
        result=mach.getMemoryMap();
    }

    return result;
}

void XFormats::filterFileTypes(QSet<XBinary::FT> *pStFileTypes)
{
    if( pStFileTypes->contains(XBinary::FT_MSDOS)||
        pStFileTypes->contains(XBinary::FT_NE)||
        pStFileTypes->contains(XBinary::FT_LE)||
        pStFileTypes->contains(XBinary::FT_LX)||
        pStFileTypes->contains(XBinary::FT_PE)||
        pStFileTypes->contains(XBinary::FT_PE32)||
        pStFileTypes->contains(XBinary::FT_PE64)||
        pStFileTypes->contains(XBinary::FT_ELF)||
        pStFileTypes->contains(XBinary::FT_ELF32)||
        pStFileTypes->contains(XBinary::FT_ELF64)||
        pStFileTypes->contains(XBinary::FT_MACH)||
        pStFileTypes->contains(XBinary::FT_MACH32)||
        pStFileTypes->contains(XBinary::FT_MACH64))
    {
        pStFileTypes->remove(XBinary::FT_BINARY);
    }
    else
    {
        pStFileTypes->insert(XBinary::FT_COM);
    }
}

void XFormats::filterFileTypes(QSet<XBinary::FT> *pStFileTypes, XBinary::FT fileType)
{
    if(fileType==XBinary::FT_BINARY)
    {
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
        pStFileTypes->remove(XBinary::FT_MACH);
        pStFileTypes->remove(XBinary::FT_MACH32);
        pStFileTypes->remove(XBinary::FT_MACH64);
    }
    else if(fileType==XBinary::FT_COM)
    {
        pStFileTypes->remove(XBinary::FT_BINARY);
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
