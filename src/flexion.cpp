/*      flexion.cpp
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

#include "flexion.h"

#include <QDebug>

/**
 * \file flexion.cpp
 * \brief module de flexion des formes latines
 */

/**
 * \fn Flexion::Flexion (QObject *parent);
 * \brief Constructeur de la classe Flexion.
 * \param       *parent est un pointeur sur le lemCore.
 */
Flexion::Flexion(QObject *parent) : QObject(parent)
{
    _lemCore = qobject_cast<LemCore *>(parent);
}

const QString Flexion::entete =
    "<table border=1 cellspacing=\"0\" cellpadding=\"5\">";
const QString Flexion::lina = "<tr><td>";
const QString Flexion::linb = "</td><td>";
const QString Flexion::linc = "</td></tr>";
const QString Flexion::queue = "</table>";

/*const QStringList Flexion::cas = QStringList() << "nominatif"
                                               << "vocatif"
                                               << "accusatif"
                                               << "génitif"
                                               << "datif"
                                               << "ablatif";

const QStringList Flexion::genres = QStringList() << "masculin"
                                                  << "féminin"
                                                  << "neutre";
const QStringList Flexion::nombres = QStringList() << "singulier"
                                                   << "pluriel";

const QStringList Flexion::temps = QStringList() << "présent"
                                                 << "imparfait"
                                                 << "futur"
                                                 << "parfait"
                                                 << "plus-que-parfait"
                                                 << "futur antérieur";
*/
QString Flexion::entreParenth(QString e)
{
    return QString("(%1)").arg(e);
}

/**
 * \fn QString Flexion::forme (int n, bool label)
 * \brief Renvoie entre virgules les formes dont
 *        morphologie occupe de rang n dans la liste
 *        des morphologies du lemmatiseur. Si label est
 *        true, le retour est précédé de la morphologie.
 */
QString Flexion::forme(int n, bool label)
{
    if (_lemme == 0) return "lemme absent";
    Modele *m = _lemme->modele();
    QList<Desinence *> ld = m->desinences(n);
    if (ld.empty()) return "-";
    QStringList lres;
    if (label) lres.append(_lemCore->morpho(n));
    bool excl = false;
    QString firr = _lemme->irreg(n, &excl);
    if (!firr.isEmpty()) lres.append(firr);
    if (!excl)
    {
        foreach (Desinence *d, ld)
        {
            QString grqd = d->grq();
            // désinence trop rare, non affichée :
            if (d->rarete() <= omis) continue;
            int nr = d->numRad();
            QList<Radical *> lr = _lemme->radical(nr);
            foreach (Radical *r, lr)
            {
                QString grqr = r->grq();
                if (d->rarete() <= parenth)
                    lres.append(entreParenth(grqr + grqd));
                else lres.append(grqr + grqd);
            }
        }
    }
    lres.removeDuplicates();
    return lres.join(", ");
}

/**
 * \fn QString Flexion::gras (QString g)
 * \brief Utilitaire renvoyant g encadré
 *        des balises html <strong> et </strong>.
 */
QString Flexion::gras(QString g)
{
    return QString("<strong>%1</strong>").arg(g);
}

/**
 * \fn void Flexion::setLemme (Lemme *l)
 * \brief Attribue le lemme l à l'objet Flexion.
 *        Aucun tableau ne peut être calculé
 *        avant que cette fonction ait été appelée.
 */
void Flexion::setLemme(Lemme *l)
{
    _lemme = l;
}

/**
 * \fn QString Flexion::tableau (Lemme *l)
 * \brief Renvoie le tableau de flexion de l.
 *        Cette fonction se contente d'appeler
 *        la fonction spécialisée correspondant
 *        à la catégorie du lemme.
 */
QString Flexion::tableau(Lemme *l)
{
    if (l == 0) return "lemme absent\n";
    setLemme(l);
    QStringList ret;
    QString pos = l->pos();
    if (pos.contains('n')) ret.append(tabNom());
    if (pos.contains('p')) ret.append(tabPron());
    if (pos.contains('a')) ret.append(tabAdj());
    if (pos.contains('d')) ret.append(tabAdv());
    if (pos.contains('v')) ret.append(tabV());
    if (ret.empty()) return l->humain(false,_lemCore->cible());
    ret.removeDuplicates();
    return ret.join("");
}

