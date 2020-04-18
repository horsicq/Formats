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
#include "xelf.h"

XELF::XELF(QIODevice *__pDevice, bool bIsImage, qint64 nImageBase): XBinary(__pDevice,bIsImage,nImageBase)
{
}

XELF::~XELF()
{
}

bool XELF::isValid()
{
    bool bResult=false;

    if(getIdent_Magic()==XELF_DEF::S_ELFMAG)
    {
        quint8 nClass=getIdent_class();

        if((nClass==XELF_DEF::S_ELFCLASS32)||(nClass==XELF_DEF::S_ELFCLASS64))
        {
            bResult=true;
        }
    }

    return bResult;
}

bool XELF::isBigEndian()
{
    return getIdent_data()==XELF_DEF::S_ELFDATA2MSB;
}

bool XELF::is32()
{
    return getIdent_class()==XELF_DEF::S_ELFCLASS32;
}

bool XELF::is64()
{
    return getIdent_class()==XELF_DEF::S_ELFCLASS64;
}

bool XELF::is64(QIODevice *pDevice)
{
    bool bResult=false;

    XELF xelf(pDevice);

    if(xelf.isValid())
    {
        bResult=xelf.is64();
    }

    return bResult;
}

qint64 XELF::getEhdrOffset()
{
    return 0;
}

qint64 XELF::getEhdr32Size()
{
    return sizeof(XELF_DEF::Elf32_Ehdr);
}

qint64 XELF::getEhdr64Size()
{
    return sizeof(XELF_DEF::Elf64_Ehdr);
}

quint32 XELF::getIdent_Magic()
{
    return read_uint32((quint64)XELF_DEF::S_EI_MAG0);
}

void XELF::setIdent_Magic(quint32 value)
{
    write_uint32((quint64)XELF_DEF::S_EI_MAG0,value);
}

quint8 XELF::getIdent_mag(int nMag)
{
    quint8 value=0;

    if(nMag<4)
    {
        value=read_uint8(nMag);
    }

    return value;
}

void XELF::setIdent_mag(quint8 value, int nMag)
{
    if(nMag<4)
    {
        write_uint8(nMag,value);
    }
}

quint8 XELF::getIdent_class()
{
    return read_uint8(XELF_DEF::S_EI_CLASS);
}

void XELF::setIdent_class(quint8 value)
{
    write_uint8(XELF_DEF::S_EI_CLASS,value);
}

quint8 XELF::getIdent_data()
{
    return read_uint8(XELF_DEF::S_EI_DATA);
}

void XELF::setIdent_data(quint8 value)
{
    write_uint8(XELF_DEF::S_EI_DATA,value);
}

quint8 XELF::getIdent_version()
{
    return read_uint8(XELF_DEF::S_EI_VERSION);
}

void XELF::setIdent_version(quint8 value)
{
    write_uint8(XELF_DEF::S_EI_VERSION,value);
}

quint8 XELF::getIdent_osabi()
{
    return read_uint8(XELF_DEF::S_EI_OSABI);
}

void XELF::setIdent_osabi(quint8 value)
{
    write_uint8(XELF_DEF::S_EI_OSABI,value);
}

quint8 XELF::getIdent_abiversion()
{
    return read_uint8(XELF_DEF::S_EI_ABIVERSION);
}

void XELF::setIdent_abiversion(quint8 value)
{
    write_uint8(XELF_DEF::S_EI_ABIVERSION,value);
}

quint8 XELF::getIdent_pad(int nPad)
{
    quint8 value=0;

    if(nPad<7)
    {
        value=read_uint8(XELF_DEF::S_EI_ABIVERSION+1+nPad);
    }

    return value;
}

void XELF::setIdent_pad(quint8 value, int nPad)
{
    if(nPad<7)
    {
        write_uint8(XELF_DEF::S_EI_ABIVERSION+1+nPad,value);
    }
}

quint16 XELF::getHdr32_type()
{
    return read_uint16(offsetof(XELF_DEF::Elf32_Ehdr,e_type),isBigEndian());
}

void XELF::setHdr32_type(quint16 value)
{
    write_uint16(offsetof(XELF_DEF::Elf32_Ehdr,e_type),value,isBigEndian());
}

quint16 XELF::getHdr32_machine()
{
    return read_uint16(offsetof(XELF_DEF::Elf32_Ehdr,e_machine),isBigEndian());
}

void XELF::setHdr32_machine(quint16 value)
{
    write_uint16(offsetof(XELF_DEF::Elf32_Ehdr,e_machine),value,isBigEndian());
}

quint32 XELF::getHdr32_version()
{
    return read_uint32(offsetof(XELF_DEF::Elf32_Ehdr,e_version),isBigEndian());
}

void XELF::setHdr32_version(quint32 value)
{
    write_uint32(offsetof(XELF_DEF::Elf32_Ehdr,e_version),value,isBigEndian());
}

quint32 XELF::getHdr32_entry()
{
    return read_uint32(offsetof(XELF_DEF::Elf32_Ehdr,e_entry),isBigEndian());
}

void XELF::setHdr32_entry(quint32 value)
{
    write_uint32(offsetof(XELF_DEF::Elf32_Ehdr,e_entry),value,isBigEndian());
}

quint32 XELF::getHdr32_phoff()
{
    return read_uint32(offsetof(XELF_DEF::Elf32_Ehdr,e_phoff),isBigEndian());
}

void XELF::setHdr32_phoff(quint32 value)
{
    write_uint32(offsetof(XELF_DEF::Elf32_Ehdr,e_phoff),value,isBigEndian());
}

quint32 XELF::getHdr32_shoff()
{
    return read_uint32(offsetof(XELF_DEF::Elf32_Ehdr,e_shoff),isBigEndian());
}

void XELF::setHdr32_shoff(quint32 value)
{
    write_uint32(offsetof(XELF_DEF::Elf32_Ehdr,e_shoff),value,isBigEndian());
}

quint32 XELF::getHdr32_flags()
{
    return read_uint32(offsetof(XELF_DEF::Elf32_Ehdr,e_flags),isBigEndian());
}

void XELF::setHdr32_flags(quint32 value)
{
    write_uint32(offsetof(XELF_DEF::Elf32_Ehdr,e_flags),value,isBigEndian());
}

quint16 XELF::getHdr32_ehsize()
{
    return read_uint16(offsetof(XELF_DEF::Elf32_Ehdr,e_ehsize),isBigEndian());
}

void XELF::setHdr32_ehsize(quint16 value)
{
    write_uint16(offsetof(XELF_DEF::Elf32_Ehdr,e_ehsize),value,isBigEndian());
}

quint16 XELF::getHdr32_phentsize()
{
    return read_uint16(offsetof(XELF_DEF::Elf32_Ehdr,e_phentsize),isBigEndian());
}

void XELF::setHdr32_phentsize(quint16 value)
{
    write_uint16(offsetof(XELF_DEF::Elf32_Ehdr,e_phentsize),value,isBigEndian());
}

quint16 XELF::getHdr32_phnum()
{
    return read_uint16(offsetof(XELF_DEF::Elf32_Ehdr,e_phnum),isBigEndian());
}

void XELF::setHdr32_phnum(quint16 value)
{
    write_uint16(offsetof(XELF_DEF::Elf32_Ehdr,e_phnum),value,isBigEndian());
}

quint16 XELF::getHdr32_shentsize()
{
    return read_uint16(offsetof(XELF_DEF::Elf32_Ehdr,e_shentsize),isBigEndian());
}

void XELF::setHdr32_shentsize(quint16 value)
{
    write_uint16(offsetof(XELF_DEF::Elf32_Ehdr,e_shentsize),value,isBigEndian());
}

quint16 XELF::getHdr32_shnum()
{
    return read_uint16(offsetof(XELF_DEF::Elf32_Ehdr,e_shnum),isBigEndian());
}

void XELF::setHdr32_shnum(quint16 value)
{
    write_uint16(offsetof(XELF_DEF::Elf32_Ehdr,e_shnum),value,isBigEndian());
}

quint16 XELF::getHdr32_shstrndx()
{
    return read_uint16(offsetof(XELF_DEF::Elf32_Ehdr,e_shstrndx),isBigEndian());
}

void XELF::setHdr32_shstrndx(quint16 value)
{
    write_uint16(offsetof(XELF_DEF::Elf32_Ehdr,e_shstrndx),value,isBigEndian());
}

quint16 XELF::getHdr64_type()
{
    return read_uint16(offsetof(XELF_DEF::Elf64_Ehdr,e_type),isBigEndian());
}

void XELF::setHdr64_type(quint16 value)
{
    write_uint16(offsetof(XELF_DEF::Elf64_Ehdr,e_type),value,isBigEndian());
}

quint16 XELF::getHdr64_machine()
{
    return read_uint16(offsetof(XELF_DEF::Elf64_Ehdr,e_machine),isBigEndian());
}

void XELF::setHdr64_machine(quint16 value)
{
    write_uint16(offsetof(XELF_DEF::Elf64_Ehdr,e_machine),value,isBigEndian());
}

quint32 XELF::getHdr64_version()
{
    return read_uint32(offsetof(XELF_DEF::Elf64_Ehdr,e_version),isBigEndian());
}

void XELF::setHdr64_version(quint32 value)
{
    write_uint32(offsetof(XELF_DEF::Elf64_Ehdr,e_version),value,isBigEndian());
}

quint64 XELF::getHdr64_entry()
{
    return read_uint64(offsetof(XELF_DEF::Elf64_Ehdr,e_entry),isBigEndian());
}

void XELF::setHdr64_entry(quint64 value)
{
    write_uint64(offsetof(XELF_DEF::Elf64_Ehdr,e_entry),value,isBigEndian());
}

quint64 XELF::getHdr64_phoff()
{
    return read_uint64(offsetof(XELF_DEF::Elf64_Ehdr,e_phoff),isBigEndian());
}

void XELF::setHdr64_phoff(quint64 value)
{
    write_uint64(offsetof(XELF_DEF::Elf64_Ehdr,e_phoff),value,isBigEndian());
}

quint64 XELF::getHdr64_shoff()
{
    return read_uint64(offsetof(XELF_DEF::Elf64_Ehdr,e_shoff),isBigEndian());
}

void XELF::setHdr64_shoff(quint64 value)
{
    write_uint64(offsetof(XELF_DEF::Elf64_Ehdr,e_shoff),value,isBigEndian());
}

quint32 XELF::getHdr64_flags()
{
    return read_uint32(offsetof(XELF_DEF::Elf64_Ehdr,e_flags),isBigEndian());
}

void XELF::setHdr64_flags(quint32 value)
{
    write_uint32(offsetof(XELF_DEF::Elf64_Ehdr,e_flags),value,isBigEndian());
}

quint16 XELF::getHdr64_ehsize()
{
    return read_uint16(offsetof(XELF_DEF::Elf64_Ehdr,e_ehsize),isBigEndian());
}

void XELF::setHdr64_ehsize(quint16 value)
{
    write_uint16(offsetof(XELF_DEF::Elf64_Ehdr,e_ehsize),value,isBigEndian());
}

quint16 XELF::getHdr64_phentsize()
{
    return read_uint16(offsetof(XELF_DEF::Elf64_Ehdr,e_phentsize),isBigEndian());
}

void XELF::setHdr64_phentsize(quint16 value)
{
    write_uint16(offsetof(XELF_DEF::Elf64_Ehdr,e_phentsize),value,isBigEndian());
}

quint16 XELF::getHdr64_phnum()
{
    return read_uint16(offsetof(XELF_DEF::Elf64_Ehdr,e_phnum),isBigEndian());
}

void XELF::setHdr64_phnum(quint16 value)
{
    write_uint16(offsetof(XELF_DEF::Elf64_Ehdr,e_phnum),value,isBigEndian());
}

quint16 XELF::getHdr64_shentsize()
{
    return read_uint16(offsetof(XELF_DEF::Elf64_Ehdr,e_shentsize),isBigEndian());
}

void XELF::setHdr64_shentsize(quint16 value)
{
    write_uint16(offsetof(XELF_DEF::Elf64_Ehdr,e_shentsize),value,isBigEndian());
}

quint16 XELF::getHdr64_shnum()
{
    return read_uint16(offsetof(XELF_DEF::Elf64_Ehdr,e_shnum),isBigEndian());
}

void XELF::setHdr64_shnum(quint16 value)
{
    write_uint16(offsetof(XELF_DEF::Elf64_Ehdr,e_shnum),value,isBigEndian());
}

quint16 XELF::getHdr64_shstrndx()
{
    return read_uint16(offsetof(XELF_DEF::Elf64_Ehdr,e_shstrndx),isBigEndian());
}

void XELF::setHdr64_shstrndx(quint16 value)
{
    write_uint16(offsetof(XELF_DEF::Elf64_Ehdr,e_shstrndx),value,isBigEndian());
}

