/* Copyright (c) 2017-2022 hors<horsicq@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef XELF_DEF_H
#define XELF_DEF_H

#include <QtGlobal>

#define S_ELF32_R_SYM(info)             ((info)>>8)
#define S_ELF32_R_TYPE(info)            ((quint8)(info))
#define S_ELF32_R_INFO(sym,type)        (((sym)<<8)+(quint8)(type))

#define S_ELF64_R_SYM(info)             ((info)>>32)
#define S_ELF64_R_TYPE(info)            ((info)&0xFFFFFFFFL)
#define S_ELF64_R_INFO(sym,type)        (((sym)<<32)+((type)&0xFFFFFFFFL))

#define S_ELF32_ST_BIND(info)           ((info)>>4)
#define S_ELF32_ST_TYPE(info)           ((info)&0xF)
#define S_ELF32_ST_INFO(bind,type)      (((bind)<<4)+((type)&0xF))

#define S_ELF64_ST_BIND(info)           ((info)>>4)
#define S_ELF64_ST_TYPE(info)           ((info)&0xF)
#define S_ELF64_ST_INFO(bind,type)      (((bind)<<4)+((type)&0xF))

namespace XELF_DEF
{
//typedef quint32        Elf32_Addr;
//typedef quint16        Elf32_Half;
//typedef quint32        Elf32_Off;
//typedef qint32         Elf32_Sword;
//typedef quint32        Elf32_Word;
//typedef quint32        Elf32_Size;

//typedef quint64        Elf64_Addr;
//typedef quint16        Elf64_Half;
//typedef quint64        Elf64_Off;
//typedef qint32         Elf64_Sword;
//typedef quint32        Elf64_Word;
//typedef quint64        Elf64_Size;
//typedef int64_t        Elf64_Sxword;
//typedef uint32_t       Elf64_Word;
//typedef uint64_t       Elf64_Lword;
//typedef uint64_t       Elf64_Xword;

const quint8 EI_NIDENT                      =16;
const quint8 EI_MAG0                        =0;
const quint8 ELFMAG0                        =0x7F;
const quint8 EI_MAG1                        =1;
const quint8 ELFMAG1                        ='E';
const quint8 EI_MAG2                        =2;
const quint8 ELFMAG2                        ='L';
const quint8 EI_MAG3                        =3;
const quint8 ELFMAG3                        ='F';
//const quint8 ELFMAG          "\177ELF"
const quint32 ELFMAG                        =0x464C457F;
const quint8 SELFMAG                        =4;
const quint8 EI_CLASS                       =4;                     /* File class byte index */
const quint8 ELFCLASSNONE                   =0;                     /* Invalid class */
const quint8 ELFCLASS32                     =1;                     /* 32-bit objects */
const quint8 ELFCLASS64                     =2;                     /* 64-bit objects */
const quint8 EI_DATA                        =5;                     /* Data encodeing byte index */
const quint8 ELFDATANONE                    =0;                     /* Invalid data encoding */
const quint8 ELFDATA2LSB                    =1;                     /* 2's complement little endian */
const quint8 ELFDATA2MSB                    =2;                     /* 2's complement big endian */
const quint8 EI_VERSION                     =6;                     /* File version byte index */
const quint8 EV_NONE                        =0;                     /* Invalid ELF Version */
const quint8 EV_CURRENT                     =1;                     /* Current version */
const quint8 EI_OSABI                       =7;                     /* Operating system/ABI identification */
const quint8 EI_ABIVERSION                  =8;                     /* ABI version */
const quint8 SHN_UNDEF                      =0;

const quint16 ET_NONE                       =0;
const quint16 ET_REL                        =1;
const quint16 ET_EXEC                       =2;
const quint16 ET_DYN                        =3;
const quint16 ET_CORE                       =4;
const quint16 ET_NUM                        =5;
const quint16 ET_LOPROC                     =0xFF00;
const quint16 ET_HIPROC                     =0xFFFF;

