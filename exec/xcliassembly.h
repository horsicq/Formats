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
#ifndef XCLIASSEMBLY_H
#define XCLIASSEMBLY_H

#include "xbinary.h"
#include "xcliassembly_def.h"

class XCLIAssembly : public XBinary {
    Q_OBJECT

public:
    struct INTERNAL_INFO : XBinary::INTERNAL_INFO {};

    virtual bool handleInternalInfo(PDSTRUCT *pPdStruct) override;
    virtual void *getInternalInfo(PDSTRUCT *pPdStruct) override;
    virtual void setInternalInfo(void *pInternalInfo) override;

    enum TYPE {
        TYPE_UNKNOWN = 0,
        TYPE_EXE,
        TYPE_DLL,
    };

    enum STRUCTID {
        STRUCTID_UNKNOWN = 0,
        STRUCTID_MZDOSHEADER,
        STRUCTID_NTHEADER,
        STRUCTID_CLRHEADER,
    };

    struct CLI_METADATA_HEADER {
        quint32 nSignature;
        quint16 nMajorVersion;
        quint16 nMinorVersion;
        quint32 nReserved;
        quint32 nVersionStringLength;
        QString sVersion;
        quint16 nFlags;
        quint16 nStreams;
    };

    struct CLI_METADATA_STREAM {
        qint64 nOffset;
        qint64 nSize;
        QString sName;
    };

    struct CLI_METADATA {
        CLI_METADATA_HEADER header;
        QList<CLI_METADATA_STREAM> listStreams;
        OFFSETSIZE osMetadata;
        quint32 nTables_Reserved1;
        quint8 cTables_MajorVersion;
        quint8 cTables_MinorVersion;
        quint8 cTables_HeapOffsetSizes;
        quint8 cTables_Reserved2;
        quint64 nTables_Valid;
        quint64 nTables_Sorted;
        quint32 nTables_Valid_NumberOfRows;        // TODO remove
        quint32 Tables_TablesNumberOfIndexes[64];  // TODO const
        qint64 Tables_TablesOffsets[64];           // TODO const
        qint64 Tables_TableElementSizes[64];       // TODO const
        OFFSETSIZE osStrings;
        OFFSETSIZE osUS;
        OFFSETSIZE osBlob;
        OFFSETSIZE osGUID;
        QByteArray baMetadata;
        QByteArray baStrings;
        QByteArray baUS;
        QByteArray baBlob;
        QByteArray baGUID;
        qint64 nEntryPoint;
        qint64 nEntryPointSize;
        // QList<QString> listAnsiStrings;
        // QList<QString> listUnicodeStrings;
        // QList<QString> listGUIDs;
        qint32 nStringIndexSize;
        qint32 nGUIDIndexSize;
        qint32 nBLOBIndexSize;
        qint32 nResolutionScopeSize;
        qint32 nTypeDefOrRefSize;
        qint32 nMethodDefOrRefSize;
        qint32 nMemberRefParentSize;
        qint32 nHasConstantSize;
        qint32 nHasCustomAttributeSize;
        qint32 nCustomAttributeTypeSize;
        qint32 nHasFieldMarshalSize;
        qint32 nHasDeclSecuritySize;
        qint32 nHasSemanticsSize;
        qint32 nMemberForwardedSize;
        qint32 nImplementationSize;
        qint32 nTypeOrMethodDefSize;
        qint32 nHasCustomDebugInformationSize;
        quint32 indexSize[64];
    };

    struct CLI_INFO {
        bool bValid;
        bool bHidden;
        qint64 nHeaderOffset;
        XCLIASSEMBLY_DEF::IMAGE_COR20_HEADER header;
        qint64 nMetaDataOffset;
        CLI_METADATA metaData;
    };

    struct CLI_METADATA_RECORD {
        quint32 nNumber;
        QString sId;
        quint32 nCount;
        bool bIsSorted;
        qint64 nTableOffset;
        qint64 nTableSize;
    };

    explicit XCLIAssembly(QIODevice *pDevice = nullptr, bool bIsImage = false, XADDR nModuleAddress = -1);
    ~XCLIAssembly();

