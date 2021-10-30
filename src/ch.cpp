/*      ch.cpp
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
 * \file ch.cpp
 * \brief constantes et utilitaires de traitement
 *        des chaînes de caractères
 *
 */

#include "ch.h"

#include <QDebug>

/**
 * \fn Ch::ajoute (QString mot, QStringList liste)
 * \brief Ajoute mot au début de chaque item de liste.
 */
QStringList Ch::ajoute(QString mot, QStringList liste)
{
    QStringList res;
    foreach (QString s, liste)
        res.append(s.prepend(mot));
    return res;
}

/**
 * \fn Ch::allonge(QString *f)
 * \brief modifie f pour que sa dernière voyelle
 *        devienne longue.
 */
void Ch::allonge(QString *f)
{
    if (f->isEmpty()) return;
    int taille = f->size();
    // Je sais que le morceau à attacher commence par une consonne.
    if ((taille > 1) && consonnes.contains(f->at(taille - 1)) &&
            (f->at(taille - 2) == QChar(774))) // combining breve
        f->remove(taille - 2, 1);
    else if (consonnes.contains(f->at(taille - 1)) &&
        !QString("\u0101e \u0101u \u0113u \u014de \u0113i")
             .contains(f->mid(taille - 3, 2).toLower()))
    { // Il ne faudrait pas allonger le 2e voyelles des diphtongues.
        // Ajout de la diphtongue "ei" que l'on trouve dans "dein".
        f->replace(QRegExp("[a\u0103]([" + consonnes + "])$"), "\u0101\\1");
        f->replace(QRegExp("[e\u0115]([" + consonnes + "])$"), "\u0113\\1");
        f->replace(QRegExp("[i\u012d]([" + consonnes + "])$"), "\u012b\\1");
        f->replace(QRegExp("[o\u014F]([" + consonnes + "])$"), "\u014d\\1");
        f->replace(QRegExp("[u\u016d]([" + consonnes + "])$"), "\u016b\\1");
        f->replace(QRegExp("[y\u0233]([" + consonnes + "])$"), "\u045e\\1");
        f->replace(QRegExp("[A\u0102]([" + consonnes + "])$"), "\u0100\\1");
        f->replace(QRegExp("[E\u0114]([" + consonnes + "])$"), "\u0112\\1");
        f->replace(QRegExp("[I\u012c]([" + consonnes + "])$"), "\u012a\\1");
        f->replace(QRegExp("[O\u014e]([" + consonnes + "])$"), "\u014c\\1");
        f->replace(QRegExp("[U\u016c]([" + consonnes + "])$"), "\u016a\\1");
        f->replace(QRegExp("[Y\u0232]([" + consonnes + "])$"), "\u040e\\1");
    }
}

/**
 * \fn Ch::atone(QString a, bool bdc)
 * \brief supprime tous les diacritiques de la chaîne a
 * \param a : le mot à traiter
 * \param bdc : si bdc est à false (par défaut), les diacritiques des majuscules
 *        sont également supprimés.
 * \return le mot sans diacritique.
 */
QString Ch::atone(QString a, bool bdc)
{
    // Supprimer le combining breve à la fin du mot
    // if (a.endsWith(0x0306)) a.chop(1);
    // minuscules
    a.replace(0x0101, 'a');
    a.replace(0x0103, 'a');  // ā ă
    a.replace(0x0113, 'e');
    a.replace(0x0115, 'e');  // ē ĕ
    a.replace(0x012b, 'i');
    a.replace(0x012d, 'i');  // ī ĭ
    a.replace(0x014d, 'o');
    a.replace(0x014f, 'o');  // ō ŏ
    a.replace(0x016b, 'u');
    a.replace(0x016d, 'u');  // ū ŭ
    a.replace(0x0233, 'y');
    a.replace(0x045e, 'y');  // ȳ ў
    if (!bdc)
    {
        // majuscule
        a.replace(0x0100, 'A');
        a.replace(0x0102, 'A');  // Ā Ă
        a.replace(0x0112, 'E');
        a.replace(0x0114, 'E');  // Ē Ĕ
        a.replace(0x012a, 'I');
        a.replace(0x012c, 'I');  // Ī Ĭ
        a.replace(0x014c, 'O');
        a.replace(0x014e, 'O');  // Ō Ŏ
        a.replace(0x016a, 'U');
        a.replace(0x016c, 'U');  // Ū Ŭ
        a.replace(0x0232, 'Y');
        a.replace(0x040e, 'Y');  // Ȳ Ў
    }
    a.replace(0x0131, 'i');
    a.replace(0x1ee5, 'u');
    // combining breve
    a.remove(0x0306);  //ō̆ etc.
    return a;
}