const quint32 DT_NULL                       =0;                     /* Marks end of dynamic section */
const quint32 DT_NEEDED                     =1;                     /* Name of needed library */
const quint32 DT_PLTRELSZ                   =2;                     /* Size in bytes of PLT relocs */
const quint32 DT_PLTGOT                     =3;                     /* Processor defined value */
const quint32 DT_HASH                       =4;                     /* Address of symbol hash table */
const quint32 DT_STRTAB                     =5;                     /* Address of string table */
const quint32 DT_SYMTAB                     =6;                     /* Address of symbol table */
const quint32 DT_RELA                       =7;                     /* Address of Rela relocs */
const quint32 DT_RELASZ                     =8;                     /* Total size of Rela relocs */
const quint32 DT_RELAENT                    =9;                     /* Size of one Rela reloc */
const quint32 DT_STRSZ                      =10;                    /* Size of string table */
const quint32 DT_RUNPATH                    =29;                    // Run path

const quint32 PT_LOAD                       =1;
const quint32 PT_DYNAMIC                    =2;
const quint32 PT_INTERP                     =3;
const quint32 PT_NOTE                       =4;

const quint32 SHT_NOTE                      =7;

const quint32 ELFOSABI_HPUX                 =1;
const quint32 ELFOSABI_NETBSD               =2;
const quint32 ELFOSABI_LINUX                =3;
const quint32 ELFOSABI_HURD                 =4;
const quint32 ELFOSABI_86OPEN               =5;
const quint32 ELFOSABI_SOLARIS              =6;
const quint32 ELFOSABI_AIX                  =7;
const quint32 ELFOSABI_IRIX                 =8;
const quint32 ELFOSABI_FREEBSD              =9;
const quint32 ELFOSABI_TRU64                =10;
const quint32 ELFOSABI_MODESTO              =11;
const quint32 ELFOSABI_OPENBSD              =12;
const quint32 ELFOSABI_OPENVMS              =13;
const quint32 ELFOSABI_NSK                  =14;
const quint32 ELFOSABI_AROS                 =15;
const quint32 ELFOSABI_FENIXOS              =16;
const quint32 ELFOSABI_OPENVOS              =18;

struct Elf32_Ehdr
{
    quint8 e_ident[EI_NIDENT];      /* ident bytes */
    quint16 e_type;                 /* file type */
    quint16 e_machine;              /* target machine */
    quint32 e_version;              /* file version */
    quint32 e_entry;                /* start address */
    quint32 e_phoff;                /* phdr file offset */
    quint32 e_shoff;                /* shdr file offset */
    quint32 e_flags;                /* file flags */
    quint16 e_ehsize;               /* sizeof ehdr */
    quint16 e_phentsize;            /* sizeof phdr */
    quint16 e_phnum;                /* number phdrs */
    quint16 e_shentsize;            /* sizeof shdr */
    quint16 e_shnum;                /* number shdrs */
    quint16 e_shstrndx;             /* shdr string index */
};

struct Elf64_Ehdr
{
    quint8 e_ident[EI_NIDENT];      /* ident bytes */
    quint16 e_type;                 /* file type */
    quint16 e_machine;              /* target machine */
    quint32 e_version;              /* file version */
    quint64 e_entry;                /* start address */
    quint64 e_phoff;                /* phdr file offset */
    quint64 e_shoff;                /* shdr file offset */
    quint32 e_flags;                /* file flags */
    quint16 e_ehsize;               /* sizeof ehdr */
    quint16 e_phentsize;            /* sizeof phdr */
    quint16 e_phnum;                /* number phdrs */
    quint16 e_shentsize;            /* sizeof shdr */
    quint16 e_shnum;                /* number shdrs */
    quint16 e_shstrndx;             /* shdr string index */
};

struct Elf_Ehdr
{
    quint8 e_ident[EI_NIDENT];      /* ident bytes */
    quint16 e_type;                 /* file type */
    quint16 e_machine;              /* target machine */
    quint32 e_version;              /* file version */
    quint64 e_entry;                /* start address */
    quint64 e_phoff;                /* phdr file offset */
    quint64 e_shoff;                /* shdr file offset */
    quint32 e_flags;                /* file flags */
    quint16 e_ehsize;               /* sizeof ehdr */
    quint16 e_phentsize;            /* sizeof phdr */
    quint16 e_phnum;                /* number phdrs */
    quint16 e_shentsize;            /* sizeof shdr */
    quint16 e_shnum;                /* number shdrs */
    quint16 e_shstrndx;             /* shdr string index */
};

