INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

#macx {
#     LIBS += "-framework CoreFoundation"
#}

HEADERS += \
    $$PWD/qbinary.h \
    $$PWD/subdevice.h

SOURCES += \
    $$PWD/qbinary.cpp \
    $$PWD/subdevice.cpp
