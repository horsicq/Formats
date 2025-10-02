/* Copyright (c) 2020-2025 hors<horsicq@gmail.com>
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
#include "xatarist.h"

XAtariST::XAtariST(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress) : XBinary(pDevice, bIsImage, nModuleAddress)
{
}

XAtariST::~XAtariST()
{
}

bool XAtariST::isValid(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    bool bResult = false;

    if (getSize() >= (qint64)sizeof(XATARIST_DEF::HEADER)) {
        quint16 nMagic = read_uint16(0, true);  // Big-endian

        if (nMagic == XATARIST_DEF::MAGIC) {
            bResult = true;
        }
    }

    return bResult;
}

bool XAtariST::isValid(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    XAtariST xatarist(pDevice, bIsImage, nModuleAddress);

    return xatarist.isValid();
}

bool XAtariST::isExecutable()
{
    return true;
}

XBinary::MODE XAtariST::getMode(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    XAtariST xatarist(pDevice, bIsImage, nModuleAddress);

    return xatarist.getMode();
}

XATARIST_DEF::HEADER XAtariST::getHeader()
{
    XATARIST_DEF::HEADER result = {};

    result.nMagic = read_uint16(0, true);
    result.nTextSize = read_uint32(2, true);
    result.nDataSize = read_uint32(6, true);
    result.nBssSize = read_uint32(10, true);
    result.nSymbolSize = read_uint32(14, true);
    result.nReserved = read_uint32(18, true);
    result.nFlags = read_uint32(22, true);
    result.nRelocation = read_uint16(26, true);

    return result;
}

XBinary::_MEMORY_MAP XAtariST::getMemoryMap(XBinary::MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(mapMode)

    XBinary::PDSTRUCT pdStructEmpty = {};

    if (!pPdStruct) {
        pdStructEmpty = XBinary::createPdStruct();
        pPdStruct = &pdStructEmpty;
    }

    _MEMORY_MAP result = {};

    XATARIST_DEF::HEADER header = getHeader();

    qint64 nTotalSize = getSize();
    qint64 nHeaderSize = sizeof(XATARIST_DEF::HEADER);

    result.nModuleAddress = getModuleAddress();
    result.nEntryPointAddress = result.nModuleAddress;
    result.nBinarySize = nTotalSize;
    result.fileType = getFileType();
    result.mode = getMode();
    result.sArch = getArch();
    result.endian = getEndian();
    result.sType = getTypeAsString();

    qint64 nCurrentOffset = nHeaderSize;

    // Text segment
    if (header.nTextSize > 0) {
        _MEMORY_RECORD recordText = {};
        recordText.nAddress = result.nModuleAddress;
        recordText.nOffset = nCurrentOffset;
        recordText.nSize = qMin((qint64)header.nTextSize, nTotalSize - nCurrentOffset);
        recordText.nIndex = 0;
        // recordText.type = MMT_LOADSEGMENT;
        recordText.sName = QString("Text");

        result.listRecords.append(recordText);
        nCurrentOffset += recordText.nSize;
    }

    // Data segment
    if ((header.nDataSize > 0) && (nCurrentOffset < nTotalSize)) {
        _MEMORY_RECORD recordData = {};
        recordData.nAddress = result.nModuleAddress + header.nTextSize;
        recordData.nOffset = nCurrentOffset;
        recordData.nSize = qMin((qint64)header.nDataSize, nTotalSize - nCurrentOffset);
        recordData.nIndex = 1;
        // recordData.type = MMT_LOADSEGMENT;
        recordData.sName = QString("Data");

        result.listRecords.append(recordData);
        nCurrentOffset += recordData.nSize;
    }

    // BSS segment (virtual)
    if (header.nBssSize > 0) {
        _MEMORY_RECORD recordBss = {};
        recordBss.nAddress = result.nModuleAddress + header.nTextSize + header.nDataSize;
        recordBss.nOffset = -1;
        recordBss.nSize = header.nBssSize;
        recordBss.nIndex = 2;
        recordBss.bIsVirtual = true;
        // recordBss.type = MMT_LOADSEGMENT;
        recordBss.sName = QString("BSS");

        result.listRecords.append(recordBss);
    }

    result.nImageSize = header.nTextSize + header.nDataSize + header.nBssSize;

    _handleOverlay(&result);

    return result;
}

QString XAtariST::getArch()
{
    return QString("68000");
}

XBinary::MODE XAtariST::getMode()
{
    return MODE_32;
}

XBinary::ENDIAN XAtariST::getEndian()
{
    return ENDIAN_BIG;
}

qint64 XAtariST::getImageSize()
{
    XATARIST_DEF::HEADER header = getHeader();

    return header.nTextSize + header.nDataSize + header.nBssSize;
}

XBinary::FT XAtariST::getFileType()
{
    return FT_ATARIST;
}

qint32 XAtariST::getType()
{
    return TYPE_EXECUTABLE;
}

XBinary::OSNAME XAtariST::getOsName()
{
    return OSNAME_ATARIST;
}

QString XAtariST::typeIdToString(qint32 nType)
{
    QString sResult = tr("Unknown");

    switch (nType) {
        case TYPE_UNKNOWN: sResult = tr("Unknown"); break;
        case TYPE_EXECUTABLE: sResult = QString("Executable"); break;
    }

    return sResult;
}

QString XAtariST::getFileFormatExtsString()
{
    return QString("tos,ttp,prg");
}
