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
#include "xbinary.h"

XBinary::XBinary(QIODevice *__pDevice, bool bIsImage, qint64 nImageBase)
{
    setData(__pDevice);
    setIsImage(bIsImage);
    setBaseAddress(0);
    setImageBase(nImageBase);
    setEntryPointOffset(0);
}

void XBinary::setData(QIODevice *__pDevice)
{
    this->__pDevice=__pDevice;
}

qint64 XBinary::getSize()
{
    return __pDevice->size();
}

quint32 XBinary::random32()
{
    static quint32 nSeed=0;
#if (QT_VERSION_MAJOR>=5)&&(QT_VERSION_MINOR>=10)
    nSeed=QRandomGenerator::global()->generate();
#else
    if(!nSeed)
    {
        quint32 nRValue=QDateTime::currentMSecsSinceEpoch()&0xFFFFFFFF;

        nSeed^=nRValue;
        qsrand(nSeed);
    }

    quint16 nValue1=(quint32)qrand();
    quint16 nValue2=(quint32)qrand();

    nSeed^=(nValue1<<16)+nValue2;
#endif
    return nSeed;
}

quint32 XBinary::random64()
{
    quint64 nVal1=random32();
    quint64 nVal2=random32();
    nVal1=nVal1<<32;
    return nVal1+nVal2;
}

QString XBinary::convertFileName(QString sFileName)
{
#ifdef Q_OS_MAC // TODO Check
    //    if(sFileName.startsWith("/.file/id="))
    //    {
    //        CFStringRef relCFStringRef =
    //            CFStringCreateWithCString(
    //                kCFAllocatorDefault,
    //                sFileName.toUtf8().constData(),
    //                kCFStringEncodingUTF8
    //            );
    //    CFURLRef relCFURL =
    //        CFURLCreateWithFileSystemPath(
    //            kCFAllocatorDefault,
    //            relCFStringRef,
    //            kCFURLPOSIXPathStyle,
    //            false // isDirectory
    //        );
    //    CFErrorRef error=0;
    //    CFURLRef absCFURL =
    //    CFURLCreateFilePathURL(
    //            kCFAllocatorDefault,
    //            relCFURL,
    //            &error
    //        );
    //    if(!error)
    //    {
    //        static const CFIndex maxAbsPathCStrBufLen=4096;
    //        char absPathCStr[maxAbsPathCStrBufLen];
    //        if(CFURLGetFileSystemRepresentation(
    //                absCFURL,
    //                true, // resolveAgainstBase
    //                reinterpret_cast<UInt8 *>(&absPathCStr[0] ),
    //                maxAbsPathCStrBufLen
    //            ))
    //            {
    //            sFileName=QString(absPathCStr);
    //            }
    //        }
    //        CFRelease(absCFURL);
    //        CFRelease(relCFURL);
    //        CFRelease(relCFStringRef);
    //    }
#endif

    QFileInfo fiLink(sFileName);

    if(fiLink.isSymLink())
    {
        sFileName=fiLink.symLinkTarget();
    }

    return sFileName;
}

void XBinary::findFiles(QString sDirectoryName, QList<QString> *pListFileNames)
{
    if((sDirectoryName!=".")&&(sDirectoryName!=".."))
    {
        QFileInfo fi(sDirectoryName);

        if(fi.isFile())
        {
            pListFileNames->append(fi.absoluteFilePath());
        }
        else if(fi.isDir())
        {
            QDir dir(sDirectoryName);

            QFileInfoList eil=dir.entryInfoList();

            for(int i=0; i<eil.count(); i++)
            {
                findFiles(eil.at(i).absoluteFilePath(),pListFileNames);
            }
        }
    }
}

void XBinary::findFiles(QString sDirectoryName, XBinary::FFOPTIONS *pFFOption, qint32 nLevel)
{
    *(pFFOption->pnNumberOfFiles)=pFFOption->pListFiles->count();

    if(!(*pFFOption->pbIsStop))
    {
        QFileInfo fi(sDirectoryName);

        if(fi.isFile())
        {
            pFFOption->pListFiles->append(fi.absoluteFilePath());
        }
        else if(fi.isDir()&&((pFFOption->bSubdirectories)||(nLevel==0)))
        {
            QDir dir(sDirectoryName);

            QFileInfoList eil=dir.entryInfoList();

            for(int i=0; (i<eil.count())&&(!(*(pFFOption->pbIsStop))); i++)
            {
                QString sFN=eil.at(i).fileName();

                if((sFN!=".")&&(sFN!=".."))
                {
                    findFiles(eil.at(i).absoluteFilePath(),pFFOption,nLevel+1);
                }
            }
        }
    }
}

QString XBinary::regExp(QString sRegExp, QString sString, int nIndex)
{
    QString sResult;
#if (QT_VERSION_MAJOR<5)
    QRegExp rxString(sRegExp);
    rxString.indexIn(sString);

    QStringList list=rxString.capturedTexts();

    if(list.count()>nIndex)
    {
        sResult=list.at(nIndex);
    }
#else
    QRegularExpression rxString(sRegExp);
    QRegularExpressionMatch matchString=rxString.match(sString);

    if(matchString.hasMatch())
    {
        sResult=matchString.captured(nIndex);
    }
#endif

    return sResult;
}

bool XBinary::isRegExpPresent(QString sRegExp, QString sString)
{
    return (regExp(sRegExp,sString,0)!="");
}

qint64 XBinary::read_array(qint64 nOffset, char *pBuffer, qint64 nMaxSize)
{
    qint64 nResult=0;

    if(__pDevice->seek(nOffset))
    {
        if(nMaxSize==-1)
        {
            nMaxSize=getSize()-nOffset;
        }

        nResult=__pDevice->read(pBuffer,nMaxSize);  // Check for read large files
    }

    return nResult;
}

QByteArray XBinary::read_array(qint64 nOffset, qint64 nSize)
{
    QByteArray baResult;

    baResult.resize((qint32)nSize);

    qint64 nBytes=read_array(nOffset,baResult.data(),nSize);

    if(nSize!=nBytes)
    {
        baResult.resize((qint32)nBytes);
    }

    return baResult;
}

qint64 XBinary::write_array(qint64 nOffset, char *pBuffer, qint64 nMaxSize)
{
    qint64 nResult=0;

    qint64 _nTotalSize=getSize();

    if(nMaxSize<=(_nTotalSize-nOffset))
    {
        if(__pDevice->seek(nOffset))
        {
            nResult=__pDevice->write(pBuffer,nMaxSize);
        }
    }

    return nResult;
}

quint8 XBinary::read_uint8(qint64 nOffset)
{
    quint8 result=0;

    read_array(nOffset,(char *)(&result),1);

    return result;
}

qint8 XBinary::read_int8(qint64 nOffset)
{
    quint8 result=0;

    read_array(nOffset,(char *)(&result),1);

    return (qint8)result;
}

quint16 XBinary::read_uint16(qint64 nOffset, bool bIsBigEndian)
{
    quint16 result=0;

    read_array(nOffset,(char *)(&result),2);

    if(bIsBigEndian)
    {
        result=qFromBigEndian(result);
    }
    else
    {
        result=qFromLittleEndian(result);
    }

    return result;
}

qint16 XBinary::read_int16(qint64 nOffset, bool bIsBigEndian)
{
    quint16 result=0;

    read_array(nOffset,(char *)(&result),2);

    if(bIsBigEndian)
    {
        result=qFromBigEndian(result);
    }
    else
    {
        result=qFromLittleEndian(result);
    }

    return (qint16)result;
}

quint32 XBinary::read_uint32(qint64 nOffset, bool bIsBigEndian)
{
    quint32 result=0;

    read_array(nOffset,(char *)(&result),4);

    if(bIsBigEndian)
    {
        result=qFromBigEndian(result);
    }
    else
    {
        result=qFromLittleEndian(result);
    }

    return result;
}

qint32 XBinary::read_int32(qint64 nOffset, bool bIsBigEndian)
{
    quint32 result=0;

    read_array(nOffset,(char *)(&result),4);

    if(bIsBigEndian)
    {
        result=qFromBigEndian(result);
    }
    else
    {
        result=qFromLittleEndian(result);
    }

    return (qint32)result;
}

quint64 XBinary::read_uint64(qint64 nOffset, bool bIsBigEndian)
{
    qint64 result=0;

    read_array(nOffset,(char *)(&result),8);

    if(bIsBigEndian)
    {
        result=qFromBigEndian(result);
    }
    else
    {
        result=qFromLittleEndian(result);
    }

    return result;
}

qint64 XBinary::read_int64(qint64 nOffset, bool bIsBigEndian)
{
    qint64 result=0;

    read_array(nOffset,(char *)(&result),8);

    if(bIsBigEndian)
    {
        result=qFromBigEndian(result);
    }
    else
    {
        result=qFromLittleEndian(result);
    }

    return (qint64)result;
}

qint64 XBinary::write_ansiString(qint64 nOffset, QString sString)
{
    return write_array(nOffset,sString.toLatin1().data(),sString.length()+1);
}

QString XBinary::read_ansiString(qint64 nOffset,qint64 nMaxSize)
{
    QString sResult;

    if(nMaxSize)
    {
        if(__pDevice->seek(nOffset))
        {
            // TODO optimize
            QByteArray baData=read_array(nOffset,nMaxSize);

            sResult=baData.data();
        }
    }

    return sResult;
}

