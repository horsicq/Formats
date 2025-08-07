/* Copyright (c) 2025 hors<horsicq@gmail.com>
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
#ifndef XTTF_H
#define XTTF_H

#include "xbinary.h"

class XTTF : public XBinary {
    Q_OBJECT

public:
    struct TTF_TABLE_RECORD {
        quint32 tag;  // Table name/tag (e.g. 'name', 'cmap', etc.)
        quint32 checkSum;
        quint32 offset;
        quint32 length;
    };

    struct TTF_HEADER {
        quint32 sfntVersion;
        quint16 numTables;
        quint16 searchRange;
        quint16 entrySelector;
        quint16 rangeShift;
    };

    explicit XTTF(QIODevice *pDevice = nullptr);
    virtual ~XTTF();

    // Core overrides
    virtual bool isValid(PDSTRUCT *pPdStruct = nullptr) override;
    virtual FT getFileType() override;
    virtual MODE getMode() override;
    virtual QString getMIMEString() override;
    virtual qint32 getType() override;
    virtual QString typeIdToString(qint32 nType) override;
    virtual ENDIAN getEndian() override;
    virtual QString getArch() override;
    virtual QString getFileFormatExt() override;
    virtual qint64 getFileFormatSize(PDSTRUCT *pPdStruct) override;
    virtual bool isSigned() override;
    virtual OSNAME getOsName() override;
    virtual QString getOsVersion() override;
    virtual QString getVersion() override;
    QString getVersion(QList<TTF_TABLE_RECORD> *pListTables, PDSTRUCT *pPdStruct);
    virtual bool isEncrypted() override;
    virtual QList<MAPMODE> getMapModesList() override;
    virtual _MEMORY_MAP getMemoryMap(MAPMODE mapMode = MAPMODE_UNKNOWN, PDSTRUCT *pPdStruct = nullptr) override;

    virtual QList<HREGION> getNativeRegions(PDSTRUCT *pPdStruct = nullptr) override;
    virtual QList<HREGION> getNativeSubRegions(PDSTRUCT *pPdStruct = nullptr) override;
    virtual QList<HREGION> getHData(PDSTRUCT *pPdStruct = nullptr) override;

    virtual QList<DATA_HEADER> getDataHeaders(const DATA_HEADERS_OPTIONS &dataHeadersOptions, PDSTRUCT *pPdStruct) override;
    virtual QList<QString> getTableTitles(const DATA_RECORDS_OPTIONS &dataRecordsOptions) override;

    // TTF-specific
    static QString tagToString(quint32 tag);

    TTF_HEADER readHeader();
    QList<TTF_TABLE_RECORD> getTableDirectory(quint16 numTables);
};

#endif  // XTTF_H