XELF_DEF::Elf_Ehdr XELF::getHdr()
{
    XELF_DEF::Elf_Ehdr result={};

    bool bIsbigEndian=isBigEndian();

    if(!is64())
    {
        result.e_type=read_uint16(offsetof(XELF_DEF::Elf32_Ehdr,e_type),bIsbigEndian);
        result.e_machine=read_uint16(offsetof(XELF_DEF::Elf32_Ehdr,e_machine),bIsbigEndian);
        result.e_version=read_uint32(offsetof(XELF_DEF::Elf32_Ehdr,e_version),bIsbigEndian);
        result.e_entry=read_uint32(offsetof(XELF_DEF::Elf32_Ehdr,e_entry),bIsbigEndian);
        result.e_phoff=read_uint32(offsetof(XELF_DEF::Elf32_Ehdr,e_phoff),bIsbigEndian);
        result.e_shoff=read_uint32(offsetof(XELF_DEF::Elf32_Ehdr,e_shoff),bIsbigEndian);
        result.e_flags=read_uint32(offsetof(XELF_DEF::Elf32_Ehdr,e_flags),bIsbigEndian);
        result.e_ehsize=read_uint16(offsetof(XELF_DEF::Elf32_Ehdr,e_ehsize),bIsbigEndian);
        result.e_phentsize=read_uint16(offsetof(XELF_DEF::Elf32_Ehdr,e_phentsize),bIsbigEndian);
        result.e_phnum=read_uint16(offsetof(XELF_DEF::Elf32_Ehdr,e_phnum),bIsbigEndian);
        result.e_shentsize=read_uint16(offsetof(XELF_DEF::Elf32_Ehdr,e_shentsize),bIsbigEndian);
        result.e_shnum=read_uint16(offsetof(XELF_DEF::Elf32_Ehdr,e_shnum),bIsbigEndian);
        result.e_shstrndx=read_uint16(offsetof(XELF_DEF::Elf32_Ehdr,e_shstrndx),bIsbigEndian);
    }
    else
    {
        result.e_type=read_uint16(offsetof(XELF_DEF::Elf64_Ehdr,e_type),bIsbigEndian);
        result.e_machine=read_uint16(offsetof(XELF_DEF::Elf64_Ehdr,e_machine),bIsbigEndian);
        result.e_version=read_uint32(offsetof(XELF_DEF::Elf64_Ehdr,e_version),bIsbigEndian);
        result.e_entry=read_uint64(offsetof(XELF_DEF::Elf64_Ehdr,e_entry),bIsbigEndian);
        result.e_phoff=read_uint64(offsetof(XELF_DEF::Elf64_Ehdr,e_phoff),bIsbigEndian);
        result.e_shoff=read_uint64(offsetof(XELF_DEF::Elf64_Ehdr,e_shoff),bIsbigEndian);
        result.e_flags=read_uint32(offsetof(XELF_DEF::Elf64_Ehdr,e_flags),bIsbigEndian);
        result.e_ehsize=read_uint16(offsetof(XELF_DEF::Elf64_Ehdr,e_ehsize),bIsbigEndian);
        result.e_phentsize=read_uint16(offsetof(XELF_DEF::Elf64_Ehdr,e_phentsize),bIsbigEndian);
        result.e_phnum=read_uint16(offsetof(XELF_DEF::Elf64_Ehdr,e_phnum),bIsbigEndian);
        result.e_shentsize=read_uint16(offsetof(XELF_DEF::Elf64_Ehdr,e_shentsize),bIsbigEndian);
        result.e_shnum=read_uint16(offsetof(XELF_DEF::Elf64_Ehdr,e_shnum),bIsbigEndian);
        result.e_shstrndx=read_uint16(offsetof(XELF_DEF::Elf64_Ehdr,e_shstrndx),bIsbigEndian);
    }

    return result;
}

QMap<quint64, QString> XELF::getTypes()
{
    QMap<quint64, QString> mapResult;

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

QMap<quint64, QString> XELF::getTypesS()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0,"NONE");
    mapResult.insert(1,"REL");
    mapResult.insert(2,"EXEC");
    mapResult.insert(3,"DYN");
    mapResult.insert(4,"CORE");
    mapResult.insert(5,"NUM");
    mapResult.insert(0xff00,"LOPROC");
    mapResult.insert(0xffff,"HIPROC");

    return mapResult;
}

QMap<quint64, QString> XELF::getMachines()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0,"EM_NONE"); // TODO http://users.sosdg.org/~qiyong/mxr/source/sys/sys/exec_XELF_DEF::Elf.h
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
    mapResult.insert(11,"EM_RS6000");
    mapResult.insert(15,"EM_PARISC");
    mapResult.insert(16,"EM_nCUBE");
    mapResult.insert(17,"EM_VPP500");
    mapResult.insert(18,"EM_SPARC32PLUS");
    mapResult.insert(19,"EM_960");
    mapResult.insert(20,"EM_PPC");
    mapResult.insert(21,"EM_PPC64");
    mapResult.insert(22,"EM_S390");
    mapResult.insert(23,"EM_SPU");
    mapResult.insert(36,"EM_V800");
    mapResult.insert(37,"EM_FR20");
    mapResult.insert(38,"EM_RH32");
    mapResult.insert(39,"EM_RCE");
    mapResult.insert(40,"EM_ARM");
    mapResult.insert(41,"EM_ALPHA");
    mapResult.insert(42,"EM_SH");
    mapResult.insert(43,"EM_SPARCV9");
    mapResult.insert(44,"EM_TRICORE");
    mapResult.insert(45,"EM_ARC");
    mapResult.insert(46,"EM_H8_300");
    mapResult.insert(47,"EM_H8_300H");
    mapResult.insert(48,"EM_H8S");
    mapResult.insert(49,"EM_H8_500");
    mapResult.insert(50,"EM_IA_64");
    mapResult.insert(51,"EM_MIPS_X");
    mapResult.insert(52,"EM_COLDFIRE");
    mapResult.insert(53,"EM_68HC12");
    mapResult.insert(54,"EM_MMA");
    mapResult.insert(55,"EM_PCP");
    mapResult.insert(56,"EM_NCPU");
    mapResult.insert(57,"EM_NDR1");
    mapResult.insert(58,"EM_STARCORE");
    mapResult.insert(59,"EM_ME16");
    mapResult.insert(60,"EM_ST100");
    mapResult.insert(61,"EM_TINYJ");
    mapResult.insert(62,"EM_AMD64"); // EM_X86_64
    mapResult.insert(63,"EM_PDSP");
    mapResult.insert(66,"EM_FX66");
    mapResult.insert(67,"EM_ST9PLUS");
    mapResult.insert(68,"EM_ST7");
    mapResult.insert(69,"EM_68HC16");
    mapResult.insert(70,"EM_68HC11");
    mapResult.insert(71,"EM_68HC08");
    mapResult.insert(72,"EM_68HC05");
    mapResult.insert(73,"EM_SVX");
    mapResult.insert(74,"EM_ST19");
    mapResult.insert(75,"EM_VAX");
    mapResult.insert(76,"EM_CRIS");
    mapResult.insert(77,"EM_JAVELIN");
    mapResult.insert(78,"EM_FIREPATH");
    mapResult.insert(79,"EM_ZSP");
    mapResult.insert(80,"EM_MMIX");
    mapResult.insert(81,"EM_HUANY");
    mapResult.insert(82,"EM_PRISM");
    mapResult.insert(83,"EM_AVR");
    mapResult.insert(84,"EM_FR30");
    mapResult.insert(85,"EM_D10V");
    mapResult.insert(86,"EM_D30V");
    mapResult.insert(87,"EM_V850");
    mapResult.insert(88,"EM_M32R");
    mapResult.insert(89,"EM_MN10300");
    mapResult.insert(90,"EM_MN10200");
    mapResult.insert(91,"EM_PJ");
    mapResult.insert(92,"EM_OPENRISC");
    mapResult.insert(93,"EM_ARC_A5");
    mapResult.insert(94,"EM_XTENSA");
    mapResult.insert(95,"EM_NUM"); // TODO
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

QMap<quint64, QString> XELF::getMachinesS()
{
    QMap<quint64, QString> mapResult;

    // TODO ftp://ftp.eso.org/pub/solaris/rec_patches/8/8_Recommended/109147-39/SUNWhea/reloc/usr/include/sys/elf.h
    // TODO http://users.sosdg.org/~qiyong/mxr/source/sys/sys/elf.h
    mapResult.insert(0,"NONE");
    mapResult.insert(1,"M32");
    mapResult.insert(2,"SPARC");
    mapResult.insert(3,"386");
    mapResult.insert(4,"68K");
    mapResult.insert(5,"88K");
    mapResult.insert(6,"486"); /* Perhaps disused */
    mapResult.insert(7,"860");
    mapResult.insert(8,"MIPS");
    mapResult.insert(9,"S370");
    mapResult.insert(10,"MIPS_RS3_LE");
    mapResult.insert(11,"RS6000");
    mapResult.insert(15,"PARISC");
    mapResult.insert(16,"nCUBE");
    mapResult.insert(17,"VPP500");
    mapResult.insert(18,"SPARC32PLUS");
    mapResult.insert(19,"960");
    mapResult.insert(20,"PPC");
    mapResult.insert(21,"PPC64");
    mapResult.insert(22,"S390");
    mapResult.insert(23,"SPU");
    mapResult.insert(36,"V800");
    mapResult.insert(37,"FR20");
    mapResult.insert(38,"RH32");
    mapResult.insert(39,"RCE");
    mapResult.insert(40,"ARM");
    mapResult.insert(41,"ALPHA");
    mapResult.insert(42,"SH");
    mapResult.insert(43,"SPARCV9");
    mapResult.insert(44,"TRICORE");
    mapResult.insert(45,"ARC");
    mapResult.insert(46,"H8_300");
    mapResult.insert(47,"H8_300H");
    mapResult.insert(48,"H8S");
    mapResult.insert(49,"H8_500");
    mapResult.insert(50,"IA_64");
    mapResult.insert(51,"MIPS_X");
    mapResult.insert(52,"COLDFIRE");
    mapResult.insert(53,"68HC12");
    mapResult.insert(54,"MMA");
    mapResult.insert(55,"PCP");
    mapResult.insert(56,"NCPU");
    mapResult.insert(57,"NDR1");
    mapResult.insert(58,"STARCORE");
    mapResult.insert(59,"ME16");
    mapResult.insert(60,"ST100");
    mapResult.insert(61,"TINYJ");
    mapResult.insert(62,"AMD64"); // X86_64
    mapResult.insert(63,"PDSP");
    mapResult.insert(66,"FX66");
    mapResult.insert(67,"ST9PLUS");
    mapResult.insert(68,"ST7");
    mapResult.insert(69,"68HC16");
    mapResult.insert(70,"68HC11");
    mapResult.insert(71,"68HC08");
    mapResult.insert(72,"68HC05");
    mapResult.insert(73,"SVX");
    mapResult.insert(74,"ST19");
    mapResult.insert(75,"VAX");
    mapResult.insert(76,"CRIS");
    mapResult.insert(77,"JAVELIN");
    mapResult.insert(78,"FIREPATH");
    mapResult.insert(79,"ZSP");
    mapResult.insert(80,"MMIX");
    mapResult.insert(81,"HUANY");
    mapResult.insert(82,"PRISM");
    mapResult.insert(83,"AVR");
    mapResult.insert(84,"FR30");
    mapResult.insert(85,"D10V");
    mapResult.insert(86,"D30V");
    mapResult.insert(87,"V850");
    mapResult.insert(88,"M32R");
    mapResult.insert(89,"MN10300");
    mapResult.insert(89,"MN10300");
    mapResult.insert(90,"MN10200");
    mapResult.insert(91,"PJ"); // TODO
    mapResult.insert(92,"OPENRISC");
    mapResult.insert(93,"ARC_A5");
    mapResult.insert(94,"XTENSA");
    mapResult.insert(95,"NUM");
    mapResult.insert(106,"BLACKFIN");
    mapResult.insert(113,"ALTERA_NIOS2");
    mapResult.insert(140,"TI_C6000");
    mapResult.insert(183,"AARCH64");
    mapResult.insert(0x5441,"FRV");
    mapResult.insert(0x18ad,"AVR32");
    mapResult.insert(0x9026,"ALPHA");
    mapResult.insert(0x9080,"CYGNUS_V850");
    mapResult.insert(0x9041,"CYGNUS_M32R");
    mapResult.insert(0xA390,"S390_OLD");
    mapResult.insert(0xbeef,"CYGNUS_MN10300");

    return mapResult;
}

QMap<quint64, QString> XELF::getHeaderVersionList()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(1,"EV_CURRENT");

    return mapResult;
}

QMap<quint64, QString> XELF::getIndentClasses()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0,"ELFCLASSNONE");
    mapResult.insert(1,"ELFCLASS32");
    mapResult.insert(2,"ELFCLASS64");

    return mapResult;
}

QMap<quint64, QString> XELF::getIndentClassesS()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0,"NONE");
    mapResult.insert(1,"32");
    mapResult.insert(2,"64");

    return mapResult;
}

QMap<quint64, QString> XELF::getIndentDatas()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0,"ELFDATANONE");
    mapResult.insert(1,"ELFDATA2LSB");
    mapResult.insert(2,"ELFDATA2MSB");

    return mapResult;
}

QMap<quint64, QString> XELF::getIndentDatasS()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0,"NONE");
    mapResult.insert(1,"2LSB");
    mapResult.insert(2,"2MSB");

    return mapResult;
}

QMap<quint64, QString> XELF::getIndentVersions()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(1,"EV_CURRENT");

    return mapResult;
}

QMap<quint64, QString> XELF::getIndentVersionsS()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(1,"CURRENT");

    return mapResult;
}

QMap<quint64, QString> XELF::getIndentOsabis()
{
    QMap<quint64, QString> mapResult;

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
    mapResult.insert(64,"ELFOSABI_ARM_AEABI");
    mapResult.insert(97,"ELFOSABI_ARM");
    mapResult.insert(255,"ELFOSABI_STANDALONE");

    return mapResult;
}

QMap<quint64, QString> XELF::getIndentOsabisS()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0,"SYSV");
    mapResult.insert(1,"HPUX");
    mapResult.insert(2,"NETBSD");
    mapResult.insert(3,"LINUX");
    mapResult.insert(4,"HURD");
    mapResult.insert(5,"86OPEN");
    mapResult.insert(6,"SOLARIS");
    mapResult.insert(7,"MONTEREY");
    mapResult.insert(8,"IRIX");
    mapResult.insert(9,"FREEBSD");
    mapResult.insert(10,"TRU64");
    mapResult.insert(11,"MODESTO");
    mapResult.insert(12,"OPENBSD");
    mapResult.insert(13,"OPENVMS");
    mapResult.insert(14,"NSK");
    mapResult.insert(15,"AROS");
    mapResult.insert(64,"ARM_AEABI");
    mapResult.insert(97,"ARM");
    mapResult.insert(255,"STANDALONE");

    return mapResult;
}

QMap<quint64, QString> XELF::getSectionTypes()
{
    QMap<quint64, QString> mapResult;

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

QMap<quint64, QString> XELF::getSectionTypesS()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0,"NULL");
    mapResult.insert(1,"PROGBITS");
    mapResult.insert(2,"SYMTAB");
    mapResult.insert(3,"STRTAB");
    mapResult.insert(4,"RELA");
    mapResult.insert(5,"HASH");
    mapResult.insert(6,"DYNAMIC");
    mapResult.insert(7,"NOTE");
    mapResult.insert(8,"NOBITS");
    mapResult.insert(9,"REL");
    mapResult.insert(10,"SHLIB");
    mapResult.insert(11,"DYNSYM");
    mapResult.insert(14,"INIT_ARRAY");
    mapResult.insert(15,"FINI_ARRAY");
    mapResult.insert(16,"PREINIT_ARRAY");
    mapResult.insert(17,"GROUP");
    mapResult.insert(18,"SYMTAB_SHNDX");
    mapResult.insert(19,"NUM");
    mapResult.insert(0x60000000,"LOOS");
    mapResult.insert(0x6ffffff6,"GNU_HASH");
    mapResult.insert(0x6ffffffa,"SUNW_move");
    mapResult.insert(0x6ffffffc,"SUNW_syminfo");
    mapResult.insert(0x6ffffffd,"GNU_verdef");
    mapResult.insert(0x6ffffffe,"GNU_verneed");
    mapResult.insert(0x6fffffff,"GNU_versym");
    mapResult.insert(0x70000000,"LOPROC");
    mapResult.insert(0x70000001,"AMD64_UNWIND");
    mapResult.insert(0x70000002,"ARM_PREEMPTMAP");
    mapResult.insert(0x70000003,"ARM_ATTRIBUTES");
    mapResult.insert(0x70000004,"ARM_DEBUGOVERLAY");
    mapResult.insert(0x70000005,"ARM_OVERLAYSECTION");
    mapResult.insert(0x7fffffff,"HIPROC");
    mapResult.insert(0x80000000,"LOUSER");
    mapResult.insert(0xffffffff,"HIUSER");

    return mapResult;
}

