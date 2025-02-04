/* Copyright (c) 2017-2024 hors<horsicq@gmail.com>
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
#include "xbinary.h"

const quint32 _crc32_EDB88320_tab[] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b,
    0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856, 0x646ba8c0,
    0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd,
    0xa50ab56b, 0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
    0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190,
    0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433, 0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
    0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea,
    0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65, 0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
    0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525,
    0x206f85b3, 0xb966d409, 0xce61e49f, 0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6,
    0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27,
    0x7d079eb1, 0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b, 0xd80d2bda,
    0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703,
    0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c,
    0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
    0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777, 0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff,
    0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9, 0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729,
    0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d};

const quint16 _crc16_tab[] = {
    0x0000, 0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241, 0xc601, 0x06c0, 0x0780, 0xc741, 0x0500, 0xc5c1, 0xc481, 0x0440, 0xcc01, 0x0cc0, 0x0d80, 0xcd41,
    0x0f00, 0xcfc1, 0xce81, 0x0e40, 0x0a00, 0xcac1, 0xcb81, 0x0b40, 0xc901, 0x09c0, 0x0880, 0xc841, 0xd801, 0x18c0, 0x1980, 0xd941, 0x1b00, 0xdbc1, 0xda81, 0x1a40,
    0x1e00, 0xdec1, 0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41, 0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641, 0xd201, 0x12c0, 0x1380, 0xd341,
    0x1100, 0xd1c1, 0xd081, 0x1040, 0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240, 0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441,
    0x3c00, 0xfcc1, 0xfd81, 0x3d40, 0xff01, 0x3fc0, 0x3e80, 0xfe41, 0xfa01, 0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840, 0x2800, 0xe8c1, 0xe981, 0x2940,
    0xeb01, 0x2bc0, 0x2a80, 0xea41, 0xee01, 0x2ec0, 0x2f80, 0xef41, 0x2d00, 0xedc1, 0xec81, 0x2c40, 0xe401, 0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640,
    0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0, 0x2080, 0xe041, 0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281, 0x6240, 0x6600, 0xa6c1, 0xa781, 0x6740,
    0xa501, 0x65c0, 0x6480, 0xa441, 0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41, 0xaa01, 0x6ac0, 0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840,
    0x7800, 0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41, 0xbe01, 0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40, 0xb401, 0x74c0, 0x7580, 0xb541,
    0x7700, 0xb7c1, 0xb681, 0x7640, 0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041, 0x5000, 0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0, 0x5280, 0x9241,
    0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481, 0x5440, 0x9c01, 0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40, 0x5a00, 0x9ac1, 0x9b81, 0x5b40,
    0x9901, 0x59c0, 0x5880, 0x9841, 0x8801, 0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40, 0x4e00, 0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41,
    0x4400, 0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641, 0x8201, 0x42c0, 0x4380, 0x8341, 0x4100, 0x81c1, 0x8081, 0x4040};

const double XBinary::D_ENTROPY_THRESHOLD = 6.5;

XBinary::XBinary(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    setData(pDevice, bIsImage, nModuleAddress);
}

XBinary::XBinary(const QString &sFileName)
{
    g_sFileName = sFileName;

    if (g_pFile) {
        g_pFile->close();

        delete g_pFile;
    }

    g_pFile = new QFile(sFileName);

    tryToOpen(g_pFile);

    setData(g_pFile);
}

XBinary::~XBinary()
{
    if (g_pFile) {
        g_pFile->close();

        delete g_pFile;
    }
}

void XBinary::setData(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    g_pReadWriteMutex = nullptr;
    g_nSize = 0;
    g_nFileFormatSize = 0;
    g_pFile = nullptr;
    g_pConstMemory = nullptr;

    setDevice(pDevice);
    setIsImage(bIsImage);
    XBinary::setBaseAddress(0);
    setModuleAddress(nModuleAddress);
    setEndian(ENDIAN_UNKNOWN);
    XBinary::setEntryPointOffset(0);
    setMode(MODE_UNKNOWN);
    setFileType(FT_BINARY);
    setArch("NOEXEC");
    setVersion("");
    setType(TYPE_UNKNOWN);
    setOsType(OSNAME_UNKNOWN);
    setOsVersion("");
    setMultiSearchCallbackState(false);

    if (pDevice) {
        // qDebug("%s",XBinary::valueToHex((quint64)pDevice).toLatin1().data());
        setFileFormatSize(pDevice->size());
    }
}

void XBinary::setDevice(QIODevice *pDevice)
{
    g_pDevice = pDevice;

    if (g_pDevice) {
        QBuffer *pBuffer = dynamic_cast<QBuffer *>(pDevice);

        if (pBuffer) {
            g_pConstMemory = pBuffer->data().data();
        } else {
            g_pConstMemory = nullptr;
        }

        if (g_pReadWriteMutex) g_pReadWriteMutex->lock();

        // qDebug("%s",XBinary::valueToHex((quint64)g_pDevice).toLatin1().data());

        g_nSize = g_pDevice->size();

        if (g_pReadWriteMutex) g_pReadWriteMutex->unlock();
    }
}

void XBinary::setReadWriteMutex(QMutex *pReadWriteMutex)
{
    g_pReadWriteMutex = pReadWriteMutex;
}

void XBinary::setFileName(const QString &sFileName)
{
    g_sFileName = sFileName;
}

qint64 XBinary::safeReadData(QIODevice *pDevice, qint64 nPos, char *pData, qint64 nMaxLen, PDSTRUCT *pPdStruct)
{
    qint64 nResult = 0;

    if (g_pReadWriteMutex) g_pReadWriteMutex->lock();

    if ((pDevice->size() > nPos) && (nPos >= 0)) {
        if (pDevice->seek(nPos)) {
            while ((nMaxLen > 0) && (!(pPdStruct->bIsStop))) {
                qint64 nCurrentSize = qMin(nMaxLen, (qint64)READWRITE_BUFFER_SIZE);

                nCurrentSize = pDevice->read(pData, nCurrentSize);

                if (nCurrentSize <= 0) {
                    break;
                }

                nMaxLen -= nCurrentSize;
                pData += nCurrentSize;
                nResult += nCurrentSize;
            }
        } else {
#ifdef QT_DEBUG
            qDebug("Cannot seek");
#endif
        }

    } else {
#ifdef QT_DEBUG
        qDebug("Invalid pos: %X", nPos);
#endif
    }

    if (g_pReadWriteMutex) g_pReadWriteMutex->unlock();

    return nResult;
}

qint64 XBinary::safeWriteData(QIODevice *pDevice, qint64 nPos, const char *pData, qint64 nLen, PDSTRUCT *pPdStruct)
{
    qint64 nResult = 0;

    if (g_pReadWriteMutex) g_pReadWriteMutex->lock();

    if (pDevice->size() > nPos) {
        if (pDevice->seek(nPos)) {
            while ((nLen > 0) && (!(pPdStruct->bIsStop))) {
                qint64 nCurrentSize = qMin(nLen, (qint64)READWRITE_BUFFER_SIZE);

                nCurrentSize = pDevice->write(pData, nCurrentSize);

                if (nCurrentSize == 0) {
                    break;
                }

                nLen -= nCurrentSize;
                pData += nCurrentSize;
                nResult += nCurrentSize;
            }
        } 
    }

    if (g_pReadWriteMutex) g_pReadWriteMutex->unlock();

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

qint64 XBinary::getSize(const QString &sFileName)
{
    qint64 nResult = 0;

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        nResult = getSize(&file);

        file.close();
    }

    return nResult;
}

void XBinary::setMode(XBinary::MODE mode)
{
    g_mode = mode;
}

XBinary::MODE XBinary::getMode()
{
    return g_mode;
}

void XBinary::setType(qint32 nType)
{
    g_nType = nType;
}

qint32 XBinary::getType()
{
    return g_nType;
}

QString XBinary::typeIdToString(qint32 nType)
{
    QString sResult = tr("Unknown");

    switch (nType) {
        case TYPE_UNKNOWN: sResult = tr("Unknown");
    }

    return sResult;
}

QString XBinary::getTypeAsString()
{
    return typeIdToString(getType());
}

void XBinary::setFileType(XBinary::FT fileType)
{
    g_fileType = fileType;
}

XBinary::FT XBinary::getFileType()
{
    return g_fileType;
}

QString XBinary::modeIdToString(XBinary::MODE mode)
{
    QString sResult = tr("Unknown");

    switch (mode) {
        case MODE_UNKNOWN: sResult = tr("Unknown"); break;
        case MODE_DATA: sResult = QString("Data"); break;  // mb TODO translate
        case MODE_BIT: sResult = QString("BIT"); break;    // mb TODO translate
        case MODE_8: sResult = tr("8-bit"); break;
        case MODE_16: sResult = tr("16-bit"); break;
        case MODE_16SEG: sResult = QString("16SEG"); break;
        case MODE_32: sResult = tr("32-bit"); break;
        case MODE_64: sResult = tr("64-bit"); break;
        case MODE_128: sResult = tr("128-bit"); break;
        case MODE_256: sResult = tr("256-bit"); break;
        case MODE_FREG: sResult = QString("freg"); break;
    }

    return sResult;
}

QString XBinary::endianToString(ENDIAN endian)
{
    QString sResult;

    if (endian == ENDIAN_UNKNOWN) {
        sResult = tr("Unknown");
    } else if (endian == ENDIAN_BIG) {
        sResult = "BE";
    } else if (endian == ENDIAN_LITTLE) {
        sResult = "LE";
    }

    return sResult;
}

void XBinary::setArch(const QString &sArch)
{
    g_sArch = sArch;
}

QString XBinary::getArch()
{
    return g_sArch;
}

void XBinary::setFileFormatName(const QString &sFileFormatString)
{
    g_sFileFormatString = sFileFormatString;
}

QString XBinary::getFileFormatString()
{
#ifdef QT_DEBUG
    // qDebug("TODO: XBinary::getFileFormatString()");
#endif
    return g_sFileFormatString;
}

void XBinary::setFileFormatExt(const QString &sFileFormatExt)
{
    g_sFileFormatExt = sFileFormatExt;
}

QString XBinary::getFileFormatExt()
{
#ifdef QT_DEBUG
    // qDebug("TODO: XBinary::getFileFormatExt()");
#endif
    return g_sFileFormatExt;
}

void XBinary::setFileFormatSize(qint64 nFileFormatSize)
{
    g_nFileFormatSize = nFileFormatSize;
}

qint64 XBinary::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    return g_nFileFormatSize;
}

bool XBinary::isSigned()
{
    return false;
}

XBinary::OFFSETSIZE XBinary::getSignOffsetSize()
{
    OFFSETSIZE osResult = {};

    return osResult;
}

void XBinary::setOsType(OSNAME osName)
{
    g_osName = osName;
}

void XBinary::setOsVersion(const QString &sOsVersion)
{
    g_sOsVersion = sOsVersion;
}

XBinary::OSINFO XBinary::getOsInfo()
{
    OSINFO result = {};

    result.osName = g_osName;
    result.sOsVersion = g_sOsVersion;
    result.sArch = getArch();
    result.mode = getMode();
    result.sType = typeIdToString(getType());
    result.endian = getEndian();

    return result;
}

XBinary::FILEFORMATINFO XBinary::getFileFormatInfo(PDSTRUCT *pPdStruct)
{
    FILEFORMATINFO result = {};

    result.bIsValid = isValid(pPdStruct);

    if (result.bIsValid) {
        result.nSize = getFileFormatSize(pPdStruct);

        if (result.nSize > 0) {
            result.fileType = getFileType();
            result.sString = getFileFormatString();
            result.sExt = getFileFormatExt();
            result.sVersion = getVersion();
            result.sOptions = getOptions();
        } else {
            result.bIsValid = false;
        }
    }

    return result;
}

void XBinary::setEndian(ENDIAN endian)
{
    g_endian = endian;
}

XBinary::ENDIAN XBinary::getEndian()
{
    return g_endian;
}

bool XBinary::isPacked(double dEntropy)
{
    return (dEntropy >= D_ENTROPY_THRESHOLD);  // TODO Check
}

quint8 XBinary::random8()
{
    return (quint8)random16();
}

quint16 XBinary::random16()
{
    quint16 nResult = 0;

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    nResult = (quint16)(QRandomGenerator::global()->generate());
#elif (QT_VERSION_MAJOR >= 6)
    nResult = (quint16)(QRandomGenerator::global()->generate());
#else
    static quint32 nSeed = 0;

    if (!nSeed) {
        quint32 nRValue = QDateTime::currentMSecsSinceEpoch() & 0xFFFFFFFF;

        nSeed ^= nRValue;
        qsrand(nSeed);
    }
    nResult = (quint16)qrand();
#endif

    return nResult;
}

quint32 XBinary::random32()
{
    quint16 nValue1 = random16();
    quint16 nValue2 = random16();

    return (nValue1 << 16) + nValue2;
}

quint64 XBinary::random64()
{
    quint64 nVal1 = random32();
    quint64 nVal2 = random32();

    nVal1 = nVal1 << 32;

    return nVal1 + nVal2;
}

quint64 XBinary::random(quint64 nLimit)
{
    quint64 nResult = 0;

    if (nLimit) {
        nResult = (random64()) % nLimit;
    }

    return nResult;
}

QString XBinary::randomString(qint32 nSize)
{
    QString sResult;
    sResult.reserve(nSize);

    for (int i = 0; i < nSize; i++) {
        // sResult += QChar('A' + (char)random(26 * 2));
        sResult += QChar('A' + (char)random(26));
    }

    return sResult;
}

QString XBinary::fileTypeIdToString(XBinary::FT fileType)
{
    QString sResult = tr("Unknown");

    switch (fileType) {
        case FT_UNKNOWN: sResult = tr("Unknown"); break;
        case FT_DATA: sResult = tr("Data"); break;
        case FT_REGION: sResult = tr("Region"); break;
        case FT_PROCESS: sResult = tr("Process"); break;
        case FT_BINARY: sResult = QString("Binary"); break;
        case FT_BINARY16: sResult = QString("Binary16"); break;
        case FT_BINARY32: sResult = QString("Binary32"); break;
        case FT_BINARY64: sResult = QString("Binary64"); break;
        case FT_COM: sResult = QString("COM"); break;
        case FT_MSDOS: sResult = QString("MSDOS"); break;
        case FT_DOS16M: sResult = QString("DOS/16M"); break;
        case FT_DOS4G: sResult = QString("DOS/4G"); break;
        case FT_NE: sResult = QString("NE"); break;
        case FT_LE: sResult = QString("LE"); break;
        case FT_LX: sResult = QString("LX"); break;
        case FT_PE: sResult = QString("PE"); break;
        case FT_PE32: sResult = QString("PE32"); break;
        case FT_PE64: sResult = QString("PE64"); break;
        case FT_ELF: sResult = QString("ELF"); break;
        case FT_ELF32: sResult = QString("ELF32"); break;
        case FT_ELF64: sResult = QString("ELF64"); break;
        case FT_MACHO: sResult = QString("Mach-O"); break;
        case FT_MACHO32: sResult = QString("Mach-O32"); break;
        case FT_MACHO64: sResult = QString("Mach-O64"); break;
        case FT_AMIGAHUNK: sResult = QString("Amiga Hunk"); break;
        // Extra
        case FT_7Z: sResult = QString("7-Zip"); break;
        case FT_ANDROIDASRC: sResult = QString("Android ASRC"); break;
        case FT_ANDROIDXML: sResult = QString("Android XML"); break;
        case FT_APK: sResult = QString("APK"); break;
        case FT_APKS: sResult = QString("APKS"); break;
        case FT_AR: sResult = QString("ar"); break;  // TODO DEB
        case FT_ARCHIVE: sResult = tr("Archive"); break;
        case FT_AUDIO: sResult = tr("Audio"); break;
        case FT_AVI: sResult = QString("AVI"); break;
        case FT_BMP: sResult = QString("BMP"); break;
        case FT_CAB: sResult = QString("CAB"); break;
        case FT_CUR: sResult = QString("CUR"); break;
        case FT_DEX: sResult = QString("DEX"); break;
        case FT_DOCUMENT: sResult = tr("Document"); break;
        case FT_GIF: sResult = QString("GIF"); break;
        case FT_GZIP: sResult = QString("GZIP"); break;
        case FT_ICO: sResult = QString("ICO"); break;
        case FT_IMAGE: sResult = tr("Image"); break;
        case FT_IPA: sResult = QString("IPA"); break;
        case FT_JAR: sResult = QString("JAR"); break;
        case FT_JPEG: sResult = QString("JPEG"); break;
        case FT_LHA: sResult = QString("LHA"); break;
        case FT_MACHOFAT: sResult = QString("Mach-O FAT"); break;
        case FT_MP3: sResult = QString("MP3"); break;
        case FT_MP4: sResult = QString("MP4"); break;
        case FT_NPM: sResult = QString("NPM"); break;
        case FT_PDF: sResult = QString("PDF"); break;
        case FT_PLAINTEXT: sResult = tr("Plain Text"); break;
        case FT_PNG: sResult = QString("PNG"); break;
        case FT_RAR: sResult = QString("RAR"); break;
        case FT_RIFF: sResult = QString("RIFF"); break;
        case FT_SIGNATURE: sResult = tr("Signature"); break;
        case FT_TAR: sResult = QString("tar"); break;
        case FT_TARGZ: sResult = QString("tar.gz"); break;
        case FT_TEXT: sResult = tr("Text"); break;
        case FT_TIFF: sResult = QString("TIFF"); break;
        case FT_UNICODE: sResult = QString("Unicode"); break;
        case FT_UNICODE_BE: sResult = QString("Unicode BE"); break;
        case FT_UNICODE_LE: sResult = QString("Unicode LE"); break;
        case FT_UTF8: sResult = QString("UTF8"); break;
        case FT_VIDEO: sResult = tr("Video"); break;
        case FT_WEBP: sResult = QString("WebP"); break;
        case FT_ZIP: sResult = QString("ZIP"); break;
        case FT_ZLIB: sResult = QString("zlib"); break;
        case FT_DEB: sResult = QString("deb"); break;
        case FT_BWDOS16M: sResult = QString("BW DOS16M"); break;
        case FT_JAVACLASS: sResult = QString("Java Class"); break;
    }

    return sResult;
}

QString XBinary::fileTypeIdToExts(FT fileType)
{
    QString sResult = tr("Unknown");

    switch (fileType) {
        case FT_PE: sResult = QString("PE(exe, dll, sys)"); break;  // TODO Check, add more
        case FT_ELF: sResult = QString("ELF(elf, so)"); break;
        case FT_ZIP: sResult = QString("ZIP(zip, jar, apk, ipa, docx)"); break;
        case FT_RAR: sResult = QString("RAR"); break;
        case FT_PDF: sResult = QString("PDF"); break;
        case FT_7Z: sResult = QString("7-Zip"); break;
        case FT_PNG: sResult = QString("PNG"); break;
        case FT_JPEG: sResult = QString("JPEG"); break;
        case FT_CAB: sResult = QString("CAB"); break;
        case FT_ICO: sResult = QString("ICO"); break;
        case FT_CUR: sResult = QString("CUR"); break;
        case FT_TIFF: sResult = QString("TIFF"); break;
        case FT_DEX: sResult = QString("DEX"); break;
        case FT_MACHOFAT: sResult = QString("MACHOFAT"); break;
        case FT_MACHO: sResult = QString("MACHO"); break;
        case FT_BMP: sResult = QString("BMP"); break;
        case FT_GIF: sResult = QString("GIF"); break;
        case FT_MP3: sResult = QString("MP3"); break;
        case FT_MP4: sResult = QString("MP4"); break;
        case FT_RIFF: sResult = QString("RIFF(avi, webp)"); break;
        case FT_ZLIB: sResult = QString("zlib"); break;
        case FT_GZIP: sResult = QString("GZIP(gz, tgz, tar.gz)"); break;
        case FT_LE: sResult = QString("LE(le, lx)"); break;
        case FT_NE: sResult = QString("NE"); break;
        case FT_AMIGAHUNK: sResult = QString("AmigaHunk"); break;
        case FT_SIGNATURE: sResult = tr("Signatures"); break;
        default: sResult = tr("Unknown");
    }

    return sResult;
}

XBinary::FT XBinary::ftStringToFileTypeId(QString sFileType)
{
    FT result = FT_UNKNOWN;

    if (sFileType == "DATA") result = FT_DATA;
    else if (sFileType == "REGION") result = FT_REGION;
    else if (sFileType == "PROCESS") result = FT_PROCESS;
    else if (sFileType == "BINARY") result = FT_BINARY;
    else if (sFileType == "BINARY16") result = FT_BINARY16;
    else if (sFileType == "BINARY32") result = FT_BINARY32;
    else if (sFileType == "BINARY64") result = FT_BINARY64;
    else if (sFileType == "COM") result = FT_COM;
    else if (sFileType == "MSDOS") result = FT_MSDOS;
    else if (sFileType == "DOS16M") result = FT_DOS16M;
    else if (sFileType == "DOS4G") result = FT_DOS4G;
    else if (sFileType == "NE") result = FT_NE;
    else if (sFileType == "LE") result = FT_LE;
    else if (sFileType == "LX") result = FT_LX;
    else if (sFileType == "PE") result = FT_PE;
    else if (sFileType == "PE32") result = FT_PE32;
    else if (sFileType == "PE64") result = FT_PE64;
    else if (sFileType == "ELF") result = FT_ELF;
    else if (sFileType == "ELF32") result = FT_ELF32;
    else if (sFileType == "ELF64") result = FT_ELF64;
    else if (sFileType == "MACHO") result = FT_MACHO;
    else if (sFileType == "MACHO32") result = FT_MACHO32;
    else if (sFileType == "MACHO64") result = FT_MACHO64;
    else if (sFileType == "AMIGAHUNK") result = FT_AMIGAHUNK;
    else if (sFileType == "7Z") result = FT_7Z;
    else if (sFileType == "ANDROIDASRC") result = FT_ANDROIDASRC;
    else if (sFileType == "ANDROIDXML") result = FT_ANDROIDXML;
    else if (sFileType == "APK") result = FT_APK;
    else if (sFileType == "APKS") result = FT_APKS;
    else if (sFileType == "AR") result = FT_AR;
    else if (sFileType == "TAR") result = FT_TAR;
    else if (sFileType == "TARGZ") result = FT_TARGZ;
    else if (sFileType == "ARCHIVE") result = FT_ARCHIVE;
    else if (sFileType == "CAB") result = FT_CAB;
    else if (sFileType == "DEX") result = FT_DEX;
    else if (sFileType == "DOCUMENT") result = FT_DOCUMENT;
    else if (sFileType == "GIF") result = FT_GIF;
    else if (sFileType == "BMP") result = FT_BMP;
    else if (sFileType == "IMAGE") result = FT_IMAGE;
    else if (sFileType == "VIDEO") result = FT_VIDEO;
    else if (sFileType == "AUDIO") result = FT_AUDIO;
    else if (sFileType == "IPA") result = FT_IPA;
    else if (sFileType == "JAR") result = FT_JAR;
    else if (sFileType == "JPEG") result = FT_JPEG;
    else if (sFileType == "MACHOFAT") result = FT_MACHOFAT;
    else if (sFileType == "PDF") result = FT_PDF;
    else if (sFileType == "PLAINTEXT") result = FT_PLAINTEXT;
    else if (sFileType == "PNG") result = FT_PNG;
    else if (sFileType == "RAR") result = FT_RAR;
    else if (sFileType == "TEXT") result = FT_TEXT;
    else if (sFileType == "TIFF") result = FT_TIFF;
    else if (sFileType == "UNICODE") result = FT_UNICODE;
    else if (sFileType == "UNICODE_BE") result = FT_UNICODE_BE;
    else if (sFileType == "UNICODE_LE") result = FT_UNICODE_LE;
    else if (sFileType == "UTF8") result = FT_UTF8;
    else if (sFileType == "ZIP") result = FT_ZIP;
    else if (sFileType == "GZIP") result = FT_GZIP;
    else if (sFileType == "ZLIB") result = FT_ZLIB;
    else if (sFileType == "LHA") result = FT_LHA;
    else if (sFileType == "ICO") result = FT_ICO;
    else if (sFileType == "CUR") result = FT_CUR;
    else if (sFileType == "MP3") result = FT_MP3;
    else if (sFileType == "MP4") result = FT_MP4;
    else if (sFileType == "RIFF") result = FT_RIFF;
    else if (sFileType == "AVI") result = FT_AVI;
    else if (sFileType == "WEBP") result = FT_WEBP;
    else if (sFileType == "SIGNATURE") result = FT_SIGNATURE;
    else if (sFileType == "NPM") result = FT_NPM;
    else if (sFileType == "DEB") result = FT_DEB;
    else if (sFileType == "BWDOS16M") result = FT_BWDOS16M;
    else if (sFileType == "JAVACLASS") result = FT_JAVACLASS;

    return result;
}

QString XBinary::fileTypeIdToFtString(FT fileType)
{
    QString sResult = "";

    switch (fileType) {
        case FT_UNKNOWN: sResult = "UNKNOWN"; break;
        case FT_DATA: sResult = "DATA"; break;
        case FT_REGION: sResult = "REGION"; break;
        case FT_PROCESS: sResult = "PROCESS"; break;
        case FT_BINARY: sResult = "BINARY"; break;
        case FT_BINARY16: sResult = "BINARY16"; break;
        case FT_BINARY32: sResult = "BINARY32"; break;
        case FT_BINARY64: sResult = "BINARY64"; break;
        case FT_COM: sResult = "COM"; break;
        case FT_MSDOS: sResult = "MSDOS"; break;
        case FT_DOS16M: sResult = "DOS16M"; break;
        case FT_DOS4G: sResult = "DOS4G"; break;
        case FT_NE: sResult = "NE"; break;
        case FT_LE: sResult = "LE"; break;
        case FT_LX: sResult = "LX"; break;
        case FT_PE: sResult = "PE"; break;
        case FT_PE32: sResult = "PE32"; break;
        case FT_PE64: sResult = "PE64"; break;
        case FT_ELF: sResult = "ELF"; break;
        case FT_ELF32: sResult = "ELF32"; break;
        case FT_ELF64: sResult = "ELF64"; break;
        case FT_MACHO: sResult = "MACHO"; break;
        case FT_MACHO32: sResult = "MACHO32"; break;
        case FT_MACHO64: sResult = "MACHO64"; break;
        case FT_AMIGAHUNK: sResult = "AMIGAHUNK"; break;
        case FT_7Z: sResult = "7Z"; break;
        case FT_ANDROIDASRC: sResult = "ANDROIDASRC"; break;
        case FT_ANDROIDXML: sResult = "ANDROIDXML"; break;
        case FT_APK: sResult = "APK"; break;
        case FT_APKS: sResult = "APKS"; break;
        case FT_AR: sResult = "AR"; break;
        case FT_ARCHIVE: sResult = "ARCHIVE"; break;
        case FT_AUDIO: sResult = "AUDIO"; break;
        case FT_AVI: sResult = "AVI"; break;
        case FT_BMP: sResult = "BMP"; break;
        case FT_CAB: sResult = "CAB"; break;
        case FT_CUR: sResult = "CUR"; break;
        case FT_DEX: sResult = "DEX"; break;
        case FT_DOCUMENT: sResult = "DOCUMENT"; break;
        case FT_GIF: sResult = "GIF"; break;
        case FT_GZIP: sResult = "GZIP"; break;
        case FT_ICO: sResult = "ICO"; break;
        case FT_IMAGE: sResult = "IMAGE"; break;
        case FT_IPA: sResult = "IPA"; break;
        case FT_JAR: sResult = "JAR"; break;
        case FT_JPEG: sResult = "JPEG"; break;
        case FT_LHA: sResult = "LHA"; break;
        case FT_MACHOFAT: sResult = "MACHOFAT"; break;
        case FT_MP3: sResult = "MP3"; break;
        case FT_MP4: sResult = "MP4"; break;
        case FT_NPM: sResult = "NPM"; break;
        case FT_PDF: sResult = "PDF"; break;
        case FT_PLAINTEXT: sResult = "PLAINTEXT"; break;
        case FT_PNG: sResult = "PNG"; break;
        case FT_RAR: sResult = "RAR"; break;
        case FT_RIFF: sResult = "RIFF"; break;
        case FT_SIGNATURE: sResult = "SIGNATURE"; break;
        case FT_TAR: sResult = "TAR"; break;
        case FT_TARGZ: sResult = "TARGZ"; break;
        case FT_TEXT: sResult = "TEXT"; break;
        case FT_TIFF: sResult = "TIFF"; break;
        case FT_UNICODE: sResult = "UNICODE"; break;
        case FT_UNICODE_BE: sResult = "UNICODE_BE"; break;
        case FT_UNICODE_LE: sResult = "UNICODE_LE"; break;
        case FT_UTF8: sResult = "UTF8"; break;
        case FT_VIDEO: sResult = "VIDEO"; break;
        case FT_WEBP: sResult = "WEBP"; break;
        case FT_ZIP: sResult = "ZIP"; break;
        case FT_ZLIB: sResult = "ZLIB"; break;
        case FT_DEB: sResult = "DEB"; break;
        case FT_BWDOS16M: sResult = "BWDOS16M"; break;
        case FT_JAVACLASS: sResult = "JAVACLASS"; break;
    }

    return sResult;
}

QString XBinary::convertFileName(const QString &sFileName)  // TODO Check
{
    QString sResult = sFileName;
#ifdef Q_OS_MAC  // Old Qt(4.X)
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

    if (fiLink.isSymLink()) {
        sResult = fiLink.symLinkTarget();
    }

    return sResult;
}

QString XBinary::convertPathName(const QString &sPathName)
{
    QString sResult = sPathName;

    // TODO more
    if (sPathName.contains("$app")) {
        sResult.replace("$app", QCoreApplication::applicationDirPath());
        sResult.replace("/", QDir::separator());
    }

    if (sPathName.contains("$data"))  // TODO Check Linux
    {
#ifdef Q_OS_MAC
        sResult.replace("$data", QCoreApplication::applicationDirPath() + "/../Resources");
#else
        sResult.replace("$data", QCoreApplication::applicationDirPath());
#endif
        sResult.replace("/", QDir::separator());
    }

    return sResult;
}

XBinary::OS_STRING XBinary::getOsAnsiString(qint64 nOffset, qint64 nSize)
{
    OS_STRING result = {};

    result.nOffset = nOffset;
    result.nSize = nSize;
    result.sString = read_ansiString(nOffset, nSize);

    return result;
}

void XBinary::findFiles(const QString &sDirectoryName, XBinary::FFOPTIONS *pFFOption, qint32 nLevel)
{
    *(pFFOption->pnNumberOfFiles) = pFFOption->pListFileNames->count();

    if (!(*pFFOption->pbIsStop)) {
        QFileInfo fi(sDirectoryName);

        if (fi.isFile()) {
            pFFOption->pListFileNames->append(fi.absoluteFilePath());
        } else if (fi.isDir() && ((pFFOption->bSubdirectories) || (nLevel == 0))) {
            QDir dir(sDirectoryName);

            QFileInfoList eil = dir.entryInfoList();

            qint32 nNumberOfFiles = eil.count();

            for (qint32 i = 0; (i < nNumberOfFiles) && (!(*(pFFOption->pbIsStop))); i++) {
                QString sFN = eil.at(i).fileName();

                if ((sFN != ".") && (sFN != "..")) {
                    findFiles(eil.at(i).absoluteFilePath(), pFFOption, nLevel + 1);
                }
            }
        }
    }
}

void XBinary::findFiles(const QString &sDirectoryName, QList<QString> *pListFileNames)
{
    QFileInfo fi(sDirectoryName);

    if (fi.isFile()) {
        pListFileNames->append(fi.absoluteFilePath());
    } else if (fi.isDir()) {
        QDir dir(sDirectoryName);

        QFileInfoList eil = dir.entryInfoList();

        qint32 nNumberOfFiles = eil.count();

        for (qint32 i = 0; i < nNumberOfFiles; i++) {
            QString sFN = eil.at(i).fileName();

            if ((sFN != ".") && (sFN != "..")) {
                findFiles(eil.at(i).absoluteFilePath(), pListFileNames);
            }
        }
    }
}

void XBinary::findFiles(const QString &sDirectoryName, QList<QString> *pListFileNames, bool bSubDirectories, qint32 nLevel, PDSTRUCT *pPdStruct)
{
    qint32 _nFreeIndex = 0;

    if (nLevel == 0) {
        _nFreeIndex = XBinary::getFreeIndex(pPdStruct);
        XBinary::setPdStructInit(pPdStruct, _nFreeIndex, 0);
    }

    if (!(pPdStruct->bIsStop)) {
        QFileInfo fi(sDirectoryName);

        if (fi.isFile()) {
            pListFileNames->append(fi.absoluteFilePath());
        } else if (fi.isDir() && ((bSubDirectories) || (nLevel == 0))) {
            QDir dir(sDirectoryName);

            QFileInfoList eil = dir.entryInfoList();

            qint32 nNumberOfFiles = eil.count();

            for (qint32 i = 0; (i < nNumberOfFiles) && (!(pPdStruct->bIsStop)); i++) {
                QString sFN = eil.at(i).fileName();

                if ((sFN != ".") && (sFN != "..")) {
                    findFiles(eil.at(i).absoluteFilePath(), pListFileNames, bSubDirectories, nLevel + 1, pPdStruct);
                }
            }
        }
    }

    if (nLevel == 0) {
        XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);
    }
}

QString XBinary::regExp(const QString &sRegExp, const QString &sString, qint32 nIndex)
{
    QString sResult;
#if (QT_VERSION_MAJOR < 5)
    QRegExp rxString(sRegExp);
    rxString.indexIn(sString);

    QStringList list = rxString.capturedTexts();

    if (list.count() > nIndex) {
        sResult = list.at(nIndex);
    }
#else
    QRegularExpression rxString(sRegExp);
    QRegularExpressionMatch matchString = rxString.match(sString);

    if (matchString.hasMatch()) {
        sResult = matchString.captured(nIndex);
    }
#endif

    return sResult;
}

bool XBinary::isRegExpPresent(const QString &sRegExp, const QString &sString)
{
    return (regExp(sRegExp, sString, 0) != "");
}

qint32 XBinary::getRegExpCount(const QString &sRegExp, const QString &sString)
{
    quint32 nResult = 0;
#if (QT_VERSION_MAJOR < 5)
    QRegExp rxString(sRegExp);
    rxString.indexIn(sString);
    nResult = rxString.capturedTexts().count();
#else
    QRegularExpression rxString(sRegExp);
    QRegularExpressionMatchIterator i = rxString.globalMatch(sString);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        if (match.hasMatch()) {
            nResult++;
        }
    }
#endif

    return nResult;
}

QString XBinary::getRegExpSection(const QString &sRegExp, const QString &sString, qint32 nStart, qint32 nEnd)
{
#if (QT_VERSION_MAJOR < 5)
    return sString.section(QRegExp(sRegExp), nStart, nEnd);
#else
    return sString.section(QRegularExpression(sRegExp), nStart, nEnd);
#endif
}

bool XBinary::isRegExpValid(const QString &sRegExp)
{
#if (QT_VERSION_MAJOR < 5)
    QRegExp rxString(sRegExp);
    return rxString.isValid();
#else
    QRegularExpression rxString(sRegExp);
    return rxString.isValid();
#endif
}

qint64 XBinary::read_array(qint64 nOffset, char *pBuffer, qint64 nMaxSize, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    qint64 nResult = 0;

    nResult = safeReadData(g_pDevice, nOffset, pBuffer, nMaxSize, pPdStruct);  // Check for read large files

    return nResult;
}

QByteArray XBinary::read_array(qint64 nOffset, qint64 nSize, PDSTRUCT *pPdStruct)
{
    QByteArray baResult;

    XBinary::OFFSETSIZE osRegion = convertOffsetAndSize(nOffset, nSize);

    if (osRegion.nOffset != -1) {
        baResult.resize((qint32)osRegion.nSize);

        qint64 nBytes = read_array(osRegion.nOffset, baResult.data(), osRegion.nSize, pPdStruct);

        if (osRegion.nSize != nBytes) {
            baResult.resize((qint32)nBytes);
        }
    }

    return baResult;
}

qint64 XBinary::write_array(qint64 nOffset, const char *pBuffer, qint64 nSize, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    qint64 nResult = 0;

    qint64 _nTotalSize = getSize();

    if ((nSize <= (_nTotalSize - nOffset)) && (nOffset >= 0)) {
        nResult = safeWriteData(g_pDevice, nOffset, pBuffer, nSize, pPdStruct);
    }

    return nResult;
}

qint64 XBinary::write_array(qint64 nOffset, const QByteArray &baData, PDSTRUCT *pPdStruct)
{
    return write_array(nOffset, baData.data(), baData.size(), pPdStruct);
}

QByteArray XBinary::read_array(QIODevice *pDevice, qint64 nOffset, qint64 nSize, PDSTRUCT *pPdStruct)
{
    XBinary binary(pDevice);

    return binary.read_array(nOffset, nSize, pPdStruct);
}

qint64 XBinary::read_array(QIODevice *pDevice, qint64 nOffset, char *pBuffer, qint64 nSize, PDSTRUCT *pPdStruct)
{
    XBinary binary(pDevice);

    return binary.read_array(nOffset, pBuffer, nSize, pPdStruct);
}

qint64 XBinary::write_array(QIODevice *pDevice, qint64 nOffset, char *pBuffer, qint64 nSize, PDSTRUCT *pPdStruct)
{
    XBinary binary(pDevice);

    return binary.write_array(nOffset, pBuffer, nSize, pPdStruct);
}

qint64 XBinary::write_array(QIODevice *pDevice, qint64 nOffset, const QByteArray &baData, PDSTRUCT *pPdStruct)
{
    XBinary binary(pDevice);

    return binary.write_array(nOffset, baData, pPdStruct);
}

quint8 XBinary::read_uint8(qint64 nOffset)
{
    quint8 result = 0;

    read_array(nOffset, (char *)(&result), 1);

    return result;
}

qint8 XBinary::read_int8(qint64 nOffset)
{
    quint8 result = 0;

    read_array(nOffset, (char *)(&result), 1);

    return (qint8)result;
}

quint16 XBinary::read_uint16(qint64 nOffset, bool bIsBigEndian)
{
    quint16 result = 0;

    read_array(nOffset, (char *)(&result), 2);

    if (bIsBigEndian) {
        result = qFromBigEndian(result);
    } else {
        result = qFromLittleEndian(result);
    }

    return result;
}

qint16 XBinary::read_int16(qint64 nOffset, bool bIsBigEndian)
{
    quint16 result = 0;

    read_array(nOffset, (char *)(&result), 2);

    if (bIsBigEndian) {
        result = qFromBigEndian(result);
    } else {
        result = qFromLittleEndian(result);
    }

    return (qint16)result;
}

quint32 XBinary::read_uint32(qint64 nOffset, bool bIsBigEndian)
{
    quint32 result = 0;

    read_array(nOffset, (char *)(&result), 4);

    if (bIsBigEndian) {
        result = qFromBigEndian(result);
    } else {
        result = qFromLittleEndian(result);
    }

    return result;
}

qint32 XBinary::read_int32(qint64 nOffset, bool bIsBigEndian)
{
    quint32 result = 0;

    read_array(nOffset, (char *)(&result), 4);

    if (bIsBigEndian) {
        result = qFromBigEndian(result);
    } else {
        result = qFromLittleEndian(result);
    }

    return (qint32)result;
}

quint64 XBinary::read_uint64(qint64 nOffset, bool bIsBigEndian)
{
    quint64 result = 0;

    read_array(nOffset, (char *)(&result), 8);

    if (bIsBigEndian) {
        result = qFromBigEndian(result);
    } else {
        result = qFromLittleEndian(result);
    }

    return result;
}

qint64 XBinary::read_int64(qint64 nOffset, bool bIsBigEndian)
{
    qint64 result = 0;

    read_array(nOffset, (char *)(&result), 8);

    if (bIsBigEndian) {
        result = qFromBigEndian(result);
    } else {
        result = qFromLittleEndian(result);
    }

    return (qint64)result;
}

float XBinary::read_float16(qint64 nOffset, bool bIsBigEndian)
{
    // TODO Check
    quint16 float16_value = read_uint16(nOffset, bIsBigEndian);

    // MSB -> LSB
    // float16=1bit: sign, 5bit: exponent, 10bit: fraction
    // float32=1bit: sign, 8bit: exponent, 23bit: fraction
    // for normal exponent(1 to 0x1e): value=2**(exponent-15)*(1.fraction)
    // for denormalized exponent(0): value=2**-14*(0.fraction)
    quint32 sign = float16_value >> 15;
    quint32 exponent = (float16_value >> 10) & 0x1F;
    quint32 fraction = (float16_value & 0x3FF);
    quint32 float32_value = 0;
    if (exponent == 0) {
        if (fraction == 0) {
            // zero
            float32_value = (sign << 31);
        } else {
            // can be represented as ordinary value in float32
            // 2 ** -14 * 0.0101
            // => 2 ** -16 * 1.0100
            // qint32 int_exponent = -14;
            exponent = 127 - 14;
            while ((fraction & (1 << 10)) == 0) {
                // int_exponent--;
                exponent--;
                fraction <<= 1;
            }
            fraction &= 0x3FF;
            // int_exponent += 127;
            float32_value = (sign << 31) | (exponent << 23) | (fraction << 13);
        }
    } else if (exponent == 0x1F) {
        /* Inf or NaN */
        float32_value = (sign << 31) | (0xFF << 23) | (fraction << 13);
    } else {
        /* ordinary number */
        float32_value = (sign << 31) | ((exponent + (127 - 15)) << 23) | (fraction << 13);
    }

    return *((float *)&float32_value);
}

float XBinary::read_float(qint64 nOffset, bool bIsBigEndian)
{
    float result = 0;

    read_array(nOffset, (char *)(&result), 4);

    endian_float(&result, bIsBigEndian);

    return result;
}

double XBinary::read_double(qint64 nOffset, bool bIsBigEndian)
{
    double result = 0;

    read_array(nOffset, (char *)(&result), 8);

    endian_double(&result, bIsBigEndian);

    return result;
}

quint32 XBinary::read_uint24(qint64 nOffset, bool bIsBigEndian)
{
    quint32 result = 0;

    if (bIsBigEndian) {
        read_array(nOffset, (char *)(&result) + 1, 3);
        result = qFromBigEndian(result);
    } else {
        read_array(nOffset, (char *)(&result) + 0, 3);
        result = qFromLittleEndian(result);
    }

    return (result & (0xFFFFFF));
}

qint32 XBinary::read_int24(qint64 nOffset, bool bIsBigEndian)
{
    return (qint32)(read_uint24(nOffset, bIsBigEndian));
}

qint64 XBinary::write_ansiString(qint64 nOffset, const QString &sString, qint64 nMaxSize)
{
    qint64 _nMaxSize = sString.length() + 1;
    if ((nMaxSize == -1) || (nMaxSize > _nMaxSize)) {
        nMaxSize = _nMaxSize;
    }

    return write_array(nOffset, sString.toLatin1().data(), nMaxSize);
}

void XBinary::write_ansiStringFix(qint64 nOffset, qint64 nSize, const QString &sString)
{
    QString _sString = sString;
    char *pBuffer = new char[nSize + 1];  // mb TODO Check

    _zeroMemory(pBuffer, nSize + 1);

    if (_sString.size() > nSize) {
        _sString.resize(nSize);
    }

    _copyMemory(pBuffer, _sString.toLatin1().data(), _sString.size());

    XBinary::write_array(nOffset, pBuffer, nSize);

    delete[] pBuffer;
}

qint64 XBinary::write_unicodeString(qint64 nOffset, const QString &sString, qint64 nMaxSize, bool bIsBigEndian)
{
    qint64 nResult = 0;

    if ((nMaxSize > 0) && (nMaxSize < 0x10000)) {
        qint64 nSize = qMin((qint64)sString.size(), nMaxSize);

        quint16 *pUtf16 = (quint16 *)sString.utf16();

        for (qint32 i = 0; i < nSize; i++) {
            write_uint16(nOffset + 2 * i, *(pUtf16 + i), bIsBigEndian);
            nResult++;
        }
    }

    return nResult;
}

QString XBinary::read_ansiString(qint64 nOffset, qint64 nMaxSize)
{
    QString sResult;

    if (nMaxSize > 0x10000) {
        nMaxSize = 0x10000;
    }

    if (nMaxSize > 0) {
        quint8 *pBuffer = new quint8[nMaxSize + 1];

        for (qint32 i = 0; i < nMaxSize; i++) {
            pBuffer[i] = read_uint8(nOffset + i);

            if (pBuffer[i] == 0) {
                break;
            }

            if (i == nMaxSize - 1) {
                pBuffer[nMaxSize] = 0;
            }
        }

        sResult.append((char *)pBuffer);

        delete[] pBuffer;
    }

    return sResult;
}

QString XBinary::read_unicodeString(qint64 nOffset, qint64 nMaxSize, bool bIsBigEndian)
{
    QString sResult;

    if ((nMaxSize > 0) && (nMaxSize < 0x10000)) {
        quint16 *pBuffer = new quint16[nMaxSize + 1];

        for (qint32 i = 0; i < nMaxSize; i++) {
            pBuffer[i] = read_uint16(nOffset + 2 * i, bIsBigEndian);

            if (pBuffer[i] == 0) {
                break;
            }

            if (i == nMaxSize - 1) {
                pBuffer[nMaxSize] = 0;
            }
        }

        sResult = QString::fromUtf16(pBuffer);  // TODO Check Qt6

        delete[] pBuffer;
    }

    return sResult;
}

QString XBinary::read_ucsdString(qint64 nOffset)
{
    QString sResult;

    qint32 nSize = read_uint8(nOffset);

    if (nSize > 0x10000) {
        nSize = 0x10000;
    }

    if (nSize > 0) {
        nOffset++;
        quint8 *pBuffer = new quint8[nSize + 1];

        for (qint32 i = 0; i < nSize; i++) {
            pBuffer[i] = read_uint8(nOffset + i);

            if (pBuffer[i] == 0) {
                pBuffer[i] = 0x20;  // Space
            }

            if (i == nSize - 1) {
                pBuffer[nSize] = 0;
            }
        }

        sResult.append((char *)pBuffer);

        delete[] pBuffer;
    }

    return sResult;
}

QString XBinary::read_utf8String(qint64 nOffset, qint64 nMaxSize)
{
    QString sResult;

    if (nMaxSize) {
        qint32 nRealSize = 0;

        for (qint32 i = 0; i < nMaxSize; i++) {
            quint8 nByte = read_uint8(nOffset + nRealSize);

            if (nByte == 0) {
                break;
            }

            // TODO Check !!!
            if ((nByte >> 7) & 0x1) {
                nRealSize++;
            } else if ((nByte >> 5) & 0x1) {
                nRealSize += 2;
            } else if ((nByte >> 4) & 0x1) {
                nRealSize += 3;
            } else if ((nByte >> 3) & 0x1) {
                nRealSize += 4;
            }
        }

        if (nRealSize) {
            QByteArray baString = read_array(nOffset, nRealSize);
            sResult = QString::fromUtf8(baString.data());
        }
    }

    return sResult;
}

QString XBinary::_read_utf8String(qint64 nOffset, qint64 nMaxSize)
{
    QString sResult;

    PACKED_UINT ulebSize = read_uleb128(nOffset, nMaxSize);

    sResult = read_utf8String(nOffset + ulebSize.nByteSize,
                              ulebSize.nValue);  // TODO mutf8

    return sResult;
}

QString XBinary::_read_utf8String(char *pData, qint64 nMaxSize)
{
    QString sResult;

    PACKED_UINT ulebSize = _read_uleb128(pData, nMaxSize);

    qint32 nStringSize = qMin((qint64)ulebSize.nValue, nMaxSize - ulebSize.nByteSize);

    if (nStringSize > 0) {
        sResult = QString::fromUtf8(pData + ulebSize.nByteSize, nStringSize);
    }

    return sResult;
}

QString XBinary::_read_utf8String(qint64 nOffset, char *pData, qint32 nDataSize, qint32 nDataOffset)
{
    QString sResult;

    if ((nOffset >= nDataOffset) && (nOffset < (nDataOffset + nDataSize))) {
        char *pStringData = pData + (nOffset - nDataOffset);
        qint32 nStringSize = nDataSize - (nOffset - nDataOffset);
        sResult = XBinary::_read_utf8String(pStringData, nStringSize);
    }

    return sResult;
}

QString XBinary::read_codePageString(qint64 nOffset, qint64 nMaxByteSize, const QString &sCodePage)
{
    QString sResult;

#if (QT_VERSION_MAJOR < 6) || defined(QT_CORE5COMPAT_LIB)
    QByteArray baData = read_array(nOffset, nMaxByteSize);

    QTextCodec *pCodec = QTextCodec::codecForName(sCodePage.toLatin1().data());

    if (pCodec) {
        sResult = pCodec->toUnicode(baData);
    }
#endif

    sResult = sResult.section(QChar(0xFFFD), 0, 0);
    sResult = sResult.section(QChar(0), 0, 0);

    return sResult;
}

bool XBinary::isUnicodeStringLatin(qint64 nOffset, qint64 nMaxSize, bool bIsBigEndian)
{
    bool bResult = true;

    if ((nMaxSize > 0) && (nMaxSize < 0x10000)) {
        for (qint32 i = 0; i < nMaxSize; i++) {
            quint16 nWord = read_uint16(nOffset + 2 * i, bIsBigEndian);

            if (nWord == 0) {
                break;
            } else if (nWord > 0x7F) {
                bResult = false;
                break;
            }
        }
    }

    return bResult;
}

void XBinary::write_uint8(qint64 nOffset, quint8 nValue)
{
    write_array(nOffset, (char *)(&nValue), 1);
}

void XBinary::write_int8(qint64 nOffset, qint8 nValue)
{
    quint8 _value = (quint8)nValue;
    write_array(nOffset, (char *)(&_value), 1);
}

void XBinary::write_uint16(qint64 nOffset, quint16 nValue, bool bIsBigEndian)
{
    if (bIsBigEndian) {
        nValue = qFromBigEndian(nValue);
    } else {
        nValue = qFromLittleEndian(nValue);
    }

    write_array(nOffset, (char *)(&nValue), 2);
}

void XBinary::write_int16(qint64 nOffset, qint16 nValue, bool bIsBigEndian)
{
    quint16 _value = (quint16)nValue;

    if (bIsBigEndian) {
        _value = qFromBigEndian(_value);
    } else {
        _value = qFromLittleEndian(_value);
    }

    write_array(nOffset, (char *)(&_value), 2);
}

void XBinary::write_uint32(qint64 nOffset, quint32 nValue, bool bIsBigEndian)
{
    if (bIsBigEndian) {
        nValue = qFromBigEndian(nValue);
    } else {
        nValue = qFromLittleEndian(nValue);
    }

    write_array(nOffset, (char *)(&nValue), 4);
}

void XBinary::write_int32(qint64 nOffset, qint32 nValue, bool bIsBigEndian)
{
    quint32 _value = (quint32)nValue;

    if (bIsBigEndian) {
        _value = qFromBigEndian(_value);
    } else {
        _value = qFromLittleEndian(_value);
    }

    write_array(nOffset, (char *)(&_value), 4);
}

void XBinary::write_uint64(qint64 nOffset, quint64 nValue, bool bIsBigEndian)
{
    if (bIsBigEndian) {
        nValue = qFromBigEndian(nValue);
    } else {
        nValue = qFromLittleEndian(nValue);
    }

    write_array(nOffset, (char *)(&nValue), 8);
}

void XBinary::write_int64(qint64 nOffset, qint64 nValue, bool bIsBigEndian)
{
    quint64 _value = (quint64)nValue;

    if (bIsBigEndian) {
        _value = qFromBigEndian(_value);
    } else {
        _value = qFromLittleEndian(_value);
    }

    write_array(nOffset, (char *)(&_value), 8);
}

void XBinary::write_float16(qint64 nOffset, float fValue, bool bIsBigEndian)
{
    // TODO Check
    quint32 fltInt32 = *(quint32 *)(&fValue);
    quint16 fltInt16 = 0;

    fltInt16 = (fltInt32 >> 31) << 5;
    quint16 tmp = (fltInt32 >> 23) & 0xff;
    tmp = (tmp - 0x70) & ((quint32)((qint32)(0x70 - tmp) >> 4) >> 27);
    fltInt16 = (fltInt16 | tmp) << 10;
    fltInt16 |= (fltInt32 >> 13) & 0x3ff;

    if (bIsBigEndian) {
        fltInt16 = qFromBigEndian(fltInt16);
    } else {
        fltInt16 = qFromLittleEndian(fltInt16);
    }

    write_array(nOffset, (char *)(&fltInt16), 2);
}

void XBinary::write_float(qint64 nOffset, float fValue, bool bIsBigEndian)
{
    endian_float(&fValue, bIsBigEndian);

    write_array(nOffset, (char *)(&fValue), 4);
}

void XBinary::write_double(qint64 nOffset, double dValue, bool bIsBigEndian)
{
    endian_double(&dValue, bIsBigEndian);

    write_array(nOffset, (char *)(&dValue), 8);
}

QString XBinary::read_UUID_bytes(qint64 nOffset)
{
    // TODO check!
    // TODO Check Endian
    QString sResult = QString("%1-%2-%3-%4-%5")
                          .arg(read_array(nOffset + 0, 4).toHex().data(), read_array(nOffset + 4, 2).toHex().data(), read_array(nOffset + 6, 2).toHex().data(),
                               read_array(nOffset + 8, 2).toHex().data(), read_array(nOffset + 10, 6).toHex().data());

    return sResult;
}

void XBinary::write_UUID_bytes(qint64 nOffset, const QString &sValue)
{
    QString _sValue = sValue;
    // TODO Check
    _sValue = _sValue.remove("-");

    QByteArray baUUID = QByteArray::fromHex(_sValue.toLatin1().data());

    write_array(nOffset, baUUID.data(), 16);
}

QString XBinary::read_UUID(qint64 nOffset, bool bIsBigEndian)
{
    QString sResult = QString("%1-%2-%3-%4-%5")
                          .arg(valueToHex(read_uint32(nOffset + 0, bIsBigEndian), bIsBigEndian), valueToHex(read_uint16(nOffset + 4, bIsBigEndian), bIsBigEndian),
                               valueToHex(read_uint16(nOffset + 6, bIsBigEndian), bIsBigEndian), valueToHex(read_uint16(nOffset + 8, bIsBigEndian), bIsBigEndian),
                               read_array(nOffset + 10, 6).toHex().data());

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
    quint16 result = *(quint16 *)pData;

    if (bIsBigEndian) {
        result = qFromBigEndian(result);
    } else {
        result = qFromLittleEndian(result);
    }

    return result;
}

qint16 XBinary::_read_int16(char *pData, bool bIsBigEndian)
{
    qint16 result = *(qint16 *)pData;

    if (bIsBigEndian) {
        result = qFromBigEndian(result);
    } else {
        result = qFromLittleEndian(result);
    }

    return result;
}

quint32 XBinary::_read_uint32(char *pData, bool bIsBigEndian)
{
    quint32 result = *(quint32 *)pData;

    if (bIsBigEndian) {
        result = qFromBigEndian(result);
    } else {
        result = qFromLittleEndian(result);
    }

    return result;
}

qint32 XBinary::_read_int32(char *pData, bool bIsBigEndian)
{
    qint32 result = *(qint32 *)pData;

    if (bIsBigEndian) {
        result = qFromBigEndian(result);
    } else {
        result = qFromLittleEndian(result);
    }

    return result;
}

quint64 XBinary::_read_uint64(char *pData, bool bIsBigEndian)
{
    quint64 result = *(quint64 *)pData;

    if (bIsBigEndian) {
        result = qFromBigEndian(result);
    } else {
        result = qFromLittleEndian(result);
    }

    return result;
}

qint64 XBinary::_read_int64(char *pData, bool bIsBigEndian)
{
    qint64 result = *(qint64 *)pData;

    if (bIsBigEndian) {
        result = qFromBigEndian(result);
    } else {
        result = qFromLittleEndian(result);
    }

    return result;
}

QString XBinary::_read_ansiString(char *pData, qint32 nMaxSize)
{
    QString sResult;

    QByteArray baData(pData, nMaxSize);
    sResult.append(baData.data());

    return sResult;
}

QByteArray XBinary::_read_byteArray(char *pData, qint32 nSize)
{
    return QByteArray(pData, nSize);
}

float XBinary::_read_float(char *pData, bool bIsBigEndian)
{
    float result = *(float *)pData;

    endian_float(&result, bIsBigEndian);

    return result;
}

double XBinary::_read_double(char *pData, bool bIsBigEndian)
{
    double result = *(double *)pData;

    endian_double(&result, bIsBigEndian);

    return result;
}

quint64 XBinary::_read_value(MODE mode, char *pData, bool bIsBigEndian)
{
    quint64 nResult = 0;

    if (mode == MODE::MODE_8) {
        nResult = _read_uint8(pData);
    } else if (mode == MODE::MODE_16) {
        nResult = _read_uint16(pData, bIsBigEndian);
    } else if (mode == MODE::MODE_32) {
        nResult = _read_uint32(pData, bIsBigEndian);
    } else if (mode == MODE::MODE_64) {
        nResult = _read_uint64(pData, bIsBigEndian);
    }

    return nResult;
}

quint8 XBinary::_read_uint8_safe(char *pBuffer, qint32 nBufferSize, qint32 nOffset)
{
    quint8 result = 0;

    if ((nOffset >= 0) && (nOffset < nBufferSize)) {
        result = _read_uint8(pBuffer + nOffset);
    }

    return result;
}

quint16 XBinary::_read_uint16_safe(char *pBuffer, qint32 nBufferSize, qint32 nOffset, bool bIsBigEndian)
{
    quint16 result = 0;

    if ((nOffset >= 0) && (nOffset + 1 < nBufferSize)) {
        result = _read_uint16(pBuffer + nOffset, bIsBigEndian);
    }

    return result;
}

quint32 XBinary::_read_uint32_safe(char *pBuffer, qint32 nBufferSize, qint32 nOffset, bool bIsBigEndian)
{
    quint32 result = 0;

    if ((nOffset >= 0) && (nOffset + 3 < nBufferSize)) {
        result = _read_uint32(pBuffer + nOffset, bIsBigEndian);
    }

    return result;
}

quint64 XBinary::_read_uint64_safe(char *pBuffer, qint32 nBufferSize, qint32 nOffset, bool bIsBigEndian)
{
    quint64 result = 0;

    if ((nOffset >= 0) && (nOffset + 7 < nBufferSize)) {
        result = _read_uint64(pBuffer + nOffset, bIsBigEndian);
    }

    return result;
}

QString XBinary::_read_ansiString_safe(char *pBuffer, qint32 nBufferSize, qint32 nOffset, qint32 nMaxSize)
{
    QString sResult;

    if ((nOffset >= 0) && (nOffset < nBufferSize)) {
        sResult = _read_ansiString(pBuffer + nOffset, nMaxSize);
    }

    return sResult;
}

void XBinary::_write_uint8(char *pData, quint8 nValue)
{
    *(quint8 *)pData = nValue;
}

void XBinary::_write_int8(char *pData, qint8 nValue)
{
    *(qint8 *)pData = nValue;
}

void XBinary::_write_uint16(char *pData, quint16 nValue, bool bIsBigEndian)
{
    if (bIsBigEndian) {
        nValue = qToBigEndian(nValue);
    } else {
        nValue = qToLittleEndian(nValue);
    }

    *(quint16 *)pData = nValue;
}

void XBinary::_write_int16(char *pData, qint16 nValue, bool bIsBigEndian)
{
    if (bIsBigEndian) {
        nValue = qToBigEndian(nValue);
    } else {
        nValue = qToLittleEndian(nValue);
    }

    *(qint16 *)pData = nValue;
}

void XBinary::_write_uint32(char *pData, quint32 nValue, bool bIsBigEndian)
{
    if (bIsBigEndian) {
        nValue = qToBigEndian(nValue);
    } else {
        nValue = qToLittleEndian(nValue);
    }

    *(quint32 *)pData = nValue;
}

void XBinary::_write_int32(char *pData, qint32 nValue, bool bIsBigEndian)
{
    if (bIsBigEndian) {
        nValue = qToBigEndian(nValue);
    } else {
        nValue = qToLittleEndian(nValue);
    }

    *(qint32 *)pData = nValue;
}

void XBinary::_write_uint64(char *pData, quint64 nValue, bool bIsBigEndian)
{
    if (bIsBigEndian) {
        nValue = qToBigEndian(nValue);
    } else {
        nValue = qToLittleEndian(nValue);
    }

    *(quint64 *)pData = nValue;
}

void XBinary::_write_int64(char *pData, qint64 nValue, bool bIsBigEndian)
{
    if (bIsBigEndian) {
        nValue = qToBigEndian(nValue);
    } else {
        nValue = qToLittleEndian(nValue);
    }

    *(qint64 *)pData = nValue;
}

void XBinary::_write_float(char *pData, float fValue, bool bIsBigEndian)
{
    endian_float(&fValue, bIsBigEndian);

    *(float *)pData = fValue;
}

void XBinary::_write_double(char *pData, double dValue, bool bIsBigEndian)
{
    endian_double(&dValue, bIsBigEndian);

    *(double *)pData = dValue;
}

void XBinary::_write_value(MODE mode, char *pData, quint64 nValue, bool bIsBigEndian)
{
    if (mode == MODE::MODE_8) {
        _write_uint8(pData, nValue);
    } else if (mode == MODE::MODE_16) {
        _write_uint16(pData, nValue, bIsBigEndian);
    } else if (mode == MODE::MODE_32) {
        _write_uint32(pData, nValue, bIsBigEndian);
    } else if (mode == MODE::MODE_64) {
        _write_uint64(pData, nValue, bIsBigEndian);
    }
}

quint8 XBinary::read_bcd_uint8(qint64 nOffset)
{
    quint8 nResult = 0;

    nResult = _bcd_decimal(read_uint8(nOffset));

    return nResult;
}

quint16 XBinary::read_bcd_uint16(qint64 nOffset, bool bIsBigEndian)
{
    quint16 nResult = 0;

    quint16 nValue = read_uint16(nOffset, bIsBigEndian);

    nResult = _bcd_decimal(0xFF & nValue) + _bcd_decimal(0xFF & (nValue >> 8)) * 10;

    return nResult;
}

quint16 XBinary::read_bcd_uint32(qint64 nOffset, bool bIsBigEndian)
{
    quint32 nResult = 0;

    quint32 nValue = read_uint32(nOffset, bIsBigEndian);

    nResult =
        _bcd_decimal(0xFF & nValue) + _bcd_decimal(0xFF & (nValue >> 8)) * 10 + _bcd_decimal(0xFF & (nValue >> 16)) * 100 + _bcd_decimal(0xFF & (nValue >> 24)) * 1000;

    return nResult;
}

quint16 XBinary::read_bcd_uint64(qint64 nOffset, bool bIsBigEndian)
{
    quint64 nResult = 0;

    quint64 nValue = read_uint64(nOffset, bIsBigEndian);

    nResult = _bcd_decimal(0xFF & nValue) + _bcd_decimal(0xFF & (nValue >> 8)) * 10 + _bcd_decimal(0xFF & (nValue >> 16)) * 100 +
              _bcd_decimal(0xFF & (nValue >> 24)) * 1000 + _bcd_decimal(0xFF & (nValue >> 32)) * 10000 + _bcd_decimal(0xFF & (nValue >> 40)) * 100000 +
              _bcd_decimal(0xFF & (nValue >> 48)) * 1000000 + _bcd_decimal(0xFF & (nValue >> 56)) * 10000000;

    return nResult;
}

quint8 XBinary::_bcd_decimal(quint8 nValue)
{
    quint8 nResult = 0;

    nResult = ((nValue & 0xF0) >> 4) * 10 + (nValue & 0x0F);

    return nResult;
}

qint64 XBinary::_find_array(ST st, qint64 nOffset, qint64 nSize, const char *pArray, qint64 nArraySize, PDSTRUCT *pPdStruct)
{
    qint64 nResult = -1;

    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }
    // TODO CheckSize function
    // TODO Optimize
    qint64 _nSize = getSize();

    if (nSize == -1) {
        nSize = _nSize - nOffset;
    }

    if (nSize <= 0) {
        return -1;
    }

    if (nOffset + nSize > _nSize) {
        return -1;
    }

    if (nArraySize > nSize) {
        return -1;
    }

    if (nOffset < 0) {
        return -1;
    }

    qint64 nTemp = 0;

    qint32 _nFreeIndex = XBinary::getFreeIndex(pPdStruct);
    XBinary::setPdStructInit(pPdStruct, _nFreeIndex, nSize);

    qint64 nStartOffset = nOffset;

    char *pBuffer = nullptr;

    if (!g_pConstMemory) {
        pBuffer = new char[READWRITE_BUFFER_SIZE + (nArraySize - 1)];
    }

    while ((nSize > nArraySize - 1) && (!(pPdStruct->bIsStop))) {
        nTemp = qMin((qint64)(READWRITE_BUFFER_SIZE + (nArraySize - 1)), nSize);

        if (g_pConstMemory) {
            pBuffer = (char *)g_pConstMemory + nOffset;
        } else {
            if (read_array(nOffset, pBuffer, nTemp, pPdStruct) != nTemp) {
                pPdStruct->sInfoString = tr("Read error");

                break;
            }
        }

        if (st == ST_COMPAREBYTES) {
            for (quint32 i = 0; i < nTemp - (nArraySize - 1); i++) {
                if (compareMemory(pBuffer + i, pArray, nArraySize)) {
                    nResult = nOffset + i;

                    break;
                }
            }
        } else if (st == ST_NOTNULL) {
            for (quint32 i = 0; i < nTemp - (nArraySize - 1); i++) {
                if (_isMemoryNotNull(pBuffer + i, nArraySize)) {
                    nResult = nOffset + i;

                    break;
                }
            }
        } else if (st == ST_ANSI) {
            for (quint32 i = 0; i < nTemp - (nArraySize - 1); i++) {
                if (_isMemoryAnsi(pBuffer + i, nArraySize)) {
                    nResult = nOffset + i;

                    break;
                }
            }
        } else if (st == ST_NOTANSI) {
            for (quint32 i = 0; i < nTemp - (nArraySize - 1); i++) {
                if (_isMemoryNotAnsi(pBuffer + i, nArraySize)) {
                    nResult = nOffset + i;

                    break;
                }
            }
        } else if (st == ST_NOTANSIANDNULL) {
            for (quint32 i = 0; i < nTemp - (nArraySize - 1); i++) {
                if (_isMemoryNotAnsiAndNull(pBuffer + i, nArraySize)) {
                    nResult = nOffset + i;

                    break;
                }
            }
        }

        if (nResult != -1) {
            break;
        }

        nSize -= nTemp - (nArraySize - 1);
        nOffset += nTemp - (nArraySize - 1);

        XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, nOffset - nStartOffset);
    }

    if (!g_pConstMemory) {
        delete[] pBuffer;
    }

    setPdStructFinished(pPdStruct, _nFreeIndex);

    return nResult;
}

qint64 XBinary::find_array(qint64 nOffset, qint64 nSize, const char *pArray, qint64 nArraySize, PDSTRUCT *pPdStruct)
{
    return _find_array(ST_COMPAREBYTES, nOffset, nSize, pArray, nArraySize, pPdStruct);
}

qint64 XBinary::find_byteArray(qint64 nOffset, qint64 nSize, const QByteArray &baData, PDSTRUCT *pPdStruct)
{
    return find_array(nOffset, nSize, baData.data(), baData.size(), pPdStruct);
}

qint64 XBinary::find_uint8(qint64 nOffset, qint64 nSize, quint8 nValue, PDSTRUCT *pPdStruct)
{
    return find_array(nOffset, nSize, (char *)&nValue, 1, pPdStruct);
}

qint64 XBinary::find_int8(qint64 nOffset, qint64 nSize, qint8 nValue, PDSTRUCT *pPdStruct)
{
    return find_array(nOffset, nSize, (char *)&nValue, 1, pPdStruct);
}

qint64 XBinary::find_uint16(qint64 nOffset, qint64 nSize, quint16 nValue, bool bIsBigEndian, PDSTRUCT *pPdStruct)
{
    if (bIsBigEndian) {
        nValue = qFromBigEndian(nValue);
    } else {
        nValue = qFromLittleEndian(nValue);
    }

    return find_array(nOffset, nSize, (char *)&nValue, 2, pPdStruct);
}

qint64 XBinary::find_int16(qint64 nOffset, qint64 nSize, qint16 nValue, bool bIsBigEndian, PDSTRUCT *pPdStruct)
{
    quint16 _value = (quint16)nValue;

    if (bIsBigEndian) {
        _value = qFromBigEndian(_value);
    } else {
        _value = qFromLittleEndian(_value);
    }

    return find_array(nOffset, nSize, (char *)&_value, 2, pPdStruct);
}

qint64 XBinary::find_uint32(qint64 nOffset, qint64 nSize, quint32 nValue, bool bIsBigEndian, PDSTRUCT *pPdStruct)
{
    if (bIsBigEndian) {
        nValue = qFromBigEndian(nValue);
    } else {
        nValue = qFromLittleEndian(nValue);
    }

    return find_array(nOffset, nSize, (char *)&nValue, 4, pPdStruct);
}

qint64 XBinary::find_int32(qint64 nOffset, qint64 nSize, qint32 nValue, bool bIsBigEndian, PDSTRUCT *pPdStruct)
{
    quint32 _value = (quint32)nValue;

    if (bIsBigEndian) {
        _value = qFromBigEndian(_value);
    } else {
        _value = qFromLittleEndian(_value);
    }

    return find_array(nOffset, nSize, (char *)&_value, 4, pPdStruct);
}

qint64 XBinary::find_uint64(qint64 nOffset, qint64 nSize, quint64 nValue, bool bIsBigEndian, PDSTRUCT *pPdStruct)
{
    if (bIsBigEndian) {
        nValue = qFromBigEndian(nValue);
    } else {
        nValue = qFromLittleEndian(nValue);
    }

    return find_array(nOffset, nSize, (char *)&nValue, 8, pPdStruct);
}

qint64 XBinary::find_int64(qint64 nOffset, qint64 nSize, qint64 nValue, bool bIsBigEndian, PDSTRUCT *pPdStruct)
{
    quint64 _value = (quint64)nValue;

    if (bIsBigEndian) {
        _value = qFromBigEndian(_value);
    } else {
        _value = qFromLittleEndian(_value);
    }

    return find_array(nOffset, nSize, (char *)&_value, 8, pPdStruct);
}

qint64 XBinary::find_float(qint64 nOffset, qint64 nSize, float fValue, bool bIsBigEndian, PDSTRUCT *pPdStruct)
{
    float _value = fValue;

    endian_float(&_value, bIsBigEndian);

    return find_array(nOffset, nSize, (char *)&_value, 4, pPdStruct);
}

qint64 XBinary::find_double(qint64 nOffset, qint64 nSize, double dValue, bool bIsBigEndian, PDSTRUCT *pPdStruct)
{
    double _value = dValue;

    endian_double(&_value, bIsBigEndian);

    return find_array(nOffset, nSize, (char *)&_value, 8, pPdStruct);
}

void XBinary::endian_float(float *pValue, bool bIsBigEndian)
{
    bool bReverse = false;

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
    bReverse = !bIsBigEndian;
#else
    bReverse = bIsBigEndian;
#endif

    if (bReverse) {
        qSwap(((quint8 *)pValue)[0], ((quint8 *)pValue)[3]);
        qSwap(((quint8 *)pValue)[1], ((quint8 *)pValue)[2]);
    }
}

void XBinary::endian_double(double *pValue, bool bIsBigEndian)
{
    bool bReverse = false;

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
    bReverse = !bIsBigEndian;
#else
    bReverse = bIsBigEndian;
#endif

    if (bReverse) {
        qSwap(((quint8 *)pValue)[0], ((quint8 *)pValue)[7]);
        qSwap(((quint8 *)pValue)[1], ((quint8 *)pValue)[6]);
        qSwap(((quint8 *)pValue)[2], ((quint8 *)pValue)[5]);
        qSwap(((quint8 *)pValue)[3], ((quint8 *)pValue)[4]);
    }
}

qint64 XBinary::find_ansiString(qint64 nOffset, qint64 nSize, const QString &sString, PDSTRUCT *pPdStruct)
{
    QByteArray baString = sString.toLatin1();

    return find_array(nOffset, nSize, baString.data(), baString.size(), pPdStruct);
}

qint64 XBinary::find_unicodeString(qint64 nOffset, qint64 nSize, const QString &sString, bool bIsBigEndian, PDSTRUCT *pPdStruct)
{
    // TODO convert
    QByteArray baString = getUnicodeString(sString, bIsBigEndian);

    return find_array(nOffset, nSize, (char *)baString.data(), baString.size() * 2, pPdStruct);
}

qint64 XBinary::find_utf8String(qint64 nOffset, qint64 nSize, const QString &sString, PDSTRUCT *pPdStruct)
{
    QByteArray baData = sString.toUtf8();

    return find_array(nOffset, nSize, (char *)baData.data(), baData.size(), pPdStruct);
}

qint64 XBinary::find_signature(qint64 nOffset, qint64 nSize, const QString &sSignature, qint64 *pnResultSize, PDSTRUCT *pPdStruct)
{
    _MEMORY_MAP memoryMap = XBinary::getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);

    return find_signature(&memoryMap, nOffset, nSize, sSignature, pnResultSize, pPdStruct);
}

qint64 XBinary::find_signature(_MEMORY_MAP *pMemoryMap, qint64 nOffset, qint64 nSize, const QString &sSignature, qint64 *pnResultSize, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    //    bool bDisableSignals=true;

    //    if(pPdStruct->bIsDisable) // If we call find_signature in another
    //    search function
    //    {
    //        bDisableSignals=false; // TODO Check !!!
    //    }

    // TODO CheckSize function
    qint64 _nSize = getSize();

    qint64 nResultSize = 0;

    if (pnResultSize == 0) {
        pnResultSize = &nResultSize;
    }

    if (nSize == -1) {
        nSize = _nSize - nOffset;
    }

    if (nSize <= 0) {
        return -1;
    }

    if (nOffset + nSize > _nSize) {
        return -1;
    }

    QString _sSignature = convertSignature(sSignature);

    if (_sSignature.contains(QChar('$')) || _sSignature.contains(QChar('#')) || _sSignature.contains(QChar('+'))) {
        *pnResultSize = 1;
    } else {
        // Fix size
        *pnResultSize = _sSignature.size() / 2;
    }

    qint64 nResult = -1;

    if (_sSignature.contains(QChar('.')) || _sSignature.contains(QChar('$')) || _sSignature.contains(QChar('#')) || _sSignature.contains(QChar('+')) ||
        _sSignature.contains(QChar('*')) || _sSignature.contains(QChar('%')) || _sSignature.contains(QChar('!')) || _sSignature.contains(QChar('_'))) {
        bool bIsValid = true;

        QList<SIGNATURE_RECORD> listSignatureRecords = getSignatureRecords(_sSignature, &bIsValid, pPdStruct);

        if (listSignatureRecords.count()) {
            qint32 _nFreeIndex = XBinary::getFreeIndex(pPdStruct);

            qint32 nSearchFirstIndex = 0;
            qint64 nDelta = 0;

            if (!_sSignature.contains(QChar('+'))) {
                qint32 nNumberOfRecords = listSignatureRecords.count();
                qint64 nMaxSize = 0;
                qint64 nCurrentDelta = 0;

                for (qint32 i = 0; i < nNumberOfRecords; i++) {
                    if ((listSignatureRecords.at(i).st == ST_ADDRESS) || (listSignatureRecords.at(i).st == ST_RELOFFSET)) {
                        break;
                    } else if ((listSignatureRecords.at(i).nSize > nMaxSize) && (listSignatureRecords.at(i).st == ST_COMPAREBYTES)) {
                        nMaxSize = listSignatureRecords.at(i).nSize;
                        nDelta = nCurrentDelta;
                        nSearchFirstIndex = i;
                    }
                    nCurrentDelta += listSignatureRecords.at(i).nSize;
                }
            }

            if (nSearchFirstIndex > 0) {
                qint64 nTmpOffset = nOffset + nDelta;
                qint64 nTmpSize = nSize - nDelta;

                XBinary::setPdStructInit(pPdStruct, _nFreeIndex, nTmpSize);

                QByteArray baData = listSignatureRecords.at(nSearchFirstIndex).baData;

                char *pData = baData.data();
                qint32 nDataSize = baData.size();

                for (qint64 i = 0; (i < nTmpSize) && (!(pPdStruct->bIsStop));) {
                    qint64 nCurrentOffset = _find_array(ST_COMPAREBYTES, nTmpOffset + i, nTmpSize - i, pData, nDataSize, pPdStruct);

                    if (nCurrentOffset != -1) {
                        if (_compareSignature(pMemoryMap, &listSignatureRecords, nCurrentOffset - nDelta)) {
                            nResult = nCurrentOffset - nDelta;

                            break;
                        }
                    } else {
                        break;
                    }

                    i = nCurrentOffset + 1 - nTmpOffset;

                    XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, i);
                }
            } else if ((listSignatureRecords.at(0).st == ST_COMPAREBYTES) || (listSignatureRecords.at(0).st == ST_FINDBYTES) ||
                       (listSignatureRecords.at(0).st == ST_NOTNULL) || (listSignatureRecords.at(0).st == ST_ANSI) || (listSignatureRecords.at(0).st == ST_NOTANSI) ||
                       (listSignatureRecords.at(0).st == ST_NOTANSIANDNULL)) {
                ST _st = listSignatureRecords.at(0).st;

                if (listSignatureRecords.at(0).st == ST_FINDBYTES) {
                    _st = ST_COMPAREBYTES;
                }

                XBinary::setPdStructInit(pPdStruct, _nFreeIndex, nSize);

                QByteArray baFirst = listSignatureRecords.at(0).baData;

                char *pData = baFirst.data();
                qint32 nDataSize = listSignatureRecords.at(0).nSize;

                for (qint64 i = 0; (i < nSize) && (!(pPdStruct->bIsStop));) {
                    qint64 nTempOffset = _find_array(_st, nOffset + i, nSize - i, pData, nDataSize, pPdStruct);

                    if (nTempOffset != -1) {
                        if (_compareSignature(pMemoryMap, &listSignatureRecords, nTempOffset)) {
                            nResult = nTempOffset;

                            break;
                        }
                    } else {
                        break;
                    }

                    i = nTempOffset + 1 - nOffset;

                    XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, i);
                }
            } else {
                for (qint64 i = 0; (i < nSize) && (!(pPdStruct->bIsStop)); i++) {
                    if (_compareSignature(pMemoryMap, &listSignatureRecords, nOffset + i)) {
                        nResult = nOffset + i;
                        break;
                    }

                    XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, i);
                }
            }

            XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);
        }
    } else {
        QByteArray baData = QByteArray::fromHex(QByteArray(_sSignature.toLatin1().data()));

        if (baData.size()) {
            nResult = find_array(nOffset, nSize, baData.data(), baData.size(), pPdStruct);
        }
    }

    if (pPdStruct->sInfoString != "") {
        _errorMessage(pPdStruct->sInfoString);
        pPdStruct->sInfoString = "";
    }

    return nResult;
}

qint64 XBinary::find_ansiStringI(qint64 nOffset, qint64 nSize, const QString &sString, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    qint64 nResult = -1;
    // TODO CheckSize function
    qint64 nStringSize = sString.size();
    qint64 _nSize = getSize();

    if (nSize == -1) {
        nSize = _nSize - nOffset;
    }

    if ((nSize > 0) && (nOffset + nSize <= _nSize) && (nStringSize <= nSize)) {
        qint32 _nFreeIndex = XBinary::getFreeIndex(pPdStruct);

        XBinary::setPdStructInit(pPdStruct, _nFreeIndex, nSize);

        qint64 nTemp = 0;

        char *pBuffer = new char[READWRITE_BUFFER_SIZE + (nStringSize - 1)];

        QByteArray baUpper = sString.toUpper().toLatin1();
        QByteArray baLower = sString.toLower().toLatin1();

        qint64 nStartOffset = nOffset;

        while ((nSize > nStringSize - 1) && (!(pPdStruct->bIsStop))) {
            nTemp = qMin((qint64)(READWRITE_BUFFER_SIZE + (nStringSize - 1)), nSize);

            if (read_array(nOffset, pBuffer, nTemp) != nTemp) {
                pPdStruct->sInfoString = tr("Read error");

                break;
            }

            for (quint32 i = 0; i < nTemp - (nStringSize - 1); i++) {
                if (compareMemoryByteI((quint8 *)(pBuffer + i), (quint8 *)baUpper.data(), (quint8 *)baLower.data(), nStringSize)) {
                    nResult = nOffset + i;

                    break;
                }
            }

            if (nResult != -1) {
                break;
            }

            nSize -= nTemp - (nStringSize - 1);
            nOffset += nTemp - (nStringSize - 1);

            XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, nOffset - nStartOffset);
        }

        XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);

        delete[] pBuffer;
    }

    return nResult;
}

qint64 XBinary::find_unicodeStringI(qint64 nOffset, qint64 nSize, const QString &sString, bool bIsBigEndian, PDSTRUCT *pPdStruct)
{
    qint64 nResult = -1;
    // TODO optimize
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }
    // TODO CheckSize function
    // TODO Optimize
    qint64 nStringSize = sString.size();
    qint64 _nSize = getSize();

    if (nSize == -1) {
        nSize = _nSize - nOffset;
    }

    if (nSize <= 0) {
        return -1;
    }

    if (nOffset + nSize > _nSize) {
        return -1;
    }

    if (nStringSize > nSize) {
        return -1;
    }

    qint32 _nFreeIndex = XBinary::getFreeIndex(pPdStruct);

    XBinary::setPdStructInit(pPdStruct, _nFreeIndex, nSize);

    qint64 nTemp = 0;

    char *pBuffer = new char[READWRITE_BUFFER_SIZE + 2 * (nStringSize - 1)];

    QByteArray baUpper = getUnicodeString(sString.toUpper(), bIsBigEndian);
    QByteArray baLower = getUnicodeString(sString.toLower(), bIsBigEndian);

    qint64 nStartOffset = nOffset;

    while ((nSize > 2 * (nStringSize - 1)) && (!(pPdStruct->bIsStop))) {
        nTemp = qMin((qint64)(READWRITE_BUFFER_SIZE + 2 * (nStringSize - 1)), nSize);

        if (read_array(nOffset, pBuffer, nTemp) != nTemp) {
            pPdStruct->sInfoString = tr("Read error");
            break;
        }

        for (quint32 i = 0; i < nTemp - 2 * (nStringSize - 1); i++) {
            if (compareMemoryWordI((quint16 *)(pBuffer + i), (quint16 *)baUpper.data(), (quint16 *)baLower.data(), nStringSize)) {
                nResult = nOffset + i;

                break;
            }
        }

        nSize -= nTemp - 2 * (nStringSize - 1);
        nOffset += nTemp - 2 * (nStringSize - 1);

        XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, nOffset - nStartOffset);
    }

    XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);

    delete[] pBuffer;

    return nResult;
}

qint64 XBinary::find_utf8StringI(qint64 nOffset, qint64 nSize, const QString &sString, PDSTRUCT *pPdStruct)
{
    // TODO !!!
    // Double
    return find_utf8String(nOffset, nSize, sString, pPdStruct);
}

quint8 XBinary::getBits_uint8(quint8 nValue, qint32 nBitOffset, qint32 nBitSize)
{
    quint8 nResult = nValue;

    if (nBitSize) {
        quint8 nMask = 0xFF;

        for (qint32 i = 0; i < nBitOffset; i++) {
            nResult = nResult >> 1;
        }

        for (qint32 i = 0; i < nBitSize; i++) {
            nMask = nMask << 1;
        }

        nResult = nResult & (~nMask);
    }

    return nResult;
}

quint16 XBinary::getBits_uint16(quint16 nValue, qint32 nBitOffset, qint32 nBitSize)
{
    quint16 nResult = nValue;

    if (nBitSize) {
        quint16 nMask = 0xFFFF;

        for (qint32 i = 0; i < nBitOffset; i++) {
            nResult = nResult >> 1;
        }

        for (qint32 i = 0; i < nBitSize; i++) {
            nMask = nMask << 1;
        }

        nResult = nResult & (~nMask);
    }

    return nResult;
}

quint32 XBinary::getBits_uint32(quint32 nValue, qint32 nBitOffset, qint32 nBitSize)
{
    quint32 nResult = nValue;

    if (nBitSize) {
        quint32 nMask = 0xFFFFFFFF;

        for (qint32 i = 0; i < nBitOffset; i++) {
            nResult = nResult >> 1;
        }

        for (qint32 i = 0; i < nBitSize; i++) {
            nMask = nMask << 1;
        }

        nResult = nResult & (~nMask);
    }

    return nResult;
}

quint64 XBinary::getBits_uint64(quint64 nValue, qint32 nBitOffset, qint32 nBitSize)
{
    quint64 nResult = nValue;

    if (nBitSize) {
        quint64 nMask = 0xFFFFFFFFFFFFFFFF;

        for (qint32 i = 0; i < nBitOffset; i++) {
            nResult = nResult >> 1;
        }

        for (qint32 i = 0; i < nBitSize; i++) {
            nMask = nMask << 1;
        }

        nResult = nResult & (~nMask);
    }

    return nResult;
}

bool XBinary::_addMultiSearchStringRecord(QVector<MS_RECORD> *pList, MS_RECORD *pRecord, QString sString, STRINGSEARCH_OPTIONS *pSsOptions)
{
    bool bResult = false;

    bool bAdd = true;

    if (pSsOptions->bLinks) {
        bAdd = false;

        if (sString.contains("http:") || sString.contains("www.") || sString.contains("mailto:")) {
            bAdd = true;
        }
    }

    if (pSsOptions->sMask != "") {
        bAdd = isRegExpPresent(pSsOptions->sMask, sString);
    }

    if (bAdd) {
        pList->append(*pRecord);

        bResult = true;
    }

    return bResult;
}

QVector<XBinary::MS_RECORD> XBinary::multiSearch_allStrings(_MEMORY_MAP *pMemoryMap, qint64 nOffset, qint64 nSize, STRINGSEARCH_OPTIONS ssOptions, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    OFFSETSIZE osRegion = convertOffsetAndSize(nOffset, nSize);

    nOffset = osRegion.nOffset;
    nSize = osRegion.nSize;

    QVector<XBinary::MS_RECORD> listResult;

    if (ssOptions.nMinLenght == 0) {
        ssOptions.nMinLenght = 1;
    }

    if (ssOptions.nMaxLenght == 0) {
        ssOptions.nMaxLenght = 128;  // TODO Check
    }

    // bool bANSICodec = false;

    // TODO Check Qt6
    // #if (QT_VERSION_MAJOR < 6) || defined(QT_CORE5COMPAT_LIB)
    //     QTextCodec *pCodec = nullptr;

    //     if (ssOptions.sANSICodec != "") {
    //         bANSICodec = true;
    //         pCodec = QTextCodec::codecForName(ssOptions.sANSICodec.toLatin1().data());
    //     }
    // #endif

    qint64 _nSize = nSize;
    qint64 _nOffset = nOffset;
    qint64 _nRawOffset = 0;

    bool bReadError = false;

    char *pBuffer = new char[READWRITE_BUFFER_SIZE];

    char *pAnsiBuffer = new char[ssOptions.nMaxLenght + 1];
    char *pUTF8Buffer = new char[ssOptions.nMaxLenght * 4 + 1];

    //    _zeroMemory(pUTF8Buffer,ssOptions.nMaxLenght*4);

    quint16 *pUnicodeBuffer[2] = {new quint16[ssOptions.nMaxLenght + 1], new quint16[ssOptions.nMaxLenght + 1]};
    qint64 nCurrentUnicodeSize[2] = {0, 0};
    qint64 nCurrentUnicodeOffset[2] = {0, 0};

    qint64 nCurrentAnsiSize = 0;
    qint64 nCurrentAnsiOffset = 0;

    // qint64 nCurrentUTF8Size = 0;
    // qint64 nCurrentUTF8Offset = 0;
    // qint32 nLastUTF8Width = 0;
    // qint64 nLastUTF8Offset = -1;

    bool bIsStart = true;
    char cPrevSymbol = 0;

    qint32 _nFreeIndex = XBinary::getFreeIndex(pPdStruct);

    XBinary::setPdStructInit(pPdStruct, _nFreeIndex, nSize);

    qint32 nCurrentRecords = 0;

    while ((_nSize > 0) && (!(pPdStruct->bIsStop))) {
        qint64 nCurrentSize = _nSize;

        nCurrentSize = qMin((qint64)READWRITE_BUFFER_SIZE, nCurrentSize);

        if (read_array(_nOffset, pBuffer, nCurrentSize, pPdStruct) != nCurrentSize) {
            bReadError = true;
            break;
        }

        for (qint64 i = 0; i < nCurrentSize; i++) {
            bool bIsEnd = ((i == (nCurrentSize - 1)) && (_nSize == nCurrentSize));
            qint32 nParity = (_nOffset + i) % 2;

            char cSymbol = *(pBuffer + i);

            bool bIsAnsiSymbol = false;
            // bool bIsUTF8Symbol = false;

            // bool bNewUTF8String = false;
            bool bLongString = false;

            if (ssOptions.bAnsi) {
                // bIsAnsiSymbol = isAnsiSymbol((quint8)cSymbol, bANSICodec);
                bIsAnsiSymbol = isAnsiSymbol((quint8)cSymbol);
            }

            // if (ssOptions.bUTF8) {
            //     qint32 nUTF8SymbolWidth = 0;

            //     bIsUTF8Symbol = isUTF8Symbol((quint8)cSymbol, &nUTF8SymbolWidth);

            //     if (bIsUTF8Symbol) {
            //         if (nLastUTF8Offset == -1) {
            //             if (nUTF8SymbolWidth == 0)  // Cannot start with rest
            //             {
            //                 bIsUTF8Symbol = false;
            //             } else {
            //                 nLastUTF8Offset = _nOffset + i;
            //                 nLastUTF8Width = nUTF8SymbolWidth;
            //             }
            //         } else {
            //             if (nUTF8SymbolWidth) {
            //                 if (((_nOffset + i) - nLastUTF8Offset) < nLastUTF8Width) {
            //                     bIsUTF8Symbol = false;
            //                     bNewUTF8String = true;
            //                 }

            //                 nLastUTF8Offset = _nOffset + i;
            //                 nLastUTF8Width = nUTF8SymbolWidth;
            //             }
            //         }
            //     }
            // }

            if (bIsAnsiSymbol) {
                if (nCurrentAnsiSize == 0) {
                    nCurrentAnsiOffset = _nOffset + i;
                }

                if (nCurrentAnsiSize < ssOptions.nMaxLenght) {
                    *(pAnsiBuffer + nCurrentAnsiSize) = cSymbol;
                } else {
                    bIsAnsiSymbol = false;
                    bLongString = true;
                }

                nCurrentAnsiSize++;
            }

            // if (bIsUTF8Symbol) {
            //     if (nCurrentUTF8Size == 0) {
            //         nCurrentUTF8Offset = _nOffset + i;
            //     }

            //     if (nCurrentUTF8Size < ssOptions.nMaxLenght) {
            //         *(pUTF8Buffer + nCurrentUTF8Size) = cSymbol;
            //     } else {
            //         bIsUTF8Symbol = false;
            //         bNewUTF8String = true;
            //         bLongString = true;
            //     }

            //     nCurrentUTF8Size++;
            // }

            if ((!bIsAnsiSymbol) || (bIsEnd)) {
                if (nCurrentAnsiSize >= ssOptions.nMinLenght) {
                    if (nCurrentAnsiSize - 1 < ssOptions.nMaxLenght) {
                        pAnsiBuffer[nCurrentAnsiSize] = 0;
                    } else {
                        pAnsiBuffer[ssOptions.nMaxLenght] = 0;
                    }

                    if (ssOptions.bAnsi) {
                        QString sString;

                        //                         if (!bANSICodec) {
                        //                             sString = pAnsiBuffer;
                        //                         } else {
                        //                             // TODO Check Qt6
                        //                             QByteArray baString = QByteArray(pAnsiBuffer, nCurrentAnsiSize);
                        // #if (QT_VERSION_MAJOR < 6) || defined(QT_CORE5COMPAT_LIB)
                        //                             sString = pCodec->toUnicode(baString);
                        // #else
                        //                             sString = QString::fromLatin1(baString);  // TODO
                        // #endif
                        //                         }

                        sString = pAnsiBuffer;

                        bool bAdd = true;

                        if (ssOptions.bNullTerminated && cSymbol && (!bLongString)) {
                            bAdd = false;
                        }

                        if (bAdd) {
                            MS_RECORD record = {};
                            record.valueType = VT_ANSISTRING;
                            record.nOffset = nCurrentAnsiOffset;
                            record.nSize = nCurrentAnsiSize;
                            record.nAddress = offsetToAddress(pMemoryMap, record.nOffset);
                            record.nRegionIndex = getMemoryIndexByOffset(pMemoryMap, record.nOffset);

                            if (_addMultiSearchStringRecord(&listResult, &record, sString, &ssOptions)) {
                                nCurrentRecords++;
                            }

                            if (nCurrentRecords >= ssOptions.nLimit) {
                                break;
                            }
                        }
                    }
                }

                nCurrentAnsiSize = 0;
            }

            // if ((!bIsUTF8Symbol) || (bIsEnd)) {
            //     if (nCurrentUTF8Size >= ssOptions.nMinLenght) {
            //         pUTF8Buffer[nCurrentUTF8Size] = 0;

            //         if (ssOptions.bUTF8) {
            //             QString sString = QString::fromUtf8(pUTF8Buffer, -1);

            //             qint32 nStringSize = sString.size();

            //             bool bAdd = true;

            //             if (ssOptions.bNullTerminated && cSymbol && (!bLongString)) {
            //                 bAdd = false;
            //             }

            //             if (nStringSize < ssOptions.nMinLenght) {
            //                 bAdd = false;
            //             }

            //             if ((nStringSize == nCurrentUTF8Size) && (ssOptions.bAnsi)) {
            //                 bAdd = false;
            //             }

            //             if (bAdd) {
            //                 MS_RECORD record = {};
            //                 record.recordType = MS_RECORD_TYPE_STRING_UTF8;
            //                 record.nOffset = nCurrentUTF8Offset;
            //                 record.nSize = nCurrentUTF8Size;
            //                 record.sString = sString;
            //                 record.nAddress = offsetToAddress(pMemoryMap, record.nOffset);
            //                 record.sRegion = getMemoryRecordByOffset(pMemoryMap, record.nOffset).sName;

            //                 if (_addMultiSearchStringRecord(&listResult, &record, &ssOptions)) {
            //                     nCurrentRecords++;
            //                 }

            //                 if (nCurrentRecords >= ssOptions.nLimit) {
            //                     break;
            //                 }
            //             }
            //         }
            //     }

            //     if (bNewUTF8String) {
            //         *(pUTF8Buffer) = cSymbol;
            //         nCurrentUTF8Offset = _nOffset + i;
            //         nCurrentUTF8Size = 1;
            //     } else {
            //         nCurrentUTF8Size = 0;
            //         nLastUTF8Offset = -1;
            //     }
            // }

            if (!bIsStart) {
                quint16 nCode = cPrevSymbol + (cSymbol << 8);  // TODO BE/LE

                bool bIsUnicodeSymbol = false;

                if (ssOptions.bUnicode) {
                    bIsUnicodeSymbol = isUnicodeSymbol(nCode, true);
                }

                if (nCurrentUnicodeSize[nParity] >= ssOptions.nMaxLenght) {
                    bIsUnicodeSymbol = false;
                    bLongString = true;
                }

                if (bIsUnicodeSymbol) {
                    if (nCurrentUnicodeSize[nParity] == 0) {
                        nCurrentUnicodeOffset[nParity] = _nOffset - 1 + i;
                    }

                    if (nCurrentUnicodeSize[nParity] < ssOptions.nMaxLenght) {
                        *(pUnicodeBuffer[nParity] + nCurrentUnicodeSize[nParity]) = nCode;
                    }

                    nCurrentUnicodeSize[nParity]++;
                }

                if ((!bIsUnicodeSymbol) || (bIsEnd)) {
                    if (nCurrentUnicodeSize[nParity] >= ssOptions.nMinLenght) {
                        if (nCurrentUnicodeSize[nParity] - 1 < ssOptions.nMaxLenght) {
                            pUnicodeBuffer[nParity][nCurrentUnicodeSize[nParity]] = 0;
                        } else {
                            pUnicodeBuffer[nParity][ssOptions.nMaxLenght] = 0;
                        }

                        if (ssOptions.bUnicode) {
                            QString sString = QString::fromUtf16(pUnicodeBuffer[nParity]);  // TODO Check Qt6

                            bool bAdd = true;

                            if (ssOptions.bNullTerminated && nCode && (!bLongString)) {
                                bAdd = false;
                            }

                            if (bAdd) {
                                MS_RECORD record = {};
                                record.valueType = VT_UNICODESTRING;
                                record.nOffset = nCurrentUnicodeOffset[nParity];
                                record.nSize = nCurrentUnicodeSize[nParity] * 2;
                                // record.sString = sString;
                                record.nAddress = offsetToAddress(pMemoryMap, record.nOffset);
                                record.nRegionIndex = getMemoryIndexByOffset(pMemoryMap, record.nOffset);

                                if (_addMultiSearchStringRecord(&listResult, &record, sString, &ssOptions)) {
                                    nCurrentRecords++;
                                }

                                if (nCurrentRecords >= ssOptions.nLimit) {
                                    break;
                                }
                            }
                        }
                    }

                    if (bIsEnd) {
                        qint32 nO = (nParity == 1) ? (0) : (1);

                        if (nCurrentUnicodeSize[nO] >= ssOptions.nMinLenght) {
                            if (nCurrentUnicodeSize[nO] - 1 < ssOptions.nMaxLenght) {
                                pUnicodeBuffer[nO][nCurrentUnicodeSize[nO]] = 0;
                            } else {
                                pUnicodeBuffer[nO][ssOptions.nMaxLenght] = 0;
                            }

                            if (ssOptions.bUnicode) {
                                QString sString = QString::fromUtf16(pUnicodeBuffer[nO]);  // TODO Check Qt6

                                bool bAdd = true;

                                if (bAdd) {
                                    MS_RECORD record = {};
                                    record.valueType = VT_UNICODESTRING;
                                    record.nOffset = nCurrentUnicodeOffset[nO];
                                    record.nSize = nCurrentUnicodeSize[nO] * 2;
                                    // record.sString = sString;
                                    record.nAddress = offsetToAddress(pMemoryMap, record.nOffset);
                                    record.nRegionIndex = getMemoryIndexByOffset(pMemoryMap, record.nOffset);

                                    if (_addMultiSearchStringRecord(&listResult, &record, sString, &ssOptions)) {
                                        nCurrentRecords++;
                                    }

                                    if (nCurrentRecords >= ssOptions.nLimit) {
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    nCurrentUnicodeSize[nParity] = 0;
                }
            }

            cPrevSymbol = cSymbol;

            if (bIsStart) {
                bIsStart = false;
            }
        }

        _nSize -= nCurrentSize;
        _nOffset += nCurrentSize;
        _nRawOffset += nCurrentSize;

        XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, _nOffset - nOffset);

        if (nCurrentRecords >= ssOptions.nLimit) {
            pPdStruct->sInfoString = QString("%1: %2").arg(tr("Maximum"), QString::number(nCurrentRecords));

            break;
        }
    }

    if (bReadError) {
        pPdStruct->sInfoString = tr("Read error");
    }

    delete[] pBuffer;
    delete[] pAnsiBuffer;
    delete[] pUTF8Buffer;
    delete[] pUnicodeBuffer[0];
    delete[] pUnicodeBuffer[1];

    XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);

    return listResult;
}

QVector<XBinary::MS_RECORD> XBinary::multiSearch_signature(qint64 nOffset, qint64 nSize, qint32 nLimit, const QString &sSignature, quint32 nInfo,
                                                           PDSTRUCT *pPdStruct)
{
    _MEMORY_MAP memoryMap = getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);

    return multiSearch_signature(&memoryMap, nOffset, nSize, nLimit, sSignature, nInfo, pPdStruct);
}

QVector<XBinary::MS_RECORD> XBinary::multiSearch_signature(_MEMORY_MAP *pMemoryMap, qint64 nOffset, qint64 nSize, qint32 nLimit, const QString &sSignature,
                                                           quint32 nInfo, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    if (nSize == -1) {
        nSize = getSize() - nOffset;
    }

    QVector<XBinary::MS_RECORD> listResult;

    qint64 _nSize = nSize;
    qint64 _nOffset = nOffset;

    qint32 _nFreeIndex = XBinary::getFreeIndex(pPdStruct);

    XBinary::setPdStructInit(pPdStruct, _nFreeIndex, nSize);

    qint32 nCurrentRecords = 0;

    while ((_nSize > 0) && (!(pPdStruct->bIsStop))) {
        qint64 nSignatureSize = 0;
        qint64 nSignatureOffset = find_signature(pMemoryMap, _nOffset, _nSize, sSignature, &nSignatureSize, pPdStruct);

        if (nSignatureOffset == -1) {
            break;
        }

        MS_RECORD record = {};
        record.valueType = VT_SIGNATURE;
        record.nOffset = nSignatureOffset;
        record.nSize = nSignatureSize;
        // record.sString = sSignature;
        record.nInfo = nInfo;
        record.nAddress = offsetToAddress(pMemoryMap, record.nOffset);
        record.nRegionIndex = getMemoryIndexByOffset(pMemoryMap, record.nOffset);

        listResult.append(record);

        nCurrentRecords++;

        if (nCurrentRecords >= nLimit) {
            pPdStruct->sInfoString = QString("%1: %2").arg(tr("Maximum"), QString::number(nCurrentRecords));

            break;
        }

        _nOffset = nSignatureOffset + nSignatureSize;
        _nSize = nSize - (_nOffset - nOffset);

        XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, _nOffset - nOffset);
    }

    XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);

    return listResult;
}

QVector<XBinary::MS_RECORD> XBinary::multiSearch_value(qint64 nOffset, qint64 nSize, qint32 nLimit, QVariant varValue, VT valueType, bool bIsBigEndian,
                                                       PDSTRUCT *pPdStruct)
{
    _MEMORY_MAP memoryMap = getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);

    return multiSearch_value(&memoryMap, nOffset, nSize, nLimit, varValue, valueType, bIsBigEndian, pPdStruct);
}

QVector<XBinary::MS_RECORD> XBinary::multiSearch_value(_MEMORY_MAP *pMemoryMap, qint64 nOffset, qint64 nSize, qint32 nLimit, QVariant varValue, VT valueType,
                                                       bool bIsBigEndian, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    if (nSize == -1) {
        nSize = getSize() - nOffset;
    }

    // QString sValuePrefix = valueTypeToString(valueType);
    // QString sValue = getValueString(varValue, valueType);
    qint64 nValSize = getValueSize(varValue, valueType);

    QVector<XBinary::MS_RECORD> listResult;

    qint64 _nSize = nSize;
    qint64 _nOffset = nOffset;

    qint32 _nFreeIndex = XBinary::getFreeIndex(pPdStruct);

    XBinary::setPdStructInit(pPdStruct, _nFreeIndex, nSize);

    qint32 nCurrentRecords = 0;

    while ((_nSize > 0) && (!(pPdStruct->bIsStop))) {
        qint64 nValOffset = find_value(pMemoryMap, _nOffset, _nSize, varValue, valueType, bIsBigEndian, &nValSize, pPdStruct);

        if (nValOffset == -1) {
            break;
        }

        // QString _sValue;

        MS_RECORD record = {};
        record.valueType = valueType;
        record.nOffset = nValOffset;
        record.nSize = nValSize;
        record.nAddress = offsetToAddress(pMemoryMap, record.nOffset);
        record.nRegionIndex = getMemoryIndexByOffset(pMemoryMap, record.nOffset);

        // if (valueType == VT_ANSISTRING_I) {
        //     _sValue = read_ansiString(nValOffset, nValSize);
        // } else if (valueType == VT_UNICODESTRING_I) {
        //     _sValue = read_unicodeString(nValOffset, nValSize / 2, bIsBigEndian);
        // } else if (valueType == VT_UTF8STRING_I) {
        //     _sValue = read_unicodeString(nValOffset, nValSize, bIsBigEndian);
        // } else if (valueType == VT_SIGNATURE) {
        //     _sValue = getSignature(nValOffset, nValSize);
        // } else {
        //     _sValue = sValue;
        // }

        // record.sString = QString("%1: %2").arg(sValuePrefix, _sValue);

        listResult.append(record);

        nCurrentRecords++;

        if (nCurrentRecords >= nLimit) {
            pPdStruct->sInfoString = QString("%1: %2").arg(tr("Maximum"), QString::number(nCurrentRecords));

            break;
        }

        _nOffset = nValOffset + nValSize;
        _nSize = nSize - (_nOffset - nOffset);

        XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, _nOffset - nOffset);
    }

    XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);

    return listResult;
}

qint64 XBinary::find_value(_MEMORY_MAP *pMemoryMap, qint64 nOffset, qint64 nSize, QVariant varValue, VT valueType, bool bIsBigEndian, qint64 *pnResultSize,
                           PDSTRUCT *pPdStruct)
{
    qint64 nResult = -1;

    // TODO more mb pascal strings
    if (valueType == XBinary::VT_ANSISTRING) {
        nResult = find_ansiString(nOffset, nSize, varValue.toString(), pPdStruct);
    } else if (valueType == XBinary::VT_ANSISTRING_I) {
        nResult = find_ansiStringI(nOffset, nSize, varValue.toString(), pPdStruct);
    } else if (valueType == XBinary::VT_UNICODESTRING) {
        nResult = find_unicodeString(nOffset, nSize, varValue.toString(), bIsBigEndian, pPdStruct);
    } else if (valueType == XBinary::VT_UNICODESTRING_I) {
        nResult = find_unicodeStringI(nOffset, nSize, varValue.toString(), bIsBigEndian, pPdStruct);
    } else if (valueType == XBinary::VT_UTF8STRING) {
        nResult = find_utf8String(nOffset, nSize, varValue.toString(), pPdStruct);
    } else if (valueType == XBinary::VT_UTF8STRING_I) {
        nResult = find_utf8StringI(nOffset, nSize, varValue.toString(), pPdStruct);
    } else if (valueType == XBinary::VT_SIGNATURE) {
        nResult = find_signature(pMemoryMap, nOffset, nSize, varValue.toString(), pnResultSize, pPdStruct);
    } else if (valueType == XBinary::VT_BYTE) {
        nResult = find_uint8(nOffset, nSize, (quint8)(varValue.toULongLong()), pPdStruct);
    } else if (valueType == XBinary::VT_WORD) {
        nResult = find_uint16(nOffset, nSize, (qint16)(varValue.toULongLong()), pPdStruct);
    } else if (valueType == XBinary::VT_DWORD) {
        nResult = find_uint32(nOffset, nSize, (qint32)(varValue.toULongLong()), bIsBigEndian, pPdStruct);
    } else if (valueType == XBinary::VT_QWORD) {
        nResult = find_uint64(nOffset, nSize, (qint64)(varValue.toULongLong()), bIsBigEndian, pPdStruct);
    } else if (valueType == XBinary::VT_CHAR) {
        nResult = find_int8(nOffset, nSize, (qint8)(varValue.toULongLong()), pPdStruct);
    } else if (valueType == XBinary::VT_UCHAR) {
        nResult = find_uint8(nOffset, nSize, (quint8)(varValue.toULongLong()), pPdStruct);
    } else if (valueType == XBinary::VT_SHORT) {
        nResult = find_int16(nOffset, nSize, (qint16)(varValue.toULongLong()), bIsBigEndian, pPdStruct);
    } else if (valueType == XBinary::VT_USHORT) {
        nResult = find_uint16(nOffset, nSize, (quint16)(varValue.toULongLong()), bIsBigEndian, pPdStruct);
    } else if (valueType == XBinary::VT_INT) {
        nResult = find_int32(nOffset, nSize, (qint32)(varValue.toULongLong()), bIsBigEndian, pPdStruct);
    } else if (valueType == XBinary::VT_UINT) {
        nResult = find_uint32(nOffset, nSize, (quint32)(varValue.toULongLong()), bIsBigEndian, pPdStruct);
    } else if (valueType == XBinary::VT_INT64) {
        nResult = find_int64(nOffset, nSize, (qint64)(varValue.toULongLong()), bIsBigEndian, pPdStruct);
    } else if (valueType == XBinary::VT_UINT64) {
        nResult = find_uint64(nOffset, nSize, (quint64)(varValue.toULongLong()), bIsBigEndian, pPdStruct);
    } else if (valueType == XBinary::VT_FLOAT) {
        nResult = find_float(nOffset, nSize, (float)(varValue.toFloat()), bIsBigEndian, pPdStruct);
    } else if (valueType == XBinary::VT_DOUBLE) {
        nResult = find_double(nOffset, nSize, (double)(varValue.toDouble()), bIsBigEndian, pPdStruct);
    }

    return nResult;
}


QString XBinary::read_valueString(VT valueType, qint64 nOffset, qint64 nSize, bool bIsBigEndian)
{
    QString sResult;

    nSize = qMin(nSize, qint64(128));

    if ((valueType == XBinary::VT_ANSISTRING) || (valueType == XBinary::VT_ANSISTRING_I)) {
        sResult = read_ansiString(nOffset, nSize);
    } else if ((valueType == XBinary::VT_UTF8STRING) || (valueType == XBinary::VT_UTF8STRING_I)) {
        sResult = read_utf8String(nOffset, nSize);
    } else if ((valueType == XBinary::VT_UNICODESTRING) || (valueType == XBinary::VT_UNICODESTRING_I)) {
        sResult = read_unicodeString(nOffset, nSize, bIsBigEndian);
    }

    return sResult;
}

QString XBinary::valueTypeToString(VT valueType)
{
    QString sResult;

    if ((valueType == XBinary::VT_ANSISTRING) || (valueType == XBinary::VT_ANSISTRING_I)) {
        sResult = QString("A");
    } else if ((valueType == XBinary::VT_UNICODESTRING) || (valueType == XBinary::VT_UNICODESTRING_I)) {
        sResult = QString("U");
    } else if ((valueType == XBinary::VT_UTF8STRING) || (valueType == XBinary::VT_UTF8STRING_I)) {
        sResult = QString("UTF8");
    } else if (valueType == XBinary::VT_SIGNATURE) {
        sResult = QString("S");
    } else if (valueType == XBinary::VT_BYTE) {
        sResult = QString("byte");
    } else if (valueType == XBinary::VT_WORD) {
        sResult = QString("word");
    } else if (valueType == XBinary::VT_DWORD) {
        sResult = QString("dword");
    } else if (valueType == XBinary::VT_QWORD) {
        sResult = QString("qword");
    } else if (valueType == XBinary::VT_CHAR) {
        sResult = QString("char");
    } else if (valueType == XBinary::VT_UCHAR) {
        sResult = QString("uchar");
    } else if (valueType == XBinary::VT_SHORT) {
        sResult = QString("short");
    } else if (valueType == XBinary::VT_USHORT) {
        sResult = QString("ushort");
    } else if (valueType == XBinary::VT_INT) {
        sResult = QString("int");
    } else if (valueType == XBinary::VT_UINT) {
        sResult = QString("uint");
    } else if (valueType == XBinary::VT_INT64) {
        sResult = QString("int64");
    } else if (valueType == XBinary::VT_UINT64) {
        sResult = QString("uint64");
    } else if (valueType == XBinary::VT_FLOAT) {
        sResult = QString("float");
    } else if (valueType == XBinary::VT_DOUBLE) {
        sResult = QString("double");
    }

    return sResult;
}

QString XBinary::getValueString(QVariant varValue, VT valueType)
{
    QString sResult;

    if (valueType == XBinary::VT_ANSISTRING) {
        sResult = varValue.toString();
    } else if (valueType == XBinary::VT_ANSISTRING_I) {
        sResult = varValue.toString();
    } else if (valueType == XBinary::VT_UNICODESTRING) {
        sResult = varValue.toString();
    } else if (valueType == XBinary::VT_UNICODESTRING_I) {
        sResult = varValue.toString();
    } else if (valueType == XBinary::VT_UTF8STRING) {
        sResult = varValue.toString();
    } else if (valueType == XBinary::VT_UTF8STRING_I) {
        sResult = varValue.toString();
    } else if (valueType == XBinary::VT_SIGNATURE) {
        sResult = varValue.toString();
    } else if (valueType == XBinary::VT_BYTE) {
        sResult = valueToHex((quint8)(varValue.toULongLong()));
    } else if (valueType == XBinary::VT_WORD) {
        sResult = valueToHex((quint16)(varValue.toULongLong()));
    } else if (valueType == XBinary::VT_DWORD) {
        sResult = valueToHex((quint32)(varValue.toULongLong()));
    } else if (valueType == XBinary::VT_QWORD) {
        sResult = valueToHex((quint64)(varValue.toULongLong()));
    } else if (valueType == XBinary::VT_CHAR) {
        sResult = QString("%1").arg((qint8)(varValue.toULongLong()));
    } else if (valueType == XBinary::VT_UCHAR) {
        sResult = QString("%1").arg((quint8)(varValue.toULongLong()));
    } else if (valueType == XBinary::VT_SHORT) {
        sResult = QString("%1").arg((qint16)(varValue.toULongLong()));
    } else if (valueType == XBinary::VT_USHORT) {
        sResult = QString("%1").arg((quint16)(varValue.toULongLong()));
    } else if (valueType == XBinary::VT_INT) {
        sResult = QString("%1").arg((qint32)(varValue.toULongLong()));
    } else if (valueType == XBinary::VT_UINT) {
        sResult = QString("%1").arg((quint32)(varValue.toULongLong()));
    } else if (valueType == XBinary::VT_INT64) {
        sResult = QString("%1").arg((qint64)(varValue.toULongLong()));
    } else if (valueType == XBinary::VT_UINT64) {
        sResult = QString("%1").arg((quint64)(varValue.toULongLong()));
    } else if (valueType == XBinary::VT_FLOAT) {
        sResult = QString("%1").arg(varValue.toFloat());
    } else if (valueType == XBinary::VT_DOUBLE) {
        sResult = QString("%1").arg(varValue.toDouble());
    }

    return sResult;
}

qint32 XBinary::getValueSize(QVariant varValue, VT valueType)
{
    qint32 nResult = 1;

    if (valueType == XBinary::VT_ANSISTRING) {
        nResult = varValue.toString().size();
    } else if (valueType == XBinary::VT_ANSISTRING_I) {
        nResult = varValue.toString().size();
    } else if (valueType == XBinary::VT_UNICODESTRING) {
        nResult = varValue.toString().size() * 2;
    } else if (valueType == XBinary::VT_UNICODESTRING_I) {
        nResult = varValue.toString().size() * 2;
    } else if (valueType == XBinary::VT_UTF8STRING) {
        nResult = varValue.toString().toUtf8().size();
    } else if (valueType == XBinary::VT_UTF8STRING_I) {
        nResult = varValue.toString().toUtf8().size();
    } else if (valueType == XBinary::VT_SIGNATURE) {
        QString sSignature = convertSignature(varValue.toString());
        nResult = sSignature.size() / 2;
    } else if (valueType == XBinary::VT_BYTE) {
        nResult = 1;
    } else if (valueType == XBinary::VT_WORD) {
        nResult = 2;
    } else if (valueType == XBinary::VT_DWORD) {
        nResult = 4;
    } else if (valueType == XBinary::VT_QWORD) {
        nResult = 8;
    } else if (valueType == XBinary::VT_CHAR) {
        nResult = 1;
    } else if (valueType == XBinary::VT_UCHAR) {
        nResult = 1;
    } else if (valueType == XBinary::VT_SHORT) {
        nResult = 2;
    } else if (valueType == XBinary::VT_USHORT) {
        nResult = 2;
    } else if (valueType == XBinary::VT_INT) {
        nResult = 4;
    } else if (valueType == XBinary::VT_UINT) {
        nResult = 4;
    } else if (valueType == XBinary::VT_INT64) {
        nResult = 8;
    } else if (valueType == XBinary::VT_UINT64) {
        nResult = 8;
    } else if (valueType == XBinary::VT_FLOAT) {
        nResult = 4;
    } else if (valueType == XBinary::VT_DOUBLE) {
        nResult = 8;
    }

    return nResult;
}

XBinary::VT XBinary::getValueType(quint64 nValue)
{
    XBinary::VT result = VT_QWORD;

    if (nValue <= 0xFFFFFFFF) {
        result = VT_DWORD;
    } else if (nValue <= 0xFFFF) {
        result = VT_WORD;
    } else if (nValue <= 0xFF) {
        result = VT_BYTE;
    }

    return result;
}

QByteArray XBinary::getUnicodeString(const QString &sString, bool bIsBigEndian)
{
    QByteArray baResult;

    qint32 nSize = sString.size() * 2;

    baResult.resize(nSize);

    baResult.fill(0);

    char *pData = baResult.data();

    _copyMemory(pData, (char *)sString.utf16(), nSize);

    for (qint32 i = 0; i < nSize; i++) {
        if ((i % 2) == 0) {
            quint16 nValue = *(quint16 *)(pData + i);

            if (bIsBigEndian) {
                nValue = qFromBigEndian(nValue);
            } else {
                nValue = qFromLittleEndian(nValue);
            }

            *(quint16 *)(pData + i) = nValue;
        }
    }

    return baResult;
}

QByteArray XBinary::getStringData(VT valueType, const QString &sString, bool bAddNull)
{
    QByteArray baResult;

    qint32 nSize = sString.size();

    char buffer[4] = {};

    if (valueType == VT_ANSISTRING) {
        baResult = sString.toLatin1();

        if (bAddNull) {
            baResult.append(buffer, 1);
        }
    } else if (valueType == VT_UNICODESTRING) {
        baResult.resize(nSize * 2);

        baResult.fill(0);

        QByteArray baString = getUnicodeString(sString, false);  // mb TODO Endian

        _copyMemory(baResult.data(), baString.data(), baString.size());

        if (bAddNull) {
            baResult.append(buffer, 2);
        }
    } else if (valueType == VT_UTF8STRING) {
        baResult = sString.toUtf8();

        if (bAddNull) {
            baResult.append(buffer, 1);
        }
    }

    return baResult;
}

bool XBinary::isSignaturePresent(_MEMORY_MAP *pMemoryMap, qint64 nOffset, qint64 nSize, const QString &sSignature, PDSTRUCT *pPdStruct)
{
    qint64 nResultSize = 0;

    return (find_signature(pMemoryMap, nOffset, nSize, sSignature, &nResultSize, pPdStruct) != -1);
}

bool XBinary::isSignatureValid(const QString &sSignature, PDSTRUCT *pPdStruct)
{
    bool bResult = false;

    if (sSignature.size()) {
        QString _sSignature = convertSignature(sSignature);

        bResult = true;

        QList<SIGNATURE_RECORD> listSignatureRecords = getSignatureRecords(_sSignature, &bResult, pPdStruct);
    }

    return bResult;
}

bool XBinary::createFile(const QString &sFileName, qint64 nFileSize)
{
    bool bResult = false;

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadWrite)) {
        bResult = true;

        if (nFileSize) {
            bResult = file.resize(nFileSize);
        }

        file.close();
    }

    return bResult;
}

bool XBinary::isFileExists(const QString &sFileName, bool bTryToOpen)
{
    bool bResult = false;

    QFileInfo fi(sFileName);

    bResult = ((fi.exists() && fi.isFile()));

    if (bResult && bTryToOpen) {
        QFile file(sFileName);

        bResult = tryToOpen(&file);

        file.close();
    }

    return bResult;
}

bool XBinary::removeFile(const QString &sFileName)
{
#ifdef Q_OS_WIN
    QFile::Permissions perm = QFile::permissions(sFileName);

    if (!(perm & QFile::WriteOther)) {
        QFile::setPermissions(sFileName, (QFile::Permissions)0x6666);
    }
#endif

    return QFile::remove(sFileName);
}

bool XBinary::copyFile(const QString &sSrcFileName, const QString &sDestFileName)
{
    // mb TODO remove first
    return QFile::copy(sSrcFileName, sDestFileName);
}

bool XBinary::moveFile(const QString &sSrcFileName, const QString &sDestFileName)
{
    bool bResult = false;

    if (copyFile(sSrcFileName, sDestFileName)) {
        bResult = removeFile(sSrcFileName);

        if (!bResult) {
            removeFile(sDestFileName);
        }
    }

    return bResult;
}

bool XBinary::moveFileToDirectory(const QString &sSrcFileName, const QString &sDestDirectory)
{
    QFileInfo fi(sSrcFileName);

    return moveFile(sSrcFileName, sDestDirectory + QDir::separator() + fi.fileName());
}

QString XBinary::convertFileNameSymbols(const QString &sFileName)
{
    QString sResult = sFileName;

    sResult = sResult.replace("/", "_");
    sResult = sResult.replace("\\", "_");
    sResult = sResult.replace("?", "_");
    sResult = sResult.replace("*", "_");
    sResult = sResult.replace("\"", "_");
    sResult = sResult.replace("<", "_");
    sResult = sResult.replace(">", "_");
    sResult = sResult.replace("|", "_");
    sResult = sResult.replace(":", "_");
    sResult = sResult.replace("\n", "_");
    sResult = sResult.replace("\r", "_");

    return sResult;
}

QString XBinary::getBaseFileName(const QString &sFileName)
{
    QFileInfo fi(sFileName);

    return fi.baseName();
}

bool XBinary::createDirectory(const QString &sDirectoryName)
{
    return QDir().mkpath(sDirectoryName);
}

bool XBinary::isDirectoryExists(const QString &sDirectoryName)
{
    QFileInfo fi(sDirectoryName);

    return (fi.exists() && fi.isDir());
}

bool XBinary::removeDirectory(const QString &sDirectoryName)
{
    bool bResult = false;

    QDir dir(sDirectoryName);

#if (QT_VERSION_MAJOR > 4)
    bResult = dir.removeRecursively();
#else
    bResult = dir.remove(sDirectoryName);
#endif

    return bResult;
}

bool XBinary::isDirectoryEmpty(const QString &sDirectoryName)
{
    return (QDir(sDirectoryName).entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries).count() == 0);
}

QByteArray XBinary::readFile(const QString &sFileName, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    // TODO PDSTRUCT

    QByteArray baResult;

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        baResult = file.readAll();

        file.close();
    }

    return baResult;
}

bool XBinary::readFile(const QString &sFileName, char *pBuffer, qint64 nSize, PDSTRUCT *pPdStruct)
{
    bool bResult = false;

    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        qint32 _nFreeIndex = XBinary::getFreeIndex(pPdStruct);
        XBinary::setPdStructInit(pPdStruct, _nFreeIndex, nSize);
        qint64 nOffset = 0;

        bResult = true;

        while ((nSize > 0) && !(pPdStruct->bIsStop)) {
            qint64 nCurrentSize = qMin(nSize, (qint64)READWRITE_BUFFER_SIZE);

            if (nCurrentSize != file.read(pBuffer, nCurrentSize)) {
                bResult = false;
                break;
            }

            nSize -= nCurrentSize;
            pBuffer += nCurrentSize;
            nOffset += nCurrentSize;

            XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, nOffset);
        }

        XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);

        file.close();
    }

    return bResult;
}

void XBinary::_copyMemory(char *pDest, const char *pSource, qint64 nSize)
{
    while (nSize > 0) {
        if (nSize >= 8) {
            *((quint64 *)pDest) = *((quint64 *)pSource);
            pDest += 8;
            pSource += 8;
            nSize -= 8;
        } else if (nSize >= 4) {
            *((quint32 *)pDest) = *((quint32 *)pSource);
            pDest += 4;
            pSource += 4;
            nSize -= 4;
        } else if (nSize >= 2) {
            *((quint16 *)pDest) = *((quint16 *)pSource);
            pDest += 2;
            pSource += 2;
            nSize -= 2;
        } else {
            *(pDest) = *(pSource);
            pDest++;
            pSource++;
            nSize--;
        }
    }
}

void XBinary::_zeroMemory(char *pDest, qint64 nSize)
{
    // TODO optimize
    while (nSize) {
        *pDest = 0;
        pDest++;
        nSize--;
    }
}

bool XBinary::_isMemoryZeroFilled(char *pSource, qint64 nSize)
{
    bool bResult = true;
    // TODO optimize
    while (nSize) {
        if (*pSource) {
            bResult = false;

            break;
        }

        pSource++;
        nSize--;
    }

    return bResult;
}

bool XBinary::_isMemoryNotNull(char *pSource, qint64 nSize)
{
    bool bResult = true;
    // TODO optimize
    while (nSize) {
        if ((*pSource) == 0) {
            bResult = false;

            break;
        }

        pSource++;
        nSize--;
    }

    return bResult;
}

bool XBinary::_isMemoryAnsi(char *pSource, qint64 nSize)
{
    bool bResult = true;
    // TODO optimize
    while (nSize) {
        if (!isAnsiSymbol(*pSource)) {
            bResult = false;

            break;
        }

        pSource++;
        nSize--;
    }

    return bResult;
}

bool XBinary::_isMemoryNotAnsi(char *pSource, qint64 nSize)
{
    bool bResult = true;
    // TODO optimize
    while (nSize) {
        if (isAnsiSymbol(*pSource)) {
            bResult = false;

            break;
        }

        pSource++;
        nSize--;
    }

    return bResult;
}

bool XBinary::_isMemoryNotAnsiAndNull(char *pSource, qint64 nSize)
{
    bool bResult = true;
    // TODO optimize
    while (nSize) {
        if (((*pSource) == 0) || isAnsiSymbol(*pSource)) {
            bResult = false;

            break;
        }

        pSource++;
        nSize--;
    }

    return bResult;
}

bool XBinary::copyDeviceMemory(QIODevice *pSourceDevice, qint64 nSourceOffset, QIODevice *pDestDevice, qint64 nDestOffset, qint64 nSize, quint32 nBufferSize)
{
    // TODO optimize
    if ((!pSourceDevice->seek(nSourceOffset)) || (!pDestDevice->seek(nDestOffset))) {
        return false;
    }

    char *pBuffer = new char[nBufferSize];

    while (nSize > 0) {
        qint64 nCurrentBufferSize = qMin(nSize, (qint64)nBufferSize);

        if (nCurrentBufferSize != pSourceDevice->read(pBuffer, nCurrentBufferSize)) {
            break;
        }

        if (nCurrentBufferSize != pDestDevice->write(pBuffer, nCurrentBufferSize)) {
            break;
        }

        nSize -= nCurrentBufferSize;
    }

    delete[] pBuffer;

    return (bool)(nSize == 0);
}

bool XBinary::copyMemory(qint64 nSourceOffset, qint64 nDestOffset, qint64 nSize, quint32 nBufferSize, bool bReverse)
{
    bool bResult = false;

    // TODO optimize
    if (nBufferSize == 0) {
        return false;
    }

    if (nBufferSize >= 0x1000) {
        nBufferSize = 0x1000;
    }

    if (nDestOffset == nSourceOffset) {
        return true;
    }

    qint64 nMaxSize = getSize();

    if ((nDestOffset + nSize > nMaxSize) || (nSourceOffset + nSize > nMaxSize)) {
        return false;
    }

    // TODO
    char *pBuffer = new char[nBufferSize];

    if (bReverse) {
        nSourceOffset += nSize;
        nDestOffset += nSize;
    }

    bResult = true;

    while (nSize > 0) {
        qint64 nTempSize = qMin(nSize, (qint64)nBufferSize);

        if (bReverse) {
            nSourceOffset -= nTempSize;
            nDestOffset -= nTempSize;
        }

        read_array(nSourceOffset, pBuffer, nTempSize);
        write_array(nDestOffset, pBuffer, nTempSize);

        if (!bReverse) {
            nSourceOffset += nTempSize;
            nDestOffset += nTempSize;
        }

        nSize -= nTempSize;
    }

    delete[] pBuffer;

    return bResult;
}

bool XBinary::zeroFill(qint64 nOffset, qint64 nSize)
{
    // TODO optimize
    if (nSize == 0) {
        return false;
    }

    qint64 nMaxSize = getSize();

    if (nOffset + nSize > nMaxSize) {
        return false;
    }

    quint8 cZero = 0;

    // TODO optimize with dwords
    for (qint32 i = 0; i < nSize; i++) {
        write_array(nOffset, (char *)&cZero, 1);
    }

    return true;
}

bool XBinary::compareMemory(char *pMemory1, const char *pMemory2, qint64 nSize)
{
    while (nSize > 0) {
        if (nSize >= 4) {
            if (*((quint32 *)pMemory1) != *((quint32 *)pMemory2)) {
                return false;
            }

            pMemory1 += 4;
            pMemory2 += 4;
            nSize -= 4;
        } else if (nSize >= 2) {
            if (*((quint16 *)pMemory1) != *((quint16 *)pMemory2)) {
                return false;
            }

            pMemory1 += 2;
            pMemory2 += 2;
            nSize -= 2;
        } else {
            if (*(pMemory1) != *(pMemory2)) {
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
    bool bResult = true;

    while (nSize > 0) {
        if ((*(pMemory) != *(pMemoryU)) && (*(pMemory) != *(pMemoryL))) {
            bResult = false;
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
    bool bResult = true;

    while (nSize > 0) {
        if ((*(pMemory) != *(pMemoryU)) && (*(pMemory) != *(pMemoryL))) {
            bResult = false;
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
    _MEMORY_MAP memoryMap = getMemoryMap();

    return isOffsetValid(&memoryMap, nOffset);
}

bool XBinary::isAddressValid(XADDR nAddress)
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return isAddressValid(&memoryMap, nAddress);
}

bool XBinary::isRelAddressValid(qint64 nRelAddress)
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return isRelAddressValid(&memoryMap, nRelAddress);
}

XADDR XBinary::offsetToAddress(qint64 nOffset)
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return offsetToAddress(&memoryMap, nOffset);
}

qint64 XBinary::addressToOffset(quint64 nAddress)
{
    _MEMORY_MAP memoryMap = getMemoryMap();
    return addressToOffset(&memoryMap, nAddress);
}

XADDR XBinary::offsetToRelAddress(qint64 nOffset)
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return offsetToRelAddress(&memoryMap, nOffset);
}

qint64 XBinary::relAddressToOffset(qint64 nRelAddress)
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return relAddressToOffset(&memoryMap, nRelAddress);
}

bool XBinary::isOffsetValid(XBinary::_MEMORY_MAP *pMemoryMap, qint64 nOffset)
{
    bool bResult = false;

    if (pMemoryMap->nBinarySize) {
        bResult = ((nOffset >= 0) && (nOffset < pMemoryMap->nBinarySize));
    } else {
        qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

        for (qint32 i = 0; i < nNumberOfRecords; i++) {
            if (pMemoryMap->listRecords.at(i).nSize && (pMemoryMap->listRecords.at(i).nOffset != -1)) {
                if ((pMemoryMap->listRecords.at(i).nOffset <= nOffset) && (nOffset < pMemoryMap->listRecords.at(i).nOffset + pMemoryMap->listRecords.at(i).nSize)) {
                    bResult = true;
                    break;
                }
            }
        }
    }

    return bResult;
}

bool XBinary::isOffsetAndSizeValid(XBinary::_MEMORY_MAP *pMemoryMap, XBinary::OFFSETSIZE *pOsRegion)
{
    return isOffsetAndSizeValid(pMemoryMap, pOsRegion->nOffset, pOsRegion->nSize);
}

bool XBinary::isOffsetAndSizeValid(qint64 nOffset, qint64 nSize)
{
    XBinary::_MEMORY_MAP memoryMap = getMemoryMap();

    return isOffsetAndSizeValid(&memoryMap, nOffset, nSize);
}

bool XBinary::isOffsetAndSizeValid(XBinary::_MEMORY_MAP *pMemoryMap, qint64 nOffset, qint64 nSize)
{
    bool bResult = false;

    if (nSize > 0) {
        bool bValidOffset = isOffsetValid(pMemoryMap, nOffset);
        bool bValidSize = isOffsetValid(pMemoryMap, nOffset + nSize - 1);

        bResult = bValidOffset && bValidSize;
    }

    return bResult;
}

bool XBinary::isOffsetAndSizeValid(QIODevice *pDevice, qint64 nOffset, qint64 nSize)
{
    XBinary binary(pDevice);

    _MEMORY_MAP memoryMap = binary.getMemoryMap();

    return isOffsetAndSizeValid(&memoryMap, nOffset, nSize);
}

bool XBinary::isAddressValid(XBinary::_MEMORY_MAP *pMemoryMap, XADDR nAddress)
{
    bool bResult = false;

    if (pMemoryMap->nImageSize) {
        bResult = ((pMemoryMap->nModuleAddress <= nAddress) && (nAddress < (pMemoryMap->nModuleAddress + pMemoryMap->nImageSize)));
    } else {
        qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

        for (qint32 i = 0; i < nNumberOfRecords; i++) {
            if (pMemoryMap->listRecords.at(i).nSize && (pMemoryMap->listRecords.at(i).nAddress != (XADDR)-1)) {
                if ((pMemoryMap->listRecords.at(i).nAddress <= nAddress) && (nAddress < pMemoryMap->listRecords.at(i).nAddress + pMemoryMap->listRecords.at(i).nSize)) {
                    bResult = true;
                    break;
                }
            }
        }
    }

    return bResult;
}

bool XBinary::isRelAddressValid(XBinary::_MEMORY_MAP *pMemoryMap, qint64 nRelAddress)
{
    return isAddressValid(pMemoryMap, pMemoryMap->nModuleAddress + nRelAddress);
}

bool XBinary::isAddressPhysical(XBinary::_MEMORY_MAP *pMemoryMap, XADDR nAddress)
{
    qint64 nOffset = addressToOffset(pMemoryMap, nAddress);

    return (nOffset != -1);
}

bool XBinary::isRelAddressPhysical(_MEMORY_MAP *pMemoryMap, XADDR nAddress)
{
    qint64 nOffset = relAddressToOffset(pMemoryMap, nAddress);

    return (nOffset != -1);
}

XADDR XBinary::offsetToAddress(XBinary::_MEMORY_MAP *pMemoryMap, qint64 nOffset)
{
    XADDR nResult = -1;

    qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

    //    for (qint32 i = 0; i < nNumberOfRecords; i++) {
    //        if (pMemoryMap->listRecords.at(i).nSize && (pMemoryMap->listRecords.at(i).nOffset != -1) && (pMemoryMap->listRecords.at(i).nAddress != -1)) {
    //            if ((pMemoryMap->listRecords.at(i).nOffset <= nOffset) && (nOffset < pMemoryMap->listRecords.at(i).nOffset + pMemoryMap->listRecords.at(i).nSize)) {
    //                nResult = (nOffset - pMemoryMap->listRecords.at(i).nOffset) + pMemoryMap->listRecords.at(i).nAddress;
    //                break;
    //            }
    //        }
    //    }

    // From the last to the fist

    for (qint32 i = nNumberOfRecords - 1; i >= 0; i--) {
        if (pMemoryMap->listRecords.at(i).nSize && (pMemoryMap->listRecords.at(i).nOffset != -1) && (pMemoryMap->listRecords.at(i).nAddress != (XADDR)-1)) {
            if ((pMemoryMap->listRecords.at(i).nOffset <= nOffset) && (nOffset < pMemoryMap->listRecords.at(i).nOffset + pMemoryMap->listRecords.at(i).nSize)) {
                nResult = (nOffset - pMemoryMap->listRecords.at(i).nOffset) + pMemoryMap->listRecords.at(i).nAddress;
                break;
            }
        }
    }

    return nResult;
}

qint64 XBinary::addressToOffset(XBinary::_MEMORY_MAP *pMemoryMap, XADDR nAddress)
{
    qint64 nResult = -1;

    //    if(pMemoryMap->mode==MODE_16) // Check COM Check 16SEG
    //    {
    //        if(nAddress>0xFFFF)
    //        {
    //            nAddress=((nAddress>>16)&0xFFFF)*16+(nAddress&0xFFFF);
    //        }
    //    }

    // if (pMemoryMap->fileType == FT_MSDOS) {
    //     qint64 _nResult = ((nAddress >> 16) & 0xFFFF) * 16 + (nAddress & 0xFFFF);

    //     if (_nResult >= 0x10000000) {
    //         _nResult -= 0x10000000;
    //     }

    //     if (_nResult == 0x100000) {
    //         _nResult = 0;
    //     }

    //     nResult = _nResult + pMemoryMap->nSegmentBase;

    //     if (nResult > pMemoryMap->nBinarySize) {
    //         nResult = -1;
    //     }
    // } else {
    //     qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

    //     for (qint32 i = 0; i < nNumberOfRecords; i++) {
    //         if (pMemoryMap->listRecords.at(i).nSize && (pMemoryMap->listRecords.at(i).nAddress != (XADDR)-1) && (pMemoryMap->listRecords.at(i).nOffset != -1)) {
    //             if ((pMemoryMap->listRecords.at(i).nAddress <= nAddress) && (nAddress < pMemoryMap->listRecords.at(i).nAddress + pMemoryMap->listRecords.at(i).nSize))
    //             {
    //                 nResult = (nAddress - pMemoryMap->listRecords.at(i).nAddress) + pMemoryMap->listRecords.at(i).nOffset;
    //                 break;
    //             }
    //         }
    //     }
    // }

    qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (pMemoryMap->listRecords.at(i).nSize && (pMemoryMap->listRecords.at(i).nAddress != (XADDR)-1) && (pMemoryMap->listRecords.at(i).nOffset != -1)) {
            if ((pMemoryMap->listRecords.at(i).nAddress <= nAddress) && (nAddress < pMemoryMap->listRecords.at(i).nAddress + pMemoryMap->listRecords.at(i).nSize)) {
                nResult = (nAddress - pMemoryMap->listRecords.at(i).nAddress) + pMemoryMap->listRecords.at(i).nOffset;
                break;
            }
        }
    }

    return nResult;
}

qint64 XBinary::offsetToRelAddress(XBinary::_MEMORY_MAP *pMemoryMap, qint64 nOffset)
{
    qint64 nResult = offsetToAddress(pMemoryMap, nOffset);

    if (nResult != -1) {
        nResult -= pMemoryMap->nModuleAddress;
    }

    return nResult;
}

qint64 XBinary::relAddressToOffset(XBinary::_MEMORY_MAP *pMemoryMap, qint64 nRelAddress)
{
    return addressToOffset(pMemoryMap, nRelAddress + pMemoryMap->nModuleAddress);
}

qint64 XBinary::relAddressToAddress(XBinary::_MEMORY_MAP *pMemoryMap, qint64 nRelAddress)
{
    qint64 nResult = -1;

    if (isRelAddressValid(pMemoryMap, nRelAddress)) {
        nResult = nRelAddress + pMemoryMap->nModuleAddress;
    }

    return nResult;
}

qint64 XBinary::addressToRelAddress(XBinary::_MEMORY_MAP *pMemoryMap, XADDR nAddress)
{
    qint64 nResult = -1;

    if (isAddressValid(pMemoryMap, nAddress)) {
        nResult = nAddress - pMemoryMap->nModuleAddress;
    }

    return nResult;
}

XADDR XBinary::getSegmentAddress(quint16 nSegment, quint16 nAddress)
{
    XADDR nResult = nSegment * 16 + nAddress;

    if (nResult >= 0x100000) {
        nResult -= 0x100000;
    }

    return nResult;
}

qint32 XBinary::getMemoryIndexByOffset(_MEMORY_MAP *pMemoryMap, qint64 nOffset)
{
    qint32 nResult = -1;

    qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (pMemoryMap->listRecords.at(i).nSize && (pMemoryMap->listRecords.at(i).nOffset != -1)) {
            if ((pMemoryMap->listRecords.at(i).nOffset <= nOffset) && (nOffset < pMemoryMap->listRecords.at(i).nOffset + pMemoryMap->listRecords.at(i).nSize)) {
                nResult = i;
                break;
            }
        }
    }

    return nResult;
}

XBinary::_MEMORY_RECORD XBinary::getMemoryRecordByOffset(XBinary::_MEMORY_MAP *pMemoryMap, qint64 nOffset)
{
    _MEMORY_RECORD result = {};

    qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (pMemoryMap->listRecords.at(i).nSize && (pMemoryMap->listRecords.at(i).nOffset != -1)) {
            if ((pMemoryMap->listRecords.at(i).nOffset <= nOffset) && (nOffset < pMemoryMap->listRecords.at(i).nOffset + pMemoryMap->listRecords.at(i).nSize)) {
                result = pMemoryMap->listRecords.at(i);
                break;
            }
        }
    }

    return result;
}

XBinary::_MEMORY_RECORD XBinary::getMemoryRecordByAddress(XBinary::_MEMORY_MAP *pMemoryMap, XADDR nAddress)
{
    _MEMORY_RECORD result = {};

    qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (pMemoryMap->listRecords.at(i).nSize && (pMemoryMap->listRecords.at(i).nAddress != (XADDR)-1)) {
            if ((pMemoryMap->listRecords.at(i).nAddress <= nAddress) && (nAddress < pMemoryMap->listRecords.at(i).nAddress + pMemoryMap->listRecords.at(i).nSize)) {
                result = pMemoryMap->listRecords.at(i);
                break;
            }
        }
    }

    return result;
}

XBinary::_MEMORY_RECORD XBinary::getMemoryRecordByRelAddress(XBinary::_MEMORY_MAP *pMemoryMap, qint64 nRelAddress)
{
    _MEMORY_RECORD result = {};

    XADDR nAddress = relAddressToAddress(pMemoryMap, nRelAddress);

    if (nAddress != (XADDR)-1) {
        result = getMemoryRecordByAddress(pMemoryMap, nAddress);
    }

    return result;
}

qint32 XBinary::addressToLoadSection(_MEMORY_MAP *pMemoryMap, XADDR nAddress)
{
    qint32 nResult = -1;

    _MEMORY_RECORD mm = getMemoryRecordByAddress(pMemoryMap, nAddress);

    if (mm.type == MMT_LOADSEGMENT) {
        nResult = mm.nLoadSectionNumber;
    }

    return nResult;
}

qint32 XBinary::relAddressToLoadSection(_MEMORY_MAP *pMemoryMap, qint64 nRelAddress)
{
    qint32 nResult = -1;

    _MEMORY_RECORD mm = getMemoryRecordByRelAddress(pMemoryMap, nRelAddress);

    if (mm.type == MMT_LOADSEGMENT) {
        nResult = mm.nLoadSectionNumber;
    }

    return nResult;
}

bool XBinary::isAddressInHeader(_MEMORY_MAP *pMemoryMap, XADDR nAddress)
{
    bool bResult = false;

    _MEMORY_RECORD mm = getMemoryRecordByAddress(pMemoryMap, nAddress);

    if (mm.type == MMT_HEADER) {
        bResult = true;
    }

    return bResult;
}

bool XBinary::isRelAddressInHeader(_MEMORY_MAP *pMemoryMap, qint64 nRelAddress)
{
    bool bResult = false;

    _MEMORY_RECORD mm = getMemoryRecordByRelAddress(pMemoryMap, nRelAddress);

    if (mm.type == MMT_HEADER) {
        bResult = true;
    }

    return bResult;
}

QString XBinary::getLoadSectionNameByOffset(_MEMORY_MAP *pMemoryMap, qint64 nOffset)
{
    QString sResult;

    qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (pMemoryMap->listRecords.at(i).nSize && (pMemoryMap->listRecords.at(i).nOffset != -1) && (pMemoryMap->listRecords.at(i).nAddress != (XADDR)-1)) {
            if ((pMemoryMap->listRecords.at(i).nOffset <= nOffset) && (nOffset < pMemoryMap->listRecords.at(i).nOffset + pMemoryMap->listRecords.at(i).nSize)) {
                sResult = pMemoryMap->listRecords.at(i).sName;
                break;
            }
        }
    }

    return sResult;
}

bool XBinary::isSolidAddressRange(XBinary::_MEMORY_MAP *pMemoryMap, quint64 nAddress, qint64 nSize)
{
    bool bResult = false;

    qint32 nIndex1 = getMemoryRecordByAddress(pMemoryMap, nAddress).nIndex;
    qint32 nIndex2 = getMemoryRecordByAddress(pMemoryMap, nAddress + nSize - 1).nIndex;

    bResult = (nIndex1 == nIndex2);

    return bResult;
}

QString XBinary::getMemoryRecordInfoByOffset(qint64 nOffset)
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return getMemoryRecordInfoByOffset(&memoryMap, nOffset);
}

QString XBinary::getMemoryRecordInfoByAddress(XADDR nAddress)
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return getMemoryRecordInfoByAddress(&memoryMap, nAddress);
}

QString XBinary::getMemoryRecordInfoByRelAddress(qint64 nRelAddress)
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return getMemoryRecordInfoByRelAddress(&memoryMap, nRelAddress);
}

QString XBinary::getMemoryRecordInfoByOffset(XBinary::_MEMORY_MAP *pMemoryMap, qint64 nOffset)
{
    XBinary::_MEMORY_RECORD memoryRecord = getMemoryRecordByOffset(pMemoryMap, nOffset);

    return getMemoryRecordName(&memoryRecord);
}

QString XBinary::getMemoryRecordInfoByAddress(XBinary::_MEMORY_MAP *pMemoryMap, XADDR nAddress)
{
    XBinary::_MEMORY_RECORD memoryRecord = getMemoryRecordByAddress(pMemoryMap, nAddress);

    return getMemoryRecordName(&memoryRecord);
}

QString XBinary::getMemoryRecordInfoByRelAddress(XBinary::_MEMORY_MAP *pMemoryMap, qint64 nRelAddress)
{
    XBinary::_MEMORY_RECORD memoryRecord = getMemoryRecordByAddress(pMemoryMap, nRelAddress + pMemoryMap->nModuleAddress);

    return getMemoryRecordName(&memoryRecord);
}

QString XBinary::getMemoryRecordName(XBinary::_MEMORY_RECORD *pMemoryRecord)
{
    QString sRecord;

    sRecord = pMemoryRecord->sName;  // TODO

    return sRecord;
}

QString XBinary::mapModeToString(MAPMODE mapMode)
{
    QString sResult = tr("Unknown");

    switch (mapMode) {
        case MAPMODE_UNKNOWN: sResult = tr("Unknown"); break;
        case MAPMODE_REGIONS: sResult = tr("Regions"); break;
        case MAPMODE_SEGMENTS: sResult = tr("Segments"); break;
        case MAPMODE_SECTIONS: sResult = tr("Sections"); break;
        case MAPMODE_OBJECTS: sResult = tr("Objects"); break;
        case MAPMODE_MAPS: sResult = tr("Maps"); break;
    }

    return sResult;
}

QList<XBinary::MAPMODE> XBinary::getMapModesList()
{
    QList<MAPMODE> listResult;

    listResult.append(MAPMODE_REGIONS);

    return listResult;
}

XBinary::_MEMORY_MAP XBinary::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(mapMode)

    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    _MEMORY_MAP result = {};

    qint64 nTotalSize = getSize();

    result.nModuleAddress = getModuleAddress();
    result.nBinarySize = nTotalSize;
    result.nImageSize = nTotalSize;
    result.fileType = getFileType();
    result.mode = getMode();
    result.sArch = getArch();
    result.endian = getEndian();
    result.sType = getTypeAsString();

    _MEMORY_RECORD record = {};
    record.nAddress = result.nModuleAddress;
    record.segment = ADDRESS_SEGMENT_FLAT;
    record.nOffset = 0;
    record.nSize = nTotalSize;
    record.nIndex = 0;
    record.sName = tr("Data");

    result.listRecords.append(record);

    return result;
}

qint32 XBinary::getNumberOfPhysicalRecords(XBinary::_MEMORY_MAP *pMemoryMap)
{
    qint32 nResult = 0;

    qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (!pMemoryMap->listRecords.at(i).bIsVirtual) {
            nResult++;
        }
    }

    return nResult;
}

qint32 XBinary::getNumberOfVirtualRecords(_MEMORY_MAP *pMemoryMap)
{
    qint32 nResult = 0;

    qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (pMemoryMap->listRecords.at(i).bIsVirtual) {
            nResult++;
        }
    }

    return nResult;
}

qint64 XBinary::getRecordsTotalRowSize(_MEMORY_MAP *pMemoryMap)
{
    qint64 nResult = 0;

    qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (!pMemoryMap->listRecords.at(i).bIsVirtual) {
            nResult += pMemoryMap->listRecords.at(i).nSize;
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
    this->g_nBaseAddress = nBaseAddress;
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
    g_bIsImage = bValue;
}

void XBinary::setMultiSearchCallbackState(bool bState)
{
    g_bMultiSearchCallback = bState;
}

bool XBinary::compareSignature(const QString &sSignature, qint64 nOffset)
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return compareSignature(&memoryMap, sSignature, nOffset);
}

bool XBinary::compareSignature(_MEMORY_MAP *pMemoryMap, const QString &sSignature, qint64 nOffset, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    bool bResult = false;

    QString sOrigin = sSignature;

    QString _sSignature = convertSignature(sSignature);

    bool bValid = true;

    QList<SIGNATURE_RECORD> listSignatureRecords = getSignatureRecords(_sSignature, &bValid, pPdStruct);

    if (listSignatureRecords.count()) {
        bResult = _compareSignature(pMemoryMap, &listSignatureRecords, nOffset);
    } else {
        _errorMessage(QString("%1: %2").arg(tr("Invalid signature"), sOrigin));
    }

    return bResult;
}

bool XBinary::_compareByteArrayWithSignature(const QByteArray &baData, const QString &sSignature)
{
    bool bResult = false;

    QString sHex = baData.toHex().data();

    if (sHex.size() == sSignature.size()) {
        bResult = true;

        qint32 nNumberOfSymbols = sSignature.size();

        for (qint32 i = 0; i < nNumberOfSymbols; i++) {
            if (sSignature.at(i) != QChar('.')) {
                if (sSignature.at(i) != sHex.at(i)) {
                    bResult = false;
                    break;
                }
            }
        }
    }

    return bResult;
}

QString XBinary::_createSignature(const QString &sSignature1, const QString &sSignature2)
{
    QString sResult;

    qint32 nSize = sSignature1.size();

    if (sSignature1.size() == sSignature2.size()) {
        for (qint32 i = 0; i < nSize; i += 2) {
            if (sSignature1.mid(i, 2) == sSignature2.mid(i, 2)) {
                sResult.append(sSignature1.mid(i, 2));
            } else {
                sResult.append("..");
            }
        }
    }

    return sResult;
}

bool XBinary::compareSignatureOnAddress(const QString &sSignature, XADDR nAddress)
{
    XBinary::_MEMORY_MAP memoryMap = getMemoryMap();

    return compareSignatureOnAddress(&memoryMap, sSignature, nAddress);
}

bool XBinary::compareSignatureOnAddress(XBinary::_MEMORY_MAP *pMemoryMap, const QString &sSignature, XADDR nAddress)
{
    bool bResult = false;

    qint64 nOffset = addressToOffset(pMemoryMap, nAddress);

    if (nOffset != -1) {
        bResult = compareSignature(pMemoryMap, sSignature, nOffset);
    }

    return bResult;
}

qint64 XBinary::_getEntryPointOffset()
{
    XBinary::_MEMORY_MAP memoryMap = getMemoryMap();

    return getEntryPointOffset(&memoryMap);
}

qint64 XBinary::getEntryPointOffset(_MEMORY_MAP *pMemoryMap)
{
    return addressToOffset(pMemoryMap, pMemoryMap->nEntryPointAddress);
}

void XBinary::setEntryPointOffset(qint64 nEntryPointOffset)
{
    this->g_nEntryPointOffset = nEntryPointOffset;
}

XADDR XBinary::getEntryPointAddress()
{
    XBinary::_MEMORY_MAP memoryMap = getMemoryMap();

    return getEntryPointAddress(&memoryMap);
}

XADDR XBinary::getEntryPointAddress(XBinary::_MEMORY_MAP *pMemoryMap)
{
    return pMemoryMap->nEntryPointAddress;
}

qint64 XBinary::getEntryPointRVA()
{
    XBinary::_MEMORY_MAP memoryMap = getMemoryMap();

    return getEntryPointRVA(&memoryMap);
}

qint64 XBinary::getEntryPointRVA(_MEMORY_MAP *pMemoryMap)
{
    return (pMemoryMap->nEntryPointAddress) - pMemoryMap->nModuleAddress;
}

XADDR XBinary::getLowestAddress(XBinary::_MEMORY_MAP *pMemoryMap)
{
    XADDR nResult = -1;

    qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (pMemoryMap->listRecords.at(i).nAddress != (XADDR)-1) {
            if (nResult == (XADDR)-1) {
                nResult = pMemoryMap->listRecords.at(i).nAddress;
            }

            nResult = qMin(pMemoryMap->listRecords.at(i).nAddress, nResult);
        }
    }

    return nResult;
}

qint64 XBinary::getTotalVirtualSize(XBinary::_MEMORY_MAP *pMemoryMap)
{
    qint64 nResult = 0;

    qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (pMemoryMap->listRecords.at(i).type != MMT_OVERLAY)  // TODO Check ELF, MachO -1
        {
            nResult += pMemoryMap->listRecords.at(i).nSize;
        }
    }

    return nResult;
}

quint64 XBinary::positionToVirtualAddress(_MEMORY_MAP *pMemoryMap, qint64 nPosition)
{
    XADDR nResult = -1;

    qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

    qint64 _nSize = 0;

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if ((_nSize <= nPosition) && (nPosition < _nSize + pMemoryMap->listRecords.at(i).nSize)) {
            nResult = pMemoryMap->listRecords.at(i).nAddress + (nPosition - _nSize);
        }

        _nSize += pMemoryMap->listRecords.at(i).nSize;
    }

    return nResult;
}

void XBinary::setModuleAddress(quint64 nValue)
{
    this->g_nModuleAddress = nValue;
}

XADDR XBinary::getModuleAddress()
{
    XADDR nResult = 0;

    if (g_nModuleAddress != (XADDR)-1) {
        nResult = g_nModuleAddress;
    } else {
        nResult = getBaseAddress();
    }

    return nResult;
}

bool XBinary::compareEntryPoint(const QString &sSignature, qint64 nOffset)
{
    XBinary::_MEMORY_MAP memoryMap = getMemoryMap();

    return compareEntryPoint(&memoryMap, sSignature, nOffset);
}

bool XBinary::compareEntryPoint(XBinary::_MEMORY_MAP *pMemoryMap, const QString &sSignature, qint64 nOffset)
{
    qint64 nEPOffset = getEntryPointOffset(pMemoryMap) + nOffset;

    return compareSignature(pMemoryMap, sSignature, nEPOffset);
}

bool XBinary::moveMemory(qint64 nSourceOffset, qint64 nDestOffset, qint64 nSize)
{
    bool bResult = false;

    if (nDestOffset == nSourceOffset) {
        return true;
    }

    qint64 nMaxSize = getSize();

    if ((nDestOffset + nSize > nMaxSize) || (nSourceOffset + nSize > nMaxSize)) {
        return false;
    }

    qint64 nDelta = nDestOffset - nSourceOffset;

    if (nDelta > 0) {
        bResult = copyMemory(nSourceOffset, nDestOffset, nSize, nDelta, true);
        zeroFill(nSourceOffset, nDelta);
    } else {
        bResult = copyMemory(nSourceOffset, nDestOffset, nSize, -nDelta, false);
        zeroFill(nDestOffset + nSize, -nDelta);
    }

    return bResult;
}

bool XBinary::moveMemory(QIODevice *pDevice, qint64 nSourceOffset, qint64 nDestOffset, qint64 nSize)
{
    XBinary binary(pDevice);

    return binary.moveMemory(nSourceOffset, nDestOffset, nSize);
}

bool XBinary::dumpToFile(const QString &sFileName, const char *pData, qint64 nDataSize)
{
    bool bResult = false;

    QFile file;
    file.setFileName(sFileName);
    file.resize(0);

    if (file.open(QIODevice::ReadWrite)) {
        file.write(pData, nDataSize);
        file.close();
        bResult = true;
    }

    return bResult;
}

bool XBinary::dumpToFile(const QString &sFileName, qint64 nDataOffset, qint64 nDataSize, PDSTRUCT *pPdStruct)
{
    bool bResult = false;

    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    if (nDataSize == -1) {
        qint64 _nSize = getSize();
        nDataSize = _nSize - nDataOffset;
    }

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadWrite)) {
        file.resize(0);

        char *pBuffer = new char[0x1000];  // TODO const

        qint64 nSourceOffset = nDataOffset;
        qint64 nDestOffset = 0;

        qint32 _nFreeIndex = XBinary::getFreeIndex(pPdStruct);

        XBinary::setPdStructInit(pPdStruct, _nFreeIndex, nDataSize);

        bResult = true;

        while ((nDataSize > 0) && (!(pPdStruct->bIsStop))) {
            qint64 nTempSize = qMin(nDataSize, (qint64)0x1000);  // TODO const

            if (read_array(nSourceOffset, pBuffer, nTempSize, pPdStruct) != nTempSize) {
                pPdStruct->sInfoString = tr("Read error");
                bResult = false;
                break;
            }

            if (!((file.seek(nDestOffset)) && (file.write(pBuffer, nTempSize) == nTempSize))) {
                pPdStruct->sInfoString = tr("Write error");
                bResult = false;
                break;
            }

            nSourceOffset += nTempSize;
            nDestOffset += nTempSize;

            nDataSize -= nTempSize;

            XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, nDestOffset);
        }

        XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);

        delete[] pBuffer;

        file.close();
    } else {
        _errorMessage(QString("%1: %2").arg(QObject::tr("Cannot open file"), sFileName));
    }

    return bResult;
}

bool XBinary::patchFromFile(const QString &sFileName, qint64 nDataOffset, qint64 nDataSize, PDSTRUCT *pPdStruct)
{
    bool bResult = false;

    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    if (nDataSize == -1) {
        qint64 _nSize = getSize();
        nDataSize = _nSize - nDataOffset;
    }

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        char *pBuffer = new char[0x1000];  // TODO const

        qint64 nSourceOffset = 0;
        qint64 nDestOffset = nDataOffset;

        qint32 _nFreeIndex = XBinary::getFreeIndex(pPdStruct);

        XBinary::setPdStructInit(pPdStruct, _nFreeIndex, nDataSize);

        bResult = true;

        while ((nDataSize > 0) && (!(pPdStruct->bIsStop))) {
            qint64 nTempSize = qMin(nDataSize, (qint64)0x1000);  // TODO const

            if (safeReadData(&file, nSourceOffset, pBuffer, nTempSize, pPdStruct) != nTempSize) {
                pPdStruct->sInfoString = tr("Read error");
                bResult = false;
                break;
            }

            if (safeWriteData(g_pDevice, nDestOffset, pBuffer, nTempSize, pPdStruct) != nTempSize) {
                pPdStruct->sInfoString = tr("Write error");
                bResult = false;
                break;
            }

            nSourceOffset += nTempSize;
            nDestOffset += nTempSize;

            nDataSize -= nTempSize;

            XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, nDestOffset);
        }

        XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);

        delete[] pBuffer;

        file.close();
    } else {
        _errorMessage(QString("%1: %2").arg(QObject::tr("Cannot open file"), sFileName));
    }

    return bResult;
}

QSet<XBinary::FT> XBinary::getFileTypes(bool bExtra)
{
    QSet<XBinary::FT> stResult;

    stResult.insert(FT_BINARY);

    QByteArray baHeader;
    QByteArray baNewHeader;
    baHeader = read_array(0, qMin(getSize(), (qint64)0x200));  // TODO const
    char *pOffset = baHeader.data();
    qint64 nSize = getSize();
    bool bAllFound = false;

    if (nSize >= (qint64)sizeof(XMSDOS_DEF::IMAGE_DOS_HEADEREX)) {
        if ((_read_uint16(pOffset) == XMSDOS_DEF::S_IMAGE_DOS_SIGNATURE_MZ) || (_read_uint16(pOffset) == XMSDOS_DEF::S_IMAGE_DOS_SIGNATURE_ZM)) {
            stResult.insert(FT_MSDOS);
            // TODO rewrite for NE, LE
            quint32 nLfanew = _read_uint32(pOffset + offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_lfanew));
            quint32 nHeaderSize = (quint32)baHeader.size() - sizeof(XPE_DEF::IMAGE_NT_HEADERS32);

            bool bIsNewHeaderValid = false;

            if ((nLfanew < nHeaderSize) && ((quint32)baHeader.size() > (nLfanew + sizeof(XPE_DEF::IMAGE_NT_HEADERS32))))  // TODO do not use
                                                                                                                          // IMAGE_NT_HEADERS32
            {
                pOffset += nLfanew;
                bIsNewHeaderValid = true;
            } else {
                qint64 nNtHeadersSize = 4 + sizeof(XPE_DEF::IMAGE_FILE_HEADER);

                baNewHeader = read_array(nLfanew, nNtHeadersSize);

                nHeaderSize = baNewHeader.size();

                if (nHeaderSize == nNtHeadersSize) {
                    pOffset = baNewHeader.data();
                    bIsNewHeaderValid = true;
                }
            }

            if (bIsNewHeaderValid) {
                bIsNewHeaderValid = false;

                if (_read_uint32(pOffset) == XPE_DEF::S_IMAGE_NT_SIGNATURE) {
                    stResult.insert(FT_PE);

                    quint16 nMachine = _read_uint32(pOffset + 4 + offsetof(XPE_DEF::IMAGE_FILE_HEADER, Machine));

                    // TODO more
                    if ((nMachine == XPE_DEF::S_IMAGE_FILE_MACHINE_AMD64) || (nMachine == XPE_DEF::S_IMAGE_FILE_MACHINE_IA64) ||
                        (nMachine == XPE_DEF::S_IMAGE_FILE_MACHINE_ARM64) || (nMachine == XPE_DEF::S_IMAGE_FILE_MACHINE_ALPHA64) ||
                        (nMachine == XPE_DEF::S_IMAGE_FILE_MACHINE_RISCV64) || (nMachine == XPE_DEF::S_IMAGE_FILE_MACHINE_LOONGARCH64)) {
                        stResult.insert(FT_PE64);
                    } else {
                        stResult.insert(FT_PE32);
                    }
                    bIsNewHeaderValid = true;
                } else if (_read_uint16(pOffset) == XNE_DEF::S_IMAGE_OS2_SIGNATURE) {
                    stResult.insert(FT_NE);
                    bIsNewHeaderValid = true;
                } else if (_read_uint16(pOffset) == XLE_DEF::S_IMAGE_VXD_SIGNATURE) {
                    stResult.insert(FT_LE);
                    bIsNewHeaderValid = true;
                } else if (_read_uint16(pOffset) == XLE_DEF::S_IMAGE_LX_SIGNATURE) {
                    stResult.insert(FT_LX);
                    bIsNewHeaderValid = true;
                }
            }

            if (!bIsNewHeaderValid) {
                quint16 nCP = read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_cp));
                quint16 nCblp = read_uint16(offsetof(XMSDOS_DEF::IMAGE_DOS_HEADER, e_cblp));

                if (nCP > 0) {
                    qint64 nSignatureOffset = (nCP - 1) * 512 + nCblp;
                    if (nSize - nSignatureOffset) {
                        bool bBW = false;
                        bool b16M = false;
                        bool b4G = false;
                        while (true) {
                            quint16 nSignature = read_uint16(nSignatureOffset);

                            if (nSignature == 0x5742) {  // BW
                                bBW = true;
                                b16M = true;
                                nSignatureOffset = read_uint32(nSignatureOffset + offsetof(XMSDOS_DEF::dos16m_exe_header, next_header_pos));
                            } else if (nSignature == 0x464D) {  // MF - find info
                                nSignatureOffset += read_uint32(nSignatureOffset + 2);
                            } else if (nSignature == 0x5A4D) {  // MZ
                                qint64 nSignatureOffsetOpt = read_uint32(nSignatureOffset + offsetof(XMSDOS_DEF::IMAGE_DOS_HEADEREX, e_lfanew));
                                quint16 nSignatureOpt = read_uint16(nSignatureOffsetOpt + nSignatureOffset);

                                if (nSignatureOpt == 0x454E) {  // NE
                                    b16M = true;
                                } else if (nSignatureOpt == 0x454C) {  // LE
                                    b4G = true;
                                } else if (nSignatureOpt == 0x584C) {  // LX
                                    b4G = true;
                                }
                                break;
                            } else {
                                break;
                            }
                        }

                        if (bBW && b4G) {
                            stResult.insert(FT_DOS4G);
                        } else if (bBW && b16M) {
                            stResult.insert(FT_DOS16M);
                        }
                    }
                }
            }

            bAllFound = true;
        }
    }

    if ((!bAllFound) && (nSize >= (qint64)sizeof(XELF_DEF::Elf32_Ehdr))) {
        if ((((XELF_DEF::Elf32_Ehdr *)pOffset)->e_ident[0] == 0x7f) && (((XELF_DEF::Elf32_Ehdr *)pOffset)->e_ident[1] == 'E') &&
            (((XELF_DEF::Elf32_Ehdr *)pOffset)->e_ident[2] == 'L') && (((XELF_DEF::Elf32_Ehdr *)pOffset)->e_ident[3] == 'F')) {
            stResult.insert(FT_ELF);

            if (((XELF_DEF::Elf32_Ehdr *)pOffset)->e_ident[4] == 1) {
                stResult.insert(FT_ELF32);
            } else if (((XELF_DEF::Elf32_Ehdr *)pOffset)->e_ident[4] == 2) {
                stResult.insert(FT_ELF64);
            }
            // mb TODO another e_ident[4]
            bAllFound = true;
        }
    }

    if ((!bAllFound) && (nSize >= (qint64)sizeof(XMACH_DEF::mach_header))) {
        if ((_read_uint32(pOffset) == XMACH_DEF::S_MH_MAGIC) || (_read_uint32(pOffset) == XMACH_DEF::S_MH_CIGAM)) {
            stResult.insert(FT_MACHO);
            stResult.insert(FT_MACHO32);

            bAllFound = true;
        } else if ((_read_uint32(pOffset) == XMACH_DEF::S_MH_MAGIC_64) || (_read_uint32(pOffset) == XMACH_DEF::S_MH_CIGAM_64)) {
            stResult.insert(FT_MACHO);
            stResult.insert(FT_MACHO64);

            bAllFound = true;
        }
    }

    if ((!bAllFound) && (nSize >= 8)) {
        quint32 nMagic = _read_uint32(pOffset);
        if ((nMagic == 0xf3030000) || (nMagic == 0xe7030000)) {
            stResult.insert(FT_AMIGAHUNK);

            bAllFound = true;
        }
    }

    if ((!bAllFound) && bExtra) {
        _MEMORY_MAP memoryMap = XBinary::getMemoryMap();
        UNICODE_TYPE unicodeType = getUnicodeType(&baHeader);

        bAllFound = true;

        if (compareSignature(&memoryMap, "'PK'0304", 0) || compareSignature(&memoryMap, "'PK'0506", 0))  // TODO baHeader
        {
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_ZIP);
            // TODO Check APK, JAR
            // TODO basic ZIP
        } else if (compareSignature(&memoryMap, "1F8B08")) {
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_GZIP);
        } else if (compareSignature(&memoryMap, "7801") || compareSignature(&memoryMap, "785E") || compareSignature(&memoryMap, "789C") ||
                   compareSignature(&memoryMap, "78DA")) {
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_ZLIB);
        } else if (compareSignature(&memoryMap, "....'-lh'..2d") || compareSignature(&memoryMap, "....'-lz'..2d")) {
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_LHA);
        } else if (compareSignature(&memoryMap, "'!<arch>'0a")) {
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_AR);
            // TODO DEB
        } else if ((memoryMap.nBinarySize >= 0x200) && compareSignature(&memoryMap, "00'ustar'", 0x100)) {
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_TAR);
        } else if (compareSignature(&memoryMap, "'RE~^'") || compareSignature(&memoryMap, "'Rar!'1A07")) {
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_RAR);
        } else if (compareSignature(&memoryMap, "'MSCF'", 0)) {
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_CAB);
        } else if (compareSignature(&memoryMap, "'7z'BCAF271C", 0)) {
            stResult.insert(FT_ARCHIVE);
            stResult.insert(FT_7Z);
        } else if (compareSignature(&memoryMap, "89'PNG\r\n'1A0A", 0)) {
            stResult.insert(FT_IMAGE);
            stResult.insert(FT_PNG);
        } else if (compareSignature(&memoryMap, "FFD8FFE0....'JFIF'00", 0) || compareSignature(&memoryMap, "FFD8FFE1....'Exif'00", 0) ||
                   compareSignature(&memoryMap, "FFD8FFDB", 0)) {
            stResult.insert(FT_IMAGE);
            stResult.insert(FT_JPEG);
        } else if (compareSignature(&memoryMap, "'GIF87a'", 0) || compareSignature(&memoryMap, "'GIF89a'", 0)) {
            stResult.insert(FT_IMAGE);
            stResult.insert(FT_GIF);
        } else if (compareSignature(&memoryMap, "'BM'..................000000", 0)) {
            stResult.insert(FT_IMAGE);
            stResult.insert(FT_BMP);
        } else if (compareSignature(&memoryMap, "'MM'002A", 0) || compareSignature(&memoryMap, "'II'2A00", 0)) {
            stResult.insert(FT_IMAGE);
            stResult.insert(FT_TIFF);
        } else if (compareSignature(&memoryMap, "00000100", 0)) {
            stResult.insert(FT_IMAGE);
            stResult.insert(FT_ICO);
        } else if (compareSignature(&memoryMap, "00000200", 0)) {
            stResult.insert(FT_IMAGE);
            stResult.insert(FT_CUR);
        } else if (compareSignature(&memoryMap, "'ID3'..00", 0)) {
            stResult.insert(FT_AUDIO);
            stResult.insert(FT_MP3);
        } else if (compareSignature(&memoryMap, "000000..'ftyp'", 0)) {
            stResult.insert(FT_VIDEO);
            stResult.insert(FT_MP4);
        } else if (compareSignature(&memoryMap, "'dex\n'......00")) {
            stResult.insert(FT_DEX);
        } else if (compareSignature(&memoryMap, "02000C00")) {
            stResult.insert(FT_ANDROIDASRC);
        } else if (compareSignature(&memoryMap, "03000800")) {
            stResult.insert(FT_ANDROIDXML);
        } else if (compareSignature(&memoryMap, "'%PDF'", 0)) {
            stResult.insert(FT_DOCUMENT);
            stResult.insert(FT_PDF);
        } else if (compareSignature(&memoryMap, "'RIFF'", 0) || compareSignature(&memoryMap, "'RIFX'", 0)) {
            // TODO AIFF
            stResult.insert(FT_RIFF);
            if (compareSignature(&memoryMap, "'RIFF'........'AVI '", 0)) {
                stResult.insert(FT_VIDEO);
                stResult.insert(FT_AVI);
            } else if (compareSignature(&memoryMap, "'RIFF'........'WEBPVP8'", 0)) {
                stResult.insert(FT_IMAGE);
                stResult.insert(FT_WEBP);
            } /*else if (compareSignature(&memoryMap, "'RIFF'........'ACON'", 0)) {
                stResult.insert(FT_IMAGE);
                stResult.insert(FT_ANI);
            }*/
        } else if (compareSignature(&memoryMap, "'BW'....00..00000000", 0)) {
            stResult.insert(FT_BWDOS16M);
        } else {
            bAllFound = false;
        }

        if (!bAllFound) {
            if (nSize >= (qint64)sizeof(XMACH_DEF::fat_header) + (qint64)sizeof(XMACH_DEF::fat_arch)) {
                if (read_uint32(0, true) == XMACH_DEF::S_FAT_MAGIC) {
                    if (read_uint32(4, true) < 10) {
                        stResult.insert(FT_ARCHIVE);
                        stResult.insert(FT_MACHOFAT);
                        bAllFound = true;
                    }
                } else if (read_uint32(0, false) == XMACH_DEF::S_FAT_MAGIC) {
                    if (read_uint32(4, false) < 10) {
                        stResult.insert(FT_ARCHIVE);
                        stResult.insert(FT_MACHOFAT);
                        bAllFound = true;
                    }
                }
            }
        }

        if (!bAllFound) {
            if (nSize > 8) {
                if (read_uint32(0, true) == 0xCAFEBABE) {
                    if (read_uint32(4, true) > 10) {
                        stResult.insert(FT_JAVACLASS);
                        bAllFound = true;
                    }
                }
            }
        }

        if (isPlainTextType(&baHeader)) {
            stResult.insert(FT_TEXT);
            stResult.insert(FT_PLAINTEXT);
        } else if (isUTF8TextType(&baHeader)) {
            stResult.insert(FT_TEXT);
            stResult.insert(FT_UTF8);
        } else if (unicodeType != UNICODE_TYPE_NONE) {
            stResult.insert(FT_TEXT);
            stResult.insert(FT_UNICODE);

            if (unicodeType == UNICODE_TYPE_LE) {
                stResult.insert(FT_UNICODE_LE);
            } else {
                stResult.insert(FT_UNICODE_BE);
            }
        }
        // TODO more
        // TODO MIME

        // Fix
        if (stResult.contains(FT_GIF) && stResult.contains(FT_TEXT)) {
            stResult.remove(FT_GIF);
        }
    }

    if (bExtra) {
        if ((stResult.count() <= 1) || (stResult.contains(FT_PLAINTEXT))) {
            if ((nSize >= 0) && (nSize <= (0x10000 - 0x100))) {
                stResult.insert(FT_DATA);
                stResult.insert(FT_COM);
            }
        }
    }

    return stResult;
}

QSet<XBinary::FT> XBinary::getFileTypes(QIODevice *pDevice, bool bExtra)
{
    XBinary _binary(pDevice);

    return _binary.getFileTypes(bExtra);
}

QSet<XBinary::FT> XBinary::getFileTypes(const QString &sFileName, bool bExtra)
{
    QSet<XBinary::FT> result;

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        XBinary _binary(&file);

        result = _binary.getFileTypes(bExtra);

        file.close();
    }

    return result;
}

QSet<XBinary::FT> XBinary::getFileTypes(QByteArray *pbaData, bool bExtra)
{
    QSet<XBinary::FT> result;

    QBuffer buffer;

    buffer.setBuffer(pbaData);

    if (buffer.open(QIODevice::ReadOnly)) {
        XBinary _binary(&buffer);

        result = _binary.getFileTypes(bExtra);

        buffer.close();
    }

    return result;
}

XBinary::FT XBinary::_getPrefFileType(QSet<FT> *pStFileTypes)
{
    XBinary::FT result = FT_UNKNOWN;

    if (pStFileTypes->contains(FT_PE32)) {
        result = FT_PE32;
    } else if (pStFileTypes->contains(FT_PE64)) {
        result = FT_PE64;
    } else if (pStFileTypes->contains(FT_MACHO32)) {
        result = FT_MACHO32;
    } else if (pStFileTypes->contains(FT_MACHO64)) {
        result = FT_MACHO64;
    } else if (pStFileTypes->contains(FT_MACHOFAT)) {
        result = FT_MACHOFAT;
    } else if (pStFileTypes->contains(FT_ELF32)) {
        result = FT_ELF32;
    } else if (pStFileTypes->contains(FT_ELF64)) {
        result = FT_ELF64;
    } else if (pStFileTypes->contains(FT_LE)) {
        result = FT_LE;
    } else if (pStFileTypes->contains(FT_LX)) {
        result = FT_LX;
    } else if (pStFileTypes->contains(FT_NE)) {
        result = FT_NE;
    } else if (pStFileTypes->contains(FT_AMIGAHUNK)) {
        result = FT_AMIGAHUNK;
    } else if (pStFileTypes->contains(FT_DOS16M)) {
        result = FT_DOS16M;
    } else if (pStFileTypes->contains(FT_DOS4G)) {
        result = FT_DOS4G;
    } else if (pStFileTypes->contains(FT_MSDOS)) {
        result = FT_MSDOS;
    } else if (pStFileTypes->contains(FT_CAB)) {
        result = FT_CAB;
    } else if (pStFileTypes->contains(FT_RAR)) {
        result = FT_RAR;
    } else if (pStFileTypes->contains(FT_APKS)) {
        result = FT_APKS;
    } else if (pStFileTypes->contains(FT_APK)) {
        result = FT_APK;
    } else if (pStFileTypes->contains(FT_IPA)) {
        result = FT_IPA;
    } else if (pStFileTypes->contains(FT_JAR)) {
        result = FT_JAR;
    } else if (pStFileTypes->contains(FT_ZIP)) {
        result = FT_ZIP;
    } else if (pStFileTypes->contains(FT_NPM)) {
        result = FT_NPM;
    } else if (pStFileTypes->contains(FT_TAR)) {
        result = FT_TAR;
    } else if (pStFileTypes->contains(FT_TARGZ)) {
        result = FT_TARGZ;
    } else if (pStFileTypes->contains(FT_GZIP)) {
        result = FT_GZIP;
    } else if (pStFileTypes->contains(FT_ZLIB)) {
        result = FT_ZLIB;
    } else if (pStFileTypes->contains(FT_LHA)) {
        result = FT_LHA;
    } else if (pStFileTypes->contains(FT_7Z)) {
        result = FT_7Z;
    } else if (pStFileTypes->contains(FT_DEB)) {
        result = FT_DEB;
    } else if (pStFileTypes->contains(FT_AR)) {
        result = FT_AR;
    } else if (pStFileTypes->contains(FT_ANDROIDXML)) {
        result = FT_ANDROIDXML;
    } else if (pStFileTypes->contains(FT_DEX)) {
        result = FT_DEX;
    } else if (pStFileTypes->contains(FT_PNG)) {
        result = FT_PNG;
    } else if (pStFileTypes->contains(FT_ICO)) {
        result = FT_ICO;
    } else if (pStFileTypes->contains(FT_CUR)) {
        result = FT_CUR;
    } else if (pStFileTypes->contains(FT_JPEG)) {
        result = FT_JPEG;
    } else if (pStFileTypes->contains(FT_BMP)) {
        result = FT_BMP;
    } else if (pStFileTypes->contains(FT_GIF)) {
        result = FT_GIF;
    } else if (pStFileTypes->contains(FT_TIFF)) {
        result = FT_TIFF;
    } else if (pStFileTypes->contains(FT_MP3)) {
        result = FT_MP3;
    } else if (pStFileTypes->contains(FT_MP4)) {
        result = FT_MP4;
    } else if (pStFileTypes->contains(FT_AVI)) {
        result = FT_AVI;
    } else if (pStFileTypes->contains(FT_WEBP)) {
        result = FT_WEBP;
    } else if (pStFileTypes->contains(FT_SIGNATURE)) {
        result = FT_SIGNATURE;
    } else if (pStFileTypes->contains(FT_RIFF)) {
        result = FT_RIFF;
    } else if (pStFileTypes->contains(FT_PDF)) {
        result = FT_PDF;
    } else if (pStFileTypes->contains(FT_BWDOS16M)) {
        result = FT_BWDOS16M;
    } else if (pStFileTypes->contains(FT_JAVACLASS)) {
        result = FT_JAVACLASS;
    } else if (pStFileTypes->contains(FT_DATA)) {
        result = FT_DATA;
    } else if (pStFileTypes->contains(FT_BINARY)) {
        result = FT_BINARY;
    }

    return result;
}

QSet<XBinary::FT> XBinary::getFileTypes(QIODevice *pDevice, qint64 nOffset, qint64 nSize, bool bExtra)
{
    QSet<XBinary::FT> result;

    SubDevice sd(pDevice, nOffset, nSize);

    if (sd.open(QIODevice::ReadOnly)) {
        result = getFileTypes(&sd, bExtra);

        sd.close();
    }

    return result;
}

XBinary::FT XBinary::getPrefFileType(QIODevice *pDevice, bool bExtra)
{
    QSet<XBinary::FT> stFileTypes = getFileTypes(pDevice, bExtra);

    return _getPrefFileType(&stFileTypes);
}

XBinary::FT XBinary::getPrefFileType(const QString &sFileName, bool bExtra)
{
    XBinary::FT result = XBinary::FT_UNKNOWN;

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        XBinary _binary(&file);

        result = _binary.getPrefFileType(&file, bExtra);

        file.close();
    }

    return result;
}

QList<XBinary::FT> XBinary::_getFileTypeListFromSet(const QSet<FT> &stFileTypes, TL_OPTION tlOption)
{
    QList<XBinary::FT> listResult;

    // TODO optimize !
    if ((tlOption == TL_OPTION_DEFAULT) || (tlOption == TL_OPTION_EXECUTABLE) || (tlOption == TL_OPTION_ALL)) {
        if (stFileTypes.contains(FT_REGION)) listResult.append(FT_REGION);
        if (stFileTypes.contains(FT_DATA)) listResult.append(FT_DATA);
        if (stFileTypes.contains(FT_BINARY)) listResult.append(FT_BINARY);
        if (stFileTypes.contains(FT_BINARY16)) listResult.append(FT_BINARY16);
        if (stFileTypes.contains(FT_BINARY32)) listResult.append(FT_BINARY32);
        if (stFileTypes.contains(FT_BINARY64)) listResult.append(FT_BINARY64);
    }

    if (tlOption == TL_OPTION_ALL) {
        if (stFileTypes.contains(FT_ARCHIVE)) listResult.append(FT_ARCHIVE);
    }

    if ((tlOption == TL_OPTION_DEFAULT) || (tlOption == TL_OPTION_ALL)) {
        if (stFileTypes.contains(FT_ZIP)) listResult.append(FT_ZIP);
        if (stFileTypes.contains(FT_GZIP)) listResult.append(FT_GZIP);
        if (stFileTypes.contains(FT_ZLIB)) listResult.append(FT_ZLIB);
        if (stFileTypes.contains(FT_LHA)) listResult.append(FT_LHA);
        if (stFileTypes.contains(FT_RAR)) listResult.append(FT_RAR);
        if (stFileTypes.contains(FT_JAR)) listResult.append(FT_JAR);
        if (stFileTypes.contains(FT_APK)) listResult.append(FT_APK);
        if (stFileTypes.contains(FT_IPA)) listResult.append(FT_IPA);
        if (stFileTypes.contains(FT_7Z)) listResult.append(FT_7Z);
        if (stFileTypes.contains(FT_DEX)) listResult.append(FT_DEX);
        if (stFileTypes.contains(FT_PDF)) listResult.append(FT_PDF);
        if (stFileTypes.contains(FT_PNG)) listResult.append(FT_PNG);
        if (stFileTypes.contains(FT_ICO)) listResult.append(FT_ICO);
        if (stFileTypes.contains(FT_JPEG)) listResult.append(FT_JPEG);
        if (stFileTypes.contains(FT_BMP)) listResult.append(FT_BMP);
        if (stFileTypes.contains(FT_GIF)) listResult.append(FT_GIF);
        if (stFileTypes.contains(FT_TIFF)) listResult.append(FT_TIFF);
        if (stFileTypes.contains(FT_MP3)) listResult.append(FT_MP3);
        if (stFileTypes.contains(FT_MP4)) listResult.append(FT_MP4);
        if (stFileTypes.contains(FT_RIFF)) listResult.append(FT_RIFF);
        if (stFileTypes.contains(FT_AVI)) listResult.append(FT_AVI);
        if (stFileTypes.contains(FT_WEBP)) listResult.append(FT_WEBP);
        if (stFileTypes.contains(FT_SIGNATURE)) listResult.append(FT_SIGNATURE);
        if (stFileTypes.contains(FT_TAR)) listResult.append(FT_TAR);
        if (stFileTypes.contains(FT_TARGZ)) listResult.append(FT_TARGZ);
        if (stFileTypes.contains(FT_NPM)) listResult.append(FT_NPM);
        if (stFileTypes.contains(FT_MACHOFAT)) listResult.append(FT_MACHOFAT);
        if (stFileTypes.contains(FT_DEB)) listResult.append(FT_DEB);
        if (stFileTypes.contains(FT_AR)) listResult.append(FT_AR);
        if (stFileTypes.contains(FT_JAVACLASS)) listResult.append(FT_JAVACLASS);
    }

    if ((tlOption == TL_OPTION_DEFAULT) || (tlOption == TL_OPTION_EXECUTABLE) || (tlOption == TL_OPTION_ALL)) {
        if (stFileTypes.contains(FT_COM)) listResult.append(FT_COM);
        if (stFileTypes.contains(FT_MSDOS)) listResult.append(FT_MSDOS);
        if (stFileTypes.contains(FT_NE)) listResult.append(FT_NE);
        if (stFileTypes.contains(FT_LE)) listResult.append(FT_LE);
        if (stFileTypes.contains(FT_LX)) listResult.append(FT_LX);
        if (stFileTypes.contains(FT_PE32)) listResult.append(FT_PE32);
        if (stFileTypes.contains(FT_PE64)) listResult.append(FT_PE64);
        if (stFileTypes.contains(FT_ELF32)) listResult.append(FT_ELF32);
        if (stFileTypes.contains(FT_ELF64)) listResult.append(FT_ELF64);
        if (stFileTypes.contains(FT_MACHO32)) listResult.append(FT_MACHO32);
        if (stFileTypes.contains(FT_MACHO64)) listResult.append(FT_MACHO64);
        if (stFileTypes.contains(FT_BWDOS16M)) listResult.append(FT_BWDOS16M);
        if (stFileTypes.contains(FT_AMIGAHUNK)) listResult.append(FT_AMIGAHUNK);
    }

    if ((tlOption == TL_OPTION_DEFAULT) || (tlOption == TL_OPTION_ALL)) {
        if (stFileTypes.contains(FT_DOS16M)) listResult.append(FT_DOS16M);
        if (stFileTypes.contains(FT_DOS4G)) listResult.append(FT_DOS4G);
    }

    return listResult;
}

QString XBinary::valueToHex(quint8 nValue)
{
    return QString("%1").arg(nValue, 2, 16, QChar('0'));
}

QString XBinary::valueToHex(qint8 nValue)
{
    return valueToHex((quint8)nValue);
}

QString XBinary::valueToHex(quint16 nValue, bool bIsBigEndian)
{
    if (bIsBigEndian) {
        nValue = qFromBigEndian(nValue);
    } else {
        nValue = qFromLittleEndian(nValue);
    }

    return QString("%1").arg(nValue, 4, 16, QChar('0'));
}

QString XBinary::valueToHex(qint16 nValue, bool bIsBigEndian)
{
    if (bIsBigEndian) {
        nValue = qFromBigEndian(nValue);
    } else {
        nValue = qFromLittleEndian(nValue);
    }

    return valueToHex((quint16)nValue);
}

QString XBinary::valueToHex(quint32 nValue, bool bIsBigEndian)
{
    if (bIsBigEndian) {
        nValue = qFromBigEndian(nValue);
    } else {
        nValue = qFromLittleEndian(nValue);
    }

    return QString("%1").arg(nValue, 8, 16, QChar('0'));
}

QString XBinary::valueToHex(qint32 nValue, bool bIsBigEndian)
{
    if (bIsBigEndian) {
        nValue = qFromBigEndian(nValue);
    } else {
        nValue = qFromLittleEndian(nValue);
    }

    return valueToHex((quint32)nValue);
}

QString XBinary::valueToHex(quint64 nValue, bool bIsBigEndian)
{
    if (bIsBigEndian) {
        nValue = qFromBigEndian(nValue);
    } else {
        nValue = qFromLittleEndian(nValue);
    }

    return QString("%1").arg(nValue, 16, 16, QChar('0'));
}

QString XBinary::valueToHex(qint64 nValue, bool bIsBigEndian)
{
    if (bIsBigEndian) {
        nValue = qFromBigEndian(nValue);
    } else {
        nValue = qFromLittleEndian(nValue);
    }

    return valueToHex((quint64)nValue);
}

QString XBinary::valueToHex(float fValue, bool bIsBigEndian)
{
    float _value = fValue;

    endian_float(&_value, bIsBigEndian);

    quint32 _nValue = 0;

    _copyMemory((char *)&_nValue, (char *)&_value, 4);

    return QString("%1").arg(_nValue, 8, 16, QChar('0'));
}

QString XBinary::valueToHex(double dValue, bool bIsBigEndian)
{
    double _value = dValue;

    endian_double(&_value, bIsBigEndian);

    quint64 _nValue = 0;

    _copyMemory((char *)&_nValue, (char *)&_value, 8);

    return QString("%1").arg(_nValue, 16, 16, QChar('0'));
}

QString XBinary::valueToHex(XBinary::MODE mode, quint64 nValue, bool bIsBigEndian)
{
    QString sResult;

    if (mode == MODE_UNKNOWN) {
        mode = getWidthModeFromSize(nValue);
    }

    if (mode == MODE_BIT) {
        if (nValue) {
            sResult = "1";
        } else {
            sResult = "0";
        }
    } else if (mode == MODE_8) {
        sResult = valueToHex((quint8)nValue);
    } else if (mode == MODE_16) {
        sResult = valueToHex((quint16)nValue, bIsBigEndian);
    } else if (mode == MODE_32) {
        sResult = valueToHex((quint32)nValue, bIsBigEndian);
    } else if (mode == MODE_64) {
        sResult = valueToHex((quint64)nValue, bIsBigEndian);
    }

    return sResult;
}

QString XBinary::valueToHexEx(quint64 nValue, bool bIsBigEndian)
{
    XBinary::MODE mode = getWidthModeFromSize(nValue);

    return valueToHex(mode, nValue, bIsBigEndian);
}

QString XBinary::valueToHexOS(quint64 nValue, bool bIsBigEndian)
{
    MODE mode = MODE_32;

    if (sizeof(void *) == 8) {
        mode = MODE_64;
    }

    return valueToHex(mode, nValue, bIsBigEndian);
}

QString XBinary::valueToHexColon(MODE mode, quint64 nValue, bool bIsBigEndian)
{
    QString sResult;

    if (mode == MODE_64) {
        quint32 nHigh = (quint32)(nValue >> 32);
        quint32 nLow = (quint32)(nValue);
        sResult = QString("%1:%2").arg(valueToHex(nHigh), valueToHex(nLow));
    } else if (mode == MODE_32) {
        quint16 nHigh = (quint16)(nValue >> 16);
        quint16 nLow = (quint16)(nValue);
        sResult = QString("%1:%2").arg(valueToHex(nHigh), valueToHex(nLow));
    } else {
        sResult = valueToHex(mode, nValue, bIsBigEndian);
    }

    return sResult;
}

QString XBinary::xVariantToHex(XVARIANT value)
{
    QString sResult;

    if (value.mode == MODE_BIT) {
        if (value.var.v_bool) {
            sResult = "1";
        } else {
            sResult = "0";
        }
    } else if (value.mode == MODE_8) {
        sResult = valueToHex(value.var.v_uint8);
    } else if (value.mode == MODE_16) {
        sResult = valueToHex(value.var.v_uint16, value.bIsBigEndian);
    } else if (value.mode == MODE_32) {
        sResult = valueToHex(value.var.v_uint32, value.bIsBigEndian);
    } else if (value.mode == MODE_64) {
        sResult = valueToHex(value.var.v_uint64, value.bIsBigEndian);
    } else if (value.mode == MODE_128) {
        QString sLow = valueToHex(value.var.v_uint128[0], value.bIsBigEndian);
        QString sHigh = valueToHex(value.var.v_uint128[1], value.bIsBigEndian);

        if (value.bIsBigEndian) {
            sResult = sLow + sHigh;
        } else {
            sResult = sHigh + sLow;
        }
    } else if (value.mode == MODE_256) {
        QString s0 = valueToHex(value.var.v_uint256[0], value.bIsBigEndian);
        QString s1 = valueToHex(value.var.v_uint256[1], value.bIsBigEndian);
        QString s2 = valueToHex(value.var.v_uint256[2], value.bIsBigEndian);
        QString s3 = valueToHex(value.var.v_uint256[3], value.bIsBigEndian);

        if (value.bIsBigEndian) {
            sResult = s0 + s1 + s2 + s3;
        } else {
            sResult = s3 + s2 + s1 + s0;
        }
    } else if (value.mode == MODE_FREG) {
        for (qint32 i = 0; i < 10; i++) {
            sResult += valueToHex(value.var.v_freg[i]);
        }
    }

    return sResult;
}

QString XBinary::thisToString(qint64 nDelta, qint32 nBase)
{
    QString sResult;

    if (nDelta == 0) {
        sResult = "$ ==>";
    } else if (nDelta > 0) {
        sResult = QString("$+%1").arg(nDelta, 0, nBase);
    } else if (nDelta < 0) {
        sResult = QString("$-%1").arg(-nDelta, 0, nBase);
    }

    return sResult;
}

bool XBinary::checkString_byte(const QString &sValue)
{
    bool bResult = false;

    // TODO Check

    quint16 nValue = sValue.toUShort(&bResult, 16);

    if (bResult) {
        bResult = (nValue <= 256);
    }

    return bResult;
}

bool XBinary::checkString_word(const QString &sValue)
{
    bool bResult = false;

    sValue.toUShort(&bResult, 16);

    return bResult;
}

bool XBinary::checkString_dword(const QString &sValue)
{
    bool bResult = false;

    sValue.toUInt(&bResult, 16);

    return bResult;
}

bool XBinary::checkString_qword(const QString &sValue)
{
    bool bResult = false;

    sValue.toULongLong(&bResult, 16);

    return bResult;
}

bool XBinary::checkString_uint8(const QString &sValue)
{
    bool bResult = false;

    // TODO Check

    quint16 nValue = sValue.toUShort(&bResult);

    if (bResult) {
        bResult = (nValue <= 256);
    }

    return bResult;
}

bool XBinary::checkString_int8(const QString &sValue)
{
    bool bResult = false;

    qint16 nValue = sValue.toShort(&bResult);

    if (bResult) {
        bResult = ((-127 <= nValue) && (nValue <= 128));
    }

    return bResult;
}

bool XBinary::checkString_uint16(const QString &sValue)
{
    bool bResult = false;

    sValue.toUShort(&bResult);

    return bResult;
}

bool XBinary::checkString_int16(const QString &sValue)
{
    bool bResult = false;

    sValue.toShort(&bResult);

    return bResult;
}

bool XBinary::checkString_uint32(const QString &sValue)
{
    bool bResult = false;

    sValue.toUInt(&bResult);

    return bResult;
}

bool XBinary::checkString_int32(const QString &sValue)
{
    bool bResult = false;

    sValue.toInt(&bResult);

    return bResult;
}

bool XBinary::checkString_uint64(const QString &sValue)
{
    bool bResult = false;

    sValue.toULongLong(&bResult);

    return bResult;
}

bool XBinary::checkString_int64(const QString &sValue)
{
    bool bResult = false;

    sValue.toLongLong(&bResult);

    return bResult;
}

bool XBinary::checkString_float(const QString &sValue)
{
    bool bResult = false;

    sValue.toFloat(&bResult);

    return bResult;
}

bool XBinary::checkString_double(const QString &sValue)
{
    bool bResult = false;

    sValue.toDouble(&bResult);

    return bResult;
}

QString XBinary::boolToString(bool bValue)
{
    QString sResult;

    if (bValue) {
        sResult = tr("true");
    } else {
        sResult = tr("false");
    }

    return sResult;
}

QString XBinary::getSpaces(qint32 nNumberOfSpaces)
{
    QString sResult;

    sResult = sResult.rightJustified(nNumberOfSpaces, QChar(' '));

    return sResult;
}

QString XBinary::getUnpackedFileName(QIODevice *pDevice, bool bShort)
{
    QString sResult = "unpacked";

    QFile *pFile = dynamic_cast<QFile *>(pDevice);

    if (pFile) {
        QString sFileName = pFile->fileName();

        if (sFileName != "") {
            sResult = getUnpackedFileName(sFileName);
        }

        if (bShort) {
            QFileInfo fi(sResult);

            sResult = fi.completeBaseName() + "." + fi.suffix();
        }
    }

    return sResult;
}

QString XBinary::getUnpackedFileName(const QString &sFileName)
{
    QFileInfo fileInfo(sFileName);
    QString sResult = fileInfo.absolutePath() + QDir::separator() + fileInfo.completeBaseName() + ".unp." + fileInfo.suffix();
    //            sResult=fi.absolutePath()+QDir::separator()+fi.baseName()+".unp."+fi.completeSuffix();

    return sResult;
}

QString XBinary::getDumpFileName(QIODevice *pDevice)
{
    QString sResult = "dump";

    QFile *pFile = dynamic_cast<QFile *>(pDevice);

    if (pFile) {
        QString sFileName = pFile->fileName();

        if (sFileName != "") {
            sResult = getDumpFileName(sFileName);
        }
    }

    return sResult;
}

QString XBinary::getDumpFileName(const QString &sFileName)
{
    QFileInfo fileInfo(sFileName);
    QString sResult = fileInfo.absolutePath() + QDir::separator() + fileInfo.completeBaseName() + ".dump." + fileInfo.suffix();

    return sResult;
}

QString XBinary::getBackupFileName(QIODevice *pDevice)
{
    QString sResult = QString("Backup_%1.BAK").arg(getCurrentBackupDate());

    QFile *pFile = dynamic_cast<QFile *>(pDevice);

    if (pFile) {
        QString sFileName = pFile->fileName();

        if (sFileName != "") {
            sResult = getBackupFileName(sFileName);
        }
    }

    return sResult;
}

QString XBinary::getBackupFileName(const QString &sFileName)
{
    QFileInfo fi(sFileName);
    QString sResult;

    sResult += fi.absolutePath() + QDir::separator() + fi.completeBaseName();

    QString sSuffix = fi.suffix();

    if (sSuffix != "") {
        sResult += "." + sSuffix;
    }

    sResult += QString("_%1.BAK").arg(getCurrentBackupDate());

    return sResult;
}

QString XBinary::getResultFileName(QIODevice *pDevice, const QString &sAppendix)
{
    QString sResult = sAppendix;

    QFile *pFile = dynamic_cast<QFile *>(pDevice);

    if (pFile) {
        QString sFileName = pFile->fileName();

        if (sFileName != "") {
            sResult = getResultFileName(sFileName, sAppendix);
        }
    }

    return sResult;
}

QString XBinary::getResultFileName(const QString &sFileName, const QString &sAppendix)
{
    QString sResult;
    // mb TODO if file exists write other .1 .2 ...
    QFileInfo fileInfo(sFileName);

    QString sSuffix = fileInfo.suffix();

    sResult += fileInfo.absolutePath() + QDir::separator() + fileInfo.completeBaseName() + ".";

    if (sSuffix != "") {
        sResult += sSuffix + ".";
    }

    sResult += sAppendix;

    return sResult;
}

QString XBinary::getDeviceFileName(QIODevice *pDevice)
{
    QString sResult;

    QFile *pFile = dynamic_cast<QFile *>(pDevice);

    if (pFile) {
        sResult = pFile->fileName();
    }

    return sResult;
}

QString XBinary::getDeviceFilePath(QIODevice *pDevice)
{
    QString sResult;

    QFile *pFile = dynamic_cast<QFile *>(pDevice);

    if (pFile) {
        QString sFileName = pFile->fileName();

        if (sFileName != "") {
            QFileInfo fi(sFileName);

            sResult = fi.absoluteFilePath();
        }
    }

    return sResult;
}

QString XBinary::getDeviceDirectory(QIODevice *pDevice)
{
    QString sResult;

    QFile *pFile = dynamic_cast<QFile *>(pDevice);

    if (pFile) {
        QString sFileName = pFile->fileName();

        if (sFileName != "") {
            QFileInfo fi(sFileName);

            sResult = fi.absolutePath();
        }
    } else {
        sResult = pDevice->property("DeviceDirectory").toString();
    }

    return sResult;
}

QString XBinary::getDeviceFileBaseName(QIODevice *pDevice)
{
    QString sResult;

    QFile *pFile = dynamic_cast<QFile *>(pDevice);

    if (pFile) {
        QString sFileName = pFile->fileName();

        if (sFileName != "") {
            QFileInfo fi(sFileName);

            sResult = fi.baseName();
        }
    } else {
        sResult = pDevice->property("DeviceFileBaseName").toString();
    }

    return sResult;
}

QString XBinary::getDeviceFileCompleteSuffix(QIODevice *pDevice)
{
    QString sResult;

    QFile *pFile = dynamic_cast<QFile *>(pDevice);

    if (pFile) {
        QString sFileName = pFile->fileName();

        if (sFileName != "") {
            QFileInfo fi(sFileName);

            sResult = fi.completeSuffix();
        }
    } else {
        sResult = pDevice->property("DeviceFileCompleteSuffix").toString();
    }

    return sResult;
}

QString XBinary::getDeviceFileSuffix(QIODevice *pDevice)
{
    QString sResult;

    QFile *pFile = dynamic_cast<QFile *>(pDevice);

    if (pFile) {
        QString sFileName = pFile->fileName();

        if (sFileName != "") {
            QFileInfo fi(sFileName);

            sResult = fi.suffix();
        }
    } else {
        sResult = pDevice->property("DeviceFileSuffix").toString();
    }

    return sResult;
}

QString XBinary::getFileDirectory(const QString &sFileName)
{
    QFileInfo fi(sFileName);

    return fi.absolutePath();
}

QIODevice *XBinary::getBackupDevice(QIODevice *pDevice)
{
    QIODevice *pResult = nullptr;

    pResult = (QIODevice *)pDevice->property("BACKUPDEVICE").toULongLong();

    if (!pResult) {
        pResult = pDevice;
    }

    return pResult;
}

bool XBinary::isBackupPresent(QIODevice *pDevice)
{
    return XBinary::isFileExists(XBinary::getBackupFileName(pDevice));
}

bool XBinary::saveBackup(QIODevice *pDevice)
{
    bool bResult = false;

    QString sBackupFileName = XBinary::getBackupFileName(pDevice);

    if (sBackupFileName != "") {
        if (!QFile::exists(sBackupFileName)) {
            QString sFileName = XBinary::getDeviceFileName(pDevice);

            if (sFileName != "") {
                bResult = QFile::copy(sFileName, sBackupFileName);
            } else {
                bResult = XBinary::writeToFile(sBackupFileName, pDevice);
            }
        } else {
            bResult = true;
        }
    }

    return bResult;
}

QString XBinary::getCurrentBackupDate()
{
    QString sResult;

    sResult = QDate::currentDate().toString("yyyy-MM-dd");

    return sResult;
}

QList<qint64> XBinary::getFixupList(QIODevice *pDevice1, QIODevice *pDevice2, qint64 nDelta)
{
    QList<qint64> listResult;

    qint64 nSize1 = pDevice1->size();
    qint64 nSize2 = pDevice2->size();

    if (nSize1 == nSize2) {
        qint64 nSize = nSize1;
        qint64 nTemp = 0;

        char *pBuffer1 = new char[READWRITE_BUFFER_SIZE + 3];
        char *pBuffer2 = new char[READWRITE_BUFFER_SIZE + 3];
        qint64 nOffset = 0;

        while (nSize > 3) {
            nTemp = qMin((qint64)(READWRITE_BUFFER_SIZE + 3), nSize);

            pDevice1->seek(nOffset);

            if (pDevice1->read(pBuffer1, nTemp) <= 0) {
                break;
            }

            pDevice2->seek(nOffset);

            if (pDevice2->read(pBuffer2, nTemp) <= 0) {
                break;
            }

            for (quint32 i = 0; i < nTemp - 3; i++) {
                qint32 nValue1 = *(qint32 *)(pBuffer1 + i);
                qint32 nValue2 = *(qint32 *)(pBuffer2 + i);

                if (nValue1 + nDelta == nValue2) {
                    listResult.append(nOffset + i);
                }
            }

            nSize -= nTemp - 3;
            nOffset += nTemp - 3;
        }

        delete[] pBuffer1;
        delete[] pBuffer2;
    }

    return listResult;
}

QString XBinary::getHash(XBinary::HASH hash, const QString &sFileName, PDSTRUCT *pPdStruct)
{
    QString sResult;

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        sResult = XBinary::getHash(hash, &file, pPdStruct);

        file.close();
    }

    return sResult;
}

QString XBinary::getHash(XBinary::HASH hash, QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    QString sResult;

    XBinary binary(pDevice);

    sResult = binary.getHash(hash, 0, -1, pPdStruct);

    pDevice->reset();

    return sResult;
}

QString XBinary::getHash(XBinary::HASH hash, qint64 nOffset, qint64 nSize, PDSTRUCT *pPdStruct)
{
    QString sResult;

    OFFSETSIZE osRegion = convertOffsetAndSize(nOffset, nSize);

    if (osRegion.nOffset != -1) {
        QList<OFFSETSIZE> listOS;
        listOS.append(osRegion);

        sResult = getHash(hash, &listOS, pPdStruct);
    }

    return sResult;
}

QString XBinary::getHash(HASH hash, QList<OFFSETSIZE> *pListOS, PDSTRUCT *pPdStruct)
{
    QString sResult;

    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    qint64 nTemp = 0;
    char *pBuffer = new char[READWRITE_BUFFER_SIZE];

    QCryptographicHash::Algorithm algorithm = QCryptographicHash::Md4;

    switch (hash)  // TODO Check new versions of Qt
    {
        case HASH_MD4: algorithm = QCryptographicHash::Md4; break;
        case HASH_MD5: algorithm = QCryptographicHash::Md5; break;
        case HASH_SHA1: algorithm = QCryptographicHash::Sha1; break;
#ifndef QT_CRYPTOGRAPHICHASH_ONLY_SHA
#if (QT_VERSION_MAJOR > 4)
        case HASH_SHA224: algorithm = QCryptographicHash::Sha224; break;  // Keccak_224 ?
        case HASH_SHA256: algorithm = QCryptographicHash::Sha256; break;
        case HASH_SHA384: algorithm = QCryptographicHash::Sha384; break;
        case HASH_SHA512: algorithm = QCryptographicHash::Sha512; break;
#endif
#endif
    }

    QCryptographicHash crypto(algorithm);

    qint32 nNumberOfRecords = pListOS->count();

    qint64 nCurrentSize = 0;

    qint32 _nFreeIndex = XBinary::getFreeIndex(pPdStruct);

    bool bReadError = false;

    for (qint32 i = 0; (i < nNumberOfRecords) && (!(pPdStruct->bIsStop)); i++) {
        qint64 nOffset = pListOS->at(i).nOffset;
        qint64 nSize = pListOS->at(i).nSize;

        XBinary::setPdStructInit(pPdStruct, _nFreeIndex, nSize);

        while ((nSize > 0) && (!(pPdStruct->bIsStop))) {
            nTemp = qMin((qint64)READWRITE_BUFFER_SIZE, nSize);

            if (read_array(nOffset, pBuffer, nTemp) != nTemp) {
                pPdStruct->sInfoString = tr("Read error");

                bReadError = true;

                break;
            }

            crypto.addData(pBuffer, nTemp);

            nSize -= nTemp;
            nOffset += nTemp;
            nCurrentSize += nTemp;

            XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, nCurrentSize);
        }
    }

    XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);

    delete[] pBuffer;

    if (!bReadError) {
        sResult = crypto.result().toHex();
    }

    if (pPdStruct->bIsStop) {
        sResult = "";
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
#if (QT_VERSION_MAJOR > 4)
    stResult.insert(HASH_SHA224);
    stResult.insert(HASH_SHA256);
    stResult.insert(HASH_SHA384);
    stResult.insert(HASH_SHA512);
#endif
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
#if (QT_VERSION_MAJOR > 4)
    listResult.append(HASH_SHA224);
    listResult.append(HASH_SHA256);
    listResult.append(HASH_SHA384);
    listResult.append(HASH_SHA512);
#endif
#endif

    return listResult;
}

QString XBinary::hashIdToString(XBinary::HASH hash)
{
    QString sResult = tr("Unknown");

    switch (hash) {
        case HASH_MD4: sResult = QString("MD4"); break;
        case HASH_MD5: sResult = QString("MD5"); break;
        case HASH_SHA1: sResult = QString("SHA1"); break;
#ifndef QT_CRYPTOGRAPHICHASH_ONLY_SHA1
#if (QT_VERSION_MAJOR > 4)
        case HASH_SHA224: sResult = QString("SHA224"); break;
        case HASH_SHA256: sResult = QString("SHA256"); break;
        case HASH_SHA384: sResult = QString("SHA384"); break;
        case HASH_SHA512: sResult = QString("SHA512"); break;
#endif
#endif
    }

    return sResult;
}

bool XBinary::isFileHashValid(XBinary::HASH hash, const QString &sFileName, const QString &sHash)
{
    bool bResult = false;

    if (isFileExists(sFileName)) {
        bResult = (getHash(hash, sFileName).toUpper() == sHash.toUpper());
    }

    return bResult;
}

quint32 XBinary::getAdler32(const QString &sFileName)
{
    quint32 nResult = 0;

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        nResult = XBinary::getAdler32(&file);

        file.close();
    }

    return nResult;
}

quint32 XBinary::getAdler32(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    quint32 nResult = 0;

    XBinary binary(pDevice);

    nResult = binary.getAdler32(0, -1, pPdStruct);

    pDevice->reset();

    return nResult;
}

quint32 XBinary::getAdler32(qint64 nOffset, qint64 nSize, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }
    // TODO Check crash
    // TODO optimize!!!
    // TODO Progress bar
    quint32 nResult = 0;

    OFFSETSIZE osRegion = convertOffsetAndSize(nOffset, nSize);

    nOffset = osRegion.nOffset;
    nSize = osRegion.nSize;

    const quint32 MOD_ADLER = 65521;

    if (nOffset != -1) {
        qint64 nTemp = 0;
        char *pBuffer = new char[READWRITE_BUFFER_SIZE];

        quint32 a = 1;
        quint32 b = 0;

        qint32 _nFreeIndex = XBinary::getFreeIndex(pPdStruct);

        XBinary::setPdStructInit(pPdStruct, _nFreeIndex, nSize);

        while ((nSize > 0) && (!(pPdStruct->bIsStop))) {
            nTemp = qMin((qint64)READWRITE_BUFFER_SIZE, nSize);

            if (read_array(nOffset, pBuffer, nTemp) != nTemp) {
                pPdStruct->sInfoString = tr("Read error");
                delete[] pBuffer;
                return 0;
            }

            for (qint64 i = 0; i < nTemp; i++) {
                a = (a + (quint8)(pBuffer[nOffset + i])) % MOD_ADLER;
                b = (b + a) % MOD_ADLER;
            }

            nSize -= nTemp;
            nOffset += nTemp;

            XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, nOffset);
        }

        XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);

        delete[] pBuffer;

        nResult = (b << 16) | a;
    }

    return nResult;
}

void XBinary::_createCRC32Table(quint32 *pCRCTable, quint32 nPoly)
{
    for (qint32 i = 0; i < 256; i++) {
        quint32 crc = i;

        for (qint32 j = 0; j < 8; j++) {
            crc = (crc & 1) ? ((crc >> 1) ^ nPoly) : (crc >> 1);
        }

        *(pCRCTable + i) = crc;
    }
}

quint32 *XBinary::_getCRC32Table_EDB88320()
{
    return (quint32 *)_crc32_EDB88320_tab;
}

quint16 *XBinary::_getCRC16Table()
{
    return (quint16 *)_crc16_tab;
}

quint32 XBinary::_getCRC32(const QString &sFileName, PDSTRUCT *pPdStruct)
{
    quint32 nResult = 0;

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        nResult = XBinary::_getCRC32(&file, pPdStruct);

        file.close();
    }

    return nResult;
}

quint32 XBinary::_getCRC32(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    quint32 nResult = 0;

    XBinary binary(pDevice);

    nResult = binary._getCRC32(0, -1, 0xFFFFFFFF, pPdStruct);

    pDevice->reset();

    //    quint32 nResult2 = binary._getCRC32_2(0, -1);

    return nResult;
}

quint32 XBinary::_getCRC32(const char *pData, qint32 nDataSize, quint32 nInit, quint32 *pCRCTable)
{
    quint32 nResult = nInit;

    while (nDataSize > 0) {
        quint8 nIndex = (nResult ^ (((quint8)(*pData)) & 0xFF));
        nResult = (*(pCRCTable + nIndex)) ^ (nResult >> 8);

        nDataSize--;
        pData++;
    }

    return nResult;
}

quint16 XBinary::_getCRC16(const char *pData, qint32 nDataSize, quint16 nInit, quint16 *pCRCTable)
{
    quint16 nResult = nInit;

    while (nDataSize > 0) {
        quint8 nIndex = (nResult ^ (((quint8)(*pData)) & 0xFF));
        nResult = (*(pCRCTable + nIndex)) ^ (nResult >> 8);

        nDataSize--;
        pData++;
    }

    return nResult;
}

quint32 XBinary::_getCRC32(const QByteArray &baData, quint32 nInit, quint32 *pCRCTable)
{
    return _getCRC32(baData.data(), baData.size(), nInit, pCRCTable);
}

quint16 XBinary::_getCRC16(const QByteArray &baData, quint16 nInit, quint16 *pCRCTable)
{
    return _getCRC16(baData.data(), baData.size(), nInit, pCRCTable);
}

quint32 XBinary::_getCRC32(qint64 nOffset, qint64 nSize, quint32 nInit, PDSTRUCT *pPdStruct)
{
    // TODO optimize!!!
    quint32 nResult = nInit;  // ~0 // TODO make nInit arg

    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    OFFSETSIZE osRegion = convertOffsetAndSize(nOffset, nSize);

    nOffset = osRegion.nOffset;
    nSize = osRegion.nSize;

    qint32 _nFreeIndex = XBinary::getFreeIndex(pPdStruct);

    if ((nOffset != -1) && (!(pPdStruct->bIsStop))) {
        XBinary::setPdStructInit(pPdStruct, _nFreeIndex, nSize);

        qint64 nTemp = 0;
        char *pBuffer = new char[READWRITE_BUFFER_SIZE];

        while (nSize > 0) {
            nTemp = qMin((qint64)READWRITE_BUFFER_SIZE, nSize);

            if (read_array(nOffset, pBuffer, nTemp) != nTemp) {
                pPdStruct->sInfoString = tr("Read error");

                nResult = 0;

                break;
            }

            nResult = _getCRC32(pBuffer, nTemp, nResult, XBinary::_getCRC32Table_EDB88320());

            nSize -= nTemp;
            nOffset += nTemp;

            XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, nOffset);
        }

        delete[] pBuffer;
    }

    nResult ^= 0xFFFFFFFF;

    XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);

    if (pPdStruct->bIsStop) {
        nResult = 0;
    }

    return nResult;
}

quint16 XBinary::_getCRC16(qint64 nOffset, qint64 nSize, quint16 nInit, PDSTRUCT *pPdStruct)
{
    quint16 nResult = nInit;

    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    OFFSETSIZE osRegion = convertOffsetAndSize(nOffset, nSize);

    nOffset = osRegion.nOffset;
    nSize = osRegion.nSize;

    qint32 _nFreeIndex = XBinary::getFreeIndex(pPdStruct);

    if ((nOffset != -1) && (!(pPdStruct->bIsStop))) {
        XBinary::setPdStructInit(pPdStruct, _nFreeIndex, nSize);

        qint64 nTemp = 0;
        char *pBuffer = new char[READWRITE_BUFFER_SIZE];

        while (nSize > 0) {
            nTemp = qMin((qint64)READWRITE_BUFFER_SIZE, nSize);

            if (read_array(nOffset, pBuffer, nTemp) != nTemp) {
                pPdStruct->sInfoString = tr("Read error");

                nResult = 0;

                break;
            }

            nResult = _getCRC16(pBuffer, nTemp, nResult, XBinary::_getCRC16Table());

            nSize -= nTemp;
            nOffset += nTemp;

            XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, nOffset);
        }

        delete[] pBuffer;
    }

    nResult ^= 0xFFFF;

    XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);

    if (pPdStruct->bIsStop) {
        nResult = 0;
    }

    return nResult;
}

quint32 XBinary::_getCRC32ByFileContent(const QString &sFileName)
{
    return _getCRC32(readFile(sFileName), 0xFFFFFFFF, _getCRC32Table_EDB88320());
}

quint32 XBinary::_getCRC32ByDirectory(const QString &sDirectoryName, bool bRecursive, quint32 nInit)
{
    quint32 nResult = nInit;

    QDirIterator it(sDirectoryName);

    while (it.hasNext()) {
        QString sRecord = it.next();

        QFileInfo fi(sRecord);

        if (fi.isDir() && bRecursive) {
            nResult = _getCRC32ByDirectory(fi.absoluteFilePath(), bRecursive, nResult);
        }

        QByteArray baFileName = fi.baseName().toUtf8();
        nResult = _getCRC32(baFileName, nResult, _getCRC32Table_EDB88320());
    }

    return nResult;
}

double XBinary::getEntropy(const QString &sFileName, PDSTRUCT *pPdStruct)
{
    double dResult = 0;

    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        dResult = XBinary::getEntropy(&file, pPdStruct);

        file.close();
    }

    return dResult;
}

double XBinary::getEntropy(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    double dResult = 0;

    XBinary binary(pDevice);

    dResult = binary.getBinaryStatus(BSTATUS_ENTROPY, 0, -1, pPdStruct);

    pDevice->reset();

    return dResult;
}

double XBinary::getBinaryStatus(BSTATUS bstatus, qint64 nOffset, qint64 nSize, PDSTRUCT *pPdStruct)
{
    double dResult = 0;

    if (bstatus == BSTATUS_ENTROPY) {
        dResult = 1.4426950408889634073599246810023;
    }

    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    OFFSETSIZE osRegion = convertOffsetAndSize(nOffset, nSize);

    nOffset = osRegion.nOffset;
    nSize = osRegion.nSize;

    if (nSize == 0) {
        dResult = 0;
    }

    bool bReadError = false;

    qint32 _nFreeIndex = XBinary::getFreeIndex(pPdStruct);

    if ((nOffset != -1) && (!(pPdStruct->bIsStop))) {
        XBinary::setPdStructInit(pPdStruct, _nFreeIndex, nSize);

        double bytes[256] = {0.0};
        qint64 nSymbolCount = 0;
        quint64 nSum = 0;

        qint64 nTemp = 0;
        char *pBuffer = new char[READWRITE_BUFFER_SIZE];

        while ((nSize > 0) && (!(pPdStruct->bIsStop))) {
            nTemp = qMin((qint64)READWRITE_BUFFER_SIZE, nSize);

            if (read_array(nOffset, pBuffer, nTemp) != nTemp) {
                pPdStruct->sInfoString = tr("Read error");
                bReadError = true;

                break;
            }

            if (bstatus == BSTATUS_ENTROPY) {
                for (qint64 i = 0; i < nTemp; i++) {
                    bytes[(unsigned char)pBuffer[i]] += 1;
                }
            } else if (bstatus == BSTATUS_ZEROS) {
                for (qint64 i = 0; i < nTemp; i++) {
                    if (pBuffer[i] == 0) {
                        nSymbolCount++;
                    }
                }
            } else if (bstatus == BSTATUS_GRADIENT) {
                for (qint64 i = 0; i < nTemp; i++) {
                    nSum += (quint8)pBuffer[i];
                }
            } else if (bstatus == BSTATUS_TEXT) {
                for (qint64 i = 0; i < nTemp; i++) {
                    char _bchar = pBuffer[i];
                    QChar _char(_bchar);
                    if (_char.isPrint() || (_bchar == 8) || (_bchar == 10) || (_bchar == 13)) {
                        nSymbolCount++;
                    }
                }
            }

            nSize -= nTemp;
            nOffset += nTemp;

            XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, nOffset - osRegion.nOffset);
        }

        delete[] pBuffer;

        if ((!(pPdStruct->bIsStop)) && (!bReadError)) {
            if (bstatus == BSTATUS_ENTROPY) {
                for (qint32 j = 0; j < 256; j++) {
                    double dTemp = bytes[j] / (double)osRegion.nSize;

                    if (dTemp) {
                        dResult += (-log(dTemp) / log((double)2)) * bytes[j];
                    }
                }

                dResult = dResult / (double)osRegion.nSize;
            } else if (bstatus == BSTATUS_ZEROS) {
                dResult = (double)nSymbolCount / (double)(osRegion.nSize);
            } else if (bstatus == BSTATUS_GRADIENT) {
                dResult = (double)nSum / ((double)(osRegion.nSize) * (double)0xFF);
            } else if (bstatus == BSTATUS_TEXT) {
                dResult = (double)nSymbolCount / (double)(osRegion.nSize);
            }
        }
    }

    XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);

    if (pPdStruct->bIsStop) {
        dResult = 0;
    }

    return dResult;
}

XBinary::BYTE_COUNTS XBinary::getByteCounts(qint64 nOffset, qint64 nSize, PDSTRUCT *pPdStruct)
{
    BYTE_COUNTS result = {};

    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    OFFSETSIZE osRegion = convertOffsetAndSize(nOffset, nSize);

    nOffset = osRegion.nOffset;
    nSize = osRegion.nSize;

    bool bReadError = false;

    qint32 _nFreeIndex = XBinary::getFreeIndex(pPdStruct);

    if ((nOffset != -1) && (!(pPdStruct->bIsStop))) {
        result.nSize = nSize;

        XBinary::setPdStructInit(pPdStruct, _nFreeIndex, nSize);

        qint64 nTemp = 0;
        char *pBuffer = new char[READWRITE_BUFFER_SIZE];

        while ((nSize > 0) && (!(pPdStruct->bIsStop))) {
            nTemp = qMin((qint64)READWRITE_BUFFER_SIZE, nSize);

            if (read_array(nOffset, pBuffer, nTemp) != nTemp) {
                _errorMessage(tr("Read error"));

                bReadError = true;

                break;
            }

            for (qint64 i = 0; i < nTemp; i++) {
                result.nCount[(unsigned char)pBuffer[i]] += 1;
            }

            nSize -= nTemp;
            nOffset += nTemp;

            XBinary::setPdStructCurrent(pPdStruct, _nFreeIndex, nOffset - osRegion.nOffset);
        }

        delete[] pBuffer;
    }

    XBinary::setPdStructFinished(pPdStruct, _nFreeIndex);

    if ((pPdStruct->bIsStop) || (bReadError)) {
        result = BYTE_COUNTS();
    }

    return result;
}

void XBinary::_xor(quint8 nXorValue, qint64 nOffset, qint64 nSize, PDSTRUCT *pPdStruct)
{
    // TODO Optimize
    OFFSETSIZE osRegion = convertOffsetAndSize(nOffset, nSize);

    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    nOffset = osRegion.nOffset;
    nSize = osRegion.nSize;

    if (nOffset != -1) {
        if (nOffset != -1) {
            qint64 nTemp = 0;
            char *pBuffer = new char[READWRITE_BUFFER_SIZE];

            while ((nSize > 0) && (!(pPdStruct->bIsStop))) {
                nTemp = qMin((qint64)READWRITE_BUFFER_SIZE, nSize);

                if (read_array(nOffset, pBuffer, nTemp) != nTemp) {
                    _errorMessage(tr("Read error"));
                    break;
                }

                for (qint32 i = 0; i < nTemp; i++) {
                    *(pBuffer + i) ^= nXorValue;
                }

                if (!write_array(nOffset, pBuffer, nTemp)) {
                    _errorMessage(tr("Write error"));
                    break;
                }

                nSize -= nTemp;
                nOffset += nTemp;
            }

            delete[] pBuffer;
        }
    }
}

// quint32 XBinary::_ror32(quint32 nValue, quint32 nShift)
//{
//     // TODO Check
//     nShift&=(31);
//     return (nValue>>nShift)|(nValue<<((-nShift)&31));
// }

// quint32 XBinary::_rol32(quint32 nValue, quint32 nShift)
//{
//     // TODO Check
//     nShift&=(31);
//     return (nValue<<nShift)|(nValue>>((-nShift)&31));
// }

quint32 XBinary::getStringCustomCRC32(const QString &sString)
{
    quint32 nResult = 0;  // not ~0 !!! if ~0 (0xFFFFFFFF) it will be a CRC32C

    qint32 nSize = sString.size();
    QByteArray baString = sString.toUtf8();

    for (qint32 i = 0; i < nSize; i++) {
        unsigned char _char = (unsigned char)baString.data()[i];
        //        unsigned char _char1=(unsigned char)sString.at(i).toLatin1();

        //        if(_char!=_char1)
        //        {
        //            qFatal("Error"); // TODO remove
        //        }

        nResult ^= _char;

        for (qint32 k = 0; k < 8; k++) {
            nResult = (nResult & 1) ? ((nResult >> 1) ^ 0x82f63b78) : (nResult >> 1);
        }
    }

    nResult = ~nResult;

    return nResult;
}

QIODevice *XBinary::getDevice()
{
    return g_pDevice;
}

bool XBinary::isValid(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    return true;
}

bool XBinary::isValid(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    XBinary xbinary(pDevice, bIsImage, nModuleAddress);

    return xbinary.isValid();
}

XBinary::MODE XBinary::getMode(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    XBinary xbinary(pDevice, bIsImage, nModuleAddress);

    return xbinary.getMode();
}

bool XBinary::isBigEndian()
{
    return (getEndian() == ENDIAN_BIG);
}

bool XBinary::is16()
{
    MODE mode = getMode();

    return ((mode == MODE_16) || (mode == MODE_16SEG));
}

bool XBinary::is32()
{
    MODE mode = getMode();

    return (mode == MODE_32);
}

bool XBinary::is64()
{
    MODE mode = getMode();

    return (mode == MODE_64);
}

bool XBinary::isBigEndian(XBinary::_MEMORY_MAP *pMemoryMap)
{
    return (pMemoryMap->endian == ENDIAN_BIG);
}

bool XBinary::is16(XBinary::_MEMORY_MAP *pMemoryMap)
{
    return (pMemoryMap->mode == MODE_16) || (pMemoryMap->mode == MODE_16SEG);
}

bool XBinary::is32(XBinary::_MEMORY_MAP *pMemoryMap)
{
    return (pMemoryMap->mode == MODE_32);
}

bool XBinary::is64(XBinary::_MEMORY_MAP *pMemoryMap)
{
    return (pMemoryMap->mode == MODE_64);
}

void XBinary::setVersion(const QString &sVersion)
{
    g_sVersion = sVersion;
}

void XBinary::setOptions(const QString &sOptions)
{
    g_sOptions = sOptions;
}

QString XBinary::getVersion()
{
    return g_sVersion;
}

QString XBinary::getOptions()
{
    return g_sOptions;
}

bool XBinary::isEncrypted()
{
    return false;
}

QString XBinary::getSignature(QIODevice *pDevice, qint64 nOffset, qint64 nSize)
{
    XBinary binary(pDevice);

    return binary.getSignature(nOffset, nSize);
}

QString XBinary::getSignature(qint64 nOffset, qint64 nSize)
{
    QString sResult;

    if (nOffset != -1) {
        OFFSETSIZE osRegion = convertOffsetAndSize(nOffset, -1);

        nSize = qMin(osRegion.nSize, nSize);

        sResult = read_array(nOffset, nSize).toHex().toUpper();
    }

    return sResult;
}

XBinary::OFFSETSIZE XBinary::convertOffsetAndSize(qint64 nOffset, qint64 nSize)
{
    OFFSETSIZE osResult = {};

    osResult.nOffset = -1;
    osResult.nSize = 0;

    qint64 nTotalSize = getSize();

    if ((nSize == -1) && (nTotalSize > nOffset)) {
        nSize = nTotalSize - nOffset;
    }

    if ((nOffset + nSize > nTotalSize) && (nOffset < nTotalSize)) {
        nSize = nTotalSize - nOffset;
    }

    if ((nSize > 0) && (nOffset >= 0) && (nOffset < nTotalSize) && (nOffset + nSize - 1 < nTotalSize)) {
        osResult.nOffset = nOffset;
        osResult.nSize = nSize;
    }

    return osResult;
}

XBinary::OFFSETSIZE XBinary::convertOffsetAndSize(QIODevice *pDevice, qint64 nOffset, qint64 nSize)
{
    XBinary binary(pDevice);

    return binary.convertOffsetAndSize(nOffset, nSize);
}

bool XBinary::compareSignatureStrings(const QString &sBaseSignature, const QString &sOptSignature)
{
    bool bResult = false;
    // TODO optimize
    // TODO check
    QString _sBaseSignature = convertSignature(sBaseSignature);
    QString _sOptSignature = convertSignature(sOptSignature);

    qint32 nSize = qMin(_sBaseSignature.size(), _sOptSignature.size());

    if ((nSize) && (_sBaseSignature.size() >= _sOptSignature.size())) {
        bResult = true;

        for (qint32 i = 0; i < nSize; i++) {
            QChar _qchar1 = _sBaseSignature.at(i);
            QChar _qchar2 = _sOptSignature.at(i);

            if ((_qchar1 != QChar('.')) && (_qchar2 != QChar('.'))) {
                if (_qchar1 != _qchar2) {
                    bResult = false;

                    break;
                }
            }
        }
    }

    return bResult;
}

void XBinary::_errorMessage(const QString &sErrorMessage)
{
#ifdef QT_DEBUG
    QFile *pFile = dynamic_cast<QFile *>(g_pDevice);

    if (pFile) {
        qDebug("Filename: %s", pFile->fileName().toUtf8().data());
    }

    qDebug("Error: %s", sErrorMessage.toLatin1().data());
#endif
    emit errorMessage(sErrorMessage);
}

void XBinary::_infoMessage(const QString &sInfoMessage)
{
#ifdef QT_DEBUG
    qDebug("Info: %s", sInfoMessage.toLatin1().data());
#endif
    emit infoMessage(sInfoMessage);
}

qint64 XBinary::_calculateRawSize(PDSTRUCT *pPdStruct)
{
    _MEMORY_MAP memoryMap = getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);

    return _calculateRawSize(&memoryMap, pPdStruct);
}

qint64 XBinary::_calculateRawSize(XBinary::_MEMORY_MAP *pMemoryMap, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    qint64 nResult = 0;

    qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

    qint64 _nOverlayOffset = -1;

    for (qint32 i = 0; (i < nNumberOfRecords) && (!(pPdStruct->bIsStop)); i++) {
        if ((pMemoryMap->listRecords.at(i).nOffset != -1) && (pMemoryMap->listRecords.at(i).type != MMT_OVERLAY)) {
            nResult = qMax(nResult, (qint64)(pMemoryMap->listRecords.at(i).nOffset + pMemoryMap->listRecords.at(i).nSize));
        }

        if (pMemoryMap->listRecords.at(i).type == MMT_OVERLAY) {
            _nOverlayOffset = pMemoryMap->listRecords.at(i).nOffset;
        }
    }

    if (_nOverlayOffset != -1) {
        nResult = qMin(nResult, _nOverlayOffset);
    }

    return nResult;
}

QString XBinary::convertSignature(const QString &sSignature)
{
    QString _sSignature = sSignature;
    // 'AnsiString'
    // TODO more defs
    if (_sSignature.contains(QChar(39))) {
        QString sTemp;
        qint32 nStringSize = _sSignature.size();
        bool bAnsiString = false;

        for (qint32 i = 0; i < nStringSize; i++) {
            QChar c = _sSignature.at(i);

            if (c == QChar(39)) {
                bAnsiString = !bAnsiString;
            } else if (bAnsiString) {
                sTemp.append(qcharToHex(c));
            } else {
                sTemp.append(c);
            }
        }

        _sSignature = sTemp;
    }

    if (_sSignature.contains(QChar(' '))) {
        _sSignature = _sSignature.remove(QChar(' '));
    }

    if (_sSignature.contains(QChar('?'))) {
        _sSignature = _sSignature.replace(QChar('?'), QChar('.'));
    }

    _sSignature = _sSignature.toLower();

    return _sSignature;
}

bool XBinary::isDebugBuild()
{
    return false;
}

bool XBinary::isReleaseBuild()
{
    return true;
}

QList<QString> XBinary::getFileFormatMessages(const QList<FMT_MSG> *pListFmtMsg)
{
    QList<QString> listResult;

    if (pListFmtMsg) {
        qint32 nNumberOfRecords = pListFmtMsg->count();

        for (qint32 i = 0; i < nNumberOfRecords; i++) {
            QString sRecord;

            if (pListFmtMsg->at(i).type == FMT_MSG_TYPE_INFO) sRecord += QString("[%1]").arg(tr("Info"));
            else if (pListFmtMsg->at(i).type == FMT_MSG_TYPE_WARNING) sRecord += QString("[%1]").arg(tr("Warning"));
            else if (pListFmtMsg->at(i).type == FMT_MSG_TYPE_ERROR) sRecord += QString("[%1]").arg(tr("Error"));

            sRecord += QString("(%1) ").arg(pListFmtMsg->at(i).code, 4, 16, QChar('0'));

            sRecord += pListFmtMsg->at(i).sString;
            listResult.append(sRecord);
        }
    }

    return listResult;
}

bool XBinary::isFmtMsgCodePresent(const QList<FMT_MSG> *pListFmtMsgs, FMT_MSG_CODE code, FMT_MSG_TYPE type, PDSTRUCT *pPdStruct)
{
    bool bResult = false;

    if (pListFmtMsgs) {
        qint32 nNumberOfRecords = pListFmtMsgs->count();

        for (qint32 i = 0; (i < nNumberOfRecords) && (!(pPdStruct->bIsStop)); i++) {
            if ((pListFmtMsgs->at(i).code == code) && (pListFmtMsgs->at(i).type == type)) {
                bResult = true;
                break;
            }
        }
    }

    return bResult;
}

bool XBinary::_addCheckFormatTest(QList<FMT_MSG> *pListFmtMsgs, bool *pbContinue, FMT_MSG_CODE code, FMT_MSG_TYPE type, const QString &sString, QVariant value,
                                  QString sInfo, bool bFailCase)
{
    bool bResult = !bFailCase;

    if (*pbContinue) {
        if (bFailCase) {
            FMT_MSG record = {};
            record.type = type;
            record.code = code;
            record.sString += QString("%1: %2: %3").arg(sString, tr("Corrupted data"), sInfo);
            record.value = value;

            pListFmtMsgs->append(record);

            if (type == FMT_MSG_TYPE_ERROR) {
                *pbContinue = false;
            }
        }
    }

    return bResult;
}

QList<XBinary::FMT_MSG> XBinary::checkFileFormat(bool bDeep, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(bDeep)
    Q_UNUSED(pPdStruct)

    QList<XBinary::FMT_MSG> listResult;

    return listResult;
}

bool XBinary::isFileFormatValid(bool bDeep, PDSTRUCT *pPdStruct)
{
    bool bResult = true;

    QList<FMT_MSG> list = checkFileFormat(bDeep, pPdStruct);

    qint32 nNumberOfRecords = list.count();

    for (qint32 i = 0; (i < nNumberOfRecords) && (!(pPdStruct->bIsStop)); i++) {
        if (list.at(i).type == FMT_MSG_TYPE_ERROR) {
            bResult = false;
            break;
        }
    }

    return bResult;
}

QList<XBinary::STRINGTABLE_RECORD> XBinary::getStringTable_ANSI(qint64 nOffset, qint64 nSize, PDSTRUCT *pPdStruct)
{
    QList<XBinary::STRINGTABLE_RECORD> listResult;

    QByteArray baStringTable = read_array(nOffset, nSize, pPdStruct);

    char *_pOffset = baStringTable.data();
    qint32 _nSize = baStringTable.size();

    // TODO UTF8
    for (qint32 i = 0; i < _nSize; i++) {
        XBinary::STRINGTABLE_RECORD record = {};

        record.nOffsetFromStart = i;
        record.sString = _pOffset;  // TODO
        record.nSizeInBytes = record.sString.size();

        listResult.append(record);

        _pOffset += (record.nSizeInBytes + 1);
        i += record.nSizeInBytes;
    }

    return listResult;
}

QString XBinary::qcharToHex(QChar cSymbol)
{
    // TODO mb
    return QString("%1").arg((quint8)(cSymbol.toLatin1()), 2, 16, QChar('0'));
}

QString XBinary::stringToHex(const QString &sString)
{
    return QString::fromLatin1(sString.toLatin1().toHex());
}

QString XBinary::hexToString(const QString &sHex)
{
    QString sResult;

    sResult = QByteArray::fromHex(sHex.toLatin1().data());

    return sResult;
}

QString XBinary::floatToString(float fValue, qint32 nPrec)
{
    return QString("%1").arg(fValue, 0, 'f', nPrec);
}

QString XBinary::doubleToString(double dValue, qint32 nPrec)
{
    return QString("%1").arg(dValue, 0, 'f', nPrec);
}

quint8 XBinary::hexToUint8(const QString &sHex)
{
    QString _sHex = sHex;
    quint8 nResult = 0;

    if ((quint32)_sHex.length() >= sizeof(quint8)) {
        _sHex = _sHex.mid(0, 2 * sizeof(quint8));
        bool bStatus = false;
        nResult = (quint8)(_sHex.toInt(&bStatus, 16));
    }

    return nResult;
}

qint8 XBinary::hexToInt8(const QString &sHex)
{
    QString _sHex = sHex;
    quint8 nResult = 0;

    if ((quint32)_sHex.length() >= sizeof(qint8)) {
        _sHex = _sHex.mid(0, 2 * sizeof(qint8));
        bool bStatus = false;
        nResult = (qint8)(_sHex.toInt(&bStatus, 16));
    }

    return nResult;
}

quint16 XBinary::hexToUint16(const QString &sHex, bool bIsBigEndian)
{
    QString _sHex = sHex;
    quint16 nResult = 0;

    if ((quint32)_sHex.length() >= sizeof(quint16)) {
        if (!bIsBigEndian) {
            _sHex = invertHexByteString(_sHex.mid(0, 2 * sizeof(quint16)));
        }

        bool bStatus = false;
        nResult = _sHex.toUShort(&bStatus, 16);
    }

    return nResult;
}

qint16 XBinary::hexToInt16(const QString &sHex, bool bIsBigEndian)
{
    QString _sHex = sHex;
    qint16 nResult = 0;

    if ((quint32)_sHex.length() >= sizeof(qint16)) {
        if (!bIsBigEndian) {
            _sHex = invertHexByteString(_sHex.mid(0, 2 * sizeof(qint16)));
        }

        bool bStatus = false;
        nResult = _sHex.toShort(&bStatus, 16);
    }

    return nResult;
}

quint32 XBinary::hexToUint32(const QString &sHex, bool bIsBigEndian)
{
    QString _sHex = sHex;
    quint32 nResult = 0;

    if ((quint32)_sHex.length() >= sizeof(quint32)) {
        if (!bIsBigEndian) {
            _sHex = invertHexByteString(_sHex.mid(0, 2 * sizeof(quint32)));
        }

        bool bStatus = false;
        nResult = _sHex.toUInt(&bStatus, 16);
    }

    return nResult;
}

qint32 XBinary::hexToInt32(const QString &sHex, bool bIsBigEndian)
{
    QString _sHex = sHex;
    qint32 nResult = 0;

    if ((quint32)_sHex.length() >= sizeof(qint32)) {
        if (!bIsBigEndian) {
            _sHex = invertHexByteString(_sHex.mid(0, 2 * sizeof(qint32)));
        }

        bool bStatus = false;
        nResult = _sHex.toInt(&bStatus, 16);
    }

    return nResult;
}

quint64 XBinary::hexToUint64(const QString &sHex, bool bIsBigEndian)
{
    QString _sHex = sHex;
    quint64 nResult = 0;

    if ((quint32)_sHex.length() >= sizeof(quint64)) {
        if (!bIsBigEndian) {
            _sHex = invertHexByteString(_sHex.mid(0, 2 * sizeof(quint64)));
        }

        bool bStatus = false;
        nResult = _sHex.toULongLong(&bStatus, 16);
    }

    return nResult;
}

qint64 XBinary::hexToInt64(const QString &sHex, bool bIsBigEndian)
{
    QString _sHex = sHex;
    qint64 nResult = 0;

    if ((quint32)_sHex.length() >= sizeof(qint64)) {
        if (!bIsBigEndian) {
            _sHex = invertHexByteString(_sHex.mid(0, 2 * sizeof(qint64)));
        }

        bool bStatus = false;
        nResult = _sHex.toLongLong(&bStatus, 16);
    }

    return nResult;
}

QString XBinary::invertHexByteString(const QString &sHex)
{
    QString sResult;

    for (qint32 i = sHex.length() - 2; i >= 0; i -= 2) {
        sResult += sHex.mid(i, 2);
    }

    return sResult;
}

void XBinary::_swapBytes(char *pSource, qint32 nSize)
{
    for (qint32 i = 0; i < (nSize / 2); i++) {
        char cTemp = pSource[i];
        pSource[i] = pSource[(nSize - 1) - i];
        pSource[(nSize - 1) - i] = cTemp;
    }
}

quint16 XBinary::swapBytes(quint16 nValue)
{
    _swapBytes((char *)&nValue, 2);

    return nValue;
}

quint32 XBinary::swapBytes(quint32 nValue)
{
    _swapBytes((char *)&nValue, 4);

    return nValue;
}

quint64 XBinary::swapBytes(quint64 nValue)
{
    _swapBytes((char *)&nValue, 8);

    return nValue;
}

bool XBinary::isPlainTextType()
{
    QByteArray baData = read_array(0, qMin(getSize(), (qint64)0x2000));

    return isPlainTextType(&baData);
}

bool XBinary::isPlainTextType(QByteArray *pbaData)
{
    bool bResult = false;

    unsigned char *pDataOffset = (unsigned char *)(pbaData->data());
    qint32 nDataSize = pbaData->size();

    if (nDataSize) {
        bResult = true;

        for (qint32 i = 0; i < nDataSize; i++) {
            if (pDataOffset[i] < 0x9) {
                bResult = false;
                break;
            }
        }
    }

    return bResult;
}

bool XBinary::isUTF8TextType()
{
    QByteArray baData = read_array(0, qMin(getSize(), (qint64)0x100));

    return isUTF8TextType(&baData);
}

bool XBinary::isUTF8TextType(QByteArray *pbaData)
{
    // EFBBBF
    bool bResult = false;

    unsigned char *pDataOffset = (unsigned char *)(pbaData->data());
    qint32 nDataSize = pbaData->size();

    if (nDataSize >= 3) {
        if ((pDataOffset[0] == 0xEF) && (pDataOffset[1] == 0xBB) && (pDataOffset[2] == 0xBF)) {
            bResult = true;
        }
    }

    if (bResult) {
        unsigned char *pDataOffset = (unsigned char *)(pbaData->data());
        pDataOffset += 3;

        nDataSize = pbaData->size();
        nDataSize = nDataSize - 3;

        for (qint32 i = 0; i < nDataSize; i++) {
            if (pDataOffset[i] == 0) {
                bResult = false;
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
    QByteArray baData = read_array(0, qMin(getSize(), (qint64)0x2));

    return getUnicodeType(&baData);
}

XBinary::UNICODE_TYPE XBinary::getUnicodeType(QByteArray *pbaData)
{
    XBinary::UNICODE_TYPE result = XBinary::UNICODE_TYPE_NONE;

    unsigned char *pDataOffset = (unsigned char *)(pbaData->data());
    qint32 nDataSize = pbaData->size();

    if (nDataSize) {
        quint16 nSymbol = *((quint16 *)(pDataOffset));

        nSymbol = qFromLittleEndian(nSymbol);

        if (nSymbol == 0xFFFE) {
            result = UNICODE_TYPE_BE;
        } else if (nSymbol == 0xFEFF) {
            result = UNICODE_TYPE_LE;
        } else {
            result = UNICODE_TYPE_NONE;
        }
    }
    // TODO 0 end

    return result;
}

bool XBinary::tryToOpen(QIODevice *pDevice)
{
    bool bResult = false;
    bool bCheck = false;

    QFile *pFile = dynamic_cast<QFile *>(pDevice);

    if (pFile) {
        bCheck = (pFile->fileName() != "");
    } else {
        bCheck = true;
    }

    if (bCheck) {
        bResult = pDevice->open(QIODevice::ReadWrite);

        if (!bResult) {
            bResult = pDevice->open(QIODevice::ReadOnly);
        }
    }

    return bResult;
}

bool XBinary::checkOffsetSize(XBinary::OFFSETSIZE osRegion)
{
    qint64 nTotalSize = getSize();

    bool bOffsetValid = (osRegion.nOffset >= 0) && (osRegion.nOffset < nTotalSize);
    bool bSizeValid = (osRegion.nSize > 0) && ((osRegion.nOffset + osRegion.nSize - 1) < nTotalSize);

    return (bOffsetValid) && (bSizeValid);
}

bool XBinary::checkOffsetSize(qint64 nOffset, qint64 nSize)
{
    XBinary::OFFSETSIZE os = {};
    os.nOffset = nOffset;
    os.nSize = nSize;

    return checkOffsetSize(os);
}

QString XBinary::get_uint8_full_version(quint8 nValue)
{
    return QString("%1").arg(QString::number((nValue)&0xFF));
}

QString XBinary::get_uint16_full_version(quint16 nValue)
{
    return QString("%1.%2").arg(QString::number((nValue >> 8) & 0xFF), QString::number((nValue)&0xFF));
}

QString XBinary::get_uint32_full_version(quint32 nValue)
{
    return QString("%1.%2.%3").arg(QString::number((nValue >> 16) & 0xFFFF), QString::number((nValue >> 8) & 0xFF), QString::number((nValue)&0xFF));
}

QString XBinary::get_uint64_full_version(quint64 nValue)
{
    QString sResult;

    quint32 nValue1 = (nValue >> 32) & 0xFFFFFFFF;
    quint32 nValue2 = nValue & 0xFFFFFFFF;

    sResult = QString("%1.%2").arg(get_uint32_full_version(nValue1), get_uint32_full_version(nValue2));

    return sResult;
}

QString XBinary::get_uint16_version(quint16 nValue)
{
    return QString("%1").arg(QString::number((nValue)&0xFFFF));
}

QString XBinary::get_uint32_version(quint32 nValue)
{
    return QString("%1.%2").arg(QString::number((nValue >> 16) & 0xFFFF), QString::number((nValue)&0xFFFF));
}

bool XBinary::isResizeEnable(QIODevice *pDevice)
{
    // mb TODO casr
    bool bResult = false;

    QString sClassName = pDevice->metaObject()->className();

    if (sClassName == "QFile") {
        bResult = true;
    } else if (sClassName == "QBuffer") {
        bResult = true;
    } else if (sClassName == "QTemporaryFile") {
        bResult = true;
    }

    return bResult;
}

bool XBinary::resize(QIODevice *pDevice, qint64 nSize)
{
    bool bResult = false;

    QString sClassName = pDevice->metaObject()->className();  // TODO

    if (sClassName == "QFile") {
        bResult = ((QFile *)pDevice)->resize(nSize);
    } else if (sClassName == "QBuffer") {
        ((QBuffer *)pDevice)->buffer().resize((qint32)nSize);
        bResult = true;
    } else if (sClassName == "QTemporaryFile") {
        bResult = ((QTemporaryFile *)pDevice)->resize(nSize);
    }

    return bResult;
}

XBinary::PACKED_UINT XBinary::read_uleb128(qint64 nOffset, qint64 nSize)
{
    PACKED_UINT result = {};

    quint32 nShift = 0;

    for (qint32 i = 0; i < nSize; i++) {
        quint8 nByte = read_uint8(nOffset + i);
        result.nValue |= ((nByte & 0x7F) << nShift);
        result.nByteSize++;
        nShift += 7;

        if ((nByte & 0x80) == 0) {
            result.bIsValid = true;
            break;
        }
    }

    return result;
}

XBinary::PACKED_UINT XBinary::_read_uleb128(char *pData, qint64 nSize)
{
    PACKED_UINT result = {};

    quint32 nShift = 0;

    for (qint32 i = 0; i < nSize; i++) {
        quint8 nByte = (quint8)(*(pData + i));
        result.nValue |= ((nByte & 0x7F) << nShift);
        result.nByteSize++;
        nShift += 7;

        if ((nByte & 0x80) == 0) {
            result.bIsValid = true;
            break;
        }
    }

    return result;
}

XBinary::PACKED_UINT XBinary::read_acn1_integer(qint64 nOffset, qint64 nSize)
{
    PACKED_UINT result = {};

    if (nSize > 0) {
        quint8 nByte = read_uint8(nOffset);

        if ((nByte & 0x80) == 0) {
            result.bIsValid = true;
            result.nByteSize = 1;
            result.nValue = nByte;
        } else {
            quint8 _nSize = (nByte & 0x7F);

            if ((_nSize <= 4) && (_nSize <= nSize)) {
                result.bIsValid = true;
                result.nByteSize = 1 + _nSize;

                for (qint32 i = 0; i < _nSize; i++) {
                    result.nValue <<= 8;
                    result.nValue |= read_uint8(nOffset + 1 + i);
                }
            }
        }
    }

    return result;
}

XBinary::PACKED_UINT XBinary::_read_packedNumber(char *pData, qint64 nSize)
{
    PACKED_UINT result = {};

    quint8 nFirstByte = (quint8)(*(pData));

    result.nByteSize = 0;

    if ((nFirstByte & 0x80) == 0) {  // 0xxxxxxx
        result.nValue = nFirstByte & 0x7F;
        result.nByteSize = 1;
    } else if ((nFirstByte & 0xC0) == 0x80) {  // 10xxxxxx
        result.nValue = nFirstByte & 0x3F;
        result.nByteSize = 2;
    } else if ((nFirstByte & 0xE0) == 0xC0) {  // 110xxxxx
        result.nValue = nFirstByte & 0x1F;
        result.nByteSize = 3;
    } else if ((nFirstByte & 0xF0) == 0xE0) {  // 1110xxxx
        result.nValue = nFirstByte & 0x0F;
        result.nByteSize = 4;
    } else if ((nFirstByte & 0xF8) == 0xF0) {  // 11110xxx
        result.nValue = nFirstByte & 0x07;
        result.nByteSize = 5;
    } else if ((nFirstByte & 0xFC) == 0xF8) {  // 111110xx
        result.nValue = nFirstByte & 0x03;
        result.nByteSize = 6;
    } else if ((nFirstByte & 0xFE) == 0xFC) {  // 1111110x
        result.nValue = nFirstByte & 0x01;
        result.nByteSize = 7;
    } else if (nFirstByte == 0xFE) {  // 11111110
        result.nValue = 0;
        result.nByteSize = 8;
    } else if (nFirstByte == 0xFF) {  // 11111111
        result.nValue = 0;
        result.nByteSize = 9;
    }

    if (result.nByteSize <= nSize) {
        result.bIsValid = true;

        for (qint32 i = 1; i < result.nByteSize; ++i) {
            quint8 _nByte = (quint8)(*(pData + i));
            result.nValue |= static_cast<quint32>(_nByte) << (8 * (i - 1));
        }
    }

    return result;
}

QList<QString> XBinary::getListFromFile(const QString &sFileName)
{
    QList<QString> listResult;

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString sLine = in.readLine().trimmed();
            if (sLine != "") {
                listResult.append(sLine);
            }
        }

        file.close();
    }

    return listResult;
}

qint64 XBinary::getOverlaySize(PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    _MEMORY_MAP memoryMap = getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);

    return getOverlaySize(&memoryMap, pPdStruct);
}

qint64 XBinary::getOverlaySize(XBinary::_MEMORY_MAP *pMemoryMap, PDSTRUCT *pPdStruct)
{
    qint64 nSize = pMemoryMap->nBinarySize;
    qint64 nOverlayOffset = getOverlayOffset(pMemoryMap, pPdStruct);
    qint64 nDelta = 0;

    if (nOverlayOffset > 0) {
        nDelta = nSize - nOverlayOffset;
    }

    return qMax(nDelta, (qint64)0);
}

qint64 XBinary::getOverlayOffset(PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    _MEMORY_MAP memoryMap = getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);

    return getOverlayOffset(&memoryMap, pPdStruct);
}

qint64 XBinary::getOverlayOffset(XBinary::_MEMORY_MAP *pMemoryMap, PDSTRUCT *pPdStruct)
{
    qint64 nResult = -1;
    qint64 nRawSize = _calculateRawSize(pMemoryMap, pPdStruct);

    if (nRawSize) {
        nResult = nRawSize;
    }

    return nResult;
}

bool XBinary::isOverlayPresent(PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    _MEMORY_MAP memoryMap = getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);

    return isOverlayPresent(&memoryMap, pPdStruct);
}

bool XBinary::isOverlayPresent(XBinary::_MEMORY_MAP *pMemoryMap, PDSTRUCT *pPdStruct)
{
    return (getOverlaySize(pMemoryMap, pPdStruct) != 0);
}

bool XBinary::compareOverlay(const QString &sSignature, qint64 nOffset, PDSTRUCT *pPdStruct)
{
    _MEMORY_MAP memoryMap = getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);

    return compareOverlay(&memoryMap, sSignature, nOffset, pPdStruct);
}

bool XBinary::compareOverlay(XBinary::_MEMORY_MAP *pMemoryMap, const QString &sSignature, qint64 nOffset, PDSTRUCT *pPdStruct)
{
    bool bResult = false;

    if (isOverlayPresent(pMemoryMap, pPdStruct)) {
        qint64 nOverlayOffset = getOverlayOffset(pMemoryMap, pPdStruct) + nOffset;

        bResult = compareSignature(pMemoryMap, sSignature, nOverlayOffset, pPdStruct);
    }

    return bResult;
}

bool XBinary::addOverlay(char *pData, qint64 nDataSize, PDSTRUCT *pPdStruct)
{
    bool bResult = false;

    qint64 nRawSize = getOverlayOffset(pPdStruct);

    if (resize(getDevice(), nRawSize + nDataSize)) {
        if (nDataSize) {
            write_array(nRawSize, pData, nDataSize);

            bResult = true;
        }
    }

    return bResult;
}

bool XBinary::addOverlay(const QString &sFileName, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    bool bResult = false;

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        qint64 nRawSize = getOverlayOffset(pPdStruct);
        qint64 nDataSize = file.size();

        if (resize(getDevice(), nRawSize + nDataSize)) {
            if (nDataSize) {
                bResult = copyDeviceMemory(&file, 0, getDevice(), nRawSize, nDataSize);
            }
        }

        file.close();
    }

    return bResult;
}

bool XBinary::removeOverlay()
{
    return addOverlay(0, 0);
}

bool XBinary::isSignatureInLoadSegmentPresent(qint32 nLoadSegment, const QString &sSignature)
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return isSignatureInLoadSegmentPresent(&memoryMap, nLoadSegment, sSignature);
}

bool XBinary::isSignatureInLoadSegmentPresent(XBinary::_MEMORY_MAP *pMemoryMap, qint32 nLoadSegment, const QString &sSignature, PDSTRUCT *pPdStruct)
{
    bool bResult = false;

    qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

    for (qint32 i = 0; (i < nNumberOfRecords) && (!pPdStruct->bIsStop); i++) {
        if ((pMemoryMap->listRecords.at(i).type == MMT_LOADSEGMENT) && (pMemoryMap->listRecords.at(i).nLoadSectionNumber == nLoadSegment)) {
            if (pMemoryMap->listRecords.at(i).nOffset != -1) {
                bResult = isSignaturePresent(pMemoryMap, pMemoryMap->listRecords.at(i).nOffset, pMemoryMap->listRecords.at(i).nSize, sSignature, pPdStruct);

                break;
            }
        }
    }

    return bResult;
}

QString XBinary::getStringCollision(QList<QString> *pListStrings, const QString &sString1, const QString &sString2)
{
    // TODO Check&optimize
    QString sResult;

    qint32 nNumberOfStrings = pListStrings->count();

    QString sRoot1;
    QString sRoot2;

    for (qint32 i = 0; i < nNumberOfStrings; i++) {
        QString sCurrentString = pListStrings->at(i);

        if (sCurrentString.contains(sString1)) {
            sRoot1 = sCurrentString.section(sString1, 0, 0);
        }

        if ((sRoot1 != "") && sCurrentString.contains(sString2)) {
            sRoot2 = sCurrentString.section(sString2, 0, 0);

            break;
        }
    }

    if ((sRoot1 != "") && (sRoot1 == sRoot2)) {
        sResult = sRoot1;
    }

    return sResult;
}

bool XBinary::writeToFile(const QString &sFileName, const QByteArray &baData)
{
    bool bResult = false;

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadWrite)) {
        file.resize(0);
        file.write(baData.data(), baData.size());
        file.close();
        bResult = true;
    }

    return bResult;
}

bool XBinary::writeToFile(const QString &sFileName, QIODevice *pDevice)
{
    bool bResult = false;

    if (createFile(sFileName, pDevice->size())) {
        QFile file;
        file.setFileName(sFileName);

        if (file.open(QIODevice::ReadWrite)) {
            bResult = copyDeviceMemory(pDevice, 0, &file, 0, pDevice->size(), READWRITE_BUFFER_SIZE);
            file.close();
        }
    }

    return bResult;
}

bool XBinary::appendToFile(const QString &sFileName, const QString &sString)
{
    QString _sString = sString;
    bool bResult = false;

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadWrite | QIODevice::Append)) {
        _sString += "\r\n";  // TODO Linux
        file.write(_sString.toUtf8());
        file.close();
        bResult = true;
    }

    return bResult;
}

bool XBinary::clearFile(const QString &sFileName)
{
    bool bResult = false;

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadWrite)) {
        file.resize(0);
        file.close();
        bResult = true;
    }

    return bResult;
}

qint32 XBinary::getStringNumberFromList(QList<QString> *pListStrings, const QString &sString, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    qint32 nResult = -1;

    qint32 nNumberOfRecords = pListStrings->count();

    for (qint32 i = 0; (i < nNumberOfRecords) && (!(pPdStruct->bIsStop)); i++) {
        if (pListStrings->at(i) == sString) {
            nResult = i;

            break;
        }
    }

    return nResult;
}

qint32 XBinary::getStringNumberFromListExp(QList<QString> *pListStrings, const QString &sString, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    qint32 nResult = -1;

    qint32 nNumberOfRecords = pListStrings->count();

    for (qint32 i = 0; (i < nNumberOfRecords) && (!(pPdStruct->bIsStop)); i++) {
        if (isRegExpPresent(sString, pListStrings->at(i))) {
            nResult = i;

            break;
        }
    }

    return nResult;
}

bool XBinary::isStringInListPresent(QList<QString> *pListStrings, const QString &sString, PDSTRUCT *pPdStruct)
{
    return (getStringNumberFromList(pListStrings, sString, pPdStruct) != -1);
}

bool XBinary::isStringInListPresentExp(QList<QString> *pListStrings, const QString &sString, PDSTRUCT *pPdStruct)
{
    return (getStringNumberFromListExp(pListStrings, sString, pPdStruct) != -1);
}

QString XBinary::getStringByIndex(QList<QString> *pListStrings, qint32 nIndex, qint32 nNumberOfStrings)
{
    QString sResult;

    if (nNumberOfStrings == -1) {
        nNumberOfStrings = pListStrings->count();
    }

    if ((nIndex > 0) && (nIndex < nNumberOfStrings)) {
        sResult = pListStrings->at(nIndex);
    }

    return sResult;
}

bool XBinary::isStringUnicode(const QString &sString, qint32 nMaxCheckSize)
{
    // TODO Optimize
    // TODO Check
    bool bResult = false;

    if (nMaxCheckSize == -1) {
        nMaxCheckSize = sString.size();
    } else {
        nMaxCheckSize = qMin(sString.size(), nMaxCheckSize);
    }

    for (qint32 i = 0; i < nMaxCheckSize; i++) {
        unsigned char cChar = sString.at(i).toLatin1();
        if ((cChar > 127) || (cChar < 27)) {
            bResult = true;
            break;
        }
    }

    return bResult;
}

quint32 XBinary::elfHash(const quint8 *pData)
{
    quint32 nResult = 0;

    while (*pData) {
        nResult = (nResult << 4) + (*pData);

        quint32 nHigh = nResult & 0xF0000000;

        if (nHigh) {
            nResult ^= (nResult >> 24);
        }

        nResult &= (~nHigh);

        pData++;
    }

    return nResult;
}

QString XBinary::getVersionString(const QString &sString)
{
    QString sResult;

    qint32 nSize = sString.size();

    for (qint32 i = 0; i < nSize; i++) {
        QChar c = sString.at(i);

        if (((QChar('9') >= c) && (c >= QChar('0'))) || (c == QChar('.'))) {
            sResult.append(c);
        } else {
            break;
        }
    }

    return sResult;
}

qint64 XBinary::getVersionIntValue(const QString &sString)
{
    qint64 nResult = 0;

    qint32 nNumberOfDots = sString.count(QChar('.'));

    for (qint32 i = 0; i < (nNumberOfDots + 1); i++) {
        nResult *= 10;

        nResult += sString.section(QChar('.'), i, i).toLongLong();
    }

    return nResult;
}

bool XBinary::checkStringNumber(const QString &sString, quint32 nMin, quint32 nMax)
{
    bool bResult = false;

    quint32 nValue = sString.toUInt();

    bResult = ((nValue >= nMin) && (nValue <= nMax));

    return bResult;
}

QDateTime XBinary::valueToTime(quint64 nValue, DT_TYPE type)
{
    QDateTime result;

    if (type == DT_TYPE_POSIX) {
        result.setMSecsSinceEpoch(nValue * 1000);
    }

    return result;
}

QString XBinary::valueToTimeString(quint64 nValue, XBinary::DT_TYPE type)
{
    QString sResult;

    sResult = valueToTime(nValue, type).toString("yyyy-MM-dd hh:mm:ss");

    return sResult;
}

QString XBinary::msecToDate(quint64 nValue)
{
    QString sResult;

    quint64 _nTmp = nValue / 1000;

    quint64 nSec = _nTmp % 60;
    _nTmp /= 60;
    quint64 nMin = _nTmp % 60;
    _nTmp /= 60;
    quint64 nHour = _nTmp % 24;
    _nTmp /= 24;
    quint64 nDay = _nTmp;

    if (nDay > 0) {
        sResult += QString("%1:").arg(nDay, 2, 10, QChar('0'));
    }

    sResult += QString("%1:").arg(nHour, 2, 10, QChar('0'));
    sResult += QString("%1:").arg(nMin, 2, 10, QChar('0'));
    sResult += QString("%1").arg(nSec, 2, 10, QChar('0'));

    return sResult;
}

QString XBinary::valueToFlagsString(quint64 nValue, QMap<quint64, QString> mapFlags, VL_TYPE vlType)
{
    QString sResult;

    if (vlType == VL_TYPE_LIST) {
        sResult = mapFlags.value(nValue);
    } else if (vlType == VL_TYPE_FLAGS) {
        QMapIterator<quint64, QString> iter(mapFlags);

        while (iter.hasNext()) {
            iter.next();

            quint64 nFlag = iter.key();

            if (nValue & nFlag) {
                if (sResult != "") {
                    sResult += "|";
                }

                sResult += iter.value();
            }
        }
    }

    return sResult;
}

bool XBinary::isX86asm(const QString &sArch)
{
    // TODO remove, use getDisasmMode
    bool bResult = false;

    QString _sArch = sArch.toUpper();

    // TODO Check
    if ((_sArch == "8086") || (_sArch == "80286") || (_sArch == "80386") || (_sArch == "80486") || (_sArch == "80586") || (_sArch == "386") || (_sArch == "I386") ||
        (_sArch == "AMD64") || (_sArch == "X86_64")) {
        bResult = true;
    }

    return bResult;
}

QString XBinary::disasmIdToString(XBinary::DM disasmMode)
{
    QString sResult = tr("Unknown");

    switch (disasmMode) {
        case DM_DATA: sResult = tr("Data"); break;
        case DM_X86_16: sResult = QString("x86 16-bit mode"); break;
        case DM_X86_32: sResult = QString("x86 32-bit mode"); break;
        case DM_X86_64: sResult = QString("x86 64-bit mode"); break;
        case DM_ARM_LE: sResult = QString("ARM"); break;
        case DM_ARM_BE: sResult = QString("ARM, big endian"); break;
        case DM_AARCH64_LE: sResult = QString("AArch64"); break;
        case DM_AARCH64_BE: sResult = QString("AArch64, big endian"); break;
        case DM_CORTEXM: sResult = QString("CORTEXM"); break;
        case DM_THUMB_LE: sResult = QString("THUMB"); break;
        case DM_THUMB_BE: sResult = QString("THUMB BE"); break;
        case DM_MIPS_LE: sResult = QString("MIPS"); break;
        case DM_MIPS_BE: sResult = QString("MIPS BE"); break;
        case DM_MIPS64_LE: sResult = QString("MIPS64"); break;
        case DM_MIPS64_BE: sResult = QString("MIPS64 BE"); break;
        case DM_PPC_LE: sResult = QString("PPC"); break;
        case DM_PPC_BE: sResult = QString("PPC BE"); break;
        case DM_PPC64_LE: sResult = QString("PPC64"); break;
        case DM_PPC64_BE: sResult = QString("PPC64 BE"); break;
        case DM_SPARC: sResult = QString("Sparc"); break;
        case DM_SPARCV9: sResult = QString("Sparc V9"); break;
        case DM_S390X: sResult = QString("S390X"); break;
        case DM_XCORE: sResult = QString("XCORE"); break;
        case DM_M68K: sResult = QString("M68K"); break;
        case DM_M68K00: sResult = QString("M68K00"); break;
        case DM_M68K10: sResult = QString("M68K10"); break;
        case DM_M68K20: sResult = QString("M68K20"); break;
        case DM_M68K30: sResult = QString("M68K30"); break;
        case DM_M68K40: sResult = QString("M68K40"); break;
        case DM_M68K60: sResult = QString("M68K60"); break;
        case DM_TMS320C64X: sResult = QString("TMS320C64X"); break;
        case DM_M6800: sResult = QString("M6800"); break;
        case DM_M6801: sResult = QString("M6801"); break;
        case DM_M6805: sResult = QString("M6805"); break;
        case DM_M6808: sResult = QString("M6808"); break;
        case DM_M6809: sResult = QString("M6809"); break;
        case DM_M6811: sResult = QString("M6811"); break;
        case DM_CPU12: sResult = QString("CPU12"); break;
        case DM_HD6301: sResult = QString("HD6301"); break;
        case DM_HD6309: sResult = QString("HD6309"); break;
        case DM_HCS08: sResult = QString("HCS08"); break;
        case DM_EVM: sResult = QString("EVM"); break;
        case DM_RISKV32: sResult = QString("RISKV32"); break;
        case DM_RISKV64: sResult = QString("RISKV64"); break;
        case DM_RISKVC: sResult = QString("RISKVC"); break;
        case DM_MOS65XX: sResult = QString("MOS65XX"); break;
        case DM_WASM: sResult = QString("WASM"); break;
        case DM_BPF_LE: sResult = QString("BPF LE"); break;
        case DM_BPF_BE: sResult = QString("BPF BE"); break;
        case DM_CUSTOM_MACH_REBASE: sResult = QString("MACH REBASE"); break;
        case DM_CUSTOM_7ZIP_PROPERTIES: sResult = QString("7ZIP PROPERTIES"); break;
        default: sResult = tr("Unknown");
    }

    return sResult;
}

QString XBinary::syntaxIdToString(SYNTAX syntax)
{
    QString sResult = tr("Unknown");

    switch (syntax) {
        case SYNTAX_DEFAULT: sResult = tr("Default"); break;
        case SYNTAX_ATT: sResult = QString("ATT"); break;
        case SYNTAX_INTEL: sResult = QString("INTEL"); break;
        case SYNTAX_MASM: sResult = QString("MASM"); break;
        case SYNTAX_MOTOROLA: sResult = QString("MOTOROLA"); break;
    }

    return sResult;
}

XBinary::SYNTAX XBinary::stringToSyntaxId(const QString &sString)
{
    SYNTAX result = SYNTAX_DEFAULT;

    if (sString == "ATT") result = SYNTAX_ATT;
    else if (sString == "INTEL") result = SYNTAX_INTEL;
    else if (sString == "MASM") result = SYNTAX_MASM;
    else if (sString == "MOTOROLA") result = SYNTAX_MOTOROLA;

    return result;
}

QString XBinary::osNameIdToString(OSNAME osName)
{
    QString sResult = tr("Unknown");

    switch (osName) {
        case OSNAME_MULTIPLATFORM: sResult = tr("Multiplatform"); break;
        case OSNAME_AIX: sResult = QString("AIX"); break;
        case OSNAME_ALPINELINUX: sResult = QString("Alpine Linux"); break;
        case OSNAME_AMIGA: sResult = QString("Amiga"); break;
        case OSNAME_ANDROID: sResult = QString("Android"); break;
        case OSNAME_AROS: sResult = QString("Amiga Research OS"); break;
        case OSNAME_ASPLINUX: sResult = QString("ASPLinux"); break;
        case OSNAME_BORLANDOSSERVICES: sResult = QString("Borland OS Services"); break;
        case OSNAME_BRIDGEOS: sResult = QString("bridgeOS"); break;
        case OSNAME_DEBIANLINUX: sResult = QString("Debian Linux"); break;
        case OSNAME_FENIXOS: sResult = QString("FenixOS"); break;
        case OSNAME_FREEBSD: sResult = QString("FreeBSD"); break;
        case OSNAME_GENTOOLINUX: sResult = QString("Gentoo Linux"); break;
        case OSNAME_HANCOMLINUX: sResult = QString("Hancom Linux"); break;
        case OSNAME_HPUX: sResult = QString("Hewlett-Packard HP-UX"); break;
        case OSNAME_IOS: sResult = QString("iOS"); break;
        case OSNAME_IPADOS: sResult = QString("iPadOS"); break;
        case OSNAME_IPHONEOS: sResult = QString("iPhone OS"); break;
        case OSNAME_IRIX: sResult = QString("IRIX"); break;
        case OSNAME_LINUX: sResult = QString("Linux"); break;
        case OSNAME_MACOS: sResult = QString("macOS"); break;
        case OSNAME_MAC_OS: sResult = QString("Mac OS"); break;
        case OSNAME_MAC_OS_X: sResult = QString("Mac OS X"); break;
        case OSNAME_MACCATALYST: sResult = QString("Mac Catalyst"); break;
        case OSNAME_MANDRAKELINUX: sResult = QString("Mandrake Linux"); break;
        case OSNAME_MACDRIVERKIT: sResult = QString("Mac DriverKit"); break;
        case OSNAME_MACFIRMWARE: sResult = QString("Mac Firmware"); break;
        case OSNAME_MCLINUX: sResult = QString("mClinux"); break;
        case OSNAME_MINIX: sResult = QString("Minix"); break;
        case OSNAME_MODESTO: sResult = QString("Novell Modesto"); break;
        case OSNAME_MSDOS: sResult = QString("MS-DOS"); break;
        case OSNAME_NETBSD: sResult = QString("NetBSD"); break;
        case OSNAME_NSK: sResult = QString("Hewlett-Packard Non-Stop Kernel"); break;
        case OSNAME_OPENBSD: sResult = QString("OpenBSD"); break;
        case OSNAME_OPENVMS: sResult = QString("Open VMS"); break;
        case OSNAME_OPENVOS: sResult = QString("Open VOS"); break;
        case OSNAME_OS2: sResult = QString("OS/2"); break;
        case OSNAME_OS_X: sResult = QString("OS X"); break;
        case OSNAME_POSIX: sResult = QString("Posix"); break;
        case OSNAME_QNX: sResult = QString("QNX"); break;
        case OSNAME_REDHATLINUX: sResult = QString("Red Hat Linux"); break;
        case OSNAME_SEPOS: sResult = QString("sepOS"); break;
        case OSNAME_SOLARIS: sResult = QString("Sun Solaris"); break;
        case OSNAME_STARTOSLINUX: sResult = QString("StartOS Linux"); break;
        case OSNAME_SUNOS: sResult = QString("SunOS"); break;
        case OSNAME_SUSELINUX: sResult = QString("SUSE Linux"); break;
        case OSNAME_SYLLABLE: sResult = QString("Syllable"); break;
        case OSNAME_TRU64: sResult = QString("Compaq TRU64 UNIX"); break;
        case OSNAME_TURBOLINUX: sResult = QString("Turbolinux"); break;
        case OSNAME_TVOS: sResult = QString("tvOS"); break;
        case OSNAME_UBUNTULINUX: sResult = QString("Ubuntu Linux"); break;
        case OSNAME_UEFI: sResult = QString("UEFI"); break;
        case OSNAME_UNIX: sResult = QString("Unix"); break;
        case OSNAME_VINELINUX: sResult = QString("Vine Linux"); break;
        case OSNAME_WATCHOS: sResult = QString("watchOS"); break;
        case OSNAME_WINDOWS: sResult = QString("Windows"); break;
        case OSNAME_WINDOWSCE: sResult = QString("Windows CE"); break;
        case OSNAME_WINDRIVERLINUX: sResult = QString("Wind River Linux"); break;
        case OSNAME_XBOX: sResult = QString("XBOX"); break;
        case OSNAME_JVM: sResult = QString("JVM"); break;

        default: sResult = tr("Unknown");
    }

    return sResult;
}

XBinary::DM XBinary::getDisasmMode()
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return getDisasmMode(&memoryMap);
}

XBinary::DM XBinary::getDisasmMode(XBinary::_MEMORY_MAP *pMemoryMap)
{
    return getDisasmMode(pMemoryMap->sArch, (pMemoryMap->endian == ENDIAN_BIG), pMemoryMap->mode);
}

XBinary::DM XBinary::getDisasmMode(const QString &sArch, bool bIsBigEndian, MODE mode)
{
    XBinary::DM dmResult = DM_UNKNOWN;

    QString _sArch = sArch.toUpper();

    if (_sArch == "PPC") {
        if (bIsBigEndian) {
            dmResult = DM_PPC_BE;
        } else {
            dmResult = DM_PPC_LE;
        }
    } else if (_sArch == "PPC64") {
        if (bIsBigEndian) {
            dmResult = DM_PPC64_BE;
        } else {
            dmResult = DM_PPC64_LE;
        }
    } else if ((_sArch == "MIPS") || (_sArch == "R3000") || (_sArch == "R4000") || (_sArch == "R10000") || (_sArch == "WCEMIPSV2")) {
        if (bIsBigEndian) {
            dmResult = DM_MIPS_BE;
        } else {
            dmResult = DM_MIPS_LE;
        }
    } else if ((_sArch == "ARM") || (_sArch == "ARM_V6") || (_sArch == "ARM_V7") || (_sArch == "ARM_V7S") || (_sArch == "ARM64_32")) {
        if (bIsBigEndian) {
            dmResult = DM_ARM_BE;
        } else {
            dmResult = DM_ARM_LE;
        }
    } else if ((_sArch == "AARCH64") || (_sArch == "ARM64") || (_sArch == "ARM64E") || (_sArch == "ARM_A500")) {
        if (bIsBigEndian) {
            dmResult = DM_AARCH64_BE;
        } else {
            dmResult = DM_AARCH64_LE;
        }
    } else if ((_sArch == "8086") || (_sArch == "286")) {
        dmResult = DM_X86_16;
    } else if ((_sArch == "386") || (_sArch == "80386") || (_sArch == "80486") || (_sArch == "80586") || (_sArch == "I386") || (_sArch == "486") || (_sArch == "X86")) {
        if ((mode == MODE_16) || (mode == MODE_16SEG)) {
            dmResult = DM_X86_16;
        } else {
            dmResult = DM_X86_32;
        }
    } else if ((_sArch == "AMD64") || (_sArch == "X86_64") || (_sArch == "X64")) {
        dmResult = DM_X86_64;
    } else if ((_sArch == "68K") || (_sArch == "MC680x0")) {
        dmResult = DM_M68K;
    } else if ((_sArch == "MC68030") || (_sArch == "MC68030_ONLY")) {
        dmResult = DM_M68K30;
    } else if (_sArch == "MC68040") {
        dmResult = DM_M68K40;
    } else if (_sArch == "POWERPC") {
        if (bIsBigEndian) {
            dmResult = DM_PPC_BE;
        } else {
            dmResult = DM_PPC_LE;
        }
    } else if (_sArch == "POWERPC_BE") {
        dmResult = DM_PPC_BE;
    } else if (_sArch == "SPARC") {
        dmResult = DM_SPARC;
    } else if ((_sArch == "RISC_V") || (_sArch == "RISCV32") || (_sArch == "RISCV64")) {
        if (mode == MODE_64) {
            dmResult = DM_RISKV64;
        } else {
            dmResult = DM_RISKV32;
        }
    }
    // else if (_sArch == "MACH_REBASE") {
    //     dmResult = DM_CUSTOM_MACH_REBASE;
    // }
    // TODO SH
    // TODO more

    return dmResult;
}

XBinary::DM XBinary::getDisasmMode(OSINFO osInfo)
{
    return getDisasmMode(osInfo.sArch, (osInfo.endian == ENDIAN_BIG), osInfo.mode);
}

XBinary::DMFAMILY XBinary::getDisasmFamily(XBinary::DM disasmMode)
{
    DMFAMILY result = DMFAMILY_UNKNOWN;

    if ((disasmMode == DM_X86_16) || (disasmMode == DM_X86_32) || (disasmMode == DM_X86_64)) {
        result = DMFAMILY_X86;
    } else if ((disasmMode == DM_ARM_BE) || (disasmMode == DM_ARM_LE)) {
        result = DMFAMILY_ARM;
    } else if ((disasmMode == DM_AARCH64_BE) || (disasmMode == DM_AARCH64_LE)) {
        result = DMFAMILY_ARM64;
    } else if ((disasmMode == DM_MIPS64_BE) || (disasmMode == DM_MIPS64_LE) || (disasmMode == DM_MIPS_BE) || (disasmMode == DM_MIPS_LE)) {
        result = DMFAMILY_MIPS;
    } else if ((disasmMode == DM_PPC64_BE) || (disasmMode == DM_PPC64_LE) || (disasmMode == DM_PPC_BE) || (disasmMode == DM_PPC_LE)) {
        result = DMFAMILY_PPC;
    } else if (disasmMode == DM_SPARC) {
        result = DMFAMILY_SPARC;
    } else if (disasmMode == DM_S390X) {
        result = DMFAMILY_SYSZ;
    } else if (disasmMode == DM_XCORE) {
        result = DMFAMILY_XCORE;
    } else if ((disasmMode == DM_M68K) || (disasmMode == DM_M68K00) || (disasmMode == DM_M68K10) || (disasmMode == DM_M68K20) || (disasmMode == DM_M68K30) ||
               (disasmMode == DM_M68K40) || (disasmMode == DM_M68K60)) {
        result = DMFAMILY_M68K;
    } else if ((disasmMode == DM_M6800) || (disasmMode == DM_M6801) || (disasmMode == DM_M6805) || (disasmMode == DM_M6808) || (disasmMode == DM_M6809) ||
               (disasmMode == DM_M6811) || (disasmMode == DM_CPU12) || (disasmMode == DM_HD6301) || (disasmMode == DM_HD6309) || (disasmMode == DM_HCS08)) {
        result = DMFAMILY_M68OK;
    } else if ((disasmMode == DM_RISKV32) || (disasmMode == DM_RISKV64) || (disasmMode == DM_RISKVC)) {
        result = DMFAMILY_RISCV;
    } else if (disasmMode == DM_EVM) {
        result = DMFAMILY_EVM;
    } else if (disasmMode == DM_MOS65XX) {
        result = DMFAMILY_MOS65XX;
    } else if (disasmMode == DM_WASM) {
        result = DMFAMILY_WASM;
    } else if ((disasmMode == DM_BPF_LE) || (disasmMode == DM_BPF_BE)) {
        result = DMFAMILY_BPF;
    } else if (disasmMode == DM_CUSTOM_MACH_REBASE) {
        result = DMFAMILY_CUSTOM_MACH_REBASE;
    } else if ((disasmMode == DM_CUSTOM_MACH_BIND) || (disasmMode == DM_CUSTOM_MACH_WEAK)) {
        result = DMFAMILY_CUSTOM_MACH_BIND;
    } else if (disasmMode == DM_CUSTOM_MACH_EXPORT) {
        result = DMFAMILY_CUSTOM_MACH_EXPORT;
    } else if (disasmMode == DM_CUSTOM_7ZIP_PROPERTIES) {
        result = DMFAMILY_CUSTOM_7ZIP;
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

    if (getDisasmFamily(disasmMode) == DMFAMILY_X86) {
        listResult.append(SYNTAX_ATT);
        listResult.append(SYNTAX_INTEL);
        listResult.append(SYNTAX_MASM);
    }

    // TODO Motorola

    return listResult;
}

XBinary::MODE XBinary::getModeFromDisasmMode(DM disasmMode)
{
    MODE result = MODE_32;

    if (disasmMode == DM_X86_16) {
        result = MODE_16;
    } else if (disasmMode == DM_X86_32) {
        result = MODE_32;
    } else if (disasmMode == DM_X86_64) {
        result = MODE_64;
    } else if ((disasmMode == DM_ARM_BE) || (disasmMode == DM_ARM_LE)) {
        result = MODE_32;
    } else if ((disasmMode == DM_AARCH64_BE) || (disasmMode == DM_AARCH64_LE)) {
        result = MODE_64;
    } else if ((disasmMode == DM_MIPS_BE) || (disasmMode == DM_MIPS_LE)) {
        result = MODE_32;
    } else if ((disasmMode == DM_MIPS64_BE) || (disasmMode == DM_MIPS64_LE)) {
        result = MODE_64;
    }

    return result;
}

bool XBinary::checkFileType(XBinary::FT fileTypeMain, XBinary::FT fileTypeOptional)
{
    bool bResult = false;

    if ((fileTypeMain == FT_PE) && ((fileTypeOptional == FT_PE) || (fileTypeOptional == FT_PE32) || (fileTypeOptional == FT_PE64))) {
        bResult = true;
    } else if ((fileTypeMain == FT_ELF) && ((fileTypeOptional == FT_ELF) || (fileTypeOptional == FT_ELF32) || (fileTypeOptional == FT_ELF64))) {
        bResult = true;
    } else if ((fileTypeMain == FT_MACHO) && ((fileTypeOptional == FT_MACHO) || (fileTypeOptional == FT_MACHO32) || (fileTypeOptional == FT_MACHO64))) {
        bResult = true;
    } else if ((fileTypeMain == FT_ICO) && ((fileTypeOptional == FT_ICO) || (fileTypeOptional == FT_CUR))) {
        bResult = true;
    } else if (fileTypeMain == fileTypeOptional) {
        bResult = true;
    }

    return bResult;
}

void XBinary::filterFileTypes(QSet<XBinary::FT> *pStFileTypes)
{
    // TODO Check!
    // TODO optimize! new Types create remove function
    if (pStFileTypes->contains(XBinary::FT_MSDOS) || pStFileTypes->contains(XBinary::FT_DOS16M) || pStFileTypes->contains(XBinary::FT_DOS4G) ||
        pStFileTypes->contains(XBinary::FT_NE) || pStFileTypes->contains(XBinary::FT_LE) || pStFileTypes->contains(XBinary::FT_LX) ||
        pStFileTypes->contains(XBinary::FT_PE) || pStFileTypes->contains(XBinary::FT_PE32) || pStFileTypes->contains(XBinary::FT_PE64) ||
        pStFileTypes->contains(XBinary::FT_ELF) || pStFileTypes->contains(XBinary::FT_ELF32) || pStFileTypes->contains(XBinary::FT_ELF64) ||
        pStFileTypes->contains(XBinary::FT_MACHO) || pStFileTypes->contains(XBinary::FT_MACHO32) || pStFileTypes->contains(XBinary::FT_MACHO64) ||
        pStFileTypes->contains(XBinary::FT_DEX) || pStFileTypes->contains(XBinary::FT_ZIP) || pStFileTypes->contains(XBinary::FT_GZIP) ||
        pStFileTypes->contains(XBinary::FT_ZLIB) || pStFileTypes->contains(XBinary::FT_LHA) || pStFileTypes->contains(XBinary::FT_AMIGAHUNK)) {
        pStFileTypes->remove(XBinary::FT_BINARY);
    } else {
        pStFileTypes->insert(XBinary::FT_COM);
    }
}

void XBinary::filterFileTypes(QSet<XBinary::FT> *pStFileTypes, XBinary::FT fileType)
{
    // TODO Check!
    if (fileType != XBinary::FT_UNKNOWN) {
        QSet<XBinary::FT> stFileTypesNew;

        if (fileType == XBinary::FT_PE) {
            if (pStFileTypes->contains(XBinary::FT_PE)) stFileTypesNew.insert(XBinary::FT_PE);
            if (pStFileTypes->contains(XBinary::FT_PE32)) stFileTypesNew.insert(XBinary::FT_PE32);
            if (pStFileTypes->contains(XBinary::FT_PE64)) stFileTypesNew.insert(XBinary::FT_PE64);
        } else if (fileType == XBinary::FT_ELF) {
            if (pStFileTypes->contains(XBinary::FT_ELF)) stFileTypesNew.insert(XBinary::FT_ELF);
            if (pStFileTypes->contains(XBinary::FT_ELF32)) stFileTypesNew.insert(XBinary::FT_ELF32);
            if (pStFileTypes->contains(XBinary::FT_ELF64)) stFileTypesNew.insert(XBinary::FT_ELF64);
        } else if (fileType == XBinary::FT_MACHO) {
            if (pStFileTypes->contains(XBinary::FT_MACHO)) stFileTypesNew.insert(XBinary::FT_MACHO);
            if (pStFileTypes->contains(XBinary::FT_MACHO32)) stFileTypesNew.insert(XBinary::FT_MACHO32);
            if (pStFileTypes->contains(XBinary::FT_MACHO64)) stFileTypesNew.insert(XBinary::FT_MACHO64);
        } else {
            if (pStFileTypes->contains(fileType)) stFileTypesNew.insert(fileType);
        }

        *pStFileTypes = stFileTypesNew;
    }
}

bool XBinary::isFileTypePresent(QSet<XBinary::FT> *pStFileTypes, QSet<XBinary::FT> *pStAvailableFileTypes)
{
    bool bResult = false;

    QSet<XBinary::FT>::iterator i = pStFileTypes->begin();

    while (i != pStFileTypes->end()) {
        if (pStAvailableFileTypes->contains(*i)) {
            bResult = true;

            break;
        }

        i++;
    }

    return bResult;
}

XBinary::PERCENTAGE XBinary::percentageInit(qint64 nMaxValue, bool bTimer)
{
    PERCENTAGE result = {};
    result.bTimer = bTimer;

    result.nMaxValue = nMaxValue;

    if (!(result.bTimer)) {
        result.nMaxPercentage = 1;

        if (result.nMaxValue > 0x100000000) {
            result.nMaxPercentage = 100;
        } else if (result.nMaxValue > 0x100000) {
            result.nMaxPercentage = 10;
        } else if (result.nMaxValue > 0x1000) {
            result.nMaxPercentage = 5;
        }
    } else {
        result.timer.start();
        result.nMaxPercentage = 100;
    }

    return result;
}

bool XBinary::percentageSetCurrentValue(XBinary::PERCENTAGE *pPercentage, qint64 nCurrentValue)
{
    bool bResult = false;

    pPercentage->nCurrentValue = nCurrentValue;

    if (!(pPercentage->bTimer)) {
        if (pPercentage->nCurrentValue > ((pPercentage->nCurrentPercentage + 1) * (pPercentage->nMaxValue / pPercentage->nMaxPercentage))) {
            pPercentage->nCurrentPercentage++;
            bResult = true;
        }
    } else {
        if (pPercentage->timer.elapsed() >= 1000)  // TODO Check speed
        {
            pPercentage->timer.restart();

            qint32 _nCurrent = (pPercentage->nCurrentValue * 100) / (pPercentage->nMaxValue);

            pPercentage->nCurrentPercentage = _nCurrent;
            bResult = true;
        }
    }

    return bResult;
}

qint64 XBinary::getTotalOSSize(QList<OFFSETSIZE> *pListOffsetSize)
{
    qint64 nResult = 0;

    qint32 nNumberOfRecords = pListOffsetSize->count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        nResult += pListOffsetSize->at(i).nSize;
    }

    return nResult;
}

quint32 XBinary::getByteSizeFromWidthMode(MODE mode)
{
    quint32 nResult = 0;

    switch (mode) {
        case MODE_8: nResult = 1; break;
        case MODE_16: nResult = 2; break;
        case MODE_32: nResult = 4; break;
        case MODE_64: nResult = 8; break;
    }

    return nResult;
}

XBinary::MODE XBinary::getWidthModeFromSize(quint64 nSize)
{
    MODE result = MODE_32;

    if (((quint64)nSize) >= 0xFFFFFFFF) {
        result = MODE_64;
    } else if (((quint64)nSize) >= 0xFFFF) {
        result = MODE_32;
    } else if (((quint64)nSize) >= 0xFF) {
        result = MODE_16;
    } else {
        result = MODE_8;
    }

    return result;
}

XBinary::MODE XBinary::getWidthModeFromMemoryMap(XBinary::_MEMORY_MAP *pMemoryMap)
{
    MODE result = MODE_32;

    qint64 nMax = qMax(pMemoryMap->nModuleAddress + pMemoryMap->nImageSize, (XADDR)(pMemoryMap->nBinarySize));

    result = getWidthModeFromSize(nMax);

    return result;
}

XBinary::MODE XBinary::getWidthModeFromByteSize(quint32 nByteSize)
{
    MODE result = MODE_32;

    if (nByteSize == 2) {
        result = MODE_8;
    } else if (nByteSize == 4) {
        result = MODE_16;
    } else if (nByteSize == 8) {
        result = MODE_32;
    } else if (nByteSize == 16) {
        result = MODE_64;
    }

    return result;
}

bool XBinary::isAnsiSymbol(quint8 cCode, bool bExtra)
{
    bool bResult = false;

    if (!bExtra) {
        if ((cCode >= 20) && (cCode < 0x80)) {
            bResult = true;
        }
    } else {
        if (cCode >= 20) {
            bResult = true;
        }
    }

    return bResult;
}

bool XBinary::isUTF8Symbol(quint8 cCode, qint32 *pnWidth)
{
    bool bResult = false;

    if (cCode >= 20) {
        if ((cCode >> 7) == 0) {
            *pnWidth = 1;
            bResult = true;
        } else if ((cCode >> 5) == 0x6) {
            *pnWidth = 2;
            bResult = true;
        } else if ((cCode >> 4) == 0xE) {
            *pnWidth = 3;
            bResult = true;
        } else if ((cCode >> 3) == 0x1E) {
            *pnWidth = 4;
            bResult = true;
        } else if ((cCode >> 6) == 0x2)  // Rest bytes
        {
            *pnWidth = 0;
            bResult = true;
        }
    }

    return bResult;
}

bool XBinary::isUnicodeSymbol(quint16 nCode, bool bExtra)
{
    bool bResult = false;

    if (!bExtra) {
        if ((nCode >= 20) && (nCode < 0x80)) {
            bResult = true;
        }
    } else {
        if ((nCode >= 20) && (nCode <= 0xFF)) {
            bResult = true;
        } else if ((nCode >= 0x0400) && (nCode <= 0x04FF))  // Cyrillic
        {
            bResult = true;
        }
    }

    return bResult;
}

QString XBinary::getStringFromIndex(qint64 nOffset, qint64 nSize, qint32 nIndex)
{
    QString sResult;

    if (nIndex < nSize) {
        sResult = read_ansiString(nOffset + nIndex);
    }

    return sResult;
}

QList<QString> XBinary::getAllFilesFromDirectory(const QString &sDirectory, const QString &sExtension)
{
    QDir directory(sDirectory);

    return directory.entryList(QStringList() << sExtension, QDir::Files);
}

QList<XBinary::OPCODE> XBinary::getOpcodes(qint64 nOffset, XADDR nStartAddress, qint64 nSize, quint32 nType, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    QList<OPCODE> listResult;

    OFFSETSIZE osRegion = convertOffsetAndSize(nOffset, nSize);

    nOffset = osRegion.nOffset;
    nSize = osRegion.nSize;

    if (nOffset != -1) {
        char *pBuffer = new char[READWRITE_BUFFER_SIZE];

        while ((nSize > 0) && (!(pPdStruct->bIsStop))) {
            qint64 nTempSize = qMin((qint64)READWRITE_BUFFER_SIZE, nSize);

            if (read_array(nOffset, pBuffer, nTempSize) != nTempSize) {
                pPdStruct->sInfoString = tr("Read error");

                break;
            }

            qint64 nOpcodesSize = 0;
            OPCODE_STATUS opcodeStatus = OPCODE_STATUS_SUCCESS;

            for (qint32 i = 0; i < nTempSize;) {
                qint64 _nSize = readOpcodes(nType, pBuffer + i, nStartAddress + i, nTempSize - i, &listResult, &opcodeStatus);

                i += _nSize;
                nOpcodesSize += _nSize;

                if ((_nSize == 0) || (opcodeStatus == OPCODE_STATUS_END)) {
                    break;
                }
            }

            if ((nOpcodesSize == 0) || (opcodeStatus == OPCODE_STATUS_END)) {
                //                _errorMessage(tr("Read error"));
                break;
            }

            nSize -= nOpcodesSize;
            nOffset += nOpcodesSize;
            nStartAddress += nOpcodesSize;
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

    *pOpcodeStatus = OPCODE_STATUS_END;

    return false;
}

bool XBinary::_read_opcode_uleb128(OPCODE *pOpcode, char **ppData, qint64 *pnSize, XADDR *pnAddress, XADDR *pnResult, const QString &sPrefix)
{
    bool bResult = false;

    if (*pnSize > 0) {
        PACKED_UINT uleb128 = _read_uleb128(*ppData, *pnSize);

        if (uleb128.bIsValid) {
            pOpcode->nAddress = *pnAddress;
            pOpcode->nSize = uleb128.nByteSize;
            pOpcode->sName = QString("%1(%2)").arg(sPrefix, QString::number((qint64)uleb128.nValue));

            *pnSize -= pOpcode->nSize;
            *ppData += pOpcode->nSize;
            *pnResult += pOpcode->nSize;
            *pnAddress += pOpcode->nSize;

            bResult = true;
        }
    }

    return bResult;
}

bool XBinary::_read_opcode_ansiString(XBinary::OPCODE *pOpcode, char **ppData, qint64 *pnSize, XADDR *pnAddress, XADDR *pnResult, const QString &sPrefix)
{
    bool bResult = false;

    if (*pnSize > 0) {
        QString sString = *ppData;

        if (sString.size() < (*pnSize))  // We need cstring with \0
        {
            pOpcode->nAddress = *pnAddress;
            pOpcode->nSize = sString.size() + 1;
            pOpcode->sName = QString("%1(\"%2\")").arg(sPrefix, sString);

            *pnSize -= pOpcode->nSize;
            *ppData += pOpcode->nSize;
            *pnResult += pOpcode->nSize;
            *pnAddress += pOpcode->nSize;

            bResult = true;
        }
    }

    return bResult;
}

QList<quint32> XBinary::get_uint32_list(qint64 nOffset, qint32 nNumberOfRecords, bool bIsBigEndian)
{
    QList<quint32> listResult;

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        quint32 nRecord = read_uint32(nOffset + i * sizeof(quint32), bIsBigEndian);

        listResult.append(nRecord);
    }

    return listResult;
}

QList<quint64> XBinary::get_uint64_list(qint64 nOffset, qint32 nNumberOfRecords, bool bIsBigEndian)
{
    QList<quint64> listResult;

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        quint64 nRecord = read_uint64(nOffset + i * sizeof(quint64), bIsBigEndian);

        listResult.append(nRecord);
    }

    return listResult;
}

bool XBinary::_isOffsetsCrossed(qint64 nOffset1, qint64 nSize1, qint64 nOffset2, qint64 nSize2)
{
    bool bResult = false;

    if (((nOffset2 >= nOffset1) && ((nOffset1 + nSize1) > nOffset2)) || ((nOffset1 >= nOffset2) && ((nOffset2 + nSize2) > nOffset1))) {
        bResult = true;
    }

    return bResult;
}

bool XBinary::_isAddressCrossed(XADDR nAddress1, qint64 nSize1, XADDR nAddress2, qint64 nSize2)
{
    bool bResult = false;

    // #ifdef QT_DEBUG
    //     if (nAddress1 == 0x771F8164) {
    //         bResult = false;
    //     }
    // #endif

    if (((nAddress2 >= nAddress1) && ((nAddress1 + nSize1) > nAddress2)) || ((nAddress1 >= nAddress2) && ((nAddress2 + nSize2) > nAddress1))) {
        bResult = true;
    }

    return bResult;
}

bool XBinary::_isReplaced(qint64 nOffset, qint64 nSize, QList<XBinary::MEMORY_REPLACE> *pListMemoryReplace)
{
    bool bResult = false;

    if (nSize) {
        qint32 nNumberOfRecords = pListMemoryReplace->count();

        for (qint32 i = 0; i < nNumberOfRecords; i++) {
            if (_isOffsetsCrossed(nOffset, nSize, pListMemoryReplace->at(i).nOffset, pListMemoryReplace->at(i).nSize)) {
                bResult = true;
                break;
            }
        }
    }

    return bResult;
}

bool XBinary::_replaceMemory(qint64 nDataOffset, char *pData, qint64 nDataSize, QList<XBinary::MEMORY_REPLACE> *pListMemoryReplace)
{
    bool bResult = false;

    qint32 nNumberOfRecords = pListMemoryReplace->count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (_isOffsetsCrossed(nDataOffset, nDataSize, pListMemoryReplace->at(i).nOffset, pListMemoryReplace->at(i).nSize)) {
            MEMORY_REPLACE memoryReplace = pListMemoryReplace->at(i);

            for (qint32 j = 0; j < memoryReplace.nSize; j++) {
                bResult = true;

                pData[(memoryReplace.nOffset + j) - nDataOffset] = memoryReplace.baOriginal.data()[j];

                if ((memoryReplace.nOffset + j) > (nDataOffset + nDataSize)) {
                    break;
                }
            }
        }
    }

    return bResult;
}

bool XBinary::_updateReplaces(qint64 nDataOffset, char *pData, qint64 nDataSize, QList<MEMORY_REPLACE> *pListMemoryReplace)
{
    bool bResult = false;

    qint32 nNumberOfRecords = pListMemoryReplace->count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (_isOffsetsCrossed(nDataOffset, nDataSize, pListMemoryReplace->at(i).nOffset, pListMemoryReplace->at(i).nSize)) {
            MEMORY_REPLACE memoryReplace = pListMemoryReplace->at(i);

            for (qint32 j = 0; j < memoryReplace.nSize; j++) {
                bResult = true;

                memoryReplace.baOriginal.data()[j] = pData[(memoryReplace.nOffset + j) - nDataOffset];

                if ((memoryReplace.nOffset + j) > (nDataOffset + nDataSize)) {
                    break;
                }
            }
        }
    }

    return bResult;
}

QList<XBinary::SYMBOL_RECORD> XBinary::getSymbolRecords(XBinary::_MEMORY_MAP *pMemoryMap, SYMBOL_TYPE symbolType)
{
    Q_UNUSED(pMemoryMap)
    Q_UNUSED(symbolType)

    QList<XBinary::SYMBOL_RECORD> listResult;

    // TODO

    return listResult;
}

XBinary::SYMBOL_RECORD XBinary::findSymbolByAddress(QList<SYMBOL_RECORD> *pListSymbolRecords, XADDR nAddress)
{
    SYMBOL_RECORD result = {};

    qint32 nNumberOfRecords = pListSymbolRecords->count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (pListSymbolRecords->at(i).nAddress == nAddress) {
            result = pListSymbolRecords->at(i);

            break;
        }
    }

    return result;
}

XBinary::SYMBOL_RECORD XBinary::findSymbolByName(QList<SYMBOL_RECORD> *pListSymbolRecords, const QString &sName)
{
    SYMBOL_RECORD result = {};

    qint32 nNumberOfRecords = pListSymbolRecords->count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (pListSymbolRecords->at(i).sName == sName) {
            result = pListSymbolRecords->at(i);

            break;
        }
    }

    return result;
}

XBinary::SYMBOL_RECORD XBinary::findSymbolByOrdinal(QList<SYMBOL_RECORD> *pListSymbolRecords, qint32 nOrdinal)
{
    SYMBOL_RECORD result = {};

    qint32 nNumberOfRecords = pListSymbolRecords->count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        if (pListSymbolRecords->at(i).nOrdinal == nOrdinal) {
            result = pListSymbolRecords->at(i);

            break;
        }
    }

    return result;
}

QString XBinary::generateUUID()
{
    return QUuid::createUuid().toString().remove("{").remove("}");
}

QString XBinary::appendText(const QString &sResult, const QString &sString, const QString &sSeparate)
{
    QString _sResult = sResult;

    if (sString != "") {
        if (_sResult != "") _sResult += sSeparate;
        _sResult += sString;
    }

    return _sResult;
}

QString XBinary::bytesCountToString(quint64 nValue, quint64 nBase)
{
    QString sResult;

    if (nBase != 1024) {
        nBase = 1000;
    }

    QString sValue;
    QString sUnit;

    if (nValue < nBase) {
        sValue = QString::number(nValue);
        sUnit = tr("Bytes");
    } else if (nValue < (nBase * nBase)) {
        sValue = QString::number((double)nValue / nBase, 'f', 2);

        if (nBase == 1024) sUnit = tr("KiB");
        else tr("kB");
    } else if (nValue < (nBase * nBase * nBase)) {
        sValue = QString::number((double)nValue / (nBase * nBase), 'f', 2);

        if (nBase == 1024) sUnit = tr("MiB");
        else tr("MB");
    } else if (nValue < (nBase * nBase * nBase * nBase)) {
        sValue = QString::number((double)nValue / (nBase * nBase * nBase), 'f', 2);

        if (nBase == 1024) sUnit = tr("GiB");
        else tr("GB");
    } else {
        sValue = QString::number((double)nValue / (nBase * nBase * nBase * nBase), 'f', 2);

        if (nBase == 1024) sUnit = tr("TiB");
        else tr("TB");
    }

    sResult = QString("%1 %2").arg(sValue, sUnit);

    return sResult;
}

QString XBinary::numberToString(quint64 nValue)
{
    QString sResult;

    sResult = QString("\"%1\"").arg(nValue);

    return sResult;
}

QString XBinary::fullVersionByteToString(quint8 nValue)
{
    return QString("\"%1\"").arg(get_uint8_full_version(nValue));
}

QString XBinary::fullVersionWordToString(quint16 nValue)
{
    return QString("\"%1\"").arg(get_uint16_full_version(nValue));
}

QString XBinary::fullVersionDwordToString(quint32 nValue)
{
    return QString("\"%1\"").arg(get_uint32_full_version(nValue));
}

QString XBinary::fullVersionQwordToString(quint64 nValue)
{
    return QString("\"%1\"").arg(get_uint64_full_version(nValue));
}

QString XBinary::versionWordToString(quint16 nValue)
{
    return QString("\"%1\"").arg(get_uint16_version(nValue));
}

QString XBinary::versionDwordToString(quint32 nValue)
{
    return QString("\"%1\"").arg(get_uint32_version(nValue));
}

QString XBinary::formatXML(const QString &sXML)
{
    QString sResult;

    QXmlStreamReader reader(sXML);
    QXmlStreamWriter writer(&sResult);
    writer.setAutoFormatting(true);

    while (!reader.atEnd()) {
        reader.readNext();
        if (!reader.isWhitespace()) {
            writer.writeCurrentToken(reader);
        }
    }

    return sResult;
}

quint32 XBinary::make_dword(XBinary::XDWORD xdword)
{
    quint32 nResult = 0;

    nResult += (xdword.nValue1) << 16;
    nResult += xdword.nValue2;

    return nResult;
}

quint32 XBinary::make_dword(quint16 nValue1, quint16 nValue2)
{
    XDWORD xdword = {};
    xdword.nValue1 = nValue1;
    xdword.nValue2 = nValue2;

    return make_dword(xdword);
}

XBinary::XDWORD XBinary::make_xdword(quint32 nValue)
{
    XDWORD result = {};

    result.nValue1 = (quint16)(nValue >> 16);
    result.nValue2 = (quint16)(nValue & 0xFFFF);

    return result;
}

QString XBinary::recordFilePartIdToString(FILEPART id)
{
    QString sResult = tr("Unknown");

    switch (id) {
        // TODO more
        case FILEPART_UNKNOWN: sResult = tr("Unknown"); break;
        case FILEPART_HEADER: sResult = tr("Header"); break;
        case FILEPART_OVERLAY: sResult = tr("Overlay"); break;
        case FILEPART_ARCHIVERECORD: sResult = tr("Archive record"); break;
        case FILEPART_RESOURCE: sResult = tr("Resource"); break;
        case FILEPART_REGION: sResult = tr("Region"); break;
        case FILEPART_DEBUGDATA: sResult = tr("Debug data"); break;
    }

    return sResult;
}

bool XBinary::checkVersionString(const QString &sVersion)
{
    bool bResult = false;

    if (sVersion.trimmed() != "") {
        bResult = true;

        qint32 nStringSize = sVersion.size();

        // TODO
        for (qint32 i = 0; i < nStringSize; i++) {
            QChar _char = sVersion.at(i);

            if ((_char >= QChar('0')) && (_char <= QChar('9'))) {
            } else if (_char == QChar('.')) {
            } else {
                bResult = false;
                break;
            }
        }
    }

    return bResult;
}

QString XBinary::cleanString(const QString &sString)
{
    QString sResult;

    qint32 nNumberOfChars = sString.count();

    for (qint32 i = 0; i < nNumberOfChars; i++) {
        QChar _char = sString.at(i);

        bool bAdd = false;

        if (_char.isLetterOrNumber() || _char.isPunct()) {
            bAdd = true;
        }

        if (bAdd) {
            sResult.append(_char);
        }
    }

    return sResult;
}

XBinary::XVARIANT XBinary::getXVariant(bool bValue)
{
    XVARIANT result = {};

    result.mode = MODE_BIT;
    result.var.v_bool = bValue;

    return result;
}

XBinary::XVARIANT XBinary::getXVariant(quint8 nValue)
{
    XVARIANT result = {};

    result.mode = MODE_8;
    result.var.v_uint8 = nValue;

    return result;
}

XBinary::XVARIANT XBinary::getXVariant(quint16 nValue, bool bIsBigEndian)
{
    XVARIANT result = {};

    result.bIsBigEndian = bIsBigEndian;
    result.mode = MODE_16;
    result.var.v_uint16 = nValue;

    return result;
}

XBinary::XVARIANT XBinary::getXVariant(quint32 nValue, bool bIsBigEndian)
{
    XVARIANT result = {};

    result.bIsBigEndian = bIsBigEndian;
    result.mode = MODE_32;
    result.var.v_uint32 = nValue;

    return result;
}

XBinary::XVARIANT XBinary::getXVariant(quint64 nValue, bool bIsBigEndian)
{
    XVARIANT result = {};

    result.bIsBigEndian = bIsBigEndian;
    result.mode = MODE_64;
    result.var.v_uint64 = nValue;

    return result;
}

XBinary::XVARIANT XBinary::getXVariant(quint64 nValue[2], bool bIsBigEndian)
{
    XVARIANT result = {};

    result.bIsBigEndian = bIsBigEndian;
    result.mode = MODE_128;
    result.var.v_uint128[0] = nValue[0];
    result.var.v_uint128[1] = nValue[1];

    return result;
}

XBinary::XVARIANT XBinary::getXVariant(quint64 nLow, quint64 nHigh, bool bIsBigEndian)
{
    XVARIANT result = {};

    result.bIsBigEndian = bIsBigEndian;
    result.mode = MODE_128;
    result.var.v_uint128[0] = nLow;
    result.var.v_uint128[1] = nHigh;

    return result;
}

XBinary::XVARIANT XBinary::getXVariant(quint64 nLow1, quint64 nLow2, quint64 nHigh1, quint64 nHigh2, bool bIsBigEndian)
{
    XVARIANT result = {};

    result.bIsBigEndian = bIsBigEndian;
    result.mode = MODE_256;
    result.var.v_uint256[0] = nLow1;
    result.var.v_uint256[1] = nLow2;
    result.var.v_uint256[2] = nHigh1;
    result.var.v_uint256[3] = nHigh2;

    return result;
}

XBinary::XVARIANT XBinary::getXVariant(quint8 nValue[10], bool bIsBigEndian)
{
    XVARIANT result = {};

    result.bIsBigEndian = bIsBigEndian;
    result.mode = MODE_FREG;

    for (qint32 i = 0; i < 10; i++) {
        result.var.v_freg[i] = nValue[i];
    }

    return result;
}

quint64 XBinary::xVariantToQword(XVARIANT xvariant)
{
    quint64 nResult = 0;

    if (xvariant.mode == MODE_8) nResult = xvariant.var.v_uint8;
    else if (xvariant.mode == MODE_16) nResult = xvariant.var.v_uint16;
    else if (xvariant.mode == MODE_32) nResult = xvariant.var.v_uint32;
    else if (xvariant.mode == MODE_64) nResult = xvariant.var.v_uint64;

    return nResult;
}

quint32 XBinary::getDwordFromQword(quint64 nValue, qint32 nIndex)
{
    quint32 nResult = 0;

    if (nIndex < 2) {
        nValue = nValue >> (nIndex * 32);

        nResult = nValue & 0xFFFFFFFF;
    }

    return nResult;
}

quint16 XBinary::getWordFromQword(quint64 nValue, qint32 nIndex)
{
    quint16 nResult = 0;

    if (nIndex < 4) {
        nValue = nValue >> (nIndex * 16);

        nResult = nValue & 0xFFFF;
    }

    return nResult;
}

quint8 XBinary::getByteFromQword(quint64 nValue, qint32 nIndex)
{
    quint8 nResult = 0;

    if (nIndex < 8) {
        nValue = nValue >> (nIndex * 8);

        nResult = nValue & 0xFF;
    }

    return nResult;
}

quint16 XBinary::getWordFromDword(quint32 nValue, qint32 nIndex)
{
    quint16 nResult = 0;

    if (nIndex < 2) {
        nValue = nValue >> (nIndex * 16);

        nResult = nValue & 0xFFFF;
    }

    return nResult;
}

quint8 XBinary::getByteFromDword(quint32 nValue, qint32 nIndex)
{
    quint8 nResult = 0;

    if (nIndex < 4) {
        nValue = nValue >> (nIndex * 8);

        nResult = nValue & 0xFF;
    }

    return nResult;
}

quint8 XBinary::getByteFromWord(quint16 nValue, qint32 nIndex)
{
    quint8 nResult = 0;

    if (nIndex < 2) {
        nValue = nValue >> (nIndex * 8);

        nResult = nValue & 0xFF;
    }

    return nResult;
}

bool XBinary::getBitFromByte(quint8 nValue, qint32 nIndex)
{
    bool bResult = false;

    if (nIndex < 8) {
        bResult = (nValue >> nIndex) & 0x1;
    }

    return bResult;
}

bool XBinary::getBitFromWord(quint16 nValue, qint32 nIndex)
{
    bool bResult = false;

    if (nIndex < 16) {
        bResult = (nValue >> nIndex) & 0x1;
    }

    return bResult;
}

bool XBinary::getBitFromDword(quint32 nValue, qint32 nIndex)
{
    bool bResult = false;

    if (nIndex < 32) {
        bResult = (nValue >> nIndex) & 0x1;
    }

    return bResult;
}

bool XBinary::getBitFromQword(quint64 nValue, qint32 nIndex)
{
    bool bResult = false;

    if (nIndex < 64) {
        bResult = (nValue >> nIndex) & 0x1;
    }

    return bResult;
}

quint64 XBinary::setDwordToQword(quint64 nInit, quint32 nValue, qint32 nIndex)
{
    quint64 nResult = nInit;

    if (nIndex < 2) {
        quint64 nFF = 0xFFFFFFFF;
        quint64 _nValue = nValue;

        nFF = nFF << (nIndex * 32);
        _nValue = _nValue << (nIndex * 32);

        nResult = nResult & (~nFF);
        nResult = nResult | _nValue;
    }

    return nResult;
}

quint64 XBinary::setWordToQword(quint64 nInit, quint16 nValue, qint32 nIndex)
{
    quint64 nResult = nInit;

    if (nIndex < 4) {
        quint64 nFF = 0xFFFF;
        quint64 _nValue = nValue;

        nFF = nFF << (nIndex * 16);
        _nValue = _nValue << (nIndex * 16);

        nResult = nResult & (~nFF);
        nResult = nResult | _nValue;
    }

    return nResult;
}

quint64 XBinary::setByteToQword(quint64 nInit, quint8 nValue, qint32 nIndex)
{
    quint64 nResult = nInit;

    if (nIndex < 8) {
        quint64 nFF = 0xFF;
        quint64 _nValue = nValue;

        nFF = nFF << (nIndex * 8);
        _nValue = _nValue << (nIndex * 8);

        nResult = nResult & (~nFF);
        nResult = nResult | _nValue;
    }

    return nResult;
}

quint32 XBinary::setWordToDword(quint32 nInit, quint16 nValue, qint32 nIndex)
{
    quint32 nResult = nInit;

    if (nIndex < 2) {
        quint32 nFF = 0xFFFF;
        quint32 _nValue = nValue;

        nFF = nFF << (nIndex * 16);
        _nValue = _nValue << (nIndex * 16);

        nResult = nResult & (~nFF);
        nResult = nResult | _nValue;
    }

    return nResult;
}

quint32 XBinary::setByteToDword(quint32 nInit, quint8 nValue, qint32 nIndex)
{
    quint32 nResult = nInit;

    if (nIndex < 4) {
        quint32 nFF = 0xFF;
        quint32 _nValue = nValue;

        nFF = nFF << (nIndex * 8);
        _nValue = _nValue << (nIndex * 8);

        nResult = nResult & (~nFF);
        nResult = nResult | _nValue;
    }

    return nResult;
}

quint16 XBinary::setByteToWord(quint16 nInit, quint8 nValue, qint32 nIndex)
{
    quint16 nResult = nInit;

    if (nIndex < 2) {
        quint16 nFF = 0xFF;
        quint16 _nValue = nValue;

        nFF = nFF << (nIndex * 8);
        _nValue = _nValue << (nIndex * 8);

        nResult = nResult & (~nFF);
        nResult = nResult | _nValue;
    }

    return nResult;
}

quint8 XBinary::setBitToByte(quint8 nInit, bool bValue, qint32 nIndex)
{
    quint8 nResult = nInit;

    if (nIndex < 8) {
        quint8 nFF = 0x01;
        quint8 _nValue = (quint8)bValue;

        nFF = nFF << nIndex;
        _nValue = _nValue << nIndex;

        nResult = nResult & (~nFF);
        nResult = nResult | _nValue;
    }

    return nResult;
}

quint16 XBinary::setBitToWord(quint16 nInit, bool bValue, qint32 nIndex)
{
    quint16 nResult = nInit;

    if (nIndex < 16) {
        quint16 nFF = 0x01;
        quint16 _nValue = (quint16)bValue;

        nFF = nFF << nIndex;
        _nValue = _nValue << nIndex;

        nResult = nResult & (~nFF);
        nResult = nResult | _nValue;
    }

    return nResult;
}

quint32 XBinary::setBitToDword(quint32 nInit, bool bValue, qint32 nIndex)
{
    quint32 nResult = nInit;

    if (nIndex < 32) {
        quint32 nFF = 0x01;
        quint32 _nValue = (quint32)bValue;

        nFF = nFF << nIndex;
        _nValue = _nValue << nIndex;

        nResult = nResult & (~nFF);
        nResult = nResult | _nValue;
    }

    return nResult;
}

quint64 XBinary::setBitToQword(quint64 nInit, bool bValue, qint32 nIndex)
{
    quint64 nResult = nInit;

    if (nIndex < 64) {
        quint64 nFF = 0x01;
        quint64 _nValue = (quint64)bValue;

        nFF = nFF << nIndex;
        _nValue = _nValue << nIndex;

        nResult = nResult & (~nFF);
        nResult = nResult | _nValue;
    }

    return nResult;
}

bool XBinary::isXVariantEqual(XVARIANT value1, XVARIANT value2)
{
    bool bResult = false;

    if ((value1.mode == value2.mode) && (value1.bIsBigEndian == value2.bIsBigEndian)) {
        if (value1.mode == MODE_BIT) bResult = (value1.var.v_bool == value2.var.v_bool);
        else if (value1.mode == MODE_8) bResult = (value1.var.v_uint8 == value2.var.v_uint8);
        else if (value1.mode == MODE_16) bResult = (value1.var.v_uint16 == value2.var.v_uint16);
        else if (value1.mode == MODE_32) bResult = (value1.var.v_uint32 == value2.var.v_uint32);
        else if (value1.mode == MODE_64) bResult = (value1.var.v_uint64 == value2.var.v_uint64);
        else if (value1.mode == MODE_128) {
            bResult = (value1.var.v_uint128[0] == value2.var.v_uint128[0]) && (value1.var.v_uint128[1] == value2.var.v_uint128[1]);
        } else if (value1.mode == MODE_256) {
            bResult = true;

            for (qint32 i = 0; i < 4; i++) {
                if (value1.var.v_uint256[i] != value2.var.v_uint256[i]) {
                    bResult = false;
                    break;
                }
            }
        } else if (value1.mode == MODE_FREG) {
            bResult = true;

            for (qint32 i = 0; i < 10; i++) {
                if (value1.var.v_freg[i] != value2.var.v_freg[i]) {
                    bResult = false;
                    break;
                }
            }
        }
    }

    return bResult;
}

void XBinary::clearXVariant(XVARIANT *pVar)
{
    if (pVar->mode == MODE_BIT) pVar->var.v_bool = false;
    else if (pVar->mode == MODE_8) pVar->var.v_uint8 = 0;
    else if (pVar->mode == MODE_16) pVar->var.v_uint16 = 0;
    else if (pVar->mode == MODE_32) pVar->var.v_uint32 = 0;
    else if (pVar->mode == MODE_64) pVar->var.v_uint64 = 0;
    else if (pVar->mode == MODE_128) {
        pVar->var.v_uint128[0] = 0;
        pVar->var.v_uint128[1] = 0;
    } else if (pVar->mode == MODE_FREG) {
        for (qint32 i = 0; i < 10; i++) {
            pVar->var.v_freg[i] = 0;
        }
    }
}

XBinary::MODE XBinary::getModeOS()
{
    XBinary::MODE modeResult = XBinary::MODE_UNKNOWN;

    if (sizeof(void *) == 8) {
        modeResult = XBinary::MODE_64;
    } else {
        modeResult = XBinary::MODE_32;
    }

    return modeResult;
}

XBinary::PDSTRUCT XBinary::createPdStruct()
{
    PDSTRUCT result = {};

    result.bIsStop = false;
    result.nFinished = false;

    for (qint32 i = 0; i < N_NUMBER_PDRECORDS; i++) {
        result._pdRecord[i].bIsValid = false;
        result._pdRecord[i].nCurrent = 0;
        result._pdRecord[i].nTotal = 0;
        result._pdRecord[i].sStatus = "";
    }

    return result;
}

void XBinary::setPdStructInit(PDSTRUCT *pPdStruct, qint32 nIndex, qint64 nTotal)
{
    if ((nIndex >= 0) && (nIndex < N_NUMBER_PDRECORDS)) {
        pPdStruct->_pdRecord[nIndex].bIsValid = true;
        pPdStruct->_pdRecord[nIndex].nCurrent = 0;
        pPdStruct->_pdRecord[nIndex].nTotal = nTotal;
        pPdStruct->_pdRecord[nIndex].sStatus = "";
    }
}

void XBinary::setPdStructTotal(PDSTRUCT *pPdStruct, qint32 nIndex, qint64 nValue)
{
    if ((nIndex >= 0) && (nIndex < N_NUMBER_PDRECORDS)) {
        pPdStruct->_pdRecord[nIndex].nTotal = nValue;
    }
}

void XBinary::setPdStructCurrent(PDSTRUCT *pPdStruct, qint32 nIndex, qint64 nValue)
{
    if ((nIndex >= 0) && (nIndex < N_NUMBER_PDRECORDS)) {
        pPdStruct->_pdRecord[nIndex].nCurrent = nValue;
    }
}

void XBinary::setPdStructCurrentIncrement(PDSTRUCT *pPdStruct, qint32 nIndex)
{
    if ((nIndex >= 0) && (nIndex < N_NUMBER_PDRECORDS)) {
        pPdStruct->_pdRecord[nIndex].nCurrent++;
    }
}

void XBinary::setPdStructStatus(PDSTRUCT *pPdStruct, qint32 nIndex, const QString &sStatus)
{
    if ((nIndex >= 0) && (nIndex < N_NUMBER_PDRECORDS)) {
        pPdStruct->_pdRecord[nIndex].sStatus = sStatus;
    }
}

void XBinary::setPdStructFinished(PDSTRUCT *pPdStruct, qint32 nIndex)
{
    if ((nIndex >= 0) && (nIndex < N_NUMBER_PDRECORDS)) {
        pPdStruct->_pdRecord[nIndex].bIsValid = false;
        pPdStruct->_pdRecord[nIndex].nCurrent = 0;
        pPdStruct->_pdRecord[nIndex].nTotal = 0;
        pPdStruct->_pdRecord[nIndex].sStatus.clear();

        pPdStruct->nFinished++;
    }
}

qint32 XBinary::getFreeIndex(PDSTRUCT *pPdStruct)
{
    qint32 nResult = -1;

    for (qint32 i = 0; i < N_NUMBER_PDRECORDS; i++) {
        if (!pPdStruct->_pdRecord[i].bIsValid) {
            nResult = i;

            break;
        }
    }

    return nResult;
}

bool XBinary::isPdStructFinished(PDSTRUCT *pPdStruct)
{
    bool bResult = true;

    for (qint32 i = 0; i < N_NUMBER_PDRECORDS; i++) {
        if (pPdStruct->_pdRecord[i].bIsValid) {
            bResult = false;

            break;
        }
    }

    if (pPdStruct->nFinished == 0) {
        bResult = false;
    }

    return bResult;
}

bool XBinary::isPdStructSuccess(PDSTRUCT *pPdStruct)
{
    bool bResult = false;

    if (isPdStructFinished(pPdStruct) && (!(pPdStruct->bIsStop))) {
        if (pPdStruct->nFinished > 0) {
            bResult = true;
        }
    }

    if (pPdStruct->bCriticalError) {
        bResult = false;
    }

    return bResult;
}

qint32 XBinary::getPdStructPercentage(PDSTRUCT *pPdStruct)
{
    qint32 nResult = 0;

    for (qint32 i = 0; i < N_NUMBER_PDRECORDS; i++) {
        if (pPdStruct->_pdRecord[i].nTotal) {
            nResult = (pPdStruct->_pdRecord[i].nCurrent * 100) / pPdStruct->_pdRecord[i].nTotal;

            break;
        }
    }

    return nResult;
}

XBinary::REGION_FILL XBinary::getRegionFill(qint64 nOffset, qint64 nSize, qint32 nAlignment)
{
    REGION_FILL result = {};
    result.nByte = read_uint8(nOffset);

    qint64 nMaxOffset = qMin(g_pDevice->size(), nOffset + nSize);
    char *pData = new char[nAlignment];

    bool bError = false;

    for (qint64 nCurrentOffset = nOffset; nCurrentOffset < nMaxOffset;) {
        qint64 nDataSize = qMin((qint64)nAlignment, nMaxOffset - nCurrentOffset);

        if (nDataSize < nAlignment) {
            break;
        }

        read_array(nCurrentOffset, pData, nDataSize);

        for (qint32 i = 0; i < nDataSize; i++) {
            if ((quint8)(pData[i]) != result.nByte) {
                bError = true;
                break;
            }
        }

        if (bError) {
            break;
        }

        result.nSize += nDataSize;

        nCurrentOffset += nDataSize;
    }

    delete[] pData;

    return result;
}

QString XBinary::getDataString(char *pData, qint32 nDataSize, const QString &sBaseType, bool bIsBigEndian)
{
    // TODO optimize
    QString sResult;

    if ((sBaseType == "db") || (sBaseType == "dw") || (sBaseType == "dd") || (sBaseType == "dq")) {
        for (qint32 i = 0; i < nDataSize;) {
            if (i > 0) {
                sResult.append(", ");
            }

            QString sString;
            qint32 _nSize = 1;

            if (sBaseType == "db") {
                sString = valueToHex(*(quint8 *)(pData));
                _nSize = 1;
            } else if (sBaseType == "dw") {
                sString = valueToHex(*(quint16 *)(pData), bIsBigEndian);
                _nSize = 2;
            } else if (sBaseType == "dd") {
                sString = valueToHex(*(quint32 *)(pData), bIsBigEndian);
                _nSize = 4;
            } else if (sBaseType == "dq") {
                sString = valueToHex(*(quint64 *)(pData), bIsBigEndian);
                _nSize = 8;
            }

            sResult.append("0x" + sString);
            pData += _nSize;
            i += _nSize;
        }
    }

    return sResult;
}

QList<XBinary::HREGION> XBinary::_getHRegions(_MEMORY_MAP *pMemoryMap, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    QList<XBinary::HREGION> listResult;

    qint32 nNumberOfRecords = pMemoryMap->listRecords.count();

    for (qint32 i = 0; (i < nNumberOfRecords) && (!(pPdStruct->bIsStop)); i++) {
        HREGION region = {};
        region.nAddress = pMemoryMap->listRecords.at(i).nAddress;
        region.nOffset = pMemoryMap->listRecords.at(i).nOffset;
        region.nSize = pMemoryMap->listRecords.at(i).nSize;
        region.sName = pMemoryMap->listRecords.at(i).sName;

        listResult.append(region);
    }

    return listResult;
}

QList<XBinary::HREGION> XBinary::getHighlights(_MEMORY_MAP *pMemoryMap, const HLOPTIONS &hlOptions, PDSTRUCT *pPdStruct)
{
    QList<XBinary::HREGION> listResult;

    if (hlOptions.bRegions) {
        listResult.append(_getHRegions(pMemoryMap, pPdStruct));
    }

    return listResult;
}

qint64 XBinary::align_up(qint64 nValue, qint64 nAlignment)
{
    qint64 nResult = nValue;

    if (nAlignment) {
        qint64 r = nValue % nAlignment;

        if (r) {
            nResult += (nAlignment - r);
        }
    }

    return nResult;
}

qint64 XBinary::align_down(qint64 nValue, qint64 nAlignment)
{
    qint64 nResult = nValue;

    if (nAlignment) {
        qint64 r = nValue % nAlignment;

        if (r) {
            nResult -= r;
        }
    }

    return nResult;
}

char *XBinary::strCopy(char *pszDest, char *pszSource)
{
    return strcpy(pszDest, pszSource);
}

QString XBinary::getAndroidVersionFromApi(quint32 nAPI)
{
    QString sResult = tr("Unknown");

    // https://developer.android.com/tools/releases/platforms
    if (nAPI == 1) sResult = QString("1.0");
    else if (nAPI == 2) sResult = QString("1.1");
    else if (nAPI == 3) sResult = QString("1.5");
    else if (nAPI == 4) sResult = QString("1.6");
    else if (nAPI == 5) sResult = QString("2.0");
    else if (nAPI == 6) sResult = QString("2.0.1");
    else if (nAPI == 7) sResult = QString("2.1");
    else if (nAPI == 8) sResult = QString("2.2.X");
    else if (nAPI == 9) sResult = QString("2.3-2.3.2");
    else if (nAPI == 10) sResult = QString("2.3.3-2.3.7");
    else if (nAPI == 11) sResult = QString("3.0");
    else if (nAPI == 12) sResult = QString("3.1");
    else if (nAPI == 13) sResult = QString("3.2.X");
    else if (nAPI == 14) sResult = QString("4.0.1-4.0.2");
    else if (nAPI == 15) sResult = QString("4.0.3-4.0.4");
    else if (nAPI == 16) sResult = QString("4.1.X");
    else if (nAPI == 17) sResult = QString("4.2.X");
    else if (nAPI == 18) sResult = QString("4.3.X");
    else if (nAPI == 19) sResult = QString("4.4-4.4.4");
    else if (nAPI == 20) sResult = QString("4.4W");
    else if (nAPI == 21) sResult = QString("5.0");
    else if (nAPI == 22) sResult = QString("5.1");
    else if (nAPI == 23) sResult = QString("6.0");
    else if (nAPI == 24) sResult = QString("7.0");
    else if (nAPI == 25) sResult = QString("7.1");
    else if (nAPI == 26) sResult = QString("8.0");
    else if (nAPI == 27) sResult = QString("8.1");
    else if (nAPI == 28) sResult = QString("9.0");
    else if (nAPI == 29) sResult = QString("10.0");
    else if (nAPI == 30) sResult = QString("11.0");
    else if (nAPI == 31) sResult = QString("12.0");
    else if (nAPI == 32) sResult = QString("12.1");
    else if (nAPI == 33) sResult = QString("13.0");
    else if (nAPI == 34) sResult = QString("14.0");
    else if (nAPI == 35) sResult = QString("15.0");

    return sResult;
}

QString XBinary::_fromWCharArray(const wchar_t *pWString, qint32 size)
{
    QString sResult;

#if (QT_VERSION_MAJOR < 5)  // TODO Check
    sResult = QString::fromWCharArray(pWString, size);
    // set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:wchar_t-")
    // set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zc:wchar_t-")
    // set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zc:wchar_t-")
    // set (CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /Zc:wchar_t-")
#else
    sResult = QString::fromWCharArray(pWString, size);
#endif

    return sResult;
}

qint32 XBinary::_toWCharArray(const QString &sString, wchar_t *pWString)
{
    qint32 nResult = 0;

#if (QT_VERSION_MAJOR < 5)                     // TODO Check
    nResult = sString.toWCharArray(pWString);  // TODO
    // set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:wchar_t-")
    // set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zc:wchar_t-")
    // set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zc:wchar_t-")
    // set (CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /Zc:wchar_t-")
#else
    nResult = sString.toWCharArray(pWString);
#endif

    return nResult;
}

QString XBinary::dataToString(const QByteArray &baData, DSMODE dsmode)
{
    QString sResult;

    qint32 nSize = baData.size();
    sResult.reserve(nSize);

    for (qint32 i = 0; i < nSize; i++) {
        QChar _char = QChar(baData.at(i));
        if (dsmode == DSMODE_NONE) {
            sResult += _char;
        } else if (dsmode == DSMODE_NOPRINT_TO_DOT) {
            if (_char.isPrint()) {
                sResult += _char;
            } else {
                sResult += QChar('.');
            }
        }
    }

    return sResult;
}

QList<XBinary::SIGNATURE_RECORD> XBinary::getSignatureRecords(const QString &sSignature, bool *pbValid, PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    QList<SIGNATURE_RECORD> listResult;

    qint32 nSignatureSize = sSignature.size();

    for (qint32 i = 0; i < nSignatureSize;) {
        QChar cSymbol = sSignature.at(i);
        QChar cSymbol2;

        if ((cSymbol == QChar('%')) || (cSymbol == QChar('!')) || (cSymbol == QChar('_'))) {
            if ((i + 1) < (nSignatureSize)) {
                cSymbol2 = sSignature.at(i + 1);
            }
        }

        if (cSymbol == QChar('.')) {
            i += _getSignatureSkip(&listResult, sSignature, i);
        } else if (cSymbol == QChar('*')) {
            i += _getSignatureNotNull(&listResult, sSignature, i);
        } else if ((cSymbol == QChar('%')) && (cSymbol2 == QChar('%'))) {
            i += _getSignatureANSI(&listResult, sSignature, i);
        } else if ((cSymbol == QChar('!')) && (cSymbol2 == QChar('%'))) {
            i += _getSignatureNotANSI(&listResult, sSignature, i);
        } else if ((cSymbol == QChar('_')) && (cSymbol2 == QChar('%'))) {
            i += _getSignatureNotANSIAndNull(&listResult, sSignature, i);
        } else if (cSymbol == QChar('+')) {
            i += _getSignatureDelta(&listResult, sSignature, i, pbValid, pPdStruct);
        } else if (cSymbol == QChar('$')) {
            i += _getSignatureRelOffset(&listResult, sSignature, i);
        } else if (cSymbol == QChar('#')) {  // TODO Check []
            i += _getSignatureAddress(&listResult, sSignature, i);
        } else {
            qint32 nBytes = _getSignatureBytes(&listResult, sSignature, i, pbValid, pPdStruct);

            if (nBytes) {
                i += nBytes;
            } else {
                break;
            }
        }
    }

    return listResult;
}

bool XBinary::_compareSignature(_MEMORY_MAP *pMemoryMap, QList<XBinary::SIGNATURE_RECORD> *pListSignatureRecords, qint64 nOffset)
{
    // TODO optimize

    qint32 nNumberOfSignatures = pListSignatureRecords->count();

    for (qint32 i = 0; i < nNumberOfSignatures; i++) {
        switch (pListSignatureRecords->at(i).st) {
            case XBinary::ST_COMPAREBYTES: {
                QByteArray baData = read_array(nOffset, pListSignatureRecords->at(i).baData.size());

                if (baData.size() != pListSignatureRecords->at(i).baData.size()) {
                    return false;
                }

                if (!compareMemory(baData.data(), (char *)(pListSignatureRecords->at(i).baData.data()), baData.size())) {
                    return false;
                }

                nOffset += baData.size();
            } break;

            case XBinary::ST_NOTNULL:
            case XBinary::ST_ANSI:
            case XBinary::ST_NOTANSI:
            case XBinary::ST_NOTANSIANDNULL: {
                QByteArray baData = read_array(nOffset, pListSignatureRecords->at(i).nSize);

                if (baData.size() != pListSignatureRecords->at(i).nSize) {
                    return false;
                }

                if (pListSignatureRecords->at(i).st == XBinary::ST_NOTNULL) {
                    if (!_isMemoryNotNull(baData.data(), baData.size())) {
                        return false;
                    }
                } else if (pListSignatureRecords->at(i).st == XBinary::ST_ANSI) {
                    if (!_isMemoryAnsi(baData.data(), baData.size())) {
                        return false;
                    }
                } else if (pListSignatureRecords->at(i).st == XBinary::ST_NOTANSI) {
                    if (!_isMemoryNotAnsi(baData.data(), baData.size())) {
                        return false;
                    }
                } else if (pListSignatureRecords->at(i).st == XBinary::ST_NOTANSIANDNULL) {
                    if (!_isMemoryNotAnsiAndNull(baData.data(), baData.size())) {
                        return false;
                    }
                }

                nOffset += baData.size();
            } break;

            case XBinary::ST_FINDBYTES: {
                qint64 nResult =
                    find_byteArray(nOffset, pListSignatureRecords->at(i).nFindDelta + pListSignatureRecords->at(i).baData.size(), pListSignatureRecords->at(i).baData);

                if (nResult == -1) {
                    return false;
                }

                nOffset = nResult + pListSignatureRecords->at(i).baData.size();
            } break;

            case XBinary::ST_SKIP:
                // TODO Check
                nOffset += pListSignatureRecords->at(i).nSize;
                break;

            case XBinary::ST_RELOFFSET: {
                qint64 nValue = 0;

                switch (pListSignatureRecords->at(i).nSizeOfAddr) {
                    case 1: nValue = 1 + read_int8(nOffset); break;

                    case 2: nValue = 2 + read_int16(nOffset, isBigEndian(pMemoryMap)); break;

                    case 4: nValue = 4 + read_int32(nOffset, isBigEndian(pMemoryMap)); break;

                    case 8: nValue = 8 + read_int64(nOffset, isBigEndian(pMemoryMap)); break;
                }

                if ((pMemoryMap->fileType == FT_COM) || (pMemoryMap->fileType == FT_MSDOS)) {
                    nOffset += nValue;
                } else {
                    XADDR _nAddress = offsetToAddress(pMemoryMap, nOffset);
                    _nAddress += nValue;
                    nOffset = addressToOffset(pMemoryMap, _nAddress);
                }
            }

            break;

            case XBinary::ST_ADDRESS: {
                XADDR _nAddress = 0;
                switch (pListSignatureRecords->at(i).nSizeOfAddr) {
                    case 1: _nAddress = read_uint8(nOffset); break;

                    case 2: _nAddress = read_uint16(nOffset, isBigEndian(pMemoryMap)); break;

                    case 4: _nAddress = read_uint32(nOffset, isBigEndian(pMemoryMap)); break;

                    case 8: _nAddress = read_uint64(nOffset, isBigEndian(pMemoryMap)); break;
                }

                if (pMemoryMap->fileType == FT_MSDOS) {
                    if (pListSignatureRecords->at(i).nSizeOfAddr == 2) {
                        _nAddress += pMemoryMap->nCodeBase;
                        nOffset = addressToOffset(pMemoryMap, _nAddress);  // TODO!
                    } else if (pListSignatureRecords->at(i).nSizeOfAddr == 4) {
                        quint16 nLow = (quint16)_nAddress;
                        quint16 nHigh = (quint16)(_nAddress >> 16);
                        nOffset = pMemoryMap->nStartLoadOffset + getSegmentAddress(nHigh, nLow);
                    }
                } else {
                    nOffset = addressToOffset(pMemoryMap, _nAddress);  // TODO!
                }
            } break;

                if (!isOffsetValid(pMemoryMap, nOffset)) {
                    return false;
                }
        }
    }
    //    CompareBytes,
    //    RelOffsetFix,
    //    RelOffset,
    //    Address

    return true;
}

qint32 XBinary::_getSignatureSkip(QList<XBinary::SIGNATURE_RECORD> *pListSignatureRecords, const QString &sSignature, qint32 nStartIndex)
{
    qint32 nResult = 0;
    qint32 nSignatureSize = sSignature.size();

    for (qint32 i = nStartIndex; i < nSignatureSize; i++) {
        if (sSignature.at(i) == QChar('.')) {
            nResult++;
        } else {
            break;
        }
    }

    if (nResult) {
        SIGNATURE_RECORD record = {};

        record.st = XBinary::ST_SKIP;
        record.nSizeOfAddr = 0;
        record.nSize = nResult / 2;

        pListSignatureRecords->append(record);
    }

    return nResult;
}

qint32 XBinary::_getSignatureNotNull(QList<SIGNATURE_RECORD> *pListSignatureRecords, const QString &sSignature, qint32 nStartIndex)
{
    qint32 nResult = 0;
    qint32 nSignatureSize = sSignature.size();

    for (qint32 i = nStartIndex; i < nSignatureSize; i++) {
        if (sSignature.at(i) == QChar('*')) {
            nResult++;
        } else {
            break;
        }
    }

    if (nResult) {
        SIGNATURE_RECORD record = {};

        record.st = XBinary::ST_NOTNULL;
        record.nSizeOfAddr = 0;
        record.nSize = nResult / 2;

        pListSignatureRecords->append(record);
    }

    return nResult;
}

qint32 XBinary::_getSignatureANSI(QList<SIGNATURE_RECORD> *pListSignatureRecords, const QString &sSignature, qint32 nStartIndex)
{
    qint32 nResult = 0;
    qint32 nSignatureSize = sSignature.size();

    for (qint32 i = nStartIndex; i < nSignatureSize; i += 2) {
        if (sSignature.mid(i, 2) == "%%") {
            nResult += 2;
        } else {
            break;
        }
    }

    if (nResult) {
        SIGNATURE_RECORD record = {};

        record.st = XBinary::ST_ANSI;
        record.nSizeOfAddr = 0;
        record.nSize = nResult / 2;

        pListSignatureRecords->append(record);
    }

    return nResult;
}

qint32 XBinary::_getSignatureNotANSI(QList<SIGNATURE_RECORD> *pListSignatureRecords, const QString &sSignature, qint32 nStartIndex)
{
    qint32 nResult = 0;
    qint32 nSignatureSize = sSignature.size();

    for (qint32 i = nStartIndex; i < nSignatureSize; i += 2) {
        if (sSignature.mid(i, 2) == "!%") {
            nResult += 2;
        } else {
            break;
        }
    }

    if (nResult) {
        SIGNATURE_RECORD record = {};

        record.st = XBinary::ST_NOTANSI;
        record.nSizeOfAddr = 0;
        record.nSize = nResult / 2;

        pListSignatureRecords->append(record);
    }

    return nResult;
}

qint32 XBinary::_getSignatureNotANSIAndNull(QList<SIGNATURE_RECORD> *pListSignatureRecords, const QString &sSignature, qint32 nStartIndex)
{
    qint32 nResult = 0;
    qint32 nSignatureSize = sSignature.size();

    for (qint32 i = nStartIndex; i < nSignatureSize; i += 2) {
        if (sSignature.mid(i, 2) == "_%") {
            nResult += 2;
        } else {
            break;
        }
    }

    if (nResult) {
        SIGNATURE_RECORD record = {};

        record.st = XBinary::ST_NOTANSIANDNULL;
        record.nSizeOfAddr = 0;
        record.nSize = nResult / 2;

        pListSignatureRecords->append(record);
    }

    return nResult;
}

qint32 XBinary::_getSignatureDelta(QList<XBinary::SIGNATURE_RECORD> *pListSignatureRecords, const QString &sSignature, qint32 nStartIndex, bool *pbValid,
                                   PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }
    // TODO Check!!!
    qint32 nResult = 0;
    qint32 nSignatureSize = sSignature.size();

    for (qint32 i = nStartIndex; i < nSignatureSize; i++) {
        if (sSignature.at(i) == QChar('+')) {
            nResult++;
        } else {
            break;
        }
    }

    if (nResult) {
        QList<XBinary::SIGNATURE_RECORD> _listSignatureRecords;

        qint32 nTemp = _getSignatureBytes(&_listSignatureRecords, sSignature, nStartIndex + nResult, pbValid, pPdStruct);

        if (_listSignatureRecords.count()) {
            SIGNATURE_RECORD record = {};

            record.st = XBinary::ST_FINDBYTES;
            record.nSizeOfAddr = 0;
            record.nSize = 0;
            record.baData = _listSignatureRecords.at(0).baData;
            record.nFindDelta = 32 * nResult;

            pListSignatureRecords->append(record);

            nResult += nTemp;
        }
    }

    return nResult;
}

qint32 XBinary::_getSignatureRelOffset(QList<XBinary::SIGNATURE_RECORD> *pListSignatureRecords, const QString &sSignature, qint32 nStartIndex)
{
    qint32 nResult = 0;

    qint32 nSignatureSize = sSignature.size();

    for (qint32 i = nStartIndex; i < nSignatureSize; i++) {
        if (sSignature.at(i) == QChar('$')) {
            nResult++;
        } else {
            break;
        }
    }

    if (nResult) {
        SIGNATURE_RECORD record = {};

        record.st = XBinary::ST_RELOFFSET;
        record.nSizeOfAddr = nResult / 2;
        record.nSize = 0;

        pListSignatureRecords->append(record);
    }

    return nResult;
}

qint32 XBinary::_getSignatureAddress(QList<XBinary::SIGNATURE_RECORD> *pListSignatureRecords, const QString &sSignature, qint32 nStartIndex)
{
    qint32 nResult = 0;

    qint32 nSignatureSize = sSignature.size();
    QString sBaseAddress;
    bool bIsBaseAddress = false;
    qint32 nSizeOfAddress = 0;

    for (qint32 i = nStartIndex; i < nSignatureSize; i++) {
        if (sSignature.at(i) == QChar('#')) {
            nResult++;
            nSizeOfAddress++;
        } else if (sSignature.at(i) == QChar('[')) {
            nResult++;
            bIsBaseAddress = true;
        } else if (sSignature.at(i) == QChar(']')) {
            nResult++;
            bIsBaseAddress = false;
        } else if (bIsBaseAddress) {
            nResult++;
            sBaseAddress.append(sSignature.at(i));
        } else {
            break;
        }
    }

    if (nResult) {
        SIGNATURE_RECORD record = {};

        record.st = XBinary::ST_ADDRESS;
        record.nSizeOfAddr = nSizeOfAddress / 2;
        record.nBaseAddress = sBaseAddress.toInt(0, 16);

        pListSignatureRecords->append(record);
    }

    return nResult;
}

qint32 XBinary::_getSignatureBytes(QList<XBinary::SIGNATURE_RECORD> *pListSignatureRecords, const QString &sSignature, qint32 nStartIndex, bool *pbValid,
                                   PDSTRUCT *pPdStruct)
{
    PDSTRUCT pdStructEmpty = XBinary::createPdStruct();

    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }

    qint32 nResult = 0;

    qint32 nSignatureSize = sSignature.size();
    QString sBytes;

    for (qint32 i = nStartIndex; i < nSignatureSize; i++) {
        if (((sSignature.at(i) >= QChar('a')) && (sSignature.at(i) <= QChar('f'))) || ((sSignature.at(i) >= QChar('0')) && (sSignature.at(i) <= QChar('9')))) {
            nResult++;
            sBytes.append(sSignature.at(i));
        } else if ((sSignature.at(i) == '.') || (sSignature.at(i) == '$') || (sSignature.at(i) == '#') || (sSignature.at(i) == '*') || (sSignature.at(i) == '!') ||
                   (sSignature.at(i) == '_') || (sSignature.at(i) == '%') || (sSignature.at(i) == '+')) {
            break;
        } else {
            *pbValid = false;
            break;
        }
    }

    if (nResult) {
        SIGNATURE_RECORD record = {};

        record.st = XBinary::ST_COMPAREBYTES;
        record.nSizeOfAddr = 0;
        record.baData = QByteArray::fromHex(sBytes.toUtf8());  // TODO Check
        record.nSize = record.baData.size();

        pListSignatureRecords->append(record);
    }

#ifdef QT_DEBUG
    if (!(*pbValid)) {
        pPdStruct->sInfoString = QString("%1: %2").arg(tr("Invalid signature"), sSignature);
    }
#endif

    return nResult;
}

qint64 XBinary::getPhysSize(char *pBuffer, qint64 nSize)
{
    while (nSize > 0) {
        char *pOffset = pBuffer + nSize - 1;

        if (*pOffset) {
            break;
        }

        nSize--;
    }

    return nSize;
}

bool XBinary::isEmptyData(char *pBuffer, qint64 nSize)  // TODO dwords
{
    bool bResult = true;

    for (qint64 i = 0; i < nSize; i++) {
        char *pOffset = (pBuffer + i);

        if (*pOffset) {
            bResult = false;
            break;
        }
    }

    return bResult;
}

bool XBinary::_isOffsetValid(qint64 nOffset)
{
    qint64 nFileSize = getSize();

    return (nOffset < nFileSize);
}

bool XBinary::isAddressPhysical(XADDR nAddress)
{
    _MEMORY_MAP memoryMap = getMemoryMap();

    return isAddressPhysical(&memoryMap, nAddress);
}
