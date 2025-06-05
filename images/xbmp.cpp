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
#include "xbmp.h"

XBMP::XBMP(QIODevice *pDevice) : XBinary(pDevice)
{
}

XBMP::~XBMP()
{
}

bool XBMP::isValid(PDSTRUCT *pPdStruct)
{
    bool bResult = false;

    _MEMORY_MAP memoryMap = XBinary::getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);

    if (compareSignature(&memoryMap, "'BM'..................000000", 0, pPdStruct)) {
        quint32 nSize = read_uint32(2);

        if ((nSize > 0) && (nSize <= getSize())) {
            quint32 nDBISize = read_uint32(0x0E);

            if (nDBISize == 40) {
                bResult = true;
            } else if (nDBISize == 108) {
                bResult = true;
            } else if (nDBISize == 124) {
                bResult = true;
            }
        }
    }

    // TODO
    //    BM Windows 3.1x, 95, NT, ... etc.
    //    BA OS/2 struct bitmap array
    //    CI OS/2 struct color icon
    //    CP OS/2 const color pointer
    //    IC OS/2 struct icon
    //    PT OS/2 pointer

    return bResult;
}

bool XBMP::isValid(QIODevice *pDevice)
{
    XBMP xbmp(pDevice);

    return xbmp.isValid();
}

QString XBMP::getMIMEString()
{
    return "image/bmp";
}

QString XBMP::getArch()
{
    return "";
}

XBinary::ENDIAN XBMP::getEndian()
{
    return XBinary::ENDIAN_LITTLE;
}

XBinary::MODE XBMP::getMode()
{
    return XBinary::MODE_DATA;
}

XBinary::FT XBMP::getFileType()
{
    return FT_BMP;
}

QString XBMP::getFileFormatExt()
{
    return "bmp";
}

XBinary::_MEMORY_MAP XBMP::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    _MEMORY_MAP memoryMap = {};
    memoryMap.fileType = getFileType();
    memoryMap.mode = getMode();
    memoryMap.sArch = getArch();
    memoryMap.endian = getEndian();
    memoryMap.sType = typeIdToString(getType());
    memoryMap.nBinarySize = getSize();
    memoryMap.nImageSize = getSize();
    memoryMap.nModuleAddress = getModuleAddress();
    memoryMap.bIsImage = false;

    BMPFILEHEADER fileHeader = getFileHeader();
    BMPINFOHEADER infoHeader = getInfoHeader();

    // Add Header
    _MEMORY_RECORD headerRecord = {};
    headerRecord.nOffset = 0;
    headerRecord.nAddress = -1;
    headerRecord.segment = ADDRESS_SEGMENT_FLAT;
    headerRecord.nSize = 14 + infoHeader.biSize;
    headerRecord.type = MMT_HEADER;
    headerRecord.sName = "Header";
    headerRecord.nIndex = 0;
    headerRecord.bIsVirtual = false;
    headerRecord.bIsInvisible = false;
    headerRecord.nID = 0;
    memoryMap.listRecords.append(headerRecord);

    // Add Bitmap Data (Object)
    _MEMORY_RECORD objectRecord = {};
    objectRecord.nOffset = fileHeader.bfOffBits;
    objectRecord.nAddress = -1;
    objectRecord.segment = ADDRESS_SEGMENT_FLAT;
    objectRecord.nSize = fileHeader.bfSize - fileHeader.bfOffBits;
    objectRecord.type = MMT_DATA;
    objectRecord.sName = "Bitmap Data";
    objectRecord.nIndex = 1;
    objectRecord.bIsVirtual = false;
    objectRecord.bIsInvisible = false;
    memoryMap.listRecords.append(objectRecord);

    // Add Overlay if present
    if (fileHeader.bfSize < getSize()) {
        _MEMORY_RECORD overlayRecord = {};
        overlayRecord.nOffset = fileHeader.bfSize;
        overlayRecord.nAddress = -1;
        overlayRecord.segment = ADDRESS_SEGMENT_FLAT;
        overlayRecord.nSize = getSize() - fileHeader.bfSize;;
        overlayRecord.type = MMT_OVERLAY;
        overlayRecord.nLoadSectionNumber = -1;
        overlayRecord.sName = "Overlay";
        overlayRecord.nIndex = 2;
        overlayRecord.bIsVirtual = false;
        overlayRecord.bIsInvisible = false;
        memoryMap.listRecords.append(overlayRecord);
    }

    return memoryMap;
}

QString XBMP::getVersion()
{
    QString sResult;

    quint32 nDBISize = read_uint32(0x0E);

    if (nDBISize == 40) {
        sResult = "Windows NT, 3.1x";
    } else if (nDBISize == 108) {
        sResult = "Windows NT 4.0, 95";
    } else if (nDBISize == 124) {
        sResult = "Windows NT 5.0, 98";
    }

    return sResult;
}

XBMP::BMPINFOHEADER XBMP::getInfoHeader()
{
    BMPINFOHEADER info = {};
    info.biSize = read_uint32(14, false);
    info.biWidth = read_int32(18, false);
    info.biHeight = read_int32(22, false);
    info.biPlanes = read_uint16(26, false);
    info.biBitCount = read_uint16(28, false);
    info.biCompression = read_uint32(30, false);
    info.biSizeImage = read_uint32(34, false);
    info.biXPelsPerMeter = read_int32(38, false);
    info.biYPelsPerMeter = read_int32(42, false);
    info.biClrUsed = read_uint32(46, false);
    info.biClrImportant = read_uint32(50, false);
    return info;
}

XBMP::BMPFILEHEADER XBMP::getFileHeader()
{
    BMPFILEHEADER header = {};
    header.bfType = read_uint16(0, false);
    header.bfSize = read_uint32(2, false);
    header.bfReserved1 = read_uint16(6, false);
    header.bfReserved2 = read_uint16(8, false);
    header.bfOffBits = read_uint32(10, false);
    return header;
}
