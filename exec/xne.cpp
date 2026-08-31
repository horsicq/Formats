/* Copyright (c) 2020-2026 hors<horsicq@gmail.com>
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
#include "xne.h"

namespace {
QString readNePascalString(XNE *pNe, qint64 nOffset)
{
    if (!pNe->checkOffsetSize(nOffset, 1)) {
        return QString();
    }

    const quint8 nLength = pNe->read_uint8(nOffset);
    if ((nLength == 0) || !pNe->checkOffsetSize(nOffset + 1, nLength)) {
        return QString();
    }

    return QString::fromLatin1(pNe->read_array(nOffset + 1, nLength));
}

void readNeNameTable(XNE *pNe, QMap<quint16, QString> *pMapNames, qint64 nOffset, qint64 nSize)
{
    if (!pNe->isOffsetValid(nOffset) || (nSize <= 0)) {
        return;
    }

    const qint64 nEnd = qMin<qint64>(pNe->getSize(), nOffset + nSize);
    qint32 nGuard = 0;

    while ((nOffset < nEnd) && (nGuard++ < 0x10000)) {
        const quint8 nLength = pNe->read_uint8(nOffset++);
        if (nLength == 0) {
            break;
        }
        if ((nOffset > nEnd - nLength) || (nOffset + nLength > nEnd - 2)) {
            break;
        }

        const QString sName = QString::fromLatin1(pNe->read_array(nOffset, nLength));
        nOffset += nLength;
        const quint16 nOrdinal = pNe->read_uint16(nOffset);
        nOffset += 2;

        if ((nOrdinal != 0) && !sName.isEmpty()) {
            pMapNames->insert(nOrdinal, sName);
        }
    }
}

QString readNeResourceName(XNE *pNe, qint64 nTableOffset, quint16 nID)
{
    if (nID & 0x8000) {
        return QString();
    }

    const qint64 nOffset = nTableOffset + nID;
    if (!pNe->checkOffsetSize(nOffset, 1)) {
        return QString();
    }

    const quint8 nLength = pNe->read_uint8(nOffset);
    return pNe->checkOffsetSize(nOffset + 1, nLength) ? QString::fromLatin1(pNe->read_array(nOffset + 1, nLength)) : QString();
}
}  // namespace

XBinary::XCONVERT _TABLE_XNE_STRUCTID[] = {
    {XNE::STRUCTID_UNKNOWN, "Unknown", QObject::tr("Unknown")},
    {XNE::STRUCTID_IMAGE_DOS_HEADER, "IMAGE_DOS_HEADER", QString("IMAGE_DOS_HEADER")},
    {XNE::STRUCTID_IMAGE_DOS_HEADEREX, "IMAGE_DOS_HEADER", QString("IMAGE_DOS_HEADER")},
    {XNE::STRUCTID_IMAGE_OS2_HEADER, "IMAGE_OS2_HEADER", QString("IMAGE_OS2_HEADER")},
    {XNE::STRUCTID_ENTRY_TABLE, "ENTRY_TABLE", QString("Entry table")},
    {XNE::STRUCTID_SEGMENT_TABLE, "SEGMENT_TABLE", QString("Segment table")},
    {XNE::STRUCTID_RESOURCE_TABLE, "RESOURCE_TABLE", QString("Resource table")},
    {XNE::STRUCTID_RESIDENT_NAME_TABLE, "RESIDENT_NAME_TABLE", QString("Resident name table")},
    {XNE::STRUCTID_MODULE_REFERENCE_TABLE, "MODULE_REFERENCE_TABLE", QString("Module reference table")},
    {XNE::STRUCTID_IMPORTED_NAMES_TABLE, "IMPORTED_NAMES_TABLE", QString("Imported names table")},
    {XNE::STRUCTID_NONRESIDENT_NAME_TABLE, "NONRESIDENT_NAME_TABLE", QString("Non-resident name table")},
};

XBinary::XIDSTRING _TABLE_XNE_ImageNEMagics[] = {
    {0x454E, "OS2_SIGNATURE"},
};

XBinary::XIDSTRING _TABLE_XNE_ImageNEFlags[] = {
    {0x0001, "single shared"},
    {0x0002, "multiple"},
    {0x0004, "Global initialization"},
    {0x0008, "Protected mode only"},
    {0x0010, "8086 instructions"},
    {0x0020, "80286 instructions"},
    {0x0040, "80386 instructions"},
    {0x0080, "80x87 instructions"},
    {0x0100, "Full screen"},
    {0x0200, "Compatible with Windows/P.M."},
    {0x0800, "OS/2 family application"},
    {0x1000, "reserved?"},
    {0x2000, "Errors in image/executable"},
    {0x4000, "non-conforming program"},
    {0x8000, "DLL or driver"},
};

XBinary::XIDSTRING _TABLE_XNE_ImageNEExetypes[] = {
    {0x0000, "Unknown"},     {0x0001, "OS/2"},
    {0x0002, "Windows"},     {0x0003, "European MS-DOS 4.x"},
    {0x0004, "Windows 386"}, {0x0005, "BOSS (Borland Operating System Services)"},
};

XBinary::XIDSTRING _TABLE_XNE_ImageNEFlagsothers[] = {
    {0x0001, "Long filename support"},
    {0x0002, "2.x protected mode"},
    {0x0004, "2.x proportional fonts"},
    {0x0008, "Executable has gangload area"},
};

XBinary::XIDSTRING _TABLE_XNE_ImageSegmentTypes[] = {
    {0x0000, "CODE"},
    {0x0001, "DATA"},
};

const QString XNE::PREFIX_ImageNEMagics = "IMAGE_";

XNE::XNE(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress) : XMSDOS(pDevice, bIsImage, nModuleAddress)
{
}

bool XNE::isValid(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    quint16 magic = get_magic();

    if (magic == XMSDOS_DEF::S_IMAGE_DOS_SIGNATURE_MZ) {
        qint64 nImageHeaderOffset = getImageOS2HeaderOffset();

        if ((nImageHeaderOffset >= 0) && _isOffsetValid(nImageHeaderOffset + (qint64)sizeof(quint16) - 1)) {
            quint32 signature = read_uint16(nImageHeaderOffset);

            if (signature == XNE_DEF::S_IMAGE_OS2_SIGNATURE) {
                return true;
            }
        }
    }

    return false;
}

bool XNE::isValid(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress, PDSTRUCT *pPdStruct)
{
    XNE xne(pDevice, bIsImage, nModuleAddress);

    return xne.isValid(pPdStruct);
}

XBinary::MODE XNE::getMode(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    XNE xne(pDevice, bIsImage, nModuleAddress);

    return xne.getMode();
}

qint64 XNE::getImageOS2HeaderOffset()
{
    qint64 nResult = get_lfanew();

    if (!_isOffsetValid(nResult)) {
        nResult = -1;
    }

    return nResult;
}

qint64 XNE::getImageOS2HeaderSize()
{
    return sizeof(XNE_DEF::IMAGE_OS2_HEADER);
}

XNE_DEF::IMAGE_OS2_HEADER XNE::getImageOS2Header()
{
    XNE_DEF::IMAGE_OS2_HEADER result = {};

    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        result.ne_magic = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_magic));
        result.ne_ver = read_uint8(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_ver));
        result.ne_rev = read_uint8(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_rev));
        result.ne_enttab = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_enttab));
        result.ne_cbenttab = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_cbenttab));
        result.ne_crc = read_uint32(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_crc));
        result.ne_flags = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_flags));
        result.ne_autodata = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_autodata));
        result.ne_heap = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_heap));
        result.ne_stack = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_stack));
        result.ne_csip = read_uint32(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_csip));
        result.ne_sssp = read_uint32(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_sssp));
        result.ne_cseg = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_cseg));
        result.ne_cmod = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_cmod));
        result.ne_cbnrestab = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_cbnrestab));
        result.ne_segtab = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_segtab));
        result.ne_rsrctab = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_rsrctab));
        result.ne_restab = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_restab));
        result.ne_modtab = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_modtab));
        result.ne_imptab = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_imptab));
        result.ne_nrestab = read_uint32(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_nrestab));
        result.ne_cmovent = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_cmovent));
        result.ne_align = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_align));
        result.ne_cres = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_cres));
        result.ne_exetyp = read_uint8(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_exetyp));
        result.ne_flagsothers = read_uint8(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_flagsothers));
        result.ne_pretthunks = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_pretthunks));
        result.ne_psegrefbytes = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_psegrefbytes));
        result.ne_swaparea = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_swaparea));
        result.ne_expver = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_expver));
    }

    return result;
}

quint16 XNE::getImageOS2Header_magic()
{
    quint16 nResult = 0;

    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        nResult = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_magic));
    }

    return nResult;
}

quint8 XNE::getImageOS2Header_ver()
{
    quint8 nResult = 0;

    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        nResult = read_uint8(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_ver));
    }

    return nResult;
}

quint8 XNE::getImageOS2Header_rev()
{
    quint8 nResult = 0;

    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        nResult = read_uint8(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_rev));
    }

    return nResult;
}

quint16 XNE::getImageOS2Header_enttab()
{
    quint16 nResult = 0;

    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        nResult = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_enttab));
    }

    return nResult;
}

quint16 XNE::getImageOS2Header_cbenttab()
{
    quint16 nResult = 0;

    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        nResult = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_cbenttab));
    }

    return nResult;
}

quint32 XNE::getImageOS2Header_crc()
{
    quint32 nResult = 0;

    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        nResult = read_uint32(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_crc));
    }

    return nResult;
}

quint16 XNE::getImageOS2Header_flags()
{
    quint16 nResult = 0;

    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        nResult = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_flags));
    }

    return nResult;
}

quint16 XNE::getImageOS2Header_autodata()
{
    quint16 nResult = 0;

    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        nResult = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_autodata));
    }

    return nResult;
}

quint16 XNE::getImageOS2Header_heap()
{
    quint16 nResult = 0;

    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        nResult = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_heap));
    }

    return nResult;
}

quint16 XNE::getImageOS2Header_stack()
{
    quint16 nResult = 0;

    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        nResult = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_stack));
    }

    return nResult;
}

quint32 XNE::getImageOS2Header_csip()
{
    quint32 nResult = 0;

    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        nResult = read_uint32(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_csip));
    }

    return nResult;
}

quint32 XNE::getImageOS2Header_sssp()
{
    quint32 nResult = 0;

    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        nResult = read_uint32(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_sssp));
    }

    return nResult;
}

quint16 XNE::getImageOS2Header_cseg()
{
    quint16 nResult = 0;

    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        nResult = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_cseg));
    }

    return nResult;
}

quint16 XNE::getImageOS2Header_cmod()
{
    quint16 nResult = 0;

    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        nResult = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_cmod));
    }

    return nResult;
}

quint16 XNE::getImageOS2Header_cbnrestab()
{
    quint16 nResult = 0;

    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        nResult = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_cbnrestab));
    }

    return nResult;
}

quint16 XNE::getImageOS2Header_segtab()
{
    quint16 nResult = 0;

    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        nResult = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_segtab));
    }

    return nResult;
}

quint16 XNE::getImageOS2Header_rsrctab()
{
    quint16 nResult = 0;

    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        nResult = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_rsrctab));
    }

    return nResult;
}

quint16 XNE::getImageOS2Header_restab()
{
    quint16 nResult = 0;

    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        nResult = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_restab));
    }

    return nResult;
}

quint16 XNE::getImageOS2Header_modtab()
{
    quint16 nResult = 0;

    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        nResult = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_modtab));
    }

    return nResult;
}

quint16 XNE::getImageOS2Header_imptab()
{
    quint16 nResult = 0;

    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        nResult = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_imptab));
    }

    return nResult;
}

quint32 XNE::getImageOS2Header_nrestab()
{
    quint32 nResult = 0;

    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        nResult = read_uint32(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_nrestab));
    }

    return nResult;
}

quint16 XNE::getImageOS2Header_cmovent()
{
    quint16 nResult = 0;

    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        nResult = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_cmovent));
    }

    return nResult;
}

quint16 XNE::getImageOS2Header_align()
{
    quint16 nResult = 0;

    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        nResult = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_align));
    }

    return nResult;
}

quint16 XNE::getImageOS2Header_cres()
{
    quint16 nResult = 0;

    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        nResult = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_cres));
    }

    return nResult;
}

quint8 XNE::getImageOS2Header_exetyp()
{
    quint8 nResult = 0;

    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        nResult = read_uint8(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_exetyp));
    }

    return nResult;
}

quint8 XNE::getImageOS2Header_flagsothers()
{
    quint8 nResult = 0;

    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        nResult = read_uint8(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_flagsothers));
    }

    return nResult;
}

quint16 XNE::getImageOS2Header_pretthunks()
{
    quint16 nResult = 0;

    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        nResult = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_pretthunks));
    }

    return nResult;
}

quint16 XNE::getImageOS2Header_psegrefbytes()
{
    quint16 nResult = 0;

    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        nResult = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_psegrefbytes));
    }

    return nResult;
}

quint16 XNE::getImageOS2Header_swaparea()
{
    quint16 nResult = 0;

    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        nResult = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_swaparea));
    }

    return nResult;
}

quint16 XNE::getImageOS2Header_expver()
{
    quint16 nResult = 0;

    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        nResult = read_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_expver));
    }

    return nResult;
}

void XNE::setImageOS2Header_magic(quint16 nValue)
{
    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        write_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_magic), nValue);
    }
}

void XNE::setImageOS2Header_ver(quint8 nValue)
{
    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        write_uint8(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_ver), nValue);
    }
}

void XNE::setImageOS2Header_rev(quint8 nValue)
{
    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        write_uint8(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_rev), nValue);
    }
}

void XNE::setImageOS2Header_enttab(quint16 nValue)
{
    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        write_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_enttab), nValue);
    }
}

void XNE::setImageOS2Header_cbenttab(quint16 nValue)
{
    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        write_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_cbenttab), nValue);
    }
}

void XNE::setImageOS2Header_crc(quint32 nValue)
{
    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        write_uint32(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_crc), nValue);
    }
}

void XNE::setImageOS2Header_flags(quint16 nValue)
{
    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        write_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_flags), nValue);
    }
}

void XNE::setImageOS2Header_autodata(quint16 nValue)
{
    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        write_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_autodata), nValue);
    }
}

void XNE::setImageOS2Header_heap(quint16 nValue)
{
    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        write_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_heap), nValue);
    }
}

void XNE::setImageOS2Header_stack(quint16 nValue)
{
    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        write_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_stack), nValue);
    }
}

void XNE::setImageOS2Header_csip(quint32 nValue)
{
    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        write_uint32(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_csip), nValue);
    }
}

void XNE::setImageOS2Header_sssp(quint32 nValue)
{
    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        write_uint32(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_sssp), nValue);
    }
}

void XNE::setImageOS2Header_cseg(quint16 nValue)
{
    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        write_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_cseg), nValue);
    }
}

void XNE::setImageOS2Header_cmod(quint16 nValue)
{
    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        write_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_cmod), nValue);
    }
}

void XNE::setImageOS2Header_cbnrestab(quint16 nValue)
{
    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        write_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_cbnrestab), nValue);
    }
}

void XNE::setImageOS2Header_segtab(quint16 nValue)
{
    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        write_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_segtab), nValue);
    }
}

void XNE::setImageOS2Header_rsrctab(quint16 nValue)
{
    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        write_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_rsrctab), nValue);
    }
}

void XNE::setImageOS2Header_restab(quint16 nValue)
{
    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        write_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_restab), nValue);
    }
}

void XNE::setImageOS2Header_modtab(quint16 nValue)
{
    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        write_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_modtab), nValue);
    }
}

void XNE::setImageOS2Header_imptab(quint16 nValue)
{
    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        write_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_imptab), nValue);
    }
}

void XNE::setImageOS2Header_nrestab(quint32 nValue)
{
    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        write_uint32(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_nrestab), nValue);
    }
}

void XNE::setImageOS2Header_cmovent(quint16 nValue)
{
    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        write_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_cmovent), nValue);
    }
}

void XNE::setImageOS2Header_align(quint16 nValue)
{
    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        write_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_align), nValue);
    }
}

void XNE::setImageOS2Header_cres(quint16 nValue)
{
    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        write_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_cres), nValue);
    }
}

void XNE::setImageOS2Header_exetyp(quint8 nValue)
{
    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        write_uint8(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_exetyp), nValue);
    }
}

void XNE::setImageOS2Header_flagsothers(quint8 nValue)
{
    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        write_uint8(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_flagsothers), nValue);
    }
}

void XNE::setImageOS2Header_pretthunks(quint16 nValue)
{
    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        write_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_pretthunks), nValue);
    }
}

void XNE::setImageOS2Header_psegrefbytes(quint16 nValue)
{
    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        write_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_psegrefbytes), nValue);
    }
}

void XNE::setImageOS2Header_swaparea(quint16 nValue)
{
    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        write_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_swaparea), nValue);
    }
}

void XNE::setImageOS2Header_expver(quint16 nValue)
{
    qint64 nOffset = getImageOS2HeaderOffset();

    if (nOffset != -1) {
        write_uint16(nOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_expver), nValue);
    }
}

qint64 XNE::getEntryTableOffset()
{
    return getImageOS2HeaderOffset() + getImageOS2Header_enttab();
}

qint64 XNE::getEntryTableSize()
{
    return getImageOS2Header_cbenttab();
}

qint64 XNE::getSegmentTableOffset()
{
    return getImageOS2HeaderOffset() + getImageOS2Header_segtab();
}

qint64 XNE::getResourceTableOffset()
{
    return getImageOS2HeaderOffset() + getImageOS2Header_rsrctab();
}

qint64 XNE::getResidentNameTableOffset()
{
    return getImageOS2HeaderOffset() + getImageOS2Header_restab();
}

qint64 XNE::getModuleReferenceTableOffset()
{
    return getImageOS2HeaderOffset() + getImageOS2Header_modtab();
}

qint64 XNE::getImportedNamesTableOffset()
{
    return getImageOS2HeaderOffset() + getImageOS2Header_imptab();
}

qint64 XNE::getNotResindentNameTableOffset()
{
    return getImageOS2Header_nrestab();
}

bool XNE::isImportPresent()
{
    return !getImportStructs().isEmpty();
}

bool XNE::isExportPresent()
{
    return !getExportStructs().isEmpty();
}

bool XNE::isResourcesPresent()
{
    return !getResourceStructs().isEmpty();
}

QVector<XBinary::XIMPORT_STRUCT> XNE::getImportStructs()
{
    QVector<XIMPORT_STRUCT> listResult;
    const quint16 nModuleCount = qMin<quint16>(getImageOS2Header_cmod(), 0x4000);
    const qint64 nModuleTableOffset = getModuleReferenceTableOffset();
    const qint64 nNamesOffset = getImportedNamesTableOffset();

    if ((nModuleCount == 0) || !checkOffsetSize(nModuleTableOffset, (qint64)nModuleCount * 2) || !isOffsetValid(nNamesOffset)) {
        return listResult;
    }

    QStringList listModules;
    QVector<bool> listModuleUsed(nModuleCount, false);
    listModules.reserve(nModuleCount);

    for (quint16 i = 0; i < nModuleCount; ++i) {
        const quint16 nNameOffset = read_uint16(nModuleTableOffset + (qint64)i * 2);
        listModules.append(readNePascalString(this, nNamesOffset + nNameOffset));
    }

    const QList<XNE_DEF::NE_SEGMENT> listSegments = getSegmentList();
    const quint16 nShift = getImageOS2Header_align();

    if (nShift <= 47) {
        for (qint32 i = 0; i < listSegments.count(); ++i) {
            const XNE_DEF::NE_SEGMENT &segment = listSegments.at(i);

            // NSRELOC: the relocation count and eight-byte records follow the segment data.
            if ((segment.dwFlags & 0x0100) == 0) {
                continue;
            }

            const qint64 nSegmentOffset = ((qint64)segment.dwFileOffset) << nShift;
            const qint64 nSegmentSize = segment.dwFileSize ? segment.dwFileSize : 0x10000;
            const qint64 nRelocationsOffset = nSegmentOffset + nSegmentSize;

            if (!checkOffsetSize(nRelocationsOffset, 2)) {
                continue;
            }

            const quint16 nRelocationCount = qMin<quint16>(read_uint16(nRelocationsOffset), 0x4000);
            if (!checkOffsetSize(nRelocationsOffset + 2, (qint64)nRelocationCount * 8)) {
                continue;
            }

            for (quint16 j = 0; j < nRelocationCount; ++j) {
                const qint64 nRecordOffset = nRelocationsOffset + 2 + (qint64)j * 8;
                const quint8 nFlags = read_uint8(nRecordOffset + 1);
                const quint8 nTargetType = nFlags & 0x03;

                if ((nTargetType != 1) && (nTargetType != 2)) {
                    continue;
                }

                const quint16 nSourceOffset = read_uint16(nRecordOffset + 2);
                const quint16 nModuleIndex = read_uint16(nRecordOffset + 4);
                const quint16 nTarget = read_uint16(nRecordOffset + 6);

                if ((nModuleIndex == 0) || (nModuleIndex > nModuleCount)) {
                    continue;
                }

                XIMPORT_STRUCT record = {};
                record.nOffset = nRecordOffset;
                record.nSize = 8;
                record.nAddress = getModuleAddress() + (XADDR)(i + 1) * 0x10000 + nSourceOffset;
                record.sLibrary = listModules.at(nModuleIndex - 1);
                record.nOrdinal = -1;

                if (nTargetType == 1) {
                    record.nOrdinal = nTarget;
                } else {
                    record.sFunction = readNePascalString(this, nNamesOffset + nTarget);
                }

                listModuleUsed[nModuleIndex - 1] = true;
                listResult.append(record);
            }
        }
    }

    // Some valid NE files omit per-segment fixups. Preserve their imported-module
    // information instead of reporting an empty import list.
    for (qint32 i = 0; i < nModuleCount; ++i) {
        if (!listModuleUsed.at(i)) {
            XIMPORT_STRUCT record = {};
            record.nOffset = nModuleTableOffset + (qint64)i * 2;
            record.nSize = 2;
            record.nAddress = (XADDR)-1;
            record.sLibrary = listModules.at(i);
            record.nOrdinal = -1;
            listResult.append(record);
        }
    }

    return listResult;
}

QVector<XBinary::XEXPORT_STRUCT> XNE::getExportStructs()
{
    QVector<XEXPORT_STRUCT> listResult;
    QMap<quint16, QString> mapNames;

    const qint64 nResidentOffset = getResidentNameTableOffset();
    const qint64 nModuleOffset = getModuleReferenceTableOffset();
    readNeNameTable(this, &mapNames, nResidentOffset, nModuleOffset - nResidentOffset);
    readNeNameTable(this, &mapNames, getNotResindentNameTableOffset(), getImageOS2Header_cbnrestab());

    const qint64 nEntryOffset = getEntryTableOffset();
    const qint64 nEntrySize = getEntryTableSize();
    if (!checkOffsetSize(nEntryOffset, nEntrySize) || (nEntrySize <= 0)) {
        return listResult;
    }

    const QList<XNE_DEF::NE_SEGMENT> listSegments = getSegmentList();
    const quint16 nShift = getImageOS2Header_align();
    const qint64 nEnd = nEntryOffset + nEntrySize;
    qint64 nCurrentOffset = nEntryOffset;
    quint16 nOrdinal = 1;
    qint32 nGuard = 0;

    while ((nCurrentOffset + 2 <= nEnd) && (nGuard++ < 0x10000)) {
        const quint8 nCount = read_uint8(nCurrentOffset++);
        const quint8 nBundleType = read_uint8(nCurrentOffset++);
        if (nCount == 0) {
            break;
        }

        if (nBundleType == 0) {
            nOrdinal = (quint16)(nOrdinal + nCount);
            continue;
        }

        const qint32 nRecordSize = (nBundleType == 0xFF) ? 6 : 3;
        if (nCurrentOffset > nEnd - (qint64)nCount * nRecordSize) {
            break;
        }

        for (quint8 i = 0; i < nCount; ++i, ++nOrdinal) {
            quint8 nSegment = nBundleType;
            quint16 nFunctionOffset = 0;

            if (nBundleType == 0xFF) {
                nSegment = read_uint8(nCurrentOffset + 3);
                nFunctionOffset = read_uint16(nCurrentOffset + 4);
            } else {
                nFunctionOffset = read_uint16(nCurrentOffset + 1);
            }

            XEXPORT_STRUCT record = {};
            record.nOffset = -1;
            record.nSize = 0;
            record.nAddress = getModuleAddress() + (XADDR)nSegment * 0x10000 + nFunctionOffset;
            record.sFunction = mapNames.value(nOrdinal, QString("Ordinal_%1").arg(nOrdinal));
            record.nOrdinal = nOrdinal;

            if ((nShift <= 47) && (nSegment > 0) && (nSegment <= listSegments.count())) {
                const XNE_DEF::NE_SEGMENT &segment = listSegments.at(nSegment - 1);
                record.nOffset = (((qint64)segment.dwFileOffset) << nShift) + nFunctionOffset;
            }

            listResult.append(record);
            nCurrentOffset += nRecordSize;
        }
    }

    return listResult;
}

QVector<XBinary::XRESOURCE_STRUCT> XNE::getResourceStructs()
{
    QVector<XRESOURCE_STRUCT> listResult;
    const qint64 nTableOffset = getResourceTableOffset();

    if (!checkOffsetSize(nTableOffset, 2)) {
        return listResult;
    }

    const quint16 nShift = read_uint16(nTableOffset);
    if (nShift > 47) {
        return listResult;
    }

    qint64 nCurrentOffset = nTableOffset + 2;
    qint32 nGuard = 0;

    while (checkOffsetSize(nCurrentOffset, 8) && (nGuard++ < 0x4000)) {
        const quint16 nTypeID = read_uint16(nCurrentOffset);
        if (nTypeID == 0) {
            break;
        }

        const quint16 nCount = qMin<quint16>(read_uint16(nCurrentOffset + 2), 0x4000);
        const QString sTypeName = readNeResourceName(this, nTableOffset, nTypeID);
        nCurrentOffset += 8;

        if (!checkOffsetSize(nCurrentOffset, (qint64)nCount * 12)) {
            break;
        }

        for (quint16 i = 0; i < nCount; ++i) {
            const qint64 nNameInfoOffset = nCurrentOffset + (qint64)i * 12;
            const quint16 nNameID = read_uint16(nNameInfoOffset + 6);
            const qint64 nResourceOffset = ((qint64)read_uint16(nNameInfoOffset)) << nShift;
            const qint64 nResourceSize = ((qint64)read_uint16(nNameInfoOffset + 2)) << nShift;

            if (!checkOffsetSize(nResourceOffset, nResourceSize)) {
                continue;
            }

            XRESOURCE_STRUCT record = {};
            record.nOffset = nResourceOffset;
            record.nSize = nResourceSize;
            record.nAddress = offsetToAddress(nResourceOffset);
            record.nType = (nTypeID & 0x8000) ? (nTypeID & 0x7FFF) : 0;
            record.nID = (nNameID & 0x8000) ? (nNameID & 0x7FFF) : 0;
            record.sName = readNeResourceName(this, nTableOffset, nNameID);

            if (record.sName.isEmpty()) {
                record.sName = sTypeName;
            }
            if (record.sName.isEmpty()) {
                record.sName = QString("Resource_%1").arg(record.nID ? record.nID : (quint32)(listResult.count() + 1));
            }

            listResult.append(record);
        }

        nCurrentOffset += (qint64)nCount * 12;
    }

    return listResult;
}

QList<XNE_DEF::NE_SEGMENT> XNE::getSegmentList()
{
    QList<XNE_DEF::NE_SEGMENT> listResult;

    qint64 nOffset = getSegmentTableOffset();
    if (!_isOffsetValid(nOffset)) {
        return listResult;
    }

    qint64 nNumberOfSegmentsMax = (getSize() - nOffset) / sizeof(XNE_DEF::NE_SEGMENT);
    qint64 nNumberOfSegments = getImageOS2Header_cseg();

    nNumberOfSegments = qMin<qint64>(nNumberOfSegments, nNumberOfSegmentsMax);

    for (qint64 i = 0; i < nNumberOfSegments; i++) {
        XNE_DEF::NE_SEGMENT segment = _read_NE_SEGMENT(nOffset);

        listResult.append(segment);

        nOffset += sizeof(XNE_DEF::NE_SEGMENT);
    }

    return listResult;
}

XNE_DEF::NE_SEGMENT XNE::_read_NE_SEGMENT(qint64 nOffset)
{
    XNE_DEF::NE_SEGMENT result = {};

    result.dwFileOffset = read_uint16(nOffset + offsetof(XNE_DEF::NE_SEGMENT, dwFileOffset));
    result.dwFileSize = read_uint16(nOffset + offsetof(XNE_DEF::NE_SEGMENT, dwFileSize));
    result.dwFlags = read_uint16(nOffset + offsetof(XNE_DEF::NE_SEGMENT, dwFlags));
    result.dwMinAllocSize = read_uint16(nOffset + offsetof(XNE_DEF::NE_SEGMENT, dwMinAllocSize));

    return result;
}

XBinary::_MEMORY_MAP XNE::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    XBinary::_MEMORY_MAP result = {};

    if (mapMode == MAPMODE_UNKNOWN) {
        mapMode = MAPMODE_SEGMENTS;
    }

    if (mapMode == MAPMODE_SEGMENTS) {
        result = _getMemoryMap(FILEPART_HEADER | FILEPART_SEGMENT | FILEPART_OVERLAY, pPdStruct);
    }

    return result;
}

QMap<quint64, QString> XNE::getImageNEMagics()
{
    return XBinary::XIDSTRING_createMapPrefix(_TABLE_XNE_ImageNEMagics, sizeof(_TABLE_XNE_ImageNEMagics) / sizeof(XBinary::XIDSTRING), PREFIX_ImageNEMagics);
}

QMap<quint64, QString> XNE::getImageNEMagicsS()
{
    return XBinary::XIDSTRING_createMap(_TABLE_XNE_ImageNEMagics, sizeof(_TABLE_XNE_ImageNEMagics) / sizeof(XBinary::XIDSTRING));
}

QMap<quint64, QString> XNE::getImageNEFlagsS()
{
    return XBinary::XIDSTRING_createMap(_TABLE_XNE_ImageNEFlags, sizeof(_TABLE_XNE_ImageNEFlags) / sizeof(XBinary::XIDSTRING));
}

QMap<quint64, QString> XNE::getImageNEExetypesS()
{
    return XBinary::XIDSTRING_createMap(_TABLE_XNE_ImageNEExetypes, sizeof(_TABLE_XNE_ImageNEExetypes) / sizeof(XBinary::XIDSTRING));
}

QMap<quint64, QString> XNE::getImageNEFlagsothersS()
{
    return XBinary::XIDSTRING_createMap(_TABLE_XNE_ImageNEFlagsothers, sizeof(_TABLE_XNE_ImageNEFlagsothers) / sizeof(XBinary::XIDSTRING));
}

QMap<quint64, QString> XNE::getImageSegmentTypesS()
{
    return XBinary::XIDSTRING_createMap(_TABLE_XNE_ImageSegmentTypes, sizeof(_TABLE_XNE_ImageSegmentTypes) / sizeof(XBinary::XIDSTRING));
}

qint64 XNE::getModuleAddress()
{
    return 0x10000;
}

XBinary::MODE XNE::getMode()
{
    if (isValid()) {
        return MODE_16SEG;
    }

    return MODE_UNKNOWN;
}

QString XNE::getArch()
{
    QString sResult = "286";

    quint16 nOS = getImageOS2Header_exetyp();

    if (nOS == 4) {
        sResult = "386";
    } else if (nOS == 5) {
        sResult = "386";
    } else if (nOS == 0x81) {
        sResult = "286";
    } else if (nOS == 0x82) {
        sResult = "286";
    }

    return sResult;
}

XBinary::ENDIAN XNE::getEndian()
{
    return ENDIAN_LITTLE;
}

XBinary::FT XNE::getFileType()
{
    return FT_NE;
}

qint32 XNE::getType()
{
    TYPE result = TYPE_EXE;

    quint16 nFlags = getImageOS2Header_flags();

    if (nFlags & 0x8000) {
        // Heuristic: scan non-resident name table for the word 'driver'
        // If found, treat as DRIVER; if 'font' present, treat as FONT; otherwise classify as DLL
        bool bIsDriver = false;
        bool bIsFont = false;
        qint64 nNRTableOffset = getNotResindentNameTableOffset();
        quint16 nNRTableSize = getImageOS2Header_cbnrestab();

        if ((nNRTableOffset > 0) && (nNRTableSize > 0) && checkOffsetSize({nNRTableOffset, nNRTableSize})) {
            qint64 nPos = nNRTableOffset;
            qint64 nEnd = nNRTableOffset + nNRTableSize;
            while (nPos < nEnd) {
                quint8 nLen = read_uint8(nPos);
                nPos += 1;
                if (nLen == 0) {
                    break;
                }
                QString sName = read_ansiString(nPos, qMin<qint64>(nLen, 255));
                if (sName.contains("driver", Qt::CaseInsensitive)) {
                    bIsDriver = true;
                    break;
                }
                if (sName.contains("font", Qt::CaseInsensitive)) {
                    bIsFont = true;
                    // don't break yet; prefer 'driver' if both appear, but mark font
                }
                nPos += nLen + 2;
            }
        }

        if (bIsDriver) {
            result = TYPE_DRIVER;
        } else if (bIsFont) {
            result = TYPE_FONT;
        } else {
            result = TYPE_DLL;
        }
    }

    return result;
}

qint64 XNE::getImageSize()
{
    qint32 nNumberOfSegments = qMax<qint32>(1, getImageOS2Header_cseg());
    return static_cast<qint64>(nNumberOfSegments) * 0x10000;
}

QString XNE::getOsVersion()
{
    quint16 nOS = getImageOS2Header_exetyp();

    if (nOS == 3) return "4.x";
    if (nOS == 0x81 || nOS == 0x82) return "PharLap Dos Extender";

    return {};
}

XBinary::OSNAME XNE::getOsName()
{
    OSNAME result = OSNAME_UNKNOWN;

    quint16 nOS = getImageOS2Header_exetyp();

    if (nOS == 1) {
        result = OSNAME_OS2;
    } else if (nOS == 2) {
        result = OSNAME_WINDOWS;
    } else if (nOS == 3) {
        result = OSNAME_MSDOS;
    } else if (nOS == 4) {
        result = OSNAME_WINDOWS;
    } else if (nOS == 5) {
        result = OSNAME_BORLANDOSSERVICES;
    } else if (nOS == 0x81) {
        result = OSNAME_OS2;
    } else if (nOS == 0x82) {
        result = OSNAME_WINDOWS;
    }

    return result;
}

QString XNE::typeIdToString(qint32 nType)
{
    switch (nType) {
        case TYPE_UNKNOWN: return tr("Unknown");
        case TYPE_EXE: return "EXE";
        case TYPE_DLL: return "DLL";
        case TYPE_DRIVER: return "Driver";
        case TYPE_FONT: return "Font";
    }

    return tr("Unknown");
}

QString XNE::getFileFormatExtsString()
{
    return "ne";
}

QList<XBinary::MAPMODE> XNE::getMapModesList()
{
    return {MAPMODE_SEGMENTS};
}

XADDR XNE::_getEntryPointAddress()
{
    XADDR nModule = getModuleAddress();
    quint32 csip = getImageOS2Header_csip();
    quint16 ip = static_cast<quint16>(csip & 0xFFFF);
    quint16 cs = static_cast<quint16>((csip >> 16) & 0xFFFF);
    return nModule + static_cast<XADDR>(cs) * 0x10000 + ip;
}

QString XNE::structIDToString(quint32 nID)
{
    return XBinary::XCONVERT_idToTransString(nID, _TABLE_XNE_STRUCTID, sizeof(_TABLE_XNE_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QString XNE::structIDToFtString(quint32 nID)
{
    return XBinary::XCONVERT_idToFtString(nID, _TABLE_XNE_STRUCTID, sizeof(_TABLE_XNE_STRUCTID) / sizeof(XBinary::XCONVERT));
}

quint32 XNE::ftStringToStructID(const QString &sFtString)
{
    return XCONVERT_ftStringToId(sFtString, _TABLE_XNE_STRUCTID, sizeof(_TABLE_XNE_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QList<XBinary::XFHEADER> XNE::getXFHeaders(const XFSTRUCT &xfStruct, PDSTRUCT *pPdStruct)
{
    QList<XBinary::XFHEADER> listResult;

    quint32 nStructID = xfStruct.nStructID;

    if (nStructID == STRUCTID_UNKNOWN) {
        // DOS stub header via the base class
        listResult.append(XMSDOS::getXFHeaders(xfStruct, pPdStruct));

        if (xfStruct.bIsParent) {
            XFSTRUCT _xfStruct = xfStruct;

            if (!listResult.isEmpty()) {
                _xfStruct.sParent = listResult.first().sTag;
            }

            _xfStruct.nStructID = STRUCTID_IMAGE_OS2_HEADER;
            _xfStruct.xLoc = offsetToLoc(getImageOS2HeaderOffset());
            listResult.append(getXFHeaders(_xfStruct, pPdStruct));
        }
    } else if ((nStructID == STRUCTID_IMAGE_DOS_HEADER) || (nStructID == STRUCTID_IMAGE_DOS_HEADEREX)) {
        listResult.append(XMSDOS::getXFHeaders(xfStruct, pPdStruct));
    } else if (nStructID == STRUCTID_IMAGE_OS2_HEADER) {
        XLOC headerLoc = xfStruct.xLoc;
        if (headerLoc.locType == LT_UNKNOWN) {
            headerLoc = offsetToLoc(getImageOS2HeaderOffset());
        }

        qint64 nHeaderOffset = locToOffset(xfStruct.pMemoryMap, headerLoc);

        if (nHeaderOffset != -1) {
            XFHEADER xfHeader = {};
            xfHeader.sParentTag = xfStruct.sParent;
            xfHeader.fileType = xfStruct.fileType;
            xfHeader.structID = static_cast<XBinary::STRUCTID>(STRUCTID_IMAGE_OS2_HEADER);
            xfHeader.xLoc = headerLoc;
            xfHeader.nSize = sizeof(XNE_DEF::IMAGE_OS2_HEADER);
            xfHeader.xfType = XFTYPE_HEADER;
            xfHeader.listFields = getXFRecords(xfStruct.fileType, STRUCTID_IMAGE_OS2_HEADER, headerLoc);
            xfHeader.sTag = xfHeaderToTag(xfHeader, structIDToString(STRUCTID_IMAGE_OS2_HEADER), xfHeader.sParentTag);
            listResult.append(xfHeader);

            if (xfStruct.bIsParent) {
                quint16 nSegTab = read_uint16(nHeaderOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_segtab));
                quint16 nCSeg = read_uint16(nHeaderOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_cseg));

                XFSTRUCT _xfStruct = xfStruct;
                _xfStruct.sParent = xfHeader.sTag;
                _xfStruct.nStructID = STRUCTID_SEGMENT_TABLE;
                _xfStruct.xLoc = offsetToLoc(nHeaderOffset + nSegTab);
                _xfStruct.nCount = nCSeg;
                listResult.append(getXFHeaders(_xfStruct, pPdStruct));
            }
        }
    } else if (nStructID == STRUCTID_SEGMENT_TABLE) {
        qint64 nOffset = locToOffset(xfStruct.pMemoryMap, xfStruct.xLoc);
        qint32 nCount = xfStruct.nCount;
        qint64 nFileSize = getSize();

        if ((nOffset == -1) || (nCount == 0)) {
            qint64 nHeaderOffset = getImageOS2HeaderOffset();
            nOffset = nHeaderOffset + read_uint16(nHeaderOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_segtab));
            nCount = read_uint16(nHeaderOffset + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_cseg));
        }

        if ((nOffset > 0) && (nCount > 0)) {
            XFHEADER xfHeader = {};
            xfHeader.sParentTag = xfStruct.sParent;
            xfHeader.fileType = xfStruct.fileType;
            xfHeader.structID = static_cast<XBinary::STRUCTID>(STRUCTID_SEGMENT_TABLE);
            xfHeader.xLoc = offsetToLoc(nOffset);
            xfHeader.xfType = XFTYPE_TABLE;
            xfHeader.listFields = getXFRecords(xfStruct.fileType, STRUCTID_SEGMENT_TABLE, xfHeader.xLoc);

            qint64 nCurrentOffset = nOffset;
            for (qint32 i = 0; i < nCount; i++) {
                if ((nCurrentOffset + (qint64)sizeof(XNE_DEF::NE_SEGMENT)) > nFileSize) {
                    break;
                }
                xfHeader.listRowLocations.append(nCurrentOffset);
                nCurrentOffset += sizeof(XNE_DEF::NE_SEGMENT);
            }

            xfHeader.sTag = xfHeaderToTag(xfHeader, structIDToString(STRUCTID_SEGMENT_TABLE), xfHeader.sParentTag);
            listResult.append(xfHeader);
        }
    }

    return listResult;
}

QList<XBinary::XFRECORD> XNE::getXFRecords(FT fileType, quint32 nStructID, const XLOC &xLoc)
{
    QList<XBinary::XFRECORD> listResult;

    if ((nStructID == STRUCTID_IMAGE_DOS_HEADER) || (nStructID == STRUCTID_IMAGE_DOS_HEADEREX)) {
        listResult.append(XMSDOS::getXFRecords(fileType, nStructID, xLoc));
    } else if (nStructID == STRUCTID_IMAGE_OS2_HEADER) {
        listResult.append({"ne_magic", (qint32)offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_magic), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"ne_ver", (qint32)offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_ver), 1, XFRECORD_FLAG_VERSION_MAJOR, VT_UINT8});
        listResult.append({"ne_rev", (qint32)offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_rev), 1, XFRECORD_FLAG_VERSION_MINOR, VT_UINT8});
        listResult.append({"ne_enttab", (qint32)offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_enttab), 2, XFRECORD_FLAG_RELATIVE_OFFSET, VT_UINT16});
        listResult.append({"ne_cbenttab", (qint32)offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_cbenttab), 2, XFRECORD_FLAG_SIZE, VT_UINT16});
        listResult.append({"ne_crc", (qint32)offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_crc), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"ne_flags", (qint32)offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_flags), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"ne_autodata", (qint32)offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_autodata), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"ne_heap", (qint32)offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_heap), 2, XFRECORD_FLAG_SIZE, VT_UINT16});
        listResult.append({"ne_stack", (qint32)offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_stack), 2, XFRECORD_FLAG_SIZE, VT_UINT16});
        listResult.append({"ne_csip", (qint32)offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_csip), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"ne_sssp", (qint32)offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_sssp), 4, XFRECORD_FLAG_NONE, VT_UINT32});
        listResult.append({"ne_cseg", (qint32)offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_cseg), 2, XFRECORD_FLAG_COUNT, VT_UINT16});
        listResult.append({"ne_cmod", (qint32)offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_cmod), 2, XFRECORD_FLAG_COUNT, VT_UINT16});
        listResult.append({"ne_cbnrestab", (qint32)offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_cbnrestab), 2, XFRECORD_FLAG_SIZE, VT_UINT16});
        listResult.append({"ne_segtab", (qint32)offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_segtab), 2, XFRECORD_FLAG_RELATIVE_OFFSET, VT_UINT16});
        listResult.append({"ne_rsrctab", (qint32)offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_rsrctab), 2, XFRECORD_FLAG_RELATIVE_OFFSET, VT_UINT16});
        listResult.append({"ne_restab", (qint32)offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_restab), 2, XFRECORD_FLAG_RELATIVE_OFFSET, VT_UINT16});
        listResult.append({"ne_modtab", (qint32)offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_modtab), 2, XFRECORD_FLAG_RELATIVE_OFFSET, VT_UINT16});
        listResult.append({"ne_imptab", (qint32)offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_imptab), 2, XFRECORD_FLAG_RELATIVE_OFFSET, VT_UINT16});
        listResult.append({"ne_nrestab", (qint32)offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_nrestab), 4, XFRECORD_FLAG_OFFSET, VT_UINT32});
        listResult.append({"ne_cmovent", (qint32)offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_cmovent), 2, XFRECORD_FLAG_COUNT, VT_UINT16});
        listResult.append({"ne_align", (qint32)offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_align), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"ne_cres", (qint32)offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_cres), 2, XFRECORD_FLAG_COUNT, VT_UINT16});
        listResult.append({"ne_exetyp", (qint32)offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_exetyp), 1, XFRECORD_FLAG_NONE, VT_UINT8});
        listResult.append({"ne_flagsothers", (qint32)offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_flagsothers), 1, XFRECORD_FLAG_NONE, VT_UINT8});
        listResult.append({"ne_pretthunks", (qint32)offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_pretthunks), 2, XFRECORD_FLAG_RELATIVE_OFFSET, VT_UINT16});
        listResult.append({"ne_psegrefbytes", (qint32)offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_psegrefbytes), 2, XFRECORD_FLAG_RELATIVE_OFFSET, VT_UINT16});
        listResult.append({"ne_swaparea", (qint32)offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_swaparea), 2, XFRECORD_FLAG_SIZE, VT_UINT16});
        listResult.append({"ne_expver", (qint32)offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_expver), 2, XFRECORD_FLAG_VERSION_DIVMOD, VT_UINT16});
    } else if (nStructID == STRUCTID_SEGMENT_TABLE) {
        // dwFileOffset is in logical sectors (units of 1 << ne_align), not bytes
        listResult.append({"dwFileOffset", (qint32)offsetof(XNE_DEF::NE_SEGMENT, dwFileOffset), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"dwFileSize", (qint32)offsetof(XNE_DEF::NE_SEGMENT, dwFileSize), 2, XFRECORD_FLAG_SIZE, VT_UINT16});
        listResult.append({"dwFlags", (qint32)offsetof(XNE_DEF::NE_SEGMENT, dwFlags), 2, XFRECORD_FLAG_NONE, VT_UINT16});
        listResult.append({"dwMinAllocSize", (qint32)offsetof(XNE_DEF::NE_SEGMENT, dwMinAllocSize), 2, XFRECORD_FLAG_SIZE, VT_UINT16});
    }

    return listResult;
}

// QList<XBinary::DATA_HEADER> XNE::getDataHeaders(const DATA_HEADERS_OPTIONS &dataHeadersOptions, PDSTRUCT *pPdStruct)
// {
//     QList<DATA_HEADER> listResult;

//     if (dataHeadersOptions.nID == STRUCTID_UNKNOWN) {
//         DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
//         _dataHeadersOptions.bChildren = true;
//         _dataHeadersOptions.dsID_parent = _addDefaultHeaders(&listResult, pPdStruct);
//         _dataHeadersOptions.dhMode = XBinary::DHMODE_HEADER;
//         _dataHeadersOptions.fileType = dataHeadersOptions.pMemoryMap->fileType;

//         {
//             DATA_HEADERS_OPTIONS dosOpts = _dataHeadersOptions;
//             dosOpts.nID = XMSDOS::STRUCTID_IMAGE_DOS_HEADEREX;
//             dosOpts.locType = LT_OFFSET;
//             dosOpts.nLocation = 0;
//             QList<DATA_HEADER> listDos = XMSDOS::getDataHeaders(dosOpts, pPdStruct);
//             listResult.append(listDos);
//         }

//         {
//             DATA_HEADERS_OPTIONS neOpts = _dataHeadersOptions;
//             neOpts.nID = STRUCTID_IMAGE_OS2_HEADER;
//             neOpts.locType = LT_OFFSET;
//             neOpts.nLocation = getImageOS2HeaderOffset();
//             listResult.append(getDataHeaders(neOpts, pPdStruct));
//         }
//     } else {
//         qint64 nStartOffset = locationToOffset(dataHeadersOptions.pMemoryMap, dataHeadersOptions.locType, dataHeadersOptions.nLocation);

//         if (nStartOffset != -1) {
//             if (dataHeadersOptions.nID == STRUCTID_IMAGE_OS2_HEADER) {
//                 DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, XNE::structIDToString(dataHeadersOptions.nID));
//                 dataHeader.nSize = sizeof(XNE_DEF::IMAGE_OS2_HEADER);

//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_magic), 2, "ne_magic", VT_WORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_ver), 1, "ne_ver", VT_UINT8, DRF_VERSION, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_rev), 1, "ne_rev", VT_UINT8, DRF_VERSION, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_enttab), 2, "ne_enttab", VT_WORD, DRF_OFFSET, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_cbenttab), 2, "ne_cbenttab", VT_WORD, DRF_SIZE, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_crc), 4, "ne_crc", VT_DWORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_flags), 2, "ne_flags", VT_WORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_autodata), 2, "ne_autodata", VT_WORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_heap), 2, "ne_heap", VT_WORD, DRF_SIZE, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_stack), 2, "ne_stack", VT_WORD, DRF_SIZE, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_csip), 4, "ne_csip", VT_DWORD, DRF_ADDRESS, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_sssp), 4, "ne_sssp", VT_DWORD, DRF_ADDRESS, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_cseg), 2, "ne_cseg", VT_WORD, DRF_COUNT, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_cmod), 2, "ne_cmod", VT_WORD, DRF_COUNT, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_cbnrestab), 2, "ne_cbnrestab", VT_WORD, DRF_SIZE, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_segtab), 2, "ne_segtab", VT_WORD, DRF_OFFSET, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_rsrctab), 2, "ne_rsrctab", VT_WORD, DRF_OFFSET, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_restab), 2, "ne_restab", VT_WORD, DRF_OFFSET, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_modtab), 2, "ne_modtab", VT_WORD, DRF_OFFSET, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_imptab), 2, "ne_imptab", VT_WORD, DRF_OFFSET, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_nrestab), 4, "ne_nrestab", VT_DWORD, DRF_OFFSET, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_cmovent), 2, "ne_cmovent", VT_WORD, DRF_COUNT, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_align), 2, "ne_align", VT_WORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_cres), 2, "ne_cres", VT_WORD, DRF_COUNT, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_exetyp), 1, "ne_exetyp", VT_UINT8, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_flagsothers), 1, "ne_flagsothers", VT_UINT8, DRF_UNKNOWN,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_pretthunks), 2, "ne_pretthunks", VT_WORD, DRF_OFFSET, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(getDataRecord(offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_psegrefbytes), 2, "ne_psegrefbytes", VT_WORD, DRF_OFFSET,
//                                                             dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_swaparea), 2, "ne_swaparea", VT_WORD, DRF_SIZE, dataHeadersOptions.pMemoryMap->endian));
//                 dataHeader.listRecords.append(
//                     getDataRecord(offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_expver), 2, "ne_expver", VT_WORD, DRF_VERSION, dataHeadersOptions.pMemoryMap->endian));

//                 listResult.append(dataHeader);

//                 if (dataHeadersOptions.bChildren) {
//                     const qint64 nBase = getImageOS2HeaderOffset();
//                     const quint16 offEntry = read_uint16(nBase + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_enttab));
//                     const quint16 cbEntry = read_uint16(nBase + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_cbenttab));
//                     const quint16 offSeg = read_uint16(nBase + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_segtab));
//                     const quint16 offRes = read_uint16(nBase + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_rsrctab));
//                     const quint16 offResNames = read_uint16(nBase + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_restab));
//                     const quint16 offModRef = read_uint16(nBase + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_modtab));
//                     const quint16 offImpNames = read_uint16(nBase + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_imptab));
//                     const quint32 offNonRes = read_uint32(nBase + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_nrestab));
//                     const quint16 cSeg = read_uint16(nBase + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_cseg));

//                     if (cbEntry && _isOffsetValid(nBase + offEntry)) {
//                         listResult.append(_dataHeaderHex(dataHeadersOptions, XNE::structIDToString(STRUCTID_ENTRY_TABLE), dataHeader.dsID, STRUCTID_ENTRY_TABLE,
//                                                          nBase + offEntry, cbEntry));
//                     }

//                     if (cSeg && _isOffsetValid(nBase + offSeg)) {
//                         DATA_HEADERS_OPTIONS segOpts = dataHeadersOptions;
//                         segOpts.dhMode = XBinary::DHMODE_TABLE;
//                         segOpts.nID = STRUCTID_SEGMENT_TABLE;
//                         segOpts.locType = LT_OFFSET;
//                         segOpts.nLocation = nBase + offSeg;
//                         segOpts.nCount = cSeg;
//                         segOpts.nSize = static_cast<qint64>(cSeg) * static_cast<qint64>(sizeof(XNE_DEF::NE_SEGMENT));

//                         DATA_HEADER segHeader = _initDataHeader(segOpts, XNE::structIDToString(STRUCTID_SEGMENT_TABLE));
//                         segHeader.dsID = segOpts.dsID_parent;  // keep chaining
//                         segHeader.dsID_parent = dataHeader.dsID;
//                         segHeader.nSize = segOpts.nSize;
//                         segHeader.nCount = cSeg;
//                         segHeader.dhMode = XBinary::DHMODE_TABLE;
//                         segHeader.locType = LT_OFFSET;
//                         segHeader.nLocation = segOpts.nLocation;
//                         segHeader.listRecords.append(
//                             getDataRecord(offsetof(XNE_DEF::NE_SEGMENT, dwFileOffset), 2, "FileOffset", VT_WORD, DRF_OFFSET, dataHeadersOptions.pMemoryMap->endian));
//                         segHeader.listRecords.append(
//                             getDataRecord(offsetof(XNE_DEF::NE_SEGMENT, dwFileSize), 2, "FileSize", VT_WORD, DRF_SIZE, dataHeadersOptions.pMemoryMap->endian));
//                         segHeader.listRecords.append(
//                             getDataRecord(offsetof(XNE_DEF::NE_SEGMENT, dwFlags), 2, "Flags", VT_WORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
//                         segHeader.listRecords.append(
//                             getDataRecord(offsetof(XNE_DEF::NE_SEGMENT, dwMinAllocSize), 2, "MinAllocSize", VT_WORD, DRF_SIZE, dataHeadersOptions.pMemoryMap->endian));

//                         listResult.append(segHeader);
//                     }

//                     if (_isOffsetValid(nBase + offRes) && (offResNames > offRes)) {
//                         qint64 nResSize = (qint64)offResNames - (qint64)offRes;
//                         if (nResSize > 0) {
//                             listResult.append(_dataHeaderHex(dataHeadersOptions, XNE::structIDToString(STRUCTID_RESOURCE_TABLE), dataHeader.dsID,
//                             STRUCTID_RESOURCE_TABLE,
//                                                              nBase + offRes, nResSize));
//                         }
//                     }

//                     if (_isOffsetValid(nBase + offResNames) && (offModRef > offResNames)) {
//                         qint64 nSize = (qint64)offModRef - (qint64)offResNames;
//                         if (nSize > 0) {
//                             listResult.append(_dataHeaderHex(dataHeadersOptions, XNE::structIDToString(STRUCTID_RESIDENT_NAME_TABLE), dataHeader.dsID,
//                                                              STRUCTID_RESIDENT_NAME_TABLE, nBase + offResNames, nSize));
//                         }
//                     }

//                     if (_isOffsetValid(nBase + offModRef) && (offImpNames > offModRef)) {
//                         qint64 nSize = (qint64)offImpNames - (qint64)offModRef;
//                         if (nSize > 0) {
//                             listResult.append(_dataHeaderHex(dataHeadersOptions, XNE::structIDToString(STRUCTID_MODULE_REFERENCE_TABLE), dataHeader.dsID,
//                                                              STRUCTID_MODULE_REFERENCE_TABLE, nBase + offModRef, nSize));
//                         }
//                     }

//                     if (_isOffsetValid(nBase + offImpNames)) {
//                         qint64 nEnd = offNonRes ? (qint64)offNonRes : (qint64)getSize();
//                         if (nEnd > (nBase + offImpNames)) {
//                             qint64 nSize = nEnd - (nBase + offImpNames);
//                             if (nSize > 0) {
//                                 listResult.append(_dataHeaderHex(dataHeadersOptions, XNE::structIDToString(STRUCTID_IMPORTED_NAMES_TABLE), dataHeader.dsID,
//                                                                  STRUCTID_IMPORTED_NAMES_TABLE, nBase + offImpNames, nSize));
//                             }
//                         }
//                     }

//                     const quint16 cbNonRes = read_uint16(nBase + offsetof(XNE_DEF::IMAGE_OS2_HEADER, ne_cbnrestab));
//                     if (offNonRes && cbNonRes && _isOffsetValid(offNonRes)) {
//                         listResult.append(_dataHeaderHex(dataHeadersOptions, XNE::structIDToString(STRUCTID_NONRESIDENT_NAME_TABLE), dataHeader.dsID,
//                                                          STRUCTID_NONRESIDENT_NAME_TABLE, offNonRes, cbNonRes));
//                     }
//                 }
//             } else if ((dataHeadersOptions.nID == STRUCTID_IMAGE_DOS_HEADER) || (dataHeadersOptions.nID == STRUCTID_IMAGE_DOS_HEADEREX)) {
//                 listResult.append(XMSDOS::getDataHeaders(dataHeadersOptions, pPdStruct));
//             }
//         }
//     }

//     return listResult;
// }

QList<XBinary::FPART> XNE::getFileParts(quint32 nFileParts, qint32 nLimit, PDSTRUCT *pPdStruct)
{
    QList<XBinary::FPART> listResult;
    QList<XBinary::FPART> _listCalc;

    if ((nLimit < -1) || (nLimit == 0)) {
        return listResult;
    }

    bool bCalcAddress = false;
    if (nFileParts & FILEPART_SEGMENT) {
        bCalcAddress = true;
    }

    XADDR nModuleAddress = getModuleAddress();
    qint64 nTotalSize = getSize();
    qint64 nMaxOffset = 0;

    if (bCalcAddress || (nFileParts & FILEPART_HEADER) || (nFileParts & FILEPART_OVERLAY)) {
        FPART record = {};
        record.filePart = FILEPART_HEADER;
        record.nVirtualAddress = nModuleAddress;
        record.nVirtualSize = 0x200;
        record.nFileOffset = 0;
        qint64 nImageHeaderOffset = getImageOS2HeaderOffset();
        qint64 nImageHeaderSize = 0;

        if (nImageHeaderOffset >= 0) {
            nImageHeaderSize = qMin<qint64>(nImageHeaderOffset + sizeof(XNE_DEF::IMAGE_OS2_HEADER), nTotalSize);
        }

        record.nFileSize = nImageHeaderSize;
        record.sName = tr("Header");

        if (bCalcAddress) {
            _listCalc.append(record);
        }
        if ((nFileParts & FILEPART_HEADER) && (nImageHeaderSize > 0)) {
            listResult.append(record);
            if ((nLimit != -1) && (listResult.count() >= nLimit)) return listResult;
        }
        nMaxOffset = qMax(nMaxOffset, record.nFileOffset + record.nFileSize);
    }

    if (bCalcAddress || (nFileParts & FILEPART_SEGMENT) || (nFileParts & FILEPART_OVERLAY)) {
        QList<XNE_DEF::NE_SEGMENT> listSegments = getSegmentList();
        quint16 nShift = getImageOS2Header_align();

        for (qint32 i = 0; i < listSegments.count(); i++) {
            const XNE_DEF::NE_SEGMENT &seg = listSegments.at(i);

            qint64 nFileSize = seg.dwFileSize ? seg.dwFileSize : 0x10000;
            if (nShift > 62) {
                continue;
            }

            qint64 nSegmentBase = getSize() - 1;
            qint64 nFileOffset = static_cast<qint64>(seg.dwFileOffset);
            if ((nSegmentBase > 0) && (nFileOffset > (nSegmentBase >> nShift))) {
                continue;
            }
            nFileOffset <<= nShift;

            if (nFileOffset > nTotalSize) {
                continue;
            }
            if (nFileOffset > (nTotalSize - nFileSize)) {
                nFileSize = nTotalSize - nFileOffset;
            }

            FPART record = {};
            record.filePart = FILEPART_SEGMENT;
            record.nFileOffset = nFileOffset;
            record.nFileSize = nFileSize;
            record.nVirtualAddress = nModuleAddress + static_cast<XADDR>((i + 1) * 0x10000);
            record.nVirtualSize = 0x10000;
            record.sName = QString("%1 %2").arg(tr("Segment")).arg(QString::number(i + 1));

            if (bCalcAddress) {
                _listCalc.append(record);
            }
            if (nFileParts & FILEPART_SEGMENT) {
                listResult.append(record);
                if ((nLimit != -1) && (listResult.count() >= nLimit)) return listResult;
            }

            nMaxOffset = qMax(nMaxOffset, record.nFileOffset + record.nFileSize);
        }
    }

    if (nFileParts & FILEPART_OVERLAY) {
        if (nMaxOffset < nTotalSize) {
            FPART record = {};
            record.filePart = FILEPART_OVERLAY;
            record.nFileOffset = nMaxOffset;
            record.nFileSize = nTotalSize - nMaxOffset;
            record.nVirtualAddress = (XADDR)-1;
            record.sName = tr("Overlay");
            listResult.append(record);
            if ((nLimit != -1) && (listResult.count() >= nLimit)) return listResult;
        }
    }

    Q_UNUSED(pPdStruct)
    return listResult;
}

QList<QString> XNE::getSearchSignatures()
{
    return {"'MZ'"};
}

XBinary *XNE::createInstance(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    Q_UNUSED(bIsImage)
    Q_UNUSED(nModuleAddress)

    return new XNE(pDevice);
}

bool XNE::handleInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XNE> guardedThis(this);
    bool bResult = true;

    if (!isInternalInfoHandled()) {
        bResult = guardedThis->XMSDOS::handleInternalInfo(pPdStruct);
        if (!guardedThis || !bResult) return false;

        XMSDOS::INTERNAL_INFO *pInfo = static_cast<XMSDOS::INTERNAL_INFO *>(guardedThis->XMSDOS::getInternalInfo(pPdStruct));
        if (!guardedThis || !pInfo) return false;

        static_cast<XMSDOS::INTERNAL_INFO &>(guardedThis->m_internalInfo) = *pInfo;
        guardedThis->setIsInternalInfoHandled(true);
    }

    return guardedThis && bResult;
}

void *XNE::getInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XNE> guardedThis(this);
    const bool bHandled = guardedThis->handleInternalInfo(pPdStruct);
    if (!guardedThis || !bHandled) return nullptr;

    return &guardedThis->m_internalInfo;
}

void XNE::setInternalInfo(void *pInternalInfo)
{
    if (pInternalInfo) {
        m_internalInfo = *static_cast<INTERNAL_INFO *>(pInternalInfo);
        XMSDOS::setInternalInfo(static_cast<XMSDOS::INTERNAL_INFO *>(&m_internalInfo));
        setIsInternalInfoHandled(true);
    } else {
        m_internalInfo = INTERNAL_INFO();
        XMSDOS::setInternalInfo(nullptr);
        setIsInternalInfoHandled(false);
    }
}