QString XBinary::read_unicodeString(qint64 nOffset, qint64 nMaxSize,bool bIsBigEndian)
{
    QString sResult;

    if(nMaxSize)
    {
        quint16 *pBuffer=new quint16[nMaxSize+1];

        for(int i=0; i<nMaxSize; i++)
        {
            pBuffer[i]=read_uint16(nOffset+2*i,bIsBigEndian);

            if(pBuffer[i]==0)
            {
                break;
            }

            if(i==nMaxSize-1)
            {
                pBuffer[nMaxSize]=0;
            }
        }

        sResult=QString::fromUtf16(pBuffer);

        delete [] pBuffer;
    }

    return sResult;
}

QString XBinary::read_utf8String(qint64 nOffset, qint64 nMaxSize)
{
    QString sResult;

    if(nMaxSize)
    {
        qint32 nRealSize=0;

        for(int i=0; i<nMaxSize; i++)
        {
            quint8 nByte=read_uint8(nOffset+nRealSize);

            if(nByte==0)
            {
                break;
            }

            if((nByte>>7)&0x1)
            {
                nRealSize++;
            }
            else if((nByte>>5)&0x1)
            {
                nRealSize+=2;
            }
            else if((nByte>>4)&0x1)
            {
                nRealSize+=3;
            }
            else if((nByte>>3)&0x1)
            {
                nRealSize+=4;
            }
        }

        if(nRealSize)
        {
            QByteArray baString=read_array(nOffset,nRealSize);
            sResult=QString::fromUtf8(baString.data());
        }
    }

    return sResult;
}

void XBinary::write_uint8(qint64 nOffset, quint8 value)
{
    write_array(nOffset,(char *)(&value),1);
}

void XBinary::write_int8(qint64 nOffset, qint8 value)
{
    quint8 _value=(quint8)value;
    write_array(nOffset,(char *)(&_value),1);
}

void XBinary::write_uint16(qint64 nOffset, quint16 value, bool bIsBigEndian)
{
    if(bIsBigEndian)
    {
        value=qFromBigEndian(value);
    }
    else
    {
        value=qFromLittleEndian(value);
    }

    write_array(nOffset,(char *)(&value),2);
}

void XBinary::write_int16(qint64 nOffset, qint16 value, bool bIsBigEndian)
{
    quint16 _value=(quint16)value;

    if(bIsBigEndian)
    {
        _value=qFromBigEndian(_value);
    }
    else
    {
        _value=qFromLittleEndian(_value);
    }

    write_array(nOffset,(char *)(&_value),2);
}

void XBinary::write_uint32(qint64 nOffset, quint32 value, bool bIsBigEndian)
{
    if(bIsBigEndian)
    {
        value=qFromBigEndian(value);
    }
    else
    {
        value=qFromLittleEndian(value);
    }

    write_array(nOffset,(char *)(&value),4);
}

void XBinary::write_int32(qint64 nOffset, qint32 value, bool bIsBigEndian)
{
    quint32 _value=(quint32)value;

    if(bIsBigEndian)
    {
        _value=qFromBigEndian(_value);
    }
    else
    {
        _value=qFromLittleEndian(_value);
    }

    write_array(nOffset,(char *)(&_value),4);
}

void XBinary::write_uint64(qint64 nOffset, quint64 value, bool bIsBigEndian)
{
    if(bIsBigEndian)
    {
        value=qFromBigEndian(value);
    }
    else
    {
        value=qFromLittleEndian(value);
    }

    write_array(nOffset,(char *)(&value),8);
}

void XBinary::write_int64(qint64 nOffset, qint64 value, bool bIsBigEndian)
{
    qint64 _value=(qint64)value;

    if(bIsBigEndian)
    {
        _value=qFromBigEndian(_value);
    }
    else
    {
        _value=qFromLittleEndian(_value);
    }

    write_array(nOffset,(char *)(&_value),8);
}

quint8 XBinary::_read_uint8(char *pData)
{
    return *(quint8 *)pData;
}

qint8 XBinary::_read_int8(char *pData)
{
    return *(qint8 *)pData;
}

quint16 XBinary::_read_uint16(char *pData, bool bIsBigEndian)
{
    quint16 result=*(quint16 *)pData;

    if(bIsBigEndian)
    {
        result=qFromBigEndian(result);
    }
    else
    {
        result=qFromLittleEndian(result);
    }

    return result;
}

qint16 XBinary::_read_int16(char *pData, bool bIsBigEndian)
{
    qint16 result=*(qint16 *)pData;

    if(bIsBigEndian)
    {
        result=qFromBigEndian(result);
    }
    else
    {
        result=qFromLittleEndian(result);
    }

    return result;
}

quint32 XBinary::_read_uint32(char *pData, bool bIsBigEndian)
{
    quint32 result=*(quint32 *)pData;

    if(bIsBigEndian)
    {
        result=qFromBigEndian(result);
    }
    else
    {
        result=qFromLittleEndian(result);
    }

    return result;
}

qint32 XBinary::_read_int32(char *pData, bool bIsBigEndian)
{
    qint32 result=*(qint32 *)pData;

    if(bIsBigEndian)
    {
        result=qFromBigEndian(result);
    }
    else
    {
        result=qFromLittleEndian(result);
    }

    return result;
}

qint64 XBinary::_read_uint64(char *pData, bool bIsBigEndian)
{
    qint64 result=*(qint64 *)pData;

    if(bIsBigEndian)
    {
        result=qFromBigEndian(result);
    }
    else
    {
        result=qFromLittleEndian(result);
    }

    return result;
}

qint64 XBinary::_read_int64(char *pData, bool bIsBigEndian)
{
    qint64 result=*(qint64 *)pData;

    if(bIsBigEndian)
    {
        result=qFromBigEndian(result);
    }
    else
    {
        result=qFromLittleEndian(result);
    }

    return result;
}

QString XBinary::_read_ansiString(char *pData, int nMaxSize)
{
    QByteArray baData(pData,nMaxSize);
    QString sResult;
    sResult.append(baData.data());
    return sResult;
}

QByteArray XBinary::_read_byteArray(char *pData, int nSize)
{
    return QByteArray(pData,nSize);
}

void XBinary::_write_uint8(char *pData, quint8 value)
{
    *(quint8 *)pData=value;
}

void XBinary::_write_int8(char *pData, qint8 value)
{
    *(qint8 *)pData=value;
}

void XBinary::_write_uint16(char *pData, quint16 value, bool bIsBigEndian)
{
    if(bIsBigEndian)
    {
        value=qToBigEndian(value);
    }
    else
    {
        value=qToLittleEndian(value);
    }

    *(quint16 *)pData=value;
}

void XBinary::_write_int16(char *pData, qint16 value, bool bIsBigEndian)
{
    if(bIsBigEndian)
    {
        value=qToBigEndian(value);
    }
    else
    {
        value=qToLittleEndian(value);
    }

    *(qint16 *)pData=value;
}

void XBinary::_write_uint32(char *pData, quint32 value, bool bIsBigEndian)
{
    if(bIsBigEndian)
    {
        value=qToBigEndian(value);
    }
    else
    {
        value=qToLittleEndian(value);
    }

    *(quint32 *)pData=value;
}

void XBinary::_write_int32(char *pData, qint32 value, bool bIsBigEndian)
{
    if(bIsBigEndian)
    {
        value=qToBigEndian(value);
    }
    else
    {
        value=qToLittleEndian(value);
    }

    *(qint32 *)pData=value;
}

void XBinary::_write_uint64(char *pData, qint64 value, bool bIsBigEndian)
{
    if(bIsBigEndian)
    {
        value=qToBigEndian(value);
    }
    else
    {
        value=qToLittleEndian(value);
    }

    *(qint64 *)pData=value;
}

void XBinary::_write_int64(char *pData, qint64 value, bool bIsBigEndian)
{
    if(bIsBigEndian)
    {
        value=qToBigEndian(value);
    }
    else
    {
        value=qToLittleEndian(value);
    }

    *(qint64 *)pData=value;
}

qint64 XBinary::find_array(qint64 nOffset, qint64 nSize,const char *pArray, qint64 nArraySize)
{
    qint64 _nSize=getSize();

    if(nSize==-1)
    {
        nSize=_nSize-nOffset;
    }

    if(nSize<=0)
    {
        return -1;
    }

    if(nOffset+nSize>_nSize)
    {
        return -1;
    }

    if(nArraySize>nSize)
    {
        return -1;
    }

    qint64 nTemp=0;
    const int BUFFER_SIZE=0x1000;
    char *pBuffer=new char[BUFFER_SIZE+(nArraySize-1)];

    while(nSize>nArraySize-1)
    {
        nTemp=qMin((qint64)(BUFFER_SIZE+(nArraySize-1)),nSize);

        if(!read_array(nOffset,pBuffer,nTemp))
        {
            break;
        }

        for(unsigned int i=0; i<nTemp-(nArraySize-1); i++)
        {
            if(compareMemory(pBuffer+i,pArray,nArraySize))
            {
                delete[] pBuffer;

                return nOffset+i;
            }
        }

        nSize-=nTemp-(nArraySize-1);
        nOffset+=nTemp-(nArraySize-1);
    }

    delete[] pBuffer;

    return -1;
}

