/* Copyright (c) 2020-2026 hors<horsicq@gmail.com>
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
#ifndef XCOM_H
#define XCOM_H

#include "xbinary.h"
#include "xcom_def.h"

class XCOM : public XBinary {
    Q_OBJECT

public:
    enum TYPE {
        TYPE_UNKNOWN = 0,
        TYPE_EXECUTABLE
    };

    enum STRUCTID {
        STRUCTID_UNKNOWN = 0,
        STRUCTID_HEADER,
    };

    explicit XCOM(QIODevice *pDevice = nullptr, bool bIsImage = false, XADDR nModuleAddress = -1);

    bool isValid(PDSTRUCT *pPdStruct = nullptr) override;
    static bool isValid(QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1, PDSTRUCT *pPdStruct = nullptr);
    static MODE getMode(QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1);

    bool isExecutable() override;

    QList<MAPMODE> getMapModesList() override;
    _MEMORY_MAP getMemoryMap(XBinary::MAPMODE mapMode = XBinary::MAPMODE_UNKNOWN, PDSTRUCT *pPdStruct = nullptr) override;
    QString getArch() override;
    MODE getMode() override;
    ENDIAN getEndian() override;
    qint64 getImageSize() override;
    FT getFileType() override;
    QString getVersion() override;
    qint32 getType() override;
    OSNAME getOsName() override;
    QString typeIdToString(qint32 nType) override;
    QString getMIMEString() override;
    QString getFileFormatExt() override;
    QString getFileFormatExtsString() override;
    qint64 getFileFormatSize(PDSTRUCT *pPdStruct = nullptr) override;

    QString structIDToString(quint32 nID) override;
    QString structIDToFtString(quint32 nID) override;
    quint32 ftStringToStructID(const QString &sFtString) override;
    // QList<DATA_HEADER> getDataHeaders(const DATA_HEADERS_OPTIONS &dataHeadersOptions, PDSTRUCT *pPdStruct) override;
    QList<FPART> getFileParts(quint32 nFileParts, qint32 nLimit = -1, PDSTRUCT *pPdStruct = nullptr) override;
};

#endif  // XCOM_H
