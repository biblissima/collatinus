/*               mot.cpp
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

/**
 * \file mot.cpp
 * \brief définit la classe Mot
 *
 * Cette classe est utilisée par le tagueur, Tagueur
 */

#include "mot.h"

Mot::Mot(QString forme, int rang, bool debVers, QObject *parent)
{
//    qDebug() << forme;
    _forme = forme;
    _rang = rang;
    _lemCore = qobject_cast<LemCore *>(parent);
    _probas.clear();
    _tagEncl = "";
    if (forme.isEmpty())
    {
        _tags << "snt";
        _probas["snt"] = 1;
    }
    else
    {
        _mapLem = _lemCore->lemmatiseM(forme, (rang == 0) || debVers);
        QString enclitique = "";
        // échecs
        if (_mapLem.empty())
        {
            // Je ne sais pas encore quoi faire.
        }
        else foreach (Lemme *l, _mapLem.keys())
        {
            QString lem = l->humain(true, _lemCore->cible(), true);
            int nb = l->nbOcc();
            foreach (SLem m, _mapLem.value(l))
            {
                QString lt = _lemCore->tag(l, m.morpho); // Maintenant, c'est une liste de tags.
//                qDebug() << lem << lt;
                // Pour les analyses, je garde la liste de tags.
                long fr = nb * _lemCore->fraction(lt);
                _lemmes.append(lem);
                _tags.append(lt);
                _nbOcc.append(fr);
                //                    qDebug() << forme << lem << nb << lt << t << fr;
                if (m.sufq.isEmpty())
                {
                    if (m.morpho == 416) _morphos.append(m.grq);
                    else _morphos.append(m.grq + " " + _lemCore->morpho(m.morpho));
                }
                else
                {
                    if (m.morpho == 416) _morphos.append(m.grq + " + " + m.sufq);
                    else _morphos.append(m.grq + " + " + m.sufq + " " + _lemCore->morpho(m.morpho));
                    enclitique = m.sufq;
                }
                while (lt.size() > 2)
                {
                    QString t = lt.mid(0,3);
                    lt = lt.mid(4);
                    fr = nb * _lemCore->fraction(t);
                    _probas[t] += fr;
                }
            }
        }
//        if (Ch::abrev.contains(forme))
        if (_lemCore->estAbr(forme))
        {
            // C'est un nom à n'importe quel cas !
            _probas.clear();
//            _tags.clear();
            QString pseudo = "n%1";
            for (int i = 1; i < 7; i++)
            {
                QString t = pseudo.arg(i)+"1";
//                _tags.append(t);
                _probas[t] = _lemCore->tagOcc(t);
            }
        }
        // J'ai construit les listes de lemmes, morphos, tags et nombres d'occurrences.
        // J'ai aussi une QMap qui associe les tags aux probas, que je dois normaliser.
        long total = 0;
        foreach (QString t, _probas.keys()) total += _probas[t];
        if (total == 0)
        {
            total = 1;
            //qDebug() << forme << " : toutes les probas sont nulles !";
        }
        _maxProb = "";
        long prMax = -1;
        foreach (QString t, _probas.keys())
        {
            _bestOf[t] = 0.; // Je prépare une liste des tags
//            qDebug() << t << _probas[t];
            long pr = _probas[t] * 1024 /total;
            if (prMax < pr)
            {
                prMax = pr;
                _maxProb = t;
            }
            if (pr == 0) pr++;
            _probas[t] = pr;
        }

        if ((enclitique == "quĕ") || (enclitique == "vĕ")) _tagEncl = "ce ";
        else if (enclitique == "nĕ") _tagEncl = "de ";
        else if (enclitique == "st") _tagEncl = "v11";
        if (forme.endsWith("cum"))
        {
            bool encl = (forme == "mecum") || (forme == "tecum") || (forme == "secum");
            encl = encl || (forme == "nobiscum") || (forme == "vobiscum") || (forme == "quibuscum");
            encl = encl || (forme == "quacum") || (forme == "quocum") || (forme == "quicum");
            if (encl)
            {
//                qDebug() << forme << " avec enclitique";
                _tagEncl = "re ";
                if (_tags.isEmpty())
                {
                    _tags.append("p61");
                    _probas.insert("p61",1024);
                    _lemmes.append(forme);
                    _morphos.append("...");
                    _nbOcc.append(1);
                }
                else if (_probas.size() == 1)
                {
                    if (!_probas.keys().contains("p61"))
                    {
                        _tags[0]="p61";
                        _probas.clear();
                        _probas.insert("p61",1024);
                    }
                }
                else qDebug() << "Erreur sur " << forme << " : " << _tags;
            }
        }
    }
//    qDebug() << forme << _tags.size() << _tags;
}

QString Mot::choisir(QString t, int np, bool tout)
{
//    qDebug() << _forme << t << np << tout << _tags.isEmpty() << _tags.size();
    QString choix = "";
    int valeur = -1;
    for (int i=0; i < _tags.size(); i++)
        if ((_tags[i].contains(t)) && (valeur < _nbOcc[i]))
        {
            // _tags peut être une liste de tags, alors que t est un tag.
            choix = _lemmes[i] + " — " + _morphos[i];
            valeur = _nbOcc[i];
        }
    if (!choix.isEmpty())
    {
        choix.prepend("<br/>\n—&gt;&nbsp;<span style='color:black'>");
        choix.append("</span>\n");
    }
    if (tout || choix.isEmpty())
    {
        choix.append("<span style='color:#777777'><ul>\n");
        for (int i=0; i < _tags.size(); i++)
        {
            QString format = "%1 : %2 ; ";
            QString lg = "<li>" + _lemmes[i] + " — " + _morphos[i] + " (";
            QString lt = _tags[i];
//            qDebug() << lg << lt;
            if (lt.size() > 2)
            {
                while (lt.size() > 2)
                {
                    QString t = lt.mid(0,3);
                    lt = lt.mid(4);
                    lg.append(format.arg(t).arg(_bestOf[t]));
                }
                lg.chop(3);
                lg.append(")</li>\n");
            }
            else lg.append(" ? )</li>\n");
            choix.append(lg);
        }
        choix.append("</ul></span>\n");
    }
    QString ajout;
    if (t == _maxProb) ajout = t;
    else ajout = t + " (" + _maxProb + ")";
    QString debut = "<li id='S_%1_W_%2'><strong>";
    choix.prepend(debut.arg(np).arg(_rang) + _forme + "</strong> " + ajout);
    choix.append("</li>");
    return choix;
}

QString Mot::tagEncl()
{
    return _tagEncl;
}

bool Mot::inconnu()
{
    return _tags.isEmpty();
}

QStringList Mot::tags()
{
    QStringList ret = _probas.keys();
    return ret;
}

long Mot::proba(QString t)
{
    if (_probas.contains(t))
        return _probas[t];
    return 0;
}

QString Mot::forme()
{
    return _forme;
}

void Mot::setBestOf(QString t, double pr)
{
//    qDebug() << t << pr;
    if (_bestOf.keys().contains(t))
    {
        if (pr > _bestOf[t]) _bestOf[t] = pr;
    }
    else qDebug() << "tag non trouvé pour" << _forme << t << pr;
        // _bestOf[t] = pr;
}
