// copyright (c) 2017-2018 hors<horsicq@gmail.com>
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
#include "qelf.h"

QELF::QELF(QIODevice *__pDevice,bool bIsImage): QBinary(__pDevice,bIsImage)
{
}

QELF::~QELF()
{
}

bool QELF::bIsValid()
{
    return getIdent_Magic()==ELFMAG;
}

bool QELF::isBigEndian()
{
    return getIdent_DataEncoding()==ELFDATA2MSB;
}
bool QELF::is64()
{
    return getIdent_FileClass()==ELFCLASS64;
}

quint32 QELF::getIdent_Magic()
{
    return read_uint32((quint64)EI_MAG0);
}

void QELF::setIdent_Magic(quint32 value)
{
    write_uint32((quint64)EI_MAG0,value);
}

quint8 QELF::getIdent_FileClass()
{
    return read_uint8(EI_CLASS);
}

void QELF::setIdent_FileClass(quint8 value)
{
    write_uint8(EI_CLASS,value);
}

quint8 QELF::getIdent_DataEncoding()
{
    return read_uint8(EI_DATA);
}

void QELF::setIdent_DataEncoding(quint8 value)
{
    write_uint8(EI_DATA,value);
}

quint8 QELF::getIdent_FileVersion()
{
    return read_uint8(EI_VERSION);
}

void QELF::setIdent_FileVersion(quint8 value)
{
    write_uint8(EI_VERSION,value);
}

quint8 QELF::getIdent_osabi()
{
    return read_uint8(EI_OSABI);
}

void QELF::setIdent_osabi(quint8 value)
{
    write_uint8(EI_OSABI,value);
}

quint8 QELF::getIdent_abiversion()
{
    return read_uint8(EI_ABIVERSION);
}

void QELF::setIdent_abiversion(quint8 value)
{
    write_uint8(EI_ABIVERSION,value);
}

quint8 QELF::getIdent_Pad(int nPad)
{
    quint8 value=0;

    if(nPad<7)
    {
        value=read_uint8(EI_ABIVERSION+1+nPad);
    }

    return value;
}

void QELF::setIdent_Pad(quint8 value, int nPad)
{
    if(nPad<7)
    {
        write_uint8(EI_ABIVERSION+1+nPad,value);
    }
}

quint16 QELF::getHdr32_type()
{
    return read_uint16(offsetof(Elf32_Ehdr,e_type),isBigEndian());
}

void QELF::setHdr32_type(quint16 value)
{
    write_uint16(offsetof(Elf32_Ehdr,e_type),value,isBigEndian());
}

quint16 QELF::getHdr32_machine()
{
    return read_uint16(offsetof(Elf32_Ehdr,e_machine),isBigEndian());
}

void QELF::setHdr32_machine(quint16 value)
{
    write_uint16(offsetof(Elf32_Ehdr,e_machine),value,isBigEndian());
}

quint32 QELF::getHdr32_version()
{
    return read_uint32(offsetof(Elf32_Ehdr,e_version),isBigEndian());
}

void QELF::setHdr32_version(quint32 value)
{
    write_uint32(offsetof(Elf32_Ehdr,e_version),value,isBigEndian());
}

quint32 QELF::getHdr32_entry()
{
    return read_uint32(offsetof(Elf32_Ehdr,e_entry),isBigEndian());
}

void QELF::setHdr32_entry(quint32 value)
{
    write_uint32(offsetof(Elf32_Ehdr,e_entry),value,isBigEndian());
}

quint32 QELF::getHdr32_phoff()
{
    return read_uint32(offsetof(Elf32_Ehdr,e_phoff),isBigEndian());
}

void QELF::setHdr32_phoff(quint32 value)
{
    write_uint32(offsetof(Elf32_Ehdr,e_phoff),value,isBigEndian());
}

quint32 QELF::getHdr32_shoff()
{
    return read_uint32(offsetof(Elf32_Ehdr,e_shoff),isBigEndian());
}

void QELF::setHdr32_shoff(quint32 value)
{
    write_uint32(offsetof(Elf32_Ehdr,e_shoff),value,isBigEndian());
}

quint32 QELF::getHdr32_flags()
{
    return read_uint32(offsetof(Elf32_Ehdr,e_flags),isBigEndian());
}

void QELF::setHdr32_flags(quint32 value)
{
    write_uint32(offsetof(Elf32_Ehdr,e_flags),value,isBigEndian());
}

quint16 QELF::getHdr32_ehsize()
{
    return read_uint16(offsetof(Elf32_Ehdr,e_ehsize),isBigEndian());
}

void QELF::setHdr32_ehsize(quint16 value)
{
    write_uint16(offsetof(Elf32_Ehdr,e_ehsize),value,isBigEndian());
}

quint16 QELF::getHdr32_phentsize()
{
    return read_uint16(offsetof(Elf32_Ehdr,e_phentsize),isBigEndian());
}

void QELF::setHdr32_phentsize(quint16 value)
{
    write_uint16(offsetof(Elf32_Ehdr,e_phentsize),value,isBigEndian());
}

quint16 QELF::getHdr32_phnum()
{
    return read_uint16(offsetof(Elf32_Ehdr,e_phnum),isBigEndian());
}

void QELF::setHdr32_phnum(quint16 value)
{
    write_uint16(offsetof(Elf32_Ehdr,e_phnum),value,isBigEndian());
}

quint16 QELF::getHdr32_shentsize()
{
    return read_uint16(offsetof(Elf32_Ehdr,e_shentsize),isBigEndian());
}

void QELF::setHdr32_shentsize(quint16 value)
{
    write_uint16(offsetof(Elf32_Ehdr,e_shentsize),value,isBigEndian());
}

quint16 QELF::getHdr32_shnum()
{
    return read_uint16(offsetof(Elf32_Ehdr,e_shnum),isBigEndian());
}

void QELF::setHdr32_shnum(quint16 value)
{
    write_uint16(offsetof(Elf32_Ehdr,e_shnum),value,isBigEndian());
}

quint16 QELF::getHdr32_shstrndx()
{
    return read_uint16(offsetof(Elf32_Ehdr,e_shstrndx),isBigEndian());
}

void QELF::setHdr32_shstrndx(quint16 value)
{
    write_uint16(offsetof(Elf32_Ehdr,e_shstrndx),value,isBigEndian());
}

quint16 QELF::getHdr64_type()
{
    return read_uint16(offsetof(Elf64_Ehdr,e_type),isBigEndian());
}

void QELF::setHdr64_type(quint16 value)
{
    write_uint16(offsetof(Elf64_Ehdr,e_type),value,isBigEndian());
}

quint16 QELF::getHdr64_machine()
{
    return read_uint16(offsetof(Elf64_Ehdr,e_machine),isBigEndian());
}

void QELF::setHdr64_machine(quint16 value)
{
    write_uint16(offsetof(Elf64_Ehdr,e_machine),value,isBigEndian());
}

quint32 QELF::getHdr64_version()
{
    return read_uint32(offsetof(Elf64_Ehdr,e_version),isBigEndian());
}

void QELF::setHdr64_version(quint32 value)
{
    write_uint32(offsetof(Elf64_Ehdr,e_version),value,isBigEndian());
}

