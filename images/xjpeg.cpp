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
#include "xjpeg.h"

XJpeg::XJpeg(QIODevice *pDevice) : XBinary(pDevice)
{
}

XJpeg::~XJpeg()
{
}

bool XJpeg::isValid()
{
    bool bIsValid = false;

    if (getSize() >= 20) {
        bIsValid = compareSignature("FFD8FFE0....'JFIF'00");
    }

    return bIsValid;
}

bool XJpeg::isValid(QIODevice *pDevice)
{
    XJpeg xjpeg(pDevice);

    return xjpeg.isValid();
}

XBinary::FT XJpeg::getFileType()
{
    return FT_JPEG;
}

QString XJpeg::getFileFormatString()
{
    QString sResult;

    sResult = QString("JPEG");

    return sResult;
}

QString XJpeg::getFileFormatExt()
{
    return "jpeg";
}

qint64 XJpeg::getFileFormatSize()
{
    qint64 nResult = 0;

    QList<CHUNK> listChunks = getChunks();

    qint32 nNumberOfChunks = listChunks.count();

    if (nNumberOfChunks > 0) {
        if (listChunks.at(nNumberOfChunks - 1).nId == 0xD9) {
            nResult = listChunks.at(nNumberOfChunks - 1).nDataOffset;
        }
    }

    return nResult;
}

QList<XJpeg::CHUNK> XJpeg::getChunks(PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = {};

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    QList<CHUNK> listResult;

    qint64 nOffset = 0;

    while (true) {
        if (nOffset == -1) {
            break;
        }

        CHUNK chunk = _readChunk(nOffset);

        if (!chunk.bValid) {
            break;
        }

        listResult.append(chunk);

        nOffset = chunk.nDataOffset + chunk.nDataSize;

        if (chunk.nId == 0xDA) {
            while (true) {
                nOffset = find_uint8(nOffset, -1, 0xFF, pPdStruct);  // TODO PDStruct;

                if (nOffset == -1) {
                    break;
                }

                if (read_uint8(nOffset + 1) != 0x00) {
                    break;
                } else {
                    nOffset++;
                }
            }
        }

        if (chunk.nId == 0xD9) {  // END of image
            break;
        }
    }

    return listResult;
}

XJpeg::CHUNK XJpeg::_readChunk(qint64 nOffset)
{
    CHUNK result = {};

    quint8 nBegin = read_uint8(nOffset);

    if (nBegin == 0xFF) {
        result.bValid = true;
        result.nId = read_uint8(nOffset + 1);

        result.nDataOffset = nOffset + 2;

        if ((result.nId == 0xD8) || (result.nId == 0xD9)) {
            result.nDataSize = 0;
        } else if (result.nId == 0xDD) {
            result.nDataSize = 4;
        } else if ((result.nId >= 0xD0) && (result.nId <= 0xD7)) {
            result.nDataSize = 0;
        } else if (result.nId != 0x00) {
            result.nDataSize = read_uint16(nOffset + 2, true);
        } else {
            result.bValid = false;
        }
    }

    return result;
}
