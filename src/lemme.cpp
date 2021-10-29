/*      lemme.cpp
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
 * \file lemme.cpp
 * \brief définit les classes Radical et Lemme
 *
 * Ces classes font partie des couches profondes utilisées par
 * le noyau de lemmatisation, LemCore.
 */

#include "lemme.h"

/////////////
// RADICAL //
/////////////
/**
 * \fn Radical::Radical (QString g, int n, QObject *parent)
 * \brief Créateur de la classe Radical. g est la forme
 *        canonique avec ses quantités, n est le numéro du radical
 */
Radical::Radical(QString g, int n, QObject *parent)
{
    _lemme = qobject_cast<Lemme *>(parent);
    _grq = Ch::communes(g);
    _gr = Ch::atone(g);
    _numero = n;
}

/**
 * \fn QString Radical::gr ()
 * \brief Renvoie la graphie du radical
 *        dépourvue de diacritiques.
 */
QString Radical::gr()
{
    return _gr;
}


/**
 * \fn QString Radical::grq ()
 * \brief Renvoie la graphie du radical
 *        pourvue de ѕes diacritiques.
 */
QString Radical::grq() const
{
    return _grq;
}

/**
 * \fn Lemme* Radical::lemme ()
 * \brief Le lemme auquel appartient le radical.
 */
Lemme *Radical::lemme() { return _lemme; }
/**
 * \fn Modele* Radical::modele ()
 * \brief Le modèle de flexion du radical
 */
Modele *Radical::modele() { return _lemme->modele(); }
/**
 * \fn int Radical::numRad ()
 * \brief Le numéro du radical.
 */
int Radical::numRad() { return _numero; }
///////////
// LEMME //
///////////

/**
 * \fn Lemme::Lemme (QString linea, QObject *parent)
 * \brief Constructeur de la classe Lemme à partir de la
 *        ligne linea. *parent est le noyau de lemmatisation (classe LemCore).
 */
Lemme::Lemme(const QString linea, const int origin, QObject *parent)
{
    // cădo|lego|cĕcĭd|cās|is, ere, cecidi, casum|687
    //   0 | 1  | 2   | 3 |     4                | 5
    _lemCore = qobject_cast<LemCore *>(parent);
    QStringList eclats = linea.split('|');
    QStringList lg = eclats.at(0).split('=');
    _nh = 1;
    _cle = Ch::atone(Ch::deramise(lg.at(0)));
    _grd = oteNh(lg.at(0), _nh);
    if (lg.count() == 1)
        _grq = _grd;
    else
        _grq = lg.at(1);
    // pour l'affichage des dictionnaires, on élimine les doubles de la forme canonique
    _gr = Ch::atone(_grq.section(',',0,0));
    _grModele = eclats.at(1);
    _modele = _lemCore->modele(_grModele);
    _hyphen = "";
    _origin = origin;
    _nbOcc = 1; // Tous les lemmes doivent avoir été rencontrés une fois
    // contrôle de format. la liste doit avoir 6 items
    if (eclats.count() < 6)
    {
        qDebug() << "Ligne mal formée : "<<_gr<<"dernier champ"<<eclats.last()<< linea;
        //return;
    }
    // lecture des radicaux, champs 2 et 3
    for (int i = 2; i < 4; ++i)
        if (!eclats.at(i).isEmpty())
        {
            QStringList lrad = eclats.at(i).split(',');
            foreach (QString rad, lrad)
                _radicaux[i-1].append(new Radical(rad, i-1, this));
        }
    _lemCore->ajRadicaux(this);

    _indMorph = eclats.at(4);
    QRegExp c("cf\\.\\s(\\w+)$");
    int pos = c.indexIn(_indMorph);
    if (pos > -1)
    {
        _renvoi = c.cap(1);
    }
    else
        _renvoi = "";

    _pos.clear();
    if (_indMorph.contains("adj."))
        _pos.append('a');
    if (_indMorph.contains("conj"))
        _pos.append('c');
    if (_indMorph.contains("excl."))
        _pos.append('e');
    if (_indMorph.contains("interj"))
        _pos.append('i');
    if (_indMorph.contains("num."))
        _pos.append('m');
    if (_indMorph.contains("pron."))
        _pos.append('p');
    if (_indMorph.contains("prép"))
        _pos.append('r');
    if (_indMorph.contains("adv"))
        _pos.append('d');
    if (_indMorph.contains(" nom ") || _indMorph.contains("npr."))
        _pos.append('n');
    if (_pos.isEmpty())
    {
        _pos.append(_modele->pos());
        // Je prends le POS du modèle
        if (_pos == "d" && !_renvoi.isEmpty())
            _pos = "";
        // S'il y a un renvoi (cf.) et que le modèle a donné le POS "d" (adverbe),
        // je prendrai le pos du renvoi (les indéclinables ont le POS par défaut "d").
        // Je ne peux pas le faire maintenant !
    }
    // nombre d'occurrences
    _nbOcc = eclats.at(5).toInt();


/* Avec l'internationalisation des morphos, le genre dépend de la langue choisie.
 * Il faut donc le définir à la demande.
    _genre.clear();
    if (_indMorph.contains(" m."))
        _genre.append(" " + _lemCore->genre(0));
//        _genre.append(" masculin"); // Peut-être mieux d'utiliser Flexion::genres[0] ?
    if (_indMorph.contains(" f."))
        _genre.append(" " + _lemCore->genre(1));
//        _genre.append(" féminin");
    if (_indMorph.contains(" n."))
        _genre.append(" " + _lemCore->genre(2));
//        _genre.append(" neutre");
    _genre = _genre.trimmed();
*/
}

