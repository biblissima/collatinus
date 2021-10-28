/**
 * \file client_main.cpp
 * \brief petit utilitaire pour interroger le serveur de Collatinus
 * \author Philippe Verkerk
 * \date 2016
 * 
 * Le client C11 est un petit utilitaire en ligne de commande
 * qui permet de transférer des requêtes à Collatinus par son
 * serveur sur le port interne 5555.
 * Initialement, c'était pour tester le fonctionnement du serveur.
 * Depuis, ce programme est utilisé comme relais pour transmettre
 * des requêtes des macros dans Libre Office à Collatinus.
 * Il pourrait aussi servir dans un _pipe_.
 * 
 * L'exécutable qui en est tiré est intégré à la distribution de
 * Collatinus pour Windows et MacOS.
 *
 */

#include <QCoreApplication>
#include <iostream>
#include <QtWidgets>
#include <QtNetwork>

class QTcpSocket;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QString req = "";
    if (argc > 1)
    {
        int i = 1;
        while (i < argc)
        {
            QString suite(argv[i]);
            req += " " + suite;
            i++;
        }
    }
    else req = "-?"; // pour afficher l'aide.

    QTcpSocket * tcpSocket = new QTcpSocket();
    tcpSocket->abort();
    tcpSocket->connectToHost(QHostAddress::LocalHost, 5555);
    QByteArray ba = req.toUtf8();
    tcpSocket->write(ba);
    tcpSocket->waitForBytesWritten();
    tcpSocket->waitForReadyRead();
    ba = tcpSocket->readAll();
    tcpSocket->disconnectFromHost();
    tcpSocket->close();
    QString rep(ba);
    std::cout << rep.toStdString();

    a.quit();
}

