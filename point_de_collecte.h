#ifndef POINTCOLLECTE_H
#define POINTCOLLECTE_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QObject>
#include <QMessageBox>

class PointCollecte
{
private:
    int id_pc;
    QString zone;
    QString adresse;
    int niveau;
    QString derniere_collecte;
    QString type_zone;

public:
    // Constructeurs
    PointCollecte();
    PointCollecte(int, QString, QString, int, QString, QString);

    // CRUD
    bool ajouter();
    QSqlQueryModel* afficher();
    bool supprimer(int);
    bool modifier(int id);

    //recherche
    QSqlQueryModel* rechercher(QString texte);
};

#endif // POINTCOLLECTE_H
