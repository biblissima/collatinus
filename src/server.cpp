/*   server.cpp
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
 *
 * Je supprime tout ce qui était lié aux fenêtres pour ne garder que le serveur.
 * Philippe Janvier 2019.
 *
 */

#include "server.h"

/**
 * \fn Server::Server()
 * \brief Créateur du serveur de l'appli.
 */
Server::Server()
{

    QString resDir = "/usr/share/collatinus";
    resDir += "/data/";
    _lemCore = new LemCore(this, resDir);
    _lemmatiseur = new Lemmatiseur(this,_lemCore,"",resDir);
    flechisseur = new Flexion(_lemCore);
    lasla = new Lasla(this,_lemCore,resDir);
    tagueur = new Tagueur(this,_lemCore,"",resDir);
    scandeur = new Scandeur(this,_lemCore,resDir);

    _lemCore->setExtension(true); // Je charge l'extension du lexique,
    _lemCore->setExtension(false); // mais je ne l'utilise que comme réserve.
    // Ainsi, n'apparaissent les analyses issues de lextension que
    // si elles sont les seules possibles.

    createDicos();

    // Lancement du server
    serveur = new QTcpServer (this);
    connect (serveur, SIGNAL(newConnection()), this, SLOT (connexion ()));
    if (!serveur->listen (QHostAddress::LocalHost, 5555))
    {
        std::cout << "Ne peux écouter.\n";
        return;
    }
    std::cout << "Collatinus 11.2 est lancé en console. Ctrl-C pour l'arrêter.\n";
}

/**
 * \fn void Server::createDicos(bool prim)
 * \brief Chargement des index et des fichiers de
 *        configuration des dictionnaires.
 */
void Server::createDicos(bool prim)
{
    QDir chDicos(qApp->applicationDirPath() + "/dicos");
    QStringList lcfg = chDicos.entryList(QStringList() << "*.cfg");
    ldic.clear();
    foreach (QString fcfg, lcfg)
    {
        Dictionnaire *d = new Dictionnaire(fcfg);
        listeD.ajoute(d);
        ldic << d->nom();
        std::cout << d->nom().toStdString() << "\n";
    }
}

QString Server::lem2csv(QString texte)
{
    QString res;
    QString forme;
    QString ligne;
    QString trad;
    QString fc;
    int level;
    int nbOcc;
    int nn = 0;
    int pos;
    QString f1 = "%1\t%2\t"; // Début de ligne avec deux champs numériques
    QString f2 = "\"%1\"\t\"%2\"\t\"%3\"\t%4\n"; // Fin de ligne avec quatre champs
    while (texte.contains("\n"))
    {
        pos = texte.indexOf("\n");
        ligne = texte.mid(0,pos).simplified();
        texte = texte.mid(pos + 1);
//        qDebug() << ligne << texte;
        if (ligne.startsWith("*"))
        {
            forme = ligne.mid(2);
            nn += 1;
        }
        else if (ligne.startsWith("-"))
        {
            ligne = ligne.mid(2);
            fc = ligne.section(":",0,0).trimmed();
            trad =ligne.section(":",1).trimmed();
            if (fc.contains("("))
            {
                fc.chop(1);
                nbOcc = fc.section("(",1).toInt();
                fc = fc.section("(",0,0).trimmed();
            }
            else nbOcc = 0;
            if (nbOcc < 500) level = 3;
            else if (nbOcc < 5000) level = 2;
            else level = 1;
            res.append(f1.arg(nn).arg(level));
            res.append(f2.arg(forme).arg(fc).arg(trad).arg(nbOcc));
        }
        else if (ligne.startsWith(">"))
        {
            nn += 1;
            ligne = ligne.mid(2);
            forme = ligne.section(" ",0,0);
            res.append(f1.arg(nn).arg(3));
            res.append(f2.arg(forme).arg("unknown").arg("").arg(""));
        }
    }
    return res;
}

