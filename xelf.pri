INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/xelf_def.h \
    $$PWD/xelf.h

SOURCES += \
    $$PWD/xelf.cpp

!contains(XCONFIG, xbinary) {
    XCONFIG += xbinary
    include(xbinary.pri)
}
