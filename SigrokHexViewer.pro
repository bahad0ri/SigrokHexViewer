QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += core gui widgets

CONFIG += c++17

TEMPLATE = app
TARGET = SigrokHexViewer

# Select the proper MSYS2 prefix depending on whether a 32- or 64-bit
# MinGW toolchain is used.  This avoids mixing architectures which leads
# to "file format not recognized" link errors.
MSYS2_PREFIX = C:/msys64/mingw64
equals(QT_ARCH, "i386") {
    MSYS2_PREFIX = C:/msys64/mingw32
}

INCLUDEPATH += $$MSYS2_PREFIX/include
INCLUDEPATH += $$MSYS2_PREFIX/include/glib-2.0
INCLUDEPATH += $$MSYS2_PREFIX/lib/glib-2.0/include

LIBS += -L$$MSYS2_PREFIX/lib \
        -lsigrok \
        -lglib-2.0 -lgobject-2.0 \
        -lusb-1.0 -lsetupapi -lws2_32 \
        -lzip -lz -lwinpthread

# Enable libsigrokdecode by defining ENABLE_SRD in your build.
contains(DEFINES, ENABLE_SRD) {
    LIBS += -lsigrokdecode
}

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    sigrokworker.cpp

HEADERS += \
    mainwindow.h \
    sigrokworker.h
