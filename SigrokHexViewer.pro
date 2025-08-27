QT += core gui widgets
CONFIG += c++17
TEMPLATE = app
TARGET = SigrokHexViewer

MSYS2_PREFIX = C:/msys64/mingw64

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    sigrokworker.cpp

HEADERS += \
    mainwindow.h \
    sigrokworker.h

FORMS += mainwindow.ui

INCLUDEPATH += \
    $$MSYS2_PREFIX/include \
    $$MSYS2_PREFIX/include/glib-2.0 \
    $$MSYS2_PREFIX/lib/glib-2.0/include

LIBS += -L$$MSYS2_PREFIX/lib \
        -lsigrok \
        -lglib-2.0 -lgobject-2.0 -lgio-2.0 -lintl \
        -lusb-1.0 -lserialport \
        -lzip -lz \
        -lws2_32 -lsetupapi -lwinmm

contains(DEFINES, ENABLE_SRD) {
    LIBS += -lsigrokdecode
}
