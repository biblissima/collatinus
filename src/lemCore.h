/*      lemCore.h
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

#ifndef LEMCORE_H
#define LEMCORE_H

#include <QMap>
#include <QString>
#include <QStringList>
#include <QtCore/QCoreApplication>
#include <QDir>
#include <QFile>
#include <QRegExp>
#include <QTextStream>
#include <iostream>

#include "ch.h"
#include "irregs.h"
#include "lemme.h"
#include "modele.h"

class Irreg;
class Lemme;
class Radical;
class Desinence;

/**
 * @brief structure pour stocker le résultat d'une lemmatisation
 *
 * Cette structure groupe la forme avec quantités, un éventuel suffixe
 * et l'analyse morphologique associée.
 */
typedef struct
{
    QString grq; /*!< la forme avec quantités */
    int morpho; /*!< l'analyse morphologique (entier) */
    QString sufq; /*!< l'éventuel suffixe avec quantités */
} SLem;

/**
 * @brief Une MapLem regroupe par lemme les résultats d'une lemmatisation
 *
 * Il s'agit d'une QMap qui associe le pointeur (clef) vers un objet Lemme
 * à une liste (valeur) de SLem qui contiennent chacun un résultat de
 * lemmatisation et d'analyse.
 * C'est donc un moyen simple de regrouper, par lemme,
 * les différentes analyses possibles d'une forme.
 */
typedef QMap<Lemme*, QList<SLem> > MapLem;

/**
 * @brief ModLem est une variante de MapLem pour l'identification des formes inconnues.
 *
 * Pour l'identification des formes inconnues, je dois garder
 * le modèle, le supposé radical et la morpho.
 * Je réutilise le SLem pour ces deux derniers et je les
 * ordonne en fonction du modèle.
 * Si je fais cette tentative d'identification sur l'ensemble
 * des formes non-reconnues d'un texte, je peux privilégier
 * les identifications liées au même modèle de formes différentes
 * avec le même supposé radical.
 */
typedef QMap<Modele*, QList<SLem> > ModLem;

/**
 * @brief Une Reglep regroupe une expression rationnelle et la chaine de remplacement
 *
 * Une liste de telles Reglep permet de faire des remplacements à la chaine.
 * Utilisée dans LemCore::transfMed et Scandeur::parPos
 */
typedef QPair<QRegExp, QString> Reglep;

/**
 * @brief La classe LemCore est le noyau de lemmatisation
 *
 * Ce module est le cœur du programme :
 * c'est lui qui va organiser les données et lemmatiser les formes.
 * Il est donc appelé par les modules *intermédiaires*,
 * Lemmatiseur, Scandeur et Tagueur.
 * A priori, c'est plutôt aux classes *intermédiaires*
 * que l'on s'adressera pour ré-utiliser ce code.
 *
 * En lisant les fichiers de données, il va créer les collections
 * d'objets dont il a besoin : Lemme, Modele, Desinence, Radical et Irreg.
 * Les noms de ces classes sont assez explicites.
 * Leur fonctionnement est détaillé dans les pages correspondantes.
 *
 * La fonction importante dans cette classe est surtout
 * LemCore::lemmatiseM qui lemmatise un mot en cherchant
 * les diverses transformations qu'il a pu subir.
 * Elle appelle LemCore::lemmatise qui lemmatise la forme
 * sans transformation.
 *
 * \todo Il manque un lexique personnel dans Collatinus 11.
 * C'est en principe résolu (en grand) avec Collatinus 12.
 *
 * \todo La gestion des formes non-reconnues est aussi un peu sommaire.
 * Dans la lemmatisation d'un texte, les formes non-reconnues
 * sont juste groupées à la fin de la liste (si l'option
 * correspondante est validée).
 * Dans la scansion, on marque la quantité des syllabes
 * lorsqu'elle est déterminable par position.
 * Dans le tagueur, les mots non-reconnus sont ignorés...
 * Je ne sais pas au juste comment gérer ça.
 * En particulier, on ne peut pas le faire sur une forme isolée.
 * Dans un texte, si plusieurs mots ne sont pas reconnus
 * qui commencent avec un même potentiel radical,
 * on peut avoir une piste intéressante pour déterminer
 * un paradigme et voir si toutes ces formes peuvent
 * conduire à un lemme plausible (difficile pour la 3e déclinaison).
 * Commencer par se faire une idée de la fréquence d'utilisation
 * des diverses désinences ?
 */
