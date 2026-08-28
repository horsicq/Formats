/* Copyright (c) 2026 hors<horsicq@gmail.com>
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
#ifndef XWASM_H
#define XWASM_H

#include "xbinary.h"

class XWASM : public XBinary {
    Q_OBJECT

public:
    enum SECTION_ID : quint8 {
        SECTION_ID_CUSTOM = 0,
        SECTION_ID_TYPE = 1,
        SECTION_ID_IMPORT = 2,
        SECTION_ID_FUNCTION = 3,
        SECTION_ID_TABLE = 4,
        SECTION_ID_MEMORY = 5,
        SECTION_ID_GLOBAL = 6,
        SECTION_ID_EXPORT = 7,
        SECTION_ID_START = 8,
        SECTION_ID_ELEMENT = 9,
        SECTION_ID_CODE = 10,
        SECTION_ID_DATA = 11,
        SECTION_ID_DATA_COUNT = 12,
        SECTION_ID_TAG = 13
    };

    struct SECTION_RECORD {
        quint8 nID;
        qint64 nOffset;
        qint64 nHeaderSize;
        qint64 nPayloadOffset;
        quint32 nPayloadSize;
    };

    explicit XWASM(QIODevice *pDevice = nullptr);
    virtual ~XWASM();

    virtual bool isValid(PDSTRUCT *pPdStruct = nullptr) override;
    static bool isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct = nullptr);

    virtual QString getArch() override;
    virtual MODE getMode() override;
    virtual ENDIAN getEndian() override;
    virtual FT getFileType() override;
    virtual QString getVersion() override;
    virtual QString getFileFormatExt() override;
    virtual QString getFileFormatExtsString() override;
    virtual qint64 getFileFormatSize(PDSTRUCT *pPdStruct = nullptr) override;
    virtual QString getMIMEString() override;
    virtual _MEMORY_MAP getMemoryMap(MAPMODE mapMode = MAPMODE_UNKNOWN, PDSTRUCT *pPdStruct = nullptr) override;
    virtual QList<FPART> getFileParts(quint32 nFileParts, qint32 nLimit = -1, PDSTRUCT *pPdStruct = nullptr) override;

    QList<SECTION_RECORD> getSections(PDSTRUCT *pPdStruct = nullptr);
    static QString sectionIdToString(quint8 nID);

private:
    bool _hasValidHeader();
    bool _readULEB128(qint64 *pnOffset, quint32 *pnValue);
    QList<SECTION_RECORD> _getSections(bool *pbValid, PDSTRUCT *pPdStruct);
};

#endif  // XWASM_H