    virtual bool isValid(PDSTRUCT *pPdStruct = nullptr) override;
    static bool isValid(QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1, PDSTRUCT *pPdStruct = nullptr);
    static XBinary::MODE getMode(QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1);

    virtual bool isExecutable() override;
    virtual QList<MAPMODE> getMapModesList() override;
    virtual _MEMORY_MAP getMemoryMap(XBinary::MAPMODE mapMode = XBinary::MAPMODE_UNKNOWN, PDSTRUCT *pPdStruct = nullptr) override;
    virtual QString getArch() override;
    virtual MODE getMode() override;
    virtual ENDIAN getEndian() override;
    virtual qint64 getImageSize() override;
    virtual FT getFileType() override;
    virtual QString getVersion() override;
    virtual qint32 getType() override;
    virtual OSNAME getOsName() override;
    virtual QString typeIdToString(qint32 nType) override;
    virtual QString getMIMEString() override;
    virtual QString getFileFormatExt() override;
    virtual QString getFileFormatExtsString() override;
    virtual qint64 getFileFormatSize(PDSTRUCT *pPdStruct = nullptr) override;

    virtual QString structIDToString(quint32 nID) override;
    virtual QString structIDToFtString(quint32 nID) override;
    virtual quint32 ftStringToStructID(const QString &sFtString) override;
    // virtual QList<DATA_HEADER> getDataHeaders(const DATA_HEADERS_OPTIONS &dataHeadersOptions, PDSTRUCT *pPdStruct) override;
    virtual QList<FPART> getFileParts(quint32 nFileParts, qint32 nLimit = -1, PDSTRUCT *pPdStruct = nullptr) override;

    virtual QVector<XIMPORT_STRUCT> getImportStructs() override;
    virtual QVector<XEXPORT_STRUCT> getExportStructs() override;
    virtual QVector<XSYMBOL_STRUCT> getSymbolStructs() override;
    virtual QVector<XRESOURCE_STRUCT> getResourceStructs() override;
    virtual QVector<XMETADATA_STRUCT> getMetadataStructs() override;

    // .NET / CLI metadata parsing (moved from XPE)
    CLI_INFO getCliInfo(bool bFindHidden = false, PDSTRUCT *pPdStruct = nullptr);

    QList<QString> getAnsiStrings(CLI_INFO *pCliInfo, PDSTRUCT *pPdStruct = nullptr);
    QList<QString> getUnicodeStrings(CLI_INFO *pCliInfo, PDSTRUCT *pPdStruct = nullptr);

    bool isNetGlobalCctorPresent(CLI_INFO *pCliInfo, PDSTRUCT *pPdStruct = nullptr);
    bool isNetTypePresent(CLI_INFO *pCliInfo, const QString &sTypeNamespace, const QString &sTypeName, PDSTRUCT *pPdStruct = nullptr);
    bool isNetMethodPresent(CLI_INFO *pCliInfo, QString sTypeNamespace, QString sTypeName, QString sMethodName, PDSTRUCT *pPdStruct = nullptr);
    bool isNetFieldPresent(CLI_INFO *pCliInfo, QString sTypeNamespace, QString sTypeName, QString sFieldName, PDSTRUCT *pPdStruct = nullptr);

