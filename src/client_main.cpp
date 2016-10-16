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

