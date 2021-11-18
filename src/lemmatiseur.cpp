/*      lemmatiseur.cpp
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

#include "lemmatiseur.h"

/**
 * \file lemmatiseur.cpp
 * \brief module de lemmatisation des formes latines
 */

/**
 * @brief Constructeur de la classe Lemmatiseur
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
 * La (ou les) langue(s) cible(s) pour donner les traductions.
 * \else
 * The language for the translations.
 * \endif
 * @param resDir :
 * \if French
 * Le chemin complet du dossier contenant les fichiers de donnée.
 * \else
 * The absolute path for the resources directory.
 * \endif
 *
 * \if French
 * La classe Lemmatiseur propose les outils nécessaires pour lemmatiser un texte.
 * Elle utilise le moteur de lemmatisation de Collatinus (LemCore)
 * qui lui est passé en paramètre. Si ce moteur ne lui est pas donné,
 * elle le crée ici.
 * Si l'application envisagée utilise plusieurs modules
 * intermédiaires (Tagueur, Scandeur...),
 * il vaut mieux créer un seul moteur commun.
 *
 * Le paramètre optionnel @a resDir donne
 * le chemin complet du dossier contenant les fichiers de donnée.
 * Par défaut, il s'agit du dossier "data" placé à côté de l'exécutable.
 * \else
 * The Lemmatiseur class is meant to lemmatize texts.
 * As part of Collatinus, it uses its lemmatization core (LemCore).
 * If this core has been created elsewhere, a pointer is given to this creator.
 * If the pointer l is empty, then the lemmatization core is created here.
 * When the developped application uses different modules as this one
 * (e.g. Scandeur, Tagueur), it is recommended to create a shared core.
 *
 * The optional parameter @a resDir gives the absolute path to the
 * resources directory. If empty, the resources are assumed to be
 * in a folder "data" placed in the same dir as the exe.
 * \endif
 */
Lemmatiseur::Lemmatiseur(QObject *parent, LemCore *l, QString cible, QString resDir) : QObject(parent)
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

    _alpha = false;
    _formeT = false;
    _html = false;
    _majPert = false;
    _morpho = false;

    if (cible != "") setCible(cible);
    else setCible("fr en es");
}

/**
 * \fn QStringList Lemmatiseur::lemmatiseF (QString f, bool deb)
 * \brief Lemmatise la chaîne f, sans tenir compte des majuscules
 *        si deb (= début de phrase) est à true, et renvoie le
 *        résultat dans une QStringList.
 */
QStringList Lemmatiseur::lemmatiseF(QString f, bool deb)
{
    QStringList res;
    MapLem ml = _lemCore->lemmatiseM(f, deb);
    foreach (Lemme *l, ml.keys())
        res.append(l->humain(_html,_cible));
    // if (res.empty()) res.append(f);
    return res;
}

/**
 * \fn QStringList Lemmatiseur::frequences (QString txt)
 * \brief Lemmatise le texte txt et renvoie le résultat accompagné
 *        d'informations sur la fréquence d'emploi de
 *        chaque lemme.
 */
