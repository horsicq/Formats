// copyright (c) 2020-2021 hors<horsicq@gmail.com>
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
#include "xformats.h"

XFormats::XFormats(QObject *pParent) : QObject(pParent)
{

}

XBinary::_MEMORY_MAP XFormats::getMemoryMap(XBinary::FT fileType,QIODevice *pDevice,bool bIsImage,qint64 nModuleAddress)
{
    XBinary::_MEMORY_MAP result={};

    if(XBinary::checkFileType(XBinary::FT_BINARY,fileType))
    {
        XBinary binary(pDevice,bIsImage,nModuleAddress);
        result=binary.getMemoryMap();
    }
    else if(XBinary::checkFileType(XBinary::FT_COM,fileType))
    {
        XCOM com(pDevice,bIsImage,nModuleAddress);
        result=com.getMemoryMap();
    }
    else if(XBinary::checkFileType(XBinary::FT_MSDOS,fileType))
    {
        XMSDOS msdos(pDevice,bIsImage,nModuleAddress);
        result=msdos.getMemoryMap();
    }
    else if(XBinary::checkFileType(XBinary::FT_NE,fileType))
    {
        XNE ne(pDevice,bIsImage,nModuleAddress);
        result=ne.getMemoryMap();
    }
    else if(XBinary::checkFileType(XBinary::FT_LE,fileType))
    {
        XLE le(pDevice,bIsImage,nModuleAddress);
        result=le.getMemoryMap();
    }
    else if(XBinary::checkFileType(XBinary::FT_PE,fileType))
    {
        XPE pe(pDevice,bIsImage,nModuleAddress);
        result=pe.getMemoryMap();
    }
    else if(XBinary::checkFileType(XBinary::FT_ELF,fileType))
    {
        XELF elf(pDevice,bIsImage,nModuleAddress);
        result=elf.getMemoryMap();
    }
    else if(XBinary::checkFileType(XBinary::FT_MACHO,fileType))
    {
        XMACH mach(pDevice,bIsImage,nModuleAddress);
        result=mach.getMemoryMap();
    }
#ifdef USE_DEX // TODO Check !!!
    else if(XBinary::checkFileType(XBinary::FT_DEX,fileType))
    {
        XDEX dex(pDevice);
        result=dex.getMemoryMap();
    }
#endif
#ifdef USE_ARCHIVE
    else if(XBinary::checkFileType(XBinary::FT_ZIP,fileType))
    {
        XZip zip(pDevice);
        result=zip.getMemoryMap();
    }
#endif
    else
    {
        XBinary binary(pDevice,bIsImage,nModuleAddress);
        result=binary.getMemoryMap();
    }

    return result;
}

qint64 XFormats::getEntryPointAddress(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage, qint64 nModuleAddress)
{
    // TODO pMemoryMap
    qint64 nResult=0; // FT_DEX, FT_ZIP

    if(XBinary::checkFileType(XBinary::FT_BINARY,fileType))
    {
        XBinary binary(pDevice,bIsImage,nModuleAddress);
        nResult=binary.getEntryPointAddress();
    }
    else if(XBinary::checkFileType(XBinary::FT_COM,fileType))
    {
        XCOM com(pDevice,bIsImage,nModuleAddress);
        nResult=com.getEntryPointAddress();
    }
    else if(XBinary::checkFileType(XBinary::FT_MSDOS,fileType))
    {
        XMSDOS msdos(pDevice,bIsImage,nModuleAddress);
        nResult=msdos.getEntryPointAddress();
    }
    else if(XBinary::checkFileType(XBinary::FT_NE,fileType))
    {
        XNE ne(pDevice,bIsImage,nModuleAddress);
        nResult=ne.getEntryPointAddress();
    }
    else if(XBinary::checkFileType(XBinary::FT_LE,fileType))
    {
        XLE le(pDevice,bIsImage,nModuleAddress);
        nResult=le.getEntryPointAddress();
    }
    else if(XBinary::checkFileType(XBinary::FT_PE,fileType))
    {
        XPE pe(pDevice,bIsImage,nModuleAddress);
        nResult=pe.getEntryPointAddress();
    }
    else if(XBinary::checkFileType(XBinary::FT_ELF,fileType))
    {
        XELF elf(pDevice,bIsImage,nModuleAddress);
        nResult=elf.getEntryPointAddress();
    }
    else if(XBinary::checkFileType(XBinary::FT_MACHO,fileType))
    {
        XMACH mach(pDevice,bIsImage,nModuleAddress);
        nResult=mach.getEntryPointAddress();
    }
    else
    {
        nResult=0;
    }

    return nResult;
}

