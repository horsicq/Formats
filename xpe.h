// copyright (c) 2017-2020 hors<horsicq@gmail.com>
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
#ifndef XPE_H
#define XPE_H

#include "xmsdos.h"
#include "xpe_def.h"

class XPE : public XMSDOS
{
    Q_OBJECT

public:
    struct SECTION_RECORD
    {
        QString sName;
        qint64 nOffset;
        qint64 nRVA;
        qint64 nSize;
        qint64 nCharacteristics;
    };

    struct SECTIONRVA_RECORD
    {
        qint64 nRVA;
        qint64 nSize;
        qint64 nCharacteristics;
    };

    struct IMPORT_RECORD
    {
        qint64 nOffset;
        qint64 nRVA;
        QString sLibrary;
        QString sFunction;
    };

    struct IMPORT_POSITION
    {
        qint64 nThunkRVA;
        qint64 nThunkOffset;
        quint64 nThunkValue;
        QString sName;
        quint16 nHint;
        qint64 nOrdinal;
        QString sFunction;
    };

    struct IMPORT_HEADER
    {
        QString sName;
        quint32 nFirstThunk; // For patch only!
        QList<IMPORT_POSITION> listPositions;
    };

    struct DELAYIMPORT_POSITION
    {
        qint64 nNameThunkRVA;
        qint64 nNameThunkOffset;
        quint64 nNameThunkValue;
        QString sName;
        quint16 nHint;
        qint64 nOrdinal;
        QString sFunction;
        qint64 nAddressThunkRVA;
        qint64 nAddressThunkOffset;
        quint64 nAddressThunkValue;
        qint64 nBoundThunkRVA;
        qint64 nBoundThunkOffset;
        quint64 nBoundThunkValue;
    };

    struct EXPORT_RECORD
    {
        quint32 nOrdinal;
        QString sFunctionName;
        qint64 nLibraryBase;
        QString sLibraryName;
    };

    struct EXPORT_POSITION
    {
        quint16 nOrdinal;
        quint32 nRVA;
        qint64 nAddress;
        quint32 nNameRVA;
        QString sFunctionName;
    };

    struct EXPORT_HEADER
    {
        XPE_DEF::IMAGE_EXPORT_DIRECTORY directory;
        QString sName;
        QList<EXPORT_POSITION> listPositions;
    };

    struct TLS_HEADER
    {
        quint64 StartAddressOfRawData;
        quint64 EndAddressOfRawData;
        quint64 AddressOfIndex;
        quint64 AddressOfCallBacks;
        quint32 SizeOfZeroFill;
        quint32 Characteristics;
    };

    struct RESOURCES_ID_NAME
    {
        bool bIsName;
        quint32 nID;
        QString sName;
        quint32 nNameOffset;
    };

    struct RESOURCE_RECORD
    {
        RESOURCES_ID_NAME irin[3];
        qint64 nIRDEOffset;
        qint64 nAddress;
        qint64 nRVA;
        qint64 nOffset;
        qint64 nSize;
    };

    struct RESOURCE_POSITION
    {
        qint64 nOffset;
        bool bIsValid;
        quint32 nLevel;
        bool bIsDataDirectory;
        RESOURCES_ID_NAME rin;
        XPE_DEF::IMAGE_RESOURCE_DIRECTORY directory;
        XPE_DEF::IMAGE_RESOURCE_DIRECTORY_ENTRY dirEntry;
        XPE_DEF::IMAGE_RESOURCE_DATA_ENTRY dataEntry;
        qint64 nDataAddress;
        qint64 nDataOffset;
        QList<RESOURCE_POSITION> listPositions;
    };

    struct RESOURCE_HEADER
    {
        qint64 nOffset;
        XPE_DEF::IMAGE_RESOURCE_DIRECTORY directory;
        QList<RESOURCE_POSITION> listPositions;
    };

    struct RELOCS_POSITION
    {
        qint16 nTypeOffset;
        quint32 nType;
        qint64 nAddress;
    };

    struct RELOCS_HEADER
    {
        qint64 nOffset;
        XPE_DEF::IMAGE_BASE_RELOCATION ibr;
        qint32 nCount;
    };

    struct DUMP_OPTIONS
    {
        quint32 nImageBase;
        quint32 nAddressOfEntryPoint;
        //        bool bClearHeader;
        //        bool bCheckImport;
        //        QMap<qint64,QString> mapImport;
        //        QMap<qint64,quint64> mapInitImportOffsets;
        //        bool bIs64;
        //        bool bSaveIAT;
    };

    struct RESOURCE_VERSION
    {
        qint64 nFixedFileInfoOffset;
        XPE_DEF::S_tagVS_FIXEDFILEINFO fileInfo;
        QList<QString> listRecords; // TODO rename
        // TODO VarFileInfo
    };

    struct CLI_METADATA_HEADER
    {
        quint32 nSignature;
        quint16 nMajorVersion;
        quint16 nMinorVersion;
        quint32 nReserved;
        quint32 nVersionStringLength;
        QString sVersion;
        quint16 nFlags;
        quint16 nStreams;
    };

    struct CLI_METADATA_STREAM
    {
        qint64 nOffset;
        qint64 nSize;
        QString sName;
    };

    struct CLI_METADATA
    {
        CLI_METADATA_HEADER header;
        QList<CLI_METADATA_STREAM> listStreams;
        qint64 nTablesHeaderOffset;
        qint64 nTablesSize;
        quint32 nTables_Reserved1;
        quint8 cTables_MajorVersion;
        quint8 cTables_MinorVersion;
        quint8 cTables_HeapOffsetSizes;
        quint8 cTables_Reserved2;
        quint64 nTables_Valid;
        quint64 nTables_Sorted;
        quint32 nTables_Valid_NumberOfRows;
        quint32 Tables_TablesNumberOfIndexes[64]; // TODO const
        qint64 Tables_TablesOffsets[64]; // TODO const
        qint64 Tables_TablesSizes[64]; // TODO const
        qint64 nStringsOffset;
        qint64 nStringsSize;
        qint64 nUSOffset;
        qint64 nUSSize;
        qint64 nBlobOffset;
        qint64 nBlobSize;
        qint64 nGUIDOffset;
        qint64 nGUIDSize;
        qint64 nEntryPoint;
        qint64 nEntryPointSize;
        QList<QString> listAnsiStrings;
        QList<QString> listUnicodeStrings;
    };

    struct CLI_INFO
    {
        bool bValid;
        bool bHidden;
        qint64 nHeaderOffset;
        XPE_DEF::IMAGE_COR20_HEADER header;
        qint64 nMetaDataOffset;
        CLI_METADATA cliMetadata;
    };

    struct IMAGE_IMPORT_DESCRIPTOR_EX
    {
        union
        {
            quint32 Characteristics;
            quint32 OriginalFirstThunk;
        };
        quint32 TimeDateStamp;
        quint32 ForwarderChain;
        quint32 Name;
        quint32 FirstThunk;
        QString sLibrary; // Extra
    };

