#ifndef WASTERECYCLING_H
#define WASTERECYCLING_H
#include "arduinohandler.h"
#include <QMainWindow>
#include <QGraphicsBlurEffect>
#include <QAbstractItemView>
#include "machine.h"
#include "employee.h"
#include "conteneur.h"
#include "Camion.h"
#include "client.h"
#include "point_de_collecte.h"
#include <QSqlQueryModel>
#include <QModelIndex>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QColor>
#include <QListWidget>       // ← ajouter
#include <QTextEdit>         // ← ajouter
#include <QJsonObject>
#include <QMediaRecorder>
#include <QMediaCaptureSession>
#include <QNetworkAccessManager>
/////////////////////////progress bar GPC niveau de remplissage ---------------------------------------------------------------
class ProgressBarDelegate : public QStyledItemDelegate
{
public:
    explicit ProgressBarDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

private:
    QColor getFillColor(int niveau) const;
};

QT_BEGIN_NAMESPACE
namespace Ui {
class WasteRecycling;
}
QT_END_NAMESPACE

class WasteRecycling : public QMainWindow
{
    Q_OBJECT

public:
    WasteRecycling(QWidget *parent = nullptr);
    ~WasteRecycling();

private slots:

    // ********************* SLOTS NAVIGATION SIDEBAR *************************
    void on_GEmpBtn_clicked();
    void on_GContBtn_clicked();
    void on_GMachinBtn_clicked();
    void on_GCamBtn_clicked();
    void on_GPcBtn_clicked();
    void on_GClientBtn_clicked();
    // ********************* SLOTS NAVIGATION SIDEBAR *************************

    // ********************* SLOTS GESTION POINTS DE COLLECTE *************************
    void on_GPC_BtnSave_clicked();
    void on_GPC_BtnModifier_clicked();
    void on_gpcBtnSupprimer_clicked();
    void on_gpcTableView_doubleClicked(const QModelIndex &index);
    void on_GPC_TxtSearch_textChanged(const QString &text);

    // ********************* SLOTS GESTION POINTS DE COLLECTE *************************


    // ********************* SLOTS GESTION CAMION *************************

    // Navigation Metier Crud
    void on_crudMet1CamBtn_clicked();
    void on_crudMet2CamBtn_clicked();
    void on_f1AccCamBtn_clicked();
    void on_f1Met2CamBtn_clicked();
    void on_f2AccCamBtn_clicked();
    void on_f2Met1CamBtn_clicked();

    // Fonctionalite Crud
    void on_addCamBtn_clicked();
    void on_delCamBtn_clicked();
    void on_tableCam_doubleClicked(const QModelIndex &index);

    // Fonctionalite Métier
    void on_rechCamBtn_clicked();
    // ********************* SLOTS GESTION CAMION *************************


    // ********************* SLOTS GESTION CLIENT *************************
    void on_clbtnajt_clicked();
    void on_clbtnsup_clicked();
    void on_clbtnmodif_clicked();
    void on_clbtnstat_clicked();
    void on_clbtnsysprio_clicked();
    void on_clbtnsysfide_clicked();
    void on_clretour1_clicked();
    void on_clretour2_clicked();
    void on_clretour3_clicked();
    void on_clsearch_textChanged(const QString &text);
    void on_clbtntri_clicked();
    void on_clbtnpdf_clicked();
    void on_cllistec_doubleClicked(const QModelIndex &index);
    void chargerFidelite();
    void on_clbtnvocSup_clicked();

    // ********************* SLOTS GESTION CLIENT *************************



    //--------------------------------------container------------------------------
    // CRUD Slots (Controller)
    void on_GCont_addButton_clicked();        // CREATE
    void on_GCont_refreshButton_clicked();    // READ (refresh)
    void on_GCont_updateButton_clicked();     // UPDATE
    void on_GCont_deleteButton_clicked();     // DELETE

    // UI Slots
    void on_GCont_clearButton_clicked();
    void on_GCont_previewButton_clicked();
    void on_GCont_sortButton_clicked();
    void on_GCont_searchButton_clicked();
    void on_GCont_containerTable_doubleClicked(const QModelIndex &index);
    QString genererMessageLocal(QString nom, int points);
    //--------------------------------------container------------------------------
    // Container Management Tab Slots (only remaining functions)

    //--------------------------------------machine------------------------------
    void on_MRbtn_lancer_clicked();
    void on_MRbtn_supprimer_clicked();
    void on_MRtable_doubleClicked(const QModelIndex &index);
    void on_MRtable_clicked(const QModelIndex &index);
    void on_MRbtn_stats_clicked();
    void on_MRline_recherche_returnPressed();
    void on_MRbtn_trier_clicked();
    //--------------------------------------machine------------------------------

