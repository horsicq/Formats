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
#include "xicc.h"

XICC::XICC(QIODevice *pDevice) : XBinary(pDevice)
{
}

XICC::~XICC()
{
}

bool XICC::isValid(PDSTRUCT *pPdStruct)
{
    bool bIsValid = false;

    if (getSize() >= 128) {
        // ICC profiles start with profile size and have specific signatures
        quint32 nProfileSize = read_uint32(0, true);
        quint32 nSignature = read_uint32(12, true);

        // 'scnr' 73636E72h input devices - scanners and digital cameras
        // 'mntr' 6D6E7472h display devices - CRTs and LCDs
        // 'prtr' 70727472h output devices - printers.
        
        bIsValid = (nProfileSize <= getSize()) &&
                   ((nSignature == 0x73636E72) || (nSignature == 0x6D6E7472) || (nSignature == 0x70727472));
    }

    return bIsValid;
}

bool XICC::isValid(QIODevice *pDevice)
{
    XICC xicc(pDevice);

    return xicc.isValid();
}

XBinary::_MEMORY_MAP XICC::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(mapMode)

    XBinary::PDSTRUCT pdStructEmpty = {};

    if (!pPdStruct) {
        pdStructEmpty = XBinary::createPdStruct();
        pPdStruct = &pdStructEmpty;
    }

    XBinary::_MEMORY_MAP result = {};
    result.nBinarySize = getSize();

    qint32 nIndex = 0;

    // Add header
    _MEMORY_RECORD recordHeader = {};
    recordHeader.nIndex = nIndex++;
    recordHeader.filePart = FILEPART_HEADER;
    recordHeader.sName = tr("Header");
    recordHeader.nOffset = 0;
    recordHeader.nSize = 128;
    recordHeader.nAddress = -1;
    result.listRecords.append(recordHeader);

    // Add tag table
    if (getSize() > 128) {
        quint32 nTagCount = read_uint32(128, true);
        
        _MEMORY_RECORD recordTagTable = {};
        recordTagTable.nIndex = nIndex++;
        recordTagTable.filePart = FILEPART_OBJECT;
        recordTagTable.sName = tr("Tag table");
        recordTagTable.nOffset = 128;
        recordTagTable.nSize = 4 + (nTagCount * 12);
        recordTagTable.nAddress = -1;
        result.listRecords.append(recordTagTable);

        // Add tags
        QList<XICC::TAG> listTags = getTags(pPdStruct);

        qint32 nNumberOfTags = listTags.count();

        for (qint32 i = 0; (i < nNumberOfTags) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
            _MEMORY_RECORD record = {};

            record.nIndex = nIndex++;
            record.filePart = FILEPART_OBJECT;
            record.sName = listTags.at(i).sTagName;
            record.nOffset = listTags.at(i).nOffset;
            record.nSize = listTags.at(i).nSize;
            record.nAddress = -1;

            result.listRecords.append(record);
        }
    }

    _handleOverlay(&result);

    return result;
}

XBinary::FT XICC::getFileType()
{
    return FT_ICC;
}

QString XICC::getFileFormatExt()
{
    return "icc";
}

QString XICC::getFileFormatExtsString()
{
    return "ICC Profile (*.icc *.icm)";
}

qint64 XICC::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    return _calculateRawSize(pPdStruct);
}

QString XICC::getVersion()
{
    QString sResult;

    quint32 nVersion = read_uint32(8, true);
    quint8 nMajor = (nVersion >> 24) & 0xFF;
    quint8 nMinor = (nVersion >> 20) & 0x0F;
    quint8 nBugFix = (nVersion >> 16) & 0x0F;

    sResult = QString("%1.%2.%3").arg(QString::number(nMajor), QString::number(nMinor), QString::number(nBugFix));

    return sResult;
}