    struct BOUND_IMPORT_POSITION
    {
        XPE_DEF::S_IMAGE_BOUND_IMPORT_DESCRIPTOR descriptor;
        QString sName;
    };

    enum TYPE
    {
        // mb TODO CEDLL
        TYPE_UNKNOWN=0,
        TYPE_GUI,
        TYPE_CONSOLE,
        TYPE_DLL,
        TYPE_DRIVER,
        TYPE_EFIBOOT,
        TYPE_EFI,
        TYPE_EFIRUNTIMEDRIVER,
        TYPE_XBOX,
        TYPE_OS2,
        TYPE_POSIX,
        TYPE_CE
        // TODO more from subsystems
    };

    explicit XPE(QIODevice *pDevice=nullptr,bool bIsImage=false,qint64 nImageBase=-1);
    virtual bool isValid();

    virtual MODE getMode();
    virtual QString getArch();
    virtual bool isBigEndian();

    virtual int getType();
    virtual QString typeIdToString(int nType);

    qint64 getNtHeadersOffset();
    quint32 getNtHeaders_Signature();
    void setNtHeaders_Signature(quint32 nValue);
    qint64 getFileHeaderOffset();
    qint64 getFileHeaderSize();

    XPE_DEF::S_IMAGE_FILE_HEADER getFileHeader();
    void setFileHeader(XPE_DEF::S_IMAGE_FILE_HEADER *pFileHeader);

    quint16 getFileHeader_Machine();
    quint16 getFileHeader_NumberOfSections();
    quint32 getFileHeader_TimeDateStamp();
    quint32 getFileHeader_PointerToSymbolTable();
    quint32 getFileHeader_NumberOfSymbols();
    quint16 getFileHeader_SizeOfOptionalHeader();
    quint16 getFileHeader_Characteristics();

    void setFileHeader_Machine(quint16 nValue);
    void setFileHeader_NumberOfSections(quint16 nValue);
    void setFileHeader_TimeDateStamp(quint32 nValue);
    void setFileHeader_PointerToSymbolTable(quint32 nValue);
    void setFileHeader_NumberOfSymbols(quint32 nValue);
    void setFileHeader_SizeOfOptionalHeader(quint16 nValue);
    void setFileHeader_Characteristics(quint16 nValue);

    qint64 getOptionalHeaderOffset();
    qint64 getOptionalHeaderSize();

    XPE_DEF::IMAGE_OPTIONAL_HEADER32 getOptionalHeader32();
    XPE_DEF::IMAGE_OPTIONAL_HEADER64 getOptionalHeader64();

    void setOptionalHeader32(XPE_DEF::IMAGE_OPTIONAL_HEADER32 *pOptionalHeader32);
    void setOptionalHeader64(XPE_DEF::IMAGE_OPTIONAL_HEADER64 *pOptionalHeader64);

    XPE_DEF::IMAGE_OPTIONAL_HEADER32S getOptionalHeader32S();
    XPE_DEF::IMAGE_OPTIONAL_HEADER64S getOptionalHeader64S();

    void setOptionalHeader32S(XPE_DEF::IMAGE_OPTIONAL_HEADER32S *pOptionalHeader32S);
    void setOptionalHeader64S(XPE_DEF::IMAGE_OPTIONAL_HEADER64S *pOptionalHeader64S);

    quint16 getOptionalHeader_Magic();
    quint8 getOptionalHeader_MajorLinkerVersion();
    quint8 getOptionalHeader_MinorLinkerVersion();
    quint32 getOptionalHeader_SizeOfCode();
    quint32 getOptionalHeader_SizeOfInitializedData();
    quint32 getOptionalHeader_SizeOfUninitializedData();
    quint32 getOptionalHeader_AddressOfEntryPoint();
    quint32 getOptionalHeader_BaseOfCode();
    quint32 getOptionalHeader_BaseOfData();
    quint64 getOptionalHeader_ImageBase();
    quint32 getOptionalHeader_SectionAlignment();
    quint32 getOptionalHeader_FileAlignment();
    quint16 getOptionalHeader_MajorOperatingSystemVersion();
    quint16 getOptionalHeader_MinorOperatingSystemVersion();
    quint16 getOptionalHeader_MajorImageVersion();
    quint16 getOptionalHeader_MinorImageVersion();
    quint16 getOptionalHeader_MajorSubsystemVersion();
    quint16 getOptionalHeader_MinorSubsystemVersion();
    quint32 getOptionalHeader_Win32VersionValue();
    quint32 getOptionalHeader_SizeOfImage();
    quint32 getOptionalHeader_SizeOfHeaders();
    quint32 getOptionalHeader_CheckSum();
    quint16 getOptionalHeader_Subsystem();
    quint16 getOptionalHeader_DllCharacteristics();
    qint64 getOptionalHeader_SizeOfStackReserve();
    qint64 getOptionalHeader_SizeOfStackCommit();
    qint64 getOptionalHeader_SizeOfHeapReserve();
    qint64 getOptionalHeader_SizeOfHeapCommit();
    quint32 getOptionalHeader_LoaderFlags();
    quint32 getOptionalHeader_NumberOfRvaAndSizes();

    void setOptionalHeader_Magic(quint16 nValue);
    void setOptionalHeader_MajorLinkerVersion(quint8 nValue);
    void setOptionalHeader_MinorLinkerVersion(quint8 nValue);
    void setOptionalHeader_SizeOfCode(quint32 nValue);
    void setOptionalHeader_SizeOfInitializedData(quint32 nValue);
    void setOptionalHeader_SizeOfUninitializedData(quint32 nValue);
    void setOptionalHeader_AddressOfEntryPoint(quint32 nValue);
    void setOptionalHeader_BaseOfCode(quint32 nValue);
    void setOptionalHeader_BaseOfData(quint32 nValue);
    void setOptionalHeader_ImageBase(quint64 nValue);
    void setOptionalHeader_SectionAlignment(quint32 nValue);
    void setOptionalHeader_FileAlignment(quint32 nValue);
    void setOptionalHeader_MajorOperatingSystemVersion(quint16 nValue);
    void setOptionalHeader_MinorOperatingSystemVersion(quint16 nValue);
    void setOptionalHeader_MajorImageVersion(quint16 nValue);
    void setOptionalHeader_MinorImageVersion(quint16 nValue);
    void setOptionalHeader_MajorSubsystemVersion(quint16 nValue);
    void setOptionalHeader_MinorSubsystemVersion(quint16 nValue);
    void setOptionalHeader_Win32VersionValue(quint32 nValue);
    void setOptionalHeader_SizeOfImage(quint32 nValue);
    void setOptionalHeader_SizeOfHeaders(quint32 nValue);
    void setOptionalHeader_CheckSum(quint32 nValue);
    void setOptionalHeader_Subsystem(quint16 nValue);
    void setOptionalHeader_DllCharacteristics(quint16 nValue);
    void setOptionalHeader_SizeOfStackReserve(quint64 nValue);
    void setOptionalHeader_SizeOfStackCommit(quint64 nValue);
    void setOptionalHeader_SizeOfHeapReserve(quint64 nValue);
    void setOptionalHeader_SizeOfHeapCommit(quint64 nValue);
    void setOptionalHeader_LoaderFlags(quint32 nValue);
    void setOptionalHeader_NumberOfRvaAndSizes(quint32 nValue);