quint64 QELF::getHdr64_entry()
{
    return read_uint64(offsetof(Elf64_Ehdr,e_entry),isBigEndian());
}

void QELF::setHdr64_entry(quint64 value)
{
    write_uint64(offsetof(Elf64_Ehdr,e_entry),value,isBigEndian());
}

quint64 QELF::getHdr64_phoff()
{
    return read_uint64(offsetof(Elf64_Ehdr,e_phoff),isBigEndian());
}

void QELF::setHdr64_phoff(quint64 value)
{
    write_uint64(offsetof(Elf64_Ehdr,e_phoff),value,isBigEndian());
}

quint64 QELF::getHdr64_shoff()
{
    return read_uint64(offsetof(Elf64_Ehdr,e_shoff),isBigEndian());
}

void QELF::setHdr64_shoff(quint64 value)
{
    write_uint64(offsetof(Elf64_Ehdr,e_shoff),value,isBigEndian());
}

quint32 QELF::getHdr64_flags()
{
    return read_uint32(offsetof(Elf64_Ehdr,e_flags),isBigEndian());
}

void QELF::setHdr64_flags(quint32 value)
{
    write_uint32(offsetof(Elf64_Ehdr,e_flags),value,isBigEndian());
}

quint16 QELF::getHdr64_ehsize()
{
    return read_uint16(offsetof(Elf64_Ehdr,e_ehsize),isBigEndian());
}

void QELF::setHdr64_ehsize(quint16 value)
{
    write_uint16(offsetof(Elf64_Ehdr,e_ehsize),value,isBigEndian());
}

quint16 QELF::getHdr64_phentsize()
{
    return read_uint16(offsetof(Elf64_Ehdr,e_phentsize),isBigEndian());
}

void QELF::setHdr64_phentsize(quint16 value)
{
    write_uint16(offsetof(Elf64_Ehdr,e_phentsize),value,isBigEndian());
}

quint16 QELF::getHdr64_phnum()
{
    return read_uint16(offsetof(Elf64_Ehdr,e_phnum),isBigEndian());
}

void QELF::setHdr64_phnum(quint16 value)
{
    write_uint16(offsetof(Elf64_Ehdr,e_phnum),value,isBigEndian());
}

quint16 QELF::getHdr64_shentsize()
{
    return read_uint16(offsetof(Elf64_Ehdr,e_shentsize),isBigEndian());
}

void QELF::setHdr64_shentsize(quint16 value)
{
    write_uint16(offsetof(Elf64_Ehdr,e_shentsize),value,isBigEndian());
}

quint16 QELF::getHdr64_shnum()
{
    return read_uint16(offsetof(Elf64_Ehdr,e_shnum),isBigEndian());
}

void QELF::setHdr64_shnum(quint16 value)
{
    write_uint16(offsetof(Elf64_Ehdr,e_shnum),value,isBigEndian());
}

quint16 QELF::getHdr64_shstrndx()
{
    return read_uint16(offsetof(Elf64_Ehdr,e_shstrndx),isBigEndian());
}

void QELF::setHdr64_shstrndx(quint16 value)
{
    write_uint16(offsetof(Elf64_Ehdr,e_shstrndx),value,isBigEndian());
}

QMultiMap<quint32, QString> QELF::getHeaderTypeList()
{
    QMultiMap<quint32, QString> mapResult;
    mapResult.insert(0,"ET_NONE");
    mapResult.insert(1,"ET_REL");
    mapResult.insert(2,"ET_EXEC");
    mapResult.insert(3,"ET_DYN");
    mapResult.insert(4,"ET_CORE");
    mapResult.insert(5,"ET_NUM");
    mapResult.insert(0xff00,"ET_LOPROC");
    mapResult.insert(0xffff,"ET_HIPROC");
    return mapResult;
}

QMultiMap<quint32, QString> QELF::getHeaderMachineList()
{
    QMultiMap<quint32, QString> mapResult;
    mapResult.insert(0,"EM_NONE"); // TODO http://users.sosdg.org/~qiyong/mxr/source/sys/sys/exec_elf.h
    mapResult.insert(1,"EM_M32");
    mapResult.insert(2,"EM_SPARC");
    mapResult.insert(3,"EM_386");
    mapResult.insert(4,"EM_68K");
    mapResult.insert(5,"EM_88K");
    mapResult.insert(6,"EM_486"); /* Perhaps disused */
    mapResult.insert(7,"EM_860");
    mapResult.insert(8,"EM_MIPS");
    mapResult.insert(9,"EM_S370");
    mapResult.insert(10,"EM_MIPS_RS3_LE");
    mapResult.insert(15,"EM_PARISC");
    mapResult.insert(18,"EM_SPARC32PLUS");
    mapResult.insert(20,"EM_PPC");
    mapResult.insert(21,"EM_PPC64");
    mapResult.insert(22,"EM_S390");
    mapResult.insert(23,"EM_SPU");
    mapResult.insert(40,"EM_ARM");
    mapResult.insert(42,"EM_SH");
    mapResult.insert(43,"EM_SPARCV9");
    mapResult.insert(50,"EM_IA_64");
    mapResult.insert(62,"EM_X86_64");
    mapResult.insert(76,"EM_CRIS");
    mapResult.insert(87,"EM_V850");
    mapResult.insert(88,"EM_M32R");
    mapResult.insert(89,"EM_MN10300");
    mapResult.insert(92,"EM_OPENRISC");
    mapResult.insert(106,"EM_BLACKFIN");
    mapResult.insert(113,"EM_ALTERA_NIOS2");
    mapResult.insert(140,"EM_TI_C6000");
    mapResult.insert(183,"EM_AARCH64");
    mapResult.insert(0x5441,"EM_FRV");
    mapResult.insert(0x18ad,"EM_AVR32");
    mapResult.insert(0x9026,"EM_ALPHA");
    mapResult.insert(0x9080,"EM_CYGNUS_V850");
    mapResult.insert(0x9041,"EM_CYGNUS_M32R");
    mapResult.insert(0xA390,"EM_S390_OLD");
    mapResult.insert(0xbeef,"EM_CYGNUS_MN10300");
    return mapResult;
}

QMultiMap<quint32, QString> QELF::getHeaderVersionList()
{
    QMultiMap<quint32, QString> mapResult;
    mapResult.insert(1,"EV_CURRENT");
    return mapResult;
}

QMultiMap<quint32, QString> QELF::getIndentClassList()
{
    QMultiMap<quint32, QString> mapResult;
    mapResult.insert(0,"ELFCLASSNONE");
    mapResult.insert(1,"ELFCLASS32");
    mapResult.insert(2,"ELFCLASS64");
    return mapResult;
}

QMultiMap<quint32, QString> QELF::getIndentDataList()
{
    QMultiMap<quint32, QString> mapResult;
    mapResult.insert(0,"ELFDATANONE");
    mapResult.insert(1,"ELFDATA2LSB");
    mapResult.insert(2,"ELFDATA2MSB");
    return mapResult;
}

QMultiMap<quint32, QString> QELF::getIndentVersionList()
{
    QMultiMap<quint32, QString> mapResult;
    mapResult.insert(1,"EV_CURRENT");
    return mapResult;
}