struct Elf32_Shdr
{
    quint32	sh_name;                /* Section name (index into the section header string table). */
    quint32	sh_type;                /* Section type. */
    quint32	sh_flags;               /* Section flags. */
    quint32	sh_addr;                /* Address in memory image. */
    quint32	sh_offset;              /* Offset in file. */
    quint32	sh_size;                /* Size in bytes. */
    quint32	sh_link;                /* Index of a related section. */
    quint32	sh_info;                /* Depends on section type. */
    quint32	sh_addralign;           /* Alignment in bytes. */
    quint32	sh_entsize;             /* Size of each entry in section. */
};

struct Elf64_Shdr
{
    quint32	sh_name;                /* Section name (index into the section header string table). */
    quint32	sh_type;                /* Section type. */
    quint64	sh_flags;               /* Section flags. */
    quint64	sh_addr;                /* Address in memory image. */
    quint64	sh_offset;              /* Offset in file. */
    quint64	sh_size;                /* Size in bytes. */
    quint32	sh_link;                /* Index of a related section. */
    quint32	sh_info;                /* Depends on section type. */
    quint64	sh_addralign;           /* Alignment in bytes. */
    quint64	sh_entsize;             /* Size of each entry in section. */
};

struct Elf_Shdr
{
    quint32	sh_name;                /* Section name (index into the section header string table). */
    quint32	sh_type;                /* Section type. */
    quint64	sh_flags;               /* Section flags. */
    quint64	sh_addr;                /* Address in memory image. */
    quint64	sh_offset;              /* Offset in file. */
    quint64	sh_size;                /* Size in bytes. */
    quint32	sh_link;                /* Index of a related section. */
    quint32	sh_info;                /* Depends on section type. */
    quint64	sh_addralign;           /* Alignment in bytes. */
    quint64	sh_entsize;             /* Size of each entry in section. */
};

struct Elf32_Phdr
{
    quint32 p_type;                 /* entry type */
    quint32 p_offset;               /* offset */
    quint32 p_vaddr;                /* virtual address */
    quint32 p_paddr;                /* physical address */
    quint32 p_filesz;               /* file size */
    quint32 p_memsz;                /* memory size */
    quint32 p_flags;                /* flags */
    quint32 p_align;                /* memory & file alignment */
};

struct Elf64_Phdr
{
    quint32 p_type;                 /* entry type */
    quint32 p_flags;                /* flags */
    quint64 p_offset;               /* offset */
    quint64 p_vaddr;                /* virtual address */
    quint64 p_paddr;                /* physical address */
    quint64 p_filesz;               /* file size */
    quint64 p_memsz;                /* memory size */
    quint64 p_align;                /* memory & file alignment */
};

struct Elf_Phdr
{
    quint32 p_type;                 /* entry type */
    quint32 p_flags;                /* flags */
    quint64 p_offset;               /* offset */
    quint64 p_vaddr;                /* virtual address */
    quint64 p_paddr;                /* physical address */
    quint64 p_filesz;               /* file size */
    quint64 p_memsz;                /* memory size */
    quint64 p_align;                /* memory & file alignment */
};

struct Elf32_Sym
{
    quint32 st_name;                /* String table index of name. */
    quint32 st_value;               /* Symbol value. */
    quint32 st_size;                /* Size of associated object. */
    quint8 st_info;                 /* Type and binding information. */
    quint8 st_other;                /* Reserved (not used). */
    quint16 st_shndx;               /* Section index of symbol. */
};

struct Elf64_Sym
{
    quint32 st_name;
    quint8 st_info;
    quint8 st_other;
    quint16 st_shndx;
    quint64 st_value;
    quint64 st_size;
};

struct Elf32_Rel
{
    quint32 r_offset;
    quint32 r_info;
};

struct Elf32_Rela
{
    quint32 r_offset;
    quint32 r_info;
    qint32 r_addend;
};

struct Elf64_Rel
{
    quint64 r_offset;
    quint64 r_info;
};

struct Elf64_Rela
{
    quint64 r_offset;
    quint64 r_info;
    qint64 r_addend;
};

}
#endif // XELF_DEF_H
