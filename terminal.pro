TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c \
    terminal.c \
    cmd_help.c

HEADERS += \
    terminal.h \
    cmd_help.h