QStringList Lemmatiseur::frequences(QString txt)
{
    // J'essaie d'échanger comptage et lemmatisation Ph.
    QStringList formes = txt.split(Ch::reEspace);
    QHash<QString, int> freq;  // Occurrences par formes
    QStringList res;
    QString forme;
    int c = formes.count();
    for (int i = 0; i < c; ++i)
    {
        forme = formes.at(i);
        if (forme.isEmpty() || forme.toInt()) continue;
        // supprimer les ponctuations
        int pos = Ch::reAlphas.indexIn(forme);
        if (pos < 0) continue;
        forme = Ch::reAlphas.cap(1);
        if ((i == 0) || formes[i - 1].contains(Ch::rePonct))
            forme.prepend('*');  // = "*" + forme;
        freq[forme]++;  // je suppose que le créateur d'entiers l'initialise à 0
                        /* sinon prendre des précautons :
                           if (freq.keys ().contains (r))
                           freq[r] = freq[r]+1;
                           else freq[r] = 1;
                        */
    }
    QHash<QString, int> lemOcc;  // Nombre d'occurrences par lemme
    QHash<QString, QStringList>
        lemFormUnic;  // liste de formes uniques = par lemme
    QHash<QString, QStringList>
        lemFormAmb;  // Liste de formes ambiguës par lemme
    QHash<QString, QStringList>
        formLemAmb;  // Liste de lemmes par forme ambiguë
    foreach (forme, freq.keys())
    {
        if (forme.startsWith("*"))
        {
            QString forme2 = forme.mid(1);
            res = lemmatiseF(forme2, true);
        }
        else
            res = lemmatiseF(forme, false);
        int occ = freq[forme];
        if (res.count() == 1)
        {
            // lemFormUnic[res.at(0)].append(forme);
            // lemOcc[res.at(0)] += occ;
            lemFormUnic[res.first()].append(forme);
            lemOcc[res.first()] += occ;
        }
        else
        {
            for (int i = 0; i < res.count(); i++)
            {
                lemFormAmb[res.at(i)].append(forme);
                lemOcc[res.at(i)] += occ;
            }
            formLemAmb[forme] = res;
        }
    }
    formes.clear();
    formes << lemFormUnic.keys();
    formes << lemFormAmb.keys();
    formes.removeDuplicates();
    QStringList sortie;
    // formater les nombres pour tri inverse
    int nUnic;
    float nTotLem;
    int nAmb;
    float xAmb;
    foreach (QString lemme, formes)
    {
        nUnic = 0;
        foreach (forme, lemFormUnic[lemme])
        {
            nUnic += freq[forme];
        }
        nAmb = 0;
        xAmb = 0;
        foreach (forme, lemFormAmb[lemme])
        {  // Le lemme considéré a des formes qu'il partage avec d'autres lemmes
            nTotLem = 0;
            foreach (QString lem, formLemAmb[forme])
                nTotLem += lemOcc[lem];
            // Je somme les occurrences de tous les lemmes qui se rattachent à
            // ma forme
            nAmb += freq[forme];
            xAmb += freq[forme] * lemOcc[lemme] / nTotLem;
            // J'attribue une contribution de ma forme au lemme au prorata des
            // occ des lemmes
        }
        int n = xAmb + nUnic + 10000.5;
        QString numero;
        numero.setNum(n);
        numero = numero.mid(1);
        n = xAmb + 0.5;  // pour faire un arrondi et pas une troncature
        if (_hLem.isEmpty())
            sortie << QString("%1 (%2, %3, %5)\t%4<br/>\n")
                      .arg(numero)
                      .arg(nUnic)
                      .arg(nAmb)
                      .arg(lemme)
                      .arg(n);
        else
        {
            // J'ai une liste de lemmes connus : je mets des couleurs !
            QString lem = Ch::atone(lemme.left(lemme.indexOf(",")));
            lem.replace("j","i");
            lem.replace("J","I");
            lem.remove("<strong>");
            lem.remove("</strong>");
            QString format = "%1 (%2, %3, %5)\t<span style=\"color:";
            if (_hLem.contains(lem)) format += _couleurs[0];
            else format += _couleurs[1];
            format += "\">%4</span><br/>\n";
            sortie << format.arg (numero).arg(nUnic).arg(nAmb).arg (lemme).arg(n);
        }
    }
    qSort(sortie.begin(), sortie.end(), Ch::inv_sort_i);
    // déformatage des nombres
    int cs = sortie.count();
    for (int i = 0; i < cs; ++i)
    {
        QString ls = sortie.at(i);
        int z = 0;
        while (ls.at(z) == '0') ++z;
        ls = ls.mid(z);
        if (ls.at(0) == ' ') ls.prepend("&lt;1");
        sortie[i] = ls;
    }
    sortie.insert(0, "légende : n (a, b, c)<br/>\n");
    sortie.insert(1, "n = a+c<br/>\n");
    sortie.insert(
        2, "a = nombre de formes rattachées seulement à ce lemme<br/>\n");
    sortie.insert(3,
                  "b = nombre de formes ambigu\u00ebs (partagées par plusieurs "
                  "lemmes)<br/>\n");
    sortie.insert(4,
                  "c = nombre probable de formes ambigu\u00ebs rattachées à ce "
                  "lemme<br/>\n");
    sortie.insert(5, "------------<br/>\n");
    return sortie;
}

