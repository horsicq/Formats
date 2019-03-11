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
#include "qmsdos.h"

QMSDOS::QMSDOS(QIODevice *__pDevice, bool bIsImage, qint64 nImageAddress): QBinary(__pDevice,bIsImage,nImageAddress)
{
}

bool QMSDOS::isValid()
{
    quint16 magic=get_magic();

    if((magic==S_IMAGE_DOS_SIGNATURE)||(magic==S_IMAGE_DOS_SIGNATURE_ZM))
    {
        return true;
    }

    return false;
}

quint16 QMSDOS::get_magic()
{
    return read_uint16((qint64)offsetof(S_IMAGE_DOS_HEADEREX,e_magic));
}

qint32 QMSDOS::get_lfanew()
{
    return read_int32(offsetof(S_IMAGE_DOS_HEADEREX,e_lfanew));
}

S_IMAGE_DOS_HEADER QMSDOS::getDosHeader()
{
    S_IMAGE_DOS_HEADER result=S_IMAGE_DOS_HEADER();

    read_array((qint64)offsetof(S_IMAGE_DOS_HEADER,e_magic),(char *)&result,sizeof(S_IMAGE_DOS_HEADER));

    return result;
}

S_IMAGE_DOS_HEADEREX QMSDOS::getDosHeaderEx()
{
    S_IMAGE_DOS_HEADEREX result=S_IMAGE_DOS_HEADEREX();

    read_array((qint64)offsetof(S_IMAGE_DOS_HEADEREX,e_magic),(char *)&result,sizeof(S_IMAGE_DOS_HEADEREX));

    return result;
}

void QMSDOS::setDosHeader(S_IMAGE_DOS_HEADER *pDosHeader)
{
    write_array((qint64)offsetof(S_IMAGE_DOS_HEADER,e_magic),(char *)pDosHeader,sizeof(S_IMAGE_DOS_HEADER));
}

void QMSDOS::setDosHeaderEx(S_IMAGE_DOS_HEADEREX *pDosHeaderEx)
{
    write_array((qint64)offsetof(S_IMAGE_DOS_HEADEREX,e_magic),(char *)pDosHeaderEx,sizeof(S_IMAGE_DOS_HEADEREX));
}

void QMSDOS::set_e_magic(quint16 value)
{
    write_uint16(offsetof(S_IMAGE_DOS_HEADEREX,e_magic),value);
}

void QMSDOS::set_e_cblp(quint16 value)
{
    write_uint16(offsetof(S_IMAGE_DOS_HEADEREX,e_cblp),value);
}

void QMSDOS::set_e_cp(quint16 value)
{
    write_uint16(offsetof(S_IMAGE_DOS_HEADEREX,e_cp),value);
}

void QMSDOS::set_e_crlc(quint16 value)
{
    write_uint16(offsetof(S_IMAGE_DOS_HEADEREX,e_crlc),value);
}

void QMSDOS::set_e_cparhdr(quint16 value)
{
    write_uint16(offsetof(S_IMAGE_DOS_HEADEREX,e_cparhdr),value);
}

void QMSDOS::set_e_minalloc(quint16 value)
{
    write_uint16(offsetof(S_IMAGE_DOS_HEADEREX,e_minalloc),value);
}

void QMSDOS::set_e_maxalloc(quint16 value)
{
    write_uint16(offsetof(S_IMAGE_DOS_HEADEREX,e_maxalloc),value);
}

void QMSDOS::set_e_ss(quint16 value)
{
    write_uint16(offsetof(S_IMAGE_DOS_HEADEREX,e_ss),value);
}

void QMSDOS::set_e_sp(quint16 value)
{
    write_uint16(offsetof(S_IMAGE_DOS_HEADEREX,e_sp),value);
}

void QMSDOS::set_e_csum(quint16 value)
{
    write_uint16(offsetof(S_IMAGE_DOS_HEADEREX,e_csum),value);
}

void QMSDOS::set_e_ip(quint16 value)
{
    write_uint16(offsetof(S_IMAGE_DOS_HEADEREX,e_ip),value);
}

void QMSDOS::set_e_cs(quint16 value)
{
    write_uint16(offsetof(S_IMAGE_DOS_HEADEREX,e_cs),value);
}

void QMSDOS::set_e_lfarlc(quint16 value)
{
    write_uint16(offsetof(S_IMAGE_DOS_HEADEREX,e_lfarlc),value);
}

void QMSDOS::set_e_ovno(quint16 value)
{
    write_uint16(offsetof(S_IMAGE_DOS_HEADEREX,e_ovno),value);
}

void QMSDOS::set_e_res(int nPosition, quint16 value)
{
    if(nPosition<4)
    {
        write_uint16(offsetof(S_IMAGE_DOS_HEADEREX,e_res)+sizeof(quint16)*nPosition,value);
    }
}

void QMSDOS::set_e_oemid(quint16 value)
{
    write_uint16(offsetof(S_IMAGE_DOS_HEADEREX,e_oemid),value);
}

void QMSDOS::set_e_oeminfo(quint16 value)
{
    write_uint16(offsetof(S_IMAGE_DOS_HEADEREX,e_oeminfo),value);
}

void QMSDOS::set_e_res2(int nPosition, quint16 value)
{
    if(nPosition<10)
    {
        write_uint16(offsetof(S_IMAGE_DOS_HEADEREX,e_res2)+sizeof(quint16)*nPosition,value);
    }
}

void QMSDOS::set_e_lfanew(quint32 value)
{
    write_uint32(offsetof(S_IMAGE_DOS_HEADEREX,e_lfanew),value);
}
