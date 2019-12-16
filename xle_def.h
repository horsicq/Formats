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
#ifndef XLE_DEF_H
#define XLE_DEF_H

#include <QtGlobal>

namespace XLE_DEF
{
const quint32 S_IMAGE_VXD_SIGNATURE         =0x454C;      // LE

struct IMAGE_VXD_HEADER
{      // Windows VXD header
    quint32 e32_magic;                   // Magic number
//    BYTE   e32_border;                  // The byte ordering for the VXD
//    BYTE   e32_worder;                  // The word ordering for the VXD
//    DWORD  e32_level;                   // The EXE format level for now = 0
//    WORD   e32_cpu;                     // The CPU type
//    WORD   e32_os;                      // The OS type
//    DWORD  e32_ver;                     // Module version
//    DWORD  e32_mflags;                  // Module flags
//    DWORD  e32_mpages;                  // Module # pages
//    DWORD  e32_startobj;                // Object # for instruction pointer
//    DWORD  e32_eip;                     // Extended instruction pointer
//    DWORD  e32_stackobj;                // Object # for stack pointer
//    DWORD  e32_esp;                     // Extended stack pointer
//    DWORD  e32_pagesize;                // VXD page size
//    DWORD  e32_lastpagesize;            // Last page size in VXD
//    DWORD  e32_fixupsize;               // Fixup section size
//    DWORD  e32_fixupsum;                // Fixup section checksum
//    DWORD  e32_ldrsize;                 // Loader section size
//    DWORD  e32_ldrsum;                  // Loader section checksum
//    DWORD  e32_objtab;                  // Object table offset
//    DWORD  e32_objcnt;                  // Number of objects in module
//    DWORD  e32_objmap;                  // Object page map offset
//    DWORD  e32_itermap;                 // Object iterated data map offset
//    DWORD  e32_rsrctab;                 // Offset of Resource Table
//    DWORD  e32_rsrccnt;                 // Number of resource entries
//    DWORD  e32_restab;                  // Offset of resident name table
//    DWORD  e32_enttab;                  // Offset of Entry Table
//    DWORD  e32_dirtab;                  // Offset of Module Directive Table
//    DWORD  e32_dircnt;                  // Number of module directives
//    DWORD  e32_fpagetab;                // Offset of Fixup Page Table
//    DWORD  e32_frectab;                 // Offset of Fixup Record Table
//    DWORD  e32_impmod;                  // Offset of Import Module Name Table
//    DWORD  e32_impmodcnt;               // Number of entries in Import Module Name Table
//    DWORD  e32_impproc;                 // Offset of Import Procedure Name Table
//    DWORD  e32_pagesum;                 // Offset of Per-Page Checksum Table
//    DWORD  e32_datapage;                // Offset of Enumerated Data Pages
//    DWORD  e32_preload;                 // Number of preload pages
//    DWORD  e32_nrestab;                 // Offset of Non-resident Names Table
//    DWORD  e32_cbnrestab;               // Size of Non-resident Name Table
//    DWORD  e32_nressum;                 // Non-resident Name Table Checksum
//    DWORD  e32_autodata;                // Object # for automatic data object
//    DWORD  e32_debuginfo;               // Offset of the debugging information
//    DWORD  e32_debuglen;                // The length of the debugging info. in bytes
//    DWORD  e32_instpreload;             // Number of instance pages in preload section of VXD file
//    DWORD  e32_instdemand;              // Number of instance pages in demand load section of VXD file
//    DWORD  e32_heapsize;                // Size of heap - for 16-bit apps
//    BYTE   e32_res3[12];                // Reserved words
//    DWORD  e32_winresoff;
//    DWORD  e32_winreslen;
//    WORD   e32_devid;                   // Device ID for VxD
//    WORD   e32_ddkver;                  // DDK version for VxD
};
}

#endif // XLE_DEF_H
