/*   mainwindow.cpp
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

#include <QDebug>
#include <QPrintDialog>
#include <QPrinter>

#include "flexion.h"
#include "mainwindow.h"
#include "maj.h"

/**
 * \fn EditLatin::EditLatin (QWidget *parent): QTextEdit (parent)
 * \brief Créateur de la classe EditLatin, dérivée de
 * QTextEdit afin de pouvoir redéfinir l'action
 * connectée au clic de souris sur un mot ou après
 * sélection d'une portion de texte.
 */
EditLatin::EditLatin(QWidget *parent) : QTextEdit(parent)
{
    mainwindow = qobject_cast<MainWindow *>(parent);
}

/**
 * \fn bool EditLatin::event(QEvent *event)
 * \brief Captation du survol de la souris pour
 *        afficher dans une bulle lemmatisation et
 *        analyses morphologiques.
 */
bool EditLatin::event(QEvent *event)
{
    switch (event->type())
    {
        case QEvent::ToolTip:
        {
            QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
            QPoint P = mapFromGlobal(helpEvent->globalPos());
            QTextCursor tc = cursorForPosition(P);
            tc.select(QTextCursor::WordUnderCursor);
            QString mot = tc.selectedText();
            QString txtBulle = mainwindow->lemmatiseur->lemmatiseT(
                mot, true, true, true, false);
            txtBulle.prepend("<p style='white-space:pre'>");
            txtBulle.append("</p>");
            QToolTip::setFont(font());
            QToolTip::showText(helpEvent->globalPos(), txtBulle.trimmed(),
                               this);
            return true;
        }
        default:
            return QTextEdit::event(event);
    }
}

/**
 * \fn void EditLatin::mouseReleaseEvent (QMouseEvent *e)
 * \brief Captation de la fin du clic de souris : ajout
 *        des lemmatisations et analyses morpho dans
 *        le dock correspondant.
 */
void EditLatin::mouseReleaseEvent(QMouseEvent *e)
{
    QTextCursor cursor = textCursor();
    if (!cursor.hasSelection()) cursor.select(QTextCursor::WordUnderCursor);
    QString st = cursor.selectedText();
    bool unSeulMot = !st.contains(' ');
    MapLem ml = mainwindow->lemmatiseur->lemmatiseM(st);
    // 1. dock de lemmatisation
    if (!mainwindow->dockLem->visibleRegion().isEmpty())
    {
        if (unSeulMot && mainwindow->calepAct->isChecked())
            foreach (Lemme *l, ml.keys())
                mainwindow->textEditLem->append(l->ambrogio());
        else
        {
            if (mainwindow->html())
            {
                QString texteHtml = mainwindow->textEditLem->toHtml();
                texteHtml.insert(texteHtml.indexOf("</body>"),mainwindow->lemmatiseur->lemmatiseT(st));
                mainwindow->textEditLem->setText(texteHtml);
                mainwindow->textEditLem->moveCursor(QTextCursor::End);
            }
            else
                mainwindow->textEditLem->insertPlainText(
                    mainwindow->lemmatiseur->lemmatiseT(st));
        }
    }
    // 2. dock scansion
    if (!mainwindow->dockScand->visibleRegion().isEmpty())
    {
        int accent = mainwindow->lireOptionsAccent();
        mainwindow->textEditScand->append(
            mainwindow->lemmatiseur->scandeTxt(st, accent, false));
    }
    if (unSeulMot)
    {
        // 3. dock de flexion
        if (!mainwindow->dockFlex->visibleRegion().isEmpty())
        {
            if (!ml.empty())
            {
                mainwindow->textBrowserFlex->clear();
                mainwindow->textBrowserFlex->append(
                    mainwindow->flechisseur->tableaux(&ml));
                mainwindow->textBrowserFlex->moveCursor(QTextCursor::Start);
            }
        }
        // 4. dock dictionnaires
        QStringList lemmes = mainwindow->lemmatiseur->lemmes(ml);
        if (!mainwindow->dockDic->visibleRegion().isEmpty())
            mainwindow->afficheLemsDic(lemmes);
        if (mainwindow->wDic->isVisible() && mainwindow->syncAct->isChecked())
            mainwindow->afficheLemsDicW(lemmes);
        // 5. dock Syntaxe
        if (!mainwindow->dockSynt->visibleRegion().isEmpty())
        {
            // passer le texte au module syntaxe pour calcul
            mainwindow->textBrowserSynt->setText(mainwindow->syntaxe->analyse(
                toPlainText(), textCursor().position()));
            // appondre le résultat
            // = liens du mot cliqué.
            mainwindow->textBrowserSynt->moveCursor(QTextCursor::Start);
        }
    }
    QTextEdit::mouseReleaseEvent(e);
}

/**
 * \fn MainWindow::MainWindow()
 * \brief Créateur de la fenêtre de l'appli.
 *        Les différentes tâches sont regrouppées
 *        et confiées à des fonctions spécialisées.
 */
MainWindow::MainWindow()
{
    QFile styleFile(":/res/collatinus.css");
    styleFile.open(QFile::ReadOnly);
    QString style(styleFile.readAll());
    qApp->setStyleSheet(style);

    editLatin = new EditLatin(this);
    setCentralWidget(editLatin);

    lemmatiseur = new Lemmat(this);
    flechisseur = new Flexion(lemmatiseur);
    syntaxe = new Syntaxe(editLatin->toPlainText(), lemmatiseur);

    setLangue();

    createStatusBar();
    createActions();
    createDockWindows();
    createDicWindow();
    createMenus();
    createToolBars();
    createConnections();
    createDicos();
    createDicos(false);
    createCibles();

    setWindowTitle(tr("Collatinus 11"));
    setWindowIcon(QIcon(":/res/collatinus.svg"));

    setUnifiedTitleAndToolBarOnMac(true);

    // setTabPosition(Qt::BottomDockWidgetArea, QTabWidget::North );

    readSettings();
}

/**
 * \fn void MainWindow::afficheLemsDic (bool litt, bool prim)
 * \brief Surcharge. Récupère le contenu de la ligne de saisie du
 *        dock des dictionnaires si prim est à true,
 *        sinon la ligne de saisie de la fenêtre
 *        supplémentaire. Ce contenu est lemmatisé si litt
 *        est à false, puis la ou les pages/entrées
 *        correspondantes sont affichées, soit dans le
 *        dock, soit dans la fenêtre supplémentaire.
 */
void MainWindow::afficheLemsDic(bool litt, bool prim)
{
    QLineEdit *lineEdit;
    if (prim)
        lineEdit = lineEditDic;
    else
        lineEdit = lineEditDicW;
    if (lineEdit->text().isEmpty()) return;
    lemsDic.clear();
    QStringList requete;
    if (!litt)
    {
        MapLem lm = lemmatiseur->lemmatiseM(lineEdit->text(), true);
        requete = lemmatiseur->lemmes(lm);
    }
    if (requete.empty()) requete << lineEdit->text();
    requete.removeDuplicates();
    if (prim)
        afficheLemsDic(requete, 0);
    else
        afficheLemsDicW(requete, 0);
    lineEdit->selectAll();
    lineEdit->setFocus();
}

/**
 * \fn void MainWindow::afficheLemsDicLitt()
 * \brief Fonction de relais permettant d'utiliser
 *        la connexion entre une action et la fonction
 *        afficheLemsDic().
 */
void MainWindow::afficheLemsDicLitt() { afficheLemsDic(true); }
/**
 * \fn void MainWindow::afficheLemsDicW () * \brief Fonction de relais
 * permettant d'utiliser
 *        la connexion entre une action et la fonction
 *        afficheLemsDicW().
 *
 */
void MainWindow::afficheLemsDicW() { afficheLemsDic(false, false); }
/**
 * \fn afficheLemsDic(true,false);
 * \brief
 * \brief Fonction de relais permettant d'utiliser
 *        la connexion entre une action et la fonction
 *        afficheLemsDicW(), sans lemmatisation.
 */
void MainWindow::afficheLemsDicLittW() { afficheLemsDic(true, false); }
/**
 * \fn void MainWindow::afficheLemsDic(QStringList ll, int no)
 * \brief Affiche la page ou les entrées de
 * dictionnaire correspondant au lemme d'ordre no de la
 * liste ll, et règle le texte des boutons de
 * navigation.
 */
void MainWindow::afficheLemsDic(QStringList ll, int no)
{
    if (textBrowserDic == 0) return;
    lemsDic = ll;
    if (ll.empty() || no < 0 || listeD.courant() == NULL) return;
    textBrowserDic->clear();
    textBrowserDic->setHtml(listeD.courant()->page(ll, no));
    lineEditDic->setText(ll.at(no));
    if (listeD.courant()->estXml())
    {
        anteButton->setText(listeD.courant()->pgPrec());
        postButton->setText(listeD.courant()->pgSuiv());
    }
    else
    {
        anteButton->setText(tr("Retro"));
        postButton->setText(tr("Porro"));
        labelLewis->setText(QString::number(listeD.courant()->noPageDjvu()));
    }
    textBrowserDic->moveCursor(QTextCursor::Start);
}

/**
 * \fn void MainWindow::afficheLemsDicW(QStringList ll, int no)
 * \brief comme afficheLemsDic, mais pour le
 * dictionnaire supplémentaire.
 *
 */
void MainWindow::afficheLemsDicW(QStringList ll, int no)
{
    if (textBrowserW == 0) return;
    // lemsDic = ll;
    if (ll.empty() || no < 0 || listeD.courant2() == NULL) return;
    textBrowserW->clear();
    textBrowserW->setHtml(listeD.courant2()->page(ll, no));
    lineEditDicW->setText(ll.at(no));
    if (listeD.courant2()->estXml())
    {
        anteButtonW->setText(listeD.courant2()->pgPrec());
        postButtonW->setText(listeD.courant2()->pgSuiv());
    }
    else
    {
        anteButtonW->setText(tr("Retro"));
        postButtonW->setText(tr("Porro"));
        labelLewisW->setText(QString::number(listeD.courant2()->noPageDjvu()));
    }
    textBrowserW->moveCursor(QTextCursor::Start);
}

