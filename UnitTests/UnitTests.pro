QT += core network sql testlib
QT -= gui
CONFIG += c++17 console testcase
TARGET = UnitTests
TEMPLATE = app
SOURCES += tst_tests.cpp ../EchoServer/functionsforserver.cpp
HEADERS += ../EchoServer/functionsforserver.h ../EchoServer/dataBase.h
LIBS += -lssl -lcrypto
# Windows MinGW: OpenSSL из комплекта Qt (есть headers + import libs + DLL)
win32 {
    INCLUDEPATH += C:/Qt/Tools/mingw1310_64/opt/include
    LIBS += -LC:/Qt/Tools/mingw1310_64/opt/lib
}
