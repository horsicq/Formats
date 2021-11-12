// Copyright (c) 2020-2021 hors<horsicq@gmail.com>
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
#ifndef XCOM_H
#define XCOM_H

#include "xbinary.h"
#include "xcom_def.h"

class XCOM : public XBinary
{
    Q_OBJECT

public:
    enum TYPE
    {
        TYPE_UNKNOWN=0,
        TYPE_EXE
        // TODO more
    };

    explicit XCOM(QIODevice *pDevice=nullptr,bool bIsImage=false,qint64 nModuleAddress=-1);
    ~XCOM();
    virtual bool isValid();
    static bool isValid(QIODevice *pDevice,bool bIsImage=false,qint64 nModuleAddress=-1);
    static MODE getMode(QIODevice *pDevice,bool bIsImage=false,qint64 nModuleAddress=-1);

    virtual _MEMORY_MAP getMemoryMap();
    virtual QString getArch();
    virtual MODE getMode();
    virtual bool isBigEndian();
    virtual qint64 getEntryPointOffset(_MEMORY_MAP *pMemoryMap);
    virtual qint64 getImageSize();
    virtual FT getFileType();
    virtual qint32 getType();
    virtual QString typeIdToString(int nType);
};

#endif // XCOM_H
