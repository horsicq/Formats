/* Copyright (c) 2022 hors<horsicq@gmail.com>
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
#include "xmp3.h"

XMP3::XMP3(QIODevice *pDevice) : XBinary(pDevice)
{
}

XMP3::~XMP3()
{
}

bool XMP3::isValid()
{
    bool bResult = false;

    if (getSize() > 0x20) {
        _MEMORY_MAP memoryMap = XBinary::getMemoryMap();

        if (compareSignature(&memoryMap, "'ID3'0200", 0) || compareSignature(&memoryMap, "'ID3'0300", 0) || compareSignature(&memoryMap, "'ID3'0400", 0)) {
            // TODO more checks
            bResult = true;
        }
    }

    return bResult;
}

bool XMP3::isValid(QIODevice *pDevice)
{
    XMP3 mp3(pDevice);

    return mp3.isValid();
}

QString XMP3::getFileFormatString()
{
    QString sResult;

    sResult = QString("MP3(%1)").arg(getVersion());

    return sResult;
}

QString XMP3::getFileFormatExt()
{
    return "mp3";
}

qint64 XMP3::getFileFormatSize()
{
    return _calculateRawSize();
}

XBinary::_MEMORY_MAP XMP3::getMemoryMap(PDSTRUCT *pPdStruct)
{
    PDSTRUCT ppStructEmpty = {};

    if (!pPdStruct) {
        pPdStruct = &ppStructEmpty;
    }

    XBinary::_MEMORY_MAP result = {};

    result.nRawSize = getSize();

    qint32 nIndex = 0;

    _MEMORY_MAP _memoryMap = XBinary::getMemoryMap();

    if (compareSignature(&_memoryMap, "'ID3'..00", 0)) {
        quint32 nVar[4] = {};
        nVar[0] = read_uint8(6);
        nVar[1] = read_uint8(7);
        nVar[2] = read_uint8(8);
        nVar[3] = read_uint8(9);

        qint64 nOffset = 10 + ((nVar[0] << 21) | (nVar[1] << 14) | (nVar[2] << 7) | (nVar[3]));

        {
            _MEMORY_RECORD record = {};

            record.nIndex = nIndex++;
            record.type = MMT_HEADER;
            record.nOffset = 0;
            record.nSize = nOffset;
            record.nAddress = -1;
            record.sName = tr("Header");

            result.listRecords.append(record);
        }
        // TODO
        decodeFrame(nOffset);
    }

    return result;
}

XBinary::FT XMP3::getFileType()
{
    return FT_MP3;
}

QString XMP3::getVersion()
{
    QString sResult;

    if (getSize() > 0x20) {
        _MEMORY_MAP memoryMap = XBinary::getMemoryMap();

        if (compareSignature(&memoryMap, "'ID3'0200", 0)) {
            sResult = "3.2";
        } else if (compareSignature(&memoryMap, "'ID3'0300", 0)) {
            sResult = "3.3";
        } else if (compareSignature(&memoryMap, "'ID3'0400", 0)) {
            sResult = "3.4";
        }
    }

    return sResult;
}

void XMP3::decodeFrame(qint64 nOffset)
{
    quint32 nHeader = read_uint32(nOffset, true);

    if (nHeader & 0xFFFFFF00) {
        quint8 nVersion = (nHeader >> 18) & 0x1;
        quint8 nLayer = (nHeader >> 17) & 0x1;
        quint8 nErrorProtection = (nHeader >> 16) & 0x1;
        quint8 nBitRate = (nHeader >> 15) & 0xF;
        quint8 nFrequency = (nHeader >> 11) & 0x3;
        quint8 nPadBit = (nHeader >> 9) & 0x1;
        quint8 nPrivBit = (nHeader >> 8) & 0x1;
        quint8 nMode = (nHeader >> 7) & 0x3;
        quint8 nIntensiveStereo = (nHeader >> 5) & 0x1;
        quint8 nMSStereo = (nHeader >> 4) & 0x1;
        quint8 nCopy = (nHeader >> 3) & 0x1;
        quint8 nOriginal = (nHeader >> 2) & 0x1;
        quint8 nEmphasis = (nHeader >> 0) & 0x3;

        if (nLayer == 1) {  // III

        } else if (nLayer == 2) {  // II

        } else if (nLayer == 3) {  // I
        }
    }
}
