/* Copyright (c) 2024 hors<horsicq@gmail.com>
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
#ifndef XAMIGAHUNK_H
#define XAMIGAHUNK_H

#include "xbinary.h"
#include "xamigahunk_def.h"
// https://github.com/robcowell/Atari_ST_Sources/blob/6f0b3c85c64fb8b0a2e03be8d4efe07c97fd7aee/Docs/92GUIDE/doc/amiga.txt#L225
// http://amiga-dev.wikidot.com/file-format:hunk
// https://github.com/corkami/pics/blob/master/binary/hunk.png
// https://github.com/yuduo/IDA/blob/936973010a27a22b6ee04e2d3a1a908a89b0c581/idasdk66/ldr/amiga/amiga.cpp#L44
class XAmigaHunk : public XBinary {
    Q_OBJECT

public:
    struct HUNK {
        quint32 nId;
        qint64 nOffset;
        qint64 nSize;
    };

    enum TYPE {
        TYPE_UNKNOWN = 0,
        TYPE_EXECUTABLE,
        TYPE_OBJECT,
        // TODO More
        // TODO Library
    };

    explicit XAmigaHunk(QIODevice *pDevice = nullptr, bool bIsImage = false, XADDR nModuleAddress = -1);
    ~XAmigaHunk();

    virtual bool isValid(PDSTRUCT *pPdStruct = nullptr);
    static QList<MAPMODE> getMapModesList();
    virtual _MEMORY_MAP getMemoryMap(MAPMODE mapMode = MAPMODE_UNKNOWN, PDSTRUCT *pPdStruct = nullptr);
    virtual ENDIAN getEndian();
    virtual QString getArch();
    QString getArch(QList<HUNK> *pListHunks);
    virtual MODE getMode();
    MODE getMode(QList<HUNK> *pListHunks);

    QList<HUNK> getHunks(PDSTRUCT *pPdStruct = nullptr);
    static QString hunkTypeToString(quint32 nHunkType);

    bool isHunkPresent(QList<HUNK> *pListHunks, quint32 nHunkType);
    static QList<HUNK> _getHunksByType(QList<HUNK> *pListHunks, quint32 nHunkType);

    virtual FT getFileType();
    virtual OSINFO getOsInfo();
    virtual QString getFileFormatExt();
    virtual qint64 getFileFormatSize(PDSTRUCT *pPdStruct);
    virtual QString getFileFormatString();
    virtual FILEFORMATINFO getFileFormatInfo(PDSTRUCT *pPdStruct);

    virtual qint32 getType();
    QString typeIdToString(qint32 nType);
};

#endif  // XAMIGAHUNK_H
