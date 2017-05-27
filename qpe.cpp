// Copyright (c) 2017 hors<horsicq@gmail.com>
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
#include "qpe.h"

QPE::QPE(QIODevice *pData,bool bIsImage): QMSDOS(pData,bIsImage)
{
}

bool QPE::isValid()
{
    quint16 magic=get_magic();

    if((magic==S_IMAGE_DOS_SIGNATURE))
    {
        qint32 lfanew=get_lfanew();

        if(lfanew>0)
        {
            quint32 signature=read_uint32(lfanew);

            if(signature==S_IMAGE_NT_SIGNATURE)
            {
                return true;
            }
        }
    }

    return false;
}

bool QPE::is64()
{
    return (getFileHeader_Machine()==S_IMAGE_FILE_MACHINE_AMD64)||(getFileHeader_Machine()==S_IMAGE_FILE_MACHINE_IA64);
}

bool QPE::isRichSignaturePresent()
{
    bool bResult=false;
    int nOffset=sizeof(S_IMAGE_DOS_HEADER);
    int nSize=get_lfanew()-sizeof(S_IMAGE_DOS_HEADER);

    if((nSize>0x200)||(nSize<=0))
    {
        return false;
    }

    QByteArray baStub=read_array(nOffset,nSize);

    bResult=baStub.contains("Rich");


    return bResult;
}

QList<QPE::RICH_RECORD> QPE::getRichSignatureRecords()
{
    QList<RICH_RECORD> listResult;

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
                    RICH_RECORD record;

                    quint32 nValue1=read_uint32(nCurrentOffset)^nXORkey;
                    record.nId=nValue1>>16;
                    record.nVersion=nValue1&0xFFFF;

                    //                    quint32 n1=nValue1>>24;
                    //                    quint32 n2=(nValue1>>16)&0xFF;


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

qint64 QPE::getDosStubSize()
{
    qint64 nSize=(qint64)get_lfanew()-sizeof(S_IMAGE_DOS_HEADEREX);

    nSize=qMax(nSize,(qint64)0);

    return nSize;
}

qint64 QPE::getDosStubOffset()
{
    return sizeof(S_IMAGE_DOS_HEADEREX);
}

QByteArray QPE::getDosStub()
{
    return read_array(getDosStubOffset(),getDosStubSize());
}

bool QPE::isDosStubPresent()
{
    return getDosStubSize()!=0;
}

qint64 QPE::getNtHeadersOffset()
{
    qint64 result=get_lfanew();

    if(!_isOffsetValid(result))
    {
        result=-1;
    }

    return result;
}

quint32 QPE::getNtHeaders_Signature()
{
    qint64 nOffset=getNtHeadersOffset();
    return read_uint32(nOffset);
}

void QPE::setNtHeaders_Signature(quint32 value)
{
    write_uint32(getNtHeadersOffset(),value);
}

qint64 QPE::getFileHeaderOffset()
{
    qint64 result=get_lfanew()+4;

    if(!_isOffsetValid(result))
    {
        result=-1;
    }

    return result;
}

S_IMAGE_FILE_HEADER QPE::getFileHeader()
{
    S_IMAGE_FILE_HEADER result= {0};

    read_array(getFileHeaderOffset(),(char *)&result,sizeof(S_IMAGE_FILE_HEADER));

    return result;
}

void QPE::setFileHeader(S_IMAGE_FILE_HEADER *pFileHeader)
{
    write_array(getFileHeaderOffset(),(char *)pFileHeader,sizeof(S_IMAGE_FILE_HEADER));
}

quint16 QPE::getFileHeader_Machine()
{
    return read_uint16(getFileHeaderOffset()+offsetof(S_IMAGE_FILE_HEADER,Machine));
}

quint16 QPE::getFileHeader_NumberOfSections()
{
    return read_uint16(getFileHeaderOffset()+offsetof(S_IMAGE_FILE_HEADER,NumberOfSections));
}

quint32 QPE::getFileHeader_TimeDateStamp()
{
    return read_uint32(getFileHeaderOffset()+offsetof(S_IMAGE_FILE_HEADER,TimeDateStamp));
}

quint32 QPE::getFileHeader_PointerToSymbolTable()
{
    return read_uint32(getFileHeaderOffset()+offsetof(S_IMAGE_FILE_HEADER,PointerToSymbolTable));
}

quint32 QPE::getFileHeader_NumberOfSymbols()
{
    return read_uint32(getFileHeaderOffset()+offsetof(S_IMAGE_FILE_HEADER,NumberOfSymbols));
}

quint16 QPE::getFileHeader_SizeOfOptionalHeader()
{
    return read_uint16(getFileHeaderOffset()+offsetof(S_IMAGE_FILE_HEADER,SizeOfOptionalHeader));
}

quint16 QPE::getFileHeader_Characteristics()
{
    return read_uint16(getFileHeaderOffset()+offsetof(S_IMAGE_FILE_HEADER,Characteristics));
}

void QPE::setFileHeader_Machine(quint16 value)
{
    write_uint16(getFileHeaderOffset()+offsetof(S_IMAGE_FILE_HEADER,Machine),value);
}

void QPE::setFileHeader_NumberOfSections(quint16 value)
{
    write_uint16(getFileHeaderOffset()+offsetof(S_IMAGE_FILE_HEADER,NumberOfSections),value);
}

void QPE::setFileHeader_TimeDateStamp(quint32 value)
{
    write_uint32(getFileHeaderOffset()+offsetof(S_IMAGE_FILE_HEADER,TimeDateStamp),value);
}

void QPE::setFileHeader_PointerToSymbolTable(quint32 value)
{
    write_uint32(getFileHeaderOffset()+offsetof(S_IMAGE_FILE_HEADER,PointerToSymbolTable),value);
}

void QPE::setFileHeader_NumberOfSymbols(quint32 value)
{
    write_uint32(getFileHeaderOffset()+offsetof(S_IMAGE_FILE_HEADER,NumberOfSymbols),value);
}

void QPE::setFileHeader_SizeOfOptionalHeader(quint16 value)
{
    write_uint16(getFileHeaderOffset()+offsetof(S_IMAGE_FILE_HEADER,SizeOfOptionalHeader),value);
}

void QPE::setFileHeader_Characteristics(quint16 value)
{
    write_uint16(getFileHeaderOffset()+offsetof(S_IMAGE_FILE_HEADER,Characteristics),value);
}

qint64 QPE::getOptionalHeaderOffset()
{
    qint64 result=get_lfanew()+4+sizeof(S_IMAGE_FILE_HEADER);

    if(!_isOffsetValid(result))
    {
        result=-1;
    }

    return result;
}

S_IMAGE_OPTIONAL_HEADER32 QPE::getOptionalHeader32()
{
    S_IMAGE_OPTIONAL_HEADER32 result= {0};

    read_array(getOptionalHeaderOffset(),(char *)&result,sizeof(S_IMAGE_OPTIONAL_HEADER32));

    return result;
}

S_IMAGE_OPTIONAL_HEADER64 QPE::getOptionalHeader64()
{
    S_IMAGE_OPTIONAL_HEADER64 result= {0};

    read_array(getOptionalHeaderOffset(),(char *)&result,sizeof(S_IMAGE_OPTIONAL_HEADER64));

    return result;
}

void QPE::setOptionalHeader32(S_IMAGE_OPTIONAL_HEADER32 *pOptionalHeader32)
{
    write_array(getOptionalHeaderOffset(),(char *)pOptionalHeader32,sizeof(S_IMAGE_OPTIONAL_HEADER32));
}

void QPE::setOptionalHeader64(S_IMAGE_OPTIONAL_HEADER64 *pOptionalHeader64)
{
    write_array(getOptionalHeaderOffset(),(char *)pOptionalHeader64,sizeof(S_IMAGE_OPTIONAL_HEADER64));
}

S_IMAGE_OPTIONAL_HEADER32S QPE::getOptionalHeader32S()
{
    S_IMAGE_OPTIONAL_HEADER32S result= {0};

    read_array(getOptionalHeaderOffset(),(char *)&result,sizeof(S_IMAGE_OPTIONAL_HEADER32S));

    return result;
}

S_IMAGE_OPTIONAL_HEADER64S QPE::getOptionalHeader64S()
{
    S_IMAGE_OPTIONAL_HEADER64S result= {0};

    read_array(getOptionalHeaderOffset(),(char *)&result,sizeof(S_IMAGE_OPTIONAL_HEADER64S));

    return result;
}

void QPE::setOptionalHeader32S(S_IMAGE_OPTIONAL_HEADER32S *pOptionalHeader32S)
{
    // TODO check -1
    write_array(getOptionalHeaderOffset(),(char *)pOptionalHeader32S,sizeof(S_IMAGE_OPTIONAL_HEADER32S));
}

void QPE::setOptionalHeader64S(S_IMAGE_OPTIONAL_HEADER64S *pOptionalHeader64S)
{
    // TODO check -1
    write_array(getOptionalHeaderOffset(),(char *)pOptionalHeader64S,sizeof(S_IMAGE_OPTIONAL_HEADER64S));
}

quint16 QPE::getOptionalHeader_Magic()
{
    return read_uint16(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,Magic));
}

quint8 QPE::getOptionalHeader_MajorLinkerVersion()
{
    return read_uint8(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,MajorLinkerVersion));
}

quint8 QPE::getOptionalHeader_MinorLinkerVersion()
{
    return read_uint8(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,MinorLinkerVersion));
}

quint32 QPE::getOptionalHeader_SizeOfCode()
{
    return read_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,SizeOfCode));
}

quint32 QPE::getOptionalHeader_SizeOfInitializedData()
{
    return read_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,SizeOfInitializedData));
}

quint32 QPE::getOptionalHeader_SizeOfUninitializedData()
{
    return read_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,SizeOfUninitializedData));
}

quint32 QPE::getOptionalHeader_AddressOfEntryPoint()
{
    return read_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,AddressOfEntryPoint));
}

quint32 QPE::getOptionalHeader_BaseOfCode()
{
    return read_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,BaseOfCode));
}

quint32 QPE::getOptionalHeader_BaseOfData()
{
    return read_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,BaseOfData));
}

qint64 QPE::getOptionalHeader_ImageBase()
{
    if(is64())
    {
        return read_uint64(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER64,ImageBase));
    }
    else
    {
        return read_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,ImageBase));
    }

}

quint32 QPE::getOptionalHeader_SectionAlignment()
{
    return read_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,SectionAlignment));
}

quint32 QPE::getOptionalHeader_FileAlignment()
{
    return read_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,FileAlignment));
}

quint16 QPE::getOptionalHeader_MajorOperatingSystemVersion()
{
    return read_uint16(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,MajorOperatingSystemVersion));
}

quint16 QPE::getOptionalHeader_MinorOperatingSystemVersion()
{
    return read_uint16(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,MinorOperatingSystemVersion));
}

quint16 QPE::getOptionalHeader_MajorImageVersion()
{
    return read_uint16(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,MajorImageVersion));
}

quint16 QPE::getOptionalHeader_MinorImageVersion()
{
    return read_uint16(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,MinorImageVersion));
}

quint16 QPE::getOptionalHeader_MajorSubsystemVersion()
{
    return read_uint16(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,MajorSubsystemVersion));
}

quint16 QPE::getOptionalHeader_MinorSubsystemVersion()
{
    return read_uint16(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,MinorSubsystemVersion));
}

quint32 QPE::getOptionalHeader_Win32VersionValue()
{
    return read_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,Win32VersionValue));
}

quint32 QPE::getOptionalHeader_SizeOfImage()
{
    return read_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,SizeOfImage));
}

quint32 QPE::getOptionalHeader_SizeOfHeaders()
{
    return read_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,SizeOfHeaders));
}

quint32 QPE::getOptionalHeader_CheckSum()
{
    return read_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,CheckSum));
}

quint16 QPE::getOptionalHeader_Subsystem()
{
    return read_uint16(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,Subsystem));
}

quint16 QPE::getOptionalHeader_DllCharacteristics()
{
    return read_uint16(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,DllCharacteristics));
}

qint64 QPE::getOptionalHeader_SizeOfStackReserve()
{
    if(is64())
    {
        return read_uint64(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER64,SizeOfStackReserve));
    }
    else
    {
        return read_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,SizeOfStackReserve));
    }
}

qint64 QPE::getOptionalHeader_SizeOfStackCommit()
{
    if(is64())
    {
        return read_uint64(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER64,SizeOfStackCommit));
    }
    else
    {
        return read_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,SizeOfStackCommit));
    }
}

qint64 QPE::getOptionalHeader_SizeOfHeapReserve()
{
    if(is64())
    {
        return read_uint64(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER64,SizeOfHeapReserve));
    }
    else
    {
        return read_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,SizeOfHeapReserve));
    }
}

qint64 QPE::getOptionalHeader_SizeOfHeapCommit()
{
    if(is64())
    {
        return read_uint64(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER64,SizeOfHeapCommit));
    }
    else
    {
        return read_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,SizeOfHeapCommit));
    }
}

quint32 QPE::getOptionalHeader_LoaderFlags()
{
    if(is64())
    {
        return read_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER64,LoaderFlags));
    }
    else
    {
        return read_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,LoaderFlags));
    }
}

quint32 QPE::getOptionalHeader_NumberOfRvaAndSizes()
{
    if(is64())
    {
        return read_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER64,NumberOfRvaAndSizes));
    }
    else
    {
        return read_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,NumberOfRvaAndSizes));
    }
}

void QPE::setOptionalHeader_Magic(quint16 value)
{
    write_uint16(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,Magic),value);
}

void QPE::setOptionalHeader_MajorLinkerVersion(quint8 value)
{
    write_uint8(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,MajorLinkerVersion),value);
}

void QPE::setOptionalHeader_MinorLinkerVersion(quint8 value)
{
    write_uint8(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,MinorLinkerVersion),value);
}

void QPE::setOptionalHeader_SizeOfCode(quint32 value)
{
    write_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,SizeOfCode),value);
}

void QPE::setOptionalHeader_SizeOfInitializedData(quint32 value)
{
    write_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,SizeOfInitializedData),value);
}

void QPE::setOptionalHeader_SizeOfUninitializedData(quint32 value)
{
    write_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,SizeOfUninitializedData),value);
}

void QPE::setOptionalHeader_AddressOfEntryPoint(quint32 value)
{
    write_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,AddressOfEntryPoint),value);
}

void QPE::setOptionalHeader_BaseOfCode(quint32 value)
{
    write_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,BaseOfCode),value);
}

void QPE::setOptionalHeader_BaseOfData(quint32 value)
{
    // There is no BaseOfData for PE64
    write_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,BaseOfData),value);
}

void QPE::setOptionalHeader_ImageBase(qint64 value)
{
    if(is64())
    {
        write_uint64(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER64,ImageBase),value);
    }
    else
    {
        write_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,ImageBase),value);
    }
}

void QPE::setOptionalHeader_SectionAlignment(quint32 value)
{
    write_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,SectionAlignment),value);
}

void QPE::setOptionalHeader_FileAlignment(quint32 value)
{
    write_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,FileAlignment),value);
}

void QPE::setOptionalHeader_MajorOperatingSystemVersion(quint16 value)
{
    write_uint16(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,MajorOperatingSystemVersion),value);
}

void QPE::setOptionalHeader_MinorOperatingSystemVersion(quint16 value)
{
    write_uint16(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,MinorOperatingSystemVersion),value);
}

void QPE::setOptionalHeader_MajorImageVersion(quint16 value)
{
    write_uint16(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,MajorImageVersion),value);
}

void QPE::setOptionalHeader_MinorImageVersion(quint16 value)
{
    write_uint16(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,MinorImageVersion),value);
}

void QPE::setOptionalHeader_MajorSubsystemVersion(quint16 value)
{
    write_uint16(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,MajorSubsystemVersion),value);
}

void QPE::setOptionalHeader_MinorSubsystemVersion(quint16 value)
{
    write_uint16(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,MinorSubsystemVersion),value);
}

void QPE::setOptionalHeader_Win32VersionValue(quint32 value)
{
    write_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,Win32VersionValue),value);
}

void QPE::setOptionalHeader_SizeOfImage(quint32 value)
{
    write_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,SizeOfImage),value);
}

void QPE::setOptionalHeader_SizeOfHeaders(quint32 value)
{
    write_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,SizeOfHeaders),value);
}

void QPE::setOptionalHeader_CheckSum(quint32 value)
{
    write_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,CheckSum),value);
}

void QPE::setOptionalHeader_Subsystem(quint16 value)
{
    write_uint16(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,Subsystem),value);
}

void QPE::setOptionalHeader_DllCharacteristics(quint16 value)
{
    write_uint16(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,DllCharacteristics),value);
}

void QPE::setOptionalHeader_SizeOfStackReserve(qint64 value)
{
    if(is64())
    {
        write_uint64(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER64,SizeOfStackReserve),value);
    }
    else
    {
        write_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,SizeOfStackReserve),value);
    }

}

void QPE::setOptionalHeader_SizeOfStackCommit(qint64 value)
{
    if(is64())
    {
        write_uint64(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER64,SizeOfStackCommit),value);
    }
    else
    {
        write_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,SizeOfStackCommit),value);
    }

}

