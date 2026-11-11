QT += core gui sql charts network printsupport serialport multimedia
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets charts

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Camion.cpp \
    arduinohandler.cpp \
    client.cpp \
    connection.cpp \
    conteneur.cpp \
    employee.cpp \
    machine.cpp \
    main.cpp \
    point_de_collecte.cpp \
    wasterecycling.cpp

HEADERS += \
    Camion.h \
    arduinohandler.h \
    client.h \
    connection.h \
    conteneur.h \
    employee.h \
    machine.h \
    point_de_collecte.h \
    wasterecycling.h

FORMS += \
    wasterecycling.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    sidebarRessources.qrc
QT+= sql
CONFIG += console
