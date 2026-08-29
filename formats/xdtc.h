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
#ifndef XDTC_H
#define XDTC_H

#include "xbinary.h"

class XDTC : public XBinary {
    Q_OBJECT

public:
    struct INTERNAL_INFO : XBinary::INTERNAL_INFO {};

#pragma pack(push, 1)
    struct HEADER {
        char magic[4];
        quint16 nInterfaceVersion;
        quint16 nEntryPointCount;
        quint16 nFormatRevision;
        quint16 nFooterOffset;
        char embeddedMagic[4];
        quint16 nFlags;
        quint8 nReserved[6];
    };
#pragma pack(pop)

    struct LAYOUT {
        HEADER header;
        qint64 nEntryTableOffset;
        qint64 nEntryTableSize;
        qint64 nModuleOffset;
        qint64 nModuleSize;
        qint64 nFooterOffset;
        qint64 nFooterSize;
        qint64 nFileSize;
        QList<quint16> listEntryPoints;
    };

    explicit XDTC(QIODevice *pDevice = nullptr);
    ~XDTC();

    bool handleInternalInfo(PDSTRUCT *pPdStruct) override;
    void *getInternalInfo(PDSTRUCT *pPdStruct) override;
    void setInternalInfo(void *pInternalInfo) override;

    bool isValid(PDSTRUCT *pPdStruct = nullptr) override;
    static bool isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct = nullptr);

    bool isExecutable() override;
    QString getArch() override;
    MODE getMode() override;
    ENDIAN getEndian() override;
    FT getFileType() override;
    QString getVersion() override;
    OSNAME getOsName() override;
    QString getMIMEString() override;
    QString getFileFormatExt() override;
    QString getFileFormatExtsString() override;
    qint64 getFileFormatSize(PDSTRUCT *pPdStruct = nullptr) override;
    qint64 getImageSize() override;
    QList<MAPMODE> getMapModesList() override;
    _MEMORY_MAP getMemoryMap(MAPMODE mapMode = MAPMODE_UNKNOWN, PDSTRUCT *pPdStruct = nullptr) override;
    QVector<XMETADATA_STRUCT> getMetadataStructs() override;
    QList<FPART> getFileParts(quint32 nFileParts, qint32 nLimit = -1, PDSTRUCT *pPdStruct = nullptr) override;
    QList<QString> getSearchSignatures() override;
    XBinary *createInstance(QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1) override;

    HEADER _read_HEADER(qint64 nOffset = 0);
    QList<quint16> getEntryPoints(PDSTRUCT *pPdStruct = nullptr);

private:
    bool _parseLayout(LAYOUT *pLayout, PDSTRUCT *pPdStruct);

    INTERNAL_INFO m_internalInfo;
};

#endif  // XDTC_H
