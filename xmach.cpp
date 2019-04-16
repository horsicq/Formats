// copyright (c) 2017-2019 hors<horsicq@gmail.com>
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
#include "xmach.h"

XMACH::XMACH(QIODevice *__pDevice, bool bIsImage, qint64 nImageAddress): XBinary(__pDevice,bIsImage,nImageAddress)
{

}

XMACH::~XMACH()
{

}

bool XMACH::isValid()
{
    bool bResult=false;
    unsigned int nMagic=read_uint32(0);

    if((nMagic==S_MH_MAGIC)||(nMagic==S_MH_CIGAM)||(nMagic==S_MH_MAGIC_64)||(nMagic==S_MH_CIGAM_64))
    {
        bResult=true;
    }

    return bResult;
}

bool XMACH::isBigEndian()
{
    bool bResult=false;
    quint32 nMagic=read_uint32(0);

    if((nMagic==S_MH_CIGAM)||(nMagic==S_MH_CIGAM_64))
    {
        bResult=true;
    }

    return bResult;
}

bool XMACH::is64()
{
    return (getHeader_magic()==S_MH_MAGIC_64);
}

quint32 XMACH::getHeader_magic()
{
    return read_uint32(offsetof(S_mach_header,magic),isBigEndian());
}

qint32 XMACH::getHeader_cputype()
{
    return read_int32(offsetof(S_mach_header,cputype),isBigEndian());
}

qint32 XMACH::getHeader_cpusubtype()
{
    return read_int32(offsetof(S_mach_header,cpusubtype),isBigEndian());
}

quint32 XMACH::getHeader_filetype()
{
    return read_uint32(offsetof(S_mach_header,filetype),isBigEndian());
}

quint32 XMACH::getHeader_ncmds()
{
    return read_uint32(offsetof(S_mach_header,ncmds),isBigEndian());
}

quint32 XMACH::getHeader_sizeofcmds()
{
    return read_uint32(offsetof(S_mach_header,sizeofcmds),isBigEndian());
}

quint32 XMACH::getHeader_flags()
{
    return read_uint32(offsetof(S_mach_header,flags),isBigEndian());
}

quint32 XMACH::getHeader_reserved()
{
    return read_uint32(offsetof(S_mach_header_64,reserved),isBigEndian());
}

void XMACH::setHeader_magic(quint32 nValue)
{
    write_uint32(offsetof(S_mach_header,magic),nValue,isBigEndian());
}

void XMACH::setHeader_cputype(qint32 nValue)
{
    write_int32(offsetof(S_mach_header,cputype),nValue,isBigEndian());
}

void XMACH::setHeader_cpusubtype(qint32 nValue)
{
    write_int32(offsetof(S_mach_header,cpusubtype),nValue,isBigEndian());
}

void XMACH::setHeader_filetype(quint32 nValue)
{
    write_uint32(offsetof(S_mach_header,filetype),nValue,isBigEndian());
}

void XMACH::setHeader_ncmds(quint32 nValue)
{
    write_uint32(offsetof(S_mach_header,ncmds),nValue,isBigEndian());
}

void XMACH::setHeader_sizeofcmds(quint32 nValue)
{
    write_uint32(offsetof(S_mach_header,sizeofcmds),nValue,isBigEndian());
}

void XMACH::setHeader_flags(quint32 nValue)
{
    write_uint32(offsetof(S_mach_header,flags),nValue,isBigEndian());
}

void XMACH::setHeader_reserved(quint32 nValue)
{
    write_uint32(offsetof(S_mach_header_64,reserved),nValue,isBigEndian());
}

QMap<quint64, QString> XMACH::getHeaderMagics()
{
    QMap<quint64, QString> mapResult;
    mapResult.insert(0xfeedface,"MH_MAGIC");
    mapResult.insert(0xcefaedfe,"MH_CIGAM");
    mapResult.insert(0xfeedfacf,"MH_MAGIC_64");
    mapResult.insert(0xcffaedfe,"MH_CIGAM_64");
    return mapResult;
}

QMap<quint64, QString> XMACH::getHeaderMagicsS()
{
    QMap<quint64, QString> mapResult;
    mapResult.insert(0xfeedface,"MAGIC");
    mapResult.insert(0xcefaedfe,"CIGAM");
    mapResult.insert(0xfeedfacf,"MAGIC_64");
    mapResult.insert(0xcffaedfe,"CIGAM_64");
    return mapResult;
}

QMap<quint64, QString> XMACH::getHeaderCpuTypes()
{
    QMap<quint64, QString> mapResult;
    mapResult.insert(7,"CPU_TYPE_I386");
    mapResult.insert(0x1000007,"CPU_TYPE_X86_64");
    mapResult.insert(12,"CPU_TYPE_ARM");
    mapResult.insert(14,"CPU_TYPE_SPARC");
    mapResult.insert(18,"CPU_TYPE_POWERPC");
    mapResult.insert(0x1000018,"CPU_TYPE_POWERPC64");
    return mapResult;
}

QMap<quint64, QString> XMACH::getHeaderCpuTypesS()
{
    QMap<quint64, QString> mapResult;
    mapResult.insert(7,"I386");
    mapResult.insert(0x1000007,"X86_64");
    mapResult.insert(12,"ARM");
    mapResult.insert(14,"SPARC");
    mapResult.insert(18,"POWERPC");
    mapResult.insert(0x1000018,"POWERPC64");
    return mapResult;
}

QMap<quint64, QString> XMACH::getHeaderFileTypes()
{
    QMap<quint64, QString> mapResult;
    mapResult.insert(0x1,"MH_OBJECT");
    mapResult.insert(0x2,"MH_EXECUTE");
    mapResult.insert(0x3,"MH_FVMLIB");
    mapResult.insert(0x4,"MH_CORE");
    mapResult.insert(0x5,"MH_PRELOAD");
    mapResult.insert(0x6,"MH_DYLIB");
    mapResult.insert(0x7,"MH_DYLINKER");
    mapResult.insert(0x8,"MH_BUNDLE");
    mapResult.insert(0x9,"MH_DYLIB_STUB");
    mapResult.insert(0xa,"MH_DSYM");
    mapResult.insert(0xb,"MH_KEXT_BUNDLE");
    return mapResult;
}

QMap<quint64, QString> XMACH::getHeaderFileTypesS()
{
    QMap<quint64, QString> mapResult;
    mapResult.insert(0x1,"OBJECT");
    mapResult.insert(0x2,"EXECUTE");
    mapResult.insert(0x3,"FVMLIB");
    mapResult.insert(0x4,"CORE");
    mapResult.insert(0x5,"PRELOAD");
    mapResult.insert(0x6,"DYLIB");
    mapResult.insert(0x7,"DYLINKER");
    mapResult.insert(0x8,"BUNDLE");
    mapResult.insert(0x9,"DYLIB_STUB");
    mapResult.insert(0xa,"DSYM");
    mapResult.insert(0xb,"KEXT_BUNDLE");
    return mapResult;
}
