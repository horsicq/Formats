/* Copyright (c) 2017-2022 hors<horsicq@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "xbinary.h"

const double XBinary::D_ENTROPY_THRESHOLD=6.5;

XBinary::XBinary(QIODevice *pDevice,bool bIsImage,XADDR nModuleAddress)
{
    setData(pDevice,bIsImage,nModuleAddress);
}

XBinary::XBinary(QString sFileName)
{
    g_sFileName=sFileName;

    QFile *pFile=new QFile(sFileName);

    tryToOpen(pFile);

    setData(pFile);
}

XBinary::~XBinary()
{
    if(g_pMemory)
    {
        QFileDevice *pFileDevice=dynamic_cast<QFileDevice *>(g_pDevice);

        if(pFileDevice)
        {
            pFileDevice->unmap((unsigned char *)g_pMemory);
        }
    }

    if(g_sFileName!="")
    {
        QFile *pFile=dynamic_cast<QFile *>(g_pDevice);

        if(pFile)
        {
            pFile->close();
        }
    }
}

void XBinary::setData(QIODevice *pDevice,bool bIsImage,XADDR nModuleAddress)
{
    g_pReadWriteMutex=nullptr;
    g_pMemory=nullptr;

    setDevice(pDevice);
    setIsImage(bIsImage);
    XBinary::setBaseAddress(0);
    setModuleAddress(nModuleAddress);
    setEndianness(false); // LE
    XBinary::setEntryPointOffset(0);
    setMode(MODE_UNKNOWN);
    setFileType(FT_BINARY);
    setArch("NOEXEC");
    setVersion("");
    setType(TYPE_UNKNOWN);
    setOsType(OSNAME_UNKNOWN);
    setOsVersion("");

    if(pDevice)
    {
        setFileFormatSize(pDevice->size());
    }

    g_bLog=false;

#ifdef X_USE_MAP
    QFileDevice *pFileDevice=dynamic_cast<QFileDevice *>(pDevice);

    if(pFileDevice)
    {
        g_pMemory=(char *)pFileDevice->map(0,pFileDevice->size());
    }
#endif
}

void XBinary::setDevice(QIODevice *pDevice)
{
    g_pDevice=pDevice;

    if(g_pDevice)
    {
        if(g_pReadWriteMutex) g_pReadWriteMutex->lock();

        g_nSize=g_pDevice->size();

        if(g_pReadWriteMutex) g_pReadWriteMutex->unlock();
    }
}

void XBinary::setReadWriteMutex(QMutex *pReadWriteMutex)
{
    g_pReadWriteMutex=pReadWriteMutex;
}

qint64 XBinary::safeReadData(QIODevice *pDevice,qint64 nPos,char *pData,qint64 nMaxLen)
{
    qint64 nResult=0;

    if(g_pReadWriteMutex) g_pReadWriteMutex->lock();

    if(g_pMemory)
    {
        nMaxLen=qMin(getSize()-nPos,nMaxLen);
        nMaxLen=qMax(nMaxLen,(qint64)0);

        _copyMemory(pData,g_pMemory+nPos,nMaxLen);

        nResult=nMaxLen;
    }
    else
    {
        if(pDevice->seek(nPos))
        {
            nResult=pDevice->read(pData,nMaxLen);
        }
    }

    if(g_pReadWriteMutex) g_pReadWriteMutex->unlock();

    return nResult;
}

qint64 XBinary::safeWriteData(QIODevice *pDevice,qint64 nPos,const char *pData,qint64 nLen)
{
    qint64 nResult=0;

    if(g_pReadWriteMutex) g_pReadWriteMutex->lock();

    if(g_pMemory)
    {
        nLen=qMin(getSize()-nPos,nLen);
        nLen=qMax(nLen,(qint64)0);

        _copyMemory(g_pMemory+nPos,(char *)pData,nLen);

        nResult=nLen;
    }
    else
    {
        if(pDevice->seek(nPos))
        {
            nResult=pDevice->write(pData,nLen);
        }
    }

    if(g_pReadWriteMutex) g_pReadWriteMutex->unlock();

    return nResult;
}

qint64 XBinary::getSize()
{
    return g_nSize;
}

qint64 XBinary::getSize(QIODevice *pDevice)
{
    XBinary binary(pDevice);

    return binary.getSize();
}

qint64 XBinary::getSize(QString sFileName)
{
    qint64 nResult=0;

    QFile file;

    file.setFileName(sFileName);

    if(file.open(QIODevice::ReadOnly))
    {
        nResult=getSize(&file);

        file.close();
    }

    return nResult;
}

void XBinary::setMode(XBinary::MODE mode)
{
    g_mode=mode;
}

XBinary::MODE XBinary::getMode()
{
    return g_mode;
}

void XBinary::setType(qint32 nType)
{
    g_nType=nType;
}

qint32 XBinary::getType()
{
    return g_nType;
}

QString XBinary::typeIdToString(qint32 nType)
{
    QString sResult=tr("Unknown");

    switch(nType)
    {
        case TYPE_UNKNOWN:      sResult=tr("Unknown");
    }

    return sResult;
}

QString XBinary::getTypeAsString()
{
    return typeIdToString(getType());
}

void XBinary::setFileType(XBinary::FT fileType)
{
    g_fileType=fileType;
}

XBinary::FT XBinary::getFileType()
{
    return g_fileType;
}

QString XBinary::modeIdToString(XBinary::MODE mode)
{
    QString sResult=tr("Unknown");

    switch(mode)
    {
        case MODE_UNKNOWN:          sResult=tr("Unknown");          break;
        case MODE_DATA:             sResult=QString("Data");        break; // mb TODO translate
        case MODE_BIT:              sResult=QString("BIT");         break; // mb TODO translate
        case MODE_8:                sResult=tr("8-bit");            break;
        case MODE_16:               sResult=tr("16-bit");           break;
        case MODE_16SEG:            sResult=QString("16SEG");       break;
        case MODE_32:               sResult=tr("32-bit");           break;
        case MODE_64:               sResult=tr("64-bit");           break;
        case MODE_128:              sResult=tr("128-bit");          break;
    }

    return sResult;
}

QString XBinary::endiannessToString(bool bIsBigEndian)
{
    QString sResult;

    if(bIsBigEndian)
    {
        sResult="BE";
    }
    else
    {
        sResult="LE";
    }

    return sResult;
}

void XBinary::setArch(QString sArch)
{
    g_sArch=sArch;
}

QString XBinary::getArch()
{  
    return g_sArch;
}

void XBinary::setFileFormatName(QString sFileFormatName)
{
    g_sFileFormatName=sFileFormatName;
}

QString XBinary::getFileFormatName()
{
    // TODO baseFileName from pDevice
    return g_sFileFormatName;
}

void XBinary::setFileFormatSize(qint64 nFileFormatSize)
{
    g_nFileFormatSize=nFileFormatSize;
}

qint64 XBinary::getFileFormatSize()
{
    return g_nFileFormatSize;
}

bool XBinary::isSigned()
{
    return false;
}

XBinary::OFFSETSIZE XBinary::getSignOffsetSize()
{
    OFFSETSIZE osResult={};

    return osResult;
}

void XBinary::setOsType(OSNAME osName)
{
    g_osName=osName;
}

void XBinary::setOsVersion(QString sOsVersion)
{
    g_sOsVersion=sOsVersion;
}

XBinary::OSINFO XBinary::getOsInfo()
{
    OSINFO result={};

    result.osName=g_osName;
    result.sOsVersion=g_sOsVersion;
    result.sArch=getArch();
    result.mode=getMode();
    result.sType=typeIdToString(getType());
    result.bIsBigEndian=isBigEndian();

    return result;
}

void XBinary::setEndianness(bool bIsBigEndian)
{
    g_bIsBigEndian=bIsBigEndian;
}

bool XBinary::isPacked(double dEntropy)
{
    return (dEntropy>=D_ENTROPY_THRESHOLD);
}

quint8 XBinary::random8()
{
    return (quint8)random16();
}

quint16 XBinary::random16()
{
    quint16 nResult=0;

#if QT_VERSION >=QT_VERSION_CHECK(5,10,0)
    nResult=(quint16)(QRandomGenerator::global()->generate());
#elif (QT_VERSION_MAJOR>=6)
    nResult=(quint16)(QRandomGenerator::global()->generate());
#else
    static quint32 nSeed=0;

    if(!nSeed)
    {
        quint32 nRValue=QDateTime::currentMSecsSinceEpoch()&0xFFFFFFFF;

        nSeed^=nRValue;
        qsrand(nSeed);
    }
    nResult=(quint16)qrand();
#endif

    return nResult;
}

quint32 XBinary::random32()
{
    quint16 nValue1=random16();
    quint16 nValue2=random16();

    return (nValue1<<16)+nValue2;
}

quint64 XBinary::random64()
{
    quint64 nVal1=random32();
    quint64 nVal2=random32();

    nVal1=nVal1<<32;

    return nVal1+nVal2;
}

quint64 XBinary::random(quint64 nLimit)
{
    quint64 nResult=0;

    if(nLimit)
    {
        nResult=(random64())%nLimit;
    }

    return nResult;
}

QString XBinary::fileTypeIdToString(XBinary::FT fileType)
{
    QString sResult=tr("Unknown");

    switch(fileType)
    {
        case FT_UNKNOWN:            sResult=tr("Unknown");              break;
        case FT_REGION:             sResult=tr("Region");               break;
        case FT_BINARY:             sResult=QString("Binary");          break;
        case FT_BINARY16:           sResult=QString("Binary16");        break;
        case FT_BINARY32:           sResult=QString("Binary32");        break;
        case FT_BINARY64:           sResult=QString("Binary64");        break;
        case FT_COM:                sResult=QString("COM");             break;
        case FT_MSDOS:              sResult=QString("MSDOS");           break;
        case FT_NE:                 sResult=QString("NE");              break;
        case FT_LE:                 sResult=QString("LE");              break;
        case FT_LX:                 sResult=QString("LX");              break;
        case FT_PE:                 sResult=QString("PE");              break;
        case FT_PE32:               sResult=QString("PE32");            break;
        case FT_PE64:               sResult=QString("PE64");            break;
        case FT_ELF:                sResult=QString("ELF");             break;
        case FT_ELF32:              sResult=QString("ELF32");           break;
        case FT_ELF64:              sResult=QString("ELF64");           break;
        case FT_MACHO:              sResult=QString("Mach-O");          break;
        case FT_MACHO32:            sResult=QString("Mach-O32");        break;
        case FT_MACHO64:            sResult=QString("Mach-O64");        break;
        // Extra
        case FT_7Z:                 sResult=QString("7Z");              break;
        case FT_ANDROIDASRC:        sResult=QString("Android ASRC");    break;
        case FT_ANDROIDXML:         sResult=QString("Android XML");     break;
        case FT_APK:                sResult=QString("APK");             break;
        case FT_APKS:               sResult=QString("APKS");            break;
        case FT_AR:                 sResult=QString("ar");              break; // TODO DEB
        case FT_ARCHIVE:            sResult=tr("Archive");              break;
        case FT_CAB:                sResult=QString("CAB");             break;
        case FT_DEX:                sResult=QString("DEX");             break;
        case FT_DOCUMENT:           sResult=tr("Document");             break;
        case FT_GIF:                sResult=QString("GIF");             break;
        case FT_IMAGE:              sResult=tr("Image");                break;
        case FT_IPA:                sResult=QString("IPA");             break;
        case FT_JAR:                sResult=QString("JAR");             break;
        case FT_JPEG:               sResult=QString("JPEG");            break;
        case FT_MACHOFAT:           sResult=QString("Mach-O FAT");      break;
        case FT_PDF:                sResult=QString("PDF");             break;
        case FT_PLAINTEXT:          sResult=QString("Plain Text");      break; // mb TODO translate
        case FT_PNG:                sResult=QString("PNG");             break;
        case FT_RAR:                sResult=QString("RAR");             break;
        case FT_TEXT:               sResult=tr("Text");                 break;
        case FT_TIFF:               sResult=QString("TIFF");            break;
        case FT_UNICODE:            sResult=QString("Unicode");         break;
        case FT_UNICODE_BE:         sResult=QString("Unicode BE");      break;
        case FT_UNICODE_LE:         sResult=QString("Unicode LE");      break;
        case FT_UTF8:               sResult=QString("UTF8");            break;
        case FT_ZIP:                sResult=QString("ZIP");             break;
        case FT_GZIP:               sResult=QString("GZIP");            break;
    }

    return sResult;
}

QString XBinary::convertFileName(QString sFileName) // TODO Check
{
#ifdef Q_OS_MAC // Old Qt(4.X)
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

QString XBinary::convertPathName(QString sPathName)
{
    QString sResult=sPathName;

    // TODO more
    if(sPathName.contains("$app"))
    {
        sResult.replace("$app",QCoreApplication::applicationDirPath());
        sResult.replace("/",QDir::separator());
    }

    if(sPathName.contains("$data")) // TODO Check Linux
    {
#ifdef Q_OS_MAC
        sResult.replace("$data",QCoreApplication::applicationDirPath()+"/../Resources");
#else
        sResult.replace("$data",QCoreApplication::applicationDirPath());
#endif
        sResult.replace("/",QDir::separator());
    }

    return sResult;
}

XBinary::OS_STRING XBinary::getOsAnsiString(qint64 nOffset,qint64 nSize)
{
    OS_STRING result={};

    result.nOffset=nOffset;
    result.nSize=nSize;
    result.sString=read_ansiString(nOffset,nSize);

    return result;
}

void XBinary::findFiles(QString sDirectoryName,XBinary::FFOPTIONS *pFFOption,qint32 nLevel)
{
    *(pFFOption->pnNumberOfFiles)=pFFOption->pListFileNames->count();

    if(!(*pFFOption->pbIsStop))
    {
        QFileInfo fi(sDirectoryName);

        if(fi.isFile())
        {
            pFFOption->pListFileNames->append(fi.absoluteFilePath());
        }
        else if(fi.isDir()&&((pFFOption->bSubdirectories)||(nLevel==0)))
        {
            QDir dir(sDirectoryName);

            QFileInfoList eil=dir.entryInfoList();

            qint32 nNumberOfFiles=eil.count();

            for(qint32 i=0;(i<nNumberOfFiles)&&(!(*(pFFOption->pbIsStop)));i++)
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

void XBinary::findFiles(QString sDirectoryName,QList<QString> *pListFileNames)
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

        qint32 nNumberOfFiles=eil.count();

        for(qint32 i=0;i<nNumberOfFiles;i++)
        {
            QString sFN=eil.at(i).fileName();

            if((sFN!=".")&&(sFN!=".."))
            {
                findFiles(eil.at(i).absoluteFilePath(),pListFileNames);
            }
        }
    }
}

void XBinary::findFiles(QString sDirectoryName,QList<QString> *pListFileNames,bool bSubDirectories,qint32 nLevel,PDSTRUCT *pPdStruct)
{
    qint32 _nFreeIndex=0;

    if(nLevel==0)
    {
        _nFreeIndex=XBinary::getFreeIndex(pPdStruct);
        XBinary::setPdStructInit(pPdStruct,_nFreeIndex,0);
    }

    if(!(pPdStruct->bIsStop))
    {
        QFileInfo fi(sDirectoryName);

        if(fi.isFile())
        {
            pListFileNames->append(fi.absoluteFilePath());
        }
        else if(fi.isDir()&&((bSubDirectories)||(nLevel==0)))
        {
            QDir dir(sDirectoryName);

            QFileInfoList eil=dir.entryInfoList();

            qint32 nNumberOfFiles=eil.count();

            for(qint32 i=0;(i<nNumberOfFiles)&&(!(pPdStruct->bIsStop));i++)
            {
                QString sFN=eil.at(i).fileName();

                if((sFN!=".")&&(sFN!=".."))
                {
                    findFiles(eil.at(i).absoluteFilePath(),pListFileNames,bSubDirectories,nLevel+1,pPdStruct);
                }
            }
        }
    }

    if(nLevel==0)
    {
        XBinary::setPdStructFinished(pPdStruct,_nFreeIndex);
    }
}

QString XBinary::regExp(QString sRegExp,QString sString,qint32 nIndex)
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

bool XBinary::isRegExpPresent(QString sRegExp,QString sString)
{
    return (regExp(sRegExp,sString,0)!="");
}

qint64 XBinary::read_array(qint64 nOffset,char *pBuffer,qint64 nMaxSize)
{
    qint64 nResult=0;

    nResult=safeReadData(g_pDevice,nOffset,pBuffer,nMaxSize);  // Check for read large files

    return nResult;
}

QByteArray XBinary::read_array(qint64 nOffset, qint64 nSize)
{
    QByteArray baResult;

    XBinary::OFFSETSIZE osRegion=convertOffsetAndSize(nOffset,nSize);

    if(osRegion.nOffset!=-1)
    {
        baResult.resize((qint32)osRegion.nSize);

        qint64 nBytes=read_array(osRegion.nOffset,baResult.data(),osRegion.nSize);

        if(osRegion.nSize!=nBytes)
        {
            baResult.resize((qint32)nBytes);
        }
    }

    return baResult;
}

qint64 XBinary::write_array(qint64 nOffset,char *pBuffer,qint64 nSize)
{
    qint64 nResult=0;

    qint64 _nTotalSize=getSize();

    if((nSize<=(_nTotalSize-nOffset))&&(nOffset>=0))
    {
        nResult=safeWriteData(g_pDevice,nOffset,pBuffer,nSize);
    }

    return nResult;
}

qint64 XBinary::write_array(qint64 nOffset, QByteArray baData)
{
    return write_array(nOffset,baData.data(),baData.size());
}

QByteArray XBinary::read_array(QIODevice *pDevice,qint64 nOffset,qint64 nSize)
{
    XBinary binary(pDevice);

    return binary.read_array(nOffset,nSize);
}

qint64 XBinary::read_array(QIODevice *pDevice,qint64 nOffset,char *pBuffer,qint64 nSize)
{
    XBinary binary(pDevice);

    return binary.read_array(nOffset,pBuffer,nSize);
}

qint64 XBinary::write_array(QIODevice *pDevice,qint64 nOffset,char *pBuffer,qint64 nSize)
{
    XBinary binary(pDevice);

    return binary.write_array(nOffset,pBuffer,nSize);
}

qint64 XBinary::write_array(QIODevice *pDevice,qint64 nOffset,QByteArray baData)
{
    XBinary binary(pDevice);

    return binary.write_array(nOffset,baData);
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

quint16 XBinary::read_uint16(qint64 nOffset,bool bIsBigEndian)
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

qint16 XBinary::read_int16(qint64 nOffset,bool bIsBigEndian)
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

qint32 XBinary::read_int32(qint64 nOffset,bool bIsBigEndian)
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
    quint64 result=0;

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

qint64 XBinary::read_int64(qint64 nOffset,bool bIsBigEndian)
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

float XBinary::read_float16(qint64 nOffset, bool bIsBigEndian)
{
    // TODO Check
    quint16 float16_value=read_uint16(nOffset,bIsBigEndian);

    // MSB -> LSB
    // float16=1bit: sign, 5bit: exponent, 10bit: fraction
    // float32=1bit: sign, 8bit: exponent, 23bit: fraction
    // for normal exponent(1 to 0x1e): value=2**(exponent-15)*(1.fraction)
    // for denormalized exponent(0): value=2**-14*(0.fraction)
    quint32 sign = float16_value >> 15;
    quint32 exponent = (float16_value >> 10) & 0x1F;
    quint32 fraction = (float16_value & 0x3FF);
    quint32 float32_value=0;
    if (exponent == 0)
    {
        if (fraction == 0)
        {
          // zero
          float32_value = (sign << 31);
        }
        else
        {
          // can be represented as ordinary value in float32
          // 2 ** -14 * 0.0101
          // => 2 ** -16 * 1.0100
          // int int_exponent = -14;
          exponent = 127 - 14;
          while ((fraction & (1 << 10)) == 0)
          {
            //int_exponent--;
            exponent--;
            fraction <<= 1;
          }
          fraction &= 0x3FF;
          // int_exponent += 127;
          float32_value = (sign << 31) | (exponent << 23) | (fraction << 13);
        }
    }
    else if (exponent == 0x1F)
    {
        /* Inf or NaN */
        float32_value = (sign << 31) | (0xFF << 23) | (fraction << 13);
    }
    else
    {
        /* ordinary number */
        float32_value = (sign << 31) | ((exponent + (127-15)) << 23) | (fraction << 13);
    }

    return *((float*)&float32_value);
}

float XBinary::read_float(qint64 nOffset, bool bIsBigEndian)
{
    float result=0;

    read_array(nOffset,(char *)(&result),4);

    endian_float(&result,bIsBigEndian);

    return result;
}

double XBinary::read_double(qint64 nOffset,bool bIsBigEndian)
{
    double result=0;

    read_array(nOffset,(char *)(&result),8);

    endian_double(&result,bIsBigEndian);

    return result;
}

quint32 XBinary::read_uint24(qint64 nOffset,bool bIsBigEndian)
{
    quint32 result=0;

    if(bIsBigEndian)
    {
        read_array(nOffset,(char *)(&result)+1,3);
        result=qFromBigEndian(result);
    }
    else
    {
        read_array(nOffset,(char *)(&result)+0,3);
        result=qFromLittleEndian(result);
    }

    return (result&(0xFFFFFF));
}

qint32 XBinary::read_int24(qint64 nOffset, bool bIsBigEndian)
{
    return (qint32)(read_uint24(nOffset,bIsBigEndian));
}

qint64 XBinary::write_ansiString(qint64 nOffset,QString sString)
{
    return write_array(nOffset,sString.toLatin1().data(),sString.length()+1);
}

void XBinary::write_ansiStringFix(qint64 nOffset,qint64 nSize,QString sString)
{
    char *pBuffer=new char[nSize+1]; // mb TODO Check

    _zeroMemory(pBuffer,nSize+1);

    if(sString.size()>nSize)
    {
        sString.resize(nSize);
    }

    _copyMemory(pBuffer,sString.toLatin1().data(),sString.size());

    XBinary::write_array(nOffset,pBuffer,nSize);

    delete [] pBuffer;
}

QString XBinary::read_ansiString(qint64 nOffset,qint64 nMaxSize)
{
    QString sResult;

    if(nMaxSize>0x10000)
    {
        nMaxSize=0x10000;
    }

    if(nMaxSize>0)
    {
        quint8 *pBuffer=new quint8[nMaxSize+1];

        for(qint32 i=0;i<nMaxSize;i++)
        {
            pBuffer[i]=read_uint8(nOffset+i);

            if(pBuffer[i]==0)
            {
                break;
            }

            if(i==nMaxSize-1)
            {
                pBuffer[nMaxSize]=0;
            }
        }

        sResult.append((char *)pBuffer);

        delete [] pBuffer;
    }

    return sResult;
}