/**
 * @brief Lemmatise un texte
 * @param t : une chaine avec le texte (passée par adresse)
 * @param alpha : option pour l'ordre alphabétique
 * @param cumVocibus : option pour les formes du texte
 * @param cumMorpho : option pour indiquer l'analyse
 * @param nreconnu : option pour regrouper les formes non reconnues
 * @return Renvoie sous forme de chaîne la lemmatisation
 *        et la morphologie de chaque mot du texte t.
 *
 *        Les paramètres permettent de classer la sortie
 *        par ordre alphabétique ; de reproduire la
 *        forme du texte au début de chaque lemmatisation ;
 *        de donner les morphologies de chaque forme ; ou
 *        de rejeter les échecs en fin de liste. D'autres
 *        paramètres, comme le format de sortie (txt ou html)
 *        et la pertinence de la majuscule,
 *        sont donnés par des variables de classe.
 *
 *        Par effet de bord, la fonction modifie le texte
 *        t, passé par adresse dans le paramètre &t, en
 *        tenant compte de la liste des mots connus définie
 *        par l'utilisateur via l'option
 *        Fichier/Lire une liste de mots connus.
 * Voir Lemmatiseur::verbaCognita
 *
 */
QString Lemmatiseur::lemmatiseT(QString &t, bool alpha, bool cumVocibus,
                           bool cumMorpho, bool nreconnu)
{
    // pour mesurer :
    // QElapsedTimer timer;
    // timer.start();
/*
    alpha = alpha || _alpha;
    cumVocibus = cumVocibus || _formeT;
    cumMorpho = cumMorpho || _morpho;
    nreconnu = nreconnu || _nonRec;
*/
    // Pour coloriser le texte
    bool cumColoribus = !_couleurs.isEmpty();
    bool listeVide = _hLem.isEmpty();
    int colPrec = 0;
    int formesConnues = 0;
    // éliminer les chiffres et les espaces surnuméraires
    t.remove(QRegExp("\\d"));
//    t = t.simplified();
    // découpage en mots
    QStringList lm = t.split(QRegExp("\\b"));
    // conteneur pour les résultats
    QStringList lsv;
    // conteneur pour les échecs
    QStringList nonReconnus;
    // lemmatisation pour chaque mot
    if (lm.size() < 2)
    {
//        qDebug() << t << lm.size() << lm;
        return "";
        // Ça peut arriver que le texte ne contienne qu"une ponctuation
    }
    int i = 1;
    QMap<QString,int> occCode;
    while (i < lm.length())
    {
        if ((lm[i-1].endsWith("&") || lm[i-1].endsWith("&#")) && lm[i+1].startsWith(";"))
        {
            // Il y a un caractère en code html.
            if (lm[i].endsWith("gr") || lm[i].endsWith("aquo") ||
                    lm[i].endsWith("long") || lm[i-1].endsWith("&#"))
            {
                // C'est un caractère grec : je ne le traite pas.
                lm[i-1].append(lm[i]);
                lm.removeAt(i);
                lm[i-1].append(lm[i]);
                lm.removeAt(i);
                // Le pseudo-mot et le séparateur suivant sont groupés
                // dans le séparateur précédent sans être modifiés.
            }
            else
            {
                if (lm[i].endsWith("acute") || lm[i].endsWith("grave") ||
                        lm[i].endsWith("circ") || lm[i].endsWith("uml"))
                {
                    // C'est un acute, grave, circ ou uml :
                    // seul le premier caractère m'intéresse.
                    lm[i] = lm[i].mid(0,1);
                    if (lm[i+1] == ";")
                    {
                        // Cette voyelle se colle au mot suivant.
                        lm.removeAt(i+1);
                        lm[i].append(lm[i+1]);
                        lm.removeAt(i+1);
                        if (lm[i-1] == "&")
                        {
                            // Cette voyelle se colle au mot précédent.
                            lm[i-2].append(lm[i]);
                            lm.removeAt(i);
                            lm.removeAt(i-1);
                        }
                        else
                        {
                            lm[i-1].chop(1);
                            i += 2;
                        }
                    }
                    else
                    {
                        lm[i+1] = lm[i+1].mid(1);
                        if (lm[i-1] == "&")
                        {
                            // Cette voyelle se colle au mot précédent.
                            lm[i-2].append(lm[i]);
                            lm.removeAt(i);
                            lm.removeAt(i-1);
                        }
                        else
                        {
                            lm[i-1].chop(1);
                            i += 2;
                        }
                    }
                }
                else
                {
                    if (occCode.contains(lm[i])) occCode[lm[i]]++;
                    else occCode[lm[i]] = 1;
                    i += 2;
                }
            }
        }
        else i += 2;
    }
    foreach (QString code, occCode.keys())
        qDebug() << code << occCode[code];
    // Pour l'instant, juste la liste des codes rencontrés
    for (int i = 1; i < lm.length(); i += 2)
    {
        QString f = lm.at(i);
        if (f.toInt() != 0) continue;
        // nettoyage et identification des débuts de phrase
        QString sep = lm.at(i - 1);
        bool debPhr = ((i == 1 && lm.count() !=3) || sep.contains(Ch::rePonct));
        // lemmatisation de la forme
        MapLem map = _lemCore->lemmatiseM(f, !_majPert || debPhr);
        // échecs
        if (map.empty())
        {
            if (nreconnu)
                nonReconnus.append(f + "\n");
            else
            {
                if (_html)
                    lsv.append("<li style=\"color:blue;\">" + f + "</li>");
                else
                    lsv.append("> " + f + " ÉCHEC\n");
            }
            if (cumColoribus)
            {
                if (!listeVide)
                {
                    // La liste de mots connus n'est pas vide. Le mot en fait-il partie ?
                    QString lem = f;
                    lem.replace("j","i");
                    lem.replace("v","u");
                    lem.replace("J","I");
                    lem.replace("V","U");
                    // qDebug() << lem;
                    if (_hLem.contains(lem))
                    {
                        _hLem[lem]++;
                        if (colPrec != 0)
                        {
                            lm[i].prepend("</span><span style=\"color:"+_couleurs[0]+"\">");
                            colPrec = 0;
                        }
                    }
                    else if (colPrec != 2)
                    {
                        lm[i].prepend("</span><span style=\"color:"+_couleurs[2]+"\">");
                        colPrec = 2;
                    }
                }
                else if (colPrec != 2)
                {
                    lm[i].prepend("</span><span style=\"color:"+_couleurs[2]+"\">");
                    colPrec = 2;
                }
            }
        }
        else
        {
            bool connu = false;
            if (cumColoribus)
            {
                if (!listeVide)
                {
                    // La liste de mots connus n'est pas vide. Un des lemmes identifiés en fait-il partie ?
                    foreach (Lemme *l, map.keys())
                        if (_hLem.contains(l->cle()))
                        {
                            connu = true;
                            _hLem[l->cle()]++;
                        }
//                        connu = connu || _hLem.contains(l->cle());
                }
                if (connu)
                {
                    formesConnues += 1;
                    if (colPrec != 0)
                    {
                        lm[i].prepend("</span><span style=\"color:"+_couleurs[0]+"\">");
                        colPrec = 0;
                    }
                }
                else if (colPrec != 1)
                {
                    lm[i].prepend("</span><span style=\"color:"+_couleurs[1]+"\">");
                    colPrec = 1;
                }
            }

            if (cumVocibus)
            {
                // avec affichage des formes du texte
                // Le 10 novembre 2017,
                // Je reprends tout le passage qui ordonne les solutions
                // pour que ça se fasse pour tous les modes d'affichage.
                // Jusqu'à présent, ça ne marchait qu'en html avec les formes.
                QString debMorph = "\n    . ";
                QString sepMorph = "\n    . ";
                QString finMorph = "";
                QString debLem = "  - ";
                QString finLem = "\n";
                // Je définis les chaines de début et fin d'entités
                if (_html)
                {
                    // et les modifient si l'affichage est en html.
                    debMorph = "<ul><li>";
                    sepMorph = "</li><li>";
                    finMorph = "</li></ul>";
                    debLem = "<li>";
                    finLem = "</li>";
                }
                QMultiMap<int,QString> listeLem;
                // Je construis un QMultiMap avec un nombre d'occurrences en clef.
                foreach (Lemme *l, map.keys())
                {
                    QString lem = debLem + l->humain(_html, _cible, true);
                    int frMax = 0;
                    if (cumMorpho && !_lemCore->inv(l, map))
                    {
                        // Je veux aussi donner les morphos associées
                        // ce qui n'a de sens que si la forme du texte apparaît.
                        QMultiMap<int,QString> listeMorph;
                        // Un 2e QMultiMap
                        foreach (SLem m, map.value(l))
                        {
                            int fr = _lemCore->fraction(_lemCore->tag(l,m.morpho));
                            if (fr > frMax) frMax = fr;
                            if (m.sufq.isEmpty())
                                listeMorph.insert(-fr,m.grq + " " + _lemCore->morpho(m.morpho));
                            // La fréquence est négative pour inverser l'ordre.
                            else
                                listeMorph.insert(-fr,m.grq + " + " + m.sufq +
                                                  " " + _lemCore->morpho(m.morpho));
                        }
                        lem.append(debMorph);
                        QStringList lMorph = listeMorph.values();
                        // Liste des morphos ordonnée en ordre croissant des clefs.
                        // Comme la clef est -fr, la plus fréquente vient d'abord.
                        lem.append(lMorph.join(sepMorph));
                        // Je joins les différentes morphos.
                        lem.append(finMorph);
                        // J'ai encapsulé les morphos et les ai ajoutées au lemme.
                    }
                    else foreach (SLem m, map.value(l))
                    {
                        // Sans donner les morphos, je dois quand même évaluer frMax.
                        int fr = _lemCore->fraction(_lemCore->tag(l,m.morpho));
                        if (fr > frMax) frMax = fr;
                    }
                    if (frMax == 0) frMax = 1024;
                    lem.append(finLem);
                    // Je finis d'encapsuler le lemme
                    listeLem.insert(-frMax * l->nbOcc(),lem);
                    // et lui associe la plus grande fréquence observée pour les morphos.
                }
                QStringList lLem = listeLem.values();
                // Les valeurs sont en ordre croissant
                // Comme les fréquences sont négatives, la plus fréquente vient d'abord.
                QString lin = lLem.join("");
                // L'ensemble des solutions forme un tout que j'encapsule
                if (_html)
                {
                    lin.prepend("<li><h4>" + f + "</h4><ul>");
                    lin.append("</ul></li>\n");
                }
                else
                {
                    lin.prepend("* " + f + "\n");
                }
//                lsv.append(lin);
                if (!connu || listeVide) lsv.append(lin);
                // Par défaut, pas d'aide pour les mots connus.
            }
            else  // sans les formes du texte
            {
                foreach (Lemme *l, map.keys())
                {
                    QString lin = l->humain(_html, _cible);
                    if (cumMorpho && !_lemCore->inv(l, map) && !alpha)
                    {
                        // Sans les formes du texte et avec les lemmes en ordre
                        // alphabétique, la morpho n'aurait que peu de sens.
                        QTextStream fl(&lin);
                        if (_html)
                        {
                            fl << "<ul>";
                            foreach (SLem m, map.value(l))
                                fl << "<li>" << m.grq << " " << _lemCore->morpho(m.morpho) << "</li>";
                            fl << "</ul>\n";
                        }
                        else
                            foreach (SLem m, map.value(l))
                                fl << "\n    . " << m.grq << " " << _lemCore->morpho(m.morpho);
                    }
                    if (_html)
                    {
                        lin.prepend("<li>");
                        lin.append("</li>");
                    }
                    else
                    {
                        lin.prepend("* ");
                        lin.append("\n");
                    }
//                    lsv.append(lin);
                    if (!connu || listeVide) lsv.append(lin);
                    // Par défaut, pas d'aide pour les mots connus.
                }
            }
        }
    }  // fin de boucle de lemmatisation pour chaque mot

    if (alpha)
    {
        lsv.removeDuplicates();
        qSort(lsv.begin(), lsv.end(), Ch::sort_i);
    }
    // peupler lRet avec les résultats
    QStringList lRet = lsv;
    if (_html)
    {
        lRet.prepend("<ul>");
        lRet.append("</ul>\n");
    }
/*    foreach (QString item, lsv)
    {
        if (_html)
            lRet.append("<li>" + item + "</li>");
        else
            lRet.append("* " + item + "\n");
    }
    if (_html) lRet.append("</ul>\n");*/
    // non-reconnus en fin de liste si l'option nreconnu
    // est armée
    if (nreconnu && !nonReconnus.empty())
    {
        nonReconnus.removeDuplicates();
        QString nl;
        if (_html) nl = "<br/>";
        if (alpha) qSort(nonReconnus.begin(), nonReconnus.end(), Ch::sort_i);
        QString titreNR;
        int tot = (lm.count() - 1) / 2;
        QTextStream(&titreNR) << "--- " << nonReconnus.count() << "/"
                              << tot << " ("
                              << (((nonReconnus.count() * 200 + tot) / tot) / 2)
                              << " %) FORMES NON RECONNUES ---" << nl << "\n";
        lRet.append(titreNR + nl);
        foreach (QString nr, nonReconnus)
            lRet.append(nr + nl);
    }
    if (cumColoribus)
    {
        lm[0].append("<span style=\"color:"+_couleurs[0]+"\">");
        lm[lm.size()-1].append("</span>");
        t = lm.join("");
        t.replace("\n","<br/>\n");
        if (!listeVide)
        {
            QString stats = "<strong>Formes connues : %1 sur %2 (%3%)<br/></strong>";
            lRet.prepend(stats.arg(formesConnues).arg((lm.size()/2)).arg((200*formesConnues)/(lm.size()-1)));
        }
    }
    // fin de la mesure :
    // qDebug()<<"Eneide"<<timer.nsecsElapsed()<<"ns";
    return lRet.join("");
}

