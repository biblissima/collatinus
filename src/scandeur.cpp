/*   scandeur.cpp
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

#include "scandeur.h"

/**
 * \file scandeur.cpp
 * \brief module de scansion des formes latines
 */

/**
 * @brief
 * \if French
 * Créateur de la classe pour scander des textes.
 * \else
 * Constructs a Scandeur to scan the texts.
 * \endif
 * @param parent :
 * \if French
 * Un pointeur vers l'objet qui crée cette classe.
 * \else
 * The owner of this class.
 * \endif
 * @param l :
 * \if French
 * Un pointeur vers un moteur de lemmatisation (LemCore).
 * \else
 * A pointeur to the lemmatization core (LemCore).
 * \endif
 * @param resDir :
 * \if French
 * Le chemin complet du dossier contenant les fichiers de donnée.
 * \else
 * The absolute path for the resources directory.
 * \endif
 *
 * \if French
 * La classe Scandeur propose les outils nécessaires pour scander un texte.
 * Elle utilise le moteur de lemmatisation de Collatinus (LemCore)
 * qui lui est passé en paramètre. Si ce moteur ne lui est pas donné,
 * elle le crée ici.
 * Si l'application envisagée utilise plusieurs modules
 * _intermédiaires_ (Tagueur, Lemmatiseur...),
 * il vaut mieux créer un seul moteur commun.
 *
 * Le paramètre optionnel @a resDir donne
 * le chemin complet du dossier contenant les fichiers de donnée.
 * Par défaut, il s'agit du fichier "data" placé à côté de l'exécutable.
 * \else
 * The Scandeur class is meant to scan texts.
 * As part of Collatinus, it uses its lemmatization core (LemCore).
 * If this core has been created elsewhere, a pointer is given to this creator.
 * If the pointer @a l is empty, then the lemmatization core is created here.
 * When the developped application uses different modules as this one
 * (e.g. Lemmatiseur, Tagueur), it is recommended to create a shared core.
 *
 * The optional parameter @a resDir gives the absolute path to the
 * resources directory. If empty, the resources are assumed to be
 * in a folder "data" placed in the same dir as the exe.
 * \endif
 *
 * \attention
 * \if French
 * Il s'agit ici de _scander_ un texte :
 * on marquera donc la _quantité des syllabes_ et pas la _longueur des voyelles_.
 * Pour les syllabes _ouvertes_, les deux se confondent, mais pas pour
 * les syllabes _fermées_ (qui se terminent avec une consonne).
 * On peut avoir une voyelle brève dans une syllabe longue !
 * \else
 * This module _scans_ a text:
 * it marks the _quantity of the syllables_, not the _length of the vowels_.
 * \endif
 *
 */
Scandeur::Scandeur(QObject *parent, LemCore *l, QString resDir) : QObject(parent)
{
    if (l==0)
    {
        _lemCore = new LemCore(this, resDir);
        // Je crée le lemmatiseur...
        _lemCore->setExtension(true);
        // ... et charge l'extension du lexique.
    }
    else _lemCore = l;
    if (resDir == "")
        _resDir = qApp->applicationDirPath() + "/data/";
    else if (resDir.endsWith("/")) _resDir = resDir;
    else _resDir = resDir + "/";
    lisParPos();

}


/**
 * \brief
 * \if French
 * Lecture des règles de quantité par position
 * \else
 * to read the rules for the quantities by position
 * \endif
 *
 *
 * \if French
 * Les règles de quantité par position sont
 * enregistrées dans le fichier data/parpos.txt.
 * Elles se présentent comme une expression rationnelle,
 * suivie par la chaine de remplacement.
 * Elles sont utilisées dans la fonction parPos.
 * \else
 * The rules to determine the quantities by position
 * are stored in the file "data/parpos.txt".
 * They take the form of a regular expression followed
 * by the remplacement string.
 * They are used in the parPos function.
 * \endif
 *
 */
void Scandeur::lisParPos()
{
    QStringList lignes = _lemCore->lignesFichier(_resDir + "parpos.txt");
    QStringList rr;
    foreach (QString ligne, lignes)
    {
        rr = ligne.split(";");
        _reglesp.append(Reglep(QRegExp(rr.at(0)), rr.at(1)));
    }
}

/**
 * @brief
 * \if French
 * détermine les quantités par position.
 * \else
 * determines the quantities by position.
 * \endif
 * @param f :
 * \if French
 * la forme sans quantité
 * \else
 * the form without quantity
 * \endif
 * @return
 * \if French
 * la forme avec les quantités déterminables par position.
 * \else
 * the form with all the quantities that can be determined by their position
 * \endif
 *
 * \if French
 * Quand une forme n'est pas reconnue,
 * on essaie quand même de trouver les quantités.
 * Ça n'est possible que pour les quantités définies par position.
 * On ne peut pas deviner les quantités par nature.
 * Les règles utilisées sont données dans le fichier parpos.txt
 * qui est lu par la routine lisParPos.
 * \else
 * When a form is not known, some of its quantities can
 * be deduced from their position.
 * One cannot know the quantities by nature.
 * The rules are given in the file parpos.txt
 * which is read in the lisParPos function.
 * \endif
 */
QString Scandeur::parPos(QString f)
{
    bool maj = f.at(0).isUpper();
    f = f.toLower();
    foreach (Reglep r, _reglesp)
        f.replace(r.first, r.second);
    if (f.endsWith("m") && !f.endsWith("āem") && !f.endsWith("ōem"))
        f[f.size() - 2] = Ch::breve(f[f.size() - 2]);
    // Un mot se terminant par "m" se termine par une syllabe brève.
    if (maj) f[0] = f[0].toUpper();
    return f;
}

