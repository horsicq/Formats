/* Copyright (c) 2025-2026 hors<horsicq@gmail.com>
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
#ifndef XPYC_H
#define XPYC_H

#include "xbinary.h"

class XPYC : public XBinary {
    Q_OBJECT

public:
    enum STRUCTID {
        STRUCTID_UNKNOWN = 0,
        STRUCTID_HEADER,
        STRUCTID_FLAGS,
        STRUCTID_HASH,
        STRUCTID_TIMESTAMP,
        STRUCTID_SOURCESIZE,
        STRUCTID_CODEOBJECT
    };

    struct INFO {
        quint16 nMagicValue;
        quint16 nMagicMarker;
        QString sVersion;
        quint32 nFlags;
        bool bHashBased;
        quint32 nTimestamp;
        quint32 nSourceSize;
        QByteArray baHash;
    };

    explicit XPYC(QIODevice *pDevice = nullptr);
    virtual ~XPYC();

    virtual bool isValid(PDSTRUCT *pPdStruct = nullptr) override;
    static bool isValid(QIODevice *pDevice);

    virtual QString getArch() override;
    virtual MODE getMode() override;
    virtual ENDIAN getEndian() override;
    virtual FT getFileType() override;
    virtual QString getVersion() override;
    virtual QString getFileFormatExt() override;
    virtual QString getFileFormatExtsString() override;
    virtual QString getMIMEString() override;
    virtual QString structIDToString(quint32 nID) override;

    INFO getInternalInfo(PDSTRUCT *pPdStruct = nullptr);
    virtual _MEMORY_MAP getMemoryMap(MAPMODE mapMode = MAPMODE_UNKNOWN, PDSTRUCT *pPdStruct = nullptr) override;
    virtual QList<FPART> getFileParts(quint32 nFileParts, qint32 nLimit = -1, PDSTRUCT *pPdStruct = nullptr) override;
    virtual QList<DATA_HEADER> getDataHeaders(const DATA_HEADERS_OPTIONS &dataHeadersOptions, PDSTRUCT *pPdStruct = nullptr) override;

private:
    static QString _magicToVersion(quint16 nMagicValue);
    static bool _isMagicKnown(quint16 nMagicValue);
    static void _parseVersionNumbers(const QString &sVersion, qint32 *pnMajor, qint32 *pnMinor);
};

#endif  // XPYC_H