XICC::HEADER XICC::getHeader()
{
    HEADER result = {};

    result.nProfileSize = read_uint32(0, true);
    result.nCMMType = read_uint32(4, true);
    result.nVersion = read_uint32(8, true);
    result.nDeviceClass = read_uint32(12, true);
    result.nDataColorSpace = read_uint32(16, true);
    result.nPCS = read_uint32(20, true);
    
    read_array(24, (char*)result.nDate, 12);
    
    result.nPlatform = read_uint32(40, true);
    result.nFlags = read_uint32(44, true);
    result.nDeviceManufacturer = read_uint32(48, true);
    result.nDeviceModel = read_uint32(52, true);
    result.nDeviceAttributes = read_uint64(56, true);
    result.nRenderingIntent = read_uint32(64, true);
    result.nIlluminantX = read_uint32(68, true);
    result.nIlluminantY = read_uint32(72, true);
    result.nIlluminantZ = read_uint32(76, true);
    result.nCreator = read_uint32(80, true);
    
    read_array(84, (char*)result.nReserved, 44);

    return result;
}

QList<XICC::TAG> XICC::getTags(PDSTRUCT *pPdStruct)
{
    QList<TAG> listResult;

    if (getSize() > 132) {
        quint32 nTagCount = read_uint32(128, true);
        
        for (quint32 i = 0; (i < nTagCount) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
            TAG tag = _readTag(132 + (i * 12));
            
            if (tag.bValid) {
                listResult.append(tag);
            }
        }
    }

    return listResult;
}

QList<XICC::TAG> XICC::_getTagsBySignature(QList<TAG> *pListTags, quint32 nSignature)
{
    QList<XICC::TAG> listResult;

    qint32 nNumberOfRecords = pListTags->count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (pListTags->at(i).nSignature == nSignature) {
            listResult.append(pListTags->at(i));
        }
    }

    return listResult;
}

QString XICC::getTagName(quint32 nSignature)
{
    QString sResult;

    switch (nSignature) {
        case 0x64657363: sResult = "desc"; break;  // Description
        case 0x63707274: sResult = "cprt"; break;  // Copyright
        case 0x77747074: sResult = "wtpt"; break;  // White point
        case 0x72545243: sResult = "rTRC"; break;  // Red tone reproduction curve
        case 0x67545243: sResult = "gTRC"; break;  // Green tone reproduction curve
        case 0x62545243: sResult = "bTRC"; break;  // Blue tone reproduction curve
        case 0x72585950: sResult = "rXYZ"; break;  // Red colorant
        case 0x67585950: sResult = "gXYZ"; break;  // Green colorant
        case 0x62585950: sResult = "bXYZ"; break;  // Blue colorant
        case 0x646D6E64: sResult = "dmnd"; break;  // Device manufacturer description
        case 0x646D6464: sResult = "dmdd"; break;  // Device model description
        default: sResult = _fourCCToString(nSignature); break;
    }

    return sResult;
}

QString XICC::getDescription(QList<TAG> *pListTags, PDSTRUCT *pPdStruct)
{
    XBinary::PDSTRUCT pdStructEmpty = {};

    if (!pPdStruct) {
        pdStructEmpty = XBinary::createPdStruct();
        pPdStruct = &pdStructEmpty;
    }

    QString sResult;

    QList<XICC::TAG> listDesc = _getTagsBySignature(pListTags, 0x64657363);  // 'desc'

    if (listDesc.count() > 0) {
        TAG tagDesc = listDesc.at(0);
        quint32 nType = read_uint32(tagDesc.nOffset, true);
        
        if (nType == 0x64657363) {  // 'desc' type
            sResult = _readTextType(tagDesc.nOffset);
        } else if (nType == 0x6D6C7563) {  // 'mluc' type
            sResult = _readMultiLocalizedUnicodeType(tagDesc.nOffset);
        }
    }

    return sResult;
}

QString XICC::getDescription()
{
    QList<TAG> listTags = getTags();

    return getDescription(&listTags);
}

QString XICC::getCopyright(QList<TAG> *pListTags, PDSTRUCT *pPdStruct)
{
    XBinary::PDSTRUCT pdStructEmpty = {};

    if (!pPdStruct) {
        pdStructEmpty = XBinary::createPdStruct();
        pPdStruct = &pdStructEmpty;
    }

    QString sResult;

    QList<XICC::TAG> listCopyright = _getTagsBySignature(pListTags, 0x63707274);  // 'cprt'

    if (listCopyright.count() > 0) {
        TAG tagCopyright = listCopyright.at(0);
        quint32 nType = read_uint32(tagCopyright.nOffset, true);
        
        if (nType == 0x74657874) {  // 'text' type
            sResult = _readTextType(tagCopyright.nOffset);
        } else if (nType == 0x6D6C7563) {  // 'mluc' type
            sResult = _readMultiLocalizedUnicodeType(tagCopyright.nOffset);
        }
    }

    return sResult;
}

