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
#include "xpyc.h"

namespace {
struct MAGIC_RECORD {
    quint16 nMagic;
    const char *pszVersion;
};

static const MAGIC_RECORD g_records[] = {{2012, "1.5.2"},  {3000, "3000"},   {3111, "3.0a4"},  {3131, "3.0b1"},  {3141, "3.1a1"},
                                         {3151, "3.1a1"},  {3160, "3.2a1"},  {3170, "3.2a2"},  {3180, "3.2a3"},  {3190, "3.3a1"},
                                         {3200, "3.3a1"},  {3210, "3.3a1"},  {3220, "3.3a2"},  {3230, "3.3a4"},  {3250, "3.4a1"},
                                         {3260, "3.4a1"},  {3270, "3.4a1"},  {3280, "3.4a1"},  {3290, "3.4a4"},  {3300, "3.4a4"},
                                         {3310, "3.4rc2"}, {3320, "3.5a1"},  {3330, "3.5b1"},  {3340, "3.5b2"},  {3350, "3.5b3"},
                                         {3351, "3.5.2"},  {3360, "3.6a0"},  {3361, "3.6a1"},  {3370, "3.6a2"},  {3371, "3.6a2"},
                                         {3372, "3.6a2"},  {3373, "3.6b1"},  {3375, "3.6b1"},  {3376, "3.6b1"},  {3377, "3.6b1"},
                                         {3378, "3.6b2"},  {3379, "3.6rc1"}, {3390, "3.7a1"},  {3391, "3.7a2"},  {3392, "3.7a4"},
                                         {3393, "3.7b1"},  {3394, "3.7b5"},  {3400, "3.8a1"},  {3401, "3.8a1"},  {3410, "3.8a1"},
                                         {3411, "3.8b2"},  {3412, "3.8b2"},  {3413, "3.8b4"},  {3420, "3.9a0"},  {3421, "3.9a0"},
                                         {3422, "3.9a0"},  {3423, "3.9a2"},  {3424, "3.9a2"},  {3425, "3.9a2"},  {3430, "3.10a1"},
                                         {3431, "3.10a1"}, {3432, "3.10a2"}, {3433, "3.10a2"}, {3434, "3.10a6"}, {3435, "3.10a7"},
                                         {3436, "3.10b1"}, {3437, "3.10b1"}, {3438, "3.10b1"}, {3439, "3.10b1"}, {3450, "3.11a1"},
                                         {3451, "3.11a1"}, {3452, "3.11a1"}, {3453, "3.11a1"}, {3454, "3.11a1"}, {3455, "3.11a1"},
                                         {3456, "3.11a1"}, {3457, "3.11a1"}, {3458, "3.11a1"}, {3459, "3.11a1"}, {3460, "3.11a1"},
                                         {3461, "3.11a1"}, {3462, "3.11a2"}, {3463, "3.11a3"}, {3464, "3.11a3"}, {3465, "3.11a3"},
                                         {3466, "3.11a4"}, {3467, "3.11a4"}, {3468, "3.11a4"}, {3469, "3.11a4"}, {3470, "3.11a4"},
                                         {3471, "3.11a4"}, {3472, "3.11a4"}, {3473, "3.11a4"}, {3474, "3.11a4"}, {3475, "3.11a5"},
                                         {3476, "3.11a5"}, {3477, "3.11a5"}, {3478, "3.11a5"}, {3479, "3.11a5"}, {3480, "3.11a5"},
                                         {3481, "3.11a5"}, {3482, "3.11a5"}, {3483, "3.11a5"}, {3484, "3.11a5"}, {3485, "3.11a5"},
                                         {3486, "3.11a6"}, {3487, "3.11a6"}, {3488, "3.11a6"}, {3489, "3.11a6"}, {3490, "3.11a6"},
                                         {3491, "3.11a6"}, {3492, "3.11a7"}, {3493, "3.11a7"}, {3494, "3.11a7"}, {3495, "3.11b4"},
                                         {3500, "3.12a1"}, {3501, "3.12a1"}, {3502, "3.12a1"}, {3503, "3.12a1"}, {3504, "3.12a1"},
                                         {3505, "3.12a1"}, {3506, "3.12a1"}, {3507, "3.12a1"}, {3508, "3.12a1"}, {3509, "3.12a1"},
                                         {3510, "3.12a2"}, {3511, "3.12a2"}, {3512, "3.12a2"}, {3513, "3.12a4"}, {3514, "3.12a4"},
                                         {3515, "3.12a5"}, {3516, "3.12a5"}, {3517, "3.12a5"}, {3518, "3.12a6"}, {3519, "3.12a6"},
                                         {3520, "3.12a6"}, {3521, "3.12a7"}, {3522, "3.12a7"}, {3523, "3.12a7"}, {3524, "3.12a7"},
                                         {3525, "3.12b1"}, {3526, "3.12b1"}, {3527, "3.12b1"}, {3528, "3.12b1"}, {3529, "3.12b1"},
                                         {3530, "3.12b1"}, {3531, "3.12b1"}, {3550, "3.13a1"}, {3551, "3.13a1"}, {3552, "3.13a1"},
                                         {3553, "3.13a1"}, {3554, "3.13a1"}, {3555, "3.13a1"}, {3556, "3.13a1"}, {3557, "3.13a1"},
                                         {3558, "3.13a1"}, {3559, "3.13a1"}, {3560, "3.13a1"}, {3561, "3.13a1"}, {3562, "3.13a1"},
                                         {3563, "3.13a1"}, {3564, "3.13a1"}, {3565, "3.13a1"}, {3566, "3.13a1"}, {3567, "3.13a1"},
                                         {3568, "3.13a1"}, {3569, "3.13a5"}, {3570, "3.13a6"}, {3571, "3.13b1"}, {3600, "3.14 will start with"},
                                         {5042, "1.6"},    {5082, "2.0.1"},  {6020, "2.1.2"},  {6071, "2.2"},    {6201, "2.3a0"},
                                         {6202, "2.3a0"},  {6204, "2.4a0"},  {6205, "2.4a3"},  {6206, "2.4b1"},  {6207, "2.5a0"},
                                         {6208, "2.5a0"},  {6209, "2.5a0"},  {6210, "2.5b3"},  {6211, "2.5b3"},  {6212, "2.5c1"},
                                         {6213, "2.5c2"},  {6215, "2.6a0"},  {6216, "2.6a1"},  {6217, "2.7a0"},  {6218, "2.7a0"},
                                         {6219, "2.7a0"},  {6220, "2.7a0"},  {6221, "2.7a0"}};

static qint32 getRecordsCount()
{
    return (qint32)(sizeof(g_records) / sizeof(g_records[0]));
}
}  // namespace