/**
 * @brief
 * \if French
 * Recherche de dactyles et de spondées dans le schéma métrique d'un vers.
 * \else
 * \endif
 * @param nbr :
 * \if French
 * Nombre de mètres recherchés.
 * \else
 * \endif
 * @param ligne :
 * \if French
 * Le schéma métrique du vers.
 * \else
 * \endif
 * @param i :
 * \if French
 * Point de départ en nombre de caractère.
 * \else
 * \endif
 * @param pentam :
 * \if French
 * Pour la recherche de pentamètres.
 * \else
 * \endif
 * @return
 * \if French
 * La liste de toutes les combinaisons de dactyles et spondées trouvées.
 * \else
 * The list of all the combinasions of dactyls and spondees
 * \endif
 *
 * \if French
 * Le but est de déceler d'éventuels
 * hexamètres ou pentamètres dans de la prose.
 * Si la fin de vers n'est pas répérée, on ne peut pas appliquer
 * la méthode habituelle de scansion qui consiste à commencer par la fin.
 *
 * Le schéma métrique ne contient que les longueurs des syllabes.
 * Ici, les longues sont notées + et les brèves -.
 * Les mots sont séparés par un espace et on a gardé une trace
 * des élisions avec un `.
 * Une grande partie de la difficulté vient des voyelles communes
 * ou indéterminées, notées *. S'il n'y avait que des + et des -,
 * on n'aurait que D = + - - et S = + +. Avec l'* en plus, il faut considérer
 * toutes les possibilités :
 * s = * +, + * ou * *
 * d = * - -, + * -, + - *, + * *, * - *, * * - ou * * *.
 * Le découpage en mètres n'est plus nécessairement univoque.
 *
 * cherchePieds est une routine récursive qui construit la liste
 * de toutes les combinaisons possibles de dactyles et de spondées
 * en commençant par le caractère d'indice i.
 * Elle est récursive car chaque fois qu'elle a trouvé un D (ou d) ou un S (ou s),
 * elle va chercher toutes les combinaisons possibles à partir
 * du caractère d'indice i+3 ou i+2.
 *
 * Un hexamètre sera représenté par une séquence de cinq mètres,
 * dactyles (D ou d) ou spondées (S ou s) suivi d'un sixième noté X
 * (presque spondée: deux syllabes dont la première est longue,
 * la dernière étant indifférente).
 * Le pentamètre est lui composé de deux groupes de deux mètres,
 * dactyles ou spondées, séparés par une longue (notée Y ou y)
 * et terminé par une syllabe indifférente (notée Z).
 *
 * Le paramètre pentam est false au début de la recherche, qui va donc
 * chercher indifféremment pentamètres et hexamètres.
 * Après avoir trouvé deux mètres et une longue supposés former le début
 * d'un pentamètre, il ne faut plus que chercher une fin de pentamètre :
 * pour cela, pentam bascule à true.
 * \else
 * \endif
 */
QStringList Scandeur::cherchePieds(int nbr, QString ligne, int i, bool pentam)
{
    QStringList res;
    QString longueurs = "+-*";
    int ll = ligne.count();
    if (i >= ll)  // trop loin !
    {
        res << "";
        return res;
    }
    if (nbr == 1)  // Dernier pied !
    {
        // qDebug () << i << " " << ll << " " << ligne << ligne[i] <<
        // ligne[i+1];
        if (pentam)
        {
            // C'est un pentamètre, je ne dois avoir
            // qu'une quantité indifférente
            if (ligne[i] == ' ') i += 1;
            // J'étais sur un blanc (espace entre deux mots),
            // j'ai avancé d'une syllabe
            if ((i == ll - 1) || (ligne[i + 1] == ' ')) res << "Z";
            // Fin de ligne ou du mot
            else
                res << "";
            return res;
        }
        else
        {
            // C'est un hexamètre, je cherche encore deux syllabes
            while (!longueurs.contains(ligne[i]) && i < ll) i += 1;
            // qDebug()<<i<<" "<<ll<<" "<<ligne<< ligne[i] << ligne[i+1];
            if (i > ll - 2)
            {
                res << "";
                return res;
            }
            if (ligne[i] != '-')
            {
                if (i == ll - 2 && longueurs.contains(ligne[i + 1]))
                    res << "X";
                else if (ligne[i + 2] == ' ')
                    res << "X";
                else
                    res << "";
            }
            else
                res << "";
            return res;
        }
    }
    // J'ai traité les cas qui terminent la récursion
    while (!longueurs.contains(ligne[i]) && i < ll) i += 1;
    if (i == ll)
    {  // Encore un cas qui termine
        res << "";
        return res;
    }
    QChar car1 = ligne[i];
    int j = i + 1;
    while (!longueurs.contains(ligne[j]) && j < ll) j += 1;
    if (j == ll)  // Je n'ai qu'une syllabe : fin prématurée de pentamètre ?
    {
        res << "z";
        return res;
    }
    QChar car2 = ligne[j];
    QChar car3;
    int k = j + 1;
    while (!longueurs.contains(ligne[k]) && k < ll) k += 1;
    if (k == ll)
        car3 = ' ';
    else
        car3 = ligne[k];
    if (car1 == '-')
    {  // Encore un cas qui termine : aucun pied ne commence par une brève
        res << "";
        return res;
    }
    if (nbr == 4 && car1 == '+')
        res << Ch::ajoute("Y", cherchePieds(3, ligne, i + 1, true));
    if (nbr == 4 && car1 == '*')
        res << Ch::ajoute("y", cherchePieds(3, ligne, i + 1, true));
    if (car1 == '+' && car2 == '+')
        res << Ch::ajoute("S", cherchePieds(nbr - 1, ligne, j + 1, pentam));
    if ((car1 == '+' && car2 == '*') || (car1 == '*' && car2 == '+') ||
        (car1 == '*' && car2 == '*'))
        res << Ch::ajoute("s", cherchePieds(nbr - 1, ligne, j + 1, pentam));
    if (car1 == '+' && car2 == '-' && car3 == '-')
        res << Ch::ajoute("D", cherchePieds(nbr - 1, ligne, k + 1, pentam));
    if (car1 == '*' && (car2 == '-' || car2 == '*') &&
        (car3 == '-' || car3 == '*'))
        res << Ch::ajoute("d", cherchePieds(nbr - 1, ligne, k + 1, pentam));
    if (car1 == '+' && ((car2 == '*' && (car3 == '-' || car3 == '*')) ||
                        (car2 == '-' && car3 == '*')))
        res << Ch::ajoute("d", cherchePieds(nbr - 1, ligne, k + 1, pentam));
    return res;
}

