# Collatinus-11, BRANCHE SERVEUR

Octobre 2016

## Introduction

Collatinus, dans sa forme actuelle, se prête mal à une utilisation en ligne de commande,
parce qu'il doit charger beaucoup de données en mémoire au démarrage.
Une façon de contourner ce problème consiste à avoir le programme qui tourne
en tâche de fond (avec toutes les données chargées) et qu'en ligne de commande
on n'ait qu'un client qui va interroger un serveur. Cette architecture a déjà été
utilisée pour Collatinus-web où le démon avait été dérivé de la version standard.
L'interrogation venait directement du PHP qui gérait la page web.
L'idée est maintenant d'utiliser le même Collatinus 11 comme serveur.

Une autre façon de réduire le temps de chargement est de le limiter à l'essentiel.
On pourrait imaginer un Collatinus pour l'utilisation en ligne de commande
qui ne chargerait que les désinences en mémoire. Dans la décomposition radical+désinence,
une fois que la désinence est trouvée, le programme irait chercher dans un fichier
"radices.la" si le radical correspondant existe. Cela suppose une espèce de compilation
des fichiers actuels pour en tirer la liste des radicaux et la liste des formes 
singulières (irréguliers et formes canoniques sans désinence). Ces fichiers
seront probablement beaucoup moins lisibles que les actuels lemmes.*
mais ils pourraient en être tirés (c'est à dire que l'utilisateur continuerait
à éditer les lemmes.* ; un utilitaire bien fait en tirerait les fichiers nécessaires).
Cette approche est sûrement intéressante, mais nécessiterait une refonte
complète du code : peut-être pour une version 12…

Pour pouvoir converser avec le serveur, j'ai aussi écrit un client qui 
fonctionne en console. Il se contente de transmettre ses arguments
au serveur, de récupérer la réponse et de l'afficher.
La syntaxe ressemble à celle d'Unix. Par exemple :

./Client_C11 -lfr arma

## Syntaxe de la requête

La syntaxe est '[cmd] [texte]' ou '[cmd] -f nom_de_fichier'. 
Éventuellement complétée par '-o nom_de_fichier_de_sortie'.

Contrairement à l'usage Unix, on ne peut pas combiner plusieurs commandes.
Si on veut obtenir, par exemple, la scansion et la lemmatisation d'un texte,
il faudra envoyer deux requêtes distinctes.

Par défaut (sans cmd), on obtient la scansion du texte.

Les commandes possibles sont : 
- -s : Scansion du texte (-s1 : avec recherche des mètres).
- -a : Accentuation du texte (avec options -a0..-a15).
- -l : Lemmatisation du texte (avec options -l0..-l16).
- -h : Lemmatisation du texte en HTML (mêmes options que -l)
- -S, -A, -L, -H : Les mêmes avec Majuscules pertinentes.
- -t : Langue cible pour les traductions (par exemple -tfr, -tde).
- -C : Majuscules pertinentes.
- -c : Majuscules non-pertinentes.
- -? : Affichage de l'aide.

Avec l'option '-f nom_de_fichier', c'est le contenu du fichier qui est traité.
Comme le fichier est ouvert dans Collatinus, il vaut probablement mieux
donner le chemin complet du fichier. Par exemple :

/Users/Philippe/Documents/Virgile/Eneide1.txt

Avec l'option '-o nom_de_fichier_de_sortie', le résultat de l'opération est stocké
dans le fichier. Comme pour -f, il vaut mieux donner le chemin complet.

## Détails des commandes

Attention, les commandes sans texte (-c, -C et -t) modifient les paramètres
correspondants de l'application. L'affichage de l'aide n'affecte en rien l'application.
Si un texte suit ces commandes, il est ignoré.
- La langue cible : on peut changer la langue dans laquelle sont données les traductions
en spécifiant l'intitulé de la langue en deux caractères.
 * Par exemple -tfr permet d'obtenir les traductions en français.
Dans l'application, la langue-cible est également modifiée.
 * La commande nue, -t, ou avec un complément inconnu, -txx, donne la liste
