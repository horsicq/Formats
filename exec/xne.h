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
#ifndef XNE_H
#define XNE_H

#include "xmsdos.h"
#include "xne_def.h"

// https://fd.lod.bz/rbil/interrup/dos_kernel/214b.html#table-01596
class XNE : public XMSDOS {
    Q_OBJECT

public:
    enum STRUCTID {
        STRUCTID_UNKNOWN = 0,
        STRUCTID_IMAGE_DOS_HEADER,        // Reuse from base via call
        STRUCTID_IMAGE_DOS_HEADEREX,      // Reuse from base via call
        STRUCTID_IMAGE_OS2_HEADER,
        STRUCTID_ENTRY_TABLE,
        STRUCTID_SEGMENT_TABLE,
        STRUCTID_RESOURCE_TABLE,
        STRUCTID_RESIDENT_NAME_TABLE,
        STRUCTID_MODULE_REFERENCE_TABLE,
        STRUCTID_IMPORTED_NAMES_TABLE,
        STRUCTID_NONRESIDENT_NAME_TABLE
    };
    enum TYPE {
        TYPE_UNKNOWN = 0,
        TYPE_EXE,
        TYPE_DLL,
        TYPE_DRIVER,
        TYPE_FONT
        // TODO Check More
    };

    explicit XNE(QIODevice *pDevice = nullptr, bool bIsImage = false, XADDR nModuleAddress = -1);
    bool isValid(PDSTRUCT *pPdStruct = nullptr) override;
    static bool isValid(QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1, PDSTRUCT *pPdStruct = nullptr);
    static MODE getMode(QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1);

    qint64 getImageOS2HeaderOffset();
    qint64 getImageOS2HeaderSize();

    XNE_DEF::IMAGE_OS2_HEADER getImageOS2Header();

    quint16 getImageOS2Header_magic();
    quint8 getImageOS2Header_ver();
    quint8 getImageOS2Header_rev();
    quint16 getImageOS2Header_enttab();
    quint16 getImageOS2Header_cbenttab();
    quint32 getImageOS2Header_crc();
    quint16 getImageOS2Header_flags();
    quint16 getImageOS2Header_autodata();
    quint16 getImageOS2Header_heap();
    quint16 getImageOS2Header_stack();
    quint32 getImageOS2Header_csip();
    quint32 getImageOS2Header_sssp();
    quint16 getImageOS2Header_cseg();
    quint16 getImageOS2Header_cmod();
    quint16 getImageOS2Header_cbnrestab();
    quint16 getImageOS2Header_segtab();
    quint16 getImageOS2Header_rsrctab();
    quint16 getImageOS2Header_restab();
    quint16 getImageOS2Header_modtab();
    quint16 getImageOS2Header_imptab();
    quint32 getImageOS2Header_nrestab();
    quint16 getImageOS2Header_cmovent();
    quint16 getImageOS2Header_align();
    quint16 getImageOS2Header_cres();
    quint8 getImageOS2Header_exetyp();
    quint8 getImageOS2Header_flagsothers();
    quint16 getImageOS2Header_pretthunks();
    quint16 getImageOS2Header_psegrefbytes();
    quint16 getImageOS2Header_swaparea();
    quint16 getImageOS2Header_expver();

    void setImageOS2Header_magic(quint16 nValue);
    void setImageOS2Header_ver(quint8 nValue);
    void setImageOS2Header_rev(quint8 nValue);
    void setImageOS2Header_enttab(quint16 nValue);
    void setImageOS2Header_cbenttab(quint16 nValue);
    void setImageOS2Header_crc(quint32 nValue);
    void setImageOS2Header_flags(quint16 nValue);
    void setImageOS2Header_autodata(quint16 nValue);
    void setImageOS2Header_heap(quint16 nValue);
    void setImageOS2Header_stack(quint16 nValue);
    void setImageOS2Header_csip(quint32 nValue);
    void setImageOS2Header_sssp(quint32 nValue);
    void setImageOS2Header_cseg(quint16 nValue);
    void setImageOS2Header_cmod(quint16 nValue);
    void setImageOS2Header_cbnrestab(quint16 nValue);
    void setImageOS2Header_segtab(quint16 nValue);
    void setImageOS2Header_rsrctab(quint16 nValue);
    void setImageOS2Header_restab(quint16 nValue);
    void setImageOS2Header_modtab(quint16 nValue);
    void setImageOS2Header_imptab(quint16 nValue);
    void setImageOS2Header_nrestab(quint32 nValue);
    void setImageOS2Header_cmovent(quint16 nValue);
    void setImageOS2Header_align(quint16 nValue);
    void setImageOS2Header_cres(quint16 nValue);
    void setImageOS2Header_exetyp(quint8 nValue);
    void setImageOS2Header_flagsothers(quint8 nValue);
    void setImageOS2Header_pretthunks(quint16 nValue);
    void setImageOS2Header_psegrefbytes(quint16 nValue);
    void setImageOS2Header_swaparea(quint16 nValue);
    void setImageOS2Header_expver(quint16 nValue);

    qint64 getEntryTableOffset();
    qint64 getEntryTableSize();
    qint64 getSegmentTableOffset();
    qint64 getResourceTableOffset();
    qint64 getResidentNameTableOffset();
    qint64 getModuleReferenceTableOffset();
    qint64 getImportedNamesTableOffset();
    qint64 getNotResindentNameTableOffset();

    QList<XNE_DEF::NE_SEGMENT> getSegmentList();
    XNE_DEF::NE_SEGMENT _read_NE_SEGMENT(qint64 nOffset);

    static QMap<quint64, QString> getImageNEMagics();
    static QMap<quint64, QString> getImageNEMagicsS();
    static QMap<quint64, QString> getImageNEFlagsS();
    static QMap<quint64, QString> getImageNEExetypesS();
    static QMap<quint64, QString> getImageNEFlagsothersS();
    static QMap<quint64, QString> getImageSegmentTypesS();

    static const QString PREFIX_ImageNEMagics;

    qint64 getModuleAddress();

    _MEMORY_MAP getMemoryMap(MAPMODE mapMode = MAPMODE_UNKNOWN, PDSTRUCT *pPdStruct = nullptr) override;
    MODE getMode() override;
    QString getArch() override;
    ENDIAN getEndian() override;
    FT getFileType() override;
    qint32 getType() override;
    qint64 getImageSize() override;
    XADDR _getEntryPointAddress() override;
    OSNAME getOsName() override;
    QString getOsVersion() override;
    QString typeIdToString(qint32 nType) override;

    QString getFileFormatExtsString() override;
    QList<MAPMODE> getMapModesList() override;

    QString structIDToString(quint32 nID) override;
    QString structIDToFtString(quint32 nID) override;
    quint32 ftStringToStructID(const QString &sFtString) override;
    QList<DATA_HEADER> getDataHeaders(const DATA_HEADERS_OPTIONS &dataHeadersOptions, PDSTRUCT *pPdStruct) override;

    QList<XBinary::FPART> getFileParts(quint32 nFileParts, qint32 nLimit = -1, PDSTRUCT *pPdStruct = nullptr) override;
    QList<QString> getSearchSignatures() override;
    XBinary *createInstance(QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1) override;
};

#endif  // XNE_H
