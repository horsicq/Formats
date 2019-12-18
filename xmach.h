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
#ifndef XMACH_H
#define XMACH_H

#include "xbinary.h"
#include "xmach_def.h"

class XMACH : public XBinary
{
    Q_OBJECT

public:
    struct COMMAND_RECORD
    {
        quint32 nType;
        qint64 nOffset;
        qint64 nSize;
    };

    struct LIBRARY_RECORD
    {
        QString sName;
        QString sFullName;
        quint32 timestamp;
        quint32 current_version;
        quint32 compatibility_version;
    };

    struct SEGMENT_RECORD
    {
        qint64 nStructOffset;
        char segname[16];
        quint64 vmaddr;
        quint64 vmsize;
        quint64 fileoff;
        quint64 filesize;
        qint32 maxprot;
        qint32 initprot;
        quint32 nsects;
        quint32 flags;
    };

    struct SECTION_RECORD
    {
        qint64 nStructOffset;
        char sectname[16];
        char segname[16];
        quint64	addr;
        quint64	size;
        quint32	offset;
        quint32	align;
        quint32	reloff;
        quint32	nreloc;
        quint32	flags;
    };

    XMACH(QIODevice *__pDevice=nullptr,bool bIsImage=false,qint64 nImageAddress=-1);
    ~XMACH();

    bool isValid();
    bool isBigEndian();
    bool is64();
    static bool is64(QIODevice *pDevice);

    quint32 getHeader_magic();
    qint32 getHeader_cputype();
    qint32 getHeader_cpusubtype();
    quint32 getHeader_filetype();
    quint32 getHeader_ncmds();
    quint32 getHeader_sizeofcmds();
    quint32 getHeader_flags();
    quint32 getHeader_reserved();

    void setHeader_magic(quint32 nValue);
    void setHeader_cputype(qint32 nValue);
    void setHeader_cpusubtype(qint32 nValue);
    void setHeader_filetype(quint32 nValue);
    void setHeader_ncmds(quint32 nValue);
    void setHeader_sizeofcmds(quint32 nValue);
    void setHeader_flags(quint32 nValue);
    void setHeader_reserved(quint32 nValue);

    qint64 getHeaderSize();

    static QMap<quint64,QString> getHeaderMagics();
    static QMap<quint64,QString> getHeaderMagicsS();
    static QMap<quint64,QString> getHeaderCpuTypes();
    static QMap<quint64,QString> getHeaderCpuTypesS();
    static QMap<quint64,QString> getHeaderFileTypes();
    static QMap<quint64,QString> getHeaderFileTypesS();
    static QMap<quint64,QString> getHeaderFlags();
    static QMap<quint64,QString> getHeaderFlagsS();
    static QMap<quint64,QString> getLoadCommandTypes();
    static QMap<quint64,QString> getLoadCommandTypesS();

    QList<COMMAND_RECORD> getCommandRecords();
    static QList<COMMAND_RECORD> getCommandRecords(quint32 nCommandID,QList<COMMAND_RECORD> *pList);

    bool isCommandPresent(quint32 nCommandID,int nIndex);
    bool isCommandPresent(quint32 nCommandID,int nIndex,QList<COMMAND_RECORD> *pList);

    QByteArray getCommand(quint32 nCommandID,int nIndex=0);
    bool setCommand(quint32 nCommandID,QByteArray baData,int nIndex=0);
    QByteArray getCommand(quint32 nCommandID,int nIndex,QList<COMMAND_RECORD> *pList);
    bool setCommand(quint32 nCommandID,QByteArray baData,int nIndex,QList<COMMAND_RECORD> *pList);

    qint64 getAddressOfEntryPoint();

    virtual _MEMORY_MAP getMemoryMap();
    virtual qint64 getEntryPointOffset();

    QList<LIBRARY_RECORD> getLibraryRecords();
    QList<LIBRARY_RECORD> getLibraryRecords(QList<COMMAND_RECORD> *pList);
    static LIBRARY_RECORD getLibraryRecordByName(QString sName,QList<LIBRARY_RECORD> *pList);
    bool isLibraryRecordNamePresent(QString sName);
    static bool isLibraryRecordNamePresent(QString sName,QList<LIBRARY_RECORD> *pList);

    QList<SEGMENT_RECORD> getSegmentRecords();
    QList<SEGMENT_RECORD> getSegmentRecords(QList<COMMAND_RECORD> *pList);

    QList<SECTION_RECORD> getSectionRecords();
    QList<SECTION_RECORD> getSectionRecords(QList<COMMAND_RECORD> *pList);

    quint32 getNumberOfSections();
    quint32 getNumberOfSections(QList<COMMAND_RECORD> *pList);

    bool isSectionNamePresent(QString sName);
    static bool isSectionNamePresent(QString sName,QList<SECTION_RECORD> *pList);

    static qint32 getSectionNumber(QString sName,QList<SECTION_RECORD> *pList);
    qint32 getSectionNumber(QString sName);
};

#endif // XMACH_H
