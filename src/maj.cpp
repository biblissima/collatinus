/*               maj.cpp
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

#include "maj.h"

Maj::Maj(bool dic, QDialog *parent) : QDialog(parent)
{
    _dico = dic;
    QLabel *icon = new QLabel;
    icon->setPixmap(QPixmap(":/res/collatinus.ico"));
    // label d'information
    QString texte =
        tr("Sur Collatinus peut se greffer une collection "
           "de lexiques et de dictionnaires. À l'installation, "
           "on ne dispose que d'une partie de ces ressources. "
           "Pour en ajouter, il faut se rendre sur le site "
           "<a href='http://outils.biblissima.fr/fr/collatinus/'> "
           "(<em>http://outils.biblissima.fr/fr/collatinus/</em>)</a>, "
           "consulter la liste des fichiers disponibles et leur "
           "version, et les télécharger en notant bien l'endroit "
           "où on les enregistre.<br/>\n"
           "Lorsque ce sera fait, il faudra aller les chercher en "
           "cliquant sur le bouton <em>Installer les paquets téléchargés</em> "
           "ci-dessous.<br/>\n"
           "Il est conseillé de revenir régulièrement sur "
           "<a href='http://outils.biblissima.fr/fr/collatinus/'> "
           "(<em>http://outils.biblissima.fr/fr/collatinus/</em>)</a> "
           "pour vérifier que l'on possède les dernières versions "
           "des lexiques et dictionnaires. Voici la liste de "
           "ce qui est installé sur cet ordinateur. "
           "Par exemple, le nom\n"
           "<b>Lewis_and_Short_1879-fev16.cz</b>\n"
           "signifie que ce dictionnaire a été mis en ligne en février "
           "2016.\n<br>\n<table><tr><td>• ");
    // liste des lexiques et dictionnaires + version
    label = new QLabel(this);
    label->setFont(this->font());
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignJustify);
    // liste des paquets installés
    if (dic)
    {
//        texte.append("<ul>\n<li>");
        QString dicDir = QDir::homePath() + "/collatinus/dicos/";
        QDir chDicos(dicDir);
        QStringList lcfg = chDicos.entryList(QStringList() << "*.cfg");
        for (int i = 0; i < lcfg.count(); ++i)
        {
            lcfg[i].remove(".cfg");
            if (lcfg[i][lcfg[i].size() - 6] == '-')
            {
                QString date = lcfg[i].section("-",-1);
                lcfg[i] = lcfg[i].section("-",0,-2);
                lcfg[i].append("&nbsp;</td><td>&nbsp;" + date);
            }
        }
        texte.append(lcfg.join("</td></tr>\n<tr><td>• "));
//        texte.append(lcfg.join("</li>\n<li>"));
//        texte.append("</li>\n</ul>");
    }
    else
    {
        // Les lexiques.
//        texte.append("<br>\n<table><tr><td>• ");
        QString resDir = "/usr/share/collatinus";
        QDir chDicos(resDir + "/data");
        QStringList lcfg = chDicos.entryList(QStringList() << "lem*.*");
        for (int i = 0; i < lcfg.count(); ++i)
        {
            QFile fi(qApp->applicationDirPath() + "/data/" + lcfg[i]);
            fi.open(QFile::ReadOnly|QFile::Text);
            QString blabla = fi.readLine();
            blabla = fi.readLine();
            if (blabla.startsWith("!")) blabla = blabla.mid(1);
            lcfg[i].append("&nbsp;</td><td>&nbsp;" + blabla);
            blabla = fi.readLine();
            lcfg[i].append("&nbsp;</td><td>&nbsp;" + blabla.mid(1));
            fi.close();
        }
        texte.append(lcfg.join("</td></tr>\n<tr><td>• "));
    }
    texte.append("</td></tr></table>");
    label->setText(texte);
    label->setOpenExternalLinks(true);

    // barre de boutons
    QPushButton *installButton =
        new QPushButton(tr("Installer les paquets téléchargés"));
    QPushButton *cloreButton = new QPushButton(tr("Fermer"));
    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addStretch();
    bottomLayout->addWidget(installButton);
    bottomLayout->addWidget(cloreButton);

    QVBoxLayout *verticalLayout = new QVBoxLayout(this);
    verticalLayout->addWidget(icon);
    verticalLayout->addWidget(label);
    verticalLayout->addLayout(bottomLayout);

    // Connexions
    connect(installButton, SIGNAL(clicked()), this, SLOT(selectionne()));
    connect(cloreButton, SIGNAL(clicked()), this, SLOT(close()));
}

bool Maj::installe(QString nfcol)
{
    // ouverture
    QFile fcol(nfcol);
    fcol.open(QFile::ReadOnly);
    // lecture des adresses en queue de fichier
    fcol.seek(fcol.size() - 100);
    QStringList lignes;
    while (!fcol.atEnd()) lignes << fcol.readLine().trimmed();
    if (!lignes[1].contains(":"))
    {
        QMessageBox::critical(
                    this, tr("Collatinus 11"),
                    tr("Impossible de comprendre le fichier ") + nfcol.toUtf8() +
                       tr(". Le format semble être inadéquat."));
        return false;
    }
//        qDebug() << lignes;

    if (_dico)
    {
        // nom du paquet
        QString nom = QFileInfo(nfcol).baseName();
        // Supprimer les versions antérieures
        QString nomSansDate = nom.section("-",0,-2) + "*.*";
        QString dicDir = QDir::homePath() + "/collatinus/dicos/";
        QDir rep(dicDir,nomSansDate);
        QStringList lfrem = rep.entryList();
//        qDebug() << lfrem;
        foreach (QString n, lfrem)
        {
            QFile::remove(dicDir + n);
        }
        // fichiers destination
        QString nf(dicDir + nom + ".");
        QString nfcz = nf + lignes[1].section(":",0,0);
        // Taille du 1er morceau
        qint64 taille = lignes[1].section(':', 1, 1).toLongLong();
        // Créations
        QDir dir(dicDir);
        if (!dir.exists()) dir.mkpath(".");
        QFile fcz(nfcz);
        if (!fcz.open(QFile::WriteOnly))
        {
            QMessageBox::critical(
                        this, tr("Collatinus 11"),
                        tr("Impossible de créer le fichier ") + nfcz.toUtf8() +
                           tr(". Vérifiez vos drois d'accès, et éventuellent "
                           "connectez-vous en administrateur avant de lancer Collatinus."));
            return false;
        }
        // écriture cz
        fcol.reset();
        fcz.write(fcol.read(taille));
        fcz.close();
        // décompression et écriture des autres
        for (int i = 2; i<lignes.size(); i++)
        {
            fcz.setFileName(nf + lignes[i].section(":",0,0));
            taille = lignes[i].section(':', 1, 1).toLongLong();
            fcz.open(QFile::WriteOnly);
            fcz.write(qUncompress(fcol.read(taille)));
            fcz.close();
        }
        // fermeture
        fcol.close();
    }
    else
    {
        // installer un lexique
        // nom du paquet
        QString nom = QFileInfo(nfcol).baseName();
//        qDebug() << nom;
        // fichiers destination
        QString nfDest(qApp->applicationDirPath() + "/data/");
        if (nom.startsWith("lemmes")) nfDest.append("lemmes.");
        else if (nom.startsWith("lem_ext")) nfDest.append("lem_ext.");
        else return false;
        /*
        if (QFile::exists(nfDest))
            QFile::remove(nfDest);
        // On ne peut pas copier si le fichier existe déjà
        QFile::copy(nfcol,nfDest);*/
        fcol.reset();
        QFile fcz;