    XPE_DEF::IMAGE_DATA_DIRECTORY read_IMAGE_DATA_DIRECTORY(qint64 nOffset);
    void write_IMAGE_DATA_DIRECTORY(qint64 nOffset,XPE_DEF::IMAGE_DATA_DIRECTORY *pDataDirectory);

    XPE_DEF::IMAGE_DATA_DIRECTORY getOptionalHeader_DataDirectory(quint32 nNumber);
    void setOptionalHeader_DataDirectory(quint32 nNumber,XPE_DEF::IMAGE_DATA_DIRECTORY *pDataDirectory);
    void setOptionalHeader_DataDirectory_VirtualAddress(quint32 nNumber,quint32 nValue);
    void setOptionalHeader_DataDirectory_Size(quint32 nNumber,quint32 nValue);

    void clearOptionalHeader_DataDirectory(quint32 nNumber);
    bool isOptionalHeader_DataDirectoryPresent(quint32 nNumber);

    QList<XPE_DEF::IMAGE_DATA_DIRECTORY> getDirectories();
    void setDirectories(QList<XPE_DEF::IMAGE_DATA_DIRECTORY> *pListDirectories);

    qint64 getDataDirectoryOffset(quint32 nNumber);
    qint64 getDataDirectoryOffset(_MEMORY_MAP *pMemoryMap,quint32 nNumber);
    QByteArray getDataDirectory(quint32 nNumber);
    QByteArray getDataDirectory(_MEMORY_MAP *pMemoryMap,quint32 nNumber);

    qint64 getSectionsTableOffset();
    qint64 getSectionHeaderOffset(quint32 nNumber);
    qint64 getSectionHeaderSize();

    bool isSectionsTablePresent();

    XPE_DEF::IMAGE_SECTION_HEADER getSectionHeader(quint32 nNumber);
    void setSectionHeader(quint32 nNumber,XPE_DEF::IMAGE_SECTION_HEADER *pSectionHeader);

    QList<XPE_DEF::IMAGE_SECTION_HEADER> getSectionHeaders();
    // TODO with __getSectionOffsetAndSize
    static QList<SECTION_RECORD> getSectionRecords(QList<XPE_DEF::IMAGE_SECTION_HEADER> *pListSectionHeaders,bool bIsImage);
    static QList<QString> getSectionNames(QList<XPE::SECTION_RECORD> *pListSectionRecords);

    QList<SECTIONRVA_RECORD> getSectionRVARecords();

    QString getSection_NameAsString(quint32 nNumber);
    quint32 getSection_VirtualSize(quint32 nNumber);
    quint32 getSection_VirtualAddress(quint32 nNumber);
    quint32 getSection_SizeOfRawData(quint32 nNumber);
    quint32 getSection_PointerToRawData(quint32 nNumber);
    quint32 getSection_PointerToRelocations(quint32 nNumber);
    quint32 getSection_PointerToLinenumbers(quint32 nNumber);
    quint16 getSection_NumberOfRelocations(quint32 nNumber);
    quint16 getSection_NumberOfLinenumbers(quint32 nNumber);
    quint32 getSection_Characteristics(quint32 nNumber);

    void setSection_NameAsString(quint32 nNumber,QString sName);
    void setSection_VirtualSize(quint32 nNumber,quint32 nValue);
    void setSection_VirtualAddress(quint32 nNumber,quint32 nValue);
    void setSection_SizeOfRawData(quint32 nNumber,quint32 nValue);
    void setSection_PointerToRawData(quint32 nNumber,quint32 nValue);
    void setSection_PointerToRelocations(quint32 nNumber,quint32 nValue);
    void setSection_PointerToLinenumbers(quint32 nNumber,quint32 nValue);
    void setSection_NumberOfRelocations(quint32 nNumber,quint16 nValue);
    void setSection_NumberOfLinenumbers(quint32 nNumber,quint16 nValue);
    void setSection_Characteristics(quint32 nNumber,quint32 nValue);

    QString getSection_NameAsString(quint32 nNumber,QList<QString> *pListSectionNames);
    quint32 getSection_VirtualSize(quint32 nNumber,QList<XPE_DEF::IMAGE_SECTION_HEADER> *pListSectionHeaders);
    quint32 getSection_VirtualAddress(quint32 nNumber,QList<XPE_DEF::IMAGE_SECTION_HEADER> *pListSectionHeaders);
    quint32 getSection_SizeOfRawData(quint32 nNumber,QList<XPE_DEF::IMAGE_SECTION_HEADER> *pListSectionHeaders);
    quint32 getSection_PointerToRawData(quint32 nNumber,QList<XPE_DEF::IMAGE_SECTION_HEADER> *pListSectionHeaders);
    quint32 getSection_PointerToRelocations(quint32 nNumber,QList<XPE_DEF::IMAGE_SECTION_HEADER> *pListSectionHeaders);
    quint32 getSection_PointerToLinenumbers(quint32 nNumber,QList<XPE_DEF::IMAGE_SECTION_HEADER> *pListSectionHeaders);
    quint16 getSection_NumberOfRelocations(quint32 nNumber,QList<XPE_DEF::IMAGE_SECTION_HEADER> *pListSectionHeaders);
    quint16 getSection_NumberOfLinenumbers(quint32 nNumber,QList<XPE_DEF::IMAGE_SECTION_HEADER> *pListSectionHeaders);
    quint32 getSection_Characteristics(quint32 nNumber,QList<XPE_DEF::IMAGE_SECTION_HEADER> *pListSectionHeaders);

    bool isSectionNamePresent(QString sSectionName);
    static bool isSectionNamePresent(QString sSectionName,QList<XPE_DEF::IMAGE_SECTION_HEADER> *pListSections);
    static XPE_DEF::IMAGE_SECTION_HEADER getSectionByName(QString sSectionName,QList<XPE_DEF::IMAGE_SECTION_HEADER> *pListSections);

    qint32 getSectionNumber(QString sSectionName);
    static qint32 getSectionNumber(QString sSectionName,QList<XPE_DEF::IMAGE_SECTION_HEADER> *pListSections);

    bool isImportPresent();

    QList<IMPORT_RECORD> getImportRecords();
    QList<IMPORT_RECORD> getImportRecords(_MEMORY_MAP *pMemoryMap);

    quint64 getImportHash64(_MEMORY_MAP *pMemoryMap);
    quint32 getImportHash32(_MEMORY_MAP *pMemoryMap);

    qint64 getImportDescriptorOffset(quint32 nNumber);
    qint64 getImportDescriptorSize();

