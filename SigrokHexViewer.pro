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

# Avoid linking against MSYS2 runtime libraries that conflict with Qt's MinGW
# Only link the specific libraries we need from MSYS2, not the entire lib directory
LIBS += $$MSYS2_PREFIX/lib/libsigrok.dll.a \
        $$MSYS2_PREFIX/lib/libglib-2.0.dll.a \
        $$MSYS2_PREFIX/lib/libgobject-2.0.dll.a \
        $$MSYS2_PREFIX/lib/libusb-1.0.dll.a \
        -lsetupapi -lws2_32 \
        $$MSYS2_PREFIX/lib/libzip.dll.a \
        -lz

# Enable libsigrokdecode by defining ENABLE_SRD in your build.
contains(DEFINES, ENABLE_SRD) {
    LIBS += $$MSYS2_PREFIX/lib/libsigrokdecode.dll.a
}

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    sigrokworker.cpp

HEADERS += \
    mainwindow.h \
    sigrokworker.h