void QPE::setOptionalHeader_SizeOfHeapReserve(qint64 value)
{
    if(is64())
    {
        write_uint64(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER64,SizeOfHeapReserve),value);
    }
    else
    {
        write_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,SizeOfHeapReserve),value);
    }

}

void QPE::setOptionalHeader_SizeOfHeapCommit(qint64 value)
{
    if(is64())
    {
        write_uint64(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER64,SizeOfHeapCommit),value);
    }
    else
    {
        write_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,SizeOfHeapCommit),value);
    }

}

void QPE::setOptionalHeader_LoaderFlags(quint32 value)
{
    if(is64())
    {
        write_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER64,LoaderFlags),value);
    }
    else
    {
        write_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,LoaderFlags),value);
    }

}

void QPE::setOptionalHeader_NumberOfRvaAndSizes(quint32 value)
{
    if(is64())
    {
        write_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER64,NumberOfRvaAndSizes),value);
    }
    else
    {
        write_uint32(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,NumberOfRvaAndSizes),value);
    }

}

S_IMAGE_DATA_DIRECTORY QPE::getOptionalHeader_DataDirectory(quint32 nNumber)
{
    S_IMAGE_DATA_DIRECTORY result= {0};

    if(nNumber<getOptionalHeader_NumberOfRvaAndSizes())
    {
        if(is64())
        {
            read_array(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER64,DataDirectory)+nNumber*sizeof(S_IMAGE_DATA_DIRECTORY),(char *)&result,sizeof(S_IMAGE_DATA_DIRECTORY));
        }
        else
        {
            read_array(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,DataDirectory)+nNumber*sizeof(S_IMAGE_DATA_DIRECTORY),(char *)&result,sizeof(S_IMAGE_DATA_DIRECTORY));
        }
    }

    return result;
}

void QPE::setOptionalHeader_DataDirectory(quint32 nNumber,S_IMAGE_DATA_DIRECTORY *pDataDirectory)
{
    if(nNumber<16)
    {
        if(is64())
        {
            write_array(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER64,DataDirectory)+nNumber*sizeof(S_IMAGE_DATA_DIRECTORY),(char *)pDataDirectory,sizeof(S_IMAGE_DATA_DIRECTORY));
        }
        else
        {
            write_array(getOptionalHeaderOffset()+offsetof(S_IMAGE_OPTIONAL_HEADER32,DataDirectory)+nNumber*sizeof(S_IMAGE_DATA_DIRECTORY),(char *)pDataDirectory,sizeof(S_IMAGE_DATA_DIRECTORY));
        }
    }
}

bool QPE::isOptionalHeader_DataDirectoryPresent(quint32 nNumber)
{
    S_IMAGE_DATA_DIRECTORY dd=getOptionalHeader_DataDirectory(nNumber);

    //    return (dd.Size)&&(dd.VirtualAddress)&&(isAddressValid(dd.VirtualAddress+getBaseAddress())); // TODO Check
    return (dd.Size)&&(dd.VirtualAddress);
}

QList<S_IMAGE_DATA_DIRECTORY> QPE::getDirectories()
{
    QList<S_IMAGE_DATA_DIRECTORY> listResult;

    int nCount=getOptionalHeader_NumberOfRvaAndSizes();
    nCount=qMin(nCount,16);

    qint64 nDirectoriesOffset=getOptionalHeaderOffset();

    if(is64())
    {
        nDirectoriesOffset+=offsetof(S_IMAGE_OPTIONAL_HEADER64,DataDirectory);
    }
    else
    {
        nDirectoriesOffset+=offsetof(S_IMAGE_OPTIONAL_HEADER32,DataDirectory);
    }

    for(int i=0; i<nCount; i++)
    {
        S_IMAGE_DATA_DIRECTORY record= {0};

        read_array(nDirectoriesOffset+i*sizeof(S_IMAGE_DATA_DIRECTORY),(char *)&record,sizeof(S_IMAGE_DATA_DIRECTORY));

        listResult.append(record);
    }

    return listResult;
}

void QPE::setDirectories(QList<S_IMAGE_DATA_DIRECTORY> *pListDirectories)
{
    int nCount=getOptionalHeader_NumberOfRvaAndSizes();
    nCount=qMin(nCount,16);

    qint64 nDirectoriesOffset=getOptionalHeaderOffset();

    if(is64())
    {
        nDirectoriesOffset+=offsetof(S_IMAGE_OPTIONAL_HEADER64,DataDirectory);
    }
    else
    {
        nDirectoriesOffset+=offsetof(S_IMAGE_OPTIONAL_HEADER32,DataDirectory);
    }

    for(int i=0; i<nCount; i++)
    {
        write_array(nDirectoriesOffset+i*sizeof(S_IMAGE_DATA_DIRECTORY),(char *)&(pListDirectories->at(i)),sizeof(S_IMAGE_DATA_DIRECTORY));
    }
}

qint64 QPE::getDataDirectoryOffset(quint32 nNumber)
{
    S_IMAGE_DATA_DIRECTORY dataResources=getOptionalHeader_DataDirectory(nNumber);

    if(dataResources.VirtualAddress)
    {
        QList<MEMORY_MAP> list=getMemoryMapList();
        qint64 nBaseAddress=getBaseAddress();

        return addressToOffset(&list,dataResources.VirtualAddress+nBaseAddress);
    }

    return -1;
}

QByteArray QPE::getDataDirectory(quint32 nNumber)
{
    QByteArray baResult;
    S_IMAGE_DATA_DIRECTORY dataDirectory=getOptionalHeader_DataDirectory(nNumber);

    if(dataDirectory.VirtualAddress)
    {
        QList<MEMORY_MAP> list=getMemoryMapList();
        qint64 nBaseAddress=getBaseAddress();

        qint64 nOffset=addressToOffset(&list,dataDirectory.VirtualAddress+nBaseAddress);

        if(nOffset!=-1)
        {
            baResult=read_array(nOffset,dataDirectory.Size);
        }
    }

    return baResult;
}

qint64 QPE::getSectionsTableOffset()
{
    qint64 nResult=0;

    nResult=getOptionalHeaderOffset()+getFileHeader_SizeOfOptionalHeader();

    return nResult;
}

S_IMAGE_SECTION_HEADER QPE::getSectionHeader(quint32 nNumber)
{
    S_IMAGE_SECTION_HEADER result= {0};
    quint32 nNumberOfSections=getFileHeader_NumberOfSections();

    if(nNumber<nNumberOfSections)
    {
        read_array(getSectionsTableOffset()+nNumber*sizeof(S_IMAGE_SECTION_HEADER),(char *)&result,sizeof(S_IMAGE_SECTION_HEADER));
    }

    return result;
}

void QPE::setSectionHeader(quint32 nNumber, S_IMAGE_SECTION_HEADER *pSectionHeader)
{
    quint32 nNumberOfSections=getFileHeader_NumberOfSections();

    if(nNumber<nNumberOfSections)
    {
        write_array(getSectionsTableOffset()+nNumber*sizeof(S_IMAGE_SECTION_HEADER),(char *)pSectionHeader,sizeof(S_IMAGE_SECTION_HEADER));
    }
}

QList<S_IMAGE_SECTION_HEADER> QPE::getSectionHeaders()
{
    QList<S_IMAGE_SECTION_HEADER> listResult;

    quint32 nNumberOfSections=getFileHeader_NumberOfSections();
    qint64 nSectionOffset=getSectionsTableOffset();

    // Fix
    if(nNumberOfSections>100)
    {
        nNumberOfSections=100;
    }

    for(int i=0; i<(int)nNumberOfSections; i++)
    {
        S_IMAGE_SECTION_HEADER record= {0};

        read_array(nSectionOffset+i*sizeof(S_IMAGE_SECTION_HEADER),(char *)&record,sizeof(S_IMAGE_SECTION_HEADER));

        listResult.append(record);
    }

    return listResult;
}

QString QPE::getSection_NameAsString(quint32 nNumber)
{
    QString sResult;
    quint32 nNumberOfSections=getFileHeader_NumberOfSections();

    char cBuffer[9]= {0};

    if(nNumber<nNumberOfSections)
    {
        QBinary::read_array(getSectionsTableOffset()+nNumber*sizeof(S_IMAGE_SECTION_HEADER)+offsetof(S_IMAGE_SECTION_HEADER,Name),cBuffer,8);
    }

    sResult.append(cBuffer);

    return sResult;
}

quint32 QPE::getSection_VirtualSize(quint32 nNumber)
{
    quint32 nResult=0;
    quint32 nNumberOfSections=getFileHeader_NumberOfSections();

    if(nNumber<nNumberOfSections)
    {
        nResult=read_uint32(getSectionsTableOffset()+nNumber*sizeof(S_IMAGE_SECTION_HEADER)+offsetof(S_IMAGE_SECTION_HEADER,Misc.VirtualSize));
    }

    return nResult;
}

quint32 QPE::getSection_VirtualAddress(quint32 nNumber)
{
    quint32 nResult=0;
    quint32 nNumberOfSections=getFileHeader_NumberOfSections();

    if(nNumber<nNumberOfSections)
    {
        nResult=read_uint32(getSectionsTableOffset()+nNumber*sizeof(S_IMAGE_SECTION_HEADER)+offsetof(S_IMAGE_SECTION_HEADER,VirtualAddress));
    }

    return nResult;
}

quint32 QPE::getSection_SizeOfRawData(quint32 nNumber)
{
    quint32 nResult=0;
    quint32 nNumberOfSections=getFileHeader_NumberOfSections();

    if(nNumber<nNumberOfSections)
    {
        nResult=read_uint32(getSectionsTableOffset()+nNumber*sizeof(S_IMAGE_SECTION_HEADER)+offsetof(S_IMAGE_SECTION_HEADER,SizeOfRawData));
    }

    return nResult;
}

quint32 QPE::getSection_PointerToRawData(quint32 nNumber)
{
    quint32 nResult=0;
    quint32 nNumberOfSections=getFileHeader_NumberOfSections();

    if(nNumber<nNumberOfSections)
    {
        nResult=read_uint32(getSectionsTableOffset()+nNumber*sizeof(S_IMAGE_SECTION_HEADER)+offsetof(S_IMAGE_SECTION_HEADER,PointerToRawData));
    }

    return nResult;
}

quint32 QPE::getSection_PointerToRelocations(quint32 nNumber)
{
    quint32 nResult=0;
    quint32 nNumberOfSections=getFileHeader_NumberOfSections();

    if(nNumber<nNumberOfSections)
    {
        nResult=read_uint32(getSectionsTableOffset()+nNumber*sizeof(S_IMAGE_SECTION_HEADER)+offsetof(S_IMAGE_SECTION_HEADER,PointerToRelocations));
    }

    return nResult;
}

quint32 QPE::getSection_PointerToLinenumbers(quint32 nNumber)
{
    quint32 nResult=0;
    quint32 nNumberOfSections=getFileHeader_NumberOfSections();

    if(nNumber<nNumberOfSections)
    {
        nResult=read_uint32(getSectionsTableOffset()+nNumber*sizeof(S_IMAGE_SECTION_HEADER)+offsetof(S_IMAGE_SECTION_HEADER,PointerToLinenumbers));
    }

    return nResult;
}

quint16 QPE::getSection_NumberOfRelocations(quint32 nNumber)
{
    quint16 nResult=0;
    quint32 nNumberOfSections=getFileHeader_NumberOfSections();

    if(nNumber<nNumberOfSections)
    {
        nResult=read_uint16(getSectionsTableOffset()+nNumber*sizeof(S_IMAGE_SECTION_HEADER)+offsetof(S_IMAGE_SECTION_HEADER,NumberOfRelocations));
    }

    return nResult;
}

quint16 QPE::getSection_NumberOfLinenumbers(quint32 nNumber)
{
    quint16 nResult=0;
    quint32 nNumberOfSections=getFileHeader_NumberOfSections();

    if(nNumber<nNumberOfSections)
    {
        nResult=read_uint16(getSectionsTableOffset()+nNumber*sizeof(S_IMAGE_SECTION_HEADER)+offsetof(S_IMAGE_SECTION_HEADER,NumberOfLinenumbers));
    }

    return nResult;
}

quint32 QPE::getSection_Characteristics(quint32 nNumber)
{
    quint32 nResult=0;
    quint32 nNumberOfSections=getFileHeader_NumberOfSections();

    if(nNumber<nNumberOfSections)
    {
        nResult=read_uint32(getSectionsTableOffset()+nNumber*sizeof(S_IMAGE_SECTION_HEADER)+offsetof(S_IMAGE_SECTION_HEADER,Characteristics));
    }

    return nResult;
}

void QPE::setSection_NameAsString(quint32 nNumber, QString sName)
{
    quint32 nNumberOfSections=getFileHeader_NumberOfSections();

    char cBuffer[9]= {0};

    sName.resize(8);

    strcpy(cBuffer,sName.toLatin1().data());

    if(nNumber<nNumberOfSections)
    {
        QBinary::write_array(getSectionsTableOffset()+nNumber*sizeof(S_IMAGE_SECTION_HEADER)+offsetof(S_IMAGE_SECTION_HEADER,Name),cBuffer,8);
    }
}

void QPE::setSection_VirtualSize(quint32 nNumber, quint32 value)
{
    quint32 nNumberOfSections=getFileHeader_NumberOfSections();

    if(nNumber<nNumberOfSections)
    {
        write_uint32(getSectionsTableOffset()+nNumber*sizeof(S_IMAGE_SECTION_HEADER)+offsetof(S_IMAGE_SECTION_HEADER,Misc.VirtualSize),value);
    }
}

void QPE::setSection_VirtualAddress(quint32 nNumber, quint32 value)
{
    quint32 nNumberOfSections=getFileHeader_NumberOfSections();

    if(nNumber<nNumberOfSections)
    {
        write_uint32(getSectionsTableOffset()+nNumber*sizeof(S_IMAGE_SECTION_HEADER)+offsetof(S_IMAGE_SECTION_HEADER,VirtualAddress),value);
    }
}

void QPE::setSection_SizeOfRawData(quint32 nNumber, quint32 value)
{
    quint32 nNumberOfSections=getFileHeader_NumberOfSections();

    if(nNumber<nNumberOfSections)
    {
        write_uint32(getSectionsTableOffset()+nNumber*sizeof(S_IMAGE_SECTION_HEADER)+offsetof(S_IMAGE_SECTION_HEADER,SizeOfRawData),value);
    }
}

void QPE::setSection_PointerToRawData(quint32 nNumber, quint32 value)
{
    quint32 nNumberOfSections=getFileHeader_NumberOfSections();

    if(nNumber<nNumberOfSections)
    {
        write_uint32(getSectionsTableOffset()+nNumber*sizeof(S_IMAGE_SECTION_HEADER)+offsetof(S_IMAGE_SECTION_HEADER,PointerToRawData),value);
    }
}

void QPE::setSection_PointerToRelocations(quint32 nNumber, quint32 value)
{
    quint32 nNumberOfSections=getFileHeader_NumberOfSections();

    if(nNumber<nNumberOfSections)
    {
        write_uint32(getSectionsTableOffset()+nNumber*sizeof(S_IMAGE_SECTION_HEADER)+offsetof(S_IMAGE_SECTION_HEADER,PointerToRelocations),value);
    }
}

void QPE::setSection_PointerToLinenumbers(quint32 nNumber, quint32 value)
{
    quint32 nNumberOfSections=getFileHeader_NumberOfSections();

    if(nNumber<nNumberOfSections)
    {
        write_uint32(getSectionsTableOffset()+nNumber*sizeof(S_IMAGE_SECTION_HEADER)+offsetof(S_IMAGE_SECTION_HEADER,PointerToLinenumbers),value);
    }
}

void QPE::setSection_NumberOfRelocations(quint32 nNumber, quint16 value)
{
    quint32 nNumberOfSections=getFileHeader_NumberOfSections();

    if(nNumber<nNumberOfSections)
    {
        write_uint16(getSectionsTableOffset()+nNumber*sizeof(S_IMAGE_SECTION_HEADER)+offsetof(S_IMAGE_SECTION_HEADER,NumberOfRelocations),value);
    }
}

void QPE::setSection_NumberOfLinenumbers(quint32 nNumber, quint16 value)
{
    quint32 nNumberOfSections=getFileHeader_NumberOfSections();

    if(nNumber<nNumberOfSections)
    {
        write_uint16(getSectionsTableOffset()+nNumber*sizeof(S_IMAGE_SECTION_HEADER)+offsetof(S_IMAGE_SECTION_HEADER,NumberOfLinenumbers),value);
    }
}

void QPE::setSection_Characteristics(quint32 nNumber, quint32 value)
{
    quint32 nNumberOfSections=getFileHeader_NumberOfSections();

    if(nNumber<nNumberOfSections)
    {
        write_uint32(getSectionsTableOffset()+nNumber*sizeof(S_IMAGE_SECTION_HEADER)+offsetof(S_IMAGE_SECTION_HEADER,Characteristics),value);
    }
}

