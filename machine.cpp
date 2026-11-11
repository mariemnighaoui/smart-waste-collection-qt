#include "machine.h"
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <QSqlDatabase>

Machine::Machine() {
    id_machine = 0;
    capacite = 0;
    type_dechet = "";
    nom_machine = "";
    date_ajout = QDateTime::currentDateTime();
}

Machine::Machine(int id, QString type, QString nom, int capacite, QDateTime date) {
    this->id_machine = id;
    this->type_dechet = type;
    this->nom_machine = nom;
    this->capacite = capacite;
    this->date_ajout = date;
}

bool Machine::ajouter()
{
    qDebug() << "=== DÉBUT AJOUT MACHINE ===";
    qDebug() << "Type déchet :" << type_dechet;
    qDebug() << "Nom machine :" << nom_machine;
    qDebug() << "Capacité    :" << capacite;
    qDebug() << "Date ajout  :" << date_ajout.toString("yyyy-MM-dd HH:mm:ss");

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qDebug() << "Connexion non ouverte !" << db.lastError().text();
        return false;
    }

    QSqlQuery query(db);
    query.prepare(
        "INSERT INTO MACHINES "
        "(TYPE_DECHET, NOM_MACHINE, CAPACITE, DATE_AJOUT) "
        "VALUES (:type, :nom, :capacite, :date)"
        );

    query.bindValue(":type", type_dechet);
    query.bindValue(":nom", nom_machine);
    query.bindValue(":capacite", capacite);
    query.bindValue(":date", date_ajout);

    if (!query.exec()) {
        qDebug() << "Erreur ajout :" << query.lastError().text();
        qDebug() << "SQL         :" << query.lastQuery();
        return false;
    }

    qDebug() << "Machine ajoutée avec succès";
    return true;
}

QSqlQueryModel* Machine::afficher()
{
    QSqlQueryModel* modelmach = new QSqlQueryModel();

    QSqlQuery query;
    query.prepare(
        "SELECT ID_MACHINE, TYPE_DECHET, NOM_MACHINE, CAPACITE, "
        "TO_CHAR(DATE_AJOUT, 'DD/MM/YYYY HH24:MI:SS') AS DATE_AJOUT "
        "FROM MACHINES "
        "ORDER BY DATE_AJOUT DESC"
        );

    if (!query.exec()) {
        qDebug() << "Erreur affichage :" << query.lastError().text();
        return modelmach;
    }

    modelmach->setQuery(std::move(query));  // ← Utiliser std::move

    modelmach->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    modelmach->setHeaderData(1, Qt::Horizontal, QObject::tr("Type déchet"));
    modelmach->setHeaderData(2, Qt::Horizontal, QObject::tr("Nom machine"));
    modelmach->setHeaderData(3, Qt::Horizontal, QObject::tr("Capacité"));
    modelmach->setHeaderData(4, Qt::Horizontal, QObject::tr("Date ajout"));

    return modelmach;
}

bool Machine::modifier(int id)
{
    qDebug() << "=== MODIFICATION MACHINE ID =" << id << "===";

    QSqlQuery query;
    query.prepare(
        "UPDATE MACHINES SET "
        "TYPE_DECHET = :type, "
        "NOM_MACHINE = :nom, "
        "CAPACITE = :capacite "
        "WHERE ID_MACHINE = :id"
        );

    query.bindValue(":type", type_dechet);
    query.bindValue(":nom", nom_machine);
    query.bindValue(":capacite", capacite);
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Erreur modification :" << query.lastError().text();
        return false;
    }

    qDebug() << "Machine modifiée avec succès";
    return true;
}

bool Machine::supprimer(int id)
{
    qDebug() << "=== SUPPRESSION MACHINE ID =" << id << "===";

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qDebug() << "Connexion non ouverte !" << db.lastError().text();
        return false;
    }

    QSqlQuery query(db);
    query.prepare("DELETE FROM MACHINES WHERE ID_MACHINE = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Erreur suppression :" << query.lastError().text();
        return false;
    }

    qDebug() << "Machine supprimée avec succès";
    return true;
}

QSqlQueryModel* Machine::trierParType(QString type)
{
    QSqlQueryModel* model = new QSqlQueryModel();

    QSqlQuery query;

    if (type.isEmpty() || type == "Tous") {
        query.prepare(
            "SELECT ID_MACHINE, TYPE_DECHET, NOM_MACHINE, CAPACITE, "
            "TO_CHAR(DATE_AJOUT, 'DD/MM/YYYY HH24:MI:SS') AS DATE_AJOUT "
            "FROM MACHINES "
            "ORDER BY TYPE_DECHET ASC"  // ← Tri alphabétique par type
            );
    } else {
        query.prepare(
            "SELECT ID_MACHINE, TYPE_DECHET, NOM_MACHINE, CAPACITE, "
            "TO_CHAR(DATE_AJOUT, 'DD/MM/YYYY HH24:MI:SS') AS DATE_AJOUT "
            "FROM MACHINES "
            "WHERE TYPE_DECHET = :type "
            "ORDER BY TYPE_DECHET ASC"  // ← Tri alphabétique
            );
        query.bindValue(":type", type);
    }

    if (!query.exec()) {
        qDebug() << "Erreur tri :" << query.lastError().text();
        return model;
    }

    model->setQuery(std::move(query));

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Type déchet"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Nom machine"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Capacité"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Date ajout"));

    return model;
}

QSqlQueryModel* Machine::rechercher(QString nom)
{
    QSqlQueryModel* model = new QSqlQueryModel();

    QSqlQuery query;
    query.prepare(
        "SELECT ID_MACHINE, TYPE_DECHET, NOM_MACHINE, CAPACITE, "
        "TO_CHAR(DATE_AJOUT, 'DD/MM/YYYY HH24:MI:SS') AS DATE_AJOUT "
        "FROM MACHINES "
        "WHERE NOM_MACHINE LIKE :nom "
        "ORDER BY DATE_AJOUT DESC"
        );
    query.bindValue(":nom", "%" + nom + "%");

    if (!query.exec()) {
        qDebug() << "Erreur recherche :" << query.lastError().text();
        return model;
    }

    model->setQuery(std::move(query));

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Type déchet"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Nom machine"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Capacité"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Date ajout"));

    return model;
}