des langues disponibles (et ne modifie pas la langue dans l'application).
- Majuscules pertinentes : on peut décider si Aeneas est équivalent à aeneas ou pas.
En début de phrase ou de vers, la majuscule initiale n'est pas considérée
comme pertinente.
 * -C ou -c1 : les majuscules sont respectées.
 * -c : les majuscules sont ignorées.

Les commandes avec texte modifient temporairement les paramètres de l'application,
mais les rétablissent à la fin du traitement.
Les commandes -s, -a, -l et -h ont leur contrepartie en majuscule (-S, -A, -L et -H) 
qui considèreront que les majuscules sont pertinentes.
- Scansion du texte : avec les commandes -s et -S, le texte est seulement scandé ; 
alors que les commandes -s1 et -S1 cherchent également les schémas métriques 
et font des statistiques. 
Attention, avec l'option 1, la réponse est en HTML avec des balises de mise en forme.
- Accentuation du texte : les commandes -a et -A peuvent être suivies d'un ou deux 
chiffres qui définissent les options d'accentuation. 
Les options de groupes différents s'ajoutent (OU binaire).
 * Le premier groupe d'option détermine le comportement à adopter si la pénultième
voyelle est commune. Ces options occupent les deux bits de poids faibles :
     * -a0 est équivalent à -s et retourne donc le texte scandé. 
Par exemple : ./Client_C11 -a0 tenebrae ==> tĕnē̆brāe
     * -a1 considère qu'une voyelle commune en pénultième position est accentuée.
Par exemple : ./Client_C11 -a1 tenebrae ==> tenébræ (paroxyton)
     * -a2 considère qu'une voyelle commune en pénultième position n'est pas accentuée
Par exemple : ./Client_C11 -a2 tenebrae ==> ténebræ (proparoxyton)
     * -a3 n'accentue pas les mots dont la pénultième est commune.
 * Le troisième bit (de valeur 4) permet la syllabisation des mots. 
Il est inactif si les deux premiers bits sont nuls.
     * -a5 ==> teˌnéˌbræ
     * -a6 ==> téˌneˌbræ
     * -a7 ==> teˌneˌbræ
 * Le quatrième bit (de valeur 8) introduit "l'exception illius" :
Le latin ecclésiastique considère en effet les voyelles communes comme brèves, 
sauf dans le cas des génitifs en -ī̆ŭs, dont le plus fréquent est illius. 
La "bonne option" pour le latin ecclésiastique serait donc -a10 qui donnera :
ténebræ (proparoxyton) mais illíus (paroxyton).
- Lemmatisation du texte : les commandes -l, -L, -h et -H retournent l'ensemble des 
lemmes du texte. -L et -H considèrent les majuscules pertinentes. 
Les commandes -l et -L retournent l'ensemble des lemmes en texte simple,
alors que -h et -H donne une réponse en HTML avec des balises de mise en forme.
Ces commandes peuvent être suivies d'une option numérique de 0 à 16 et 
de la langue dans laquelle seront données les traductions sur deux lettres.
Attention, l'ordre est imposé : la langue, si elle est donnée, vient en dernier.
L'option numérique peut être omise (0 est alors la valeur par défaut).
Tout comme la langue, auquel cas c'est la langue choisie dans l'appli qui
est prise par défaut. Pour connaître la liste des langues disponibles,
on utilisera la commande -t (voir ci-dessus).
Comme pour les options de l'accentuation, les options de la lemmatisation s'ajoutent.
 * Bit de poids faible (de valeur 1) : détermine l'ordre des lemmes
     * -l0 : les lemmes sont donnés dans l'ordre du texte.
     * -l1 : les lemmes sont donnés dans l'ordre alphabétique.
 * Deuxième bit (de valeur 2) : les formes du texte
     * -l2 : affiche les formes du texte, avant d'en donner la lemmatisation.
     * -l3 : les formes du texte sont rangées en ordre alphabétique et lemmatisées.
 * Troisième bit (de valeur 4) : l'analyse morphologique
     * -l4 : associe à chaque lemmatisation les différentes analyses possibles.
     * -l5..7 : combinaison avec les options précédentes.
 * Quatrième bit (de valeur 8) : regroupe les formes non-reconnues à la fin
     * -l8..15 : combinaison avec les options précédentes.
 * -l16 est la dernière option numérique possible et n'est pas combinable : 
elle évalue la fréquence des lemmes dans le texte.

