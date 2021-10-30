/*            mainwindow.h
 *
 *  This file is part of COLLATINUS.
 *
 *  COLLATINUS is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  COLLATINVS is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with COLLATINUS; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * © Yves Ouvrard, 2009 - 2016
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include <QtNetwork>
#include <QDebug>
#include <QPrintDialog>
#include <QPrinter>

#include "flexion.h"
#include "lemCore.h"
#include "dicos.h"
#include "ch.h"
#include "lasla.h"
#include "tagueur.h"
#include "scandeur.h"
#include "lemmatiseur.h"
#include "maj.h"

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QTextBrowser;
class QTextEdit;
QT_END_NAMESPACE

class MainWindow;

/**
 * @brief La classe EditLatin est dérivée de
 * QTextEdit afin de pouvoir redéfinir l'action
 * connectée au clic de souris sur un mot ou après
 * sélection d'une portion de texte.
 */
class EditLatin : public QTextEdit
{
    Q_OBJECT

   private:
    MainWindow *mainwindow; /*!< un pointeur vers la GUI pour lui envoyer les informations à afficher */

   protected:
    void mouseReleaseEvent(QMouseEvent *e);

   public:
    EditLatin(QWidget *parent);
    bool event(QEvent *event);
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

   public:
    MainWindow();
    // docks
    QDockWidget *dockLem;
    QDockWidget *dockDic;
    QDockWidget *dockScand;
    QDockWidget *dockFlex;
    QDockWidget *dockTag;
    // et second dictionnaire
    QWidget *wDic;
    // cœur
    LemCore *_lemCore;
    Flexion *flechisseur;
    // modules divers
    Lasla *lasla;
    Tagueur *tagueur;
    Scandeur *scandeur;
    Lemmatiseur *_lemmatiseur;
    // widgets d'édition et d'affichage
    EditLatin *editLatin;
    QTextEdit *textEditLem;
    QTextEdit *textEditScand;
    QTextBrowser *textBrowserDic;
    QTextBrowser *textBrowserW;
    QTextBrowser *textBrowserFlex;
    QTextBrowser *textBrowserTag;
    QTextEdit *editeurRes();
    QLineEdit *lineEditLem;
    QLineEdit *lineEditDic;
    QLineEdit *lineEditDicW;
    QLineEdit *lineEditFlex;
    QLineEdit *lineEditScand;
    // cases à cocher pour la copie
    QCheckBox *cbTexteLatin;
    QCheckBox *cbLemmatisation;
    QCheckBox *cbScansion;
    // contrôle des options
    QCheckBox *cbAlpha;
    QCheckBox *cbHtml;
    QCheckBox *cbMajPert;
    QCheckBox *cbMorpho;
    bool html();
    QAction *syncAct;
    QAction *calepAct;
    QAction *majPertAct;
    // L'option de majuscule pertinente doit être accessible depuis EditLatin.
    QStringList lemsDic;
    int lireOptionsAccent(bool force = false);
    // Pour le serveur
    QString startServer ();
    QString stopServer ();
//    int nbLg(QString str);

    QString lem2csv(QString texte);

    void tagger(QString t, int p); // Je voudrais créer dans MainWindow l'interface du tagger.


   private slots:
    void afficheLemsDic(bool litt = false,
                        bool prim = true);  // ligne de saisie
    void afficheLemsDicLitt();              // relais pour le précédent
    void afficheLemsDicW();                 // ligne de saisie
    void afficheLemsDicLittW();             // relais pour le précédent
    void afficheLien(QUrl url);
    void afficheLienW(QUrl url);
    void alpha();
    void apropos();
    void auxilium();
    void changeGlossarium(QString nomDic);
    void changeGlossariumW(QString nomDic);
    void changePageDjvu(int p, bool prim = true);
    void clicAnte();
    void clicAnteW();
    void clicPost();
    void clicPostW();
    void closeEvent(QCloseEvent *event);
    void copie();
    void dialogueCopie();
    void effaceRes();
    void exportPdf();
    void exportCsv();
    void flechisLigne();
    void imprimer();
    void langueInterface();
    void lancer();
    void lemmatiseLigne();
    void lemmatiseTxt();
    void majDic();
    void majLex();
    void montreWDic(bool visible);
    void nouveau();
    void ouvrir();
    void police();
    void readSettings();
    void recherche();
    void rechercheBis();
    void scandeLigne();
    void scandeTxt();
    void setCible();
    void setHtml(bool h);
    void stat();
    void syncDW();
    void syncWD();
    // Slots d'accentuation
    void setAccent(bool b);
    void lireFichierHyphen();
    void oteDiacritiques();
    // Slots du serveur
    void lancerServeur(bool run=false);
    void connexion ();
    void exec ();
    // Restauration des docks
    void dockRestore ();
    void verbaCognita(bool vb=false);
    void verbaOut();
    void txt2csv();
//    void txt2ePub();
//    void txt2vocab();