qint64 XBinary::find_uint8(qint64 nOffset, qint64 nSize, quint8 value)
{
    return find_array(nOffset,nSize,(char *)&value,1);
}

qint64 XBinary::find_int8(qint64 nOffset, qint64 nSize, qint8 value)
{
    return find_array(nOffset,nSize,(char *)&value,1);
}

qint64 XBinary::find_uint16(qint64 nOffset, qint64 nSize, quint16 value, bool bIsBigEndian)
{
    if(bIsBigEndian)
    {
        value=qFromBigEndian(value);
    }
    else
    {
        value=qFromLittleEndian(value);
    }

    return find_array(nOffset,nSize,(char *)&value,2);
}

qint64 XBinary::find_int16(qint64 nOffset, qint64 nSize, qint16 value, bool bIsBigEndian)
{
    quint16 _value=(quint16)value;

    if(bIsBigEndian)
    {
        _value=qFromBigEndian(_value);
    }
    else
    {
        _value=qFromLittleEndian(_value);
    }

    return find_array(nOffset,nSize,(char *)&_value,2);
}

qint64 XBinary::find_uint32(qint64 nOffset, qint64 nSize, quint32 value, bool bIsBigEndian)
{
    if(bIsBigEndian)
    {
        value=qFromBigEndian(value);
    }
    else
    {
        value=qFromLittleEndian(value);
    }

    return find_array(nOffset,nSize,(char *)&value,4);
}

qint64 XBinary::find_int32(qint64 nOffset, qint64 nSize, qint32 value, bool bIsBigEndian)
{
    quint32 _value=(quint32)value;

    if(bIsBigEndian)
    {
        _value=qFromBigEndian(_value);
    }
    else
    {
        _value=qFromLittleEndian(_value);
    }

    return find_array(nOffset,nSize,(char *)&value,4);
}

qint64 XBinary::find_uint64(qint64 nOffset, qint64 nSize, qint64 value, bool bIsBigEndian)
{
    if(bIsBigEndian)
    {
        value=qFromBigEndian(value);
    }
    else
    {
        value=qFromLittleEndian(value);
    }

    return find_array(nOffset,nSize,(char *)&value,8);
}

qint64 XBinary::find_int64(qint64 nOffset, qint64 nSize, qint64 value, bool bIsBigEndian)
{
    qint64 _value=(qint64)value;

    if(bIsBigEndian)
    {
        _value=qFromBigEndian(_value);
    }
    else
    {
        _value=qFromLittleEndian(_value);
    }

    return find_array(nOffset,nSize,(char *)&_value,8);
}

qint64 XBinary::find_ansiString(qint64 nOffset, qint64 nSize, QString sString)
{
    return find_array(nOffset,nSize,sString.toLatin1().data(),sString.size());
}

qint64 XBinary::find_unicodeString(qint64 nOffset, qint64 nSize, QString sString)
{
    return find_array(nOffset,nSize,(char *)sString.utf16(),sString.size()*2);
}

qint64 XBinary::find_signature(qint64 nOffset, qint64 nSize, QString sSignature)
{
    sSignature=convertSignature(sSignature);

    qint64 nResult=-1;

    if(sSignature.contains(".")||sSignature.contains("$")||sSignature.contains("#"))
    {
        sSignature=convertSignature(sSignature);

        QList<SIGNATURE_RECORD> records=getSignatureRecords(sSignature);

        for(qint64 i=0; i<nSize; i++)
        {
            if(_compareSignature(&records,nOffset+i))
            {
                nResult=nOffset+i;
                break;
            }
        }
    }
    else
    {
        QByteArray baData=QByteArray::fromHex(QByteArray(sSignature.toLatin1().data()));
        nResult=find_array(nOffset,nSize,baData.data(),baData.size());
    }

    return nResult;
}

bool XBinary::createFile(QString sFileName, qint64 nFileSize)
{
    bool bResult=false;

    QFile file;

    file.setFileName(sFileName);

    if(file.open(QIODevice::ReadWrite))
    {
        bResult=true;

        if(nFileSize)
        {
            bResult=file.resize(nFileSize);
        }

        file.close();
    }

    return bResult;
}

bool XBinary::isFileExists(QString sFileName)
{
    QFileInfo fi(sFileName);

    return (fi.exists()&&fi.isFile());
}

bool XBinary::removeFile(QString sFileName)
{
    return QFile::remove(sFileName);
}

bool XBinary::copyFile(QString sSrcFileName,QString sDestFileName)
{
    return QFile::copy(sSrcFileName,sDestFileName);
}

bool XBinary::moveFile(QString sSrcFileName, QString sDestFileName)
{
    bool bResult=false;

    if(copyFile(sSrcFileName,sDestFileName))
    {
        bResult=removeFile(sSrcFileName);

        if(!bResult)
        {
            removeFile(sDestFileName);
        }
    }

    return bResult;
}

bool XBinary::moveFileToDirectory(QString sSrcFileName, QString sDestDirectory)
{
    QFileInfo fi(sSrcFileName);

    return moveFile(sSrcFileName,sDestDirectory+QDir::separator()+fi.fileName());
}

void XBinary::_copyMemory(char *pDest,char *pSource, qint64 nSize)
{
    // TODO optimize
    while(nSize)
    {
        *pDest=*pSource;
        pDest++;
        pSource++;
        nSize--;
    }
}

bool XBinary::copyDeviceMemory(QIODevice *pSourceDevice,qint64 nSourceOffset,QIODevice *pDestDevice,qint64 nDestOffset,qint64 nSize,quint32 nBufferSize)
{
    // TODO optimize
    if((!pSourceDevice->seek(nSourceOffset))||(!pDestDevice->seek(nDestOffset)))
    {
        return false;
    }

    char *pBuffer=new char[nBufferSize];

    while(nSize>0)
    {
        qint64 nCurrentBufferSize=qMin(nSize,(qint64)nBufferSize);

        if(nCurrentBufferSize!=pSourceDevice->read(pBuffer,nCurrentBufferSize))
        {
            break;
        }

        if(nCurrentBufferSize!=pDestDevice->write(pBuffer,nCurrentBufferSize))
        {
            break;
        }

        nSize-=nCurrentBufferSize;
    }

    delete [] pBuffer;

    return (bool)(nSize==0);
}

bool XBinary::copyMemory(qint64 nSourceOffset, qint64 nDestOffset, qint64 nSize, quint32 nBufferSize, bool bReverse)
{
    // TODO optimize
    if(nBufferSize==0)
    {
        return false;
    }

    if(nDestOffset==nSourceOffset)
    {
        return true;
    }

    qint64 nMaxSize=getSize();

    if((nDestOffset+nSize>nMaxSize)||(nSourceOffset+nSize>nMaxSize))
    {
        return false;
    }

    // TODO
    char *pBuffer=new char[nBufferSize];

    if(bReverse)
    {
        nSourceOffset+=nSize;
        nDestOffset+=nSize;
    }

    while(nSize>0)
    {
        qint64 nTempSize=qMin(nSize,(qint64)nBufferSize);

        if(bReverse)
        {
            nSourceOffset-=nTempSize;
            nDestOffset-=nTempSize;
        }

        read_array(nSourceOffset,pBuffer,nTempSize);
        write_array(nDestOffset,pBuffer,nTempSize);

        if(!bReverse)
        {
            nSourceOffset+=nTempSize;
            nDestOffset+=nTempSize;
        }

        nSize-=nTempSize;
    }

    delete [] pBuffer;

    return false;
}

bool XBinary::zeroFill(qint64 nOffset, qint64 nSize)
{
    // TODO optimize
    if(nSize==0)
    {
        return false;
    }

    qint64 nMaxSize=getSize();

    if(nOffset+nSize>nMaxSize)
    {
        return false;
    }

    quint8 cZero=0;

    // TODO optimize with dwords
    if(__pDevice->seek(nOffset))
    {
        while(nSize>0)
        {
            __pDevice->write((char *)&cZero,1);
            nSize--;
        }
    }

    return true;
}

bool XBinary::compareMemory(char *pMemory1,const char *pMemory2, qint64 nSize)
{
    // TODO optimize
    while(nSize>0)
    {
        if(nSize>=4)
        {
            if(*((unsigned int *)pMemory1)!=*((unsigned int *)pMemory2))
            {
                return false;
            }

            pMemory1+=4;
            pMemory2+=4;
            nSize-=4;
        }
        else if(nSize>=2)
        {
            if(*((unsigned short *)pMemory1)!=*((unsigned short *)pMemory2))
            {
                return false;
            }

            pMemory1+=2;
            pMemory2+=2;
            nSize-=2;
        }
        else
        {
            if(*(pMemory1)!=*(pMemory2))
            {
                return false;
            }

            pMemory1++;
            pMemory2++;
            nSize--;
        }
    }

    return true;
}

bool XBinary::isOffsetValid(qint64 nOffset)
{
    QList<MEMORY_MAP> list=getMemoryMapList();

    return isOffsetValid(&list,nOffset);
}