QMultiMap<quint32, QString> QELF::getIndentOsabiList()
{
    QMultiMap<quint32, QString> mapResult;
    mapResult.insert(0,"ELFOSABI_SYSV");
    mapResult.insert(1,"ELFOSABI_HPUX");
    mapResult.insert(2,"ELFOSABI_NETBSD");
    mapResult.insert(3,"ELFOSABI_LINUX");
    mapResult.insert(4,"ELFOSABI_HURD");
    mapResult.insert(5,"ELFOSABI_86OPEN");
    mapResult.insert(6,"ELFOSABI_SOLARIS");
    mapResult.insert(7,"ELFOSABI_MONTEREY");
    mapResult.insert(8,"ELFOSABI_IRIX");
    mapResult.insert(9,"ELFOSABI_FREEBSD");
    mapResult.insert(10,"ELFOSABI_TRU64");
    mapResult.insert(11,"ELFOSABI_MODESTO");
    mapResult.insert(12,"ELFOSABI_OPENBSD");
    mapResult.insert(13,"ELFOSABI_OPENVMS");
    mapResult.insert(14,"ELFOSABI_NSK");
    mapResult.insert(15,"ELFOSABI_AROS");
    mapResult.insert(97,"ELFOSABI_ARM");
    mapResult.insert(255,"ELFOSABI_STANDALONE");
    return mapResult;
}

QMultiMap<quint32, QString> QELF::getSectionTypeList()
{
    QMultiMap<quint32, QString> mapResult;
    mapResult.insert(0,"SHT_NULL");
    mapResult.insert(1,"SHT_PROGBITS");
    mapResult.insert(2,"SHT_SYMTAB");
    mapResult.insert(3,"SHT_STRTAB");
    mapResult.insert(4,"SHT_RELA");
    mapResult.insert(5,"SHT_HASH");
    mapResult.insert(6,"SHT_DYNAMIC");
    mapResult.insert(7,"SHT_NOTE");
    mapResult.insert(8,"SHT_NOBITS");
    mapResult.insert(9,"SHT_REL");
    mapResult.insert(10,"SHT_SHLIB");
    mapResult.insert(11,"SHT_DYNSYM");
    mapResult.insert(14,"SHT_INIT_ARRAY");
    mapResult.insert(15,"SHT_FINI_ARRAY");
    mapResult.insert(16,"SHT_PREINIT_ARRAY");
    mapResult.insert(17,"SHT_GROUP");
    mapResult.insert(18,"SHT_SYMTAB_SHNDX");
    mapResult.insert(19,"SHT_NUM");
    mapResult.insert(0x60000000,"SHT_LOOS");
    mapResult.insert(0x6ffffff6,"SHT_GNU_HASH");
    mapResult.insert(0x6ffffffa,"SHT_SUNW_move");
    mapResult.insert(0x6ffffffc,"SHT_SUNW_syminfo");
    mapResult.insert(0x6ffffffd,"SHT_GNU_verdef");
    mapResult.insert(0x6ffffffe,"SHT_GNU_verneed");
    mapResult.insert(0x6fffffff,"SHT_GNU_versym");
    mapResult.insert(0x70000000,"SHT_LOPROC");
    mapResult.insert(0x70000001,"SHT_AMD64_UNWIND");
    mapResult.insert(0x70000002,"SHT_ARM_PREEMPTMAP");
    mapResult.insert(0x70000003,"SHT_ARM_ATTRIBUTES");
    mapResult.insert(0x70000004,"SHT_ARM_DEBUGOVERLAY");
    mapResult.insert(0x70000005,"SHT_ARM_OVERLAYSECTION");
    mapResult.insert(0x7fffffff,"SHT_HIPROC");
    mapResult.insert(0x80000000,"SHT_LOUSER");
    mapResult.insert(0xffffffff,"SHT_HIUSER");
    return mapResult;
}

QMultiMap<quint32, QString> QELF::getSectionFlagList()
{
    QMultiMap<quint32, QString> mapResult;
    mapResult.insert(0x00000001,"SHF_WRITE");
    mapResult.insert(0x00000002,"SHF_ALLOC");
    mapResult.insert(0x00000004,"SHF_EXECINSTR");
    mapResult.insert(0x00000010,"SHF_MERGE");
    mapResult.insert(0x00000020,"SHF_STRINGS");
    mapResult.insert(0x00000040,"SHF_INFO_LINK");
    mapResult.insert(0x00000080,"SHF_LINK_ORDER");
    mapResult.insert(0x00000100,"SHF_OS_NONCONFORMING");
    mapResult.insert(0x00000200,"SHF_GROUP");
    mapResult.insert(0x00000400,"SHF_TLS");
    mapResult.insert(0x0ff00000,"SHF_MASKOS");
    mapResult.insert(0xf0000000,"SHF_MASKPROC");
    mapResult.insert(0x40000000,"SHF_ORDERED");
    mapResult.insert(0x80000000,"SHF_EXCLUDE");
    return mapResult;
}

QMultiMap<quint32, QString> QELF::getProgramTypeList()
{
    QMultiMap<quint32, QString> mapResult;
    mapResult.insert(0,"PT_NULL");
    mapResult.insert(1,"PT_LOAD");
    mapResult.insert(2,"PT_DYNAMIC");
    mapResult.insert(3,"PT_INTERP");
    mapResult.insert(4,"PT_NOTE");
    mapResult.insert(5,"PT_SHLIB");
    mapResult.insert(6,"PT_PHDR");
    mapResult.insert(7,"PT_TLS");
    mapResult.insert(8,"PT_NUM");
    mapResult.insert(0x60000000,"PT_LOOS");
    mapResult.insert(0x6474e550,"PT_GNU_EH_FRAME");
    mapResult.insert(0x6474e551,"PT_GNU_STACK");
    mapResult.insert(0x6474e552,"PT_GNU_RELRO");
    mapResult.insert(0x6fffffff,"PT_HIOS");
    mapResult.insert(0x70000000,"PT_LOPROC");
    mapResult.insert(0x7fffffff,"PT_HIPROC");
    mapResult.insert(0x70000000,"PT_MIPS_REGINFO");
    return mapResult;
}

QMultiMap<quint32, QString> QELF::getProgramFlagList()
{
    QMultiMap<quint32, QString> mapResult;
    mapResult.insert(0x00000001,"PF_X");
    mapResult.insert(0x00000002,"PF_W");
    mapResult.insert(0x00000004,"PF_R");
    mapResult.insert(0x0ff00000,"PF_MASKOS");
    mapResult.insert(0xf0000000,"PF_MASKPROC");
    return mapResult;
}

QMultiMap<quint32, QString> QELF::getStringList(quint32 nSection)
{
    QMultiMap<quint32, QString> mapResult;

    if(nSection==0)
    {
        if(is64())
        {
            nSection=getHdr64_shstrndx();
        }
        else
        {
            nSection=getHdr32_shstrndx();
        }
    }

    QByteArray section=getSection(nSection);
    int nSize=section.size();
    char *pOffset=section.data();
    int nCount=0;

    while(nSize>0)
    {
        QString sString(pOffset+nCount);

        if(sString.length())
        {
            mapResult.insert(nCount,sString);
        }

        nCount+=sString.length()+1;
        nSize-=sString.length()+1;
    }

    return mapResult;
}