bool QPE::isSectionNamePresent(QString sSectionName, QList<S_IMAGE_SECTION_HEADER> *pListSections)
{
    int nNumberOfSections=pListSections->count();

    for(int i=0; i<nNumberOfSections; i++)
    {
        QString _sSectionName=QString((char *)pListSections->at(i).Name);
        _sSectionName.resize(qMin(_sSectionName.length(),S_IMAGE_SIZEOF_SHORT_NAME));

        if(_sSectionName==sSectionName)
        {
            return true;
        }
    }

    return false;
}

S_IMAGE_SECTION_HEADER QPE::getSectionByName(QString sSectionName, QList<S_IMAGE_SECTION_HEADER> *pListSections)
{
    S_IMAGE_SECTION_HEADER result= {0};

    int nNumberOfSections=pListSections->count();

    for(int i=0; i<nNumberOfSections; i++)
    {
        QString _sSectionName=QString((char *)pListSections->at(i).Name);
        _sSectionName.resize(qMin(_sSectionName.length(),S_IMAGE_SIZEOF_SHORT_NAME));

        if(_sSectionName==sSectionName)
        {
            result=pListSections->at(i);

            break;
        }
    }

    return result;
}

bool QPE::isImportPresent()
{
    return isOptionalHeader_DataDirectoryPresent(S_IMAGE_DIRECTORY_ENTRY_IMPORT);
}

QList<QBinary::MEMORY_MAP> QPE::getMemoryMapList()
{
    QList<MEMORY_MAP> list;

    quint32 nNumberOfSections=getFileHeader_NumberOfSections();
    quint32 nFileAlignment=getOptionalHeader_FileAlignment();
    quint32 nSectionAlignment=getOptionalHeader_SectionAlignment();
    qint64 nBaseAddress=getOptionalHeader_ImageBase();
    quint32 nHeadersSize=getOptionalHeader_SizeOfHeaders(); // mb TODO calc for UPX

    if(nFileAlignment==nSectionAlignment)
    {
        nFileAlignment=1;
    }

    quint32 nVirtualSizeofHeaders=__ALIGN_UP(nHeadersSize,nSectionAlignment);
    qint64 nMaxOffset=0;

    MEMORY_MAP recordHeaderRaw= {0};

    if(!isImage())
    {
        recordHeaderRaw.bIsHeader=true;
        recordHeaderRaw.nAddress=nBaseAddress;
        recordHeaderRaw.nOffset=0;
        recordHeaderRaw.nSize=nHeadersSize;

        list.append(recordHeaderRaw);

        if(nVirtualSizeofHeaders-nHeadersSize)
        {
            MEMORY_MAP record= {0};
            record.bIsHeader=true;

            record.nAddress=nBaseAddress+nHeadersSize;
            record.nOffset=-1;
            record.nSize=nVirtualSizeofHeaders-nHeadersSize;

            list.append(record);
        }
    }
    else
    {
        recordHeaderRaw.bIsHeader=true;
        recordHeaderRaw.nAddress=nBaseAddress;
        recordHeaderRaw.nOffset=0;
        recordHeaderRaw.nSize=nVirtualSizeofHeaders;

        list.append(recordHeaderRaw);
    }

    nMaxOffset=recordHeaderRaw.nSize;


    for(quint32 i=0; i<nNumberOfSections; i++)
    {
        S_IMAGE_SECTION_HEADER section=getSectionHeader(i);
        // TODO for corrupted files
        qint64 nFileOffset=section.PointerToRawData;
        //
        nFileOffset=__ALIGN_DOWN(nFileOffset,nFileAlignment);
        qint64 nFileSize=__ALIGN_UP(section.SizeOfRawData,nFileAlignment);
        qint64 nVirtualAddress=nBaseAddress+section.VirtualAddress;
        qint64 nVirtualSize=__ALIGN_UP(section.Misc.VirtualSize,nSectionAlignment);

        nMaxOffset=qMax(nMaxOffset,(qint64)(nFileOffset+nFileSize));

        if(!isImage())
        {
            if(nFileSize)
            {
                MEMORY_MAP record= {0};

                record.bIsSection=true;
                record.nSection=i;

                record.nAddress=nVirtualAddress;
                record.nOffset=nFileOffset;
                record.nSize=nFileSize;

                list.append(record);
            }

            if(nVirtualSize-nFileSize)
            {
                MEMORY_MAP record= {0};

                record.bIsSection=true;
                record.nSection=i;

                record.nAddress=nVirtualAddress+nFileSize;
                record.nOffset=-1;
                record.nSize=nVirtualSize-nFileSize;

                list.append(record);
            }
        }
        else
        {
            MEMORY_MAP record= {0};

            record.bIsSection=true;
            record.nSection=i;

            record.nAddress=nVirtualAddress;
            record.nOffset=nVirtualAddress;
            record.nSize=nVirtualSize;

            list.append(record);
        }
    }

    // Overlay;
    MEMORY_MAP record= {0};

    record.bIsOvelay=true;

    record.nAddress=-1;
    record.nOffset=nMaxOffset;
    record.nSize=0;

    if(!isImage())
    {
        if(getSize()-nMaxOffset>0)
        {
            record.nSize=getSize()-nMaxOffset;
        }
    }

    list.append(record);

    return list;
}

qint64 QPE::getBaseAddress()
{
    return getOptionalHeader_ImageBase();
}

qint64 QPE::getEntryPointOffset()
{
    return addressToOffset(getBaseAddress()+getOptionalHeader_AddressOfEntryPoint());
}