QMap<quint64, QString> XELF::getSectionFlags()
{
    QMap<quint64, QString> mapResult;

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

QMap<quint64, QString> XELF::getSectionFlagsS()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0x00000001,"WRITE");
    mapResult.insert(0x00000002,"ALLOC");
    mapResult.insert(0x00000004,"EXECINSTR");
    mapResult.insert(0x00000010,"MERGE");
    mapResult.insert(0x00000020,"STRINGS");
    mapResult.insert(0x00000040,"INFO_LINK");
    mapResult.insert(0x00000080,"LINK_ORDER");
    mapResult.insert(0x00000100,"OS_NONCONFORMING");
    mapResult.insert(0x00000200,"GROUP");
    mapResult.insert(0x00000400,"TLS");
    mapResult.insert(0x0ff00000,"MASKOS");
    mapResult.insert(0xf0000000,"MASKPROC");
    mapResult.insert(0x40000000,"ORDERED");
    mapResult.insert(0x80000000,"EXCLUDE");

    return mapResult;
}

QMap<quint64, QString> XELF::getProgramTypes()
{
    QMap<quint64, QString> mapResult;

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
    mapResult.insert(0x6464e550,"PT_SUNW_UNWIND");
    mapResult.insert(0x6474e550,"PT_GNU_EH_FRAME"); // PT_SUNW_EH_FRAME
    mapResult.insert(0x6474e551,"PT_GNU_STACK");
    mapResult.insert(0x6474e552,"PT_GNU_RELRO");
    mapResult.insert(0x6ffffffa,"PT_LOSUNW");
    mapResult.insert(0x6ffffffa,"PT_SUNWBSS");
    mapResult.insert(0x6ffffffb,"PT_SUNWSTACK");
    mapResult.insert(0x6ffffffc,"PT_SUNWDTRACE");
    mapResult.insert(0x6ffffffd,"PT_SUNWCAP");
    mapResult.insert(0x6fffffff,"PT_HIOS"); // PT_HISUNW
    mapResult.insert(0x70000000,"PT_LOPROC");
    mapResult.insert(0x7fffffff,"PT_HIPROC");
//    mapResult.insert(0x70000000,"PT_MIPXELF_DEF::REGINFO");

    return mapResult;   
}

QMap<quint64, QString> XELF::getProgramTypesS()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0,"NULL");
    mapResult.insert(1,"LOAD");
    mapResult.insert(2,"DYNAMIC");
    mapResult.insert(3,"INTERP");
    mapResult.insert(4,"NOTE");
    mapResult.insert(5,"SHLIB");
    mapResult.insert(6,"PHDR");
    mapResult.insert(7,"TLS");
    mapResult.insert(8,"NUM");
    mapResult.insert(0x60000000,"LOOS");
    mapResult.insert(0x6464e550,"SUNW_UNWIND");
    mapResult.insert(0x6474e550,"GNU_EH_FRAME"); // SUNW_EH_FRAME
    mapResult.insert(0x6474e551,"GNU_STACK");
    mapResult.insert(0x6474e552,"GNU_RELRO");
    mapResult.insert(0x6ffffffa,"LOSUNW");
    mapResult.insert(0x6ffffffa,"SUNWBSS");
    mapResult.insert(0x6ffffffb,"SUNWSTACK");
    mapResult.insert(0x6ffffffc,"SUNWDTRACE");
    mapResult.insert(0x6ffffffd,"SUNWCAP");
    mapResult.insert(0x6fffffff,"HIOS"); // HISUNW
    mapResult.insert(0x70000000,"LOPROC");
    mapResult.insert(0x7fffffff,"HIPROC");
//    mapResult.insert(0x70000000,"MIPXELF_DEF::REGINFO");

    return mapResult;
}

QMap<quint64, QString> XELF::getProgramFlags()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0x00000001,"PF_X");
    mapResult.insert(0x00000002,"PF_W");
    mapResult.insert(0x00000004,"PF_R");
    mapResult.insert(0x0ff00000,"PF_MASKOS");
    mapResult.insert(0xf0000000,"PF_MASKPROC");

    return mapResult;
}

QMap<quint64, QString> XELF::getProgramFlagsS()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0x00000001,"X");
    mapResult.insert(0x00000002,"W");
    mapResult.insert(0x00000004,"R");
    mapResult.insert(0x0ff00000,"MASKOS");
    mapResult.insert(0xf0000000,"MASKPROC");

    return mapResult;
}

quint16 XELF::getSectionStringTable()
{
    return getSectionStringTable(is64());
}

quint16 XELF::getSectionStringTable(bool bIs64)
{
    quint32 nResult=0;

    if(bIs64)
    {
        nResult=getHdr64_shstrndx();
    }
    else
    {
        nResult=getHdr32_shstrndx();
    }

    return nResult;
}

XBinary::OFFSETSIZE XELF::getSectionOffsetSize(quint32 nSection)
{
    OFFSETSIZE result={};

    if(is64())
    {
        XELF_DEF::Elf64_Shdr section_header=getElf64_Shdr(nSection);
        result.nOffset=section_header.sh_offset;
        result.nSize=section_header.sh_size;
    }
    else
    {
        XELF_DEF::Elf32_Shdr section_header=getElf32_Shdr(nSection);
        result.nOffset=section_header.sh_offset;
        result.nSize=section_header.sh_size;
    }

    return result;
}

QString XELF::getStringFromIndex(qint64 nOffset, qint64 nSize, int nIndex)
{
    QString sResult;

    if(nIndex<nSize)
    {
        sResult=read_ansiString(nOffset+nIndex);
    }

    return sResult;
}

QMap<quint32, QString> XELF::getStringsFromSection(quint32 nSection)
{
    QMap<quint32, QString> mapResult;

    if(nSection!=XELF_DEF::S_SHN_UNDEF)
    {
        QByteArray section=getSection(nSection);
        int nSize=section.size();
        char *pOffset=section.data();
        quint32 nCount=0;

        while(nSize>0)
        {
            QString sString(pOffset+nCount);

            if(sString.length())
            {
                mapResult.insert(nCount,sString);
            }

            nCount+=(quint32)sString.length()+1;
            nSize-=sString.length()+1;
        }
    }

    return mapResult;
}

QString XELF::getStringFromSection(quint32 nIndex, quint32 nSection)
{
    QString sResult;

    XBinary::OFFSETSIZE os=getSectionOffsetSize(nSection);

    sResult=getStringFromIndex(os.nOffset,os.nSize,nIndex);

    return sResult;
}

QMap<quint32, QString> XELF::getStringsFromMainSection()
{
    quint32 nSection=getSectionStringTable();

    return getStringsFromSection(nSection);
}

QString XELF::getStringFromMainSection(quint32 nIndex)
{
    quint32 nSection=getSectionStringTable();

    return getStringFromSection(nIndex,nSection); // TODO optimize
}

QByteArray XELF::getSection(quint32 nIndex)
{
    qint64 nOffset=0;
    qint64 nSize=0;

    if(is64())
    {
        XELF_DEF::Elf64_Shdr section_header=getElf64_Shdr(nIndex);
        nSize=section_header.sh_size;
        nOffset=(isImage())?(section_header.sh_addr):(section_header.sh_offset);
    }
    else
    {
        XELF_DEF::Elf32_Shdr section_header=getElf32_Shdr(nIndex);
        nSize=section_header.sh_size;
        nOffset=(isImage())?(section_header.sh_addr):(section_header.sh_offset);
    }

    return read_array(nOffset,nSize);
}

bool XELF::isSectionValid(quint32 nIndex)
{
    quint32 nNumberOfSections=getHdr32_shnum();

    return (nIndex<nNumberOfSections)&&(nIndex!=(quint32)-1);
}

QList<XELF_DEF::Elf32_Shdr> XELF::getElf32_ShdrList()
{
    QList<XELF_DEF::Elf32_Shdr> listResult;

    quint32 nNumberOfSections=getHdr32_shnum();
    nNumberOfSections=qMin((quint32)1000,nNumberOfSections);

    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    for(quint32 i=0; i<nNumberOfSections; i++)
    {
        XELF_DEF::Elf32_Shdr record;

        record.sh_name=read_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_name),bIsBigEndian);
        record.sh_type=read_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_type),bIsBigEndian);
        record.sh_flags=read_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_flags),bIsBigEndian);
        record.sh_addr=read_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_addr),bIsBigEndian);
        record.sh_offset=read_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_offset),bIsBigEndian);
        record.sh_size=read_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_size),bIsBigEndian);
        record.sh_link=read_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_link),bIsBigEndian);
        record.sh_info=read_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_info),bIsBigEndian);
        record.sh_addralign=read_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_addralign),bIsBigEndian);
        record.sh_entsize=read_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_entsize),bIsBigEndian);

        listResult.append(record);

        offset+=sizeof(XELF_DEF::Elf32_Shdr);
    }

    return listResult;
}

QList<XELF_DEF::Elf64_Shdr> XELF::getElf64_ShdrList()
{
    QList<XELF_DEF::Elf64_Shdr> listResult;

    quint32 nNumberOfSections=getHdr64_shnum();
    nNumberOfSections=qMin((quint32)1000,nNumberOfSections);

    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    for(quint32 i=0; i<nNumberOfSections; i++)
    {
        XELF_DEF::Elf64_Shdr record={};

        record.sh_name=read_uint32(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_name),bIsBigEndian);
        record.sh_type=read_uint32(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_type),bIsBigEndian);
        record.sh_flags=read_uint64(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_flags),bIsBigEndian);
        record.sh_addr=read_uint64(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_addr),bIsBigEndian);
        record.sh_offset=read_uint64(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_offset),bIsBigEndian);
        record.sh_size=read_uint64(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_size),bIsBigEndian);
        record.sh_link=read_uint32(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_link),bIsBigEndian);
        record.sh_info=read_uint32(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_info),bIsBigEndian);
        record.sh_addralign=read_uint64(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_addralign),bIsBigEndian);
        record.sh_entsize=read_uint64(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_entsize),bIsBigEndian);

        listResult.append(record);

        offset+=sizeof(XELF_DEF::Elf64_Shdr);
    }

    return listResult;
}

QList<XELF_DEF::Elf_Shdr> XELF::getElf_ShdrList()
{
    QList<XELF_DEF::Elf_Shdr> listResult;

    bool bIs64=is64();

    if(bIs64)
    {
        QList<XELF_DEF::Elf64_Shdr> list=getElf64_ShdrList();
        int nCount=list.count();

        for(int i=0;i<nCount;i++)
        {
            XELF_DEF::Elf_Shdr record={};

            record.sh_name=list.at(i).sh_name;
            record.sh_type=list.at(i).sh_type;
            record.sh_flags=list.at(i).sh_flags;
            record.sh_addr=list.at(i).sh_addr;
            record.sh_offset=list.at(i).sh_offset;
            record.sh_size=list.at(i).sh_size;
            record.sh_link=list.at(i).sh_link;
            record.sh_info=list.at(i).sh_info;
            record.sh_addralign=list.at(i).sh_addralign;
            record.sh_entsize=list.at(i).sh_entsize;

            listResult.append(record);
        }
    }
    else
    {
        QList<XELF_DEF::Elf32_Shdr> list=getElf32_ShdrList();
        int nCount=list.count();

        for(int i=0;i<nCount;i++)
        {
            XELF_DEF::Elf_Shdr record={};

            record.sh_name=list.at(i).sh_name;
            record.sh_type=list.at(i).sh_type;
            record.sh_flags=list.at(i).sh_flags;
            record.sh_addr=list.at(i).sh_addr;
            record.sh_offset=list.at(i).sh_offset;
            record.sh_size=list.at(i).sh_size;
            record.sh_link=list.at(i).sh_link;
            record.sh_info=list.at(i).sh_info;
            record.sh_addralign=list.at(i).sh_addralign;
            record.sh_entsize=list.at(i).sh_entsize;

            listResult.append(record);
        }
    }

    return listResult;
}

XELF_DEF::Elf32_Shdr XELF::getElf32_Shdr(quint32 nIndex)
{
    XELF_DEF::Elf32_Shdr result={};

    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Shdr);
        result.sh_name=read_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_name),bIsBigEndian);
        result.sh_type=read_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_type),bIsBigEndian);
        result.sh_flags=read_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_flags),bIsBigEndian);
        result.sh_addr=read_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_addr),bIsBigEndian);
        result.sh_offset=read_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_offset),bIsBigEndian);
        result.sh_size=read_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_size),bIsBigEndian);
        result.sh_link=read_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_link),bIsBigEndian);
        result.sh_info=read_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_info),bIsBigEndian);
        result.sh_addralign=read_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_addralign),bIsBigEndian);
        result.sh_entsize=read_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_entsize),bIsBigEndian);
    }

    return result;
}

XELF_DEF::Elf64_Shdr XELF::getElf64_Shdr(quint32 nIndex)
{
    XELF_DEF::Elf64_Shdr result={};

    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Shdr);
        result.sh_name=read_uint32(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_name),bIsBigEndian);
        result.sh_type=read_uint32(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_type),bIsBigEndian);
        result.sh_flags=read_uint64(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_flags),bIsBigEndian);
        result.sh_addr=read_uint64(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_addr),bIsBigEndian);
        result.sh_offset=read_uint64(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_offset),bIsBigEndian);
        result.sh_size=read_uint64(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_size),bIsBigEndian);
        result.sh_link=read_uint32(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_link),bIsBigEndian);
        result.sh_info=read_uint32(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_info),bIsBigEndian);
        result.sh_addralign=read_uint64(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_addralign),bIsBigEndian);
        result.sh_entsize=read_uint64(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_entsize),bIsBigEndian);
    }

    return result;
}

