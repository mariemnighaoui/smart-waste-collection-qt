#include "Camion.h"


Camion::Camion() {
    this->matricule = "";
    this->capacite_max = 0;
    this->remplissage = 0;
    this->consomation = 0;
    this->status = "";
    this->camEditMode=false;
}

Camion::Camion(QString matricule, float capacite_max, float remplissage, float consomation, QString status)
{
    this->matricule = matricule;
    this->capacite_max = capacite_max;
    this->remplissage = remplissage;
    this->consomation = consomation;
    this->status = status;
    this->camEditMode=false;
}

bool Camion::addCamion()const
{
    QSqlQuery query;
    query.prepare("insert into CAMIONS (matricule, capacite_max, remplissage, consomation, status) "
                  "values (:matricule, :capacite_max, :remplissage, :consomation, :status)");

    query.bindValue(":matricule",matricule);
    query.bindValue(":capacite_max",capacite_max);
    query.bindValue(":remplissage",remplissage);
    query.bindValue(":consomation",consomation);
    query.bindValue(":status",status);
    return query.exec();
}
QSqlQueryModel * Camion::showCamion()const
{
    QSqlQueryModel * model= new QSqlQueryModel();
    //model->setTable("CAMIONS");
    //model->setEditStrategy(QSqlTableModel::OnFieldChange);
    //model->select();
    model->setQuery("select matricule, capacite_max, remplissage, consomation, status from CAMIONS");
    model->setHeaderData(0,Qt::Horizontal,QObject::tr("Matricule"));
    model->setHeaderData(1,Qt::Horizontal,QObject::tr("Capacité max"));
    model->setHeaderData(2,Qt::Horizontal,QObject::tr("Remplissage"));
    model->setHeaderData(3,Qt::Horizontal,QObject::tr("Consomation"));
    model->setHeaderData(4,Qt::Horizontal,QObject::tr("Status"));

    return model;
}
bool Camion::editCamion()const
{
    QSqlQuery query;
    query.prepare(
        "UPDATE CAMIONS SET "
        "matricule = :matricule, "
        "capacite_max = :capacite_max, "
        "remplissage = :remplissage, "
        "consomation = :consomation, "
        "status = :status  "
        "where id_camion = :id_camion"
        );
    query.bindValue(":id_camion",id_camion);
    query.bindValue(":matricule",matricule);
    query.bindValue(":capacite_max",capacite_max);
    query.bindValue(":remplissage",remplissage);
    query.bindValue(":consomation",consomation);
    query.bindValue(":status",status);
    return query.exec();
}
bool Camion::deleteCamion(QString matricule)const
{
    QSqlQuery query;
    query.prepare("Delete from CAMIONS where MATRICULE= :matricule");
    query.bindValue(":matricule", matricule);
    return query.exec();
}

bool Camion::fetchCamionByMatricule(QString matricule)
{
    QSqlQuery query;
    query.prepare("Select * from CAMIONS where matricule = :matricule");
    query.bindValue(":matricule",matricule);
    query.exec();

    if(query.isActive() && !query.next())
    {
        return false;
    }
    this->id_camion = query.value("id_camion").toInt();
    this->matricule = query.value("matricule").toString();
    this->capacite_max = query.value("capacite_max").toFloat();
    this->remplissage = query.value("remplissage").toFloat();
    this->consomation = query.value("consomation").toFloat();
    this->status = query.value("status").toString();
    return true;
}

bool Camion::fetchCamionById(int id)
{
    QSqlQuery query;
    query.prepare("Select * from CAMIONS where id_camion = :id");
    query.bindValue(":id",id);
    query.exec();

    if(query.isActive() && !query.next())
    {
        return false;
    }
    this->id_camion = query.value("id_camion").toInt();
    this->matricule = query.value("matricule").toString();
    this->capacite_max = query.value("capacite_max").toFloat();
    this->remplissage = query.value("remplissage").toFloat();
    this->consomation = query.value("consomation").toFloat();
    this->status = query.value("status").toString();
    return true;
}

bool Camion::camControleSaisie()const
{
    if(capacite_max <= 0 || capacite_max > 70000)
    {
        QMessageBox::critical(
            nullptr,
            QMessageBox::tr("Erreur Saisie"),
            QMessageBox::tr("La capacite maximale saisie est invalide")
            );
        return false;
    }else if(consomation <=0 || consomation >100)
    {
        QMessageBox::critical(
            nullptr,
            QMessageBox::tr("Erreur Saisie"),
            QMessageBox::tr("La Consomation saisie est invalide")
            );
        return false;
    }
    return true;
}

QSqlQueryModel * Camion::camRechMat(QString Matricule)const
{
    QSqlQueryModel * model= new QSqlQueryModel();
    QSqlQuery query;
    query.prepare("select matricule, capacite_max, remplissage, consomation, status from CAMIONS "
                  "where matricule = :matricule");
    query.bindValue(":matricule", Matricule);
    if(!query.exec())
    {
        qDebug() << "Erreur SQL:";
    }
    model->setQuery(std::move(query));
    model->setHeaderData(0,Qt::Horizontal,QObject::tr("Matricule"));
    model->setHeaderData(1,Qt::Horizontal,QObject::tr("Capacité max"));
    model->setHeaderData(2,Qt::Horizontal,QObject::tr("Remplissage"));
    model->setHeaderData(3,Qt::Horizontal,QObject::tr("Consomation"));
    model->setHeaderData(4,Qt::Horizontal,QObject::tr("Status"));

    return model;
}
