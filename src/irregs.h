/*      irregs.h
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

#ifndef IRREGS_H
#define IRREGS_H

#include <QList>
#include <QString>
#include <QStringList>

#include "lemCore.h"
#include "ch.h"
#include "lemme.h"
#include "modele.h"

#include <QDebug>

class LemCore;
class Lemme;

/**
 * @brief La classe Irreg décrit les formes irrégulières
 * dans la flexion d'un lemme
 */
class Irreg : public QObject
{
    Q_OBJECT

   private:
    bool _exclusif; /*!<  booléen qui signale que cette forme remplace la forme régulière */
    QString _gr; /*!< forme irrégulière sans quantité */
    QString _grq; /*!< forme irrégulière avec ses quantités */
    LemCore* _lemCore; /*!< pointeur vers le noyau de lemmatisation */
    Lemme* _lemme; /*!< pointeur vers le lemme */
    QList<int> _morphos; /*!< liste des morphos (entiers) pour cette forme et ce lemme*/

   public:
    Irreg(QString l, QObject* parent = 0);
    bool exclusif();
    QString gr();
    QString grq();
    Lemme* lemme();
    QList<int> morphos();
};

#endif
