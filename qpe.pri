INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/qpe.h \
    $$PWD/qpe_def.h

SOURCES += \
    $$PWD/qpe.cpp

!contains(XCONFIG, qmsdos) {
    XCONFIG += qmsdos
    include(qmsdos.pri)
}
