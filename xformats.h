/* Copyright (c) 2020-2022 hors<horsicq@gmail.com>
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

#include "xbinary.h"
#include "xcom.h"
#include "xelf.h"
#include "xicon.h"
#include "xjpeg.h"
#include "xle.h"
#include "xmach.h"
#include "xmsdos.h"
#include "xne.h"
#include "xpe.h"
#include "xpng.h"
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
#endif

class XFormats : public QObject {
    Q_OBJECT

public:
    explicit XFormats(QObject *pParent = nullptr);

    static XBinary::_MEMORY_MAP getMemoryMap(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1);
    static XBinary::_MEMORY_MAP getMemoryMap(QString sFileName, bool bIsImage = false, XADDR nModuleAddress = -1);
    static qint64 getEntryPointAddress(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1);
    static qint64 getEntryPointOffset(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1);
    static bool isBigEndian(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1);
    static bool isSigned(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1);
    static bool isSigned(QString sFileName);
    static XBinary::OFFSETSIZE getSignOffsetSize(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1);
    static XBinary::OFFSETSIZE getSignOffsetSize(QString sFileName);
    static QList<XBinary::SYMBOL_RECORD> getSymbolRecords(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1,
                                                          XBinary::SYMBOL_TYPE symBolType = XBinary::SYMBOL_TYPE_ALL);
    static QSet<XBinary::FT> getFileTypes(QIODevice *pDevice, bool bExtra = false);
    static QSet<XBinary::FT> getFileTypes(QIODevice *pDevice, qint64 nOffset, qint64 nSize, bool bExtra = false);
    static QSet<XBinary::FT> getFileTypes(QString sFileName, bool bExtra = false);
    static QSet<XBinary::FT> getFileTypes(QByteArray *pbaData, bool bExtra = false);
    static XBinary::OSINFO getOsInfo(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1);
    static XBinary::FILEFORMATINFO getFileFormatInfo(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1);

#ifdef USE_ARCHIVE
    static QSet<XBinary::FT> getFileTypes(QIODevice *pDevice, XArchive::RECORD *pRecord, bool bExtra = false);
    static QSet<XBinary::FT> getFileTypesZIP(QIODevice *pDevice, QList<XArchive::RECORD> *pListRecords);
#endif
#ifdef QT_GUI_LIB
    static XBinary::FT setFileTypeComboBox(XBinary::FT fileType, QIODevice *pDevice, QComboBox *pComboBox);
    static XBinary::FT setFileTypeComboBox(QString sFileName, QComboBox *pComboBox);
    static bool setEndiannessComboBox(QComboBox *pComboBox, bool bIsBigEndian);
#endif

private:
    static QSet<XBinary::FT> _getFileTypes(QIODevice *pDevice, bool bExtra = false);
};

#endif  // XFORMATS_H
