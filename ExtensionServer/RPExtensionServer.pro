QT += core network texttospeech
CONFIG += c++17 console
CONFIG -= app_bundle

TARGET = RPExtensionServer
TEMPLATE = app

SOURCES += main.cpp \
    rpextensionserver.cpp

HEADERS += \
    rpextensionserver.h
