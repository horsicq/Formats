// Copyright (c) 2017 hors<horsicq@gmail.com>
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
#ifndef QELF_DEF
#define QELF_DEF

#include <QtGlobal>

#define    EI_NIDENT    16

typedef quint32        Elf32_Addr;
typedef quint16        Elf32_Half;
typedef quint32        Elf32_Off;
typedef qint32         Elf32_Sword;
typedef quint32        Elf32_Word;
typedef quint32        Elf32_Size;

typedef quint64        Elf64_Addr;
typedef quint16        Elf64_Half;
typedef quint64        Elf64_Off;
typedef qint32         Elf64_Sword;
typedef quint32        Elf64_Word;
typedef quint64        Elf64_Size;

#define EI_MAG0         0
#define ELFMAG0         0x7f

#define EI_MAG1         1
#define ELFMAG1         'E'

#define EI_MAG2         2
#define ELFMAG2         'L'

#define EI_MAG3         3
#define ELFMAG3         'F'

//#define ELFMAG          "\177ELF"
#define ELFMAG          0x464C457F
#define SELFMAG         4

#define EI_CLASS        4       /* File class byte index */
#define ELFCLASSNONE    0       /* Invalid class */
#define ELFCLASS32      1       /* 32-bit objects */
#define ELFCLASS64      2       /* 64-bit objects */

#define EI_DATA         5       /* Data encodeing byte index */
#define ELFDATANONE     0       /* Invalid data encoding */
#define ELFDATA2LSB     1       /* 2's complement little endian */
#define ELFDATA2MSB     2       /* 2's complement big endian */

#define EI_VERSION      6       /* File version byte index */

#define EV_NONE         0       /* Invalid ELF Version */
#define EV_CURRENT      1       /* Current version */

#define EI_OSABI        7       /* Operating system/ABI identification */
#define EI_ABIVERSION   8       /* ABI version */

typedef struct
{
    unsigned char   e_ident[EI_NIDENT];     /* ident bytes */
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
} Elf32_Ehdr;

typedef struct
{
    unsigned char   e_ident[EI_NIDENT];     /* ident bytes */
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
} Elf64_Ehdr;

typedef struct
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
} Elf32_Shdr;

typedef struct
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
} Elf64_Shdr;

typedef struct
{
    quint32      p_type;         /* entry type */
    quint32      p_offset;       /* offset */
    quint32      p_vaddr;        /* virtual address */
    quint32      p_paddr;        /* physical address */
    quint32      p_filesz;       /* file size */
    quint32      p_memsz;        /* memory size */
    quint32      p_flags;        /* flags */
    quint32      p_align;        /* memory & file alignment */
} Elf32_Phdr;

typedef struct
{
    quint32      p_type;         /* entry type */
    quint32      p_flags;        /* flags */
    quint64      p_offset;       /* offset */
    quint64      p_vaddr;        /* virtual address */
    quint64      p_paddr;        /* physical address */
    quint64      p_filesz;       /* file size */
    quint64      p_memsz;        /* memory size */
    quint64      p_align;        /* memory & file alignment */
} Elf64_Phdr;

#endif // QELF_DEF

