VERSION = "11.3"

DEFINES += VERSION=\\\"$$VERSION\\\"
DEFINES += MEDIEVAL

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

TRANSLATIONS    = collatinus_fr.ts
TRANSLATIONS    += collatinus_en.ts

unix:!macx:DESTDIR = bin
OBJECTS_DIR= obj/
MOC_DIR = moc/
QMAKE_DISTCLEAN += $${DESTDIR}/collatinus

# Input
HEADERS += src/ch.h \
           src/flexion.h \
           src/irregs.h \
           src/lemme.h \
           src/dicos.h \
		   src/modele.h \
#           src/flexfr.h \
           src/mainwindow.h \
		   src/maj.h \
    src/mot.h \
    src/lasla.h \
    src/tagueur.h \
    src/scandeur.h \
    src/lemCore.h \
    src/lemmatiseur.h

SOURCES += src/ch.cpp \
           src/flexion.cpp \
#		       src/frequences.cpp \
           src/irregs.cpp \
           src/lemme.cpp \
           src/dicos.cpp \
#           src/flexfr.cpp \
           src/main.cpp \
		   src/maj.cpp \
           src/mainwindow.cpp \
           src/modele.cpp \
           src/scandeur.cpp \
    src/mot.cpp \
    src/lasla.cpp \
    src/tagueur.cpp \
    src/lemCore.cpp \
    src/lemmatiseur.cpp

RESOURCES += collatinus.qrc
RC_ICONS = res/collatinus.ico

macx:{
    TARGET = Collatinus_$${VERSION}
    #note mac os x, fair un $ qmake -spec macx-g++
    #CONFIG += x86 ppc
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.8
    ICON = collatinus.icns
    #QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.4u.sdk

    #QMAKE_POST_LINK=strip Collatinus.app/Contents/MacOS/collatinus

    # install into app bundle
    # à changer en ressources
    data.path = $${TARGET}.app/Contents/MacOS/data
    data.files =  bin/data/*
#    deploy.depends = install_documentation
    deploy.depends += install
#    documentation.path = Collatinus_11.app/Contents/MacOS/doc/
#    documentation.files = doc/*.html doc/*.css
    # ajouter un cible qui fait macdeploy Collatinus.app
    deploy.commands = macdeployqt $${TARGET}.app
#    dmg.depends = deploy
#	dmg.commands = ./MacOS/Collatinus.sh
#    INSTALLS += documentation
    INSTALLS += data
    QMAKE_EXTRA_TARGETS += deploy
#    QMAKE_EXTRA_TARGETS += dmg
}
