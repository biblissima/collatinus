/*          modele.cpp
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
 * \file modele.cpp
 * \brief définit les classes Desinence et Modele
 *
 * Ces classes font partie des couches profondes utilisées par
 * le noyau de lemmatisation, LemCore.
 */

#include "modele.h"

///////////////
// DESINENCE //
///////////////

/**
 * \fn Desinence::Desinence (QString d, int morph, int nr, Modele *parent)
 * \brief Constructeur de la classe Desinence.
 * \param d est la graphie avec quantités
 * \param morph est le numéro de morphologie (dans la liste de la classe
 * LemCore)
 * \param nr est le numéro de radical accepté par la désinence
 * \param parent est
 *        un pointeur sur le modèle qui utilise cette
 *        désinence.
 *
 * Un paradigme, Modele, est associé à un (ou plusieurs) radical(aux), Radical,
 * et une collection de désinences.
 * Chaque Desinence est donnée par sa graphie (avec quantité)
 * mais doit aussi contenir des informations cruciales :
 *  * l'analyse morphologique à laquelle elle est associée
 *  * le numéro du radical auquel elle peut se coller
 *  * le modèle.
 */
Desinence::Desinence(QString d, int morph, int nr, Modele *parent)
{
    // der, le dernier caractère de d, s'il est un nombre, donne le degré de
    // rareté de la désinence, qui est 10 par défaut.
    int der = -1;
    if (!d.isEmpty())
        der = d.at(d.length()-1).digitValue();
    if (der > 0)
    {
        _rarete = der;
        d.chop(1);
    }
    else _rarete = 10;
    // '-' est la désinence zéro
    if (d == "-") d = "";
    _grq = d;
    _gr = Ch::atone(_grq);
    _morpho = morph;
    _numR = nr;
    _modele = parent;
//    _modele = qobject_cast<Modele *>(parent);
}

/**
 * \fn QString Desinence::gr ()
 * \brief Graphie de la désinence, ramiste et sans quantités.
 */
QString Desinence::gr() { return _gr; }

/**
 * \fn QString Desinence::grq ()
 * \brief Graphie ramiste avec quantités.
 */
QString Desinence::grq() { return _grq; }


/**
 * \fn Modele* Desinence::modele ()
 * \brief Modèle de la désinence.
 */
Modele *Desinence::modele() { return _modele; }

/**
 * \fn int Desinence::morphoNum ()
 * \brief Numéro de morpho de la désinence.
 */
int Desinence::morphoNum()
{
    return _morpho;
}

/**
 * \fn int Desinence::numRad ()
 * \brief Numéro de radical de la désinence.
 */

int Desinence::numRad()
{
    return _numR;
}

/**
 * @brief accesseur de la rareté
 * @return la valeur de Desinence::_rarete
 *
 * La rareté est un paramètre qui permet de séparer l'utilisation
 * d'une désinence en analyse et en flexion.
 * En effet, certains paradigmes admettent des désinences rares ou archaïques
 * qu'il faut reconnaître quand on les rencontre dans un texte.
 * En revanche, les tableaux de flexion sont plutôt destinés aux débutants.
 * Il ne serait donc pas opportun d'encombrer leur mémoire de formes
 * qu'ils ont peu de chance de rencontrer (dans l'immédiat).
 * Dans le module de Flexion, deux paramètres, @c OMIS et @c PARENTH,
 * décident de l'affichage ou pas de la forme construite sur ces
 * désinences rares. Les trois possibilités sont :
 *  * l'omission
 *  * l'affichage entre parenthèses
 *  * l'affichage normal.
 *
 * @note Le nom semble mal choisi car les désinences usuelles
 * sont associées à la valeur @c 10. Les plus rares ont @c 0.
 */
int Desinence::rarete()
{
    return _rarete;
}

/**
 * \brief Attribue un modèle à la désinence.
 *
 * @deprecated Semble inutilisé.
 * La valeur de Desinence::_modele est définie lors de la création.
 */
void Desinence::setModele(Modele *m)
{
    _modele = m;
}

////////////
// MODELE //
////////////

/**
 * \fn Modele::Modele (QStringList ll, LemCore *parent)
 * \brief Constructeur de la classe modèle.
 * \param ll : liste de chaines de caractères
 * \param parent : pointeur vers le noyau de lemmatisation, LemCore
 *
 * Chaque item
 *        de la liste \a ll est constitué de champs séparés par
 *        le caractère <tt>':'</tt>. Le premier champ est un mot clé
 * (voir Modele::cles).
 * Pour le format du
 *        fichier <tt>data/modeles.la</tt>, consulter la documentation
 *        utilisateur.
 */
