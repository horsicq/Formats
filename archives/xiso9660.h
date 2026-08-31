/* Copyright (c) 2025-2026 hors<horsicq@gmail.com>
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
#ifndef XISO9660_H
#define XISO9660_H

#include <QFile>
#include <QPointer>

#include "xarchive.h"

class XISO9660 : public XArchive {
    Q_OBJECT

#pragma pack(push)
#pragma pack(1)
    struct ISO9660_PVDESC {
        quint8 nDescType;             // [0]   Descriptor type: 1 for PVD
        char szStandard[5];           // [1]   Standard identifier: "CD001"
        quint8 nDescVersion;          // [6]   Descriptor version
        quint8 nUnused1;              // [7]   Unused
        char szSystemId[32];          // [8]   System identifier
        char szVolumeId[32];          // [40]  Volume identifier
        quint8 nUnused2[8];           // [72]  Unused
        quint32 nVolumeSpaceSizeLE;   // [80]  Total number of logical blocks (LE)
        quint32 nVolumeSpaceSizeBE;   // [84]  Total number of logical blocks (BE)
        quint8 nUnused3[32];          // [88]  Unused
        quint16 nVolumeSetSizeLE;     // [120] Volume set size (LE)
        quint16 nVolumeSetSizeBE;     // [122] Volume set size (BE)
        quint16 nVolumeSeqNumLE;      // [124] Volume sequence number (LE)
        quint16 nVolumeSeqNumBE;      // [126] Volume sequence number (BE)
        quint16 nLogicalBlockSizeLE;  // [128] Logical block size in bytes (LE)
        quint16 nLogicalBlockSizeBE;  // [130] Logical block size in bytes (BE)
        quint32 nPathTableSizeLE;     // [132] Path table size in bytes (LE)
        quint32 nPathTableSizeBE;     // [136] Path table size in bytes (BE)
        quint32 nLPathTableLoc;       // [140] Location of Type-L Path Table
        quint32 nOptLPathTableLoc;    // [144] Location of optional Type-L Path Table
        quint32 nMPathTableLoc;       // [148] Location of Type-M Path Table
        quint32 nOptMPathTableLoc;    // [152] Location of optional Type-M Path Table
        quint8 nRootDirRecord[34];    // [156] Directory Record for Root Directory
        char szVolSetId[128];         // [190] Volume set identifier
        char szPublisherId[128];      // [318] Publisher identifier
        char szDataPreparerId[128];   // [446] Data preparer identifier
        char szApplicationId[128];    // [574] Application identifier
        char szCopyrightFile[37];     // [702] Copyright file identifier
        char szAbstractFile[37];      // [739] Abstract file identifier
        char szBiblioFile[37];        // [776] Bibliographic file identifier
        char szCreationTime[17];      // [813] Creation date/time
        char szModificationTime[17];  // [830] Modification date/time
        char szExpirationTime[17];    // [847] Expiration date/time
        char szEffectiveTime[17];     // [864] Effective date/time
        quint8 nFileStructVersion;    // [881] File structure version
        quint8 nUnused4;              // [882] Unused
        quint8 nAppData[512];         // [883] Application-specific data
        quint8 nUnused5[653];         // [1395] Reserved
    };

    struct ISO9660_DIR_RECORD {
        quint8 nLength;             // Directory record length
        quint8 nExtentAttrLength;   // Extended attribute record length
        quint32 nExtentLocationLE;  // Logical block number (little-endian)
        quint32 nExtentLocationBE;  // Logical block number (big-endian)
        quint32 nDataLengthLE;      // File size (little-endian)
        quint32 nDataLengthBE;      // File size (big-endian)
        quint8 nYear;               // Year (offset from 1900)
        quint8 nMonth;              // Month
        quint8 nDay;                // Day
        quint8 nHour;               // Hour
        quint8 nMinute;             // Minute
        quint8 nSecond;             // Second
        quint8 nGMTOffset;          // GMT offset
        quint8 nFileFlags;          // File flags
        quint8 nFileUnitSize;       // File unit size
        quint8 nInterleaveGapSize;  // Interleave gap size
        quint16 nSequenceNumberLE;  // Volume sequence number (little-endian)
        quint16 nSequenceNumberBE;  // Volume sequence number (big-endian)
        quint8 nFileIdLength;       // Length of file identifier
        // Followed by file identifier and padding
    };
#pragma pack(pop)

public:
    struct INTERNAL_INFO : XArchive::INTERNAL_INFO {};

    bool handleInternalInfo(PDSTRUCT *pPdStruct) override;
    void *getInternalInfo(PDSTRUCT *pPdStruct) override;
    void setInternalInfo(void *pInternalInfo) override;

    virtual QList<QString> getSearchSignatures() override;
    virtual XBinary *createInstance(QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1) override;
    enum STRUCTID {
        STRUCTID_UNKNOWN = 0,
        STRUCTID_PVDESC,
        STRUCTID_DIR_RECORD
    };

    explicit XISO9660(QIODevice *pDevice = nullptr);
    ~XISO9660();

    virtual bool isValid(PDSTRUCT *pPdStruct = nullptr) override;
    static bool isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct = nullptr);
    // CUE sheets and raw CD sectors need the native logical-sector adapter;
    // filename-based ISO handlers only understand contiguous 2048-byte data.
    static bool isCueOrRawImage(QIODevice *pDevice, PDSTRUCT *pPdStruct = nullptr);
    virtual QString getFileFormatExt() override;
    virtual QString getFileFormatExtsString() override;
    virtual qint64 getFileFormatSize(PDSTRUCT *pPdStruct = nullptr) override;
    virtual QString getMIMEString() override;
    virtual FT getFileType() override;
    virtual QList<MAPMODE> getMapModesList() override;
    virtual _MEMORY_MAP getMemoryMap(MAPMODE mapMode = MAPMODE_UNKNOWN, PDSTRUCT *pPdStruct = nullptr) override;
    virtual QString structIDToString(quint32 nID) override;
    virtual QString structIDToFtString(quint32 nID) override;
    virtual quint32 ftStringToStructID(const QString &sFtString) override;
    // virtual QList<DATA_HEADER> getDataHeaders(const DATA_HEADERS_OPTIONS &dataHeadersOptions, PDSTRUCT *pPdStruct) override;
    virtual QList<XFHEADER> getXFHeaders(const XFSTRUCT &xfStruct, PDSTRUCT *pPdStruct) override;
    virtual QList<XFRECORD> getXFRecords(FT fileType, quint32 nStructID, const XLOC &xLoc) override;
    virtual QList<FPART> getFileParts(quint32 nFileParts, qint32 nLimit = -1, PDSTRUCT *pPdStruct = nullptr) override;

    virtual QMap<UNPACK_PROP, QVariant> getDefaultUnpackProperties() override;
    virtual bool initUnpack(UNPACK_STATE *pState, const QMap<UNPACK_PROP, QVariant> &mapProperties, PDSTRUCT *pPdStruct = nullptr) override;
    virtual ARCHIVERECORD infoCurrent(UNPACK_STATE *pState, PDSTRUCT *pPdStruct = nullptr) override;
    virtual bool moveToNext(UNPACK_STATE *pState, PDSTRUCT *pPdStruct = nullptr) override;
    virtual bool finishUnpack(UNPACK_STATE *pState, PDSTRUCT *pPdStruct = nullptr) override;

    ISO9660_PVDESC _readPrimaryVolumeDescriptor(qint64 nOffset);

    QString getSystemIdentifier();
    QString getVolumeIdentifier();
    QString getVolumeSetIdentifier();
    QString getPublisherIdentifier();
    QString getDataPreparerIdentifier();
    QString getApplicationIdentifier();
    QString getCopyrightFileIdentifier();
    QString getAbstractFileIdentifier();
    QString getBibliographicFileIdentifier();
    QString getCreationDateTime();
    QString getModificationDateTime();
    QString getExpirationDateTime();
    QString getEffectiveDateTime();

private:
    struct ISO9660_SCAN_CONTEXT {
        qint32 nLogicalBlockSize;
        qint64 nRootDirOffset;
        qint64 nRootDirSize;
    };

    struct ISO9660_UNPACK_CONTEXT {
        qint32 nLogicalBlockSize;
        QList<ARCHIVERECORD> listAllRecords;  // Flat list of all files/dirs built upfront via BFS
    };

    qint32 _getLogicalBlockSize();
    qint64 _getPrimaryVolumeDescriptorOffset();
    bool _isValidDescriptor(qint64 nOffset, PDSTRUCT *pPdStruct);
    bool _configureLogicalImage(QIODevice *pDevice);
    bool _selectVolumeDescriptor();
    QList<ARCHIVERECORD> _collectAllRecords(qint64 nRootOffset, qint64 nRootSize, qint32 nBlockSize, PDSTRUCT *pPdStruct);
    QList<ARCHIVERECORD> _parseDirectoryEntries(qint64 nOffset, qint64 nSize, qint32 nBlockSize, const QString &sParentPath, PDSTRUCT *pPdStruct);
    QString _cleanFileName(const QString &sFileName);

    QString m_sSystemIdentifier;
    QString m_sVolumeIdentifier;
    QString m_sVolumeSetIdentifier;
    QString m_sPublisherIdentifier;
    QString m_sDataPreparerIdentifier;
    QString m_sApplicationIdentifier;
    QString m_sCopyrightFileIdentifier;
    QString m_sAbstractFileIdentifier;
    QString m_sBibliographicFileIdentifier;
    QString m_sCreationDateTime;
    QString m_sModificationDateTime;
    QString m_sExpirationDateTime;
    QString m_sEffectiveDateTime;
    QPointer<QIODevice> m_pLogicalImageDevice;
    QPointer<QFile> m_pOwnedImageFile;
    qint64 m_nVolumeDescriptorOffset;
    bool m_bJoliet;
    bool m_bCueSource;
    bool m_bRawSectorSource;

private:
    INTERNAL_INFO m_internalInfo;
};

#endif  // XISO9660_H
