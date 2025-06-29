// Copyright (c) 2017-2025 hors<horsicq@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#include "xdjvu.h"

XDJVU::XDJVU(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress) : XBinary(pDevice, bIsImage, nModuleAddress)
{
    g_header = {};
}

bool XDJVU::isValid(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    bool bResult = false;

    quint64 nSize = getSize();

    if (nSize >= 12) {
        QString sSignature = read_ansiString(0, 8);
        
        if ((sSignature == "AT&TFORM") || (sSignature == "SDJVFORM")) {
            QString sType = read_ansiString(12, 4);
            
            if ((sType == "DJVU") || (sType == "DJVM") || (sType == "DJVI") || (sType.startsWith("THUM"))) {
                bResult = true;
            }
        }
    }

    return bResult;
}

QString XDJVU::getVersion()
{
    return "";
}

XBinary::FT XDJVU::getFileType()
{
    return FT_DJVU;
}

XDJVU::HEADER XDJVU::getHeader()
{
    if (g_header.sSignature == "") {
        g_header = _getHeader();
    }

    return g_header;
}

bool XDJVU::isValid(QIODevice *pDevice)
{
    XDJVU djvu(pDevice);

    return djvu.isValid();
}

QString XDJVU::getFileFormatExt()
{
    return "djvu";
}

qint64 XDJVU::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    qint64 nResult = 0;

    HEADER header = getHeader();

    if (header.bIsValid) {
        nResult = header.nSize + 12;
    }

    return nResult;
}

QString XDJVU::getMIMEString()
{
    return "application/x-djvu";
}

QString XDJVU::getFileFormatExtsString()
{
    return "DjVu (*.djvu *.djv)";
}

bool XDJVU::isSecure()
{
    return getHeader().bIsSecure;
}

quint32 XDJVU::getDocumentSize()
{
    return getHeader().nSize;
}

XDJVU::INFO_RECORD XDJVU::getInfoRecord(qint64 nOffset, PDSTRUCT *pPdStruct)
{
    return _getInfoRecord(nOffset, pPdStruct);
}

QMap<quint64, QString> XDJVU::getImageTypes()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0, "UNKNOWN");
    mapResult.insert(1, "Single page");
    mapResult.insert(2, "Multi-page");
    mapResult.insert(3, "Multi-file");
    mapResult.insert(4, "Thumbnails");
    mapResult.insert(5, "Secure");

    return mapResult;
}

QMap<quint64, QString> XDJVU::getImageTypesS()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0, "Unknown");
    mapResult.insert(1, "Single");
    mapResult.insert(2, "Multi");
    mapResult.insert(3, "MultiFile");
    mapResult.insert(4, "Thumbnails");
    mapResult.insert(5, "Secure");

    return mapResult;
}

XDJVU::HEADER XDJVU::_getHeader()
{
    HEADER result = {};

    quint64 nSize = getSize();

    if (nSize >= 12) {
        result.sSignature = read_ansiString(0, 8);
        
        if (result.sSignature == "AT&TFORM") {
            result.bIsSecure = false;
            result.nSize = read_uint32(8, true); // Big endian
            
            QString sType = read_ansiString(12, 4);
            
            if (sType == "DJVU") {
                result.type = TYPE_SINGLE_PAGE;
                result.bIsValid = true;
            } else if (sType == "DJVM") {
                result.type = TYPE_MULTI_PAGE;
                result.bIsValid = true;
            } else if (sType == "DJVI") {
                result.type = TYPE_MULTI_FILE;
                result.bIsValid = true;
            } else if (sType.startsWith("THUM")) {
                result.type = TYPE_THUMBNAILS;
                result.bIsValid = true;
            }
        } else if (result.sSignature == "SDJVFORM") {
            result.bIsSecure = true;
            result.type = TYPE_SECURE;
            result.nSize = read_uint32(8, true); // Big endian
            result.bIsValid = true;
        }
    }

    return result;
}

QList<XDJVU::CHUNK_RECORD> XDJVU::_getChunkRecords(PDSTRUCT *pPdStruct)
{
    QList<CHUNK_RECORD> listResult;

    HEADER header = getHeader();

    if (header.bIsValid && !header.bIsSecure) {
        qint64 nOffset = 16; // Start after main header
        qint64 nEndOffset = qMin((qint64)(header.nSize + 8), getSize());

        while (nOffset < nEndOffset) {
            if (nOffset + 8 > nEndOffset) break;

            CHUNK_RECORD record = {};
            record.nOffset = nOffset;
            record.sName = read_ansiString(nOffset, 4);
            record.nSize = read_uint32(nOffset + 4, true); // Big endian
            record.nDataOffset = nOffset + 8;
            record.nDataSize = record.nSize;
            record.nHeaderSize = 8;

            if (!_isChunkValid(record.sName)) {
                break;
            }

            if (record.nDataOffset + record.nDataSize > nEndOffset) {
                break;
            }

            listResult.append(record);

            nOffset = record.nDataOffset + record.nDataSize;
            
            // Chunks are aligned on even boundaries
            if (nOffset & 1) {
                nOffset++;
            }

            if (pPdStruct) {
                if (pPdStruct->bIsStop) {
                    break;
                }
            }
        }
    }

    return listResult;
}

XDJVU::INFO_RECORD XDJVU::_getInfoRecord(qint64 nOffset, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    INFO_RECORD result = {};

    if (nOffset + 10 <= getSize()) {
        result.nWidth = read_uint16(nOffset, true); // Big endian
        result.nHeight = read_uint16(nOffset + 2, true); // Big endian
        result.nMinorVersion = read_uint8(nOffset + 4);
        result.nMajorVersion = read_uint8(nOffset + 5);
        result.nDPI = read_uint16(nOffset + 6, false); // Little endian (yes, it's mixed!)
        result.nGamma = read_uint8(nOffset + 8);
        result.nFlags = read_uint8(nOffset + 9);
        
        if (nOffset + 12 <= getSize()) {
            result.nRotation = read_uint8(nOffset + 10);
            result.nReserved = read_uint8(nOffset + 11);
        }
    }

    return result;
}

bool XDJVU::_isChunkValid(const QString &sChunkName)
{
    // List of known DjVu chunk names
    QStringList listValidChunks = {
        "INFO", "Sjbz", "FG44", "BG44", "FGbz", "BGjp", "BGbz",
        "TXTa", "TXTz", "ANTa", "ANTz", "DJBZ", "FORM", "DIRM",
        "NAVM", "Smmr", "INCL", "CIDa", "LTAn", "KTAn"
    };

    return listValidChunks.contains(sChunkName);
}