quint32 XELF::getElf32_Shdr_name(quint32 nIndex)
{
    quint32 result=0;

    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Shdr);
        result=read_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_name),bIsBigEndian);
    }

    return result;
}

quint32 XELF::getElf32_Shdr_type(quint32 nIndex)
{
    quint32 result=0;

    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Shdr);
        result=read_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_type),bIsBigEndian);
    }

    return result;
}

quint32 XELF::getElf32_Shdr_flags(quint32 nIndex)
{
    quint32 result=0;

    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Shdr);
        result=read_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_flags),bIsBigEndian);
    }

    return result;
}

quint32 XELF::getElf32_Shdr_addr(quint32 nIndex)
{
    quint32 result=0;

    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Shdr);
        result=read_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_addr),bIsBigEndian);
    }

    return result;
}

quint32 XELF::getElf32_Shdr_offset(quint32 nIndex)
{
    quint32 result=0;

    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Shdr);
        result=read_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_offset),bIsBigEndian);
    }

    return result;
}

quint32 XELF::getElf32_Shdr_size(quint32 nIndex)
{
    quint32 result=0;

    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Shdr);
        result=read_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_size),bIsBigEndian);
    }

    return result;
}

quint32 XELF::getElf32_Shdr_link(quint32 nIndex)
{
    quint32 result=0;

    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Shdr);
        result=read_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_link),bIsBigEndian);
    }

    return result;
}

quint32 XELF::getElf32_Shdr_info(quint32 nIndex)
{
    quint32 result=0;

    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Shdr);
        result=read_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_info),bIsBigEndian);
    }

    return result;
}

quint32 XELF::getElf32_Shdr_addralign(quint32 nIndex)
{
    quint32 result=0;

    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Shdr);
        result=read_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_addralign),bIsBigEndian);
    }

    return result;
}

quint32 XELF::getElf32_Shdr_entsize(quint32 nIndex)
{
    quint32 result=0;

    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Shdr);
        result=read_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_entsize),bIsBigEndian);
    }

    return result;
}

void XELF::setElf32_Shdr_name(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Shdr);
        write_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_name),value,bIsBigEndian);
    }
}

void XELF::setElf32_Shdr_type(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Shdr);
        write_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_type),value,bIsBigEndian);
    }
}

void XELF::setElf32_Shdr_flags(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Shdr);
        write_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_flags),value,bIsBigEndian);
    }
}

void XELF::setElf32_Shdr_addr(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Shdr);
        write_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_addr),value,bIsBigEndian);
    }
}

void XELF::setElf32_Shdr_offset(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Shdr);
        write_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_offset),value,bIsBigEndian);
    }
}

void XELF::setElf32_Shdr_size(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Shdr);
        write_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_size),value,bIsBigEndian);
    }
}

void XELF::setElf32_Shdr_link(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Shdr);
        write_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_link),value,bIsBigEndian);
    }
}

void XELF::setElf32_Shdr_info(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Shdr);
        write_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_info),value,bIsBigEndian);
    }
}

void XELF::setElf32_Shdr_addralign(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Shdr);
        write_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_addralign),value,bIsBigEndian);
    }
}

void XELF::setElf32_Shdr_entsize(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfSections=getHdr32_shnum();
    quint32 offset=getHdr32_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Shdr);
        write_uint32(offset+offsetof(XELF_DEF::Elf32_Shdr,sh_entsize),value,bIsBigEndian);
    }
}

quint32 XELF::getElf64_Shdr_name(quint32 nIndex)
{
    quint32 result=0;

    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Shdr);
        result=read_uint32(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_name),bIsBigEndian);
    }

    return result;
}

quint32 XELF::getElf64_Shdr_type(quint32 nIndex)
{
    quint32 result=0;

    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Shdr);
        result=read_uint32(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_type),bIsBigEndian);
    }

    return result;
}

quint64 XELF::getElf64_Shdr_flags(quint32 nIndex)
{
    quint64 result=0;

    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Shdr);
        result=read_uint64(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_flags),bIsBigEndian);
    }

    return result;
}

quint64 XELF::getElf64_Shdr_addr(quint32 nIndex)
{
    quint64 result=0;

    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Shdr);
        result=read_uint64(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_addr),bIsBigEndian);
    }

    return result;
}

quint64 XELF::getElf64_Shdr_offset(quint32 nIndex)
{
    quint64 result=0;

    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Shdr);
        result=read_uint64(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_offset),bIsBigEndian);
    }

    return result;
}

quint64 XELF::getElf64_Shdr_size(quint32 nIndex)
{
    quint64 result=0;

    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Shdr);
        result=read_uint64(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_size),bIsBigEndian);
    }

    return result;
}

quint32 XELF::getElf64_Shdr_link(quint32 nIndex)
{
    quint32 result=0;

    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Shdr);
        result=read_uint32(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_link),bIsBigEndian);
    }

    return result;
}

quint32 XELF::getElf64_Shdr_info(quint32 nIndex)
{
    quint32 result=0;

    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Shdr);
        result=read_uint32(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_info),bIsBigEndian);
    }

    return result;
}

quint64 XELF::getElf64_Shdr_addralign(quint32 nIndex)
{
    quint64 result=0;

    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Shdr);
        result=read_uint64(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_addralign),bIsBigEndian);
    }

    return result;
}

quint64 XELF::getElf64_Shdr_entsize(quint32 nIndex)
{
    quint64 result=0;

    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Shdr);
        result=read_uint64(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_entsize),bIsBigEndian);
    }

    return result;
}

void XELF::setElf64_Shdr_name(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Shdr);
        write_uint32(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_name),value,bIsBigEndian);
    }
}

void XELF::setElf64_Shdr_type(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Shdr);
        write_uint32(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_type),value,bIsBigEndian);
    }
}

void XELF::setElf64_Shdr_flags(quint32 nIndex, quint64 value)
{
    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Shdr);
        write_uint64(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_flags),value,bIsBigEndian);
    }
}

void XELF::setElf64_Shdr_addr(quint32 nIndex, quint64 value)
{
    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Shdr);
        write_uint64(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_addr),value,bIsBigEndian);
    }
}

void XELF::setElf64_Shdr_offset(quint32 nIndex, quint64 value)
{
    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Shdr);
        write_uint64(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_offset),value,bIsBigEndian);
    }
}

void XELF::setElf64_Shdr_size(quint32 nIndex, quint64 value)
{
    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Shdr);
        write_uint64(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_size),value,bIsBigEndian);
    }
}

void XELF::setElf64_Shdr_link(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Shdr);
        write_uint32(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_link),value,bIsBigEndian);
    }
}

void XELF::setElf64_Shdr_info(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Shdr);
        write_uint32(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_info),value,bIsBigEndian);
    }
}

void XELF::setElf64_Shdr_addralign(quint32 nIndex, quint64 value)
{
    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Shdr);
        write_uint64(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_addralign),value,bIsBigEndian);
    }
}

void XELF::setElf64_Shdr_entsize(quint32 nIndex, quint64 value)
{
    quint32 nNumberOfSections=getHdr64_shnum();
    quint64 offset=getHdr64_shoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfSections)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Shdr);
        write_uint64(offset+offsetof(XELF_DEF::Elf64_Shdr,sh_entsize),value,bIsBigEndian);
    }
}

quint32 XELF::getElf_Shdr_name(quint32 nIndex, QList<XELF_DEF::Elf_Shdr> *pList)
{
    quint32 nResult=0;

    if(nIndex<(quint32)pList->count())
    {
        nResult=pList->at(nIndex).sh_name;
    }

    return nResult;
}

quint32 XELF::getElf_Shdr_type(quint32 nIndex, QList<XELF_DEF::Elf_Shdr> *pList)
{
    quint32 nResult=0;

    if(nIndex<(quint32)pList->count())
    {
        nResult=pList->at(nIndex).sh_type;
    }

    return nResult;
}

quint64 XELF::getElf_Shdr_flags(quint32 nIndex, QList<XELF_DEF::Elf_Shdr> *pList)
{
    quint64 nResult=0;

    if(nIndex<(quint32)pList->count())
    {
        nResult=pList->at(nIndex).sh_flags;
    }

    return nResult;
}

quint64 XELF::getElf_Shdr_addr(quint32 nIndex, QList<XELF_DEF::Elf_Shdr> *pList)
{
    quint64 nResult=0;

    if(nIndex<(quint32)pList->count())
    {
        nResult=pList->at(nIndex).sh_addr;
    }

    return nResult;
}

quint64 XELF::getElf_Shdr_offset(quint32 nIndex, QList<XELF_DEF::Elf_Shdr> *pList)
{
    quint64 nResult=0;

    if(nIndex<(quint32)pList->count())
    {
        nResult=pList->at(nIndex).sh_offset;
    }

    return nResult;
}

quint64 XELF::getElf_Shdr_size(quint32 nIndex, QList<XELF_DEF::Elf_Shdr> *pList)
{
    quint64 nResult=0;

    if(nIndex<(quint32)pList->count())
    {
        nResult=pList->at(nIndex).sh_size;
    }

    return nResult;
}

quint32 XELF::getElf_Shdr_link(quint32 nIndex, QList<XELF_DEF::Elf_Shdr> *pList)
{
    quint32 nResult=0;

    if(nIndex<(quint32)pList->count())
    {
        nResult=pList->at(nIndex).sh_link;
    }

    return nResult;
}

quint32 XELF::getElf_Shdr_info(quint32 nIndex, QList<XELF_DEF::Elf_Shdr> *pList)
{
    quint32 nResult=0;

    if(nIndex<(quint32)pList->count())
    {
        nResult=pList->at(nIndex).sh_info;
    }

    return nResult;
}

quint64 XELF::getElf_Shdr_addralign(quint32 nIndex, QList<XELF_DEF::Elf_Shdr> *pList)
{
    quint64 nResult=0;

    if(nIndex<(quint32)pList->count())
    {
        nResult=pList->at(nIndex).sh_addralign;
    }

    return nResult;
}

quint64 XELF::getElf_Shdr_entsize(quint32 nIndex, QList<XELF_DEF::Elf_Shdr> *pList)
{
    quint32 nResult=0;

    if(nIndex<(quint32)pList->count())
    {
        nResult=pList->at(nIndex).sh_entsize;
    }

    return nResult;
}

qint64 XELF::getShdrOffset(quint32 nIndex)
{
    quint64 nResult=-1;

    bool bIs64=is64();
    quint32 nNumberOfSections=0;

    if(bIs64)
    {
        nNumberOfSections=getHdr64_shnum();

        if(nIndex<nNumberOfSections)
        {
            nResult=getHdr64_shoff()+nIndex*sizeof(XELF_DEF::Elf64_Shdr);
        }
    }
    else
    {
        nNumberOfSections=getHdr32_shnum();

        if(nIndex<nNumberOfSections)
        {
            nResult=getHdr32_shoff()+nIndex*sizeof(XELF_DEF::Elf32_Shdr);
        }
    }

    return nResult;
}

qint64 XELF::getShdrSize()
{
    quint64 nResult=0;

    bool bIs64=is64();

    if(bIs64)
    {
        nResult=sizeof(XELF_DEF::Elf64_Shdr);
    }
    else
    {
        nResult=sizeof(XELF_DEF::Elf32_Shdr);
    }

    return nResult;
}

QList<XELF_DEF::Elf32_Phdr> XELF::getElf32_PhdrList()
{
    QList<XELF_DEF::Elf32_Phdr> result;

    quint32 nNumberOfProgramms=getHdr32_phnum();
    nNumberOfProgramms=qMin((quint32)1000,nNumberOfProgramms);
    quint32 offset=getHdr32_phoff();
    bool bIsBigEndian=isBigEndian();

    for(quint32 i=0; i<nNumberOfProgramms; i++)
    {
        XELF_DEF::Elf32_Phdr record={};

        record.p_type=read_uint32(offset+offsetof(XELF_DEF::Elf32_Phdr,p_type),bIsBigEndian);
        record.p_offset=read_uint32(offset+offsetof(XELF_DEF::Elf32_Phdr,p_offset),bIsBigEndian);
        record.p_vaddr=read_uint32(offset+offsetof(XELF_DEF::Elf32_Phdr,p_vaddr),bIsBigEndian);
        record.p_paddr=read_uint32(offset+offsetof(XELF_DEF::Elf32_Phdr,p_paddr),bIsBigEndian);
        record.p_filesz=read_uint32(offset+offsetof(XELF_DEF::Elf32_Phdr,p_filesz),bIsBigEndian);
        record.p_memsz=read_uint32(offset+offsetof(XELF_DEF::Elf32_Phdr,p_memsz),bIsBigEndian);
        record.p_flags=read_uint32(offset+offsetof(XELF_DEF::Elf32_Phdr,p_flags),bIsBigEndian);
        record.p_align=read_uint32(offset+offsetof(XELF_DEF::Elf32_Phdr,p_align),bIsBigEndian);

        result.append(record);

        offset+=sizeof(XELF_DEF::Elf32_Phdr);
    }

    return result;
}

QList<XELF_DEF::Elf64_Phdr> XELF::getElf64_PhdrList()
{
    QList<XELF_DEF::Elf64_Phdr> result;

    quint32 nNumberOfProgramms=getHdr64_phnum();
    nNumberOfProgramms=qMin((quint32)1000,nNumberOfProgramms);
    quint64 offset=getHdr64_phoff();
    bool bIsBigEndian=isBigEndian();

    for(quint32 i=0; i<nNumberOfProgramms; i++)
    {
        XELF_DEF::Elf64_Phdr record={};

        record.p_type=read_uint32(offset+offsetof(XELF_DEF::Elf64_Phdr,p_type),bIsBigEndian);
        record.p_offset=read_uint64(offset+offsetof(XELF_DEF::Elf64_Phdr,p_offset),bIsBigEndian);
        record.p_vaddr=read_uint64(offset+offsetof(XELF_DEF::Elf64_Phdr,p_vaddr),bIsBigEndian);
        record.p_paddr=read_uint64(offset+offsetof(XELF_DEF::Elf64_Phdr,p_paddr),bIsBigEndian);
        record.p_filesz=read_uint64(offset+offsetof(XELF_DEF::Elf64_Phdr,p_filesz),bIsBigEndian);
        record.p_memsz=read_uint64(offset+offsetof(XELF_DEF::Elf64_Phdr,p_memsz),bIsBigEndian);
        record.p_flags=read_uint32(offset+offsetof(XELF_DEF::Elf64_Phdr,p_flags),bIsBigEndian);
        record.p_align=read_uint64(offset+offsetof(XELF_DEF::Elf64_Phdr,p_align),bIsBigEndian);

        result.append(record);

        offset+=sizeof(XELF_DEF::Elf64_Phdr);
    }

    return result;
}

