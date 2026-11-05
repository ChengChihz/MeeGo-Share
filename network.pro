TEMPLATE = app
TARGET = probe
CONFIG += console release
CONFIG -= app_bundle
QT += network
QT -= gui
INCLUDEPATH += third_party/mbedtls-3.6.7/include third_party/qjson
SOURCES += main.cpp third_party/qjson/parser.cpp third_party/qjson/json_scanner.cpp third_party/qjson/json_parser.cc third_party/qjson/serializer.cpp
LIBS += -L../mbedtls -ln9mbedtls -lpthread -lrt


