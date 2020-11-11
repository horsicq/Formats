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
#ifndef XBINARY_H
#define XBINARY_H

#include <QObject>
#include <QIODevice>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QtEndian>
#include <QMap>
#include <QBuffer>
#include <QSet>
#include <QTemporaryFile>
#include <QCryptographicHash>
#include <QCoreApplication>
#if (QT_VERSION_MAJOR<5) // TODO Check
#include <QRegExp>
#else
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#endif
#include <math.h>
#include <QDebug>
#include <QElapsedTimer>
#if (QT_VERSION_MAJOR>=5)&&(QT_VERSION_MINOR>=10) // TODO CHECK
#include <QRandomGenerator>
#endif
#include <QDateTime>
#include "xmsdos_def.h"
#include "xne_def.h"
#include "xle_def.h"
#include "xpe_def.h"
#include "xelf_def.h"
#include "xmach_def.h"
#include "subdevice.h"

#define S_ALIGN_DOWN(value,align)   ((value)&~(align-1))
#define S_ALIGN_UP(value,align)     (((value)&(align-1))?S_ALIGN_DOWN(value,align)+align:value)
#define S_LOWORD(value)             ((quint16)((quint32)(value)&0xffff))
#define S_HIWORD(value)             ((quint16)((quint32)(value)>>16))

#ifdef Q_OS_MAC
#include <CoreFoundation/CoreFoundation.h> // Check
#endif

class XBinary : public QObject
{
    Q_OBJECT

static const double D_ENTROPY_THRESHOLD; // 6.5

public:
    struct DATASET
    {
        qint64 nOffset;
        qint64 nAddress;
        qint64 nSize;
        QString sName;
        quint32 nType;
        // Optional
        qint64 nStringTableOffset;
        qint64 nStringTableSize;
    };

    struct BYTE_COUNTS
    {
        qint64 nSize;
        qint64 nCount[256]; // TODO const
    };

    struct OS_ANSISTRING
    {
        qint64 nOffset;
        qint64 nSize;
        QString sAnsiString;
    };

    struct OFFSETSIZE
    {
        qint64 nOffset;
        qint64 nSize;
    };

    struct ADDRESSSIZE
    {
        qint64 nAddress;
        qint64 nSize;
    };

    enum ADDRESS_SEGMENT
    {
        ADDRESS_SEGMENT_UNKNOWN=-1,
        ADDRESS_SEGMENT_FLAT=0,
        ADDRESS_SEGMENT_CODE,
//        ADDRESS_SEGMENT_DATA
    };

    enum MMT
    {
        MMT_UNKNOWN=0,
        MMT_HEADER,
        MMT_LOADSECTION,    // Section in PE; LoadProgram in ELF
        MMT_NOLOADABLE,     // For ELF
        MMT_FILESEGMENT,
        MMT_OVERLAY
    };

    struct _MEMORY_RECORD
    {
        qint64 nOffset;
        qint64 nAddress;
        ADDRESS_SEGMENT segment;
        qint64 nSize;
        MMT type;
        qint32 nLoadSection;
        QString sName;
        qint32 nIndex;
        bool bIsVirtual;
    };

    enum FT
    {
        // TODO IMAGE
        FT_UNKNOWN=0,
        FT_BINARY,
        FT_BINARY16,
        FT_BINARY32,
        FT_BINARY64,
        FT_COM,
        FT_MSDOS,
        FT_NE,
        FT_LE,
        FT_LX,
        FT_PE,
        FT_PE32,
        FT_PE64,
        FT_ELF,
        FT_ELF32,
        FT_ELF64,
        FT_MACHO,
        FT_MACHO32,
        FT_MACHO64,
        // Extra
        FT_ZIP,
        FT_CAB,
        FT_RAR,
        FT_7Z,
        FT_PNG,
        FT_JPEG,
        FT_GIF,
        FT_TIFF,
        FT_DEX,
        FT_ANDROIDASRC,
        FT_ANDROIDXML,
        FT_APK,
        FT_JAR,
        FT_TEXT,
        FT_PLAINTEXT,
        FT_UTF8,
        FT_UNICODE,
        FT_UNICODE_LE,
        FT_UNICODE_BE,
        FT_DOCUMENT,
        FT_IMAGE,
        FT_ARCHIVE
    };

