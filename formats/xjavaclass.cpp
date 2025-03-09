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

    bool bResult = false;

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
    QString sResult;

    quint16 nMinor = read_uint16(4, true);
    quint16 nMajor = read_uint16(6, true);

    if (nMajor) {
        sResult = _getJDKVersion(nMajor, nMinor);
    }

    return sResult;
}

QString XJavaClass::getFileFormatExt()
{
    return "class";
}

QString XJavaClass::_getJDKVersion(quint16 nMajor, quint16 nMinor)
{
    QString sResult;

    switch (nMajor) {
        case 0x2D: sResult = "JDK 1.1"; break;
        case 0x2E: sResult = "JDK 1.2"; break;
        case 0x2F: sResult = "JDK 1.3"; break;
        case 0x30: sResult = "JDK 1.4"; break;
        case 0x31: sResult = "Java SE 5.0"; break;
        case 0x32: sResult = "Java SE 6"; break;
        case 0x33: sResult = "Java SE 7"; break;
        case 0x34: sResult = "Java SE 8"; break;
        case 0x35: sResult = "Java SE 9"; break;
        case 0x36: sResult = "Java SE 10"; break;
        case 0x37: sResult = "Java SE 11"; break;
        case 0x38: sResult = "Java SE 12"; break;
        case 0x39: sResult = "Java SE 13"; break;
        case 0x3A: sResult = "Java SE 14"; break;
        case 0x3B: sResult = "Java SE 15"; break;
        case 0x3C: sResult = "Java SE 16"; break;
        case 0x3D: sResult = "Java SE 17"; break;
        case 0x3E: sResult = "Java SE 18"; break;
        case 0x3F: sResult = "Java SE 19"; break;
        case 0x40: sResult = "Java SE 20"; break;
        case 0x41: sResult = "Java SE 21"; break;
        case 0x42: sResult = "Java SE 22"; break;
        case 0x43: sResult = "Java SE 23"; break;
        case 0x44: sResult = "Java SE 24"; break;
    }

    if ((sResult != "") && (nMinor)) {
        sResult += QString(".%1").arg(nMinor);
    }

    return sResult;
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
