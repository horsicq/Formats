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
#ifndef XNE_H
#define XNE_H

#include "xmsdos.h"
#include "xne_def.h"

class XNE : public XMSDOS
{
    Q_OBJECT

public:
    explicit XNE(QIODevice *__pDevice=nullptr,bool bIsImage=false,qint64 nImageBase=-1);
    virtual bool isValid();

    qint64 getImageOS2HeaderOffset();
    XNE_DEF::IMAGE_OS2_HEADER getImageOS2Header();

    quint16 getImageOS2Header_magic();
    void setImageOS2Header_magic(quint16 value);

    static QMap<quint64, QString> getImageNEMagics();
    static QMap<quint64, QString> getImageNEMagicsS();
};

#endif // XNE_H