/**
 * \fn void Lemme::ajIrreg (Irreg *irr)
 * \brief Ajoute au lemme l'obet irr, qui représente
 *        une forme irrégulière. Lorsque les formes irrégulières
 *        sont trop nombreuses, ou lorsque plusieurs lemmes
 *        ont des formes analogues, mieux vaut ajouter un modèle
 *        dans data/modeles.la.
 */
void Lemme::ajIrreg(Irreg *irr)
{
    _irregs.append(irr);
    // ajouter les numéros de morpho à la liste
    // des morphos irrégulières du lemme :
    if (irr->exclusif()) _morphosIrrExcl.append(irr->morphos());
}

/**
 * \fn void Lemme::ajNombre(int n)
 * \brief Ajoute l'entier n au nombre d'occurrences du lemme.
 *
 *      Un lemme de Collatinus peut être associé à plusieurs lemmes du LASLA.
 *      D'où la somme.
 */
void Lemme::ajNombre(int n)
{
    _nbOcc += n;
    // Un lemme de Collatinus peut être associé à plusieurs lemmes du LASLA.
    // D'où la somme.
}

/**
 * \fn void Lemme::ajRadical (int i, Radical* r)
 * \brief Ajoute le radical r de numéro i à la map des
 *        radicaux du lemme.
 */
void Lemme::ajRadical(int i, Radical *r)
{
    _radicaux[i].append(r);
}

/**
 * \fn void Lemme::ajTrad (QString t, QString l)
 * \brief ajoute la traduction t de langue l à
 *        la map des traductions du lemme.
 */
void Lemme::ajTrad(QString t, QString l)
{
//    if (_traduction.contains(l) && _traduction[l] != "")
//        qDebug() << _grq << t << l << _traduction[l];
    _traduction[l] = t;
}

/**
 * \fn QString Lemme::ambrogio()
 * \brief Renvoie dans une chaîne un résumé
 *        de la traduction du lemme dans toutes les
 *        langues cibles disponibles.
 */
QString Lemme::ambrogio()
{
    QString retour;
    QTextStream ss(&retour);
    ss << "<hr/>" << humain() << "<br/>";
    ss << "<table>";
    foreach (QString lang, _traduction.keys())
    {
        QString trad = _traduction[lang];
        QString langue = _lemCore->cibles()[lang];
        if (!trad.isEmpty())
            ss << "<tr><td>- " << langue << "</td><td>&nbsp;" << trad
               << "</td></tr>\n";
    }
    ss << "</table>";
    return retour;
}

/**
 * \fn QString Lemme::cle ()
 * \brief Renvoie la clé sous laquel le
 *        lemme est enregistré dans le lemmatiseur parent.
 */
QString Lemme::cle()
{
    return _cle;
}

/**
 * \fn QList<int> Lemme::clesR ()
 * \brief Retourne toutes les clés (formes non-ramistes
 *        sans diacritiques) de la map des radicaux du lemme.
 */
