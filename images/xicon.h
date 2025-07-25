/* Copyright (c) 2022-2025 hors<horsicq@gmail.com>
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
#ifndef XICON_H
#define XICON_H

#include "xbinary.h"

class XIcon : public XBinary {
    Q_OBJECT

public:
#pragma pack(push)
#pragma pack(1)
    struct ICONDIR {
        quint16 idReserved;
        quint16 idType;
        quint16 idCount;
    };

    struct ICONDIRENTRY {
        quint8 bWidth;
        quint8 bHeight;
        quint8 bColorCount;
        quint8 bReserved;
        quint16 wPlanes;
        quint16 wBitCount;
        quint32 dwBytesInRes;
        quint32 dwImageOffset;
    };

    struct GRPICONDIRENTRY {
        quint8 bWidth;
        quint8 bHeight;
        quint8 bColorCount;
        quint8 bReserved;
        quint16 wPlanes;
        quint16 wBitCount;
        quint32 dwBytesInRes;
        quint16 nID;
    };
#pragma pack(pop)
    explicit XIcon(QIODevice *pDevice = nullptr);
    ~XIcon();

    virtual bool isValid(PDSTRUCT *pPdStruct = nullptr);
    static bool isValid(QIODevice *pDevice);
    virtual FT getFileType();
    virtual QString getFileFormatExt();
    virtual QString getFileFormatExtsString();
    virtual qint64 getFileFormatSize(PDSTRUCT *pPdStruct);
    virtual _MEMORY_MAP getMemoryMap(MAPMODE mapMode = MAPMODE_UNKNOWN, PDSTRUCT *pPdStruct = nullptr);

    ICONDIR readICONDIR();
    ICONDIRENTRY readICONDIRENTRY(qint64 nOffset);
    GRPICONDIRENTRY readGPRICONDIRENTRY(qint64 nOffset);
    QList<ICONDIRENTRY> getIconDirectories();
    QList<GRPICONDIRENTRY> getIconGPRDirectories();
    virtual QString getMIMEString();
};

#endif  // XICON_H
