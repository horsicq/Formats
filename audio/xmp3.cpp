/* Copyright (c) 2022-2026 hors<horsicq@gmail.com>
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
#include "xmp3.h"

static XBinary::XCONVERT _TABLE_XMP3_STRUCTID[] = {
    {XMP3::STRUCTID_UNKNOWN, "Unknown", QObject::tr("Unknown")},
};

XMP3::XMP3(QIODevice *pDevice) : XBinary(pDevice)
{
}

XMP3::~XMP3()
{
}

bool XMP3::isValid(PDSTRUCT *pPdStruct)
{
    bool bResult = false;

    if (getSize() > 0x20) {
        _MEMORY_MAP memoryMap = XBinary::getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);

        if (compareSignature(&memoryMap, "'ID3'0200", 0, pPdStruct) || compareSignature(&memoryMap, "'ID3'0300", 0, pPdStruct) ||
            compareSignature(&memoryMap, "'ID3'0400", 0, pPdStruct)) {
            // TODO more checks
            bResult = true;
        }
    }

    return bResult;
}

bool XMP3::isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    XMP3 mp3(pDevice);

    return mp3.isValid(pPdStruct);
}

QString XMP3::getFileFormatExt()
{
    return "mp3";
}

QString XMP3::getFileFormatExtsString()
{
    return "MP3";
}

qint64 XMP3::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    return _calculateRawSize(pPdStruct);
}

XBinary::_MEMORY_MAP XMP3::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(mapMode)
    return _getMemoryMap(FILEPART_HEADER | FILEPART_REGION | FILEPART_OVERLAY, pPdStruct);
}

XBinary::FT XMP3::getFileType()
{
    return FT_MP3;
}

QString XMP3::getVersion()
{
    QString sResult;

    if (getSize() > 0x20) {
        _MEMORY_MAP memoryMap = XBinary::getMemoryMap();

        if (compareSignature(&memoryMap, "'ID3'0200", 0)) {
            sResult = "3.2";
        } else if (compareSignature(&memoryMap, "'ID3'0300", 0)) {
            sResult = "3.3";
        } else if (compareSignature(&memoryMap, "'ID3'0400", 0)) {
            sResult = "3.4";
        }
    }

    return sResult;
}

QString XMP3::getMIMEString()
{
    return "audio/mpeg";
}

QVector<XBinary::XMETADATA_STRUCT> XMP3::getMetadataStructs()
{
    QVector<XMETADATA_STRUCT> listResult;
    if (!isValid((PDSTRUCT *)nullptr) || !checkOffsetSize(0, 10)) {
        return listResult;
    }

    auto appendMetadata = [this, &listResult](qint64 nOffset, qint64 nSize, XMETADATA_ID id, const QString &sName, const QVariant &varValue) {
        XMETADATA_STRUCT record = {};
        record.nOffset = nOffset;
        record.nSize = nSize;
        record.nAddress = offsetToAddress(nOffset);
        record.id = id;
        record.sName = sName;
        record.varValue = varValue;
        listResult.append(record);
    };

    auto readSynchsafe = [this](qint64 nOffset) -> quint32 {
        return ((quint32)(read_uint8(nOffset) & 0x7F) << 21) | ((quint32)(read_uint8(nOffset + 1) & 0x7F) << 14) |
               ((quint32)(read_uint8(nOffset + 2) & 0x7F) << 7) | (read_uint8(nOffset + 3) & 0x7F);
    };

    auto decodeText = [](const QByteArray &baData) -> QString {
        if (baData.isEmpty()) {
            return QString();
        }

        const quint8 nEncoding = (quint8)baData.at(0);
        const QByteArray baText = baData.mid(1);
        QString sResult;
        if (nEncoding == 0) {
            sResult = QString::fromLatin1(baText);
        } else if (nEncoding == 3) {
            sResult = QString::fromUtf8(baText);
        } else if ((nEncoding == 1) || (nEncoding == 2)) {
            bool bBigEndian = (nEncoding == 2);
            qint32 nStart = 0;
            if (baText.size() >= 2) {
                const quint8 nFirst = (quint8)baText.at(0);
                const quint8 nSecond = (quint8)baText.at(1);
                if ((nFirst == 0xFE) && (nSecond == 0xFF)) {
                    bBigEndian = true;
                    nStart = 2;
                } else if ((nFirst == 0xFF) && (nSecond == 0xFE)) {
                    bBigEndian = false;
                    nStart = 2;
                }
            }
            for (qint32 i = nStart; i + 1 < baText.size(); i += 2) {
                const quint16 nCharacter = bBigEndian ? (((quint8)baText.at(i) << 8) | (quint8)baText.at(i + 1))
                                                       : (((quint8)baText.at(i + 1) << 8) | (quint8)baText.at(i));
                if (!nCharacter) {
                    break;
                }
                sResult.append(QChar(nCharacter));
            }
        }
        sResult.remove(QChar('\0'));
        return sResult.trimmed();
    };

    const quint8 nVersion = read_uint8(3);
    const quint8 nFlags = read_uint8(5);
    const quint32 nTagSize = readSynchsafe(6);
    const qint64 nTagEnd = qMin<qint64>(getSize(), 10 + (qint64)nTagSize);
    qint64 nFrameOffset = 10;

    if ((nFlags & 0x40) && (nFrameOffset + 4 <= nTagEnd)) {
        const quint32 nExtendedSize = (nVersion == 4) ? readSynchsafe(nFrameOffset) : read_uint32(nFrameOffset, true);
        nFrameOffset += (nVersion == 4) ? nExtendedSize : (4 + nExtendedSize);
    }

    while (nFrameOffset < nTagEnd) {
        const qint32 nHeaderSize = (nVersion == 2) ? 6 : 10;
        const qint32 nIdSize = (nVersion == 2) ? 3 : 4;
        if (nFrameOffset + nHeaderSize > nTagEnd) {
            break;
        }

        const QByteArray baId = read_array(nFrameOffset, nIdSize);
        if (baId.isEmpty() || (baId.at(0) == 0)) {
            break;
        }

        quint32 nFrameSize = 0;
        if (nVersion == 2) {
            nFrameSize = read_uint24(nFrameOffset + 3, true);
        } else if (nVersion == 4) {
            nFrameSize = readSynchsafe(nFrameOffset + 4);
        } else {
            nFrameSize = read_uint32(nFrameOffset + 4, true);
        }
        const qint64 nValueOffset = nFrameOffset + nHeaderSize;
        if (!nFrameSize || ((qint64)nFrameSize > nTagEnd - nValueOffset)) {
            break;
        }

        XMETADATA_ID id = XMETADATA_ID_UNKNOWN;
        QString sName;
        if ((baId == QByteArrayLiteral("TIT2")) || (baId == QByteArrayLiteral("TT2"))) {
            id = XMETADATA_ID_TITLE;
            sName = QString("Title");
        } else if ((baId == QByteArrayLiteral("TPE1")) || (baId == QByteArrayLiteral("TP1"))) {
            id = XMETADATA_ID_ARTIST;
            sName = QString("Artist");
        } else if ((baId == QByteArrayLiteral("TALB")) || (baId == QByteArrayLiteral("TAL"))) {
            id = XMETADATA_ID_ALBUM;
            sName = QString("Album");
        } else if ((baId == QByteArrayLiteral("TCON")) || (baId == QByteArrayLiteral("TCO"))) {
            id = XMETADATA_ID_GENRE;
            sName = QString("Genre");
        } else if ((baId == QByteArrayLiteral("TRCK")) || (baId == QByteArrayLiteral("TRK"))) {
            id = XMETADATA_ID_TRACK_NUMBER;
            sName = QString("Track number");
        }

        if (id != XMETADATA_ID_UNKNOWN) {
            const QString sValue = decodeText(read_array(nValueOffset, nFrameSize));
            if (!sValue.isEmpty()) {
                appendMetadata(nValueOffset, nFrameSize, id, sName, sValue);
                if ((id == XMETADATA_ID_TRACK_NUMBER) && sValue.contains(QChar('/'))) {
                    const QString sTrackCount = sValue.section(QChar('/'), 1, 1).trimmed();
                    if (!sTrackCount.isEmpty()) {
                        appendMetadata(nValueOffset, nFrameSize, XMETADATA_ID_TRACK_COUNT, QString("Track count"), sTrackCount);
                    }
                }
            }
        }

        nFrameOffset = nValueOffset + nFrameSize;
    }

    const qint64 nAudioOffset = 10 + (qint64)nTagSize;
    if (checkOffsetSize(nAudioOffset, 4)) {
        const quint32 nHeader = read_uint32(nAudioOffset, true);
        if ((nHeader & 0xFFE00000U) == 0xFFE00000U) {
            static const quint16 nMpeg1Layer3Bitrates[] = {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0};
            static const quint16 nMpeg2Layer3Bitrates[] = {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0};
            static const quint32 nSampleRates[] = {44100, 48000, 32000, 0};
            const quint8 nMpegVersion = (nHeader >> 19) & 3;
            const quint8 nLayer = (nHeader >> 17) & 3;
            const quint8 nBitrateIndex = (nHeader >> 12) & 0xF;
            const quint8 nSampleRateIndex = (nHeader >> 10) & 3;
            if ((nMpegVersion != 1) && (nLayer == 1) && (nSampleRateIndex != 3)) {
                const quint32 nBitrate = ((nMpegVersion == 3) ? nMpeg1Layer3Bitrates[nBitrateIndex] : nMpeg2Layer3Bitrates[nBitrateIndex]) * 1000;
                quint32 nSampleRate = nSampleRates[nSampleRateIndex];
                if (nMpegVersion == 2) nSampleRate /= 2;
                if (nMpegVersion == 0) nSampleRate /= 4;
                const quint32 nChannels = (((nHeader >> 6) & 3) == 3) ? 1 : 2;
                appendMetadata(nAudioOffset, 4, XMETADATA_ID_CODEC, QString("Codec"),
                               (nMpegVersion == 3) ? QString("MPEG-1 Layer III") : ((nMpegVersion == 2) ? QString("MPEG-2 Layer III") : QString("MPEG-2.5 Layer III")));
                appendMetadata(nAudioOffset, 4, XMETADATA_ID_BITRATE, QString("Bitrate"), nBitrate);
                appendMetadata(nAudioOffset, 4, XMETADATA_ID_SAMPLE_RATE, QString("Sample rate"), nSampleRate);
                appendMetadata(nAudioOffset, 4, XMETADATA_ID_CHANNELS, QString("Channels"), nChannels);
                if (nBitrate) {
                    appendMetadata(nAudioOffset, getSize() - nAudioOffset, XMETADATA_ID_DURATION, QString("Approximate duration"),
                                   (double)(getSize() - nAudioOffset) * 8 / nBitrate);
                }
            }
        }
    }

    return listResult;
}

qint64 XMP3::decodeFrame(qint64 nOffset)
{
    qint64 nResult = 0;

    quint32 nHeader = read_uint32(nOffset, true);

    if (nHeader & 0xFFE00000) {
        quint8 nVersion = (nHeader >> 19) & 0x3;
        quint8 nLayer = (nHeader >> 17) & 0x3;
        quint8 nErrorProtection = (nHeader >> 16) & 0x1;
        quint8 nBitRateIndex = (nHeader >> 12) & 0xF;
        quint8 nFrequencyIndex = (nHeader >> 10) & 0x3;
        quint8 nPadBit = (nHeader >> 9) & 0x1;
        quint8 nPrivBit = (nHeader >> 8) & 0x1;
        quint8 nMode = (nHeader >> 7) & 0x3;
        quint8 nIntensiveStereo = (nHeader >> 5) & 0x1;
        quint8 nMSStereo = (nHeader >> 4) & 0x1;
        quint8 nCopy = (nHeader >> 3) & 0x1;
        quint8 nOriginal = (nHeader >> 2) & 0x1;
        quint8 nEmphasis = (nHeader >> 0) & 0x3;

        Q_UNUSED(nPrivBit)
        Q_UNUSED(nMode)
        Q_UNUSED(nIntensiveStereo)
        Q_UNUSED(nErrorProtection)
        Q_UNUSED(nMSStereo)
        Q_UNUSED(nCopy)
        Q_UNUSED(nOriginal)
        Q_UNUSED(nEmphasis)

        quint32 nBitRate = 0;
        quint32 nFrequency = 0;
        double dTime = 0;

        if ((nVersion == 1) || (nVersion == 2)) {
            if ((nLayer == 1) || (nLayer == 2)) {  // III & II
                if (nBitRateIndex == 1) {
                    nBitRate = 8;
                } else if (nBitRateIndex == 2) {
                    nBitRate = 16;
                } else if (nBitRateIndex == 3) {
                    nBitRate = 24;
                } else if (nBitRateIndex == 4) {
                    nBitRate = 32;
                } else if (nBitRateIndex == 5) {
                    nBitRate = 40;
                } else if (nBitRateIndex == 6) {
                    nBitRate = 48;
                } else if (nBitRateIndex == 7) {
                    nBitRate = 56;
                } else if (nBitRateIndex == 8) {
                    nBitRate = 64;
                } else if (nBitRateIndex == 9) {
                    nBitRate = 80;
                } else if (nBitRateIndex == 10) {
                    nBitRate = 96;
                } else if (nBitRateIndex == 11) {
                    nBitRate = 112;
                } else if (nBitRateIndex == 12) {
                    nBitRate = 128;
                } else if (nBitRateIndex == 13) {
                    nBitRate = 144;
                } else if (nBitRateIndex == 14) {
                    nBitRate = 160;
                }
            } else if (nLayer == 3) {  // I
                if (nBitRateIndex == 1) {
                    nBitRate = 32;
                } else if (nBitRateIndex == 2) {
                    nBitRate = 48;
                } else if (nBitRateIndex == 3) {
                    nBitRate = 56;
                } else if (nBitRateIndex == 4) {
                    nBitRate = 64;
                } else if (nBitRateIndex == 5) {
                    nBitRate = 80;
                } else if (nBitRateIndex == 6) {
                    nBitRate = 96;
                } else if (nBitRateIndex == 7) {
                    nBitRate = 112;
                } else if (nBitRateIndex == 8) {
                    nBitRate = 128;
                } else if (nBitRateIndex == 9) {
                    nBitRate = 144;
                } else if (nBitRateIndex == 10) {
                    nBitRate = 160;
                } else if (nBitRateIndex == 11) {
                    nBitRate = 176;
                } else if (nBitRateIndex == 12) {
                    nBitRate = 192;
                } else if (nBitRateIndex == 13) {
                    nBitRate = 224;
                } else if (nBitRateIndex == 14) {
                    nBitRate = 256;
                }
            }
        } else if (nVersion == 3) {
            if (nLayer == 1) {  // III
                if (nBitRateIndex == 1) {
                    nBitRate = 32;
                } else if (nBitRateIndex == 2) {
                    nBitRate = 40;
                } else if (nBitRateIndex == 3) {
                    nBitRate = 48;
                } else if (nBitRateIndex == 4) {
                    nBitRate = 56;
                } else if (nBitRateIndex == 5) {
                    nBitRate = 64;
                } else if (nBitRateIndex == 6) {
                    nBitRate = 80;
                } else if (nBitRateIndex == 7) {
                    nBitRate = 96;
                } else if (nBitRateIndex == 8) {
                    nBitRate = 112;
                } else if (nBitRateIndex == 9) {
                    nBitRate = 128;
                } else if (nBitRateIndex == 10) {
                    nBitRate = 160;
                } else if (nBitRateIndex == 11) {
                    nBitRate = 192;
                } else if (nBitRateIndex == 12) {
                    nBitRate = 224;
                } else if (nBitRateIndex == 13) {
                    nBitRate = 256;
                } else if (nBitRateIndex == 14) {
                    nBitRate = 320;
                }
            } else if (nLayer == 2) {  // II
                if (nBitRateIndex == 1) {
                    nBitRate = 32;
                } else if (nBitRateIndex == 2) {
                    nBitRate = 48;
                } else if (nBitRateIndex == 3) {
                    nBitRate = 56;
                } else if (nBitRateIndex == 4) {
                    nBitRate = 64;
                } else if (nBitRateIndex == 5) {
                    nBitRate = 80;
                } else if (nBitRateIndex == 6) {
                    nBitRate = 96;
                } else if (nBitRateIndex == 7) {
                    nBitRate = 112;
                } else if (nBitRateIndex == 8) {
                    nBitRate = 128;
                } else if (nBitRateIndex == 9) {
                    nBitRate = 160;
                } else if (nBitRateIndex == 10) {
                    nBitRate = 192;
                } else if (nBitRateIndex == 11) {
                    nBitRate = 224;
                } else if (nBitRateIndex == 12) {
                    nBitRate = 256;
                } else if (nBitRateIndex == 13) {
                    nBitRate = 320;
                } else if (nBitRateIndex == 14) {
                    nBitRate = 384;
                }
            } else if (nLayer == 3) {  // I
                if (nBitRateIndex == 1) {
                    nBitRate = 32;
                } else if (nBitRateIndex == 2) {
                    nBitRate = 64;
                } else if (nBitRateIndex == 3) {
                    nBitRate = 96;
                } else if (nBitRateIndex == 4) {
                    nBitRate = 128;
                } else if (nBitRateIndex == 5) {
                    nBitRate = 160;
                } else if (nBitRateIndex == 6) {
                    nBitRate = 192;
                } else if (nBitRateIndex == 7) {
                    nBitRate = 224;
                } else if (nBitRateIndex == 8) {
                    nBitRate = 256;
                } else if (nBitRateIndex == 9) {
                    nBitRate = 288;
                } else if (nBitRateIndex == 10) {
                    nBitRate = 320;
                } else if (nBitRateIndex == 11) {
                    nBitRate = 352;
                } else if (nBitRateIndex == 12) {
                    nBitRate = 384;
                } else if (nBitRateIndex == 13) {
                    nBitRate = 416;
                } else if (nBitRateIndex == 14) {
                    nBitRate = 448;
                }
            }
        }

        if (nVersion == 1) {  // v2.5
            if (nFrequencyIndex == 0) {
                nFrequency = 11025;
            } else if (nFrequencyIndex == 1) {
                nFrequency = 12000;
            } else if (nFrequencyIndex == 2) {
                nFrequency = 8000;
            }
        } else if (nVersion == 2) {  // v2
            if (nFrequencyIndex == 0) {
                nFrequency = 22050;
            } else if (nFrequencyIndex == 1) {
                nFrequency = 24000;
            } else if (nFrequencyIndex == 2) {
                nFrequency = 16000;
            }
        } else if (nVersion == 3) {  // v1
            if (nFrequencyIndex == 0) {
                nFrequency = 44100;
            } else if (nFrequencyIndex == 1) {
                nFrequency = 48000;
            } else if (nFrequencyIndex == 2) {
                nFrequency = 32000;
            }
        }

        if (nFrequency) {
            if (nLayer == 3) {
                dTime = (double)384 / nFrequency;
            } else if ((nLayer == 2) || (nVersion == 3)) {
                dTime = (double)1152 / nFrequency;
            } else {
                dTime = (double)576 / nFrequency;
            }

            if (nLayer == 3) {
                nResult += std::floor(12000 * (double)nBitRate / nFrequency + nPadBit) * 4;
            } else {
                // 125 = 1000/8 = kilobits to bytes
                nResult += std::floor(125 * nBitRate * dTime + nPadBit);
            }
        }
    }

    return nResult;
}

QList<XBinary::FPART> XMP3::getFileParts(quint32 nFileParts, qint32 nLimit, PDSTRUCT *pPdStruct)
{
    QList<FPART> list;

    if ((nLimit < -1) || (nLimit == 0)) {
        return list;
    }

    const qint64 total = getSize();

    qint64 offset = 0;
    qint64 startAfterId3 = 0;

    // Optional ID3v2 header
    if (total >= 10) {
        if (read_ansiString(0, 3) == "ID3") {
            quint32 b0 = read_uint8(6);
            quint32 b1 = read_uint8(7);
            quint32 b2 = read_uint8(8);
            quint32 b3 = read_uint8(9);
            qint64 id3Size = 10 + ((b0 << 21) | (b1 << 14) | (b2 << 7) | b3);
            if (nFileParts & FILEPART_HEADER) {
                FPART h = {};
                h.filePart = FILEPART_HEADER;
                h.nFileOffset = 0;
                h.nFileSize = qMin(id3Size, total);
                h.nVirtualAddress = (XADDR)-1;
                h.sName = tr("Header");
                list.append(h);
                if ((nLimit != -1) && (list.count() >= nLimit)) return list;
            }
            startAfterId3 = qMin(id3Size, total);
            offset = startAfterId3;
        }
    }

    if (nFileParts & FILEPART_REGION) {
        // Walk frames conservatively
        while (offset + 4 <= total && XBinary::isPdStructNotCanceled(pPdStruct)) {
            quint32 header = read_uint32(offset, true);
            if ((header & 0xFFE00000) != 0xFFE00000) break;  // sync bits
            qint64 frameSize = decodeFrame(offset);
            if (frameSize <= 0) break;
            if (offset + frameSize > total) {
                frameSize = total - offset;
            }
            FPART f = {};
            f.filePart = FILEPART_REGION;
            f.nFileOffset = offset;
            f.nFileSize = frameSize;
            f.nVirtualAddress = (XADDR)-1;
            f.sName = QString("Frame");
            list.append(f);
            if ((nLimit != -1) && (list.count() >= nLimit)) return list;
            offset += frameSize;
        }
    }

    if (nFileParts & FILEPART_OVERLAY) {
        qint64 maxEnd = 0;
        for (qint32 i = 0; i < list.size(); ++i) {
            const FPART &p = list.at(i);
            maxEnd = qMax(maxEnd, p.nFileOffset + p.nFileSize);
        }
        if (maxEnd < total) {
            FPART ov = {};
            ov.filePart = FILEPART_OVERLAY;
            ov.nFileOffset = maxEnd;
            ov.nFileSize = total - maxEnd;
            ov.nVirtualAddress = (XADDR)-1;
            ov.sName = tr("Overlay");
            list.append(ov);
            if ((nLimit != -1) && (list.count() >= nLimit)) return list;
        }
    }

    return list;
}

QString XMP3::structIDToString(quint32 nID)
{
    return XBinary::XCONVERT_idToTransString(nID, _TABLE_XMP3_STRUCTID, sizeof(_TABLE_XMP3_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QString XMP3::structIDToFtString(quint32 nID)
{
    return XBinary::XCONVERT_idToFtString(nID, _TABLE_XMP3_STRUCTID, sizeof(_TABLE_XMP3_STRUCTID) / sizeof(XBinary::XCONVERT));
}

quint32 XMP3::ftStringToStructID(const QString &sFtString)
{
    return XCONVERT_ftStringToId(sFtString, _TABLE_XMP3_STRUCTID, sizeof(_TABLE_XMP3_STRUCTID) / sizeof(XBinary::XCONVERT));
}

QList<XBinary::MAPMODE> XMP3::getMapModesList()
{
    QList<MAPMODE> list;

    list.append(MAPMODE_UNKNOWN);

    return list;
}

QList<QString> XMP3::getSearchSignatures()
{
    QList<QString> listResult;

    listResult.append("'ID3'02");
    listResult.append("'ID3'03");
    listResult.append("'ID3'04");

    return listResult;
}

XBinary *XMP3::createInstance(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    Q_UNUSED(bIsImage)
    Q_UNUSED(nModuleAddress)

    return new XMP3(pDevice);
}

bool XMP3::handleInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XMP3> guardedThis(this);
    bool bResult = true;

    if (!isInternalInfoHandled()) {
        bResult = guardedThis->XBinary::handleInternalInfo(pPdStruct);
        if (!guardedThis || !bResult) return false;

        XBinary::INTERNAL_INFO *pInfo =
            static_cast<XBinary::INTERNAL_INFO *>(
                guardedThis->XBinary::getInternalInfo(pPdStruct));
        if (!guardedThis || !pInfo) return false;

        static_cast<XBinary::INTERNAL_INFO &>(
            guardedThis->m_internalInfo) = *pInfo;
        guardedThis->setIsInternalInfoHandled(true);
    }

    return guardedThis && bResult;
}

void *XMP3::getInternalInfo(PDSTRUCT *pPdStruct)
{
    QPointer<XMP3> guardedThis(this);
    const bool bHandled = guardedThis->handleInternalInfo(pPdStruct);
    if (!guardedThis || !bHandled) return nullptr;

    return &guardedThis->m_internalInfo;
}

void XMP3::setInternalInfo(void *pInternalInfo)
{
    if (pInternalInfo) {
        m_internalInfo = *static_cast<INTERNAL_INFO *>(pInternalInfo);
        XBinary::setInternalInfo(static_cast<XBinary::INTERNAL_INFO *>(&m_internalInfo));
        setIsInternalInfoHandled(true);
    } else {
        m_internalInfo = INTERNAL_INFO();
        XBinary::setInternalInfo(nullptr);
        setIsInternalInfoHandled(false);
    }
}