/**
 * @brief Scande ou accentue une forme
 * @param forme : la forme à scander ou à accentuer.
 * @param nonTrouve : booléen qui indique que la forme a été reconnue.
 * @param debPhr : booléen qui indique que l'initiale peut être en majuscule.
 * @param accent : entier qui détermine le comportement de l'accentuation.
 * @return Renvoie une liste avec la forme scandée (ou accentuée) de
 * toutes les manières possibles.
 *
 * Cette fonction commence par lemmatiser la forme donnée
 * en utilisant le lemmatiseur de Collatinus.
 * Elle renvoie alors une liste avec la forme scandée (ou accentuée) de
 * toutes les manières possibles en appliquant
 * les quantités données par les dictionnaires et les règles prosodiques.
 * Si le mot n'est pas reconnu, seules les quantités déterminables
 * par leur position sont indiquées.
 * Les solutions sont ordonnées en fonction de leur fréquence,
 * la plus probable venant en premier.
 *
 * Le paramètre accent détermine le comportement de la fonction.
 * S'il vaut 0, la forme est scandée.
 * Non nul, la forme sera accentuée et plusieurs options sont possibles :
 *
 * - Les valeurs 1, 2 et 3 gèrent le comportement de l'accent
 * lorsque l'avant dernière syllabe est commune.
 *
 * - La valeur 4 (ajoutée aux précédentes) conduira à marquer les syllabes.
 *
 * - La valeur 8 (ajoutée aux précédentes) introduit "l'exception illius"
 * qui est toujours paroxyton quand son i est commun (supplante les valeurs 2 et 3)
 *
 * Les options et le groupe d'options sont additifs.
 * Les valeurs permises sont 0 (forme scandée),
 * 1-3 et 9-11 forme accentuée, 5-7 et 13-15 forme accentuée avec les syllabes marquées.
 * Les valeurs non-nulles règlent le comportement de l'accent si la pénultième
 * est commune : 1, 5, 9 et 13 la considère comme longue,
 * 2, 6, 10 et 14 comme brève (sauf pour illius avec les valeurs 10 et 14),
 * 3, 7, 11 et 15 ne place pas l'accent car la pénultième est ambiguë.
 *
 * \todo Lorsque l'on cherche à accentuer une forme qui n'a pas été reconnue,
 * je retourne la forme telle qu'elle est.
 * Or si la avant-dernière syllabe est fermée (double consonne), je sais que le mot
 * est paroxyton. Il est souvent proparoxyton si les deux dernières voyelles
 * se suivent. Le nom propre María est une exception (pas la seule ?).
 * Pas sûr que ça vaille le coup...
 */
QStringList Scandeur::formeq(QString forme, bool *nonTrouve, bool debPhr,
                           int accent)
{
    *nonTrouve = true;
    if (forme.isEmpty()) return QStringList();
    MapLem mp = _lemCore->lemmatiseM(forme, debPhr);
    if (mp.empty())
    {
        if (accent == 0)
            return QStringList() << parPos(forme);
        else
            return QStringList() << forme;
    }
    *nonTrouve = false;
    QStringList lforme;
    QMap<QString,int> mFormes;
    bool maj = forme.at(0).isUpper();
    foreach (Lemme *l, mp.keys())
    {
        foreach (SLem s, mp.value(l))
        {
            // QString f = Ch::ajoutSuff(s.grq, s.sufq, "", accent);
            // Le 3e paramètre, actuellement "", est prévu pour accepter
            // l->getHyphen(). Donc
            QString f = Ch::ajoutSuff(s.grq,s.sufq,l->getHyphen(),accent);
            //			if (s.grq == "-") f = l->grq();
            //			else f = parPos(s.grq);
            if (maj) f[0] = f[0].toUpper();
            mFormes[f] += _lemCore->fraction(_lemCore->tag(l,s.morpho)) * l->nbOcc();
            // Je compte le nombre d'occurrences de chaque forme.
//            lforme.append(f);
        }
    }
    //    lforme.removeDuplicates();
    foreach (QString f, mFormes.keys())
    {
        int nb = mFormes[f];
        int i = 0;
        while (i< lforme.size())
        {
            if (mFormes[lforme[i]] > nb) i += 1;
            else
            {
                // Le nombres d'occurrences de la forme courante est supérieur à la forme actuellement en position i.
                lforme.insert(i,f); // J'insère la forme courante en i.
                i = lforme.size() + 1; // Je sors !
            }
        }
        if (i == lforme.size()) lforme.append(f); // Je suis arrivé à la fin de la liste sans insérer la forme courante.
    }
    return lforme;
}

