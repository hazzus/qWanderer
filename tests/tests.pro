CONFIG += testcase c++17
QT = core gui testlib
QT += widgets

TEMPLATE = app

SOURCES += tests.cpp

INCLUDEPATH += ../sDsid/
DEFINES += TESTS
