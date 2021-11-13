/*               lasla.h
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

#ifndef LASLA_H
#define LASLA_H

#include "lemCore.h"
#include "lemme.h"
#include "ch.h"

/**
 * @brief La classe Lasla introduit des fonctions pour la première version
 * du tagueur LASLA-Tagger.
 *
 * Probablement inutile depuis que la deuxième version de ce tagueur
 * intègre cette partie de Collatinus.
 * Ces fonctions étaient appelées exclusivement par le serveur MainWindow::exec.
 */
class Lasla : public QObject
{
//    Q_OBJECT

public:
    Lasla(QObject *parent = 0, LemCore *l=0, QString resDir="");
    // Créateur de la classe
    QString k9(QString m);
    // Code en 9 pour le LASLA

private:
    /*! Un pointeur vers le noyau de lemmatisation qui peut être partagé. */
    LemCore * _lemCore;
    /*! Le nom du répertoire contenant les données. */
    QString _resDir;
    QMap<QString,QString> _catLasla;
    /*!< QMap avec les correspondances entre les modèles de Collatinus et les catégories du LASLA */
    void lisCat();
    // Pour la lecture du fichier.
};

#endif // LASLA_H