   public slots:
    void afficheLemsDic(QStringList ll, int no = 0);
    void afficheLemsDicW(QStringList ll, int no = 0);

   private:
    // initialisation
    void createActions();
    void createCibles();  // menu des langues cibles
    void createConnections();
    void createDicos(bool prim = true);
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void createDockWindows();
    void createDicWindow();  // second dictionnaire
    void setLangue();

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *viewMenu;
    QMenu *lFrEngMenu;
    QMenu *lexMenu;
    QMenu *optMenu;
    QMenu *extraMenu;
    QMenu *helpMenu;

    QToolBar *toolBar;

    // bascules de lemmatisation
    QAction *alphaAct;
    QAction *alphaOptAct;
    QAction *formeTAct;
    QAction *htmlAct;
    QAction *morphoAct;
    QAction *nonRecAct;
    // bascule d'accentuation
    QAction *accentAct;
    // et options
    QActionGroup *optionsAccent;
    QAction *longueAct;
    QAction *breveAct;
    QAction *ambigueAct;
    QAction *illiusAct;
    QAction *hyphenAct;
    QAction *lireHyphenAct;
    QAction *actionVerba_cognita;
    QAction *verba_cognita_out;
    // Nom du répertoire du fichier hyphen.la
    QString repHyphen;
    QString ficHyphen;
    QString repVerba;
    // Bascule du serveur
    QAction *serverAct;
    // Restauration des docks
    QAction *dockRestoreAct;

    // bascules du tagger
    QAction *affToutAct;

    // bascule pour une liste de vocabulaire en double page.
    // true : texte à gauche, vocabulaire à droite.
    // false : texte en haut, vocabulaire en dessous.
    QAction *dbPageAct;
    // Option pour les textes en prose (double colonne 11cm + 0,69cm)
    QAction *proseAct;

    // Pour le serveur
    QTcpServer * serveur;
    QTcpSocket * soquette;

    // actions et groupes d'actions
    QAction *aproposAct;
    QAction *auxAct;
    QAction *balaiAct;
    QAction *copieAct;
    QAction *deZoomAct;
    QAction *dicAct;
    QAction *dicActW;
    QAction *dicLittAct;
    QAction *dicLittActW;
    QAction *enAct;
    QAction *exportAct;
    QAction *exportCsvAct;
    QAction *fontAct;
    QAction *findAct;
    QAction *frAct;
    QActionGroup *grCibles;
    QAction *lancAct;
    QAction *majDicAct;
    QAction *majLexAct;
    QAction *nouvAct;
    QAction *oteAAct;
    QAction *ouvrirAct;
    QAction *printAct;
    QAction *quitAct;
    QAction *reFindAct;
    QAction *statAct;
    QAction *syncDWAct;
    QAction *syncWDAct;
    QAction *visibleWAct;
    QAction *extensionWAct;
    QAction *medievalAct;
    QAction *zoomAct;
    QAction *actionTxt2csv;
//    QAction *actionTxt2ePub;
//    QAction *actionTxt2vocab;
    // QAction *undoAct;
    // widgets, variables et fonctions du dock dictionnaires
    QComboBox *comboGlossaria;
    QPushButton *anteButton;
    QLabel *labelLewis;
    QPushButton *postButton;
    ListeDic listeD;
    QStringList ldic;
    // les mêmes, pour le widget dictionnaires
    QComboBox *comboGlossariaW;
    QPushButton *anteButtonW;
    QLabel *labelLewisW;
    QPushButton *postButtonW;
    // widgets des docks
    QWidget *dockWidgetLem;
    QWidget *dockWidgetDic;
    QWidget *dockWidgetScand;
    QWidget *dockWidgetFlex;
    QWidget *dockWidgetTag;
    bool dockVisible(QDockWidget *d);
    // fonctions et variables diverses
    void charger(QString f);
    QFont font;
    QString nfAb;       // nom du fichier à charger
    QString nfAd;       // nom du fichier de sortie
    bool precaution();  // autorise ou non la fermeture du fichier chargé
    QString rech;       // dernière chaîne recherchée
    QString repertoire;
    // traductions
    QString langueI;
    QTranslator *translator;
    bool alerte();
};

#endif
