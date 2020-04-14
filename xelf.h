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
#ifndef XELF_H
#define XELF_H

#include "xbinary.h"
#include "xelf_def.h"

// TODO buffer big Endian();
class XELF : public XBinary
{
    Q_OBJECT

public:
    struct NOTE
    {
        qint64 nOffset;
        qint64 nSize;
        quint32 nType;
        QString name;
        qint64 nDataOffset;
    };

    struct TAG_STRUCT
    {
        qint64 nOffset;
        qint64 nTag;
        qint64 nValue;
    };

    struct SECTION_RECORD
    {
        QString sName;
        qint64 nOffset;
        qint64 nSize;
        qint64 nFlags;
    };

    XELF(QIODevice *__pDevice=nullptr,bool bIsImage=false,qint64 nImageBase=-1);
    ~XELF();

    virtual bool isValid();
    bool isBigEndian();
    bool is64();

    static bool is64(QIODevice *pDevice);

    qint64 getEhdrOffset();
    qint64 getEhdr32Size();
    qint64 getEhdr64Size();

    quint32 getIdent_Magic();
    void setIdent_Magic(quint32 value);

    quint8 getIdent_mag(int nMag);
    void setIdent_mag(quint8 value,int nMag);

    quint8 getIdent_class();
    void setIdent_class(quint8 value);
    quint8 getIdent_data();
    void setIdent_data(quint8 value);
    quint8 getIdent_version();
    void setIdent_version(quint8 value);

    quint8 getIdent_osabi();
    void setIdent_osabi(quint8 value);
    quint8 getIdent_abiversion();
    void setIdent_abiversion(quint8 value);

    quint8 getIdent_pad(int nPad);
    void setIdent_pad(quint8 value,int nPad);

    // TODO Hdr32 getHdr32();
    quint16 getHdr32_type();
    void setHdr32_type(quint16 value);
    quint16 getHdr32_machine();
    void setHdr32_machine(quint16 value);
    quint32 getHdr32_version();
    void setHdr32_version(quint32 value);
    quint32 getHdr32_entry();
    void setHdr32_entry(quint32 value);
    quint32 getHdr32_phoff();
    void setHdr32_phoff(quint32 value);
    quint32 getHdr32_shoff();
    void setHdr32_shoff(quint32 value);
    quint32 getHdr32_flags();
    void setHdr32_flags(quint32 value);
    quint16 getHdr32_ehsize();
    void setHdr32_ehsize(quint16 value);
    quint16 getHdr32_phentsize();
    void setHdr32_phentsize(quint16 value);
    quint16 getHdr32_phnum();
    void setHdr32_phnum(quint16 value);
    quint16 getHdr32_shentsize();
    void setHdr32_shentsize(quint16 value);
    quint16 getHdr32_shnum();
    void setHdr32_shnum(quint16 value);
    quint16 getHdr32_shstrndx();
    void setHdr32_shstrndx(quint16 value);

    quint16 getHdr64_type();
    void setHdr64_type(quint16 value);
    quint16 getHdr64_machine();
    void setHdr64_machine(quint16 value);
    quint32 getHdr64_version();
    void setHdr64_version(quint32 value);
    quint64 getHdr64_entry();
    void setHdr64_entry(quint64 value);
    quint64 getHdr64_phoff();
    void setHdr64_phoff(quint64 value);
    quint64 getHdr64_shoff();
    void setHdr64_shoff(quint64 value);
    quint32 getHdr64_flags();
    void setHdr64_flags(quint32 value);
    quint16 getHdr64_ehsize();
    void setHdr64_ehsize(quint16 value);
    quint16 getHdr64_phentsize();
    void setHdr64_phentsize(quint16 value);
    quint16 getHdr64_phnum();
    void setHdr64_phnum(quint16 value);
    quint16 getHdr64_shentsize();
    void setHdr64_shentsize(quint16 value);
    quint16 getHdr64_shnum();
    void setHdr64_shnum(quint16 value);
    quint16 getHdr64_shstrndx();
    void setHdr64_shstrndx(quint16 value);

    XELF_DEF::Elf_Ehdr getHdr();

    static QMap<quint64,QString> getHeaderVersionList();