/**
 * @brief
 * \if French
 * Scande ou accentue le texte.
 * \else
 * Scan the text.
 * \endif
 * @param texte :
 * \if French
 * le texte à scander ou à accentuer
 * \else
 * the text to be scanned or accented.
 * \endif
 * @param accent :
 * \if French
 * un entier qui détermine si le résultat est
 * scandé ou accentué.
 * \else
 * an integer that determine the behavior
 * \endif
 * @param stats :
 * \if French
 * booléen qui affiche ou non les statistiques
 * \else
 * to compute the statistics.
 * \endif
 * @param majAut :
 * \if French
 * booléen qui autorise les majuscules initiales
 * \else
 * to neglect the initial uppercase letter.
 * \endif
 * @return
 * \if French
 * le texte scandé ou accentué.
 * \else
 * the text scanned or accented.
 * \endif
 *
 * Cette fonction scande ou accentue le texte donné.
 *
 * Le paramètre _accent_ détermine le comportement de la fonction.
 * S'il vaut 0, le texte est scandé.
 * Non nul, le texte sera accentué et plusieurs options sont possibles :
 *
 * - Les valeurs 1, 2 et 3 gèrent le comportement de l'accent
 * lorsque l'avant dernière syllabe est commune.
 *
 * - La valeur 4 (ajoutée aux précédentes) conduira à marquer les syllabes.
 *
 * - La valeur 8 (ajoutée aux précédentes) introduit "l'exception illius"
 * qui est toujours paroxyton quand son i est commun (supplante les valeurs 2 et 3)
 *
 * Les options et le groupe d'options sont additifs.
 * Les valeurs permises sont 0 (texte scandé),
 * 1-3 et 9-11 texte accentué, 5-7 et 13-15 texte accentué avec les syllabes marquées.
 * Les valeurs non-nulles règlent le comportement de l'accent si la pénultième
 * est commune : 1, 5, 9 et 13 la considère comme longue,
 * 2, 6, 10 et 14 comme brève (sauf pour illius avec les valeurs 10 et 14),
 * 3, 7, 11 et 15 ne place pas l'accent car la pénultième est ambiguë.
 *
 * Les valeurs >15 sont tronquées à leur quatre bits de poids faibles.
 * Les valeurs 4, 8 et 12 pourraient conduire à des résultats inattendus,
 * bien qu'aujourd'hui elles donnent le même résultat que la valeur 0 (texte scandé).
 *
 * Lorsque le texte est scandé (_accent = 0_) et que le booléen stats est true,
 * quelques statistiques seront faites sur les schémas métriques des vers
 * (séquences des longues et des brèves). Une recherche des hexamètres et des
 * pentamètres sera aussi effectuée, même si le texte est en prose.
 *
 * Si un mot est ambigu et que cela conduit à des formes accentuées ou scandées différentes,
 * elles seront toutes données. La plus probable sera la première
 * et les autres seront données entre parenthèses.
 * Pour la scansion, les règles usuelles d'allongement et d'élision
 * sont appliquées. Les voyelles élidées sont conservées pour que le texte reste
 * lisible, mais elles sont placées entre crochets droits [ ], sans quantité.
 * \note Une inexactitude subsiste dans l'élision :
 * le _e_ de l'auxiliaire _est_ est plus faible que les autres voyelles
 * et c'est lui qu'il faudrait élider.
 * Il faudrait écrire _dōctā [e]st_ plutôt que _dōct[a] ēst_.
 */