    enum MODE
    {
        MODE_UNKNOWN=0,
        MODE_16,
        MODE_16SEG,
        MODE_32,
        MODE_64
    };

    struct _MEMORY_MAP
    {
        qint64 nBaseAddress;
        qint64 nImageSize;
        qint64 nRawSize;
        FT fileType;
        MODE mode;
        bool bIsBigEndian;
        QString sArch;
        QString sType;
        QList<_MEMORY_RECORD> listRecords;
    };

    enum HASH
    {
        HASH_MD4=0,
        HASH_MD5,
        HASH_SHA1,
#ifndef QT_CRYPTOGRAPHICHASH_ONLY_SHA1
        HASH_SHA224,
        HASH_SHA256,
        HASH_SHA384,
        HASH_SHA512,
//        HASH_KECCAK_224,
//        HASH_KECCAK_256,
//        HASH_KECCAK_384,
//        HASH_KECCAK_512
#endif
    };

    enum TYPE
    {
        TYPE_UNKNOWN=0,
    };

private:
    enum ST
    {
        ST_COMPAREBYTES=0,
        ST_RELOFFSETFIX,
        ST_RELOFFSET,
        ST_ADDRESS
    };

    struct SIGNATURE_RECORD
    {
        qint64 nBaseAddress;
        ST st;
        QByteArray baData;
        quint32 nSizeOfAddr;
    };

public:
    explicit XBinary(QIODevice *pDevice=nullptr,bool bIsImage=false,qint64 nImageBase=-1); // mb TODO parent for signals/slot
    void setDevice(QIODevice *pDevice);
    qint64 getSize();
    static qint64 getSize(QIODevice *pDevice);
    static qint64 getSize(QString sFileName);

    void setMode(MODE mode);
    virtual MODE getMode();

    void setType(int nType);
    virtual int getType();
    virtual QString typeIdToString(int nType);
    QString getTypeAsString();

    static QString modeIdToString(MODE mode);
    static QString endiannessToString(bool bIsBigEndian);

    void setArch(QString sArch);
    virtual QString getArch();

    void setEndianness(bool bIsBigEndian);

    static bool isPacked(double dEntropy);

    static quint8 random8();
    static quint16 random16();
    static quint32 random32();
    static quint64 random64();
    static quint64 random(quint64 nLimit);

    static QString fileTypeIdToString(FT fileType);

    static QString convertFileName(QString sFileName);
    static QString convertPathName(QString sPathName);

    OS_ANSISTRING getOsAnsiString(qint64 nOffset,qint64 nSize);

    struct FFOPTIONS
    {
        QList<QString> *pListFileNames;
        bool bSubdirectories;
        bool *pbIsStop;
        qint32 *pnNumberOfFiles;
    };
    static void findFiles(QString sDirectoryName,FFOPTIONS *pFFOption,qint32 nLevel=0);
    static void findFiles(QString sDirectoryName,QList<QString> *pListFileNames);

    static QString regExp(QString sRegExp,QString sString,int nIndex);
    static bool isRegExpPresent(QString sRegExp,QString sString);
    qint64 read_array(qint64 nOffset,char *pBuffer,qint64 nMaxSize);
    QByteArray read_array(qint64 nOffset,qint64 nSize);
    qint64 write_array(qint64 nOffset,char *pBuffer,qint64 nMaxSize);

    static QByteArray read_array(QIODevice *pDevice,qint64 nOffset,qint64 nSize);
    static qint64 read_array(QIODevice *pDevice,qint64 nOffset,char *pBuffer,qint64 nSize);

    quint8 read_uint8(qint64 nOffset);
    qint8 read_int8(qint64 nOffset);
    quint16 read_uint16(qint64 nOffset,bool bIsBigEndian=false);
    qint16 read_int16(qint64 nOffset,bool bIsBigEndian=false);
    quint32 read_uint32(qint64 nOffset,bool bIsBigEndian=false);
    qint32 read_int32(qint64 nOffset,bool bIsBigEndian=false);
    quint64 read_uint64(qint64 nOffset,bool bIsBigEndian=false);
    qint64 read_int64(qint64 nOffset,bool bIsBigEndian=false);
    // TODO double float

