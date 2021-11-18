/*               scandeur.h
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

#ifndef SCANDEUR
#define SCANDEUR

#include <QDebug>

#include "ch.h"
#include "lemCore.h"

/**
 * @brief La classe Scandeur regroupe les fonctions nécessaires
 * à la scansion et à l'accentuation des formes ou des textes.
 *
 * Actuellement, dans Collatinus, elle est appelée par MainWindow
 * qui gère l'affichage et les lectures/écritures des fichiers.
 * Elle partage donc le noyau de lemmatisation, LemCore,
 * avec d'autres classes _intermédiaires_.
 * Toutefois, cette classe pourrait être autonome, avec une autre interface,
 * si on voulait, par exemple, n'avoir qu'un programme spécialisé
 * dans la scansion et l'accentuation.
 *
 * Les points d'entrée sont les deux fonctions publiques
 * Scandeur::scandeTxt et Scandeur::txt2csv.
 * Elles prennent toutes les deux un texte en entrée
 * et elles le préparent respectivement pour l'affichage
 * à l'écran (scandé **ou** accentué) et pour la sauvegarde
 * dans un fichier de type CSV (scandé **et** accentué).
 */
class Scandeur : public QObject
{
public:
    Scandeur(QObject *parent = 0, LemCore *l=0, QString resDir="");
    // Pour scander, un texte.
    QString parPos(QString f);
    QString scandeTxt(QString texte, int accent = 0, bool stats = false, bool majAut = false);
    QString txt2csv(QString texte, int accent = 9, bool majAut = false);

private:
    /*! Un pointeur vers le noyau de lemmatisation qui peut être partagé. */
    LemCore * _lemCore;
    /*! Le nom du répertoire contenant les données. */
    QString _resDir;
    /*! La liste des règles pour déterminer les quantités par position. */
    QList<Reglep> _reglesp;
    void lisParPos();
    QStringList cherchePieds(int nbr, QString ligne, int i, bool pentam);
    QStringList formeq(QString forme, bool *nonTrouve, bool debPhr,
                       int accent = 0);
    QString code(QString PC, int accent);
};

#endif // SCANDEUR

