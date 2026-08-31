/* Copyright (c) 2017-2026 hors<horsicq@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef XZIP_H
#define XZIP_H

#include "xarchive.h"

// TODO OSNAME
class XZip : public XArchive {
    Q_OBJECT

public:
    struct INTERNAL_INFO : XArchive::INTERNAL_INFO {
        bool bECDOffsetCached = false;
        qint64 nECDOffset = -1;
    };

    bool handleInternalInfo(PDSTRUCT *pPdStruct) override;
    void *getInternalInfo(PDSTRUCT *pPdStruct) override;
    void setInternalInfo(void *pInternalInfo) override;

    virtual QList<QString> getSearchSignatures() override;
    virtual XBinary *createInstance(QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1) override;
    enum SIGNATURE {
        SIGNATURE_ECD = 0x06054B50,
        SIGNATURE_CFD = 0x02014B50,
        SIGNATURE_LFD = 0x04034B50
    };

    enum STRUCTID {
        STRUCTID_UNKNOWN = 0,
        STRUCTID_LOCALFILEHEADER,
        STRUCTID_CENTRALDIRECTORYFILEHEADER,
        STRUCTID_ENDOFCENTRALDIRECTORYRECORD,
    };

    //    0 - The file is stored (no compression)
    //    1 - The file is Shrunk
    //    2 - The file is Reduced with compression factor 1
    //    3 - The file is Reduced with compression factor 2
    //    4 - The file is Reduced with compression factor 3
    //    5 - The file is Reduced with compression factor 4
    //    6 - The file is Imploded
    //    7 - Reserved for Tokenizing compression algorithm
    //    8 - The file is Deflated
    //    9 - Enhanced Deflating using Deflate64(tm)
    //    10 - PKWARE Data Compression Library Imploding (old IBM TERSE)
    //    11 - Reserved by PKWARE
    //    12 - File is compressed using BZIP2 algorithm
    //    13 - Reserved by PKWARE
    //    14 - LZMA (EFS)
    //    15 - Reserved by PKWARE
    //    16 - Reserved by PKWARE
    //    17 - Reserved by PKWARE
    //    18 - File is compressed using IBM TERSE (new)
    //    19 - IBM LZ77 z Architecture (PFS)
    //    97 - WavPack compressed data
    //    98 - PPMd version I, Rev 1
    //    99 - Apple LZFSE

    enum CMETHOD {
        CMETHOD_STORE = 0,
        CMETHOD_SHRINK = 1,
        CMETHOD_REDUCED_1 = 2,
        CMETHOD_REDUCED_2 = 3,
        CMETHOD_REDUCED_3 = 4,
        CMETHOD_REDUCED_4 = 5,
        CMETHOD_IMPLODED = 6,
        CMETHOD_DEFLATE = 8,
        CMETHOD_DEFLATE64 = 9,
        CMETHOD_PKWARE_DCL_IMPLODING = 10,  // Also used for older PPMd variant
        CMETHOD_BZIP2 = 12,
        CMETHOD_LZMA = 14,
        CMETHOD_ZSTD = 93,
        CMETHOD_XZ = 95,
        CMETHOD_JPEG = 96,  // WinZip JPEG recompression
        CMETHOD_WAVPACK = 97,
        CMETHOD_PPMD = 98,
        CMETHOD_AES = 99,  // Apple or AES?
        // TODO 94 - MP3 (WinZip proprietary, no published specification)
    };

    // AES encryption mode constants
    enum AES_ENCRYPTION_MODE {
        AES_ENCRYPTION_MODE_128 = 0x01,  // 128-bit AES encryption
        AES_ENCRYPTION_MODE_192 = 0x02,  // 192-bit AES encryption
        AES_ENCRYPTION_MODE_256 = 0x03   // 256-bit AES encryption
    };

    // ZIP AES extra field header ID
    static const quint16 ZIP_AES_EXTRA_FIELD_HEADER_ID = 0x9901;
    static const quint16 ZIP_AES_EXTRA_FIELD_DATA_SIZE = 0x0007;
    static const quint16 ZIP_AES_VENDOR_ID_AE = 0x4541;  // 'AE' in little-endian

#pragma pack(push)
#pragma pack(1)
    struct LOCALFILEHEADER {
        quint32 nSignature;  // SIGNATURE_LFD
        quint8 nMinVersion;
        quint8 nMinOS;
        quint16 nFlags;
        quint16 nMethod;
        quint16 nLastModTime;
        quint16 nLastModDate;
        quint32 nCRC32;
        quint32 nCompressedSize;
        quint32 nUncompressedSize;
        quint16 nFileNameLength;
        quint16 nExtraFieldLength;
        // File name
        // Extra field
    };

    struct ENDOFCENTRALDIRECTORYRECORD {
        quint32 nSignature;  // SIGNATURE_ECD
        quint16 nDiskNumber;
        quint16 nStartDisk;
        quint16 nDiskNumberOfRecords;
        quint16 nTotalNumberOfRecords;
        quint32 nSizeOfCentralDirectory;
        quint32 nOffsetToCentralDirectory;
        quint16 nCommentLength;
        // Comment
    };

    struct CENTRALDIRECTORYFILEHEADER {
        quint32 nSignature;  // SIGNATURE_CFD
        quint8 nVersion;
        quint8 nOS;
        quint8 nMinVersion;
        quint8 nMinOS;
        quint16 nFlags;
        quint16 nMethod;
        quint16 nLastModTime;
        quint16 nLastModDate;
        quint32 nCRC32;
        quint32 nCompressedSize;
        quint32 nUncompressedSize;
        quint16 nFileNameLength;
        quint16 nExtraFieldLength;
        quint16 nFileCommentLength;
        quint16 nStartDisk;
        quint16 nInternalFileAttributes;
        quint32 nExternalFileAttributes;
        quint32 nOffsetToLocalFileHeader;
        // File name
        // Extra field
        // File Comment
    };

    /**
     * ZIP AES Extra Field Structure (Header ID: 0x9901)
     * Used in ZIP archives to indicate AES encryption
     * Total size: 11 bytes (4 byte header + 7 byte data)
     */
    struct AES_EXTRA_FIELD {
        quint16 nHeaderID;           // Extra field header ID (0x9901)
        quint16 nDataSize;           // Data size (typically 7 bytes)
        quint16 nAESVersion;         // AES version number (vendor-specific)
        quint16 nVendorID;           // 2-character vendor ID (e.g., 'AE' = 0x4145)
        quint8 nEncryptionMode;      // AES encryption strength:
                                     // 0x01 = 128-bit
                                     // 0x02 = 192-bit
                                     // 0x03 = 256-bit
        quint16 nCompressionMethod;  // Actual compression method used
    };