/**
 * \fn void MainWindow::afficheLien (QUrl url)
 * \brief Prend en charge l'affichage des hyperliens de
 *        navigations insérés dans les pages/entrées
 *        des dictionnaires.
 *
 */
void MainWindow::afficheLien(QUrl url)
{
    if (listeD.courant()->estXml()) return;
    // la ligne de liens en tête de page doit être gardée
    QStringList liens = listeD.courant()->liens();
    int no = liens.indexOf(url.toString());
    if (no < 0) no = 0;
    afficheLemsDic(liens, no);
}

/**
 * \fn void MainWindow::afficheLienW (QUrl url)
 * \brief Comme afficheLien, pour le dictionnaire
 * supplémentaire.
 */
void MainWindow::afficheLienW(QUrl url)
{
    if (listeD.courant2()->estXml()) return;
    // la ligne de liens en tête de page doit être gardée
    QStringList liens = listeD.courant2()->liens();
    int no = liens.indexOf(url.toString());
    if (no < 0) no = 0;
    afficheLemsDicW(liens, no);
}

/**
 * \fn void MainWindow::alpha()
 * \brief Force la lemmatisation alphabétique de
 *        tout le texte, quelle que soit l'option alpha
 *        du lemmatiseur.
 */
void MainWindow::alpha()
{
    // pour que l'action provoque le basculement à true
    // de l'option alpha du lemmatiseur, supprimer la
    // première et la dernière ligne.
    bool tmpAlpha = lemmatiseur->optAlpha();
    lemmatiseur->setAlpha(true);
    lancer();
    //	lemmatiseTxt();
    lemmatiseur->setAlpha(tmpAlpha);
}

/**
 * \fn void MainWindow::apropos ()
 * \brief Affiche les informations essentielles au
 *        sujet de Collatinus 11.
 */
void MainWindow::apropos()
{
    QMessageBox::about(
        this, tr("Collatinus 11"),
        tr("COLLATINVS\nLinguae latinae lemmatizatio \n"
           "Licentia GPL, © Yves Ouvrard, 2009 - 2016 \n"
           "Nonnullas partes operis scripsit Philippe Verkerk\n"
           "Versio " VERSION "\n"
           "Gratias illis habeo :\n"
           "William Whitaker †\n"
           "Jose Luis Redrejo,\n"
           "Georges Khaznadar,\n"
           "Matthias Bussonier,\n"
           "Gérard Jeanneau,\n"
           "Jean-Paul Woitrain,\n"
           "Perseus Digital Library <http://www.perseus.tufts.edu>"));
}

/**
 * \fn void MainWindow::changeGlossarium (QString nomDic)
 * \brief Change le dictionnaire actif du dock
 * dictionnaires.
 *
 */
void MainWindow::changeGlossarium(QString nomDic)
{
    listeD.change_courant(nomDic);
    if (listeD.courant() == NULL) return;
    if (listeD.courant()->estXml())
        labelLewis->setText("↔");  // "\u2194"
    else
    {
        listeD.courant()->vide_index();
        labelLewis->clear();
    }
    if (!lemsDic.empty())
        afficheLemsDic(lemsDic, lemsDic.indexOf(lineEditDic->text()));
    else if (!lineEditDic->text().isEmpty())
        afficheLemsDic(QStringList() << lineEditDic->text());
}

/**
 * \fn void MainWindow::changeGlossariumW (QString nomDic)
 * \brief Comme ChangeGlossarium, pour le dictionnaire
 *        supplémentaire.
 */
void MainWindow::changeGlossariumW(QString nomDic)
{
    listeD.change_courant2(nomDic);
    if (listeD.courant2() == NULL) return;
    if (listeD.courant2()->estXml())
        labelLewisW->setText("↔");  // "\u2194"
    else
    {
        listeD.courant2()->vide_index();
        labelLewisW->clear();
    }
    if (!lemsDic.empty())
        afficheLemsDicW(lemsDic, lemsDic.indexOf(lineEditDicW->text()));
    else if (!lineEditDicW->text().isEmpty())
        afficheLemsDicW(QStringList() << lineEditDicW->text());
}

/**
 * \fn void MainWindow::changePageDjvu (int p, bool prim)
 * \brief Change la page d'un dictionnaire au format
 *        djvu, pour le dock dictionnaire si prim est à
 *        true, sinon pour le dictionnaire
 *        supplémentaire.
 */
void MainWindow::changePageDjvu(int p, bool prim)
{
    QTextBrowser *browser;
    QLabel *label;
    if (prim)
    {
        browser = textBrowserDic;
        label = labelLewis;
    }
    else
    {
        browser = textBrowserW;
        label = labelLewisW;
    }
    browser->clear();
    if (prim)
        browser->setHtml(listeD.courant()->pageDjvu(p));
    else
        browser->setHtml(listeD.courant2()->pageDjvu(p));
    label->setText(QString::number(p));
    browser->moveCursor(QTextCursor::Start);
}

/**
 * \fn void MainWindow::charger (QString f)
 * \brief Charge le fichier nommé f dans l'éditeur
 *        de texte latin.
 */
void MainWindow::charger(QString f)
{
    QFile file(f);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::warning(this, tr("Collatinus"),
                             tr("%1: Lecture impossible,\n%2.")
                                 .arg(nfAb)
                                 .arg(file.errorString()));
        return;
    }
    QTextStream in(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QString contenu = in.readAll();
    file.close();
    editLatin->setPlainText(contenu);
    QApplication::restoreOverrideCursor();
}

/**
 * \fn void MainWindow::clicAnte ()
 * \brief Gère le passage à la page précédente.
 */
void MainWindow::clicAnte()
{
    if (!listeD.courant()) return;
    listeD.courant()->vide_ligneLiens();
    if (listeD.courant()->estXml())
    {
        afficheLemsDic(QStringList() << anteButton->text());
    }
    else
    {
        int p = labelLewis->text().toInt();
        if (p > 0) changePageDjvu(labelLewis->text().toInt() - 1);
    }
}

/**
 * \fn void MainWindow::clicAnteW()
 * \brief Comme clicAnte, pour le dictionnaire
 * supplémentaire.
 *
 */
void MainWindow::clicAnteW()
{
    if (!listeD.courant2()) return;
    listeD.courant2()->vide_ligneLiens();
    if (listeD.courant2()->estXml())
    {
        afficheLemsDicW(QStringList() << anteButton->text());
    }
    else
    {
        int p = labelLewisW->text().toInt();
        if (p > 0) changePageDjvu(labelLewisW->text().toInt() - 1, false);
    }
}

/**
 * \fn void MainWindow::clicPost ()
 * \brief Gère le passage du dictionnaire à la page
 *        suivante.
 */
void MainWindow::clicPost()
{
    if (!listeD.courant()) return;
    listeD.courant()->vide_ligneLiens();
    if (listeD.courant()->estXml())
    {
        afficheLemsDic(QStringList() << postButton->text());
    }
    else
    {
        int p = labelLewis->text().toInt();
        if (p < 8888)  // ATTENTION, déclarer la dernière page dans les cfg !
            changePageDjvu(labelLewis->text().toInt() + 1);
    }
}

/**
 * \fn void MainWindow::clicPostW()
 * \brief Comme clicPost, pour le dictionnaire
 *        supplémentaire.
 *
 */
void MainWindow::clicPostW()
{
    if (!listeD.courant2()) return;
    listeD.courant2()->vide_ligneLiens();
    if (listeD.courant2()->estXml())
    {
        afficheLemsDicW(QStringList() << postButtonW->text());
    }
    else
    {
        int p = labelLewisW->text().toInt();
        if (p < 8888)  // ATTENTION, déclarer la dernière page dans les cfg !
            changePageDjvu(labelLewisW->text().toInt() + 1, false);
    }
}

/**
 * \fn void MainWindow::closeEvent(QCloseEvent *event)
 * \brief Enregistre certains paramètres le la session
 *        avant fermeture de l'application.
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings("Collatinus", "collatinus11");
    settings.beginGroup("interface");
    settings.setValue("langue", langueI);
    settings.endGroup();
    settings.beginGroup("fenetre");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.endGroup();
    settings.beginGroup("fichiers");
    if (!nfAb.isEmpty()) settings.setValue("nfAb", nfAb);
    settings.endGroup();
    settings.beginGroup("options");
    // settings.setValue("police", font.family());
    settings.setValue("zoom", editLatin->font().pointSize());
    // options
    settings.setValue("alpha", alphaOptAct->isChecked());
    settings.setValue("html", htmlAct->isChecked());
    settings.setValue("formetxt", formeTAct->isChecked());
    settings.setValue("extensionlexique", extensionWAct->isChecked());
    settings.setValue("majpert", majPertAct->isChecked());
    settings.setValue("morpho", morphoAct->isChecked());
    settings.setValue("nonrec", nonRecAct->isChecked());
    settings.setValue("cible", lemmatiseur->cible());
    // accentuation
    settings.setValue("accentuation", accentAct->isChecked());
    settings.setValue("longue", longueAct->isChecked());
    settings.setValue("breve", breveAct->isChecked());
    settings.setValue("ambigue", ambigueAct->isChecked());
    settings.setValue("illius", illiusAct->isChecked());
    settings.setValue("hyphenation", hyphenAct->isChecked());
    settings.setValue("repHyphen", repHyphen);
    settings.endGroup();
    settings.beginGroup("dictionnaires");
    settings.setValue("courant", comboGlossaria->currentIndex());
    settings.setValue("wdic", wDic->isVisible());
    settings.setValue("courantW", comboGlossariaW->currentIndex());
    settings.setValue("posw", wDic->pos());
    settings.setValue("sizew", wDic->size());
    settings.setValue("sync", syncAct->isChecked());
    settings.endGroup();
    delete wDic;
    QMainWindow::closeEvent(event);
}

void MainWindow::copie()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->clear();

    QString texte;
    // if (cbTexteLatin->isChecked()) texte.append(editLatin->toPlainText());
    if (cbTexteLatin->isChecked()) texte.append(editLatin->toHtml());
    if (cbLemmatisation->isChecked()) texte.append(textEditLem->toHtml());
    if (cbScansion->isChecked()) texte.append(textEditScand->toHtml());
    QMimeData *mime = new QMimeData;
    mime->setHtml(texte);
    clipboard->setMimeData(mime);
}

/**
 * \fn void MainWindow::createActions()
 * \brief Fonction appelée par le créateur. Initialise
 *        toutes les actions utilisées par
 *        l'application.
 */