void Server::connexion ()
{
    soquette = serveur->nextPendingConnection ();
    connect (soquette, SIGNAL (readyRead ()), this, SLOT (exec ()));
}

void Server::exec ()
{
    QByteArray octets = soquette->readAll ();
    QString requete = QString (octets).trimmed();
    std::cout << requete.toStdString() << "\n";
    if (requete.isEmpty()) requete = "-?";
    QString texte = "";
    QString rep = "";
    bool nonHTML = true;
    QString fichierSortie = "";
    if (requete.contains("-o "))
    {
        // La requête contient un nom de fichier de sortie
        QString nom = requete.section("-o ",1,1).trimmed();
        requete = requete.section("-o ",0,0).trimmed();
        // En principe, le -o vient à la fin. Mais...
        if (nom.contains(" "))
        {
            fichierSortie = nom.section(" ",0,0);
            // Le nom de fichier ne peut pas contenir d'espace !
            if (requete.isEmpty()) requete = nom.section(" ",1);
            else requete.append(" " + nom.section(" ",1));
        }
        else fichierSortie = nom;
    }
    if (requete.contains("-f "))
    {
        // La requête contient un nom de fichier
        QString nom = requete.section("-f ",1,1).trimmed();
        requete = requete.section("-f ",0,0).trimmed();
        QFile fichier(nom);
        if (fichier.open(QFile::ReadOnly))
        {
            texte = fichier.readAll();
            fichier.close();
        }
        else rep = "fichier non trouvé !\n";
    }
    if (rep == "")
    {
    if ((requete[0] == '-') && (requete.size() > 1))
    {
        char a = requete[1].toLatin1();
        QString options = requete.mid(0,requete.indexOf(" "));
        QString lang = _lemmatiseur->cible(); // La langue actuelle;
        bool html = _lemmatiseur->optHtml(); // L'option HTML actuelle
        bool MP = _lemmatiseur->optMajPert();
        _lemmatiseur->setHtml(false); // Sans HTML, a priori
        int optAcc = 0;
        if (texte == "")
            texte = requete.mid(requete.indexOf(" ")+1);
        _lemmatiseur->setMajPert(requete[1].isUpper());
        switch (a)
        {
        case 'S':
        case 's':
            if ((options.size() > 2) && (options[2].isDigit()))
                optAcc = options[2].digitValue() & 7;
            rep = scandeur->scandeTxt(texte,0,optAcc==1, requete[1].isLower());
            if (optAcc==1) nonHTML = false;
            nonHTML = false;
//            rep.replace("\n","<br />\n");
            break;
        case 'A':
        case 'a':
            optAcc = 3; // Par défaut : un mot dont la pénultième est commune n'est pas accentué.
            if ((options.size() > 2) && (options[2].isDigit()))
            {
                optAcc = options[2].digitValue();
                if ((options.size() > 3) && (options[3].isDigit()))
                    optAcc = 10 * optAcc + options[3].digitValue();
            }
            rep = scandeur->scandeTxt(texte,optAcc,false, requete[1].isLower());
            nonHTML = false;
            rep.replace("\n","<br />\n");
            break;
        case 'H':
        case 'h':
            _lemmatiseur->setHtml(true);
            nonHTML = false;
        case 'L':
        case 'l':
            if ((options.size() > 2) && (options[2].isDigit()))
            {
                optAcc = options[2].digitValue();
                options = options.mid(3);
                if ((options.size() > 0) && (options[0].isDigit()))
                {
                    optAcc = 10*optAcc+options[0].digitValue();
                    options = options.mid(1);
                }
            }
            else options = options.mid(2); // Je coupe le "-l".
            if ((options.size() == 2) && _lemCore->cibles().keys().contains(options))
                _lemmatiseur->setCible(options);
            else if (((options.size() == 5) || (options.size() == 8)) && _lemCore->cibles().keys().contains(options.mid(0,2)))
                _lemmatiseur->setCible(options);
            if (optAcc > 15) rep = _lemmatiseur->frequences(texte).join("");
            else rep = _lemmatiseur->lemmatiseT(texte,optAcc&1,optAcc&2,optAcc&4,optAcc&8);
            rep.replace("</h4>","</big></strong>");
            rep.replace("<h4>","<strong><big>");
            _lemmatiseur->setCible(lang); // Je rétablis les langue et option HTML.
            break;
        case 'P':
        case 'p':
            // Appel au tagueur probabiliste.
            // Ici, optAcc vaut 0 : n'affiche que la meilleure solution.
            // Si je veux changer le comportement par défaut, il faut ajouter une ligne :
            // optAcc = 1;
            if ((options.size() > 2) && (options[2].isDigit()))
            {
                optAcc = options[2].digitValue();
                options = options.mid(3);
            }
            else options = options.mid(2); // Je coupe le "-l".
            if ((options.size() == 2) && _lemCore->cibles().keys().contains(options))
                _lemmatiseur->setCible(options);
            else if (((options.size() == 5) || (options.size() == 8)) &&
                     _lemCore->cibles().keys().contains(options.mid(0,2)))
                _lemmatiseur->setCible(options);
            rep = tagueur->tagTexte(texte, -1, (optAcc & 1), requete[1].isUpper(), !(optAcc & 2));
            // Par défaut, je tague tout le texte.
            _lemmatiseur->setCible(lang); // Je rétablis les langue et option HTML.
            nonHTML = false;
            // Le résultat est en html : je veux conserver les <br/>.
            break;
        case 'E':
        case 'e':
            // Pour sortir la lemmatisation sous un format CSV
            options = options.mid(2); // Je coupe le "-e".
            if ((options.size() == 2) && _lemCore->cibles().keys().contains(options))
                _lemmatiseur->setCible(options);
            else if (((options.size() == 5) || (options.size() == 8)) && _lemCore->cibles().keys().contains(options.mid(0,2)))
                _lemmatiseur->setCible(options);
            rep = lem2csv(_lemmatiseur->lemmatiseT(texte,false,true,false,false));
//            if (options.startsWith("dc")) rep.replace(":","\"\t\"");
            _lemmatiseur->setCible(lang); // Je rétablis les langue et option HTML.
            break;
        case 'X':
        case 'x':
//            rep = _lemCore->txt2XML(requete);
            rep = "Pas encore disponible";
            break;
        case 'K':
        case 'k':
            rep = lasla->k9(texte);
            break;
        case 'c':
            if (options.size() > 2)
                _lemmatiseur->setMajPert(options[2] == '1');
            break;
        case 'C':
                _lemmatiseur->setMajPert(true);
            break;
        case 'F':
            // Pour la flexion.
            rep = flechis(texte);
            break;
        case 'd':
            rep = consult(options, texte);
            nonHTML = false;
            break;
        case 't':
            options = options.mid(2); // Je coupe le "-t".
            if (((options.size() == 2) || (options.size() == 5) || (options.size() == 8)) &&
                    _lemCore->cibles().keys().contains(options.mid(0,2)))
            {
                _lemmatiseur->setCible(options);
            }
            else
            {
                QStringList clefs = _lemCore->cibles().keys();
                rep = "Les langues connues sont : " + clefs.join(" ") + "\n";
            }
            break;
//        case '?':
        default: // Tout caractère non-affecté affiche l'aide.
            rep = "La syntaxe est '[commande] [texte]' ou '[commande] -f nom_de_fichier'.\n";
            rep += "Éventuellement complétée par '-o nom_de_fichier_de_sortie'.\n";
            rep += "Par défaut (sans commande), on obtient la scansion du texte.\n";
            rep += "Les commandes possibles sont : \n";
            rep += "\t-s : Scansion du texte (-s1 : avec recherche des mètres).\n";
            rep += "\t-a : Accentuation du texte (avec options -a1..-a15).\n";
            rep += "\t-l : Lemmatisation du texte (avec options -l0..-l15, -l16 pour les fréquences).\n";
            rep += "\t-h : Lemmatisation du texte en HTML (mêmes options que -l).\n";
            rep += "\t-e : Lemmatisation du texte en CSV, sans option sauf la langue cible.\n";
            rep += "\t-S, -A, -L, -H, -E : Les mêmes avec Majuscules pertinentes.\n";
            rep += "\t-t : Langue cible pour les traductions (par exemple -tfr, -ten).\n";
            rep += "\t-C : Majuscules pertinentes.\n";
            rep += "\t-c : Majuscules non-pertinentes.\n";
            rep += "\t-? : Affichage de l'aide.\n";
 //           rep += "\t-x : Mise en XML du texte.\n";
            break;
        }
        _lemmatiseur->setHtml(html);
        if ((a != 'C') && (a != 'c'))
            _lemmatiseur->setMajPert(MP);
    }
    // sinon scande le texte
    //else if (texte != "") rep= scandeur->scandeTxt(texte);
    //else rep= scandeur->scandeTxt(requete);
    // fallback remplacé par un message d'alerte, invitant l'utilisateur à recharger la page
    else rep= "<div class=\"alert alert-danger\" role=\"alert\"><strong>Collatinus-web a été mis à jour !</strong> Veuillez recharger cette page pour bénéficier des dernières améliorations <button type=\"button\" role=\"button\" class=\"btn btn-primary\" onclick=\"location.reload(true)\"><span class=\"glyphicon glyphicon-refresh\"></span>&nbsp; Recharger la page</button></div>";
    }
/*    if (nonHTML)
    {
        rep.remove("<br />"); // Avec -H/h, j'ai la lemmatisation en HTML
        rep.remove("<br/>"); // Avec -H/h, j'ai la lemmatisation en HTML
    }
*/
    QByteArray ba = rep.toUtf8();
//    std::cout << "Je retourne la page.\n";
    soquette->write(ba);
    soquette->close ();
}

