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
#ifndef XFORMATS_H
#define XFORMATS_H

#include "xamigahunk.h"
#include "xatarist.h"
#include "xbinary.h"
#include "xcom.h"
#include "xelf.h"
#include "xbmp.h"
#include "xgif.h"
#include "xicc.h"
#include "xicon.h"
#include "xjpeg.h"
#include "xle.h"
#include "xmach.h"
#include "xmsdos.h"
#include "xne.h"
#include "xpe.h"
#include "xpng.h"
#include "xtiff.h"
#include "xmp3.h"
#include "xmp4.h"
#include "xxm.h"
#include "xriff.h"
#include "xwebp.h"
#include "xavi.h"
#include "xwav.h"
#include "xjavaclass.h"
#include "xpyc.h"
#include "xttf.h"
#include "xtext.h"
#include "xdjvu.h"
#include "xder.h"
#include "../../XOptions/xthreadobject.h"
#ifdef USE_DEX
#include "xandroidbinary.h"
#include "xdex.h"
#endif
#ifdef USE_PDF
#include "xpdf.h"
#endif
#ifdef USE_ARCHIVE
#include "xarchives.h"
#endif
#ifdef QT_GUI_LIB
#include <QComboBox>  // TODO Check TESTLIB !!!
#include <QProgressBar>
#endif

class XFormats : public XThreadObject {
    Q_OBJECT

public:
    enum MODE {
        MODE_UNKNOWN = 0,
        MODE_UNPACKDEVICETOFOLDER,
    };

    explicit XFormats(QObject *pParent = nullptr);

    static XBinary *getClass(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1);

    static bool isValid(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1, XBinary::PDSTRUCT *pPdStruct = nullptr);

    static XBinary::_MEMORY_MAP getMemoryMap(XBinary::FT fileType, XBinary::MAPMODE mapMode, QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1,
                                             XBinary::PDSTRUCT *pPdStruct = nullptr);
    static XBinary::_MEMORY_MAP getMemoryMap(const QString &sFileName, XBinary::MAPMODE mapMode, bool bIsImage = false, XADDR nModuleAddress = -1,
                                             XBinary::PDSTRUCT *pPdStruct = nullptr);
    static QList<XBinary::MAPMODE> getMapModesList(XBinary::FT fileType);
    static XADDR getEntryPointAddress(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1);
    static qint64 getEntryPointOffset(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1);
    static bool isBigEndian(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1);
    static QList<XBinary::FPART> getHighlights(XBinary::FT fileType, QIODevice *pDevice, XBinary::HLTYPE hlType, bool bIsImage = false, XADDR nModuleAddress = -1,
                                               XBinary::PDSTRUCT *pPdStruct = nullptr);
    static bool isSigned(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1);
    static bool isSigned(const QString &sFileName);
    static XBinary::OFFSETSIZE getSignOffsetSize(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1);
    static XBinary::OFFSETSIZE getSignOffsetSize(const QString &sFileName);
    static QList<XBinary::SYMBOL_RECORD> getSymbolRecords(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1,
                                                          XBinary::SYMBOL_TYPE symBolType = XBinary::SYMBOL_TYPE_ALL);  // TODO pdStruct
    static QSet<XBinary::FT> getFileTypes(QIODevice *pDevice, bool bExtra, XBinary::PDSTRUCT *pPdStruct = nullptr);
    static QSet<XBinary::FT> getFileTypes(QIODevice *pDevice, qint64 nOffset, qint64 nSize, bool bExtra = false, XBinary::PDSTRUCT *pPdStruct = nullptr);
    static QSet<XBinary::FT> getFileTypes(const QString &sFileName, bool bExtra = false, XBinary::PDSTRUCT *pPdStruct = nullptr);
    static QSet<XBinary::FT> getFileTypes(QByteArray *pbaData, bool bExtra = false);
    static XBinary::FILEFORMATINFO getFileFormatInfo(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1,
                                                     XBinary::PDSTRUCT *pPdStruct = nullptr, qint64 nOffset = 0, qint64 nSize = -1);
    static qint64 getFileFormatSize(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1, XBinary::PDSTRUCT *pPdStruct = nullptr,
                                    qint64 nOffset = 0, qint64 nSize = -1);

    static QList<XBinary::DATA_HEADER> getDataHeaders(XBinary::FT fileType, QIODevice *pDevice, const XBinary::DATA_HEADERS_OPTIONS &dataHeadersOptions,
                                                      bool bIsImage = false, XADDR nModuleAddress = -1, XBinary::PDSTRUCT *pPdStruct = nullptr);

    static QList<XBinary::FPART> getFileParts(XBinary::FT fileType, QIODevice *pDevice, quint32 nFileParts, qint32 nLimit = -1, bool bIsImage = false,
                                              XADDR nModuleAddress = -1, XBinary::PDSTRUCT *pPdStruct = nullptr);

