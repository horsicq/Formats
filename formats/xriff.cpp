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
#include "xriff.h"

XRiff::XRiff(QIODevice *pDevice) : XBinary(pDevice)
{
}

XRiff::~XRiff()
{
}

bool XRiff::isValid(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    bool bResult = false;

    if (getSize() > 0x20) {
        QString sTag = read_ansiString(0, 4);

        bool bIsValid = false;
        bool bIsBigEndian = false;

        if (sTag == "RIFF") {
            bIsValid = true;
        } else if ((sTag == "RIFX") || (sTag == "AIFF")) {
            bIsValid = true;
            bIsBigEndian = true;
        }

        if (bIsValid) {
            quint32 nSize = read_uint32(4, bIsBigEndian);
            qint64 _nSize = getSize();

            if ((nSize > 0) && (nSize <= _nSize)) {
                bResult = true;
            }
        }
    }

    return bResult;
}

bool XRiff::isValid(QIODevice *pDevice)
{
    XRiff xriff(pDevice);

    return xriff.isValid();
}

QString XRiff::getFileFormatExt()
{
    // TODO
    return read_ansiString(12, 4).trimmed().toLower();
}

QString XRiff::getFileFormatExtsString()
{
    return "RIFF (*.riff *.wav *.avi *.aiff *.aifc *.aif *.aifx *.rmi *.rmf *.rmv)";  // TODO
}

qint64 XRiff::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    return _calculateRawSize(pPdStruct);
}

QString XRiff::getMIMEString()
{
    QString sResult;

    QString sTag = read_ansiString(0, 4);

    if (sTag == "RIFF") {
        sResult = "audio/x-wav";
    } else if (sTag == "RIFX") {
        sResult = "audio/x-aiff";
    } else if (sTag == "AIFF") {
        sResult = "audio/x-aiff";
    }

    return sResult;
}

XBinary::_MEMORY_MAP XRiff::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(mapMode)

    XBinary::PDSTRUCT pdStructEmpty = {};

    if (!pPdStruct) {
        pdStructEmpty = XBinary::createPdStruct();
        pPdStruct = &pdStructEmpty;
    }

    XBinary::_MEMORY_MAP result = {};

    result.nBinarySize = getSize();
    result.endian = getEndian();

    qint32 nIndex = 0;

    qint64 nOffset = 0;

    quint32 nChunkSize = read_uint32(nOffset + 4, result.endian == ENDIAN_BIG);
    QString sTag = read_ansiString(nOffset, 4);

    {
        _MEMORY_RECORD record = {};

        record.nIndex = nIndex++;
        record.filePart = FILEPART_REGION;
        record.nOffset = nOffset;
        record.nSize = nChunkSize + 8;
        record.nAddress = -1;
        record.sName = sTag;

        result.listRecords.append(record);
    }

    _handleOverlay(&result);

    return result;
}

XBinary::FT XRiff::getFileType()
{
    FT result = FT_RIFF;

    QString sTag = read_ansiString(0, 4);
    QString sSubTag = read_ansiString(12, 4);

    // TODO
    if ((sTag == "RIFF") || (sTag == "RIFX")) {
        if (sSubTag == "AVI ") {
            result = FT_AVI;
        } else if (sSubTag == "WEBP") {
            result = FT_WEBP;
        } else if (sSubTag == "WAVE") {
            result = FT_WAV;
        }
    }

    return result;
}

XBinary::ENDIAN XRiff::getEndian()
{
    ENDIAN result = ENDIAN_UNKNOWN;

    QString sTag = read_ansiString(0, 4);

    if (sTag == "RIFF") {
        result = ENDIAN_LITTLE;
    } else if ((sTag == "RIFX") || (sTag == "AIFF")) {
        result = ENDIAN_BIG;
    }

    return result;
}