QString Server::flechis(QString texte)
{
    if (texte.isEmpty()) return "Erreur !";
    if (texte.contains(" ")) texte = texte.section(" ",0,0);
    // Un seul mot !
    MapLem ml = _lemCore->lemmatiseM(texte);
    if (ml.isEmpty()) return "";
        else return flechisseur->tableaux(&ml);
}

QString Server::consult(QString req, QString texte)
{
    if (texte.isEmpty()) return "Erreur !";
    std::cout << req.toStdString() << " " << texte.toStdString() << "\n";
    // D'abord le dico.
    Dictionnaire * dico_courant;
    QString p = req.mid(2);
    if (p == "fg")
    {
        dico_courant = listeD.dictionnaire_par_nom("Gaffiot 1934");
    }
    else dico_courant = listeD.dico_par_abr(p);
        /*if (p == "dle")
    {
        dico_courant = listeD.dictionnaire_par_nom("Lewis and Short 1879");
    }
    else if (p == "dga")
    {
        dico_courant = listeD.dictionnaire_par_nom("Gaffiot 2016");
    }
    else if (p == "ddu")
    {
        dico_courant = listeD.dictionnaire_par_nom("du Cange 1883");
    }
    else if (p == "dje")
    {
        dico_courant = listeD.dictionnaire_par_nom("Jeanneau 2017");
    }
    else return "Dico inconnu";
    */
    if (dico_courant == NULL) return "Dico inconnu";
    // puis le mot.
    QStringList lemmes;
    if (texte.contains(" ")) texte = texte.section(" ",0,0);
    // Un seul mot !
    if (texte.startsWith("@"))
    {
        lemmes << texte.mid(1);
    }
    else
    {
        MapLem ml = _lemCore->lemmatiseM(texte);
        if (ml.isEmpty()) lemmes << texte;
        else lemmes = _lemCore->lemmes(ml);
    }
    if (dico_courant->estXml())
        return dico_courant->pageXml(lemmes);
    // S'il n'est pas en XML, c'est une page png
    return dico_courant->pagePng(lemmes);
}