/**
 * \fn QString Flexion::tableaux (MapLem *ml)
 * \brief Calcule les tableaux de chaque lemme
 *        de la MapLem ml (cf. lemCore.h),
 *        et renvoie leur concaténation.
 */
QString Flexion::tableaux(MapLem *ml)
{
    QString ret;
    QTextStream fl(&ret);
    menuLem.clear();
    QTextStream flm(&menuLem);
    flm << "<h4>";
    foreach (Lemme *l, ml->keys())
    {
        // numéro d'homonymie
        flm << "<a href=\"#" << l->cle() << "\">" << l->grq() << "</a> "
            << l->humain(false,_lemCore->cible()) << "<br/>";
    }
    flm << "</h4>";
    fl << menuLem;
    foreach (Lemme *l, ml->keys())
        fl << "<hr/>" << tableau(l) << menuLem;
    return ret;
}

/**
 * \fn QString Flexion::tabNom()
 * \brief Fonction spécialisée dans les noms.
 */
QString Flexion::tabNom()
{
    QString ret;
    QTextStream fl(&ret);
    fl << "<hr/><a name=\"" << _lemme->cle() << "\"></a>";
    fl << entete;
    fl << lina << _lemCore->motsClefs(0) << linb << _lemCore->nombre(0) << linb << _lemCore->nombre(1) << linc;
    for (int i = 1; i < 7; ++i)
        fl << lina << _lemCore->cas(i - 1) << linb << forme(i)
            << linb << forme(i + 6) << linc;
    fl << queue;
    return ret;
}

/**
 * \fn QString Flexion::tabPron()
 * \brief Fonction spécialisée dans les pronoms.
 */
QString Flexion::tabPron()
{
    QString ret;
    QTextStream fl(&ret);
    fl << "<hr/><a name=\"" << _lemme->cle() << "\"></a>";
    fl << _lemCore->nombre(0) << "<p>";
    fl << entete;
    fl << lina << _lemCore->motsClefs(0) << linb
        << _lemCore->genre(0) << linb
        << _lemCore->genre(1) << linb
        << _lemCore->genre(2) << linc;
    for (int i = 13; i < 19; ++i)
        fl << lina << _lemCore->cas((i - 13) % 6) << linb
            << forme(i) << linb
            << forme(i + 12) << linb
            << forme(i + 24) << linc;
    fl << queue;
    fl << "</p>" << _lemCore->nombre(1) << "<p>";
    fl << entete;
    fl << lina << _lemCore->motsClefs(0) << linb
        << _lemCore->genre(0) << linb
        << _lemCore->genre(1) << linb
        << _lemCore->genre(2) << linc;
    for (int i = 19; i < 25; ++i)
        fl << lina << _lemCore->cas((i - 19) % 6) << linb
            << forme(i) << linb
            << forme(i + 12) << linb
            << forme(i + 24) << linc;
    fl << queue << "</p>";
    return ret;
}

/**
 * \fn QString Flexion::tabAdj()
 * \brief Fonction spécialisée dans les adjectifs.
 */