bool XBinary::isAddressValid(qint64 nAddress,ADDRESS_SEGMENT segment)
{
    QList<MEMORY_MAP> list=getMemoryMapList();

    return isAddressValid(&list,nAddress,segment);
}

qint64 XBinary::offsetToAddress(qint64 nOffset,ADDRESS_SEGMENT segment)
{
    QList<MEMORY_MAP> list=getMemoryMapList();

    return offsetToAddress(&list,nOffset,segment);
}

qint64 XBinary::addressToOffset(qint64 nAddress,ADDRESS_SEGMENT segment)
{
    QList<MEMORY_MAP> list=getMemoryMapList();

    return addressToOffset(&list,nAddress,segment);
}

bool XBinary::isOffsetValid(QList<XBinary::MEMORY_MAP> *pMemoryMap, qint64 nOffset)
{
    bool bResult=false;

    for(int i=0; i<pMemoryMap->count(); i++)
    {
        if(pMemoryMap->at(i).nSize&&(pMemoryMap->at(i).nOffset!=-1))
        {
            if((pMemoryMap->at(i).nOffset<=nOffset)&&(nOffset<pMemoryMap->at(i).nOffset+pMemoryMap->at(i).nSize))
            {
                bResult=true;
                break;
            }
        }
    }

    return bResult;
}

bool XBinary::isAddressValid(QList<XBinary::MEMORY_MAP> *pMemoryMap, qint64 nAddress, ADDRESS_SEGMENT segment)
{
    bool bResult=false;

    for(int i=0; i<pMemoryMap->count(); i++)
    {
        if(pMemoryMap->at(i).nSize&&(pMemoryMap->at(i).nAddress!=-1)&&(pMemoryMap->at(i).segment==segment))
        {
            if((pMemoryMap->at(i).nAddress<=nAddress)&&(nAddress<pMemoryMap->at(i).nAddress+pMemoryMap->at(i).nSize))
            {
                bResult=true;
                break;
            }
        }
    }

    return bResult;
}

qint64 XBinary::offsetToAddress(QList<XBinary::MEMORY_MAP> *pMemoryMap, qint64 nOffset, ADDRESS_SEGMENT segment)
{
    qint64 nResult=-1;

    for(int i=0; i<pMemoryMap->count(); i++)
    {
        if(pMemoryMap->at(i).nSize&&(pMemoryMap->at(i).nOffset!=-1)&&(pMemoryMap->at(i).nAddress!=-1)&&(pMemoryMap->at(i).segment==segment))
        {
            if((pMemoryMap->at(i).nOffset<=nOffset)&&(nOffset<pMemoryMap->at(i).nOffset+pMemoryMap->at(i).nSize))
            {
                nResult=(nOffset-pMemoryMap->at(i).nOffset)+pMemoryMap->at(i).nAddress;
                break;
            }
        }
    }

    return nResult;
}

qint64 XBinary::addressToOffset(QList<XBinary::MEMORY_MAP> *pMemoryMap, qint64 nAddress, ADDRESS_SEGMENT segment)
{
    qint64 nResult=-1;

    int nCount=pMemoryMap->count();

    for(int i=0; i<nCount; i++)
    {
        if(pMemoryMap->at(i).nSize&&(pMemoryMap->at(i).nAddress!=-1)&&(pMemoryMap->at(i).nOffset!=-1)&&(pMemoryMap->at(i).segment==segment))
        {
            if((pMemoryMap->at(i).nAddress<=nAddress)&&(nAddress<pMemoryMap->at(i).nAddress+pMemoryMap->at(i).nSize))
            {
                nResult=(nAddress-pMemoryMap->at(i).nAddress)+pMemoryMap->at(i).nOffset;
                break;
            }
        }
    }

    return nResult;
}

XBinary::MEMORY_MAP XBinary::getOffsetMemoryMap(QList<XBinary::MEMORY_MAP> *pMemoryMap, qint64 nOffset)
{
    MEMORY_MAP result= {};

    for(int i=0; i<pMemoryMap->count(); i++)
    {
        if(pMemoryMap->at(i).nSize&&(pMemoryMap->at(i).nOffset!=-1))
        {
            if((pMemoryMap->at(i).nOffset<=nOffset)&&(nOffset<pMemoryMap->at(i).nOffset+pMemoryMap->at(i).nSize))
            {
                result=pMemoryMap->at(i);
                break;
            }
        }
    }

    return result;
}

XBinary::MEMORY_MAP XBinary::getAddressMemoryMap(QList<XBinary::MEMORY_MAP> *pMemoryMap, qint64 nAddress, ADDRESS_SEGMENT segment)
{
    MEMORY_MAP result= {};

    for(int i=0; i<pMemoryMap->count(); i++)
    {
        if(pMemoryMap->at(i).nSize&&(pMemoryMap->at(i).nAddress!=-1)&&(pMemoryMap->at(i).segment==segment))
        {
            if((pMemoryMap->at(i).nAddress<=nAddress)&&(nAddress<pMemoryMap->at(i).nAddress+pMemoryMap->at(i).nSize))
            {
                result=pMemoryMap->at(i);
                break;
            }
        }
    }

    return result;
}

QList<XBinary::MEMORY_MAP> XBinary::getMemoryMapList()
{
    QList<MEMORY_MAP> listMemoryMap;

    MEMORY_MAP record= {};
    record.nAddress=_getBaseAddress();
    record.segment=ADDRESS_SEGMENT_FLAT;
    record.nOffset=0;
    record.nSize=getSize();

    listMemoryMap.append(record);

    return listMemoryMap;
}
qint64 XBinary::getBaseAddress()
{
    return this->__nBaseAddress;
}

void XBinary::setBaseAddress(qint64 nBaseAddress)
{
    this->__nBaseAddress=nBaseAddress;
}

bool XBinary::isImage()
{
    return bIsImage;
}

void XBinary::setIsImage(bool value)
{
    bIsImage=value;
}

bool XBinary::compareSignature(QString sSignature, qint64 nOffset)
{
    sSignature=convertSignature(sSignature);

    //Convert to List
    //int nType
    //0: compare bytes
    //1: relative offset(fix)
    //2: relative offset
    //3: address
    //QByteArray bytes
    //int size of address/offset
    //int Base of address

    QList<SIGNATURE_RECORD> records=getSignatureRecords(sSignature);

    return _compareSignature(&records,nOffset);
}

bool XBinary::_compareByteArrayWithSignature(QByteArray baData, QString sSignature)
{
    bool bResult=false;

    QString sHex=baData.toHex().data();

    if(sHex.size()==sSignature.size())
    {
        bResult=true;
        int nCount=sSignature.size();

        for(int i=0; i<nCount; i++)
        {
            if(sSignature.at(i)!=QChar('.'))
            {
                if(sSignature.at(i)!=sHex.at(i))
                {
                    bResult=false;
                    break;
                }
            }
        }
    }

    return bResult;
}

QString XBinary::_createSignature(QString sSignature1, QString sSignature2)
{
    QString sResult;

    if(sSignature1.size()==sSignature2.size())
    {
        for(int i=0; i<sSignature1.size(); i+=2)
        {
            if(sSignature1.mid(i,2)==sSignature2.mid(i,2))
            {
                sResult.append(sSignature1.mid(i,2));
            }
            else
            {
                sResult.append("..");
            }
        }
    }

    return sResult;
}

bool XBinary::compareSignatureOnAddress(QString sSignature, qint64 nAddress,ADDRESS_SEGMENT segment)
{
    bool bResult=false;
    qint64 nOffset=addressToOffset(nAddress,segment);

    if(nOffset!=-1)
    {
        bResult=compareSignature(sSignature,nOffset);
    }

    return bResult;
}

qint64 XBinary::getEntryPointOffset()
{
    return this->__nEntryPointOffset;
}

void XBinary::setEntryPointOffset(qint64 nEntryPointOffset)
{
    this->__nEntryPointOffset=nEntryPointOffset;
}

qint64 XBinary::getEntryPointAddress()
{
    return offsetToAddress(getEntryPointOffset());
}

qint64 XBinary::getImageAddress()
{
    return this->__nImageBase;
}

void XBinary::setImageBase(qint64 nValue)
{
    this->__nImageBase=nValue;
}

qint64 XBinary::_getBaseAddress()
{
    qint64 nResult=0;

    qint64 nImageAddress=getImageAddress();

    if(isImage()&&(nImageAddress!=-1))
    {
        nResult=nImageAddress;
    }
    else
    {
        nResult=getBaseAddress();
    }

    return nResult;
}

bool XBinary::compareEntryPoint(QString sSignature, qint64 nOffset)
{
    qint64 nEPOffset=getEntryPointOffset()+nOffset;

    return compareSignature(sSignature,nEPOffset);
}

bool XBinary::moveMemory(qint64 nSourceOffset,qint64 nDestOffset, qint64 nSize)
{
    bool bResult=false;

    if(nDestOffset==nSourceOffset)
    {
        return true;
    }

    qint64 nMaxSize=getSize();

    if((nDestOffset+nSize>nMaxSize)||(nSourceOffset+nSize>nMaxSize))
    {
        return false;
    }

    qint64 nDelta=nDestOffset-nSourceOffset;

    if(nDelta>0)
    {
        bResult=copyMemory(nSourceOffset,nDestOffset,nSize,nDelta,true);
        zeroFill(nSourceOffset,nDelta);
    }
    else
    {
        bResult=copyMemory(nSourceOffset,nDestOffset,nSize,-nDelta,false);
        zeroFill(nDestOffset+nSize,-nDelta);
    }

    return bResult;
}

