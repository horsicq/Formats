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
#ifndef XJFIF_H
#define XJFIF_H

#include "xbinary.h"

class XJFIF : public XBinary {
    Q_OBJECT

public:
    enum DENSITY_UNIT {
        DENSITY_UNIT_NONE = 0,
        DENSITY_UNIT_PIXELS_PER_INCH = 1,
        DENSITY_UNIT_PIXELS_PER_CENTIMETER = 2
    };

    struct HEADER {
        quint8 nVersionMajor;
        quint8 nVersionMinor;
        quint8 nDensityUnit;
        quint16 nXDensity;
        quint16 nYDensity;
        quint8 nThumbnailWidth;
        quint8 nThumbnailHeight;
    };

    explicit XJFIF(QIODevice *pDevice = nullptr);
    ~XJFIF();

    virtual bool isValid(PDSTRUCT *pPdStruct = nullptr) override;
    static bool isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct = nullptr);
    virtual qint64 getFileFormatSize(PDSTRUCT *pPdStruct = nullptr) override;
    virtual QString getVersion() override;

    HEADER getHeader();
    OFFSETSIZE getThumbnail();
    QString getDensityUnitString();

    virtual QVector<XMETADATA_STRUCT> getMetadataStructs() override;
};

#endif  // XJFIF_H
