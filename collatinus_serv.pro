VERSION = "11.0"
DEFINES += VERSION=\\\"$$VERSION\\\"

TEMPLATE = app
TARGET = collatinus
INCLUDEPATH += . src
DEPENDPATH += .

qtHaveModule(printsupport): QT += printsupport
QT += widgets
QT += network
QT += svg

#QMAKE_CXXFLAGS += -Wall -Wextra -pedantic -fstack-protector-strong
#QMAKE_CPPFLAGS += -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2

CONFIG += release_binary debug

TRANSLATIONS    = collatinus_en.ts \
                  collatinus_fr.ts

unix:!macx:DESTDIR = bin
OBJECTS_DIR= obj/
MOC_DIR = moc/
QMAKE_DISTCLEAN += $${DESTDIR}/collatinus

# Input
HEADERS += src/ch.h \
           src/flexion.h \
           src/irregs.h \
           src/lemmatiseur.h \
           src/lemme.h \
           src/dicos.h \
		       src/modele.h \
           src/flexfr.h \
           src/mainwindow.h \
		       src/maj.h \
           src/syntaxe.h

SOURCES += src/ch.cpp \
           src/flexion.cpp \
		       src/frequences.cpp \
           src/irregs.cpp \
           src/lemmatiseur.cpp \
           src/lemme.cpp \
           src/dicos.cpp \
           src/flexfr.cpp \
           src/main.cpp \
		       src/maj.cpp \
           src/mainwindow.cpp \
           src/modele.cpp \
           src/scandeur.cpp \
           src/syntaxe.cpp

RESOURCES += collatinus.qrc

macx:{
    TARGET = Collatinus_11s
    #note mac os x, fair un $ qmake -spec macx-g++
    #CONFIG += x86 ppc
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.8
    ICON = collatinus.icns
    #QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.4u.sdk

    #QMAKE_POST_LINK=strip Collatinus.app/Contents/MacOS/collatinus

    # install into app bundle
    # à changer en ressources
    data.path = Collatinus_11s.app/Contents/MacOS/data
    data.files =  bin/data/*
#    deploy.depends = install_documentation
    deploy.depends += install
#    documentation.path = Collatinus_11s.app/Contents/MacOS/doc-usr/
#    documentation.files = doc-usr/*.md doc-usr/*.css doc-usr/*.png
    # ajouter un cible qui fait macdeploy Collatinus.app
    deploy.commands = macdeployqt Collatinus_11s.app
#    dmg.depends = deploy
#	dmg.commands = ./MacOS/Collatinus.sh
#    INSTALLS += documentation
    INSTALLS += data
    QMAKE_EXTRA_TARGETS += deploy
#    QMAKE_EXTRA_TARGETS += dmg
}
