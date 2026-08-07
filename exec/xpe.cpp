/* Copyright (c) 2017-2026 hors<horsicq@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "xpe.h"
#include "xcliassembly.h"
#include <limits>


XBinary::XCONVERT _TABLE_XPE_STRUCTID[] = {
    {XPE::STRUCTID_UNKNOWN, "Unknown", QObject::tr("Unknown")},
    {XPE::STRUCTID_IMAGE_DOS_HEADER, "IMAGE_DOS_HEADER", QString("IMAGE_DOS_HEADER")},
    {XPE::STRUCTID_IMAGE_NT_HEADERS32, "IMAGE_NT_HEADERS32", QString("IMAGE_NT_HEADERS32")},
    {XPE::STRUCTID_IMAGE_NT_HEADERS64, "IMAGE_NT_HEADERS64", QString("IMAGE_NT_HEADERS64")},
    {XPE::STRUCTID_IMAGE_FILE_HEADER, "IMAGE_FILE_HEADER", QString("IMAGE_FILE_HEADER")},
    {XPE::STRUCTID_IMAGE_OPTIONAL_HEADER32, "IMAGE_OPTIONAL_HEADER32", QString("IMAGE_OPTIONAL_HEADER32")},
    {XPE::STRUCTID_IMAGE_OPTIONAL_HEADER64, "IMAGE_OPTIONAL_HEADER64", QString("IMAGE_OPTIONAL_HEADER64")},
    {XPE::STRUCTID_IMAGE_SECTION_HEADER, "IMAGE_SECTION_HEADER", QString("IMAGE_SECTION_HEADER")},
    {XPE::STRUCTID_IMAGE_DATA_DIRECTORY, "IMAGE_DATA_DIRECTORY", QString("IMAGE_DATA_DIRECTORY")},
    {XPE::STRUCTID_IMAGE_RESOURCE_DIRECTORY, "IMAGE_RESOURCE_DIRECTORY", QString("IMAGE_RESOURCE_DIRECTORY")},
    {XPE::STRUCTID_IMAGE_EXPORT_DIRECTORY, "IMAGE_EXPORT_DIRECTORY", QString("IMAGE_EXPORT_DIRECTORY")},
    {XPE::STRUCTID_IMAGE_EXPORT_FUNCTION, "IMAGE_EXPORT_FUNCTION", QString("IMAGE_EXPORT_FUNCTION")},
    {XPE::STRUCTID_IMAGE_IMPORT_DESCRIPTOR, "IMAGE_IMPORT_DESCRIPTOR", QString("IMAGE_IMPORT_DESCRIPTOR")},
    {XPE::STRUCTID_IMAGE_THUNK_DATA32, "IMAGE_THUNK_DATA32", QString("IMAGE_THUNK_DATA32")},
    {XPE::STRUCTID_IMAGE_THUNK_DATA64, "IMAGE_THUNK_DATA64", QString("IMAGE_THUNK_DATA64")},
    {XPE::STRUCTID_IMAGE_DELAYLOAD_DESCRIPTOR, "IMAGE_DELAYLOAD_DESCRIPTOR", QString("IMAGE_DELAYLOAD_DESCRIPTOR")},
    {XPE::STRUCTID_IMAGE_BOUND_IMPORT_DESCRIPTOR, "IMAGE_BOUND_IMPORT_DESCRIPTOR", QString("IMAGE_BOUND_IMPORT_DESCRIPTOR")},
    {XPE::STRUCTID_IMAGE_DEBUG_DIRECTORY, "IMAGE_DEBUG_DIRECTORY", QString("IMAGE_DEBUG_DIRECTORY")},
    {XPE::STRUCTID_IMAGE_TLS_DIRECTORY32, "IMAGE_TLS_DIRECTORY32", QString("IMAGE_TLS_DIRECTORY32")},
    {XPE::STRUCTID_IMAGE_TLS_DIRECTORY64, "IMAGE_TLS_DIRECTORY64", QString("IMAGE_TLS_DIRECTORY64")},
    {XPE::STRUCTID_IMAGE_LOAD_CONFIG_DIRECTORY32, "IMAGE_LOAD_CONFIG_DIRECTORY32", QString("IMAGE_LOAD_CONFIG_DIRECTORY32")},
    {XPE::STRUCTID_IMAGE_LOAD_CONFIG_DIRECTORY64, "IMAGE_LOAD_CONFIG_DIRECTORY64", QString("IMAGE_LOAD_CONFIG_DIRECTORY64")},
    {XPE::STRUCTID_IMAGE_RUNTIME_FUNCTION_ENTRY, "IMAGE_RUNTIME_FUNCTION_ENTRY", QString("IMAGE_RUNTIME_FUNCTION_ENTRY")},
    {XPE::STRUCTID_WIN_CERT_RECORD, "WIN_CERT_RECORD", QString("WIN_CERT_RECORD")},
    {XPE::STRUCTID_IMAGE_COR20_HEADER, "IMAGE_COR20_HEADER", QString("IMAGE_COR20_HEADER")},
    {XPE::STRUCTID_NET_METADATA, "NET_METADATA", QString(".NET MetaData")},
    {XPE::STRUCTID_NET_METADATA_STREAM, "NET_METADATA_STREAM", QString(".NET MetaData Stream")},
    {XPE::STRUCTID_NET_RESOURCES, "NET_RESOURCES", QString(".NET Resources")},
    {XPE::STRUCTID_NET_STRONGNAMESIGNATURE, "NET_STRONGNAMESIGNATURE", QString(".NET StrongNameSignature")},
    {XPE::STRUCTID_NET_CODEMANAGERTABLE, "NET_CODEMANAGERTABLE", QString(".NET CodeManagerTable")},
    {XPE::STRUCTID_NET_VTABLEFIXUPS, "NET_VTABLEFIXUPS", QString(".NET VTableFixups")},
    {XPE::STRUCTID_NET_EXPORTADDRESSTABLEJUMPS, "NET_EXPORTADDRESSTABLEJUMPS", QString(".NET ExportAddressTableJumps")},
    {XPE::STRUCTID_NET_MANAGEDNATIVEHEADER, "NET_MANAGEDNATIVEHEADER", QString(".NET ManagedNativeHeader")},
    {XPE::STRUCTID_IMAGE_RESOURCE_DIRECTORY_ENTRY, "IMAGE_RESOURCE_DIRECTORY_ENTRY", QString("IMAGE_RESOURCE_DIRECTORY_ENTRY")},
    {XPE::STRUCTID_IMAGE_RESOURCE_DATA_ENTRY, "IMAGE_RESOURCE_DATA_ENTRY", QString("IMAGE_RESOURCE_DATA_ENTRY")},
    {XPE::STRUCTID_IMAGE_BASE_RELOCATION, "IMAGE_BASE_RELOCATION", QString("IMAGE_BASE_RELOCATION")},
    {XPE::STRUCTID_IMAGE_BASE_RELOCATION_ENTRY, "IMAGE_BASE_RELOCATION_ENTRY", QString("IMAGE_BASE_RELOCATION_ENTRY")},
    {XPE::STRUCTID_IMAGE_BOUND_FORWARDER_REF, "IMAGE_BOUND_FORWARDER_REF", QString("IMAGE_BOUND_FORWARDER_REF")},
    {XPE::STRUCTID_IMAGE_RUNTIME_FUNCTION_ENTRY_ARM, "IMAGE_RUNTIME_FUNCTION_ENTRY_ARM", QString("IMAGE_RUNTIME_FUNCTION_ENTRY_ARM")},
    {XPE::STRUCTID_IMAGE_RUNTIME_FUNCTION_ENTRY_ALPHA, "IMAGE_RUNTIME_FUNCTION_ENTRY_ALPHA", QString("IMAGE_RUNTIME_FUNCTION_ENTRY_ALPHA")},
    {XPE::STRUCTID_IMAGE_RUNTIME_FUNCTION_ENTRY_ALPHA64, "IMAGE_RUNTIME_FUNCTION_ENTRY_ALPHA64", QString("IMAGE_RUNTIME_FUNCTION_ENTRY_ALPHA64")},
    {XPE::STRUCTID_IMAGE_EXCEPTION_DIRECTORY_RAW, "IMAGE_EXCEPTION_DIRECTORY_RAW", QString("IMAGE_EXCEPTION_DIRECTORY_RAW")},
};

XBinary::XIDSTRING _TABLE_XPE_ImageNtHeadersSignatures[] = {
    {0x00004550, "NT_SIGNATURE"},
};

XBinary::XIDSTRING _TABLE_XPE_ImageMagics[] = {
    {0x5A4D, "DOS_SIGNATURE"},
};

XBinary::XIDSTRING _TABLE_XPE_ImageFileHeaderMachines[] = {
    {0, "UNKNOWN"},        {0x014c, "I386"},      {0x014d, "I486"},     {0x014e, "PENTIUM"},     {0x0160, "R3000_BE"},
    {0x0162, "R3000"},     {0x0166, "R4000"},     {0x0168, "R10000"},   {0x0169, "WCEMIPSV2"},   {0x0184, "ALPHA"},
    {0x01F0, "POWERPC"},   {0x01a2, "SH3"},       {0x01a3, "SH3DSP"},   {0x01a4, "SH3E"},        {0x01a6, "SH4"},
    {0x01a8, "SH5"},       {0x01c0, "ARM"},       {0x01c2, "THUMB"},    {0x01c4, "ARMNT"},       {0x01d3, "AM33"},
    {0x01f1, "POWERPCFP"}, {0x01f2, "POWERPCBE"}, {0x0200, "IA64"},     {0x0266, "MIPS16"},      {0x0284, "ALPHA64"},
    {0x0366, "MIPSFPU"},   {0x0466, "MIPSFPU16"}, {0x0520, "TRICORE"},  {0x0CEF, "CEF"},         {0x0EBC, "EBC"},
    {0x5032, "RISCV32"},   {0x5064, "RISCV64"},   {0x5128, "RISCV128"}, {0x6232, "LOONGARCH32"}, {0x6264, "LOONGARCH64"},
    {0x8664, "AMD64"},     {0x9041, "M32R"},      {0xAA64, "ARM64"},    {0xC0EE, "CEE"},         {0xfd1d, "AMD64_LINUX_NI"},
};

XBinary::XIDSTRING _TABLE_XPE_ImageFileHeaderCharacteristics[] = {
    {0x0001, "RELOCS_STRIPPED"},
    {0x0002, "EXECUTABLE_IMAGE"},
    {0x0004, "LINE_NUMS_STRIPPED"},
    {0x0008, "LOCAL_SYMS_STRIPPED"},
    {0x0010, "AGGRESIVE_WS_TRIM"},
    {0x0020, "LARGE_ADDRESS_AWARE"},
    {0x0080, "BYTES_REVERSED_LO"},
    {0x0100, "32BIT_MACHINE"},
    {0x0200, "DEBUG_STRIPPED"},
    {0x0400, "REMOVABLE_RUN_FROM_SWAP"},
    {0x0800, "NET_RUN_FROM_SWAP"},
    {0x1000, "SYSTEM"},
    {0x2000, "DLL"},
    {0x4000, "UP_SYSTEM_ONLY"},
    {0x8000, "BYTES_REVERSED_HI"},
};

XBinary::XIDSTRING _TABLE_XPE_ImageOptionalHeaderMagic[] = {
    {0x10b, "NT_OPTIONAL_HDR32_MAGIC"},
    {0x20b, "NT_OPTIONAL_HDR64_MAGIC"},
    {0x107, "ROM_OPTIONAL_HDR_MAGIC"},
};

XBinary::XIDSTRING _TABLE_XPE_ImageOptionalHeaderSubsystem[] = {
    {0, "UNKNOWN"},
    {1, "NATIVE"},
    {2, "WINDOWS_GUI"},
    {3, "WINDOWS_CUI"},
    {5, "OS2_CUI"},
    {7, "POSIX_CUI"},
    {8, "NATIVE_WINDOWS"},
    {9, "WINDOWS_CE_GUI"},
    {10, "EFI_APPLICATION"},
    {11, "EFI_BOOT_SERVICE_DRIVER"},
    {12, "EFI_RUNTIME_DRIVER"},
    {13, "EFI_ROM"},
    {14, "XBOX"},
    {16, "WINDOWS_BOOT_APPLICATION"},
};

XBinary::XIDSTRING _TABLE_XPE_ImageOptionalHeaderDllCharacteristics[] = {
    {0x0020, "HIGH_ENTROPY_VA"}, {0x0040, "DYNAMIC_BASE"}, {0x0080, "FORCE_INTEGRITY"}, {0x0100, "NX_COMPAT"}, {0x0200, "NO_ISOLATION"},          {0x0400, "NO_SEH"},
    {0x0800, "NO_BIND"},         {0x1000, "APPCONTAINER"}, {0x2000, "WDM_DRIVER"},      {0x4000, "GUARD_CF"},  {0x8000, "TERMINAL_SERVER_AWARE"},
};

XBinary::XIDSTRING _TABLE_XPE_ImageOptionalHeaderDataDirectory[] = {
    {0, "EXPORT"},    {1, "IMPORT"}, {2, "RESOURCE"},     {3, "EXCEPTION"},     {4, "SECURITY"}, {5, "BASERELOC"},     {6, "DEBUG"},           {7, "ARCHITECTURE"},
    {8, "GLOBALPTR"}, {9, "TLS"},    {10, "LOAD_CONFIG"}, {11, "BOUND_IMPORT"}, {12, "IAT"},     {13, "DELAY_IMPORT"}, {14, "COM_DESCRIPTOR"}, {15, "RESERVED"},
};

XBinary::XIDSTRING _TABLE_XPE_ImageSectionHeaderFlags[] = {
    {0x00000008, "TYPE_NO_PAD"},       {0x00000020, "CNT_CODE"},      {0x00000040, "CNT_INITIALIZED_DATA"}, {0x00000080, "CNT_UNINITIALIZED_DATA"},
    {0x00000100, "LNK_OTHER"},         {0x00000200, "LNK_INFO"},      {0x00000800, "LNK_REMOVE"},           {0x00001000, "LNK_COMDAT"},
    {0x00004000, "NO_DEFER_SPEC_EXC"}, {0x00008000, "GPREL"},         {0x00020000, "MEM_PURGEABLE"},        {0x00020000, "MEM_16BIT"},
    {0x00040000, "MEM_LOCKED"},        {0x00080000, "MEM_PRELOAD"},   {0x01000000, "LNK_NRELOC_OVFL"},      {0x02000000, "MEM_DISCARDABLE"},
    {0x04000000, "MEM_NOT_CACHED"},    {0x08000000, "MEM_NOT_PAGED"}, {0x10000000, "MEM_SHARED"},           {0x20000000, "MEM_EXECUTE"},
    {0x40000000, "MEM_READ"},          {0x80000000, "MEM_WRITE"},
};

XBinary::XIDSTRING _TABLE_XPE_ImageSectionHeaderAligns[] = {
    {0x00100000, "1BYTES"},    {0x00200000, "2BYTES"},    {0x00300000, "4BYTES"},    {0x00400000, "8BYTES"},    {0x00500000, "16BYTES"},
    {0x00600000, "32BYTES"},   {0x00700000, "64BYTES"},   {0x00800000, "128BYTES"},  {0x00900000, "256BYTES"},  {0x00A00000, "512BYTES"},
    {0x00B00000, "1024BYTES"}, {0x00C00000, "2048BYTES"}, {0x00D00000, "4096BYTES"}, {0x00E00000, "8192BYTES"},
};

// XFDATASTYPE_LIST normalizes a masked value down to bit zero.
XBinary::XIDSTRING _TABLE_XPE_XFImageSectionHeaderAligns[] = {
    {1, "1BYTES"},    {2, "2BYTES"},    {3, "4BYTES"},    {4, "8BYTES"},    {5, "16BYTES"},   {6, "32BYTES"},  {7, "64BYTES"},
    {8, "128BYTES"},  {9, "256BYTES"},  {10, "512BYTES"}, {11, "1024BYTES"}, {12, "2048BYTES"}, {13, "4096BYTES"}, {14, "8192BYTES"},
};

static qint32 _getXFRecordsSize(const QList<XBinary::XFRECORD> &listRecords)
{
    qint32 nResult = 0;

    for (const XBinary::XFRECORD &record : listRecords) {
        nResult = qMax(nResult, record.nOffset + record.nSize);
    }

    return nResult;
}

static void _filterXFRecordsBySize(QList<XBinary::XFRECORD> *pListRecords, qint64 nSize)
{
    if (nSize <= 0) {
        return;
    }

    QList<XBinary::XFRECORD> listFiltered;

    for (const XBinary::XFRECORD &record : qAsConst(*pListRecords)) {
        if ((record.nOffset >= 0) && ((qint64)record.nOffset + record.nSize <= nSize)) {
            listFiltered.append(record);
        }
    }

    *pListRecords = listFiltered;
}

static qint32 _getXFHeaderSize(qint64 nSize, const QList<XBinary::XFRECORD> &listRecords)
{
    return (nSize > 0) ? (qint32)nSize : _getXFRecordsSize(listRecords);
}

static qint64 _getXPECor20HeaderSize(XPE *pPE, qint64 nOffset, qint64 nAvailableSize)
{
    if ((pPE == nullptr) || (nOffset < 0) || (nAvailableSize < (qint64)sizeof(quint32)) ||
        !pPE->checkOffsetSize(nOffset, sizeof(quint32))) {
        return 0;
    }

    quint32 nDeclaredSize = pPE->read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, cb));

    if (nDeclaredSize < sizeof(XPE_DEF::IMAGE_COR20_HEADER)) {
        return 0;
    }

    return qMin(qMin(nAvailableSize, pPE->getSize() - nOffset), (qint64)nDeclaredSize);
}

static quint32 _getXPEExceptionStructID(quint16 nMachine)
{
    if ((nMachine == XPE_DEF::S_IMAGE_FILE_MACHINE_AMD64) || (nMachine == XPE_DEF::S_IMAGE_FILE_MACHINE_IA64) || (nMachine == 0xFD1D)) {
        return XPE::STRUCTID_IMAGE_RUNTIME_FUNCTION_ENTRY;
    }

    if ((nMachine == XPE_DEF::S_IMAGE_FILE_MACHINE_ARM) || (nMachine == XPE_DEF::S_IMAGE_FILE_MACHINE_THUMB) || (nMachine == 0x01C4) ||
        (nMachine == XPE_DEF::S_IMAGE_FILE_MACHINE_ARM64)) {
        return XPE::STRUCTID_IMAGE_RUNTIME_FUNCTION_ENTRY_ARM;
    }

    if (nMachine == XPE_DEF::S_IMAGE_FILE_MACHINE_ALPHA) {
        return XPE::STRUCTID_IMAGE_RUNTIME_FUNCTION_ENTRY_ALPHA;
    }

    if (nMachine == XPE_DEF::S_IMAGE_FILE_MACHINE_ALPHA64) {
        return XPE::STRUCTID_IMAGE_RUNTIME_FUNCTION_ENTRY_ALPHA64;
    }

    return XPE::STRUCTID_IMAGE_EXCEPTION_DIRECTORY_RAW;
}

XBinary::XIDSTRING _TABLE_XPE_ResourceTypes[] = {
    {1, "CURSOR"},
    {2, "BITMAP"},
    {3, "ICON"},
    {4, "MENU"},
    {5, "DIALOG"},
    {6, "STRING"},
    {7, "FONTDIR"},
    {8, "FONT"},
    {9, "ACCELERATORS"},
    {10, "RCDATA"},
    {11, "MESSAGETABLE"},
    {12, "GROUP_CURSOR"},
    {14, "GROUP_ICON"},
    {15, "NAMETABLE"},
    {16, "VERSION"},
    {17, "DLGINCLUDE"},
    {23, "HTML"},
    {24, "MANIFEST"},
    {0x2000 + 2, "NEWBITMAP"},
    {0x2000 + 4, "NEWMENU"},
    {0x2000 + 5, "NEWDIALOG"},
};

XBinary::XIDSTRING _TABLE_XPE_ImageRelBased[] = {
    {0, "ABSOLUTE"},
    {1, "HIGH"},
    {2, "LOW"},
    {3, "HIGHLOW"},
    {4, "HIGHADJ"},
    {5, "MACHINE_SPECIFIC_5"},
    {6, "RESERVED"},
    {7, "MACHINE_SPECIFIC_7"},
    {8, "MACHINE_SPECIFIC_8"},
    {9, "MACHINE_SPECIFIC_9"},
    {10, "DIR64"},
};

XBinary::XIDSTRING _TABLE_XPE_ComImageFlags[] = {
    {0x00000001, "ILONLY"},           {0x00000002, "32BITREQUIRED"},     {0x00000004, "IL_LIBRARY"},
    {0x00000008, "STRONGNAMESIGNED"}, {0x00000010, "NATIVE_ENTRYPOINT"}, {0x00010000, "TRACKDEBUGDATA"},
};

XBinary::XIDSTRING _TABLE_XPE_DebugTypes[] = {
    {0, "UNKNOWN"},     {1, "COFF"},        {2, "CODEVIEW"},      {3, "FPO"},     {4, "MISC"},        {5, "EXCEPTION"},
    {6, "FIXUP"},       {7, "OMAP_TO_SRC"}, {8, "OMAP_FROM_SRC"}, {9, "BORLAND"}, {10, "RESERVED10"}, {11, "CLSID"},
    {12, "VC_FEATURE"}, {13, "POGO"},       {14, "ILTCG"},        {15, "MPX"},    {16, "REPRO"},      {20, "EX_DLLCHARACTERISTICS"},
};

XBinary::XIDSTRING _TABLE_XPE_ResourcesFixedFileInfoSignatures[] = {
    {0xFEEF04BD, "FIXEDFILEINFO Signature"},
};

XBinary::XIDSTRING _TABLE_XPE_ResourcesFixedFileInfoFileFlags[] = {
    {0x00000001, "DEBUG"}, {0x00000010, "INFOINFERRED"}, {0x00000004, "PATCHED"}, {0x00000002, "PRERELEASE"}, {0x00000008, "PRIVATEBUILD"}, {0x00000020, "SPECIALBUILD"},
};

XBinary::XIDSTRING _TABLE_XPE_ResourcesFixedFileInfoFileOses[] = {
    {0x00000000, "UNKNOWN"},      {0x00010000, "DOS"},           {0x00020000, "OS216"},         {0x00030000, "OS232"},      {0x00040000, "NT"},
    {0x00050000, "WINCE"},        {0x00000000, "_BASE"},         {0x00000001, "_WINDOWS16"},    {0x00000002, "_PM16"},      {0x00000003, "_PM32"},
    {0x00000004, "_WINDOWS32"},   {0x00010001, "DOS_WINDOWS16"}, {0x00010004, "DOS_WINDOWS32"}, {0x00020002, "OS216_PM16"}, {0x00030003, "OS232_PM32"},
    {0x00040004, "NT_WINDOWS32"},
};

XBinary::XIDSTRING _TABLE_XPE_ResourcesFixedFileInfoFileTypes[] = {
    {0x00000000L, "UNKNOWN"}, {0x00000001L, "APP"}, {0x00000002L, "DLL"}, {0x00000003L, "DRV"}, {0x00000004L, "FONT"}, {0x00000005L, "VXD"}, {0x00000007L, "STATIC_LIB"},
};

XBinary::XIDSTRING _TABLE_XPE_NetMetadataFlags[] = {
    {0x0001, "EXTRA_DATA"},
};

static void _decorateXPEXFHeader(XBinary::XFHEADER *pXfHeader)
{
    if (pXfHeader == nullptr) {
        return;
    }

    quint32 nStructID = (quint32)pXfHeader->structID;

    if (((nStructID == XPE::STRUCTID_IMAGE_NT_HEADERS32) || (nStructID == XPE::STRUCTID_IMAGE_NT_HEADERS64)) && (pXfHeader->listFields.count() > 0)) {
        pXfHeader->listDataSt.append(
            {0, 0, XBinary::XFDATASTYPE_LIST, _TABLE_XPE_ImageNtHeadersSignatures, sizeof(_TABLE_XPE_ImageNtHeadersSignatures) / sizeof(XBinary::XIDSTRING)});
    } else if (nStructID == XPE::STRUCTID_IMAGE_FILE_HEADER) {
        if (pXfHeader->listFields.count() > 0) {
            pXfHeader->listDataSt.append(
                {0, 0, XBinary::XFDATASTYPE_LIST, _TABLE_XPE_ImageFileHeaderMachines, sizeof(_TABLE_XPE_ImageFileHeaderMachines) / sizeof(XBinary::XIDSTRING)});
        }
        if (pXfHeader->listFields.count() > 6) {
            pXfHeader->listDataSt.append({6, 0xFFFF, XBinary::XFDATASTYPE_FLAGS, _TABLE_XPE_ImageFileHeaderCharacteristics,
                                          sizeof(_TABLE_XPE_ImageFileHeaderCharacteristics) / sizeof(XBinary::XIDSTRING)});
        }
    } else if ((nStructID == XPE::STRUCTID_IMAGE_OPTIONAL_HEADER32) || (nStructID == XPE::STRUCTID_IMAGE_OPTIONAL_HEADER64)) {
        if (pXfHeader->listFields.count() > 0) {
            pXfHeader->listDataSt.append(
                {0, 0, XBinary::XFDATASTYPE_LIST, _TABLE_XPE_ImageOptionalHeaderMagic, sizeof(_TABLE_XPE_ImageOptionalHeaderMagic) / sizeof(XBinary::XIDSTRING)});
        }

        qint32 nSubsystemField = (nStructID == XPE::STRUCTID_IMAGE_OPTIONAL_HEADER64) ? 21 : 22;
        qint32 nDllCharacteristicsField = nSubsystemField + 1;
        if (pXfHeader->listFields.count() > nSubsystemField) {
            pXfHeader->listDataSt.append({nSubsystemField, 0, XBinary::XFDATASTYPE_LIST, _TABLE_XPE_ImageOptionalHeaderSubsystem,
                                          sizeof(_TABLE_XPE_ImageOptionalHeaderSubsystem) / sizeof(XBinary::XIDSTRING)});
        }
        if (pXfHeader->listFields.count() > nDllCharacteristicsField) {
            pXfHeader->listDataSt.append({nDllCharacteristicsField, 0xFFFF, XBinary::XFDATASTYPE_FLAGS, _TABLE_XPE_ImageOptionalHeaderDllCharacteristics,
                                          sizeof(_TABLE_XPE_ImageOptionalHeaderDllCharacteristics) / sizeof(XBinary::XIDSTRING)});
        }
    } else if ((nStructID == XPE::STRUCTID_IMAGE_SECTION_HEADER) && (pXfHeader->listFields.count() > 9)) {
        pXfHeader->listDataSt.append({9, 0x00F00000, XBinary::XFDATASTYPE_LIST, _TABLE_XPE_XFImageSectionHeaderAligns,
                                      sizeof(_TABLE_XPE_XFImageSectionHeaderAligns) / sizeof(XBinary::XIDSTRING)});
        pXfHeader->listDataSt.append({9, 0xFFFFFFFF, XBinary::XFDATASTYPE_FLAGS, _TABLE_XPE_ImageSectionHeaderFlags,
                                      sizeof(_TABLE_XPE_ImageSectionHeaderFlags) / sizeof(XBinary::XIDSTRING)});
    } else if ((nStructID == XPE::STRUCTID_IMAGE_DEBUG_DIRECTORY) && (pXfHeader->listFields.count() > 4)) {
        pXfHeader->listDataSt.append(
            {4, 0, XBinary::XFDATASTYPE_LIST, _TABLE_XPE_DebugTypes, sizeof(_TABLE_XPE_DebugTypes) / sizeof(XBinary::XIDSTRING)});
    } else if ((nStructID == XPE::STRUCTID_IMAGE_COR20_HEADER) && (pXfHeader->listFields.count() > 5)) {
        pXfHeader->listDataSt.append(
            {5, 0xFFFFFFFF, XBinary::XFDATASTYPE_FLAGS, _TABLE_XPE_ComImageFlags, sizeof(_TABLE_XPE_ComImageFlags) / sizeof(XBinary::XIDSTRING)});
    } else if ((nStructID == XPE::STRUCTID_IMAGE_BASE_RELOCATION_ENTRY) && !pXfHeader->listFields.isEmpty()) {
        pXfHeader->listDataSt.append(
            {0, 0xF000, XBinary::XFDATASTYPE_LIST, _TABLE_XPE_ImageRelBased, sizeof(_TABLE_XPE_ImageRelBased) / sizeof(XBinary::XIDSTRING)});
    } else if ((nStructID == XPE::STRUCTID_NET_METADATA) && (pXfHeader->listFields.count() > 6)) {
        pXfHeader->listDataSt.append(
            {6, 0xFFFF, XBinary::XFDATASTYPE_FLAGS, _TABLE_XPE_NetMetadataFlags, sizeof(_TABLE_XPE_NetMetadataFlags) / sizeof(XBinary::XIDSTRING)});
    }
}

static QList<XADDR> _getXPEFixedTableRows(XPE *pPE, qint64 nOffset, qint64 nSize, qint32 nRowSize, bool bStopOnZero,
                                                    XBinary::PDSTRUCT *pPdStruct)
{
    QList<XADDR> listResult;

    if ((pPE == nullptr) || (nOffset < 0) || (nSize <= 0) || (nRowSize <= 0) || (nOffset >= pPE->getSize())) {
        return listResult;
    }

    qint64 nAvailableSize = qMin(nSize, pPE->getSize() - nOffset);
    qint32 nRows = (qint32)qMin((qint64)0x10000, nAvailableSize / nRowSize);

    for (qint32 i = 0; (i < nRows) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        qint64 nRowOffset = nOffset + (qint64)i * nRowSize;

        if (bStopOnZero) {
            QByteArray baRow = pPE->read_array(nRowOffset, nRowSize);

            if ((baRow.size() != nRowSize) || (baRow == QByteArray(nRowSize, 0))) {
                break;
            }
        }

        listResult.append(nRowOffset);
    }

    return listResult;
}

static void _decorateXPEDataDirectoryTable(XPE *pPE, XBinary::XFHEADER *pXfHeader, XBinary::_MEMORY_MAP *pMemoryMap)
{
    if ((pPE == nullptr) || (pXfHeader == nullptr) || pXfHeader->listRowLocations.isEmpty()) {
        return;
    }

    qint64 nOptionalHeaderOffset = pPE->getOptionalHeaderOffset();
    qint64 nCanonicalOffset = -1;

    if (nOptionalHeaderOffset != -1) {
        nCanonicalOffset = nOptionalHeaderOffset +
                           (pPE->is64() ? (qint64)sizeof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S) : (qint64)sizeof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S));
    }

    QMap<quint64, QString> mapNames = XPE::getImageOptionalHeaderDataDirectoryS();
    XBinary::XFCOLUMN xfKindColumn = {};
    xfKindColumn.sName = QString("Address kind");
    XBinary::XFCOLUMN xfOffsetColumn = {};
    xfOffsetColumn.sName = QString("File offset");
    pXfHeader->listRowNames.clear();

    for (qint32 i = 0; i < pXfHeader->listRowLocations.count(); i++) {
        qint64 nRowOffset = pXfHeader->listRowLocations.at(i);
        qint32 nDirectoryIndex = i;

        if ((nCanonicalOffset >= 0) && (nRowOffset >= nCanonicalOffset) &&
            (((nRowOffset - nCanonicalOffset) % (qint64)sizeof(XPE_DEF::IMAGE_DATA_DIRECTORY)) == 0)) {
            qint64 nCandidateIndex = (nRowOffset - nCanonicalOffset) / sizeof(XPE_DEF::IMAGE_DATA_DIRECTORY);
            if ((nCandidateIndex >= 0) && (nCandidateIndex < 16)) {
                nDirectoryIndex = (qint32)nCandidateIndex;
            }
        }

        pXfHeader->listRowNames.append(mapNames.value(nDirectoryIndex));

        if (!pPE->checkOffsetSize(nRowOffset, sizeof(XPE_DEF::IMAGE_DATA_DIRECTORY))) {
            xfKindColumn.listValues.append(QString());
            xfOffsetColumn.listValues.append(QString());
            continue;
        }

        quint32 nAddress = pPE->read_uint32(nRowOffset + offsetof(XPE_DEF::IMAGE_DATA_DIRECTORY, VirtualAddress));
        qint64 nResolvedOffset = -1;

        if (nDirectoryIndex == XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_SECURITY) {
            xfKindColumn.listValues.append(QString("File offset"));
            if ((nAddress != 0) && (nAddress < pPE->getSize())) {
                nResolvedOffset = nAddress;
            }
        } else {
            xfKindColumn.listValues.append(QString("RVA"));
            if ((nAddress != 0) && (pMemoryMap != nullptr)) {
                nResolvedOffset = XBinary::relAddressToOffset(pMemoryMap, nAddress);
            }
        }

        xfOffsetColumn.listValues.append(nResolvedOffset >= 0 ? QString("0x%1").arg((quint64)nResolvedOffset, 0, 16).toUpper() : QString());
    }

    pXfHeader->listExtraColumns.append(xfKindColumn);
    pXfHeader->listExtraColumns.append(xfOffsetColumn);
}

const QString XPE::PREFIX_ImageNtHeadersSignatures = "IMAGE_";
const QString XPE::PREFIX_ImageMagics = "IMAGE_";
const QString XPE::PREFIX_ImageFileHeaderMachines = "IMAGE_FILE_MACHINE_";
const QString XPE::PREFIX_ImageFileHeaderCharacteristics = "IMAGE_FILE_";
const QString XPE::PREFIX_ImageOptionalHeaderMagic = "IMAGE_";
const QString XPE::PREFIX_ImageOptionalHeaderSubsystem = "IMAGE_SUBSYSTEM_";
const QString XPE::PREFIX_ImageOptionalHeaderDllCharacteristics = "IMAGE_DLLCHARACTERISTICS_";
const QString XPE::PREFIX_ImageOptionalHeaderDataDirectory = "IMAGE_DIRECTORY_ENTRY_";
const QString XPE::PREFIX_ImageSectionHeaderFlags = "IMAGE_SCN_";
const QString XPE::PREFIX_ImageSectionHeaderAligns = "IMAGE_SCN_ALIGN_";
const QString XPE::PREFIX_ResourceTypes = "RT_";
const QString XPE::PREFIX_ImageRelBased = "IMAGE_REL_BASED_";
const QString XPE::PREFIX_ComImageFlags = "COMIMAGE_FLAGS_";
const QString XPE::PREFIX_DebugTypes = "IMAGE_DEBUG_TYPE_";
const QString XPE::PREFIX_ResourcesFixedFileInfoSignatures = "";
const QString XPE::PREFIX_ResourcesFixedFileInfoFileFlags = "VS_FF_";
const QString XPE::PREFIX_ResourcesFixedFileInfoFileOses = "VOS_";
const QString XPE::PREFIX_ResourcesFixedFileInfoFileTypes = "VFT_";
const QString XPE::PREFIX_NetMetadataFlags = "METADATA_";

XPE::XPE(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress) : XMSDOS(pDevice, bIsImage, nModuleAddress)
{
}

bool XPE::isValid(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    bool bResult = false;

    quint16 magic = get_magic();

    qint64 nSize = getSize();

    if ((magic == XMSDOS_DEF::S_IMAGE_DOS_SIGNATURE_MZ) || (magic == XMSDOS_DEF::S_IMAGE_DOS_SIGNATURE_ZM)) {
        qint32 lfanew = get_lfanew();

        if ((lfanew > 0) && (lfanew < (nSize & 0xFFFFFFFF))) {
            quint32 signature = read_uint32(lfanew);

            if (signature == XPE_DEF::S_IMAGE_NT_SIGNATURE) {
                bResult = true;
            }
        }
    }

    return bResult;
}

bool XPE::isValid(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress, PDSTRUCT *pPdStruct)
{
    XPE xpe(pDevice, bIsImage, nModuleAddress);

    return xpe.isValid();
}

XBinary::MODE XPE::getMode(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    XPE xpe(pDevice, bIsImage, nModuleAddress);

    return xpe.getMode();
}

XBinary::MODE XPE::getMode()
{
    MODE result = MODE_32;

    quint16 nMachine = getFileHeader_Machine();
    nMachine = _getMachine(nMachine);

    if ((nMachine == XPE_DEF::S_IMAGE_FILE_MACHINE_AMD64) || (nMachine == XPE_DEF::S_IMAGE_FILE_MACHINE_IA64) || (nMachine == XPE_DEF::S_IMAGE_FILE_MACHINE_ARM64) ||
        (nMachine == XPE_DEF::S_IMAGE_FILE_MACHINE_ALPHA64) || (nMachine == XPE_DEF::S_IMAGE_FILE_MACHINE_RISCV64) ||
        (nMachine == XPE_DEF::S_IMAGE_FILE_MACHINE_LOONGARCH64)) {
        result = MODE_64;
    } else {
        result = MODE_32;
    }

    return result;
}

QString XPE::getArch()
{
    quint16 nMachine = getFileHeader_Machine();
    nMachine = _getMachine(nMachine);

    return getImageFileHeaderMachinesS().value(nMachine, tr("Unknown"));
}

XBinary::ENDIAN XPE::getEndian()
{
    ENDIAN result = ENDIAN_LITTLE;

    quint16 nMachine = getFileHeader_Machine();
    nMachine = _getMachine(nMachine);

    if ((nMachine == XPE_DEF::S_IMAGE_FILE_MACHINE_R3000_BE) || (nMachine == XPE_DEF::S_IMAGE_FILE_MACHINE_POWERPCBE)) {
        result = ENDIAN_BIG;
    }

    return result;
}

quint16 XPE::_getMachine(quint16 nMachine)
{
    quint16 nResult = nMachine;

    if (nResult == (XPE_DEF::S_IMAGE_FILE_MACHINE_NATIVE_OS_OVERRIDE_LINUX ^ XPE_DEF::S_IMAGE_FILE_MACHINE_AMD64)) {
        nResult = XPE_DEF::S_IMAGE_FILE_MACHINE_AMD64;
    }

    return nResult;
}

XBinary::FT XPE::getFileType()
{
    FT result = FT_PE32;

    MODE mode = getMode();

    if (mode == MODE_32) {
        result = FT_PE32;
    } else if (mode == MODE_64) {
        result = FT_PE64;
    }

    return result;
}

qint32 XPE::getType()
{
    TYPE result = TYPE_APPLICATION;

    quint16 nSubsystem = getOptionalHeader_Subsystem();

    if ((nSubsystem == XPE_DEF::S_IMAGE_SUBSYSTEM_NATIVE) || (nSubsystem == XPE_DEF::S_IMAGE_SUBSYSTEM_NATIVE_WINDOWS)) {
        if (isImportLibraryPresent("ntdll.dll")) {
            result = TYPE_NATIVE;
        } else {
            result = TYPE_DRIVER;
        }
    } else if ((nSubsystem == XPE_DEF::S_IMAGE_SUBSYSTEM_WINDOWS_CUI) || (nSubsystem == XPE_DEF::S_IMAGE_SUBSYSTEM_OS2_CUI) ||
               (nSubsystem == XPE_DEF::S_IMAGE_SUBSYSTEM_POSIX_CUI)) {
        result = TYPE_CONSOLE;
    } else if (nSubsystem == XPE_DEF::S_IMAGE_SUBSYSTEM_WINDOWS_GUI) {
        result = TYPE_GUI;
    } else if (nSubsystem == XPE_DEF::S_IMAGE_SUBSYSTEM_WINDOWS_CE_GUI) {
        result = TYPE_CE_GUI;
    } else if (nSubsystem == XPE_DEF::S_IMAGE_SUBSYSTEM_XBOX) {
        result = TYPE_XBOX_APPLICATION;
    } else if (nSubsystem == XPE_DEF::S_IMAGE_SUBSYSTEM_EFI_APPLICATION) {
        result = TYPE_EFI_APPLICATION;
    } else if (nSubsystem == XPE_DEF::S_IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER) {
        result = TYPE_EFI_BOOTSERVICEDRIVER;
    } else if (nSubsystem == XPE_DEF::S_IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER) {
        result = TYPE_EFI_RUNTIMEDRIVER;
    }

    // TODO from Resource/Version
    if (result != TYPE_DRIVER)  // TODO Check
    {
        if ((getFileHeader_Characteristics() & XPE_DEF::S_IMAGE_FILE_DLL)) {
            result = TYPE_DLL;
        }
    }

    return result;
}

QString XPE::typeIdToString(qint32 nType)
{
    QString sResult = tr("Unknown");

    switch (nType) {
        case TYPE_UNKNOWN: sResult = tr("Unknown"); break;
        case TYPE_APPLICATION: sResult = tr("Application"); break;
        case TYPE_XBOX_APPLICATION: sResult = QString("XBOX %1").arg(tr("Application")); break;
        case TYPE_EFI_APPLICATION: sResult = QString("EFI %1").arg(tr("Application")); break;
        case TYPE_GUI: sResult = QString("GUI"); break;
        case TYPE_CE_GUI: sResult = QString("CE GUI"); break;
        case TYPE_CONSOLE: sResult = tr("Console"); break;
        case TYPE_DLL: sResult = QString("DLL"); break;
        case TYPE_DRIVER: sResult = tr("Driver"); break;
        case TYPE_NATIVE: sResult = tr("Native"); break;
        case TYPE_BOOTAPPLICATION: sResult = tr("Boot application"); break;
        case TYPE_EFI_RUNTIMEDRIVER: sResult = QString("EFI %1").arg(tr("Runtime driver")); break;
        case TYPE_EFI_BOOTSERVICEDRIVER:
            sResult = QString("EFI %1").arg(tr("Boot service driver"));
            break;
            //        case TYPE_EFIBOOT:          sResult=QString("EFI Boot");
            //        break; case TYPE_EFI:              sResult=QString("EFI");
            //        break; case TYPE_EFIRUNTIMEDRIVER: sResult=QString("EFI
            //        Runtime driver");  break; case TYPE_XBOX:
            //        sResult=QString("XBOX");                break; case
            //        TYPE_OS2:              sResult=QString("OS2"); break; case
            //        TYPE_POSIX:            sResult=QString("POSIX"); break;
            //        case TYPE_CE:               sResult=QString("CE"); break;
    }

    return sResult;
}

bool XPE::isSigned()
{
    return getSignOffsetSize().nSize;
}

XBinary::OFFSETSIZE XPE::getSignOffsetSize()
{
    OFFSETSIZE osResult = {};

    XPE_DEF::IMAGE_DATA_DIRECTORY idSecurity = getOptionalHeader_DataDirectory(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_SECURITY);

    OFFSETSIZE osSecurity = {};
    osSecurity.nOffset = idSecurity.VirtualAddress;
    osSecurity.nSize = idSecurity.Size;

    if (checkOffsetSize(osSecurity)) {
        osResult = osSecurity;
    }

    return osResult;
}

QString XPE::getFileFormatExt()
{
    QString sResult;

    TYPE _type = (TYPE)getType();

    if (_type == TYPE_DLL) {
        sResult = "dll";
    } else if ((_type == TYPE_EFI_APPLICATION) || (_type == TYPE_EFI_BOOTSERVICEDRIVER) || (_type == TYPE_EFI_RUNTIMEDRIVER)) {
        sResult = "efi";
    } else if (_type == TYPE_DRIVER) {
        sResult = "sys";
    } else {
        sResult = "exe";
    }

    return sResult;
}

QString XPE::getFileFormatExtsString()
{
    return QString("PE(exe, dll, sys)");
}

qint64 XPE::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    return _calculateRawSize(pPdStruct);
}

XBinary::FILEFORMATINFO XPE::getFileFormatInfo(PDSTRUCT *pPdStruct)
{
    FILEFORMATINFO result = {};

    result.bIsValid = isValid(pPdStruct);

    if (result.bIsValid) {
        result.nSize = getSize();

        result.fileType = getFileType();
        result.sExt = getFileFormatExt();
        result.sVersion = getVersion();
        result.sInfo = getInfo();
        result.sArch = getArch();
        result.mode = getMode();
        result.sType = typeIdToString(getType());
        result.endian = getEndian();
        result.sMIME = getMIMEString();

        result.osName = OSNAME_WINDOWS;

        quint16 nSubsystem = getOptionalHeader_Subsystem();
        quint16 nMachine = getFileHeader_Machine();

        if ((nSubsystem == XPE_DEF::S_IMAGE_SUBSYSTEM_WINDOWS_GUI) || (nSubsystem == XPE_DEF::S_IMAGE_SUBSYSTEM_WINDOWS_CUI) ||
            (nSubsystem == XPE_DEF::S_IMAGE_SUBSYSTEM_NATIVE_WINDOWS) || (nSubsystem == XPE_DEF::S_IMAGE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION)) {
            result.osName = OSNAME_WINDOWS;
        } else if ((nSubsystem == XPE_DEF::S_IMAGE_SUBSYSTEM_EFI_APPLICATION) || (nSubsystem == XPE_DEF::S_IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER) ||
                   (nSubsystem == XPE_DEF::S_IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER) || (nSubsystem == XPE_DEF::S_IMAGE_SUBSYSTEM_EFI_ROM)) {
            result.osName = OSNAME_UEFI;
        } else if ((nSubsystem == XPE_DEF::S_IMAGE_SUBSYSTEM_XBOX) || (nSubsystem == XPE_DEF::S_IMAGE_SUBSYSTEM_XBOX_CODE_CATALOG)) {
            result.osName = OSNAME_XBOX;
        } else if (nSubsystem == XPE_DEF::S_IMAGE_SUBSYSTEM_OS2_CUI) {
            result.osName = OSNAME_OS2;
        } else if (nSubsystem == XPE_DEF::S_IMAGE_SUBSYSTEM_POSIX_CUI) {
            result.osName = OSNAME_POSIX;
        } else if (nSubsystem == XPE_DEF::S_IMAGE_SUBSYSTEM_WINDOWS_CE_GUI) {
            result.osName = OSNAME_WINDOWSCE;
        }

        if (nMachine == (XPE_DEF::S_IMAGE_FILE_MACHINE_NATIVE_OS_OVERRIDE_LINUX ^ XPE_DEF::S_IMAGE_FILE_MACHINE_AMD64)) {
            result.osName = OSNAME_LINUX;
        }

        if (result.osName == OSNAME_WINDOWS) {
            bool bIs64 = is64();

            quint32 nOSVersion = getOperatingSystemVersion();

            if (bIs64)  // Correct version
            {
                if (nOSVersion < 0x00050002) {
                    // Server 2003
                    nOSVersion = 0x00050002;
                }
            }

            QMap<quint64, QString> mapOSVersion = XPE::getOperatingSystemVersionsS(OSNAME_WINDOWS);

            if (!mapOSVersion.contains(nOSVersion) || (nOSVersion == 0)) {
                if (bIs64) {
                    nOSVersion = 0x00050002;  // Server 2003
                } else {
                    nOSVersion = 0x00050001;  // XP
                }
            }

            // result.sOsVersion = QString("min req") + QString(": ") + mapOSVersion.value(nOSVersion);
            result.sOsVersion = mapOSVersion.value(nOSVersion);
        }

        result.sArch = getArch();
        result.mode = getMode();
        result.sType = typeIdToString(getType());
        result.endian = getEndian();

        if (result.nSize == 0) {
            result.bIsValid = false;
        }
    }

    return result;
}

QList<QString> XPE::getSearchSignatures()
{
    QList<QString> listResult;

    listResult.append("'MZ'");

    return listResult;
}

XBinary *XPE::createInstance(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    return new XPE(pDevice, bIsImage, nModuleAddress);
}

qint64 XPE::getNtHeadersOffset()
{
    qint64 result = get_lfanew();

    if (!_isOffsetValid(result)) {
        result = -1;
    }

    return result;
}

quint32 XPE::getNtHeaders_Signature()
{
    qint64 nOffset = getNtHeadersOffset();

    return read_uint32(nOffset);
}

void XPE::setNtHeaders_Signature(quint32 nValue)
{
    write_uint32(getNtHeadersOffset(), nValue);
}

qint64 XPE::getFileHeaderOffset()
{
    qint64 result = get_lfanew() + 4;

    if (!_isOffsetValid(result)) {
        result = -1;
    }

    return result;
}

qint64 XPE::getFileHeaderSize()
{
    return sizeof(XPE_DEF::IMAGE_FILE_HEADER);
}

XPE_DEF::IMAGE_FILE_HEADER XPE::getFileHeader()
{
    XPE_DEF::IMAGE_FILE_HEADER result = {};

    qint64 nFileHeaderOffset = getFileHeaderOffset();

    if (nFileHeaderOffset != -1) {
        result = _read_IMAGE_FILE_HEADER(nFileHeaderOffset);
    }

    return result;
}

void XPE::setFileHeader(XPE_DEF::IMAGE_FILE_HEADER *pFileHeader)
{
    qint64 nFileHeaderOffset = getFileHeaderOffset();

    if (nFileHeaderOffset != -1) {
        write_uint16(nFileHeaderOffset + offsetof(XPE_DEF::IMAGE_FILE_HEADER, Machine), pFileHeader->Machine);
        write_uint16(nFileHeaderOffset + offsetof(XPE_DEF::IMAGE_FILE_HEADER, NumberOfSections), pFileHeader->NumberOfSections);
        write_uint32(nFileHeaderOffset + offsetof(XPE_DEF::IMAGE_FILE_HEADER, TimeDateStamp), pFileHeader->TimeDateStamp);
        write_uint32(nFileHeaderOffset + offsetof(XPE_DEF::IMAGE_FILE_HEADER, PointerToSymbolTable), pFileHeader->PointerToSymbolTable);
        write_uint32(nFileHeaderOffset + offsetof(XPE_DEF::IMAGE_FILE_HEADER, NumberOfSymbols), pFileHeader->NumberOfSymbols);
        write_uint16(nFileHeaderOffset + offsetof(XPE_DEF::IMAGE_FILE_HEADER, SizeOfOptionalHeader), pFileHeader->SizeOfOptionalHeader);
        write_uint16(nFileHeaderOffset + offsetof(XPE_DEF::IMAGE_FILE_HEADER, Characteristics), pFileHeader->Characteristics);
    }
}

XPE_DEF::IMAGE_FILE_HEADER XPE::_read_IMAGE_FILE_HEADER(qint64 nOffset)
{
    XPE_DEF::IMAGE_FILE_HEADER result = {};

    result.Machine = read_uint16(nOffset + offsetof(XPE_DEF::IMAGE_FILE_HEADER, Machine));
    result.NumberOfSections = read_uint16(nOffset + offsetof(XPE_DEF::IMAGE_FILE_HEADER, NumberOfSections));
    result.TimeDateStamp = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_FILE_HEADER, TimeDateStamp));
    result.PointerToSymbolTable = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_FILE_HEADER, PointerToSymbolTable));
    result.NumberOfSymbols = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_FILE_HEADER, NumberOfSymbols));
    result.SizeOfOptionalHeader = read_uint16(nOffset + offsetof(XPE_DEF::IMAGE_FILE_HEADER, SizeOfOptionalHeader));
    result.Characteristics = read_uint16(nOffset + offsetof(XPE_DEF::IMAGE_FILE_HEADER, Characteristics));

    return result;
}

quint16 XPE::getFileHeader_Machine()
{
    return read_uint16(getFileHeaderOffset() + offsetof(XPE_DEF::IMAGE_FILE_HEADER, Machine));
}

quint16 XPE::getFileHeader_NumberOfSections()
{
    return read_uint16(getFileHeaderOffset() + offsetof(XPE_DEF::IMAGE_FILE_HEADER, NumberOfSections));
}

quint32 XPE::getFileHeader_TimeDateStamp()
{
    return read_uint32(getFileHeaderOffset() + offsetof(XPE_DEF::IMAGE_FILE_HEADER, TimeDateStamp));
}

quint32 XPE::getFileHeader_PointerToSymbolTable()
{
    return read_uint32(getFileHeaderOffset() + offsetof(XPE_DEF::IMAGE_FILE_HEADER, PointerToSymbolTable));
}

quint32 XPE::getFileHeader_NumberOfSymbols()
{
    return read_uint32(getFileHeaderOffset() + offsetof(XPE_DEF::IMAGE_FILE_HEADER, NumberOfSymbols));
}

quint16 XPE::getFileHeader_SizeOfOptionalHeader()
{
    return read_uint16(getFileHeaderOffset() + offsetof(XPE_DEF::IMAGE_FILE_HEADER, SizeOfOptionalHeader));
}

quint16 XPE::getFileHeader_Characteristics()
{
    return read_uint16(getFileHeaderOffset() + offsetof(XPE_DEF::IMAGE_FILE_HEADER, Characteristics));
}

void XPE::setFileHeader_Machine(quint16 nValue)
{
    write_uint16(getFileHeaderOffset() + offsetof(XPE_DEF::IMAGE_FILE_HEADER, Machine), nValue);
}

void XPE::setFileHeader_NumberOfSections(quint16 nValue)
{
    write_uint16(getFileHeaderOffset() + offsetof(XPE_DEF::IMAGE_FILE_HEADER, NumberOfSections), nValue);
}

void XPE::setFileHeader_TimeDateStamp(quint32 nValue)
{
    write_uint32(getFileHeaderOffset() + offsetof(XPE_DEF::IMAGE_FILE_HEADER, TimeDateStamp), nValue);
}

void XPE::setFileHeader_PointerToSymbolTable(quint32 nValue)
{
    write_uint32(getFileHeaderOffset() + offsetof(XPE_DEF::IMAGE_FILE_HEADER, PointerToSymbolTable), nValue);
}

void XPE::setFileHeader_NumberOfSymbols(quint32 nValue)
{
    write_uint32(getFileHeaderOffset() + offsetof(XPE_DEF::IMAGE_FILE_HEADER, NumberOfSymbols), nValue);
}

void XPE::setFileHeader_SizeOfOptionalHeader(quint16 nValue)
{
    write_uint16(getFileHeaderOffset() + offsetof(XPE_DEF::IMAGE_FILE_HEADER, SizeOfOptionalHeader), nValue);
}

void XPE::setFileHeader_Characteristics(quint16 nValue)
{
    write_uint16(getFileHeaderOffset() + offsetof(XPE_DEF::IMAGE_FILE_HEADER, Characteristics), nValue);
}

qint64 XPE::getOptionalHeaderOffset()
{
    qint64 result = get_lfanew() + 4 + sizeof(XPE_DEF::IMAGE_FILE_HEADER);

    if (!_isOffsetValid(result)) {
        result = -1;
    }

    return result;
}

qint64 XPE::getOptionalHeaderSize()
{
    qint64 nResult = 0;

    if (is64()) {
        nResult = sizeof(XPE_DEF::IMAGE_OPTIONAL_HEADER64);
    } else {
        nResult = sizeof(XPE_DEF::IMAGE_OPTIONAL_HEADER32);
    }

    return nResult;
}

XPE_DEF::IMAGE_OPTIONAL_HEADER32 XPE::getOptionalHeader32()
{
    XPE_DEF::IMAGE_OPTIONAL_HEADER32 result = {};
    // TODO
    read_array(getOptionalHeaderOffset(), (char *)&result, sizeof(XPE_DEF::IMAGE_OPTIONAL_HEADER32));

    return result;
}

XPE_DEF::IMAGE_OPTIONAL_HEADER64 XPE::getOptionalHeader64()
{
    XPE_DEF::IMAGE_OPTIONAL_HEADER64 result = {};
    // TODO
    read_array(getOptionalHeaderOffset(), (char *)&result, sizeof(XPE_DEF::IMAGE_OPTIONAL_HEADER64));

    return result;
}

void XPE::setOptionalHeader32(XPE_DEF::IMAGE_OPTIONAL_HEADER32 *pOptionalHeader32)
{
    if (pOptionalHeader32->SectionAlignment == 0) {
        pOptionalHeader32->SectionAlignment = 1;
    }
    if (pOptionalHeader32->FileAlignment == 0) {
        pOptionalHeader32->FileAlignment = 1;
    }

    // TODO
    write_array(getOptionalHeaderOffset(), (char *)pOptionalHeader32, sizeof(XPE_DEF::IMAGE_OPTIONAL_HEADER32));
}

void XPE::setOptionalHeader64(XPE_DEF::IMAGE_OPTIONAL_HEADER64 *pOptionalHeader64)
{
    if (pOptionalHeader64->SectionAlignment == 0) {
        pOptionalHeader64->SectionAlignment = 1;
    }
    if (pOptionalHeader64->FileAlignment == 0) {
        pOptionalHeader64->FileAlignment = 1;
    }

    // TODO
    write_array(getOptionalHeaderOffset(), (char *)pOptionalHeader64, sizeof(XPE_DEF::IMAGE_OPTIONAL_HEADER64));
}

XPE_DEF::IMAGE_OPTIONAL_HEADER32S XPE::getOptionalHeader32S()
{
    XPE_DEF::IMAGE_OPTIONAL_HEADER32S result = {};
    // TODO
    read_array(getOptionalHeaderOffset(), (char *)&result, sizeof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S));

    return result;
}

XPE_DEF::IMAGE_OPTIONAL_HEADER64S XPE::getOptionalHeader64S()
{
    XPE_DEF::IMAGE_OPTIONAL_HEADER64S result = {};
    // TODO
    read_array(getOptionalHeaderOffset(), (char *)&result, sizeof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S));

    return result;
}

void XPE::setOptionalHeader32S(XPE_DEF::IMAGE_OPTIONAL_HEADER32S *pOptionalHeader32S)
{
    // TODO check -1
    if (pOptionalHeader32S->SectionAlignment == 0) {
        pOptionalHeader32S->SectionAlignment = 1;
    }
    if (pOptionalHeader32S->FileAlignment == 0) {
        pOptionalHeader32S->FileAlignment = 1;
    }

    write_array(getOptionalHeaderOffset(), (char *)pOptionalHeader32S, sizeof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S));
}

void XPE::setOptionalHeader64S(XPE_DEF::IMAGE_OPTIONAL_HEADER64S *pOptionalHeader64S)
{
    // TODO check -1
    if (pOptionalHeader64S->SectionAlignment == 0) {
        pOptionalHeader64S->SectionAlignment = 1;
    }
    if (pOptionalHeader64S->FileAlignment == 0) {
        pOptionalHeader64S->FileAlignment = 1;
    }

    write_array(getOptionalHeaderOffset(), (char *)pOptionalHeader64S, sizeof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S));
}

quint16 XPE::getOptionalHeader_Magic()
{
    return read_uint16(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, Magic));
}

quint8 XPE::getOptionalHeader_MajorLinkerVersion()
{
    return read_uint8(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, MajorLinkerVersion));
}

quint8 XPE::getOptionalHeader_MinorLinkerVersion()
{
    return read_uint8(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, MinorLinkerVersion));
}

quint32 XPE::getOptionalHeader_SizeOfCode()
{
    return read_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, SizeOfCode));
}

quint32 XPE::getOptionalHeader_SizeOfInitializedData()
{
    return read_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, SizeOfInitializedData));
}

quint32 XPE::getOptionalHeader_SizeOfUninitializedData()
{
    return read_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, SizeOfUninitializedData));
}

quint32 XPE::getOptionalHeader_AddressOfEntryPoint()
{
    return read_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, AddressOfEntryPoint));
}

quint32 XPE::getOptionalHeader_BaseOfCode()
{
    return read_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, BaseOfCode));
}

quint32 XPE::getOptionalHeader_BaseOfData()
{
    if (is32()) {
        return read_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, BaseOfData));
    }

    return 0;
}

quint64 XPE::getOptionalHeader_ImageBase()
{
    quint64 nResult = 0;

    if (is64()) {
        nResult = read_uint64(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, ImageBase));
    } else {
        nResult = read_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, ImageBase));
    }

    return nResult;
}

quint32 XPE::getOptionalHeader_SectionAlignment()
{
    qint64 nOffset = getOptionalHeaderOffset();
    quint32 nResult = 0;

    if (nOffset != -1) {
        if (is64()) {
            nResult = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, SectionAlignment));
        } else {
            nResult = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, SectionAlignment));
        }
    }

    if (nResult == 0) {
        nResult = 1;
    }

    return nResult;
}

quint32 XPE::getOptionalHeader_FileAlignment()
{
    qint64 nOffset = getOptionalHeaderOffset();
    quint32 nResult = 0;

    if (nOffset != -1) {
        if (is64()) {
            nResult = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, FileAlignment));
        } else {
            nResult = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, FileAlignment));
        }
    }

    if (nResult == 0) {
        nResult = 1;
    }

    return nResult;
}

quint16 XPE::getOptionalHeader_MajorOperatingSystemVersion()
{
    if (is64()) {
        return read_uint16(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, MajorOperatingSystemVersion));
    }

    return read_uint16(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, MajorOperatingSystemVersion));
}

quint16 XPE::getOptionalHeader_MinorOperatingSystemVersion()
{
    if (is64()) {
        return read_uint16(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, MinorOperatingSystemVersion));
    }

    return read_uint16(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, MinorOperatingSystemVersion));
}

quint16 XPE::getOptionalHeader_MajorImageVersion()
{
    if (is64()) {
        return read_uint16(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, MajorImageVersion));
    }

    return read_uint16(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, MajorImageVersion));
}

quint16 XPE::getOptionalHeader_MinorImageVersion()
{
    if (is64()) {
        return read_uint16(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, MinorImageVersion));
    }

    return read_uint16(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, MinorImageVersion));
}

quint16 XPE::getOptionalHeader_MajorSubsystemVersion()
{
    if (is64()) {
        return read_uint16(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, MajorSubsystemVersion));
    }

    return read_uint16(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, MajorSubsystemVersion));
}

quint16 XPE::getOptionalHeader_MinorSubsystemVersion()
{
    if (is64()) {
        return read_uint16(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, MinorSubsystemVersion));
    }

    return read_uint16(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, MinorSubsystemVersion));
}

quint32 XPE::getOptionalHeader_Win32VersionValue()
{
    if (is64()) {
        return read_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, Win32VersionValue));
    }

    return read_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, Win32VersionValue));
}

quint32 XPE::getOptionalHeader_SizeOfImage()
{
    if (is64()) {
        return read_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, SizeOfImage));
    }

    return read_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, SizeOfImage));
}

quint32 XPE::getOptionalHeader_SizeOfHeaders()
{
    if (is64()) {
        return read_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, SizeOfHeaders));
    }

    return read_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, SizeOfHeaders));
}

quint32 XPE::getOptionalHeader_CheckSum()
{
    if (is64()) {
        return read_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, CheckSum));
    }

    return read_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, CheckSum));
}

quint16 XPE::getOptionalHeader_Subsystem()
{
    if (is64()) {
        return read_uint16(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, Subsystem));
    }

    return read_uint16(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, Subsystem));
}

quint16 XPE::getOptionalHeader_DllCharacteristics()
{
    if (is64()) {
        return read_uint16(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, DllCharacteristics));
    }

    return read_uint16(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, DllCharacteristics));
}

qint64 XPE::getOptionalHeader_SizeOfStackReserve()
{
    qint64 nResult = 0;

    if (is64()) {
        nResult = read_uint64(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, SizeOfStackReserve));
    } else {
        nResult = read_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, SizeOfStackReserve));
    }

    return nResult;
}

qint64 XPE::getOptionalHeader_SizeOfStackCommit()
{
    qint64 nResult = 0;

    if (is64()) {
        nResult = read_uint64(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, SizeOfStackCommit));
    } else {
        nResult = read_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, SizeOfStackCommit));
    }

    return nResult;
}

qint64 XPE::getOptionalHeader_SizeOfHeapReserve()
{
    qint64 nResult = 0;

    if (is64()) {
        nResult = read_uint64(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, SizeOfHeapReserve));
    } else {
        nResult = read_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, SizeOfHeapReserve));
    }

    return nResult;
}

qint64 XPE::getOptionalHeader_SizeOfHeapCommit()
{
    qint64 nResult = 0;

    if (is64()) {
        nResult = read_uint64(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, SizeOfHeapCommit));
    } else {
        nResult = read_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, SizeOfHeapCommit));
    }

    return nResult;
}

quint32 XPE::getOptionalHeader_LoaderFlags()
{
    quint32 nResult = 0;

    if (is64()) {
        nResult = read_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, LoaderFlags));
    } else {
        nResult = read_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, LoaderFlags));
    }

    return nResult;
}

quint32 XPE::getOptionalHeader_NumberOfRvaAndSizes()
{
    quint32 nResult = 0;

    if (is64()) {
        nResult = read_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, NumberOfRvaAndSizes));
    } else {
        nResult = read_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, NumberOfRvaAndSizes));
    }

    return nResult;
}

void XPE::setOptionalHeader_Magic(quint16 nValue)
{
    if (is64()) {
        write_uint16(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, Magic), nValue);
    } else {
        write_uint16(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, Magic), nValue);
    }
}

void XPE::setOptionalHeader_MajorLinkerVersion(quint8 nValue)
{
    if (is64()) {
        write_uint8(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, MajorLinkerVersion), nValue);
    } else {
        write_uint8(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, MajorLinkerVersion), nValue);
    }
}

void XPE::setOptionalHeader_MinorLinkerVersion(quint8 nValue)
{
    if (is64()) {
        write_uint8(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, MinorLinkerVersion), nValue);
    } else {
        write_uint8(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, MinorLinkerVersion), nValue);
    }
}

void XPE::setOptionalHeader_SizeOfCode(quint32 nValue)
{
    if (is64()) {
        write_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, SizeOfCode), nValue);
    } else {
        write_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, SizeOfCode), nValue);
    }
}

void XPE::setOptionalHeader_SizeOfInitializedData(quint32 nValue)
{
    if (is64()) {
        write_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, SizeOfInitializedData), nValue);
    } else {
        write_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, SizeOfInitializedData), nValue);
    }
}

void XPE::setOptionalHeader_SizeOfUninitializedData(quint32 nValue)
{
    if (is64()) {
        write_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, SizeOfUninitializedData), nValue);
    } else {
        write_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, SizeOfUninitializedData), nValue);
    }
}

void XPE::setOptionalHeader_AddressOfEntryPoint(quint32 nValue)
{
    if (is64()) {
        write_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, AddressOfEntryPoint), nValue);
    } else {
        write_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, AddressOfEntryPoint), nValue);
    }
}

void XPE::setOptionalHeader_BaseOfCode(quint32 nValue)
{
    if (is64()) {
        write_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, BaseOfCode), nValue);
    } else {
        write_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, BaseOfCode), nValue);
    }
}

void XPE::setOptionalHeader_BaseOfData(quint32 nValue)
{
    if (is64())  // There is no BaseOfData for PE64
    {
        // TODO error string
    } else {
        write_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, BaseOfData), nValue);
    }
}

void XPE::setOptionalHeader_ImageBase(quint64 nValue)
{
    if (is64()) {
        write_uint64(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, ImageBase), nValue);
    } else {
        write_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, ImageBase), nValue);
    }
}

void XPE::setOptionalHeader_SectionAlignment(quint32 nValue)
{
    if (nValue == 0) {
        nValue = 1;
    }

    if (is64()) {
        write_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, SectionAlignment), nValue);
    } else {
        write_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, SectionAlignment), nValue);
    }
}

void XPE::setOptionalHeader_FileAlignment(quint32 nValue)
{
    if (nValue == 0) {
        nValue = 1;
    }

    if (is64()) {
        write_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, FileAlignment), nValue);
    } else {
        write_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, FileAlignment), nValue);
    }
}

void XPE::setOptionalHeader_MajorOperatingSystemVersion(quint16 nValue)
{
    if (is64()) {
        write_uint16(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, MajorOperatingSystemVersion), nValue);
    } else {
        write_uint16(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, MajorOperatingSystemVersion), nValue);
    }
}

void XPE::setOptionalHeader_MinorOperatingSystemVersion(quint16 nValue)
{
    if (is64()) {
        write_uint16(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, MinorOperatingSystemVersion), nValue);
    } else {
        write_uint16(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, MinorOperatingSystemVersion), nValue);
    }
}

void XPE::setOptionalHeader_MajorImageVersion(quint16 nValue)
{
    if (is64()) {
        write_uint16(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, MajorImageVersion), nValue);
    } else {
        write_uint16(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, MajorImageVersion), nValue);
    }
}

void XPE::setOptionalHeader_MinorImageVersion(quint16 nValue)
{
    if (is64()) {
        write_uint16(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, MinorImageVersion), nValue);
    } else {
        write_uint16(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, MinorImageVersion), nValue);
    }
}

void XPE::setOptionalHeader_MajorSubsystemVersion(quint16 nValue)
{
    if (is64()) {
        write_uint16(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, MajorSubsystemVersion), nValue);
    } else {
        write_uint16(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, MajorSubsystemVersion), nValue);
    }
}

void XPE::setOptionalHeader_MinorSubsystemVersion(quint16 nValue)
{
    if (is64()) {
        write_uint16(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, MinorSubsystemVersion), nValue);
    } else {
        write_uint16(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, MinorSubsystemVersion), nValue);
    }
}

void XPE::setOptionalHeader_Win32VersionValue(quint32 nValue)
{
    if (is64()) {
        write_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, Win32VersionValue), nValue);
    } else {
        write_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, Win32VersionValue), nValue);
    }
}

void XPE::setOptionalHeader_SizeOfImage(quint32 nValue)
{
    if (is64()) {
        write_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, SizeOfImage), nValue);
    } else {
        write_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, SizeOfImage), nValue);
    }
}

void XPE::setOptionalHeader_SizeOfHeaders(quint32 nValue)
{
    if (is64()) {
        write_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, SizeOfHeaders), nValue);
    } else {
        write_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, SizeOfHeaders), nValue);
    }
}

void XPE::setOptionalHeader_CheckSum(quint32 nValue)
{
    if (is64()) {
        write_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, CheckSum), nValue);
    } else {
        write_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, CheckSum), nValue);
    }
}

void XPE::setOptionalHeader_Subsystem(quint16 nValue)
{
    if (is64()) {
        write_uint16(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, Subsystem), nValue);
    } else {
        write_uint16(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, Subsystem), nValue);
    }
}

void XPE::setOptionalHeader_DllCharacteristics(quint16 nValue)
{
    if (is64()) {
        write_uint16(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, DllCharacteristics), nValue);
    } else {
        write_uint16(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, DllCharacteristics), nValue);
    }
}

void XPE::setOptionalHeader_SizeOfStackReserve(quint64 nValue)
{
    if (is64()) {
        write_uint64(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, SizeOfStackReserve), nValue);
    } else {
        write_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, SizeOfStackReserve), nValue);
    }
}

void XPE::setOptionalHeader_SizeOfStackCommit(quint64 nValue)
{
    if (is64()) {
        write_uint64(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, SizeOfStackCommit), nValue);
    } else {
        write_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, SizeOfStackCommit), nValue);
    }
}

void XPE::setOptionalHeader_SizeOfHeapReserve(quint64 nValue)
{
    if (is64()) {
        write_uint64(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, SizeOfHeapReserve), nValue);
    } else {
        write_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, SizeOfHeapReserve), nValue);
    }
}

void XPE::setOptionalHeader_SizeOfHeapCommit(quint64 nValue)
{
    if (is64()) {
        write_uint64(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, SizeOfHeapCommit), nValue);
    } else {
        write_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, SizeOfHeapCommit), nValue);
    }
}

void XPE::setOptionalHeader_LoaderFlags(quint32 nValue)
{
    if (is64()) {
        write_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, LoaderFlags), nValue);
    } else {
        write_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, LoaderFlags), nValue);
    }
}

void XPE::setOptionalHeader_NumberOfRvaAndSizes(quint32 nValue)
{
    if (is64()) {
        write_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, NumberOfRvaAndSizes), nValue);
    } else {
        write_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, NumberOfRvaAndSizes), nValue);
    }
}

quint32 XPE::getOperatingSystemVersion()
{
    qint64 nOptionalHeaderOffset = getOptionalHeaderOffset();

    quint16 nValue1 = 0;
    quint16 nValue2 = 0;

    if (is64()) {
        nValue1 = read_uint16(nOptionalHeaderOffset + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, MajorOperatingSystemVersion));
        nValue2 = read_uint16(nOptionalHeaderOffset + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, MinorOperatingSystemVersion));
    } else {
        nValue1 = read_uint16(nOptionalHeaderOffset + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, MajorOperatingSystemVersion));
        nValue2 = read_uint16(nOptionalHeaderOffset + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, MinorOperatingSystemVersion));
    }

    return make_dword(nValue1, nValue2);
}

void XPE::setOperatingSystemVersion(quint32 nValue)
{
    XDWORD xdword = make_xdword(nValue);

    qint64 nOptionalHeaderOffset = getOptionalHeaderOffset();

    if (is64()) {
        write_uint16(nOptionalHeaderOffset + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, MajorOperatingSystemVersion), xdword.nValue1);
        write_uint16(nOptionalHeaderOffset + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, MinorOperatingSystemVersion), xdword.nValue2);
    } else {
        write_uint16(nOptionalHeaderOffset + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, MajorOperatingSystemVersion), xdword.nValue1);
        write_uint16(nOptionalHeaderOffset + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, MinorOperatingSystemVersion), xdword.nValue2);
    }
}

XPE_DEF::IMAGE_DATA_DIRECTORY XPE::read_IMAGE_DATA_DIRECTORY(qint64 nOffset)
{
    XPE_DEF::IMAGE_DATA_DIRECTORY result = {};

    result.VirtualAddress = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_DATA_DIRECTORY, VirtualAddress));
    result.Size = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_DATA_DIRECTORY, Size));

    return result;
}

void XPE::write_IMAGE_DATA_DIRECTORY(qint64 nOffset, XPE_DEF::IMAGE_DATA_DIRECTORY *pDataDirectory)
{
    write_uint32(nOffset + offsetof(XPE_DEF::IMAGE_DATA_DIRECTORY, VirtualAddress), pDataDirectory->VirtualAddress);
    write_uint32(nOffset + offsetof(XPE_DEF::IMAGE_DATA_DIRECTORY, Size), pDataDirectory->Size);
}

XPE_DEF::IMAGE_DATA_DIRECTORY XPE::getOptionalHeader_DataDirectory(quint32 nNumber)
{
    XPE_DEF::IMAGE_DATA_DIRECTORY result = {};

    //    if(nNumber<getOptionalHeader_NumberOfRvaAndSizes()) // There are some
    //    protectors with false NumberOfRvaAndSizes
    if (nNumber < 16) {
        if (is64()) {
            result = read_IMAGE_DATA_DIRECTORY(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, DataDirectory) +
                                               nNumber * sizeof(XPE_DEF::IMAGE_DATA_DIRECTORY));
        } else {
            result = read_IMAGE_DATA_DIRECTORY(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, DataDirectory) +
                                               nNumber * sizeof(XPE_DEF::IMAGE_DATA_DIRECTORY));
        }
    }

    return result;
}

void XPE::setOptionalHeader_DataDirectory(quint32 nNumber, XPE_DEF::IMAGE_DATA_DIRECTORY *pDataDirectory)
{
    //    if(nNumber<16)
    if (nNumber < getOptionalHeader_NumberOfRvaAndSizes())  // TODO Check!!!
    {
        if (is64()) {
            write_IMAGE_DATA_DIRECTORY(
                getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, DataDirectory) + nNumber * sizeof(XPE_DEF::IMAGE_DATA_DIRECTORY), pDataDirectory);
        } else {
            write_IMAGE_DATA_DIRECTORY(
                getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, DataDirectory) + nNumber * sizeof(XPE_DEF::IMAGE_DATA_DIRECTORY), pDataDirectory);
        }
    }
}

void XPE::setOptionalHeader_DataDirectory_VirtualAddress(quint32 nNumber, quint32 nValue)
{
    if (nNumber < getOptionalHeader_NumberOfRvaAndSizes()) {
        if (is64()) {
            write_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, DataDirectory) + nNumber * sizeof(XPE_DEF::IMAGE_DATA_DIRECTORY) +
                             offsetof(XPE_DEF::IMAGE_DATA_DIRECTORY, VirtualAddress),
                         nValue);
        } else {
            write_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, DataDirectory) + nNumber * sizeof(XPE_DEF::IMAGE_DATA_DIRECTORY) +
                             offsetof(XPE_DEF::IMAGE_DATA_DIRECTORY, VirtualAddress),
                         nValue);
        }
    }
}

void XPE::setOptionalHeader_DataDirectory_Size(quint32 nNumber, quint32 nValue)
{
    if (nNumber < getOptionalHeader_NumberOfRvaAndSizes()) {
        if (is64()) {
            write_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, DataDirectory) + nNumber * sizeof(XPE_DEF::IMAGE_DATA_DIRECTORY) +
                             offsetof(XPE_DEF::IMAGE_DATA_DIRECTORY, Size),
                         nValue);
        } else {
            write_uint32(getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, DataDirectory) + nNumber * sizeof(XPE_DEF::IMAGE_DATA_DIRECTORY) +
                             offsetof(XPE_DEF::IMAGE_DATA_DIRECTORY, Size),
                         nValue);
        }
    }
}

void XPE::clearOptionalHeader_DataDirectory(quint32 nNumber)
{
    XPE_DEF::IMAGE_DATA_DIRECTORY dd = {};

    setOptionalHeader_DataDirectory(nNumber, &dd);
}

bool XPE::isOptionalHeader_DataDirectoryPresent(quint32 nNumber)
{
    XPE_DEF::IMAGE_DATA_DIRECTORY dd = getOptionalHeader_DataDirectory(nNumber);

    //    return
    //    (dd.Size)&&(dd.VirtualAddress)&&(isAddressValid(dd.VirtualAddress+getBaseAddress()));
    //    // TODO Check return (dd.Size)&&(dd.VirtualAddress);
    // TODO more checks
    return (dd.VirtualAddress);
}

QList<XPE_DEF::IMAGE_DATA_DIRECTORY> XPE::getDirectories()
{
    QList<XPE_DEF::IMAGE_DATA_DIRECTORY> listResult;

    qint32 nNumberNumberOfRvaAndSizes = getOptionalHeader_NumberOfRvaAndSizes();
    nNumberNumberOfRvaAndSizes = qMin(nNumberNumberOfRvaAndSizes, 16);

    qint64 nDirectoriesOffset = getOptionalHeaderOffset();

    if (is64()) {
        nDirectoriesOffset += offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, DataDirectory);
    } else {
        nDirectoriesOffset += offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, DataDirectory);
    }

    for (qint32 i = 0; i < nNumberNumberOfRvaAndSizes; i++) {
        XPE_DEF::IMAGE_DATA_DIRECTORY record = {};

        read_array(nDirectoriesOffset + i * sizeof(XPE_DEF::IMAGE_DATA_DIRECTORY), (char *)&record, sizeof(XPE_DEF::IMAGE_DATA_DIRECTORY));

        listResult.append(record);
    }

    return listResult;
}

void XPE::setDirectories(QList<XPE_DEF::IMAGE_DATA_DIRECTORY> *pListDirectories)
{
    qint32 nNumberOfRvaAndSizes = getOptionalHeader_NumberOfRvaAndSizes();
    nNumberOfRvaAndSizes = qMin(nNumberOfRvaAndSizes, 16);

    qint64 nDirectoriesOffset = getOptionalHeaderOffset();

    if (is64()) {
        nDirectoriesOffset += offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, DataDirectory);
    } else {
        nDirectoriesOffset += offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, DataDirectory);
    }

    for (qint32 i = 0; i < nNumberOfRvaAndSizes; i++) {
        write_array(nDirectoriesOffset + i * sizeof(XPE_DEF::IMAGE_DATA_DIRECTORY), (char *)&(pListDirectories->at(i)), sizeof(XPE_DEF::IMAGE_DATA_DIRECTORY));
    }
}

qint64 XPE::getDataDirectoryOffset(quint32 nNumber)
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return getDataDirectoryOffset(&memoryMap, nNumber);
}

qint64 XPE::getDataDirectoryOffset(XBinary::_MEMORY_MAP *pMemoryMap, quint32 nNumber)
{
    qint64 nResult = -1;

    XPE_DEF::IMAGE_DATA_DIRECTORY dataResources = getOptionalHeader_DataDirectory(nNumber);

    if (dataResources.VirtualAddress) {
        nResult = addressToOffset(pMemoryMap, dataResources.VirtualAddress + pMemoryMap->nModuleAddress);
    }

    return nResult;
}

qint64 XPE::getDataDirectoryHeaderOffset(quint32 nNumber)
{
    qint64 nResult = -1;

    if (nNumber < 16) {
        if (is64()) {
            nResult = getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, DataDirectory) + nNumber * sizeof(XPE_DEF::IMAGE_DATA_DIRECTORY);
        } else {
            nResult = getOptionalHeaderOffset() + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, DataDirectory) + nNumber * sizeof(XPE_DEF::IMAGE_DATA_DIRECTORY);
        }
    }

    return nResult;
}

qint64 XPE::getDataDirectoryHeaderSize()
{
    return sizeof(XPE_DEF::IMAGE_DATA_DIRECTORY);
}

QByteArray XPE::getDataDirectory(quint32 nNumber)
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return getDataDirectory(&memoryMap, nNumber);
}

QByteArray XPE::getDataDirectory(XBinary::_MEMORY_MAP *pMemoryMap, quint32 nNumber)
{
    QByteArray baResult;

    XPE_DEF::IMAGE_DATA_DIRECTORY dataDirectory = getOptionalHeader_DataDirectory(nNumber);

    if (dataDirectory.VirtualAddress) {
        qint64 nOffset = addressToOffset(pMemoryMap, dataDirectory.VirtualAddress + pMemoryMap->nModuleAddress);

        if (nOffset != -1) {
            baResult = read_array(nOffset, dataDirectory.Size);
        }
    }

    return baResult;
}

XBinary::OFFSETSIZE XPE::getStringTable()
{
    OFFSETSIZE result = {};

    qint64 nOffset = getFileHeader_PointerToSymbolTable() + getFileHeader_NumberOfSymbols() * 18;

    if (nOffset > 0) {
        qint64 nSize = getSize() - nOffset;

        if (nSize >= 4) {
            quint32 nStringTableSize = read_uint32(nOffset);

            if ((nStringTableSize > 0) && (nStringTableSize <= nSize)) {
                result.nOffset = nOffset;
                result.nSize = nStringTableSize;
            }
        }
    }

    return result;
}

qint64 XPE::getSectionsTableOffset()
{
    qint64 nResult = -1;

    qint64 nOptionalHeaderOffset = getOptionalHeaderOffset();

    if (nOptionalHeaderOffset != -1) {
        nResult = nOptionalHeaderOffset + getFileHeader_SizeOfOptionalHeader();
    }

    return nResult;
}

qint64 XPE::getSectionHeaderOffset(quint32 nNumber)
{
    qint64 nResult = -1;

    quint32 nNumberOfSections = getFileHeader_NumberOfSections();

    if (nNumber < nNumberOfSections) {
        qint64 nSectionsTableOffset = getSectionsTableOffset();

        if (nSectionsTableOffset != -1) {
            nResult = nSectionsTableOffset + nNumber * sizeof(XPE_DEF::IMAGE_SECTION_HEADER);
        }
    }

    return nResult;
}

qint64 XPE::getSectionHeaderSize()
{
    return sizeof(XPE_DEF::IMAGE_SECTION_HEADER);
}

bool XPE::isSectionsTablePresent()
{
    return (bool)getFileHeader_NumberOfSections();
}

XPE_DEF::IMAGE_SECTION_HEADER XPE::getSectionHeader(quint32 nNumber)
{
    // TODO
    XPE_DEF::IMAGE_SECTION_HEADER result = {};

    quint32 nNumberOfSections = getFileHeader_NumberOfSections();

    if (nNumber < nNumberOfSections) {
        // TODO
        read_array(getSectionsTableOffset() + nNumber * sizeof(XPE_DEF::IMAGE_SECTION_HEADER), (char *)&result, sizeof(XPE_DEF::IMAGE_SECTION_HEADER));
    }

    return result;
}

void XPE::setSectionHeader(quint32 nNumber, XPE_DEF::IMAGE_SECTION_HEADER *pSectionHeader)
{
    // TODO
    quint32 nNumberOfSections = getFileHeader_NumberOfSections();

    if (nNumber < nNumberOfSections) {
        write_array(getSectionsTableOffset() + nNumber * sizeof(XPE_DEF::IMAGE_SECTION_HEADER), (char *)pSectionHeader, sizeof(XPE_DEF::IMAGE_SECTION_HEADER));
    }
}

XPE_DEF::IMAGE_SECTION_HEADER XPE::read_IMAGE_SECTION_HEADER(qint64 nOffset)
{
    XPE_DEF::IMAGE_SECTION_HEADER result = {};

    read_array(nOffset, (char *)(result.Name), sizeof(XPE_DEF::IMAGE_SECTION_HEADER));
    result.Misc.VirtualSize = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_SECTION_HEADER, Misc.VirtualSize));
    result.VirtualAddress = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_SECTION_HEADER, VirtualAddress));
    result.SizeOfRawData = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_SECTION_HEADER, SizeOfRawData));
    result.PointerToRawData = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_SECTION_HEADER, PointerToRawData));
    result.PointerToRelocations = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_SECTION_HEADER, PointerToRelocations));
    result.PointerToLinenumbers = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_SECTION_HEADER, PointerToLinenumbers));
    result.NumberOfRelocations = read_uint16(nOffset + offsetof(XPE_DEF::IMAGE_SECTION_HEADER, NumberOfRelocations));
    result.NumberOfLinenumbers = read_uint16(nOffset + offsetof(XPE_DEF::IMAGE_SECTION_HEADER, NumberOfLinenumbers));
    result.Characteristics = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_SECTION_HEADER, Characteristics));

    return result;
}

QList<XPE_DEF::IMAGE_SECTION_HEADER> XPE::getSectionHeaders(PDSTRUCT *pPdStruct)
{
    QList<XPE_DEF::IMAGE_SECTION_HEADER> listResult;

    quint32 nNumberOfSections = getFileHeader_NumberOfSections();
    qint64 nSectionOffset = getSectionsTableOffset();

    // Fix
    if (nNumberOfSections > XPE_DEF::S_MAX_SECTIONCOUNT)  // TODO const
    {
        nNumberOfSections = XPE_DEF::S_MAX_SECTIONCOUNT;
    }

    for (qint32 i = 0; (i < (int)nNumberOfSections) && isPdStructNotCanceled(pPdStruct); i++) {
        XPE_DEF::IMAGE_SECTION_HEADER record = {};

        read_array(nSectionOffset + i * sizeof(XPE_DEF::IMAGE_SECTION_HEADER), (char *)&record, sizeof(XPE_DEF::IMAGE_SECTION_HEADER));

        listResult.append(record);
    }

    return listResult;
}

QList<XPE::SECTION_RECORD> XPE::getSectionRecords(QList<XPE_DEF::IMAGE_SECTION_HEADER> *pListSectionHeaders, PDSTRUCT *pPdStruct)
{
    QList<SECTION_RECORD> listResult;

    bool bIsImage = isImage();
    OFFSETSIZE osStringTable = getStringTable();

    qint32 nNumberOfSections = pListSectionHeaders->count();

    for (qint32 i = 0; (i < nNumberOfSections) && isPdStructNotCanceled(pPdStruct); i++) {
        SECTION_RECORD record = {};

        record.sName = QString::fromLatin1((const char *)pListSectionHeaders->at(i).Name, qstrnlen((const char *)pListSectionHeaders->at(i).Name, 8));
        record.sName.resize(qMin(record.sName.length(), XPE_DEF::S_IMAGE_SIZEOF_SHORT_NAME));

        record.sName = convertSectionName(record.sName, &osStringTable);

        if (bIsImage) {
            record.nOffset = pListSectionHeaders->at(i).VirtualAddress;
        } else {
            record.nOffset = pListSectionHeaders->at(i).PointerToRawData;
        }
        record.nRVA = pListSectionHeaders->at(i).VirtualAddress;

        record.nSize = pListSectionHeaders->at(i).SizeOfRawData;
        record.nCharacteristics = pListSectionHeaders->at(i).Characteristics;

        listResult.append(record);
    }

    return listResult;
}

QList<QString> XPE::getSectionNames(QList<XPE::SECTION_RECORD> *pListSectionRecords, PDSTRUCT *pPdStruct)
{
    QList<QString> listResult;

    qint32 nNumberOfSections = pListSectionRecords->count();

    for (qint32 i = 0; (i < nNumberOfSections) && isPdStructNotCanceled(pPdStruct); i++) {
        listResult.append(pListSectionRecords->at(i).sName);
    }

    return listResult;
}

QList<XPE::SECTIONRVA_RECORD> XPE::getSectionRVARecords()
{
    QList<SECTIONRVA_RECORD> listResult;

    QList<XPE_DEF::IMAGE_SECTION_HEADER> listSH = getSectionHeaders();
    qint32 nSectionAlignment = getOptionalHeader_SectionAlignment();

    qint32 nNumberOfSections = listSH.count();

    for (qint32 i = 0; i < nNumberOfSections; i++) {
        SECTIONRVA_RECORD record = {};

        record.nRVA = listSH.at(i).VirtualAddress;
        record.nSize = S_ALIGN_UP(listSH.at(i).Misc.VirtualSize, nSectionAlignment);
        record.nCharacteristics = listSH.at(i).Characteristics;

        listResult.append(record);
    }

    return listResult;
}

QString XPE::convertSectionName(const QString &sName, OFFSETSIZE *pOsStringTable)
{
    QString sResult = sName;

    if (sName.size() > 1) {
        if (sName.at(0) == QChar('/')) {
            qint32 nIndex = sName.section("/", 1, -1).toInt();

            sResult = getStringFromIndex(pOsStringTable->nOffset, pOsStringTable->nSize, nIndex);

            if (sResult == "") {
                sResult = sName;
            }
        }
    }

    return sResult;
}

QString XPE::getSection_NameAsString(quint32 nNumber)
{
    QString sResult;

    quint32 nNumberOfSections = getFileHeader_NumberOfSections();

    char cBuffer[9] = {};

    if (nNumber < nNumberOfSections) {
        XBinary::read_array(getSectionsTableOffset() + nNumber * sizeof(XPE_DEF::IMAGE_SECTION_HEADER) + offsetof(XPE_DEF::IMAGE_SECTION_HEADER, Name), cBuffer, 8);
    }

    sResult.append(cBuffer);

    return sResult;
}

quint32 XPE::getSection_VirtualSize(quint32 nNumber)
{
    quint32 nResult = 0;

    quint32 nNumberOfSections = getFileHeader_NumberOfSections();

    if (nNumber < nNumberOfSections) {
        nResult = read_uint32(getSectionsTableOffset() + nNumber * sizeof(XPE_DEF::IMAGE_SECTION_HEADER) + offsetof(XPE_DEF::IMAGE_SECTION_HEADER, Misc.VirtualSize));
    }

    return nResult;
}

quint32 XPE::getSection_VirtualAddress(quint32 nNumber)
{
    quint32 nResult = 0;

    quint32 nNumberOfSections = getFileHeader_NumberOfSections();

    if (nNumber < nNumberOfSections) {
        nResult = read_uint32(getSectionsTableOffset() + nNumber * sizeof(XPE_DEF::IMAGE_SECTION_HEADER) + offsetof(XPE_DEF::IMAGE_SECTION_HEADER, VirtualAddress));
    }

    return nResult;
}

quint32 XPE::getSection_SizeOfRawData(quint32 nNumber)
{
    quint32 nResult = 0;

    quint32 nNumberOfSections = getFileHeader_NumberOfSections();

    if (nNumber < nNumberOfSections) {
        nResult = read_uint32(getSectionsTableOffset() + nNumber * sizeof(XPE_DEF::IMAGE_SECTION_HEADER) + offsetof(XPE_DEF::IMAGE_SECTION_HEADER, SizeOfRawData));
    }

    return nResult;
}

quint32 XPE::getSection_PointerToRawData(quint32 nNumber)
{
    quint32 nResult = 0;

    quint32 nNumberOfSections = getFileHeader_NumberOfSections();

    if (nNumber < nNumberOfSections) {
        nResult = read_uint32(getSectionsTableOffset() + nNumber * sizeof(XPE_DEF::IMAGE_SECTION_HEADER) + offsetof(XPE_DEF::IMAGE_SECTION_HEADER, PointerToRawData));
    }

    return nResult;
}

quint32 XPE::getSection_PointerToRelocations(quint32 nNumber)
{
    quint32 nResult = 0;

    quint32 nNumberOfSections = getFileHeader_NumberOfSections();

    if (nNumber < nNumberOfSections) {
        nResult = read_uint32(getSectionsTableOffset() + nNumber * sizeof(XPE_DEF::IMAGE_SECTION_HEADER) + offsetof(XPE_DEF::IMAGE_SECTION_HEADER, PointerToRelocations));
    }

    return nResult;
}

quint32 XPE::getSection_PointerToLinenumbers(quint32 nNumber)
{
    quint32 nResult = 0;

    quint32 nNumberOfSections = getFileHeader_NumberOfSections();

    if (nNumber < nNumberOfSections) {
        nResult = read_uint32(getSectionsTableOffset() + nNumber * sizeof(XPE_DEF::IMAGE_SECTION_HEADER) + offsetof(XPE_DEF::IMAGE_SECTION_HEADER, PointerToLinenumbers));
    }

    return nResult;
}

quint16 XPE::getSection_NumberOfRelocations(quint32 nNumber)
{
    quint16 nResult = 0;

    quint32 nNumberOfSections = getFileHeader_NumberOfSections();

    if (nNumber < nNumberOfSections) {
        nResult = read_uint16(getSectionsTableOffset() + nNumber * sizeof(XPE_DEF::IMAGE_SECTION_HEADER) + offsetof(XPE_DEF::IMAGE_SECTION_HEADER, NumberOfRelocations));
    }

    return nResult;
}

quint16 XPE::getSection_NumberOfLinenumbers(quint32 nNumber)
{
    quint16 nResult = 0;

    quint32 nNumberOfSections = getFileHeader_NumberOfSections();

    if (nNumber < nNumberOfSections) {
        nResult = read_uint16(getSectionsTableOffset() + nNumber * sizeof(XPE_DEF::IMAGE_SECTION_HEADER) + offsetof(XPE_DEF::IMAGE_SECTION_HEADER, NumberOfLinenumbers));
    }

    return nResult;
}

quint32 XPE::getSection_Characteristics(quint32 nNumber)
{
    quint32 nResult = 0;

    quint32 nNumberOfSections = getFileHeader_NumberOfSections();

    if (nNumber < nNumberOfSections) {
        nResult = read_uint32(getSectionsTableOffset() + nNumber * sizeof(XPE_DEF::IMAGE_SECTION_HEADER) + offsetof(XPE_DEF::IMAGE_SECTION_HEADER, Characteristics));
    }

    return nResult;
}

void XPE::setSection_NameAsString(quint32 nNumber, const QString &sName)
{
    quint32 nNumberOfSections = getFileHeader_NumberOfSections();

    if (nNumber < nNumberOfSections) {
        write_ansiStringFix(getSectionsTableOffset() + nNumber * sizeof(XPE_DEF::IMAGE_SECTION_HEADER) + offsetof(XPE_DEF::IMAGE_SECTION_HEADER, Name), 8, sName);
    }
}

void XPE::setSection_VirtualSize(quint32 nNumber, quint32 nValue)
{
    quint32 nNumberOfSections = getFileHeader_NumberOfSections();

    if (nNumber < nNumberOfSections) {
        write_uint32(getSectionsTableOffset() + nNumber * sizeof(XPE_DEF::IMAGE_SECTION_HEADER) + offsetof(XPE_DEF::IMAGE_SECTION_HEADER, Misc.VirtualSize), nValue);
    }
}

void XPE::setSection_VirtualAddress(quint32 nNumber, quint32 nValue)
{
    quint32 nNumberOfSections = getFileHeader_NumberOfSections();

    if (nNumber < nNumberOfSections) {
        write_uint32(getSectionsTableOffset() + nNumber * sizeof(XPE_DEF::IMAGE_SECTION_HEADER) + offsetof(XPE_DEF::IMAGE_SECTION_HEADER, VirtualAddress), nValue);
    }
}

void XPE::setSection_SizeOfRawData(quint32 nNumber, quint32 nValue)
{
    quint32 nNumberOfSections = getFileHeader_NumberOfSections();

    if (nNumber < nNumberOfSections) {
        write_uint32(getSectionsTableOffset() + nNumber * sizeof(XPE_DEF::IMAGE_SECTION_HEADER) + offsetof(XPE_DEF::IMAGE_SECTION_HEADER, SizeOfRawData), nValue);
    }
}

void XPE::setSection_PointerToRawData(quint32 nNumber, quint32 nValue)
{
    quint32 nNumberOfSections = getFileHeader_NumberOfSections();

    if (nNumber < nNumberOfSections) {
        write_uint32(getSectionsTableOffset() + nNumber * sizeof(XPE_DEF::IMAGE_SECTION_HEADER) + offsetof(XPE_DEF::IMAGE_SECTION_HEADER, PointerToRawData), nValue);
    }
}

void XPE::setSection_PointerToRelocations(quint32 nNumber, quint32 nValue)
{
    quint32 nNumberOfSections = getFileHeader_NumberOfSections();

    if (nNumber < nNumberOfSections) {
        write_uint32(getSectionsTableOffset() + nNumber * sizeof(XPE_DEF::IMAGE_SECTION_HEADER) + offsetof(XPE_DEF::IMAGE_SECTION_HEADER, PointerToRelocations), nValue);
    }
}

void XPE::setSection_PointerToLinenumbers(quint32 nNumber, quint32 nValue)
{
    quint32 nNumberOfSections = getFileHeader_NumberOfSections();

    if (nNumber < nNumberOfSections) {
        write_uint32(getSectionsTableOffset() + nNumber * sizeof(XPE_DEF::IMAGE_SECTION_HEADER) + offsetof(XPE_DEF::IMAGE_SECTION_HEADER, PointerToLinenumbers), nValue);
    }
}

void XPE::setSection_NumberOfRelocations(quint32 nNumber, quint16 nValue)
{
    quint32 nNumberOfSections = getFileHeader_NumberOfSections();

    if (nNumber < nNumberOfSections) {
        write_uint16(getSectionsTableOffset() + nNumber * sizeof(XPE_DEF::IMAGE_SECTION_HEADER) + offsetof(XPE_DEF::IMAGE_SECTION_HEADER, NumberOfRelocations), nValue);
    }
}

void XPE::setSection_NumberOfLinenumbers(quint32 nNumber, quint16 nValue)
{
    quint32 nNumberOfSections = getFileHeader_NumberOfSections();

    if (nNumber < nNumberOfSections) {
        write_uint16(getSectionsTableOffset() + nNumber * sizeof(XPE_DEF::IMAGE_SECTION_HEADER) + offsetof(XPE_DEF::IMAGE_SECTION_HEADER, NumberOfLinenumbers), nValue);
    }
}

void XPE::setSection_Characteristics(quint32 nNumber, quint32 nValue)
{
    quint32 nNumberOfSections = getFileHeader_NumberOfSections();

    if (nNumber < nNumberOfSections) {
        write_uint32(getSectionsTableOffset() + nNumber * sizeof(XPE_DEF::IMAGE_SECTION_HEADER) + offsetof(XPE_DEF::IMAGE_SECTION_HEADER, Characteristics), nValue);
    }
}

QString XPE::getSection_NameAsString(quint32 nNumber, QList<QString> *pListSectionNameStrings)
{
    QString sResult;

    if (nNumber < (quint32)pListSectionNameStrings->count()) {
        sResult = pListSectionNameStrings->at(nNumber);
    }

    return sResult;
}

quint32 XPE::getSection_VirtualSize(quint32 nNumber, QList<XPE_DEF::IMAGE_SECTION_HEADER> *pListSectionHeaders)
{
    quint32 nResult = 0;

    if (nNumber < (quint32)pListSectionHeaders->count()) {
        nResult = pListSectionHeaders->at(nNumber).Misc.VirtualSize;
    }

    return nResult;
}

quint32 XPE::getSection_VirtualAddress(quint32 nNumber, QList<XPE_DEF::IMAGE_SECTION_HEADER> *pListSectionHeaders)
{
    quint32 nResult = 0;

    if (nNumber < (quint32)pListSectionHeaders->count()) {
        nResult = pListSectionHeaders->at(nNumber).VirtualAddress;
    }

    return nResult;
}

quint32 XPE::getSection_SizeOfRawData(quint32 nNumber, QList<XPE_DEF::IMAGE_SECTION_HEADER> *pListSectionHeaders)
{
    quint32 nResult = 0;

    if (nNumber < (quint32)pListSectionHeaders->count()) {
        nResult = pListSectionHeaders->at(nNumber).SizeOfRawData;
    }

    return nResult;
}

quint32 XPE::getSection_PointerToRawData(quint32 nNumber, QList<XPE_DEF::IMAGE_SECTION_HEADER> *pListSectionHeaders)
{
    quint32 nResult = 0;

    if (nNumber < (quint32)pListSectionHeaders->count()) {
        nResult = pListSectionHeaders->at(nNumber).PointerToRawData;
    }

    return nResult;
}

quint32 XPE::getSection_PointerToRelocations(quint32 nNumber, QList<XPE_DEF::IMAGE_SECTION_HEADER> *pListSectionHeaders)
{
    quint32 nResult = 0;

    if (nNumber < (quint32)pListSectionHeaders->count()) {
        nResult = pListSectionHeaders->at(nNumber).PointerToRelocations;
    }

    return nResult;
}

quint32 XPE::getSection_PointerToLinenumbers(quint32 nNumber, QList<XPE_DEF::IMAGE_SECTION_HEADER> *pListSectionHeaders)
{
    quint32 nResult = 0;

    if (nNumber < (quint32)pListSectionHeaders->count()) {
        nResult = pListSectionHeaders->at(nNumber).PointerToLinenumbers;
    }

    return nResult;
}

quint16 XPE::getSection_NumberOfRelocations(quint32 nNumber, QList<XPE_DEF::IMAGE_SECTION_HEADER> *pListSectionHeaders)
{
    quint16 nResult = 0;

    if (nNumber < (quint32)pListSectionHeaders->count()) {
        nResult = pListSectionHeaders->at(nNumber).NumberOfRelocations;
    }

    return nResult;
}

quint16 XPE::getSection_NumberOfLinenumbers(quint32 nNumber, QList<XPE_DEF::IMAGE_SECTION_HEADER> *pListSectionHeaders)
{
    quint16 nResult = 0;

    if (nNumber < (quint32)pListSectionHeaders->count()) {
        nResult = pListSectionHeaders->at(nNumber).NumberOfLinenumbers;
    }

    return nResult;
}

quint32 XPE::getSection_Characteristics(quint32 nNumber, QList<XPE_DEF::IMAGE_SECTION_HEADER> *pListSectionHeaders)
{
    quint32 nResult = 0;

    if (nNumber < (quint32)pListSectionHeaders->count()) {
        nResult = pListSectionHeaders->at(nNumber).Characteristics;
    }

    return nResult;
}

bool XPE::isSectionNamePresent(const QString &sSectionName)
{
    QList<XPE_DEF::IMAGE_SECTION_HEADER> listSectionHeaders = getSectionHeaders();
    QList<SECTION_RECORD> listSectionRecords = getSectionRecords(&listSectionHeaders);

    return isSectionNamePresent(sSectionName, &listSectionRecords);
}

bool XPE::isSectionNamePresent(const QString &sSectionName, QList<SECTION_RECORD> *pListSectionRecords)
{
    bool bResult = false;

    qint32 nNumberOfSections = pListSectionRecords->count();

    for (qint32 i = 0; i < nNumberOfSections; i++) {
        if (pListSectionRecords->at(i).sName == sSectionName) {
            bResult = true;
            break;
        }
    }

    return bResult;
}

XPE::SECTION_RECORD XPE::getSectionRecordByName(const QString &sSectionName, QList<SECTION_RECORD> *pListSectionRecords)
{
    SECTION_RECORD result = {};

    qint32 nNumberOfSections = pListSectionRecords->count();

    for (qint32 i = 0; i < nNumberOfSections; i++) {
        if (pListSectionRecords->at(i).sName == sSectionName) {
            result = pListSectionRecords->at(i);

            break;
        }
    }

    return result;
}

qint32 XPE::getSectionNumber(const QString &sSectionName)
{
    QList<XPE_DEF::IMAGE_SECTION_HEADER> listSectionHeaders = getSectionHeaders();
    QList<SECTION_RECORD> listSectionRecords = getSectionRecords(&listSectionHeaders);

    return getSectionNumber(sSectionName, &listSectionRecords);
}

qint32 XPE::getSectionNumber(const QString &sSectionName, QList<SECTION_RECORD> *pListSectionRecords)
{
    qint32 nResult = -1;

    qint32 nNumberOfSections = pListSectionRecords->count();

    for (qint32 i = 0; i < nNumberOfSections; i++) {
        if (pListSectionRecords->at(i).sName == sSectionName) {
            nResult = i;
            break;
        }
    }

    return nResult;
}

QString XPE::sectionCharacteristicToString(quint32 nValue)
{
    QString sResult;

    if (nValue & XPE_DEF::S_IMAGE_SCN_MEM_READ) {
        sResult += "R";
    }

    if (nValue & XPE_DEF::S_IMAGE_SCN_MEM_WRITE) {
        sResult += "W";
    }

    if (nValue & XPE_DEF::S_IMAGE_SCN_MEM_EXECUTE) {
        sResult += "E";
    }

    return sResult;
}

bool XPE::isImportPresent()
{
    return isOptionalHeader_DataDirectoryPresent(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_IMPORT);
}

bool XPE::isIATPresent()
{
    return isOptionalHeader_DataDirectoryPresent(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_IAT);
}

XBinary::_MEMORY_MAP XPE::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    XBinary::_MEMORY_MAP result = {};

    if (mapMode == MAPMODE_UNKNOWN) {
        mapMode = MAPMODE_SECTIONS;  // Default mode
    }

    if (mapMode == MAPMODE_SECTIONS) {
        result = _getMemoryMap(FILEPART_HEADER | FILEPART_SECTION | FILEPART_OVERLAY, pPdStruct);
    }

    return result;
}

XADDR XPE::getBaseAddress()
{
    return (XADDR)getOptionalHeader_ImageBase();
}

void XPE::setBaseAddress(XADDR nBaseAddress)
{
    setOptionalHeader_ImageBase(nBaseAddress);
}

void XPE::setEntryPointOffset(qint64 nEntryPointOffset)
{
    setOptionalHeader_AddressOfEntryPoint(offsetToAddress(nEntryPointOffset) - getModuleAddress());
}

QList<XPE::IMPORT_RECORD> XPE::getImportRecords(PDSTRUCT *pPdStruct)
{
    XBinary::PDSTRUCT pdStructEmpty = {};

    if (!pPdStruct) {
        pdStructEmpty = XBinary::createPdStruct();
        pPdStruct = &pdStructEmpty;
    }

    _MEMORY_MAP memoryMap = getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);

    return getImportRecords(&memoryMap, pPdStruct);
}

QList<XPE::IMPORT_RECORD> XPE::getImportRecords(_MEMORY_MAP *pMemoryMap, PDSTRUCT *pPdStruct)
{
    XBinary::PDSTRUCT pdStructEmpty = {};

    if (!pPdStruct) {
        pdStructEmpty = XBinary::createPdStruct();
        pPdStruct = &pdStructEmpty;
    }

    QList<IMPORT_RECORD> listResult;

    qint64 nImportOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_IMPORT);

    if (nImportOffset != -1) {
        bool bIs64 = is64(pMemoryMap);

        while (!(pPdStruct->bIsStop)) {
            XPE_DEF::IMAGE_IMPORT_DESCRIPTOR iid = read_IMAGE_IMPORT_DESCRIPTOR(nImportOffset);

            QString sLibrary;

            if ((iid.Characteristics == 0) && (iid.Name == 0)) {
                break;
            }

            qint64 nOffset = addressToOffset(pMemoryMap, iid.Name + pMemoryMap->nModuleAddress);

            if (nOffset != -1) {
                sLibrary = read_ansiString(nOffset);

                if (sLibrary == "") {
                    break;
                }
            } else {
                break;  // corrupted
            }

            qint64 nThunksOffset = -1;
            qint64 nRVA = 0;

            if (iid.OriginalFirstThunk) {
                nThunksOffset = addressToOffset(pMemoryMap, iid.OriginalFirstThunk + pMemoryMap->nModuleAddress);
                //                nRVA=iid.OriginalFirstThunk;
            } else if ((iid.FirstThunk)) {
                nThunksOffset = addressToOffset(pMemoryMap, iid.FirstThunk + pMemoryMap->nModuleAddress);
                //                nRVA=iid.FirstThunk;
            }

            nRVA = iid.FirstThunk;

            if (nThunksOffset == -1) {
                break;
            }

            while (true) {
                QString sFunction;

                if (bIs64) {
                    qint64 nThunk64 = read_uint64(nThunksOffset);

                    if (nThunk64 == 0) {
                        break;
                    }

                    if (!(nThunk64 & 0x8000000000000000)) {
                        qint64 nOffset = addressToOffset(pMemoryMap, nThunk64 + pMemoryMap->nModuleAddress);

                        if (nOffset != -1) {
                            sFunction = read_ansiString(nOffset + 2);

                            if (sFunction == "") {
                                break;
                            }
                        } else {
                            break;
                        }
                    } else {
                        sFunction = QString("%1").arg(nThunk64 & 0x7FFFFFFFFFFFFFFF);
                    }
                } else {
                    qint64 nThunk32 = read_uint32(nThunksOffset);

                    if (nThunk32 == 0) {
                        break;
                    }

                    if (!(nThunk32 & 0x80000000)) {
                        qint64 nOffset = addressToOffset(pMemoryMap, nThunk32 + pMemoryMap->nModuleAddress);

                        if (nOffset != -1) {
                            sFunction = read_ansiString(nOffset + 2);

                            if (sFunction == "") {
                                break;
                            }
                        } else {
                            break;
                        }
                    } else {
                        sFunction = QString("%1").arg(nThunk32 & 0x7FFFFFFF);
                    }
                }

                IMPORT_RECORD record;

                record.nOffset = nThunksOffset;
                record.nRVA = nRVA;
                record.sLibrary = sLibrary;
                record.sFunction = sFunction;

                listResult.append(record);

                if (bIs64) {
                    nThunksOffset += 8;
                    nRVA += 8;  // quint64
                } else {
                    nThunksOffset += 4;
                    nRVA += 4;  // quint32
                }
            }

            nImportOffset += sizeof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR);
        }
    }

    return listResult;
}

quint64 XPE::getImportHash64(QList<IMPORT_RECORD> *pListImportRecords, PDSTRUCT *pPdStruct)
{
    quint64 nResult = 0;

    qint32 nNumberOfImports = pListImportRecords->count();

    for (qint32 i = 0; (i < nNumberOfImports) && isPdStructNotCanceled(pPdStruct); i++) {
        QString sRecord = pListImportRecords->at(i).sLibrary + " " + pListImportRecords->at(i).sFunction;

        nResult += getStringCustomCRC32(sRecord);
    }

    return nResult;
}

quint32 XPE::getImportHash32(QList<IMPORT_RECORD> *pListImportRecords, PDSTRUCT *pPdStruct)
{
    quint64 nResult = 0;

    qint32 nNumberOfImports = pListImportRecords->count();

    QString sRecord;

    for (qint32 i = 0; (i < nNumberOfImports) && isPdStructNotCanceled(pPdStruct); i++) {
        sRecord += pListImportRecords->at(i).sLibrary + pListImportRecords->at(i).sFunction;
    }

    nResult = getStringCustomCRC32(sRecord);

    return nResult;
}

qint64 XPE::getImportDescriptorOffset(quint32 nNumber)
{
    qint64 nResult = -1;

    qint64 nImportOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_IMPORT);

    if (nImportOffset != -1) {
        nResult = nImportOffset + nNumber * sizeof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR);
    }

    return nResult;
}

qint64 XPE::getImportDescriptorSize()
{
    return sizeof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR);
}

QList<XPE_DEF::IMAGE_IMPORT_DESCRIPTOR> XPE::getImportDescriptors()
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return getImportDescriptors(&memoryMap);
}

QList<XPE_DEF::IMAGE_IMPORT_DESCRIPTOR> XPE::getImportDescriptors(XBinary::_MEMORY_MAP *pMemoryMap)
{
    QList<XPE_DEF::IMAGE_IMPORT_DESCRIPTOR> listResult;

    qint64 nImportOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_IMPORT);

    if (nImportOffset != -1) {
        while (true) {
            XPE_DEF::IMAGE_IMPORT_DESCRIPTOR iid = read_IMAGE_IMPORT_DESCRIPTOR(nImportOffset);

            if ((iid.Characteristics == 0) && (iid.Name == 0)) {
                break;
            }

            qint64 nOffset = addressToOffset(pMemoryMap, iid.Name + pMemoryMap->nModuleAddress);

            if (nOffset != -1) {
                QString sName = read_ansiString(nOffset);

                if (sName == "") {
                    break;
                }
            } else {
                break;  // corrupted
            }

            listResult.append(iid);

            nImportOffset += sizeof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR);
        }
    }

    return listResult;
}

QList<XPE::IMAGE_IMPORT_DESCRIPTOR_EX> XPE::getImportDescriptorsEx()
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return getImportDescriptorsEx(&memoryMap);
}

QList<XPE::IMAGE_IMPORT_DESCRIPTOR_EX> XPE::getImportDescriptorsEx(XBinary::_MEMORY_MAP *pMemoryMap)
{
    QList<IMAGE_IMPORT_DESCRIPTOR_EX> listResult;

    qint64 nImportOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_IMPORT);

    if (nImportOffset != -1) {
        while (true) {
            IMAGE_IMPORT_DESCRIPTOR_EX record = {};
            XPE_DEF::IMAGE_IMPORT_DESCRIPTOR iid = read_IMAGE_IMPORT_DESCRIPTOR(nImportOffset);

            if ((iid.Characteristics == 0) && (iid.Name == 0)) {
                break;
            }

            qint64 nOffset = addressToOffset(pMemoryMap, iid.Name + pMemoryMap->nModuleAddress);

            if (nOffset != -1) {
                record.sLibrary = read_ansiString(nOffset);

                if (record.sLibrary == "") {
                    break;
                }
            } else {
                break;  // corrupted
            }

            record.Characteristics = iid.Characteristics;
            record.FirstThunk = iid.FirstThunk;
            record.ForwarderChain = iid.ForwarderChain;
            record.Name = iid.Name;
            record.OriginalFirstThunk = iid.OriginalFirstThunk;
            record.TimeDateStamp = iid.TimeDateStamp;

            listResult.append(record);

            nImportOffset += sizeof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR);
        }
    }

    return listResult;
}

XPE_DEF::IMAGE_IMPORT_DESCRIPTOR XPE::getImportDescriptor(quint32 nNumber)
{
    XPE_DEF::IMAGE_IMPORT_DESCRIPTOR result = {};

    qint64 nImportOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_IMPORT);

    if (nImportOffset != -1) {
        nImportOffset += nNumber * sizeof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR);

        result = read_IMAGE_IMPORT_DESCRIPTOR(nImportOffset);
    }

    return result;
}

void XPE::setImportDescriptor(quint32 nNumber, XPE_DEF::IMAGE_IMPORT_DESCRIPTOR *pImportDescriptor)
{
    qint64 nImportOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_IMPORT);

    if (nImportOffset != -1) {
        nImportOffset += nNumber * sizeof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR);

        write_IMAGE_IMPORT_DESCRIPTOR(nImportOffset, *pImportDescriptor);
    }
}

void XPE::setImportDescriptor_OriginalFirstThunk(quint32 nNumber, quint32 nValue)
{
    qint64 nOffset = getImportDescriptorOffset(nNumber);

    write_uint32(nOffset + offsetof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR, OriginalFirstThunk), nValue);
}

void XPE::setImportDescriptor_TimeDateStamp(quint32 nNumber, quint32 nValue)
{
    qint64 nOffset = getImportDescriptorOffset(nNumber);

    write_uint32(nOffset + offsetof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR, TimeDateStamp), nValue);
}

void XPE::setImportDescriptor_ForwarderChain(quint32 nNumber, quint32 nValue)
{
    qint64 nOffset = getImportDescriptorOffset(nNumber);

    write_uint32(nOffset + offsetof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR, ForwarderChain), nValue);
}

void XPE::setImportDescriptor_Name(quint32 nNumber, quint32 nValue)
{
    qint64 nOffset = getImportDescriptorOffset(nNumber);

    write_uint32(nOffset + offsetof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR, Name), nValue);
}

void XPE::setImportDescriptor_FirstThunk(quint32 nNumber, quint32 nValue)
{
    qint64 nOffset = getImportDescriptorOffset(nNumber);

    write_uint32(nOffset + offsetof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR, FirstThunk), nValue);
}

QList<XPE::IMPORT_HEADER> XPE::getImports(PDSTRUCT *pPdStruct)
{
    XBinary::PDSTRUCT pdStructEmpty = {};

    if (!pPdStruct) {
        pdStructEmpty = XBinary::createPdStruct();
        pPdStruct = &pdStructEmpty;
    }

    _MEMORY_MAP memoryMap = getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);

    return getImports(&memoryMap, pPdStruct);
}

QList<XPE::IMPORT_HEADER> XPE::getImports(XBinary::_MEMORY_MAP *pMemoryMap, PDSTRUCT *pPdStruct)
{
    XBinary::PDSTRUCT pdStructEmpty = {};

    if (!pPdStruct) {
        pdStructEmpty = XBinary::createPdStruct();
        pPdStruct = &pdStructEmpty;
    }

    QList<IMPORT_HEADER> listResult;

    XPE_DEF::IMAGE_DATA_DIRECTORY dataResources = getOptionalHeader_DataDirectory(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_IMPORT);

    XADDR nModuleAddress = getModuleAddress();
    qint64 nImportOffset = -1;
    qint64 nImportOffsetTest = -1;
    const qint64 nFileSize = getSize();
    const qint32 nMaxImportDescriptors = 4096;
    const qint32 nMaxPositionsPerLibrary = 16384;
    const qint32 nMaxImportPositions = 65536;
    qint32 nDescriptorCount = 0;
    qint32 nTotalPositions = 0;

    if (dataResources.VirtualAddress) {
        nImportOffset = addressToOffset(pMemoryMap, dataResources.VirtualAddress + nModuleAddress);
        nImportOffsetTest =
            addressToOffset(pMemoryMap, dataResources.VirtualAddress + nModuleAddress + sizeof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR) - 2);  // Test for some (Win)Upack stubs
    }

    if (nImportOffset != -1) {
        while (!(pPdStruct->bIsStop)
               && nDescriptorCount < nMaxImportDescriptors) {
            const qint64 nDescriptorRVA =
                static_cast<qint64>(dataResources.VirtualAddress)
                + static_cast<qint64>(nDescriptorCount)
                      * sizeof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR);
            const qint64 nMappedStart =
                XBinary::relAddressToOffset(pMemoryMap, nDescriptorRVA);
            const qint64 nMappedEnd = XBinary::relAddressToOffset(
                pMemoryMap,
                nDescriptorRVA
                    + sizeof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR) - 1);
            const bool bLegacyPartialFirstDescriptor =
                nDescriptorCount == 0 && nImportOffsetTest == -1;
            if (nImportOffset < 0
                || nImportOffset > nFileSize
                                       - static_cast<qint64>(
                                           sizeof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR))
                || nMappedStart != nImportOffset
                || (!bLegacyPartialFirstDescriptor && nMappedEnd
                       != nImportOffset
                              + sizeof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR) - 1)) {
                break;
            }
            XPE_DEF::IMAGE_IMPORT_DESCRIPTOR iid = read_IMAGE_IMPORT_DESCRIPTOR(nImportOffset);

            IMPORT_HEADER importHeader = {};

            if (nImportOffsetTest == -1) {
                iid.FirstThunk &= 0x0000FFFF;
            }

            if ((iid.Characteristics == 0) && (iid.Name == 0)) {
                break;
            }

            qint64 nOffset = addressToOffset(pMemoryMap, iid.Name + nModuleAddress);

            if (nOffset != -1) {
                importHeader.sName = read_ansiString(nOffset, qMin((qint64)2048, getSize() - nOffset));

                if (importHeader.sName == "") {
                    break;
                }
            } else {
                break;  // corrupted
            }

            qint64 nThunksOffset = -1;
            qint64 nThunksRVA = 0;
            qint64 nThunksOriginalRVA = 0;
            //          qint64 nThunksOriginalOffset=0;

            if (iid.OriginalFirstThunk) {
                nThunksRVA = iid.OriginalFirstThunk;
                //                nRVA=iid.OriginalFirstThunk;
            } else if ((iid.FirstThunk)) {
                nThunksRVA = iid.FirstThunk;
                //                nRVA=iid.FirstThunk;
            }

            nThunksOriginalRVA = iid.FirstThunk;

            nThunksOffset = addressToOffset(pMemoryMap, nThunksRVA + nModuleAddress);
            //            nThunksOriginalOffset=addressToOffset(pMemoryMap,nThunksOriginalRVA+nBaseAddress);

            if (nThunksOffset != -1) {
                const qint32 nRemainingPositions =
                    qMin(nMaxPositionsPerLibrary,
                         nMaxImportPositions - nTotalPositions);
                importHeader.listPositions =
                    _getImportPositions(pMemoryMap, nThunksRVA,
                                        nThunksOriginalRVA, pPdStruct,
                                        nRemainingPositions);
                nTotalPositions += importHeader.listPositions.size();
            }

            listResult.append(importHeader);

            nImportOffset += sizeof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR);
            ++nDescriptorCount;
            if (nTotalPositions >= nMaxImportPositions) {
                break;
            }
        }
    }

    return listResult;
}

XPE_DEF::IMAGE_DATA_DIRECTORY XPE::getIAT(_MEMORY_MAP *pMemoryMap, PDSTRUCT *pPdStruct)
{
    XPE_DEF::IMAGE_DATA_DIRECTORY result = {};

    QList<XPE::IMPORT_RECORD> listImportRecords = getImportRecords(pMemoryMap, pPdStruct);

    qint32 nNumberOfRecords = listImportRecords.count();

    if (nNumberOfRecords) {
        qint64 nMin = pMemoryMap->nImageSize;
        qint64 nMax = 0;

        for (qint32 i = 0; (i < nNumberOfRecords) && isPdStructNotCanceled(pPdStruct); i++) {
            nMin = qMin(listImportRecords.at(i).nRVA, nMin);
            nMax = qMax(listImportRecords.at(i).nRVA, nMax);
        }

        result.VirtualAddress = nMin;
        result.Size = nMax - nMin;

        if (pMemoryMap->mode == MODE_32) {
            result.Size += 4;
        } else if (pMemoryMap->mode == MODE_64) {
            result.Size += 8;
        }
    }

    return result;
}

QList<QString> XPE::getImportNames(_MEMORY_MAP *pMemoryMap, PDSTRUCT *pPdStruct)
{
    XBinary::PDSTRUCT pdStructEmpty = {};

    if (!pPdStruct) {
        pdStructEmpty = XBinary::createPdStruct();
        pPdStruct = &pdStructEmpty;
    }

    QList<QString> listResult;

    XPE_DEF::IMAGE_DATA_DIRECTORY dataResources = getOptionalHeader_DataDirectory(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_IMPORT);

    XADDR nModuleAddress = getModuleAddress();
    qint64 nImportOffset = -1;
    qint64 nImportOffsetTest = -1;
    const qint64 nFileSize = getSize();
    const qint32 nMaxImportDescriptors = 4096;
    qint32 nDescriptorCount = 0;

    if (dataResources.VirtualAddress) {
        nImportOffset = addressToOffset(pMemoryMap, dataResources.VirtualAddress + nModuleAddress);
        nImportOffsetTest =
            addressToOffset(pMemoryMap, dataResources.VirtualAddress + nModuleAddress + sizeof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR) - 2);  // Test for some (Win)Upack stubs
    }

    if (nImportOffset != -1) {
        while (!(pPdStruct->bIsStop)
               && nDescriptorCount < nMaxImportDescriptors) {
            const qint64 nDescriptorRVA =
                static_cast<qint64>(dataResources.VirtualAddress)
                + static_cast<qint64>(nDescriptorCount)
                      * sizeof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR);
            const qint64 nMappedStart =
                XBinary::relAddressToOffset(pMemoryMap, nDescriptorRVA);
            const qint64 nMappedEnd = XBinary::relAddressToOffset(
                pMemoryMap,
                nDescriptorRVA
                    + sizeof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR) - 1);
            const bool bLegacyPartialFirstDescriptor =
                nDescriptorCount == 0 && nImportOffsetTest == -1;
            if (nImportOffset < 0
                || nImportOffset > nFileSize
                                       - static_cast<qint64>(
                                           sizeof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR))
                || nMappedStart != nImportOffset
                || (!bLegacyPartialFirstDescriptor && nMappedEnd
                       != nImportOffset
                              + sizeof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR) - 1)) {
                break;
            }
            XPE_DEF::IMAGE_IMPORT_DESCRIPTOR iid = read_IMAGE_IMPORT_DESCRIPTOR(nImportOffset);

            QString sLibrary;

            if (nImportOffsetTest == -1) {
                iid.FirstThunk &= 0x0000FFFF;
            }

            if ((iid.Characteristics == 0) && (iid.Name == 0)) {
                break;
            }

            qint64 nOffset = addressToOffset(pMemoryMap, iid.Name + nModuleAddress);

            if (nOffset != -1) {
                sLibrary = read_ansiString(nOffset);

                if (sLibrary == "") {
                    break;
                }
            } else {
                break;  // corrupted
            }

            listResult.append(sLibrary);

            nImportOffset += sizeof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR);
            ++nDescriptorCount;
        }
    }

    return listResult;
}

QList<XPE::IMPORT_POSITION> XPE::_getImportPositions(XBinary::_MEMORY_MAP *pMemoryMap, qint64 nThunksRVA, qint64 nRVA,
                                                    PDSTRUCT *pPdStruct, qint32 nMaxPositions)
{
    XBinary::PDSTRUCT pdStructEmpty = {};

    if (!pPdStruct) {
        pdStructEmpty = XBinary::createPdStruct();
        pPdStruct = &pdStructEmpty;
    }

    QList<IMPORT_POSITION> listResult;

    qint64 nThunksOffset = XBinary::relAddressToOffset(pMemoryMap, nThunksRVA);

    bool bIs64 = is64(pMemoryMap);
    const qint64 nFileSize = getSize();
    const qint64 nThunkSize = bIs64 ? 8 : 4;
    nMaxPositions = qBound<qint32>(0, nMaxPositions, 16384);

    while (!(pPdStruct->bIsStop)
           && listResult.size() < nMaxPositions) {
        const qint64 nMappedStart =
            XBinary::relAddressToOffset(pMemoryMap, nThunksRVA);
        const qint64 nMappedEnd =
            nThunksRVA <= (std::numeric_limits<qint64>::max)()
                                  - (nThunkSize - 1)
                ? XBinary::relAddressToOffset(
                      pMemoryMap, nThunksRVA + nThunkSize - 1)
                : -1;
        if (nThunksOffset < 0
            || nThunksOffset > nFileSize - nThunkSize
            || nMappedStart != nThunksOffset
            || nMappedEnd != nThunksOffset + nThunkSize - 1) {
            break;
        }
        IMPORT_POSITION importPosition = {};
        importPosition.nThunkOffset = nThunksOffset;
        // The thunk value (name pointer / ordinal) is read from nThunksRVA -- the
        // OriginalFirstThunk (INT) when present -- but nThunkRVA must name the IAT slot
        // that the loader overwrites with the resolved address, i.e. FirstThunk (passed in
        // as nRVA and advanced in lockstep below). When OFT != FirstThunk (e.g. FSG-packed
        // binaries) these differ, and using nThunksRVA here patches the read-only INT.
        importPosition.nThunkRVA = nRVA;

        if (bIs64) {
            importPosition.nThunkValue = read_uint64(nThunksOffset);

            if (importPosition.nThunkValue == 0) {
                break;
            }

            if (!(importPosition.nThunkValue & 0x8000000000000000)) {
                qint64 nOffset = addressToOffset(pMemoryMap, importPosition.nThunkValue + pMemoryMap->nModuleAddress);

                if ((nOffset != -1) && checkOffsetSize(nOffset, 3)) {
                    importPosition.nHint = read_uint16(nOffset);
                    importPosition.sName = read_ansiString(nOffset + 2, qMin((qint64)2048, getSize() - nOffset - 2));

                    if (importPosition.sName == "") {
                        break;
                    }
                } else {
                    break;
                }
            } else {
                importPosition.nOrdinal = importPosition.nThunkValue & 0x7FFFFFFFFFFFFFFF;
            }
        } else {
            importPosition.nThunkValue = read_uint32(nThunksOffset);

            if (importPosition.nThunkValue == 0) {
                break;
            }

            if (!(importPosition.nThunkValue & 0x80000000)) {
                qint64 nOffset = addressToOffset(pMemoryMap, importPosition.nThunkValue + pMemoryMap->nModuleAddress);

                if ((nOffset != -1) && checkOffsetSize(nOffset, 3)) {
                    importPosition.nHint = read_uint16(nOffset);
                    importPosition.sName = read_ansiString(nOffset + 2, qMin((qint64)2048, getSize() - nOffset - 2));

                    if (importPosition.sName == "") {
                        break;
                    }
                } else {
                    break;
                }
            } else {
                importPosition.nOrdinal = importPosition.nThunkValue & 0x7FFFFFFF;
            }
        }

        if (importPosition.nOrdinal == 0) {
            importPosition.sFunction = importPosition.sName;
        } else {
            importPosition.sFunction = QString("%1").arg(importPosition.nOrdinal);
        }

        listResult.append(importPosition);
        if (nThunksRVA
                > (std::numeric_limits<qint64>::max)() - nThunkSize
            || nThunksOffset
                   > (std::numeric_limits<qint64>::max)() - nThunkSize
            || nRVA
                   > (std::numeric_limits<qint64>::max)() - nThunkSize) {
            break;
        }
        if (bIs64) {
            nThunksRVA += 8;
            nThunksOffset += 8;
            nRVA += 8;
        } else {
            nThunksRVA += 4;
            nThunksOffset += 4;
            nRVA += 4;
        }

    }

    return listResult;
}

QList<XPE::IMPORT_POSITION> XPE::getImportPositions(qint32 nIndex, PDSTRUCT *pPdStruct)
{
    XBinary::PDSTRUCT pdStructEmpty = {};

    if (!pPdStruct) {
        pdStructEmpty = XBinary::createPdStruct();
        pPdStruct = &pdStructEmpty;
    }

    QList<IMPORT_POSITION> listResult;

    qint64 nImportOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_IMPORT);

    const qint32 nMaxImportDescriptors = 4096;
    if (nImportOffset != -1 && nIndex >= 0
        && nIndex < nMaxImportDescriptors) {
        _MEMORY_MAP memoryMap = getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);
        const qint64 nFileSize = getSize();
        const XPE_DEF::IMAGE_DATA_DIRECTORY dataResources =
            getOptionalHeader_DataDirectory(
                XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_IMPORT);

        qint32 _nIndex = 0;

        while (!(pPdStruct->bIsStop)
               && _nIndex < nMaxImportDescriptors) {
            const qint64 nDescriptorRVA =
                static_cast<qint64>(dataResources.VirtualAddress)
                + static_cast<qint64>(_nIndex)
                      * sizeof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR);
            const qint64 nMappedStart =
                XBinary::relAddressToOffset(&memoryMap, nDescriptorRVA);
            const qint64 nMappedEnd = XBinary::relAddressToOffset(
                &memoryMap,
                nDescriptorRVA
                    + sizeof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR) - 1);
            if (nImportOffset < 0
                || nImportOffset > nFileSize
                                       - static_cast<qint64>(
                                           sizeof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR))
                || nMappedStart != nImportOffset
                || nMappedEnd
                       != nImportOffset
                              + sizeof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR) - 1) {
                break;
            }
            IMPORT_HEADER importHeader = {};
            XPE_DEF::IMAGE_IMPORT_DESCRIPTOR iid = read_IMAGE_IMPORT_DESCRIPTOR(nImportOffset);

            if ((iid.Characteristics == 0) && (iid.Name == 0)) {
                break;
            }

            qint64 nOffset = addressToOffset(&memoryMap, iid.Name + memoryMap.nModuleAddress);

            if (nOffset != -1) {
                importHeader.sName = read_ansiString(nOffset);

                if (importHeader.sName == "") {
                    break;
                }
            } else {
                break;  // corrupted
            }

            qint64 nThunksOffset = -1;
            qint64 nRVA = 0;
            qint64 nThunksRVA = -1;

            if (iid.OriginalFirstThunk) {
                nThunksRVA = iid.OriginalFirstThunk;
            } else if ((iid.FirstThunk)) {
                nThunksRVA = iid.FirstThunk;
            }

            nRVA = iid.FirstThunk;
            nThunksOffset = relAddressToOffset(&memoryMap, nThunksRVA);

            if (nThunksOffset == -1) {
                break;
            }

            if (_nIndex == nIndex) {
                listResult = _getImportPositions(&memoryMap, nThunksRVA, nRVA);

                break;
            }

            nImportOffset += sizeof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR);
            _nIndex++;
        }
    }

    return listResult;
}

QList<quint32> XPE::getImportPositionHashes(bool bLibraryName)
{
    QList<IMPORT_HEADER> listImport = getImports();

    return getImportPositionHashes(&listImport, bLibraryName);
}

QList<quint32> XPE::getImportPositionHashes(QList<IMPORT_HEADER> *pListImport, bool bLibraryName)
{
    QList<quint32> listResult;

    qint32 nNumberOfImports = pListImport->count();

    for (qint32 i = 0; i < nNumberOfImports; i++) {
        IMPORT_HEADER record = pListImport->at(i);

        qint32 nNumberOfPositions = record.listPositions.count();

        QString sString;

        for (qint32 j = 0; j < nNumberOfPositions; j++) {
            if (bLibraryName) {
                sString += record.sName;
            }

            sString += record.listPositions.at(j).sFunction;
        }

        listResult.append(getStringCustomCRC32(sString));
    }

    return listResult;
}

bool XPE::isImportPositionHashPresent(QList<quint32> *pListImportHashes, qint32 nIndex, quint32 nHash, PDSTRUCT *pPdStruct)
{
    XBinary::PDSTRUCT pdStructEmpty = {};

    if (!pPdStruct) {
        pdStructEmpty = XBinary::createPdStruct();
        pPdStruct = &pdStructEmpty;
    }

    bool bResult = false;

    if (nIndex == -1) {
        qint32 nNumberOfImports = pListImportHashes->count();

        for (qint32 i = 0; (i < nNumberOfImports) && (!(pPdStruct->bIsStop)); i++) {
            if (pListImportHashes->at(i) == nHash) {
                bResult = true;

                break;
            }
        }
    } else if (nIndex >= 0) {
        if (nIndex < pListImportHashes->count()) {
            bResult = (pListImportHashes->at(nIndex) == nHash);
        }
    }

    return bResult;
}

bool XPE::isImportLibraryPresent(const QString &sLibrary, PDSTRUCT *pPdStruct)
{
    _MEMORY_MAP memoryMap = _getSimpleMemoryMap(FILEPART_HEADER | FILEPART_SECTION | FILEPART_OVERLAY, pPdStruct);

    QList<QString> listLibraries = getImportNames(&memoryMap, pPdStruct);

    return listLibraries.contains(sLibrary);
}

bool XPE::isImportLibraryPresent(const QString &sLibrary, QList<IMPORT_HEADER> *pListImportHeaders, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    bool bResult = false;

    qint32 nNumberOfImports = pListImportHeaders->count();

    for (qint32 i = 0; (i < nNumberOfImports) && (!(pPdStruct->bIsStop)); i++) {
        if (pListImportHeaders->at(i).sName == sLibrary) {
            bResult = true;
            break;
        }
    }

    return bResult;
}

bool XPE::isImportLibraryPresentI(const QString &sLibrary, PDSTRUCT *pPdStruct)
{
    QList<IMPORT_HEADER> listImportHeaders = getImports(pPdStruct);

    return isImportLibraryPresentI(sLibrary, &listImportHeaders, pPdStruct);
}

bool XPE::isImportLibraryPresentI(const QString &sLibrary, QList<XPE::IMPORT_HEADER> *pListImportHeaders, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    bool bResult = false;

    qint32 nNumberOfImports = pListImportHeaders->count();

    for (qint32 i = 0; i < nNumberOfImports; i++) {
        if (pListImportHeaders->at(i).sName.toUpper() == sLibrary.toUpper()) {
            bResult = true;
            break;
        }
    }

    if (!bResult) {
        QString sShortName = sLibrary.section(".", 0, 0).toUpper();

        for (qint32 i = 0; i < nNumberOfImports; i++) {
            if (pListImportHeaders->at(i).sName.toUpper().section(".", 0, 0) == sShortName) {
                bResult = true;
                break;
            }
        }
    }

    return bResult;
}

bool XPE::isImportFunctionPresentI(const QString &sLibrary, const QString &sFunction, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    QList<IMPORT_HEADER> listImportHeaders = getImports(pPdStruct);

    return isImportFunctionPresentI(sLibrary, sFunction, &listImportHeaders, pPdStruct);
}

bool XPE::isImportFunctionPresentI(const QString &sLibrary, const QString &sFunction, QList<XPE::IMPORT_HEADER> *pListImportHeaders, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    bool bResult = false;

    // TODO Optimize!

    qint32 nNumberOfImports = pListImportHeaders->count();

    for (qint32 i = 0; (i < nNumberOfImports) && (!(pPdStruct->bIsStop)); i++) {
        if (pListImportHeaders->at(i).sName.toUpper() == sLibrary.toUpper()) {
            qint32 nNumberOfPositions = pListImportHeaders->at(i).listPositions.count();

            for (qint32 j = 0; (j < nNumberOfPositions) && (!(pPdStruct->bIsStop)); j++) {
                if (pListImportHeaders->at(i).listPositions.at(j).sFunction == sFunction) {
                    bResult = true;
                    break;
                }
            }
        }
    }

    return bResult;
}

bool XPE::isFunctionPresent(const QString &sFunction, QList<IMPORT_HEADER> *pListImportHeaders, PDSTRUCT *pPdStruct)
{
    bool bResult = false;

    // TODO Optimize!

    qint32 nNumberOfImports = pListImportHeaders->count();

    for (qint32 i = 0; (i < nNumberOfImports) && isPdStructNotCanceled(pPdStruct); i++) {
        qint32 nNumberOfPositions = pListImportHeaders->at(i).listPositions.count();

        for (qint32 j = 0; (j < nNumberOfPositions) && isPdStructNotCanceled(pPdStruct); j++) {
            if (pListImportHeaders->at(i).listPositions.at(j).sFunction == sFunction) {
                bResult = true;
                break;
            }
        }
    }

    return bResult;
}

bool XPE::setImports(QList<XPE::IMPORT_HEADER> *pListImportHeaders, PDSTRUCT *pPdStruct)
{
    return setImports(getDevice(), isImage(), pListImportHeaders, pPdStruct);
}

bool XPE::setImports(QIODevice *pDevice, bool bIsImage, QList<XPE::IMPORT_HEADER> *pListImportHeaders, PDSTRUCT *pPdStruct)
{
    bool bResult = false;

    if (isResizeEnable(pDevice)) {
        XPE pe(pDevice, bIsImage);

        if (pe.isValid()) {
            qint32 nAddressSize = pe.is64() ? 8 : 4;

            QByteArray baImport;
            QList<XADDR> listPatches;

            quint32 nIATSize = 0;
            quint32 nImportTableSize = (pListImportHeaders->count() + 1) * sizeof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR);
            quint32 nAnsiDataSize = 0;

            qint32 nNumberOfHeaders = pListImportHeaders->count();

            for (qint32 i = 0; i < nNumberOfHeaders; i++) {
                qint32 nNumberOfPositions = pListImportHeaders->at(i).listPositions.count();

                nIATSize += (nNumberOfPositions + 1) * nAddressSize;

                QByteArray baName = pListImportHeaders->at(i).sName.toLatin1();
                nAnsiDataSize += baName.size() + 1;

                for (qint32 j = 0; j < nNumberOfPositions; j++) {
                    const QString &funcName = pListImportHeaders->at(i).listPositions.at(j).sName;
                    if (!funcName.isEmpty()) {
                        QByteArray baFunc = funcName.toLatin1();
                        nAnsiDataSize += 2 + baFunc.size() + 1;
                    }
                }
            }

            nImportTableSize = S_ALIGN_UP(nImportTableSize, 16);
            nIATSize = S_ALIGN_UP(nIATSize, 16);
            nAnsiDataSize = S_ALIGN_UP(nAnsiDataSize, 16);

            baImport.resize(nIATSize + nImportTableSize + nIATSize + nAnsiDataSize);
            baImport.fill(0);

            char *pDataOffset = baImport.data();
            char *pIAT = pDataOffset;
            XPE_DEF::IMAGE_IMPORT_DESCRIPTOR *pIID = (XPE_DEF::IMAGE_IMPORT_DESCRIPTOR *)(pDataOffset + nIATSize);
            char *pOIAT = pDataOffset + nIATSize + nImportTableSize;

            char *pAnsiBase = pDataOffset + nIATSize + nImportTableSize + nIATSize;
            char *pAnsiData = pAnsiBase;

            quint32 nAnsiOffset = 0;

            for (qint32 i = 0; i < nNumberOfHeaders; i++) {
                pIID->FirstThunk = pIAT - pDataOffset;
                listPatches.append((char *)pIID - pDataOffset + offsetof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR, FirstThunk));

                pIID->Name = pAnsiData - pDataOffset;
                listPatches.append((char *)pIID - pDataOffset + offsetof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR, Name));

                pIID->OriginalFirstThunk = pOIAT - pDataOffset;
                listPatches.append((char *)pIID - pDataOffset + offsetof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR, OriginalFirstThunk));

                QByteArray baName = pListImportHeaders->at(i).sName.toLatin1();
                size_t remaining = nAnsiDataSize - nAnsiOffset;

                strCopy(pAnsiData, baName.constData(), remaining);

                quint32 nWritten = (quint32)(baName.size() + 1);
                pAnsiData += nWritten;
                nAnsiOffset += nWritten;

                pIID++;

                qint32 nNumberOfPositions = pListImportHeaders->at(i).listPositions.count();

                for (qint32 j = 0; j < nNumberOfPositions; j++) {
                    const IMPORT_POSITION &pos = pListImportHeaders->at(i).listPositions.at(j);

                    if (!pos.sName.isEmpty()) {
                        *((quint32 *)pOIAT) = pAnsiData - pDataOffset;
                        *((quint32 *)pIAT) = *((quint32 *)pOIAT);

                        listPatches.append(pOIAT - pDataOffset);
                        listPatches.append(pIAT - pDataOffset);

                        remaining = nAnsiDataSize - nAnsiOffset;
                        if (remaining < 2) break;

                        *((quint16 *)pAnsiData) = pos.nHint;
                        pAnsiData += 2;
                        nAnsiOffset += 2;
                        remaining -= 2;

                        QByteArray baFunc = pos.sName.toLatin1();
                        strCopy(pAnsiData, baFunc.constData(), remaining);

                        nWritten = (quint32)(baFunc.size() + 1);
                        pAnsiData += nWritten;
                        nAnsiOffset += nWritten;

                    } else {
                        if (nAddressSize == 4) {
                            *((quint32 *)pOIAT) = pos.nOrdinal + 0x80000000;
                            *((quint32 *)pIAT) = *((quint32 *)pOIAT);
                        } else {
                            *((quint64 *)pOIAT) = pos.nOrdinal + 0x8000000000000000;
                            *((quint64 *)pIAT) = *((quint64 *)pOIAT);
                        }
                    }

                    pIAT += nAddressSize;
                    pOIAT += nAddressSize;
                }

                pIAT += nAddressSize;
                pOIAT += nAddressSize;
            }

            XPE_DEF::IMAGE_SECTION_HEADER ish = {};
            ish.Characteristics = 0xc0000040;

            if (addSection(pDevice, bIsImage, &ish, baImport.data(), baImport.size(), pPdStruct)) {
                _MEMORY_MAP memoryMap = pe.getMemoryMap();

                XPE_DEF::IMAGE_DATA_DIRECTORY iddIAT = {};
                iddIAT.VirtualAddress = ish.VirtualAddress;
                iddIAT.Size = nIATSize;

                XPE_DEF::IMAGE_DATA_DIRECTORY iddImportTable = {};
                iddImportTable.VirtualAddress = nIATSize + ish.VirtualAddress;
                iddImportTable.Size = nImportTableSize;

                pe.setOptionalHeader_DataDirectory(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_IAT, &iddIAT);
                pe.setOptionalHeader_DataDirectory(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_IMPORT, &iddImportTable);

                for (XADDR patch : listPatches) {
                    qint64 nCurrentOffset = ish.PointerToRawData + patch;
                    quint32 nValue = pe.read_uint32(nCurrentOffset);
                    pe.write_uint32(nCurrentOffset, nValue + ish.VirtualAddress);
                }

                for (qint32 i = 0; i < nNumberOfHeaders; i++) {
                    if (pListImportHeaders->at(i).nFirstThunk) {
                        XPE_DEF::IMAGE_IMPORT_DESCRIPTOR iid = pe.getImportDescriptor(i);

                        qint64 nSrcOffset = pe.addressToOffset(&memoryMap, iid.FirstThunk + memoryMap.nModuleAddress);
                        qint64 nDstOffset = pe.addressToOffset(&memoryMap, pListImportHeaders->at(i).nFirstThunk + memoryMap.nModuleAddress);

                        if ((nSrcOffset != -1) && (nDstOffset != -1)) {
                            while (true) {
                                quint32 nValue = pe.read_uint32(nSrcOffset);
                                pe.write_uint32(nDstOffset, nValue);
                                if (nValue == 0) break;

                                nSrcOffset += nAddressSize;
                                nDstOffset += nAddressSize;
                            }

                            iid.FirstThunk = pListImportHeaders->at(i).nFirstThunk;
                            pe.setImportDescriptor(i, &iid);
                        }
                    }
                }

                bResult = true;
            }
        }
    }

    return bResult;
}

bool XPE::setImports(const QString &sFileName, bool bIsImage, QList<XPE::IMPORT_HEADER> *pListImportHeaders, PDSTRUCT *pPdStruct)
{
    bool bResult = false;

    QFile file(sFileName);

    if (file.open(QIODevice::ReadWrite)) {
        bResult = setImports(&file, bIsImage, pListImportHeaders, pPdStruct);

        file.close();
    }

    return bResult;
}

QString XPE::getImportFunctionName(quint32 nImport, quint32 nFunctionNumber, QList<XPE::IMPORT_HEADER> *pListImportHeaders)
{
    QString sResult;

    if (nImport < (quint32)pListImportHeaders->count()) {
        if (nFunctionNumber < (quint32)pListImportHeaders->at(nImport).listPositions.count()) {
            sResult = pListImportHeaders->at(nImport).listPositions.at(nFunctionNumber).sFunction;
        }
    }

    return sResult;
}

XPE::RESOURCE_HEADER XPE::getResourceHeader()
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return getResourceHeader(&memoryMap);
}

XPE::RESOURCE_HEADER XPE::getResourceHeader(_MEMORY_MAP *pMemoryMap)
{
    RESOURCE_HEADER result = {};

    qint64 nResourceOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_RESOURCE);

    if (nResourceOffset != -1) {
        qint64 nOffset = nResourceOffset;

        result.nOffset = nOffset;
        result.directory = read_IMAGE_RESOURCE_DIRECTORY(nOffset);

        if ((result.directory.NumberOfIdEntries + result.directory.NumberOfNamedEntries <= 1000) && (result.directory.Characteristics == 0))  // check corrupted
        {
            nOffset += sizeof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY);

            for (qint32 i = 0; i < result.directory.NumberOfIdEntries + result.directory.NumberOfNamedEntries; i++) {
                RESOURCE_POSITION rp = _getResourcePosition(pMemoryMap, pMemoryMap->nModuleAddress, nResourceOffset, nOffset, 0);

                if (!rp.bIsValid) {
                    break;
                }

                result.listPositions.append(rp);
                nOffset += sizeof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY_ENTRY);
            }
        }
    }

    return result;
}

QList<XPE::RESOURCE_RECORD> XPE::getResources(qint32 nLimit, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    _MEMORY_MAP memoryMap = getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);

    return getResources(&memoryMap, nLimit, pPdStruct);
}

QList<XPE::RESOURCE_RECORD> XPE::getResources(XBinary::_MEMORY_MAP *pMemoryMap, qint32 nLimit, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }
    qint32 nCount = 0;
    // TODO BE LE
    QList<RESOURCE_RECORD> listResult;

    qint64 nResourceOffset = getDataDirectoryOffset(pMemoryMap, XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_RESOURCE);

    if (nResourceOffset != -1) {
        XADDR nModuleAddress = getModuleAddress();
        RESOURCE_RECORD record = {};

        qint64 nOffsetLevel[3] = {};
        XPE_DEF::IMAGE_RESOURCE_DIRECTORY rd[3] = {};
        XPE_DEF::IMAGE_RESOURCE_DIRECTORY_ENTRY rde[3] = {};

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
        RESOURCES_ID_NAME irin[3] = {};
#else
        RESOURCES_ID_NAME irin[3] = {0};  // MinGW 4.9 bug? // TODO Check
#endif

        nOffsetLevel[0] = nResourceOffset;
        rd[0] = read_IMAGE_RESOURCE_DIRECTORY(nOffsetLevel[0]);

        if ((rd[0].NumberOfIdEntries + rd[0].NumberOfNamedEntries <= 1000) && (rd[0].Characteristics == 0))  // check corrupted  TODO const
        {
            nOffsetLevel[0] += sizeof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY);

            for (qint32 i = 0; i < rd[0].NumberOfIdEntries + rd[0].NumberOfNamedEntries; i++) {
                rde[0] = read_IMAGE_RESOURCE_DIRECTORY_ENTRY(nOffsetLevel[0]);

                if (rde[0].OffsetToDirectory == 0) {
                    break;
                }

                irin[0] = getResourcesIDName(nResourceOffset, rde[0].Name);
                record.irin[0] = irin[0];

                nOffsetLevel[1] = nResourceOffset + rde[0].OffsetToDirectory;

                rd[1] = read_IMAGE_RESOURCE_DIRECTORY(nOffsetLevel[1]);

                if (rd[1].Characteristics != 0) {
                    break;
                }

                nOffsetLevel[1] += sizeof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY);

                if (rd[1].NumberOfIdEntries + rd[1].NumberOfNamedEntries <= 1000) {
                    for (qint32 j = 0; j < rd[1].NumberOfIdEntries + rd[1].NumberOfNamedEntries; j++) {
                        rde[1] = read_IMAGE_RESOURCE_DIRECTORY_ENTRY(nOffsetLevel[1]);

                        irin[1] = getResourcesIDName(nResourceOffset, rde[1].Name);
                        record.irin[1] = irin[1];

                        nOffsetLevel[2] = nResourceOffset + rde[1].OffsetToDirectory;

                        rd[2] = read_IMAGE_RESOURCE_DIRECTORY(nOffsetLevel[2]);

                        if (rd[2].Characteristics != 0) {
                            break;
                        }

                        nOffsetLevel[2] += sizeof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY);

                        if (rd[2].NumberOfIdEntries + rd[2].NumberOfNamedEntries <= 1000) {
                            for (qint32 k = 0; k < rd[2].NumberOfIdEntries + rd[2].NumberOfNamedEntries; k++) {
                                rde[2] = read_IMAGE_RESOURCE_DIRECTORY_ENTRY(nOffsetLevel[2]);

                                irin[2] = getResourcesIDName(nResourceOffset, rde[2].Name);
                                record.irin[2] = irin[2];

                                record.nIRDEOffset = rde[2].OffsetToData;
                                XPE_DEF::IMAGE_RESOURCE_DATA_ENTRY irde = read_IMAGE_RESOURCE_DATA_ENTRY(nResourceOffset + record.nIRDEOffset);
                                record.nRVA = irde.OffsetToData;
                                record.nAddress = irde.OffsetToData + nModuleAddress;
                                record.nOffset = addressToOffset(pMemoryMap, record.nAddress);
                                record.nSize = irde.Size;

                                listResult.append(record);

                                nCount++;

                                if ((nLimit != -1) && (nCount >= nLimit)) {
                                    break;
                                }

                                nOffsetLevel[2] += sizeof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY_ENTRY);
                            }
                        }

                        if ((nLimit != -1) && (nCount >= nLimit)) {
                            break;
                        }

                        nOffsetLevel[1] += sizeof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY_ENTRY);
                    }
                }

                if ((nLimit != -1) && (nCount >= nLimit)) {
                    break;
                }

                nOffsetLevel[0] += sizeof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY_ENTRY);
            }
        }
    }

    return listResult;
}

XPE::RESOURCE_RECORD XPE::getResourceRecord(quint32 nID1, quint32 nID2, QList<XPE::RESOURCE_RECORD> *pListResourceRecords)
{
    RESOURCE_RECORD result = {};

    result.nOffset = -1;

    qint32 nNumberOfResources = pListResourceRecords->count();

    for (qint32 i = 0; i < nNumberOfResources; i++) {
        if (pListResourceRecords->at(i).irin[0].nID == nID1) {
            if ((pListResourceRecords->at(i).irin[1].nID == nID2) || (nID2 == (quint32)-1)) {
                result = pListResourceRecords->at(i);

                break;
            }
        }
    }

    return result;
}

XPE::RESOURCE_RECORD XPE::getResourceRecord(quint32 nID1, const QString &sName2, QList<XPE::RESOURCE_RECORD> *pListResourceRecords)
{
    RESOURCE_RECORD result = {};

    result.nOffset = -1;

    qint32 nNumberOfResources = pListResourceRecords->count();

    for (qint32 i = 0; i < nNumberOfResources; i++) {
        if ((pListResourceRecords->at(i).irin[0].nID == nID1) && (pListResourceRecords->at(i).irin[1].sName == sName2)) {
            result = pListResourceRecords->at(i);

            break;
        }
    }

    return result;
}

XPE::RESOURCE_RECORD XPE::getResourceRecord(const QString &sName1, quint32 nID2, QList<XPE::RESOURCE_RECORD> *pListResourceRecords)
{
    RESOURCE_RECORD result = {};

    result.nOffset = -1;

    qint32 nNumberOfResources = pListResourceRecords->count();

    for (qint32 i = 0; i < nNumberOfResources; i++) {
        if (pListResourceRecords->at(i).irin[0].sName == sName1) {
            if ((pListResourceRecords->at(i).irin[1].nID == nID2) || (nID2 == (quint32)-1)) {
                result = pListResourceRecords->at(i);

                break;
            }
        }
    }

    return result;
}

XPE::RESOURCE_RECORD XPE::getResourceRecord(const QString &sName1, const QString &sName2, QList<XPE::RESOURCE_RECORD> *pListResourceRecords)
{
    RESOURCE_RECORD result = {};

    result.nOffset = -1;

    qint32 nNumberOfResources = pListResourceRecords->count();

    for (qint32 i = 0; i < nNumberOfResources; i++) {
        if ((pListResourceRecords->at(i).irin[0].sName == sName1) && (pListResourceRecords->at(i).irin[1].sName == sName2)) {
            result = pListResourceRecords->at(i);

            break;
        }
    }

    return result;
}

QList<XPE::RESOURCE_RECORD> XPE::getResourceRecords(quint32 nID1, quint32 nID2, QList<RESOURCE_RECORD> *pListResourceRecords)
{
    QList<XPE::RESOURCE_RECORD> listResult;

    qint32 nNumberOfResources = pListResourceRecords->count();

    for (qint32 i = 0; i < nNumberOfResources; i++) {
        if (pListResourceRecords->at(i).irin[0].nID == nID1) {
            if ((pListResourceRecords->at(i).irin[1].nID == nID2) || (nID2 == (quint32)-1)) {
                listResult.append(pListResourceRecords->at(i));
            }
        }
    }

    return listResult;
}

bool XPE::isResourcePresent(quint32 nID1, quint32 nID2, QList<XPE::RESOURCE_RECORD> *pListResourceRecords)
{
    return (getResourceRecord(nID1, nID2, pListResourceRecords).nSize);
}

bool XPE::isResourcePresent(quint32 nID1, const QString &sName2, QList<XPE::RESOURCE_RECORD> *pListResourceRecords)
{
    return (getResourceRecord(nID1, sName2, pListResourceRecords).nSize);
}

bool XPE::isResourcePresent(const QString &sName1, quint32 nID2, QList<XPE::RESOURCE_RECORD> *pListResourceRecords)
{
    return (getResourceRecord(sName1, nID2, pListResourceRecords).nSize);
}

bool XPE::isResourcePresent(const QString &sName1, const QString &sName2, QList<XPE::RESOURCE_RECORD> *pListResourceRecords)
{
    return (getResourceRecord(sName1, sName2, pListResourceRecords).nSize);
}

bool XPE::isResourceStringTablePresent()
{
    QList<RESOURCE_RECORD> listResources = getResources(10000);

    return isResourceStringTablePresent(&listResources);
}

bool XPE::isResourceStringTablePresent(QList<RESOURCE_RECORD> *pListResourceRecords)
{
    return isResourcePresent(XPE_DEF::S_RT_STRING, -1, pListResourceRecords);
}

QList<XPE::RESOURCE_STRINGTABLE_RECORD> XPE::getResourceStringTableRecords()
{
    QList<RESOURCE_RECORD> listResources = getResources(10000);
    _MEMORY_MAP memoryMap = getMemoryMap();

    return getResourceStringTableRecords(&listResources, &memoryMap);
}

QList<XPE::RESOURCE_STRINGTABLE_RECORD> XPE::getResourceStringTableRecords(QList<RESOURCE_RECORD> *pListResourceRecords, XBinary::_MEMORY_MAP *pMemoryMap)
{
    QList<XPE::RESOURCE_STRINGTABLE_RECORD> listResult;

    QList<XPE::RESOURCE_RECORD> listResourceRecords = getResourceRecords(XPE_DEF::S_RT_STRING, -1, pListResourceRecords);

    qint32 nNumberOfRecords = listResourceRecords.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        RESOURCE_RECORD rh = listResourceRecords.at(i);

        if (rh.nOffset != -1) {
            qint64 nDataOffset = rh.nOffset;
            qint64 nDataSize = rh.nSize;

            if (isOffsetAndSizeValid(pMemoryMap, nDataOffset, nDataSize)) {
                quint32 nStartID = (rh.irin[1].nID - 1) * 16;
                quint32 nLanguage = rh.irin[2].nID;

                qint64 nCurrentOffset = nDataOffset;

                for (qint32 j = 0; j < 16; j++) {
                    quint16 nStringSize = read_uint16(nCurrentOffset);
                    qint64 _nOffset = nCurrentOffset;

                    nCurrentOffset += 2;

                    if (nCurrentOffset - nDataOffset >= nDataSize) {
                        break;
                    }

                    if (nStringSize) {
                        nStringSize = qMin((quint16)((nDataSize - nCurrentOffset) / 2), nStringSize);

                        QString sString = read_unicodeString(nCurrentOffset, nStringSize);

                        XPE::RESOURCE_STRINGTABLE_RECORD record = {};
                        record.nID = nStartID + j;
                        record.nLanguage = nLanguage;
                        record.sString = sString;
                        record.nOffset = _nOffset;
                        record.nAddress = offsetToAddress(pMemoryMap, _nOffset);
                        record.nSize = nStringSize;

                        listResult.append(record);

                        nCurrentOffset += 2 * nStringSize;

                        if (nCurrentOffset - nDataOffset >= nDataSize) {
                            break;
                        }
                    }
                }
            }
        }
    }

    return listResult;
}

bool XPE::isResourceManifestPresent()
{
    QList<RESOURCE_RECORD> listResources = getResources(10000);

    return isResourceManifestPresent(&listResources);
}

bool XPE::isResourceManifestPresent(QList<XPE::RESOURCE_RECORD> *pListResourceRecords)
{
    return isResourcePresent(XPE_DEF::S_RT_MANIFEST, -1, pListResourceRecords);
}

QString XPE::getResourceManifest()
{
    QList<RESOURCE_RECORD> listResources = getResources(10000);

    return getResourceManifest(&listResources);
}

QString XPE::getResourceManifest(QList<XPE::RESOURCE_RECORD> *pListResourceRecords)
{
    QString sResult;

    RESOURCE_RECORD rh = getResourceRecord(XPE_DEF::S_RT_MANIFEST, -1, pListResourceRecords);

    if (rh.nOffset != -1) {
        rh.nSize = qMin(rh.nSize, qint64(4000));
        sResult = read_ansiString(rh.nOffset, rh.nSize);
    }

    return sResult;
}

bool XPE::isResourceVersionPresent()
{
    QList<RESOURCE_RECORD> listResources = getResources(10000);

    return isResourceVersionPresent(&listResources);
}

bool XPE::isResourceVersionPresent(QList<XPE::RESOURCE_RECORD> *pListResourceRecords)
{
    return isResourcePresent(XPE_DEF::S_RT_VERSION, -1, pListResourceRecords);
}

XPE_DEF::S_VS_VERSION_INFO XPE::readVS_VERSION_INFO(qint64 nOffset)
{
    XPE_DEF::S_VS_VERSION_INFO result = {};

    result.wLength = read_uint16(nOffset + offsetof(XPE_DEF::S_VS_VERSION_INFO, wLength));
    result.wValueLength = read_uint16(nOffset + offsetof(XPE_DEF::S_VS_VERSION_INFO, wValueLength));
    result.wType = read_uint16(nOffset + offsetof(XPE_DEF::S_VS_VERSION_INFO, wType));

    read_array(nOffset, (char *)&result, sizeof(XPE_DEF::S_VS_VERSION_INFO));

    return result;
}

quint32 XPE::__getResourcesVersion(XPE::RESOURCES_VERSION *pResourcesVersionResult, qint64 nOffset, qint64 nSize, const QString &sPrefix, qint32 nLevel)
{
    QString _sPrefix = sPrefix;
    quint32 nResult = 0;

    if ((quint32)nSize >= sizeof(XPE_DEF::S_VS_VERSION_INFO)) {
        XPE_DEF::S_VS_VERSION_INFO vi = readVS_VERSION_INFO(nOffset);

        if (vi.wLength <= nSize) {
            if (vi.wValueLength < vi.wLength) {
                QString sTitle = read_unicodeString(nOffset + sizeof(XPE_DEF::S_VS_VERSION_INFO));

                qint32 nDelta = sizeof(XPE_DEF::S_VS_VERSION_INFO);
                nDelta += (sTitle.length() + 1) * (qint32)sizeof(quint16);
                nDelta = S_ALIGN_UP(nDelta, 4);

                if (_sPrefix != "") {
                    _sPrefix += ".";
                }

                _sPrefix += sTitle;

                if (_sPrefix == "VS_VERSION_INFO") {
                    if (vi.wValueLength >= sizeof(XPE_DEF::tagVS_FIXEDFILEINFO)) {
                        pResourcesVersionResult->nFixedFileInfoOffset = nOffset + nDelta;
                        // TODO Check Signature?
                        pResourcesVersionResult->fileInfo.dwSignature = read_uint32(nOffset + nDelta + offsetof(XPE_DEF::tagVS_FIXEDFILEINFO, dwSignature));
                        pResourcesVersionResult->fileInfo.dwStrucVersion = read_uint32(nOffset + nDelta + offsetof(XPE_DEF::tagVS_FIXEDFILEINFO, dwStrucVersion));
                        pResourcesVersionResult->fileInfo.dwFileVersionMS = read_uint32(nOffset + nDelta + offsetof(XPE_DEF::tagVS_FIXEDFILEINFO, dwFileVersionMS));
                        pResourcesVersionResult->fileInfo.dwFileVersionLS = read_uint32(nOffset + nDelta + offsetof(XPE_DEF::tagVS_FIXEDFILEINFO, dwFileVersionLS));
                        pResourcesVersionResult->fileInfo.dwProductVersionMS = read_uint32(nOffset + nDelta + offsetof(XPE_DEF::tagVS_FIXEDFILEINFO, dwProductVersionMS));
                        pResourcesVersionResult->fileInfo.dwProductVersionLS = read_uint32(nOffset + nDelta + offsetof(XPE_DEF::tagVS_FIXEDFILEINFO, dwProductVersionLS));
                        pResourcesVersionResult->fileInfo.dwFileFlagsMask = read_uint32(nOffset + nDelta + offsetof(XPE_DEF::tagVS_FIXEDFILEINFO, dwFileFlagsMask));
                        pResourcesVersionResult->fileInfo.dwFileFlags = read_uint32(nOffset + nDelta + offsetof(XPE_DEF::tagVS_FIXEDFILEINFO, dwFileFlags));
                        pResourcesVersionResult->fileInfo.dwFileOS = read_uint32(nOffset + nDelta + offsetof(XPE_DEF::tagVS_FIXEDFILEINFO, dwFileOS));
                        pResourcesVersionResult->fileInfo.dwFileType = read_uint32(nOffset + nDelta + offsetof(XPE_DEF::tagVS_FIXEDFILEINFO, dwFileType));
                        pResourcesVersionResult->fileInfo.dwFileSubtype = read_uint32(nOffset + nDelta + offsetof(XPE_DEF::tagVS_FIXEDFILEINFO, dwFileSubtype));
                        pResourcesVersionResult->fileInfo.dwFileDateMS = read_uint32(nOffset + nDelta + offsetof(XPE_DEF::tagVS_FIXEDFILEINFO, dwFileDateMS));
                        pResourcesVersionResult->fileInfo.dwFileDateLS = read_uint32(nOffset + nDelta + offsetof(XPE_DEF::tagVS_FIXEDFILEINFO, dwFileDateLS));
                    }
                }

                if (nLevel == 3) {
                    QString sValue = read_unicodeString(nOffset + nDelta);
                    _sPrefix += QString(":%1").arg(sValue);

                    pResourcesVersionResult->listRecords.append(_sPrefix);
                }

                if (_sPrefix == "VS_VERSION_INFO.VarFileInfo.Translation") {
                    if (vi.wValueLength == 4) {
                        quint32 nValue = read_uint32(nOffset + nDelta);
                        QString sValue = XBinary::valueToHex(nValue);
                        _sPrefix += QString(":%1").arg(sValue);

                        pResourcesVersionResult->listRecords.append(_sPrefix);
                    }
                }

                nDelta += vi.wValueLength;

                qint32 _nSize = vi.wLength - nDelta;

                if (nLevel < 3) {
                    while (_nSize > 0) {
                        qint32 _nDelta = __getResourcesVersion(pResourcesVersionResult, nOffset + nDelta, vi.wLength - nDelta, _sPrefix, nLevel + 1);

                        if (_nDelta == 0) {
                            break;
                        }

                        _nDelta = S_ALIGN_UP(_nDelta, 4);

                        nDelta += _nDelta;
                        _nSize -= _nDelta;
                    }
                }

                nResult = vi.wLength;
            }
        }
    }

    return nResult;
}

XPE::RESOURCES_VERSION XPE::getResourcesVersion()
{
    QList<RESOURCE_RECORD> listResourceRecords = getResources(10000);

    return getResourcesVersion(&listResourceRecords);
}

XPE::RESOURCES_VERSION XPE::getResourcesVersion(QList<XPE::RESOURCE_RECORD> *pListResourceRecords, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    RESOURCES_VERSION result = {};
    result.nFixedFileInfoOffset = -1;

    RESOURCE_RECORD resourceRecord = getResourceRecord(XPE_DEF::S_RT_VERSION, -1, pListResourceRecords);  // TODO pdstruct

    if (resourceRecord.nOffset != -1) {
        __getResourcesVersion(&result, resourceRecord.nOffset, resourceRecord.nSize, "", 0);
    }

    return result;
}

QString XPE::getFileVersion()
{
    RESOURCES_VERSION resoursesVersion = getResourcesVersion();

    return getFileVersion(&resoursesVersion);
}

QString XPE::getFileVersion(RESOURCES_VERSION *pResourceVersion)
{
    // TODO !!!
    //    return
    //    QString("%1.%2").arg(get_uint32_version(pResourceVersion->fileInfo.dwFileVersionMS)).arg(get_uint32_version(pResourceVersion->fileInfo.dwFileVersionLS));
    return getResourcesVersionValue("FileVersion", pResourceVersion);
}

QString XPE::getFileVersionMS(RESOURCES_VERSION *pResourceVersion)
{
    return get_uint32_version(pResourceVersion->fileInfo.dwFileVersionMS);
}

void XPE::setFixedFileInfo_dwSignature(quint32 nValue)
{
    qint64 nOffset = getResourcesVersion().nFixedFileInfoOffset;

    if (nOffset != -1) {
        write_uint32(nOffset + offsetof(XPE_DEF::tagVS_FIXEDFILEINFO, dwSignature), nValue);
    }
}

void XPE::setFixedFileInfo_dwStrucVersion(quint32 nValue)
{
    qint64 nOffset = getResourcesVersion().nFixedFileInfoOffset;

    if (nOffset != -1) {
        write_uint32(nOffset + offsetof(XPE_DEF::tagVS_FIXEDFILEINFO, dwStrucVersion), nValue);
    }
}

void XPE::setFixedFileInfo_dwFileVersionMS(quint32 nValue)
{
    qint64 nOffset = getResourcesVersion().nFixedFileInfoOffset;

    if (nOffset != -1) {
        write_uint32(nOffset + offsetof(XPE_DEF::tagVS_FIXEDFILEINFO, dwFileVersionMS), nValue);
    }
}

void XPE::setFixedFileInfo_dwFileVersionLS(quint32 nValue)
{
    qint64 nOffset = getResourcesVersion().nFixedFileInfoOffset;

    if (nOffset != -1) {
        write_uint32(nOffset + offsetof(XPE_DEF::tagVS_FIXEDFILEINFO, dwFileVersionLS), nValue);
    }
}

void XPE::setFixedFileInfo_dwProductVersionMS(quint32 nValue)
{
    qint64 nOffset = getResourcesVersion().nFixedFileInfoOffset;

    if (nOffset != -1) {
        write_uint32(nOffset + offsetof(XPE_DEF::tagVS_FIXEDFILEINFO, dwProductVersionMS), nValue);
    }
}

void XPE::setFixedFileInfo_dwProductVersionLS(quint32 nValue)
{
    qint64 nOffset = getResourcesVersion().nFixedFileInfoOffset;

    if (nOffset != -1) {
        write_uint32(nOffset + offsetof(XPE_DEF::tagVS_FIXEDFILEINFO, dwProductVersionLS), nValue);
    }
}

void XPE::setFixedFileInfo_dwFileFlagsMask(quint32 nValue)
{
    qint64 nOffset = getResourcesVersion().nFixedFileInfoOffset;

    if (nOffset != -1) {
        write_uint32(nOffset + offsetof(XPE_DEF::tagVS_FIXEDFILEINFO, dwFileFlagsMask), nValue);
    }
}

void XPE::setFixedFileInfo_dwFileFlags(quint32 nValue)
{
    qint64 nOffset = getResourcesVersion().nFixedFileInfoOffset;

    if (nOffset != -1) {
        write_uint32(nOffset + offsetof(XPE_DEF::tagVS_FIXEDFILEINFO, dwFileFlags), nValue);
    }
}

void XPE::setFixedFileInfo_dwFileOS(quint32 nValue)
{
    qint64 nOffset = getResourcesVersion().nFixedFileInfoOffset;

    if (nOffset != -1) {
        write_uint32(nOffset + offsetof(XPE_DEF::tagVS_FIXEDFILEINFO, dwFileOS), nValue);
    }
}

void XPE::setFixedFileInfo_dwFileType(quint32 nValue)
{
    qint64 nOffset = getResourcesVersion().nFixedFileInfoOffset;

    if (nOffset != -1) {
        write_uint32(nOffset + offsetof(XPE_DEF::tagVS_FIXEDFILEINFO, dwFileType), nValue);
    }
}

void XPE::setFixedFileInfo_dwFileSubtype(quint32 nValue)
{
    qint64 nOffset = getResourcesVersion().nFixedFileInfoOffset;

    if (nOffset != -1) {
        write_uint32(nOffset + offsetof(XPE_DEF::tagVS_FIXEDFILEINFO, dwFileSubtype), nValue);
    }
}

void XPE::setFixedFileInfo_dwFileDateMS(quint32 nValue)
{
    qint64 nOffset = getResourcesVersion().nFixedFileInfoOffset;

    if (nOffset != -1) {
        write_uint32(nOffset + offsetof(XPE_DEF::tagVS_FIXEDFILEINFO, dwFileDateMS), nValue);
    }
}

void XPE::setFixedFileInfo_dwFileDateLS(quint32 nValue)
{
    qint64 nOffset = getResourcesVersion().nFixedFileInfoOffset;

    if (nOffset != -1) {
        write_uint32(nOffset + offsetof(XPE_DEF::tagVS_FIXEDFILEINFO, dwFileDateLS), nValue);
    }
}

QString XPE::getResourcesVersionValue(const QString &sKey)
{
    QList<RESOURCE_RECORD> listResourceRecords = getResources(10000);
    RESOURCES_VERSION resVersion = getResourcesVersion(&listResourceRecords);

    return getResourcesVersionValue(sKey, &resVersion);
}

QString XPE::getResourcesVersionValue(const QString &sKey, XPE::RESOURCES_VERSION *pResourcesVersion)
{
    QString sResult;

    qint32 nNumberOfRecords = pResourcesVersion->listRecords.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        QString sRecord = pResourcesVersion->listRecords.at(i).section(".", 3, -1);
        QString _sKey = sRecord.section(":", 0, 0);

        if (_sKey == sKey) {
            sResult = sRecord.section(":", 1, -1);

            break;
        }
    }

    return sResult;
}

quint32 XPE::getResourceIdByNumber(quint32 nNumber)
{
    QList<RESOURCE_RECORD> listResources = getResources(10000);

    return getResourceIdByNumber(nNumber, &listResources);
}

quint32 XPE::getResourceIdByNumber(quint32 nNumber, QList<XPE::RESOURCE_RECORD> *pListResourceRecords)
{
    quint32 nResult = 0;

    if ((qint32)nNumber < pListResourceRecords->count()) {
        nResult = pListResourceRecords->at(nNumber).irin[1].nID;
    }

    return nResult;
}

QString XPE::getResourceNameByNumber(quint32 nNumber)
{
    QList<RESOURCE_RECORD> listResources = getResources(10000);

    return getResourceNameByNumber(nNumber, &listResources);
}

QString XPE::getResourceNameByNumber(quint32 nNumber, QList<XPE::RESOURCE_RECORD> *pListResourceRecords)
{
    QString sResult;

    if ((qint32)nNumber < pListResourceRecords->count()) {
        sResult = pListResourceRecords->at(nNumber).irin[1].sName;

        if (sResult == "") {
            sResult = pListResourceRecords->at(nNumber).irin[0].sName;
        }
    }

    return sResult;
}

qint64 XPE::getResourceOffsetByNumber(quint32 nNumber)
{
    QList<RESOURCE_RECORD> listResources = getResources(10000);

    return getResourceOffsetByNumber(nNumber, &listResources);
}

qint64 XPE::getResourceOffsetByNumber(quint32 nNumber, QList<XPE::RESOURCE_RECORD> *pListResourceRecords)
{
    qint64 nResult = -1;

    if ((qint32)nNumber < pListResourceRecords->count()) {
        nResult = pListResourceRecords->at(nNumber).nOffset;
    }

    return nResult;
}

qint64 XPE::getResourceSizeByNumber(quint32 nNumber)
{
    QList<RESOURCE_RECORD> listResources = getResources(10000);

    return getResourceSizeByNumber(nNumber, &listResources);
}

qint64 XPE::getResourceSizeByNumber(quint32 nNumber, QList<XPE::RESOURCE_RECORD> *pListResourceRecords)
{
    qint64 nResult = 0;

    if ((qint32)nNumber < pListResourceRecords->count()) {
        nResult = pListResourceRecords->at(nNumber).nSize;
    }

    return nResult;
}

quint32 XPE::getResourceTypeByNumber(quint32 nNumber)
{
    QList<RESOURCE_RECORD> listResources = getResources(10000);

    return getResourceTypeByNumber(nNumber, &listResources);
}

quint32 XPE::getResourceTypeByNumber(quint32 nNumber, QList<XPE::RESOURCE_RECORD> *pListResourceRecords)
{
    qint64 nResult = 0;

    if ((qint32)nNumber < pListResourceRecords->count()) {
        nResult = pListResourceRecords->at(nNumber).irin[0].nID;
    }

    return nResult;
}

qint64 XPE::getResourceNameOffset(const QString &sName)
{
    QList<RESOURCE_RECORD> listResources = getResources(10000);

    return getResourceNameOffset(sName, &listResources);
}

qint64 XPE::getResourceNameOffset(const QString &sName, QList<XPE::RESOURCE_RECORD> *pListResourceRecords)
{
    qint64 nResult = -1;

    qint32 nNumberOfResources = pListResourceRecords->count();

    for (qint32 i = 0; i < nNumberOfResources; i++) {
        if (pListResourceRecords->at(i).irin[1].sName == sName) {
            nResult = pListResourceRecords->at(i).nOffset;
            break;
        }
    }

    return nResult;
}

qint64 XPE::getResourceGroupNameOffset(const QString &sName)
{
    QList<RESOURCE_RECORD> listResources = getResources(10000);

    return getResourceGroupNameOffset(sName, &listResources);
}

qint64 XPE::getResourceGroupNameOffset(const QString &sName, QList<XPE::RESOURCE_RECORD> *pListResourceRecords)
{
    qint64 nResult = -1;

    qint32 nNumberOfResources = pListResourceRecords->count();

    for (qint32 i = 0; i < nNumberOfResources; i++) {
        if (pListResourceRecords->at(i).irin[0].sName == sName) {
            nResult = pListResourceRecords->at(i).nOffset;
            break;
        }
    }

    return nResult;
}

qint64 XPE::getResourceGroupIdOffset(quint32 nID)
{
    QList<RESOURCE_RECORD> listResources = getResources(10000);

    return getResourceGroupIdOffset(nID, &listResources);
}

qint64 XPE::getResourceGroupIdOffset(quint32 nID, QList<XPE::RESOURCE_RECORD> *pListResourceRecords)
{
    qint64 nResult = -1;

    qint32 nNumberOfResources = pListResourceRecords->count();

    for (qint32 i = 0; i < nNumberOfResources; i++) {
        if (pListResourceRecords->at(i).irin[0].nID == nID) {
            nResult = pListResourceRecords->at(i).nOffset;
            break;
        }
    }

    return nResult;
}

bool XPE::isResourceNamePresent(const QString &sName)
{
    QList<RESOURCE_RECORD> listResources = getResources(10000);

    return isResourceNamePresent(sName, &listResources);
}

bool XPE::isResourceNamePresent(const QString &sName, QList<XPE::RESOURCE_RECORD> *pListResourceRecords)
{
    return (getResourceNameOffset(sName, pListResourceRecords) != -1);
}

bool XPE::isResourceGroupNamePresent(const QString &sName)
{
    QList<RESOURCE_RECORD> listResources = getResources(10000);

    return isResourceGroupNamePresent(sName, &listResources);
}

bool XPE::isResourceGroupNamePresent(const QString &sName, QList<XPE::RESOURCE_RECORD> *pListResourceRecords)
{
    return (getResourceGroupNameOffset(sName, pListResourceRecords) != -1);
}

bool XPE::isResourceGroupIdPresent(quint32 nID)
{
    QList<RESOURCE_RECORD> listResources = getResources(10000);

    return isResourceGroupIdPresent(nID, &listResources);
}

bool XPE::isResourceGroupIdPresent(quint32 nID, QList<XPE::RESOURCE_RECORD> *pListResourceRecords)
{
    return (getResourceGroupIdOffset(nID, pListResourceRecords) != -1);
}

bool XPE::isResourceGroupIconsPresent()
{
    QList<RESOURCE_RECORD> listResources = getResources(10000);

    return isResourceGroupIconsPresent(&listResources);
}

bool XPE::isResourceGroupIconsPresent(QList<RESOURCE_RECORD> *pListResourceRecords)
{
    return isResourcePresent(XPE_DEF::S_RT_GROUP_ICON, -1, pListResourceRecords);
}

bool XPE::isResourceGroupCursorsPresent()
{
    QList<RESOURCE_RECORD> listResources = getResources(10000);

    return isResourceGroupCursorsPresent(&listResources);
}

bool XPE::isResourceGroupCursorsPresent(QList<RESOURCE_RECORD> *pListResourceRecords)
{
    return isResourcePresent(XPE_DEF::S_RT_GROUP_CURSOR, -1, pListResourceRecords);
}

QString XPE::resourceRecordToString(const RESOURCE_RECORD &resourceRecord)
{
    QString sResult;

    QString sResID1 = XPE::resourceIdNameToString(resourceRecord.irin[0], 0);
    QString sResID2 = XPE::resourceIdNameToString(resourceRecord.irin[1], 1);
    QString sResID3 = XPE::resourceIdNameToString(resourceRecord.irin[2], 2);

    sResult = QString("%1_%2_%3.bin").arg(sResID1).arg(sResID2).arg(sResID3);

    return sResult;
}

XADDR XPE::getModuleAddress()
{
    // mb TODO
    return XBinary::getModuleAddress();
}

QList<XBinary::MAPMODE> XPE::getMapModesList()
{
    QList<MAPMODE> listResult;

    listResult.append(MAPMODE_SECTIONS);

    return listResult;
}

XPE_DEF::IMAGE_IMPORT_DESCRIPTOR XPE::read_IMAGE_IMPORT_DESCRIPTOR(qint64 nOffset)
{
    XPE_DEF::IMAGE_IMPORT_DESCRIPTOR result = {};

    result.OriginalFirstThunk = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR, OriginalFirstThunk));
    result.TimeDateStamp = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR, TimeDateStamp));
    result.ForwarderChain = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR, ForwarderChain));
    result.Name = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR, Name));
    result.FirstThunk = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR, FirstThunk));

    return result;
}

void XPE::write_IMAGE_IMPORT_DESCRIPTOR(qint64 nOffset, XPE_DEF::IMAGE_IMPORT_DESCRIPTOR idd)
{
    // TODO !!
    write_array(nOffset, (char *)&idd, sizeof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR));
}

XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR XPE::_read_IMAGE_DELAYLOAD_DESCRIPTOR(qint64 nOffset)
{
    XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR result = {};

    result.AllAttributes = read_uint32(nOffset + offsetof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR, AllAttributes));
    result.DllNameRVA = read_uint32(nOffset + offsetof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR, DllNameRVA));
    result.ModuleHandleRVA = read_uint32(nOffset + offsetof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR, ModuleHandleRVA));
    result.ImportAddressTableRVA = read_uint32(nOffset + offsetof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR, ImportAddressTableRVA));
    result.ImportNameTableRVA = read_uint32(nOffset + offsetof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR, ImportNameTableRVA));
    result.BoundImportAddressTableRVA = read_uint32(nOffset + offsetof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR, BoundImportAddressTableRVA));
    result.UnloadInformationTableRVA = read_uint32(nOffset + offsetof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR, UnloadInformationTableRVA));
    result.TimeDateStamp = read_uint32(nOffset + offsetof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR, TimeDateStamp));

    return result;
}

bool XPE::isExportPresent()
{
    return isOptionalHeader_DataDirectoryPresent(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXPORT);
}

XPE::EXPORT_HEADER XPE::getExport(bool bValidOnly, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    _MEMORY_MAP memoryMap = getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);

    return getExport(&memoryMap, bValidOnly, pPdStruct);
}

XPE::EXPORT_HEADER XPE::getExport(_MEMORY_MAP *pMemoryMap, bool bValidOnly, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    EXPORT_HEADER result = {};

    qint64 nExportOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXPORT);

    if (nExportOffset != -1) {
        read_array(nExportOffset, (char *)&result.directory, sizeof(XPE_DEF::IMAGE_EXPORT_DIRECTORY));

        qint64 nNameOffset = addressToOffset(pMemoryMap, result.directory.Name + pMemoryMap->nModuleAddress);

        if (nNameOffset != -1) {
            result.sName = read_ansiString(nNameOffset);
        }

        qint64 nAddressOfFunctionsOffset = addressToOffset(pMemoryMap, result.directory.AddressOfFunctions + pMemoryMap->nModuleAddress);
        qint64 nAddressOfNamesOffset = addressToOffset(pMemoryMap, result.directory.AddressOfNames + pMemoryMap->nModuleAddress);
        qint64 nAddressOfNameOrdinalsOffset = addressToOffset(pMemoryMap, result.directory.AddressOfNameOrdinals + pMemoryMap->nModuleAddress);

        if ((result.directory.NumberOfFunctions < 0xFFFF) && (result.directory.NumberOfNames < 0xFFFF)) {
            if ((nAddressOfFunctionsOffset != -1) && (nAddressOfNamesOffset != -1) && (nAddressOfNameOrdinalsOffset != -1)) {
                qint32 _nFreeIndexScan = XBinary::getFreeIndex(pPdStruct);
                XBinary::setPdStructInit(pPdStruct, _nFreeIndexScan, result.directory.NumberOfFunctions);

                QMap<quint16, EXPORT_POSITION> mapNames;

                for (qint32 i = 0; (i < (int)result.directory.NumberOfNames) && (!(pPdStruct->bIsStop)); i++) {
                    EXPORT_POSITION position = {};

                    qint32 nIndex = read_uint16(nAddressOfNameOrdinalsOffset + 2 * i);
                    position.nOrdinal = nIndex + result.directory.Base;
                    position.nRVA = read_uint32(nAddressOfFunctionsOffset + 4 * nIndex);
                    position.nAddress = position.nRVA + pMemoryMap->nModuleAddress;
                    position.nNameRVA = read_uint32(nAddressOfNamesOffset + 4 * i);

                    qint64 nFunctionNameOffset = addressToOffset(pMemoryMap, position.nNameRVA + pMemoryMap->nModuleAddress);

                    if (nFunctionNameOffset != -1) {
                        position.sFunctionName = read_ansiString(nFunctionNameOffset, 2048);
                    }

                    mapNames.insert(position.nOrdinal, position);

                    XBinary::setPdStructCurrentIncrement(pPdStruct, _nFreeIndexScan);
                    XBinary::setPdStructStatus(pPdStruct, _nFreeIndexScan, position.sFunctionName);
                }

                for (qint32 i = 0; (i < (int)result.directory.NumberOfFunctions) && (!(pPdStruct->bIsStop)); i++) {
                    EXPORT_POSITION position = {};

                    qint32 nIndex = i;
                    position.nOrdinal = nIndex + result.directory.Base;

                    if (mapNames.contains(position.nOrdinal)) {
                        position = mapNames.value(position.nOrdinal);
                    } else {
                        position.nRVA = read_uint32(nAddressOfFunctionsOffset + 4 * nIndex);
                        position.nAddress = position.nRVA + pMemoryMap->nModuleAddress;
                    }

                    bool bInsert = true;

                    if (bValidOnly) {
                        if ((position.nRVA == 0) || (!isAddressValid(pMemoryMap, position.nAddress))) {
                            bInsert = false;
                        }
                    }

                    if (bInsert) {
                        result.listPositions.append(position);
                    }

                    XBinary::setPdStructCurrentIncrement(pPdStruct, _nFreeIndexScan);
                }

                XBinary::setPdStructFinished(pPdStruct, _nFreeIndexScan);
            }
        }
    }

    return result;
}

QList<QString> XPE::getExportFunctionsList(PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    EXPORT_HEADER exportHeader = getExport(false, pPdStruct);

    return getExportFunctionsList(&exportHeader, pPdStruct);
}

QList<QString> XPE::getExportFunctionsList(EXPORT_HEADER *pExportHeader, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    QList<QString> listResult;

    qint32 nNumberOfPositions = pExportHeader->listPositions.count();

    for (qint32 i = 0; (i < nNumberOfPositions) && (!(pPdStruct->bIsStop)); i++) {
        listResult.append(pExportHeader->listPositions.at(i).sFunctionName);
    }

    return listResult;
}
QList<XADDR> XPE::getExportFunctionAddressesList(PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    _MEMORY_MAP memoryMap = getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);
    XPE_DEF::IMAGE_EXPORT_DIRECTORY ied = getExportDirectory();

    return getExportFunctionAddressesList(&memoryMap, &ied, pPdStruct);
}

QList<XADDR> XPE::getExportFunctionAddressesList(_MEMORY_MAP *pMemoryMap, XPE_DEF::IMAGE_EXPORT_DIRECTORY *pIED, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    QList<XADDR> listResult;

    qint32 nSize = pIED->NumberOfFunctions * 4;

    qint64 nOffset = XBinary::addressToOffset(pMemoryMap, pIED->AddressOfFunctions + pMemoryMap->nModuleAddress);

    QByteArray baData = read_array(nOffset, nSize);

    char *_pData = baData.data();
    qint32 _nSize = baData.size();

    for (qint32 i = 0; (i < _nSize) && (!(pPdStruct->bIsStop)); i += 4) {
        quint32 nAddress = _read_uint32(_pData + i);

        listResult.append(nAddress + pMemoryMap->nModuleAddress);
    }

    return listResult;
}

QList<quint16> XPE::getExportNameOrdinalsList(_MEMORY_MAP *pMemoryMap, XPE_DEF::IMAGE_EXPORT_DIRECTORY *pIED, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    QList<quint16> listResult;

    qint32 nSize = pIED->NumberOfNames * 2;

    qint64 nOffset = XBinary::addressToOffset(pMemoryMap, pIED->AddressOfNameOrdinals + pMemoryMap->nModuleAddress);

    QByteArray baData = read_array(nOffset, nSize);

    char *_pData = baData.data();
    qint32 _nSize = baData.size();

    for (qint32 i = 0; (i < _nSize) && (!(pPdStruct->bIsStop)); i += 2) {
        quint16 nOrdinal = _read_uint16(_pData + i);

        listResult.append(nOrdinal);
    }

    return listResult;
}

QList<XADDR> XPE::getExportNamesList(_MEMORY_MAP *pMemoryMap, XPE_DEF::IMAGE_EXPORT_DIRECTORY *pIED, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    QList<XADDR> listResult;

    qint32 nSize = pIED->NumberOfNames * 4;

    qint64 nOffset = XBinary::addressToOffset(pMemoryMap, pIED->AddressOfNames + pMemoryMap->nModuleAddress);

    QByteArray baData = read_array(nOffset, nSize);

    char *_pData = baData.data();
    qint32 _nSize = baData.size();

    for (qint32 i = 0; (i < _nSize) && (!(pPdStruct->bIsStop)); i += 4) {
        quint32 nAddress = _read_uint32(_pData + i);

        listResult.append(nAddress + pMemoryMap->nModuleAddress);
    }

    return listResult;
}

XPE_DEF::IMAGE_EXPORT_DIRECTORY XPE::read_IMAGE_EXPORT_DIRECTORY(qint64 nOffset)
{
    XPE_DEF::IMAGE_EXPORT_DIRECTORY result = {};

    result.Characteristics = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, Characteristics));
    result.TimeDateStamp = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, TimeDateStamp));
    result.MajorVersion = read_uint16(nOffset + offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, MajorVersion));
    result.MinorVersion = read_uint16(nOffset + offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, MinorVersion));
    result.Name = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, Name));
    result.Base = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, Base));
    result.NumberOfFunctions = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, NumberOfFunctions));
    result.NumberOfNames = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, NumberOfNames));
    result.AddressOfFunctions = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, AddressOfFunctions));
    result.AddressOfNames = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, AddressOfNames));
    result.AddressOfNameOrdinals = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, AddressOfNameOrdinals));

    return result;
}

void XPE::write_IMAGE_EXPORT_DIRECTORY(qint64 nOffset, XPE_DEF::IMAGE_EXPORT_DIRECTORY *pIED)
{
    write_uint32(nOffset + offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, Characteristics), pIED->Characteristics);
    write_uint32(nOffset + offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, TimeDateStamp), pIED->TimeDateStamp);
    write_uint16(nOffset + offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, MajorVersion), pIED->MajorVersion);
    write_uint16(nOffset + offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, MinorVersion), pIED->MinorVersion);
    write_uint32(nOffset + offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, Name), pIED->Name);
    write_uint32(nOffset + offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, Base), pIED->Base);
    write_uint32(nOffset + offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, NumberOfFunctions), pIED->NumberOfFunctions);
    write_uint32(nOffset + offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, NumberOfNames), pIED->NumberOfNames);
    write_uint32(nOffset + offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, AddressOfFunctions), pIED->AddressOfFunctions);
    write_uint32(nOffset + offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, AddressOfNames), pIED->AddressOfNames);
    write_uint32(nOffset + offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, AddressOfNameOrdinals), pIED->AddressOfNameOrdinals);
}

XPE_DEF::IMAGE_EXPORT_DIRECTORY XPE::getExportDirectory()
{
    XPE_DEF::IMAGE_EXPORT_DIRECTORY result = {};

    qint64 nExportOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXPORT);

    if (nExportOffset != -1) {
        result = read_IMAGE_EXPORT_DIRECTORY(nExportOffset);
    }

    return result;
}

void XPE::setExportDirectory(XPE_DEF::IMAGE_EXPORT_DIRECTORY *pExportDirectory)
{
    qint64 nExportOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXPORT);

    if (nExportOffset != -1) {
        write_IMAGE_EXPORT_DIRECTORY(nExportOffset, pExportDirectory);
    }
}

void XPE::setExportDirectory_Characteristics(quint32 nValue)
{
    qint64 nExportOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXPORT);

    if (nExportOffset != -1) {
        write_uint32(nExportOffset + offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, Characteristics), nValue);
    }
}

void XPE::setExportDirectory_TimeDateStamp(quint32 nValue)
{
    qint64 nExportOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXPORT);

    if (nExportOffset != -1) {
        write_uint32(nExportOffset + offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, TimeDateStamp), nValue);
    }
}

void XPE::setExportDirectory_MajorVersion(quint16 nValue)
{
    qint64 nExportOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXPORT);

    if (nExportOffset != -1) {
        write_uint16(nExportOffset + offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, MajorVersion), nValue);
    }
}

void XPE::setExportDirectory_MinorVersion(quint16 nValue)
{
    qint64 nExportOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXPORT);

    if (nExportOffset != -1) {
        write_uint16(nExportOffset + offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, MinorVersion), nValue);
    }
}

void XPE::setExportDirectory_Name(quint32 nValue)
{
    qint64 nExportOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXPORT);

    if (nExportOffset != -1) {
        write_uint32(nExportOffset + offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, Name), nValue);
    }
}

void XPE::setExportDirectory_Base(quint32 nValue)
{
    qint64 nExportOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXPORT);

    if (nExportOffset != -1) {
        write_uint32(nExportOffset + offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, Base), nValue);
    }
}

void XPE::setExportDirectory_NumberOfFunctions(quint32 nValue)
{
    qint64 nExportOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXPORT);

    if (nExportOffset != -1) {
        write_uint32(nExportOffset + offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, NumberOfFunctions), nValue);
    }
}

void XPE::setExportDirectory_NumberOfNames(quint32 nValue)
{
    qint64 nExportOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXPORT);

    if (nExportOffset != -1) {
        write_uint32(nExportOffset + offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, NumberOfNames), nValue);
    }
}

void XPE::setExportDirectory_AddressOfFunctions(quint32 nValue)
{
    qint64 nExportOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXPORT);

    if (nExportOffset != -1) {
        write_uint32(nExportOffset + offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, AddressOfFunctions), nValue);
    }
}

void XPE::setExportDirectory_AddressOfNames(quint32 nValue)
{
    qint64 nExportOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXPORT);

    if (nExportOffset != -1) {
        write_uint32(nExportOffset + offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, AddressOfNames), nValue);
    }
}

void XPE::setExportDirectory_AddressOfNameOrdinals(quint32 nValue)
{
    qint64 nExportOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXPORT);

    if (nExportOffset != -1) {
        write_uint32(nExportOffset + offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, AddressOfNameOrdinals), nValue);
    }
}

QByteArray XPE::getHeaders()
{
    // TODO Check
    QByteArray baResult;

    qint32 nSizeOfHeaders = getOptionalHeader_SizeOfHeaders();

    if (isImage()) {
        quint32 nSectionAlignment = getOptionalHeader_SectionAlignment();
        nSizeOfHeaders = S_ALIGN_UP(nSizeOfHeaders, nSectionAlignment);
    }

    baResult = read_array(0, nSizeOfHeaders);

    if (baResult.size() != nSizeOfHeaders) {
        baResult.resize(0);
    }

    return baResult;
}

XBinary::OFFSETSIZE XPE::__getSectionOffsetAndSize(quint32 nSection)
{
    OFFSETSIZE osResult = {};

    XPE_DEF::IMAGE_SECTION_HEADER sectionHeader = getSectionHeader(nSection);
    qint64 nSectionAlignment = getOptionalHeader_SectionAlignment();
    qint64 nFileAlignment = getOptionalHeader_FileAlignment();

    if (nFileAlignment == nSectionAlignment) {
        nFileAlignment = 1;
    }

    bool bIsSectionValid = false;

    if (!isImage()) {
        bIsSectionValid = (bool)(sectionHeader.SizeOfRawData != 0);
    } else {
        bIsSectionValid = (bool)(sectionHeader.Misc.VirtualSize != 0);
    }

    if (bIsSectionValid) {
        qint64 nSectionOffset = 0;
        qint64 nSectionSize = 0;

        if (!isImage()) {
            nSectionOffset = sectionHeader.PointerToRawData;
            nSectionOffset = S_ALIGN_DOWN64(nSectionOffset, nFileAlignment);
            nSectionSize = sectionHeader.SizeOfRawData + (sectionHeader.PointerToRawData - nSectionOffset);
        } else {
            nSectionOffset = sectionHeader.VirtualAddress;
            nSectionSize = sectionHeader.Misc.VirtualSize;
        }

        osResult = convertOffsetAndSize(nSectionOffset, nSectionSize);
    } else {
        osResult.nOffset = -1;
    }

    return osResult;
}

QByteArray XPE::getSection(quint32 nSection)
{
    QByteArray baResult;

    OFFSETSIZE osRegion = __getSectionOffsetAndSize(nSection);

    if (osRegion.nOffset != -1) {
        baResult = read_array(osRegion.nOffset, osRegion.nSize);

        if (baResult.size() != osRegion.nSize)  // TODO check???
        {
            baResult.resize(0);
        }
    }

    return baResult;
}

bool XPE::addImportSection(QMap<qint64, QString> *pMapIAT, PDSTRUCT *pPdStruct)
{
    return addImportSection(getDevice(), isImage(), pMapIAT, pPdStruct);
}

bool XPE::addImportSection(QIODevice *pDevice, bool bIsImage, QMap<qint64, QString> *pMapIAT, PDSTRUCT *pPdStruct)
{
#ifdef QT_DEBUG
    QElapsedTimer timer;
    timer.start();
    qDebug("addImportSection");
#endif

    bool bResult = false;

    if (isResizeEnable(pDevice)) {
        XPE pe(pDevice, bIsImage);

        if (pe.isValid()) {
            QList<IMPORT_HEADER> listImportHeaders = mapIATToList(pMapIAT, pe.is64());
#ifdef QT_DEBUG
            qDebug("addImportSection:mapIATToList: %lld msec", timer.elapsed());
#endif
            bResult = setImports(pDevice, bIsImage, &listImportHeaders, pPdStruct);
#ifdef QT_DEBUG
            qDebug("addImportSection:setImports: %lld msec", timer.elapsed());
#endif
        }
    }

#ifdef QT_DEBUG
    qDebug("addImportSection: %lld msec", timer.elapsed());
#endif

    return bResult;
}

bool XPE::addImportSection(const QString &sFileName, bool bIsImage, QMap<qint64, QString> *pMapIAT, PDSTRUCT *pPdStruct)
{
    bool bResult = false;

    QFile file(sFileName);

    if (file.open(QIODevice::ReadWrite)) {
        bResult = addImportSection(&file, bIsImage, pMapIAT, pPdStruct);

        file.close();
    }

    return bResult;
}

QList<XPE::IMPORT_HEADER> XPE::mapIATToList(QMap<qint64, QString> *pMapIAT, bool bIs64)
{
    QList<IMPORT_HEADER> listResult;

    IMPORT_HEADER record = {};

    quint64 nCurrentRVA = 0;

    quint32 nStep = 0;

    if (bIs64) {
        nStep = 8;
    } else {
        nStep = 4;
    }

    QMapIterator<qint64, QString> i(*pMapIAT);

    while (i.hasNext()) {
        i.next();

        QString sLibrary = i.value().section("#", 0, 0);
        QString sFunction = i.value().section("#", 1, 1);

        if (((qint64)(nCurrentRVA + nStep) != i.key()) || ((record.sName != "") && (record.sName != sLibrary))) {
            if (record.sName != "") {
                listResult.append(record);
            }

            record.sName = sLibrary;
            record.nFirstThunk = i.key();
            record.listPositions.clear();
        }

        nCurrentRVA = i.key();

        IMPORT_POSITION position = {};

        position.nHint = 0;

        if (sFunction.toInt()) {
            position.nOrdinal = sFunction.toInt();
        } else {
            position.sName = sFunction;
        }

        position.nThunkRVA = i.key();

        record.listPositions.append(position);

        if (!i.hasNext()) {
            if (record.sName != "") {
                listResult.append(record);
            }
        }
    }

    return listResult;
}

quint32 XPE::calculateCheckSum(PDSTRUCT *pPdStruct)
{
    quint32 nCalcSum = _checkSum(0, getSize(), pPdStruct);
    quint32 nHdrSum = getOptionalHeader_CheckSum();

    if (S_LOWORD(nCalcSum) >= S_LOWORD(nHdrSum)) {
        nCalcSum -= S_LOWORD(nHdrSum);
    } else {
        nCalcSum = ((S_LOWORD(nCalcSum) - S_LOWORD(nHdrSum)) & 0xFFFF) - 1;
    }

    if (S_LOWORD(nCalcSum) >= S_HIWORD(nHdrSum))  //!!!!!
    {
        nCalcSum -= S_HIWORD(nHdrSum);
    } else {
        nCalcSum = ((S_LOWORD(nCalcSum) - S_HIWORD(nHdrSum)) & 0xFFFF) - 1;
    }

    nCalcSum += getSize();

    return nCalcSum;
}

bool XPE::addSection(const QString &sFileName, bool bIsImage, XPE_DEF::IMAGE_SECTION_HEADER *pSectionHeader, char *pData, qint64 nDataSize, PDSTRUCT *pPdStruct)
{
    bool bResult = false;

    QFile file(sFileName);

    if (file.open(QIODevice::ReadWrite)) {
        bResult = addSection(&file, bIsImage, pSectionHeader, pData, nDataSize, pPdStruct);

        file.close();
    } else {
        _errorMessage(QString("%1: %2").arg(tr("Cannot open file")).arg(sFileName), pPdStruct);
    }

    return bResult;
}

bool XPE::addSection(QIODevice *pDevice, bool bIsImage, XPE_DEF::IMAGE_SECTION_HEADER *pSectionHeader, char *pData, qint64 nDataSize, PDSTRUCT *pPdStruct)
{
    bool bResult = false;

    if (isResizeEnable(pDevice)) {
        XPE pe(pDevice, bIsImage);

        if (pe.isValid()) {
            qint64 nHeadersSize = pe._fixHeadersSize();
            qint64 nNewHeadersSize = pe._calculateHeadersSize(pe.getSectionsTableOffset(), pe.getFileHeader_NumberOfSections() + 1);
            qint64 nFileAlignment = pe.getOptionalHeader_FileAlignment();
            qint64 nSectionAlignment = pe.getOptionalHeader_SectionAlignment();

            if (pSectionHeader->PointerToRawData == 0) {
                pSectionHeader->PointerToRawData = pe._calculateRawSize(pPdStruct);
            }

            if (pSectionHeader->SizeOfRawData == 0) {
                pSectionHeader->SizeOfRawData = S_ALIGN_UP64(nDataSize, nFileAlignment);
            }

            if (pSectionHeader->VirtualAddress == 0) {
                pSectionHeader->VirtualAddress = S_ALIGN_UP64(pe.getOptionalHeader_SizeOfImage(), nSectionAlignment);
            }

            if (pSectionHeader->Misc.VirtualSize == 0) {
                pSectionHeader->Misc.VirtualSize = S_ALIGN_UP64(nDataSize, nSectionAlignment);
            }

            qint64 nDelta = nNewHeadersSize - nHeadersSize;
            qint64 nFileSize = pDevice->size();

            if (nDelta > 0) {
                resize(pDevice, nFileSize + nDelta);
                pe.moveMemory(nHeadersSize, nNewHeadersSize, nFileSize - nHeadersSize);
            } else if (nDelta < 0) {
                pe.moveMemory(nHeadersSize, nNewHeadersSize, nFileSize - nHeadersSize);
                resize(pDevice, nFileSize + nDelta);
            }

            pe._fixFileOffsets(nDelta);

            pSectionHeader->PointerToRawData += nDelta;
            nFileSize += nDelta;

            if (nFileSize < nHeadersSize) {
                nFileSize = nHeadersSize;
            }

            // TODO
            resize(pDevice, nFileSize + pSectionHeader->SizeOfRawData);

            quint32 nNumberOfSections = pe.getFileHeader_NumberOfSections();
            pe.setFileHeader_NumberOfSections(nNumberOfSections + 1);
            pe.setSectionHeader(nNumberOfSections, pSectionHeader);

            // Overlay
            if (pe.isOverlayPresent(pPdStruct)) {
                qint64 nOverlayOffset = pe.getOverlayOffset(pPdStruct);
                qint64 nOverlaySize = pe.getOverlaySize(pPdStruct);
                pe.moveMemory(nOverlayOffset - pSectionHeader->SizeOfRawData, nOverlayOffset, nOverlaySize);
            }

            pe.write_array(pSectionHeader->PointerToRawData, pData, nDataSize);

            pe.zeroFill(pSectionHeader->PointerToRawData + nDataSize, (pSectionHeader->SizeOfRawData) - nDataSize);

            qint64 nNewImageSize = S_ALIGN_UP(pSectionHeader->VirtualAddress + pSectionHeader->Misc.VirtualSize, nSectionAlignment);
            pe.setOptionalHeader_SizeOfImage(nNewImageSize);

            // TODO flag
            pe.fixCheckSum();

            bResult = true;
        }
    }

    return bResult;
}

bool XPE::removeLastSection(PDSTRUCT *pPdStruct)
{
    return removeLastSection(getDevice(), isImage(), pPdStruct);
}

bool XPE::removeLastSection(QIODevice *pDevice, bool bIsImage, PDSTRUCT *pPdStruct)
{
    // TODO Check
    bool bResult = false;

    if (isResizeEnable(pDevice)) {
        XPE pe(pDevice, bIsImage);

        if (pe.isValid()) {
            qint32 nNumberOfSections = pe.getFileHeader_NumberOfSections();

            if (nNumberOfSections) {
                qint64 nHeadersSize = pe._fixHeadersSize();
                qint64 nNewHeadersSize = pe._calculateHeadersSize(pe.getSectionsTableOffset(), nNumberOfSections - 1);
                quint32 nFileAlignment = pe.getOptionalHeader_FileAlignment();
                quint32 nSectionAlignment = pe.getOptionalHeader_SectionAlignment();
                bool bIsOverlayPresent = pe.isOverlayPresent(pPdStruct);
                qint64 nOverlayOffset = pe.getOverlayOffset(pPdStruct);
                qint64 nOverlaySize = pe.getOverlaySize(pPdStruct);

                XPE_DEF::IMAGE_SECTION_HEADER ish = pe.getSectionHeader(nNumberOfSections - 1);
                XPE_DEF::IMAGE_SECTION_HEADER ish0 = {};
                pe.setSectionHeader(nNumberOfSections - 1, &ish0);
                pe.setFileHeader_NumberOfSections(nNumberOfSections - 1);

                ish.SizeOfRawData = S_ALIGN_UP(ish.SizeOfRawData, nFileAlignment);
                ish.Misc.VirtualSize = S_ALIGN_UP(ish.Misc.VirtualSize, nSectionAlignment);

                qint64 nDelta = nNewHeadersSize - nHeadersSize;
                qint64 nFileSize = pDevice->size();

                if (nDelta > 0) {
                    resize(pDevice, nFileSize + nDelta);
                    pe.moveMemory(nHeadersSize, nNewHeadersSize, nFileSize - nHeadersSize);
                } else if (nDelta < 0) {
                    pe.moveMemory(nHeadersSize, nNewHeadersSize, nFileSize - nHeadersSize);
                    resize(pDevice, nFileSize + nDelta);
                }

                pe._fixFileOffsets(nDelta);

                nFileSize += nDelta;
                nOverlayOffset += nDelta;

                if (bIsOverlayPresent) {
                    pe.moveMemory(nOverlayOffset, nOverlayOffset - ish.SizeOfRawData, nOverlaySize);
                }

                resize(pDevice, nFileSize - ish.SizeOfRawData);

                qint64 nNewImageSize = S_ALIGN_UP(ish.VirtualAddress, nSectionAlignment);
                pe.setOptionalHeader_SizeOfImage(nNewImageSize);

                pe.fixCheckSum();

                bResult = true;
            }
        }
    }

    return bResult;
}

bool XPE::removeLastSection(const QString &sFileName, bool bIsImage, PDSTRUCT *pPdStruct)
{
    bool bResult = false;

    QFile file(sFileName);

    if (file.open(QIODevice::ReadWrite)) {
        bResult = removeLastSection(&file, bIsImage, pPdStruct);

        file.close();
    }

    return bResult;
}

bool XPE::addSection(XPE_DEF::IMAGE_SECTION_HEADER *pSectionHeader, char *pData, qint64 nDataSize, PDSTRUCT *pPdStruct)
{
    return addSection(getDevice(), isImage(), pSectionHeader, pData, nDataSize, pPdStruct);
}

XPE::RESOURCE_POSITION XPE::_getResourcePosition(XBinary::_MEMORY_MAP *pMemoryMap, qint64 nBaseAddress, qint64 nResourceOffset, qint64 nOffset, quint32 nLevel)
{
    RESOURCE_POSITION result = {};

    result.nOffset = nOffset;
    result.nLevel = nLevel;
    result.dirEntry = read_IMAGE_RESOURCE_DIRECTORY_ENTRY(nOffset);
    result.rin = getResourcesIDName(nResourceOffset, result.dirEntry.Name);
    result.bIsDataDirectory = result.dirEntry.DataIsDirectory;

    if (result.bIsDataDirectory) {
        qint64 nDirectoryOffset = nResourceOffset + result.dirEntry.OffsetToDirectory;
        result.directory = read_IMAGE_RESOURCE_DIRECTORY(nDirectoryOffset);
        nDirectoryOffset += sizeof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY);

        if ((result.directory.NumberOfIdEntries + result.directory.NumberOfNamedEntries <= 1000) && (result.directory.Characteristics == 0))  // check corrupted
        {
            result.bIsValid = true;

            if (nLevel < 3) {
                for (qint32 i = 0; i < result.directory.NumberOfIdEntries + result.directory.NumberOfNamedEntries; i++) {
                    RESOURCE_POSITION rp = _getResourcePosition(pMemoryMap, nBaseAddress, nResourceOffset, nDirectoryOffset, nLevel + 1);

                    if (!rp.bIsValid) {
                        break;
                    }

                    result.listPositions.append(rp);
                    nDirectoryOffset += sizeof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY_ENTRY);
                }
            }
        }
    } else {
        result.bIsValid = true;
        result.dataEntry = read_IMAGE_RESOURCE_DATA_ENTRY(nResourceOffset + result.dirEntry.OffsetToData);
        result.nDataAddress = nBaseAddress + result.dataEntry.OffsetToData;
        result.nDataOffset = addressToOffset(pMemoryMap, result.nDataAddress);
    }

    return result;
}

void XPE::fixCheckSum()
{
    setOptionalHeader_CheckSum(calculateCheckSum());
}

QList<XPE_DEF::IMAGE_SECTION_HEADER> XPE::splitSection(QByteArray *pbaData, XPE_DEF::IMAGE_SECTION_HEADER sectionHeaderOriginal, quint32 nBlockSize)
{
    QList<XPE_DEF::IMAGE_SECTION_HEADER> listResult;
    //    qint32 nBlockSize=0x1000;
    qint32 nSize = pbaData->size();
    char *pOffset = pbaData->data();
    char *pOffsetStart = pOffset;
    qint32 nCount = nSize / nBlockSize;
    //    XADDR nVirtualAddress=shOriginal.VirtualAddress;
    qint64 nRelVirtualStart = 0;
    qint64 nRelVirtualEnd = S_ALIGN_UP(sectionHeaderOriginal.Misc.VirtualSize, nBlockSize);
    qint64 nRelCurrent = nRelVirtualStart;

    if (nCount > 1) {
        // Check the first block
        while (isEmptyData(pOffset, nBlockSize)) {
            pOffset += nBlockSize;
            nRelCurrent += nBlockSize;
            nCount--;

            if (pOffset >= pOffsetStart + nSize) {
                break;
            }
        }

        if (pOffset != pOffsetStart) {
            XPE_DEF::IMAGE_SECTION_HEADER sectionHeader = sectionHeaderOriginal;
            //            sh.VirtualAddress=nVirtualAddress;
            //            sh.Misc.VirtualSize=pOffset-pOffsetStart;
            sectionHeader.Misc.VirtualSize = nRelCurrent - nRelVirtualStart;
            sectionHeader.SizeOfRawData = (quint32)XBinary::getPhysSize(pOffsetStart, sectionHeader.Misc.VirtualSize);
            listResult.append(sectionHeader);

            //            nVirtualAddress+=sh.Misc.VirtualSize;
        }

        bool bNew = false;
        pOffsetStart = pOffset;
        nRelVirtualStart = nRelCurrent;

        while (nCount > 0) {
            if (isEmptyData(pOffset, nBlockSize)) {
                bNew = true;
            } else {
                if (bNew) {
                    XPE_DEF::IMAGE_SECTION_HEADER sectionHeader = sectionHeaderOriginal;
                    //                    sh.VirtualAddress=nVirtualAddress;
                    //                    sh.Misc.VirtualSize=pOffset-pOffsetStart;
                    sectionHeader.Misc.VirtualSize = nRelCurrent - nRelVirtualStart;
                    sectionHeader.SizeOfRawData = (quint32)XBinary::getPhysSize(pOffsetStart, sectionHeader.Misc.VirtualSize);
                    listResult.append(sectionHeader);

                    //                    nVirtualAddress+=sh.Misc.VirtualSize;

                    pOffsetStart = pOffset;
                    nRelVirtualStart = nRelCurrent;
                    bNew = false;
                }
            }

            pOffset += nBlockSize;
            nRelCurrent += nBlockSize;
            nCount--;
        }

        if (pOffset != pOffsetStart) {
            XPE_DEF::IMAGE_SECTION_HEADER sectionHeader = sectionHeaderOriginal;
            //            sh.VirtualAddress=nVirtualAddress;
            //            sh.Misc.VirtualSize=pOffset-pOffsetStart;
            sectionHeader.Misc.VirtualSize = nRelVirtualEnd - nRelVirtualStart;
            sectionHeader.SizeOfRawData = (quint32)XBinary::getPhysSize(pOffsetStart, nSize - (pOffsetStart - pbaData->data()));

            if (sectionHeader.Misc.VirtualSize) {
                listResult.append(sectionHeader);
            }

            //            nVirtualAddress+=sh.Misc.VirtualSize;
        }
    } else {
        listResult.append(sectionHeaderOriginal);
    }

    return listResult;
}

QByteArray XPE::createHeaderStub(HEADER_OPTIONS *pHeaderOptions)  // TODO options
{
    QByteArray baResult;

    baResult.resize(0x200);  // TODO const
    baResult.fill(0);

    QBuffer buffer(&baResult);

    if (buffer.open(QIODevice::ReadWrite)) {
        XPE pe(&buffer);

        pe.set_e_magic(XMSDOS_DEF::S_IMAGE_DOS_SIGNATURE_MZ);
        pe.set_e_lfanew(0x40);
        pe.setNtHeaders_Signature(XPE_DEF::S_IMAGE_NT_SIGNATURE);
        pe.setFileHeader_SizeOfOptionalHeader(0xE0);  // TODO
        pe.setFileHeader_Machine(pHeaderOptions->nMachine);
        pe.setFileHeader_Characteristics(pHeaderOptions->nCharacteristics);
        pe.setOptionalHeader_Magic(pHeaderOptions->nMagic);
        pe.setOptionalHeader_ImageBase(pHeaderOptions->nImagebase);
        pe.setOptionalHeader_DllCharacteristics(pHeaderOptions->nDllcharacteristics);
        pe.setOptionalHeader_Subsystem(pHeaderOptions->nSubsystem);
        pe.setOptionalHeader_MajorOperatingSystemVersion(pHeaderOptions->nMajorOperationSystemVersion);
        pe.setOptionalHeader_MinorOperatingSystemVersion(pHeaderOptions->nMinorOperationSystemVersion);
        pe.setOptionalHeader_FileAlignment(pHeaderOptions->nFileAlignment);
        pe.setOptionalHeader_SectionAlignment(pHeaderOptions->nSectionAlignment);
        pe.setOptionalHeader_AddressOfEntryPoint(pHeaderOptions->nAddressOfEntryPoint);
        pe.setOptionalHeader_NumberOfRvaAndSizes(0x10);

        pe.setOptionalHeader_DataDirectory_VirtualAddress(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_RESOURCE, pHeaderOptions->nResourceRVA);
        pe.setOptionalHeader_DataDirectory_Size(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_RESOURCE, pHeaderOptions->nResourceSize);

        buffer.close();
    }

    return baResult;
}

XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32 XPE::getLoadConfigDirectory32()
{
    XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32 result = {};

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        result.Size = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, Size));
        result.TimeDateStamp = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, TimeDateStamp));
        result.MajorVersion = read_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, MajorVersion));
        result.MinorVersion = read_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, MinorVersion));
        result.GlobalFlagsClear = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GlobalFlagsClear));
        result.GlobalFlagsSet = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GlobalFlagsSet));
        result.CriticalSectionDefaultTimeout = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, CriticalSectionDefaultTimeout));
        result.DeCommitFreeBlockThreshold = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, DeCommitFreeBlockThreshold));
        result.DeCommitTotalFreeThreshold = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, DeCommitTotalFreeThreshold));
        result.LockPrefixTable = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, LockPrefixTable));
        result.MaximumAllocationSize = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, MaximumAllocationSize));
        result.VirtualMemoryThreshold = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, VirtualMemoryThreshold));
        result.ProcessAffinityMask = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, ProcessAffinityMask));
        result.CSDVersion = read_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, CSDVersion));
        result.DependentLoadFlags = read_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, DependentLoadFlags));
        result.EditList = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, EditList));
        result.SecurityCookie = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, SecurityCookie));
        result.SEHandlerTable = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, SEHandlerTable));
        result.SEHandlerCount = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, SEHandlerCount));
        // Extra
        result.GuardCFCheckFunctionPointer = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardCFCheckFunctionPointer));
        result.GuardCFDispatchFunctionPointer = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardCFDispatchFunctionPointer));
        result.GuardCFFunctionTable = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardCFFunctionTable));
        result.GuardCFFunctionCount = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardCFFunctionCount));
        result.GuardFlags = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardFlags));
        result.CodeIntegrity.Flags = read_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, CodeIntegrity.Flags));
        result.CodeIntegrity.Catalog = read_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, CodeIntegrity.Catalog));
        result.CodeIntegrity.CatalogOffset = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, CodeIntegrity.CatalogOffset));
        result.CodeIntegrity.Reserved = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, CodeIntegrity.Reserved));
        result.GuardAddressTakenIatEntryTable = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardAddressTakenIatEntryTable));
        result.GuardAddressTakenIatEntryCount = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardAddressTakenIatEntryCount));
        result.GuardLongJumpTargetTable = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardLongJumpTargetTable));
        result.GuardLongJumpTargetCount = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardLongJumpTargetCount));
        result.DynamicValueRelocTable = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, DynamicValueRelocTable));
        result.CHPEMetadataPointer = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, CHPEMetadataPointer));
        result.GuardRFFailureRoutine = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardRFFailureRoutine));
        result.GuardRFFailureRoutineFunctionPointer =
            read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardRFFailureRoutineFunctionPointer));
        result.DynamicValueRelocTableOffset = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, DynamicValueRelocTableOffset));
        result.DynamicValueRelocTableSection = read_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, DynamicValueRelocTableSection));
        result.Reserved2 = read_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, Reserved2));
        result.GuardRFVerifyStackPointerFunctionPointer =
            read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardRFVerifyStackPointerFunctionPointer));
        result.HotPatchTableOffset = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, HotPatchTableOffset));
        result.Reserved3 = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, Reserved3));
        result.EnclaveConfigurationPointer = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, EnclaveConfigurationPointer));
        result.VolatileMetadataPointer = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, VolatileMetadataPointer));
        result.GuardEHContinuationTable = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardEHContinuationTable));
        result.GuardEHContinuationCount = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardEHContinuationCount));
        result.GuardXFGCheckFunctionPointer = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardXFGCheckFunctionPointer));
        result.GuardXFGDispatchFunctionPointer = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardXFGDispatchFunctionPointer));
        result.GuardXFGTableDispatchFunctionPointer =
            read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardXFGTableDispatchFunctionPointer));
        result.CastGuardOsDeterminedFailureMode = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, CastGuardOsDeterminedFailureMode));
        result.GuardMemcpyFunctionPointer = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardMemcpyFunctionPointer));
    }

    return result;
}

XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64 XPE::getLoadConfigDirectory64()
{
    XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64 result = {};

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        result.Size = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, Size));
        result.TimeDateStamp = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, TimeDateStamp));
        result.MajorVersion = read_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, MajorVersion));
        result.MinorVersion = read_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, MinorVersion));
        result.GlobalFlagsClear = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GlobalFlagsClear));
        result.GlobalFlagsSet = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GlobalFlagsSet));
        result.CriticalSectionDefaultTimeout = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, CriticalSectionDefaultTimeout));
        result.DeCommitFreeBlockThreshold = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, DeCommitFreeBlockThreshold));
        result.DeCommitTotalFreeThreshold = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, DeCommitTotalFreeThreshold));
        result.LockPrefixTable = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, LockPrefixTable));
        result.MaximumAllocationSize = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, MaximumAllocationSize));
        result.VirtualMemoryThreshold = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, VirtualMemoryThreshold));
        result.ProcessAffinityMask = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, ProcessAffinityMask));
        result.CSDVersion = read_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, CSDVersion));
        result.DependentLoadFlags = read_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, DependentLoadFlags));
        result.EditList = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, EditList));
        result.SecurityCookie = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, SecurityCookie));
        result.SEHandlerTable = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, SEHandlerTable));
        result.SEHandlerCount = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, SEHandlerCount));
        // Extra
        result.GuardCFCheckFunctionPointer = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardCFCheckFunctionPointer));
        result.GuardCFDispatchFunctionPointer = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardCFDispatchFunctionPointer));
        result.GuardCFFunctionTable = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardCFFunctionTable));
        result.GuardCFFunctionCount = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardCFFunctionCount));
        result.GuardFlags = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardFlags));
        result.CodeIntegrity.Flags = read_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, CodeIntegrity.Flags));
        result.CodeIntegrity.Catalog = read_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, CodeIntegrity.Catalog));
        result.CodeIntegrity.CatalogOffset = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, CodeIntegrity.CatalogOffset));
        result.CodeIntegrity.Reserved = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, CodeIntegrity.Reserved));
        result.GuardAddressTakenIatEntryTable = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardAddressTakenIatEntryTable));
        result.GuardAddressTakenIatEntryCount = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardAddressTakenIatEntryCount));
        result.GuardLongJumpTargetTable = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardLongJumpTargetTable));
        result.GuardLongJumpTargetCount = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardLongJumpTargetCount));
        result.DynamicValueRelocTable = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, DynamicValueRelocTable));
        result.CHPEMetadataPointer = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, CHPEMetadataPointer));
        result.GuardRFFailureRoutine = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardRFFailureRoutine));
        result.GuardRFFailureRoutineFunctionPointer =
            read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardRFFailureRoutineFunctionPointer));
        result.DynamicValueRelocTableOffset = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, DynamicValueRelocTableOffset));
        result.DynamicValueRelocTableSection = read_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, DynamicValueRelocTableSection));
        result.Reserved2 = read_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, Reserved2));
        result.GuardRFVerifyStackPointerFunctionPointer =
            read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardRFVerifyStackPointerFunctionPointer));
        result.HotPatchTableOffset = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, HotPatchTableOffset));
        result.Reserved3 = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, Reserved3));
        result.EnclaveConfigurationPointer = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, EnclaveConfigurationPointer));
        result.VolatileMetadataPointer = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, VolatileMetadataPointer));
        result.GuardEHContinuationTable = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardEHContinuationTable));
        result.GuardEHContinuationCount = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardEHContinuationCount));
        result.GuardXFGCheckFunctionPointer = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardXFGCheckFunctionPointer));
        result.GuardXFGDispatchFunctionPointer = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardXFGDispatchFunctionPointer));
        result.GuardXFGTableDispatchFunctionPointer =
            read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardXFGTableDispatchFunctionPointer));
        result.CastGuardOsDeterminedFailureMode = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, CastGuardOsDeterminedFailureMode));
        result.GuardMemcpyFunctionPointer = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardMemcpyFunctionPointer));
    }

    return result;
}

qint64 XPE::getLoadConfigDirectoryOffset()
{
    return getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);
}

qint64 XPE::getLoadConfigDirectorySize()
{
    return getLoadConfig_Size();
}

quint32 XPE::getLoadConfig_Size()
{
    quint32 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, Size));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, Size));
        }
    }

    return nResult;
}

quint32 XPE::getLoadConfig_TimeDateStamp()
{
    quint32 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, TimeDateStamp));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, TimeDateStamp));
        }
    }

    return nResult;
}

quint16 XPE::getLoadConfig_MajorVersion()
{
    quint16 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, MajorVersion));
        } else {
            nResult = read_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, MajorVersion));
        }
    }

    return nResult;
}

quint16 XPE::getLoadConfig_MinorVersion()
{
    quint16 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, MinorVersion));
        } else {
            nResult = read_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, MinorVersion));
        }
    }

    return nResult;
}

quint32 XPE::getLoadConfig_GlobalFlagsClear()
{
    quint32 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GlobalFlagsClear));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GlobalFlagsClear));
        }
    }

    return nResult;
}

quint32 XPE::getLoadConfig_GlobalFlagsSet()
{
    quint32 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GlobalFlagsSet));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GlobalFlagsSet));
        }
    }

    return nResult;
}

quint32 XPE::getLoadConfig_CriticalSectionDefaultTimeout()
{
    quint32 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, CriticalSectionDefaultTimeout));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, CriticalSectionDefaultTimeout));
        }
    }

    return nResult;
}

quint64 XPE::getLoadConfig_DeCommitFreeBlockThreshold()
{
    quint64 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, DeCommitFreeBlockThreshold));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, DeCommitFreeBlockThreshold));
        }
    }

    return nResult;
}

quint64 XPE::getLoadConfig_DeCommitTotalFreeThreshold()
{
    quint64 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, DeCommitTotalFreeThreshold));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, DeCommitTotalFreeThreshold));
        }
    }

    return nResult;
}

quint64 XPE::getLoadConfig_LockPrefixTable()
{
    quint64 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, LockPrefixTable));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, LockPrefixTable));
        }
    }

    return nResult;
}

quint64 XPE::getLoadConfig_MaximumAllocationSize()
{
    quint64 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, MaximumAllocationSize));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, MaximumAllocationSize));
        }
    }

    return nResult;
}

quint64 XPE::getLoadConfig_VirtualMemoryThreshold()
{
    quint64 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, VirtualMemoryThreshold));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, VirtualMemoryThreshold));
        }
    }

    return nResult;
}

quint64 XPE::getLoadConfig_ProcessAffinityMask()
{
    quint64 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, ProcessAffinityMask));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, ProcessAffinityMask));
        }
    }

    return nResult;
}

quint16 XPE::getLoadConfig_CSDVersion()
{
    quint16 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, CSDVersion));
        } else {
            nResult = read_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, CSDVersion));
        }
    }

    return nResult;
}

quint16 XPE::getLoadConfig_DependentLoadFlags()
{
    quint16 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, DependentLoadFlags));
        } else {
            nResult = read_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, DependentLoadFlags));
        }
    }

    return nResult;
}

quint64 XPE::getLoadConfig_EditList()
{
    quint64 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, EditList));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, EditList));
        }
    }

    return nResult;
}

quint64 XPE::getLoadConfig_SecurityCookie()
{
    quint64 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, SecurityCookie));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, SecurityCookie));
        }
    }

    return nResult;
}

quint64 XPE::getLoadConfig_SEHandlerTable()
{
    quint64 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, SEHandlerTable));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, SEHandlerTable));
        }
    }

    return nResult;
}

quint64 XPE::getLoadConfig_SEHandlerCount()
{
    quint64 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, SEHandlerCount));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, SEHandlerCount));
        }
    }

    return nResult;
}

quint64 XPE::getLoadConfig_GuardCFCheckFunctionPointer()
{
    quint64 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardCFCheckFunctionPointer));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardCFCheckFunctionPointer));
        }
    }

    return nResult;
}

quint64 XPE::getLoadConfig_GuardCFDispatchFunctionPointer()
{
    quint64 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardCFDispatchFunctionPointer));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardCFDispatchFunctionPointer));
        }
    }

    return nResult;
}

quint64 XPE::getLoadConfig_GuardCFFunctionTable()
{
    quint64 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardCFFunctionTable));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardCFFunctionTable));
        }
    }

    return nResult;
}

quint64 XPE::getLoadConfig_GuardCFFunctionCount()
{
    quint64 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardCFFunctionCount));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardCFFunctionCount));
        }
    }

    return nResult;
}

quint32 XPE::getLoadConfig_GuardFlags()
{
    quint32 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardFlags));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardFlags));
        }
    }

    return nResult;
}

quint16 XPE::getLoadConfig_CodeIntegrity_Flags()
{
    quint16 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, CodeIntegrity.Flags));
        } else {
            nResult = read_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, CodeIntegrity.Flags));
        }
    }

    return nResult;
}

quint16 XPE::getLoadConfig_CodeIntegrity_Catalog()
{
    quint16 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, CodeIntegrity.Catalog));
        } else {
            nResult = read_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, CodeIntegrity.Catalog));
        }
    }

    return nResult;
}

quint32 XPE::getLoadConfig_CodeIntegrity_CatalogOffset()
{
    quint32 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, CodeIntegrity.CatalogOffset));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, CodeIntegrity.CatalogOffset));
        }
    }

    return nResult;
}

quint32 XPE::getLoadConfig_CodeIntegrity_Reserved()
{
    quint32 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, CodeIntegrity.Reserved));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, CodeIntegrity.Reserved));
        }
    }

    return nResult;
}

quint64 XPE::getLoadConfig_GuardAddressTakenIatEntryTable()
{
    quint64 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardAddressTakenIatEntryTable));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardAddressTakenIatEntryTable));
        }
    }

    return nResult;
}

quint64 XPE::getLoadConfig_GuardAddressTakenIatEntryCount()
{
    quint64 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardAddressTakenIatEntryCount));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardAddressTakenIatEntryCount));
        }
    }

    return nResult;
}

quint64 XPE::getLoadConfig_GuardLongJumpTargetTable()
{
    quint64 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardLongJumpTargetTable));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardLongJumpTargetTable));
        }
    }

    return nResult;
}

quint64 XPE::getLoadConfig_GuardLongJumpTargetCount()
{
    quint64 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardLongJumpTargetCount));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardLongJumpTargetCount));
        }
    }

    return nResult;
}

quint64 XPE::getLoadConfig_DynamicValueRelocTable()
{
    quint64 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, DynamicValueRelocTable));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, DynamicValueRelocTable));
        }
    }

    return nResult;
}

quint64 XPE::getLoadConfig_CHPEMetadataPointer()
{
    quint64 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, CHPEMetadataPointer));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, CHPEMetadataPointer));
        }
    }

    return nResult;
}

quint64 XPE::getLoadConfig_GuardRFFailureRoutine()
{
    quint64 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardRFFailureRoutine));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardRFFailureRoutine));
        }
    }

    return nResult;
}

quint64 XPE::getLoadConfig_GuardRFFailureRoutineFunctionPointer()
{
    quint64 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardRFFailureRoutineFunctionPointer));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardRFFailureRoutineFunctionPointer));
        }
    }

    return nResult;
}

quint32 XPE::getLoadConfig_DynamicValueRelocTableOffset()
{
    quint32 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, DynamicValueRelocTableOffset));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, DynamicValueRelocTableOffset));
        }
    }

    return nResult;
}

quint16 XPE::getLoadConfig_DynamicValueRelocTableSection()
{
    quint16 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, DynamicValueRelocTableSection));
        } else {
            nResult = read_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, DynamicValueRelocTableSection));
        }
    }

    return nResult;
}

quint16 XPE::getLoadConfig_Reserved2()
{
    quint16 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, Reserved2));
        } else {
            nResult = read_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, Reserved2));
        }
    }

    return nResult;
}

quint64 XPE::getLoadConfig_GuardRFVerifyStackPointerFunctionPointer()
{
    quint64 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardRFVerifyStackPointerFunctionPointer));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardRFVerifyStackPointerFunctionPointer));
        }
    }

    return nResult;
}

quint32 XPE::getLoadConfig_HotPatchTableOffset()
{
    quint32 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, HotPatchTableOffset));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, HotPatchTableOffset));
        }
    }

    return nResult;
}

quint32 XPE::getLoadConfig_Reserved3()
{
    quint32 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, Reserved3));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, Reserved3));
        }
    }

    return nResult;
}

quint64 XPE::getLoadConfig_EnclaveConfigurationPointer()
{
    quint64 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, EnclaveConfigurationPointer));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, EnclaveConfigurationPointer));
        }
    }

    return nResult;
}

quint64 XPE::getLoadConfig_VolatileMetadataPointer()
{
    quint64 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, VolatileMetadataPointer));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, VolatileMetadataPointer));
        }
    }

    return nResult;
}

quint64 XPE::getLoadConfig_GuardEHContinuationTable()
{
    quint64 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardEHContinuationTable));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardEHContinuationTable));
        }
    }

    return nResult;
}

quint64 XPE::getLoadConfig_GuardEHContinuationCount()
{
    quint64 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardEHContinuationCount));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardEHContinuationCount));
        }
    }

    return nResult;
}

quint64 XPE::getLoadConfig_GuardXFGCheckFunctionPointer()
{
    quint64 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardXFGCheckFunctionPointer));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardXFGCheckFunctionPointer));
        }
    }

    return nResult;
}

quint64 XPE::getLoadConfig_GuardXFGDispatchFunctionPointer()
{
    quint64 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardXFGDispatchFunctionPointer));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardXFGDispatchFunctionPointer));
        }
    }

    return nResult;
}

quint64 XPE::getLoadConfig_GuardXFGTableDispatchFunctionPointer()
{
    quint64 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardXFGTableDispatchFunctionPointer));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardXFGTableDispatchFunctionPointer));
        }
    }

    return nResult;
}

quint64 XPE::getLoadConfig_CastGuardOsDeterminedFailureMode()
{
    quint64 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, CastGuardOsDeterminedFailureMode));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, CastGuardOsDeterminedFailureMode));
        }
    }

    return nResult;
}

quint64 XPE::getLoadConfig_GuardMemcpyFunctionPointer()
{
    quint64 nResult = 0;

    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardMemcpyFunctionPointer));
        } else {
            nResult = read_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardMemcpyFunctionPointer));
        }
    }

    return nResult;
}

void XPE::setLoadConfig_Size(quint32 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, Size), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, Size), nValue);
        }
    }
}

void XPE::setLoadConfig_TimeDateStamp(quint32 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, TimeDateStamp), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, TimeDateStamp), nValue);
        }
    }
}

void XPE::setLoadConfig_MajorVersion(quint16 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, MajorVersion), nValue);
        } else {
            write_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, MajorVersion), nValue);
        }
    }
}

void XPE::setLoadConfig_MinorVersion(quint16 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, MinorVersion), nValue);
        } else {
            write_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, MinorVersion), nValue);
        }
    }
}

void XPE::setLoadConfig_GlobalFlagsClear(quint32 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GlobalFlagsClear), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GlobalFlagsClear), nValue);
        }
    }
}

void XPE::setLoadConfig_GlobalFlagsSet(quint32 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GlobalFlagsSet), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GlobalFlagsSet), nValue);
        }
    }
}

void XPE::setLoadConfig_CriticalSectionDefaultTimeout(quint32 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, CriticalSectionDefaultTimeout), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, CriticalSectionDefaultTimeout), nValue);
        }
    }
}

void XPE::setLoadConfig_DeCommitFreeBlockThreshold(quint64 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, DeCommitFreeBlockThreshold), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, DeCommitFreeBlockThreshold), nValue);
        }
    }
}

void XPE::setLoadConfig_DeCommitTotalFreeThreshold(quint64 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, DeCommitTotalFreeThreshold), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, DeCommitTotalFreeThreshold), nValue);
        }
    }
}

void XPE::setLoadConfig_LockPrefixTable(quint64 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, LockPrefixTable), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, LockPrefixTable), nValue);
        }
    }
}

void XPE::setLoadConfig_MaximumAllocationSize(quint64 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, MaximumAllocationSize), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, MaximumAllocationSize), nValue);
        }
    }
}

void XPE::setLoadConfig_VirtualMemoryThreshold(quint64 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, VirtualMemoryThreshold), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, VirtualMemoryThreshold), nValue);
        }
    }
}

void XPE::setLoadConfig_ProcessAffinityMask(quint64 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, ProcessAffinityMask), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, ProcessAffinityMask), nValue);
        }
    }
}

void XPE::setLoadConfig_CSDVersion(quint16 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, CSDVersion), nValue);
        } else {
            write_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, CSDVersion), nValue);
        }
    }
}

void XPE::setLoadConfig_DependentLoadFlags(quint16 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, DependentLoadFlags), nValue);
        } else {
            write_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, DependentLoadFlags), nValue);
        }
    }
}

void XPE::setLoadConfig_EditList(quint64 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, EditList), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, EditList), nValue);
        }
    }
}

void XPE::setLoadConfig_SecurityCookie(quint64 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, SecurityCookie), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, SecurityCookie), nValue);
        }
    }
}

void XPE::setLoadConfig_SEHandlerTable(quint64 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, SEHandlerTable), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, SEHandlerTable), nValue);
        }
    }
}

void XPE::setLoadConfig_SEHandlerCount(quint64 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, SEHandlerCount), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, SEHandlerCount), nValue);
        }
    }
}

void XPE::setLoadConfig_GuardCFCheckFunctionPointer(quint64 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardCFCheckFunctionPointer), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardCFCheckFunctionPointer), nValue);
        }
    }
}

void XPE::setLoadConfig_GuardCFDispatchFunctionPointer(quint64 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardCFDispatchFunctionPointer), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardCFDispatchFunctionPointer), nValue);
        }
    }
}

void XPE::setLoadConfig_GuardCFFunctionTable(quint64 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardCFFunctionTable), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardCFFunctionTable), nValue);
        }
    }
}

void XPE::setLoadConfig_GuardCFFunctionCount(quint64 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardCFFunctionCount), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardCFFunctionCount), nValue);
        }
    }
}

void XPE::setLoadConfig_GuardFlags(quint32 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardFlags), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardFlags), nValue);
        }
    }
}

void XPE::setLoadConfig_CodeIntegrity_Flags(quint16 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, CodeIntegrity.Flags), nValue);
        } else {
            write_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, CodeIntegrity.Flags), nValue);
        }
    }
}

void XPE::setLoadConfig_CodeIntegrity_Catalog(quint16 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, CodeIntegrity.Catalog), nValue);
        } else {
            write_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, CodeIntegrity.Catalog), nValue);
        }
    }
}

void XPE::setLoadConfig_CodeIntegrity_CatalogOffset(quint32 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, CodeIntegrity.CatalogOffset), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, CodeIntegrity.CatalogOffset), nValue);
        }
    }
}

void XPE::setLoadConfig_CodeIntegrity_Reserved(quint32 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, CodeIntegrity.Reserved), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, CodeIntegrity.Reserved), nValue);
        }
    }
}

void XPE::setLoadConfig_GuardAddressTakenIatEntryTable(quint64 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardAddressTakenIatEntryTable), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardAddressTakenIatEntryTable), nValue);
        }
    }
}

void XPE::setLoadConfig_GuardAddressTakenIatEntryCount(quint64 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardAddressTakenIatEntryCount), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardAddressTakenIatEntryCount), nValue);
        }
    }
}

void XPE::setLoadConfig_GuardLongJumpTargetTable(quint64 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardLongJumpTargetTable), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardLongJumpTargetTable), nValue);
        }
    }
}

void XPE::setLoadConfig_GuardLongJumpTargetCount(quint64 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardLongJumpTargetTable), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardLongJumpTargetTable), nValue);
        }
    }
}

void XPE::setLoadConfig_DynamicValueRelocTable(quint64 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, DynamicValueRelocTable), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, DynamicValueRelocTable), nValue);
        }
    }
}

void XPE::setLoadConfig_CHPEMetadataPointer(quint64 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, CHPEMetadataPointer), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, CHPEMetadataPointer), nValue);
        }
    }
}

void XPE::setLoadConfig_GuardRFFailureRoutine(quint64 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardRFFailureRoutine), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardRFFailureRoutine), nValue);
        }
    }
}

void XPE::setLoadConfig_GuardRFFailureRoutineFunctionPointer(quint64 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardRFFailureRoutineFunctionPointer), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardRFFailureRoutineFunctionPointer), nValue);
        }
    }
}

void XPE::setLoadConfig_DynamicValueRelocTableOffset(quint32 nValue)
{
    qint32 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, DynamicValueRelocTableOffset), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, DynamicValueRelocTableOffset), nValue);
        }
    }
}

void XPE::setLoadConfig_DynamicValueRelocTableSection(quint16 nValue)
{
    qint32 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, DynamicValueRelocTableSection), nValue);
        } else {
            write_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, DynamicValueRelocTableSection), nValue);
        }
    }
}

void XPE::setLoadConfig_Reserved2(quint16 nValue)
{
    qint32 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, Reserved2), nValue);
        } else {
            write_uint16(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, Reserved2), nValue);
        }
    }
}

void XPE::setLoadConfig_GuardRFVerifyStackPointerFunctionPointer(quint64 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardRFVerifyStackPointerFunctionPointer), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardRFVerifyStackPointerFunctionPointer), nValue);
        }
    }
}

void XPE::setLoadConfig_HotPatchTableOffset(quint32 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, HotPatchTableOffset), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, HotPatchTableOffset), nValue);
        }
    }
}

void XPE::setLoadConfig_Reserved3(quint32 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, Reserved3), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, Reserved3), nValue);
        }
    }
}

void XPE::setLoadConfig_EnclaveConfigurationPointer(quint64 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, EnclaveConfigurationPointer), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, EnclaveConfigurationPointer), nValue);
        }
    }
}

void XPE::setLoadConfig_VolatileMetadataPointer(quint64 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, VolatileMetadataPointer), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, VolatileMetadataPointer), nValue);
        }
    }
}

void XPE::setLoadConfig_GuardEHContinuationTable(quint64 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardEHContinuationTable), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardEHContinuationTable), nValue);
        }
    }
}

void XPE::setLoadConfig_GuardEHContinuationCount(quint64 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardEHContinuationCount), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardEHContinuationCount), nValue);
        }
    }
}

void XPE::setLoadConfig_GuardXFGCheckFunctionPointer(quint64 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardXFGCheckFunctionPointer), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardXFGCheckFunctionPointer), nValue);
        }
    }
}

void XPE::setLoadConfig_GuardXFGDispatchFunctionPointer(quint64 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardXFGDispatchFunctionPointer), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardXFGDispatchFunctionPointer), nValue);
        }
    }
}

void XPE::setLoadConfig_GuardXFGTableDispatchFunctionPointer(quint64 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardXFGTableDispatchFunctionPointer), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardXFGTableDispatchFunctionPointer), nValue);
        }
    }
}

void XPE::setLoadConfig_CastGuardOsDeterminedFailureMode(quint64 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, CastGuardOsDeterminedFailureMode), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, CastGuardOsDeterminedFailureMode), nValue);
        }
    }
}

void XPE::setLoadConfig_GuardMemcpyFunctionPointer(quint64 nValue)
{
    qint64 nLoadConfigOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);

    if (nLoadConfigOffset != -1) {
        if (is64()) {
            write_uint64(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardMemcpyFunctionPointer), nValue);
        } else {
            write_uint32(nLoadConfigOffset + offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardMemcpyFunctionPointer), nValue);
        }
    }
}

XPE_DEF::S_IMAGE_RUNTIME_FUNCTION_ENTRY XPE::_read_IMAGE_RUNTIME_FUNCTION_ENTRY(qint64 nOffset)
{
    XPE_DEF::S_IMAGE_RUNTIME_FUNCTION_ENTRY result = {};

    result.BeginAddress = read_uint32(nOffset + offsetof(XPE_DEF::S_IMAGE_RUNTIME_FUNCTION_ENTRY, BeginAddress));
    result.EndAddress = read_uint32(nOffset + offsetof(XPE_DEF::S_IMAGE_RUNTIME_FUNCTION_ENTRY, EndAddress));
    result.UnwindInfoAddress = read_uint32(nOffset + offsetof(XPE_DEF::S_IMAGE_RUNTIME_FUNCTION_ENTRY, UnwindInfoAddress));

    return result;
}

XPE_DEF::S_IMAGE_RUNTIME_FUNCTION_ENTRY XPE::_getException(qint32 nNumber)
{
    XPE_DEF::S_IMAGE_RUNTIME_FUNCTION_ENTRY result = {};

    qint64 nExceptionOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXCEPTION);

    if (nExceptionOffset != -1) {
        result = _read_IMAGE_RUNTIME_FUNCTION_ENTRY(nExceptionOffset + nNumber * sizeof(XPE_DEF::S_IMAGE_RUNTIME_FUNCTION_ENTRY));
    }

    return result;
}

void XPE::setException_BeginAddress(qint32 nNumber, quint32 nValue)
{
    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXCEPTION + nNumber * sizeof(XPE_DEF::S_IMAGE_RUNTIME_FUNCTION_ENTRY));

    write_uint32(nOffset + offsetof(XPE_DEF::S_IMAGE_RUNTIME_FUNCTION_ENTRY, BeginAddress), nValue);
}

void XPE::setException_EndAddress(qint32 nNumber, quint32 nValue)
{
    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXCEPTION + nNumber * sizeof(XPE_DEF::S_IMAGE_RUNTIME_FUNCTION_ENTRY));

    write_uint32(nOffset + offsetof(XPE_DEF::S_IMAGE_RUNTIME_FUNCTION_ENTRY, EndAddress), nValue);
}

void XPE::setException_UnwindInfoAddress(qint32 nNumber, quint32 nValue)
{
    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXCEPTION + nNumber * sizeof(XPE_DEF::S_IMAGE_RUNTIME_FUNCTION_ENTRY));

    write_uint32(nOffset + offsetof(XPE_DEF::S_IMAGE_RUNTIME_FUNCTION_ENTRY, UnwindInfoAddress), nValue);
}

qint64 XPE::getExceptionRecordOffset(qint32 nNumber)
{
    qint64 nResult = -1;

    qint64 nExceptionOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXCEPTION);

    if (nExceptionOffset != -1) {
        nResult = nExceptionOffset + nNumber * sizeof(XPE_DEF::S_IMAGE_RUNTIME_FUNCTION_ENTRY);
    }

    return nResult;
}

qint64 XPE::getExceptionRecordSize()
{
    return sizeof(XPE_DEF::S_IMAGE_RUNTIME_FUNCTION_ENTRY);
}

QList<XPE_DEF::S_IMAGE_RUNTIME_FUNCTION_ENTRY> XPE::getExceptionsList()
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return getExceptionsList(&memoryMap);
}

QList<XPE_DEF::S_IMAGE_RUNTIME_FUNCTION_ENTRY> XPE::getExceptionsList(XBinary::_MEMORY_MAP *pMemoryMap)
{
    QList<XPE_DEF::S_IMAGE_RUNTIME_FUNCTION_ENTRY> listResult;

    qint64 nExceptionOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXCEPTION);

    if (nExceptionOffset != -1) {
        while (true) {
            XPE_DEF::S_IMAGE_RUNTIME_FUNCTION_ENTRY record = _read_IMAGE_RUNTIME_FUNCTION_ENTRY(nExceptionOffset);

            if (record.BeginAddress && record.EndAddress && isAddressValid(pMemoryMap, pMemoryMap->nModuleAddress + record.BeginAddress) &&
                isAddressValid(pMemoryMap, pMemoryMap->nModuleAddress + record.EndAddress)) {
                listResult.append(record);
            } else {
                break;
            }

            nExceptionOffset += sizeof(XPE_DEF::S_IMAGE_RUNTIME_FUNCTION_ENTRY);
        }
    }

    return listResult;
}

XPE_DEF::S_IMAGE_DEBUG_DIRECTORY XPE::_read_IMAGE_DEBUG_DIRECTORY(qint64 nOffset)
{
    XPE_DEF::S_IMAGE_DEBUG_DIRECTORY result = {};

    result.Characteristics = read_uint32(nOffset + offsetof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY, Characteristics));
    result.TimeDateStamp = read_uint32(nOffset + offsetof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY, TimeDateStamp));
    result.MajorVersion = read_uint16(nOffset + offsetof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY, MajorVersion));
    result.MinorVersion = read_uint16(nOffset + offsetof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY, MinorVersion));
    result.Type = read_uint32(nOffset + offsetof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY, Type));
    result.SizeOfData = read_uint32(nOffset + offsetof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY, SizeOfData));
    result.AddressOfRawData = read_uint32(nOffset + offsetof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY, AddressOfRawData));
    result.PointerToRawData = read_uint32(nOffset + offsetof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY, PointerToRawData));

    return result;
}

QList<XPE_DEF::S_IMAGE_DEBUG_DIRECTORY> XPE::getDebugList(PDSTRUCT *pPdStruct)
{
    _MEMORY_MAP memoryMap = getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);

    return getDebugList(&memoryMap, pPdStruct);
}

QList<XPE_DEF::S_IMAGE_DEBUG_DIRECTORY> XPE::getDebugList(XBinary::_MEMORY_MAP *pMemoryMap, PDSTRUCT *pPdStruct)
{
    QList<XPE_DEF::S_IMAGE_DEBUG_DIRECTORY> listResult;

    XPE_DEF::IMAGE_DATA_DIRECTORY dataResources = getOptionalHeader_DataDirectory(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_DEBUG);

    if (dataResources.VirtualAddress) {
        qint64 nDebugOffset = addressToOffset(pMemoryMap, dataResources.VirtualAddress + pMemoryMap->nModuleAddress);
        qint64 nCurrent = 0;

        if (nDebugOffset != -1) {
            while ((isPdStructNotCanceled(pPdStruct)) && (nCurrent < dataResources.Size)) {
                XPE_DEF::S_IMAGE_DEBUG_DIRECTORY record = _read_IMAGE_DEBUG_DIRECTORY(nDebugOffset);

                if (record.PointerToRawData && isOffsetValid(pMemoryMap, record.PointerToRawData)) {
                    listResult.append(record);
                } else {
                    break;
                }

                nDebugOffset += sizeof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY);
                nCurrent += sizeof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY);
            }
        }
    }

    return listResult;
}

qint64 XPE::getDebugHeaderOffset(quint32 nNumber)
{
    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_DEBUG);

    nOffset += sizeof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY) * nNumber;

    return nOffset;
}

qint64 XPE::getDebugHeaderSize()
{
    return sizeof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY);
}

XPE_DEF::S_IMAGE_DEBUG_DIRECTORY XPE::getDebugHeader(quint32 nNumber)
{
    XPE_DEF::S_IMAGE_DEBUG_DIRECTORY result = {};

    // TODO Check number of headers

    qint64 nDebugOffset = getDebugHeaderOffset(nNumber);

    result = _read_IMAGE_DEBUG_DIRECTORY(nDebugOffset);

    return result;
}

void XPE::setDebugHeader_Characteristics(quint32 nNumber, quint32 nValue)
{
    qint64 nOffset = getDebugHeaderOffset(nNumber);

    write_uint32(nOffset + offsetof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY, Characteristics), nValue);
}

void XPE::setDebugHeader_TimeDateStamp(quint32 nNumber, quint32 nValue)
{
    qint64 nOffset = getDebugHeaderOffset(nNumber);

    write_uint32(nOffset + offsetof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY, TimeDateStamp), nValue);
}

void XPE::setDebugHeader_MajorVersion(quint32 nNumber, quint16 nValue)
{
    qint64 nOffset = getDebugHeaderOffset(nNumber);

    write_uint16(nOffset + offsetof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY, MajorVersion), nValue);
}

void XPE::setDebugHeader_MinorVersion(quint32 nNumber, quint16 nValue)
{
    qint64 nOffset = getDebugHeaderOffset(nNumber);

    write_uint16(nOffset + offsetof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY, MinorVersion), nValue);
}

void XPE::setDebugHeader_Type(quint32 nNumber, quint32 nValue)
{
    qint64 nOffset = getDebugHeaderOffset(nNumber);

    write_uint32(nOffset + offsetof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY, Type), nValue);
}

void XPE::setDebugHeader_SizeOfData(quint32 nNumber, quint32 nValue)
{
    qint64 nOffset = getDebugHeaderOffset(nNumber);

    write_uint32(nOffset + offsetof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY, SizeOfData), nValue);
}

void XPE::setDebugHeader_AddressOfRawData(quint32 nNumber, quint32 nValue)
{
    qint64 nOffset = getDebugHeaderOffset(nNumber);

    write_uint32(nOffset + offsetof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY, AddressOfRawData), nValue);
}

void XPE::setDebugHeader_PointerToRawData(quint32 nNumber, quint32 nValue)
{
    qint64 nOffset = getDebugHeaderOffset(nNumber);

    write_uint32(nOffset + offsetof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY, PointerToRawData), nValue);
}

qint64 XPE::getDelayImportRecordOffset(qint32 nNumber)
{
    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT);

    nOffset += sizeof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR) * nNumber;

    return nOffset;
}

qint64 XPE::getDelayImportRecordSize()
{
    return sizeof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR);
}

QList<XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR> XPE::getDelayImportsList()
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return getDelayImportsList(&memoryMap);
}

QList<XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR> XPE::getDelayImportsList(XBinary::_MEMORY_MAP *pMemoryMap)
{
    QList<XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR> listResult;

    qint64 nDelayImportOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT);

    if (nDelayImportOffset != -1) {
        while (true) {
            XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR record = _read_IMAGE_DELAYLOAD_DESCRIPTOR(nDelayImportOffset);

            if (record.DllNameRVA && isAddressValid(pMemoryMap, pMemoryMap->nModuleAddress + record.DllNameRVA)) {
                listResult.append(record);
            } else {
                break;
            }

            nDelayImportOffset += sizeof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR);
        }
    }

    return listResult;
}

void XPE::setDelayImport_AllAttributes(quint32 nNumber, quint32 nValue)
{
    qint64 nOffset = getDelayImportRecordOffset(nNumber);

    write_uint32(nOffset + offsetof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR, AllAttributes), nValue);
}

void XPE::setDelayImport_DllNameRVA(quint32 nNumber, quint32 nValue)
{
    qint64 nOffset = getDelayImportRecordOffset(nNumber);

    write_uint32(nOffset + offsetof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR, DllNameRVA), nValue);
}

void XPE::setDelayImport_ModuleHandleRVA(quint32 nNumber, quint32 nValue)
{
    qint64 nOffset = getDelayImportRecordOffset(nNumber);

    write_uint32(nOffset + offsetof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR, ModuleHandleRVA), nValue);
}

void XPE::setDelayImport_ImportAddressTableRVA(quint32 nNumber, quint32 nValue)
{
    qint64 nOffset = getDelayImportRecordOffset(nNumber);

    write_uint32(nOffset + offsetof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR, ImportAddressTableRVA), nValue);
}

void XPE::setDelayImport_ImportNameTableRVA(quint32 nNumber, quint32 nValue)
{
    qint64 nOffset = getDelayImportRecordOffset(nNumber);

    write_uint32(nOffset + offsetof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR, ImportNameTableRVA), nValue);
}

void XPE::setDelayImport_BoundImportAddressTableRVA(quint32 nNumber, quint32 nValue)
{
    qint64 nOffset = getDelayImportRecordOffset(nNumber);

    write_uint32(nOffset + offsetof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR, BoundImportAddressTableRVA), nValue);
}

void XPE::setDelayImport_UnloadInformationTableRVA(quint32 nNumber, quint32 nValue)
{
    qint64 nOffset = getDelayImportRecordOffset(nNumber);

    write_uint32(nOffset + offsetof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR, UnloadInformationTableRVA), nValue);
}

void XPE::setDelayImport_TimeDateStamp(quint32 nNumber, quint32 nValue)
{
    qint64 nOffset = getDelayImportRecordOffset(nNumber);

    write_uint32(nOffset + offsetof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR, TimeDateStamp), nValue);
}

QList<XPE::DELAYIMPORT_POSITION> XPE::getDelayImportPositions(qint32 nIndex)
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return getDelayImportPositions(&memoryMap, nIndex);
}

QList<XPE::DELAYIMPORT_POSITION> XPE::getDelayImportPositions(XBinary::_MEMORY_MAP *pMemoryMap, qint32 nIndex)
{
    QList<DELAYIMPORT_POSITION> listResult;

    qint64 nDelayImportOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT);

    if (nDelayImportOffset != -1) {
        nDelayImportOffset += sizeof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR) * nIndex;

        XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR idd = _read_IMAGE_DELAYLOAD_DESCRIPTOR(nDelayImportOffset);

        qint64 nNameThunksRVA = idd.ImportNameTableRVA;
        qint64 nAddressThunksRVA = idd.ImportAddressTableRVA;
        qint64 nBoundThunksRVA = idd.BoundImportAddressTableRVA;

        qint64 nNameThunksOffset = XBinary::relAddressToOffset(pMemoryMap, nNameThunksRVA);
        qint64 nAddressThunksOffset = XBinary::relAddressToOffset(pMemoryMap, nAddressThunksRVA);
        qint64 nBoundThunksOffset = XBinary::relAddressToOffset(pMemoryMap, nBoundThunksRVA);

        bool bIs64 = is64(pMemoryMap);

        while (true) {
            DELAYIMPORT_POSITION importPosition = {};
            importPosition.nNameThunkOffset = nNameThunksOffset;
            importPosition.nNameThunkRVA = nNameThunksRVA;
            importPosition.nAddressThunkOffset = nNameThunksOffset;
            importPosition.nAddressThunkRVA = nAddressThunksOffset;
            importPosition.nBoundThunkOffset = nBoundThunksOffset;
            importPosition.nBoundThunkRVA = nBoundThunksRVA;

            if (bIs64) {
                importPosition.nNameThunkValue = read_uint64(nNameThunksOffset);
                importPosition.nAddressThunkValue = read_uint64(nAddressThunksOffset);
                importPosition.nBoundThunkValue = read_uint64(nBoundThunksOffset);

                if (importPosition.nNameThunkValue == 0) {
                    break;
                }

                // mb TODO check importPosition.nAddressThunkValue

                if (!(importPosition.nNameThunkValue & 0x8000000000000000)) {
                    qint64 nOffset = addressToOffset(pMemoryMap, importPosition.nNameThunkValue + pMemoryMap->nModuleAddress);

                    if (nOffset != -1) {
                        importPosition.nHint = read_uint16(nOffset);
                        importPosition.sName = read_ansiString(nOffset + 2);

                        if (importPosition.sName == "") {
                            break;
                        }
                    } else {
                        break;
                    }
                } else {
                    importPosition.nOrdinal = importPosition.nNameThunkValue & 0x7FFFFFFFFFFFFFFF;
                }
            } else {
                importPosition.nNameThunkValue = read_uint32(nNameThunksOffset);
                importPosition.nAddressThunkValue = read_uint32(nAddressThunksOffset);
                importPosition.nBoundThunkValue = read_uint32(nBoundThunksOffset);

                if (importPosition.nNameThunkValue == 0) {
                    break;
                }

                if (!(importPosition.nNameThunkValue & 0x80000000)) {
                    qint64 nOffset = addressToOffset(pMemoryMap, importPosition.nNameThunkValue + pMemoryMap->nModuleAddress);

                    if (nOffset != -1) {
                        importPosition.nHint = read_uint16(nOffset);
                        importPosition.sName = read_ansiString(nOffset + 2);

                        if (importPosition.sName == "") {
                            break;
                        }
                    } else {
                        break;
                    }
                } else {
                    importPosition.nOrdinal = importPosition.nNameThunkValue & 0x7FFFFFFF;
                }
            }

            if (importPosition.nOrdinal == 0) {
                importPosition.sFunction = importPosition.sName;
            } else {
                importPosition.sFunction = QString("%1").arg(importPosition.nOrdinal);
            }

            if (bIs64) {
                nNameThunksRVA += 8;
                nNameThunksOffset += 8;
                nAddressThunksRVA += 8;
                nAddressThunksOffset += 8;
                nBoundThunksRVA += 8;
                nBoundThunksOffset += 8;
            } else {
                nNameThunksRVA += 4;
                nNameThunksOffset += 4;
                nAddressThunksRVA += 4;
                nAddressThunksOffset += 4;
                nBoundThunksRVA += 4;
                nBoundThunksOffset += 4;
            }

            listResult.append(importPosition);
        }
    }

    return listResult;
}

QList<XPE::BOUND_IMPORT_POSITION> XPE::getBoundImportPositions()
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return getBoundImportPositions(&memoryMap);
}

QList<XPE::BOUND_IMPORT_POSITION> XPE::getBoundImportPositions(XBinary::_MEMORY_MAP *pMemoryMap)
{
    Q_UNUSED(pMemoryMap)

    QList<BOUND_IMPORT_POSITION> listResult;

    qint64 nBoundImportOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT);

    if (nBoundImportOffset != -1) {
        qint64 nOffset = nBoundImportOffset;

        while (true) {
            BOUND_IMPORT_POSITION record = {};

            record.descriptor = _read_IMAGE_BOUND_IMPORT_DESCRIPTOR(nOffset);

            if ((record.descriptor.TimeDateStamp) && (record.descriptor.OffsetModuleName)) {
                record.sName = read_ansiString(nBoundImportOffset + record.descriptor.OffsetModuleName);

                listResult.append(record);
            } else {
                break;
            }

            nOffset += sizeof(XPE_DEF::IMAGE_BOUND_IMPORT_DESCRIPTOR);
        }
    }

    return listResult;
}

XPE_DEF::IMAGE_BOUND_IMPORT_DESCRIPTOR XPE::_read_IMAGE_BOUND_IMPORT_DESCRIPTOR(qint64 nOffset)
{
    XPE_DEF::IMAGE_BOUND_IMPORT_DESCRIPTOR result = {};

    result.TimeDateStamp = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_BOUND_IMPORT_DESCRIPTOR, TimeDateStamp));
    result.OffsetModuleName = read_uint16(nOffset + offsetof(XPE_DEF::IMAGE_BOUND_IMPORT_DESCRIPTOR, OffsetModuleName));
    result.NumberOfModuleForwarderRefs = read_uint16(nOffset + offsetof(XPE_DEF::IMAGE_BOUND_IMPORT_DESCRIPTOR, NumberOfModuleForwarderRefs));

    return result;
}

qint64 XPE::getBoundImportRecordOffset(qint32 nNumber)
{
    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT);

    nOffset += sizeof(XPE_DEF::IMAGE_BOUND_IMPORT_DESCRIPTOR) * nNumber;

    return nOffset;
}

qint64 XPE::getBoundImportRecordSize()
{
    return sizeof(XPE_DEF::IMAGE_BOUND_IMPORT_DESCRIPTOR);
}

void XPE::setBoundImport_TimeDateStamp(quint32 nNumber, quint32 nValue)
{
    qint64 nOffset = getBoundImportRecordOffset(nNumber);

    write_uint32(nOffset + offsetof(XPE_DEF::IMAGE_BOUND_IMPORT_DESCRIPTOR, TimeDateStamp), nValue);
}

void XPE::setBoundImport_OffsetModuleName(quint32 nNumber, quint16 nValue)
{
    qint64 nOffset = getBoundImportRecordOffset(nNumber);

    write_uint16(nOffset + offsetof(XPE_DEF::IMAGE_BOUND_IMPORT_DESCRIPTOR, OffsetModuleName), nValue);
}

void XPE::setBoundImport_NumberOfModuleForwarderRefs(quint32 nNumber, quint16 nValue)
{
    qint64 nOffset = getBoundImportRecordOffset(nNumber);

    write_uint16(nOffset + offsetof(XPE_DEF::IMAGE_BOUND_IMPORT_DESCRIPTOR, NumberOfModuleForwarderRefs), nValue);
}

qint32 XPE::getNumberOfImports()
{
    QList<XPE_DEF::IMAGE_IMPORT_DESCRIPTOR> listImports = getImportDescriptors();

    return listImports.count();
}

QString XPE::getImportLibraryName(quint32 nNumber)
{
    QString sResult;

    QList<IMAGE_IMPORT_DESCRIPTOR_EX> listImports = getImportDescriptorsEx();  // TODO Check

    if (nNumber < (quint32)listImports.count()) {
        sResult = listImports.at(nNumber).sLibrary;
    }

    return sResult;
}

QString XPE::getImportLibraryName(quint32 nNumber, QList<XPE::IMPORT_HEADER> *pListImport)
{
    QString sResult;

    if ((qint32)nNumber < pListImport->count()) {
        sResult = pListImport->at(nNumber).sName;
    }

    return sResult;
}

qint32 XPE::getNumberOfImportThunks(quint32 nNumber)
{
    QList<IMPORT_HEADER> listImportHeaders = getImports();

    return getNumberOfImportThunks(nNumber, &listImportHeaders);
}

qint32 XPE::getNumberOfImportThunks(quint32 nNumber, QList<XPE::IMPORT_HEADER> *pListImport)
{
    qint32 nResult = 0;

    if (nNumber < (quint32)pListImport->count()) {
        nResult = pListImport->at(nNumber).listPositions.count();
    }

    return nResult;
}

qint64 XPE::getNetHeaderOffset()
{
    return getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);
}

qint64 XPE::getNetHeaderSize()
{
    qint64 nResult = 0;

    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        nResult = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, cb));
    }

    return nResult;
}

XPE_DEF::IMAGE_COR20_HEADER XPE::_read_IMAGE_COR20_HEADER(qint64 nOffset)
{
    XPE_DEF::IMAGE_COR20_HEADER result = {};

    result.cb = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, cb));
    result.MajorRuntimeVersion = read_uint16(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, MajorRuntimeVersion));
    result.MinorRuntimeVersion = read_uint16(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, MinorRuntimeVersion));
    result.MetaData = read_IMAGE_DATA_DIRECTORY(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, MetaData));
    result.Flags = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, Flags));
    result.EntryPointRVA = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, EntryPointRVA));
    result.Resources = read_IMAGE_DATA_DIRECTORY(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, Resources));
    result.StrongNameSignature = read_IMAGE_DATA_DIRECTORY(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, StrongNameSignature));
    result.CodeManagerTable = read_IMAGE_DATA_DIRECTORY(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, CodeManagerTable));
    result.VTableFixups = read_IMAGE_DATA_DIRECTORY(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, VTableFixups));
    result.ExportAddressTableJumps = read_IMAGE_DATA_DIRECTORY(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, ExportAddressTableJumps));
    result.ManagedNativeHeader = read_IMAGE_DATA_DIRECTORY(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, ManagedNativeHeader));

    return result;
}

XPE_DEF::IMAGE_COR20_HEADER XPE::getNetHeader()
{
    XPE_DEF::IMAGE_COR20_HEADER result = {};

    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        result = _read_IMAGE_COR20_HEADER(nOffset);
    }

    return result;
}

quint32 XPE::getNetHeader_cb()
{
    quint32 nResult = 0;

    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        nResult = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, cb));
    }

    return nResult;
}

quint16 XPE::getNetHeader_MajorRuntimeVersion()
{
    quint16 nResult = 0;

    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        nResult = read_uint16(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, MajorRuntimeVersion));
    }

    return nResult;
}

quint16 XPE::getNetHeader_MinorRuntimeVersion()
{
    quint16 nResult = 0;

    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        nResult = read_uint16(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, MinorRuntimeVersion));
    }

    return nResult;
}

quint32 XPE::getNetHeader_MetaData_Address()
{
    quint32 nResult = 0;

    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        nResult = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, MetaData.VirtualAddress));
    }

    return nResult;
}

quint32 XPE::getNetHeader_MetaData_Size()
{
    quint32 nResult = 0;

    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        nResult = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, MetaData.Size));
    }

    return nResult;
}

quint32 XPE::getNetHeader_Flags()
{
    quint32 nResult = 0;

    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        nResult = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, Flags));
    }

    return nResult;
}

quint32 XPE::getNetHeader_EntryPoint()
{
    quint32 nResult = 0;

    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        nResult = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, EntryPointRVA));
    }

    return nResult;
}

quint32 XPE::getNetHeader_Resources_Address()
{
    quint32 nResult = 0;

    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        nResult = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, Resources.VirtualAddress));
    }

    return nResult;
}

quint32 XPE::getNetHeader_Resources_Size()
{
    quint32 nResult = 0;

    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        nResult = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, Resources.Size));
    }

    return nResult;
}

quint32 XPE::getNetHeader_StrongNameSignature_Address()
{
    quint32 nResult = 0;

    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        nResult = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, StrongNameSignature.VirtualAddress));
    }

    return nResult;
}

quint32 XPE::getNetHeader_StrongNameSignature_Size()
{
    quint32 nResult = 0;

    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        nResult = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, StrongNameSignature.Size));
    }

    return nResult;
}

quint32 XPE::getNetHeader_CodeManagerTable_Address()
{
    quint32 nResult = 0;

    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        nResult = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, CodeManagerTable.VirtualAddress));
    }

    return nResult;
}

quint32 XPE::getNetHeader_CodeManagerTable_Size()
{
    quint32 nResult = 0;

    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        nResult = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, CodeManagerTable.Size));
    }

    return nResult;
}

quint32 XPE::getNetHeader_VTableFixups_Address()
{
    quint32 nResult = 0;

    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        nResult = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, VTableFixups.VirtualAddress));
    }

    return nResult;
}

quint32 XPE::getNetHeader_VTableFixups_Size()
{
    quint32 nResult = 0;

    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        nResult = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, VTableFixups.Size));
    }

    return nResult;
}

quint32 XPE::getNetHeader_ExportAddressTableJumps_Address()
{
    quint32 nResult = 0;

    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        nResult = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, ExportAddressTableJumps.VirtualAddress));
    }

    return nResult;
}

quint32 XPE::getNetHeader_ExportAddressTableJumps_Size()
{
    quint32 nResult = 0;

    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        nResult = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, ExportAddressTableJumps.Size));
    }

    return nResult;
}

quint32 XPE::getNetHeader_ManagedNativeHeader_Address()
{
    quint32 nResult = 0;

    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        nResult = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, ManagedNativeHeader.VirtualAddress));
    }

    return nResult;
}

quint32 XPE::getNetHeader_ManagedNativeHeader_Size()
{
    quint32 nResult = 0;

    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        nResult = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, ManagedNativeHeader.Size));
    }

    return nResult;
}

void XPE::setNetHeader_cb(quint32 nValue)
{
    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        write_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, cb), nValue);
    }
}

void XPE::setNetHeader_MajorRuntimeVersion(quint16 nValue)
{
    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        write_uint16(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, MajorRuntimeVersion), nValue);
    }
}

void XPE::setNetHeader_MinorRuntimeVersion(quint16 nValue)
{
    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        write_uint16(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, MinorRuntimeVersion), nValue);
    }
}

void XPE::setNetHeader_MetaData_Address(quint32 nValue)
{
    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        write_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, MetaData.VirtualAddress), nValue);
    }
}

void XPE::setNetHeader_MetaData_Size(quint32 nValue)
{
    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        write_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, MetaData.Size), nValue);
    }
}

void XPE::setNetHeader_Flags(quint32 nValue)
{
    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        write_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, Flags), nValue);
    }
}

void XPE::setNetHeader_EntryPoint(quint32 nValue)
{
    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        write_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, EntryPointRVA), nValue);
    }
}

void XPE::setNetHeader_Resources_Address(quint32 nValue)
{
    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        write_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, Resources.VirtualAddress), nValue);
    }
}

void XPE::setNetHeader_Resources_Size(quint32 nValue)
{
    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        write_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, Resources.Size), nValue);
    }
}

void XPE::setNetHeader_StrongNameSignature_Address(quint32 nValue)
{
    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        write_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, StrongNameSignature.VirtualAddress), nValue);
    }
}

void XPE::setNetHeader_StrongNameSignature_Size(quint32 nValue)
{
    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        write_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, StrongNameSignature.Size), nValue);
    }
}

void XPE::setNetHeader_CodeManagerTable_Address(quint32 nValue)
{
    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        write_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, CodeManagerTable.VirtualAddress), nValue);
    }
}

void XPE::setNetHeader_CodeManagerTable_Size(quint32 nValue)
{
    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        write_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, CodeManagerTable.Size), nValue);
    }
}

void XPE::setNetHeader_VTableFixups_Address(quint32 nValue)
{
    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        write_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, VTableFixups.VirtualAddress), nValue);
    }
}

void XPE::setNetHeader_VTableFixups_Size(quint32 nValue)
{
    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        write_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, VTableFixups.Size), nValue);
    }
}

void XPE::setNetHeader_ExportAddressTableJumps_Address(quint32 nValue)
{
    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        write_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, ExportAddressTableJumps.VirtualAddress), nValue);
    }
}

void XPE::setNetHeader_ExportAddressTableJumps_Size(quint32 nValue)
{
    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        write_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, ExportAddressTableJumps.Size), nValue);
    }
}

void XPE::setNetHeader_ManagedNativeHeader_Address(quint32 nValue)
{
    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        write_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, ManagedNativeHeader.VirtualAddress), nValue);
    }
}

void XPE::setNetHeader_ManagedNativeHeader_Size(quint32 nValue)
{
    qint64 nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

    if (nOffset != -1) {
        write_uint32(nOffset + offsetof(XPE_DEF::IMAGE_COR20_HEADER, ManagedNativeHeader.Size), nValue);
    }
}

QList<XBinary::SYMBOL_RECORD> XPE::getSymbolRecords(XBinary::_MEMORY_MAP *pMemoryMap, SYMBOL_TYPE symbolType)
{
    // TODO Import
    QList<SYMBOL_RECORD> listResult;

    XADDR nModuleAddress = getModuleAddress();

    if (symbolType & SYMBOL_TYPE_EXPORT) {
        EXPORT_HEADER exportHeader = getExport(pMemoryMap, false);

        qint32 nNumberOfPositions = exportHeader.listPositions.count();

        for (qint32 i = 0; i < nNumberOfPositions; i++) {
            SYMBOL_RECORD record = {};

            record.symbolType = SYMBOL_TYPE_EXPORT;
            record.nAddress = exportHeader.listPositions.at(i).nAddress;
            record.nSize = 0;  // Check
            record.nModuleAddress = nModuleAddress;
            record.nOrdinal = exportHeader.listPositions.at(i).nOrdinal;
            record.sName = exportHeader.listPositions.at(i).sFunctionName;
            record.sFunction = record.sName;

            listResult.append(record);
        }
    }

    if (symbolType & SYMBOL_TYPE_IMPORT) {
        QList<XPE::IMPORT_HEADER> importHeaders = getImports(pMemoryMap);

        qint32 nNumberOfRecords = importHeaders.count();

        for (qint32 i = 0; i < nNumberOfRecords; i++) {
            QString sName = importHeaders.at(i).sName.toUpper();

            qint32 nNumberOfPositions = importHeaders.at(i).listPositions.count();

            for (qint32 j = 0; j < nNumberOfPositions; j++) {
                SYMBOL_RECORD record = {};

                record.symbolType = SYMBOL_TYPE_IMPORT;
                record.nAddress = importHeaders.at(i).listPositions.at(j).nThunkValue;
                record.nSize = 0;  // TODO 4/8
                record.nModuleAddress = nModuleAddress;
                record.nOrdinal = importHeaders.at(i).listPositions.at(j).nOrdinal;
                record.sName = importHeaders.at(i).listPositions.at(j).sName;
                record.sFunction = QString("%1#%2").arg(sName).arg(importHeaders.at(i).listPositions.at(j).sFunction);

                listResult.append(record);
            }
        }
    }

    return listResult;
}

bool XPE::removeDosStub()
{
    return _resizeDosStubSize(0);
}

bool XPE::addDosStub(const QString &sFileName)
{
    bool bResult = false;

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        qint64 nNewSize = file.size();

        bResult = _resizeDosStubSize(nNewSize);

        if (bResult) {
            bResult = copyDeviceMemory(&file, 0, getDevice(), getDosStubOffset(), nNewSize);
        }

        file.close();
    }

    return bResult;
}

bool XPE::_resizeDosStubSize(qint64 nNewStubSize)
{
    bool bResult = true;

    qint64 nStubOffset = getDosStubOffset();
    qint64 nStubSize = getDosStubSize();

    qint32 nRawDelta = nNewStubSize - nStubSize;
    nRawDelta = S_ALIGN_UP(nRawDelta, 4);

    if (nRawDelta) {
        qint64 nSectionsTableOffset = getSectionsTableOffset();
        qint32 nNumberOfSections = getFileHeader_NumberOfSections();

        qint64 nHeadersSize = _calculateHeadersSize(nSectionsTableOffset, nNumberOfSections);
        qint64 nNewHeadersSize = _calculateHeadersSize(nSectionsTableOffset + nRawDelta, nNumberOfSections);

        qint64 nAlignDelta = nNewHeadersSize - nHeadersSize;

        qint64 nHeadersRawSize = nSectionsTableOffset + sizeof(XPE_DEF::IMAGE_SECTION_HEADER) * nNumberOfSections - getNtHeadersOffset();

        if (nRawDelta > 0) {
            if (nAlignDelta) {
                if (bResult) {
                    bResult = resize(getDevice(), getSize() + nAlignDelta);
                }

                if (bResult) {
                    bResult = moveMemory(nHeadersSize, nNewHeadersSize, getSize() - nHeadersSize);
                }
            }

            if (bResult) {
                // Move headers
                bResult = moveMemory(nStubOffset + nStubSize, nStubOffset + nStubSize + nRawDelta, nHeadersRawSize);
            }

        } else if (nRawDelta < 0) {
            if (bResult) {
                // Move headers
                bResult = moveMemory(nStubOffset + nStubSize, nStubOffset + nStubSize + nRawDelta, nHeadersRawSize);
            }

            if (nAlignDelta) {
                if (bResult) {
                    bResult = moveMemory(nHeadersSize, nNewHeadersSize, getSize() - nHeadersSize);
                }

                if (bResult) {
                    bResult = resize(getDevice(), getSize() + nAlignDelta);
                }
            }
        }

        if (bResult) {
            set_e_lfanew((quint32)(nStubOffset + nStubSize + nRawDelta));
            _fixFileOffsets(nAlignDelta);
            _fixHeadersSize();
        }
    }

    return bResult;
}

// bool XPE::_setLFANEW(quint64 nNewOffset)
//{
//     bool bResult = true;

//    qint64 nRawDelta = nNewOffset - get_e_lfanew();

//    if (nRawDelta) {
//        qint64 nSectionsTableOffset = getSectionsTableOffset();
//        qint32 nNumberOfSections = getFileHeader_NumberOfSections();

//        qint64 nHeadersSize = _calculateHeadersSize(nSectionsTableOffset,nNumberOfSections);
//        qint64 nNewHeadersSize = _calculateHeadersSize(nSectionsTableOffset+nRawDelta,nNumberOfSections);

//        set_e_lfanew((quint32)nNewOffset);

//        _fixHeadersSize();
//        _fixFileOffsets(nNewHeadersSize - nHeadersSize);
//    }

//    return bResult;
//}

XPE_DEF::WIN_CERT_RECORD XPE::read_WIN_CERT_RECORD(qint64 nOffset)
{
    XPE_DEF::WIN_CERT_RECORD result = {};

    result.dwLength = read_uint32(nOffset + offsetof(XPE_DEF::WIN_CERT_RECORD, dwLength));
    result.wRevision = read_uint16(nOffset + offsetof(XPE_DEF::WIN_CERT_RECORD, wRevision));
    result.wCertificateType = read_uint16(nOffset + offsetof(XPE_DEF::WIN_CERT_RECORD, wCertificateType));

    return result;
}

QList<XPE::CERT> XPE::getCertList()
{
    XPE_DEF::IMAGE_DATA_DIRECTORY dd = getOptionalHeader_DataDirectory(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_SECURITY);

    return getCertList(dd.VirtualAddress, dd.Size);
}

QList<XPE::CERT> XPE::getCertList(qint64 nOffset, qint64 nSize)
{
    QList<CERT> listResult;

    while (nSize > 0) {
        CERT record = {};
        record.nOffset = nOffset;
        record.record = read_WIN_CERT_RECORD(nOffset);

        if (record.record.dwLength > nSize) {
            break;
        }

        if (record.record.wRevision != 0x0200) {
            break;
        }

        qint64 _nOffset = nOffset + sizeof(XPE_DEF::WIN_CERT_RECORD);
        qint64 _nSize = nSize - sizeof(XPE_DEF::WIN_CERT_RECORD);

        record.bIsValid = true;

        getCertRecord(&record, _nOffset, _nSize, &(record.certRecord));

        // TODO Check function

        //        while(_nSize>0)
        //        {
        //            CERT_TAG certTag=read_CertTag(_nOffset,0);

        //            if((!certTag.bValid)||(certTag.nSize>_nSize))
        //            {
        //                break;
        //            }

        //            _nOffset+=certTag.nHeaderSize;
        //            _nSize-=certTag.nHeaderSize;

        //        #ifdef QT_DEBUG
        //            qDebug("TAG: %x",certTag.nTag);
        //            qDebug("Size: %d",certTag.nSize);
        //            qDebug("_Size: %d",_nSize);
        //        #endif

        //            if(certTag.nTag==XPE_DEF::S_MBEDTLS_ASN1_OID)
        //            {
        //                QString
        //                sOID=read_ASN_OIDString(certTag.nOffset+certTag.nHeaderSize,certTag.nSize);
        //            #ifdef QT_DEBUG
        //                qDebug("OID %s",sOID.toLatin1().data());
        //            #endif
        //                _nOffset+=certTag.nSize;
        //                _nSize-=certTag.nSize;
        //            }
        //            else if(certTag.nTag==XPE_DEF::S_MBEDTLS_ASN1_INTEGER)
        //            {
        //                qint64
        //                nVersion=read_ASN_Integer(certTag.nOffset+certTag.nHeaderSize,certTag.nSize);
        //            #ifdef QT_DEBUG
        //                qDebug("INTEGER %d",nVersion);
        //            #endif
        //                _nOffset+=certTag.nSize;
        //                _nSize-=certTag.nSize;
        //            }
        //            else if(certTag.nTag==XPE_DEF::S_MBEDTLS_ASN1_BIT_STRING)
        //            {
        //            #ifdef QT_DEBUG
        //                qDebug("BIT_STRING TODO");
        //            #endif
        //                _nOffset+=certTag.nSize;
        //                _nSize-=certTag.nSize;
        //            }
        //            else
        //            if(certTag.nTag==XPE_DEF::S_MBEDTLS_ASN1_OCTET_STRING)
        //            {
        //            #ifdef QT_DEBUG
        //                qDebug("OCTET_STRING TODO");
        //            #endif
        //                _nOffset+=certTag.nSize;
        //                _nSize-=certTag.nSize;
        //            }
        //            else
        //            if(certTag.nTag==XPE_DEF::S_MBEDTLS_ASN1_CONTEXT_SPECIFIC)
        //            {
        //            #ifdef QT_DEBUG
        //                qDebug("CONTEXT_SPECIFIC TODO");
        //            #endif
        //                _nOffset+=certTag.nSize;
        //                _nSize-=certTag.nSize;
        //            }
        //            else if(certTag.nTag==XPE_DEF::S_MBEDTLS_ASN1_BOOLEAN)
        //            {
        //            #ifdef QT_DEBUG
        //                qDebug("BOOLEAN TODO");
        //            #endif
        //                _nOffset+=certTag.nSize;
        //                _nSize-=certTag.nSize;
        //            }
        //            else
        //            if(certTag.nTag==(XPE_DEF::S_MBEDTLS_ASN1_CONSTRUCTED+XPE_DEF::S_MBEDTLS_ASN1_PRINTABLE_STRING))
        //            {
        //            #ifdef QT_DEBUG
        //                qDebug("PRINTABLE_STRING TODO");
        //            #endif
        //                _nOffset+=certTag.nSize;
        //                _nSize-=certTag.nSize;
        //            }
        //            else
        //            if(certTag.nTag==XPE_DEF::S_MBEDTLS_ASN1_PRINTABLE_STRING)
        //            {
        //            #ifdef QT_DEBUG
        //                qDebug("PRINTABLE_STRING TODO");
        //            #endif
        //                _nOffset+=certTag.nSize;
        //                _nSize-=certTag.nSize;
        //            }
        //            else if(certTag.nTag==XPE_DEF::S_MBEDTLS_ASN1_UTC_TIME)
        //            {
        //            #ifdef QT_DEBUG
        //                qDebug("UTC_TIME TODO");
        //            #endif
        //                _nOffset+=certTag.nSize;
        //                _nSize-=certTag.nSize;
        //            }
        //            else
        //            if(certTag.nTag==(XPE_DEF::S_MBEDTLS_ASN1_CONTEXT_SPECIFIC+XPE_DEF::S_MBEDTLS_ASN1_BOOLEAN))
        //            {
        //            #ifdef QT_DEBUG
        //                qDebug("BOOLEAN TODO");
        //            #endif
        //                _nOffset+=certTag.nSize;
        //                _nSize-=certTag.nSize;
        //            }
        //        }

        //        {
        //            CERT_TAG
        //            certTag=read_CertTag(_nOffset,(XPE_DEF::S_MBEDTLS_ASN1_CONSTRUCTED)|(XPE_DEF::S_MBEDTLS_ASN1_SEQUENCE));

        //            if((!certTag.bValid)||(certTag.nSize>_nSize)) break;

        //            _nOffset+=certTag.nHeaderSize;
        //            _nSize-=certTag.nHeaderSize;
        //        }
        //        {
        //            CERT_TAG
        //            certTag=read_CertTag(_nOffset,XPE_DEF::S_MBEDTLS_ASN1_OID);

        //            if((!certTag.bValid)||(certTag.nSize>_nSize)) break;

        //            _nOffset+=certTag.nHeaderSize;
        //            _nSize-=certTag.nHeaderSize;

        //            QString
        //            sOID=read_ASN_OIDString(certTag.nOffset+certTag.nHeaderSize,certTag.nSize);

        //            if(sOID!="1.2.840.113549.1.7.2") // "PKCS #7 Signed Data"
        //            {
        //                break;
        //            }

        //            _nOffset+=certTag.nSize;
        //            _nSize-=certTag.nSize;
        //        }
        //        {
        //            CERT_TAG
        //            certTag=read_CertTag(_nOffset,(XPE_DEF::S_MBEDTLS_ASN1_CONSTRUCTED)|(XPE_DEF::S_MBEDTLS_ASN1_CONTEXT_SPECIFIC));

        //            if((!certTag.bValid)||(certTag.nSize>_nSize)) break;

        //            _nOffset+=certTag.nHeaderSize;
        //            _nSize-=certTag.nHeaderSize;
        //        }
        //        {
        //            CERT_TAG
        //            certTag=read_CertTag(_nOffset,(XPE_DEF::S_MBEDTLS_ASN1_CONSTRUCTED)|(XPE_DEF::S_MBEDTLS_ASN1_SEQUENCE));

        //            if((!certTag.bValid)||(certTag.nSize>_nSize)) break;

        //            _nOffset+=certTag.nHeaderSize;
        //            _nSize-=certTag.nHeaderSize;
        //        }
        //        {
        //            CERT_TAG
        //            certTag=read_CertTag(_nOffset,XPE_DEF::S_MBEDTLS_ASN1_INTEGER);

        //            if((!certTag.bValid)||(certTag.nSize>_nSize)) break;

        //            _nOffset+=certTag.nHeaderSize;
        //            _nSize-=certTag.nHeaderSize;

        //            qint64
        //            nVersion=read_ASN_Integer(certTag.nOffset+certTag.nHeaderSize,certTag.nSize);

        //            if(nVersion!=1) // Version=1
        //            {
        //                break;
        //            }

        //            _nOffset+=certTag.nSize;
        //            _nSize-=certTag.nSize;
        //        }

        //        {
        //            CERT_TAG
        //            certTag=read_CertTag(_nOffset,(XPE_DEF::S_MBEDTLS_ASN1_CONSTRUCTED)|(XPE_DEF::S_MBEDTLS_ASN1_SET));

        //            if((!certTag.bValid)||(certTag.nSize>_nSize)) break;

        //            _nOffset+=certTag.nHeaderSize;
        //            _nSize-=certTag.nHeaderSize;
        //        }
        //        {
        //            CERT_TAG
        //            certTag=read_CertTag(_nOffset,(XPE_DEF::S_MBEDTLS_ASN1_CONSTRUCTED)|(XPE_DEF::S_MBEDTLS_ASN1_SEQUENCE));

        //            if((!certTag.bValid)||(certTag.nSize>_nSize)) break;

        //            _nOffset+=certTag.nHeaderSize;
        //            _nSize-=certTag.nHeaderSize;
        //        }
        //        {
        //            CERT_TAG
        //            certTag=read_CertTag(_nOffset,XPE_DEF::S_MBEDTLS_ASN1_OID);

        //            if((!certTag.bValid)||(certTag.nSize>_nSize)) break;

        //            _nOffset+=certTag.nHeaderSize;
        //            _nSize-=certTag.nHeaderSize;

        //            QString
        //            sOID=read_ASN_OIDString(certTag.nOffset+certTag.nHeaderSize,certTag.nSize);

        //            // TODO SHA1

        //            _nOffset+=certTag.nSize;
        //            _nSize-=certTag.nSize;
        //        }
        //        {
        //            CERT_TAG
        //            certTag=read_CertTag(_nOffset,XPE_DEF::S_MBEDTLS_ASN1_NULL);

        //            if((!certTag.bValid)||(certTag.nSize>_nSize)) break;

        //            _nOffset+=certTag.nHeaderSize;
        //            _nSize-=certTag.nHeaderSize;
        //        }
        //        {
        //            CERT_TAG
        //            certTag=read_CertTag(_nOffset,(XPE_DEF::S_MBEDTLS_ASN1_CONSTRUCTED)|(XPE_DEF::S_MBEDTLS_ASN1_SEQUENCE));

        //            if((!certTag.bValid)||(certTag.nSize>_nSize)) break;

        //            _nOffset+=certTag.nHeaderSize;
        //            _nSize-=certTag.nHeaderSize;
        //        }

        //        while(_nSize>0)
        //        {
        //            CERT_TAG certTag=read_CertTag(_nOffset,0);

        //            if((!certTag.bValid)||(certTag.nSize>_nSize))   break;

        //            _nOffset+=certTag.nHeaderSize;
        //            _nSize-=certTag.nHeaderSize;

        //            _nOffset+=certTag.nSize;
        //            _nSize-=certTag.nSize;
        //        }

        //        {
        //            CERT_TAG
        //            certTag=read_CertTag(_nOffset,(XPE_DEF::S_MBEDTLS_ASN1_CONSTRUCTED)|(XPE_DEF::S_MBEDTLS_ASN1_CONTEXT_SPECIFIC));

        //            if((!certTag.bValid)||(certTag.nSize>_nSize)) break;

        //            _nOffset+=certTag.nHeaderSize;
        //            _nSize-=certTag.nHeaderSize;
        //        }
        //        {
        //            CERT_TAG
        //            certTag=read_CertTag(_nOffset,(XPE_DEF::S_MBEDTLS_ASN1_CONSTRUCTED)|(XPE_DEF::S_MBEDTLS_ASN1_SEQUENCE));

        //            if((!certTag.bValid)||(certTag.nSize>_nSize)) break;

        //            _nOffset+=certTag.nHeaderSize;
        //            _nSize-=certTag.nHeaderSize;
        //        }
        //        {
        //            CERT_TAG
        //            certTag=read_CertTag(_nOffset,(XPE_DEF::S_MBEDTLS_ASN1_CONSTRUCTED)|(XPE_DEF::S_MBEDTLS_ASN1_SEQUENCE));

        //            if((!certTag.bValid)||(certTag.nSize>_nSize)) break;

        //            _nOffset+=certTag.nHeaderSize;
        //            _nSize-=certTag.nHeaderSize;
        //        }

        //        while(_nSize>0)
        //        {
        //            CERT_TAG certTag=read_CertTag(_nOffset,0);

        //            if((!certTag.bValid)||(certTag.nSize>_nSize))   break;

        //            _nOffset+=certTag.nHeaderSize;
        //            _nSize-=certTag.nHeaderSize;

        //            _nOffset+=certTag.nSize;
        //            _nSize-=certTag.nSize;
        //        }

        //        CERT_TAG
        //        certTagConstruct=read_CertTag(_nOffset,(XPE_DEF::S_MBEDTLS_ASN1_CONSTRUCTED)|(XPE_DEF::S_MBEDTLS_ASN1_SEQUENCE));

        //        if(!certTagConstruct.bValid)
        //        {
        //            break;
        //        }

        //        _nOffset+=certTagConstruct.nHeaderSize;

        //        CERT_TAG
        //        certTagOID=read_CertTag(_nOffset,(XPE_DEF::S_MBEDTLS_ASN1_OID));

        //        if(!certTagOID.bValid)
        //        {
        //            break;
        //        }

        listResult.append(record);

        nOffset += (record.record.dwLength + sizeof(XPE_DEF::WIN_CERT_RECORD));
        nSize -= (record.record.dwLength + sizeof(XPE_DEF::WIN_CERT_RECORD));
    }

    return listResult;
}

QList<XPE::CERT> XPE::getCertList(QIODevice *pDevice, qint64 nOffset, qint64 nSize)
{
    XPE pe(pDevice);

    return pe.getCertList(nOffset, nSize);
}

QString XPE::certListToString(QList<CERT> *pCertList)
{
    QString sResult;

    qint32 nNumberOfCerts = pCertList->count();

    for (qint32 i = 0; i < nNumberOfCerts; i++) {
        sResult += QString("Valid: %1\n").arg(XBinary::boolToString(pCertList->at(i).bIsValid));
        sResult += QString("Offset: %1\n").arg(XBinary::valueToHex(MODE_UNKNOWN, pCertList->at(i).nOffset));
        sResult += QString("dwLength: %1\n").arg(XBinary::valueToHex(MODE_UNKNOWN, pCertList->at(i).record.dwLength));
        sResult += QString("wRevision: %1\n").arg(XBinary::valueToHex(MODE_UNKNOWN, pCertList->at(i).record.wRevision));
        sResult += QString("wCertificateType: %1\n").arg(XBinary::valueToHex(MODE_UNKNOWN, pCertList->at(i).record.wCertificateType));

        qint32 nNumberOfRecords = pCertList->at(i).certRecord.listRecords.count();

        for (qint32 j = 0; j < nNumberOfRecords; j++) {
            sResult += certRecordToString(pCertList->at(i).certRecord.listRecords.at(j), 0);
        }
    }

    return sResult;
}

QString XPE::certRecordToString(CERT_RECORD certRecord, qint32 nLevel)
{
    QString sResult;

    sResult += getSpaces(2 * nLevel) + QString("Valid: %1\n").arg(XBinary::boolToString(certRecord.certTag.bIsValid));
    sResult += getSpaces(2 * nLevel) + QString("Tag: %1\n").arg(XBinary::valueToHex(MODE_UNKNOWN, certRecord.certTag.nTag));
    sResult += getSpaces(2 * nLevel) + QString("Offset: %1\n").arg(XBinary::valueToHex(MODE_UNKNOWN, certRecord.certTag.nOffset));
    sResult += getSpaces(2 * nLevel) + QString("Size: %1\n").arg(XBinary::valueToHex(MODE_UNKNOWN, certRecord.certTag.nSize));

    if (certRecord.varValue.toString().size()) {
        sResult += getSpaces(2 * nLevel) + QString("Value: %1\n").arg(certRecord.varValue.toString());
    }

    qint32 nNumberOfRecords = certRecord.listRecords.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        sResult += certRecordToString(certRecord.listRecords.at(i), nLevel + 1);
    }

    return sResult;
}

XPE::CERT_TAG XPE::read_CertTag(qint64 nOffset, quint32 nTag)
{
    CERT_TAG result = {};

    result.nOffset = nOffset;
    result.nTag = read_uint8(nOffset);

    if (nTag) {
        result.bIsValid = (result.nTag == nTag);
    } else {
        result.bIsValid = true;
    }

    if (result.bIsValid) {
        PACKED_UINT packedInt = read_acn1_integer(nOffset + 1, 4);
        result.bIsValid = packedInt.bIsValid;
        result.nSize = packedInt.nValue;
        result.nHeaderSize = packedInt.nByteSize + 1;
    }

    return result;
}

void XPE::getCertRecord(CERT *pCert, qint64 nOffset, qint64 nSize, CERT_RECORD *pCertRecord)
{
    while ((nSize > 0) && (pCert->bIsValid)) {
        CERT_RECORD certRecord = {};

        certRecord.certTag = read_CertTag(nOffset, 0);

        if ((!certRecord.certTag.bIsValid) || (certRecord.certTag.nSize > nSize)) {
            pCert->bIsValid = false;
            break;
        }

        nOffset += certRecord.certTag.nHeaderSize;
        nSize -= certRecord.certTag.nHeaderSize;

        if ((certRecord.certTag.nTag) & (XPE_DEF::S_ASN1_CONSTRUCTED)) {
            getCertRecord(pCert, nOffset, certRecord.certTag.nSize, &certRecord);
        } else {
            if (certRecord.certTag.nTag == XPE_DEF::S_ASN1_OBJECT_ID) {
                certRecord.varValue = read_ASN_OIDString(nOffset, certRecord.certTag.nSize);
            } else if (certRecord.certTag.nTag == XPE_DEF::S_ASN1_INTEGER) {
                certRecord.varValue = read_ASN_Integer(nOffset, certRecord.certTag.nSize);
            }
        }

        nOffset += certRecord.certTag.nSize;
        nSize -= certRecord.certTag.nSize;

        pCertRecord->listRecords.append(certRecord);
    }
}

QString XPE::certTagToString(quint32 nTag)
{
    QString sResult;

    QString sSeparate = " | ";

    if (nTag & (XPE_DEF::S_ASN1_CONSTRUCTED)) sResult = appendText(sResult, "CONSTRUCTED", sSeparate);
    if (nTag & (XPE_DEF::S_ASN1_CONTEXT_SPECIFIC)) sResult = appendText(sResult, "CONTEXT_SPECIFIC", sSeparate);

    nTag &= 0x1F;

    if (nTag == (XPE_DEF::S_ASN1_BOOLEAN)) sResult = appendText(sResult, "BOOLEAN", sSeparate);
    if (nTag == (XPE_DEF::S_ASN1_INTEGER)) sResult = appendText(sResult, "INTEGER", sSeparate);
    if (nTag == (XPE_DEF::S_ASN1_BIT_STRING)) sResult = appendText(sResult, "BIT_STRING", sSeparate);
    if (nTag == (XPE_DEF::S_ASN1_OCTET_STRING)) sResult = appendText(sResult, "OCTET_STRING", sSeparate);
    if (nTag == (XPE_DEF::S_ASN1_NULL)) sResult = appendText(sResult, "NULL", sSeparate);
    if (nTag == (XPE_DEF::S_ASN1_OBJECT_ID)) sResult = appendText(sResult, "OBJECT_ID", sSeparate);
    if (nTag == (XPE_DEF::S_ASN1_OBJECT_DESCRIPTOR)) sResult = appendText(sResult, "OBJECT_DESCRIPTOR", sSeparate);
    if (nTag == (XPE_DEF::S_ASN1_REAL)) sResult = appendText(sResult, "REAL", sSeparate);
    if (nTag == (XPE_DEF::S_ASN1_ENUMERATED)) sResult = appendText(sResult, "ENUMERATED", sSeparate);
    if (nTag == (XPE_DEF::S_ASN1_EMBEDDED_PDV)) sResult = appendText(sResult, "EMBEDDED_PDV", sSeparate);
    if (nTag == (XPE_DEF::S_ASN1_UTF8_STRING)) sResult = appendText(sResult, "UTF8_STRING", sSeparate);
    if (nTag == (XPE_DEF::S_ASN1_SEQUENCE)) sResult = appendText(sResult, "SEQUENCE", sSeparate);
    if (nTag == (XPE_DEF::S_ASN1_SET)) sResult = appendText(sResult, "SET", sSeparate);
    if (nTag == (XPE_DEF::S_ASN1_NUMERIC_STRING)) sResult = appendText(sResult, "NUMERIC_STRING", sSeparate);
    if (nTag == (XPE_DEF::S_ASN1_PRINTABLE_STRING)) sResult = appendText(sResult, "PRINTABLE_STRING", sSeparate);
    if (nTag == (XPE_DEF::S_ASN1_T61_STRING)) sResult = appendText(sResult, "T61_STRING", sSeparate);
    if (nTag == (XPE_DEF::S_ASN1_VIDEOTEX_STRING)) sResult = appendText(sResult, "VIDEOTEX_STRING", sSeparate);
    if (nTag == (XPE_DEF::S_ASN1_IA5_STRING)) sResult = appendText(sResult, "IA5_STRING", sSeparate);
    if (nTag == (XPE_DEF::S_ASN1_UTC_TIME)) sResult = appendText(sResult, "UTC_TIME", sSeparate);
    if (nTag == (XPE_DEF::S_ASN1_GENERALIZED_TIME)) sResult = appendText(sResult, "GENERALIZED_TIME", sSeparate);
    if (nTag == (XPE_DEF::S_ASN1_ASN1_GRAPHIC_STRING)) sResult = appendText(sResult, "ASN1_GRAPHIC_STRING", sSeparate);
    if (nTag == (XPE_DEF::S_ASN1_VISIBLE_STRING)) sResult = appendText(sResult, "VISIBLE_STRING", sSeparate);
    if (nTag == (XPE_DEF::S_ASN1_GENERAL_STRING)) sResult = appendText(sResult, "GENERAL_STRING", sSeparate);
    if (nTag == (XPE_DEF::S_ASN1_UNIVERSAL_STRING)) sResult = appendText(sResult, "UNIVERSAL_STRING", sSeparate);
    if (nTag == (XPE_DEF::S_ASN1_BMP_STRING)) sResult = appendText(sResult, "BMP_STRING", sSeparate);
    if (nTag == (XPE_DEF::S_ASN1_PRIMITIVE)) sResult = appendText(sResult, "PRIMITIVE", sSeparate);

    return sResult;
}

QString XPE::objectIdToString(const QString &sObjectID)
{
    QString sResult;

    if (sObjectID == "1.2.840.113549.1.1.1") sResult = "RSA";
    else if (sObjectID == "1.2.840.113549.1.1.5") sResult = "SHA1-RSA";
    else if (sObjectID == "1.2.840.113549.1.1.11") sResult = "SHA256-RSA";
    else if (sObjectID == "1.2.840.113549.1.1.12") sResult = "SHA384-RSA";
    else if (sObjectID == "1.2.840.113549.1.7.2") sResult = "RSA over SignedData";
    else if (sObjectID == "1.2.840.113549.1.7.1") sResult = "id-data";
    else if (sObjectID == "1.2.840.113549.1.9.3") sResult = "id-contentType";
    else if (sObjectID == "1.2.840.113549.1.9.4") sResult = "id-messageDigest";
    else if (sObjectID == "1.2.840.113549.1.9.5") sResult = "id-signingTime";
    else if (sObjectID == "1.2.840.113549.1.9.6") sResult = "id-countersignature";
    else if (sObjectID == "1.2.840.113549.1.9.16.1.4") sResult = "id-ct-TSTInfo";
    else if (sObjectID == "1.2.840.113549.1.9.16.2.12") sResult = "S/MIME signing certificate";
    else if (sObjectID == "1.2.840.113549.1.9.16.2.47") sResult = "Signing certificate V2";
    else if (sObjectID == "1.3.6.1.4.1.311.20.2") sResult = "szOID_ENROLL_CERTTYPE_EXTENSION";
    else if (sObjectID == "1.3.6.1.4.1.311.21.1") sResult = "Certificate services Certification Authority (CA) version";
    else if (sObjectID == "1.3.6.1.4.1.311.2.1.4") sResult = "SPC_INDIRECT_DATA_OBJID";
    else if (sObjectID == "1.3.6.1.4.1.311.2.1.11") sResult = "SPC_STATEMENT_TYPE_OBJID";
    else if (sObjectID == "1.3.6.1.4.1.311.2.1.12") sResult = "SPC_SP_OPUS_INFO_OBJID";
    else if (sObjectID == "1.3.6.1.4.1.311.2.1.15") sResult = "SPC_PE_IMAGE_DATA_OBJID";
    else if (sObjectID == "1.3.6.1.4.1.311.2.1.21") sResult = "SPC_INDIVIDUAL_SP_KEY_PURPOSE_OBJID";
    else if (sObjectID == "1.3.6.1.4.1.311.2.4.1") sResult = "Ms-SpcNestedSignature";
    else if (sObjectID == "1.3.6.1.4.1.311.3.3.1") sResult = "Timestamping signature (Ms-CounterSign)";
    else if (sObjectID == "1.3.6.1.5.5.7.1.1") sResult = "Certificate authority information access";
    //    else if (sObjectID=="1.3.6.1.4.1.311.10.3.28")      sResult="";
    else if (sObjectID == "1.3.6.1.4.1.601.10.3.2") sResult = "SPC_STATEMENT_TYPE_OBJID";
    else if (sObjectID == "1.3.14.3.2.26") sResult = "SHA-1";
    else if (sObjectID == "2.5.4.3") sResult = "Common name";
    else if (sObjectID == "2.5.4.6") sResult = "Country name";
    else if (sObjectID == "2.5.4.7") sResult = "Locality Name";
    else if (sObjectID == "2.5.4.8") sResult = "State or Province name";
    else if (sObjectID == "2.5.4.9") sResult = "Street address";
    else if (sObjectID == "2.5.4.10") sResult = "Organization name";
    else if (sObjectID == "2.5.4.11") sResult = "Organization unit name";
    else if (sObjectID == "2.5.29.14") sResult = "Subject key identifier";
    else if (sObjectID == "2.5.29.15") sResult = "Key usage";
    else if (sObjectID == "2.5.29.17") sResult = "subjectAltName";
    else if (sObjectID == "2.5.29.19") sResult = "Basic constraints";
    else if (sObjectID == "2.5.29.31") sResult = "Certificate Revocation List distribution points";
    else if (sObjectID == "2.5.29.32") sResult = "Certificate policies";
    else if (sObjectID == "2.5.29.35") sResult = "Authority key identifier";
    else if (sObjectID == "2.5.29.37") sResult = "Certificate extension: \"extKeyUsage\" (Extended key usage)";
    else if (sObjectID == "2.16.840.1.113730.1.1") sResult = "Netscape certificate type";
    else if (sObjectID == "2.16.840.1.101.3.4.2.1") sResult = "SHA256";
    else {
        sResult = sObjectID;
#ifdef QT_DEBUG
        qDebug("Object ID: %s", sObjectID.toLatin1().data());
#endif
    }

    return sResult;
}

QString XPE::getCertHash(XBinary::HASH hash)
{
    Q_UNUSED(hash)

    QString sResult;

    // TODO

    return sResult;
}

QList<XBinary::FMT_MSG> XPE::checkFileFormat(bool bDeep, PDSTRUCT *pPdStruct)
{
    QList<XBinary::FMT_MSG> listResult;

    bool bSuccess = true;

    _MEMORY_MAP memoryMap = getMemoryMap(MAPMODE_SECTIONS, pPdStruct);

    quint32 nFileAlignment = getOptionalHeader_FileAlignment();
    quint32 nSectionAlignment = getOptionalHeader_SectionAlignment();

    if (bSuccess) {
        quint32 nRelEP = getOptionalHeader_AddressOfEntryPoint();
        _addCheckFormatTest(&listResult, &bSuccess, FMT_MSG_CODE_INVALID_ENTRYPOINT, FMT_MSG_TYPE_ERROR, "OptionalHeader.AddressOfEntryPoint", nRelEP,
                            XBinary::valueToHex(nRelEP), !isRelAddressValid(&memoryMap, nRelEP));
    }

    if (bSuccess && bDeep) {
        quint32 nCheckSumOrig = getOptionalHeader_CheckSum();
        quint32 nCheckSumCalc = calculateCheckSum();

        _addCheckFormatTest(&listResult, &bSuccess, FMT_MSG_CODE_INVALID_CHECKSUM, FMT_MSG_TYPE_WARNING, "OptionalHeader.CheckSum", nCheckSumOrig,
                            XBinary::valueToHex(nCheckSumOrig), nCheckSumOrig != nCheckSumCalc);
    }

    if (bSuccess) {
        _addCheckFormatTest(&listResult, &bSuccess, FMT_MSG_CODE_INVALID_SECTIONSTABLE, FMT_MSG_TYPE_ERROR, "OptionalHeader.FileAlignment", nFileAlignment,
                            XBinary::valueToHex(nFileAlignment), nFileAlignment & (nFileAlignment - 1));
        _addCheckFormatTest(&listResult, &bSuccess, FMT_MSG_CODE_INVALID_SECTIONSTABLE, FMT_MSG_TYPE_ERROR, "OptionalHeader.FileAlignment", nFileAlignment,
                            XBinary::valueToHex(nFileAlignment), nFileAlignment < 0x200);
    }

    if (bSuccess) {
        _addCheckFormatTest(&listResult, &bSuccess, FMT_MSG_CODE_INVALID_SECTIONSTABLE, FMT_MSG_TYPE_ERROR, "OptionalHeader.SectionAlignment", nSectionAlignment,
                            XBinary::valueToHex(nSectionAlignment), nSectionAlignment & (nSectionAlignment - 1));
        _addCheckFormatTest(&listResult, &bSuccess, FMT_MSG_CODE_INVALID_SECTIONSTABLE, FMT_MSG_TYPE_ERROR, "OptionalHeader.SectionAlignment", nSectionAlignment,
                            XBinary::valueToHex(nSectionAlignment), nSectionAlignment < 0x1000);
    }

    if (bSuccess) {
        QList<XPE_DEF::IMAGE_SECTION_HEADER> list = getSectionHeaders(pPdStruct);

        qint32 nNumberOfSections = list.count();

        for (qint32 i = 0; (i < nNumberOfSections) && bSuccess && isPdStructNotCanceled(pPdStruct); i++) {
            XPE_DEF::IMAGE_SECTION_HEADER sectionHeader = list.at(i);

            _addCheckFormatTest(&listResult, &bSuccess, FMT_MSG_CODE_INVALID_SECTIONSTABLE, FMT_MSG_TYPE_ERROR,
                                QString("IMAGE_SECTION_HEADER[%1].PointerToRawData").arg(i + 1), sectionHeader.PointerToRawData,
                                XBinary::valueToHex(sectionHeader.PointerToRawData), sectionHeader.PointerToRawData > memoryMap.nBinarySize);
            _addCheckFormatTest(&listResult, &bSuccess, FMT_MSG_CODE_INVALID_SECTIONSTABLE, FMT_MSG_TYPE_WARNING,
                                QString("IMAGE_SECTION_HEADER[%1].PointerToRawData").arg(i + 1), sectionHeader.PointerToRawData,
                                XBinary::valueToHex(sectionHeader.PointerToRawData), sectionHeader.PointerToRawData % nFileAlignment);
            _addCheckFormatTest(&listResult, &bSuccess, FMT_MSG_CODE_INVALID_SECTIONSTABLE, FMT_MSG_TYPE_ERROR,
                                QString("IMAGE_SECTION_HEADER[%1].VirtualAddress").arg(i + 1), sectionHeader.VirtualAddress,
                                XBinary::valueToHex(sectionHeader.VirtualAddress), sectionHeader.VirtualAddress > memoryMap.nImageSize);
            _addCheckFormatTest(&listResult, &bSuccess, FMT_MSG_CODE_INVALID_SECTIONSTABLE, FMT_MSG_TYPE_WARNING,
                                QString("IMAGE_SECTION_HEADER[%1].VirtualAddress").arg(i + 1), sectionHeader.VirtualAddress,
                                XBinary::valueToHex(sectionHeader.VirtualAddress), sectionHeader.VirtualAddress % nSectionAlignment);
            _addCheckFormatTest(&listResult, &bSuccess, FMT_MSG_CODE_INVALID_SECTIONSTABLE, FMT_MSG_TYPE_ERROR,
                                QString("IMAGE_SECTION_HEADER[%1].SizeOfRawData").arg(i + 1), sectionHeader.SizeOfRawData,
                                XBinary::valueToHex(sectionHeader.SizeOfRawData), (sectionHeader.PointerToRawData + sectionHeader.SizeOfRawData) > memoryMap.nBinarySize);
            _addCheckFormatTest(&listResult, &bSuccess, FMT_MSG_CODE_INVALID_SECTIONSTABLE, FMT_MSG_TYPE_ERROR,
                                QString("IMAGE_SECTION_HEADER[%1].SizeOfRawData").arg(i + 1), sectionHeader.SizeOfRawData,
                                XBinary::valueToHex(sectionHeader.SizeOfRawData),
                                align_up(sectionHeader.SizeOfRawData, nFileAlignment) > align_up(sectionHeader.Misc.VirtualSize, nSectionAlignment));
            _addCheckFormatTest(&listResult, &bSuccess, FMT_MSG_CODE_INVALID_SECTIONSTABLE, FMT_MSG_TYPE_WARNING,
                                QString("IMAGE_SECTION_HEADER[%1].SizeOfRawData").arg(i + 1), sectionHeader.SizeOfRawData,
                                XBinary::valueToHex(sectionHeader.SizeOfRawData), sectionHeader.SizeOfRawData % nFileAlignment);
            _addCheckFormatTest(&listResult, &bSuccess, FMT_MSG_CODE_INVALID_SECTIONSTABLE, FMT_MSG_TYPE_ERROR,
                                QString("IMAGE_SECTION_HEADER[%1].VirtualSize").arg(i + 1), sectionHeader.Misc.VirtualSize,
                                XBinary::valueToHex(sectionHeader.Misc.VirtualSize),
                                (sectionHeader.VirtualAddress + sectionHeader.Misc.VirtualSize) > memoryMap.nImageSize);
        }
    }

    if (bSuccess) {
        QList<XPE_DEF::IMAGE_IMPORT_DESCRIPTOR> list = getImportDescriptors(&memoryMap);

        qint32 nNumberOfImports = list.count();

        for (qint32 i = 0; (i < nNumberOfImports) && bSuccess && isPdStructNotCanceled(pPdStruct); i++) {
            XPE_DEF::IMAGE_IMPORT_DESCRIPTOR importDescriptor = list.at(i);

            _addCheckFormatTest(&listResult, &bSuccess, FMT_MSG_CODE_INVALID_IMPORTTABLE, FMT_MSG_TYPE_WARNING,
                                QString("IMAGE_IMPORT_DESCRIPTOR[%1].OriginalFirstThunk").arg(i), importDescriptor.OriginalFirstThunk,
                                XBinary::valueToHex(importDescriptor.OriginalFirstThunk),
                                !(importDescriptor.OriginalFirstThunk) || !isRelAddressValid(&memoryMap, importDescriptor.OriginalFirstThunk));
            _addCheckFormatTest(&listResult, &bSuccess, FMT_MSG_CODE_INVALID_IMPORTTABLE, FMT_MSG_TYPE_ERROR, QString("IMAGE_IMPORT_DESCRIPTOR[%1].FirstThunk").arg(i),
                                importDescriptor.FirstThunk, XBinary::valueToHex(importDescriptor.FirstThunk),
                                !(importDescriptor.FirstThunk) || !isRelAddressValid(&memoryMap, importDescriptor.FirstThunk));
            _addCheckFormatTest(&listResult, &bSuccess, FMT_MSG_CODE_INVALID_IMPORTTABLE, FMT_MSG_TYPE_ERROR, QString("IMAGE_IMPORT_DESCRIPTOR[%1].FirstThunk").arg(i),
                                importDescriptor.Name, XBinary::valueToHex(importDescriptor.Name),
                                !(importDescriptor.Name) || !isRelAddressValid(&memoryMap, importDescriptor.Name));
        }
    }

    if (bSuccess) {
        if (isExportPresent()) {
            XPE_DEF::IMAGE_EXPORT_DIRECTORY ied = getExportDirectory();

            _addCheckFormatTest(&listResult, &bSuccess, FMT_MSG_CODE_INVALID_EXPORTTABLE, FMT_MSG_TYPE_ERROR, QString("IMAGE_EXPORT_DIRECTORY.AddressOfFunctions"),
                                ied.AddressOfFunctions, XBinary::valueToHex(ied.AddressOfFunctions),
                                !(ied.AddressOfFunctions) || !isRelAddressValid(&memoryMap, ied.AddressOfFunctions));
            _addCheckFormatTest(&listResult, &bSuccess, FMT_MSG_CODE_INVALID_EXPORTTABLE, FMT_MSG_TYPE_WARNING, QString("IMAGE_EXPORT_DIRECTORY.AddressOfNames"),
                                ied.AddressOfNames, XBinary::valueToHex(ied.AddressOfNames), !(ied.AddressOfNames) || !isRelAddressValid(&memoryMap, ied.AddressOfNames));
            _addCheckFormatTest(&listResult, &bSuccess, FMT_MSG_CODE_INVALID_EXPORTTABLE, FMT_MSG_TYPE_WARNING, QString("IMAGE_EXPORT_DIRECTORY.AddressOfNameOrdinals"),
                                ied.AddressOfNameOrdinals, XBinary::valueToHex(ied.AddressOfNameOrdinals),
                                !(ied.AddressOfNameOrdinals) || !isRelAddressValid(&memoryMap, ied.AddressOfNameOrdinals));
            _addCheckFormatTest(&listResult, &bSuccess, FMT_MSG_CODE_INVALID_EXPORTTABLE, FMT_MSG_TYPE_WARNING, QString("IMAGE_EXPORT_DIRECTORY.Name"), ied.Name,
                                XBinary::valueToHex(ied.Name), !(ied.Name) || !isRelAddressValid(&memoryMap, ied.Name));
        }
    }

    if (bSuccess) {
        if (isResourcesPresent()) {
            XPE::RESOURCE_HEADER rh = getResourceHeader(&memoryMap);

            // _addCheckFormatTest(&listResult, &bSuccess, FMT_MSG_CODE_INVALID_RESOURCESTABLE, FMT_MSG_TYPE_ERROR,
            // QString("IMAGE_RESOURCE_DIRECTORY.OriginalFirstThunk"), rh.directory., XBinary::valueToHex(ied.AddressOfFunctions), !(ied.AddressOfFunctions) ||
            // !isRelAddressValid(&memoryMap, ied.AddressOfFunctions));
        }
    }

    if (bSuccess) {
        if (isRelocsPresent()) {
            // QList<XPE_DEF::IMAGE_BASE_RELOCATION> list = getReloc(&memoryMap);

            // qint32 nNumberOfRelocs = list.count();

            // for (qint32 i = 0; (i < nNumberOfRelocs) && bSuccess && (!(pPdStruct->bIsStop)); i++) {
            //     XPE_DEF::IMAGE_BASE_RELOCATION reloc = list.at(i);

            //     _addCheckFormatTest(&listResult, &bSuccess, FMT_MSG_CODE_INVALID_RELOCSTABLE, FMT_MSG_TYPE_ERROR,
            //     QString("IMAGE_BASE_RELOCATION[%1].VirtualAddress").arg(i), reloc.VirtualAddress, XBinary::valueToHex(reloc.VirtualAddress),
            //     !isRelAddressValid(&memoryMap, reloc.VirtualAddress));
            // }
        }
    }

    return listResult;
}

XPE::XCERT_INFO XPE::getCertInfo(const QString &sFileName)
{
#ifndef _MSC_VER
    Q_UNUSED(sFileName)
#endif

    XPE::XCERT_INFO result = {};

#if defined(_MSC_VER)
    wchar_t wszFilePath[512] = {};

    if (XBinary::_toWCharArray(sFileName, wszFilePath)) {
        WINTRUST_FILE_INFO wintrustFileInfo = {};
        wintrustFileInfo.cbStruct = sizeof(WINTRUST_FILE_INFO);
        wintrustFileInfo.pcwszFilePath = wszFilePath;
        wintrustFileInfo.hFile = NULL;
        wintrustFileInfo.pgKnownSubject = NULL;

        GUID WVTPolicyGUID = WINTRUST_ACTION_GENERIC_VERIFY_V2;
        WINTRUST_DATA wintrustData = {};
        wintrustData.cbStruct = sizeof(wintrustData);
        wintrustData.pPolicyCallbackData = NULL;
        wintrustData.pSIPClientData = NULL;
        wintrustData.dwUIChoice = WTD_UI_NONE;
        wintrustData.fdwRevocationChecks = WTD_REVOKE_NONE;
        wintrustData.dwUnionChoice = WTD_CHOICE_FILE;
        wintrustData.dwStateAction = WTD_STATEACTION_VERIFY;
        wintrustData.hWVTStateData = NULL;
        wintrustData.pwszURLReference = NULL;
        wintrustData.dwUIContext = 0;
        wintrustData.pFile = &wintrustFileInfo;

        LONG lStatus = WinVerifyTrust(NULL, &WVTPolicyGUID, &wintrustData);

        if (lStatus == ERROR_SUCCESS) {
            result.bIsValid = true;
            result.sStatus = tr("The file is signed and the signature was verified");
        } else if (lStatus == TRUST_E_NOSIGNATURE) {
            result.sStatus = tr("The file is not signed");
        } else if (lStatus == TRUST_E_EXPLICIT_DISTRUST) {
            result.sStatus = tr("The signature is present, but specifically disallowed");
        } else if (lStatus == TRUST_E_SUBJECT_NOT_TRUSTED) {
            result.sStatus = tr("The signature is present, but not trusted");
        } else if (lStatus == CRYPT_E_SECURITY_SETTINGS) {
            result.sStatus = tr("The signature error");
        } else {
            result.sStatus = QString("%1: %2").arg(tr("Error")).arg(valueToHex((quint32)lStatus));
        }

        HCERTSTORE hStore = NULL;
        HCRYPTMSG hMsg = NULL;
        DWORD dwEncoding = 0;
        DWORD dwContentType = 0;
        DWORD dwFormatType = 0;
        DWORD dwSignerInfo = 0;

        if (CryptQueryObject(CERT_QUERY_OBJECT_FILE, wszFilePath, CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED, CERT_QUERY_FORMAT_FLAG_BINARY, 0, &dwEncoding,
                             &dwContentType, &dwFormatType, &hStore, &hMsg, NULL)) {
            if (CryptMsgGetParam(hMsg, CMSG_SIGNER_INFO_PARAM, 0, NULL, &dwSignerInfo)) {
                char *_pSignerInfo = new char[dwSignerInfo];

                if (CryptMsgGetParam(hMsg, CMSG_SIGNER_INFO_PARAM, 0, (PVOID)_pSignerInfo, &dwSignerInfo)) {
                    if (dwSignerInfo >= sizeof(CMSG_SIGNER_INFO)) {
                        CMSG_SIGNER_INFO *pSignerInfo = (CMSG_SIGNER_INFO *)_pSignerInfo;

                        for (DWORD n = 0; n < pSignerInfo->AuthAttrs.cAttr; n++) {
                            //                            qDebug("%s",pSignerInfo->AuthAttrs.rgAttr[n].pszObjId);

                            if (QString(pSignerInfo->AuthAttrs.rgAttr[n].pszObjId) == QString(SPC_SP_OPUS_INFO_OBJID)) {
                                DWORD dwOpusInfo = 0;

                                if (CryptDecodeObject(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, SPC_SP_OPUS_INFO_OBJID, pSignerInfo->AuthAttrs.rgAttr[n].rgValue[0].pbData,
                                                      pSignerInfo->AuthAttrs.rgAttr[n].rgValue[0].cbData, 0, NULL, &dwOpusInfo)) {
                                    char *_pOpusInfo = new char[dwOpusInfo];

                                    if (CryptDecodeObject(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, SPC_SP_OPUS_INFO_OBJID,
                                                          pSignerInfo->AuthAttrs.rgAttr[n].rgValue[0].pbData, pSignerInfo->AuthAttrs.rgAttr[n].rgValue[0].cbData, 0,
                                                          (PVOID)_pOpusInfo, &dwOpusInfo)) {
                                        SPC_SP_OPUS_INFO *pOpusInfo = (SPC_SP_OPUS_INFO *)_pOpusInfo;

                                        result.sProgramName = XBinary::_fromWCharArray(pOpusInfo->pwszProgramName);

                                        if (pOpusInfo->pPublisherInfo) {
                                            if (pOpusInfo->pPublisherInfo->dwLinkChoice == SPC_URL_LINK_CHOICE) {
                                                result.sPublisher = XBinary::_fromWCharArray(pOpusInfo->pPublisherInfo->pwszUrl);
                                            } else if (pOpusInfo->pPublisherInfo->dwLinkChoice == SPC_FILE_LINK_CHOICE) {
                                                result.sPublisher = XBinary::_fromWCharArray(pOpusInfo->pPublisherInfo->pwszFile);
                                            }
                                        }

                                        if (pOpusInfo->pMoreInfo) {
                                            if (pOpusInfo->pMoreInfo->dwLinkChoice == SPC_URL_LINK_CHOICE) {
                                                result.sMoreInfo = XBinary::_fromWCharArray(pOpusInfo->pMoreInfo->pwszUrl);
                                            } else if (pOpusInfo->pMoreInfo->dwLinkChoice == SPC_FILE_LINK_CHOICE) {
                                                result.sMoreInfo = XBinary::_fromWCharArray(pOpusInfo->pMoreInfo->pwszFile);
                                            }
                                        }
                                    }

                                    CERT_INFO CertInfo = {};

                                    CertInfo.Issuer = pSignerInfo->Issuer;
                                    CertInfo.SerialNumber = pSignerInfo->SerialNumber;

                                    PCCERT_CONTEXT pCertContext =
                                        CertFindCertificateInStore(hStore, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 0, CERT_FIND_SUBJECT_CERT, (PVOID)&CertInfo, NULL);

                                    if (pCertContext) {
                                        DWORD dwData = pCertContext->pCertInfo->SerialNumber.cbData;
                                        for (DWORD n = 0; n < dwData; n++) {
                                            result.sSerialNumber.append(
                                                QString("%1 ").arg(XBinary::valueToHex(pCertContext->pCertInfo->SerialNumber.pbData[dwData - (n + 1)])));
                                        }

                                        result.sIssuer = getCertNameString(pCertContext, CERTNAMESTRING_ISSUER);
                                        result.sSubject = getCertNameString(pCertContext, CERTNAMESTRING_SUBJECT);

                                        CertFreeCertificateContext(pCertContext);
                                    }

                                    delete[] _pOpusInfo;
                                }
                            }
                        }

                        for (DWORD n = 0; n < pSignerInfo->UnauthAttrs.cAttr; n++) {
                            qDebug("%s", pSignerInfo->UnauthAttrs.rgAttr[n].pszObjId);

                            if (QString(pSignerInfo->UnauthAttrs.rgAttr[n].pszObjId) == QString(szOID_RSA_counterSign))
                            //                            if(QString(pSignerInfo->UnauthAttrs.rgAttr[n].pszObjId)==QString(szOID_RFC3161_counterSign))
                            {
                                DWORD dwCounterSignerInfo = 0;

                                if (CryptDecodeObject(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, PKCS7_SIGNER_INFO, pSignerInfo->UnauthAttrs.rgAttr[n].rgValue[0].pbData,
                                                      pSignerInfo->UnauthAttrs.rgAttr[n].rgValue[0].cbData, 0, NULL, &dwCounterSignerInfo)) {
                                    char *_pCounterSignerInfo = new char[dwCounterSignerInfo];

                                    if (CryptDecodeObject(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, PKCS7_SIGNER_INFO,
                                                          pSignerInfo->UnauthAttrs.rgAttr[n].rgValue[0].pbData, pSignerInfo->UnauthAttrs.rgAttr[n].rgValue[0].cbData, 0,
                                                          (PVOID)_pCounterSignerInfo, &dwCounterSignerInfo)) {
                                        CMSG_SIGNER_INFO *pCounterSignerInfo = (CMSG_SIGNER_INFO *)_pCounterSignerInfo;

                                        CERT_INFO CertInfo = {};

                                        CertInfo.Issuer = pCounterSignerInfo->Issuer;
                                        CertInfo.SerialNumber = pCounterSignerInfo->SerialNumber;

                                        PCCERT_CONTEXT pCertContext = CertFindCertificateInStore(hStore, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 0,
                                                                                                 CERT_FIND_SUBJECT_CERT, (PVOID)&CertInfo, NULL);

                                        if (pCertContext) {
                                            DWORD dwData = pCertContext->pCertInfo->SerialNumber.cbData;
                                            for (DWORD n = 0; n < dwData; n++) {
                                                result.sTSSerialNumber.append(
                                                    QString("%1 ").arg(XBinary::valueToHex(pCertContext->pCertInfo->SerialNumber.pbData[dwData - (n + 1)])));
                                            }

                                            result.sTSIssuer = getCertNameString(pCertContext, CERTNAMESTRING_ISSUER);
                                            result.sTSSubject = getCertNameString(pCertContext, CERTNAMESTRING_SUBJECT);

                                            CertFreeCertificateContext(pCertContext);
                                        }
                                    }

                                    delete[] _pCounterSignerInfo;
                                }
                                //                                else
                                //                                {
                                //                                    qDebug("GetLastError:
                                //                                    %X",GetLastError());
                                //                                }
                            }
                        }
                    }
                }

                delete[] _pSignerInfo;
            }
        }

        if (hMsg) {
            CryptMsgClose(hMsg);
        }

        if (hStore) {
            CertCloseStore(hStore, 0);
        }

        wintrustData.dwStateAction = WTD_STATEACTION_CLOSE;

        lStatus = WinVerifyTrust(NULL, &WVTPolicyGUID, &wintrustData);
    }
#endif

    return result;
}

// QList<XBinary::HREGION> XPE::getHData(PDSTRUCT *pPdStruct)
// {
//     QList<XBinary::HREGION> listResult;

//     _MEMORY_MAP memoryMap = getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);

//     {
//         HREGION region = {};
//         region.sGUID = generateUUID();
//         region.nVirtualAddress = memoryMap.nEntryPointAddress;
//         region.nFileOffset = addressToOffset(&memoryMap, region.nVirtualAddress);
//         region.nFileSize = 1;
//         region.nVirtualSize = region.nFileSize;
//         region.sName = tr("Entry point");

//         listResult.append(region);
//     }
//     {
//         XPE_DEF::IMAGE_DATA_DIRECTORY dataDirectory = getOptionalHeader_DataDirectory(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXPORT);

//         if (isDataDirectoryValid(&dataDirectory, &memoryMap)) {
//             HREGION region = {};
//             region.sGUID = generateUUID();
//             region.nVirtualAddress = dataDirectory.VirtualAddress;
//             region.nFileOffset = relAddressToOffset(&memoryMap, region.nVirtualAddress);
//             region.nFileSize = dataDirectory.Size;
//             region.nVirtualSize = region.nFileSize;
//             region.sName = tr("Export");

//             listResult.append(region);
//         }
//     }
//     {
//         XPE_DEF::IMAGE_DATA_DIRECTORY dataDirectory = getOptionalHeader_DataDirectory(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_IMPORT);

//         if (isDataDirectoryValid(&dataDirectory, &memoryMap)) {
//             HREGION region = {};
//             region.sGUID = generateUUID();
//             region.nVirtualAddress = dataDirectory.VirtualAddress;
//             region.nFileOffset = relAddressToOffset(&memoryMap, region.nVirtualAddress);
//             region.nFileSize = dataDirectory.Size;
//             region.nVirtualSize = region.nFileSize;
//             region.sName = tr("Import");

//             listResult.append(region);
//         }
//     }
//     {
//         XPE_DEF::IMAGE_DATA_DIRECTORY dataDirectory = getOptionalHeader_DataDirectory(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_BASERELOC);

//         if (isDataDirectoryValid(&dataDirectory, &memoryMap)) {
//             HREGION region = {};
//             region.sGUID = generateUUID();
//             region.nVirtualAddress = dataDirectory.VirtualAddress;
//             region.nFileOffset = relAddressToOffset(&memoryMap, region.nVirtualAddress);
//             region.nFileSize = dataDirectory.Size;
//             region.nVirtualSize = region.nFileSize;
//             region.sName = tr("Relocs");

//             listResult.append(region);
//         }
//     }
//     {
//         XPE_DEF::IMAGE_DATA_DIRECTORY dataDirectory = getOptionalHeader_DataDirectory(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_RESOURCE);

//         if (isDataDirectoryValid(&dataDirectory, &memoryMap)) {
//             HREGION region = {};
//             region.sGUID = generateUUID();
//             region.nVirtualAddress = dataDirectory.VirtualAddress;
//             region.nFileOffset = relAddressToOffset(&memoryMap, region.nVirtualAddress);
//             region.nFileSize = dataDirectory.Size;
//             region.nVirtualSize = region.nFileSize;
//             region.sName = tr("Resources");

//             listResult.append(region);
//         }
//     }
//     {
//         XPE_DEF::IMAGE_DATA_DIRECTORY dataDirectory = getOptionalHeader_DataDirectory(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_TLS);

//         if (isDataDirectoryValid(&dataDirectory, &memoryMap)) {
//             HREGION region = {};
//             region.sGUID = generateUUID();
//             region.nVirtualAddress = dataDirectory.VirtualAddress;
//             region.nFileOffset = relAddressToOffset(&memoryMap, region.nVirtualAddress);
//             region.nFileSize = dataDirectory.Size;
//             region.nVirtualSize = region.nFileSize;
//             region.sName = QString("TLS");

//             listResult.append(region);
//         }
//     }
//     {
//         XPE_DEF::IMAGE_DATA_DIRECTORY dataDirectory = getOptionalHeader_DataDirectory(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

//         if (isDataDirectoryValid(&dataDirectory, &memoryMap)) {
//             HREGION region = {};
//             region.sGUID = generateUUID();
//             region.nVirtualAddress = dataDirectory.VirtualAddress;
//             region.nFileOffset = relAddressToOffset(&memoryMap, region.nVirtualAddress);
//             region.nFileSize = dataDirectory.Size;
//             region.nVirtualSize = region.nFileSize;
//             region.sName = QString(".NET");

//             listResult.append(region);
//         }
//     }
//     {
//         XPE_DEF::IMAGE_DATA_DIRECTORY dataDirectory = getOptionalHeader_DataDirectory(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_SECURITY);

//         if (dataDirectory.VirtualAddress && isOffsetValid(&memoryMap, dataDirectory.VirtualAddress)) {
//             HREGION region = {};
//             region.sGUID = generateUUID();
//             region.nVirtualAddress = -1;
//             region.nFileOffset = dataDirectory.VirtualAddress;
//             region.nFileSize = dataDirectory.Size;
//             region.nVirtualSize = region.nFileSize;
//             region.sName = tr("Certificate");

//             listResult.append(region);
//         }
//     }

//     return listResult;
// }
#if defined(_MSC_VER)
QString XPE::getCertNameString(PCCERT_CONTEXT pCertContext, CERTNAMESTRING certNameString)
{
    QString sResult;

    DWORD dwType = 0;
    DWORD dwFlags = 0;

    if (certNameString == CERTNAMESTRING_ISSUER) {
        dwType = CERT_NAME_SIMPLE_DISPLAY_TYPE;
        dwFlags = CERT_NAME_ISSUER_FLAG;
    } else if (certNameString == CERTNAMESTRING_SUBJECT) {
        dwType = CERT_NAME_SIMPLE_DISPLAY_TYPE;
        dwFlags = 0;
    }

    DWORD dwData = CertGetNameStringW(pCertContext, dwType, dwFlags, NULL, NULL, 0);

    if (dwData) {
        char *_pBuffer = new char[dwData * sizeof(TCHAR)];

        if (CertGetNameStringW(pCertContext, dwType, dwFlags, NULL, (LPWSTR)_pBuffer, dwData)) {
            sResult = XBinary::_fromWCharArray((wchar_t *)_pBuffer);
        }

        delete[] _pBuffer;
    }

    return sResult;
}
#endif
quint64 XPE::getImageFileHeader(XPE_DEF::IMAGE_FILE_HEADER *pHeader, const QString &sString)
{
    quint64 nResult = 0;

    if (sString == "Machine") nResult = pHeader->Machine;
    else if (sString == "NumberOfSections") nResult = pHeader->NumberOfSections;
    else if (sString == "TimeDateStamp") nResult = pHeader->TimeDateStamp;
    else if (sString == "PointerToSymbolTable") nResult = pHeader->PointerToSymbolTable;
    else if (sString == "NumberOfSymbols") nResult = pHeader->NumberOfSymbols;
    else if (sString == "SizeOfOptionalHeader") nResult = pHeader->SizeOfOptionalHeader;
    else if (sString == "Characteristics") nResult = pHeader->Characteristics;
    else {
        emit errorMessage(QString("%1: %2").arg(tr("Invalid")).arg(sString));
    }

    return nResult;
}

quint64 XPE::getImageOptionalHeader32(XPE_DEF::IMAGE_OPTIONAL_HEADER32 *pHeader, const QString &sString)
{
    quint64 nResult = 0;

    if (sString == "Magic") nResult = pHeader->Magic;
    else if (sString == "MajorLinkerVersion") nResult = pHeader->MajorLinkerVersion;
    else if (sString == "MinorLinkerVersion") nResult = pHeader->MinorLinkerVersion;
    else if (sString == "SizeOfCode") nResult = pHeader->SizeOfCode;
    else if (sString == "SizeOfInitializedData") nResult = pHeader->SizeOfInitializedData;
    else if (sString == "SizeOfUninitializedData") nResult = pHeader->SizeOfUninitializedData;
    else if (sString == "AddressOfEntryPoint") nResult = pHeader->AddressOfEntryPoint;
    else if (sString == "BaseOfCode") nResult = pHeader->BaseOfCode;
    else if (sString == "BaseOfData") nResult = pHeader->BaseOfData;
    else if (sString == "ImageBase") nResult = pHeader->ImageBase;
    else if (sString == "SectionAlignment") nResult = (pHeader->SectionAlignment == 0) ? 1 : pHeader->SectionAlignment;
    else if (sString == "FileAlignment") nResult = (pHeader->FileAlignment == 0) ? 1 : pHeader->FileAlignment;
    else if (sString == "MajorOperatingSystemVersion") nResult = pHeader->MajorOperatingSystemVersion;
    else if (sString == "MinorOperatingSystemVersion") nResult = pHeader->MinorOperatingSystemVersion;
    else if (sString == "MajorImageVersion") nResult = pHeader->MajorImageVersion;
    else if (sString == "MinorImageVersion") nResult = pHeader->MinorImageVersion;
    else if (sString == "MajorSubsystemVersion") nResult = pHeader->MajorSubsystemVersion;
    else if (sString == "MinorSubsystemVersion") nResult = pHeader->MinorSubsystemVersion;
    else if (sString == "Win32VersionValue") nResult = pHeader->Win32VersionValue;
    else if (sString == "SizeOfImage") nResult = pHeader->SizeOfImage;
    else if (sString == "SizeOfHeaders") nResult = pHeader->SizeOfHeaders;
    else if (sString == "CheckSum") nResult = pHeader->CheckSum;
    else if (sString == "Subsystem") nResult = pHeader->Subsystem;
    else if (sString == "DllCharacteristics") nResult = pHeader->DllCharacteristics;
    else if (sString == "SizeOfStackReserve") nResult = pHeader->SizeOfStackReserve;
    else if (sString == "SizeOfStackCommit") nResult = pHeader->SizeOfStackCommit;
    else if (sString == "SizeOfHeapReserve") nResult = pHeader->SizeOfHeapReserve;
    else if (sString == "SizeOfHeapCommit") nResult = pHeader->SizeOfHeapCommit;
    else if (sString == "LoaderFlags") nResult = pHeader->LoaderFlags;
    else if (sString == "NumberOfRvaAndSizes") nResult = pHeader->NumberOfRvaAndSizes;
    else {
        emit errorMessage(QString("%1: %2").arg(tr("Invalid")).arg(sString));
    }

    return nResult;
}

quint64 XPE::getImageOptionalHeader64(XPE_DEF::IMAGE_OPTIONAL_HEADER64 *pHeader, QString sString)
{
    quint64 nResult = 0;

    if (sString == "Magic") nResult = pHeader->Magic;
    else if (sString == "MajorLinkerVersion") nResult = pHeader->MajorLinkerVersion;
    else if (sString == "MinorLinkerVersion") nResult = pHeader->MinorLinkerVersion;
    else if (sString == "SizeOfCode") nResult = pHeader->SizeOfCode;
    else if (sString == "SizeOfInitializedData") nResult = pHeader->SizeOfInitializedData;
    else if (sString == "SizeOfUninitializedData") nResult = pHeader->SizeOfUninitializedData;
    else if (sString == "AddressOfEntryPoint") nResult = pHeader->AddressOfEntryPoint;
    else if (sString == "BaseOfCode") nResult = pHeader->BaseOfCode;
    else if (sString == "BaseOfData") nResult = 0;
    else if (sString == "ImageBase") nResult = pHeader->ImageBase;
    else if (sString == "SectionAlignment") nResult = (pHeader->SectionAlignment == 0) ? 1 : pHeader->SectionAlignment;
    else if (sString == "FileAlignment") nResult = (pHeader->FileAlignment == 0) ? 1 : pHeader->FileAlignment;
    else if (sString == "MajorOperatingSystemVersion") nResult = pHeader->MajorOperatingSystemVersion;
    else if (sString == "MinorOperatingSystemVersion") nResult = pHeader->MinorOperatingSystemVersion;
    else if (sString == "MajorImageVersion") nResult = pHeader->MajorImageVersion;
    else if (sString == "MinorImageVersion") nResult = pHeader->MinorImageVersion;
    else if (sString == "MajorSubsystemVersion") nResult = pHeader->MajorSubsystemVersion;
    else if (sString == "MinorSubsystemVersion") nResult = pHeader->MinorSubsystemVersion;
    else if (sString == "Win32VersionValue") nResult = pHeader->Win32VersionValue;
    else if (sString == "SizeOfImage") nResult = pHeader->SizeOfImage;
    else if (sString == "SizeOfHeaders") nResult = pHeader->SizeOfHeaders;
    else if (sString == "CheckSum") nResult = pHeader->CheckSum;
    else if (sString == "Subsystem") nResult = pHeader->Subsystem;
    else if (sString == "DllCharacteristics") nResult = pHeader->DllCharacteristics;
    else if (sString == "SizeOfStackReserve") nResult = pHeader->SizeOfStackReserve;
    else if (sString == "SizeOfStackCommit") nResult = pHeader->SizeOfStackCommit;
    else if (sString == "SizeOfHeapReserve") nResult = pHeader->SizeOfHeapReserve;
    else if (sString == "SizeOfHeapCommit") nResult = pHeader->SizeOfHeapCommit;
    else if (sString == "LoaderFlags") nResult = pHeader->LoaderFlags;
    else if (sString == "NumberOfRvaAndSizes") nResult = pHeader->NumberOfRvaAndSizes;
    else {
        emit errorMessage(QString("%1: %2").arg(tr("Invalid")).arg(sString));
    }

    return nResult;
}

QString XPE::structIDToString(quint32 nID)
{
    return XBinary::XCONVERT_idToTransString(nID, _TABLE_XPE_STRUCTID, sizeof(_TABLE_XPE_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QString XPE::structIDToFtString(quint32 nID)
{
    return XBinary::XCONVERT_idToFtString(nID, _TABLE_XPE_STRUCTID, sizeof(_TABLE_XPE_STRUCTID) / sizeof(XBinary::XCONVERT));
}

quint32 XPE::ftStringToStructID(const QString &sFtString)
{
    return XCONVERT_ftStringToId(sFtString, _TABLE_XPE_STRUCTID, sizeof(_TABLE_XPE_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QList<XBinary::XFRECORD> XPE::getXFRecords(FT fileType, quint32 nStructID, const XLOC &xLoc)
{
    Q_UNUSED(fileType)

    QList<XBinary::XFRECORD> listResult;

    if ((nStructID == STRUCTID_IMAGE_NT_HEADERS32) || (nStructID == STRUCTID_IMAGE_NT_HEADERS64)) {
        listResult.append({"Signature", 0, 4, XFRECORD_FLAG_NONE, VT_UINT32});
    } else if (nStructID == STRUCTID_IMAGE_FILE_HEADER) {
        listResult.append({"Machine", (qint32)offsetof(XPE_DEF::IMAGE_FILE_HEADER, Machine), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"NumberOfSections", (qint32)offsetof(XPE_DEF::IMAGE_FILE_HEADER, NumberOfSections), 2, XFRECORD_FLAG_COUNT, VT_UINT16});
        listResult.append({"TimeDateStamp", (qint32)offsetof(XPE_DEF::IMAGE_FILE_HEADER, TimeDateStamp), 4, XFRECORD_FLAG_UNIXTIME, VT_UINT32});
        listResult.append({"PointerToSymbolTable", (qint32)offsetof(XPE_DEF::IMAGE_FILE_HEADER, PointerToSymbolTable), 4, XFRECORD_FLAG_OFFSET, VT_UINT32});
        listResult.append({"NumberOfSymbols", (qint32)offsetof(XPE_DEF::IMAGE_FILE_HEADER, NumberOfSymbols), 4, XFRECORD_FLAG_COUNT, VT_UINT32});
        listResult.append({"SizeOfOptionalHeader", (qint32)offsetof(XPE_DEF::IMAGE_FILE_HEADER, SizeOfOptionalHeader), 2, XFRECORD_FLAG_SIZE, VT_UINT16});
        listResult.append({"Characteristics", (qint32)offsetof(XPE_DEF::IMAGE_FILE_HEADER, Characteristics), 2, XFRECORD_FLAG_NONE, VT_UINT16});
    } else if (nStructID == STRUCTID_IMAGE_OPTIONAL_HEADER32) {
        listResult.append({"Magic", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S, Magic), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"MajorLinkerVersion", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S, MajorLinkerVersion), 1, XFRECORD_FLAG_VERSION_MAJOR, VT_UINT8});
        listResult.append({"MinorLinkerVersion", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S, MinorLinkerVersion), 1, XFRECORD_FLAG_VERSION_MINOR, VT_UINT8});
        listResult.append({"SizeOfCode", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S, SizeOfCode), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"SizeOfInitializedData", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S, SizeOfInitializedData), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"SizeOfUninitializedData", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S, SizeOfUninitializedData), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append(
            {"AddressOfEntryPoint", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S, AddressOfEntryPoint), 4, XFRECORD_FLAG_RELATIVE_ADDRESS, VT_UINT32});
        listResult.append({"BaseOfCode", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S, BaseOfCode), 4, XFRECORD_FLAG_RELATIVE_ADDRESS, VT_UINT32});
        listResult.append({"BaseOfData", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S, BaseOfData), 4, XFRECORD_FLAG_RELATIVE_ADDRESS, VT_UINT32});
        listResult.append({"ImageBase", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S, ImageBase), 4, XFRECORD_FLAG_ADDRESS, VT_UINT32});
        listResult.append({"SectionAlignment", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S, SectionAlignment), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"FileAlignment", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S, FileAlignment), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append(
            {"MajorOperatingSystemVersion", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S, MajorOperatingSystemVersion), 2, XFRECORD_FLAG_VERSION_MAJOR, VT_UINT16});
        listResult.append(
            {"MinorOperatingSystemVersion", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S, MinorOperatingSystemVersion), 2, XFRECORD_FLAG_VERSION_MINOR, VT_UINT16});
        listResult.append({"MajorImageVersion", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S, MajorImageVersion), 2, XFRECORD_FLAG_VERSION_MAJOR, VT_UINT16});
        listResult.append({"MinorImageVersion", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S, MinorImageVersion), 2, XFRECORD_FLAG_VERSION_MINOR, VT_UINT16});
        listResult.append(
            {"MajorSubsystemVersion", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S, MajorSubsystemVersion), 2, XFRECORD_FLAG_VERSION_MAJOR, VT_UINT16});
        listResult.append(
            {"MinorSubsystemVersion", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S, MinorSubsystemVersion), 2, XFRECORD_FLAG_VERSION_MINOR, VT_UINT16});
        listResult.append({"Win32VersionValue", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S, Win32VersionValue), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"SizeOfImage", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S, SizeOfImage), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"SizeOfHeaders", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S, SizeOfHeaders), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"CheckSum", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S, CheckSum), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"Subsystem", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S, Subsystem), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"DllCharacteristics", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S, DllCharacteristics), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"SizeOfStackReserve", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S, SizeOfStackReserve), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"SizeOfStackCommit", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S, SizeOfStackCommit), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"SizeOfHeapReserve", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S, SizeOfHeapReserve), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"SizeOfHeapCommit", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S, SizeOfHeapCommit), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"LoaderFlags", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S, LoaderFlags), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"NumberOfRvaAndSizes", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S, NumberOfRvaAndSizes), 4, XFRECORD_FLAG_COUNT, VT_UINT32});
    } else if (nStructID == STRUCTID_IMAGE_OPTIONAL_HEADER64) {
        listResult.append({"Magic", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S, Magic), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"MajorLinkerVersion", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S, MajorLinkerVersion), 1, XFRECORD_FLAG_VERSION_MAJOR, VT_UINT8});
        listResult.append({"MinorLinkerVersion", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S, MinorLinkerVersion), 1, XFRECORD_FLAG_VERSION_MINOR, VT_UINT8});
        listResult.append({"SizeOfCode", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S, SizeOfCode), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"SizeOfInitializedData", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S, SizeOfInitializedData), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"SizeOfUninitializedData", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S, SizeOfUninitializedData), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append(
            {"AddressOfEntryPoint", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S, AddressOfEntryPoint), 4, XFRECORD_FLAG_RELATIVE_ADDRESS, VT_UINT32});
        listResult.append({"BaseOfCode", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S, BaseOfCode), 4, XFRECORD_FLAG_RELATIVE_ADDRESS, VT_UINT32});
        listResult.append({"ImageBase", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S, ImageBase), 8, XFRECORD_FLAG_ADDRESS, VT_UINT64});
        listResult.append({"SectionAlignment", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S, SectionAlignment), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"FileAlignment", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S, FileAlignment), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append(
            {"MajorOperatingSystemVersion", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S, MajorOperatingSystemVersion), 2, XFRECORD_FLAG_VERSION_MAJOR, VT_UINT16});
        listResult.append(
            {"MinorOperatingSystemVersion", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S, MinorOperatingSystemVersion), 2, XFRECORD_FLAG_VERSION_MINOR, VT_UINT16});
        listResult.append({"MajorImageVersion", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S, MajorImageVersion), 2, XFRECORD_FLAG_VERSION_MAJOR, VT_UINT16});
        listResult.append({"MinorImageVersion", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S, MinorImageVersion), 2, XFRECORD_FLAG_VERSION_MINOR, VT_UINT16});
        listResult.append(
            {"MajorSubsystemVersion", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S, MajorSubsystemVersion), 2, XFRECORD_FLAG_VERSION_MAJOR, VT_UINT16});
        listResult.append(
            {"MinorSubsystemVersion", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S, MinorSubsystemVersion), 2, XFRECORD_FLAG_VERSION_MINOR, VT_UINT16});
        listResult.append({"Win32VersionValue", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S, Win32VersionValue), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"SizeOfImage", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S, SizeOfImage), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"SizeOfHeaders", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S, SizeOfHeaders), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"CheckSum", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S, CheckSum), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"Subsystem", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S, Subsystem), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"DllCharacteristics", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S, DllCharacteristics), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"SizeOfStackReserve", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S, SizeOfStackReserve), 8, XFRECORD_FLAG_SIZE, VT_UINT64});
        listResult.append({"SizeOfStackCommit", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S, SizeOfStackCommit), 8, XFRECORD_FLAG_SIZE, VT_UINT64});
        listResult.append({"SizeOfHeapReserve", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S, SizeOfHeapReserve), 8, XFRECORD_FLAG_SIZE, VT_UINT64});
        listResult.append({"SizeOfHeapCommit", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S, SizeOfHeapCommit), 8, XFRECORD_FLAG_SIZE, VT_UINT64});
        listResult.append({"LoaderFlags", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S, LoaderFlags), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"NumberOfRvaAndSizes", (qint32)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S, NumberOfRvaAndSizes), 4, XFRECORD_FLAG_COUNT, VT_UINT32});
    } else if (nStructID == STRUCTID_IMAGE_SECTION_HEADER) {
        listResult.append({"Name", (qint32)offsetof(XPE_DEF::IMAGE_SECTION_HEADER, Name), 8, XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
        listResult.append({"VirtualSize", (qint32)offsetof(XPE_DEF::IMAGE_SECTION_HEADER, Misc), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"VirtualAddress", (qint32)offsetof(XPE_DEF::IMAGE_SECTION_HEADER, VirtualAddress), 4, XFRECORD_FLAG_RELATIVE_ADDRESS, VT_UINT32});
        listResult.append({"SizeOfRawData", (qint32)offsetof(XPE_DEF::IMAGE_SECTION_HEADER, SizeOfRawData), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"PointerToRawData", (qint32)offsetof(XPE_DEF::IMAGE_SECTION_HEADER, PointerToRawData), 4, XFRECORD_FLAG_OFFSET, VT_UINT32});
        listResult.append({"PointerToRelocations", (qint32)offsetof(XPE_DEF::IMAGE_SECTION_HEADER, PointerToRelocations), 4, XFRECORD_FLAG_OFFSET, VT_UINT32});
        listResult.append({"PointerToLinenumbers", (qint32)offsetof(XPE_DEF::IMAGE_SECTION_HEADER, PointerToLinenumbers), 4, XFRECORD_FLAG_OFFSET, VT_UINT32});
        listResult.append({"NumberOfRelocations", (qint32)offsetof(XPE_DEF::IMAGE_SECTION_HEADER, NumberOfRelocations), 2, XFRECORD_FLAG_COUNT, VT_UINT16});
        listResult.append({"NumberOfLinenumbers", (qint32)offsetof(XPE_DEF::IMAGE_SECTION_HEADER, NumberOfLinenumbers), 2, XFRECORD_FLAG_COUNT, VT_UINT16});
        listResult.append({"Characteristics", (qint32)offsetof(XPE_DEF::IMAGE_SECTION_HEADER, Characteristics), 4, XFRECORD_FLAG_NONE, VT_UINT32});
    } else if (nStructID == STRUCTID_IMAGE_DATA_DIRECTORY) {
        // The SECURITY entry stores a raw file offset while all other entries store RVAs.
        // Keep the shared table field neutral and expose the per-row interpretation in extra columns.
        listResult.append({"VirtualAddress", (qint32)offsetof(XPE_DEF::IMAGE_DATA_DIRECTORY, VirtualAddress), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"Size", (qint32)offsetof(XPE_DEF::IMAGE_DATA_DIRECTORY, Size), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
    } else if (nStructID == STRUCTID_IMAGE_EXPORT_FUNCTION) {
        listResult.append({"RVA", 0, 4, XFRECORD_FLAG_RELATIVE_ADDRESS, VT_UINT32});
    } else if (nStructID == STRUCTID_IMAGE_EXPORT_DIRECTORY) {
        listResult.append({"Characteristics", (qint32)offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, Characteristics), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"TimeDateStamp", (qint32)offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, TimeDateStamp), 4, XFRECORD_FLAG_UNIXTIME, VT_UINT32});
        listResult.append({"MajorVersion", (qint32)offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, MajorVersion), 2, XFRECORD_FLAG_VERSION_MAJOR, VT_UINT16});
        listResult.append({"MinorVersion", (qint32)offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, MinorVersion), 2, XFRECORD_FLAG_VERSION_MINOR, VT_UINT16});
        listResult.append(
            {"Name", (qint32)offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, Name), 4, XFRECORD_FLAG_RELATIVE_ADDRESS | XFRECORD_FLAG_RELATIVE_ADDRESS_STRING, VT_UINT32});
        listResult.append({"Base", (qint32)offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, Base), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"NumberOfFunctions", (qint32)offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, NumberOfFunctions), 4, XFRECORD_FLAG_COUNT, VT_UINT32});
        listResult.append({"NumberOfNames", (qint32)offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, NumberOfNames), 4, XFRECORD_FLAG_COUNT, VT_UINT32});
        listResult.append({"AddressOfFunctions", (qint32)offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, AddressOfFunctions), 4, XFRECORD_FLAG_RELATIVE_ADDRESS, VT_UINT32});
        listResult.append({"AddressOfNames", (qint32)offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, AddressOfNames), 4, XFRECORD_FLAG_RELATIVE_ADDRESS, VT_UINT32});
        listResult.append(
            {"AddressOfNameOrdinals", (qint32)offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, AddressOfNameOrdinals), 4, XFRECORD_FLAG_RELATIVE_ADDRESS, VT_UINT32});
    } else if (nStructID == STRUCTID_IMAGE_IMPORT_DESCRIPTOR) {
        listResult.append({"OriginalFirstThunk", (qint32)offsetof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR, OriginalFirstThunk), 4, XFRECORD_FLAG_RELATIVE_ADDRESS, VT_UINT32});
        listResult.append({"TimeDateStamp", (qint32)offsetof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR, TimeDateStamp), 4, XFRECORD_FLAG_UNIXTIME, VT_UINT32});
        listResult.append({"ForwarderChain", (qint32)offsetof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR, ForwarderChain), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append(
            {"Name", (qint32)offsetof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR, Name), 4, XFRECORD_FLAG_RELATIVE_ADDRESS | XFRECORD_FLAG_RELATIVE_ADDRESS_STRING, VT_UINT32});
        listResult.append({"FirstThunk", (qint32)offsetof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR, FirstThunk), 4, XFRECORD_FLAG_RELATIVE_ADDRESS, VT_UINT32});
    } else if (nStructID == STRUCTID_IMAGE_THUNK_DATA32) {
        listResult.append({"Value", 0, 4, XFRECORD_FLAG_NONE, VT_UINT32});
    } else if (nStructID == STRUCTID_IMAGE_THUNK_DATA64) {
        listResult.append({"Value", 0, 8, XFRECORD_FLAG_NONE, VT_UINT64});
    } else if (nStructID == STRUCTID_IMAGE_DEBUG_DIRECTORY) {
        listResult.append({"Characteristics", (qint32)offsetof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY, Characteristics), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"TimeDateStamp", (qint32)offsetof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY, TimeDateStamp), 4, XFRECORD_FLAG_UNIXTIME, VT_UINT32});
        listResult.append({"MajorVersion", (qint32)offsetof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY, MajorVersion), 2, XFRECORD_FLAG_VERSION_MAJOR, VT_UINT16});
        listResult.append({"MinorVersion", (qint32)offsetof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY, MinorVersion), 2, XFRECORD_FLAG_VERSION_MINOR, VT_UINT16});
        listResult.append({"Type", (qint32)offsetof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY, Type), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"SizeOfData", (qint32)offsetof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY, SizeOfData), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"AddressOfRawData", (qint32)offsetof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY, AddressOfRawData), 4, XFRECORD_FLAG_RELATIVE_ADDRESS, VT_UINT32});
        listResult.append({"PointerToRawData", (qint32)offsetof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY, PointerToRawData), 4, XFRECORD_FLAG_OFFSET, VT_UINT32});
    } else if (nStructID == STRUCTID_IMAGE_TLS_DIRECTORY32) {
        listResult.append({"StartAddressOfRawData", (qint32)offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY32, StartAddressOfRawData), 4, XFRECORD_FLAG_ADDRESS, VT_UINT32});
        listResult.append({"EndAddressOfRawData", (qint32)offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY32, EndAddressOfRawData), 4, XFRECORD_FLAG_ADDRESS, VT_UINT32});
        listResult.append({"AddressOfIndex", (qint32)offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY32, AddressOfIndex), 4, XFRECORD_FLAG_ADDRESS, VT_UINT32});
        listResult.append({"AddressOfCallBacks", (qint32)offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY32, AddressOfCallBacks), 4, XFRECORD_FLAG_ADDRESS, VT_UINT32});
        listResult.append({"SizeOfZeroFill", (qint32)offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY32, SizeOfZeroFill), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"Characteristics", (qint32)offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY32, Characteristics), 4, XFRECORD_FLAG_NONE, VT_UINT32});
    } else if (nStructID == STRUCTID_IMAGE_TLS_DIRECTORY64) {
        listResult.append({"StartAddressOfRawData", (qint32)offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY64, StartAddressOfRawData), 8, XFRECORD_FLAG_ADDRESS, VT_UINT64});
        listResult.append({"EndAddressOfRawData", (qint32)offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY64, EndAddressOfRawData), 8, XFRECORD_FLAG_ADDRESS, VT_UINT64});
        listResult.append({"AddressOfIndex", (qint32)offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY64, AddressOfIndex), 8, XFRECORD_FLAG_ADDRESS, VT_UINT64});
        listResult.append({"AddressOfCallBacks", (qint32)offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY64, AddressOfCallBacks), 8, XFRECORD_FLAG_ADDRESS, VT_UINT64});
        listResult.append({"SizeOfZeroFill", (qint32)offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY64, SizeOfZeroFill), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"Characteristics", (qint32)offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY64, Characteristics), 4, XFRECORD_FLAG_NONE, VT_UINT32});
    } else if (nStructID == STRUCTID_IMAGE_LOAD_CONFIG_DIRECTORY32) {
        listResult.append({"Size", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, Size), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"TimeDateStamp", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, TimeDateStamp), 4, XFRECORD_FLAG_UNIXTIME, VT_UINT32});
        listResult.append({"MajorVersion", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, MajorVersion), 2, XFRECORD_FLAG_VERSION_MAJOR, VT_UINT16});
        listResult.append({"MinorVersion", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, MinorVersion), 2, XFRECORD_FLAG_VERSION_MINOR, VT_UINT16});
        listResult.append({"GlobalFlagsClear", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GlobalFlagsClear), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"GlobalFlagsSet", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GlobalFlagsSet), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"CriticalSectionDefaultTimeout", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, CriticalSectionDefaultTimeout), 4,
                           XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append(
            {"DeCommitFreeBlockThreshold", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, DeCommitFreeBlockThreshold), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append(
            {"DeCommitTotalFreeThreshold", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, DeCommitTotalFreeThreshold), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"LockPrefixTable", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, LockPrefixTable), 4, XFRECORD_FLAG_ADDRESS, VT_UINT32});
        listResult.append({"MaximumAllocationSize", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, MaximumAllocationSize), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append(
            {"VirtualMemoryThreshold", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, VirtualMemoryThreshold), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"ProcessAffinityMask", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, ProcessAffinityMask), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"ProcessHeapFlags", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, ProcessHeapFlags), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"CSDVersion", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, CSDVersion), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"DependentLoadFlags", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, DependentLoadFlags), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"EditList", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, EditList), 4, XFRECORD_FLAG_ADDRESS, VT_UINT32});
        listResult.append({"SecurityCookie", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, SecurityCookie), 4, XFRECORD_FLAG_ADDRESS, VT_UINT32});
        listResult.append({"SEHandlerTable", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, SEHandlerTable), 4, XFRECORD_FLAG_ADDRESS, VT_UINT32});
        listResult.append({"SEHandlerCount", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, SEHandlerCount), 4, XFRECORD_FLAG_COUNT, VT_UINT32});
        listResult.append({"GuardCFCheckFunctionPointer", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardCFCheckFunctionPointer), 4,
                           XFRECORD_FLAG_ADDRESS, VT_UINT32});
        listResult.append({"GuardCFDispatchFunctionPointer", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardCFDispatchFunctionPointer), 4,
                           XFRECORD_FLAG_ADDRESS, VT_UINT32});
        listResult.append(
            {"GuardCFFunctionTable", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardCFFunctionTable), 4, XFRECORD_FLAG_ADDRESS, VT_UINT32});
        listResult.append({"GuardCFFunctionCount", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardCFFunctionCount), 4, XFRECORD_FLAG_COUNT, VT_UINT32});
        listResult.append({"GuardFlags", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardFlags), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"CodeIntegrity.Flags",
                           (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, CodeIntegrity) +
                               (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_CODE_INTEGRITY, Flags),
                           2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"CodeIntegrity.Catalog",
                           (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, CodeIntegrity) +
                               (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_CODE_INTEGRITY, Catalog),
                           2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"CodeIntegrity.CatalogOffset",
                           (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, CodeIntegrity) +
                               (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_CODE_INTEGRITY, CatalogOffset),
                           4, XFRECORD_FLAG_OFFSET, VT_UINT32});
        listResult.append({"CodeIntegrity.Reserved",
                           (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, CodeIntegrity) +
                               (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_CODE_INTEGRITY, Reserved),
                           4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"GuardAddressTakenIatEntryTable", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardAddressTakenIatEntryTable), 4,
                           XFRECORD_FLAG_ADDRESS, VT_UINT32});
        listResult.append({"GuardAddressTakenIatEntryCount", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardAddressTakenIatEntryCount), 4,
                           XFRECORD_FLAG_COUNT, VT_UINT32});
        listResult.append({"GuardLongJumpTargetTable", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardLongJumpTargetTable), 4,
                           XFRECORD_FLAG_ADDRESS, VT_UINT32});
        listResult.append({"GuardLongJumpTargetCount", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardLongJumpTargetCount), 4,
                           XFRECORD_FLAG_COUNT, VT_UINT32});
        listResult.append(
            {"DynamicValueRelocTable", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, DynamicValueRelocTable), 4, XFRECORD_FLAG_ADDRESS, VT_UINT32});
        listResult.append({"CHPEMetadataPointer", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, CHPEMetadataPointer), 4, XFRECORD_FLAG_ADDRESS, VT_UINT32});
        listResult.append(
            {"GuardRFFailureRoutine", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardRFFailureRoutine), 4, XFRECORD_FLAG_ADDRESS, VT_UINT32});
        listResult.append({"GuardRFFailureRoutineFunctionPointer",
                           (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardRFFailureRoutineFunctionPointer), 4, XFRECORD_FLAG_ADDRESS, VT_UINT32});
        listResult.append({"DynamicValueRelocTableOffset", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, DynamicValueRelocTableOffset), 4,
                           XFRECORD_FLAG_OFFSET, VT_UINT32});
        listResult.append({"DynamicValueRelocTableSection", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, DynamicValueRelocTableSection), 2,
                           XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"Reserved2", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, Reserved2), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"GuardRFVerifyStackPointerFunctionPointer",
                           (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardRFVerifyStackPointerFunctionPointer), 4, XFRECORD_FLAG_ADDRESS, VT_UINT32});
        listResult.append(
            {"HotPatchTableOffset", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, HotPatchTableOffset), 4, XFRECORD_FLAG_OFFSET, VT_UINT32});
        listResult.append({"Reserved3", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, Reserved3), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"EnclaveConfigurationPointer", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, EnclaveConfigurationPointer), 4,
                           XFRECORD_FLAG_ADDRESS, VT_UINT32});
        listResult.append(
            {"VolatileMetadataPointer", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, VolatileMetadataPointer), 4, XFRECORD_FLAG_ADDRESS, VT_UINT32});
        listResult.append({"GuardEHContinuationTable", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardEHContinuationTable), 4,
                           XFRECORD_FLAG_ADDRESS, VT_UINT32});
        listResult.append({"GuardEHContinuationCount", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardEHContinuationCount), 4,
                           XFRECORD_FLAG_COUNT, VT_UINT32});
        listResult.append({"GuardXFGCheckFunctionPointer", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardXFGCheckFunctionPointer), 4,
                           XFRECORD_FLAG_ADDRESS, VT_UINT32});
        listResult.append({"GuardXFGDispatchFunctionPointer", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardXFGDispatchFunctionPointer), 4,
                           XFRECORD_FLAG_ADDRESS, VT_UINT32});
        listResult.append({"GuardXFGTableDispatchFunctionPointer", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardXFGTableDispatchFunctionPointer), 4,
                           XFRECORD_FLAG_ADDRESS, VT_UINT32});
        listResult.append({"CastGuardOsDeterminedFailureMode", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, CastGuardOsDeterminedFailureMode), 4,
                           XFRECORD_FLAG_ADDRESS, VT_UINT32});
        listResult.append(
            {"GuardMemcpyFunctionPointer", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardMemcpyFunctionPointer), 4, XFRECORD_FLAG_ADDRESS, VT_UINT32});
    } else if (nStructID == STRUCTID_IMAGE_LOAD_CONFIG_DIRECTORY64) {
        listResult.append({"Size", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, Size), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"TimeDateStamp", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, TimeDateStamp), 4, XFRECORD_FLAG_UNIXTIME, VT_UINT32});
        listResult.append({"MajorVersion", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, MajorVersion), 2, XFRECORD_FLAG_VERSION_MAJOR, VT_UINT16});
        listResult.append({"MinorVersion", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, MinorVersion), 2, XFRECORD_FLAG_VERSION_MINOR, VT_UINT16});
        listResult.append({"GlobalFlagsClear", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GlobalFlagsClear), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"GlobalFlagsSet", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GlobalFlagsSet), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"CriticalSectionDefaultTimeout", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, CriticalSectionDefaultTimeout), 4,
                           XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append(
            {"DeCommitFreeBlockThreshold", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, DeCommitFreeBlockThreshold), 8, XFRECORD_FLAG_SIZE, VT_UINT64});
        listResult.append(
            {"DeCommitTotalFreeThreshold", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, DeCommitTotalFreeThreshold), 8, XFRECORD_FLAG_SIZE, VT_UINT64});
        listResult.append({"LockPrefixTable", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, LockPrefixTable), 8, XFRECORD_FLAG_ADDRESS, VT_UINT64});
        listResult.append({"MaximumAllocationSize", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, MaximumAllocationSize), 8, XFRECORD_FLAG_SIZE, VT_UINT64});
        listResult.append(
            {"VirtualMemoryThreshold", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, VirtualMemoryThreshold), 8, XFRECORD_FLAG_SIZE, VT_UINT64});
        listResult.append({"ProcessAffinityMask", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, ProcessAffinityMask), 8, XFRECORD_FLAG_NONE, VT_UINT64});
        listResult.append({"ProcessHeapFlags", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, ProcessHeapFlags), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"CSDVersion", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, CSDVersion), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"DependentLoadFlags", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, DependentLoadFlags), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"EditList", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, EditList), 8, XFRECORD_FLAG_ADDRESS, VT_UINT64});
        listResult.append({"SecurityCookie", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, SecurityCookie), 8, XFRECORD_FLAG_ADDRESS, VT_UINT64});
        listResult.append({"SEHandlerTable", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, SEHandlerTable), 8, XFRECORD_FLAG_ADDRESS, VT_UINT64});
        listResult.append({"SEHandlerCount", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, SEHandlerCount), 8, XFRECORD_FLAG_COUNT, VT_UINT64});
        listResult.append({"GuardCFCheckFunctionPointer", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardCFCheckFunctionPointer), 8,
                           XFRECORD_FLAG_ADDRESS, VT_UINT64});
        listResult.append({"GuardCFDispatchFunctionPointer", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardCFDispatchFunctionPointer), 8,
                           XFRECORD_FLAG_ADDRESS, VT_UINT64});
        listResult.append(
            {"GuardCFFunctionTable", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardCFFunctionTable), 8, XFRECORD_FLAG_ADDRESS, VT_UINT64});
        listResult.append({"GuardCFFunctionCount", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardCFFunctionCount), 8, XFRECORD_FLAG_COUNT, VT_UINT64});
        listResult.append({"GuardFlags", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardFlags), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"CodeIntegrity.Flags",
                           (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, CodeIntegrity) +
                               (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_CODE_INTEGRITY, Flags),
                           2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"CodeIntegrity.Catalog",
                           (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, CodeIntegrity) +
                               (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_CODE_INTEGRITY, Catalog),
                           2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"CodeIntegrity.CatalogOffset",
                           (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, CodeIntegrity) +
                               (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_CODE_INTEGRITY, CatalogOffset),
                           4, XFRECORD_FLAG_OFFSET, VT_UINT32});
        listResult.append({"CodeIntegrity.Reserved",
                           (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, CodeIntegrity) +
                               (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_CODE_INTEGRITY, Reserved),
                           4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"GuardAddressTakenIatEntryTable", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardAddressTakenIatEntryTable), 8,
                           XFRECORD_FLAG_ADDRESS, VT_UINT64});
        listResult.append({"GuardAddressTakenIatEntryCount", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardAddressTakenIatEntryCount), 8,
                           XFRECORD_FLAG_COUNT, VT_UINT64});
        listResult.append({"GuardLongJumpTargetTable", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardLongJumpTargetTable), 8,
                           XFRECORD_FLAG_ADDRESS, VT_UINT64});
        listResult.append({"GuardLongJumpTargetCount", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardLongJumpTargetCount), 8,
                           XFRECORD_FLAG_COUNT, VT_UINT64});
        listResult.append(
            {"DynamicValueRelocTable", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, DynamicValueRelocTable), 8, XFRECORD_FLAG_ADDRESS, VT_UINT64});
        listResult.append({"CHPEMetadataPointer", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, CHPEMetadataPointer), 8, XFRECORD_FLAG_ADDRESS, VT_UINT64});
        listResult.append(
            {"GuardRFFailureRoutine", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardRFFailureRoutine), 8, XFRECORD_FLAG_ADDRESS, VT_UINT64});
        listResult.append({"GuardRFFailureRoutineFunctionPointer",
                           (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardRFFailureRoutineFunctionPointer), 8, XFRECORD_FLAG_ADDRESS, VT_UINT64});
        listResult.append({"DynamicValueRelocTableOffset", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, DynamicValueRelocTableOffset), 4,
                           XFRECORD_FLAG_OFFSET, VT_UINT32});
        listResult.append({"DynamicValueRelocTableSection", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, DynamicValueRelocTableSection), 2,
                           XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"Reserved2", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, Reserved2), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"GuardRFVerifyStackPointerFunctionPointer",
                           (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardRFVerifyStackPointerFunctionPointer), 8, XFRECORD_FLAG_ADDRESS, VT_UINT64});
        listResult.append(
            {"HotPatchTableOffset", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, HotPatchTableOffset), 4, XFRECORD_FLAG_OFFSET, VT_UINT32});
        listResult.append({"Reserved3", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, Reserved3), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"EnclaveConfigurationPointer", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, EnclaveConfigurationPointer), 8,
                           XFRECORD_FLAG_ADDRESS, VT_UINT64});
        listResult.append(
            {"VolatileMetadataPointer", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, VolatileMetadataPointer), 8, XFRECORD_FLAG_ADDRESS, VT_UINT64});
        listResult.append({"GuardEHContinuationTable", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardEHContinuationTable), 8,
                           XFRECORD_FLAG_ADDRESS, VT_UINT64});
        listResult.append({"GuardEHContinuationCount", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardEHContinuationCount), 8,
                           XFRECORD_FLAG_COUNT, VT_UINT64});
        listResult.append({"GuardXFGCheckFunctionPointer", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardXFGCheckFunctionPointer), 8,
                           XFRECORD_FLAG_ADDRESS, VT_UINT64});
        listResult.append({"GuardXFGDispatchFunctionPointer", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardXFGDispatchFunctionPointer), 8,
                           XFRECORD_FLAG_ADDRESS, VT_UINT64});
        listResult.append({"GuardXFGTableDispatchFunctionPointer", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardXFGTableDispatchFunctionPointer), 8,
                           XFRECORD_FLAG_ADDRESS, VT_UINT64});
        listResult.append({"CastGuardOsDeterminedFailureMode", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, CastGuardOsDeterminedFailureMode), 8,
                           XFRECORD_FLAG_ADDRESS, VT_UINT64});
        listResult.append(
            {"GuardMemcpyFunctionPointer", (qint32)offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardMemcpyFunctionPointer), 8, XFRECORD_FLAG_ADDRESS, VT_UINT64});
    } else if (nStructID == STRUCTID_IMAGE_RUNTIME_FUNCTION_ENTRY) {
        listResult.append({"BeginAddress", (qint32)offsetof(XPE_DEF::S_IMAGE_RUNTIME_FUNCTION_ENTRY, BeginAddress), 4, XFRECORD_FLAG_RELATIVE_ADDRESS, VT_UINT32});
        listResult.append({"EndAddress", (qint32)offsetof(XPE_DEF::S_IMAGE_RUNTIME_FUNCTION_ENTRY, EndAddress), 4, XFRECORD_FLAG_RELATIVE_ADDRESS, VT_UINT32});
        listResult.append(
            {"UnwindInfoAddress", (qint32)offsetof(XPE_DEF::S_IMAGE_RUNTIME_FUNCTION_ENTRY, UnwindInfoAddress), 4, XFRECORD_FLAG_RELATIVE_ADDRESS, VT_UINT32});
    } else if (nStructID == STRUCTID_IMAGE_RUNTIME_FUNCTION_ENTRY_ARM) {
        listResult.append(
            {"BeginAddress", (qint32)offsetof(XPE_DEF::S_IMAGE_ARM_RUNTIME_FUNCTION_ENTRY, BeginAddress), 4, XFRECORD_FLAG_RELATIVE_ADDRESS, VT_UINT32});
        listResult.append({"UnwindData", (qint32)offsetof(XPE_DEF::S_IMAGE_ARM_RUNTIME_FUNCTION_ENTRY, UnwindData), 4, XFRECORD_FLAG_NONE, VT_UINT32});
    } else if (nStructID == STRUCTID_IMAGE_RUNTIME_FUNCTION_ENTRY_ALPHA) {
        listResult.append(
            {"BeginAddress", (qint32)offsetof(XPE_DEF::S_IMAGE_ALPHA_RUNTIME_FUNCTION_ENTRY, BeginAddress), 4, XFRECORD_FLAG_RELATIVE_ADDRESS, VT_UINT32});
        listResult.append(
            {"EndAddress", (qint32)offsetof(XPE_DEF::S_IMAGE_ALPHA_RUNTIME_FUNCTION_ENTRY, EndAddress), 4, XFRECORD_FLAG_RELATIVE_ADDRESS, VT_UINT32});
        listResult.append({"ExceptionHandler", (qint32)offsetof(XPE_DEF::S_IMAGE_ALPHA_RUNTIME_FUNCTION_ENTRY, ExceptionHandler), 4,
                           XFRECORD_FLAG_RELATIVE_ADDRESS, VT_UINT32});
        listResult.append(
            {"HandlerData", (qint32)offsetof(XPE_DEF::S_IMAGE_ALPHA_RUNTIME_FUNCTION_ENTRY, HandlerData), 4, XFRECORD_FLAG_RELATIVE_ADDRESS, VT_UINT32});
        listResult.append({"PrologEndAddress", (qint32)offsetof(XPE_DEF::S_IMAGE_ALPHA_RUNTIME_FUNCTION_ENTRY, PrologEndAddress), 4,
                           XFRECORD_FLAG_RELATIVE_ADDRESS, VT_UINT32});
    } else if (nStructID == STRUCTID_IMAGE_RUNTIME_FUNCTION_ENTRY_ALPHA64) {
        listResult.append(
            {"BeginAddress", (qint32)offsetof(XPE_DEF::S_IMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY, BeginAddress), 8, XFRECORD_FLAG_RELATIVE_ADDRESS, VT_UINT64});
        listResult.append(
            {"EndAddress", (qint32)offsetof(XPE_DEF::S_IMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY, EndAddress), 8, XFRECORD_FLAG_RELATIVE_ADDRESS, VT_UINT64});
        listResult.append({"ExceptionHandler", (qint32)offsetof(XPE_DEF::S_IMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY, ExceptionHandler), 8,
                           XFRECORD_FLAG_RELATIVE_ADDRESS, VT_UINT64});
        listResult.append(
            {"HandlerData", (qint32)offsetof(XPE_DEF::S_IMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY, HandlerData), 8, XFRECORD_FLAG_RELATIVE_ADDRESS, VT_UINT64});
        listResult.append({"PrologEndAddress", (qint32)offsetof(XPE_DEF::S_IMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY, PrologEndAddress), 8,
                           XFRECORD_FLAG_RELATIVE_ADDRESS, VT_UINT64});
    } else if (nStructID == STRUCTID_IMAGE_BOUND_IMPORT_DESCRIPTOR) {
        listResult.append({"TimeDateStamp", (qint32)offsetof(XPE_DEF::IMAGE_BOUND_IMPORT_DESCRIPTOR, TimeDateStamp), 4, XFRECORD_FLAG_UNIXTIME, VT_UINT32});
        listResult.append(
            {"OffsetModuleName", (qint32)offsetof(XPE_DEF::IMAGE_BOUND_IMPORT_DESCRIPTOR, OffsetModuleName), 2, XFRECORD_FLAG_RELATIVE_OFFSET, VT_UINT16});
        listResult.append(
            {"NumberOfModuleForwarderRefs", (qint32)offsetof(XPE_DEF::IMAGE_BOUND_IMPORT_DESCRIPTOR, NumberOfModuleForwarderRefs), 2, XFRECORD_FLAG_COUNT, VT_UINT16});
    } else if (nStructID == STRUCTID_IMAGE_BOUND_FORWARDER_REF) {
        listResult.append({"TimeDateStamp", (qint32)offsetof(XPE_DEF::IMAGE_BOUND_FORWARDER_REF, TimeDateStamp), 4, XFRECORD_FLAG_UNIXTIME, VT_UINT32});
        listResult.append(
            {"OffsetModuleName", (qint32)offsetof(XPE_DEF::IMAGE_BOUND_FORWARDER_REF, OffsetModuleName), 2, XFRECORD_FLAG_RELATIVE_OFFSET, VT_UINT16});
        listResult.append({"Reserved", (qint32)offsetof(XPE_DEF::IMAGE_BOUND_FORWARDER_REF, Reserved), 2, XFRECORD_FLAG_NONE, VT_UINT16});
    } else if (nStructID == STRUCTID_IMAGE_DELAYLOAD_DESCRIPTOR) {
        listResult.append({"AllAttributes", (qint32)offsetof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR, AllAttributes), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"DllName", (qint32)offsetof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR, DllNameRVA), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"ModuleHandle", (qint32)offsetof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR, ModuleHandleRVA), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"ImportAddressTable", (qint32)offsetof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR, ImportAddressTableRVA), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"ImportNameTable", (qint32)offsetof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR, ImportNameTableRVA), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"BoundImportAddressTable", (qint32)offsetof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR, BoundImportAddressTableRVA), 4,
                           XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"UnloadInformationTable", (qint32)offsetof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR, UnloadInformationTableRVA), 4,
                           XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"TimeDateStamp", (qint32)offsetof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR, TimeDateStamp), 4, XFRECORD_FLAG_UNIXTIME, VT_UINT32});
    } else if (nStructID == STRUCTID_IMAGE_COR20_HEADER) {
        listResult.append({"cb", (qint32)offsetof(XPE_DEF::IMAGE_COR20_HEADER, cb), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"MajorRuntimeVersion", (qint32)offsetof(XPE_DEF::IMAGE_COR20_HEADER, MajorRuntimeVersion), 2, XFRECORD_FLAG_VERSION_MAJOR, VT_UINT16});
        listResult.append({"MinorRuntimeVersion", (qint32)offsetof(XPE_DEF::IMAGE_COR20_HEADER, MinorRuntimeVersion), 2, XFRECORD_FLAG_VERSION_MINOR, VT_UINT16});
        listResult.append({"MetaData.VirtualAddress", (qint32)offsetof(XPE_DEF::IMAGE_COR20_HEADER, MetaData) + offsetof(XPE_DEF::IMAGE_DATA_DIRECTORY, VirtualAddress),
                           4, XFRECORD_FLAG_RELATIVE_ADDRESS, VT_UINT32});
        listResult.append(
            {"MetaData.Size", (qint32)offsetof(XPE_DEF::IMAGE_COR20_HEADER, MetaData) + offsetof(XPE_DEF::IMAGE_DATA_DIRECTORY, Size), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"Flags", (qint32)offsetof(XPE_DEF::IMAGE_COR20_HEADER, Flags), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        bool bNativeEntryPoint = false;
        if ((xLoc.locType == LT_OFFSET) && checkOffsetSize(xLoc.nLocation + offsetof(XPE_DEF::IMAGE_COR20_HEADER, Flags), sizeof(quint32))) {
            bNativeEntryPoint = (read_uint32(xLoc.nLocation + offsetof(XPE_DEF::IMAGE_COR20_HEADER, Flags)) & 0x00000010U) != 0;
        }
        listResult.append({bNativeEntryPoint ? QString("EntryPointRVA") : QString("EntryPointToken"),
                           (qint32)offsetof(XPE_DEF::IMAGE_COR20_HEADER, EntryPointToken), 4,
                           bNativeEntryPoint ? (quint64)XFRECORD_FLAG_RELATIVE_ADDRESS : (quint64)XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"Resources.VirtualAddress", (qint32)offsetof(XPE_DEF::IMAGE_COR20_HEADER, Resources) + offsetof(XPE_DEF::IMAGE_DATA_DIRECTORY, VirtualAddress),
                           4, XFRECORD_FLAG_RELATIVE_ADDRESS, VT_UINT32});
        listResult.append({"Resources.Size", (qint32)offsetof(XPE_DEF::IMAGE_COR20_HEADER, Resources) + offsetof(XPE_DEF::IMAGE_DATA_DIRECTORY, Size), 4,
                           XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"StrongNameSignature.VirtualAddress",
                           (qint32)offsetof(XPE_DEF::IMAGE_COR20_HEADER, StrongNameSignature) + offsetof(XPE_DEF::IMAGE_DATA_DIRECTORY, VirtualAddress), 4,
                           XFRECORD_FLAG_RELATIVE_ADDRESS, VT_UINT32});
        listResult.append({"StrongNameSignature.Size", (qint32)offsetof(XPE_DEF::IMAGE_COR20_HEADER, StrongNameSignature) + offsetof(XPE_DEF::IMAGE_DATA_DIRECTORY, Size),
                           4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"CodeManagerTable.VirtualAddress",
                           (qint32)offsetof(XPE_DEF::IMAGE_COR20_HEADER, CodeManagerTable) + offsetof(XPE_DEF::IMAGE_DATA_DIRECTORY, VirtualAddress), 4,
                           XFRECORD_FLAG_RELATIVE_ADDRESS, VT_UINT32});
        listResult.append({"CodeManagerTable.Size",
                           (qint32)offsetof(XPE_DEF::IMAGE_COR20_HEADER, CodeManagerTable) + offsetof(XPE_DEF::IMAGE_DATA_DIRECTORY, Size), 4, XFRECORD_FLAG_SIZE,
                           VT_UINT32});
        listResult.append({"VTableFixups.VirtualAddress",
                           (qint32)offsetof(XPE_DEF::IMAGE_COR20_HEADER, VTableFixups) + offsetof(XPE_DEF::IMAGE_DATA_DIRECTORY, VirtualAddress), 4,
                           XFRECORD_FLAG_RELATIVE_ADDRESS, VT_UINT32});
        listResult.append({"VTableFixups.Size", (qint32)offsetof(XPE_DEF::IMAGE_COR20_HEADER, VTableFixups) + offsetof(XPE_DEF::IMAGE_DATA_DIRECTORY, Size), 4,
                           XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"ExportAddressTableJumps.VirtualAddress",
                           (qint32)offsetof(XPE_DEF::IMAGE_COR20_HEADER, ExportAddressTableJumps) + offsetof(XPE_DEF::IMAGE_DATA_DIRECTORY, VirtualAddress), 4,
                           XFRECORD_FLAG_RELATIVE_ADDRESS, VT_UINT32});
        listResult.append({"ExportAddressTableJumps.Size",
                           (qint32)offsetof(XPE_DEF::IMAGE_COR20_HEADER, ExportAddressTableJumps) + offsetof(XPE_DEF::IMAGE_DATA_DIRECTORY, Size), 4,
                           XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"ManagedNativeHeader.VirtualAddress",
                           (qint32)offsetof(XPE_DEF::IMAGE_COR20_HEADER, ManagedNativeHeader) + offsetof(XPE_DEF::IMAGE_DATA_DIRECTORY, VirtualAddress), 4,
                           XFRECORD_FLAG_RELATIVE_ADDRESS, VT_UINT32});
        listResult.append({"ManagedNativeHeader.Size",
                           (qint32)offsetof(XPE_DEF::IMAGE_COR20_HEADER, ManagedNativeHeader) + offsetof(XPE_DEF::IMAGE_DATA_DIRECTORY, Size), 4,
                           XFRECORD_FLAG_SIZE, VT_UINT32});
    } else if (nStructID == STRUCTID_IMAGE_RESOURCE_DIRECTORY) {
        listResult.append({"Characteristics", (qint32)offsetof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY, Characteristics), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"TimeDateStamp", (qint32)offsetof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY, TimeDateStamp), 4, XFRECORD_FLAG_UNIXTIME, VT_UINT32});
        listResult.append({"MajorVersion", (qint32)offsetof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY, MajorVersion), 2, XFRECORD_FLAG_VERSION_MAJOR, VT_UINT16});
        listResult.append({"MinorVersion", (qint32)offsetof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY, MinorVersion), 2, XFRECORD_FLAG_VERSION_MINOR, VT_UINT16});
        listResult.append({"NumberOfNamedEntries", (qint32)offsetof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY, NumberOfNamedEntries), 2, XFRECORD_FLAG_COUNT, VT_UINT16});
        listResult.append({"NumberOfIdEntries", (qint32)offsetof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY, NumberOfIdEntries), 2, XFRECORD_FLAG_COUNT, VT_UINT16});
    } else if (nStructID == STRUCTID_IMAGE_RESOURCE_DIRECTORY_ENTRY) {
        // High bit of Name = name is a string; high bit of OffsetToData = entry is a subdirectory
        listResult.append({"Name", 0, 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"OffsetToData", 4, 4, XFRECORD_FLAG_NONE, VT_UINT32});
    } else if (nStructID == STRUCTID_IMAGE_RESOURCE_DATA_ENTRY) {
        listResult.append({"OffsetToData", (qint32)offsetof(XPE_DEF::IMAGE_RESOURCE_DATA_ENTRY, OffsetToData), 4, XFRECORD_FLAG_RELATIVE_ADDRESS, VT_UINT32});
        listResult.append({"Size", (qint32)offsetof(XPE_DEF::IMAGE_RESOURCE_DATA_ENTRY, Size), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"CodePage", (qint32)offsetof(XPE_DEF::IMAGE_RESOURCE_DATA_ENTRY, CodePage), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"Reserved", (qint32)offsetof(XPE_DEF::IMAGE_RESOURCE_DATA_ENTRY, Reserved), 4, XFRECORD_FLAG_NONE, VT_UINT32});
    } else if (nStructID == STRUCTID_IMAGE_BASE_RELOCATION) {
        listResult.append({"VirtualAddress", (qint32)offsetof(XPE_DEF::IMAGE_BASE_RELOCATION, VirtualAddress), 4, XFRECORD_FLAG_RELATIVE_ADDRESS, VT_UINT32});
        listResult.append({"SizeOfBlock", (qint32)offsetof(XPE_DEF::IMAGE_BASE_RELOCATION, SizeOfBlock), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
    } else if (nStructID == STRUCTID_IMAGE_BASE_RELOCATION_ENTRY) {
        listResult.append({"TypeOffset", 0, 2, XFRECORD_FLAG_NONE, VT_UINT16});
    } else if (nStructID == STRUCTID_WIN_CERT_RECORD) {
        listResult.append({"dwLength", (qint32)offsetof(XPE_DEF::WIN_CERT_RECORD, dwLength), 4, XFRECORD_FLAG_SIZE, VT_UINT32});
        listResult.append({"wRevision", (qint32)offsetof(XPE_DEF::WIN_CERT_RECORD, wRevision), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"wCertificateType", (qint32)offsetof(XPE_DEF::WIN_CERT_RECORD, wCertificateType), 2, XFRECORD_FLAG_NONE, VT_UINT16});
    } else if (nStructID == STRUCTID_NET_METADATA) {
        listResult.append({"Signature", 0, 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"MajorVersion", 4, 2, XFRECORD_FLAG_VERSION_MAJOR, VT_UINT16});
        listResult.append({"MinorVersion", 6, 2, XFRECORD_FLAG_VERSION_MINOR, VT_UINT16});
        listResult.append({"Reserved", 8, 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"VersionStringLength", 12, 4, XFRECORD_FLAG_SIZE, VT_UINT32});

        if ((xLoc.locType == LT_OFFSET) && checkOffsetSize(xLoc.nLocation, 16)) {
            quint32 nVersionStringLength = read_uint32(xLoc.nLocation + 12);

            if ((nVersionStringLength <= 0x10000) && checkOffsetSize(xLoc.nLocation + 16, (qint64)nVersionStringLength + 4)) {
                listResult.append({"Version", 16, (qint32)nVersionStringLength, XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
                listResult.append({"Flags", 16 + (qint32)nVersionStringLength, 2, XFRECORD_FLAG_NONE, VT_UINT16});
                listResult.append({"Streams", 18 + (qint32)nVersionStringLength, 2, XFRECORD_FLAG_COUNT, VT_UINT16});
            }
        }
    } else if (nStructID == STRUCTID_NET_METADATA_STREAM) {
        listResult.append({"Offset", 0, 4, XFRECORD_FLAG_RELATIVE_OFFSET, VT_UINT32});
        listResult.append({"Size", 4, 4, XFRECORD_FLAG_SIZE, VT_UINT32});

        if ((xLoc.locType == LT_OFFSET) && checkOffsetSize(xLoc.nLocation, 8)) {
            qint64 nNameBytes = qMin((qint64)32, getSize() - (qint64)xLoc.nLocation - 8);
            QByteArray baName = read_array(xLoc.nLocation + 8, nNameBytes);
            qint32 nTerminator = baName.indexOf('\0');

            if (nTerminator != -1) {
                qint32 nStorageSize = (nTerminator + 1 + 3) & ~3;

                if (checkOffsetSize(xLoc.nLocation + 8, nStorageSize)) {
                    listResult.append({"Name", 8, nStorageSize, XFRECORD_FLAG_NONE, VT_CHAR_ARRAY});
                }
            }
        }
    } else {
        listResult = XMSDOS::getXFRecords(fileType, nStructID, xLoc);
    }

    if (((nStructID == STRUCTID_IMAGE_LOAD_CONFIG_DIRECTORY32) || (nStructID == STRUCTID_IMAGE_LOAD_CONFIG_DIRECTORY64)) && (xLoc.locType == LT_OFFSET) &&
        checkOffsetSize(xLoc.nLocation, sizeof(quint32))) {
        quint32 nLoadConfigSize = read_uint32(xLoc.nLocation);

        if (nLoadConfigSize) {
            QList<XBinary::XFRECORD> listFiltered;

            for (const XBinary::XFRECORD &record : qAsConst(listResult)) {
                if ((record.nOffset >= 0) && ((quint32)(record.nOffset + record.nSize) <= nLoadConfigSize)) {
                    listFiltered.append(record);
                }
            }

            listResult = listFiltered;
        }
    }

    // PE/COFF and CLR metadata fields are encoded little-endian even for the
    // legacy big-endian target-machine identifiers. Keep XF presentation in
    // sync with the PE readers, which also decode these structures as LE.
    for (XBinary::XFRECORD &record : listResult) {
        record.nFlags &= ~((quint64)XFRECORD_FLAG_BE);
        record.nFlags |= XFRECORD_FLAG_LE;
    }

    return listResult;
}

void XPE::_decorateExportFunctionTable(XFHEADER *pXfHeader, const XFSTRUCT &xfStruct, qint64 nExportDirOffset, qint64 nSize,
                                       PDSTRUCT *pPdStruct)
{
    if ((pXfHeader == nullptr) || (xfStruct.pMemoryMap == nullptr) || pXfHeader->listRowLocations.isEmpty() ||
        (nSize < (qint64)sizeof(XPE_DEF::IMAGE_EXPORT_DIRECTORY)) ||
        !checkOffsetSize(nExportDirOffset, sizeof(XPE_DEF::IMAGE_EXPORT_DIRECTORY))) {
        return;
    }

    qint64 nExportSize = qMin(nSize, getSize() - nExportDirOffset);
    qint64 nExportEndOffset = nExportDirOffset + nExportSize;
    XPE_DEF::IMAGE_EXPORT_DIRECTORY ied = read_IMAGE_EXPORT_DIRECTORY(nExportDirOffset);
    qint64 nFunctionsTableOffset = relAddressToOffset(xfStruct.pMemoryMap, ied.AddressOfFunctions);

    if ((nFunctionsTableOffset < nExportDirOffset) ||
        ((qint64)sizeof(quint32) > (nExportEndOffset - nFunctionsTableOffset))) {
        return;
    }

    qint32 nNumberOfFunctions = (qint32)qMin((quint64)ied.NumberOfFunctions,
                                             qMin((quint64)0x10000, (quint64)((nExportEndOffset - nFunctionsTableOffset) / sizeof(quint32))));
    QMap<quint32, QString> mapFunctionNames;
    QMap<quint32, quint32> mapFunctionNameRVAs;

    if ((ied.NumberOfNames != 0) && (ied.AddressOfNames != 0) && (ied.AddressOfNameOrdinals != 0)) {
        qint64 nNamesTableOffset = relAddressToOffset(xfStruct.pMemoryMap, ied.AddressOfNames);
        qint64 nOrdinalsTableOffset = relAddressToOffset(xfStruct.pMemoryMap, ied.AddressOfNameOrdinals);

        if ((nNamesTableOffset >= nExportDirOffset) && ((qint64)sizeof(quint32) <= (nExportEndOffset - nNamesTableOffset)) &&
            (nOrdinalsTableOffset >= nExportDirOffset) && ((qint64)sizeof(quint16) <= (nExportEndOffset - nOrdinalsTableOffset))) {
            quint64 nNamesByPointers = (quint64)((nExportEndOffset - nNamesTableOffset) / sizeof(quint32));
            quint64 nNamesByOrdinals = (quint64)((nExportEndOffset - nOrdinalsTableOffset) / sizeof(quint16));
            qint32 nNumberOfNames = (qint32)qMin((quint64)ied.NumberOfNames, qMin((quint64)0x10000, qMin(nNamesByPointers, nNamesByOrdinals)));

            for (qint32 i = 0; (i < nNumberOfNames) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
                quint16 nFunctionIndex = read_uint16(nOrdinalsTableOffset + (qint64)i * sizeof(quint16));

                if (nFunctionIndex >= nNumberOfFunctions) {
                    continue;
                }

                quint32 nNameRVA = read_uint32(nNamesTableOffset + (qint64)i * sizeof(quint32));
                qint64 nNameOffset = relAddressToOffset(xfStruct.pMemoryMap, nNameRVA);

                if ((nNameOffset < nExportDirOffset) || (nNameOffset >= nExportEndOffset)) {
                    continue;
                }

                QString sName = read_ansiString(nNameOffset, qMin((qint64)2048, nExportEndOffset - nNameOffset));

                if (!sName.isEmpty()) {
                    mapFunctionNames.insert(nFunctionIndex, sName);
                    mapFunctionNameRVAs.insert(nFunctionIndex, nNameRVA);
                }
            }
        }
    }

    if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
        return;
    }

    QList<QString> listRowNames;
    XBinary::XFCOLUMN xfOrdinalColumn = {};
    xfOrdinalColumn.sName = QString("Ordinal");
    XBinary::XFCOLUMN xfNameRVAColumn = {};
    xfNameRVAColumn.sName = QString("Name RVA");
    bool bHasValidRow = false;

    for (qint32 i = 0; (i < pXfHeader->listRowLocations.count()) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        qint64 nRowOffset = pXfHeader->listRowLocations.at(i);
        qint64 nRelativeOffset = nRowOffset - nFunctionsTableOffset;
        qint32 nFunctionIndex = -1;

        if ((nRelativeOffset >= 0) && ((nRelativeOffset % (qint64)sizeof(quint32)) == 0)) {
            qint64 nCandidateIndex = nRelativeOffset / sizeof(quint32);
            if ((nCandidateIndex >= 0) && (nCandidateIndex < nNumberOfFunctions)) {
                nFunctionIndex = (qint32)nCandidateIndex;
            }
        }

        if (nFunctionIndex >= 0) {
            bHasValidRow = true;
            listRowNames.append(mapFunctionNames.value(nFunctionIndex));
            xfOrdinalColumn.listValues.append(QString::number((quint64)ied.Base + (quint32)nFunctionIndex));
            xfNameRVAColumn.listValues.append(mapFunctionNameRVAs.contains(nFunctionIndex)
                                                  ? QString("0x%1").arg(mapFunctionNameRVAs.value(nFunctionIndex), 0, 16).toUpper()
                                                  : QString());
        } else {
            listRowNames.append(QString());
            xfOrdinalColumn.listValues.append(QString());
            xfNameRVAColumn.listValues.append(QString());
        }
    }

    if (bHasValidRow && XBinary::isPdStructNotCanceled(pPdStruct) &&
        (listRowNames.count() == pXfHeader->listRowLocations.count())) {
        pXfHeader->listRowNames = listRowNames;
        pXfHeader->listExtraColumns.append(xfOrdinalColumn);
        pXfHeader->listExtraColumns.append(xfNameRVAColumn);
    }
}

void XPE::_appendExportFunctionNames(QList<XFHEADER> &listResult, const XFSTRUCT &xfStruct, qint64 nExportDirOffset, qint64 nSize,
                                     const QString &sParentTag, PDSTRUCT *pPdStruct)
{
    if ((nSize < (qint64)sizeof(XPE_DEF::IMAGE_EXPORT_DIRECTORY)) ||
        !checkOffsetSize(nExportDirOffset, sizeof(XPE_DEF::IMAGE_EXPORT_DIRECTORY))) {
        return;
    }

    qint64 nExportSize = qMin(nSize, getSize() - nExportDirOffset);
    qint64 nExportEndOffset = nExportDirOffset + nExportSize;

    XPE_DEF::IMAGE_EXPORT_DIRECTORY ied = read_IMAGE_EXPORT_DIRECTORY(nExportDirOffset);

    if ((ied.NumberOfFunctions == 0) || (ied.AddressOfFunctions == 0)) {
        return;
    }

    qint64 nFunctionsTableOffset = relAddressToOffset(xfStruct.pMemoryMap, ied.AddressOfFunctions);

    if ((nFunctionsTableOffset < nExportDirOffset) ||
        ((qint64)sizeof(quint32) > (nExportEndOffset - nFunctionsTableOffset))) {
        return;
    }

    qint32 nNumberOfFunctions = (qint32)qMin((quint64)ied.NumberOfFunctions,
                                             qMin((quint64)0x10000, (quint64)((nExportEndOffset - nFunctionsTableOffset) / sizeof(quint32))));

    if (nNumberOfFunctions <= 0) {
        return;
    }

    QMap<quint32, QString> mapFunctionNames;
    QMap<quint32, quint32> mapFunctionNameRVAs;

    if ((ied.NumberOfNames != 0) && (ied.AddressOfNames != 0) && (ied.AddressOfNameOrdinals != 0)) {
        qint64 nNamesTableOffset = relAddressToOffset(xfStruct.pMemoryMap, ied.AddressOfNames);
        qint64 nOrdinalsTableOffset = relAddressToOffset(xfStruct.pMemoryMap, ied.AddressOfNameOrdinals);

        if ((nNamesTableOffset >= nExportDirOffset) && ((qint64)sizeof(quint32) <= (nExportEndOffset - nNamesTableOffset)) &&
            (nOrdinalsTableOffset >= nExportDirOffset) && ((qint64)sizeof(quint16) <= (nExportEndOffset - nOrdinalsTableOffset))) {
            quint64 nNamesByPointers = (quint64)((nExportEndOffset - nNamesTableOffset) / sizeof(quint32));
            quint64 nNamesByOrdinals = (quint64)((nExportEndOffset - nOrdinalsTableOffset) / sizeof(quint16));
            qint32 nNumberOfNames = (qint32)qMin((quint64)ied.NumberOfNames, qMin((quint64)0x10000, qMin(nNamesByPointers, nNamesByOrdinals)));

            for (qint32 i = 0; (i < nNumberOfNames) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
                quint16 nFunctionIndex = read_uint16(nOrdinalsTableOffset + (qint64)i * sizeof(quint16));

                if (nFunctionIndex >= nNumberOfFunctions) {
                    continue;
                }

                quint32 nNameRVA = read_uint32(nNamesTableOffset + (qint64)i * sizeof(quint32));
                qint64 nNameOffset = relAddressToOffset(xfStruct.pMemoryMap, nNameRVA);

                if ((nNameOffset < nExportDirOffset) || (nNameOffset >= nExportEndOffset)) {
                    continue;
                }

                QString sName = read_ansiString(nNameOffset, qMin((qint64)2048, nExportEndOffset - nNameOffset));

                if (!sName.isEmpty()) {
                    mapFunctionNames.insert(nFunctionIndex, sName);
                    mapFunctionNameRVAs.insert(nFunctionIndex, nNameRVA);
                }
            }
        }
    }

    XFHEADER xfFunctionsTable = {};
    xfFunctionsTable.sParentTag = sParentTag;
    xfFunctionsTable.fileType = xfStruct.fileType;
    xfFunctionsTable.structID = static_cast<XBinary::STRUCTID>(STRUCTID_IMAGE_EXPORT_FUNCTION);
    xfFunctionsTable.xLoc = offsetToLoc(nFunctionsTableOffset);
    xfFunctionsTable.xfType = XFTYPE_TABLE;
    xfFunctionsTable.listFields = getXFRecords(xfStruct.fileType, STRUCTID_IMAGE_EXPORT_FUNCTION, xfFunctionsTable.xLoc);
    xfFunctionsTable.nSize = sizeof(quint32);

    XBinary::XFCOLUMN xfOrdinalColumn = {};
    xfOrdinalColumn.sName = QString("Ordinal");

    XBinary::XFCOLUMN xfNameRVAColumn = {};
    xfNameRVAColumn.sName = QString("Name RVA");

    for (qint32 i = 0; (i < nNumberOfFunctions) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        xfFunctionsTable.listRowLocations.append(nFunctionsTableOffset + (qint64)i * sizeof(quint32));
        xfFunctionsTable.listRowNames.append(mapFunctionNames.value(i));
        xfOrdinalColumn.listValues.append(QString::number((quint64)ied.Base + (quint32)i));

        if (mapFunctionNameRVAs.contains(i)) {
            xfNameRVAColumn.listValues.append(QString("0x%1").arg(mapFunctionNameRVAs.value(i), 0, 16).toUpper());
        } else {
            xfNameRVAColumn.listValues.append(QString());
        }
    }

    if (xfFunctionsTable.listRowLocations.isEmpty()) {
        return;
    }

    xfFunctionsTable.listExtraColumns.append(xfOrdinalColumn);
    xfFunctionsTable.listExtraColumns.append(xfNameRVAColumn);
    xfFunctionsTable.sTag = xfHeaderToTag(xfFunctionsTable, structIDToString(STRUCTID_IMAGE_EXPORT_FUNCTION), sParentTag);
    listResult.append(xfFunctionsTable);
}

void XPE::_appendResourceEntries(QList<XFHEADER> &listResult, const XFSTRUCT &xfStruct, qint64 nResourceDirOffset, qint64 nSize, const QString &sParentTag,
                                 PDSTRUCT *pPdStruct)
{
    if ((nResourceDirOffset < 0) || (nResourceDirOffset >= getSize())) {
        return;
    }

    if (nSize <= 0) {
        nSize = getSize() - nResourceDirOffset;
    }

    qint64 nResourceSize = qMin(nSize, getSize() - nResourceDirOffset);
    qint64 nResourceEndOffset = nResourceDirOffset + nResourceSize;

    if (nResourceSize < (qint64)sizeof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY)) {
        return;
    }

    struct RESOURCE_DIRECTORY_NODE {
        qint64 nOffset;
        QString sParentTag;
        qint32 nLevel;
    };

    QList<RESOURCE_DIRECTORY_NODE> listDirectories;
    QSet<qint64> stQueuedDirectories;
    QSet<qint64> stVisitedDirectories;
    QSet<qint64> stDataEntries;
    listDirectories.append({nResourceDirOffset, sParentTag, 0});
    stQueuedDirectories.insert(nResourceDirOffset);
    qint32 nHeadersEmitted = 0;

    for (qint32 nDirectoryIndex = 0; (nDirectoryIndex < listDirectories.count()) && (nHeadersEmitted < 0x10000) &&
                                          XBinary::isPdStructNotCanceled(pPdStruct);
         nDirectoryIndex++) {
        const RESOURCE_DIRECTORY_NODE directoryNode = listDirectories.at(nDirectoryIndex);

        if ((directoryNode.nLevel > 16) || stVisitedDirectories.contains(directoryNode.nOffset) ||
            (directoryNode.nOffset < nResourceDirOffset) ||
            ((directoryNode.nOffset + (qint64)sizeof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY)) > nResourceEndOffset)) {
            continue;
        }

        stVisitedDirectories.insert(directoryNode.nOffset);
        XPE_DEF::IMAGE_RESOURCE_DIRECTORY ird = read_IMAGE_RESOURCE_DIRECTORY(directoryNode.nOffset);
        qint32 nNumberOfEntries = (qint32)ird.NumberOfNamedEntries + (qint32)ird.NumberOfIdEntries;

        if ((nNumberOfEntries <= 0) || (nNumberOfEntries > 0x2000)) {
            continue;
        }

        qint64 nEntriesOffset = directoryNode.nOffset + sizeof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY);
        qint64 nEntriesSize = (qint64)nNumberOfEntries * sizeof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY_ENTRY);

        if ((nEntriesOffset < nResourceDirOffset) || (nEntriesSize > (nResourceEndOffset - nEntriesOffset))) {
            continue;
        }

        XFHEADER xfEntryTable = {};
        xfEntryTable.sParentTag = directoryNode.sParentTag;
        xfEntryTable.fileType = xfStruct.fileType;
        xfEntryTable.structID = static_cast<XBinary::STRUCTID>(STRUCTID_IMAGE_RESOURCE_DIRECTORY_ENTRY);
        xfEntryTable.xLoc = offsetToLoc(nEntriesOffset);
        xfEntryTable.xfType = XFTYPE_TABLE;
        xfEntryTable.listFields = getXFRecords(xfStruct.fileType, STRUCTID_IMAGE_RESOURCE_DIRECTORY_ENTRY, xfEntryTable.xLoc);
        xfEntryTable.nSize = sizeof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY_ENTRY);

        for (qint32 i = 0; (i < nNumberOfEntries) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
            qint64 nEntryOffset = nEntriesOffset + (qint64)i * sizeof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY_ENTRY);
            quint32 nRawName = read_uint32(nEntryOffset);
            RESOURCES_ID_NAME resourceName = {};

            if (nRawName & 0x80000000U) {
                qint64 nNameOffset = nResourceDirOffset + (nRawName & 0x7FFFFFFFU);

                if ((nNameOffset >= nResourceDirOffset) && ((nNameOffset + (qint64)sizeof(quint16)) <= nResourceEndOffset)) {
                    quint16 nNameLength = read_uint16(nNameOffset);
                    qint64 nNameSize = (qint64)nNameLength * sizeof(quint16);

                    if ((nNameLength <= 1024) && (nNameSize <= (nResourceEndOffset - nNameOffset - (qint64)sizeof(quint16)))) {
                        QByteArray baName = read_array(nNameOffset + sizeof(quint16), nNameSize);

                        if (baName.size() == nNameSize) {
                            resourceName.bIsName = true;
                            resourceName.nNameOffset = (quint32)(nNameOffset - nResourceDirOffset);
                            resourceName.sName = QString::fromUtf16(reinterpret_cast<const ushort *>(baName.constData()), nNameLength);
                        }
                    }
                }
            } else {
                resourceName.nID = nRawName & 0xFFFFU;
            }

            xfEntryTable.listRowLocations.append(nEntryOffset);

            QString sRowName = resourceIdNameToString(resourceName, directoryNode.nLevel);
            if (sRowName.isEmpty()) {
                sRowName = (nRawName & 0x80000000U) ? QString("Name @ 0x%1").arg(nRawName & 0x7FFFFFFFU, 0, 16)
                                                    : QString::number(nRawName & 0xFFFFU);
            }
            xfEntryTable.listRowNames.append(sRowName);
        }

        if (xfEntryTable.listRowLocations.isEmpty()) {
            continue;
        }

        xfEntryTable.sTag = xfHeaderToTag(xfEntryTable, structIDToString(STRUCTID_IMAGE_RESOURCE_DIRECTORY_ENTRY), xfEntryTable.sParentTag);
        listResult.append(xfEntryTable);
        nHeadersEmitted++;

        for (qint32 i = 0; (i < xfEntryTable.listRowLocations.count()) && (nHeadersEmitted < 0x10000) &&
                              XBinary::isPdStructNotCanceled(pPdStruct);
             i++) {
            qint64 nEntryOffset = xfEntryTable.listRowLocations.at(i);
            quint32 nRawTarget = read_uint32(nEntryOffset + sizeof(quint32));
            qint64 nTargetOffset = nResourceDirOffset + (nRawTarget & 0x7FFFFFFFU);

            if (nRawTarget & 0x80000000U) {
                if ((directoryNode.nLevel >= 16) || stQueuedDirectories.contains(nTargetOffset) || (nTargetOffset < nResourceDirOffset) ||
                    ((nTargetOffset + (qint64)sizeof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY)) > nResourceEndOffset)) {
                    continue;
                }

                XFHEADER xfDirectory = {};
                xfDirectory.sParentTag = xfEntryTable.sTag;
                xfDirectory.fileType = xfStruct.fileType;
                xfDirectory.structID = static_cast<XBinary::STRUCTID>(STRUCTID_IMAGE_RESOURCE_DIRECTORY);
                xfDirectory.xLoc = offsetToLoc(nTargetOffset);
                xfDirectory.nSize = sizeof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY);
                xfDirectory.xfType = XFTYPE_HEADER;
                xfDirectory.listFields = getXFRecords(xfStruct.fileType, STRUCTID_IMAGE_RESOURCE_DIRECTORY, xfDirectory.xLoc);
                xfDirectory.sTag = xfHeaderToTag(xfDirectory, structIDToString(STRUCTID_IMAGE_RESOURCE_DIRECTORY), xfDirectory.sParentTag);
                listResult.append(xfDirectory);
                nHeadersEmitted++;

                listDirectories.append({nTargetOffset, xfDirectory.sTag, directoryNode.nLevel + 1});
                stQueuedDirectories.insert(nTargetOffset);
            } else {
                if (stDataEntries.contains(nTargetOffset) || (nTargetOffset < nResourceDirOffset) ||
                    ((nTargetOffset + (qint64)sizeof(XPE_DEF::IMAGE_RESOURCE_DATA_ENTRY)) > nResourceEndOffset)) {
                    continue;
                }

                XFHEADER xfDataEntry = {};
                xfDataEntry.sParentTag = xfEntryTable.sTag;
                xfDataEntry.fileType = xfStruct.fileType;
                xfDataEntry.structID = static_cast<XBinary::STRUCTID>(STRUCTID_IMAGE_RESOURCE_DATA_ENTRY);
                xfDataEntry.xLoc = offsetToLoc(nTargetOffset);
                xfDataEntry.nSize = sizeof(XPE_DEF::IMAGE_RESOURCE_DATA_ENTRY);
                xfDataEntry.xfType = XFTYPE_HEADER;
                xfDataEntry.listFields = getXFRecords(xfStruct.fileType, STRUCTID_IMAGE_RESOURCE_DATA_ENTRY, xfDataEntry.xLoc);
                xfDataEntry.sTag = xfHeaderToTag(xfDataEntry, structIDToString(STRUCTID_IMAGE_RESOURCE_DATA_ENTRY), xfDataEntry.sParentTag);
                listResult.append(xfDataEntry);
                stDataEntries.insert(nTargetOffset);
                nHeadersEmitted++;
            }
        }
    }
}

void XPE::_appendBaseRelocationBlocks(QList<XFHEADER> &listResult, const XFSTRUCT &xfStruct, qint64 nBaseRelocOffset, qint64 nSize, const QString &sParentTag,
                                      PDSTRUCT *pPdStruct)
{
    if ((nBaseRelocOffset < 0) || (nSize <= 0) || (nBaseRelocOffset >= getSize())) {
        return;
    }

    qint64 nAvailableSize = qMin(nSize, getSize() - nBaseRelocOffset);
    qint64 nEndOffset = nBaseRelocOffset + nAvailableSize;
    qint64 nCurrentOffset = nBaseRelocOffset;
    qint32 nBlocks = 0;

    while (((nCurrentOffset + (qint64)sizeof(XPE_DEF::IMAGE_BASE_RELOCATION)) <= nEndOffset) && (nBlocks < 0x10000) &&
           XBinary::isPdStructNotCanceled(pPdStruct)) {
        quint32 nSizeOfBlock = read_uint32(nCurrentOffset + offsetof(XPE_DEF::IMAGE_BASE_RELOCATION, SizeOfBlock));

        if ((nSizeOfBlock < sizeof(XPE_DEF::IMAGE_BASE_RELOCATION)) || ((qint64)nSizeOfBlock > (nEndOffset - nCurrentOffset))) {
            break;
        }

        XFHEADER xfBlock = {};
        xfBlock.sParentTag = sParentTag;
        xfBlock.fileType = xfStruct.fileType;
        xfBlock.structID = static_cast<XBinary::STRUCTID>(STRUCTID_IMAGE_BASE_RELOCATION);
        xfBlock.xLoc = offsetToLoc(nCurrentOffset);
        xfBlock.nSize = sizeof(XPE_DEF::IMAGE_BASE_RELOCATION);
        xfBlock.xfType = XFTYPE_HEADER;
        xfBlock.listFields = getXFRecords(xfStruct.fileType, STRUCTID_IMAGE_BASE_RELOCATION, xfBlock.xLoc);
        xfBlock.sTag = xfHeaderToTag(xfBlock, structIDToString(STRUCTID_IMAGE_BASE_RELOCATION), xfBlock.sParentTag);
        listResult.append(xfBlock);

        qint32 nEntries = (qint32)qMin((qint64)0x10000,
                                       ((qint64)nSizeOfBlock - (qint64)sizeof(XPE_DEF::IMAGE_BASE_RELOCATION)) / (qint64)sizeof(quint16));

        if ((nEntries > 0) && xfStruct.bIsParent) {
            XFHEADER xfEntries = {};
            xfEntries.sParentTag = xfBlock.sTag;
            xfEntries.fileType = xfStruct.fileType;
            xfEntries.structID = static_cast<XBinary::STRUCTID>(STRUCTID_IMAGE_BASE_RELOCATION_ENTRY);
            qint64 nEntriesOffset = nCurrentOffset + sizeof(XPE_DEF::IMAGE_BASE_RELOCATION);
            xfEntries.xLoc = offsetToLoc(nEntriesOffset);
            xfEntries.xfType = XFTYPE_TABLE;
            xfEntries.listFields = getXFRecords(xfStruct.fileType, STRUCTID_IMAGE_BASE_RELOCATION_ENTRY, xfEntries.xLoc);
            xfEntries.nSize = sizeof(quint16);

            for (qint32 i = 0; (i < nEntries) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
                xfEntries.listRowLocations.append(nEntriesOffset + (qint64)i * sizeof(quint16));
            }

            if (!xfEntries.listRowLocations.isEmpty()) {
                _decorateXPEXFHeader(&xfEntries);
                xfEntries.sTag = xfHeaderToTag(xfEntries, structIDToString(STRUCTID_IMAGE_BASE_RELOCATION_ENTRY), xfEntries.sParentTag);
                listResult.append(xfEntries);
            }
        }

        nCurrentOffset += nSizeOfBlock;
        nBlocks++;

        if (!xfStruct.bIsParent) {
            break;
        }
    }
}

void XPE::_appendBoundImportRecords(QList<XFHEADER> &listResult, const XFSTRUCT &xfStruct, qint64 nBoundImportOffset, qint64 nSize,
                                    const QString &sParentTag, PDSTRUCT *pPdStruct)
{
    if ((nBoundImportOffset < 0) || (nSize <= 0) || (nBoundImportOffset >= getSize())) {
        return;
    }

    qint64 nAvailableSize = qMin(nSize, getSize() - nBoundImportOffset);
    qint64 nEndOffset = nBoundImportOffset + nAvailableSize;
    qint64 nCurrentOffset = nBoundImportOffset;
    qint32 nDescriptors = 0;

    while (((nCurrentOffset + (qint64)sizeof(XPE_DEF::IMAGE_BOUND_IMPORT_DESCRIPTOR)) <= nEndOffset) && (nDescriptors < 0x10000) &&
           XBinary::isPdStructNotCanceled(pPdStruct)) {
        QByteArray baDescriptor = read_array(nCurrentOffset, sizeof(XPE_DEF::IMAGE_BOUND_IMPORT_DESCRIPTOR));

        if ((baDescriptor.size() != (qint32)sizeof(XPE_DEF::IMAGE_BOUND_IMPORT_DESCRIPTOR)) ||
            (baDescriptor == QByteArray(sizeof(XPE_DEF::IMAGE_BOUND_IMPORT_DESCRIPTOR), 0))) {
            break;
        }

        quint16 nForwarderRefs = read_uint16(nCurrentOffset + offsetof(XPE_DEF::IMAGE_BOUND_IMPORT_DESCRIPTOR, NumberOfModuleForwarderRefs));
        qint64 nRecordSize = (qint64)sizeof(XPE_DEF::IMAGE_BOUND_IMPORT_DESCRIPTOR) +
                             (qint64)nForwarderRefs * sizeof(XPE_DEF::IMAGE_BOUND_FORWARDER_REF);

        if ((nRecordSize <= 0) || (nRecordSize > (nEndOffset - nCurrentOffset))) {
            break;
        }

        XFHEADER xfDescriptor = {};
        xfDescriptor.sParentTag = sParentTag;
        xfDescriptor.fileType = xfStruct.fileType;
        xfDescriptor.structID = static_cast<XBinary::STRUCTID>(STRUCTID_IMAGE_BOUND_IMPORT_DESCRIPTOR);
        xfDescriptor.xLoc = offsetToLoc(nCurrentOffset);
        xfDescriptor.nSize = sizeof(XPE_DEF::IMAGE_BOUND_IMPORT_DESCRIPTOR);
        xfDescriptor.xfType = XFTYPE_HEADER;
        xfDescriptor.listFields = getXFRecords(xfStruct.fileType, STRUCTID_IMAGE_BOUND_IMPORT_DESCRIPTOR, xfDescriptor.xLoc);
        xfDescriptor.sTag = xfHeaderToTag(xfDescriptor, structIDToString(STRUCTID_IMAGE_BOUND_IMPORT_DESCRIPTOR), xfDescriptor.sParentTag);
        listResult.append(xfDescriptor);

        if ((nForwarderRefs > 0) && xfStruct.bIsParent) {
            qint64 nForwardersOffset = nCurrentOffset + sizeof(XPE_DEF::IMAGE_BOUND_IMPORT_DESCRIPTOR);
            XFHEADER xfForwarders = {};
            xfForwarders.sParentTag = xfDescriptor.sTag;
            xfForwarders.fileType = xfStruct.fileType;
            xfForwarders.structID = static_cast<XBinary::STRUCTID>(STRUCTID_IMAGE_BOUND_FORWARDER_REF);
            xfForwarders.xLoc = offsetToLoc(nForwardersOffset);
            xfForwarders.nSize = sizeof(XPE_DEF::IMAGE_BOUND_FORWARDER_REF);
            xfForwarders.xfType = XFTYPE_TABLE;
            xfForwarders.listFields = getXFRecords(xfStruct.fileType, STRUCTID_IMAGE_BOUND_FORWARDER_REF, xfForwarders.xLoc);

            for (qint32 i = 0; (i < nForwarderRefs) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
                qint64 nForwarderOffset = nForwardersOffset + (qint64)i * sizeof(XPE_DEF::IMAGE_BOUND_FORWARDER_REF);
                xfForwarders.listRowLocations.append(nForwarderOffset);

                quint16 nNameOffset = read_uint16(nForwarderOffset + offsetof(XPE_DEF::IMAGE_BOUND_FORWARDER_REF, OffsetModuleName));
                qint64 nNameFileOffset = nBoundImportOffset + nNameOffset;
                QString sName;

                if ((nNameOffset != 0) && (nNameFileOffset >= nBoundImportOffset) && (nNameFileOffset < nEndOffset)) {
                    sName = read_ansiString(nNameFileOffset, qMin((qint64)2048, nEndOffset - nNameFileOffset));
                }

                xfForwarders.listRowNames.append(sName);
            }

            if (!xfForwarders.listRowLocations.isEmpty()) {
                xfForwarders.sTag = xfHeaderToTag(xfForwarders, structIDToString(STRUCTID_IMAGE_BOUND_FORWARDER_REF), xfForwarders.sParentTag);
                listResult.append(xfForwarders);
            }
        }

        nCurrentOffset += nRecordSize;
        nDescriptors++;

        if (!xfStruct.bIsParent) {
            break;
        }
    }
}

void XPE::_appendWinCertRecords(QList<XFHEADER> &listResult, const XFSTRUCT &xfStruct, qint64 nCertOffset, qint64 nSize, const QString &sParentTag, PDSTRUCT *pPdStruct)
{
    if ((nCertOffset < 0) || (nSize <= 0) || (nCertOffset >= getSize())) {
        return;
    }

    qint64 nAvailableSize = qMin(nSize, getSize() - nCertOffset);
    qint64 nEndOffset = nCertOffset + nAvailableSize;
    qint64 nCurrentOffset = nCertOffset;
    qint32 nRecords = 0;

    while (((nCurrentOffset + (qint64)sizeof(XPE_DEF::WIN_CERT_RECORD)) <= nEndOffset) && (nRecords < 0x10000) &&
           XBinary::isPdStructNotCanceled(pPdStruct)) {
        quint32 nLength = read_uint32(nCurrentOffset + offsetof(XPE_DEF::WIN_CERT_RECORD, dwLength));

        if (nLength < sizeof(XPE_DEF::WIN_CERT_RECORD)) {
            break;
        }

        qint64 nAlignedLength = ((qint64)nLength + 7) & ~(qint64)7;

        if (((qint64)nLength > (nEndOffset - nCurrentOffset)) || (nAlignedLength > (nEndOffset - nCurrentOffset))) {
            break;
        }

        XFHEADER xfCertificate = {};
        xfCertificate.sParentTag = sParentTag;
        xfCertificate.fileType = xfStruct.fileType;
        xfCertificate.structID = static_cast<XBinary::STRUCTID>(STRUCTID_WIN_CERT_RECORD);
        xfCertificate.xLoc = offsetToLoc(nCurrentOffset);
        xfCertificate.nSize = nLength;
        xfCertificate.xfType = XFTYPE_HEADER;
        xfCertificate.listFields = getXFRecords(xfStruct.fileType, STRUCTID_WIN_CERT_RECORD, xfCertificate.xLoc);
        xfCertificate.sTag = xfHeaderToTag(xfCertificate, structIDToString(STRUCTID_WIN_CERT_RECORD), xfCertificate.sParentTag);
        listResult.append(xfCertificate);

        nCurrentOffset += nAlignedLength;  // Records are 8-byte aligned
        nRecords++;

        if (!xfStruct.bIsParent) {
            break;
        }
    }
}

void XPE::_appendNetMetadata(QList<XFHEADER> &listResult, const XFSTRUCT &xfStruct, const QString &sParentTag, PDSTRUCT *pPdStruct, qint64 nMetadataOffset,
                             qint64 nMetadataSize)
{
    if (nMetadataOffset == -1) {
        OFFSETSIZE osMetadata = getNet_MetadataOffsetSize();
        nMetadataOffset = osMetadata.nOffset;
        nMetadataSize = osMetadata.nSize;
    }

    if ((nMetadataOffset < 0) || (nMetadataOffset >= getSize())) {
        return;
    }

    if (nMetadataSize <= 0) {
        nMetadataSize = getSize() - nMetadataOffset;
    }

    qint64 nAvailableSize = qMin(nMetadataSize, getSize() - nMetadataOffset);

    if ((nAvailableSize < 20) || (read_uint32(nMetadataOffset) != 0x424A5342)) {  // 'BSJB'
        return;
    }

    quint32 nVersionStringLength = read_uint32(nMetadataOffset + 12);

    if ((nVersionStringLength > 0x10000) || ((qint64)nVersionStringLength + 20 > nAvailableSize)) {
        return;
    }

    qint64 nMetadataHeaderSize = 20 + nVersionStringLength;
    quint16 nDeclaredStreams = read_uint16(nMetadataOffset + 18 + nVersionStringLength);

    XLOC metadataLoc = offsetToLoc(nMetadataOffset);

    XFHEADER xfHeader = {};
    xfHeader.sParentTag = sParentTag;
    xfHeader.fileType = xfStruct.fileType;
    xfHeader.structID = static_cast<XBinary::STRUCTID>(STRUCTID_NET_METADATA);
    xfHeader.xLoc = metadataLoc;
    xfHeader.nSize = nMetadataHeaderSize;
    xfHeader.xfType = XFTYPE_HEADER;
    xfHeader.listFields = getXFRecords(xfStruct.fileType, STRUCTID_NET_METADATA, metadataLoc);
    _filterXFRecordsBySize(&xfHeader.listFields, nMetadataHeaderSize);
    _decorateXPEXFHeader(&xfHeader);
    xfHeader.sTag = xfHeaderToTag(xfHeader, structIDToString(STRUCTID_NET_METADATA), xfHeader.sParentTag);
    listResult.append(xfHeader);

    if (!xfStruct.bIsParent) {
        return;
    }

    qint32 nNumberOfStreams = qMin((qint32)nDeclaredStreams, 0x100);
    qint64 nEndOffset = nMetadataOffset + nAvailableSize;
    qint64 nCurrentOffset = nMetadataOffset + nMetadataHeaderSize;

    for (qint32 i = 0; (i < nNumberOfStreams) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        if ((nCurrentOffset + 8) > nEndOffset) {
            break;
        }

        qint64 nNameBytes = qMin((qint64)32, nEndOffset - nCurrentOffset - 8);
        QByteArray baName = read_array(nCurrentOffset + 8, nNameBytes);
        qint32 nTerminator = baName.indexOf('\0');

        if (nTerminator == -1) {
            break;
        }

        qint32 nNameStorageSize = (nTerminator + 1 + 3) & ~3;
        qint64 nStreamHeaderSize = 8 + nNameStorageSize;

        if ((nCurrentOffset + nStreamHeaderSize) > nEndOffset) {
            break;
        }

        XFHEADER xfStream = {};
        xfStream.sParentTag = xfHeader.sTag;
        xfStream.fileType = xfStruct.fileType;
        xfStream.structID = static_cast<XBinary::STRUCTID>(STRUCTID_NET_METADATA_STREAM);
        xfStream.xLoc = offsetToLoc(nCurrentOffset);
        xfStream.nSize = nStreamHeaderSize;
        xfStream.xfType = XFTYPE_HEADER;
        xfStream.listFields = getXFRecords(xfStruct.fileType, STRUCTID_NET_METADATA_STREAM, xfStream.xLoc);
        _filterXFRecordsBySize(&xfStream.listFields, nStreamHeaderSize);
        xfStream.sTag = xfHeaderToTag(xfStream, structIDToString(STRUCTID_NET_METADATA_STREAM), xfStream.sParentTag);
        listResult.append(xfStream);

        nCurrentOffset += nStreamHeaderSize;
    }
}

void XPE::_appendCor20Children(QList<XFHEADER> &listResult, const XFSTRUCT &xfStruct, qint64 nCor20Offset, qint64 nSize, const QString &sParentTag,
                               PDSTRUCT *pPdStruct)
{
    if ((nCor20Offset < 0) || (nCor20Offset >= getSize())) {
        return;
    }

    qint64 nAvailableSize = getSize() - nCor20Offset;
    if (nSize > 0) {
        nAvailableSize = qMin(nAvailableSize, nSize);
    }

    nAvailableSize = _getXPECor20HeaderSize(this, nCor20Offset, nAvailableSize);

    if (nAvailableSize <= 0) {
        return;
    }

    struct COR20_DIRECTORY_RECORD {
        quint32 nStructID;
        qint32 nOffset;
    };

    const COR20_DIRECTORY_RECORD records[] = {
        {STRUCTID_NET_METADATA, (qint32)offsetof(XPE_DEF::IMAGE_COR20_HEADER, MetaData)},
        {STRUCTID_NET_RESOURCES, (qint32)offsetof(XPE_DEF::IMAGE_COR20_HEADER, Resources)},
        {STRUCTID_NET_STRONGNAMESIGNATURE, (qint32)offsetof(XPE_DEF::IMAGE_COR20_HEADER, StrongNameSignature)},
        {STRUCTID_NET_CODEMANAGERTABLE, (qint32)offsetof(XPE_DEF::IMAGE_COR20_HEADER, CodeManagerTable)},
        {STRUCTID_NET_VTABLEFIXUPS, (qint32)offsetof(XPE_DEF::IMAGE_COR20_HEADER, VTableFixups)},
        {STRUCTID_NET_EXPORTADDRESSTABLEJUMPS, (qint32)offsetof(XPE_DEF::IMAGE_COR20_HEADER, ExportAddressTableJumps)},
        {STRUCTID_NET_MANAGEDNATIVEHEADER, (qint32)offsetof(XPE_DEF::IMAGE_COR20_HEADER, ManagedNativeHeader)},
    };

    for (const COR20_DIRECTORY_RECORD &record : records) {
        if (!XBinary::isPdStructNotCanceled(pPdStruct) || ((qint64)record.nOffset + (qint64)sizeof(XPE_DEF::IMAGE_DATA_DIRECTORY) > nAvailableSize)) {
            break;
        }

        qint64 nDirectoryOffset = nCor20Offset + record.nOffset;
        quint32 nRVA = read_uint32(nDirectoryOffset + offsetof(XPE_DEF::IMAGE_DATA_DIRECTORY, VirtualAddress));
        quint32 nDirectorySize = read_uint32(nDirectoryOffset + offsetof(XPE_DEF::IMAGE_DATA_DIRECTORY, Size));

        XPE_DEF::IMAGE_DATA_DIRECTORY dataDirectory = {};
        dataDirectory.VirtualAddress = nRVA;
        dataDirectory.Size = nDirectorySize;

        if (!isDataDirectoryValid(&dataDirectory, xfStruct.pMemoryMap)) {
            continue;
        }

        qint64 nDataOffset = relAddressToOffset(xfStruct.pMemoryMap, nRVA);

        if ((nDataOffset < 0) || (nDataOffset >= getSize())) {
            continue;
        }

        qint64 nDataSize = qMin((qint64)nDirectorySize, getSize() - nDataOffset);

        if (nDataSize <= 0) {
            continue;
        }

        if (record.nStructID == STRUCTID_NET_METADATA) {
            _appendNetMetadata(listResult, xfStruct, sParentTag, pPdStruct, nDataOffset, nDataSize);
            continue;
        }

        qint32 nRecordSize = (qint32)qMin(nDataSize, (qint64)0x7FFFFFFF);
        XFHEADER xfData = {};
        xfData.sParentTag = sParentTag;
        xfData.fileType = xfStruct.fileType;
        xfData.structID = static_cast<XBinary::STRUCTID>(record.nStructID);
        xfData.xLoc = offsetToLoc(nDataOffset);
        xfData.nSize = nRecordSize;
        xfData.xfType = XFTYPE_HEADER;
        xfData.listFields.append({"Data", 0, nRecordSize, XFRECORD_FLAG_NONE, VT_BYTE_ARRAY});
        xfData.sTag = xfHeaderToTag(xfData, structIDToString(record.nStructID), xfData.sParentTag);
        listResult.append(xfData);
    }
}

void XPE::_decorateImportThunkTable(XFHEADER *pXfHeader, const XFSTRUCT &xfStruct, PDSTRUCT *pPdStruct)
{
    if ((pXfHeader == nullptr) || (xfStruct.pMemoryMap == nullptr) || pXfHeader->listRowLocations.isEmpty()) {
        return;
    }

    QList<IMPORT_HEADER> listImports = getImports(xfStruct.pMemoryMap, pPdStruct);

    if (listImports.isEmpty() || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return;
    }

    QList<QString> listRowNames;

    XBinary::XFCOLUMN xfDllColumn = {};
    xfDllColumn.sName = QString("DLL");

    XBinary::XFCOLUMN xfHintColumn = {};
    xfHintColumn.sName = QString("Hint");

    XBinary::XFCOLUMN xfOrdinalColumn = {};
    xfOrdinalColumn.sName = QString("Ordinal");

    XBinary::XFCOLUMN xfKindColumn = {};
    xfKindColumn.sName = QString("Value kind");

    struct IMPORT_DISPLAY_RECORD {
        QString sDll;
        QString sFunction;
        QString sHint;
        QString sOrdinal;
        QString sKind;
    };

    QMap<qint64, IMPORT_DISPLAY_RECORD> mapImportRecords;

    for (qint32 i = 0; (i < listImports.count()) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        const IMPORT_HEADER &importHeader = listImports.at(i);

        for (qint32 j = 0; (j < importHeader.listPositions.count()) && XBinary::isPdStructNotCanceled(pPdStruct); j++) {
            const IMPORT_POSITION &importPosition = importHeader.listPositions.at(j);

            if (mapImportRecords.contains(importPosition.nThunkOffset)) {
                continue;
            }

            IMPORT_DISPLAY_RECORD displayRecord = {};
            displayRecord.sDll = importHeader.sName;
            displayRecord.sFunction = importPosition.sFunction;

            if (displayRecord.sFunction.isEmpty()) {
                displayRecord.sFunction = importPosition.sName;
            }

            if (displayRecord.sFunction.isEmpty() && (importPosition.nOrdinal != 0)) {
                displayRecord.sFunction = QString::number(importPosition.nOrdinal);
            }

            if (importPosition.nOrdinal == 0) {
                displayRecord.sHint = QString::number(importPosition.nHint);
                displayRecord.sKind = QString("Name RVA");
            } else {
                displayRecord.sOrdinal = QString::number(importPosition.nOrdinal);
                displayRecord.sKind = QString("Ordinal");
            }

            mapImportRecords.insert(importPosition.nThunkOffset, displayRecord);
        }
    }

    if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
        return;
    }

    bool bHasImportInfo = false;

    for (qint32 nRow = 0; (nRow < pXfHeader->listRowLocations.count()) && XBinary::isPdStructNotCanceled(pPdStruct); nRow++) {
        qint64 nThunkOffset = pXfHeader->listRowLocations.at(nRow);
        QMap<qint64, IMPORT_DISPLAY_RECORD>::const_iterator it = mapImportRecords.constFind(nThunkOffset);
        IMPORT_DISPLAY_RECORD displayRecord = {};

        if (it != mapImportRecords.constEnd()) {
            displayRecord = it.value();
            bHasImportInfo = true;
        }

        listRowNames.append(displayRecord.sFunction);
        xfDllColumn.listValues.append(displayRecord.sDll);
        xfHintColumn.listValues.append(displayRecord.sHint);
        xfOrdinalColumn.listValues.append(displayRecord.sOrdinal);
        xfKindColumn.listValues.append(displayRecord.sKind);
    }

    if (bHasImportInfo && (listRowNames.count() == pXfHeader->listRowLocations.count())) {
        pXfHeader->listRowNames = listRowNames;
        pXfHeader->listExtraColumns.append(xfDllColumn);
        pXfHeader->listExtraColumns.append(xfHintColumn);
        pXfHeader->listExtraColumns.append(xfOrdinalColumn);
        pXfHeader->listExtraColumns.append(xfKindColumn);
    }
}

void XPE::_decorateDelayImportTable(XFHEADER *pXfHeader, const XFSTRUCT &xfStruct, PDSTRUCT *pPdStruct)
{
    if ((pXfHeader == nullptr) || (xfStruct.pMemoryMap == nullptr) || pXfHeader->listRowLocations.isEmpty()) {
        return;
    }

    QList<QString> listRowNames;
    XBinary::XFCOLUMN xfKindColumn = {};
    xfKindColumn.sName = QString("Pointer kind");
    XBinary::XFCOLUMN xfOffsetColumn = {};
    xfOffsetColumn.sName = QString("DLL file offset");

    for (qint32 i = 0; (i < pXfHeader->listRowLocations.count()) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        qint64 nRowOffset = pXfHeader->listRowLocations.at(i);
        quint32 nAttributes = read_uint32(nRowOffset + offsetof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR, AllAttributes));
        quint32 nDllName = read_uint32(nRowOffset + offsetof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR, DllNameRVA));
        bool bRvaBased = (nAttributes & 1U) != 0;
        qint64 nNameOffset = -1;

        if (nDllName != 0) {
            nNameOffset = bRvaBased ? XBinary::relAddressToOffset(xfStruct.pMemoryMap, nDllName)
                                    : XBinary::addressToOffset(xfStruct.pMemoryMap, nDllName);
        }

        QString sName;
        if ((nNameOffset >= 0) && (nNameOffset < getSize())) {
            sName = read_ansiString(nNameOffset, qMin((qint64)2048, getSize() - nNameOffset));
        }

        listRowNames.append(sName);
        xfKindColumn.listValues.append(bRvaBased ? QString("RVA") : QString("VA"));
        xfOffsetColumn.listValues.append(nNameOffset >= 0 ? QString("0x%1").arg((quint64)nNameOffset, 0, 16).toUpper() : QString());
    }

    if (listRowNames.count() != pXfHeader->listRowLocations.count()) {
        return;
    }

    pXfHeader->listRowNames = listRowNames;
    pXfHeader->listExtraColumns.append(xfKindColumn);
    pXfHeader->listExtraColumns.append(xfOffsetColumn);
}

void XPE::_appendImportThunks(QList<XFHEADER> &listResult, const XFSTRUCT &xfStruct, const QString &sParentTag, PDSTRUCT *pPdStruct)
{
    if (xfStruct.pMemoryMap == nullptr) {
        return;
    }

    QList<IMPORT_HEADER> listImports = getImports(xfStruct.pMemoryMap, pPdStruct);

    if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
        return;
    }

    bool bIs64 = is64(xfStruct.pMemoryMap);
    quint32 nThunkStructID = bIs64 ? STRUCTID_IMAGE_THUNK_DATA64 : STRUCTID_IMAGE_THUNK_DATA32;
    qint32 nThunkSize = bIs64 ? 8 : 4;

    for (qint32 i = 0; (i < listImports.count()) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        const IMPORT_HEADER &importHeader = listImports.at(i);

        if (importHeader.listPositions.isEmpty()) {
            continue;
        }

        XFHEADER xfThunksTable = {};
        xfThunksTable.sParentTag = sParentTag;
        xfThunksTable.fileType = xfStruct.fileType;
        xfThunksTable.structID = static_cast<XBinary::STRUCTID>(nThunkStructID);
        xfThunksTable.xfType = XFTYPE_TABLE;
        xfThunksTable.nSize = nThunkSize;

        XBinary::XFCOLUMN xfDllColumn = {};
        xfDllColumn.sName = QString("DLL");

        XBinary::XFCOLUMN xfHintColumn = {};
        xfHintColumn.sName = QString("Hint");

        XBinary::XFCOLUMN xfOrdinalColumn = {};
        xfOrdinalColumn.sName = QString("Ordinal");

        XBinary::XFCOLUMN xfKindColumn = {};
        xfKindColumn.sName = QString("Value kind");

        for (qint32 j = 0; (j < importHeader.listPositions.count()) && XBinary::isPdStructNotCanceled(pPdStruct); j++) {
            const IMPORT_POSITION &importPosition = importHeader.listPositions.at(j);

            if ((importPosition.nThunkOffset == -1) || (!checkOffsetSize(importPosition.nThunkOffset, nThunkSize))) {
                continue;
            }

            QString sFunction = importPosition.sFunction;

            if (sFunction.isEmpty()) {
                sFunction = importPosition.sName;
            }

            if (sFunction.isEmpty() && (importPosition.nOrdinal != 0)) {
                sFunction = QString::number(importPosition.nOrdinal);
            }

            xfThunksTable.listRowLocations.append(importPosition.nThunkOffset);
            xfThunksTable.listRowNames.append(sFunction);
            xfDllColumn.listValues.append(importHeader.sName);

            if (importPosition.nOrdinal == 0) {
                xfHintColumn.listValues.append(QString::number(importPosition.nHint));
                xfOrdinalColumn.listValues.append(QString());
                xfKindColumn.listValues.append(QString("Name RVA"));
            } else {
                xfHintColumn.listValues.append(QString());
                xfOrdinalColumn.listValues.append(QString::number(importPosition.nOrdinal));
                xfKindColumn.listValues.append(QString("Ordinal"));
            }
        }

        if (xfThunksTable.listRowLocations.isEmpty()) {
            continue;
        }

        xfThunksTable.xLoc = offsetToLoc(xfThunksTable.listRowLocations.at(0));
        xfThunksTable.listFields = getXFRecords(xfStruct.fileType, nThunkStructID, xfThunksTable.xLoc);
        xfThunksTable.listExtraColumns.append(xfDllColumn);
        xfThunksTable.listExtraColumns.append(xfHintColumn);
        xfThunksTable.listExtraColumns.append(xfOrdinalColumn);
        xfThunksTable.listExtraColumns.append(xfKindColumn);
        xfThunksTable.sTag = xfHeaderToTag(xfThunksTable, structIDToString(nThunkStructID), sParentTag);
        listResult.append(xfThunksTable);
    }
}

QList<XBinary::XFHEADER> XPE::getXFHeaders(const XFSTRUCT &xfStruct, PDSTRUCT *pPdStruct)
{
    QList<XBinary::XFHEADER> listResult;

    quint32 nStructID = xfStruct.nStructID;

    if (nStructID == 0) {
        XFSTRUCT dosStruct = xfStruct;
        dosStruct.nStructID = XMSDOS::STRUCTID_IMAGE_DOS_HEADEREX;
        dosStruct.xLoc = offsetToLoc(0);
        dosStruct.fileType = FT_MSDOS;
        dosStruct.bIsParent = false;
        dosStruct.nSize = 0;
        dosStruct.nCount = 0;

        QList<XFHEADER> listDosHeaders = XMSDOS::getXFHeaders(dosStruct, pPdStruct);
        listResult.append(listDosHeaders);

        if (xfStruct.bIsParent) {
            qint64 nNtHeadersOffset = getNtHeadersOffset();

            if (nNtHeadersOffset != -1) {
                XFSTRUCT ntStruct = xfStruct;
                ntStruct.sParent = listDosHeaders.isEmpty() ? QString() : listDosHeaders.last().sTag;
                ntStruct.nStructID = is64() ? STRUCTID_IMAGE_NT_HEADERS64 : STRUCTID_IMAGE_NT_HEADERS32;
                ntStruct.xLoc = offsetToLoc(nNtHeadersOffset);
                ntStruct.nSize = 0;
                ntStruct.nCount = 0;

                listResult.append(getXFHeaders(ntStruct, pPdStruct));
            }
        }
    } else if (nStructID == STRUCTID_IMAGE_DOS_HEADER) {
        XFSTRUCT dosStruct = xfStruct;
        dosStruct.nStructID = XMSDOS::STRUCTID_IMAGE_DOS_HEADEREX;
        dosStruct.fileType = FT_MSDOS;
        dosStruct.bIsParent = false;

        if (locToOffset(dosStruct.pMemoryMap, dosStruct.xLoc) == -1) {
            dosStruct.xLoc = offsetToLoc(0);
        }

        listResult = XMSDOS::getXFHeaders(dosStruct, pPdStruct);

        if (xfStruct.bIsParent && !listResult.isEmpty()) {
            qint64 nNtHeadersOffset = getNtHeadersOffset();

            if (nNtHeadersOffset != -1) {
                XFSTRUCT ntStruct = xfStruct;
                ntStruct.sParent = listResult.last().sTag;
                ntStruct.nStructID = is64() ? STRUCTID_IMAGE_NT_HEADERS64 : STRUCTID_IMAGE_NT_HEADERS32;
                ntStruct.xLoc = offsetToLoc(nNtHeadersOffset);
                ntStruct.nSize = 0;
                ntStruct.nCount = 0;
                listResult.append(getXFHeaders(ntStruct, pPdStruct));
            }
        }
    } else if ((nStructID == STRUCTID_IMAGE_NT_HEADERS32) || (nStructID == STRUCTID_IMAGE_NT_HEADERS64)) {
        bool bIs64 = (nStructID == STRUCTID_IMAGE_NT_HEADERS64);
        qint64 nNtHeadersOffset = locToOffset(xfStruct.pMemoryMap, xfStruct.xLoc);

        if (nNtHeadersOffset == -1) {
            nNtHeadersOffset = getNtHeadersOffset();
        }

        if ((nNtHeadersOffset == -1) || !checkOffsetSize(nNtHeadersOffset, sizeof(quint32)) ||
            ((xfStruct.nSize > 0) && (xfStruct.nSize < (qint64)sizeof(quint32)))) {
            return listResult;
        }

        XLOC ntLoc = offsetToLoc(nNtHeadersOffset);

        // NT Headers header (contains just Signature)
        XFHEADER xfHeader = {};
        xfHeader.sParentTag = xfStruct.sParent;
        xfHeader.fileType = xfStruct.fileType;
        xfHeader.structID = static_cast<XBinary::STRUCTID>(nStructID);
        xfHeader.xLoc = ntLoc;
        xfHeader.xfType = XFTYPE_HEADER;
        xfHeader.listFields = getXFRecords(xfStruct.fileType, nStructID, ntLoc);
        xfHeader.nSize = sizeof(quint32);
        _decorateXPEXFHeader(&xfHeader);
        xfHeader.sTag = xfHeaderToTag(xfHeader, structIDToString(nStructID), xfHeader.sParentTag);

        listResult.append(xfHeader);

        if (xfStruct.bIsParent) {
            qint64 nFileHeaderOffset = nNtHeadersOffset + sizeof(quint32);

            if (checkOffsetSize(nFileHeaderOffset, sizeof(XPE_DEF::IMAGE_FILE_HEADER))) {
                XFSTRUCT fhStruct = xfStruct;
                fhStruct.sParent = xfHeader.sTag;
                fhStruct.nStructID = STRUCTID_IMAGE_FILE_HEADER;
                fhStruct.xLoc = offsetToLoc(nFileHeaderOffset);
                fhStruct.nSize = sizeof(XPE_DEF::IMAGE_FILE_HEADER);
                fhStruct.bIsParent = false;

                listResult.append(getXFHeaders(fhStruct, pPdStruct));

                qint32 nNumberOfSections = qMin((qint32)read_uint16(nFileHeaderOffset + offsetof(XPE_DEF::IMAGE_FILE_HEADER, NumberOfSections)),
                                                 XPE_DEF::S_MAX_SECTIONCOUNT);
                quint16 nOptionalHeaderSize = read_uint16(nFileHeaderOffset + offsetof(XPE_DEF::IMAGE_FILE_HEADER, SizeOfOptionalHeader));
                qint64 nOptionalHeaderOffset = nFileHeaderOffset + sizeof(XPE_DEF::IMAGE_FILE_HEADER);
                qint64 nOptionalHeaderFixedSize = bIs64 ? (qint64)sizeof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S) : (qint64)sizeof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S);

                if ((nOptionalHeaderSize > 0) && checkOffsetSize(nOptionalHeaderOffset, nOptionalHeaderSize)) {
                    XFSTRUCT ohStruct = xfStruct;
                    ohStruct.sParent = xfHeader.sTag;
                    ohStruct.nStructID = bIs64 ? STRUCTID_IMAGE_OPTIONAL_HEADER64 : STRUCTID_IMAGE_OPTIONAL_HEADER32;
                    ohStruct.xLoc = offsetToLoc(nOptionalHeaderOffset);
                    ohStruct.nSize = qMin((qint64)nOptionalHeaderSize, nOptionalHeaderFixedSize);

                    listResult.append(getXFHeaders(ohStruct, pPdStruct));
                }

                qint64 nSectionsTableOffset = nOptionalHeaderOffset + nOptionalHeaderSize;

                if ((nNumberOfSections > 0) && checkOffsetSize(nSectionsTableOffset, (qint64)nNumberOfSections * sizeof(XPE_DEF::IMAGE_SECTION_HEADER))) {
                    XFSTRUCT sectionStruct = xfStruct;
                    sectionStruct.sParent = xfHeader.sTag;
                    sectionStruct.nStructID = STRUCTID_IMAGE_SECTION_HEADER;
                    sectionStruct.xLoc = offsetToLoc(nSectionsTableOffset);
                    sectionStruct.nSize = sizeof(XPE_DEF::IMAGE_SECTION_HEADER);
                    sectionStruct.nCount = nNumberOfSections;
                    sectionStruct.bIsParent = false;

                    listResult.append(getXFHeaders(sectionStruct, pPdStruct));
                }
            }
        }
    } else if (nStructID == STRUCTID_IMAGE_FILE_HEADER) {
        qint64 nFileHeaderOffset = locToOffset(xfStruct.pMemoryMap, xfStruct.xLoc);

        if (nFileHeaderOffset == -1) {
            nFileHeaderOffset = getFileHeaderOffset();
        }

        if (!checkOffsetSize(nFileHeaderOffset, sizeof(XPE_DEF::IMAGE_FILE_HEADER))) {
            return listResult;
        }

        XLOC fhLoc = offsetToLoc(nFileHeaderOffset);

        XFHEADER xfHeader = {};
        xfHeader.sParentTag = xfStruct.sParent;
        xfHeader.fileType = xfStruct.fileType;
        xfHeader.structID = static_cast<XBinary::STRUCTID>(STRUCTID_IMAGE_FILE_HEADER);
        xfHeader.xLoc = fhLoc;
        xfHeader.xfType = XFTYPE_HEADER;
        xfHeader.listFields = getXFRecords(xfStruct.fileType, STRUCTID_IMAGE_FILE_HEADER, fhLoc);
        _filterXFRecordsBySize(&xfHeader.listFields, xfStruct.nSize);
        xfHeader.nSize = _getXFHeaderSize(xfStruct.nSize, xfHeader.listFields);
        _decorateXPEXFHeader(&xfHeader);
        xfHeader.sTag = xfHeaderToTag(xfHeader, structIDToString(STRUCTID_IMAGE_FILE_HEADER), xfHeader.sParentTag);

        listResult.append(xfHeader);

        if (xfStruct.bIsParent) {
            qint32 nNumberOfSections = qMin((qint32)read_uint16(nFileHeaderOffset + offsetof(XPE_DEF::IMAGE_FILE_HEADER, NumberOfSections)),
                                             XPE_DEF::S_MAX_SECTIONCOUNT);
            quint16 nOptionalHeaderSize = read_uint16(nFileHeaderOffset + offsetof(XPE_DEF::IMAGE_FILE_HEADER, SizeOfOptionalHeader));
            qint64 nSectionsTableOffset = nFileHeaderOffset + sizeof(XPE_DEF::IMAGE_FILE_HEADER) + nOptionalHeaderSize;

            if ((nNumberOfSections > 0) && checkOffsetSize(nSectionsTableOffset, (qint64)nNumberOfSections * sizeof(XPE_DEF::IMAGE_SECTION_HEADER))) {
                XFSTRUCT sectionStruct = xfStruct;
                sectionStruct.sParent = xfHeader.sTag;
                sectionStruct.nStructID = STRUCTID_IMAGE_SECTION_HEADER;
                sectionStruct.xLoc = offsetToLoc(nSectionsTableOffset);
                sectionStruct.nSize = sizeof(XPE_DEF::IMAGE_SECTION_HEADER);
                sectionStruct.nCount = nNumberOfSections;
                sectionStruct.bIsParent = false;

                listResult.append(getXFHeaders(sectionStruct, pPdStruct));
            }
        }
    } else if ((nStructID == STRUCTID_IMAGE_OPTIONAL_HEADER32) || (nStructID == STRUCTID_IMAGE_OPTIONAL_HEADER64)) {
        bool bIs64 = (nStructID == STRUCTID_IMAGE_OPTIONAL_HEADER64);
        qint64 nOptionalHeaderOffset = locToOffset(xfStruct.pMemoryMap, xfStruct.xLoc);

        if (nOptionalHeaderOffset == -1) {
            nOptionalHeaderOffset = getOptionalHeaderOffset();
        }

        if (nOptionalHeaderOffset == -1) {
            return listResult;
        }

        XLOC ohLoc = offsetToLoc(nOptionalHeaderOffset);
        qint64 nOptionalHeaderFixedSize = bIs64 ? (qint64)sizeof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S) : (qint64)sizeof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S);
        qint64 nDeclaredOptionalHeaderSize = 0;
        qint64 nCanonicalOptionalHeaderOffset = getOptionalHeaderOffset();

        if (nOptionalHeaderOffset == nCanonicalOptionalHeaderOffset) {
            nDeclaredOptionalHeaderSize = getFileHeader_SizeOfOptionalHeader();
        } else if (xfStruct.nSize > 0) {
            nDeclaredOptionalHeaderSize = xfStruct.nSize;
        }

        if (nDeclaredOptionalHeaderSize <= 0) {
            nDeclaredOptionalHeaderSize = nOptionalHeaderFixedSize;
        }

        qint64 nHeaderSize = qMin(nDeclaredOptionalHeaderSize, nOptionalHeaderFixedSize);

        if (!checkOffsetSize(nOptionalHeaderOffset, nHeaderSize)) {
            return listResult;
        }

        XFHEADER xfHeader = {};
        xfHeader.sParentTag = xfStruct.sParent;
        xfHeader.fileType = xfStruct.fileType;
        xfHeader.structID = static_cast<XBinary::STRUCTID>(nStructID);
        xfHeader.xLoc = ohLoc;
        xfHeader.xfType = XFTYPE_HEADER;
        xfHeader.listFields = getXFRecords(xfStruct.fileType, nStructID, ohLoc);
        _filterXFRecordsBySize(&xfHeader.listFields, nHeaderSize);
        xfHeader.nSize = _getXFHeaderSize(nHeaderSize, xfHeader.listFields);
        _decorateXPEXFHeader(&xfHeader);
        xfHeader.sTag = xfHeaderToTag(xfHeader, structIDToString(nStructID), xfHeader.sParentTag);

        listResult.append(xfHeader);

        if (xfStruct.bIsParent) {
            // DATA_DIRECTORY table child
            qint64 nNumberFieldOffset = nOptionalHeaderOffset +
                                        (bIs64 ? (qint64)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S, NumberOfRvaAndSizes)
                                               : (qint64)offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S, NumberOfRvaAndSizes));
            quint32 nNumberOfRvaAndSizes = checkOffsetSize(nNumberFieldOffset, sizeof(quint32)) ? read_uint32(nNumberFieldOffset) : 0;
            qint64 nAvailableDirectoryBytes = qMax((qint64)0, qMin(nDeclaredOptionalHeaderSize, getSize() - nOptionalHeaderOffset) - nOptionalHeaderFixedSize);
            qint32 nDataDirCount = qMin((qint32)qMin((quint32)16, nNumberOfRvaAndSizes),
                                        (qint32)(nAvailableDirectoryBytes / sizeof(XPE_DEF::IMAGE_DATA_DIRECTORY)));

            if (nDataDirCount > 0) {
                qint64 nDataDirOffset = 0;

                if (bIs64) {
                    nDataDirOffset = nOptionalHeaderOffset + sizeof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S);
                } else {
                    nDataDirOffset = nOptionalHeaderOffset + sizeof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S);
                }

                XFHEADER xfDataDirTable = {};
                xfDataDirTable.sParentTag = xfHeader.sTag;
                xfDataDirTable.fileType = xfStruct.fileType;
                xfDataDirTable.structID = static_cast<XBinary::STRUCTID>(STRUCTID_IMAGE_DATA_DIRECTORY);
                xfDataDirTable.xLoc = offsetToLoc(nDataDirOffset);
                xfDataDirTable.xfType = XFTYPE_TABLE;
                xfDataDirTable.listFields = getXFRecords(xfStruct.fileType, STRUCTID_IMAGE_DATA_DIRECTORY, offsetToLoc(nDataDirOffset));
                xfDataDirTable.nSize = _getXFHeaderSize(0, xfDataDirTable.listFields);

                for (qint32 i = 0; (i < nDataDirCount) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
                    xfDataDirTable.listRowLocations.append(nDataDirOffset + i * xfDataDirTable.nSize);
                }

                if (xfDataDirTable.listRowLocations.isEmpty()) {
                    return listResult;
                }

                _decorateXPEDataDirectoryTable(this, &xfDataDirTable, xfStruct.pMemoryMap);
                xfDataDirTable.sTag = xfHeaderToTag(xfDataDirTable, structIDToString(STRUCTID_IMAGE_DATA_DIRECTORY), xfDataDirTable.sParentTag);

                listResult.append(xfDataDirTable);

                // Add children for each non-empty data directory entry
                qint32 nActualDataDirCount = xfDataDirTable.listRowLocations.count();
                for (qint32 i = 0; (i < nActualDataDirCount) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
                    XPE_DEF::IMAGE_DATA_DIRECTORY idd = read_IMAGE_DATA_DIRECTORY(nDataDirOffset + i * sizeof(XPE_DEF::IMAGE_DATA_DIRECTORY));

                    // Security dir holds WIN_CERT records at a raw file offset (often in the overlay), so handle it before the RVA check
                    if (i == XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_SECURITY) {
                        if ((idd.VirtualAddress != 0) && checkOffsetSize(idd.VirtualAddress, idd.Size)) {
                            _appendWinCertRecords(listResult, xfStruct, idd.VirtualAddress, idd.Size, xfDataDirTable.sTag, pPdStruct);
                        }
                        continue;
                    }

                    if (!isDataDirectoryValid(&idd, xfStruct.pMemoryMap)) {
                        continue;
                    }

                    // Security dir (index 4) uses raw file offset, not RVA
                    qint64 nChildOffset = -1;
                    if (i == XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_SECURITY) {
                        if (checkOffsetSize(idd.VirtualAddress, idd.Size)) {
                            nChildOffset = idd.VirtualAddress;
                        }
                    } else {
                        nChildOffset = relAddressToOffset(xfStruct.pMemoryMap, idd.VirtualAddress);
                    }

                    if (nChildOffset == -1) {
                        continue;
                    }

                    // Base relocations are variable-size blocks; walked by a dedicated helper
                    if (i == XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_BASERELOC) {
                        _appendBaseRelocationBlocks(listResult, xfStruct, nChildOffset, idd.Size, xfDataDirTable.sTag, pPdStruct);
                        continue;
                    }

                    if (i == XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT) {
                        _appendBoundImportRecords(listResult, xfStruct, nChildOffset, idd.Size, xfDataDirTable.sTag, pPdStruct);
                        continue;
                    }

                    quint32 nChildStructID = STRUCTID_UNKNOWN;
                    bool bChildIsTable = false;

                    switch (i) {
                        case XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXPORT: nChildStructID = STRUCTID_IMAGE_EXPORT_DIRECTORY; break;
                        case XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_RESOURCE: nChildStructID = STRUCTID_IMAGE_RESOURCE_DIRECTORY; break;
                        case XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_IMPORT:
                            nChildStructID = STRUCTID_IMAGE_IMPORT_DESCRIPTOR;
                            bChildIsTable = true;
                            break;
                        case XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXCEPTION:
                            nChildStructID = _getXPEExceptionStructID(getFileHeader_Machine());
                            bChildIsTable = (nChildStructID != STRUCTID_IMAGE_EXCEPTION_DIRECTORY_RAW);
                            break;
                        case XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_DEBUG:
                            nChildStructID = STRUCTID_IMAGE_DEBUG_DIRECTORY;
                            bChildIsTable = true;
                            break;
                        case XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_TLS: nChildStructID = bIs64 ? STRUCTID_IMAGE_TLS_DIRECTORY64 : STRUCTID_IMAGE_TLS_DIRECTORY32; break;
                        case XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG:
                            nChildStructID = bIs64 ? STRUCTID_IMAGE_LOAD_CONFIG_DIRECTORY64 : STRUCTID_IMAGE_LOAD_CONFIG_DIRECTORY32;
                            break;
                        case XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT:
                            nChildStructID = STRUCTID_IMAGE_DELAYLOAD_DESCRIPTOR;
                            bChildIsTable = true;
                            break;
                        case XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR: nChildStructID = STRUCTID_IMAGE_COR20_HEADER; break;
                        default: break;
                    }

                    if (nChildStructID == STRUCTID_UNKNOWN) {
                        continue;
                    }

                    XLOC childLoc = offsetToLoc(nChildOffset);
                    qint64 nChildAvailableSize = qMin((qint64)idd.Size, getSize() - nChildOffset);

                    if (nChildStructID == STRUCTID_IMAGE_COR20_HEADER) {
                        nChildAvailableSize = _getXPECor20HeaderSize(this, nChildOffset, nChildAvailableSize);
                    }

                    if (nChildAvailableSize <= 0) {
                        continue;
                    }

                    XFHEADER xfChild = {};
                    xfChild.sParentTag = xfDataDirTable.sTag;
                    xfChild.fileType = xfStruct.fileType;
                    xfChild.structID = static_cast<XBinary::STRUCTID>(nChildStructID);
                    xfChild.xLoc = childLoc;
                    xfChild.xfType = bChildIsTable ? XFTYPE_TABLE : XFTYPE_HEADER;
                    xfChild.listFields = getXFRecords(xfStruct.fileType, nChildStructID, childLoc);

                    if (bChildIsTable) {
                        xfChild.nSize = _getXFHeaderSize(0, xfChild.listFields);

                        bool bStopOnZero = (nChildStructID == STRUCTID_IMAGE_IMPORT_DESCRIPTOR) || (nChildStructID == STRUCTID_IMAGE_DELAYLOAD_DESCRIPTOR);
                        xfChild.listRowLocations = _getXPEFixedTableRows(this, nChildOffset, nChildAvailableSize, xfChild.nSize, bStopOnZero, pPdStruct);

                        if (xfChild.listRowLocations.isEmpty()) {
                            continue;
                        }
                    } else {
                        if (nChildStructID == STRUCTID_IMAGE_EXCEPTION_DIRECTORY_RAW) {
                            qint32 nRawSize = (qint32)qMin(nChildAvailableSize, (qint64)0x7FFFFFFF);
                            xfChild.listFields.append({"Data", 0, nRawSize, XFRECORD_FLAG_NONE, VT_BYTE_ARRAY});
                            xfChild.nSize = nRawSize;
                        } else {
                            _filterXFRecordsBySize(&xfChild.listFields, nChildAvailableSize);
                            xfChild.nSize = qMin(nChildAvailableSize, (qint64)_getXFHeaderSize(0, xfChild.listFields));
                        }

                        if ((xfChild.nSize <= 0) || xfChild.listFields.isEmpty()) {
                            continue;
                        }
                    }

                    _decorateXPEXFHeader(&xfChild);
                    if (nChildStructID == STRUCTID_IMAGE_DELAYLOAD_DESCRIPTOR) {
                        _decorateDelayImportTable(&xfChild, xfStruct, pPdStruct);
                    }
                    xfChild.sTag = xfHeaderToTag(xfChild, structIDToString(nChildStructID), xfChild.sParentTag);
                    listResult.append(xfChild);

                    if (nChildStructID == STRUCTID_IMAGE_EXPORT_DIRECTORY) {
                        _appendExportFunctionNames(listResult, xfStruct, nChildOffset, idd.Size, xfChild.sTag, pPdStruct);
                    } else if (nChildStructID == STRUCTID_IMAGE_IMPORT_DESCRIPTOR) {
                        _appendImportThunks(listResult, xfStruct, xfChild.sTag, pPdStruct);
                    } else if (nChildStructID == STRUCTID_IMAGE_RESOURCE_DIRECTORY) {
                        _appendResourceEntries(listResult, xfStruct, nChildOffset, idd.Size, xfChild.sTag, pPdStruct);
                    } else if (nChildStructID == STRUCTID_IMAGE_COR20_HEADER) {
                        _appendCor20Children(listResult, xfStruct, nChildOffset, nChildAvailableSize, xfChild.sTag, pPdStruct);
                    }
                }
            }
        }
    } else if (nStructID == STRUCTID_IMAGE_SECTION_HEADER) {
        qint64 nOffset = locToOffset(xfStruct.pMemoryMap, xfStruct.xLoc);

        if (nOffset == -1) {
            nOffset = getSectionsTableOffset();
        }

        qint32 nRows = xfStruct.nCount > 0 ? xfStruct.nCount : qMin((qint32)getFileHeader_NumberOfSections(), XPE_DEF::S_MAX_SECTIONCOUNT);
        qint64 nRowSize = xfStruct.nSize > 0 ? xfStruct.nSize : (qint64)sizeof(XPE_DEF::IMAGE_SECTION_HEADER);

        if ((nOffset != -1) && (nRows > 0) && (nRowSize > 0) && (nOffset < getSize())) {
            nRows = qMin(nRows, (qint32)qMin((qint64)XPE_DEF::S_MAX_SECTIONCOUNT, (getSize() - nOffset) / nRowSize));

            if (nRows <= 0) {
                return listResult;
            }

            XFHEADER xfSectionTable = {};
            xfSectionTable.sParentTag = xfStruct.sParent;
            xfSectionTable.fileType = xfStruct.fileType;
            xfSectionTable.structID = static_cast<XBinary::STRUCTID>(STRUCTID_IMAGE_SECTION_HEADER);
            xfSectionTable.xLoc = offsetToLoc(nOffset);
            xfSectionTable.xfType = XFTYPE_TABLE;
            xfSectionTable.listFields = getXFRecords(xfStruct.fileType, STRUCTID_IMAGE_SECTION_HEADER, xfSectionTable.xLoc);
            _filterXFRecordsBySize(&xfSectionTable.listFields, nRowSize);
            xfSectionTable.nSize = nRowSize;
            _decorateXPEXFHeader(&xfSectionTable);

            for (qint32 i = 0; (i < nRows) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
                qint64 nRowOffset = nOffset + (qint64)i * nRowSize;
                xfSectionTable.listRowLocations.append(nRowOffset);

                QByteArray baName = read_array(nRowOffset + offsetof(XPE_DEF::IMAGE_SECTION_HEADER, Name), sizeof(XPE_DEF::IMAGE_SECTION_HEADER::Name));
                qint32 nNameLength = baName.indexOf('\0');
                if (nNameLength == -1) {
                    nNameLength = baName.size();
                }
                QString sSectionName = QString::fromLatin1(baName.constData(), nNameLength).trimmed();
                xfSectionTable.listRowNames.append(sSectionName.isEmpty() ? QString("Section %1").arg(i + 1) : sSectionName);
            }
            xfSectionTable.sTag = xfHeaderToTag(xfSectionTable, structIDToString(STRUCTID_IMAGE_SECTION_HEADER), xfSectionTable.sParentTag);
            listResult.append(xfSectionTable);
        }
    } else if (nStructID == STRUCTID_IMAGE_DATA_DIRECTORY) {
        qint64 nOffset = locToOffset(xfStruct.pMemoryMap, xfStruct.xLoc);
        quint32 nNumberOfRvaAndSizes = getOptionalHeader_NumberOfRvaAndSizes();
        bool bIs64 = is64();
        qint64 nOptionalHeaderFixedSize = bIs64 ? (qint64)sizeof(XPE_DEF::IMAGE_OPTIONAL_HEADER64S) : (qint64)sizeof(XPE_DEF::IMAGE_OPTIONAL_HEADER32S);
        qint64 nOptionalHeaderOffset = getOptionalHeaderOffset();

        if ((nOffset == -1) && (nOptionalHeaderOffset != -1)) {
            nOffset = nOptionalHeaderOffset + nOptionalHeaderFixedSize;
        }

        qint32 nRows = xfStruct.nCount > 0 ? qMin(xfStruct.nCount, 16) : (qint32)qMin((quint32)16, nNumberOfRvaAndSizes);

        if ((nOffset != -1) && (nOptionalHeaderOffset != -1) && (nOffset == (nOptionalHeaderOffset + nOptionalHeaderFixedSize))) {
            qint64 nDeclaredDirectoryBytes = qMax((qint64)0, (qint64)getFileHeader_SizeOfOptionalHeader() - nOptionalHeaderFixedSize);
            nRows = qMin(nRows, (qint32)(nDeclaredDirectoryBytes / sizeof(XPE_DEF::IMAGE_DATA_DIRECTORY)));
        }

        if ((nOffset != -1) && (nOffset < getSize())) {
            nRows = qMin(nRows, (qint32)qMin((qint64)16, (getSize() - nOffset) / (qint64)sizeof(XPE_DEF::IMAGE_DATA_DIRECTORY)));
        }

        if ((nOffset != -1) && (nRows > 0)) {
            XFHEADER xfDataDirTable = {};
            xfDataDirTable.sParentTag = xfStruct.sParent;
            xfDataDirTable.fileType = xfStruct.fileType;
            xfDataDirTable.structID = static_cast<XBinary::STRUCTID>(STRUCTID_IMAGE_DATA_DIRECTORY);
            xfDataDirTable.xLoc = offsetToLoc(nOffset);
            xfDataDirTable.xfType = XFTYPE_TABLE;
            xfDataDirTable.listFields = getXFRecords(xfStruct.fileType, STRUCTID_IMAGE_DATA_DIRECTORY, xfDataDirTable.xLoc);
            xfDataDirTable.nSize = sizeof(XPE_DEF::IMAGE_DATA_DIRECTORY);
            _decorateXPEXFHeader(&xfDataDirTable);
            for (qint32 i = 0; (i < nRows) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
                xfDataDirTable.listRowLocations.append(nOffset + (qint64)i * xfDataDirTable.nSize);
            }
            if (xfDataDirTable.listRowLocations.isEmpty()) {
                return listResult;
            }
            _decorateXPEDataDirectoryTable(this, &xfDataDirTable, xfStruct.pMemoryMap);
            xfDataDirTable.sTag = xfHeaderToTag(xfDataDirTable, structIDToString(STRUCTID_IMAGE_DATA_DIRECTORY), xfDataDirTable.sParentTag);
            listResult.append(xfDataDirTable);

            if (xfStruct.bIsParent) {
                qint32 nActualRows = xfDataDirTable.listRowLocations.count();
                for (qint32 i = 0; (i < nActualRows) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
                    XPE_DEF::IMAGE_DATA_DIRECTORY idd = read_IMAGE_DATA_DIRECTORY(nOffset + i * sizeof(XPE_DEF::IMAGE_DATA_DIRECTORY));

                    // Security dir holds WIN_CERT records at a raw file offset (often in the overlay), so handle it before the RVA check
                    if (i == XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_SECURITY) {
                        if ((idd.VirtualAddress != 0) && checkOffsetSize(idd.VirtualAddress, idd.Size)) {
                            _appendWinCertRecords(listResult, xfStruct, idd.VirtualAddress, idd.Size, xfDataDirTable.sTag, pPdStruct);
                        }
                        continue;
                    }

                    if (!isDataDirectoryValid(&idd, xfStruct.pMemoryMap)) {
                        continue;
                    }

                    // Security dir (index 4) uses raw file offset, not RVA
                    qint64 nChildOffset = -1;
                    if (i == XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_SECURITY) {
                        if (checkOffsetSize(idd.VirtualAddress, idd.Size)) {
                            nChildOffset = idd.VirtualAddress;
                        }
                    } else {
                        nChildOffset = relAddressToOffset(xfStruct.pMemoryMap, idd.VirtualAddress);
                    }

                    if (nChildOffset == -1) {
                        continue;
                    }

                    // Base relocations are variable-size blocks; walked by a dedicated helper
                    if (i == XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_BASERELOC) {
                        _appendBaseRelocationBlocks(listResult, xfStruct, nChildOffset, idd.Size, xfDataDirTable.sTag, pPdStruct);
                        continue;
                    }

                    if (i == XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT) {
                        _appendBoundImportRecords(listResult, xfStruct, nChildOffset, idd.Size, xfDataDirTable.sTag, pPdStruct);
                        continue;
                    }

                    quint32 nChildStructID = STRUCTID_UNKNOWN;
                    bool bChildIsTable = false;

                    switch (i) {
                        case XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXPORT: nChildStructID = STRUCTID_IMAGE_EXPORT_DIRECTORY; break;
                        case XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_RESOURCE: nChildStructID = STRUCTID_IMAGE_RESOURCE_DIRECTORY; break;
                        case XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_IMPORT:
                            nChildStructID = STRUCTID_IMAGE_IMPORT_DESCRIPTOR;
                            bChildIsTable = true;
                            break;
                        case XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXCEPTION:
                            nChildStructID = _getXPEExceptionStructID(getFileHeader_Machine());
                            bChildIsTable = (nChildStructID != STRUCTID_IMAGE_EXCEPTION_DIRECTORY_RAW);
                            break;
                        case XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_DEBUG:
                            nChildStructID = STRUCTID_IMAGE_DEBUG_DIRECTORY;
                            bChildIsTable = true;
                            break;
                        case XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_TLS: nChildStructID = bIs64 ? STRUCTID_IMAGE_TLS_DIRECTORY64 : STRUCTID_IMAGE_TLS_DIRECTORY32; break;
                        case XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG:
                            nChildStructID = bIs64 ? STRUCTID_IMAGE_LOAD_CONFIG_DIRECTORY64 : STRUCTID_IMAGE_LOAD_CONFIG_DIRECTORY32;
                            break;
                        case XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT:
                            nChildStructID = STRUCTID_IMAGE_DELAYLOAD_DESCRIPTOR;
                            bChildIsTable = true;
                            break;
                        case XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR: nChildStructID = STRUCTID_IMAGE_COR20_HEADER; break;
                        default: break;
                    }

                    if (nChildStructID == STRUCTID_UNKNOWN) {
                        continue;
                    }

                    XLOC childLoc = offsetToLoc(nChildOffset);
                    qint64 nChildAvailableSize = qMin((qint64)idd.Size, getSize() - nChildOffset);

                    if (nChildStructID == STRUCTID_IMAGE_COR20_HEADER) {
                        nChildAvailableSize = _getXPECor20HeaderSize(this, nChildOffset, nChildAvailableSize);
                    }

                    if (nChildAvailableSize <= 0) {
                        continue;
                    }

                    XFHEADER xfChild = {};
                    xfChild.sParentTag = xfDataDirTable.sTag;
                    xfChild.fileType = xfStruct.fileType;
                    xfChild.structID = static_cast<XBinary::STRUCTID>(nChildStructID);
                    xfChild.xLoc = childLoc;
                    xfChild.xfType = bChildIsTable ? XFTYPE_TABLE : XFTYPE_HEADER;
                    xfChild.listFields = getXFRecords(xfStruct.fileType, nChildStructID, childLoc);

                    if (bChildIsTable) {
                        xfChild.nSize = _getXFHeaderSize(0, xfChild.listFields);

                        bool bStopOnZero = (nChildStructID == STRUCTID_IMAGE_IMPORT_DESCRIPTOR) || (nChildStructID == STRUCTID_IMAGE_DELAYLOAD_DESCRIPTOR);
                        xfChild.listRowLocations = _getXPEFixedTableRows(this, nChildOffset, nChildAvailableSize, xfChild.nSize, bStopOnZero, pPdStruct);

                        if (xfChild.listRowLocations.isEmpty()) {
                            continue;
                        }
                    } else {
                        if (nChildStructID == STRUCTID_IMAGE_EXCEPTION_DIRECTORY_RAW) {
                            qint32 nRawSize = (qint32)qMin(nChildAvailableSize, (qint64)0x7FFFFFFF);
                            xfChild.listFields.append({"Data", 0, nRawSize, XFRECORD_FLAG_NONE, VT_BYTE_ARRAY});
                            xfChild.nSize = nRawSize;
                        } else {
                            _filterXFRecordsBySize(&xfChild.listFields, nChildAvailableSize);
                            xfChild.nSize = qMin(nChildAvailableSize, (qint64)_getXFHeaderSize(0, xfChild.listFields));
                        }

                        if ((xfChild.nSize <= 0) || xfChild.listFields.isEmpty()) {
                            continue;
                        }
                    }

                    _decorateXPEXFHeader(&xfChild);
                    if (nChildStructID == STRUCTID_IMAGE_DELAYLOAD_DESCRIPTOR) {
                        _decorateDelayImportTable(&xfChild, xfStruct, pPdStruct);
                    }
                    xfChild.sTag = xfHeaderToTag(xfChild, structIDToString(nChildStructID), xfChild.sParentTag);
                    listResult.append(xfChild);

                    if (nChildStructID == STRUCTID_IMAGE_EXPORT_DIRECTORY) {
                        _appendExportFunctionNames(listResult, xfStruct, nChildOffset, idd.Size, xfChild.sTag, pPdStruct);
                    } else if (nChildStructID == STRUCTID_IMAGE_IMPORT_DESCRIPTOR) {
                        _appendImportThunks(listResult, xfStruct, xfChild.sTag, pPdStruct);
                    } else if (nChildStructID == STRUCTID_IMAGE_RESOURCE_DIRECTORY) {
                        _appendResourceEntries(listResult, xfStruct, nChildOffset, idd.Size, xfChild.sTag, pPdStruct);
                    } else if (nChildStructID == STRUCTID_IMAGE_COR20_HEADER) {
                        _appendCor20Children(listResult, xfStruct, nChildOffset, nChildAvailableSize, xfChild.sTag, pPdStruct);
                    }
                }
            }
        }
    } else if ((nStructID == STRUCTID_IMAGE_EXPORT_DIRECTORY) || (nStructID == STRUCTID_IMAGE_TLS_DIRECTORY32) || (nStructID == STRUCTID_IMAGE_TLS_DIRECTORY64) ||
               (nStructID == STRUCTID_IMAGE_LOAD_CONFIG_DIRECTORY32) || (nStructID == STRUCTID_IMAGE_LOAD_CONFIG_DIRECTORY64) ||
               (nStructID == STRUCTID_IMAGE_COR20_HEADER) || (nStructID == STRUCTID_IMAGE_RESOURCE_DIRECTORY) ||
               (nStructID == STRUCTID_IMAGE_RESOURCE_DATA_ENTRY)) {
        qint64 nOffset = locToOffset(xfStruct.pMemoryMap, xfStruct.xLoc);
        qint32 nDirectoryIndex = -1;

        if (nStructID == STRUCTID_IMAGE_EXPORT_DIRECTORY) {
            nDirectoryIndex = XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXPORT;
        } else if ((nStructID == STRUCTID_IMAGE_TLS_DIRECTORY32) || (nStructID == STRUCTID_IMAGE_TLS_DIRECTORY64)) {
            nDirectoryIndex = XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_TLS;
        } else if ((nStructID == STRUCTID_IMAGE_LOAD_CONFIG_DIRECTORY32) || (nStructID == STRUCTID_IMAGE_LOAD_CONFIG_DIRECTORY64)) {
            nDirectoryIndex = XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG;
        } else if (nStructID == STRUCTID_IMAGE_COR20_HEADER) {
            nDirectoryIndex = XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR;
        } else if (nStructID == STRUCTID_IMAGE_RESOURCE_DIRECTORY) {
            nDirectoryIndex = XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_RESOURCE;
        }

        XPE_DEF::IMAGE_DATA_DIRECTORY dataDirectory = {};
        qint64 nCanonicalOffset = -1;

        if (nDirectoryIndex != -1) {
            dataDirectory = getOptionalHeader_DataDirectory(nDirectoryIndex);
            nCanonicalOffset = getDataDirectoryOffset(nDirectoryIndex);
        }

        if (nOffset == -1) {
            nOffset = nCanonicalOffset;
        }

        if ((nOffset >= 0) && (nOffset < getSize())) {
            qint64 nAvailableSize = getSize() - nOffset;

            if (xfStruct.nSize > 0) {
                nAvailableSize = qMin(nAvailableSize, xfStruct.nSize);
            }

            if ((nOffset == nCanonicalOffset) && (dataDirectory.Size > 0)) {
                nAvailableSize = qMin(nAvailableSize, (qint64)dataDirectory.Size);
            }

            if (nStructID == STRUCTID_IMAGE_COR20_HEADER) {
                nAvailableSize = _getXPECor20HeaderSize(this, nOffset, nAvailableSize);
            }

            XLOC headerLoc = offsetToLoc(nOffset);
            XFHEADER xfHeader = {};
            xfHeader.sParentTag = xfStruct.sParent;
            xfHeader.fileType = xfStruct.fileType;
            xfHeader.structID = static_cast<XBinary::STRUCTID>(nStructID);
            xfHeader.xLoc = headerLoc;
            xfHeader.xfType = XFTYPE_HEADER;
            xfHeader.listFields = getXFRecords(xfStruct.fileType, nStructID, headerLoc);
            _filterXFRecordsBySize(&xfHeader.listFields, nAvailableSize);
            xfHeader.nSize = qMin(nAvailableSize, (qint64)_getXFRecordsSize(xfHeader.listFields));

            if ((xfHeader.nSize <= 0) || xfHeader.listFields.isEmpty()) {
                return listResult;
            }

            _decorateXPEXFHeader(&xfHeader);
            xfHeader.sTag = xfHeaderToTag(xfHeader, structIDToString(nStructID), xfHeader.sParentTag);
            listResult.append(xfHeader);

            if (xfStruct.bIsParent && (nStructID == STRUCTID_IMAGE_EXPORT_DIRECTORY)) {
                _appendExportFunctionNames(listResult, xfStruct, nOffset, nAvailableSize, xfHeader.sTag, pPdStruct);
            } else if (xfStruct.bIsParent && (nStructID == STRUCTID_IMAGE_RESOURCE_DIRECTORY)) {
                qint64 nResourceSize = 0;
                if ((nOffset == nCanonicalOffset) && (dataDirectory.Size > 0)) {
                    nResourceSize = qMin((qint64)dataDirectory.Size, getSize() - nOffset);
                } else if (xfStruct.nSize > (qint64)sizeof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY)) {
                    nResourceSize = qMin(xfStruct.nSize, getSize() - nOffset);
                }
                if (nResourceSize > (qint64)sizeof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY)) {
                    _appendResourceEntries(listResult, xfStruct, nOffset, nResourceSize, xfHeader.sTag, pPdStruct);
                }
            } else if (xfStruct.bIsParent && (nStructID == STRUCTID_IMAGE_COR20_HEADER)) {
                _appendCor20Children(listResult, xfStruct, nOffset, nAvailableSize, xfHeader.sTag, pPdStruct);
            }
        }
    } else if ((nStructID == STRUCTID_IMAGE_EXPORT_FUNCTION) || (nStructID == STRUCTID_IMAGE_IMPORT_DESCRIPTOR) || (nStructID == STRUCTID_IMAGE_THUNK_DATA32) ||
               (nStructID == STRUCTID_IMAGE_THUNK_DATA64) || (nStructID == STRUCTID_IMAGE_DEBUG_DIRECTORY) ||
               (nStructID == STRUCTID_IMAGE_RUNTIME_FUNCTION_ENTRY) || (nStructID == STRUCTID_IMAGE_RUNTIME_FUNCTION_ENTRY_ARM) ||
               (nStructID == STRUCTID_IMAGE_RUNTIME_FUNCTION_ENTRY_ALPHA) || (nStructID == STRUCTID_IMAGE_RUNTIME_FUNCTION_ENTRY_ALPHA64) ||
               (nStructID == STRUCTID_IMAGE_BOUND_FORWARDER_REF) ||
               (nStructID == STRUCTID_IMAGE_DELAYLOAD_DESCRIPTOR) || (nStructID == STRUCTID_IMAGE_RESOURCE_DIRECTORY_ENTRY) ||
               (nStructID == STRUCTID_IMAGE_BASE_RELOCATION_ENTRY)) {
        qint64 nOffset = locToOffset(xfStruct.pMemoryMap, xfStruct.xLoc);
        bool bExplicitLocation = (nOffset != -1);
        qint32 nRows = xfStruct.nCount > 0 ? qMin(xfStruct.nCount, 0x10000) : 0;
        qint32 nDirectoryIndex = -1;
        qint64 nOwnerStartOffset = -1;
        qint64 nOwnerEndOffset = -1;

        if (nStructID == STRUCTID_IMAGE_IMPORT_DESCRIPTOR) {
            nDirectoryIndex = XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_IMPORT;
        } else if ((nStructID == STRUCTID_IMAGE_RUNTIME_FUNCTION_ENTRY) || (nStructID == STRUCTID_IMAGE_RUNTIME_FUNCTION_ENTRY_ARM) ||
                   (nStructID == STRUCTID_IMAGE_RUNTIME_FUNCTION_ENTRY_ALPHA) || (nStructID == STRUCTID_IMAGE_RUNTIME_FUNCTION_ENTRY_ALPHA64)) {
            nDirectoryIndex = XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXCEPTION;
        } else if (nStructID == STRUCTID_IMAGE_DEBUG_DIRECTORY) {
            nDirectoryIndex = XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_DEBUG;
        } else if (nStructID == STRUCTID_IMAGE_DELAYLOAD_DESCRIPTOR) {
            nDirectoryIndex = XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT;
        }

        XPE_DEF::IMAGE_DATA_DIRECTORY dataDirectory = {};
        qint64 nCanonicalOffset = -1;

        if (nDirectoryIndex != -1) {
            dataDirectory = getOptionalHeader_DataDirectory(nDirectoryIndex);
            nCanonicalOffset = getDataDirectoryOffset(nDirectoryIndex);
        }

        if (nStructID == STRUCTID_IMAGE_EXPORT_FUNCTION) {
            qint64 nExportDirectoryOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXPORT);
            XPE_DEF::IMAGE_DATA_DIRECTORY exportDataDirectory = getOptionalHeader_DataDirectory(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXPORT);

            if (isDataDirectoryValid(&exportDataDirectory, xfStruct.pMemoryMap)) {
                nOwnerStartOffset = nExportDirectoryOffset;
                nOwnerEndOffset = nOwnerStartOffset + exportDataDirectory.Size;
            }

            if (checkOffsetSize(nExportDirectoryOffset, sizeof(XPE_DEF::IMAGE_EXPORT_DIRECTORY))) {
                XPE_DEF::IMAGE_EXPORT_DIRECTORY exportDirectory = read_IMAGE_EXPORT_DIRECTORY(nExportDirectoryOffset);
                qint64 nFunctionsOffset = relAddressToOffset(xfStruct.pMemoryMap, exportDirectory.AddressOfFunctions);

                if ((nFunctionsOffset >= nOwnerStartOffset) && (nFunctionsOffset < nOwnerEndOffset)) {
                    nCanonicalOffset = nFunctionsOffset;
                    if (nRows <= 0) {
                        nRows = (qint32)qMin((quint64)exportDirectory.NumberOfFunctions, (quint64)0x10000);
                    }
                }
            }
        } else if (nStructID == STRUCTID_IMAGE_RESOURCE_DIRECTORY_ENTRY) {
            qint64 nResourceDirectoryOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_RESOURCE);
            XPE_DEF::IMAGE_DATA_DIRECTORY resourceDataDirectory = getOptionalHeader_DataDirectory(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_RESOURCE);

            if (isDataDirectoryValid(&resourceDataDirectory, xfStruct.pMemoryMap)) {
                nOwnerStartOffset = nResourceDirectoryOffset;
                nOwnerEndOffset = nOwnerStartOffset + resourceDataDirectory.Size;
            }

            if ((nOwnerStartOffset >= 0) && checkOffsetSize(nResourceDirectoryOffset, sizeof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY))) {
                XPE_DEF::IMAGE_RESOURCE_DIRECTORY resourceDirectory = read_IMAGE_RESOURCE_DIRECTORY(nResourceDirectoryOffset);
                qint32 nRootEntries = (qint32)resourceDirectory.NumberOfNamedEntries + (qint32)resourceDirectory.NumberOfIdEntries;
                qint64 nEntriesOffset = nResourceDirectoryOffset + sizeof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY);

                if ((nRootEntries > 0) && (nRootEntries <= 0x2000) &&
                    ((qint64)nRootEntries * (qint64)sizeof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY_ENTRY) <= (nOwnerEndOffset - nEntriesOffset))) {
                    nCanonicalOffset = nEntriesOffset;
                    if (nRows <= 0) {
                        nRows = nRootEntries;
                    }
                }
            }
        }

        if (nOffset == -1) {
            nOffset = nCanonicalOffset;
        }

        if (!bExplicitLocation && (nCanonicalOffset == -1)) {
            return listResult;
        }

        if ((nRows <= 0) && (nOffset >= 0) && ((nDirectoryIndex == -1) || (dataDirectory.Size == 0))) {
            nRows = 1;
        }

        if ((nOffset >= 0) && (nOffset < getSize())) {
            XLOC tableLoc = offsetToLoc(nOffset);
            XFHEADER xfTable = {};
            xfTable.sParentTag = xfStruct.sParent;
            xfTable.fileType = xfStruct.fileType;
            xfTable.structID = static_cast<XBinary::STRUCTID>(nStructID);
            xfTable.xLoc = tableLoc;
            xfTable.xfType = XFTYPE_TABLE;
            xfTable.listFields = getXFRecords(xfStruct.fileType, nStructID, tableLoc);
            qint64 nRequestedRowSize = (xfStruct.nSize > 0) ? xfStruct.nSize : (qint64)_getXFRecordsSize(xfTable.listFields);

            if ((nRequestedRowSize <= 0) || (nRequestedRowSize > 0x7FFFFFFF)) {
                return listResult;
            }

            _filterXFRecordsBySize(&xfTable.listFields, nRequestedRowSize);
            qint32 nStructSize = (qint32)nRequestedRowSize;

            if (xfTable.listFields.isEmpty()) {
                return listResult;
            }

            qint64 nAvailableSize = getSize() - nOffset;

            if ((nOwnerStartOffset >= 0) && (nOffset >= nOwnerStartOffset) && (nOffset < nOwnerEndOffset)) {
                nAvailableSize = qMin(nAvailableSize, nOwnerEndOffset - nOffset);
            } else if (!bExplicitLocation && (nOwnerStartOffset >= 0)) {
                return listResult;
            }

            if ((nOffset == nCanonicalOffset) && (dataDirectory.Size > 0)) {
                nAvailableSize = qMin(nAvailableSize, (qint64)dataDirectory.Size);
            }

            if (nRows > 0) {
                nAvailableSize = qMin(nAvailableSize, (qint64)nRows * nStructSize);
            }

            bool bStopOnZero = (nStructID == STRUCTID_IMAGE_IMPORT_DESCRIPTOR) || (nStructID == STRUCTID_IMAGE_DELAYLOAD_DESCRIPTOR) ||
                               (nStructID == STRUCTID_IMAGE_THUNK_DATA32) || (nStructID == STRUCTID_IMAGE_THUNK_DATA64);
            xfTable.listRowLocations = _getXPEFixedTableRows(this, nOffset, nAvailableSize, nStructSize, bStopOnZero, pPdStruct);

            if ((nRows > 0) && (xfTable.listRowLocations.count() > nRows)) {
                xfTable.listRowLocations = xfTable.listRowLocations.mid(0, nRows);
            }

            if (xfTable.listRowLocations.isEmpty()) {
                return listResult;
            }

            xfTable.nSize = nStructSize;
            _decorateXPEXFHeader(&xfTable);

            if ((nStructID == STRUCTID_IMAGE_THUNK_DATA32) || (nStructID == STRUCTID_IMAGE_THUNK_DATA64)) {
                _decorateImportThunkTable(&xfTable, xfStruct, pPdStruct);
            } else if (nStructID == STRUCTID_IMAGE_DELAYLOAD_DESCRIPTOR) {
                _decorateDelayImportTable(&xfTable, xfStruct, pPdStruct);
            }
            xfTable.sTag = xfHeaderToTag(xfTable, structIDToString(nStructID), xfTable.sParentTag);
            listResult.append(xfTable);
        }
    } else if (nStructID == STRUCTID_IMAGE_BOUND_IMPORT_DESCRIPTOR) {
        qint64 nOffset = locToOffset(xfStruct.pMemoryMap, xfStruct.xLoc);
        bool bExplicitLocation = (nOffset != -1);
        XPE_DEF::IMAGE_DATA_DIRECTORY idd = getOptionalHeader_DataDirectory(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT);

        if (nOffset == -1) {
            nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT);
        }

        qint64 nBoundImportSize = idd.Size;

        if (bExplicitLocation && checkOffsetSize(nOffset, sizeof(XPE_DEF::IMAGE_BOUND_IMPORT_DESCRIPTOR))) {
            quint16 nForwarderRefs = read_uint16(nOffset + offsetof(XPE_DEF::IMAGE_BOUND_IMPORT_DESCRIPTOR, NumberOfModuleForwarderRefs));
            nBoundImportSize = (qint64)sizeof(XPE_DEF::IMAGE_BOUND_IMPORT_DESCRIPTOR) +
                               (qint64)nForwarderRefs * sizeof(XPE_DEF::IMAGE_BOUND_FORWARDER_REF);
        } else if ((nBoundImportSize <= 0) && (xfStruct.nSize > 0)) {
            nBoundImportSize = xfStruct.nSize;
        }

        _appendBoundImportRecords(listResult, xfStruct, nOffset, nBoundImportSize, xfStruct.sParent, pPdStruct);
    } else if (nStructID == STRUCTID_IMAGE_BASE_RELOCATION) {
        qint64 nOffset = locToOffset(xfStruct.pMemoryMap, xfStruct.xLoc);
        bool bExplicitLocation = (nOffset != -1);

        XPE_DEF::IMAGE_DATA_DIRECTORY idd = getOptionalHeader_DataDirectory(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_BASERELOC);

        if (nOffset == -1) {
            nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_BASERELOC);
        }

        qint64 nRelocationSize = idd.Size;

        if (bExplicitLocation && checkOffsetSize(nOffset, sizeof(XPE_DEF::IMAGE_BASE_RELOCATION))) {
            nRelocationSize = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_BASE_RELOCATION, SizeOfBlock));
        } else if ((nRelocationSize <= 0) && (xfStruct.nSize > 0)) {
            nRelocationSize = xfStruct.nSize;
        }

        _appendBaseRelocationBlocks(listResult, xfStruct, nOffset, nRelocationSize, xfStruct.sParent, pPdStruct);
    } else if (nStructID == STRUCTID_WIN_CERT_RECORD) {
        qint64 nOffset = locToOffset(xfStruct.pMemoryMap, xfStruct.xLoc);
        bool bExplicitLocation = (nOffset != -1);

        XPE_DEF::IMAGE_DATA_DIRECTORY idd = getOptionalHeader_DataDirectory(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_SECURITY);

        if (nOffset == -1) {
            nOffset = idd.VirtualAddress;  // Security dir stores a raw file offset
        }

        qint64 nCertificateSize = idd.Size;

        if (bExplicitLocation && checkOffsetSize(nOffset, sizeof(XPE_DEF::WIN_CERT_RECORD))) {
            quint32 nLength = read_uint32(nOffset + offsetof(XPE_DEF::WIN_CERT_RECORD, dwLength));
            nCertificateSize = ((qint64)nLength + 7) & ~(qint64)7;
        } else if ((nCertificateSize <= 0) && (xfStruct.nSize > 0)) {
            nCertificateSize = xfStruct.nSize;
        }

        _appendWinCertRecords(listResult, xfStruct, nOffset, nCertificateSize, xfStruct.sParent, pPdStruct);
    } else if ((nStructID == STRUCTID_NET_RESOURCES) || (nStructID == STRUCTID_NET_STRONGNAMESIGNATURE) ||
               (nStructID == STRUCTID_NET_CODEMANAGERTABLE) || (nStructID == STRUCTID_NET_VTABLEFIXUPS) ||
               (nStructID == STRUCTID_NET_EXPORTADDRESSTABLEJUMPS) || (nStructID == STRUCTID_NET_MANAGEDNATIVEHEADER) ||
               (nStructID == STRUCTID_IMAGE_EXCEPTION_DIRECTORY_RAW)) {
        qint64 nOffset = locToOffset(xfStruct.pMemoryMap, xfStruct.xLoc);
        qint64 nDataSize = xfStruct.nSize;

        if (nOffset == -1) {
            if (nStructID == STRUCTID_IMAGE_EXCEPTION_DIRECTORY_RAW) {
                XPE_DEF::IMAGE_DATA_DIRECTORY exceptionDirectory = getOptionalHeader_DataDirectory(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXCEPTION);

                if (isDataDirectoryValid(&exceptionDirectory, xfStruct.pMemoryMap)) {
                    nOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXCEPTION);
                    nDataSize = exceptionDirectory.Size;
                }
            }

            qint32 nDirectoryFieldOffset = -1;

            if (nStructID == STRUCTID_NET_RESOURCES) {
                nDirectoryFieldOffset = offsetof(XPE_DEF::IMAGE_COR20_HEADER, Resources);
            } else if (nStructID == STRUCTID_NET_STRONGNAMESIGNATURE) {
                nDirectoryFieldOffset = offsetof(XPE_DEF::IMAGE_COR20_HEADER, StrongNameSignature);
            } else if (nStructID == STRUCTID_NET_CODEMANAGERTABLE) {
                nDirectoryFieldOffset = offsetof(XPE_DEF::IMAGE_COR20_HEADER, CodeManagerTable);
            } else if (nStructID == STRUCTID_NET_VTABLEFIXUPS) {
                nDirectoryFieldOffset = offsetof(XPE_DEF::IMAGE_COR20_HEADER, VTableFixups);
            } else if (nStructID == STRUCTID_NET_EXPORTADDRESSTABLEJUMPS) {
                nDirectoryFieldOffset = offsetof(XPE_DEF::IMAGE_COR20_HEADER, ExportAddressTableJumps);
            } else if (nStructID == STRUCTID_NET_MANAGEDNATIVEHEADER) {
                nDirectoryFieldOffset = offsetof(XPE_DEF::IMAGE_COR20_HEADER, ManagedNativeHeader);
            }

            qint64 nCor20Offset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);
            XPE_DEF::IMAGE_DATA_DIRECTORY cor20Directory = getOptionalHeader_DataDirectory(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);
            qint64 nCor20Size = isDataDirectoryValid(&cor20Directory, xfStruct.pMemoryMap)
                                    ? _getXPECor20HeaderSize(this, nCor20Offset, cor20Directory.Size)
                                    : 0;

            if ((nOffset == -1) && (nCor20Offset >= 0) && (nDirectoryFieldOffset >= 0) &&
                ((qint64)nDirectoryFieldOffset + (qint64)sizeof(XPE_DEF::IMAGE_DATA_DIRECTORY) <= nCor20Size)) {
                XPE_DEF::IMAGE_DATA_DIRECTORY nestedDirectory = {};
                nestedDirectory.VirtualAddress = read_uint32(nCor20Offset + nDirectoryFieldOffset + offsetof(XPE_DEF::IMAGE_DATA_DIRECTORY, VirtualAddress));
                nestedDirectory.Size = read_uint32(nCor20Offset + nDirectoryFieldOffset + offsetof(XPE_DEF::IMAGE_DATA_DIRECTORY, Size));

                if (isDataDirectoryValid(&nestedDirectory, xfStruct.pMemoryMap)) {
                    nDataSize = nestedDirectory.Size;
                    nOffset = relAddressToOffset(xfStruct.pMemoryMap, nestedDirectory.VirtualAddress);
                }
            }
        }

        if ((nOffset >= 0) && (nOffset < getSize()) && (nDataSize > 0)) {
            qint32 nRecordSize = (qint32)qMin(qMin(nDataSize, getSize() - nOffset), (qint64)0x7FFFFFFF);

            if (nRecordSize > 0) {
                XFHEADER xfData = {};
                xfData.sParentTag = xfStruct.sParent;
                xfData.fileType = xfStruct.fileType;
                xfData.structID = static_cast<XBinary::STRUCTID>(nStructID);
                xfData.xLoc = offsetToLoc(nOffset);
                xfData.nSize = nRecordSize;
                xfData.xfType = XFTYPE_HEADER;
                xfData.listFields.append({"Data", 0, nRecordSize, XFRECORD_FLAG_NONE, VT_BYTE_ARRAY});
                xfData.sTag = xfHeaderToTag(xfData, structIDToString(nStructID), xfData.sParentTag);
                listResult.append(xfData);
            }
        }
    } else if (nStructID == STRUCTID_NET_METADATA) {
        qint64 nOffset = locToOffset(xfStruct.pMemoryMap, xfStruct.xLoc);
        _appendNetMetadata(listResult, xfStruct, xfStruct.sParent, pPdStruct, nOffset, xfStruct.nSize);
    } else if (nStructID == STRUCTID_NET_METADATA_STREAM) {
        qint64 nOffset = locToOffset(xfStruct.pMemoryMap, xfStruct.xLoc);
        qint64 nEndOffset = getSize();

        if (nOffset == -1) {
            OFFSETSIZE osMetadata = getNet_MetadataOffsetSize();

            if ((osMetadata.nOffset != -1) && (osMetadata.nSize >= 20) && checkOffsetSize(osMetadata.nOffset, 20) &&
                (read_uint32(osMetadata.nOffset) == 0x424A5342)) {
                quint32 nVersionStringLength = read_uint32(osMetadata.nOffset + 12);

                if ((nVersionStringLength <= 0x10000) && ((qint64)nVersionStringLength + 20 <= osMetadata.nSize)) {
                    nOffset = osMetadata.nOffset + 20 + nVersionStringLength;
                    nEndOffset = qMin(osMetadata.nOffset + osMetadata.nSize, getSize());
                }
            }
        } else if (xfStruct.nSize > 0) {
            nEndOffset = qMin(nOffset + xfStruct.nSize, getSize());
        }

        qint32 nRows = xfStruct.nCount > 0 ? qMin(xfStruct.nCount, 0x100) : 1;

        for (qint32 i = 0; (i < nRows) && (nOffset >= 0) && ((nOffset + 8) <= nEndOffset) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
            qint64 nNameBytes = qMin((qint64)32, nEndOffset - nOffset - 8);
            QByteArray baName = read_array(nOffset + 8, nNameBytes);
            qint32 nTerminator = baName.indexOf('\0');

            if (nTerminator == -1) {
                break;
            }

            qint32 nNameStorageSize = (nTerminator + 1 + 3) & ~3;
            qint64 nHeaderSize = 8 + nNameStorageSize;

            if ((nOffset + nHeaderSize) > nEndOffset) {
                break;
            }

            XFHEADER xfHeader = {};
            xfHeader.sParentTag = xfStruct.sParent;
            xfHeader.fileType = xfStruct.fileType;
            xfHeader.structID = static_cast<XBinary::STRUCTID>(STRUCTID_NET_METADATA_STREAM);
            xfHeader.xLoc = offsetToLoc(nOffset);
            xfHeader.nSize = nHeaderSize;
            xfHeader.xfType = XFTYPE_HEADER;
            xfHeader.listFields = getXFRecords(xfStruct.fileType, STRUCTID_NET_METADATA_STREAM, xfHeader.xLoc);
            _filterXFRecordsBySize(&xfHeader.listFields, nHeaderSize);
            xfHeader.sTag = xfHeaderToTag(xfHeader, structIDToString(STRUCTID_NET_METADATA_STREAM), xfHeader.sParentTag);
            listResult.append(xfHeader);

            nOffset += nHeaderSize;
        }
    } else {
        listResult = XMSDOS::getXFHeaders(xfStruct, pPdStruct);
    }

    return listResult;
}

XADDR XPE::_getEntryPointAddress()
{
    return getModuleAddress() + getOptionalHeader_AddressOfEntryPoint();
}

quint32 XPE::hlTypeToFParts(HLTYPE hlType)
{
    quint32 nResult = 0;

    if (hlType == HLTYPE_NATIVEREGIONS) {
        nResult = FILEPART_HEADER + FILEPART_SECTION + FILEPART_OVERLAY;
    } else if (hlType == HLTYPE_NATIVESUBREGIONS) {
        // TODO
    } else if (hlType == HLTYPE_DATA) {
        // nResult = FILEPART_RESOURCE;
    }

    return nResult;
}

// QList<XBinary::DATA_HEADER> XPE::getDataHeaders(const DATA_HEADERS_OPTIONS &dataHeadersOptions, PDSTRUCT *pPdStruct)
// {
//     QList<XBinary::DATA_HEADER> listResult;

//     if (dataHeadersOptions.nID == STRUCTID_UNKNOWN) {
//         DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
//         _dataHeadersOptions.bChildren = true;
//         _dataHeadersOptions.dsID_parent = _addDefaultHeaders(&listResult, pPdStruct);
//         _dataHeadersOptions.dhMode = XBinary::DHMODE_HEADER;
//         _dataHeadersOptions.fileType = dataHeadersOptions.pMemoryMap->fileType;
//         _dataHeadersOptions.nID = STRUCTID_IMAGE_DOS_HEADER;

//         listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//     } else {
//         qint64 nStartOffset = locationToOffset(dataHeadersOptions.pMemoryMap, dataHeadersOptions.locType, dataHeadersOptions.nLocation);

//         if (nStartOffset != -1) {
//             if (dataHeadersOptions.nID == XPE::STRUCTID_IMAGE_DOS_HEADER) {
//                 {
//                     DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
//                     _dataHeadersOptions.bChildren = false;
//                     _dataHeadersOptions.dhMode = XBinary::DHMODE_HEADER;
//                     _dataHeadersOptions.fileType = FT_MSDOS;
//                     _dataHeadersOptions.nID = XMSDOS::STRUCTID_IMAGE_DOS_HEADEREX;

//                     listResult.append(XMSDOS::getDataHeaders(_dataHeadersOptions, pPdStruct));
//                 }

//                 if (dataHeadersOptions.bChildren) {
//                     {
//                         // DOS Stub
//                     }
//                     {
//                         // Rich
//                     }

//                     {
//                         quint32 nLNew = read_uint32(nStartOffset + offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_lfanew));

//                         DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
//                         _dataHeadersOptions.nLocation += nLNew;
//                         _dataHeadersOptions.dhMode = XBinary::DHMODE_HEADER;

//                         if (dataHeadersOptions.fileType == FT_PE32) {
//                             _dataHeadersOptions.nID = STRUCTID_IMAGE_NT_HEADERS32;
//                         } else {
//                             _dataHeadersOptions.nID = STRUCTID_IMAGE_NT_HEADERS64;
//                         }

//                         listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//                     }
//                 }
//             } else if ((dataHeadersOptions.nID == STRUCTID_IMAGE_NT_HEADERS32) || (dataHeadersOptions.nID == STRUCTID_IMAGE_NT_HEADERS64)) {
//                 XBinary::DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, structIDToString(dataHeadersOptions.nID));

//                 if (dataHeadersOptions.nID == STRUCTID_IMAGE_NT_HEADERS32) {
//                     dataHeader.nSize = sizeof(XPE_DEF::IMAGE_NT_HEADERS32);
//                 } else if (dataHeadersOptions.nID == STRUCTID_IMAGE_NT_HEADERS64) {
//                     dataHeader.nSize = sizeof(XPE_DEF::IMAGE_NT_HEADERS64);
//                 }

//                 dataHeader.listRecords.append(getDataRecordDV(offsetof(XPE_DEF::IMAGE_NT_HEADERS32, Signature), 4, "Signature", VT_DWORD, DRF_UNKNOWN,
//                                                               dataHeadersOptions.pMemoryMap->endian, XPE::getImageNtHeadersSignaturesS(), VL_TYPE_LIST));

//                 listResult.append(dataHeader);

//                 if (dataHeadersOptions.bChildren) {
//                     {
//                         DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
//                         _dataHeadersOptions.nLocation += 4;
//                         _dataHeadersOptions.dsID_parent = dataHeader.dsID;
//                         _dataHeadersOptions.dhMode = XBinary::DHMODE_HEADER;
//                         _dataHeadersOptions.nID = STRUCTID_IMAGE_FILE_HEADER;
//                         listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//                     }
//                     {
//                         DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
//                         _dataHeadersOptions.nLocation += 4 + sizeof(XPE_DEF::IMAGE_FILE_HEADER);
//                         _dataHeadersOptions.dsID_parent = dataHeader.dsID;
//                         _dataHeadersOptions.dhMode = XBinary::DHMODE_HEADER;

//                         if (dataHeadersOptions.nID == STRUCTID_IMAGE_NT_HEADERS32) {
//                             _dataHeadersOptions.nID = STRUCTID_IMAGE_OPTIONAL_HEADER32;
//                         } else if (dataHeadersOptions.nID == STRUCTID_IMAGE_NT_HEADERS64) {
//                             _dataHeadersOptions.nID = STRUCTID_IMAGE_OPTIONAL_HEADER64;
//                         }

//                         listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//                     }
//                 }
//             } else if (dataHeadersOptions.nID == STRUCTID_IMAGE_FILE_HEADER) {
//                 XBinary::DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, structIDToString(dataHeadersOptions.nID));
//                 dataHeader.nSize = sizeof(XPE_DEF::IMAGE_FILE_HEADER);

//                 dataHeader.listRecords.append(getDataRecordDV(offsetof(XPE_DEF::IMAGE_FILE_HEADER, Machine), 2, "Machine", VT_WORD, DRF_UNKNOWN,
//                                                               dataHeadersOptions.pMemoryMap->endian, XPE::getImageFileHeaderMachinesS(), VL_TYPE_LIST));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_FILE_HEADER, NumberOfSections), 2, "NumberOfSections", VT_WORD, DRF_COUNT,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XPE_DEF::IMAGE_FILE_HEADER, TimeDateStamp), 4, "TimeDateStamp", VT_DWORD, DRF_UNKNOWN,
//                     dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_FILE_HEADER, PointerToSymbolTable), 4, "PointerToSymbolTable", VT_DWORD,
//                 DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_FILE_HEADER, NumberOfSymbols), 4, "NumberOfSymbols", VT_DWORD, DRF_COUNT,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_FILE_HEADER, SizeOfOptionalHeader), 2, "SizeOfOptionalHeader", VT_WORD, DRF_SIZE,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecordDV(offsetof(XPE_DEF::IMAGE_FILE_HEADER, Characteristics), 2, "Characteristics", VT_WORD, DRF_UNKNOWN,
//                                                               dataHeadersOptions.pMemoryMap->endian, XPE::getImageFileHeaderCharacteristicsS(), VL_TYPE_FLAGS));
//                 listResult.append(dataHeader);

//                 if (dataHeadersOptions.bChildren) {
//                     quint32 SizeOfOptionalHeader = read_uint16(nStartOffset + offsetof(XPE_DEF::IMAGE_FILE_HEADER, SizeOfOptionalHeader));
//                     quint32 NumberOfSections = read_uint16(nStartOffset + offsetof(XPE_DEF::IMAGE_FILE_HEADER, NumberOfSections));

//                     DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
//                     _dataHeadersOptions.nLocation += sizeof(XPE_DEF::IMAGE_FILE_HEADER) + SizeOfOptionalHeader;
//                     _dataHeadersOptions.dsID_parent = dataHeader.dsID;
//                     _dataHeadersOptions.dhMode = XBinary::DHMODE_TABLE;
//                     _dataHeadersOptions.nCount = NumberOfSections;
//                     _dataHeadersOptions.nSize = sizeof(XPE_DEF::IMAGE_SECTION_HEADER) * NumberOfSections;
//                     _dataHeadersOptions.nID = STRUCTID_IMAGE_SECTION_HEADER;
//                     listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//                 }
//             } else if (dataHeadersOptions.nID == STRUCTID_IMAGE_OPTIONAL_HEADER32) {
//                 XBinary::DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, structIDToString(dataHeadersOptions.nID));
//                 dataHeader.nSize = sizeof(XPE_DEF::IMAGE_OPTIONAL_HEADER32);

//                 dataHeader.listRecords.append(getDataRecordDV(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, Magic), 2, "Magic", VT_WORD, DRF_UNKNOWN,
//                                                               dataHeadersOptions.pMemoryMap->endian, XPE::getImageOptionalHeaderMagicS(), VL_TYPE_LIST));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, MajorLinkerVersion), 1, "MajorLinkerVersion", VT_BYTE,
//                 DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, MinorLinkerVersion), 1, "MinorLinkerVersion", VT_BYTE,
//                 DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, SizeOfCode), 4, "SizeOfCode", VT_DWORD, DRF_SIZE, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, SizeOfInitializedData), 4, "SizeOfInitializedData", VT_DWORD,
//                                                             DRF_SIZE, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, SizeOfUninitializedData), 4, "SizeOfUninitializedData",
//                 VT_DWORD,
//                                                             DRF_SIZE, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, AddressOfEntryPoint), 4, "AddressOfEntryPoint", VT_DWORD,
//                                                             DRF_ADDRESS, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, BaseOfCode), 4, "BaseOfCode", VT_DWORD, DRF_ADDRESS,
//                     dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, BaseOfData), 4, "BaseOfData", VT_DWORD, DRF_ADDRESS,
//                     dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, ImageBase), 4, "ImageBase", VT_DWORD, DRF_ADDRESS,
//                     dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, SectionAlignment), 4, "SectionAlignment", VT_DWORD, DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, FileAlignment), 4, "FileAlignment", VT_DWORD, DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, MajorOperatingSystemVersion), 2, "MajorOperatingSystemVersion",
//                                                             VT_WORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, MinorOperatingSystemVersion), 2, "MinorOperatingSystemVersion",
//                                                             VT_WORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, MajorImageVersion), 2, "MajorImageVersion", VT_WORD,
//                 DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, MinorImageVersion), 2, "MinorImageVersion", VT_WORD,
//                 DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, MajorSubsystemVersion), 2, "MajorSubsystemVersion", VT_WORD,
//                                                             DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, MinorSubsystemVersion), 2, "MinorSubsystemVersion", VT_WORD,
//                                                             DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, Win32VersionValue), 4, "Win32VersionValue", VT_DWORD,
//                 DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, SizeOfImage), 4, "SizeOfImage", VT_DWORD, DRF_SIZE,
//                     dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, SizeOfHeaders), 4, "SizeOfHeaders", VT_DWORD, DRF_SIZE,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, CheckSum), 4, "CheckSum", VT_DWORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecordDV(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, Subsystem), 2, "Subsystem", VT_WORD, DRF_UNKNOWN,
//                                                               dataHeadersOptions.pMemoryMap->endian, XPE::getImageOptionalHeaderSubsystemS(), VL_TYPE_LIST));
//                 dataHeader.listRecords.append(getDataRecordDV(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, DllCharacteristics), 2, "DllCharacteristics", VT_WORD,
//                                                               DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian, XPE::getImageOptionalHeaderDllCharacteristicsS(),
//                                                               VL_TYPE_FLAGS));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, SizeOfStackReserve), 4, "SizeOfStackReserve", VT_DWORD,
//                 DRF_SIZE,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, SizeOfStackCommit), 4, "SizeOfStackCommit", VT_DWORD, DRF_SIZE,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, SizeOfHeapReserve), 4, "SizeOfHeapReserve", VT_DWORD, DRF_SIZE,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, SizeOfHeapCommit), 4, "SizeOfHeapCommit", VT_DWORD, DRF_SIZE,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, LoaderFlags), 4, "LoaderFlags", VT_DWORD, DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, NumberOfRvaAndSizes), 4, "NumberOfRvaAndSizes", VT_DWORD,
//                                                             DRF_COUNT, dataHeadersOptions.pMemoryMap->endian));

//                 listResult.append(dataHeader);

//                 if (dataHeadersOptions.bChildren) {
//                     quint32 NumberOfRvaAndSizes = read_uint16(nStartOffset + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, NumberOfRvaAndSizes));

//                     DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
//                     _dataHeadersOptions.nLocation += offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER32, DataDirectory);
//                     _dataHeadersOptions.dsID_parent = dataHeader.dsID;
//                     _dataHeadersOptions.dhMode = XBinary::DHMODE_TABLE;
//                     _dataHeadersOptions.nCount = NumberOfRvaAndSizes;
//                     _dataHeadersOptions.nSize = sizeof(XPE_DEF::IMAGE_DATA_DIRECTORY) * NumberOfRvaAndSizes;
//                     _dataHeadersOptions.nID = STRUCTID_IMAGE_DATA_DIRECTORY;
//                     listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//                 }
//             } else if (dataHeadersOptions.nID == STRUCTID_IMAGE_OPTIONAL_HEADER64) {
//                 XBinary::DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, structIDToString(dataHeadersOptions.nID));
//                 dataHeader.nSize = sizeof(XPE_DEF::IMAGE_OPTIONAL_HEADER64);

//                 dataHeader.listRecords.append(getDataRecordDV(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, Magic), 2, "Magic", VT_WORD, DRF_UNKNOWN,
//                                                               dataHeadersOptions.pMemoryMap->endian, XPE::getImageOptionalHeaderMagicS(), VL_TYPE_LIST));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, MajorLinkerVersion), 1, "MajorLinkerVersion", VT_BYTE,
//                 DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, MinorLinkerVersion), 1, "MinorLinkerVersion", VT_BYTE,
//                 DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, SizeOfCode), 4, "SizeOfCode", VT_DWORD, DRF_SIZE, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, SizeOfInitializedData), 4, "SizeOfInitializedData", VT_DWORD,
//                                                             DRF_SIZE, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, SizeOfUninitializedData), 4, "SizeOfUninitializedData",
//                 VT_DWORD,
//                                                             DRF_SIZE, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, AddressOfEntryPoint), 4, "AddressOfEntryPoint", VT_DWORD,
//                                                             DRF_ADDRESS, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, BaseOfCode), 4, "BaseOfCode", VT_DWORD, DRF_ADDRESS,
//                     dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, ImageBase), 8, "ImageBase", VT_QWORD, DRF_ADDRESS,
//                     dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, SectionAlignment), 4, "SectionAlignment", VT_DWORD, DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, FileAlignment), 4, "FileAlignment", VT_DWORD, DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, MajorOperatingSystemVersion), 2, "MajorOperatingSystemVersion",
//                                                             VT_WORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, MinorOperatingSystemVersion), 2, "MinorOperatingSystemVersion",
//                                                             VT_WORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, MajorImageVersion), 2, "MajorImageVersion", VT_WORD,
//                 DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, MinorImageVersion), 2, "MinorImageVersion", VT_WORD,
//                 DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, MajorSubsystemVersion), 2, "MajorSubsystemVersion", VT_WORD,
//                                                             DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, MinorSubsystemVersion), 2, "MinorSubsystemVersion", VT_WORD,
//                                                             DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, Win32VersionValue), 4, "Win32VersionValue", VT_DWORD,
//                 DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, SizeOfImage), 4, "SizeOfImage", VT_DWORD, DRF_SIZE,
//                     dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, SizeOfHeaders), 4, "SizeOfHeaders", VT_DWORD, DRF_SIZE,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, CheckSum), 4, "CheckSum", VT_DWORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecordDV(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, Subsystem), 2, "Subsystem", VT_WORD, DRF_UNKNOWN,
//                                                               dataHeadersOptions.pMemoryMap->endian, XPE::getImageOptionalHeaderSubsystemS(), VL_TYPE_LIST));
//                 dataHeader.listRecords.append(getDataRecordDV(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, DllCharacteristics), 2, "DllCharacteristics", VT_WORD,
//                                                               DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian, XPE::getImageOptionalHeaderDllCharacteristicsS(),
//                                                               VL_TYPE_FLAGS));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, SizeOfStackReserve), 8, "SizeOfStackReserve", VT_QWORD,
//                 DRF_SIZE,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, SizeOfStackCommit), 8, "SizeOfStackCommit", VT_QWORD, DRF_SIZE,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, SizeOfHeapReserve), 8, "SizeOfHeapReserve", VT_QWORD, DRF_SIZE,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, SizeOfHeapCommit), 8, "SizeOfHeapCommit", VT_QWORD, DRF_SIZE,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, LoaderFlags), 4, "LoaderFlags", VT_DWORD, DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, NumberOfRvaAndSizes), 4, "NumberOfRvaAndSizes", VT_DWORD,
//                                                             DRF_COUNT, dataHeadersOptions.pMemoryMap->endian));
//                 listResult.append(dataHeader);

//                 if (dataHeadersOptions.bChildren) {
//                     quint32 NumberOfRvaAndSizes = read_uint16(nStartOffset + offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, NumberOfRvaAndSizes));

//                     DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
//                     _dataHeadersOptions.nLocation += offsetof(XPE_DEF::IMAGE_OPTIONAL_HEADER64, DataDirectory);
//                     _dataHeadersOptions.dsID_parent = dataHeader.dsID;
//                     _dataHeadersOptions.dhMode = XBinary::DHMODE_TABLE;
//                     _dataHeadersOptions.nCount = NumberOfRvaAndSizes;
//                     _dataHeadersOptions.nSize = sizeof(XPE_DEF::IMAGE_DATA_DIRECTORY) * NumberOfRvaAndSizes;
//                     _dataHeadersOptions.nID = STRUCTID_IMAGE_DATA_DIRECTORY;
//                     listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//                 }
//             } else if (dataHeadersOptions.nID == STRUCTID_IMAGE_SECTION_HEADER) {
//                 XBinary::DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, structIDToString(dataHeadersOptions.nID));
//                 dataHeader.nSize = sizeof(XPE_DEF::IMAGE_SECTION_HEADER);

//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XPE_DEF::IMAGE_SECTION_HEADER, Name), 8, "Name", VT_CHAR_ARRAY, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_SECTION_HEADER, VirtualAddress), 4, "VirtualAddress", VT_DWORD, DRF_ADDRESS,
//                                                             dataHeadersOptions.pMemoryMap->endian));

//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XPE_DEF::IMAGE_SECTION_HEADER, SizeOfRawData), 4, "SizeOfRawData", VT_DWORD, DRF_SIZE,
//                     dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_SECTION_HEADER, PointerToRawData), 4, "PointerToRawData", VT_DWORD, DRF_OFFSET,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_SECTION_HEADER, PointerToRelocations), 4, "PointerToRelocations", VT_DWORD,
//                                                             DRF_OFFSET, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_SECTION_HEADER, PointerToLinenumbers), 4, "PointerToLinenumbers", VT_DWORD,
//                                                             DRF_OFFSET, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_SECTION_HEADER, NumberOfRelocations), 2, "NumberOfRelocations", VT_WORD, DRF_COUNT,
//                                                             dataHeadersOptions.pMemoryMap->endian));

//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_SECTION_HEADER, NumberOfLinenumbers), 2, "NumberOfLinenumbers", VT_WORD, DRF_COUNT,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecordDV(offsetof(XPE_DEF::IMAGE_SECTION_HEADER, Characteristics), 4, "Characteristics", VT_DWORD, DRF_UNKNOWN,
//                                                               dataHeadersOptions.pMemoryMap->endian, XPE::getImageSectionHeaderFlags(), VL_TYPE_FLAGS));

//                 listResult.append(dataHeader);
//             } else if (dataHeadersOptions.nID == STRUCTID_IMAGE_DATA_DIRECTORY) {
//                 XBinary::DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, structIDToString(dataHeadersOptions.nID));
//                 dataHeader.nSize = sizeof(XPE_DEF::IMAGE_DATA_DIRECTORY);

//                 // TODO OFFSET for cert
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_DATA_DIRECTORY, VirtualAddress), 4, "VirtualAddress", VT_DWORD, DRF_ADDRESS,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XPE_DEF::IMAGE_DATA_DIRECTORY, Size), 4, "Size", VT_DWORD, DRF_SIZE, dataHeadersOptions.pMemoryMap->endian));
//                 listResult.append(dataHeader);

//                 if (dataHeadersOptions.bChildren) {
//                     for (qint32 i = 0; i < dataHeadersOptions.nCount; i++) {
//                         XPE_DEF::IMAGE_DATA_DIRECTORY idd = read_IMAGE_DATA_DIRECTORY(nStartOffset + i * sizeof(XPE_DEF::IMAGE_DATA_DIRECTORY));

//                         if (!isDataDirectoryValid(&idd, dataHeadersOptions.pMemoryMap)) {
//                             continue;
//                         }

//                         // Special-case: Security directory uses file offset, not RVA
//                         if (i == XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_SECURITY) {
//                             if (checkOffsetSize(idd.VirtualAddress, idd.Size)) {
//                                 listResult.append(_dataHeaderHex(dataHeadersOptions, structIDToString(STRUCTID_WIN_CERT_RECORD), dataHeader.dsID,
//                                                                  STRUCTID_WIN_CERT_RECORD, idd.VirtualAddress, idd.Size));
//                             }
//                             continue;
//                         }

//                         qint64 nStructOffset = relAddressToOffset(dataHeadersOptions.pMemoryMap, idd.VirtualAddress);
//                         if (nStructOffset == -1) {
//                             continue;
//                         }

//                         DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
//                         _dataHeadersOptions.nLocation = nStructOffset;
//                         _dataHeadersOptions.locType = XBinary::LT_OFFSET;
//                         _dataHeadersOptions.dsID_parent = dataHeader.dsID;
//                         _dataHeadersOptions.dhMode = XBinary::DHMODE_HEADER;
//                         _dataHeadersOptions.nCount = 1;
//                         _dataHeadersOptions.nSize = idd.Size;

//                         switch (i) {
//                             case XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXPORT: _dataHeadersOptions.nID = STRUCTID_IMAGE_EXPORT_DIRECTORY; break;
//                             case XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_IMPORT:
//                                 _dataHeadersOptions.nID = STRUCTID_IMAGE_IMPORT_DESCRIPTOR;
//                                 _dataHeadersOptions.dhMode = XBinary::DHMODE_TABLE;
//                                 _dataHeadersOptions.nCount = idd.Size / sizeof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR);
//                                 break;
//                             case XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_RESOURCE: _dataHeadersOptions.nID = STRUCTID_IMAGE_RESOURCE_DIRECTORY; break;
//                             case XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXCEPTION:
//                                 if (getMode() == MODE_64) {
//                                     _dataHeadersOptions.nID = STRUCTID_IMAGE_RUNTIME_FUNCTION_ENTRY;
//                                     _dataHeadersOptions.dhMode = XBinary::DHMODE_TABLE;
//                                     _dataHeadersOptions.nCount = idd.Size / sizeof(XPE_DEF::S_IMAGE_RUNTIME_FUNCTION_ENTRY);
//                                 } else {
//                                     _dataHeadersOptions.nID = STRUCTID_UNKNOWN;
//                                 }
//                                 break;
//                             case XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_DEBUG:
//                                 _dataHeadersOptions.nID = STRUCTID_IMAGE_DEBUG_DIRECTORY;
//                                 _dataHeadersOptions.dhMode = XBinary::DHMODE_TABLE;
//                                 _dataHeadersOptions.nCount = idd.Size / sizeof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY);
//                                 break;
//                             case XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_TLS:
//                                 _dataHeadersOptions.nID = (getMode() == MODE_64) ? STRUCTID_IMAGE_TLS_DIRECTORY64 : STRUCTID_IMAGE_TLS_DIRECTORY32;
//                                 break;
//                             case XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG:
//                                 _dataHeadersOptions.nID = (getMode() == MODE_64) ? STRUCTID_IMAGE_LOAD_CONFIG_DIRECTORY64 : STRUCTID_IMAGE_LOAD_CONFIG_DIRECTORY32;
//                                 break;
//                             case XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT:
//                                 _dataHeadersOptions.nID = STRUCTID_IMAGE_BOUND_IMPORT_DESCRIPTOR;
//                                 _dataHeadersOptions.dhMode = XBinary::DHMODE_TABLE;
//                                 _dataHeadersOptions.nCount = idd.Size / sizeof(XPE_DEF::IMAGE_BOUND_IMPORT_DESCRIPTOR);
//                                 break;
//                             case XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_IAT:
//                                 // Show as HEX chunk
//                                 listResult.append(_dataHeaderHex(dataHeadersOptions, QString("IAT"), dataHeader.dsID, STRUCTID_UNKNOWN, nStructOffset, idd.Size));
//                                 _dataHeadersOptions.nID = STRUCTID_UNKNOWN;
//                                 break;
//                             case XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT:
//                                 _dataHeadersOptions.nID = STRUCTID_IMAGE_DELAYLOAD_DESCRIPTOR;
//                                 _dataHeadersOptions.dhMode = XBinary::DHMODE_TABLE;
//                                 _dataHeadersOptions.nCount = idd.Size / sizeof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR);
//                                 break;
//                             case XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR: _dataHeadersOptions.nID = STRUCTID_IMAGE_COR20_HEADER; break;
//                             default: _dataHeadersOptions.nID = STRUCTID_UNKNOWN; break;
//                         }

//                         if (_dataHeadersOptions.nID != STRUCTID_UNKNOWN) {
//                             listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//                         }
//                     }
//                 }
//             } else if (dataHeadersOptions.nID == STRUCTID_IMAGE_RESOURCE_DIRECTORY) {
//                 XBinary::DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, structIDToString(dataHeadersOptions.nID));
//                 dataHeader.nSize = sizeof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY);

//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY, Characteristics), 4, "Characteristics", VT_DWORD, DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY, TimeDateStamp), 4, "TimeDateStamp", VT_DWORD, DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY, MajorVersion), 2, "MajorVersion", VT_WORD, DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY, MinorVersion), 2, "MinorVersion", VT_WORD, DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY, NumberOfNamedEntries), 2, "NumberOfNamedEntries", VT_WORD,
//                                                             DRF_COUNT, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY, NumberOfIdEntries), 2, "NumberOfIdEntries", VT_WORD, DRF_COUNT,
//                                                             dataHeadersOptions.pMemoryMap->endian));

//                 listResult.append(dataHeader);

//                 // if (dataHeadersOptions.bChildren) {
//                 //     DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
//                 //     _dataHeadersOptions.nLocation += sizeof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY);
//                 //     _dataHeadersOptions.locType = XBinary::LT_OFFSET;
//                 //     _dataHeadersOptions.nID = STRUCTID_IMAGE_RESOURCE_DIRECTORY_ENTRY;
//                 //     _dataHeadersOptions.dsID_parent = dataHeader.dsID;
//                 //     _dataHeadersOptions.dhMode = XBinary::DHMODE_TABLE;
//                 //     _dataHeadersOptions.nCount = read_uint16(nStartOffset + offsetof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY, NumberOfNamedEntries)) +
//                 //                                 read_uint16(nStartOffset + offsetof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY, NumberOfIdEntries));
//                 //     _dataHeadersOptions.nSize = sizeof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY_ENTRY) *
//                 //                                 _dataHeadersOptions.nCount; // TODO: check size
//                 //     listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
//                 // }
//             } else if (dataHeadersOptions.nID == STRUCTID_IMAGE_COR20_HEADER) {
//                 XBinary::DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, structIDToString(dataHeadersOptions.nID));
//                 dataHeader.nSize = sizeof(XPE_DEF::IMAGE_COR20_HEADER);

//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XPE_DEF::IMAGE_COR20_HEADER, cb), 4, "cb", VT_DWORD, DRF_SIZE, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_COR20_HEADER, MajorRuntimeVersion), 2, "MajorRuntimeVersion", VT_WORD, DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_COR20_HEADER, MinorRuntimeVersion), 2, "MinorRuntimeVersion", VT_WORD, DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_COR20_HEADER, MetaData.VirtualAddress), 4, "MetaData.VirtualAddress", VT_DWORD,
//                                                             DRF_ADDRESS, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XPE_DEF::IMAGE_COR20_HEADER, MetaData.Size), 4, "MetaData.Size", VT_DWORD, DRF_SIZE,
//                     dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XPE_DEF::IMAGE_COR20_HEADER, Flags), 4, "Flags", VT_DWORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_COR20_HEADER, EntryPointToken), 4, "EntryPointToken", VT_DWORD, DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_COR20_HEADER, Resources.VirtualAddress), 4, "Resources.VirtualAddress", VT_DWORD,
//                                                             DRF_ADDRESS, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XPE_DEF::IMAGE_COR20_HEADER, Resources.Size), 4, "Resources.Size", VT_DWORD, DRF_SIZE,
//                     dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_COR20_HEADER, StrongNameSignature.VirtualAddress), 4,
//                                                             "StrongNameSignature.VirtualAddress", VT_DWORD, DRF_ADDRESS, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_COR20_HEADER, StrongNameSignature.Size), 4, "StrongNameSignature.Size", VT_DWORD,
//                                                             DRF_SIZE, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_COR20_HEADER, CodeManagerTable.VirtualAddress), 4,
//                 "CodeManagerTable.VirtualAddress",
//                                                             VT_DWORD, DRF_ADDRESS, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_COR20_HEADER, CodeManagerTable.Size), 4, "CodeManagerTable.Size", VT_DWORD,
//                 DRF_SIZE,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_COR20_HEADER, VTableFixups.VirtualAddress), 4, "VTableFixups.VirtualAddress",
//                                                             VT_DWORD, DRF_ADDRESS, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_COR20_HEADER, VTableFixups.Size), 4, "VTableFixups.Size", VT_DWORD, DRF_SIZE,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_COR20_HEADER, ExportAddressTableJumps.VirtualAddress), 4,
//                                                             "ExportAddressTableJumps.VirtualAddress", VT_DWORD, DRF_ADDRESS, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_COR20_HEADER, ExportAddressTableJumps.Size), 4, "ExportAddressTableJumps.Size",
//                                                             VT_DWORD, DRF_SIZE, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_COR20_HEADER, ManagedNativeHeader.VirtualAddress), 4,
//                                                             "ManagedNativeHeader.VirtualAddress", VT_DWORD, DRF_ADDRESS, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_COR20_HEADER, ManagedNativeHeader.Size), 4, "ManagedNativeHeader.Size", VT_DWORD,
//                                                             DRF_SIZE, dataHeadersOptions.pMemoryMap->endian));

//                 listResult.append(dataHeader);

//                 if (dataHeadersOptions.bChildren) {
//                     XPE_DEF::IMAGE_COR20_HEADER ich = _read_IMAGE_COR20_HEADER(nStartOffset);

//                     if (isDataDirectoryValid(&(ich.MetaData), dataHeadersOptions.pMemoryMap)) {
//                         qint64 nStructOffset = relAddressToOffset(dataHeadersOptions.pMemoryMap, ich.MetaData.VirtualAddress);

//                         if (nStructOffset != -1) {
//                             quint32 nVersionLength = read_uint32(nStructOffset + 12);

//                             DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
//                             _dataHeadersOptions.nLocation = nStructOffset;
//                             _dataHeadersOptions.dsID_parent = dataHeader.dsID;
//                             _dataHeadersOptions.dhMode = XBinary::DHMODE_HEADER;
//                             _dataHeadersOptions.nCount = 1;
//                             _dataHeadersOptions.nSize = 20 + nVersionLength;  // Header + version string + 2 words
//                             // _dataHeadersOptions.nSize = ich.MetaData.Size;
//                             _dataHeadersOptions.nID = STRUCTID_NET_METADATA;
//                             listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));

//                             quint16 nNumberOfStreams = read_uint16(nStructOffset + 18 + nVersionLength);

//                             // Streams
//                             qint64 nStreamOffset = nStructOffset + 20 + nVersionLength;
//                             qint32 i = 0;

//                             while ((i < nNumberOfStreams) && (nStreamOffset + 8 < nStructOffset + ich.MetaData.Size)) {
//                                 QString sName = read_ansiString(nStreamOffset + 8, 32);
//                                 qint32 j = sName.length();

//                                 // Align
//                                 j = (j + 4) & ~3;

//                                 quint32 nOffset = read_uint32(nStreamOffset);    // TODO Check
//                                 quint32 nSize = read_uint32(nStreamOffset + 4);  // TODO Check

//                                 listResult.append(_dataHeaderHex(dataHeadersOptions, sName, dataHeader.dsID, STRUCTID_NET_METADATA_STREAM, nOffset, nSize));

//                                 nStreamOffset += 8 + j;
//                                 i++;
//                             }
//                         }
//                     }

//                     if (isDataDirectoryValid(&(ich.Resources), dataHeadersOptions.pMemoryMap)) {
//                         qint64 nStructOffset = relAddressToOffset(dataHeadersOptions.pMemoryMap, ich.Resources.VirtualAddress);

//                         if (nStructOffset != -1) {
//                             listResult.append(_dataHeaderHex(dataHeadersOptions, structIDToString(STRUCTID_NET_RESOURCES), dataHeader.dsID, STRUCTID_NET_RESOURCES,
//                                                              nStructOffset, ich.Resources.Size));
//                         }
//                     }

//                     if (isDataDirectoryValid(&(ich.StrongNameSignature), dataHeadersOptions.pMemoryMap)) {
//                         qint64 nStructOffset = relAddressToOffset(dataHeadersOptions.pMemoryMap, ich.StrongNameSignature.VirtualAddress);

//                         if (nStructOffset != -1) {
//                             listResult.append(_dataHeaderHex(dataHeadersOptions, structIDToString(STRUCTID_NET_STRONGNAMESIGNATURE), dataHeader.dsID,
//                                                              STRUCTID_NET_STRONGNAMESIGNATURE, nStructOffset, ich.StrongNameSignature.Size));
//                         }
//                     }

//                     if (isDataDirectoryValid(&(ich.CodeManagerTable), dataHeadersOptions.pMemoryMap)) {
//                         qint64 nStructOffset = relAddressToOffset(dataHeadersOptions.pMemoryMap, ich.CodeManagerTable.VirtualAddress);

//                         if (nStructOffset != -1) {
//                             listResult.append(_dataHeaderHex(dataHeadersOptions, structIDToString(STRUCTID_NET_CODEMANAGERTABLE), dataHeader.dsID,
//                                                              STRUCTID_NET_CODEMANAGERTABLE, nStructOffset, ich.CodeManagerTable.Size));
//                         }
//                     }

//                     if (isDataDirectoryValid(&(ich.VTableFixups), dataHeadersOptions.pMemoryMap)) {
//                         qint64 nStructOffset = relAddressToOffset(dataHeadersOptions.pMemoryMap, ich.VTableFixups.VirtualAddress);

//                         if (nStructOffset != -1) {
//                             listResult.append(_dataHeaderHex(dataHeadersOptions, structIDToString(STRUCTID_NET_VTABLEFIXUPS), dataHeader.dsID,
//                             STRUCTID_NET_VTABLEFIXUPS,
//                                                              nStructOffset, ich.VTableFixups.Size));
//                         }
//                     }

//                     if (isDataDirectoryValid(&(ich.ExportAddressTableJumps), dataHeadersOptions.pMemoryMap)) {
//                         qint64 nStructOffset = relAddressToOffset(dataHeadersOptions.pMemoryMap, ich.ExportAddressTableJumps.VirtualAddress);

//                         if (nStructOffset != -1) {
//                             listResult.append(_dataHeaderHex(dataHeadersOptions, structIDToString(STRUCTID_NET_EXPORTADDRESSTABLEJUMPS), dataHeader.dsID,
//                                                              STRUCTID_NET_EXPORTADDRESSTABLEJUMPS, nStructOffset, ich.ExportAddressTableJumps.Size));
//                         }
//                     }

//                     if (isDataDirectoryValid(&(ich.ManagedNativeHeader), dataHeadersOptions.pMemoryMap)) {
//                         qint64 nStructOffset = relAddressToOffset(dataHeadersOptions.pMemoryMap, ich.ManagedNativeHeader.VirtualAddress);

//                         if (nStructOffset != -1) {
//                             listResult.append(_dataHeaderHex(dataHeadersOptions, structIDToString(STRUCTID_NET_MANAGEDNATIVEHEADER), dataHeader.dsID,
//                                                              STRUCTID_NET_MANAGEDNATIVEHEADER, nStructOffset, ich.ManagedNativeHeader.Size));
//                         }
//                     }
//                 }
//             } else if (dataHeadersOptions.nID == STRUCTID_IMAGE_EXPORT_DIRECTORY) {
//                 XBinary::DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, structIDToString(dataHeadersOptions.nID));
//                 dataHeader.nSize = sizeof(XPE_DEF::IMAGE_EXPORT_DIRECTORY);
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, Characteristics), 4, "Characteristics", VT_DWORD, DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, TimeDateStamp), 4, "TimeDateStamp", VT_DWORD, DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, MajorVersion), 2, "MajorVersion", VT_WORD, DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, MinorVersion), 2, "MinorVersion", VT_WORD, DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, Name), 4, "Name", VT_DWORD, DRF_ADDRESS, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, Base), 4, "Base", VT_DWORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, NumberOfFunctions), 4, "NumberOfFunctions", VT_DWORD, DRF_COUNT,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, NumberOfNames), 4, "NumberOfNames", VT_DWORD, DRF_COUNT,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, AddressOfFunctions), 4, "AddressOfFunctions", VT_DWORD,
//                 DRF_ADDRESS,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, AddressOfNames), 4, "AddressOfNames", VT_DWORD, DRF_ADDRESS,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_EXPORT_DIRECTORY, AddressOfNameOrdinals), 4, "AddressOfNameOrdinals", VT_DWORD,
//                                                             DRF_ADDRESS, dataHeadersOptions.pMemoryMap->endian));
//                 listResult.append(dataHeader);
//             } else if (dataHeadersOptions.nID == STRUCTID_IMAGE_IMPORT_DESCRIPTOR) {
//                 XBinary::DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, structIDToString(dataHeadersOptions.nID));
//                 dataHeader.nSize = sizeof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR);
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR, OriginalFirstThunk), 4, "OriginalFirstThunk", VT_DWORD,
//                                                             DRF_ADDRESS, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR, TimeDateStamp), 4, "TimeDateStamp", VT_DWORD, DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR, ForwarderChain), 4, "ForwarderChain", VT_DWORD, DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR, Name), 4, "Name", VT_DWORD, DRF_ADDRESS, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XPE_DEF::IMAGE_IMPORT_DESCRIPTOR, FirstThunk), 4, "FirstThunk", VT_DWORD, DRF_ADDRESS,
//                     dataHeadersOptions.pMemoryMap->endian));
//                 listResult.append(dataHeader);
//             } else if (dataHeadersOptions.nID == STRUCTID_IMAGE_BOUND_IMPORT_DESCRIPTOR) {
//                 XBinary::DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, structIDToString(dataHeadersOptions.nID));
//                 dataHeader.nSize = sizeof(XPE_DEF::IMAGE_BOUND_IMPORT_DESCRIPTOR);
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_BOUND_IMPORT_DESCRIPTOR, TimeDateStamp), 4, "TimeDateStamp", VT_DWORD, DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_BOUND_IMPORT_DESCRIPTOR, OffsetModuleName), 2, "OffsetModuleName", VT_WORD,
//                                                             DRF_OFFSET, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_BOUND_IMPORT_DESCRIPTOR, NumberOfModuleForwarderRefs), 2,
//                                                             "NumberOfModuleForwarderRefs", VT_WORD, DRF_COUNT, dataHeadersOptions.pMemoryMap->endian));
//                 listResult.append(dataHeader);
//             } else if (dataHeadersOptions.nID == STRUCTID_IMAGE_DEBUG_DIRECTORY) {
//                 XBinary::DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, structIDToString(dataHeadersOptions.nID));
//                 dataHeader.nSize = sizeof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY);
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY, Characteristics), 4, "Characteristics", VT_DWORD, DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY, TimeDateStamp), 4, "TimeDateStamp", VT_DWORD, DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY, MajorVersion), 2, "MajorVersion", VT_WORD, DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY, MinorVersion), 2, "MinorVersion", VT_WORD, DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecordDV(offsetof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY, Type), 4, "Type", VT_DWORD, DRF_UNKNOWN,
//                                                               dataHeadersOptions.pMemoryMap->endian, XPE::getDebugTypesS(), VL_TYPE_LIST));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY, SizeOfData), 4, "SizeOfData", VT_DWORD, DRF_SIZE, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY, AddressOfRawData), 4, "AddressOfRawData", VT_DWORD, DRF_ADDRESS,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_DEBUG_DIRECTORY, PointerToRawData), 4, "PointerToRawData", VT_DWORD, DRF_OFFSET,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 listResult.append(dataHeader);
//             } else if (dataHeadersOptions.nID == STRUCTID_IMAGE_TLS_DIRECTORY32) {
//                 XBinary::DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, structIDToString(dataHeadersOptions.nID));
//                 dataHeader.nSize = sizeof(XPE_DEF::S_IMAGE_TLS_DIRECTORY32);
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY32, StartAddressOfRawData), 4, "StartAddressOfRawData", VT_DWORD,
//                                                             DRF_ADDRESS, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY32, EndAddressOfRawData), 4, "EndAddressOfRawData", VT_DWORD,
//                                                             DRF_ADDRESS, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY32, AddressOfIndex), 4, "AddressOfIndex", VT_DWORD, DRF_ADDRESS,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY32, AddressOfCallBacks), 4, "AddressOfCallBacks", VT_DWORD,
//                                                             DRF_ADDRESS, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY32, SizeOfZeroFill), 4, "SizeOfZeroFill", VT_DWORD, DRF_SIZE,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY32, Characteristics), 4, "Characteristics", VT_DWORD, DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 listResult.append(dataHeader);
//             } else if (dataHeadersOptions.nID == STRUCTID_IMAGE_TLS_DIRECTORY64) {
//                 XBinary::DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, structIDToString(dataHeadersOptions.nID));
//                 dataHeader.nSize = sizeof(XPE_DEF::S_IMAGE_TLS_DIRECTORY64);
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY64, StartAddressOfRawData), 8, "StartAddressOfRawData", VT_QWORD,
//                                                             DRF_ADDRESS, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY64, EndAddressOfRawData), 8, "EndAddressOfRawData", VT_QWORD,
//                                                             DRF_ADDRESS, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY64, AddressOfIndex), 8, "AddressOfIndex", VT_QWORD, DRF_ADDRESS,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY64, AddressOfCallBacks), 8, "AddressOfCallBacks", VT_QWORD,
//                                                             DRF_ADDRESS, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY64, SizeOfZeroFill), 4, "SizeOfZeroFill", VT_DWORD, DRF_SIZE,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY64, Characteristics), 4, "Characteristics", VT_DWORD, DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 listResult.append(dataHeader);
//             } else if (dataHeadersOptions.nID == STRUCTID_IMAGE_LOAD_CONFIG_DIRECTORY32) {
//                 XBinary::DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, structIDToString(dataHeadersOptions.nID));
//                 dataHeader.nSize = sizeof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32);
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, Size), 4, "Size", VT_DWORD, DRF_SIZE, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, TimeDateStamp), 4, "TimeDateStamp", VT_DWORD,
//                 DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, MajorVersion), 2, "MajorVersion", VT_WORD, DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, MinorVersion), 2, "MinorVersion", VT_WORD, DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, SecurityCookie), 4, "SecurityCookie", VT_DWORD,
//                                                             DRF_ADDRESS, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32, GuardFlags), 4, "GuardFlags", VT_DWORD, DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 listResult.append(dataHeader);
//             } else if (dataHeadersOptions.nID == STRUCTID_IMAGE_LOAD_CONFIG_DIRECTORY64) {
//                 XBinary::DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, structIDToString(dataHeadersOptions.nID));
//                 dataHeader.nSize = sizeof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64);
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, Size), 4, "Size", VT_DWORD, DRF_SIZE, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, TimeDateStamp), 4, "TimeDateStamp", VT_DWORD,
//                 DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, MajorVersion), 2, "MajorVersion", VT_WORD, DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, MinorVersion), 2, "MinorVersion", VT_WORD, DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, SecurityCookie), 8, "SecurityCookie", VT_QWORD,
//                                                             DRF_ADDRESS, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64, GuardFlags), 4, "GuardFlags", VT_DWORD, DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 listResult.append(dataHeader);
//             } else if (dataHeadersOptions.nID == STRUCTID_IMAGE_DELAYLOAD_DESCRIPTOR) {
//                 XBinary::DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, structIDToString(dataHeadersOptions.nID));
//                 dataHeader.nSize = sizeof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR);
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR, AllAttributes), 4, "AllAttributes", VT_DWORD, DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR, DllNameRVA), 4, "DllNameRVA", VT_DWORD, DRF_ADDRESS,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR, ModuleHandleRVA), 4, "ModuleHandleRVA", VT_DWORD,
//                 DRF_ADDRESS,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR, ImportAddressTableRVA), 4, "ImportAddressTableRVA",
//                 VT_DWORD,
//                                                             DRF_ADDRESS, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR, ImportNameTableRVA), 4, "ImportNameTableRVA", VT_DWORD,
//                                                             DRF_ADDRESS, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR, BoundImportAddressTableRVA), 4,
//                 "BoundImportAddressTableRVA",
//                                                             VT_DWORD, DRF_ADDRESS, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR, UnloadInformationTableRVA), 4, "UnloadInformationTableRVA",
//                                                             VT_DWORD, DRF_ADDRESS, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR, TimeDateStamp), 4, "TimeDateStamp", VT_DWORD, DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 listResult.append(dataHeader);
//             } else if (dataHeadersOptions.nID == STRUCTID_IMAGE_RESOURCE_DIRECTORY_ENTRY) {
//                 XBinary::DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, structIDToString(dataHeadersOptions.nID));
//                 dataHeader.nSize = sizeof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY_ENTRY);
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY_ENTRY, Name), 4, "Name", VT_DWORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY_ENTRY, OffsetToData), 4, "OffsetToData", VT_DWORD, DRF_OFFSET,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 listResult.append(dataHeader);
//             } else if (dataHeadersOptions.nID == STRUCTID_IMAGE_RESOURCE_DATA_ENTRY) {
//                 XBinary::DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, structIDToString(dataHeadersOptions.nID));
//                 dataHeader.nSize = sizeof(XPE_DEF::IMAGE_RESOURCE_DATA_ENTRY);
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_RESOURCE_DATA_ENTRY, OffsetToData), 4, "OffsetToData", VT_DWORD, DRF_OFFSET,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XPE_DEF::IMAGE_RESOURCE_DATA_ENTRY, Size), 4, "Size", VT_DWORD, DRF_SIZE, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XPE_DEF::IMAGE_RESOURCE_DATA_ENTRY, CodePage), 4, "CodePage", VT_DWORD, DRF_UNKNOWN,
//                     dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XPE_DEF::IMAGE_RESOURCE_DATA_ENTRY, Reserved), 4, "Reserved", VT_DWORD, DRF_UNKNOWN,
//                     dataHeadersOptions.pMemoryMap->endian));
//                 listResult.append(dataHeader);
//             } else if (dataHeadersOptions.nID == STRUCTID_IMAGE_BASE_RELOCATION) {
//                 XBinary::DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, structIDToString(dataHeadersOptions.nID));
//                 dataHeader.nSize = sizeof(XPE_DEF::IMAGE_BASE_RELOCATION);
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::IMAGE_BASE_RELOCATION, VirtualAddress), 4, "VirtualAddress", VT_DWORD, DRF_ADDRESS,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XPE_DEF::IMAGE_BASE_RELOCATION, SizeOfBlock), 4, "SizeOfBlock", VT_DWORD, DRF_SIZE, dataHeadersOptions.pMemoryMap->endian));
//                 listResult.append(dataHeader);
//             } else if (dataHeadersOptions.nID == STRUCTID_IMAGE_RUNTIME_FUNCTION_ENTRY) {
//                 XBinary::DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, structIDToString(dataHeadersOptions.nID));
//                 dataHeader.nSize = sizeof(XPE_DEF::S_IMAGE_RUNTIME_FUNCTION_ENTRY);
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_RUNTIME_FUNCTION_ENTRY, BeginAddress), 4, "BeginAddress", VT_DWORD, DRF_ADDRESS,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_RUNTIME_FUNCTION_ENTRY, EndAddress), 4, "EndAddress", VT_DWORD, DRF_ADDRESS,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XPE_DEF::S_IMAGE_RUNTIME_FUNCTION_ENTRY, UnwindInfoAddress), 4, "UnwindInfoAddress", VT_DWORD,
//                                                             DRF_ADDRESS, dataHeadersOptions.pMemoryMap->endian));
//                 listResult.append(dataHeader);
//             } else if (dataHeadersOptions.nID == STRUCTID_NET_METADATA) {
//                 XBinary::DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, structIDToString(dataHeadersOptions.nID));

//                 // .NET Metadata Storage Signature Header
//                 // quint32 nSignature = read_uint32(nStartOffset);
//                 // quint16 nMajorVersion = read_uint16(nStartOffset + 4);
//                 // quint16 nMinorVersion = read_uint16(nStartOffset + 6);
//                 // quint32 nExtraData = read_uint32(nStartOffset + 8);
//                 quint32 nVersionLength = read_uint32(nStartOffset + 12);

//                 dataHeader.nSize = 16 + nVersionLength;  // Header + version string

//                 dataHeader.listRecords.append(getDataRecord(0, 4, "Signature", VT_DWORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(4, 2, "MajorVersion", VT_WORD, DRF_VERSION, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(6, 2, "MinorVersion", VT_WORD, DRF_VERSION, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(8, 4, "ExtraData", VT_DWORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(12, 4, "VersionLength", VT_DWORD, DRF_SIZE, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(16, nVersionLength, "Version", VT_CHAR_ARRAY, DRF_VOLATILE, dataHeadersOptions.pMemoryMap->endian));
//                 // Flags
//                 dataHeader.listRecords.append(getDataRecordDV(16 + nVersionLength, 2, "Flags", VT_WORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian,
//                                                               XPE::getNetMetadataFlagsS(), VL_TYPE_FLAGS));
//                 dataHeader.listRecords.append(getDataRecord(18 + nVersionLength, 2, "NumberOfStreams", VT_WORD, DRF_COUNT, dataHeadersOptions.pMemoryMap->endian));

//                 listResult.append(dataHeader);
//             }
//         }
//     }

//     return listResult;
// }

QList<XBinary::FPART> XPE::getFileParts(quint32 nFileParts, qint32 nLimit, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(nLimit);

    QList<XBinary::FPART> listResult;
    QList<XBinary::FPART> _listCalc;

    bool bCalcAdress = false;

    if ((nFileParts & FILEPART_RESOURCE) || (nFileParts & FILEPART_DEBUGDATA)) {
        bCalcAdress = true;
    }

    OFFSETSIZE osStringTable = getStringTable();

    XADDR nModuleAddress = getModuleAddress();
    qint64 nTotalSize = getSize();
    qint64 nMaxOffset = 0;
    qint32 nNumberOfSections = qMin((qint32)getFileHeader_NumberOfSections(), XPE_DEF::S_MAX_SECTIONCOUNT);
    qint64 nFileAlignment = getOptionalHeader_FileAlignment();
    qint64 nSectionAlignment = getOptionalHeader_SectionAlignment();
    // qint64 nBaseAddress=getOptionalHeader_ImageBase();
    qint64 nHeadersSize = getOptionalHeader_SizeOfHeaders();  // mb TODO calc for UPX

    if (nFileAlignment > 0x10000)  // Invalid file
    {
        nFileAlignment = 0x200;
    }

    if (nSectionAlignment > 0x10000)  // Invalid file
    {
        nSectionAlignment = 0x1000;
    }

    if (nHeadersSize > getSize()) {
        nHeadersSize = getSize();
    }

    //    if(nFileAlignment==nSectionAlignment)
    //    {
    //        nFileAlignment=1;
    //    }
    // Check Format
    bool bValid = false;

    if (nHeadersSize != 0) {
        bValid = true;
    }

    if (bCalcAdress || (nFileParts & FILEPART_HEADER) || (nFileParts & FILEPART_OVERLAY)) {
        FPART record = {};
        record.filePart = FILEPART_HEADER;
        record.nVirtualAddress = nModuleAddress;
        record.nVirtualSize = S_ALIGN_UP64(nHeadersSize, nSectionAlignment);
        record.nFileOffset = 0;
        record.nFileSize = nHeadersSize;
        record.sName = tr("Header");

        if (bCalcAdress) {
            _listCalc.append(record);
        }

        if (nFileParts & FILEPART_HEADER) {
            listResult.append(record);
        }

        nMaxOffset = qMax(nMaxOffset, record.nFileOffset + record.nFileSize);
    }

    if (bCalcAdress || (nFileParts & FILEPART_SECTION) || (nFileParts & FILEPART_OVERLAY)) {
        for (qint32 i = 0; i < nNumberOfSections; i++) {
            XPE_DEF::IMAGE_SECTION_HEADER section = getSectionHeader(i);

            if (section.PointerToRawData > nTotalSize) {
                section.PointerToRawData = 0;
            }

            if (section.PointerToRawData + section.SizeOfRawData > nTotalSize) {
                // Corrupted files
                section.SizeOfRawData = nTotalSize - section.PointerToRawData;
            }

            FPART record = {};
            record.filePart = FILEPART_SECTION;
            record.nVirtualAddress = nModuleAddress + section.VirtualAddress;
            record.nVirtualSize = S_ALIGN_UP(section.Misc.VirtualSize, nSectionAlignment);
            record.nFileOffset = S_ALIGN_DOWN64(section.PointerToRawData, nFileAlignment);
            record.nFileSize = section.SizeOfRawData + (section.PointerToRawData - record.nFileOffset);

            if (bCalcAdress) {
                _listCalc.append(record);
            }

            if (nFileParts & FILEPART_SECTION) {
                QString _sSectionName = QString::fromLatin1((const char *)section.Name, qstrnlen((const char *)section.Name, 8));
                _sSectionName.resize(qMin(_sSectionName.length(), XPE_DEF::S_IMAGE_SIZEOF_SHORT_NAME));
                _sSectionName = convertSectionName(_sSectionName, &osStringTable);
                record.mapProperties.insert(FPART_PROP_ORIGINALNAME, _sSectionName);
                record.sName = QString("%1 (%2) [\"%3\"]").arg(tr("Section")).arg(QString::number(i + 1)).arg(_sSectionName);
                listResult.append(record);
            }

            nMaxOffset = qMax(nMaxOffset, record.nFileOffset + record.nFileSize);
        }
    }

    _MEMORY_MAP memoryMap = {};

    if (bCalcAdress) {
        memoryMap = _getMemoryMap(&_listCalc, pPdStruct);
    }

    if (nFileParts & FILEPART_RESOURCE) {
        if (isResourcesPresent()) {
            QList<XPE::RESOURCE_RECORD> listResources = getResources(&memoryMap, 10000, pPdStruct);

            qint32 nNumberOfRecords = listResources.count();

            qint32 _nFreeIndex = XBinary::getFreeIndex(pPdStruct);
            XBinary::setPdStructInit(pPdStruct, _nFreeIndex, nNumberOfRecords);

            for (qint32 i = 0; (i < nNumberOfRecords) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
                qint64 nResourceOffset = listResources.at(i).nOffset;
                qint64 nResourceSize = listResources.at(i).nSize;
                XADDR nAddress = listResources.at(i).nAddress;

                if (checkOffsetSize(nResourceOffset, nResourceSize)) {
                    FPART record = {};
                    record.filePart = XBinary::FILEPART_RESOURCE;
                    record.nFileOffset = nResourceOffset;
                    record.nFileSize = nResourceSize;
                    record.nVirtualAddress = nAddress;
                    record.nVirtualSize = nResourceSize;
                    record.sName = QString("%1 %2").arg(tr("Resource")).arg(QString::number(i));

                    if (listResources.at(i).irin[0].bIsName) {
                        record.mapProperties.insert(FPART_PROP_RESOURCEID, listResources.at(i).irin[0].sName);
                    } else {
                        record.mapProperties.insert(FPART_PROP_RESOURCEID, listResources.at(i).irin[0].nID);
                    }

                    listResult.append(record);
                }

                XBinary::setPdStructCurrentIncrement(pPdStruct, _nFreeIndex);
            }

            XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);
        }
    }

    if (nFileParts & FILEPART_DEBUGDATA) {
        if (isDebugPresent()) {
            QList<XPE_DEF::S_IMAGE_DEBUG_DIRECTORY> listDebug = getDebugList(&memoryMap, pPdStruct);

            qint32 nNumberOfRecords = listDebug.count();

            qint32 _nFreeIndex = XBinary::getFreeIndex(pPdStruct);
            XBinary::setPdStructInit(pPdStruct, _nFreeIndex, nNumberOfRecords);

            for (qint32 i = 0; (i < nNumberOfRecords) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
                qint64 nRecordOffset = listDebug.at(i).PointerToRawData;
                qint64 nRecordSize = listDebug.at(i).SizeOfData;
                quint32 nRecordType = listDebug.at(i).Type;

                if ((nRecordType == 0) || (nRecordType == 2)) {
                    if (checkOffsetSize(nRecordOffset, nRecordSize)) {
                        FPART record = {};
                        record.filePart = XBinary::FILEPART_DEBUGDATA;
                        record.nFileOffset = nRecordOffset;
                        record.nFileSize = nRecordSize;
                        record.sName = QString::number(nRecordType);

                        listResult.append(record);
                    }
                }

                XBinary::setPdStructCurrentIncrement(pPdStruct, _nFreeIndex);
            }

            XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);
        }
    }

    if (nFileParts & FILEPART_OVERLAY) {
        if (nMaxOffset < nTotalSize) {
            FPART record = {};

            record.filePart = FILEPART_OVERLAY;
            record.nFileOffset = nMaxOffset;
            record.nFileSize = getSize() - nMaxOffset;
            record.nVirtualAddress = -1;
            record.sName = tr("Overlay");

            listResult.append(record);
        }
    }

    return listResult;
}

qint64 XPE::calculateHeadersSize()
{
    return _calculateHeadersSize(getSectionsTableOffset(), getFileHeader_NumberOfSections());
}

qint64 XPE::_calculateHeadersSize(qint64 nSectionsTableOffset, quint32 nNumberOfSections)
{
    qint64 nHeadersSize = nSectionsTableOffset + sizeof(XPE_DEF::IMAGE_SECTION_HEADER) * nNumberOfSections;
    qint64 nFileAlignment = getOptionalHeader_FileAlignment();
    nHeadersSize = S_ALIGN_UP64(nHeadersSize, nFileAlignment);

    return nHeadersSize;
}

bool XPE::isDll()
{
    return (getType() == TYPE_DLL);
}

bool XPE::isDll(const QString &sFileName)
{
    bool bResult = false;

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        XPE pe(&file);

        if (pe.isValid()) {
            bResult = pe.isDll();
        }

        file.close();
    }

    return bResult;
}

bool XPE::isConsole()
{
    return (getType() == TYPE_CONSOLE);
}

bool XPE::isConsole(const QString &sFileName)
{
    bool bResult = false;

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        XPE pe(&file);

        if (pe.isValid()) {
            bResult = pe.isConsole();
        }

        file.close();
    }

    return bResult;
}

bool XPE::isDriver()
{
    return (getType() == TYPE_DRIVER);
}

bool XPE::isNETPresent()
{
    // TODO more checks
    return isOptionalHeader_DataDirectoryPresent(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);
}

bool XPE::isNETPresent(QIODevice *pDevice)
{
    XPE pe(pDevice);

    return pe.isNETPresent();
}

XCLIAssembly *XPE::getCliAssembly(PDSTRUCT *pPdStruct)
{
    XCLIAssembly *pResult = nullptr;

    if (isNETPresent()) {
        pResult = new XCLIAssembly(getDevice(), isImage(), getModuleAddress());

        initCLIAssembly(pResult, pPdStruct);
    }

    return pResult;
}

void XPE::initCLIAssembly(XCLIAssembly *pCLIAssembly, PDSTRUCT *pPdStruct)
{
    if (!isNETPresent()) {
        return;
    }

    qint64 nCLIHeaderOffset = getNetHeaderOffset();

    if (nCLIHeaderOffset == -1) {
        return;
    }

    pCLIAssembly->setNetHeaderOffset(nCLIHeaderOffset);

    // Resolve the metadata offset here: only the PE knows how to map the RVA.
    XPE_DEF::IMAGE_COR20_HEADER header = _read_IMAGE_COR20_HEADER(nCLIHeaderOffset);

    if ((header.cb == 0x48) && header.MetaData.VirtualAddress && header.MetaData.Size) {
        _MEMORY_MAP memoryMap = getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);

        qint64 nMetaDataOffset = addressToOffset(&memoryMap, memoryMap.nModuleAddress + header.MetaData.VirtualAddress);

        if (nMetaDataOffset != -1) {
            pCLIAssembly->setNetMetaDataOffset(nMetaDataOffset);
        }
    }
}

XBinary::OFFSETSIZE XPE::getNet_MetadataOffsetSize()
{
    OFFSETSIZE osResult = {};
    osResult.nOffset = -1;

    _MEMORY_MAP memoryMap = getMemoryMap();

    qint64 nCLIHeaderOffset = -1;

    if (isNETPresent()) {
        XPE_DEF::IMAGE_DATA_DIRECTORY _idd = getOptionalHeader_DataDirectory(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);

        nCLIHeaderOffset = relAddressToOffset(&memoryMap, _idd.VirtualAddress);
    } else {
        // mb TODO
        // TODO Check!
        nCLIHeaderOffset = addressToOffset(&memoryMap, memoryMap.nModuleAddress + 0x2008);
    }

    if (nCLIHeaderOffset != -1) {
        XPE_DEF::IMAGE_COR20_HEADER header = _read_IMAGE_COR20_HEADER(nCLIHeaderOffset);

        if ((header.cb == 0x48) && header.MetaData.VirtualAddress && header.MetaData.Size) {
            osResult.nOffset = relAddressToOffset(&memoryMap, header.MetaData.VirtualAddress);
            osResult.nSize = header.MetaData.Size;
        }
    }

    return osResult;
}

XPE::CLI_METADATA_HEADER XPE::_read_MetadataHeader(qint64 nOffset)
{
    CLI_METADATA_HEADER result = {};

    result.nSignature = read_uint32(nOffset);
    result.nMajorVersion = read_uint16(nOffset + 4);
    result.nMinorVersion = read_uint16(nOffset + 6);
    result.nReserved = read_uint32(nOffset + 8);
    result.nVersionStringLength = read_uint32(nOffset + 12);
    result.sVersion = read_ansiString(nOffset + 16, result.nVersionStringLength);
    result.nFlags = read_uint16(nOffset + 16 + result.nVersionStringLength);
    result.nStreams = read_uint16(nOffset + 16 + result.nVersionStringLength + 2);

    if (result.sVersion.size() > 20) {
        result.sVersion = "";
    }

    return result;
}

void XPE::setMetadataHeader_Signature(quint32 nValue)
{
    qint64 nOffset = getNet_MetadataOffsetSize().nOffset;

    if (nOffset != -1) {
        write_uint32(nOffset + 0, nValue);
    }
}

void XPE::setMetadataHeader_MajorVersion(quint16 nValue)
{
    qint64 nOffset = getNet_MetadataOffsetSize().nOffset;

    if (nOffset != -1) {
        write_uint16(nOffset + 4, nValue);
    }
}

void XPE::setMetadataHeader_MinorVersion(quint16 nValue)
{
    qint64 nOffset = getNet_MetadataOffsetSize().nOffset;

    if (nOffset != -1) {
        write_uint16(nOffset + 6, nValue);
    }
}

void XPE::setMetadataHeader_Reserved(quint32 nValue)
{
    qint64 nOffset = getNet_MetadataOffsetSize().nOffset;

    if (nOffset != -1) {
        write_uint32(nOffset + 8, nValue);
    }
}

void XPE::setMetadataHeader_VersionStringLength(quint32 nValue)
{
    qint64 nOffset = getNet_MetadataOffsetSize().nOffset;

    if (nOffset != -1) {
        write_uint32(nOffset + 12, nValue);
    }
}

void XPE::setMetadataHeader_Version(const QString &sValue)
{
    QString _sValue = sValue;
    qint64 nOffset = getNet_MetadataOffsetSize().nOffset;

    if (nOffset != -1) {
        quint32 nVersionStringLength = read_uint32(nOffset + 12);

        if (_sValue.size() > (qint32)nVersionStringLength) {
            _sValue.resize(nVersionStringLength);
        }

        write_ansiString(nOffset + 16, _sValue);
    }
}

void XPE::setMetadataHeader_Flags(quint16 nValue)
{
    qint64 nOffset = getNet_MetadataOffsetSize().nOffset;

    if (nOffset != -1) {
        quint32 nVersionStringLength = read_uint32(nOffset + 12);

        write_uint16(nOffset + 16 + nVersionStringLength, nValue);
    }
}

void XPE::setMetadataHeader_Streams(quint16 nValue)
{
    qint64 nOffset = getNet_MetadataOffsetSize().nOffset;

    if (nOffset != -1) {
        quint32 nVersionStringLength = read_uint32(nOffset + 12);

        write_uint16(nOffset + 16 + nVersionStringLength + 2, nValue);
    }
}

bool XPE::isDataDirectoryValid(XPE_DEF::IMAGE_DATA_DIRECTORY *pDataDirectory)
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return isDataDirectoryValid(pDataDirectory, &memoryMap);
}

bool XPE::isDataDirectoryValid(XPE_DEF::IMAGE_DATA_DIRECTORY *pDataDirectory, XBinary::_MEMORY_MAP *pMemoryMap)
{
    if ((pDataDirectory == nullptr) || (pMemoryMap == nullptr) || (pDataDirectory->VirtualAddress == 0) || (pDataDirectory->Size == 0)) {
        return false;
    }

    quint64 nStartRVA = pDataDirectory->VirtualAddress;
    quint64 nEndRVA = nStartRVA + (quint64)pDataDirectory->Size - 1;

    if ((nEndRVA < nStartRVA) || (nEndRVA > 0xFFFFFFFFULL)) {
        return false;
    }

    qint64 nStartOffset = relAddressToOffset(pMemoryMap, (qint64)nStartRVA);
    qint64 nEndOffset = relAddressToOffset(pMemoryMap, (qint64)nEndRVA);

    if ((nStartOffset < 0) || (nEndOffset < nStartOffset) ||
        ((quint64)(nEndOffset - nStartOffset) != ((quint64)pDataDirectory->Size - 1))) {
        return false;
    }

    return checkOffsetSize(nStartOffset, pDataDirectory->Size);
}


quint32 XPE::getNetId()
{
    quint32 nResult = 0;

    if (isNETPresent()) {
        quint32 nTimeDateStamp = getFileHeader_TimeDateStamp();

        if (nTimeDateStamp & 0x80000000) {
            nResult = nTimeDateStamp;
        }
    }

    return nResult;
}


qint32 XPE::getEntryPointSection()
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return getEntryPointSection(&memoryMap);
}

qint32 XPE::getEntryPointSection(_MEMORY_MAP *pMemoryMap)
{
    qint32 nResult = -1;

    XADDR nAddressOfEntryPoint = getOptionalHeader_AddressOfEntryPoint();

    if (nAddressOfEntryPoint) {
        nResult = addressToFileTypeNumber(pMemoryMap, getModuleAddress() + nAddressOfEntryPoint);
    }

    if (nResult > 0) {
        nResult--;
    }

    return nResult;
}

qint32 XPE::getImportSection()
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return getImageDirectoryEntrySection(&memoryMap, XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_IMPORT);
}

qint32 XPE::getExportSection()
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return getImageDirectoryEntrySection(&memoryMap, XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXPORT);
}

qint32 XPE::getTLSSection()
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return getImageDirectoryEntrySection(&memoryMap, XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_TLS);
}

qint32 XPE::getIATSection()
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return getImageDirectoryEntrySection(&memoryMap, XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_IAT);
}

qint32 XPE::getResourcesSection()
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return getImageDirectoryEntrySection(&memoryMap, XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_RESOURCE);
}

qint32 XPE::getRelocsSection()
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return getImageDirectoryEntrySection(&memoryMap, XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_RESOURCE);
}

qint32 XPE::getImageDirectoryEntrySection(_MEMORY_MAP *pMemoryMap, qint32 nImageDirectoryEntry)
{
    qint32 nResult = -1;

    XADDR nAddressOfRecord = getOptionalHeader_DataDirectory(nImageDirectoryEntry).VirtualAddress;

    if (nAddressOfRecord) {
        nResult = addressToFileTypeNumber(pMemoryMap, getModuleAddress() + nAddressOfRecord);
    }

    if (nResult > 0) {
        nResult--;
    }

    return nResult;
}

qint32 XPE::getNormalCodeSection()
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return getNormalCodeSection(&memoryMap);
}

qint32 XPE::getNormalCodeSection(_MEMORY_MAP *pMemoryMap)
{
    qint32 nResult = -1;
    // TODO opimize

    QList<XPE_DEF::IMAGE_SECTION_HEADER> listSections = getSectionHeaders();
    qint32 nNumberOfSections = listSections.count();
    nNumberOfSections = qMin(nNumberOfSections, 2);

    for (qint32 i = 0; i < nNumberOfSections; i++) {
        QString sSectionName = QString::fromLatin1((const char *)listSections.at(i).Name, qstrnlen((const char *)listSections.at(i).Name, 8));
        sSectionName.resize(qMin(sSectionName.length(), XPE_DEF::S_IMAGE_SIZEOF_SHORT_NAME));
        quint32 nSectionCharacteristics = listSections.at(i).Characteristics;
        nSectionCharacteristics &= 0xFF0000FF;

        // .textbss
        // 0x60500060 mingw
        if ((((sSectionName == "CODE") || sSectionName == ".text")) && (nSectionCharacteristics == 0x60000020) && (listSections.at(i).SizeOfRawData)) {
            nResult = addressToFileTypeNumber(pMemoryMap, getModuleAddress() + listSections.at(i).VirtualAddress);
            break;
        }
    }

    if (nResult == -1) {
        if (nNumberOfSections > 0) {
            if (listSections.at(0).SizeOfRawData) {
                nResult = addressToFileTypeNumber(pMemoryMap, getModuleAddress() + listSections.at(0).VirtualAddress);
            }
        }
    }

    if (nResult > 0) {
        nResult--;
    }

    return nResult;
}

qint32 XPE::getNormalDataSection()
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return getNormalDataSection(&memoryMap);
}

qint32 XPE::getNormalDataSection(_MEMORY_MAP *pMemoryMap)
{
    qint32 nResult = -1;
    // TODO opimize

    QList<XPE_DEF::IMAGE_SECTION_HEADER> listSections = getSectionHeaders();
    qint32 nNumberOfSections = listSections.count();

    qint32 nImportSection = getImageDirectoryEntrySection(pMemoryMap, XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_IMPORT);

    for (qint32 i = 1; i < nNumberOfSections; i++) {
        // 0xc0700040 MinGW
        // 0xc0600040 MinGW
        // 0xc0300040 MinGW
        QString sSectionName = QString::fromLatin1((const char *)listSections.at(i).Name, qstrnlen((const char *)listSections.at(i).Name, 8));
        sSectionName.resize(qMin(sSectionName.length(), XPE_DEF::S_IMAGE_SIZEOF_SHORT_NAME));
        quint32 nSectionCharacteristics = listSections.at(i).Characteristics;
        nSectionCharacteristics &= 0xFF0000FF;

        if ((((sSectionName == "DATA") || sSectionName == ".data")) && (nSectionCharacteristics == 0xC0000040) && (listSections.at(i).SizeOfRawData) &&
            (nImportSection != i)) {
            nResult = addressToFileTypeNumber(pMemoryMap, getModuleAddress() + listSections.at(i).VirtualAddress);
            break;
        }
    }

    if (nResult == -1) {
        for (qint32 i = 1; i < nNumberOfSections; i++) {
            if (listSections.at(i).SizeOfRawData && (nImportSection != i) && (listSections.at(i).Characteristics != 0x60000020) &&
                (listSections.at(i).Characteristics != 0x40000040)) {
                nResult = addressToFileTypeNumber(pMemoryMap, getModuleAddress() + listSections.at(i).VirtualAddress);
                break;
            }
        }
    }

    if (nResult > 0) {
        nResult--;
    }

    return nResult;
}

qint32 XPE::getConstDataSection()
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return getConstDataSection(&memoryMap);
}

qint32 XPE::getConstDataSection(_MEMORY_MAP *pMemoryMap)
{
    qint32 nResult = -1;
    // TODO opimize

    QList<XPE_DEF::IMAGE_SECTION_HEADER> listSections = getSectionHeaders();
    qint32 nNumberOfSections = listSections.count();

    for (qint32 i = 1; i < nNumberOfSections; i++) {
        // 0x40700040 MinGW
        // 0x40600040 MinGW
        // 0x40300040 MinGW
        QString sSectionName = QString::fromLatin1((const char *)listSections.at(i).Name, qstrnlen((const char *)listSections.at(i).Name, 8));
        sSectionName.resize(qMin(sSectionName.length(), XPE_DEF::S_IMAGE_SIZEOF_SHORT_NAME));
        quint32 nSectionCharacteristics = listSections.at(i).Characteristics;
        nSectionCharacteristics &= 0xFF0000FF;

        if ((sSectionName == ".rdata") && (nSectionCharacteristics == 0x40000040) && (listSections.at(i).SizeOfRawData)) {
            nResult = addressToFileTypeNumber(pMemoryMap, getModuleAddress() + listSections.at(i).VirtualAddress);
            break;
        }
    }

    if (nResult > 0) {
        nResult--;
    }

    return nResult;
}

bool XPE::rebuildDump(const QString &sResultFile, REBUILD_OPTIONS *pRebuildOptions, PDSTRUCT *pPdStruct)
{
    // TODO rework!
#ifdef QT_DEBUG
    QElapsedTimer timer;
    timer.start();
    qDebug("XPE::rebuildDump");
#endif
    bool bResult = false;

    if (sResultFile != "") {
        quint32 nTotalSize = 0;
        quint32 nHeaderSize = 0;
        QList<quint32> listSectionsSize;
        QList<quint32> listSectionsOffsets;

        quint32 nFileAlignment = getOptionalHeader_FileAlignment();

        quint32 nSectionAlignment = getOptionalHeader_SectionAlignment();

        if (pRebuildOptions->bOptimize) {
            QByteArray baHeader = getHeaders();
            qint32 nNumberOfSections = getFileHeader_NumberOfSections();

            //            if(pRebuildOptions->bClearHeader)
            //            {
            //                nHeaderSize=(qint32)getSectionsTableOffset()+nNumberOfSections*sizeof(XPE_DEF::IMAGE_SECTION_HEADER);
            //            }
            //            else
            //            {
            //                nHeaderSize=(quint32)XBinary::getPhysSize(baHeader.data(),baHeader.size());
            //            }

            nHeaderSize = (quint32)XBinary::getPhysSize(baHeader.data(), baHeader.size());

            for (qint32 i = 0; i < nNumberOfSections; i++) {
                QByteArray baSection = read_array(getSection_VirtualAddress(i), getSection_VirtualSize(i));
                quint32 nSectionSize = (quint32)XBinary::getPhysSize(baSection.data(), baSection.size());
                listSectionsSize.append(nSectionSize);
            }

            nTotalSize += S_ALIGN_UP(nHeaderSize, nFileAlignment);

            for (qint32 i = 0; i < listSectionsSize.size(); i++) {
                listSectionsOffsets.append(nTotalSize);

                if (listSectionsSize.at(i)) {
                    nTotalSize += S_ALIGN_UP(listSectionsSize.at(i), nFileAlignment);
                }
            }
        } else {
            nTotalSize = getSize();
        }
#ifdef QT_DEBUG
        qDebug("XPE::rebuildDump:totalsize: %lld msec", timer.elapsed());
#endif
        QByteArray baBuffer;
        baBuffer.resize(nTotalSize);
        baBuffer.fill(0);
        QBuffer buffer;
        buffer.setBuffer(&baBuffer);

        if (buffer.open(QIODevice::ReadWrite)) {
            XPE bufPE(&buffer, false);

            if (pRebuildOptions->bOptimize) {
                XBinary::copyDeviceMemory(getDevice(), 0, &buffer, 0, nHeaderSize);
                bufPE.setOptionalHeader_SizeOfHeaders(S_ALIGN_UP(nHeaderSize, nFileAlignment));
            } else {
                XBinary::copyDeviceMemory(getDevice(), 0, &buffer, 0, nTotalSize);
            }
#ifdef QT_DEBUG
            qDebug("XPE::rebuildDump:copy: %lld msec", timer.elapsed());
#endif
            qint32 nNumberOfSections = getFileHeader_NumberOfSections();

            for (qint32 i = 0; i < nNumberOfSections; i++) {
                if (pRebuildOptions->bOptimize) {
                    XBinary::copyDeviceMemory(getDevice(), getSection_VirtualAddress(i), &buffer, listSectionsOffsets.at(i), listSectionsSize.at(i));
                    bufPE.setSection_PointerToRawData(i, listSectionsOffsets.at(i));
                    bufPE.setSection_SizeOfRawData(i, S_ALIGN_UP(listSectionsSize.at(i), nFileAlignment));
                } else {
                    quint32 nSectionAddress = getSection_VirtualAddress(i);
                    quint32 nSectionSize = getSection_VirtualSize(i);
                    bufPE.setSection_SizeOfRawData(i, S_ALIGN_UP(nSectionSize, nSectionAlignment));
                    bufPE.setSection_PointerToRawData(i, nSectionAddress);
                }

                bufPE.setSection_Characteristics(i, 0xe0000020);  // !!!
            }
#ifdef QT_DEBUG
            qDebug("XPE::rebuildDump:copysections: %lld msec", timer.elapsed());
#endif
            bResult = true;

            buffer.close();
        }

        QFile file;
        file.setFileName(sResultFile);
        bResult = false;

        if (file.open(QIODevice::ReadWrite)) {
#ifdef QT_DEBUG
            qDebug("XPE::rebuildDump:write:start: %lld msec", timer.elapsed());
#endif
            bool bWriteOk = file.resize(baBuffer.size());

            if (bWriteOk) {
                bWriteOk = (file.write(baBuffer.data(), baBuffer.size()) == baBuffer.size());
            }

            file.close();
#ifdef QT_DEBUG
            qDebug("XPE::rebuildDump:write: %lld msec", timer.elapsed());
#endif
            bResult = bWriteOk && (file.error() == QFile::NoError);
        }
    }

    if (bResult) {
        bResult = false;

        QFile file;
        file.setFileName(sResultFile);

        if (file.open(QIODevice::ReadWrite)) {
            XPE _pe(&file, false);

            if (_pe.isValid()) {
                //                if(pRebuildOptions->bRemoveLastSection)
                //                {
                //                    _pe.removeLastSection();
                //                }
                //            #ifdef QT_DEBUG
                //                qDebug("XPE::rebuildDump:removelastsection:
                //                %lld msec",timer.elapsed());
                //            #endif
                if (!pRebuildOptions->mapPatches.empty()) {
                    _MEMORY_MAP memoryMap = getMemoryMap();

                    QMapIterator<XADDR, quint64> i(pRebuildOptions->mapPatches);

                    while (i.hasNext()) {
                        i.next();

                        XADDR nAddress = i.key();
                        quint64 nValue = i.value();

                        quint64 nOffset = _pe.addressToOffset(&memoryMap, nAddress);

                        if (_pe.is64()) {
                            _pe.write_uint64(nOffset, nValue);
                        } else {
                            _pe.write_uint32(nOffset, (quint32)nValue);
                        }
                    }
                }

#ifdef QT_DEBUG
                qDebug("XPE::rebuildDump:mapPatches: %lld msec", timer.elapsed());
#endif
                if (pRebuildOptions->bSetEntryPoint) {
                    _pe.setOptionalHeader_AddressOfEntryPoint(pRebuildOptions->nEntryPoint);
                }

#ifdef QT_DEBUG
                qDebug("XPE::rebuildDump:setentrypoint: %lld msec", timer.elapsed());
#endif
                if (!pRebuildOptions->mapIAT.isEmpty()) {
                    if (!_pe.addImportSection(&(pRebuildOptions->mapIAT), pPdStruct)) {
                        //                        _errorMessage(tr("Cannot add
                        //                        import section"));
                    }
                }

#ifdef QT_DEBUG
                qDebug("XPE::rebuildDump:addimportsection: %lld msec", timer.elapsed());
#endif
                if (pRebuildOptions->bRenameSections) {
                    qint32 nNumberOfSections = _pe.getFileHeader_NumberOfSections();

                    for (qint32 i = 0; i < nNumberOfSections; i++) {
                        QString sSection = _pe.getSection_NameAsString(i);

                        if (sSection != ".rsrc") {
                            _pe.setSection_NameAsString(i, pRebuildOptions->sSectionName);
                        }
                    }
                }

#ifdef QT_DEBUG
                qDebug("XPE::rebuildDump:renamesections: %lld msec", timer.elapsed());
#endif
                if (pRebuildOptions->listRelocsRVAs.count()) {
                    _pe.addRelocsSection(&(pRebuildOptions->listRelocsRVAs), pPdStruct);
                }
#ifdef QT_DEBUG
                qDebug("XPE::rebuildDump:addrelocssection: %lld msec", timer.elapsed());
#endif
                if (pRebuildOptions->bFixChecksum) {
                    _pe.fixCheckSum();
                }
#ifdef QT_DEBUG
                qDebug("XPE::rebuildDump:fixchecksum: %lld msec", timer.elapsed());
#endif
            }

            bResult = true;

            file.close();
        }
    }
#ifdef QT_DEBUG
    qDebug("XPE::rebuildDump: %lld msec", timer.elapsed());
#endif

    return bResult;
}

bool XPE::rebuildDump(const QString &sInputFile, const QString &sResultFile, REBUILD_OPTIONS *pRebuildOptions, PDSTRUCT *pPdStruct)
{
    // TODO rework!
    bool bResult = false;

    QFile file;
    file.setFileName(sInputFile);

    if (file.open(QIODevice::ReadOnly)) {
        XPE pe(&file, false);

        if (pe.isValid()) {
            bResult = pe.rebuildDump(sResultFile, pRebuildOptions, pPdStruct);
        }

        file.close();
    }

    return bResult;
}

bool XPE::fixCheckSum(const QString &sFileName, bool bIsImage)
{
    bool bResult = false;

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadWrite)) {
        XPE pe(&file, bIsImage);

        if (pe.isValid()) {
            pe.fixCheckSum();
            bResult = true;
        }

        file.close();
    }

    return bResult;
}

bool XPE::fixDump(const QString &sResultFile, const FIXDUMP_OPTIONS &fixDumpOptions, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(sResultFile)
    Q_UNUSED(fixDumpOptions)
    Q_UNUSED(pPdStruct)

    bool bResult = false;

    // TODO

    return bResult;
}

XPE::FIXDUMP_OPTIONS XPE::getFixDumpOptions(PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    _MEMORY_MAP memoryMap = getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);

    FIXDUMP_OPTIONS result = {};

    result.bOptimizeSize = true;
    result.bCleanHeader = false;
    result.bFixSections = true;
    result.bSetFileAlignment = false;
    result.nFileAlignment = getOptionalHeader_FileAlignment();
    result.bSetSectionAlignment = false;
    result.nSectionAlignment = getOptionalHeader_SectionAlignment();
    result.bSetEntryPoint = false;
    result.nEntryPoint = getOptionalHeader_AddressOfEntryPoint();
    result.bSetImageBase = false;
    result.nImageBase = getOptionalHeader_ImageBase();
    result.ddIAT = getIAT(&memoryMap, pPdStruct);

    return result;
}

qint64 XPE::_fixHeadersSize()
{
    quint32 nNumberOfSections = getFileHeader_NumberOfSections();
    qint64 nSectionsTableOffset = getSectionsTableOffset();
    qint64 nHeadersSize = _calculateHeadersSize(nSectionsTableOffset, nNumberOfSections);

    // MB TODO
    setOptionalHeader_SizeOfHeaders(nHeadersSize);

    return nHeadersSize;
}

// qint64 XPE::_getMinSectionOffset()
// {
//     qint64 nResult = -1;

//     _MEMORY_MAP memoryMap = getMemoryMap();

//     qint32 nNumberOfRecords = memoryMap.listRecords.count();

//     for (qint32 i = 0; i < nNumberOfRecords; i++) {
//         if (memoryMap.listRecords.at(i).type == MMT_LOADSEGMENT) {
//             if (nResult == -1) {
//                 nResult = memoryMap.listRecords.at(i).nOffset;
//             } else {
//                 nResult = qMin(nResult, memoryMap.listRecords.at(i).nOffset);
//             }
//         }
//     }

//     return nResult;
// }

void XPE::_fixFileOffsets(qint64 nDelta)
{
    if (nDelta) {
        setOptionalHeader_SizeOfHeaders(getOptionalHeader_SizeOfHeaders() + nDelta);  // TODO mb calculate SizeOfHeaders
        quint32 nNumberOfSections = getFileHeader_NumberOfSections();

        for (quint32 i = 0; i < nNumberOfSections; i++) {
            quint32 nFileOffset = getSection_PointerToRawData(i);
            setSection_PointerToRawData(i, nFileOffset + nDelta);
        }

        // TODO Offset to Cert
    }
}

quint16 XPE::_checkSum(qint64 nStartValue, qint64 nDataSize, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }
    // TODO Check
    // TODO Optimize
    const qint32 BUFFER_SIZE = 0x1000;
    qint32 nSum = (qint32)nStartValue;
    quint32 nTemp = 0;
    qint32 nBufferSize = getBufferSize(pPdStruct);
    char *pBuffer = new char[nBufferSize];
    char *pOffset;

    while ((nDataSize > 0) && (!(pPdStruct->bIsStop))) {
        nTemp = qMin((qint64)BUFFER_SIZE, nDataSize);

        if (!read_array(nStartValue, pBuffer, nTemp)) {
            delete[] pBuffer;

            return 0;
        }

        pOffset = pBuffer;

        for (quint32 i = 0; i < (nTemp + 1) / 2; i++) {
            nSum += *((quint16 *)pOffset);

            if (S_HIWORD(nSum) != 0) {
                nSum = S_LOWORD(nSum) + S_HIWORD(nSum);
            }

            pOffset += 2;
        }

        nDataSize -= nTemp;
        nStartValue += nTemp;
    }

    delete[] pBuffer;

    return (quint16)(S_LOWORD(nSum) + S_HIWORD(nSum));
}

XPE_DEF::IMAGE_RESOURCE_DIRECTORY_ENTRY XPE::read_IMAGE_RESOURCE_DIRECTORY_ENTRY(qint64 nOffset)
{
    XPE_DEF::IMAGE_RESOURCE_DIRECTORY_ENTRY result = {};

    read_array(nOffset, (char *)&result, sizeof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY_ENTRY));

    return result;
}

XPE_DEF::IMAGE_RESOURCE_DIRECTORY XPE::read_IMAGE_RESOURCE_DIRECTORY(qint64 nOffset)
{
    XPE_DEF::IMAGE_RESOURCE_DIRECTORY result = {};

    read_array(nOffset, (char *)&result, sizeof(XPE_DEF::IMAGE_RESOURCE_DIRECTORY));

    return result;
}

XPE_DEF::IMAGE_RESOURCE_DATA_ENTRY XPE::read_IMAGE_RESOURCE_DATA_ENTRY(qint64 nOffset)
{
    XPE_DEF::IMAGE_RESOURCE_DATA_ENTRY result = {};

    read_array(nOffset, (char *)&result, sizeof(XPE_DEF::IMAGE_RESOURCE_DATA_ENTRY));

    return result;
}

XPE::RESOURCES_ID_NAME XPE::getResourcesIDName(qint64 nResourceOffset, quint32 nValue)
{
    RESOURCES_ID_NAME result = {};

    if (nValue & 0x80000000) {
        result.bIsName = true;
        nValue &= 0x7FFFFFFF;
        result.nNameOffset = nValue;
        result.nID = 0;
        quint16 nStringLength = read_uint16(nResourceOffset + nValue);

        nStringLength = qMin((quint16)1024, nStringLength);

        QByteArray baName = read_array(nResourceOffset + nValue + 2, nStringLength * 2);
        result.sName = QString::fromUtf16((quint16 *)(baName.data()), nStringLength);
    } else {
        result.nID = nValue;
        result.sName = "";
        result.nNameOffset = 0;
    }

    return result;
}

QString XPE::resourceIdNameToString(const RESOURCES_ID_NAME &resourceIdName, qint32 nNumber)
{
    QString sResult;

    if (resourceIdName.bIsName) {
        sResult = QString("\"%1\"").arg(resourceIdName.sName);
    } else {
        if (nNumber == 0) {
            QMap<quint64, QString> mapRT = XPE::getResourceTypes();
            QString sType = mapRT.value(resourceIdName.nID);

            if (sType != "") {
                sResult = QString("%1(%2)").arg(sType).arg(QString::number(resourceIdName.nID));
            } else {
                sResult = QString("%1").arg(resourceIdName.nID);
            }
        } else {
            sResult = QString("%1").arg(resourceIdName.nID);
        }
    }

    return sResult;
}

QList<qint64> XPE::getRelocsAsRVAList()
{
    QSet<qint64> stResult;

    // TODO 64
    qint64 nRelocsOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_BASERELOC);
    const XPE_DEF::IMAGE_DATA_DIRECTORY directory =
        getOptionalHeader_DataDirectory(
            XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_BASERELOC);
    const qint64 nFileSize = getSize();
    const quint64 nImageSize =
        getOptionalHeader_SizeOfImage();
    const qint32 nMaxRelocationBlocks = 4096;
    const qint32 nMaxRelocationRecords = 65536;
    const quint32 nExpectedRelocationType = is64() ? 10 : 3;
    _MEMORY_MAP relocationMemoryMap = getMemoryMap();
    bool bPhysicalDirectory = false;
    if (relocationMemoryMap.nModuleAddress
            <= (std::numeric_limits<quint64>::max)()
                   - directory.VirtualAddress) {
        const XADDR nDirectoryAddress =
            relocationMemoryMap.nModuleAddress
            + directory.VirtualAddress;
        bPhysicalDirectory =
            isPhysicalAddressRange(
                &relocationMemoryMap, nDirectoryAddress,
                static_cast<qint64>(directory.Size))
            && addressToOffset(
                   &relocationMemoryMap,
                   nDirectoryAddress)
                   == nRelocsOffset;
    }

    if (nRelocsOffset >= 0
        && !(directory.VirtualAddress & 3U)
        && directory.Size >= sizeof(XPE_DEF::IMAGE_BASE_RELOCATION)
        && directory.VirtualAddress <= nImageSize
        && directory.Size
               <= nImageSize - directory.VirtualAddress
        && nRelocsOffset <= nFileSize
                                 - static_cast<qint64>(
                                     sizeof(XPE_DEF::IMAGE_BASE_RELOCATION))
        && directory.Size <= static_cast<quint64>(nFileSize - nRelocsOffset)
        && bPhysicalDirectory) {
        const qint64 nDirectoryOffset = nRelocsOffset;
        const qint64 nDirectoryEnd =
            nRelocsOffset + static_cast<qint64>(directory.Size);
        qint32 nBlockCount = 0;
        qint32 nRecordCount = 0;
        while (nBlockCount < nMaxRelocationBlocks
               && nRecordCount < nMaxRelocationRecords
               && nRelocsOffset
                      <= nDirectoryEnd
                             - static_cast<qint64>(
                                 sizeof(XPE_DEF::IMAGE_BASE_RELOCATION))) {
            const qint64 nBlockOffset = nRelocsOffset;
            const quint64 nBlockRVA =
                static_cast<quint64>(
                    directory.VirtualAddress)
                + static_cast<quint64>(
                    nBlockOffset - nDirectoryOffset);
            if (nBlockRVA & 3U) {
                break;
            }
            XPE_DEF::IMAGE_BASE_RELOCATION ibr = _readIMAGE_BASE_RELOCATION(nRelocsOffset);

            if (ibr.SizeOfBlock == 0) {
                break;
            }

            if (ibr.VirtualAddress & 0xFFF) {
                break;
            }
            if (ibr.SizeOfBlock
                    < sizeof(XPE_DEF::IMAGE_BASE_RELOCATION)
                || (ibr.SizeOfBlock
                    - sizeof(XPE_DEF::IMAGE_BASE_RELOCATION))
                       % sizeof(quint16)
                       != 0
                || static_cast<quint64>(ibr.SizeOfBlock)
                       > static_cast<quint64>(
                           nDirectoryEnd - nBlockOffset)) {
                break;
            }

            qint32 nNumberOfRecords =
                (ibr.SizeOfBlock
                 - sizeof(XPE_DEF::IMAGE_BASE_RELOCATION))
                / sizeof(quint16);
            nNumberOfRecords =
                qMin(nNumberOfRecords,
                     nMaxRelocationRecords - nRecordCount);
            qint64 nRecordOffset =
                nBlockOffset
                + sizeof(XPE_DEF::IMAGE_BASE_RELOCATION);
            for (qint32 i = 0; i < nNumberOfRecords; i++) {
                quint16 nRecord = read_uint16(nRecordOffset);
                const quint32 nType = nRecord >> 12;
                const quint32 nPageOffset = nRecord & 0x0FFF;

                // This type-erased compatibility API can safely expose only
                // the canonical full-width fixup for the current PE class.
                // Callers needing HIGH/LOW/HIGHADJ or other relocation kinds
                // must use getRelocsHeaders()/getRelocsPositions(), which
                // preserve nType.
                if (nType == nExpectedRelocationType) {
                    stResult.insert(
                        static_cast<qint64>(ibr.VirtualAddress)
                        + nPageOffset);
                }

                nRecordOffset += sizeof(quint16);
            }
            nRecordCount += nNumberOfRecords;
            ++nBlockCount;
            nRelocsOffset =
                nBlockOffset + static_cast<qint64>(ibr.SizeOfBlock);
        }
    }

    return stResult.values();
}

QList<XPE::RELOCS_HEADER> XPE::getRelocsHeaders(PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    QList<RELOCS_HEADER> listResult;

    qint64 nRelocsOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_BASERELOC);
    const XPE_DEF::IMAGE_DATA_DIRECTORY directory =
        getOptionalHeader_DataDirectory(
            XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_BASERELOC);
    const qint64 nFileSize = getSize();
    const quint64 nImageSize =
        getOptionalHeader_SizeOfImage();
    const qint32 nMaxRelocationBlocks = 4096;
    const qint32 nMaxRelocationRecords = 65536;
    _MEMORY_MAP relocationMemoryMap = getMemoryMap();
    bool bPhysicalDirectory = false;
    if (relocationMemoryMap.nModuleAddress
            <= (std::numeric_limits<quint64>::max)()
                   - directory.VirtualAddress) {
        const XADDR nDirectoryAddress =
            relocationMemoryMap.nModuleAddress
            + directory.VirtualAddress;
        bPhysicalDirectory =
            isPhysicalAddressRange(
                &relocationMemoryMap, nDirectoryAddress,
                static_cast<qint64>(directory.Size))
            && addressToOffset(
                   &relocationMemoryMap,
                   nDirectoryAddress)
                   == nRelocsOffset;
    }

    if (nRelocsOffset >= 0
        && !(directory.VirtualAddress & 3U)
        && directory.Size >= sizeof(XPE_DEF::IMAGE_BASE_RELOCATION)
        && directory.VirtualAddress <= nImageSize
        && directory.Size
               <= nImageSize - directory.VirtualAddress
        && nRelocsOffset
               <= nFileSize
                      - static_cast<qint64>(
                          sizeof(XPE_DEF::IMAGE_BASE_RELOCATION))
        && directory.Size
               <= static_cast<quint64>(nFileSize - nRelocsOffset)
        && bPhysicalDirectory) {
        const qint64 nDirectoryOffset = nRelocsOffset;
        const qint64 nDirectoryEnd =
            nRelocsOffset + static_cast<qint64>(directory.Size);
        qint32 nBlockCount = 0;
        qint32 nRecordCount = 0;
        while (!(pPdStruct->bIsStop)
               && nBlockCount < nMaxRelocationBlocks
               && nRecordCount < nMaxRelocationRecords
               && nRelocsOffset
                      <= nDirectoryEnd
                             - static_cast<qint64>(
                                 sizeof(XPE_DEF::IMAGE_BASE_RELOCATION))) {
            RELOCS_HEADER record = {};

            record.nOffset = nRelocsOffset;
            const quint64 nBlockRVA =
                static_cast<quint64>(
                    directory.VirtualAddress)
                + static_cast<quint64>(
                    record.nOffset - nDirectoryOffset);
            if (nBlockRVA & 3U) {
                break;
            }

            record.baseRelocation = _readIMAGE_BASE_RELOCATION(nRelocsOffset);

            if (record.baseRelocation.SizeOfBlock == 0) {
                break;
            }

            if (record.baseRelocation.VirtualAddress & 0xFFF) {
                break;
            }

            if (record.baseRelocation.SizeOfBlock
                    < sizeof(XPE_DEF::IMAGE_BASE_RELOCATION)
                || (record.baseRelocation.SizeOfBlock
                    - sizeof(XPE_DEF::IMAGE_BASE_RELOCATION))
                       % sizeof(quint16)
                       != 0
                || static_cast<quint64>(
                       record.baseRelocation.SizeOfBlock)
                       > static_cast<quint64>(
                           nDirectoryEnd - nRelocsOffset)) {
                break;
            }

            const qint64 nCount =
                (static_cast<qint64>(
                     record.baseRelocation.SizeOfBlock)
                 - static_cast<qint64>(
                     sizeof(XPE_DEF::IMAGE_BASE_RELOCATION)))
                / static_cast<qint64>(sizeof(quint16));
            if (nCount < 0
                || nCount > nMaxRelocationRecords - nRecordCount) {
                break;
            }
            record.nCount = static_cast<qint32>(nCount);

            listResult.append(record);
            nRecordCount += record.nCount;
            ++nBlockCount;
            nRelocsOffset +=
                static_cast<qint64>(
                    record.baseRelocation.SizeOfBlock);
        }
    }

    return listResult;
}

QList<XPE::RELOCS_POSITION> XPE::getRelocsPositions(qint64 nOffset)
{
    QList<RELOCS_POSITION> listResult;

    const qint64 nFileSize = getSize();
    const qint32 nMaxRelocationRecords = 65536;
    if (nOffset < 0
        || nOffset
               > nFileSize
                      - static_cast<qint64>(
                          sizeof(XPE_DEF::IMAGE_BASE_RELOCATION))) {
        return listResult;
    }

    XPE_DEF::IMAGE_BASE_RELOCATION ibr =
        _readIMAGE_BASE_RELOCATION(nOffset);
    if (!ibr.SizeOfBlock
        || (ibr.VirtualAddress & 0xFFF)
        || ibr.SizeOfBlock
               < sizeof(XPE_DEF::IMAGE_BASE_RELOCATION)
        || (ibr.SizeOfBlock
            - sizeof(XPE_DEF::IMAGE_BASE_RELOCATION))
               % sizeof(quint16)
               != 0
        || static_cast<quint64>(ibr.SizeOfBlock)
               > static_cast<quint64>(nFileSize - nOffset)) {
        return listResult;
    }
    _MEMORY_MAP relocationMemoryMap = getMemoryMap();
    const XADDR nBlockAddress =
        offsetToAddress(&relocationMemoryMap, nOffset);
    if (nBlockAddress == (XADDR)-1
        || (nBlockAddress & 3U)
        || !isPhysicalAddressRange(
            &relocationMemoryMap, nBlockAddress,
            static_cast<qint64>(ibr.SizeOfBlock))
        || addressToOffset(
               &relocationMemoryMap, nBlockAddress)
               != nOffset) {
        return listResult;
    }

    const qint64 nCount64 =
        (static_cast<qint64>(ibr.SizeOfBlock)
         - static_cast<qint64>(
             sizeof(XPE_DEF::IMAGE_BASE_RELOCATION)))
        / static_cast<qint64>(sizeof(quint16));
    if (nCount64 < 0 || nCount64 > nMaxRelocationRecords) {
        return listResult;
    }

    nOffset += sizeof(XPE_DEF::IMAGE_BASE_RELOCATION);
    const qint32 nCount = static_cast<qint32>(nCount64);
    for (qint32 i = 0; i < nCount; i++) {
        RELOCS_POSITION record = {};

        quint16 nRecord = read_uint16(nOffset);

        record.nTypeOffset = nRecord;
        record.nAddress = ibr.VirtualAddress + (nRecord & 0x0FFF);
        record.nType = nRecord >> 12;

        listResult.append(record);

        nOffset += sizeof(quint16);
    }

    return listResult;
}

XPE_DEF::IMAGE_BASE_RELOCATION XPE::_readIMAGE_BASE_RELOCATION(qint64 nOffset)
{
    XPE_DEF::IMAGE_BASE_RELOCATION result = {};

    result.VirtualAddress = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_BASE_RELOCATION, VirtualAddress));
    result.SizeOfBlock = read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_BASE_RELOCATION, SizeOfBlock));

    return result;
}

quint32 XPE::getRelocsVirtualAddress(qint64 nOffset)
{
    return read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_BASE_RELOCATION, VirtualAddress));
}

quint32 XPE::getRelocsSizeOfBlock(qint64 nOffset)
{
    return read_uint32(nOffset + offsetof(XPE_DEF::IMAGE_BASE_RELOCATION, SizeOfBlock));
}

void XPE::setRelocsVirtualAddress(qint64 nOffset, quint32 nValue)
{
    write_uint32(nOffset + offsetof(XPE_DEF::IMAGE_BASE_RELOCATION, VirtualAddress), nValue);
}

void XPE::setRelocsSizeOfBlock(qint64 nOffset, quint32 nValue)
{
    write_uint32(nOffset + offsetof(XPE_DEF::IMAGE_BASE_RELOCATION, SizeOfBlock), nValue);
}

bool XPE::addRelocsSection(QList<XADDR> *pList, PDSTRUCT *pPdStruct)
{
    return addRelocsSection(getDevice(), isImage(), pList, pPdStruct);
}

bool XPE::addRelocsSection(QIODevice *pDevice, bool bIsImage, QList<XADDR> *pListRelocs, PDSTRUCT *pPdStruct)
{
    bool bResult = false;

    if ((isResizeEnable(pDevice)) && (pListRelocs->count())) {
        XPE pe(pDevice, bIsImage);

        if (pe.isValid()) {
            // Check valid
            _MEMORY_MAP memoryMap = pe.getMemoryMap();

            QList<XADDR> listRVAs;

            qint32 nNumberOfRelocs = pListRelocs->count();

            for (qint32 i = 0; i < nNumberOfRelocs; i++) {
                if (pe.isAddressValid(&memoryMap, pListRelocs->at(i) + memoryMap.nModuleAddress)) {
                    listRVAs.append(pListRelocs->at(i));
                }
            }

            QByteArray baRelocs = relocsAsRVAListToByteArray(&listRVAs, pe.is64());

            XPE_DEF::IMAGE_SECTION_HEADER ish = {};

            ish.Characteristics = 0x42000040;
            QString sSectionName = ".reloc";
            XBinary::_copyMemory((char *)&ish.Name, sSectionName.toLatin1().data(), qMin(XPE_DEF::S_IMAGE_SIZEOF_SHORT_NAME, sSectionName.length()));

            bResult = addSection(pDevice, bIsImage, &ish, baRelocs.data(), baRelocs.size(), pPdStruct);

            if (bResult) {
                XPE_DEF::IMAGE_DATA_DIRECTORY dd = {};

                dd.VirtualAddress = ish.VirtualAddress;
                dd.Size = ish.Misc.VirtualSize;

                pe.setOptionalHeader_DataDirectory(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_BASERELOC, &dd);

                bResult = true;
            }
        }
    }

    return bResult;
}

bool XPE::addRelocsSection(const QString &sFileName, bool bIsImage, QList<XADDR> *pListRelocs, PDSTRUCT *pPdStruct)
{
    bool bResult = false;

    QFile file(sFileName);

    if (file.open(QIODevice::ReadWrite)) {
        bResult = addRelocsSection(&file, bIsImage, pListRelocs, pPdStruct);

        file.close();
    }

    return bResult;
}

QByteArray XPE::relocsAsRVAListToByteArray(QList<XADDR> *pListRelocs, bool bIs64)
{
    QByteArray baResult;
    // GetHeaders
    // pList must be sorted!

    XADDR nBaseAddress = -1;
    quint32 nSize = 0;

    qint32 nNumberOfRelocs = pListRelocs->count();

    for (qint32 i = 0; i < nNumberOfRelocs; i++) {
        XADDR _nBaseAddress = S_ALIGN_DOWN(pListRelocs->at(i), 0x1000);

        if (nBaseAddress != _nBaseAddress) {
            nBaseAddress = _nBaseAddress;
            nSize = S_ALIGN_UP(nSize, 4);
            nSize += sizeof(XPE_DEF::IMAGE_BASE_RELOCATION);
        }

        nSize += 2;
    }

    nSize = S_ALIGN_UP(nSize, 4);

    baResult.resize(nSize);

    nBaseAddress = -1;
    quint32 nOffset = 0;
    char *pData = baResult.data();
    char *pVirtualAddress = 0;
    char *pSizeOfBlock = 0;
    quint32 nCurrentBlockSize = 0;

    nNumberOfRelocs = pListRelocs->count();

    for (qint32 i = 0; i < nNumberOfRelocs; i++) {
        XADDR _nBaseAddress = S_ALIGN_DOWN(pListRelocs->at(i), 0x1000);

        if (nBaseAddress != _nBaseAddress) {
            nBaseAddress = _nBaseAddress;
            quint32 _nOffset = S_ALIGN_UP(nOffset, 4);

            if (nOffset != _nOffset) {
                nCurrentBlockSize += 2;
                XBinary::_write_uint32(pSizeOfBlock, nCurrentBlockSize);
                XBinary::_write_uint16(pData + nOffset, 0);
                nOffset = _nOffset;
            }

            pVirtualAddress = pData + nOffset + offsetof(XPE_DEF::IMAGE_BASE_RELOCATION, VirtualAddress);
            pSizeOfBlock = pData + nOffset + offsetof(XPE_DEF::IMAGE_BASE_RELOCATION, SizeOfBlock);
            XBinary::_write_uint32(pVirtualAddress, nBaseAddress);
            nCurrentBlockSize = sizeof(XPE_DEF::IMAGE_BASE_RELOCATION);
            XBinary::_write_uint32(pSizeOfBlock, nCurrentBlockSize);

            nOffset += sizeof(XPE_DEF::IMAGE_BASE_RELOCATION);
        }

        nCurrentBlockSize += 2;
        XBinary::_write_uint32(pSizeOfBlock, nCurrentBlockSize);

        if (!bIs64) {
            XBinary::_write_uint16(pData + nOffset, pListRelocs->at(i) - nBaseAddress + 0x3000);
        } else {
            XBinary::_write_uint16(pData + nOffset, pListRelocs->at(i) - nBaseAddress + 0xA000);
        }

        nOffset += 2;
    }

    quint32 _nOffset = S_ALIGN_UP(nOffset, 4);

    if (nOffset != _nOffset) {
        nCurrentBlockSize += 2;
        XBinary::_write_uint32(pSizeOfBlock, nCurrentBlockSize);
        XBinary::_write_uint16(pData + nOffset, 0);
    }

    return baResult;
}

bool XPE::isResourcesPresent()
{
    return isOptionalHeader_DataDirectoryPresent(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_RESOURCE);
}

bool XPE::isSymbolsPresent()
{
    return (getFileHeader_PointerToSymbolTable() != 0) && (getFileHeader_NumberOfSymbols() != 0);
}

QVector<XBinary::XIMPORT_STRUCT> XPE::getImportStructs()
{
    QVector<XIMPORT_STRUCT> listResult;

    QList<IMPORT_HEADER> listImports = getImports();

    qint32 nNumberOfHeaders = listImports.count();

    for (qint32 i = 0; i < nNumberOfHeaders; i++) {
        const IMPORT_HEADER &header = listImports.at(i);

        qint32 nNumberOfPositions = header.listPositions.count();

        for (qint32 j = 0; j < nNumberOfPositions; j++) {
            const IMPORT_POSITION &position = header.listPositions.at(j);

            XIMPORT_STRUCT record = {};
            record.nOffset = position.nThunkOffset;
            record.nSize = 0;
            record.nAddress = position.nThunkValue;
            record.sLibrary = header.sName;
            record.sFunction = position.sFunction;
            record.nOrdinal = (qint32)position.nOrdinal;

            listResult.append(record);
        }
    }

    return listResult;
}

QVector<XBinary::XEXPORT_STRUCT> XPE::getExportStructs()
{
    QVector<XEXPORT_STRUCT> listResult;

    EXPORT_HEADER exportHeader = getExport();

    qint32 nNumberOfPositions = exportHeader.listPositions.count();

    for (qint32 i = 0; i < nNumberOfPositions; i++) {
        const EXPORT_POSITION &position = exportHeader.listPositions.at(i);

        XEXPORT_STRUCT record = {};
        record.nOffset = 0;
        record.nSize = 0;
        record.nAddress = position.nAddress;
        record.sFunction = position.sFunctionName;
        record.nOrdinal = position.nOrdinal;

        listResult.append(record);
    }

    return listResult;
}

QVector<XBinary::XSYMBOL_STRUCT> XPE::getSymbolStructs()
{
    QVector<XSYMBOL_STRUCT> listResult;

    quint32 nSymbolTableOffset = getFileHeader_PointerToSymbolTable();
    quint32 nNumberOfSymbols = getFileHeader_NumberOfSymbols();

    const qint64 IMAGE_SYMBOL_SIZE = 18;
    const quint32 MAX_SYMBOLS = 200000;  // Guard against corrupted NumberOfSymbols on untrusted files

    if ((nSymbolTableOffset != 0) && (nNumberOfSymbols != 0) && (nNumberOfSymbols <= MAX_SYMBOLS)) {
        qint64 nStringTableOffset = (qint64)nSymbolTableOffset + (qint64)nNumberOfSymbols * IMAGE_SYMBOL_SIZE;

        quint32 nIndex = 0;

        while (nIndex < nNumberOfSymbols) {
            qint64 nEntryOffset = (qint64)nSymbolTableOffset + (qint64)nIndex * IMAGE_SYMBOL_SIZE;

            quint32 nShortOrZero = read_uint32(nEntryOffset);
            quint8 nNumberOfAuxSymbols = read_uint8(nEntryOffset + 17);

            QString sName;

            if (nShortOrZero != 0) {
                sName = read_ansiString(nEntryOffset, 8);
            } else {
                quint32 nNameOffset = read_uint32(nEntryOffset + 4);
                sName = read_ansiString(nStringTableOffset + nNameOffset);
            }

            XSYMBOL_STRUCT record = {};
            record.nOffset = nEntryOffset;
            record.nSize = IMAGE_SYMBOL_SIZE;
            record.nAddress = read_uint32(nEntryOffset + 8);
            record.sName = sName;
            record.symbolType = SYMBOL_TYPE_LABEL;

            listResult.append(record);

            nIndex += (1 + nNumberOfAuxSymbols);
        }
    }

    return listResult;
}

QVector<XBinary::XRESOURCE_STRUCT> XPE::getResourceStructs()
{
    QVector<XRESOURCE_STRUCT> listResult;

    QList<RESOURCE_RECORD> listResources = getResources(10000);

    qint32 nNumberOfRecords = listResources.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        const RESOURCE_RECORD &resource = listResources.at(i);

        XRESOURCE_STRUCT record = {};
        record.nOffset = resource.nOffset;
        record.nSize = resource.nSize;
        record.nAddress = resource.nAddress;
        record.nType = resource.irin[0].bIsName ? 0 : resource.irin[0].nID;
        record.nID = resource.irin[1].bIsName ? 0 : resource.irin[1].nID;
        record.sName = resource.irin[1].bIsName ? resource.irin[1].sName : resource.irin[0].sName;

        listResult.append(record);
    }

    return listResult;
}

bool XPE::isRelocsPresent()
{
    return isOptionalHeader_DataDirectoryPresent(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_BASERELOC);
}

bool XPE::isDebugPresent()
{
    return isOptionalHeader_DataDirectoryPresent(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_DEBUG);
}

bool XPE::isTLSPresent()
{
    return isOptionalHeader_DataDirectoryPresent(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_TLS);
}

bool XPE::isSignPresent()
{
    return isOptionalHeader_DataDirectoryPresent(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_SECURITY);
}

bool XPE::isExceptionPresent()
{
    return isOptionalHeader_DataDirectoryPresent(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_EXCEPTION);
}

bool XPE::isLoadConfigPresent()
{
    return isOptionalHeader_DataDirectoryPresent(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG);
}

bool XPE::isBoundImportPresent()
{
    return isOptionalHeader_DataDirectoryPresent(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT);
}

bool XPE::isDelayImportPresent()
{
    return isOptionalHeader_DataDirectoryPresent(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT);
}

qint64 XPE::getTLSHeaderOffset()
{
    return getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_TLS);
}

qint64 XPE::getTLSHeaderSize()
{
    qint64 nResult = 0;

    if (is64()) {
        nResult = sizeof(XPE_DEF::S_IMAGE_TLS_DIRECTORY64);
    } else {
        nResult = sizeof(XPE_DEF::S_IMAGE_TLS_DIRECTORY32);
    }

    return nResult;
}

XPE_DEF::S_IMAGE_TLS_DIRECTORY32 XPE::getTLSDirectory32()
{
    XPE_DEF::S_IMAGE_TLS_DIRECTORY32 result = {};

    qint64 nTLSOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_TLS);

    if (nTLSOffset != -1) {
        // TODO read function!!!
        read_array(nTLSOffset, (char *)&result, sizeof(XPE_DEF::S_IMAGE_TLS_DIRECTORY32));
    }

    return result;
}

XPE_DEF::S_IMAGE_TLS_DIRECTORY64 XPE::getTLSDirectory64()
{
    XPE_DEF::S_IMAGE_TLS_DIRECTORY64 result = {};

    qint64 nTLSOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_TLS);

    if (nTLSOffset != -1) {
        // TODO read function!!!
        read_array(nTLSOffset, (char *)&result, sizeof(XPE_DEF::S_IMAGE_TLS_DIRECTORY64));
    }

    return result;
}

quint64 XPE::getTLS_StartAddressOfRawData()
{
    quint64 nResult = 0;

    qint64 nTLSOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_TLS);

    if (nTLSOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nTLSOffset + offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY64, StartAddressOfRawData));
        } else {
            nResult = read_uint32(nTLSOffset + offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY32, StartAddressOfRawData));
        }
    }

    return nResult;
}

quint64 XPE::getTLS_EndAddressOfRawData()
{
    quint64 nResult = 0;

    qint64 nTLSOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_TLS);

    if (nTLSOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nTLSOffset + offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY64, EndAddressOfRawData));
        } else {
            nResult = read_uint32(nTLSOffset + offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY32, EndAddressOfRawData));
        }
    }

    return nResult;
}

quint64 XPE::getTLS_AddressOfIndex()
{
    quint64 nResult = 0;

    qint64 nTLSOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_TLS);

    if (nTLSOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nTLSOffset + offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY64, AddressOfIndex));
        } else {
            nResult = read_uint32(nTLSOffset + offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY32, AddressOfIndex));
        }
    }

    return nResult;
}

quint64 XPE::getTLS_AddressOfCallBacks()
{
    quint64 nResult = 0;

    qint64 nTLSOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_TLS);

    if (nTLSOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nTLSOffset + offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY64, AddressOfCallBacks));
        } else {
            nResult = read_uint32(nTLSOffset + offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY32, AddressOfCallBacks));
        }
    }

    return nResult;
}

quint32 XPE::getTLS_SizeOfZeroFill()
{
    quint32 nResult = 0;

    qint64 nTLSOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_TLS);

    if (nTLSOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nTLSOffset + offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY64, SizeOfZeroFill));
        } else {
            nResult = read_uint32(nTLSOffset + offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY32, SizeOfZeroFill));
        }
    }

    return nResult;
}

quint32 XPE::getTLS_Characteristics()
{
    quint32 nResult = 0;

    qint64 nTLSOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_TLS);

    if (nTLSOffset != -1) {
        if (is64()) {
            nResult = read_uint64(nTLSOffset + offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY64, Characteristics));
        } else {
            nResult = read_uint32(nTLSOffset + offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY32, Characteristics));
        }
    }

    return nResult;
}

void XPE::setTLS_StartAddressOfRawData(quint64 nValue)
{
    qint64 nTLSOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_TLS);

    if (nTLSOffset != -1) {
        if (is64()) {
            write_uint64(nTLSOffset + offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY64, StartAddressOfRawData), nValue);
        } else {
            write_uint32(nTLSOffset + offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY32, StartAddressOfRawData), nValue);
        }
    }
}

void XPE::setTLS_EndAddressOfRawData(quint64 nValue)
{
    qint64 nTLSOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_TLS);

    if (nTLSOffset != -1) {
        if (is64()) {
            write_uint64(nTLSOffset + offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY64, EndAddressOfRawData), nValue);
        } else {
            write_uint32(nTLSOffset + offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY32, EndAddressOfRawData), nValue);
        }
    }
}

void XPE::setTLS_AddressOfIndex(quint64 nValue)
{
    qint64 nTLSOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_TLS);

    if (nTLSOffset != -1) {
        if (is64()) {
            write_uint64(nTLSOffset + offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY64, AddressOfIndex), nValue);
        } else {
            write_uint32(nTLSOffset + offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY32, AddressOfIndex), nValue);
        }
    }
}

void XPE::setTLS_AddressOfCallBacks(quint64 nValue)
{
    qint64 nTLSOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_TLS);

    if (nTLSOffset != -1) {
        if (is64()) {
            write_uint64(nTLSOffset + offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY64, AddressOfCallBacks), nValue);
        } else {
            write_uint32(nTLSOffset + offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY32, AddressOfCallBacks), nValue);
        }
    }
}

void XPE::setTLS_SizeOfZeroFill(quint32 nValue)
{
    qint64 nTLSOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_TLS);

    if (nTLSOffset != -1) {
        if (is64()) {
            write_uint64(nTLSOffset + offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY64, SizeOfZeroFill), nValue);
        } else {
            write_uint32(nTLSOffset + offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY32, SizeOfZeroFill), nValue);
        }
    }
}

void XPE::setTLS_Characteristics(quint32 nValue)
{
    qint64 nTLSOffset = getDataDirectoryOffset(XPE_DEF::S_IMAGE_DIRECTORY_ENTRY_TLS);

    if (nTLSOffset != -1) {
        if (is64()) {
            write_uint64(nTLSOffset + offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY64, Characteristics), nValue);
        } else {
            write_uint32(nTLSOffset + offsetof(XPE_DEF::S_IMAGE_TLS_DIRECTORY32, Characteristics), nValue);
        }
    }
}

QList<XADDR> XPE::getTLS_CallbacksList()  // TODO limit
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return getTLS_CallbacksList(&memoryMap);
}

QList<XADDR> XPE::getTLS_CallbacksList(XBinary::_MEMORY_MAP *pMemoryMap, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    QList<XADDR> listResult;

    qint64 nOffset = addressToOffset(pMemoryMap, getTLS_AddressOfCallBacks());

    if (nOffset != -1) {
        for (qint32 i = 0; (i < 100) && (!(pPdStruct->bIsStop)); i++)  // TODO const or parameter
        {
            XADDR nAddress = 0;

            if (is64()) {
                nAddress = read_uint64(nOffset);

                nOffset += 8;
            } else {
                nAddress = read_uint32(nOffset);

                nOffset += 4;
            }

            if (nAddress && isAddressValid(pMemoryMap, nAddress)) {
                listResult.append(nAddress);
            } else {
                break;
            }
        }
    }

    return listResult;
}

bool XPE::isTLSCallbacksPresent()
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return isTLSCallbacksPresent(&memoryMap);
}

bool XPE::isTLSCallbacksPresent(XBinary::_MEMORY_MAP *pMemoryMap)
{
    return getTLS_CallbacksList(pMemoryMap).count();
}

XPE::TLS_HEADER XPE::getTLSHeader()
{
    TLS_HEADER result = {};

    if (isTLSPresent()) {
        if (is64()) {
            XPE_DEF::S_IMAGE_TLS_DIRECTORY64 tls64 = getTLSDirectory64();

            result.AddressOfCallBacks = tls64.AddressOfCallBacks;
            result.AddressOfIndex = tls64.AddressOfIndex;
            result.Characteristics = tls64.Characteristics;
            result.EndAddressOfRawData = tls64.EndAddressOfRawData;
            result.SizeOfZeroFill = tls64.SizeOfZeroFill;
            result.StartAddressOfRawData = tls64.StartAddressOfRawData;
        } else {
            XPE_DEF::S_IMAGE_TLS_DIRECTORY32 tls32 = getTLSDirectory32();

            result.AddressOfCallBacks = tls32.AddressOfCallBacks;
            result.AddressOfIndex = tls32.AddressOfIndex;
            result.Characteristics = tls32.Characteristics;
            result.EndAddressOfRawData = tls32.EndAddressOfRawData;
            result.SizeOfZeroFill = tls32.SizeOfZeroFill;
            result.StartAddressOfRawData = tls32.StartAddressOfRawData;
        }
    }

    return result;
}

QMap<quint64, QString> XPE::getImageNtHeadersSignatures()
{
    return XBinary::XIDSTRING_createMapPrefix(_TABLE_XPE_ImageNtHeadersSignatures, sizeof(_TABLE_XPE_ImageNtHeadersSignatures) / sizeof(XBinary::XIDSTRING),
                                              PREFIX_ImageNtHeadersSignatures);
}

QMap<quint64, QString> XPE::getImageNtHeadersSignaturesS()
{
    return XBinary::XIDSTRING_createMap(_TABLE_XPE_ImageNtHeadersSignatures, sizeof(_TABLE_XPE_ImageNtHeadersSignatures) / sizeof(XBinary::XIDSTRING));
}

QMap<quint64, QString> XPE::getImageMagics()
{
    return XBinary::XIDSTRING_createMapPrefix(_TABLE_XPE_ImageMagics, sizeof(_TABLE_XPE_ImageMagics) / sizeof(XBinary::XIDSTRING), PREFIX_ImageMagics);
}

QMap<quint64, QString> XPE::getImageMagicsS()
{
    return XBinary::XIDSTRING_createMap(_TABLE_XPE_ImageMagics, sizeof(_TABLE_XPE_ImageMagics) / sizeof(XBinary::XIDSTRING));
}

QMap<quint64, QString> XPE::getImageFileHeaderMachines()
{
    return XBinary::XIDSTRING_createMapPrefix(_TABLE_XPE_ImageFileHeaderMachines, sizeof(_TABLE_XPE_ImageFileHeaderMachines) / sizeof(XBinary::XIDSTRING),
                                              PREFIX_ImageFileHeaderMachines);
}

QMap<quint64, QString> XPE::getImageFileHeaderMachinesS()
{
    QMap<quint64, QString> mapResult =
        XBinary::XIDSTRING_createMap(_TABLE_XPE_ImageFileHeaderMachines, sizeof(_TABLE_XPE_ImageFileHeaderMachines) / sizeof(XBinary::XIDSTRING));

    return mapResult;
}

QMap<quint64, QString> XPE::getImageFileHeaderCharacteristics()
{
    return XBinary::XIDSTRING_createMapPrefix(_TABLE_XPE_ImageFileHeaderCharacteristics, sizeof(_TABLE_XPE_ImageFileHeaderCharacteristics) / sizeof(XBinary::XIDSTRING),
                                              PREFIX_ImageFileHeaderCharacteristics);
}

QMap<quint64, QString> XPE::getImageFileHeaderCharacteristicsS()
{
    return XBinary::XIDSTRING_createMap(_TABLE_XPE_ImageFileHeaderCharacteristics, sizeof(_TABLE_XPE_ImageFileHeaderCharacteristics) / sizeof(XBinary::XIDSTRING));
}

QMap<quint64, QString> XPE::getImageOptionalHeaderMagic()
{
    return XBinary::XIDSTRING_createMapPrefix(_TABLE_XPE_ImageOptionalHeaderMagic, sizeof(_TABLE_XPE_ImageOptionalHeaderMagic) / sizeof(XBinary::XIDSTRING),
                                              PREFIX_ImageOptionalHeaderMagic);
}

QMap<quint64, QString> XPE::getImageOptionalHeaderMagicS()
{
    return XBinary::XIDSTRING_createMap(_TABLE_XPE_ImageOptionalHeaderMagic, sizeof(_TABLE_XPE_ImageOptionalHeaderMagic) / sizeof(XBinary::XIDSTRING));
}

QMap<quint64, QString> XPE::getImageOptionalHeaderSubsystem()
{
    return XBinary::XIDSTRING_createMapPrefix(_TABLE_XPE_ImageOptionalHeaderSubsystem, sizeof(_TABLE_XPE_ImageOptionalHeaderSubsystem) / sizeof(XBinary::XIDSTRING),
                                              PREFIX_ImageOptionalHeaderSubsystem);
}

QMap<quint64, QString> XPE::getImageOptionalHeaderSubsystemS()
{
    QMap<quint64, QString> mapResult =
        XBinary::XIDSTRING_createMap(_TABLE_XPE_ImageOptionalHeaderSubsystem, sizeof(_TABLE_XPE_ImageOptionalHeaderSubsystem) / sizeof(XBinary::XIDSTRING));

    mapResult[0] = tr("Unknown");

    return mapResult;
}

QMap<quint64, QString> XPE::getImageOptionalHeaderDllCharacteristics()
{
    return XBinary::XIDSTRING_createMapPrefix(_TABLE_XPE_ImageOptionalHeaderDllCharacteristics,
                                              sizeof(_TABLE_XPE_ImageOptionalHeaderDllCharacteristics) / sizeof(XBinary::XIDSTRING),
                                              PREFIX_ImageOptionalHeaderDllCharacteristics);
}

QMap<quint64, QString> XPE::getImageOptionalHeaderDllCharacteristicsS()
{
    return XBinary::XIDSTRING_createMap(_TABLE_XPE_ImageOptionalHeaderDllCharacteristics,
                                        sizeof(_TABLE_XPE_ImageOptionalHeaderDllCharacteristics) / sizeof(XBinary::XIDSTRING));
}

QMap<quint64, QString> XPE::getImageOptionalHeaderDataDirectory()
{
    return XBinary::XIDSTRING_createMapPrefix(_TABLE_XPE_ImageOptionalHeaderDataDirectory,
                                              sizeof(_TABLE_XPE_ImageOptionalHeaderDataDirectory) / sizeof(XBinary::XIDSTRING), PREFIX_ImageOptionalHeaderDataDirectory);
}

QMap<quint64, QString> XPE::getImageOptionalHeaderDataDirectoryS()
{
    return XBinary::XIDSTRING_createMap(_TABLE_XPE_ImageOptionalHeaderDataDirectory, sizeof(_TABLE_XPE_ImageOptionalHeaderDataDirectory) / sizeof(XBinary::XIDSTRING));
}

QMap<quint64, QString> XPE::getImageSectionHeaderFlags()
{
    return XBinary::XIDSTRING_createMapPrefix(_TABLE_XPE_ImageSectionHeaderFlags, sizeof(_TABLE_XPE_ImageSectionHeaderFlags) / sizeof(XBinary::XIDSTRING),
                                              PREFIX_ImageSectionHeaderFlags);
}

QMap<quint64, QString> XPE::getImageSectionHeaderFlagsS()
{
    return XBinary::XIDSTRING_createMap(_TABLE_XPE_ImageSectionHeaderFlags, sizeof(_TABLE_XPE_ImageSectionHeaderFlags) / sizeof(XBinary::XIDSTRING));
}

QMap<quint64, QString> XPE::getImageSectionHeaderAligns()
{
    return XBinary::XIDSTRING_createMapPrefix(_TABLE_XPE_ImageSectionHeaderAligns, sizeof(_TABLE_XPE_ImageSectionHeaderAligns) / sizeof(XBinary::XIDSTRING),
                                              PREFIX_ImageSectionHeaderAligns);
}

QMap<quint64, QString> XPE::getImageSectionHeaderAlignsS()
{
    return XBinary::XIDSTRING_createMap(_TABLE_XPE_ImageSectionHeaderAligns, sizeof(_TABLE_XPE_ImageSectionHeaderAligns) / sizeof(XBinary::XIDSTRING));
}

QMap<quint64, QString> XPE::getResourceTypes()
{
    return XBinary::XIDSTRING_createMapPrefix(_TABLE_XPE_ResourceTypes, sizeof(_TABLE_XPE_ResourceTypes) / sizeof(XBinary::XIDSTRING), PREFIX_ResourceTypes);
}

QMap<quint64, QString> XPE::getResourceTypesS()
{
    return XBinary::XIDSTRING_createMap(_TABLE_XPE_ResourceTypes, sizeof(_TABLE_XPE_ResourceTypes) / sizeof(XBinary::XIDSTRING));
}

QMap<quint64, QString> XPE::getImageRelBased()
{
    return XBinary::XIDSTRING_createMapPrefix(_TABLE_XPE_ImageRelBased, sizeof(_TABLE_XPE_ImageRelBased) / sizeof(XBinary::XIDSTRING), PREFIX_ImageRelBased);
}

QMap<quint64, QString> XPE::getImageRelBasedS()
{
    return XBinary::XIDSTRING_createMap(_TABLE_XPE_ImageRelBased, sizeof(_TABLE_XPE_ImageRelBased) / sizeof(XBinary::XIDSTRING));
}

QMap<quint64, QString> XPE::getComImageFlags()
{
    return XBinary::XIDSTRING_createMapPrefix(_TABLE_XPE_ComImageFlags, sizeof(_TABLE_XPE_ComImageFlags) / sizeof(XBinary::XIDSTRING), PREFIX_ComImageFlags);
}

QMap<quint64, QString> XPE::getComImageFlagsS()
{
    return XBinary::XIDSTRING_createMap(_TABLE_XPE_ComImageFlags, sizeof(_TABLE_XPE_ComImageFlags) / sizeof(XBinary::XIDSTRING));
}

QMap<quint64, QString> XPE::getDebugTypes()
{
    return XBinary::XIDSTRING_createMapPrefix(_TABLE_XPE_DebugTypes, sizeof(_TABLE_XPE_DebugTypes) / sizeof(XBinary::XIDSTRING), PREFIX_DebugTypes);
}

QMap<quint64, QString> XPE::getDebugTypesS()
{
    QMap<quint64, QString> mapResult = XBinary::XIDSTRING_createMap(_TABLE_XPE_DebugTypes, sizeof(_TABLE_XPE_DebugTypes) / sizeof(XBinary::XIDSTRING));

    // mapResult[0] = tr("Unknown");

    return mapResult;
}

QMap<quint64, QString> XPE::getNetMetadataFlags()
{
    return XBinary::XIDSTRING_createMapPrefix(_TABLE_XPE_NetMetadataFlags, sizeof(_TABLE_XPE_NetMetadataFlags) / sizeof(XBinary::XIDSTRING), PREFIX_NetMetadataFlags);
}

QMap<quint64, QString> XPE::getNetMetadataFlagsS()
{
    return XBinary::XIDSTRING_createMap(_TABLE_XPE_NetMetadataFlags, sizeof(_TABLE_XPE_NetMetadataFlags) / sizeof(XBinary::XIDSTRING));
}

QMap<quint64, QString> XPE::getOperatingSystemVersions(OSNAME osName)
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0x00000000, tr("Unknown"));

    // https://learn.microsoft.com/en-us/windows/win32/sysinfo/operating-system-version?redirectedfrom=MSDN
    if (osName == OSNAME_WINDOWS) {
        mapResult.insert(0x0003000A, QString("Windows NT 3.1"));
        mapResult.insert(0x00030032, QString("Windows NT 3.5"));
        mapResult.insert(0x00030033, QString("Windows NT 3.51"));
        mapResult.insert(0x00040000, QString("Windows 95"));
        mapResult.insert(0x00040001, QString("Windows 98"));
        mapResult.insert(0x00040009, QString("Windows Millenium"));
        mapResult.insert(0x00050000, QString("Windows 2000"));
        mapResult.insert(0x00050001, QString("Windows XP"));
        mapResult.insert(0x00050002, QString("Windows Server 2003"));
        mapResult.insert(0x00060000, QString("Windows Vista"));
        mapResult.insert(0x00060001, QString("Windows 7"));
        mapResult.insert(0x00060002, QString("Windows 8"));
        mapResult.insert(0x00060003, QString("Windows 8.1"));
        mapResult.insert(0x000A0000, QString("Windows 10"));
    }

    return mapResult;
}

QMap<quint64, QString> XPE::getOperatingSystemVersionsS(OSNAME osName)
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0x00000000, tr("Unknown"));

    if (osName == OSNAME_WINDOWS) {
        mapResult.insert(0x0003000A, QString("NT 3.1"));
        mapResult.insert(0x00030032, QString("NT 3.5"));
        mapResult.insert(0x00030033, QString("NT 3.51"));
        mapResult.insert(0x00040000, QString("95"));
        mapResult.insert(0x00040001, QString("98"));
        mapResult.insert(0x00040009, QString("Millenium"));
        mapResult.insert(0x00050000, QString("2000"));
        mapResult.insert(0x00050001, QString("XP"));
        mapResult.insert(0x00050002, QString("Server 2003"));
        mapResult.insert(0x00060000, QString("Vista"));
        mapResult.insert(0x00060001, QString("7"));
        mapResult.insert(0x00060002, QString("8"));
        mapResult.insert(0x00060003, QString("8.1"));
        mapResult.insert(0x000A0000, QString("10"));
    }

    return mapResult;
}

QMap<quint64, QString> XPE::getMajorOperatingSystemVersion()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0x0003, QString("Windows 3.X"));
    mapResult.insert(0x0004, QString("Windows 4.X"));
    mapResult.insert(0x0005, QString("Windows 5.X"));
    mapResult.insert(0x0006, QString("Windows 6.X"));
    mapResult.insert(0x000A, QString("Windows 10.X"));

    return mapResult;
}

QMap<quint64, QString> XPE::getMajorOperatingSystemVersionS()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0x0003, QString("3.X"));
    mapResult.insert(0x0004, QString("4.X"));
    mapResult.insert(0x0005, QString("5.X"));
    mapResult.insert(0x0006, QString("6.X"));
    mapResult.insert(0x000A, QString("10.X"));

    return mapResult;
}

QMap<quint64, QString> XPE::getMinorOperatingSystemVersion(quint16 nMajorOperatingSystemVersion)
{
    QMap<quint64, QString> mapResult;

    if (nMajorOperatingSystemVersion == 3) {
        mapResult.insert(0x000A, QString("Windows NT 3.1"));
        mapResult.insert(0x0032, QString("Windows NT 3.5"));
        mapResult.insert(0x0033, QString("Windows NT 3.51"));
    } else if (nMajorOperatingSystemVersion == 4) {
        mapResult.insert(0x0000, QString("Windows 95"));
        mapResult.insert(0x0001, QString("Windows 98"));
        mapResult.insert(0x0009, QString("Windows Millenium"));
    } else if (nMajorOperatingSystemVersion == 5) {
        mapResult.insert(0x0000, QString("Windows 2000"));
        mapResult.insert(0x0001, QString("Windows XP"));
        mapResult.insert(0x0002, QString("Windows Server 2003"));
    } else if (nMajorOperatingSystemVersion == 6) {
        mapResult.insert(0x0000, QString("Windows Vista"));
        mapResult.insert(0x0001, QString("Windows 7"));
        mapResult.insert(0x0002, QString("Windows 8"));
        mapResult.insert(0x0003, QString("Windows 8.1"));
    } else if (nMajorOperatingSystemVersion == 10) {
        mapResult.insert(0x0000, QString("Windows 10"));
    }

    return mapResult;
}

QMap<quint64, QString> XPE::getMinorOperatingSystemVersionS(quint16 nMajorOperatingSystemVersion)
{
    QMap<quint64, QString> mapResult;

    if (nMajorOperatingSystemVersion == 3) {
        mapResult.insert(0x000A, QString("NT 3.1"));
        mapResult.insert(0x0032, QString("NT 3.5"));
        mapResult.insert(0x0033, QString("NT 3.51"));
    } else if (nMajorOperatingSystemVersion == 4) {
        mapResult.insert(0x0000, QString("95"));
        mapResult.insert(0x0001, QString("98"));
        mapResult.insert(0x0009, QString("Millenium"));
    } else if (nMajorOperatingSystemVersion == 5) {
        mapResult.insert(0x0000, QString("2000"));
        mapResult.insert(0x0001, QString("XP"));
        mapResult.insert(0x0002, QString("Server 2003"));
    } else if (nMajorOperatingSystemVersion == 6) {
        mapResult.insert(0x0000, QString("Vista"));
        mapResult.insert(0x0001, QString("7"));
        mapResult.insert(0x0002, QString("8"));
        mapResult.insert(0x0003, QString("8.1"));
    } else if (nMajorOperatingSystemVersion == 10) {
        mapResult.insert(0x0000, QString("10"));
    }

    return mapResult;
}

QMap<quint64, QString> XPE::getResourcesFixedFileInfoSignatures()
{
    return XBinary::XIDSTRING_createMap(_TABLE_XPE_ResourcesFixedFileInfoSignatures, sizeof(_TABLE_XPE_ResourcesFixedFileInfoSignatures) / sizeof(XBinary::XIDSTRING));
}

QMap<quint64, QString> XPE::getResourcesFixedFileInfoSignaturesS()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0xFEEF04BD, QString("Signature"));

    return mapResult;
}

QMap<quint64, QString> XPE::getResourcesFixedFileInfoFileFlags()
{
    return XBinary::XIDSTRING_createMapPrefix(_TABLE_XPE_ResourcesFixedFileInfoFileFlags, sizeof(_TABLE_XPE_ResourcesFixedFileInfoFileFlags) / sizeof(XBinary::XIDSTRING),
                                              PREFIX_ResourcesFixedFileInfoFileFlags);
}

QMap<quint64, QString> XPE::getResourcesFixedFileInfoFileFlagsS()
{
    return XBinary::XIDSTRING_createMap(_TABLE_XPE_ResourcesFixedFileInfoFileFlags, sizeof(_TABLE_XPE_ResourcesFixedFileInfoFileFlags) / sizeof(XBinary::XIDSTRING));
}

QMap<quint64, QString> XPE::getResourcesFixedFileInfoFileOses()
{
    return XBinary::XIDSTRING_createMapPrefix(_TABLE_XPE_ResourcesFixedFileInfoFileOses, sizeof(_TABLE_XPE_ResourcesFixedFileInfoFileOses) / sizeof(XBinary::XIDSTRING),
                                              PREFIX_ResourcesFixedFileInfoFileOses);
}

QMap<quint64, QString> XPE::getResourcesFixedFileInfoFileOsesS()
{
    return XBinary::XIDSTRING_createMap(_TABLE_XPE_ResourcesFixedFileInfoFileOses, sizeof(_TABLE_XPE_ResourcesFixedFileInfoFileOses) / sizeof(XBinary::XIDSTRING));
}

QMap<quint64, QString> XPE::getResourcesFixedFileInfoFileTypes()
{
    return XBinary::XIDSTRING_createMapPrefix(_TABLE_XPE_ResourcesFixedFileInfoFileTypes, sizeof(_TABLE_XPE_ResourcesFixedFileInfoFileTypes) / sizeof(XBinary::XIDSTRING),
                                              PREFIX_ResourcesFixedFileInfoFileTypes);
}

QMap<quint64, QString> XPE::getResourcesFixedFileInfoFileTypesS()
{
    return XBinary::XIDSTRING_createMap(_TABLE_XPE_ResourcesFixedFileInfoFileTypes, sizeof(_TABLE_XPE_ResourcesFixedFileInfoFileTypes) / sizeof(XBinary::XIDSTRING));
}

bool XPE::handleInternalInfo(PDSTRUCT *pPdStruct)
{
    bool bResult = true;

    if (!isInternalInfoHandled()) {
        bResult = XMSDOS::handleInternalInfo(pPdStruct);

        if (bResult) {
            static_cast<XMSDOS::INTERNAL_INFO &>(m_internalInfo) =
                *static_cast<XMSDOS::INTERNAL_INFO *>(XMSDOS::getInternalInfo(pPdStruct));
            setIsInternalInfoHandled(true);
        }
    }

    return bResult;
}

void *XPE::getInternalInfo(PDSTRUCT *pPdStruct)
{
    handleInternalInfo(pPdStruct);

    return &m_internalInfo;
}

void XPE::setInternalInfo(void *pInternalInfo)
{
    if (pInternalInfo) {
        m_internalInfo = *static_cast<INTERNAL_INFO *>(pInternalInfo);
        XMSDOS::setInternalInfo(static_cast<XMSDOS::INTERNAL_INFO *>(&m_internalInfo));
        setIsInternalInfoHandled(true);
    } else {
        m_internalInfo = INTERNAL_INFO();
        XMSDOS::setInternalInfo(nullptr);
        setIsInternalInfoHandled(false);
    }
}
