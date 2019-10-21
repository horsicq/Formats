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
#if (QT_VERSION_MAJOR<5)
#include <QRegExp>
#else
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#endif
#include <math.h>
#include <QDebug>
#include <QElapsedTimer>
#if (QT_VERSION_MAJOR>=5)&&(QT_VERSION_MINOR>=10)
#include <QRandomGenerator>
#endif
#include <QDateTime>

#include "xmsdos_def.h"
#include "xpe_def.h"
#include "xelf_def.h"
#include "xmach_def.h"

#include "subdevice.h"

#define S_ALIGN_DOWN(x,align)     ((x)&~(align-1))
#define S_ALIGN_UP(x,align)       (((x)&(align-1))?S_ALIGN_DOWN(x,align)+align:x)

#define S_LOWORD(l)               ((quint16)((quint32)(l)&0xffff))
#define S_HIWORD(l)               ((quint16)((quint32)(l)>>16))

#ifdef Q_OS_MAC
#include <CoreFoundation/CoreFoundation.h> // Check
#endif

class XBinary : public QObject
{
    Q_OBJECT

public:
    struct OFFSETSIZE
    {
        qint64 nOffset;
        qint64 nSize;
    };

    enum ADDRESS_SEGMENT
    {
        ADDRESS_SEGMENT_UNKNOWN=-1,
        ADDRESS_SEGMENT_FLAT=0,
        ADDRESS_SEGMENT_CODE,
        ADDRESS_SEGMENT_DATA
    };

    struct MEMORY_MAP
    {
        qint64 nOffset;
        qint64 nAddress;
        ADDRESS_SEGMENT segment;
        qint64 nSize;
        bool bIsHeader;
        bool bIsLoadSection; // Section in PE; LoadProgram in ELF
        bool bIsOvelay;
        qint32 nLoadSection;
    };

    enum FT
    {
        FT_UNKNOWN=0,
        FT_BINARY,
        FT_MSDOS,
        FT_PE,
        FT_PE32,
        FT_PE64,
        FT_ELF,
        FT_ELF32,
        FT_ELF64,
        FT_MACH,
        FT_MACH32,
        FT_MACH64
    };

    enum MODE
    {
        MODE_UNKNOWN=0,
        MODE_16,
        MODE_32,
        MODE_64
    };

    enum ARCH
    {
        ARCH_UNKNOWN=0,
        ARCH_X86
    };

    struct UNPACK_OPTIONS
    {
        // PE/PE+
        bool bCopyOverlay;     // In
    };

private:
    struct SIGNATURE_RECORD
    {
        quint8 nType;
        QByteArray baData;
        quint8 nSizeOfAddr;
        qint64 nBaseAddress;
    };

    enum SIGNATURE_TYPES
    {
        CompareBytes,
        RelOffsetFix,
        RelOffset,
        Address
    };

public:
    explicit XBinary(QIODevice *__pDevice=nullptr,bool bIsImage=false,qint64 nImageBase=-1); // mb TODO parent for signals/slot
    void setData(QIODevice *__pDevice);
    qint64 getSize();

    virtual MODE getMode();
    virtual ARCH getArch();

    static quint32 random32();
    static quint64 random64();

    static QString convertFileName(QString sFileName);
//    static void findFiles(QString sDirectoryName,QList<QString> *pListFileNames); // TODO Check

    struct FFOPTIONS
    {
        QList<QString> *pListFiles;
        bool bSubdirectories;
        bool *pbIsStop;
        qint32 *pnNumberOfFiles;
    };
    static void findFiles(QString sDirectoryName,FFOPTIONS *pFFOption,qint32 nLevel=0);

    static QString regExp(QString sRegExp,QString sString,int nIndex);
    static bool isRegExpPresent(QString sRegExp,QString sString);
    qint64 read_array(qint64 nOffset,char *pBuffer,qint64 nMaxSize);
    QByteArray read_array(qint64 nOffset,qint64 nSize);
    qint64 write_array(qint64 nOffset,char *pBuffer,qint64 nMaxSize);

