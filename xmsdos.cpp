// copyright (c) 2017-2020 hors<horsicq@gmail.com>
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

    if( (magic==XMSDOS_DEF::S_IMAGE_DOS_SIGNATURE)||
        (magic==XMSDOS_DEF::S_IMAGE_DOS_SIGNATURE_ZM))
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
    XMSDOS_DEF::IMAGE_DOS_HEADER result={};

    read_array((qint64)offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER,e_magic),(char *)&result,sizeof(XMSDOS_DEF::IMAGE_DOS_HEADER));

    return result;
}

XMSDOS_DEF::IMAGE_DOS_HEADEREX XMSDOS::getDosHeaderEx()
{
    XMSDOS_DEF::IMAGE_DOS_HEADEREX result={};

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

quint16 XMSDOS::get_e_oemid()
{
    return read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_oemid));
}

quint16 XMSDOS::get_e_oeminfo()
{
    return read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_oeminfo));
}

quint32 XMSDOS::get_e_lfanew()
{
    return read_uint32(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX,e_lfanew));
}

XBinary::_MEMORY_MAP XMSDOS::getMemoryMap()
{
    _MEMORY_MAP result={};

    qint32 nIndex=0;

    result.fileType=FT_MSDOS;
    result.mode=MODE_16;
    result.nRawSize=getSize();

    qint64 nMaxOffset=(get_e_cp()-1)*512+get_e_cblp();

    qint64 nHeaderOffset=0;
    qint64 nHeaderSize=(quint16)(get_e_cparhdr()*16);
    //    qint64 nDataOffset=nHeaderOffset+nHeaderSize;
    //    qint64 nDataSize=get_e_cs()*16;
//    qint64 nCodeOffset=(quint16)((quint16)(get_e_cparhdr()*16)+(quint16)(get_e_cs()*16));
    qint64 nCodeOffset=((qint16)get_e_cparhdr()*16)+((qint16)get_e_cs()*16);

    qint64 nCodeAddress=0;

    qint64 nCodeSize=S_ALIGN_UP(nMaxOffset-nCodeOffset,512);
//    nCodeSize=qMin(nCodeSize,getSize());
    qint64 nOverlayOffset=nMaxOffset;
    qint64 nOverlaySize=qMax(getSize()-nMaxOffset,(qint64)0);

//    qint64 nBaseAddress=_getBaseAddress();

    _MEMORY_RECORD recordHeader={};
    recordHeader.nSize=nHeaderSize;
    recordHeader.nOffset=nHeaderOffset;
    recordHeader.nAddress=-1;
    recordHeader.segment=ADDRESS_SEGMENT_UNKNOWN;
    recordHeader.type=MMT_HEADER;
    recordHeader.nIndex=nIndex++;

    result.listRecords.append(recordHeader);

    // TODO ADDRESS_SEGMENT_DATA
    if(nCodeOffset<0) // Virtual
    {
        qint64 nDelta=nCodeOffset-nHeaderSize;

        _MEMORY_RECORD recordVirtualCode={};

        recordVirtualCode.nSize=qAbs(nDelta);
        recordVirtualCode.nOffset=-1;
        recordVirtualCode.nAddress=0;

        recordVirtualCode.segment=ADDRESS_SEGMENT_CODE; // CODE
        recordVirtualCode.type=MMT_LOADSECTION;
        recordVirtualCode.nIndex=nIndex++;

        result.listRecords.append(recordVirtualCode);

        nCodeSize-=qAbs(nDelta);
        nCodeOffset=nHeaderSize;
        nCodeAddress+=qAbs(nDelta);
    }

    _MEMORY_RECORD recordCode={};

    recordCode.nSize=nCodeSize;
    recordCode.nOffset=nCodeOffset;
    recordCode.nAddress=nCodeAddress;

    recordCode.segment=ADDRESS_SEGMENT_CODE; // CODE
    recordCode.type=MMT_LOADSECTION;
    recordCode.nIndex=nIndex++;

    result.listRecords.append(recordCode);

    // TODO

    if(nOverlaySize)
    {
        _MEMORY_RECORD recordOverlay={};
        recordOverlay.nSize=nOverlaySize;
        recordOverlay.nOffset=nOverlayOffset;
        recordOverlay.nAddress=-1;

        recordOverlay.segment=ADDRESS_SEGMENT_UNKNOWN;
        recordOverlay.type=MMT_OVERLAY;
        recordOverlay.nIndex=nIndex++;

        result.listRecords.append(recordOverlay);
    }

    return result;
}

qint64 XMSDOS::getEntryPointOffset(_MEMORY_MAP *pMemoryMap)
{
    return addressToOffset(pMemoryMap,get_e_ip());
}

