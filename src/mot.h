/*               mot.h
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

#ifndef MOT_H
#define MOT_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QtCore/QCoreApplication>

#include "lemCore.h"
#include "lemme.h"
#include "ch.h"

/**
 * @brief La classe Mot contient le mot et ses analyses possibles
 *
 * Cette classe est utilisée par le Tagueur pour y stocker les résultats
 * de l'analyse d'une forme ainsi que le tag associé.
 * Le Tagueur évalue des probabilités et en déduit la séquence de tags
 * la plus probable pour la phrase. Il revient ensuite aux mots
 * pour associer un lemme à ce tag.
 */
class Mot : public QObject
{
    Q_OBJECT
public:
    Mot(QString forme, int rang, bool debVers, QObject *parent = 0);
    QString choisir(QString t = "", int np = 0, bool tout = true);
    long proba(QString t);
    QStringList tags();
    QString forme();
    QString tagEncl();
    bool inconnu();
    void setBestOf(QString t, double pr);
    double bestOf(QString t);

private:
    LemCore* _lemCore;
    QString _forme;
    int _rang;
    QString _tagEncl;
    MapLem _mapLem;
    QStringList _lemmes;
    QStringList _morphos;
    QStringList _tags;
    QList<int> _nbOcc;
    QMap<QString,long> _probas;
    QString _maxProb;
    QMap<QString,double> _bestOf;
};

#endif // MOT_H