bool XBinary::dumpToFile(QString sFileName, const char *pData, qint64 nDataSize)
{
    bool bResult=false;
    QFile file;

    file.setFileName(sFileName);
    file.resize(0);

    if(file.open(QIODevice::ReadWrite))
    {
        file.write(pData,nDataSize);

        bResult=true;

        file.close();
    }

    return bResult;
}
// TODO ProgressBar
bool XBinary::dumpToFile(QString sFileName, qint64 nDataOffset, qint64 nDataSize)
{
    bool bResult=false;
    QFile file;

    file.setFileName(sFileName);
    file.resize(0);

    if(file.open(QIODevice::ReadWrite))
    {
        // TODO
        char *pBuffer=new char[0x1000];

        qint64 nSourceOffset=nDataOffset;
        qint64 nDestOffset=0;

        bResult=true;

        while(nDataSize>0)
        {
            qint64 nTempSize=qMin(nDataSize,(qint64)0x1000);

            read_array(nSourceOffset,pBuffer,nTempSize);

            file.seek(nDestOffset);

            if(file.write(pBuffer,nTempSize)==0)
            {
                bResult=false;
                break;
            }

            nSourceOffset+=nTempSize;
            nDestOffset+=nTempSize;

            nDataSize-=nTempSize;
        }

        delete [] pBuffer;

        file.close();
    }

    return bResult;
}

QSet<XBinary::FT> XBinary::getFileTypes()
{
    QSet<XBinary::FT> stResult;

    stResult.insert(FT_BINARY);

    QByteArray baHeader;
    baHeader=read_array(0,0x200);
    char *pOffset=baHeader.data();
    unsigned int nSize=getSize();

    if(nSize>=(int)sizeof(XMSDOS_DEF::IMAGE_DOS_HEADEREX))
    {
        if((((XMSDOS_DEF::IMAGE_DOS_HEADEREX *)pOffset)->e_magic==XMSDOS_DEF::S_IMAGE_DOS_SIGNATURE)||(((XMSDOS_DEF::IMAGE_DOS_HEADEREX *)pOffset)->e_magic==0x4D5A))
        {
            unsigned int nLfanew=0;
            nLfanew=((XMSDOS_DEF::IMAGE_DOS_HEADEREX *)pOffset)->e_lfanew;
            unsigned int nHeaderSize=baHeader.size()-sizeof(XPE_DEF::IMAGE_NT_HEADERS32);
            QByteArray baNTHeaders;

            bool isHeaderValid=false;

            if((nLfanew<nHeaderSize)&&((quint32)baHeader.size()>sizeof(XPE_DEF::IMAGE_NT_HEADERS32)))
            {
                pOffset+=nLfanew;
                isHeaderValid=true;
            }
            else
            {
                baNTHeaders=read_array(nLfanew,sizeof(XPE_DEF::IMAGE_NT_HEADERS32));

                if(baNTHeaders.size()==sizeof(XPE_DEF::IMAGE_NT_HEADERS32))
                {
                    pOffset=baNTHeaders.data();
                    isHeaderValid=true;
                }
            }

            if(isHeaderValid)
            {
                if((((XPE_DEF::IMAGE_NT_HEADERS32 *)pOffset))->Signature==XPE_DEF::S_IMAGE_NT_SIGNATURE)
                {
                    if((((XPE_DEF::IMAGE_NT_HEADERS32 *)pOffset)->FileHeader.Machine)==XPE_DEF::S_IMAGE_FILE_MACHINE_AMD64)
                    {
                        stResult.insert(FT_PE64);
                    }
                    else if((((XPE_DEF::IMAGE_NT_HEADERS32 *)pOffset)->FileHeader.Machine)==XPE_DEF::S_IMAGE_FILE_MACHINE_IA64)
                    {
                        stResult.insert(FT_PE64);
                    }
                    else
                    {
                        stResult.insert(FT_PE32);
                    }
                }
            }

            stResult.insert(FT_MSDOS);
        }
    }

    if(nSize>=(int)sizeof(XELF_DEF::Elf32_Ehdr))
    {
        if((((XELF_DEF::Elf32_Ehdr *)pOffset)->e_ident[0]==0x7f) &&
                (((XELF_DEF::Elf32_Ehdr *)pOffset)->e_ident[1]=='E') &&
                (((XELF_DEF::Elf32_Ehdr *)pOffset)->e_ident[2]=='L') &&
                (((XELF_DEF::Elf32_Ehdr *)pOffset)->e_ident[3]=='F'))
        {
            if(((XELF_DEF::Elf32_Ehdr *)pOffset)->e_ident[4]==1)
            {
                stResult.insert(FT_ELF32);
            }
            else if(((XELF_DEF::Elf32_Ehdr *)pOffset)->e_ident[4]==2)
            {
                stResult.insert(FT_ELF64);
            }
        }
    }

    if(nSize>=(int)sizeof(XMACH_DEF::mach_header))
    {
        if(((XMACH_DEF::mach_header *)pOffset)->filetype<0xFFFF)
        {
            if((((XMACH_DEF::mach_header *)pOffset)->magic==XMACH_DEF::S_MH_MAGIC)||(((XMACH_DEF::mach_header *)pOffset)->magic==XMACH_DEF::S_MH_CIGAM))
            {
                stResult.insert(FT_MACH32);
            }
            else if((((XMACH_DEF::mach_header *)pOffset)->magic==XMACH_DEF::S_MH_MAGIC_64)||(((XMACH_DEF::mach_header *)pOffset)->magic==XMACH_DEF::S_MH_CIGAM_64))
            {
                stResult.insert(FT_MACH32);
            }
        }
    }
    //    if(isPlainText())
    //    {
    //        listResult.append(QString("Text"));
    //    }

    return stResult;
}

QSet<XBinary::FT> XBinary::getFileTypes(QIODevice *pDevice)
{
    XBinary _binary(pDevice);

    return _binary.getFileTypes();
}

QSet<XBinary::FT> XBinary::getFileTypes(QString sFileName)
{
    QSet<XBinary::FT> result;

    QFile file;
    file.setFileName(sFileName);

    if(file.open(QIODevice::ReadOnly))
    {
        XBinary _binary(&file);

        result=_binary.getFileTypes();

        file.close();
    }

    return result;
}

QString XBinary::valueToHex(quint8 value)
{
    return QString("%1").arg(value,2,16,QChar('0'));
}

QString XBinary::valueToHex(qint8 value)
{
    return valueToHex((quint8)value);
}

QString XBinary::valueToHex(quint16 value)
{
    return QString("%1").arg(value,4,16,QChar('0'));
}

QString XBinary::valueToHex(qint16 value)
{
    return valueToHex((quint16)value);
}

QString XBinary::valueToHex(quint32 value)
{
    return QString("%1").arg(value,8,16,QChar('0'));
}

QString XBinary::valueToHex(qint32 value)
{
    return valueToHex((quint32)value);
}

QString XBinary::valueToHex(quint64 value)
{
    return QString("%1").arg(value,16,16,QChar('0'));
}

QString XBinary::valueToHex(qint64 value)
{
    return valueToHex((quint64)value);
}

QString XBinary::getUnpackedName(QIODevice *pDevice)
{
    QString sResult="unpacked";

    QString sClassName=pDevice->metaObject()->className();

    if(sClassName=="QFile")
    {
        QFile *pFile=(QFile *)pDevice;

        QString sFileName=pFile->fileName(); // TODO

        if(sFileName!="")
        {
            sResult=getUnpackedName(sFileName);
        }
    }

    return sResult;
}

QString XBinary::getUnpackedName(QString sFileName)
{
    QFileInfo fi(sFileName);
    QString sResult=fi.absolutePath()+QDir::separator()+fi.completeBaseName()+".unp."+fi.suffix();
    //            sResult=fi.absolutePath()+QDir::separator()+fi.baseName()+".unp."+fi.completeSuffix();

    return sResult;
}

QString XBinary::getBackupName(QIODevice *pDevice)
{
    QString sResult="backup";

    QString sClassName=pDevice->metaObject()->className();

    if(sClassName=="QFile")
    {
        QFile *pFile=(QFile *)pDevice;

        QString sFileName=pFile->fileName(); // TODO

        if(sFileName!="")
        {
            sResult=getBackupName(sFileName);
        }
    }

    return sResult;
}

QString XBinary::getBackupName(QString sFileName)
{
    QFileInfo fi(sFileName);
    QString sResult=fi.absolutePath()+QDir::separator()+fi.completeBaseName()+fi.suffix()+".BAK";

    return sResult;
}

QString XBinary::getDeviceFileName(QIODevice *pDevice)
{
    QString sResult;

    QString sClassName=pDevice->metaObject()->className();

    if(sClassName=="QFile")
    {
        QFile *pFile=(QFile *)pDevice;

        sResult=pFile->fileName();
    }

    return sResult;
}

