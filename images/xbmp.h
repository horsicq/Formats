/* Copyright (c) 2022-2026 hors<horsicq@gmail.com>
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
#ifndef XBMP_H
#define XBMP_H

#include "xbinary.h"

class XBMP : public XBinary {
    Q_OBJECT

public:
    enum STRUCTID {
        STRUCTID_UNKNOWN = 0,
        STRUCTID_BMPFILEHEADER,
        STRUCTID_BMPINFOHEADER
    };

    explicit XBMP(QIODevice *pDevice = nullptr);

    bool isValid(PDSTRUCT *pPdStruct = nullptr) override;
    static bool isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct = nullptr);
    FT getFileType() override;
    QString getMIMEString() override;
    QString getArch() override;
    MODE getMode() override;
    ENDIAN getEndian() override;

    QString getFileFormatExt() override;
    QString getFileFormatExtsString() override;
    qint64 getFileFormatSize(PDSTRUCT *pPdStruct = nullptr) override;
    _MEMORY_MAP getMemoryMap(MAPMODE mapMode = MAPMODE_UNKNOWN, PDSTRUCT *pPdStruct = nullptr) override;
    QString getVersion() override;

    struct BMPFILEHEADER {
        quint16 bfType;
        quint32 bfSize;
        quint16 bfReserved1;
        quint16 bfReserved2;
        quint32 bfOffBits;
    };

    struct BMPINFOHEADER {
        quint32 biSize;
        qint32 biWidth;
        qint32 biHeight;
        quint16 biPlanes;
        quint16 biBitCount;
        quint32 biCompression;
        quint32 biSizeImage;
        qint32 biXPelsPerMeter;
        qint32 biYPelsPerMeter;
        quint32 biClrUsed;
        quint32 biClrImportant;
    };

    BMPFILEHEADER getFileHeader();
    BMPINFOHEADER getInfoHeader();

    QString structIDToString(quint32 nID) override;
    QString structIDToFtString(quint32 nID) override;
    quint32 ftStringToStructID(const QString &sFtString) override;

    QList<DATA_HEADER> getDataHeaders(const DATA_HEADERS_OPTIONS &dataHeadersOptions, PDSTRUCT *pPdStruct) override;

    QList<FPART> getFileParts(quint32 nFileParts, qint32 nLimit = -1, PDSTRUCT *pPdStruct = nullptr) override;
    QList<QString> getSearchSignatures() override;
    XBinary *createInstance(QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1) override;
};

#endif  // XBMP_H