    static QMap<quint64,QString> getIndentClasses();
    static QMap<quint64,QString> getIndentClassesS();
    static QMap<quint64,QString> getIndentDatas();
    static QMap<quint64,QString> getIndentDatasS();
    static QMap<quint64,QString> getIndentVersions();
    static QMap<quint64,QString> getIndentVersionsS();
    static QMap<quint64,QString> getIndentOsabis();
    static QMap<quint64,QString> getIndentOsabisS();

    static QMap<quint64,QString> getTypes();
    static QMap<quint64,QString> getTypesS();
    static QMap<quint64,QString> getMachines();
    static QMap<quint64,QString> getMachinesS();

    static QMap<quint64,QString> getSectionTypes();
    static QMap<quint64,QString> getSectionTypesS();
    static QMap<quint64,QString> getSectionFlags();
    static QMap<quint64,QString> getSectionFlagsS();

    static QMap<quint64,QString> getProgramTypes();
    static QMap<quint64,QString> getProgramTypesS();
    static QMap<quint64,QString> getProgramFlags();
    static QMap<quint64,QString> getProgramFlagsS();

    quint16 getSectionStringTable();
    quint16 getSectionStringTable(bool bIs64);

    QMap<quint32,QString> getStringsFromSection(quint32 nSection);
    QString getStringFromSection(quint32 nIndex,quint32 nSection);
    QMap<quint32,QString> getStringsFromMainSection();
    QString getStringFromMainSection(quint32 nIndex);
    QByteArray getSection(quint32 nIndex);
    bool isSectionValid(quint32 nIndex);
    // TODO Set
    QList<XELF_DEF::Elf32_Shdr> getElf32_ShdrList();
    QList<XELF_DEF::Elf64_Shdr> getElf64_ShdrList();
    QList<XELF_DEF::Elf_Shdr> getElf_ShdrList();
    // TODO Set
    XELF_DEF::Elf32_Shdr getElf32_Shdr(quint32 nIndex);
    XELF_DEF::Elf64_Shdr getElf64_Shdr(quint32 nIndex);

    quint32 getElf32_Shdr_name(quint32 nIndex);
    quint32 getElf32_Shdr_type(quint32 nIndex);
    quint32 getElf32_Shdr_flags(quint32 nIndex);
    quint32 getElf32_Shdr_addr(quint32 nIndex);
    quint32 getElf32_Shdr_offset(quint32 nIndex);
    quint32 getElf32_Shdr_size(quint32 nIndex);
    quint32 getElf32_Shdr_link(quint32 nIndex);
    quint32 getElf32_Shdr_info(quint32 nIndex);
    quint32 getElf32_Shdr_addralign(quint32 nIndex);
    quint32 getElf32_Shdr_entsize(quint32 nIndex);

    void setElf32_Shdr_name(quint32 nIndex,quint32 value);
    void setElf32_Shdr_type(quint32 nIndex,quint32 value);
    void setElf32_Shdr_flags(quint32 nIndex,quint32 value);
    void setElf32_Shdr_addr(quint32 nIndex,quint32 value);
    void setElf32_Shdr_offset(quint32 nIndex,quint32 value);
    void setElf32_Shdr_size(quint32 nIndex,quint32 value);
    void setElf32_Shdr_link(quint32 nIndex,quint32 value);
    void setElf32_Shdr_info(quint32 nIndex,quint32 value);
    void setElf32_Shdr_addralign(quint32 nIndex,quint32 value);
    void setElf32_Shdr_entsize(quint32 nIndex,quint32 value);

    quint32 getElf64_Shdr_name(quint32 nIndex);
    quint32 getElf64_Shdr_type(quint32 nIndex);
    quint64 getElf64_Shdr_flags(quint32 nIndex);
    quint64 getElf64_Shdr_addr(quint32 nIndex);
    quint64 getElf64_Shdr_offset(quint32 nIndex);
    quint64 getElf64_Shdr_size(quint32 nIndex);
    quint32 getElf64_Shdr_link(quint32 nIndex);
    quint32 getElf64_Shdr_info(quint32 nIndex);
    quint64 getElf64_Shdr_addralign(quint32 nIndex);
    quint64 getElf64_Shdr_entsize(quint32 nIndex);