void MainWindow::createActions()
{
    /*
    undoAct = new QAction(QIcon(":/images/undo.png"), tr("&Undo"), this);
    undoAct->setShortcuts(QKeySequence::Undo);
    undoAct->setStatusTip(tr("Undo the last editing action"));
    connect(undoAct, SIGNAL(triggered()), this, SLOT(undo()));
    // aussi SLOT(redo())
    */
    alphaAct = new QAction(QIcon(":res/edit-alpha.svg"),
                           tr("Lancer et classer &alphabétiquement"), this);
    aproposAct =
        new QAction(QIcon(":/res/collatinus.svg"), tr("à &Propos"), this);
    balaiAct = new QAction(QIcon(":res/edit-clear.svg"),
                           tr("&Effacer les résultats"), this);
    copieAct = new QAction(QIcon(":res/copie.svg"),
                           tr("&Copier dans un traitement de textes"), this);
    deZoomAct = new QAction(QIcon(":res/dezoom.svg"), tr("Plus petit"), this);
    findAct = new QAction(QIcon(":res/edit-find.svg"), tr("&Chercher"), this);
    fontAct = new QAction(tr("Police de caractères"), this);
    lancAct = new QAction(QIcon(":res/gear.svg"), tr("&Lancer"), this);
    majAct = new QAction(tr("Télécharger lexiques et dictionnaires"), this);
    nouvAct =
        new QAction(QIcon(":/res/document-new.svg"), tr("&Nouveau"), this);
    ouvrirAct =
        new QAction(QIcon(":/res/document-open.svg"), tr("&Ouvrir"), this);
    exportAct = new QAction(QIcon(":res/pdf.svg"), tr("Exporter en pdf"), this);
    printAct = new QAction(QIcon(":res/print.svg"), tr("Im&primer"), this);
    quitAct = new QAction(QIcon(":/res/power.svg"), tr("&Quitter"), this);
    quitAct->setStatusTip(tr("Quitter l'application"));
    oteAAct = new QAction(tr("Ôter les accents"), this);
    reFindAct = new QAction(tr("Chercher &encore"), this);
    statAct = new QAction(QIcon(":res/abacus.svg"), tr("S&tatistiques"), this);
    zoomAct = new QAction(QIcon(":res/zoom.svg"), tr("Plus gros"), this);

    // langues d'interface
    enAct = new QAction(tr("English Interface"), this);
    enAct->setCheckable(true);
    frAct = new QAction(tr("Interface en français"), this);
    frAct->setCheckable(true);

    // raccourcis
    findAct->setShortcut(QKeySequence::Find);
    nouvAct->setShortcuts(QKeySequence::New);
    ouvrirAct->setShortcuts(QKeySequence::Open);
    printAct->setShortcuts(QKeySequence::Print);
    reFindAct->setShortcut(QKeySequence(tr("Ctrl+J")));
    quitAct->setShortcut(
        QKeySequence(tr("Ctrl+Q")));  // QKeySequence::Quit inopérant

    // lemmatisation et options
    // ordre alpha
    alphaOptAct = new QAction(tr("ordre alpha"), this);
    alphaOptAct->setCheckable(true);
    // calepino
    calepAct = new QAction(tr("Calepino"), this);
    calepAct->setCheckable(true);
    // formes du texte dans la lemmatisation
    formeTAct = new QAction(tr("avec formes"), this);
    formeTAct->setCheckable(true);
    // lemmatisation en html
    htmlAct = new QAction(tr("format html"), this);
    htmlAct->setCheckable(true);
    // prise en compte des majuscules
    majPertAct = new QAction(tr("majuscules"), this);
    majPertAct->setCheckable(true);
    // analyses morpho dans la lemmatisation
    morphoAct = new QAction(tr("Morpho"), this);
    morphoAct->setCheckable(true);
    // non reconnus en fin de lemmatisation
    nonRecAct = new QAction(tr("grouper échecs"), this);
    nonRecAct->setCheckable(true);

    // actions pour les accents
    accentAct = new QAction(tr("accentuer"), this);
    accentAct->setCheckable(true);
    accentAct->setChecked(false);
    optionsAccent = new QActionGroup(this);
    longueAct = new QAction(tr("   -̆ => ¯ "), this);
    longueAct->setCheckable(true);
    breveAct = new QAction(tr("   -̆ => ˘ "), this);
    breveAct->setCheckable(true);
    ambigueAct = new QAction(tr("   -̆ => pas d'accent"), this);
    ambigueAct->setCheckable(true);
    ambigueAct->setChecked(true);
    illiusAct = new QAction(tr("except illíus"), this);
    illiusAct->setCheckable(true);
    illiusAct->setChecked(true);
    hyphenAct = new QAction(tr("marquer les syllabes"), this);
    hyphenAct->setCheckable(true);
    hyphenAct->setEnabled(false);
    optionsAccent->addAction(longueAct);
    optionsAccent->addAction(breveAct);
    optionsAccent->addAction(ambigueAct);
    optionsAccent->setEnabled(false);
    lireHyphenAct = new QAction(tr("Lire les césures"),this);

    // actions pour le serveur
    serverAct = new QAction(tr("Serveur"), this);
    serverAct->setCheckable(true);
    serverAct->setChecked(false);

    // Restauration des docks
    dockRestoreAct = new QAction(tr("Restaurer les docks"),this);

    // actions pour les dictionnaires
    dicAct = new QAction(QIcon(":/res/dicolem.svg"),
                         tr("Lemmatiser et chercher"), this);
    dicLittAct = new QAction(QIcon(":/res/dicolitt.svg"), tr("Chercher"), this);
    dicActW = new QAction(QIcon(":/res/dicolem.svg"),
                          tr("Lemmatiser et chercher"), this);
    dicLittActW =
        new QAction(QIcon(":/res/dicolitt.svg"), tr("Chercher"), this);
    // synchronisation des deux dictionnaires
    syncAct = new QAction(tr("sync+"), this);
    syncAct->setCheckable(true);  // synchronisation des deux fenêtres
    syncDWAct = new QAction(tr("sync->"), this);
    syncWDAct = new QAction(tr("<-sync"), this);
    visibleWAct = new QAction(tr("Dictionnaire +"), this);
    visibleWAct->setCheckable(true);
    extensionWAct = new QAction(tr("Extension du lexique"), this);
    extensionWAct->setCheckable(true);
}

/**
 * \fn void MainWindow::createCibles()
 * \brief Initialise toutes les actions liées aux
 *        fonctions de traduction.
 */
void MainWindow::createCibles()
{
    grCibles = new QActionGroup(lexMenu);
    foreach (QString cle, lemmatiseur->cibles().keys())
    {
        QAction *action = new QAction(grCibles);
        action->setText(lemmatiseur->cibles()[cle]);
        action->setCheckable(true);
        lexMenu->addAction(action);
        connect(action, SIGNAL(triggered()), this, SLOT(setCible()));
    }
}

/**
 * \fn void MainWindow::createConnections()
 * \brief Initialisation des connections qui lancent
 *        toutes les actions des menus et des barres d'outils.
 */
