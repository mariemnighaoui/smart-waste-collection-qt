#include "client.h"
#include <QSqlError>
client::client() {}

client::client(int id, QString nom, QString type, QString adr, int nbPts, QString email, QString tel, int nbTrans)
{
    this->id_client = id;
    this->nom       = nom;
    this->type      = type;
    this->adr       = adr;
    this->nbPts     = nbPts;
    this->email     = email;
    this->tel       = tel;
    this->nbTrans   = nbTrans;
}

bool client::ajouterClient()
{
    // Génère un ID = MAX existant + 1
    QSqlQuery seqQuery;
    if (!seqQuery.exec("SELECT NVL(MAX(id_client), 0) + 1 FROM CLIENTS")) {
        qDebug() << "Erreur ID:" << seqQuery.lastError().text();
        return false;
    }
    if (!seqQuery.next()) return false;
    id_client = seqQuery.value(0).toInt();
    nbPts = nbTrans * 10;
    QSqlQuery query;
    query.prepare(
        "INSERT INTO CLIENTS (id_client, nom, type, adresse, points, email, telephone, nb_transaction) "
        "VALUES (:id_client, :nom, :type, :adresse, :points, :email, :telephone, :nb_transaction)"
        );
    query.bindValue(":id_client",      id_client);
    query.bindValue(":nom",            nom);
    query.bindValue(":type",           type);
    query.bindValue(":adresse",        adr);
    query.bindValue(":points",         QString::number(nbPts));
    query.bindValue(":email",          email);
    query.bindValue(":telephone",      tel);        // ← QString directement
    query.bindValue(":nb_transaction", nbTrans);

    bool ok = query.exec();
    if (!ok)
    {
        QSqlError err = query.lastError();
        qDebug() << "=== ERREUR SQL ===";
        qDebug() << "Database text:" << err.databaseText();
        qDebug() << "Native code  :" << err.nativeErrorCode();
    }
    return ok;
}
bool client::supprimerClient(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM CLIENTS WHERE id_client = :id");
    query.bindValue(":id", id);
    return query.exec();
}

bool client::modifierClient()
{
    nbPts = nbTrans * 10;
    QSqlQuery query;
    query.prepare(
        "UPDATE CLIENTS SET "
        "nom             = :nom, "
        "type            = :type, "
        "adresse         = :adresse, "
        "points          = :points, "
        "email           = :email, "
        "telephone       = :telephone, "
        "nb_transaction = :nb_transaction "
        "WHERE id_client = :id"
        );
    query.bindValue(":id",              id_client);
    query.bindValue(":nom",             nom);
    query.bindValue(":type",            type);
    query.bindValue(":adresse",         adr);
    query.bindValue(":points",         QString::number(nbPts));  // ← VARCHAR2
    query.bindValue(":telephone",      tel);         // ← NUMBER
    query.bindValue(":email",           email);
    query.bindValue(":nb_transaction", nbTrans);
    return query.exec();
}

QSqlQueryModel* client::afficherClient()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery(
        "SELECT id_client, nom, type, telephone, adresse, email, points, nb_transaction "
        "FROM CLIENTS"
        );
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Nom"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Type"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Téléphone"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Adresse"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Email"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Points"));
    model->setHeaderData(7, Qt::Horizontal, QObject::tr("Nb transaction"));
    return model;
}
void client::calculerPoints()
{
    // 1 transaction = 10 points
    nbPts = nbTrans * 10;
}
QString client::getNiveauFidelite()
{
    if(nbPts < 100) return "Bronze";
    else if(nbPts < 300) return "Silver";
    else if(nbPts < 600) return "Gold";
    else return "VIP";
}
QString client::getReward()
{
    QString niveau = getNiveauFidelite();

    if(niveau == "VIP") return "Réduction 20%";
    else if(niveau == "Gold") return "Réduction 15%";
    else if(niveau == "Silver") return "Réduction 10%";
    else return "Réduction 5%";
}
int calculerScoreClient(client c)
{
    int score = 0;

    if (c.getNiveauFidelite() == "VIP")
        score += 3;
    else if (c.getNiveauFidelite() == "Gold")
        score += 2;
    else
        score += 1;

    return score;
}
QString analyserStock(int qte)
{
    if (qte == 0)
        return "RUPTURE";
    else if (qte <= 5)
        return "FAIBLE";
    else
        return "NORMAL";
}
QSqlQueryModel* client::afficherNomsClients()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery("SELECT id_client, nom FROM CLIENTS ORDER BY nom");
    return model;
}

bool client::supprimerTousClients()
{
    QSqlQuery q;
    return q.exec("DELETE FROM CLIENTS");
}