QString QELF::getStringFromList(quint32 nIndex, quint32 nSection)
{
    QString sResult;

    if(nSection==0)
    {
        if(is64())
        {
            nSection=getHdr64_shstrndx();
        }
        else
        {
            nSection=getHdr32_shstrndx();
        }
    }

    quint64 offset=0;
    quint64 size=0;

    if(is64())
    {
        Elf64_Shdr section_header=getElf64_Shdr(nSection);
        offset=section_header.sh_offset;
        size=section_header.sh_size;
    }
    else
    {
        Elf32_Shdr section_header=getElf32_Shdr(nSection);
        offset=section_header.sh_offset;
        size=section_header.sh_size;
    }

    if(nIndex<size)
    {
        sResult=read_ansiString(offset+nIndex);
    }

    return sResult;
}

QByteArray QELF::getSection(quint32 nIndex)
{
    if(is64())
    {
        Elf64_Shdr section_header=getElf64_Shdr(nIndex);
        return read_array(section_header.sh_offset,section_header.sh_size);
    }
    else
    {
        Elf32_Shdr section_header=getElf32_Shdr(nIndex);
        return read_array(section_header.sh_offset,section_header.sh_size);
    }
}

bool QELF::isSectionValid(quint32 nIndex)
{
    quint32 nNumberOfSections=getHdr32_shnum();
    return (nIndex<nNumberOfSections)&&(nIndex!=(quint32)-1);
}

QList<Elf32_Shdr> QELF::getElf32_ShdrList()
{
    QList<Elf32_Shdr> result;
    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    for(quint32 i=0; i<nNumberOfSections; i++)
    {
        Elf32_Shdr record;
        record.sh_name=read_uint32(offset+offsetof(Elf32_Shdr,sh_name),bIsBigEndian);
        record.sh_type=read_uint32(offset+offsetof(Elf32_Shdr,sh_type),bIsBigEndian);
        record.sh_flags=read_uint32(offset+offsetof(Elf32_Shdr,sh_flags),bIsBigEndian);
        record.sh_addr=read_uint32(offset+offsetof(Elf32_Shdr,sh_addr),bIsBigEndian);
        record.sh_offset=read_uint32(offset+offsetof(Elf32_Shdr,sh_offset),bIsBigEndian);
        record.sh_size=read_uint32(offset+offsetof(Elf32_Shdr,sh_size),bIsBigEndian);
        record.sh_link=read_uint32(offset+offsetof(Elf32_Shdr,sh_link),bIsBigEndian);
        record.sh_info=read_uint32(offset+offsetof(Elf32_Shdr,sh_info),bIsBigEndian);
        record.sh_addralign=read_uint32(offset+offsetof(Elf32_Shdr,sh_addralign),bIsBigEndian);
        record.sh_entsize=read_uint32(offset+offsetof(Elf32_Shdr,sh_entsize),bIsBigEndian);
        result.append(record);
        offset+=sizeof(Elf32_Shdr);
    }

    return result;
}

QList<Elf64_Shdr> QELF::getElf64_ShdrList()
{
    QList<Elf64_Shdr> result;
    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    for(quint32 i=0; i<nNumberOfSections; i++)
    {
        Elf64_Shdr record={};
        record.sh_name=read_uint32(offset+offsetof(Elf64_Shdr,sh_name),bIsBigEndian);
        record.sh_type=read_uint32(offset+offsetof(Elf64_Shdr,sh_type),bIsBigEndian);
        record.sh_flags=read_uint64(offset+offsetof(Elf64_Shdr,sh_flags),bIsBigEndian);
        record.sh_addr=read_uint64(offset+offsetof(Elf64_Shdr,sh_addr),bIsBigEndian);
        record.sh_offset=read_uint64(offset+offsetof(Elf64_Shdr,sh_offset),bIsBigEndian);
        record.sh_size=read_uint64(offset+offsetof(Elf64_Shdr,sh_size),bIsBigEndian);
        record.sh_link=read_uint32(offset+offsetof(Elf64_Shdr,sh_link),bIsBigEndian);
        record.sh_info=read_uint32(offset+offsetof(Elf64_Shdr,sh_info),bIsBigEndian);
        record.sh_addralign=read_uint64(offset+offsetof(Elf64_Shdr,sh_addralign),bIsBigEndian);
        record.sh_entsize=read_uint64(offset+offsetof(Elf64_Shdr,sh_entsize),bIsBigEndian);
        result.append(record);
        offset+=sizeof(Elf64_Shdr);
    }

    return result;
}

Elf32_Shdr QELF::getElf32_Shdr(quint32 nIndex)
{
    Elf32_Shdr result={};
    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf32_Shdr);
        result.sh_name=read_uint32(offset+offsetof(Elf32_Shdr,sh_name),bIsBigEndian);
        result.sh_type=read_uint32(offset+offsetof(Elf32_Shdr,sh_type),bIsBigEndian);
        result.sh_flags=read_uint32(offset+offsetof(Elf32_Shdr,sh_flags),bIsBigEndian);
        result.sh_addr=read_uint32(offset+offsetof(Elf32_Shdr,sh_addr),bIsBigEndian);
        result.sh_offset=read_uint32(offset+offsetof(Elf32_Shdr,sh_offset),bIsBigEndian);
        result.sh_size=read_uint32(offset+offsetof(Elf32_Shdr,sh_size),bIsBigEndian);
        result.sh_link=read_uint32(offset+offsetof(Elf32_Shdr,sh_link),bIsBigEndian);
        result.sh_info=read_uint32(offset+offsetof(Elf32_Shdr,sh_info),bIsBigEndian);
        result.sh_addralign=read_uint32(offset+offsetof(Elf32_Shdr,sh_addralign),bIsBigEndian);
        result.sh_entsize=read_uint32(offset+offsetof(Elf32_Shdr,sh_entsize),bIsBigEndian);
    }

    return result;
}

Elf64_Shdr QELF::getElf64_Shdr(quint32 nIndex)
{
    Elf64_Shdr result={};
    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf64_Shdr);
        result.sh_name=read_uint32(offset+offsetof(Elf64_Shdr,sh_name),bIsBigEndian);
        result.sh_type=read_uint32(offset+offsetof(Elf64_Shdr,sh_type),bIsBigEndian);
        result.sh_flags=read_uint64(offset+offsetof(Elf64_Shdr,sh_flags),bIsBigEndian);
        result.sh_addr=read_uint64(offset+offsetof(Elf64_Shdr,sh_addr),bIsBigEndian);
        result.sh_offset=read_uint64(offset+offsetof(Elf64_Shdr,sh_offset),bIsBigEndian);
        result.sh_size=read_uint64(offset+offsetof(Elf64_Shdr,sh_size),bIsBigEndian);
        result.sh_link=read_uint32(offset+offsetof(Elf64_Shdr,sh_link),bIsBigEndian);
        result.sh_info=read_uint32(offset+offsetof(Elf64_Shdr,sh_info),bIsBigEndian);
        result.sh_addralign=read_uint64(offset+offsetof(Elf64_Shdr,sh_addralign),bIsBigEndian);
        result.sh_entsize=read_uint64(offset+offsetof(Elf64_Shdr,sh_entsize),bIsBigEndian);
    }

    return result;
}

