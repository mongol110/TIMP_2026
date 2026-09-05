QT -= gui
QT += core network sql
CONFIG += c++17 console
CONFIG -= app_bundle
TARGET = EchoServer
TEMPLATE = app
SOURCES += main.cpp mytcpserver.cpp functionsforserver.cpp
HEADERS += mytcpserver.h functionsforserver.h dataBase.h
# Криптография вручную, без OpenSSL.
