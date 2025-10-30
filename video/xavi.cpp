/* Copyright (c) 2022-2025 hors<horsicq@gmail.com>
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
#include "xavi.h"

XBinary::XCONVERT _TABLE_XAVI_STRUCTID[] = {{XAVI::STRUCTID_UNKNOWN, "Unknown", QObject::tr("Unknown")}, {XAVI::STRUCTID_HEADER, "HEADER", QObject::tr("Header")},
                                            {XAVI::STRUCTID_CHUNK, "CHUNK", QObject::tr("Chunk")},       {XAVI::STRUCTID_AVIH, "AVIH", QObject::tr("AVI Header")},
                                            {XAVI::STRUCTID_STRH, "STRH", QObject::tr("Stream Header")}, {XAVI::STRUCTID_STRF, "STRF", QObject::tr("Stream Format")},
                                            {XAVI::STRUCTID_JUNK, "JUNK", QObject::tr("Junk")},          {XAVI::STRUCTID_MOVI, "MOVI", QObject::tr("Movie Data")},
                                            {XAVI::STRUCTID_IDX1, "IDX1", QObject::tr("Index")}};

XAVI::XAVI(QIODevice *pDevice) : XRiff(pDevice)
{
}
XAVI::~XAVI()
{
}

bool XAVI::isValid(PDSTRUCT *pPdStruct)
{
    if (!XRiff::isValid(pPdStruct)) return false;
    // AVI: RIFF header with form type "AVI " at offset 8
    QString form = read_ansiString(8, 4);
    return (form == "AVI ");
}

bool XAVI::isValid(QIODevice *pDevice)
{
    XAVI x(pDevice);
    return x.isValid();
}

QString XAVI::getFileFormatExt()
{
    return "avi";
}
QString XAVI::getFileFormatExtsString()
{
    return "AVI";
}
qint64 XAVI::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    return _calculateRawSize(pPdStruct);
}
XBinary::FT XAVI::getFileType()
{
    return FT_AVI;
}
QString XAVI::getMIMEString()
{
    return "video/x-msvideo";
}

QString XAVI::structIDToString(quint32 nID)
{
    return XBinary::XCONVERT_idToTransString(nID, _TABLE_XAVI_STRUCTID, sizeof(_TABLE_XAVI_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QString XAVI::getArch()
{
    return "";  // AVI is a multimedia container, no specific architecture
}

XBinary::MODE XAVI::getMode()
{
    return MODE_DATA;  // AVI files contain data, not executable code
}

XBinary::ENDIAN XAVI::getEndian()
{
    return XRiff::getEndian();  // Delegate to parent class
}

QString XAVI::getVersion()
{
    return "";  // AVI format doesn't have a version number
}