Modele::Modele(QStringList ll, LemCore *parent) : QObject(parent)
{
    _lemCore = parent;
//    _lemCore = qobject_cast<LemCore *>(parent);
    _pere = 0;
    _pos = '\0';
    QMultiMap<QString, int> msuff;
    QRegExp re("[:;]([\\w]*)\\+{0,1}(\\$\\w+)");
    foreach (QString l, ll)
    {
        // remplacement des variables par leur valeur
        while (re.indexIn(l) > -1)
        {
            QString v = re.cap(2);
            QString var = _lemCore->variable(v);
            QString pre = re.cap(1);
            if (!pre.isEmpty()) var.replace(";", ";" + pre);
            l.replace(v, var);
        }
        QStringList eclats = l.simplified().split(":");
        // modele pere des desْ+ R   abs
        //  0      1    2   3   4   5
        int p = cles.indexOf(eclats.first());
        switch (p)
        {
            case 0:  // modèle
                _gr = eclats.at(1);
                break;
            case 1:  // père
                _pere = parent->modele(eclats.at(1));
                break;
            case 2:  // des+: désinences s'ajoutant à celles du père
            case 3:  // des: désinences écrasant celles du père
            {
                QList<int> li = listeI(eclats.at(1));
                int r = eclats.at(2).toInt();
                QStringList ld = eclats.at(3).split(';');
                for (int i = 0; i < li.count(); ++i)
                {
                    QStringList ldd;
                    if (i < ld.count())
                        ldd = ld.at(i).split(',');
                    else
                        ldd = ld.last().split(',');
                    foreach (QString g, ldd)
                    {
                        Desinence *nd = new Desinence(g, li.at(i), r, this);
                        _desinences.insert(nd->morphoNum(), nd);
                        _lemCore->ajDesinence(nd);
                    }
                }
                // si des+, aller chercher les autres désinences chez le père :
                if (p == 3 && _pere != 0)
                {
                    foreach (int i, li)
                    {
                        QList<Desinence *> ldp = _pere->desinences(i);
                        foreach (Desinence *dp, ldp)
                        {
                            // cloner la désinece
                            Desinence *dh = clone(dp);
                            _desinences.insert(i, dh);
                            _lemCore->ajDesinence(dh);
                        }
                    }
                }
                break;
            }
            case 4:  // R:n: radical n
            {
                int nr = eclats.at(1).toInt();
                _genRadicaux[nr] = eclats.at(2);
                break;
            }
            case 8: // abs+
                _absents.append(listeI(eclats.at(1)));
                break;
            case 5: // abs
                _absents = listeI(eclats.at(1));
                break;
            case 6:  // suffixes suf:<intervalle>:valeur
            {
                QList<int> lsuf = listeI(eclats.at(1));
                QString gr = eclats.at(2);  // TODO verif : bien formée ?
                foreach (int m, lsuf)
                    msuff.insert(gr, m);
                break;
            }
            case 7:  // sufd: les désinences du père, mais suffixées
            {
                if (_pere != 0)
                {
                    _suf = eclats.at(1);
                    QList<Desinence *> ld = _pere->desinences();
                    foreach (Desinence *d, ld)
                    {
                        if (_absents.contains(d->morphoNum()))
                            // morpho absente chez le descendant
                            continue;
                        QString nd = d->grq();
                        Ch::allonge (&nd);
                        Desinence *dsuf = new Desinence
                            (nd+_suf, d->morphoNum(), d->numRad(), this);
                        _desinences.insert(dsuf->morphoNum(), dsuf);
                        _lemCore->ajDesinence(dsuf);
                    }
                }
                break;
            }
            case 9: // POS
            {
                _pos = eclats.at(1).at(0);
                break;
            }
            default:
                qDebug() << "Modèle, erreur" << l;
        }

    }  // fin de l'interprétation des lignes

    // père
    if (_pere != 0)
    {
        // héritage du pos
        if (_pos == '\0') _pos = _pere->pos();
        // héritage des désinences
        foreach (int m, _pere->morphos())
        {
            if (deja(m)) continue;
            QList<Desinence *> ld = _pere->desinences(m);
            foreach (Desinence *d, ld)
            {
                if (_absents.contains(
                        d->morphoNum()))  // morpho absente chez le descendant
                    continue;
                Desinence *dh = clone(d);
                _desinences.insert(dh->morphoNum(), dh);
                _lemCore->ajDesinence(dh);
            }
        }
        // héritage des radicaux
        foreach (Desinence *d, _desinences)
        {
            if (!_genRadicaux.contains(d->numRad()))
            {
                QString nr = _pere->genRadical(d->numRad());
                _genRadicaux.insert(d->numRad(), nr);
            }
        }
        // héritage des absents
        _absents = _pere->absents();
    }
    // génération des désinences suffixées
    QList<Desinence *> ldsuf;
    QStringList clefsSuff = msuff.keys();
    clefsSuff.removeDuplicates();
    foreach (QString suff, clefsSuff)
    {
        foreach (Desinence *d, _desinences)
        {
            if (msuff.values(suff).contains(d->morphoNum()))
            {
                QString gq = d->grq();
                if (gq == "-") gq.clear();
                gq.append(suff);
                Desinence *dsuf =
                    new Desinence(gq, d->morphoNum(), d->numRad(), this);
                ldsuf.insert(dsuf->morphoNum(), dsuf);
            }
        }
    }
    foreach (Desinence *dsuf, ldsuf)
    {
        _desinences.insert(dsuf->morphoNum(), dsuf);
        _lemCore->ajDesinence(dsuf);
    }
}

