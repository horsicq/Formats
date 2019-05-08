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

qint64 XMACH::getHeaderSize()
{
    qint64 nResult=0;

    if(is64())
    {
        nResult=sizeof(S_mach_header_64);
    }
    else
    {
        nResult=sizeof(S_mach_header);
    }

    return nResult;
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

QMap<quint64, QString> XMACH::getHeaderFlags()
{
    QMap<quint64, QString> mapResult;
    mapResult.insert(0x1,"MH_NOUNDEFS");
    mapResult.insert(0x2,"MH_INCRLINK");
    mapResult.insert(0x4,"MH_DYLDLINK");
    mapResult.insert(0x8,"MH_BINDATLOAD");
    mapResult.insert(0x10,"MH_PREBOUND");
    mapResult.insert(0x20,"MH_SPLIT_SEGS");
    mapResult.insert(0x40,"MH_LAZY_INIT");
    mapResult.insert(0x80,"MH_TWOLEVEL");
    mapResult.insert(0x100,"MH_FORCE_FLAT");
    mapResult.insert(0x200,"MH_NOMULTIDEFS");
    mapResult.insert(0x400,"MH_NOFIXPREBINDING");
    mapResult.insert(0x800,"MH_PREBINDABLE");
    mapResult.insert(0x1000,"MH_ALLMODSBOUND");
    mapResult.insert(0x2000,"MH_SUBSECTIONS_VIA_SYMBOLS");
    mapResult.insert(0x4000,"MH_CANONICAL");
    mapResult.insert(0x8000,"MH_WEAK_DEFINES");
    mapResult.insert(0x10000,"MH_BINDS_TO_WEAK");
    mapResult.insert(0x20000,"MH_ALLOW_STACK_EXECUTION");
    mapResult.insert(0x40000,"MH_ROOT_SAFE");
    mapResult.insert(0x80000,"MH_SETUID_SAFE");
    mapResult.insert(0x100000,"MH_NO_REEXPORTED_DYLIBS");
    mapResult.insert(0x200000,"MH_PIE");
    mapResult.insert(0x400000,"MH_DEAD_STRIPPABLE_DYLIB");
    mapResult.insert(0x800000,"MH_HAS_TLV_DESCRIPTORS");
    mapResult.insert(0x1000000,"MH_NO_HEAP_EXECUTION");
    mapResult.insert(0x02000000,"MH_APP_EXTENSION_SAFE");
    mapResult.insert(0x04000000,"MH_NLIST_OUTOFSYNC_WITH_DYLDINFO");
    mapResult.insert(0x08000000,"MH_SIM_SUPPORT");
    return mapResult;
}

QMap<quint64, QString> XMACH::getHeaderFlagsS()
{
    QMap<quint64, QString> mapResult;
    mapResult.insert(0x1,"NOUNDEFS");
    mapResult.insert(0x2,"INCRLINK");
    mapResult.insert(0x4,"DYLDLINK");
    mapResult.insert(0x8,"BINDATLOAD");
    mapResult.insert(0x10,"PREBOUND");
    mapResult.insert(0x20,"SPLIT_SEGS");
    mapResult.insert(0x40,"LAZY_INIT");
    mapResult.insert(0x80,"TWOLEVEL");
    mapResult.insert(0x100,"FORCE_FLAT");
    mapResult.insert(0x200,"NOMULTIDEFS");
    mapResult.insert(0x400,"NOFIXPREBINDING");
    mapResult.insert(0x800,"PREBINDABLE");
    mapResult.insert(0x1000,"ALLMODSBOUND");
    mapResult.insert(0x2000,"SUBSECTIONS_VIA_SYMBOLS");
    mapResult.insert(0x4000,"CANONICAL");
    mapResult.insert(0x8000,"WEAK_DEFINES");
    mapResult.insert(0x10000,"BINDS_TO_WEAK");
    mapResult.insert(0x20000,"ALLOW_STACK_EXECUTION");
    mapResult.insert(0x40000,"ROOT_SAFE");
    mapResult.insert(0x80000,"SETUID_SAFE");
    mapResult.insert(0x100000,"NO_REEXPORTED_DYLIBS");
    mapResult.insert(0x200000,"PIE");
    mapResult.insert(0x400000,"DEAD_STRIPPABLE_DYLIB");
    mapResult.insert(0x800000,"HAS_TLV_DESCRIPTORS");
    mapResult.insert(0x1000000,"NO_HEAP_EXECUTION");
    mapResult.insert(0x02000000,"APP_EXTENSION_SAFE");
    mapResult.insert(0x04000000,"NLIST_OUTOFSYNC_WITH_DYLDINFO");
    mapResult.insert(0x08000000,"SIM_SUPPORT");
    return mapResult;
}

QMap<quint64, QString> XMACH::getLoadCommandTypes()
{
    QMap<quint64, QString> mapResult;
    mapResult.insert(0x1,"LC_SEGMENT");
    mapResult.insert(0x2,"LC_SYMTAB");
    mapResult.insert(0x3,"LC_SYMSEG");
    mapResult.insert(0x4,"LC_THREAD");
    mapResult.insert(0x5,"LC_UNIXTHREAD");
    mapResult.insert(0x6,"LC_LOADFVMLIB");
    mapResult.insert(0x7,"LC_IDFVMLIB");
    mapResult.insert(0x8,"LC_IDENT");
    mapResult.insert(0x9,"LC_FVMFILE");
    mapResult.insert(0xa,"LC_PREPAGE");
    mapResult.insert(0xb,"LC_DYSYMTAB");
    mapResult.insert(0xc,"LC_LOAD_DYLIB");
    mapResult.insert(0xd,"LC_ID_DYLIB");
    mapResult.insert(0xe,"LC_LOAD_DYLINKER");
    mapResult.insert(0xf,"LC_ID_DYLINKER");
    mapResult.insert(0x10,"LC_PREBOUND_DYLIB");
    mapResult.insert(0x11,"LC_ROUTINES");
    mapResult.insert(0x12,"LC_SUB_FRAMEWORK");
    mapResult.insert(0x13,"LC_SUB_UMBRELLA");
    mapResult.insert(0x14,"LC_SUB_CLIENT");
    mapResult.insert(0x15,"LC_SUB_LIBRARY");
    mapResult.insert(0x16,"LC_TWOLEVEL_HINTS");
    mapResult.insert(0x17,"LC_PREBIND_CKSUM");
    mapResult.insert(0x18|0x80000000,"LC_LOAD_WEAK_DYLIB");
    mapResult.insert(0x19,"LC_SEGMENT_64");
    mapResult.insert(0x1a,"LC_ROUTINES_64");
    mapResult.insert(0x1b,"LC_UUID");
    mapResult.insert(0x1c|0x80000000,"LC_RPATH");
    mapResult.insert(0x1d,"LC_CODE_SIGNATURE");
    mapResult.insert(0x1e,"LC_SEGMENT_SPLIT_INFO");
    mapResult.insert(0x1f|0x80000000,"LC_REEXPORT_DYLIB");
    mapResult.insert(0x20,"LC_LAZY_LOAD_DYLIB");
    mapResult.insert(0x21,"LC_ENCRYPTION_INFO");
    mapResult.insert(0x22,"LC_DYLD_INFO");
    mapResult.insert(0x22|0x80000000,"LC_DYLD_INFO_ONLY");
    mapResult.insert(0x23|0x80000000,"LC_LOAD_UPWARD_DYLIB");
    mapResult.insert(0x24,"LC_VERSION_MIN_MACOSX");
    mapResult.insert(0x25,"LC_VERSION_MIN_IPHONEOS");
    mapResult.insert(0x26,"LC_FUNCTION_STARTS");
    mapResult.insert(0x24,"LC_VERSION_MIN_MACOSX");
    mapResult.insert(0x25,"LC_VERSION_MIN_IPHONEOS");
    mapResult.insert(0x26,"LC_FUNCTION_STARTS");
    mapResult.insert(0x27,"LC_DYLD_ENVIRONMENT");
    mapResult.insert(0x28|0x80000000,"LC_MAIN");
    mapResult.insert(0x29,"LC_DATA_IN_CODE");
    mapResult.insert(0x2A,"LC_SOURCE_VERSION");
    mapResult.insert(0x2B,"LC_DYLIB_CODE_SIGN_DRS");
    mapResult.insert(0x2C,"LC_ENCRYPTION_INFO_64");
    mapResult.insert(0x2D,"LC_LINKER_OPTION");
    mapResult.insert(0x2E,"LC_LINKER_OPTIMIZATION_HINT");
    mapResult.insert(0x2F,"LC_VERSION_MIN_TVOS");
    mapResult.insert(0x30,"LC_VERSION_MIN_WATCHOS");
    return mapResult;
}

QMap<quint64, QString> XMACH::getLoadCommandTypesS()
{
    QMap<quint64, QString> mapResult;
    mapResult.insert(0x1,"SEGMENT");
    mapResult.insert(0x2,"SYMTAB");
    mapResult.insert(0x3,"SYMSEG");
    mapResult.insert(0x4,"THREAD");
    mapResult.insert(0x5,"UNIXTHREAD");
    mapResult.insert(0x6,"LOADFVMLIB");
    mapResult.insert(0x7,"IDFVMLIB");
    mapResult.insert(0x8,"IDENT");
    mapResult.insert(0x9,"FVMFILE");
    mapResult.insert(0xa,"PREPAGE");
    mapResult.insert(0xb,"DYSYMTAB");
    mapResult.insert(0xc,"LOAD_DYLIB");
    mapResult.insert(0xd,"ID_DYLIB");
    mapResult.insert(0xe,"LOAD_DYLINKER");
    mapResult.insert(0xf,"ID_DYLINKER");
    mapResult.insert(0x10,"PREBOUND_DYLIB");
    mapResult.insert(0x11,"ROUTINES");
    mapResult.insert(0x12,"SUB_FRAMEWORK");
    mapResult.insert(0x13,"SUB_UMBRELLA");
    mapResult.insert(0x14,"SUB_CLIENT");
    mapResult.insert(0x15,"SUB_LIBRARY");
    mapResult.insert(0x16,"TWOLEVEL_HINTS");
    mapResult.insert(0x17,"PREBIND_CKSUM");
    mapResult.insert(0x18|0x80000000,"LOAD_WEAK_DYLIB");
    mapResult.insert(0x19,"SEGMENT_64");
    mapResult.insert(0x1a,"ROUTINES_64");
    mapResult.insert(0x1b,"UUID");
    mapResult.insert(0x1c|0x80000000,"RPATH");
    mapResult.insert(0x1d,"CODE_SIGNATURE");
    mapResult.insert(0x1e,"SEGMENT_SPLIT_INFO");
    mapResult.insert(0x1f|0x80000000,"REEXPORT_DYLIB");
    mapResult.insert(0x20,"LAZY_LOAD_DYLIB");
    mapResult.insert(0x21,"ENCRYPTION_INFO");
    mapResult.insert(0x22,"DYLD_INFO");
    mapResult.insert(0x22|0x80000000,"DYLD_INFO_ONLY");
    mapResult.insert(0x23|0x80000000,"LOAD_UPWARD_DYLIB");
    mapResult.insert(0x24,"VERSION_MIN_MACOSX");
    mapResult.insert(0x25,"VERSION_MIN_IPHONEOS");
    mapResult.insert(0x26,"FUNCTION_STARTS");
    mapResult.insert(0x24,"VERSION_MIN_MACOSX");
    mapResult.insert(0x25,"VERSION_MIN_IPHONEOS");
    mapResult.insert(0x26,"FUNCTION_STARTS");
    mapResult.insert(0x27,"DYLD_ENVIRONMENT");
    mapResult.insert(0x28|0x80000000,"MAIN");
    mapResult.insert(0x29,"DATA_IN_CODE");
    mapResult.insert(0x2A,"SOURCE_VERSION");
    mapResult.insert(0x2B,"DYLIB_CODE_SIGN_DRS");
    mapResult.insert(0x2C,"ENCRYPTION_INFO_64");
    mapResult.insert(0x2D,"LINKER_OPTION");
    mapResult.insert(0x2E,"LINKER_OPTIMIZATION_HINT");
    mapResult.insert(0x2F,"VERSION_MIN_TVOS");
    mapResult.insert(0x30,"VERSION_MIN_WATCHOS");
    return mapResult;
}

QList<XMACH::COMMAND_RECORD> XMACH::getCommandRecords()
{
    QList<COMMAND_RECORD> listResult;

    quint32 nNumberOfCommands=getHeader_ncmds();
    quint32 nSizeOfCommands=getHeader_sizeofcmds();

    qint64 nOffset=getHeaderSize();
    bool bIsBigEndian=isBigEndian();
    bool bIs64=is64();

    qint64 nSize=0;

    for(quint32 i=0;i<nNumberOfCommands;i++)
    {
        COMMAND_RECORD record={};

        record.nOffset=nOffset;
        record.nType=read_uint32(nOffset+offsetof(S_load_command,cmd),bIsBigEndian);
        record.nSize=read_uint32(nOffset+offsetof(S_load_command,cmdsize),bIsBigEndian);

        listResult.append(record);

        qint64 _nSize=record.nSize;

        if(bIs64)
        {
            _nSize=__ALIGN_UP(_nSize,8);
        }
        else
        {
            _nSize=__ALIGN_UP(_nSize,4);
        }
        nSize+=_nSize;
        nOffset+=_nSize;

        if(nSize>nSizeOfCommands)
        {
            break;
        }
    }

    return listResult;
}

QList<XMACH::COMMAND_RECORD> XMACH::getCommandRecords(QList<XMACH::COMMAND_RECORD> *pList, quint32 nCommandID)
{
    QList<COMMAND_RECORD> listResult;

    int nCount=pList->count();

    for(int i=0;i<nCount;i++)
    {
        if(pList->at(i).nType==nCommandID)
        {
            listResult.append(pList->at(i));
        }
    }

    return listResult;
}

bool XMACH::isCommandPresent(quint32 nCommandID, int nIndex)
{
    QList<COMMAND_RECORD> list=getCommandRecords();

    return isCommandPresent(&list,nCommandID,nIndex);
}

bool XMACH::isCommandPresent(QList<XMACH::COMMAND_RECORD> *pList, quint32 nCommandID, int nIndex)
{
    bool bResult=false;

    int nCount=pList->count();

    int nCurrentIndex=0;
    for(int i=0;i<nCount;i++)
    {
        if(pList->at(i).nType==nCommandID)
        {
            if(nCurrentIndex==nIndex)
            {
                bResult=true;
                break;
            }
            nCurrentIndex++;
        }
    }

    return bResult;
}

QByteArray XMACH::getCommand(quint32 nCommandID, int nIndex)
{
    QList<COMMAND_RECORD> list=getCommandRecords();

    return getCommand(&list,nCommandID,nIndex);
}

bool XMACH::setCommand(quint32 nCommandID, QByteArray baData, int nIndex)
{
    QList<COMMAND_RECORD> list=getCommandRecords();

    return setCommand(&list,nCommandID,baData,nIndex);
}

QByteArray XMACH::getCommand(QList<XMACH::COMMAND_RECORD> *pList, quint32 nCommandID, int nIndex)
{
    QByteArray baResult;

    int nCount=pList->count();

    int nCurrentIndex=0;
    for(int i=0;i<nCount;i++)
    {
        if(pList->at(i).nType==nCommandID)
        {
            if(nCurrentIndex==nIndex)
            {
                baResult=read_array(pList->at(i).nOffset,pList->at(i).nSize);
                break;
            }
            nCurrentIndex++;
        }
    }

    return baResult;
}

bool XMACH::setCommand(QList<XMACH::COMMAND_RECORD> *pList, quint32 nCommandID, QByteArray baData, int nIndex)
{
    bool bResult=false;

    int nCount=pList->count();

    int nCurrentIndex=0;
    for(int i=0;i<nCount;i++)
    {
        if(pList->at(i).nType==nCommandID)
        {
            qint32 nSize=baData.size();

            if(nCurrentIndex==nIndex)
            {
                if(nSize==pList->at(i).nSize)
                {
                    bResult=(write_array(pList->at(i).nOffset,baData.data(),pList->at(i).nSize)==nSize);
                }
                break;
            }
            nCurrentIndex++;
        }
    }

    return bResult;
}
