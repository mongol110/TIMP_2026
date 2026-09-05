QT += core gui widgets network
CONFIG += c++17
TARGET = Client
TEMPLATE = app
SOURCES += main.cpp mainwindow.cpp connectiondialog.cpp
HEADERS += mainwindow.h connectiondialog.h tcpclient.h