QList<int> Lemme::clesR()
{
    return _radicaux.keys();
}

/**
 * \fn bool Lemme::estIrregExcl (int nm)
 * \param nm : numéro de morpho
 * \brief Renvoie vrai si la forme irrégulière
 *        avec le n° nm remplace celle construite
 *        sur le radical , faux si la
 *        forme régulière existe aussi.
 */
bool Lemme::estIrregExcl(int nm)
{
    return _morphosIrrExcl.contains(nm);
}

/**
 * @brief Le genre du lemme
 * @return : le (ou les) genre(s) du mot.
 *
 * Cette routine convertit les indications morphologiques,
 * données dans le fichier lemmes.la,
 * pour exprimer le genre du mot dans la langue courante.
 *
 * Introduite pour assurer l'accord entre un nom et son adjectif.
 *
 */
QString Lemme::genre()
{
    QString _genre;
    if (_indMorph.contains(" m."))
        _genre.append(" " + _lemCore->genre(0));
// J'ai ainsi le genre dans la langue choisie.
    if (_indMorph.contains(" f."))
        _genre.append(" " + _lemCore->genre(1));
//        _genre.append(" féminin");
    if (_indMorph.contains(" n."))
        _genre.append(" " + _lemCore->genre(2));
//        _genre.append(" neutre");
    _genre = _genre.trimmed();
    if (!_renvoi.isEmpty() && _genre.isEmpty())
    {
        Lemme *lr = _lemCore->lemme(_renvoi);
        if (lr != NULL) return lr->genre();
    }
    return _genre;
}

/**
 * \brief Retourne la graphie ramiste du lemme sans diacritiques.
 * \return _gr;
 */
QString Lemme::gr()
{
    return _gr;
}

/**
 * \fn QString Lemme::grq ()
 * \brief Retourne la graphie ramiste du lemme sans diacritiques.
 */
QString Lemme::grq()
{
    return _grq;
}

/**
 * \fn QString Lemme::grModele ()
 * \brief Retourne la graphie du modèle du lemme.
 */
QString Lemme::grModele()
{
    return _grModele;
}

/**
 * \fn QString Lemme::humain (bool html, QString l)
 * \brief Retourne une chaîne donnant le lemme ramiste avec diacritiques,
 *        ses indications morphologiques et sa traduction dans la langue l.
 *        Si html est true, le retour est au format html.
 */
QString Lemme::humain(bool html, QString l, bool nbr)
{
    QString res;
    QString tr;
    if (!_renvoi.isEmpty())
    {
        Lemme *lr = _lemCore->lemme(_renvoi);
        if (lr != 0)
            tr = lr->traduction(l);
        else
            tr = "renvoi non trouvé";
    }
    else
        tr = traduction(l);
    QTextStream flux(&res);
    QString grq = _grq;
    if (grq.contains(","))
    {
        grq.replace(",",", ");
        grq.replace("  "," ");
    }
    if (html)
    {
        if (_nh > 1)
            flux << "<strong>" << grq << "<sup>" << _nh << "</sup></strong>, "
                     << "<em>" << _indMorph << "</em>";
        // Si j'ai un numéro d'homonymie, je le mets en exposant (12 janvier 2021).
        else
            flux << "<strong>" << grq << "</strong>, "
                          << "<em>" << _indMorph << "</em>";
    }
    else
        if (_nh > 1)
            flux << grq << "_" << _nh << ", " << _indMorph;
        else flux << grq << ", " << _indMorph;
    if ((_nbOcc != 1) && nbr)
    {
        if (html)
            flux << " <small>(" << _nbOcc << ")</small>";
        else flux << " (" << _nbOcc << ")";
    }
    flux << " : " << tr;
    return res;
}

QString Lemme::indMorph()
{
    return _indMorph;
}

/**
 * \fn QString Lemme::irreg (int i, bool *excl)
 * \brief Renvoie la forme irrégulière de morpho i. excl devient
 *        true si elle est exclusive, false sinon.
 */
QString Lemme::irreg(int i, bool *excl)
{
    foreach (Irreg *ir, _irregs)
    {
        if (ir->morphos().contains(i))
        {
            *excl = ir->exclusif();
            return ir->grq();
        }
    }
    return "";
}

