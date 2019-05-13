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
#ifndef XELF_DEF_H
#define XELF_DEF_H

#include <QtGlobal>

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

const quint8 S_EI_NIDENT=16;
const quint8 S_EI_MAG0=0;
const quint8 S_ELFMAG0=0x7f;
const quint8 S_EI_MAG1=1;
const quint8 S_ELFMAG1='E';
const quint8 S_EI_MAG2=2;
const quint8 S_ELFMAG2='L';
const quint8 S_EI_MAG3=3;
const quint8 S_ELFMAG3='F';
//const quint8 ELFMAG          "\177ELF"
const quint8 S_ELFMAG=0x464C457F;
const quint8 S_SELFMAG=4;
const quint8 S_EI_CLASS=4;       /* File class byte index */
const quint8 S_ELFCLASSNONE=0;       /* Invalid class */
const quint8 S_ELFCLASS32=1;       /* 32-bit objects */
const quint8 S_ELFCLASS64=2;       /* 64-bit objects */
const quint8 S_EI_DATA=5;       /* Data encodeing byte index */
const quint8 S_ELFDATANONE=0;       /* Invalid data encoding */
const quint8 S_ELFDATA2LSB=1;       /* 2's complement little endian */
const quint8 S_ELFDATA2MSB=2;       /* 2's complement big endian */
const quint8 S_EI_VERSION=6;       /* File version byte index */
const quint8 S_EV_NONE=0;       /* Invalid ELF Version */
const quint8 S_EV_CURRENT=1;       /* Current version */
const quint8 S_EI_OSABI=7;       /* Operating system/ABI identification */
const quint8 S_EI_ABIVERSION=8;       /* ABI version */
const quint8 S_SHN_UNDEF=0;

struct Elf32_Ehdr
{
    unsigned char   e_ident[S_EI_NIDENT];     /* ident bytes */
    quint16      e_type;                 /* file type */
    quint16      e_machine;              /* target machine */
    quint32      e_version;              /* file version */
    quint32      e_entry;                /* start address */
    quint32      e_phoff;                /* phdr file offset */
    quint32      e_shoff;                /* shdr file offset */
    quint32      e_flags;                /* file flags */
    quint16      e_ehsize;               /* sizeof ehdr */
    quint16      e_phentsize;            /* sizeof phdr */
    quint16      e_phnum;                /* number phdrs */
    quint16      e_shentsize;            /* sizeof shdr */
    quint16      e_shnum;                /* number shdrs */
    quint16      e_shstrndx;             /* shdr string index */
};

struct Elf64_Ehdr
{
    unsigned char   e_ident[S_EI_NIDENT];     /* ident bytes */
    quint16      e_type;                 /* file type */
    quint16      e_machine;              /* target machine */
    quint32      e_version;              /* file version */
    quint64      e_entry;                /* start address */
    quint64      e_phoff;                /* phdr file offset */
    quint64      e_shoff;                /* shdr file offset */
    quint32      e_flags;                /* file flags */
    quint16      e_ehsize;               /* sizeof ehdr */
    quint16      e_phentsize;            /* sizeof phdr */
    quint16      e_phnum;                /* number phdrs */
    quint16      e_shentsize;            /* sizeof shdr */
    quint16      e_shnum;                /* number shdrs */
    quint16      e_shstrndx;             /* shdr string index */
};

struct Elf32_Shdr
{
    quint32	sh_name;	/* Section name (index into the section header string table). */
    quint32	sh_type;	/* Section type. */
    quint32	sh_flags;	/* Section flags. */
    quint32	sh_addr;	/* Address in memory image. */
    quint32	sh_offset;	/* Offset in file. */
    quint32	sh_size;	/* Size in bytes. */
    quint32	sh_link;	/* Index of a related section. */
    quint32	sh_info;	/* Depends on section type. */
    quint32	sh_addralign;	/* Alignment in bytes. */
    quint32	sh_entsize;	/* Size of each entry in section. */
};

struct Elf64_Shdr
{
    quint32	sh_name;	/* Section name (index into the section header string table). */
    quint32	sh_type;	/* Section type. */
    quint64	sh_flags;	/* Section flags. */
    quint64	sh_addr;	/* Address in memory image. */
    quint64	sh_offset;	/* Offset in file. */
    quint64	sh_size;	/* Size in bytes. */
    quint32	sh_link;	/* Index of a related section. */
    quint32	sh_info;	/* Depends on section type. */
    quint64	sh_addralign;	/* Alignment in bytes. */
    quint64	sh_entsize;	/* Size of each entry in section. */
};

struct Elf32_Phdr
{
    quint32      p_type;         /* entry type */
    quint32      p_offset;       /* offset */
    quint32      p_vaddr;        /* virtual address */
    quint32      p_paddr;        /* physical address */
    quint32      p_filesz;       /* file size */
    quint32      p_memsz;        /* memory size */
    quint32      p_flags;        /* flags */
    quint32      p_align;        /* memory & file alignment */
};

struct Elf64_Phdr
{
    quint32      p_type;         /* entry type */
    quint32      p_flags;        /* flags */
    quint64      p_offset;       /* offset */
    quint64      p_vaddr;        /* virtual address */
    quint64      p_paddr;        /* physical address */
    quint64      p_filesz;       /* file size */
    quint64      p_memsz;        /* memory size */
    quint64      p_align;        /* memory & file alignment */
};
}
#endif // XELF_DEF_H

