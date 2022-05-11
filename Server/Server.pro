QT += core network
QT -= gui

TEMPLATE = app

DIR_LEVEL = ../

CONFIG += c++17 console

CONFIG(release, debug|release): DESTDIR = $${DIR_LEVEL}bin/release
CONFIG(debug, debug|release): DESTDIR = $${DIR_LEVEL}bin/debug

INCLUDEPATH += $$PWD/../Libs/

LIBS += -L$${DESTDIR} -lLibs

HEADERS += *.h
SOURCES += *.cpp

contains(QT_ARCH, i386){
win32:CONFIG(release, debug|release): LIBS += -L'C:/Program Files/Swabian Instruments/TimeTagger2_7_6/driver/x86/' -lTimeTagger
else:win32:CONFIG(debug, debug|release): LIBS += -L'C:/Program Files/Swabian Instruments/TimeTagger2_7_6/driver/x86/' -lTimeTaggerd
else:unix: LIBS += -L'C:/Program Files/Swabian Instruments/TimeTagger2_7_6/driver/x86/' -lTimeTagger
}else{
win32:CONFIG(release, debug|release): LIBS += -L'C:/Program Files/Swabian Instruments/TimeTagger2_7_6/driver/x64/' -lTimeTagger
else:win32:CONFIG(debug, debug|release): LIBS += -L'C:/Program Files/Swabian Instruments/TimeTagger2_7_6/driver/x64/' -lTimeTaggerd
else:unix: LIBS += -L'C:/Program Files/Swabian Instruments/TimeTagger2_7_6/driver/x64/' -lTimeTagger
}

INCLUDEPATH += 'C:/Program Files/Swabian Instruments/TimeTagger2_7_6/driver/include'
DEPENDPATH += 'C:/Program Files/Swabian Instruments/TimeTagger2_7_6/driver/include'

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