void MainWindow::createConnections()
{
    // synchroniser zoom et dezoom
    connect(zoomAct, SIGNAL(triggered()), editLatin, SLOT(zoomIn()));
    connect(zoomAct, SIGNAL(triggered()), textBrowserDic, SLOT(zoomIn()));
    connect(zoomAct, SIGNAL(triggered()), textBrowserW, SLOT(zoomIn()));
    connect(zoomAct, SIGNAL(triggered()), textBrowserFlex, SLOT(zoomIn()));
    connect(zoomAct, SIGNAL(triggered()), textBrowserSynt, SLOT(zoomIn()));
    connect(zoomAct, SIGNAL(triggered()), textEditLem, SLOT(zoomIn()));
    connect(zoomAct, SIGNAL(triggered()), textEditScand, SLOT(zoomIn()));

    connect(deZoomAct, SIGNAL(triggered()), editLatin, SLOT(zoomOut()));
    connect(deZoomAct, SIGNAL(triggered()), textBrowserDic, SLOT(zoomOut()));
    connect(deZoomAct, SIGNAL(triggered()), textBrowserW, SLOT(zoomOut()));
    connect(deZoomAct, SIGNAL(triggered()), textBrowserFlex, SLOT(zoomOut()));
    connect(deZoomAct, SIGNAL(triggered()), textBrowserSynt, SLOT(zoomOut()));
    connect(deZoomAct, SIGNAL(triggered()), textEditLem, SLOT(zoomOut()));
    connect(deZoomAct, SIGNAL(triggered()), textEditScand, SLOT(zoomOut()));

    // connexions des lignes de saisie
    connect(lineEditLem, SIGNAL(returnPressed()), this, SLOT(lemmatiseLigne()));
    connect(lineEditFlex, SIGNAL(returnPressed()), this, SLOT(flechisLigne()));
    connect(lineEditScand, SIGNAL(returnPressed()), this, SLOT(scandeLigne()));

    // options et actions du lemmatiseur
    connect(alphaOptAct, SIGNAL(toggled(bool)), lemmatiseur,
            SLOT(setAlpha(bool)));
    connect(formeTAct, SIGNAL(toggled(bool)), lemmatiseur,
            SLOT(setFormeT(bool)));
    connect(htmlAct, SIGNAL(toggled(bool)), lemmatiseur, SLOT(setHtml(bool)));
    connect(majPertAct, SIGNAL(toggled(bool)), lemmatiseur,
            SLOT(setMajPert(bool)));
    connect(morphoAct, SIGNAL(toggled(bool)), lemmatiseur,
            SLOT(setMorpho(bool)));
    connect(nonRecAct, SIGNAL(toggled(bool)), lemmatiseur,
            SLOT(setNonRec(bool)));
    connect(extensionWAct, SIGNAL(toggled(bool)), lemmatiseur, SLOT(setExtension(bool)));

    // actions et options de l'accentuation
    connect(accentAct, SIGNAL(toggled(bool)), this, SLOT(setAccent(bool)));
    connect(lireHyphenAct, SIGNAL(triggered()), this, SLOT(lireFichierHyphen()));
    connect(oteAAct, SIGNAL(triggered()), this, SLOT(oteDiacritiques()));

    // lancer ou arrêter le serveur
    connect(serverAct, SIGNAL(toggled(bool)), this, SLOT(lancerServeur(bool)));
    // restaurer les docks
    connect(dockRestoreAct, SIGNAL(triggered()), this, SLOT(dockRestore()));

    // actions des dictionnaires
    connect(anteButton, SIGNAL(clicked()), this, SLOT(clicAnte()));
    connect(comboGlossaria, SIGNAL(currentIndexChanged(QString)), this,
            SLOT(changeGlossarium(QString)));
    connect(dicAct, SIGNAL(triggered()), this, SLOT(afficheLemsDic()));
    connect(dicLittAct, SIGNAL(triggered()), this, SLOT(afficheLemsDicLitt()));
    connect(lineEditDic, SIGNAL(returnPressed()), this, SLOT(afficheLemsDic()));
    connect(postButton, SIGNAL(clicked()), this, SLOT(clicPost()));
    connect(syncDWAct, SIGNAL(triggered()), this, SLOT(syncDW()));
    connect(textBrowserDic, SIGNAL(anchorClicked(QUrl)), this,
            SLOT(afficheLien(QUrl)));

    connect(anteButtonW, SIGNAL(clicked()), this, SLOT(clicAnteW()));
    connect(comboGlossariaW, SIGNAL(currentIndexChanged(QString)), this,
            SLOT(changeGlossariumW(QString)));
    connect(dicActW, SIGNAL(triggered()), this, SLOT(afficheLemsDicW()));
    connect(dicLittActW, SIGNAL(triggered()), this,
            SLOT(afficheLemsDicLittW()));
    connect(lineEditDicW, SIGNAL(returnPressed()), this,
            SLOT(afficheLemsDicW()));
    connect(majAct, SIGNAL(triggered()), this, SLOT(maj()));
    connect(postButtonW, SIGNAL(clicked()), this, SLOT(clicPostW()));
    connect(syncWDAct, SIGNAL(triggered()), this, SLOT(syncWD()));
    connect(textBrowserW, SIGNAL(anchorClicked(QUrl)), this,
            SLOT(afficheLienW(QUrl)));
    connect(visibleWAct, SIGNAL(toggled(bool)), this, SLOT(montreWDic(bool)));

    // langue d'interface
    connect(frAct, SIGNAL(triggered()), this, SLOT(langueInterface()));
    connect(enAct, SIGNAL(triggered()), this, SLOT(langueInterface()));

    // autres actions
    connect(alphaAct, SIGNAL(triggered()), this, SLOT(alpha()));
    connect(aproposAct, SIGNAL(triggered()), this, SLOT(apropos()));
    connect(balaiAct, SIGNAL(triggered()), this, SLOT(effaceRes()));
    connect(copieAct, SIGNAL(triggered()), this, SLOT(dialogueCopie()));
    connect(exportAct, SIGNAL(triggered()), this, SLOT(exportPdf()));
    connect(findAct, SIGNAL(triggered()), this, SLOT(recherche()));
    connect(fontAct, SIGNAL(triggered()), this, SLOT(police()));
    connect(lancAct, SIGNAL(triggered()), this, SLOT(lancer()));
    connect(nouvAct, SIGNAL(triggered()), this, SLOT(nouveau()));
    connect(ouvrirAct, SIGNAL(triggered()), this, SLOT(ouvrir()));
    connect(printAct, SIGNAL(triggered()), this, SLOT(imprimer()));
    connect(quitAct, SIGNAL(triggered()), this, SLOT(close()));
    connect(reFindAct, SIGNAL(triggered()), this, SLOT(rechercheBis()));
    connect(statAct, SIGNAL(triggered()), this, SLOT(stat()));
}

/**
 * \fn void MainWindow::createDicos(bool prim)
 * \brief Chargement des index et des fichiers de
 *        configuration des dictionnaires.
 */
void MainWindow::createDicos(bool prim)
{
    QComboBox *combo = 0;
    if (prim)
        combo = comboGlossaria;
    else
        combo = comboGlossariaW;
    combo->clear();
    QDir chDicos(qApp->applicationDirPath() + "/data/dicos");
    QStringList lcfg = chDicos.entryList(QStringList() << "*.cfg");
    ldic.clear();
    foreach (QString fcfg, lcfg)
    {
        Dictionnaire *d = new Dictionnaire(fcfg);
        listeD.ajoute(d);
        ldic << d->nom();
    }
    combo->insertItems(0, ldic);
}

/**
 * \fn void MainWindow::createMenus()
 * \brief Initialisation des menus à partir des actions définies
 *        dans MainWindow::createActions().
 *
 */
void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&Fichier"));
    fileMenu->addAction(nouvAct);
    fileMenu->addAction(ouvrirAct);
    fileMenu->addSeparator();
    fileMenu->addAction(copieAct);
    fileMenu->addAction(exportAct);
    fileMenu->addAction(printAct);
    fileMenu->addSeparator();
    fileMenu->addAction(oteAAct);
    fileMenu->addAction(lireHyphenAct);
    fileMenu->addSeparator();
    fileMenu->addAction(quitAct);

    editMenu = menuBar()->addMenu(tr("&Edition"));
    editMenu->addAction(findAct);
    editMenu->addAction(reFindAct);
    // editMenu->addAction(undoAct);

    viewMenu = menuBar()->addMenu(tr("&Vue"));
    viewMenu->addAction(balaiAct);
    viewMenu->addAction(zoomAct);
    viewMenu->addAction(deZoomAct);
    viewMenu->addAction(fontAct);
    viewMenu->addSeparator();
    viewMenu->addAction(visibleWAct);
    viewMenu->addAction(dockRestoreAct);
    viewMenu->addSeparator();
    QActionGroup *frEngAg = new QActionGroup(this);
    frAct->setActionGroup(frEngAg);
    enAct->setActionGroup(frEngAg);
    viewMenu->addAction(frAct);
    viewMenu->addAction(enAct);
    if (langueI == "fr")
        frAct->setChecked(true);
    else if (langueI == "en")
        enAct->setChecked(true);

    lexMenu = menuBar()->addMenu(tr("&Lexique"));
    lexMenu->addAction(lancAct);
    lexMenu->addAction(alphaAct);
    lexMenu->addAction(statAct);
    lexMenu->addSeparator();
    lexMenu->addAction(extensionWAct);
    lexMenu->addSeparator();

    optMenu = menuBar()->addMenu(tr("&Options"));
    optMenu->addAction(alphaOptAct);
    optMenu->addAction(formeTAct);
    optMenu->addAction(htmlAct);
    optMenu->addAction(majPertAct);
    optMenu->addAction(morphoAct);
    optMenu->addAction(nonRecAct);
    optMenu->addSeparator();
    optMenu->addAction(accentAct);
    //    optMenu->addAction(optionsAccent)
    optMenu->addAction(longueAct);
    optMenu->addAction(breveAct);
    optMenu->addAction(ambigueAct);
    optMenu->addAction(illiusAct);
    optMenu->addAction(hyphenAct);
//    optMenu->addSeparator();
//    optMenu->addAction(fontAct);
//    optMenu->addAction(majAct);

    extraMenu = menuBar()->addMenu(tr("Extra"));
    extraMenu->addAction(serverAct);
    extraMenu->addAction(majAct);

    helpMenu = menuBar()->addMenu(tr("&Aide"));
    helpMenu->addAction(aproposAct);
}

/**
 * \fn void MainWindow::createToolBars()
 * \brief Initialisation de la barre d'outils à partir
 *        des actions.
 */
void MainWindow::createToolBars()
{
    toolBar = new QToolBar(this);
    toolBar->setObjectName("toolbar");
    addToolBar(Qt::TopToolBarArea, toolBar);

    toolBar->addAction(nouvAct);
    toolBar->addAction(ouvrirAct);
    toolBar->addAction(copieAct);
    toolBar->addAction(zoomAct);
    toolBar->addAction(deZoomAct);
    toolBar->addAction(findAct);
    toolBar->addSeparator();
    toolBar->addAction(lancAct);
    toolBar->addAction(alphaAct);
    toolBar->addAction(statAct);
    toolBar->addAction(calepAct);
    toolBar->addAction(visibleWAct);
    toolBar->addAction(balaiAct);
    toolBar->addSeparator();
    toolBar->addAction(quitAct);
}

