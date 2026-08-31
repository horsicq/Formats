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
#ifndef XMACHOFAT_H
#define XMACHOFAT_H

#include "xarchive.h"
#include "xmach.h"

class XMACHOFat : public XArchive {
    Q_OBJECT

public:
    struct INTERNAL_INFO : XArchive::INTERNAL_INFO {};

    bool handleInternalInfo(PDSTRUCT *pPdStruct) override;
    void *getInternalInfo(PDSTRUCT *pPdStruct) override;
    void setInternalInfo(void *pInternalInfo) override;

    /*!
        \brief XMACHOFat class for handling Universal Mach-O (fat) binary files
        Universal Mach-O files contain multiple architecture-specific Mach-O binaries
        in a single file, allowing for multi-architecture support on macOS.
    */
    enum STRUCTID {
        STRUCTID_UNKNOWN = 0,
        STRUCTID_HEADER,
        STRUCTID_ARCHITECTURE
    };

    enum TYPE {
        TYPE_UNKNOWN = 0,
        TYPE_BUNDLE,
    };

    explicit XMACHOFat(QIODevice *pDevice = nullptr);

    virtual bool isValid(PDSTRUCT *pPdStruct = nullptr) override;
    static bool isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct = nullptr);
    virtual ENDIAN getEndian() override;
    virtual quint64 getNumberOfRecords(PDSTRUCT *pPdStruct) override;
    virtual QList<RECORD> getRecords(qint32 nLimit, PDSTRUCT *pPdStruct) override;

    virtual OSNAME getOsName() override;
    virtual QString getFileFormatExt() override;
    virtual QString getFileFormatExtsString() override;
    virtual qint64 getFileFormatSize(PDSTRUCT *pPdStruct) override;
    virtual QList<MAPMODE> getMapModesList() override;
    virtual _MEMORY_MAP getMemoryMap(MAPMODE mapMode = MAPMODE_UNKNOWN, PDSTRUCT *pPdStruct = nullptr) override;
    virtual QString getArch() override;
    virtual qint32 getType() override;
    virtual QString typeIdToString(qint32 nType) override;
    virtual FT getFileType() override;
    virtual QString getMIMEString() override;
    virtual bool isArchive() override;
    virtual QString structIDToString(quint32 nID) override;
    virtual QString structIDToFtString(quint32 nID) override;
    virtual quint32 ftStringToStructID(const QString &sFtString) override;

    virtual QList<XFHEADER> getXFHeaders(const XFSTRUCT &xfStruct, PDSTRUCT *pPdStruct) override;
    virtual QList<XFRECORD> getXFRecords(FT fileType, quint32 nStructID, const XLOC &xLoc) override;

    virtual QMap<UNPACK_PROP, QVariant> getDefaultUnpackProperties() override;
    virtual bool initUnpack(UNPACK_STATE *pState, const QMap<UNPACK_PROP, QVariant> &mapProperties, PDSTRUCT *pPdStruct = nullptr) override;
    virtual ARCHIVERECORD infoCurrent(UNPACK_STATE *pState, PDSTRUCT *pPdStruct = nullptr) override;
    virtual bool moveToNext(UNPACK_STATE *pState, PDSTRUCT *pPdStruct = nullptr) override;
    virtual bool finishUnpack(UNPACK_STATE *pState, PDSTRUCT *pPdStruct = nullptr) override;

    XMACH_DEF::fat_header read_fat_header();
    XMACH_DEF::fat_arch read_fat_arch(qint32 nIndex);
    QList<XMACH_DEF::fat_arch> read_fat_arch_list(PDSTRUCT *pPdStruct);
    XMACH_DEF::fat_arch_64 read_fat_arch_64(qint32 nIndex);
    QList<XMACH_DEF::fat_arch_64> read_fat_arch_64_list(PDSTRUCT *pPdStruct);

    static QMap<quint64, QString> getHeaderMagics();
    static QMap<quint64, QString> getHeaderMagicsS();

    QString getArchitectureString(qint32 nIndex);
    qint64 getArchitectureOffset(qint32 nIndex);
    qint64 getArchitectureSize(qint32 nIndex);
    bool isArchitectureValid(qint32 nIndex);
    virtual QList<QString> getSearchSignatures() override;
    virtual XBinary *createInstance(QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1) override;

private:
    struct ARCHITECTURE_RECORD {
        quint32 cputype;
        quint32 cpusubtype;
        quint64 offset;
        quint64 size;
        quint32 align;
        quint32 reserved;
    };

    bool _getFatFormat(bool *pbIs64, bool *pbIsBigEndian);
    qint64 _getArchitectureRecordSize();
    bool _readArchitectureRecord(qint32 nIndex, ARCHITECTURE_RECORD *pRecord);
    bool _isArchitectureRangeValid(const ARCHITECTURE_RECORD &record);
    quint32 _getValidatedArchitectureCount(PDSTRUCT *pPdStruct, bool bValidateRanges);

    INTERNAL_INFO m_internalInfo;
};

#endif  // XMACHOFAT_H