    quint8 read_uint8(qint64 nOffset);
    qint8 read_int8(qint64 nOffset);
    quint16 read_uint16(qint64 nOffset,bool bIsBigEndian=false);
    qint16 read_int16(qint64 nOffset,bool bIsBigEndian=false);
    quint32 read_uint32(qint64 nOffset,bool bIsBigEndian=false);
    qint32 read_int32(qint64 nOffset,bool bIsBigEndian=false);
    quint64 read_uint64(qint64 nOffset,bool bIsBigEndian=false);
    qint64 read_int64(qint64 nOffset,bool bIsBigEndian=false);

    qint64 write_ansiString(qint64 nOffset,QString sString);

    QString read_ansiString(qint64 nOffset,qint64 nMaxSize=256);
    QString read_unicodeString(qint64 nOffset,qint64 nMaxSize=256,bool bIsBigEndian=false);
    QString read_utf8String(qint64 nOffset,qint64 nMaxSize=256);

    void write_uint8(qint64 nOffset,quint8 value);
    void write_int8(qint64 nOffset,qint8 value);
    void write_uint16(qint64 nOffset,quint16 value,bool bIsBigEndian=false);
    void write_int16(qint64 nOffset,qint16 value,bool bIsBigEndian=false);
    void write_uint32(qint64 nOffset,quint32 value,bool bIsBigEndian=false);
    void write_int32(qint64 nOffset,qint32 value,bool bIsBigEndian=false);
    void write_uint64(qint64 nOffset,quint64 value,bool bIsBigEndian=false);
    void write_int64(qint64 nOffset,qint64 value,bool bIsBigEndian=false);

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

    static void _write_uint8(char *pData,quint8 value);
    static void _write_int8(char *pData,qint8 value);
    static void _write_uint16(char *pData,quint16 value,bool bIsBigEndian=false);
    static void _write_int16(char *pData,qint16 value,bool bIsBigEndian=false);
    static void _write_uint32(char *pData,quint32 value,bool bIsBigEndian=false);
    static void _write_int32(char *pData,qint32 value,bool bIsBigEndian=false);
    static void _write_uint64(char *pData,qint64 value,bool bIsBigEndian=false);
    static void _write_int64(char *pData,qint64 value,bool bIsBigEndian=false);

    qint64 find_array(qint64 nOffset,qint64 nSize,const char *pArray,qint64 nArraySize);
    qint64 find_uint8(qint64 nOffset,qint64 nSize,quint8 value);
    qint64 find_int8(qint64 nOffset,qint64 nSize,qint8 value);
    qint64 find_uint16(qint64 nOffset,qint64 nSize,quint16 value,bool bIsBigEndian=false);
    qint64 find_int16(qint64 nOffset,qint64 nSize,qint16 value,bool bIsBigEndian=false);
    qint64 find_uint32(qint64 nOffset,qint64 nSize,quint32 value,bool bIsBigEndian=false);
    qint64 find_int32(qint64 nOffset,qint64 nSize,qint32 value,bool bIsBigEndian=false);
    qint64 find_uint64(qint64 nOffset,qint64 nSize,qint64 value,bool bIsBigEndian=false);
    qint64 find_int64(qint64 nOffset,qint64 nSize,qint64 value,bool bIsBigEndian=false);
    qint64 find_ansiString(qint64 nOffset,qint64 nSize,QString sString);
    qint64 find_unicodeString(qint64 nOffset,qint64 nSize,QString sString); // mb TODO endian
    qint64 find_signature(qint64 nOffset,qint64 nSize,QString sSignature);

    static bool createFile(QString sFileName,qint64 nFileSize=0);
    static bool isFileExists(QString sFileName);
    static bool removeFile(QString sFileName);
    static bool copyFile(QString sSrcFileName,QString sDestFileName);
    static bool moveFile(QString sSrcFileName,QString sDestFileName);
    static bool moveFileToDirectory(QString sSrcFileName,QString sDestDirectory);
    static QString convertFileNameSymbols(QString sFileName);
    static QString getBaseFileName(QString sFileName);
    static bool createDirectory(QString sDirectoryName);

    static QByteArray readFile(QString sFileName);

