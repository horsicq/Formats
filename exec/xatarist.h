/* Copyright (c) 2020-2025 hors<horsicq@gmail.com>
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
#ifndef XATARIST_H
#define XATARIST_H

#include "xbinary.h"
#include "xatarist_def.h"

class XAtariST : public XBinary {
    Q_OBJECT

public:
    enum STRUCTID {
        STRUCTID_UNKNOWN = 0,
        STRUCTID_HEADER
    };
    enum TYPE {
        TYPE_UNKNOWN = 0,
        TYPE_EXECUTABLE
    };

    explicit XAtariST(QIODevice *pDevice = nullptr, bool bIsImage = false, XADDR nModuleAddress = -1);
    ~XAtariST();

    virtual bool isValid(PDSTRUCT *pPdStruct = nullptr) override;
    static bool isValid(QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1);
    static MODE getMode(QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1);

    virtual bool isExecutable() override;

    XATARIST_DEF::HEADER getHeader();

    virtual bool _initMemoryMap(_MEMORY_MAP *pMemoryMap, PDSTRUCT *pPdStruct) override;
    virtual _MEMORY_MAP getMemoryMap(XBinary::MAPMODE mapMode = XBinary::MAPMODE_UNKNOWN, PDSTRUCT *pPdStruct = nullptr) override;
    virtual QList<MAPMODE> getMapModesList() override;
    virtual QList<FPART> getFileParts(quint32 nFileParts, qint32 nLimit = -1, PDSTRUCT *pPdStruct = nullptr) override;
    virtual QString getArch() override;
    virtual MODE getMode() override;
    virtual ENDIAN getEndian() override;
    virtual qint64 getImageSize() override;
    virtual FT getFileType() override;
    virtual qint32 getType() override;
    virtual OSNAME getOsName() override;
    virtual QString structIDToString(quint32 nID) override;
    virtual QString typeIdToString(qint32 nType) override;
    virtual QString getFileFormatExtsString() override;
};

#endif  // XATARIST_H