QString Scandeur::scandeTxt(QString texte, int accent, bool stats, bool majAut)
{
    accent = accent & 15;
    QString schemaMetric;
    QMap<QString, int> freqMetric;
    bool deb_phr;
    int decalage;
    QStringList vers;
    QStringList formes;
    QStringList aff;
    QStringList lignes = texte.split("\n");
    foreach (QString ligne, lignes)
    {
        QStringList separ;
        if (ligne.isEmpty())
            separ.append(ligne);
        else
            separ = ligne.split(QRegExp("\\b"));
        if (separ.count() > 0 && separ.at(0).count() > 0 &&
            separ.at(0).at(0).isLetter())
            separ.prepend("");
        if (separ.count() > 0 && separ.at(separ.count() - 1).count() > 0 &&
            separ.at(separ.count() - 1).at(0).isLetter())
            separ.append("");
        // J'ai maintenant une liste de formes et une liste de séparateurs
        // la ligne d'origine est la concaténation de separ[i]
        // Les termes pairs sont les séparateurs.
        // Les termes impairs sont les mots.
        // J'ai toujours un dernier séparateur, éventuellement vide.
        // La scansion peut commencer !
        decalage = aff.count();
        if (separ.size() < 3)
        {
            aff.append(ligne + "<br />\n");
            // C'est une ligne vide ou ne contenant pas de lettre :
            // je la laisse comme elle est !
            continue;
        }
        bool nonTr, nonTrSuiv;
        QStringList lforme;
        QStringList lfs = formeq(separ[1], &nonTrSuiv, true, accent);
        schemaMetric = "";
        for (int i = 1; i < separ.length(); i += 2)
        {
            aff.append(separ[i - 1]);
            lforme = lfs;
            nonTr = nonTrSuiv;
            if (i < separ.length() - 2)
            {
                deb_phr = separ[i + 1].contains(Ch::rePonct) || majAut;
                lfs = formeq(separ[i + 2], &nonTrSuiv, deb_phr, accent);
                if (accent == 0)
                {
                    if (Ch::consonnes.contains(lfs[0].at(0).toLower()))
                        for (int j = 0; j < lforme.length(); ++j)
                            Ch::allonge(&lforme[j]);
                    else
                        for (int j = 0; j < lforme.length(); ++j)
                            Ch::elide(&lforme[j]);
                }
            }
            lforme.removeDuplicates();
            // C'est le bon moment pour extraire le schéma métrique
            if (stats)
            {
                if (nonTr)
                    schemaMetric.append("?" + Ch::versPC(lforme[0]) + " ");
                else
                {
                    QString schMet = Ch::versPedeCerto(lforme[0]);
                    if (lforme.length() > 1)
                        for (int ii = 1; ii < lforme.length(); ii++)
                        {
                            QString schMet2 = Ch::versPedeCerto(lforme[ii]);
                            if (schMet.size() != schMet2.size())
                            {
                                schMet = "@" + lforme[0];
                                continue;
                            }
                            else
                                for (int j = 0; j < schMet.size(); j++)
                                    if (schMet[j] != schMet2[j])
                                        schMet[j] = '*';
                            // En cas de réponse multiple,
                            // je marque comme communes les voyelles qui
                            // diffèrent
                        }
                    schemaMetric.append(schMet + " ");
                }
            }
            // ajouter des parenthèses pour les analyses multiples
            if (lforme.length() > 1)
            {
                lforme[1].prepend('(');
                lforme[lforme.length() - 1].append(')');
            }
            if (nonTr)
                aff.append("<em>" + lforme[0] + "</em>");
            else
                aff.append(lforme.join(" "));
            // pour les analyses multiples, je dois insérer des espaces.
        }
        aff.append(separ[separ.length() - 1] + "<br />\n");
        // Je termine la ligne par le dernier séparateur et un saut de ligne.
        if (stats)
        {
            // Je cherche des vers dans la prose
//            qDebug() << schemaMetric;
            int ii = 0;
            int numMot = 1;
            int lsch = schemaMetric.count() - 10;
            // Un pentamètre compte au moins 10 syllabes, l'hexamètre 12.
            QString longueurs = "+-*";
            QStringList result;
            while (ii < lsch)
            {
                while (!longueurs.contains(schemaMetric[ii]) && ii < lsch)
                {
                    if (schemaMetric[ii] == ' ') numMot +=2;
                    // Si je franchis un blanc, je passe au mot suivant.
                    ii += 1;
                }
                // Je suis au début du mot.
                // Si le mot est de longueur ambigüe (son schéma commence par un ?),
                // je le traite quand même.
                result.clear();
                if (ii < lsch && schemaMetric[ii] != '-')
                    result = cherchePieds(6, schemaMetric, ii, false);
                // L'hexamètre et le pentamètre commence avec une longue ou une commune.
                // analyse du résultat
                QString numero;
                numero.setNum(ii);
                QString ajout = "";
                foreach (QString item, result)
                {
                    if (item.count() == 6)
                    {
                        // Hexamètres (eg DSDSDX) et pentamètres (eg DSYSSZ) ont 6 lettres.
                        if (ajout == "")
                            ajout = "<span style='color:red' title='" + item;
                        else
                            ajout += "\n" + item;
                        int syllabes = 0;
                        for (int a = 0; a < 6; a++)
                        {
                            if (item[a] == 'S' || item[a] == 's' ||
                                item[a] == 'X')
                                syllabes += 2;
                            if (item[a] == 'D' || item[a] == 'd') syllabes += 3;
                            if (item[a] == 'Y' || item[a] == 'y' ||
                                item[a] == 'Z')
                                syllabes += 1;
                        }
//                        qDebug() << ii << item << decalage << numMot << syllabes << schemaMetric.mid(ii, 2*syllabes);
                        int j = ii;
                        int nbMots = 1; // ToDo Vérifier ce qui se passe si la dernière syllabe est élidée :
                        // dSsSDX : mūltă (mūltā) quŏqu[e] (quōqu[e] quō̆qu[e]) ēt bēllō (bēllō̆) pāssūs, dūm cōndĕrĕt ūrb[em], īnfērrētquĕ
                        // L'hexamètre est +* *` + +* ++ + +-- +`, l'élision finale ne se faisant pas avec le vers suivant.
                        while (syllabes > 0)
                        {
                            if (schemaMetric[j] == '?' ||
                                schemaMetric[j] == '@')
                            {
                                while (schemaMetric[j] != ' ')
                                    j += 1;
                                // mot inconnu ou au nombre de syllabes indéterminé (eg Troiae)
                                // Je le saute (même si c'est improbable...)
                                nbMots += 2;
                                // Vérifier ce que je fais avec les ?
                            }
                            else if (longueurs.contains(schemaMetric[j]))
                            {
                                j += 1;
                                syllabes -= 1;
                            }
                            else if (schemaMetric[j] == '`')
                            {
                                if (syllabes == 1)
                                // J'ai une dernière syllabe qui est élidée à tort.
                                syllabes = 0;
                                else j += 1;
                                // Je peux avoir un mot entier élidé !
                                // Par exemple, c[um] Āntĭŏchō
                            }
                            else
                            {
                                nbMots += 2; // mot et séparateur.
                                while (!longueurs.contains(schemaMetric[j]) &&
                                       (j < schemaMetric.size()))
                                    j += 1;
                            }
                        }
                        QString it = item + " : ";
                        for (j = 0; j < nbMots; j++)
                            it += aff[decalage + numMot + j];
                        if (item.endsWith("Z"))
                            it = "<span style='color:red'>" + it + "</span>";
                        vers << it + "<br>\n";
                    }
                }
                if (ajout != "")
                {
                    // decalage+numMot est le numéro du
                    // mot, dans la liste aff, où mon analyse a commencé.
                    aff[decalage + numMot] =
                        ajout + "'>" + aff[decalage + numMot];
                    // 3 premiers mots en rouge
                    // aff[decalage+numMot+5]=aff[decalage+numMot+5]+"</span>";
                    // aff[decalage+numMot+5]+="</span>";
                    if (aff.count() > decalage + numMot + 5)
                        aff[decalage + numMot + 5].append("</span>");
                }
                // ii est le début du mot que je viens de traiter
                while ((schemaMetric[ii] != ' ') && ii < lsch) ii += 1;
                // J'avance jusqu'au blanc qui le suit.
//                numMot += 2;
                // Je suis sur le blanc qui précède un mot
//                if (schemaMetric[ii+1] == '@' || schemaMetric[ii+1] == '?')
  //                  numMot +=2; // J'ai un mot inconnu ou ambigu.
            }
            // Je remplace les +-* par des signes plus conventionnels
            schemaMetric.replace('-', "∪");
            schemaMetric.replace('+', "‑");
            schemaMetric.replace('*', "∪̲");
            // schemaMetric.replace('-', "u");
            // schemaMetric.replace('+', "-");
            // schemaMetric.replace('*', "-\u0306");
            aff.append("&nbsp;<small>" + schemaMetric + "</small>&nbsp;<br>\n");
            schemaMetric.remove(" ");
            schemaMetric.remove("`");
            // Pour ignorer la longueur de la dernière voyelle
            // if (!schemaMetric.endsWith("\u0306"))
            // {
            // schemaMetric[schemaMetric.length()-1]='-';
            // schemaMetric.append("\u0306");
            // }
            freqMetric[schemaMetric] += 1;
        }
    }
    if (stats)
    {
        // Il me reste à trier les freqMetric
        formes.clear();
        foreach (QString schM, freqMetric.keys())
            if (freqMetric[schM] > 1)
            {
                // Je ne garde que les schéma qui apparaissent plus d'une fois.
                int n = freqMetric[schM] + 10000;
                QString numero;
                numero.setNum(n);
                numero = numero.mid(1);
                formes << numero + " : " + schM;
            }
        formes.sort();
        aff.prepend(
            "<a href='#statistiques'>Statistiques</a> "
            "<a href='#analyses'>Analyses</a><br>\n");
        aff.prepend("<a name='texte'></a>");
        // aff.prepend("------------<br/>\n");
        // Pour séparer la liste du texte.
        vers.prepend(
            "<hr><a href='#texte'>Texte</a> "
            "<a href='#statistiques'>Statistiques</a><br>\n");
        vers.prepend("<a name='analyses'></a>");
        for (int i = 0; i < formes.size(); i++)
        {
            QString lg = formes[i];
            while (lg[0] == '0') lg = lg.mid(1);
            vers.prepend(lg + "<br/>\n");
            // En faisant un prepend, j'inverse l'ordre :
            // le plus fréquent finira premier
        }
        vers.prepend(
            "<hr><a href='#texte'>Texte</a> "
            "<a href='#analyses'>Analyses</a><br>\n");
        vers.prepend("<a name='statistiques'></a>");
        vers.append(
            "<a href='#texte'>Texte</a> "
            "<a href='#statistiques'>Statistiques</a> "
            "<a href='#analyses'>Analyses</a><br>\n");
        aff << vers;
        // aff.prepend("------------<br/>\n");
        // Pour séparer la liste du texte.
        // foreach (QString ligne, vers) aff.prepend(ligne);
    }
    return aff.join("");
}

