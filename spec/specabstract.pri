INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/specabstract.h

SOURCES += \
    $$PWD/specabstract.cpp

!contains(XCONFIG, xpe) {
    XCONFIG += xpe
    include(../xpe.pri)
}

!contains(XCONFIG, xelf) {
    XCONFIG += xelf
    include(../xelf.pri)
}

!contains(XCONFIG, xmach) {
    XCONFIG += xmach
    include(../xmach.pri)
}

!contains(XCONFIG, xarchive) {
    XCONFIG += xarchive
    include(../../XArchive/xarchive.pri)
}