//        qDebug() << lignes.size() << lignes[1];
        // décompression et écriture des lexiques
        for (int i = 1; i<lignes.size(); i++)
        {
            fcz.setFileName(nfDest + lignes[i].section(":",0,0));
//            qDebug() << fcz.fileName();
            qint64 taille = lignes[i].section(':', 1, 1).toLongLong();
            fcz.open(QFile::WriteOnly);
            fcz.write(qUncompress(fcol.read(taille)));
            fcz.close();
        }
        // fermeture
        fcol.close();
    }
    return true;
}

void Maj::selectionne()
{
    if (_dico)
    {
        QStringList nfichiers = QFileDialog::getOpenFileNames(
                    this, "Sélectionner un ou plusieurs paquets", QDir::homePath(),
                    "paquets dictionnaires (*.col)");
        listeF = nfichiers;
    }
    else
    {
        QStringList nfichiers = QFileDialog::getOpenFileNames(
                    this, "Sélectionner un ou plusieurs paquets", QDir::homePath(),
                    "paquets lexiques (*.col)");
        listeF = nfichiers;
    }
    if (listeF.empty()) return;
    bool OK = true;
    foreach (QString nfcol, listeF)
        if (_dico || nfcol.contains("/lemmes") || nfcol.contains("/lem_ext"))
        {
            bool OK1 = installe(nfcol);
            if (OK1) qDebug() << "installé" << nfcol;
            else OK = false;
        }
    // info
    if (OK) QMessageBox::information(this, tr("Collatinus 11"),
                             tr("L'installation s'est bien passée. "
                                "Au prochain lancement, les nouveaux lexiques "
                                "et dictionnaires seront disponibles."));
/*
    // Provisoirement, j'utilise la mise à jour pour créer les .col à partir des djvu.
    QStringList nfichiers = QFileDialog::getOpenFileNames(
        this, "Sélectionner un ou plusieurs paquets", qApp->applicationDirPath() + "/data/dicos/",
//        "dictionnaires djvu (*.djvu)");
                "lexiques (*.*)");
    listeF = nfichiers;
    if (listeF.empty()) return;
    bool OK = true;
    foreach (QString nfcol, listeF)
    {
//        bool OK1 = djvu2col(nfcol);
        bool OK1 = lem2col(nfcol);
        if (OK1) qDebug() << "installé" << nfcol;
        else OK = false;
    }
    // info
    if (OK) QMessageBox::information(this, tr("Collatinus 11"),
                             tr("La copie s'est bien passée. "));
*/
}

