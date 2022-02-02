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

Mot::Mot(QString forme, int rang, bool debVers, Tagueur *parent) : QObject(parent)
{
//    qDebug() << forme;
    _abr = forme.endsWith(".");
    if (_abr)  forme.chop(1);
    _forme = forme;
    _rang = rang;
//    _lemCore = qobject_cast<LemCore *>(parent);
    _tagueur = parent;
    _lemCore = _tagueur->lemCore();
    _debVers = debVers;
    _probas.clear();
    _tagEncl = "";
    _inconnue = false;
    if (forme.isEmpty())
    {
        _tags << "snt";
        _probas["snt"] = 1;
    }
    else if (_abr)
    {
        // C'est un nom à n'importe quel cas !
//        qDebug() << forme << "abr ?";
        QString pseudo = "n%1";
        if (forme.startsWith("K")) pseudo = "n%2";
        // K, Kal ou Kalen seraient plutôt au pluriel
        for (int i = 1; i < 7; i++)
        {
            QString t = pseudo.arg(i)+"1";
            _tags.append(t);
            _morphos.append(_lemCore->morpho(i));
            _lemmes.append("<strong>Abreviation</strong>, <em>" + forme + "</em> : ?");
            _nbOcc.append(_lemCore->tagOcc(t));
            _probas[t] = _lemCore->tagOcc(t);
        }
    }
    else
    {
        _mapLem = _lemCore->lemmatiseM(forme, (rang == 0) || debVers);
        QString enclitique = "";
        // échecs
        if (_mapLem.empty())
        {
            _inconnue = true;
            /* Je ne sais pas encore quoi faire.
             * En décembre 2021, j'ai mis en place une tentative d'identification
             * des formes inconnues qui retourne les radicaux et modèles possibles.
             * Quand on travaille sur tout le texte, on peut privilégier les paires
             * radical+modèle qui expliquent plusieurs formes.
             * Mais, ici, je n'ai que le mot et je traite une phrase à la fois...
             *
             * Faire une pré-lemmatisation dans le Tagueur et introduire une
             * fonction qui retourne le nombre de formes associées au radical+modèle.
             * Ça ne semble pas très rationnel car je fais alors une première lemmatisation
             * dont je ne garde aucune trace.
             * C'est ce que je fais pour le moment (janvier 2022).
             *
             * Une alternative possible serait de construire la liste de mots
             * de l'ensemble du texte en collectant l'informations sur les formes inconnues
             * et en revenant dans un second temps sur les formes inconnues.
             */
            ModLem ml = _lemCore->inconnue(forme);
            // ml n'est jamais vide car j'ai des modèles validés qui ont une désinence vide.
            foreach (Modele *m, ml.keys())
            {
//                int frMax = 0;
                int nb = m->nbr();
//                QMultiMap<int,QString> listeMorph;
                foreach (SLem sl, ml.value(m))
                {
                    QString t = _lemCore->tag(QString(m->pos()),sl.morpho).mid(0,3);
                    // t est le tag de cette solution.
                    long fr = nb * _lemCore->fraction(t);
                    QString morph = sl.grq.section(" ",0,0); // c'est le radical
                    QString toto = sl.grq + " : " + m->gr();
                    // radMod[toto] est une liste de formes qui pourraient comme
                    // radical et modèle "toto".
                    // Plus cette liste est longue, plus il faut la privilégier !
                    // Comment ? Linéaire : 1, 2, 3 etc...
                    // Quadratique : 1, 4, 9 etc...
                    // Affine : 1, 3, 5 etc... ou 1, 4, 7 etc...
                    fr *= (2 * _tagueur->radMod(toto) - 1);
                    if (sl.sufq.isEmpty() || (sl.sufq == "quĕ"))
                    {
                        // La désinence vide convient à toutes les formes.
                        // Il faut la rétrograder !
                        // Il y a deux fois plus de "miles" que de "uita", "templum" ou "lupus".
                        if (fr > 9) fr = fr / 5;
                        else fr = 1;
                        if (sl.sufq == "quĕ") enclitique = "quĕ";
                    }
                    else
                    {
                        // J'ai une désinence dans sl.sufq
                        morph.append(" + " + sl.sufq);
                        if (sl.sufq.endsWith("quĕ"))
                        {
                            morph.chop(3);
                            enclitique = "quĕ";
                        }
                    }
//                    if (fr > frMax) frMax = fr;
                    if (sl.morpho != 414) morph.append(" " + _lemCore->morpho(sl.morpho));
                    _lemmes.append("<strong>unknown</strong>, <em>" + m->gr() + " " +
                                   sl.grq.section(" ",1,1) + "</em> : ?");
                    // C'est l'info que j'ai : le modèle et le n° du radical.
                    // Je la mets au format de Lemme::humain
                    _morphos.append(morph);
                    // C'est le radical suivi de la désinence et de l'analyse associée.
                    _tags.append(t);
                    _nbOcc.append(fr);
                    _probas[t] += fr;
                } // Fin de la boucle sur les analyses possibles.
            } // Fin de la boucle sur les modèles possibles.

            if (!enclitique.isEmpty()) _tagEncl = "ce ";
            // Pour les formes non reconnues, le seul enclitique recherché est "quĕ".
        } // Fin de l'identification des formes inconnues.
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
        // J'ai construit les listes de lemmes, morphos, tags et nombres d'occurrences.

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
        _bestOf[t] = 0.;
        // Je prépare une QMap qui associe les tags à la meilleure probabilité
        // trouvée pour une séquence (établie quand le tag risque de disparaître,
        // c'est à dire quand j'ai traité les deux mots qui suivent celui-ci).
//            qDebug() << t << _probas[t];
        long pr = (_probas[t] * 1024 + total/2) /total;
        if (prMax < pr)
        {
            prMax = pr;
            _maxProb = t;
        }
        if (pr == 0) pr = 1;
        _probas[t] = pr;
    }
//    if ((forme == "Post") || (forme == "post"))
//    qDebug() << forme << _tags.size() << _tags << _probas.keys();
}