quint32 QELF::getElf32_Shdr_name(quint32 nIndex)
{
    quint32 result=0;
    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf32_Shdr);
        result=read_uint32(offset+offsetof(Elf32_Shdr,sh_name),bIsBigEndian);
    }

    return result;
}

quint32 QELF::getElf32_Shdr_type(quint32 nIndex)
{
    quint32 result=0;
    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf32_Shdr);
        result=read_uint32(offset+offsetof(Elf32_Shdr,sh_type),bIsBigEndian);
    }

    return result;
}

quint32 QELF::getElf32_Shdr_flags(quint32 nIndex)
{
    quint32 result=0;
    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf32_Shdr);
        result=read_uint32(offset+offsetof(Elf32_Shdr,sh_flags),bIsBigEndian);
    }

    return result;
}

quint32 QELF::getElf32_Shdr_addr(quint32 nIndex)
{
    quint32 result=0;
    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf32_Shdr);
        result=read_uint32(offset+offsetof(Elf32_Shdr,sh_addr),bIsBigEndian);
    }

    return result;
}

quint32 QELF::getElf32_Shdr_offset(quint32 nIndex)
{
    quint32 result=0;
    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf32_Shdr);
        result=read_uint32(offset+offsetof(Elf32_Shdr,sh_offset),bIsBigEndian);
    }

    return result;
}

quint32 QELF::getElf32_Shdr_size(quint32 nIndex)
{
    quint32 result=0;
    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf32_Shdr);
        result=read_uint32(offset+offsetof(Elf32_Shdr,sh_size),bIsBigEndian);
    }

    return result;
}

quint32 QELF::getElf32_Shdr_link(quint32 nIndex)
{
    quint32 result=0;
    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf32_Shdr);
        result=read_uint32(offset+offsetof(Elf32_Shdr,sh_link),bIsBigEndian);
    }

    return result;
}

quint32 QELF::getElf32_Shdr_info(quint32 nIndex)
{
    quint32 result=0;
    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf32_Shdr);
        result=read_uint32(offset+offsetof(Elf32_Shdr,sh_info),bIsBigEndian);
    }

    return result;
}

quint32 QELF::getElf32_Shdr_addralign(quint32 nIndex)
{
    quint32 result=0;
    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf32_Shdr);
        result=read_uint32(offset+offsetof(Elf32_Shdr,sh_addralign),bIsBigEndian);
    }

    return result;
}

quint32 QELF::getElf32_Shdr_entsize(quint32 nIndex)
{
    quint32 result=0;
    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf32_Shdr);
        result=read_uint32(offset+offsetof(Elf32_Shdr,sh_entsize),bIsBigEndian);
    }

    return result;
}

void QELF::setElf32_Shdr_name(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf32_Shdr);
        write_uint32(offset+offsetof(Elf32_Shdr,sh_name),value,bIsBigEndian);
    }
}

void QELF::setElf32_Shdr_type(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf32_Shdr);
        write_uint32(offset+offsetof(Elf32_Shdr,sh_type),value,bIsBigEndian);
    }
}

void QELF::setElf32_Shdr_flags(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf32_Shdr);
        write_uint32(offset+offsetof(Elf32_Shdr,sh_flags),value,bIsBigEndian);
    }
}

void QELF::setElf32_Shdr_addr(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf32_Shdr);
        write_uint32(offset+offsetof(Elf32_Shdr,sh_addr),value,bIsBigEndian);
    }
}

void QELF::setElf32_Shdr_offset(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf32_Shdr);
        write_uint32(offset+offsetof(Elf32_Shdr,sh_offset),value,bIsBigEndian);
    }
}

void QELF::setElf32_Shdr_size(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf32_Shdr);
        write_uint32(offset+offsetof(Elf32_Shdr,sh_size),value,bIsBigEndian);
    }
}

void QELF::setElf32_Shdr_link(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf32_Shdr);
        write_uint32(offset+offsetof(Elf32_Shdr,sh_link),value,bIsBigEndian);
    }
}

void QELF::setElf32_Shdr_info(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf32_Shdr);
        write_uint32(offset+offsetof(Elf32_Shdr,sh_info),value,bIsBigEndian);
    }
}

void QELF::setElf32_Shdr_addralign(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf32_Shdr);
        write_uint32(offset+offsetof(Elf32_Shdr,sh_addralign),value,bIsBigEndian);
    }
}

void QELF::setElf32_Shdr_entsize(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf32_Shdr);
        write_uint32(offset+offsetof(Elf32_Shdr,sh_entsize),value,bIsBigEndian);
    }
}

quint32 QELF::getElf64_Shdr_name(quint32 nIndex)
{
    quint32 result=0;
    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf64_Shdr);
        result=read_uint32(offset+offsetof(Elf64_Shdr,sh_name),bIsBigEndian);
    }

    return result;
}

quint32 QELF::getElf64_Shdr_type(quint32 nIndex)
{
    quint32 result=0;
    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf64_Shdr);
        result=read_uint32(offset+offsetof(Elf64_Shdr,sh_type),bIsBigEndian);
    }

    return result;
}

quint64 QELF::getElf64_Shdr_flags(quint32 nIndex)
{
    quint64 result=0;
    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf64_Shdr);
        result=read_uint64(offset+offsetof(Elf64_Shdr,sh_flags),bIsBigEndian);
    }

    return result;
}

quint64 QELF::getElf64_Shdr_addr(quint32 nIndex)
{
    quint64 result=0;
    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf64_Shdr);
        result=read_uint64(offset+offsetof(Elf64_Shdr,sh_addr),bIsBigEndian);
    }

    return result;
}

quint64 QELF::getElf64_Shdr_offset(quint32 nIndex)
{
    quint64 result=0;
    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf64_Shdr);
        result=read_uint64(offset+offsetof(Elf64_Shdr,sh_offset),bIsBigEndian);
    }

    return result;
}

quint64 QELF::getElf64_Shdr_size(quint32 nIndex)
{
    quint64 result=0;
    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf64_Shdr);
        result=read_uint64(offset+offsetof(Elf64_Shdr,sh_size),bIsBigEndian);
    }

    return result;
}

quint32 QELF::getElf64_Shdr_link(quint32 nIndex)
{
    quint32 result=0;
    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf64_Shdr);
        result=read_uint32(offset+offsetof(Elf64_Shdr,sh_link),bIsBigEndian);
    }

    return result;
}

quint32 QELF::getElf64_Shdr_info(quint32 nIndex)
{
    quint32 result=0;
    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf64_Shdr);
        result=read_uint32(offset+offsetof(Elf64_Shdr,sh_info),bIsBigEndian);
    }

    return result;
}

quint64 QELF::getElf64_Shdr_addralign(quint32 nIndex)
{
    quint64 result=0;
    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf64_Shdr);
        result=read_uint64(offset+offsetof(Elf64_Shdr,sh_addralign),bIsBigEndian);
    }

    return result;
}

