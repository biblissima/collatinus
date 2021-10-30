/*               maj.h
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

#ifndef MAJ_H
#define MAJ_H


#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include <QDialog>
#include <QLabel>
#include <QStringList>

#include <QDebug>

/**
 * @brief La classe Maj permet de faire les mises à jour des lexiques et des dictionnaires
 */
class Maj : public QDialog
{
    Q_OBJECT

   private:
    bool installe(
        QString nfcol);  // décompresse nfcol et l'installe dans data/dicos/
    bool djvu2col(QString nfdjvu);
    // Fonction pour générer un .col à partir d'un djvu dans data/dicos/
    bool lem2col(QString nfLem);
    // Fonction pour générer un .col à partir d'un fichier lemmes.* ou lem_ext.*
    QLabel *label;
    QStringList listeF;  // liste des fichiers téléchargés
    bool _dico; // Mise à jour des dicos ou des lexiques (si faux)

   private slots:
    void selectionne();  // lance un dialogue de sélection de fichier

   public:
    Maj(bool dic = true, QDialog *parent = 0);
    void setFont(QFont font);
};

#endif
