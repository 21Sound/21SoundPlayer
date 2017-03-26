#-------------------------------------------------
#
# Project created by QtCreator 2016-09-17T11:27:16
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = 21SoundPlayer
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
        CRtAudio.cpp \
        qcustomplot.cpp \
        CMpgRead.cpp \
        CWavRW.cpp \
        fft.cpp

HEADERS  += mainwindow.h \
        CRtAudio.h \
        qcustomplot.h \
        CMpgRead.h \
        CWavRW.h \
        complex_float32.h \
        complex_float64.h \
        fft.h

win32: LIBS += -L$$PWD/dependencies32/lib/rtaudio/ -llibrtaudio_shared.dll

INCLUDEPATH += $$PWD/dependencies32/headers/rtaudio
DEPENDPATH += $$PWD/dependencies32/headers/rtaudio

win32: LIBS += -L$$PWD/dependencies32/lib/libmp3/ -lmpg123

INCLUDEPATH += $$PWD/dependencies32/headers/libmp3
DEPENDPATH += $$PWD/dependencies32/headers/libmp3

win32-g++: PRE_TARGETDEPS += $$PWD/dependencies32/lib/libmp3/libmpg123.a