/**
 * \fn void MainWindow::createStatusBar()
 * \brief Initialisation de la barre d'état. À compléter.
 *
 */
void MainWindow::createStatusBar() {}
/**
 * \fn void MainWindow::createDockWindows()
 * \brief Initialisation des différents docks.
 *
 */
void MainWindow::createDockWindows()
{
    dockLem = new QDockWidget(tr("Lexique et morphologie"), this);
    dockLem->setObjectName("docklem");
    dockLem->setAllowedAreas(Qt::BottomDockWidgetArea |
                             Qt::RightDockWidgetArea);
    dockLem->setFloating(false);
    dockLem->setFeatures(QDockWidget::DockWidgetFloatable |
                         QDockWidget::DockWidgetMovable);
    dockWidgetLem = new QWidget(dockLem);
    dockWidgetLem->setSizePolicy(QSizePolicy::Expanding,
                                 QSizePolicy::Expanding);
    QVBoxLayout *vLayoutLem = new QVBoxLayout(dockWidgetLem);
    QHBoxLayout *hLayoutLem = new QHBoxLayout();
    lineEditLem = new QLineEdit(dockWidgetLem);
    // boutons d'options
    QToolButton *tbCalep = new QToolButton(this);
    tbCalep->setDefaultAction(calepAct);
    QToolButton *tbMorpho = new QToolButton(this);
    tbMorpho->setDefaultAction(morphoAct);
    QToolButton *tbAlpha = new QToolButton(this);
    tbAlpha->setDefaultAction(alphaOptAct);
    QToolButton *tbFormeT = new QToolButton(this);
    tbFormeT->setDefaultAction(formeTAct);
    QToolButton *tbHtml = new QToolButton(this);
    tbHtml->setDefaultAction(htmlAct);
    QToolButton *tbMajPert = new QToolButton(this);
    tbMajPert->setDefaultAction(majPertAct);
    QToolButton *tbNonRec = new QToolButton(this);
    tbNonRec->setDefaultAction(nonRecAct);
    QSpacerItem *hSpacerLem = new QSpacerItem(40, 20);
    hLayoutLem->addWidget(lineEditLem);
    hLayoutLem->addWidget(tbCalep);
    hLayoutLem->addWidget(tbMorpho);
    hLayoutLem->addWidget(tbAlpha);
    hLayoutLem->addWidget(tbFormeT);
    hLayoutLem->addWidget(tbHtml);
    hLayoutLem->addWidget(tbMajPert);
    hLayoutLem->addWidget(tbNonRec);
    hLayoutLem->addItem(hSpacerLem);
    textEditLem = new QTextEdit(dockWidgetLem);
    vLayoutLem->addLayout(hLayoutLem);
    vLayoutLem->addWidget(textEditLem);
    dockLem->setWidget(dockWidgetLem);
//    qDebug() << dockLem->testAttribute(Qt::WA_DeleteOnClose) << dockWidgetLem->testAttribute(Qt::WA_DeleteOnClose);

    dockDic = new QDockWidget(tr("Dictionnaires"), this);
    dockDic->setObjectName("dockdic");
    dockDic->setFloating(false);
    dockDic->setFeatures(QDockWidget::DockWidgetFloatable |
                         QDockWidget::DockWidgetMovable);
    dockDic->setAllowedAreas(Qt::BottomDockWidgetArea);
    dockWidgetDic = new QWidget(dockDic);
    QVBoxLayout *vLayoutDic = new QVBoxLayout(dockWidgetDic);
    QHBoxLayout *hLayoutDic = new QHBoxLayout();
    lineEditDic = new QLineEdit(dockWidgetDic);
    lineEditDic->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Fixed);
    lineEditDic->setMinimumWidth(40);
    // Lemmatisation + recherche
    QToolButton *tbDic = new QToolButton(this);
    tbDic->setDefaultAction(dicAct);
    // recherche sans lemmatisation
    QToolButton *tbDicLitt = new QToolButton(this);
    tbDicLitt->setDefaultAction(dicLittAct);
    // dictionnaire
    QToolButton *tbSync = new QToolButton(this);
    tbSync->setDefaultAction(syncAct);
    tbSync->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Fixed);
    tbSync->setMinimumWidth(40);
    QToolButton *tbDicW = new QToolButton(this);
    tbDicW->setDefaultAction(visibleWAct);
    tbDicW->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Fixed);
    tbDicW->setMinimumWidth(40);
    tbDicW->setMaximumSize(90);
    QToolButton *tbSyncDW = new QToolButton(this);
    tbSyncDW->setDefaultAction(syncDWAct);
    tbSyncDW->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Fixed);
    tbSyncDW->setMinimumWidth(40);
    // choix des dictionnaires
    comboGlossaria = new QComboBox(this);
    anteButton = new QPushButton(this);
    labelLewis = new QLabel(this);
    postButton = new QPushButton(this);
//    QSpacerItem *hSpacerDic = new QSpacerItem(40, 20);
    //, QSizePolicy::Expanding, QSizePolicy::Minimum);
    hLayoutDic->addWidget(lineEditDic);
    hLayoutDic->addWidget(tbDic);
    hLayoutDic->addWidget(tbDicLitt);
    hLayoutDic->addWidget(comboGlossaria);
    hLayoutDic->addWidget(anteButton);
    hLayoutDic->addWidget(labelLewis);
    hLayoutDic->addWidget(postButton);
//    hLayoutDic->addItem(hSpacerDic);
    hLayoutDic->addStretch();
    hLayoutDic->addWidget(tbSync);
    hLayoutDic->addWidget(tbDicW);
    hLayoutDic->addWidget(tbSyncDW);
    textBrowserDic = new QTextBrowser(dockWidgetDic);
    textBrowserDic->setOpenExternalLinks(true);
    vLayoutDic->addLayout(hLayoutDic);
    vLayoutDic->addWidget(textBrowserDic);
    dockDic->setWidget(dockWidgetDic);

    dockScand = new QDockWidget(tr("Scansion"), this);
    dockScand->setObjectName("dockscand");
    dockScand->setFloating(false);
    dockScand->setFeatures(QDockWidget::DockWidgetFloatable |
                           QDockWidget::DockWidgetMovable);
    dockScand->setAllowedAreas(Qt::BottomDockWidgetArea |
                               Qt::RightDockWidgetArea);
    dockWidgetScand = new QWidget(dockScand);
    QVBoxLayout *vLayoutScand = new QVBoxLayout(dockWidgetScand);
    QHBoxLayout *hLayoutScand = new QHBoxLayout();
    lineEditScand = new QLineEdit(dockWidgetScand);
    //QSpacerItem *hSpacerScand =
    //   new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    hLayoutScand->addWidget(lineEditScand);
    // ajouter ici des boutons...
    QToolButton *tbAccent = new QToolButton(this);
    tbAccent->setDefaultAction(accentAct);
    QToolButton *tbLongue = new QToolButton(this);
    tbLongue->setDefaultAction(longueAct);
    QToolButton *tbBreve = new QToolButton(this);
    tbBreve->setDefaultAction(breveAct);
    QToolButton *tbAmbigue = new QToolButton(this);
    tbAmbigue->setDefaultAction(ambigueAct);
    QToolButton *tbIllius = new QToolButton(this);
    tbIllius->setDefaultAction(illiusAct);
    QToolButton *tbHyphen = new QToolButton(this);
    tbHyphen->setDefaultAction(hyphenAct);
    hLayoutScand->addWidget(tbAccent);
    hLayoutScand->addWidget(tbLongue);
    hLayoutScand->addWidget(tbBreve);
    hLayoutScand->addWidget(tbAmbigue);
    hLayoutScand->addWidget(tbIllius);
    hLayoutScand->addWidget(tbHyphen);
    //    hLayoutScand->addItem (hSpacerScand);
    textEditScand = new QTextEdit(dockWidgetScand);
    vLayoutScand->addLayout(hLayoutScand);
    vLayoutScand->addWidget(textEditScand);
    dockScand->setWidget(dockWidgetScand);

    dockFlex = new QDockWidget(tr("Flexion"), this);
    dockFlex->setObjectName("dockflex");
    dockFlex->setFloating(false);
    dockFlex->setFeatures(QDockWidget::DockWidgetFloatable |
                          QDockWidget::DockWidgetMovable);
    dockFlex->setAllowedAreas(Qt::BottomDockWidgetArea);
    dockWidgetFlex = new QWidget(dockFlex);
    QVBoxLayout *vLayoutFlex = new QVBoxLayout(dockWidgetFlex);
    QHBoxLayout *hLayoutFlex = new QHBoxLayout();
    lineEditFlex = new QLineEdit(dockWidgetFlex);
    QSpacerItem *hSpacerFlex =
        new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    hLayoutFlex->addWidget(lineEditFlex);
    hLayoutFlex->addItem(hSpacerFlex);
    textBrowserFlex = new QTextBrowser(dockWidgetFlex);
    textBrowserFlex->setSizePolicy(QSizePolicy::Expanding,
                                   QSizePolicy::Expanding);
    vLayoutFlex->addLayout(hLayoutFlex);
    vLayoutFlex->addWidget(textBrowserFlex);
    dockFlex->setWidget(dockWidgetFlex);

    dockSynt = new QDockWidget(tr("Syntaxe"), this);
    dockSynt->setObjectName("docksynt");
    dockSynt->setFloating(false);
    dockSynt->setFeatures(QDockWidget::DockWidgetFloatable |
                          QDockWidget::DockWidgetMovable);
    dockSynt->setAllowedAreas(Qt::BottomDockWidgetArea);
    dockWidgetSynt = new QWidget(dockSynt);
    QVBoxLayout *vLayoutSynt = new QVBoxLayout(dockWidgetSynt);
    QHBoxLayout *hLayoutSynt = new QHBoxLayout();
    textBrowserSynt = new QTextBrowser(dockWidgetSynt);
    textBrowserSynt->setSizePolicy(QSizePolicy::Expanding,
                                   QSizePolicy::Expanding);
    vLayoutSynt->addLayout(hLayoutSynt);
    vLayoutSynt->addWidget(textBrowserSynt);
    dockSynt->setWidget(dockWidgetSynt);

    addDockWidget(Qt::BottomDockWidgetArea, dockLem);
    addDockWidget(Qt::BottomDockWidgetArea, dockDic);
    addDockWidget(Qt::BottomDockWidgetArea, dockScand);
    addDockWidget(Qt::BottomDockWidgetArea, dockFlex);
    addDockWidget(Qt::BottomDockWidgetArea, dockSynt);

    tabifyDockWidget(dockLem, dockDic);
    tabifyDockWidget(dockDic, dockScand);
    tabifyDockWidget(dockScand, dockFlex);
    tabifyDockWidget(dockFlex, dockSynt);

    setTabPosition(Qt::BottomDockWidgetArea, QTabWidget::North);
    dockLem->raise();
}

