INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/xamigahunk.h

SOURCES += \
    $$PWD/xamigahunk.cpp

!contains(XCONFIG, xbinary) {
    XCONFIG += xbinary
    include($$PWD/xbinary.pri)
}
