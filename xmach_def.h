// copyright (c) 2017-2021 hors<horsicq@gmail.com>
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
#ifndef QMACH_DEF_H
#define QMACH_DEF_H

#include <QtGlobal>

namespace XMACH_DEF
{
/* Constant for the magic field of the mach_header (32-bit architectures) */
const quint32 MH_MAGIC          =0xFEEDFACE;        /* the mach magic number */
const quint32 MH_CIGAM          =0xCEFAEDFE	;       /* NXSwapInt(MH_MAGIC) */
/* Constant for the magic field of the mach_header_64 (64-bit architectures) */
const quint32 MH_MAGIC_64       =0xFEEDFACF;        /* the 64-bit mach magic number */
const quint32 MH_CIGAM_64       =0xCFFAEDFE;        /* NXSwapInt(MH_MAGIC_64) */

const quint32 FAT_MAGIC         =0xCAFEBABE;
const quint32 FAT_CIGAM         =0xBEBAFECA;

const quint32 CPU_TYPE_VAX      =0x0000001;
const quint32 CPU_TYPE_ROMP     =0x0000002;
const quint32 CPU_TYPE_NS32032  =0x0000004;
const quint32 CPU_TYPE_NS32332  =0x0000005;
const quint32 CPU_TYPE_MC680x0  =0x0000006;
const quint32 CPU_TYPE_I386     =0x0000007;
const quint32 CPU_TYPE_X86_64   =0x1000007;
const quint32 CPU_TYPE_MIPS     =0x0000008;
const quint32 CPU_TYPE_NS32532  =0x0000009;
const quint32 CPU_TYPE_HPPA     =0x000000B;
const quint32 CPU_TYPE_ARM      =0x000000C;
const quint32 CPU_TYPE_ARM64    =0x100000C;
const quint32 CPU_TYPE_MC88000  =0x000000D;
const quint32 CPU_TYPE_SPARC    =0x000000E;
const quint32 CPU_TYPE_I860     =0x000000F;
const quint32 CPU_TYPE_RS6000   =0x0000011;
const quint32 CPU_TYPE_MC98000  =0x0000012;
const quint32 CPU_TYPE_POWERPC  =0x0000012;
const quint32 CPU_TYPE_POWERPC64=0x1000012;
const quint32 CPU_TYPE_VEO      =0x00000FF;

struct fat_header
{
    quint32 magic;
    quint32 nfat_arch;
};

struct fat_arch
{
    quint32 cputype;
    quint32 cpusubtype;
    quint32 offset;
    quint32 size;
    quint32 align;
};

/*
 * The 32-bit mach header appears at the very beginning of the object file for
 * 32-bit architectures.
 */
struct mach_header
{
    quint32 magic;          /* mach magic number identifier */
    qint32 cputype;         /* cpu specifier */
    qint32 cpusubtype;      /* machine specifier */
    quint32 filetype;       /* type of file */
    quint32 ncmds;          /* number of load commands */
    quint32 sizeofcmds;     /* the size of all the load commands */
    quint32 flags;          /* flags */
};

/*
 * The 64-bit mach header appears at the very beginning of object files for
 * 64-bit architectures.
 */
struct mach_header_64
{
    quint32 magic;          /* mach magic number identifier */
    qint32 cputype;         /* cpu specifier */
    qint32 cpusubtype;      /* machine specifier */
    quint32 filetype;       /* type of file */
    quint32 ncmds;          /* number of load commands */
    quint32 sizeofcmds;     /* the size of all the load commands */
    quint32 flags;          /* flags */
    quint32 reserved;       /* reserved */
};

struct load_command
{
    quint32 cmd;
    quint32 cmdsize;
};

struct segment_command
{
    quint32 cmd;
    quint32 cmdsize;
    char segname[16];
    quint32 vmaddr;
    quint32 vmsize;
    quint32 fileoff;
    quint32 filesize;
    qint32 maxprot;
    qint32 initprot;
    quint32 nsects;
    quint32 flags;
};

struct segment_command_64
{
    quint32 cmd;
    quint32 cmdsize;
    char segname[16];
    quint64 vmaddr;
    quint64 vmsize;
    quint64 fileoff;
    quint64 filesize;
    qint32 maxprot;
    qint32 initprot;
    quint32 nsects;
    quint32 flags;
};

struct section              /* for 32-bit architectures */
{
    char sectname[16];      /* name of this section */
    char segname[16];       /* segment this section goes in */
    quint32	addr;           /* memory address of this section */
    quint32	size;           /* size in bytes of this section */
    quint32	offset;         /* file offset of this section */
    quint32	align;          /* section alignment (power of 2) */
    quint32	reloff;         /* file offset of relocation entries */
    quint32	nreloc;         /* number of relocation entries */
    quint32	flags;          /* flags (section type and attributes)*/
    quint32	reserved1;      /* reserved (for offset or index) */
    quint32	reserved2;      /* reserved (for count or sizeof) */
};

struct section_64           /* for 64-bit architectures */
{
    char sectname[16];      /* name of this section */
    char segname[16];       /* segment this section goes in */
    quint64	addr;           /* memory address of this section */
    quint64	size;           /* size in bytes of this section */
    quint32	offset;         /* file offset of this section */
    quint32	align;          /* section alignment (power of 2) */
    quint32	reloff;         /* file offset of relocation entries */
    quint32	nreloc;         /* number of relocation entries */
    quint32	flags;          /* flags (section type and attributes)*/
    quint32	reserved1;      /* reserved (for offset or index) */
    quint32	reserved2;      /* reserved (for count or sizeof) */
    quint32	reserved3;      /* reserved */
};

const quint32 x86_THREAD_STATE32                    =1;
const quint32 x86_FLOAT_STATE32                     =2;
const quint32 x86_EXCEPTION_STATE32                 =3;
const quint32 x86_THREAD_STATE64                    =4;
const quint32 x86_FLOAT_STATE64                     =5;
const quint32 x86_EXCEPTION_STATE64                 =6;
const quint32 x86_THREAD_STATE                      =7;
const quint32 x86_FLOAT_STATE                       =8;
const quint32 x86_EXCEPTION_STATE                   =9;
const quint32 x86_DEBUG_STATE32                     =10;
const quint32 x86_DEBUG_STATE64                     =11;
const quint32 x86_DEBUG_STATE                       =12;
const quint32 THREAD_STATE_NONE                     =13;

const quint32 LC_REQ_DYLD                           =0x80000000;
const quint32 LC_SEGMENT                            =0x1;           /* segment of this file to be mapped */
const quint32 LC_SYMTAB                             =0x2;           /* link-edit stab symbol table info */
const quint32 LC_SYMSEG                             =0x3;        	/* link-edit gdb symbol table info (obsolete) */
const quint32 LC_THREAD                             =0x4;	        /* thread */
const quint32 LC_UNIXTHREAD                         =0x5;	        /* unix thread (includes a stack) */
const quint32 LC_LOADFVMLIB                         =0x6;	        /* load a specified fixed VM shared library */
const quint32 LC_IDFVMLIB                           =0x7;	        /* fixed VM shared library identification */
const quint32 LC_IDENT                              =0x8;	        /* object identification info (obsolete) */
const quint32 LC_FVMFILE                            =0x9;	        /* fixed VM file inclusion (internal use) */
const quint32 LC_PREPAGE                            =0xA;           /* prepage command (internal use) */
const quint32 LC_DYSYMTAB                           =0xB;	        /* dynamic link-edit symbol table info */
const quint32 LC_LOAD_DYLIB                         =0xC;	        /* load a dynamicly linked shared library */
const quint32 LC_ID_DYLIB                           =0xD;	        /* dynamicly linked shared lib identification */
const quint32 LC_LOAD_DYLINKER                      =0xE;	        /* load a dynamic linker */
const quint32 LC_ID_DYLINKER                        =0xF;	        /* dynamic linker identification */
const quint32 LC_PREBOUND_DYLIB                     =0x10;	        /* modules prebound for a dynamicly */
const quint32 LC_ROUTINES                           =0x11;	        /* image routines */
const quint32 LC_SUB_FRAMEWORK                      =0x12;	        /* sub framework */
const quint32 LC_SUB_UMBRELLA                       =0x13;	        /* sub umbrella */
const quint32 LC_SUB_CLIENT                         =0x14;	        /* sub client */
const quint32 LC_SUB_LIBRARY                        =0x15;	        /* sub library */
const quint32 LC_TWOLEVEL_HINTS                     =0x16;	        /* two-level namespace lookup hints */
const quint32 LC_PREBIND_CKSUM                      =0x17;	        /* prebind checksum */
const quint32 LC_LOAD_WEAK_DYLIB                    =(0x18|LC_REQ_DYLD);
const quint32 LC_SEGMENT_64                         =0x19;	        /* 64-bit segment of this file to bemapped */
const quint32 LC_ROUTINES_64                        =0x1A;	        /* 64-bit image routines */
const quint32 LC_UUID                               =0x1B;	        /* the uuid */
const quint32 LC_RPATH                              =0x1C|0x80000000;
const quint32 LC_ENCRYPTION_INFO                    =0x21;
const quint32 LC_DYLD_INFO                          =0x22;
const quint32 LC_DYLD_INFO_ONLY                     =(0x22|LC_REQ_DYLD);
const quint32 LC_VERSION_MIN_MACOSX                 =0x24;
const quint32 LC_VERSION_MIN_IPHONEOS               =0x25;
const quint32 LC_SOURCE_VERSION                     =0x2A;
const quint32 LC_ENCRYPTION_INFO_64                 =0x2C;
const quint32 LC_VERSION_MIN_TVOS                   =0x2F;
const quint32 LC_VERSION_MIN_WATCHOS                =0x30;

const quint32 LC_MAIN                               =(0x28|LC_REQ_DYLD);       /* main */

const quint32 MH_OBJECT                             =0x1;		/* relocatable object file */
const quint32 MH_EXECUTE	                        =0x2;		/* demand paged executable file */
const quint32 MH_FVMLIB                             =0x3;		/* fixed VM shared library file */
const quint32 MH_CORE		                        =0x4;		/* core file */
const quint32 MH_PRELOAD	                        =0x5;		/* preloaded executable file */
const quint32 MH_DYLIB                              =0x6;		/* dynamicly bound shared library file*/
const quint32 MH_DYLINKER	                        =0x7;		/* dynamic link editor */
const quint32 MH_BUNDLE                             =0x8;		/* dynamicly bound bundle file */

struct STRUCT_X86_THREAD_STATE32
{
    quint32 eax;
    quint32 ebx;
    quint32 ecx;
    quint32 edx;
    quint32 edi;
    quint32 esi;
    quint32 ebp;
    quint32 esp;
    quint32 ss;
    quint32 eflags;
    quint32 eip;
    quint32 cs;
    quint32 ds;
    quint32 es;
    quint32 fs;
    quint32 gs;
};

struct STRUCT_X86_THREAD_STATE64
{
    quint64 rax;
    quint64 rbx;
    quint64 rcx;
    quint64 rdx;
    quint64 rdi;
    quint64 rsi;
    quint64 rbp;
    quint64 rsp;
    quint64 r8;
    quint64 r9;
    quint64 r10;
    quint64 r11;
    quint64 r12;
    quint64 r13;
    quint64 r14;
    quint64 r15;
    quint64 rip;
    quint64 rflags;
    quint64 cs;
    quint64 fs;
    quint64 gs;
};

struct entry_point_command
{
    quint32 cmd;                /* LC_MAIN only used in MH_EXECUTE filetypes */
    quint32 cmdsize;            /* 24 */
    quint64 entryoff;           /* file (__TEXT) offset of main() */
    quint64 stacksize;          /* if not zero, initial stack size */
};

struct dylib
{
    quint32 name;               // rel offset
    quint32 timestamp;
    quint32 current_version;
    quint32 compatibility_version;
};

struct dylib_command
{
    quint32 cmd;
    quint32 cmdsize;
    dylib _dylib;
};

struct dyld_info_command
{
    quint32 cmd;
    quint32 cmdsize;
    quint32 rebase_off;
    quint32 rebase_size;
    quint32 bind_off;
    quint32 bind_size;
    quint32 weak_bind_off;
    quint32 weak_bind_size;
    quint32 lazy_bind_off;
    quint32 lazy_bind_size;
    quint32 export_off;
    quint32 export_size;
};

struct uuid_command
{
    quint32 cmd;
    quint32 cmdsize;
    quint8 uuid[16];
};

struct symtab_command
{
    quint32 cmd;
    quint32 cmdsize;
    quint32 symoff;
    quint32 nsyms;
    quint32 stroff;
    quint32 strsize;
};

struct dysymtab_command
{
    quint32 cmd;
    quint32 cmdsize;
    quint32 ilocalsym;
    quint32 nlocalsym;
    quint32 iextdefsym;
    quint32 nextdefsym;
    quint32 iundefsym;
    quint32 nundefsym;
    quint32 tocoff;
    quint32 ntoc;
    quint32 modtaboff;
    quint32 nmodtab;
    quint32 extrefsymoff;
    quint32 nextrefsyms;
    quint32 indirectsymoff;
    quint32 nindirectsyms;
    quint32 extreloff;
    quint32 nextrel;
    quint32 locreloff;
    quint32 nlocrel;
};

struct version_min_command
{
    quint32 cmd;     // LC_VERSION_MIN_MACOSX or
                    // LC_VERSION_MIN_IPHONEOS
    quint32 cmdsize; // sizeof(struct version_min_command)
    quint32 version; // X.Y.Z is encoded in nibbles xxxx.yy.zz
    quint32 sdk;     // X.Y.Z is encoded in nibbles xxxx.yy.zz
};

struct dylinker_command
{
    quint32 cmd;
    quint32 cmdsize;
    quint32 name;
};

struct rpath_command
{
    quint32 cmd;
    quint32 cmdsize;
    quint32 path;
};

struct source_version_command
{
    quint32 cmd;
    quint32 cmdsize;
    quint64 version;
};

struct encryption_info_command
{
    quint32 cmd;
    quint32 cmdsize;
    quint32 cryptoff;
    quint32 cryptsize;
    quint32 cryptid;
};

struct encryption_info_command_64
{
    quint32 cmd;
    quint32 cmdsize;
    quint32 cryptoff;
    quint32 cryptsize;
    quint32 cryptid;
    quint32 pad;
};

struct dylib_module
{
    quint32 module_name;
    quint32 iextdefsym;
    quint32 nextdefsym;
    quint32 irefsym;
    quint32 nrefsym;
    quint32 ilocalsym;
    quint32 nlocalsym;
    quint32 iextrel;
    quint32 nextrel;
    quint32 iinit_iterm;
    quint32 ninit_nterm;
    quint32 objc_module_info_addr;
    quint32 objc_module_info_size;
};

struct dylib_module_64
{
    quint32 module_name;
    quint32 iextdefsym;
    quint32 nextdefsym;
    quint32 irefsym;
    quint32 nrefsym;
    quint32 ilocalsym;
    quint32 nlocalsym;
    quint32 iextrel;
    quint32 nextrel;
    quint32 iinit_iterm;
    quint32 ninit_nterm;
    quint32 objc_module_info_size;
    quint64 objc_module_info_addr;
};

struct routines_command
{
    quint32 cmd;
    quint32 cmdsize;
    quint32 init_address;
    quint32 init_module;
    quint32 reserved1;
    quint32 reserved2;
    quint32 reserved3;
    quint32 reserved4;
    quint32 reserved5;
    quint32 reserved6;
};

struct routines_command_64
{
    quint32 cmd;
    quint32 cmdsize;
    quint64 init_address;
    quint64 init_module;
    quint64 reserved1;
    quint64 reserved2;
    quint64 reserved3;
    quint64 reserved4;
    quint64 reserved5;
    quint64 reserved6;
};

// https://llvm.org/doxygen/BinaryFormat_2MachO_8h_source.html
// http://formats.kaitai.io/mach_o/
// https://gist.github.com/yamaya/2924292
// TODO code_signature_command

enum reloc_type_x86_64
{
    X86_64_RELOC_UNSIGNED,		// for absolute addresses
    X86_64_RELOC_SIGNED,		// for signed 32-bit displacement
    X86_64_RELOC_BRANCH,		// a CALL/JMP instruction with 32-bit displacement
    X86_64_RELOC_GOT_LOAD,		// a MOVQ load of a GOT entry
    X86_64_RELOC_GOT,			// other GOT references
    X86_64_RELOC_SUBTRACTOR,	// must be followed by a X86_64_RELOC_UNSIGNED
    X86_64_RELOC_SIGNED_1,		// for signed 32-bit displacement with a -1 addend
    X86_64_RELOC_SIGNED_2,		// for signed 32-bit displacement with a -2 addend
    X86_64_RELOC_SIGNED_4,		// for signed 32-bit displacement with a -4 addend
};

enum reloc_type_arm
{
    ARM_RELOC_VANILLA,	/* generic relocation as discribed above */
    ARM_RELOC_PAIR,	/* the second relocation entry of a pair */
    ARM_RELOC_SECTDIFF,	/* a PAIR follows with subtract symbol value */
    ARM_RELOC_LOCAL_SECTDIFF, /* like ARM_RELOC_SECTDIFF, but the symbol referenced was local.  */
    ARM_RELOC_PB_LA_PTR,/* prebound lazy pointer */
    ARM_RELOC_BR24,	/* 24 bit branch displacement (to a word address) */
    ARM_THUMB_RELOC_BR22, /* 22 bit branch displacement (to a half-word address) */
};

enum reloc_type_ppc
{
    PPC_RELOC_VANILLA,	/* generic relocation as discribed above */
    PPC_RELOC_PAIR,	/* the second relocation entry of a pair */
    PPC_RELOC_BR14,	/* 14 bit branch displacement (to a word address) */
    PPC_RELOC_BR24,	/* 24 bit branch displacement (to a word address) */
    PPC_RELOC_HI16,	/* a PAIR follows with the low half */
    PPC_RELOC_LO16,	/* a PAIR follows with the high half */
    PPC_RELOC_HA16,	/* Same as the RELOC_HI16 except the low 16 bits and the
             * high 16 bits are added together with the low 16 bits
             * sign extened first.  This means if bit 15 of the low
             * 16 bits is set the high 16 bits stored in the
             * instruction will be adjusted.
             */
    PPC_RELOC_LO14,	/* Same as the LO16 except that the low 2 bits are not
             * stored in the instruction and are always zero.  This
             * is used in double word load/store instructions.
             */
    PPC_RELOC_SECTDIFF,	/* a PAIR follows with subtract symbol value */
    PPC_RELOC_PB_LA_PTR,/* prebound lazy pointer */
    PPC_RELOC_HI16_SECTDIFF, /* section difference forms of above.  a PAIR */
    PPC_RELOC_LO16_SECTDIFF, /* follows these with subtract symbol value */
    PPC_RELOC_HA16_SECTDIFF,
    PPC_RELOC_JBSR,
    PPC_RELOC_LO14_SECTDIFF,
    PPC_RELOC_LOCAL_SECTDIFF  /* like PPC_RELOC_SECTDIFF, but the symbol
                 referenced was local.  */
};
}

#endif // QMACH_DEF_H