QList<QPE::IMPORT_RECORD> QPE::getImportRecords()
{
    QList<IMPORT_RECORD> listResult;

    qint64 nImportOffset=getDataDirectoryOffset(S_IMAGE_DIRECTORY_ENTRY_IMPORT);

    if(nImportOffset!=-1)
    {
        QList<MEMORY_MAP> listMemoryMap=getMemoryMapList();
        qint64 nBaseAddress=getBaseAddress();
        bool bIs64=is64();

        while(true)
        {
            S_IMAGE_IMPORT_DESCRIPTOR iid=read_S_IMAGE_IMPORT_DESCRIPTOR(nImportOffset);

            QString sLibrary;

            if((iid.Characteristics==0)&&(iid.Name==0))
            {
                break;
            }

            qint64 nOffset=addressToOffset(&listMemoryMap,iid.Name+nBaseAddress);

            if(nOffset!=-1)
            {
                sLibrary=read_ansiString(nOffset);

                if(sLibrary=="")
                {
                    break;
                }
            }
            else
            {
                break; // corrupted
            }

            qint64 nThunksOffset=-1;
            qint64 nRVA=0;

            if(iid.OriginalFirstThunk)
            {
                nThunksOffset=addressToOffset(&listMemoryMap,iid.OriginalFirstThunk+nBaseAddress);
                //                nRVA=iid.OriginalFirstThunk;
            }
            else if((iid.FirstThunk))
            {
                nThunksOffset=addressToOffset(&listMemoryMap,iid.FirstThunk+nBaseAddress);
                //                nRVA=iid.FirstThunk;
            }

            nRVA=iid.FirstThunk;

            if(nThunksOffset==-1)
            {
                break;
            }

            while(true)
            {
                QString sFunction;

                if(bIs64)
                {
                    qint64 nThunk64=read_uint64(nThunksOffset);

                    if(nThunk64==0)
                    {
                        break;
                    }

                    if(!(nThunk64&0x8000000000000000))
                    {
                        qint64 nOffset=addressToOffset(&listMemoryMap,nThunk64+nBaseAddress);

                        if(nOffset!=-1)
                        {
                            sFunction=read_ansiString(nOffset+2);

                            if(sFunction=="")
                            {
                                break;
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                    else
                    {
                        sFunction=QString("%1").arg(nThunk64&0x7FFFFFFFFFFFFFFF);
                    }

                }
                else
                {
                    qint64 nThunk32=read_uint32(nThunksOffset);

                    if(nThunk32==0)
                    {
                        break;
                    }

                    if(!(nThunk32&0x80000000))
                    {
                        qint64 nOffset=addressToOffset(&listMemoryMap,nThunk32+nBaseAddress);

                        if(nOffset!=-1)
                        {
                            sFunction=read_ansiString(nOffset+2);

                            if(sFunction=="")
                            {
                                break;
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                    else
                    {
                        sFunction=QString("%1").arg(nThunk32&0x7FFFFFFF);
                    }
                }

                IMPORT_RECORD record;

                record.nOffset=nThunksOffset;
                record.nRVA=nRVA;
                record.sLibrary=sLibrary;
                record.sFunction=sFunction;

                listResult.append(record);

                if(bIs64)
                {
                    nThunksOffset+=8;
                    nRVA+=8;
                }
                else
                {
                    nThunksOffset+=4;
                    nRVA+=4;
                }
            }

            nImportOffset+=sizeof(S_IMAGE_IMPORT_DESCRIPTOR);
        }
    }

    return listResult;
}

QList<S_IMAGE_IMPORT_DESCRIPTOR> QPE::getImportDescriptors()
{
    QList<S_IMAGE_IMPORT_DESCRIPTOR> listResult;

    qint64 nImportOffset=getDataDirectoryOffset(S_IMAGE_DIRECTORY_ENTRY_IMPORT);

    if(nImportOffset!=-1)
    {
        QList<MEMORY_MAP> listMemoryMap=getMemoryMapList();
        qint64 nBaseAddress=getBaseAddress();

        while(true)
        {
            S_IMAGE_IMPORT_DESCRIPTOR iid=read_S_IMAGE_IMPORT_DESCRIPTOR(nImportOffset);


            if((iid.Characteristics==0)&&(iid.Name==0))
            {
                break;
            }

            qint64 nOffset=addressToOffset(&listMemoryMap,iid.Name+nBaseAddress);

            if(nOffset!=-1)
            {
                QString sName=read_ansiString(nOffset);

                if(sName=="")
                {
                    break;
                }
            }
            else
            {
                break; // corrupted
            }


            listResult.append(iid);

            nImportOffset+=sizeof(S_IMAGE_IMPORT_DESCRIPTOR);
        }
    }

    return listResult;
}

QList<S_IMAGE_IMPORT_DESCRIPTOR_EX> QPE::getImportDescriptorsEx()
{
    QList<S_IMAGE_IMPORT_DESCRIPTOR_EX> listResult;

    qint64 nImportOffset=getDataDirectoryOffset(S_IMAGE_DIRECTORY_ENTRY_IMPORT);

    if(nImportOffset!=-1)
    {
        QList<MEMORY_MAP> listMemoryMap=getMemoryMapList();
        qint64 nBaseAddress=getBaseAddress();
        //        bool bIs64=is64();

        while(true)
        {
            S_IMAGE_IMPORT_DESCRIPTOR_EX record= {0};
            S_IMAGE_IMPORT_DESCRIPTOR iid=read_S_IMAGE_IMPORT_DESCRIPTOR(nImportOffset);


            if((iid.Characteristics==0)&&(iid.Name==0))
            {
                break;
            }

            qint64 nOffset=addressToOffset(&listMemoryMap,iid.Name+nBaseAddress);

            if(nOffset!=-1)
            {
                record.sLibrary=read_ansiString(nOffset);

                if(record.sLibrary=="")
                {
                    break;
                }
            }
            else
            {
                break; // corrupted
            }

            record.Characteristics=iid.Characteristics;
            record.FirstThunk=iid.FirstThunk;
            record.ForwarderChain=iid.ForwarderChain;
            record.Name=iid.Name;
            record.OriginalFirstThunk=iid.OriginalFirstThunk;
            record.TimeDateStamp=iid.TimeDateStamp;

            listResult.append(record);

            nImportOffset+=sizeof(S_IMAGE_IMPORT_DESCRIPTOR);
        }
    }

    return listResult;
}

S_IMAGE_IMPORT_DESCRIPTOR QPE::getImportDescriptor(quint32 nNumber)
{
    S_IMAGE_IMPORT_DESCRIPTOR result= {0};

    qint64 nImportOffset=getDataDirectoryOffset(S_IMAGE_DIRECTORY_ENTRY_IMPORT);

    if(nImportOffset!=-1)
    {
        nImportOffset+=nNumber*sizeof(S_IMAGE_IMPORT_DESCRIPTOR);

        result=read_S_IMAGE_IMPORT_DESCRIPTOR(nImportOffset);
    }

    return result;
}

void QPE::setImportDescriptor(quint32 nNumber, S_IMAGE_IMPORT_DESCRIPTOR *pImportDescriptor)
{
    qint64 nImportOffset=getDataDirectoryOffset(S_IMAGE_DIRECTORY_ENTRY_IMPORT);

    if(nImportOffset!=-1)
    {
        nImportOffset+=nNumber*sizeof(S_IMAGE_IMPORT_DESCRIPTOR);

        write_S_IMAGE_IMPORT_DESCRIPTOR(nImportOffset,*pImportDescriptor);
    }
}

QList<QPE::IMPORT_HEADER> QPE::getImports()
{
    QList<IMPORT_HEADER> listResult;

    qint64 nImportOffset=getDataDirectoryOffset(S_IMAGE_DIRECTORY_ENTRY_IMPORT);

    if(nImportOffset!=-1)
    {
        QList<MEMORY_MAP> listMemoryMap=getMemoryMapList();
        qint64 nBaseAddress=getBaseAddress();
        bool bIs64=is64();

        while(true)
        {
            IMPORT_HEADER importHeader= {0};
            S_IMAGE_IMPORT_DESCRIPTOR iid=read_S_IMAGE_IMPORT_DESCRIPTOR(nImportOffset);


            if((iid.Characteristics==0)&&(iid.Name==0))
            {
                break;
            }

            qint64 nOffset=addressToOffset(&listMemoryMap,iid.Name+nBaseAddress);

            if(nOffset!=-1)
            {
                importHeader.sName=read_ansiString(nOffset);

                if(importHeader.sName=="")
                {
                    break;
                }
            }
            else
            {
                break; // corrupted
            }

            qint64 nThunksOffset=-1;
            qint64 nThunksRVA=0;
            qint64 nThunksOriginalRVA=0;
            qint64 nThunksOriginalOffset=0;

            if(iid.OriginalFirstThunk)
            {
                nThunksRVA=iid.OriginalFirstThunk;
                //                nRVA=iid.OriginalFirstThunk;
            }
            else if((iid.FirstThunk))
            {
                nThunksRVA=iid.FirstThunk;
                //                nRVA=iid.FirstThunk;
            }

            nThunksOriginalRVA=iid.FirstThunk;

            nThunksOffset=addressToOffset(&listMemoryMap,nThunksRVA+nBaseAddress);
            nThunksOriginalOffset=addressToOffset(&listMemoryMap,nThunksOriginalRVA+nBaseAddress);

            if(nThunksOffset==-1)
            {
                break;
            }

            while(true)
            {
                IMPORT_POSITION importPosition= {0};
                importPosition.nThunkRVA=nThunksOriginalRVA;
                importPosition.nThunkOffset=nThunksOriginalOffset;

                if(bIs64)
                {
                    importPosition.nThunkValue=read_uint64(nThunksOffset);

                    if(importPosition.nThunkValue==0)
                    {
                        break;
                    }

                    if(!(importPosition.nThunkValue&0x8000000000000000))
                    {
                        qint64 nOffset=addressToOffset(&listMemoryMap,importPosition.nThunkValue+nBaseAddress);

                        if(nOffset!=-1)
                        {
                            importPosition.nHint=read_uint16(nOffset);
                            importPosition.sName=read_ansiString(nOffset+2);

                            if(importPosition.sName=="")
                            {
                                break;
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                    else
                    {
                        importPosition.nOrdinal=importPosition.nThunkValue&0x7FFFFFFFFFFFFFFF;
                    }

                }
                else
                {
                    importPosition.nThunkValue=read_uint32(nThunksOffset);

                    if(importPosition.nThunkValue==0)
                    {
                        break;
                    }

                    if(!(importPosition.nThunkValue&0x80000000))
                    {
                        qint64 nOffset=addressToOffset(&listMemoryMap,importPosition.nThunkValue+nBaseAddress);

                        if(nOffset!=-1)
                        {
                            importPosition.nHint=read_uint16(nOffset);
                            importPosition.sName=read_ansiString(nOffset+2);

                            if(importPosition.sName=="")
                            {
                                break;
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                    else
                    {
                        importPosition.nOrdinal=importPosition.nThunkValue&0x7FFFFFFF;
                    }
                }

                if(importPosition.nOrdinal==0)
                {
                    importPosition.sFunction=importPosition.sName;
                }
                else
                {
                    importPosition.sFunction=QString("%1").arg(importPosition.nOrdinal);
                }

                if(bIs64)
                {
                    nThunksOffset+=8;
                    nThunksRVA+=8;
                    nThunksOriginalRVA+=8;
                    nThunksOriginalOffset+=8;
                }
                else
                {
                    nThunksOffset+=4;
                    nThunksRVA+=4;
                    nThunksOriginalRVA+=4;
                    nThunksOriginalOffset+=4;
                }

                importHeader.listPositions.append(importPosition);
            }

            listResult.append(importHeader);

            nImportOffset+=sizeof(S_IMAGE_IMPORT_DESCRIPTOR);
        }
    }

    return listResult;
}

QList<QPE::IMPORT_POSITION> QPE::getImportPositions(int nIndex)
{
    QList<IMPORT_POSITION> listResult;

    qint64 nImportOffset=getDataDirectoryOffset(S_IMAGE_DIRECTORY_ENTRY_IMPORT);

    if(nImportOffset!=-1)
    {
        QList<MEMORY_MAP> listMemoryMap=getMemoryMapList();
        qint64 nBaseAddress=getBaseAddress();
        bool bIs64=is64();

        int _nIndex=0;

        while(true)
        {
            IMPORT_HEADER importHeader= {0};
            S_IMAGE_IMPORT_DESCRIPTOR iid=read_S_IMAGE_IMPORT_DESCRIPTOR(nImportOffset);

            if((iid.Characteristics==0)&&(iid.Name==0))
            {
                break;
            }

            qint64 nOffset=addressToOffset(&listMemoryMap,iid.Name+nBaseAddress);

            if(nOffset!=-1)
            {
                importHeader.sName=read_ansiString(nOffset);

                if(importHeader.sName=="")
                {
                    break;
                }
            }
            else
            {
                break; // corrupted
            }

            qint64 nThunksOffset=-1;
            qint64 nRVA=0;
            qint64 nThunksRVA=-1;

            if(iid.OriginalFirstThunk)
            {
                nThunksRVA=iid.OriginalFirstThunk+nBaseAddress;
                nThunksOffset=addressToOffset(&listMemoryMap,nThunksRVA);
                //                nRVA=iid.OriginalFirstThunk;
            }
            else if((iid.FirstThunk))
            {
                nThunksRVA=iid.FirstThunk+nBaseAddress;
                nThunksOffset=addressToOffset(&listMemoryMap,nThunksRVA);
                //                nRVA=iid.FirstThunk;
            }

            nRVA=iid.FirstThunk;

            if(nThunksOffset==-1)
            {
                break;
            }

            if(_nIndex==nIndex)
            {
                while(true)
                {
                    IMPORT_POSITION importPosition= {0};
                    importPosition.nThunkOffset=nThunksOffset;
                    importPosition.nThunkRVA=nThunksRVA;

                    if(bIs64)
                    {
                        importPosition.nThunkValue=read_uint64(nThunksOffset);

                        if(importPosition.nThunkValue==0)
                        {
                            break;
                        }

                        if(!(importPosition.nThunkValue&0x8000000000000000))
                        {
                            qint64 nOffset=addressToOffset(&listMemoryMap,importPosition.nThunkValue+nBaseAddress);

                            if(nOffset!=-1)
                            {
                                importPosition.nHint=read_uint16(nOffset);
                                importPosition.sName=read_ansiString(nOffset+2);

                                if(importPosition.sName=="")
                                {
                                    break;
                                }
                            }
                            else
                            {
                                break;
                            }
                        }
                        else
                        {
                            importPosition.nOrdinal=importPosition.nThunkValue&0x7FFFFFFFFFFFFFFF;
                        }

                    }
                    else
                    {
                        importPosition.nThunkValue=read_uint32(nThunksOffset);

                        if(importPosition.nThunkValue==0)
                        {
                            break;
                        }

                        if(!(importPosition.nThunkValue&0x80000000))
                        {
                            qint64 nOffset=addressToOffset(&listMemoryMap,importPosition.nThunkValue+nBaseAddress);

                            if(nOffset!=-1)
                            {
                                importPosition.nHint=read_uint16(nOffset);
                                importPosition.sName=read_ansiString(nOffset+2);

                                if(importPosition.sName=="")
                                {
                                    break;
                                }
                            }
                            else
                            {
                                break;
                            }
                        }
                        else
                        {
                            importPosition.nOrdinal=importPosition.nThunkValue&0x7FFFFFFF;
                        }
                    }

                    if(bIs64)
                    {
                        nThunksRVA+=8;
                        nThunksOffset+=8;
                        nRVA+=8;
                    }
                    else
                    {
                        nThunksRVA+=8;
                        nThunksOffset+=4;
                        nRVA+=4;
                    }

                    listResult.append(importPosition);
                }

                break;
            }

            nImportOffset+=sizeof(S_IMAGE_IMPORT_DESCRIPTOR);
            _nIndex++;
        }
    }

    return listResult;
}

bool QPE::isImportLibraryPresentI(QString sLibrary, QList<QPE::IMPORT_HEADER> *pListImport)
{
    bool bResult=false;

    for(int i=0; i<pListImport->count(); i++)
    {
        if(pListImport->at(i).sName.toUpper()==sLibrary.toUpper())
        {
            bResult=true;
            break;
        }
    }

    return bResult;
}

bool QPE::setImports(QList<QPE::IMPORT_HEADER> *pListHeaders)
{
    return setImports(getDevice(),pListHeaders);
}

bool QPE::setImports(QIODevice *pDevice, QList<QPE::IMPORT_HEADER> *pListHeaders)
{
    bool bResult=false;

    QString sClassName=pDevice->metaObject()->className();

    if(sClassName=="QFile")
    {
        QPE pe(pDevice);

        if(pe.isValid())
        {
            int nAddressSize=4;

            if(pe.is64())
            {
                nAddressSize=8;
            }
            else
            {
                nAddressSize=4;
            }

            QByteArray baImport;
            QList<qint64> listPatches; // Addresses for patch
            //    QMap<qint64,qint64> mapMove;

            // Calculate
            quint32 nIATSize=0;
            quint32 nImportTableSize=(pListHeaders->count()+1)*sizeof(S_IMAGE_IMPORT_DESCRIPTOR);
            quint32 nAnsiDataSize=0;

            for(int i=0; i<pListHeaders->count(); i++)
            {
                // TODO 64
                nIATSize+=(pListHeaders->at(i).listPositions.count()+1)*nAddressSize;
                nAnsiDataSize+=pListHeaders->at(i).sName.length()+3;

                for(int j=0; j<pListHeaders->at(i).listPositions.count(); j++)
                {
                    if(pListHeaders->at(i).listPositions.at(j).sName!="")
                    {
                        nAnsiDataSize+=2+pListHeaders->at(i).listPositions.at(j).sName.length()+1;
                    }
                }
            }

            nImportTableSize=__ALIGN_UP(nImportTableSize,16);
            nIATSize=__ALIGN_UP(nIATSize,16);
            nAnsiDataSize=__ALIGN_UP(nAnsiDataSize,16);


            baImport.resize(nIATSize+nImportTableSize+nIATSize+nAnsiDataSize);
            baImport.fill(0);


            char *pDataOffset=baImport.data();
            char *pIAT=pDataOffset;
            S_IMAGE_IMPORT_DESCRIPTOR *pIID=(S_IMAGE_IMPORT_DESCRIPTOR *)(pDataOffset+nIATSize);
            char *pOIAT=pDataOffset+nIATSize+nImportTableSize;
            char *pAnsiData=pDataOffset+nIATSize+nImportTableSize+nIATSize;

            for(int i=0; i<pListHeaders->count(); i++)
            {
                pIID->FirstThunk=pIAT-pDataOffset;
                listPatches.append((char *)pIID-pDataOffset+offsetof(S_IMAGE_IMPORT_DESCRIPTOR,FirstThunk));


                pIID->Name=pAnsiData-pDataOffset;
                listPatches.append((char *)pIID-pDataOffset+offsetof(S_IMAGE_IMPORT_DESCRIPTOR,Name));

                pIID->OriginalFirstThunk=pOIAT-pDataOffset;
                listPatches.append((char *)pIID-pDataOffset+offsetof(S_IMAGE_IMPORT_DESCRIPTOR,OriginalFirstThunk));

                strcpy(pAnsiData,pListHeaders->at(i).sName.toLatin1().data());
                pAnsiData+=pListHeaders->at(i).sName.length()+3;

                for(int j=0; j<pListHeaders->at(i).listPositions.count(); j++)
                {
                    if(pListHeaders->at(i).listPositions.at(j).sName!="")
                    {
                        *((quint32 *)pOIAT)=pAnsiData-pDataOffset;
                        *((quint32 *)pIAT)=*((quint32 *)pOIAT);

                        listPatches.append(pOIAT-pDataOffset);
                        listPatches.append(pIAT-pDataOffset);

                        *((quint16 *)pAnsiData)=pListHeaders->at(i).listPositions.at(j).nHint;
                        pAnsiData+=2;

                        strcpy(pAnsiData,pListHeaders->at(i).listPositions.at(j).sName.toLatin1().data());

                        pAnsiData+=pListHeaders->at(i).listPositions.at(j).sName.length()+1;
                    }
                    else
                    {
                        // TODO 64
                        if(nAddressSize==4)
                        {
                            *((quint32 *)pOIAT)=pListHeaders->at(i).listPositions.at(j).nOrdinal+0x80000000;
                            *((quint32 *)pIAT)=*((quint32 *)pOIAT);
                        }
                        else
                        {
                            *((quint64 *)pOIAT)=pListHeaders->at(i).listPositions.at(j).nOrdinal+0x8000000000000000;
                            *((quint64 *)pIAT)=*((quint64 *)pOIAT);
                        }
                    }

                    //            if(pListHeaders->at(i).nFirstThunk)
                    //            {
                    //                mapMove.insert(pListHeaders->at(i).listPositions.at(j).nThunkRVA,pIAT-pDataOffset);
                    //            }

                    pIAT+=nAddressSize;
                    pOIAT+=nAddressSize;


                }

                pIAT+=nAddressSize;
                pOIAT+=nAddressSize;
                pIID++;
            }

            S_IMAGE_SECTION_HEADER ish= {0};

            ish.Characteristics=0xc0000040;

            // TODO section name!

            //    char *pszTest="ABCDE";

            //    addSection(sFileName,&ish,pszTest,5);

            if(addSection(pDevice,&ish,baImport.data(),baImport.size()))
            {
                QList<MEMORY_MAP> listMP=pe.getMemoryMapList();
                qint64 nBaseAddress=pe.getBaseAddress();

                S_IMAGE_DATA_DIRECTORY iddIAT= {0};
                iddIAT.VirtualAddress=ish.VirtualAddress;
                iddIAT.Size=nIATSize;
                S_IMAGE_DATA_DIRECTORY iddImportTable= {0};
                iddImportTable.VirtualAddress=nIATSize+ish.VirtualAddress;
                iddImportTable.Size=nImportTableSize;

                pe.setOptionalHeader_DataDirectory(S_IMAGE_DIRECTORY_ENTRY_IAT,&iddIAT);
                pe.setOptionalHeader_DataDirectory(S_IMAGE_DIRECTORY_ENTRY_IMPORT,&iddImportTable);

                for(int i=0; i<listPatches.count(); i++)
                {
                    // TODO 64
                    qint64 nCurrentOffset=ish.PointerToRawData+listPatches.at(i);
                    quint32 nValue=pe.read_uint32(nCurrentOffset);
                    pe.write_uint32(nCurrentOffset,nValue+ish.VirtualAddress);
                }

                for(int i=0; i<pListHeaders->count(); i++)
                {
                    if(pListHeaders->at(i).nFirstThunk)
                    {
                        S_IMAGE_IMPORT_DESCRIPTOR iid=pe.getImportDescriptor(i);

                        //                        qDebug("pListHeaders->at(i).nFirstThunk(%d): %x",i,(quint32)pListHeaders->at(i).nFirstThunk);
                        //                        qDebug("FirstThunk(%d): %x",i,(quint32)iid.FirstThunk);
                        //                        qDebug("Import offset(%d): %x",i,(quint32)pe.getDataDirectoryOffset(S_IMAGE_DIRECTORY_ENTRY_IMPORT));

                        qint64 nSrcOffset=pe.addressToOffset(&listMP,iid.FirstThunk+nBaseAddress);
                        qint64 nDstOffset=pe.addressToOffset(&listMP,pListHeaders->at(i).nFirstThunk+nBaseAddress);

                        //                        qDebug("src: %x",(quint32)nSrcOffset);
                        //                        qDebug("dst: %x",(quint32)nDstOffset);

                        if((nSrcOffset!=-1)&&(nDstOffset!=-1))
                        {
                            // TODO 64 ????
                            while(true)
                            {
                                quint32 nValue=pe.read_uint32(nSrcOffset);

                                pe.write_uint32(nDstOffset,nValue);

                                if(nValue==0)
                                {
                                    break;
                                }

                                nSrcOffset+=nAddressSize;
                                nDstOffset+=nAddressSize;
                            }

                            //                            iid.OriginalFirstThunk=0;
                            iid.FirstThunk=pListHeaders->at(i).nFirstThunk;

                            pe.setImportDescriptor(i,&iid);
                        }
                    }
                }

                bResult=true;
            }
        }
    }

    return bResult;
}

bool QPE::setImports(QString sFileName,QList<QPE::IMPORT_HEADER> *pListHeaders)
{
    bool bResult=false;

    QFile file(sFileName);

    if(file.open(QIODevice::ReadWrite))
    {
        bResult=setImports(&file,pListHeaders);

        file.close();
    }

    return bResult;
}

QList<QPE::RESOURCE_HEADER> QPE::getResources()
{
    // TODO BE LE
    QList<RESOURCE_HEADER> listResources;

    qint64 nResourceOffset=getDataDirectoryOffset(S_IMAGE_DIRECTORY_ENTRY_RESOURCE);

    if(nResourceOffset!=-1)
    {
        QList<MEMORY_MAP> memoryMap=getMemoryMapList();
        qint64 nBaseAddress=getBaseAddress();
        RESOURCE_HEADER record= {0};


        qint64 nOffsetLevel[3]= {0};
        S_IMAGE_RESOURCE_DIRECTORY rd[3]= {0};
        S_IMAGE_RESOURCE_DIRECTORY_ENTRY rde[3]= {0};
        RESOURCES_ID_NAME irin[3]= {0};

        nOffsetLevel[0]=nResourceOffset;
        rd[0]=read_S_IMAGE_RESOURCE_DIRECTORY(nOffsetLevel[0]);

        if((rd[0].NumberOfIdEntries+rd[0].NumberOfNamedEntries<=100)&&(rd[0].Characteristics==0)) // check corrupted
        {
            nOffsetLevel[0]+=sizeof(S_IMAGE_RESOURCE_DIRECTORY);

            for(int i=0; i<rd[0].NumberOfIdEntries+rd[0].NumberOfNamedEntries; i++)
            {
                rde[0]=read_S_IMAGE_RESOURCE_DIRECTORY_ENTRY(nOffsetLevel[0]);

                irin[0]=getResourcesIDName(nResourceOffset,rde[0].Name);
                record.nID[0]=irin[0].nID;
                record.sName[0]=irin[0].sName;
                record.nNameOffset[0]=irin[0].nNameOffset;

                nOffsetLevel[1]=nResourceOffset+rde[0].OffsetToDirectory;

                rd[1]=read_S_IMAGE_RESOURCE_DIRECTORY(nOffsetLevel[1]);

                if(rd[1].Characteristics!=0)
                {
                    break;
                }

                nOffsetLevel[1]+=sizeof(S_IMAGE_RESOURCE_DIRECTORY);

                for(int j=0; j<rd[1].NumberOfIdEntries+rd[1].NumberOfNamedEntries; j++)
                {
                    rde[1]=read_S_IMAGE_RESOURCE_DIRECTORY_ENTRY(nOffsetLevel[1]);

                    irin[1]=getResourcesIDName(nResourceOffset,rde[1].Name);
                    record.nID[1]=irin[1].nID;
                    record.sName[1]=irin[1].sName;
                    record.nNameOffset[1]=irin[1].nNameOffset;

                    nOffsetLevel[2]=nResourceOffset+rde[1].OffsetToDirectory;

                    rd[2]=read_S_IMAGE_RESOURCE_DIRECTORY(nOffsetLevel[2]);

                    if(rd[2].Characteristics!=0)
                    {
                        break;
                    }

                    nOffsetLevel[2]+=sizeof(S_IMAGE_RESOURCE_DIRECTORY);

                    for(int k=0; k<rd[2].NumberOfIdEntries+rd[2].NumberOfNamedEntries; k++)
                    {
                        rde[2]=read_S_IMAGE_RESOURCE_DIRECTORY_ENTRY(nOffsetLevel[2]);

                        irin[2]=getResourcesIDName(nResourceOffset,rde[2].Name);
                        record.nID[2]=irin[2].nID;
                        record.sName[2]=irin[2].sName;
                        record.nNameOffset[2]=irin[2].nNameOffset;

                        record.nIRDEOffset=rde[2].OffsetToData;
                        S_IMAGE_RESOURCE_DATA_ENTRY irde=read_S_IMAGE_RESOURCE_DATA_ENTRY(nResourceOffset+record.nIRDEOffset);
                        record.nAddress=irde.OffsetToData+nBaseAddress;
                        record.nOffset=addressToOffset(&memoryMap,record.nAddress);
                        record.nSize=irde.Size;

                        listResources.append(record);

                        nOffsetLevel[2]+=sizeof(S_IMAGE_RESOURCE_DIRECTORY_ENTRY);
                    }

                    nOffsetLevel[1]+=sizeof(S_IMAGE_RESOURCE_DIRECTORY_ENTRY);
                }

                nOffsetLevel[0]+=sizeof(S_IMAGE_RESOURCE_DIRECTORY_ENTRY);
            }
        }
    }

    return listResources;
}

QPE::RESOURCE_HEADER QPE::getResourceHeader(int nID1, int nID2, QList<QPE::RESOURCE_HEADER> *pListHeaders)
{
    RESOURCE_HEADER result= {0};
    result.nOffset=-1;

    for(int i=0; i<pListHeaders->count(); i++)
    {
        if((pListHeaders->at(i).nID[0]==nID1)&&(pListHeaders->at(i).nID[1]==nID2))
        {
            result=pListHeaders->at(i);
        }
    }

    return result;
}

QPE::RESOURCE_HEADER QPE::getResourceHeader(int nID1, QString sName2, QList<QPE::RESOURCE_HEADER> *pListHeaders)
{
    RESOURCE_HEADER result= {0};
    result.nOffset=-1;

    for(int i=0; i<pListHeaders->count(); i++)
    {
        if((pListHeaders->at(i).nID[0]==nID1)&&(pListHeaders->at(i).sName[1]==sName2))
        {
            result=pListHeaders->at(i);
        }
    }

    return result;
}

QPE::RESOURCE_HEADER QPE::getResourceHeader(QString sName1, int nID2, QList<QPE::RESOURCE_HEADER> *pListHeaders)
{
    RESOURCE_HEADER result= {0};
    result.nOffset=-1;

    for(int i=0; i<pListHeaders->count(); i++)
    {
        if((pListHeaders->at(i).sName[0]==sName1)&&(pListHeaders->at(i).nID[1]==nID2))
        {
            result=pListHeaders->at(i);
        }
    }

    return result;
}

QPE::RESOURCE_HEADER QPE::getResourceHeader(QString sName1, QString sName2, QList<QPE::RESOURCE_HEADER> *pListHeaders)
{
    RESOURCE_HEADER result= {0};
    result.nOffset=-1;

    for(int i=0; i<pListHeaders->count(); i++)
    {
        if((pListHeaders->at(i).sName[0]==sName1)&&(pListHeaders->at(i).sName[1]==sName2))
        {
            result=pListHeaders->at(i);
        }
    }

    return result;
}

bool QPE::isResourcePresent(int nID1, int nID2, QList<QPE::RESOURCE_HEADER> *pListHeaders)
{
    return (getResourceHeader(nID1,nID2,pListHeaders).nSize);
}

bool QPE::isResourcePresent(int nID1, QString sName2, QList<QPE::RESOURCE_HEADER> *pListHeaders)
{
    return (getResourceHeader(nID1,sName2,pListHeaders).nSize);
}

bool QPE::isResourcePresent(QString sName1, int nID2, QList<QPE::RESOURCE_HEADER> *pListHeaders)
{
    return (getResourceHeader(sName1,nID2,pListHeaders).nSize);
}

bool QPE::isResourcePresent(QString sName1, QString sName2, QList<QPE::RESOURCE_HEADER> *pListHeaders)
{
    return (getResourceHeader(sName1,sName2,pListHeaders).nSize);
}

S_IMAGE_IMPORT_DESCRIPTOR QPE::read_S_IMAGE_IMPORT_DESCRIPTOR(qint64 nOffset)
{
    S_IMAGE_IMPORT_DESCRIPTOR result= {0};

    read_array(nOffset,(char *)&result,sizeof(S_IMAGE_IMPORT_DESCRIPTOR));

    return result;
}

void QPE::write_S_IMAGE_IMPORT_DESCRIPTOR(qint64 nOffset, S_IMAGE_IMPORT_DESCRIPTOR value)
{
    write_array(nOffset,(char *)&value,sizeof(S_IMAGE_IMPORT_DESCRIPTOR));
}

bool QPE::isExportPresent()
{
    return isOptionalHeader_DataDirectoryPresent(S_IMAGE_DIRECTORY_ENTRY_EXPORT);
}

QPE::EXPORT_HEADER QPE::getExport()
{
    EXPORT_HEADER result= {0};

    qint64 nExportOffset=getDataDirectoryOffset(S_IMAGE_DIRECTORY_ENTRY_EXPORT);

    if(nExportOffset!=-1)
    {
        read_array(nExportOffset,(char *)&result.directory,sizeof(S_IMAGE_EXPORT_DIRECTORY));

        QList<MEMORY_MAP> listMemoryMap=getMemoryMapList();
        qint64 nBaseAddress=getBaseAddress();

        qint64 nNameOffset=addressToOffset(&listMemoryMap,result.directory.Name+nBaseAddress);

        if(nNameOffset!=-1)
        {
            result.sName=read_ansiString(nNameOffset);
        }

        qint64 nAddressOfFunctionsOffset=addressToOffset(&listMemoryMap,result.directory.AddressOfFunctions+nBaseAddress);
        qint64 nAddressOfNamesOffset=addressToOffset(&listMemoryMap,result.directory.AddressOfNames+nBaseAddress);
        qint64 nAddressOfNameOrdinalsOffset=addressToOffset(&listMemoryMap,result.directory.AddressOfNameOrdinals+nBaseAddress);

        if(result.directory.NumberOfFunctions<0xFFFF)
        {
            if((nAddressOfFunctionsOffset!=-1)&&(nAddressOfNamesOffset!=-1)&&(nAddressOfNameOrdinalsOffset!=-1))
            {
                for(int i=0; i<(int)result.directory.NumberOfFunctions; i++)
                {
                    EXPORT_POSITION position= {0};

                    int nIndex=read_uint16(nAddressOfNameOrdinalsOffset+2*i);
                    position.nOrdinal=nIndex+result.directory.Base;
                    position.nRVA=read_uint32(nAddressOfFunctionsOffset+4*nIndex);
                    quint32 nFunctionNameRVA=read_uint32(nAddressOfNamesOffset+4*nIndex);
                    qint64 nFunctionNameOffset=addressToOffset(&listMemoryMap,nFunctionNameRVA+nBaseAddress);

                    if(nFunctionNameOffset!=-1)
                    {
                        position.sFunctionName=read_ansiString(nFunctionNameOffset);
                    }

                    result.listPositions.append(position);
                }
            }
        }
    }

    return result;
}

bool QPE::isExportFunctionPresent(QString sFunction,QPE::EXPORT_HEADER *pExportHeader)
{
    bool bResult=false;

    for(int i=0; i<pExportHeader->listPositions.count(); i++)
    {
        if(pExportHeader->listPositions.at(i).sFunctionName==sFunction)
        {
            bResult=true;
            break;
        }
    }

    return bResult;
}

S_IMAGE_EXPORT_DIRECTORY QPE::getExportDirectory()
{
    S_IMAGE_EXPORT_DIRECTORY result= {0};

    qint64 nExportOffset=getDataDirectoryOffset(S_IMAGE_DIRECTORY_ENTRY_EXPORT);

    if(nExportOffset!=-1)
    {
        read_array(nExportOffset,(char *)&result,sizeof(S_IMAGE_EXPORT_DIRECTORY));
    }

    return result;
}

void QPE::setExportDirectory(S_IMAGE_EXPORT_DIRECTORY *pExportDirectory)
{
    qint64 nExportOffset=getDataDirectoryOffset(S_IMAGE_DIRECTORY_ENTRY_EXPORT);

    if(nExportOffset!=-1)
    {
        write_array(nExportOffset,(char *)pExportDirectory,sizeof(S_IMAGE_EXPORT_DIRECTORY));
    }
}

QByteArray QPE::getHeaders()
{
    QByteArray baResult;

    int nSizeOfHeaders=getOptionalHeader_SizeOfHeaders();

    if(isImage())
    {
        quint32 nSectionAlignment=getOptionalHeader_SectionAlignment();
        nSizeOfHeaders=__ALIGN_UP(nSizeOfHeaders,nSectionAlignment);
    }

    baResult=read_array(0,nSizeOfHeaders);

    if(baResult.size()!=nSizeOfHeaders)
    {
        baResult.resize(0);
    }

    return baResult;
}

QBinary::OFFSETSIZE QPE::__getSectionOffsetAndSize(quint32 nSection)
{
    OFFSETSIZE result= {0};

    S_IMAGE_SECTION_HEADER sectionHeader=getSectionHeader(nSection);
    quint32 nSectionAlignment=getOptionalHeader_SectionAlignment();
    quint32 nFileAlignment=getOptionalHeader_FileAlignment();

    if(nFileAlignment==nSectionAlignment)
    {
        nFileAlignment=1;
    }

    bool bIsSectionValid=false;

    if(!isImage())
    {
        bIsSectionValid=(bool)(sectionHeader.SizeOfRawData!=0);
    }
    else
    {
        bIsSectionValid=(bool)(sectionHeader.Misc.VirtualSize!=0);
    }

    if(bIsSectionValid)
    {
        qint64 nSectionOffset=0;
        qint64 nSectionSize=0;

        if(!isImage())
        {
            nSectionOffset=sectionHeader.PointerToRawData;
            nSectionOffset=__ALIGN_DOWN(nSectionOffset,nFileAlignment);
            nSectionSize=sectionHeader.SizeOfRawData;
        }
        else
        {
            nSectionOffset=sectionHeader.VirtualAddress;
            nSectionSize=sectionHeader.Misc.VirtualSize;
        }

        result=convertOffsetAndSize(nSectionOffset,nSectionSize);
    }
    else
    {
        result.nOffset=-1;
    }

    return result;
}

QByteArray QPE::getSection(quint32 nSection)
{
    QByteArray baResult;
    OFFSETSIZE offsize=__getSectionOffsetAndSize(nSection);

    if(offsize.nOffset!=-1)
    {
        baResult=read_array(offsize.nOffset,offsize.nSize);

        if(baResult.size()!=offsize.nSize) // TODO check???
        {
            baResult.resize(0);
        }
    }

    return baResult;
}

QString QPE::getSectionMD5(quint32 nSection)
{
    QString sResult;
    OFFSETSIZE offsize=__getSectionOffsetAndSize(nSection);

    if(offsize.nOffset!=-1)
    {
        sResult=getMD5(offsize.nOffset,offsize.nSize);
    }

    return sResult;
}

double QPE::getSectionEntropy(quint32 nSection)
{
    double dResult=0;
    OFFSETSIZE offsize=__getSectionOffsetAndSize(nSection);

    if(offsize.nOffset!=-1)
    {
        dResult=getEntropy(offsize.nOffset,offsize.nSize);
    }

    return dResult;
}

qint32 QPE::addressToSection(qint64 nAddress)
{
    QList<MEMORY_MAP> list=getMemoryMapList();

    return addressToSection(&list,nAddress);
}

qint32 QPE::addressToSection(QList<QBinary::MEMORY_MAP> *pMemoryMap, qint64 nAddress)
{
    MEMORY_MAP mm=getAddressMemoryMap(pMemoryMap,nAddress);

    if(mm.bIsSection)
    {
        return mm.nSection;
    }
    else
    {
        return -1;
    }
}

bool QPE::fixDumpFile(QString sFileName,DUMP_OPTIONS *pDumpOptions)
{
    bool bResult=false;
    QFile file;
    file.setFileName(sFileName);

    if(file.open(QIODevice::ReadWrite))
    {
        QPE pe(&file,true);

        if(pe.isValid())
        {
            pe.setOptionalHeader_AddressOfEntryPoint(pDumpOptions->nAddressOfEntryPoint-pDumpOptions->nImageBase);

            qint64 nCurrentOffset=0;
            quint32 nFileAlignment=pe.getOptionalHeader_FileAlignment();
            QByteArray baBuffer;
            baBuffer.resize(file.size());
            baBuffer.fill(0);
            QBuffer buffer;
            buffer.setBuffer(&baBuffer);

            if(buffer.open(QIODevice::ReadWrite))
            {

                QPE bufPE(&buffer);

                QByteArray baHeader=pe.getHeaders();
                quint32 nSizeOfHeaders=0;

                nSizeOfHeaders=QBinary::getPhysSize(baHeader.data(),baHeader.size());

                bufPE.write_array(nCurrentOffset,baHeader.data(),nSizeOfHeaders);

                nSizeOfHeaders=__ALIGN_UP(nSizeOfHeaders,nFileAlignment);
                nCurrentOffset+=nSizeOfHeaders;

                bufPE.setOptionalHeader_SizeOfHeaders(nSizeOfHeaders);

                int nNumberOfSections=pe.getFileHeader_NumberOfSections();

                for(int i=0; i<nNumberOfSections; i++)
                {
                    QByteArray baSection=pe.getSection(i);
                    quint32 nNewSectionSize=QBinary::getPhysSize(baSection.data(),baSection.size());

                    bufPE.write_array(nCurrentOffset,baSection.data(),nNewSectionSize);

                    nNewSectionSize=__ALIGN_UP(nNewSectionSize,nFileAlignment);

                    bufPE.setSection_PointerToRawData(i,nCurrentOffset);
                    bufPE.setSection_SizeOfRawData(i,nNewSectionSize);
                    bufPE.setSection_Characteristics(i,0xe0000020); // !!!
                    nCurrentOffset+=nNewSectionSize;
                }

                //                bufPE._fixCheckSum();

                // TODO init import

                buffer.close();

                bResult=true;
            }

            file.resize(0);
            file.write(baBuffer.data(),nCurrentOffset);
        }

        file.close();
    }

    return bResult;
}

bool QPE::addImportSection(QMap<qint64, QString> *pMapIAT)
{
    return addImportSection(getDevice(),pMapIAT);
}

bool QPE::addImportSection(QIODevice *pDevice, QMap<qint64, QString> *pMapIAT)
{
    bool bResult=false;

    QString sClassName=pDevice->metaObject()->className();

    if(sClassName=="QFile")
    {
        QPE pe(pDevice);

        if(pe.isValid())
        {
            QList<QPE::IMPORT_HEADER> list=mapIATToList(pMapIAT,pe.is64());

            bResult=setImports(pDevice,&list);
        }
    }

    return bResult;
}

bool QPE::addImportSection(QString sFileName, QMap<qint64, QString> *pMapIAT)
{
    bool bResult=false;
    QFile file(sFileName);

    if(file.open(QIODevice::ReadWrite))
    {
        bResult=addImportSection(&file,pMapIAT);

        file.close();
    }

    return bResult;
}

QList<QPE::IMPORT_HEADER> QPE::mapIATToList(QMap<qint64, QString> *pMapIAT,bool bIs64)
{
    QList<QPE::IMPORT_HEADER> listResult;

    QMapIterator<qint64, QString> i(*pMapIAT);

    IMPORT_HEADER record= {0};

    quint64 nCurrentRVA=0;

    quint32 nStep=0;

    if(bIs64)
    {
        nStep=8;
    }
    else
    {
        nStep=4;
    }

    while(i.hasNext())
    {
        i.next();

        QString sLibrary=i.value().section("#",0,0);
        QString sFunction=i.value().section("#",1,1);

        if(nCurrentRVA+nStep!=i.key())
        {
            if(record.sName!="")
            {
                listResult.append(record);
            }

            record.sName=sLibrary;
            record.nFirstThunk=i.key();
            record.listPositions.clear();
        }

        nCurrentRVA=i.key();

        IMPORT_POSITION position= {0};

        position.nHint=0;

        if(sFunction.toInt())
        {
            position.nOrdinal=sFunction.toInt();
        }
        else
        {
            position.sName=sFunction;
        }

        position.nThunkRVA=i.key();

        record.listPositions.append(position);

        if(!i.hasNext())
        {
            if(record.sName!="")
            {
                listResult.append(record);
            }
        }
    }

    return listResult;
}

quint32 QPE::calculateCheckSum()
{
    quint32 nCalcSum=_checkSum(0,getSize());
    quint32 nHdrSum=getOptionalHeader_CheckSum();

    if(__LOWORD(nCalcSum)>=__LOWORD(nHdrSum))
    {
        nCalcSum-=__LOWORD(nHdrSum);
    }
    else
    {
        nCalcSum=((__LOWORD(nCalcSum)-__LOWORD(nHdrSum))&0xFFFF)-1;
    }

    if(__LOWORD(nCalcSum)>=__HIWORD(nHdrSum)) //!!!!!
    {
        nCalcSum-=__HIWORD(nHdrSum);
    }
    else
    {
        nCalcSum=((__LOWORD(nCalcSum)-__HIWORD(nHdrSum))&0xFFFF)-1;
    }

    nCalcSum+=getSize();

    return nCalcSum;
}

qint64 QPE::getOverlaySize()
{
    qint64 nSize=getSize();
    qint64 nRawSize=_calculateRawSize();
    qint64 nDelta=nSize-nRawSize;

    return qMax(nDelta,(qint64)0);
}

qint64 QPE::getOverlayOffset()
{
    return _calculateRawSize();
}

bool QPE::isOverlayPresent()
{
    return getOverlaySize()!=0;
}

bool QPE::addOverlay(char *pData, qint64 nDataSize)
{
    return addOverlay(getDevice(),pData,nDataSize);
}

bool QPE::addOverlay(QString sFileName, char *pData, qint64 nDataSize)
{
    bool bResult=false;
    QFile file;
    file.setFileName(sFileName);

    if(file.open(QIODevice::ReadWrite))
    {
        bResult=addOverlay(&file,pData,nDataSize);

        file.close();
    }

    return bResult;
}

bool QPE::addOverlay(QIODevice *pDevice, char *pData, qint64 nDataSize)
{
    bool bResult=false;

    QString sClassName=pDevice->metaObject()->className();

    if(sClassName=="QFile")
    {
        QPE pe(pDevice);

        if(pe.isValid())
        {
            qint64 nRawSize=pe.getOverlayOffset();
            ((QFile *)pDevice)->resize(nRawSize+nDataSize);

            if(nDataSize)
            {
                pe.write_array(nRawSize,pData,nDataSize);
            }

            pe._fixCheckSum();
            bResult=true;
        }
    }

    return bResult;
}

bool QPE::addOverlayFromDevice(QIODevice *pSourceDevice, qint64 nOffset, qint64 nSize)
{
    return addOverlayFromDevice(getDevice(),pSourceDevice,nOffset,nSize);
}

bool QPE::addOverlayFromDevice(QIODevice *pDevice, QIODevice *pSourceDevice, qint64 nOffset, qint64 nSize)
{
    bool bResult=false;
    const int BUFFER_SIZE=0x1000;

    QString sClassName=pDevice->metaObject()->className();

    if(sClassName=="QFile")
    {
        QPE pe(pDevice);

        if(pe.isValid())
        {
            qint64 nRawSize=pe.getOverlayOffset();
            ((QFile *)pDevice)->resize(nRawSize+nSize);

            if(nSize)
            {
                char *pBuffer=new char[BUFFER_SIZE];

                qint64 nSourceOffset=nOffset;
                qint64 nDestOffset=pe.getOverlayOffset();

                bResult=true;

                while(nSize>0)
                {
                    qint64 nTemp=qMin((qint64)(BUFFER_SIZE),nSize);

                    pSourceDevice->seek(nOffset);

                    if(pSourceDevice->read(pBuffer,nTemp)<=0)
                    {
                        bResult=false;
                        break;
                    }

                    write_array(nDestOffset,pBuffer,nTemp);

                    nSourceOffset+=nTemp;
                    nDestOffset+=nTemp;
                    nSize-=nTemp;
                }

                delete [] pBuffer;
            }

            pe._fixCheckSum();

        }
    }

    return bResult;
}

bool QPE::removeOverlay()
{
    return removeOverlay(getDevice());
}

bool QPE::removeOverlay(QIODevice *pDevice)
{
    bool bResult=false;

    QString sClassName=pDevice->metaObject()->className();

    if(sClassName=="QFile")
    {
        QPE pe(pDevice);

        if(pe.isValid())
        {
            bResult=addOverlay(pDevice,0,0);
        }
    }

    return bResult;
}

bool QPE::addSection(QString sFileName, S_IMAGE_SECTION_HEADER *pSectionHeader, char *pData, qint64 nDataSize)
{
    bool bResult=false;
    QFile file(sFileName);

    if(file.open(QIODevice::ReadWrite))
    {
        bResult=addSection(&file,pSectionHeader,pData,nDataSize);

        file.close();
    }
    else
    {
        qDebug("Cannot open file"); // TODO emit
    }

    return bResult;
}

bool QPE::addSection(QIODevice *pDevice, S_IMAGE_SECTION_HEADER *pSectionHeader, char *pData, qint64 nDataSize)
{
    bool bResult=false;

    QString sClassName=pDevice->metaObject()->className();

    if(sClassName=="QFile")
    {
        QPE pe(pDevice);

        if(pe.isValid())
        {
            qint64 nHeadersSize=pe._fixHeadersSize();
            qint64 nNewHeadersSize=pe._calculateHeadersSize(pe.getSectionsTableOffset(),pe.getFileHeader_NumberOfSections()+1);
            quint32 nFileAlignment=pe.getOptionalHeader_FileAlignment();
            quint32 nSectionAlignment=pe.getOptionalHeader_SectionAlignment();

            if(pSectionHeader->PointerToRawData==0)
            {
                pSectionHeader->PointerToRawData=pe._calculateRawSize();
            }

            if(pSectionHeader->SizeOfRawData==0)
            {
                pSectionHeader->SizeOfRawData=__ALIGN_UP(nDataSize,nFileAlignment);
            }

            if(pSectionHeader->VirtualAddress==0)
            {
                pSectionHeader->VirtualAddress=__ALIGN_UP(pe.getOptionalHeader_SizeOfImage(),nSectionAlignment);
            }

            if(pSectionHeader->Misc.VirtualSize==0)
            {
                pSectionHeader->Misc.VirtualSize=__ALIGN_UP(nDataSize,nSectionAlignment);
            }

            qint64 nDelta=nNewHeadersSize-nHeadersSize;
            qint64 nFileSize=pDevice->size();

            if(nDelta>0)
            {
                ((QFile *)pDevice)->resize(nFileSize+nDelta);
                pe.moveMemory(nHeadersSize,nNewHeadersSize,nFileSize-nHeadersSize);
            }
            else if(nDelta<0)
            {
                pe.moveMemory(nHeadersSize,nNewHeadersSize,nFileSize-nHeadersSize);
                ((QFile *)pDevice)->resize(nFileSize+nDelta);
            }

            pe._fixFileOffsets(nDelta);

            pSectionHeader->PointerToRawData+=nDelta;
            nFileSize+=nDelta;

            ((QFile *)pDevice)->resize(nFileSize+pSectionHeader->SizeOfRawData);

            quint32 nNumberOfSections=pe.getFileHeader_NumberOfSections();
            pe.setFileHeader_NumberOfSections(nNumberOfSections+1);
            pe.setSectionHeader(nNumberOfSections,pSectionHeader);

            // Overlay
            if(pe.isOverlayPresent())
            {
                qint64 nOverlayOffset=pe.getOverlayOffset();
                qint64 nOverlaySize=pe.getOverlaySize();
                pe.moveMemory(nOverlayOffset-pSectionHeader->SizeOfRawData,nOverlayOffset,nOverlaySize);
            }

            pe.zeroFill(pSectionHeader->PointerToRawData,pSectionHeader->SizeOfRawData);

            pe.write_array(pSectionHeader->PointerToRawData,pData,nDataSize);

            qint64 nNewImageSize=__ALIGN_UP(pSectionHeader->VirtualAddress+pSectionHeader->Misc.VirtualSize,nSectionAlignment);
            pe.setOptionalHeader_SizeOfImage(nNewImageSize);


            pe._fixCheckSum();

            bResult=true;
        }
    }

    return bResult;
}

bool QPE::removeLastSection()
{
    return removeLastSection(getDevice());
}

bool QPE::removeLastSection(QIODevice *pDevice)
{
    bool bResult=false;

    QString sClassName=pDevice->metaObject()->className();

    if(sClassName=="QFile")
    {
        QPE pe(pDevice);

        if(pe.isValid())
        {
            int nNumberOfSections=pe.getFileHeader_NumberOfSections();

            if(nNumberOfSections)
            {
                qint64 nHeadersSize=pe._fixHeadersSize();
                qint64 nNewHeadersSize=pe._calculateHeadersSize(pe.getSectionsTableOffset(),nNumberOfSections-1);
                quint32 nFileAlignment=pe.getOptionalHeader_FileAlignment();
                quint32 nSectionAlignment=pe.getOptionalHeader_SectionAlignment();
                bool bIsOverlayPresent=pe.isOverlayPresent();
                qint64 nOverlayOffset=pe.getOverlayOffset();
                qint64 nOverlaySize=pe.getOverlaySize();

                S_IMAGE_SECTION_HEADER ish=pe.getSectionHeader(nNumberOfSections-1);
                S_IMAGE_SECTION_HEADER ish0= {0};
                pe.setSectionHeader(nNumberOfSections-1,&ish0);
                pe.setFileHeader_NumberOfSections(nNumberOfSections-1);


                ish.SizeOfRawData=__ALIGN_UP(ish.SizeOfRawData,nFileAlignment);
                ish.Misc.VirtualSize=__ALIGN_UP(ish.Misc.VirtualSize,nSectionAlignment);


                qint64 nDelta=nNewHeadersSize-nHeadersSize;
                qint64 nFileSize=pDevice->size();

                if(nDelta>0)
                {
                    ((QFile *)pDevice)->resize(nFileSize+nDelta);
                    pe.moveMemory(nHeadersSize,nNewHeadersSize,nFileSize-nHeadersSize);
                }
                else if(nDelta<0)
                {
                    pe.moveMemory(nHeadersSize,nNewHeadersSize,nFileSize-nHeadersSize);
                    ((QFile *)pDevice)->resize(nFileSize+nDelta);
                }

                pe._fixFileOffsets(nDelta);

                nFileSize+=nDelta;
                nOverlayOffset+=nDelta;

                if(bIsOverlayPresent)
                {
                    pe.moveMemory(nOverlayOffset,nOverlayOffset-ish.SizeOfRawData,nOverlaySize);
                }

                ((QFile *)pDevice)->resize(nFileSize-ish.SizeOfRawData);


                qint64 nNewImageSize=__ALIGN_UP(ish.VirtualAddress,nSectionAlignment);
                pe.setOptionalHeader_SizeOfImage(nNewImageSize);


                pe._fixCheckSum();

                bResult=true;
            }

        }
    }

    return bResult;
}

bool QPE::removeLastSection(QString sFileName)
{
    bool bResult=false;
    QFile file(sFileName);

    if(file.open(QIODevice::ReadWrite))
    {
        bResult=removeLastSection(&file);

        file.close();
    }

    return bResult;
}

bool QPE::removeOverlay(QString sFileName)
{
    return addOverlay(sFileName,0,0);
}

bool QPE::addSection(S_IMAGE_SECTION_HEADER *pSectionHeader, char *pData, qint64 nDataSize)
{
    return addSection(getDevice(),pSectionHeader,pData,nDataSize);
}

qint64 QPE::_calculateRawSize()
{
    qint64 nResult=0;
    QList<MEMORY_MAP> list=getMemoryMapList();

    for(int i=0; i<list.count(); i++)
    {
        if(list.at(i).bIsOvelay)
        {
            nResult=list.at(i).nOffset;
            break;
        }
    }

    return nResult;
}

void QPE::_fixCheckSum()
{
    setOptionalHeader_CheckSum(calculateCheckSum());
}

qint64 QPE::_calculateHeadersSize(qint64 nSectionsTableOffset, quint32 nNumberOfSections)
{
    qint64 nHeadersSize=nSectionsTableOffset+sizeof(S_IMAGE_SECTION_HEADER)*nNumberOfSections;
    quint32 nFileAlignment=getOptionalHeader_FileAlignment();
    nHeadersSize=__ALIGN_UP(nHeadersSize,nFileAlignment);

    return nHeadersSize;
}

bool QPE::isDll()
{
    if(getOptionalHeader_Subsystem()!=S_IMAGE_SUBSYSTEM_NATIVE)
    {
        return (getFileHeader_Characteristics()&S_IMAGE_FILE_DLL);
    }

    return false;
}

bool QPE::isNETPresent()
{
    return isOptionalHeader_DataDirectoryPresent(S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);
}

QPE::CLI_INFO QPE::getCliInfo()
{
    CLI_INFO result= {0};

    if(isNETPresent())
    {
        QList<MEMORY_MAP> listMM=getMemoryMapList();
        qint64 nBaseAddress=getBaseAddress();

        result.nEntryPointSize=0;


        S_IMAGE_DATA_DIRECTORY _idd=getOptionalHeader_DataDirectory(S_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);
        result.nCLIHeaderOffset=addressToOffset(&listMM,nBaseAddress+_idd.VirtualAddress);

        read_array(result.nCLIHeaderOffset,(char *)&(result.header),sizeof(S_IMAGE_COR20_HEADER));


        result.nEntryPoint=result.header.EntryPointRVA;

        if(result.header.MetaData.VirtualAddress&&result.header.MetaData.Size)
        {
            result.nCLI_MetaDataOffset=addressToOffset(&listMM,nBaseAddress+result.header.MetaData.VirtualAddress);

            if(result.nCLI_MetaDataOffset!=-1)
            {
                result.bInit=true;

                result.nCLI_MetaData_Signature=read_uint32(result.nCLI_MetaDataOffset);

                if(result.nCLI_MetaData_Signature==0x424a5342)
                {
                    result.sCLI_MetaData_MajorVersion=read_uint16(result.nCLI_MetaDataOffset+4);
                    result.sCLI_MetaData_MinorVersion=read_uint16(result.nCLI_MetaDataOffset+6);
                    result.nCLI_MetaData_Reserved=read_uint32(result.nCLI_MetaDataOffset+8);
                    result.nCLI_MetaData_VersionStringLength=read_uint32(result.nCLI_MetaDataOffset+12);

                    result.sCLI_MetaData_Version=read_ansiString(result.nCLI_MetaDataOffset+16,result.nCLI_MetaData_VersionStringLength);
                    result.sCLI_MetaData_Flags=read_uint16(result.nCLI_MetaDataOffset+16+result.nCLI_MetaData_VersionStringLength);
                    result.sCLI_MetaData_Streams=read_uint16(result.nCLI_MetaDataOffset+16+result.nCLI_MetaData_VersionStringLength+2);


                    qint64 nOffset=result.nCLI_MetaDataOffset+20+result.nCLI_MetaData_VersionStringLength;

                    for(int i=0; i<result.sCLI_MetaData_Streams; i++)
                    {
                        result.listCLI_MetaData_Stream_Offsets.append(read_uint32(nOffset));
                        result.listCLI_MetaData_Stream_Sizes.append(read_uint32(nOffset+4));
                        result.listCLI_MetaData_Stream_Names.append(read_ansiString(nOffset+8));

                        if(result.listCLI_MetaData_Stream_Names.at(i)=="#~")
                        {
                            result.nCLI_MetaData_TablesHeaderOffset=result.listCLI_MetaData_Stream_Offsets.at(i)+result.nCLI_MetaDataOffset;
                            result.nCLI_MetaData_TablesSize=result.listCLI_MetaData_Stream_Sizes.at(i);
                        }
                        else if(result.listCLI_MetaData_Stream_Names.at(i)=="#Strings")
                        {
                            result.nCLI_MetaData_StringsOffset=result.listCLI_MetaData_Stream_Offsets.at(i)+result.nCLI_MetaDataOffset;
                            result.nCLI_MetaData_StringsSize=result.listCLI_MetaData_Stream_Sizes.at(i);

                            QByteArray baStrings=read_array(result.nCLI_MetaData_StringsOffset,result.nCLI_MetaData_StringsSize);

                            char *_pOffset=baStrings.data();
                            int _nSize=baStrings.size();

                            for(int i=1; i<_nSize; i++)
                            {
                                _pOffset++;
                                QString sTemp=_pOffset;
                                result.listAnsiStrings.append(sTemp);

                                _pOffset+=sTemp.size();
                                i+=sTemp.size();
                            }
                        }
                        else if(result.listCLI_MetaData_Stream_Names.at(i)=="#US")
                        {
                            result.nCLI_MetaData_USOffset=result.listCLI_MetaData_Stream_Offsets.at(i)+result.nCLI_MetaDataOffset;
                            result.nCLI_MetaData_USSize=result.listCLI_MetaData_Stream_Sizes.at(i);

                            QByteArray baStrings=read_array(result.nCLI_MetaData_USOffset,result.nCLI_MetaData_USSize);

                            char *_pOffset=baStrings.data();
                            char *__pOffset=_pOffset;
                            int _nSize=baStrings.size();

                            __pOffset++;

                            for(int i=1; i<_nSize; i++)
                            {
                                int nStringSize=(*((unsigned char *)__pOffset));

                                if(nStringSize==0x80)
                                {
                                    nStringSize=0;
                                }

                                if(nStringSize>_nSize-i)
                                {
                                    break;
                                }

                                __pOffset++;

                                if(__pOffset>_pOffset+_nSize)
                                {
                                    break;
                                }

                                QString sTemp=QString::fromUtf16((ushort *)__pOffset,nStringSize/2);

                                result.listUnicodeStrings.append(sTemp);

                                __pOffset+=nStringSize;
                                i+=nStringSize;
                            }
                        }
                        else if(result.listCLI_MetaData_Stream_Names.at(i)=="#Blob")
                        {
                            result.nCLI_MetaData_BlobOffset=result.listCLI_MetaData_Stream_Offsets.at(i)+result.nCLI_MetaDataOffset;
                            result.nCLI_MetaData_BlobSize=result.listCLI_MetaData_Stream_Sizes.at(i);
                        }
                        else if(result.listCLI_MetaData_Stream_Names.at(i)=="#GUID")
                        {
                            result.nCLI_MetaData_GUIDOffset=result.listCLI_MetaData_Stream_Offsets.at(i)+result.nCLI_MetaDataOffset;
                            result.nCLI_MetaData_GUIDSize=result.listCLI_MetaData_Stream_Sizes.at(i);
                        }

                        nOffset+=8;
                        nOffset+=__ALIGN_UP(result.listCLI_MetaData_Stream_Names.at(i).length()+1,4);
                    }

                    if(result.nCLI_MetaData_TablesHeaderOffset)
                    {
                        result.nCLI_MetaData_Tables_Reserved1=read_uint32(result.nCLI_MetaData_TablesHeaderOffset);
                        result.cCLI_MetaData_Tables_MajorVersion=read_uint8(result.nCLI_MetaData_TablesHeaderOffset+4);
                        result.cCLI_MetaData_Tables_MinorVersion=read_uint8(result.nCLI_MetaData_TablesHeaderOffset+5);
                        result.cCLI_MetaData_Tables_HeapOffsetSizes=read_uint8(result.nCLI_MetaData_TablesHeaderOffset+6);
                        result.cCLI_MetaData_Tables_Reserved2=read_uint8(result.nCLI_MetaData_TablesHeaderOffset+7);
                        result.nCLI_MetaData_Tables_Valid=read_uint64(result.nCLI_MetaData_TablesHeaderOffset+8);
                        result.nCLI_MetaData_Tables_Sorted=read_uint64(result.nCLI_MetaData_TablesHeaderOffset+16);

                        unsigned long long nValid=result.nCLI_MetaData_Tables_Valid;

                        unsigned int nTemp=0;

                        for(nTemp = 0; nValid; nTemp++)
                        {
                            nValid &= nValid - 1;
                        }

                        result.nCLI_MetaData_Tables_Valid_NumberOfRows=nTemp;

                        nOffset=result.nCLI_MetaData_TablesHeaderOffset+24;

                        for(int i=0; i<64; i++)
                        {
                            if(result.nCLI_MetaData_Tables_Valid&((unsigned long long)1<<i))
                            {
                                result.CLI_MetaData_Tables_TablesNumberOfIndexes[i]=read_uint32(nOffset);
                                nOffset+=4;
                            }
                        }

                        unsigned int nSize=0;
                        int nStringIndexSize=2;
                        int nGUIDIndexSize=2;
                        int nBLOBIndexSize=2;
                        int nResolutionScope=2;
                        int nTypeDefOrRef=2;
                        int nField=2;
                        int nMethodDef=2;
                        int nParamList=2;

                        unsigned char cHeapOffsetSizes=0;

                        cHeapOffsetSizes=result.cCLI_MetaData_Tables_HeapOffsetSizes;

                        if(cHeapOffsetSizes&0x01)
                        {
                            nStringIndexSize=4;
                        }

                        if(cHeapOffsetSizes&0x02)
                        {
                            nGUIDIndexSize=4;
                        }

                        if(cHeapOffsetSizes&0x04)
                        {
                            nBLOBIndexSize=4;
                        }

                        if(result.CLI_MetaData_Tables_TablesNumberOfIndexes[0]>0x3FFF)
                        {
                            nResolutionScope=4;
                        }

                        if(result.CLI_MetaData_Tables_TablesNumberOfIndexes[26]>0x3FFF)
                        {
                            nResolutionScope=4;
                        }

                        if(result.CLI_MetaData_Tables_TablesNumberOfIndexes[35]>0x3FFF)
                        {
                            nResolutionScope=4;
                        }

                        if(result.CLI_MetaData_Tables_TablesNumberOfIndexes[1]>0x3FFF)
                        {
                            nResolutionScope=4;
                        }

                        if(result.CLI_MetaData_Tables_TablesNumberOfIndexes[1]>0x3FFF)
                        {
                            nTypeDefOrRef=4;
                        }

                        if(result.CLI_MetaData_Tables_TablesNumberOfIndexes[2]>0x3FFF)
                        {
                            nTypeDefOrRef=4;
                        }

                        if(result.CLI_MetaData_Tables_TablesNumberOfIndexes[27]>0x3FFF)
                        {
                            nTypeDefOrRef=4;
                        }


                        if(result.CLI_MetaData_Tables_TablesNumberOfIndexes[4]>0xFFFF)
                        {
                            nField=4;
                        }

                        if(result.CLI_MetaData_Tables_TablesNumberOfIndexes[6]>0xFFFF)
                        {
                            nMethodDef=4;
                        }

                        if(result.CLI_MetaData_Tables_TablesNumberOfIndexes[8]>0xFFFF)
                        {
                            nParamList=4;
                        }

                        nSize=0;
                        nSize+=2;
                        nSize+=nStringIndexSize;
                        nSize+=nGUIDIndexSize;
                        nSize+=nGUIDIndexSize;
                        nSize+=nGUIDIndexSize;
                        result.CLI_MetaData_Tables_TablesSizes[0]=nSize;
                        nSize=0;
                        nSize+=nResolutionScope;
                        nSize+=nStringIndexSize;
                        nSize+=nStringIndexSize;
                        result.CLI_MetaData_Tables_TablesSizes[1]=nSize;
                        nSize=0;
                        nSize+=4;
                        nSize+=nStringIndexSize;
                        nSize+=nStringIndexSize;
                        nSize+=nTypeDefOrRef;
                        nSize+=nField;
                        nSize+=nMethodDef;
                        result.CLI_MetaData_Tables_TablesSizes[2]=nSize;
                        nSize=0;
                        result.CLI_MetaData_Tables_TablesSizes[3]=nSize;
                        nSize=0;
                        nSize+=2;
                        nSize+=nStringIndexSize;
                        nSize+=nBLOBIndexSize;
                        result.CLI_MetaData_Tables_TablesSizes[4]=nSize;
                        nSize=0;
                        result.CLI_MetaData_Tables_TablesSizes[5]=nSize;
                        nSize=0;
                        nSize+=4;
                        nSize+=2;
                        nSize+=2;
                        nSize+=nStringIndexSize;
                        nSize+=nBLOBIndexSize;
                        nSize+=nParamList;
                        result.CLI_MetaData_Tables_TablesSizes[6]=nSize;


                        for(int i=0; i<64; i++)
                        {
                            if(result.CLI_MetaData_Tables_TablesNumberOfIndexes[i])
                            {
                                result.CLI_MetaData_Tables_TablesOffsets[i]=nOffset;
                                nOffset+=result.CLI_MetaData_Tables_TablesSizes[i]*result.CLI_MetaData_Tables_TablesNumberOfIndexes[i];
                            }
                        }

                        if(!(result.header.Flags&S_COMIMAGE_FLAGS_NATIVE_ENTRYPOINT))
                        {
                            if(((result.nEntryPoint&0xFF000000)>>24)==6)
                            {
                                unsigned int nIndex=result.nEntryPoint&0xFFFFFF;

                                if(nIndex<=result.CLI_MetaData_Tables_TablesNumberOfIndexes[6])
                                {
                                    nOffset=result.CLI_MetaData_Tables_TablesOffsets[6];
                                    nOffset+=result.CLI_MetaData_Tables_TablesSizes[6]*(nIndex-1);

                                    result.nEntryPoint=read_uint32(nOffset);
                                }
                                else
                                {
                                    result.nEntryPoint=0;
                                }
                            }
                            else
                            {
                                result.nEntryPoint=0;
                            }
                        }
                    }
                }

            }


        }
    }

    //    emit appendError(".NET is not present");

    return result;
}

bool QPE::isNETAnsiStringPresent(QString sString, QPE::CLI_INFO *pCliInfo)
{
    bool bResult=false;

    for(int i=0; i<pCliInfo->listAnsiStrings.count(); i++)
    {
        QString sRecord=pCliInfo->listAnsiStrings.at(i);

        if(sRecord!="")
        {
            if(sString==sRecord)
            {
                bResult=true;
                break;
            }
        }
    }

    return bResult;
}

int QPE::getEntryPointSection()
{
    qint64 nAddressOfEntryPoint=getOptionalHeader_AddressOfEntryPoint();

    if(nAddressOfEntryPoint)
    {
        return addressToSection(getBaseAddress()+nAddressOfEntryPoint);
    }

    return -1;
}

int QPE::getImportSection()
{
    qint64 nAddressOfImport=getOptionalHeader_DataDirectory(S_IMAGE_DIRECTORY_ENTRY_IMPORT).VirtualAddress;

    if(nAddressOfImport)
    {
        return addressToSection(getBaseAddress()+nAddressOfImport);
    }

    return -1;
}

int QPE::getExportSection()
{
    qint64 nAddressOfExport=getOptionalHeader_DataDirectory(S_IMAGE_DIRECTORY_ENTRY_EXPORT).VirtualAddress;

    if(nAddressOfExport)
    {
        return addressToSection(getBaseAddress()+nAddressOfExport);
    }

    return -1;
}

int QPE::getTLSSection()
{
    qint64 nAddressOfTLS=getOptionalHeader_DataDirectory(S_IMAGE_DIRECTORY_ENTRY_TLS).VirtualAddress;

    if(nAddressOfTLS)
    {
        return addressToSection(getBaseAddress()+nAddressOfTLS);
    }

    return -1;
}

int QPE::getResourcesSection()
{
    qint64 nAddressOfResources=getOptionalHeader_DataDirectory(S_IMAGE_DIRECTORY_ENTRY_RESOURCE).VirtualAddress;

    if(nAddressOfResources)
    {
        return addressToSection(getBaseAddress()+nAddressOfResources);
    }

    return -1;
}

int QPE::getRelocsSection()
{
    qint64 nAddressOfRelocs=getOptionalHeader_DataDirectory(S_IMAGE_DIRECTORY_ENTRY_BASERELOC).VirtualAddress;

    if(nAddressOfRelocs)
    {
        return addressToSection(getBaseAddress()+nAddressOfRelocs);
    }

    return -1;
}

int QPE::getNormalCodeSection()
{
    int nResult=-1;
    // TODO opimize

    QList<S_IMAGE_SECTION_HEADER> listSections=getSectionHeaders();
    int nNumberOfSections=listSections.count();
    nNumberOfSections=qMin(nNumberOfSections,2);


    for(int i=0; i<nNumberOfSections; i++)
    {
        QString sSectionName=QString((char *)listSections.at(i).Name);
        sSectionName.resize(qMin(sSectionName.length(),S_IMAGE_SIZEOF_SHORT_NAME));
        quint32 nSectionCharacteristics=listSections.at(i).Characteristics;
        nSectionCharacteristics&=0xFF0000FF;

        // .textbss
        // 0x60500060 mingw
        if((((sSectionName=="CODE")||sSectionName==".text"))&&
                (nSectionCharacteristics==0x60000020)&&
                (listSections.at(i).SizeOfRawData))
        {
            nResult=addressToSection(getBaseAddress()+listSections.at(i).VirtualAddress);
            break;
        }
    }

    if(nResult==-1)
    {
        if(nNumberOfSections>0)
        {
            if(listSections.at(0).SizeOfRawData)
            {
                nResult=addressToSection(getBaseAddress()+listSections.at(0).VirtualAddress);
            }
        }
    }

    return nResult;
}

int QPE::getNormalDataSection()
{
    int nResult=-1;
    // TODO opimize

    QList<S_IMAGE_SECTION_HEADER> listSections=getSectionHeaders();
    int nNumberOfSections=listSections.count();


    int nImportSection=getImportSection();

    for(int i=1; i<nNumberOfSections; i++)
    {
        // 0xc0700040 MinGW
        // 0xc0600040 MinGW
        // 0xc0300040 MinGW
        QString sSectionName=QString((char *)listSections.at(i).Name);
        sSectionName.resize(qMin(sSectionName.length(),S_IMAGE_SIZEOF_SHORT_NAME));
        quint32 nSectionCharacteristics=listSections.at(i).Characteristics;
        nSectionCharacteristics&=0xFF0000FF;

        if((((sSectionName=="DATA")||sSectionName==".data"))&&
                (nSectionCharacteristics==0xC0000040)&&
                (listSections.at(i).SizeOfRawData)&&
                (nImportSection!=i))
        {
            nResult=addressToSection(getBaseAddress()+listSections.at(i).VirtualAddress);
            break;
        }
    }


    if(nResult==-1)
    {
        for(int i=1; i<nNumberOfSections; i++)
        {
            if(listSections.at(i).SizeOfRawData&&(nImportSection!=i)&&
                    (listSections.at(i).Characteristics!=0x60000020)&&
                    (listSections.at(i).Characteristics!=0x40000040))
            {
                nResult=addressToSection(getBaseAddress()+listSections.at(i).VirtualAddress);
                break;
            }
        }
    }


    return nResult;
}

int QPE::getConstDataSection()
{
    int nResult=-1;
    // TODO opimize

    QList<S_IMAGE_SECTION_HEADER> listSections=getSectionHeaders();
    int nNumberOfSections=listSections.count();

    for(int i=1; i<nNumberOfSections; i++)
    {
        // 0x40700040 MinGW
        // 0x40600040 MinGW
        // 0x40300040 MinGW
        QString sSectionName=QString((char *)listSections.at(i).Name);
        sSectionName.resize(qMin(sSectionName.length(),S_IMAGE_SIZEOF_SHORT_NAME));
        quint32 nSectionCharacteristics=listSections.at(i).Characteristics;
        nSectionCharacteristics&=0xFF0000FF;

        if((sSectionName==".rdata")&&
                (nSectionCharacteristics==0x40000040)&&
                (listSections.at(i).SizeOfRawData))
        {
            nResult=addressToSection(getBaseAddress()+listSections.at(i).VirtualAddress);
            break;
        }
    }

    return nResult;
}

qint64 QPE::_fixHeadersSize()
{
    quint32 nNumberOfSections=getFileHeader_NumberOfSections();
    qint64 nSectionsTableOffset=getSectionsTableOffset();
    qint64 nHeadersSize=_calculateHeadersSize(nSectionsTableOffset,nNumberOfSections);

    // MB TODO
    setOptionalHeader_SizeOfHeaders(nHeadersSize);

    return nHeadersSize;
}

qint64 QPE::_getMinSectionOffset()
{
    qint64 nResult=-1;

    QList<MEMORY_MAP> listMM=getMemoryMapList();

    for(int i=0; i<listMM.count(); i++)
    {
        if(listMM.at(i).bIsSection)
        {
            if(nResult==-1)
            {
                nResult=listMM.at(i).nOffset;
            }
            else
            {
                nResult=qMin(nResult,listMM.at(i).nOffset);
            }
        }
    }

    return nResult;
}

void QPE::_fixFileOffsets(qint64 nDelta)
{
    if(nDelta)
    {
        setOptionalHeader_SizeOfHeaders(getOptionalHeader_SizeOfHeaders()+nDelta);
        quint32 nNumberOfSections=getFileHeader_NumberOfSections();

        for(quint32 i=0; i<nNumberOfSections; i++)
        {
            quint32 nFileOffset=getSection_PointerToRawData(i);
            setSection_PointerToRawData(i,nFileOffset+nDelta);
        }
    }
}

quint16 QPE::_checkSum(qint64 nStartValue,qint64 nDataSize)
{
    const int BUFFER_SIZE=0x1000;
    int nSum=(int)nStartValue;
    unsigned int nTemp=0;
    char *pBuffer=new char[BUFFER_SIZE];
    char *pOffset;

    while(nDataSize>0)
    {
        nTemp=qMin((qint64)BUFFER_SIZE,nDataSize);

        if(!read_array(nStartValue,pBuffer,nTemp))
        {
            delete[] pBuffer;

            return false;
        }

        pOffset=pBuffer;

        for(unsigned int i=0; i<(nTemp+1)/2; i++)
        {
            nSum+=*((unsigned short *)pOffset);

            if(__HIWORD(nSum)!=0)
            {
                nSum=__LOWORD(nSum)+__HIWORD(nSum);
            }

            pOffset+=2;
        }

        nDataSize-=nTemp;
        nStartValue+=nTemp;
    }

    delete[] pBuffer;

    return (unsigned short)(__LOWORD(nSum)+__HIWORD(nSum));
}

S_IMAGE_RESOURCE_DIRECTORY_ENTRY QPE::read_S_IMAGE_RESOURCE_DIRECTORY_ENTRY(qint64 nOffset)
{
    S_IMAGE_RESOURCE_DIRECTORY_ENTRY result= {0};

    read_array(nOffset,(char *)&result,sizeof(S_IMAGE_RESOURCE_DIRECTORY_ENTRY));

    return result;
}

S_IMAGE_RESOURCE_DIRECTORY QPE::read_S_IMAGE_RESOURCE_DIRECTORY(qint64 nOffset)
{
    S_IMAGE_RESOURCE_DIRECTORY result= {0};

    read_array(nOffset,(char *)&result,sizeof(S_IMAGE_RESOURCE_DIRECTORY));

    return result;
}

S_IMAGE_RESOURCE_DATA_ENTRY QPE::read_S_IMAGE_RESOURCE_DATA_ENTRY(qint64 nOffset)
{
    S_IMAGE_RESOURCE_DATA_ENTRY result= {0};

    read_array(nOffset,(char *)&result,sizeof(S_IMAGE_RESOURCE_DATA_ENTRY));

    return result;
}

QPE::RESOURCES_ID_NAME QPE::getResourcesIDName(qint64 nResourceOffset,quint32 value)
{
    RESOURCES_ID_NAME result= {0};

    if(value&0x80000000)
    {
        value&=0x7FFFFFFF;
        result.nNameOffset=value;
        result.nID=0;
        int nStringLength=read_uint16(nResourceOffset+value);
        QByteArray baName=read_array(nResourceOffset+value+2,nStringLength*2);
        result.sName=QString::fromUtf16((quint16 *)(baName.data()),nStringLength);
    }
    else
    {
        result.nID=value;
        result.sName="";
        result.nNameOffset=0;
    }

    return result;
}

QList<qint64> QPE::getRelocsAsRVAList()
{
    QList<qint64> listResult;

    // TODO 64
    qint64 nRelocsOffset=getDataDirectoryOffset(S_IMAGE_DIRECTORY_ENTRY_BASERELOC);

    if(nRelocsOffset!=-1)
    {
        while(true)
        {
            _S_IMAGE_BASE_RELOCATION ibr= {0};

            if(!read_array(nRelocsOffset,(char *)&ibr,sizeof(_S_IMAGE_BASE_RELOCATION)))
            {
                break;
            }

            if(ibr.VirtualAddress==0)
            {
                break;
            }

            nRelocsOffset+=sizeof(_S_IMAGE_BASE_RELOCATION);

            int nCount=(ibr.SizeOfBlock-sizeof(_S_IMAGE_BASE_RELOCATION))/sizeof(quint16);

            for(int i=0; i<nCount; i++)
            {
                quint16 nRecord=read_uint16(nRelocsOffset);

                if(nRecord)
                {
                    nRecord=nRecord&0x0FFF;
                    listResult.append(ibr.VirtualAddress+nRecord);
                }

                nRelocsOffset+=sizeof(quint16);
            }
        }
    }

    return listResult;
}

bool QPE::addRelocsSection(QList<qint64> *pList)
{
    return addRelocsSection(getDevice(),pList);
}

bool QPE::addRelocsSection(QIODevice *pDevice, QList<qint64> *pList)
{
    bool bResult=false;

    QString sClassName=pDevice->metaObject()->className();

    if(sClassName=="QFile")
    {
        QPE pe(pDevice);

        if(pe.isValid())
        {
            // Check valid
            QList<MEMORY_MAP> listMM=pe.getMemoryMapList();
            qint64 nBaseAddress=pe.getBaseAddress();
            QList<qint64> listRVAs;

            for(int i=0; i<pList->count(); i++)
            {
                if(pe.isAddressValid(&listMM,pList->at(i)+nBaseAddress))
                {
                    listRVAs.append(pList->at(i));
                }
            }

            QByteArray baRelocs=QPE::relocsAsRVAListToByteArray(&listRVAs,pe.is64());

            S_IMAGE_SECTION_HEADER ish= {0};

            ish.Characteristics=0x42000040;
            QString sSectionName=".reloc";
            QBinary::_copyMemory((char *)&ish.Name,sSectionName.toLatin1().data(),qMin(S_IMAGE_SIZEOF_SHORT_NAME,sSectionName.length()));


            bResult=addSection(pDevice,&ish,baRelocs.data(),baRelocs.size());

            if(bResult)
            {
                S_IMAGE_DATA_DIRECTORY dd= {0};

                dd.VirtualAddress=ish.VirtualAddress;
                dd.Size=ish.Misc.VirtualSize;

                pe.setOptionalHeader_DataDirectory(S_IMAGE_DIRECTORY_ENTRY_BASERELOC,&dd);

                bResult=true;
            }
        }
    }

    return bResult;
}

bool QPE::addRelocsSection(QString sFileName, QList<qint64> *pList)
{
    bool bResult=false;
    QFile file(sFileName);

    if(file.open(QIODevice::ReadWrite))
    {
        bResult=addRelocsSection(&file,pList);

        file.close();
    }

    return bResult;
}

QByteArray QPE::relocsAsRVAListToByteArray(QList<qint64> *pList, bool bIs64)
{
    QByteArray baResult;
    // GetHeaders

    // pList must be sorted!

    qint32 nBaseAddress=-1;
    quint32 nSize=0;

    for(int i=0; i<pList->count(); i++)
    {
        qint32 _nBaseAddress=__ALIGN_DOWN(pList->at(i),0x1000);

        if(nBaseAddress!=_nBaseAddress)
        {
            nBaseAddress=_nBaseAddress;
            nSize=__ALIGN_UP(nSize,4);
            nSize+=sizeof(_S_IMAGE_BASE_RELOCATION);
        }

        nSize+=2;
    }

    nSize=__ALIGN_UP(nSize,4);

    baResult.resize(nSize);

    nBaseAddress=-1;
    quint32 nOffset=0;
    char *pData=baResult.data();
    char *pVirtualAddress;
    char *pSizeOfBlock;
    quint32 nCurrentBlockSize=0;

    for(int i=0; i<pList->count(); i++)
    {
        qint32 _nBaseAddress=__ALIGN_DOWN(pList->at(i),0x1000);

        if(nBaseAddress!=_nBaseAddress)
        {
            nBaseAddress=_nBaseAddress;
            quint32 _nOffset=__ALIGN_UP(nOffset,4);

            if(nOffset!=_nOffset)
            {
                nCurrentBlockSize+=2;
                QBinary::_write_uint32(pSizeOfBlock,nCurrentBlockSize);
                QBinary::_write_uint16(pData+nOffset,0);
                nOffset=_nOffset;
            }

            pVirtualAddress=pData+nOffset+offsetof(_S_IMAGE_BASE_RELOCATION,VirtualAddress);
            pSizeOfBlock=pData+nOffset+offsetof(_S_IMAGE_BASE_RELOCATION,SizeOfBlock);
            QBinary::_write_uint32(pVirtualAddress,nBaseAddress);
            nCurrentBlockSize=sizeof(_S_IMAGE_BASE_RELOCATION);
            QBinary::_write_uint32(pSizeOfBlock,nCurrentBlockSize);

            nOffset+=sizeof(_S_IMAGE_BASE_RELOCATION);
        }

        nCurrentBlockSize+=2;
        QBinary::_write_uint32(pSizeOfBlock,nCurrentBlockSize);

        if(!bIs64)
        {
            QBinary::_write_uint16(pData+nOffset,pList->at(i)-nBaseAddress+0x3000);
        }
        else
        {
            QBinary::_write_uint16(pData+nOffset,pList->at(i)-nBaseAddress+0xA000);
        }

        nOffset+=2;
    }

    quint32 _nOffset=__ALIGN_UP(nOffset,4);

    if(nOffset!=_nOffset)
    {
        nCurrentBlockSize+=2;
        QBinary::_write_uint32(pSizeOfBlock,nCurrentBlockSize);
        QBinary::_write_uint16(pData+nOffset,0);
    }

    return baResult;
}

bool QPE::isResourcesPresent()
{
    return isOptionalHeader_DataDirectoryPresent(S_IMAGE_DIRECTORY_ENTRY_RESOURCE);
}

bool QPE::isRelocsPresent()
{
    return isOptionalHeader_DataDirectoryPresent(S_IMAGE_DIRECTORY_ENTRY_BASERELOC);
}

bool QPE::isTLSPresent()
{
    return isOptionalHeader_DataDirectoryPresent(S_IMAGE_DIRECTORY_ENTRY_TLS);
}

QMap<quint64, QString> QPE::getImageFileMachines()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0,"IMAGE_FILE_MACHINE_UNKNOWN");
    mapResult.insert(0x014c,"IMAGE_FILE_MACHINE_I386");
    mapResult.insert(0x0162,"IMAGE_FILE_MACHINE_R3000");
    mapResult.insert(0x0166,"IMAGE_FILE_MACHINE_R4000");
    mapResult.insert(0x0168,"IMAGE_FILE_MACHINE_R10000");
    mapResult.insert(0x0169,"IMAGE_FILE_MACHINE_WCEMIPSV2");
    mapResult.insert(0x0184,"IMAGE_FILE_MACHINE_ALPHA");
    mapResult.insert(0x01a2,"IMAGE_FILE_MACHINE_SH3");
    mapResult.insert(0x01a3,"IMAGE_FILE_MACHINE_SH3DSP");
    mapResult.insert(0x01a4,"IMAGE_FILE_MACHINE_SH3E");
    mapResult.insert(0x01a6,"IMAGE_FILE_MACHINE_SH4");
    mapResult.insert(0x01a8,"IMAGE_FILE_MACHINE_SH5");
    mapResult.insert(0x01c0,"IMAGE_FILE_MACHINE_ARM");
    mapResult.insert(0x01c2,"IMAGE_FILE_MACHINE_THUMB");
    mapResult.insert(0x01c4,"IMAGE_FILE_MACHINE_ARMNT");
    mapResult.insert(0x01d3,"IMAGE_FILE_MACHINE_AM33");
    mapResult.insert(0x01F0,"IMAGE_FILE_MACHINE_POWERPC");
    mapResult.insert(0x01f1,"IMAGE_FILE_MACHINE_POWERPCFP");
    mapResult.insert(0x0200,"IMAGE_FILE_MACHINE_IA64");
    mapResult.insert(0x0266,"IMAGE_FILE_MACHINE_MIPS16");
    mapResult.insert(0x0284,"IMAGE_FILE_MACHINE_ALPHA64");
    mapResult.insert(0x0366,"IMAGE_FILE_MACHINE_MIPSFPU");
    mapResult.insert(0x0466,"IMAGE_FILE_MACHINE_MIPSFPU16");
    mapResult.insert(0x0520,"IMAGE_FILE_MACHINE_TRICORE");
    mapResult.insert(0x0CEF,"IMAGE_FILE_MACHINE_CEF");
    mapResult.insert(0x0EBC,"IMAGE_FILE_MACHINE_EBC");
    mapResult.insert(0x8664,"IMAGE_FILE_MACHINE_AMD64");
    mapResult.insert(0x9041,"IMAGE_FILE_MACHINE_M32R");
    mapResult.insert(0xC0EE,"IMAGE_FILE_MACHINE_CEE");

    return mapResult;
}

QMap<quint64, QString> QPE::getImageFileCharacteristics()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0x0001,"IMAGE_FILE_RELOCS_STRIPPED");
    mapResult.insert(0x0002,"IMAGE_FILE_EXECUTABLE_IMAGE");
    mapResult.insert(0x0004,"IMAGE_FILE_LINE_NUMS_STRIPPED");
    mapResult.insert(0x0008,"IMAGE_FILE_LOCAL_SYMS_STRIPPED");
    mapResult.insert(0x0010,"IMAGE_FILE_AGGRESIVE_WS_TRIM");
    mapResult.insert(0x0020,"IMAGE_FILE_LARGE_ADDRESS_AWARE");
    mapResult.insert(0x0080,"IMAGE_FILE_BYTES_REVERSED_LO");
    mapResult.insert(0x0100,"IMAGE_FILE_32BIT_MACHINE");
    mapResult.insert(0x0200,"IMAGE_FILE_DEBUG_STRIPPED");
    mapResult.insert(0x0400,"IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP");
    mapResult.insert(0x0800,"IMAGE_FILE_NET_RUN_FROM_SWAP");
    mapResult.insert(0x1000,"IMAGE_FILE_SYSTEM");
    mapResult.insert(0x2000,"IMAGE_FILE_DLL");
    mapResult.insert(0x4000,"IMAGE_FILE_UP_SYSTEM_ONLY");
    mapResult.insert(0x8000,"IMAGE_FILE_BYTES_REVERSED_HI");

    return mapResult;
}

QMap<quint64, QString> QPE::getImageNtOptionalMagic()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0x10b,"IMAGE_NT_OPTIONAL_HDR32_MAGIC");
    mapResult.insert(0x20b,"IMAGE_NT_OPTIONAL_HDR64_MAGIC");
    mapResult.insert(0x107,"IMAGE_ROM_OPTIONAL_HDR_MAGIC");

    return mapResult;
}

QMap<quint64, QString> QPE::getImageNtOptionalSubsystem()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0,"IMAGE_SUBSYSTEM_UNKNOWN");
    mapResult.insert(1,"IMAGE_SUBSYSTEM_NATIVE");
    mapResult.insert(2,"IMAGE_SUBSYSTEM_WINDOWS_GUI");
    mapResult.insert(3,"IMAGE_SUBSYSTEM_WINDOWS_CUI");
    mapResult.insert(5,"IMAGE_SUBSYSTEM_OS2_CUI");
    mapResult.insert(7,"IMAGE_SUBSYSTEM_POSIX_CUI");
    mapResult.insert(8,"IMAGE_SUBSYSTEM_NATIVE_WINDOWS");
    mapResult.insert(9,"IMAGE_SUBSYSTEM_WINDOWS_CE_GUI");
    mapResult.insert(10,"IMAGE_SUBSYSTEM_EFI_APPLICATION");
    mapResult.insert(11,"IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER");
    mapResult.insert(12,"IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER");
    mapResult.insert(13,"IMAGE_SUBSYSTEM_EFI_ROM");
    mapResult.insert(14,"IMAGE_SUBSYSTEM_XBOX");
    mapResult.insert(16,"IMAGE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION");

    return mapResult;
}

QMap<quint64, QString> QPE::getImageNtOptionalDllCharacteristics()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0x0020,"IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA");
    mapResult.insert(0x0040,"IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE");
    mapResult.insert(0x0080,"IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY");
    mapResult.insert(0x0100,"IMAGE_DLLCHARACTERISTICS_NX_COMPAT");
    mapResult.insert(0x0200,"IMAGE_DLLCHARACTERISTICS_NO_ISOLATION");
    mapResult.insert(0x0400,"IMAGE_DLLCHARACTERISTICS_NO_SEH");
    mapResult.insert(0x0800,"IMAGE_DLLCHARACTERISTICS_NO_BIND");
    mapResult.insert(0x1000,"IMAGE_DLLCHARACTERISTICS_APPCONTAINER");
    mapResult.insert(0x2000,"IMAGE_DLLCHARACTERISTICS_WDM_DRIVER");
    mapResult.insert(0x4000,"IMAGE_DLLCHARACTERISTICS_GUARD_CF");
    mapResult.insert(0x8000,"IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE");

    return mapResult;
}

QMap<quint64, QString> QPE::getImageSectionFlags()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0x00000008,"IMAGE_SCN_TYPE_NO_PAD");
    mapResult.insert(0x00000020,"IMAGE_SCN_CNT_CODE");
    mapResult.insert(0x00000040,"IMAGE_SCN_CNT_INITIALIZED_DATA");
    mapResult.insert(0x00000080,"IMAGE_SCN_CNT_UNINITIALIZED_DATA");
    mapResult.insert(0x00000100,"IMAGE_SCN_LNK_OTHER");
    mapResult.insert(0x00000200,"IMAGE_SCN_LNK_INFO");
    mapResult.insert(0x00000800,"IMAGE_SCN_LNK_REMOVE");
    mapResult.insert(0x00001000,"IMAGE_SCN_LNK_COMDAT");
    mapResult.insert(0x00004000,"IMAGE_SCN_NO_DEFER_SPEC_EXC");
    mapResult.insert(0x00008000,"IMAGE_SCN_GPREL");
    mapResult.insert(0x00020000,"IMAGE_SCN_MEM_PURGEABLE");
    mapResult.insert(0x00020000,"IMAGE_SCN_MEM_16BIT");
    mapResult.insert(0x00040000,"IMAGE_SCN_MEM_LOCKED");
    mapResult.insert(0x00080000,"IMAGE_SCN_MEM_PRELOAD");
    mapResult.insert(0x01000000,"IMAGE_SCN_LNK_NRELOC_OVFL");
    mapResult.insert(0x02000000,"IMAGE_SCN_MEM_DISCARDABLE");
    mapResult.insert(0x04000000,"IMAGE_SCN_MEM_NOT_CACHED");
    mapResult.insert(0x08000000,"IMAGE_SCN_MEM_NOT_PAGED");
    mapResult.insert(0x10000000,"IMAGE_SCN_MEM_SHARED");
    mapResult.insert(0x20000000,"IMAGE_SCN_MEM_EXECUTE");
    mapResult.insert(0x40000000,"IMAGE_SCN_MEM_READ");
    mapResult.insert(0x80000000,"IMAGE_SCN_MEM_WRITE");

    return mapResult;
}