/**
 * \fn void MainWindow::createDicWindow()
 * \brief Initialisation du widget de dictionnaire
 *        supplémentaire.
 */
void MainWindow::createDicWindow()
{
    wDic = new QWidget();
    wDic->setObjectName("wDic");
    QVBoxLayout *vLayout = new QVBoxLayout(wDic);
    QHBoxLayout *hLayout = new QHBoxLayout();
    lineEditDicW = new QLineEdit(wDic);
    // Lemmatisation + recherche
    QToolButton *tbDic = new QToolButton(this);
    tbDic->setDefaultAction(dicActW);
    // recherche sans lemmatisation
    QToolButton *tbDicLittW = new QToolButton(this);
    tbDicLittW->setDefaultAction(dicLittActW);
    comboGlossariaW = new QComboBox(this);
    anteButtonW = new QPushButton(this);
    labelLewisW = new QLabel(this);
    postButtonW = new QPushButton(this);
    QSpacerItem *hSpacerDic = new QSpacerItem(40, 20);
    //, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QToolButton *tbSyncWD = new QToolButton(this);
    tbSyncWD->setDefaultAction(syncWDAct);
    hLayout->addWidget(lineEditDicW);
    hLayout->addWidget(tbDic);
    hLayout->addWidget(tbDicLittW);
    hLayout->addWidget(comboGlossariaW);
    hLayout->addWidget(anteButtonW);
    hLayout->addWidget(labelLewisW);
    hLayout->addWidget(postButtonW);
    hLayout->addItem(hSpacerDic);
    hLayout->addWidget(tbSyncWD);
    textBrowserW = new QTextBrowser(wDic);
    textBrowserW->setOpenExternalLinks(true);
    vLayout->addLayout(hLayout);
    vLayout->addWidget(textBrowserW);
}

/**
 * \fn void MainWindow::dialogueCopie()
 * \brief Ouvre une boite de dialogue qui permet de
 *        sélectionner les parties à copier, et
 *        les place dans le presse-papier du système
 */
void MainWindow::dialogueCopie()
{
    QLabel *icon = new QLabel;
    icon->setPixmap(QPixmap(":/res/collatinus.ico"));
    QLabel *text = new QLabel;
    text->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    text->setWordWrap(true);
    text->setText(
        "<p>Pour récupérer et modifier votre travail, la meilleure manière est "
        "d'ouvrir le traitement de textes de votre choix, puis de sélectionner "
        "ci-dessous ce que vous voulez utiliser. Cliquez ensuite sur le bouton "
        "«Appliquer». Pour terminer, revenez dans votre traitement de texte, "
        "et copiez votre sélection avec un raccourci clavier, ou l'option de "
        "menu <b>Édition/Coller</b>.");

    cbTexteLatin = new QCheckBox(tr("Texte latin"));
    cbLemmatisation = new QCheckBox(tr("Lemmatisation"));
    cbScansion = new QCheckBox(tr("Scansion"));

    QPushButton *appliButton = new QPushButton(tr("Appliquer"));
    QPushButton *cloreButton = new QPushButton(tr("Fermer"));

    QVBoxLayout *topLayout = new QVBoxLayout;
    topLayout->addWidget(icon);
    topLayout->addWidget(text);

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addStretch();
    bottomLayout->addWidget(appliButton);
    bottomLayout->addWidget(cloreButton);
    bottomLayout->addStretch();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);
    topLayout->addWidget(cbTexteLatin);
    topLayout->addWidget(cbLemmatisation);
    topLayout->addWidget(cbScansion);
    mainLayout->addLayout(bottomLayout);

    QDialog dCopie(this);
    dCopie.setModal(true);
    dCopie.setWindowTitle(tr("Récupérer son travail"));
    dCopie.setLayout(mainLayout);

    connect(appliButton, SIGNAL(clicked()), this, SLOT(copie()));
    connect(cloreButton, SIGNAL(clicked()), &dCopie, SLOT(close()));
    dCopie.exec();
}

/**
 * \fn bool MainWindow::dockVisible (QDockWidget *d)
 * \brief renvoie true si le dock d est visible.
 *
 */
bool MainWindow::dockVisible(QDockWidget *d)
{
    return !d->visibleRegion().isEmpty();
}

/**
 * \fn void MainWindow::effaceRes()
 * \brief Efface le contenu des docs visibles.
 */
void MainWindow::effaceRes()
{
    if (dockVisible(dockLem)) textEditLem->clear();
    if (dockVisible(dockFlex)) textBrowserFlex->clear();
    if (dockVisible(dockScand)) textEditScand->clear();
    if (dockVisible(dockSynt)) textBrowserSynt->clear();
}

/**
 * \fn
 * \brief
 *
 */
void MainWindow::exportPdf()
{
#ifndef QT_NO_PRINTER
    QString nf =
        QFileDialog::getSaveFileName(this, "Export PDF", QString(), "*.pdf");
    if (!nf.isEmpty())
    {
        if (QFileInfo(nf).suffix().isEmpty()) nf.append(".pdf");
        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(nf);
        QTextEdit *tmpTE = new QTextEdit();
        tmpTE->setHtml(editLatin->toHtml());
        tmpTE->append(textEditLem->toHtml());
        tmpTE->document()->print(&printer);
        delete tmpTE;
    }
#endif
}

/**
 * \fn void MainWindow::imprimer()
 * \brief Lance le dialogue d'impression pour la lemmatisation.
 */
void MainWindow::imprimer()
{
#if !defined(QT_NO_PRINTER) && !defined(QT_NO_PRINTDIALOG)
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog *dlg = new QPrintDialog(&printer, this);
    if (textEditLem->textCursor().hasSelection())
        dlg->addEnabledOption(QAbstractPrintDialog::PrintSelection);
    dlg->setWindowTitle(tr("Imprimer le texte et le lexique"));
    if (dlg->exec() == QDialog::Accepted)
    {
        QTextEdit *tmpTE = new QTextEdit();
        tmpTE->setHtml(editLatin->toHtml());
        tmpTE->append(textEditLem->toHtml());
        tmpTE->print(&printer);
        delete tmpTE;
    }
    delete dlg;
#endif
}

/**
 * \fn void MainWindow::langueInterface()
 * \brief Sonde les actions frAct et enAct, et
 *        bascule l'interface dans la langue de l'action cochée.
 */
void MainWindow::langueInterface()
{
    if (frAct->isChecked())
    {
        langueI = "fr";
    }
    else if (enAct->isChecked())
    {
        langueI = "en";
    }
    else
        langueI = "fr";
    QMessageBox::about(this, tr("Collatinus 11"),
                       tr("Le changement de langue prendra effet"
                          "au prochain lancement de Collatinus."));
}

/**
 * \fn void MainWindow::flechisLigne()
 * \brief Provoque l'affichage des lemmes pouvant donner
 *        la forme affichée dans la ligne de saisie du dock
 *        Flexion.
 */
void MainWindow::flechisLigne()
{
    MapLem ml = lemmatiseur->lemmatiseM(lineEditFlex->text());
    if (!ml.empty())
    {
        textBrowserFlex->clear();
        textBrowserFlex->append(flechisseur->tableaux(&ml));
        // foreach (Lemme *l, ml.keys())
        //	textBrowserFlex->append (flechisseur->tableau(l));
    }
}

/**
 * \fn bool MainWindow::html()
 * \brief Renvoie vrai si l'option html du lemmatiseur
 *        est armée.
 */
bool MainWindow::html() { return htmlAct->isChecked(); }
/**
 * \fn void MainWindow::lancer()
 * \brief Lance la lemmatisation et la scansion si
 *        les docks correspondants sont visibles.
 */
void MainWindow::lancer()
{
    if (dockVisible(dockLem)) lemmatiseTxt();
    if (dockVisible(dockScand)) scandeTxt();
}

/**
 * \fn void MainWindow::lemmatiseLigne()
 * \brief Lance la lemmatisation des formes *
 *        présentes dans la ligne de saisie du dock
 *        lemmatisation.
 */
void MainWindow::lemmatiseLigne()
{
    QString texteHtml = textEditLem->toHtml();
    texteHtml.insert(texteHtml.indexOf("</body>"),
                     lemmatiseur->lemmatiseT(lineEditLem->text()));
    textEditLem->setText(texteHtml);
    textEditLem->moveCursor(QTextCursor::End);
//    textEditLem->append(lemmatiseur->lemmatiseT(lineEditLem->text()));
}

/**
 * \fn void MainWindow::lemmatiseTxt()
 * \brief Lance la lemmatisation de la totalité du
 *        texte contenu dans l'éditeur editLatin (partie supérieure
 *        de la fenêtre de l'application).
 */
