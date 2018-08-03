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
#ifndef QPE_DEF_H
#define QPE_DEF_H

#include <QtGlobal>

#define S_IMAGE_NT_SIGNATURE 0x00004550  // PE00
#define S_IMAGE_NUMBEROF_DIRECTORY_ENTRIES 16


#define S_IMAGE_DIRECTORY_ENTRY_EXPORT          0   // Export Directory
#define S_IMAGE_DIRECTORY_ENTRY_IMPORT          1   // Import Directory
#define S_IMAGE_DIRECTORY_ENTRY_RESOURCE        2   // Resource Directory
#define S_IMAGE_DIRECTORY_ENTRY_EXCEPTION       3   // Exception Directory
#define S_IMAGE_DIRECTORY_ENTRY_SECURITY        4   // Security Directory
#define S_IMAGE_DIRECTORY_ENTRY_BASERELOC       5   // Base Relocation Table
#define S_IMAGE_DIRECTORY_ENTRY_DEBUG           6   // Debug Directory
//      S_IMAGE_DIRECTORY_ENTRY_COPYRIGHT       7   // (X86 usage)
#define S_IMAGE_DIRECTORY_ENTRY_ARCHITECTURE    7   // Architecture Specific Data
#define S_IMAGE_DIRECTORY_ENTRY_GLOBALPTR       8   // RVA of GP
#define S_IMAGE_DIRECTORY_ENTRY_TLS             9   // TLS Directory
#define S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG    10   // Load Configuration Directory
#define S_IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT   11   // Bound Import Directory in headers
#define S_IMAGE_DIRECTORY_ENTRY_IAT            12   // Import Address Table
#define S_IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT   13   // Delay Load Import Descriptors
#define S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR 14   // COM Runtime descriptor

#define S_IMAGE_SIZEOF_FILE_HEADER             20

#define S_IMAGE_FILE_RELOCS_STRIPPED           0x0001  // Relocation info stripped from file.
#define S_IMAGE_FILE_EXECUTABLE_IMAGE          0x0002  // File is executable  (i.e. no unresolved externel references).
#define S_IMAGE_FILE_LINE_NUMS_STRIPPED        0x0004  // Line nunbers stripped from file.
#define S_IMAGE_FILE_LOCAL_SYMS_STRIPPED       0x0008  // Local symbols stripped from file.
#define S_IMAGE_FILE_AGGRESIVE_WS_TRIM         0x0010  // Agressively trim working set
#define S_IMAGE_FILE_LARGE_ADDRESS_AWARE       0x0020  // App can handle >2gb addresses
#define S_IMAGE_FILE_BYTES_REVERSED_LO         0x0080  // Bytes of machine word are reversed.
#define S_IMAGE_FILE_32BIT_MACHINE             0x0100  // 32 bit word machine.
#define S_IMAGE_FILE_DEBUG_STRIPPED            0x0200  // Debugging info stripped from file in .DBG file
#define S_IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP   0x0400  // If Image is on removable media, copy and run from the swap file.
#define S_IMAGE_FILE_NET_RUN_FROM_SWAP         0x0800  // If Image is on Net, copy and run from the swap file.
#define S_IMAGE_FILE_SYSTEM                    0x1000  // System File.
#define S_IMAGE_FILE_DLL                       0x2000  // File is a DLL.
#define S_IMAGE_FILE_UP_SYSTEM_ONLY            0x4000  // File should only be run on a UP machine
#define S_IMAGE_FILE_BYTES_REVERSED_HI         0x8000  // Bytes of machine word are reversed.