qint64 XFormats::getEntryPointOffset(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage, qint64 nModuleAddress)
{
    qint64 nResult=0;

    if(XBinary::checkFileType(XBinary::FT_BINARY,fileType))
    {
        XBinary binary(pDevice,bIsImage,nModuleAddress);
        nResult=binary._getEntryPointOffset();
    }
    else if(XBinary::checkFileType(XBinary::FT_COM,fileType))
    {
        XCOM com(pDevice,bIsImage,nModuleAddress);
        nResult=com._getEntryPointOffset();
    }
    else if(XBinary::checkFileType(XBinary::FT_MSDOS,fileType))
    {
        XMSDOS msdos(pDevice,bIsImage,nModuleAddress);
        nResult=msdos._getEntryPointOffset();
    }
    else if(XBinary::checkFileType(XBinary::FT_NE,fileType))
    {
        XNE ne(pDevice,bIsImage,nModuleAddress);
        nResult=ne._getEntryPointOffset();
    }
    else if(XBinary::checkFileType(XBinary::FT_LE,fileType))
    {
        XLE le(pDevice,bIsImage,nModuleAddress);
        nResult=le._getEntryPointOffset();
    }
    else if(XBinary::checkFileType(XBinary::FT_PE,fileType))
    {
        XPE pe(pDevice,bIsImage,nModuleAddress);
        nResult=pe._getEntryPointOffset();
    }
    else if(XBinary::checkFileType(XBinary::FT_ELF,fileType))
    {
        XELF elf(pDevice,bIsImage,nModuleAddress);
        nResult=elf._getEntryPointOffset();
    }
    else if(XBinary::checkFileType(XBinary::FT_MACHO,fileType))
    {
        XMACH mach(pDevice,bIsImage,nModuleAddress);
        nResult=mach._getEntryPointOffset();
    }
    else
    {
        nResult=0;
    }

    return nResult;
}

bool XFormats::isBigEndian(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage, qint64 nModuleAddress)
{
    bool bResult=false;

    if(XBinary::checkFileType(XBinary::FT_BINARY,fileType))
    {
        XBinary binary(pDevice,bIsImage,nModuleAddress);
        bResult=binary.isBigEndian();
    }
    else if(XBinary::checkFileType(XBinary::FT_COM,fileType))
    {
        XCOM com(pDevice,bIsImage,nModuleAddress);
        bResult=com.isBigEndian();
    }
    else if(XBinary::checkFileType(XBinary::FT_MSDOS,fileType))
    {
        XMSDOS msdos(pDevice,bIsImage,nModuleAddress);
        bResult=msdos.isBigEndian();
    }
    else if(XBinary::checkFileType(XBinary::FT_NE,fileType))
    {
        XNE ne(pDevice,bIsImage,nModuleAddress);
        bResult=ne.isBigEndian();
    }
    else if(XBinary::checkFileType(XBinary::FT_LE,fileType))
    {
        XLE le(pDevice,bIsImage,nModuleAddress);
        bResult=le.isBigEndian();
    }
    else if(XBinary::checkFileType(XBinary::FT_PE,fileType))
    {
        XPE pe(pDevice,bIsImage,nModuleAddress);
        bResult=pe.isBigEndian();
    }
    else if(XBinary::checkFileType(XBinary::FT_ELF,fileType))
    {
        XELF elf(pDevice,bIsImage,nModuleAddress);
        bResult=elf.isBigEndian();
    }
    else if(XBinary::checkFileType(XBinary::FT_MACHO,fileType))
    {
        XMACH mach(pDevice,bIsImage,nModuleAddress);
        bResult=mach.isBigEndian();
    }
#ifdef USE_DEX
    else if(XBinary::checkFileType(XBinary::FT_DEX,fileType))
    {
        XDEX dex(pDevice);
        bResult=dex.isBigEndian();
    }
#endif
#ifdef USE_ARCHIVE
    else if(XBinary::checkFileType(XBinary::FT_ZIP,fileType))
    {
        XZip zip(pDevice);
        bResult=zip.isBigEndian();
    }
#endif
    else
    {
        bResult=false;
    }

    return bResult;
}