    quint32 read_uint24(qint64 nOffset,bool bIsBigEndian=false); // Uses UPX in header

    qint64 write_ansiString(qint64 nOffset,QString sString);

    QString read_ansiString(qint64 nOffset,qint64 nMaxSize=256);
    QString read_unicodeString(qint64 nOffset,qint64 nMaxSize=256,bool bIsBigEndian=false);
    QString read_utf8String(qint64 nOffset,qint64 nMaxSize=256);
    QString _read_utf8String(qint64 nOffset);
    QString _read_utf8String(char *pData,qint32 nDataSize);
    QString _read_utf8String(qint64 nOffset,char *pData,qint32 nDataSize,qint32 nDataOffset);

    void write_uint8(qint64 nOffset,quint8 nValue);
    void write_int8(qint64 nOffset,qint8 nValue);
    void write_uint16(qint64 nOffset,quint16 nValue,bool bIsBigEndian=false);
    void write_int16(qint64 nOffset,qint16 nValue,bool bIsBigEndian=false);
    void write_uint32(qint64 nOffset,quint32 nValue,bool bIsBigEndian=false);
    void write_int32(qint64 nOffset,qint32 nValue,bool bIsBigEndian=false);
    void write_uint64(qint64 nOffset,quint64 nValue,bool bIsBigEndian=false);
    void write_int64(qint64 nOffset,qint64 nValue,bool bIsBigEndian=false);
    // TODO double float

    static quint8 _read_uint8(char *pData);
    static qint8 _read_int8(char *pData);
    static quint16 _read_uint16(char *pData,bool bIsBigEndian=false);
    static qint16 _read_int16(char *pData,bool bIsBigEndian=false);
    static quint32 _read_uint32(char *pData,bool bIsBigEndian=false);
    static qint32 _read_int32(char *pData,bool bIsBigEndian=false);
    static qint64 _read_uint64(char *pData,bool bIsBigEndian=false);
    static qint64 _read_int64(char *pData,bool bIsBigEndian=false);
    static QString _read_ansiString(char *pData,int nMaxSize=50);
    static QByteArray _read_byteArray(char *pData,int nSize);
    // TODO double float

    static void _write_uint8(char *pData,quint8 nValue);
    static void _write_int8(char *pData,qint8 nValue);
    static void _write_uint16(char *pData,quint16 nValue,bool bIsBigEndian=false);
    static void _write_int16(char *pData,qint16 nValue,bool bIsBigEndian=false);
    static void _write_uint32(char *pData,quint32 nValue,bool bIsBigEndian=false);
    static void _write_int32(char *pData,qint32 nValue,bool bIsBigEndian=false);
    static void _write_uint64(char *pData,qint64 nValue,bool bIsBigEndian=false);
    static void _write_int64(char *pData,qint64 nValue,bool bIsBigEndian=false);
    // TODO double float

    qint64 find_array(qint64 nOffset,qint64 nSize,const char *pArray,qint64 nArraySize);
    qint64 find_uint8(qint64 nOffset,qint64 nSize,quint8 nValue);
    qint64 find_int8(qint64 nOffset,qint64 nSize,qint8 nValue);
    qint64 find_uint16(qint64 nOffset,qint64 nSize,quint16 nValue,bool bIsBigEndian=false);
    qint64 find_int16(qint64 nOffset,qint64 nSize,qint16 nValue,bool bIsBigEndian=false);
    qint64 find_uint32(qint64 nOffset,qint64 nSize,quint32 nValue,bool bIsBigEndian=false);
    qint64 find_int32(qint64 nOffset,qint64 nSize,qint32 nValue,bool bIsBigEndian=false);
    qint64 find_uint64(qint64 nOffset,qint64 nSize,quint64 nValue,bool bIsBigEndian=false);
    qint64 find_int64(qint64 nOffset,qint64 nSize,qint64 nValue,bool bIsBigEndian=false);
    qint64 find_float(qint64 nOffset,qint64 nSize,float fValue,bool bIsBigEndian=false);
    qint64 find_double(qint64 nOffset,qint64 nSize,double dValue,bool bIsBigEndian=false);

