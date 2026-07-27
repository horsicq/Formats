INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/xcliassembly.h \
    $$PWD/xcliassembly_def.h

SOURCES += \
    $$PWD/xcliassembly.cpp

!contains(XCONFIG, xbinary) {
    XCONFIG += xbinary
    include($$PWD/../xbinary.pri)
}