    static void _copyMemory(char *pDest,char *pSource, qint64 nSize);
    static bool copyDeviceMemory(QIODevice *pSourceDevice,qint64 nSourceOffset,QIODevice *pDestDevice,qint64 nDestOffset,qint64 nSize,quint32 nBufferSize=0x1000);
    bool copyMemory(qint64 nSourceOffset, qint64 nDestOffset,qint64 nSize,quint32 nBufferSize=1,bool bReverse=false);
    bool zeroFill(qint64 nOffset,qint64 nSize);
    static bool compareMemory(char *pMemory1,const char *pMemory2,qint64 nSize);

    bool isOffsetValid(qint64 nOffset);
    bool isAddressValid(qint64 nAddress,ADDRESS_SEGMENT segment=ADDRESS_SEGMENT_FLAT);

    qint64 offsetToAddress(qint64 nOffset,ADDRESS_SEGMENT segment=ADDRESS_SEGMENT_FLAT);
    qint64 addressToOffset(qint64 nAddress,ADDRESS_SEGMENT segment=ADDRESS_SEGMENT_FLAT);

    static bool isOffsetValid(QList<MEMORY_MAP> *pMemoryMap,qint64 nOffset);
    static bool isAddressValid(QList<MEMORY_MAP> *pMemoryMap,qint64 nAddress,ADDRESS_SEGMENT segment=ADDRESS_SEGMENT_FLAT);

    static qint64 offsetToAddress(QList<MEMORY_MAP> *pMemoryMap,qint64 nOffset,ADDRESS_SEGMENT segment=ADDRESS_SEGMENT_FLAT);
    static qint64 addressToOffset(QList<MEMORY_MAP> *pMemoryMap,qint64 nAddress,ADDRESS_SEGMENT segment=ADDRESS_SEGMENT_FLAT);

    static MEMORY_MAP getOffsetMemoryMap(QList<MEMORY_MAP> *pMemoryMap,qint64 nOffset);
    static MEMORY_MAP getAddressMemoryMap(QList<MEMORY_MAP> *pMemoryMap,qint64 nAddress,ADDRESS_SEGMENT segment=ADDRESS_SEGMENT_FLAT);

    virtual QList<MEMORY_MAP> getMemoryMapList();
    virtual qint64 getBaseAddress();
    virtual void setBaseAddress(qint64 nBaseAddress);
    virtual qint64 getEntryPointOffset();
    virtual void setEntryPointOffset(qint64 nEntryPointOffset);
    qint64 getEntryPointAddress();

    static qint64 getLowestAddress(QList<MEMORY_MAP> *pMemoryMap);
    static qint64 getTotalVirtualSize(QList<MEMORY_MAP> *pMemoryMap);
    static qint64 positionToVirtualAddress(QList<MEMORY_MAP> *pMemoryMap,qint64 nPosition);

    qint64 getImageAddress();
    void setImageBase(qint64 nValue);

    qint64 _getBaseAddress();

    bool isImage();
    void setIsImage(bool value);

    static qint64 getPhysSize(char *pBuffer,qint64 nSize); // TODO
    static bool isEmptyData(char *pBuffer,qint64 nSize);
    bool compareSignature(QString sSignature,qint64 nOffset=0);
    static bool _compareByteArrayWithSignature(QByteArray baData,QString sSignature);
    static QString _createSignature(QString sSignature1,QString sSignature2);
    bool compareSignatureOnAddress(QString sSignature,qint64 nAddress,ADDRESS_SEGMENT segment=ADDRESS_SEGMENT_FLAT);
    bool compareEntryPoint(QString sSignature,qint64 nOffset=0);
    bool moveMemory(qint64 nSourceOffset,qint64 nDestOffset,qint64 nSize);

    static bool dumpToFile(QString sFileName,const char *pData,qint64 nDataSize);
    bool dumpToFile(QString sFileName,qint64 nDataOffset,qint64 nDataSize);

    QSet<FT> getFileTypes();
    static QSet<FT> getFileTypes(QIODevice *pDevice);
    static QSet<FT> getFileTypes(QString sFileName);

    static QString valueToHex(quint8 value);
    static QString valueToHex(qint8 value);
    static QString valueToHex(quint16 value);
    static QString valueToHex(qint16 value);
    static QString valueToHex(quint32 value);
    static QString valueToHex(qint32 value);
    static QString valueToHex(quint64 value);
    static QString valueToHex(qint64 value);