    static void endian_float(float *pValue,bool bIsBigEndian);
    static void endian_double(double *pValue,bool bIsBigEndian);

    qint64 find_ansiString(qint64 nOffset,qint64 nSize,QString sString);
    qint64 find_unicodeString(qint64 nOffset,qint64 nSize,QString sString); // mb TODO endian
    qint64 find_signature(qint64 nOffset,qint64 nSize,QString sSignature);
    qint64 find_signature(_MEMORY_MAP *pMemoryMap,qint64 nOffset,qint64 nSize,QString sSignature);
    qint64 find_ansiStringI(qint64 nOffset,qint64 nSize,QString sString);
    qint64 find_unicodeStringI(qint64 nOffset,qint64 nSize,QString sString);

    QByteArray getUnicodeString(QString sString);

    void setFindProcessEnable(bool bState);
    void setDumpProcessEnable(bool bState);
    void setEntropyProcessEnable(bool bState);
    void setHashProcessEnable(bool bState);

    bool isSignaturePresent(_MEMORY_MAP *pMemoryMap,qint64 nOffset,qint64 nSize,QString sSignature);

    static bool createFile(QString sFileName,qint64 nFileSize=0);
    static bool isFileExists(QString sFileName);
    static bool removeFile(QString sFileName);
    static bool copyFile(QString sSrcFileName,QString sDestFileName);
    static bool moveFile(QString sSrcFileName,QString sDestFileName);
    static bool moveFileToDirectory(QString sSrcFileName,QString sDestDirectory);
    static QString convertFileNameSymbols(QString sFileName);
    static QString getBaseFileName(QString sFileName);
    static bool createDirectory(QString sDirectoryName);
    static bool isDirectoryExists(QString sDirectoryName);
    static bool removeDirectory(QString sDirectoryName);
    static bool isDirectoryEmpty(QString sDirectoryName);

    static QByteArray readFile(QString sFileName);

    static void _copyMemory(char *pDest,char *pSource,qint64 nSize);
    static void _zeroMemory(char *pDest,qint64 nSize);
    static bool _isMemoryZeroFilled(char *pDest,qint64 nSize);
    static bool copyDeviceMemory(QIODevice *pSourceDevice,qint64 nSourceOffset,QIODevice *pDestDevice,qint64 nDestOffset,qint64 nSize,quint32 nBufferSize=0x1000);
    bool copyMemory(qint64 nSourceOffset,qint64 nDestOffset,qint64 nSize,quint32 nBufferSize=1,bool bReverse=false);
    bool zeroFill(qint64 nOffset,qint64 nSize);
    static bool compareMemory(char *pMemory1,const char *pMemory2,qint64 nSize);
    static bool compareMemoryByteI(quint8 *pMemory,const quint8 *pMemoryU,const quint8 *pMemoryL,qint64 nSize);
    static bool compareMemoryWordI(quint16 *pMemory,const quint16 *pMemoryU,const quint16 *pMemoryL,qint64 nSize);

    bool isOffsetValid(qint64 nOffset);
    bool isAddressValid(qint64 nAddress);
    bool isRelAddressValid(qint64 nRelAddress);
    bool isAddressPhysical(qint64 nAddress);

    qint64 offsetToAddress(qint64 nOffset);
    qint64 addressToOffset(qint64 nAddress);
    qint64 offsetToRelAddress(qint64 nOffset);
    qint64 relAddressToOffset(qint64 nRelAddress);

    static bool isOffsetValid(_MEMORY_MAP *pMemoryMap,qint64 nOffset);
    static bool isOffsetAndSizeValid(_MEMORY_MAP *pMemoryMap,OFFSETSIZE *pOffsetSize);
    bool isOffsetAndSizeValid(qint64 nOffset,qint64 nSize);
    static bool isOffsetAndSizeValid(_MEMORY_MAP *pMemoryMap,qint64 nOffset,qint64 nSize);

    static bool isAddressValid(_MEMORY_MAP *pMemoryMap,qint64 nAddress);
    static bool isRelAddressValid(_MEMORY_MAP *pMemoryMap,qint64 nRelAddress);

    static bool isAddressPhysical(_MEMORY_MAP *pMemoryMap,qint64 nAddress);