quint64 QELF::getElf64_Shdr_entsize(quint32 nIndex)
{
    quint64 result=0;
    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf64_Shdr);
        result=read_uint64(offset+offsetof(Elf64_Shdr,sh_entsize),bIsBigEndian);
    }

    return result;
}

void QELF::setElf64_Shdr_name(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf64_Shdr);
        write_uint32(offset+offsetof(Elf64_Shdr,sh_name),value,bIsBigEndian);
    }
}

void QELF::setElf64_Shdr_type(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf64_Shdr);
        write_uint32(offset+offsetof(Elf64_Shdr,sh_type),value,bIsBigEndian);
    }
}

void QELF::setElf64_Shdr_flags(quint32 nIndex, quint64 value)
{
    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf64_Shdr);
        write_uint64(offset+offsetof(Elf64_Shdr,sh_flags),value,bIsBigEndian);
    }
}

void QELF::setElf64_Shdr_addr(quint32 nIndex, quint64 value)
{
    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf64_Shdr);
        write_uint64(offset+offsetof(Elf64_Shdr,sh_addr),value,bIsBigEndian);
    }
}

void QELF::setElf64_Shdr_offset(quint32 nIndex, quint64 value)
{
    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf64_Shdr);
        write_uint64(offset+offsetof(Elf64_Shdr,sh_offset),value,bIsBigEndian);
    }
}

void QELF::setElf64_Shdr_size(quint32 nIndex, quint64 value)
{
    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf64_Shdr);
        write_uint64(offset+offsetof(Elf64_Shdr,sh_size),value,bIsBigEndian);
    }
}

void QELF::setElf64_Shdr_link(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf64_Shdr);
        write_uint32(offset+offsetof(Elf64_Shdr,sh_link),value,bIsBigEndian);
    }
}

void QELF::setElf64_Shdr_info(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf64_Shdr);
        write_uint32(offset+offsetof(Elf64_Shdr,sh_info),value,bIsBigEndian);
    }
}

void QELF::setElf64_Shdr_addralign(quint32 nIndex, quint64 value)
{
    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf64_Shdr);
        write_uint64(offset+offsetof(Elf64_Shdr,sh_addralign),value,bIsBigEndian);
    }
}

void QELF::setElf64_Shdr_entsize(quint32 nIndex, quint64 value)
{
    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(Elf64_Shdr);
        write_uint64(offset+offsetof(Elf64_Shdr,sh_entsize),value,bIsBigEndian);
    }
}

QList<Elf32_Phdr> QELF::getElf32_PhdrList()
{
    QList<Elf32_Phdr> result;
    quint32 nNumberOfProgramms=getHdr32_phnum();
    quint32 offset=getHdr32_phoff();
    bool bIsBigEndian=isBigEndian();

    for(quint32 i=0; i<nNumberOfProgramms; i++)
    {
        Elf32_Phdr record={};
        record.p_type=read_uint32(offset+offsetof(Elf32_Phdr,p_type),bIsBigEndian);
        record.p_offset=read_uint32(offset+offsetof(Elf32_Phdr,p_offset),bIsBigEndian);
        record.p_vaddr=read_uint32(offset+offsetof(Elf32_Phdr,p_vaddr),bIsBigEndian);
        record.p_paddr=read_uint32(offset+offsetof(Elf32_Phdr,p_paddr),bIsBigEndian);
        record.p_filesz=read_uint32(offset+offsetof(Elf32_Phdr,p_filesz),bIsBigEndian);
        record.p_memsz=read_uint32(offset+offsetof(Elf32_Phdr,p_memsz),bIsBigEndian);
        record.p_flags=read_uint32(offset+offsetof(Elf32_Phdr,p_flags),bIsBigEndian);
        record.p_align=read_uint32(offset+offsetof(Elf32_Phdr,p_align),bIsBigEndian);
        result.append(record);
        offset+=sizeof(Elf32_Phdr);
    }

    return result;
}

QList<Elf64_Phdr> QELF::getElf64_PhdrList()
{
    QList<Elf64_Phdr> result;
    quint32 nNumberOfProgramms=getHdr64_phnum();
    quint64 offset=getHdr64_phoff();
    bool bIsBigEndian=isBigEndian();

    for(quint32 i=0; i<nNumberOfProgramms; i++)
    {
        Elf64_Phdr record={};
        record.p_type=read_uint32(offset+offsetof(Elf64_Phdr,p_type),bIsBigEndian);
        record.p_offset=read_uint64(offset+offsetof(Elf64_Phdr,p_offset),bIsBigEndian);
        record.p_vaddr=read_uint64(offset+offsetof(Elf64_Phdr,p_vaddr),bIsBigEndian);
        record.p_paddr=read_uint64(offset+offsetof(Elf64_Phdr,p_paddr),bIsBigEndian);
        record.p_filesz=read_uint64(offset+offsetof(Elf64_Phdr,p_filesz),bIsBigEndian);
        record.p_memsz=read_uint64(offset+offsetof(Elf64_Phdr,p_memsz),bIsBigEndian);
        record.p_flags=read_uint32(offset+offsetof(Elf64_Phdr,p_flags),bIsBigEndian);
        record.p_align=read_uint64(offset+offsetof(Elf64_Phdr,p_align),bIsBigEndian);
        result.append(record);
        offset+=sizeof(Elf64_Phdr);
    }

    return result;
}

Elf32_Phdr QELF::getElf32_Phdr(quint32 nIndex)
{
    Elf32_Phdr result={};
    quint32 nNumberOfPrograms=getHdr32_phnum();
    quint32 offset=getHdr32_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(Elf32_Phdr);
        result.p_type=read_uint32(offset+offsetof(Elf32_Phdr,p_type),bIsBigEndian);
        result.p_offset=read_uint32(offset+offsetof(Elf32_Phdr,p_offset),bIsBigEndian);
        result.p_vaddr=read_uint32(offset+offsetof(Elf32_Phdr,p_vaddr),bIsBigEndian);
        result.p_paddr=read_uint32(offset+offsetof(Elf32_Phdr,p_paddr),bIsBigEndian);
        result.p_filesz=read_uint32(offset+offsetof(Elf32_Phdr,p_filesz),bIsBigEndian);
        result.p_memsz=read_uint32(offset+offsetof(Elf32_Phdr,p_memsz),bIsBigEndian);
        result.p_flags=read_uint32(offset+offsetof(Elf32_Phdr,p_flags),bIsBigEndian);
        result.p_align=read_uint32(offset+offsetof(Elf32_Phdr,p_align),bIsBigEndian);
    }

    return result;
}

