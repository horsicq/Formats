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
#include "xamigahunk.h"

XAmigaHunk::XAmigaHunk(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress) : XBinary(pDevice, bIsImage, nModuleAddress)
{
}

XAmigaHunk::~XAmigaHunk()
{
}

bool XAmigaHunk::isValid(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    bool bResult = false;

    if (getSize() > 8) {
        quint32 nMagic = read_uint32(0, true);

        if ((nMagic == 0x03f3) || (nMagic == 0x03e7)) {
            bResult = true;
        }
    }

    return bResult;
}

QList<XBinary::MAPMODE> XAmigaHunk::getMapModesList()
{
    QList<XBinary::MAPMODE> listResult;

    listResult.append(XBinary::MAPMODE_UNKNOWN);
    listResult.append(XBinary::MAPMODE_REGIONS);

    return listResult;
}

XBinary::_MEMORY_MAP XAmigaHunk::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(mapMode)
    Q_UNUSED(pPdStruct)

    XBinary::_MEMORY_MAP result = {};
    result.nBinarySize = getSize();

    qint64 nCurrentOffset = 0;
    qint32 nIndex = 0;

    while (true) {
        qint32 nRegionStart = nCurrentOffset;
        quint32 nMagic = read_uint32(nCurrentOffset, true);

        if (nMagic == 0x03f3) {
            nCurrentOffset += 4;
            quint32 nEndOfList = read_uint32(nCurrentOffset, true);

            if (nEndOfList) {
                // Resident library names
                // QString sName = read_ansiString(nCurrentOffset);
            } else {
                nCurrentOffset += 4;
            }
            quint32 nTableSize = read_uint32(nCurrentOffset + 0, true);
            // quint32 nFirstLoaded = read_uint32(nCurrentOffset + 4, true);
            // quint32 nLastLoaded = read_uint32(nCurrentOffset + 8, true);

            nCurrentOffset += 12;

            QList<qint64> listSizes;

            nTableSize = qMin(nTableSize, (quint32)100);

            for (quint32 i = 0; i < nTableSize; i ++) {
                listSizes.append(read_uint32(nCurrentOffset, true));
                nCurrentOffset += 4;
            }

            {
                _MEMORY_RECORD record = {};

                record.nIndex = nIndex++;
                record.type = MMT_HEADER;
                record.nOffset = nRegionStart;
                record.nSize =  nCurrentOffset - nRegionStart;
                record.nAddress = 0;
                record.sName = tr("Header");

                result.listRecords.append(record);
            }
        } else {
            break;
        }
    }

    return result;
}