QList<XELF_DEF::Elf_Phdr> XELF::getElf_PhdrList()
{
    QList<XELF_DEF::Elf_Phdr> listResult;

    bool bIs64=is64();

    if(bIs64)
    {
        QList<XELF_DEF::Elf64_Phdr> list=getElf64_PhdrList();
        int nCount=list.count();

        for(int i=0;i<nCount;i++)
        {
            XELF_DEF::Elf_Phdr record={};

            record.p_type=list.at(i).p_type;
            record.p_flags=list.at(i).p_flags;
            record.p_offset=list.at(i).p_offset;
            record.p_vaddr=list.at(i).p_vaddr;
            record.p_paddr=list.at(i).p_paddr;
            record.p_filesz=list.at(i).p_filesz;
            record.p_memsz=list.at(i).p_memsz;
            record.p_align=list.at(i).p_align;

            listResult.append(record);
        }
    }
    else
    {
        QList<XELF_DEF::Elf32_Phdr> list=getElf32_PhdrList();
        int nCount=list.count();

        for(int i=0;i<nCount;i++)
        {
            XELF_DEF::Elf_Phdr record={};

            record.p_type=list.at(i).p_type;
            record.p_flags=list.at(i).p_flags;
            record.p_offset=list.at(i).p_offset;
            record.p_vaddr=list.at(i).p_vaddr;
            record.p_paddr=list.at(i).p_paddr;
            record.p_filesz=list.at(i).p_filesz;
            record.p_memsz=list.at(i).p_memsz;
            record.p_align=list.at(i).p_align;

            listResult.append(record);
        }
    }

    return listResult;
}

XELF_DEF::Elf32_Phdr XELF::getElf32_Phdr(quint32 nIndex)
{
    XELF_DEF::Elf32_Phdr result={};

    quint32 nNumberOfPrograms=getHdr32_phnum();
    quint32 offset=getHdr32_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Phdr);
        result.p_type=read_uint32(offset+offsetof(XELF_DEF::Elf32_Phdr,p_type),bIsBigEndian);
        result.p_offset=read_uint32(offset+offsetof(XELF_DEF::Elf32_Phdr,p_offset),bIsBigEndian);
        result.p_vaddr=read_uint32(offset+offsetof(XELF_DEF::Elf32_Phdr,p_vaddr),bIsBigEndian);
        result.p_paddr=read_uint32(offset+offsetof(XELF_DEF::Elf32_Phdr,p_paddr),bIsBigEndian);
        result.p_filesz=read_uint32(offset+offsetof(XELF_DEF::Elf32_Phdr,p_filesz),bIsBigEndian);
        result.p_memsz=read_uint32(offset+offsetof(XELF_DEF::Elf32_Phdr,p_memsz),bIsBigEndian);
        result.p_flags=read_uint32(offset+offsetof(XELF_DEF::Elf32_Phdr,p_flags),bIsBigEndian);
        result.p_align=read_uint32(offset+offsetof(XELF_DEF::Elf32_Phdr,p_align),bIsBigEndian);
    }

    return result;
}

XELF_DEF::Elf64_Phdr XELF::getElf64_Phdr(quint32 nIndex)
{
    XELF_DEF::Elf64_Phdr result={};

    quint32 nNumberOfPrograms=getHdr64_phnum();
    quint64 offset=getHdr64_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Phdr);
        result.p_type=read_uint32(offset+offsetof(XELF_DEF::Elf64_Phdr,p_type),bIsBigEndian);
        result.p_offset=read_uint64(offset+offsetof(XELF_DEF::Elf64_Phdr,p_offset),bIsBigEndian);
        result.p_vaddr=read_uint64(offset+offsetof(XELF_DEF::Elf64_Phdr,p_vaddr),bIsBigEndian);
        result.p_paddr=read_uint64(offset+offsetof(XELF_DEF::Elf64_Phdr,p_paddr),bIsBigEndian);
        result.p_filesz=read_uint64(offset+offsetof(XELF_DEF::Elf64_Phdr,p_filesz),bIsBigEndian);
        result.p_memsz=read_uint64(offset+offsetof(XELF_DEF::Elf64_Phdr,p_memsz),bIsBigEndian);
        result.p_flags=read_uint32(offset+offsetof(XELF_DEF::Elf64_Phdr,p_flags),bIsBigEndian);
        result.p_align=read_uint64(offset+offsetof(XELF_DEF::Elf64_Phdr,p_align),bIsBigEndian);
    }

    return result;
}

quint32 XELF::getElf32_Phdr_type(quint32 nIndex)
{
    quint32 result=0;

    quint32 nNumberOfPrograms=getHdr32_phnum();
    quint32 offset=getHdr32_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Phdr);
        result=read_uint32(offset+offsetof(XELF_DEF::Elf32_Phdr,p_type),bIsBigEndian);
    }

    return result;
}

quint32 XELF::getElf32_Phdr_offset(quint32 nIndex)
{
    quint32 result=0;

    quint32 nNumberOfPrograms=getHdr32_phnum();
    quint32 offset=getHdr32_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Phdr);
        result=read_uint32(offset+offsetof(XELF_DEF::Elf32_Phdr,p_offset),bIsBigEndian);
    }

    return result;
}

quint32 XELF::getElf32_Phdr_vaddr(quint32 nIndex)
{
    quint32 result=0;

    quint32 nNumberOfPrograms=getHdr32_phnum();
    quint32 offset=getHdr32_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Phdr);
        result=read_uint32(offset+offsetof(XELF_DEF::Elf32_Phdr,p_vaddr),bIsBigEndian);
    }

    return result;
}

quint32 XELF::getElf32_Phdr_paddr(quint32 nIndex)
{
    quint32 result=0;

    quint32 nNumberOfPrograms=getHdr32_phnum();
    quint32 offset=getHdr32_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Phdr);
        result=read_uint32(offset+offsetof(XELF_DEF::Elf32_Phdr,p_paddr),bIsBigEndian);
    }

    return result;
}

quint32 XELF::getElf32_Phdr_filesz(quint32 nIndex)
{
    quint32 result=0;

    quint32 nNumberOfPrograms=getHdr32_phnum();
    quint32 offset=getHdr32_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Phdr);
        result=read_uint32(offset+offsetof(XELF_DEF::Elf32_Phdr,p_filesz),bIsBigEndian);
    }

    return result;
}

quint32 XELF::getElf32_Phdr_memsz(quint32 nIndex)
{
    quint32 result=0;

    quint32 nNumberOfPrograms=getHdr32_phnum();
    quint32 offset=getHdr32_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Phdr);
        result=read_uint32(offset+offsetof(XELF_DEF::Elf32_Phdr,p_memsz),bIsBigEndian);
    }

    return result;
}

quint32 XELF::getElf32_Phdr_flags(quint32 nIndex)
{
    quint32 result=0;

    quint32 nNumberOfPrograms=getHdr32_phnum();
    quint32 offset=getHdr32_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Phdr);
        result=read_uint32(offset+offsetof(XELF_DEF::Elf32_Phdr,p_flags),bIsBigEndian);
    }

    return result;
}

quint32 XELF::getElf32_Phdr_align(quint32 nIndex)
{
    quint32 result=0;

    quint32 nNumberOfPrograms=getHdr32_phnum();
    quint32 offset=getHdr32_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Phdr);
        result=read_uint32(offset+offsetof(XELF_DEF::Elf32_Phdr,p_align),bIsBigEndian);
    }

    return result;
}

void XELF::setElf32_Phdr_type(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfPrograms=getHdr32_phnum();
    quint32 offset=getHdr32_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Phdr);
        write_uint32(offset+offsetof(XELF_DEF::Elf32_Phdr,p_type),value,bIsBigEndian);
    }
}

void XELF::setElf32_Phdr_offset(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfPrograms=getHdr32_phnum();
    quint32 offset=getHdr32_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Phdr);
        write_uint32(offset+offsetof(XELF_DEF::Elf32_Phdr,p_offset),value,bIsBigEndian);
    }
}

void XELF::setElf32_Phdr_vaddr(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfPrograms=getHdr32_phnum();
    quint32 offset=getHdr32_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Phdr);
        write_uint32(offset+offsetof(XELF_DEF::Elf32_Phdr,p_vaddr),value,bIsBigEndian);
    }
}

void XELF::setElf32_Phdr_paddr(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfPrograms=getHdr32_phnum();
    quint32 offset=getHdr32_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Phdr);
        write_uint32(offset+offsetof(XELF_DEF::Elf32_Phdr,p_paddr),value,bIsBigEndian);
    }
}

void XELF::setElf32_Phdr_filesz(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfPrograms=getHdr32_phnum();
    quint32 offset=getHdr32_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Phdr);
        write_uint32(offset+offsetof(XELF_DEF::Elf32_Phdr,p_filesz),value,bIsBigEndian);
    }
}

void XELF::setElf32_Phdr_memsz(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfPrograms=getHdr32_phnum();
    quint32 offset=getHdr32_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Phdr);
        write_uint32(offset+offsetof(XELF_DEF::Elf32_Phdr,p_memsz),value,bIsBigEndian);
    }
}

void XELF::setElf32_Phdr_flags(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfPrograms=getHdr32_phnum();
    quint32 offset=getHdr32_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Phdr);
        write_uint32(offset+offsetof(XELF_DEF::Elf32_Phdr,p_flags),value,bIsBigEndian);
    }
}

void XELF::setElf32_Phdr_align(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfPrograms=getHdr32_phnum();
    quint32 offset=getHdr32_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf32_Phdr);
        write_uint32(offset+offsetof(XELF_DEF::Elf32_Phdr,p_align),value,bIsBigEndian);
    }
}

quint32 XELF::getElf64_Phdr_type(quint32 nIndex)
{
    quint32 result=0;

    quint32 nNumberOfPrograms=getHdr64_phnum();
    quint64 offset=getHdr64_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Phdr);
        result=read_uint32(offset+offsetof(XELF_DEF::Elf64_Phdr,p_type),bIsBigEndian);
    }

    return result;
}

quint64 XELF::getElf64_Phdr_offset(quint32 nIndex)
{
    quint64 result=0;

    quint32 nNumberOfPrograms=getHdr64_phnum();
    quint64 offset=getHdr64_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Phdr);
        result=read_uint64(offset+offsetof(XELF_DEF::Elf64_Phdr,p_offset),bIsBigEndian);
    }

    return result;
}

quint64 XELF::getElf64_Phdr_vaddr(quint32 nIndex)
{
    quint64 result=0;

    quint32 nNumberOfPrograms=getHdr64_phnum();
    quint64 offset=getHdr64_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Phdr);
        result=read_uint64(offset+offsetof(XELF_DEF::Elf64_Phdr,p_vaddr),bIsBigEndian);
    }

    return result;
}

quint64 XELF::getElf64_Phdr_paddr(quint32 nIndex)
{
    quint64 result=0;

    quint32 nNumberOfPrograms=getHdr64_phnum();
    quint64 offset=getHdr64_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Phdr);
        result=read_uint64(offset+offsetof(XELF_DEF::Elf64_Phdr,p_paddr),bIsBigEndian);
    }

    return result;
}

quint64 XELF::getElf64_Phdr_filesz(quint32 nIndex)
{
    quint64 result=0;

    quint32 nNumberOfPrograms=getHdr64_phnum();
    quint64 offset=getHdr64_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Phdr);
        result=read_uint64(offset+offsetof(XELF_DEF::Elf64_Phdr,p_filesz),bIsBigEndian);
    }

    return result;
}

quint64 XELF::getElf64_Phdr_memsz(quint32 nIndex)
{
    quint64 result=0;

    quint32 nNumberOfPrograms=getHdr64_phnum();
    quint64 offset=getHdr64_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Phdr);
        result=read_uint64(offset+offsetof(XELF_DEF::Elf64_Phdr,p_memsz),bIsBigEndian);
    }

    return result;
}

quint32 XELF::getElf64_Phdr_flags(quint32 nIndex)
{
    quint32 result=0;

    quint32 nNumberOfPrograms=getHdr64_phnum();
    quint64 offset=getHdr64_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Phdr);
        result=read_uint32(offset+offsetof(XELF_DEF::Elf64_Phdr,p_flags),bIsBigEndian);
    }

    return result;
}

quint64 XELF::getElf64_Phdr_align(quint32 nIndex)
{
    quint64 result=0;

    quint32 nNumberOfPrograms=getHdr64_phnum();
    quint64 offset=getHdr64_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Phdr);
        result=read_uint64(offset+offsetof(XELF_DEF::Elf64_Phdr,p_align),bIsBigEndian);
    }

    return result;
}

void XELF::setElf64_Phdr_type(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfPrograms=getHdr64_phnum();
    quint64 offset=getHdr64_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Phdr);
        write_uint32(offset+offsetof(XELF_DEF::Elf64_Phdr,p_type),value,bIsBigEndian);
    }
}

void XELF::setElf64_Phdr_offset(quint32 nIndex, quint64 value)
{
    quint32 nNumberOfPrograms=getHdr64_phnum();
    quint64 offset=getHdr64_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Phdr);
        write_uint64(offset+offsetof(XELF_DEF::Elf64_Phdr,p_offset),value,bIsBigEndian);
    }
}

void XELF::setElf64_Phdr_vaddr(quint32 nIndex, quint64 value)
{
    quint32 nNumberOfPrograms=getHdr64_phnum();
    quint64 offset=getHdr64_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Phdr);
        write_uint64(offset+offsetof(XELF_DEF::Elf64_Phdr,p_vaddr),value,bIsBigEndian);
    }
}

void XELF::setElf64_Phdr_paddr(quint32 nIndex, quint64 value)
{
    quint32 nNumberOfPrograms=getHdr64_phnum();
    quint64 offset=getHdr64_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Phdr);
        write_uint64(offset+offsetof(XELF_DEF::Elf64_Phdr,p_paddr),value,bIsBigEndian);
    }
}

void XELF::setElf64_Phdr_filesz(quint32 nIndex, quint64 value)
{
    quint32 nNumberOfPrograms=getHdr64_phnum();
    quint64 offset=getHdr64_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Phdr);
        write_uint64(offset+offsetof(XELF_DEF::Elf64_Phdr,p_filesz),value,bIsBigEndian);
    }
}

