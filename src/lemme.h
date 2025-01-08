/*               lemme.h
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

#ifndef LEMME_H
#define LEMME_H

#include <QMultiMap>
#include <QObject>
#include <QString>
#include <QDebug>
#include <QStringList>

#include "irregs.h"
#include "lemCore.h"
#include "modele.h"
#include "ch.h"

class Irreg;
class LemCore;
class Lemme;
class Modele;

/**
 * @brief La classe Radical décrit les radicaux associés aux lemmes
 */
class Radical : public QObject
{
   private:
    QString _gr;
    QString _grq;
    Lemme* _lemme;
    int _numero;

   public:
    Radical(QString g, int n, QObject* parent);
    QString gr();
    QString grq() const;
    Lemme* lemme();
    Modele* modele();
    int numRad();
};

/**
 * @brief La classe Lemme décrit les lemmes.
 */
class Lemme : public QObject
{
    Q_OBJECT
   private:
    static    LemCore*          _lemCore;
    QString                     _cle;
    QString                     _gr;
    QString                     _grd;
    QString                     _grq;
    QString                     _grModele;
    QString                     _hyphen; // Pour les césures étymologies
    QString                     _indMorph;
    QList<Irreg*>               _irregs;
    Modele*                     _modele;
    int                         _nh;
    QList<int>                  _morphosIrrExcl;
    int                         _nbOcc; // Nombre d'occurrences du lemme dans les textes du LASLA
    int                         _origin; // lemmes ou lem_ext
    QString                     _pos;
    QMap<int, QList<Radical*> > _radicaux;
    QString                     _renvoi;
    QMap<QString,QString>       _traduction;

   public:
    Lemme(const QString linea, const int origin, QObject* parent);
    static    void      setLemCore(LemCore* l);
    void                ajIrreg(Irreg* irr);
    void                ajNombre(int n);
    void                ajRadical(int i, Radical* r);
    void                ajTrad(QString t, QString l);
    QString             ambrogio();
    QString             cle();
    QList<int>          clesR();
    bool                estIrregExcl(int nm);
    QString             genre();
    QString             getHyphen (); // Accesseurs pour les césures étymologiques
    QString             gr();
    QString             grq();
    QString             grModele();
    QString             humain(bool html = false, QString l = "fr", bool nbr = false);
    QString             indMorph();
    QString             irreg(int i, bool* excl);
    Modele*             modele();
    int                 nbOcc() const;    // Retourne le nombre d'occurrences du lemme
    void                clearOcc(); // Efface       "           "            "
    int                 nh();
    int                 origin();
    QString static      oteNh(QString g, int& nh);
    QString             pos();
    QList<Radical*>     radical(int r);
    bool                renvoi();
    void                setHyphen (QString h);
    QString             traduction(QString l);
    inline bool operator<(const Lemme &l) const;
};

#endif
