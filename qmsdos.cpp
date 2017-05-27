// Copyright (c) 2017 hors<horsicq@gmail.com>
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

QMSDOS::QMSDOS(QIODevice *pData,bool bIsImage): QBinary(pData,bIsImage)
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
    S_IMAGE_DOS_HEADER result= {0};

    read_array((qint64)offsetof(S_IMAGE_DOS_HEADER,e_magic),(char *)&result,sizeof(S_IMAGE_DOS_HEADER));

    return result;
}

S_IMAGE_DOS_HEADEREX QMSDOS::getDosHeaderEx()
{
    S_IMAGE_DOS_HEADEREX result= {0};

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