void client::getStockConteneurs(int &qteActuelle, int &capMax)
{
    QSqlQuery q;
    q.exec("SELECT SUM(QUANTITE_ACTUELLE), SUM(CAPACITE_MAX) FROM CONTENEURS");
    if (q.next()) {
        qteActuelle = q.value(0).toInt();
        capMax      = q.value(1).toInt();
    }
}
QString genererMessage(client c)
{
    QSqlQuery query;
    query.exec("SELECT QUANTITE_ACTUELLE FROM CONTENEURS");

    int qte = 0;
    if (query.next())
        qte = query.value(0).toInt();

    QString stock = analyserStock(qte);
    int score = calculerScoreClient(c);

    QString message;

    message += "Bonjour " + c.getNom() + ",\n\n";

    message += "Votre profil client : ";

    if (score >= 3)
        message += "Premium ⭐\n";
    else
        message += "Standard\n";

    message += "\n";

    // 🧠 LOGIQUE STOCK
    if (stock == "RUPTURE")
    {
        message += "🚨 Situation critique : rupture de stock.\n";
        message += "Les collectes sont temporairement suspendues.\n";
    }
    else if (stock == "FAIBLE")
    {
        message += "⚠️ Stock faible détecté.\n";
        message += "Une collecte est recommandée prochainement.\n";
    }
    else
    {
        message += "✅ Stock normal disponible.\n";
        message += "Vous pouvez planifier une collecte à tout moment.\n";
    }

    // 🎯 PERSONNALISATION BONUS
    if (score >= 3 && stock == "NORMAL")
    {
        message += "\n🎁 Offre VIP active : priorité de collecte + réduction.\n";
    }

    message += "\nCordialement,\nWaste Recycling Team";

    return message;
}
void client::getStatsFidelite(int &bronze, int &silver, int &gold, int &vip)
{
    bronze = silver = gold = vip = 0;
    QSqlQuery q;
    q.exec("SELECT points FROM CLIENTS");
    while (q.next()) {
        int pts = q.value(0).toInt();
        if      (pts < 100) bronze++;
        else if (pts < 300) silver++;
        else if (pts < 600) gold++;
        else                vip++;
    }
}
QSqlQueryModel* client::rechercherClient(const QString &texte)
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery(
        "SELECT id_client, nom, type, telephone, adresse, email, points, nb_transaction "
        "FROM CLIENTS WHERE "
        "LOWER(nom)       LIKE LOWER('%" + texte + "%') OR "
                  "LOWER(type)      LIKE LOWER('%" + texte + "%') OR "
                  "LOWER(email)     LIKE LOWER('%" + texte + "%') OR "
                  "LOWER(adresse)   LIKE LOWER('%" + texte + "%') OR "
                  "TO_CHAR(id_client) LIKE '%" + texte + "%'"
        );
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Nom"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Type"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Téléphone"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Adresse"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Email"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Points"));
    model->setHeaderData(7, Qt::Horizontal, QObject::tr("Nb transaction"));
    return model;
}

QSqlQueryModel* client::trierClients(int colonne)
{
    QStringList cols = {
        "id_client", "nom", "type", "telephone",
        "adresse", "email", "points", "nb_transaction"
    };

    if (colonne < 0 || colonne >= cols.size())
        colonne = 0;

    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery(
        "SELECT id_client, nom, type, telephone, adresse, email, points, nb_transaction "
        "FROM CLIENTS ORDER BY " + cols[colonne]
        );
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Nom"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Type"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Téléphone"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Adresse"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Email"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Points"));
    model->setHeaderData(7, Qt::Horizontal, QObject::tr("Nb transaction"));
    return model;
}
// ── Retourne la liste nom+points triée pour chargerFidelite() ──────────────
QList<client::ClientFideliteInfo> client::getClientsFideliteData()
{
    QList<ClientFideliteInfo> liste;
    QSqlQuery q;
    q.exec("SELECT nom, points FROM CLIENTS ORDER BY points DESC");
    while (q.next()) {
        ClientFideliteInfo info;
        info.nom    = q.value(0).toString();
        info.points = q.value(1).toInt();
        liste.append(info);
    }
    return liste;
}

// ── Retourne email+points+type d'un client par son ID ──────────────────────
client::ClientMailInfo client::getClientMailInfo(int id)
{
    ClientMailInfo info;
    info.points = 0;
    QSqlQuery q;
    q.prepare("SELECT email, points, type FROM CLIENTS WHERE id_client = :id");
    q.bindValue(":id", id);
    q.exec();
    if (q.next()) {
        info.email  = q.value(0).toString();
        info.points = q.value(1).toInt();
        info.type   = q.value(2).toString();
    }
    return info;
}
// ✅ NOUVEAU — requête directe, pas besoin d'objet
QList<QPair<int, QString>> client::getListeNomsClients()
{
    QList<QPair<int, QString>> liste;
    QSqlQuery q;
    q.exec("SELECT id_client, nom FROM CLIENTS ORDER BY nom");
    while (q.next()) {
        int     id  = q.value(0).toInt();
        QString nom = q.value(1).toString();
        liste.append(qMakePair(id, nom));
    }
    return liste;
}