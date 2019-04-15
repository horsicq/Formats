// copyright (c) 2017-2019 hors<horsicq@gmail.com>
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
#ifndef XMACH_H
#define XMACH_H

#include "xbinary.h"
#include "xmach_def.h"

class XMACH : public XBinary
{
    Q_OBJECT
public:
    XMACH(QIODevice *__pDevice=nullptr,bool bIsImage=false,qint64 nImageAddress=-1);
    ~XMACH();

    bool isValid();
    bool isBigEndian();
    bool is64();

    quint32 getHeader_magic();
    qint32 getHeader_cputype();
    qint32 getHeader_cpusubtype();
    quint32 getHeader_filetype();
    quint32 getHeader_ncmds();
    quint32 getHeader_sizeofcmds();
    quint32 getHeader_flags();
    quint32 getHeader_reserved();

    void setHeader_magic(quint32 nValue);
    void setHeader_cputype(qint32 nValue);
    void setHeader_cpusubtype(qint32 nValue);
    void setHeader_filetype(quint32 nValue);
    void setHeader_ncmds(quint32 nValue);
    void setHeader_sizeofcmds(quint32 nValue);
    void setHeader_flags(quint32 nValue);
    void setHeader_reserved(quint32 nValue);

    static QMap<quint64,QString> getHeaderMagics();
    static QMap<quint64,QString> getHeaderMagicsS();
};

#endif // XMACH_H