/**
 * @brief Transforme un texte en CSV
 * @param texte : le texte à scander et accentuer
 * @param accent : les paramètres pour l'accentuation
 * @param majAut : booléen qui autorise les majuscules initiales
 * @return Une chaine avec le contenu du fichier CSV à créer.
 * La gestion des fichiers est laissée à la routine qui appelle cette fonction,
 * actuellement MainWindow::txt2csv ou MainWindow::exportCsv.
 *
 * Cette routine est assez proche de Scandeur::scandeTxt.
 * Toutefois, elle s'en distingue car elle donne à la fois la forme scandée
 * et la forme accentuée, ainsi que les séparateurs des mots.
 * À partir du fichier CSV, on peut donc reconstruire le texte d'origine,
 * le texte scandé et le texte accentué,
 * en choisissant la forme appropriée dans chaque ligne
 * que l'on fait suivre par les séparateurs trouvés sur cette même ligne.
 *
 * Pour la forme accentuée, les options sont déterminées par le paramètre _accent_
 * qui a les mêmes significations que dans Scandeur::scandeTxt.
 *
 * Le séparateur de champs dans le CSV est la tabulation.
 * Les colonnes en sont :
 *        1. un numéro d'ordre
 *        2. le numéro du paragraphe (ou vers)
 *        3. le numéro du mot dans ce paragraphe (ou vers)
 *        4. la forme du texte
 *        5. le séparateur (tout ce qu'il y a après ce mot et avant le mot suivant)
 *        6. la forme scandée
 *        7. le mot réduit à ses quantités
 *        8. la forme accentuée
 *        9. le rythme (voir Scandeur::code)
 *
 */
