#include "conteneur.h"

// Constructors
Conteneur::Conteneur()
{
    id_conteneur = 0;
    capacite_max = 0;
    quantite_actuelle = 0;
    seuil_alerte = 0;
    date_maj = QDate::currentDate();
}

Conteneur::Conteneur(int id, QString t, int cap, int qte, int seuil, QDate date)
{
    this->id_conteneur = id;
    this->type = t;
    this->capacite_max = cap;
    this->quantite_actuelle = qte;
    this->seuil_alerte = seuil;
    this->date_maj = date;
}

// Calculated fields
int Conteneur::getPourcentageRemplissage() const
{
    if (capacite_max <= 0) return 0;
    return (quantite_actuelle * 100) / capacite_max;
}

QString Conteneur::getNiveauAlerte() const
{
    int pourcentage = getPourcentageRemplissage();
    if (pourcentage >= 90) {
        return "Critique";
    } else if (pourcentage >= seuil_alerte) {
        return "Avertissement";
    } else {
        return "Normal";
    }
}

// Vérifier si un ID existe déjà
bool Conteneur::idExiste(int id)
{
    Connection& conn = Connection::createInstance();
    QSqlDatabase db = conn.getDatabase();

    if (!db.isOpen()) {
        qDebug() << "Database is not open!";
        return false;
    }

    QSqlQuery query(db);
    // FIXED: Changed CONTENEUR to CONTENEURS
    query.prepare("SELECT COUNT(*) FROM CONTENEURS WHERE ID_CONTENEUR = :id");
    query.bindValue(":id", id);

    if (query.exec()) {
        if (query.next()) {
            return query.value(0).toInt() > 0;
        }
    } else {
        qDebug() << "Error in idExiste:" << query.lastError().text();
    }

    return false;
}

// CREATE - AJOUT (PDF page 33)
bool Conteneur::ajouter()
{
    Connection& conn = Connection::createInstance();
    QSqlDatabase db = conn.getDatabase();

    if (!db.isOpen()) {
        qDebug() << "Database is not open!";
        return false;
    }

    QSqlQuery query(db);
    // FIXED: Changed CONTENEUR to CONTENEURS
    query.prepare("INSERT INTO CONTENEURS (ID_CONTENEUR, TYPE, CAPACITE_MAX, QUANTITE_ACTUELLE, SEUIL_ALERTE, DATE_MAJ) "
                  "VALUES (:id, :type, :capacite, :quantite, :seuil, :date)");

    query.bindValue(":id", id_conteneur);
    query.bindValue(":type", type);
    query.bindValue(":capacite", capacite_max);
    query.bindValue(":quantite", quantite_actuelle);
    query.bindValue(":seuil", seuil_alerte);
    query.bindValue(":date", date_maj);

    bool result = query.exec();

    if (!result) {
        qDebug() << "Error in ajouter:" << query.lastError().text();
        qDebug() << "Query:" << query.lastQuery();
    }

    return result;
}

// READ - AFFICHER (PDF page 40)
QSqlQueryModel* Conteneur::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    // FIXED: Changed CONTENEUR to CONTENEURS
    model->setQuery("SELECT * FROM CONTENEURS ORDER BY ID_CONTENEUR");

    if (model->lastError().isValid()) {
        qDebug() << "Error in afficher:" << model->lastError().text();
    }

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Type"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Capacité Max"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Quantité Actuelle"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Seuil d'Alerte"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Date Mise à Jour"));

    return model;
}

// UPDATE - MODIFIER
bool Conteneur::modifier(int id)
{
    Connection& conn = Connection::createInstance();
    QSqlDatabase db = conn.getDatabase();

    if (!db.isOpen()) {
        qDebug() << "Database is not open!";
        return false;
    }

    QSqlQuery query(db);
    // FIXED: Changed CONTENEUR to CONTENEURS
    query.prepare("UPDATE CONTENEURS SET TYPE = :type, CAPACITE_MAX = :capacite, "
                  "QUANTITE_ACTUELLE = :quantite, SEUIL_ALERTE = :seuil, DATE_MAJ = :date "
                  "WHERE ID_CONTENEUR = :id");

    query.bindValue(":id", id);
    query.bindValue(":type", type);
    query.bindValue(":capacite", capacite_max);
    query.bindValue(":quantite", quantite_actuelle);
    query.bindValue(":seuil", seuil_alerte);
    query.bindValue(":date", date_maj);

    return query.exec();
}

// DELETE - SUPPRIMER (PDF page 36)
bool Conteneur::supprimer(int id)
{
    Connection& conn = Connection::createInstance();
    QSqlDatabase db = conn.getDatabase();

    if (!db.isOpen()) {
        qDebug() << "Database is not open!";
        return false;
    }

    QSqlQuery query(db);
    // FIXED: Changed CONTENEUR to CONTENEURS
    query.prepare("DELETE FROM CONTENEURS WHERE ID_CONTENEUR = :id");
    query.bindValue(":id", id);

    return query.exec();
}

void Conteneur::clear()
{
    id_conteneur = 0;
    type = "";
    capacite_max = 0;
    quantite_actuelle = 0;
    seuil_alerte = 0;
    date_maj = QDate::currentDate();
}