/**
 * @brief retourne la voyelle brève
 * @param c : une voyelle sans quantité en minuscule
 * @return la fonction retourne la voyelle brève correspondant à c.
 * Si c n'est pas une voyelle (aeiouy), la fonction retourne c.
 */
QChar Ch::breve(QChar c)
{
    switch (c.unicode()) {
    case 97: // 'a'
        return voyelles[1];
        break;
    case 101:
        return voyelles[3];
        break;
    case 105:
        return voyelles[5];
        break;
    case 111:
        return voyelles[7];
        break;
    case 117:
        return voyelles[9];
        break;
    case 121:
        return voyelles[11];
        break;
    default:
        return c;
        break;
    }
}
/**
 * \fn Ch::communes(QString g)
 * \brief note comme communes toutes les voyelles qui ne portent pas de quantité.
 */
QString Ch::communes(QString g)
{
    bool maj = g[0].isUpper();
    g = g.toLower();
    if (g.contains("a") || g.contains("e") || g.contains("i") || g.contains("o") || g.contains("u") || g.contains("y"))
    {
        g.replace("a","ā̆");
        g.replace(QRegExp("([^āăō])e"),"\\1ē̆"); // e qui n'est pas précédé de āăō
        g.replace(QRegExp("^e"),"ē̆"); // e en début de mot
        g.replace(QRegExp("([^ē])i"),"\\1ī̆"); // i nu, sauf s'il est dans une diphtongue "ei" (deinde).
        g.replace("o","ō̆");
        g.replace(QRegExp("([^āēq])u"),"\\1ū̆");
        g.replace(QRegExp("^u"),"ū̆");
        g.replace(QRegExp("^y"),"ȳ̆");
        g.replace(QRegExp("([^ā])y"),"\\1ȳ̆");
    }
    if (maj) g[0] = g[0].toUpper();
    return g;
}

/**
 * \fn Ch::deQuant(QString *c)
 * \brief supprime la quantité de la voyelle finale
 * \param c : le mot
 * \return le mot sans la quantité de la voyelle finale
 *
 * Cette fonction est utilisée en cas d'élision.
 * Elle supprime la quantité de la voyelle finale de la chaine c
 * lorsque cette voyelle est en fin de mot ou suivie d'un "m".
 */
void Ch::deQuant(QString *c)
{
    if (c->endsWith("\u0306"))
        c->chop(1);  // Supprimer le combining breve à la fin du mot
    c->replace(QRegExp("[\u0101\u0103](m?)$"), "a\\1");  // ā ă
    c->replace(QRegExp("[\u0113\u0115](m?)$"), "e\\1");
    c->replace(QRegExp("[\u012b\u012d](m?)$"), "i\\1");
    c->replace(QRegExp("[\u014d\u014f](m?)$"), "o\\1");
    c->replace(QRegExp("[\u016b\u016d](m?)$"), "u\\1");
    c->replace(QRegExp("[\u0232\u0233](m?)$"), "y\\1");
}

