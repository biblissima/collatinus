/*               tagueur.cpp
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

#include "tagueur.h"

/**
 * \file tagueur.cpp
 * \brief module de lemmatisation et de désambiguïsation des formes latines
 */

/**
 * @brief
 * \if French
 * Créateur de la classe Tagueur pour lemmatiser et désambiguïser des textes.
 * \else
 * Constructs a Tagueur to tag sentences.
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
 * @param cible :
 * \if French
 * La langue cible pour les traductions et les analyses.
 * \else
 * The language for the translations and analyses.
 * \endif
 * @param resDir :
 * \if French
 * Le chemin complet du dossier contenant les fichiers de donnée.
 * \else
 * The absolute path for the resources directory.
 * \endif
 *
 * \if French
 * La classe Tagueur est conçue pour proposer une fonction d'étiquetage
 * basée sur un modèle de Markov caché (HMM) au 2e ordre.
 * Initialement intégrée à Collatinus, elle en utilise le moteur de lemmatisation (LemCore).
 * Si le moteur est déjà créé par ailleurs,
 * il suffit de passer le pointeur en question.
 * Si le pointeur n'est pas donné,
 * le moteur (LemCore) sera créé ici.
 * Si l'application envisagée utilise plusieurs modules
 * intermédiaires (tagueur, scandeur...),
 * il vaut mieux créer un seul moteur commun.
 *
 * Lors de la création de cette classe (si on n'a pas créé le moteur), on peut choisir
 * la langue cible dans laquelle seront données les traductions et analyses.
 * Il s'agit d'une chaine de caractères contenant au moins deux lettres
 * (fr : français ; en : anglais ; etc...) et au plus huit
 * (jusqu'à trois groupes de deux lettres séparés par un espace).
 * Le fait de spécifier plusieurs langues précise l'ordre dans lequel
 * seront cherchées les traductions si on ne dispose pas de la traduction
 * dans la première langue choisie. Ainsi "it fr en" va conduire le
 * programme à chercher la traduction en italien. Si la traduction
 * n'existe pas en italien, elle sera d'abord cherchée dans le lexique
 * français et en désespoir de cause en anglais.
 * Par défaut, elle vaut "fr en es", valeur définie dans LemCore.
 * \attention la langue cible peut être changée dans LemCore,
 * mais à l'heure actuelle le Tagueur ne prévoit pas l'accès
 * à la fonction LemCore::setCible qui est sollicitée par MainWindow.
 *
 * Le paramètre optionnel resDir donne
 * le chemin complet du dossier contenant les fichiers de donnée.
 * Par défaut, il s'agit du fichier "data" placé à côté de l'exécutable.
 * \else
 * The Tagueur class is meant to desambiguate with tags
 * using a second order Hidden Markov Model (HMM).
 * As part of Collatinus, it uses its lemmatization core (LemCore).
 * If this core has been created elsewhere, a pointer is given to this creator.
 * If the pointer l is empty, then the lemmatization core is created here.
 * When the developped application uses different modules as this one
 * (e.g. Lemmatiseur, Scandeur), it is recommended to create a shared core.
 *
 * One can change the language used for the translations and the analyses
 * with the QString cible. It should contain at least two characters
 * (en : English; fr : French) and at most eight (three groups of 2,
 * separated by a space). For instance "de en fr" will give the translations
 * in German, if possible. If the German translation is not found,
 * the English one will be searched for. French would be the last trial.
 * The default value is "fr en es" as defined in LemCore.
 *
 * The optional parameter resDir gives the absolute path to the
 * resources directory. If empty, the resources are assumed to be
 * in a folder "data" placed in the same dir as the exe.
 * \endif
 */
Tagueur::Tagueur(QObject *parent, LemCore *l, QString cible, QString resDir) : QObject(parent)
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
    if (cible != "") _lemCore->setCible(cible);
}

