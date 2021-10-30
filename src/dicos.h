/*                    dicos.h
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

#ifndef LEWIS_H
#define LEWIS_H

#include <QtCore>

//typedef QPair<QString, qint64> pairL;

/**
 * @brief groupe le lemme de l'article, sa position dans le fichier et sa taille
 *
 * Cette structure sera utilisée pour afficher les articles des dictionnaires
 * en XML/HTML.
 */
typedef struct
{
    QString article; /*!< le lemme */
    qint64 pos; /*!< la position de l'article dans le fichier */
    qint64 taille; /*!< la taille de l'article compressé */
} llew;
// typedef QList <pairL> llew;

/**
 * @brief La classe Dictionnaire contient l'information relative à un dictionnaire
 *
 */
class Dictionnaire : public QObject
{
    Q_OBJECT

   private:
    QString auteur;
    QString chData;  // chemin complet du fichier djvu ou xml
    QString chopNum(const QString c);
    QString cond_jv;
    // Pour pouvoir lire l'index de dicos en Tchèque, par exemple,
    // je dois pouvoir modifier l'ordre alphabétique.
    QString alphabet;
    // Dans le fichier cfg, la variable "alphabet" donne
    // toutes les "lettres" utilisées, séparées par un point ".".
    QStringList caracteres; // En Tchèque, le digraphe "ch" est entre le h et le i.
    QList<int> indices;
    // Les caractères ont dû être ordonnés, mais en conservant leur indice
    // dans l'ordre alphabétique.
    int compChaines(QString s1, QString s2);
    int nbCar;
    // Une fonction pour comparer deux chaines en tenant compte
    // de l'ordre alphabétique donné par alphabet.
    int debut;
    bool djvu;
    QString echelle;  // échelle pour l'extraction d'une image d'un djvu
    QStringList idxDjvu;
    QString idxJv;  // chemin de l'index jv
    bool ji;
    bool JI;
    bool jv;
    QStringList _liens;
    QString ligneLiens;
    QString linPrec(QTextStream *s, qint64 pos);
    QString n;  // nom
    int pdj;
    QString prec;  // pages précédente et suivante
                   // QString     readLineBack (QFile *f, int fois=1);
    QString repertoire;
    QString suiv;
//    qint64 tailleprec;
    QString url;
    bool xml;
    bool xsl;

   public:
    Dictionnaire(QString cfg, QObject *parent = 0);
    QString entree_pos(qint64 pos, qint64 taille);
    bool estXml();
//    QString indexIu(); // N'est pas définie dans dicos.cpp ?
    QString indexJv();
    bool lis_index_djvu();
    QStringList liens();  // renvoie liens
    QString nom();
    int noPageDjvu();
    QString page(QStringList req,
                 int no = 0);  // lien si la requête vient d'un hyperlien djvu
    QString pageDjvu(int p);
    QString pageDjvu(QStringList req,
                     int no = 0);  // surcharge avec calcul des no de page
    QString pageXml(QStringList lReq);
    QString pgPrec();
    QString pgSuiv();
    QString ramise(QString f);
    void vide_index();
    void vide_ligneLiens();
};

/**
 * @brief La classe ListeDic gère l'ensemble des dictionnaires présents
 *
 * Cette classe est un peu étrange. En particulier, les dictionnaires
 * courants y sont stockés, alors qu'a priori ils sont liés à la GUI,
 * donc à MainWindow. Je me demande si je ne devrais pas réorganiser
 * tout ça en remettant la QMap liste et les dictionnaires courants
 * directement dans MainWindow. C'est d'autant plus vrai que
 * MainWindow::createDicos crée les dicos deux fois, pour peupler
 * les deux comboBox, et maintient une liste des noms des dicos.
 *
 * La seule fonction "utile" dans cette classe serait
 * ListeDic::dictionnaire_par_nom
 * mais elle serait avantageusement remplacée par une ligne du genre
 * if (_listeDicos.contains(nom)) courant = _listeDicos.value(nom);
 * D'autre part, on peut récupérer l'indice entier de l'élément choisi
 * dans la comboBox, donc il n'y a rien à chercher...
 *
 * @bug Pourquoi liste est-elle une QMultiMap ?
 * Comme la recherche se fait par nom (donc par la clef) et
 * qu'on ne retient que la première réponse, plusieurs
 * occurrences d'une même clef sont inutiles.
 * Autant utiliser une QMap.
 */
class ListeDic : public QObject
{
    Q_OBJECT

   private:
    /*! QMap associant le nom du dictionnaire avec un pointeur vers l'objet */
    QMultiMap<QString, Dictionnaire *> liste;
    /*! pointeur vers le dictionnaire courant */
    Dictionnaire *currens = NULL;
    /*! pointeur vers le dictionnaire de la 2e fenêtre */
    Dictionnaire *currens2 = NULL;

   public:
    Dictionnaire *dictionnaire_par_nom(QString nom);
    void ajoute(Dictionnaire *d);
    void change_courant(QString nom);
    Dictionnaire *courant();
    void change_courant2(QString nom);
    Dictionnaire *courant2();
    // page renvoie le code html de la page
    // du dictionnaire courant
    // correspondant au lemme l.
};
// Revoir cette classe ListeDic : elle ne devrait pas contenir les dicos
// courants

#endif
