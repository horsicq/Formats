/* Copyright (c) 2022-2026 hors<horsicq@gmail.com>
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
#include "xwav.h"

XWAV::XWAV(QIODevice *pDevice) : XRiff(pDevice)
{
}

XWAV::~XWAV()
{
}

bool XWAV::isValid(PDSTRUCT *pPdStruct)
{
    if (!XRiff::isValid(pPdStruct)) return false;
    QString form = read_ansiString(8, 4);
    return (form == "WAVE");
}

bool XWAV::isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    XWAV x(pDevice);
    return x.isValid(pPdStruct);
}

QString XWAV::getFileFormatExt()
{
    return "wav";
}

QString XWAV::getFileFormatExtsString()
{
    return "WAV";
}

qint64 XWAV::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    return _calculateRawSize(pPdStruct);
}

XBinary::FT XWAV::getFileType()
{
    return FT_WAV;
}

QString XWAV::getMIMEString()
{
    return "audio/x-wav";
}

QList<QString> XWAV::getSearchSignatures()
{
    QList<QString> listResult;

    listResult.append("'RIFF'....'WAVE'");

    return listResult;
}

XBinary *XWAV::createInstance(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    Q_UNUSED(bIsImage)
    Q_UNUSED(nModuleAddress)

    return new XWAV(pDevice);
}
