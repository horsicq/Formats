# Enable AVX2 for optimized binary operations on x86/x64 architectures
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|i686|i386")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")
        #add_definitions(-DXBINARY_USE_AVX2)
    elseif(MSVC)
        #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX2")
        #add_definitions(-DXBINARY_USE_AVX2)
    endif()
endif()

include_directories(${CMAKE_CURRENT_LIST_DIR})
include_directories(${CMAKE_CURRENT_LIST_DIR}/images)
include_directories(${CMAKE_CURRENT_LIST_DIR}/video)
include_directories(${CMAKE_CURRENT_LIST_DIR}/audio)
include_directories(${CMAKE_CURRENT_LIST_DIR}/exec)
include_directories(${CMAKE_CURRENT_LIST_DIR}/archives)
include_directories(${CMAKE_CURRENT_LIST_DIR}/Algos)
include_directories(${CMAKE_CURRENT_LIST_DIR}/texts)
include_directories(${CMAKE_CURRENT_LIST_DIR}/formats)
include_directories(${CMAKE_CURRENT_LIST_DIR}/xsimd/src)

# The ZIP/decompression core below is owned by this file, but its sources still
# live in the XArchive folder (XCompress/XDecompress, the compressed-device
# helpers and every Algos decoder). XAndroidBinary comes from XDEX -- xapk.cpp
# includes it directly.
include_directories(${CMAKE_CURRENT_LIST_DIR}/../XArchive)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../XArchive/archives)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../XArchive/compressors)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../XArchive/core)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../XArchive/diskimages)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../XArchive/documents)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../XArchive/filesystems)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../XArchive/games)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../XArchive/installers)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../XArchive/packages)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../XArchive/sfx)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../XArchive/tar)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../XArchive/transport)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/include)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../XDEX)

