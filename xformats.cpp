/* Copyright (c) 2020-2025 hors<horsicq@gmail.com>
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
#include "xformats.h"

XFormats::XFormats(QObject *pParent) : QObject(pParent)
{
}

XBinary *XFormats::getClass(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    if (XBinary::checkFileType(XBinary::FT_BINARY, fileType)) return new XBinary(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_COM, fileType)) return new XCOM(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_MSDOS, fileType)) return new XMSDOS(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_NE, fileType)) return new XNE(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_LE, fileType) || XBinary::checkFileType(XBinary::FT_LX, fileType))
        return new XLE(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_PE, fileType)) return new XPE(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_ELF, fileType)) return new XELF(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_MACHO, fileType)) return new XMACH(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_AMIGAHUNK, fileType)) return new XAmigaHunk(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_PNG, fileType)) return new XPNG(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_JPEG, fileType)) return new XJpeg(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_ICO, fileType)) return new XIcon(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_BMP, fileType)) return new XBMP(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_GIF, fileType)) return new XGif(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_TIFF, fileType)) return new XTiff(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_MP4, fileType)) return new XMP4(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_MP3, fileType)) return new XMP3(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_RIFF, fileType)) return new XRiff(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_JAVACLASS, fileType)) return new XJavaClass(pDevice);
#ifdef USE_DEX
    else if (XBinary::checkFileType(XBinary::FT_DEX, fileType)) return new XDEX(pDevice);
#endif
#ifdef USE_PDF
    else if (XBinary::checkFileType(XBinary::FT_PDF, fileType)) return new XPDF(pDevice);
#endif
#ifdef USE_ARCHIVE
    else if (XBinary::checkFileType(XBinary::FT_ZIP, fileType)) return new XZip(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_JAR, fileType)) return new XJAR(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_APK, fileType)) return new XAPK(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_IPA, fileType)) return new XIPA(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_APKS, fileType)) return new XAPKS(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_7Z, fileType)) return new XSevenZip(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_CAB, fileType)) return new XCab(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_RAR, fileType)) return new XRar(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_MACHOFAT, fileType)) return new XMACHOFat(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_NPM, fileType)) return new XNPM(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_TARGZ, fileType)) return new XTGZ(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_GZIP, fileType)) return new XGzip(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_ZLIB, fileType)) return new XZlib(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_LHA, fileType)) return new XLHA(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_CFBF, fileType)) return new XCFBF(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_DOS4G, fileType) || XBinary::checkFileType(XBinary::FT_DOS16M, fileType))
        return new XDOS16(pDevice);
#endif
    else {
#ifdef QT_DEBUG
        qDebug() << "XFormats::getClass: Unknown file type" << XBinary::fileTypeIdToString(fileType);
#endif
        return new XBinary(pDevice, bIsImage, nModuleAddress);
    }
}

XBinary::_MEMORY_MAP XFormats::getMemoryMap(XBinary::FT fileType, XBinary::MAPMODE mapMode, QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress,
                                            XBinary::PDSTRUCT *pPdStruct)
{
    XBinary::_MEMORY_MAP result = {};

    XBinary *pBinary = XFormats::getClass(fileType, pDevice, bIsImage, nModuleAddress);
    result = pBinary->getMemoryMap(mapMode, pPdStruct);
    delete pBinary;

    return result;
}

XBinary::_MEMORY_MAP XFormats::getMemoryMap(const QString &sFileName, XBinary::MAPMODE mapMode, bool bIsImage, XADDR nModuleAddress, XBinary::PDSTRUCT *pPdStruct)
{
    XBinary::_MEMORY_MAP result = {};

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        result = getMemoryMap(XBinary::getPrefFileType(&file, true), mapMode, &file, bIsImage, nModuleAddress, pPdStruct);

        file.close();
    }

    return result;
}

QList<XBinary::MAPMODE> XFormats::getMapModesList(XBinary::FT fileType)
{
    QList<XBinary::MAPMODE> listResult = {};

    XBinary *pBinary = XFormats::getClass(fileType, nullptr);
    listResult = pBinary->getMapModesList();
    delete pBinary;

    return listResult;
}

XADDR XFormats::getEntryPointAddress(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    // TODO pMemoryMap !!!
    XADDR nResult = 0;

    XBinary *pBinary = XFormats::getClass(fileType, pDevice, bIsImage, nModuleAddress);
    nResult = pBinary->getEntryPointAddress();
    delete pBinary;

    return nResult;
}

qint64 XFormats::getEntryPointOffset(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    qint64 nResult = 0;

    XBinary *pBinary = XFormats::getClass(fileType, pDevice, bIsImage, nModuleAddress);
    nResult = pBinary->_getEntryPointOffset();
    delete pBinary;

    return nResult;
}

bool XFormats::isBigEndian(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    bool bResult = false;

    if (XBinary::checkFileType(XBinary::FT_BINARY, fileType)) {
        XBinary binary(pDevice, bIsImage, nModuleAddress);
        bResult = binary.isBigEndian();
    } else if (XBinary::checkFileType(XBinary::FT_COM, fileType)) {
        XCOM com(pDevice, bIsImage, nModuleAddress);
        bResult = com.isBigEndian();
    } else if (XBinary::checkFileType(XBinary::FT_MSDOS, fileType)) {
        XMSDOS msdos(pDevice, bIsImage, nModuleAddress);
        bResult = msdos.isBigEndian();
    } else if (XBinary::checkFileType(XBinary::FT_NE, fileType)) {
        XNE ne(pDevice, bIsImage, nModuleAddress);
        bResult = ne.isBigEndian();
    } else if (XBinary::checkFileType(XBinary::FT_LE, fileType)) {
        XLE le(pDevice, bIsImage, nModuleAddress);
        bResult = le.isBigEndian();
    } else if (XBinary::checkFileType(XBinary::FT_LX, fileType)) {
        XLE le(pDevice, bIsImage, nModuleAddress);
        bResult = le.isBigEndian();
    } else if (XBinary::checkFileType(XBinary::FT_PE, fileType)) {
        XPE pe(pDevice, bIsImage, nModuleAddress);
        bResult = pe.isBigEndian();
    } else if (XBinary::checkFileType(XBinary::FT_ELF, fileType)) {
        XELF elf(pDevice, bIsImage, nModuleAddress);
        bResult = elf.isBigEndian();
    } else if (XBinary::checkFileType(XBinary::FT_MACHO, fileType)) {
        XMACH mach(pDevice, bIsImage, nModuleAddress);
        bResult = mach.isBigEndian();
    } else if (XBinary::checkFileType(XBinary::FT_AMIGAHUNK, fileType)) {
        XAmigaHunk amigaHunk(pDevice, bIsImage, nModuleAddress);
        bResult = amigaHunk.isBigEndian();
    } else if (XBinary::checkFileType(XBinary::FT_JAVACLASS, fileType)) {
        XJavaClass xjc(pDevice);
        bResult = xjc.isBigEndian();
    }
#ifdef USE_DEX
    else if (XBinary::checkFileType(XBinary::FT_DEX, fileType)) {
        XDEX dex(pDevice);
        bResult = dex.isBigEndian();
    }
#endif
#ifdef USE_PDF
    else if (XBinary::checkFileType(XBinary::FT_PDF, fileType)) {
        XPDF pdf(pDevice);
        bResult = pdf.isBigEndian();
    }
#endif
#ifdef USE_ARCHIVE
    else if (XBinary::checkFileType(XBinary::FT_CFBF, fileType)) {
        XCFBF xjc(pDevice);
        bResult = xjc.isBigEndian();
    } else if (XBinary::checkFileType(XBinary::FT_ZIP, fileType)) {
        XZip zip(pDevice);
        bResult = zip.isBigEndian();
    }
#endif
    else {
        bResult = false;
    }

    return bResult;
}

QList<XBinary::HREGION> XFormats::getHighlights(XBinary::FT fileType, QIODevice *pDevice, XBinary::HLTYPE hlType, bool bIsImage, XADDR nModuleAddress,
                                                XBinary::PDSTRUCT *pPdStruct)
{
    QList<XBinary::HREGION> listResult;

    XBinary *pBinary = XFormats::getClass(fileType, pDevice, bIsImage, nModuleAddress);
    listResult = pBinary->getHighlights(hlType, pPdStruct);
    delete pBinary;

    return listResult;
}

bool XFormats::isSigned(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    bool bResult = false;

    if (XBinary::checkFileType(XBinary::FT_BINARY, fileType)) {
        XBinary binary(pDevice, bIsImage, nModuleAddress);
        bResult = binary.isSigned();
    } else if (XBinary::checkFileType(XBinary::FT_COM, fileType)) {
        XCOM com(pDevice, bIsImage, nModuleAddress);
        bResult = com.isSigned();
    } else if (XBinary::checkFileType(XBinary::FT_MSDOS, fileType)) {
        XMSDOS msdos(pDevice, bIsImage, nModuleAddress);
        bResult = msdos.isSigned();
    } else if (XBinary::checkFileType(XBinary::FT_NE, fileType)) {
        XNE ne(pDevice, bIsImage, nModuleAddress);
        bResult = ne.isSigned();
    } else if (XBinary::checkFileType(XBinary::FT_LE, fileType)) {
        XLE le(pDevice, bIsImage, nModuleAddress);
        bResult = le.isSigned();
    } else if (XBinary::checkFileType(XBinary::FT_LX, fileType)) {
        XLE le(pDevice, bIsImage, nModuleAddress);
        bResult = le.isSigned();
    } else if (XBinary::checkFileType(XBinary::FT_PE, fileType)) {
        XPE pe(pDevice, bIsImage, nModuleAddress);
        bResult = pe.isSigned();
    } else if (XBinary::checkFileType(XBinary::FT_ELF, fileType)) {
        XELF elf(pDevice, bIsImage, nModuleAddress);
        bResult = elf.isSigned();
    } else if (XBinary::checkFileType(XBinary::FT_MACHO, fileType)) {
        XMACH mach(pDevice, bIsImage, nModuleAddress);
        bResult = mach.isSigned();
    } else if (XBinary::checkFileType(XBinary::FT_JAVACLASS, fileType)) {
        XJavaClass xjc(pDevice);
        bResult = xjc.isSigned();
    }
#ifdef USE_DEX
    else if (XBinary::checkFileType(XBinary::FT_DEX, fileType)) {
        XDEX dex(pDevice);
        bResult = dex.isSigned();
    }
#endif
#ifdef USE_PDF
    else if (XBinary::checkFileType(XBinary::FT_PDF, fileType)) {
        XPDF pdf(pDevice);
        bResult = pdf.isSigned();
    }
#endif
#ifdef USE_ARCHIVE
    else if (XBinary::checkFileType(XBinary::FT_ZIP, fileType)) {
        XZip zip(pDevice);
        bResult = zip.isSigned();
    }
#endif
    else {
        bResult = false;
    }

    return bResult;
}

XBinary::OFFSETSIZE XFormats::getSignOffsetSize(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    XBinary::OFFSETSIZE osResult = {};

    if (XBinary::checkFileType(XBinary::FT_BINARY, fileType)) {
        XBinary binary(pDevice, bIsImage, nModuleAddress);
        osResult = binary.getSignOffsetSize();
    } else if (XBinary::checkFileType(XBinary::FT_COM, fileType)) {
        XCOM com(pDevice, bIsImage, nModuleAddress);
        osResult = com.getSignOffsetSize();
    } else if (XBinary::checkFileType(XBinary::FT_MSDOS, fileType)) {
        XMSDOS msdos(pDevice, bIsImage, nModuleAddress);
        osResult = msdos.getSignOffsetSize();
    } else if (XBinary::checkFileType(XBinary::FT_NE, fileType)) {
        XNE ne(pDevice, bIsImage, nModuleAddress);
        osResult = ne.getSignOffsetSize();
    } else if (XBinary::checkFileType(XBinary::FT_LE, fileType)) {
        XLE le(pDevice, bIsImage, nModuleAddress);
        osResult = le.getSignOffsetSize();
    } else if (XBinary::checkFileType(XBinary::FT_LX, fileType)) {
        XLE le(pDevice, bIsImage, nModuleAddress);
        osResult = le.getSignOffsetSize();
    } else if (XBinary::checkFileType(XBinary::FT_PE, fileType)) {
        XPE pe(pDevice, bIsImage, nModuleAddress);
        osResult = pe.getSignOffsetSize();
    } else if (XBinary::checkFileType(XBinary::FT_ELF, fileType)) {
        XELF elf(pDevice, bIsImage, nModuleAddress);
        osResult = elf.getSignOffsetSize();
    } else if (XBinary::checkFileType(XBinary::FT_MACHO, fileType)) {
        XMACH mach(pDevice, bIsImage, nModuleAddress);
        osResult = mach.getSignOffsetSize();
    } else if (XBinary::checkFileType(XBinary::FT_JAVACLASS, fileType)) {
        XJavaClass xjc(pDevice);
        osResult = xjc.getSignOffsetSize();
    }
#ifdef USE_DEX
    else if (XBinary::checkFileType(XBinary::FT_DEX, fileType)) {
        XDEX dex(pDevice);
        osResult = dex.getSignOffsetSize();
    }
#endif
#ifdef USE_PDF
    else if (XBinary::checkFileType(XBinary::FT_PDF, fileType)) {
        XPDF pdf(pDevice);
        osResult = pdf.getSignOffsetSize();
    }
#endif
#ifdef USE_ARCHIVE
    else if (XBinary::checkFileType(XBinary::FT_ZIP, fileType)) {
        XZip zip(pDevice);
        osResult = zip.getSignOffsetSize();
    }
#endif

    return osResult;
}

XBinary::OFFSETSIZE XFormats::getSignOffsetSize(const QString &sFileName)
{
    XBinary::OFFSETSIZE result = {};

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        result = getSignOffsetSize(XBinary::getPrefFileType(&file, true), &file);

        file.close();
    }

    return result;
}

bool XFormats::isSigned(const QString &sFileName)
{
    bool bResult = false;

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        bResult = isSigned(XBinary::getPrefFileType(&file, true), &file);

        file.close();
    }

    return bResult;
}

QList<XBinary::SYMBOL_RECORD> XFormats::getSymbolRecords(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress, XBinary::SYMBOL_TYPE symBolType)
{
    QList<XBinary::SYMBOL_RECORD> listResult;

    if (XBinary::checkFileType(XBinary::FT_BINARY, fileType)) {
        XBinary binary(pDevice, bIsImage, nModuleAddress);

        XBinary::_MEMORY_MAP memoryMap = binary.getMemoryMap();
        listResult = binary.getSymbolRecords(&memoryMap, symBolType);
    } else if (XBinary::checkFileType(XBinary::FT_COM, fileType)) {
        XCOM com(pDevice, bIsImage, nModuleAddress);

        XBinary::_MEMORY_MAP memoryMap = com.getMemoryMap();
        listResult = com.getSymbolRecords(&memoryMap, symBolType);
    } else if (XBinary::checkFileType(XBinary::FT_MSDOS, fileType)) {
        XMSDOS msdos(pDevice, bIsImage, nModuleAddress);

        XBinary::_MEMORY_MAP memoryMap = msdos.getMemoryMap();
        listResult = msdos.getSymbolRecords(&memoryMap, symBolType);
    } else if (XBinary::checkFileType(XBinary::FT_NE, fileType)) {
        XNE ne(pDevice, bIsImage, nModuleAddress);

        XBinary::_MEMORY_MAP memoryMap = ne.getMemoryMap();
        listResult = ne.getSymbolRecords(&memoryMap, symBolType);
    } else if (XBinary::checkFileType(XBinary::FT_LE, fileType)) {
        XLE le(pDevice, bIsImage, nModuleAddress);

        XBinary::_MEMORY_MAP memoryMap = le.getMemoryMap();
        listResult = le.getSymbolRecords(&memoryMap, symBolType);
    } else if (XBinary::checkFileType(XBinary::FT_LX, fileType)) {
        XLE le(pDevice, bIsImage, nModuleAddress);

        XBinary::_MEMORY_MAP memoryMap = le.getMemoryMap();
        listResult = le.getSymbolRecords(&memoryMap, symBolType);
    } else if (XBinary::checkFileType(XBinary::FT_PE, fileType)) {
        XPE pe(pDevice, bIsImage, nModuleAddress);

        XBinary::_MEMORY_MAP memoryMap = pe.getMemoryMap();
        listResult = pe.getSymbolRecords(&memoryMap, symBolType);
    } else if (XBinary::checkFileType(XBinary::FT_ELF, fileType)) {
        XELF elf(pDevice, bIsImage, nModuleAddress);

        XBinary::_MEMORY_MAP memoryMap = elf.getMemoryMap();
        listResult = elf.getSymbolRecords(&memoryMap, symBolType);
    } else if (XBinary::checkFileType(XBinary::FT_MACHO, fileType)) {
        XMACH mach(pDevice, bIsImage, nModuleAddress);

        XBinary::_MEMORY_MAP memoryMap = mach.getMemoryMap();
        listResult = mach.getSymbolRecords(&memoryMap, symBolType);
    } else if (XBinary::checkFileType(XBinary::FT_AMIGAHUNK, fileType)) {
        XAmigaHunk amigaHunk(pDevice, bIsImage, nModuleAddress);

        XBinary::_MEMORY_MAP memoryMap = amigaHunk.getMemoryMap();
        listResult = amigaHunk.getSymbolRecords(&memoryMap, symBolType);
    } else if (XBinary::checkFileType(XBinary::FT_JAVACLASS, fileType)) {
        XJavaClass xjc(pDevice);

        XBinary::_MEMORY_MAP memoryMap = xjc.getMemoryMap();
        listResult = xjc.getSymbolRecords(&memoryMap, symBolType);
    }
#ifdef USE_DEX
    else if (XBinary::checkFileType(XBinary::FT_DEX, fileType)) {
        XDEX dex(pDevice);

        XBinary::_MEMORY_MAP memoryMap = dex.getMemoryMap();
        listResult = dex.getSymbolRecords(&memoryMap, symBolType);
    }
#endif
#ifdef USE_PDF
    else if (XBinary::checkFileType(XBinary::FT_PDF, fileType)) {
        XPDF pdf(pDevice);

        XBinary::_MEMORY_MAP memoryMap = pdf.getMemoryMap();
        listResult = pdf.getSymbolRecords(&memoryMap, symBolType);
    }
#endif
#ifdef USE_ARCHIVE
    else if (XBinary::checkFileType(XBinary::FT_ZIP, fileType)) {
        XZip zip(pDevice);

        XBinary::_MEMORY_MAP memoryMap = zip.getMemoryMap();
        listResult = zip.getSymbolRecords(&memoryMap, symBolType);
    }
#endif

    return listResult;
}

QSet<XBinary::FT> XFormats::getFileTypes(QIODevice *pDevice, bool bExtra, XBinary::PDSTRUCT *pPdStruct)
{
    XBinary::PDSTRUCT pdStructEmpty = {};

    if (!pPdStruct) {
        pdStructEmpty = XBinary::createPdStruct();
        pPdStruct = &pdStructEmpty;
    }

    return _getFileTypes(pDevice, bExtra, pPdStruct);
}

bool XFormats::saveAllPEIconsToDirectory(QIODevice *pDevice, const QString &sDirectoryName)
{
    bool bResult = false;

    XPE pe(pDevice);

    if (pe.isValid()) {
        QList<XPE::RESOURCE_RECORD> listResources = pe.getResources(10000);
        QList<XPE::RESOURCE_RECORD> listIcons = pe.getResourceRecords(XPE_DEF::S_RT_GROUP_ICON, -1, &listResources);

        qint32 nNumberOfRecords = listIcons.count();

        for (qint32 i = 0; i < nNumberOfRecords; i++) {
            XPE::RESOURCE_RECORD resourceRecord = listIcons.at(i);

            QString sFileName = sDirectoryName + QDir::separator() + QString("%1.ico").arg(XBinary::convertFileNameSymbols(XPE::resourceRecordToString(resourceRecord)));

            XFormats::savePE_ICOToFile(pDevice, &listResources, resourceRecord, sFileName);
        }
    }

    return bResult;
}

bool XFormats::saveAllPECursorsToDirectory(QIODevice *pDevice, const QString &sDirectoryName)
{
    bool bResult = true;

    XPE pe(pDevice);

    if (pe.isValid()) {
        QList<XPE::RESOURCE_RECORD> listResources = pe.getResources(10000);
        QList<XPE::RESOURCE_RECORD> listIcons = pe.getResourceRecords(XPE_DEF::S_RT_GROUP_CURSOR, -1, &listResources);

        qint32 nNumberOfRecords = listIcons.count();

        for (qint32 i = 0; i < nNumberOfRecords; i++) {
            XPE::RESOURCE_RECORD resourceRecord = listIcons.at(i);

            QString sFileName = sDirectoryName + QDir::separator() + QString("%1.cur").arg(XBinary::convertFileNameSymbols(XPE::resourceRecordToString(resourceRecord)));

            if (!XFormats::savePE_ICOToFile(pDevice, &listResources, resourceRecord, sFileName)) {
                bResult = false;
            }
        }
    }

    return bResult;
}

bool XFormats::savePE_ICOToFile(QIODevice *pDevice, QList<XPE::RESOURCE_RECORD> *pListResourceRecords, XPE::RESOURCE_RECORD resourceRecord, const QString &sFileName)
{
    bool bResult = false;

    qint32 nChunkType = XPE_DEF::S_RT_ICON;
    qint32 idType = 1;

    if (resourceRecord.irin[0].nID == XPE_DEF::S_RT_GROUP_CURSOR) {
        nChunkType = XPE_DEF::S_RT_CURSOR;
        idType = 2;
    }

    XPE xpe(pDevice);

    if (xpe.isValid()) {
        if (resourceRecord.nSize) {
            SubDevice sd(pDevice, resourceRecord.nOffset, resourceRecord.nSize);

            if (sd.open(QIODevice::ReadOnly)) {
                XIcon icon(&sd);

                if (icon.isValid()) {
                    QList<XPE::RESOURCE_RECORD> listChunkRecords;
                    qint64 nTotalDataSize = 0;

                    QList<XIcon::GRPICONDIRENTRY> listDirectories = icon.getIconGPRDirectories();

                    qint32 nNumberOfRecords = listDirectories.count();

                    for (qint32 i = 0; i < nNumberOfRecords; i++) {
                        XPE::RESOURCE_RECORD _resourceRecord = xpe.getResourceRecord(nChunkType, listDirectories.at(i).nID, pListResourceRecords);

                        listChunkRecords.append(_resourceRecord);

                        nTotalDataSize += _resourceRecord.nSize;
                    }

                    QFile file;
                    file.setFileName(sFileName);

                    if (file.open(QIODevice::ReadWrite)) {
                        file.resize(sizeof(XIcon::ICONDIR) + sizeof(XIcon::ICONDIRENTRY) * nNumberOfRecords + nTotalDataSize);

                        XBinary binaryNew(&file);

                        binaryNew.write_uint16(offsetof(XIcon::ICONDIR, idReserved), 0);
                        binaryNew.write_uint16(offsetof(XIcon::ICONDIR, idType), idType);
                        binaryNew.write_uint16(offsetof(XIcon::ICONDIR, idCount), nNumberOfRecords);

                        qint64 nCurrentTableOffset = sizeof(XIcon::ICONDIR);
                        qint64 nCurrentDataOffset = sizeof(XIcon::ICONDIR) + sizeof(XIcon::ICONDIRENTRY) * nNumberOfRecords;

                        for (qint32 i = 0; i < nNumberOfRecords; i++) {
                            binaryNew.write_uint8(nCurrentTableOffset + offsetof(XIcon::ICONDIRENTRY, bWidth), listDirectories.at(i).bWidth);
                            binaryNew.write_uint8(nCurrentTableOffset + offsetof(XIcon::ICONDIRENTRY, bHeight), listDirectories.at(i).bHeight);
                            binaryNew.write_uint8(nCurrentTableOffset + offsetof(XIcon::ICONDIRENTRY, bColorCount), listDirectories.at(i).bColorCount);
                            binaryNew.write_uint8(nCurrentTableOffset + offsetof(XIcon::ICONDIRENTRY, bReserved), listDirectories.at(i).bReserved);
                            binaryNew.write_uint16(nCurrentTableOffset + offsetof(XIcon::ICONDIRENTRY, wPlanes), listDirectories.at(i).wPlanes);
                            binaryNew.write_uint16(nCurrentTableOffset + offsetof(XIcon::ICONDIRENTRY, wBitCount), listDirectories.at(i).wBitCount);
                            binaryNew.write_uint32(nCurrentTableOffset + offsetof(XIcon::ICONDIRENTRY, dwBytesInRes), listDirectories.at(i).dwBytesInRes);
                            binaryNew.write_uint32(nCurrentTableOffset + offsetof(XIcon::ICONDIRENTRY, dwImageOffset), nCurrentDataOffset);

                            XBinary::copyDeviceMemory(pDevice, listChunkRecords.at(i).nOffset, &file, nCurrentDataOffset, listDirectories.at(i).dwBytesInRes);

                            nCurrentTableOffset += sizeof(XIcon::ICONDIRENTRY);
                            nCurrentDataOffset += listDirectories.at(i).dwBytesInRes;
                        }

                        file.close();
                    }
                }

                sd.close();
            }
        }
    }

    return bResult;
}

QSet<XBinary::FT> XFormats::getFileTypes(QIODevice *pDevice, qint64 nOffset, qint64 nSize, bool bExtra, XBinary::PDSTRUCT *pPdStruct)
{
    QSet<XBinary::FT> result;

    if (nOffset >= 0) {
        SubDevice sd(pDevice, nOffset, nSize);

        if (sd.open(QIODevice::ReadOnly)) {
            result = getFileTypes(&sd, bExtra, pPdStruct);

            sd.close();
        }
    }

    return result;
}

QSet<XBinary::FT> XFormats::getFileTypes(const QString &sFileName, bool bExtra, XBinary::PDSTRUCT *pPdStruct)
{
    QSet<XBinary::FT> stResult;

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        stResult = getFileTypes(&file, bExtra, pPdStruct);

        file.close();
    }

    return stResult;
}

QSet<XBinary::FT> XFormats::getFileTypes(QByteArray *pbaData, bool bExtra)
{
    QSet<XBinary::FT> stResult;

    QBuffer buffer;

    buffer.setBuffer(pbaData);

    if (buffer.open(QIODevice::ReadOnly)) {
        stResult = getFileTypes(&buffer, bExtra);

        buffer.close();
    }

    return stResult;
}

XBinary::FILEFORMATINFO XFormats::getFileFormatInfo(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress, XBinary::PDSTRUCT *pPdStruct,
                                                    qint64 nOffset, qint64 nSize)
{
    XBinary::FILEFORMATINFO result = {};

    QIODevice *_pDevice = nullptr;
    SubDevice *pSubDevice = nullptr;

    if (nOffset != 0) {
        pSubDevice = new SubDevice(pDevice, nOffset, nSize);
        pSubDevice->open(QIODevice::ReadOnly);
        _pDevice = pSubDevice;
    } else {
        _pDevice = pDevice;
    }

    // Executable formats
    if (XBinary::checkFileType(XBinary::FT_COM, fileType)) result = XCOM(_pDevice, bIsImage, nModuleAddress).getFileFormatInfo(pPdStruct);

    else if (XBinary::checkFileType(XBinary::FT_MSDOS, fileType)) result = XMSDOS(_pDevice, bIsImage, nModuleAddress).getFileFormatInfo(pPdStruct);

    else if (XBinary::checkFileType(XBinary::FT_NE, fileType)) result = XNE(_pDevice, bIsImage, nModuleAddress).getFileFormatInfo(pPdStruct);

    // Combine LE and LX cases
    else if (XBinary::checkFileType(XBinary::FT_LE, fileType) || XBinary::checkFileType(XBinary::FT_LX, fileType))
        result = XLE(_pDevice, bIsImage, nModuleAddress).getFileFormatInfo(pPdStruct);

    else if (XBinary::checkFileType(XBinary::FT_PE, fileType)) result = XPE(_pDevice, bIsImage, nModuleAddress).getFileFormatInfo(pPdStruct);

    else if (XBinary::checkFileType(XBinary::FT_ELF, fileType)) result = XELF(_pDevice, bIsImage, nModuleAddress).getFileFormatInfo(pPdStruct);

    else if (XBinary::checkFileType(XBinary::FT_MACHO, fileType)) result = XMACH(_pDevice, bIsImage, nModuleAddress).getFileFormatInfo(pPdStruct);

    else if (XBinary::checkFileType(XBinary::FT_AMIGAHUNK, fileType)) result = XAmigaHunk(_pDevice, bIsImage, nModuleAddress).getFileFormatInfo(pPdStruct);

    // Image formats
    else if (XBinary::checkFileType(XBinary::FT_PNG, fileType)) result = XPNG(_pDevice).getFileFormatInfo(pPdStruct);

    else if (XBinary::checkFileType(XBinary::FT_JPEG, fileType)) result = XJpeg(_pDevice).getFileFormatInfo(pPdStruct);

    else if (XBinary::checkFileType(XBinary::FT_ICO, fileType)) result = XIcon(_pDevice).getFileFormatInfo(pPdStruct);

    else if (XBinary::checkFileType(XBinary::FT_BMP, fileType)) result = XBMP(_pDevice).getFileFormatInfo(pPdStruct);

    else if (XBinary::checkFileType(XBinary::FT_GIF, fileType)) result = XGif(_pDevice).getFileFormatInfo(pPdStruct);

    else if (XBinary::checkFileType(XBinary::FT_TIFF, fileType)) result = XTiff(_pDevice).getFileFormatInfo(pPdStruct);

    // Audio/Video formats
    else if (XBinary::checkFileType(XBinary::FT_MP4, fileType)) result = XMP4(_pDevice).getFileFormatInfo(pPdStruct);

    else if (XBinary::checkFileType(XBinary::FT_MP3, fileType)) result = XMP3(_pDevice).getFileFormatInfo(pPdStruct);

    else if (XBinary::checkFileType(XBinary::FT_RIFF, fileType)) result = XRiff(_pDevice).getFileFormatInfo(pPdStruct);

    // Document formats
    else if (XBinary::checkFileType(XBinary::FT_JAVACLASS, fileType)) result = XJavaClass(_pDevice).getFileFormatInfo(pPdStruct);

#ifdef USE_DEX
    else if (XBinary::checkFileType(XBinary::FT_DEX, fileType)) result = XDEX(_pDevice).getFileFormatInfo(pPdStruct);
#endif

#ifdef USE_PDF
    else if (XBinary::checkFileType(XBinary::FT_PDF, fileType)) result = XPDF(_pDevice).getFileFormatInfo(pPdStruct);
#endif

#ifdef USE_ARCHIVE
    // Archive formats
    else if (XBinary::checkFileType(XBinary::FT_ZIP, fileType)) result = XZip(_pDevice).getFileFormatInfo(pPdStruct);

    else if (XBinary::checkFileType(XBinary::FT_JAR, fileType)) result = XJAR(_pDevice).getFileFormatInfo(pPdStruct);

    else if (XBinary::checkFileType(XBinary::FT_APK, fileType)) result = XAPK(_pDevice).getFileFormatInfo(pPdStruct);

    else if (XBinary::checkFileType(XBinary::FT_IPA, fileType)) result = XIPA(_pDevice).getFileFormatInfo(pPdStruct);

    else if (XBinary::checkFileType(XBinary::FT_APKS, fileType)) result = XAPKS(_pDevice).getFileFormatInfo(pPdStruct);

    else if (XBinary::checkFileType(XBinary::FT_AR, fileType)) result = X_Ar(_pDevice).getFileFormatInfo(pPdStruct);

    else if (XBinary::checkFileType(XBinary::FT_DEB, fileType)) result = XDEB(_pDevice).getFileFormatInfo(pPdStruct);

    else if (XBinary::checkFileType(XBinary::FT_7Z, fileType)) result = XSevenZip(_pDevice).getFileFormatInfo(pPdStruct);

    else if (XBinary::checkFileType(XBinary::FT_CAB, fileType)) result = XCab(_pDevice).getFileFormatInfo(pPdStruct);

    else if (XBinary::checkFileType(XBinary::FT_RAR, fileType)) result = XRar(_pDevice).getFileFormatInfo(pPdStruct);

    else if (XBinary::checkFileType(XBinary::FT_MACHOFAT, fileType)) result = XMACHOFat(_pDevice).getFileFormatInfo(pPdStruct);

    else if (XBinary::checkFileType(XBinary::FT_GZIP, fileType)) result = XGzip(_pDevice).getFileFormatInfo(pPdStruct);

    else if (XBinary::checkFileType(XBinary::FT_TAR, fileType)) result = XTAR(_pDevice).getFileFormatInfo(pPdStruct);

    else if (XBinary::checkFileType(XBinary::FT_TARGZ, fileType)) result = XTGZ(_pDevice).getFileFormatInfo(pPdStruct);

    else if (XBinary::checkFileType(XBinary::FT_NPM, fileType)) result = XNPM(_pDevice).getFileFormatInfo(pPdStruct);

    else if (XBinary::checkFileType(XBinary::FT_ZLIB, fileType)) result = XZlib(_pDevice).getFileFormatInfo(pPdStruct);

    else if (XBinary::checkFileType(XBinary::FT_LHA, fileType)) result = XLHA(_pDevice).getFileFormatInfo(pPdStruct);

    else if (XBinary::checkFileType(XBinary::FT_CFBF, fileType)) result = XCFBF(_pDevice).getFileFormatInfo(pPdStruct);

    else if (XBinary::checkFileType(XArchive::FT_DOS4G, fileType) || XBinary::checkFileType(XArchive::FT_DOS16M, fileType))
        result = XDOS16(_pDevice).getFileFormatInfo(pPdStruct);
#endif

    // Default fallback
    else result = XBinary(_pDevice).getFileFormatInfo(pPdStruct);

    if (pSubDevice) {
        pSubDevice->close();
        delete pSubDevice;
    }

    return result;
}

QList<XBinary::DATA_HEADER> XFormats::getDataHeaders(XBinary::FT fileType, QIODevice *pDevice, const XBinary::DATA_HEADERS_OPTIONS &dataHeadersOptions, XBinary::PDSTRUCT *pPdStruct)
{
    if (XBinary::checkFileType(XBinary::FT_BINARY, fileType)) return XBinary(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
    else if (XBinary::checkFileType(XBinary::FT_COM, fileType)) return XCOM(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
    else if (XBinary::checkFileType(XBinary::FT_MSDOS, fileType)) return XMSDOS(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
    else if (XBinary::checkFileType(XBinary::FT_NE, fileType)) return XNE(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
    else if (XBinary::checkFileType(XBinary::FT_LE, fileType) || XBinary::checkFileType(XBinary::FT_LX, fileType))
        return XLE(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
    else if (XBinary::checkFileType(XBinary::FT_PE, fileType)) return XPE(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
    else if (XBinary::checkFileType(XBinary::FT_ELF, fileType)) return XELF(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
    else if (XBinary::checkFileType(XBinary::FT_MACHO, fileType)) return XMACH(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
    else if (XBinary::checkFileType(XBinary::FT_AMIGAHUNK, fileType)) return XAmigaHunk(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
    else if (XBinary::checkFileType(XBinary::FT_PNG, fileType)) return XPNG(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
    else if (XBinary::checkFileType(XBinary::FT_JPEG, fileType)) return XJpeg(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
    else if (XBinary::checkFileType(XBinary::FT_ICO, fileType)) return XIcon(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
    else if (XBinary::checkFileType(XBinary::FT_BMP, fileType)) return XBMP(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
    else if (XBinary::checkFileType(XBinary::FT_GIF, fileType)) return XGif(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
    else if (XBinary::checkFileType(XBinary::FT_TIFF, fileType)) return XTiff(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
    else if (XBinary::checkFileType(XBinary::FT_MP4, fileType)) return XMP4(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
    else if (XBinary::checkFileType(XBinary::FT_MP3, fileType)) return XMP3(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
    else if (XBinary::checkFileType(XBinary::FT_RIFF, fileType)) return XRiff(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
    else if (XBinary::checkFileType(XBinary::FT_JAVACLASS, fileType)) return XJavaClass(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
#ifdef USE_DEX
    else if (XBinary::checkFileType(XBinary::FT_DEX, fileType)) return XDEX(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
#endif
#ifdef USE_PDF
    else if (XBinary::checkFileType(XBinary::FT_PDF, fileType)) return XPDF(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
#endif
#ifdef USE_ARCHIVE
    else if (XBinary::checkFileType(XBinary::FT_ZIP, fileType)) return XZip(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
    else if (XBinary::checkFileType(XBinary::FT_JAR, fileType)) return XJAR(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
    else if (XBinary::checkFileType(XBinary::FT_APK, fileType)) return XAPK(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
    else if (XBinary::checkFileType(XBinary::FT_IPA, fileType)) return XIPA(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
    else if (XBinary::checkFileType(XBinary::FT_APKS, fileType)) return XAPKS(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
    else if (XBinary::checkFileType(XBinary::FT_7Z, fileType)) return XSevenZip(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
    else if (XBinary::checkFileType(XBinary::FT_CAB, fileType)) return XCab(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
    else if (XBinary::checkFileType(XBinary::FT_RAR, fileType)) return XRar(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
    else if (XBinary::checkFileType(XBinary::FT_MACHOFAT, fileType)) return XMACHOFat(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
    else if (XBinary::checkFileType(XBinary::FT_NPM, fileType)) return XNPM(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
    else if (XBinary::checkFileType(XBinary::FT_TARGZ, fileType)) return XTGZ(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
    else if (XBinary::checkFileType(XBinary::FT_GZIP, fileType)) return XGzip(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
    else if (XBinary::checkFileType(XBinary::FT_ZLIB, fileType)) return XZlib(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
    else if (XBinary::checkFileType(XBinary::FT_LHA, fileType)) return XLHA(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
    else if (XBinary::checkFileType(XBinary::FT_CFBF, fileType)) return XCFBF(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
    else if (XBinary::checkFileType(XArchive::FT_DOS4G, fileType) || XBinary::checkFileType(XArchive::FT_DOS16M, fileType))
        return XDOS16(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
#endif
    // Default fallback
    return XBinary(pDevice).getDataHeaders(dataHeadersOptions, pPdStruct);
}

QList<XBinary::FPART> XFormats::getFileParts(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress, XBinary::PDSTRUCT *pPdStruct)
{
    QList<XBinary::FPART> listResult;

    XBinary *pBinary = XFormats::getClass(fileType, pDevice, bIsImage, nModuleAddress);
    listResult = pBinary->getFileParts(pPdStruct);
    delete pBinary;

    return listResult;
}
#ifdef USE_ARCHIVE
QSet<XBinary::FT> XFormats::getFileTypes(QIODevice *pDevice, XArchive::RECORD *pRecord, bool bExtra)
{
    QSet<XBinary::FT> stResult;

    QByteArray baData = XArchives::decompress(pDevice, pRecord, 0, 0, 0x200);

    stResult = XFormats::getFileTypes(&baData, bExtra);

    return stResult;
}
#endif
#ifdef USE_ARCHIVE
QSet<XBinary::FT> XFormats::getFileTypesZIP(QIODevice *pDevice, QList<XArchive::RECORD> *pListRecords, XBinary::PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pDevice)  // TODO

    QSet<XBinary::FT> stResult;

    // XBinary::FT fileType = XZip::_getFileType(pDevice, pListRecords, true);
    stResult.insert(XBinary::FT_ZIP);

    if (XAPK::isValid(pListRecords, pPdStruct)) {
        stResult.insert(XBinary::FT_APK);
        stResult.insert(XBinary::FT_JAR);
    } else if (XIPA::isValid(pListRecords, pPdStruct)) {
        stResult.insert(XBinary::FT_IPA);
    } else if (XJAR::isValid(pListRecords, pPdStruct)) {
        stResult.insert(XBinary::FT_JAR);
    }
    // TODO APKS

    return stResult;
}
#endif
#ifdef USE_ARCHIVE
QSet<XBinary::FT> XFormats::getFileTypesZIP(QIODevice *pDevice, XBinary::PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pDevice)  // TODO

    QSet<XBinary::FT> stResult;

    XZip xzip(pDevice);

    if (xzip.isValid(pPdStruct)) {
        stResult.insert(XBinary::FT_ZIP);

        qint64 nECDOffset = xzip.findECDOffset(pPdStruct);

        if (xzip.isAPK(nECDOffset, pPdStruct)) {
            stResult.insert(XBinary::FT_APK);
            stResult.insert(XBinary::FT_JAR);
        } else if (xzip.isIPA(nECDOffset, pPdStruct)) {
            stResult.insert(XBinary::FT_IPA);
        } else if (xzip.isJAR(nECDOffset, pPdStruct)) {
            stResult.insert(XBinary::FT_JAR);
        }
        // TODO APKS
    }

    // XBinary::FT fileType = XZip::_getFileType(pDevice, pListRecords, true);
    return stResult;
}
#endif
#ifdef USE_ARCHIVE
QSet<XBinary::FT> XFormats::getFileTypesTGZ(QIODevice *pDevice, QList<XArchive::RECORD> *pListRecords, XBinary::PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pDevice)  // TODO

    QSet<XBinary::FT> stResult;

    stResult.insert(XBinary::FT_TARGZ);

    if (XNPM::isValid(pListRecords, pPdStruct)) {
        stResult.insert(XBinary::FT_NPM);
    }

    return stResult;
}
#endif
#ifdef USE_ARCHIVE
QSet<XBinary::FT> XFormats::getFileTypesAR(QIODevice *pDevice, QList<XArchive::RECORD> *pListRecords, XBinary::PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pDevice)  // TODO

    QSet<XBinary::FT> stResult;

    stResult.insert(XBinary::FT_AR);

    if (XDEB::isValid(pListRecords, pPdStruct)) {
        stResult.insert(XBinary::FT_DEB);
    }

    return stResult;
}
#endif
#ifdef USE_ARCHIVE
QSet<XBinary::FT> XFormats::getFileTypesGZIP(QIODevice *pDevice, QList<XArchive::RECORD> *pListRecords, XBinary::PDSTRUCT *pPdStruct)
{
    QSet<XBinary::FT> stResult;

    if (pListRecords->count()) {
        const XArchive::RECORD &record = pListRecords->at(0);
        QByteArray baData = XArchives::decompress(pDevice, &record, pPdStruct, 0, 0x200);

        QSet<XBinary::FT> _ft = getFileTypes(&baData, true);

        if (_ft.contains(XBinary::FT_TAR)) {
            XTGZ xtgz(pDevice);
            QList<XArchive::RECORD> listArchiveRecords = xtgz.getRecords(20000, pPdStruct);

            stResult += getFileTypesTGZ(pDevice, &listArchiveRecords, pPdStruct);
        }
    }

    return stResult;
}
#endif
QSet<XBinary::FT> XFormats::_getFileTypes(QIODevice *pDevice, bool bExtra, XBinary::PDSTRUCT *pPdStruct)
{
#ifdef QT_DEBUG
    qDebug("XFormats::_getFileTypes");
    QElapsedTimer timer;
    timer.start();
#endif

    QSet<XBinary::FT> stResult = XBinary::getFileTypes(pDevice, bExtra);

#ifdef USE_ARCHIVE
    if (pDevice->property(XBinary::fileTypeIdToFtString(XBinary::FT_ARCHIVE).toLatin1().data()).toBool()) {
        // Cache
        QList<QByteArray> listProperties = pDevice->dynamicPropertyNames();

        qint32 nNumberOfProperties = listProperties.count();

        for (qint32 i = 0; i < nNumberOfProperties; i++) {
            QByteArray baProperty = listProperties.at(i);
            XBinary::FT fileType = XBinary::ftStringToFileTypeId(baProperty);

            if (fileType != XBinary::FT_UNKNOWN) {
                stResult.insert(fileType);
            }
        }
    } else {
        if (stResult.contains(XBinary::FT_ZIP)) {
            XZip xzip(pDevice);

            if (xzip.isValid(pPdStruct)) {
                // QList<XArchive::RECORD> listArchiveRecords = xzip.getRecords(20000, pPdStruct);

                // stResult += getFileTypesZIP(pDevice, &listArchiveRecords, pPdStruct);
                stResult += getFileTypesZIP(pDevice, pPdStruct);
            }
        } else if (stResult.contains(XBinary::FT_AR)) {
            X_Ar xar(pDevice);

            if (xar.isValid(pPdStruct)) {
                QList<XArchive::RECORD> listArchiveRecords = xar.getRecords(20000, pPdStruct);

                stResult += getFileTypesAR(pDevice, &listArchiveRecords, pPdStruct);
            }
        } else if (stResult.contains(XBinary::FT_GZIP)) {
            // TODO Check
            if (pDevice->size() < 100000000) {
                XGzip xgzip(pDevice);

                if (xgzip.isValid(pPdStruct)) {
                    QList<XArchive::RECORD> listArchiveRecords = xgzip.getRecords(1, pPdStruct);

                    stResult += getFileTypesGZIP(pDevice, &listArchiveRecords, pPdStruct);
                }
            }
        }

        if (stResult.contains(XBinary::FT_ARCHIVE)) {
            QList<XBinary::FT> listFT = stResult.values();

            qint32 nNumberOfFT = listFT.count();

            for (qint32 i = 0; i < nNumberOfFT; i++) {
                XBinary::FT ft = listFT.at(i);

                pDevice->setProperty(XBinary::fileTypeIdToFtString(ft).toLatin1().data(), true);
            }
        }
    }
#endif

#ifdef QT_DEBUG
    qint64 nElapsed = timer.elapsed();
    qDebug("XFormats::_getFileTypes time=%lld", nElapsed);
#endif

    return stResult;
}

#ifdef QT_GUI_LIB
XBinary::FT XFormats::setFileTypeComboBox(XBinary::FT fileType, QIODevice *pDevice, QComboBox *pComboBox, XBinary::TL_OPTION tlOption)
{
    const bool bBlocked1 = pComboBox->blockSignals(true);

    QList<XBinary::FT> listFileTypes;

    if (fileType != XBinary::FT_REGION) {
        QSet<XBinary::FT> stFileType = XFormats::getFileTypes(pDevice, true);

        listFileTypes = XBinary::_getFileTypeListFromSet(stFileType, tlOption);
    } else {
        listFileTypes.append(XBinary::FT_REGION);
    }

    pComboBox->clear();

    qint32 nNumberOfListTypes = listFileTypes.count();

    for (qint32 i = 0; i < nNumberOfListTypes; i++) {
        XBinary::FT _fileType = listFileTypes.at(i);
        pComboBox->addItem(XBinary::fileTypeIdToString(_fileType), _fileType);
    }

    if (nNumberOfListTypes) {
        if (fileType == XBinary::FT_UNKNOWN) {
            pComboBox->setCurrentIndex(nNumberOfListTypes - 1);
        } else {
            setComboBoxCurrent(pComboBox, fileType);
        }
    }

    pComboBox->blockSignals(bBlocked1);

    return (XBinary::FT)(pComboBox->currentData().toUInt());
}
#endif
#ifdef QT_GUI_LIB
XBinary::FT XFormats::setFileTypeComboBox(XBinary::FT fileType, const QString &sFileName, QComboBox *pComboBox, XBinary::TL_OPTION tlOption)
{
    XBinary::FT result = XBinary::FT_UNKNOWN;

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        result = setFileTypeComboBox(fileType, &file, pComboBox, tlOption);

        file.close();
    }

    return result;
}
#endif
#ifdef QT_GUI_LIB
QVariant XFormats::setComboBoxCurrent(QComboBox *pComboBox, QVariant varValue)
{
    QVariant varResult;

    const bool bBlocked1 = pComboBox->blockSignals(true);

    qint32 nNumberOfItems = pComboBox->count();

    for (qint32 i = 0; i < nNumberOfItems; i++) {
        if (pComboBox->itemData(i) == varValue) {
            pComboBox->setCurrentIndex(i);

            varResult = varValue;

            break;
        }
    }

    pComboBox->blockSignals(bBlocked1);

    return varResult;
}

#endif
#ifdef QT_GUI_LIB
XBinary::ENDIAN XFormats::setEndiannessComboBox(QComboBox *pComboBox, XBinary::ENDIAN endian)
{
    const bool bBlocked1 = pComboBox->blockSignals(true);

    pComboBox->clear();

    pComboBox->addItem(XBinary::endianToString(XBinary::ENDIAN_LITTLE), XBinary::ENDIAN_LITTLE);
    pComboBox->addItem(XBinary::endianToString(XBinary::ENDIAN_BIG), XBinary::ENDIAN_BIG);

    XBinary::ENDIAN result = (XBinary::ENDIAN)setComboBoxCurrent(pComboBox, endian).toUInt();

    pComboBox->blockSignals(bBlocked1);

    return result;
}
#endif
#ifdef QT_GUI_LIB
qint32 XFormats::setBaseComboBox(QComboBox *pComboBox, qint32 nBase)
{
    const bool bBlocked1 = pComboBox->blockSignals(true);

    pComboBox->clear();

    pComboBox->addItem(QString("10"), 10);
    pComboBox->addItem(QString("16"), 16);

    qint32 nResult = setComboBoxCurrent(pComboBox, nBase).toInt();

    pComboBox->blockSignals(bBlocked1);

    return nResult;
}
#endif
#ifdef QT_GUI_LIB
XBinary::MAPMODE XFormats::getMapModesList(XBinary::FT fileType, QComboBox *pComboBox)
{
    XBinary::MAPMODE result = XBinary::MAPMODE_UNKNOWN;

    const bool bBlocked1 = pComboBox->blockSignals(true);

    pComboBox->clear();

    QList<XBinary::MAPMODE> listMapModes = getMapModesList(fileType);

    qint32 nNumberOfRecords = listMapModes.count();

    if (nNumberOfRecords) {
        result = listMapModes.at(0);

        for (qint32 i = 0; i < nNumberOfRecords; i++) {
            XBinary::MAPMODE record = listMapModes.at(i);
            pComboBox->addItem(XBinary::mapModeToString(record), record);
        }
    }

    pComboBox->blockSignals(bBlocked1);

    return result;
}
#endif
#ifdef QT_GUI_LIB
void XFormats::setProgressBar(QProgressBar *pProgressBar, XBinary::PDRECORD pdRecord)
{
    if ((pdRecord.nTotal) || (pdRecord.sStatus != "")) {
        pProgressBar->show();

        if (pdRecord.nTotal) {
            pProgressBar->setMaximum(pdRecord.nTotal);
            pProgressBar->setValue(pdRecord.nCurrent);
        }

        if (pdRecord.sStatus != "") {
            pProgressBar->setFormat(pdRecord.sStatus);
        }
    } else {
        pProgressBar->hide();
    }
}
#endif
#ifdef QT_GUI_LIB
XBinary::DM XFormats::setDisasmModeComboBox(XBinary::DM disasmMode, QComboBox *pComboBox)
{
    XBinary::DM result = disasmMode;

    const bool bBlocked1 = pComboBox->blockSignals(true);

    pComboBox->clear();

    if (disasmMode == XBinary::DM_UNKNOWN) {
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_X86_16), XBinary::DM_X86_16);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_X86_32), XBinary::DM_X86_32);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_X86_64), XBinary::DM_X86_64);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_ARM_LE), XBinary::DM_ARM_LE);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_ARM_BE), XBinary::DM_ARM_BE);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_AARCH64_LE), XBinary::DM_AARCH64_LE);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_AARCH64_BE), XBinary::DM_AARCH64_BE);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_CORTEXM), XBinary::DM_CORTEXM);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_THUMB_LE), XBinary::DM_THUMB_LE);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_THUMB_BE), XBinary::DM_THUMB_BE);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_MIPS_LE), XBinary::DM_MIPS_LE);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_MIPS_BE), XBinary::DM_MIPS_BE);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_MIPS64_LE), XBinary::DM_MIPS64_LE);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_MIPS64_BE), XBinary::DM_MIPS64_BE);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_PPC_LE), XBinary::DM_PPC_LE);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_PPC_BE), XBinary::DM_PPC_BE);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_PPC64_LE), XBinary::DM_PPC64_LE);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_PPC64_BE), XBinary::DM_PPC64_BE);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_SPARC), XBinary::DM_SPARC);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_S390X), XBinary::DM_S390X);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_XCORE), XBinary::DM_XCORE);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_M68K), XBinary::DM_M68K);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_M68K00), XBinary::DM_M68K00);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_M68K10), XBinary::DM_M68K10);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_M68K20), XBinary::DM_M68K20);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_M68K30), XBinary::DM_M68K30);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_M68K40), XBinary::DM_M68K40);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_M68K60), XBinary::DM_M68K60);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_TMS320C64X), XBinary::DM_TMS320C64X);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_M6800), XBinary::DM_M6800);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_M6801), XBinary::DM_M6801);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_M6805), XBinary::DM_M6805);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_M6808), XBinary::DM_M6808);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_M6809), XBinary::DM_M6809);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_M6811), XBinary::DM_M6811);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_CPU12), XBinary::DM_CPU12);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_HD6301), XBinary::DM_HD6301);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_HD6309), XBinary::DM_HD6309);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_HCS08), XBinary::DM_HCS08);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_EVM), XBinary::DM_EVM);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_MOS65XX), XBinary::DM_MOS65XX);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_RISKV32), XBinary::DM_RISKV32);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_RISKV64), XBinary::DM_RISKV64);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_RISKVC), XBinary::DM_RISKVC);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_MOS65XX), XBinary::DM_MOS65XX);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_WASM), XBinary::DM_WASM);
    } else {
        pComboBox->addItem(XBinary::disasmIdToString(disasmMode), disasmMode);
    }

    pComboBox->blockSignals(bBlocked1);

    return result;
}
#endif