#define S_IMAGE_FILE_MACHINE_UNKNOWN           0
#define S_IMAGE_FILE_MACHINE_I386              0x014c  // Intel 386.
#define S_IMAGE_FILE_MACHINE_R3000             0x0162  // MIPS little-endian, 0x160 big-endian
#define S_IMAGE_FILE_MACHINE_R4000             0x0166  // MIPS little-endian
#define S_IMAGE_FILE_MACHINE_R10000            0x0168  // MIPS little-endian
#define S_IMAGE_FILE_MACHINE_WCEMIPSV2         0x0169  // MIPS little-endian WCE v2
#define S_IMAGE_FILE_MACHINE_ALPHA             0x0184  // Alpha_AXP
#define S_IMAGE_FILE_MACHINE_SH3               0x01a2  // SH3 little-endian
#define S_IMAGE_FILE_MACHINE_SH3DSP            0x01a3
#define S_IMAGE_FILE_MACHINE_SH3E              0x01a4  // SH3E little-endian
#define S_IMAGE_FILE_MACHINE_SH4               0x01a6  // SH4 little-endian
#define S_IMAGE_FILE_MACHINE_SH5               0x01a8  // SH5
#define S_IMAGE_FILE_MACHINE_ARM               0x01c0  // ARM Little-Endian
#define S_IMAGE_FILE_MACHINE_THUMB             0x01c2
#define S_IMAGE_FILE_MACHINE_AM33              0x01d3
#define S_IMAGE_FILE_MACHINE_POWERPC           0x01F0  // IBM PowerPC Little-Endian
#define S_IMAGE_FILE_MACHINE_POWERPCFP         0x01f1
#define S_IMAGE_FILE_MACHINE_IA64              0x0200  // Intel 64
#define S_IMAGE_FILE_MACHINE_MIPS16            0x0266  // MIPS
#define S_IMAGE_FILE_MACHINE_ALPHA64           0x0284  // ALPHA64
#define S_IMAGE_FILE_MACHINE_MIPSFPU           0x0366  // MIPS
#define S_IMAGE_FILE_MACHINE_MIPSFPU16         0x0466  // MIPS
#define S_IMAGE_FILE_MACHINE_AXP64             S_IMAGE_FILE_MACHINE_ALPHA64
#define S_IMAGE_FILE_MACHINE_TRICORE           0x0520  // Infineon
#define S_IMAGE_FILE_MACHINE_CEF               0x0CEF
#define S_IMAGE_FILE_MACHINE_EBC               0x0EBC  // EFI Byte Code
#define S_IMAGE_FILE_MACHINE_AMD64             0x8664  // AMD64 (K8)
#define S_IMAGE_FILE_MACHINE_M32R              0x9041  // M32R little-endian
#define S_IMAGE_FILE_MACHINE_CEE               0xC0EE

#define S_IMAGE_SUBSYSTEM_UNKNOWN 0
#define S_IMAGE_SUBSYSTEM_NATIVE 1
#define S_IMAGE_SUBSYSTEM_WINDOWS_GUI 2
#define S_IMAGE_SUBSYSTEM_WINDOWS_CUI 3
#define S_IMAGE_SUBSYSTEM_OS2_CUI 5
#define S_IMAGE_SUBSYSTEM_POSIX_CUI 7
#define S_IMAGE_SUBSYSTEM_NATIVE_WINDOWS 8
#define S_IMAGE_SUBSYSTEM_WINDOWS_CE_GUI 9
#define S_IMAGE_SUBSYSTEM_EFI_APPLICATION 10
#define S_IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER 11
#define S_IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER 12
#define S_IMAGE_SUBSYSTEM_EFI_ROM 13
#define S_IMAGE_SUBSYSTEM_XBOX 14
#define S_IMAGE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION 16

struct S_IMAGE_FILE_HEADER
{
    quint16 Machine;
    quint16 NumberOfSections;
    quint32 TimeDateStamp;
    quint32 PointerToSymbolTable;
    quint32 NumberOfSymbols;
    quint16 SizeOfOptionalHeader;
    quint16 Characteristics;
};

struct S_IMAGE_DATA_DIRECTORY
{
    quint32 VirtualAddress;
    quint32 Size;
};

