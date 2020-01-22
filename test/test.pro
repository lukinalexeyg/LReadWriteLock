#-------------------------------------------------
#
# Project created by QtCreator 2019-03-15T10:48:43
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LMutex test
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

QMAKE_LFLAGS  += /INCREMENTAL:NO /LTCG /DEBUG /OPT:REF /OPT:ICF
QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.01
QMAKE_CFLAGS  += -O2 -GL
QMAKE_CFLAGS  += /Zi
QMAKE_CXXFLAGS  += -O2 -GL
QMAKE_CXXFLAGS  += /Zi

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += APP_BUNDLE=\\\"Schoolfeed\\\"
DEFINES += APP_PRODUCT=\\\"$$QMAKE_TARGET_PRODUCT\\\"
DEFINES += APP_VERSION=\\\"$$VERSION\\\"
DEFINES += APP_COMPANY=\\\"$$QMAKE_TARGET_COMPANY\\\"
DEFINES += PSAPI_VERSION=1

LIBS += -lsetupAPI
LIBS += -lwinspool
LIBS += -lpsapi
LIBS += DbgHelp.lib

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
    log.cpp \
    main.cpp \
    mainwindow.cpp \
    storage.cpp \
    worker.cpp

HEADERS += \
    log.h \
    mainwindow.h \
    storage.h \
    worker.h

FORMS +=

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

include(../LReadWriteLock/LReadWriteLock.pri)
