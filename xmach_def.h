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
#ifndef QMACH_DEF_H
#define QMACH_DEF_H

#include <QtGlobal>

namespace XMACH_DEF
{
/* Constant for the magic field of the mach_header (32-bit architectures) */
const quint32 S_MH_MAGIC        =0xfeedface;	/* the mach magic number */
const quint32 S_MH_CIGAM        =0xcefaedfe	;/* NXSwapInt(MH_MAGIC) */
/* Constant for the magic field of the mach_header_64 (64-bit architectures) */
const quint32 S_MH_MAGIC_64     =0xfeedfacf; /* the 64-bit mach magic number */
const quint32 S_MH_CIGAM_64     =0xcffaedfe; /* NXSwapInt(MH_MAGIC_64) */

/*
 * The 32-bit mach header appears at the very beginning of the object file for
 * 32-bit architectures.
 */
struct mach_header
{
    quint32	magic;		/* mach magic number identifier */
    qint32	cputype;	/* cpu specifier */
    qint32	cpusubtype;	/* machine specifier */
    quint32	filetype;	/* type of file */
    quint32	ncmds;		/* number of load commands */
    quint32	sizeofcmds;	/* the size of all the load commands */
    quint32	flags;		/* flags */
};

/*
 * The 64-bit mach header appears at the very beginning of object files for
 * 64-bit architectures.
 */
struct mach_header_64
{
    quint32	magic;		/* mach magic number identifier */
    qint32	cputype;	/* cpu specifier */
    qint32	cpusubtype;	/* machine specifier */
    quint32	filetype;	/* type of file */
    quint32	ncmds;		/* number of load commands */
    quint32	sizeofcmds;	/* the size of all the load commands */
    quint32	flags;		/* flags */
    quint32	reserved;	/* reserved */
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

const quint32 S_x86_THREAD_STATE32=1;
const quint32 S_x86_FLOAT_STATE32=2;
const quint32 S_x86_EXCEPTION_STATE32=3;
const quint32 S_x86_THREAD_STATE64=4;
const quint32 S_x86_FLOAT_STATE64=5;
const quint32 S_x86_EXCEPTION_STATE64=6;
const quint32 S_x86_THREAD_STATE=7;
const quint32 S_x86_FLOAT_STATE=8;
const quint32 S_x86_EXCEPTION_STATE=9;
const quint32 S_x86_DEBUG_STATE32=10;
const quint32 S_x86_DEBUG_STATE64=11;
const quint32 S_x86_DEBUG_STATE=12;
const quint32 S_THREAD_STATE_NONE=13;


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
    quint32  cmd;    /* LC_MAIN only used in MH_EXECUTE filetypes */
    quint32  cmdsize;    /* 24 */
    quint64  entryoff;    /* file (__TEXT) offset of main() */
    quint64  stacksize;/* if not zero, initial stack size */
};

struct dylib
{
    quint32 name; // rel offset
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
}

#endif // QMACH_DEF_H
