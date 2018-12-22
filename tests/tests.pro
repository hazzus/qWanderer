CONFIG += testcase c++17
QT = core gui testlib
QT += widgets

TEMPLATE = app

SOURCES += tests.cpp

INCLUDEPATH += ../indexer

LIBS += \
        -L../indexer
        -lindexer

DEFINES += TESTS
