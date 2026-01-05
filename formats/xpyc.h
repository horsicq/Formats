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

    struct CODE_OBJECT {
        QString sName;              // co_name
        QString sFileName;          // co_filename
        qint32 nFirstLineNo;        // co_firstlineno
        qint32 nArgCount;           // co_argcount
        qint32 nPosOnlyArgCount;    // co_posonlyargcount (3.8+)
        qint32 nKwOnlyArgCount;     // co_kwonlyargcount
        qint32 nNLocals;            // co_nlocals
        qint32 nStackSize;          // co_stacksize
        qint32 nFlags;              // co_flags
        QByteArray baCode;          // co_code (bytecode instructions)
        QList<QString> listConsts;  // co_consts (simplified)
        QList<QString> listNames;   // co_names
        QList<QString> listVarNames;// co_varnames
        QList<QString> listFreeVars;// co_freevars
        QList<QString> listCellVars;// co_cellvars
        bool bValid;                // parsing success flag
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
    virtual qint64 getFileFormatSize(PDSTRUCT *pPdStruct = nullptr) override;
    virtual QString getMIMEString() override;
    virtual QString structIDToString(quint32 nID) override;

    INFO getInternalInfo(PDSTRUCT *pPdStruct = nullptr);
    CODE_OBJECT getCodeObject(PDSTRUCT *pPdStruct = nullptr);
    virtual _MEMORY_MAP getMemoryMap(MAPMODE mapMode = MAPMODE_UNKNOWN, PDSTRUCT *pPdStruct = nullptr) override;
    virtual QList<FPART> getFileParts(quint32 nFileParts, qint32 nLimit = -1, PDSTRUCT *pPdStruct = nullptr) override;
    virtual QList<DATA_HEADER> getDataHeaders(const DATA_HEADERS_OPTIONS &dataHeadersOptions, PDSTRUCT *pPdStruct = nullptr) override;

private:
    static QString _magicToVersion(quint16 nMagicValue);
    static bool _isMagicKnown(quint16 nMagicValue);
    static void _parseVersionNumbers(const QString &sVersion, qint32 *pnMajor, qint32 *pnMinor);
};

#endif  // XPYC_H
