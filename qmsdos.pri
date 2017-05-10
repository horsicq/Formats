INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/qmsdos.h \
    $$PWD/qmsdos_def.h

SOURCES += \
    $$PWD/qmsdos.cpp

!contains(XCONFIG, qbinary) {
    XCONFIG += qbinary
    include(qbinary.pri)
}
    