QMap<quint64, QString> XMSDOS::getImageMagics()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0x5A4D,"IMAGE_DOS_SIGNATURE");
    mapResult.insert(0x4D5A,"IMAGE_DOS_SIGNATURE_ZM");

    return mapResult;
}

QMap<quint64, QString> XMSDOS::getImageMagicsS()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0x5A4D,"DOS_SIGNATURE");
    mapResult.insert(0x4D5A,"DOS_SIGNATURE_ZM");

    return mapResult;
}

bool XMSDOS::isLE()
{
    quint32 nOffset=get_e_lfanew();
    quint16 nNew=read_uint16(nOffset);

    return (nNew==0x454C); // TODO const
}

bool XMSDOS::isLX()
{
    quint32 nOffset=get_e_lfanew();
    quint16 nNew=read_uint16(nOffset);

    return (nNew==0x584C); // TODO const
}

bool XMSDOS::isNE()
{
    quint32 nOffset=get_e_lfanew();
    quint16 nNew=read_uint16(nOffset);

    return (nNew==0x454E); // TODO const
}

bool XMSDOS::isPE()
{
    quint32 nOffset=get_e_lfanew();
    quint16 nNew=read_uint16(nOffset);

    return (nNew==0x4550); // TODO const
}

bool XMSDOS::isRichSignaturePresent()
{
    bool bResult=false;

    int nOffset=sizeof(XMSDOS_DEF::IMAGE_DOS_HEADER);
    int nSize=get_lfanew()-sizeof(XMSDOS_DEF::IMAGE_DOS_HEADER);

    if((nSize>0)&&(nSize<=0x200))
    {
        QByteArray baStub=read_array(nOffset,nSize);

        bResult=baStub.contains("Rich");
    }

    return bResult;
}

QList<XMSDOS::MS_RICH_RECORD> XMSDOS::getRichSignatureRecords()
{
    QList<MS_RICH_RECORD> listResult;

    qint64 nOffset=find_ansiString(getDosStubOffset(),getDosStubSize(),"Rich");

    if(nOffset!=-1)
    {
        quint32 nXORkey=read_uint32(nOffset+4);

        qint64 nCurrentOffset=nOffset-4;

        while(nCurrentOffset>getDosStubOffset()) // TODO optimize
        {
            quint32 nTemp=read_uint32(nCurrentOffset)^nXORkey;

            if(nTemp==0x536e6144) // DanS
            {
                nCurrentOffset+=16;

                for(; nCurrentOffset<nOffset; nCurrentOffset+=8)
                {
                    MS_RICH_RECORD record={};

                    quint32 nValue1=read_uint32(nCurrentOffset)^nXORkey;
                    record.nId=nValue1>>16;
                    record.nVersion=nValue1&0xFFFF;

                    quint32 nValue2=read_uint32(nCurrentOffset+4)^nXORkey;
                    record.nCount=nValue2;

                    listResult.append(record);
                }

                break;
            }

            nCurrentOffset-=4;
        }
    }

    return listResult;
}

qint32 XMSDOS::getNumberOfRichIDs()
{
    QList<MS_RICH_RECORD> listRecords=getRichSignatureRecords();

    return getNumberOfRichIDs(&listRecords);
}

qint32 XMSDOS::getNumberOfRichIDs(QList<XMSDOS::MS_RICH_RECORD> *pListRich)
{
    return pListRich->count();
}

bool XMSDOS::isRichVersionPresent(quint32 nVersion)
{
    QList<MS_RICH_RECORD> listRecords=getRichSignatureRecords();

    return isRichVersionPresent(nVersion,&listRecords);
}

bool XMSDOS::isRichVersionPresent(quint32 nVersion, QList<XMSDOS::MS_RICH_RECORD> *pListRich)
{
    bool bResult=false;

    int nCount=pListRich->count();

    for(int i=0;i<nCount;i++)
    {
        if(pListRich->at(i).nVersion==nVersion)
        {
            bResult=true;

            break;
        }
    }

    return bResult;
}

qint64 XMSDOS::getDosStubSize()
{
    qint64 nSize=(qint64)get_lfanew()-sizeof(XMSDOS_DEF::IMAGE_DOS_HEADEREX);

    nSize=qMax(nSize,(qint64)0);

    return nSize;
}

qint64 XMSDOS::getDosStubOffset()
{
    return sizeof(XMSDOS_DEF::IMAGE_DOS_HEADEREX);
}

QByteArray XMSDOS::getDosStub()
{
    return read_array(getDosStubOffset(),getDosStubSize());
}

bool XMSDOS::isDosStubPresent()
{
    return getDosStubSize()!=0;
}

XBinary::MODE XMSDOS::getMode()
{
    return MODE_16;
}

QString XMSDOS::getArch()
{
    return QString("8086");
}
