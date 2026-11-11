#include "point_de_collecte.h"
#include <QDate>


PointCollecte::PointCollecte()
{
    id_pc = 0;
    niveau = 0;
}

PointCollecte::PointCollecte(int id, QString z, QString a, int n, QString d, QString t)
{
    id_pc = id;
    zone = z;
    adresse = a;
    niveau = n;
    derniere_collecte = d;
    type_zone = t;
}

bool PointCollecte::ajouter()
{
    QSqlQuery query;
    query.prepare("INSERT INTO POINTS_DE_COLLECTE "
                  "(ZONE, ADRESSE, NIVEAU, DERNIERE_COLLECTE, TYPE_ZONE) "
                  "VALUES (:zone, :adresse, :niveau, TRUNC(TO_DATE(:date_collecte, 'YYYY-MM-DD')), :type_zone)");

    query.bindValue(":zone", zone);
    query.bindValue(":adresse", adresse);
    query.bindValue(":niveau", niveau);
    query.bindValue(":date_collecte", derniere_collecte);
    query.bindValue(":type_zone", type_zone);
    if(adresse.isEmpty()){
        QMessageBox::warning(nullptr, "Erreur", "tous les champs doivent être complétés !");
        return false;
    }if(adresse.length()<4){
        QMessageBox::warning(nullptr, "Erreur", "l'adresse est courte !");
        return false;
    }
    QDate dateSaisie = QDate::fromString(derniere_collecte, "yyyy-MM-dd");
    // Obtenir la date actuelle
    QDate dateActuelle = QDate::currentDate();

    if(dateSaisie > dateActuelle){
        QMessageBox::warning(nullptr, "Erreur", "La date ne peut pas être postérieure à aujourd'hui !\n"
                                                "Date maximum autorisée : " + dateActuelle.toString("yyyy-MM-dd"));
        return false;

    }else{
        return query.exec();
    }

}

QSqlQueryModel* PointCollecte::afficher()
{
    QSqlQueryModel* model = new QSqlQueryModel();

    model->setQuery(
        "SELECT ID_PC, ZONE, ADRESSE, NIVEAU, "
        "TO_CHAR(DERNIERE_COLLECTE, 'DD/MM/YYYY') AS DERNIERE_COLLECTE, "
        "TYPE_ZONE "
        "FROM POINTS_DE_COLLECTE"
        );

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Zone"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Adresse"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Niveau"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Dernière Collecte"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Type Zone"));

    return model;
}

bool PointCollecte::supprimer(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM POINTS_DE_COLLECTE WHERE ID_PC = :id");
    query.bindValue(":id", id);

    return query.exec();
}

bool PointCollecte::modifier(int id)
{
    QSqlQuery query;

    query.prepare("UPDATE POINTS_DE_COLLECTE SET "
                  "ZONE = :zone, "
                  "ADRESSE = :adresse, "
                  "NIVEAU = :niveau, "
                  "DERNIERE_COLLECTE = TRUNC(TO_DATE(:date_collecte, 'YYYY-MM-DD')), "
                  "TYPE_ZONE = :type_zone "
                  "WHERE ID_PC = :id");

    query.bindValue(":zone", zone);
    query.bindValue(":adresse", adresse);
    query.bindValue(":niveau", niveau);
    query.bindValue(":date_collecte", derniere_collecte);
    query.bindValue(":type_zone", type_zone);
    query.bindValue(":id", id);
    if(adresse.isEmpty()){
        QMessageBox::warning(nullptr, "Erreur", "tous les champs doivent être complétés !");
        return false;
    }if(adresse.length()<4){
        QMessageBox::warning(nullptr, "Erreur", "l'adresse est courte !");
        return false;
    }
    QDate dateSaisie = QDate::fromString(derniere_collecte, "yyyy-MM-dd");
    // Obtenir la date actuelle
    QDate dateActuelle = QDate::currentDate();

    if(dateSaisie > dateActuelle){
        QMessageBox::warning(nullptr, "Erreur", "La date ne peut pas être postérieure à aujourd'hui !\n"
                                                "Date maximum autorisée : " + dateActuelle.toString("yyyy-MM-dd"));
        return false;

    }else{
        return query.exec();
    }
}


///////recherche
QSqlQueryModel* PointCollecte::rechercher(QString texte)
{
    QSqlQueryModel* model = new QSqlQueryModel();

    QSqlQuery query;
    query.prepare(
        "SELECT ID_PC, ZONE, ADRESSE, NIVEAU, "
        "TO_CHAR(DERNIERE_COLLECTE, 'DD/MM/YYYY') AS DERNIERE_COLLECTE, "
        "TYPE_ZONE "
        "FROM POINTS_DE_COLLECTE "
        "WHERE LOWER(ZONE) LIKE LOWER(:texte) "
        "OR LOWER(ADRESSE) LIKE LOWER(:texte)"
        );

    query.bindValue(":texte", "%" + texte + "%");
    query.exec();

    model->setQuery(std::move(query));

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Zone"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Adresse"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Niveau"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Dernière Collecte"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Type Zone"));

    return model;
}
