#ifndef MACHINE_H
#define MACHINE_H
#include<QString>
#include<QSqlQuery>
#include<QSqlQueryModel>
#include <QDateTime>

class Machine
{
private :
    int id_machine , capacite;
    QString type_dechet,nom_machine;
    QDateTime date_ajout;

public:
    // Constructeurs
    Machine();
    Machine(int id, QString type, QString nom, int capacite, QDateTime date);

    // Getters
    int getIdMachine() const { return id_machine; }
    int getCapacite() const { return capacite; }
    QString getTypeDechet() const { return type_dechet; }
    QString getNomMachine() const { return nom_machine; }
    QDateTime getDateAjout() const { return date_ajout; }

    // Setters
    void setIdMachine(int id) { id_machine = id; }
    void setCapacite(int c) { capacite = c; }
    void setTypeDechet(QString type) { type_dechet = type; }
    void setNomMachine(QString nom) { nom_machine = nom; }
    void setDateAjout(QDateTime date) { date_ajout = date; }

    // Méthodes CRUD
    bool ajouter();
    QSqlQueryModel* afficher();
    bool supprimer(int id);
    bool modifier(int id);
    QSqlQueryModel* rechercher(QString nom);
    QSqlQueryModel* trierParType(QString type);
};

#endif // MACHINE_H
