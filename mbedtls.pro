TEMPLATE = lib
TARGET = n9mbedtls
CONFIG += staticlib release
CONFIG -= qt
QMAKE_CFLAGS += -std=c99 -D_POSIX_C_SOURCE=200112L
INCLUDEPATH += third_party/mbedtls-3.6.7/include third_party/mbedtls-3.6.7/library
SOURCES += $$files(third_party/mbedtls-3.6.7/library/*.c)