/**
 * @brief
 * \if French
 * Pour désambiguïser une phrase (ou un texte) par étiquetage (HMM).
 * \else
 * To disambiguate a sentence (or a text) using a Hidden Markov Model.
 * \endif
 * @param t :
 * \if French
 * Le texte.
 * \else
 * The text.
 * \endif
 * @param p :
 * \if French
 * Une position dans le texte en nombre de caractères.
 * \else
 * A position in the text as a character count.
 * \endif
 * @param affTout :
 * \if French
 * Option pour afficher toutes les possibilités d'analyse
 * plutôt que la seule choisie.
 * \else
 * Option to show all the possible analyses.
 * \endif
 * @param majPert :
 * \if French
 * Option pour tenir compte des majuscules initiales.
 * \else
 * Option to take into account the initial uppercase characters.
 * \endif
 * @param affHTML :
 * \if French
 * Option pour choisir l'affichage en HTML (par defaut) ou une sortie en CSV.
 * \else
 * Option to choose between the standard HTML display and a CSV-output.
 * \endif
 * @return
 * \if French
 * Une chaine de caractères contenant la meilleure solution en HTML.
 * \else
 * A QString with the best solution encoded in HTML.
 * \endif
 *
 * \if French
 * Cette fonction cherche la meilleure analyse des mots d'une phrase
 * en tenant compte du contexte à l'aide d'un modèle de Markov caché (HMM).
 * Pour cela, elle attribue des étiquettes aux mots et calcule une
 * probabilité pour chaque séquence d'étiquettes possible.
 * Pour ce faire, elle s'appuie sur les données statistiques
 * tirées des textes lemmatisés du LASLA.
 *
 * La phrase traitée est celle autour de la position p dans le texte t.
 * Si p&lt;0, tout le texte sera traité en séparant chaque phrase.
 * Pour p≥0, le programme va, en partant de la position p, remonter et descendre
 * dans le texte jusqu'à trouver une ponctuation forte (. ! ? : ;).
 *
 * Le format de sortie est principalement destiné à l'affichage HTML.
 * J'ai ajouté quelques ancres pour faciliter la navigation,
 * mais je n'ai pas mis de liens pour y aller :
 * elles servent de repères.
 * Chaque phrase est entre des balises &lt;div id='Sentence_x'> et &lt;/div>
 * où x est le numéro de la phrase. La numérotation commence à 0.
 * Ça n'a de sens que si l'ensemble du texte est tagué.
 * Si on ne tague qu'une seule phrase, ce numéro vaut toujours 0.
 *
 * Ces &lt;div> commencent par donner la phrase, puis la meilleure séquence de tags
 * et la probabilité associées. Vient ensuite le second choix qui ne sera pas explicité.
 * La meilleure séquence de tags est explicitée sous la forme d'une
 * &lt;ul id='sent_x'> où chaque mot est un &lt;li id='S_x_W_y'>
 * avec x le numéro de la phrase et y le numéro du mot dans celle-ci.
 * Chaque &lt;li> commence avec la forme du texte suivie du tag qui
 * lui est attribué par la meilleure séquence.
 * Le tag le plus probable hors contexte est donné entre parenthèses
 * lorsqu'il diffère de celui choisi par le tagueur.
 *
 * Si l'option affTout est valisée (affTout = true), toutes les analyses possibles
 * sont ajoutées, dans le &lt;li> précédent, sous la forme d'une &lt;ul> sans ancre.
 * À la fin de chaque &lt;li> de cette seconde liste,
 * sont donnés le tag et la probabilité associée.
 * S'il y a des homonymes qui ont des analyses avec le même tag,
 * ces probabilités sont les mêmes (elles sont associées au tag).
 * Le choix se fait alors en fonction du nombre d'occurrences
 * relevé pour le lemme. Ce nombre est donné aussi (lorsqu'il n'est pas nul).
 * Il est entre des balises &lt;small> et &lt;/small> et entre parenthèses.
 * \else
 * This function looks for the best analysis of the words in the sentence
 * taking into account the context with tags and a Hidden Markov Model (HMM).
 * The statistical data used in the HMM are taken from the texts
 * lemmatized by the LASLA.
 *
 * The treated sentence in around the position p in the text.
 * If p&lt;0, all the text is treated, sentence by sentence.
 * For p≥0, the program looks forward and backward for the next
 * strong punctuation mark (. ! ? : ;).
 *
 * The output format is mainly intended for the display.
 * I have added a few anchors to facilitate the browsing,
 * but I do not add any link to them.
 * Each sentence is enclosed in tags &lt;div id='Sentence_x'> and &lt;/div>
 * where x is the rank of the sentence, starting from 0.
 * It makes sense only if all the text is treated.
 * If a single sentence is treated, this numbre is always 0.
 *
 * These &lt;div> start with the sentence, and then give
 * the best sequence of tags together with its probability.
 * Then comes the second choice and the associated probability.
 * The best choice is made explicit as a &lt;ul id='sent_x'> where
 * each word is an item &lt;li id='S_x_W_y'>, x and y being
 * the rank of respectively the sentence and the word.
 * Each item begins with the form of the text and the associated tag.
 * The most probable tag without taking into account the context
 * is given between parenthesis if different.
 *
 * With affTout=true, all the possible analyses of the form are given
 * in the previous item as a &lt;ul> without any anchor.
 * At the end of each item of this second list, the tag and
 * the associated probability are given.
 * \endif
 */
