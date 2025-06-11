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

XTTF::XTTF(QIODevice *pDevice) : XBinary(pDevice)
{
}

XTTF::~XTTF()
{
}

bool XTTF::isValid(PDSTRUCT *pPdStruct)
{
    // TTF starts with: 0x00010000 or 'OTTO' (for CFF, OpenType)
    if (getSize() < 12) return false;
    quint32 version = read_uint32(0, true);                     // Big-endian
    return (version == 0x00010000) || (version == 0x4F54544F);  // 'OTTO'
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
    QList<TTF_TABLE_RECORD> listTables = getTableDirectory(readHeader().numTables);

    return getVersion(&listTables, nullptr);
}

QString XTTF::getVersion(QList<TTF_TABLE_RECORD> *pListTables, PDSTRUCT *pPdStruct)
{
    QString sResult;

    qint32 nNumberOfTables = pListTables->count();
    qint64 nHeadOffset = -1;

    for (qint32 i = 0; (i < nNumberOfTables) && isPdStructNotCanceled(pPdStruct); i++) {
        if (pListTables->at(i).tag == 0x68656164) {  // 'head'
            nHeadOffset = pListTables->at(i).offset;
            break;
        }
    }

    if ((nHeadOffset != -1) && isOffsetValid(nHeadOffset + 8)) {
        quint32 nVersionFixed = read_uint32(nHeadOffset, true);
        quint32 nRevisionFixed = read_uint32(nHeadOffset + 4, true);

        QString sVersion = XBinary::get_uint32_version(nVersionFixed);
        QString sRevision = XBinary::get_uint32_version(nRevisionFixed);

        sResult = QString("%1 rev %2").arg(sVersion, sRevision);
    }

    return sResult;
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

XBinary::_MEMORY_MAP XTTF::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)
    _MEMORY_MAP result = {};
    result.fileType = getFileType();
    result.mode = getMode();
    result.sArch = getArch();
    result.endian = getEndian();
    result.sType = typeIdToString(getType());
    result.nImageSize = getSize();
    result.nBinarySize = getSize();
    result.nModuleAddress = getModuleAddress();

    // TTF Header
    TTF_HEADER header = readHeader();

    // Table Directory
    QList<TTF_TABLE_RECORD> tableRecords = getTableDirectory(header.numTables);

    qint64 nHeaderOffset = 0;
    qint64 nHeaderSize = 12 + header.numTables * 16;

    _MEMORY_RECORD memHeader = {};
    memHeader.nOffset = nHeaderOffset;
    memHeader.nAddress = -1;  // TTF header does not have a specific address in memory
    memHeader.nSize = nHeaderSize;
    memHeader.type = MMT_HEADER;
    memHeader.sName = "Header";
    result.listRecords.append(memHeader);

    // Add Table Objects
    for (qint32 i = 0, nNumberOfTables = tableRecords.size(); (i < nNumberOfTables) && isPdStructNotCanceled(pPdStruct); i++) {
        _MEMORY_RECORD rec = {};
        rec.nOffset = tableRecords.at(i).offset;
        rec.nSize = tableRecords.at(i).length;
        rec.type = MMT_OBJECT;
        rec.sName = tagToString(tableRecords.at(i).tag);
        rec.nIndex = i;
        rec.nAddress = -1;  // TTF tables do not have a specific address in memory
        result.listRecords.append(rec);
    }

    _handleOverlay(&result);

    return result;
}

QList<XBinary::HREGION> XTTF::getNativeRegions(PDSTRUCT *pPdStruct)
{
    return _getPhysRegions(MAPMODE_UNKNOWN, pPdStruct);
}

QList<XBinary::HREGION> XTTF::getNativeSubRegions(PDSTRUCT * /*pPdStruct*/)
{
    return QList<HREGION>();
}

QList<XBinary::HREGION> XTTF::getHData(PDSTRUCT *pPdStruct)
{
    return getNativeRegions(pPdStruct);
}

QList<XBinary::DATA_HEADER> XTTF::getDataHeaders(const DATA_HEADERS_OPTIONS &dataHeadersOptions, PDSTRUCT *pPdStruct)
{
    QList<XBinary::DATA_HEADER> listResult;

    if (dataHeadersOptions.nID == STRUCTID_UNKNOWN) {
        DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
        _dataHeadersOptions.bChildren = true;
        _dataHeadersOptions.dsID_parent = _addDefaultHeaders(&listResult, pPdStruct);
        _dataHeadersOptions.dhMode = XBinary::DHMODE_HEADER;

        _dataHeadersOptions.nID = -1;

        listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
    } else {
        qint64 nStartOffset = locationToOffset(dataHeadersOptions.pMemoryMap, dataHeadersOptions.locType, dataHeadersOptions.nLocation);

        if (nStartOffset != -1) {
            // TODO
        }
    }

    return listResult;
}

QList<QString> XTTF::getTableTitles(const DATA_RECORDS_OPTIONS & /*dataRecordsOptions*/)
{
    QList<QString> list;
    list << "Tag"
         << "Checksum"
         << "Offset"
         << "Length";
    return list;
}

qint32 XTTF::readTableRow(qint32 nRow, LT /*locType*/, XADDR /*nLocation*/, const DATA_RECORDS_OPTIONS & /*dataRecordsOptions*/, QList<QVariant> *pListValues,
                          PDSTRUCT * /*pPdStruct*/)
{
    // QList<TTF_TABLE_RECORD> tables = getTableDirectory();
    // qint32 nNumberOfTables = tables.size();
    // if ((nRow >= 0) && (nRow < nNumberOfTables)) {
    //     if (pListValues) {
    //         pListValues->clear();
    //         pListValues->append(tagToString(tables.at(nRow).tag));
    //         pListValues->append(QString("0x%1").arg(tables.at(nRow).checkSum, 8, 16, QChar('0')));
    //         pListValues->append(QString("0x%1").arg(tables.at(nRow).offset, 8, 16, QChar('0')));
    //         pListValues->append(QString("0x%1").arg(tables.at(nRow).length, 8, 16, QChar('0')));
    //     }
    //     return 4;
    // }
    return 0;
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

QList<XTTF::TTF_TABLE_RECORD> XTTF::getTableDirectory(quint16 numTables)
{
    QList<TTF_TABLE_RECORD> list;
    qint64 nOffset = 12;
    for (qint32 i = 0, nNumberOfTables = numTables; i < nNumberOfTables; i++) {
        TTF_TABLE_RECORD rec = {};
        rec.tag = read_uint32(nOffset, true);
        rec.checkSum = read_uint32(nOffset + 4, true);
        rec.offset = read_uint32(nOffset + 8, true);
        rec.length = read_uint32(nOffset + 12, true);
        list.append(rec);
        nOffset += 16;
    }
    return list;
}

XTTF::TTF_HEADER XTTF::readHeader()
{
    TTF_HEADER header = {};
    header.sfntVersion = read_uint32(0, true);
    header.numTables = read_uint16(4, true);
    header.searchRange = read_uint16(6, true);
    header.entrySelector = read_uint16(8, true);
    header.rangeShift = read_uint16(10, true);
    return header;
}
