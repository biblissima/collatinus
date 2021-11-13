/*      lemmatiseur.h
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

#ifndef LEMMATISEUR_H
#define LEMMATISEUR_H

#include <QObject>
#include <QHash>

#include <QDebug>

#include "ch.h"
#include "lemCore.h"

/**
 * @brief La classe Lemmatiseur regroupe les fonctions nécessaires
 * à la lemmatisation et à l'analyse morphologique des formes ou des textes.
 *
 * Actuellement, dans Collatinus, elle est appelée par MainWindow
 * qui gère l'affichage et les lectures/écritures des fichiers.
 * Elle partage donc le noyau de lemmatisation, LemCore,
 * avec d'autres classes _intermédiaires_.
 * Toutefois, cette classe pourrait être autonome, avec une autre interface,
 * si on voulait, par exemple, n'avoir qu'un programme spécialisé
 * dans la lemmatisation et l'analyse morphologique.
 */
class Lemmatiseur : public QObject
{
    Q_OBJECT

public:
    Lemmatiseur(QObject *parent = 0, LemCore *l=0, QString cible="", QString resDir="");
    // Créateur de la classe
    QStringList frequences(QString txt);
    QStringList lemmatiseF(QString f, bool deb);
    QString lemmatiseFichier(QString f, bool alpha = false,
                             bool cumVocibus = false, bool cumMorpho = false,
                             bool nreconnu = true);
    // lemmatiseT lemmatise un texte
    QString lemmatiseT(QString &t, bool alpha, bool cumVocibus = false,
                       bool cumMorpho = false, bool nreconnu = false);
    QString lemmatiseT(QString &t);

    void verbaOut(QString fichier); // Connaître l'usage des mots connus
    void verbaCognita(QString fichier, bool vb=false); // Coloriser le texte avec les mots connus

    // accesseurs d'options
    bool optAlpha();
    bool optHtml();
    bool optFormeT();
    bool optMajPert();
    bool optMorpho();
    bool optNonRec();
    QString cible();

public slots :
    // modificateurs d'options
    void setAlpha(bool a);
    void setCible(QString c);
    void setHtml(bool h);
    void setFormeT(bool f);
    void setMajPert(bool mp);
    void setMorpho(bool m);
    void setNonRec(bool n);

private:
    /*! Un pointeur vers le noyau de lemmatisation qui peut être partagé. */
    LemCore * _lemCore;
    /*! Le nom du répertoire contenant les données. */
    QString _resDir;
    QHash<QString,int> _hLem; /*!< Liste des lemmes connus. Voir Lemmatiseur::verbaCognita */
    QStringList _couleurs; /*!< Les couleurs pour le TextiColor. Voir Lemmatiseur::verbaCognita */
    // options
    bool _alpha; /*!< Option pour que les résultats soient présentés en ordre alphabétique. */
    bool _formeT; /*!< Option pour que la lemmatisation soit précédée par la forme du texte. */
    bool _html; /*!< Option pour que les résultats soient présentés en HTML. */
    bool _majPert; /*!< Option pour que la majuscule soit considérée comme pertinente. */
    bool _morpho; /*!< Option pour que la lemmatisation soit suivie par l'analyse morphologique. */
    bool _nonRec; /*!< Option pour que les formes inconnues soient regroupées à la fin des résultats. */
    QString _cible;  /*!< langue courante, 2 caractères ou plus */

};

#endif // LEMMATISEUR_H
