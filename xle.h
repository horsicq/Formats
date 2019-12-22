// copyright (c) 2019 hors<horsicq@gmail.com>
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
#ifndef XLE_H
#define XLE_H

#include "xmsdos.h"
#include "xle_def.h"

class XLE : public XMSDOS
{
    Q_OBJECT

public:
    explicit XLE(QIODevice *__pDevice=nullptr,bool bIsImage=false,qint64 nImageBase=-1);
    virtual bool isValid();

    qint64 getImageVxdHeaderOffset();
    XLE_DEF::IMAGE_VXD_HEADER getImageVxdHeader();

    quint16 getImageVxdHeader_magic();
    quint8 getImageVxdHeader_border();
    quint8 getImageVxdHeader_worder();
    quint32 getImageVxdHeader_level();
    quint16 getImageVxdHeader_cpu();
    quint16 getImageVxdHeader_os();
    quint32 getImageVxdHeader_ver();
    quint32 getImageVxdHeader_mflags();
    quint32 getImageVxdHeader_mpages();
    quint32 getImageVxdHeader_startobj();
    quint32 getImageVxdHeader_eip();
    quint32 getImageVxdHeader_stackobj();
    quint32 getImageVxdHeader_esp();
    quint32 getImageVxdHeader_pagesize();

    void setImageVxdHeader_magic(quint16 value);
    void setImageVxdHeader_border(quint8 value);
    void setImageVxdHeader_worder(quint8 value);
    void setImageVxdHeader_level(quint32 value);
    void setImageVxdHeader_cpu(quint16 value);
    void setImageVxdHeader_os(quint16 value);
    void setImageVxdHeader_ver(quint32 value);
    void setImageVxdHeader_mflags(quint32 value);
    void setImageVxdHeader_mpages(quint32 value);
    void setImageVxdHeader_startobj(quint32 value);
    void setImageVxdHeader_eip(quint32 value);
    void setImageVxdHeader_stackobj(quint32 value);
    void setImageVxdHeader_esp(quint32 value);
    void setImageVxdHeader_pagesize(quint32 value);

    static QMap<quint64, QString> getImageLEMagics();
    static QMap<quint64, QString> getImageLEMagicsS();
};

#endif // XLE_H
