/* Copyright (c) 2019-2026 hors<horsicq@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef XARCHIVE_H
#define XARCHIVE_H

#include "xbinary.h"
#include "xcompress.h"
#include "xdecompress.h"
#include "Algos/xlzhdecoder.h"
#include "Algos/xrardecoder.h"
#include "Algos/xstoredecoder.h"
#include "Algos/xdeflatedecoder.h"
#include "Algos/xbzip2decoder.h"
#include "Algos/xlzmadecoder.h"
#include "Algos/xlzssdecoder.h"
#include <QSharedPointer>

class XArchive : public XBinary {
    Q_OBJECT

public:
    struct INTERNAL_INFO : XBinary::INTERNAL_INFO {};

    struct SOURCE_DEVICE_CHAIN_ITEM {
        QPointer<QIODevice> pDevice;
        bool bIsSubDevice;
        quint64 nInitLocation;
        qint64 nSize;
    };

    enum SOURCE_DEVICE_ROOT_KIND {
        SOURCE_DEVICE_ROOT_UNKNOWN = 0,
        SOURCE_DEVICE_ROOT_BUFFER,
        SOURCE_DEVICE_ROOT_FILE,
        SOURCE_DEVICE_ROOT_GENERIC
    };

    // Immutable description of the device object/backing that was parsed by
    // initUnpack().  QPointers make destroyed caller-owned devices fail closed;
    // the complete SubDevice chain prevents an open wrapper from being
    // retargeted to another range of the same backing object.  The content
    // fingerprint also covers same-sized changes, including writes through a
    // QBuffer raw pointer acquired before the snapshot.
    struct SOURCE_DEVICE_SNAPSHOT {
        QPointer<QIODevice> pSourceDevice;
        QPointer<QIODevice> pRootDevice;
        QList<SOURCE_DEVICE_CHAIN_ITEM> listChain;
        SOURCE_DEVICE_ROOT_KIND rootKind;
        qint64 nRootSize;
        quintptr nBufferBackingIdentity;
        QByteArray baBufferSnapshot;
        QString sFilePath;
        QByteArray baFilePhysicalIdentity;
        QByteArray baFileMutationIdentity;
        QByteArray baContentFingerprint;
        bool bContentFingerprintRequired;
        quint64 nOwnerDeviceGeneration;
    };

    bool handleInternalInfo(PDSTRUCT *pPdStruct) override;
    void *getInternalInfo(PDSTRUCT *pPdStruct) override;
    void setInternalInfo(void *pInternalInfo) override;

    virtual QList<QString> getSearchSignatures() override;
    virtual XBinary *createInstance(QIODevice *pDevice, bool bIsImage = false, XADDR nModuleAddress = -1) override;
    enum TYPE {
        TYPE_UNKNOWN = 0,
        TYPE_ARCHIVE,
        TYPE_DOSEXTENDER
        // TODO more
    };

    struct COMPRESS_INFO {
        HANDLE_METHOD compressMethod;
        QString sOptions;
    };

    struct SPINFO {
        QString sRecordName;
        quint32 nCRC32;
        qint64 nUncompressedSize;
        quint64 nWindowSize;
        bool bIsSolid;  // For RAR
        HANDLE_METHOD compressMethod;
        HANDLE_METHOD compressMethod2;  // Optional second stage for layered streams.
        // FT fileType;
    };

    struct RECORD {  // Obsolete, use ARCHIVERECORD
        SPINFO spInfo;
        // Preserve the complete modern record contract for legacy callers.
        // Solid-substream coordinates, CRC type, folder state, encryption
        // properties, and codec-specific metadata cannot be represented by
        // SPINFO alone.
        QMap<FPART_PROP, QVariant> mapProperties;
        qint64 nDataOffset;
        qint64 nDataSize;
        qint64 nHeaderOffset;
        qint64 nHeaderSize;
        qint64 nOptHeaderOffset;
        qint64 nOptHeaderSize;
        QString sUUID;
        // for targz
        // qint64 nLayerOffset;
        // qint64 nLayerSize;
        // HANDLE_METHOD layerCompressMethod;
    };

    enum COMPRESS_RESULT {
        COMPRESS_RESULT_UNKNOWN = 0,
        COMPRESS_RESULT_OK,
        COMPRESS_RESULT_DATAERROR,
        COMPRESS_RESULT_MEMORYERROR,
        COMPRESS_RESULT_BUFFERERROR,
        COMPRESS_RESULT_METHODERROR,
        COMPRESS_RESULT_READERROR,
        COMPRESS_RESULT_WRITEERROR
        // TODO more
    };

    static const qint32 COMPRESS_BUFFERSIZE = 0x4000;    // TODO Check mb set/get ???
    static const qint32 DECOMPRESS_BUFFERSIZE = 0x4000;  // TODO Check mb set/get ???

    bool captureSourceDeviceSnapshot(QIODevice *pDevice, SOURCE_DEVICE_SNAPSHOT *pSnapshot);
    bool isSourceDeviceSnapshotCurrent(const SOURCE_DEVICE_SNAPSHOT &snapshot, QIODevice *pCurrentDevice, PDSTRUCT *pPdStruct = nullptr);

    // Bind a successful streaming initialization to both the source it parsed
    // and the exact UNPACK_STATE object that owns the live context.  Bitwise
    // state copies are never authorized to traverse or destroy that context.
    bool bindUnpackSource(UNPACK_STATE *pState, PDSTRUCT *pPdStruct = nullptr);
    bool getBoundUnpackSourceSnapshot(const UNPACK_STATE *pState, SOURCE_DEVICE_SNAPSHOT *pSnapshot) const;
    bool validateAndFinalizeUnpackSource(UNPACK_STATE *pState, PDSTRUCT *pPdStruct = nullptr);
    template <typename T>
    bool validateAndFinalizeUnpackSource(UNPACK_STATE *pState, T *pContext, PDSTRUCT *pPdStruct = nullptr)
    {
        if (!pState || !pContext || (pState->pContext != static_cast<void *>(pContext))) {
            return false;
        }

        QPointer<XArchive> guardedArchive(this);
        if (!registerUnpackContextCleanup(pState, pContext, &deleteUnpackContext<T>)) {
            return false;
        }

        const bool bValidated = validateAndFinalizeUnpackSource(pState, pPdStruct);
        if (!guardedArchive) {
            // Archive destruction transfers the registered context to the
            // shared operation guard.  The caller must not touch it.
            return false;
        }
        if (!bValidated) {
            // A failed final source callback must not leave the provisional
            // deleter armed.  releaseUnpackSource() normally performs the
            // same removal, but this keeps the template safe if validation
            // failed before the caller reaches its common cleanup path.
            guardedArchive->unregisterUnpackContextCleanup(pContext);
        }
        return bValidated;
    }
    bool isUnpackSourceCurrent(const UNPACK_STATE *pState, PDSTRUCT *pPdStruct = nullptr);
    bool ownsUnpackSource(const UNPACK_STATE *pState);
    void releaseUnpackSource(UNPACK_STATE *pState);
    bool transferUnpackSourceOwnership(UNPACK_STATE *pFromState, UNPACK_STATE *pToState);

protected:
    struct UNPACK_GUARD_STATE {
        typedef void (*CONTEXT_DELETER)(void *);

        struct CONTEXT_CLEANUP {
            void *pContext;
            CONTEXT_DELETER pDeleter;
        };

        UNPACK_GUARD_STATE() : bOperationInProgress(false), bNestedInfoAuthorized(false), bOwnerAlive(true), nPrivateRecordAuthorized(0)
        {
        }

        ~UNPACK_GUARD_STATE()
        {
            cleanupContexts();
        }

        void cleanupContexts()
        {
            QList<CONTEXT_CLEANUP> listCleanup;
            listCleanup.swap(listContexts);
            for (const CONTEXT_CLEANUP &cleanup : listCleanup) {
                if (cleanup.pContext && cleanup.pDeleter) {
                    cleanup.pDeleter(cleanup.pContext);
                }
            }
        }

        bool bOperationInProgress;
        bool bNestedInfoAuthorized;
        bool bOwnerAlive;
        // Raised only while an archive is decoding a member for itself, and
        // read by infoCurrent() implementations whose internal record shape
        // must never be published (see PRIVATE_RECORD_AUTHORIZATION).  Lives
        // in the shared guard state, not in the archive object, so a caller
        // callback that destroys the archive mid-decode cannot leave a
        // dangling counter behind.
        qint32 nPrivateRecordAuthorized;
        QList<CONTEXT_CLEANUP> listContexts;
    };

    // Some formats can decode a member two ways: through coordinates that are
    // correct but describe a stream SHARED by every member (a solid block, or
    // a whole-container compressed stream), or through their own session by
    // logical record index.  The first shape decodes correctly but must never
    // be published, because its (offset,size) pair resolves on the raw
    // container: a caller who changes nothing but the method field gets
    // container bytes at the member's declared length.  An infoCurrent() that
    // can build both shapes builds the coordinate one only while this
    // authorization is held by the archive's own decode path.
    class PRIVATE_RECORD_AUTHORIZATION {
    public:
        explicit PRIVATE_RECORD_AUTHORIZATION(const QSharedPointer<UNPACK_GUARD_STATE> &pState)
            : m_pState((pState && (pState->nPrivateRecordAuthorized >= 0)) ? pState : QSharedPointer<UNPACK_GUARD_STATE>())
        {
            if (m_pState) m_pState->nPrivateRecordAuthorized++;
        }

        ~PRIVATE_RECORD_AUTHORIZATION()
        {
            if (m_pState && (m_pState->nPrivateRecordAuthorized > 0)) {
                m_pState->nPrivateRecordAuthorized--;
            }
        }

        bool isAuthorized() const
        {
            return !m_pState.isNull();
        }

        static bool isHeldBy(const QSharedPointer<UNPACK_GUARD_STATE> &pState)
        {
            return pState && (pState->nPrivateRecordAuthorized > 0);
        }

    private:
        Q_DISABLE_COPY(PRIVATE_RECORD_AUTHORIZATION)
        QSharedPointer<UNPACK_GUARD_STATE> m_pState;
    };

    template <typename T>
    static void deleteUnpackContext(void *pContext)
    {
        delete static_cast<T *>(pContext);
    }

    bool registerUnpackContextCleanup(UNPACK_STATE *pState, void *pContext, UNPACK_GUARD_STATE::CONTEXT_DELETER pDeleter);
    void unregisterUnpackContextCleanup(void *pContext);

    // Compatibility facade for existing format callsites.  Taking the
    // address of this object no longer exposes archive-owned storage: guards
    // copy the shared heap state before any caller-controlled callback.
    class UNPACK_GUARD_FLAG {
    public:
        UNPACK_GUARD_FLAG() : m_bNestedAuthorization(false)
        {
        }
        UNPACK_GUARD_FLAG(const QSharedPointer<UNPACK_GUARD_STATE> &pState, bool bNestedAuthorization) : m_pState(pState), m_bNestedAuthorization(bNestedAuthorization)
        {
        }

        operator bool() const
        {
            return m_pState && (m_bNestedAuthorization ? m_pState->bNestedInfoAuthorized : m_pState->bOperationInProgress);
        }

        const QSharedPointer<UNPACK_GUARD_STATE> &state() const
        {
            return m_pState;
        }

        bool isNestedAuthorization() const
        {
            return m_bNestedAuthorization;
        }

    private:
        QSharedPointer<UNPACK_GUARD_STATE> m_pState;
        bool m_bNestedAuthorization;
    };

    // Custom archive implementations must decode into a private device and
    // use this publication path.  It rejects destinations for which exact
    // replacement/rollback cannot be guaranteed and leaves an empty,
    // position-zero destination if publication itself fails.
    bool isUnpackOutputSupported(QIODevice *pDevice) const;
    bool publishUnpackOutput(QIODevice *pStageDevice, QIODevice *pOutputDevice, const UNPACK_STATE *pState, PDSTRUCT *pPdStruct = nullptr);

    bool isDeviceReplacementAllowed() const override
    {
        return !m_pUnpackGuardState || !m_pUnpackGuardState->bOperationInProgress;
    }

    // QIODevice implementations are caller-controlled and may re-enter an
    // archive while size/seek/read/write is in progress.  Streaming methods
    // use this guard before retaining context pointers or record references
    // across any such virtual call.
    class UNPACK_OPERATION_GUARD {
    public:
        explicit UNPACK_OPERATION_GUARD(const QSharedPointer<UNPACK_GUARD_STATE> &pState, bool bInfoOperation = false)
            : m_pState(pState), m_bAcquired(false), m_bAllowed(false)
        {
            if (!m_pState) return;
            if (!m_pState->bOperationInProgress) {
                m_pState->bOperationInProgress = true;
                m_bAcquired = true;
                m_bAllowed = true;
            } else if (bInfoOperation && m_pState->bNestedInfoAuthorized) {
                m_pState->bNestedInfoAuthorized = false;
                m_bAllowed = true;
            }
        }

        explicit UNPACK_OPERATION_GUARD(UNPACK_GUARD_FLAG *pBusy, UNPACK_GUARD_FLAG *pNestedInfoAuthorization = nullptr)
            : UNPACK_OPERATION_GUARD(pBusy ? pBusy->state() : QSharedPointer<UNPACK_GUARD_STATE>(),
                                     pBusy && pNestedInfoAuthorization && !pBusy->isNestedAuthorization() && pNestedInfoAuthorization->isNestedAuthorization() &&
                                         (pBusy->state() == pNestedInfoAuthorization->state()))
        {
        }

        ~UNPACK_OPERATION_GUARD()
        {
            release();
        }

        bool isAcquired() const
        {
            return m_bAcquired;
        }
        bool isAllowed() const
        {
            return m_bAllowed;
        }

        // A guarded initializer may need to delegate terminal cleanup to its
        // public finishUnpack() implementation.  Release first so that the
        // cleanup method can acquire the same guard for the duration of the
        // delete/reset operation.  Callers must not continue guarded work
        // after releasing it.
        void release()
        {
            if (m_pState && m_bAcquired) {
                m_pState->bOperationInProgress = false;
                m_bAcquired = false;
                if (!m_pState->bOwnerAlive) {
                    m_pState->cleanupContexts();
                }
            }
        }

    private:
        QSharedPointer<UNPACK_GUARD_STATE> m_pState;
        bool m_bAcquired;
        bool m_bAllowed;
    };

    class UNPACK_INFO_AUTHORIZATION {
    public:
        explicit UNPACK_INFO_AUTHORIZATION(const QSharedPointer<UNPACK_GUARD_STATE> &pState)
            : m_pState((pState && pState->bOperationInProgress && !pState->bNestedInfoAuthorized) ? pState : QSharedPointer<UNPACK_GUARD_STATE>())
        {
            if (m_pState) m_pState->bNestedInfoAuthorized = true;
        }

        UNPACK_INFO_AUTHORIZATION(UNPACK_GUARD_FLAG *pBusy, UNPACK_GUARD_FLAG *pNestedInfoAuthorization)
            : UNPACK_INFO_AUTHORIZATION((pBusy && pNestedInfoAuthorization && !pBusy->isNestedAuthorization() && pNestedInfoAuthorization->isNestedAuthorization() &&
                                         (pBusy->state() == pNestedInfoAuthorization->state()))
                                            ? pBusy->state()
                                            : QSharedPointer<UNPACK_GUARD_STATE>())
        {
        }

        ~UNPACK_INFO_AUTHORIZATION()
        {
            // The nested info guard normally consumes the permission.  Clear
            // an unused permission when a call returns before entering it.
            if (m_pState) m_pState->bNestedInfoAuthorized = false;
        }

        bool isAuthorized() const
        {
            return !m_pState.isNull();
        }

    private:
        QSharedPointer<UNPACK_GUARD_STATE> m_pState;
    };

    QObject *getArchiveSourceSessionRegistry(bool bCreate) const;

    // Deliberately not a QObject child: QObject::children() is public and
    // must not expose authorization/session bookkeeping to caller callbacks.
    mutable QObject *m_pArchiveSourceSessionRegistry;
    QSharedPointer<UNPACK_GUARD_STATE> m_pUnpackGuardState;
    UNPACK_GUARD_FLAG m_bUnpackOperationInProgress;
    UNPACK_GUARD_FLAG m_bNestedUnpackInfoAuthorized;

public:
    explicit XArchive(QIODevice *pDevice = nullptr);
    ~XArchive() override;

    virtual quint64 getNumberOfRecords(PDSTRUCT *pPdStruct);               // Depricated
    virtual QList<RECORD> getRecords(qint32 nLimit, PDSTRUCT *pPdStruct);  // Depricated

    virtual bool isResourcesPresent() override;
    virtual QVector<XRESOURCE_STRUCT> getResourceStructs() override;
    virtual QVector<XMETADATA_STRUCT> getMetadataStructs() override;

    struct DECOMPRESSSTRUCT {
        SPINFO spInfo;
        QIODevice *pSourceDevice;
        QIODevice *pDestDevice;
        qint64 nInSize;
        qint64 nOutSize;
        qint64 nDecompressedOffset;
        qint64 nDecompressedLimit;
        qint64 nDecompressedWrote;
        bool bLimit;
    };

    static COMPRESS_RESULT _decompress(DECOMPRESSSTRUCT *pDecompressStruct, PDSTRUCT *pPdStruct = nullptr);
    // The trailing property map is what carries UNPACK_PROP_MAX_OUTPUT_SIZE
    // into the decoder chain. Without it every downstream output gate on this
    // path resolved to -1, so a caller could set a limit and have it silently
    // discarded. The default lives here and NOT on the definition below, which
    // already supplies defaults for the two preceding parameters - repeating it
    // there is a redefinition-of-default-argument error.
    static bool _decompressRecord(const RECORD *pRecord, QIODevice *pSourceDevice, QIODevice *pDestDevice, PDSTRUCT *pPdStruct, qint64 nDecompressedOffset,
                                  qint64 nDecompressedLimit, const QMap<UNPACK_PROP, QVariant> &mapUnpackProperties = QMap<UNPACK_PROP, QVariant>());
    static COMPRESS_RESULT _compress(HANDLE_METHOD compressMethod, QIODevice *pSourceDevice, QIODevice *pDestDevice, PDSTRUCT *pPdStruct = nullptr);
    static COMPRESS_RESULT _compress_deflate(QIODevice *pSourceDevice, QIODevice *pDestDevice, qint32 nLevel, qint32 nMethod, qint32 nWindowsBits, qint32 nMemLevel,
                                             qint32 nStrategy, PDSTRUCT *pPdStruct = nullptr);
    QByteArray decompress(const RECORD *pRecord, PDSTRUCT *pPdStruct = nullptr, qint64 nDecompressedOffset = 0, qint64 nDecompressedLimit = -1,
                          const QMap<UNPACK_PROP, QVariant> &mapUnpackProperties = QMap<UNPACK_PROP, QVariant>());
    QByteArray decompress(QList<RECORD> *pListArchive, const QString &sRecordFileName, PDSTRUCT *pPdStruct = nullptr);
    QByteArray decompress(const QString &sRecordFileName, PDSTRUCT *pPdStruct = nullptr);
    bool decompressToFile(const RECORD *pRecord, const QString &sResultFileName, PDSTRUCT *pPdStruct = nullptr,
                          const QMap<UNPACK_PROP, QVariant> &mapUnpackProperties = QMap<UNPACK_PROP, QVariant>());
    bool decompressToDevice(const RECORD *pRecord, QIODevice *pDestDevice, PDSTRUCT *pPdStruct = nullptr,
                            const QMap<UNPACK_PROP, QVariant> &mapUnpackProperties = QMap<UNPACK_PROP, QVariant>());
    bool decompressToFile(QList<RECORD> *pListArchive, const QString &sRecordFileName, const QString &sResultFileName, PDSTRUCT *pPdStruct = nullptr);
    bool decompressToPath(QList<RECORD> *pListArchive, const QString &sRecordFileName, const QString &sResultPathName, PDSTRUCT *pPdStruct = nullptr);
    bool decompressToFile(const QString &sArchiveFileName, const QString &sRecordFileName, const QString &sResultFileName, PDSTRUCT *pPdStruct = nullptr);
    bool decompressToPath(const QString &sArchiveFileName, const QString &sRecordPathName, const QString &sResultPathName, PDSTRUCT *pPdStruct = nullptr);
    // Resolve a tagged logical record by index through a fresh streaming
    // session.  The expected public record is re-read and matched exactly
    // before extraction; output remains transactional through finishUnpack().
    bool unpackArchiveStreamRecord(const ARCHIVERECORD &expectedRecord, QIODevice *pOutputDevice, const QMap<UNPACK_PROP, QVariant> &mapProperties,
                                   PDSTRUCT *pPdStruct = nullptr);
    bool unpackToFolder(const QString &sResultPathName, PDSTRUCT *pPdStruct = nullptr);
    bool dumpToFile(const RECORD *pRecord, const QString &sFileName, PDSTRUCT *pPdStruct = nullptr);
    static RECORD getArchiveRecord(const QString &sRecordFileName, QList<RECORD> *pListRecords, PDSTRUCT *pPdStruct = nullptr);
    static RECORD getArchiveRecordByUUID(const QString &sUUID, QList<RECORD> *pListRecords, PDSTRUCT *pPdStruct = nullptr);
    bool isArchiveRecordPresent(const QString &sRecordFileName, PDSTRUCT *pPdStruct = nullptr);
    static bool isArchiveRecordPresent(const QString &sRecordFileName, QList<RECORD> *pListRecords, PDSTRUCT *pPdStruct = nullptr);
    static bool isArchiveRecordPresentExp(const QString &sRecordFileName, QList<RECORD> *pListRecords, PDSTRUCT *pPdStruct = nullptr);
    static quint32 getCompressBufferSize();
    static quint32 getDecompressBufferSize();
    static void showRecords(QList<RECORD> *pListArchive);
    virtual QList<FPART_PROP> getAvailableFPARTProperties() override;
    virtual MODE getMode();
    virtual QMap<UNPACK_PROP, QVariant> getDefaultUnpackProperties() override;
    virtual bool unpackCurrent(UNPACK_STATE *pState, QIODevice *pDevice, PDSTRUCT *pPdStruct = nullptr) override;
    //    virtual _MEMORY_MAP getMemoryMap(); // TODO
    virtual qint32 getType();
    virtual QString typeIdToString(qint32 nType);
    virtual bool isArchive();

private:
    static bool _writeToDevice(char *pBuffer, qint32 nBufferSize, DECOMPRESSSTRUCT *pDecompressStruct);
    static QString _normalizeOutputPath(const QString &sPath);
    static bool _isSafeChildPath(const QString &sPath, const QString &sCanonicalRoot);
    INTERNAL_INFO m_internalInfo;
};

#endif  // XARCHIVE_H