QString XBinary::getDeviceFilePath(QIODevice *pDevice)
{
    QString sResult;

    QString sClassName=pDevice->metaObject()->className();

    if(sClassName=="QFile")
    {
        QFile *pFile=(QFile *)pDevice;

        QString sFileName=pFile->fileName(); // TODO

        if(sFileName!="")
        {
            QFileInfo fi(sFileName);

            sResult=fi.absoluteFilePath();
        }
    }

    return sResult;
}

QList<qint64> XBinary::getFixupList(QIODevice *pDevice1, QIODevice *pDevice2, qint64 nDelta)
{
    QList<qint64> listResult;

    qint64 nSize1=pDevice1->size();
    qint64 nSize2=pDevice2->size();

    if(nSize1==nSize2)
    {
        qint64 nSize=nSize1;
        qint64 nTemp=0;
        const int BUFFER_SIZE=0x1000;
        char *pBuffer1=new char[BUFFER_SIZE+3];
        char *pBuffer2=new char[BUFFER_SIZE+3];
        qint64 nOffset=0;

        while(nSize>3)
        {
            nTemp=qMin((qint64)(BUFFER_SIZE+3),nSize);

            pDevice1->seek(nOffset);

            if(pDevice1->read(pBuffer1,nTemp)<=0)
            {
                break;
            }

            pDevice2->seek(nOffset);

            if(pDevice2->read(pBuffer2,nTemp)<=0)
            {
                break;
            }

            for(unsigned int i=0; i<nTemp-3; i++)
            {
                qint32 nValue1=*(qint32 *)(pBuffer1+i);
                qint32 nValue2=*(qint32 *)(pBuffer2+i);

                if(nValue1+nDelta==nValue2)
                {
                    listResult.append(nOffset+i);
                }
            }

            nSize-=nTemp-3;
            nOffset+=nTemp-3;
        }

        delete[] pBuffer1;
        delete[] pBuffer2;
    }

    return listResult;
}
QString XBinary::getMD5(qint64 nOffset,qint64 nSize)
{
    QString sResult;

    OFFSETSIZE offsize=convertOffsetAndSize(nOffset,nSize);

    nOffset=offsize.nOffset;
    nSize=offsize.nSize;

    if(nOffset!=-1)
    {
        const int BUFFER_SIZE=0x1000;

        quint64 nTemp=0;
        char *pBuffer=new char[BUFFER_SIZE];

        QCryptographicHash crypto(QCryptographicHash::Md5);

        while(nSize>0)
        {
            nTemp=qMin((qint64)BUFFER_SIZE,nSize);

            if(!read_array(nOffset,pBuffer,nTemp))
            {
                delete[] pBuffer;
                return "";
            }

            crypto.addData(pBuffer,nTemp);

            nSize-=nTemp;
            nOffset+=nTemp;
        }

        delete[] pBuffer;

        sResult=crypto.result().toHex();
    }

    return sResult;
}

QString XBinary::getMD5(QString sFileName)
{
    QString sResult;

    QFile file;
    file.setFileName(sFileName);

    if(file.open(QIODevice::ReadOnly))
    {
        XBinary binary(&file);

        sResult=binary.getMD5(0,-1);

        file.close();
    }

    return sResult;
}

QString XBinary::getSHA1(qint64 nOffset, qint64 nSize)
{
    QString sResult;

    OFFSETSIZE offsize=convertOffsetAndSize(nOffset,nSize);

    nOffset=offsize.nOffset;
    nSize=offsize.nSize;

    if(nOffset!=-1)
    {
        const int BUFFER_SIZE=0x1000;

        quint64 nTemp=0;
        char *pBuffer=new char[BUFFER_SIZE];

        QCryptographicHash crypto(QCryptographicHash::Sha1);

        while(nSize>0)
        {
            nTemp=qMin((qint64)BUFFER_SIZE,nSize);

            if(!read_array(nOffset,pBuffer,nTemp))
            {
                delete[] pBuffer;
                return "";
            }

            crypto.addData(pBuffer,nTemp);

            nSize-=nTemp;
            nOffset+=nTemp;
        }

        delete[] pBuffer;

        sResult=crypto.result().toHex();
    }

    return sResult;
}

quint32 XBinary::getAdler32(qint64 nOffset, qint64 nSize)
{
    // TODO optimize!!!
    quint32 nResult=0;

    OFFSETSIZE offsize=convertOffsetAndSize(nOffset,nSize);

    nOffset=offsize.nOffset;
    nSize=offsize.nSize;

    const quint32 MOD_ADLER=65521;

    if(nOffset!=-1)
    {
        quint32 a=1;
        quint32 b=0;

        // Process each byte of the data in order
        for(qint64 index=0;index<nSize;++index)
        {
            a=(a+read_uint8(nOffset+index))%MOD_ADLER;
            b=(b+a)%MOD_ADLER;
        }

        nResult=(b<<16)|a;
    }

    return nResult;
}

double XBinary::getEntropy(qint64 nOffset, qint64 nSize)
{
    double dResult=1.4426950408889634073599246810023;

    OFFSETSIZE offsize=convertOffsetAndSize(nOffset,nSize);

    nOffset=offsize.nOffset;
    nSize=offsize.nSize;

    if(nOffset!=-1)
    {
        double bytes[256]= {0.0};

        const int BUFFER_SIZE=0x1000;

        quint64 nTemp=0;
        char *pBuffer=new char[BUFFER_SIZE];

        while(nSize>0)
        {
            nTemp=qMin((qint64)BUFFER_SIZE,nSize);

            if(!read_array(nOffset,pBuffer,nTemp))
            {
                delete[] pBuffer;
                return 0;
            }

            for(quint64 i=0; i<nTemp; i++)
            {
                bytes[(unsigned char)pBuffer[i]]+=1;
            }

            nSize-=nTemp;
            nOffset+=nTemp;
        }

        delete[] pBuffer;

        double dTemp;

        for(int j=0; j<256; j++)
        {
            dTemp=bytes[j]/(double)offsize.nSize;

            if(dTemp)
            {
                dResult+=(-log(dTemp)/log((double)2))*bytes[j];
            }
        }

        dResult=dResult/(double)offsize.nSize;
    }

    return dResult;
}

void XBinary::_xor(quint8 nXorValue, qint64 nOffset, qint64 nSize)
{
    OFFSETSIZE offsize=convertOffsetAndSize(nOffset,nSize);

    nOffset=offsize.nOffset;
    nSize=offsize.nSize;

    if(nOffset!=-1)
    {
        if(nOffset!=-1)
        {
            const int BUFFER_SIZE=0x1000;

            quint64 nTemp=0;
            char *pBuffer=new char[BUFFER_SIZE];

            while(nSize>0)
            {
                nTemp=qMin((qint64)BUFFER_SIZE,nSize);

                if(!read_array(nOffset,pBuffer,nTemp))
                {
                    break;
                }

                for(int i=0;i<nTemp;i++)
                {
                    *(pBuffer+i)^=nXorValue;
                }

                if(!write_array(nOffset,pBuffer,nTemp))
                {
                    break;
                }

                nSize-=nTemp;
                nOffset+=nTemp;
            }

            delete[] pBuffer;
        }
    }
}

//quint32 XBinary::_ror32(quint32 nValue, quint32 nShift)
//{
//    // TODO Check
//    nShift&=(31);
//    return (nValue>>nShift)|(nValue<<((-nShift)&31));
//}

//quint32 XBinary::_rol32(quint32 nValue, quint32 nShift)
//{
//    // TODO Check
//    nShift&=(31);
//    return (nValue<<nShift)|(nValue>>((-nShift)&31));
//}

quint32 XBinary::getCRC32(QString sString)
{
    quint32 nResult=0;

    //    for(int i=0;i<sString.size();i++)
    //    {
    //        unsigned char _char=(unsigned char)sString.at(i).toLatin1();
    //        nResult=_rol32(nResult,_char);
    //        nResult+=_char;
    //    }

    //    char *pData=sString.toLatin1().data();
    int nSize=sString.size();

    for(int i=0; i<nSize; i++)
    {
        unsigned char _char=(unsigned char)sString.at(i).toLatin1();
        //        unsigned char _char=(unsigned char)pData[i];
        nResult^=_char;

        for(int k=0; k<8; k++)
        {
            nResult=nResult&1?(nResult>>1)^0x82f63b78:nResult>>1;
        }
    }

    nResult=~nResult;

    return nResult;
}

QIODevice *XBinary::getDevice()
{
    return __pDevice;
}

bool XBinary::isValid()
{
    return true;
}

QString XBinary::getSignature(qint64 nOffset, qint64 nSize)
{
    QString sResult;

    if(nOffset!=-1)
    {
        sResult=read_array(nOffset,nSize).toHex().toUpper();
    }

    return sResult;
}

XBinary::OFFSETSIZE XBinary::convertOffsetAndSize(qint64 nOffset, qint64 nSize)
{
    OFFSETSIZE result= {};

    result.nOffset=-1;
    result.nSize=0;

    qint64 nTotalSize=getSize();

    if((nSize)&&(nOffset>=0)&&(nOffset<nTotalSize))
    {
        if(nSize==-1)
        {
            nSize=nTotalSize-nOffset;
        }

        result.nOffset=nOffset;
        result.nSize=nSize;
    }

    return result;
}

