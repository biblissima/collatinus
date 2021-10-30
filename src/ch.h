/*       ch.h     */

#ifndef CH_H
#define CH_H

#include <QRegExp>
#include <QString>
#include <QStringList>

/**
 * \brief constantes et utilitaires de traitement
 *        des chaînes de caractères
 *
 */
namespace Ch
{
/* Je passe à une liste d'abréviations contenue dans un fichier.
QStringList const abrev  = QStringList()
    <<"Agr"<<"Ap"<<"A"<<"K"<<"D"<<"F"<<"C"
    <<"Cn"<<"Kal"<<"L"<<"Mam"<<"M\""<<"M"<<"N"<<"Oct"
    <<"Opet"<<"Post"<<"Pro"<<"P"<<"Q"<<"Sert"
    <<"Ser"<<"Sex"<<"S"<<"St"<<"Ti"<<"T"<<"V"
    <<"Vol"<<"Vop"<<"Pl";
// Liste des abréviations prise dans Praelector le 11/11/2016
*/
QStringList ajoute(QString mot, QStringList liste);
void allonge(QString *f);
QString atone(const QString a, bool bdc = false);
QString communes(QString g);
void deQuant(QString *c);
/*! Chaine donnant les consonnes */
QString const consonnes = "bcdfgjklmnpqrstvwxz";
void genStrNum(const QString s, QString *ch, int *n);
QString deramise(QString r);
QString deAccent(QString c);
void elide(QString *mp);
/*! Expression rationnelle pour un ensemble de lettres (avec sélection) */
const QRegExp reAlphas("(\\w+)");
/*! Expression rationnelle pour un ensemble d'espaces */
const QRegExp reEspace("\\s+");
/*! Expression rationnelle pour une lettre (sans sélection) */
const QRegExp reLettres("\\w");
/*! Expression rationnelle pour une ponctuation (avec sélection) */
const QRegExp rePonct("([\\.?!;:])");
// const QRegExp rePonct("([\\.?!;:]|$$)");
bool sort_i(const QString &a, const QString &b);
bool inv_sort_i(const QString &a, const QString &b);
QString versPC(QString k);
QString versPedeCerto(QString k);
/*! Chaine donnant les voyelles avec macron ou breve */
QString const voyelles = "āăēĕīĭōŏūŭȳўĀĂĒĔĪĬŌŎŪŬȲЎ";
QChar breve(QChar c);

//QChar const separSyll = 0x02CC;
//QChar const separSyll = 0x00AD;
/*! Caractère utilisé pour séparer les syllabes (ici un _MIDDLE DOT_ : ·) */
QChar const separSyll = 0x00B7;
QString transforme(QString k);
QString accentue(QString l);
QString ajoutSuff(QString fq, QString suffixe, QString l_etym, int accent);
}
#endif