/**
 * \brief supprime tous les accents d'un texte
 * \param c : une chaine (mot ou texte)
 * \return la chaine sans accent
 *
 * Cette fonction est utilisée pour enlever les accents, les diacritiques
 * (acute, macron, breve, tilde etc...) ou les cédilles d'un texte.
 * En effet, certains textes (récupérés sur internet) contiennent
 * des accents ou autres signes diacritiques qui peuvent nuire
 * à la lemmatisation.
 * Cette fonction utilise la décomposition normalisée des caractères Unicode
 * et supprime les signes combinants qui ont été séparés du caractère de base.
 */
QString Ch::deAccent(QString c)
{
    c = c.normalized(QString::NormalizationForm_D, QChar::currentUnicodeVersion());
    c.remove("\u0300"); // Accent grave
    c.remove("\u0301"); // Accent aigu
    c.remove("\u0302"); // Accent circonflexe
    c.remove("\u0303"); // Tilde
    c.remove("\u0304"); // macron
    c.remove("\u0306"); // breve
    c.remove("\u0308"); // Trémas
    c.remove("\u0327"); // Cédille
    c.remove("\u0328"); // Ogonek
    return c;
}

/**
 * \fn QString Ch::deramise(QString r)
 * \brief retourne une graphie non-ramiste
 *        de r, càd dont tous les j deviennent i,
 *        et tous les v deviennent u.
 */
QString Ch::deramise(QString r)
{
    r.replace('J', 'I');
    r.replace('j', 'i');
    r.replace('v', 'u');
    r.replace("æ", "ae");
    r.replace("Æ", "Ae");
    r.replace("œ", "oe");
    r.replace("Œ", "Oe");
    r.replace("ȩ", "ae");
    r.replace("ę", "ae");
    r.replace(0x1ee5, 'u');  // ụ le u muet de suavis, suadeo, etc...
    r.replace ('V', 'U');
    return r;
}

/**
 * \fn Ch::elide(QString *mp)
 * \brief met entre crochets la dernière syllabe de mp.
 */
void Ch::elide(QString *mp)
{
    //"Tāntāene"
    //bool debog = (*mp == "Tāntāene");
    //if (debog) qDebug() << "tantaene" << *mp;
    int taille = mp->size();
    if ((taille > 1) && ((mp->endsWith('m') || mp->endsWith("\u0101e")) ||
                         mp->endsWith("\u0306")) &&
        voyelles.contains(mp->at(taille - 2)))
    {
        //if (debog) qDebug() << "cond1";
        deQuant(mp);
        mp->insert(taille - 2, '[');
        mp->append(']');
    }
    else if (voyelles.contains(mp->at(taille - 1)) && *mp != "\u014d")
    {
        //if (debog) qDebug() << "cond2";
        deQuant(mp);
        mp->insert(taille - 1, '[');
        mp->append(']');
    }
    //if (debog) qDebug() << *mp;
}

/**
 * @brief sépare la chaine d'entrée en lettres et nombre
 * @param s : la chaine à découper
 * @param ch : une chaine passée par adresse qui reçoit les lettres précédant un chiffre
 * @param n : un entier passée par adresse qui reçoit la valeur numérique finale
 *
 * @deprecated Cette fonction semble inusitée...
 */
void Ch::genStrNum(const QString s, QString *ch, int *n)
{
    ch->clear();
    *n = 0;
    for (int i = 0; i < s.length(); ++i)
        if (!s.at(i).isNumber())
            ch->append(s.at(i));
        else
        {
            *n = s.mid(i).toInt();
            break;
        }
}

/**
 * \fn Ch::sort_i(const QString &a, const QString &b)
 * \brief compare a et b sans tenir compte des diacritiques ni de la casse.
 * \return true si a < b.
 */
bool Ch::sort_i(const QString &a, const QString &b)
{
    return QString::compare(atone(a), atone(b), Qt::CaseInsensitive) < 0;
}

