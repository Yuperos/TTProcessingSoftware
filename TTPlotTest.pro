QT += core gui network printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app

DIR_LEVEL = ../

CONFIG += c++17

CONFIG(release, debug|release): DESTDIR = $${DIR_LEVEL}bin/release
CONFIG(debug, debug|release): DESTDIR = $${DIR_LEVEL}bin/debug

HEADERS += *.h
SOURCES += *.cpp
FORMS   += *.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