/**
 * \fn bool Modele::absent (int a)
 * \brief Renvoie true si la morpho de rang a
 *        n'existe pas dans le modèle.
 *
 * Certains substantifs n'ont pas de singulier,
 *        certains verbes n'ont pas de passif.
 * Pour afficher correctement la flexion, il faut savoir
 * quelles analyses morphologiques ne sont pas utilisées
 * pour ce modèle.
 */
bool Modele::absent(int a) { return _absents.contains(a); }

/**
 * \fn QList<int> Modele::absents ()
 * \brief Retourne la liste des numéros des morphos absentes.
 */
QList<int> Modele::absents() { return _absents; }

/**
 * \fn QList<int> Modele::clesR ()
 * \brief Liste des numéros de radicaux utilisés, et
 *        rangés dans la map _genRadicaux.
 */
QList<int> Modele::clesR() { return _genRadicaux.keys(); }

/**
 * \fn Desinence* Modele::clone (Desinence *d)
 * \brief Crée une Désinence copiée sur la désinence d.
 */
Desinence *Modele::clone(Desinence *d)
{
    return new Desinence(d->grq()+_suf, d->morphoNum(), d->numRad(), this);
}

/**
 * \fn bool Modele::deja (int m)
 * \brief Renvoie true si le modèle a déjà une désinence avec la morpho de rang m.
 *
 * Cette fonction permet de savoir s'il faut aller chercher la désinence
 *        de morpho m chez le modèle père.
 */
bool Modele::deja(int m) { return _desinences.contains(m); }

/**
 * \fn QList<Desinence*> Modele::desinences (int d)
 * \brief Renvoie la liste des désinence de morpho d du modèle.
 */
QList<Desinence *> Modele::desinences(int d) { return _desinences.values(d); }

/**
 * \fn QList<Desinence*> Modele::desinences ()
 * \brief Renvoie toutes les désinences du modèle.
 */
QList<Desinence *> Modele::desinences() { return _desinences.values(); }

/**
 * \fn bool Modele::estUn (QString m)
 * \brief Renvoie true si le modèle se nomme m, ou si
 *        l'un de ses ancêtres se nomme m
 */
bool Modele::estUn(QString m)
{
    if (_gr == m) return true;
    if (_pere == 0) return false;
    return _pere->estUn(m);
}

/**
 * \fn QString Modele::gr ()
 * \brief Nom du modèle.
 */
QString Modele::gr() { return _gr; }

QStringList const Modele::cles = QStringList() << "modele"  // 0
                                               << "pere"    // 1
                                               << "des"     // 2
                                               << "des+"    // 3
                                               << "R"       // 4
                                               << "abs"     // 5
                                               << "suf"     // 6
                                               << "sufd"    // 7
                                               << "abs+"    // 8
                                               << "pos";    // 9

/**
 * \fn QString Modele::genRadical (int r)
 * \brief générateur d'un radical
 * \param r est le numéro du radical.
 * \return une chaîne permettant de calculer un radical à partir
 *        de la forme canonique d'un lemme.
 */
QString Modele::genRadical(int r)
{
    return _genRadicaux[r];
}

/**
 * \fn QList<int> Modele::listeI (QString l)
 * \brief conversion d'une chaine de caractère en liste d'entiers
 * \param l : la chaine initiale
 * \return la liste des entiers contenus dans la chaine
 *
 * Fonction importante permettant de renvoyer
 *        une liste d'entiers à partir d'une chaîne @a l.
 *        La chaîne est une liste de sections séparées
 *        par des virgules. Une section peut être soit
 *        un entier, soit un intervalle d'entiers. On
 *        donne alors les limites inférieure et supérieure
 *        de l'intervale, séparées par le caractère '-'.
 * Les limites sont incluses.
 *
 * Nombreux exemples d'intervalles dans le fichier
 *        <tt>data/modeles.la</tt>.
 */
QList<int> Modele::listeI(QString l)
{
    QList<int> result;
    QStringList lvirg = l.split(',');
    foreach (QString virg, lvirg)
    {
        if (virg.contains('-'))
        {
            int deb = virg.section('-', 0, 0).toInt();
            int fin = virg.section('-', 1, 1).toInt();
            for (int i = deb; i <= fin; ++i) result.append(i);
        }
        else
        {
            result.append(virg.toInt());
        }
    }
    return result;
}

/**
 * \fn QList<int> Modele::morphos ()
 * \brief Liste des numéros des désinences définies par le modèle.
 */
QList<int> Modele::morphos() { return _desinences.keys(); }

/**
 * \fn QChar Modele::pos()
 * \brief Retourne la catégorie du modèle, en utilisant
 *        les ancêtres du modèle.
 */
QChar Modele::pos()
{
    if (_pos == '\0') return 'd';
    return _pos;
    /*
    if (estUn("uita") || estUn("lupus") || estUn("miles") || estUn("manus") ||
        estUn("res") || estUn("perseus"))
        return 'n';
    if (estUn("doctus") || estUn("fortis")) return 'a';
    if (estUn("amo") || estUn("imitor")) return 'v';
    return 'd';
    */
}