/**
 * \fn Ch::inv_sort_i(const QString &a, const QString &b)
 * \brief compare a et b sans tenir compte des diacritiques ni de la casse.
 * \return true si a > b.
 * Utilisée pour ranger les mots en fontions des fréquences descendantes
 */
bool Ch::inv_sort_i(const QString &a, const QString &b)
{
    return QString::compare(atone(a), atone(b), Qt::CaseInsensitive) > 0;
}

/**
 * \fn Ch::versPC(QString k)
 * \brief Comme Ch::versPedeCerto, mais ici le mot n'a pas été trouvé.
 *        Les voyelles ne sont pas marquées sauf par position...
 */
QString Ch::versPC(QString k)
{
    k = k.toLower();
    if (k.contains("[")) k = k.section("[", 0, 0) + "`";
    k.replace("qu", "");
    k.replace("gu", "");
    k.replace("āe", "+");
    k.replace("ōe", "+");
    k.replace("āu", "+");
    k.replace("ēu", "+");
    k.replace("ēi", "+");
    // Incomplet : manque la recherche de doubles consonnes ou voyelles
    k.replace("a", "*");
    k.replace("e", "*");
    k.replace("i", "*");
    k.replace("o", "*");
    k.replace("u", "*");
    k.replace("y", "*");
    return versPedeCerto(k);
}

/**
 * \fn Ch::versPedeCerto(QString k)
 * \brief remplace les longues de k par +, les brèves par - et les communes par *
 *
 */
QString Ch::versPedeCerto(QString k)
{
    // Je remplace les longues par +, les brèves par - et les communes par *
    // minuscules
    k.replace(0x0101, '+');
    k.replace(0x0103, '-');  // ā ă
    k.replace(0x0113, '+');
    k.replace(0x0115, '-');  // ē ĕ
    k.replace(0x012b, '+');
    k.replace(0x012d, '-');  // ī ĭ
    k.replace(0x014d, '+');
    k.replace(0x014f, '-');  // ō ŏ
    k.replace(0x016b, '+');
    k.replace(0x016d, '-');  // ū ŭ
    k.replace(0x0233, '+');
    k.replace(0x045e, '-');  // ȳ ў
    // majuscule
    k.replace(0x0100, '+');
    k.replace(0x0102, '-');  // Ā Ă
    k.replace(0x0112, '+');
    k.replace(0x0114, '-');  // Ē Ĕ
    k.replace(0x012a, '+');
    k.replace(0x012c, '-');  // Ī Ĭ
    k.replace(0x014c, '+');
    k.replace(0x014e, '-');  // Ō Ŏ
    k.replace(0x016a, '+');
    k.replace(0x016c, '-');  // Ū Ŭ
    k.replace(0x0232, '+');
    k.replace(0x040e, '-');  // Ȳ Ў
    // "+" + combining breve = voyelle commune
    k.replace("+\u0306", "*");
    if (k.contains("[")) k = k.section("[", 0, 0) + "`";
    // Je garde une trace de l'élision (pour le rythme)
    k.remove(0x1ee5);  // suppression du u-exponctué.
    k.remove(reLettres);
    return k;
}

/**
 * @brief transforme les quantités en + et - en conservant les voyelles
 * @param k : la chaine à transformer
 * @return la chaine transformée
 *
 * Cette fonction transforme les quantités indiquées avec macron ou breve
 * par les signes + et -, tout en conservant la voyelle.
 * Une "*" pour les voyelles communes.
 * Elle est utilisée pour accentuer un mot, dans Ch::ajoutSuff.
 *
 */