void XELF::setElf64_Phdr_memsz(quint32 nIndex, quint64 value)
{
    quint32 nNumberOfPrograms=getHdr64_phnum();
    quint64 offset=getHdr64_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Phdr);
        write_uint64(offset+offsetof(XELF_DEF::Elf64_Phdr,p_memsz),value,bIsBigEndian);
    }
}

void XELF::setElf64_Phdr_flags(quint32 nIndex, quint32 value)
{
    quint32 nNumberOfPrograms=getHdr64_phnum();
    quint64 offset=getHdr64_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Phdr);
        write_uint32(offset+offsetof(XELF_DEF::Elf64_Phdr,p_flags),value,bIsBigEndian);
    }
}

void XELF::setElf64_Phdr_align(quint32 nIndex, quint64 value)
{
    quint32 nNumberOfPrograms=getHdr64_phnum();
    quint64 offset=getHdr64_phoff();
    bool bIsBigEndian=isBigEndian();

    if(nIndex<nNumberOfPrograms)
    {
        offset+=nIndex*sizeof(XELF_DEF::Elf64_Phdr);
        write_uint64(offset+offsetof(XELF_DEF::Elf64_Phdr,p_align),value,bIsBigEndian);
    }
}

quint32 XELF::getElf_Phdr_type(quint32 nIndex, QList<XELF_DEF::Elf_Phdr> *pList)
{
    quint32 nResult=0;

    if(nIndex<(quint32)pList->count())
    {
        nResult=pList->at(nIndex).p_type;
    }

    return nResult;
}

quint64 XELF::getElf_Phdr_offset(quint32 nIndex, QList<XELF_DEF::Elf_Phdr> *pList)
{
    quint64 nResult=0;

    if(nIndex<(quint32)pList->count())
    {
        nResult=pList->at(nIndex).p_offset;
    }

    return nResult;
}

quint64 XELF::getElf_Phdr_vaddr(quint32 nIndex, QList<XELF_DEF::Elf_Phdr> *pList)
{
    quint64 nResult=0;

    if(nIndex<(quint32)pList->count())
    {
        nResult=pList->at(nIndex).p_vaddr;
    }

    return nResult;
}

quint64 XELF::getElf_Phdr_paddr(quint32 nIndex, QList<XELF_DEF::Elf_Phdr> *pList)
{
    quint64 nResult=0;

    if(nIndex<(quint32)pList->count())
    {
        nResult=pList->at(nIndex).p_paddr;
    }

    return nResult;
}

quint64 XELF::getElf_Phdr_filesz(quint32 nIndex, QList<XELF_DEF::Elf_Phdr> *pList)
{
    quint64 nResult=0;

    if(nIndex<(quint32)pList->count())
    {
        nResult=pList->at(nIndex).p_filesz;
    }

    return nResult;
}

quint64 XELF::getElf_Phdr_memsz(quint32 nIndex, QList<XELF_DEF::Elf_Phdr> *pList)
{
    quint64 nResult=0;

    if(nIndex<(quint32)pList->count())
    {
        nResult=pList->at(nIndex).p_memsz;
    }

    return nResult;
}

quint32 XELF::getElf_Phdr_flags(quint32 nIndex, QList<XELF_DEF::Elf_Phdr> *pList)
{
    quint32 nResult=0;

    if(nIndex<(quint32)pList->count())
    {
        nResult=pList->at(nIndex).p_flags;
    }

    return nResult;
}

quint64 XELF::getElf_Phdr_align(quint32 nIndex, QList<XELF_DEF::Elf_Phdr> *pList)
{
    quint64 nResult=0;

    if(nIndex<(quint32)pList->count())
    {
        nResult=pList->at(nIndex).p_align;
    }

    return nResult;
}

qint64 XELF::getPhdrOffset(quint32 nIndex)
{
    quint64 nResult=-1;

    bool bIs64=is64();
    quint32 nNumberOfPrograms=0;

    if(bIs64)
    {
        nNumberOfPrograms=getHdr64_phnum();

        if(nIndex<nNumberOfPrograms)
        {
            nResult=getHdr64_phoff()+nIndex*sizeof(XELF_DEF::Elf64_Phdr);
        }
    }
    else
    {
        nNumberOfPrograms=getHdr32_phnum();

        if(nIndex<nNumberOfPrograms)
        {
            nResult=getHdr32_phoff()+nIndex*sizeof(XELF_DEF::Elf32_Phdr);
        }
    }

    return nResult;
}

qint64 XELF::getPhdrSize()
{
    quint64 nResult=0;

    bool bIs64=is64();

    if(bIs64)
    {
        nResult=sizeof(XELF_DEF::Elf64_Phdr);
    }
    else
    {
        nResult=sizeof(XELF_DEF::Elf32_Phdr);
    }

    return nResult;
}

int XELF::getSectionIndexByName(QString sSectionName)
{
    int nResult=-1;

    if(is64())
    {
        for(quint32 i=0; i<getHdr64_shnum(); i++)
        {
            quint32 nCurrentNameIndex=getElf64_Shdr_name(i);

            if(getStringFromMainSection(nCurrentNameIndex)==sSectionName)
            {
                nResult=i;
                break;
            }
        }
    }
    else
    {
        for(quint32 i=0; i<getHdr32_shnum(); i++)
        {
            quint32 nCurrentNameIndex=getElf32_Shdr_name(i);

            if(getStringFromMainSection(nCurrentNameIndex)==sSectionName)
            {
                nResult=i;
                break;
            }
        }
    }

    return nResult;
}

QByteArray XELF::getSectionByName(QString sSectionName)
{
    QByteArray baResult;

    quint32 nIndex=getSectionIndexByName(sSectionName);

    if(nIndex!=(quint32)-1)
    {
        baResult=getSection(nIndex);
    }

    return baResult;
}

XBinary::OS_ANSISTRING XELF::getProgramInterpreterName()
{
    QList<XELF_DEF::Elf_Phdr> _listPhdr=getElf_PhdrList();

    return getProgramInterpreterName(&_listPhdr);
}

XBinary::OS_ANSISTRING XELF::getProgramInterpreterName(QList<XELF_DEF::Elf_Phdr> *pPhdrList)
{
    OS_ANSISTRING result={};

    QList<XELF_DEF::Elf_Phdr> listInterps=_getPrograms(pPhdrList,3); // TODO const

    if(listInterps.count())
    {
        result=getOsAnsiString(listInterps.at(0).p_offset,listInterps.at(0).p_filesz);
    }

    return result;
}

QString XELF::getCommentString()
{
    QString sResult;

    sResult.append(getSectionByName(".comment"));

    return sResult;
}

QList<XELF::NOTE> XELF::getNotes()
{
    QList<XELF_DEF::Elf_Phdr> _listPhdr=getElf_PhdrList();

    return getNotes(&_listPhdr);
}

//QString XELF::getCompatibleKernelVersion()
//{
//    QString sResult;

//    QByteArray baData=getSectionByName(".note.ABI-tag");
//    bool bIsBigEndian=isBigEndian();
//    NOTE note=getNote(baData,bIsBigEndian);

//    if((note.nType==1)&&(note.name=="GNU"))
//    {
//        quint32 kv[4];
//        kv[0]=_read_uint32(note.desc.data()+0,bIsBigEndian);
//        kv[1]=_read_uint32(note.desc.data()+4,bIsBigEndian);
//        kv[2]=_read_uint32(note.desc.data()+8,bIsBigEndian);
//        kv[3]=_read_uint32(note.desc.data()+12,bIsBigEndian);

//        if(kv[0]==0)
//        {
//            sResult=QString("%1.%2.%3").arg(kv[1]).arg(kv[2]).arg(kv[3]);
//        }
//    }

//    return sResult;
//}

QList<XELF::NOTE> XELF::getNotes(QList<XELF_DEF::Elf_Phdr> *pPhdrList)
{
    QList<XELF::NOTE> listResult;

    QList<XELF_DEF::Elf_Phdr> listNotes=_getPrograms(pPhdrList,4); // TODO const

    bool bIsBigEndian=isBigEndian();

    int nCount=listNotes.count();

    for(int i=0;i<nCount;i++)
    {
        qint64 nOffset=listNotes.at(i).p_offset;
        qint64 nSize=listNotes.at(i).p_filesz;

        listResult=_getNotes(nOffset,nSize,bIsBigEndian);
    }

    return listResult;
}

QList<XELF::NOTE> XELF::_getNotes(qint64 nOffset, qint64 nSize, bool bIsBigEndian)
{
    QList<XELF::NOTE> listResult;

    while(nSize>0)
    {
        NOTE note=_readNote(nOffset,nSize,bIsBigEndian);

        if(note.nSize)
        {
            listResult.append(note);

            nOffset+=note.nSize;
            nSize-=note.nSize;
        }
        else
        {
            break;
        }
    }

    return listResult;
}

XELF::NOTE XELF::_readNote(qint64 nOffset, qint64 nSize, bool bIsBigEndian)
{
    NOTE result={};

    if(nSize>=6)
    {
        quint16 nNameLength=   read_uint32(nOffset+0,bIsBigEndian);
        quint16 nDescLength=   read_uint32(nOffset+4,bIsBigEndian);
        quint16 nType=         read_uint32(nOffset+8,bIsBigEndian);

        qint32 nNoteSize=12+S_ALIGN_UP(nNameLength,4)+S_ALIGN_UP(nDescLength,4);

        if(nSize>=nNoteSize)
        {
            result.nOffset=nOffset;
            result.nSize=nNoteSize;
            result.nType=nType;

            if(nNameLength>=1)
            {
                result.sName=read_ansiString(nOffset+12,nNameLength-1);
            }

            result.nDataOffset=nOffset+12+S_ALIGN_UP(nNameLength,4);
        }
    }

    return result;
}

QList<XELF::TAG_STRUCT> XELF::getTagStructs()
{
    _MEMORY_MAP memoryMap=getMemoryMap();
    QList<XELF_DEF::Elf_Phdr> _listPhdr=getElf_PhdrList();

    return getTagStructs(&_listPhdr,&memoryMap);
}

QList<XELF::TAG_STRUCT> XELF::getTagStructs(QList<XELF_DEF::Elf_Phdr> *pPhdrList,XBinary::_MEMORY_MAP *pMemoryMap)
{
    QList<TAG_STRUCT> listResult;

    bool bIs64=is64();
    bool bIsBigEndian=isBigEndian();

    QList<XELF_DEF::Elf_Phdr> listTags=_getPrograms(pPhdrList,2); // TODO const

    int nCount=listTags.count();

    for(int i=0;i<nCount;i++)
    {
        qint64 nOffset=listTags.at(i).p_offset; //  Check image
        qint64 nSize=listTags.at(i).p_filesz;

        if(isOffsetAndSizeValid(pMemoryMap,nOffset,nSize))
        {
            listResult.append(_getTagStructs(nOffset,nSize,bIs64,bIsBigEndian));
        }
    }

    return listResult;
}

QList<XELF::TAG_STRUCT> XELF::_getTagStructs(qint64 nOffset, qint64 nSize, bool bIs64, bool bIsBigEndian)
{
    QList<TAG_STRUCT> listResult;

    while(nSize>0)
    {
        TAG_STRUCT tagStruct={};
        tagStruct.nOffset=nOffset;

        if(bIs64)
        {
            tagStruct.nTag=read_int64(nOffset,bIsBigEndian);
            tagStruct.nValue=read_int64(nOffset+8,bIsBigEndian);
            nOffset+=16;
            nSize-=16;
        }
        else
        {
            tagStruct.nTag=read_int32(nOffset,bIsBigEndian);
            tagStruct.nValue=read_int32(nOffset+4,bIsBigEndian);
            nOffset+=8;
            nSize-=8;
        }

        if(tagStruct.nTag)
        {
            listResult.append(tagStruct);
        }
        else
        {
            break;
        }
    }

    return listResult;
}

QList<XELF::TAG_STRUCT> XELF::_getTagStructs(QList<XELF::TAG_STRUCT> *pList,qint64 nTag)
{
    QList<XELF::TAG_STRUCT> listResult;

    int nCount=pList->count();

    for(int i=0;i<nCount;i++)
    {
        if(pList->at(i).nTag==nTag)
        {
            listResult.append(pList->at(i));
        }
    }

    return listResult;
}

qint64 XELF::getDynamicArraySize()
{
    quint64 nResult=0;

    bool bIs64=is64();

    if(bIs64)
    {
        nResult=16; // TODO defs
    }
    else
    {
        nResult=8; // TODO defs
    }

    return nResult;
}

qint64 XELF::getDynamicArrayTag(qint64 nOffset)
{
    qint64 nResult=0;

    bool bIs64=is64();
    bool bIsBigEndian=isBigEndian();

    if(bIs64)
    {
        nResult=read_int64(nOffset,bIsBigEndian);
    }
    else
    {
        nResult=read_int32(nOffset,bIsBigEndian);
    }

    return nResult;
}

qint64 XELF::getDynamicArrayValue(qint64 nOffset)
{
    qint64 nResult=0;

    bool bIs64=is64();
    bool bIsBigEndian=isBigEndian();

    if(bIs64)
    {
        nResult=read_int64(nOffset+8,bIsBigEndian);
    }
    else
    {
        nResult=read_int32(nOffset+4,bIsBigEndian);
    }

    return nResult;
}

void XELF::setDynamicArrayTag(qint64 nOffset, qint64 nValue)
{
    bool bIs64=is64();
    bool bIsBigEndian=isBigEndian();

    if(bIs64)
    {
        write_int64(nOffset,nValue,bIsBigEndian);
    }
    else
    {
        write_int32(nOffset,(qint32)nValue,bIsBigEndian);
    }
}

void XELF::setDynamicArrayValue(qint64 nOffset, qint64 nValue)
{
    bool bIs64=is64();
    bool bIsBigEndian=isBigEndian();

    if(bIs64)
    {
        write_int64(nOffset+8,nValue,bIsBigEndian);
    }
    else
    {
        write_int32(nOffset+4,(qint32)nValue,bIsBigEndian);
    }
}

XBinary::OFFSETSIZE XELF::getStringTable()
{
    _MEMORY_MAP memoryMap=getMemoryMap();
    QList<XELF::TAG_STRUCT> listStructs=getTagStructs();

    return getStringTable(&memoryMap,&listStructs);
}

