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
#include "xjfif.h"

namespace {
const qint64 JFIF_HEADER_SIZE = 14;
const qint64 JFIF_THUMBNAIL_OFFSET = JFIF_HEADER_SIZE;
const qint32 JFIF_IDENTIFIER_SIZE = 5;

const qint64 JFIF_VERSION_MAJOR_OFFSET = 5;
const qint64 JFIF_VERSION_MINOR_OFFSET = 6;
const qint64 JFIF_DENSITY_UNIT_OFFSET = 7;
const qint64 JFIF_X_DENSITY_OFFSET = 8;
const qint64 JFIF_Y_DENSITY_OFFSET = 10;
const qint64 JFIF_THUMBNAIL_WIDTH_OFFSET = 12;
const qint64 JFIF_THUMBNAIL_HEIGHT_OFFSET = 13;
}  // namespace

XJFIF::XJFIF(QIODevice *pDevice) : XBinary(pDevice)
{
}

XJFIF::~XJFIF()
{
}

bool XJFIF::isValid(PDSTRUCT *pPdStruct)
{
    if ((getSize() < JFIF_HEADER_SIZE) || !XBinary::isPdStructNotCanceled(pPdStruct) ||
        (read_array(0, JFIF_IDENTIFIER_SIZE) != QByteArray("JFIF\0", JFIF_IDENTIFIER_SIZE))) {
        return false;
    }

    const HEADER header = getHeader();

    if (header.nDensityUnit > DENSITY_UNIT_PIXELS_PER_CENTIMETER) {
        return false;
    }

    const qint64 nThumbnailPixels = (qint64)header.nThumbnailWidth * header.nThumbnailHeight;
    const qint64 nThumbnailSize = nThumbnailPixels * 3;

    return (nThumbnailSize <= getSize() - JFIF_THUMBNAIL_OFFSET) && XBinary::isPdStructNotCanceled(pPdStruct);
}

bool XJFIF::isValid(QIODevice *pDevice, PDSTRUCT *pPdStruct)
{
    XJFIF jfif(pDevice);

    return jfif.isValid(pPdStruct);
}

qint64 XJFIF::getFileFormatSize(PDSTRUCT *pPdStruct)
{
    if (!isValid(pPdStruct)) {
        return 0;
    }

    const HEADER header = getHeader();

    return JFIF_THUMBNAIL_OFFSET + (qint64)header.nThumbnailWidth * header.nThumbnailHeight * 3;
}

QString XJFIF::getVersion()
{
    QString sResult;

    if (getSize() >= JFIF_HEADER_SIZE) {
        const HEADER header = getHeader();
        sResult = QString("%1.%2").arg(QString::number(header.nVersionMajor)).arg(QString::number(header.nVersionMinor));
    }

    return sResult;
}

XJFIF::HEADER XJFIF::getHeader()
{
    HEADER result = {};

    if (getSize() < JFIF_HEADER_SIZE) {
        return result;
    }

    result.nVersionMajor = read_uint8(JFIF_VERSION_MAJOR_OFFSET);
    result.nVersionMinor = read_uint8(JFIF_VERSION_MINOR_OFFSET);
    result.nDensityUnit = read_uint8(JFIF_DENSITY_UNIT_OFFSET);
    result.nXDensity = read_uint16(JFIF_X_DENSITY_OFFSET, true);
    result.nYDensity = read_uint16(JFIF_Y_DENSITY_OFFSET, true);
    result.nThumbnailWidth = read_uint8(JFIF_THUMBNAIL_WIDTH_OFFSET);
    result.nThumbnailHeight = read_uint8(JFIF_THUMBNAIL_HEIGHT_OFFSET);

    return result;
}

XBinary::OFFSETSIZE XJFIF::getThumbnail()
{
    OFFSETSIZE result = {};
    const HEADER header = getHeader();
    const qint64 nThumbnailSize = (qint64)header.nThumbnailWidth * header.nThumbnailHeight * 3;

    if ((nThumbnailSize > 0) && (nThumbnailSize <= getSize() - JFIF_THUMBNAIL_OFFSET)) {
        result.nOffset = JFIF_THUMBNAIL_OFFSET;
        result.nSize = nThumbnailSize;
    }

    return result;
}

QString XJFIF::getDensityUnitString()
{
    QString sResult;
    const HEADER header = getHeader();

    if (header.nDensityUnit == DENSITY_UNIT_NONE) {
        sResult = QString("Aspect ratio");
    } else if (header.nDensityUnit == DENSITY_UNIT_PIXELS_PER_INCH) {
        sResult = QString("Pixels per inch");
    } else if (header.nDensityUnit == DENSITY_UNIT_PIXELS_PER_CENTIMETER) {
        sResult = QString("Pixels per centimeter");
    }

    return sResult;
}

QVector<XBinary::XMETADATA_STRUCT> XJFIF::getMetadataStructs()
{
    QVector<XMETADATA_STRUCT> listResult;

    if (!isValid()) {
        return listResult;
    }

    const HEADER header = getHeader();

    auto appendMetadata = [this, &listResult](qint64 nOffset, qint64 nSize, const QString &sName, const QVariant &varValue) {
        XMETADATA_STRUCT record = {};
        record.nOffset = nOffset;
        record.nSize = nSize;
        record.nAddress = offsetToAddress(nOffset);
        record.id = XMETADATA_ID_UNKNOWN;
        record.sName = sName;
        record.varValue = varValue;
        listResult.append(record);
    };

    appendMetadata(JFIF_VERSION_MAJOR_OFFSET, 2, QString("JFIF version"), getVersion());
    appendMetadata(JFIF_DENSITY_UNIT_OFFSET, 1, QString("Density unit"), getDensityUnitString());
    appendMetadata(JFIF_X_DENSITY_OFFSET, 2, QString("X density"), header.nXDensity);
    appendMetadata(JFIF_Y_DENSITY_OFFSET, 2, QString("Y density"), header.nYDensity);

    if (header.nThumbnailWidth || header.nThumbnailHeight) {
        appendMetadata(JFIF_THUMBNAIL_WIDTH_OFFSET, 1, QString("Thumbnail width"), header.nThumbnailWidth);
        appendMetadata(JFIF_THUMBNAIL_HEIGHT_OFFSET, 1, QString("Thumbnail height"), header.nThumbnailHeight);
    }

    return listResult;
}
