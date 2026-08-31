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
#ifndef XAPK_H
#define XAPK_H

#include "xjar.h"
#include "xandroidbinary.h"

class XAPK : public XJAR {
    Q_OBJECT

public:
    struct INTERNAL_INFO : XJAR::INTERNAL_INFO {};

    bool handleInternalInfo(PDSTRUCT *pPdStruct) override;
    void *getInternalInfo(PDSTRUCT *pPdStruct) override;
    void setInternalInfo(void *pInternalInfo) override;

    enum TYPE {
        TYPE_UNKNOWN = 0,
        TYPE_PACKAGE,
        // TODO more
    };

    explicit XAPK(QIODevice *pDevice = nullptr);
    virtual bool isValid(PDSTRUCT *pPdStruct = nullptr) override;
    static bool isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct = nullptr);
    static bool isValid(QList<RECORD> *pListRecords, PDSTRUCT *pPdStruct);
    static bool isValid(QIODevice *pDevice, QList<RECORD> *pListRecords, PDSTRUCT *pPdStruct);

    virtual FT getFileType() override;
    virtual FILEFORMATINFO getFileFormatInfo(PDSTRUCT *pPdStruct) override;
    virtual QString getFileFormatExt() override;
    virtual MODE getMode() override;
    virtual QString getArch() override;
    virtual qint32 getType() override;
    virtual QString typeIdToString(qint32 nType) override;
    virtual bool isSigned() override;
    virtual OFFSETSIZE getSignOffsetSize() override;

    bool isAPKSignBlockPresent();  // For APK Check jar

    struct APK_SIG_BLOCK_RECORD {
        quint32 nID;
        quint64 nDataOffset;
        quint64 nDataSize;
    };

    QList<APK_SIG_BLOCK_RECORD> getAPKSignaturesBlockRecordsList();
    static bool isAPKSignatureBlockRecordPresent(QList<APK_SIG_BLOCK_RECORD> *pList, quint32 nID);
    static APK_SIG_BLOCK_RECORD getAPKSignatureBlockRecord(QList<APK_SIG_BLOCK_RECORD> *pList, quint32 nID);

private:
    qint64 findAPKSignBlockOffset(PDSTRUCT *pPdStruct = nullptr);

private:
    INTERNAL_INFO m_internalInfo;
};

#endif  // XAPK_H