    static qint64 offsetToAddress(_MEMORY_MAP *pMemoryMap,qint64 nOffset);
    static qint64 addressToOffset(_MEMORY_MAP *pMemoryMap,qint64 nAddress);
    static qint64 offsetToRelAddress(_MEMORY_MAP *pMemoryMap,qint64 nOffset);
    static qint64 relAddressToOffset(_MEMORY_MAP *pMemoryMap,qint64 nRelAddress);
    static qint64 relAddressToAddress(_MEMORY_MAP *pMemoryMap,qint64 nRelAddress);
    static qint64 addressToRelAddress(_MEMORY_MAP *pMemoryMap,qint64 nAddress);

    static _MEMORY_RECORD getMemoryRecordByOffset(_MEMORY_MAP *pMemoryMap,qint64 nOffset);
    static _MEMORY_RECORD getMemoryRecordByAddress(_MEMORY_MAP *pMemoryMap,qint64 nAddress);
    static _MEMORY_RECORD getMemoryRecordByRelAddress(_MEMORY_MAP *pMemoryMap,qint64 nRelAddress);

    static qint32 addressToLoadSection(_MEMORY_MAP *pMemoryMap,qint64 nAddress);

    static bool isSolidAddressRange(_MEMORY_MAP *pMemoryMap,qint64 nAddress,qint64 nSize);

    QString getMemoryRecordInfoByOffset(qint64 nOffset);
    QString getMemoryRecordInfoByAddress(qint64 nAddress);
    QString getMemoryRecordInfoByRelAddress(qint64 nRelAddress);

    static QString getMemoryRecordInfoByOffset(_MEMORY_MAP *pMemoryMap,qint64 nOffset);
    static QString getMemoryRecordInfoByAddress(_MEMORY_MAP *pMemoryMap,qint64 nAddress);
    static QString getMemoryRecordInfoByRelAddress(_MEMORY_MAP *pMemoryMap,qint64 nRelAddress);

    static QString getMemoryRecordInfo(_MEMORY_RECORD *pMemoryRecord);

    virtual _MEMORY_MAP getMemoryMap();

    static qint32 getNumberOfPhysicalRecords(_MEMORY_MAP *pMemoryMap);

    virtual qint64 getBaseAddress();
    virtual void setBaseAddress(qint64 nBaseAddress);

    virtual qint64 getImageSize();

    qint64 _getEntryPointOffset();
    virtual qint64 getEntryPointOffset(_MEMORY_MAP *pMemoryMap);

    virtual void setEntryPointOffset(qint64 nEntryPointOffset);
    qint64 getEntryPointAddress();
    qint64 getEntryPointAddress(_MEMORY_MAP *pMemoryMap);

    static qint64 getLowestAddress(_MEMORY_MAP *pMemoryMap);
    static qint64 getTotalVirtualSize(_MEMORY_MAP *pMemoryMap);
    static qint64 positionToVirtualAddress(_MEMORY_MAP *pMemoryMap,qint64 nPosition);

    qint64 getImageAddress();
    void setImageBase(qint64 nValue);

    qint64 _getBaseAddress();

    bool isImage();
    void setIsImage(bool bValue);

    static qint64 getPhysSize(char *pBuffer,qint64 nSize); // TODO Check!
    static bool isEmptyData(char *pBuffer,qint64 nSize);
    bool compareSignature(QString sSignature,qint64 nOffset=0);
    bool compareSignature(_MEMORY_MAP *pMemoryMap,QString sSignature,qint64 nOffset=0);
    static bool _compareByteArrayWithSignature(QByteArray baData,QString sSignature);
    static QString _createSignature(QString sSignature1,QString sSignature2);

    bool compareSignatureOnAddress(QString sSignature,qint64 nAddress);
    bool compareSignatureOnAddress(_MEMORY_MAP *pMemoryMap,QString sSignature,qint64 nAddress);

    bool compareEntryPoint(QString sSignature,qint64 nOffset=0);
    bool compareEntryPoint(_MEMORY_MAP *pMemoryMap,QString sSignature,qint64 nOffset=0);

    bool moveMemory(qint64 nSourceOffset,qint64 nDestOffset,qint64 nSize);