XPYC::XPYC(QIODevice *pDevice) : XBinary(pDevice)
{
}

XPYC::~XPYC()
{
}

bool XPYC::isValid(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    bool bResult = false;

    if (getSize() >= 12) {
        quint16 nMagicMarker = read_uint16(2);

        if (nMagicMarker == 0x0A0D) {
            quint16 nMagicValue = read_uint16(0);

            if (_isMagicKnown(nMagicValue)) {
                bResult = true;
            }
        }
    }

    return bResult;
}

bool XPYC::isValid(QIODevice *pDevice)
{
    XPYC xpyc(pDevice);

    return xpyc.isValid();
}

QString XPYC::getArch()
{
    return "Python VM";
}

XBinary::MODE XPYC::getMode()
{
    return MODE_DATA;
}

XBinary::ENDIAN XPYC::getEndian()
{
    return ENDIAN_LITTLE;
}

XBinary::FT XPYC::getFileType()
{
    return FT_PYC;
}

QString XPYC::getVersion()
{
    INFO info = getInternalInfo();

    return info.sVersion;
}

QString XPYC::getFileFormatExt()
{
    return "pyc";
}

QString XPYC::getFileFormatExtsString()
{
    return "Python Compiled (*.pyc)";
}

QString XPYC::getMIMEString()
{
    return "application/x-python-code";
}