    static QString getUnpackedName(QIODevice *pDevice);
    static QString getUnpackedName(QString sFileName);
    static QString getBackupName(QIODevice *pDevice);
    static QString getBackupName(QString sFileName);
    static QString getDeviceFileName(QIODevice *pDevice);
    static QString getDeviceFilePath(QIODevice *pDevice);
    static QList<qint64> getFixupList(QIODevice *pDevice1,QIODevice *pDevice2,qint64 nDelta);

    QString getMD5(qint64 nOffset=0,qint64 nSize=-1);
    static QString getMD5(QString sFileName); // TODO one function
    QString getSHA1(qint64 nOffset=0,qint64 nSize=-1);
    quint32 getAdler32(qint64 nOffset=0,qint64 nSize=-1);

    double getEntropy(qint64 nOffset=0,qint64 nSize=-1);

    void _xor(quint8 nXorValue,qint64 nOffset=0,qint64 nSize=-1);

//    static quint32 _ror32(quint32 nValue,quint32 nShift);
//    static quint32 _rol32(quint32 nValue,quint32 nShift);
    static quint32 getCRC32(QString sString);

    QIODevice *getDevice();
    virtual bool isValid();

    QString getSignature(qint64 nOffset,qint64 nSize);

    OFFSETSIZE convertOffsetAndSize(qint64 nOffset,qint64 nSize);

    static bool compareSignatureStrings(QString sBaseSignature, QString sOptSignature);
    static QString stringToHex(QString sString);
    static QString hexToString(QString sHex);

    static quint8 hexToUint8(QString sHex);
    static qint8 hexToInt8(QString sHex);
    static quint16 hexToUint16(QString sHex,bool bIsBigEndian=false);
    static qint16 hexToInt16(QString sHex,bool bIsBigEndian=false);
    static quint32 hexToUint32(QString sHex,bool bIsBigEndian=false);
    static qint32 hexToInt32(QString sHex,bool bIsBigEndian=false);
    static quint64 hexToUint64(QString sHex,bool bIsBigEndian=false);
    static qint64 hexToInt64(QString sHex,bool bIsBigEndian=false);

    static QString invertHexByteString(QString sHex);

    bool isPlainTextType();
    bool isUTF8TextType();

    enum UNICODE_TYPE
    {
        UNICODE_TYPE_NONE=0,
        UNICODE_TYPE_LE,
        UNICODE_TYPE_BE
    };

    UNICODE_TYPE getUnicodeType();

    static bool tryToOpen(QIODevice *pDevice);

    static bool checkOffsetSize(OFFSETSIZE os);

    static QString get_uint32_version(quint32 nValue);
    static bool isResizeEnable(QIODevice *pDevice);
    static bool resize(QIODevice *pDevice,qint64 nSize);

    struct ULEB128
    {
        quint64 nValue;
        quint32 nByteSize;
    };

    ULEB128 get_uleb128(qint64 nOffset);

    static QList<QString> getListFromFile(QString sFileName);

private:
    static QString convertSignature(QString sSignature);
    static QString qcharToHex(QChar c);

    QList<SIGNATURE_RECORD> getSignatureRecords(QString sSignature);
    bool _compareSignature(QList<SIGNATURE_RECORD> *pListSignatures,qint64 nOffset);

    int _getSignatureRelOffsetFix(QList<SIGNATURE_RECORD> *pListSignatures,QString sSignature,int nStartIndex);
    int _getSignatureRelOffset(QList<SIGNATURE_RECORD> *pListSignatures,QString sSignature,int nStartIndex);
    int _getSignatureAddress(QList<SIGNATURE_RECORD> *pListSignatures,QString sSignature,int nStartIndex);
    int _getSignatureBytes(QList<SIGNATURE_RECORD> *pListSignatures,QString sSignature,int nStartIndex);

protected:
    bool _isOffsetValid(qint64 nOffset);
    void _errorMessage(QString sMessage);
    void _infoMessage(QString sMessage);

signals:
    void errorMessage(QString sMessage);
    void infoMessage(QString sMessage);

private:
    QIODevice *__pDevice;
    bool bIsImage;
    qint64 __nBaseAddress;
    qint64 __nEntryPointOffset;
    qint64 __nImageBase;
};

#endif // XBINARY_H