QString Mot::choisir(QString t, int np, bool tout, bool html)
{
//    qDebug() << _forme << t << np << tout << _tags.isEmpty() << _tags.size();
    QString choix = "";
    int valeur = -1;
    if (html)
    {
        // Pour une sortie en html, je laisse comme c'était.
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
            QString format = "%1 : %2 ; ";
            for (int i=0; i < _tags.size(); i++)
            {
                QString lg = "<li>" + _lemmes[i] + " — " + _morphos[i] + " (";
                QString lt = _tags[i];
                //            qDebug() << lg << lt;
                if (lt.size() > 2)
                {
                    while (lt.size() > 2)
                    {
                        QString t1 = lt.mid(0,3);
                        lt = lt.mid(4);
                        lg.append(format.arg(t1).arg(_bestOf[t1]));
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
    }
    else
    {
        // Pour une sortie en csv
        int iCh = -1;
        for (int i=0; i < _tags.size(); i++)
            if ((_tags[i].contains(t)) && (valeur < _nbOcc[i]))
            {
                // _tags peut être une liste de tags, alors que t est un tag.
                iCh = i;
                valeur = _nbOcc[i];
                // je retiens l'item qui a le bon tag et qui a le plus d'occurrences.
            }
        QString debut = "\t%1"; // Pour ajouter plus tard, les numéros de mot et de phrase...
        debut.append(_forme + "\t" + t);
        if (iCh == -1) debut.append(" ?"); // Si je n'ai pas retrouvé le tag, je le signale.
        if (t == _maxProb) debut.append("\t");
        else debut.append(" (" + _maxProb + ")\t");
        // Si le tag choisi par le tagueur n'est pas le plus probable
        // (sans tenir compte du contexte), j'indique ce dernier entre parenthèses.
        // Ce début de ligne sera répété autant de fois que nécessaire.

        if (iCh != -1)
        {
            // Le tag existe bien.
            choix = ">" + debut + ligneCSV(_lemmes[iCh]) + _morphos[iCh] + "\t\n";
            // Le ">" dans la première colonne indique que c'est la choix du tagueur.
        }
        if (tout || (iCh == -1))
        {
            // Dans ces deux cas, je donne toutes les solutions possibles.
            // Je voudrais les ordonner...
            QString format = "%1 : %2 ; ";
            QMap<double, QStringList> b;
            foreach (QString tt, _bestOf.keys())
                for (int i=0; i < _tags.size(); i++)
                    if (_tags[i].contains(tt))
                        b[-_bestOf[tt]].append(format.arg(tt).arg(i));
            // J'ai associé à la proba -_bestOf[tt] la liste des indices qui correspondent au tag tt
//            qDebug() << b.size();
            foreach (double p, b.keys())
                if (b[p].size() == 1)
                {
                    // Je n'en ai qu'un.
                    QString bla = b[p].at(0);
                    bla.chop(3);
                    int ii = bla.mid(6).toInt();
                    QString tt = bla.mid(0,3);
                    choix.append(debut + ligneCSV(_lemmes[ii]) + _morphos[ii] + "\t");
                    choix.append(format.arg(tt).arg(_bestOf[tt]));
                    choix.chop(3);
                    choix.append("\n");
                }
                else
                {
                    QMap<int,QStringList> bi;
                    QStringList li = b[p];
                    for (int i = 0; i < li.size(); i++)
                    {
                        QString bla = li.at(i);
                        bla.chop(3);
                        int ii = bla.mid(6).toInt();
                        bi[-_nbOcc[ii]].append(bla);
                    }
                    foreach (int i, bi.keys()) {
                        li = bi[i];
                        for (int i = 0; i < li.size(); i++)
                        {
                            QString bla = li.at(i);
                            int ii = bla.mid(6).toInt();
                            QString tt = bla.mid(0,3);
                            choix.append(debut + ligneCSV(_lemmes[ii]) + _morphos[ii] + "\t");
                            choix.append(format.arg(tt).arg(_bestOf[tt]));
                            choix.chop(3);
                            choix.append("\n");
                        }
                    }
                }
/*          // Ci-dessous la première version sans classement
            for (int i=0; i < _tags.size(); i++)
            {
                choix.append(debut + ligneCSV(_lemmes[i]) + _morphos[i] + "\t");
                QString lt = _tags[i];
                if (lt.size() > 2)
                {
                    while (lt.size() > 2)
                    {
                        QString t1 = lt.mid(0,3);
                        lt = lt.mid(4);
                        choix.append(format.arg(t1).arg(_bestOf[t1]));
                    }
                    choix.chop(3);
                    choix.append("\n");
                }
                else choix.append("?\n");
            }
            */
        }
    }
    choix.chop(1); // Je dois supprimer le dernier \n
    return choix;
}

QString Mot::tagEncl()
{
    return _tagEncl;
}

bool Mot::inconnu()
{
//    return _inconnue;
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

QString Mot::ligneCSV(QString lemmeHumain)
{
    // Il s'agit de convertir la sortie de Lemme::humain en une ligne CSV.
    lemmeHumain.replace("<sup>","_");
    lemmeHumain.remove("</sup>");
    QString lem = Ch::atone(lemmeHumain.mid(0,lemmeHumain.indexOf("</str")));
    lem.remove("<strong>");
    QString indic = lemmeHumain.section(":",0,0);
    indic.remove("<strong>");
    indic.replace("</strong>, <em>",", ");
    if (indic.contains("<small>("))
    {
        indic.replace("</em> <small>(","\t");
        indic.replace(")</small> ","\t");
    }
    else indic.replace("</em> ","\t\t");
    // indic contient le lemme avec quantités et indications.
    // S'il y avait un nombre d'occurrences, il est dans une colonne séparée.
    lem.append("\t");
    lem.append(indic);
    indic = lemmeHumain.section(":",1); // La traduction
    indic.replace("\t", " ");
    indic.append("\t");
    return lem + indic;
}