#pragma pack(pop)

    struct ZIPFILE_RECORD {
        QString sFileName;
        quint8 nVersion;
        quint8 nOS;
        quint8 nMinVersion;
        quint8 nMinOS;
        quint16 nFlags;
        CMETHOD method;
        QDateTime dtTime;
        quint32 nCRC32;
        qint64 nCompressedSize;
        qint64 nUncompressedSize;
        qint64 nHeaderOffset;
        qint64 nDataOffset;
        quint32 nExternalFileAttributes;
        // TODO Comment!!!
    };

    struct ZIP_PACK_CONTEXT {
        QList<ZIPFILE_RECORD> *pListZipFileRecords;
        QIODevice *pDevice;
        qint64 nStartOffset;
        qint64 nCurrentOffset;
        qint32 nNumberOfRecords;
        bool bFailed;
    };

    // Format-specific context structures
    struct ZIP_UNPACK_CONTEXT {
        bool bIsECD;
        qint64 nCentralDirectoryOffset;
        qint64 nCentralDirectoryEnd;
    };

    explicit XZip(QIODevice *pDevice = nullptr);
    virtual bool isValid(PDSTRUCT *pPdStruct = nullptr) override;
    static bool isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct = nullptr);
    virtual QString getVersion() override;
    virtual bool isEncrypted() override;
    virtual bool isCommentPresent() override;
    virtual QString getComment() override;
    virtual QString getCompressMethodString() override;

    virtual FT getFileType() override;

    static FT _getFileType(QIODevice *pDevice, QList<XArchive::RECORD> *pListRecords, bool bDeep, PDSTRUCT *pPdStruct);

    static bool addLocalFileRecord(QIODevice *pSource, QIODevice *pDest, ZIPFILE_RECORD *pZipFileRecord, PDSTRUCT *pPdStruct = nullptr);
    static bool addCentralDirectory(QIODevice *pDest, QList<ZIPFILE_RECORD> *pListZipFileRecords, const QString &sComment = "", PDSTRUCT *pPdStruct = nullptr,
                                    qint64 nStartPosition = -1);

    virtual QString getFileFormatExt() override;
    virtual QString getFileFormatExtsString() override;
    virtual qint64 getFileFormatSize(PDSTRUCT *pPdStruct) override;
    virtual QString getMIMEString() override;

    CENTRALDIRECTORYFILEHEADER read_CENTRALDIRECTORYFILEHEADER(qint64 nOffset, PDSTRUCT *pPdStruct);
    LOCALFILEHEADER read_LOCALFILEHEADER(qint64 nOffset, PDSTRUCT *pPdStruct);
    AES_EXTRA_FIELD read_AES_EXTRA_FIELD(qint64 nOffset, PDSTRUCT *pPdStruct);
    qint64 findECDOffset(PDSTRUCT *pPdStruct);

    bool isAPK(qint64 nECDOffset, PDSTRUCT *pPdStruct);
    bool isIPA(qint64 nECDOffset, PDSTRUCT *pPdStruct);
    bool isJAR(qint64 nECDOffset, PDSTRUCT *pPdStruct);

    virtual QString structIDToString(quint32 nID) override;
    virtual QString structIDToFtString(quint32 nID) override;
    virtual quint32 ftStringToStructID(const QString &sFtString) override;
    // virtual QList<DATA_HEADER> getDataHeaders(const DATA_HEADERS_OPTIONS &dataHeadersOptions, PDSTRUCT *pPdStruct) override;
    virtual QList<XFHEADER> getXFHeaders(const XFSTRUCT &xfStruct, PDSTRUCT *pPdStruct) override;
    virtual QList<XFRECORD> getXFRecords(FT fileType, quint32 nStructID, const XLOC &xLoc) override;
    virtual QList<FPART> getFileParts(quint32 nFileParts, qint32 nLimit = -1, PDSTRUCT *pPdStruct = nullptr) override;

    // virtual qint32 readTableRow(qint32 nRow, LT locType, XADDR nLocation, const DATA_RECORDS_OPTIONS &dataRecordsOptions, QList<DATA_RECORD_ROW> *pListDataRecords,
    //                             void *pUserData, PDSTRUCT *pPdStruct);

    virtual QList<MAPMODE> getMapModesList() override;
    virtual _MEMORY_MAP getMemoryMap(MAPMODE mapMode = MAPMODE_UNKNOWN, PDSTRUCT *pPdStruct = nullptr) override;

    virtual bool initPack(PACK_STATE *pState, QIODevice *pDevice, const QMap<PACK_PROP, QVariant> &mapProperties, PDSTRUCT *pPdStruct = nullptr) override;
    virtual bool addFile(PACK_STATE *pState, const QString &sFilePath, PDSTRUCT *pPdStruct = nullptr) override;
    virtual bool addFolder(PACK_STATE *pState, const QString &sDirectoryPath, PDSTRUCT *pPdStruct = nullptr) override;
    virtual bool finishPack(PACK_STATE *pState, PDSTRUCT *pPdStruct = nullptr) override;
    virtual QList<PM_INFO> unpackImplemented() override;
    virtual QList<PM_INFO> packImplemented() override;

    virtual QMap<UNPACK_PROP, QVariant> getDefaultUnpackProperties() override;
    virtual bool initUnpack(UNPACK_STATE *pState, const QMap<UNPACK_PROP, QVariant> &mapProperties, PDSTRUCT *pPdStruct = nullptr) override;
    virtual ARCHIVERECORD infoCurrent(UNPACK_STATE *pState, PDSTRUCT *pPdStruct = nullptr) override;
    virtual bool moveToNext(UNPACK_STATE *pState, PDSTRUCT *pPdStruct = nullptr) override;
    virtual bool finishUnpack(UNPACK_STATE *pState, PDSTRUCT *pPdStruct = nullptr) override;
    virtual QList<FPART_PROP> getAvailableFPARTProperties() override;

    virtual QVariant calculateHash(QIODevice *pDevice, PDSTRUCT *pPdStruct = nullptr) override;

    static QMap<quint64, QString> getHeaderSignatures();
    static QMap<quint64, QString> getHeaderSignaturesS();

    static const QString PREFIX_SIGNATURE;

    static quint32 filePermissionsToExternalAttributes(QFile::Permissions permissions);
    static QFile::Permissions externalAttributesToFilePermissions(quint32 nExternalAttributes);

protected:
    HANDLE_METHOD zipToCompressMethod(quint16 nZipMethod, quint32 nFlags);
    bool _readFileName(qint64 nFileNameOffset, qint64 nFileNameLength, quint16 nFlags, qint64 nExtraFieldOffset, qint64 nExtraFieldLength, QString *pFileName);
    bool _isRecordNamePresent(qint64 nECDOffset, QString sRecordName1, QString sRecordName2, PDSTRUCT *pPdStruct, bool bStartWith, bool bRequireNonEmpty = false);
    qint32 _getNumberOfLocalFileHeaders(qint64 nOffset, qint64 nSize, qint64 *pnRealSize, PDSTRUCT *pPdStruct);
    bool _isECDSignaturePresent(qint64 nOffset, PDSTRUCT *pPdStruct);

private:
    INTERNAL_INFO m_internalInfo;
};

#endif  // XZIP_H