    QList<XPE_DEF::IMAGE_IMPORT_DESCRIPTOR> getImportDescriptors();
    QList<XPE_DEF::IMAGE_IMPORT_DESCRIPTOR> getImportDescriptors(_MEMORY_MAP *pMemoryMap);
    QList<IMAGE_IMPORT_DESCRIPTOR_EX> getImportDescriptorsEx();
    QList<IMAGE_IMPORT_DESCRIPTOR_EX> getImportDescriptorsEx(_MEMORY_MAP *pMemoryMap);

    XPE_DEF::IMAGE_IMPORT_DESCRIPTOR getImportDescriptor(quint32 nNumber);
    void setImportDescriptor(quint32 nNumber,XPE_DEF::IMAGE_IMPORT_DESCRIPTOR *pImportDescriptor);

    void setImportDescriptor_OriginalFirstThunk(quint32 nNumber,quint32 nValue);
    void setImportDescriptor_TimeDateStamp(quint32 nNumber,quint32 nValue);
    void setImportDescriptor_ForwarderChain(quint32 nNumber,quint32 nValue);
    void setImportDescriptor_Name(quint32 nNumber,quint32 nValue);
    void setImportDescriptor_FirstThunk(quint32 nNumber,quint32 nValue);

    QList<IMPORT_HEADER> getImports();
    QList<IMPORT_HEADER> getImports(_MEMORY_MAP *pMemoryMap);

    QList<IMPORT_POSITION> _getImportPositions(XBinary::_MEMORY_MAP *pMemoryMap,qint64 nThunksRVA,qint64 nRVA);
    QList<IMPORT_POSITION> getImportPositions(int nIndex);

    QList<quint32> getImportPositionHashes(_MEMORY_MAP *pMemoryMap);

    bool isImportLibraryPresentI(QString sLibrary);
    static bool isImportLibraryPresentI(QString sLibrary,QList<IMPORT_HEADER> *pListImport);

    bool isImportFunctionPresentI(QString sLibrary,QString sFunction);
    static bool isImportFunctionPresentI(QString sLibrary,QString sFunction,QList<IMPORT_HEADER> *pListImport);

    bool setImports(QList<IMPORT_HEADER> *pListHeaders);
    bool setImports(QIODevice *pDevice,bool bIsImage,QList<IMPORT_HEADER> *pListImportHeaders);
    bool setImports(QString sFileName,bool bIsImage,QList<IMPORT_HEADER> *pListHeaders);

    QString getImportFunctionName(quint32 nImport,quint32 nFunctionNumber,QList<IMPORT_HEADER> *pListHeaders);

    RESOURCE_HEADER getResourceHeader();
    RESOURCE_HEADER getResourceHeader(_MEMORY_MAP *pMemoryMap);
    QList<RESOURCE_RECORD> getResources();
    QList<RESOURCE_RECORD> getResources(_MEMORY_MAP *pMemoryMap);

    static RESOURCE_RECORD getResourceRecord(quint32 nID1,quint32 nID2,QList<RESOURCE_RECORD> *pListResourceRecords);
    static RESOURCE_RECORD getResourceRecord(quint32 nID1,QString sName2,QList<RESOURCE_RECORD> *pListResourceRecords);
    static RESOURCE_RECORD getResourceRecord(QString sName1,quint32 nID2,QList<RESOURCE_RECORD> *pListResourceRecords);
    static RESOURCE_RECORD getResourceRecord(QString sName1,QString sName2,QList<RESOURCE_RECORD> *pListResourceRecords);

    static bool isResourcePresent(quint32 nID1,quint32 nID2,QList<RESOURCE_RECORD> *pListResourceRecords);
    static bool isResourcePresent(quint32 nID1,QString sName2,QList<RESOURCE_RECORD> *pListResourceRecords);
    static bool isResourcePresent(QString sName1,quint32 nID2,QList<RESOURCE_RECORD> *pListResourceRecords);
    static bool isResourcePresent(QString sName1,QString sName2,QList<RESOURCE_RECORD> *pListResourceRecords);

    bool isResourceManifestPresent();
    bool isResourceManifestPresent(QList<XPE::RESOURCE_RECORD> *pListResourceRecords);

    QString getResourceManifest();
    QString getResourceManifest(QList<XPE::RESOURCE_RECORD> *pListResourceRecords);

    bool isResourceVersionPresent();
    bool isResourceVersionPresent(QList<XPE::RESOURCE_RECORD> *pListResourceRecords);

    RESOURCE_VERSION getResourceVersion();
    XPE_DEF::S_VS_VERSION_INFO readVS_VERSION_INFO(qint64 nOffset);

    RESOURCE_VERSION getResourceVersion(QList<XPE::RESOURCE_RECORD> *pListResourceRecords);

    QString getFileVersion();
    QString getFileVersion(RESOURCE_VERSION *pResourceVersion);

    void setFixedFileInfo_dwSignature(quint32 nValue);
    void setFixedFileInfo_dwStrucVersion(quint32 nValue);
    void setFixedFileInfo_dwFileVersionMS(quint32 nValue);
    void setFixedFileInfo_dwFileVersionLS(quint32 nValue);
    void setFixedFileInfo_dwProductVersionMS(quint32 nValue);
    void setFixedFileInfo_dwProductVersionLS(quint32 nValue);
    void setFixedFileInfo_dwFileFlagsMask(quint32 nValue);
    void setFixedFileInfo_dwFileFlags(quint32 nValue);
    void setFixedFileInfo_dwFileOS(quint32 nValue);
    void setFixedFileInfo_dwFileType(quint32 nValue);
    void setFixedFileInfo_dwFileSubtype(quint32 nValue);
    void setFixedFileInfo_dwFileDateMS(quint32 nValue);
    void setFixedFileInfo_dwFileDateLS(quint32 nValue);

    QString getResourceVersionValue(QString sKey);
    static QString getResourceVersionValue(QString sKey,XPE::RESOURCE_VERSION *pResourceVersion);

    quint32 getResourceIdByNumber(quint32 nNumber);
    quint32 getResourceIdByNumber(quint32 nNumber,QList<XPE::RESOURCE_RECORD> *pListResourceRecords);
    QString getResourceNameByNumber(quint32 nNumber);
    QString getResourceNameByNumber(quint32 nNumber,QList<XPE::RESOURCE_RECORD> *pListResourceRecords);
    qint64 getResourceOffsetByNumber(quint32 nNumber);
    qint64 getResourceOffsetByNumber(quint32 nNumber,QList<XPE::RESOURCE_RECORD> *pListResourceRecords);
    qint64 getResourceSizeByNumber(quint32 nNumber);
    qint64 getResourceSizeByNumber(quint32 nNumber,QList<XPE::RESOURCE_RECORD> *pListResourceRecords);
    quint32 getResourceTypeByNumber(quint32 nNumber);
    quint32 getResourceTypeByNumber(quint32 nNumber,QList<XPE::RESOURCE_RECORD> *pListResourceRecords);

    qint64 getResourceNameOffset(QString sName);
    qint64 getResourceNameOffset(QString sName,QList<XPE::RESOURCE_RECORD> *pListResourceRecords);
    bool isResourceNamePresent(QString sName);
    bool isResourceNamePresent(QString sName,QList<XPE::RESOURCE_RECORD> *pListResourceRecords);