QMap<quint64, QString> QPE::getImageSectionAligns()
{
    QMap<quint64, QString> mapResult;

    mapResult.insert(0x00100000,"IMAGE_SCN_ALIGN_1BYTES");
    mapResult.insert(0x00200000,"IMAGE_SCN_ALIGN_2BYTES");
    mapResult.insert(0x00300000,"IMAGE_SCN_ALIGN_4BYTES");
    mapResult.insert(0x00400000,"IMAGE_SCN_ALIGN_8BYTES");
    mapResult.insert(0x00500000,"IMAGE_SCN_ALIGN_16BYTES");
    mapResult.insert(0x00600000,"IMAGE_SCN_ALIGN_32BYTES");
    mapResult.insert(0x00700000,"IMAGE_SCN_ALIGN_64BYTES");
    mapResult.insert(0x00800000,"IMAGE_SCN_ALIGN_128BYTES");
    mapResult.insert(0x00900000,"IMAGE_SCN_ALIGN_256BYTES");
    mapResult.insert(0x00A00000,"IMAGE_SCN_ALIGN_512BYTES");
    mapResult.insert(0x00B00000,"IMAGE_SCN_ALIGN_1024BYTES");
    mapResult.insert(0x00C00000,"IMAGE_SCN_ALIGN_2048BYTES");
    mapResult.insert(0x00D00000,"IMAGE_SCN_ALIGN_4096BYTES");
    mapResult.insert(0x00E00000,"IMAGE_SCN_ALIGN_8192BYTES");

    return mapResult;
}