/**
 * \brief lemmatise un texte en prenant les options de la classe
 * \return La lemmatisation
 *        et la morphologie de chaque mot du texte t, sous forme de chaîne de caractères.
 *
 * Les options sont ici prises dans les paramètres de la classe.
 *
 * Voir aussi Lemmatiseur::lemmatiseT(QString &t, bool alpha, bool cumVocibus,
 *                           bool cumMorpho, bool nreconnu)
 */
QString Lemmatiseur::lemmatiseT(QString &t)
{
    return lemmatiseT(t, _alpha, _formeT, _morpho, _nonRec);
}

/**
 * \brief Applique lemmatiseT sur le contenu du fichier
 *        f et renvoie le résultat. Les paramètres sont
 *        les mêmes que ceux de lemmatiseT.
 *
 * \obsolete Cette fonction ne semble pas être utilisée
 */
QString Lemmatiseur::lemmatiseFichier(QString f, bool alpha, bool cumVocibus,
                                 bool cumMorpho, bool nreconnu)
{
    // lecture du fichier
    QFile fichier(f);
    fichier.open(QFile::ReadOnly);
    QTextStream flf(&fichier);
    flf.setCodec("UTF-8"); // Pour windôze !
    QString texte = flf.readAll();
    fichier.close();
    return lemmatiseT(texte, alpha, cumVocibus, cumMorpho, nreconnu);
}

