# Collatinus
Sources du lemmatiseur-scandeur-analyseur morphologique latin Collatinus.
Les sources de la version 10 sont sur le dépôt [collatinus-10-src](https://github.com/biblissima/collatinus-10-src).

**Collatinus est une application libre, gratuite et
multi-plateforme (Mac, Windows et Debian GNU/Linux),
simple à installer et facile à utiliser.**

**Page de présentation et téléchargement sur le site Biblissima** : [http://outils.biblissima.fr/collatinus/](http://outils.biblissima.fr/collatinus/) (exécutables disponibles pour Mac OS, GNU/Linux et Windows).

Collatinus est à la fois un **lemmatiseur** et un **analyseur
morphologique de textes latins** : il est capable, si on
lui donne une forme déclinée ou conjuguée, de trouver
quel mot il faudra chercher dans le dictionnaire pour
avoir sa traduction dans une autre langue, ses
différents sens, et toutes les autres données que
fournit habituellement le dictionnaire.

En pratique, il est utile surtout au professeur de
latin, qui peut ainsi très rapidement, à partir d’un
texte hors-manuel, distribuer à ses élèves un texte
inédit avec son aide lexicale. Les élèves s’en servent
souvent pour lire plus facilement le latin lorsque
leurs connaissances lexicales et morphologiques sont
encore insuffisantes.

## Principales fonctionnalités

* Lemmatisation de mots latins ou d'un texte latin entier,
* Traduction des lemmes grâce aux dictionnaires de
  latin incorporés dans l'application,
* Affichage des quantités (durée longue ou brève des
  syllabes) et des flexions (déclinaison ou conjugaison).

## Historique

Collatinus était destiné, à l'origine, à produire des
documents sur papier, et c'est encore souvent dans ce
but qu'il est utilisé. Il a ensuite été perfectionné
pour satisfaire de nouveaux usages et besoins :

1. Disposer, lorsqu'on lit un texte latin, d'une aide
   lexicale et morphologique immédiate et discrète ;
2. Faire des recherches lexicales et stylistiques ;
3. Donner aux élèves des tâches d'identification, de
   relevé, de transformation.

## Principes de fonctionnement

Contrairement à la majorité des lemmatiseurs qui
utilisent une liste de formes fléchies, Collatinus
utilise un lexique contenant les lemmes et les
informations nécessaires pour leur flexion. L'avantage
est qu'avec 11 000 lemmes, Collatinus est capable de
reconnaître plus d'un demi-million de formes. L'ajout
du lemme correspondant à une variante orthographique
(médiévale, par exemple) permettrait également de
reconnaître toutes ses formes fléchies.

À partir du lemme et des désinences qui lui sont
associées, Collatinus peut aussi donner des tableaux de
flexion qui peuvent être utiles lors de l'apprentissage
du latin.

Enfin, lorsque les quantités sont connues pour le
lemme, Collatinus peut scander le mot et par là même
tout un texte. Lorsqu'il scande un texte, Collatinus
applique les règles habituelles d'allongement et
d'élision.

## Licence

Ce programme est mis à disposition par Yves Ouvrard et Philippe Verkerk sous licence [GNU GPL v3](http://www.gnu.org/licenses/gpl.html).