bool XFormats::isSigned(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage, qint64 nModuleAddress)
{
    bool bResult=false;

    if(XBinary::checkFileType(XBinary::FT_BINARY,fileType))
    {
        XBinary binary(pDevice,bIsImage,nModuleAddress);
        bResult=binary.isSigned();
    }
    else if(XBinary::checkFileType(XBinary::FT_COM,fileType))
    {
        XCOM com(pDevice,bIsImage,nModuleAddress);
        bResult=com.isSigned();
    }
    else if(XBinary::checkFileType(XBinary::FT_MSDOS,fileType))
    {
        XMSDOS msdos(pDevice,bIsImage,nModuleAddress);
        bResult=msdos.isSigned();
    }
    else if(XBinary::checkFileType(XBinary::FT_NE,fileType))
    {
        XNE ne(pDevice,bIsImage,nModuleAddress);
        bResult=ne.isSigned();
    }
    else if(XBinary::checkFileType(XBinary::FT_LE,fileType))
    {
        XLE le(pDevice,bIsImage,nModuleAddress);
        bResult=le.isSigned();
    }
    else if(XBinary::checkFileType(XBinary::FT_PE,fileType))
    {
        XPE pe(pDevice,bIsImage,nModuleAddress);
        bResult=pe.isSigned();
    }
    else if(XBinary::checkFileType(XBinary::FT_ELF,fileType))
    {
        XELF elf(pDevice,bIsImage,nModuleAddress);
        bResult=elf.isSigned();
    }
    else if(XBinary::checkFileType(XBinary::FT_MACHO,fileType))
    {
        XMACH mach(pDevice,bIsImage,nModuleAddress);
        bResult=mach.isSigned();
    }
#ifdef USE_DEX
    else if(XBinary::checkFileType(XBinary::FT_DEX,fileType))
    {
        XDEX dex(pDevice);
        bResult=dex.isSigned();
    }
#endif
#ifdef USE_ARCHIVE
    else if(XBinary::checkFileType(XBinary::FT_ZIP,fileType))
    {
        XZip zip(pDevice);
        bResult=zip.isSigned();
    }
#endif
    else
    {
        bResult=false;
    }

    return bResult;
}