class LemCore : public QObject
{
    Q_OBJECT

   private:
    // fonction d'initialisation
    void ajAssims();
    void ajAbrev();
    /*! Liste des abréviations, voir LemCore::ajAbr */
    QStringList abr;
    // Pour avoir une liste d'abréviation éditable...
    void ajContractions();
    int  aRomano(QString f);
    void lisIrreguliers();
    void lisFichierLexique(QString filepath);
    void lisLexique();
    void lisExtension();
    void lisModeles();
    void lisMorphos(QString lang);
    void lisTraductions(bool base, bool extension);
    // variables et utils
    /*! Association des préfixes assimilés et non-assimilés sans quantité */
    QMap<QString, QString> assims;
    /*! Association des préfixes assimilés et non-assimilés avec quantités */
    QMap<QString, QString> assimsq;
    // Pas sûr que les QMaps soient une bonne idée ! Ph.
    QStringList lAss;
    QStringList lAssq;
    QStringList lDesAss;
    QStringList lDesAssq;
// Je crée des listes pour faire la même chose.
    /*! Association des formes contractées et non-contractées */
    QMap<QString, QString> _contractions;
    /*! Liste des désinences avec forme (clef) et pointeur (valeur) */
    QMultiMap<QString, Desinence *> _desinences;
    QString decontracte(QString d);
    QMultiMap<QString, Irreg *> _irregs;
    /*! Liste des langues cibles en forme abrégée (clef) et longue (valeur) */
    QMap<QString, QString> _cibles;
    /*! Liste des lemmes avec forme (clef) et pointeur (valeur) */
    QMap<QString, Lemme *> _lemmes;
    /*! Liste des modèles avec nom (clef) et pointeur (valeur) */
    QMap<QString, Modele *> _modeles;
    /*! Liste des analyses morphologiques avec langue (clef) et liste lisible (valeur) */
    QMap<QString,QStringList> _morphos;
    /*! Liste des cas avec langue (clef) et liste lisible (valeur) */
    QMap<QString,QStringList> _cas;
    /*! Liste des genres avec langue (clef) et liste lisible (valeur) */
    QMap<QString,QStringList> _genres;
    /*! Liste des nombres avec langue (clef) et liste lisible (valeur) */
    QMap<QString,QStringList> _nombres;
    /*! Liste des temps avec langue (clef) et liste lisible (valeur) */
    QMap<QString,QStringList> _temps;
    /*! Liste des modes avec langue (clef) et liste lisible (valeur) */
    QMap<QString,QStringList> _modes;
    /*! Liste des voix avec langue (clef) et liste lisible (valeur) */
    QMap<QString,QStringList> _voix;
    /*! Liste des autres mots-clefs avec langue (clef) et liste lisible (valeur) */
    QMap<QString,QStringList> _motsClefs;
    // Les morphos doivent pouvoir être données en anglais !
    /*! Liste des radicaux avec forme (clef) et pointeur (valeur) */
    QMultiMap<QString, Radical *> _radicaux;
    /*! Liste des méta-variables du fichier modeles.la avec nom (clef) et liste de désinences (valeur) */
    QMap<QString, QString> _variables;