    void setElf64_Shdr_name(quint32 nIndex,quint32 value);
    void setElf64_Shdr_type(quint32 nIndex,quint32 value);
    void setElf64_Shdr_flags(quint32 nIndex,quint64 value);
    void setElf64_Shdr_addr(quint32 nIndex,quint64 value);
    void setElf64_Shdr_offset(quint32 nIndex,quint64 value);
    void setElf64_Shdr_size(quint32 nIndex,quint64 value);
    void setElf64_Shdr_link(quint32 nIndex,quint32 value);
    void setElf64_Shdr_info(quint32 nIndex,quint32 value);
    void setElf64_Shdr_addralign(quint32 nIndex,quint64 value);
    void setElf64_Shdr_entsize(quint32 nIndex,quint64 value);

    quint32 getElf_Shdr_name(quint32 nIndex,QList<XELF_DEF::Elf_Shdr> *pList);
    quint32 getElf_Shdr_type(quint32 nIndex,QList<XELF_DEF::Elf_Shdr> *pList);
    quint64 getElf_Shdr_flags(quint32 nIndex,QList<XELF_DEF::Elf_Shdr> *pList);
    quint64 getElf_Shdr_addr(quint32 nIndex,QList<XELF_DEF::Elf_Shdr> *pList);
    quint64 getElf_Shdr_offset(quint32 nIndex,QList<XELF_DEF::Elf_Shdr> *pList);
    quint64 getElf_Shdr_size(quint32 nIndex,QList<XELF_DEF::Elf_Shdr> *pList);
    quint32 getElf_Shdr_link(quint32 nIndex,QList<XELF_DEF::Elf_Shdr> *pList);
    quint32 getElf_Shdr_info(quint32 nIndex,QList<XELF_DEF::Elf_Shdr> *pList);
    quint64 getElf_Shdr_addralign(quint32 nIndex,QList<XELF_DEF::Elf_Shdr> *pList);
    quint64 getElf_Shdr_entsize(quint32 nIndex,QList<XELF_DEF::Elf_Shdr> *pList);

    qint64 getShdrOffset(quint32 nIndex);
    qint64 getShdrSize();

    QList<XELF_DEF::Elf32_Phdr> getElf32_PhdrList();
    QList<XELF_DEF::Elf64_Phdr> getElf64_PhdrList();
    QList<XELF_DEF::Elf_Phdr> getElf_PhdrList();

    XELF_DEF::Elf32_Phdr getElf32_Phdr(quint32 nIndex);
    XELF_DEF::Elf64_Phdr getElf64_Phdr(quint32 nIndex);

    quint32 getElf32_Phdr_type(quint32 nIndex);
    quint32 getElf32_Phdr_offset(quint32 nIndex);
    quint32 getElf32_Phdr_vaddr(quint32 nIndex);
    quint32 getElf32_Phdr_paddr(quint32 nIndex);
    quint32 getElf32_Phdr_filesz(quint32 nIndex);
    quint32 getElf32_Phdr_memsz(quint32 nIndex);
    quint32 getElf32_Phdr_flags(quint32 nIndex);
    quint32 getElf32_Phdr_align(quint32 nIndex);

    void setElf32_Phdr_type(quint32 nIndex,quint32 value);
    void setElf32_Phdr_offset(quint32 nIndex,quint32 value);
    void setElf32_Phdr_vaddr(quint32 nIndex,quint32 value);
    void setElf32_Phdr_paddr(quint32 nIndex,quint32 value);
    void setElf32_Phdr_filesz(quint32 nIndex,quint32 value);
    void setElf32_Phdr_memsz(quint32 nIndex,quint32 value);
    void setElf32_Phdr_flags(quint32 nIndex,quint32 value);
    void setElf32_Phdr_align(quint32 nIndex,quint32 value);

    quint32 getElf64_Phdr_type(quint32 nIndex);
    quint64 getElf64_Phdr_offset(quint32 nIndex);
    quint64 getElf64_Phdr_vaddr(quint32 nIndex);
    quint64 getElf64_Phdr_paddr(quint32 nIndex);
    quint64 getElf64_Phdr_filesz(quint32 nIndex);
    quint64 getElf64_Phdr_memsz(quint32 nIndex);
    quint32 getElf64_Phdr_flags(quint32 nIndex);
    quint64 getElf64_Phdr_align(quint32 nIndex);

