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
#ifndef XTAR_H
#define XTAR_H

#include "xarchive.h"
#include "xgzip.h"

class XTAR : public XArchive {
    Q_OBJECT

public:
#pragma pack(push)
#pragma pack(1)
    struct posix_header {   /* byte offset */
        char name[100];     /*   0 */
        char mode[8];       /* 100 */
        char uid[8];        /* 108 */
        char gid[8];        /* 116 */
        char size[12];      /* 124 */
        char mtime[12];     /* 136 */
        char chksum[8];     /* 148 */
        char typeflag[1];   /* 156 */
        char linkname[100]; /* 157 */
        char magic[6];      /* 257 */
        char version[2];    /* 263 */
        char uname[32];     /* 265 */
        char gname[32];     /* 297 */
        char devmajor[8];   /* 329 */
        char devminor[8];   /* 337 */
        char prefix[155];   /* 345 */
                            /* 500 */
    };
#pragma pack(pop)

private:
    enum TAR_FORMAT {
        TAR_FORMAT_DEFAULT = 0,
        TAR_FORMAT_POSIX,  // POSIX ustar format (default)
        TAR_FORMAT_GNU     // GNU tar format (supports longer filenames)
    };

public:
    struct INTERNAL_INFO : XArchive::INTERNAL_INFO {};

    bool handleInternalInfo(PDSTRUCT *pPdStruct) override;
    void *getInternalInfo(PDSTRUCT *pPdStruct) override;
    void setInternalInfo(void *pInternalInfo) override;

    virtual QList<QString> getSearchSignatures() override;
    virtual XBinary *createInstance(QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1) override;
    enum STRUCTID {
        STRUCTID_UNKNOWN = 0,
        STRUCTID_POSIX_HEADER
    };

    explicit XTAR(QIODevice *pDevice = nullptr);

    virtual bool isValid(PDSTRUCT *pPdStruct = nullptr) override;
    bool _isValid(_MEMORY_MAP *pMemoryMap, qint64 nOffset, PDSTRUCT *pPdStruct = nullptr);
    static bool isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct = nullptr);
    virtual QString getFileFormatExt() override;
    virtual QString getFileFormatExtsString() override;
    virtual QList<MAPMODE> getMapModesList() override;
    virtual FT getFileType() override;
    virtual _MEMORY_MAP getMemoryMap(MAPMODE mapMode = MAPMODE_UNKNOWN, PDSTRUCT *pPdStruct = nullptr) override;
    virtual QString getMIMEString() override;
    virtual QList<FPART> getFileParts(quint32 nFileParts, qint32 nLimit = -1, PDSTRUCT *pPdStruct = nullptr) override;
    virtual QString structIDToString(quint32 nID) override;
    virtual QString structIDToFtString(quint32 nID) override;
    virtual quint32 ftStringToStructID(const QString &sFtString) override;
    // virtual QList<DATA_HEADER> getDataHeaders(const DATA_HEADERS_OPTIONS &dataHeadersOptions, PDSTRUCT *pPdStruct) override;
    virtual QList<XFHEADER> getXFHeaders(const XFSTRUCT &xfStruct, PDSTRUCT *pPdStruct) override;
    virtual QList<XFRECORD> getXFRecords(FT fileType, quint32 nStructID, const XLOC &xLoc) override;
    // virtual qint32 readTableRow(qint32 nRow, LT locType, XADDR nLocation, const DATA_RECORDS_OPTIONS &dataRecordsOptions, QList<DATA_RECORD_ROW> *pListDataRecords,
    // void *pUserData, PDSTRUCT *pPdStruct) override;

    // Streaming unpacking API
    virtual QMap<UNPACK_PROP, QVariant> getDefaultUnpackProperties() override;
    virtual bool initUnpack(UNPACK_STATE *pState, const QMap<UNPACK_PROP, QVariant> &mapProperties, PDSTRUCT *pPdStruct = nullptr) override;
    virtual ARCHIVERECORD infoCurrent(UNPACK_STATE *pState, PDSTRUCT *pPdStruct = nullptr) override;
    virtual bool moveToNext(UNPACK_STATE *pState, PDSTRUCT *pPdStruct = nullptr) override;
    virtual bool finishUnpack(UNPACK_STATE *pState, PDSTRUCT *pPdStruct = nullptr) override;
    virtual QList<FPART_PROP> getAvailableFPARTProperties() override;

    // Streaming packing API
    virtual bool initPack(PACK_STATE *pState, QIODevice *pDevice, const QMap<PACK_PROP, QVariant> &mapProperties, PDSTRUCT *pPdStruct = nullptr) override;
    virtual bool addFile(PACK_STATE *pState, const QString &sFileName, PDSTRUCT *pPdStruct = nullptr) override;
    virtual bool addFolder(PACK_STATE *pState, const QString &sDirectoryPath, PDSTRUCT *pPdStruct = nullptr) override;
    virtual bool finishPack(PACK_STATE *pState, PDSTRUCT *pPdStruct = nullptr) override;

protected:
    struct TAR_RECORD {
        posix_header header;
        qint64 nHeaderOffset;
        qint64 nDataOffset;
        qint64 nFileSize;
        qint64 nRecordSize;
        QString sPath;
        QString sLinkPath;
        bool bHasLinkPath;
        QMap<QByteArray, QByteArray> mapMetadata;
    };

    struct UNPACK_CONTEXT {
        QList<TAR_RECORD> listRecords;
        qint64 nArchiveEnd;
    };

private:
    posix_header read_posix_header(qint64 nOffset);
    qint32 _getNumberOf_posix_headers(qint64 nOffset, PDSTRUCT *pPdStruct);
    qint64 _getSize(const posix_header &header);
    static QString _getRecordPath(const posix_header &header);
    static bool _parseNumber(const char *pData, qint32 nSize, qint64 *pValue);
    bool _readHeader(qint64 nOffset, qint64 nTotalSize, posix_header *pHeader, bool *pIsZeroBlock, PDSTRUCT *pPdStruct);
    bool _readRecord(qint64 nOffset, qint64 nTotalSize, posix_header *pHeader, qint64 *pFileSize, qint64 *pRecordSize, bool *pIsZeroBlock, PDSTRUCT *pPdStruct,
                     qint64 nSizeOverride = -1);
    bool _collectRecords(qint64 nOffset, qint64 nTotalSize, QList<TAR_RECORD> *pListRecords, qint64 *pEndOffset, PDSTRUCT *pPdStruct);
    bool _scanArchive(qint64 nOffset, qint64 nTotalSize, qint32 *pNumberOfRecords, qint64 *pEndOffset, PDSTRUCT *pPdStruct);
    static bool createHeader(const QString &sFileName, const QString &sBasePath, qint64 nFileSize, quint32 nMode, qint64 nMTime, posix_header *pHeader);
    static quint32 calculateChecksum(const posix_header &header);
    static bool writeOctal(char *pDest, qint32 nSize, qint64 nValue);

signals:
private:
    INTERNAL_INFO m_internalInfo;
};

#endif  // XTAR_H