void MainWindow::lemmatiseTxt()
{
    // si la tâche dure trop longtemps :
    // setUpdatesEnabled(false);
    if (html())
        textEditLem->setHtml(lemmatiseur->lemmatiseT(editLatin->toPlainText()));
    else
        textEditLem->setPlainText(
            lemmatiseur->lemmatiseT(editLatin->toPlainText()));
    // setUpdatesEnabled(true);
}

/**
 * \fn void MainWindow::maj()
 * \brief Lance le dialogue de mise à jour des
 *        lexiques et dictionnaires.
 */

void MainWindow::maj()
{
    Maj *majDial = new Maj();
    majDial->setFont(editLatin->font());
    majDial->exec();
}

/**
 * \fn void MainWindow::montreWDic(bool visible)
 * \brief Rend visible le dictionnaire supplémentaire,
 *        et met à jour son contenu.
 */
void MainWindow::montreWDic(bool visible)
{
    wDic->move(x() + width() + 80, y());
    wDic->setVisible(visible);
    lineEditDicW->setText(lineEditDic->text());
    afficheLemsDicW();
    lineEditDicW->clearFocus();
}

/**
 * \fn void MainWindow::nouveau()
 * \brief Après confirmation efface le texte et les résultats,
 *        permettant à l'utilisateur de recommencer /ab initio/
 */
void MainWindow::nouveau()
{
    if (precaution()) return;
    editLatin->clear();
    textEditLem->clear();
    textEditScand->clear();
}

/**
 * \fn void MainWindow::ouvrir()
 * \brief Affiche le dialogue d'ouverture de fichier.
 */
void MainWindow::ouvrir()
{
    if (precaution()) return;
    nfAb = QFileDialog::getOpenFileName(this, "Collatinus - Ouvrir un fichier",
                                        repertoire);
    if (nfAb.isEmpty()) return;
    charger(nfAb);
    nfAd = nfAb;
    nfAd.prepend("coll-");
}

void MainWindow::police()
{
    bool ok;
    QFont police = QFontDialog::getFont(&ok, font, this);
    if (ok)
    {
        font = police;
        editLatin->setFont(font);
        textEditLem->setFont(font);
        textBrowserDic->setFont(font);
        textBrowserW->setFont(font);
        textEditScand->setFont(font);
        textBrowserFlex->setFont(font);
        textBrowserSynt->setFont(font);
    }
}

/**
 * \fn bool MainWindow::precaution()
 * \brief Dialogue de précaution avant l'effacement du texte latin.
 *        Renvoie false si Oui/Yes a été cliqué.
 */
bool MainWindow::precaution()
{
    if (!editLatin->document()->isEmpty() ||
        !textEditLem->document()->isEmpty() ||
        !textEditScand->document()->isEmpty())
    {
        int ret = QMessageBox::warning(
            this, tr("Collatinus"), tr("Un ou plusieurs onglets ont été "
                                       "modifiés. Effacer leur contenu ?"),
            QMessageBox::Yes | QMessageBox::Default, QMessageBox::No,
            QMessageBox::Cancel | QMessageBox::Escape);
        if (ret == QMessageBox::Yes) return false;
        return true;
    }
    return false;
}

/**
 * \fn void MainWindow::readSettings()
 * \brief Appelée à l'initialisation de l'application,
 *        pour retrouver les paramètres importants de
 *        la dernière session.
 */
void MainWindow::readSettings()
{
    QSettings settings("Collatinus", "collatinus11");
    // état de la fenêtre
    settings.beginGroup("fenetre");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
    settings.endGroup();
    // dernier fichier chargé
    settings.beginGroup("fichiers");
    nfAb = settings.value("nfAb").toString();
    if (!nfAb.isEmpty())
    {
        charger(nfAb);
        nfAd = nfAb;
        nfAd.prepend("coll-");
    }
    settings.endGroup();
    settings.beginGroup("options");
    // police
    font.setPointSize(settings.value("zoom").toInt());
    // font.setFamily(settings.value("police").toString());
    editLatin->setFont(font);
    textEditLem->setFont(font);
    textBrowserDic->setFont(font);
    textBrowserW->setFont(font);
    textEditScand->setFont(font);
    textBrowserFlex->setFont(font);
    textBrowserSynt->setFont(font);
    // options de lemmatisation
    alphaOptAct->setChecked(settings.value("alpha").toBool());
    formeTAct->setChecked(settings.value("formetxt").toBool());
    extensionWAct->setChecked(settings.value("extensionlexique").toBool());
    htmlAct->setChecked(settings.value("html").toBool());
    majPertAct->setChecked(settings.value("majpert").toBool());
    morphoAct->setChecked(settings.value("morpho").toBool());
    nonRecAct->setChecked(settings.value("nonrec").toBool());
    // options d'accentuation
    accentAct->setChecked(settings.value("accentuation").toBool());
    optionsAccent->setEnabled(settings.value("accentuation").toBool());
    longueAct->setChecked(settings.value("longue").toBool());
    breveAct->setChecked(settings.value("breve").toBool());
    ambigueAct->setChecked(settings.value("ambigue").toBool());
    illiusAct->setChecked(settings.value("illius").toBool());
    hyphenAct->setChecked(settings.value("hyphenation").toBool());
    repHyphen = settings.value("repHyphen").toString();
    if (repHyphen.isEmpty()) repHyphen = qApp->applicationDirPath() + "/data";

    QString l = settings.value("cible").toString();
    lemmatiseur->setCible(l);
    foreach (QAction *action, grCibles->actions())
        if (action->text() == lemmatiseur->cibles()[l])
            action->setChecked(true);
    settings.endGroup();
    // options appliquées au lemmatiseur
    lemmatiseur->setAlpha(alphaOptAct->isChecked());
    lemmatiseur->setFormeT(formeTAct->isChecked());
    lemmatiseur->setExtension(extensionWAct->isChecked());
    lemmatiseur->setHtml(htmlAct->isChecked());
    lemmatiseur->setMajPert(majPertAct->isChecked());
    lemmatiseur->setMorpho(morphoAct->isChecked());
    settings.beginGroup("dictionnaires");
    comboGlossaria->setCurrentIndex(settings.value("courant").toInt());
    wDic->move(settings.value("posw").toPoint());
    wDic->resize(settings.value("sizew").toSize());
    wDic->setVisible(settings.value("wdic").toBool());
    comboGlossariaW->setCurrentIndex(settings.value("courantW").toInt());
    syncAct->setChecked(settings.value("sync").toBool());
    settings.endGroup();
}

/**
 * \fn void MainWindow::recherche()
 * \brief Recherche dans l'éditeur de texte latin.
 */
void MainWindow::recherche()
{
    bool ok;
    rech = QInputDialog::getText(this, tr("Recherche"), tr("Chercher :"),
                                 QLineEdit::Normal, rech, &ok);
    if (ok && !rech.isEmpty())
    {
        if (!editLatin->find(rech))
        {
            rech = QInputDialog::getText(this, tr("Chercher"),
                                         tr("Retour au début ?"),
                                         QLineEdit::Normal, rech, &ok);
            if (ok && !rech.isEmpty())
            {
                // Retourner au debut
                editLatin->moveCursor(QTextCursor::Start);
                // Chercher à nouveau
                editLatin->find(rech);
            }
        }
    }
}

/**
 * \fn void MainWindow::rechercheBis()
 * \brief Suite de la recherche.
 *
 */
void MainWindow::rechercheBis()
{
    if (rech.isEmpty()) return;
    bool ok = editLatin->find(rech);
    if (!ok)
    {
        QTextCursor tc = editLatin->textCursor();
        editLatin->moveCursor(QTextCursor::Start);
        ok = editLatin->find(rech);
        if (!ok) editLatin->setTextCursor(tc);
    }
}

/**
 * \fn void MainWindow::scandeLigne()
 * \brief scande le contenu de la ligne de saisie du
 *        dock Scansion, et affiche le résultat.
 */
void MainWindow::scandeLigne()
{
    int accent = lireOptionsAccent();
    textEditScand->setHtml(
        lemmatiseur->scandeTxt(lineEditScand->text(), accent, false));
}

/**
 * \fn void MainWindow::scandeTxt()
 * \brief Lance la scansion du texte latin, et affiche le
 *        résultat dans le dock scansion.
 */
void MainWindow::scandeTxt()
{
    int accent = lireOptionsAccent();
    textEditScand->setHtml(
        lemmatiseur->scandeTxt(editLatin->toPlainText(), accent, false));
}

/**
 * \fn void MainWindow::setCible()
 * \brief Coordonne la langue cible cochée dans le menu
 *        et la langue cible du lemmatiseur.
 */
void MainWindow::setCible()
{
    QAction *action = grCibles->checkedAction();
    foreach (QString cle, lemmatiseur->cibles().keys())
    {
        if (lemmatiseur->cibles()[cle] == action->text())
        {
            lemmatiseur->setCible(cle);
            break;
        }
    }
}

/**
 * \fn void MainWindow::setLangue()
 * \brief lis la langue d'interface, et
 *        procède aux initialisations.
 */
void MainWindow::setLangue()
{
    QSettings settings("Collatinus", "collatinus11");
    settings.beginGroup("interface");
    langueI = settings.value("langue").toString();
    settings.endGroup();
    if (!langueI.isEmpty())
    {
        translator = new QTranslator(qApp);
        translator->load(qApp->applicationDirPath() + "/collatinus_" + langueI);
        qApp->installTranslator(translator);
    }
    else
        langueI = "fr";
}

/**
 * \fn void MainWindow::stat()
 * \brief Affiche les statistiques de lemmatisation et
 *        de scansion si le dock correspondant est visible.
 */
void MainWindow::stat()
{
    if (dockVisible(dockLem))
    {
        textEditLem->setHtml(
            lemmatiseur->frequences(editLatin->toPlainText()).join(""));
    }
    if (dockVisible(dockScand))
        textEditScand->setHtml(
            lemmatiseur->scandeTxt(editLatin->toPlainText(), 0, true));
}