    virtual _MEMORY_MAP getMemoryMap();
    virtual qint64 getBaseAddress();
    virtual void setBaseAddress(qint64 nBaseAddress);
    virtual qint64 getEntryPointOffset(_MEMORY_MAP *pMemoryMap);
    virtual void setEntryPointOffset(qint64 nEntryPointOffset);
    XPE_DEF::IMAGE_IMPORT_DESCRIPTOR read_IMAGE_IMPORT_DESCRIPTOR(qint64 nOffset);
    void write_IMAGE_IMPORT_DESCRIPTOR(qint64 nOffset,XPE_DEF::IMAGE_IMPORT_DESCRIPTOR idd);

    XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR _read_IMAGE_DELAYLOAD_DESCRIPTOR(qint64 nOffset);

    bool isExportPresent();

    EXPORT_HEADER getExport();
    EXPORT_HEADER getExport(_MEMORY_MAP *pMemoryMap);
    static QList<QString> getExportFunctionsList(EXPORT_HEADER *pExportHeader);

    XPE_DEF::IMAGE_EXPORT_DIRECTORY getExportDirectory();
    void setExportDirectory(XPE_DEF::IMAGE_EXPORT_DIRECTORY *pExportDirectory);

    void setExportDirectory_Characteristics(quint32 nValue);
    void setExportDirectory_TimeDateStamp(quint32 nValue);
    void setExportDirectory_MajorVersion(quint16 nValue);
    void setExportDirectory_MinorVersion(quint16 nValue);
    void setExportDirectory_Name(quint32 nValue);
    void setExportDirectory_Base(quint32 nValue);
    void setExportDirectory_NumberOfFunctions(quint32 nValue);
    void setExportDirectory_NumberOfNames(quint32 nValue);
    void setExportDirectory_AddressOfFunctions(quint32 nValue);
    void setExportDirectory_AddressOfNames(quint32 nValue);
    void setExportDirectory_AddressOfNameOrdinals(quint32 nValue);

    QByteArray getHeaders();

    OFFSETSIZE __getSectionOffsetAndSize(quint32 nSection); // TODO move to Xbinary

    QByteArray getSection(quint32 nSection); // TODO move to Xbinary

    QString getSectionHash(HASH hash,quint32 nSection); // TODO move to Xbinary
    double getSectionEntropy(quint32 nSection); // TODO move to Xbinary

    bool addImportSection(QMap<qint64,QString> *pMapIAT);
    bool addImportSection(QIODevice *pDevice,bool bIsImage,QMap<qint64,QString> *pMapIAT);
    bool addImportSection(QString sFileName,bool bIsImage,QMap<qint64,QString> *pMapIAT);

    static QList<XPE::IMPORT_HEADER> mapIATToList(QMap<qint64,QString> *pMapIAT,bool bIs64);

    quint32 calculateCheckSum();

    bool addSection(XPE_DEF::IMAGE_SECTION_HEADER *pSectionHeader,char *pData,qint64 nDataSize);
    bool addSection(QString sFileName,bool bIsImage,XPE_DEF::IMAGE_SECTION_HEADER *pSectionHeader,char *pData,qint64 nDataSize);
    bool addSection(QIODevice *pDevice, bool bIsImage, XPE_DEF::IMAGE_SECTION_HEADER *pSectionHeader, char *pData, qint64 nDataSize);

    bool removeLastSection();
    static bool removeLastSection(QIODevice *pDevice, bool bIsImage);
    static bool removeLastSection(QString sFileName,bool bIsImage);
    // TODO copy Overlay function -> XBinary
    XPE_DEF::IMAGE_RESOURCE_DIRECTORY_ENTRY read_IMAGE_RESOURCE_DIRECTORY_ENTRY(qint64 nOffset);
    XPE_DEF::IMAGE_RESOURCE_DIRECTORY read_IMAGE_RESOURCE_DIRECTORY(qint64 nOffset);
    XPE_DEF::IMAGE_RESOURCE_DATA_ENTRY read_IMAGE_RESOURCE_DATA_ENTRY(qint64 nOffset);
    XPE::RESOURCES_ID_NAME getResourcesIDName(qint64 nResourceOffset, quint32 nValue);

    QList<qint64> getRelocsAsRVAList();

    QList<RELOCS_HEADER> getRelocsHeaders();
    QList<RELOCS_POSITION> getRelocsPositions(qint64 nOffset);

    XPE_DEF::IMAGE_BASE_RELOCATION _readIMAGE_BASE_RELOCATION(qint64 nOffset);

    quint32 getRelocsVirtualAddress(qint64 nOffset);
    quint32 getRelocsSizeOfBlock(qint64 nOffset);
    void setRelocsVirtualAddress(qint64 nOffset,quint32 nValue);
    void setRelocsSizeOfBlock(qint64 nOffset,quint32 nValue);

    bool addRelocsSection(QList<qint64> *pList);
    bool addRelocsSection(QIODevice *pDevice, bool bIsImage, QList<qint64> *pListRelocs);
    bool addRelocsSection(QString sFileName, bool bIsImage, QList<qint64> *pListRelocs);
    static QByteArray relocsAsRVAListToByteArray(QList<qint64> *pList,bool bIs64);

    bool isResourcesPresent();
    bool isRelocsPresent();
    bool isDebugPresent();
    bool isTLSPresent();
    bool isSignPresent();
    bool isExceptionPresent();
    bool isLoadConfigPresent();
    bool isBoundImportPresent();
    bool isDelayImportPresent();

    qint64 getTLSHeaderOffset();
    qint64 getTLSHeaderSize();

    XPE_DEF::S_IMAGE_TLS_DIRECTORY32 getTLSDirectory32();
    XPE_DEF::S_IMAGE_TLS_DIRECTORY64 getTLSDirectory64();

    quint64 getTLS_StartAddressOfRawData();
    quint64 getTLS_EndAddressOfRawData();
    quint64 getTLS_AddressOfIndex();
    quint64 getTLS_AddressOfCallBacks();
    quint32 getTLS_SizeOfZeroFill();
    quint32 getTLS_Characteristics();

    void setTLS_StartAddressOfRawData(quint64 nValue);
    void setTLS_EndAddressOfRawData(quint64 nValue);
    void setTLS_AddressOfIndex(quint64 nValue);
    void setTLS_AddressOfCallBacks(quint64 nValue);
    void setTLS_SizeOfZeroFill(quint32 nValue);
    void setTLS_Characteristics(quint32 nValue);

    QList<qint64> getTLS_CallbacksList();
    QList<qint64> getTLS_CallbacksList(XBinary::_MEMORY_MAP *pMemoryMap);

    TLS_HEADER getTLSHeader();

