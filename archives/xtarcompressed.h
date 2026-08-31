/* Copyright (c) 2026 hors<horsicq@gmail.com>
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
#ifndef XTARCOMPRESSED_H
#define XTARCOMPRESSED_H

#include "xtar.h"

class XTARCOMPRESSED : public XTAR {
    Q_OBJECT

public:
    struct INTERNAL_INFO : XTAR::INTERNAL_INFO {};

    bool handleInternalInfo(PDSTRUCT *pPdStruct) override;
    void *getInternalInfo(PDSTRUCT *pPdStruct) override;
    void setInternalInfo(void *pInternalInfo) override;

    enum COMPRESSION_TYPE {
        COMPRESSION_UNKNOWN = 0,
        COMPRESSION_GZIP,      // .tar.gz, .tgz
        COMPRESSION_BZIP2,     // .tar.bz2, .tbz, .tbz2, .tb2, .tz2
        COMPRESSION_XZ,        // .tar.xz, .txz
        COMPRESSION_LZMA,      // .tar.lzma, .tlz
        COMPRESSION_ZSTD,      // .tar.zst, .tzst
        COMPRESSION_COMPRESS,  // .tar.Z, .taz, .tZ, .taZ
        COMPRESSION_LZIP,      // .tar.lz
        COMPRESSION_LZOP,      // .tar.lzo
        COMPRESSION_LZ4        // .tar.lz4, .tlz4
    };

public:
    explicit XTARCOMPRESSED(QIODevice *pDevice = nullptr);
    virtual ~XTARCOMPRESSED();

    virtual bool isValid(PDSTRUCT *pPdStruct = nullptr) override;
    static bool isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct = nullptr);
    static COMPRESSION_TYPE detectCompressionType(QIODevice *pDevice);
    static XArchive *getCompressionClassInstance(COMPRESSION_TYPE compressionType, QIODevice *pDevice);

    virtual QMap<UNPACK_PROP, QVariant> getDefaultUnpackProperties() override;
    virtual bool initUnpack(UNPACK_STATE *pState, const QMap<UNPACK_PROP, QVariant> &mapProperties, PDSTRUCT *pPdStruct = nullptr) override;
    virtual ARCHIVERECORD infoCurrent(UNPACK_STATE *pState, PDSTRUCT *pPdStruct = nullptr) override;
    virtual QIODevice *getRecordStreamDevice(UNPACK_STATE *pState) override;
    virtual bool moveToNext(UNPACK_STATE *pState, PDSTRUCT *pPdStruct = nullptr) override;
    virtual bool unpackCurrent(UNPACK_STATE *pState, QIODevice *pDevice, PDSTRUCT *pPdStruct = nullptr) override;
    virtual bool finishUnpack(UNPACK_STATE *pState, PDSTRUCT *pPdStruct = nullptr) override;

protected:
    QPointer<QIODevice> m_pDecompressedData;
    QPointer<QIODevice> m_pOriginalDevice;
    COMPRESSION_TYPE m_compressionType;

    // Outer (compressed) stream location in the original file, populated during initUnpack
    // by getOuterStreamInfo(). Used to build solid-block ARCHIVERECORD in infoCurrent().
    qint64 m_nOuterStreamOffset;
    qint64 m_nOuterStreamSize;
    HANDLE_METHOD m_outerHandleMethod;
    qint64 m_nMaterializedOutputLimit;

    // Override in derived classes to provide the outer compressed-stream location.
    // Returns false when the information is not available (keeps legacy zero behaviour).
    virtual bool getOuterStreamInfo(qint64 &nOuterStreamOffset, qint64 &nOuterStreamSize, HANDLE_METHOD &handleMethod);

    // Virtual method for derived classes to implement decompression
    virtual QIODevice *decompressData(PDSTRUCT *pPdStruct) = 0;

    // Shared helper: unpack source stream with XDecompress into memory.
    QIODevice *decompressByMethod(HANDLE_METHOD handleMethod, qint64 nOffset, qint64 nSize, PDSTRUCT *pPdStruct);

    // Utility methods
    static QIODevice *createMemoryBuffer(const QByteArray &baData);

    // The solid-block record shape - real coordinates over the container's one
    // compressed stream, plus the ISSOLID/SOLIDFOLDERINDEX/STREAMUNPACKEDSIZE
    // decode contract - is how this class actually decodes a member of an
    // outer-stream container (.tar.gz).  Those coordinates are the WHOLE
    // stream, not the member's own bytes, so a published record must never
    // carry them: forge its method field and the container's raw bytes come
    // back at the member's declared length.  infoCurrent() therefore builds
    // that shape only while this class's own unpackCurrent() holds the
    // authorization, and publishes an index-paired archive-stream record to
    // everyone else.  Decoding is unchanged; only who may see the coordinates
    // is.
    bool isSolidRecordAuthorized() const;

private:
    INTERNAL_INFO m_internalInfo;
};

#endif  // XTARCOMPRESSED_H
