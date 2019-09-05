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
#ifndef XPE_H
#define XPE_H

#include "xmsdos.h"
#include "xpe_def.h"

class XPE : public XMSDOS
{
    Q_OBJECT

public:
    struct SECTIONFILE_RECORD
    {
        QString sName;
        qint64 nOffset;
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
        quint32 nFirstThunk; // For path only!
        QList<IMPORT_POSITION> listPositions;
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
        //qint64 nFunctionOffset;
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
        QString sName[3];
        quint32 nNameOffset[3];
        quint32 nID[3];
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
        XPE_DEF::IMAGE_RESOURCE_DIRECTORY_ENTRY dir_entry;
        XPE_DEF::IMAGE_RESOURCE_DATA_ENTRY data_entry;
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

    struct RICH_RECORD
    {
        quint32 nId;
        quint32 nVersion;
        quint32 nCount;
    };

    struct RESOURCE_VERSION
    {
        XPE_DEF::S_tagVS_FIXEDFILEINFO fileInfo;
        QList<QString> listRecords; // TODO rename
    };

    struct CLI_INFO
    {
        bool bInit;
        bool bHidden;
        qint64 nCLIHeaderOffset;
        XPE_DEF::IMAGE_COR20_HEADER header;

        qint64 nCLI_MetaDataOffset;
        quint32 nCLI_MetaData_Signature;
        quint16 sCLI_MetaData_MajorVersion;
        quint16 sCLI_MetaData_MinorVersion;
        quint32 nCLI_MetaData_Reserved;
        quint32 nCLI_MetaData_VersionStringLength;
        QString sCLI_MetaData_Version;
        quint16 sCLI_MetaData_Flags;
        quint16 sCLI_MetaData_Streams;

        QList <qint64> listCLI_MetaData_Stream_Offsets;
        QList <qint64> listCLI_MetaData_Stream_Sizes;
        QList <QString> listCLI_MetaData_Stream_Names;

        qint64 nCLI_MetaData_TablesHeaderOffset;
        qint64 nCLI_MetaData_TablesSize;

        quint32 nCLI_MetaData_Tables_Reserved1;
        quint8 cCLI_MetaData_Tables_MajorVersion;
        quint8 cCLI_MetaData_Tables_MinorVersion;
        quint8 cCLI_MetaData_Tables_HeapOffsetSizes;
        quint8 cCLI_MetaData_Tables_Reserved2;
        quint64 nCLI_MetaData_Tables_Valid;
        quint64 nCLI_MetaData_Tables_Sorted;
        quint32 nCLI_MetaData_Tables_Valid_NumberOfRows;
        quint32 CLI_MetaData_Tables_TablesNumberOfIndexes[64];
        qint64 CLI_MetaData_Tables_TablesOffsets[64];
        qint64 CLI_MetaData_Tables_TablesSizes[64];

        qint64 nCLI_MetaData_StringsOffset;
        qint64 nCLI_MetaData_StringsSize;
        qint64 nCLI_MetaData_USOffset;
        qint64 nCLI_MetaData_USSize;
        qint64 nCLI_MetaData_BlobOffset;
        qint64 nCLI_MetaData_BlobSize;
        qint64 nCLI_MetaData_GUIDOffset;
        qint64 nCLI_MetaData_GUIDSize;

        qint64 nEntryPoint;
        qint64 nEntryPointSize;

        QList<QString> listAnsiStrings;
        QList<QString> listUnicodeStrings;
    };

    struct IMAGE_IMPORT_DESCRIPTOR_EX
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

    explicit XPE(QIODevice *__pDevice=nullptr,bool bIsImage=false,qint64 nImageBase=-1);
    virtual bool isValid();
    bool is64();

    bool isRichSignaturePresent();
    QList<XPE::RICH_RECORD> getRichSignatureRecords();

    qint64 getDosStubSize();
    qint64 getDosStubOffset();
    QByteArray getDosStub();
    bool isDosStubPresent();

    qint64 getNtHeadersOffset();
    quint32 getNtHeaders_Signature();
    void setNtHeaders_Signature(quint32 value);
    qint64 getFileHeaderOffset();