/**
 * @brief Lire une liste de mots connus
 * @param fichier : nom du fichier avec la liste des mots connus
 * @param vb : booléen pour activer/désactiver le TextiColor
 *
 * Activation/désactivation de la fonction TextiColor.
 * Reçoit de MainWindow::verbaCognita le nom du fichier
 * contenant une liste de mots connus.
 * Les mots de cette liste sont lemmatisés et **toutes les formes**
 * des lemmes correspondants seront supposées connues.
 * Voir aussi Lemmatiseur::verbaOut.
 *
 * Le fichier de mots connus peut contenir les couleurs utilisées.
 * Les trois couleurs seront sur trois lignes successives
 * sous la forme d'un dièse suivi de six chiffres hexadécimaux #RrVvBb.
 * \attention Le dièse doit être le premier caractère de la ligne
 * qui comptera 7 caractères exactement.
 * \attention #FFFFFF est le _blanc_ qui ne se verra pas.
 * \attention #FF0000 est un _rouge vif_ pas nécessairement des plus lisibles.
 *
 * Par défaut, elles sont :
 *      * _vert_ pour les mots que l'élève est censé connaître
 *      * _noir_ pour les mots que Collatinus reconnaît
 *      * _rouge_ pour les mots inconnus
 *
 * Ce qui correspond à :
 * \code
 * #00A000
 * #000000
 * #A00000
 * \endcode

 * Si le nom est vide (aucun fichier sélectionné),
 * Collatinus n'utilisera que les deux couleurs standards :
 *     * _noir_ pour les mots que Collatinus reconnaît.
 *     * _rouge_ pour les mots inconnus
 *
 * Sans fichier, le TextiColor peut servir à repérer les fautes d'OCR.
 */
