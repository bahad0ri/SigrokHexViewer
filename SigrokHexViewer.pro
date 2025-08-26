QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += core gui widgets

CONFIG += c++17

TEMPLATE = app
TARGET = SigrokHexViewer

INCLUDEPATH += C:/msys64/mingw64/include
INCLUDEPATH += C:/msys64/mingw64/include/glib-2.0
INCLUDEPATH += C:/msys64/mingw64/lib/glib-2.0/include

LIBS += -LC:/msys64/mingw64/lib \
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
