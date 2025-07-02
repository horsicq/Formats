/* Copyright (c) 2024-2025 hors<horsicq@gmail.com>
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
#include "xjavaclass.h"

XJavaClass::XJavaClass(QIODevice *pDevice) : XBinary(pDevice)
{
}

XJavaClass::~XJavaClass()
{
}

bool XJavaClass::isValid(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    bool bResult = false;

    if (getSize() >= 24) {
        if (read_uint32(0, true) == 0xCAFEBABE) {
            if (read_uint32(4, true) > 10) {
                bResult = true;
            }
        }
    }

    return bResult;
}

bool XJavaClass::isValid(QIODevice *pDevice)
{
    XJavaClass xjavaclass(pDevice);

    return xjavaclass.isValid();
}

QString XJavaClass::getArch()
{
    return "JVM";
}

QString XJavaClass::getVersion()
{
    QString sResult;

    quint16 nMinor = read_uint16(4, true);
    quint16 nMajor = read_uint16(6, true);

    if (nMajor) {
        sResult = _getJDKVersion(nMajor, nMinor);
    }

    return sResult;
}

QString XJavaClass::getFileFormatExt()
{
    return "class";
}

QString XJavaClass::getFileFormatExtsString()
{
    return "Java Class (*.class)";
}

XJavaClass::INFO XJavaClass::getInfo(PDSTRUCT *pPdStruct)
{
    INFO result = {};

    result.nMinorVersion = read_uint16(4, true);
    result.nMajorVersion = read_uint16(6, true);
    result.nConstantPoolCount = read_uint16(8, true);

    qint64 nOffset = 10;

    for (int i = 1; (i < result.nConstantPoolCount) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        quint8 nTag = read_uint8(nOffset);

        cp_info cpInfo = {};
        cpInfo.nOffset = nOffset;
        cpInfo.nTag = nTag;

        // handle tags
        switch (nTag) {
            case CONSTANT_Utf8: {
                quint16 nLength = read_uint16(nOffset + 1, true);
                QByteArray baData = read_array(nOffset + 3, nLength);
                cpInfo.varInfo = QString(baData);
                nOffset += 3 + nLength;
            } break;
            case CONSTANT_Integer: {
                cpInfo.varInfo = read_uint32(nOffset + 1, true);
                nOffset += 5;
            } break;
            case CONSTANT_Float: {
                cpInfo.varInfo = read_float(nOffset + 1, true);
                nOffset += 5;
            } break;
            case CONSTANT_Long: {
                cpInfo.varInfo = read_uint64(nOffset + 1, true);
                nOffset += 9;
            } break;
            case CONSTANT_Double: {
                cpInfo.varInfo = read_double(nOffset + 1, true);
                nOffset += 9;
            } break;
            case CONSTANT_Class: {
                cpInfo.varInfo = read_uint16(nOffset + 1, true);
                nOffset += 3;
            } break;
            case CONSTANT_String: {
                cpInfo.varInfo = read_uint16(nOffset + 1, true);
                nOffset += 3;
            } break;
            case CONSTANT_Fieldref: {
                cpInfo.varInfo = read_uint16(nOffset + 1, true);
                nOffset += 5;
            } break;
            case CONSTANT_Methodref: {
                cpInfo.varInfo = read_uint16(nOffset + 1, true);
                nOffset += 5;
            } break;
            case CONSTANT_InterfaceMethodref: {
                cpInfo.varInfo = read_uint16(nOffset + 1, true);
                nOffset += 5;
            } break;
            case CONSTANT_NameAndType: {
                cpInfo.varInfo = read_uint16(nOffset + 1, true);  // TODO two indexes
                nOffset += 5;
            } break;
            case CONSTANT_MethodHandle: {
                cpInfo.varInfo = read_uint8(nOffset + 1);  // TODO
                nOffset += 4;
            } break;
            case CONSTANT_MethodType: {
                cpInfo.varInfo = read_uint16(nOffset + 1, true);
                nOffset += 3;
            } break;
            case CONSTANT_InvokeDynamic: {
                cpInfo.varInfo = read_uint16(nOffset + 1, true);
                nOffset += 5;
            } break;
            case CONSTANT_Module: {
                cpInfo.varInfo = read_uint16(nOffset + 1, true);
                nOffset += 3;
            } break;
            case CONSTANT_Package: {
                cpInfo.varInfo = read_uint16(nOffset + 1, true);
                nOffset += 3;
            } break;
            default: {
#ifdef QT_DEBUG
                qDebug("Unknown tag: %02X", nTag);
#endif
                break;
            }
        }

        // Print offset, tag, value
        // qDebug("%08X %02X %s", cpInfo.nOffset, cpInfo.nTag, cpInfo.varInfo.toString().toLatin1().data());

        // add to list
        result.listCP.append(cpInfo);

        if (nTag == CONSTANT_Long || nTag == CONSTANT_Double) {
            i++;
        }

        if (nOffset >= getSize()) {
            break;
        }
    }

    result.nAccessFlags = read_uint16(nOffset, true);
    nOffset += 2;
    result.nThisClass = read_uint16(nOffset, true);
    nOffset += 2;
    result.nSuperClass = read_uint16(nOffset, true);
    nOffset += 2;
    result.nInterfacesCount = read_uint16(nOffset, true);
    nOffset += 2;

    for (int i = 0; (i < result.nInterfacesCount) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        result.listInterfaces.append(read_uint16(nOffset, true));
        nOffset += 2;
    }

    result.nFieldsCount = read_uint16(nOffset, true);
    nOffset += 2;

    for (int i = 0; (i < result.nFieldsCount) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        record_info fieldInfo = {};

        nOffset += _read_record_info(nOffset, &fieldInfo);

        result.listFields.append(fieldInfo);
    }

    result.nMethodsCount = read_uint16(nOffset, true);
    nOffset += 2;

    for (int i = 0; (i < result.nMethodsCount) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        record_info methodInfo = {};

        nOffset += _read_record_info(nOffset, &methodInfo);

        result.listMethods.append(methodInfo);
    }

    result.nAttributesCount = read_uint16(nOffset, true);
    nOffset += 2;

    for (int i = 0; (i < result.nAttributesCount) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        attribute_info attributeInfo = {};

        nOffset += _read_attribute_info(nOffset, &attributeInfo);

        result.listAttributes.append(attributeInfo);
    }

    result.nSize = nOffset;

    return result;
}

QString XJavaClass::_getJDKVersion(quint16 nMajor, quint16 nMinor)
{
    QString sResult;

    switch (nMajor) {
        case 0x2D: sResult = "JDK 1.1"; break;
        case 0x2E: sResult = "JDK 1.2"; break;
        case 0x2F: sResult = "JDK 1.3"; break;
        case 0x30: sResult = "JDK 1.4"; break;
        case 0x31: sResult = "Java SE 5.0"; break;
        case 0x32: sResult = "Java SE 6"; break;
        case 0x33: sResult = "Java SE 7"; break;
        case 0x34: sResult = "Java SE 8"; break;
        case 0x35: sResult = "Java SE 9"; break;
        case 0x36: sResult = "Java SE 10"; break;
        case 0x37: sResult = "Java SE 11"; break;
        case 0x38: sResult = "Java SE 12"; break;
        case 0x39: sResult = "Java SE 13"; break;
        case 0x3A: sResult = "Java SE 14"; break;
        case 0x3B: sResult = "Java SE 15"; break;
        case 0x3C: sResult = "Java SE 16"; break;
        case 0x3D: sResult = "Java SE 17"; break;
        case 0x3E: sResult = "Java SE 18"; break;
        case 0x3F: sResult = "Java SE 19"; break;
        case 0x40: sResult = "Java SE 20"; break;
        case 0x41: sResult = "Java SE 21"; break;
        case 0x42: sResult = "Java SE 22"; break;
        case 0x43: sResult = "Java SE 23"; break;
        case 0x44: sResult = "Java SE 24"; break;
        case 0x45: sResult = "Java SE 25"; break;
        case 0x46: sResult = "Java SE 26"; break;
        case 0x47: sResult = "Java SE 27"; break;
        case 0x48: sResult = "Java SE 28"; break;
        case 0x49: sResult = "Java SE 29"; break;
        case 0x4A: sResult = "Java SE 30"; break;
    }

    if ((sResult != "") && (nMinor)) {
        sResult += QString(".%1").arg(nMinor);
    }

    return sResult;
}

qint32 XJavaClass::_read_attribute_info(qint64 nOffset, attribute_info *pAttributeInfo)
{
    qint32 nResult = 0;

    pAttributeInfo->nAttributeNameIndex = read_uint16(nOffset, true);
    pAttributeInfo->nAttributeLength = read_uint32(nOffset + 2, true);
    pAttributeInfo->baInfo = read_array(nOffset + 6, pAttributeInfo->nAttributeLength);

    nResult = 6 + pAttributeInfo->nAttributeLength;

    return nResult;
}

qint32 XJavaClass::_read_record_info(qint64 nOffset, record_info *pRecordInfo)
{
    qint32 nResult = 0;
    qint64 nOriginalOffset = nOffset;

    pRecordInfo->nAccessFlags = read_uint16(nOffset, true);
    pRecordInfo->nNameIndex = read_uint16(nOffset + 2, true);
    pRecordInfo->nDescriptorIndex = read_uint16(nOffset + 4, true);
    pRecordInfo->nAttributesCount = read_uint16(nOffset + 6, true);

    nOffset += 8;

    for (int i = 0; i < pRecordInfo->nAttributesCount; i++) {
        attribute_info attributeInfo = {};

        nOffset += _read_attribute_info(nOffset, &attributeInfo);

        pRecordInfo->listAttributes.append(attributeInfo);
    }

    nResult = nOffset - nOriginalOffset;

    return nResult;
}

XBinary::_MEMORY_MAP XJavaClass::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(mapMode)

    qint64 nTotalSize = getSize();

    _MEMORY_MAP result = {};

    INFO info = getInfo(pPdStruct);

    result.nBinarySize = nTotalSize;
    result.fileType = getFileType();
    result.mode = getMode();
    result.sArch = getArch();
    result.endian = getEndian();
    result.sType = getTypeAsString();

    qint32 nIndex = 0;

    _MEMORY_RECORD record = {};
    record.nAddress = -1;
    record.nOffset = 0;
    record.nSize = info.nSize;
    record.nIndex = 0;
    record.sName = tr("Data");
    record.nIndex = nIndex++;
    record.filePart = FILEPART_DATA;

    result.listRecords.append(record);

    _handleOverlay(&result);

    return result;
}

XBinary::FT XJavaClass::getFileType()
{
    return FT_JAVACLASS;
}

XBinary::ENDIAN XJavaClass::getEndian()
{
    return ENDIAN_BIG;
}

XBinary::MODE XJavaClass::getMode()
{
    return MODE_32;
}
