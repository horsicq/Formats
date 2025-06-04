/* Copyright (c) 2025 hors<horsicq@gmail.com>
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
#include "xttf.h"

XTTF::XTTF(QIODevice *pDevice)
    : XBinary(pDevice), m_sfntVersion(0), m_numTables(0)
{
    _readHeader();
    _readTableDirectory();
}

XTTF::~XTTF()
{
}

bool XTTF::isValid(PDSTRUCT *pPdStruct)
{
    // TTF starts with: 0x00010000 or 'OTTO' (for CFF, OpenType)
    if (getSize() < 12)
        return false;
    quint32 version = read_uint32(0, true); // Big-endian
    return (version == 0x00010000) || (version == 0x4F54544F); // 'OTTO'
}

XBinary::FT XTTF::getFileType()
{
    return XBinary::FT_TTF;
}

XBinary::MODE XTTF::getMode()
{
    return XBinary::MODE_8;
}

QString XTTF::getMIMEString()
{
    return "font/ttf";
}

qint32 XTTF::getType()
{
    return 0;
}

QString XTTF::typeIdToString(qint32 nType)
{
    return QString::number(nType);
}

XBinary::ENDIAN XTTF::getEndian()
{
    return XBinary::ENDIAN_BIG;
}

QString XTTF::getArch()
{
    return QString();
}

QString XTTF::getFileFormatExt()
{
    return "ttf";
}

qint64 XTTF::getFileFormatSize(PDSTRUCT * /*pPdStruct*/)
{
    return getSize();
}

bool XTTF::isSigned()
{
    return false;
}

XBinary::OSNAME XTTF::getOsName()
{
    return XBinary::OSNAME_UNKNOWN;
}

QString XTTF::getOsVersion()
{
    return QString();
}

QString XTTF::getVersion()
{
    return QString();
}

QString XTTF::getInfo()
{
    return "";
}

bool XTTF::isEncrypted()
{
    return false;
}

QList<XBinary::MAPMODE> XTTF::getMapModesList()
{
    QList<MAPMODE> list;
    list.append(MAPMODE_OBJECTS);
    return list;
}

XBinary::_MEMORY_MAP XTTF::getMemoryMap(MAPMODE mapMode, PDSTRUCT * pPdStruct)
{
    _MEMORY_MAP memoryMap = {};
    memoryMap.fileType = getFileType();
    memoryMap.mode = getMode();
    memoryMap.sArch = getArch();
    memoryMap.endian = getEndian();
    memoryMap.sType = typeIdToString(getType());
    memoryMap.nBinarySize = getSize();
    memoryMap.nImageSize = getSize();

    if (mapMode == MAPMODE_OBJECTS) {
        QList<TTF_TABLE_RECORD> tables = getTableDirectory();
        qint32 nNumberOfTables = tables.size();
        for (qint32 i = 0; (i < nNumberOfTables) && isPdStructNotCanceled(pPdStruct); i++) {
            _MEMORY_RECORD rec = {};
            rec.nOffset = tables.at(i).offset;
            rec.nAddress = -1; // TTF tables do not have a virtual address;
            rec.segment = ADDRESS_SEGMENT_FLAT;
            rec.nSize = tables.at(i).length;
            rec.type = MMT_OBJECT;
            rec.nLoadSectionNumber = i;
            rec.sName = tagToString(tables.at(i).tag);
            rec.nIndex = i;
            rec.bIsVirtual = false;
            rec.bIsInvisible = false;
            rec.nID = i;
            memoryMap.listRecords.append(rec);
        }
    }

    return memoryMap;
}

