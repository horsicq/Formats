/* Copyright (c) 2025-2026 hors<horsicq@gmail.com>
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
#include "example_class.h"

XBinary::XCONVERT _TABLE_EXAMPLE_CLASS_STRUCTID[] = {{EXAMPLE_CLASS::STRUCTID_UNKNOWN, "Unknown", QObject::tr("Unknown")},
                                                     {EXAMPLE_CLASS::STRUCTID_ORIGINALHEADERNAME, "ORIGINALHEADERNAME", QString("ORIGINALHEADERNAME")},
                                                     {EXAMPLE_CLASS::STRUCTID_DATA_STRUCT1, "DATA_STRUCT1", QString("DATA_STRUCT1")},
                                                     {EXAMPLE_CLASS::STRUCTID_DATA_STRUCT2, "DATA_STRUCT2", QString("DATA_STRUCT2")}};

EXAMPLE_CLASS::EXAMPLE_CLASS(QIODevice *pDevice) : XBinary(pDevice)
{
    // Constructor implementation
}

EXAMPLE_CLASS::~EXAMPLE_CLASS()
{
    // Destructor implementation
}

bool EXAMPLE_CLASS::isValid(PDSTRUCT *pPdStruct)
{
    // Check if the file is valid
    bool bResult = false;

    if (getSize() > 0) {
        // Perform validation checks here
        bResult = true;  // Replace with actual validation logic
    }

    return bResult;
}

XBinary::FT EXAMPLE_CLASS::getFileType()
{
    // Return the file type
    return XBinary::FT_UNKNOWN;  // Replace with actual implementation
}

XBinary::MODE EXAMPLE_CLASS::getMode()
{
    // Return the mode
    return XBinary::MODE_UNKNOWN;  // Replace with actual implementation
}

QString EXAMPLE_CLASS::getMIMEString()
{
    // Return the MIME type string
    return QString();  // Replace with actual implementation
}

qint32 EXAMPLE_CLASS::getType()
{
    // Return the type
    return 0;  // Replace with actual implementation
}

QString EXAMPLE_CLASS::typeIdToString(qint32 nType)
{
    // Convert type ID to string
    return QString::number(nType);  // Replace with actual implementation
}

XBinary::ENDIAN EXAMPLE_CLASS::getEndian()
{
    // Return the endianness
    return XBinary::ENDIAN_UNKNOWN;  // Replace with actual implementation
}

QString EXAMPLE_CLASS::getArch()
{
    // Return the architecture string
    return QString();  // Replace with actual implementation
}

QString EXAMPLE_CLASS::getFileFormatExt()
{
    // Return the file format extension
    return QString();  // Replace with actual implementation
}

QString EXAMPLE_CLASS::getFileFormatExtsString()
{
    // Return the file format extension
    return QString();  // Replace with actual implementation
}

qint64 EXAMPLE_CLASS::getFileFormatSize(XBinary::PDSTRUCT *pPdStruct)
{
    // Return the file format size
    return -1;  // Replace with actual implementation
}

bool EXAMPLE_CLASS::isSigned()
{
    // Check if the file is signed
    return false;  // Replace with actual implementation
}

QString EXAMPLE_CLASS::getOsVersion()
{
    // Return the OS version string
    return QString();  // Replace with actual implementation
}

QString EXAMPLE_CLASS::getVersion()
{
    // Return the version string
    return QString();  // Replace with actual implementation
}

QString EXAMPLE_CLASS::getInfo()
{
    // Return additional information about the file
    return QString();  // Replace with actual implementation
}

bool EXAMPLE_CLASS::isEncrypted()
{
    // Check if the file is encrypted
    return false;  // Replace with actual implementation
}

QString EXAMPLE_CLASS::structIDToString(quint32 nID)
{
    return XBinary::XCONVERT_idToTransString(nID, _TABLE_EXAMPLE_CLASS_STRUCTID, sizeof(_TABLE_EXAMPLE_CLASS_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QString EXAMPLE_CLASS::structIDToFtString(quint32 nID)
{
    return XBinary::XCONVERT_idToFtString(nID, _TABLE_EXAMPLE_CLASS_STRUCTID, sizeof(_TABLE_EXAMPLE_CLASS_STRUCTID) / sizeof(XBinary::XCONVERT));
}

quint32 EXAMPLE_CLASS::ftStringToStructID(const QString &sFtString)
{
    return XCONVERT_ftStringToId(sFtString, _TABLE_EXAMPLE_CLASS_STRUCTID, sizeof(_TABLE_EXAMPLE_CLASS_STRUCTID) / sizeof(XBinary::XCONVERT));
}

EXAMPLE_CLASS::DATA_STRUCT2 EXAMPLE_CLASS::_read_DATA_STRUCT2(qint64 nOffset)
{
    DATA_STRUCT2 dataStruct2 = {};

    dataStruct2.nFieldX = read_int8(nOffset + offsetof(DATA_STRUCT2, nFieldX));
    dataStruct2.nFieldY = read_int32(nOffset + offsetof(DATA_STRUCT2, nFieldY));
    dataStruct2.nFieldZ = read_int64(nOffset + offsetof(DATA_STRUCT2, nFieldZ));

    return dataStruct2;
}

EXAMPLE_CLASS::DATA_STRUCT1 EXAMPLE_CLASS::_read_DATA_STRUCT1(qint64 nOffset)
{
    DATA_STRUCT1 dataStruct1 = {};

    dataStruct1.fieldA = read_int32(nOffset + offsetof(DATA_STRUCT1, fieldA));
    dataStruct1.fieldB = read_int32(nOffset + offsetof(DATA_STRUCT1, fieldB));

    return dataStruct1;
}

EXAMPLE_CLASS::ORIGINALHEADERNAME EXAMPLE_CLASS::_read_ORIGINALHEADERNAME(qint64 nOffset)
{
    ORIGINALHEADERNAME originalHeaderName = {};

    originalHeaderName.field1 = read_uint64(nOffset + offsetof(ORIGINALHEADERNAME, field1));
    originalHeaderName.field2 = read_int32(nOffset + offsetof(ORIGINALHEADERNAME, field2));

    return originalHeaderName;
}

QList<XBinary::FPART> EXAMPLE_CLASS::getHData(PDSTRUCT *pPdStruct)
{
    // Return a list of header data parts
    QList<FPART> listRegions;
    // Populate the list with regions as needed
    // ...

    return listRegions;  // Replace with actual implementation
}

XBinary::_MEMORY_MAP EXAMPLE_CLASS::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    // Return the memory map based on the specified mode
    _MEMORY_MAP result = {};
    result.fileType = getFileType();
    result.nBinarySize = getSize();
    result.mode = getMode();
    result.sArch = getArch();
    result.endian = getEndian();
    result.sType = typeIdToString(getType());

    qint32 nIndex = 0;

    _MEMORY_RECORD recordHeader = {};
    recordHeader.nAddress = -1;
    recordHeader.nOffset = 0;
    recordHeader.nSize = sizeof(ORIGINALHEADERNAME);
    recordHeader.nIndex = nIndex++;
    recordHeader.filePart = FILEPART_HEADER;
    recordHeader.sName = tr("Header");

    result.listRecords.append(recordHeader);
    // Populate other fields as needed
    // ...

    return result;  // Replace with actual implementation
}

XBinary::OSNAME EXAMPLE_CLASS::getOsName()
{
    // Return the OS name
    return XBinary::OSNAME_UNKNOWN;  // Replace with actual implementation
}

bool EXAMPLE_CLASS::handleInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<EXAMPLE_CLASS> guardedThis(this);
    bool bResult = true;

    if (!isInternalInfoHandled()) {
        bResult = guardedThis->XBinary::handleInternalInfo(pPdStruct);
        if (!guardedThis || !bResult) return false;

        XBinary::INTERNAL_INFO *pInfo =
            static_cast<XBinary::INTERNAL_INFO *>(
                guardedThis->XBinary::getInternalInfo(pPdStruct));
        if (!guardedThis || !pInfo) return false;

        static_cast<XBinary::INTERNAL_INFO &>(
            guardedThis->m_internalInfo) = *pInfo;
        guardedThis->setIsInternalInfoHandled(true);
    }

    return guardedThis && bResult;
}

void *EXAMPLE_CLASS::getInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<EXAMPLE_CLASS> guardedThis(this);
    const bool bHandled = guardedThis->handleInternalInfo(pPdStruct);
    if (!guardedThis || !bHandled) return nullptr;

    return &guardedThis->m_internalInfo;
}

void EXAMPLE_CLASS::setInternalInfo(void *pInternalInfo)
{
    if (pInternalInfo) {
        m_internalInfo = *static_cast<INTERNAL_INFO *>(pInternalInfo);
        XBinary::setInternalInfo(static_cast<XBinary::INTERNAL_INFO *>(&m_internalInfo));
        setIsInternalInfoHandled(true);
    } else {
        m_internalInfo = INTERNAL_INFO();
        XBinary::setInternalInfo(nullptr);
        setIsInternalInfoHandled(false);
    }
}
