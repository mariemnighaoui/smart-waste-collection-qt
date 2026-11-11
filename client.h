#ifndef CLIENT_H
#define CLIENT_H
#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QList>
#include <QPair>
class client
{
public:
    static QList<QPair<int, QString>> getListeNomsClients();
    // ── Structs internes ──────────────────────────────────────────────────
    struct ClientFideliteInfo {
        QString nom;
        int     points;
    };
    struct ClientMailInfo {
        QString email;
        int     points;
        QString type;
    };
private:
    int     id_client;
    int     nbTrans;
    int     nbPts;
    QString nom;
    QString type;
    QString adr;
    QString email;
    QString tel;
public:
    // Constructeurs
    client();
    client(int id, QString nom, QString type, QString adr,
           int nbPts, QString email, QString tel, int nbTrans);
    // Getters
    int     getId()      const { return id_client; }
    QString getNom()     const { return nom; }
    QString getType()    const { return type; }
    QString getAdr()     const { return adr; }
    QString getEmail()   const { return email; }
    QString getTel()     const { return tel; }
    int     getNbTrans() const { return nbTrans; }
    int     getNbPts()   const { return nbPts; }
    // Logique fidélité
    QString getNiveauFidelite();
    QString getReward();
    void    calculerPoints();
    // Stats statiques
    static void getStatsFidelite(int &bronze, int &silver, int &gold, int &vip);
    static void getStockConteneurs(int &qteActuelle, int &capMax);
    static QList<ClientFideliteInfo> getClientsFideliteData();
    static ClientMailInfo            getClientMailInfo(int id);
    // CRUD
    bool            ajouterClient();
    bool            supprimerClient(int id);
    bool            supprimerTousClients();
    bool            modifierClient();
    QSqlQueryModel* afficherClient();
    // Affichage / recherche / tri
    QSqlQueryModel* afficherNomsClients();
    QSqlQueryModel* rechercherClient(const QString &texte);
    QSqlQueryModel* trierClients(int colonne);
};
// Fonctions libres (définies dans client.cpp, hors classe)
int     calculerScoreClient(client c);
QString analyserStock(int qte);
QString genererMessage(client c);
#endif // CLIENT_H