QString XBinary::read_unicodeString(qint64 nOffset,qint64 nMaxSize,bool bIsBigEndian)
{
    QString sResult;

    if((nMaxSize>0)&&(nMaxSize<0x10000))
    {
        quint16 *pBuffer=new quint16[nMaxSize+1];

        for(qint32 i=0;i<nMaxSize;i++)
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

QString XBinary::read_ucsdString(qint64 nOffset)
{
    QString sResult;

    quint8 nSize=read_uint8(nOffset);

    sResult=read_ansiString(nOffset+1,nSize);

    return sResult;
}

QString XBinary::read_utf8String(qint64 nOffset,qint64 nMaxSize)
{
    QString sResult;

    if(nMaxSize)
    {
        qint32 nRealSize=0;

        for(qint32 i=0;i<nMaxSize;i++)
        {
            quint8 nByte=read_uint8(nOffset+nRealSize);

            if(nByte==0)
            {
                break;
            }

            // TODO Check !!!
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

QString XBinary::_read_utf8String(qint64 nOffset,qint64 nMaxSize)
{
    QString sResult;

    PACKED_INT ulebSize=read_uleb128(nOffset,nMaxSize);

    sResult=read_utf8String(nOffset+ulebSize.nByteSize,ulebSize.nValue); // TODO mutf8

    return sResult;
}

QString XBinary::_read_utf8String(char *pData,qint64 nMaxSize)
{
    QString sResult;

    PACKED_INT ulebSize=_read_uleb128(pData,nMaxSize);

    qint32 nStringSize=qMin((qint64)ulebSize.nValue,nMaxSize-ulebSize.nByteSize);

    if(nStringSize>0)
    {
        sResult=QString::fromUtf8(pData+ulebSize.nByteSize,nStringSize);
    }

    return sResult;
}

QString XBinary::_read_utf8String(qint64 nOffset,char *pData,qint32 nDataSize,qint32 nDataOffset)
{
    QString sResult;

    if((nOffset>=nDataOffset)&&(nOffset<(nDataOffset+nDataSize)))
    {
        char *pStringData=pData+(nOffset-nDataOffset);
        qint32 nStringSize=nDataSize-(nOffset-nDataOffset);
        sResult=XBinary::_read_utf8String(pStringData,nStringSize);
    }

    return sResult;
}

QString XBinary::read_codePageString(qint64 nOffset,qint64 nMaxByteSize,QString sCodePage)
{
    QString sResult;

#if (QT_VERSION_MAJOR<6)||defined(QT_CORE5COMPAT_LIB)
    QByteArray baData=read_array(nOffset,nMaxByteSize);

    QTextCodec *pCodec=QTextCodec::codecForName(sCodePage.toLatin1().data());

    if(pCodec)
    {
        sResult=pCodec->toUnicode(baData);
    }
#endif

    return sResult;
}

bool XBinary::isUnicodeStringLatin(qint64 nOffset,qint64 nMaxSize,bool bIsBigEndian)
{
    bool bResult=true;

    if((nMaxSize>0)&&(nMaxSize<0x10000))
    {
        for(qint32 i=0;i<nMaxSize;i++)
        {
            quint16 nWord=read_uint16(nOffset+2*i,bIsBigEndian);

            if(nWord==0)
            {
                break;
            }
            else if(nWord>0x7F)
            {
                bResult=false;
                break;
            }
        }
    }

    return bResult;
}

void XBinary::write_uint8(qint64 nOffset,quint8 nValue)
{
    write_array(nOffset,(char *)(&nValue),1);
}

void XBinary::write_int8(qint64 nOffset,qint8 nValue)
{
    quint8 _value=(quint8)nValue;
    write_array(nOffset,(char *)(&_value),1);
}

void XBinary::write_uint16(qint64 nOffset,quint16 nValue,bool bIsBigEndian)
{
    if(bIsBigEndian)
    {
        nValue=qFromBigEndian(nValue);
    }
    else
    {
        nValue=qFromLittleEndian(nValue);
    }

    write_array(nOffset,(char *)(&nValue),2);
}

void XBinary::write_int16(qint64 nOffset,qint16 nValue,bool bIsBigEndian)
{
    quint16 _value=(quint16)nValue;

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

void XBinary::write_uint32(qint64 nOffset,quint32 nValue,bool bIsBigEndian)
{
    if(bIsBigEndian)
    {
        nValue=qFromBigEndian(nValue);
    }
    else
    {
        nValue=qFromLittleEndian(nValue);
    }

    write_array(nOffset,(char *)(&nValue),4);
}

void XBinary::write_int32(qint64 nOffset,qint32 nValue,bool bIsBigEndian)
{
    quint32 _value=(quint32)nValue;

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

void XBinary::write_uint64(qint64 nOffset,quint64 nValue,bool bIsBigEndian)
{
    if(bIsBigEndian)
    {
        nValue=qFromBigEndian(nValue);
    }
    else
    {
        nValue=qFromLittleEndian(nValue);
    }

    write_array(nOffset,(char *)(&nValue),8);
}

void XBinary::write_int64(qint64 nOffset,qint64 nValue,bool bIsBigEndian)
{
    quint64 _value=(quint64)nValue;

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

void XBinary::write_float16(qint64 nOffset,float fValue,bool bIsBigEndian)
{
    // TODO Check
    quint32 fltInt32=*(quint32 *)(&fValue);
    quint16 fltInt16=0;

    fltInt16 = (fltInt32 >> 31) << 5;
    quint16 tmp = (fltInt32 >> 23) & 0xff;
    tmp = (tmp - 0x70) & ((quint32)((qint32)(0x70 - tmp) >> 4) >> 27);
    fltInt16 = (fltInt16 | tmp) << 10;
    fltInt16 |= (fltInt32 >> 13) & 0x3ff;

    if(bIsBigEndian)
    {
        fltInt16=qFromBigEndian(fltInt16);
    }
    else
    {
        fltInt16=qFromLittleEndian(fltInt16);
    }

    write_array(nOffset,(char *)(&fltInt16),2);
}

void XBinary::write_float(qint64 nOffset,float fValue,bool bIsBigEndian)
{
    endian_float(&fValue,bIsBigEndian);

    write_array(nOffset,(char *)(&fValue),4);
}

void XBinary::write_double(qint64 nOffset,double dValue,bool bIsBigEndian)
{
    endian_double(&dValue,bIsBigEndian);

    write_array(nOffset,(char *)(&dValue),8);
}

QString XBinary::read_UUID_bytes(qint64 nOffset)
{
    // TODO check!
    // TODO Check Endian
    QString sResult=QString("%1-%2-%3-%4-%5").arg(
            read_array(nOffset+0,4).toHex().data(),
            read_array(nOffset+4,2).toHex().data(),
            read_array(nOffset+6,2).toHex().data(),
            read_array(nOffset+8,2).toHex().data(),
            read_array(nOffset+10,6).toHex().data());

    return sResult;
}

void XBinary::write_UUID_bytes(qint64 nOffset,QString sValue)
{
    // TODO Check
    sValue=sValue.remove("-");

    QByteArray baUUID=QByteArray::fromHex(sValue.toLatin1().data());

    write_array(nOffset,baUUID.data(),16);
}

QString XBinary::read_UUID(qint64 nOffset, bool bIsBigEndian)
{
    QString sResult=QString("%1-%2-%3-%4-%5").arg(
            valueToHex(read_uint32(nOffset+0,bIsBigEndian),bIsBigEndian),
            valueToHex(read_uint16(nOffset+4,bIsBigEndian),bIsBigEndian),
            valueToHex(read_uint16(nOffset+6,bIsBigEndian),bIsBigEndian),
            valueToHex(read_uint16(nOffset+8,bIsBigEndian),bIsBigEndian),
            read_array(nOffset+10,6).toHex().data());

    return sResult;
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

qint16 XBinary::_read_int16(char *pData,bool bIsBigEndian)
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

quint32 XBinary::_read_uint32(char *pData,bool bIsBigEndian)
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

quint64 XBinary::_read_uint64(char *pData,bool bIsBigEndian)
{
    quint64 result=*(quint64 *)pData;

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

qint64 XBinary::_read_int64(char *pData,bool bIsBigEndian)
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

QString XBinary::_read_ansiString(char *pData,qint32 nMaxSize)
{
    QString sResult;

    QByteArray baData(pData,nMaxSize);
    sResult.append(baData.data());

    return sResult;
}

QByteArray XBinary::_read_byteArray(char *pData,int nSize)
{
    return QByteArray(pData,nSize);
}

float XBinary::_read_float(char *pData,bool bIsBigEndian)
{
    float result=*(float *)pData;

    endian_float(&result,bIsBigEndian);

    return result;
}

double XBinary::_read_double(char *pData,bool bIsBigEndian)
{
    double result=*(double *)pData;

    endian_double(&result,bIsBigEndian);

    return result;
}

quint64 XBinary::_read_value(MODE mode,char *pData,bool bIsBigEndian)
{
    quint64 nResult=0;

    if(mode==MODE::MODE_8)
    {
        nResult=_read_uint8(pData);
    }
    else if(mode==MODE::MODE_16)
    {
        nResult=_read_uint16(pData,bIsBigEndian);
    }
    else if(mode==MODE::MODE_32)
    {
        nResult=_read_uint32(pData,bIsBigEndian);
    }
    else if(mode==MODE::MODE_64)
    {
        nResult=_read_uint64(pData,bIsBigEndian);
    }

    return nResult;
}

void XBinary::_write_uint8(char *pData,quint8 nValue)
{
    *(quint8 *)pData=nValue;
}

void XBinary::_write_int8(char *pData, qint8 nValue)
{
    *(qint8 *)pData=nValue;
}

void XBinary::_write_uint16(char *pData,quint16 nValue,bool bIsBigEndian)
{
    if(bIsBigEndian)
    {
        nValue=qToBigEndian(nValue);
    }
    else
    {
        nValue=qToLittleEndian(nValue);
    }

    *(quint16 *)pData=nValue;
}

void XBinary::_write_int16(char *pData,qint16 nValue,bool bIsBigEndian)
{
    if(bIsBigEndian)
    {
        nValue=qToBigEndian(nValue);
    }
    else
    {
        nValue=qToLittleEndian(nValue);
    }

    *(qint16 *)pData=nValue;
}

void XBinary::_write_uint32(char *pData,quint32 nValue,bool bIsBigEndian)
{
    if(bIsBigEndian)
    {
        nValue=qToBigEndian(nValue);
    }
    else
    {
        nValue=qToLittleEndian(nValue);
    }

    *(quint32 *)pData=nValue;
}

void XBinary::_write_int32(char *pData,qint32 nValue,bool bIsBigEndian)
{
    if(bIsBigEndian)
    {
        nValue=qToBigEndian(nValue);
    }
    else
    {
        nValue=qToLittleEndian(nValue);
    }

    *(qint32 *)pData=nValue;
}

void XBinary::_write_uint64(char *pData,quint64 nValue,bool bIsBigEndian)
{
    if(bIsBigEndian)
    {
        nValue=qToBigEndian(nValue);
    }
    else
    {
        nValue=qToLittleEndian(nValue);
    }

    *(quint64 *)pData=nValue;
}

void XBinary::_write_int64(char *pData,qint64 nValue,bool bIsBigEndian)
{
    if(bIsBigEndian)
    {
        nValue=qToBigEndian(nValue);
    }
    else
    {
        nValue=qToLittleEndian(nValue);
    }

    *(qint64 *)pData=nValue;
}

void XBinary::_write_float(char *pData,float fValue,bool bIsBigEndian)
{
    endian_float(&fValue,bIsBigEndian);

    *(float *)pData=fValue;
}

void XBinary::_write_double(char *pData,double dValue,bool bIsBigEndian)
{
    endian_double(&dValue,bIsBigEndian);

    *(double *)pData=dValue;
}

void XBinary::_write_value(MODE mode,char *pData,quint64 nValue,bool bIsBigEndian)
{
    if(mode==MODE::MODE_8)
    {
        _write_uint8(pData,nValue);
    }
    else if(mode==MODE::MODE_16)
    {
        _write_uint16(pData,nValue,bIsBigEndian);
    }
    else if(mode==MODE::MODE_32)
    {
        _write_uint32(pData,nValue,bIsBigEndian);
    }
    else if(mode==MODE::MODE_64)
    {
        _write_uint64(pData,nValue,bIsBigEndian);
    }
}

qint64 XBinary::find_array(qint64 nOffset,qint64 nSize,const char *pArray,qint64 nArraySize,PDSTRUCT *pProcessData)
{
    qint64 nResult=-1;

    PDSTRUCT processDataEmpty={};

    if(!pProcessData)
    {
        pProcessData=&processDataEmpty;
    }
    // TODO CheckSize function
    // TODO Optimize
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

    if(nOffset<0)
    {
        return -1;
    }

    qint64 nTemp=0;

    qint32 _nFreeIndex=XBinary::getFreeIndex(pProcessData);
    XBinary::setPdStructInit(pProcessData,_nFreeIndex,nSize);

    qint64 nStartOffset=nOffset;

    char *pBuffer=new char[READWRITE_BUFFER_SIZE+(nArraySize-1)];

    while((nSize>nArraySize-1)&&(!(pProcessData->bIsStop)))
    {
        nTemp=qMin((qint64)(READWRITE_BUFFER_SIZE+(nArraySize-1)),nSize);

        if(read_array(nOffset,pBuffer,nTemp)!=nTemp)
        {
            _errorMessage(tr("Read error"));

            break;
        }

        for(unsigned int i=0;i<nTemp-(nArraySize-1);i++)
        {
            if(compareMemory(pBuffer+i,pArray,nArraySize))
            {
                nResult=nOffset+i;

                break;
            }
        }

        nSize-=nTemp-(nArraySize-1);
        nOffset+=nTemp-(nArraySize-1);

        XBinary::setPdStructCurrent(pProcessData,_nFreeIndex,nOffset-nStartOffset);
    }

    delete[] pBuffer;

    setPdStructFinished(pProcessData,_nFreeIndex);

    return nResult;
}

qint64 XBinary::find_byteArray(qint64 nOffset,qint64 nSize,QByteArray baData,PDSTRUCT *pProcessData)
{
    return find_array(nOffset,nSize,baData.data(),baData.size(),pProcessData);
}

qint64 XBinary::find_uint8(qint64 nOffset,qint64 nSize,quint8 nValue,PDSTRUCT *pProcessData)
{
    return find_array(nOffset,nSize,(char *)&nValue,1,pProcessData);
}

qint64 XBinary::find_int8(qint64 nOffset,qint64 nSize,qint8 nValue,PDSTRUCT *pProcessData)
{
    return find_array(nOffset,nSize,(char *)&nValue,1,pProcessData);
}

qint64 XBinary::find_uint16(qint64 nOffset,qint64 nSize,quint16 nValue,bool bIsBigEndian,PDSTRUCT *pProcessData)
{
    if(bIsBigEndian)
    {
        nValue=qFromBigEndian(nValue);
    }
    else
    {
        nValue=qFromLittleEndian(nValue);
    }

    return find_array(nOffset,nSize,(char *)&nValue,2,pProcessData);
}

qint64 XBinary::find_int16(qint64 nOffset,qint64 nSize,qint16 nValue,bool bIsBigEndian,PDSTRUCT *pProcessData)
{
    quint16 _value=(quint16)nValue;

    if(bIsBigEndian)
    {
        _value=qFromBigEndian(_value);
    }
    else
    {
        _value=qFromLittleEndian(_value);
    }

    return find_array(nOffset,nSize,(char *)&_value,2,pProcessData);
}

qint64 XBinary::find_uint32(qint64 nOffset,qint64 nSize,quint32 nValue,bool bIsBigEndian,PDSTRUCT *pProcessData)
{
    if(bIsBigEndian)
    {
        nValue=qFromBigEndian(nValue);
    }
    else
    {
        nValue=qFromLittleEndian(nValue);
    }

    return find_array(nOffset,nSize,(char *)&nValue,4,pProcessData);
}

qint64 XBinary::find_int32(qint64 nOffset,qint64 nSize,qint32 nValue,bool bIsBigEndian,PDSTRUCT *pProcessData)
{
    quint32 _value=(quint32)nValue;

    if(bIsBigEndian)
    {
        _value=qFromBigEndian(_value);
    }
    else
    {
        _value=qFromLittleEndian(_value);
    }

    return find_array(nOffset,nSize,(char *)&_value,4,pProcessData);
}

qint64 XBinary::find_uint64(qint64 nOffset,qint64 nSize,quint64 nValue,bool bIsBigEndian,PDSTRUCT *pProcessData)
{
    if(bIsBigEndian)
    {
        nValue=qFromBigEndian(nValue);
    }
    else
    {
        nValue=qFromLittleEndian(nValue);
    }

    return find_array(nOffset,nSize,(char *)&nValue,8,pProcessData);
}

qint64 XBinary::find_int64(qint64 nOffset,qint64 nSize,qint64 nValue,bool bIsBigEndian,PDSTRUCT *pProcessData)
{
    quint64 _value=(quint64)nValue;

    if(bIsBigEndian)
    {
        _value=qFromBigEndian(_value);
    }
    else
    {
        _value=qFromLittleEndian(_value);
    }

    return find_array(nOffset,nSize,(char *)&_value,8,pProcessData);
}

qint64 XBinary::find_float(qint64 nOffset,qint64 nSize,float fValue,bool bIsBigEndian,PDSTRUCT *pProcessData)
{
    float _value=fValue;

    endian_float(&_value,bIsBigEndian);

    return find_array(nOffset,nSize,(char *)&_value,4,pProcessData);
}

qint64 XBinary::find_double(qint64 nOffset,qint64 nSize,double dValue,bool bIsBigEndian,PDSTRUCT *pProcessData)
{
    double _value=dValue;

    endian_double(&_value,bIsBigEndian);

    return find_array(nOffset,nSize,(char *)&_value,8,pProcessData);
}

void XBinary::endian_float(float *pValue,bool bIsBigEndian)
{
    bool bReverse=false;

#if Q_BYTE_ORDER==Q_BIG_ENDIAN
    bReverse=!bIsBigEndian;
#else
    bReverse=bIsBigEndian;
#endif

    if(bReverse)
    {
        qSwap(((quint8 *)pValue)[0],((quint8 *)pValue)[3]);
        qSwap(((quint8 *)pValue)[1],((quint8 *)pValue)[2]);
    }
}

void XBinary::endian_double(double *pValue,bool bIsBigEndian)
{
    bool bReverse=false;

#if Q_BYTE_ORDER==Q_BIG_ENDIAN
    bReverse=!bIsBigEndian;
#else
    bReverse=bIsBigEndian;
#endif

    if(bReverse)
    {
        qSwap(((quint8 *)pValue)[0],((quint8 *)pValue)[7]);
        qSwap(((quint8 *)pValue)[1],((quint8 *)pValue)[6]);
        qSwap(((quint8 *)pValue)[2],((quint8 *)pValue)[5]);
        qSwap(((quint8 *)pValue)[3],((quint8 *)pValue)[4]);
    }
}

qint64 XBinary::find_ansiString(qint64 nOffset,qint64 nSize,QString sString,PDSTRUCT *pProcessData)
{
    return find_array(nOffset,nSize,sString.toLatin1().data(),sString.size(),pProcessData);
}

qint64 XBinary::find_unicodeString(qint64 nOffset,qint64 nSize,QString sString,PDSTRUCT *pProcessData)
{
    return find_array(nOffset,nSize,(char *)sString.utf16(),sString.size()*2,pProcessData);
}

qint64 XBinary::find_utf8String(qint64 nOffset,qint64 nSize,QString sString,PDSTRUCT *pProcessData)
{
    QByteArray baData=sString.toUtf8();

    return find_array(nOffset,nSize,(char *)baData.data(),baData.size(),pProcessData);
}

qint64 XBinary::find_signature(qint64 nOffset,qint64 nSize,QString sSignature,qint64 *pnResultSize,PDSTRUCT *pProcessData)
{
    _MEMORY_MAP memoryMap=XBinary::getMemoryMap();

    return find_signature(&memoryMap,nOffset,nSize,sSignature,pnResultSize,pProcessData);
}

qint64 XBinary::find_signature(_MEMORY_MAP *pMemoryMap,qint64 nOffset,qint64 nSize,QString sSignature,qint64 *pnResultSize,PDSTRUCT *pProcessData)
{
    PDSTRUCT processDataEmpty={};

    if(!pProcessData)
    {
        pProcessData=&processDataEmpty;
    }

//    bool bDisableSignals=true;

//    if(pProcessData->bIsDisable) // If we call find_signature in another search function
//    {
//        bDisableSignals=false; // TODO Check !!!
//    }

    // TODO CheckSize function
    qint64 _nSize=getSize();

    qint64 nResultSize=0;

    if(pnResultSize==0)
    {
        pnResultSize=&nResultSize;
    }

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

    sSignature=convertSignature(sSignature);

    if(sSignature.contains("$")||sSignature.contains("#")||sSignature.contains("+"))
    {
        *pnResultSize=1;
    }
    else
    {
        *pnResultSize=sSignature.size()/2;
    }

    qint64 nResult=-1;

    if(sSignature.contains(".")||sSignature.contains("$")||sSignature.contains("#")||sSignature.contains("+"))
    {
        qint32 _nFreeIndex=XBinary::getFreeIndex(pProcessData);

        sSignature=convertSignature(sSignature);

        QList<SIGNATURE_RECORD> listSignatureRecords=getSignatureRecords(sSignature);

        if(listSignatureRecords.count()&&((listSignatureRecords.at(0).st==ST_COMPAREBYTES)||(listSignatureRecords.at(0).st==ST_FINDBYTES)))
        {
            XBinary::setPdStructInit(pProcessData,_nFreeIndex,nSize);

            QByteArray baFirst=listSignatureRecords.at(0).baData;

            char *pData=baFirst.data();
            qint32 nDataSize=baFirst.size();

            for(qint64 i=0;(i<nSize)&&(!(pProcessData->bIsStop));)
            {
                qint64 nTempOffset=find_array(nOffset+i,nSize-1,pData,nDataSize,pProcessData);

                if(nTempOffset!=-1)
                {
                    if(_compareSignature(pMemoryMap,&listSignatureRecords,nTempOffset))
                    {
                        nResult=nTempOffset;

                        break;
                    }
                }
                else
                {
                    break;
                }

                i=nTempOffset+nDataSize-nOffset;

                XBinary::setPdStructCurrent(pProcessData,_nFreeIndex,i);
            }
        }
        else
        {
            for(qint64 i=0;(i<nSize)&&(!(pProcessData->bIsStop));i++)
            {
                if(_compareSignature(pMemoryMap,&listSignatureRecords,nOffset+i))
                {
                    nResult=nOffset+i;
                    break;
                }

                XBinary::setPdStructCurrent(pProcessData,_nFreeIndex,i);
            }
        }

        XBinary::setPdStructFinished(pProcessData,_nFreeIndex);
    }
    else
    {
        QByteArray baData=QByteArray::fromHex(QByteArray(sSignature.toLatin1().data()));
        nResult=find_array(nOffset,nSize,baData.data(),baData.size(),pProcessData);
    }

    return nResult;
}

qint64 XBinary::find_ansiStringI(qint64 nOffset,qint64 nSize,QString sString,PDSTRUCT *pProcessData)
{
    PDSTRUCT processDataEmpty={};

    if(!pProcessData)
    {
        pProcessData=&processDataEmpty;
    }

    qint64 nResult=-1;
    // TODO CheckSize function
    qint64 nStringSize=sString.size();
    qint64 _nSize=getSize();

    if(nSize==-1)
    {
        nSize=_nSize-nOffset;
    }

    if((nSize>0)&&(nOffset+nSize<=_nSize)&&(nStringSize<=nSize))
    {
        qint32 _nFreeIndex=XBinary::getFreeIndex(pProcessData);

        XBinary::setPdStructInit(pProcessData,_nFreeIndex,nSize);

        qint64 nTemp=0;

        char *pBuffer=new char[READWRITE_BUFFER_SIZE+(nStringSize-1)];

        QByteArray baUpper=sString.toUpper().toLatin1();
        QByteArray baLower=sString.toLower().toLatin1();

        qint64 nStartOffset=nOffset;

        while((nSize>nStringSize-1)&&(!(pProcessData->bIsStop)))
        {
            nTemp=qMin((qint64)(READWRITE_BUFFER_SIZE+(nStringSize-1)),nSize);

            if(read_array(nOffset,pBuffer,nTemp)!=nTemp)
            {
                _errorMessage(tr("Read error"));

                break;
            }

            for(unsigned int i=0;i<nTemp-(nStringSize-1);i++)
            {
                if(compareMemoryByteI((quint8 *)(pBuffer+i),(quint8 *)baUpper.data(),(quint8 *)baLower.data(),nStringSize))
                {
                    nResult=nOffset+i;

                    break;
                }
            }

            if(nResult!=-1)
            {
                break;
            }

            nSize-=nTemp-(nStringSize-1);
            nOffset+=nTemp-(nStringSize-1);

            XBinary::setPdStructCurrent(pProcessData,_nFreeIndex,nOffset-nStartOffset);
        }

        XBinary::setPdStructFinished(pProcessData,_nFreeIndex);

        delete[] pBuffer;
    }

    return nResult;
}

qint64 XBinary::find_unicodeStringI(qint64 nOffset,qint64 nSize,QString sString,PDSTRUCT *pProcessData)
{
    qint64 nResult=-1;
    // TODO optimize
    PDSTRUCT processDataEmpty={};

    if(!pProcessData)
    {
        pProcessData=&processDataEmpty;
    }
    // TODO CheckSize function
    // TODO Optimize
    qint64 nStringSize=sString.size();
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

    if(nStringSize>nSize)
    {
        return -1;
    }

    qint32 _nFreeIndex=XBinary::getFreeIndex(pProcessData);

    XBinary::setPdStructInit(pProcessData,_nFreeIndex,nSize);

    qint64 nTemp=0;

    char *pBuffer=new char[READWRITE_BUFFER_SIZE+2*(nStringSize-1)];

    QByteArray baUpper=getUnicodeString(sString.toUpper());
    QByteArray baLower=getUnicodeString(sString.toLower());

    qint64 nStartOffset=nOffset;

    while((nSize>2*(nStringSize-1))&&(!(pProcessData->bIsStop)))
    {
        nTemp=qMin((qint64)(READWRITE_BUFFER_SIZE+2*(nStringSize-1)),nSize);

        if(read_array(nOffset,pBuffer,nTemp)!=nTemp)
        {
            _errorMessage(tr("Read error"));
            break;
        }

        for(unsigned int i=0;i<nTemp-2*(nStringSize-1);i++)
        {
            if(compareMemoryWordI((quint16 *)(pBuffer+i),(quint16 *)baUpper.data(),(quint16 *)baLower.data(),nStringSize))
            {
                nResult=nOffset+i;

                break;
            }
        }

        nSize-=nTemp-2*(nStringSize-1);
        nOffset+=nTemp-2*(nStringSize-1);

        XBinary::setPdStructCurrent(pProcessData,_nFreeIndex,nOffset-nStartOffset);
    }

    XBinary::setPdStructFinished(pProcessData,_nFreeIndex);

    delete[] pBuffer;

    return nResult;
}

qint64 XBinary::find_utf8StringI(qint64 nOffset,qint64 nSize,QString sString,PDSTRUCT *pProcessData)
{
    // TODO !!!
    return find_utf8String(nOffset,nSize,sString,pProcessData);
}

quint8 XBinary::getBits_uint8(quint8 nValue,qint32 nBitOffset,qint32 nBitSize)
{
    quint8 nResult=nValue;

    if(nBitSize)
    {
        quint8 nMask=0xFF;

        for(qint32 i=0;i<nBitOffset;i++)
        {
            nResult=nResult>>1;
        }

        for(qint32 i=0;i<nBitSize;i++)
        {
            nMask=nMask<<1;
        }

        nResult=nResult&(~nMask);
    }

    return nResult;
}

quint16 XBinary::getBits_uint16(quint16 nValue,qint32 nBitOffset,qint32 nBitSize)
{
    quint16 nResult=nValue;

    if(nBitSize)
    {
        quint16 nMask=0xFFFF;

        for(qint32 i=0;i<nBitOffset;i++)
        {
            nResult=nResult>>1;
        }

        for(qint32 i=0;i<nBitSize;i++)
        {
            nMask=nMask<<1;
        }

        nResult=nResult&(~nMask);
    }

    return nResult;
}

quint32 XBinary::getBits_uint32(quint32 nValue, qint32 nBitOffset, qint32 nBitSize)
{
    quint32 nResult=nValue;

    if(nBitSize)
    {
        quint32 nMask=0xFFFFFFFF;

        for(qint32 i=0;i<nBitOffset;i++)
        {
            nResult=nResult>>1;
        }

        for(qint32 i=0;i<nBitSize;i++)
        {
            nMask=nMask<<1;
        }

        nResult=nResult&(~nMask);
    }

    return nResult;
}

quint64 XBinary::getBits_uint64(quint64 nValue,qint32 nBitOffset,qint32 nBitSize)
{
    quint64 nResult=nValue;

    if(nBitSize)
    {
        quint64 nMask=0xFFFFFFFFFFFFFFFF;

        for(qint32 i=0;i<nBitOffset;i++)
        {
            nResult=nResult>>1;
        }

        for(qint32 i=0;i<nBitSize;i++)
        {
            nMask=nMask<<1;
        }

        nResult=nResult&(~nMask);
    }

    return nResult;
}

bool XBinary::_addMultiSearchStringRecord(QList<MS_RECORD> *pList,MS_RECORD *pRecord,STRINGSEARCH_OPTIONS *pSsOptions)
{
    bool bResult=false;

    bool bAdd=true;

    if(pSsOptions->bLinks)
    {
        bAdd=false;

        if(pRecord->sString.contains("http:")||pRecord->sString.contains("www.")||pRecord->sString.contains("mailto:"))
        {
            bAdd=true;
        }
    }

    if(bAdd)
    {
        pList->append(*pRecord);

        bResult=true;
    }

    return bResult;
}

QList<XBinary::MS_RECORD> XBinary::multiSearch_allStrings(qint64 nOffset,qint64 nSize,STRINGSEARCH_OPTIONS ssOptions,PDSTRUCT *pProcessData)
{
    PDSTRUCT processDataEmpty={};

    if(!pProcessData)
    {
        pProcessData=&processDataEmpty;
    }

    OFFSETSIZE osRegion=convertOffsetAndSize(nOffset,nSize);

    nOffset=osRegion.nOffset;
    nSize=osRegion.nSize;

    QList<XBinary::MS_RECORD> listResult;

    bool bFilter=(ssOptions.sExpFilter!="");

    if(ssOptions.nMinLenght==0)
    {
        ssOptions.nMinLenght=1;
    }

    if(ssOptions.nMaxLenght==0)
    {
        ssOptions.nMaxLenght=128; // TODO Check
    }

    bool bANSICodec=false;

    // TODO Check Qt6
#if (QT_VERSION_MAJOR<6)||defined(QT_CORE5COMPAT_LIB)
    QTextCodec *pCodec=nullptr;

    if(ssOptions.sANSICodec!="")
    {
        bANSICodec=true;
        pCodec=QTextCodec::codecForName(ssOptions.sANSICodec.toLatin1().data());
    }
#endif

    qint64 _nSize=nSize;
    qint64 _nOffset=nOffset;
    qint64 _nRawOffset=0;

    bool bReadError=false;

    char *pBuffer=nullptr;

    if(!g_pMemory)
    {
        pBuffer=new char[READWRITE_BUFFER_SIZE];
    }
    else
    {
        pBuffer=g_pMemory+nOffset;
    }

    char *pAnsiBuffer=new char[ssOptions.nMaxLenght+1];
    char *pUTF8Buffer=new char[ssOptions.nMaxLenght*4+1];

//    _zeroMemory(pUTF8Buffer,ssOptions.nMaxLenght*4);

    quint16 *pUnicodeBuffer[2]={new quint16[ssOptions.nMaxLenght+1],new quint16[ssOptions.nMaxLenght+1]};
    qint64 nCurrentUnicodeSize[2]={0,0};
    qint64 nCurrentUnicodeOffset[2]={0,0};

    qint64 nCurrentAnsiSize=0;
    qint64 nCurrentAnsiOffset=0;

    qint64 nCurrentUTF8Size=0;
    qint64 nCurrentUTF8Offset=0;
    qint32 nLastUTF8Width=0;
    qint64 nLastUTF8Offset=-1;

    bool bIsStart=true;
    char cPrevSymbol=0;

    qint32 _nFreeIndex=XBinary::getFreeIndex(pProcessData);

    XBinary::setPdStructInit(pProcessData,_nFreeIndex,nSize);

    qint32 nCurrentRecords=0;

    while((_nSize>0)&&(!(pProcessData->bIsStop)))
    {
        qint64 nCurrentSize=_nSize;

        if(!g_pMemory)
        {
            nCurrentSize=qMin((qint64)READWRITE_BUFFER_SIZE,nCurrentSize);

            if(safeReadData(g_pDevice,_nOffset,pBuffer,nCurrentSize)!=nCurrentSize)
            {
                bReadError=true;
                break;
            }
        }

        for(qint64 i=0;i<nCurrentSize;i++)
        {
            bool bIsEnd=((i==(nCurrentSize-1))&&(_nSize==nCurrentSize));
            qint32 nParity=(_nOffset+i)%2;

            char cSymbol=*(pBuffer+i);

            bool bIsAnsiSymbol=false;
            bool bIsUTF8Symbol=false;

            bool bNewUTF8String=false;
            bool bLongString=false;

            if(ssOptions.bAnsi)
            {
                bIsAnsiSymbol=isAnsiSymbol((quint8)cSymbol,bANSICodec);
            }

            if(ssOptions.bUTF8)
            {
                qint32 nUTF8SymbolWidth=0;

                bIsUTF8Symbol=isUTF8Symbol((quint8)cSymbol,&nUTF8SymbolWidth);

                if(bIsUTF8Symbol)
                {
                    if(nLastUTF8Offset==-1)
                    {
                        if(nUTF8SymbolWidth==0) // Cannot start with rest
                        {
                            bIsUTF8Symbol=false;
                        }
                        else
                        {
                            nLastUTF8Offset=_nOffset+i;
                            nLastUTF8Width=nUTF8SymbolWidth;
                        }
                    }
                    else
                    {
                        if(nUTF8SymbolWidth)
                        {
                            if(((_nOffset+i)-nLastUTF8Offset)<nLastUTF8Width)
                            {
                                bIsUTF8Symbol=false;
                                bNewUTF8String=true;
                            }

                            nLastUTF8Offset=_nOffset+i;
                            nLastUTF8Width=nUTF8SymbolWidth;
                        }
                    }
                }
            }

            if(bIsAnsiSymbol)
            {
                if(nCurrentAnsiSize==0)
                {
                    nCurrentAnsiOffset=_nOffset+i;
                }

                if(nCurrentAnsiSize<ssOptions.nMaxLenght)
                {
                    *(pAnsiBuffer+nCurrentAnsiSize)=cSymbol;
                }
                else
                {
                    bIsAnsiSymbol=false;
                    bLongString=true;
                }

                nCurrentAnsiSize++;
            }

            if(bIsUTF8Symbol)
            {
                if(nCurrentUTF8Size==0)
                {
                    nCurrentUTF8Offset=_nOffset+i;
                }

                if(nCurrentUTF8Size<ssOptions.nMaxLenght)
                {
                    *(pUTF8Buffer+nCurrentUTF8Size)=cSymbol;
                }
                else
                {
                    bIsUTF8Symbol=false;
                    bNewUTF8String=true;
                    bLongString=true;
                }

                nCurrentUTF8Size++;
            }

            if((!bIsAnsiSymbol)||(bIsEnd))
            {
                if(nCurrentAnsiSize>=ssOptions.nMinLenght)
                {
                    if(nCurrentAnsiSize-1<ssOptions.nMaxLenght)
                    {
                        pAnsiBuffer[nCurrentAnsiSize]=0;
                    }
                    else
                    {
                        pAnsiBuffer[ssOptions.nMaxLenght]=0;
                    }

                    if(ssOptions.bAnsi)
                    {
                        QString sString;

                        if(!bANSICodec)
                        {
                            sString=pAnsiBuffer;
                        }
                        else
                        {
                            // TODO Check Qt6
                            QByteArray baString=QByteArray(pAnsiBuffer,nCurrentAnsiSize);
                        #if (QT_VERSION_MAJOR<6)||defined(QT_CORE5COMPAT_LIB)
                            sString=pCodec->toUnicode(baString);
                        #else
                            sString=QString::fromLatin1(baString); // TODO
                        #endif
                        }

                        bool bAdd=true;

                        if(bFilter)
                        {
                            bAdd=isRegExpPresent(ssOptions.sExpFilter,sString);
//                            bAdd=sString.contains(ssOptions.sExpFilter,Qt::CaseInsensitive);
                        }

                        if(ssOptions.bCStrings&&cSymbol&&(!bLongString))
                        {
                            bAdd=false;
                        }

                        if(bAdd)
                        {
                            MS_RECORD record={};
                            record.recordType=MS_RECORD_TYPE_ANSI;
                            record.nOffset=nCurrentAnsiOffset;
                            record.nSize=nCurrentAnsiSize;
                            record.sString=sString;

                            if(_addMultiSearchStringRecord(&listResult,&record,&ssOptions))
                            {
                                nCurrentRecords++;
                            }

                            if(nCurrentRecords>=ssOptions.nLimit)
                            {
                                break;
                            }
                        }
                    }
                }

                nCurrentAnsiSize=0;
            }

            if((!bIsUTF8Symbol)||(bIsEnd))
            {
                if(nCurrentUTF8Size>=ssOptions.nMinLenght)
                {
                    pUTF8Buffer[nCurrentUTF8Size]=0;

                    if(ssOptions.bUTF8)
                    {
                        QString sString=QString::fromUtf8(pUTF8Buffer,-1);

                        qint32 nStringSize=sString.size();

                        bool bAdd=true;

                        if(bFilter)
                        {
                            bAdd=isRegExpPresent(ssOptions.sExpFilter,sString);
//                            bAdd=sString.contains(ssOptions.sExpFilter,Qt::CaseInsensitive);
                        }

                        if(ssOptions.bCStrings&&cSymbol&&(!bLongString))
                        {
                            bAdd=false;
                        }

                        if(nStringSize<ssOptions.nMinLenght)
                        {
                             bAdd=false;
                        }

                        if((nStringSize==nCurrentUTF8Size)&&(ssOptions.bAnsi))
                        {
                            bAdd=false;
                        }

                        if(bAdd)
                        {
                            MS_RECORD record={};
                            record.recordType=MS_RECORD_TYPE_UTF8;
                            record.nOffset=nCurrentUTF8Offset;
                            record.nSize=nCurrentUTF8Size;
                            record.sString=sString;

                            if(_addMultiSearchStringRecord(&listResult,&record,&ssOptions))
                            {
                                nCurrentRecords++;
                            }

                            if(nCurrentRecords>=ssOptions.nLimit)
                            {
                                break;
                            }
                        }
                    }
                }

                if(bNewUTF8String)
                {
                    *(pUTF8Buffer)=cSymbol;
                    nCurrentUTF8Offset=_nOffset+i;
                    nCurrentUTF8Size=1;
                }
                else
                {
                    nCurrentUTF8Size=0;
                    nLastUTF8Offset=-1;
                }
            }

            if(!bIsStart)
            {
                quint16 nCode=cPrevSymbol+(cSymbol<<8); // TODO BE/LE

                bool bIsUnicodeSymbol=false;

                if(ssOptions.bUnicode)
                {
                    bIsUnicodeSymbol=isUnicodeSymbol(nCode,true);
                }

                if(nCurrentUnicodeSize[nParity]>=ssOptions.nMaxLenght)
                {
                    bIsUnicodeSymbol=false;
                    bLongString=true;
                }

                if(bIsUnicodeSymbol)
                {
                    if(nCurrentUnicodeSize[nParity]==0)
                    {
                        nCurrentUnicodeOffset[nParity]=_nOffset-1+i;
                    }

                    if(nCurrentUnicodeSize[nParity]<ssOptions.nMaxLenght)
                    {
                        *(pUnicodeBuffer[nParity]+nCurrentUnicodeSize[nParity])=nCode;
                    }

                    nCurrentUnicodeSize[nParity]++;
                }

                if((!bIsUnicodeSymbol)||(bIsEnd))
                {
                    if(nCurrentUnicodeSize[nParity]>=ssOptions.nMinLenght)
                    {
                        if(nCurrentUnicodeSize[nParity]-1<ssOptions.nMaxLenght)
                        {
                            pUnicodeBuffer[nParity][nCurrentUnicodeSize[nParity]]=0;
                        }
                        else
                        {
                            pUnicodeBuffer[nParity][ssOptions.nMaxLenght]=0;
                        }

                        if(ssOptions.bUnicode)
                        {
                            QString sString=QString::fromUtf16(pUnicodeBuffer[nParity]);

                            bool bAdd=true;

                            if(bFilter)
                            {
                                bAdd=isRegExpPresent(ssOptions.sExpFilter,sString);
//                                bAdd=sString.contains(ssOptions.sExpFilter,Qt::CaseInsensitive);
                            }

                            if(ssOptions.bCStrings&&nCode&&(!bLongString))
                            {
                                bAdd=false;
                            }

                            if(bAdd)
                            {
                                MS_RECORD record={};
                                record.recordType=MS_RECORD_TYPE_UNICODE;
                                record.nOffset=nCurrentUnicodeOffset[nParity];
                                record.nSize=nCurrentUnicodeSize[nParity]*2;
                                record.sString=sString;

                                if(_addMultiSearchStringRecord(&listResult,&record,&ssOptions))
                                {
                                    nCurrentRecords++;
                                }

                                if(nCurrentRecords>=ssOptions.nLimit)
                                {
                                    break;
                                }
                            }
                        }
                    }

                    if(bIsEnd)
                    {
                        qint32 nO=(nParity==1)?(0):(1);

                        if(nCurrentUnicodeSize[nO]>=ssOptions.nMinLenght)
                        {
                            if(nCurrentUnicodeSize[nO]-1<ssOptions.nMaxLenght)
                            {
                                pUnicodeBuffer[nO][nCurrentUnicodeSize[nO]]=0;
                            }
                            else
                            {
                                pUnicodeBuffer[nO][ssOptions.nMaxLenght]=0;
                            }

                            if(ssOptions.bUnicode)
                            {
                                QString sString=QString::fromUtf16(pUnicodeBuffer[nO]);

                                bool bAdd=true;

                                if(bFilter)
                                {
                                    bAdd=isRegExpPresent(ssOptions.sExpFilter,sString);
//                                    bAdd=sString.contains(ssOptions.sExpFilter,Qt::CaseInsensitive);
                                }

                                if(bAdd)
                                {
                                    MS_RECORD record={};
                                    record.recordType=MS_RECORD_TYPE_UNICODE;
                                    record.nOffset=nCurrentUnicodeOffset[nO];
                                    record.nSize=nCurrentUnicodeSize[nO]*2;
                                    record.sString=sString;

                                    if(_addMultiSearchStringRecord(&listResult,&record,&ssOptions))
                                    {
                                        nCurrentRecords++;
                                    }

                                    if(nCurrentRecords>=ssOptions.nLimit)
                                    {
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    nCurrentUnicodeSize[nParity]=0;
                }
            }

            cPrevSymbol=cSymbol;

            if(bIsStart)
            {
                bIsStart=false;
            }
        }

        _nSize-=nCurrentSize;
        _nOffset+=nCurrentSize;
        _nRawOffset+=nCurrentSize;

        XBinary::setPdStructCurrent(pProcessData,_nFreeIndex,_nOffset-nOffset);

        if(nCurrentRecords>=ssOptions.nLimit)
        {
            _errorMessage(QString("%1: %2").arg(tr("Maximum"),QString::number(nCurrentRecords)));

            break;
        }
    }

    XBinary::setPdStructFinished(pProcessData,_nFreeIndex);

    if(bReadError)
    {
        _errorMessage(tr("Read error"));
    }

    if(!g_pMemory)
    {
        delete [] pBuffer;
    }

    delete [] pAnsiBuffer;
    delete [] pUTF8Buffer;
    delete [] pUnicodeBuffer[0];
    delete [] pUnicodeBuffer[1];

    return listResult;
}

QList<XBinary::MS_RECORD> XBinary::multiSearch_signature(qint64 nOffset,qint64 nSize,qint32 nLimit,QString sSignature,QString sInfo,PDSTRUCT *pProcessData)
{
    _MEMORY_MAP memoryMap=getMemoryMap();

    return multiSearch_signature(&memoryMap,nOffset,nSize,nLimit,sSignature,sInfo,pProcessData);
}

QList<XBinary::MS_RECORD> XBinary::multiSearch_signature(_MEMORY_MAP *pMemoryMap,qint64 nOffset,qint64 nSize,qint32 nLimit,QString sSignature,QString sInfo,PDSTRUCT *pProcessData)
{
    PDSTRUCT processDataEmpty={};

    if(!pProcessData)
    {
        pProcessData=&processDataEmpty;
    }

    QList<XBinary::MS_RECORD> listResult;

    qint64 _nSize=nSize;
    qint64 _nOffset=nOffset;

    qint32 _nFreeIndex=XBinary::getFreeIndex(pProcessData);

    XBinary::setPdStructInit(pProcessData,_nFreeIndex,nSize);

    qint32 nCurrentRecords=0;

    while((_nSize>0)&&(!(pProcessData->bIsStop)))
    {
        qint64 nSignatureSize=0;
        qint64 nSignatureOffset=find_signature(pMemoryMap,_nOffset,_nSize,sSignature,&nSignatureSize,pProcessData);

        if(nSignatureOffset==-1)
        {
            break;
        }

        MS_RECORD record={};
        record.recordType=MS_RECORD_TYPE_SIGNATURE;
        record.nOffset=nSignatureOffset;
        record.nSize=nSignatureSize;
        record.sString=sSignature;
        record.sInfo=sInfo;

        listResult.append(record);

        nCurrentRecords++;

        if(nCurrentRecords>=nLimit)
        {
            _errorMessage(QString("%1: %2").arg(tr("Maximum"),QString::number(nCurrentRecords)));

            break;
        }

        _nOffset=nSignatureOffset+nSignatureSize;
        _nSize=nSize-(_nOffset-nOffset);

        XBinary::setPdStructCurrent(pProcessData,_nFreeIndex,_nOffset-nOffset);
    }

    XBinary::setPdStructFinished(pProcessData,_nFreeIndex);

    return listResult;
}

QString XBinary::msRecordTypeIdToString(MS_RECORD_TYPE msRecordTypeId)
{
    QString sResult;

    if(msRecordTypeId==XBinary::MS_RECORD_TYPE_ANSI)
    {
        sResult="A";
    }
    else if(msRecordTypeId==XBinary::MS_RECORD_TYPE_UTF8)
    {
        sResult="UTF8";
    }
    else if(msRecordTypeId==XBinary::MS_RECORD_TYPE_UNICODE)
    {
        sResult="U";
    }

    return sResult;
}

QByteArray XBinary::getUnicodeString(QString sString)
{
    QByteArray baResult;

    qint32 nSize=sString.size();

    baResult.resize(nSize*2);

    baResult.fill(0);

    _copyMemory(baResult.data(),(char *)sString.utf16(),nSize*2);

    return baResult;
}

QByteArray XBinary::getStringData(MS_RECORD_TYPE msRecordTypeId, QString sString, bool bAddNull)
{
    QByteArray baResult;

    qint32 nSize=sString.size();

    char buffer[4]={};

    if(msRecordTypeId==MS_RECORD_TYPE_ANSI)
    {
        baResult=sString.toLatin1();

        if(bAddNull)
        {
            baResult.append(buffer,1);
        }
    }
    else if(msRecordTypeId==MS_RECORD_TYPE_UNICODE)
    {
        baResult.resize(nSize*2);

        baResult.fill(0);

        _copyMemory(baResult.data(),(char *)sString.utf16(),sString.size()*2);

        if(bAddNull)
        {
            baResult.append(buffer,2);
        }
    }
    else if(msRecordTypeId==MS_RECORD_TYPE_UTF8)
    {
        baResult=sString.toUtf8();

        if(bAddNull)
        {
            baResult.append(buffer,1);
        }
    }

    return baResult;
}

bool XBinary::isSignaturePresent(_MEMORY_MAP *pMemoryMap,qint64 nOffset,qint64 nSize,QString sSignature,PDSTRUCT *pProcessData)
{
    qint64 nResultSize=0;

    return (find_signature(pMemoryMap,nOffset,nSize,sSignature,&nResultSize,pProcessData)!=-1);
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

bool XBinary::isFileExists(QString sFileName, bool bTryToOpen)
{
    bool bResult=false;

    QFileInfo fi(sFileName);

    bResult=((fi.exists()&&fi.isFile()));

    if(bResult&&bTryToOpen)
    {
        QFile file(sFileName);

        bResult=tryToOpen(&file);

        file.close();
    }

    return bResult;
}

bool XBinary::removeFile(QString sFileName)
{
    return QFile::remove(sFileName);
}

bool XBinary::copyFile(QString sSrcFileName,QString sDestFileName)
{
    // mb TODO remove first
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

bool XBinary::moveFileToDirectory(QString sSrcFileName,QString sDestDirectory)
{
    QFileInfo fi(sSrcFileName);

    return moveFile(sSrcFileName,sDestDirectory+QDir::separator()+fi.fileName());
}

QString XBinary::convertFileNameSymbols(QString sFileName)
{
    sFileName=sFileName.replace("/","_");
    sFileName=sFileName.replace("\\","_");
    sFileName=sFileName.replace("?","_");
    sFileName=sFileName.replace("*","_");
    sFileName=sFileName.replace("\"","_");
    sFileName=sFileName.replace("<","_");
    sFileName=sFileName.replace(">","_");
    sFileName=sFileName.replace("|","_");
    sFileName=sFileName.replace(":","_");
    sFileName=sFileName.replace("\n","_");
    sFileName=sFileName.replace("\r","_");

    return sFileName;
}

QString XBinary::getBaseFileName(QString sFileName)
{
    QFileInfo fi(sFileName);

    return fi.baseName();
}

bool XBinary::createDirectory(QString sDirectoryName)
{
    return QDir().mkpath(sDirectoryName);
}

bool XBinary::isDirectoryExists(QString sDirectoryName)
{
    QFileInfo fi(sDirectoryName);

    return (fi.exists()&&fi.isDir());
}

bool XBinary::removeDirectory(QString sDirectoryName)
{
    QDir dir(sDirectoryName);

    return dir.removeRecursively();
}

bool XBinary::isDirectoryEmpty(QString sDirectoryName)
{
    return (QDir(sDirectoryName).entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count()==0);
}

QByteArray XBinary::readFile(QString sFileName)
{
    QByteArray baResult;

    QFile file;
    file.setFileName(sFileName);

    if(file.open(QIODevice::ReadOnly))
    {
        baResult=file.readAll();

        file.close();
    }

    return baResult;
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

void XBinary::_zeroMemory(char *pDest, qint64 nSize)
{
    // TODO optimize
    while(nSize)
    {
        *pDest=0;
        pDest++;
        nSize--;
    }
}

bool XBinary::_isMemoryZeroFilled(char *pDest, qint64 nSize)
{
    bool bResult=true;

    while(nSize)
    {
        if(*pDest)
        {
            bResult=false;

            break;
        }

        pDest++;
        nSize--;
    }

    return bResult;
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
    for(qint32 i=0;i<nSize;i++)
    {
        safeWriteData(g_pDevice,nOffset,(char *)&cZero,1);
    }

    return true;
}

bool XBinary::compareMemory(char *pMemory1,const char *pMemory2, qint64 nSize)
{
    while(nSize>0)
    {
        if(nSize>=4)
        {
            if(*((quint32 *)pMemory1)!=*((quint32 *)pMemory2))
            {
                return false;
            }

            pMemory1+=4;
            pMemory2+=4;
            nSize-=4;
        }
        else if(nSize>=2)
        {
            if(*((quint16 *)pMemory1)!=*((quint16 *)pMemory2))
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

bool XBinary::compareMemoryByteI(quint8 *pMemory, const quint8 *pMemoryU, const quint8 *pMemoryL, qint64 nSize)
{
    bool bResult=true;

    while(nSize>0)
    {
        if((*(pMemory)!=*(pMemoryU))&&(*(pMemory)!=*(pMemoryL)))
        {
            bResult=false;
            break;
        }

        pMemory++;
        pMemoryU++;
        pMemoryL++;
        nSize--;
    }

    return bResult;
}

bool XBinary::compareMemoryWordI(quint16 *pMemory, const quint16 *pMemoryU, const quint16 *pMemoryL, qint64 nSize)
{
    bool bResult=true;

    while(nSize>0)
    {
        if((*(pMemory)!=*(pMemoryU))&&(*(pMemory)!=*(pMemoryL)))
        {
            bResult=false;
            break;
        }

        pMemory++;
        pMemoryU++;
        pMemoryL++;
        nSize--;
    }

    return bResult;
}

bool XBinary::isOffsetValid(qint64 nOffset)
{
    _MEMORY_MAP memoryMap=getMemoryMap();

    return isOffsetValid(&memoryMap,nOffset);
}

bool XBinary::isAddressValid(XADDR nAddress)
{
    _MEMORY_MAP memoryMap=getMemoryMap();

    return isAddressValid(&memoryMap,nAddress);
}

bool XBinary::isRelAddressValid(qint64 nRelAddress)
{
    _MEMORY_MAP memoryMap=getMemoryMap();

    return isRelAddressValid(&memoryMap,nRelAddress);
}

XADDR XBinary::offsetToAddress(qint64 nOffset)
{
    _MEMORY_MAP memoryMap=getMemoryMap();

    return offsetToAddress(&memoryMap,nOffset);
}

qint64 XBinary::addressToOffset(quint64 nAddress)
{
    _MEMORY_MAP memoryMap=getMemoryMap();
    return addressToOffset(&memoryMap,nAddress);
}

XADDR XBinary::offsetToRelAddress(qint64 nOffset)
{
    _MEMORY_MAP memoryMap=getMemoryMap();

    return offsetToRelAddress(&memoryMap,nOffset);
}

qint64 XBinary::relAddressToOffset(qint64 nRelAddress)
{
    _MEMORY_MAP memoryMap=getMemoryMap();

    return relAddressToOffset(&memoryMap,nRelAddress);
}

bool XBinary::isOffsetValid(XBinary::_MEMORY_MAP *pMemoryMap, qint64 nOffset)
{
    bool bResult=false;

    if(pMemoryMap->nRawSize)
    {
        bResult=((nOffset>=0)&&(nOffset<pMemoryMap->nRawSize));
    }
    else
    {
        qint32 nNumberOfRecords=pMemoryMap->listRecords.count();

        for(qint32 i=0;i<nNumberOfRecords;i++)
        {
            if(pMemoryMap->listRecords.at(i).nSize&&(pMemoryMap->listRecords.at(i).nOffset!=-1))
            {
                if((pMemoryMap->listRecords.at(i).nOffset<=nOffset)&&(nOffset<pMemoryMap->listRecords.at(i).nOffset+pMemoryMap->listRecords.at(i).nSize))
                {
                    bResult=true;
                    break;
                }
            }
        }
    }

    return bResult;
}

bool XBinary::isOffsetAndSizeValid(XBinary::_MEMORY_MAP *pMemoryMap, XBinary::OFFSETSIZE *pOsRegion)
{
    return isOffsetAndSizeValid(pMemoryMap,pOsRegion->nOffset,pOsRegion->nSize);
}

bool XBinary::isOffsetAndSizeValid(qint64 nOffset, qint64 nSize)
{
    XBinary::_MEMORY_MAP memoryMap=getMemoryMap();

    return isOffsetAndSizeValid(&memoryMap,nOffset,nSize);
}

bool XBinary::isOffsetAndSizeValid(XBinary::_MEMORY_MAP *pMemoryMap, qint64 nOffset, qint64 nSize)
{
    bool bResult=false;

    if(nSize>0)
    {
        bool bValidOffset=isOffsetValid(pMemoryMap,nOffset);
        bool bValidSize=isOffsetValid(pMemoryMap,nOffset+nSize-1);
        bResult=bValidOffset&&bValidSize;
    }

    return bResult;
}

bool XBinary::isAddressValid(XBinary::_MEMORY_MAP *pMemoryMap, XADDR nAddress)
{
    bool bResult=false;

    if(pMemoryMap->nImageSize)
    {
        bResult=((pMemoryMap->nModuleAddress<=nAddress)&&(nAddress<(pMemoryMap->nModuleAddress+pMemoryMap->nImageSize)));
    }
    else
    {
        qint32 nNumberOfRecords=pMemoryMap->listRecords.count();

        for(qint32 i=0;i<nNumberOfRecords;i++)
        {
            if(pMemoryMap->listRecords.at(i).nSize&&(pMemoryMap->listRecords.at(i).nAddress!=(XADDR)-1))
            {
                if((pMemoryMap->listRecords.at(i).nAddress<=nAddress)&&(nAddress<pMemoryMap->listRecords.at(i).nAddress+pMemoryMap->listRecords.at(i).nSize))
                {
                    bResult=true;
                    break;
                }
            }
        }
    }

    return bResult;
}

bool XBinary::isRelAddressValid(XBinary::_MEMORY_MAP *pMemoryMap,qint64 nRelAddress)
{
    return isAddressValid(pMemoryMap,pMemoryMap->nModuleAddress+nRelAddress);
}

bool XBinary::isAddressPhysical(XBinary::_MEMORY_MAP *pMemoryMap, XADDR nAddress)
{
    qint64 nOffset=addressToOffset(pMemoryMap,nAddress);

    return (nOffset!=-1);
}

bool XBinary::isRelAddressPhysical(_MEMORY_MAP *pMemoryMap, XADDR nAddress)
{
    qint64 nOffset=relAddressToOffset(pMemoryMap,nAddress);

    return (nOffset!=-1);
}

XADDR XBinary::offsetToAddress(XBinary::_MEMORY_MAP *pMemoryMap, qint64 nOffset)
{
    XADDR nResult=-1;

    qint32 nNumberOfRecords=pMemoryMap->listRecords.count();

    for(qint32 i=0;i<nNumberOfRecords;i++)
    {
        if(pMemoryMap->listRecords.at(i).nSize&&(pMemoryMap->listRecords.at(i).nOffset!=-1)&&(pMemoryMap->listRecords.at(i).nAddress!=-1))
        {
            if((pMemoryMap->listRecords.at(i).nOffset<=nOffset)&&(nOffset<pMemoryMap->listRecords.at(i).nOffset+pMemoryMap->listRecords.at(i).nSize))
            {
                nResult=(nOffset-pMemoryMap->listRecords.at(i).nOffset)+pMemoryMap->listRecords.at(i).nAddress;
                break;
            }
        }
    }

    return nResult;
}

qint64 XBinary::addressToOffset(XBinary::_MEMORY_MAP *pMemoryMap, XADDR nAddress)
{
    qint64 nResult=-1;

//    if(pMemoryMap->mode==MODE_16) // Check COM Check 16SEG
//    {
//        if(nAddress>0xFFFF)
//        {
//            nAddress=((nAddress>>16)&0xFFFF)*16+(nAddress&0xFFFF);
//        }
//    }

    if(pMemoryMap->fileType==FT_MSDOS)
    {
        qint64 _nResult=((nAddress>>16)&0xFFFF)*16+(nAddress&0xFFFF);

        if(_nResult>=0x10000000)
        {
            _nResult-=0x10000000;
        }

        if(_nResult==0x100000)
        {
            _nResult=0;
        }

        nResult=_nResult+pMemoryMap->nSegmentBase;

        if(nResult>pMemoryMap->nRawSize)
        {
            nResult=-1;
        }
    }
    else
    {
        qint32 nNumberOfRecords=pMemoryMap->listRecords.count();

        for(qint32 i=0;i<nNumberOfRecords;i++)
        {
            if(pMemoryMap->listRecords.at(i).nSize&&(pMemoryMap->listRecords.at(i).nAddress!=-1)&&(pMemoryMap->listRecords.at(i).nOffset!=-1))
            {
                if((pMemoryMap->listRecords.at(i).nAddress<=nAddress)&&(nAddress<pMemoryMap->listRecords.at(i).nAddress+pMemoryMap->listRecords.at(i).nSize))
                {
                    nResult=(nAddress-pMemoryMap->listRecords.at(i).nAddress)+pMemoryMap->listRecords.at(i).nOffset;
                    break;
                }
            }
        }
    }

    return nResult;
}

qint64 XBinary::offsetToRelAddress(XBinary::_MEMORY_MAP *pMemoryMap, qint64 nOffset)
{
    qint64 nResult=offsetToAddress(pMemoryMap,nOffset);

    if(nResult!=-1)
    {
        nResult-=pMemoryMap->nModuleAddress;
    }

    return nResult;
}

qint64 XBinary::relAddressToOffset(XBinary::_MEMORY_MAP *pMemoryMap, qint64 nRelAddress)
{
    return addressToOffset(pMemoryMap,nRelAddress+pMemoryMap->nModuleAddress);
}

qint64 XBinary::relAddressToAddress(XBinary::_MEMORY_MAP *pMemoryMap, qint64 nRelAddress)
{
    qint64 nResult=-1;

    if(isRelAddressValid(pMemoryMap,nRelAddress))
    {
        nResult=nRelAddress+pMemoryMap->nModuleAddress;
    }

    return nResult;
}

qint64 XBinary::addressToRelAddress(XBinary::_MEMORY_MAP *pMemoryMap, XADDR nAddress)
{
    qint64 nResult=-1;

    if(isAddressValid(pMemoryMap,nAddress))
    {
        nResult=nAddress-=pMemoryMap->nModuleAddress;
    }

    return nResult;
}

XBinary::_MEMORY_RECORD XBinary::getMemoryRecordByOffset(XBinary::_MEMORY_MAP *pMemoryMap, qint64 nOffset)
{
    _MEMORY_RECORD result={};

    qint32 nNumberOfRecords=pMemoryMap->listRecords.count();

    for(qint32 i=0;i<nNumberOfRecords;i++)
    {
        if(pMemoryMap->listRecords.at(i).nSize&&(pMemoryMap->listRecords.at(i).nOffset!=-1))
        {
            if((pMemoryMap->listRecords.at(i).nOffset<=nOffset)&&(nOffset<pMemoryMap->listRecords.at(i).nOffset+pMemoryMap->listRecords.at(i).nSize))
            {
                result=pMemoryMap->listRecords.at(i);
                break;
            }
        }
    }

    return result;
}

XBinary::_MEMORY_RECORD XBinary::getMemoryRecordByAddress(XBinary::_MEMORY_MAP *pMemoryMap, XADDR nAddress)
{
    _MEMORY_RECORD result={};

    qint32 nNumberOfRecords=pMemoryMap->listRecords.count();

    for(qint32 i=0;i<nNumberOfRecords;i++)
    {
        if(pMemoryMap->listRecords.at(i).nSize&&(pMemoryMap->listRecords.at(i).nAddress!=-1))
        {
            if((pMemoryMap->listRecords.at(i).nAddress<=nAddress)&&(nAddress<pMemoryMap->listRecords.at(i).nAddress+pMemoryMap->listRecords.at(i).nSize))
            {
                result=pMemoryMap->listRecords.at(i);
                break;
            }
        }
    }

    return result;
}

XBinary::_MEMORY_RECORD XBinary::getMemoryRecordByRelAddress(XBinary::_MEMORY_MAP *pMemoryMap, qint64 nRelAddress)
{
    _MEMORY_RECORD result={};

    XADDR nAddress=relAddressToAddress(pMemoryMap,nRelAddress);

    if(nAddress!=-1)
    {
        getMemoryRecordByAddress(pMemoryMap,nAddress);
    }

    return result;
}

qint32 XBinary::addressToLoadSection(_MEMORY_MAP *pMemoryMap, XADDR nAddress)
{
    qint32 nResult=-1;

    _MEMORY_RECORD mm=getMemoryRecordByAddress(pMemoryMap,nAddress);

    if(mm.type==MMT_LOADSEGMENT)
    {
        nResult=mm.nLoadSection;
    }

    return nResult;
}

bool XBinary::isSolidAddressRange(XBinary::_MEMORY_MAP *pMemoryMap, quint64 nAddress, qint64 nSize)
{
    bool bResult=false;

    qint32 nIndex1=getMemoryRecordByAddress(pMemoryMap,nAddress).nIndex;
    qint32 nIndex2=getMemoryRecordByAddress(pMemoryMap,nAddress+nSize-1).nIndex;

    bResult=(nIndex1==nIndex2);

    return bResult;
}

QString XBinary::getMemoryRecordInfoByOffset(qint64 nOffset)
{
    _MEMORY_MAP memoryMap=getMemoryMap();

    return getMemoryRecordInfoByOffset(&memoryMap,nOffset);
}

QString XBinary::getMemoryRecordInfoByAddress(XADDR nAddress)
{
    _MEMORY_MAP memoryMap=getMemoryMap();

    return getMemoryRecordInfoByAddress(&memoryMap,nAddress);
}

QString XBinary::getMemoryRecordInfoByRelAddress(qint64 nRelAddress)
{
    _MEMORY_MAP memoryMap=getMemoryMap();

    return getMemoryRecordInfoByRelAddress(&memoryMap,nRelAddress);
}

QString XBinary::getMemoryRecordInfoByOffset(XBinary::_MEMORY_MAP *pMemoryMap, qint64 nOffset)
{
    XBinary::_MEMORY_RECORD memoryRecord=getMemoryRecordByOffset(pMemoryMap,nOffset);

    return getMemoryRecordName(&memoryRecord);
}

QString XBinary::getMemoryRecordInfoByAddress(XBinary::_MEMORY_MAP *pMemoryMap, XADDR nAddress)
{
    XBinary::_MEMORY_RECORD memoryRecord=getMemoryRecordByAddress(pMemoryMap,nAddress);

    return getMemoryRecordName(&memoryRecord);
}

QString XBinary::getMemoryRecordInfoByRelAddress(XBinary::_MEMORY_MAP *pMemoryMap, qint64 nRelAddress)
{
    XBinary::_MEMORY_RECORD memoryRecord=getMemoryRecordByAddress(pMemoryMap,nRelAddress+pMemoryMap->nModuleAddress);

    return getMemoryRecordName(&memoryRecord);
}

QString XBinary::getMemoryRecordName(XBinary::_MEMORY_RECORD *pMemoryRecord)
{
    QString sRecord;

    sRecord=pMemoryRecord->sName; // TODO

    return sRecord;
}

XBinary::_MEMORY_MAP XBinary::getMemoryMap()
{
    _MEMORY_MAP result={};

    qint64 nTotalSize=getSize();

    result.nModuleAddress=getModuleAddress();
    result.nRawSize=nTotalSize;
    result.nImageSize=nTotalSize;
    result.fileType=getFileType();
    result.mode=getMode();
    result.sArch=getArch();
    result.bIsBigEndian=isBigEndian();
    result.sType=getTypeAsString();

    _MEMORY_RECORD record={};
    record.nAddress=result.nModuleAddress;
    record.segment=ADDRESS_SEGMENT_FLAT;
    record.nOffset=0;
    record.nSize=nTotalSize;
    record.nIndex=0;

    result.listRecords.append(record);

    return result;
}

qint32 XBinary::getNumberOfPhysicalRecords(XBinary::_MEMORY_MAP *pMemoryMap)
{
    qint32 nResult=0;

    qint32 nNumberOfRecords=pMemoryMap->listRecords.count();

    for(qint32 i=0;i<nNumberOfRecords;i++)
    {
        if(!pMemoryMap->listRecords.at(i).bIsVirtual)
        {
            nResult++;
        }
    }

    return nResult;
}

XADDR XBinary::getBaseAddress()
{
    return this->g_nBaseAddress;
}

void XBinary::setBaseAddress(XADDR nBaseAddress)
{
    this->g_nBaseAddress=nBaseAddress;
}

qint64 XBinary::getImageSize()
{
    return getSize();
}

bool XBinary::isImage()
{
    return g_bIsImage;
}

void XBinary::setIsImage(bool bValue)
{
    g_bIsImage=bValue;
}

bool XBinary::compareSignature(QString sSignature, qint64 nOffset)
{
    _MEMORY_MAP memoryMap=getMemoryMap();

    return compareSignature(&memoryMap,sSignature,nOffset);
}

bool XBinary::compareSignature(_MEMORY_MAP *pMemoryMap,QString sSignature, qint64 nOffset)
{
    bool bResult=false;

    QString sOrigin=sSignature;

    sSignature=convertSignature(sSignature);

    QList<SIGNATURE_RECORD> listSignatureRecords=getSignatureRecords(sSignature);

    if(listSignatureRecords.count())
    {
        bResult=_compareSignature(pMemoryMap,&listSignatureRecords,nOffset);
    }
    else
    {
        emit errorMessage(QString("%1: %2").arg(tr("Invalid signature"),sOrigin));
    }

    return bResult;
}

bool XBinary::_compareByteArrayWithSignature(QByteArray baData, QString sSignature)
{
    bool bResult=false;

    QString sHex=baData.toHex().data();

    if(sHex.size()==sSignature.size())
    {
        bResult=true;

        qint32 nNumberOfSymbols=sSignature.size();

        for(qint32 i=0;i<nNumberOfSymbols;i++)
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

    qint32 nSize=sSignature1.size();

    if(sSignature1.size()==sSignature2.size())
    {
        for(qint32 i=0;i<nSize;i+=2)
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

bool XBinary::compareSignatureOnAddress(QString sSignature, XADDR nAddress)
{
    XBinary::_MEMORY_MAP memoryMap=getMemoryMap();

    return compareSignatureOnAddress(&memoryMap,sSignature,nAddress);
}

bool XBinary::compareSignatureOnAddress(XBinary::_MEMORY_MAP *pMemoryMap, QString sSignature, XADDR nAddress)
{
    bool bResult=false;

    qint64 nOffset=addressToOffset(pMemoryMap,nAddress);

    if(nOffset!=-1)
    {
        bResult=compareSignature(pMemoryMap,sSignature,nOffset);
    }

    return bResult;
}

qint64 XBinary::_getEntryPointOffset()
{
    XBinary::_MEMORY_MAP memoryMap=getMemoryMap();

    return getEntryPointOffset(&memoryMap);
}

qint64 XBinary::getEntryPointOffset(_MEMORY_MAP *pMemoryMap)
{
    return addressToOffset(pMemoryMap,pMemoryMap->nEntryPointAddress);
}

void XBinary::setEntryPointOffset(qint64 nEntryPointOffset)
{
    this->g_nEntryPointOffset=nEntryPointOffset;
}

XADDR XBinary::getEntryPointAddress()
{
    XBinary::_MEMORY_MAP memoryMap=getMemoryMap();

    return getEntryPointAddress(&memoryMap);
}

XADDR XBinary::getEntryPointAddress(XBinary::_MEMORY_MAP *pMemoryMap)
{
    return pMemoryMap->nEntryPointAddress;
}

qint64 XBinary::getEntryPointRVA()
{
    XBinary::_MEMORY_MAP memoryMap=getMemoryMap();

    return getEntryPointRVA(&memoryMap);
}

qint64 XBinary::getEntryPointRVA(_MEMORY_MAP *pMemoryMap)
{
    return (pMemoryMap->nEntryPointAddress)-pMemoryMap->nModuleAddress;
}

XADDR XBinary::getLowestAddress(XBinary::_MEMORY_MAP *pMemoryMap)
{
    XADDR nResult=-1;

    qint32 nNumberOfRecords=pMemoryMap->listRecords.count();

    for(qint32 i=0;i<nNumberOfRecords;i++)
    {
        if(pMemoryMap->listRecords.at(i).nAddress!=-1)
        {
            if(nResult==-1)
            {
                nResult=pMemoryMap->listRecords.at(i).nAddress;
            }

            nResult=qMin(pMemoryMap->listRecords.at(i).nAddress,nResult);
        }
    }

    return nResult;
}

qint64 XBinary::getTotalVirtualSize(XBinary::_MEMORY_MAP *pMemoryMap)
{
    qint64 nResult=0;

    qint32 nNumberOfRecords=pMemoryMap->listRecords.count();

    for(qint32 i=0;i<nNumberOfRecords;i++)
    {
        if(pMemoryMap->listRecords.at(i).type!=MMT_OVERLAY) // TODO Check ELF, MachO -1
        {
            nResult+=pMemoryMap->listRecords.at(i).nSize;
        }
    }

    return nResult;
}

quint64 XBinary::positionToVirtualAddress(_MEMORY_MAP *pMemoryMap,qint64 nPosition)
{
    XADDR nResult=-1;

    qint32 nNumberOfRecords=pMemoryMap->listRecords.count();

    qint64 _nSize=0;

    for(qint32 i=0;i<nNumberOfRecords;i++)
    {
        if((_nSize<=nPosition)&&(nPosition<_nSize+pMemoryMap->listRecords.at(i).nSize))
        {
            nResult=pMemoryMap->listRecords.at(i).nAddress+(nPosition-_nSize);
        }

       _nSize+=pMemoryMap->listRecords.at(i).nSize;
    }

    return nResult;
}

void XBinary::setModuleAddress(quint64 nValue)
{
    this->g_nModuleAddress=nValue;
}

XADDR XBinary::getModuleAddress()
{
    XADDR nResult=0;

    if(g_nModuleAddress!=(XADDR)-1)
    {
        nResult=g_nModuleAddress;
    }
    else
    {
        nResult=getBaseAddress();
    }

    return nResult;
}

bool XBinary::compareEntryPoint(QString sSignature, qint64 nOffset)
{
    XBinary::_MEMORY_MAP memoryMap=getMemoryMap();

    return compareEntryPoint(&memoryMap,sSignature,nOffset);
}

bool XBinary::compareEntryPoint(XBinary::_MEMORY_MAP *pMemoryMap, QString sSignature, qint64 nOffset)
{
    qint64 nEPOffset=getEntryPointOffset(pMemoryMap)+nOffset;

    return compareSignature(pMemoryMap,sSignature,nEPOffset);
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

bool XBinary::dumpToFile(QString sFileName, qint64 nDataOffset, qint64 nDataSize, PDSTRUCT *pProcessData)
{
    bool bResult=false;

    PDSTRUCT processDataEmpty={};

    if(!pProcessData)
    {
        pProcessData=&processDataEmpty;
    }

    // TODO convert -1 to fileSize

    QFile file;
    file.setFileName(sFileName);

    if(file.open(QIODevice::ReadWrite))
    {
        file.resize(0);

        char *pBuffer=new char[0x1000]; // TODO const

        qint64 nSourceOffset=nDataOffset;
        qint64 nDestOffset=0;

        qint32 _nFreeIndex=XBinary::getFreeIndex(pProcessData);

        XBinary::setPdStructInit(pProcessData,_nFreeIndex,nDataSize);

        bResult=true;

        while((nDataSize>0)&&(!(pProcessData->bIsStop)))
        {
            qint64 nTempSize=qMin(nDataSize,(qint64)0x1000); // TODO const

            if(safeReadData(g_pDevice,nSourceOffset,pBuffer,nTempSize)!=nTempSize)
            {
                _errorMessage(QObject::tr("Read error"));
                bResult=false;
                break;
            }

            if(!((file.seek(nDestOffset))&&(file.write(pBuffer,nTempSize)==nTempSize)))
            {
                _errorMessage(QObject::tr("Write error"));
                bResult=false;
                break;
            }

            nSourceOffset+=nTempSize;
            nDestOffset+=nTempSize;

            nDataSize-=nTempSize;

            XBinary::setPdStructCurrent(pProcessData,_nFreeIndex,nDestOffset);
        }

        XBinary::setPdStructFinished(pProcessData,_nFreeIndex);

        delete [] pBuffer;

        file.close();
    }
    else
    {
        _errorMessage(QString("%1: %2").arg(QObject::tr("Cannot open file"),sFileName));
    }

    return bResult;
}

QSet<XBinary::FT> XBinary::getFileTypes(bool bExtra)
{
    QSet<XBinary::FT> stResult;

    stResult.insert(FT_BINARY);

    QByteArray baHeader;
    baHeader=read_array(0,qMin(getSize(),(qint64)0x200)); // TODO const
    char *pOffset=baHeader.data();
    quint32 nSize=getSize();

    if(nSize>=(int)sizeof(XMSDOS_DEF::IMAGE_DOS_HEADEREX))
    {
        if( (((XMSDOS_DEF::IMAGE_DOS_HEADEREX *)pOffset)->e_magic==XMSDOS_DEF::S_IMAGE_DOS_SIGNATURE_MZ)||
            (((XMSDOS_DEF::IMAGE_DOS_HEADEREX *)pOffset)->e_magic==XMSDOS_DEF::S_IMAGE_DOS_SIGNATURE_ZM))
        {
            stResult.insert(FT_MSDOS);
            // TODO rewrite for NE, LE
            quint32 nLfanew=((XMSDOS_DEF::IMAGE_DOS_HEADEREX *)pOffset)->e_lfanew;
            quint32 nHeaderSize=baHeader.size()-sizeof(XPE_DEF::IMAGE_NT_HEADERS32);
            QByteArray baNewHeader;

            bool bIsNewHeaderValid=false;

            if((nLfanew<nHeaderSize)&&((quint32)baHeader.size()>(nLfanew+sizeof(XPE_DEF::IMAGE_NT_HEADERS32)))) // TODO do not use IMAGE_NT_HEADERS32
            {
                pOffset+=nLfanew;
                bIsNewHeaderValid=true;
            }
            else
            {
                qint64 nNtHeadersSize=4+sizeof(XPE_DEF::IMAGE_FILE_HEADER);

                baNewHeader=read_array(nLfanew,nNtHeadersSize);

                nHeaderSize=baNewHeader.size();

                if(nHeaderSize==nNtHeadersSize)
                {
                    pOffset=baNewHeader.data();
                    bIsNewHeaderValid=true;
                }
            }

            if(bIsNewHeaderValid)
            {
                if((((XPE_DEF::IMAGE_NT_HEADERS32 *)pOffset))->Signature==XPE_DEF::S_IMAGE_NT_SIGNATURE)
                {
                    stResult.insert(FT_PE);

                    quint16 nMachine=((XPE_DEF::IMAGE_NT_HEADERS32 *)pOffset)->FileHeader.Machine;

                    // TODO more
                    if( (nMachine==XPE_DEF::S_IMAGE_FILE_MACHINE_AMD64)||
                        (nMachine==XPE_DEF::S_IMAGE_FILE_MACHINE_IA64)||
                        (nMachine==XPE_DEF::S_IMAGE_FILE_MACHINE_ARM64))
                    {
                        stResult.insert(FT_PE64);
                    }
                    else
                    {
                        stResult.insert(FT_PE32);
                    }
                }
                else if((((XNE_DEF::IMAGE_OS2_HEADER *)pOffset))->ne_magic==XNE_DEF::S_IMAGE_OS2_SIGNATURE)
                {
                    stResult.insert(FT_NE);
                }
                else if((((XPE_DEF::IMAGE_NT_HEADERS32 *)pOffset))->Signature==XLE_DEF::S_IMAGE_VXD_SIGNATURE)
                {
                    stResult.insert(FT_LE);
                }
                else if((((XPE_DEF::IMAGE_NT_HEADERS32 *)pOffset))->Signature==XLE_DEF::S_IMAGE_LX_SIGNATURE)
                {
                    stResult.insert(FT_LX);
                }
            }
        }
    }

    if(nSize>=(int)sizeof(XELF_DEF::Elf32_Ehdr))
    {
        if( (((XELF_DEF::Elf32_Ehdr *)pOffset)->e_ident[0]==0x7f) &&
            (((XELF_DEF::Elf32_Ehdr *)pOffset)->e_ident[1]=='E') &&
            (((XELF_DEF::Elf32_Ehdr *)pOffset)->e_ident[2]=='L') &&
            (((XELF_DEF::Elf32_Ehdr *)pOffset)->e_ident[3]=='F'))
        {
            stResult.insert(FT_ELF);

            if(((XELF_DEF::Elf32_Ehdr *)pOffset)->e_ident[4]==1)
            {
                stResult.insert(FT_ELF32);
            }
            else if(((XELF_DEF::Elf32_Ehdr *)pOffset)->e_ident[4]==2)
            {
                stResult.insert(FT_ELF64);
            }
            // mb TODO another e_ident[4]
        }
    }

    if(nSize>=(int)sizeof(XMACH_DEF::mach_header))
    {
        if((((XMACH_DEF::mach_header *)pOffset)->magic==XMACH_DEF::S_MH_MAGIC)||(((XMACH_DEF::mach_header *)pOffset)->magic==XMACH_DEF::S_MH_CIGAM))
        {
            stResult.insert(FT_MACHO);
            stResult.insert(FT_MACHO32);
        }
        else if((((XMACH_DEF::mach_header *)pOffset)->magic==XMACH_DEF::S_MH_MAGIC_64)||(((XMACH_DEF::mach_header *)pOffset)->magic==XMACH_DEF::S_MH_CIGAM_64))
        {
            stResult.insert(FT_MACHO);
            stResult.insert(FT_MACHO64);
        }
    }

    if(bExtra)
    {
        _MEMORY_MAP memoryMap=XBinary::getMemoryMap();
        UNICODE_TYPE unicodeType=getUnicodeType(&baHeader);

        if(compareSignature(&memoryMap,"'PK'0304",0)||compareSignature(&memoryMap,"'PK'0506",0)) // TODO baHeader
        {
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_ZIP);
            // TODO Check APK, JAR
            // TODO basic ZIP
        }
        else if(compareSignature(&memoryMap,"1F8B"))
        {
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_GZIP);
            // TODO DEB
        }
        else if(compareSignature(&memoryMap,"'!<arch>'0a"))
        {
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_AR);
            // TODO DEB
        }
        else if(compareSignature(&memoryMap,"'RE~^'")||compareSignature(&memoryMap,"'Rar!'1A07"))
        {
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_RAR);
        }
        else if(compareSignature(&memoryMap,"'MSCF'",0))
        {
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_CAB);
        }
        else if(compareSignature(&memoryMap,"'7z'BCAF271C",0))
        {
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_7Z);
        }
        else if(compareSignature(&memoryMap,"89'PNG\r\n'1A0A........'IHDR'",0))
        {
            stResult.insert(FT_IMAGE);
            stResult.insert(FT_PNG);
        }
        else if(compareSignature(&memoryMap,"FFD8FFE0....'JFIF'00",0))
        {
            stResult.insert(FT_IMAGE);
            stResult.insert(FT_JPEG);
        }
        else if(compareSignature(&memoryMap,"'GIF8'",0))
        {
            stResult.insert(FT_IMAGE);
            stResult.insert(FT_GIF);
        }
        else if(compareSignature(&memoryMap,"'MM'002A",0)||compareSignature(&memoryMap,"'II'2A00",0))
        {
            stResult.insert(FT_IMAGE);
            stResult.insert(FT_TIFF);
        }
        else if(compareSignature(&memoryMap,"'dex\n'......00"))
        {
            stResult.insert(FT_DEX);
        }
        else if(compareSignature(&memoryMap,"02000C00"))
        {
            stResult.insert(FT_ANDROIDASRC);
        }
        else if(compareSignature(&memoryMap,"03000800"))
        {
            stResult.insert(FT_ANDROIDXML);
        }
        else if(compareSignature(&memoryMap,"'%PDF'",0))
        {
            stResult.insert(FT_DOCUMENT);
            stResult.insert(FT_PDF);
        }

        if(isPlainTextType(&baHeader))
        {
            stResult.insert(FT_TEXT);
            stResult.insert(FT_PLAINTEXT);
        }
        else if(isUTF8TextType(&baHeader))
        {
            stResult.insert(FT_TEXT);
            stResult.insert(FT_UTF8);
        }
        else if(unicodeType!=UNICODE_TYPE_NONE)
        {
            stResult.insert(FT_TEXT);
            stResult.insert(FT_UNICODE);

            if(unicodeType==UNICODE_TYPE_LE)
            {
                stResult.insert(FT_UNICODE_LE);
            }
            else
            {
                stResult.insert(FT_UNICODE_BE);
            }
        }
        else if(nSize>=(int)sizeof(XMACH_DEF::fat_header)+(int)sizeof(XMACH_DEF::fat_arch))
        {
            if(read_uint32(0,true)==XMACH_DEF::S_FAT_MAGIC)
            {
                if(read_uint32(4,true)<10)
                {
                    stResult.insert(FT_ARCHIVE);
                    stResult.insert(FT_MACHOFAT);
                }
            }
            else if(read_uint32(0,false)==XMACH_DEF::S_FAT_MAGIC)
            {
                if(read_uint32(4,false)<10)
                {
                    stResult.insert(FT_ARCHIVE);
                    stResult.insert(FT_MACHOFAT);
                }
            }
        }
        // TODO more
        // TODO MIME

        // Fix
        if(stResult.contains(FT_GIF)&&stResult.contains(FT_TEXT))
        {
            stResult.remove(FT_GIF);
        }
    }

    if(bExtra)
    {
        if(stResult.count()<=1)
        {
            stResult.insert(FT_COM);
        }
    }

    return stResult;
}

QSet<XBinary::FT> XBinary::getFileTypes(QIODevice *pDevice,bool bExtra)
{
    XBinary _binary(pDevice);

    return _binary.getFileTypes(bExtra);
}

QSet<XBinary::FT> XBinary::getFileTypes(QString sFileName,bool bExtra)
{
    QSet<XBinary::FT> result;

    QFile file;
    file.setFileName(sFileName);

    if(file.open(QIODevice::ReadOnly))
    {
        XBinary _binary(&file);

        result=_binary.getFileTypes(bExtra);

        file.close();
    }

    return result;
}

QSet<XBinary::FT> XBinary::getFileTypes(QByteArray *pbaData,bool bExtra)
{
    QSet<XBinary::FT> result;

    QBuffer buffer;

    buffer.setBuffer(pbaData);

    if(buffer.open(QIODevice::ReadOnly))
    {
        XBinary _binary(&buffer);

        result=_binary.getFileTypes(bExtra);

        buffer.close();
    }

    return result;
}

XBinary::FT XBinary::_getPrefFileType(QSet<FT> *pStFileTypes)
{
    XBinary::FT result=FT_UNKNOWN;

    if(pStFileTypes->contains(FT_PE32))
    {
        result=FT_PE32;
    }
    else if(pStFileTypes->contains(FT_PE64))
    {
        result=FT_PE64;
    }
    else if(pStFileTypes->contains(FT_MACHO32))
    {
        result=FT_MACHO32;
    }
    else if(pStFileTypes->contains(FT_MACHO64))
    {
        result=FT_MACHO64;
    }
    else if(pStFileTypes->contains(FT_MACHOFAT))
    {
        result=FT_MACHOFAT;
    }
    else if(pStFileTypes->contains(FT_ELF32))
    {
        result=FT_ELF32;
    }
    else if(pStFileTypes->contains(FT_ELF64))
    {
        result=FT_ELF64;
    }
    else if(pStFileTypes->contains(FT_LE))
    {
        result=FT_LE;
    }
    else if(pStFileTypes->contains(FT_LX))
    {
        result=FT_LX;
    }
    else if(pStFileTypes->contains(FT_NE))
    {
        result=FT_NE;
    }
    else if(pStFileTypes->contains(FT_MSDOS))
    {
        result=FT_MSDOS;
    }
    else if(pStFileTypes->contains(FT_ZIP))
    {
        result=FT_ZIP;
    }
    else if(pStFileTypes->contains(FT_GZIP))
    {
        result=FT_GZIP;
    }
    else if(pStFileTypes->contains(FT_ANDROIDXML))
    {
        result=FT_ANDROIDXML;
    }
    else if(pStFileTypes->contains(FT_DEX))
    {
        result=FT_DEX;
    }
    else if(pStFileTypes->contains(FT_BINARY))
    {
        result=FT_BINARY;
    }

    return result;
}

QSet<XBinary::FT> XBinary::getFileTypes(QIODevice *pDevice, qint64 nOffset, qint64 nSize, bool bExtra)
{
    QSet<XBinary::FT> result;

    SubDevice sd(pDevice,nOffset,nSize);

    if(sd.open(QIODevice::ReadOnly))
    {
        result=getFileTypes(&sd,bExtra);

        sd.close();
    }

    return result;
}

XBinary::FT XBinary::getPrefFileType(QIODevice *pDevice, bool bExtra)
{
    QSet<XBinary::FT> stFileTypes=getFileTypes(pDevice,bExtra);

    return _getPrefFileType(&stFileTypes);
}

XBinary::FT XBinary::getPrefFileType(QString sFileName, bool bExtra)
{
    XBinary::FT result=XBinary::FT_UNKNOWN;

    QFile file;
    file.setFileName(sFileName);

    if(file.open(QIODevice::ReadOnly))
    {
        XBinary _binary(&file);

        result=_binary.getPrefFileType(&file,bExtra);

        file.close();
    }

    return result;
}

QList<XBinary::FT> XBinary::_getFileTypeListFromSet(QSet<XBinary::FT> stFileTypes)
{
    QList<XBinary::FT> listResult;

    // TODO optimize !
    if(stFileTypes.contains(FT_REGION))     listResult.append(FT_REGION);
    if(stFileTypes.contains(FT_BINARY))     listResult.append(FT_BINARY);
    if(stFileTypes.contains(FT_BINARY16))   listResult.append(FT_BINARY16);
    if(stFileTypes.contains(FT_BINARY32))   listResult.append(FT_BINARY32);
    if(stFileTypes.contains(FT_BINARY64))   listResult.append(FT_BINARY64);
    if(stFileTypes.contains(FT_ZIP))        listResult.append(FT_ZIP);
    if(stFileTypes.contains(FT_GZIP))       listResult.append(FT_GZIP);
    if(stFileTypes.contains(FT_JAR))        listResult.append(FT_JAR);
    if(stFileTypes.contains(FT_APK))        listResult.append(FT_APK);
    if(stFileTypes.contains(FT_IPA))        listResult.append(FT_IPA);
    if(stFileTypes.contains(FT_DEX))        listResult.append(FT_DEX);
    if(stFileTypes.contains(FT_PDF))        listResult.append(FT_PDF);
    if(stFileTypes.contains(FT_COM))        listResult.append(FT_COM);
    if(stFileTypes.contains(FT_MSDOS))      listResult.append(FT_MSDOS);
    if(stFileTypes.contains(FT_NE))         listResult.append(FT_NE);
    if(stFileTypes.contains(FT_LE))         listResult.append(FT_LE);
    if(stFileTypes.contains(FT_LX))         listResult.append(FT_LX);
    if(stFileTypes.contains(FT_PE32))       listResult.append(FT_PE32);
    if(stFileTypes.contains(FT_PE64))       listResult.append(FT_PE64);
    if(stFileTypes.contains(FT_ELF32))      listResult.append(FT_ELF32);
    if(stFileTypes.contains(FT_ELF64))      listResult.append(FT_ELF64);
    if(stFileTypes.contains(FT_MACHO32))    listResult.append(FT_MACHO32);
    if(stFileTypes.contains(FT_MACHO64))    listResult.append(FT_MACHO64);
    if(stFileTypes.contains(FT_MACHOFAT))   listResult.append(FT_MACHOFAT);

    return listResult;
}

QString XBinary::valueToHex(quint8 nValue)
{
    return QString("%1").arg(nValue,2,16,QChar('0'));
}

QString XBinary::valueToHex(qint8 nValue)
{
    return valueToHex((quint8)nValue);
}

QString XBinary::valueToHex(quint16 nValue, bool bIsBigEndian)
{
    if(bIsBigEndian)
    {
        nValue=qFromBigEndian(nValue);
    }
    else
    {
        nValue=qFromLittleEndian(nValue);
    }

    return QString("%1").arg(nValue,4,16,QChar('0'));
}

QString XBinary::valueToHex(qint16 nValue, bool bIsBigEndian)
{
    if(bIsBigEndian)
    {
        nValue=qFromBigEndian(nValue);
    }
    else
    {
        nValue=qFromLittleEndian(nValue);
    }

    return valueToHex((quint16)nValue);
}

QString XBinary::valueToHex(quint32 nValue, bool bIsBigEndian)
{
    if(bIsBigEndian)
    {
        nValue=qFromBigEndian(nValue);
    }
    else
    {
        nValue=qFromLittleEndian(nValue);
    }

    return QString("%1").arg(nValue,8,16,QChar('0'));
}

QString XBinary::valueToHex(qint32 nValue, bool bIsBigEndian)
{
    if(bIsBigEndian)
    {
        nValue=qFromBigEndian(nValue);
    }
    else
    {
        nValue=qFromLittleEndian(nValue);
    }

    return valueToHex((quint32)nValue);
}

QString XBinary::valueToHex(quint64 nValue, bool bIsBigEndian)
{
    if(bIsBigEndian)
    {
        nValue=qFromBigEndian(nValue);
    }
    else
    {
        nValue=qFromLittleEndian(nValue);
    }

    return QString("%1").arg(nValue,16,16,QChar('0'));
}

QString XBinary::valueToHex(qint64 nValue, bool bIsBigEndian)
{
    if(bIsBigEndian)
    {
        nValue=qFromBigEndian(nValue);
    }
    else
    {
        nValue=qFromLittleEndian(nValue);
    }

    return valueToHex((quint64)nValue);
}

QString XBinary::valueToHex(float fValue, bool bIsBigEndian)
{
    float _value=fValue;

    endian_float(&_value,bIsBigEndian);

    quint32 _nValue=0;

    _copyMemory((char *)&_nValue,(char *)&_value,4);

    return QString("%1").arg(_nValue,8,16,QChar('0'));
}

QString XBinary::valueToHex(double dValue, bool bIsBigEndian)
{
    double _value=dValue;

    endian_double(&_value,bIsBigEndian);

    quint64 _nValue=0;

    _copyMemory((char *)&_nValue,(char *)&_value,8);

    return QString("%1").arg(_nValue,16,16,QChar('0'));
}

QString XBinary::valueToHex(XBinary::MODE mode, quint64 nValue, bool bIsBigEndian)
{
    QString sResult;

    if(mode==MODE_UNKNOWN)
    {
        mode=getWidthModeFromSize(nValue);
    }

    if(mode==MODE_BIT)
    {
        if(nValue)
        {
            sResult="1";
        }
        else
        {
            sResult="0";
        }
    }
    else if(mode==MODE_8)
    {
        sResult=valueToHex((quint8)nValue);
    }
    else if(mode==MODE_16)
    {
        sResult=valueToHex((quint16)nValue,bIsBigEndian);
    }
    else if(mode==MODE_32)
    {
        sResult=valueToHex((quint32)nValue,bIsBigEndian);
    }
    else if(mode==MODE_64)
    {
        sResult=valueToHex((quint64)nValue,bIsBigEndian);
    }

    return sResult;
}

QString XBinary::valueToHexEx(quint64 nValue, bool bIsBigEndian)
{
    XBinary::MODE mode=getWidthModeFromSize(nValue);

    return valueToHex(mode,nValue,bIsBigEndian);
}

QString XBinary::valueToHexOS(quint64 nValue,bool bIsBigEndian)
{
    MODE mode=MODE_32;

    if(sizeof(void *)==8)
    {
        mode=MODE_64;
    }

    return valueToHex(mode,nValue,bIsBigEndian);
}

QString XBinary::valueToHexColon(MODE mode, quint64 nValue, bool bIsBigEndian)
{
    QString sResult;

    if(mode==MODE_64)
    {
        quint32 nHigh=(quint32)(nValue>>32);
        quint32 nLow=(quint32)(nValue);
        sResult=QString("%1:%2").arg(valueToHex(nHigh),valueToHex(nLow));
    }
    else if(mode==MODE_32)
    {
        quint16 nHigh=(quint16)(nValue>>16);
        quint16 nLow=(quint16)(nValue);
        sResult=QString("%1:%2").arg(valueToHex(nHigh),valueToHex(nLow));
    }
    else
    {
        sResult=valueToHex(mode,nValue,bIsBigEndian);
    }

    return sResult;
}

QString XBinary::xVariantToHex(XVARIANT value)
{
    QString sResult;

    if(value.mode==MODE_BIT)
    {
        if(value.var.v_bool)
        {
            sResult="1";
        }
        else
        {
            sResult="0";
        }
    }
    else if(value.mode==MODE_8)
    {
        sResult=valueToHex(value.var.v_uint8);
    }
    else if(value.mode==MODE_16)
    {
        sResult=valueToHex(value.var.v_uint16,value.bIsBigEndian);
    }
    else if(value.mode==MODE_32)
    {
        sResult=valueToHex(value.var.v_uint32,value.bIsBigEndian);
    }
    else if(value.mode==MODE_64)
    {
        sResult=valueToHex(value.var.v_uint64,value.bIsBigEndian);
    }
    else if(value.mode==MODE_128)
    {
        QString sLow=valueToHex(value.var.v_uint128.low,value.bIsBigEndian);
        QString sHigh=valueToHex(value.var.v_uint128.high,value.bIsBigEndian);

        if(value.bIsBigEndian)
        {
            sResult=sLow+sHigh;
        }
        else
        {
            sResult=sHigh+sLow;
        }
    }

    return sResult;
}

QString XBinary::thisToString(qint64 nDelta)
{
    QString sResult;

    if(nDelta==0)
    {
        sResult="$ ==>";
    }
    else if(nDelta>0)
    {
        sResult=QString("$+%1").arg(nDelta,0,16);
    }
    else if(nDelta<0)
    {
        sResult=QString("$-%1").arg(-nDelta,0,16);
    }

    return sResult;
}

bool XBinary::checkString_uint8(QString sValue)
{
    bool bResult=false;

    // TODO Check

    quint16 nValue=sValue.toUShort(&bResult);

    if(bResult)
    {
        bResult=(nValue<=256);
    }

    return bResult;
}

bool XBinary::checkString_int8(QString sValue)
{
    bool bResult=false;

    qint16 nValue=sValue.toShort(&bResult);

    if(bResult)
    {
        bResult=((-127<=nValue)&&(nValue<=128));
    }

    return bResult;
}

bool XBinary::checkString_uint16(QString sValue)
{
    bool bResult=false;

    sValue.toUShort(&bResult);

    return bResult;
}

bool XBinary::checkString_int16(QString sValue)
{
    bool bResult=false;

    sValue.toShort(&bResult);

    return bResult;
}

bool XBinary::checkString_uint32(QString sValue)
{
    bool bResult=false;

    sValue.toUInt(&bResult);

    return bResult;
}

bool XBinary::checkString_int32(QString sValue)
{
    bool bResult=false;

    sValue.toInt(&bResult);

    return bResult;
}

bool XBinary::checkString_uint64(QString sValue)
{
    bool bResult=false;

    sValue.toULongLong(&bResult);

    return bResult;
}

bool XBinary::checkString_int64(QString sValue)
{
    bool bResult=false;

    sValue.toLongLong(&bResult);

    return bResult;
}

bool XBinary::checkString_float(QString sValue)
{
    bool bResult=false;

    sValue.toFloat(&bResult);

    return bResult;
}

bool XBinary::checkString_double(QString sValue)
{
    bool bResult=false;

    sValue.toDouble(&bResult);

    return bResult;
}

QString XBinary::boolToString(bool bValue)
{
    QString sResult;

    if(bValue)
    {
        sResult="true";
    }
    else
    {
        sResult="false";
    }

    return sResult;
}

QString XBinary::getSpaces(qint32 nNumberOfSpaces)
{
    QString sResult;

    sResult=sResult.rightJustified(nNumberOfSpaces,QChar(' '));

    return sResult;
}

QString XBinary::getUnpackedFileName(QIODevice *pDevice)
{
    QString sResult="unpacked";

    QFile *pFile=dynamic_cast<QFile *>(pDevice);

    if(pFile)
    {
        QString sFileName=pFile->fileName();

        if(sFileName!="")
        {
            sResult=getUnpackedFileName(sFileName);
        }
    }

    return sResult;
}

QString XBinary::getUnpackedFileName(QString sFileName)
{
    QFileInfo fileInfo(sFileName);
    QString sResult=fileInfo.absolutePath()+QDir::separator()+fileInfo.completeBaseName()+".unp."+fileInfo.suffix();
    //            sResult=fi.absolutePath()+QDir::separator()+fi.baseName()+".unp."+fi.completeSuffix();

    return sResult;
}

QString XBinary::getDumpFileName(QIODevice *pDevice)
{
    QString sResult="dump";

    QFile *pFile=dynamic_cast<QFile *>(pDevice);

    if(pFile)
    {
        QString sFileName=pFile->fileName();

        if(sFileName!="")
        {
            sResult=getDumpFileName(sFileName);
        }
    }

    return sResult;
}

QString XBinary::getDumpFileName(QString sFileName)
{
    QFileInfo fileInfo(sFileName);
    QString sResult=fileInfo.absolutePath()+QDir::separator()+fileInfo.completeBaseName()+".dump."+fileInfo.suffix();

    return sResult;
}

QString XBinary::getBackupFileName(QIODevice *pDevice)
{
    QString sResult=QString("Backup.%1.BAK").arg(getCurrentBackupDate());

    QFile *pFile=dynamic_cast<QFile *>(pDevice);

    if(pFile)
    {
        QString sFileName=pFile->fileName();

        if(sFileName!="")
        {
            sResult=getBackupFileName(sFileName);
        }
    }

    return sResult;
}

QString XBinary::getBackupFileName(QString sFileName)
{
    QFileInfo fi(sFileName);
    QString sResult;

    sResult+=fi.absolutePath()+QDir::separator()+fi.completeBaseName();

    QString sSuffix=fi.suffix();

    if(sSuffix!="")
    {
        sResult+="."+sSuffix;
    }

    sResult+=QString(".%1.BAK").arg(getCurrentBackupDate());

    return sResult;
}

QString XBinary::getResultFileName(QIODevice *pDevice, QString sAppendix)
{
    QString sResult=sAppendix;

    QFile *pFile=dynamic_cast<QFile *>(pDevice);

    if(pFile)
    {
        QString sFileName=pFile->fileName();

        if(sFileName!="")
        {
            sResult=getResultFileName(sFileName,sAppendix);
        }
    }

    return sResult;
}

QString XBinary::getResultFileName(QString sFileName,QString sAppendix)
{
    QString sResult;
    // mb TODO if file exists write other .1 .2 ...
    QFileInfo fileInfo(sFileName);

    QString sSuffix=fileInfo.suffix();

    sResult+=fileInfo.absolutePath()+QDir::separator()+fileInfo.completeBaseName()+".";

    if(sSuffix!="")
    {
        sResult+=sSuffix+".";
    }

    sResult+=sAppendix;

    return sResult;
}

QString XBinary::getDeviceFileName(QIODevice *pDevice)
{
    QString sResult;

    QFile *pFile=dynamic_cast<QFile *>(pDevice);

    if(pFile)
    {
        sResult=pFile->fileName();
    }

    return sResult;
}

QString XBinary::getDeviceFilePath(QIODevice *pDevice)
{
    QString sResult;

    QFile *pFile=dynamic_cast<QFile *>(pDevice);

    if(pFile)
    {
        QString sFileName=pFile->fileName();

        if(sFileName!="")
        {
            QFileInfo fi(sFileName);

            sResult=fi.absoluteFilePath();
        }
    }

    return sResult;
}

QString XBinary::getDeviceDirectory(QIODevice *pDevice)
{
    QString sResult;

    QFile *pFile=dynamic_cast<QFile *>(pDevice);

    if(pFile)
    {
        QString sFileName=pFile->fileName();

        if(sFileName!="")
        {
            QFileInfo fi(sFileName);

            sResult=fi.absolutePath();
        }
    }

    return sResult;
}

QString XBinary::getDeviceFileBaseName(QIODevice *pDevice)
{
    QString sResult;

    QFile *pFile=dynamic_cast<QFile *>(pDevice);

    if(pFile)
    {
        QString sFileName=pFile->fileName();

        if(sFileName!="")
        {
            QFileInfo fi(sFileName);

            sResult=fi.baseName();
        }
    }

    return sResult;
}

QString XBinary::getDeviceFileCompleteSuffix(QIODevice *pDevice)
{
    QString sResult;

    QFile *pFile=dynamic_cast<QFile *>(pDevice);

    if(pFile)
    {
        QString sFileName=pFile->fileName();

        if(sFileName!="")
        {
            QFileInfo fi(sFileName);

            sResult=fi.completeSuffix();
        }
    }

    return sResult;
}

QString XBinary::getDeviceFileSuffix(QIODevice *pDevice)
{
    QString sResult;

    QFile *pFile=dynamic_cast<QFile *>(pDevice);

    if(pFile)
    {
        QString sFileName=pFile->fileName();

        if(sFileName!="")
        {
            QFileInfo fi(sFileName);

            sResult=fi.suffix();
        }
    }

    return sResult;
}

bool XBinary::isBackupPresent(QIODevice *pDevice)
{
    return XBinary::isFileExists(XBinary::getBackupFileName(pDevice));
}

bool XBinary::saveBackup(QIODevice *pDevice)
{
    bool bResult=false;

    QString sBackupFileName=XBinary::getBackupFileName(pDevice);

    if(sBackupFileName!="")
    {
        if(!QFile::exists(sBackupFileName))
        {
            QString sFileName=XBinary::getDeviceFileName(pDevice);

            if(sFileName!="")
            {
                bResult=QFile::copy(sFileName,sBackupFileName);
            }
            else
            {
                bResult=XBinary::writeToFile(sBackupFileName,pDevice);
            }
        }
    }

    return bResult;
}

QString XBinary::getCurrentBackupDate()
{
    QString sResult;

    sResult=QDate::currentDate().toString("yyyy-MM-dd");

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

        char *pBuffer1=new char[READWRITE_BUFFER_SIZE+3];
        char *pBuffer2=new char[READWRITE_BUFFER_SIZE+3];
        qint64 nOffset=0;

        while(nSize>3)
        {
            nTemp=qMin((qint64)(READWRITE_BUFFER_SIZE+3),nSize);

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

            for(unsigned int i=0;i<nTemp-3;i++)
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

QString XBinary::getHash(XBinary::HASH hash, QString sFileName, PDSTRUCT *pProcessData)
{
    QString sResult;

    QFile file;
    file.setFileName(sFileName);

    if(file.open(QIODevice::ReadOnly))
    {
        sResult=XBinary::getHash(hash,&file,pProcessData);

        file.close();
    }

    return sResult;
}

QString XBinary::getHash(XBinary::HASH hash, QIODevice *pDevice, PDSTRUCT *pProcessData)
{
    QString sResult;

    XBinary binary(pDevice);

    sResult=binary.getHash(hash,0,-1,pProcessData);

    pDevice->reset();

    return sResult;
}

QString XBinary::getHash(XBinary::HASH hash, qint64 nOffset, qint64 nSize, PDSTRUCT *pProcessData)
{
    QString sResult;

    OFFSETSIZE osRegion=convertOffsetAndSize(nOffset,nSize);

    if(osRegion.nOffset!=-1)
    {
        QList<OFFSETSIZE> listOS;
        listOS.append(osRegion);

        sResult=getHash(hash,&listOS,pProcessData);
    }

    return sResult;
}

QString XBinary::getHash(HASH hash, QList<OFFSETSIZE> *pListOS, PDSTRUCT *pProcessData)
{
    QString sResult;

    PDSTRUCT processDataEmpty={};

    if(!pProcessData)
    {
        pProcessData=&processDataEmpty;
    }

    qint64 nTemp=0;
    char *pBuffer=new char[READWRITE_BUFFER_SIZE];

    QCryptographicHash::Algorithm algorithm=QCryptographicHash::Md4;

    switch(hash) // TODO Check new versions of Qt
    {
        case HASH_MD4:          algorithm=QCryptographicHash::Md4;          break;
        case HASH_MD5:          algorithm=QCryptographicHash::Md5;          break;
        case HASH_SHA1:         algorithm=QCryptographicHash::Sha1;         break;
    #ifndef QT_CRYPTOGRAPHICHASH_ONLY_SHA1
        case HASH_SHA224:       algorithm=QCryptographicHash::Sha224;       break; // Keccak_224 ?
        case HASH_SHA256:       algorithm=QCryptographicHash::Sha256;       break;
        case HASH_SHA384:       algorithm=QCryptographicHash::Sha384;       break;
        case HASH_SHA512:       algorithm=QCryptographicHash::Sha512;       break;
    #endif
    }

    QCryptographicHash crypto(algorithm);

    qint32 nNumberOfRecords=pListOS->count();

    qint64 nCurrentSize=0;

    qint32 _nFreeIndex=XBinary::getFreeIndex(pProcessData);

    bool bReadError=false;

    for(qint32 i=0;(i<nNumberOfRecords)&&(!(pProcessData->bIsStop));i++)
    {
        qint64 nOffset=pListOS->at(i).nOffset;
        qint64 nSize=pListOS->at(i).nSize;

        XBinary::setPdStructInit(pProcessData,_nFreeIndex,nSize);

        while((nSize>0)&&(!(pProcessData->bIsStop)))
        {
            nTemp=qMin((qint64)READWRITE_BUFFER_SIZE,nSize);

            if(read_array(nOffset,pBuffer,nTemp)!=nTemp)
            {
                _errorMessage(tr("Read error"));

                bReadError=true;

                break;
            }

            crypto.addData(pBuffer,nTemp);

            nSize-=nTemp;
            nOffset+=nTemp;
            nCurrentSize+=nTemp;

            XBinary::setPdStructCurrent(pProcessData,_nFreeIndex,nCurrentSize);
        }
    }

    XBinary::setPdStructFinished(pProcessData,_nFreeIndex);

    delete[] pBuffer;

    if(!bReadError)
    {
        sResult=crypto.result().toHex();
    }

    if(pProcessData->bIsStop)
    {
        sResult="";
    }

    return sResult;
}

QSet<XBinary::HASH> XBinary::getHashMethods()
{
    QSet<XBinary::HASH> stResult;

    stResult.insert(HASH_MD4);
    stResult.insert(HASH_MD5);
    stResult.insert(HASH_SHA1);
#ifndef QT_CRYPTOGRAPHICHASH_ONLY_SHA1
    stResult.insert(HASH_SHA224);
    stResult.insert(HASH_SHA256);
    stResult.insert(HASH_SHA384);
    stResult.insert(HASH_SHA512);
#endif

    return stResult;
}

QList<XBinary::HASH> XBinary::getHashMethodsAsList()
{
    QList<XBinary::HASH> listResult;

    listResult.append(HASH_MD4);
    listResult.append(HASH_MD5);
    listResult.append(HASH_SHA1);
#ifndef QT_CRYPTOGRAPHICHASH_ONLY_SHA1
    listResult.append(HASH_SHA224);
    listResult.append(HASH_SHA256);
    listResult.append(HASH_SHA384);
    listResult.append(HASH_SHA512);
#endif

    return listResult;
}

QString XBinary::hashIdToString(XBinary::HASH hash)
{
    QString sResult=tr("Unknown");

    switch(hash)
    {
        case HASH_MD4:          sResult=QString("MD4");         break;
        case HASH_MD5:          sResult=QString("MD5");         break;
        case HASH_SHA1:         sResult=QString("SHA1");        break;
#ifndef QT_CRYPTOGRAPHICHASH_ONLY_SHA1
        case HASH_SHA224:       sResult=QString("SHA224");      break;
        case HASH_SHA256:       sResult=QString("SHA256");      break;
        case HASH_SHA384:       sResult=QString("SHA384");      break;
        case HASH_SHA512:       sResult=QString("SHA512");      break;
#endif
    }

    return sResult;
}

bool XBinary::isFileHashValid(XBinary::HASH hash, QString sFileName, QString sHash)
{
    bool bResult=false;

    if(isFileExists(sFileName))
    {
        bResult=(getHash(hash,sFileName).toUpper()==sHash.toUpper());
    }

    return bResult;
}

quint32 XBinary::getAdler32(QString sFileName)
{
    quint32 nResult=0;

    QFile file;
    file.setFileName(sFileName);

    if(file.open(QIODevice::ReadOnly))
    {
        nResult=XBinary::getAdler32(&file);

        file.close();
    }

    return nResult;
}

quint32 XBinary::getAdler32(QIODevice *pDevice)
{
    quint32 nResult=0;

    XBinary binary(pDevice);

    nResult=binary.getAdler32(0,-1);

    pDevice->reset();

    return nResult;
}

quint32 XBinary::getAdler32(qint64 nOffset, qint64 nSize)
{
    // TODO optimize!!!
    quint32 nResult=0;

    OFFSETSIZE osRegion=convertOffsetAndSize(nOffset,nSize);

    nOffset=osRegion.nOffset;
    nSize=osRegion.nSize;

    const quint32 MOD_ADLER=65521;

    if(nOffset!=-1)
    {
        qint64 nTemp=0;
        char *pBuffer=new char[READWRITE_BUFFER_SIZE];

        quint32 a=1;
        quint32 b=0;

        while(nSize>0)
        {
            nTemp=qMin((qint64)READWRITE_BUFFER_SIZE,nSize);

            if(read_array(nOffset,pBuffer,nTemp)!=nTemp)
            {
                _errorMessage(tr("Read error"));
                delete[] pBuffer;
                return 0;
            }

            for(qint64 i=0;i<nTemp;i++)
            {
                a=(a+(quint8)(pBuffer[nOffset+i]))%MOD_ADLER;
                b=(b+a)%MOD_ADLER;
            }

            nSize-=nTemp;
            nOffset+=nTemp;
        }

        delete[] pBuffer;

        nResult=(b<<16)|a;
    }

    return nResult;
}

quint32 XBinary::_getCRC32(QString sFileName)
{
    quint32 nResult=0;

    QFile file;
    file.setFileName(sFileName);

    if(file.open(QIODevice::ReadOnly))
    {
        nResult=XBinary::_getCRC32(&file);

        file.close();
    }

    return nResult;
}

quint32 XBinary::_getCRC32(QIODevice *pDevice)
{
    quint32 nResult=0;

    XBinary binary(pDevice);

    nResult=binary._getCRC32(0,-1);

    pDevice->reset();

    return nResult;
}

quint32 XBinary::_getCRC32(qint64 nOffset, qint64 nSize, PDSTRUCT *pProcessData)
{
    // TODO optimize!!!
    quint32 nResult=0xFFFFFFFF; // ~0

    PDSTRUCT processDataEmpty={};

    if(!pProcessData)
    {
        pProcessData=&processDataEmpty;
    }

    OFFSETSIZE osRegion=convertOffsetAndSize(nOffset,nSize);

    nOffset=osRegion.nOffset;
    nSize=osRegion.nSize;

    qint32 _nFreeIndex=XBinary::getFreeIndex(pProcessData);

    if((nOffset!=-1)&&(!(pProcessData->bIsStop)))
    {
        XBinary::setPdStructInit(pProcessData,_nFreeIndex,nSize);

        quint32 crc_table[256];

        for(qint32 i=0;i<256;i++)
        {
            quint32 crc=i;

            for(qint32 j=0;j<8;j++)
            {
                crc=(crc&1)?((crc>>1)^0xEDB88320):(crc>>1);
            }

            crc_table[i]=crc;
        }

        qint64 nTemp=0;
        char *pBuffer=new char[READWRITE_BUFFER_SIZE];

        while(nSize>0)
        {
            nTemp=qMin((qint64)READWRITE_BUFFER_SIZE,nSize);

            if(read_array(nOffset,pBuffer,nTemp)!=nTemp)
            {
                _errorMessage(tr("Read error"));

                nResult=0;

                break;
            }

            for(qint32 i=0;i<nTemp;i++)
            {
                nResult=crc_table[(nResult^((quint8)pBuffer[i]))&0xFF]^(nResult>>8);
            }

            nSize-=nTemp;
            nOffset+=nTemp;

            XBinary::setPdStructCurrent(pProcessData,_nFreeIndex,nOffset);
        }

        delete[] pBuffer;
    }

    nResult^=0xFFFFFFFF;

    XBinary::setPdStructFinished(pProcessData,_nFreeIndex);

    if(pProcessData->bIsStop)
    {
        nResult=0;
    }

    return nResult;
}

double XBinary::getEntropy(QString sFileName)
{
    double dResult=0;

    QFile file;

    file.setFileName(sFileName);

    if(file.open(QIODevice::ReadOnly))
    {
        dResult=XBinary::getEntropy(&file);

        file.close();
    }

    return dResult;
}

double XBinary::getEntropy(QIODevice *pDevice, PDSTRUCT *pProcessData)
{
    double dResult=0;

    XBinary binary(pDevice);

    dResult=binary.getEntropy(0,-1,pProcessData);

    pDevice->reset();

    return dResult;
}

double XBinary::getEntropy(qint64 nOffset,qint64 nSize,PDSTRUCT *pProcessData)
{
    double dResult=1.4426950408889634073599246810023;

    PDSTRUCT processDataEmpty={};

    if(!pProcessData)
    {
        pProcessData=&processDataEmpty;
    }

    OFFSETSIZE osRegion=convertOffsetAndSize(nOffset,nSize);

    nOffset=osRegion.nOffset;
    nSize=osRegion.nSize;

    if(nSize==0)
    {
        dResult=0;
    }

    bool bReadError=false;

    qint32 _nFreeIndex=XBinary::getFreeIndex(pProcessData);

    if((nOffset!=-1)&&(!(pProcessData->bIsStop)))
    {
        XBinary::setPdStructInit(pProcessData,_nFreeIndex,nSize);

        double bytes[256]={0.0};

        qint64 nTemp=0;
        char *pBuffer=new char[READWRITE_BUFFER_SIZE];

        while((nSize>0)&&(!(pProcessData->bIsStop)))
        {
            nTemp=qMin((qint64)READWRITE_BUFFER_SIZE,nSize);

            if(read_array(nOffset,pBuffer,nTemp)!=nTemp)
            {
                _errorMessage(tr("Read error"));
                bReadError=true;

                break;
            }

            for(qint64 i=0;i<nTemp;i++)
            {
                bytes[(unsigned char)pBuffer[i]]+=1;
            }

            nSize-=nTemp;
            nOffset+=nTemp;

            XBinary::setPdStructCurrent(pProcessData,_nFreeIndex,nOffset-osRegion.nOffset);
        }

        delete[] pBuffer;

        if((!(pProcessData->bIsStop))&&(!bReadError))
        {
            for(qint32 j=0;j<256;j++)
            {
                double dTemp=bytes[j]/(double)osRegion.nSize;

                if(dTemp)
                {
                    dResult+=(-log(dTemp)/log((double)2))*bytes[j];
                }
            }

            dResult=dResult/(double)osRegion.nSize;
        }
    }

    XBinary::setPdStructFinished(pProcessData,_nFreeIndex);

    if(pProcessData->bIsStop)
    {
        dResult=0;
    }

    return dResult;
}

XBinary::BYTE_COUNTS XBinary::getByteCounts(qint64 nOffset,qint64 nSize,PDSTRUCT *pPdStruct)
{
    BYTE_COUNTS result={0};

    PDSTRUCT pdStructEmpty={};

    if(!pPdStruct)
    {
        pPdStruct=&pdStructEmpty;
    }

    OFFSETSIZE osRegion=convertOffsetAndSize(nOffset,nSize);

    nOffset=osRegion.nOffset;
    nSize=osRegion.nSize;

    bool bReadError=false;

    qint32 _nFreeIndex=XBinary::getFreeIndex(pPdStruct);

    if((nOffset!=-1)&&(!(pPdStruct->bIsStop)))
    {
        result.nSize=nSize;

        XBinary::setPdStructInit(pPdStruct,_nFreeIndex,nSize);

        qint64 nTemp=0;
        char *pBuffer=new char[READWRITE_BUFFER_SIZE];

        while((nSize>0)&&(!(pPdStruct->bIsStop)))
        {
            nTemp=qMin((qint64)READWRITE_BUFFER_SIZE,nSize);

            if(read_array(nOffset,pBuffer,nTemp)!=nTemp)
            {
                _errorMessage(tr("Read error"));

                bReadError=true;

                break;
            }

            for(qint64 i=0;i<nTemp;i++)
            {
                result.nCount[(unsigned char)pBuffer[i]]+=1;
            }

            nSize-=nTemp;
            nOffset+=nTemp;

            XBinary::setPdStructCurrent(pPdStruct,_nFreeIndex,nOffset-osRegion.nOffset);
        }

        delete[] pBuffer;
    }

    XBinary::setPdStructFinished(pPdStruct,_nFreeIndex);

    if((pPdStruct->bIsStop)||(bReadError))
    {
        result={0};
    }

    return result;
}

void XBinary::_xor(quint8 nXorValue, qint64 nOffset, qint64 nSize)
{
    // TODO Optimize
    OFFSETSIZE osRegion=convertOffsetAndSize(nOffset,nSize);

    nOffset=osRegion.nOffset;
    nSize=osRegion.nSize;

    if(nOffset!=-1)
    {
        if(nOffset!=-1)
        {
            qint64 nTemp=0;
            char *pBuffer=new char[READWRITE_BUFFER_SIZE];

            while(nSize>0)
            {
                nTemp=qMin((qint64)READWRITE_BUFFER_SIZE,nSize);

                if(read_array(nOffset,pBuffer,nTemp)!=nTemp)
                {
                    _errorMessage(tr("Read error"));
                    break;
                }

                for(qint32 i=0;i<nTemp;i++)
                {
                    *(pBuffer+i)^=nXorValue;
                }

                if(!write_array(nOffset,pBuffer,nTemp))
                {
                    _errorMessage(tr("Write error"));
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

quint32 XBinary::getStringCustomCRC32(QString sString)
{
    quint32 nResult=0; // not ~0 !!! if ~0 (0xFFFFFFFF) it will be a CRC32C

    qint32 nSize=sString.size();
    QByteArray baString=sString.toUtf8();

    for(qint32 i=0;i<nSize;i++)
    {
        unsigned char _char=(unsigned char)baString.data()[i];
//        unsigned char _char1=(unsigned char)sString.at(i).toLatin1();

//        if(_char!=_char1)
//        {
//            qFatal("Error"); // TODO remove
//        }

        nResult^=_char;

        for(qint32 k=0;k<8;k++)
        {
            nResult=(nResult&1)?((nResult>>1)^0x82f63b78):(nResult>>1);
        }
    }

    nResult=~nResult;

    return nResult;
}

QIODevice *XBinary::getDevice()
{
    return g_pDevice;
}

bool XBinary::isValid()
{
    return true;
}

bool XBinary::isValid(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    XBinary xbinary(pDevice,bIsImage,nModuleAddress);

    return xbinary.isValid();
}

XBinary::MODE XBinary::getMode(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    XBinary xbinary(pDevice,bIsImage,nModuleAddress);

    return xbinary.getMode();
}

bool XBinary::isBigEndian()
{
    return g_bIsBigEndian;
}

bool XBinary::is16()
{
    MODE mode=getMode();

    return ((mode==MODE_16)||(mode==MODE_16SEG));
}

bool XBinary::is32()
{
    MODE mode=getMode();

    return (mode==MODE_32);
}

bool XBinary::is64()
{
    MODE mode=getMode();

    return (mode==MODE_64);
}

bool XBinary::isBigEndian(XBinary::_MEMORY_MAP *pMemoryMap)
{
    return pMemoryMap->bIsBigEndian;
}

bool XBinary::is16(XBinary::_MEMORY_MAP *pMemoryMap)
{
    return (pMemoryMap->mode==MODE_16)||(pMemoryMap->mode==MODE_16SEG);
}

bool XBinary::is32(XBinary::_MEMORY_MAP *pMemoryMap)
{
    return (pMemoryMap->mode==MODE_32);
}

bool XBinary::is64(XBinary::_MEMORY_MAP *pMemoryMap)
{
    return (pMemoryMap->mode==MODE_64);
}

void XBinary::setVersion(QString sVersion)
{
    g_sVersion=sVersion;
}

QString XBinary::getVersion()
{
    return g_sVersion;
}

bool XBinary::isEncrypted()
{
    return false;
}

QString XBinary::getSignature(QIODevice *pDevice, qint64 nOffset, qint64 nSize)
{
    XBinary binary(pDevice);

    return binary.getSignature(nOffset,nSize);
}

QString XBinary::getSignature(qint64 nOffset, qint64 nSize)
{
    QString sResult;

    if(nOffset!=-1)
    {
        OFFSETSIZE osRegion=convertOffsetAndSize(nOffset,-1);

        nSize=qMin(osRegion.nSize,nSize);

        sResult=read_array(nOffset,nSize).toHex().toUpper();
    }

    return sResult;
}

XBinary::OFFSETSIZE XBinary::convertOffsetAndSize(qint64 nOffset, qint64 nSize)
{
    OFFSETSIZE osResult={};

    osResult.nOffset=-1;
    osResult.nSize=0;

    qint64 nTotalSize=getSize();

    if((nSize==-1)&&(nTotalSize>nOffset))
    {
        nSize=nTotalSize-nOffset;
    }

    if((nOffset+nSize>nTotalSize)&&(nOffset<nTotalSize))
    {
        nSize=nTotalSize-nOffset;
    }

    if((nSize>0)&&(nOffset>=0)&&(nOffset<nTotalSize)&&(nOffset+nSize-1<nTotalSize))
    {
        osResult.nOffset=nOffset;
        osResult.nSize=nSize;
    }

    return osResult;
}

XBinary::OFFSETSIZE XBinary::convertOffsetAndSize(QIODevice *pDevice, qint64 nOffset, qint64 nSize)
{
    XBinary binary(pDevice);

    return binary.convertOffsetAndSize(nOffset,nSize);
}

bool XBinary::compareSignatureStrings(QString sBaseSignature, QString sOptSignature)
{
    bool bResult=false;
    // TODO optimize
    // TODO check
    sBaseSignature=convertSignature(sBaseSignature);
    sOptSignature=convertSignature(sOptSignature);

    qint32 nSize=qMin(sBaseSignature.size(),sOptSignature.size());

    if((nSize)&&(sBaseSignature.size()>=sOptSignature.size()))
    {
        bResult=true;

        for(qint32 i=0;i<nSize;i++)
        {
            QChar _qchar1=sBaseSignature.at(i);
            QChar _qchar2=sOptSignature.at(i);

            if((_qchar1!=QChar('.'))&&(_qchar2!=QChar('.')))
            {
                if(_qchar1!=_qchar2)
                {
                    bResult=false;

                    break;
                }
            }
        }
    }

    return bResult;
}

void XBinary::_errorMessage(QString sErrorMessage)
{
#ifdef QT_DEBUG
    qDebug("Error: %s",sErrorMessage.toLatin1().data());
#endif
    emit errorMessage(sErrorMessage);
}

void XBinary::_infoMessage(QString sInfoMessage)
{
#ifdef QT_DEBUG
    qDebug("Info: %s",sInfoMessage.toLatin1().data());
#endif
    emit infoMessage(sInfoMessage);
}

qint64 XBinary::_calculateRawSize()
{
    _MEMORY_MAP memoryMap=getMemoryMap();

    return _calculateRawSize(&memoryMap);
}

qint64 XBinary::_calculateRawSize(XBinary::_MEMORY_MAP *pMemoryMap)
{
    qint64 nResult=0;

    qint32 nNumberOfRecords=pMemoryMap->listRecords.count();

    qint64 _nOverlayOffset=-1;

    for(qint32 i=0;i<nNumberOfRecords;i++)
    {
        if((pMemoryMap->listRecords.at(i).nOffset!=-1)&&(pMemoryMap->listRecords.at(i).type!=MMT_OVERLAY))
        {
            nResult=qMax(nResult,(qint64)(pMemoryMap->listRecords.at(i).nOffset+pMemoryMap->listRecords.at(i).nSize));
        }

        if(pMemoryMap->listRecords.at(i).type==MMT_OVERLAY)
        {
            _nOverlayOffset=pMemoryMap->listRecords.at(i).nOffset;
        }
    }

    if(_nOverlayOffset!=-1)
    {
        nResult=qMin(nResult,_nOverlayOffset);
    }

    return nResult;
}

QString XBinary::convertSignature(QString sSignature)
{
    // 'AnsiString'
    // TODO more defs
    if(sSignature.contains(QChar(39)))
    {
        QString sTemp;
        qint32 nStringSize=sSignature.size();
        bool bAnsiString=false;

        for(qint32 i=0;i<nStringSize;i++)
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

QString XBinary::qcharToHex(QChar cSymbol)
{
    // TODO mb
    return QString("%1").arg((quint8)(cSymbol.toLatin1()),2,16,QChar('0'));
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

QString XBinary::floatToString(float fValue,int nPrec)
{
    return QString("%1").arg(fValue,0,'f',nPrec);
}

QString XBinary::doubleToString(double dValue,int nPrec)
{
    return QString("%1").arg(dValue,0,'f',nPrec);
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

    for(qint32 i=sHex.length()-2;i>=0;i-=2)
    {
        sResult+=sHex.mid(i,2);
    }

    return sResult;
}

void XBinary::_swapBytes(char *pSource,qint32 nSize)
{
    for(qint32 i=0;i<(nSize/2);i++)
    {
        char cTemp=pSource[i];
        pSource[i]=pSource[(nSize-1)-i];
        pSource[(nSize-1)-i]=cTemp;
    }
}

quint16 XBinary::swapBytes(quint16 nValue)
{
    _swapBytes((char *)&nValue,2);

    return nValue;
}

quint32 XBinary::swapBytes(quint32 nValue)
{
    _swapBytes((char *)&nValue,4);

    return nValue;
}

quint64 XBinary::swapBytes(quint64 nValue)
{
    _swapBytes((char *)&nValue,8);

    return nValue;
}

bool XBinary::isPlainTextType()
{
    QByteArray baData=read_array(0,qMin(getSize(),(qint64)0x100));

    return isPlainTextType(&baData);
}

bool XBinary::isPlainTextType(QByteArray *pbaData)
{
    bool bResult=false;

    unsigned char *pDataOffset=(unsigned char *)(pbaData->data());
    qint32 nDataSize=pbaData->size();

    if(nDataSize)
    {
        bResult=true;

        for(qint32 i=0;i<nDataSize;i++)
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
    QByteArray baData=read_array(0,qMin(getSize(),(qint64)0x100));

    return isUTF8TextType(&baData);
}

bool XBinary::isUTF8TextType(QByteArray *pbaData)
{
    // EFBBBF
    bool bResult=false;

    unsigned char *pDataOffset=(unsigned char *)(pbaData->data());
    qint32 nDataSize=pbaData->size();

    if(nDataSize>=3)
    {
        if((pDataOffset[0]==0xEF)&&(pDataOffset[1]==0xBB)&&(pDataOffset[2]==0xBF))
        {
            bResult=true;
        }
    }

    if(bResult)
    {
        unsigned char *pDataOffset=(unsigned char *)(pbaData->data());
        pDataOffset+=3;

        nDataSize=pbaData->size();
        nDataSize=nDataSize-3;

        for(qint32 i=0;i<nDataSize;i++)
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

bool XBinary::isPlainTextType(QIODevice *pDevice)
{
    XBinary binary(pDevice);

    return binary.isPlainTextType();
}

XBinary::UNICODE_TYPE XBinary::getUnicodeType()
{
    QByteArray baData=read_array(0,qMin(getSize(),(qint64)0x2));

    return getUnicodeType(&baData);
}

XBinary::UNICODE_TYPE XBinary::getUnicodeType(QByteArray *pbaData)
{
    XBinary::UNICODE_TYPE result=XBinary::UNICODE_TYPE_NONE;

    unsigned char *pDataOffset=(unsigned char *)(pbaData->data());
    qint32 nDataSize=pbaData->size();

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

bool XBinary::checkOffsetSize(XBinary::OFFSETSIZE osRegion)
{
    qint64 nTotalSize=getSize();

    bool bOffsetValid=(osRegion.nOffset>=0)&&(osRegion.nOffset<nTotalSize);
    bool bSizeValid=(osRegion.nSize>0)&&((osRegion.nOffset+osRegion.nSize-1)<nTotalSize);

    return (bOffsetValid)&&(bSizeValid);
}

QString XBinary::get_uint32_full_version(quint32 nValue)
{
    QString sResult=QString("%1.%2.%3").arg(    QString::number((nValue>>16)&0xFFFF),
                                                QString::number((nValue>>8)&0xFF),
                                                QString::number((nValue)&0xFF));

    return sResult;
}

QString XBinary::get_uint32_version(quint32 nValue)
{
    QString sResult=QString("%1.%2").arg(   QString::number((nValue>>16)&0xFFFF),
                                            QString::number((nValue)&0xFFFF));

    return sResult;
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

    QString sClassName=pDevice->metaObject()->className(); // TODO

    if(sClassName=="QFile")
    {
        bResult=((QFile *)pDevice)->resize(nSize);
    }
    else if(sClassName=="QBuffer")
    {
        ((QBuffer *)pDevice)->buffer().resize((qint32)nSize);
        bResult=true;
    }

    return bResult;
}

XBinary::PACKED_INT XBinary::read_uleb128(qint64 nOffset,qint64 nSize)
{
    PACKED_INT result={};

    quint32 nShift=0;

    for(qint32 i=0;i<nSize;i++)
    {
        quint8 nByte=read_uint8(nOffset+i);
        result.nValue|=((nByte&0x7F)<<nShift);
        result.nByteSize++;
        nShift+=7;

        if((nByte&0x80)==0)
        {
            result.bIsValid=true;
            break;
        }
    }

    return result;
}

XBinary::PACKED_INT XBinary::_read_uleb128(char *pData,qint64 nSize)
{
    PACKED_INT result={};

    quint32 nShift=0;

    for(qint32 i=0;i<nSize;i++)
    {
        quint8 nByte=(quint8)(*(pData+i));
        result.nValue|=((nByte&0x7F)<<nShift);
        result.nByteSize++;
        nShift+=7;

        if((nByte&0x80)==0)
        {
            result.bIsValid=true;
            break;
        }
    }

    return result;
}

XBinary::PACKED_INT XBinary::read_acn1_integer(qint64 nOffset, qint64 nSize)
{
    PACKED_INT result={};

    if(nSize>0)
    {
        quint8 nByte=read_uint8(nOffset);

        if((nByte&0x80)==0)
        {
            result.bIsValid=true;
            result.nByteSize=1;
            result.nValue=nByte;
        }
        else
        {
            quint8 _nSize=(nByte&0x7F);

            if((_nSize<=4)&&(_nSize<=nSize))
            {
                result.bIsValid=true;
                result.nByteSize=1+_nSize;

                for(qint32 i=0;i<_nSize;i++)
                {
                    result.nValue<<=8;
                    result.nValue|=read_uint8(nOffset+1+i);
                }
            }
        }
    }

    return result;
}

XBinary::PACKED XBinary::get_packedNumber(qint64 nOffset)
{
    PACKED result={};

    quint8 nFirstByte=0;
    quint8 nSecondByte=0;
    quint8 nMask=0;
    qint64 _nStartOffset=nOffset;

    for(qint32 i=0;i<8;i++)
    {
        if(i==0)
        {
            nFirstByte=read_uint8(nOffset);
            nOffset++;
            if((nFirstByte&0x80)==0)
            {
                result.nValue=nFirstByte;
                break;
            }
        }
        if(i==1)
        {
            nSecondByte=read_uint8(nOffset);
            nOffset++;
            if((nFirstByte&0x40)==0)
            {
                result.nValue=(((quint32)nFirstByte&0x3F)<<8)|nSecondByte;
                break;
            }
            nMask=read_uint8(nOffset);
            nOffset++;
            result.nValue=nSecondByte|((quint32)nMask<<8);
            nMask=0x20;
        }
        if(i==2)
        {
            if((nFirstByte&nMask)==0)
            {
                quint64 nHighPart=nFirstByte&(nMask-1);
                result.nValue|=(nHighPart<<(8*i));
                break;
            }

            quint8 nByte=read_uint8(nOffset);
            nOffset++;
            result.nValue|=((quint64)nByte)<<(8*i);
            nMask>>=1;
        }
    }

    result.nByteSize=nOffset-_nStartOffset;

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

qint64 XBinary::getOverlaySize()
{
    _MEMORY_MAP memoryMap=getMemoryMap();

    return getOverlaySize(&memoryMap);
}

qint64 XBinary::getOverlaySize(XBinary::_MEMORY_MAP *pMemoryMap)
{
    qint64 nSize=getSize();
    qint64 nOverlayOffset=getOverlayOffset(pMemoryMap);
    qint64 nDelta=0;

    if(nOverlayOffset>0)
    {
        nDelta=nSize-nOverlayOffset;
    }

    return qMax(nDelta,(qint64)0);
}

qint64 XBinary::getOverlayOffset()
{
    _MEMORY_MAP memoryMap=getMemoryMap();

    return getOverlayOffset(&memoryMap);
}

qint64 XBinary::getOverlayOffset(XBinary::_MEMORY_MAP *pMemoryMap)
{
    qint64 nResult=-1;
    qint64 nRawSize=_calculateRawSize(pMemoryMap);

    if(nRawSize)
    {
        nResult=nRawSize;
    }

    return nResult;
}

bool XBinary::isOverlayPresent()
{
    _MEMORY_MAP memoryMap=getMemoryMap();

    return isOverlayPresent(&memoryMap);
}

bool XBinary::isOverlayPresent(XBinary::_MEMORY_MAP *pMemoryMap)
{
    return (getOverlaySize(pMemoryMap)!=0);
}

bool XBinary::compareOverlay(QString sSignature, qint64 nOffset)
{
    _MEMORY_MAP memoryMap=getMemoryMap();

    return compareOverlay(&memoryMap,sSignature,nOffset);
}

bool XBinary::compareOverlay(XBinary::_MEMORY_MAP *pMemoryMap, QString sSignature, qint64 nOffset)
{
    bool bResult=false;

    if(isOverlayPresent(pMemoryMap))
    {
        qint64 nOverlayOffset=getOverlayOffset(pMemoryMap)+nOffset;

        bResult=compareSignature(pMemoryMap,sSignature,nOverlayOffset);
    }

    return bResult;
}

bool XBinary::addOverlay(char *pData, qint64 nDataSize)
{
    bool bResult=false;

    qint64 nRawSize=getOverlayOffset();

    if(resize(getDevice(),nRawSize+nDataSize))
    {
        if(nDataSize)
        {
            write_array(nRawSize,pData,nDataSize);

            bResult=true;
        }
    }

    return bResult;
}

bool XBinary::removeOverlay()
{
    return addOverlay(0,0);
}

bool XBinary::isSignatureInLoadSegmentPresent(qint32 nLoadSegment, QString sSignature)
{
    _MEMORY_MAP memoryMap=getMemoryMap();

    return isSignatureInLoadSegmentPresent(&memoryMap,nLoadSegment,sSignature);
}

bool XBinary::isSignatureInLoadSegmentPresent(XBinary::_MEMORY_MAP *pMemoryMap, qint32 nLoadSegment, QString sSignature, PDSTRUCT *pProcessData)
{
    bool bResult=false;

    qint32 nNumberOfRecords=pMemoryMap->listRecords.count();

    for(qint32 i=0;i<nNumberOfRecords;i++)
    {
        if((pMemoryMap->listRecords.at(i).type==MMT_LOADSEGMENT)&&(pMemoryMap->listRecords.at(i).nLoadSection==nLoadSegment))
        {
            if(pMemoryMap->listRecords.at(i).nOffset!=-1)
            {
                bResult=isSignaturePresent(pMemoryMap,pMemoryMap->listRecords.at(i).nOffset,pMemoryMap->listRecords.at(i).nSize,sSignature,pProcessData);

                break;
            }
        }
    }

    return bResult;
}

QString XBinary::getStringCollision(QList<QString> *pListStrings, QString sString1, QString sString2)
{
    // TODO Check&optimize
    QString sResult;

    qint32 nNumberOfStrings=pListStrings->count();

    QString sRoot1;
    QString sRoot2;

    for(qint32 i=0;i<nNumberOfStrings;i++)
    {
        QString sCurrentString=pListStrings->at(i);

        if(sCurrentString.contains(sString1))
        {
            sRoot1=sCurrentString.section(sString1,0,0);
        }

        if((sRoot1!="")&&sCurrentString.contains(sString2))
        {
            sRoot2=sCurrentString.section(sString2,0,0);

            break;
        }
    }

    if((sRoot1!="")&&(sRoot1==sRoot2))
    {
        sResult=sRoot1;
    }

    return sResult;
}

bool XBinary::writeToFile(QString sFileName, QByteArray baData)
{
    bool bResult=false;

    QFile file;
    file.setFileName(sFileName);

    if(file.open(QIODevice::ReadWrite))
    {
        file.resize(0);

        file.write(baData.data(),baData.size());

        file.close();
        bResult=true;
    }

    return bResult;
}

bool XBinary::writeToFile(QString sFileName, QIODevice *pDevice)
{
    bool bResult=false;

    if(createFile(sFileName,pDevice->size()))
    {
        QFile file;
        file.setFileName(sFileName);

        if(file.open(QIODevice::ReadWrite))
        {
            bResult=copyDeviceMemory(pDevice,0,&file,0,pDevice->size(),READWRITE_BUFFER_SIZE);

            file.close();
        }
    }

    return bResult;
}

bool XBinary::appendToFile(QString sFileName, QString sString)
{
    bool bResult=false;

    QFile file;
    file.setFileName(sFileName);

    if(file.open(QIODevice::ReadWrite|QIODevice::Append))
    {
        sString+="\r\n"; // TODO Linux
        file.write(sString.toUtf8());

        file.close();
        bResult=true;
    }

    return bResult;
}

bool XBinary::clearFile(QString sFileName)
{
    bool bResult=false;

    QFile file;
    file.setFileName(sFileName);

    if(file.open(QIODevice::ReadWrite))
    {
        file.resize(0);

        file.close();
        bResult=true;
    }

    return bResult;
}

qint32 XBinary::getStringNumberFromList(QList<QString> *pListStrings, QString sString, PDSTRUCT *pProcessData)
{
    PDSTRUCT processDataEmpty={};

    if(!pProcessData)
    {
        pProcessData=&processDataEmpty;
    }

    qint32 nResult=-1;

    qint32 nNumberOfRecords=pListStrings->count();

    for(qint32 i=0;(i<nNumberOfRecords)&&(!(pProcessData->bIsStop));i++)
    {
        if(pListStrings->at(i)==sString)
        {
            nResult=i;

            break;
        }
    }

    return nResult;
}

qint32 XBinary::getStringNumberFromListExp(QList<QString> *pListStrings, QString sString, PDSTRUCT *pProcessData)
{
    PDSTRUCT processDataEmpty={};

    if(!pProcessData)
    {
        pProcessData=&processDataEmpty;
    }

    qint32 nResult=-1;

    qint32 nNumberOfRecords=pListStrings->count();

    for(qint32 i=0;(i<nNumberOfRecords)&&(!(pProcessData->bIsStop));i++)
    {
        if(isRegExpPresent(sString,pListStrings->at(i)))
        {
            nResult=i;

            break;
        }
    }

    return nResult;
}

bool XBinary::isStringInListPresent(QList<QString> *pListStrings, QString sString, PDSTRUCT *pProcessData)
{
    return (getStringNumberFromList(pListStrings,sString,pProcessData)!=-1);
}

bool XBinary::isStringInListPresentExp(QList<QString> *pListStrings, QString sString, PDSTRUCT *pProcessData)
{
    return (getStringNumberFromListExp(pListStrings,sString,pProcessData)!=-1);
}

QString XBinary::getStringByIndex(QList<QString> *pListStrings, int nIndex, qint32 nNumberOfStrings)
{
    QString sResult;

    if(nNumberOfStrings==-1)
    {
        nNumberOfStrings=pListStrings->count();
    }

    if((nIndex>0)&&(nIndex<nNumberOfStrings))
    {
        sResult=pListStrings->at(nIndex);
    }

    return sResult;
}

bool XBinary::isStringUnicode(QString sString, qint32 nMaxCheckSize)
{
    // TODO Optimize
    // TODO Check
    bool bResult=false;

    if(nMaxCheckSize==-1)
    {
        nMaxCheckSize=sString.size();
    }
    else
    {
        nMaxCheckSize=qMin(sString.size(),nMaxCheckSize);
    }

    for(qint32 i=0;i<nMaxCheckSize;i++)
    {
        unsigned char cChar=sString.at(i).toLatin1();
        if((cChar>127)||(cChar<27))
        {
            bResult=true;
            break;
        }
    }

    return bResult;
}

quint32 XBinary::elfHash(const quint8 *pData)
{
    quint32 nResult=0;

    while(*pData)
    {
        nResult=(nResult<<4)+(*pData);

        quint32 nHigh=nResult&0xF0000000;

        if(nHigh)
        {
            nResult^=(nResult>>24);
        }

        nResult&=(~nHigh);

        pData++;
    }

    return nResult;
}

QString XBinary::getVersionString(QString sString)
{
    QString sResult;

    qint32 nSize=sString.size();

    for(qint32 i=0;i<nSize;i++)
    {
        QChar c=sString.at(i);

        if(((QChar('9')>=c)&&(c>=QChar('0')))||(c==QChar('.')))
        {
            sResult.append(c);
        }
        else
        {
            break;
        }
    }

    return sResult;
}

qint64 XBinary::getVersionIntValue(QString sString)
{
    qint64 nResult=0;

    qint32 nNumberOfDots=sString.count(QChar('.'));

    for(qint32 i=0;i<(nNumberOfDots+1);i++)
    {
        nResult*=10;

        nResult+=sString.section(QChar('.'),i,i).toLongLong();
    }

    return nResult;
}

bool XBinary::checkStringNumber(QString sString, quint32 nMin, quint32 nMax)
{
    bool bResult=false;

    quint32 nValue=sString.toUInt();

    bResult=((nValue>=nMin)&&(nValue<=nMax));

    return bResult;
}

QDateTime XBinary::valueToTime(quint64 nValue, DT_TYPE type)
{
    QDateTime result;

    if(type==DT_TYPE_POSIX)
    {
        result.setMSecsSinceEpoch(nValue*1000);
    }

    return result;
}

QString XBinary::valueToTimeString(quint64 nValue, XBinary::DT_TYPE type)
{
    QString sResult;

    sResult=valueToTime(nValue,type).toString("yyyy-MM-dd hh:mm:ss");

    return sResult;
}

QString XBinary::valueToFlagsString(quint64 nValue, QMap<quint64, QString> mapFlags, VL_TYPE vlType)
{
    QString sResult;

    if(vlType==VL_TYPE_LIST)
    {
        sResult=mapFlags.value(nValue);
    }
    else if(vlType==VL_TYPE_FLAGS)
    {
        QMapIterator<quint64,QString> iter(mapFlags);

        while(iter.hasNext())
        {
            iter.next();

            quint64 nFlag=iter.key();

            if(nValue&nFlag)
            {
                if(sResult!="")
                {
                    sResult+="|";
                }

                sResult+=iter.value();
            }
        }
    }

    return sResult;
}

bool XBinary::isX86asm(QString sArch)
{
    // TODO remove, use getDisasmMode
    bool bResult=false;

    sArch=sArch.toUpper();

    // TODO Check
    if( (sArch=="8086")||
        (sArch=="80286")||
        (sArch=="80386")||
        (sArch=="80486")||
        (sArch=="80586")||
        (sArch=="386")||
        (sArch=="I386")||
        (sArch=="AMD64")||
        (sArch=="X86_64"))
    {
        bResult=true;
    }

    return bResult;
}

QString XBinary::disasmIdToString(XBinary::DM disasmMode)
{
    QString sResult=tr("Unknown");

    switch(disasmMode)
    {
        case DM_X86_16:             sResult=QString("X86-16");          break;
        case DM_X86_32:             sResult=QString("X86-32");          break;
        case DM_X86_64:             sResult=QString("X86-64");          break;
        case DM_ARM_LE:             sResult=QString("ARM");             break;
        case DM_ARM_BE:             sResult=QString("ARM BE");          break;
        case DM_ARM64_LE:           sResult=QString("ARM64");           break;
        case DM_ARM64_BE:           sResult=QString("ARM64 BE");        break;
        case DM_CORTEXM:            sResult=QString("CORTEXM");         break;
        case DM_THUMB_LE:           sResult=QString("THUMB");           break;
        case DM_THUMB_BE:           sResult=QString("THUMB BE");        break;
        case DM_MIPS_LE:            sResult=QString("MIPS");            break;
        case DM_MIPS_BE:            sResult=QString("MIPS BE");         break;
        case DM_MIPS64_LE:          sResult=QString("MIPS64");          break;
        case DM_MIPS64_BE:          sResult=QString("MIPS64 BE");       break;
        case DM_PPC_LE:             sResult=QString("PPC");             break;
        case DM_PPC_BE:             sResult=QString("PPC BE");          break;
        case DM_PPC64_LE:           sResult=QString("PPC64");           break;
        case DM_PPC64_BE:           sResult=QString("PPC64 BE");        break;
        case DM_SPARC:              sResult=QString("SPARC");           break;
        case DM_S390X:              sResult=QString("S390X");           break;
        case DM_XCORE:              sResult=QString("XCORE");           break;
        case DM_M68K:               sResult=QString("M68K");            break;
        case DM_M68K40:             sResult=QString("M68K40");          break;
        case DM_TMS320C64X:         sResult=QString("TMS320C64X");      break;
        case DM_M6800:              sResult=QString("M6800");           break;
        case DM_M6801:              sResult=QString("M6801");           break;
        case DM_M6805:              sResult=QString("M6805");           break;
        case DM_M6808:              sResult=QString("M6808");           break;
        case DM_M6809:              sResult=QString("M6809");           break;
        case DM_M6811:              sResult=QString("M6811");           break;
        case DM_CPU12:              sResult=QString("CPU12");           break;
        case DM_HD6301:             sResult=QString("HD6301");          break;
        case DM_HD6309:             sResult=QString("HD6309");          break;
        case DM_HCS08:              sResult=QString("HCS08");           break;
        case DM_EVM:                sResult=QString("EVM");             break;
        case DM_RISKV32:            sResult=QString("RISKV32");         break;
        case DM_RISKV64:            sResult=QString("RISKV64");         break;
        case DM_RISKVC:             sResult=QString("RISKVC");          break;
        case DM_MOS65XX:            sResult=QString("MOS65XX");         break;
        case DM_WASM:               sResult=QString("WASM");            break;
        case DM_BPF_LE:             sResult=QString("BPF LE");          break;
        case DM_BPF_BE:             sResult=QString("BPF BE");          break;
    }

    return sResult;
}

QString XBinary::syntaxIdToString(SYNTAX syntax)
{
    QString sResult=tr("Unknown");

    switch(syntax)
    {
        case SYNTAX_DEFAULT:            sResult=tr("Default");              break;
        case SYNTAX_ATT:                sResult=QString("ATT");             break;
        case SYNTAX_INTEL:              sResult=QString("INTEL");           break;
        case SYNTAX_MASM:               sResult=QString("MASM");            break;
        case SYNTAX_MOTOROLA:           sResult=QString("MOTOROLA");        break;
    }

    return sResult;
}

XBinary::SYNTAX XBinary::stringToSyntaxId(QString sString)
{
    SYNTAX result=SYNTAX_DEFAULT;

    if      (sString=="ATT")        result=SYNTAX_ATT;
    else if (sString=="INTEL")      result=SYNTAX_INTEL;
    else if (sString=="MASM")       result=SYNTAX_MASM;
    else if (sString=="MOTOROLA")   result=SYNTAX_MOTOROLA;

    return result;
}

QString XBinary::osNameIdToString(OSNAME osName)
{
    QString sResult=tr("Unknown");

    switch(osName)
    {
        case OSNAME_UNIX:               sResult=QString("Unix");                            break;
        case OSNAME_POSIX:              sResult=QString("Posix");                           break;
        case OSNAME_LINUX:              sResult=QString("Linux");                           break;
        case OSNAME_WINDOWS:            sResult=QString("Windows");                         break;
        case OSNAME_WINDOWSCE:          sResult=QString("Windows CE");                      break;
        case OSNAME_MSDOS:              sResult=QString("MSDOS");                           break;
        case OSNAME_UEFI:               sResult=QString("UEFI");                            break;
        case OSNAME_XBOX:               sResult=QString("XBOX");                            break;
        case OSNAME_OS2:                sResult=QString("OS/2");                            break;
        case OSNAME_MAC_OS:             sResult=QString("Mac OS");                          break;
        case OSNAME_MAC_OS_X:           sResult=QString("Mac OS X");                        break;
        case OSNAME_OS_X:               sResult=QString("OS X");                            break;
        case OSNAME_MACOS:              sResult=QString("macOS");                           break;
        case OSNAME_IPHONEOS:           sResult=QString("iPhone OS");                       break;
        case OSNAME_IPADOS:             sResult=QString("iPadOS");                          break;
        case OSNAME_IOS:                sResult=QString("iOS");                             break;
        case OSNAME_WATCHOS:            sResult=QString("watchOS");                         break;
        case OSNAME_TVOS:               sResult=QString("tvOS");                            break;
        case OSNAME_BRIDGEOS:           sResult=QString("bridgeOS");                        break;
        case OSNAME_ANDROID:            sResult=QString("Android");                         break;
        case OSNAME_FREEBSD:            sResult=QString("FreeBSD");                         break;
        case OSNAME_OPENBSD:            sResult=QString("OpenBSD");                         break;
        case OSNAME_NETBSD:             sResult=QString("NetBSD");                          break;
        case OSNAME_HPUX:               sResult=QString("Hewlett-Packard HP-UX");           break;
        case OSNAME_SOLARIS:            sResult=QString("Sun Solaris");                     break;
        case OSNAME_AIX:                sResult=QString("AIX");                             break;
        case OSNAME_IRIX:               sResult=QString("IRIX");                            break;
        case OSNAME_TRU64:              sResult=QString("Compaq TRU64 UNIX");               break;
        case OSNAME_MODESTO:            sResult=QString("Novell Modesto");                  break;
        case OSNAME_OPENVMS:            sResult=QString("Open VMS");                        break;
        case OSNAME_FENIXOS:            sResult=QString("FenixOS");                         break;
        case OSNAME_BORLANDOSSERVICES:  sResult=QString("Borland OS Services");             break;
        case OSNAME_NSK:                sResult=QString("Hewlett-Packard Non-Stop Kernel"); break;
        case OSNAME_AROS:               sResult=QString("Amiga Research OS");               break;
        case OSNAME_UBUNTULINUX:        sResult=QString("Ubuntu Linux");                    break;
        case OSNAME_DEBIANLINUX:        sResult=QString("Debian Linux");                    break;
        case OSNAME_STARTOSLINUX:       sResult=QString("StartOS Linux");                   break;
        case OSNAME_GENTOOLINUX:        sResult=QString("Gentoo Linux");                    break;
        case OSNAME_ALPINELINUX:        sResult=QString("Alpine Linux");                    break;
        case OSNAME_WINDRIVERLINUX:     sResult=QString("Wind River Linux");                break;
        case OSNAME_SUSELINUX:          sResult=QString("SUSE Linux");                      break;
        case OSNAME_MANDRAKELINUX:      sResult=QString("Mandrake Linux");                  break;
        case OSNAME_ASPLINUX:           sResult=QString("ASPLinux");                        break;
        case OSNAME_REDHATLINUX:        sResult=QString("Red Hat Linux");                   break;
        case OSNAME_HANCOMLINUX:        sResult=QString("Hancom Linux");                    break;
        case OSNAME_TURBOLINUX:         sResult=QString("Turbolinux");                      break;
        case OSNAME_VINELINUX:          sResult=QString("Vine Linux");                      break;
        case OSNAME_SUNOS:              sResult=QString("SunOS");                           break;
        case OSNAME_OPENVOS:            sResult=QString("Open VOS");                        break;
        case OSNAME_MCLINUX:            sResult=QString("mClinux");                         break;
        case OSNAME_QNX:                sResult=QString("QNX");                             break;
        case OSNAME_SYLLABLE:           sResult=QString("Syllable ");                       break;
    }

    return sResult;
}

XBinary::DM XBinary::getDisasmMode()
{
    _MEMORY_MAP memoryMap=getMemoryMap();

    return getDisasmMode(&memoryMap);
}

XBinary::DM XBinary::getDisasmMode(XBinary::_MEMORY_MAP *pMemoryMap)
{
    return getDisasmMode(pMemoryMap->sArch,pMemoryMap->bIsBigEndian,pMemoryMap->mode);
}

XBinary::DM XBinary::getDisasmMode(QString sArch,bool bIsBigEndian,MODE mode)
{
    XBinary::DM dmResult=DM_X86_16;

    if(sArch=="PPC")
    {
        if(bIsBigEndian)
        {
            dmResult=DM_PPC_BE;
        }
        else
        {
            dmResult=DM_PPC_LE;
        }
    }
    else if(sArch=="PPC64")
    {
        if(bIsBigEndian)
        {
            dmResult=DM_PPC64_BE;
        }
        else
        {
            dmResult=DM_PPC64_LE;
        }
    }
    else if((sArch=="MIPS")||
            (sArch=="R3000")||
            (sArch=="R4000")||
            (sArch=="R10000")||
            (sArch=="WCEMIPSV2"))
    {        
        if(bIsBigEndian)
        {
            dmResult=DM_MIPS_BE;
        }
        else
        {
            dmResult=DM_MIPS_LE;
        }
    }
    else if((sArch=="ARM")||(sArch=="ARM_V6")||(sArch=="ARM_V7"))
    {
        if(bIsBigEndian)
        {
            dmResult=DM_ARM_BE;
        }
        else
        {
            dmResult=DM_ARM_LE;
        }
    }
    else if((sArch=="AARCH64")||
            (sArch=="ARM64"))
    {
        if(bIsBigEndian)
        {
            dmResult=DM_ARM64_BE;
        }
        else
        {
            dmResult=DM_ARM64_LE;
        }
    }
    else if(sArch=="8086") // TODO
    {
        dmResult=DM_X86_16;
    }
    else if((sArch=="386")||
            (sArch=="80386")||
            (sArch=="80486")||
            (sArch=="80586")||
            (sArch=="I386")||
            (sArch=="486")||
            (sArch=="X86"))
    {
        dmResult=DM_X86_32;
    }
    else if((sArch=="AMD64")||
            (sArch=="X86_64")||
            (sArch=="X64"))
    {
        dmResult=DM_X86_64;
    }
    else if((sArch=="68K")||
            (sArch=="MC680x0")||
            (sArch=="MC68030"))
    {
        dmResult=DM_M68K;
    }
    else if(sArch=="MC68040")
    {
        dmResult=DM_M68K40;
    }
    else if(sArch=="POWERPC")
    {
        if(bIsBigEndian)
        {
            dmResult=DM_PPC_BE;
        }
        else
        {
            dmResult=DM_PPC_LE;
        }
    }
    else if(sArch=="SPARC")
    {
        dmResult=DM_SPARC;
    }
    else if((sArch=="RISC_V")||
            (sArch=="RISCV32")||
            (sArch=="RISCV64"))
    {
        if(mode==MODE_64)
        {
            dmResult=DM_RISKV64;
        }
        else
        {
            dmResult=DM_RISKV32;
        }
    }
    // TODO SH

    return dmResult;
}

XBinary::DMFAMILY XBinary::getDisasmFamily(XBinary::DM disasmMode)
{
    DMFAMILY result=DMFAMILY_UNKNOWN;

    if((disasmMode==DM_X86_16)||(disasmMode==DM_X86_32)||(disasmMode==DM_X86_64))
    {
        result=DMFAMILY_X86;
    }
    else if((disasmMode==DM_ARM_BE)||(disasmMode==DM_ARM_LE))
    {
        result=DMFAMILY_ARM;
    }
    else if((disasmMode==DM_ARM64_BE)||(disasmMode==DM_ARM64_LE))
    {
        result=DMFAMILY_ARM64;
    }
    else if((disasmMode==DM_MIPS64_BE)||(disasmMode==DM_MIPS64_LE)||
            (disasmMode==DM_MIPS_BE)||(disasmMode==DM_MIPS_LE))
    {
        result=DMFAMILY_MIPS;
    }
    else if((disasmMode==DM_PPC64_BE)||(disasmMode==DM_PPC64_LE)||
            (disasmMode==DM_PPC_BE)||(disasmMode==DM_PPC_LE))
    {
        result=DMFAMILY_PPC;
    }
    else if(disasmMode==DM_SPARC)
    {
        result=DMFAMILY_SPARC;
    }
    else if(disasmMode==DM_S390X)
    {
        result=DMFAMILY_SYSZ;
    }
    else if(disasmMode==DM_XCORE)
    {
        result=DMFAMILY_XCORE;
    }
    else if((disasmMode==DM_M68K)||(disasmMode==DM_M68K40))
    {
        result=DMFAMILY_M68K;
    }
    else if((disasmMode==DM_M6800)||(disasmMode==DM_M6801)||
            (disasmMode==DM_M6805)||(disasmMode==DM_M6808)||
            (disasmMode==DM_M6809)||(disasmMode==DM_M6811)||
            (disasmMode==DM_CPU12)||(disasmMode==DM_HD6301)||
            (disasmMode==DM_HD6309)||(disasmMode==DM_HCS08))
    {
        result=DMFAMILY_M68OK;
    }
    else if((disasmMode==DM_RISKV32)||(disasmMode==DM_RISKV64)||(disasmMode==DM_RISKVC))
    {
        result=DMFAMILY_RISCV;
    }
    else if(disasmMode==DM_EVM)
    {
        result=DMFAMILY_EVM;
    }
    else if(disasmMode==DM_MOS65XX)
    {
        result=DMFAMILY_MOS65XX;
    }
    else if(disasmMode==DM_WASM)
    {
        result=DMFAMILY_WASM;
    }
    else if((disasmMode==DM_BPF_LE)||(disasmMode==DM_BPF_BE))
    {
        result=DMFAMILY_BPF;
    }

    return result;
}

XBinary::DMFAMILY XBinary::getDisasmFamily(XBinary::_MEMORY_MAP *pMemoryMap)
{
    return getDisasmFamily(getDisasmMode(pMemoryMap));
}

QList<XBinary::SYNTAX> XBinary::getDisasmSyntax(DM disasmMode)
{
    QList<SYNTAX> listResult;

    listResult.append(SYNTAX_DEFAULT);

    if(getDisasmFamily(disasmMode)==DMFAMILY_X86)
    {
        listResult.append(SYNTAX_ATT);
        listResult.append(SYNTAX_INTEL);
        listResult.append(SYNTAX_MASM);
    }

    // TODO Motorola

    return listResult;
}

bool XBinary::checkFileType(XBinary::FT fileTypeMain, XBinary::FT fileTypeOptional)
{
    bool bResult=false;

    if((fileTypeMain==FT_PE)&&((fileTypeOptional==FT_PE)||(fileTypeOptional==FT_PE32)||(fileTypeOptional==FT_PE64)))
    {
        bResult=true;
    }
    else if((fileTypeMain==FT_ELF)&&((fileTypeOptional==FT_ELF)||(fileTypeOptional==FT_ELF32)||(fileTypeOptional==FT_ELF64)))
    {
        bResult=true;
    }
    else if((fileTypeMain==FT_MACHO)&&((fileTypeOptional==FT_MACHO)||(fileTypeOptional==FT_MACHO32)||(fileTypeOptional==FT_MACHO64)))
    {
        bResult=true;
    }
    else if(fileTypeMain==fileTypeOptional)
    {
        bResult=true;
    }

    return  bResult;
}

void XBinary::filterFileTypes(QSet<XBinary::FT> *pStFileTypes)
{
    // TODO Check!
    // TODO optimize! new Types create remove function
    if( pStFileTypes->contains(XBinary::FT_MSDOS)||
        pStFileTypes->contains(XBinary::FT_NE)||
        pStFileTypes->contains(XBinary::FT_LE)||
        pStFileTypes->contains(XBinary::FT_LX)||
        pStFileTypes->contains(XBinary::FT_PE)||
        pStFileTypes->contains(XBinary::FT_PE32)||
        pStFileTypes->contains(XBinary::FT_PE64)||
        pStFileTypes->contains(XBinary::FT_ELF)||
        pStFileTypes->contains(XBinary::FT_ELF32)||
        pStFileTypes->contains(XBinary::FT_ELF64)||
        pStFileTypes->contains(XBinary::FT_MACHO)||
        pStFileTypes->contains(XBinary::FT_MACHO32)||
        pStFileTypes->contains(XBinary::FT_MACHO64)||
        pStFileTypes->contains(XBinary::FT_DEX)||
        pStFileTypes->contains(XBinary::FT_ZIP)||
        pStFileTypes->contains(XBinary::FT_GZIP))
    {
        pStFileTypes->remove(XBinary::FT_BINARY);
    }
    else
    {
        pStFileTypes->insert(XBinary::FT_COM);
    }
}

void XBinary::filterFileTypes(QSet<XBinary::FT> *pStFileTypes, XBinary::FT fileType)
{
    // TODO Check!
    if(fileType!=XBinary::FT_UNKNOWN)
    {
        QSet<XBinary::FT> stFileTypesNew;

        if(fileType==XBinary::FT_PE)
        {
            if(pStFileTypes->contains(XBinary::FT_PE)) stFileTypesNew.insert(XBinary::FT_PE);
            if(pStFileTypes->contains(XBinary::FT_PE32)) stFileTypesNew.insert(XBinary::FT_PE32);
            if(pStFileTypes->contains(XBinary::FT_PE64)) stFileTypesNew.insert(XBinary::FT_PE64);
        }
        else if(fileType==XBinary::FT_ELF)
        {
            if(pStFileTypes->contains(XBinary::FT_ELF)) stFileTypesNew.insert(XBinary::FT_ELF);
            if(pStFileTypes->contains(XBinary::FT_ELF32)) stFileTypesNew.insert(XBinary::FT_ELF32);
            if(pStFileTypes->contains(XBinary::FT_ELF64)) stFileTypesNew.insert(XBinary::FT_ELF64);
        }
        else if(fileType==XBinary::FT_MACHO)
        {
            if(pStFileTypes->contains(XBinary::FT_MACHO)) stFileTypesNew.insert(XBinary::FT_MACHO);
            if(pStFileTypes->contains(XBinary::FT_MACHO32)) stFileTypesNew.insert(XBinary::FT_MACHO32);
            if(pStFileTypes->contains(XBinary::FT_MACHO64)) stFileTypesNew.insert(XBinary::FT_MACHO64);
        }
        else
        {
            if(pStFileTypes->contains(fileType)) stFileTypesNew.insert(fileType);
        }

        *pStFileTypes=stFileTypesNew;
    }
}

bool XBinary::isFileTypePresent(QSet<XBinary::FT> *pStFileTypes, QSet<XBinary::FT> *pStAvailableFileTypes)
{
    bool bResult=false;

    QSet<XBinary::FT>::iterator i=pStFileTypes->begin();

    while(i!=pStFileTypes->end())
    {
        if(pStAvailableFileTypes->contains(*i))
        {
            bResult=true;

            break;
        }

        i++;
    }

    return bResult;
}

XBinary::PROCENT XBinary::procentInit(qint64 nMaxValue,bool bTimer)
{
    PROCENT result={};
    result.bTimer=bTimer;

    result.nMaxValue=nMaxValue;

    if(!(result.bTimer))
    {
        result.nMaxProcent=1;

        if(result.nMaxValue>0x100000000)
        {
            result.nMaxProcent=100;
        }
        else if(result.nMaxValue>0x100000)
        {
            result.nMaxProcent=10;
        }
        else if(result.nMaxValue>0x1000)
        {
            result.nMaxProcent=5;
        }
    }
    else
    {
        result.timer.start();
        result.nMaxProcent=100;
    }

    return result;
}

bool XBinary::procentSetCurrentValue(XBinary::PROCENT *pProcent, qint64 nCurrentValue)
{
    bool bResult=false;

    pProcent->nCurrentValue=nCurrentValue;

    if(!(pProcent->bTimer))
    {
        if(pProcent->nCurrentValue>((pProcent->nCurrentProcent+1)*(pProcent->nMaxValue/pProcent->nMaxProcent)))
        {
            pProcent->nCurrentProcent++;
            bResult=true;
        }
    }
    else
    {
        if(pProcent->timer.elapsed()>=1000) // TODO Check speed
        {
            pProcent->timer.restart();

            qint32 _nCurrent=(pProcent->nCurrentValue*100)/(pProcent->nMaxValue);

            pProcent->nCurrentProcent=_nCurrent;
            bResult=true;
        }
    }

    return bResult;
}

qint64 XBinary::getTotalOSSize(QList<OFFSETSIZE> *pListOffsetSize)
{
    qint64 nResult=0;

    qint32 nNumberOfRecords=pListOffsetSize->count();

    for(qint32 i=0;i<nNumberOfRecords;i++)
    {
        nResult+=pListOffsetSize->at(i).nSize;
    }

    return nResult;
}

XBinary::MODE XBinary::getWidthModeFromSize(quint64 nSize)
{
    MODE result=MODE_32;

    if(((quint64)nSize)>=0xFFFFFFFF)
    {
        result=MODE_64;
    }
    else if(((quint64)nSize)>=0xFFFF)
    {
        result=MODE_32;
    }
    else if(((quint64)nSize)>=0xFF)
    {
        result=MODE_16;
    }
    else
    {
        result=MODE_8;
    }

    return result;
}

XBinary::MODE XBinary::getWidthModeFromMemoryMap(XBinary::_MEMORY_MAP *pMemoryMap)
{
    MODE result=MODE_32;

    qint64 nMax=qMax(pMemoryMap->nModuleAddress+pMemoryMap->nImageSize,(XADDR)(pMemoryMap->nRawSize));

    result=getWidthModeFromSize(nMax);

    return result;
}

XBinary::MODE XBinary::getWidthModeFromByteSize(quint32 nByteSize)
{
    MODE result=MODE_32;

    if(nByteSize==2)
    {
        result=MODE_8;
    }
    else if(nByteSize==4)
    {
        result=MODE_16;
    }
    else if(nByteSize==8)
    {
        result=MODE_32;
    }
    else if(nByteSize==16)
    {
        result=MODE_64;
    }

    return result;
}

bool XBinary::isAnsiSymbol(quint8 cCode, bool bExtra)
{
    bool bResult=false;

    if(!bExtra)
    {
        if((cCode>=20)&&(cCode<0x80))
        {
            bResult=true;
        }
    }
    else
    {
        if(cCode>=20)
        {
            bResult=true;
        }
    }

    return bResult;
}

bool XBinary::isUTF8Symbol(quint8 cCode, qint32 *pnWidth)
{
    bool bResult=false;

    if(cCode>=20)
    {
        if((cCode>>7)==0)
        {
            *pnWidth=1;
            bResult=true;
        }
        else if((cCode>>5)==0x6)
        {
            *pnWidth=2;
            bResult=true;
        }
        else if((cCode>>4)==0xE)
        {
            *pnWidth=3;
            bResult=true;
        }
        else if((cCode>>3)==0x1E)
        {
            *pnWidth=4;
            bResult=true;
        }
        else if((cCode>>6)==0x2) // Rest bytes
        {
            *pnWidth=0;
            bResult=true;
        }
    }

    return bResult;
}

bool XBinary::isUnicodeSymbol(quint16 nCode, bool bExtra)
{
    bool bResult=false;

    if(!bExtra)
    {
        if((nCode>=20)&&(nCode<0x80))
        {
            bResult=true;
        }
    }
    else
    {
        if((nCode>=20)&&(nCode<=0xFF))
        {
            bResult=true;
        }
        else if((nCode>=0x0400)&&(nCode<=0x04FF)) // Cyrillic
        {
            bResult=true;
        }
    }

    return bResult;
}

QString XBinary::getStringFromIndex(qint64 nOffset, qint64 nSize, qint32 nIndex)
{
    QString sResult;

    if(nIndex<nSize)
    {
        sResult=read_ansiString(nOffset+nIndex);
    }

    return sResult;
}

QList<QString> XBinary::getAllFilesFromDirectory(QString sDirectory, QString sExtension)
{
    QDir directory(sDirectory);

    return directory.entryList(QStringList()<<sExtension,QDir::Files);
}

QList<XBinary::OPCODE> XBinary::getOpcodes(qint64 nOffset, XADDR nStartAddress, qint64 nSize, quint32 nType)
{
    QList<OPCODE> listResult;

    OFFSETSIZE osRegion=convertOffsetAndSize(nOffset,nSize);

    nOffset=osRegion.nOffset;
    nSize=osRegion.nSize;

    if(nOffset!=-1)
    {
        char *pBuffer=new char[READWRITE_BUFFER_SIZE];

        while(nSize>0)
        {
            qint64 nTempSize=qMin((qint64)READWRITE_BUFFER_SIZE,nSize);

            if(read_array(nOffset,pBuffer,nTempSize)!=nTempSize)
            {
                _errorMessage(tr("Read error"));

                break;
            }

            qint64 nOpcodesSize=0;
            OPCODE_STATUS opcodeStatus=OPCODE_STATUS_SUCCESS;

            for(qint32 i=0;i<nTempSize;)
            {
                qint64 _nSize=readOpcodes(nType,pBuffer+i,nStartAddress+i,nTempSize-i,&listResult,&opcodeStatus);

                i+=_nSize;
                nOpcodesSize+=_nSize;

                if((_nSize==0)||(opcodeStatus==OPCODE_STATUS_END))
                {
                    break;
                }
            }

            if((nOpcodesSize==0)||(opcodeStatus==OPCODE_STATUS_END))
            {
//                _errorMessage(tr("Read error"));
                break;
            }

            nSize-=nOpcodesSize;
            nOffset+=nOpcodesSize;
            nStartAddress+=nOpcodesSize;
        }

        delete[] pBuffer;
    }

    return listResult;
}

XADDR XBinary::readOpcodes(quint32 nType, char *pData, XADDR nStartAddress, qint64 nSize, QList<OPCODE> *pListOpcodes, OPCODE_STATUS *pOpcodeStatus)
{
    Q_UNUSED(nType)
    Q_UNUSED(pData)
    Q_UNUSED(nStartAddress)
    Q_UNUSED(nSize)
    Q_UNUSED(pListOpcodes)

    *pOpcodeStatus=OPCODE_STATUS_END;

    return false;
}

bool XBinary::_read_opcode_uleb128(OPCODE *pOpcode, char **ppData, qint64 *pnSize, XADDR *pnAddress, XADDR *pnResult, QString sPrefix)
{
    bool bResult=false;

    if(*pnSize>0)
    {
        PACKED_INT uleb128=_read_uleb128(*ppData,*pnSize);

        if(uleb128.bIsValid)
        {
            pOpcode->nAddress=*pnAddress;
            pOpcode->nSize=uleb128.nByteSize;
            pOpcode->sName=QString("%1(%2)").arg(sPrefix,QString::number((qint64)uleb128.nValue));

            *pnSize-=pOpcode->nSize;
            *ppData+=pOpcode->nSize;
            *pnResult+=pOpcode->nSize;
            *pnAddress+=pOpcode->nSize;

            bResult=true;
        }
    }

    return bResult;
}

bool XBinary::_read_opcode_ansiString(XBinary::OPCODE *pOpcode, char **ppData, qint64 *pnSize, XADDR *pnAddress, XADDR *pnResult, QString sPrefix)
{
    bool bResult=false;

    if(*pnSize>0)
    {
        QString sString=*ppData;

        if(sString.size()<(*pnSize)) // We need cstring with \0
        {
            pOpcode->nAddress=*pnAddress;
            pOpcode->nSize=sString.size()+1;
            pOpcode->sName=QString("%1(\"%2\")").arg(sPrefix,sString);

            *pnSize-=pOpcode->nSize;
            *ppData+=pOpcode->nSize;
            *pnResult+=pOpcode->nSize;
            *pnAddress+=pOpcode->nSize;

            bResult=true;
        }
    }

    return bResult;
}

QList<quint32> XBinary::get_uint32_list(qint64 nOffset, qint32 nNumberOfRecords, bool bIsBigEndian)
{
    QList<quint32> listResult;

    for(qint32 i=0;i<nNumberOfRecords;i++)
    {
        quint32 nRecord=read_uint32(nOffset+i*sizeof(quint32),bIsBigEndian);

        listResult.append(nRecord);
    }

    return listResult;
}

QList<quint64> XBinary::get_uint64_list(qint64 nOffset, qint32 nNumberOfRecords, bool bIsBigEndian)
{
    QList<quint64> listResult;

    for(qint32 i=0;i<nNumberOfRecords;i++)
    {
        quint64 nRecord=read_uint64(nOffset+i*sizeof(quint64),bIsBigEndian);

        listResult.append(nRecord);
    }

    return listResult;
}

bool XBinary::_isOffsetsCrossed(qint64 nOffset1, qint64 nSize1, qint64 nOffset2, qint64 nSize2)
{
    bool bResult=false;

    if(((nOffset2>=nOffset1)&&((nOffset1+nSize1)>nOffset2))||((nOffset1>=nOffset2)&&((nOffset2+nSize2)>nOffset1)))
    {
        bResult=true;
    }

    return bResult;
}

bool XBinary::_isReplaced(qint64 nOffset, qint64 nSize, QList<XBinary::MEMORY_REPLACE> *pListMemoryReplace)
{
    bool bResult=false;

    qint32 nNumberOfRecords=pListMemoryReplace->count();

    for(qint32 i=0;i<nNumberOfRecords;i++)
    {
        if(_isOffsetsCrossed(nOffset,nSize,pListMemoryReplace->at(i).nOffset,pListMemoryReplace->at(i).nSize))
        {
            bResult=true;
            break;
        }
    }

    return bResult;
}

bool XBinary::_replaceMemory(qint64 nDataOffset, char *pData, qint64 nDataSize, QList<XBinary::MEMORY_REPLACE> *pListMemoryReplace)
{
    bool bResult=false;

    qint32 nNumberOfRecords=pListMemoryReplace->count();

    for(qint32 i=0;i<nNumberOfRecords;i++)
    {
        if(_isOffsetsCrossed(nDataOffset,nDataSize,pListMemoryReplace->at(i).nOffset,pListMemoryReplace->at(i).nSize))
        {
            MEMORY_REPLACE memoryReplace=pListMemoryReplace->at(i);

            for(qint32 j=0;j<memoryReplace.nSize;j++)
            {
                bResult=true;

                pData[(memoryReplace.nOffset+j)-nDataOffset]=memoryReplace.baOriginal.data()[j];

                if((memoryReplace.nOffset+j)>(nDataOffset+nDataSize))
                {
                    break;
                }
            }
        }
    }

    return bResult;
}

bool XBinary::_updateReplaces(qint64 nDataOffset, char *pData, qint64 nDataSize, QList<MEMORY_REPLACE> *pListMemoryReplace)
{
    bool bResult=false;

    qint32 nNumberOfRecords=pListMemoryReplace->count();

    for(qint32 i=0;i<nNumberOfRecords;i++)
    {
        if(_isOffsetsCrossed(nDataOffset,nDataSize,pListMemoryReplace->at(i).nOffset,pListMemoryReplace->at(i).nSize))
        {
            MEMORY_REPLACE memoryReplace=pListMemoryReplace->at(i);

            for(qint32 j=0;j<memoryReplace.nSize;j++)
            {
                bResult=true;

                memoryReplace.baOriginal.data()[j]=pData[(memoryReplace.nOffset+j)-nDataOffset];

                if((memoryReplace.nOffset+j)>(nDataOffset+nDataSize))
                {
                    break;
                }
            }
        }
    }

    return bResult;
}

QList<XBinary::SYMBOL_RECORD> XBinary::getSymbolRecords(XBinary::_MEMORY_MAP *pMemoryMap,SYMBOL_TYPE symbolType)
{
    Q_UNUSED(pMemoryMap)
    Q_UNUSED(symbolType)

    QList<XBinary::SYMBOL_RECORD> listResult;

    // TODO

    return listResult;
}

XBinary::SYMBOL_RECORD XBinary::findSymbolByAddress(QList<SYMBOL_RECORD> *pListSymbolRecords, XADDR nAddress)
{
    SYMBOL_RECORD result={};

    qint32 nNumberOfRecords=pListSymbolRecords->count();

    for(qint32 i=0;i<nNumberOfRecords;i++)
    {
        if(pListSymbolRecords->at(i).nAddress==nAddress)
        {
            result=pListSymbolRecords->at(i);

            break;
        }
    }

    return result;
}

XBinary::SYMBOL_RECORD XBinary::findSymbolByName(QList<SYMBOL_RECORD> *pListSymbolRecords, QString sName)
{
    SYMBOL_RECORD result={};

    qint32 nNumberOfRecords=pListSymbolRecords->count();

    for(qint32 i=0;i<nNumberOfRecords;i++)
    {
        if(pListSymbolRecords->at(i).sName==sName)
        {
            result=pListSymbolRecords->at(i);

            break;
        }
    }

    return result;
}

XBinary::SYMBOL_RECORD XBinary::findSymbolByOrdinal(QList<SYMBOL_RECORD> *pListSymbolRecords, qint32 nOrdinal)
{
    SYMBOL_RECORD result={};

    qint32 nNumberOfRecords=pListSymbolRecords->count();

    for(qint32 i=0;i<nNumberOfRecords;i++)
    {
        if(pListSymbolRecords->at(i).nOrdinal==nOrdinal)
        {
            result=pListSymbolRecords->at(i);

            break;
        }
    }

    return result;
}

QString XBinary::generateUUID()
{
    return QUuid::createUuid().toString().remove("{").remove("}");
}

QString XBinary::appendText(QString sResult, QString sString, QString sSeparate)
{
    if(sString!="")
    {
        if(sResult!="") sResult+=sSeparate;
        sResult+=sString;
    }

    return sResult;
}

QString XBinary::bytesCountToString(quint64 nValue, quint64 nBase)
{
    QString sResult;

    if(nBase!=1024)
    {
        nBase=1000;
    }

    QString sValue;
    QString sUnit;

    if(nValue<nBase)
    {
        sValue=QString::number(nValue);
        sUnit=tr("Bytes");
    }
    else if(nValue<(nBase*nBase))
    {
        sValue=QString::number((double)nValue/nBase,'f',2);

        if (nBase==1024) sUnit=tr("KiB");
        else tr("kB");
    }
    else if(nValue<(nBase*nBase*nBase))
    {
        sValue=QString::number((double)nValue/(nBase*nBase),'f',2);

        if (nBase==1024) sUnit=tr("MiB");
        else tr("MB");
    }
    else if(nValue<(nBase*nBase*nBase*nBase))
    {
        sValue=QString::number((double)nValue/(nBase*nBase*nBase),'f',2);

        if (nBase==1024) sUnit=tr("GiB");
        else tr("GB");
    }
    else
    {
        sValue=QString::number((double)nValue/(nBase*nBase*nBase*nBase),'f',2);

        if (nBase==1024) sUnit=tr("TiB");
        else tr("TB");
    }

    sResult=QString("%1 %2").arg(sValue,sUnit);

    return sResult;
}

QString XBinary::numberToString(quint64 nValue)
{
    QString sResult;

    sResult=QString("\"%1\"").arg(nValue);

    return sResult;
}

QString XBinary::fullVersionDwordToString(quint32 nValue)
{
    QString sResult;

    sResult=QString("\"%1\"").arg(get_uint32_full_version(nValue));

    return sResult;
}

QString XBinary::versionDwordToString(quint32 nValue)
{
    QString sResult;

    sResult=QString("\"%1\"").arg(get_uint32_version(nValue));

    return sResult;
}

QString XBinary::formatXML(QString sXML)
{
    QString sResult;

    QXmlStreamReader reader(sXML);
    QXmlStreamWriter writer(&sResult);
    writer.setAutoFormatting(true);

    while(!reader.atEnd())
    {
        reader.readNext();
        if(!reader.isWhitespace())
        {
            writer.writeCurrentToken(reader);
        }
    }

    return sResult;
}

quint32 XBinary::make_dword(XBinary::XDWORD xdword)
{
    quint32 nResult=0;

    nResult+=(xdword.nValue1)<<16;
    nResult+=xdword.nValue2;

    return nResult;
}

quint32 XBinary::make_dword(quint16 nValue1, quint16 nValue2)
{
    XDWORD xdword={};
    xdword.nValue1=nValue1;
    xdword.nValue2=nValue2;

    return make_dword(xdword);
}

XBinary::XDWORD XBinary::make_xdword(quint32 nValue)
{
    XDWORD result={};

    result.nValue1=(quint16)(nValue>>16);
    result.nValue2=(quint16)(nValue&0xFFFF);

    return result;
}

QString XBinary::recordFilePartIdToString(FILEPART id)
{
    QString sResult=tr("Unknown");

    switch(id)
    {
        // TODO more
        case FILEPART_UNKNOWN:                          sResult=tr("Unknown");                                          break;
        case FILEPART_HEADER:                           sResult=tr("Header");                                           break;
        case FILEPART_OVERLAY:                          sResult=tr("Overlay");                                          break;
        case FILEPART_ARCHIVERECORD:                    sResult=tr("Archive record");                                   break;
        case FILEPART_RESOURCE:                         sResult=tr("Resource");                                         break;
    }

    return sResult;
}

QString XBinary::createTypeString(const SCANSTRUCT *pScanStruct)
{
    QString sResult;

    if(pScanStruct->parentId.filePart!=XBinary::FILEPART_HEADER)
    {
        sResult+=XBinary::recordFilePartIdToString(pScanStruct->parentId.filePart);

        if(pScanStruct->parentId.sVersion!="")
        {
            sResult+=QString("(%1)").arg(pScanStruct->parentId.sVersion);
        }

        if(pScanStruct->parentId.sInfo!="")
        {
            sResult+=QString("[%1]").arg(pScanStruct->parentId.sInfo);
        }

        sResult+=": ";
    }

    sResult+=XBinary::fileTypeIdToString(pScanStruct->id.fileType);

    return sResult;
}

XBinary::SCANSTRUCT XBinary::createHeaderScanStruct(const SCANSTRUCT *pScanStruct)
{
    SCANSTRUCT result=*pScanStruct;

    result.id.sUuid=XBinary::generateUUID();
    result.sType="";
    result.sName="";
    result.sVersion="";
    result.sInfo="";
    result.globalColor=Qt::transparent;

    return result;
}

QString XBinary::createResultString2(const SCANSTRUCT *pScanStruct)
{
    QString sResult;

    if(pScanStruct->bIsHeuristic)
    {
        sResult+="(Heuristic)";
    }

    sResult+=QString("%1: %2").arg(pScanStruct->sType,pScanStruct->sName);

    if(pScanStruct->sVersion!="")
    {
        sResult+=QString("(%1)").arg(pScanStruct->sVersion);
    }

    if(pScanStruct->sInfo!="")
    {
        sResult+=QString("[%1]").arg(pScanStruct->sInfo);
    }

    return sResult;
}

bool XBinary::checkVersionString(QString sVersion)
{
    bool bResult=false;

    if(sVersion.trimmed()!="")
    {
        bResult=true;

        qint32 nStringSize=sVersion.size();

        // TODO
        for(qint32 i=0;i<nStringSize;i++)
        {
            QChar _char=sVersion.at(i);

            if((_char>=QChar('0'))&&(_char<=QChar('9')))
            {

            }
            else if(_char==QChar('.'))
            {

            }
            else
            {
                bResult=false;
                break;
            }
        }
    }

    return bResult;
}

XBinary::XVARIANT XBinary::getXVariant(bool bValue)
{
    XVARIANT result={};

    result.mode=MODE_BIT;
    result.var.v_bool=bValue;

    return result;
}

XBinary::XVARIANT XBinary::getXVariant(quint8 nValue)
{
    XVARIANT result={};

    result.mode=MODE_8;
    result.var.v_uint8=nValue;

    return result;
}

XBinary::XVARIANT XBinary::getXVariant(quint16 nValue, bool bIsBigEndian)
{
    XVARIANT result={};

    result.bIsBigEndian=bIsBigEndian;
    result.mode=MODE_16;
    result.var.v_uint16=nValue;

    return result;
}

XBinary::XVARIANT XBinary::getXVariant(quint32 nValue, bool bIsBigEndian)
{
    XVARIANT result={};

    result.bIsBigEndian=bIsBigEndian;
    result.mode=MODE_32;
    result.var.v_uint32=nValue;

    return result;
}

XBinary::XVARIANT XBinary::getXVariant(quint64 nValue, bool bIsBigEndian)
{
    XVARIANT result={};

    result.bIsBigEndian=bIsBigEndian;
    result.mode=MODE_64;
    result.var.v_uint64=nValue;

    return result;
}

XBinary::XVARIANT XBinary::getXVariant(XUINT128 value, bool bIsBigEndian)
{
    XVARIANT result={};

    result.bIsBigEndian=bIsBigEndian;
    result.mode=MODE_128;
    result.var.v_uint128=value;

    return result;
}

XBinary::XVARIANT XBinary::getXVariant(quint64 nLow, quint64 nHigh, bool bIsBigEndian)
{
    XVARIANT result={};

    result.bIsBigEndian=bIsBigEndian;
    result.mode=MODE_128;
    result.var.v_uint128.high=nHigh;
    result.var.v_uint128.low=nLow;

    return result;
}

quint64 XBinary::xVariantToQword(XVARIANT xvariant)
{
    quint64 nResult=0;

    if      (xvariant.mode==MODE_8)     nResult=xvariant.var.v_uint8;
    else if (xvariant.mode==MODE_16)    nResult=xvariant.var.v_uint16;
    else if (xvariant.mode==MODE_32)    nResult=xvariant.var.v_uint32;
    else if (xvariant.mode==MODE_64)    nResult=xvariant.var.v_uint64;

    return nResult;
}

quint32 XBinary::getDwordFromQword(quint64 nValue, qint32 nIndex)
{
    quint32 nResult=0;

    if(nIndex<2)
    {
        nValue=nValue>>(nIndex*32);

        nResult=nValue&0xFFFFFFFF;
    }

    return nResult;
}

quint16 XBinary::getWordFromQword(quint64 nValue, qint32 nIndex)
{
    quint16 nResult=0;

    if(nIndex<4)
    {
        nValue=nValue>>(nIndex*16);

        nResult=nValue&0xFFFF;
    }

    return nResult;
}

quint8 XBinary::getByteFromQword(quint64 nValue, qint32 nIndex)
{
    quint8 nResult=0;

    if(nIndex<8)
    {
        nValue=nValue>>(nIndex*8);

        nResult=nValue&0xFF;
    }

    return nResult;
}

quint64 XBinary::setDwordToQword(quint64 nInit, quint32 nValue, qint32 nIndex)
{
    quint64 nResult=nInit;

    if(nIndex<2)
    {
        quint64 nFF=0xFFFFFFFF;
        quint64 _nValue=nValue;

        nFF=nFF<<(nIndex*32);
        _nValue=_nValue<<(nIndex*32);

        nResult=nResult&(~nFF);
        nResult=nResult|_nValue;
    }

    return nResult;
}

quint64 XBinary::setWordToQword(quint64 nInit, quint16 nValue, qint32 nIndex)
{
    quint64 nResult=nInit;

    if(nIndex<4)
    {
        quint64 nFF=0xFFFF;
        quint64 _nValue=nValue;

        nFF=nFF<<(nIndex*16);
        _nValue=_nValue<<(nIndex*16);

        nResult=nResult&(~nFF);
        nResult=nResult|_nValue;
    }

    return nResult;
}

quint64 XBinary::setByteToQword(quint64 nInit, quint8 nValue, qint32 nIndex)
{
    quint64 nResult=nInit;

    if(nIndex<8)
    {
        quint64 nFF=0xFF;
        quint64 _nValue=nValue;

        nFF=nFF<<(nIndex*8);
        _nValue=_nValue<<(nIndex*8);

        nResult=nResult&(~nFF);
        nResult=nResult|_nValue;
    }

    return nResult;
}

bool XBinary::isXVariantEqual(XVARIANT value1, XVARIANT value2)
{
    bool bResult=false;

    if((value1.mode==value2.mode)&&(value1.bIsBigEndian==value2.bIsBigEndian))
    {
        if      (value1.mode==MODE_BIT)     bResult=(value1.var.v_bool==value2.var.v_bool);
        else if (value1.mode==MODE_8)       bResult=(value1.var.v_uint8==value2.var.v_uint8);
        else if (value1.mode==MODE_16)      bResult=(value1.var.v_uint16==value2.var.v_uint16);
        else if (value1.mode==MODE_32)      bResult=(value1.var.v_uint32==value2.var.v_uint32);
        else if (value1.mode==MODE_64)      bResult=(value1.var.v_uint64==value2.var.v_uint64);
        else if (value1.mode==MODE_128)
        {
            bResult=(value1.var.v_uint128.high==value2.var.v_uint128.high)&&(value1.var.v_uint128.low==value2.var.v_uint128.low);
        }
    }

    return bResult;
}

XBinary::MODE XBinary::getModeOS()
{
    XBinary::MODE modeResult=XBinary::MODE_UNKNOWN;

    if(sizeof(void *)==8)
    {
        modeResult=XBinary::MODE_64;
    }
    else
    {
        modeResult=XBinary::MODE_32;
    }

    return modeResult;
}

void XBinary::setPdStructInit(PDSTRUCT *pPdStruct, qint32 nIndex, qint64 nTotal)
{
    if(nIndex!=-1)
    {
        pPdStruct->_pdRecord[nIndex].bIsValid=true;
        pPdStruct->_pdRecord[nIndex].nCurrent=0;
        pPdStruct->_pdRecord[nIndex].nTotal=nTotal;
        pPdStruct->_pdRecord[nIndex].sStatus="";
    }
}

void XBinary::setPdStructTotal(PDSTRUCT *pPdStruct, qint32 nIndex, qint64 nValue)
{
    if(nIndex!=-1)
    {
        pPdStruct->_pdRecord[nIndex].nTotal=nValue;
    }
}

void XBinary::setPdStructCurrent(PDSTRUCT *pPdStruct, qint32 nIndex, qint64 nValue)
{
    if(nIndex!=-1)
    {
        pPdStruct->_pdRecord[nIndex].nCurrent=nValue;
    }
}

void XBinary::setPdStructCurrentIncrement(PDSTRUCT *pPdStruct, qint32 nIndex)
{
    if(nIndex!=-1)
    {
        pPdStruct->_pdRecord[nIndex].nCurrent++;
    }
}

void XBinary::setPdStructStatus(PDSTRUCT *pPdStruct, qint32 nIndex, QString sStatus)
{
    if(nIndex!=-1)
    {
        pPdStruct->_pdRecord[nIndex].sStatus=sStatus;
    }
}

void XBinary::setPdStructFinished(PDSTRUCT *pPdStruct, qint32 nIndex)
{
    if(nIndex!=-1)
    {
        pPdStruct->_pdRecord[nIndex].bIsValid=false;
        pPdStruct->_pdRecord[nIndex].nCurrent=0;
        pPdStruct->_pdRecord[nIndex].nTotal=0;
        pPdStruct->_pdRecord[nIndex].sStatus="";

        pPdStruct->nFinished++;
    }
}

qint32 XBinary::getFreeIndex(PDSTRUCT *pPdStruct)
{
    qint32 nResult=-1;

    for(qint32 i=0;i<N_NUMBER_PDRECORDS;i++)
    {
        if(!pPdStruct->_pdRecord[i].bIsValid)
        {
            nResult=i;

            break;
        }
    }

    return nResult;
}

bool XBinary::isPdStructFinished(PDSTRUCT *pPdStruct)
{
    bool bResult=true;

    for(qint32 i=0;i<N_NUMBER_PDRECORDS;i++)
    {
        if(pPdStruct->_pdRecord[i].bIsValid)
        {
            bResult=false;

            break;
        }
    }

    if(pPdStruct->nFinished==0)
    {
        bResult=false;
    }

    return bResult;
}

bool XBinary::isPdStructSuccess(PDSTRUCT *pPdStruct)
{
    bool bResult=false;

    if(isPdStructFinished(pPdStruct)&&(!(pPdStruct->bIsStop)))
    {
        if(pPdStruct->nFinished>0)
        {
            bResult=true;
        }
    }

    return bResult;
}

qint32 XBinary::getPdStructProcent(PDSTRUCT *pPdStruct)
{
    qint32 nResult=0;

    for(qint32 i=0;i<N_NUMBER_PDRECORDS;i++)
    {
        if(pPdStruct->_pdRecord[i].nTotal)
        {
            nResult=(pPdStruct->_pdRecord[i].nCurrent*100)/pPdStruct->_pdRecord[i].nTotal;

            break;
        }
    }

    return nResult;
}

QList<XBinary::SIGNATURE_RECORD> XBinary::getSignatureRecords(QString sSignature)
{
    // TODO Error checks!
    QList<SIGNATURE_RECORD> listResult;

    qint32 nSignatureSize=sSignature.size();

    for(qint32 i=0;i<nSignatureSize;)
    {
        if(sSignature.at(i)==QChar('.'))
        {
            i+=_getSignatureRelOffsetFix(&listResult,sSignature,i);
        }
        else if(sSignature.at(i)==QChar('+'))
        {
            i+=_getSignatureDelta(&listResult,sSignature,i);
        }
        else if(sSignature.at(i)==QChar('$'))
        {
            i+=_getSignatureRelOffset(&listResult,sSignature,i);
        }
        else if(sSignature.at(i)==QChar('#')) // TODO Check []
        {
            i+=_getSignatureAddress(&listResult,sSignature,i);
        }
        else
        {
            qint32 nBytes=_getSignatureBytes(&listResult,sSignature,i);

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

    return listResult;
}

bool XBinary::_compareSignature(_MEMORY_MAP *pMemoryMap, QList<XBinary::SIGNATURE_RECORD> *pListSignatureRecords, qint64 nOffset)
{
    // TODO optimize

    qint32 nNumberOfSignatures=pListSignatureRecords->count();

    for(qint32 i=0;i<nNumberOfSignatures;i++)
    {
        XADDR _nAddress=0;

        switch(pListSignatureRecords->at(i).st)
        {
            case XBinary::ST_COMPAREBYTES:
                {
                    QByteArray baData=read_array(nOffset,pListSignatureRecords->at(i).baData.size());

                    if(baData.size()!=pListSignatureRecords->at(i).baData.size())
                    {
                        return false;
                    }

                    if(!compareMemory(baData.data(),(char *)(pListSignatureRecords->at(i).baData.data()),baData.size()))
                    {
                        return false;
                    }

                    nOffset+=baData.size();
                }
                break;

            case XBinary::ST_FINDBYTES:
                {
                    qint64 nResult=find_byteArray(nOffset,pListSignatureRecords->at(i).nFindDelta+pListSignatureRecords->at(i).baData.size(),pListSignatureRecords->at(i).baData);

                    if(nResult==-1)
                    {
                        return false;
                    }

                    nOffset=nResult+pListSignatureRecords->at(i).baData.size();
                }
                break;

            case XBinary::ST_RELOFFSETFIX:
                // TODO Check
                nOffset+=pListSignatureRecords->at(i).nBaseAddress;
                break;

            case XBinary::ST_RELOFFSET:
                _nAddress=offsetToAddress(pMemoryMap,nOffset);

                switch(pListSignatureRecords->at(i).nSizeOfAddr)
                {
                    case 1:
                        _nAddress+=1+read_int8(nOffset);
                        break;

                    case 2:
                        if(pMemoryMap->fileType==FT_COM)
                        {
                            _nAddress+=2+read_uint16(nOffset);
                        }
                        else
                        {
                            _nAddress+=2+read_int16(nOffset); // TODO mb BE<->LE
                        }

                        break;

                    case 4:
                        _nAddress+=4+read_int32(nOffset); // TODO mb BE<->LE
                        break;

                    case 8:
                        _nAddress+=8+read_int64(nOffset); // TODO mb BE<->LE
                        break;
                }

                nOffset=addressToOffset(pMemoryMap,_nAddress);

                break;

            case XBinary::ST_ADDRESS:
                switch(pListSignatureRecords->at(i).nSizeOfAddr)
                {
                    case 1:
                        _nAddress=read_uint8(nOffset);
                        break;

                    case 2:
                        _nAddress=read_uint16(nOffset); // TODO mb BE<->LE
                        break;

                    case 4:
                        _nAddress=read_uint32(nOffset); // TODO mb BE<->LE
                        break;

                    case 8:
                        _nAddress=read_uint64(nOffset); // TODO mb BE<->LE
                        break;
                }

                // TODO Base address!!!

                nOffset=addressToOffset(pMemoryMap,_nAddress); // TODO!
                break;
        }

        if((!isOffsetValid(pMemoryMap,nOffset))&&(nOffset!=getSize()))
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

int XBinary::_getSignatureRelOffsetFix(QList<XBinary::SIGNATURE_RECORD> *pListSignatureRecords, QString sSignature, qint32 nStartIndex)
{
    int nResult=0;
    qint32 nSignatureSize=sSignature.size();

    for(qint32 i=nStartIndex;i<nSignatureSize;i++)
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
        SIGNATURE_RECORD record={};

        record.st=XBinary::ST_RELOFFSETFIX;
        record.nSizeOfAddr=0;
        record.nBaseAddress=nResult/2;

        pListSignatureRecords->append(record);
    }

    return nResult;
}

qint32 XBinary::_getSignatureDelta(QList<XBinary::SIGNATURE_RECORD> *pListSignatureRecords, QString sSignature, int nStartIndex)
{
    // TODO Check!!!
    qint32 nResult=0;
    qint32 nSignatureSize=sSignature.size();

    for(qint32 i=nStartIndex;i<nSignatureSize;i++)
    {
        if(sSignature.at(i)==QChar('+'))
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
        QList<XBinary::SIGNATURE_RECORD> _listSignatureRecords;
        qint32 nTemp=_getSignatureBytes(&_listSignatureRecords,sSignature,nStartIndex+nResult);

        if(_listSignatureRecords.count())
        {
            SIGNATURE_RECORD record={};

            record.st=XBinary::ST_FINDBYTES;
            record.nSizeOfAddr=0;
            record.nBaseAddress=0;
            record.baData=_listSignatureRecords.at(0).baData;
            record.nFindDelta=32*nResult;

            pListSignatureRecords->append(record);

            nResult+=nTemp;
        }
    }

    return nResult;
}

int XBinary::_getSignatureRelOffset(QList<XBinary::SIGNATURE_RECORD> *pListSignatureRecords, QString sSignature, int nStartIndex)
{
    int nResult=0;

    qint32 nSignatureSize=sSignature.size();

    for(qint32 i=nStartIndex;i<nSignatureSize;i++)
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
        SIGNATURE_RECORD record={};

        record.st=XBinary::ST_RELOFFSET;
        record.nSizeOfAddr=nResult/2;
        record.nBaseAddress=0;

        pListSignatureRecords->append(record);
    }

    return nResult;
}

int XBinary::_getSignatureAddress(QList<XBinary::SIGNATURE_RECORD> *pListSignatureRecords, QString sSignature, int nStartIndex)
{
    int nResult=0;

    qint32 nSignatureSize=sSignature.size();
    QString sBaseAddress;
    bool bIsBaseAddress=false;
    int nSizeOfAddress=0;

    for(qint32 i=nStartIndex;i<nSignatureSize;i++)
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
        SIGNATURE_RECORD record={};

        record.st=XBinary::ST_ADDRESS;
        record.nSizeOfAddr=nSizeOfAddress/2;
        record.nBaseAddress=sBaseAddress.toInt(0,16);

        pListSignatureRecords->append(record);
    }

    return nResult;
}

qint32 XBinary::_getSignatureBytes(QList<XBinary::SIGNATURE_RECORD> *pListSignatureRecords, QString sSignature, qint32 nStartIndex)
{
    int nResult=0;

    qint32 nSignatureSize=sSignature.size();
    QString sBytes;

    for(qint32 i=nStartIndex;i<nSignatureSize;i++)
    {
        if( ((sSignature.at(i)>=QChar('a'))&&(sSignature.at(i)<=QChar('f')))||
            ((sSignature.at(i)>=QChar('0'))&&(sSignature.at(i)<=QChar('9'))))
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
        SIGNATURE_RECORD record={};

        record.st=XBinary::ST_COMPAREBYTES;
        record.nSizeOfAddr=0;
        record.nBaseAddress=0;
        record.baData=QByteArray::fromHex(sBytes.toUtf8()); // TODO Check

        pListSignatureRecords->append(record);
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

    for(qint64 i=0;i<nSize;i++)
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

bool XBinary::isAddressPhysical(XADDR nAddress)
{
    _MEMORY_MAP memoryMap=getMemoryMap();

    return isAddressPhysical(&memoryMap,nAddress);
}
