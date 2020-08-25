// copyright (c) 2017-2020 hors<horsicq@gmail.com>
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

XMACH::XMACH(QIODevice *pDevice, bool bIsImage, qint64 nImageAddress): XBinary(pDevice,bIsImage,nImageAddress)
{

}

XMACH::~XMACH()
{

}

bool XMACH::isValid()
{
    bool bResult=false;

    quint32 nMagic=read_uint32(0);

    if((nMagic==XMACH_DEF::S_MH_MAGIC)||
            (nMagic==XMACH_DEF::S_MH_CIGAM)||
            (nMagic==XMACH_DEF::S_MH_MAGIC_64)||
            (nMagic==XMACH_DEF::S_MH_CIGAM_64))
    {
        bResult=true;
    }

    return bResult;
}

bool XMACH::isBigEndian()
{
    bool bResult=false;

    quint32 nMagic=read_uint32(0);

    if((nMagic==XMACH_DEF::S_MH_CIGAM)||
            (nMagic==XMACH_DEF::S_MH_CIGAM_64))
    {
        bResult=true;
    }

    return bResult;
}

qint64 XMACH::getHeaderOffset()
{
    return 0;
}

qint64 XMACH::getHeader32Size()
{
    return sizeof(XMACH_DEF::mach_header);
}

qint64 XMACH::getHeader64Size()
{
    return sizeof(XMACH_DEF::mach_header_64);
}

quint32 XMACH::getHeader_magic()
{
    return read_uint32(offsetof(XMACH_DEF::mach_header,magic),isBigEndian());
}

qint32 XMACH::getHeader_cputype()
{
    return read_int32(offsetof(XMACH_DEF::mach_header,cputype),isBigEndian());
}

qint32 XMACH::getHeader_cpusubtype()
{
    return read_int32(offsetof(XMACH_DEF::mach_header,cpusubtype),isBigEndian());
}

quint32 XMACH::getHeader_filetype()
{
    return read_uint32(offsetof(XMACH_DEF::mach_header,filetype),isBigEndian());
}

quint32 XMACH::getHeader_ncmds()
{
    return read_uint32(offsetof(XMACH_DEF::mach_header,ncmds),isBigEndian());
}

quint32 XMACH::getHeader_sizeofcmds()
{
    return read_uint32(offsetof(XMACH_DEF::mach_header,sizeofcmds),isBigEndian());
}

quint32 XMACH::getHeader_flags()
{
    return read_uint32(offsetof(XMACH_DEF::mach_header,flags),isBigEndian());
}

quint32 XMACH::getHeader_reserved()
{
    return read_uint32(offsetof(XMACH_DEF::mach_header_64,reserved),isBigEndian());
}

void XMACH::setHeader_magic(quint32 nValue)
{
    write_uint32(offsetof(XMACH_DEF::mach_header,magic),nValue,isBigEndian());
}

void XMACH::setHeader_cputype(qint32 nValue)
{
    write_int32(offsetof(XMACH_DEF::mach_header,cputype),nValue,isBigEndian());
}

void XMACH::setHeader_cpusubtype(qint32 nValue)
{
    write_int32(offsetof(XMACH_DEF::mach_header,cpusubtype),nValue,isBigEndian());
}

void XMACH::setHeader_filetype(quint32 nValue)
{
    write_uint32(offsetof(XMACH_DEF::mach_header,filetype),nValue,isBigEndian());
}

void XMACH::setHeader_ncmds(quint32 nValue)
{
    write_uint32(offsetof(XMACH_DEF::mach_header,ncmds),nValue,isBigEndian());
}

void XMACH::setHeader_sizeofcmds(quint32 nValue)
{
    write_uint32(offsetof(XMACH_DEF::mach_header,sizeofcmds),nValue,isBigEndian());
}

void XMACH::setHeader_flags(quint32 nValue)
{
    write_uint32(offsetof(XMACH_DEF::mach_header,flags),nValue,isBigEndian());
}

void XMACH::setHeader_reserved(quint32 nValue)
{
    write_uint32(offsetof(XMACH_DEF::mach_header_64,reserved),nValue,isBigEndian());
}