/**
 * \fn void MainWindow::syncDW()
 * \brief effectue dans le dictionnaire supplémentaire
 *        la même recherche que celle qui a été faite dans le
 *        principal.
 */
void MainWindow::syncDW()
{
    if (wDic->isVisible())
    {
        lineEditDicW->setText(lineEditDic->text());
        afficheLemsDicW();
    }
    else
        montreWDic(true);
}

/**
 * \fn void MainWindow::syncWD()
 * \brief effectue dans le dictionnaire principal
 *        la même recherche que celle qui a été faite dans le
 *        supplémentaire.
 */
void MainWindow::syncWD()
{
    lineEditDic->setText(lineEditDicW->text());
    afficheLemsDic();
}

void MainWindow::setAccent(bool b)
{
    optionsAccent->setEnabled(b);
    illiusAct->setEnabled(b);
    hyphenAct->setEnabled(b);
}

int MainWindow::lireOptionsAccent()
{
    int retour = 0;
    if (accentAct->isChecked())
    {
        if (illiusAct->isChecked()) retour += 8;
        if (hyphenAct->isChecked()) retour += 4;
        if (longueAct->isChecked()) return retour + 1;
        if (breveAct->isChecked()) return retour + 2;
        retour += 3;
    }
    return retour;
}

void MainWindow::lireFichierHyphen()
{
    QString ficIn = QFileDialog::getOpenFileName(this, "Capsam legere", repHyphen+"/hyphen.la");
    if (!ficIn.isEmpty()) repHyphen = QFileInfo (ficIn).absolutePath ();
    lemmatiseur->lireHyphen(ficIn);
}

void MainWindow::oteDiacritiques()
{
    QString texte = editLatin->toPlainText();
    texte.replace("ç","s");
    texte.replace("Ç","S");
    texte = texte.normalized(QString::NormalizationForm_D, QChar::currentUnicodeVersion());
    texte.remove("\u0300");
    texte.remove("\u0301");
    texte.remove("\u0302");
    texte.remove("\u0304");
    texte.remove("\u0306");
    texte.remove("\u0308");
    editLatin->setText(texte);
}

void MainWindow::lancerServeur(bool run)
{
    if (run)
    {
        QMessageBox::about(this,
             tr("Serveur de Collatinus"), startServer());

    }
    else
    {
        QMessageBox::about(this,
             tr("Serveur de Collatinus"), stopServer());

    }
}

void MainWindow::connexion ()
{
    soquette = serveur->nextPendingConnection ();
    connect (soquette, SIGNAL (readyRead ()), this, SLOT (exec ()));
}

void MainWindow::exec ()
{
    QByteArray octets = soquette->readAll ();
    QString requete = QString (octets).trimmed();
    if (requete.isEmpty()) requete = "-?";
    QString texte = "";
    QString rep = "";
    bool nonHTML = true;
    QString fichierSortie = "";
    if (requete.contains("-o "))
    {
        // La requête contient un nom de fichier de sortie
        QString nom = requete.section("-o ",1,1).trimmed();
        requete = requete.section("-o ",0,0).trimmed();
        // En principe, le -o vient à la fin. Mais...
        if (nom.contains(" "))
        {
            fichierSortie = nom.section(" ",0,0);
            // Le nom de fichier ne peut pas contenir d'espace !
            if (requete.isEmpty()) requete = nom.section(" ",1);
            else requete.append(" " + nom.section(" ",1));
        }
        else fichierSortie = nom;
    }
    if (requete.contains("-f "))
    {
        // La requête contient un nom de fichier
        QString nom = requete.section("-f ",1,1).trimmed();
        requete = requete.section("-f ",0,0).trimmed();
        QFile fichier(nom);
        if (fichier.open(QFile::ReadOnly))
        {
            texte = fichier.readAll();
            fichier.close();
        }
        else rep = "fichier non trouvé !\n";
    }
    if (rep == "")
    {
    if ((requete[0] == '-') && (requete.size() > 1))
    {
        char a = requete[1].toLatin1();
        QString options = requete.mid(0,requete.indexOf(" "));
        QString lang = lemmatiseur->cible(); // La langue actuelle;
        bool html = lemmatiseur->optHtml(); // L'option HTML actuelle
        bool MP = lemmatiseur->optMajPert();
        lemmatiseur->setHtml(false); // Sans HTML, a priori
        int optAcc = 0;
        if (texte == "")
            texte = requete.mid(requete.indexOf(" ")+1);
        lemmatiseur->setMajPert(requete[1].isUpper());
        switch (a)
        {
        case 'S':
        case 's':
            if ((options.size() > 2) && (options[2].isDigit()))
                optAcc = options[2].digitValue() & 7;
            rep = lemmatiseur->scandeTxt(texte,0,optAcc==1);
            if (optAcc==1) nonHTML = false;
            break;
        case 'A':
        case 'a':
            optAcc = 3; // Par défaut : un mot dont la pénultième est commune n'est pas accentué.
            if ((options.size() > 2) && (options[2].isDigit()))
            {
                optAcc = options[2].digitValue();
                if ((options.size() > 3) && (options[3].isDigit()))
                    optAcc = 10 * optAcc + options[3].digitValue();
            }
            rep = lemmatiseur->scandeTxt(texte,optAcc,false);
            break;
        case 'H':
        case 'h':
            lemmatiseur->setHtml(true);
            nonHTML = false;
        case 'L':
        case 'l':
            if ((options.size() > 2) && (options[2].isDigit()))
            {
                optAcc = options[2].digitValue();
                options = options.mid(3);
                if ((options.size() > 0) && (options[0].isDigit()))
                {
                    optAcc = 10*optAcc+options[0].digitValue();
                    options = options.mid(1);
                }
            }
            else options = options.mid(2); // Je coupe le "-l".
            if ((options.size() == 2) && lemmatiseur->cibles().keys().contains(options))
                lemmatiseur->setCible(options);
            if (optAcc > 15) rep = lemmatiseur->frequences(texte).join("");
            else rep = lemmatiseur->lemmatiseT(texte,optAcc&1,optAcc&2,optAcc&4,optAcc&8);
            lemmatiseur->setCible(lang); // Je rétablis les langue et option HTML.
            break;
        case 'X':
        case 'x':
//            rep = lemmatiseur->txt2XML(requete);
            rep = "Pas encore disponible";
            break;
        case 'c':
            if (options.size() > 2)
                lemmatiseur->setMajPert(options[2] == '1');
            break;
        case 't':
            if (options.size() == 4)
            {
                lemmatiseur->setCible(options.mid(2,2));
            }
            else
            {
                QStringList clefs = lemmatiseur->cibles().keys();
                rep = "Les langues connues sont : " + clefs.join(" ") + "\n";
            }
            break;
//        case '?':
        default: // Tout caractère non-affecté affiche l'aide.
            rep = "La syntaxe est '[commande] [texte]' ou '[commande] -f nom_de_fichier'.\n";
            rep += "Éventuellement complétée par '-o nom_de_fichier_de_sortie'.\n";
            rep += "Par défaut (sans commande), on obtient la scansion du texte.\n";
            rep += "Les commandes possibles sont : \n";
            rep += "\t-s : Scansion du texte (-s1 : avec recherche des mètres).\n";
            rep += "\t-a : Accentuation du texte (avec options -a1..-a15).\n";
            rep += "\t-l : Lemmatisation du texte (avec options -l0..-l15, -l16 pour les fréquences).\n";
            rep += "\t-h : Lemmatisation du texte en HTML (mêmes options que -l).\n";
            rep += "\t-S, -A, -L, -H : Les mêmes avec Majuscules pertinentes.\n";
            rep += "\t-t : Langue cible pour les traductions (par exemple -tfr, -tuk).\n";
            rep += "\t-C : Majuscules pertinentes.\n";
            rep += "\t-c : Majuscules non-pertinentes.\n";
            rep += "\t-? : Affichage de l'aide.\n";
 //           rep += "\t-x : Mise en XML du texte.\n";
            break;
        }
        lemmatiseur->setHtml(html);
        if ((a != 'C') && (a != 'c'))
            lemmatiseur->setMajPert(MP);
    }
    else if (texte != "") rep= lemmatiseur->scandeTxt(texte);
    else rep= lemmatiseur->scandeTxt(requete);
    }
    if (nonHTML)
    {
        rep.remove("<br />"); // Avec -H/h, j'ai la lemmatisation en HTML
        rep.remove("<br/>"); // Avec -H/h, j'ai la lemmatisation en HTML
    }
//    rep.replace("<br />","\n");
    if (fichierSortie == "")
    {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(rep);
    }
    else
    {
        QFile ficOut(fichierSortie);
        if (ficOut.open(QFile::WriteOnly))
        {
            ficOut.write(rep.toUtf8());
            ficOut.close();
            rep = "Done !\n";
        }
        else rep = "Unable to write !\n";
    }
    QByteArray ba = rep.toUtf8();
    soquette->write(ba);
}

QString MainWindow::startServer()
{
    serveur = new QTcpServer (this);
    connect (serveur, SIGNAL(newConnection()), this, SLOT (connexion ()));
    if (!serveur->listen (QHostAddress::LocalHost, 5555))
    {
        return "Ne peux écouter.";
    }
    return "Le serveur est lancé.";
}

QString MainWindow::stopServer()
{
    serveur->close();
    delete serveur;
    return "Le serveur est éteint.";
}

void MainWindow::dockRestore()
{
    dockLem->setFloating(false);
    dockLem->show();
    dockScand->setFloating(false);
    dockScand->show();
    dockDic->setFloating(false);
    dockDic->show();
    dockFlex->setFloating(false);
    dockFlex->show();
    dockSynt->setFloating(false);
    dockSynt->show();
}