void Lemmatiseur::verbaCognita(QString fichier,bool vb)
{
    _hLem.clear();
    _couleurs.clear();
    if (vb)
    {
        // Couleurs par défaut
        _couleurs << "#00A000"; // vert
        _couleurs << "#000000"; // noir
        _couleurs << "#A00000"; // rouge
    }
    // Je peux activer le textiColor sans avoir de liste.
    // Auquel cas, j'aurai les mots connus par Collatinus en noir
    // et les non-reconnus en rouge.
    if (vb && !fichier.isEmpty())
    {
        QFile file(fichier);
        if (file.open(QFile::ReadOnly | QFile::Text))
        {
            QTextStream in(&file);
            QString ligne = in.readLine();
            while (ligne.startsWith("!") || ligne.isEmpty()) ligne = in.readLine();
            // Je saute les commentaires et les lignes vides.
            int i = 0;
            while (ligne.startsWith("#") &&  !in.atEnd())
            {
                if ((i<3) && (ligne.size() == 7)) _couleurs[i] = ligne;
                i+=1;
                ligne = in.readLine();
            }
            // Je peux changer les couleurs dans le fichier
            MapLem item;
            while (!in.atEnd())
            {
                if (!ligne.startsWith("!") && !ligne.isEmpty()) // hLem.insert(ligne,1);
                {
                    item = _lemCore->lemmatiseM (ligne, false, false);
                    foreach (Lemme *lem, item.keys())
                        _hLem.insert(lem->cle(),0);
                }
                ligne = in.readLine();
            }
        }
    }
}

