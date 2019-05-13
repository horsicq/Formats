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
const quint32 S_MH_MAGIC      =0xfeedface;	/* the mach magic number */
const quint32 S_MH_CIGAM      =0xcefaedfe	;/* NXSwapInt(MH_MAGIC) */
/* Constant for the magic field of the mach_header_64 (64-bit architectures) */
const quint32 S_MH_MAGIC_64=0xfeedfacf; /* the 64-bit mach magic number */
const quint32 S_MH_CIGAM_64=0xcffaedfe; /* NXSwapInt(MH_MAGIC_64) */

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
}

#endif // QMACH_DEF_H
