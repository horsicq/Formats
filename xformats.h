// copyright (c) 2020-2021 hors<horsicq@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#ifndef XFORMATS_H
#define XFORMATS_H

#include "xbinary.h"
#include "xcom.h"
#include "xmsdos.h"
#include "xne.h"
#include "xle.h"
#include "xpe.h"
#include "xelf.h"
#include "xmach.h"
#ifdef USE_DEX
#include "xdex.h"
#include "xandroidbinary.h"
#endif
#ifdef USE_ARCHIVE
#include "xarchives.h"
#endif
#ifdef QT_GUI_LIB
#include <QComboBox> // TODO Check TESTLIB !!!
#endif

class XFormats : public QObject
{
    Q_OBJECT

public:
    explicit XFormats(QObject *pParent=nullptr);
    static XBinary::_MEMORY_MAP getMemoryMap(XBinary::FT fileType,QIODevice *pDevice,bool bIsImage=false,qint64 nModuleAddress=0);
    static qint64 getEntryPointAddress(XBinary::FT fileType,QIODevice *pDevice,bool bIsImage=false,qint64 nModuleAddress=0);
    static qint64 getEntryPointOffset(XBinary::FT fileType,QIODevice *pDevice,bool bIsImage=false,qint64 nModuleAddress=0);
    static bool isBigEndian(XBinary::FT fileType,QIODevice *pDevice,bool bIsImage=false,qint64 nModuleAddress=0);
    static QList<XBinary::SYMBOL_RECORD> getSymbolRecords(XBinary::FT fileType,QIODevice *pDevice,bool bIsImage=false,qint64 nModuleAddress=0,XBinary::SYMBOL_TYPE symBolType=XBinary::SYMBOL_TYPE_ALL);
#ifdef QT_GUI_LIB
    static XBinary::FT setFileTypeComboBox(QComboBox *pComboBox,QList<XBinary::FT> *pListFileTypes,XBinary::FT fileType);
    static bool setEndianessComboBox(QComboBox *pComboBox,bool bIsBigEndian);
#endif
};

#endif // XFORMATS_H