    static QMap<quint64,QString> getImageNtHeadersSignatures();
    static QMap<quint64,QString> getImageNtHeadersSignaturesS();
    static QMap<quint64,QString> getImageMagics();
    static QMap<quint64,QString> getImageMagicsS();
    static QMap<quint64,QString> getImageFileHeaderMachines();
    static QMap<quint64,QString> getImageFileHeaderMachinesS();
    static QMap<quint64,QString> getImageFileHeaderCharacteristics();
    static QMap<quint64,QString> getImageFileHeaderCharacteristicsS();
    static QMap<quint64,QString> getImageOptionalHeaderMagic();
    static QMap<quint64,QString> getImageOptionalHeaderMagicS();
    static QMap<quint64,QString> getImageOptionalHeaderSubsystem();
    static QMap<quint64,QString> getImageOptionalHeaderSubsystemS();
    static QMap<quint64,QString> getImageOptionalHeaderDllCharacteristics();
    static QMap<quint64,QString> getImageOptionalHeaderDllCharacteristicsS();
    static QMap<quint64,QString> getImageOptionalHeaderDataDirectory();
    static QMap<quint64,QString> getImageOptionalHeaderDataDirectoryS();
    static QMap<quint64,QString> getImageSectionHeaderFlags();
    static QMap<quint64,QString> getImageSectionHeaderFlagsS();
    static QMap<quint64,QString> getImageSectionHeaderAligns();
    static QMap<quint64,QString> getImageSectionHeaderAlignsS();
    static QMap<quint64,QString> getResourceTypes();
    static QMap<quint64,QString> getResourceTypesS();
    static QMap<quint64,QString> getImageRelBased();
    static QMap<quint64,QString> getImageRelBasedS();
    static QMap<quint64,QString> getComImageFlags();
    static QMap<quint64,QString> getComImageFlagsS();
    static QMap<quint64,QString> getDebugTypes();
    static QMap<quint64,QString> getDebugTypesS();

    qint64 calculateHeadersSize();
    qint64 _calculateHeadersSize(qint64 nSectionsTableOffset, quint32 nNumberOfSections);

    bool isDll();
    static bool isDll(QString sFileName);
    bool isConsole();
    bool isDriver();
    bool isNETPresent();

    CLI_INFO getCliInfo(bool bFindHidden);
    CLI_INFO getCliInfo(bool bFindHidden,XBinary::_MEMORY_MAP *pMemoryMap);

    OFFSETSIZE getNet_MetadataOffsetSize();

    CLI_METADATA_HEADER _read_MetadataHeader(qint64 nOffset);

    void setMetadataHeader_Signature(quint32 nValue);
    void setMetadataHeader_MajorVersion(quint16 nValue);
    void setMetadataHeader_MinorVersion(quint16 nValue);
    void setMetadataHeader_Reserved(quint32 nValue);
    void setMetadataHeader_VersionStringLength(quint32 nValue);
    void setMetadataHeader_Version(QString sValue);
    void setMetadataHeader_Flags(quint16 nValue);
    void setMetadataHeader_Streams(quint16 nValue);

    bool isDataDirectoryValid(XPE_DEF::IMAGE_DATA_DIRECTORY *pDataDirectory);
    bool isDataDirectoryValid(XPE_DEF::IMAGE_DATA_DIRECTORY *pDataDirectory, XBinary::_MEMORY_MAP *pMemoryMap);

    bool isNetMetadataPresent();
    bool isNetMetadataPresent(CLI_INFO *pCliInfo,XBinary::_MEMORY_MAP *pMemoryMap);

    bool isNETAnsiStringPresent(QString sString);
    static bool isNETAnsiStringPresent(QString sString,CLI_INFO *pCliInfo);

    bool isNETUnicodeStringPresent(QString sString);
    static bool isNETUnicodeStringPresent(QString sString,CLI_INFO *pCliInfo);

    int getEntryPointSection();
    int getEntryPointSection(XBinary::_MEMORY_MAP *pMemoryMap);
    int getImportSection();
    int getImportSection(XBinary::_MEMORY_MAP *pMemoryMap);
    int getExportSection();
    int getExportSection(XBinary::_MEMORY_MAP *pMemoryMap);
    int getTLSSection();
    int getTLSSection(XBinary::_MEMORY_MAP *pMemoryMap);
    int getResourcesSection();
    int getResourcesSection(XBinary::_MEMORY_MAP *pMemoryMap);
    int getRelocsSection();
    int getRelocsSection(XBinary::_MEMORY_MAP *pMemoryMap);
    int getNormalCodeSection();
    int getNormalCodeSection(XBinary::_MEMORY_MAP *pMemoryMap);
    int getNormalDataSection();
    int getNormalDataSection(XBinary::_MEMORY_MAP *pMemoryMap);
    int getConstDataSection();
    int getConstDataSection(XBinary::_MEMORY_MAP *pMemoryMap);

    // TODO .INIT section Check

    struct REBUILD_OPTIONS
    {
        bool bOptimize;
        bool bClearHeader;
        //        bool bRemoveLastSection;
        bool bSetEntryPoint;
        quint32 nEntryPoint;
        //        bool bAddImportSection;
        QMap<qint64,QString> mapIAT;
        QMap<qint64,quint64> mapPatches;
        //        bool bAddRelocsSection;
        QList<qint64> listRelocsRVAs;
        bool bRenameSections;
        QString sSectionName;
        bool bFixChecksum;
    };

    bool rebuildDump(QString sResultFile,REBUILD_OPTIONS *pRebuildOptions);
    static bool rebuildDump(QString sInputFile,QString sResultFile,REBUILD_OPTIONS *pRebuildOptions);
    static bool fixCheckSum(QString sFileName,bool bIsImage);
    void fixCheckSum();

    static QList<XPE_DEF::IMAGE_SECTION_HEADER> splitSection(QByteArray *pbaData,XPE_DEF::IMAGE_SECTION_HEADER sectionHeaderOriginal,quint32 nBlockSize);

    struct HEADER_OPTIONS
    {
        quint16 nMachine;
        quint16 nCharacteristics;
        quint16 nMagic;
        quint64 nImagebase;
        quint16 nSubsystem;
        quint16 nDllcharacteristics;
        quint8 nMajorOperationSystemVersion;
        quint8 nMinorOperationSystemVersion;
        quint32 nFileAlignment;
        quint32 nSectionAlignment;
        quint32 nAddressOfEntryPoint;
        quint32 nResourceRVA;
        quint32 nResourceSize;
    };

    static QByteArray createHeaderStub(HEADER_OPTIONS *pHeaderOptions);

    XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY32 getLoadConfigDirectory32();
    XPE_DEF::S_IMAGE_LOAD_CONFIG_DIRECTORY64 getLoadConfigDirectory64();

    qint64 getLoadConfigDirectoryOffset();
    qint64 getLoadConfigDirectorySize();

