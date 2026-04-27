/* Copyright (c) 2017-2026 hors<horsicq@gmail.com>
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
#include "xcliassembly.h"

XBinary::XCONVERT _TABLE_XCLIASSEMBLY_STRUCTID[] = {
    {XCLIAssembly::STRUCTID_UNKNOWN, "Unknown", QObject::tr("Unknown")},
    {XCLIAssembly::STRUCTID_MZDOSHEADER, "MZDOSHEADER", QString("MZ DOS Header")},
    {XCLIAssembly::STRUCTID_NTHEADER, "NTHEADER", QString("NT Header")},
    {XCLIAssembly::STRUCTID_CLRHEADER, "CLRHEADER", QString("CLR Header")},
};

XCLIAssembly::XCLIAssembly(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress) : XFilePart(pDevice, bIsImage, nModuleAddress)
{
    setParentFileType(FT_PE);
    setFilePart(FILEPART_DATA);
    setFilePartName(QString("CLI Assembly"));
    g_nNetHeaderOffset = -1;
    g_nNetMetaDataOffset = -1;
}

XCLIAssembly::~XCLIAssembly()
{
}

bool XCLIAssembly::isValid(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    bool bResult = false;

    // TODO

    return bResult;
}

bool XCLIAssembly::isValid(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress, PDSTRUCT *pPdStruct)
{
    XCLIAssembly xcliassembly(pDevice, bIsImage, nModuleAddress);

    return xcliassembly.isValid(pPdStruct);
}

XBinary::MODE XCLIAssembly::getMode(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    XCLIAssembly xcliassembly(pDevice, bIsImage, nModuleAddress);

    return xcliassembly.getMode();
}

bool XCLIAssembly::isExecutable()
{
    return true;
}

QList<XBinary::MAPMODE> XCLIAssembly::getMapModesList()
{
    QList<MAPMODE> listResult;

    listResult.append(MAPMODE_REGIONS);

    return listResult;
}

XBinary::_MEMORY_MAP XCLIAssembly::getMemoryMap(XBinary::MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(mapMode)

    XBinary::PDSTRUCT pdStructEmpty = {};

    if (!pPdStruct) {
        pdStructEmpty = XBinary::createPdStruct();
        pPdStruct = &pdStructEmpty;
    }

    _MEMORY_MAP result = {};

    qint64 nTotalSize = getSize();

    result.nModuleAddress = getModuleAddress();
    result.nEntryPointAddress = result.nModuleAddress;
    result.nBinarySize = nTotalSize;
    result.nImageSize = getImageSize();
    result.fileType = getFileType();
    result.mode = getMode();
    result.sArch = getArch();
    result.endian = getEndian();
    result.sType = getTypeAsString();

    _MEMORY_RECORD record = {};
    record.nAddress = result.nModuleAddress;
    record.nOffset = 0;
    record.nSize = nTotalSize;
    record.nIndex = 0;

    result.listRecords.append(record);

    _handleOverlay(&result);

    return result;
}

QString XCLIAssembly::getArch()
{
    QString sResult = QString("MSIL");

    return sResult;
}

XBinary::MODE XCLIAssembly::getMode()
{
    MODE modeResult = MODE_32;

    // TODO

    return modeResult;
}

XBinary::ENDIAN XCLIAssembly::getEndian()
{
    return ENDIAN_LITTLE;
}

qint64 XCLIAssembly::getImageSize()
{
    return getSize();
}

XBinary::FT XCLIAssembly::getFileType()
{
    return FT_CLI_ASSEMBLY;
}

QString XCLIAssembly::getVersion()
{
    return g_sVersion;
}

void XCLIAssembly::setNetHeaderOffset(qint64 nOffset)
{
    g_nNetHeaderOffset = nOffset;
}

void XCLIAssembly::setNetMetaDataOffset(qint64 nOffset)
{
    g_nNetMetaDataOffset = nOffset;
}

void XCLIAssembly::setVersion(const QString &sVersion)
{
    g_sVersion = sVersion;
}

qint32 XCLIAssembly::getType()
{
    qint32 nResult = TYPE_EXE;

    return nResult;
}

XBinary::OSNAME XCLIAssembly::getOsName()
{
    return OSNAME_WINDOWS;
}

QString XCLIAssembly::typeIdToString(qint32 nType)
{
    QString sResult = tr("Unknown");

    switch (nType) {
        case TYPE_UNKNOWN: sResult = tr("Unknown"); break;
        case TYPE_EXE: sResult = QString("EXE"); break;
        case TYPE_DLL: sResult = QString("DLL"); break;
    }

    return sResult;
}

QString XCLIAssembly::getMIMEString()
{
    return "application/x-msdownload";
}

QString XCLIAssembly::getFileFormatExt()
{
    if (getType() == TYPE_DLL) {
        return "dll";
    }

    return "exe";
}

QString XCLIAssembly::getFileFormatExtsString()
{
    return "CLI Assembly files (*.exe *.dll);;";
}

qint64 XCLIAssembly::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    return getSize();
}

QString XCLIAssembly::structIDToString(quint32 nID)
{
    return XBinary::XCONVERT_idToTransString(nID, _TABLE_XCLIASSEMBLY_STRUCTID, sizeof(_TABLE_XCLIASSEMBLY_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QString XCLIAssembly::structIDToFtString(quint32 nID)
{
    return XBinary::XCONVERT_idToFtString(nID, _TABLE_XCLIASSEMBLY_STRUCTID, sizeof(_TABLE_XCLIASSEMBLY_STRUCTID) / sizeof(XBinary::XCONVERT));
}

quint32 XCLIAssembly::ftStringToStructID(const QString &sFtString)
{
    return XCONVERT_ftStringToId(sFtString, _TABLE_XCLIASSEMBLY_STRUCTID, sizeof(_TABLE_XCLIASSEMBLY_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QList<XBinary::DATA_HEADER> XCLIAssembly::getDataHeaders(const DATA_HEADERS_OPTIONS &dataHeadersOptions, PDSTRUCT *pPdStruct)
{
    QList<DATA_HEADER> listResult;

    // TODO

    return listResult;
}

QList<XBinary::FPART> XCLIAssembly::getFileParts(quint32 nFileParts, qint32 nLimit, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(nLimit)
    Q_UNUSED(pPdStruct)

    QList<FPART> listResult;

    qint64 nTotalSize = getSize();

    if (nFileParts & FILEPART_HEADER) {
        FPART record = {};

        record.filePart = FILEPART_HEADER;
        record.nFileOffset = 0;
        record.nFileSize = nTotalSize;
        record.nVirtualAddress = getModuleAddress();
        record.sName = tr("Header");

        listResult.append(record);
    }

    return listResult;
}