    void setElf64_Phdr_type(quint32 nIndex,quint32 value);
    void setElf64_Phdr_offset(quint32 nIndex,quint64 value);
    void setElf64_Phdr_vaddr(quint32 nIndex,quint64 value);
    void setElf64_Phdr_paddr(quint32 nIndex,quint64 value);
    void setElf64_Phdr_filesz(quint32 nIndex,quint64 value);
    void setElf64_Phdr_memsz(quint32 nIndex,quint64 value);
    void setElf64_Phdr_flags(quint32 nIndex,quint32 value);
    void setElf64_Phdr_align(quint32 nIndex,quint64 value);

    quint32 getElf_Phdr_type(quint32 nIndex,QList<XELF_DEF::Elf_Phdr> *pList);
    quint64 getElf_Phdr_offset(quint32 nIndex,QList<XELF_DEF::Elf_Phdr> *pList);
    quint64 getElf_Phdr_vaddr(quint32 nIndex,QList<XELF_DEF::Elf_Phdr> *pList);
    quint64 getElf_Phdr_paddr(quint32 nIndex,QList<XELF_DEF::Elf_Phdr> *pList);
    quint64 getElf_Phdr_filesz(quint32 nIndex,QList<XELF_DEF::Elf_Phdr> *pList);
    quint64 getElf_Phdr_memsz(quint32 nIndex,QList<XELF_DEF::Elf_Phdr> *pList);
    quint32 getElf_Phdr_flags(quint32 nIndex,QList<XELF_DEF::Elf_Phdr> *pList);
    quint64 getElf_Phdr_align(quint32 nIndex,QList<XELF_DEF::Elf_Phdr> *pList);

    qint64 getPhdrOffset(quint32 nIndex);
    qint64 getPhdrSize();

    int getSectionIndexByName(QString sSectionName);
    QByteArray getSectionByName(QString sSectionName);

    OS_ANSISTRING getProgramInterpreterName();
    OS_ANSISTRING getProgramInterpreterName(QList<XELF_DEF::Elf_Phdr> *pPhdrList);
    QString getCommentString();
//    QStrin
    QList<NOTE> getNotes();
    QList<NOTE> getNotes(QList<XELF_DEF::Elf_Phdr> *pPhdrList);
    NOTE _readNote(qint64 nOffset,qint64 nSize,bool bIsBigEndian);

    QList<TAG_STRUCT> getTagStructs();
    QList<TAG_STRUCT> getTagStructs(QList<XELF_DEF::Elf_Phdr> *pPhdrList,_MEMORY_MAP *pMemoryMap);

    static QList<TAG_STRUCT> _getTagStructs(QList<TAG_STRUCT> *pList, qint64 nTag);

    qint64 getDynamicArraySize();

    qint64 getDynamicArrayTag(qint64 nOffset);
    qint64 getDynamicArrayValue(qint64 nOffset);

    void setDynamicArrayTag(qint64 nOffset,qint64 nValue);
    void setDynamicArrayValue(qint64 nOffset,qint64 nValue);

    QList<QString> getLibraries();
    QList<QString> getLibraries(_MEMORY_MAP *pMemoryMap, QList<TAG_STRUCT> *pList);

    OS_ANSISTRING getRunPath();
    OS_ANSISTRING getRunPath(_MEMORY_MAP *pMemoryMap, QList<TAG_STRUCT> *pList);

    static QMap<quint64,QString> getDynamicTags();
    static QMap<quint64,QString> getDynamicTagsS();

    virtual _MEMORY_MAP getMemoryMap();
    virtual qint64 getEntryPointOffset(_MEMORY_MAP *pMemoryMap);

    static QList<SECTION_RECORD> getSectionRecords(QList<XELF_DEF::Elf_Shdr> *pList,bool bIsImage,QByteArray *pbaSectionTable);
    bool isSectionNamePresent(QString sSectionName);
    static bool isSectionNamePresent(QString sSectionName,QList<SECTION_RECORD> *pListSections);
    qint32 getSectionNumber(QString sSectionName);
    static qint32 getSectionNumber(QString sSectionName,QList<SECTION_RECORD> *pListSections);
    static SECTION_RECORD getSectionRecord(QString sSectionName,QList<SECTION_RECORD> *pListSections);

    virtual MODE getMode();
    virtual QString getArch();

    virtual qint64 getBaseAddress();
    virtual void setBaseAddress(qint64 nValue);

    QList<XELF_DEF::Elf_Phdr> getPrograms(QList<XELF_DEF::Elf_Phdr> *pList, quint32 nType);
};

#endif // XELF_H
