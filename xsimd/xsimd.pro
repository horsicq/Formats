QT -= core gui

TARGET = xsimd
TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += $$PWD/xsimd
DEPENDPATH += $$PWD/xsimd

include(../../build.pri)

# Enable SIMD instructions (SSE, AVX, AVX2)
win32-msvc* {
    QMAKE_CFLAGS += /arch:AVX2
    QMAKE_CXXFLAGS += /arch:AVX2
}
else:gcc|clang {
    QMAKE_CFLAGS += -msse -msse2 -mavx -mavx2
    QMAKE_CXXFLAGS += -msse -msse2 -mavx -mavx2
}

win32{
    TARGET = xsimd-win-$${QT_ARCH}
}
unix:!macx {
    TARGET = xsimd-unix-$${QT_ARCH}
}
unix:macx {
    TARGET = xsimd-macos-$${QT_ARCH}
}

SOURCES += \
    $$PWD/src/xsimd.c
	
TARGETLIB_PATH = $$PWD

DESTDIR=$${TARGETLIB_PATH}/libs
