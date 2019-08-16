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
#ifndef XMSDOS_H
#define XMSDOS_H

#include "xbinary.h"
#include "xmsdos_def.h"

class XMSDOS : public XBinary
{
    Q_OBJECT

public:
    explicit XMSDOS(QIODevice *__pDevice=nullptr,bool bIsImage=false,qint64 nImageBase=-1);
    virtual bool isValid();

    quint16 get_magic();
    qint32 get_lfanew();

    XMSDOS_DEF::IMAGE_DOS_HEADER getDosHeader();
    XMSDOS_DEF::IMAGE_DOS_HEADEREX getDosHeaderEx();
    void setDosHeader(XMSDOS_DEF::IMAGE_DOS_HEADER *pDosHeader);
    void setDosHeaderEx(XMSDOS_DEF::IMAGE_DOS_HEADEREX *pDosHeaderEx);

    void set_e_magic(quint16 value);
    void set_e_cblp(quint16 value);
    void set_e_cp(quint16 value);
    void set_e_crlc(quint16 value);
    void set_e_cparhdr(quint16 value);
    void set_e_minalloc(quint16 value);
    void set_e_maxalloc(quint16 value);
    void set_e_ss(quint16 value);
    void set_e_sp(quint16 value);
    void set_e_csum(quint16 value);
    void set_e_ip(quint16 value);
    void set_e_cs(quint16 value);
    void set_e_lfarlc(quint16 value);
    void set_e_ovno(quint16 value);
    void set_e_res(int nPosition,quint16 value);
    void set_e_oemid(quint16 value);
    void set_e_oeminfo(quint16 value);
    void set_e_res2(int nPosition,quint16 value);
    void set_e_lfanew(quint32 value);

    quint16 get_e_magic();
    quint16 get_e_cblp();
    quint16 get_e_cp();
    quint16 get_e_crlc();
    quint16 get_e_cparhdr();
    quint16 get_e_minalloc();
    quint16 get_e_maxalloc();
    quint16 get_e_ss();
    quint16 get_e_sp();
    quint16 get_e_csum();
    quint16 get_e_ip();
    quint16 get_e_cs();
    quint16 get_e_lfarlc();
    quint16 get_e_ovno();

    virtual QList<MEMORY_MAP> getMemoryMapList();
    virtual qint64 getEntryPointOffset();

    static QMap<quint64, QString> getImageMagics();
    static QMap<quint64, QString> getImageMagicsS();

    qint64 getOverlayOffset(); // mb TODO move to XBinary
    qint64 getOverlaySize();
    bool isOverlayPresent();

//private:
//    const qint64 N_DATABASEADDRESS=0x10000;
//    const qint64 N_CODEBASEADDRESS=0x00000;
};

#endif // XMSDOS_H
