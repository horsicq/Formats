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
#ifndef QMSDOS_H
#define QMSDOS_H

#include "qbinary.h"
#include "qmsdos_def.h"

class QMSDOS : public QBinary
{
public:
    explicit QMSDOS(QIODevice *__pDevice=0,bool bIsImage=false);
    virtual bool isValid();

    quint16 get_magic();
    qint32 get_lfanew();

    S_IMAGE_DOS_HEADER getDosHeader();
    S_IMAGE_DOS_HEADEREX getDosHeaderEx();
    void setDosHeader(S_IMAGE_DOS_HEADER *pDosHeader);
    void setDosHeaderEx(S_IMAGE_DOS_HEADEREX *pDosHeaderEx);
};

#endif // QMSDOS_H