/**
 * \fn Modele* Lemme::modele ()
 * \brief Renvoie l'objet modèle du lemme.
 */
Modele *Lemme::modele()
{
    return _modele;
}

/**
 * \fn int Lemme::nbOcc()
 * \brief Renvoie le nombre d'occurrences du lemme dans les textes du LASLA.
 */
int Lemme::nbOcc() const
{
    return _nbOcc;
}

/**
 * @brief Lemme::clearOcc
 * Initialise le nombre d'occurrences.
 */
void Lemme::clearOcc()
{
    _nbOcc = 1;
}

/**
 * \fn int Lemme::nh()
 * \brief Renvoie le numéro d'homonymie du lemme.
 */
int Lemme::nh()
{
    return _nh;
}

/**
 * \fn int Lemme::origin()
 * \brief Renvoie l'origine du lemme : 0 pour le lexique de base, 1 pour l'extension.
 */
int Lemme::origin()
{
    return _origin;
}

/**
 * \fn QString Lemme::oteNh (QString g, int &nh)
 * \brief Supprime le dernier caractère de g si c'est
 *        un nombre et revoie le résultat après avoir
 *        donné la valeur de ce nombre à nh.
 */
QString Lemme::oteNh(QString g, int &nh)
{
    int c = g.right(1).toInt();
    if (c > 0)
    {
        nh = c;
        g.chop(1);
    }
    else
        c = 1;
    return g;
}

/**
 * \fn QString Lemme::pos ()
 * \brief Renvoie un caractère représentant la
 *        catégorie (part of speech, pars orationis)
 *        du lemme.
 */
QString Lemme::pos()
{
    if (_pos.isEmpty() && !_renvoi.isEmpty())
    {
        Lemme *lr = _lemCore->lemme(_renvoi);
        if (lr != NULL) return lr->pos();
    }
    return _pos;
}

/**
 * \fn QList<Radical*> Lemme::radical (int r)
 * \brief Renvoie le radical numéro r du lemme.
 */
QList<Radical *> Lemme::radical(int r)
{
    return _radicaux.value(r);
}

/**
 * \fn bool Lemme::renvoi()
 * \brief Renvoie true si le lemme est une forme
 *        alternative renvoyant à une autre entrée
 *        du lexique.
 */
bool Lemme::renvoi() { return _indMorph.contains("cf. "); }
/**
 * \fn QString Lemme::traduction(QString l)
 * \brief Renvoie la traduction du lemme dans la langue
 *        cible l (2 caractères, éventuellement plus
 *        pour donner l'ordre des langues de secours).
 *        J'ai opté pour un format "l1.l2.l3" où
 *        les trois langues sont en 2 caractères.
 */
QString Lemme::traduction(QString l)
{
    if (l.size() == 2)
    {
    if (_traduction.keys().contains(l))
        return _traduction[l];
    else if (_traduction.keys().contains("fr"))
        return _traduction["fr"];
    else return _traduction["en"];
    }
    else if (_traduction.keys().contains(l.mid(0,2)))
        return _traduction[l.mid(0,2)];
    else if (_traduction.keys().contains(l.mid(3,2)))
        return _traduction[l.mid(3,2)];
    else if ((l.size() == 8) && _traduction.keys().contains(l.mid(6,2)))
        return _traduction[l.mid(6,2)];
    return "non traduit / Translation not available.";
}

/**
 * \fn bool Lemme::operator<(Lemme &l)
 * \brief vrai si la fréquence du lemme de gauche est
 *        inférieure à celle de celui de droite.
 *        commenté : vrai si la graphie du lemme de gauche
 *        précède celle de celui de droite dans
 *        l'ordre alphabétique.
 */
bool Lemme::operator<(const Lemme &l) const
{
    //qDebug()<<"operator<"<<_gr;
    return _nbOcc < l.nbOcc();
    //return _gr < l.gr();
}

/**
 * @brief Lemme::setHyphen
 * @param h : indique où se fait la césure.
 * \brief stocke l'information sur la césure étymologique du lemme
 */
void Lemme::setHyphen(QString h)
{
    _hyphen = h;
}

/**
 * @brief Lemme::getHyphen
 * @return la césure étymologique du lemme
 */
QString Lemme::getHyphen()
{
    return _hyphen;
}