struct S_IMAGE_OPTIONAL_HEADER32
{
    //
    // Standard fields.
    //
    quint16 Magic;
    quint8  MajorLinkerVersion;
    quint8  MinorLinkerVersion;
    quint32 SizeOfCode;
    quint32 SizeOfInitializedData;
    quint32 SizeOfUninitializedData;
    quint32 AddressOfEntryPoint;
    quint32 BaseOfCode;
    quint32 BaseOfData;
    //
    // NT additional fields.
    //
    quint32 ImageBase;
    quint32 SectionAlignment;
    quint32 FileAlignment;
    quint16 MajorOperatingSystemVersion;
    quint16 MinorOperatingSystemVersion;
    quint16 MajorImageVersion;
    quint16 MinorImageVersion;
    quint16 MajorSubsystemVersion;
    quint16 MinorSubsystemVersion;
    quint32 Win32VersionValue;
    quint32 SizeOfImage;
    quint32 SizeOfHeaders;
    quint32 CheckSum;
    quint16 Subsystem;
    quint16 DllCharacteristics;
    quint32 SizeOfStackReserve;
    quint32 SizeOfStackCommit;
    quint32 SizeOfHeapReserve;
    quint32 SizeOfHeapCommit;
    quint32 LoaderFlags;
    quint32 NumberOfRvaAndSizes;
    S_IMAGE_DATA_DIRECTORY DataDirectory[S_IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
};

struct S_IMAGE_OPTIONAL_HEADER32S
{
    //
    // Standard fields.
    //
    quint16 Magic;
    quint8  MajorLinkerVersion;
    quint8  MinorLinkerVersion;
    quint32 SizeOfCode;
    quint32 SizeOfInitializedData;
    quint32 SizeOfUninitializedData;
    quint32 AddressOfEntryPoint;
    quint32 BaseOfCode;
    quint32 BaseOfData;
    //
    // NT additional fields.
    //
    quint32 ImageBase;
    quint32 SectionAlignment;
    quint32 FileAlignment;
    quint16 MajorOperatingSystemVersion;
    quint16 MinorOperatingSystemVersion;
    quint16 MajorImageVersion;
    quint16 MinorImageVersion;
    quint16 MajorSubsystemVersion;
    quint16 MinorSubsystemVersion;
    quint32 Win32VersionValue;
    quint32 SizeOfImage;
    quint32 SizeOfHeaders;
    quint32 CheckSum;
    quint16 Subsystem;
    quint16 DllCharacteristics;
    quint32 SizeOfStackReserve;
    quint32 SizeOfStackCommit;
    quint32 SizeOfHeapReserve;
    quint32 SizeOfHeapCommit;
    quint32 LoaderFlags;
    quint32 NumberOfRvaAndSizes;
};


struct S_IMAGE_OPTIONAL_HEADER64
{
    //
    // Standard fields.
    //
    quint16 Magic;
    quint8  MajorLinkerVersion;
    quint8  MinorLinkerVersion;
    quint32 SizeOfCode;
    quint32 SizeOfInitializedData;
    quint32 SizeOfUninitializedData;
    quint32 AddressOfEntryPoint;
    quint32 BaseOfCode;
    //
    // NT additional fields.
    //
    qint64 ImageBase;
    quint32 SectionAlignment;
    quint32 FileAlignment;
    quint16 MajorOperatingSystemVersion;
    quint16 MinorOperatingSystemVersion;
    quint16 MajorImageVersion;
    quint16 MinorImageVersion;
    quint16 MajorSubsystemVersion;
    quint16 MinorSubsystemVersion;
    quint32 Win32VersionValue;
    quint32 SizeOfImage;
    quint32 SizeOfHeaders;
    quint32 CheckSum;
    quint16 Subsystem;
    quint16 DllCharacteristics;
    qint64 SizeOfStackReserve;
    qint64 SizeOfStackCommit;
    qint64 SizeOfHeapReserve;
    qint64 SizeOfHeapCommit;
    quint32 LoaderFlags;
    quint32 NumberOfRvaAndSizes;
    S_IMAGE_DATA_DIRECTORY DataDirectory[S_IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
};

struct S_IMAGE_OPTIONAL_HEADER64S
{
    //
    // Standard fields.
    //
    quint16 Magic;
    quint8  MajorLinkerVersion;
    quint8  MinorLinkerVersion;
    quint32 SizeOfCode;
    quint32 SizeOfInitializedData;
    quint32 SizeOfUninitializedData;
    quint32 AddressOfEntryPoint;
    quint32 BaseOfCode;
    //
    // NT additional fields.
    //
    qint64 ImageBase;
    quint32 SectionAlignment;
    quint32 FileAlignment;
    quint16 MajorOperatingSystemVersion;
    quint16 MinorOperatingSystemVersion;
    quint16 MajorImageVersion;
    quint16 MinorImageVersion;
    quint16 MajorSubsystemVersion;
    quint16 MinorSubsystemVersion;
    quint32 Win32VersionValue;
    quint32 SizeOfImage;
    quint32 SizeOfHeaders;
    quint32 CheckSum;
    quint16 Subsystem;
    quint16 DllCharacteristics;
    qint64 SizeOfStackReserve;
    qint64 SizeOfStackCommit;
    qint64 SizeOfHeapReserve;
    qint64 SizeOfHeapCommit;
    quint32 LoaderFlags;
    quint32 NumberOfRvaAndSizes;
};

struct S_IMAGE_NT_HEADERS64
{
    quint32 Signature;
    S_IMAGE_FILE_HEADER FileHeader;
    S_IMAGE_OPTIONAL_HEADER64 OptionalHeader;
};

struct S_IMAGE_NT_HEADERS32
{
    quint32 Signature;
    S_IMAGE_FILE_HEADER FileHeader;
    S_IMAGE_OPTIONAL_HEADER32 OptionalHeader;
};

#define S_IMAGE_SIZEOF_SHORT_NAME 8

#define S_IMAGE_SCN_MEM_EXECUTE 0x20000000
#define S_IMAGE_SCN_MEM_READ 0x40000000
#define S_IMAGE_SCN_MEM_WRITE 0x80000000

struct S_IMAGE_SECTION_HEADER
{
    quint8  Name[S_IMAGE_SIZEOF_SHORT_NAME];
    union
    {
        quint32 PhysicalAddress;
        quint32 VirtualSize;
    } Misc;
    quint32 VirtualAddress;
    quint32 SizeOfRawData;
    quint32 PointerToRawData;
    quint32 PointerToRelocations;
    quint32 PointerToLinenumbers;
    quint16 NumberOfRelocations;
    quint16 NumberOfLinenumbers;
    quint32 Characteristics;
};

struct S_IMAGE_RESOURCE_DIRECTORY
{
    quint32 Characteristics;
    quint32 TimeDateStamp;
    quint16 MajorVersion;
    quint16 MinorVersion;
    quint16 NumberOfNamedEntries;
    quint16 NumberOfIdEntries;
};

struct S_IMAGE_RESOURCE_DIRECTORY_ENTRY
{
    union
    {
        struct
        {
            quint32 NameOffset:31;
            quint32 NameIsString:1;
        };
        quint32 Name;
        quint16 Id;
    };
    union
    {
        quint32 OffsetToData;
        struct
        {
            quint32 OffsetToDirectory:31;
            quint32 DataIsDirectory:1;
        };
    };
};

struct S_IMAGE_RESOURCE_DATA_ENTRY
{
    quint32 OffsetToData;
    quint32 Size;
    quint32 CodePage;
    quint32 Reserved;
};

//
// Resource Format.
//
/* Predefined resource types */
#define    S_RT_NEWRESOURCE      0x2000
#define    S_RT_ERROR            0x7fff
#define    S_RT_CURSOR           1
#define    S_RT_BITMAP           2
#define    S_RT_ICON             3
#define    S_RT_MENU             4
#define    S_RT_DIALOG           5
#define    S_RT_STRING           6
#define    S_RT_FONTDIR          7
#define    S_RT_FONT             8
#define    S_RT_ACCELERATORS     9
#define    S_RT_RCDATA           10
#define    S_RT_MESSAGETABLE     11
#define    S_RT_GROUP_CURSOR     12
#define    S_RT_GROUP_ICON       14
#define    S_RT_VERSION          16
#define    S_RT_NEWBITMAP        (S_RT_BITMAP|S_RT_NEWRESOURCE)
#define    S_RT_NEWMENU          (S_RT_MENU|S_RT_NEWRESOURCE)
#define    S_RT_NEWDIALOG        (S_RT_DIALOG|S_RT_NEWRESOURCE)
#define    S_RT_MANIFEST         24

struct S_IMAGE_IMPORT_DESCRIPTOR
{
    union
    {
        quint32   Characteristics;
        quint32   OriginalFirstThunk;
    };
    quint32   TimeDateStamp;
    quint32   ForwarderChain;
    quint32   Name;
    quint32   FirstThunk;
};

struct S_IMAGE_IMPORT_DESCRIPTOR_EX
{
    union
    {
        quint32   Characteristics;
        quint32   OriginalFirstThunk;
    };
    quint32   TimeDateStamp;
    quint32   ForwarderChain;
    quint32   Name;
    quint32   FirstThunk;
    //
    QString sLibrary;
};

//struct _VS_VERSION_INFO
//{
//    quint16 wLength;             /* Length of the version resource */
//    quint16 wValueLength;        /* Length of the value field for this block */
//    quint16 wType;
//};

//struct _VS_FIXEDFILEINFO
//{
//    quint32 dwSignature;        // e.g.  0xfeef04bd
//    quint32 dwStrucVersion;     // e.g.  0x00000042 = "0.42"
//    quint32 dwFileVersionMS;    // e.g.  0x00030075 = "3.75"
//    quint32 dwFileVersionLS;    // e.g.  0x00000031 = "0.31"
//    quint32 dwProductVersionMS; // e.g.  0x00030010 = "3.10"
//    quint32 dwProductVersionLS; // e.g.  0x00000031 = "0.31"
//    quint32 dwFileFlagsMask;    // = 0x3F for version "0.42"
//    quint32 dwFileFlags;        // e.g.  VFF_DEBUG | VFF_PRERELEASE
//    quint32 dwFileOS;           // e.g.  VOS_DOS_WINDOWS16
//    quint32 dwFileType;         // e.g.  VFT_DRIVER
//    quint32 dwFileSubtype;      // e.g.  VFT2_DRV_KEYBOARD
//    quint32 dwFileDateMS;       // e.g.  0
//    quint32 dwFileDateLS;       // e.g.  0
//};

//struct _S_IMAGE_COR20_HEADER
//{
//    // Header versioning
//    quint32                   cb;
//    quint16                    MajorRuntimeVersion;
//    quint16                    MinorRuntimeVersion;
//    // Symbol table and startup information
//    S_IMAGE_DATA_DIRECTORY    MetaData;
//    quint32                   Flags;
//    // DDBLD - Added next section to replace following lin
//    // DDBLD - Still verifying, since not in NT SDK
//    // DWORD                   EntryPointToken;
//    // If COMS_IMAGE_FLAGS_NATIVE_ENTRYPOINT is not set,
//    // EntryPointToken represents a managed entrypoint.
//    // If COMS_IMAGE_FLAGS_NATIVE_ENTRYPOINT is set,
//    // EntryPointRVA represents an RVA to a native entrypoint.
//    union
//    {
//        quint32               EntryPointToken;
//        quint32               EntryPointRVA;
//    };
//    // DDBLD - End of Added Area
//    // Binding information
//    S_IMAGE_DATA_DIRECTORY    Resources;
//    S_IMAGE_DATA_DIRECTORY    StrongNameSignature;
//    // Regular fixup and binding information
//    S_IMAGE_DATA_DIRECTORY    CodeManagerTable;
//    S_IMAGE_DATA_DIRECTORY    VTableFixups;
//    S_IMAGE_DATA_DIRECTORY    ExportAddressTableJumps;
//    // Precompiled image info (internal use only - set to zero)
//    S_IMAGE_DATA_DIRECTORY    ManagedNativeHeader;

//};

struct S_IMAGE_BASE_RELOCATION
{
    quint32   VirtualAddress;
    quint32   SizeOfBlock;
    // WORD    TypeOffset[1];
};

struct S_IMAGE_EXPORT_DIRECTORY
{
    quint32   Characteristics;
    quint32   TimeDateStamp;
    quint16   MajorVersion;
    quint16   MinorVersion;
    quint32   Name;
    quint32   Base;
    quint32   NumberOfFunctions;
    quint32   NumberOfNames;
    quint32   AddressOfFunctions;     // RVA from base of image
    quint32   AddressOfNames;         // RVA from base of image
    quint32   AddressOfNameOrdinals;  // RVA from base of image
};

struct S_SYSTEM_INFO32
{
    union
    {
        quint32 dwOemId;
        struct
        {
            quint16 wProcessorArchitecture;
            quint16 wReserved;
        };
    };
    quint32 dwPageSize;
    quint32 lpMinimumApplicationAddress;
    quint32 lpMaximumApplicationAddress;
    quint32 dwActiveProcessorMask;
    quint32 dwNumberOfProcessors;
    quint32 dwProcessorType;
    quint32 dwAllocationGranularity;
    quint16 wProcessorLevel;
    quint16 wProcessorRevision;
};

struct S_SYSTEM_INFO64
{
    union
    {
        quint32 dwOemId;
        struct
        {
            quint16 wProcessorArchitecture;
            quint16 wReserved;
        };
    };
    quint32 dwPageSize;
    quint64 lpMinimumApplicationAddress;
    quint64 lpMaximumApplicationAddress;
    quint32 dwActiveProcessorMask;
    quint32 dwNumberOfProcessors;
    quint32 dwProcessorType;
    quint32 dwAllocationGranularity;
    quint16 wProcessorLevel;
    quint16 wProcessorRevision;
};

struct S_IMAGE_COR20_HEADER
{
    // Header versioning

