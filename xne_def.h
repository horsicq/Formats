// copyright (c) 2019 hors<horsicq@gmail.com>
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
#ifndef XNE_DEF_H
#define XNE_DEF_H

#include <QtGlobal>

namespace XNE_DEF
{
const quint16 S_IMAGE_OS2_SIGNATURE     =0x454E;      // NE

struct IMAGE_OS2_HEADER  // OS/2 .EXE header
{
    quint16 ne_magic;                    // Magic number
    quint8  ne_ver;                      // Version number
    quint8  ne_rev;                      // Revision number
    quint16 ne_enttab;                   // Offset of Entry Table
    quint16 ne_cbenttab;                 // Number of bytes in Entry Table
    quint32 ne_crc;                      // Checksum of whole file
//    WORD   ne_flags;                    // Flag word
//    WORD   ne_autodata;                 // Automatic data segment number
//    WORD   ne_heap;                     // Initial heap allocation
//    WORD   ne_stack;                    // Initial stack allocation
//    LONG   ne_csip;                     // Initial CS:IP setting
//    LONG   ne_sssp;                     // Initial SS:SP setting
//    WORD   ne_cseg;                     // Count of file segments
//    WORD   ne_cmod;                     // Entries in Module Reference Table
//    WORD   ne_cbnrestab;                // Size of non-resident name table
//    WORD   ne_segtab;                   // Offset of Segment Table
//    WORD   ne_rsrctab;                  // Offset of Resource Table
//    WORD   ne_restab;                   // Offset of resident name table
//    WORD   ne_modtab;                   // Offset of Module Reference Table
//    WORD   ne_imptab;                   // Offset of Imported Names Table
//    LONG   ne_nrestab;                  // Offset of Non-resident Names Table
//    WORD   ne_cmovent;                  // Count of movable entries
//    WORD   ne_align;                    // Segment alignment shift count
//    WORD   ne_cres;                     // Count of resource segments
//    BYTE   ne_exetyp;                   // Target Operating system
//    BYTE   ne_flagsothers;              // Other .EXE flags
//    WORD   ne_pretthunks;               // offset to return thunks
//    WORD   ne_psegrefbytes;             // offset to segment ref. bytes
//    WORD   ne_swaparea;                 // Minimum code swap area size
//    WORD   ne_expver;                   // Expected Windows version number
  };
}

#endif // XNE_DEF_H
