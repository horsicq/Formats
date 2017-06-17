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
#ifndef QPE_H
#define QPE_H

#include "qmsdos.h"
#include "qpe_def.h"

// TODO info
class QPE : public QMSDOS
{
public:
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
        QString sFunctionName;
        qint64 nFunctionOffset;
    };

    struct EXPORT_HEADER
    {
        S_IMAGE_EXPORT_DIRECTORY directory;
        QString sName;
        QList<EXPORT_POSITION> listPositions;
    };

    struct RESOURCES_ID_NAME
    {
        QString sName;
        quint32 nNameOffset;
        quint32 nID;
    };

    struct RESOURCE_HEADER
    {
        QString sName[3];
        quint32 nNameOffset[3];
        quint32 nID[3];
        qint64 nIRDEOffset;
        qint64 nAddress;
        qint64 nOffset;
        qint64 nSize;
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
        S__tagVS_FIXEDFILEINFO fileInfo;
        QList<QString> listRecords; // TODO rename
    };

    struct CLI_INFO
    {
        bool bInit;
        qint64 nCLIHeaderOffset;
        S_IMAGE_COR20_HEADER header;

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

    explicit QPE(QIODevice *__pDevice=0,bool bIsImage=false);
    virtual bool isValid();
    bool is64();

    bool isRichSignaturePresent();
    QList<QPE::RICH_RECORD> getRichSignatureRecords();

    qint64 getDosStubSize();
    qint64 getDosStubOffset();
    QByteArray getDosStub();
    bool isDosStubPresent();

    qint64 getNtHeadersOffset();
    quint32 getNtHeaders_Signature();
    void setNtHeaders_Signature(quint32 value);
    qint64 getFileHeaderOffset();

    S_IMAGE_FILE_HEADER getFileHeader();
    void setFileHeader(S_IMAGE_FILE_HEADER *pFileHeader);
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

    S_IMAGE_OPTIONAL_HEADER32 getOptionalHeader32();
    S_IMAGE_OPTIONAL_HEADER64 getOptionalHeader64();

    void setOptionalHeader32(S_IMAGE_OPTIONAL_HEADER32 *pOptionalHeader32);
    void setOptionalHeader64(S_IMAGE_OPTIONAL_HEADER64 *pOptionalHeader64);

    S_IMAGE_OPTIONAL_HEADER32S getOptionalHeader32S();
    S_IMAGE_OPTIONAL_HEADER64S getOptionalHeader64S();

    void setOptionalHeader32S(S_IMAGE_OPTIONAL_HEADER32S *pOptionalHeader32S);
    void setOptionalHeader64S(S_IMAGE_OPTIONAL_HEADER64S *pOptionalHeader64S);

    quint16 getOptionalHeader_Magic();
    quint8 getOptionalHeader_MajorLinkerVersion();
    quint8 getOptionalHeader_MinorLinkerVersion();
    quint32 getOptionalHeader_SizeOfCode();
    quint32 getOptionalHeader_SizeOfInitializedData();
    quint32 getOptionalHeader_SizeOfUninitializedData();
    quint32 getOptionalHeader_AddressOfEntryPoint();
    quint32 getOptionalHeader_BaseOfCode();
    quint32 getOptionalHeader_BaseOfData();
    qint64 getOptionalHeader_ImageBase();
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

    // TODO set functions
    void setOptionalHeader_Magic(quint16 value);
    void setOptionalHeader_MajorLinkerVersion(quint8 value);
    void setOptionalHeader_MinorLinkerVersion(quint8 value);
    void setOptionalHeader_SizeOfCode(quint32 value);
    void setOptionalHeader_SizeOfInitializedData(quint32 value);
    void setOptionalHeader_SizeOfUninitializedData(quint32 value);
    void setOptionalHeader_AddressOfEntryPoint(quint32 value);
    void setOptionalHeader_BaseOfCode(quint32 value);
    void setOptionalHeader_BaseOfData(quint32 value);
    void setOptionalHeader_ImageBase(qint64 value);
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
    void setOptionalHeader_SizeOfStackReserve(qint64 value);
    void setOptionalHeader_SizeOfStackCommit(qint64 value);
    void setOptionalHeader_SizeOfHeapReserve(qint64 value);
    void setOptionalHeader_SizeOfHeapCommit(qint64 value);
    void setOptionalHeader_LoaderFlags(quint32 value);
    void setOptionalHeader_NumberOfRvaAndSizes(quint32 value);

    S_IMAGE_DATA_DIRECTORY getOptionalHeader_DataDirectory(quint32 nNumber);
    void setOptionalHeader_DataDirectory(quint32 nNumber,S_IMAGE_DATA_DIRECTORY *pDataDirectory);

    bool isOptionalHeader_DataDirectoryPresent(quint32 nNumber);

    QList<S_IMAGE_DATA_DIRECTORY> getDirectories();
    void setDirectories(QList<S_IMAGE_DATA_DIRECTORY> *pListDirectories);

    qint64 getDataDirectoryOffset(quint32 nNumber);
    QByteArray getDataDirectory(quint32 nNumber);

    qint64 getSectionsTableOffset();
    S_IMAGE_SECTION_HEADER getSectionHeader(quint32 nNumber);
    void setSectionHeader(quint32 nNumber,S_IMAGE_SECTION_HEADER *pSectionHeader);

    QList<S_IMAGE_SECTION_HEADER> getSectionHeaders();

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

    static bool isSectionNamePresent(QString sSectionName,QList<S_IMAGE_SECTION_HEADER> *pListSections);
    static S_IMAGE_SECTION_HEADER getSectionByName(QString sSectionName,QList<S_IMAGE_SECTION_HEADER> *pListSections);

    bool isImportPresent();

    QList<IMPORT_RECORD> getImportRecords();

    QList<S_IMAGE_IMPORT_DESCRIPTOR> getImportDescriptors();
    QList<S_IMAGE_IMPORT_DESCRIPTOR_EX> getImportDescriptorsEx();

    S_IMAGE_IMPORT_DESCRIPTOR getImportDescriptor(quint32 nNumber);
    void setImportDescriptor(quint32 nNumber,S_IMAGE_IMPORT_DESCRIPTOR *pImportDescriptor);

    QList<IMPORT_HEADER> getImports();
    QList<IMPORT_POSITION> getImportPositions(int nIndex);


    static bool isImportLibraryPresentI(QString sLibrary,QList<IMPORT_HEADER> *pListImport);

    bool setImports(QList<IMPORT_HEADER> *pListHeaders);
    static bool setImports(QIODevice *pDevice,QList<IMPORT_HEADER> *pListHeaders);
    static bool setImports(QString sFileName,QList<IMPORT_HEADER> *pListHeaders);

    QList<RESOURCE_HEADER> getResources();

    static RESOURCE_HEADER getResourceHeader(int nID1,int nID2,QList<RESOURCE_HEADER> *pListHeaders);
    static RESOURCE_HEADER getResourceHeader(int nID1,QString sName2,QList<RESOURCE_HEADER> *pListHeaders);
    static RESOURCE_HEADER getResourceHeader(QString sName1,int nID2,QList<RESOURCE_HEADER> *pListHeaders);
    static RESOURCE_HEADER getResourceHeader(QString sName1,QString sName2,QList<RESOURCE_HEADER> *pListHeaders);

    static bool isResourcePresent(int nID1,int nID2,QList<RESOURCE_HEADER> *pListHeaders);
    static bool isResourcePresent(int nID1,QString sName2,QList<RESOURCE_HEADER> *pListHeaders);
    static bool isResourcePresent(QString sName1,int nID2,QList<RESOURCE_HEADER> *pListHeaders);
    static bool isResourcePresent(QString sName1,QString sName2,QList<RESOURCE_HEADER> *pListHeaders);

    QString getResourceManifest(QList<QPE::RESOURCE_HEADER> *pListHeaders);
    S_VS_VERSION_INFO readResourceVersionInfo(qint64 nOffset);


    RESOURCE_VERSION getResourceVersion(QList<QPE::RESOURCE_HEADER> *pListHeaders);
    static QString getResourceVersionValue(QString sKey,QPE::RESOURCE_VERSION *pResVersion);

    virtual QList<MEMORY_MAP> getMemoryMapList();
    virtual qint64 getBaseAddress();
    virtual qint64 getEntryPointOffset();
    S_IMAGE_IMPORT_DESCRIPTOR read_S_IMAGE_IMPORT_DESCRIPTOR(qint64 nOffset);
    void write_S_IMAGE_IMPORT_DESCRIPTOR(qint64 nOffset,S_IMAGE_IMPORT_DESCRIPTOR value);

    bool isExportPresent();


    EXPORT_HEADER getExport();

    static bool isExportFunctionPresent(QString sFunction,EXPORT_HEADER *pExportHeader);

    S_IMAGE_EXPORT_DIRECTORY getExportDirectory();
    void setExportDirectory(S_IMAGE_EXPORT_DIRECTORY *pExportDirectory);

    QByteArray getHeaders();

    OFFSETSIZE __getSectionOffsetAndSize(quint32 nSection);

    QByteArray getSection(quint32 nSection);

    QString getSectionMD5(quint32 nSection);
    double getSectionEntropy(quint32 nSection);

    qint32 addressToSection(qint64 nAddress);
    qint32 addressToSection(QList<MEMORY_MAP> *pMemoryMap,qint64 nAddress);
    static bool fixDumpFile(QString sFileName,DUMP_OPTIONS *pDumpOptions);

    bool addImportSection(QMap<qint64,QString> *pMapIAT);
    static bool addImportSection(QIODevice *pDevice,QMap<qint64,QString> *pMapIAT);
    static bool addImportSection(QString sFileName,QMap<qint64,QString> *pMapIAT);

    static QList<QPE::IMPORT_HEADER> mapIATToList(QMap<qint64,QString> *pMapIAT,bool bIs64);

    quint32 calculateCheckSum();
    qint64 getOverlaySize();
    qint64 getOverlayOffset();
    bool isOverlayPresent();

    bool addOverlay(char *pData,qint64 nDataSize);
    static bool addOverlay(QString sFileName,char *pData,qint64 nDataSize);
    static bool addOverlay(QIODevice *pDevice,char *pData,qint64 nDataSize);
    bool addOverlayFromDevice(QIODevice *pSourceDevice,qint64 nOffset,qint64 nSize);
    bool addOverlayFromDevice(QIODevice *pDevice,QIODevice *pSourceDevice,qint64 nOffset,qint64 nSize);

    bool removeOverlay();
    static bool removeOverlay(QIODevice *pDevice);
    static bool removeOverlay(QString sFileName);
    bool addSection(S_IMAGE_SECTION_HEADER *pSectionHeader,char *pData,qint64 nDataSize);
    static bool addSection(QString sFileName,S_IMAGE_SECTION_HEADER *pSectionHeader,char *pData,qint64 nDataSize);
    static bool addSection(QIODevice *pDevice,S_IMAGE_SECTION_HEADER *pSectionHeader,char *pData,qint64 nDataSize);

    bool removeLastSection();
    static bool removeLastSection(QIODevice *pDevice);
    static bool removeLastSection(QString sFileName);
    // TODO copy Overlay function
    S_IMAGE_RESOURCE_DIRECTORY_ENTRY read_S_IMAGE_RESOURCE_DIRECTORY_ENTRY(qint64 nOffset);
    S_IMAGE_RESOURCE_DIRECTORY read_S_IMAGE_RESOURCE_DIRECTORY(qint64 nOffset);
    S_IMAGE_RESOURCE_DATA_ENTRY read_S_IMAGE_RESOURCE_DATA_ENTRY(qint64 nOffset);
    QPE::RESOURCES_ID_NAME getResourcesIDName(qint64 nResourceOffset, quint32 value);

    QList<qint64> getRelocsAsRVAList();
    bool addRelocsSection(QList<qint64> *pList);
    static bool addRelocsSection(QIODevice *pDevice,QList<qint64> *pList);
    static bool addRelocsSection(QString sFileName,QList<qint64> *pList);
    static QByteArray relocsAsRVAListToByteArray(QList<qint64> *pList,bool bIs64);

    bool isResourcesPresent();
    bool isRelocsPresent();
    bool isTLSPresent();

    static QMap<quint64,QString> getImageFileMachines();
    static QMap<quint64,QString> getImageFileCharacteristics();
    static QMap<quint64,QString> getImageNtOptionalMagic();
    static QMap<quint64,QString> getImageNtOptionalSubsystem();
    static QMap<quint64,QString> getImageNtOptionalDllCharacteristics();
    static QMap<quint64,QString> getImageSectionFlags();
    static QMap<quint64,QString> getImageSectionAligns();

    qint64 _calculateHeadersSize(qint64 nSectionsTableOffset, quint32 nNumberOfSections);

    bool isDll();

    bool isNETPresent();
    CLI_INFO getCliInfo();

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
private:
    quint16 _checkSum(qint64 nStartValue,qint64 nDataSize);
    qint64 _calculateRawSize();
    void _fixCheckSum();

    qint64 _fixHeadersSize();
    qint64 _getMinSectionOffset();
    void _fixFileOffsets(qint64 nDelta);
    quint32 __getResourceVersion(RESOURCE_VERSION *pResult,qint64 nOffset,qint64 nSize,QString sPrefix,int nLevel);
};

#endif // QPE_H
