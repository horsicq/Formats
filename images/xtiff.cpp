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
#include "xtiff.h"

XTiff::XTiff(QIODevice *pDevice) : XBinary(pDevice)
{
}

XTiff::~XTiff()
{
}

bool XTiff::isValid()
{
    bool bIsValid = false;

    if (getSize() >= 8) {
        _MEMORY_MAP memoryMap = XBinary::getMemoryMap();
        if (compareSignature(&memoryMap, "'MM'002A") || compareSignature(&memoryMap, "'II'2A00")) {
            bool bIsBigEndian = isBigEndian();

            quint32 nOffset = read_uint32(4, bIsBigEndian);

            if ((nOffset > 0) && (nOffset < getSize())) {
                bIsValid = true;
            }
        }
    }

    return bIsValid;
}

bool XTiff::isValid(QIODevice *pDevice)
{
    XTiff xtiff(pDevice);

    return xtiff.isValid();
}

XBinary::_MEMORY_MAP XTiff::getMemoryMap(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct) // TODO

    qint32 nIndex = 0;

    _MEMORY_MAP result = {};

    qint64 nTotalSize = getSize();

    result.nRawSize = nTotalSize;
    result.bIsBigEndian = isBigEndian();

    if(result.nRawSize > 8)
    {
        {
            _MEMORY_RECORD record = {};

            record.nIndex = nIndex++;
            record.type = MMT_HEADER;
            record.nOffset = 0;
            record.nSize = 8;
            record.nAddress = -1;
            record.sName = tr("Header");

            result.listRecords.append(record);
        }

        qint64 nTableOffset = read_uint32(4,result.bIsBigEndian);

        while (nTableOffset) {
            quint16 nTableCount = read_uint16(nTableOffset,result.bIsBigEndian);

            {
                _MEMORY_RECORD record = {};

                record.nIndex = nIndex++;
                record.type = MMT_TABLE;
                record.nOffset = nTableOffset;
                record.nSize = sizeof(quint16) + sizeof(IFD_ENTRY) * nTableCount;
                record.nAddress = -1;
                record.sName = tr("Table");

                result.listRecords.append(record);
            }

            qint64 nCurrentOffset = nTableOffset + sizeof(quint16);

            for (qint32 i = 0; i < nTableCount; i++) {

                quint16 nTag = read_uint16(nCurrentOffset + offsetof(IFD_ENTRY, nTag), result.bIsBigEndian);
                quint16 nType = read_uint16(nCurrentOffset + offsetof(IFD_ENTRY, nType), result.bIsBigEndian);
                quint32 nCount = read_uint32(nCurrentOffset + offsetof(IFD_ENTRY, nCount), result.bIsBigEndian);

                //1 = BYTE 8-bit unsigned integer.
                //2 = ASCII 8-bit byte that contains a 7-bit ASCII code; the last byte must be NUL (binary zero).
                //3 = SHORT 16-bit (2-byte) unsigned integer.
                //4 = LONG 32-bit (4-byte) unsigned integer.
                //5 = RATIONAL Two LONGs: the first represents the numerator of a fraction; the second, the denominator
                //6 = SBYTE An 8-bit signed (twos-complement) integer.
                //7 = UNDEFINED An 8-bit byte that may contain anything, depending on the definition of the field.
                //8 = SSHORT A 16-bit (2-byte) signed (twos-complement) integer.
                //9 = SLONG A 32-bit (4-byte) signed (twos-complement) integer.
                //10 = SRATIONAL Two SLONG’s: the first represents the numerator of a fraction, the second the denominator.
                //11 = FLOAT Single precision (4-byte) IEEE format.
                //12 = DOUBLE Double precision (8-byte) IEEE format.

                qint32 nBaseTypeSize = 0;

                // TODO a function
                switch (nType) {
                case 1:
                    nBaseTypeSize = 1;
                    break;
                case 2:
                    nBaseTypeSize = 1;
                    break;
                case 3:
                    nBaseTypeSize = 2;
                    break;
                case 4:
                    nBaseTypeSize = 4;
                    break;
                case 5:
                    nBaseTypeSize = 8;
                    break;
                case 6:
                    nBaseTypeSize = 1;
                    break;
                case 7:
                    nBaseTypeSize = 1;
                    break;
                case 8:
                    nBaseTypeSize = 2;
                    break;
                case 9:
                    nBaseTypeSize = 4;
                    break;
                case 10:
                    nBaseTypeSize = 8;
                    break;
                case 11:
                    nBaseTypeSize = 4;
                    break;
                case 12:
                    nBaseTypeSize = 8;
                    break;
                default:
                    nBaseTypeSize = 0;
                }

                qint64 nDataSize = nBaseTypeSize * nCount;

                if ( nDataSize > 4) {
                    quint32 nOffset = read_uint32(nCurrentOffset + offsetof(IFD_ENTRY, nOffset), result.bIsBigEndian);

                    _MEMORY_RECORD record = {};

                    record.nIndex = nIndex++;
                    record.type = MMT_FILESEGMENT;
                    record.nOffset = nOffset;
                    record.nSize = nDataSize;
                    record.nAddress = -1;
                    record.sName = QString("%1-%2").arg(XBinary::valueToHex(nTag),XBinary::valueToHex(nType));

                    result.listRecords.append(record);
                }

                nCurrentOffset += sizeof(IFD_ENTRY);
            }

            nTableOffset = read_uint32(nCurrentOffset,result.bIsBigEndian);

            {
                _MEMORY_RECORD record = {};

                record.nIndex = nIndex++;
                record.type = MMT_DATA;
                record.nOffset = nCurrentOffset;
                record.nSize = sizeof(quint32);
                record.nAddress = -1;

                result.listRecords.append(record);
            }
        }
    }

    return result;
}

XBinary::FT XTiff::getFileType()
{
    return FT_TIFF;
}

QString XTiff::getFileFormatString()
{
    QString sResult;

    sResult = QString("TIFF");

    return sResult;
}

QString XTiff::getFileFormatExt()
{
    return "tiff";
}

qint64 XTiff::getFileFormatSize()
{
    return _calculateRawSize();
}

bool XTiff::isBigEndian()
{
    quint32 nEndian = read_uint32(0);

    return (nEndian == 0x2A004D4D);
}
