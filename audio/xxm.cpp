/* Copyright (c) 2025-2026 hors<horsicq@gmail.com>
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
#include "xxm.h"

namespace {

XBinary::XCONVERT g_tableXxmStructId[] = {{XXM::STRUCTID_UNKNOWN, "Unknown", QObject::tr("Unknown")},
                                          {XXM::STRUCTID_HEADER, "HEADER", QString("Header")},
                                          {XXM::STRUCTID_PATTERN_HEADER, "PATTERN_HEADER", QString("Pattern Header")},
                                          {XXM::STRUCTID_PATTERN_DATA, "PATTERN_DATA", QString("Pattern Data")},
                                          {XXM::STRUCTID_INSTRUMENT_HEADER, "INSTRUMENT_HEADER", QString("Instrument Header")},
                                          {XXM::STRUCTID_INSTRUMENT_EXTRA_HEADER, "INSTRUMENT_EXTRA_HEADER", QString("Instrument Extra Header")},
                                          {XXM::STRUCTID_SAMPLE_HEADER, "SAMPLE_HEADER", QString("Sample Header")},
                                          {XXM::STRUCTID_SAMPLE_DATA, "SAMPLE_DATA", QString("Sample Data")}};

const qint64 kXmControlByteOffset = offsetof(XXM::HEADER, ctrl_byte_1a);
const qint64 kXmHeaderSizeFieldOffset = offsetof(XXM::HEADER, header_size);
const qint64 kXmHeaderMinimumSize = kXmHeaderSizeFieldOffset + sizeof(quint32);
const qint64 kXmHeaderDeclaredMinimumSize = sizeof(XXM::HEADER) - kXmHeaderSizeFieldOffset;
const qint64 kXmPatternHeaderSize = sizeof(XXM::PATTERN_HEADER);
const qint64 kXmInstrumentHeaderFixedSize = sizeof(XXM::INSTRUMENT_HEADER);
const qint64 kXmSampleHeaderSize = sizeof(XXM::SAMPLE_HEADER);
const qint32 kXmMaxPatterns = 256;
const qint32 kXmMaxInstruments = 256;
const qint32 kXmMaxSamplesPerInstrument = 256;

static_assert(sizeof(XXM::PATTERN_HEADER) == 9, "Unexpected XM pattern header size");
static_assert(sizeof(XXM::INSTRUMENT_HEADER) == 29, "Unexpected XM instrument header size");
static_assert(sizeof(XXM::INSTRUMENT_EXTRA_HEADER) == 214, "Unexpected XM instrument extra header size");
static_assert(sizeof(XXM::SAMPLE_HEADER) == 40, "Unexpected XM sample header size");

qint64 clampFileRangeSize(qint64 nOffset, qint64 nSize, qint64 nTotalSize)
{
    qint64 nResult = 0;

    if ((nOffset >= 0) && (nOffset < nTotalSize) && (nSize > 0)) {
        nResult = qMin(nSize, nTotalSize - nOffset);
    }

    return nResult;
}

qint64 addClamped(qint64 nOffset, qint64 nSize, qint64 nTotalSize)
{
    qint64 nResult = nOffset;

    if (nSize > 0) {
        if (nOffset > nTotalSize - nSize) {
            nResult = nTotalSize;
        } else {
            nResult = nOffset + nSize;
        }
    }

    return nResult;
}

void appendFilePart(QList<XBinary::FPART> *pList, quint32 nRequestedFileParts, XBinary::FILEPART filePart, qint64 nOffset, qint64 nSize, qint64 nTotalSize,
                    const QString &sName, qint32 nLimit)
{
    if (!(nRequestedFileParts & filePart)) {
        return;
    }

    if ((nLimit != -1) && (pList->count() >= nLimit)) {
        return;
    }

    nSize = clampFileRangeSize(nOffset, nSize, nTotalSize);

    if (nSize <= 0) {
        return;
    }

    XBinary::FPART record = {};
    record.filePart = filePart;
    record.nFileOffset = nOffset;
    record.nFileSize = nSize;
    record.nVirtualAddress = -1;
    record.sName = sName;

    pList->append(record);
}

}  // namespace

XXM::XXM(QIODevice *pDevice) : XBinary(pDevice)
{
}

XXM::~XXM()
{
}

bool XXM::isValid(PDSTRUCT *pPdStruct)
{
    bool bResult = false;

    if (getSize() >= kXmHeaderMinimumSize) {
        _MEMORY_MAP memoryMap = XBinary::getMemoryMap(MAPMODE_UNKNOWN, pPdStruct);

        if (compareSignature(&memoryMap, "'Extended Module: '", 0, pPdStruct) && (read_uint8(kXmControlByteOffset) == 0x1A)) {
            quint32 nHeaderSize = read_uint32(kXmHeaderSizeFieldOffset);

            if ((nHeaderSize >= kXmHeaderDeclaredMinimumSize) && (kXmHeaderSizeFieldOffset + nHeaderSize <= getSize())) {
                bResult = true;
            }
        }
    }

    return bResult;
}

XBinary::FT XXM::getFileType()
{
    return XBinary::FT_XM;
}

bool XXM::isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    XXM xxm(pDevice);

    return xxm.isValid(pPdStruct);
}

XBinary::MODE XXM::getMode()
{
    return XBinary::MODE_8;
}

QString XXM::getMIMEString()
{
    return "audio/x-xm";
}

qint32 XXM::getType()
{
    return 0;
}

QString XXM::typeIdToString(qint32 nType)
{
    Q_UNUSED(nType)
    return "XM";
}

XBinary::ENDIAN XXM::getEndian()
{
    // XM is little-endian
    return XBinary::ENDIAN_LITTLE;
}

QString XXM::getArch()
{
    return QString();
}

QString XXM::getFileFormatExt()
{
    return "xm";
}

QString XXM::getFileFormatExtsString()
{
    return "XM (*.xm)";
}

qint64 XXM::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    return _calculateRawSize(pPdStruct);
}

bool XXM::isSigned()
{
    return false;
}

XXM::OSNAME XXM::getOsName()
{
    return XBinary::OSNAME_UNKNOWN;
}

QString XXM::getOsVersion()
{
    return QString();
}

QString XXM::getVersion()
{
    QString sResult;

    if (getSize() < (qint64)offsetof(HEADER, header_size)) {
        return sResult;
    }

    HEADER header = _read_HEADER(0);

    if (header.version) {
        sResult = QString("%1.%2").arg(header.version >> 8).arg(header.version & 0xFF, 2, 10, QChar('0'));
    }

    return sResult;
}

bool XXM::isEncrypted()
{
    return false;
}

QList<XXM::MAPMODE> XXM::getMapModesList()
{
    QList<MAPMODE> listResult;
    listResult.append(MAPMODE_REGIONS);
    return listResult;
}

XXM::_MEMORY_MAP XXM::getMemoryMap(MAPMODE mapMode, PDSTRUCT *pPdStruct)
{
    Q_UNUSED(mapMode)
    return _getMemoryMap(FILEPART_HEADER | FILEPART_TABLE | FILEPART_DATA | FILEPART_OVERLAY, pPdStruct);
}

QList<XBinary::FPART> XXM::getFileParts(quint32 nFileParts, qint32 nLimit, PDSTRUCT *pPdStruct)
{
    QList<FPART> listResult;
    const qint64 nTotalSize = getSize();

    if (!isValid(pPdStruct)) {
        return listResult;
    }

    const HEADER header = _read_HEADER(0);
    qint64 nOffset = addClamped(kXmHeaderSizeFieldOffset, header.header_size, nTotalSize);

    if (nFileParts & FILEPART_HEADER) {
        appendFilePart(&listResult, nFileParts, FILEPART_HEADER, 0, nOffset, nTotalSize, tr("Header"), nLimit);
    }

    const qint32 nNumberOfPatterns = qMin<qint32>(header.num_patterns, kXmMaxPatterns);

    for (qint32 i = 0; (i < nNumberOfPatterns) && isPdStructNotCanceled(pPdStruct); i++) {
        if (nOffset + kXmPatternHeaderSize > nTotalSize) {
            break;
        }

        const PATTERN_HEADER patternHeader = _read_PATTERN_HEADER(nOffset);
        const qint64 nPatternHeaderSize = qMax<qint64>(kXmPatternHeaderSize, patternHeader.header_length);

        appendFilePart(&listResult, nFileParts, FILEPART_TABLE, nOffset, nPatternHeaderSize, nTotalSize, QString("Pattern %1 header").arg(i), nLimit);
        nOffset = addClamped(nOffset, nPatternHeaderSize, nTotalSize);

        appendFilePart(&listResult, nFileParts, FILEPART_DATA, nOffset, patternHeader.packed_data_size, nTotalSize, QString("Pattern %1 data").arg(i), nLimit);
        nOffset = addClamped(nOffset, patternHeader.packed_data_size, nTotalSize);

        if (nOffset >= nTotalSize) {
            break;
        }
    }

    const qint32 nNumberOfInstruments = qMin<qint32>(header.num_instruments, kXmMaxInstruments);

    for (qint32 i = 0; (i < nNumberOfInstruments) && isPdStructNotCanceled(pPdStruct); i++) {
        if (nOffset + kXmInstrumentHeaderFixedSize > nTotalSize) {
            break;
        }

        const INSTRUMENT_HEADER instrumentHeader = _read_INSTRUMENT_HEADER(nOffset);
        const qint64 nInstrumentHeaderSize = qMax<qint64>(kXmInstrumentHeaderFixedSize, instrumentHeader.instrument_header_size);
        const qint64 nInstrumentEndOffset = addClamped(nOffset, nInstrumentHeaderSize, nTotalSize);
        const qint64 nInstrumentFixedSize = qMin(kXmInstrumentHeaderFixedSize, nInstrumentHeaderSize);

        appendFilePart(&listResult, nFileParts, FILEPART_TABLE, nOffset, nInstrumentFixedSize, nTotalSize, QString("Instrument %1 header").arg(i), nLimit);

        qint64 nNextInstrumentOffset = nInstrumentEndOffset;

        if (instrumentHeader.num_samples > 0) {
            const qint64 nExtraOffset = nOffset + kXmInstrumentHeaderFixedSize;
            const qint64 nExtraSize = qMax<qint64>(0, nInstrumentHeaderSize - kXmInstrumentHeaderFixedSize);

            appendFilePart(&listResult, nFileParts, FILEPART_TABLE, nExtraOffset, nExtraSize, nTotalSize, QString("Instrument %1 extra").arg(i), nLimit);

            quint32 nSampleHeaderSize = kXmSampleHeaderSize;

            if (nExtraSize >= (qint64)sizeof(quint32)) {
                nSampleHeaderSize = qMax<quint32>((quint32)kXmSampleHeaderSize, read_uint32(nExtraOffset));
            }

            QList<quint32> listSampleLengths;
            qint64 nSampleHeaderOffset = nInstrumentEndOffset;
            const qint32 nNumberOfSamples = qMin<qint32>(instrumentHeader.num_samples, kXmMaxSamplesPerInstrument);

            for (qint32 j = 0; (j < nNumberOfSamples) && isPdStructNotCanceled(pPdStruct); j++) {
                if (nSampleHeaderOffset + kXmSampleHeaderSize > nTotalSize) {
                    break;
                }

                const SAMPLE_HEADER sampleHeader = _read_SAMPLE_HEADER(nSampleHeaderOffset);

                appendFilePart(&listResult, nFileParts, FILEPART_TABLE, nSampleHeaderOffset, nSampleHeaderSize, nTotalSize,
                               QString("Instrument %1 sample %2 header").arg(i).arg(j), nLimit);

                listSampleLengths.append(sampleHeader.sample_length);
                nSampleHeaderOffset = addClamped(nSampleHeaderOffset, nSampleHeaderSize, nTotalSize);
            }

            qint64 nSampleDataOffset = nSampleHeaderOffset;

            for (qint32 j = 0; (j < listSampleLengths.count()) && isPdStructNotCanceled(pPdStruct); j++) {
                const qint64 nSampleSize = listSampleLengths.at(j);

                appendFilePart(&listResult, nFileParts, FILEPART_DATA, nSampleDataOffset, nSampleSize, nTotalSize, QString("Instrument %1 sample %2 data").arg(i).arg(j),
                               nLimit);

                nSampleDataOffset = addClamped(nSampleDataOffset, nSampleSize, nTotalSize);
            }

            nNextInstrumentOffset = nSampleDataOffset;
        }

        if (nNextInstrumentOffset <= nOffset) {
            break;
        }

        nOffset = nNextInstrumentOffset;

        if (nOffset >= nTotalSize) {
            break;
        }
    }

    if (nFileParts & FILEPART_OVERLAY) {
        qint64 maxEnd = nOffset;
        for (qint32 i = 0; i < listResult.size(); ++i) {
            const FPART &p = listResult.at(i);
            maxEnd = qMax(maxEnd, p.nFileOffset + p.nFileSize);
        }

        if (maxEnd < nTotalSize) {
            appendFilePart(&listResult, nFileParts, FILEPART_OVERLAY, maxEnd, nTotalSize - maxEnd, nTotalSize, tr("Overlay"), nLimit);
        }
    }

    return listResult;
}

QString XXM::structIDToString(quint32 nID)
{
    return XBinary::XCONVERT_idToTransString(nID, g_tableXxmStructId, sizeof(g_tableXxmStructId) / sizeof(XBinary::XCONVERT));
}

QString XXM::structIDToFtString(quint32 nID)
{
    return XBinary::XCONVERT_idToFtString(nID, g_tableXxmStructId, sizeof(g_tableXxmStructId) / sizeof(XBinary::XCONVERT));
}

quint32 XXM::ftStringToStructID(const QString &sFtString)
{
    return XCONVERT_ftStringToId(sFtString, g_tableXxmStructId, sizeof(g_tableXxmStructId) / sizeof(XBinary::XCONVERT));
}

QList<XXM::DATA_HEADER> XXM::getDataHeaders(const DATA_HEADERS_OPTIONS &dataHeadersOptions, PDSTRUCT *pPdStruct)
{
    QList<DATA_HEADER> listResult;

    if (dataHeadersOptions.nID == STRUCTID_UNKNOWN) {
        DATA_HEADERS_OPTIONS _dataHeadersOptions = dataHeadersOptions;
        _dataHeadersOptions.bChildren = true;
        _dataHeadersOptions.dsID_parent = _addDefaultHeaders(&listResult, pPdStruct);
        _dataHeadersOptions.dhMode = XBinary::DHMODE_HEADER;
        _dataHeadersOptions.fileType = dataHeadersOptions.pMemoryMap->fileType;

        _dataHeadersOptions.nID = STRUCTID_HEADER;
        _dataHeadersOptions.nLocation = 0;
        _dataHeadersOptions.locType = XBinary::LT_OFFSET;

        listResult.append(getDataHeaders(_dataHeadersOptions, pPdStruct));
    } else {
        qint64 nStartOffset = locationToOffset(dataHeadersOptions.pMemoryMap, dataHeadersOptions.locType, dataHeadersOptions.nLocation);

        if (nStartOffset != -1) {
            DATA_HEADER dataHeader = _initDataHeader(dataHeadersOptions, XXM::structIDToString(dataHeadersOptions.nID));
            bool bAppend = true;

            if (dataHeadersOptions.nID == STRUCTID_HEADER) {
                dataHeader.nSize = sizeof(HEADER);
                dataHeader.listRecords.append(getDataRecord(offsetof(HEADER, id_text), 17, "id_text", VT_CHAR_ARRAY, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(
                    getDataRecord(offsetof(HEADER, module_name), 20, "module_name", VT_CHAR_ARRAY, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(
                    getDataRecord(offsetof(HEADER, ctrl_byte_1a), 1, "ctrl_byte_1a", VT_BYTE, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(
                    getDataRecord(offsetof(HEADER, tracker_name), 20, "tracker_name", VT_CHAR_ARRAY, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(getDataRecord(offsetof(HEADER, version), 2, "version", VT_WORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(
                    getDataRecord(offsetof(HEADER, header_size), 4, "header_size", VT_DWORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(
                    getDataRecord(offsetof(HEADER, song_length), 2, "song_length", VT_WORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(
                    getDataRecord(offsetof(HEADER, song_restart), 2, "song_restart", VT_WORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(getDataRecord(offsetof(HEADER, channels), 2, "channels", VT_WORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(
                    getDataRecord(offsetof(HEADER, num_patterns), 2, "num_patterns", VT_WORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(
                    getDataRecord(offsetof(HEADER, num_instruments), 2, "num_instruments", VT_WORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(getDataRecord(offsetof(HEADER, flags), 2, "flags", VT_WORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(
                    getDataRecord(offsetof(HEADER, default_tempo), 2, "default_tempo", VT_WORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(
                    getDataRecord(offsetof(HEADER, default_bpm), 2, "default_bpm", VT_WORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(
                    getDataRecord(offsetof(HEADER, pattern_order), 256, "pattern_order", VT_BYTE_ARRAY, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
            } else if (dataHeadersOptions.nID == STRUCTID_PATTERN_HEADER) {
                dataHeader.nSize = sizeof(PATTERN_HEADER);
                dataHeader.listRecords.append(
                    getDataRecord(offsetof(PATTERN_HEADER, header_length), 4, "header_length", VT_DWORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(
                    getDataRecord(offsetof(PATTERN_HEADER, packing_type), 1, "packing_type", VT_BYTE, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(
                    getDataRecord(offsetof(PATTERN_HEADER, num_rows), 2, "num_rows", VT_WORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(
                    getDataRecord(offsetof(PATTERN_HEADER, packed_data_size), 2, "packed_data_size", VT_WORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
            } else if (dataHeadersOptions.nID == STRUCTID_INSTRUMENT_HEADER) {
                dataHeader.nSize = sizeof(INSTRUMENT_HEADER);
                dataHeader.listRecords.append(getDataRecord(offsetof(INSTRUMENT_HEADER, instrument_header_size), 4, "instrument_header_size", VT_DWORD, DRF_UNKNOWN,
                                                            dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(getDataRecord(offsetof(INSTRUMENT_HEADER, instrument_name), 22, "instrument_name", VT_CHAR_ARRAY, DRF_UNKNOWN,
                                                            dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(
                    getDataRecord(offsetof(INSTRUMENT_HEADER, instrument_type), 1, "instrument_type", VT_BYTE, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(
                    getDataRecord(offsetof(INSTRUMENT_HEADER, num_samples), 2, "num_samples", VT_WORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
            } else if (dataHeadersOptions.nID == STRUCTID_INSTRUMENT_EXTRA_HEADER) {
                dataHeader.nSize = sizeof(INSTRUMENT_EXTRA_HEADER);
                dataHeader.listRecords.append(getDataRecord(offsetof(INSTRUMENT_EXTRA_HEADER, sample_header_size), 4, "sample_header_size", VT_DWORD, DRF_UNKNOWN,
                                                            dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(getDataRecord(offsetof(INSTRUMENT_EXTRA_HEADER, sample_number_for_notes), 96, "sample_number_for_notes", VT_BYTE_ARRAY,
                                                            DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(getDataRecord(offsetof(INSTRUMENT_EXTRA_HEADER, volume_envelope_points), 48, "volume_envelope_points", VT_BYTE_ARRAY,
                                                            DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(getDataRecord(offsetof(INSTRUMENT_EXTRA_HEADER, panning_envelope_points), 48, "panning_envelope_points", VT_BYTE_ARRAY,
                                                            DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(getDataRecord(offsetof(INSTRUMENT_EXTRA_HEADER, num_volume_points), 1, "num_volume_points", VT_BYTE, DRF_UNKNOWN,
                                                            dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(getDataRecord(offsetof(INSTRUMENT_EXTRA_HEADER, num_panning_points), 1, "num_panning_points", VT_BYTE, DRF_UNKNOWN,
                                                            dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(getDataRecord(offsetof(INSTRUMENT_EXTRA_HEADER, volume_sustain_point), 1, "volume_sustain_point", VT_BYTE, DRF_UNKNOWN,
                                                            dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(getDataRecord(offsetof(INSTRUMENT_EXTRA_HEADER, volume_loop_start_point), 1, "volume_loop_start_point", VT_BYTE,
                                                            DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(getDataRecord(offsetof(INSTRUMENT_EXTRA_HEADER, volume_loop_end_point), 1, "volume_loop_end_point", VT_BYTE, DRF_UNKNOWN,
                                                            dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(getDataRecord(offsetof(INSTRUMENT_EXTRA_HEADER, panning_sustain_point), 1, "panning_sustain_point", VT_BYTE, DRF_UNKNOWN,
                                                            dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(getDataRecord(offsetof(INSTRUMENT_EXTRA_HEADER, panning_loop_start_point), 1, "panning_loop_start_point", VT_BYTE,
                                                            DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(getDataRecord(offsetof(INSTRUMENT_EXTRA_HEADER, panning_loop_end_point), 1, "panning_loop_end_point", VT_BYTE, DRF_UNKNOWN,
                                                            dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(
                    getDataRecord(offsetof(INSTRUMENT_EXTRA_HEADER, volume_type), 1, "volume_type", VT_BYTE, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(
                    getDataRecord(offsetof(INSTRUMENT_EXTRA_HEADER, panning_type), 1, "panning_type", VT_BYTE, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(
                    getDataRecord(offsetof(INSTRUMENT_EXTRA_HEADER, vibrato_type), 1, "vibrato_type", VT_BYTE, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(
                    getDataRecord(offsetof(INSTRUMENT_EXTRA_HEADER, vibrato_sweep), 1, "vibrato_sweep", VT_BYTE, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(
                    getDataRecord(offsetof(INSTRUMENT_EXTRA_HEADER, vibrato_depth), 1, "vibrato_depth", VT_BYTE, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(
                    getDataRecord(offsetof(INSTRUMENT_EXTRA_HEADER, vibrato_rate), 1, "vibrato_rate", VT_BYTE, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(
                    getDataRecord(offsetof(INSTRUMENT_EXTRA_HEADER, volume_fadeout), 2, "volume_fadeout", VT_WORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(
                    getDataRecord(offsetof(INSTRUMENT_EXTRA_HEADER, reserved), 2, "reserved", VT_WORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
            } else if (dataHeadersOptions.nID == STRUCTID_SAMPLE_HEADER) {
                dataHeader.nSize = sizeof(SAMPLE_HEADER);
                dataHeader.listRecords.append(
                    getDataRecord(offsetof(SAMPLE_HEADER, sample_length), 4, "sample_length", VT_DWORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(
                    getDataRecord(offsetof(SAMPLE_HEADER, loop_start), 4, "loop_start", VT_DWORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(
                    getDataRecord(offsetof(SAMPLE_HEADER, loop_length), 4, "loop_length", VT_DWORD, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(getDataRecord(offsetof(SAMPLE_HEADER, volume), 1, "volume", VT_BYTE, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(
                    getDataRecord(offsetof(SAMPLE_HEADER, finetune), 1, "finetune", VT_BYTE, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(getDataRecord(offsetof(SAMPLE_HEADER, type), 1, "type", VT_BYTE, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(getDataRecord(offsetof(SAMPLE_HEADER, panning), 1, "panning", VT_BYTE, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(
                    getDataRecord(offsetof(SAMPLE_HEADER, relative_note_number), 1, "relative_note_number", VT_BYTE, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(
                    getDataRecord(offsetof(SAMPLE_HEADER, reserved), 1, "reserved", VT_BYTE, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
                dataHeader.listRecords.append(
                    getDataRecord(offsetof(SAMPLE_HEADER, sample_name), 22, "sample_name", VT_CHAR_ARRAY, DRF_UNKNOWN, dataHeadersOptions.pMemoryMap->endian));
            } else {
                bAppend = false;
            }

            if (bAppend) {
                listResult.append(dataHeader);
            }
        }
    }

    return listResult;
}

XXM::HEADER XXM::_read_HEADER(qint64 nOffset)
{
    HEADER header = {};
    read_array(nOffset + offsetof(HEADER, id_text), header.id_text, sizeof(header.id_text));
    read_array(nOffset + offsetof(HEADER, module_name), header.module_name, sizeof(header.module_name));
    header.ctrl_byte_1a = read_uint8(nOffset + offsetof(HEADER, ctrl_byte_1a));
    read_array(nOffset + offsetof(HEADER, tracker_name), header.tracker_name, sizeof(header.tracker_name));
    header.version = read_uint16(nOffset + offsetof(HEADER, version));
    header.header_size = read_uint32(nOffset + offsetof(HEADER, header_size));
    header.song_length = read_uint16(nOffset + offsetof(HEADER, song_length));
    header.song_restart = read_uint16(nOffset + offsetof(HEADER, song_restart));
    header.channels = read_uint16(nOffset + offsetof(HEADER, channels));
    header.num_patterns = read_uint16(nOffset + offsetof(HEADER, num_patterns));
    header.num_instruments = read_uint16(nOffset + offsetof(HEADER, num_instruments));
    header.flags = read_uint16(nOffset + offsetof(HEADER, flags));
    header.default_tempo = read_uint16(nOffset + offsetof(HEADER, default_tempo));
    header.default_bpm = read_uint16(nOffset + offsetof(HEADER, default_bpm));
    read_array(nOffset + offsetof(HEADER, pattern_order), reinterpret_cast<char *>(header.pattern_order), sizeof(header.pattern_order));

    return header;
}

XXM::PATTERN_HEADER XXM::_read_PATTERN_HEADER(qint64 nOffset)
{
    PATTERN_HEADER patternHeader = {};
    patternHeader.header_length = read_uint32(nOffset + offsetof(PATTERN_HEADER, header_length));
    patternHeader.packing_type = read_uint8(nOffset + offsetof(PATTERN_HEADER, packing_type));
    patternHeader.num_rows = read_uint16(nOffset + offsetof(PATTERN_HEADER, num_rows));
    patternHeader.packed_data_size = read_uint16(nOffset + offsetof(PATTERN_HEADER, packed_data_size));
    return patternHeader;
}

XXM::INSTRUMENT_HEADER XXM::_read_INSTRUMENT_HEADER(qint64 nOffset)
{
    INSTRUMENT_HEADER hdr = {};
    hdr.instrument_header_size = read_uint32(nOffset + offsetof(INSTRUMENT_HEADER, instrument_header_size));
    read_array(nOffset + offsetof(INSTRUMENT_HEADER, instrument_name), hdr.instrument_name, sizeof(hdr.instrument_name));
    hdr.instrument_type = read_uint8(nOffset + offsetof(INSTRUMENT_HEADER, instrument_type));
    hdr.num_samples = read_uint16(nOffset + offsetof(INSTRUMENT_HEADER, num_samples));
    return hdr;
}

XXM::INSTRUMENT_EXTRA_HEADER XXM::_read_INSTRUMENT_EXTRA_HEADER(qint64 nOffset)
{
    INSTRUMENT_EXTRA_HEADER hdr = {};
    hdr.sample_header_size = read_uint32(nOffset + offsetof(INSTRUMENT_EXTRA_HEADER, sample_header_size));
    read_array(nOffset + offsetof(INSTRUMENT_EXTRA_HEADER, sample_number_for_notes), reinterpret_cast<char *>(hdr.sample_number_for_notes),
               sizeof(hdr.sample_number_for_notes));
    read_array(nOffset + offsetof(INSTRUMENT_EXTRA_HEADER, volume_envelope_points), reinterpret_cast<char *>(hdr.volume_envelope_points),
               sizeof(hdr.volume_envelope_points));
    read_array(nOffset + offsetof(INSTRUMENT_EXTRA_HEADER, panning_envelope_points), reinterpret_cast<char *>(hdr.panning_envelope_points),
               sizeof(hdr.panning_envelope_points));

    hdr.num_volume_points = read_uint8(nOffset + offsetof(INSTRUMENT_EXTRA_HEADER, num_volume_points));
    hdr.num_panning_points = read_uint8(nOffset + offsetof(INSTRUMENT_EXTRA_HEADER, num_panning_points));
    hdr.volume_sustain_point = read_uint8(nOffset + offsetof(INSTRUMENT_EXTRA_HEADER, volume_sustain_point));
    hdr.volume_loop_start_point = read_uint8(nOffset + offsetof(INSTRUMENT_EXTRA_HEADER, volume_loop_start_point));
    hdr.volume_loop_end_point = read_uint8(nOffset + offsetof(INSTRUMENT_EXTRA_HEADER, volume_loop_end_point));
    hdr.panning_sustain_point = read_uint8(nOffset + offsetof(INSTRUMENT_EXTRA_HEADER, panning_sustain_point));
    hdr.panning_loop_start_point = read_uint8(nOffset + offsetof(INSTRUMENT_EXTRA_HEADER, panning_loop_start_point));
    hdr.panning_loop_end_point = read_uint8(nOffset + offsetof(INSTRUMENT_EXTRA_HEADER, panning_loop_end_point));
    hdr.volume_type = read_uint8(nOffset + offsetof(INSTRUMENT_EXTRA_HEADER, volume_type));
    hdr.panning_type = read_uint8(nOffset + offsetof(INSTRUMENT_EXTRA_HEADER, panning_type));
    hdr.vibrato_type = read_uint8(nOffset + offsetof(INSTRUMENT_EXTRA_HEADER, vibrato_type));
    hdr.vibrato_sweep = read_uint8(nOffset + offsetof(INSTRUMENT_EXTRA_HEADER, vibrato_sweep));
    hdr.vibrato_depth = read_uint8(nOffset + offsetof(INSTRUMENT_EXTRA_HEADER, vibrato_depth));
    hdr.vibrato_rate = read_uint8(nOffset + offsetof(INSTRUMENT_EXTRA_HEADER, vibrato_rate));
    hdr.volume_fadeout = read_uint16(nOffset + offsetof(INSTRUMENT_EXTRA_HEADER, volume_fadeout));
    hdr.reserved = read_uint16(nOffset + offsetof(INSTRUMENT_EXTRA_HEADER, reserved));

    return hdr;
}

QList<QString> XXM::getSearchSignatures()
{
    QList<QString> listResult;

    listResult.append("'Extended Module: '");

    return listResult;
}

XBinary *XXM::createInstance(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    Q_UNUSED(bIsImage)
    Q_UNUSED(nModuleAddress)

    return new XXM(pDevice);
}

XXM::SAMPLE_HEADER XXM::_read_SAMPLE_HEADER(qint64 nOffset)
{
    SAMPLE_HEADER hdr = {};
    hdr.sample_length = read_uint32(nOffset + offsetof(SAMPLE_HEADER, sample_length));
    hdr.loop_start = read_uint32(nOffset + offsetof(SAMPLE_HEADER, loop_start));
    hdr.loop_length = read_uint32(nOffset + offsetof(SAMPLE_HEADER, loop_length));
    hdr.volume = read_uint8(nOffset + offsetof(SAMPLE_HEADER, volume));
    hdr.finetune = read_int8(nOffset + offsetof(SAMPLE_HEADER, finetune));
    hdr.type = read_uint8(nOffset + offsetof(SAMPLE_HEADER, type));
    hdr.panning = read_uint8(nOffset + offsetof(SAMPLE_HEADER, panning));
    hdr.relative_note_number = read_int8(nOffset + offsetof(SAMPLE_HEADER, relative_note_number));
    hdr.reserved = read_uint8(nOffset + offsetof(SAMPLE_HEADER, reserved));
    read_array(nOffset + offsetof(SAMPLE_HEADER, sample_name), hdr.sample_name, sizeof(hdr.sample_name));

    return hdr;
}