bool XBinary::compareSignatureStrings(QString sBaseSignature, QString sOptSignature)
{
    // TODO optimize
    // TODO check
    sBaseSignature=convertSignature(sBaseSignature);
    sOptSignature=convertSignature(sOptSignature);

    int nSize=qMin(sBaseSignature.size(),sOptSignature.size());

    if(!nSize)
    {
        return false;
    }

    if(sBaseSignature.size()<sOptSignature.size())
    {
        return false;
    }

    for(int i=0; i<nSize; i++)
    {
        QChar _qchar1=sBaseSignature.at(i);
        QChar _qchar2=sOptSignature.at(i);

        if((_qchar1!=QChar('.'))&&(_qchar2!=QChar('.')))
        {
            if(_qchar1!=_qchar2)
            {
                return false;
            }
        }
    }

    return true;
}

void XBinary::_errorMessage(QString sMessage)
{
#ifdef QT_DEBUG
    qDebug("Error: %s",sMessage.toLatin1().data());
#endif
    emit errorMessage(sMessage);
}

void XBinary::_infoMessage(QString sMessage)
{
#ifdef QT_DEBUG
    qDebug("Info: %s",sMessage.toLatin1().data());
#endif
    emit infoMessage(sMessage);
}

QString XBinary::convertSignature(QString sSignature)
{
    // 'AnsiString'
    if(sSignature.contains(QChar(39)))
    {
        QString sTemp;
        int nStringSize=sSignature.size();
        bool bAnsiString=false;

        for(int i=0; i<nStringSize; i++)
        {
            QChar c=sSignature.at(i);

            if(c==QChar(39))
            {
                bAnsiString=!bAnsiString;
            }
            else if(bAnsiString)
            {
                sTemp.append(qcharToHex(c));
            }
            else
            {
                sTemp.append(c);
            }
        }

        sSignature=sTemp;
    }

    if(sSignature.contains(QChar(' ')))
    {
        sSignature=sSignature.remove(QChar(' '));
    }

    if(sSignature.contains(QChar('?')))
    {
        sSignature=sSignature.replace(QChar('?'),QChar('.'));
    }

    sSignature=sSignature.toLower();

    return sSignature;
}

QString XBinary::qcharToHex(QChar c)
{
    // TODO mb
    return QString("%1").arg((quint8)(c.toLatin1()),2,16,QChar('0'));
}

QString XBinary::stringToHex(QString sString)
{
    return QString::fromLatin1(sString.toLatin1().toHex());
}

QString XBinary::hexToString(QString sHex)
{
    QString sResult;

    sResult=QByteArray::fromHex(sHex.toLatin1().data());

    return sResult;
}

quint8 XBinary::hexToUint8(QString sHex)
{
    quint8 nResult=0;

    if((quint32)sHex.length()>=sizeof(quint8))
    {
        sHex=sHex.mid(0,2*sizeof(quint8));
        bool bStatus=false;
        nResult=(quint8)(sHex.toInt(&bStatus,16));
    }

    return nResult;
}

qint8 XBinary::hexToInt8(QString sHex)
{
    quint8 nResult=0;

    if((quint32)sHex.length()>=sizeof(qint8))
    {
        sHex=sHex.mid(0,2*sizeof(qint8));
        bool bStatus=false;
        nResult=(qint8)(sHex.toInt(&bStatus,16));
    }

    return nResult;
}

quint16 XBinary::hexToUint16(QString sHex, bool bIsBigEndian)
{
    quint16 nResult=0;

    if((quint32)sHex.length()>=sizeof(quint16))
    {
        if(!bIsBigEndian)
        {
            sHex=invertHexByteString(sHex.mid(0,2*sizeof(quint16)));
        }

        bool bStatus=false;
        nResult=sHex.toUShort(&bStatus,16);
    }

    return nResult;
}

qint16 XBinary::hexToInt16(QString sHex, bool bIsBigEndian)
{
    qint16 nResult=0;

    if((quint32)sHex.length()>=sizeof(qint16))
    {
        if(!bIsBigEndian)
        {
            sHex=invertHexByteString(sHex.mid(0,2*sizeof(qint16)));
        }

        bool bStatus=false;
        nResult=sHex.toShort(&bStatus,16);
    }

    return nResult;
}

quint32 XBinary::hexToUint32(QString sHex, bool bIsBigEndian)
{
    quint32 nResult=0;

    if((quint32)sHex.length()>=sizeof(quint32))
    {
        if(!bIsBigEndian)
        {
            sHex=invertHexByteString(sHex.mid(0,2*sizeof(quint32)));
        }

        bool bStatus=false;
        nResult=sHex.toUInt(&bStatus,16);
    }

    return nResult;
}

qint32 XBinary::hexToInt32(QString sHex, bool bIsBigEndian)
{
    qint32 nResult=0;

    if((quint32)sHex.length()>=sizeof(qint32))
    {
        if(!bIsBigEndian)
        {
            sHex=invertHexByteString(sHex.mid(0,2*sizeof(qint32)));
        }

        bool bStatus=false;
        nResult=sHex.toInt(&bStatus,16);
    }

    return nResult;
}

quint64 XBinary::hexToUint64(QString sHex, bool bIsBigEndian)
{
    quint64 nResult=0;

    if((quint32)sHex.length()>=sizeof(quint64))
    {
        if(!bIsBigEndian)
        {
            sHex=invertHexByteString(sHex.mid(0,2*sizeof(quint64)));
        }

        bool bStatus=false;
        nResult=sHex.toULongLong(&bStatus,16);
    }

    return nResult;
}

qint64 XBinary::hexToInt64(QString sHex, bool bIsBigEndian)
{
    qint64 nResult=0;

    if((quint32)sHex.length()>=sizeof(qint64))
    {
        if(!bIsBigEndian)
        {
            sHex=invertHexByteString(sHex.mid(0,2*sizeof(qint64)));
        }

        bool bStatus=false;
        nResult=sHex.toLongLong(&bStatus,16);
    }

    return nResult;
}

QString XBinary::invertHexByteString(QString sHex)
{
    QString sResult;

    for(int i=sHex.length()-2; i>=0; i-=2)
    {
        sResult+=sHex.mid(i,2);
    }

    return sResult;
}

bool XBinary::isPlainTextType()
{
    bool bResult=false;

    QByteArray baData=read_array(0,qMin(getSize(),(qint64)0x1000));

    unsigned char *pDataOffset=(unsigned char *)baData.data();
    int nDataSize=baData.size();

    if(nDataSize)
    {
        bResult=true;

        for(int i=0; i<nDataSize; i++)
        {
            if(pDataOffset[i]<0x9)
            {
                bResult=false;
                break;
            }
        }
    }

    return bResult;
}

bool XBinary::isUTF8TextType()
{
    // EFBBBF
    bool bResult=false;

    QByteArray baData=read_array(0,qMin(getSize(),(qint64)3));

    unsigned char *pDataOffset=(unsigned char *)baData.data();
    int nDataSize=baData.size();

    if(nDataSize==3)
    {
        if((pDataOffset[0]==0xEF)&&(pDataOffset[1]==0xBB)&&(pDataOffset[2]==0xBF))
        {
            bResult=true;
        }
    }

    if(bResult)
    {
        baData=read_array(0,qMin(getSize(),(qint64)0x1000));
        unsigned char *pDataOffset=(unsigned char *)baData.data();
        pDataOffset+=3;

        nDataSize=baData.size();
        nDataSize=nDataSize-3;

        for(int i=0; i<nDataSize; i++)
        {
            if(pDataOffset[i]==0)
            {
                bResult=false;
                break;
            }
        }
    }

    return bResult;
}

XBinary::UNICODE_TYPE XBinary::getUnicodeType()
{
    XBinary::UNICODE_TYPE result=XBinary::UNICODE_TYPE_NONE;

    QByteArray baData=read_array(0,qMin(getSize(),(qint64)0x2));

    unsigned char *pDataOffset=(unsigned char *)baData.data();
    int nDataSize=baData.size();

    if(nDataSize)
    {
        quint16 nSymbol=*((quint16 *)(pDataOffset));

        nSymbol=qFromLittleEndian(nSymbol);

        if(nSymbol==0xFFFE)
        {
            result=UNICODE_TYPE_BE;
        }
        else if(nSymbol==0xFEFF)
        {
            result=UNICODE_TYPE_LE;
        }
        else
        {
            result=UNICODE_TYPE_NONE;
        }
    }
    // TODO 0 end

    return result;
}

bool XBinary::tryToOpen(QIODevice *pDevice)
{
    bool bResult=false;

    bResult=pDevice->open(QIODevice::ReadWrite);

    if(!bResult)
    {
        bResult=pDevice->open(QIODevice::ReadOnly);
    }

    return bResult;
}

bool XBinary::checkOffsetSize(XBinary::OFFSETSIZE os)
{
    return (os.nOffset)&&(os.nSize);
}

QString XBinary::get_uint32_version(quint32 nValue)
{
    return QString("%1.%2.%3")  .arg((nValue >> 16) & 0xff)
                                .arg((nValue >> 8) & 0xff)
                                .arg((nValue) & 0xff);
}