    static bool dumpToFile(QString sFileName,const char *pData,qint64 nDataSize);
    bool dumpToFile(QString sFileName,qint64 nDataOffset,qint64 nDataSize);

    QSet<FT> getFileTypes(bool bExtra=false);
    static QSet<FT> getFileTypes(QIODevice *pDevice,bool bExtra=false); // mb TODO isImage
    static QSet<FT> getFileTypes(QString sFileName,bool bExtra=false);
    static QSet<FT> getFileTypes(QByteArray *pbaData,bool bExtra=false);

    static FT getPrefFileType(QIODevice *pDevice);

    static QList<FT> _getFileTypeListFromSet(QSet<FT> stFileTypes);

    static QString valueToHex(quint8 nValue);
    static QString valueToHex(qint8 nValue);
    static QString valueToHex(quint16 nValue,bool bIsBigEndian=false);
    static QString valueToHex(qint16 nValue,bool bIsBigEndian=false);
    static QString valueToHex(quint32 nValue,bool bIsBigEndian=false);
    static QString valueToHex(qint32 nValue,bool bIsBigEndian=false);
    static QString valueToHex(quint64 nValue,bool bIsBigEndian=false);
    static QString valueToHex(qint64 nValue,bool bIsBigEndian=false);
    static QString valueToHex(float fValue,bool bIsBigEndian=false);
    static QString valueToHex(double dValue,bool bIsBigEndian=false);
    static QString valueToHex(MODE mode,quint64 nValue,bool bIsBigEndian=false);

    static bool checkString_uint8(QString sValue);
    static bool checkString_int8(QString sValue);
    static bool checkString_uint16(QString sValue);
    static bool checkString_int16(QString sValue);
    static bool checkString_uint32(QString sValue);
    static bool checkString_int32(QString sValue);
    static bool checkString_uint64(QString sValue);
    static bool checkString_int64(QString sValue);
    static bool checkString_float(QString sValue);
    static bool checkString_double(QString sValue);

    static QString getUnpackedName(QIODevice *pDevice);
    static QString getUnpackedName(QString sFileName);
    static QString getBackupName(QIODevice *pDevice);
    static QString getBackupName(QString sFileName);
    static QString getDeviceFileName(QIODevice *pDevice);
    static QString getDeviceFilePath(QIODevice *pDevice);
    static QString getDeviceDirectory(QIODevice *pDevice);
    static QString getDeviceFileBaseName(QIODevice *pDevice);
    static QString getDeviceFileCompleteSuffix(QIODevice *pDevice);
    static QString getDeviceFileSuffix(QIODevice *pDevice);

    static QList<qint64> getFixupList(QIODevice *pDevice1,QIODevice *pDevice2,qint64 nDelta);

    static QString getHash(HASH hash,QString sFileName);
    static QString getHash(HASH hash,QIODevice *pDevice);
    QString getHash(HASH hash,qint64 nOffset=0,qint64 nSize=-1);

    static QSet<HASH> getHashMethods();
    static QList<HASH> getHashMethodsAsList();
    static QString hashIdToString(HASH hash);

    static bool isFileHashValid(HASH hash,QString sFileName,QString sHash);

    static quint32 getAdler32(QString sFileName);
    static quint32 getAdler32(QIODevice *pDevice);
    quint32 getAdler32(qint64 nOffset=0,qint64 nSize=-1);

    static quint32 _getCRC32(QString sFileName);
    static quint32 _getCRC32(QIODevice *pDevice);
    quint32 _getCRC32(qint64 nOffset=0,qint64 nSize=-1);

    double getEntropy(qint64 nOffset=0,qint64 nSize=-1);

    BYTE_COUNTS getByteCounts(qint64 nOffset=0,qint64 nSize=-1);

    void _xor(quint8 nXorValue,qint64 nOffset=0,qint64 nSize=-1);

//    static quint32 _ror32(quint32 nValue,quint32 nShift);
//    static quint32 _rol32(quint32 nValue,quint32 nShift);
    static quint32 getStringCustomCRC32(QString sString);

    QIODevice *getDevice();

    virtual bool isValid();
    virtual bool isBigEndian();
    bool is16();
    bool is32();
    bool is64();