XBinary::OFFSETSIZE XELF::getStringTable(XBinary::_MEMORY_MAP *pMemoryMap, QList<XELF::TAG_STRUCT> *pList)
{
    OFFSETSIZE result={};

    QList<XELF::TAG_STRUCT> listStrTab=XELF::_getTagStructs(pList,XELF_DEF::S_DT_STRTAB);
    QList<XELF::TAG_STRUCT> listStrSize=XELF::_getTagStructs(pList,XELF_DEF::S_DT_STRSZ);

    if(listStrTab.count()&&listStrSize.count())
    {
        qint64 nOffset=addressToOffset(pMemoryMap,listStrTab.at(0).nValue);
        qint64 nSize=listStrSize.at(0).nValue;

        if(isOffsetAndSizeValid(pMemoryMap,nOffset,nSize))
        {
            result.nOffset=nOffset;
            result.nSize=nSize;
        }
    }

    return result;
}

QList<QString> XELF::getLibraries()
{
    _MEMORY_MAP memoryMap=getMemoryMap();
    QList<XELF::TAG_STRUCT> listStructs=getTagStructs();

    return getLibraries(&memoryMap,&listStructs);
}

QList<QString> XELF::getLibraries(_MEMORY_MAP *pMemoryMap,QList<XELF::TAG_STRUCT> *pList)
{
    QList<QString> listResult;

    QList<XELF::TAG_STRUCT> listNeeded=XELF::_getTagStructs(pList,XELF_DEF::S_DT_NEEDED);

    OFFSETSIZE os=getStringTable(pMemoryMap,pList);

    if(os.nSize)
    {
        qint64 nOffset=os.nOffset;

        QByteArray baSection=read_array(nOffset,os.nSize);

        qint64 nSectionTableSize=baSection.size();

        int nCount=listNeeded.count();

        for(int i=0;i<nCount;i++)
        {
            qint64 nValue=listNeeded.at(i).nValue;

            if(nValue<nSectionTableSize)
            {
                QString sLibrary=baSection.data()+nValue;

                listResult.append(sLibrary);
            }
        }
    }

    return listResult;
}

XBinary::OS_ANSISTRING XELF::getRunPath()
{
    _MEMORY_MAP memoryMap=getMemoryMap();
    QList<XELF::TAG_STRUCT> listStructs=getTagStructs();

    return getRunPath(&memoryMap,&listStructs);
}

XBinary::OS_ANSISTRING XELF::getRunPath(XBinary::_MEMORY_MAP *pMemoryMap, QList<XELF::TAG_STRUCT> *pList)
{
    OS_ANSISTRING result={};

    QList<XELF::TAG_STRUCT> listRunPath=XELF::_getTagStructs(pList,0x1d); // TODO const
    QList<XELF::TAG_STRUCT> listStrTab=XELF::_getTagStructs(pList,XELF_DEF::S_DT_STRTAB);
    QList<XELF::TAG_STRUCT> listStrSize=XELF::_getTagStructs(pList,XELF_DEF::S_DT_STRSZ);

    if(listStrTab.count()&&listStrSize.count()&&listRunPath.count())
    {
        qint64 nOffset=addressToOffset(pMemoryMap,listStrTab.at(0).nValue);
        qint64 nSize=listStrSize.at(0).nValue;
        qint64 nRunPath=listRunPath.at(0).nValue;

        if(nRunPath<nSize)
        {
            result.nOffset=nOffset+nRunPath;
            result.sAnsiString=read_ansiString(result.nOffset);
            result.nSize=result.sAnsiString.length();
        }
    }

    return result;
}

QMap<quint64, QString> XELF::getDynamicTags()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0,"DT_NULL");
    mapResult.insert(1,"DT_NEEDED");
    mapResult.insert(2,"DT_PLTRELSZ");
    mapResult.insert(3,"DT_PLTGOT");
    mapResult.insert(4,"DT_HASH");
    mapResult.insert(5,"DT_STRTAB");
    mapResult.insert(6,"DT_SYMTAB");
    mapResult.insert(7,"DT_RELA");
    mapResult.insert(8,"DT_RELASZ");
    mapResult.insert(9,"DT_RELAENT");
    mapResult.insert(10,"DT_STRSZ");
    mapResult.insert(11,"DT_SYMENT");
    mapResult.insert(12,"DT_INIT");
    mapResult.insert(13,"DT_FINI");
    mapResult.insert(14,"DT_SONAME");
    mapResult.insert(15,"DT_RPATH");
    mapResult.insert(16,"DT_SYMBOLIC");
    mapResult.insert(17,"DT_REL");
    mapResult.insert(18,"DT_RELSZ");
    mapResult.insert(19,"DT_RELENT");
    mapResult.insert(20,"DT_PLTREL");
    mapResult.insert(21,"DT_DEBUG");
    mapResult.insert(22,"DT_TEXTREL");
    mapResult.insert(23,"DT_JMPREL");
    mapResult.insert(24,"DT_BIND_NOW");
    mapResult.insert(25,"DT_INIT_ARRAY");
    mapResult.insert(26,"DT_FINI_ARRAY");
    mapResult.insert(27,"DT_INIT_ARRAYSZ");
    mapResult.insert(28,"DT_FINI_ARRAYSZ");
    mapResult.insert(29,"DT_RUNPATH");
    mapResult.insert(30,"DT_FLAGS");
    mapResult.insert(32,"DT_PREINIT_ARRAY"); // DT_ENCODING
    mapResult.insert(33,"DT_PREINIT_ARRAYSZ");
    mapResult.insert(0x6000000d,"DT_LOOS");
    mapResult.insert(0x6000000e,"DT_SUNW_RTLDINF");
    mapResult.insert(0x6ffff000,"DT_HIOS");
    mapResult.insert(0x6ffffd00,"DT_VALRNGLO");
    mapResult.insert(0x6ffffdf8,"DT_CHECKSUM");
    mapResult.insert(0x6ffffdf9,"DT_PLTPADSZ");
    mapResult.insert(0x6ffffdfa,"DT_MOVEENT");
    mapResult.insert(0x6ffffdfb,"DT_MOVESZ");
    mapResult.insert(0x6ffffdfc,"DT_FEATURE_1");
    mapResult.insert(0x6ffffdfd,"DT_POSFLAG_1");
    mapResult.insert(0x6ffffdfe,"DT_SYMINSZ");
    mapResult.insert(0x6ffffdff,"DT_SYMINENT");
    mapResult.insert(0x6ffffdff,"DT_VALRNGHI");
    mapResult.insert(0x6ffffe00,"DT_ADDRRNGLO");
    mapResult.insert(0x6ffffef5,"DT_GNU_HASH");
    mapResult.insert(0x6ffffefa,"DT_CONFIG");
    mapResult.insert(0x6ffffefb,"DT_DEPAUDIT");
    mapResult.insert(0x6ffffefc,"DT_AUDIT");
    mapResult.insert(0x6ffffefd,"DT_PLTPAD");
    mapResult.insert(0x6ffffefe,"DT_MOVETAB");
    mapResult.insert(0x6ffffeff,"DT_SYMINFO");
    mapResult.insert(0x6ffffeff,"DT_ADDRRNGHI");
    mapResult.insert(0x6ffffff0,"DT_VERSYM");
    mapResult.insert(0x6ffffff9,"DT_RELACOUNT");
    mapResult.insert(0x6ffffffa,"DT_RELCOUNT");
    mapResult.insert(0x6ffffffb,"DT_FLAGS_1");
    mapResult.insert(0x6ffffffc,"DT_VERDEF");
    mapResult.insert(0x6ffffffd,"DT_VERDEFNUM");
    mapResult.insert(0x6ffffffe,"DT_VERNEED");
    mapResult.insert(0x6fffffff,"DT_VERNEEDNUM");
    mapResult.insert(0x70000000,"DT_LOPROC");
    mapResult.insert(0x70000001,"DT_SPARC_REGISTER");
    mapResult.insert(0x7ffffffd,"DT_AUXILIARY");
    mapResult.insert(0x7ffffffe,"DT_USED");
    mapResult.insert(0x7fffffff,"DT_HIPROC"); // DT_FILTER

    return mapResult;
}