/**
 * @brief Sauvegarder le taux d'utilisation des mots connus
 * @param fichier : nom du fichier à créer
 *
 * Cette fonction s'utilise en relation avec Lemmatiseur::verbaCognita.
 * Quand une liste de mots connus a été chargée, on peut connaître
 * le nombre de fois que chacun de ces lemmes ont été rencontrés
 * dans le ou les textes traités.
 */
void Lemmatiseur::verbaOut(QString fichier)
{
    if (_hLem.isEmpty()) return; // Rien à sauver !
    QString format = "%1\t%2\n";
    QFile file(fichier);
    if (file.open(QFile::WriteOnly | QFile::Text))
        foreach (QString lem, _hLem.keys())
    {
            file.write(format.arg(lem).arg(_hLem[lem]).toUtf8());
    }
}

/**
 * \fn bool Lemmatiseur::optAlpha()
 * \brief Accesseur de l'option Lemmatiseur::_alpha, qui
 *        permet de fournir par défaut des résultats dans
 *        l'ordre alphabétique.
 */
bool Lemmatiseur::optAlpha() { return _alpha; }
/**
 * \fn bool Lemmatiseur::optHtml()
 * \brief Accesseur de l'option Lemmatiseur::_html, qui
 *        permet de renvoyer les résultats au format html.
 */
