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
#ifndef XNPM_H
#define XNPM_H

#include "xtar_gz.h"

class XNPM : public XTAR_GZ {
    Q_OBJECT

public:
    struct INTERNAL_INFO : XTAR_GZ::INTERNAL_INFO {};

    bool handleInternalInfo(PDSTRUCT *pPdStruct) override;
    void *getInternalInfo(PDSTRUCT *pPdStruct) override;
    void setInternalInfo(void *pInternalInfo) override;

    enum STRUCTID : qint32 {
        STRUCTID_UNKNOWN = 0,
    };

    enum TYPE : qint32 {
        TYPE_UNKNOWN = 0,
        TYPE_PACKAGE,
        // TODO more
    };

    explicit XNPM(QIODevice *pDevice = nullptr);

    virtual bool isValid(PDSTRUCT *pPdStruct = nullptr);
    static bool isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct = nullptr);
    static bool isValid(QList<RECORD> *pListRecords, PDSTRUCT *pPdStruct);
    static bool isValid(QIODevice *pDevice, QList<RECORD> *pListRecords, PDSTRUCT *pPdStruct);

    virtual QString getFileFormatExt();
    virtual FT getFileType();

    virtual FILEFORMATINFO getFileFormatInfo(PDSTRUCT *pPdStruct);
    virtual MODE getMode();
    virtual QString getArch();
    virtual qint32 getType();
    virtual QString typeIdToString(qint32 nType);
    virtual QString getMIMEString();
    virtual QString structIDToString(quint32 nID);
    virtual QString structIDToFtString(quint32 nID);
    virtual quint32 ftStringToStructID(const QString &sFtString);

private:
    INTERNAL_INFO m_internalInfo;
};

#endif  // XNPM_H