QString Ch::transforme(QString k)
{
    k.replace("āe", "æ+");
    k.replace("ōe", "œ+");
    k.replace("ăe", "æ-");
    k.replace("Āe", "Æ+");
    k.replace("Ōe", "Œ+");
    k.replace("ēi", "\u0117+"); // La diphtongue "ei" dans dein et deinde.
    // Je remplace les longues par +, les brèves par - et les communes par *
    // minuscules
    k.replace(0x0101, "a+");
    k.replace(0x0103, "a-");  // ā ă
    k.replace(0x0113, "e+");
    k.replace(0x0115, "e-");  // ē ĕ
    k.replace(0x012b, "i+");
    k.replace(0x012d, "i-");  // ī ĭ
    k.replace(0x014d, "o+");
    k.replace(0x014f, "o-");  // ō ŏ
    k.replace(0x016b, "u+");
    k.replace(0x016d, "u-");  // ū ŭ
    k.replace(0x0233, "y+");
    k.replace(0x045e, "y-");  // ȳ ў
    // majuscule
    k.replace(0x0100, "A+");
    k.replace(0x0102, "A-");  // Ā Ă
    k.replace(0x0112, "E+");
    k.replace(0x0114, "E-");  // Ē Ĕ
    k.replace(0x012a, "I+");
    k.replace(0x012c, "I-");  // Ī Ĭ
    k.replace(0x014c, "O+");
    k.replace(0x014e, "O-");  // Ō Ŏ
    k.replace(0x016a, "U+");
    k.replace(0x016c, "U-");  // Ū Ŭ
    k.replace(0x0232, "Y+");
    k.replace(0x040e, "Y-");  // Ȳ Ў
    // "+" + combining breve = voyelle commune
    k.replace("+\u0306", "*");
    k.replace(0x1ee5, "u");  // suppression du u-exponctué.
    return k;
}

/**
 * @brief accentue la voyelle
 * @param l : une voyelle ou une diphtongue
 * @return la voyelle ou la diphtongue accentuée
 *
 * Cette fonction place un accent aigu sur la voyelle donnée en argument.
 * Si l'argument n'est pas une voyelle, il ne se passe rien.
 * Cette fonction est utilisée pour accentuer un mot, dans Ch::ajoutSuff.
 */
QString Ch::accentue(QString l)
{
    if ((l == "œ") || (l == "Œ")) return l + "\u0301";
    if (l == "æ") return "ǽ";
    if (l == "Æ") return "Ǽ";
    int a = l[0].unicode();
    switch (a)
    {
        case 97:
            return "á";
        case 101:
            return "é";
        case 105:
            return "í";
        case 111:
            return "ó";
        case 117:
            return "ú";
        case 121:
            return "ý";
        case 65:
            return "Á";
        case 69:
            return "É";
        case 73:
            return "Í";
        case 79:
            return "Ó";
        case 85:
            return "Ú";
        case 89:
            return "Ý";
            break;
        default:
            return l;
            break;
    }
}

/**
 * @brief ajoute le suffixe à la forme
 * @param fq : la forme avec ses quantités
 * @param suffixe : le suffixe avec ses quantités
 * @param l_etym : l'information étymologique
 * @param accent : un entier qui dit s'il faut scander ou accentuer
 * @return la forme avec son suffixe, scandée ou accentuée
 *
 * Lors de la lemmatisation, un éventuel enclitique a été séparée de la forme.
 * Il faut donc le recoller à la fin.
 * Toutefois, si la forme se termine par une consonne, on se retrouve avec
 * deux consonnes de suite et la syllabe devient nécessairement longue.
 * @note uirumque devient transitoirement vĭrŭm + quĕ, ce qui donne vĭrūmquĕ.
 *
 * Si le paramètre _accent_ est nul, on procède à une scansion.
 * Sinon, l'ensemble sera accentué et _accent_ spécifie le comportement
 * dans le cas des voyelles communes (voir Scandeur::scandeTxt pour les détails).
 *
 * Ce même paramètre _accent_ permet de séparer les syllabes.
 * Il faut alors faire attention aux éventuels préfixes
 * qui conduisent à des exceptions dans le découpage.
 * Ces exceptions sont indiquées dans *l_etym*.
 * voir LemCore::lireHyphen pour quelques détails.
 *
 * @note L'enclitique attire l'accent.
 * Le mot _rosaque_ sera paroxyton, _rosáque_ même au nominatif (où le _a_ est bref).
 *
 * @bug Il semblerait qu'il y ait un bug lorsque deux "suffixes" ont été ajoutés.
 * Un exemple serait modoquest. À corriger !
 *
 */