# ---------------------------------------------------------------------------
# ZIP / decompression core -- formerly XArchive/xzip.cmake.
#
# Everything the ZIP family needs and nothing that needs USE_ARCHIVE: XArchive
# itself, the ZIP/JAR/APK/IPA/GZIP/compressed-TAR/NPM/ISO9660 readers,
# MACHOFat, DOS16, XCompress/XDecompress and ALL Algos decoders, plus the
# XBinary/XOptions/XMACH/XPNG/XJavaClass/XAndroidBinary sources they call into.
#
# It keeps its own name, XZIP_SOURCES, and its own entry point: a caller that
# wants the core WITHOUT the rest of Formats -- xarchive.cmake, XUpdate,
# FormatDialogs/dialogtextinfo -- sets XFORMATS_CORE_ONLY, include()s this
# file, and gets exactly that.
#
# XFORMATS_CORE_ONLY is not a convenience, it is what keeps the module
# protocol intact. Every consumer in the tree composes XFormats with
# `if (NOT DEFINED XFORMATS_SOURCES) include(...) <fold it in> endif()`, so
# whichever file triggers the include is the one that folds the result into
# the target. If a core-only caller defined XFORMATS_SOURCES on the way past,
# the next consumer's test would fail, the fold would never happen, and the
# build would end at LNK2001 on XPE/XNE -- compiled by nobody, because
# XFORMATS_SOURCES reached no target. Returning before it is defined keeps the
# core-only path invisible to that protocol.
#
# The zlib / bzip2 / lzma / ppmd amalgamations are part of this list, so a
# consumer needs no codec library and no extra target_link_libraries entry: the
# core is self-contained the way the qmake build in XArchive/xzip.pri already
# was.
# ---------------------------------------------------------------------------
if (NOT DEFINED XZIP_SOURCES)
    # Carried over from the deleted XArchive/ancient.cmake, which set these two
    # in whatever directory include()d it -- so every consumer of this file has
    # had them since the ancient codecs joined the core. Kept verbatim rather
    # than dropped, because losing the pin would silently change the standard
    # the whole directory compiles at.
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    if (NOT DEFINED XBINARY_SOURCES)
        include(${CMAKE_CURRENT_LIST_DIR}/xbinary.cmake)
    endif()
    if (NOT DEFINED XOPTIONS_SOURCES)
        include(${CMAKE_CURRENT_LIST_DIR}/../XOptions/xoptions.cmake)
    endif()
    # XMACHOFat calls non-inline helpers from XMACH.
    if (NOT DEFINED XMACH_SOURCES)
        include(${CMAKE_CURRENT_LIST_DIR}/exec/xmach.cmake)
    endif()

    # The groups below are listed unconditionally so XZIP_SOURCES is complete on
    # its own: a core-only consumer has no other list to fall back on, and the
    # overlap with XFORMATS_SOURCES/XDEX_SOURCES costs nothing because CMake
    # keeps one entry per source path in a target.
    set(XZIP_SOURCES
        ${XBINARY_SOURCES}
        ${XOPTIONS_SOURCES}
        ${XMACH_SOURCES}
        # The XAncientDecoder codecs (DMS, Freeze, PowerPacker, UnixPack, RNC,
        # TPWM), formerly XArchive/ancient.cmake. They belong to the core, not
        # to the USE_ARCHIVE layer: core/xdecompress.cpp calls
        # XAncientDecoder::decode() for TYPE_RNC and TYPE_UNIX_PACK.
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancientdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancientdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancientcodecbase_p.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancientcodecbase_p.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancientdmsdecoder_p.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancientdmsdecoder_p.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancientfreezedecoder_p.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancientfreezedecoder_p.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancientpowerpackerdecoder_p.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancientpowerpackerdecoder_p.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancientunixpackdecoder_p.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancientunixpackdecoder_p.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancientrncdecoder_p.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancientrncdecoder_p.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancienttpwmdecoder_p.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancienttpwmdecoder_p.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancientdynamichuffman_p.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancienthuffman_p.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancientinputstream_p.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancientinputstream_p.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancientoutputstream_p.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancientoutputstream_p.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancientvlc_p.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancientxpk_p.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancientbuffer_p.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancientbuffer_p.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancientcrc16_p.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancientcrc16_p.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancientcommon_p.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancientcommon_p.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancientmemorybuffer_p.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancientmemorybuffer_p.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancientoverflow_p.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancientstaticbuffer_p.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancientstaticbuffer_p.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancientsubbuffer_p.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancientsubbuffer_p.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancientvectorbuffer_p.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xancientvectorbuffer_p.h
        # zlib / bzip2 / lzma / ppmd, compiled straight into the consumer's
        # target from the Algos amalgamations. They used to be four static
        # libraries declared in XArchive/algos_codecs.cmake, which every project
        # had to include() and then name in target_link_libraries; a project that
        # forgot either half got a bare word that CMake silently demoted to a raw
        # linker input and a link that failed on "inflate". Listing the sources
        # here removes both halves and matches the qmake build, which has always
        # compiled these same files directly (XArchive/xzip.pri).
        #
        # They were converted from C to C++ on 2026-08-17 and must stay C++:
        # several internal symbols (LzmaDec_InitDicAndState, Ppmd7/8_UpdateModel,
        # ...) are declared in no header, so their linkage names follow the
        # language each unit is compiled as. Each file carries an
        # #ifndef __cplusplus / #error guard.
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/zlibutil.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/zlibdeclib.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/zlibenclib.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/bzip2declib.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/sevenzip_extcodec_lzmadec.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/sevenzip_extcodec_lzma2dec.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/sevenzip_extcodec_ppmd7.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/sevenzip_extcodec_ppmd7dec.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/sevenzip_extcodec_ppmd8.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/sevenzip_extcodec_ppmd8dec.cpp
        # xdecompress.cpp needs XPNG, xjar needs XJavaClass, xapk needs XAndroidBinary.
        ${CMAKE_CURRENT_LIST_DIR}/images/xpng.cpp
        ${CMAKE_CURRENT_LIST_DIR}/images/xpng.h
        ${CMAKE_CURRENT_LIST_DIR}/formats/xjavaclass.cpp
        ${CMAKE_CURRENT_LIST_DIR}/formats/xjavaclass.h
        ${CMAKE_CURRENT_LIST_DIR}/../XDEX/xandroidbinary.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XDEX/xandroidbinary.h
        ${CMAKE_CURRENT_LIST_DIR}/xarchive.cpp
        ${CMAKE_CURRENT_LIST_DIR}/xarchive.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/compressors/xcompress.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/compressors/xcompress.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/core/xdecompress.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/core/xdecompress.h
        # Decoders dispatched from xdecompress.cpp: they must live in the shared core
        # (this file), not in xarchive.cmake, or a USE_ARCHIVE-less consumer such as
        # xbinaryviewerc fails to link (LNK2019 XAldusDecoder/XAMPKDecoder/...).
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xaldusdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xaldusdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xampkdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xampkdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xclaydecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xcopyqmdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xdiskimagedecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xchieflzdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xhadecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlimdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xaindecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xobfuscationdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xuleaddecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xtopspeeddecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xpakleodecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xtpsdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xzxzipdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/ximpdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xsfpackdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/diskimages/xvmdkarchive.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xsqxdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xvmarcdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xtersedecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xsquashfsdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xvmssavesetdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xtarx2decoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xpanoramadecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xziedecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xteledeskdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xqdadecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xc64wraptordecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xvmsdatabasedecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xtarx1decoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xvmspcsidecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xzcmpdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xzpakdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xztcdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xcharcdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xwintersoftdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xtivolidecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xzoomdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xwpkdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlzhufdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xti99arcdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xxeditpackdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xhadecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlimdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xaindecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xobfuscationdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xuleaddecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xtopspeeddecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xpakleodecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xtpsdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xzxzipdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/ximpdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xsfpackdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/diskimages/xvmdkarchive.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xsqxdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xvmarcdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xtersedecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xsquashfsdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xvmssavesetdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xtarx2decoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xpanoramadecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xziedecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xteledeskdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xqdadecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xc64wraptordecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xvmsdatabasedecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xtarx1decoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xvmspcsidecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xzcmpdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xzpakdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xztcdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xcharcdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xwintersoftdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xtivolidecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xzoomdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xwpkdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlzhufdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xti99arcdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xxeditpackdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xchieflzdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xdiskimagedecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xcopyqmdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xkboomdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xkboomdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xclaydecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xcmpdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xcmpdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xsharedlzwdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xsharedlzwdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xarcv2decoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xarcv2decoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xasymetrixdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xasymetrixdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xborlandpackdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xborlandpackdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xbzip1decoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xbzip1decoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xbsndecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xbsndecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xbthpakdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xbthpakdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xcreateinstalldecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xcreateinstalldecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xdcldecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xdcldecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xgeniuslibrarydecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xgeniuslibrarydecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xnintendolzdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xnintendolzdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xash0decoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xash0decoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/core/xcompresseddevice.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/core/xcompresseddevice.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/core/xcompanionfile.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/core/xcompanionfile.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/core/xvolumesetdevice.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/core/xvolumesetdevice.h
        ${CMAKE_CURRENT_LIST_DIR}/archives/xtar.cpp
        ${CMAKE_CURRENT_LIST_DIR}/archives/xtar.h
        ${CMAKE_CURRENT_LIST_DIR}/archives/xtarcompressed.cpp
        ${CMAKE_CURRENT_LIST_DIR}/archives/xtarcompressed.h
        ${CMAKE_CURRENT_LIST_DIR}/archives/xzip.cpp
        ${CMAKE_CURRENT_LIST_DIR}/archives/xzip.h
        ${CMAKE_CURRENT_LIST_DIR}/archives/xjar.cpp
        ${CMAKE_CURRENT_LIST_DIR}/archives/xjar.h
        ${CMAKE_CURRENT_LIST_DIR}/archives/xapk.cpp
        ${CMAKE_CURRENT_LIST_DIR}/archives/xapk.h
        ${CMAKE_CURRENT_LIST_DIR}/archives/xapks.cpp
        ${CMAKE_CURRENT_LIST_DIR}/archives/xapks.h
        ${CMAKE_CURRENT_LIST_DIR}/archives/xipa.cpp
        ${CMAKE_CURRENT_LIST_DIR}/archives/xipa.h
        ${CMAKE_CURRENT_LIST_DIR}/archives/xgzip.cpp
        ${CMAKE_CURRENT_LIST_DIR}/archives/xgzip.h
        ${CMAKE_CURRENT_LIST_DIR}/archives/xiso9660.cpp
        ${CMAKE_CURRENT_LIST_DIR}/archives/xiso9660.h
        ${CMAKE_CURRENT_LIST_DIR}/archives/xtar_gz.cpp
        ${CMAKE_CURRENT_LIST_DIR}/archives/xtar_gz.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/tar/xtar_compress.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/tar/xtar_compress.h
        ${CMAKE_CURRENT_LIST_DIR}/archives/xnpm.cpp
        ${CMAKE_CURRENT_LIST_DIR}/archives/xnpm.h
        ${CMAKE_CURRENT_LIST_DIR}/exec/xmachofat.cpp
        ${CMAKE_CURRENT_LIST_DIR}/exec/xmachofat.h
        ${CMAKE_CURRENT_LIST_DIR}/exec/xdos16.cpp
        ${CMAKE_CURRENT_LIST_DIR}/exec/xdos16.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/algo_utils.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/algo_utils.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xalgo_local.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xarcdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xarcdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xarjdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xarjdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xacedecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xacedecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xucldecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xucldecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xucldecoder_acc.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlha_legacy_lzs_p.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlha_legacy_lz5_p.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlha_legacy_lhx_p.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlha_legacy_lk7_p.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlha_legacy_pm1_p.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlha_legacy_pm2_p.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlzhdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlzhdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlha_legacy_p.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xkwajlzssdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xkwajlzssdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xkwajlzhdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xkwajlzhdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xrardecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xrardecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xit214decoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xit214decoder.h
        ${CMAKE_CURRENT_LIST_DIR}/Algos/xdeflatedecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Algos/xdeflatedecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/ximplodedecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/ximplodedecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlzmadecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlzmadecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlzwdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlzwdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xnewwavelzwdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xnewwavelzwdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xascii85decoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xascii85decoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xasciihexdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xasciihexdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xrunlengthdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xrunlengthdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/Algos/xstoredecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Algos/xstoredecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xspisrledecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xspisrledecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xamigalzxdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xamigalzxdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xmaclegacydecoders.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xmaclegacydecoders.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xpaxdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xpaxdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xvisedeflatedecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xvisedeflatedecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xmi10decoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xmi10decoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xfpakdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xfpakdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xftcompdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xftcompdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xdndecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xdndecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xsqzdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xsqzdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xflsdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xflsdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xpakdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xpakdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xssmdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xssmdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xrtpatchdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xrtpatchdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xbzip2decoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xbzip2decoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xadcdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xadcdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xbrotlidecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xbrotlidecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlzssdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlzssdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xcoktellzdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xcoktellzdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xwinzipjpegdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xwinzipjpegdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xwavpackdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xwavpackdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xshrinkdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xshrinkdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xreducedecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xreducedecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xzipcryptodecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xzipcryptodecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xppmddecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xppmddecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xppmdrangedecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xppmdrangedecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xppmdmodel.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xppmdmodel.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xppmd7model.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xppmd7model.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xaesdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xaesdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xbcj2decoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xbcj2decoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xbranchdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xbranchdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlzxdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlzxdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xquantumdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xquantumdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xxpressdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xxpressdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xsha256decoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xsha256decoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xblake2sp.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xblake2sp.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xzstddecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xzstddecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlz4decoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlz4decoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlz5decoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlz5decoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlizarddecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlizarddecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/lz5lizarddeclib.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/lz4declib.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/include/wavpack.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/zstdlegacydeclib.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/zstddeclib.cpp
        # The five entries below were converted from C to C++ on 2026-08-17 and must
        # stay C++: they define ZSTDv05/06/07_* and ZBUFFv07_* symbols that no header
        # declares, so a C build of one and a C++ build of its caller fail at link,
        # not at compile. Each file carries an #ifndef __cplusplus / #error guard.
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/zstd_xxhash.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/zstdlegacy_v04.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/zstdlegacy_v05.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/zstdlegacy_v06.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/zstdlegacy_v07.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlzodecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlzodecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xcompressdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xcompressdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xkolibrikpackdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xmathcaddecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xpcommos2decoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xnetwarepackdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xearefpackdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlzpis2decoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xnpackdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xcorelltecdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xirwinpacdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xgashuffdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xsilmarilsdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xrawlzw15vdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xriddecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xrompaqdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xarcv4decoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xealzwdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xslsdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xpcsecuredecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xqnxbasedecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xhuffdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlzhcxpdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xdsquantumdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xgenteedecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xpktdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xhdcopydecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xstylusdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xsettlersftdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xsqdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xis11decoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xpaperportdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xealibdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xniddecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xhapdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlzdietdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlzv1decoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xsafdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xhfedecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xrsvkdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xhzldecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlofidecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xinfogramespakdecoder.h
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/lz4declib.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/wavpackdeclib.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/lz5lizarddeclib.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xkolibrikpackdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xmathcaddecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xpcommos2decoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xnetwarepackdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xearefpackdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlzpis2decoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xnpackdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xcorelltecdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xirwinpacdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xgashuffdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xsilmarilsdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xrawlzw15vdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xriddecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xrompaqdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xarcv4decoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xealzwdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xslsdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xpcsecuredecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xqnxbasedecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xhuffdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlzhcxpdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xdsquantumdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xgenteedecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xpktdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xhdcopydecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xstylusdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xsettlersftdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xsqdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xis11decoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xpaperportdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xealibdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xniddecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xhapdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlzdietdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlzv1decoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xsafdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xhfedecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xrsvkdecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xhzldecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xlofidecoder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/xinfogramespakdecoder.cpp
    )

    # The amalgamated inflate/deflate is prohibitively slow at -O0 (large
    # installer streams take minutes merely to enumerate), so its former target
    # optimized it even in Debug. The source property carries that over now that
    # there is no target to set it on.
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set_source_files_properties(
            ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/zlibutil.cpp
            ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/zlibdeclib.cpp
            ${CMAKE_CURRENT_LIST_DIR}/../XArchive/Algos/zlibenclib.cpp
            PROPERTIES COMPILE_OPTIONS "$<$<CONFIG:Debug>:-O2>")
    endif()