bool Lemmatiseur::optHtml() { return _html; }

/**
 * \fn bool Lemmatiseur::optFormeT()
 * \brief Accesseur de l'option Lemmatiseur::_formeT,
 *        qui donne en tête de lemmatisation
 *        la forme qui a été analysée.
 */
bool Lemmatiseur::optFormeT() { return _formeT; }

/**
 * \fn bool Lemmatiseur::optMajPert()
 * \brief Accesseur de l'option Lemmatiseur::_majPert,
 *        qui permet de tenir compte des majuscules
 *        dans la lemmatisation.
 */
bool Lemmatiseur::optMajPert() { return _majPert; }
/**
 * \fn bool Lemmatiseur::optMorpho()
 * \brief Accesseur de l'option Lemmatiseur::_morpho,
 *        qui donne l'analyse morphologique
 *        des formes lemmatisées.
 */
bool Lemmatiseur::optMorpho()
{
    return _morpho;
}

/**
 * @brief Accesseur de l'option Lemmatiseur::_nonRec
 * pour regrouper les formes non reconnues à la fin de la liste
 */
bool Lemmatiseur::optNonRec()
{
    return _nonRec;
}
/*
QString Lemmat::transfMed(QString f, bool rad)
{
    if (f.isEmpty()) return "";
    bool maj = f.at(0).isUpper();
    f = f.toLower();
    foreach (Reglep r, _reglesMed)
        if (!r.second.endsWith("*")) f.replace(r.first, r.second);
        else if (rad)
        {
            QString rs = r.second;
            rs.chop(1);
            f.replace(r.first, rs);
        }
    if (maj) f[0] = f[0].toUpper();
    return f;
}
*/
// modificateurs d'options

/**
 * \fn void Lemmatiseur::setAlpha (bool a)
 * \brief Modificateur de l'option Lemmatiseur::_alpha.
 */
void Lemmatiseur::setAlpha(bool a) { _alpha = a; }
/**
 * \fn void Lemmatiseur::setCible(QString c)
 * \brief Permet de changer la langue cible.
 */
void Lemmatiseur::setCible(QString c)
{
    _cible = c;
    _lemCore->setCible(c);
}
/**
 * \fn void Lemmatiseur::setHtml (bool h)
 * \brief Modificateur de l'option Lemmatiseur::_html.
 */
void Lemmatiseur::setHtml(bool h) { _html = h; }
/**
 * \fn void Lemmatiseur::setFormeT (bool f)
 * \brief Modificateur de l'option Lemmatiseur::_formeT.
 */
void Lemmatiseur::setFormeT(bool f) { _formeT = f; }
/**
 * \fn void Lemmatiseur::setMajPert (bool mp)
 * \brief Modificateur de l'option Lemmatiseur::_majPert.
 */
void Lemmatiseur::setMajPert(bool mp) { _majPert = mp; }
/**
 * \fn void Lemmatiseur::setMorpho (bool m)
 * \brief Modificateur de l'option Lemmatiseur::_morpho.
 */
void Lemmatiseur::setMorpho(bool m) { _morpho = m; }
/**
 * @brief Modificateur de l'option Lemmatiseur::_nonRec
 * @param n : booléen
 */
void Lemmatiseur::setNonRec(bool n) { _nonRec = n; }

/**
 * \fn QString Lemmatiseur::cible()
 * \brief Renvoie la langue cible dans sa forme
 *        abrégée (fr, en, de, it, etc.).
 */
QString Lemmatiseur::cible()
{
    return _cible;
}