QString Flexion::tabAdj()
{
    QString ret;
    QTextStream fl(&ret);
    fl << "<a name=\"" << _lemme->cle() << "\"></a>";
    fl << "<p>" << _lemme->grq() << "</p>";
    fl << "<p>" << _lemCore->genre(0);
    fl << entete;
//    fl << "<tr><td colspan=4>" << _lemCore->nombre(0) << "</td></tr>";
    fl << "<tr><td colspan=4>" << _lemCore->nombre(0) << "</td></tr>";
    fl << lina << _lemCore->motsClefs(0) << linb << _lemCore->morpho(414) << linb <<
          _lemCore->morpho(411) << linb << _lemCore->morpho(412)
       << linc;
    for (int i = 13; i < 19; ++i)
        fl << lina << _lemCore->cas(i - 13) << linb << forme(i) << linb << forme(i + 36)
           << linb << forme(i + 72) << linc;
    fl << "<tr><td colspan=4>" << _lemCore->nombre(1) << "</td></tr>";
    fl << lina << _lemCore->motsClefs(0) << linb << _lemCore->morpho(414) << linb <<
          _lemCore->morpho(411) << linb << _lemCore->morpho(412)
       << linc;
    for (int i = 19; i < 25; ++i)
        fl << lina << _lemCore->cas(i - 19) << linb << forme(i) << linb << forme(i + 36)
           << linb << forme(i + 72) << linc;
    fl << queue << "</p>";

    fl << "<p>" << _lemCore->genre(1);
    fl << entete;
//    fl << "<tr><td colspan=4>" << _lemCore->nombre(0) << "</td></tr>";
    fl << "<tr><td colspan=4>" << _lemCore->nombre(0) << "</td></tr>";
    fl << lina << _lemCore->motsClefs(0) << linb << _lemCore->morpho(414) << linb <<
          _lemCore->morpho(411) << linb << _lemCore->morpho(412)
       << linc;
    for (int i = 25; i < 31; ++i)
        fl << lina << _lemCore->cas(i - 25) << linb << forme(i) << linb << forme(i + 36)
           << linb << forme(i + 72) << linc;
    fl << "<tr><td colspan=4>" << _lemCore->nombre(1) << "</td></tr>";
    fl << lina << _lemCore->motsClefs(0) << linb << _lemCore->morpho(414) << linb <<
          _lemCore->morpho(411) << linb << _lemCore->morpho(412)
       << linc;
    for (int i = 31; i < 37; ++i)
        fl << lina << _lemCore->cas(i - 31) << linb << forme(i) << linb << forme(i + 36)
           << linb << forme(i + 72) << linc;
    fl << queue << "</p>";

    fl << "<p>" << _lemCore->genre(2);
    fl << entete;
    fl << "<tr><td colspan=4>" << _lemCore->nombre(0) << "</td></tr>";
    fl << lina << _lemCore->motsClefs(0) << linb << _lemCore->morpho(414) << linb <<
          _lemCore->morpho(411) << linb << _lemCore->morpho(412)
       << linc;
    for (int i = 37; i < 43; ++i)
        fl << lina << _lemCore->cas(i - 37) << linb << forme(i) << linb << forme(i + 36)
           << linb << forme(i + 72) << linc;
    fl << "<tr><td colspan=4>" << _lemCore->nombre(1) << "</td></tr>";
    fl << lina << _lemCore->motsClefs(0) << linb << _lemCore->morpho(414) << linb <<
          _lemCore->morpho(411) << linb << _lemCore->morpho(412)
       << linc;
    for (int i = 43; i < 49; ++i)
        fl << lina << _lemCore->cas(i - 43) << linb << forme(i) << linb << forme(i + 36)
           << linb << forme(i + 72) << linc;
    fl << queue << "</p>";

    return ret;
}

/**
 * \fn QString Flexion::tabAdv()
 * \brief Fonction spécialisée dans les adverbes.
 */
QString Flexion::tabAdv()
{
    QString ret;
    QTextStream fl(&ret);
    fl << "<hr/><a name=\"" << _lemme->cle() << "\"></a>";
    fl << entete;
    fl << lina << _lemCore->morpho(414) << linb <<
          _lemCore->morpho(411) << linb << _lemCore->morpho(412)
       << linc;
    fl << lina << forme(414) << linb << forme(411) << linb << forme(412)
       << linc;
    fl << queue;
    return ret;
}

/**
 * \fn QString Flexion::tabV()
 * \brief Fonction spécialisée dans les verbes.
 */
