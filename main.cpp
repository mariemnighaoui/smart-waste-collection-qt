#include "wasterecycling.h"
#include "connection.h"
#include <QMessageBox>
#include <QFile>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // On récupère l'instance unique du singleton
    Connection& c = Connection::createInstance();

    // On tente d'établir la connexion
    bool test = c.createConnection();
    WasteRecycling w;
    //============================Test Connexion BD=======================================//
        //====================================================================================//
        if (test) {
        w.show();
        QMessageBox::information(
            nullptr,
            QObject::tr("Base de données"),
            QObject::tr("Connexion réussie.\nCliquez sur Annuler pour quitter."),
            QMessageBox::Cancel
            );
    } else {
        QMessageBox::critical(
            nullptr,
            QObject::tr("Base de données"),
            QObject::tr("Échec de la connexion.\nCliquez sur Annuler pour quitter."),
            QMessageBox::Cancel
            );
    }

    return a.exec();
}
