// copyright (c) 2017-2019 hors<horsicq@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#include "xmsdos.h"

XMSDOS::XMSDOS(QIODevice *__pDevice, bool bIsImage, qint64 nImageBase): XBinary(__pDevice,bIsImage,nImageBase)
{
}

bool XMSDOS::isValid()
{
    bool bResult=false;

    quint16 magic=get_magic();

    if((magic==XMSDOS_DEF::S_IMAGE_DOS_SIGNATURE)||(magic==XMSDOS_DEF::S_IMAGE_DOS_SIGNATURE_ZM))
    {
        bResult=true;
    }

    return bResult;
}

quint16 XMSDOS::get_magic()
{
    return read_uint16((qint64)offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_magic));
}

qint32 XMSDOS::get_lfanew()
{
    return read_int32(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_lfanew));
}

XMSDOS_DEF::IMAGE_DOS_HEADER XMSDOS::getDosHeader()
{
    XMSDOS_DEF::IMAGE_DOS_HEADER result=XMSDOS_DEF::IMAGE_DOS_HEADER();

    read_array((qint64)offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER,e_magic),(char *)&result,sizeof(XMSDOS_DEF::IMAGE_DOS_HEADER));

    return result;
}

XMSDOS_DEF::IMAGE_DOS_HEADEREX XMSDOS::getDosHeaderEx()
{
    XMSDOS_DEF::IMAGE_DOS_HEADEREX result=XMSDOS_DEF::IMAGE_DOS_HEADEREX();

    read_array((qint64)offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_magic),(char *)&result,sizeof(XMSDOS_DEF::IMAGE_DOS_HEADEREX));

    return result;
}

void XMSDOS::setDosHeader(XMSDOS_DEF::IMAGE_DOS_HEADER *pDosHeader)
{
    write_array((qint64)offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER,e_magic),(char *)pDosHeader,sizeof(XMSDOS_DEF::IMAGE_DOS_HEADER));
}

void XMSDOS::setDosHeaderEx(XMSDOS_DEF::IMAGE_DOS_HEADEREX *pDosHeaderEx)
{
    write_array((qint64)offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_magic),(char *)pDosHeaderEx,sizeof(XMSDOS_DEF::IMAGE_DOS_HEADEREX));
}

void XMSDOS::set_e_magic(quint16 value)
{
    write_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_magic),value);
}

void XMSDOS::set_e_cblp(quint16 value)
{
    write_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_cblp),value);
}

void XMSDOS::set_e_cp(quint16 value)
{
    write_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_cp),value);
}

void XMSDOS::set_e_crlc(quint16 value)
{
    write_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_crlc),value);
}

void XMSDOS::set_e_cparhdr(quint16 value)
{
    write_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_cparhdr),value);
}

void XMSDOS::set_e_minalloc(quint16 value)
{
    write_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_minalloc),value);
}

void XMSDOS::set_e_maxalloc(quint16 value)
{
    write_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_maxalloc),value);
}

void XMSDOS::set_e_ss(quint16 value)
{
    write_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_ss),value);
}

void XMSDOS::set_e_sp(quint16 value)
{
    write_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_sp),value);
}

void XMSDOS::set_e_csum(quint16 value)
{
    write_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_csum),value);
}

void XMSDOS::set_e_ip(quint16 value)
{
    write_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_ip),value);
}

void XMSDOS::set_e_cs(quint16 value)
{
    write_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_cs),value);
}

void XMSDOS::set_e_lfarlc(quint16 value)
{
    write_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_lfarlc),value);
}

void XMSDOS::set_e_ovno(quint16 value)
{
    write_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_ovno),value);
}

void XMSDOS::set_e_res(int nPosition, quint16 value)
{
    if(nPosition<4)
    {
        write_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_res)+sizeof(quint16)*nPosition,value);
    }
}

void XMSDOS::set_e_oemid(quint16 value)
{
    write_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_oemid),value);
}

void XMSDOS::set_e_oeminfo(quint16 value)
{
    write_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_oeminfo),value);
}

void XMSDOS::set_e_res2(int nPosition, quint16 value)
{
    if(nPosition<10)
    {
        write_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_res2)+sizeof(quint16)*nPosition,value);
    }
}

void XMSDOS::set_e_lfanew(quint32 value)
{
    write_uint32(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_lfanew),value);
}

