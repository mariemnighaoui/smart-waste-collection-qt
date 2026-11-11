#include "wasterecycling.h"
#include "connection.h"
#include <QDebug>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
Connection::Connection() {
    db = QSqlDatabase::addDatabase("QODBC");
}

Connection::~Connection() {
    if (db.isOpen()) {
        db.close(); // Libère proprement les ressources
    }
}

Connection& Connection::createInstance() {
    static Connection instance; // Instance unique du Singleton
    return instance; // Retourne une référence à cette instance
}

bool Connection::createConnection() {

    db.setDatabaseName(
        "DRIVER={Oracle in XE};"
        "DBQ=localhost:1521/XE;"
        "UID=YOUR_DB_USER;"
        "PWD=YOUR_DB_PASSWORD;"
        );

    if (db.open()) {
        qDebug() << "Connexion établie";
        return true;
    }

    qDebug() << "Tentative 1 échouée :" << db.lastError().text();

    db.setDatabaseName("Source_Projet2A");
    db.setUserName("YOUR_DB_USER");
    db.setPassword("YOUR_DB_PASSWORD");

    if (db.open()) {
        qDebug() << "Connexion établie";
        return true;
    }

    qDebug() << "Échec de la connexion :" << db.lastError().text();
    return false;
}