    XPE_DEF::S_IMAGE_FILE_HEADER getFileHeader();
    void setFileHeader(XPE_DEF::S_IMAGE_FILE_HEADER *pFileHeader);
    quint16 getFileHeader_Machine();
    quint16 getFileHeader_NumberOfSections();
    quint32 getFileHeader_TimeDateStamp();
    quint32 getFileHeader_PointerToSymbolTable();
    quint32 getFileHeader_NumberOfSymbols();
    quint16 getFileHeader_SizeOfOptionalHeader();
    quint16 getFileHeader_Characteristics();

    void setFileHeader_Machine(quint16 value);
    void setFileHeader_NumberOfSections(quint16 value);
    void setFileHeader_TimeDateStamp(quint32 value);
    void setFileHeader_PointerToSymbolTable(quint32 value);
    void setFileHeader_NumberOfSymbols(quint32 value);
    void setFileHeader_SizeOfOptionalHeader(quint16 value);
    void setFileHeader_Characteristics(quint16 value);

    qint64 getOptionalHeaderOffset();

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

    void setOptionalHeader_Magic(quint16 value);
    void setOptionalHeader_MajorLinkerVersion(quint8 value);
    void setOptionalHeader_MinorLinkerVersion(quint8 value);
    void setOptionalHeader_SizeOfCode(quint32 value);
    void setOptionalHeader_SizeOfInitializedData(quint32 value);
    void setOptionalHeader_SizeOfUninitializedData(quint32 value);
    void setOptionalHeader_AddressOfEntryPoint(quint32 value);
    void setOptionalHeader_BaseOfCode(quint32 value);
    void setOptionalHeader_BaseOfData(quint32 value);
    void setOptionalHeader_ImageBase(quint64 value);
    void setOptionalHeader_SectionAlignment(quint32 value);
    void setOptionalHeader_FileAlignment(quint32 value);
    void setOptionalHeader_MajorOperatingSystemVersion(quint16 value);
    void setOptionalHeader_MinorOperatingSystemVersion(quint16 value);
    void setOptionalHeader_MajorImageVersion(quint16 value);
    void setOptionalHeader_MinorImageVersion(quint16 value);
    void setOptionalHeader_MajorSubsystemVersion(quint16 value);
    void setOptionalHeader_MinorSubsystemVersion(quint16 value);
    void setOptionalHeader_Win32VersionValue(quint32 value);
    void setOptionalHeader_SizeOfImage(quint32 value);
    void setOptionalHeader_SizeOfHeaders(quint32 value);
    void setOptionalHeader_CheckSum(quint32 value);
    void setOptionalHeader_Subsystem(quint16 value);
    void setOptionalHeader_DllCharacteristics(quint16 value);
    void setOptionalHeader_SizeOfStackReserve(quint64 value);
    void setOptionalHeader_SizeOfStackCommit(quint64 value);
    void setOptionalHeader_SizeOfHeapReserve(quint64 value);
    void setOptionalHeader_SizeOfHeapCommit(quint64 value);
    void setOptionalHeader_LoaderFlags(quint32 value);
    void setOptionalHeader_NumberOfRvaAndSizes(quint32 value);

    XPE_DEF::IMAGE_DATA_DIRECTORY getOptionalHeader_DataDirectory(quint32 nNumber);
    void setOptionalHeader_DataDirectory(quint32 nNumber,XPE_DEF::IMAGE_DATA_DIRECTORY *pDataDirectory);

    void setOptionalHeader_DataDirectory_VirtualAddress(quint32 nNumber,quint32 value);
    void setOptionalHeader_DataDirectory_Size(quint32 nNumber,quint32 value);

    void clearOptionalHeader_DataDirectory(quint32 nNumber);

    bool isOptionalHeader_DataDirectoryPresent(quint32 nNumber);

    QList<XPE_DEF::IMAGE_DATA_DIRECTORY> getDirectories();
    void setDirectories(QList<XPE_DEF::IMAGE_DATA_DIRECTORY> *pListDirectories);

    qint64 getDataDirectoryOffset(quint32 nNumber);
    QByteArray getDataDirectory(quint32 nNumber);

    qint64 getSectionsTableOffset();
    XPE_DEF::IMAGE_SECTION_HEADER getSectionHeader(quint32 nNumber);
    void setSectionHeader(quint32 nNumber,XPE_DEF::IMAGE_SECTION_HEADER *pSectionHeader);