quint16 XMSDOS::get_e_magic()
{
    return read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_magic));
}

quint16 XMSDOS::get_e_cblp()
{
    return read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_cblp));
}

quint16 XMSDOS::get_e_cp()
{
    return read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_cp));
}

quint16 XMSDOS::get_e_crlc()
{
    return read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_crlc));
}

quint16 XMSDOS::get_e_cparhdr()
{
    return read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_cparhdr));
}

quint16 XMSDOS::get_e_minalloc()
{
    return read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_minalloc));
}

quint16 XMSDOS::get_e_maxalloc()
{
    return read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_maxalloc));
}

quint16 XMSDOS::get_e_ss()
{
    return read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_ss));
}

quint16 XMSDOS::get_e_sp()
{
    return read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_sp));
}

quint16 XMSDOS::get_e_csum()
{
    return read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_csum));
}

quint16 XMSDOS::get_e_ip()
{
    return read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_ip));
}

quint16 XMSDOS::get_e_cs()
{
    return read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_cs));
}

quint16 XMSDOS::get_e_lfarlc()
{
    return read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_lfarlc));
}

quint16 XMSDOS::get_e_ovno()
{
    return read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_ovno));
}

QList<XBinary::MEMORY_MAP> XMSDOS::getMemoryMapList()
{
    QList<MEMORY_MAP> listResult;

    qint64 nMaxOffset=(get_e_cp()-1)*512+get_e_cblp();

    qint64 nHeaderOffset=0;
    qint64 nHeaderSize=get_e_cparhdr()*16;
    //    qint64 nDataOffset=nHeaderOffset+nHeaderSize;
    //    qint64 nDataSize=get_e_cs()*16;
    qint64 nCodeOffset=nHeaderSize+get_e_cs()*16;
    qint64 nCodeSize=nMaxOffset-nCodeOffset;
    qint64 nOverlayOffset=nMaxOffset;
    qint64 nOverlaySize=qMax(getSize()-nMaxOffset,(qint64)0);

//    qint64 nBaseAddress=_getBaseAddress();

    MEMORY_MAP recordHeader= {};
    recordHeader.nSize=nHeaderSize;
    recordHeader.nOffset=nHeaderOffset;
    recordHeader.nAddress=-1;
    recordHeader.segment=ADDRESS_SEGMENT_UNKNOWN;
    recordHeader.bIsHeader=true;

    listResult.append(recordHeader);

    //    MEMORY_MAP recordData={};
    MEMORY_MAP recordCode= {};

    recordCode.nSize=nCodeSize;
    recordCode.nOffset=nCodeOffset;
    recordCode.nAddress=0;

    recordCode.segment=ADDRESS_SEGMENT_CODE;
    recordCode.bIsHeader=false;

    listResult.append(recordCode);
    // TODO

    if(nOverlaySize)
    {
        MEMORY_MAP recordOverlay= {};
        recordOverlay.nSize=nOverlayOffset;
        recordOverlay.nOffset=nOverlaySize;
        recordOverlay.nAddress=-1;

        recordOverlay.segment=ADDRESS_SEGMENT_UNKNOWN;
        recordOverlay.bIsOvelay=true;

        listResult.append(recordOverlay);
    }

    return listResult;
}

qint64 XMSDOS::getEntryPointOffset()
{
    return addressToOffset(get_e_ip(),ADDRESS_SEGMENT_CODE);
}

QMap<quint64, QString> XMSDOS::getImageMagics()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0x5A4D,"IMAGE_DOS_SIGNATURE");

    return mapResult;
}

QMap<quint64, QString> XMSDOS::getImageMagicsS()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0x5A4D,"DOS_SIGNATURE");

    return mapResult;
}

qint64 XMSDOS::getOverlayOffset()
{
    qint64 nResult=0;

    nResult=(get_e_cp()-1)*512+get_e_cblp();

    return nResult;
}

qint64 XMSDOS::getOverlaySize()
{
    qint64 nSize=getSize();
    qint64 nOverlayOffset=getOverlayOffset();
    qint64 nDelta=nSize-nOverlayOffset;

    return qMax(nDelta,(qint64)0);
}

bool XMSDOS::isOverlayPresent()
{
    return getOverlaySize()!=0;
}
