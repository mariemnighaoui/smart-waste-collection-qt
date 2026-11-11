#include "employee.h"
bool exists(QString column, QString value) {
    QSqlQuery query;
    query.prepare("SELECT * FROM employee WHERE " + column + " = :value");
    query.bindValue(":value", value);
    query.exec();
    return query.next(); // returns true if at least one row found
}

bool isOnlyNumbers(const QString &str) {
    QRegularExpression re("^[0-9]+$");
    return re.match(str).hasMatch();
}
bool employee::create_employee(){
    QSqlQuery query;
    QRegularExpression emailRegex(R"(^[\w\.-]+@[\w\.-]+\.\w{2,}$)");
    query.prepare("insert into employee (username, nom, prenom, email, password, telephone, adresse, poste, DATE_EMBAUCHE, question , reponse) "
                  "values (:username, :nom, :prenom, :email, :password, :telephone, :adresse, :poste, :DATE_EMBAUCHE, :question, :reponse)");
    query.bindValue(":username", get_username());
    query.bindValue(":nom", get_nom());
    query.bindValue(":prenom", get_prenom());
    query.bindValue(":email", get_email());
    query.bindValue(":password", get_password());
    query.bindValue(":telephone", get_telephone());
    query.bindValue(":adresse", get_adresse());
    query.bindValue(":poste", get_poste());
    query.bindValue(":DATE_EMBAUCHE", get_date());
    query.bindValue(":question", get_question());
    query.bindValue(":reponse", get_reponse());
    bool test=true;
    if(get_username()=="" || get_nom()=="" || get_prenom()=="" || get_email()=="" || get_telephone()=="" || get_adresse()=="" || get_reponse()==""){
        QMessageBox::warning(nullptr, "Erreur", "tous les champs doivent être complétés !");
    }else if(exists("username", get_username())){
        QMessageBox::warning(nullptr, "Erreur", "Ce nom d'utilisateur existe déjà !");
    }else if(!isOnlyNumbers(get_telephone())){
        QMessageBox::warning(nullptr, "Erreur", "le numéro de téléphone ne doit contenir que des numéros");
    }else if(!emailRegex.match(get_email()).hasMatch()){
        QMessageBox::warning(nullptr, "Erreur", "cet email n'est pas valide");
    }else{
        test=query.exec();
    }


    if(!test){
        qDebug() << "Insert failed:" << query.lastError().text();
    }
    return test;
}

QSqlTableModel* employee::read_employee(){
    QSqlTableModel* model = new QSqlTableModel();
    model->setTable("EMPLOYEE");
    model->setEditStrategy(QSqlTableModel::OnFieldChange); // auto-save on edit
    model->select();
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("username"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("nom"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("prenom"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("email"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("password"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("telephone"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("adresse"));
    model->setHeaderData(7, Qt::Horizontal, QObject::tr("poste"));
    model->setHeaderData(8, Qt::Horizontal, QObject::tr("date"));
    model->setHeaderData(9, Qt::Horizontal, QObject::tr("question"));
    model->setHeaderData(10, Qt::Horizontal, QObject::tr("reponse"));
    model->setHeaderData(11, Qt::Horizontal, QObject::tr("ID"));
    return model;
}
QString employee::generatePassword(int length) {
    QString chars = "abcdefghijklmnopqrstuvwxyz"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "0123456789"
                    "!@#$%&*?";

    QString password;
    for (int i = 0; i < length; i++) {
        int index = QRandomGenerator::global()->bounded(chars.length());
        password += chars[index];
    }
    return password;
}
bool employee::login(QString username, QString password) const{

    if(!exists("USERNAME", username)){
        QMessageBox::warning(nullptr, "Erreur", "ce nom d'utilisateur n'existe pas !");
        return false;
    }else{
        QSqlQuery query;
        query.prepare("SELECT password FROM employee WHERE USERNAME = :value");
        query.bindValue(":value", username);
        query.exec();
        query.next();
        QString dbPassword=query.value("password").toString();
        if(dbPassword == password) return true;
        else{
            QMessageBox::warning(nullptr, "Erreur", "ce mot de passe est incorrect !");
            return false;
        }
    }
}
/*bool employee::update_employee(){
    QSqlQuery query;
    query.prepare("update"
                  "     employee"
                  "set"
                  "     username = :username, nom = :nom, prenom = :prenom, email = :email, password =:password, telephone = :telephone, adresse = :adresse, poste = :poste, DATE_EMBAUCHE = :DATE_EMBAUCHE, question = :question , reponse = :reponse"
                  "where id = :id");
    return query.exec();
}*/
