INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/qmach.h \
    $$PWD/qmach_def.h

SOURCES += \
    $$PWD/qmach.cpp

!contains(XCONFIG, qbinary) {
    XCONFIG += qbinary
    include(qbinary.pri)
}