QString Flexion::tabV()
{
    // menu
    QString menu;
    // Ce menu est destiné à faciliter la navigation dans les
    // tableaux de conjugaison, qui sont un peu long.
    // Toutefois, il y a un problème si la forme demandée
    // a plusieurs verbes à son actif, par exemple lego :
    // les <a name=...> sont tous en double et les liens <a href=...>
    // mènent toujours au premier verbe...
    // Il faudrait dédoublonner les noms en ajoutant la cle.
    // Par exemple "<a href=\"#actif\">"<< _lemCore->voix(0) <<"</a><br/>"
    // deviendrait "<a href=\"#actif" << _lemme->cle() << "\">"<< _lemCore->voix(0) <<"</a><br/>"
    // Mais il faudrait faire de même pour tous les name !!!
    QTextStream(&menu)
        << "<a name=\"" << _lemme->cle() << "\"></a><br/>"
        << "<a href=\"#actif\">"<< _lemCore->voix(0) <<"</a><br/>"
        << "<a href=\"#indactif\">"<< _lemCore->modes(0) <<"</a>&nbsp;"
        << "<a href=\"#subactif\">"<< _lemCore->modes(1) <<"</a>&nbsp;"
        << "<a href=\"#impactif\">"<< _lemCore->modes(2) <<" &amp; "<< _lemCore->modes(3) <<"</a>&nbsp;"
        << "<a href=\"#partpres\">"<< _lemCore->modes(4) <<" "<< _lemCore->temps(0) <<"</a>&nbsp;"
        << "<a href=\"#partfut\">"<< _lemCore->modes(4) <<" "<< _lemCore->temps(2) <<"</a><br/>"
        << "<a href=\"#indpass\">"<< _lemCore->voix(1) <<"</a><br/>&nbsp;"
        << "<a href=\"#indpass\">"<< _lemCore->modes(0) <<"</a>&nbsp;"
        << "<a href=\"#subpass\">"<< _lemCore->modes(1) <<"</a>&nbsp;"
        << "<a href=\"#imppass\">"<< _lemCore->modes(2) <<" &amp; "<< _lemCore->modes(3) <<"</a>&nbsp;"
        << "<a href=\"#ppp\">"<< _lemCore->modes(4) <<" "<< _lemCore->temps(3) <<"</a>&nbsp;"
        << "<a href=\"#adjv\">"<< _lemCore->modes(5) <<"</a><br/>";

    QString ret;
    QTextStream fl(&ret);
    fl << "<a name=\"actif\"></a>";
    fl << "<div>" << _lemme->humain(false,_lemCore->cible()) << "</div>";
    fl << "<a name=\"indactif\"></a>" << menu << "<h4>"<< _lemCore->voix(0) <<"</h4><p>"
       << _lemCore->modes(0) << " infectum</p>";
    fl << entete;
    fl << lina << _lemCore->temps(0) << linb << _lemCore->temps(1) << linb << _lemCore->temps(2) << linc;
    for (int i = 121; i < 127; ++i)
        fl << lina << forme(i) << linb << forme(i + 6) << linb << forme(i + 12)
           << linc;
    fl << queue << "<p>";

    fl << _lemCore->modes(0) << " perfectum</p>";
    fl << entete;
    fl << lina << _lemCore->temps(3) << linb << _lemCore->temps(4) << linb << _lemCore->temps(5) << linc;
    for (int i = 139; i < 145; ++i)
        fl << lina << forme(i) << linb << forme(i + 6) << linb << forme(i + 12)
           << linc;
    fl << queue;

    fl << "<a name=\"subactif\"></a>";
    fl << menu;
    fl << "<p>" << _lemCore->modes(1) << "</p>";
    fl << entete;
    fl << lina << _lemCore->temps(0) << linb << _lemCore->temps(1) << linb << _lemCore->temps(3) << linb
       << _lemCore->temps(4) << linc;
    for (int i = 157; i < 163; ++i)
        fl << lina << forme(i) << linb << forme(i + 6) << linb << forme(i + 12)
           << linb << forme(i + 18) << linc;
    fl << queue;

    fl << QString(
        "<a name=\"impactif\"></a>"
        "<p>");
    fl << _lemCore->modes(2) << "</p>";
    fl << entete;
    fl << lina << _lemCore->motsClefs(1) << linb << _lemCore->nombre(0) << linb << _lemCore->nombre(1)
       << linc;
    fl << lina << _lemCore->motsClefs(2) << linb << forme(181) << linb << forme(182)
       << linc;
    fl << lina << _lemCore->motsClefs(3) << linb << forme(183) << linb << forme(185)
       << linc;
    fl << lina << _lemCore->motsClefs(4) << linb << forme(184) << linb << forme(186)
       << linc;
    fl << queue;

    fl << "<p>" << _lemCore->morpho(187) << " : " << forme(187)
       << "<br/>" << _lemCore->morpho(188) <<
          //"infinifif futur : "<<forme(415)<<"</br>"
          " : "
       << forme(188) << "</p>";

    fl << "<br/><a name=\"partpres\"></a>";
    fl << menu;
    fl << "<p>"<< _lemCore->modes(4) <<" "<< _lemCore->temps(0) <<"</p>";
    fl << entete;
    fl << lina << _lemCore->motsClefs(0) << linb << _lemCore->genre(0) << linb << _lemCore->genre(1) << linb
       << _lemCore->genre(2) << linc;
    fl << "<tr><td colspan=4>" << _lemCore->nombre(0) << "</td></tr>";
    for (int i = 189; i < 195; ++i)
        fl << lina << _lemCore->cas((i - 189) % 6) << linb << forme(i) << linb
           << forme(i + 12) << linb << forme(i + 24) << linc;
    fl << "<tr><td colspan=4>" << _lemCore->nombre(1) << "</td></tr>";
    for (int i = 195; i < 201; ++i)
        fl << lina << _lemCore->cas((i - 189) % 6) << linb << forme(i) << linb
           << forme(i + 12) << linb << forme(i + 24) << linc;
    fl << queue;

    fl << "<a name=\"partfut\"></a>";
    fl << menu;
    fl << "<p>"<< _lemCore->modes(4) <<" "<< _lemCore->temps(2) <<"</p>";
    fl << entete;
    fl << lina << _lemCore->motsClefs(0) << linb << _lemCore->genre(0) << linb << _lemCore->genre(1) << linb
       << _lemCore->genre(2) << linc;
    fl << "<tr><td colspan=4>" << _lemCore->nombre(0) << "</td></tr>";
    for (int i = 225; i < 231; ++i)
        fl << lina << _lemCore->cas((i - 225) % 6) << linb << forme(i) << linb
           << forme(i + 12) << linb << forme(i + 24) << linc;
    fl << "<tr><td colspan=4>" << _lemCore->nombre(1) << "</td></tr>";
    for (int i = 231; i < 237; ++i)
        fl << lina << _lemCore->cas((i - 225) % 6) << linb << forme(i) << linb
           << forme(i + 12) << linb << forme(i + 24) << linc;
    fl << queue;

    fl << "<br/>";
    fl << entete << lina << _lemCore->morpho(261) << linb << forme(261)
       << linc << lina << _lemCore->morpho(262) << linb
       << forme(262) << linc << lina << _lemCore->morpho(263) << linb
       << forme(263) << linc << lina << _lemCore->morpho(264) << linb
       << forme(264) << linc << lina << _lemCore->morpho(265) << linb << forme(265)
       << linc << lina << _lemCore->morpho(266) << linb << forme(266) << linc << queue;

    fl << menu;
    fl << "<a name=\"indpass\"></a><h4>"<< _lemCore->voix(1) <<"</h4>";
    fl << "<p>"<< _lemCore->modes(0) <<"</p>";
    fl << entete;
    fl << lina << _lemCore->temps(0) << linb << _lemCore->temps(1) << linb << _lemCore->temps(2) << linc;
    for (int i = 267; i < 273; ++i)
        fl << lina << forme(i) << linb << forme(i + 6) << linb << forme(i + 12)
           << linc;
    fl << queue;

    // Formes composées
    QString ppp = forme(303) + "/ă&nbsp;";
    QString fc = "<p>Formes compos\u00E9es</p>";
    QString susp = "&nbsp;\u22EE ";
    fl << fc;
    fl << entete;
    fl << lina << _lemCore->temps(3) << linb << _lemCore->temps(4) << linb
       << _lemCore->temps(5) << linc;
    fl << lina << ppp << "<i>sum</i>" << linb << ppp << "<i>eram</i>" << linb << ppp << "<i>ero</i>"
           << linc;
    fl << lina << ppp << susp << linb << ppp << susp << linb << ppp << susp
           << linc;
    fl << queue;

    fl << "<a name=\"subpass\"></a>";
    fl << menu;
    fl << "<p>"<< _lemCore->modes(1) <<"</p>";
    fl << entete;
//    fl << lina << _lemCore->temps(0) << linb << _lemCore->temps(1) << linb << _lemCore->temps(3) << linc;
    fl << lina << _lemCore->temps(0) << linb << _lemCore->temps(1) << linc;
    for (int i = 285; i < 291; ++i)
        fl << lina << forme(i) << linb << forme(i + 6) << linc;
//        fl << lina << forme(i) << linb << forme(i + 6) << linb << forme(i + 12)
//           << linc;
    fl << queue;

    // Formes composées
    fl << fc;
    fl << entete;
    fl << lina << _lemCore->temps(3) << linb << _lemCore->temps(4) << linc;
    fl << lina << ppp << "<i>sim</i>" << linb << ppp << "<i>essem</i>" << linc;
    fl << lina << ppp << susp << linb << ppp << susp << linc;
    fl << queue;

    // impératif passif
    fl << QString(
        "<a name=\"imppass\"></a>"
        "<p>");
    fl << _lemCore->modes(2) << "</p>";
    fl << entete;
    fl << lina << _lemCore->motsClefs(1) << linb << _lemCore->nombre(0) << linb << _lemCore->nombre(1)
       << linc;
    fl << lina << _lemCore->motsClefs(2) << linb << forme(297) << linb << forme(298) << linc;
    fl << lina << _lemCore->motsClefs(3) << linb << forme(299) << linb << "-" << linc;
    fl << lina << _lemCore->motsClefs(4) << linb << forme(300) << linb << forme(301) << linc;
    fl << queue;

    fl << "<p>" << _lemCore->morpho(302) << " : " << forme(302) << "</p><br/>";

    fl << "<a name=\"ppp\"></a>";
    fl << menu;
    fl << "<p>"<< _lemCore->modes(4) <<" "<< _lemCore->temps(3) <<"</p>";
    fl << entete;
    fl << lina << _lemCore->motsClefs(0) << linb << _lemCore->genre(0) << linb << _lemCore->genre(1) << linb
       << _lemCore->genre(2) << linc;
    fl << "<tr><td colspan=4>" << _lemCore->nombre(0) << "</td></tr>";
    for (int i = 303; i < 309; ++i)
        // TODO ajouter des colspan pour le nombre
        fl << lina << _lemCore->cas((i - 303) % 6) << linb << forme(i) << linb
           << forme(i + 12) << linb << forme(i + 24) << linc;
    fl << "<tr><td colspan=4>" << _lemCore->nombre(1) << "</td></tr>";
    for (int i = 309; i < 315; ++i)
        // TODO ajouter des colspan pour le nombre
        fl << lina << _lemCore->cas((i - 303) % 6) << linb << forme(i) << linb
           << forme(i + 12) << linb << forme(i + 24) << linc;
    fl << queue;

    fl << "<a name=\"adjv\"></a>";
    fl << menu;
    fl << "<p>"<< _lemCore->modes(5) <<"</p>";
    fl << entete;
    fl << lina << _lemCore->motsClefs(0) << linb << _lemCore->genre(0) << linb << _lemCore->genre(1) << linb
       << _lemCore->genre(2) << linc;
    fl << "<tr><td colspan=4>" << _lemCore->nombre(0) << "</td></tr>";
    for (int i = 339; i < 345; ++i)
        // TODO ajouter des colspan pour le nombre
        fl << lina << _lemCore->cas((i - 339) % 6) << linb << forme(i) << linb
           << forme(i + 12) << linb << forme(i + 24) << linc;
    fl << "<tr><td colspan=4>" << _lemCore->nombre(1) << "</td></tr>";
    for (int i = 345; i < 351; ++i)
        // TODO ajouter des colspan pour le nombre
        fl << lina << _lemCore->cas((i - 339) % 6) << linb << forme(i) << linb
           << forme(i + 12) << linb << forme(i + 24) << linc;
    fl << queue;

    return ret;
}