    static QList<XBinary::ARCHIVERECORD> getArchiveRecords(XBinary::FT fileType, QIODevice *pDevice, qint32 nLimit = -1, bool bIsImage = false, XADDR nModuleAddress = -1,
                                                           XBinary::PDSTRUCT *pPdStruct = nullptr);

    static qint32 getDataRecordValues(XBinary::FT fileType, QIODevice *pDevice, const XBinary::DATA_RECORDS_OPTIONS &dataRecordsOptions,
                                      QList<XBinary::DATA_RECORD_ROW> *pListDataRecords, QList<QString> *pListTitles, bool bIsImage = false, XADDR nModuleAddress = -1,
                                      XBinary::PDSTRUCT *pPdStruct = nullptr);

    static QString getFileFormatExtsString(XBinary::FT fileType);
    static bool isArchive(XBinary::FT fileType);
    static bool isExecutable(XBinary::FT fileType);

#ifdef USE_ARCHIVE
    static QSet<XBinary::FT> getFileTypes(QIODevice *pDevice, XArchive::RECORD *pRecord, bool bExtra = false);
    static QSet<XBinary::FT> getFileTypesZIP(QIODevice *pDevice, QList<XArchive::RECORD> *pListRecords, XBinary::PDSTRUCT *pPdStruct);
    static QSet<XBinary::FT> getFileTypesZIP(QIODevice *pDevice, XBinary::PDSTRUCT *pPdStruct);
    static QSet<XBinary::FT> getFileTypesTGZ(QIODevice *pDevice, QList<XArchive::RECORD> *pListRecords, XBinary::PDSTRUCT *pPdStruct);
    static QSet<XBinary::FT> getFileTypesAR(QIODevice *pDevice, QList<XArchive::RECORD> *pListRecords, XBinary::PDSTRUCT *pPdStruct);
    static QSet<XBinary::FT> getFileTypesGZIP(QIODevice *pDevice, QList<XArchive::RECORD> *pListRecords, XBinary::PDSTRUCT *pPdStruct);
#endif
#ifdef QT_GUI_LIB
    static XBinary::FT setFileTypeComboBox(XBinary::FT fileType, QIODevice *pDevice, QComboBox *pComboBox, XBinary::TL_OPTION tlOption = XBinary::TL_OPTION_DEFAULT);
    static XBinary::FT setFileTypeComboBox(XBinary::FT fileType, const QString &sFileName, QComboBox *pComboBox,
                                           XBinary::TL_OPTION tlOption = XBinary::TL_OPTION_DEFAULT);
    static QVariant setComboBoxCurrent(QComboBox *pComboBox, QVariant varValue);
    static XBinary::ENDIAN setEndiannessComboBox(QComboBox *pComboBox, XBinary::ENDIAN endian);
    static qint32 setBaseComboBox(QComboBox *pComboBox, qint32 nBase);
    static XBinary::MAPMODE getMapModesList(XBinary::FT fileType, QComboBox *pComboBox);
    static void setProgressBar(QProgressBar *pProgressBar, XBinary::PDRECORD pdRecord);
    static XBinary::DM setDisasmModeComboBox(XBinary::DM disasmMode, QComboBox *pComboBox);
#endif
    static bool saveAllPEIconsToDirectory(QIODevice *pDevice, const QString &sDirectoryName);
    static bool saveAllPECursorsToDirectory(QIODevice *pDevice, const QString &sDirectoryName);
    static bool savePE_ICOToFile(QIODevice *pDevice, QList<XPE::RESOURCE_RECORD> *pListResourceRecords, XPE::RESOURCE_RECORD resourceRecord, const QString &sFileName);

private:
    static QSet<XBinary::FT> _getFileTypes(QIODevice *pDevice, bool bExtra, XBinary::PDSTRUCT *pPdStruct);

public:
    void setData(MODE mode, XBinary::FT fileFormat, QIODevice *pDevice, QString sFolderName, XBinary::PDSTRUCT *pPdStruct);
    virtual void process();

    bool unpackDeviceToFolder(XBinary::FT fileType, QIODevice *pDevice, QString sFolderName, XBinary::PDSTRUCT *pPdStruct);
    bool extractArchiveRecordsToFolder(QList<XBinary::ARCHIVERECORD> *pListRecords, QIODevice *pDevice, QString sFolderName, XBinary::PDSTRUCT *pPdStruct);
    bool packFolderToDevice(XBinary::FT fileType, QString sFolderName, QIODevice *pDevice, void *pOptions, XBinary::PDSTRUCT *pPdStruct);

private:
    MODE m_mode;
    XBinary::FT m_fileFormat;
    QIODevice *m_pDevice;
    QString m_sFolderName;
    XBinary::PDSTRUCT *m_pPdStruct;
};

#endif  // XFORMATS_H
