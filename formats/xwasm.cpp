/* Copyright (c) 2026 hors<horsicq@gmail.com>
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
#include "xwasm.h"

XWASM::XWASM(QIODevice *pDevice) : XBinary(pDevice)
{
}

XWASM::~XWASM()
{
}

bool XWASM::isValid(PDSTRUCT *pPdStruct)
{
    bool bValid = false;

    _getSections(&bValid, pPdStruct);

    return bValid;
}

bool XWASM::isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    XWASM xwasm(pDevice);

    return xwasm.isValid(pPdStruct);
}

QString XWASM::getArch()
{
    return "WebAssembly";
}

XBinary::MODE XWASM::getMode()
{
    return MODE_DATA;
}

XBinary::ENDIAN XWASM::getEndian()
{
    return ENDIAN_LITTLE;
}

XBinary::FT XWASM::getFileType()
{
    return FT_WASM;
}

QString XWASM::getVersion()
{
    QString sResult;

    if (isValid()) {
        sResult = QString::number(read_uint32(4));
    }

    return sResult;
}

QString XWASM::getFileFormatExt()
{
    return "wasm";
}

QString XWASM::getFileFormatExtsString()
{
    return "WebAssembly (*.wasm)";
}

qint64 XWASM::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    return isValid(pPdStruct) ? getSize() : 0;
}

QString XWASM::getMIMEString()
{
    return "application/wasm";
}

XBinary::_MEMORY_MAP XWASM::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    XBinary::_MEMORY_MAP result = {};

    if (mapMode == MAPMODE_UNKNOWN) {
        mapMode = MAPMODE_DATA;
    }

    if (mapMode == MAPMODE_REGIONS) {
        result = _getMemoryMap(FILEPART_HEADER | FILEPART_REGION | FILEPART_OVERLAY, pPdStruct);
    } else if (mapMode == MAPMODE_DATA) {
        result = _getMemoryMap(FILEPART_DATA | FILEPART_OVERLAY, pPdStruct);
    }

    return result;
}

QList<XBinary::FPART> XWASM::getFileParts(quint32 nFileParts, qint32 nLimit, PDSTRUCT *pPdStruct)
{
    QList<FPART> listResult;

    if ((nLimit < -1) || (nLimit == 0)) {
        return listResult;
    }

    bool bValid = false;
    QList<SECTION_RECORD> listSections = _getSections(&bValid, pPdStruct);

    if (!bValid) {
        return listResult;
    }

    if (nFileParts & FILEPART_HEADER) {
        FPART record = {};
        record.filePart = FILEPART_HEADER;
        record.nFileOffset = 0;
        record.nFileSize = 8;
        record.nVirtualAddress = (XADDR)-1;
        record.sName = tr("Header");
        listResult.append(record);

        if ((nLimit != -1) && (listResult.count() >= nLimit)) {
            return listResult;
        }
    }

    if (nFileParts & FILEPART_REGION) {
        for (qint32 i = 0; i < listSections.count(); i++) {
            const SECTION_RECORD &section = listSections.at(i);
            FPART record = {};
            record.filePart = FILEPART_REGION;
            record.nFileOffset = section.nOffset;
            record.nFileSize = section.nHeaderSize + section.nPayloadSize;
            record.nVirtualAddress = (XADDR)-1;
            record.sName = sectionIdToString(section.nID);
            listResult.append(record);

            if ((nLimit != -1) && (listResult.count() >= nLimit)) {
                return listResult;
            }
        }
    }

    if (nFileParts & FILEPART_DATA) {
        FPART record = {};
        record.filePart = FILEPART_DATA;
        record.nFileOffset = 0;
        record.nFileSize = getSize();
        record.nVirtualAddress = (XADDR)-1;
        record.sName = tr("Data");
        listResult.append(record);
    }

    return listResult;
}

QList<XWASM::SECTION_RECORD> XWASM::getSections(PDSTRUCT *pPdStruct)
{
    bool bValid = false;

    return _getSections(&bValid, pPdStruct);
}

QString XWASM::sectionIdToString(quint8 nID)
{
    QString sResult = QString("Unknown section (%1)").arg(nID);

    switch (nID) {
        case SECTION_ID_CUSTOM: sResult = QString("Custom section"); break;
        case SECTION_ID_TYPE: sResult = QString("Type section"); break;
        case SECTION_ID_IMPORT: sResult = QString("Import section"); break;
        case SECTION_ID_FUNCTION: sResult = QString("Function section"); break;
        case SECTION_ID_TABLE: sResult = QString("Table section"); break;
        case SECTION_ID_MEMORY: sResult = QString("Memory section"); break;
        case SECTION_ID_GLOBAL: sResult = QString("Global section"); break;
        case SECTION_ID_EXPORT: sResult = QString("Export section"); break;
        case SECTION_ID_START: sResult = QString("Start section"); break;
        case SECTION_ID_ELEMENT: sResult = QString("Element section"); break;
        case SECTION_ID_CODE: sResult = QString("Code section"); break;
        case SECTION_ID_DATA: sResult = QString("Data section"); break;
        case SECTION_ID_DATA_COUNT: sResult = QString("Data count section"); break;
        case SECTION_ID_TAG: sResult = QString("Tag section"); break;
    }

    return sResult;
}

bool XWASM::_hasValidHeader()
{
    return (getSize() >= 8) && (read_uint32(0) == 0x6D736100) && (read_uint32(4) == 1);
}

bool XWASM::_readULEB128(qint64 *pnOffset, quint32 *pnValue)
{
    if ((!pnOffset) || (!pnValue)) {
        return false;
    }

    quint32 nValue = 0;

    for (qint32 i = 0; i < 5; i++) {
        if (*pnOffset >= getSize()) {
            return false;
        }

        quint8 nByte = read_uint8(*pnOffset);
        (*pnOffset)++;

        if ((i == 4) && ((nByte & 0xF0) != 0)) {
            return false;
        }

        nValue |= (quint32)(nByte & 0x7F) << (i * 7);

        if ((nByte & 0x80) == 0) {
            *pnValue = nValue;
            return true;
        }
    }

    return false;
}

QList<XWASM::SECTION_RECORD> XWASM::_getSections(bool *pbValid, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    QList<SECTION_RECORD> listResult;

    if (pbValid) {
        *pbValid = false;
    }

    if (!_hasValidHeader()) {
        return listResult;
    }

    qint64 nOffset = 8;
    qint64 nFileSize = getSize();

    while (nOffset < nFileSize) {
        SECTION_RECORD record = {};
        record.nOffset = nOffset;
        record.nID = read_uint8(nOffset);
        nOffset++;

        if (!_readULEB128(&nOffset, &record.nPayloadSize)) {
            listResult.clear();
            return listResult;
        }

        record.nPayloadOffset = nOffset;
        record.nHeaderSize = record.nPayloadOffset - record.nOffset;

        if ((qint64)record.nPayloadSize > (nFileSize - nOffset)) {
            listResult.clear();
            return listResult;
        }

        listResult.append(record);
        nOffset += record.nPayloadSize;
    }

    if (pbValid) {
        *pbValid = (nOffset == nFileSize);
    }

    return listResult;
}
