/*               lasla.cpp
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

#include "lasla.h"

/**
 * @brief Créateur de la classe Lasla.
 * @param parent : pointeur vers le parent.
 * @param l : pointeur vers un lemmatiseur.
 * @param resDir : chemin complet du répertoire de données.
 *
 * Ce module utilise le lemmatiseur de Collatinus.
 * Il a pour fonction spécifique de donner la lemmatisation
 * et le code en 9 d'une forme.
 * Il ne sert donc que pour les
 * applications avec une coloration LASLA.
 *
 * Si le lemmatiseur est déjà créé par ailleurs,
 * il suffit de passer le pointeur en question.
 * Si le pointeur n'est pas donné,
 * le lemmatiseur sera créé ici.
 * Si l'application envisagée utilise plusieurs modules
 * intermédiaires (tagueur, scandeur...),
 * il vaut mieux créer un seul lemmatiseur commun.
 *
 * Pour déterminer la catégorie LASLA à un lemme,
 * le programme utilise le modèle qui lui est associé.
 * Le fichier CatLASLA donne les correspondances.
 * Si la correspondances n'est pas trouvée, le code
 * en 9 commencera par "k9" en guise de catégorie
 * et sous-catégorie.
 */
Lasla::Lasla(QObject *parent, LemCore *l, QString resDir) : QObject(parent)
{
    if (l==0)
    {
        _lemCore = new LemCore(this, resDir);
        // Je crée le lemmatiseur...
        _lemCore->setExtension(true);
        // ... et charge l'extension du lexique.
        _lemCore->setCible("k9,fr");
    }
    else _lemCore = l;
    if (resDir == "")
        _resDir = qApp->applicationDirPath() + "/data/";
    else if (resDir.endsWith("/")) _resDir = resDir;
    else _resDir = resDir + "/";
    lisCat();
}

/**
 * @brief Lecture des correspondances Collatinus-LASLA
 *
 * Lecture des correspondances entre les modèles de Collatinus
 * et les catégories et sous-catégories du LASLA.
 * Les correspondances sont listées dans le fichier CatLASLA.txt
 * qu'il faut lire et sont rangées dans la QMap Lasla::_catLasla.
 *
 */
void Lasla::lisCat()
{
    QStringList lignes = _lemCore->lignesFichier(_resDir + "CatLASLA.txt");
    foreach (QString lin, lignes)
    {
        if (lin.contains(",")) _catLasla.insert(lin.section(",",0,0),lin.section(",",1,1));
    }
}

/**
 * @brief Lemmatisation avec le code LASLA
 * @param m : une forme
 * @return le résultat de la lemmatisation avec le code en 9.
 *
 * Le résultat se présente comme une chaine de type CSV.
 * Chaque ligne est une des possibilité d'analyse et
 * est composée de quatre champs séparés par une virgule.
 * Le premier champ donne la forme et le deuxième la clef du lemme,
 * tous les deux sans quantité.
 * Le 3e champ reste vide, il est fait pour contenir l'indice
 * d'homonymie du LASLA. Ici, l'éventuel indice est collé
 * au lemme, sous la forme d'un chiffre.
 * Le lemme de Collatinus est en minuscule (sauf l'initiale),
 * ce qui le distingue des lemmes du LASLA, toujours en majuscules.
 * Le 4e champ done l'analyse sous la forme d'un code en 9.
 */
QString Lasla::k9(QString m)
{
//    qDebug() << m;
    QStringList res;
    QString cibAct = _lemCore->cible();
    _lemCore->setCible("k9,fr");
    MapLem mm = _lemCore->lemmatiseM(m);
    if (mm.isEmpty()) return "\n";
    // Il faut répondre quelque chose, sinon j'attends 30 secondes !
    foreach (Lemme *l, mm.keys())
    {
        QString clef = l->cle() + ", ,";
        foreach (SLem s, mm.value(l))
        {
            QString code9 = _lemCore->morpho(s.morpho);
            QString forme = Ch::atone(s.grq);
            if (!s.sufq.isEmpty()) forme += "<" + Ch::atone(s.sufq) +">,";
            else forme += ",";
            if (_catLasla.contains(l->modele()->gr())) code9.replace("k9",_catLasla[l->modele()->gr()]);
//            qDebug() << clef << s.morpho << code9 << _catLasla[l->modele()->gr()];
            res << forme + clef + code9;
        }
    }

    _lemCore->setCible(cibAct);
    return res.join("\n");
}


