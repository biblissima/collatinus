QT += network widgets
QT += core
QT -= gui

TARGET = Client_C11
VERSION = "1.0"
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += src/client_main.cpp
OBJECTS_DIR= obj/
MOC_DIR = moc/
unix:!macx:DESTDIR = bin

macx:{
# Commandes spéciales pour déployer l'application sur Mac.
# J'ignore s'il faut l'équivalent pour Linux ou Windows.
# Philippe. Octobre 2016

QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.8
#ICON = collatinus_bw.icns

deploy.commands = macdeployqt Client_C11.app
QMAKE_EXTRA_TARGETS += deploy

}
