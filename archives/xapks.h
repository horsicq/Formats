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
#ifndef XAPKS_H
#define XAPKS_H

#include "xapk.h"

class XAPKS : public XAPK {
    Q_OBJECT

public:
    struct INTERNAL_INFO : XAPK::INTERNAL_INFO {};

    bool handleInternalInfo(PDSTRUCT *pPdStruct) override;
    void *getInternalInfo(PDSTRUCT *pPdStruct) override;
    void setInternalInfo(void *pInternalInfo) override;

    explicit XAPKS(QIODevice *pDevice = nullptr);
    ~XAPKS();

    virtual bool isValid(PDSTRUCT *pPdStruct = nullptr) override;
    static bool isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct = nullptr);
    static bool isValid(QList<RECORD> *pListRecords, PDSTRUCT *pPdStruct);
    static bool isValid(QIODevice *pDevice, QList<RECORD> *pListRecords, PDSTRUCT *pPdStruct);

    virtual FT getFileType() override;
    virtual FILEFORMATINFO getFileFormatInfo(PDSTRUCT *pPdStruct) override;
    virtual QString getFileFormatExt() override;
    virtual QString getMIMEString() override;

private:
    INTERNAL_INFO m_internalInfo;
};

#endif  // XAPKS_H