    /*! Liste de règles pour transformer les graphies classiques en graphies médiévales */
    QList<Reglep> _reglesMed; // Règles de transformation entre graphies classique et médiévale
    void lisTransfMed();
    QString transfMed(QString f, bool rad=false); // Exactement comme parPos, mais pour les transformations médiévales
    /*! Booléen pour traiter les graphies médiévales */
    bool _medieval; // Au cas où j'arrive avec le même code à traiter les deux cas.
    /*! Associe une graphie médiévale (clef) à une graphie classique (valeur) pour une désinence */
    QMap<QString, QString> _desMed;
    /*! Associe une graphie médiévale (clef) à une graphie classique (valeur) pour un irrégulier */
    QMap<QString, QString> _irrMed;
    /*! Associe une graphie médiévale (clef) à une graphie classique (valeur) pour un radical */
    QMultiMap<QString, QString> _radMed;

    /*! Option indiquant le chargement de l'extension du lexique, voir LemCore::setExtension.*/
    bool _extension; // = false;
    /*! La langue choisie, voir LemCore::setCible */
    QString _cible;  // langue courante, 2 caractères ou plus

    /*! Nombre d'occurrences du tag dans le corpus du LASLA */
    QMap<QString, int> _tagOcc; // Nombre d'occurrences du tag.
    /*! Nombre d'occurrences du POS (1er caractère du tag) dans le corpus du LASLA */
    QMap<QString, int> _tagTot; // Nombre total en fonction du premier caractère du tag.
    /*! Nombre d'occurrences du trigramme dans le corpus du LASLA */
    QMap<QString, int> _trigram; // Nombre d'occurrences des séquences de 3 tags.
    void lisTags(bool tout = false);

    /*! Le nom du répertoire contenant les données. */
    QString _resDir; // Le chemin du répertoire de ressources
    /*! Booléen indiquant si l'extension du lexique a été chargée */
    bool _extLoaded; // = true après chargement de l'extension
    // Lorsque j'ai chargé l'extension, je dois pouvoir ignorer les analyses qui en viennent.
//    bool _nbrLoaded; // Si les nombres ont été chargés, je dois les effacer avant de les charger à nouveau.

   public:
    LemCore(QObject *parent = 0, QString resDir="");
    bool estAbr(QString m);
    // Pour remplacer Ch::abrev.contains(m) avec la liste des abréviations chargées.
    void ajDesinence(Desinence *d);
//    void ajModele(Modele *m);
    void ajRadicaux(Lemme *l);
    QString assim(QString a);
    QString assimq(QString a);
    QMap<QString, QString> cibles();
    QString desassim(QString a);
    QString desassimq(QString a);
//    static QString deramise(QString r);
    static bool estRomain(QString f);
    bool inv(Lemme *l, const MapLem ml);
    MapLem lemmatise(QString f);  // lemmatise une forme
    ModLem inconnue(QString f);  // tente d'identifier une forme inconnue.
    // lemmatiseM lemmatise une forme en contexte
    //MapLem lemmatiseM(QString f, bool debPhr = true);
    MapLem lemmatiseM(QString f, bool debPhr = true, int etape  =0);
    Lemme *lemme(QString l);
    // lemmes(ml) renvoie la liste des graphies des lemmes
    int nbOcc(QString l);
    QStringList lemmes(MapLem ml);
    QStringList lignesFichier(QString nf);
    // Lit les lignes d'un fichier. Est devenu public.
    Modele *modele(QString m);
    QString morpho(int m);
    // QStringList           suffixes;
    /*! Association des suffixes sans et avec quantités */
    QMap<QString, QString> suffixes;
    QString variable(QString v);
    // Lire un fichier de césures étymologiques (non-phonétiques)
    void lireHyphen (QString fichierHyphen);

    // Pour l'internationalisation
    QString cas(int i);
    QString genre(int i);
    QString nombre(int i);
    QString temps(int i);
    QString modes(int i);
    QString voix(int i);
    QString motsClefs(int i);

    void setCible(QString c);
    QString cible();
    bool optExtension();


//    QString tagPhrase(QString phr);
    QString tag(QString lp, int m);
    QString tag(Lemme *l, int m);
    int fraction(QString listTags);
    int tagOcc(QString t);
    int trigram(QString seq);

   public slots:
    void setExtension(bool e);
    void setMedieval(bool e);
};

#endif // LEMCORE_H