Elf64_Phdr QELF::getElf64_Phdr(quint32 nIndex)
{
    Elf64_Phdr result={};
    quint32 nNumberOfPrograms=getHdr64_phnum();
    quint64 offset=getHdr64_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(Elf64_Phdr);
        result.p_type=read_uint32(offset+offsetof(Elf64_Phdr,p_type),bIsBigEndian);
        result.p_offset=read_uint64(offset+offsetof(Elf64_Phdr,p_offset),bIsBigEndian);
        result.p_vaddr=read_uint64(offset+offsetof(Elf64_Phdr,p_vaddr),bIsBigEndian);
        result.p_paddr=read_uint64(offset+offsetof(Elf64_Phdr,p_paddr),bIsBigEndian);
        result.p_filesz=read_uint64(offset+offsetof(Elf64_Phdr,p_filesz),bIsBigEndian);
        result.p_memsz=read_uint64(offset+offsetof(Elf64_Phdr,p_memsz),bIsBigEndian);
        result.p_flags=read_uint32(offset+offsetof(Elf64_Phdr,p_flags),bIsBigEndian);
        result.p_align=read_uint64(offset+offsetof(Elf64_Phdr,p_align),bIsBigEndian);
    }

    return result;
}

quint32 QELF::getElf32_Phdr_type(quint32 nIndex)
{
    quint32 result=0;
    quint32 nNumberOfPrograms=getHdr32_phnum();
    quint32 offset=getHdr32_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(Elf32_Phdr);
        result=read_uint32(offset+offsetof(Elf32_Phdr,p_type),bIsBigEndian);
    }

    return result;
}

quint32 QELF::getElf32_Phdr_offset(quint32 nIndex)
{
    quint32 result=0;
    quint32 nNumberOfPrograms=getHdr32_phnum();
    quint32 offset=getHdr32_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(Elf32_Phdr);
        result=read_uint32(offset+offsetof(Elf32_Phdr,p_offset),bIsBigEndian);
    }

    return result;
}

quint32 QELF::getElf32_Phdr_vaddr(quint32 nIndex)
{
    quint32 result=0;
    quint32 nNumberOfPrograms=getHdr32_phnum();
    quint32 offset=getHdr32_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(Elf32_Phdr);
        result=read_uint32(offset+offsetof(Elf32_Phdr,p_vaddr),bIsBigEndian);
    }

    return result;
}

quint32 QELF::getElf32_Phdr_paddr(quint32 nIndex)
{
    quint32 result=0;
    quint32 nNumberOfPrograms=getHdr32_phnum();
    quint32 offset=getHdr32_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(Elf32_Phdr);
        result=read_uint32(offset+offsetof(Elf32_Phdr,p_paddr),bIsBigEndian);
    }

    return result;
}

quint32 QELF::getElf32_Phdr_filesz(quint32 nIndex)
{
    quint32 result=0;
    quint32 nNumberOfPrograms=getHdr32_phnum();
    quint32 offset=getHdr32_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(Elf32_Phdr);
        result=read_uint32(offset+offsetof(Elf32_Phdr,p_filesz),bIsBigEndian);
    }

    return result;
}

quint32 QELF::getElf32_Phdr_memsz(quint32 nIndex)
{
    quint32 result=0;
    quint32 nNumberOfPrograms=getHdr32_phnum();
    quint32 offset=getHdr32_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(Elf32_Phdr);
        result=read_uint32(offset+offsetof(Elf32_Phdr,p_memsz),bIsBigEndian);
    }

    return result;
}

quint32 QELF::getElf32_Phdr_flags(quint32 nIndex)
{
    quint32 result=0;
    quint32 nNumberOfPrograms=getHdr32_phnum();
    quint32 offset=getHdr32_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(Elf32_Phdr);
        result=read_uint32(offset+offsetof(Elf32_Phdr,p_flags),bIsBigEndian);
    }

    return result;
}

quint32 QELF::getElf32_Phdr_align(quint32 nIndex)
{
    quint32 result=0;
    quint32 nNumberOfPrograms=getHdr32_phnum();
    quint32 offset=getHdr32_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(Elf32_Phdr);
        result=read_uint32(offset+offsetof(Elf32_Phdr,p_align),bIsBigEndian);
    }

    return result;
}

void QELF::setElf32_Phdr_type(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfPrograms=getHdr32_phnum();
    quint32 offset=getHdr32_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(Elf32_Phdr);
        write_uint32(offset+offsetof(Elf32_Phdr,p_type),value,bIsBigEndian);
    }
}

void QELF::setElf32_Phdr_offset(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfPrograms=getHdr32_phnum();
    quint32 offset=getHdr32_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(Elf32_Phdr);
        write_uint32(offset+offsetof(Elf32_Phdr,p_offset),value,bIsBigEndian);
    }
}

void QELF::setElf32_Phdr_vaddr(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfPrograms=getHdr32_phnum();
    quint32 offset=getHdr32_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(Elf32_Phdr);
        write_uint32(offset+offsetof(Elf32_Phdr,p_vaddr),value,bIsBigEndian);
    }
}

void QELF::setElf32_Phdr_paddr(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfPrograms=getHdr32_phnum();
    quint32 offset=getHdr32_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(Elf32_Phdr);
        write_uint32(offset+offsetof(Elf32_Phdr,p_paddr),value,bIsBigEndian);
    }
}

void QELF::setElf32_Phdr_filesz(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfPrograms=getHdr32_phnum();
    quint32 offset=getHdr32_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(Elf32_Phdr);
        write_uint32(offset+offsetof(Elf32_Phdr,p_filesz),value,bIsBigEndian);
    }
}

void QELF::setElf32_Phdr_memsz(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfPrograms=getHdr32_phnum();
    quint32 offset=getHdr32_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(Elf32_Phdr);
        write_uint32(offset+offsetof(Elf32_Phdr,p_memsz),value,bIsBigEndian);
    }
}

void QELF::setElf32_Phdr_flags(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfPrograms=getHdr32_phnum();
    quint32 offset=getHdr32_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(Elf32_Phdr);
        write_uint32(offset+offsetof(Elf32_Phdr,p_flags),value,bIsBigEndian);
    }
}

void QELF::setElf32_Phdr_align(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfPrograms=getHdr32_phnum();
    quint32 offset=getHdr32_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(Elf32_Phdr);
        write_uint32(offset+offsetof(Elf32_Phdr,p_align),value,bIsBigEndian);
    }
}

quint32 QELF::getElf64_Phdr_type(quint32 nIndex)
{
    quint32 result=0;
    quint32 nNumberOfPrograms=getHdr64_phnum();
    quint64 offset=getHdr64_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(Elf64_Phdr);
        result=read_uint32(offset+offsetof(Elf64_Phdr,p_type),bIsBigEndian);
    }

    return result;
}

quint64 QELF::getElf64_Phdr_offset(quint32 nIndex)
{
    quint64 result=0;
    quint32 nNumberOfPrograms=getHdr64_phnum();
    quint64 offset=getHdr64_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(Elf64_Phdr);
        result=read_uint64(offset+offsetof(Elf64_Phdr,p_offset),bIsBigEndian);
    }

    return result;
}

quint64 QELF::getElf64_Phdr_vaddr(quint32 nIndex)
{
    quint64 result=0;
    quint32 nNumberOfPrograms=getHdr64_phnum();
    quint64 offset=getHdr64_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(Elf64_Phdr);
        result=read_uint64(offset+offsetof(Elf64_Phdr,p_vaddr),bIsBigEndian);
    }

    return result;
}

quint64 QELF::getElf64_Phdr_paddr(quint32 nIndex)
{
    quint64 result=0;
    quint32 nNumberOfPrograms=getHdr64_phnum();
    quint64 offset=getHdr64_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(Elf64_Phdr);
        result=read_uint64(offset+offsetof(Elf64_Phdr,p_paddr),bIsBigEndian);
    }

    return result;
}