QString Ch::ajoutSuff(QString fq, QString suffixe, QString l_etym, int accent)
{
    bool illius = false;
    bool cesure = false;
    bool sansAccent = false;
    if (accent > 7)
    {
        illius = true;
        accent -= 8;
    }
    if (accent > 3)
    {
        cesure = true;
        accent -= 4;
    }
    if (accent > 0)
    {
        QString signes = "+-*";
        fq = transforme(fq);
        int l = fq.count('+') + fq.count('-') +
                fq.count('*');  // nombre de syllabes.
        int i = fq.size() - 1;
        if (suffixe.isEmpty() || (suffixe == "st"))
        {
            // Sans suffixe, l'accent est sur l'avant-dernière voyelle si elle
            // n'est pas brève
            if (l > 2)
            {
                if (illius && fq.endsWith("i*u-s"))
                {
                    fq.chop(5);
                    fq.append("í*u-s");
                }
                else
                {
                    while (!signes.contains(fq[i])) i -= 1;
                    i -= 1;
                    while (!signes.contains(fq[i])) i -= 1;
                    sansAccent = (fq[i] == '*') && (accent == 3);
                    // La pénultième est commune et je la considère comme ambiguë =
                    // pas d'accent.
                    if ((fq[i] == '-') || ((fq[i] == '*') && (accent == 2)))
                    {
                        // Remonter à l'antépénultième
                        i -= 1;
                        while (!signes.contains(fq[i])) i -= 1;
                    }
                    if (!sansAccent)
                    {
                        if (i > 1)
                            fq = fq.mid(0, i - 1) + accentue(fq.mid(i - 1, 1)) +
                                    fq.mid(i);
                        else
                            fq = accentue(fq.mid(i - 1, 1)) + fq.mid(i);
                    }
                }
            }
            fq += suffixe;
        }
        else
        {
            // Avec suffixe, l'accent est sur la dernière (avant collage)
            if (l > 1)
            {
                while (!signes.contains(fq[i])) i -= 1;
                if (i > 1)
                    fq = fq.mid(0, i - 1) + accentue(fq.mid(i - 1, 1)) +
                         fq.mid(i);
                else
                    fq = accentue(fq.mid(i - 1, 1)) + fq.mid(i);
            }
            fq = fq + suffixe;
            fq.replace("ĕ","e-"); // pour modoquest
//            fq[fq.size() - 2] = 'e';  // ôte le e-bref.
            l += 1;
        }
        // L'entier i pointe sur la longueur de la voyelle accentuée, sauf si
        // j'ai ajouté un \u0301.
        // Si je veux marquer la syllabe accentuée en gras, c'est ici !
        if ((l > 1) && cesure)
        {
            // Il y a au moins deux syllabes que je veux séparer
            int j = fq.size() - 1;
            while (!signes.contains((fq[j])) && (j > 0)) j -= 1;
            int k = j;
            j -= 2;
            while (!signes.contains((fq[j])) && (j > 0)) j -= 1;
            while (j > 0)
            {
                // la césure doit tomber quelque part entre j et k
                if (k == j + 2)
                    fq.insert(j, separSyll);  // Il n'y a que la voyelle (et une
                                              // quantité)
                else
                {
                    int nbCons = 0;
                    for (int n = j + 1; n < k - 1; n++)
                        if (consonnes.contains(fq[n]) || (fq[n] == 'h'))
                            nbCons += 1;
                    // J'ai le nombre de consonnes
                    if (nbCons == 0)
                        fq.insert(k - 1, separSyll);
                    else
                    {
                        while (!consonnes.contains(fq[k]) && (fq[k] != 'h'))
                            k -= 1;
                        if (nbCons == 1)
                            fq.insert(k, separSyll);
                        else
                        {
                            // c'est plus compliqué car j'ai au moins deux
                            // consonnes...
                            bool remonte =
                                ((fq[k] == 'l') && (fq[k - 1] != 'l') && (fq[k - 1] != 'r'));
                            remonte = remonte ||
                                      ((fq[k] == 'r') && (fq[k - 1] != 'r') && (fq[k - 1] != 'l'));
                            remonte = remonte || (fq[k] == 'h');
                            if (remonte) k -= 1;
                            remonte =
                                ((fq[k] == 'c') && (fq[k - 1] == 's') &&
                                 (fq[k + 1] != 'a') && (fq[k + 1] != 'o') &&
                                 (fq[k + 1] != 'u') && (fq[k + 1] != 'h'));
                            remonte = remonte ||
                                      ((fq[k] == 'p') && (fq[k - 1] == 's'));
                            // remonte = remonte || ((fq[k]=='t') &&
                            // (fq[k-1]=='s'));
                            remonte = remonte ||
                                      ((fq[k] == 'n') && (fq[k - 1] == 'g'));
                            if (remonte) k -= 1;
                            fq.insert(k, separSyll);
                        }
                    }
                }
                k = j;
                j -= 2;
                while ((j > 0) && !signes.contains((fq[j]))) j -= 1;
            }
            // J'ai placé les césures en suivant les règles établies.
            // Mais je peux avoir des césures étymologiques qui vont à
            // l'encontre des règles "normales".
            //            QString l_etym = e->getHyphen();
            if (!l_etym.isEmpty()) foreach (QString etym, l_etym.split(','))
                {
                    QString fq1 = fq;
                    // Je vais parcourir le mot pour vérifier que ça colle
                    int i = 0;
                    int j = 0;
                    int changement = 0;
                    bool OK = true;
                    while ((i < etym.size()) && (j < fq.size()) && OK)
                    {
                        if ((etym[i] == fq[j]) ||
                            (fq.mid(j, 1) == accentue(etym.mid(i, 1))))
//                            ((etym[i] == '-') && (fq[j] == separSyll)))
                        {
                            // Les lettres ou les césures correspondent
                            i += 1;
                            j += 1;
                        }
                        else if (signes.contains(fq[j]) || (fq[j] == 0x0301))
                            j += 1;  // C'est une quantité
                        else if ((etym[i] != separSyll) && (fq[j] != separSyll))
                            OK = false;  // Les lettres ne correspondent pas.
                        else
                        {
                            // la césure est mal placée.
                            if (etym[i] == separSyll)
                            {
                                fq.insert(j, separSyll);
                                changement += 1;
                                j += 1;
                                i += 1;
                            }
                            else
                            {
                                fq.remove(j, 1);
                                changement -= 1;
                            }
                        }
                    }
                    if (changement == 1)
                    {
                        // La césure étymologique est tombé avant la césure
                        // normale...
                        while (fq[j] != separSyll) j += 1;
                        fq.remove(j, 1);
                    }
                    if (!OK)
                        fq = fq1;  // etym ne correspondait pas aux premières
                                   // lettres de fq.
                }
        }
        fq.remove("+");
        fq.remove("-");
        fq.remove("*");
        return fq;
    }
    if (suffixe.isEmpty()) return fq;
    // les suffixes possibles sont que, ne et ve :
    // tous commencent par une consonne
    allonge(&fq);
    if (suffixe == "st")
    {
        // Si fq se termine par une voyelle, allonge ne fait rien.
        fq += "s"; // J'ajoute donc le s et je recommence.
        allonge(&fq);
        return fq + "t";
    }
    suffixe.replace("ĕst","ēst"); // Pour des formes comme modoquest
    return fq + suffixe;
}
