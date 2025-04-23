/* Copyright (c) 2025 hors<horsicq@gmail.com>
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
#ifndef XCFBF_H
#define XCFBF_H

#include "xbinary.h"

class XCFBF : public XBinary {
    Q_OBJECT

public:
    struct StructuredStorageHeader { // [offset from start (bytes), length (bytes)]
        quint8 _abSig[8];           // [00H,08] {0xd0, 0xcf, 0x11, 0xe0, 0xa1, 0xb1,
            // 0x1a, 0xe1} for current version
        quint32 _clsid[4];          // [08H,16] reserved must be zero (WriteClassStg/
            // GetClassFile uses root directory class id)
        quint16 _uMinorVersion;     // [18H,02] minor version of the format: 33 is
            // written by reference implementation
        quint16 _uDllVersion;       // [1AH,02] major version of the dll/format: 3 for
            // 512-byte sectors, 4 for 4 KB sectors
        quint16 _uByteOrder;        // [1CH,02] 0xFFFE: indicates Intel byte-ordering
        quint16 _uSectorShift;      // [1EH,02] size of sectors in power-of-two;
            // typically 9 indicating 512-byte sectors
        quint16 _uMiniSectorShift;  // [20H,02] size of mini-sectors in power-of-two;
            // typically 6 indicating 64-byte mini-sectors
        quint16 _usReserved;        // [22H,02] reserved, must be zero
        quint32 _ulReserved1;       // [24H,04] reserved, must be zero
        quint32 _csectDir;          // [28H,04] must be zero for 512-byte sectors,
            // number of SECTs in directory chain for 4 KB
            // sectors
        quint32 _csectFat;          // [2CH,04] number of SECTs in the FAT chain
        quint32 _sectDirStart;      // [30H,04] first SECT in the directory chain
        quint32 _signature;         // [34H,04] signature used for transactions; must
            // be zero. The reference implementation
            // does not support transactions
        quint32 _ulMiniSectorCutoff; // [38H,04] maximum size for a mini stream;
            // typically 4096 bytes
        quint32 _sectMiniFatStart;  // [3CH,04] first SECT in the MiniFAT chain
        quint32 _csectMiniFat;      // [40H,04] number of SECTs in the MiniFAT chain
        quint32 _sectDifStart;      // [44H,04] first SECT in the DIFAT chain
        quint32 _csectDif;          // [48H,04] number of SECTs in the DIFAT chain
        quint32 _sectFat[109];      // [4CH,436] the SECTs of first 109 FAT sectors
    };


    explicit XCFBF(QIODevice *pDevice = nullptr);
    ~XCFBF();

    virtual bool isValid(PDSTRUCT *pPdStruct = nullptr);
    static bool isValid(QIODevice *pDevice);

    virtual QString getArch();
    virtual MODE getMode();
    virtual ENDIAN getEndian();
    virtual FT getFileType();
    virtual QString getVersion();
    virtual QString getFileFormatExt();

    _MEMORY_MAP getMemoryMap(MAPMODE mapMode = MAPMODE_UNKNOWN, PDSTRUCT *pPdStruct = nullptr);

    StructuredStorageHeader read_StructuredStorageHeader(qint64 nOffset, PDSTRUCT *pPdStruct);
};

#endif  // XCFBF_H