    //*************************************EMPLOYEE******************************************
    void refreshTable(employee e);
    void onRightClick(const QPoint &pos);
    //*************************************EMPLOYEE******************************************


private:

    ///////////////////////////////////////////GPC
    PointCollecte PC;
    int gpcSelectedId = -1;

    QColor getFillColor(int niveau);
    ////////////////////////////////
    Ui::WasteRecycling *ui;
    enum PageGestion{
        PAGE_GEMPLOYE = 0,
        PAGE_GCONTAINER = 1,
        PAGE_GMACHINE = 2,
        PAGE_GCAMION = 3,
        PAGE_GPOINT_COLLECTE = 4,
        PAGE_GCLIENT = 5
    };
    // PAGE STACKED

    ///////////////////////////GCAMION
    // PAGES CAMION
    enum PagesGCamion {
        PAGE_CAMION_CRUD = 0,
        PAGE_CAMION_F1 = 2,
        PAGE_CAMION_F2 = 1
    };
    // Objet Temporaire
    Camion CamTemp;
    // fonctions supplementaires
    void resetAddEditCamForm();

    //////////////////////////////////
    // PAGE CLIENT
    client            ClientTemp;
    QSqlQueryModel   *m_model = nullptr;   // modèle lié au QTableView

    enum Pages {
        PAGE_CRUDCLIENT    = 0,
        PAGE_STATCLIENT    = 1,
        PAGE_SYSFIDECLIENT = 2,
        PAGE_SYSPRIOCLIENT = 3
    };
    // ── Vocal Arduino — contexte ──
    enum ModeVocal {
        VOCAL_AUCUN,
        VOCAL_CHAMP,        // remplir un champ précis
        VOCAL_SUPPRESSION   // commande suppression
    };
    ModeVocal  m_modeVocal     = VOCAL_AUCUN;
    int        m_clientIdModif = -1;
    QLineEdit *m_champCible    = nullptr;   // champ à remplir après Arduino
    QString    m_champType;                 // "tel", "email", "trans", ou ""
    QMessageBox *m_popupVocal = nullptr;
    // ── Helpers vocaux ──
    void attendreArduino(QLineEdit *champ, const QString &type = "");
    void attendreArduinoSuppression();
    void traiterAudioArduino(const QString &wavPath);
    void traiterCommandeSuppression(const QString &texte);
    void remplirChampDepuisTexte(const QString &texte);

    // ── Helpers ──
    void chargerClients();                          // charge/recharge la table depuis la BDD
    int  selectedId() const;                        // retourne l'id_client de la ligne sélectionnée (-1 si aucune)
    bool isValidEmail(const QString &email);
    // ── Messagerie IA (clpagesysprio) ─────────────────────────────────────────
    QListWidget           *cl_listeMsg    = nullptr;
    QTextEdit             *cl_messageIA   = nullptr;
    QPushButton           *cl_btnGenerer  = nullptr;
    QLabel                *cl_lblClientIA = nullptr;
    ArduinoHandler        *m_arduino          = nullptr;
    QString                m_clientIdEnCours;
    QNetworkAccessManager *m_network          = nullptr;
    QString                m_openaiKey        = "YOUR_OPENAI_API_KEY";
    QString                m_anthropicKey     = "YOUR_ANTHROPIC_API_KEY";
    QString m_groqKey = "YOUR_GROQ_API_KEY";
    QMediaCaptureSession  *m_session          = nullptr;
    QMediaRecorder        *m_recorder         = nullptr;
    QString                m_wavPath;
    void envoyerAudioWhisper(const QString &wavPath);
    void envoyerTexteAuLLM(const QString &texte);
    void ouvrirDialoguePreRempli(const QJsonObject &data);
    void setupMessagerieIA();
    // PAGE CLIENT
    //////////////////////////////////

    //--------------------------------------machine------------------------------
    void afficherMRtab();
    void afficherStatistiquesMachines();
    int getCapaciteTotaleMachines();
    double getCapaciteMoyenneMachines();

    Machine mach;
    //--------------------------------------machine------------------------------

    //--------------------------------------container------------------------------
    Conteneur C;        // Objet de la classe Conteneur (Modèle)

    void refreshTable();                    // Actualiser l'affichage
    void clearForm();
    void fillFormFromConteneur(const Conteneur& conteneur);
    bool validateForm();
    Conteneur getConteneurFromForm();
    int getSelectedId();

    //--------------------------------------container------------------------------
};
#endif // WASTERECYCLING_H