QString Tagueur::tagTexte(QString t, int p, bool affTout, bool majPert, bool affHTML)
{
    // éliminer les chiffres et les espaces surnuméraires
    t.remove(QRegExp("\\d"));
//    t = t.simplified(); // Rmq : perd les retours de ligne !
    int tl = t.length() - 1;
    const QString pp = ".;!?";
    int numMot = 0; // Un numéro pour les mots (utile pour le CSV).
    int numPhr = 0; // Un numéro pour les phrases (utile seulement pour le texte complet)
    int dph = p;
    bool tout = false;
    if (p < 0)
    {
        p = 0;
        dph = 0;
        tout = true; // Pour faire tout le texte, phrase par phrase.
    }
    else
    {
        if ((dph > 0) && pp.contains(t.at(dph))) --dph;
        // Si j'ai cliqué sur une ponctuation, je traite la phrase qui précède.
        // régression au début de la phrase
        while (dph > 0 && !pp.contains(t.at(dph)) && (t.mid(dph,2) != "\n\n")) --dph;
        if (dph != 0) dph += 1; // J'élimine la ponctuation de la phrase précédente.
    }

    // conteneur pour les résultats
    QStringList lsv;
    // progression jusqu'en fin de phrase
    int fph = p;
    while (fph < tl)
    {
        while (fph < tl && !pp.contains(t.at(fph)) && (t.mid(fph,2) != "\n\n")) ++fph;
        QString phr = t.mid(dph, fph - dph).trimmed();
        // Si le texte se termine sans ponctuation, je perds le dernier caractère.
//        qDebug() << tl << fph << t.at(tl);
        if ((fph == tl) && !pp.contains(t.at(tl)) && (t.mid(tl,1) != "\n"))
            phr.append(t[tl]);
        // découpage en mots
        QStringList lm = phr.split(QRegExp("\\b"));

        if (lm.size() > 1)
        {
            // Il y a au moins un mot...
//            while (Ch::abrev.contains(lm[lm.size()-2]))
            while (_lemCore->estAbr(lm[lm.size()-2]))
            {
                // Ma phrase se terminait par une abréviation : je continue.
                fph++;
                while (fph < tl && !pp.contains(t.at(fph))) ++fph;
                phr = t.mid(dph, fph - dph).trimmed();
                if ((fph == tl) && !pp.contains(t.at(tl)) && (t.mid(tl,1) != "\n"))
                    phr.append(t[tl]);
                lm = phr.split(QRegExp("\\b"));
            }

            QList<Mot*> mots;
            // lemmatisation pour chaque mot
            for (int i = 1; i < lm.length(); i += 2)
            {
                bool debVers = !majPert || lm[i-1].contains("\n");
                Mot * mot = new Mot(lm[i],(i-1)/2, debVers, _lemCore); // TODO : Vérifier si on a des vers avec majuscule...
                mots.append(mot);
            }  // fin de boucle de lemmatisation pour chaque mot
            Mot * mot = new Mot("",mots.size(),true, _lemCore); // Fin de phrase
            mots.append(mot); // J'ajoute un mot virtuel en fin de phrase avec le tag "snt".
//            qDebug() << mots.size();

            QStringList sequences;
            QList<double> probabilites;
            sequences.append("snt");
            probabilites.append(1.0);
            double branches = 1.0; // Pour savoir combien de branches a l'arbre.
            // Je suis en début de phrase : je n'ai que le tag "snt" et une proba de 1.
            for (int i = 0; i < mots.size(); i++)
            {
//                qDebug() << "Traitement du mot : " <<  i;
                Mot *mot = mots[i];
                QStringList lTags = mot->tags(); // La liste des tags possibles pour le mot
                QStringList nvlSeq; // Nouvelle liste des séquences possibles
                QList<double> nvlProba; // Nouvelle liste des probas.
                // Je dois ajouter tous les tags possibles à toutes les sequences et calculer les nouvelles probas.
                int sSeq = sequences.size();
                int sTag = lTags.size();
                if (sTag == 0) continue; // J'ignore pour l'instant les mots inconnus, cf. plus bas.
                branches *= sTag;
                for (int j = 0; j < sSeq; j++)
                {
                    QString bigr = sequences[j].right(7); // Le bigramme terminal
                    long prTot = 0;
                    QList<long> pr;
                    for (int k = 0; k < sTag; k++)
                    {
                        QString seq = bigr + " " + lTags[k];
                        long p = mot->proba(lTags[k]) * (4 * _lemCore->trigram(seq) + 1);
                        pr << p;
                        prTot += p;
                    }
                    // J'ai tout ce qui dépend de k et la somme pour normaliser.
                    if (prTot == 0)
                    {
                        prTot = 1;
                        //qDebug() << mot->forme() << "proba nulle ! " << sequences[j];
                    }
                    for (int k = 0; k < sTag; k++)
                    {
                        nvlSeq.append(sequences[j] + " " + lTags[k]);
                        nvlProba.append(probabilites[j] * pr[k] / prTot);
                        // Si j'avais gardé toutes les séquences, ce serait une vraie probabilité (normalisée à 1)
                    }
                }
                // Ajouter les enclitiques.
                if (!mot->tagEncl().isEmpty())
                {
                    QString ajout = " " + mot->tagEncl();
                    for (int j = 0; j < nvlSeq.size(); j++) nvlSeq[j].append(ajout);
                    // Comme toutes les formes à tag unique, l'enclitique ne change pas les probabilités.
                }
                // J'ai toutes les sequences et leur proba.
                // Pour chaque bigramme terminal, je ne dois garder que la séquence la plus probable.
                // En faisant ce tri, je fais une sélection sur le tag i-2 (attention aux mots avec enclitique).
                // Si je veux garder une info sur l'ordre des tags du mot i-2, c'est maintenant !
                if (i > 1)
                {
                    // Le mot i-2 existe !
                    int debut = nvlSeq[0].size() - 11;
                    if (!mot->tagEncl().isEmpty()) debut -= 4; // Je dois reculer d'un tag de plus.
                    if (!mots[i-1]->tagEncl().isEmpty()) debut -= 4; // Je dois reculer d'un tag de plus.
                    if (!mots[i-2]->tagEncl().isEmpty()) debut -= 4; // Je dois reculer d'un tag de plus.
                    // Le tag du mot i-2 est nvlSeq[j].mid(debut, 3);
                    for (int j = 0; j < nvlSeq.size(); j++) mots[i-2]->setBestOf(nvlSeq[j].mid(debut, 3), nvlProba[j]);
                }
                sequences.clear();
                probabilites.clear();
//                qDebug() << mot->forme() << nvlProba << nvlSeq;
                for (int j = 0; j < nvlSeq.size(); j++) if (nvlProba[j] > 0)
                {
                    QString bigr = nvlSeq[j].right(7); // Les deux derniers tags
                    QString seq = "";
                    double val = -1;
                    QString seq2 = "";
                    double val2 = -1;
                    for (int k = j; k < nvlSeq.size(); k += sTag) // Pour retrouver le bigramme terminal, il faut au moins le même dernier tag.
                        if (bigr == nvlSeq[k].right(7))
                        {
                            if (val2 < nvlProba[k])
                            {
                                // J'y passe au moins une fois au début.
                                // La séquence mérite la 1ère ou la 2e place.
                                if (val < nvlProba[k])
                                {
                                    // 1ère place !
                                    val2 = val;
                                    seq2 = seq;
                                    val = nvlProba[k];
                                    seq = nvlSeq[k];
                                }
                                else
                                {
                                    // Seulement la 2e place
                                    val2 = nvlProba[k];
                                    seq2 = nvlSeq[k];
                                }
                            }
                            nvlProba[k] = -1; // Pour ne pas considérer deux fois les mêmes séquences.
                        }
                    // val et seq correspondent aux proba et séquence avec le bigramme considéré qui ont la plus grande proba.
                    sequences << seq;
                    probabilites << val;
                    if (val2 > 0)
                    {
                        // J'ai une deuxième séquence assez probable.
                        sequences << seq2;
                        probabilites << val2;
                    }
                }
        //        qDebug() << mot->forme() << sSeq << sTag << nvlSeq.size() << sequences.size();
                if (sequences.size() == 0) break;
            } // fin de la phrase.

            // Les probas associées aux tags du dernier vrai mot.
            if (mots.length() > 1)
            {
                // Le mot mots.length()-2 existe !
                int debut = sequences[0].size() - 7;
                if (!mots[mots.length()-2]->tagEncl().isEmpty()) debut -= 4; // Je dois reculer d'un tag de plus.
                // Le tag du mot mots.length()-2 est sequences[j].mid(debut, 3);
                for (int j = 0; j < sequences.size(); j++)
                    mots[mots.length()-2]->setBestOf(sequences[j].mid(debut, 3), probabilites[j]);
            }
            // Le tri final !
            QString seq = "";
            double val = -1;
            QString seq2 = "";
            double val2 = -1;
            for (int i = 0; i < sequences.size(); i++)
                if (val2 < probabilites[i])
                {
                    if (val < probabilites[i])
                    {
                        val2 = val;
                        seq2 = seq;
                        val = probabilites[i];
                        seq = sequences[i];
                    }
                    else
                    {
                        val2 = probabilites[i];
                        seq2 = sequences[i];
                    }
                }

            if (affHTML)
            {
            QString prob = "<div id='Sentence_%1'>";
            lsv.append(prob.arg(numPhr) + phr + "<br/><br/>\n");
            prob = "<br/>\n avec la proba : %1 pour %2 branches.<br/>";
            lsv.append(seq + prob.arg(val).arg(branches));
            if (val2 > 0)
            {
                prob = "Deuxième choix avec la proba : %1 <br/>\n %2<br/>";
                lsv.append(prob.arg(val2).arg(seq2));
            }
            prob = "<ul id='sent_%1'>";
            lsv.append(prob.arg(numPhr)); // Je vais commencer la liste de mots
            seq = seq.mid(4); // Je supprime le premier tag qui est "snt".
            for (int i = 0; i < mots.size()-1; i++)
                if (!mots[i]->inconnu()) // Les mots inconnus ne figurent pas dans la séquence (cf. plus haut)
                {
                    lsv.append(mots[i]->choisir(seq.left(3), numPhr, affTout));
                     // Si enclitique mid(8)
                    if (mots[i]->tagEncl().isEmpty()) seq = seq.mid(4);
                    else seq = seq.mid(5 + mots[i]->tagEncl().size());
                }
                else lsv.append("<li id='unknown'>" + mots[i]->forme() + " : non trouvé</li>");

            lsv.append("</ul></div>");
            }
            else
            {
                // Pour avoir une sortie au format CSV.
//                qDebug() << "je suis sorti du tagage." << mots.size();
                QString debut = "%1\t%2\t%3\t";
                seq = seq.mid(4); // Je supprime le premier tag qui est "snt".
                for (int i = 0; i < mots.size()-1; i++)
                    if (!mots[i]->inconnu()) // Les mots inconnus ne figurent pas dans la séquence (cf. plus haut)
                    {
//                        qDebug() << "mot" << i << mots[i]->forme();
                        numMot += 1;
                        QString blabla = mots[i]->choisir(seq.left(3), numPhr, affTout);
//                        qDebug() << "fin du choix";
                        // C'est une ligne en HTML : je dois remplacer les délimiteurs par des tabulations...
                        QString entete = blabla.mid(0,blabla.indexOf("<br"));
                        if (entete.contains("<ul>")) entete = entete.mid(0,entete.indexOf("<span"));
                        entete = entete.mid(entete.indexOf("ng>")+3);
                        entete.replace("</strong> ","\t");
                        entete.prepend(debut.arg(numMot).arg(numPhr+1).arg(i+1));
                        entete.append("\t");
                        // Dans l'entête, j'ai les numéros, la forme et le tag.
                        if (blabla.contains("—&gt;"))
                        {
                            // C'est le premier choix.
                            blabla = blabla.mid(blabla.indexOf("—&gt;"));
                            QString ligne = blabla.mid(blabla.indexOf("ng>")+3);
                            ligne = ligne.mid(0,ligne.indexOf("</span"));
                            QString lem = Ch::atone( ligne.mid(0,ligne.indexOf("</str"))) + "\t";
                            lem.replace("<sup>","_");
                            lem.remove("</sup>");
                            // Le 12 janvier 2021, j'ai introduit le numéro
                            // d'homonymie en exposant.
                            // Dans le lemme, je le fais précéder par _.
                            ligne.replace("</strong>, <em>",", ");
                            if (ligne.contains("<small>("))
                            {
                                ligne.replace("</em> <small>(","\t");
                                ligne.replace(")</small> : ","\t");
                            }
                            else ligne.replace("</em> : ","\t\t");
                            ligne.replace(" — ","\t");
//                            ligne.replace(":","\t");
                            lsv.append(entete + lem + ligne);
                        }
                        if (blabla.contains("<ul>"))
                        {
                            // Il n'y a pas de choix ou tout est affiché.
                            entete.chop(4);
//                            entete.append("\t");
                            blabla = blabla.mid(blabla.indexOf("<ul>"));
                            blabla = blabla.mid(0,blabla.indexOf("</ul>"));
                            while (blabla.contains("</li>") && !blabla.isEmpty())
                            {
                                QString ligne = blabla.mid(0,blabla.indexOf("</li>"));
                                blabla = blabla.mid(blabla.indexOf("</li>")+5);
                                ligne = ligne.mid(ligne.indexOf("ng>")+3);
                                ligne = ligne.mid(0,ligne.indexOf("</span"));
                                QString lem = Ch::atone( ligne.mid(0,ligne.indexOf("</str"))) + "\t";
                                lem.replace("<sup>","_");
                                lem.remove("</sup>");
                                ligne.replace("</strong>, <em>",", ");
                                if (ligne.contains("<small>("))
                                {
                                    ligne.replace("</em> <small>(","\t");
                                    ligne.replace(")</small> : ","\t");
                                }
                                else ligne.replace("</em> : ","\t\t");
                                QString tag = ligne.mid(ligne.indexOf("(")+1, 3);
                                tag.append("\t");
                                ligne.replace(" — ","\t");
//                                ligne.replace(":","\t");
//                                ligne.replace(" (","\t");
//                                ligne.remove(")");
                                lsv.append(entete + tag + lem + ligne);
                            }
                        }
//                        lsv.append(entete + blabla);
                         // Si enclitique mid(8)
                        if (mots[i]->tagEncl().isEmpty()) seq = seq.mid(4);
                        else seq = seq.mid(5 + mots[i]->tagEncl().size());
//                        qDebug() << "fin du mot" << i << lsv;
                    }
                    else
                    {
                        // Mot inconnu !
//                        qDebug() << "mot inconnu" << i << mots[i]->forme();
                        numMot += 1;
                        QString entete = mots[i]->forme();
                        entete.prepend(debut.arg(numMot).arg(numPhr+1).arg(i+1));
                        entete.append("\t\tunknown");
                        lsv.append(entete);
                    }
            }
            if (tout)
            {
                if (affHTML) lsv << "<br/>";
                numPhr++;
            }
            else return lsv.join("\n") + "\n";
            // Je retourne le résultat de la phrase si tout est false.
        }
        dph = fph + 1;
        fph++;
    } // while (fph < tl)
    // Si tout est true, je vais jusqu'au bout de texte.
    return lsv.join("\n") + "\n";
}