    bool isBigEndian(_MEMORY_MAP *pMemoryMap);
    bool is16(_MEMORY_MAP *pMemoryMap);
    bool is32(_MEMORY_MAP *pMemoryMap);
    bool is64(_MEMORY_MAP *pMemoryMap);

    void setVersion(QString sVersion);
    virtual QString getVersion();
    virtual bool isEncrypted();

    QString getSignature(qint64 nOffset,qint64 nSize);

    OFFSETSIZE convertOffsetAndSize(qint64 nOffset,qint64 nSize);

    static bool compareSignatureStrings(QString sBaseSignature,QString sOptSignature);
    static QString stringToHex(QString sString);
    static QString hexToString(QString sHex);
    static QString floatToString(float fValue,int nPrec=2);
    static QString doubleToString(double dValue,int nPrec=2);

    static quint8 hexToUint8(QString sHex);
    static qint8 hexToInt8(QString sHex);
    static quint16 hexToUint16(QString sHex,bool bIsBigEndian=false);
    static qint16 hexToInt16(QString sHex,bool bIsBigEndian=false);
    static quint32 hexToUint32(QString sHex,bool bIsBigEndian=false);
    static qint32 hexToInt32(QString sHex,bool bIsBigEndian=false);
    static quint64 hexToUint64(QString sHex,bool bIsBigEndian=false);
    static qint64 hexToInt64(QString sHex,bool bIsBigEndian=false);

    static QString invertHexByteString(QString sHex);

    static void _swapBytes(char *pSource,int nSize);
    static quint16 swapBytes(quint16 nValue);
    static quint32 swapBytes(quint32 nValue);
    static quint64 swapBytes(quint64 nValue);

    bool isPlainTextType();
    bool isPlainTextType(QByteArray *pbaData);
    bool isUTF8TextType();
    bool isUTF8TextType(QByteArray *pbaData);

    static bool isPlainTextType(QIODevice *pDevice);

    enum UNICODE_TYPE
    {
        UNICODE_TYPE_NONE=0,
        UNICODE_TYPE_LE,
        UNICODE_TYPE_BE
    };

    UNICODE_TYPE getUnicodeType();
    UNICODE_TYPE getUnicodeType(QByteArray *pbaData);

    static bool tryToOpen(QIODevice *pDevice);

    bool checkOffsetSize(OFFSETSIZE os);

    static QString get_uint32_version(quint32 nValue);
    static bool isResizeEnable(QIODevice *pDevice);
    static bool resize(QIODevice *pDevice,qint64 nSize);

    struct ULEB128
    {
        quint64 nValue;
        quint32 nByteSize;
    };

    ULEB128 get_uleb128(qint64 nOffset);
    ULEB128 _get_uleb128(char *pData);

    struct PACKED
    {
        quint64 nValue;
        quint32 nByteSize;
    };

    PACKED get_packedNumber(qint64 nOffset);

    static QList<QString> getListFromFile(QString sFileName);

    qint64 getOverlaySize();
    qint64 getOverlaySize(_MEMORY_MAP *pMemoryMap);
    qint64 getOverlayOffset();
    qint64 getOverlayOffset(_MEMORY_MAP *pMemoryMap);
    bool isOverlayPresent();
    bool isOverlayPresent(_MEMORY_MAP *pMemoryMap);

    bool compareOverlay(QString sSignature,qint64 nOffset);
    bool compareOverlay(_MEMORY_MAP *pMemoryMap,QString sSignature,qint64 nOffset);

    bool addOverlay(char *pData,qint64 nDataSize);
    bool removeOverlay();

    bool isSignatureInLoadSectionPresent(qint32 nLoadSection,QString sSignature);
    bool isSignatureInLoadSectionPresent(_MEMORY_MAP *pMemoryMap,qint32 nLoadSection,QString sSignature);

    static QString getStringCollision(QList<QString> *pListStrings,QString sString1,QString sString2);

    static bool writeToFile(QString sFileName,QByteArray baData);