qint64 XMACH::getHeaderSize()
{
    qint64 nResult=0;

    if(is64())
    {
        nResult=sizeof(XMACH_DEF::mach_header_64);
    }
    else
    {
        nResult=sizeof(XMACH_DEF::mach_header);
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
    // https://opensource.apple.com/source/cctools/cctools-836/include/mach/machine.h
    mapResult.insert(1,"CPU_TYPE_VAX");
    mapResult.insert(2,"CPU_TYPE_ROMP");
    mapResult.insert(4,"CPU_TYPE_NS32032");
    mapResult.insert(5,"CPU_TYPE_NS32332");
    mapResult.insert(6,"CPU_TYPE_MC680x0");
    mapResult.insert(7,"CPU_TYPE_I386");
    mapResult.insert(0x1000007,"CPU_TYPE_X86_64");
    mapResult.insert(8,"CPU_TYPE_MIPS");
    mapResult.insert(9,"CPU_TYPE_NS32532");
    mapResult.insert(11,"CPU_TYPE_HPPA");
    mapResult.insert(12,"CPU_TYPE_ARM");
    mapResult.insert(13,"CPU_TYPE_MC88000");
    mapResult.insert(14,"CPU_TYPE_SPARC");
    mapResult.insert(15,"CPU_TYPE_I860");
    mapResult.insert(16,"CPU_TYPE_I860_LITTLE");
    mapResult.insert(17,"CPU_TYPE_RS6000");
    mapResult.insert(18,"CPU_TYPE_POWERPC");
    mapResult.insert(0x1000018,"CPU_TYPE_POWERPC64");
    mapResult.insert(255,"CPU_TYPE_VEO");

    return mapResult;
}

QMap<quint64, QString> XMACH::getHeaderCpuTypesS()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(1,"VAX");
    mapResult.insert(2,"ROMP");
    mapResult.insert(4,"NS32032");
    mapResult.insert(5,"NS32332");
    mapResult.insert(6,"MC680x0");
    mapResult.insert(7,"I386");
    mapResult.insert(0x1000007,"X86_64");
    mapResult.insert(8,"MIPS");
    mapResult.insert(9,"NS32532");
    mapResult.insert(11,"HPPA");
    mapResult.insert(12,"ARM");
    mapResult.insert(13,"MC88000");
    mapResult.insert(14,"SPARC");
    mapResult.insert(15,"I860");
    mapResult.insert(16,"I860_LITTLE");
    mapResult.insert(17,"RS6000");
    mapResult.insert(18,"POWERPC");
    mapResult.insert(0x1000018,"POWERPC64");
    mapResult.insert(255,"VEO");

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

    for(quint32 i=0; i<nNumberOfCommands; i++)
    {
        COMMAND_RECORD record= {};

        record.nOffset=nOffset;
        record.nType=read_uint32(nOffset+offsetof(XMACH_DEF::load_command,cmd),bIsBigEndian);
        record.nSize=read_uint32(nOffset+offsetof(XMACH_DEF::load_command,cmdsize),bIsBigEndian);

        listResult.append(record);

        qint64 _nSize=record.nSize;

        if(bIs64)
        {
            _nSize=S_ALIGN_UP(_nSize,8);
        }
        else
        {
            _nSize=S_ALIGN_UP(_nSize,4);
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

QList<XMACH::COMMAND_RECORD> XMACH::getCommandRecords(quint32 nCommandID, QList<XMACH::COMMAND_RECORD> *pList)
{
    QList<COMMAND_RECORD> listResult;

    int nCount=pList->count();

    for(int i=0; i<nCount; i++)
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

    return isCommandPresent(nCommandID,nIndex,&list);
}

bool XMACH::isCommandPresent(quint32 nCommandID, int nIndex, QList<XMACH::COMMAND_RECORD> *pList)
{
    bool bResult=false;

    int nCount=pList->count();

    int nCurrentIndex=0;

    for(int i=0; i<nCount; i++)
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

    return getCommand(nCommandID,nIndex,&list);
}

bool XMACH::setCommand(quint32 nCommandID, QByteArray baData, int nIndex)
{
    QList<COMMAND_RECORD> list=getCommandRecords();

    return setCommand(nCommandID,baData,nIndex,&list);
}

QByteArray XMACH::getCommand(quint32 nCommandID, int nIndex, QList<XMACH::COMMAND_RECORD> *pList)
{
    QByteArray baResult;

    int nCount=pList->count();

    int nCurrentIndex=0;

    for(int i=0; i<nCount; i++)
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

bool XMACH::setCommand(quint32 nCommandID, QByteArray baData, int nIndex, QList<XMACH::COMMAND_RECORD> *pList)
{
    bool bResult=false;

    int nCount=pList->count();

    int nCurrentIndex=0;

    for(int i=0; i<nCount; i++)
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

qint64 XMACH::getAddressOfEntryPoint()
{
    qint64 nResult=-1;

    bool bIsBigEndian=isBigEndian();

    QList<COMMAND_RECORD> listLC=getCommandRecords();

    int nCount=listLC.count();

    for(int i=0; i<nCount; i++)
    {
        quint32 nType=listLC.at(i).nType;
        qint64 nOffset=listLC.at(i).nOffset;

        if((nType==XMACH_DEF::S_LC_THREAD)||(nType==XMACH_DEF::S_LC_UNIXTHREAD))
        {
            quint32 nFlavor=read_uint32(nOffset+8,bIsBigEndian);

            if(nFlavor==XMACH_DEF::S_x86_THREAD_STATE32)
            {
                nResult=read_uint32(nOffset+16+offsetof(XMACH_DEF::STRUCT_X86_THREAD_STATE32,eip),bIsBigEndian);
            }
            else if(nFlavor==XMACH_DEF::S_x86_THREAD_STATE64)
            {
                nResult=read_uint64(nOffset+16+offsetof(XMACH_DEF::STRUCT_X86_THREAD_STATE64,rip),bIsBigEndian);
            }
        }
        else if(nType==XMACH_DEF::S_LC_MAIN)
        {
            qint64 nEntryPointOffset=read_uint64(nOffset+offsetof(XMACH_DEF::entry_point_command,entryoff),bIsBigEndian);

            nResult=offsetToAddress(nEntryPointOffset);
        }
    }

    return nResult;
}

XBinary::_MEMORY_MAP XMACH::getMemoryMap()
{
    _MEMORY_MAP result= {};

    qint32 nIndex=0;

    result.mode=getMode();

    if(result.mode==MODE_64)
    {
        result.fileType=FT_MACH64;
    }
    else
    {
        result.fileType=FT_MACH32;
    }

    result.sArch=getArch();
    result.bIsBigEndian=isBigEndian();
    result.sType=getTypeAsString();

    result.nRawSize=getSize();

    QList<COMMAND_RECORD> listLC=getCommandRecords();

    QList<SEGMENT_RECORD> listSegmentRecords=getSegmentRecords(&listLC);

    int nCount=listSegmentRecords.count();

    bool bImageAddressInit=false;

    qint64 nMaxOffset=0;
    qint64 nMaxAddress=0;

    for(int i=0; i<nCount; i++)
    {
        QString sSegmentName=QString("%1(%2)['%3']").arg(tr("Segment")).arg(i).arg(listSegmentRecords.at(i).segname); // TODO Limit
        // TODO Align
        // TODO File size
        qint64 nFileOffset=listSegmentRecords.at(i).fileoff;
        qint64 nVirtualAddress=listSegmentRecords.at(i).vmaddr;
        qint64 nFileSize=listSegmentRecords.at(i).filesize;
        qint64 nVirtualSize=listSegmentRecords.at(i).vmsize;

        if(nFileSize)
        {
            XBinary::_MEMORY_RECORD record= {};

            record.type=MMT_LOADSECTION;

            record.sName=sSegmentName;
            record.nAddress=nVirtualAddress;
            record.nSize=nFileSize;
            record.nOffset=nFileOffset;
            record.nIndex=nIndex++;

            result.listRecords.append(record);
        }

        if(nVirtualSize>nFileSize)
        {
            XBinary::_MEMORY_RECORD record= {};

            record.type=MMT_LOADSECTION;
            record.bIsVirtual=true;

            record.sName=sSegmentName;
            record.nAddress=nVirtualAddress+nFileSize;
            record.nSize=nVirtualSize-nFileSize;
            record.nOffset=-1;
            record.nIndex=nIndex++;

            result.listRecords.append(record);
        }

        if(!bImageAddressInit)
        {
            result.nBaseAddress=nVirtualAddress;
            bImageAddressInit=true;
        }

        nMaxOffset=qMax(nMaxOffset,nFileOffset+nFileSize);

        result.nBaseAddress=qMin(nVirtualAddress,result.nBaseAddress);
        nMaxAddress=qMax(nVirtualAddress+nVirtualSize,nMaxAddress);
    }

    result.nImageSize=nMaxAddress-result.nBaseAddress;

    return result;
}

qint64 XMACH::getEntryPointOffset(_MEMORY_MAP *pMemoryMap)
{
    qint64 nResult=-1;

    qint64 nAddress=getAddressOfEntryPoint();

    if(nAddress!=-1)
    {
        nResult=addressToOffset(pMemoryMap,nAddress);
    }

    return nResult;
}

QList<XMACH::LIBRARY_RECORD> XMACH::getLibraryRecords()
{
    QList<COMMAND_RECORD> listLC=getCommandRecords();

    return getLibraryRecords(&listLC);
}

QList<XMACH::LIBRARY_RECORD> XMACH::getLibraryRecords(QList<XMACH::COMMAND_RECORD> *pList)
{
    QList<LIBRARY_RECORD> listResult;

    bool bIsBigEndian=isBigEndian();

    QList<COMMAND_RECORD> listLCLibraries=getCommandRecords(XMACH_DEF::S_LC_LOAD_DYLIB,pList);

    int nCount=listLCLibraries.count();

    for(int i=0; i<nCount; i++)
    {
        LIBRARY_RECORD record= {};

        qint64 nOffset=listLCLibraries.at(i).nOffset;

        qint64 nNameOffset=read_uint32(nOffset+8+offsetof(XMACH_DEF::dylib,name),bIsBigEndian);
        record.sFullName=read_ansiString(nOffset+nNameOffset);
        record.sName=record.sFullName.section("/",-1,-1);
        record.timestamp=read_uint32(nOffset+8+offsetof(XMACH_DEF::dylib,timestamp),bIsBigEndian);
        record.current_version=read_uint32(nOffset+8+offsetof(XMACH_DEF::dylib,current_version),bIsBigEndian);
        record.compatibility_version=read_uint32(nOffset+8+offsetof(XMACH_DEF::dylib,compatibility_version),bIsBigEndian);

        listResult.append(record);
    }

    return listResult;
}

XMACH::LIBRARY_RECORD XMACH::getLibraryRecordByName(QString sName, QList<XMACH::LIBRARY_RECORD> *pList)
{
    LIBRARY_RECORD result= {};

    int nCount=pList->count();

    for(int i=0; i<nCount; i++)
    {
        if(pList->at(i).sName==sName)
        {
            result=pList->at(i);

            break;
        }
    }

    return result;
}

bool XMACH::isLibraryRecordNamePresent(QString sName)
{
    QList<XMACH::LIBRARY_RECORD> listLibraries=getLibraryRecords();

    return isLibraryRecordNamePresent(sName,&listLibraries);
}

bool XMACH::isLibraryRecordNamePresent(QString sName, QList<XMACH::LIBRARY_RECORD> *pList)
{
    bool bResult=false;

    int nCount=pList->count();

    for(int i=0; i<nCount; i++)
    {
        if(pList->at(i).sName==sName)
        {
            bResult=true;

            break;
        }
    }

    return bResult;
}

QList<XMACH::SEGMENT_RECORD> XMACH::getSegmentRecords()
{
    QList<XMACH::COMMAND_RECORD> listCR=getCommandRecords();

    return getSegmentRecords(&listCR);
}

QList<XMACH::SEGMENT_RECORD> XMACH::getSegmentRecords(QList<XMACH::COMMAND_RECORD> *pList)
{
    QList<SEGMENT_RECORD> listResult;

    bool bIs64=is64();
    bool bIsBigEndian=isBigEndian();

    if(bIs64)
    {
        QList<COMMAND_RECORD> listLCSegments=getCommandRecords(XMACH_DEF::S_LC_SEGMENT_64,pList);

        int nCount=listLCSegments.count();

        for(int i=0; i<nCount; i++)
        {
            qint64 nOffset=listLCSegments.at(i).nOffset;

            SEGMENT_RECORD record= {};

            record.nStructOffset=nOffset+sizeof(XMACH_DEF::load_command);
            read_array(nOffset+offsetof(XMACH_DEF::segment_command_64,segname),record.segname,sizeof(record.segname));
            record.vmaddr=read_uint64(nOffset+offsetof(XMACH_DEF::segment_command_64,vmaddr),bIsBigEndian);
            record.vmsize=read_uint64(nOffset+offsetof(XMACH_DEF::segment_command_64,vmsize),bIsBigEndian);
            record.fileoff=read_uint64(nOffset+offsetof(XMACH_DEF::segment_command_64,fileoff),bIsBigEndian);
            record.filesize=read_uint64(nOffset+offsetof(XMACH_DEF::segment_command_64,filesize),bIsBigEndian);
            record.maxprot=read_int32(nOffset+offsetof(XMACH_DEF::segment_command_64,maxprot),bIsBigEndian);
            record.initprot=read_int32(nOffset+offsetof(XMACH_DEF::segment_command_64,initprot),bIsBigEndian);
            record.nsects=read_uint32(nOffset+offsetof(XMACH_DEF::segment_command_64,nsects),bIsBigEndian);
            record.flags=read_uint32(nOffset+offsetof(XMACH_DEF::segment_command_64,flags),bIsBigEndian);

            listResult.append(record);
        }
    }
    else
    {
        QList<COMMAND_RECORD> listLCSegments=getCommandRecords(XMACH_DEF::S_LC_SEGMENT,pList);

        int nCount=listLCSegments.count();

        for(int i=0; i<nCount; i++)
        {
            qint64 nOffset=listLCSegments.at(i).nOffset;

            SEGMENT_RECORD record= {};

            record.nStructOffset=nOffset+sizeof(XMACH_DEF::load_command);
            read_array(nOffset+offsetof(XMACH_DEF::segment_command,segname),record.segname,sizeof(record.segname));
            record.vmaddr=read_uint32(nOffset+offsetof(XMACH_DEF::segment_command,vmaddr),bIsBigEndian);
            record.vmsize=read_uint32(nOffset+offsetof(XMACH_DEF::segment_command,vmsize),bIsBigEndian);
            record.fileoff=read_uint32(nOffset+offsetof(XMACH_DEF::segment_command,fileoff),bIsBigEndian);
            record.filesize=read_uint32(nOffset+offsetof(XMACH_DEF::segment_command,filesize),bIsBigEndian);
            record.maxprot=read_int32(nOffset+offsetof(XMACH_DEF::segment_command,maxprot),bIsBigEndian);
            record.initprot=read_int32(nOffset+offsetof(XMACH_DEF::segment_command,initprot),bIsBigEndian);
            record.nsects=read_uint32(nOffset+offsetof(XMACH_DEF::segment_command,nsects),bIsBigEndian);
            record.flags=read_uint32(nOffset+offsetof(XMACH_DEF::segment_command,flags),bIsBigEndian);

            listResult.append(record);
        }
    }

    return listResult;
}

QList<XMACH::SECTION_RECORD> XMACH::getSectionRecords()
{
    QList<XMACH::COMMAND_RECORD> listCR=getCommandRecords();

    return getSectionRecords(&listCR);
}

QList<XMACH::SECTION_RECORD> XMACH::getSectionRecords(QList<XMACH::COMMAND_RECORD> *pList)
{
    QList<SECTION_RECORD> listResult;

    bool bIs64=is64();
    bool bIsBigEndian=isBigEndian();

    if(bIs64)
    {
        QList<COMMAND_RECORD> listLCSegments=getCommandRecords(XMACH_DEF::S_LC_SEGMENT_64,pList);

        int nCount=listLCSegments.count();

        for(int i=0; i<nCount; i++)
        {
            qint64 nOffset=listLCSegments.at(i).nOffset;
            int nNumberOfSections=read_uint32(nOffset+offsetof(XMACH_DEF::segment_command_64,nsects),bIsBigEndian);

            nOffset+=sizeof(XMACH_DEF::segment_command_64);

            for(int j=0; j<nNumberOfSections; j++)
            {
                SECTION_RECORD record= {};

                record.nStructOffset=nOffset;

                read_array(nOffset+offsetof(XMACH_DEF::section_64,sectname),record.sectname,16);
                read_array(nOffset+offsetof(XMACH_DEF::section_64,segname),record.segname,16);
                record.addr=read_uint64(nOffset+offsetof(XMACH_DEF::section_64,addr),bIsBigEndian);
                record.size=read_uint64(nOffset+offsetof(XMACH_DEF::section_64,size),bIsBigEndian);
                record.offset=read_uint32(nOffset+offsetof(XMACH_DEF::section_64,offset),bIsBigEndian);
                record.align=read_uint32(nOffset+offsetof(XMACH_DEF::section_64,align),bIsBigEndian);
                record.reloff=read_uint32(nOffset+offsetof(XMACH_DEF::section_64,reloff),bIsBigEndian);
                record.nreloc=read_uint32(nOffset+offsetof(XMACH_DEF::section_64,nreloc),bIsBigEndian);
                record.flags=read_uint32(nOffset+offsetof(XMACH_DEF::section_64,flags),bIsBigEndian);

                listResult.append(record);

                nOffset+=sizeof(XMACH_DEF::section_64);
            }
        }
    }
    else
    {
        QList<COMMAND_RECORD> listLCSegments=getCommandRecords(XMACH_DEF::S_LC_SEGMENT,pList);

        int nCount=listLCSegments.count();

        for(int i=0; i<nCount; i++)
        {
            qint64 nOffset=listLCSegments.at(i).nOffset;
            int nNumberOfSections=read_uint32(nOffset+offsetof(XMACH_DEF::segment_command,nsects),bIsBigEndian);

            nOffset+=sizeof(XMACH_DEF::segment_command);

            for(int j=0; j<nNumberOfSections; j++)
            {
                SECTION_RECORD record= {};

                record.nStructOffset=nOffset;

                read_array(nOffset+offsetof(XMACH_DEF::section,sectname),record.sectname,16);
                read_array(nOffset+offsetof(XMACH_DEF::section,segname),record.segname,16);
                record.addr=read_uint32(nOffset+offsetof(XMACH_DEF::section,addr),bIsBigEndian);
                record.size=read_uint32(nOffset+offsetof(XMACH_DEF::section,size),bIsBigEndian);
                record.offset=read_uint32(nOffset+offsetof(XMACH_DEF::section,offset),bIsBigEndian);
                record.align=read_uint32(nOffset+offsetof(XMACH_DEF::section,align),bIsBigEndian);
                record.reloff=read_uint32(nOffset+offsetof(XMACH_DEF::section,reloff),bIsBigEndian);
                record.nreloc=read_uint32(nOffset+offsetof(XMACH_DEF::section,nreloc),bIsBigEndian);
                record.flags=read_uint32(nOffset+offsetof(XMACH_DEF::section,flags),bIsBigEndian);

                listResult.append(record);

                nOffset+=sizeof(XMACH_DEF::section);
            }
        }
    }

    return listResult;
}

quint32 XMACH::getNumberOfSections()
{
    QList<XMACH::COMMAND_RECORD> listCR=getCommandRecords();

    return getNumberOfSections(&listCR);
}

quint32 XMACH::getNumberOfSections(QList<XMACH::COMMAND_RECORD> *pList)
{
    quint32 nResult=0;

    bool bIs64=is64();
    bool bIsBigEndian=isBigEndian();

    if(bIs64)
    {
        QList<COMMAND_RECORD> listLCSegments=getCommandRecords(XMACH_DEF::S_LC_SEGMENT_64,pList);

        int nCount=listLCSegments.count();

        for(int i=0; i<nCount; i++)
        {
            qint64 nOffset=listLCSegments.at(i).nOffset;
            int nNumberOfSections=read_uint32(nOffset+offsetof(XMACH_DEF::segment_command_64,nsects),bIsBigEndian);

            nResult+=nNumberOfSections;
        }
    }
    else
    {
        QList<COMMAND_RECORD> listLCSegments=getCommandRecords(XMACH_DEF::S_LC_SEGMENT,pList);

        int nCount=listLCSegments.count();

        for(int i=0; i<nCount; i++)
        {
            qint64 nOffset=listLCSegments.at(i).nOffset;
            int nNumberOfSections=read_uint32(nOffset+offsetof(XMACH_DEF::segment_command,nsects),bIsBigEndian);

            nResult+=nNumberOfSections;
        }
    }

    return nResult;
}

bool XMACH::isSectionNamePresent(QString sName)
{
    QList<SECTION_RECORD> listSections=getSectionRecords();

    return isSectionNamePresent(sName,&listSections);
}

bool XMACH::isSectionNamePresent(QString sName, QList<XMACH::SECTION_RECORD> *pList)
{
    return (getSectionNumber(sName,pList)!=-1);
}

qint32 XMACH::getSectionNumber(QString sName, QList<XMACH::SECTION_RECORD> *pList)
{
    qint32 nResult=-1;

    int nCount=pList->count();

    for(int i=0; i<nCount; i++)
    {
        QString _sName=QString(pList->at(i).sectname);

        if(_sName.size()>16)
        {
            _sName.resize(16);
        }

        if(_sName==sName)
        {
            nResult=i;

            break;
        }
    }

    return nResult;
}

qint32 XMACH::getSectionNumber(QString sName)
{
    QList<SECTION_RECORD> listSections=getSectionRecords();

    return getSectionNumber(sName,&listSections);
}

quint32 XMACH::getLibraryCurrentVersion(QString sName, QList<XMACH::LIBRARY_RECORD> *pList)
{
    return getLibraryRecordByName(sName,pList).current_version;
}

XBinary::MODE XMACH::getMode()
{
    MODE result=MODE_32;

    if(getHeader_magic()==XMACH_DEF::S_MH_MAGIC_64)
    {
        result=MODE_64;
    }
    else
    {
        result=MODE_32;
    }

    return result;
}

QString XMACH::getArch()
{
    return getHeaderCpuTypesS().value(getHeader_cputype(),QString("UNKNOWN"));
}

int XMACH::getType()
{
    int nResult=TYPE_UNKNOWN;

    quint32 nFileType=getHeader_filetype();

    if(nFileType==XMACH_DEF::S_MH_OBJECT)
    {
        nResult=TYPE_OBJECT;
    }
    else if(nFileType==XMACH_DEF::S_MH_EXECUTE)
    {
        nResult=TYPE_EXECUTE;
    }
    else if(nFileType==XMACH_DEF::S_MH_FVMLIB)
    {
        nResult=TYPE_FVMLIB;
    }
    else if(nFileType==XMACH_DEF::S_MH_CORE)
    {
        nResult=TYPE_CORE;
    }
    else if(nFileType==XMACH_DEF::S_MH_PRELOAD)
    {
        nResult=TYPE_PRELOAD;
    }
    else if(nFileType==XMACH_DEF::S_MH_DYLIB)
    {
        nResult=TYPE_DYLIB;
    }
    else if(nFileType==XMACH_DEF::S_MH_DYLINKER)
    {
        nResult=TYPE_DYLINKER;
    }
    else if(nFileType==XMACH_DEF::S_MH_BUNDLE)
    {
        nResult=TYPE_BUNDLE;
    }

    return nResult;
}

QString XMACH::typeIdToString(int nType)
{
    QString sResult="Unknown"; // mb TODO translate

    switch(nType)
    {
        case TYPE_UNKNOWN:
            sResult=QString("Unknown");
            break; // mb TODO translate

        case TYPE_OBJECT:
            sResult=QString("OBJECT");
            break;

        case TYPE_EXECUTE:
            sResult=QString("EXECUTE");
            break;

        case TYPE_FVMLIB:
            sResult=QString("FVMLIB");
            break;

        case TYPE_CORE:
            sResult=QString("CORE");
            break;

        case TYPE_PRELOAD:
            sResult=QString("PRELOAD");
            break;

        case TYPE_DYLIB:
            sResult=QString("DYLIB");
            break;

        case TYPE_DYLINKER:
            sResult=QString("DYLINKER");
            break;

        case TYPE_BUNDLE:
            sResult=QString("BUNDLE");
            break;
    }

    return sResult;
}
