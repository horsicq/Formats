INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/specabstract.h

SOURCES += \
    $$PWD/specabstract.cpp

!contains(XCONFIG, qpe) {
    XCONFIG += qpe
    include(../qpe.pri)
}

!contains(XCONFIG, qelf) {
    XCONFIG += qelf
    include(../qelf.pri)
}
