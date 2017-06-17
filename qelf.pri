INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/qelf.h \
    $$PWD/qelf_def.h

SOURCES += \
    $$PWD/qelf.cpp

!contains(XCONFIG, qmsdos) {
    XCONFIG += qmsdos
    include(qmsdos.pri)
}