    XCLIASSEMBLY_DEF::S_METADATA_MODULE getMetadataModule(CLI_INFO *pCliInfo, qint32 nNumber);
    XCLIASSEMBLY_DEF::S_METADATA_MEMBERREF getMetadataMemberRef(CLI_INFO *pCliInfo, qint32 nNumber);
    XCLIASSEMBLY_DEF::S_METADATA_TYPEDEF getMetadataTypeDef(CLI_INFO *pCliInfo, qint32 nNumber);
    XCLIASSEMBLY_DEF::S_METADATA_TYPEREF getMetadataTypeRef(CLI_INFO *pCliInfo, qint32 nNumber);
    XCLIASSEMBLY_DEF::S_METADATA_MODULEREF getMetadataModuleRef(CLI_INFO *pCliInfo, qint32 nNumber);
    XCLIASSEMBLY_DEF::S_METADATA_METHODDEF getMetadataMethodDef(CLI_INFO *pCliInfo, qint32 nNumber);
    XCLIASSEMBLY_DEF::S_METADATA_METHODPTR getMetadataMethodPtr(CLI_INFO *pCliInfo, qint32 nNumber);
    XCLIASSEMBLY_DEF::S_METADATA_PARAM getMetadataParam(CLI_INFO *pCliInfo, qint32 nNumber);
    XCLIASSEMBLY_DEF::S_METADATA_TYPESPEC getMetadataTypeSpec(CLI_INFO *pCliInfo, qint32 nNumber);
    XCLIASSEMBLY_DEF::S_METADATA_FIELD getMetadataField(CLI_INFO *pCliInfo, qint32 nNumber);
    XCLIASSEMBLY_DEF::S_METADATA_METHODIMPL getMetadataMethodImpl(CLI_INFO *pCliInfo, qint32 nNumber);
    XCLIASSEMBLY_DEF::S_METADATA_ASSEMBLY getMetadataAssembly(CLI_INFO *pCliInfo, qint32 nNumber);
    XCLIASSEMBLY_DEF::S_METADATA_CONSTANT getMetadataConstant(CLI_INFO *pCliInfo, qint32 nNumber);
    XCLIASSEMBLY_DEF::S_METADATA_CUSTOMATTRIBUTE getMetadataCustomAttribute(CLI_INFO *pCliInfo, qint32 nNumber);
    XCLIASSEMBLY_DEF::S_METADATA_FIELDMARSHAL getMetadataFieldMarshal(CLI_INFO *pCliInfo, qint32 nNumber);
    XCLIASSEMBLY_DEF::S_METADATA_DECLSECURITY getMetadataDeclSecurity(CLI_INFO *pCliInfo, qint32 nNumber);

    QString getMetadataModuleName(CLI_INFO *pCliInfo, qint32 nNumber);
    QString getMetadataAssemblyName(CLI_INFO *pCliInfo, qint32 nNumber);

    XCLIASSEMBLY_DEF::S_METADATA_METHODDEFORREF getMetadataMethodDefOrRef(CLI_INFO *pCliInfo, quint32 nValue);

    QString getMetadataMemberRefParentName(CLI_INFO *pCliInfo, const XCLIASSEMBLY_DEF::S_METADATA_MEMBERREF &memberRef);
    static QString mdtIdToString(quint32 nID);

    CLI_METADATA_HEADER _read_MetadataHeader(qint64 nOffset);
    XCLIASSEMBLY_DEF::IMAGE_COR20_HEADER _read_IMAGE_COR20_HEADER(qint64 nOffset);

    QList<CLI_METADATA_RECORD> getCliMetadataRecords(CLI_INFO *pCliInfo, PDSTRUCT *pPdStruct = nullptr);

    qint64 findSignatureInBlob_NET(const QString &sSignature, PDSTRUCT *pPdStruct = nullptr);
    bool isSignatureInBlobPresent_NET(const QString &sSignature, PDSTRUCT *pPdStruct = nullptr);

    void setNetHeaderOffset(qint64 nOffset);
    void setNetMetaDataOffset(qint64 nOffset);
    void setVersion(const QString &sVersion);

private:
    struct PE_SECTION_REGION {
        quint32 nVirtualAddress;
        quint32 nVirtualSize;
        quint32 nPointerToRawData;
        quint32 nSizeOfRawData;
    };

    bool _parseHeaders();
    qint64 _rvaToOffset(quint32 nRVA);
    qint64 _getNetHeaderOffset();
    qint64 _getNetMetaDataOffset();

    qint64 g_nNetHeaderOffset;
    qint64 g_nNetMetaDataOffset;
    QString g_sVersion;
    bool g_bPeParsed;
    bool g_bPeValid;
    bool g_bPeIs64;
    quint16 g_nPeCharacteristics;
    quint32 g_nCliRva;
    quint32 g_nCliSize;
    QList<PE_SECTION_REGION> g_listSectionRegions;

private:
    INTERNAL_INFO m_internalInfo;
};

#endif  // XCLIASSEMBLY_H