    QList<XPE_DEF::IMAGE_SECTION_HEADER> getSectionHeaders();
    // TODO with __getSectionOffsetAndSize
    static QList<SECTIONFILE_RECORD> getSectionRecords(QList<XPE_DEF::IMAGE_SECTION_HEADER> *pList,bool bIsImage);

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
    void setSection_VirtualSize(quint32 nNumber,quint32 value);
    void setSection_VirtualAddress(quint32 nNumber,quint32 value);
    void setSection_SizeOfRawData(quint32 nNumber,quint32 value);
    void setSection_PointerToRawData(quint32 nNumber,quint32 value);
    void setSection_PointerToRelocations(quint32 nNumber,quint32 value);
    void setSection_PointerToLinenumbers(quint32 nNumber,quint32 value);
    void setSection_NumberOfRelocations(quint32 nNumber,quint16 value);
    void setSection_NumberOfLinenumbers(quint32 nNumber,quint16 value);
    void setSection_Characteristics(quint32 nNumber,quint32 value);

    static bool isSectionNamePresent(QString sSectionName,QList<XPE_DEF::IMAGE_SECTION_HEADER> *pListSections);
    static XPE_DEF::IMAGE_SECTION_HEADER getSectionByName(QString sSectionName,QList<XPE_DEF::IMAGE_SECTION_HEADER> *pListSections);

    bool isImportPresent();

    QList<IMPORT_RECORD> getImportRecords();

    QList<XPE_DEF::IMAGE_IMPORT_DESCRIPTOR> getImportDescriptors();
    QList<IMAGE_IMPORT_DESCRIPTOR_EX> getImportDescriptorsEx();

    XPE_DEF::IMAGE_IMPORT_DESCRIPTOR getImportDescriptor(quint32 nNumber);
    void setImportDescriptor(quint32 nNumber,XPE_DEF::IMAGE_IMPORT_DESCRIPTOR *pImportDescriptor);

    QList<IMPORT_HEADER> getImports();
    QList<IMPORT_POSITION> getImportPositions(int nIndex);

    static bool isImportLibraryPresentI(QString sLibrary,QList<IMPORT_HEADER> *pListImport);
    static bool isImportFunctionPresentI(QString sLibrary,QString sFunction,QList<IMPORT_HEADER> *pListImport);

    bool setImports(QList<IMPORT_HEADER> *pListHeaders);
    static bool setImports(QIODevice *pDevice,bool bIsImage,QList<IMPORT_HEADER> *pListHeaders);
    static bool setImports(QString sFileName,bool bIsImage,QList<IMPORT_HEADER> *pListHeaders);

    RESOURCE_HEADER getResourceHeader();
    QList<RESOURCE_RECORD> getResources();

    static RESOURCE_RECORD getResourceRecord(quint32 nID1,quint32 nID2,QList<RESOURCE_RECORD> *pListRecords);
    static RESOURCE_RECORD getResourceRecord(quint32 nID1,QString sName2,QList<RESOURCE_RECORD> *pListRecords);
    static RESOURCE_RECORD getResourceRecord(QString sName1,quint32 nID2,QList<RESOURCE_RECORD> *pListRecords);
    static RESOURCE_RECORD getResourceRecord(QString sName1,QString sName2,QList<RESOURCE_RECORD> *pListRecords);

    static bool isResourcePresent(quint32 nID1,quint32 nID2,QList<RESOURCE_RECORD> *pListHeaders);
    static bool isResourcePresent(quint32 nID1,QString sName2,QList<RESOURCE_RECORD> *pListHeaders);
    static bool isResourcePresent(QString sName1,quint32 nID2,QList<RESOURCE_RECORD> *pListHeaders);
    static bool isResourcePresent(QString sName1,QString sName2,QList<RESOURCE_RECORD> *pListHeaders);

    QString getResourceManifest(QList<XPE::RESOURCE_RECORD> *pListHeaders);
    XPE_DEF::S_VS_VERSION_INFO readResourceVersionInfo(qint64 nOffset);

    RESOURCE_VERSION getResourceVersion(QList<XPE::RESOURCE_RECORD> *pListHeaders);
    static QString getResourceVersionValue(QString sKey,XPE::RESOURCE_VERSION *pResVersion);