endif()

# Core-only entry point: stop here, leaving XFORMATS_SOURCES undefined so the
# consumer that composes the full library still does the fold (see above).
if (XFORMATS_CORE_ONLY)
    return()
endif()

# A second full include() is a no-op rather than a second copy of every path.
if (DEFINED XFORMATS_SOURCES)
    return()
endif()

if (NOT DEFINED XDEX_SOURCES)
    include(${CMAKE_CURRENT_LIST_DIR}/../XDEX/xdex.cmake)
    set(XFORMATS_SOURCES ${XFORMATS_SOURCES} ${XDEX_SOURCES})
endif()
if (NOT DEFINED XPDF_SOURCES)
    include(${CMAKE_CURRENT_LIST_DIR}/../XPDF/xpdf.cmake)
    set(XFORMATS_SOURCES ${XFORMATS_SOURCES} ${XPDF_SOURCES})
endif()

set(XFORMATS_SOURCES ${XFORMATS_SOURCES} ${XZIP_SOURCES})

# Match the qmake feature boundary: callers opt in to the formats that remain
# in XArchive by defining USE_ARCHIVE.  Without it the build stops at the ZIP
# core assembled above.  XFORMATS_USE_ARCHIVE is an explicit override for
# callers that configure sources before adding their compile definitions.
if (DEFINED XFORMATS_USE_ARCHIVE)
    set(_XFORMATS_USE_ARCHIVE ${XFORMATS_USE_ARCHIVE})
