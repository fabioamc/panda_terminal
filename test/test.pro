
include(../includes.pri)

QT += testlib
TARGET = WPanda-test
SOURCES += \
      main.cpp \
    testelements.cpp \
    testsimulationcontroller.cpp \
    testfiles.cpp \
    testcommands.cpp \
    testwaveform.cpp \
    testicons.cpp \
    testlogicelements.cpp \
    testarduino.cpp

HEADERS += \
    testelements.h \
    testsimulationcontroller.h \
    testfiles.h \
    testcommands.h \
    testwaveform.h \
    testicons.h \
    testlogicelements.h \
    testarduino.h

DEFINES += CURRENTDIR=\\\"$$PWD\\\"