QList<XBinary::HREGION> XTTF::getNativeRegions(PDSTRUCT * /*pPdStruct*/)
{
    QList<HREGION> list;
    // Each table is a region
    QList<TTF_TABLE_RECORD> tables = getTableDirectory();
    qint32 nNumberOfTables = tables.size();
    for (qint32 i = 0; i < nNumberOfTables; i++) {
        HREGION reg = {};
        reg.nVirtualAddress = 0;
        reg.nVirtualSize = tables.at(i).length;
        reg.nFileOffset = tables.at(i).offset;
        reg.nFileSize = tables.at(i).length;
        reg.sName = tagToString(tables.at(i).tag);
        list.append(reg);
    }
    return list;
}

QList<XBinary::HREGION> XTTF::getNativeSubRegions(PDSTRUCT * /*pPdStruct*/)
{
    return QList<HREGION>();
}

QList<XBinary::HREGION> XTTF::getHData(PDSTRUCT *pPdStruct)
{
    return getNativeRegions(pPdStruct);
}

QList<XBinary::DATA_HEADER> XTTF::getDataHeaders(const DATA_HEADERS_OPTIONS & /*dataHeadersOptions*/, PDSTRUCT * /*pPdStruct*/)
{
    QList<DATA_HEADER> list;
    // TODO: Add headers for TTF main header, and for each table
    return list;
}

QList<QString> XTTF::getTableTitles(const DATA_RECORDS_OPTIONS & /*dataRecordsOptions*/)
{
    QList<QString> list;
    list << "Tag" << "Checksum" << "Offset" << "Length";
    return list;
}

qint32 XTTF::readTableRow(qint32 nRow, LT /*locType*/, XADDR /*nLocation*/, const DATA_RECORDS_OPTIONS & /*dataRecordsOptions*/, QList<QVariant> *pListValues, PDSTRUCT * /*pPdStruct*/)
{
    QList<TTF_TABLE_RECORD> tables = getTableDirectory();
    qint32 nNumberOfTables = tables.size();
    if ((nRow >= 0) && (nRow < nNumberOfTables)) {
        if (pListValues) {
            pListValues->clear();
            pListValues->append(tagToString(tables.at(nRow).tag));
            pListValues->append(QString("0x%1").arg(tables.at(nRow).checkSum, 8, 16, QChar('0')));
            pListValues->append(QString("0x%1").arg(tables.at(nRow).offset, 8, 16, QChar('0')));
            pListValues->append(QString("0x%1").arg(tables.at(nRow).length, 8, 16, QChar('0')));
        }
        return 4;
    }
    return 0;
}

quint16 XTTF::getNumTables()
{
    return m_numTables;
}

QList<XTTF::TTF_TABLE_RECORD> XTTF::getTableDirectory()
{
    if (m_tableDirectory.isEmpty()) {
        _readTableDirectory();
    }
    return m_tableDirectory;
}

QString XTTF::tagToString(quint32 tag)
{
    QByteArray arr;
    arr.append(char((tag >> 24) & 0xFF));
    arr.append(char((tag >> 16) & 0xFF));
    arr.append(char((tag >> 8) & 0xFF));
    arr.append(char(tag & 0xFF));
    return QString::fromLatin1(arr);
}

bool XTTF::_readHeader()
{
    if (getSize() < 12)
        return false;
    m_sfntVersion = read_uint32(0, true); // Big-endian
    m_numTables = read_uint16(4, true);
    return true;
}

void XTTF::_readTableDirectory()
{
    m_tableDirectory.clear();
    if (!_readHeader())
        return;
    qint32 nNumberOfTables = m_numTables;
    for (qint32 i = 0; i < nNumberOfTables; i++) {
        qint64 nEntryOffset = 12 + (i * 16);
        if ((nEntryOffset + 16) > getSize())
            break;
        TTF_TABLE_RECORD rec;
        rec.tag = read_uint32(nEntryOffset, true);
        rec.checkSum = read_uint32(nEntryOffset + 4, true);
        rec.offset = read_uint32(nEntryOffset + 8, true);
        rec.length = read_uint32(nEntryOffset + 12, true);
        m_tableDirectory.append(rec);
    }
}