    quint32                   cb;
    quint16                    MajorRuntimeVersion;
    quint16                    MinorRuntimeVersion;
    // Symbol table and startup information

    S_IMAGE_DATA_DIRECTORY    MetaData;
    quint32                   Flags;
    // DDBLD - Added next section to replace following lin

    // DDBLD - Still verifying, since not in NT SDK

    // unsigned int                   EntryPointToken;

    // If COMIMAGE_FLAGS_NATIVE_ENTRYPOINT is not set,

    // EntryPointToken represents a managed entrypoint.

    // If COMIMAGE_FLAGS_NATIVE_ENTRYPOINT is set,

    // EntryPointRVA represents an RVA to a native entrypoint.

    union
    {
        quint32               EntryPointToken;
        quint32               EntryPointRVA;
    };
    // DDBLD - End of Added Area


    // Binding information

    S_IMAGE_DATA_DIRECTORY    Resources;
    S_IMAGE_DATA_DIRECTORY    StrongNameSignature;

    // Regular fixup and binding information

    S_IMAGE_DATA_DIRECTORY    CodeManagerTable;
    S_IMAGE_DATA_DIRECTORY    VTableFixups;
    S_IMAGE_DATA_DIRECTORY    ExportAddressTableJumps;

    // Precompiled image info (internal use only - set to zero)

