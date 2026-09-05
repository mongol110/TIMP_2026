QT -= gui
QT += core network sql
CONFIG += c++17 console
CONFIG -= app_bundle
TARGET = EchoServer
TEMPLATE = app
SOURCES += main.cpp mytcpserver.cpp functionsforserver.cpp
HEADERS += mytcpserver.h functionsforserver.h dataBase.h
LIBS += -lssl -lcrypto
# Windows MinGW: OpenSSL из комплекта Qt (есть headers + import libs + DLL)
win32 {
    INCLUDEPATH += C:/Qt/Tools/mingw1310_64/opt/include
    LIBS += -LC:/Qt/Tools/mingw1310_64/opt/lib
}