QString Scandeur::txt2csv(QString texte, int accent, bool majAut)
{
//    qDebug() << "J'entre";
    texte.replace(" & ", " et ");
    int numMot = 0; // Un numéro pour les mots
    int numPar = 0; // Un numéro pour les paragraphes
    QString debut = "%1\t%2\t%3\t";
    accent = accent & 15;
    QString forme;
    QMap<QString, int> freqMetric;
    bool deb_phr;
    int decalage;
    QStringList vers;
    QStringList formes;
    QStringList lforme;
    QStringList aff;
    QStringList lignes = texte.split("\n");
    qDebug() << lignes.size();
    aff.append("#\t#Paragr\t#Mot\tFormeTxt\tSepar\tQuant\t"
               "Metric\tAccent\tRythme");
    foreach (QString ligne, lignes)
    {
        QStringList separ;
        if (ligne.isEmpty())
            separ.append(ligne);
        else
            separ = ligne.split(QRegExp("\\b"));
        if (separ.count() > 0 && separ.at(0).count() > 0 &&
            separ.at(0).at(0).isLetter())
            separ.prepend("");
        if (separ.count() > 0 && separ.at(separ.count() - 1).count() > 0 &&
            separ.at(separ.count() - 1).at(0).isLetter())
            separ.append("");
        qDebug() << ligne << separ.size();
        // J'ai maintenant une liste de formes et une liste de séparateurs
        // la ligne d'origine est la concaténation de separ[i]
        // Les termes pairs sont les séparateurs.
        // Les termes impairs sont les mots.
        // J'ai toujours un dernier séparateur, éventuellement vide.
        int i = separ.size() - 2; // C'est le dernier mot
        while (i > 0)
        {
            separ[i+1].replace("\t"," / ");
            // Comme je vais sauver les séparateurs dans le CSV,
            // ils ne doivent pas contenir de tabulation.
            QString mot = separ[i];
            while ((mot.size()>0) && mot[0].isDigit())
            {
                separ[i-1].append(mot[0]);
                mot = mot.mid(1);
            }
            while ((mot.size()>0) && mot[mot.size() - 1].isDigit())
            {
                separ[i+1].prepend(mot[mot.size() - 1]);
                mot.chop(1);
            }
            if (mot.size() == 0)
            {
                // Ce "mot" n'était que des chiffres
                separ[i-1].append(separ[i + 1]);
                separ.removeAt(i);
                separ.removeAt(i);
            }
            else separ[i] = mot;
            i -= 2;
        }
        separ[0].replace("\t"," / ");
        // J'ai mis les chiffres qui pouvaient commencer ou finir les mots
        // dans les séparateurs.
        // La scansion peut commencer !
//        decalage = aff.count();
        ligne = debut.arg(aff.size() - 1).arg("").arg("");
        ligne.append("¶\t" + separ[0] + "\t\t\t\t");
        aff.append(ligne);
        if (separ.size() < 3) continue;

        numPar++;
        bool nonTr;
//        QStringList lfs = formeq(separ[1], &nonTrSuiv, true, 0);
//        schemaMetric = "";
        for (int i = 1; i < separ.length(); i += 2)
        {
            ligne = debut.arg(aff.size() - 1).arg(numPar).arg((i+1)/2);
            forme = separ[i];
            ligne.append(forme + "\t"); // La forme du texte et
            ligne.append(separ[i+1] + "\t"); // le séparateur qui suit.
            lforme.clear();
            bool allong = false;
            bool elide = false;
            if (i < separ.length() - 2)
            {
                lforme = formeq(separ[i + 2],&nonTr,true,0);
                if (nonTr)
                {
                    if (Ch::consonnes.contains(separ[i + 2].at(0).toLower()))
                        allong = true;
                    else elide = true;
                }
                else if (Ch::consonnes.contains(lforme[0].at(0).toLower()))
                    allong = true;
                else elide = true;
            }
            deb_phr = separ[i + 1].contains(Ch::rePonct) || majAut;
            MapLem mp = _lemCore->lemmatiseM(forme, deb_phr);
            if (mp.empty())
            {
                // La forme n'est pas reconnue
                QString fq1 = parPos(forme);
                if (allong) Ch::allonge(&fq1);
                else if (elide) Ch::elide(&fq1);
                ligne.append("¿" + fq1 + "?\t¿"); // La forme avec quantités
                QString PC = Ch::versPC(fq1);
                ligne.append(PC + "?\t¿");
                ligne.append(forme + "?\t¿");
                ligne.append(code(PC, accent) + "?");
            } // Fin de forme inconnue
            else
            {
                QMap<QString,int> mqFormes;
                QMap<QString,int> maFormes;
                bool maj = forme.at(0).isUpper();
                foreach (Lemme *l, mp.keys())
                {
                    foreach (SLem s, mp.value(l))
                    {
                        QString fq = Ch::ajoutSuff(s.grq,s.sufq,"",0);
                        QString fa = Ch::ajoutSuff(s.grq,s.sufq,l->getHyphen(),accent);
                        if (maj)
                        {
                            fq[0] = fq[0].toUpper();
                            fa[0] = fa[0].toUpper();
                        }
                        if (allong) Ch::allonge(&fq);
                        else if (elide) Ch::elide(&fq);
                        QString PC = Ch::versPedeCerto(fq);
                        fq.append("\t" + PC);
                        QString c = code(PC, accent);
                        if (!s.sufq.isEmpty() && c.endsWith("PP")) c.chop(1);
                        // L'enclitique appelle l'accent, d'où paroxyton
                        fa.append("\t" + c);
                        int nn = _lemCore->fraction(_lemCore->tag(l,s.morpho)) * l->nbOcc();
                        mqFormes[fq] += nn;
                        maFormes[fa] += nn;
                        // Je compte le nombre d'occurrences de chaque forme.
                    }
                }
                lforme.clear();
                foreach (QString f, mqFormes.keys())
                {
                    int nb = mqFormes[f];
                    int i = 0;
                    while (i< lforme.size())
                    {
                        if (mqFormes[lforme[i]] > nb) i += 1;
                        else
                        {
                            // Le nombres d'occurrences de la forme courante est supérieur à la forme actuellement en position i.
                            lforme.insert(i,f); // J'insère la forme courante en i.
                            i = lforme.size() + 1; // Je sors !
                        }
                    }
                    if (i == lforme.size()) lforme.append(f);
                    // Je suis arrivé à la fin de la liste sans insérer la forme courante.
                }
                if (lforme.size() == 1) ligne.append(lforme[0] + "\t");
                else
                {
                    // Je dois décomposer et recomposer les formes et les métriques
                    ligne.append(lforme[0].section("\t",0,0) + " (");
                    lforme[0] = lforme[0].section("\t",1,1);
                    for (int jj = 1; jj < lforme.size(); jj++)
                    {
                        ligne.append(lforme[jj].section("\t",0,0) + " ");
                        lforme[jj] = lforme[jj].section("\t",1,1);
                    }
                    ligne.chop(1);
                    ligne.append(")\t");
                    lforme.removeDuplicates();
                    if (lforme.size() == 1)
                        ligne.append(lforme[0] + "\t");
                    else
                    {
                        ligne.append(lforme[0] + " (");
                        for (int jj = 1; jj < lforme.size(); jj++)
                            ligne.append(lforme[jj] + " ");
                        ligne.chop(1);
                        ligne.append(")\t");
                    }
                }
                lforme.clear();
                foreach (QString f, maFormes.keys())
                {
                    int nb = maFormes[f];
                    int i = 0;
                    while (i< lforme.size())
                    {
                        if (maFormes[lforme[i]] > nb) i += 1;
                        else
                        {
                            // Le nombres d'occurrences de la forme courante est supérieur à la forme actuellement en position i.
                            lforme.insert(i,f); // J'insère la forme courante en i.
                            i = lforme.size() + 1; // Je sors !
                        }
                    }
                    if (i == lforme.size()) lforme.append(f);
                    // Je suis arrivé à la fin de la liste sans insérer la forme courante.
                }
                if (lforme.size() == 1) ligne.append(lforme[0]);
                else
                {
                    // Je dois décomposer et recomposer les formes et les métriques
                    ligne.append(lforme[0].section("\t",0,0) + " (");
                    lforme[0] = lforme[0].section("\t",1,1);
                    for (int jj = 1; jj < lforme.size(); jj++)
                    {
                        ligne.append(lforme[jj].section("\t",0,0) + " ");
                        lforme[jj] = lforme[jj].section("\t",1,1);
                    }
                    ligne.chop(1);
                    ligne.append(")\t");
                    lforme.removeDuplicates();
                    if (lforme.size() == 1)
                        ligne.append(lforme[0]);
                    else
                    {
                        ligne.append(lforme[0] + " (");
                        for (int jj = 1; jj < lforme.size(); jj++)
                            ligne.append(lforme[jj] + " ");
                        ligne.chop(1);
                        ligne.append(")");
                    }
                }
            } // Fin forme trouvée
            aff.append(ligne);
        } // Fin de la ligne
    } // Fin du texte
    qDebug() << aff.size();
    return aff.join("\n");
}

