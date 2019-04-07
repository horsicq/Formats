INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/xpe_def.h \
    $$PWD/xpe.h

SOURCES += \
    $$PWD/xpe.cpp

!contains(XCONFIG, xmsdos) {
    XCONFIG += xmsdos
    include(xmsdos.pri)
}
