/* Copyright (c) 2024 hors<horsicq@gmail.com>
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
#include "xjavaclass.h"

XJavaClass::XJavaClass(QIODevice *pDevice) : XBinary(pDevice)
{
}

XJavaClass::~XJavaClass()
{
}

bool XJavaClass::isValid(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    bool bResult=false;

    if (getSize() > 8) {
        if (read_uint32(0, true) == 0xCAFEBABE) {
            if (read_uint32(4, true) > 10) {
                bResult = true;
            }
        }
    }

    return bResult;
}

bool XJavaClass::isValid(QIODevice *pDevice)
{
    XJavaClass xjavaclass(pDevice);

    return xjavaclass.isValid();
}

QString XJavaClass::getArch()
{
    return "JVM";
}

QString XJavaClass::getVersion()
{
    return "1.0";
}

XBinary::FT XJavaClass::getFileType()
{
    return FT_JAVACLASS;
}

XBinary::ENDIAN XJavaClass::getEndian()
{
    return ENDIAN_BIG;
}

XBinary::MODE XJavaClass::getMode()
{
    return MODE_32;
}
