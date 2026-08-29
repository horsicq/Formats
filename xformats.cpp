/* Copyright (c) 2020-2026 hors<horsicq@gmail.com>
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
#include "xformats.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QTemporaryFile>

#include <algorithm>
#include <limits>
#include <memory>

namespace {
class DevicePositionGuard {
public:
    explicit DevicePositionGuard(QIODevice *pDevice) : m_pDevice(pDevice), m_nPosition(-1)
    {
        if (m_pDevice && !m_pDevice->isSequential()) {
            m_nPosition = m_pDevice->pos();
        }
    }

    ~DevicePositionGuard()
    {
        if (m_pDevice && (m_nPosition >= 0) && m_pDevice->isOpen()) {
            m_pDevice->seek(m_nPosition);
        }
    }

private:
    QIODevice *m_pDevice;
    qint64 m_nPosition;
};

bool isReadableSeekableDevice(QIODevice *pDevice)
{
    return pDevice && pDevice->isOpen() && pDevice->isReadable() && !pDevice->isSequential() && (pDevice->size() >= 0);
}

bool isBrotliDetectionCandidate(QIODevice *pDevice)
{
    if (!isReadableSeekableDevice(pDevice)) return false;

    const QString sSuffix = XBinary::getDeviceFileSuffix(pDevice).toLower();
    if ((sSuffix == QLatin1String("br")) || (sSuffix == QLatin1String("brotli")) || (sSuffix == QLatin1String("tbr"))) {
        return true;
    }

    // Ordinary RFC 7932 Brotli has no fixed magic, so extension-neutral
    // probing would create false positives.  The fork's MT envelope is
    // unambiguous and can safely be recognized by its complete header tuple.
    DevicePositionGuard positionGuard(pDevice);
    if (!pDevice->seek(0)) return false;
    const QByteArray baHeader = pDevice->read(16);
    return (baHeader.size() == 16) && baHeader.startsWith(QByteArray::fromHex("502A4D1808000000")) && (baHeader.mid(12, 2) == QByteArray("BR", 2));
}

#ifdef USE_ARCHIVE
bool isValidBrotliArchive(QIODevice *pDevice, XBinary::PDSTRUCT *pPdStruct)
{
    XBrotli archive(pDevice);
    return archive.isValid(pPdStruct);
}
#endif

bool isValidDeviceRange(QIODevice *pDevice, qint64 nOffset, qint64 nSize)
{
    if (!isReadableSeekableDevice(pDevice) || (nOffset < 0) || (nSize < -1)) {
        return false;
    }

    const qint64 nDeviceSize = pDevice->size();
    return (nOffset <= nDeviceSize) && ((nSize == -1) || (nSize <= (nDeviceSize - nOffset)));
}

bool writeAllDevice(QIODevice *pDevice, const char *pData, qint64 nSize)
{
    if (!pDevice || !pDevice->isWritable() || (nSize < 0) || ((nSize > 0) && !pData)) {
        return false;
    }

    qint64 nWritten = 0;
    while (nWritten < nSize) {
        const qint64 nCurrent = pDevice->write(pData + nWritten, nSize - nWritten);
        if ((nCurrent <= 0) || (nCurrent > (nSize - nWritten))) {
            return false;
        }
        nWritten += nCurrent;
    }

    return true;
}

QString canonicalPath(const QString &sPath)
{
    const QFileInfo fileInfo(sPath);
    QString sResult = fileInfo.canonicalFilePath();
    if (sResult.isEmpty()) {
        sResult = fileInfo.absoluteFilePath();
    }
    return QDir::fromNativeSeparators(QDir::cleanPath(sResult));
}

bool isPathBelowRoot(const QString &sCanonicalRoot, const QString &sCandidate)
{
    if (sCanonicalRoot.isEmpty() || sCandidate.isEmpty()) {
        return false;
    }

#ifdef Q_OS_WIN
    const Qt::CaseSensitivity caseSensitivity = Qt::CaseInsensitive;
#else
    const Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive;
#endif

    const QString sRoot = QDir::fromNativeSeparators(QDir::cleanPath(sCanonicalRoot));
    const QString sPath = QDir::fromNativeSeparators(QDir::cleanPath(sCandidate));
    const QString sPrefix = sRoot.endsWith(QLatin1Char('/')) ? sRoot : (sRoot + QLatin1Char('/'));
    return (QString::compare(sPath, sRoot, caseSensitivity) != 0) && sPath.startsWith(sPrefix, caseSensitivity);
}

bool ensureContainedDirectory(const QString &sCanonicalRoot, const QString &sRelativeDirectory, XBinary::UNPACK_FOLDER_TRANSACTION *pTransaction,
                              QString *psCanonicalDirectory)
{
    if (!pTransaction || sCanonicalRoot.isEmpty() || !QFileInfo(sCanonicalRoot).isDir()) {
        return false;
    }

    QString sCurrentDirectory = sCanonicalRoot;
    const QString sCleanRelativeDirectory = QDir::fromNativeSeparators(QDir::cleanPath(sRelativeDirectory));

    if (!sCleanRelativeDirectory.isEmpty() && (sCleanRelativeDirectory != QLatin1String("."))) {
        if (QDir::isAbsolutePath(sCleanRelativeDirectory) || (sCleanRelativeDirectory == QLatin1String("..")) ||
            sCleanRelativeDirectory.startsWith(QLatin1String("../"))) {
            return false;
        }

        const QStringList listComponents = sCleanRelativeDirectory.split(QLatin1Char('/'), Qt::SkipEmptyParts);
        for (const QString &sComponent : listComponents) {
            if ((sComponent == QLatin1String(".")) || (sComponent == QLatin1String(".."))) {
                return false;
            }

            const QString sCandidate = QDir(sCurrentDirectory).absoluteFilePath(sComponent);
            QFileInfo fileInfo(sCandidate);

            // A dangling link must never be treated as a creatable directory.
            if (fileInfo.isSymLink() && !fileInfo.exists()) {
                return false;
            }

            if (!pTransaction->ensureDirectory(sCandidate)) {
                return false;
            }
            fileInfo.setFile(sCandidate);

            const QString sCanonicalCandidate = canonicalPath(sCandidate);
            if (!fileInfo.isDir() || !isPathBelowRoot(sCanonicalRoot, sCanonicalCandidate)) {
                return false;
            }

            sCurrentDirectory = sCanonicalCandidate;
        }
    }

    if (psCanonicalDirectory) {
        *psCanonicalDirectory = sCurrentDirectory;
    }

    return true;
}

bool parseCachedFileTypes(const QString &sValue, QSet<XBinary::FT> *pResult)
{
    if (!pResult) {
        return false;
    }

    pResult->clear();
    if (sValue.trimmed().isEmpty()) {
        return false;
    }

    const QStringList listTokens = sValue.split(QLatin1Char('|'), Qt::KeepEmptyParts);
    for (const QString &sRawToken : listTokens) {
        const QString sToken = sRawToken.trimmed();
        if (sToken.isEmpty()) {
            pResult->clear();
            return false;
        }

        XBinary::FT fileType = XBinary::ftStringToFileTypeId(sToken);
        if (fileType == XBinary::FT_UNKNOWN) fileType = XBinary::ftStringToFileTypeId(sToken.toUpper());
        if (fileType == XBinary::FT_UNKNOWN) fileType = XBinary::ftStringToFileTypeId(sToken.toLower());

        if (fileType == XBinary::FT_UNKNOWN) {
            pResult->clear();
            return false;
        }

        pResult->insert(fileType);
    }

    return !pResult->isEmpty();
}

QString normalizeStructToken(const QString &sValue)
{
    QString sResult = sValue.trimmed().toUpper();

    QString sNormalized;
    const qint32 nLength = sResult.size();

    for (qint32 i = 0; i < nLength; i++) {
        QChar ch = sResult.at(i);
        if (!ch.isSpace() && (ch != QLatin1Char('-'))) {
            sNormalized.append(ch);
        }
    }

    return sNormalized;
}

QStringList splitAndTrim(const QString &sValue, const QString &sDelimiter)
{
    QStringList listResult;

    const QStringList listParts = sValue.split(sDelimiter, Qt::SkipEmptyParts);

    for (const QString &sPart : qAsConst(listParts)) {
        const QString sTrimmed = sPart.trimmed();
        if (!sTrimmed.isEmpty()) {
            listResult.append(sTrimmed);
        }
    }

    return listResult;
}

QString getCurrentStructSegment(const QString &sStruct)
{
    QString sResult = sStruct.trimmed();

    qint32 nHashIdx = sResult.lastIndexOf('#');
    if (nHashIdx != -1) {
        sResult = sResult.mid(nHashIdx + 1);
    }

    qint32 nParamIdx = sResult.indexOf('?');
    if (nParamIdx != -1) {
        sResult = sResult.left(nParamIdx);
    }

    QStringList listParts = splitAndTrim(sResult, "::");

    if (listParts.size() >= 4) {
        sResult = listParts.at(2);
    } else if (listParts.size() >= 2) {
        sResult = listParts.last();
    }

    return sResult;
}

XBinary::XFTYPE getXFTypeFromToken(const QString &sValue)
{
    XBinary::XFTYPE result = XBinary::XFTYPE_UNKNOWN;

    const QString sType = normalizeStructToken(sValue);

    if (sType == "HEADER") {
        result = XBinary::XFTYPE_HEADER;
    } else if (sType == "TABLE") {
        result = XBinary::XFTYPE_TABLE;
    } else if (sType == "COMMAND") {
        result = XBinary::XFTYPE_COMMAND;
    }

    return result;
}

XBinary::XFTYPE getXFTypeFromStructString(const QString &sStruct)
{
    XBinary::XFTYPE result = XBinary::XFTYPE_UNKNOWN;

    QString sFiltered = sStruct;
    qint32 nHashIdx = sFiltered.lastIndexOf('#');
    if (nHashIdx != -1) {
        sFiltered = sFiltered.mid(nHashIdx + 1);
    }

    const QString sInputNoParams = sFiltered;
    qint32 nParamIdx = sInputNoParams.indexOf('?');
    if (nParamIdx != -1) {
        const QString sParams = sInputNoParams.mid(nParamIdx + 1);
        sFiltered = sInputNoParams.left(nParamIdx);

        for (const QString &sParam : splitAndTrim(sParams, "&")) {
            qint32 nEqIdx = sParam.indexOf('=');
            if (nEqIdx != -1) {
                const QString sKey = normalizeStructToken(sParam.left(nEqIdx));
                const QString sValue = normalizeStructToken(sParam.mid(nEqIdx + 1));

                if (sKey == "TYPE") {
                    result = getXFTypeFromToken(sValue);

                    break;
                }
            }
        }
    }

    if (result == XBinary::XFTYPE_UNKNOWN) {
        QStringList listParts = splitAndTrim(sFiltered, "::");

        if (listParts.size() >= 4) {
            const QString sType = normalizeStructToken(listParts.at(3));
            result = getXFTypeFromToken(sType);
        }
    }

    return result;
}

qint64 parseXFOffset(const QString &sValue, bool *pbOk = nullptr)
{
    QString sTrimmed = sValue.trimmed();
    bool bOk = false;
    qint64 nOffset = 0;

    if (sTrimmed.startsWith("0x", Qt::CaseInsensitive)) {
        sTrimmed = sTrimmed.mid(2);
    }

    if (!sTrimmed.isEmpty()) {
        nOffset = sTrimmed.toLongLong(&bOk, 16);

        if (!bOk) {
            nOffset = sTrimmed.toLongLong(&bOk, 10);
        }
    }

    if (pbOk) {
        *pbOk = bOk;
    }

    return nOffset;
}

qint64 parseXFCount(const QString &sValue, bool *pbOk = nullptr)
{
    QString sTrimmed = sValue.trimmed();
    int nBase = 10;
    if (sTrimmed.startsWith(QLatin1String("0x"), Qt::CaseInsensitive)) {
        sTrimmed = sTrimmed.mid(2);
        nBase = 16;
    }

    bool bOk = false;
    const qint64 nResult = sTrimmed.isEmpty() ? 0 : sTrimmed.toLongLong(&bOk, nBase);
    if (pbOk) *pbOk = bOk;
    return nResult;
}
}  // namespace

XFormats::XFormats(QObject *pParent) : XThreadObject(pParent)
{
    m_mode = MODE_UNKNOWN;
    m_fileFormat = XBinary::FT_UNKNOWN;
    m_pDevice = nullptr;
    m_pPdStruct = nullptr;
}

QIODevice *XFormats::createDevice(const XBinary::INDATA &indate, bool bIsReadOnly)
{
    QIODevice *pResult = nullptr;

    if (indate.inDataMode == XBinary::INDATA_MODE_FILE) {
        if (indate.sFileName.isEmpty()) {
            return nullptr;
        }

        QFile *pFile = new QFile();
        pFile->setFileName(indate.sFileName);

        const QFile::OpenMode fileMode = bIsReadOnly ? QIODevice::ReadOnly : QIODevice::ReadWrite;

        if (!pFile->open(fileMode)) {
            delete pFile;
            pFile = nullptr;
        } else {
            pFile->setProperty("XFormatsOwnedDevice", true);
        }

        pResult = pFile;
    } else if (indate.inDataMode == XBinary::INDATA_MODE_DEVICE) {
        if (indate.pDevice && indate.pDevice->isOpen() && indate.pDevice->isReadable() && (bIsReadOnly || indate.pDevice->isWritable())) {
            pResult = indate.pDevice;
        }
    }

    return pResult;
}

void XFormats::removeDevice(QIODevice *pDevice, const XBinary::INDATA &indate)
{
    if (indate.inDataMode == XBinary::INDATA_MODE_FILE) {
        if (pDevice && pDevice->property("XFormatsOwnedDevice").toBool()) {
            pDevice->setProperty("XFormatsOwnedDevice", QVariant());
            pDevice->close();
            delete pDevice;
        }
    }
}

XBinary::INDATA XFormats::createINDATA(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    XBinary::INDATA result = {};
    result.inDataMode = XBinary::INDATA_MODE_DEVICE;
    result.fileType = fileType;
    result.pDevice = pDevice;
    result.bIsImage = bIsImage;
    result.nModuleAddress = nModuleAddress;

    return result;
}

XBinary::INDATA XFormats::createINDATA(XBinary::FT fileType, const QString &sFileName, bool bIsImage, XADDR nModuleAddress)
{
    XBinary::INDATA result = {};
    result.inDataMode = XBinary::INDATA_MODE_FILE;
    result.fileType = fileType;
    result.sFileName = sFileName;
    result.bIsImage = bIsImage;
    result.nModuleAddress = nModuleAddress;

    return result;
}

XBinary *XFormats::createClass(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
#ifdef USE_STATICUNPACKER
    // Dispatch the appended family-specific SFX range before the legacy
    // factory chain. Keeping this independent avoids MSVC's block-nesting
    // ceiling in the already very long else-if sequence below.
    if ((fileType >= XBinary::FT_PE32_ZIPSFX) && (fileType <= XBinary::FT_ELF64_ZPAQSFX)) {
        if (fileType <= XBinary::FT_ELF64_ZIPSFX) return new XZipSFX(pDevice, bIsImage, nModuleAddress);
        if (fileType <= XBinary::FT_ELF64_RARSFX) return new XRarSFX(pDevice, bIsImage, nModuleAddress);
        if (fileType <= XBinary::FT_ELF64_CABSFX) return new XCabSFX(pDevice, bIsImage, nModuleAddress);
        if (fileType <= XBinary::FT_ELF64_FREEARCSFX) return new XFreeArcSFX(pDevice, bIsImage, nModuleAddress);
        return new XZPAQSFX(pDevice, bIsImage, nModuleAddress);
    }
#endif

    if (XBinary::checkFileType(XBinary::FT_BINARY, fileType)) return new XBinary(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_COM, fileType)) return new XCOM(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_MSDOS, fileType)) return new XMSDOS(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_NE, fileType)) return new XNE(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_LE, fileType) || XBinary::checkFileType(XBinary::FT_LX, fileType)) return new XLE(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_PE, fileType)) return new XPE(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_CLI_ASSEMBLY, fileType)) return new XCLIAssembly(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_ELF, fileType)) return new XELF(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_MACHO, fileType)) return new XMACH(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_AMIGAHUNK, fileType)) return new XAmigaHunk(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_ATARIST, fileType)) return new XAtariST(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_DTC, fileType)) return new XDTC(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_DMA, fileType)) return new XDMA(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_MUS, fileType)) return new XMUS(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_SND, fileType)) return new XSND(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_PNG, fileType)) return new XPNG(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_JPEG, fileType)) return new XJpeg(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_ICO, fileType)) return new XIcon(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_BMP, fileType)) return new XBMP(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_GIF, fileType)) return new XGif(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_ICC, fileType)) return new XICC(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_TIFF, fileType)) return new XTiff(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_MP4, fileType)) return new XMP4(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_MP3, fileType)) return new XMP3(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_PMA, fileType)) return new XPMA(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_MDH, fileType)) return new XMDH(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_SM8, fileType)) return new XSM8(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_WAV, fileType)) return new XWAV(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_XM, fileType)) return new XXM(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_AVI, fileType)) return new XAVI(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_WEBP, fileType)) return new XWEBP(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_RIFF, fileType)) return new XRiff(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_JAVACLASS, fileType)) return new XJavaClass(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_PYC, fileType)) return new XPYC(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_WASM, fileType)) return new XWASM(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_TTF, fileType)) return new XTTF(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_DJVU, fileType)) return new XDJVU(pDevice);
    // DER is a generic ASN.1 container; we map it under DOCUMENT if requested
    else if (XBinary::checkFileType(XBinary::FT_DER, fileType)) return new XDER(pDevice);
#ifdef USE_DEX
    else if (XBinary::checkFileType(XBinary::FT_ANDROIDXML, fileType) || XBinary::checkFileType(XBinary::FT_ANDROIDASRC, fileType)) return new XAndroidBinary(pDevice);
#endif
    else if (XBinary::checkFileType(XBinary::FT_TEXT, fileType)) return new XText(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_UTF8, fileType)) return new XText(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_UNICODE, fileType)) return new XText(pDevice);
#ifdef USE_DEX
    else if (XBinary::checkFileType(XBinary::FT_DEX, fileType)) return new XDEX(pDevice);
#endif
#ifdef USE_PDF
    else if (XBinary::checkFileType(XBinary::FT_PDF, fileType)) return new XPDF(pDevice);
#endif
#ifdef USE_PDB
    else if (XBinary::checkFileType(XBinary::FT_PDB, fileType)) return new XPDB(pDevice);
#endif
    // Start a fresh guarded-return chain here.  MSVC represents a long
    // else-if sequence as nested blocks and hits C1061 when every optional
    // format family is enabled.  Every preceding match already returns, so
    // separating the chains does not change dispatch order or behaviour.
    if (XBinary::checkFileType(XBinary::FT_ZIP, fileType)) return new XZip(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_JAR, fileType)) return new XJAR(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_APK, fileType)) return new XAPK(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_IPA, fileType)) return new XIPA(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_APKS, fileType)) return new XAPKS(pDevice);
#ifdef USE_ARCHIVE
    else if (XBinary::checkFileType(XBinary::FT_DMG, fileType)) return new XDMG(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_7Z, fileType)) return new XSevenZip(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_CAB, fileType)) return new XCab(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_RAR, fileType)) return new XRar(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_MACHOFAT, fileType)) return new XMACHOFat(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_NPM, fileType)) return new XNPM(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_TAR_GZ, fileType) || XBinary::checkFileType(XBinary::FT_TAR_BZIP2, fileType) ||
             XBinary::checkFileType(XBinary::FT_TAR_LZIP, fileType) || XBinary::checkFileType(XBinary::FT_TAR_LZMA, fileType) ||
             XBinary::checkFileType(XBinary::FT_TAR_LZOP, fileType) || XBinary::checkFileType(XBinary::FT_TAR_XZ, fileType) ||
             XBinary::checkFileType(XBinary::FT_TAR_Z, fileType) || XBinary::checkFileType(XBinary::FT_TAR_ZSTD, fileType) ||
             XBinary::checkFileType(XBinary::FT_TAR_LZ4, fileType)) {
        XTARCOMPRESSED::COMPRESSION_TYPE compressionType = XTARCOMPRESSED::detectCompressionType(pDevice);
        XArchive *pArchive = (XArchive *)XTARCOMPRESSED::getCompressionClassInstance(compressionType, pDevice);

        if (pArchive) {
            return pArchive;
        }

        if (XBinary::checkFileType(XBinary::FT_TAR_BZIP2, fileType)) return new XTAR_BZIP2(pDevice);
        if (XBinary::checkFileType(XBinary::FT_TAR_LZIP, fileType)) return new XTAR_LZIP(pDevice);
        if (XBinary::checkFileType(XBinary::FT_TAR_LZMA, fileType)) return new XTAR_LZMA(pDevice);
        if (XBinary::checkFileType(XBinary::FT_TAR_LZOP, fileType)) return new XTAR_LZOP(pDevice);
        if (XBinary::checkFileType(XBinary::FT_TAR_XZ, fileType)) return new XTAR_XZ(pDevice);
        if (XBinary::checkFileType(XBinary::FT_TAR_Z, fileType)) return new XTAR_COMPRESS(pDevice);
        if (XBinary::checkFileType(XBinary::FT_TAR_ZSTD, fileType)) return new XTAR_ZSTD(pDevice);
        if (XBinary::checkFileType(XBinary::FT_TAR_LZ4, fileType)) return new XTAR_LZ4(pDevice);

        return new XTAR_GZ(pDevice);
    } else if (XFilteredArchive::isFilterFileType(fileType) && !XFilteredArchive::isRecursionSuppressed(pDevice) && XFilteredArchive::isValid(pDevice, fileType)) {
        return new XFilteredArchive(pDevice, fileType);
    } else if (XBinary::checkFileType(XBinary::FT_GZIP, fileType)) return new XGzip(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_ZLIB, fileType)) return new XZlib(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_LHA, fileType)) return new XLHA(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_SAR, fileType)) return new XSAR(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_ARX, fileType)) return new XARX(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_ARJ, fileType)) return new XARJ(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_ACE, fileType)) return new XACE(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_CFBF, fileType)) return new XCFBF(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_SZDD, fileType)) return new XSZDD(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_BZIP2, fileType)) return new XBZIP2(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_BROTLI, fileType)) return new XBrotli(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_LZ4, fileType)) return new XLZ4(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_LZ5, fileType)) return new XLZ5(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_LIZARD, fileType)) return new XLizard(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_LZMA, fileType)) return new XLZMA(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_LZO, fileType)) return new XLzo(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_COMPRESS, fileType)) return new XCompressZ(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_ZSTD, fileType)) return new XZstd(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_LZIP, fileType)) return new XLzip(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_TAR, fileType)) return new XTAR(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_XZ, fileType)) return new XXZ(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_DEB, fileType)) return new XDEB(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_AR, fileType)) return new X_Ar(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_CPIO, fileType)) return new XCPIO(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_ISO9660, fileType)) return new XISO9660(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_UDF, fileType)) return new XUDF(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_WIM, fileType)) return new XWIM(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_RPM, fileType)) return new XRPM(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_KWAJ, fileType)) return new XKWAJ(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_ASAR, fileType)) return new XASAR(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_XAR, fileType)) return new XXAR(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_ZOO, fileType)) return new XZOO(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_STK, fileType)) return new XStk(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_ARC, fileType)) return new XSEAARC(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_FREEARC, fileType)) return new XFREEARC(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_ZPAQ, fileType)) return new XZPAQ(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_BCM, fileType)) return new XBCM(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_LPAQ8, fileType)) return new XLPAQ8(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_PEA, fileType)) return new XPEA(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_PMM, fileType)) return new XPMM(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_PARSEC_ARCHIVE, fileType)) return new XParsecArchive(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_CKP, fileType)) return new XCKP(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_EDP, fileType)) return new XEDP(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_MPQ, fileType)) return new XMPQ(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_BIGF, fileType)) return new XBIGF(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_QUAKE_PAK, fileType)) return new XPAK(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_DOOM_WAD, fileType)) return new XWAD(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_BUILD_GRP, fileType)) return new XGRP(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_SQUASHFS, fileType)) return new XSquashfs(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_MINIDUMP, fileType)) return new XMiniDump(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_WARC, fileType)) return new XWARC(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_MTREE, fileType)) return new XMTree(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_UU, fileType)) return new XUU(pDevice);

    else if (XBinary::checkFileType(XBinary::FT_DOS4G, fileType) || XBinary::checkFileType(XBinary::FT_DOS16M, fileType)) return new XDOS16(pDevice);
#ifdef USE_STATICUNPACKER
    // XStaticUnpacker packer/protector/installer handle-method file types.
    else if (XBinary::checkFileType(XBinary::FT_RIB, fileType)) return new XRIB(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_ISCAB, fileType)) return new XISCab(pDevice);
    else if (XBinary::checkFileType(XBinary::FT_PE32_WIXBURN, fileType) || XBinary::checkFileType(XBinary::FT_PE64_WIXBURN, fileType))
        return new XBurn(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_PE32_7ZSFX, fileType) || XBinary::checkFileType(XBinary::FT_PE64_7ZSFX, fileType))
        return new XSevenZipSFX(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_PE32_ACTUALINSTALLER, fileType) || XBinary::checkFileType(XBinary::FT_PE64_ACTUALINSTALLER, fileType))
        return new XActualInstaller(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_PE32_ADVANCEDINSTALLER, fileType) || XBinary::checkFileType(XBinary::FT_PE64_ADVANCEDINSTALLER, fileType))
        return new XAdvancedInstaller(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_PE32_ASPACK, fileType)) return new XASPACK(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_PE32_AUTOIT, fileType) || XBinary::checkFileType(XBinary::FT_PE64_AUTOIT, fileType))
        return new XAUTOIT(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_PE32_BOXEDAPP, fileType) || XBinary::checkFileType(XBinary::FT_PE64_BOXEDAPP, fileType))
        return new XBoxedApp(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_PE32_CLICKTEAM, fileType) || XBinary::checkFileType(XBinary::FT_PE64_CLICKTEAM, fileType))
        return new XClickteam(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_PE32_CREATEINSTALL, fileType) || XBinary::checkFileType(XBinary::FT_PE64_CREATEINSTALL, fileType))
        return new XCreateInstall(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_PE32_ENIGMAVB, fileType) || XBinary::checkFileType(XBinary::FT_PE64_ENIGMAVB, fileType))
        return new XEnigmaVB(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_PE32_FSG, fileType)) return new XFSG(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_PE32_IEXPRESS, fileType) || XBinary::checkFileType(XBinary::FT_PE64_IEXPRESS, fileType))
        return new XIExpress(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_PE32_INNOSETUP, fileType) || XBinary::checkFileType(XBinary::FT_PE64_INNOSETUP, fileType))
        return new XInnoSetup(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_PE32_INSTALLFORGE, fileType) || XBinary::checkFileType(XBinary::FT_PE64_INSTALLFORGE, fileType))
        return new XInstallForge(pDevice, bIsImage, nModuleAddress);
#ifdef USE_XEMULATOR
    else if (XBinary::checkFileType(XBinary::FT_PE32_INSTALLSIMPLE, fileType) || XBinary::checkFileType(XBinary::FT_PE64_INSTALLSIMPLE, fileType))
        return new XInstallSimple(pDevice, bIsImage, nModuleAddress);
#endif
    else if (XBinary::checkFileType(XBinary::FT_PE32_MEW, fileType)) return new XMEW(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_CFBF_MSI, fileType)) return new XMSI(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_PE32_NSIS, fileType) || XBinary::checkFileType(XBinary::FT_PE64_NSIS, fileType))
        return new XNSIS(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_PE32_NSPACK, fileType)) return new XNSPACK(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_PE32_PETITE, fileType)) return new XPETITE(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_PE32_SFX, fileType) || XBinary::checkFileType(XBinary::FT_PE64_SFX, fileType) ||
             XBinary::checkFileType(XBinary::FT_ELF32_SFX, fileType) || XBinary::checkFileType(XBinary::FT_ELF64_SFX, fileType))
        return new XSFX(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_PE32_SMARTINSTALL, fileType) || XBinary::checkFileType(XBinary::FT_PE64_SMARTINSTALL, fileType))
        return new XSmartInstall(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_PE32_TARMA, fileType) || XBinary::checkFileType(XBinary::FT_PE64_TARMA, fileType))
        return new XTarma(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_UPX, fileType) || XBinary::checkFileType(XBinary::FT_PE32_UPX, fileType) ||
             XBinary::checkFileType(XBinary::FT_PE64_UPX, fileType))
        return new XUPX(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_PE32_WINRARSFX, fileType) || XBinary::checkFileType(XBinary::FT_PE64_WINRARSFX, fileType))
        return new XRarSFX(pDevice, bIsImage, nModuleAddress, true);
    else if (XBinary::checkFileType(XBinary::FT_CFBF_WIX, fileType)) return new XWiX(pDevice, bIsImage, nModuleAddress);
    else if (XBinary::checkFileType(XBinary::FT_PE32_YODA, fileType)) return new XYODA(pDevice, bIsImage, nModuleAddress);
#endif
#endif
    else {
#ifdef QT_DEBUG
        qDebug() << "XFormats::createClass: Unknown file type" << XBinary::fileTypeIdToString(fileType);
#endif
        XBinary *pBinary = new XBinary(pDevice, bIsImage, nModuleAddress);

        return pBinary;
    }
}

bool XFormats::isValid(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress, XBinary::PDSTRUCT *pPdStruct)
{
    bool bResult = false;

    if (!isReadableSeekableDevice(pDevice) || !XBinary::isPdStructNotCanceled(pPdStruct)) return false;
    DevicePositionGuard positionGuard(pDevice);

    XBinary *pBinary = XFormats::createClass(fileType, pDevice, bIsImage, nModuleAddress);
    bResult = pBinary->isValid(pPdStruct);
    delete pBinary;

    return bResult && XBinary::isPdStructNotCanceled(pPdStruct);
}

XBinary::_MEMORY_MAP XFormats::getMemoryMap(XBinary::FT fileType, XBinary::MAPMODE mapMode, QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress,
                                            XBinary::PDSTRUCT *pPdStruct)
{
    XBinary::_MEMORY_MAP result = {};

    if (!isReadableSeekableDevice(pDevice) || !XBinary::isPdStructNotCanceled(pPdStruct)) return result;
    DevicePositionGuard positionGuard(pDevice);

    XBinary *pBinary = XFormats::createClass(fileType, pDevice, bIsImage, nModuleAddress);
    result = pBinary->getMemoryMap(mapMode, pPdStruct);
    delete pBinary;

    if (!XBinary::isPdStructNotCanceled(pPdStruct)) result = {};

    return result;
}

XBinary::_MEMORY_MAP XFormats::getMemoryMap(const QString &sFileName, XBinary::MAPMODE mapMode, bool bIsImage, XADDR nModuleAddress, XBinary::PDSTRUCT *pPdStruct)
{
    XBinary::_MEMORY_MAP result = {};

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        result = getMemoryMap(XFormats::getPrefFileType(&file, XBinary::FT_FLAG_FORMATS), mapMode, &file, bIsImage, nModuleAddress, pPdStruct);

        file.close();
    }

    return result;
}

QList<XBinary::MAPMODE> XFormats::getMapModesList(XBinary::FT fileType)
{
    QList<XBinary::MAPMODE> listResult = {};

    XBinary *pBinary = XFormats::createClass(fileType, nullptr);
    listResult = pBinary->getMapModesList();
    delete pBinary;

    return listResult;
}

QList<XBinary::FT> XFormats::getAvailableFileTypes()
{
    QList<XBinary::FT> listResult;

    listResult.append(XBinary::FT_BINARY);
    listResult.append(XBinary::FT_COM);
    listResult.append(XBinary::FT_MSDOS);
    listResult.append(XBinary::FT_NE);
    listResult.append(XBinary::FT_LE);
    listResult.append(XBinary::FT_PE);
    listResult.append(XBinary::FT_ELF);
    listResult.append(XBinary::FT_MACHO);
    listResult.append(XBinary::FT_AMIGAHUNK);
    listResult.append(XBinary::FT_ATARIST);
    listResult.append(XBinary::FT_DTC);
    listResult.append(XBinary::FT_DMA);
    listResult.append(XBinary::FT_MUS);
    listResult.append(XBinary::FT_SND);
    listResult.append(XBinary::FT_PNG);
    listResult.append(XBinary::FT_JPEG);
    listResult.append(XBinary::FT_ICO);
    listResult.append(XBinary::FT_BMP);
    listResult.append(XBinary::FT_GIF);
    listResult.append(XBinary::FT_ICC);
    listResult.append(XBinary::FT_TIFF);
    listResult.append(XBinary::FT_MP4);
    listResult.append(XBinary::FT_MP3);
    listResult.append(XBinary::FT_PMA);
    listResult.append(XBinary::FT_MDH);
    listResult.append(XBinary::FT_SM8);
    listResult.append(XBinary::FT_WAV);
    listResult.append(XBinary::FT_XM);
    listResult.append(XBinary::FT_AVI);
    listResult.append(XBinary::FT_WEBP);
    listResult.append(XBinary::FT_RIFF);
    listResult.append(XBinary::FT_JAVACLASS);
    listResult.append(XBinary::FT_PYC);
    listResult.append(XBinary::FT_WASM);
    listResult.append(XBinary::FT_TTF);
    listResult.append(XBinary::FT_DJVU);
    listResult.append(XBinary::FT_DER);
#ifdef USE_DEX
    listResult.append(XBinary::FT_ANDROIDXML);
    listResult.append(XBinary::FT_ANDROIDASRC);
#endif
    listResult.append(XBinary::FT_TEXT);
    listResult.append(XBinary::FT_UTF8);
    listResult.append(XBinary::FT_UNICODE);
#ifdef USE_DEX
    listResult.append(XBinary::FT_DEX);
#endif
#ifdef USE_PDF
    listResult.append(XBinary::FT_PDF);
#endif
#ifdef USE_PDB
    listResult.append(XBinary::FT_PDB);
#endif
    listResult.append(XBinary::FT_ZIP);
    listResult.append(XBinary::FT_JAR);
    listResult.append(XBinary::FT_APK);
    listResult.append(XBinary::FT_IPA);
    listResult.append(XBinary::FT_APKS);
#ifdef USE_ARCHIVE
    listResult.append(XBinary::FT_DMG);
    listResult.append(XBinary::FT_7Z);
    listResult.append(XBinary::FT_CAB);
    listResult.append(XBinary::FT_RAR);
    listResult.append(XBinary::FT_MACHOFAT);
    listResult.append(XBinary::FT_NPM);
    listResult.append(XBinary::FT_TAR_GZ);
    listResult.append(XBinary::FT_TAR_BZIP2);
    listResult.append(XBinary::FT_TAR_LZIP);
    listResult.append(XBinary::FT_TAR_LZMA);
    listResult.append(XBinary::FT_TAR_LZOP);
    listResult.append(XBinary::FT_TAR_XZ);
    listResult.append(XBinary::FT_TAR_Z);
    listResult.append(XBinary::FT_TAR_ZSTD);
    listResult.append(XBinary::FT_TAR_LZ4);
    listResult.append(XBinary::FT_GZIP);
    listResult.append(XBinary::FT_ZLIB);
    listResult.append(XBinary::FT_LHA);
    listResult.append(XBinary::FT_SAR);
    listResult.append(XBinary::FT_ARX);
    listResult.append(XBinary::FT_ARJ);
    listResult.append(XBinary::FT_ACE);
    listResult.append(XBinary::FT_ARC);
    listResult.append(XBinary::FT_FREEARC);
    listResult.append(XBinary::FT_ZPAQ);
    listResult.append(XBinary::FT_BCM);
    listResult.append(XBinary::FT_LPAQ8);
    listResult.append(XBinary::FT_PEA);
    listResult.append(XBinary::FT_CFBF);
    listResult.append(XBinary::FT_SZDD);
    listResult.append(XBinary::FT_BZIP2);
    listResult.append(XBinary::FT_BROTLI);
    listResult.append(XBinary::FT_LZ4);
    listResult.append(XBinary::FT_LZ5);
    listResult.append(XBinary::FT_LIZARD);
    listResult.append(XBinary::FT_LZMA);
    listResult.append(XBinary::FT_LZO);
    listResult.append(XBinary::FT_COMPRESS);
    listResult.append(XBinary::FT_ZSTD);
    listResult.append(XBinary::FT_LZIP);
    listResult.append(XBinary::FT_TAR);
    listResult.append(XBinary::FT_XZ);
    listResult.append(XBinary::FT_DEB);
    listResult.append(XBinary::FT_AR);
    listResult.append(XBinary::FT_CPIO);
    listResult.append(XBinary::FT_ISO9660);
    listResult.append(XBinary::FT_UDF);
    listResult.append(XBinary::FT_SQUASHFS);
    listResult.append(XBinary::FT_WIM);
    listResult.append(XBinary::FT_RPM);
    listResult.append(XBinary::FT_KWAJ);
    listResult.append(XBinary::FT_ASAR);
    listResult.append(XBinary::FT_XAR);
    listResult.append(XBinary::FT_ZOO);
    listResult.append(XBinary::FT_STK);
    listResult.append(XBinary::FT_QUAKE_PAK);
    listResult.append(XBinary::FT_DOOM_WAD);
    listResult.append(XBinary::FT_BUILD_GRP);
    listResult.append(XBinary::FT_PMM);
    listResult.append(XBinary::FT_PARSEC_ARCHIVE);
    listResult.append(XBinary::FT_CKP);
    listResult.append(XBinary::FT_MPQ);
    listResult.append(XBinary::FT_EDP);
    listResult.append(XBinary::FT_BIGF);
    listResult.append(XBinary::FT_WARC);
    listResult.append(XBinary::FT_MTREE);
    listResult.append(XBinary::FT_UU);
    listResult.append(XBinary::FT_DOS4G);
    listResult.append(XBinary::FT_DOS16M);
#endif
#ifdef USE_STATICUNPACKER
    listResult.append(XBinary::FT_ISCAB);
    listResult.append(XBinary::FT_RIB);
#endif

    return listResult;
}

XADDR XFormats::getEntryPointAddress(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    // TODO pMemoryMap !!!
    XADDR nResult = 0;

    if (!isReadableSeekableDevice(pDevice)) return nResult;
    DevicePositionGuard positionGuard(pDevice);

    XBinary *pBinary = XFormats::createClass(fileType, pDevice, bIsImage, nModuleAddress);
    nResult = pBinary->getEntryPointAddress();
    delete pBinary;

    return nResult;
}

qint64 XFormats::getEntryPointOffset(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    qint64 nResult = 0;

    if (!isReadableSeekableDevice(pDevice)) return nResult;
    DevicePositionGuard positionGuard(pDevice);

    XBinary *pBinary = XFormats::createClass(fileType, pDevice, bIsImage, nModuleAddress);
    nResult = pBinary->_getEntryPointOffset();
    delete pBinary;

    return nResult;
}

bool XFormats::isBigEndian(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    bool bResult = false;

    if (!isReadableSeekableDevice(pDevice)) return false;
    DevicePositionGuard positionGuard(pDevice);

    XBinary *pBinary = XFormats::createClass(fileType, pDevice, bIsImage, nModuleAddress);
    bResult = pBinary->isBigEndian();
    delete pBinary;

    return bResult;
}

QList<XBinary::FPART> XFormats::getHighlights(XBinary::FT fileType, QIODevice *pDevice, XBinary::HLTYPE hlType, bool bIsImage, XADDR nModuleAddress,
                                              XBinary::PDSTRUCT *pPdStruct)
{
    QList<XBinary::FPART> listResult;

    if (!isReadableSeekableDevice(pDevice) || !XBinary::isPdStructNotCanceled(pPdStruct)) return listResult;
    DevicePositionGuard positionGuard(pDevice);

    XBinary *pBinary = XFormats::createClass(fileType, pDevice, bIsImage, nModuleAddress);
    listResult = pBinary->getHighlights(hlType, pPdStruct);
    delete pBinary;

    if (!XBinary::isPdStructNotCanceled(pPdStruct)) listResult.clear();

    return listResult;
}

bool XFormats::isSigned(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    bool bResult = false;

    if (!isReadableSeekableDevice(pDevice)) return false;
    DevicePositionGuard positionGuard(pDevice);

    XBinary *pBinary = XFormats::createClass(fileType, pDevice, bIsImage, nModuleAddress);
    bResult = pBinary->isSigned();
    delete pBinary;

    return bResult;
}

XBinary::OFFSETSIZE XFormats::getSignOffsetSize(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress)
{
    XBinary::OFFSETSIZE osResult = {};

    if (!isReadableSeekableDevice(pDevice)) return osResult;
    DevicePositionGuard positionGuard(pDevice);

    XBinary *pBinary = XFormats::createClass(fileType, pDevice, bIsImage, nModuleAddress);
    osResult = pBinary->getSignOffsetSize();
    delete pBinary;

    return osResult;
}

XBinary::OFFSETSIZE XFormats::getSignOffsetSize(const QString &sFileName)
{
    XBinary::OFFSETSIZE result = {};

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        result = getSignOffsetSize(XFormats::getPrefFileType(&file, XBinary::FT_FLAG_EXECUTABLES), &file);

        file.close();
    }

    return result;
}

bool XFormats::isSigned(const QString &sFileName)
{
    bool bResult = false;

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        bResult = isSigned(XFormats::getPrefFileType(&file, XBinary::FT_FLAG_EXECUTABLES), &file);

        file.close();
    }

    return bResult;
}

QList<XBinary::SYMBOL_RECORD> XFormats::getSymbolRecords(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress, XBinary::SYMBOL_TYPE symBolType)
{
    QList<XBinary::SYMBOL_RECORD> listResult;

    if (!isReadableSeekableDevice(pDevice)) return listResult;
    DevicePositionGuard positionGuard(pDevice);

    XBinary *pBinary = XFormats::createClass(fileType, pDevice, bIsImage, nModuleAddress);
    XBinary::_MEMORY_MAP memoryMap = pBinary->getMemoryMap();
    listResult = pBinary->getSymbolRecords(&memoryMap, symBolType);
    delete pBinary;

    return listResult;
}

QSet<XBinary::FT> XFormats::getFileTypes(QIODevice *pDevice, quint32 nFTFlags, XBinary::PDSTRUCT *pPdStruct)
{
    QSet<XBinary::FT> result;
    if (!isReadableSeekableDevice(pDevice) || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return result;
    }

    XBinary::PDSTRUCT pdStructEmpty = {};

    if (!pPdStruct) {
        pdStructEmpty = XBinary::createPdStruct();
        pPdStruct = &pdStructEmpty;
    }

    DevicePositionGuard positionGuard(pDevice);
    result = _getFileTypes(pDevice, nFTFlags, pPdStruct);
    if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
        result.clear();
    }

    return result;
}

bool XFormats::saveAllPEIconsToDirectory(QIODevice *pDevice, const QString &sDirectoryName)
{
    bool bResult = false;

    XPE pe(pDevice);

    if (pe.isValid()) {
        bResult = true;

        QList<XPE::RESOURCE_RECORD> listResources = pe.getResources(10000);
        QList<XPE::RESOURCE_RECORD> listIcons = pe.getResourceRecords(XPE_DEF::S_RT_GROUP_ICON, (quint32)-1, &listResources);

        qint32 nNumberOfRecords = listIcons.size();

        for (qint32 i = 0; i < nNumberOfRecords; i++) {
            XPE::RESOURCE_RECORD resourceRecord = listIcons.at(i);

            QString sFileName = sDirectoryName + QDir::separator() + QString("%1.ico").arg(XBinary::convertFileNameSymbols(XPE::resourceRecordToString(resourceRecord)));

            if (!XFormats::savePE_ICOToFile(pDevice, &listResources, resourceRecord, sFileName)) {
                bResult = false;
            }
        }
    }

    return bResult;
}

bool XFormats::saveAllPECursorsToDirectory(QIODevice *pDevice, const QString &sDirectoryName)
{
    bool bResult = false;

    XPE pe(pDevice);

    if (pe.isValid()) {
        bResult = true;

        QList<XPE::RESOURCE_RECORD> listResources = pe.getResources(10000);
        QList<XPE::RESOURCE_RECORD> listIcons = pe.getResourceRecords(XPE_DEF::S_RT_GROUP_CURSOR, (quint32)-1, &listResources);

        qint32 nNumberOfRecords = listIcons.size();

        for (qint32 i = 0; i < nNumberOfRecords; i++) {
            XPE::RESOURCE_RECORD resourceRecord = listIcons.at(i);

            QString sFileName = sDirectoryName + QDir::separator() + QString("%1.cur").arg(XBinary::convertFileNameSymbols(XPE::resourceRecordToString(resourceRecord)));

            if (!XFormats::savePE_ICOToFile(pDevice, &listResources, resourceRecord, sFileName)) {
                bResult = false;
            }
        }
    }

    return bResult;
}

bool XFormats::savePE_ICOToFile(QIODevice *pDevice, QList<XPE::RESOURCE_RECORD> *pListResourceRecords, XPE::RESOURCE_RECORD resourceRecord, const QString &sFileName)
{
    bool bResult = false;

    if (!isReadableSeekableDevice(pDevice) || !pListResourceRecords || sFileName.isEmpty() ||
        !isValidDeviceRange(pDevice, resourceRecord.nOffset, resourceRecord.nSize) || (resourceRecord.nSize <= 0)) {
        return false;
    }

    DevicePositionGuard positionGuard(pDevice);

    qint32 nChunkType = XPE_DEF::S_RT_ICON;
    qint32 idType = 1;

    if (resourceRecord.irin[0].nID == XPE_DEF::S_RT_GROUP_CURSOR) {
        nChunkType = XPE_DEF::S_RT_CURSOR;
        idType = 2;
    }

    XPE xpe(pDevice);

    if (xpe.isValid()) {
        if (resourceRecord.nSize) {
            SubDevice sd(pDevice, resourceRecord.nOffset, resourceRecord.nSize);

            if (sd.open(QIODevice::ReadOnly)) {
                XIcon icon(&sd);

                if (icon.isValid()) {
                    QList<XPE::RESOURCE_RECORD> listChunkRecords;
                    qint64 nTotalDataSize = 0;

                    QList<XIcon::GRPICONDIRENTRY> listDirectories = icon.getIconGPRDirectories();

                    qint32 nNumberOfRecords = listDirectories.size();

                    if ((nNumberOfRecords <= 0) || (nNumberOfRecords > (std::numeric_limits<quint16>::max)())) {
                        sd.close();
                        return false;
                    }

                    for (qint32 i = 0; i < nNumberOfRecords; i++) {
                        XPE::RESOURCE_RECORD _resourceRecord = xpe.getResourceRecord(nChunkType, listDirectories.at(i).nID, pListResourceRecords);

                        const qint64 nChunkSize = (qint64)listDirectories.at(i).dwBytesInRes;
                        if ((nChunkSize <= 0) || (nChunkSize > _resourceRecord.nSize) || !isValidDeviceRange(pDevice, _resourceRecord.nOffset, nChunkSize) ||
                            (nTotalDataSize > (std::numeric_limits<qint64>::max)() - nChunkSize)) {
                            sd.close();
                            return false;
                        }

                        listChunkRecords.append(_resourceRecord);

                        nTotalDataSize += nChunkSize;
                    }

                    const qint64 nTableSize = sizeof(XIcon::ICONDIR) + (qint64)sizeof(XIcon::ICONDIRENTRY) * nNumberOfRecords;
                    if ((nTableSize > (std::numeric_limits<qint32>::max)()) || (nTotalDataSize > (std::numeric_limits<qint64>::max)() - nTableSize) ||
                        (nTableSize + nTotalDataSize > (qint64)(std::numeric_limits<quint32>::max)())) {
                        sd.close();
                        return false;
                    }

                    QByteArray baTable((qint32)nTableSize, 0);
                    XBinary::_write_uint16(baTable.data() + offsetof(XIcon::ICONDIR, idReserved), 0);
                    XBinary::_write_uint16(baTable.data() + offsetof(XIcon::ICONDIR, idType), (quint16)idType);
                    XBinary::_write_uint16(baTable.data() + offsetof(XIcon::ICONDIR, idCount), (quint16)nNumberOfRecords);

                    qint64 nCurrentTableOffset = sizeof(XIcon::ICONDIR);
                    qint64 nCurrentDataOffset = nTableSize;

                    for (qint32 i = 0; i < nNumberOfRecords; i++) {
                        const XIcon::GRPICONDIRENTRY &source = listDirectories.at(i);
                        char *pEntry = baTable.data() + nCurrentTableOffset;
                        XBinary::_write_uint8(pEntry + offsetof(XIcon::ICONDIRENTRY, bWidth), source.bWidth);
                        XBinary::_write_uint8(pEntry + offsetof(XIcon::ICONDIRENTRY, bHeight), source.bHeight);
                        XBinary::_write_uint8(pEntry + offsetof(XIcon::ICONDIRENTRY, bColorCount), source.bColorCount);
                        XBinary::_write_uint8(pEntry + offsetof(XIcon::ICONDIRENTRY, bReserved), source.bReserved);
                        XBinary::_write_uint16(pEntry + offsetof(XIcon::ICONDIRENTRY, wPlanes), source.wPlanes);
                        XBinary::_write_uint16(pEntry + offsetof(XIcon::ICONDIRENTRY, wBitCount), source.wBitCount);
                        XBinary::_write_uint32(pEntry + offsetof(XIcon::ICONDIRENTRY, dwBytesInRes), source.dwBytesInRes);
                        XBinary::_write_uint32(pEntry + offsetof(XIcon::ICONDIRENTRY, dwImageOffset), (quint32)nCurrentDataOffset);

                        nCurrentTableOffset += sizeof(XIcon::ICONDIRENTRY);
                        nCurrentDataOffset += source.dwBytesInRes;
                    }

                    QSaveFile file(sFileName);
                    if (file.open(QIODevice::WriteOnly)) {
                        bool bWriteOK = writeAllDevice(&file, baTable.constData(), baTable.size());
                        nCurrentDataOffset = nTableSize;

                        for (qint32 i = 0; bWriteOK && (i < nNumberOfRecords); i++) {
                            const qint64 nChunkSize = (qint64)listDirectories.at(i).dwBytesInRes;
                            bWriteOK = XBinary::copyDeviceMemory(pDevice, listChunkRecords.at(i).nOffset, &file, nCurrentDataOffset, nChunkSize);
                            nCurrentDataOffset += nChunkSize;
                        }

                        if (bWriteOK && (file.size() == nTableSize + nTotalDataSize)) {
                            bResult = file.commit();
                        } else {
                            file.cancelWriting();
                        }
                    }
                }

                sd.close();
            }
        }
    }

    return bResult;
}

QSet<XBinary::FT> XFormats::getFileTypes(QIODevice *pDevice, qint64 nOffset, qint64 nSize, quint32 nFTFlags, XBinary::PDSTRUCT *pPdStruct)
{
    QSet<XBinary::FT> result;

    if (isValidDeviceRange(pDevice, nOffset, nSize) && XBinary::isPdStructNotCanceled(pPdStruct)) {
        DevicePositionGuard positionGuard(pDevice);
        SubDevice sd(pDevice, nOffset, nSize);

        if (sd.open(QIODevice::ReadOnly)) {
            result = getFileTypes(&sd, nFTFlags, pPdStruct);

            sd.close();
        }
    }

    return result;
}

QSet<XBinary::FT> XFormats::getFileTypes(const QString &sFileName, quint32 nFTFlags, XBinary::PDSTRUCT *pPdStruct)
{
    QSet<XBinary::FT> stResult;

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        stResult = getFileTypes(&file, nFTFlags, pPdStruct);

        file.close();
    }

    return stResult;
}

QSet<XBinary::FT> XFormats::getFileTypes(QByteArray *pbaData, quint32 nFTFlags)
{
    QSet<XBinary::FT> stResult;

    if (!pbaData) {
        return stResult;
    }

    QBuffer buffer;

    buffer.setBuffer(pbaData);

    if (buffer.open(QIODevice::ReadOnly)) {
        stResult = getFileTypes(&buffer, nFTFlags);

        buffer.close();
    }

    return stResult;
}

XBinary::FT XFormats::getPrefFileType(QIODevice *pDevice, quint32 nFTFlags, XBinary::PDSTRUCT *pPdStruct)
{
    QSet<XBinary::FT> stFileTypes = getFileTypes(pDevice, nFTFlags, pPdStruct);

    return XBinary::_getPrefFileType(&stFileTypes);
}

XBinary::FT XFormats::getPrefFileType(const QString &sFileName, quint32 nFTFlags, XBinary::PDSTRUCT *pPdStruct)
{
    XBinary::FT fileType = XBinary::FT_UNKNOWN;

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        fileType = getPrefFileType(&file, nFTFlags, pPdStruct);

        file.close();
    }

    return fileType;
}

XBinary::FILEFORMATINFO XFormats::getFileFormatInfo(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress, XBinary::PDSTRUCT *pPdStruct,
                                                    qint64 nOffset, qint64 nSize)
{
    XBinary::FILEFORMATINFO result = {};

    if (!isValidDeviceRange(pDevice, nOffset, nSize) || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return result;
    }

    DevicePositionGuard positionGuard(pDevice);
    QIODevice *pParserDevice = pDevice;
    SubDevice subDevice(pDevice, nOffset, nSize);

    if ((nOffset != 0) || (nSize != -1)) {
        if (!subDevice.open(QIODevice::ReadOnly)) {
            return result;
        }
        pParserDevice = &subDevice;
    }

    XBinary *pBinary = XFormats::createClass(fileType, pParserDevice, bIsImage, nModuleAddress);
    result = pBinary->getFileFormatInfo(pPdStruct);
    delete pBinary;

    if (!XBinary::isPdStructNotCanceled(pPdStruct)) result = {};

    return result;
}

qint64 XFormats::getFileFormatSize(XBinary::FT fileType, QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress, XBinary::PDSTRUCT *pPdStruct, qint64 nOffset,
                                   qint64 nSize)
{
    qint64 nResult = 0;

    if (!isValidDeviceRange(pDevice, nOffset, nSize) || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return nResult;
    }

    DevicePositionGuard positionGuard(pDevice);
    QIODevice *pParserDevice = pDevice;
    SubDevice subDevice(pDevice, nOffset, nSize);

    if ((nOffset != 0) || (nSize != -1)) {
        if (!subDevice.open(QIODevice::ReadOnly)) {
            return nResult;
        }
        pParserDevice = &subDevice;
    }

    XBinary *pBinary = XFormats::createClass(fileType, pParserDevice, bIsImage, nModuleAddress);
    nResult = pBinary->getFileFormatSize(pPdStruct);
    delete pBinary;

    if (!XBinary::isPdStructNotCanceled(pPdStruct)) nResult = 0;

    return nResult;
}

// QList<XBinary::DATA_HEADER> XFormats::getDataHeaders(XBinary::FT fileType, QIODevice *pDevice, const XBinary::DATA_HEADERS_OPTIONS &dataHeadersOptions, bool bIsImage,
//                                                      XADDR nModuleAddress, XBinary::PDSTRUCT *pPdStruct)
// {
//     QList<XBinary::DATA_HEADER> listResult;

//     XBinary *pBinary = XFormats::createClass(fileType, pDevice, bIsImage, nModuleAddress);
//     listResult = pBinary->getDataHeaders(dataHeadersOptions, pPdStruct);
//     delete pBinary;

//     return listResult;
// }

QList<XBinary::XFHEADER> XFormats::getXFHeaders(QIODevice *pDevice, const QString &sTag, bool bIsImage, XADDR nModuleAddress, XBinary::PDSTRUCT *pPdStruct)
{
    QList<XBinary::XFHEADER> listResult;

    if (!isReadableSeekableDevice(pDevice) || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return listResult;
    }

    DevicePositionGuard positionGuard(pDevice);

    // Tag format: Offset::FileType::StructID::Type[::Count]
    // With '#' separator:
    // TAG                  -> no parent (bIsParent=false)
    // TAG#                 -> with parent (bIsParent=true)
    // TAG1#TAG2[#TAG3...]  -> TAG1 with parent, filter results matching TAG2, TAG3, etc.
    // Filter tags can be partial: 5594::ZIP::LOCALFILEHEADER (offset+fileType+structID)
    // or full: 5594::ZIP::LOCALFILEHEADER::TABLE::6

    const QStringList listSegments = sTag.split("#", Qt::KeepEmptyParts);

    if (listSegments.isEmpty()) {
        return listResult;
    }

    QString sRootTag = listSegments.at(0).trimmed();
    bool bIsParent = false;
    QStringList listFilterTags;
    qint32 nSegmentCount = listSegments.size();

    if (nSegmentCount == 1) {
        // No '#' found -> no parent flag
        bIsParent = false;
    } else if ((nSegmentCount == 2) && listSegments.at(1).trimmed().isEmpty()) {
        // Trailing '#' -> parent flag, no filter
        bIsParent = true;
    } else {
        // Multiple segments -> parent flag + filter on remaining segments
        bIsParent = true;
        for (qint32 i = 1; i < nSegmentCount; i++) {
            const QString sFilterTag = listSegments.at(i).trimmed();
            if (!sFilterTag.isEmpty()) {
                listFilterTags.append(sFilterTag);
            }
        }
    }

    // Parse root tag: Offset::FileTypeFt::StructFt::TypeName[::Count]
    QStringList listParts = splitAndTrim(sRootTag, "::");

    if (listParts.size() >= 4) {
        const QString sOffset = listParts.at(0);
        const QString sFileTypeFt = normalizeStructToken(listParts.at(1));
        const QString sStructFt = normalizeStructToken(listParts.at(2));
        const QString sTypeName = listParts.at(3);

        XBinary::FT fileType = XBinary::ftStringToFileTypeId(sFileTypeFt);

        bool bOk = false;
        qint64 nOffset = parseXFOffset(sOffset, &bOk);

        if ((fileType != XBinary::FT_UNKNOWN) && bOk && (nOffset >= 0)) {
            XBinary *pBinary = XFormats::createClass(fileType, pDevice, bIsImage, nModuleAddress);

            if (pBinary) {
                quint32 nStructID = pBinary->ftStringToStructID(sStructFt);

                XBinary::XFTYPE xfType = XBinary::XFTYPE_UNKNOWN;
                xfType = getXFTypeFromToken(sTypeName);

                XBinary::_MEMORY_MAP memoryMap = pBinary->getMemoryMap(XBinary::MAPMODE_UNKNOWN, pPdStruct);

                XBinary::XLOC xLoc = {};
                xLoc.locType = XBinary::LT_OFFSET;
                xLoc.nLocation = nOffset;

                XBinary::XFSTRUCT xfStruct = {};
                xfStruct.pMemoryMap = &memoryMap;
                xfStruct.fileType = fileType;
                xfStruct.nStructID = nStructID;
                xfStruct.xLoc = xLoc;
                xfStruct.xfType = xfType;
                xfStruct.bIsParent = bIsParent;
                xfStruct.nCount = -1;

                // Count is only accepted for canonical 5-part root tags.
                // Overlong root tags are treated as noisy input and parsed with defaults.
                if (listParts.size() == 5) {
                    bool bCountOk = false;
                    qint64 nCount = parseXFCount(listParts.at(4), &bCountOk);
                    if (bCountOk && (nCount >= 0) && (nCount <= INT32_MAX)) {
                        xfStruct.nCount = static_cast<qint32>(nCount);
                    }
                }

                QList<XBinary::XFHEADER> listAllHeaders = pBinary->getXFHeaders(xfStruct, pPdStruct);

                if (listFilterTags.isEmpty()) {
                    listResult = listAllHeaders;
                } else {
                    // Pre-parse filter criteria
                    qint32 nFilterCount = 0;
                    const qint32 nFilterTagCount = listFilterTags.size();
                    QList<qint32> listFilterPartCounts;
                    QList<XBinary::FT> listFilterFileTypes;
                    QList<quint32> listFilterStructIDs;
                    QList<XBinary::XFTYPE> listFilterXfTypes;
                    QList<qint64> listFilterOffsets;
                    QList<qint32> listFilterCounts;

                    for (qint32 i = 0; (i < nFilterTagCount) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
                        QStringList listFilterParts = splitAndTrim(listFilterTags.at(i), "::");
                        qint32 nFilterPartCount = listFilterParts.size();

                        qint64 nFilterOffset = 0;
                        XBinary::FT filterFileType = XBinary::FT_UNKNOWN;
                        quint32 nFilterStructID = 0;
                        XBinary::XFTYPE filterXfType = XBinary::XFTYPE_UNKNOWN;
                        qint32 nFilterRecordCount = -1;
                        bool bFilterValid = true;

                        if (nFilterPartCount == 0) {
                            continue;
                        }
                        if (nFilterPartCount > 5) {
                            bFilterValid = false;
                        }

                        if (nFilterPartCount >= 1) {
                            bool bFilterOk = false;
                            nFilterOffset = parseXFOffset(listFilterParts.at(0), &bFilterOk);
                            if (!bFilterOk || (nFilterOffset < 0)) {
                                bFilterValid = false;
                            }
                        }

                        if (bFilterValid && (nFilterPartCount >= 2)) {
                            filterFileType = XBinary::ftStringToFileTypeId(normalizeStructToken(listFilterParts.at(1)));
                            if (filterFileType == XBinary::FT_UNKNOWN) {
                                bFilterValid = false;
                            }
                        }

                        if (bFilterValid && (nFilterPartCount >= 3)) {
                            nFilterStructID = pBinary->ftStringToStructID(normalizeStructToken(listFilterParts.at(2)));
                            if (nFilterStructID == 0) {
                                bFilterValid = false;
                            }
                        }

                        if (bFilterValid && (nFilterPartCount >= 4)) {
                            filterXfType = getXFTypeFromToken(listFilterParts.at(3));
                            if (filterXfType == XBinary::XFTYPE_UNKNOWN) {
                                bFilterValid = false;
                            }
                        }

                        if (bFilterValid && (nFilterPartCount >= 5)) {
                            bool bCountOk = false;
                            const qint64 nCount = parseXFCount(listFilterParts.at(4), &bCountOk);
                            if (!bCountOk || (nCount < 0) || (nCount > INT32_MAX) || (filterXfType != XBinary::XFTYPE_TABLE)) {
                                bFilterValid = false;
                            } else {
                                nFilterRecordCount = static_cast<qint32>(nCount);
                            }
                        }

                        if (!bFilterValid) {
                            continue;
                        }

                        listFilterPartCounts.append(nFilterPartCount);
                        listFilterFileTypes.append(filterFileType);
                        listFilterStructIDs.append(nFilterStructID);
                        listFilterXfTypes.append(filterXfType);
                        listFilterOffsets.append(nFilterOffset);
                        listFilterCounts.append(nFilterRecordCount);
                        nFilterCount++;
                    }

                    if (nFilterCount) {
                        qint32 nAllCount = listAllHeaders.size();

                        for (qint32 i = 0; (i < nAllCount) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
                            const XBinary::XFHEADER &header = listAllHeaders.at(i);

                            for (qint32 j = 0; (j < nFilterCount) && XBinary::isPdStructNotCanceled(pPdStruct); j++) {
                                bool bMatch = true;
                                qint32 nFPC = listFilterPartCounts.at(j);

                                if (bMatch && (nFPC >= 1)) {
                                    if (header.xLoc.nLocation != (XADDR)listFilterOffsets.at(j)) {
                                        bMatch = false;
                                    }
                                }
                                if (bMatch && (nFPC >= 2)) {
                                    if (header.fileType != listFilterFileTypes.at(j)) {
                                        bMatch = false;
                                    }
                                }
                                if (bMatch && (nFPC >= 3)) {
                                    if ((quint32)header.structID != listFilterStructIDs.at(j)) {
                                        bMatch = false;
                                    }
                                }
                                if (bMatch && (nFPC >= 4)) {
                                    if (header.xfType != listFilterXfTypes.at(j)) {
                                        bMatch = false;
                                    }
                                }
                                if (bMatch && (nFPC >= 5)) {
                                    if (header.listRowLocations.count() != listFilterCounts.at(j)) {
                                        bMatch = false;
                                    }
                                }

                                if (bMatch) {
                                    listResult.append(header);
                                    break;
                                }
                            }
                        }
                    }
                }

                delete pBinary;
            }
        }
    }

    if (!XBinary::isPdStructNotCanceled(pPdStruct)) listResult.clear();
    return listResult;
}

QList<XBinary::FPART> XFormats::getFileParts(XBinary::FT fileType, QIODevice *pDevice, quint32 nFileParts, qint32 nLimit, bool bIsImage, XADDR nModuleAddress,
                                             XBinary::PDSTRUCT *pPdStruct)
{
    QList<XBinary::FPART> listResult;

    if (!isReadableSeekableDevice(pDevice) || (nLimit < -1) || !XBinary::isPdStructNotCanceled(pPdStruct)) return listResult;
    DevicePositionGuard positionGuard(pDevice);

    XBinary *pBinary = XFormats::createClass(fileType, pDevice, bIsImage, nModuleAddress);
    listResult = pBinary->getFileParts(nFileParts, nLimit, pPdStruct);
    delete pBinary;

    if (!XBinary::isPdStructNotCanceled(pPdStruct)) listResult.clear();

    return listResult;
}

QList<XBinary::ARCHIVERECORD> XFormats::getArchiveRecords(XBinary::FT fileType, QIODevice *pDevice, qint32 nLimit, bool bIsImage, XADDR nModuleAddress,
                                                          XBinary::PDSTRUCT *pPdStruct)
{
    QList<XBinary::ARCHIVERECORD> listResult;

    if (!isReadableSeekableDevice(pDevice) || (nLimit < -1) || !XBinary::isPdStructNotCanceled(pPdStruct)) return listResult;
    DevicePositionGuard positionGuard(pDevice);

    XBinary *pBinary = XFormats::createClass(fileType, pDevice, bIsImage, nModuleAddress);
    listResult = pBinary->getArchiveRecords(nLimit, pPdStruct);
    delete pBinary;

    if (!XBinary::isPdStructNotCanceled(pPdStruct)) listResult.clear();

    return listResult;
}

// qint32 XFormats::getDataRecordValues(XBinary::FT fileType, QIODevice *pDevice, const XBinary::DATA_RECORDS_OPTIONS &dataRecordsOptions,
//                                      QList<XBinary::DATA_RECORD_ROW> *pListDataRecords, QList<QString> *pListTitles, bool bIsImage, XADDR nModuleAddress,
//                                      XBinary::PDSTRUCT *pPdStruct)
// {
//     qint32 nResult = 0;

//     XBinary *pBinary = XFormats::createClass(fileType, pDevice, bIsImage, nModuleAddress);
//     nResult = pBinary->getDataRecordValues(dataRecordsOptions, pListDataRecords, pListTitles, pPdStruct);
//     delete pBinary;

//     return nResult;
// }

QString XFormats::getFileFormatExtsString(XBinary::FT fileType)
{
    QString sResult;

    XBinary *pBinary = XFormats::createClass(fileType, nullptr);
    sResult = pBinary->getFileFormatExtsString();
    delete pBinary;

    return sResult;
}

bool XFormats::isArchive(XBinary::FT fileType)
{
    bool bResult = false;

    XBinary *pBinary = XFormats::createClass(fileType, nullptr);
    bResult = pBinary->isArchive();
    delete pBinary;

    return bResult;
}

bool XFormats::isStaticUnpacker(XBinary::FT fileType)
{
    // The original PE/CFBF handle-method types and the appended family-specific
    // SFX identities each occupy a contiguous range. Generic UPX predates those
    // ranges and is the non-PE counterpart used for ELF, Mach-O, and DOS streams.
    return (fileType == XBinary::FT_UPX) || (fileType == XBinary::FT_RIB) || (fileType == XBinary::FT_ISCAB) || (fileType == XBinary::FT_ELF32_SFX) ||
           (fileType == XBinary::FT_ELF64_SFX) || ((fileType >= XBinary::FT_PE32_ZIPSFX) && (fileType <= XBinary::FT_ELF64_ZPAQSFX)) ||
           ((fileType >= XBinary::FT_PE32_7ZSFX) && (fileType <= XBinary::FT_PE64_WIXBURN));
}

bool XFormats::isArchive(const QString &sFileName)
{
    bool bResult = false;

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        bResult = isArchive(XFormats::getPrefFileType(&file, XBinary::FT_FLAG_ARCHIVES));

        file.close();
    }

    return bResult;
}

bool XFormats::isExecutable(XBinary::FT fileType)
{
    bool bResult = false;

    XBinary *pBinary = XFormats::createClass(fileType, nullptr);
    bResult = pBinary->isExecutable();
    delete pBinary;

    return bResult;
}

QString XFormats::getXFHeaderStructName(const XBinary::XFHEADER &header)
{
    QString sResult;

    XBinary *pBinary = XFormats::createClass(header.fileType, nullptr);

    if (pBinary) {
        sResult = pBinary->structIDToString(header.structID);
        delete pBinary;
    }

    return sResult;
}

bool XFormats::isXFStruct(const QString &sStruct)
{
    XBinary::XFTYPE xfType = getXFTypeFromStructString(sStruct);

    return ((xfType == XBinary::XFTYPE_TABLE) || (xfType == XBinary::XFTYPE_HEADER));
}

bool XFormats::isHFCommand(const QString &sStruct)
{
    QString sCurrentStructSegment = getCurrentStructSegment(sStruct);

    return (sCurrentStructSegment.startsWith("!") || (normalizeStructToken(sCurrentStructSegment) == "COMMAND") ||
            (getXFTypeFromStructString(sStruct) == XBinary::XFTYPE_COMMAND));
}

XBinary::XFHEADER XFormats::getXFHeaderFromStructName(QIODevice *pDevice, const QString &sStruct, bool bIsImage, XADDR nModuleAddress, XBinary::PDSTRUCT *pPdStruct)
{
    XBinary::XFHEADER result = {};

    if (!isReadableSeekableDevice(pDevice) || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return result;
    }

    DevicePositionGuard positionGuard(pDevice);

    // sStruct is in xfHeaderToString format: [PARENTSTRING#][FILETYPE::]STRUCTNAME[?params]
    // Strip parent prefix (everything up to and including the last '#')
    QString sFiltered = sStruct;
    qint32 nHashIdx = sFiltered.lastIndexOf('#');
    if (nHashIdx != -1) {
        sFiltered = sFiltered.mid(nHashIdx + 1);
    }
    QString sInputNoParams = sFiltered;

    // Parse params before stripping them
    XBinary::XLOC xLoc = XBinary::offsetToLoc(0);
    XBinary::XFTYPE xfType = XBinary::XFTYPE_UNKNOWN;
    qint64 nSize = 0;
    qint32 nCount = 0;
    qint32 nParamIdx = sFiltered.indexOf('?');
    if (nParamIdx != -1) {
        const QString sParams = sFiltered.mid(nParamIdx + 1);
        sFiltered = sFiltered.left(nParamIdx);

        for (const QString &sParam : splitAndTrim(sParams, "&")) {
            qint32 nEqIdx = sParam.indexOf('=');
            if (nEqIdx != -1) {
                const QString sKey = normalizeStructToken(sParam.left(nEqIdx));
                const QString sValue = sParam.mid(nEqIdx + 1).trimmed();
                if (sKey == "OFFSET") {
                    bool bOk = false;
                    qint64 nOffset = parseXFOffset(sValue, &bOk);
                    if (bOk && (nOffset >= 0)) {
                        xLoc = XBinary::offsetToLoc(nOffset);
                    }
                } else if (sKey == "TYPE") {
                    const QString sUpper = normalizeStructToken(sValue);
                    if (sUpper == "HEADER") {
                        xfType = XBinary::XFTYPE_HEADER;
                    } else if (sUpper == "TABLE") {
                        xfType = XBinary::XFTYPE_TABLE;
                    } else if (sUpper == "COMMAND") {
                        xfType = XBinary::XFTYPE_COMMAND;
                    }
                } else if (sKey == "SIZE") {
                    bool bOk = false;
                    qint64 _nSize = parseXFOffset(sValue, &bOk);
                    if (bOk && (_nSize > 0)) {
                        nSize = _nSize;
                    }
                } else if (sKey == "ROWS") {
                    bool bOk = false;
                    qint64 nRows = parseXFOffset(sValue, &bOk);
                    if (bOk && (nRows >= 0) && (nRows <= INT32_MAX)) {
                        nCount = static_cast<qint32>(nRows);
                    }
                }
            }
        }
    }

    {
        QStringList listDisplayParts = splitAndTrim(sFiltered, "::");
        if ((listDisplayParts.size() == 2) && (listDisplayParts.at(0).contains(" ") || listDisplayParts.at(0).contains('\t'))) {
            sFiltered = listDisplayParts.at(1);
        }
    }

    // Try to extract optional FILETYPE:: prefix
    XBinary::FT fileType = XBinary::FT_UNKNOWN;
    QString sStructName = sFiltered;

    QStringList listStructParts = splitAndTrim(sFiltered, "::");
    if (listStructParts.size() >= 2) {
        QString sFileType = normalizeStructToken(listStructParts.at(0));
        if (XBinary::ftStringToFileTypeId(sFileType) != XBinary::FT_UNKNOWN) {
            fileType = XBinary::ftStringToFileTypeId(sFileType);
            sStructName = listStructParts.mid(1).join("::");
        }
    }
    if (fileType == XBinary::FT_UNKNOWN) {
        if (!sInputNoParams.contains("::") && (nParamIdx > 0)) {
            const QString sMaybeFileType = normalizeStructToken(sInputNoParams.left(nParamIdx));
            const XBinary::FT nMaybeFileType = XBinary::ftStringToFileTypeId(sMaybeFileType);
            if (nMaybeFileType != XBinary::FT_UNKNOWN) {
                fileType = nMaybeFileType;
                sStructName.clear();
            }
        }
    }
    sStructName = sStructName.trimmed();
    if (sStructName.startsWith("?")) {
        sStructName.clear();
    }

    // Fall back to device detection if no file type in string
    if (fileType == XBinary::FT_UNKNOWN) {
        QSet<XBinary::FT> stFileTypes = XFormats::getFileTypes(pDevice, XBinary::FT_FLAG_EXECUTABLES, pPdStruct);
        fileType = XBinary::_getPrefFileType(&stFileTypes);
    }

    XBinary *pBinary = XFormats::createClass(fileType, pDevice, bIsImage, nModuleAddress);

    if (pBinary) {
        XBinary::_MEMORY_MAP memoryMap = pBinary->getMemoryMap(XBinary::MAPMODE_UNKNOWN, pPdStruct);

        XBinary::XFSTRUCT xfStruct = {};
        xfStruct.fileType = fileType;
        xfStruct.nStructID = pBinary->ftStringToStructID(sStructName);
        xfStruct.pMemoryMap = &memoryMap;
        xfStruct.xLoc = xLoc;
        xfStruct.xfType = xfType;

        if ((xfType == XBinary::XFTYPE_TABLE) && (nSize > 0) && (nCount > 0)) {
            // xfHeaderToString() serializes a table's total byte size. Always
            // restore the per-row stride, including the ambiguous case where
            // total size happens to equal the structure's default row size.
            if ((nSize % nCount) == 0) {
                nSize /= nCount;
            }
        }

        xfStruct.nSize = nSize;
        xfStruct.nCount = nCount;

        QList<XBinary::XFHEADER> listResult = pBinary->getXFHeaders(xfStruct, pPdStruct);

        if (listResult.size() > 0) {
            result = listResult.last();
        }

        delete pBinary;
    }

    if (!XBinary::isPdStructNotCanceled(pPdStruct)) result = {};
    return result;
}
QSet<XBinary::FT> XFormats::getFileTypes(QIODevice *pDevice, XArchive::RECORD *pRecord, quint32 nFTFlags)
{
    Q_UNUSED(pDevice)
    Q_UNUSED(pRecord)
    Q_UNUSED(nFTFlags)

    QSet<XBinary::FT> stResult;

    // QByteArray baData = XArchives::decompress(pDevice, pRecord, 0, 0, 0x200);

    // stResult = XFormats::getFileTypes(&baData, nFTFlags);

    return stResult;
}
QSet<XBinary::FT> XFormats::getFileTypesZIP(QIODevice *pDevice, QList<XArchive::RECORD> *pListRecords, XBinary::PDSTRUCT *pPdStruct)
{
    QSet<XBinary::FT> stResult;

    // XBinary::FT fileType = XZip::_getFileType(pDevice, pListRecords, true);
    stResult.insert(XBinary::FT_ZIP);

    if (XAPKS::isValid(pDevice, pListRecords, pPdStruct)) {
        stResult.insert(XBinary::FT_APKS);
    } else if (XAPK::isValid(pDevice, pListRecords, pPdStruct)) {
        stResult.insert(XBinary::FT_APK);
    } else if (XIPA::isValid(pDevice, pListRecords, pPdStruct)) {
        stResult.insert(XBinary::FT_IPA);
    } else if (XJAR::isValid(pDevice, pListRecords, pPdStruct)) {
        stResult.insert(XBinary::FT_JAR);
    }

    return stResult;
}
QSet<XBinary::FT> XFormats::getFileTypesZIP(QIODevice *pDevice, XBinary::PDSTRUCT *pPdStruct)
{
    QSet<XBinary::FT> stResult;

    XZip xzip(pDevice);

    if (xzip.isValid(pPdStruct)) {
        QList<XArchive::RECORD> listArchiveRecords = xzip.getRecords(20000, pPdStruct);
        stResult += getFileTypesZIP(pDevice, &listArchiveRecords, pPdStruct);
    }

    // XBinary::FT fileType = XZip::_getFileType(pDevice, pListRecords, true);
    return stResult;
}
#ifdef USE_ARCHIVE
QSet<XBinary::FT> XFormats::getFileTypesTGZ(QIODevice *pDevice, QList<XArchive::RECORD> *pListRecords, XBinary::PDSTRUCT *pPdStruct)
{
    QSet<XBinary::FT> stResult;

    stResult.insert(XBinary::FT_TAR_GZ);

    if (XNPM::isValid(pDevice, pListRecords, pPdStruct)) {
        stResult.insert(XBinary::FT_NPM);
    }

    return stResult;
}
#endif
#ifdef USE_ARCHIVE
QSet<XBinary::FT> XFormats::getFileTypesTBZIP2(QIODevice *pDevice, QList<XArchive::RECORD> *pListRecords, XBinary::PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pDevice)
    Q_UNUSED(pListRecords)
    Q_UNUSED(pPdStruct)

    QSet<XBinary::FT> stResult;

    // if (pListRecords->count()) {
    //     const XArchive::RECORD &record = pListRecords->at(0);
    //     QByteArray baData = XArchives::decompress(pDevice, &record, nullptr, 0, 0x200);

    //     QSet<XBinary::FT> stFileTypes = getFileTypes(&baData, XBinary::FT_FLAG_FORMATS);

    //     if (stFileTypes.contains(XBinary::FT_TAR)) {
    //         stResult.insert(XBinary::FT_TAR_BZIP2);
    //     }
    // }

    return stResult;
}
#endif
#ifdef USE_ARCHIVE
QSet<XBinary::FT> XFormats::getFileTypesTXZ(QIODevice *pDevice, QList<XArchive::RECORD> *pListRecords, XBinary::PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pDevice)
    Q_UNUSED(pListRecords)
    Q_UNUSED(pPdStruct)

    QSet<XBinary::FT> stResult;

    // if (pListRecords->count()) {
    //     const XArchive::RECORD &record = pListRecords->at(0);
    //     QByteArray baData = XArchives::decompress(pDevice, &record, nullptr, 0, 0x200);

    //     QSet<XBinary::FT> stFileTypes = getFileTypes(&baData, XBinary::FT_FLAG_FORMATS);

    //     if (stFileTypes.contains(XBinary::FT_TAR)) {
    //         stResult.insert(XBinary::FT_TAR_XZ);
    //     }
    // }

    return stResult;
}
#endif
#ifdef USE_ARCHIVE
QSet<XBinary::FT> XFormats::getFileTypesTLZIP(QIODevice *pDevice, QList<XArchive::RECORD> *pListRecords, XBinary::PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pDevice)
    Q_UNUSED(pListRecords)
    Q_UNUSED(pPdStruct)

    QSet<XBinary::FT> stResult;

    // if (pListRecords->count()) {
    //     const XArchive::RECORD &record = pListRecords->at(0);
    //     QByteArray baData = XArchives::decompress(pDevice, &record, nullptr, 0, 0x200);

    //     QSet<XBinary::FT> stFileTypes = getFileTypes(&baData, XBinary::FT_FLAG_FORMATS);

    //     if (stFileTypes.contains(XBinary::FT_TAR)) {
    //         stResult.insert(XBinary::FT_TAR_LZIP);
    //     }
    // }

    return stResult;
}
#endif
#ifdef USE_ARCHIVE
QSet<XBinary::FT> XFormats::getFileTypesAR(QIODevice *pDevice, QList<XArchive::RECORD> *pListRecords, XBinary::PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pDevice)  // TODO

    QSet<XBinary::FT> stResult;

    stResult.insert(XBinary::FT_AR);

    if (XDEB::isValid(pListRecords, pPdStruct)) {
        stResult.insert(XBinary::FT_DEB);
    }

    return stResult;
}
#endif
#ifdef USE_ARCHIVE
QSet<XBinary::FT> XFormats::getFileTypesGZIP(QIODevice *pDevice, QList<XArchive::RECORD> *pListRecords, XBinary::PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pDevice)
    Q_UNUSED(pListRecords)
    Q_UNUSED(pPdStruct)

    QSet<XBinary::FT> stResult;

    // if (pListRecords->count()) {
    //     const XArchive::RECORD &record = pListRecords->at(0);
    //     QByteArray baData = XArchives::decompress(pDevice, &record, pPdStruct, 0, 0x200);

    //     QSet<XBinary::FT> _ft = getFileTypes(&baData, XBinary::FT_FLAG_FORMATS);

    //     if (_ft.contains(XBinary::FT_TAR)) {
    //         XTAR_GZ xtarGz(pDevice);
    //         QList<XArchive::RECORD> listArchiveRecords = xtarGz.getRecords(20000, pPdStruct);

    //         stResult += getFileTypesTGZ(pDevice, &listArchiveRecords, pPdStruct);
    //     }
    // }

    return stResult;
}
#endif
#ifdef USE_ARCHIVE
QSet<XBinary::FT> XFormats::getFileTypesBZIP2(QIODevice *pDevice, QList<XArchive::RECORD> *pListRecords, XBinary::PDSTRUCT *pPdStruct)
{
    QSet<XBinary::FT> stResult;

    stResult += getFileTypesTBZIP2(pDevice, pListRecords, pPdStruct);

    return stResult;
}
#endif
#ifdef USE_ARCHIVE
QSet<XBinary::FT> XFormats::getFileTypesXZ(QIODevice *pDevice, QList<XArchive::RECORD> *pListRecords, XBinary::PDSTRUCT *pPdStruct)
{
    QSet<XBinary::FT> stResult;

    stResult += getFileTypesTXZ(pDevice, pListRecords, pPdStruct);

    return stResult;
}
#endif
#ifdef USE_ARCHIVE
QSet<XBinary::FT> XFormats::getFileTypesLZIP(QIODevice *pDevice, QList<XArchive::RECORD> *pListRecords, XBinary::PDSTRUCT *pPdStruct)
{
    Q_UNUSED(pDevice)
    Q_UNUSED(pListRecords)
    Q_UNUSED(pPdStruct)

    QSet<XBinary::FT> stResult;

    // stResult += getFileTypesTLZIP(pDevice, pListRecords, pPdStruct);

    return stResult;
}
#endif

QSet<XBinary::FT> XFormats::_getFileTypes(QIODevice *pDevice, quint32 nFTFlags, XBinary::PDSTRUCT *pPdStruct)
{
#ifdef QT_DEBUG
    qDebug("XFormats::_getFileTypes(): Timer started");
    QElapsedTimer timer;
    timer.start();
#endif
    QSet<XBinary::FT> stResult;

    if (!isReadableSeekableDevice(pDevice) || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return stResult;
    }

    QString sFileTypes = pDevice->property("filetypes").toString();
    QSet<XBinary::FT> stCachedFileTypes;
    const bool bCacheValid = parseCachedFileTypes(sFileTypes, &stCachedFileTypes);

    // A writable QIODevice can change without its dynamic properties changing.
    // Never let a stale annotation bypass detection in that case.  Malformed
    // annotations likewise fall back to real detection instead of poisoning it.
    if (pDevice->isWritable()) {
        sFileTypes.clear();
    } else if (!sFileTypes.isEmpty() && !bCacheValid) {
        sFileTypes.clear();
    } else if (nFTFlags != XBinary::FT_FLAG_FORMATS) {
        // The external annotation has no flag-mask metadata. Re-detect for a
        // narrowed request so categories outside the requested mask cannot leak.
        sFileTypes.clear();
    }

    if (sFileTypes.isEmpty()) {
        // No cached file types, proceed with detection
        stResult.insert(XBinary::FT_BINARY);

        if ((nFTFlags & (XBinary::FT_FLAG_EXECUTABLES | XBinary::FT_FLAG_ARCHIVES)) && XMSDOS::isValid(pDevice, false, (XADDR)-1, pPdStruct)) {
            if (nFTFlags & XBinary::FT_FLAG_EXECUTABLES) {
                stResult.insert(XBinary::FT_MSDOS);

                if (XPE::isValid(pDevice, false, (XADDR)-1, pPdStruct)) {
                    stResult.insert(XBinary::FT_PE);

                    XPE xpe(pDevice, false, (XADDR)-1);

                    if (xpe.isValid(pPdStruct)) {
                        stResult.insert(xpe.getFileType());
                    }
                } else if (XNE::isValid(pDevice, false, (XADDR)-1, pPdStruct)) {
                    stResult.insert(XBinary::FT_NE);
                } else if (XLE::isValid(pDevice, false, (XADDR)-1, pPdStruct)) {
                    stResult.insert(XBinary::FT_LE);

                    XLE xle(pDevice, false, (XADDR)-1);

                    if (xle.isValid(pPdStruct)) {
                        stResult.insert(xle.getFileType());
                    }
                }
            }
#ifdef USE_ARCHIVE
            if ((nFTFlags & (XBinary::FT_FLAG_EXECUTABLES | XBinary::FT_FLAG_ARCHIVES)) && XDOS16::isValid(pDevice, pPdStruct)) {
                XDOS16 xdos16(pDevice);
                if (xdos16.isValid(pPdStruct)) {
                    if (nFTFlags & XBinary::FT_FLAG_ARCHIVES) {
                        stResult.insert(XBinary::FT_ARCHIVE);
                    }
                    stResult.insert(xdos16.getFileType());
                }
            }
#endif
        } else if ((nFTFlags & XBinary::FT_FLAG_EXECUTABLES) && XELF::isValid(pDevice, false, (XADDR)-1, pPdStruct)) {
            stResult.insert(XBinary::FT_ELF);

            XELF xelf(pDevice, false, (XADDR)-1);

            if (xelf.isValid(pPdStruct)) {
                stResult.insert(xelf.getFileType());
            }
        } else if ((nFTFlags & XBinary::FT_FLAG_EXECUTABLES) && XMACH::isValid(pDevice, false, (XADDR)-1, pPdStruct)) {
            stResult.insert(XBinary::FT_MACHO);

            XMACH xmach(pDevice);

            if (xmach.isValid(pPdStruct)) {
                stResult.insert(xmach.getFileType());
            }
        }

        if (!XBinary::isPdStructNotCanceled(pPdStruct)) return {};

        if ((nFTFlags & XBinary::FT_FLAG_EXECUTABLES) && (stResult.size() <= 1)) {
            if (XAmigaHunk::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_AMIGAHUNK);
            }
        }

        if ((nFTFlags & XBinary::FT_FLAG_EXECUTABLES) && (stResult.size() <= 1)) {
            if (XAtariST::isValid(pDevice, false, (XADDR)-1, pPdStruct)) {
                stResult.insert(XBinary::FT_ATARIST);
            }
        }

#ifdef USE_PDF
        if ((nFTFlags & XBinary::FT_FLAG_DOCUMENTS) && (stResult.size() <= 1)) {
            if (XPDF::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_PDF);
            }
        }
#endif
#ifdef USE_PDB
        if ((nFTFlags & XBinary::FT_FLAG_EXECUTABLES) && (stResult.size() <= 1)) {
            if (XPDB::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_PDB);
            }
        }
#endif
#ifdef USE_DEX
        if ((nFTFlags & XBinary::FT_FLAG_EXECUTABLES) && (stResult.size() <= 1)) {
            if (XDEX::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_DEX);
            }
        }
#endif
#ifdef USE_ARCHIVE
        if ((nFTFlags & XBinary::FT_FLAG_EXECUTABLES) && !(nFTFlags & XBinary::FT_FLAG_ARCHIVES) && (stResult.size() <= 1) && XMACHOFat::isValid(pDevice, pPdStruct)) {
            stResult.insert(XBinary::FT_MACHOFAT);
        }
#endif

        // The Parsec driver modules are executable 8086 images used by the
        // audio subsystem. Their duplicated magic, entry-point table and
        // aligned NS footer make the structural validators unambiguous.
        if ((nFTFlags & (XBinary::FT_FLAG_EXECUTABLES | XBinary::FT_FLAG_AUDIO)) && (stResult.size() <= 1)) {
            XBinary::FT driverType = XBinary::FT_UNKNOWN;
            if (XDTC::isValid(pDevice, pPdStruct)) {
                driverType = XBinary::FT_DTC;
            } else if (XDMA::isValid(pDevice, pPdStruct)) {
                driverType = XBinary::FT_DMA;
            } else if (XMUS::isValid(pDevice, pPdStruct)) {
                driverType = XBinary::FT_MUS;
            } else if (XSND::isValid(pDevice, pPdStruct)) {
                driverType = XBinary::FT_SND;
            }

            if (driverType != XBinary::FT_UNKNOWN) {
                if (nFTFlags & XBinary::FT_FLAG_AUDIO) {
                    stResult.insert(XBinary::FT_AUDIO);
                }
                stResult.insert(driverType);
            }
        }

        // Parsec fixed-magic formats must be tested before the signatureless
        // offset-table archive. Otherwise a wrapped/list payload can be
        // classified by its outer structure instead of its specific format.
#ifdef USE_STATICUNPACKER
        if ((nFTFlags & (XBinary::FT_FLAG_ARCHIVES | XBinary::FT_FLAG_STATICUNPACKERS)) && (stResult.size() <= 1) && XRIB::isValid(pDevice, pPdStruct)) {
            if (nFTFlags & XBinary::FT_FLAG_ARCHIVES) {
                stResult.insert(XBinary::FT_ARCHIVE);
            }
            stResult.insert(XBinary::FT_RIB);
        }
#endif
#ifdef USE_ARCHIVE
        if ((nFTFlags & XBinary::FT_FLAG_ARCHIVES) && (stResult.size() <= 1) && XPMM::isValid(pDevice, pPdStruct)) {
            stResult.insert(XBinary::FT_ARCHIVE);
            stResult.insert(XBinary::FT_PMM);
        }
#endif
        if ((nFTFlags & XBinary::FT_FLAG_AUDIO) && (stResult.size() <= 1)) {
            XBinary::FT audioType = XBinary::FT_UNKNOWN;
            if (XPMA::isValid(pDevice, pPdStruct)) {
                audioType = XBinary::FT_PMA;
            } else if (XMDH::isValid(pDevice, pPdStruct)) {
                audioType = XBinary::FT_MDH;
            } else if (XSM8::isValid(pDevice, pPdStruct)) {
                audioType = XBinary::FT_SM8;
            }

            if (audioType != XBinary::FT_UNKNOWN) {
                stResult.insert(XBinary::FT_AUDIO);
                stResult.insert(audioType);
            }
        }

        if ((nFTFlags & XBinary::FT_FLAG_ARCHIVES) && (stResult.size() <= 1)) {
            if (XZip::isValid(pDevice, pPdStruct)) {
                XZip xzip(pDevice);
                if (xzip.isValid(pPdStruct)) {
                    stResult.insert(XBinary::FT_ARCHIVE);
                    QList<XArchive::RECORD> listArchiveRecords = xzip.getRecords(20000, pPdStruct);
                    stResult += getFileTypesZIP(pDevice, &listArchiveRecords, pPdStruct);
                }
            }
#ifdef USE_ARCHIVE
            else if (X_Ar::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_AR);
                if (XDEB::isValid(pDevice, pPdStruct)) {
                    stResult.insert(XBinary::FT_DEB);
                }
            } else if (XTAR_GZ::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                XTAR_GZ xtarGz(pDevice);
                QList<XArchive::RECORD> listArchiveRecords = xtarGz.getRecords(20000, pPdStruct);
                stResult += getFileTypesTGZ(pDevice, &listArchiveRecords, pPdStruct);
            } else if (XGzip::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_GZIP);
            } else if (XTAR_BZIP2::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_TAR_BZIP2);
            } else if (XBZIP2::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_BZIP2);
            } else if (XTAR_XZ::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_TAR_XZ);
            } else if (XXZ::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_XZ);
            } else if (XTAR_LZIP::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_TAR_LZIP);
            } else if (XLzip::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_LZIP);
            } else if (XTAR::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_TAR);
            } else if (XSevenZip::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_7Z);
            } else if (XWIM::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_WIM);
            } else if (XRar::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_RAR);
            }
        }

        if (!XBinary::isPdStructNotCanceled(pPdStruct)) return {};

        if ((nFTFlags & XBinary::FT_FLAG_ARCHIVES) && (stResult.size() <= 1)) {
            if (XParsecArchive::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_PARSEC_ARCHIVE);
            } else if (XCKP::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_CKP);
            } else if (XEDP::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_EDP);
            } else if (XMPQ::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_MPQ);
            } else if (XBIGF::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_BIGF);
            } else if (XPAK::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_QUAKE_PAK);
            } else if (XWAD::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_DOOM_WAD);
            } else if (XGRP::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_BUILD_GRP);
            } else if (XZPAQ::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_ZPAQ);
            } else if (XBCM::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_BCM);
            } else if (XLPAQ8::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_LPAQ8);
            } else if (XPEA::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_PEA);
            } else if (XZlib::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_ZLIB);
            } else if (XARX::isValid(pDevice, pPdStruct)) {
                // Probed before LHA: ARX reuses LHA's method tag but shifts
                // every field from offset 7 onward by one byte.
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_ARX);
            } else if (XSAR::isValid(pDevice, pPdStruct)) {
                // Probed before LHA: same container shape, only the
                // method tag distinguishes them.
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_SAR);
            } else if (XLHA::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_LHA);
            } else if (XCab::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_CAB);
            } else if (XTAR_LZ4::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_TAR_LZ4);
            } else if (XLZ4::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_LZ4);
            } else if (XLZ5::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_LZ5);
            } else if (XLizard::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_LIZARD);
            } else if (isBrotliDetectionCandidate(pDevice) && isValidBrotliArchive(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_BROTLI);
            } else if (XISO9660::isCueOrRawImage(pDevice, pPdStruct)) {
                // A headerless MODE2/2336 sector can satisfy the deliberately
                // permissive five-byte LZMA-alone header probe.  Raw/CUE ISO
                // detection authenticates a complete descriptor/root layout,
                // so let that stronger result win before trying LZMA.
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_ISO9660);
            } else if (XTAR_LZMA::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_TAR_LZMA);
            } else if (XLZMA::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_LZMA);
            } else if (XTAR_LZOP::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_TAR_LZOP);
            } else if (XLzo::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_LZO);
            } else if (XTAR_COMPRESS::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_TAR_Z);
            } else if (XCompressZ::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_COMPRESS);
            } else if (XCPIO::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_CPIO);
            } else if (XWARC::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_WARC);
            } else if (XMTree::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_MTREE);
            } else if (XUU::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_UU);
            } else if (XRPM::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_RPM);
            } else if (XKWAJ::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_KWAJ);
            } else if (XXAR::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_XAR);
            } else if (XZOO::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_ZOO);
            } else if (XMiniDump::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_MINIDUMP);
            } else if (XDMG::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_DMG);
            } else if (XWIM::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_WIM);
            } else if (XASAR::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_ASAR);
            } else if (XARJ::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_ARJ);
            } else if (XACE::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_ACE);
            } else if (XSEAARC::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_ARC);
            } else if (XSZDD::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_SZDD);
            } else if (XTAR_ZSTD::isValidPrefix(pDevice, pPdStruct)) {
                // Promote a Zstandard-wrapped TAR after a bounded probe of its
                // first authenticated TAR header.  This avoids materializing
                // the complete decompressed archive during generic detection.
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_TAR_ZSTD);
            } else if (XZstd::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_ZSTD);
            } else if (XISO9660::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_ISO9660);
            } else if (XUDF::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_UDF);
            } else if (XSquashfs::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_SQUASHFS);
            } else if (XMACHOFat::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_MACHOFAT);
            }

            if (XFREEARC::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_FREEARC);
            }
#endif
        }

#ifdef USE_ARCHIVE
        // Classic STK has no magic. Probe it only as a last resort after
        // signature-based archive detectors have found nothing specific.
        if ((nFTFlags & XBinary::FT_FLAG_ARCHIVES) && (stResult.size() <= 1)) {
            if (XStk::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_ARCHIVE);
                stResult.insert(XBinary::FT_STK);
            }
        }

        if (!XBinary::isPdStructNotCanceled(pPdStruct)) return {};
#endif

#ifdef USE_ARCHIVE
        if ((nFTFlags & (XBinary::FT_FLAG_DOCUMENTS | XBinary::FT_FLAG_ARCHIVES)) && (stResult.size() <= 1)) {
            if (XCFBF::isValid(pDevice, pPdStruct)) {
                if (nFTFlags & XBinary::FT_FLAG_ARCHIVES) {
                    stResult.insert(XBinary::FT_ARCHIVE);
                }
                stResult.insert(XBinary::FT_CFBF);
            }
        }
#endif

        if (!XBinary::isPdStructNotCanceled(pPdStruct)) return {};

        if ((nFTFlags & XBinary::FT_FLAG_IMAGES) && (stResult.size() <= 1)) {
            if (XPNG::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_IMAGE);
                stResult.insert(XBinary::FT_PNG);
            } else if (XJpeg::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_IMAGE);
                stResult.insert(XBinary::FT_JPEG);
            } else if (XGif::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_IMAGE);
                stResult.insert(XBinary::FT_GIF);
            } else if (XBMP::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_IMAGE);
                stResult.insert(XBinary::FT_BMP);
            } else if (XTiff::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_IMAGE);
                stResult.insert(XBinary::FT_TIFF);
            } else if (XIcon::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_IMAGE);
                XIcon xicon(pDevice);
                stResult.insert(xicon.getFileType());  // FT_ICO or FT_CUR
            } else if (XICC::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_IMAGE);
                stResult.insert(XBinary::FT_ICC);
            }
        }

        if (!XBinary::isPdStructNotCanceled(pPdStruct)) return {};

        if ((nFTFlags & (XBinary::FT_FLAG_IMAGES | XBinary::FT_FLAG_AUDIO | XBinary::FT_FLAG_VIDEO)) && (stResult.size() <= 1)) {
            if ((nFTFlags & XBinary::FT_FLAG_AUDIO) && XMP3::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_AUDIO);
                stResult.insert(XBinary::FT_MP3);
            } else if ((nFTFlags & XBinary::FT_FLAG_VIDEO) && XMP4::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_VIDEO);
                stResult.insert(XBinary::FT_MP4);
            } else if ((nFTFlags & XBinary::FT_FLAG_AUDIO) && XXM::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_AUDIO);
                stResult.insert(XBinary::FT_XM);
            } else if ((nFTFlags & XBinary::FT_FLAG_VIDEO) && XAVI::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_RIFF);
                stResult.insert(XBinary::FT_VIDEO);
                stResult.insert(XBinary::FT_AVI);
            } else if ((nFTFlags & XBinary::FT_FLAG_IMAGES) && XWEBP::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_RIFF);
                stResult.insert(XBinary::FT_IMAGE);
                stResult.insert(XBinary::FT_WEBP);
            } else if ((nFTFlags & XBinary::FT_FLAG_AUDIO) && XWAV::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_RIFF);
                stResult.insert(XBinary::FT_AUDIO);
                stResult.insert(XBinary::FT_WAV);
            } else if (((nFTFlags & (XBinary::FT_FLAG_IMAGES | XBinary::FT_FLAG_AUDIO | XBinary::FT_FLAG_VIDEO)) ==
                        (XBinary::FT_FLAG_IMAGES | XBinary::FT_FLAG_AUDIO | XBinary::FT_FLAG_VIDEO)) &&
                       XRiff::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_RIFF);
            }
        }

        if (!XBinary::isPdStructNotCanceled(pPdStruct)) return {};

        if ((nFTFlags & (XBinary::FT_FLAG_EXECUTABLES | XBinary::FT_FLAG_DOCUMENTS)) && (stResult.size() <= 1)) {
#ifdef USE_DEX
            if ((nFTFlags & XBinary::FT_FLAG_DOCUMENTS) && XAndroidBinary::isValid(pDevice, pPdStruct)) {
                XAndroidBinary xandroid(pDevice);
                stResult.insert(xandroid.getFileType());  // FT_ANDROIDXML or FT_ANDROIDASRC
            } else
#endif
                // if (XDER::isValid(pDevice, pPdStruct)) {
                //     stResult.insert(XBinary::FT_DOCUMENT);
                //     stResult.insert(XBinary::FT_DER);
                // } else

                if ((nFTFlags & XBinary::FT_FLAG_EXECUTABLES) && XJavaClass::isValid(pDevice, pPdStruct)) {
                    stResult.insert(XBinary::FT_JAVACLASS);
                } else if ((nFTFlags & XBinary::FT_FLAG_EXECUTABLES) && XPYC::isValid(pDevice, pPdStruct)) {
                    stResult.insert(XBinary::FT_PYC);
                } else if ((nFTFlags & XBinary::FT_FLAG_EXECUTABLES) && XWASM::isValid(pDevice, pPdStruct)) {
                    stResult.insert(XBinary::FT_WASM);
                } else if ((nFTFlags & XBinary::FT_FLAG_DOCUMENTS) && XTTF::isValid(pDevice, pPdStruct)) {
                    stResult.insert(XBinary::FT_TTF);
                } else if ((nFTFlags & XBinary::FT_FLAG_DOCUMENTS) && XDJVU::isValid(pDevice, pPdStruct)) {
                    stResult.insert(XBinary::FT_DJVU);
                }
        }

        if (!XBinary::isPdStructNotCanceled(pPdStruct)) return {};

        if (nFTFlags & XBinary::FT_FLAG_TEXT) {
            if (XText::isValid(pDevice, pPdStruct)) {
                stResult.insert(XBinary::FT_TEXT);
                XText xtext(pDevice);
                XText::TEXT_TYPE textType = xtext.detectTextType(pPdStruct);

                if (textType == XText::TEXT_TYPE_UTF8 || textType == XText::TEXT_TYPE_UTF8_BOM) {
                    stResult.insert(XBinary::FT_UTF8);
                } else if (textType == XText::TEXT_TYPE_UTF16_LE || textType == XText::TEXT_TYPE_UTF32_LE) {
                    stResult.insert(XBinary::FT_UNICODE);
                    stResult.insert(XBinary::FT_UNICODE_LE);
                } else if (textType == XText::TEXT_TYPE_UTF16_BE || textType == XText::TEXT_TYPE_UTF32_BE) {
                    stResult.insert(XBinary::FT_UNICODE);
                    stResult.insert(XBinary::FT_UNICODE_BE);
                } else {
                    stResult.insert(XBinary::FT_PLAINTEXT);
                }

                // Fix: GIF can trigger text detection
                if (stResult.contains(XBinary::FT_GIF) && stResult.contains(XBinary::FT_TEXT)) {
                    stResult.remove(XBinary::FT_GIF);
                }
            }
        }

        if (!XBinary::isPdStructNotCanceled(pPdStruct)) return {};

        if ((nFTFlags & XBinary::FT_FLAG_EXECUTABLES) &&
            ((stResult.size() <= 1) || (stResult.contains(XBinary::FT_PLAINTEXT)) || (XBinary::getDeviceFileSuffix(pDevice).toUpper() == "COM"))) {
            if (XCOM::isValid(pDevice, false, (XADDR)-1, pPdStruct)) {
                if (XBinary::getDeviceFileSuffix(pDevice).toUpper() == "COM") {
                    stResult.insert(XBinary::FT_COM);
                }
            }
        }

#ifdef USE_STATICUNPACKER
        // Packer/protector/installer (XStaticUnpacker) file types. These are only
        // produced when the caller explicitly asks for them (off by default),
        // because they require probing every XStaticUnpacker class. PE-container
        // packers are only probed when a PE was detected; the original 16-bit
        // Inno Setup loader is the one supported NE-container exception. MSI/WiX
        // are only probed when a CFBF (OLE compound) container was detected.
        if ((nFTFlags & XBinary::FT_FLAG_STATICUNPACKERS) && XBinary::isPdStructNotCanceled(pPdStruct)) {
            {
                XISCab x(pDevice);
                if (x.isValid(pPdStruct)) stResult.insert(XBinary::FT_ISCAB);
            }
            if (stResult.contains(XBinary::FT_PE)) {
                {
                    XBurn x(pDevice);
                    if (x.isValid(pPdStruct)) stResult.insert(x.getFileType());
                }
                {
                    XSevenZipSFX x(pDevice);
                    if (x.isValid(pPdStruct)) stResult.insert(x.getFileType());
                }
                {
                    XActualInstaller x(pDevice);
                    if (x.isValid(pPdStruct)) stResult.insert(x.getFileType());
                }
                {
                    XAdvancedInstaller x(pDevice);
                    if (x.isValid(pPdStruct)) stResult.insert(x.getFileType());
                }
                {
                    XASPACK x(pDevice);
                    if (x.isValid(pPdStruct)) stResult.insert(x.getFileType());
                }
                {
                    XAUTOIT x(pDevice);
                    if (x.isValid(pPdStruct)) stResult.insert(x.getFileType());
                }
                {
                    XBoxedApp x(pDevice);
                    if (x.isValid(pPdStruct)) stResult.insert(x.getFileType());
                }
                {
                    XClickteam x(pDevice);
                    if (x.isValid(pPdStruct)) stResult.insert(x.getFileType());
                }
                {
                    XCreateInstall x(pDevice);
                    if (x.isValid(pPdStruct)) stResult.insert(x.getFileType());
                }
                {
                    XEnigmaVB x(pDevice);
                    if (x.isValid(pPdStruct)) stResult.insert(x.getFileType());
                }
                {
                    XFSG x(pDevice);
                    if (x.isValid(pPdStruct)) stResult.insert(x.getFileType());
                }
                {
                    XIExpress x(pDevice);
                    if (x.isValid(pPdStruct)) stResult.insert(x.getFileType());
                }
                {
                    XInnoSetup x(pDevice);
                    if (x.isValid(pPdStruct)) stResult.insert(x.getFileType());
                }
                {
                    XInstallForge x(pDevice);
                    if (x.isValid(pPdStruct)) stResult.insert(x.getFileType());
                }
#ifdef USE_XEMULATOR
                {
                    XInstallSimple x(pDevice);
                    if (x.isValid(pPdStruct)) stResult.insert(x.getFileType());
                }
#endif
                {
                    XMEW x(pDevice);
                    if (x.isValid(pPdStruct)) stResult.insert(x.getFileType());
                }
                {
                    XNSIS x(pDevice);
                    if (x.isValid(pPdStruct)) stResult.insert(x.getFileType());
                }
                {
                    XNSPACK x(pDevice);
                    if (x.isValid(pPdStruct)) stResult.insert(x.getFileType());
                }
                {
                    XPETITE x(pDevice);
                    if (x.isValid(pPdStruct)) stResult.insert(x.getFileType());
                }
                {
                    XSFX x(pDevice);
                    if (x.isValid(pPdStruct)) stResult.insert(x.getFileType());
                }
                {
                    XSmartInstall x(pDevice);
                    if (x.isValid(pPdStruct)) stResult.insert(x.getFileType());
                }
                {
                    XTarma x(pDevice);
                    if (x.isValid(pPdStruct)) stResult.insert(x.getFileType());
                }
                {
                    XUPX x(pDevice);
                    if (x.isValid(pPdStruct)) stResult.insert(x.getFileType());
                }
                {
                    XRarSFX x(pDevice, false, -1, true);
                    if (x.isValid(pPdStruct)) stResult.insert(x.getFileType());
                }
                {
                    XYODA x(pDevice);
                    if (x.isValid(pPdStruct)) stResult.insert(x.getFileType());
                }
            }
            const QString sExecutableSuffix = XBinary::getDeviceFileSuffix(pDevice).toUpper();
            if (!stResult.contains(XBinary::FT_PE) &&
                (stResult.contains(XBinary::FT_ELF) || stResult.contains(XBinary::FT_MACHO) || stResult.contains(XBinary::FT_MACHOFAT) ||
                 stResult.contains(XBinary::FT_MSDOS) || stResult.contains(XBinary::FT_COM) || (sExecutableSuffix == QStringLiteral("SYS")))) {
                // Generic SFX supports ELF wrappers whose archive lives in the
                // exact unmapped suffix. Probe it before UPX so an UPX-packed
                // extraction stub is classified by its archive payload.
                if (stResult.contains(XBinary::FT_ELF)) {
                    XSFX x(pDevice);
                    if (x.isValid(pPdStruct)) stResult.insert(x.getFileType());
                }
                // XUPX has native rebuilders for these executable families.
                // Keep the generic FT_UPX identity so it wins preference over
                // the enclosing executable and reaches the streaming factory.
                // Raw DOS SYS images do not have a container signature, so the
                // suffix is their only inexpensive entry to XUPX's structurally
                // validated DOS pack-header detector.
                XUPX x(pDevice);
                if (x.isValid(pPdStruct)) stResult.insert(x.getFileType());
            }
            if (stResult.contains(XBinary::FT_NE)) {
                {
                    XInnoSetup x(pDevice);
                    if (x.isValid(pPdStruct)) stResult.insert(x.getFileType());
                }
            }
            if (stResult.contains(XBinary::FT_CFBF)) {
                {
                    XWiX x(pDevice);
                    if (x.isValid(pPdStruct)) stResult.insert(x.getFileType());
                }
                {
                    XMSI x(pDevice);
                    if (x.isValid(pPdStruct)) stResult.insert(x.getFileType());
                }
            }
        }
#endif
    } else {
        // Cached file types available, return them
        stResult = stCachedFileTypes;
    }

    if (!XBinary::isPdStructNotCanceled(pPdStruct)) {
        stResult.clear();
    }

#ifdef QT_DEBUG
    qint64 nElapsed = timer.elapsed();
    qDebug("XFormats::_getFileTypes(): time=%lld ms", nElapsed);  // Calculated in milisecounds
    qDebug("XFormats::_getFileTypes(): Timer stopped");
#endif

    return stResult;
}

void XFormats::setData(MODE mode, XBinary::FT fileFormat, QIODevice *pDevice, QString sFolderName, XBinary::PDSTRUCT *pPdStruct)
{
    m_mode = mode;
    m_fileFormat = fileFormat;
    m_pDevice = pDevice;
    m_sFolderName = sFolderName;
    m_pPdStruct = pPdStruct;
}

void XFormats::process()
{
    XBinary::PDSTRUCT pdStructEmpty = XBinary::createPdStruct();
    XBinary::PDSTRUCT *pPdStruct = m_pPdStruct;
    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }
    const XBinary::PDSTRUCTLIFETIME progressLifetime = XBinary::retainPdStructLifetime(pPdStruct);
    if (!progressLifetime.isValid()) return;

    if (m_mode == MODE_UNPACKDEVICETOFOLDER) {
        if (m_pDevice) {
            if (!unpackDeviceToFolder(m_fileFormat, m_pDevice, m_sFolderName, pPdStruct)) {
                if (XBinary::isPdStructLifetimeAlive(progressLifetime) && XBinary::isPdStructNotCanceled(pPdStruct)) {
                    emit errorMessage(tr("Cannot unpack"));
                }
            }
        }
    }
}

bool XFormats::unpackDeviceToFolder(XBinary::FT fileType, QIODevice *pDevice, QString sFolderName, XBinary::PDSTRUCT *pPdStruct)
{
    XBinary::PDSTRUCT pdStructEmpty = XBinary::createPdStruct();
    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }
    const XBinary::PDSTRUCTLIFETIME progressLifetime = XBinary::retainPdStructLifetime(pPdStruct);
    if (!progressLifetime.isValid()) return false;

    if (!isReadableSeekableDevice(pDevice) || sFolderName.isEmpty() || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    if (fileType == XBinary::FT_UNKNOWN) {
        fileType = XFormats::getPrefFileType(pDevice, XBinary::FT_FLAG_FORMATS, pPdStruct);
        if (!XBinary::isPdStructLifetimeAlive(progressLifetime)) return false;
    }

    if ((fileType == XBinary::FT_UNKNOWN) || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return false;
    }

    bool bResult = false;

    bool bDirectory = XBinary::isDirectoryExists(sFolderName);

    if (!bDirectory) {
        bDirectory = XBinary::createDirectory(sFolderName);
    }

    if (bDirectory) {
        XBinary *pBinary = createClass(fileType, pDevice);

        if (pBinary) {
            QMap<XBinary::UNPACK_PROP, QVariant> mapProperties;
            bResult = pBinary->unpackToFolder(sFolderName, mapProperties, pPdStruct);
            // unpackToFolder() linearizes success at its transaction commit.
            // A progress owner disappearing strictly after that boundary must
            // not turn committed output into a reported failure.
            delete pBinary;
        }
    }

    return bResult;
}

bool XFormats::extractArchiveRecordsToFolder(QList<XBinary::ARCHIVERECORD> *pListRecords, QIODevice *pDevice, QString sFolderName, XBinary::PDSTRUCT *pPdStruct)
{
#ifdef QT_DEBUG
    qDebug("XFormats::extractArchiveRecordsToFolder: Starting extraction to %s", sFolderName.toLatin1().data());
#endif
    bool bResult = false;

    XBinary::PDSTRUCT pdStructEmpty = XBinary::createPdStruct();
    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }
    const XBinary::PDSTRUCTLIFETIME progressLifetime = XBinary::retainPdStructLifetime(pPdStruct);
    bool bProgressOwnerAlive = progressLifetime.isValid();
    if (!bProgressOwnerAlive) return false;

    if (!pListRecords || !isReadableSeekableDevice(pDevice) || sFolderName.isEmpty() || !XBinary::isPdStructNotCanceled(pPdStruct)) {
        return bResult;
    }

    qint32 nNumberOfRecords = pListRecords->size();
#ifdef QT_DEBUG
    qDebug("XFormats::extractArchiveRecordsToFolder: Number of records=%d", nNumberOfRecords);
#endif

    if (nNumberOfRecords > 0) {
        XDecompress xDecompress;
        connect(&xDecompress, &XDecompress::errorMessage, this, &XThreadObject::errorMessage);
        connect(&xDecompress, &XDecompress::warningMessage, this, &XThreadObject::warningMessage);
        connect(&xDecompress, &XDecompress::infoMessage, this, &XThreadObject::infoMessage);
        std::unique_ptr<XBinary> pArchiveStreamOwner;

        // One operation budget for the plain-record branch below. No property
        // map reaches this root, so the resolved defaults always run
        // shadow-metered, never enforcing. The archive-stream branch mints its
        // own per-record budget inside XBinary::_unpackRecordByIndex.
        const QMap<XBinary::UNPACK_PROP, QVariant> mapOperationProperties;
        XBinary::OUTPUT_POLICY outputPolicy = {};
        QSharedPointer<XBinary::OUTPUT_BUDGET> spOutputBudget;
        if (XBinary::resolveUnpackOutputPolicy(mapOperationProperties, &outputPolicy)) {
            spOutputBudget = QSharedPointer<XBinary::OUTPUT_BUDGET>::create();
            spOutputBudget->configureForProperties(outputPolicy, mapOperationProperties);
        }

        if (XBinary::createDirectory(sFolderName)) {
#ifdef QT_DEBUG
            qDebug("XFormats::extractArchiveRecordsToFolder: Directory created successfully");
#endif
            bResult = true;
            qint32 nGlobalIndex = XBinary::reservePdStructRecord(pPdStruct, nNumberOfRecords);

            const QString sCanonicalRoot = canonicalPath(sFolderName);

            if (sCanonicalRoot.isEmpty() || !QFileInfo(sCanonicalRoot).isDir()) {
                XBinary::setPdStructFinishedChecked(pPdStruct, nGlobalIndex, progressLifetime);
                return false;
            }

            XBinary::UNPACK_FOLDER_TRANSACTION transaction(sCanonicalRoot);
            const auto transactionError = [&]() -> QString {
                QString sError = transaction.errorString();
                if (sError.isEmpty()) {
                    sError = tr("Extraction transaction failed");
                }
                const QString sRecoveryPath = transaction.recoveryPath();
                if (!sRecoveryPath.isEmpty()) {
                    sError += QString(". %1: %2").arg(tr("Recovery path"), sRecoveryPath);
                }
                return sError;
            };
            const auto reportTransactionError = [&]() {
                const QString sError = transactionError();
                if (XBinary::isPdStructLifetimeAlive(progressLifetime)) {
                    XBinary::setPdStructErrorString(pPdStruct, sError);
                }
                emit errorMessage(sError);
            };
            const auto rollbackTransaction = [&]() {
                if (!transaction.rollback()) {
                    reportTransactionError();
                }
            };

            if (!transaction.isValid()) {
                reportTransactionError();
                XBinary::setPdStructFinishedChecked(pPdStruct, nGlobalIndex, progressLifetime);
                return false;
            }

            for (qint32 i = 0; bProgressOwnerAlive && (i < nNumberOfRecords) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
                QString sPrefName = pListRecords->at(i).mapProperties.value(XBinary::FPART_PROP_ORIGINALNAME).toString();
                const bool bIsFolder = pListRecords->at(i).mapProperties.value(XBinary::FPART_PROP_ISFOLDER, false).toBool() || sPrefName.endsWith(QLatin1Char('/')) ||
                                       sPrefName.endsWith(QLatin1Char('\\'));
#ifdef QT_DEBUG
                qDebug("XFormats::extractArchiveRecordsToFolder: Processing record %d/%d: %s", i + 1, nNumberOfRecords, sPrefName.toLatin1().data());
#endif

                XBinary::setPdStructStatus(pPdStruct, nGlobalIndex, sPrefName);

                QString sRelativePath = sPrefName;
                sRelativePath.replace(QLatin1Char('\\'), QLatin1Char('/'));
                sRelativePath = QDir::cleanPath(sRelativePath);

                const bool bDrivePath = (sRelativePath.size() >= 2) && sRelativePath.at(0).isLetter() && (sRelativePath.at(1) == QLatin1Char(':'));
#ifdef Q_OS_WIN
                const bool bAlternateDataStream = sRelativePath.contains(QLatin1Char(':'));
#else
                const bool bAlternateDataStream = false;
#endif
                const bool bUnsafeRelativePath = sRelativePath.isEmpty() || (sRelativePath == QLatin1String(".")) || (sRelativePath == QLatin1String("..")) ||
                                                 sRelativePath.startsWith(QLatin1String("../")) || QDir::isAbsolutePath(sRelativePath) || bDrivePath ||
                                                 bAlternateDataStream || sRelativePath.contains(QChar(0));
                const QString sLexicalResultFileName = bUnsafeRelativePath ? QString() : QDir(sCanonicalRoot).absoluteFilePath(sRelativePath);

                if (bUnsafeRelativePath || !isPathBelowRoot(sCanonicalRoot, sLexicalResultFileName)) {
#ifdef QT_DEBUG
                    qDebug("XFormats::extractArchiveRecordsToFolder: Path traversal detected for %s", sPrefName.toLatin1().data());
#endif
                    emit errorMessage(QString("%1: %2").arg(tr("Path traversal detected")).arg(sPrefName));
                    bResult = false;
                    break;
                }

                const qint32 nLastSeparator = sRelativePath.lastIndexOf(QLatin1Char('/'));
                const QString sRelativeParent = (nLastSeparator == -1) ? QString() : sRelativePath.left(nLastSeparator);
                const QString sBaseName = (nLastSeparator == -1) ? sRelativePath : sRelativePath.mid(nLastSeparator + 1);
                QString sCanonicalParent;

                if (ensureContainedDirectory(sCanonicalRoot, sRelativeParent, &transaction, &sCanonicalParent)) {
                    const QString sResultFileName = QDir(sCanonicalParent).absoluteFilePath(sBaseName);
                    QFileInfo fi(sResultFileName);

                    if (fi.isSymLink() || !isPathBelowRoot(sCanonicalRoot, sResultFileName)) {
                        emit errorMessage(QString("%1: %2").arg(tr("Cannot create")).arg(sResultFileName));
                        bResult = false;
                        break;
                    }

                    if (bIsFolder) {
                        const bool bFolderResult = transaction.ensureDirectory(sResultFileName);
                        if (!bFolderResult) {
                            if (!transaction.errorString().isEmpty()) {
                                reportTransactionError();
                            } else {
                                emit errorMessage(QString("%1: %2").arg(tr("Cannot create")).arg(sResultFileName));
                            }
                            bResult = false;
                            break;
                        }
                        bProgressOwnerAlive = XBinary::setPdStructCurrentIncrementChecked(pPdStruct, nGlobalIndex, progressLifetime);
                        if (!bProgressOwnerAlive) {
                            bResult = false;
                            break;
                        }
                        continue;
                    }

                    if (fi.isDir()) {
                        emit errorMessage(QString("%1: %2").arg(tr("Cannot create")).arg(sResultFileName));
                        bResult = false;
                        break;
                    }

                    QSaveFile file(sResultFileName);
                    file.setDirectWriteFallback(false);

                    if (file.open(QIODevice::WriteOnly)) {
                        const XBinary::ARCHIVERECORD &archiveRecord = pListRecords->at(i);
                        bool bRecordResult = false;

                        QMap<XBinary::UNPACK_PROP, QVariant> mapUnpackProperties;
                        qint32 nArchiveStreamIndex = -1;
                        if (XBinary::getArchiveStreamRecordIndex(archiveRecord, &nArchiveStreamIndex)) {
                            Q_UNUSED(nArchiveStreamIndex)
                            XArchive *pArchive = dynamic_cast<XArchive *>(pArchiveStreamOwner.get());
                            if (!pArchiveStreamOwner) {
                                DevicePositionGuard positionGuard(pDevice);
                                const XBinary::FT ownerType = getPrefFileType(pDevice, XBinary::FT_FLAG_ARCHIVES, pPdStruct);
                                if (XBinary::isPdStructLifetimeAlive(progressLifetime) && XBinary::isPdStructNotCanceled(pPdStruct)) {
                                    pArchiveStreamOwner.reset(createClass(ownerType, pDevice));
                                }
                                pArchive = dynamic_cast<XArchive *>(pArchiveStreamOwner.get());
                            }

                            QTemporaryFile stagedFile(QDir(sCanonicalParent).filePath(QLatin1String(".xformats-XXXXXX")));
                            bRecordResult =
                                pArchive && stagedFile.open() && pArchive->unpackArchiveStreamRecord(archiveRecord, &stagedFile, mapUnpackProperties, pPdStruct);
                            if (bRecordResult) {
                                bRecordResult = stagedFile.flush();
                            }
                            if (bRecordResult) {
                                const qint64 nStagedSize = stagedFile.size();
                                bRecordResult = (nStagedSize >= 0) && XBinary::copyDeviceMemory(&stagedFile, 0, &file, 0, nStagedSize, pPdStruct);
                            }
                        } else {
                            // QSaveFile is write-only, while the decoder rereads
                            // its complete output to verify the record's stored
                            // checksum - so decoding straight into it fails for
                            // every format that carries one (ZIP, for instance).
                            // Stage into a readable temporary exactly as the
                            // archive-stream branch above does, then publish.
                            if (spOutputBudget && !spOutputBudget->beginEntry(i, archiveRecord.mapProperties.value(XBinary::FPART_PROP_ORIGINALNAME).toString())) {
                                XBinary::OUTPUT_BUDGET::noteShadowRefusal(spOutputBudget.data());
                            }
                            QTemporaryFile stagedFile(QDir(sCanonicalParent).filePath(QLatin1String(".xformats-XXXXXX")));
                            bRecordResult = stagedFile.open() &&
                                            xDecompress.decompressArchiveRecord(archiveRecord, pDevice, &stagedFile, mapUnpackProperties, pPdStruct, spOutputBudget);
                            if (bRecordResult) {
                                bRecordResult = stagedFile.flush();
                            }
                            if (bRecordResult) {
                                const qint64 nStagedSize = stagedFile.size();
                                bRecordResult = (nStagedSize >= 0) && XBinary::copyDeviceMemory(&stagedFile, 0, &file, 0, nStagedSize, pPdStruct);
                            }
                        }
                        bProgressOwnerAlive = XBinary::isPdStructLifetimeAlive(progressLifetime);
                        if (!bProgressOwnerAlive) {
                            file.cancelWriting();
                            bResult = false;
                            break;
                        }
                        bRecordResult = bRecordResult && XBinary::isPdStructNotCanceled(pPdStruct);

                        bool bTransactionFailure = false;
                        bool bCommitted = false;
                        if (bRecordResult && QFileInfo(sResultFileName).isSymLink()) {
                            bRecordResult = false;
                        }
                        if (bRecordResult && !transaction.prepareFile(sResultFileName)) {
                            reportTransactionError();
                            bTransactionFailure = true;
                            bRecordResult = false;
                        }
                        if (bRecordResult) {
                            bCommitted = file.commit();
                            bRecordResult = bCommitted;
                        }
                        if (bRecordResult && !transaction.markFilePublished(sResultFileName)) {
                            reportTransactionError();
                            bTransactionFailure = true;
                            bRecordResult = false;
                        }

                        if (!bRecordResult) {
                            if (!bCommitted) {
                                file.cancelWriting();
                            }
#ifdef QT_DEBUG
                            qDebug() << "Cannot decompress" << sPrefName;
#endif
                            if (!bTransactionFailure) {
                                emit errorMessage(QString("%1: %2").arg(tr("Cannot decompress")).arg(sPrefName));
                            }
                            bResult = false;
                        }
#ifdef QT_DEBUG
                        qDebug("XFormats::extractArchiveRecordsToFolder: File closed successfully");
#endif
                    } else {
#ifdef QT_DEBUG
                        qDebug("XFormats::extractArchiveRecordsToFolder: Cannot create file %s", sResultFileName.toLatin1().data());
#endif
                        emit errorMessage(QString("%1: %2").arg(tr("Cannot create")).arg(sResultFileName));
                        bResult = false;
                    }
                } else {
#ifdef QT_DEBUG
                    qDebug("XFormats::extractArchiveRecordsToFolder: Cannot create directory for %s", sPrefName.toLatin1().data());
#endif
                    if (!transaction.errorString().isEmpty()) {
                        reportTransactionError();
                    } else {
                        emit errorMessage(QString("%1: %2").arg(tr("Cannot create")).arg(sPrefName));
                    }
                    bResult = false;
                }

                if (!bResult) {
                    break;
                }
                bProgressOwnerAlive = XBinary::setPdStructCurrentIncrementChecked(pPdStruct, nGlobalIndex, progressLifetime);
                if (!bProgressOwnerAlive) {
                    bResult = false;
                    break;
                }
            }

            bProgressOwnerAlive = bProgressOwnerAlive && XBinary::isPdStructLifetimeAlive(progressLifetime);
            if (bProgressOwnerAlive && !XBinary::isPdStructNotCanceled(pPdStruct)) {
                bResult = false;
            }

            if (bResult) {
                if (!transaction.commit()) {
                    reportTransactionError();
                    bResult = false;
                    rollbackTransaction();
                } else if (!transaction.errorString().isEmpty()) {
                    reportTransactionError();
                }
            } else {
                rollbackTransaction();
            }

            XBinary::setPdStructFinishedChecked(pPdStruct, nGlobalIndex, progressLifetime);
#ifdef QT_DEBUG
            qDebug("XFormats::extractArchiveRecordsToFolder: Extraction completed, bResult=%d", bResult);
#endif
        } else {
#ifdef QT_DEBUG
            qDebug("XFormats::extractArchiveRecordsToFolder: Cannot create folder %s", sFolderName.toLatin1().data());
#endif
            emit errorMessage(QString("%1: %2").arg(tr("Cannot create")).arg(sFolderName));
            bResult = false;
        }
    }

#ifdef QT_DEBUG
    qDebug("XFormats::extractArchiveRecordsToFolder: Returning bResult=%d", bResult);
#endif
    return bResult;
}

bool XFormats::packFolderToDevice(XBinary::FT fileType, QIODevice *pDevice, const QMap<XBinary::PACK_PROP, QVariant> &mapProperties, const QString &sFolderName,
                                  XBinary::PDSTRUCT *pPdStruct)
{
    bool bResult = false;

    XBinary::PDSTRUCT pdStructEmpty = XBinary::createPdStruct();
    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }
    const XBinary::PDSTRUCTLIFETIME progressLifetime = XBinary::retainPdStructLifetime(pPdStruct);
    if (!progressLifetime.isValid()) return false;

    if (pDevice && pDevice->isOpen() && pDevice->isWritable() && !pDevice->isSequential() && XBinary::isPdStructNotCanceled(pPdStruct) &&
        XBinary::isDirectoryExists(sFolderName)) {
        XBinary *pBinary = createClass(fileType, pDevice);

        if (pBinary) {
            bResult = pBinary->packFolderToDevice(pDevice, mapProperties, sFolderName, pPdStruct);
            if (!XBinary::isPdStructLifetimeAlive(progressLifetime)) bResult = false;
            delete pBinary;
        }
    }

    return bResult;
}

#ifdef QT_GUI_LIB
XBinary::FT XFormats::setFileTypeComboBox(XBinary::FT fileType, QIODevice *pDevice, QComboBox *pComboBox, quint32 nFileTypeFlags)
{
    const bool bBlocked1 = pComboBox->blockSignals(true);

    QList<XBinary::FT> listFileTypes;

    if (fileType != XBinary::FT_REGION) {
        // Always detect the standard formats (so category/symbols filtering has something to work
        // with), plus the packer/installer handle-method types only when explicitly requested.
        QSet<XBinary::FT> stFileType = XFormats::getFileTypes(pDevice, XBinary::FT_FLAG_FORMATS | (nFileTypeFlags & XBinary::FT_FLAG_STATICUNPACKERS));

        listFileTypes = XBinary::_getFileTypeListFromSet(stFileType, nFileTypeFlags);
    } else {
        listFileTypes.append(XBinary::FT_REGION);
    }

    pComboBox->clear();

    qint32 nNumberOfListTypes = listFileTypes.size();

    for (qint32 i = 0; i < nNumberOfListTypes; i++) {
        XBinary::FT _fileType = listFileTypes.at(i);
        pComboBox->addItem(XBinary::fileTypeIdToString(_fileType), _fileType);
    }

    if (nNumberOfListTypes) {
        if (fileType == XBinary::FT_UNKNOWN) {
            pComboBox->setCurrentIndex(nNumberOfListTypes - 1);
        } else {
            setComboBoxCurrent(pComboBox, fileType);
        }
    }

    pComboBox->blockSignals(bBlocked1);

    return (XBinary::FT)(pComboBox->currentData().toUInt());
}
#endif
#ifdef QT_GUI_LIB
XBinary::FT XFormats::setFileTypeComboBox(XBinary::FT fileType, const QString &sFileName, QComboBox *pComboBox, quint32 nFileTypeFlags)
{
    XBinary::FT result = XBinary::FT_UNKNOWN;

    QFile file;
    file.setFileName(sFileName);

    if (file.open(QIODevice::ReadOnly)) {
        result = setFileTypeComboBox(fileType, &file, pComboBox, nFileTypeFlags);

        file.close();
    }

    return result;
}
#endif
#ifdef QT_GUI_LIB
QVariant XFormats::setComboBoxCurrent(QComboBox *pComboBox, QVariant varValue)
{
    QVariant varResult;

    const bool bBlocked1 = pComboBox->blockSignals(true);

    qint32 nNumberOfItems = pComboBox->count();

    for (qint32 i = 0; i < nNumberOfItems; i++) {
        if (pComboBox->itemData(i) == varValue) {
            pComboBox->setCurrentIndex(i);

            varResult = varValue;

            break;
        }
    }

    pComboBox->blockSignals(bBlocked1);

    return varResult;
}

#endif
#ifdef QT_GUI_LIB
XBinary::ENDIAN XFormats::setEndiannessComboBox(QComboBox *pComboBox, XBinary::ENDIAN endian)
{
    const bool bBlocked1 = pComboBox->blockSignals(true);

    pComboBox->clear();

    pComboBox->addItem(XBinary::endianToString(XBinary::ENDIAN_LITTLE), XBinary::ENDIAN_LITTLE);
    pComboBox->addItem(XBinary::endianToString(XBinary::ENDIAN_BIG), XBinary::ENDIAN_BIG);

    XBinary::ENDIAN result = (XBinary::ENDIAN)setComboBoxCurrent(pComboBox, endian).toUInt();

    pComboBox->blockSignals(bBlocked1);

    return result;
}
#endif
#ifdef QT_GUI_LIB
XBinary::CODEPAGE XFormats::setCodepageComboBox(QComboBox *pComboBox, XBinary::CODEPAGE codepage)
{
    const bool bBlocked1 = pComboBox->blockSignals(true);

    pComboBox->clear();

    QList<XBinary::CODEPAGE> listCodepages = XBinary::getCodepagesList();

    qint32 nNumberOfRecords = listCodepages.size();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        XBinary::CODEPAGE record = listCodepages.at(i);
        pComboBox->addItem(XBinary::codepageIdToString(record), record);
    }

    XBinary::CODEPAGE result = (XBinary::CODEPAGE)setComboBoxCurrent(pComboBox, codepage).toUInt();

    pComboBox->blockSignals(bBlocked1);

    return result;
}
#endif
#ifdef QT_GUI_LIB
qint32 XFormats::setBaseComboBox(QComboBox *pComboBox, qint32 nBase)
{
    const bool bBlocked1 = pComboBox->blockSignals(true);

    pComboBox->clear();

    pComboBox->addItem(QString("10"), 10);
    pComboBox->addItem(QString("16"), 16);

    qint32 nResult = setComboBoxCurrent(pComboBox, nBase).toInt();

    pComboBox->blockSignals(bBlocked1);

    return nResult;
}
#endif
#ifdef QT_GUI_LIB
XBinary::MAPMODE XFormats::getMapModesList(XBinary::FT fileType, QComboBox *pComboBox)
{
    XBinary::MAPMODE result = XBinary::MAPMODE_UNKNOWN;

    const bool bBlocked1 = pComboBox->blockSignals(true);

    pComboBox->clear();

    QList<XBinary::MAPMODE> listMapModes = getMapModesList(fileType);

    qint32 nNumberOfRecords = listMapModes.size();

    if (nNumberOfRecords) {
        result = listMapModes.at(0);

        for (qint32 i = 0; i < nNumberOfRecords; i++) {
            XBinary::MAPMODE record = listMapModes.at(i);
            pComboBox->addItem(XBinary::mapModeToString(record), record);
        }
    }

    pComboBox->blockSignals(bBlocked1);

    return result;
}
#endif
#ifdef QT_GUI_LIB
void XFormats::setProgressBar(QProgressBar *pProgressBar, XBinary::PDRECORD pdRecord)
{
    if (!pProgressBar) return;

    const qint64 nTotal = pdRecord.nTotal.loadAcquire();
    const qint64 nCurrent = pdRecord.nCurrent.loadAcquire();

    if ((nTotal > 0) || !pdRecord.sStatus.isEmpty()) {
        pProgressBar->show();

        if (nTotal > 0) {
            const qint64 nBoundedCurrent = qBound((qint64)0, nCurrent, nTotal);
            const qint32 nValue =
                qBound((qint32)0, static_cast<qint32>((static_cast<long double>(nBoundedCurrent) * 100.0L) / static_cast<long double>(nTotal)), (qint32)100);
            pProgressBar->setMaximum(100);
            pProgressBar->setValue(nValue);
        }

        if (!pdRecord.sStatus.isEmpty()) {
            pProgressBar->setFormat(pdRecord.sStatus);
        }
    } else {
        pProgressBar->hide();
    }
}
#endif
#ifdef QT_GUI_LIB
XBinary::DM XFormats::setDisasmModeComboBox(XBinary::DM disasmMode, QComboBox *pComboBox)
{
    XBinary::DM result = disasmMode;

    const bool bBlocked1 = pComboBox->blockSignals(true);

    pComboBox->clear();

    if (disasmMode == XBinary::DM_UNKNOWN) {
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_8086), XBinary::DM_8086);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_X86_32), XBinary::DM_X86_32);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_X86_64), XBinary::DM_X86_64);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_ARM_LE), XBinary::DM_ARM_LE);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_ARM_BE), XBinary::DM_ARM_BE);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_AARCH64_LE), XBinary::DM_AARCH64_LE);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_AARCH64_BE), XBinary::DM_AARCH64_BE);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_CORTEXM), XBinary::DM_CORTEXM);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_THUMB_LE), XBinary::DM_THUMB_LE);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_THUMB_BE), XBinary::DM_THUMB_BE);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_MIPS_LE), XBinary::DM_MIPS_LE);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_MIPS_BE), XBinary::DM_MIPS_BE);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_MIPS64_LE), XBinary::DM_MIPS64_LE);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_MIPS64_BE), XBinary::DM_MIPS64_BE);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_PPC_LE), XBinary::DM_PPC_LE);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_PPC_BE), XBinary::DM_PPC_BE);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_PPC64_LE), XBinary::DM_PPC64_LE);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_PPC64_BE), XBinary::DM_PPC64_BE);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_SPARC), XBinary::DM_SPARC);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_S390X), XBinary::DM_S390X);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_XCORE), XBinary::DM_XCORE);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_M68K), XBinary::DM_M68K);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_M68K00), XBinary::DM_M68K00);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_M68K10), XBinary::DM_M68K10);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_M68K20), XBinary::DM_M68K20);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_M68K30), XBinary::DM_M68K30);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_M68K40), XBinary::DM_M68K40);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_M68K60), XBinary::DM_M68K60);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_TMS320C64X), XBinary::DM_TMS320C64X);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_M6800), XBinary::DM_M6800);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_M6801), XBinary::DM_M6801);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_M6805), XBinary::DM_M6805);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_M6808), XBinary::DM_M6808);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_M6809), XBinary::DM_M6809);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_M6811), XBinary::DM_M6811);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_CPU12), XBinary::DM_CPU12);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_HD6301), XBinary::DM_HD6301);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_HD6309), XBinary::DM_HD6309);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_HCS08), XBinary::DM_HCS08);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_EVM), XBinary::DM_EVM);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_MOS65XX), XBinary::DM_MOS65XX);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_RISKV32), XBinary::DM_RISKV32);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_RISKV64), XBinary::DM_RISKV64);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_RISKVC), XBinary::DM_RISKVC);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_MOS65XX), XBinary::DM_MOS65XX);
        pComboBox->addItem(XBinary::disasmIdToString(XBinary::DM_WASM), XBinary::DM_WASM);
    } else {
        pComboBox->addItem(XBinary::disasmIdToString(disasmMode), disasmMode);
    }

    pComboBox->blockSignals(bBlocked1);

    return result;
}
#endif

QString XFormats::toJSON(const QVector<XBinary::KeyValueItem> &listItems)
{
    QJsonObject jsonObject;
    qint32 nCount = listItems.size();
    for (qint32 i = 0; i < nCount; i++) {
        jsonObject[listItems.at(i).key] = QJsonValue::fromVariant(listItems.at(i).value);
    }
    return QString(QJsonDocument(jsonObject).toJson(QJsonDocument::Compact));
}

QString XFormats::toXML(const QVector<XBinary::KeyValueItem> &listItems)
{
    QString sResult;
    QXmlStreamWriter xml(&sResult);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement("items");
    qint32 nCount = listItems.size();
    for (qint32 i = 0; i < nCount; i++) {
        xml.writeTextElement(listItems.at(i).key, listItems.at(i).value.toString());
    }
    xml.writeEndElement();
    xml.writeEndDocument();

    if (xml.hasError()) {
        return QString();
    }

    return sResult;
}

QString XFormats::toCSV(const QVector<XBinary::KeyValueItem> &listItems)
{
    QStringList keys, values;
    qint32 nCount = listItems.size();
    for (qint32 i = 0; i < nCount; i++) {
        keys.append(listItems.at(i).key);
        values.append(listItems.at(i).value.toString());
    }
    return keys.join(',') + "\n" + values.join(',') + "\n";
}

QString XFormats::toTSV(const QVector<XBinary::KeyValueItem> &listItems)
{
    QStringList keys, values;
    qint32 nCount = listItems.size();
    for (qint32 i = 0; i < nCount; i++) {
        keys.append(listItems.at(i).key);
        values.append(listItems.at(i).value.toString());
    }
    return keys.join('\t') + "\n" + values.join('\t') + "\n";
}

QString XFormats::toFormattedString(const QVector<XBinary::KeyValueItem> &listItems)
{
    QString sResult;
    qint32 nCount = listItems.size();
    for (qint32 i = 0; i < nCount; i++) {
        sResult += QString("%1: %2\n").arg(listItems.at(i).key).arg(listItems.at(i).value.toString());
    }
    return sResult;
}

QVector<XBinary::KeyValueItem> XFormats::getEntropy(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress, XBinary::PDSTRUCT *pPdStruct)
{
    QVector<XBinary::KeyValueItem> result;

    XBinary::PDSTRUCT pdStructEmpty = XBinary::createPdStruct();
    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }
    const XBinary::PDSTRUCTLIFETIME progressLifetime = XBinary::retainPdStructLifetime(pPdStruct);
    bool bProgressOwnerAlive = progressLifetime.isValid();
    if (!bProgressOwnerAlive || !isReadableSeekableDevice(pDevice) || !XBinary::isPdStructNotCanceled(pPdStruct)) return result;
    DevicePositionGuard positionGuard(pDevice);

    XBinary binary(pDevice);

    result.append({"Total", binary.getBinaryStatus(XBinary::BSTATUS_ENTROPY, 0, -1, pPdStruct)});
    bProgressOwnerAlive = XBinary::isPdStructLifetimeAlive(progressLifetime);

    if (!bProgressOwnerAlive || !XBinary::isPdStructNotCanceled(pPdStruct)) return {};

    XBinary::FT fileType = getPrefFileType(pDevice, XBinary::FT_FLAG_FORMATS, pPdStruct);
    bProgressOwnerAlive = XBinary::isPdStructLifetimeAlive(progressLifetime);
    if (!bProgressOwnerAlive || !XBinary::isPdStructNotCanceled(pPdStruct)) return {};
    XBinary::_MEMORY_MAP memoryMap = getMemoryMap(fileType, XBinary::MAPMODE_UNKNOWN, pDevice, bIsImage, nModuleAddress, pPdStruct);
    bProgressOwnerAlive = XBinary::isPdStructLifetimeAlive(progressLifetime);
    if (!bProgressOwnerAlive) return {};

    qint32 nNumberOfRecords = memoryMap.listRecords.size();
    for (qint32 i = 0; bProgressOwnerAlive && (i < nNumberOfRecords) && XBinary::isPdStructNotCanceled(pPdStruct); i++) {
        const XBinary::_MEMORY_RECORD &record = memoryMap.listRecords.at(i);

        if (record.bIsVirtual || (record.nOffset < 0) || (record.nSize <= 0) || (record.nOffset > pDevice->size()) ||
            (record.nSize > (pDevice->size() - record.nOffset))) {
            continue;
        }

        double dEntropy = binary.getBinaryStatus(XBinary::BSTATUS_ENTROPY, record.nOffset, record.nSize, pPdStruct);
        bProgressOwnerAlive = XBinary::isPdStructLifetimeAlive(progressLifetime);
        if (!bProgressOwnerAlive) return {};

        QString sKey = record.sName.isEmpty() ? QString("Record_%1").arg(i) : record.sName;
        result.append({sKey, dEntropy});
    }

    if (bProgressOwnerAlive && !XBinary::isPdStructNotCanceled(pPdStruct)) result.clear();
    return result;
}

QVector<XBinary::KeyValueItem> XFormats::getFileInfo(QIODevice *pDevice, bool bIsImage, XADDR nModuleAddress, XBinary::PDSTRUCT *pPdStruct, XBinary::FT nForcedFileType)
{
    QVector<XBinary::KeyValueItem> result;

    XBinary::PDSTRUCT pdStructEmpty = XBinary::createPdStruct();
    if (!pPdStruct) {
        pPdStruct = &pdStructEmpty;
    }
    const XBinary::PDSTRUCTLIFETIME progressLifetime = XBinary::retainPdStructLifetime(pPdStruct);
    bool bProgressOwnerAlive = progressLifetime.isValid();
    if (!bProgressOwnerAlive || !isReadableSeekableDevice(pDevice) || !XBinary::isPdStructNotCanceled(pPdStruct)) return result;
    DevicePositionGuard positionGuard(pDevice);

    // Honor an explicitly forced file type (-F/--filetype) rather than
    // auto-detecting, so `-i -F UDF` on a bridge disc reports UDF instead of the
    // ISO 9660 bridge that always wins detection (XFU-042).
    XBinary::FT fileType = (nForcedFileType != XBinary::FT_UNKNOWN) ? nForcedFileType : getPrefFileType(pDevice, XBinary::FT_FLAG_FORMATS, pPdStruct);
    bProgressOwnerAlive = XBinary::isPdStructLifetimeAlive(progressLifetime);
    if (!bProgressOwnerAlive || !XBinary::isPdStructNotCanceled(pPdStruct)) return result;
    XBinary *pBinary = createClass(fileType, pDevice, bIsImage, nModuleAddress);

    if (pBinary) {
        XBinary::FILEFORMATINFO info = pBinary->getFileFormatInfo(pPdStruct);
        bProgressOwnerAlive = XBinary::isPdStructLifetimeAlive(progressLifetime);
        if (!bProgressOwnerAlive) {
            delete pBinary;
            return {};
        }

        QString sValue;

        sValue = XBinary::fileTypeIdToString(info.fileType);
        if (!sValue.isEmpty()) result.append({"FileType", sValue});
        sValue = XBinary::bytesCountToString(info.nSize);
        if (!sValue.isEmpty()) result.append({"Size", sValue});
        sValue = info.sExt;
        if (!sValue.isEmpty()) result.append({"Ext", sValue});
        sValue = info.sVersion;
        if (!sValue.isEmpty()) result.append({"Version", sValue});
        sValue = info.sInfo;
        if (!sValue.isEmpty()) result.append({"Info", sValue});
        sValue = info.sType;
        if (!sValue.isEmpty()) result.append({"Type", sValue});
        sValue = info.sArch;
        if (!sValue.isEmpty()) result.append({"Arch", sValue});
        sValue = XBinary::modeIdToString(info.mode);
        if (!sValue.isEmpty()) result.append({"Mode", sValue});
        sValue = XBinary::endianToString(info.endian);
        if (!sValue.isEmpty()) result.append({"Endian", sValue});
        sValue = info.sMIME;
        if (!sValue.isEmpty()) result.append({"MIME", sValue});
        sValue = XBinary::osNameIdToString(info.osName);
        if (!sValue.isEmpty()) result.append({"OsName", sValue});
        sValue = info.sOsVersion;
        if (!sValue.isEmpty()) result.append({"OsVersion", sValue});
        sValue = info.sOsBuild;
        if (!sValue.isEmpty()) result.append({"OsBuild", sValue});
        sValue = XBinary::boolToString(info.bIsVM);
        if (!sValue.isEmpty()) result.append({"IsVM", sValue});
        sValue = XBinary::boolToString(info.bIsEncrypted);
        if (!sValue.isEmpty()) result.append({"IsEncrypted", sValue});
        sValue = info.sCompresionMethod;
        if (!sValue.isEmpty()) result.append({"CompressionMethod", sValue});

        sValue = XBinary::getHash(XBinary::HASH_MD5, pDevice, pPdStruct);
        bProgressOwnerAlive = XBinary::isPdStructLifetimeAlive(progressLifetime);
        if (!bProgressOwnerAlive) {
            delete pBinary;
            return {};
        }
        if (!sValue.isEmpty()) result.append({"MD5", sValue});

        sValue = QString::number(pBinary->getBinaryStatus(XBinary::BSTATUS_ENTROPY, 0, -1, pPdStruct), 'f', 4);
        bProgressOwnerAlive = XBinary::isPdStructLifetimeAlive(progressLifetime);
        if (!bProgressOwnerAlive) {
            delete pBinary;
            return {};
        }
        if (!sValue.isEmpty()) result.append({"Entropy", sValue});

        delete pBinary;
    }

    if (bProgressOwnerAlive && !XBinary::isPdStructNotCanceled(pPdStruct)) result.clear();
    return result;
}