    quint32 getLoadConfig_Size();
    quint32 getLoadConfig_TimeDateStamp();
    quint16 getLoadConfig_MajorVersion();
    quint16 getLoadConfig_MinorVersion();
    quint32 getLoadConfig_GlobalFlagsClear();
    quint32 getLoadConfig_GlobalFlagsSet();
    quint32 getLoadConfig_CriticalSectionDefaultTimeout();
    quint64 getLoadConfig_DeCommitFreeBlockThreshold();
    quint64 getLoadConfig_DeCommitTotalFreeThreshold();
    quint64 getLoadConfig_LockPrefixTable();
    quint64 getLoadConfig_MaximumAllocationSize();
    quint64 getLoadConfig_VirtualMemoryThreshold();
    quint64 getLoadConfig_ProcessAffinityMask();
    quint16 getLoadConfig_CSDVersion();
    quint16 getLoadConfig_DependentLoadFlags();
    quint64 getLoadConfig_EditList();
    quint64 getLoadConfig_SecurityCookie();
    quint64 getLoadConfig_SEHandlerTable();
    quint64 getLoadConfig_SEHandlerCount();
    quint64 getLoadConfig_GuardCFCheckFunctionPointer();
    quint64 getLoadConfig_GuardCFDispatchFunctionPointer();
    quint64 getLoadConfig_GuardCFFunctionTable();
    quint64 getLoadConfig_GuardCFFunctionCount();
    quint32 getLoadConfig_GuardFlags();
    quint16 getLoadConfig_CodeIntegrity_Flags();
    quint16 getLoadConfig_CodeIntegrity_Catalog();
    quint32 getLoadConfig_CodeIntegrity_CatalogOffset();
    quint32 getLoadConfig_CodeIntegrity_Reserved();
    quint64 getLoadConfig_GuardAddressTakenIatEntryTable();
    quint64 getLoadConfig_GuardAddressTakenIatEntryCount();
    quint64 getLoadConfig_GuardLongJumpTargetTable();
    quint64 getLoadConfig_GuardLongJumpTargetCount();
    quint64 getLoadConfig_DynamicValueRelocTable();
    quint64 getLoadConfig_CHPEMetadataPointer();
    quint64 getLoadConfig_GuardRFFailureRoutine();
    quint64 getLoadConfig_GuardRFFailureRoutineFunctionPointer();
    quint32 getLoadConfig_DynamicValueRelocTableOffset();
    quint16 getLoadConfig_DynamicValueRelocTableSection();
    quint16 getLoadConfig_Reserved2();
    quint64 getLoadConfig_GuardRFVerifyStackPointerFunctionPointer();
    quint32 getLoadConfig_HotPatchTableOffset();
    quint32 getLoadConfig_Reserved3();
    quint64 getLoadConfig_EnclaveConfigurationPointer();
    quint64 getLoadConfig_VolatileMetadataPointer();

    void setLoadConfig_Size(quint32 nValue);
    void setLoadConfig_TimeDateStamp(quint32 nValue);
    void setLoadConfig_MajorVersion(quint16 nValue);
    void setLoadConfig_MinorVersion(quint16 nValue);
    void setLoadConfig_GlobalFlagsClear(quint32 nValue);
    void setLoadConfig_GlobalFlagsSet(quint32 nValue);
    void setLoadConfig_CriticalSectionDefaultTimeout(quint32 nValue);
    void setLoadConfig_DeCommitFreeBlockThreshold(quint64 nValue);
    void setLoadConfig_DeCommitTotalFreeThreshold(quint64 nValue);
    void setLoadConfig_LockPrefixTable(quint64 nValue);
    void setLoadConfig_MaximumAllocationSize(quint64 nValue);
    void setLoadConfig_VirtualMemoryThreshold(quint64 nValue);
    void setLoadConfig_ProcessAffinityMask(quint64 nValue);
    void setLoadConfig_CSDVersion(quint16 nValue);
    void setLoadConfig_DependentLoadFlags(quint16 nValue);
    void setLoadConfig_EditList(quint64 nValue);
    void setLoadConfig_SecurityCookie(quint64 nValue);
    void setLoadConfig_SEHandlerTable(quint64 nValue);
    void setLoadConfig_SEHandlerCount(quint64 nValue);
    void setLoadConfig_GuardCFCheckFunctionPointer(quint64 nValue);
    void setLoadConfig_GuardCFDispatchFunctionPointer(quint64 nValue);
    void setLoadConfig_GuardCFFunctionTable(quint64 nValue);
    void setLoadConfig_GuardCFFunctionCount(quint64 nValue);
    void setLoadConfig_GuardFlags(quint32 nValue);
    void setLoadConfig_CodeIntegrity_Flags(quint16 nValue);
    void setLoadConfig_CodeIntegrity_Catalog(quint16 nValue);
    void setLoadConfig_CodeIntegrity_CatalogOffset(quint32 nValue);
    void setLoadConfig_CodeIntegrity_Reserved(quint32 nValue);
    void setLoadConfig_GuardAddressTakenIatEntryTable(quint64 nValue);
    void setLoadConfig_GuardAddressTakenIatEntryCount(quint64 nValue);
    void setLoadConfig_GuardLongJumpTargetTable(quint64 nValue);
    void setLoadConfig_GuardLongJumpTargetCount(quint64 nValue);
    void setLoadConfig_DynamicValueRelocTable(quint64 nValue);
    void setLoadConfig_CHPEMetadataPointer(quint64 nValue);
    void setLoadConfig_GuardRFFailureRoutine(quint64 nValue);
    void setLoadConfig_GuardRFFailureRoutineFunctionPointer(quint64 nValue);
    void setLoadConfig_DynamicValueRelocTableOffset(quint32 nValue);
    void setLoadConfig_DynamicValueRelocTableSection(quint16 nValue);
    void setLoadConfig_Reserved2(quint16 nValue);
    void setLoadConfig_GuardRFVerifyStackPointerFunctionPointer(quint64 nValue);
    void setLoadConfig_HotPatchTableOffset(quint32 nValue);
    void setLoadConfig_Reserved3(quint32 nValue);
    void setLoadConfig_EnclaveConfigurationPointer(quint64 nValue);
    void setLoadConfig_VolatileMetadataPointer(quint64 nValue);

    XPE_DEF::S_IMAGE_RUNTIME_FUNCTION_ENTRY _read_IMAGE_RUNTIME_FUNCTION_ENTRY(qint64 nOffset);

    XPE_DEF::S_IMAGE_RUNTIME_FUNCTION_ENTRY _getException(qint32 nNumber);

    void setException_BeginAddress(qint32 nNumber,quint32 nValue);
    void setException_EndAddress(qint32 nNumber,quint32 nValue);
    void setException_UnwindInfoAddress(qint32 nNumber,quint32 nValue);

    qint64 getExceptionRecordOffset(qint32 nNumber);
    qint64 getExceptionRecordSize();

    QList<XPE_DEF::S_IMAGE_RUNTIME_FUNCTION_ENTRY> getExceptionsList();
    QList<XPE_DEF::S_IMAGE_RUNTIME_FUNCTION_ENTRY> getExceptionsList(XBinary::_MEMORY_MAP *pMemoryMap);

    XPE_DEF::S_IMAGE_DEBUG_DIRECTORY _read_IMAGE_DEBUG_DIRECTORY(qint64 nOffset);