quint64 QELF::getElf64_Phdr_filesz(quint32 nIndex)
{
    quint64 result=0;
    quint32 nNumberOfPrograms=getHdr64_phnum();
    quint64 offset=getHdr64_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(Elf64_Phdr);
        result=read_uint64(offset+offsetof(Elf64_Phdr,p_filesz),bIsBigEndian);
    }

    return result;
}

quint64 QELF::getElf64_Phdr_memsz(quint32 nIndex)
{
    quint64 result=0;
    quint32 nNumberOfPrograms=getHdr64_phnum();
    quint64 offset=getHdr64_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(Elf64_Phdr);
        result=read_uint64(offset+offsetof(Elf64_Phdr,p_memsz),bIsBigEndian);
    }

    return result;
}

quint32 QELF::getElf64_Phdr_flags(quint32 nIndex)
{
    quint32 result=0;
    quint32 nNumberOfPrograms=getHdr64_phnum();
    quint64 offset=getHdr64_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(Elf64_Phdr);
        result=read_uint32(offset+offsetof(Elf64_Phdr,p_flags),bIsBigEndian);
    }

    return result;
}

quint64 QELF::getElf64_Phdr_align(quint32 nIndex)
{
    quint64 result=0;
    quint32 nNumberOfPrograms=getHdr64_phnum();
    quint64 offset=getHdr64_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(Elf64_Phdr);
        result=read_uint64(offset+offsetof(Elf64_Phdr,p_align),bIsBigEndian);
    }

    return result;
}

void QELF::setElf64_Phdr_type(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfPrograms=getHdr64_phnum();
    quint64 offset=getHdr64_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(Elf64_Phdr);
        write_uint32(offset+offsetof(Elf64_Phdr,p_type),value,bIsBigEndian);
    }
}

void QELF::setElf64_Phdr_offset(quint32 nIndex, quint64 value)
{
    quint32 nNumberOfPrograms=getHdr64_phnum();
    quint64 offset=getHdr64_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(Elf64_Phdr);
        write_uint64(offset+offsetof(Elf64_Phdr,p_offset),value,bIsBigEndian);
    }
}

void QELF::setElf64_Phdr_vaddr(quint32 nIndex, quint64 value)
{
    quint32 nNumberOfPrograms=getHdr64_phnum();
    quint64 offset=getHdr64_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(Elf64_Phdr);
        write_uint64(offset+offsetof(Elf64_Phdr,p_vaddr),value,bIsBigEndian);
    }
}

void QELF::setElf64_Phdr_paddr(quint32 nIndex, quint64 value)
{
    quint32 nNumberOfPrograms=getHdr64_phnum();
    quint64 offset=getHdr64_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(Elf64_Phdr);
        write_uint64(offset+offsetof(Elf64_Phdr,p_paddr),value,bIsBigEndian);
    }
}

void QELF::setElf64_Phdr_filesz(quint32 nIndex, quint64 value)
{
    quint32 nNumberOfPrograms=getHdr64_phnum();
    quint64 offset=getHdr64_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(Elf64_Phdr);
        write_uint64(offset+offsetof(Elf64_Phdr,p_filesz),value,bIsBigEndian);
    }
}

void QELF::setElf64_Phdr_memsz(quint32 nIndex, quint64 value)
{
    quint32 nNumberOfPrograms=getHdr64_phnum();
    quint64 offset=getHdr64_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(Elf64_Phdr);
        write_uint64(offset+offsetof(Elf64_Phdr,p_memsz),value,bIsBigEndian);
    }
}

void QELF::setElf64_Phdr_flags(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfPrograms=getHdr64_phnum();
    quint64 offset=getHdr64_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(Elf64_Phdr);
        write_uint32(offset+offsetof(Elf64_Phdr,p_flags),value,bIsBigEndian);
    }
}

void QELF::setElf64_Phdr_align(quint32 nIndex, quint64 value)
{
    quint32 nNumberOfPrograms=getHdr64_phnum();
    quint64 offset=getHdr64_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(Elf64_Phdr);
        write_uint64(offset+offsetof(Elf64_Phdr,p_align),value,bIsBigEndian);
    }
}

int QELF::getSectionIndexByName(QString sName)
{
    int nResult=-1;

    if(is64())
    {
        for(int i=0; i<getHdr64_shnum(); i++)
        {
            int nCurrentNameIndex=getElf64_Shdr_name(i);

            if(getStringFromList(nCurrentNameIndex)==sName)
            {
                nResult=i;
                break;
            }
        }
    }
    else
    {
        for(int i=0; i<getHdr32_shnum(); i++)
        {
            int nCurrentNameIndex=getElf32_Shdr_name(i);

            if(getStringFromList(nCurrentNameIndex)==sName)
            {
                nResult=i;
                break;
            }
        }
    }

    return nResult;
}

QByteArray QELF::getSectionByName(QString sName)
{
    QByteArray baResult;
    quint32 nIndex=getSectionIndexByName(sName);

    if(nIndex!=(quint32)-1)
    {
        baResult=getSection(nIndex);
    }

    return baResult;
}

QString QELF::getProgramInterpreterName()
{
    QString sResult;
    sResult.append(getSectionByName(".interp"));
    return sResult;
}

QString QELF::getCommentString()
{
    QString sResult;
    sResult.append(getSectionByName(".comment"));
    return sResult;
}

QString QELF::getCompatibleKernelVersion()
{
    QString sResult;
    QByteArray baData=getSectionByName(".note.ABI-tag");
    bool bIsBigEndian=isBigEndian();
    NOTE note=getNote(baData,bIsBigEndian);

    if((note.type==1)&&(note.name=="GNU"))
    {
        quint32 kv[4];
        kv[0]=_read_uint32(note.desc.data()+0,bIsBigEndian);
        kv[1]=_read_uint32(note.desc.data()+4,bIsBigEndian);
        kv[2]=_read_uint32(note.desc.data()+8,bIsBigEndian);
        kv[3]=_read_uint32(note.desc.data()+12,bIsBigEndian);

        if(kv[0]==0)
        {
            sResult=QString("%1.%2.%3").arg(kv[1]).arg(kv[2]).arg(kv[3]);
        }
    }

    return sResult;
}

QELF::NOTE QELF::getNote(QByteArray &baData,bool bIsBigEndian)
{
    NOTE result={};
    char *pData=baData.data();
    int nDataSize=baData.size();

    if(nDataSize>=12)
    {
        quint16 nNameLength=   _read_int16(pData+0,bIsBigEndian);
        quint16 nDescLength=   _read_int16(pData+4,bIsBigEndian);
        quint16 nType=         _read_int16(pData+8,bIsBigEndian);

        if(nDataSize==12+nNameLength+nDescLength)
        {
            result.type=nType;

            if(nNameLength>=1)
            {
                result.name=_read_ansiString(pData+12,nNameLength-1);
            }

            result.desc=_read_byteArray((char *)(pData+12+nNameLength),(int)nDescLength);
        }
    }

    return result;
}


