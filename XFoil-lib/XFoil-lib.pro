#-------------------------------------------------
#
# Project created by QtCreator 2017-01-18T04:06:28
#
#-------------------------------------------------

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QT       -= gui

TARGET = XFoil
TEMPLATE = lib

DEFINES += XFOILLIB_LIBRARY

SOURCES += \
    XFoil.cpp

HEADERS +=\
    xfoil-lib_global.h \
    XFoil.h \
    xfoil_params.h

OBJECTS_DIR = ./build/objects
MOC_DIR = ./build/moc
RCC_DIR = ./build/rcc



macx{
#	CONFIG += lib_bundle
#	CONFIG += i386
	QMAKE_SONAME_PREFIX = @executable_path/../Frameworks
	QMAKE_MAC_SDK = macosx
}


unix{
	isEmpty(PREFIX){
		PREFIX = /usr/local
	}
	target.path = $$PREFIX/lib
	INSTALLS += target
}

