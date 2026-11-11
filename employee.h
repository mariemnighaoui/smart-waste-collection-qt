#ifndef EMPLOYEE_H
#define EMPLOYEE_H
#include <QMessageBox>
#include <QGraphicsBlurEffect>
#include <QAbstractItemView>

//***************************************GPC
#include <QHeaderView>
#include <QProgressBar>
#include <QTableWidgetItem>
#include <QSqlQuery>
#include <QSqlError>

//Stat
#include <QVBoxLayout>
#include <QPainter>

#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QDate>
#include <QString>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QRegularExpression>
#include <QRandomGenerator>
#include <vector>
#include <algorithm>

//class deff
using namespace std;
class employee{
private:
    int id;
    QString username;
    QString nom;
    QString prenom;
    QString email;
    QString password;
    QString telephone;
    QString adresse;
    QString poste;
    QDate date;
    QString question;
    QString reponse;
public:
    //getters
    int get_id() const{return id;}
    QString get_username() const{return username;}
    QString get_nom() const{return nom;}
    QString get_prenom() const{return prenom;}
    QString get_email() const{return email;}
    QString get_password() const{return password;}
    QString get_telephone() const{return telephone;}
    QString get_adresse() const{return adresse;}
    QString get_poste() const{return poste;}
    QDate get_date() const{return date;}
    QString get_question() const{return question;}
    QString get_reponse() const{return reponse;}
    //setters
    void set_id(int id){this->id=id;}
    void set_username(QString username){this->username=username;}
    void set_nom(QString nom){this->nom=nom;}
    void set_prenom(QString prenom){this->prenom=prenom;}
    void set_email(QString email){this->email=email;}
    void set_password(QString password){this->password=password;}
    void set_telephone(QString telephone){this->telephone=telephone;}
    void set_adresse(QString adresse){this->adresse=adresse;}
    void set_poste(QString poste){this->poste=poste;}
    void set_date(QDate date){this->date=date;}
    void set_question(QString question){this->question=question;}
    void set_reponse(QString reponse){this->reponse=reponse;}
    //constructors
    employee(){
        id=0;
        username="";
        nom="";
        prenom="";
        email="";
        password=generatePassword(12);
        telephone="";
        adresse=".";
        poste="";
        date=QDate::currentDate();
        question="";
        reponse="";
    }
    employee(int id, QString nom, QString prenom, QString email, QString password, QString telephone, QString adresse, QString poste, QDate date, QString question, QString reponse){
        this->id=id;
        this->username=prenom+"."+nom;
        this->nom=nom;
        this->prenom=prenom;
        this->email=email;
        this->password=password;
        this->telephone=telephone;
        this->adresse=adresse;
        this->poste=poste;
        this->date=date;
        this->question=question;
        this->reponse=reponse;
    }
    QString generatePassword(int length = 12);
    bool create_employee();
    QSqlTableModel* read_employee();
    bool update_employee();
    bool delete_employee();
    bool login(QString username, QString password) const;

};

#endif // EMPLOYEE_H
