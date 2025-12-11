QT -= core gui

CONFIG += c++11 console
CONFIG -= app_bundle

TARGET = xsimd
TEMPLATE = lib
CONFIG += staticlib

DEFINES += QT_DEPRECATED_WARNINGS

# Enable SIMD instructions (SSE, AVX, AVX2)
win32-msvc* {
    QMAKE_CFLAGS += /arch:AVX2
    QMAKE_CXXFLAGS += /arch:AVX2
}
else:gcc|clang {
    QMAKE_CFLAGS += -msse -msse2 -mavx -mavx2
    QMAKE_CXXFLAGS += -msse -msse2 -mavx -mavx2
}

SOURCES += \
    src/xsimd.c

HEADERS += \
    src/xsimd.h