    S_IMAGE_DATA_DIRECTORY    ManagedNativeHeader;

};

enum S_ReplacesCorHdrNumericDefines
{
    // COM+ Header entry point flags.
    S_COMIMAGE_FLAGS_ILONLY               =0x00000001,
    S_COMIMAGE_FLAGS_32BITREQUIRED        =0x00000002,
    S_COMIMAGE_FLAGS_IL_LIBRARY           =0x00000004,
    S_COMIMAGE_FLAGS_STRONGNAMESIGNED     =0x00000008,
    S_COMIMAGE_FLAGS_NATIVE_ENTRYPOINT    =0x00000010,
    S_COMIMAGE_FLAGS_TRACKDEBUGDATA       =0x00010000,

    // Version flags for image.
    S_COR_VERSION_MAJOR_V2                =2,
    S_COR_VERSION_MAJOR                   =S_COR_VERSION_MAJOR_V2,
    S_COR_VERSION_MINOR                   =5,
    S_COR_DELETED_NAME_LENGTH             =8,
    S_COR_VTABLEGAP_NAME_LENGTH           =8,

    // Maximum size of a NativeType descriptor.
    S_NATIVE_TYPE_MAX_CB                  =1,
    S_COR_ILMETHOD_SECT_SMALL_MAX_DATASIZE=0xFF,