    QList<XPE_DEF::S_IMAGE_DEBUG_DIRECTORY> getDebugList();
    QList<XPE_DEF::S_IMAGE_DEBUG_DIRECTORY> getDebugList(XBinary::_MEMORY_MAP *pMemoryMap);

    qint64 getDebugHeaderOffset(quint32 nNumber);
    qint64 getDebugHeaderSize();

    XPE_DEF::S_IMAGE_DEBUG_DIRECTORY getDebugHeader(quint32 nNumber);
    // TODO Get
    void setDebugHeader_Characteristics(quint32 nNumber,quint32 nValue);
    void setDebugHeader_TimeDateStamp(quint32 nNumber,quint32 nValue);
    void setDebugHeader_MajorVersion(quint32 nNumber,quint16 nValue);
    void setDebugHeader_MinorVersion(quint32 nNumber,quint16 nValue);
    void setDebugHeader_Type(quint32 nNumber,quint32 nValue);
    void setDebugHeader_SizeOfData(quint32 nNumber,quint32 nValue);
    void setDebugHeader_AddressOfRawData(quint32 nNumber,quint32 nValue);
    void setDebugHeader_PointerToRawData(quint32 nNumber,quint32 nValue);

    qint64 getDelayImportRecordOffset(qint32 nNumber);
    qint64 getDelayImportRecordSize();

    QList<XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR> getDelayImportsList();
    QList<XPE_DEF::S_IMAGE_DELAYLOAD_DESCRIPTOR> getDelayImportsList(XBinary::_MEMORY_MAP *pMemoryMap);
    // TODO Get
    void setDelayImport_AllAttributes(quint32 nNumber,quint32 nValue);
    void setDelayImport_DllNameRVA(quint32 nNumber,quint32 nValue);
    void setDelayImport_ModuleHandleRVA(quint32 nNumber,quint32 nValue);
    void setDelayImport_ImportAddressTableRVA(quint32 nNumber,quint32 nValue);
    void setDelayImport_ImportNameTableRVA(quint32 nNumber,quint32 nValue);
    void setDelayImport_BoundImportAddressTableRVA(quint32 nNumber,quint32 nValue);
    void setDelayImport_UnloadInformationTableRVA(quint32 nNumber,quint32 nValue);
    void setDelayImport_TimeDateStamp(quint32 nNumber,quint32 nValue);

    QList<DELAYIMPORT_POSITION> getDelayImportPositions(int nIndex);
    QList<DELAYIMPORT_POSITION> getDelayImportPositions(XBinary::_MEMORY_MAP *pMemoryMap,int nIndex);

    QList<BOUND_IMPORT_POSITION> getBoundImportPositions();
    QList<BOUND_IMPORT_POSITION> getBoundImportPositions(XBinary::_MEMORY_MAP *pMemoryMap);

    XPE_DEF::S_IMAGE_BOUND_IMPORT_DESCRIPTOR _read_IMAGE_BOUND_IMPORT_DESCRIPTOR(qint64 nOffset);

    qint64 getBoundImportRecordOffset(qint32 nNumber);
    qint64 getBoundImportRecordSize();

    void setBoundImport_TimeDateStamp(quint32 nNumber,quint32 nValue);
    void setBoundImport_OffsetModuleName(quint32 nNumber,quint16 nValue);
    void setBoundImport_NumberOfModuleForwarderRefs(quint32 nNumber,quint16 nValue);

    qint32 getNumberOfImports();
    QString getImportLibraryName(quint32 nNumber);
    QString getImportLibraryName(quint32 nNumber, QList<XPE::IMPORT_HEADER> *pListImport);

    qint32 getNumberOfImportThunks(quint32 nNumber);
    qint32 getNumberOfImportThunks(quint32 nNumber, QList<XPE::IMPORT_HEADER> *pListImport);

    qint64 getNetHeaderOffset();
    qint64 getNetHeaderSize();

    XPE_DEF::IMAGE_COR20_HEADER _read_IMAGE_COR20_HEADER(qint64 nOffset);

    XPE_DEF::IMAGE_COR20_HEADER getNetHeader();

    quint32 getNetHeader_cb();
    quint16 getNetHeader_MajorRuntimeVersion();
    quint16 getNetHeader_MinorRuntimeVersion();
    quint32 getNetHeader_MetaData_Address();
    quint32 getNetHeader_MetaData_Size();
    quint32 getNetHeader_Flags();
    quint32 getNetHeader_EntryPoint();
    quint32 getNetHeader_Resources_Address();
    quint32 getNetHeader_Resources_Size();
    quint32 getNetHeader_StrongNameSignature_Address();
    quint32 getNetHeader_StrongNameSignature_Size();
    quint32 getNetHeader_CodeManagerTable_Address();
    quint32 getNetHeader_CodeManagerTable_Size();
    quint32 getNetHeader_VTableFixups_Address();
    quint32 getNetHeader_VTableFixups_Size();
    quint32 getNetHeader_ExportAddressTableJumps_Address();
    quint32 getNetHeader_ExportAddressTableJumps_Size();
    quint32 getNetHeader_ManagedNativeHeader_Address();
    quint32 getNetHeader_ManagedNativeHeader_Size();

    void setNetHeader_cb(quint32 nValue);
    void setNetHeader_MajorRuntimeVersion(quint16 nValue);
    void setNetHeader_MinorRuntimeVersion(quint16 nValue);
    void setNetHeader_MetaData_Address(quint32 nValue);
    void setNetHeader_MetaData_Size(quint32 nValue);
    void setNetHeader_Flags(quint32 nValue);
    void setNetHeader_EntryPoint(quint32 nValue);
    void setNetHeader_Resources_Address(quint32 nValue);
    void setNetHeader_Resources_Size(quint32 nValue);
    void setNetHeader_StrongNameSignature_Address(quint32 nValue);
    void setNetHeader_StrongNameSignature_Size(quint32 nValue);
    void setNetHeader_CodeManagerTable_Address(quint32 nValue);
    void setNetHeader_CodeManagerTable_Size(quint32 nValue);
    void setNetHeader_VTableFixups_Address(quint32 nValue);
    void setNetHeader_VTableFixups_Size(quint32 nValue);
    void setNetHeader_ExportAddressTableJumps_Address(quint32 nValue);
    void setNetHeader_ExportAddressTableJumps_Size(quint32 nValue);
    void setNetHeader_ManagedNativeHeader_Address(quint32 nValue);
    void setNetHeader_ManagedNativeHeader_Size(quint32 nValue);

private:
    quint16 _checkSum(qint64 nStartValue,qint64 nDataSize);
    RESOURCE_POSITION _getResourcePosition(_MEMORY_MAP *pMemoryMap, qint64 nBaseAddress, qint64 nResourceOffset, qint64 nOffset, quint32 nLevel);
    qint64 _fixHeadersSize();
    qint64 _getMinSectionOffset(); // TODO move to XBinary
    void _fixFileOffsets(qint64 nDelta);
    quint32 __getResourceVersion(RESOURCE_VERSION *pResult,qint64 nOffset,qint64 nSize,QString sPrefix,int nLevel);
};

#endif // XPE_H