QString XICC::getCopyright()
{
    QList<TAG> listTags = getTags();

    return getCopyright(&listTags);
}

QString XICC::getDeviceManufacturer()
{
    HEADER header = getHeader();
    return _fourCCToString(header.nDeviceManufacturer);
}

QString XICC::getDeviceModel()
{
    HEADER header = getHeader();
    return _fourCCToString(header.nDeviceModel);
}

QString XICC::getColorSpace()
{
    HEADER header = getHeader();
    return _fourCCToString(header.nDataColorSpace);
}

QString XICC::getDeviceClass()
{
    HEADER header = getHeader();
    
    QString sResult;
    switch (header.nDeviceClass) {
        case 0x73636E72: sResult = "Input"; break;      // 'scnr'
        case 0x6D6E7472: sResult = "Display"; break;    // 'mntr'
        case 0x70727472: sResult = "Output"; break;     // 'prtr'
        case 0x6C696E6B: sResult = "DeviceLink"; break; // 'link'
        case 0x73706163: sResult = "ColorSpace"; break; // 'spac'
        case 0x61627374: sResult = "Abstract"; break;   // 'abst'
        case 0x6E6D636C: sResult = "NamedColor"; break; // 'nmcl'
        default: sResult = _fourCCToString(header.nDeviceClass); break;
    }
    
    return sResult;
}

QString XICC::getMIMEString()
{
    return "application/vnd.iccprofile";
}

XICC::TAG XICC::_readTag(qint64 nOffset)
{
    TAG result = {};

    result.nSignature = read_uint32(nOffset, true);
    result.nOffset = read_uint32(nOffset + 4, true);
    result.nSize = read_uint32(nOffset + 8, true);
    
    result.sTagName = getTagName(result.nSignature);
    result.bValid = (result.nOffset > 0) && (result.nSize > 0) && 
                    (result.nOffset + result.nSize <= getSize());

    return result;
}

QString XICC::_readTextType(qint64 nOffset)
{
    QString sResult;
    
    quint32 nType = read_uint32(nOffset, true);
    
    if (nType == 0x74657874) {  // 'text'
        // Skip type signature (4 bytes) and reserved (4 bytes)
        sResult = read_ansiString(nOffset + 8);
    } else if (nType == 0x64657363) {  // 'desc'
        // Skip type signature (4 bytes) and reserved (4 bytes)
        quint32 nLength = read_uint32(nOffset + 8, true);
        if (nLength > 0) {
            sResult = read_ansiString(nOffset + 12, qMin(nLength, quint32(1000)));
        }
    }
    
    return sResult;
}

QString XICC::_readMultiLocalizedUnicodeType(qint64 nOffset)
{
    QString sResult;
    
    quint32 nType = read_uint32(nOffset, true);
    
    if (nType == 0x6D6C7563) {  // 'mluc'
        // Skip type signature (4 bytes) and reserved (4 bytes)
        quint32 nRecordCount = read_uint32(nOffset + 8, true);
        
        if (nRecordCount > 0) {
            // Read first record (English preferred)
            quint32 nLength = read_uint32(nOffset + 16, true);
            quint32 nRecordOffset = read_uint32(nOffset + 20, true);
            
            if (nLength > 0 && nRecordOffset > 0) {
                sResult = read_unicodeString(nOffset + nRecordOffset, qMin(nLength / 2, quint32(500)), true);
            }
        }
    }
    
    return sResult;
}

QString XICC::_fourCCToString(quint32 nValue)
{
    QString sResult;
    
    char szBuffer[5];
    szBuffer[0] = (nValue >> 24) & 0xFF;
    szBuffer[1] = (nValue >> 16) & 0xFF;
    szBuffer[2] = (nValue >> 8) & 0xFF;
    szBuffer[3] = nValue & 0xFF;
    szBuffer[4] = '\0';
    
    sResult = QString::fromLatin1(szBuffer);
    
    return sResult;
}