XBinary::OFFSETSIZE XFormats::getSignOS(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage, qint64 nModuleAddress)
{
    XBinary::OFFSETSIZE result={};

    if(XBinary::checkFileType(XBinary::FT_BINARY,fileType))
    {
        XBinary binary(pDevice,bIsImage,nModuleAddress);
        result=binary.getSignOS();
    }
    else if(XBinary::checkFileType(XBinary::FT_COM,fileType))
    {
        XCOM com(pDevice,bIsImage,nModuleAddress);
        result=com.getSignOS();
    }
    else if(XBinary::checkFileType(XBinary::FT_MSDOS,fileType))
    {
        XMSDOS msdos(pDevice,bIsImage,nModuleAddress);
        result=msdos.getSignOS();
    }
    else if(XBinary::checkFileType(XBinary::FT_NE,fileType))
    {
        XNE ne(pDevice,bIsImage,nModuleAddress);
        result=ne.getSignOS();
    }
    else if(XBinary::checkFileType(XBinary::FT_LE,fileType))
    {
        XLE le(pDevice,bIsImage,nModuleAddress);
        result=le.getSignOS();
    }
    else if(XBinary::checkFileType(XBinary::FT_PE,fileType))
    {
        XPE pe(pDevice,bIsImage,nModuleAddress);
        result=pe.getSignOS();
    }
    else if(XBinary::checkFileType(XBinary::FT_ELF,fileType))
    {
        XELF elf(pDevice,bIsImage,nModuleAddress);
        result=elf.getSignOS();
    }
    else if(XBinary::checkFileType(XBinary::FT_MACHO,fileType))
    {
        XMACH mach(pDevice,bIsImage,nModuleAddress);
        result=mach.getSignOS();
    }
#ifdef USE_DEX
    else if(XBinary::checkFileType(XBinary::FT_DEX,fileType))
    {
        XDEX dex(pDevice);
        result=dex.getSignOS();
    }
#endif
#ifdef USE_ARCHIVE
    else if(XBinary::checkFileType(XBinary::FT_ZIP,fileType))
    {
        XZip zip(pDevice);
        result=zip.getSignOS();
    }
#endif

    return result;
}

XBinary::OFFSETSIZE XFormats::getSignOS(QString sFileName)
{
    XBinary::OFFSETSIZE result={};

    QFile file;

    file.setFileName(sFileName);

    if(file.open(QIODevice::ReadOnly))
    {
        result=getSignOS(XBinary::getPrefFileType(&file,true),&file);

        file.close();
    }

    return result;
}

bool XFormats::isSigned(QString sFileName)
{
    bool bResult=false;

    QFile file;

    file.setFileName(sFileName);

    if(file.open(QIODevice::ReadOnly))
    {
        bResult=isSigned(XBinary::getPrefFileType(&file,true),&file);

        file.close();
    }

    return bResult;
}

QList<XBinary::SYMBOL_RECORD> XFormats::getSymbolRecords(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage, qint64 nModuleAddress, XBinary::SYMBOL_TYPE symBolType)
{
    QList<XBinary::SYMBOL_RECORD> listResult;

    if(XBinary::checkFileType(XBinary::FT_BINARY,fileType))
    {
        XBinary binary(pDevice,bIsImage,nModuleAddress);

        XBinary::_MEMORY_MAP memoryMap=binary.getMemoryMap();
        listResult=binary.getSymbolRecords(&memoryMap,symBolType);
    }
    else if(XBinary::checkFileType(XBinary::FT_COM,fileType))
    {
        XCOM com(pDevice,bIsImage,nModuleAddress);

        XBinary::_MEMORY_MAP memoryMap=com.getMemoryMap();
        listResult=com.getSymbolRecords(&memoryMap,symBolType);
    }
    else if(XBinary::checkFileType(XBinary::FT_MSDOS,fileType))
    {
        XMSDOS msdos(pDevice,bIsImage,nModuleAddress);

        XBinary::_MEMORY_MAP memoryMap=msdos.getMemoryMap();
        listResult=msdos.getSymbolRecords(&memoryMap,symBolType);
    }
    else if(XBinary::checkFileType(XBinary::FT_NE,fileType))
    {
        XNE ne(pDevice,bIsImage,nModuleAddress);

        XBinary::_MEMORY_MAP memoryMap=ne.getMemoryMap();
        listResult=ne.getSymbolRecords(&memoryMap,symBolType);
    }
    else if(XBinary::checkFileType(XBinary::FT_LE,fileType))
    {
        XLE le(pDevice,bIsImage,nModuleAddress);

        XBinary::_MEMORY_MAP memoryMap=le.getMemoryMap();
        listResult=le.getSymbolRecords(&memoryMap,symBolType);
    }
    else if(XBinary::checkFileType(XBinary::FT_PE,fileType))
    {
        XPE pe(pDevice,bIsImage,nModuleAddress);

        XBinary::_MEMORY_MAP memoryMap=pe.getMemoryMap();
        listResult=pe.getSymbolRecords(&memoryMap,symBolType);
    }
    else if(XBinary::checkFileType(XBinary::FT_ELF,fileType))
    {
        XELF elf(pDevice,bIsImage,nModuleAddress);

        XBinary::_MEMORY_MAP memoryMap=elf.getMemoryMap();
        listResult=elf.getSymbolRecords(&memoryMap,symBolType);
    }
    else if(XBinary::checkFileType(XBinary::FT_MACHO,fileType))
    {
        XMACH mach(pDevice,bIsImage,nModuleAddress);

        XBinary::_MEMORY_MAP memoryMap=mach.getMemoryMap();
        listResult=mach.getSymbolRecords(&memoryMap,symBolType);
    }
#ifdef USE_DEX
    else if(XBinary::checkFileType(XBinary::FT_DEX,fileType))
    {
        XDEX dex(pDevice);

        XBinary::_MEMORY_MAP memoryMap=dex.getMemoryMap();
        listResult=dex.getSymbolRecords(&memoryMap,symBolType);
    }
#endif
#ifdef USE_ARCHIVE
    else if(XBinary::checkFileType(XBinary::FT_ZIP,fileType))
    {
        XZip zip(pDevice);

        XBinary::_MEMORY_MAP memoryMap=zip.getMemoryMap();
        listResult=zip.getSymbolRecords(&memoryMap,symBolType);
    }
#endif

    return listResult;
}

