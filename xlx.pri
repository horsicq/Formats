INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/xlx.h \
    $$PWD/xlx_def.h

SOURCES += \
    $$PWD/xlx.cpp

!contains(XCONFIG, xmsdos) {
    XCONFIG += xmsdos
    include(xmsdos.pri)
}