void Maj::setFont(QFont font) { label->setFont(font); }

/**
 * @brief Maj::djvu2col
 * @param nfdjvu
 * @return
 *
 * Fonction provisoire pour créer un fichier .col à partir
 * des fichiers djvu, idx et cfg présents dans /data/dicos.
 * C'est une fonction que je suis seul à utiliser, une seule fois.
 * Les utilisateurs utiliseront la fonction "installe" qui fait le contraire,
 * i.e. installer les fichiers djvu, idx et cfg dans /data/dicos
 * à partir d'un .col placé ailleurs.
 *
 */
bool Maj::djvu2col(QString nfdjvu)
{
    // nom du paquet
    QString nom = QFileInfo(nfdjvu).baseName();
    // fichiers destination
    QString dicDir = QDir::homePath() + "/collatinus/dicos/";
    QString nf(dicDir + nom);
    QString nfcol("/Users/Philippe/Documents/dicos_C11/" + nom + ".col");
    QString nfidx = nf + ".idx";
    QString nfcfg = nf + ".cfg";
    //qDebug() << nfdjvu << nfcol << nf;

    if (QFile::exists(nfcol))
        QFile::remove(nfcol);
    // On ne peut pas copier si le fichier existe déjà
    QFile::copy(nfdjvu,nfcol);
    // Je copie le fichier dans /Users/Philippe/Documents/dicos_C11/.
    QFile fcol(nfcol);
    if (!fcol.open(QFile::ReadWrite)) return false;
    fcol.seek(fcol.size());

    QFile fzi(nfidx);
    fzi.open(QFile::ReadOnly|QFile::Text);
    QString lin = fzi.readAll();
    fzi.close();

    qint64 p = fcol.pos();
    //qDebug() << p;
    QString nn = "%1:%2\n";
    QByteArray ba = qCompress(lin.toUtf8(),9);
    fcol.write(ba);
    QString piedDeFichier = "\n";
    piedDeFichier += nn.arg("djvu").arg(p);
    piedDeFichier += nn.arg("idx").arg(ba.size());

    fzi.setFileName(nfcfg);
    fzi.open (QFile::ReadOnly|QFile::Text);
    QByteArray baIn = fzi.readAll();
    fzi.close();

    ba = qCompress(baIn,9);
    p = fcol.pos();
    fcol.write(ba);
    piedDeFichier += nn.arg("cfg").arg(ba.size());

    int n = 100 - piedDeFichier.size();
    //        if (n<1) n += 64;
    //qDebug() << n;
    piedDeFichier.prepend(QString(n,' '));
    fcol.write(piedDeFichier.toUtf8());

    fcol.close();
    return true;
}

/**
 * @brief Maj::lem2col
 * @param nfLem
 * @return
 *
 * Fonction provisoire pour créer un fichier .col à partir
 * des fichiers djvu, idx et cfg présents dans /data/dicos.
 * C'est une fonction que je suis seul à utiliser, une seule fois.
 * Les utilisateurs utiliseront la fonction "installe" qui fait le contraire,
 * i.e. installer les fichiers djvu, idx et cfg dans /data/dicos
 * à partir d'un .col placé ailleurs.
 *
 */
bool Maj::lem2col(QString nfLem)
{
    // nom du paquet
    QString nom = QFileInfo(nfLem).baseName();
    QString ext = QFileInfo(nfLem).suffix();
    // fichiers destination
    QString nfcol("/Users/Philippe/Documents/dicos_C11/" + nom + "_" + ext + "-avr17.col");
    QFile fcol(nfcol);
    if (!fcol.open(QFile::WriteOnly)) return false;

    QFile fLem(nfLem);
    fLem.open(QFile::ReadOnly|QFile::Text);
    QString lin = fLem.readAll();
    fLem.close();

    QString nn = "%1:%2\n";
    QByteArray ba = qCompress(lin.toUtf8(),9);
    fcol.write(ba);
    QString piedDeFichier = "\n";
    piedDeFichier += nn.arg(ext).arg(ba.size());
    int n = 100 - piedDeFichier.size();
    //        if (n<1) n += 64;
    //qDebug() << n;
    piedDeFichier.prepend(QString(n,' '));
    fcol.write(piedDeFichier.toUtf8());

    fcol.close();
    return true;
}