#ifdef QT_GUI_LIB
XBinary::FT XFormats::setFileTypeComboBox(QComboBox *pComboBox, QList<XBinary::FT> *pListFileTypes, XBinary::FT fileType)
{
#if QT_VERSION >= 0x050300
    const QSignalBlocker block(pComboBox);
#else
    const bool bBlocked1=pComboBox->blockSignals(true);
#endif

    pComboBox->clear();

    int nNumberOfListTypes=pListFileTypes->count();

    for(int i=0;i<nNumberOfListTypes;i++)
    {
        XBinary::FT fileType=pListFileTypes->at(i);
        pComboBox->addItem(XBinary::fileTypeIdToString(fileType),fileType);
    }

    if(nNumberOfListTypes)
    {
        if(fileType==XBinary::FT_UNKNOWN)
        {
            if(pComboBox->itemData(nNumberOfListTypes-1).toUInt()!=XBinary::FT_COM)
            {
                pComboBox->setCurrentIndex(nNumberOfListTypes-1);
            }
        }
        else
        {
            int nNumberOfItems=pComboBox->count();

            for(int i=0;i<nNumberOfItems;i++)
            {
                if(pComboBox->itemData(i).toUInt()==fileType)
                {
                    pComboBox->setCurrentIndex(i);

                    break;
                }
            }
        }
    }

#if QT_VERSION < 0x050300
    pComboBox->blockSignals(bBlocked1);
#endif

    return (XBinary::FT)(pComboBox->currentData().toUInt());
}
#endif
#ifdef QT_GUI_LIB
bool XFormats::setEndianessComboBox(QComboBox *pComboBox, bool bIsBigEndian)
{ 
#if QT_VERSION >= 0x050300
    const QSignalBlocker blocker(pComboBox);
#else
    const bool bBlocked1=pComboBox->blockSignals(true);
#endif

    bool bResult=bIsBigEndian;

    pComboBox->clear();

    pComboBox->addItem("LE",false);
    pComboBox->addItem("BE",true);

    if(bIsBigEndian)
    {
        pComboBox->setCurrentIndex(0);
    }

#if QT_VERSION < 0x050300
    pComboBox->blockSignals(bBlocked1);
#endif

    return bResult;
}
#endif