QMap<quint64, QString> XELF::getDynamicTagsS()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0,"NULL");
    mapResult.insert(1,"NEEDED");
    mapResult.insert(2,"PLTRELSZ");
    mapResult.insert(3,"PLTGOT");
    mapResult.insert(4,"HASH");
    mapResult.insert(5,"STRTAB");
    mapResult.insert(6,"SYMTAB");
    mapResult.insert(7,"RELA");
    mapResult.insert(8,"RELASZ");
    mapResult.insert(9,"RELAENT");
    mapResult.insert(10,"STRSZ");
    mapResult.insert(11,"SYMENT");
    mapResult.insert(12,"INIT");
    mapResult.insert(13,"FINI");
    mapResult.insert(14,"SONAME");
    mapResult.insert(15,"RPATH");
    mapResult.insert(16,"SYMBOLIC");
    mapResult.insert(17,"REL");
    mapResult.insert(18,"RELSZ");
    mapResult.insert(19,"RELENT");
    mapResult.insert(20,"PLTREL");
    mapResult.insert(21,"DEBUG");
    mapResult.insert(22,"TEXTREL");
    mapResult.insert(23,"JMPREL");
    mapResult.insert(24,"BIND_NOW");
    mapResult.insert(25,"INIT_ARRAY");
    mapResult.insert(26,"FINI_ARRAY");
    mapResult.insert(27,"INIT_ARRAYSZ");
    mapResult.insert(28,"FINI_ARRAYSZ");
    mapResult.insert(29,"RUNPATH");
    mapResult.insert(30,"FLAGS");
    mapResult.insert(32,"PREINIT_ARRAY"); // ENCODING
    mapResult.insert(33,"PREINIT_ARRAYSZ");
    mapResult.insert(0x6000000d,"LOOS");
    mapResult.insert(0x6000000e,"SUNW_RTLDINF");
    mapResult.insert(0x6ffff000,"HIOS");
    mapResult.insert(0x6ffffd00,"VALRNGLO");
    mapResult.insert(0x6ffffdf8,"CHECKSUM");
    mapResult.insert(0x6ffffdf9,"PLTPADSZ");
    mapResult.insert(0x6ffffdfa,"MOVEENT");
    mapResult.insert(0x6ffffdfb,"MOVESZ");
    mapResult.insert(0x6ffffdfc,"FEATURE_1");
    mapResult.insert(0x6ffffdfd,"POSFLAG_1");
    mapResult.insert(0x6ffffdfe,"SYMINSZ");
    mapResult.insert(0x6ffffdff,"SYMINENT");
    mapResult.insert(0x6ffffdff,"VALRNGHI");
    mapResult.insert(0x6ffffe00,"ADDRRNGLO");
    mapResult.insert(0x6ffffef5,"GNU_HASH");
    mapResult.insert(0x6ffffefa,"CONFIG");
    mapResult.insert(0x6ffffefb,"DEPAUDIT");
    mapResult.insert(0x6ffffefc,"AUDIT");
    mapResult.insert(0x6ffffefd,"PLTPAD");
    mapResult.insert(0x6ffffefe,"MOVETAB");
    mapResult.insert(0x6ffffeff,"SYMINFO");
    mapResult.insert(0x6ffffeff,"ADDRRNGHI");
    mapResult.insert(0x6ffffff0,"VERSYM");
    mapResult.insert(0x6ffffff9,"RELACOUNT");
    mapResult.insert(0x6ffffffa,"RELCOUNT");
    mapResult.insert(0x6ffffffb,"FLAGS_1");
    mapResult.insert(0x6ffffffc,"VERDEF");
    mapResult.insert(0x6ffffffd,"VERDEFNUM");
    mapResult.insert(0x6ffffffe,"VERNEED");
    mapResult.insert(0x6fffffff,"VERNEEDNUM");
    mapResult.insert(0x70000000,"LOPROC");
    mapResult.insert(0x70000001,"SPARC_REGISTER");
    mapResult.insert(0x7ffffffd,"AUXILIARY");
    mapResult.insert(0x7ffffffe,"USED");
    mapResult.insert(0x7fffffff,"HIPROC"); // FILTER

    return mapResult;
}

 XBinary::_MEMORY_MAP XELF::getMemoryMap()
{
    XBinary::_MEMORY_MAP result={};

    qint32 nIndex=0;

    if(is64())
    {
        result.fileType=FT_ELF64;
        result.mode=MODE_64;
    }
    else
    {
        result.fileType=FT_ELF32;
        result.mode=MODE_32;
    }

    result.sArch=getArch();
    result.bIsBigEndian=isBigEndian();

    result.nRawSize=getSize();

    QList<XELF_DEF::Elf_Phdr> _listPhdr=getElf_PhdrList();
    QList<XELF_DEF::Elf_Phdr> listSegments=_getPrograms(&_listPhdr,1); // TODO const

//    bool bIs64=is64();
    int nCount=listSegments.count();

    bool bImageAddressInit=false;

    qint64 nMaxOffset=0;
    qint64 nMaxAddress=0;

    for(int i=0; i<nCount; i++)
    {
        QString sName=QString("%1(%2)").arg(tr("Segment")).arg(i);

        quint64 nAlign=listSegments.at(i).p_align;
        qint64 nVirtualAddress=S_ALIGN_DOWN(listSegments.at(i).p_vaddr,nAlign);
        qint64 nFileOffset=S_ALIGN_DOWN(listSegments.at(i).p_offset,nAlign);
        qint64 nVirtualSize=S_ALIGN_UP(listSegments.at(i).p_memsz,nAlign);
        qint64 nFileSize=S_ALIGN_UP(listSegments.at(i).p_filesz,nAlign);

        if(nFileSize)
        {
            XBinary::_MEMORY_RECORD record={};

            record.type=MMT_LOADSECTION;
            record.sName=sName;
            // TODO Section number!
            record.nAddress=nVirtualAddress;
            record.nSize=nFileSize;
            record.nOffset=nFileOffset;
            record.nIndex=nIndex++;

            result.listRecords.append(record);
        }

        if(nVirtualSize>nFileSize)
        {
            XBinary::_MEMORY_RECORD record={};

            record.type=MMT_LOADSECTION;
            record.sName=sName;
            // TODO Section number!
            record.nAddress=nVirtualAddress+nFileSize;
            record.nSize=nVirtualSize-nFileSize;
            record.nOffset=-1;
            record.nIndex=nIndex++;
            record.bIsVirtual=true;

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

    qint64 nNoLoadableSize=result.nRawSize-nMaxOffset;

    if(nNoLoadableSize>0)
    {
        XBinary::_MEMORY_RECORD record={};

        record.type=MMT_NOLOADABLE;
        // TODO Section number!
        // TODO virtual sections!
        record.nAddress=-1;
        record.nSize=nNoLoadableSize;
        record.nOffset=nMaxOffset;
        record.nIndex=nIndex++;

        result.listRecords.append(record);
    }

    return  result;
}

qint64 XELF::getEntryPointOffset(_MEMORY_MAP *pMemoryMap)
{
    qint64 nAddress=-1;

    bool bIs64=is64();

    if(bIs64)
    {
        nAddress=getHdr64_entry();
    }
    else
    {
        nAddress=getHdr32_entry();
    }

    return addressToOffset(pMemoryMap,nAddress);
}

QList<XELF::SECTION_RECORD> XELF::getSectionRecords(QList<XELF_DEF::Elf_Shdr> *pList, bool bIsImage, QByteArray *pbaSectionTable)
{
    QList<SECTION_RECORD> listRecords;

    int nCount=pList->count();
    quint32 nSectionTableSize=pbaSectionTable->size();

    for(int i=0;i<nCount;i++)
    {
        SECTION_RECORD record={};

        record.nSize=pList->at(i).sh_size;
        record.nFlags=pList->at(i).sh_flags;

        if(bIsImage)
        {
            record.nOffset=pList->at(i).sh_addr;
        }
        else
        {
            record.nOffset=pList->at(i).sh_offset;
        }

        if(pList->at(i).sh_name<nSectionTableSize)
        {
            record.sName=pbaSectionTable->data()+pList->at(i).sh_name;
        }

        listRecords.append(record);
    }

    return listRecords;
}

bool XELF::isSectionNamePresent(QString sSectionName)
{
    bool bIs64=is64();
    quint16 nStringTableSection=getSectionStringTable(bIs64);
    QByteArray baStringTable=getSection(nStringTableSection);
    QList<XELF_DEF::Elf_Shdr> listSH=getElf_ShdrList();

    QList<XELF::SECTION_RECORD> listSR=getSectionRecords(&listSH,bIs64,&baStringTable);

    return isSectionNamePresent(sSectionName,&listSR);
}

bool XELF::isSectionNamePresent(QString sSectionName, QList<XELF::SECTION_RECORD> *pListSections)
{
    bool bResult=false;

    int nNumberOfSections=pListSections->count();

    for(int i=0; i<nNumberOfSections; i++)
    {
        if(pListSections->at(i).sName==sSectionName)
        {
            bResult=true;
            break;
        }
    }

    return bResult;
}

qint32 XELF::getSectionNumber(QString sSectionName)
{
    bool bIs64=is64();
    quint16 nStringTableSection=getSectionStringTable(bIs64);
    QByteArray baStringTable=getSection(nStringTableSection);
    QList<XELF_DEF::Elf_Shdr> listSH=getElf_ShdrList();

    QList<XELF::SECTION_RECORD> listSR=getSectionRecords(&listSH,bIs64,&baStringTable);

    return getSectionNumber(sSectionName,&listSR);
}

qint32 XELF::getSectionNumber(QString sSectionName, QList<XELF::SECTION_RECORD> *pListSections)
{
    qint32 nResult=-1;

    int nNumberOfSections=pListSections->count();

    for(int i=0; i<nNumberOfSections; i++)
    {
        if(pListSections->at(i).sName==sSectionName)
        {
            nResult=i;
            break;
        }
    }

    return nResult;
}

XELF::SECTION_RECORD XELF::getSectionRecord(QString sSectionName, QList<XELF::SECTION_RECORD> *pListSections)
{
    SECTION_RECORD result={};

    int nNumberOfSections=pListSections->count();

    for(int i=0; i<nNumberOfSections; i++)
    {
        if(pListSections->at(i).sName==sSectionName)
        {
            result=pListSections->at(i);
            break;
        }
    }

    return result;
}

XBinary::MODE XELF::getMode()
{
    MODE result=MODE_32;

    if(is64())
    {
        result=MODE_64;
    }

    return result;
}

QString XELF::getArch()
{
    return getMachinesS().value(getHdr32_machine(),QString("UNKNOWN"));
}

qint64 XELF::getBaseAddress()
{
    return getMemoryMap().nBaseAddress;
}

void XELF::setBaseAddress(qint64 nValue)
{
    Q_UNUSED(nValue)
    //  TODO
}

QList<XELF_DEF::Elf_Phdr> XELF::_getPrograms(QList<XELF_DEF::Elf_Phdr> *pList, quint32 nType)
{
    QList<XELF_DEF::Elf_Phdr> listResult;

    int nCount=pList->count();

    for(int i=0;i<nCount;i++)
    {
        if(pList->at(i).p_type==nType)
        {
            listResult.append(pList->at(i));
        }
    }

    return listResult;
}

QList<XBinary::DATASET> XELF::getDatasetsFromSections(QList<XELF_DEF::Elf_Shdr> *pList)
{
    QList<XBinary::DATASET> listResult;

    quint32 nMainStringSection=getSectionStringTable();

    XBinary::OFFSETSIZE osStringTable=getSectionOffsetSize(nMainStringSection);

    int nCount=pList->count();

    for(int i=0;i<nCount;i++)
    {
        QString sSectionName=getStringFromIndex(osStringTable.nOffset,osStringTable.nSize,pList->at(i).sh_name);

        if((pList->at(i).sh_type==1)&&(sSectionName==".interp")) // Interpreter TODO const
        {
            DATASET dataset={};

            dataset.nAddress=pList->at(i).sh_addr;
            dataset.nOffset=pList->at(i).sh_offset;
            dataset.nSize=pList->at(i).sh_size;
            dataset.nType=DS_INTERPRETER;
            dataset.sName=QString("%1[%2]").arg("Interpreter").arg(sSectionName); // TODO mb translate

            listResult.append(dataset);
        }
        else if(pList->at(i).sh_type==2) // Symbol table TODO const
        {
            DATASET dataset={};

            dataset.nAddress=pList->at(i).sh_addr;
            dataset.nOffset=pList->at(i).sh_offset;
            dataset.nSize=pList->at(i).sh_size;
            dataset.nType=DS_SYMBOLTABLE;
            dataset.sName=QString("%1[%2]").arg("Symbol table").arg(sSectionName); // TODO mb translate

            listResult.append(dataset);
        }
        else if(pList->at(i).sh_type==3) // String table TODO const
        {
            DATASET dataset={};

            dataset.nAddress=pList->at(i).sh_addr;
            dataset.nOffset=pList->at(i).sh_offset;
            dataset.nSize=pList->at(i).sh_size;
            dataset.nType=DS_STRINGTABLE;
            dataset.sName=QString("%1[%2]").arg("String table").arg(sSectionName); // TODO mb translate

            listResult.append(dataset);
        }
        else if(pList->at(i).sh_type==6) // Dynamic TODO const
        {
            DATASET dataset={};

            dataset.nAddress=pList->at(i).sh_addr;
            dataset.nOffset=pList->at(i).sh_offset;
            dataset.nSize=pList->at(i).sh_size;
            dataset.nType=DS_DYNAMICTAGS;
            dataset.sName=QString("%1[%2]").arg("Dynamic tags").arg(sSectionName); // TODO mb translate

            listResult.append(dataset);
        }
        else if(pList->at(i).sh_type==7) // Notes TODO const
        {
            DATASET dataset={};

            dataset.nAddress=pList->at(i).sh_addr;
            dataset.nOffset=pList->at(i).sh_offset;
            dataset.nSize=pList->at(i).sh_size;
            dataset.nType=DS_NOTES;
            dataset.sName=QString("%1[%2]").arg("Notes").arg(sSectionName); // TODO mb translate

            listResult.append(dataset);
        }
    }

    return listResult;
}

QList<XBinary::DATASET> XELF::getDatasetsFromPrograms(QList<XELF_DEF::Elf_Phdr> *pList)
{
    QList<XBinary::DATASET> listResult;

    int nCount=pList->count();

    for(int i=0;i<nCount;i++)
    {
        if((pList->at(i).p_type==3)) // Interpreter TODO const
        {
            DATASET dataset={};

            dataset.nAddress=pList->at(i).p_vaddr;
            dataset.nOffset=pList->at(i).p_offset;
            dataset.nSize=pList->at(i).p_filesz;
            dataset.nType=DS_INTERPRETER;
            dataset.sName=QString("%1").arg("Interpreter"); // TODO mb translate

            listResult.append(dataset);
        }
        else if((pList->at(i).p_type==4)) // Notes TODO const
        {
            DATASET dataset={};

            dataset.nAddress=pList->at(i).p_vaddr;
            dataset.nOffset=pList->at(i).p_offset;
            dataset.nSize=pList->at(i).p_filesz;
            dataset.nType=DS_NOTES;
            dataset.sName=QString("%1").arg("Notes"); // TODO mb translate

            listResult.append(dataset);
        }
        else if((pList->at(i).p_type==2)) // Tags TODO const
        {
            DATASET dataset={};

            dataset.nAddress=pList->at(i).p_vaddr;
            dataset.nOffset=pList->at(i).p_offset;
            dataset.nSize=pList->at(i).p_filesz;
            dataset.nType=DS_DYNAMICTAGS;
            dataset.sName=QString("%1").arg("Dynamic tags"); // TODO mb translate

            listResult.append(dataset);
        }
    }

    return listResult;
}

QList<XBinary::DATASET> XELF::getDatasetsFromTagStructs(XBinary::_MEMORY_MAP *pMemoryMap, QList<XELF::TAG_STRUCT> *pList)
{
    QList<XBinary::DATASET> listResult;

    QList<XELF::TAG_STRUCT> listStrTab=XELF::_getTagStructs(pList,XELF_DEF::S_DT_STRTAB);
    QList<XELF::TAG_STRUCT> listStrSize=XELF::_getTagStructs(pList,XELF_DEF::S_DT_STRSZ);
    QList<XELF::TAG_STRUCT> listStrNeeded=XELF::_getTagStructs(pList,XELF_DEF::S_DT_NEEDED);
    QList<XELF::TAG_STRUCT> listRunPath=XELF::_getTagStructs(pList,0x1d); // TODO const

    if(listStrTab.count()&&listStrSize.count())
    {
        DATASET dataset={};

        dataset.nAddress=listStrTab.at(0).nValue;
        dataset.nOffset=addressToOffset(pMemoryMap,dataset.nAddress);
        dataset.nSize=listStrSize.at(0).nValue;
        dataset.nType=DS_STRINGTABLE;
        dataset.sName="String table"; // TODO mb translate

        if(isOffsetAndSizeValid(pMemoryMap,dataset.nOffset,dataset.nSize))
        {
            listResult.append(dataset);
        }
    }

    if(listStrNeeded.count())
    {
        DATASET dataset={};

        dataset.nAddress=0;
        dataset.nOffset=0;
        dataset.nSize=0;
        dataset.nType=DS_LIBRARIES;
        dataset.sName="Libraries"; // TODO mb translate

        listResult.append(dataset);
    }

    if(listRunPath.count())
    {
        qint64 nAddress=listStrTab.at(0).nValue;
        qint64 nOffset=addressToOffset(pMemoryMap,nAddress);
        qint64 nRunPath=listRunPath.at(0).nValue;
        qint64 nSize=listStrSize.at(0).nValue;

        if(nRunPath<nSize)
        {
            DATASET dataset={};

            dataset.nAddress=nAddress+nRunPath;
            dataset.nOffset=nOffset+nRunPath;
            QString sAnsiString=read_ansiString(dataset.nOffset);
            dataset.nSize=sAnsiString.length();
            dataset.nType=DS_RUNPATH;
            dataset.sName="Run path"; // TODO mb translate

            listResult.append(dataset);
        }
    }

    return listResult;
}

QList<XELF_DEF::Elf32_Sym> XELF::getElf32_SymList(qint64 nOffset, qint64 nSize)
{
    QList<XELF_DEF::Elf32_Sym> listResult;

    bool bIsBigEndian=isBigEndian();

    while(nSize>0)
    {
        XELF_DEF::Elf32_Sym record=_readElf32_Sym(nOffset,bIsBigEndian);

        listResult.append(record);

        nOffset+=sizeof(XELF_DEF::Elf32_Sym);
        nSize-=sizeof(XELF_DEF::Elf32_Sym);
    }

    return listResult;
}

QList<XELF_DEF::Elf64_Sym> XELF::getElf64_SymList(qint64 nOffset, qint64 nSize)
{
    QList<XELF_DEF::Elf64_Sym> listResult;

    bool bIsBigEndian=isBigEndian();

    while(nSize>0)
    {
        XELF_DEF::Elf64_Sym record=_readElf64_Sym(nOffset,bIsBigEndian);

        listResult.append(record);

        nOffset+=sizeof(XELF_DEF::Elf64_Sym);
        nSize-=sizeof(XELF_DEF::Elf64_Sym);
    }

    return listResult;
}

XELF_DEF::Elf32_Sym XELF::_readElf32_Sym(qint64 nOffset, bool bIsBigEndian)
{
    XELF_DEF::Elf32_Sym result={};

    result.st_name=read_uint32(nOffset+offsetof(XELF_DEF::Elf32_Sym,st_name),bIsBigEndian);
    result.st_value=read_uint32(nOffset+offsetof(XELF_DEF::Elf32_Sym,st_value),bIsBigEndian);
    result.st_size=read_uint32(nOffset+offsetof(XELF_DEF::Elf32_Sym,st_size),bIsBigEndian);
    result.st_info=read_uint8(nOffset+offsetof(XELF_DEF::Elf32_Sym,st_info));
    result.st_other=read_uint8(nOffset+offsetof(XELF_DEF::Elf32_Sym,st_other));
    result.st_shndx=read_uint16(nOffset+offsetof(XELF_DEF::Elf32_Sym,st_shndx),bIsBigEndian);

    return result;
}

XELF_DEF::Elf64_Sym XELF::_readElf64_Sym(qint64 nOffset, bool bIsBigEndian)
{
    XELF_DEF::Elf64_Sym result={};

    result.st_name=read_uint32(nOffset+offsetof(XELF_DEF::Elf64_Sym,st_name),bIsBigEndian);
    result.st_info=read_uint8(nOffset+offsetof(XELF_DEF::Elf64_Sym,st_info));
    result.st_other=read_uint8(nOffset+offsetof(XELF_DEF::Elf64_Sym,st_other));
    result.st_shndx=read_uint16(nOffset+offsetof(XELF_DEF::Elf64_Sym,st_shndx),bIsBigEndian);
    result.st_value=read_uint64(nOffset+offsetof(XELF_DEF::Elf64_Sym,st_value),bIsBigEndian);
    result.st_size=read_uint64(nOffset+offsetof(XELF_DEF::Elf64_Sym,st_size),bIsBigEndian);

    return result;
}