    virtual QList<MEMORY_MAP> getMemoryMapList();
    virtual qint64 getBaseAddress();
    virtual void setBaseAddress(qint64 nBaseAddress);
    virtual qint64 getEntryPointOffset();
    virtual void setEntryPointOffset(qint64 nEntryPointOffset);
    XPE_DEF::IMAGE_IMPORT_DESCRIPTOR read_IMAGE_IMPORT_DESCRIPTOR(qint64 nOffset);
    void write_IMAGE_IMPORT_DESCRIPTOR(qint64 nOffset,XPE_DEF::IMAGE_IMPORT_DESCRIPTOR value);

    bool isExportPresent();

    EXPORT_HEADER getExport();

    static bool isExportFunctionPresent(QString sFunction,EXPORT_HEADER *pExportHeader);

    XPE_DEF::IMAGE_EXPORT_DIRECTORY getExportDirectory();
    void setExportDirectory(XPE_DEF::IMAGE_EXPORT_DIRECTORY *pExportDirectory);

    void setExportDirectory_Characteristics(quint32 value);
    void setExportDirectory_TimeDateStamp(quint32 value);
    void setExportDirectory_MajorVersion(quint16 value);
    void setExportDirectory_MinorVersion(quint16 value);
    void setExportDirectory_Name(quint32 value);
    void setExportDirectory_Base(quint32 value);
    void setExportDirectory_NumberOfFunctions(quint32 value);
    void setExportDirectory_NumberOfNames(quint32 value);
    void setExportDirectory_AddressOfFunctions(quint32 value);
    void setExportDirectory_AddressOfNames(quint32 value);
    void setExportDirectory_AddressOfNameOrdinals(quint32 value);

    QByteArray getHeaders();

    OFFSETSIZE __getSectionOffsetAndSize(quint32 nSection);

    QByteArray getSection(quint32 nSection);

    QString getSectionMD5(quint32 nSection);
    double getSectionEntropy(quint32 nSection);

    qint32 addressToSection(qint64 nAddress);
    static qint32 addressToSection(QList<MEMORY_MAP> *pMemoryMap,qint64 nAddress);

    bool addImportSection(QMap<qint64,QString> *pMapIAT);
    static bool addImportSection(QIODevice *pDevice,bool bIsImage,QMap<qint64,QString> *pMapIAT);
    static bool addImportSection(QString sFileName,bool bIsImage,QMap<qint64,QString> *pMapIAT);

    static QList<XPE::IMPORT_HEADER> mapIATToList(QMap<qint64,QString> *pMapIAT,bool bIs64);

    quint32 calculateCheckSum();
    qint64 getOverlaySize();
    qint64 getOverlayOffset();
    bool isOverlayPresent();

    bool addOverlay(char *pData,qint64 nDataSize);
    static bool addOverlay(QString sFileName,bool bIsImage,char *pData,qint64 nDataSize);
    static bool addOverlay(QIODevice *pDevice, bool bIsImage, char *pData, qint64 nDataSize);
    bool addOverlayFromDevice(QIODevice *pSourceDevice,qint64 nOffset,qint64 nSize);
    bool addOverlayFromDevice(QIODevice *pDevice,bool bIsImage,QIODevice *pSourceDevice,qint64 nOffset,qint64 nSize);

    bool removeOverlay();
    static bool removeOverlay(QIODevice *pDevice, bool bIsImage);
    static bool removeOverlay(QString sFileName, bool bIsImage);
    bool addSection(XPE_DEF::IMAGE_SECTION_HEADER *pSectionHeader,char *pData,qint64 nDataSize);
    static bool addSection(QString sFileName,bool bIsImage,XPE_DEF::IMAGE_SECTION_HEADER *pSectionHeader,char *pData,qint64 nDataSize);
    static bool addSection(QIODevice *pDevice, bool bIsImage, XPE_DEF::IMAGE_SECTION_HEADER *pSectionHeader, char *pData, qint64 nDataSize);

    bool removeLastSection();
    static bool removeLastSection(QIODevice *pDevice, bool bIsImage);
    static bool removeLastSection(QString sFileName,bool bIsImage);
    // TODO copy Overlay function
    XPE_DEF::IMAGE_RESOURCE_DIRECTORY_ENTRY read_IMAGE_RESOURCE_DIRECTORY_ENTRY(qint64 nOffset);
    XPE_DEF::IMAGE_RESOURCE_DIRECTORY read_IMAGE_RESOURCE_DIRECTORY(qint64 nOffset);
    XPE_DEF::IMAGE_RESOURCE_DATA_ENTRY read_IMAGE_RESOURCE_DATA_ENTRY(qint64 nOffset);
    XPE::RESOURCES_ID_NAME getResourcesIDName(qint64 nResourceOffset, quint32 value);

