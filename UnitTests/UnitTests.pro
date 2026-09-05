QT += core network sql testlib
QT -= gui
CONFIG += c++17 console testcase
TARGET = UnitTests
TEMPLATE = app
SOURCES += tst_tests.cpp ../EchoServer/functionsforserver.cpp
HEADERS += ../EchoServer/functionsforserver.h ../EchoServer/dataBase.h
# Без OpenSSL: AES и SHA1 реализованы вручную.