bool XBinary::isResizeEnable(QIODevice *pDevice)
{
    bool bResult=false;

    QString sClassName=pDevice->metaObject()->className();

    if(sClassName=="QFile")
    {
        bResult=true;
    }
    else if(sClassName=="QBuffer")
    {
        bResult=true;
    }

    return bResult;
}

bool XBinary::resize(QIODevice *pDevice, qint64 nSize)
{
    bool bResult=false;

    QString sClassName=pDevice->metaObject()->className();

    if(sClassName=="QFile")
    {
        bResult=((QFile *)pDevice)->resize(nSize);
    }
    else if(sClassName=="QBuffer")
    {
        ((QBuffer *)pDevice)->buffer().resize(nSize);
        bResult=true;
    }

    return bResult;
}

XBinary::ULEB128 XBinary::get_uleb128(qint64 nOffset)
{
    ULEB128 result={};

    quint32 nShift=0;

    while(true)
    {
        quint8 nByte=read_uint8(nOffset);
        result.nValue|=((nByte&0x7F)<<nShift);
        result.nByteSize++;
        nShift+=7;
        nOffset++;

        if((nByte&0x80)==0)
        {
            break;
        }
        // TODO more checks!
    }

    return result;
}

QList<QString> XBinary::getListFromFile(QString sFileName)
{
    QList<QString> listResult;

    QFile file;
    file.setFileName(sFileName);

    if(file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);
        while(!in.atEnd())
        {
            QString sLine=in.readLine().trimmed();
            if(sLine!="")
            {
                listResult.append(sLine);
            }
        }

        file.close();
    }

    return listResult;
}

QList<XBinary::SIGNATURE_RECORD> XBinary::getSignatureRecords(QString sSignature)
{
    QList<SIGNATURE_RECORD> result;

    int nSignatureSize=sSignature.size();

    for(int i=0; i<nSignatureSize;)
    {
        if(sSignature.at(i)==QChar('.'))
        {
            i+=_getSignatureRelOffsetFix(&result,sSignature,i);
        }
        else if(sSignature.at(i)==QChar('$'))
        {
            i+=_getSignatureRelOffset(&result,sSignature,i);
        }
        else if(sSignature.at(i)==QChar('#')) // mb TODO BaseAddress
        {
            i+=_getSignatureAddress(&result,sSignature,i);
        }
        else
        {
            int nBytes=_getSignatureBytes(&result,sSignature,i);

            if(nBytes)
            {
                i+=nBytes;
            }
            else
            {
                break;
            }
        }
    }

    return result;
}

bool XBinary::_compareSignature(QList<XBinary::SIGNATURE_RECORD> *pListSignatures, qint64 nOffset)
{
    for(int i=0; i<pListSignatures->count(); i++)
    {
        qint64 _nAddress=0;

        switch(pListSignatures->at(i).nType)
        {
            case XBinary::CompareBytes:
                {
                    QByteArray baData=read_array(nOffset,pListSignatures->at(i).baData.size());

                    if(baData.size()!=pListSignatures->at(i).baData.size())
                    {
                        return false;
                    }

                    if(!compareMemory(baData.data(),(char *)(pListSignatures->at(i).baData.data()),baData.size()))
                    {
                        return false;
                    }

                    nOffset+=baData.size();
                }
                break;

            case XBinary::RelOffsetFix:
                nOffset+=pListSignatures->at(i).nBaseAddress;
                break;

            case XBinary::RelOffset:
                _nAddress=offsetToAddress(nOffset);

                switch(pListSignatures->at(i).nSizeOfAddr)
                {
                    case 1:
                        _nAddress+=1+read_int8(nOffset);
                        break;

                    case 2:
                        _nAddress+=2+read_int16(nOffset); // TODO mb BE<->LE
                        break;

                    case 4:
                        _nAddress+=4+read_int32(nOffset); // TODO mb BE<->LE
                        break;

                    case 8:
                        _nAddress+=8+read_int64(nOffset); // TODO mb BE<->LE
                        break;
                }

                nOffset=addressToOffset(_nAddress);

                break;

            case XBinary::Address:
                switch(pListSignatures->at(i).nSizeOfAddr)
                {
                    case 1:
                        nOffset=read_uint8(nOffset);
                        break;

                    case 2:
                        nOffset=read_uint16(nOffset); // TODO mb BE<->LE
                        break;

                    case 4:
                        nOffset=read_uint32(nOffset); // TODO mb BE<->LE
                        break;

                    case 8:
                        nOffset=read_uint64(nOffset); // TODO mb BE<->LE
                        break;
                }

                nOffset=addressToOffset(nOffset-pListSignatures->at(i).nBaseAddress); // TODO!
                break;
        }

        if(!isOffsetValid(nOffset))
        {
            return false;
        }
    }
    //    CompareBytes,
    //    RelOffsetFix,
    //    RelOffset,
    //    Address

    return true;
}

int XBinary::_getSignatureRelOffsetFix(QList<XBinary::SIGNATURE_RECORD> *pListSignatures, QString sSignature, int nStartIndex)
{
    int nResult=0;
    int nSignatureSize=sSignature.size();

    for(int i=nStartIndex; i<nSignatureSize; i++)
    {
        if(sSignature.at(i)==QChar('.'))
        {
            nResult++;
        }
        else
        {
            break;
        }
    }

    if(nResult)
    {
        SIGNATURE_RECORD record;

        record.nType=XBinary::RelOffsetFix;
        record.nSizeOfAddr=0;
        record.nBaseAddress=nResult/2;

        pListSignatures->append(record);
    }

    return nResult;
}

int XBinary::_getSignatureRelOffset(QList<XBinary::SIGNATURE_RECORD> *pListSignatures, QString sSignature, int nStartIndex)
{
    int nResult=0;
    int nSignatureSize=sSignature.size();

    for(int i=nStartIndex; i<nSignatureSize; i++)
    {
        if(sSignature.at(i)==QChar('$'))
        {
            nResult++;
        }
        else
        {
            break;
        }
    }

    if(nResult)
    {
        SIGNATURE_RECORD record;

        record.nType=XBinary::RelOffset;
        record.nSizeOfAddr=nResult/2;
        record.nBaseAddress=0;

        pListSignatures->append(record);
    }

    return nResult;
}

int XBinary::_getSignatureAddress(QList<XBinary::SIGNATURE_RECORD> *pListSignatures, QString sSignature, int nStartIndex)
{
    int nResult=0;
    int nSignatureSize=sSignature.size();
    QString sBaseAddress;
    bool bIsBaseAddress=false;
    int nSizeOfAddress=0;

    for(int i=nStartIndex; i<nSignatureSize; i++)
    {
        if(sSignature.at(i)==QChar('#'))
        {
            nResult++;
            nSizeOfAddress++;
        }
        else if(sSignature.at(i)==QChar('['))
        {
            nResult++;
            bIsBaseAddress=true;
        }
        else if(sSignature.at(i)==QChar(']'))
        {
            nResult++;
            bIsBaseAddress=false;
        }
        else if(bIsBaseAddress)
        {
            nResult++;
            sBaseAddress.append(sSignature.at(i));
        }
        else
        {
            break;
        }
    }

    if(nResult)
    {
        SIGNATURE_RECORD record;

        record.nType=XBinary::Address;
        record.nSizeOfAddr=nSizeOfAddress/2;
        record.nBaseAddress=sBaseAddress.toInt(0,16);

        pListSignatures->append(record);
    }

    return nResult;
}

int XBinary::_getSignatureBytes(QList<XBinary::SIGNATURE_RECORD> *pListSignatures, QString sSignature, int nStartIndex)
{
    int nResult=0;
    int nSignatureSize=sSignature.size();
    QString sBytes;

    for(int i=nStartIndex; i<nSignatureSize; i++)
    {
        if( ((sSignature.at(i)>=QChar('a'))&&(sSignature.at(i)<=QChar('f'))) ||
            (((sSignature.at(i)>=QChar('0'))&&(sSignature.at(i)<=QChar('9')))))
        {
            nResult++;
            sBytes.append(sSignature.at(i));
        }
        else
        {
            // TODO: invalid symbols
            // Check
            break;
        }
    }

    if(nResult)
    {
        SIGNATURE_RECORD record;

        record.nType=XBinary::CompareBytes;
        record.nSizeOfAddr=0;
        record.nBaseAddress=0;
        QByteArray baData;
        baData.append(sBytes);
        record.baData=QByteArray::fromHex(baData);
        pListSignatures->append(record);
    }

    return nResult;
}

qint64 XBinary::getPhysSize(char *pBuffer, qint64 nSize)
{
    while(nSize>0)
    {
        char *pOffset=pBuffer+nSize-1;

        if(*pOffset)
        {
            break;
        }

        nSize--;
    }

    return nSize;
}

bool XBinary::isEmptyData(char *pBuffer, qint64 nSize) // TODO dwords
{
    bool bResult=true;

    for(qint64 i=0; i<nSize; i++)
    {
        char *pOffset=(pBuffer+i);

        if(*pOffset)
        {
            bResult=false;
            break;
        }
    }

    return bResult;
}

bool XBinary::_isOffsetValid(qint64 nOffset)
{
    qint64 nFileSize=getSize();
    
    return (nOffset<nFileSize);
}
