/*           modele.h
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

#ifndef MODELE_H
#define MODELE_H

#include <QList>
#include <QMultiMap>
#include <QString>
#include <QStringList>
#include <QtCore>

#include <QDebug>

#include "ch.h"
#include "lemCore.h"

class LemCore;
class Modele;

class Desinence : public QObject
{
    Q_OBJECT
   private:
    QString _gr;
    QString _grq;
    int     _morpho;
    Modele *_modele;
    int     _numR;
    int     _rarete;

   public:
    Desinence(QString d, int morph, int nr, Modele *parent = 0);
    QString gr();
    QString grq();
    int     rarete();
    Modele *modele();
    int     morphoNum();
    int     numRad();
    void    setModele(Modele *m);
};

class Modele : public QObject
{
    Q_OBJECT
   private:
    QList<int> _absents;
    QStringList static const cles;
    QMultiMap<int, Desinence *> _desinences;
    QMap<int, QString> _genRadicaux;
    QString _gr;
    QString _grq;
    LemCore *_lemCore;
    Modele *_pere;
    QChar   _pos;
    QString _suf;

   public:
    Modele(QStringList ll, LemCore *parent = 0);
    bool               absent(int a);
    QList<int>         absents();
    QList<int>         clesR();
    Desinence         *clone(Desinence *d);
    bool               deja(int m);
    QList<Desinence *> desinences(int d);
    QList<Desinence *> desinences();
    bool               estUn(QString m);
    QString            genRadical(int r);
    QString            gr();
    QString            grq();
    static QList<int>  listeI(QString l);
    QList<int>         morphos();
    QChar              pos();
};

#endif
