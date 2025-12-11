INCLUDEPATH += $$PWD/src
DEPENDPATH += $$PWD/src

win32-g++ {
    LIBS += $$PWD/libs/libxsimd-win-$${QT_ARCH}.a
}
win32-msvc* {
    LIBS += $$PWD/libs/xsimd-win-$${QT_ARCH}.lib
}
unix:!macx {
    LIBS += $$PWD/libs/libxsimd-unix-$${QT_ARCH}.a
}
unix:macx {
    LIBS += $$PWD/libs/libxsimd-macos-$${QT_ARCH}.a
}
