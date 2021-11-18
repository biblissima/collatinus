/*               tagueur.h
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

#ifndef TAGUEUR_H
#define TAGUEUR_H

#include "lemCore.h"
#include "mot.h"
#include "ch.h"

/**
 * @brief La classe Tagueur regroupe les fonctions nécessaires
 * pour lemmatiser et désambiguïser des textes avec un tagueur probabiliste.
 *
 * Actuellement, dans Collatinus, elle est appelée par MainWindow
 * qui gère l'affichage et les lectures/écritures des fichiers.
 * Elle partage donc le noyau de lemmatisation, LemCore,
 * avec d'autres classes _intermédiaires_.
 * Toutefois, cette classe pourrait être autonome, avec une autre interface,
 * si on voulait, par exemple, n'avoir qu'un programme spécialisé.
 *
 * Cette classe ne définit qu'une fonction Tagueur::tagTexte
 * qui prend un texte en entrée et le prépare pour l'affichage
 * en HTML ou pour une sauvegarde au format CSV.
 * Elle utilise la classe Mot qui va contenir les
 * lemmatisations possibles des mots du texte
 * et choisira parmi les tags proposés la **meilleure**
 * séquence possible pour chaque phrase.
 *
 * \note Lors de l'affichage en HTML, je donne aussi le second choix.
 * \todo Je devrais essayer d'améliorer ce second choix en m'appuyant
 * sur les _points fixes_ (deux mots successifs n'ayant
 * qu'un seul tag possible qui vont donc réduire l'ensemble des
 * séquences de tags à une seule).
 * Il serait intéressant d'avoir pour chaque segment entre
 * deux points fixes les deux meilleurs choix de séquences.
 * Ainsi, pour une phrase contenant un _point fixe_,
 * j'aurais quatre séquences sélectionnées au lieu de deux.
 *
 */
class Tagueur : public QObject
{
public:
    Tagueur(QObject *parent = 0, LemCore *l=0, QString cible = "", QString resDir="");
    // Pour le tagger
    QString tagTexte(QString t, int p, bool affTout = true, bool majPert = true, bool affHTML = true);

private:
    /*! Un pointeur vers le noyau de lemmatisation qui peut être partagé. */
    LemCore * _lemCore;
    /*! Le nom du répertoire contenant les données. */
    QString _resDir;
};

#endif // TAGUEUR_H