/**
 * @brief Détermine le nombre de syllabes et la nature (paroxyton ou proparoxyton) du mot
 * @param PC : Le mot réduit à ses quantités selon le codage de PedeCerto
 * @param accent : un entier qui détermine le comportement si la pénultième est commune
 * @return un code qui donne la longueur du mot et la position de l'accent
 *
 * Le code est composé d'un entier (le nombre de syllabes)
 * et de "p" si le mot est paroxyton ou "pp" s'il est proparoxyton.
 * Lorsque l'avant-dernière syllabe est **commune**,
 * la position de l'accent est problématique et le comportement est déterminé
 * par la variable _accent_ (voir Scandeur::scandeTxt pour les détails ;
 * ici, seuls les deux bits de poids faible sont pris en compte).
 *
 * Les routines pour réduire un mot à ses quantités sont dans l'espace de nommage Ch.
 * Les syllabes longues sont notées +, les brèves - et
 * les voyelles communes ou indéterminées sont notées *.
 *
 * En général, un mot est **paroxyton**, c'est à dire accentué sur la pénultième,
 * si cette syllabe est longue. Si elle est brève, le mot sera **proparoxyton**,
 * c'est à dire accentué sur l'anté-pénultième (3e syllabe en partant de la fin).
 *
 */
QString Scandeur::code(QString PC, int accent)
{
    QString nSyll = "%1";
    if (PC.size() > 2)
    {
        switch (PC[PC.size() - 2].unicode())
        {
        case 45: // -
            return nSyll.arg(PC.size()) + "pp";
            break;
        case 43: // +
            return nSyll.arg(PC.size()) + "p";
            break;
        default: // *, càd voyelle commune.
            if ((accent & 3) == 1)
                return nSyll.arg(PC.size()) + "p";
            else if ((accent & 3) == 2)
                return nSyll.arg(PC.size()) + "pp";
            else return nSyll.arg(PC.size());
            break;
        }
    }
    return nSyll.arg(PC.size());
}