else()
    get_directory_property(_XFORMATS_COMPILE_DEFINITIONS COMPILE_DEFINITIONS)
    list(FIND _XFORMATS_COMPILE_DEFINITIONS "USE_ARCHIVE" _XFORMATS_USE_ARCHIVE_INDEX)
    if (_XFORMATS_USE_ARCHIVE_INDEX GREATER -1)
        set(_XFORMATS_USE_ARCHIVE TRUE)
    else()
        set(_XFORMATS_USE_ARCHIVE FALSE)
    endif()
endif()

if (_XFORMATS_USE_ARCHIVE)
    if (NOT DEFINED XARCHIVES_SOURCES)
        include(${CMAKE_CURRENT_LIST_DIR}/../XArchive/xarchives.cmake)
        set(XFORMATS_SOURCES ${XFORMATS_SOURCES} ${XARCHIVES_SOURCES})
    endif()
endif()

unset(_XFORMATS_COMPILE_DEFINITIONS)
unset(_XFORMATS_USE_ARCHIVE_INDEX)
unset(_XFORMATS_USE_ARCHIVE)

# TODO

set(XFORMATS_SOURCES
    ${XFORMATS_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/xbinary.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xbinary.h
    ${CMAKE_CURRENT_LIST_DIR}/xbinary_def.h
    ${CMAKE_CURRENT_LIST_DIR}/xmetadataappender.h
    ${CMAKE_CURRENT_LIST_DIR}/subdevice.cpp
    ${CMAKE_CURRENT_LIST_DIR}/subdevice.h
    ${CMAKE_CURRENT_LIST_DIR}/xformats.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xformats.h
    ${CMAKE_CURRENT_LIST_DIR}/xsearchprocess.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xsearchprocess.h
    ${CMAKE_CURRENT_LIST_DIR}/audio/xmp3.cpp
    ${CMAKE_CURRENT_LIST_DIR}/audio/xmp3.h
    ${CMAKE_CURRENT_LIST_DIR}/audio/xmdh.cpp
    ${CMAKE_CURRENT_LIST_DIR}/audio/xmdh.h
    ${CMAKE_CURRENT_LIST_DIR}/audio/xmus.cpp
    ${CMAKE_CURRENT_LIST_DIR}/audio/xmus.h
    ${CMAKE_CURRENT_LIST_DIR}/audio/xpma.cpp
    ${CMAKE_CURRENT_LIST_DIR}/audio/xpma.h
    ${CMAKE_CURRENT_LIST_DIR}/audio/xsnd.cpp
    ${CMAKE_CURRENT_LIST_DIR}/audio/xsnd.h
    ${CMAKE_CURRENT_LIST_DIR}/audio/xsm8.cpp
    ${CMAKE_CURRENT_LIST_DIR}/audio/xsm8.h
    ${CMAKE_CURRENT_LIST_DIR}/audio/xwav.cpp
    ${CMAKE_CURRENT_LIST_DIR}/audio/xwav.h
    ${CMAKE_CURRENT_LIST_DIR}/audio/xxm.cpp
    ${CMAKE_CURRENT_LIST_DIR}/audio/xxm.h
    ${CMAKE_CURRENT_LIST_DIR}/formats/xdtc.cpp
    ${CMAKE_CURRENT_LIST_DIR}/formats/xdtc.h
    ${CMAKE_CURRENT_LIST_DIR}/formats/xdma.cpp
    ${CMAKE_CURRENT_LIST_DIR}/formats/xdma.h
    ${CMAKE_CURRENT_LIST_DIR}/images/xbmp.cpp
    ${CMAKE_CURRENT_LIST_DIR}/images/xbmp.h
    ${CMAKE_CURRENT_LIST_DIR}/images/xgif.cpp
    ${CMAKE_CURRENT_LIST_DIR}/images/xgif.h
    ${CMAKE_CURRENT_LIST_DIR}/images/xicc.cpp
    ${CMAKE_CURRENT_LIST_DIR}/images/xicc.h
    ${CMAKE_CURRENT_LIST_DIR}/images/xicon.cpp
    ${CMAKE_CURRENT_LIST_DIR}/images/xicon.h
    ${CMAKE_CURRENT_LIST_DIR}/images/xjfif.cpp
    ${CMAKE_CURRENT_LIST_DIR}/images/xjfif.h
    ${CMAKE_CURRENT_LIST_DIR}/images/xjpeg.cpp
    ${CMAKE_CURRENT_LIST_DIR}/images/xjpeg.h
    ${CMAKE_CURRENT_LIST_DIR}/images/xpng.cpp
    ${CMAKE_CURRENT_LIST_DIR}/images/xpng.h
    ${CMAKE_CURRENT_LIST_DIR}/images/xtiff.cpp
    ${CMAKE_CURRENT_LIST_DIR}/images/xtiff.h
    ${CMAKE_CURRENT_LIST_DIR}/images/xwebp.cpp
    ${CMAKE_CURRENT_LIST_DIR}/images/xwebp.h
    ${CMAKE_CURRENT_LIST_DIR}/video/xmp4.cpp
    ${CMAKE_CURRENT_LIST_DIR}/video/xmp4.h
    ${CMAKE_CURRENT_LIST_DIR}/video/xavi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/video/xavi.h
    ${CMAKE_CURRENT_LIST_DIR}/formats/xriff.cpp
    ${CMAKE_CURRENT_LIST_DIR}/formats/xriff.h
    ${CMAKE_CURRENT_LIST_DIR}/formats/xder.h
    ${CMAKE_CURRENT_LIST_DIR}/formats/xder.cpp
    ${CMAKE_CURRENT_LIST_DIR}/formats/xpyc.h
    ${CMAKE_CURRENT_LIST_DIR}/formats/xpyc.cpp
    ${CMAKE_CURRENT_LIST_DIR}/formats/xwasm.h
    ${CMAKE_CURRENT_LIST_DIR}/formats/xwasm.cpp
    ${CMAKE_CURRENT_LIST_DIR}/exec/xamigahunk.cpp
    ${CMAKE_CURRENT_LIST_DIR}/exec/xamigahunk.h
    ${CMAKE_CURRENT_LIST_DIR}/exec/xamigahunk_def.h
    ${CMAKE_CURRENT_LIST_DIR}/exec/xatarist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/exec/xatarist.h
    ${CMAKE_CURRENT_LIST_DIR}/exec/xatarist_def.h
    ${CMAKE_CURRENT_LIST_DIR}/exec/xcliassembly.cpp
    ${CMAKE_CURRENT_LIST_DIR}/exec/xcliassembly.h
    ${CMAKE_CURRENT_LIST_DIR}/exec/xcliassembly_def.h
    ${CMAKE_CURRENT_LIST_DIR}/exec/xcom.cpp
    ${CMAKE_CURRENT_LIST_DIR}/exec/xcom.h
    ${CMAKE_CURRENT_LIST_DIR}/exec/xcom_def.h
    ${CMAKE_CURRENT_LIST_DIR}/exec/xelf.cpp
    ${CMAKE_CURRENT_LIST_DIR}/exec/xelf.h
    ${CMAKE_CURRENT_LIST_DIR}/exec/xelf_def.h
    ${CMAKE_CURRENT_LIST_DIR}/exec/xle.cpp
    ${CMAKE_CURRENT_LIST_DIR}/exec/xle.h
    ${CMAKE_CURRENT_LIST_DIR}/exec/xle_def.h
    ${CMAKE_CURRENT_LIST_DIR}/exec/xmach.cpp
    ${CMAKE_CURRENT_LIST_DIR}/exec/xmach.h
    ${CMAKE_CURRENT_LIST_DIR}/exec/xmach_def.h
    ${CMAKE_CURRENT_LIST_DIR}/exec/xmsdos.cpp
    ${CMAKE_CURRENT_LIST_DIR}/exec/xmsdos.h
    ${CMAKE_CURRENT_LIST_DIR}/exec/xmsdos_def.h
    ${CMAKE_CURRENT_LIST_DIR}/exec/xne.cpp
    ${CMAKE_CURRENT_LIST_DIR}/exec/xne.h
    ${CMAKE_CURRENT_LIST_DIR}/exec/xne_def.h
    ${CMAKE_CURRENT_LIST_DIR}/exec/xpe.cpp
    ${CMAKE_CURRENT_LIST_DIR}/exec/xpe.h
    ${CMAKE_CURRENT_LIST_DIR}/exec/xpe_def.h
    ${CMAKE_CURRENT_LIST_DIR}/formats/xjavaclass.h
    ${CMAKE_CURRENT_LIST_DIR}/formats/xjavaclass.cpp
    ${CMAKE_CURRENT_LIST_DIR}/formats/xttf.h
    ${CMAKE_CURRENT_LIST_DIR}/formats/xttf.cpp
    ${CMAKE_CURRENT_LIST_DIR}/formats/xdjvu.h
    ${CMAKE_CURRENT_LIST_DIR}/formats/xdjvu.cpp
    ${CMAKE_CURRENT_LIST_DIR}/texts/xtext.cpp
    ${CMAKE_CURRENT_LIST_DIR}/texts/xtext.h
)