    static qint32 getStringNumberFromList(QList<QString> *pListStrings,QString sString,bool *pbIsStop=nullptr);
    static qint32 getStringNumberFromListExp(QList<QString> *pListStrings,QString sString,bool *pbIsStop=nullptr);
    static bool isStringInListPresent(QList<QString> *pListStrings,QString sString,bool *pbIsStop=nullptr);
    static bool isStringInListPresentExp(QList<QString> *pListStrings,QString sString,bool *pbIsStop=nullptr);
    static QString getStringByIndex(QList<QString> *pListStrings,int nIndex,qint32 nNumberOfStrings=-1);

    static bool isStringUnicode(QString sString,qint32 nMaxCheckSize=-1);

    static quint32 elfHash(const quint8 *pData);

    static QString getVersionString(QString sString);
    static qint64 getVersionIntValue(QString sString);
    static bool checkStringNumber(QString sString,quint32 nMin,quint32 nMax);

    enum DT_TYPE
    {
        DT_TYPE_UNKNOWN=0,
        DT_TYPE_POSIX
    };

    static QString valueToTimeString(quint64 nValue,DT_TYPE type);

    static bool isX86asm(QString sArch);
    static bool checkFileType(XBinary::FT fileTypeMain,XBinary::FT fileTypeOptional);

    static void filterFileTypes(QSet<XBinary::FT> *pStFileTypes);
    static void filterFileTypes(QSet<XBinary::FT> *pStFileTypes,XBinary::FT fileType);

    struct PROCENT
    {
        qint64 nCurrentValue;
        qint64 nMaxValue;
        qint32 nCurrentProcent;
        qint32 nMaxProcent;
    };

    static PROCENT procentInit(qint64 nMaxValue);
    static bool procentSetCurrentValue(PROCENT *pProcent,qint64 nCurrentValue);

private:
    static QString convertSignature(QString sSignature);
    static QString qcharToHex(QChar cSymbol);

    QList<SIGNATURE_RECORD> getSignatureRecords(QString sSignature);
    bool _compareSignature(_MEMORY_MAP *pMemoryMap,QList<SIGNATURE_RECORD> *pListSignatureRecords,qint64 nOffset);

    int _getSignatureRelOffsetFix(QList<SIGNATURE_RECORD> *pListSignatureRecords,QString sSignature,int nStartIndex);
    int _getSignatureRelOffset(QList<SIGNATURE_RECORD> *pListSignatureRecords,QString sSignature,int nStartIndex);
    int _getSignatureAddress(QList<SIGNATURE_RECORD> *pListSignatureRecords,QString sSignature,int nStartIndex);
    int _getSignatureBytes(QList<SIGNATURE_RECORD> *pListSignatureRecords,QString sSignature,int nStartIndex);

protected:
    bool _isOffsetValid(qint64 nOffset);
    void _errorMessage(QString sErrorMessage);
    void _infoMessage(QString sInfoMessage);
    qint64 _calculateRawSize();
    qint64 _calculateRawSize(_MEMORY_MAP *pMemoryMap);

signals:
    void errorMessage(QString sErrorMessage);
    void infoMessage(QString sInfoMessage);
    void findProgressMinimumChanged(qint32 nMaximum);
    void findProgressMaximumChanged(qint32 nMaximum);
    void findProgressValueChanged(qint32 nValue);
    void dumpProgressMinimumChanged(qint32 nMaximum);
    void dumpProgressMaximumChanged(qint32 nMaximum);
    void dumpProgressValueChanged(qint32 nValue);
    void entropyProgressMinimumChanged(qint32 nMaximum);
    void entropyProgressMaximumChanged(qint32 nMaximum);
    void entropyProgressValueChanged(qint32 nValue);
    void hashProgressMinimumChanged(qint32 nMaximum);
    void hashProgressMaximumChanged(qint32 nMaximum);
    void hashProgressValueChanged(qint32 nValue);

private:
    QIODevice *g_pDevice;
    bool g_bIsImage;
    qint64 g_nBaseAddress;
    qint64 g_nEntryPointOffset;
    qint64 g_nImageBase;
    bool g_bIsBigEndian;
    bool g_bIsFindStop;
    bool g_bIsDumpStop;
    bool g_bIsEntropyStop;
    bool g_bIsHashStop;
    QString g_sArch;
    MODE g_mode;
    QString g_sVersion;
    int g_nType;
};

#endif // XBINARY_H
