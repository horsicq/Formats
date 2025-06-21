/* Copyright (c) 2017-2025 hors<horsicq@gmail.com>
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
#include "xmsdos.h"

XBinary::XCONVERT _TABLE_XMSDOS_STRUCTID[] = {
    {XMSDOS::STRUCTID_UNKNOWN, "Unknown", QObject::tr("Unknown")},
    {XMSDOS::STRUCTID_IMAGE_DOS_HEADER, "IMAGE_DOS_HEADER", QString("IMAGE_DOS_HEADER")},
    };

XMSDOS::XMSDOS(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress) : XBinary(pDevice, bIsImage, nModuleAddress)
{
}

bool XMSDOS::isValid(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)
    bool bResult = false;

    quint16 magic = get_magic();

    if ((magic == XMSDOS_DEF::S_IMAGE_DOS_SIGNATURE_MZ) || (magic == XMSDOS_DEF::S_IMAGE_DOS_SIGNATURE_ZM)) {
        bResult = true;
    }

    return bResult;
}

bool XMSDOS::isValid(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    XMSDOS xmsdos(pDevice, bIsImage, nModuleAddress);

    return xmsdos.isValid();
}

bool XMSDOS::isExecutable()
{
    return true;
}

XBinary::MODE XMSDOS::getMode(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    XMSDOS xmsdos(pDevice, bIsImage, nModuleAddress);

    return xmsdos.getMode();
}

quint16 XMSDOS::get_magic()
{
    return read_uint16((qint64)offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_magic));
}

qint32 XMSDOS::get_lfanew()
{
    return read_int32(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_lfanew));
}

qint64 XMSDOS::getDosHeaderOffset()
{
    return 0;
}

qint64 XMSDOS::getDosHeaderSize()
{
    return sizeof(XMSDOS_DEF::IMAGE_DOS_HEADER);
}

qint64 XMSDOS::getDosHeaderExOffset()
{
    return 0;
}

qint64 XMSDOS::getDosHeaderExSize()
{
    return sizeof(XMSDOS_DEF::IMAGE_DOS_HEADEREX);
}

XMSDOS_DEF::IMAGE_DOS_HEADER XMSDOS::getDosHeader()
{
    XMSDOS_DEF::IMAGE_DOS_HEADER result = {};
    // TODO
    read_array((qint64)offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_magic), (char *)&result, sizeof(XMSDOS_DEF::IMAGE_DOS_HEADER));

    return result;
}

XMSDOS_DEF::IMAGE_DOS_HEADEREX XMSDOS::getDosHeaderEx()
{
    return _read_IMAGE_DOS_HEADEREX(0);
}

void XMSDOS::setDosHeader(XMSDOS_DEF::IMAGE_DOS_HEADER *pDosHeader)
{
    // TODO
    write_array((qint64)offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_magic), (char *)pDosHeader, sizeof(XMSDOS_DEF::IMAGE_DOS_HEADER));
}

void XMSDOS::setDosHeaderEx(XMSDOS_DEF::IMAGE_DOS_HEADEREX *pDosHeaderEx)
{
    // TODO
    write_array((qint64)offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_magic), (char *)pDosHeaderEx, sizeof(XMSDOS_DEF::IMAGE_DOS_HEADEREX));
}

XMSDOS_DEF::IMAGE_DOS_HEADEREX XMSDOS::_read_IMAGE_DOS_HEADEREX(qint64 nOffset)
{
    XMSDOS_DEF::IMAGE_DOS_HEADEREX result = {};

    read_array(nOffset, (char *)&result, sizeof(XMSDOS_DEF::IMAGE_DOS_HEADEREX));

    return result;
}

void XMSDOS::set_e_magic(quint16 nValue)
{
    write_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_magic), nValue);
}

void XMSDOS::set_e_cblp(quint16 nValue)
{
    write_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_cblp), nValue);
}

void XMSDOS::set_e_cp(quint16 nValue)
{
    write_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_cp), nValue);
}

void XMSDOS::set_e_crlc(quint16 nValue)
{
    write_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_crlc), nValue);
}

void XMSDOS::set_e_cparhdr(quint16 nValue)
{
    write_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_cparhdr), nValue);
}

void XMSDOS::set_e_minalloc(quint16 nValue)
{
    write_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_minalloc), nValue);
}

void XMSDOS::set_e_maxalloc(quint16 nValue)
{
    write_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_maxalloc), nValue);
}

void XMSDOS::set_e_ss(quint16 nValue)
{
    write_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_ss), nValue);
}

void XMSDOS::set_e_sp(quint16 nValue)
{
    write_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_sp), nValue);
}

void XMSDOS::set_e_csum(quint16 nValue)
{
    write_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_csum), nValue);
}

void XMSDOS::set_e_ip(quint16 nValue)
{
    write_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_ip), nValue);
}

void XMSDOS::set_e_cs(quint16 nValue)
{
    write_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_cs), nValue);
}

void XMSDOS::set_e_lfarlc(quint16 nValue)
{
    write_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_lfarlc), nValue);
}

void XMSDOS::set_e_ovno(quint16 nValue)
{
    write_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_ovno), nValue);
}

void XMSDOS::set_e_res(qint32 nPosition, quint16 nValue)
{
    if (nPosition < 4)  // TODO nPosition>=0
    {
        write_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_res) + sizeof(quint16) * nPosition, nValue);
    }
}

void XMSDOS::set_e_oemid(quint16 nValue)
{
    write_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_oemid), nValue);
}

void XMSDOS::set_e_oeminfo(quint16 nValue)
{
    write_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_oeminfo), nValue);
}

void XMSDOS::set_e_res2(qint32 nPosition, quint16 nValue)
{
    if (nPosition < 10)  // TODO nPosition>=0
    {
        write_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_res2) + sizeof(quint16) * nPosition, nValue);
    }
}

void XMSDOS::set_e_lfanew(quint32 nValue)
{
    write_uint32(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_lfanew), nValue);
}

quint16 XMSDOS::get_e_magic()
{
    return read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_magic));
}

quint16 XMSDOS::get_e_cblp()
{
    return read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_cblp));
}

quint16 XMSDOS::get_e_cp()
{
    return read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_cp));
}

quint16 XMSDOS::get_e_crlc()
{
    return read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_crlc));
}

quint16 XMSDOS::get_e_cparhdr()
{
    return read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_cparhdr));
}

quint16 XMSDOS::get_e_minalloc()
{
    return read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_minalloc));
}

quint16 XMSDOS::get_e_maxalloc()
{
    return read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_maxalloc));
}

quint16 XMSDOS::get_e_ss()
{
    return read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_ss));
}

quint16 XMSDOS::get_e_sp()
{
    return read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_sp));
}

quint16 XMSDOS::get_e_csum()
{
    return read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_csum));
}

quint16 XMSDOS::get_e_ip()
{
    return read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_ip));
}

quint16 XMSDOS::get_e_cs()
{
    return read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_cs));
}

quint16 XMSDOS::get_e_lfarlc()
{
    return read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_lfarlc));
}

quint16 XMSDOS::get_e_ovno()
{
    return read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_ovno));
}

quint16 XMSDOS::get_e_res(qint32 nPosition)
{
    quint16 nResult = 0;

    if (nPosition < 10)  // TODO nPosition>=0
    {
        nResult = read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_res) + sizeof(quint16) * nPosition);
    }

    return nResult;
}

quint16 XMSDOS::get_e_oemid()
{
    return read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_oemid));
}

quint16 XMSDOS::get_e_oeminfo()
{
    return read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_oeminfo));
}

quint16 XMSDOS::get_e_res2(qint32 nPosition)
{
    quint16 nResult = 0;

    if (nPosition < 10)  // TODO nPosition>=0
    {
        nResult = read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_res2) + sizeof(quint16) * nPosition);
    }

    return nResult;
}

quint32 XMSDOS::get_e_lfanew()
{
    return read_uint32(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_lfanew));
}

QList<XBinary::MAPMODE> XMSDOS::getMapModesList()
{
    QList<MAPMODE> listResult;

    listResult.append(MAPMODE_SEGMENTS);

    return listResult;
}

XBinary::_MEMORY_MAP XMSDOS::getMemoryMap(XBinary::MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(mapMode)
    Q_UNUSED(pPdStruct)

    _MEMORY_MAP result = {};

    qint32 nIndex = 0;

    result.fileType = FT_MSDOS;

    result.sArch = QString("8086");
    ;
    result.sType = getTypeAsString();
    result.mode = MODE_16;
    result.nBinarySize = getSize();
    result.nImageSize = getImageSize();
    result.nModuleAddress = getModuleAddress();
    result.endian = ENDIAN_LITTLE;

    // qint64 nMaxOffset = (get_e_cp() - 1) * 512 + get_e_cblp();  // TODO Check if get_e_cp()=0
    qint64 nMaxOffset = (quint32)get_e_cp() * 0x200 - ((-get_e_cblp()) & 0x1ff);

    qint64 nHeaderOffset = 0;
    qint64 nHeaderSize = (quint32)(get_e_cparhdr() * 16);
    // qint64 nCodeOffset = (get_e_cparhdr() * 16) + (get_e_cs() * 16);

    // result.nEntryPointAddress = ((get_e_cs() << 16) + get_e_ip()) & 0xFFFFFFFF;
    result.nEntryPointAddress = result.nModuleAddress + getSegmentAddress(get_e_cs(), get_e_ip());

    // if (get_e_cs() * 16 + get_e_ip() >= 0x100000) {
    //     nCodeAddress = 0x100000;
    //     result.nEntryPointAddress = get_e_cs() * 0x10000 + get_e_ip();
    //     nCodeOffset = get_e_cparhdr() * 16;  // TODO Check
    // }

    result.nStartLoadOffset = (get_e_cparhdr() * 16);

    // qint64 nCodeSize = 0;
    qint64 nOverlayOffset = nMaxOffset;
    qint64 nOverlaySize = qMax(getSize() - nMaxOffset, (qint64)0);

    // if (nMaxOffset > nCodeOffset) {
    //     nCodeSize = S_ALIGN_UP(nMaxOffset - nCodeOffset, 512);
    //     //    nCodeSize=qMin(nCodeSize,getSize());
    //     nOverlayOffset = nMaxOffset;
    //     nOverlaySize = qMax(getSize() - nMaxOffset, (qint64)0);
    // } else {
    //     if (getSize() > nCodeOffset) {
    //         nCodeSize = getSize() - nCodeOffset;
    //     }
    // }

    {
        _MEMORY_RECORD record = {};
        record.sName = QString("MSDOS %1").arg(tr("Header"));
        record.nSize = nHeaderSize;
        record.nOffset = nHeaderOffset;
        record.nAddress = -1;
        record.segment = ADDRESS_SEGMENT_UNKNOWN;
        record.type = MMT_HEADER;
        record.nIndex = nIndex++;

        result.listRecords.append(record);
    }

    qint32 nSegmentIndex = 1;

    qint64 nDelta = nOverlayOffset - nHeaderSize;

    if (nDelta > 0) {
        _MEMORY_RECORD record = {};

        record.bIsVirtual = false;
        record.nSize = nDelta;
        record.nOffset = nHeaderSize;
        record.nAddress = nSegmentIndex * 0x10000000;

        record.segment = ADDRESS_SEGMENT_CODE;  // CODE
        record.type = MMT_LOADSEGMENT;
        record.nIndex = nIndex++;

        result.listRecords.append(record);

        nSegmentIndex++;
    }

    // if (nCodeSize > 0) {
    //     _MEMORY_RECORD record = {};

    //     record.nSize = nCodeSize;
    //     record.nOffset = nCodeOffset;
    //     record.nAddress = nSegmentIndex * 0x10000000;

    //     record.segment = ADDRESS_SEGMENT_CODE;  // CODE
    //     record.type = MMT_LOADSEGMENT;
    //     record.nIndex = nIndex++;

    //     result.nCodeBase = record.nAddress;

    //     result.listRecords.append(record);

    //     nSegmentIndex++;
    // }

    _handleOverlay(&result);

    return result;
}

qint64 XMSDOS::getImageSize()
{
    return 0x10000;  // TODO Check
}

qint64 XMSDOS::getModuleAddress()
{
    return 0x10000000;  // TODO Check
    // return 0x100000;
}

QMap<quint64, QString> XMSDOS::getImageMagics()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0x5A4D, "IMAGE_DOS_SIGNATURE");
    mapResult.insert(0x4D5A, "IMAGE_DOS_SIGNATURE_ZM");

    return mapResult;
}

QMap<quint64, QString> XMSDOS::getImageMagicsS()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0x5A4D, "DOS_SIGNATURE");
    mapResult.insert(0x4D5A, "DOS_SIGNATURE_ZM");

    return mapResult;
}

bool XMSDOS::isLE()
{
    quint32 nOffset = get_e_lfanew();
    quint16 nNew = read_uint16(nOffset);

    return (nNew == 0x454C);  // TODO const
}

bool XMSDOS::isLX()
{
    quint32 nOffset = get_e_lfanew();
    quint16 nNew = read_uint16(nOffset);

    return (nNew == 0x584C);  // TODO const
}

bool XMSDOS::isNE()
{
    quint32 nOffset = get_e_lfanew();
    quint16 nNew = read_uint16(nOffset);

    return (nNew == 0x454E);  // TODO const
}

bool XMSDOS::isPE()
{
    quint32 nOffset = get_e_lfanew();
    quint16 nNew = read_uint16(nOffset);

    return (nNew == 0x4550);  // TODO const
}

bool XMSDOS::isRichSignaturePresent()
{
    bool bResult = false;

    qint32 nOffset = sizeof(XMSDOS_DEF::IMAGE_DOS_HEADER);
    qint32 nSize = get_lfanew() - sizeof(XMSDOS_DEF::IMAGE_DOS_HEADER);

    if ((nSize > 0) && (nSize <= 0x400)) {
        QByteArray baStub = read_array(nOffset, nSize);

        bResult = baStub.contains("Rich");
    }

    return bResult;
}

QList<XMSDOS::MS_RICH_RECORD> XMSDOS::getRichSignatureRecords(PDSTRUCT *pPdStruct)
{
    QList<MS_RICH_RECORD> listResult;

    qint64 nOffset = find_ansiString(getDosStubOffset(), getDosStubSize(), "Rich", pPdStruct);

    if (nOffset != -1) {
        quint32 nXORkey = read_uint32(nOffset + 4);

        qint64 nCurrentOffset = nOffset - 4;

        while ((nCurrentOffset > getDosStubOffset()) && isPdStructNotCanceled(pPdStruct))  // TODO optimize
        {
            quint32 nTemp = read_uint32(nCurrentOffset) ^ nXORkey;

            if (nTemp == 0x536e6144)  // DanS
            {
                nCurrentOffset += 16;

                for (; nCurrentOffset < nOffset; nCurrentOffset += 8) {
                    MS_RICH_RECORD record = {};

                    quint32 nValue1 = read_uint32(nCurrentOffset) ^ nXORkey;
                    record.nId = (quint16)(nValue1 >> 16);
                    record.nVersion = (quint16)(nValue1 & 0xFFFF);

                    quint32 nValue2 = read_uint32(nCurrentOffset + 4) ^ nXORkey;
                    record.nCount = nValue2;

                    listResult.append(record);
                }

                break;
            }

            nCurrentOffset -= 4;
        }
    }

    return listResult;
}

qint32 XMSDOS::getNumberOfRichIDs(PDSTRUCT *pPdStruct)
{
    QList<MS_RICH_RECORD> listRichSignatureRecords = getRichSignatureRecords(pPdStruct);

    return getNumberOfRichIDs(&listRichSignatureRecords);
}

qint32 XMSDOS::getNumberOfRichIDs(QList<XMSDOS::MS_RICH_RECORD> *pListRichSignatureRecords)
{
    return pListRichSignatureRecords->count();
}

bool XMSDOS::isRichVersionPresent(quint32 nVersion, PDSTRUCT *pPdStruct)
{
    QList<MS_RICH_RECORD> listRichSignatureRecords = getRichSignatureRecords(pPdStruct);

    return isRichVersionPresent(nVersion, &listRichSignatureRecords);
}

bool XMSDOS::isRichVersionPresent(quint32 nVersion, QList<XMSDOS::MS_RICH_RECORD> *pListRichSignatureRecords)
{
    bool bResult = false;

    qint32 nNumberOfRichVersions = pListRichSignatureRecords->count();

    for (qint32 i = 0; i < nNumberOfRichVersions; i++) {
        if (pListRichSignatureRecords->at(i).nVersion == nVersion) {
            bResult = true;

            break;
        }
    }

    return bResult;
}

quint32 XMSDOS::getRichVersion(QList<MS_RICH_RECORD> *pListRichSignatureRecords, qint32 nPosition)
{
    quint32 nResult = 0;
    qint32 nNumberOfRichVersions = pListRichSignatureRecords->count();

    if ((nPosition < nNumberOfRichVersions) && (nPosition >= 0)) {
        nResult = pListRichSignatureRecords->at(nPosition).nVersion;
    }

    return nResult;
}

quint32 XMSDOS::getRichID(QList<MS_RICH_RECORD> *pListRichSignatureRecords, qint32 nPosition)
{
    quint32 nResult = 0;
    qint32 nNumberOfRichVersions = pListRichSignatureRecords->count();

    if ((nPosition < nNumberOfRichVersions) && (nPosition >= 0)) {
        nResult = pListRichSignatureRecords->at(nPosition).nId;
    }

    return nResult;
}

quint32 XMSDOS::getRichCount(QList<MS_RICH_RECORD> *pListRichSignatureRecords, qint32 nPosition)
{
    quint32 nResult = 0;
    qint32 nNumberOfRichVersions = pListRichSignatureRecords->count();

    if ((nPosition < nNumberOfRichVersions) && (nPosition >= 0)) {
        nResult = pListRichSignatureRecords->at(nPosition).nCount;
    }

    return nResult;
}

qint64 XMSDOS::getDosStubSize()
{
    qint64 nSize = (qint64)get_lfanew() - sizeof(XMSDOS_DEF::IMAGE_DOS_HEADEREX);

    nSize = qMax(nSize, (qint64)0);

    return nSize;
}

qint64 XMSDOS::getDosStubOffset()
{
    return sizeof(XMSDOS_DEF::IMAGE_DOS_HEADEREX);
}

QByteArray XMSDOS::getDosStub()
{
    return read_array(getDosStubOffset(), getDosStubSize());
}

bool XMSDOS::isDosStubPresent()
{
    return getDosStubSize() != 0;
}

bool XMSDOS::removeDosStub()
{
    bool bResult = false;

    return bResult;
}

bool XMSDOS::addDosStub(const QString &sFileName)
{
    Q_UNUSED(sFileName)

    bool bResult = false;

    return bResult;
}

XBinary::MODE XMSDOS::getMode()
{
    return MODE_16;
}

QString XMSDOS::getArch()
{
    return QString("8086");
}

XBinary::ENDIAN XMSDOS::getEndian()
{
    return ENDIAN_LITTLE;
}

XBinary::FT XMSDOS::getFileType()
{
    return FT_MSDOS;
}

qint32 XMSDOS::getType()
{
    qint32 nResult = TYPE_EXE;

    FT fileType = getFileType();

    if (fileType == FT_MSDOS) {
        nResult = TYPE_EXE;
    }

    return nResult;
}

XBinary::OSNAME XMSDOS::getOsName()
{
    return OSNAME_MSDOS;
}

QString XMSDOS::typeIdToString(qint32 nType)
{
    QString sResult = tr("Unknown");

    switch (nType) {
        case TYPE_UNKNOWN: sResult = tr("Unknown"); break;
        case TYPE_EXE: sResult = QString("EXE"); break;
    }

    return sResult;
}

QString XMSDOS::structIDToString(quint32 nID)
{
    return XBinary::XCONVERT_idToTransString(nID, _TABLE_XMSDOS_STRUCTID, sizeof(_TABLE_XMSDOS_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QList<XBinary::DATA_HEADER> XMSDOS::getDataHeaders(const DATA_HEADERS_OPTIONS &dataHeadersOptions, PDSTRUCT *pPdStruct)
{
    QList<XBinary::DATA_HEADER> listResult;

    if (dataHeadersOptions.nID == STRUCTID_UNKNOWN) {
        DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
        _dataHeadersOptions.bChildren = true;
        _dataHeadersOptions.dsID_parent = _addDefaultHeaders(&listResult, pPdStruct);
        _dataHeadersOptions.dhMode = XBinary::DHMODE_HEADER;
        _dataHeadersOptions.nID = STRUCTID_IMAGE_DOS_HEADER;

        listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
    } else {
        qint64 nStartOffset = locationToOffset(dataHeadersOptions.pMemoryMap, dataHeadersOptions.locType, dataHeadersOptions.nLocation);

        if (nStartOffset != -1) {
            XBinary::DATA_HEADER dataHeader = {};
            dataHeader.dsID_parent = dataHeadersOptions.dsID_parent;
            dataHeader.dsID.sGUID = generateUUID();
            dataHeader.dsID.fileType = dataHeadersOptions.pMemoryMap->fileType;
            dataHeader.dsID.nID = dataHeadersOptions.nID;
            dataHeader.locType = dataHeadersOptions.locType;
            dataHeader.nLocation = dataHeadersOptions.nLocation;
            dataHeader.sName = structIDToString(dataHeadersOptions.nID);
            dataHeader.dhMode = dataHeadersOptions.dhMode;
            dataHeader.nSize = dataHeadersOptions.nSize;
            dataHeader.nCount = dataHeadersOptions.nCount;

            // struct IMAGE_DOS_HEADER {
            //     quint16 e_magic;    /* 00: MZ Header signature */
            //     quint16 e_cblp;     /* 02: Bytes on last page of file */
            //     quint16 e_cp;       /* 04: Pages in file */
            //     quint16 e_crlc;     /* 06: Relocations */
            //     quint16 e_cparhdr;  /* 08: Size of header in paragraphs */
            //     quint16 e_minalloc; /* 0a: Minimum extra paragraphs needed */
            //     quint16 e_maxalloc; /* 0c: Maximum extra paragraphs needed */
            //     quint16 e_ss;       /* 0e: Initial (relative) SS value */
            //     quint16 e_sp;       /* 10: Initial SP value */
            //     quint16 e_csum;     /* 12: Checksum */
            //     quint16 e_ip;       /* 14: Initial IP value */
            //     quint16 e_cs;       /* 16: Initial (relative) CS value */
            //     quint16 e_lfarlc;   /* 18: File address of relocation table */
            //     quint16 e_ovno;     /* 1a: Overlay number */
            // };

            if (dataHeadersOptions.nID == STRUCTID_IMAGE_DOS_HEADER) {
                dataHeader.nSize = sizeof(XMSDOS_DEF::IMAGE_DOS_HEADER);
                dataHeader.listRecords.append(getDataRecordDV(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_magic), 2, "e_magic", VT_WORD, DRF_UNKNOWN,
                                                              dataHeadersOptions.pMemoryMap->endian, XMSDOS::getImageMagicsS(), false));
                dataHeader.listRecords.append(getDataRecord(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_cblp), 2, "e_cblp", VT_WORD, DRF_UNKNOWN,
                                                              dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(getDataRecord(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_cp), 2, "e_cp", VT_WORD, DRF_UNKNOWN,
                                                              dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(getDataRecord(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_crlc), 2, "e_crlc", VT_WORD, DRF_UNKNOWN,
                                                              dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(getDataRecord(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_cparhdr), 2, "e_cparhdr", VT_WORD, DRF_UNKNOWN,
                                                              dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(getDataRecord(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_minalloc), 2, "e_minalloc", VT_WORD, DRF_UNKNOWN,
                                                              dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(getDataRecord(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_maxalloc), 2, "e_maxalloc", VT_WORD, DRF_UNKNOWN,
                                                              dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(getDataRecord(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_ss), 2, "e_ss", VT_WORD, DRF_UNKNOWN,
                                                              dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(getDataRecord(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_sp), 2, "e_sp", VT_WORD, DRF_UNKNOWN,
                                                              dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(getDataRecord(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_csum), 2, "e_csum", VT_WORD, DRF_UNKNOWN,
                                                              dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(getDataRecord(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_ip), 2, "e_ip", VT_WORD, DRF_UNKNOWN,
                                                              dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(getDataRecord(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_cs), 2, "e_cs", VT_WORD, DRF_UNKNOWN,
                                                              dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(getDataRecord(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_lfarlc), 2, "e_lfarlc", VT_WORD, DRF_UNKNOWN,
                                                              dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(getDataRecord(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_ovno), 2, "e_ovno", VT_WORD, DRF_UNKNOWN,
                                                              dataHeadersOptions.pMemoryMap->endian));
            }

            if (dataHeader.nSize) {
                listResult.append(dataHeader);
            }

            if (dataHeadersOptions.bChildren) {

            }
        }
    }

    return listResult;
}
