INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/xformats.h

SOURCES += \
    $$PWD/xformats.cpp

!contains(XCONFIG, xbinary) {
    XCONFIG += xbinary
    include($$PWD/xbinary.pri)
}

!contains(XCONFIG, xcom) {
    XCONFIG += xcom
    include($$PWD/xcom.pri)
}

!contains(XCONFIG, xmsdos) {
    XCONFIG += xmsdos
    include($$PWD/xmsdos.pri)
}

!contains(XCONFIG, xne) {
    XCONFIG += xne
    include($$PWD/xne.pri)
}

!contains(XCONFIG, xle) {
    XCONFIG += xle
    include($$PWD/xle.pri)
}

!contains(XCONFIG, xpe) {
    XCONFIG += xpe
    include($$PWD/xpe.pri)
}

!contains(XCONFIG, xmach) {
    XCONFIG += xmach
    include($$PWD/xmach.pri)
}

!contains(XCONFIG, xelf) {
    XCONFIG += xelf
    include($$PWD/xelf.pri)
}

contains(XCONFIG, use_dex) {
    DEFINES += USE_DEX

    !contains(XCONFIG, xdex) {
        XCONFIG += xdex
        include($$PWD/../XDEX/xdex.pri)
    }
}

contains(XCONFIG, use_archive) {
    DEFINES += USE_ARCHIVE

    !contains(XCONFIG, xarchives) {
        XCONFIG += xarchives
        include($$PWD/../XArchive/xarchives.pri)
    }
}
