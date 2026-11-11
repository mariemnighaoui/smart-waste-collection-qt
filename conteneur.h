#ifndef CONTENEUR_H
#define CONTENEUR_H

#include <QString>
#include <QDate>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QDebug>
#include "connection.h"

class Conteneur
{
private:
    int id_conteneur;
    QString type;
    int capacite_max;
    int quantite_actuelle;
    int seuil_alerte;
    QDate date_maj;

public:
    // Constructors
    Conteneur();
    Conteneur(int id, QString t, int cap, int qte, int seuil, QDate date);

    // Getters
    int getId() const { return id_conteneur; }
    QString getType() const { return type; }
    int getCapaciteMax() const { return capacite_max; }
    int getQuantiteActuelle() const { return quantite_actuelle; }
    int getSeuilAlerte() const { return seuil_alerte; }
    QDate getDateMaj() const { return date_maj; }

    // Setters
    void setId(int id) { id_conteneur = id; }
    void setType(const QString& t) { type = t; }
    void setCapaciteMax(int cap) { capacite_max = cap; }
    void setQuantiteActuelle(int qte) { quantite_actuelle = qte; }
    void setSeuilAlerte(int seuil) { seuil_alerte = seuil; }
    void setDateMaj(const QDate& date) { date_maj = date; }

    // Calculated fields
    int getPourcentageRemplissage() const;
    QString getNiveauAlerte() const;

    // CRUD Methods (following PDF guidelines)
    bool ajouter();                          // CREATE
    QSqlQueryModel* afficher();               // READ
    bool modifier(int id);                    // UPDATE
    bool supprimer(int id);                    // DELETE

    // NEW: Static method to check if ID exists
    static bool idExiste(int id);              // Vérifier si un ID existe

    // Helper methods
    void clear();
};

#endif // CONTENEUR_H