    // #defines for the MIH FLAGS
    S_IMAGE_COR_MIH_METHODRVA             =0x01,
    S_IMAGE_COR_MIH_EHRVA                 =0x02,
    S_IMAGE_COR_MIH_BASICBLOCK            =0x08,

    // V-table constants
    S_COR_VTABLE_32BIT                    =0x01,          // V-table slots are 32-bits in size.
    S_COR_VTABLE_64BIT                    =0x02,          // V-table slots are 64-bits in size.
    S_COR_VTABLE_FROM_UNMANAGED           =0x04,          // If set, transition from unmanaged.
    S_COR_VTABLE_FROM_UNMANAGED_RETAIN_APPDOMAIN  =0x08,  // If set, transition from unmanaged with keeping the current appdomain.
    S_COR_VTABLE_CALL_MOST_DERIVED        =0x10,          // Call most derived method described by

    // EATJ constants
    S_IMAGE_COR_EATJ_THUNK_SIZE           =32,            // Size of a jump thunk reserved range.

    // Max name lengths
    //@todo: Change to unlimited name lengths.
    S_MAX_CLASS_NAME                      =1024,
    S_MAX_PACKAGE_NAME                    =1024,
};


struct S_VS_VERSION_INFO
{
    quint16 wLength;             /* Length of the version resource */
    quint16 wValueLength;        /* Length of the value field for this block */
    quint16 wType;
};

struct S__tagVS_FIXEDFILEINFO
{
    quint32 dwSignature;        // e.g.  0xfeef04bd
    quint32 dwStrucVersion;     // e.g.  0x00000042 = "0.42"
    quint32 dwFileVersionMS;    // e.g.  0x00030075 = "3.75"
    quint32 dwFileVersionLS;    // e.g.  0x00000031 = "0.31"
    quint32 dwProductVersionMS; // e.g.  0x00030010 = "3.10"
    quint32 dwProductVersionLS; // e.g.  0x00000031 = "0.31"
    quint32 dwFileFlagsMask;    // = 0x3F for version "0.42"
    quint32 dwFileFlags;        // e.g.  VFF_DEBUG | VFF_PRERELEASE
    quint32 dwFileOS;           // e.g.  VOS_DOS_WINDOWS16
    quint32 dwFileType;         // e.g.  VFT_DRIVER
    quint32 dwFileSubtype;      // e.g.  VFT2_DRV_KEYBOARD
    quint32 dwFileDateMS;       // e.g.  0
    quint32 dwFileDateLS;       // e.g.  0
};

//typedef VOID (NTAPI *PIMAGE_TLS_CALLBACK)(PVOID DllHandle,DWORD Reason,PVOID Reserved);

struct S_IMAGE_TLS_DIRECTORY64
{
    quint64 StartAddressOfRawData;
    quint64 EndAddressOfRawData;
    quint64 AddressOfIndex;
    quint64 AddressOfCallBacks;
    quint32 SizeOfZeroFill;
    quint32 Characteristics;
};

struct S_IMAGE_TLS_DIRECTORY32
{
    quint32 StartAddressOfRawData;
    quint32 EndAddressOfRawData;
    quint32 AddressOfIndex;
    quint32 AddressOfCallBacks;
    quint32 SizeOfZeroFill;
    quint32 Characteristics;
};

#endif // QPE_DEF_H