    QList<qint64> getRelocsAsRVAList();

    QList<RELOCS_HEADER> getRelocsHeaders();
    QList<RELOCS_POSITION> getRelocsPositions(qint64 nOffset);

    bool addRelocsSection(QList<qint64> *pList);
    static bool addRelocsSection(QIODevice *pDevice, bool bIsImage, QList<qint64> *pList);
    static bool addRelocsSection(QString sFileName, bool bIsImage, QList<qint64> *pList);
    static QByteArray relocsAsRVAListToByteArray(QList<qint64> *pList,bool bIs64);

    bool isResourcesPresent();
    bool isRelocsPresent();
    bool isTLSPresent();
    bool isSignPresent();
    bool isExceptionPresent();
    bool isLoadConfigPresent();

    XPE_DEF::S_IMAGE_TLS_DIRECTORY32 getTLSDirectory32();
    XPE_DEF::S_IMAGE_TLS_DIRECTORY64 getTLSDirectory64();

    quint64 getTLS_StartAddressOfRawData();
    quint64 getTLS_EndAddressOfRawData();
    quint64 getTLS_AddressOfIndex();
    quint64 getTLS_AddressOfCallBacks();
    quint32 getTLS_SizeOfZeroFill();
    quint32 getTLS_Characteristics();

    void setTLS_StartAddressOfRawData(quint64 value);
    void setTLS_EndAddressOfRawData(quint64 value);
    void setTLS_AddressOfIndex(quint64 value);
    void setTLS_AddressOfCallBacks(quint64 value);
    void setTLS_SizeOfZeroFill(quint32 value);
    void setTLS_Characteristics(quint32 value);

    QList<qint64> getTLS_CallbacksList();

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
    static QMap<quint64,QString> getImageSectionHeaderFlags();
    static QMap<quint64,QString> getImageSectionHeaderFlagsS();
    static QMap<quint64,QString> getImageSectionHeaderAligns();
    static QMap<quint64,QString> getImageSectionHeaderAlignsS();
    static QMap<quint64,QString> getResourceTypes();
    static QMap<quint64,QString> getResourceTypesS();
    static QMap<quint64,QString> getImageRelBased();
    static QMap<quint64,QString> getImageRelBasedS();

    qint64 _calculateHeadersSize(qint64 nSectionsTableOffset, quint32 nNumberOfSections);

    bool isDll();
    static bool isDll(QString sFileName);
    bool isConsole();

    bool isNETPresent();
    CLI_INFO getCliInfo(bool bFindHidden);

    static bool isNETAnsiStringPresent(QString sString,CLI_INFO *pCliInfo);

    int getEntryPointSection();
    int getImportSection();
    int getExportSection();
    int getTLSSection();
    int getResourcesSection();
    int getRelocsSection();
    int getNormalCodeSection();
    int getNormalDataSection();
    int getConstDataSection();

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

    static QList<XPE_DEF::IMAGE_SECTION_HEADER> splitSection(QByteArray *pbaData,XPE_DEF::IMAGE_SECTION_HEADER shOriginal,quint32 nBlockSize);

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

    quint32 getLoadConfig_Size();
    quint32 getLoadConfig_TimeDateStamp();
    quint16 getLoadConfig_MajorVersion();
    quint16 getLoadConfig_MinorVersion();

    void setLoadConfig_Size(quint32 value);
    void setLoadConfig_TimeDateStamp(quint32 value);
    void setLoadConfig_MajorVersion(quint16 value);
    void setLoadConfig_MinorVersion(quint16 value);

private:
    quint16 _checkSum(qint64 nStartValue,qint64 nDataSize);
    qint64 _calculateRawSize();
    RESOURCE_POSITION _getResourcePosition(QList<MEMORY_MAP> *pMemoryMap, qint64 nBaseAddress, qint64 nResourceOffset, qint64 nOffset, quint32 nLevel);
    qint64 _fixHeadersSize();
    qint64 _getMinSectionOffset();
    void _fixFileOffsets(qint64 nDelta);
    quint32 __getResourceVersion(RESOURCE_VERSION *pResult,qint64 nOffset,qint64 nSize,QString sPrefix,int nLevel);
};

#endif // XPE_H