XPYC::INFO XPYC::getInternalInfo(PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pPdStruct)

    INFO info = {};

    if (getSize() < 4) {
        return info;
    }

    info.nMagicValue = read_uint16(0);
    info.nMagicMarker = read_uint16(2);
    info.sVersion = _magicToVersion(info.nMagicValue);
    info.nFlags = 0;
    info.bHashBased = false;
    info.nTimestamp = 0;
    info.nSourceSize = 0;
    info.baHash.clear();

    qint32 nMajor = 0;
    qint32 nMinor = 0;
    _parseVersionNumbers(info.sVersion, &nMajor, &nMinor);

    qint64 nOffset = 4;

    if (getSize() >= (nOffset + 4)) {
        quint32 nWord = read_uint32(nOffset);
        bool bUseNewLayout = false;

        if ((nMajor > 3) || ((nMajor == 3) && (nMinor >= 7))) {
            bUseNewLayout = true;
        }

        if (bUseNewLayout) {
            info.nFlags = nWord;
            info.bHashBased = ((info.nFlags & 0x00000001) != 0);
            nOffset += 4;

            if (info.bHashBased) {
                if (getSize() >= (nOffset + 8)) {
                    info.baHash = read_array(nOffset, 8);
                    nOffset += 8;
                }

                if (getSize() >= (nOffset + 4)) {
                    info.nSourceSize = read_uint32(nOffset);
                }
            } else {
                if (getSize() >= (nOffset + 4)) {
                    info.nTimestamp = read_uint32(nOffset);
                    nOffset += 4;
                }

                if (getSize() >= (nOffset + 4)) {
                    info.nSourceSize = read_uint32(nOffset);
                }
            }
        } else {
            info.nTimestamp = nWord;

            if (getSize() >= (nOffset + 8)) {
                info.nSourceSize = read_uint32(nOffset + 4);
            }
        }
    }

    return info;
}

XBinary::_MEMORY_MAP XPYC::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(mapMode)

    _MEMORY_MAP result = {};

    qint64 nTotalSize = getSize();

    result.nBinarySize = nTotalSize;
    result.fileType = getFileType();
    result.mode = getMode();
    result.sArch = getArch();
    result.endian = getEndian();
    result.sType = getTypeAsString();

    _MEMORY_RECORD record = {};
    record.nAddress = -1;
    record.nOffset = 0;
    record.nSize = nTotalSize;
    record.filePart = FILEPART_DATA;
    record.nIndex = 0;
    record.sName = tr("Data");

    result.listRecords.append(record);

    _handleOverlay(&result);

    return result;
}

QString XPYC::_magicToVersion(quint16 nMagicValue)
{
    QString sResult;

    qint32 nCount = getRecordsCount();

    for (qint32 i = 0; i < nCount; i++) {
        if (g_records[i].nMagic == nMagicValue) {
            sResult = QString::fromLatin1(g_records[i].pszVersion);
            break;
        }
    }

    return sResult;
}

bool XPYC::_isMagicKnown(quint16 nMagicValue)
{
    bool bResult = false;

    qint32 nCount = getRecordsCount();

    for (qint32 i = 0; i < nCount; i++) {
        if (g_records[i].nMagic == nMagicValue) {
            bResult = true;
            break;
        }
    }

    return bResult;
}

void XPYC::_parseVersionNumbers(const QString &sVersion, qint32 *pnMajor, qint32 *pnMinor)
{
    if (pnMajor) {
        *pnMajor = 0;
    }

    if (pnMinor) {
        *pnMinor = 0;
    }

    qint32 nLength = sVersion.length();
    qint32 nIndex = 0;

    qint32 nCurrentMajor = 0;
    qint32 nCurrentMinor = 0;

    while ((nIndex < nLength) && sVersion.at(nIndex).isDigit()) {
        int nDigit = sVersion.at(nIndex).digitValue();

        if (nDigit >= 0) {
            nCurrentMajor = nCurrentMajor * 10 + nDigit;
        }

        nIndex++;
    }

    bool bHasDot = false;

    if ((nIndex < nLength) && (sVersion.at(nIndex) == '.')) {
        bHasDot = true;
        nIndex++;

        while ((nIndex < nLength) && sVersion.at(nIndex).isDigit()) {
            int nDigit = sVersion.at(nIndex).digitValue();

            if (nDigit >= 0) {
                nCurrentMinor = nCurrentMinor * 10 + nDigit;
            }

            nIndex++;
        }
    }

    if ((!bHasDot) && (sVersion == "3000")) {
        nCurrentMajor = 3;
        nCurrentMinor = 0;
    }

    if (pnMajor) {
        *pnMajor = nCurrentMajor;
    }

    if (pnMinor) {
        *pnMinor = nCurrentMinor;
    }
}
