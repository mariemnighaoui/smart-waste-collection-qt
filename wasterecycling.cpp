#include "wasterecycling.h"
#include "ui_wasterecycling.h"
#include <QMessageBox>
#include <QGraphicsBlurEffect>
#include <QAbstractItemView>
#include "machine.h"
#include "employee.h"
#include "point_de_collecte.h"
#include "client.h"
#include <QDesktopServices>
#include <QUrl>
#include <QHeaderView>
#include <QProgressBar>
#include <QTableWidgetItem>
#include <QIntValidator>
#include <QRegularExpressionValidator>
#include <QFormLayout>
#include <QFileDialog>
#include <QPrinter>
#include <QTextDocument>
#include <QPageLayout>
#include <QDate>
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
#include <QSqlError>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTextEdit>
#include <QHttpMultiPart>
#include <QDateTime>  // ← manquant ! utilisé dans le connect Arduino
#include <QMediaRecorder>
#include <QMediaCaptureSession>
#include <QAudioInput>
#include <QTimer>
#include <QDir>
#include <QMediaFormat>
///////////////////////////////////////////////////////////// CLIENT — Validators réutilisables /////////////////////////////////////////////////////
// Ces expressions régulières vérifient le format des champs saisis
static const QRegularExpression RE_NOM  (R"(^[a-zA-ZÀ-ÿ\s\-']+$)");  // lettres + espaces seulement
static const QRegularExpression RE_TEL  (R"(\d{8,15})");               // 8 à 15 chiffres
static const QRegularExpression RE_EMAIL(R"(^[a-zA-Z0-9._%+\-]+@[a-zA-Z0-9.\-]+\.[a-zA-Z]{2,}$)"); // format email
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

WasteRecycling::WasteRecycling(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::WasteRecycling)
{
    ui->setupUi(this);

    /// *********** affichage Table Camion ******************
    ui->tableCam->setModel(CamTemp.showCamion());

    ///////////////////////////////////////////////////////////// GPC — Affichage initial /////////////////////////////////////////////////////
    ui->gpcTableView->setModel(PC.afficher());
    ui->gpcTableView->setItemDelegateForColumn(3, new ProgressBarDelegate(this));
    ui->gpcTableView->horizontalHeader()->setStretchLastSection(true);
    ui->gpcTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->gpcTableView->verticalHeader()->setVisible(false);
    ui->gpcTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->gpcTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->gpcTableView->setAlternatingRowColors(true);
    ui->gpcTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->gpcTableView->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->gpcTableView->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->gpcTableView->setShowGrid(true);
    ui->gpcTableView->setWordWrap(false);
    ui->gpcTableView->verticalHeader()->setDefaultSectionSize(35);
    ui->gpcTableView->horizontalHeader()->setMinimumHeight(40);
    ui->gpcTableView->setStyleSheet(
        "QTableView {"
        "alternate-background-color: #2e5f5a;"
        "background-color: #2b6f68;"
        "color: white;"
        "gridline-color: #245c56;"
        "border: none;"
        "selection-background-color: #8bbf6a;"
        "selection-color: black;"
        "font-size: 14px;"
        "}"
        "QHeaderView::section {"
        "background-color: #3b6f7a;"
        "color: white;"
        "padding: 8px;"
        "border: none;"
        "font-weight: bold;"
        "font-size: 15px;"
        "}"
        );

    ///////////////////////////////////////////////////////////// CLIENT — Initialisation /////////////////////////////////////////////////////

    // Configuration du tableau de liste clients
    ui->cllistec->setSelectionBehavior(QAbstractItemView::SelectRows);   // sélection par ligne entière
    ui->cllistec->setSelectionMode(QAbstractItemView::SingleSelection);  // une seule ligne à la fois
    ui->cllistec->setEditTriggers(QAbstractItemView::NoEditTriggers);    // lecture seule
    ui->cllistec->horizontalHeader()->setStretchLastSection(true);       // dernière colonne s'étire
    ui->cllistec->verticalHeader()->setVisible(false);                   // cache les numéros de lignes

    // Texte indicatif dans le champ de recherche
    ui->clsearch->setPlaceholderText("Rechercher dans tous les champs...");

    // Chargement initial de tous les clients depuis la BDD
    chargerClients();

    // Double-clic sur une ligne → suppression du client
    connect(ui->cllistec, &QTableView::doubleClicked,
            this, &WasteRecycling::on_cllistec_doubleClicked);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////// GPC — Navigation entre pages /////////////////////////////////////////////////////
    connect(ui->GPC_BtnStats, &QPushButton::clicked, this, [=]() {
        ui->GPC_Pages->setCurrentIndex(1);
    });
    connect(ui->GPC_BtnSave, &QPushButton::clicked, this, [=]() {
        ui->GPC_Pages->setCurrentIndex(0);
    });
    connect(ui->GPC_BtnBackStats, &QPushButton::clicked, this, [=]() {
        ui->GPC_Pages->setCurrentIndex(0);
    });

    // ================= BAR CHART =================
    QBarSet *set0 = new QBarSet("Remplissage");
    *set0 << 60 << 75 << 58 << 35;
    set0->setColor(QColor(47, 125, 149));
    QBarSeries *barSeries = new QBarSeries();
    barSeries->append(set0);
    QChart *barChart = new QChart();
    barChart->addSeries(barSeries);
    barChart->legend()->hide();
    barChart->setBackgroundVisible(false);
    QStringList zones;
    zones << "Centre" << "Nord" << "Sud" << "Est";
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(zones);
    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, 100);
    barChart->addAxis(axisX, Qt::AlignBottom);
    barChart->addAxis(axisY, Qt::AlignLeft);
    barSeries->attachAxis(axisX);
    barSeries->attachAxis(axisY);
    QChartView *barView = new QChartView(barChart);
    barView->setRenderHint(QPainter::Antialiasing);
    barView->setStyleSheet("background: transparent;");
    QVBoxLayout *barLayout = new QVBoxLayout(ui->GPC_ChartBar);
    barLayout->setContentsMargins(0, 0, 0, 0);
    barLayout->addWidget(barView);

    // ================= DONUT PIE =================
    QPieSeries *pieSeries = new QPieSeries();
    pieSeries->append("Urgent", 10);
    pieSeries->append("Haute", 20);
    pieSeries->append("Normale", 40);
    pieSeries->append("Basse", 30);
    pieSeries->setHoleSize(0.45);
    QChart *pieChart = new QChart();
    pieChart->addSeries(pieSeries);
    pieChart->legend()->setAlignment(Qt::AlignLeft);
    pieChart->setBackgroundVisible(false);
    QChartView *pieView = new QChartView(pieChart);
    pieView->setRenderHint(QPainter::Antialiasing);
    pieView->setStyleSheet("background: transparent;");
    QVBoxLayout *pieLayout = new QVBoxLayout(ui->GPC_ChartPie);
    pieLayout->setContentsMargins(0, 0, 0, 0);
    pieLayout->addWidget(pieView);

    //--------------machine constructeurs------------------------
    connect(ui->MRtable, &QTableView::clicked, this, &WasteRecycling::on_MRtable_clicked);
    ui->MRtable->setModel(mach.afficher());
    ui->MRtable->setColumnWidth(4, 150);
    ui->MRtable->setCornerButtonEnabled(false);
    ui->MRtable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->MRtable->setSelectionMode(QAbstractItemView::SingleSelection);
    disconnect(ui->MRbtn_stats, nullptr, nullptr, nullptr);
    connect(ui->MRbtn_stats, &QPushButton::clicked, this, &WasteRecycling::on_MRbtn_stats_clicked);
    connect(ui->MRline_recherche, &QLineEdit::returnPressed, [this]() {
        QString texte = ui->MRline_recherche->text().trimmed();
        if (texte.isEmpty()) {
            ui->MRtable->setModel(mach.afficher());
        } else {
            ui->MRtable->setModel(mach.rechercher(texte));
        }
    });

    //----------------------------------------------------------------------container------------------------------
    setWindowTitle("Systeme de Gestion des Conteneurs - Waste Manager Pro");
    setMinimumSize(1200, 800);
    refreshTable();

    // ********************************* GESTION EMPLOYEE *****************************************
    ui->loginStackedWidget->setAttribute(Qt::WA_StyledBackground, true);
    ui->employeeComboBox->view()->setCursor(Qt::PointingHandCursor);
    QGraphicsDropShadowEffect *frameshadow = new QGraphicsDropShadowEffect;
    frameshadow->setBlurRadius(15);
    frameshadow->setOffset(0, 4);
    frameshadow->setColor(QColor(0,0,0,100));
    QGraphicsDropShadowEffect *btnshadow = new QGraphicsDropShadowEffect;
    btnshadow->setBlurRadius(20);
    btnshadow->setOffset(0, 4);
    btnshadow->setColor(QColor(0,0,0,150));
    ui->employeeBtnFrame->setGraphicsEffect(frameshadow);
    ui->empLoginReturnBtn->setGraphicsEffect(btnshadow);
    ui->questionComboBox->view()->setMinimumWidth(450);
    connect(ui->empLoginReturnBtn, &QPushButton::clicked, this, [=]() {
        ui->loginStackedWidget->setCurrentWidget(ui->loginPage);
    });
    connect(ui->empPasswordBtn, &QPushButton::clicked, this, [=]() {
        ui->loginStackedWidget->setCurrentWidget(ui->passwordPage);
    });
    connect(ui->employeeStatBtn, &QPushButton::clicked, this, [=]() {
        ui->employeeStackedWidget->setCurrentWidget(ui->statsPage);
    });
    connect(ui->employeeReturnBtn, &QPushButton::clicked, this, [=]() {
        ui->employeeStackedWidget->setCurrentWidget(ui->employeePage);
    });
    connect(ui->logoutBtn, &QPushButton::clicked, this, [=]() {
        ui->gestionsPages->hide();
        ui->loginStackedWidget->show();
        ui->loginStackedWidget->setCurrentWidget(ui->loginPage);
    });
    employee e;
    connect(ui->empTableView, &QTableView::customContextMenuRequested, this, &WasteRecycling::onRightClick);
    connect(ui->loginBtn, &QPushButton::clicked, this, [=]() {
        QString username = ui->empUsernameLineEdit->text();
        QString password = ui->empPasswordLineEdit->text();
        if(e.login(username, password)){
            ui->loginStackedWidget->hide();
            ui->gestionsPages->show();
            ui->employeeStackedWidget->show();
            ui->employeeStackedWidget->setCurrentWidget(ui->employeePage);
        }
    });
    refreshTable(e);
    ui->passwordAddLineEdit->setText(e.get_password());
    connect(ui->employeeAjoutBtn, &QPushButton::clicked, this, [=]() {
        employee e;
        e.set_username(ui->usernameAddLineEdit->text());
        e.set_nom(ui->nomAddLineEdit->text());
        e.set_prenom(ui->prenomAddLineEdit->text());
        e.set_password(ui->passwordAddLineEdit->text());
        e.set_email(ui->emailAddLineEdit->text());
        e.set_poste(ui->employeeComboBox->currentText());
        e.set_telephone(ui->telephoneAddLineEdit->text());
        e.set_question(ui->questionComboBox->currentText());
        e.set_reponse(ui->reponseAddLineEdit->text());
        bool test = e.create_employee();
        if(!test) qDebug() << "create failed:";
        refreshTable(e);
    });
    connect(ui->employeeFullTableBtn, &QPushButton::toggled, this, [=](bool checked) {
        if (checked) {
            ui->empTableView->hideColumn(3);
            ui->empTableView->hideColumn(4);
            ui->empTableView->hideColumn(6);
            ui->empTableView->hideColumn(8);
            ui->empTableView->hideColumn(9);
            ui->empTableView->hideColumn(10);
            ui->empTableView->hideColumn(11);
        } else {
            ui->empTableView->showColumn(3);
            ui->empTableView->showColumn(4);
            ui->empTableView->showColumn(6);
            ui->empTableView->showColumn(8);
            ui->empTableView->showColumn(9);
            ui->empTableView->showColumn(10);
            ui->empTableView->showColumn(11);
        }
    });
    connect(ui->employeeSearchBtn, &QPushButton::clicked, this, [=]() {
        QString empSearchWord = ui->employeeSearchLineEdit->text();
        QSqlTableModel *model = (QSqlTableModel*)ui->empTableView->model();
        model->setFilter("USERNAME LIKE '%" + empSearchWord + "%'");
        model->select();
    });
    QSqlTableModel *model = (QSqlTableModel*)ui->empTableView->model();
    connect(model, &QSqlTableModel::dataChanged, this, [=](const QModelIndex &index) {
        QString newValue = index.data().toString();
        int col = index.column();
        std::vector<QString> roles = {
            "Chauffeur", "Agent de collecte", "Responsable de flotte",
            "Chef d'équipe", "Responsable des opérations", "Administrateur",
            "Responsable RH", "Responsable sécurité", "Contrôleur qualité"
        };
        std::vector<QString> questions = {
            "Quel est le nom de votre premier animal ?",
            "Quel est le prénom de votre meilleur ami d'enfance ?",
            "Quel est le nom de votre école primaire ?",
            "Quel est votre plat préféré ?"
        };
        if (col == 5) {
            QRegularExpression re("^[0-9]+$");
            if (!re.match(newValue).hasMatch()) {
                QMessageBox::warning(nullptr, "Erreur", "Le téléphone doit contenir uniquement des chiffres !");
                model->revert(); return;
            }
        }
        if (col == 4) {
            QRegularExpression re(R"(^[\w\.-]+@[\w\.-]+\.\w{2,}$)");
            if (!re.match(newValue).hasMatch()) {
                QMessageBox::warning(nullptr, "Erreur", "L'adresse email est invalide !");
                model->revert(); return;
            }
        }
        if (col == 7) {
            if (std::find(roles.begin(), roles.end(), newValue) == roles.end()) {
                QMessageBox::warning(nullptr, "Erreur", "la poste est invalide !");
                model->revert(); return;
            }
        }
        if (col == 9) {
            if (std::find(questions.begin(), questions.end(), newValue) == questions.end()) {
                QMessageBox::warning(nullptr, "Erreur", "le question est invalide !");
                model->revert(); return;
            }
        }
        if (col == 11) {
            model->revert();
            QMessageBox::warning(nullptr, "Erreur", "vous ne pouvez pas changer l'identifiant !");
            return;
        }
    });

    ///////////////////////////////////////////////////////////// CLIENT — Messagerie /////////////////////////////////////////////////////
    // Initialisation de la page messagerie (appelée une seule fois au démarrage)
    setupMessagerieIA();
    // ── Arduino — initialisation ──────────────────────────────
    m_arduino = new ArduinoHandler(this);
    m_arduino->connecter();
    m_network = new QNetworkAccessManager(this);
    m_wavPath = QDir::tempPath() + "/recytrack_record.wav";
    qDebug() << "[Qt] WAV path initialisé :" << m_wavPath;

    // Quand Arduino appuie → micro PC démarre
    connect(m_arduino, &ArduinoHandler::enregistrementDebute,
            this, [this]() {
                if (m_modeVocal == VOCAL_AUCUN) {
                    qDebug() << "[Qt] Signal Arduino ignoré — aucun bouton cliqué";
                    return;
                }

                qDebug() << "[Qt] Enregistrement démarré";

                if (m_recorder) { m_recorder->stop(); delete m_recorder; m_recorder = nullptr; }
                if (m_session)  { delete m_session;   m_session  = nullptr; }

                m_session  = new QMediaCaptureSession(this);
                m_recorder = new QMediaRecorder(this);
                QAudioInput *audioInput = new QAudioInput(this);
                m_session->setAudioInput(audioInput);
                m_session->setRecorder(m_recorder);

                QMediaFormat format;
                format.setFileFormat(QMediaFormat::Wave);
                m_recorder->setMediaFormat(format);
                m_recorder->setAudioSampleRate(16000);
                m_recorder->setAudioChannelCount(1);
                m_recorder->setOutputLocation(QUrl::fromLocalFile(m_wavPath));
                m_recorder->record();

                qDebug() << "[Qt] Micro PC démarré →" << m_wavPath;
            }
            );


    // Quand Arduino relâche → micro PC s'arrête → Whisper traite
    connect(m_arduino, &ArduinoHandler::enregistrementTermine,
            this, [this](int duree) {
                Q_UNUSED(duree)
                if (!m_recorder) return;

                m_recorder->stop();

                QTimer::singleShot(500, this, [this]() {
                    QFileInfo fi(m_wavPath);

                    // ── Appui trop court → ignorer silencieusement ──
                    if (!fi.exists() || fi.size() < 10000) {
                        qDebug() << "[Qt] Appui trop court ignoré :" << fi.size() << "octets";
                        m_modeVocal = VOCAL_AUCUN;  // remet en attente
                        return;                      // pas de message d'erreur
                    }

                    qDebug() << "[Qt] WAV prêt :" << fi.size() << "octets → Whisper";
                    traiterAudioArduino(m_wavPath);
                });
            }
            );
    // ─────────────────────────────────────────────────────────
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

// ********************************* GESTION EMPLOYEE *****************************************
void WasteRecycling::onRightClick(const QPoint &pos) {
    QModelIndex index = ui->empTableView->indexAt(pos);
    if (!index.isValid()) return;
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Supprimer", "Voulez-vous supprimer cet employé ?",
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        QSqlTableModel *model = (QSqlTableModel*)ui->empTableView->model();
        model->removeRow(index.row());
        model->select();
    }
}
void WasteRecycling::attendreArduino(QLineEdit *champ, const QString &type)
{
    m_modeVocal  = VOCAL_CHAMP;
    m_champCible = champ;
    m_champType  = type;
    // Plus de popup — l'utilisateur appuie directement sur Arduino
}

void WasteRecycling::attendreArduinoSuppression()
{
    m_modeVocal  = VOCAL_SUPPRESSION;
    m_champCible = nullptr;
    m_champType  = "";
    // Plus de popup — l'utilisateur appuie directement sur Arduino
}
void WasteRecycling::traiterAudioArduino(const QString &wavPath)
{
    if (m_modeVocal == VOCAL_AUCUN) return;

    QFile *file = new QFile(wavPath, this);
    if (!file->open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Erreur audio",
                              "Impossible de lire le fichier audio Arduino.");
        return;
    }

    QHttpMultiPart *mp = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart modelPart;
    modelPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                        "form-data; name=\"model\"");
    modelPart.setBody("whisper-large-v3-turbo");

    QHttpPart langPart;
    langPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       "form-data; name=\"language\"");
    langPart.setBody("fr");

    QHttpPart audioPart;
    audioPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                        "form-data; name=\"file\"; filename=\"audio.wav\"");
    audioPart.setHeader(QNetworkRequest::ContentTypeHeader, "audio/wav");
    audioPart.setBodyDevice(file);
    file->setParent(mp);

    mp->append(modelPart); mp->append(langPart); mp->append(audioPart);

    QNetworkRequest req(QUrl("https://api.groq.com/openai/v1/audio/transcriptions"));
    req.setRawHeader("Authorization", "Bearer " + m_groqKey.toUtf8());
    QNetworkReply *reply = m_network->post(req, mp);
    mp->setParent(reply);

    ModeVocal modeSnapshot = m_modeVocal;  // capture avant reset
    m_modeVocal = VOCAL_AUCUN;

    connect(reply, &QNetworkReply::finished, this, [this, reply, modeSnapshot]() {
        QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
        reply->deleteLater();
        QString texte = obj["text"].toString().trimmed();
        if (texte.isEmpty()) return;

        if (modeSnapshot == VOCAL_CHAMP)
            remplirChampDepuisTexte(texte);
        else if (modeSnapshot == VOCAL_SUPPRESSION)
            traiterCommandeSuppression(texte);
    });
}
void WasteRecycling::remplirChampDepuisTexte(const QString &texte)
{
    qDebug() << "[VOCAL] texte reçu :" << texte;
    qDebug() << "[VOCAL] type :" << m_champType;
    qDebug() << "[VOCAL] champ cible :" << (m_champCible ? "OK" : "NULL");

    if (!m_champCible) return;

    QString valeur = texte;
    valeur.remove(QRegularExpression("[.،,!?]+$"));

    qDebug() << "[VOCAL] valeur avant traitement :" << valeur;

    if (m_champType == "tel" || m_champType == "trans") {
        QMap<QString, QString> mots = {
            {"zéro","0"}, {"zero","0"},
            {"un","1"}, {"une","1"},
            {"deux","2"}, {"trois","3"},
            {"quatre","4"}, {"cinq","5"},
            {"six","6"}, {"sept","7"},
            {"huit","8"}, {"neuf","9"},
            {"dix","10"}, {"onze","11"},
            {"douze","12"}, {"treize","13"},
            {"quatorze","14"}, {"quinze","15"},
            {"seize","16"}, {"vingt","20"},
            {"trente","30"}, {"quarante","40"},
            {"cinquante","50"}, {"soixante","60"},
            {"cent","100"}
        };
        for (auto it = mots.begin(); it != mots.end(); ++it) {
            QRegularExpression re("\\b" + it.key() + "\\b",
                                  QRegularExpression::CaseInsensitiveOption);
            valeur.replace(re, it.value());
        }
        valeur.remove(QRegularExpression("[^0-9]"));
    }

    qDebug() << "[VOCAL] valeur finale :" << valeur;

    m_champCible->setText(valeur);
    m_champCible = nullptr;
    m_champType  = "";
}
void WasteRecycling::traiterCommandeSuppression(const QString &texte)
{
    QString t = texte.toLower();

    // ── "supprimer tous les clients" ──
    if (t.contains("tous")) {
        if (QMessageBox::question(this, "Suppression vocale",
                                  "Vous êtes sûr de vouloir supprimer TOUS les clients ?",
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            client modele;
            if (modele.supprimerTousClients()) {
                QMessageBox::information(this, "Succès",
                                         "Tous les clients ont été supprimés.");
                chargerClients();
            } else {
                QMessageBox::critical(this, "Erreur", "Échec de la suppression.");
            }
        }
        return;
    }

    // ── "supprimer client ID X" ──
    QRegularExpression reId(R"((\d+))");
    QRegularExpressionMatch m = reId.match(t);
    if (m.hasMatch()) {
        int id = m.captured(1).toInt();
        if (QMessageBox::question(this, "Suppression vocale",
                                  QString("Vous êtes sûr de vouloir supprimer le client ID %1 ?")
                                      .arg(id),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            client modele;
            if (modele.supprimerClient(id)) {
                QMessageBox::information(this, "Succès",
                                         QString("Client ID %1 supprimé.").arg(id));
                chargerClients();
            } else {
                QMessageBox::critical(this, "Erreur",
                                      "Échec — ID introuvable ou erreur BDD.");
            }
        }
        return;
    }

    // ── Commande non reconnue ──
    QMessageBox::warning(this, "Commande non reconnue",
                         "Texte reçu : « " + texte + " »\n\n"
                                                     "Dites :\n"
                                                     "• « Supprimer client ID 5 »\n"
                                                     "• « Supprimer tous les clients »");
}
void WasteRecycling::on_clbtnvocSup_clicked()
{
    attendreArduinoSuppression();
}
void WasteRecycling::refreshTable(employee e) {
    QSqlTableModel* model = e.read_employee();
    ui->empTableView->setModel(model);
    ui->empTableView->setAlternatingRowColors(true);
    ui->empTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->empTableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->empTableView->verticalHeader()->setVisible(false);
    ui->empTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->empTableView->setContextMenuPolicy(Qt::CustomContextMenu);
}

// ********************************* NAVIGATION SIDEBAR ***************************************
WasteRecycling::~WasteRecycling() { delete ui; }

void WasteRecycling::on_GEmpBtn_clicked()   { ui->gestionsPages->setCurrentIndex(PAGE_GEMPLOYE); }
void WasteRecycling::on_GContBtn_clicked()  { ui->gestionsPages->setCurrentIndex(PAGE_GCONTAINER); }
void WasteRecycling::on_GMachinBtn_clicked(){ ui->gestionsPages->setCurrentIndex(PAGE_GMACHINE); }
void WasteRecycling::on_GCamBtn_clicked()   { ui->gestionsPages->setCurrentIndex(PAGE_GCAMION); }
void WasteRecycling::on_GPcBtn_clicked()    { ui->gestionsPages->setCurrentIndex(PAGE_GPOINT_COLLECTE); }
void WasteRecycling::on_GClientBtn_clicked(){ ui->gestionsPages->setCurrentIndex(PAGE_GCLIENT); }

// //////////////////////////////// GESTION CAMION ///////////////////////////////////////
void WasteRecycling::on_crudMet1CamBtn_clicked() {
    QStackedWidget *p = ui->gestionsPages->findChild<QWidget*>("GCamionPage")->findChild<QStackedWidget*>("pagesGcamion");
    if (p) p->setCurrentIndex(PAGE_CAMION_F1); else qDebug() << "Navigation échouée";
}
void WasteRecycling::on_crudMet2CamBtn_clicked() {
    QStackedWidget *p = ui->gestionsPages->findChild<QWidget*>("GCamionPage")->findChild<QStackedWidget*>("pagesGcamion");
    if (p) p->setCurrentIndex(PAGE_CAMION_F2); else qDebug() << "Navigation échouée";
}
void WasteRecycling::on_f1AccCamBtn_clicked() {
    QStackedWidget *p = ui->gestionsPages->findChild<QWidget*>("GCamionPage")->findChild<QStackedWidget*>("pagesGcamion");
    if (p) p->setCurrentIndex(PAGE_CAMION_CRUD); else qDebug() << "Navigation échouée";
}
void WasteRecycling::on_f1Met2CamBtn_clicked() {
    QStackedWidget *p = ui->gestionsPages->findChild<QWidget*>("GCamionPage")->findChild<QStackedWidget*>("pagesGcamion");
    if (p) p->setCurrentIndex(PAGE_CAMION_F2); else qDebug() << "Navigation échouée";
}
void WasteRecycling::on_f2AccCamBtn_clicked() {
    QStackedWidget *p = ui->gestionsPages->findChild<QWidget*>("GCamionPage")->findChild<QStackedWidget*>("pagesGcamion");
    if (p) p->setCurrentIndex(PAGE_CAMION_CRUD); else qDebug() << "Navigation échouée";
}
void WasteRecycling::on_f2Met1CamBtn_clicked() {
    QStackedWidget *p = ui->gestionsPages->findChild<QWidget*>("GCamionPage")->findChild<QStackedWidget*>("pagesGcamion");
    if (p) p->setCurrentIndex(PAGE_CAMION_F1); else qDebug() << "Navigation échouée";
}

void WasteRecycling::resetAddEditCamForm() {
    ui->matCamInp->clear();
    ui->capCamInp->clear();
    ui->conCamInp->clear();
    ui->etaCamInp->setCurrentText("Disponible");
    ui->addCamBtn->setText("Ajouter");
    CamTemp.setCamEditMode(false);
}

void WasteRecycling::on_addCamBtn_clicked() {
    bool ok1, ok2;
    QString mat = ui->matCamInp->text();
    float capacite = ui->capCamInp->text().toFloat(&ok1);
    float conso = ui->conCamInp->text().toFloat(&ok2);
    QString etat = ui->etaCamInp->currentText();
    Camion C(mat, capacite, 0, conso, etat);
    if (!ok1 || !ok2 || mat == "") {
        QMessageBox::critical(nullptr, QMessageBox::tr("Erreur Saisie"),
                              QMessageBox::tr("Veiller remplir tous les champs avec des données valide!"));
    } else if (C.camControleSaisie()) {
        if (CamTemp.getCamEditMode()) {
            QMessageBox::StandardButton reponse = QMessageBox::question(this,
                                                                        QMessageBox::tr("Confirmation"), QMessageBox::tr("Voulez-vous vraiment modifier le camion ?"),
                                                                        QMessageBox::Yes | QMessageBox::No);
            if (reponse == QMessageBox::Yes) {
                CamTemp.setMatricule(mat); CamTemp.setCapacite_max(capacite);
                CamTemp.setConsomation(conso); CamTemp.setStatus(etat);
                if (CamTemp.editCamion()) { ui->tableCam->setModel(CamTemp.showCamion()); resetAddEditCamForm(); }
                else QMessageBox::critical(nullptr, QObject::tr("Probleme Survenu"), QObject::tr("Modification non effectué."));
            } else resetAddEditCamForm();
        } else {
            if (C.fetchCamionByMatricule(mat)) {
                QMessageBox::critical(nullptr, QMessageBox::tr("Erreur saisie"), QMessageBox::tr("le Matricule saisie existe deja!!"));
            } else {
                C.setMatricule(mat); C.setCapacite_max(capacite); C.setRemplissage(0); C.setConsomation(conso); C.setStatus(etat);
                QMessageBox::StandardButton reponse = QMessageBox::question(this,
                                                                            QMessageBox::tr("Confirmation"), QMessageBox::tr("Voulez-vous vraiment ajouter ce camion ?"),
                                                                            QMessageBox::Yes | QMessageBox::No);
                if (reponse == QMessageBox::Yes) {
                    if (C.addCamion()) { ui->tableCam->setModel(CamTemp.showCamion()); resetAddEditCamForm(); }
                    else QMessageBox::critical(nullptr, QObject::tr("Probleme Survenu"), QObject::tr("Ajout non effectué."));
                }
            }
        }
    }
}

void WasteRecycling::on_delCamBtn_clicked() {
    QString mat = ui->delCamInp->text();
    QMessageBox::StandardButton response = QMessageBox::question(this,
                                                                 QMessageBox::tr("Confirmation"), QMessageBox::tr("Voulez-vous vraiment supprimer ce camion ?"),
                                                                 QMessageBox::Yes | QMessageBox::No);
    if (response == QMessageBox::Yes) {
        if (CamTemp.deleteCamion(mat)) { ui->tableCam->setModel(CamTemp.showCamion()); ui->delCamInp->clear(); }
        else QMessageBox::critical(nullptr, QObject::tr("Probleme Survenu"), QObject::tr("Suppression non effectué."));
    }
}

void WasteRecycling::on_tableCam_doubleClicked(const QModelIndex &index) {
    if (!index.isValid()) return;
    if (!CamTemp.getCamEditMode()) {
        int row = index.row();
        QAbstractItemModel *model = ui->tableCam->model();
        QString matricule = model->data(model->index(row, 0)).toString();
        QString capacite  = QString::number(model->data(model->index(row, 1)).toFloat(), 'f', 1);
        QString conso     = QString::number(model->data(model->index(row, 3)).toFloat(), 'f', 1);
        QString etat      = model->data(model->index(row, 4)).toString();
        ui->matCamInp->setText(matricule); ui->capCamInp->setText(capacite);
        ui->conCamInp->setText(conso); ui->etaCamInp->setCurrentText(etat);
        ui->addCamBtn->setText("Modifier");
        CamTemp.fetchCamionByMatricule(matricule); CamTemp.setCamEditMode(true);
    } else resetAddEditCamForm();
}

void WasteRecycling::on_rechCamBtn_clicked() {
    QString mat = ui->rechCamInp->text().trimmed();
    if (mat == "") ui->tableCam->setModel(CamTemp.showCamion());
    else           ui->tableCam->setModel(CamTemp.camRechMat(mat));
}

// ********************************* GESTION CONTENEURS *****************************************
void WasteRecycling::refreshTable() {
    QSqlQueryModel* model = C.afficher();
    ui->GCont_containerTable->setModel(model);
    ui->GCont_containerTable->horizontalHeader()->setStretchLastSection(true);
    ui->GCont_containerTable->resizeColumnsToContents();
}

void WasteRecycling::clearForm() {
    ui->GCont_idInput->clear();
    ui->GCont_typeCombo->setCurrentIndex(0);
    ui->GCont_capacitySpin->setValue(500);
    ui->GCont_currentSpin->setValue(250);
    ui->GCont_thresholdSpin->setValue(80);
    ui->GCont_dateInput->setDate(QDate::currentDate());
    ui->GCont_previewText->setPlainText("Formulaire efface. Entrez de nouvelles informations.");
    ui->GCont_idInput->setFocus();
}

bool WasteRecycling::validateForm() {
    if (ui->GCont_idInput->text().isEmpty()) {
        QMessageBox::warning(this, "Information manquante", "Veuillez saisir un ID de conteneur.");
        ui->GCont_idInput->setFocus(); return false;
    }
    bool ok;
    ui->GCont_idInput->text().toInt(&ok);
    if (!ok) {
        QMessageBox::warning(this, "ID invalide", "L'ID doit etre un nombre.");
        ui->GCont_idInput->setFocus(); return false;
    }
    if (ui->GCont_currentSpin->value() > ui->GCont_capacitySpin->value()) {
        QMessageBox::warning(this, "Donnees invalides", "Le niveau actuel ne peut pas depasser la capacite maximale.");
        ui->GCont_currentSpin->setValue(ui->GCont_capacitySpin->value()); return false;
    }
    return true;
}

Conteneur WasteRecycling::getConteneurFromForm() {
    return Conteneur(
        ui->GCont_idInput->text().toInt(),
        ui->GCont_typeCombo->currentText(),
        ui->GCont_capacitySpin->value(),
        ui->GCont_currentSpin->value(),
        ui->GCont_thresholdSpin->value(),
        ui->GCont_dateInput->date()
        );
}

void WasteRecycling::fillFormFromConteneur(const Conteneur& conteneur) {
    ui->GCont_idInput->setText(QString::number(conteneur.getId()));
    ui->GCont_typeCombo->setCurrentText(conteneur.getType());
    ui->GCont_capacitySpin->setValue(conteneur.getCapaciteMax());
    ui->GCont_currentSpin->setValue(conteneur.getQuantiteActuelle());
    ui->GCont_thresholdSpin->setValue(conteneur.getSeuilAlerte());
    ui->GCont_dateInput->setDate(conteneur.getDateMaj());
    on_GCont_previewButton_clicked();
}

int WasteRecycling::getSelectedId() {
    QModelIndex index = ui->GCont_containerTable->currentIndex();
    if (!index.isValid()) return -1;
    return ui->GCont_containerTable->model()->data(
                                                ui->GCont_containerTable->model()->index(index.row(), 0)).toInt();
}

void WasteRecycling::on_GCont_addButton_clicked() {
    if (!validateForm()) return;
    Conteneur nouveau = getConteneurFromForm();
    if (Conteneur::idExiste(nouveau.getId())) {
        QMessageBox::critical(this, "Erreur", "L'ID " + QString::number(nouveau.getId()) + " existe deja."); return;
    }
    if (nouveau.ajouter()) {
        refreshTable(); clearForm();
        QMessageBox::information(this, "Succès", QString("Conteneur ajoute!\nID: %1\nType: %2\nNiveau: %3%")
                                                     .arg(nouveau.getId()).arg(nouveau.getType()).arg(nouveau.getPourcentageRemplissage()));
    } else {
        Connection& conn = Connection::createInstance();
        if (!conn.getDatabase().isOpen())
            QMessageBox::critical(this, "Erreur de connexion", "La connexion a la base de donnees est perdue.");
        else
            QMessageBox::critical(this, "Erreur", "Echec de l'ajout du conteneur.");
    }
}

void WasteRecycling::on_GCont_refreshButton_clicked() { refreshTable(); }

void WasteRecycling::on_GCont_updateButton_clicked() {
    int id = getSelectedId();
    if (id == -1) { QMessageBox::warning(this, "Selection", "Veuillez selectionner un conteneur."); return; }
    if (!validateForm()) return;
    if (QMessageBox::question(this, "Confirmation", "Modifier le conteneur ID: " + QString::number(id) + "?",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        if (getConteneurFromForm().modifier(id)) { refreshTable(); clearForm(); QMessageBox::information(this, "Succès", "Conteneur modifie!"); }
        else QMessageBox::critical(this, "Erreur", "Echec de la modification.");
    }
}

void WasteRecycling::on_GCont_deleteButton_clicked() {
    int id = getSelectedId();
    if (id == -1) { QMessageBox::warning(this, "Selection", "Veuillez selectionner un conteneur."); return; }
    if (QMessageBox::question(this, "Confirmation", "Supprimer le conteneur ID: " + QString::number(id) + "?",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        Conteneur temp;
        if (temp.supprimer(id)) { refreshTable(); clearForm(); QMessageBox::information(this, "Succès", "Conteneur supprime!"); }
        else QMessageBox::critical(this, "Erreur", "Echec de la suppression.");
    }
}

void WasteRecycling::on_GCont_clearButton_clicked() { clearForm(); }

void WasteRecycling::on_GCont_previewButton_clicked() {
    if (ui->GCont_idInput->text().isEmpty()) {
        ui->GCont_previewText->setPlainText("Veuillez saisir un ID de conteneur."); return;
    }
    Conteneur conteneur = getConteneurFromForm();
    QString preview = "=== APERCU DU CONTENEUR ===\n\n";
    preview += "ID: "             + QString::number(conteneur.getId())              + "\n";
    preview += "Type: "           + conteneur.getType()                             + "\n";
    preview += "Capacite: "       + QString::number(conteneur.getCapaciteMax())     + " L\n";
    preview += "Niveau actuel: "  + QString::number(conteneur.getQuantiteActuelle())+ " L\n";
    preview += "Pourcentage: "    + QString::number(conteneur.getPourcentageRemplissage()) + "%\n";
    preview += "Niveau d'alerte: "+ conteneur.getNiveauAlerte()                     + "\n";
    preview += "Seuil d'alerte: " + QString::number(conteneur.getSeuilAlerte())     + "%\n";
    preview += "Date: "           + conteneur.getDateMaj().toString("dd/MM/yyyy")   + "\n";
    ui->GCont_previewText->setPlainText(preview);
}

void WasteRecycling::on_GCont_sortButton_clicked() {
    ui->GCont_containerTable->sortByColumn(1, Qt::AscendingOrder);
}

void WasteRecycling::on_GCont_searchButton_clicked() {
    QString searchText = ui->GCont_searchEdit->text();
    QString typeFilter = ui->GCont_typeFilterCombo->currentText();
    QString queryStr;
    if (!searchText.isEmpty())
        queryStr = "SELECT * FROM CONTENEURS WHERE LOWER(TYPE) LIKE LOWER('%" + searchText + "%') OR TO_CHAR(ID_CONTENEUR) LIKE '%" + searchText + "%' ORDER BY ID_CONTENEUR";
    else if (typeFilter != "Tous les types")
        queryStr = "SELECT * FROM CONTENEURS WHERE TYPE = '" + typeFilter + "' ORDER BY ID_CONTENEUR";
    else
        queryStr = "SELECT * FROM CONTENEURS ORDER BY ID_CONTENEUR";
    QSqlQueryModel* model = new QSqlQueryModel();
    model->setQuery(queryStr);
    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Type");
    model->setHeaderData(2, Qt::Horizontal, "Capacite Max");
    model->setHeaderData(3, Qt::Horizontal, "Quantite Actuelle");
    model->setHeaderData(4, Qt::Horizontal, "Seuil d'Alerte");
    model->setHeaderData(5, Qt::Horizontal, "Date MAJ");
    ui->GCont_containerTable->setModel(model);
}

void WasteRecycling::on_GCont_containerTable_doubleClicked(const QModelIndex &index) {
    int id = ui->GCont_containerTable->model()->data(
                                                  ui->GCont_containerTable->model()->index(index.row(), 0)).toInt();
    QSqlQuery query;
    query.prepare("SELECT * FROM CONTENEURS WHERE ID_CONTENEUR = :id");
    query.bindValue(":id", id);
    if (query.exec() && query.next()) {
        fillFormFromConteneur(Conteneur(
            query.value("ID_CONTENEUR").toInt(), query.value("TYPE").toString(),
            query.value("CAPACITE_MAX").toInt(), query.value("QUANTITE_ACTUELLE").toInt(),
            query.value("SEUIL_ALERTE").toInt(), query.value("DATE_MAJ").toDate()));
    }
}

// ********************************* GESTION MACHINES *******************************************
void WasteRecycling::on_MRbtn_lancer_clicked() {
    QString mode      = ui->MRbtn_lancer->property("mode").toString();
    QString typeDechet= ui->MRcombo_typeDechet->currentText();
    int     capacite  = ui->MRspinBox_capacite->value();
    QString nomMachine= ui->MRlineEdit_nomMachine->text().trimmed();
    if (nomMachine.isEmpty()) { QMessageBox::warning(this, "Erreur", "Veuillez saisir un nom de machine !"); ui->MRlineEdit_nomMachine->setFocus(); return; }
    if (nomMachine.length() > 30) { QMessageBox::warning(this, "Erreur", "Le nom ne doit pas dépasser 30 caractères !"); ui->MRlineEdit_nomMachine->setFocus(); return; }
    if (capacite <= 0) { QMessageBox::warning(this, "Erreur", "La capacité doit être supérieure à 0 !"); ui->MRspinBox_capacite->setFocus(); return; }
    Machine m;
    m.setTypeDechet(typeDechet); m.setCapacite(capacite); m.setNomMachine(nomMachine);
    if (mode == "modifier") {
        int id = ui->MRbtn_lancer->property("id_modif").toInt();
        if (id <= 0) { QMessageBox::warning(this, "Erreur", "ID invalide"); return; }
        if (m.modifier(id)) {
            QMessageBox::information(this, "Succès", "Machine modifiée");
            QAbstractItemModel *old = ui->MRtable->model(); ui->MRtable->setModel(mach.afficher());
            if (old && old != ui->MRtable->model()) delete old;
            ui->MRbtn_lancer->setText("Ajouter"); ui->MRbtn_lancer->setProperty("mode", "ajout"); ui->MRbtn_lancer->setProperty("id_modif", 0);
            ui->MRcombo_typeDechet->setCurrentIndex(0); ui->MRspinBox_capacite->setValue(0); ui->MRlineEdit_nomMachine->clear();
        } else QMessageBox::warning(this, "Erreur", "Échec de la modification");
    } else {
        if (m.ajouter()) {
            QAbstractItemModel *old = ui->MRtable->model(); ui->MRtable->setModel(mach.afficher());
            if (old && old != ui->MRtable->model()) delete old;
            ui->MRtable->scrollToBottom();
            ui->MRcombo_typeDechet->setCurrentIndex(0); ui->MRspinBox_capacite->setValue(0); ui->MRlineEdit_nomMachine->clear();
            QMessageBox::information(this, "Succès", "Machine ajoutée !");
        } else QMessageBox::critical(this, "Erreur", "Échec de l'ajout");
    }
}

void WasteRecycling::on_MRbtn_supprimer_clicked() {
    QModelIndexList lignes = ui->MRtable->selectionModel()->selectedRows();
    if (lignes.isEmpty()) { QMessageBox::warning(this, "Erreur", "Veuillez sélectionner une machine"); return; }
    int id = ui->MRtable->model()->data(ui->MRtable->model()->index(lignes.first().row(), 0)).toInt();
    if (QMessageBox::question(this, "Confirmation", "Supprimer cette machine ?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;
    Machine m;
    if (m.supprimer(id)) {
        QAbstractItemModel *old = ui->MRtable->model(); ui->MRtable->setModel(mach.afficher());
        if (old && old != ui->MRtable->model()) delete old;
        QMessageBox::information(this, "Succès", "Machine supprimée");
    } else QMessageBox::critical(this, "Erreur", "Échec de la suppression");
}

void WasteRecycling::on_MRtable_doubleClicked(const QModelIndex &index) {
    QAbstractItemModel *model = ui->MRtable->model();
    int row = index.row();
    ui->MRcombo_typeDechet->setCurrentText(model->data(model->index(row, 1)).toString());
    ui->MRlineEdit_nomMachine->setText(model->data(model->index(row, 2)).toString());
    ui->MRspinBox_capacite->setValue(model->data(model->index(row, 3)).toInt());
    ui->MRbtn_lancer->setText("Modifier");
    ui->MRbtn_lancer->setProperty("mode", "modifier");
    ui->MRbtn_lancer->setProperty("id_modif", model->data(model->index(row, 0)).toInt());
    ui->MRlineEdit_nomMachine->setFocus();
}

void WasteRecycling::on_MRtable_clicked(const QModelIndex &index) {
    if (ui->MRbtn_lancer->property("mode").toString() == "modifier") {
        int idClique  = ui->MRtable->model()->data(ui->MRtable->model()->index(index.row(), 0)).toInt();
        int idEnCours = ui->MRbtn_lancer->property("id_modif").toInt();
        if (idClique != idEnCours) {
            ui->MRbtn_lancer->setText("Ajouter"); ui->MRbtn_lancer->setProperty("mode", "ajout"); ui->MRbtn_lancer->setProperty("id_modif", 0);
            ui->MRcombo_typeDechet->setCurrentIndex(0); ui->MRspinBox_capacite->setValue(0); ui->MRlineEdit_nomMachine->clear();
        }
    }
}

void WasteRecycling::on_MRbtn_stats_clicked() {
    QDialog *statsDialog = new QDialog(this);
    statsDialog->setWindowTitle("📊 Statistiques des Machines");
    statsDialog->setFixedSize(700, 550);
    statsDialog->setStyleSheet("background-color: #ECF5E9;");
    statsDialog->setModal(true);
    statsDialog->setAttribute(Qt::WA_DeleteOnClose);
    QVBoxLayout *mainLayout = new QVBoxLayout(statsDialog);
    QLabel *titleLabel = new QLabel("Répartition des machines par type de déchet");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #25665A; padding: 10px;");
    mainLayout->addWidget(titleLabel);
    QSqlQuery query;
    query.exec("SELECT TYPE_DECHET, COUNT(*) FROM MACHINES GROUP BY TYPE_DECHET");
    QPieSeries *series = new QPieSeries();
    int total = 0;
    QMap<QString, int> data;
    while (query.next()) { data[query.value(0).toString()] = query.value(1).toInt(); total += query.value(1).toInt(); }
    for (auto it = data.begin(); it != data.end(); ++it) {
        QPieSlice *slice = series->append(it.key(), it.value());
        double pct = (it.value() * 100.0) / total;
        slice->setLabel(QString("%1\n%2%").arg(it.key()).arg(pct, 0, 'f', 1));
        slice->setLabelVisible(true);
        if      (it.key() == "Plastique") slice->setColor(QColor(52, 152, 219));
        else if (it.key() == "Verre")     slice->setColor(QColor(46, 204, 113));
        else if (it.key() == "Métal")     slice->setColor(QColor(241, 196, 15));
        else                              slice->setColor(QColor(155, 89, 182));
    }
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignRight);
    chart->setBackgroundVisible(false);
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setFixedHeight(350);
    QFrame *detailsFrame = new QFrame();
    detailsFrame->setStyleSheet("background-color: white; border-radius: 15px;");
    detailsFrame->setFixedHeight(120);
    QVBoxLayout *detailsLayout = new QVBoxLayout(detailsFrame);
    QSqlQuery capQuery;
    capQuery.exec("SELECT SUM(CAPACITE), AVG(CAPACITE) FROM MACHINES");
    int capTotale = 0; double capMoyenne = 0;
    if (capQuery.next()) { capTotale = capQuery.value(0).toInt(); capMoyenne = capQuery.value(1).toDouble(); }
    QString detailsText = QString("<b>📊 Résumé :</b><br>• <b>Nombre total :</b> %1 machine(s)<br>• <b>Capacité totale :</b> %2 kg<br>• <b>Capacité moyenne :</b> %3 kg<br><br><b>📈 Répartition :</b><br>")
                              .arg(total).arg(capTotale).arg(QString::number(capMoyenne, 'f', 0));
    for (auto it = data.begin(); it != data.end(); ++it)
        detailsText += QString("• <b>%1 :</b> %2 machine(s) (%3%)<br>").arg(it.key()).arg(it.value()).arg((it.value()*100.0)/total, 0, 'f', 1);
    QLabel *detailsLabel = new QLabel(detailsText);
    detailsLabel->setStyleSheet("color: #25665A; font-size: 12px; padding: 10px;");
    detailsLabel->setWordWrap(true);
    detailsLayout->addWidget(detailsLabel);
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *closeBtn = new QPushButton("Fermer");
    closeBtn->setFixedSize(100, 35);
    closeBtn->setStyleSheet("background-color: #25665A; color: white; border-radius: 8px; font-weight: bold;");
    buttonLayout->addStretch(); buttonLayout->addWidget(closeBtn); buttonLayout->addStretch();
    mainLayout->addWidget(chartView); mainLayout->addWidget(detailsFrame); mainLayout->addLayout(buttonLayout);
    connect(closeBtn, &QPushButton::clicked, statsDialog, &QDialog::close);
    statsDialog->exec();
}

void WasteRecycling::on_MRline_recherche_returnPressed() {
    QString r = ui->MRline_recherche->text().trimmed();
    ui->MRtable->setModel(r.isEmpty() ? mach.afficher() : mach.rechercher(r));
}

void WasteRecycling::on_MRbtn_trier_clicked() {
    ui->MRtable->setModel(mach.trierParType("Tous"));
}

///////////////////////////////////////////////////////////// GPC /////////////////////////////////////////////////////
void WasteRecycling::on_GPC_BtnSave_clicked() {
    PointCollecte pc(0, ui->GPC_FormCmbZone->currentText(), ui->GPC_FormTxtAddress->text(),
                     ui->GPC_FormLevel->value(), ui->GPC_FormDate->date().toString("yyyy-MM-dd"),
                     ui->GPC_FormCmbPriority->currentText());
    if (pc.ajouter()) {
        ui->gpcTableView->setModel(PC.afficher());
        ui->gpcTableView->setItemDelegateForColumn(3, new ProgressBarDelegate(this));
        QMessageBox::information(this, "GPC", "Ajout réussi !");
        ui->GPC_FormCmbZone->setCurrentIndex(0); ui->GPC_FormTxtAddress->clear();
        ui->GPC_FormLevel->setValue(0); ui->GPC_FormDate->setDate(QDate::currentDate());
        ui->GPC_FormCmbPriority->setCurrentIndex(0);
    } else QMessageBox::critical(this, "GPC", "Erreur ajout !");
}

ProgressBarDelegate::ProgressBarDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

QColor ProgressBarDelegate::getFillColor(int niveau) const {
    if (niveau < 50) return QColor(76, 175, 80);
    if (niveau < 80) return QColor(255, 152, 0);
    return QColor(244, 67, 54);
}

void ProgressBarDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    int value = index.data().toInt();
    painter->save();
    QRect rect = option.rect.adjusted(4, 8, -4, -8);
    painter->setPen(Qt::NoPen); painter->setBrush(QColor(217, 217, 217));
    painter->drawRoundedRect(rect, 6, 6);
    QRect progressRect(rect.left(), rect.top(), rect.width() * value / 100, rect.height());
    painter->setBrush(getFillColor(value)); painter->drawRoundedRect(progressRect, 6, 6);
    painter->setPen(Qt::black); painter->drawText(option.rect, Qt::AlignCenter, QString::number(value) + "%");
    painter->restore();
}

void WasteRecycling::on_gpcBtnSupprimer_clicked() {
    QModelIndex index = ui->gpcTableView->currentIndex();
    if (!index.isValid()) { QMessageBox::warning(this, "GPC", "Sélectionne une ligne."); return; }
    int id = ui->gpcTableView->model()->index(index.row(), 0).data().toInt();
    if (PC.supprimer(id)) {
        ui->gpcTableView->setModel(PC.afficher());
        ui->gpcTableView->setItemDelegateForColumn(3, new ProgressBarDelegate(this));
        QMessageBox::information(this, "GPC", "Suppression réussie !");
    } else QMessageBox::critical(this, "GPC", "Erreur suppression !");
}

void WasteRecycling::on_gpcTableView_doubleClicked(const QModelIndex &index) {
    int row = index.row();
    gpcSelectedId = ui->gpcTableView->model()->index(row, 0).data().toInt();
    ui->GPC_FormCmbZone->setCurrentText(ui->gpcTableView->model()->index(row, 1).data().toString());
    ui->GPC_FormTxtAddress->setText(ui->gpcTableView->model()->index(row, 2).data().toString());
    ui->GPC_FormLevel->setValue(ui->gpcTableView->model()->index(row, 3).data().toInt());
    QDate d = QDate::fromString(ui->gpcTableView->model()->index(row, 4).data().toString().left(10), "yyyy-MM-dd");
    if (d.isValid()) ui->GPC_FormDate->setDate(d);
    ui->GPC_FormCmbPriority->setCurrentText(ui->gpcTableView->model()->index(row, 5).data().toString());
    QMessageBox::information(this, "Mode édition", "Modifie les champs puis clique sur Modifier.");
}

void WasteRecycling::on_GPC_BtnModifier_clicked() {
    if (gpcSelectedId == -1) { QMessageBox::warning(this, "GPC", "Sélectionne une ligne."); return; }
    PointCollecte pc(0, ui->GPC_FormCmbZone->currentText(), ui->GPC_FormTxtAddress->text(),
                     ui->GPC_FormLevel->value(), ui->GPC_FormDate->date().toString("yyyy-MM-dd"),
                     ui->GPC_FormCmbPriority->currentText());
    if (pc.modifier(gpcSelectedId)) {
        ui->gpcTableView->setModel(PC.afficher());
        ui->gpcTableView->setItemDelegateForColumn(3, new ProgressBarDelegate(this));
        QMessageBox::information(this, "GPC", "Modification réussie !");
        gpcSelectedId = -1;
        ui->GPC_FormCmbZone->setCurrentIndex(0); ui->GPC_FormTxtAddress->clear();
        ui->GPC_FormLevel->setValue(0); ui->GPC_FormDate->setDate(QDate::currentDate());
        ui->GPC_FormCmbPriority->setCurrentIndex(0);
    } else QMessageBox::critical(this, "GPC", "Erreur modification !");
}

void WasteRecycling::on_GPC_TxtSearch_textChanged(const QString &text) {
    ui->gpcTableView->setModel(PC.rechercher(text));
}

///////////////////////////////////////////////////////////// CLIENT — CRUD /////////////////////////////////////////////////////

// Charge tous les clients depuis la BDD et les affiche dans le tableau
void WasteRecycling::chargerClients() {
    QAbstractItemModel *old = ui->cllistec->model();
    ui->cllistec->setModel(nullptr);
    delete old;
    m_model = nullptr;
    m_model = ClientTemp.afficherClient();
    if (m_model->lastError().isValid()) {
        QMessageBox::critical(this, "Erreur BDD", "Impossible de charger les clients :\n" + m_model->lastError().text()); return;
    }
    ui->cllistec->setModel(m_model);
    ui->cllistec->resizeColumnsToContents();
    ui->cllistec->setColumnHidden(6, true);
}

// Retourne l'id_client de la ligne sélectionnée (-1 si aucune)
int WasteRecycling::selectedId() const {
    QModelIndex idx = ui->cllistec->currentIndex();
    if (!idx.isValid()) return -1;
    return ui->cllistec->model()->data(ui->cllistec->model()->index(idx.row(), 0)).toInt();
}

// Valide le format de l'email avec l'expression régulière RE_EMAIL
bool WasteRecycling::isValidEmail(const QString &email) {
    return RE_EMAIL.match(email).hasMatch();
}
void WasteRecycling::on_clbtnajt_clicked()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Ajouter un client");
    dialog.setMinimumWidth(450);
    dialog.setStyleSheet(
        "QDialog{background:white;} QLabel{color:black;font-size:13px;}"
        "QLineEdit{color:black;background:white;border:1px solid #aaa;border-radius:4px;padding:4px 6px;font-size:13px;}"
        "QLineEdit:focus{border:1.5px solid #185FA5;}"
        "QComboBox{color:black;background:white;border:1px solid #aaa;border-radius:4px;padding:4px 6px;font-size:13px;}"
        "QComboBox QAbstractItemView{background:white;color:black;}"
        "QPushButton{font-size:13px;padding:6px 18px;border-radius:4px;border:1px solid #aaa;background:white;color:black;}"
        "QPushButton#btnOk{background:#185FA5;color:white;border-color:#185FA5;}"
        "QPushButton#btnOk:hover{background:#0C447C;}"
        "QPushButton#btnMic{background:#f0f0f0;border:1px solid #ccc;padding:4px 8px;border-radius:4px;font-size:16px;min-width:30px;}"
        "QPushButton#btnMic:hover{background:#e0e0e0;}"
        "QPushButton#btnAnn:hover{background:#f0f0f0;}");

    QFormLayout *form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight);
    form->setContentsMargins(20, 20, 20, 10);
    form->setVerticalSpacing(10);

    // ── Champs ──
    QLineEdit *fNom   = new QLineEdit();
    QLineEdit *fTel   = new QLineEdit();
    QLineEdit *fAdr   = new QLineEdit();
    QLineEdit *fEmail = new QLineEdit();
    QLineEdit *fTrans = new QLineEdit("0");
    QComboBox *fType = new QComboBox();
    fType->addItems({"municipalite", "entreprise", "individu"});

    QWidget *typeRowAjt = new QWidget();
    QHBoxLayout *typeLayoutAjt = new QHBoxLayout(typeRowAjt);
    typeLayoutAjt->setContentsMargins(0, 0, 0, 0);
    typeLayoutAjt->setSpacing(6);
    QPushButton *micType = new QPushButton("🎤");
    micType->setObjectName("btnMic");
    micType->setFixedWidth(34);
    typeLayoutAjt->addWidget(fType);
    typeLayoutAjt->addWidget(micType);

    auto normaliserType = [](QString t) -> QString {
        t = t.toLower().trimmed();
        t.replace("é","e").replace("è","e").replace("ê","e")
            .replace("à","a").replace("â","a");
        t.remove(QRegularExpression("[^a-z]"));
        if (t.contains("muni"))                       return "municipalite";
        if (t.contains("entre")||t.contains("antre")) return "entreprise";
        if (t.contains("indiv")||t.contains("endiv")) return "individu";
        return "";
    };

    connect(micType, &QPushButton::clicked, this, [=]() {
        micType->setText("⏺"); micType->setEnabled(false);
        if (m_recorder) { m_recorder->stop(); delete m_recorder; m_recorder = nullptr; }
        if (m_session)  { delete m_session;   m_session  = nullptr; }
        m_session  = new QMediaCaptureSession(this);
        m_recorder = new QMediaRecorder(this);
        QAudioInput *audioInput = new QAudioInput(this);
        m_session->setAudioInput(audioInput);
        m_session->setRecorder(m_recorder);
        QMediaFormat format; format.setFileFormat(QMediaFormat::Wave);
        m_recorder->setMediaFormat(format);
        m_recorder->setAudioSampleRate(16000); m_recorder->setAudioChannelCount(1);
        m_recorder->setOutputLocation(QUrl::fromLocalFile(m_wavPath));
        m_recorder->record();
        QTimer::singleShot(3000, this, [=]() {
            if (m_recorder) m_recorder->stop();
            micType->setText("🎤"); micType->setEnabled(true);
            QFileInfo fi(m_wavPath);
            if (!fi.exists() || fi.size() < 1000) { QMessageBox::warning(this, "Micro", "Trop court."); return; }
            QFile *file = new QFile(m_wavPath, this);
            if (!file->open(QIODevice::ReadOnly)) return;
            QHttpMultiPart *mp = new QHttpMultiPart(QHttpMultiPart::FormDataType);
            QHttpPart modelPart; modelPart.setHeader(QNetworkRequest::ContentDispositionHeader, "form-data; name=\"model\""); modelPart.setBody("whisper-large-v3-turbo");
            QHttpPart langPart;  langPart.setHeader(QNetworkRequest::ContentDispositionHeader,  "form-data; name=\"language\""); langPart.setBody("fr");
            QHttpPart audioPart; audioPart.setHeader(QNetworkRequest::ContentDispositionHeader, "form-data; name=\"file\"; filename=\"audio.wav\"");
            audioPart.setHeader(QNetworkRequest::ContentTypeHeader, "audio/wav"); audioPart.setBodyDevice(file); file->setParent(mp);
            mp->append(modelPart); mp->append(langPart); mp->append(audioPart);
            QNetworkRequest req(QUrl("https://api.groq.com/openai/v1/audio/transcriptions"));
            req.setRawHeader("Authorization", "Bearer " + m_groqKey.toUtf8());
            QNetworkReply *reply = m_network->post(req, mp); mp->setParent(reply);
            connect(reply, &QNetworkReply::finished, this, [=](){
                QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object(); reply->deleteLater();
                QString typeNorm = normaliserType(obj["text"].toString().trimmed());
                if (!typeNorm.isEmpty()) fType->setCurrentText(typeNorm);
                else QMessageBox::warning(this, "Type non reconnu", "Dites : municipalité, entreprise ou individu");
            });
        });
    });
    fNom->setValidator(new QRegularExpressionValidator(RE_NOM, &dialog));
    fTel->setValidator(new QRegularExpressionValidator(RE_TEL, &dialog));
    fTrans->setValidator(new QIntValidator(0, 1'000'000, &dialog));

    // ── Helper : crée widget champ + bouton 🎤 (flow Arduino) ──
    auto makeArduinoRow = [&](QLineEdit *field, const QString &type) -> QWidget* {
        QWidget *row = new QWidget();
        QHBoxLayout *hl = new QHBoxLayout(row);
        hl->setContentsMargins(0, 0, 0, 0);
        hl->setSpacing(6);
        QPushButton *mic = new QPushButton("🎤");
        mic->setObjectName("btnMic");
        mic->setFixedWidth(34);
        mic->setToolTip("Dicter ce champ via Arduino");
        hl->addWidget(field);
        hl->addWidget(mic);
        connect(mic, &QPushButton::clicked, this, [this, field, type]() {
            attendreArduino(field, type);
        });
        return row;
    };

    // ── Construction du formulaire ──
    form->addRow("Nom :",             makeArduinoRow(fNom,   ""));
    form->addRow("Type :",            typeRowAjt);       // ComboBox + micro existant, inchangé
    form->addRow("Téléphone :",       makeArduinoRow(fTel,   "tel"));
    form->addRow("Adresse :",         makeArduinoRow(fAdr,   ""));
    form->addRow("Email :",           makeArduinoRow(fEmail, "email"));
    form->addRow("Nb transactions :", makeArduinoRow(fTrans, "trans"));

    QPushButton *btnOk  = new QPushButton("Ajouter");  btnOk->setObjectName("btnOk");
    QPushButton *btnAnn = new QPushButton("Annuler");   btnAnn->setObjectName("btnAnn");
    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch(); btnRow->addWidget(btnAnn); btnRow->addWidget(btnOk);
    btnRow->setContentsMargins(20, 0, 20, 16);

    QVBoxLayout *mainL = new QVBoxLayout(&dialog);
    mainL->setSpacing(8); mainL->addLayout(form); mainL->addLayout(btnRow);

    connect(btnOk, &QPushButton::clicked, this, [&]()
            {

                if (fNom->text().trimmed().isEmpty())                         { QMessageBox::warning(&dialog,"Ajouter","Le nom est obligatoire.");     fNom->setFocus();   return; }
                if (!fEmail->text().isEmpty() && !isValidEmail(fEmail->text())){ QMessageBox::warning(&dialog,"Ajouter","Email invalide.");            fEmail->setFocus(); return; }
                if (!fTel->text().isEmpty() && !fTel->hasAcceptableInput())   { QMessageBox::warning(&dialog,"Ajouter","Téléphone invalide.");        fTel->setFocus();   return; }
                dialog.accept();
            });
    connect(btnAnn, &QPushButton::clicked, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted) return;


    static const QStringList types = {"municipalite", "entreprise", "individu"};
    client nouveau(0, fNom->text(), types[fType->currentIndex()],
                   fAdr->text(), 0, fEmail->text(), fTel->text(), fTrans->text().toInt());
    if (nouveau.ajouterClient()) {
        QMessageBox::information(this, "Ajouter", "Client ajouté avec succès.");
        chargerClients();
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de l'ajout.\n\nDétail : " +
                                                  QSqlDatabase::database().lastError().text());
    }
}
// Modifier un client — ouvre un dialogue pré-rempli avec les données actuelles
void WasteRecycling::on_clbtnmodif_clicked() {
    int id = selectedId();
    if (id < 0) { QMessageBox::warning(this, "Modifier", "Veuillez sélectionner un client."); return; }
    QModelIndex cur = ui->cllistec->currentIndex();
    int row = cur.row();
    auto val = [&](int col) { return ui->cllistec->model()->data(ui->cllistec->model()->index(row, col)).toString(); };
    QDialog dialog(this);
    dialog.setWindowTitle(QString("Modifier client ID %1").arg(id));
    dialog.setMinimumWidth(400);
    dialog.setStyleSheet(
        "QDialog{background:white;} QLabel{color:black;font-size:13px;}"
        "QLineEdit{color:black;background:white;border:1px solid #aaa;border-radius:4px;padding:4px 6px;font-size:13px;}"
        "QLineEdit:focus{border:1.5px solid #185FA5;}"
        "QComboBox{color:black;background:white;border:1px solid #aaa;border-radius:4px;padding:4px 6px;font-size:13px;}"
        "QComboBox QAbstractItemView{background:white;color:black;}"
        "QPushButton{font-size:13px;padding:6px 18px;border-radius:4px;border:1px solid #aaa;background:white;color:black;}"
        "QPushButton#btnOk{background:#185FA5;color:white;border-color:#185FA5;}"
        "QPushButton#btnOk:hover{background:#0C447C;}"
        "QPushButton#btnAnn:hover{background:#f0f0f0;}");
    QFormLayout *form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight); form->setContentsMargins(20,20,20,10); form->setVerticalSpacing(10);
    QLineEdit *fNom = new QLineEdit(val(1)), *fTel = new QLineEdit(val(3));
    QLineEdit *fAdr = new QLineEdit(val(4)), *fEmail = new QLineEdit(val(5));
    QLineEdit *fTrans = new QLineEdit(val(7));

    // ── Remplacement du champ Type dans le formulaire ──


    // Widget visible = ComboBox + bouton micro côte à côte
    QWidget *typeRow = new QWidget();
    QHBoxLayout *typeLayout = new QHBoxLayout(typeRow);
    typeLayout->setContentsMargins(0, 0, 0, 0);
    typeLayout->setSpacing(6);

    QComboBox *fType = new QComboBox();
    fType->addItems({"municipalite", "entreprise", "individu"});

    QPushButton *micType = new QPushButton("🎤");
    micType->setObjectName("btnMic");
    micType->setFixedWidth(34);
    micType->setToolTip("Dicter le type");

    typeLayout->addWidget(fType);
    typeLayout->addWidget(micType);

    auto makeArduinoRow = [&](QLineEdit *field, const QString &type) -> QWidget* {
        QWidget *row = new QWidget();
        QHBoxLayout *hl = new QHBoxLayout(row);
        hl->setContentsMargins(0, 0, 0, 0);
        hl->setSpacing(6);
        QPushButton *mic = new QPushButton("🎤");
        mic->setObjectName("btnMic");
        mic->setFixedWidth(34);
        hl->addWidget(field);
        hl->addWidget(mic);
        connect(mic, &QPushButton::clicked, this, [this, field, type]() {
            attendreArduino(field, type);
        });
        return row;
    };

    form->addRow("Nom :",             makeArduinoRow(fNom,   ""));
    form->addRow("Type :",            typeRow);           // ComboBox + micro existant, inchangé
    form->addRow("Téléphone :",       makeArduinoRow(fTel,   "tel"));
    form->addRow("Adresse :",         makeArduinoRow(fAdr,   ""));
    form->addRow("Email :",           makeArduinoRow(fEmail, "email"));
    form->addRow("Nb transactions :", makeArduinoRow(fTrans, "trans"));

    QPushButton *btnOk = new QPushButton("Confirmer"); btnOk->setObjectName("btnOk");
    QPushButton *btnAnn = new QPushButton("Annuler"); btnAnn->setObjectName("btnAnn");
    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch(); btnRow->addWidget(btnAnn); btnRow->addWidget(btnOk); btnRow->setContentsMargins(20,0,20,16);
    QVBoxLayout *mainL = new QVBoxLayout(&dialog);
    mainL->setSpacing(8); mainL->addLayout(form); mainL->addLayout(btnRow);
    connect(btnOk, &QPushButton::clicked, this, [&]() {
        if (fNom->text().trimmed().isEmpty())  { QMessageBox::warning(&dialog,"Modifier","Le nom est obligatoire.");     fNom->setFocus();   return; }
        if (fAdr->text().trimmed().isEmpty())  { QMessageBox::warning(&dialog,"Modifier","L'adresse est obligatoire."); fAdr->setFocus();   return; }
        if (fEmail->text().trimmed().isEmpty()){ QMessageBox::warning(&dialog,"Modifier","L'email est obligatoire.");   fEmail->setFocus(); return; }
        if (!isValidEmail(fEmail->text()))     { QMessageBox::warning(&dialog,"Modifier","Email invalide.");            fEmail->setFocus(); return; }
        if (fTel->text().trimmed().isEmpty())  { QMessageBox::warning(&dialog,"Modifier","Le téléphone est obligatoire."); fTel->setFocus(); return; }
        if (!fTel->hasAcceptableInput())       { QMessageBox::warning(&dialog,"Modifier","Téléphone invalide.");        fTel->setFocus();   return; }
        dialog.accept();
    });
    connect(btnAnn, &QPushButton::clicked, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted) return;
    static const QStringList types = {"municipalite", "entreprise", "individu"};
    client modifie(id, fNom->text(), types[fType->currentIndex()], fAdr->text(),
                   0, fEmail->text(), fTel->text(), fTrans->text().toInt());
    if (modifie.modifierClient()) {
        QMessageBox::information(this, "Modifier", "Client modifié avec succès.");
        chargerClients();
        // Mémorise l'ID APRÈS succès → Arduino peut maintenant enregistrer
        m_clientIdEnCours = QString::number(id);
        QMessageBox::information(this, "Note vocale Arduino",
                                 "Client modifié !\n\nAppuyez maintenant sur le bouton Arduino\npour enregistrer une note vocale.");
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de la modification en base de données.");
    }
}

// Supprimer un client sélectionné ou tous les clients si rien n'est sélectionné
void WasteRecycling::on_clbtnsup_clicked() {
    int id = selectedId();
    if (id < 0) {
        if (m_model && m_model->rowCount() == 0) { QMessageBox::information(this, "Info", "La liste est déjà vide."); return; }
        if (QMessageBox::question(this, "Suppression", "Supprimer TOUS les clients ?") != QMessageBox::Yes) return;
        client modele;
        if (modele.supprimerTousClients()) { QMessageBox::information(this, "Suppression", "Tous supprimés."); chargerClients(); }
        else QMessageBox::critical(this, "Erreur", "Échec.");
        return;
    }
    if (QMessageBox::question(this, "Suppression", QString("Supprimer le client ID %1 ?").arg(id)) != QMessageBox::Yes) return;
    client modele;
    if (modele.supprimerClient(id)) { QMessageBox::information(this, "Suppression", "Client supprimé."); chargerClients(); }
    else QMessageBox::critical(this, "Erreur", "Échec de la suppression.");
}

// Double-clic sur une ligne → suppression directe du client
void WasteRecycling::on_cllistec_doubleClicked(const QModelIndex &index) {
    if (!index.isValid()) return;
    int id = ui->cllistec->model()->data(ui->cllistec->model()->index(index.row(), 0)).toInt();
    if (QMessageBox::question(this, "Suppression", QString("Supprimer le client ID %1 ?").arg(id)) == QMessageBox::Yes) {
        client modele;
        if (modele.supprimerClient(id)) { QMessageBox::information(this, "Suppression", "Client supprimé."); chargerClients(); }
        else QMessageBox::critical(this, "Erreur", "Échec.");
    }
}

///////////////////////////////////////////////////////////// CLIENT — METIERS /////////////////////////////////////////////////////

// Recherche en temps réel : filtre le tableau à chaque frappe
void WasteRecycling::on_clsearch_textChanged(const QString &text) {
    if (text.trimmed().isEmpty()) { chargerClients(); return; }
    QAbstractItemModel *old = ui->cllistec->model();
    ui->cllistec->setModel(nullptr);
    delete old;
    ui->cllistec->setModel(ClientTemp.rechercherClient(text)); // appelle rechercherClient() dans client.cpp
    ui->cllistec->resizeColumnsToContents();
    ui->cllistec->setColumnHidden(6, true);
}

// Tri : trie le tableau selon la colonne choisie dans le combobox
void WasteRecycling::on_clbtntri_clicked() {
    int col = ui->clcombosort->currentIndex();
    QAbstractItemModel *old = ui->cllistec->model();
    ui->cllistec->setModel(nullptr);
    delete old;
    m_model = ClientTemp.trierClients(col); // appelle trierClients() dans client.cpp
    ui->cllistec->setModel(m_model);
    ui->cllistec->resizeColumnsToContents();
    QMessageBox::information(this, "Tri", "Trié par : " + ui->clcombosort->currentText());
    ui->cllistec->setColumnHidden(6, true);
}

// Export PDF : génère un fichier PDF du tableau affiché
void WasteRecycling::on_clbtnpdf_clicked() {
    if (!m_model || m_model->rowCount() == 0) { QMessageBox::information(this, "Export PDF", "Aucun client à exporter."); return; }
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter en PDF",
                                                    "clients_" + QDate::currentDate().toString("yyyyMMdd") + ".pdf", "PDF (*.pdf)");
    if (fileName.isEmpty()) return;
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageOrientation(QPageLayout::Landscape);
    QAbstractItemModel *displayed = ui->cllistec->model();
    int n = displayed->rowCount();
    QString html =
        "<html><head><meta charset='utf-8'><style>"
        "body{font-family:Arial,sans-serif;font-size:10pt;color:#111;}"
        "h2{color:#185FA5;margin:0 0 4px;font-size:16pt;}"
        ".meta{font-size:9pt;color:#666;margin-bottom:16px;}"
        "table{width:100%;border-collapse:collapse;margin-top:8px;}"
        "th{background:#185FA5;color:white;padding:6px 10px;text-align:left;font-size:9pt;}"
        "td{padding:5px 10px;border-bottom:1px solid #ddd;font-size:9pt;}"
        "tr:nth-child(even) td{background:#f0f5fb;}"
        "</style></head><body>"
        "<h2>Liste des Clients</h2>"
        "<p class='meta'>Date : <b>" + QDate::currentDate().toString("dd/MM/yyyy") +
        "</b> &nbsp;|&nbsp; " + QString::number(n) + " client(s)</p>"
                                                     "<table><thead><tr>"
                                                     "<th>ID</th><th>Nom</th><th>Type</th><th>Téléphone</th>"
                                                     "<th>Adresse</th><th>Email</th><th>Points</th><th>Nb transactions</th>"
                                                     "</tr></thead><tbody>";
    for (int i = 0; i < n; i++) {
        auto v = [&](int col) { return displayed->data(displayed->index(i, col)).toString().toHtmlEscaped(); };
        html += "<tr><td>" + v(0) + "</td><td><b>" + v(1) + "</b></td><td>" + v(2) + "</td><td>" + v(3) +
                "</td><td>" + v(4) + "</td><td>" + v(5) + "</td>"
                                                          "<td style='text-align:center'>" + v(6) + "</td><td style='text-align:center'>" + v(7) + "</td></tr>";
    }
    html += "</tbody></table></body></html>";
    QTextDocument doc; doc.setHtml(html); doc.print(&printer);
    QMessageBox::information(this, "Export PDF", "PDF exporté !\n" + fileName);
}

// Statistiques : affiche un camembert de répartition par type de client
void WasteRecycling::on_clbtnstat_clicked() {
    if (!m_model || m_model->rowCount() == 0) { QMessageBox::information(this, "Statistiques", "Aucun client."); return; }
    int nb[3] = {0, 0, 0};
    for (int i = 0; i < m_model->rowCount(); i++) {
        QString type = m_model->data(m_model->index(i, 2)).toString().toLower();
        if      (type == "municipalite") nb[0]++;
        else if (type == "entreprise")   nb[1]++;
        else                             nb[2]++;
    }
    static const QStringList labels = {"Municipalité", "Entreprise", "Individu"};
    QPieSeries *series = new QPieSeries();
    for (int i = 0; i < 3; i++)
        if (nb[i] > 0) series->append(QString("%1 (%2)").arg(labels[i]).arg(nb[i]), nb[i]);
    for (auto *s : series->slices()) s->setLabelVisible(true);
    QChart *chart = new QChart();
    chart->addSeries(series); chart->setTitle("Répartition des clients"); chart->legend()->setAlignment(Qt::AlignBottom);
    QChartView *view = new QChartView(chart); view->setRenderHint(QPainter::Antialiasing);
    QLayoutItem *child;
    while ((child = ui->verticalLayout_stat->takeAt(0))) { delete child->widget(); delete child; }
    ui->verticalLayout_stat->addWidget(view);
    ui->stackedWidgetclient->setCurrentIndex(PAGE_STATCLIENT);
}

// Messagerie : initialise la liste + génère un message personnalisé par client
void WasteRecycling::setupMessagerieIA() {
    ui->cl_btnGenerer->setEnabled(false); // désactivé jusqu'à sélection d'un client

    // Charge la liste des clients (id+nom) depuis client.cpp
    QList<QPair<int,QString>> liste = client::getListeNomsClients();
    for (const QPair<int,QString> &entry : liste) {
        QListWidgetItem *item = new QListWidgetItem(entry.second); // affiche le nom
        item->setData(Qt::UserRole, entry.first);                  // stocke l'id en caché
        ui->cl_listeMsg->addItem(item);
    }

    // Clic sur un client → génère le message personnalisé
    connect(ui->cl_listeMsg, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        int clientId = item->data(Qt::UserRole).toInt();
        QString nom  = item->text();

        // Récupère email + points + type depuis client.cpp (pas de SQL ici)
        client::ClientMailInfo info = client::getClientMailInfo(clientId);
        if (info.email.isEmpty() && info.points == 0 && info.type.isEmpty()) return;

        QString email = info.email;
        int     pts   = info.points;
        QString type  = info.type;

        // Calcul du niveau de fidélité
        QString niveau, reduction;
        if      (pts < 100) { niveau = "Bronze"; reduction = "5%";  }
        else if (pts < 300) { niveau = "Silver"; reduction = "10%"; }
        else if (pts < 600) { niveau = "Gold";   reduction = "15%"; }
        else                { niveau = "VIP";    reduction = "20%"; }

        // Calcul de l'état du stock des conteneurs
        int qteActuelle = 0, capMax = 0;
        client::getStockConteneurs(qteActuelle, capMax);
        int pourcentage = (capMax > 0) ? (qteActuelle * 100 / capMax) : 0;
        QString etatStock;
        if      (pourcentage == 0)  etatStock = "malheureusement en rupture de stock pour le moment";
        else if (pourcentage < 25)  etatStock = "actuellement en stock faible, une collecte sera bientôt disponible";
        else if (pourcentage < 65)  etatStock = "disponible normalement pour planifier votre collecte";
        else                        etatStock = "bien rempli et disponible pour votre collecte";

        // Construction du message personnalisé
        QString message =
            "Bonjour " + nom + ",\n\n"
                               "Nous avons le plaisir de vous informer que, en tant que client "
            + type + " fidèle de WasteRecycling, vous bénéficiez actuellement "
                     "d'une réduction exclusive de " + reduction + " sur nos services.\n\n"
                          "Cette réduction est attribuée grâce à votre niveau de fidélité : "
            + niveau + " (" + QString::number(pts) + " points accumulés).\n\n"
                                                     "Concernant notre service de collecte, notre stock est " + etatStock + ".\n\n"
                          "Nous vous remercions de votre confiance et restons à votre disposition "
                          "pour toute question ou planification de collecte.\n\n"
                          "Cordialement,\nL'équipe WasteRecycling";

        ui->cl_messageIA->setText(message);
        ui->cl_lblClientIA->setText("Client : " + nom + " | " + email);
        ui->cl_btnGenerer->setEnabled(!email.isEmpty()); // active si email existe
        ui->cl_btnGenerer->setProperty("email", email); // stocke email dans propriété bouton
        ui->cl_btnGenerer->setProperty("nom", nom);
    });

    // Bouton Envoyer → ouvre le client mail avec l'URL mailto
    connect(ui->cl_btnGenerer, &QPushButton::clicked, this, [this]() {
        QString email   = ui->cl_btnGenerer->property("email").toString();
        QString message = ui->cl_messageIA->toPlainText();
        if (email.isEmpty()) { QMessageBox::warning(this, "Email", "Ce client n'a pas d'adresse email."); return; }
        QString sujet  = "WasteRecycling - Votre réduction fidélité";
        QString mailto = "mailto:" + email +
                         "?subject=" + QUrl::toPercentEncoding(sujet) +
                         "&body="    + QUrl::toPercentEncoding(message);
        QDesktopServices::openUrl(QUrl(mailto)); // délègue l'envoi au client mail du système
    });
}

// Fidélisation : charge les niveaux et remplit le tableau avec couleurs
void WasteRecycling::chargerFidelite() {
    // Comptage par niveau depuis client.cpp
    int bronze = 0, silver = 0, gold = 0, vip = 0;
    client::getStatsFidelite(bronze, silver, gold, vip);
    ui->cl_lblBronzeCount->setText(QString::number(bronze));
    ui->cl_lblSilverCount->setText(QString::number(silver));
    ui->cl_lblGoldCount->setText(QString::number(gold));
    ui->cl_lblVipCount->setText(QString::number(vip));

    // Préparation du tableau
    ui->cl_tableFidelite->clear();
    ui->cl_tableFidelite->setColumnCount(4);
    ui->cl_tableFidelite->setHorizontalHeaderLabels({"Nom", "Points", "Niveau", "Récompense"});
    ui->cl_tableFidelite->horizontalHeader()->setStretchLastSection(true);
    ui->cl_tableFidelite->verticalHeader()->setVisible(false);
    ui->cl_tableFidelite->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->cl_tableFidelite->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Remplissage ligne par ligne depuis client.cpp (triés par points DESC)
    QList<client::ClientFideliteInfo> liste = client::getClientsFideliteData();
    int row = 0;
    for (const client::ClientFideliteInfo &data : liste) {
        QString nom = data.nom;
        int     pts = data.points;
        QString niveau, recompense;
        QColor  couleur;
        if      (pts < 100) { niveau = "Bronze"; recompense = "Reduction 5%";  couleur = QColor("#f5deb3"); }
        else if (pts < 300) { niveau = "Silver"; recompense = "Reduction 10%"; couleur = QColor("#e8e8e8"); }
        else if (pts < 600) { niveau = "Gold";   recompense = "Reduction 15%"; couleur = QColor("#fff4cc"); }
        else                { niveau = "VIP";    recompense = "Reduction 20%"; couleur = QColor("#e8d5ff"); }

        ui->cl_lblBronzeCount->setAlignment(Qt::AlignCenter);
        ui->cl_lblSilverCount->setAlignment(Qt::AlignCenter);
        ui->cl_lblGoldCount->setAlignment(Qt::AlignCenter);
        ui->cl_lblVipCount->setAlignment(Qt::AlignCenter);
        ui->cl_lblBronzeCount->setText("Bronze : " + QString::number(bronze));
        ui->cl_lblSilverCount->setText("Silver : " + QString::number(silver));
        ui->cl_lblGoldCount->setText("Gold : "     + QString::number(gold));
        ui->cl_lblVipCount->setText("VIP : "       + QString::number(vip));

        ui->cl_tableFidelite->insertRow(row);
        ui->cl_tableFidelite->setItem(row, 0, new QTableWidgetItem(nom));
        ui->cl_tableFidelite->setItem(row, 1, new QTableWidgetItem(QString::number(pts) + " pts"));
        ui->cl_tableFidelite->setItem(row, 2, new QTableWidgetItem(niveau));
        ui->cl_tableFidelite->setItem(row, 3, new QTableWidgetItem(recompense));
        for (int col = 0; col < 4; col++)
            ui->cl_tableFidelite->item(row, col)->setBackground(couleur); // couleur par niveau
        row++;
    }
}

// Génère un message local de fidélité (version simplifiée sans stock)
QString WasteRecycling::genererMessageLocal(QString nom, int points) {
    float reduction = (points < 100) ? 5 : (points < 300) ? 10 : (points < 600) ? 15 : 20;
    return "Objet : Offre de fidélité\n\nBonjour " + nom + ",\n\n"
                                                           "Vous bénéficiez d'une réduction de " + QString::number(reduction) + "% grâce à votre fidélité.\n\n"
                                          "Vos points : " + QString::number(points) + ".\n\n"
                                       "Cordialement,\nL'équipe WasteRecycling";
}
void WasteRecycling::envoyerAudioWhisper(const QString &wavPath)
{
    QFile *file = new QFile(wavPath, this);
    if (!file->open(QIODevice::ReadOnly)) return;

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart modelPart;
    modelPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                        "form-data; name=\"model\"");
    modelPart.setBody("whisper-large-v3-turbo");

    QHttpPart audioPart;
    audioPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                        "form-data; name=\"file\"; filename=\"audio.wav\"");
    audioPart.setHeader(QNetworkRequest::ContentTypeHeader, "audio/wav");
    audioPart.setBodyDevice(file);
    file->setParent(multiPart);

    multiPart->append(modelPart);
    multiPart->append(audioPart);

    QNetworkRequest req(QUrl("https://api.groq.com/openai/v1/audio/transcriptions"));
    req.setRawHeader("Authorization", "Bearer " + m_groqKey.toUtf8());

    QNetworkReply *reply = m_network->post(req, multiPart);
    multiPart->setParent(reply);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
        QString texte = obj["text"].toString();
        reply->deleteLater();
        envoyerTexteAuLLM(texte);   // → étape suivante
    });
}
void WasteRecycling::envoyerTexteAuLLM(const QString &texte)
{
    QString prompt =
        "Extrait les informations client depuis ce texte et réponds UNIQUEMENT en JSON valide "
        "avec les clés : nom, telephone, email, adresse, type (municipalite/entreprise/individu), "
        "nb_transactions. Laisse vide si absent.\nTexte : " + texte;

    QJsonObject body;
    body["model"] = "claude-sonnet-4-20250514";  // ou "gemini-1.5-flash"
    body["max_tokens"] = 300;
    body["messages"] = QJsonArray{
        QJsonObject{{"role","user"},{"content", prompt}}
    };

    QNetworkRequest req(QUrl("https://api.anthropic.com/v1/messages"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("x-api-key", m_anthropicKey.toUtf8());
    req.setRawHeader("anthropic-version", "2023-06-01");

    QNetworkReply *reply = m_network->post(req, QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        reply->deleteLater();
        // Extrait le texte JSON de la réponse Claude
        QString jsonStr = doc["content"][0]["text"].toString();
        // Nettoie les backticks si présents
        jsonStr.remove("```json").remove("```").trimmed();
        QJsonObject data = QJsonDocument::fromJson(jsonStr.toUtf8()).object();
        ouvrirDialoguePreRempli(data);
    });
}
void WasteRecycling::ouvrirDialoguePreRempli(const QJsonObject &data)
{
    QDialog dialog(this);
    dialog.setWindowTitle("Ajouter un client — note vocale");
    dialog.setMinimumWidth(400);

    QFormLayout *form = new QFormLayout();
    form->setContentsMargins(20, 20, 20, 10);
    form->setVerticalSpacing(10);

    QLineEdit *fNom   = new QLineEdit(data["nom"].toString());
    QLineEdit *fTel   = new QLineEdit(data["telephone"].toString());
    QLineEdit *fAdr   = new QLineEdit(data["adresse"].toString());
    QLineEdit *fEmail = new QLineEdit(data["email"].toString());
    QLineEdit *fTrans = new QLineEdit(data["nb_transactions"].toString());

    QWidget *typeRow = new QWidget();
    QHBoxLayout *typeLayout = new QHBoxLayout(typeRow);
    typeLayout->setContentsMargins(0, 0, 0, 0);
    typeLayout->setSpacing(6);
    QComboBox *fType = new QComboBox();
    fType->addItems({"municipalite", "entreprise", "individu"});
    QPushButton *micType = new QPushButton("🎤");
    micType->setObjectName("btnMic");
    micType->setFixedWidth(34);
    typeLayout->addWidget(fType);
    typeLayout->addWidget(micType);
    fType->setCurrentText(data["type"].toString());

    auto normaliserType = [](QString t) -> QString {
        t = t.toLower().trimmed();
        t.replace("é","e").replace("è","e").replace("ê","e")
            .replace("à","a").replace("â","a");
        t.remove(QRegularExpression("[^a-z]"));
        if (t.contains("muni"))                       return "municipalite";
        if (t.contains("entre")||t.contains("antre")) return "entreprise";
        if (t.contains("indiv")||t.contains("endiv")) return "individu";
        return "";
    };

    connect(micType, &QPushButton::clicked, this, [=]() {
        micType->setText("⏺"); micType->setEnabled(false);
        if (m_recorder) { m_recorder->stop(); delete m_recorder; m_recorder = nullptr; }
        if (m_session)  { delete m_session;   m_session  = nullptr; }
        m_session  = new QMediaCaptureSession(this);
        m_recorder = new QMediaRecorder(this);
        QAudioInput *audioInput = new QAudioInput(this);
        m_session->setAudioInput(audioInput);
        m_session->setRecorder(m_recorder);
        QMediaFormat format; format.setFileFormat(QMediaFormat::Wave);
        m_recorder->setMediaFormat(format);
        m_recorder->setAudioSampleRate(16000); m_recorder->setAudioChannelCount(1);
        m_recorder->setOutputLocation(QUrl::fromLocalFile(m_wavPath));
        m_recorder->record();
        QTimer::singleShot(3000, this, [=]() {
            if (m_recorder) m_recorder->stop();
            micType->setText("🎤"); micType->setEnabled(true);
            QFileInfo fi(m_wavPath);
            if (!fi.exists() || fi.size() < 1000) { QMessageBox::warning(this, "Micro", "Trop court."); return; }
            QFile *file = new QFile(m_wavPath, this);
            if (!file->open(QIODevice::ReadOnly)) return;
            QHttpMultiPart *mp = new QHttpMultiPart(QHttpMultiPart::FormDataType);
            QHttpPart modelPart; modelPart.setHeader(QNetworkRequest::ContentDispositionHeader, "form-data; name=\"model\""); modelPart.setBody("whisper-large-v3-turbo");
            QHttpPart langPart;  langPart.setHeader(QNetworkRequest::ContentDispositionHeader,  "form-data; name=\"language\""); langPart.setBody("fr");
            QHttpPart audioPart; audioPart.setHeader(QNetworkRequest::ContentDispositionHeader, "form-data; name=\"file\"; filename=\"audio.wav\"");
            audioPart.setHeader(QNetworkRequest::ContentTypeHeader, "audio/wav"); audioPart.setBodyDevice(file); file->setParent(mp);
            mp->append(modelPart); mp->append(langPart); mp->append(audioPart);
            QNetworkRequest req(QUrl("https://api.groq.com/openai/v1/audio/transcriptions"));
            req.setRawHeader("Authorization", "Bearer " + m_groqKey.toUtf8());
            QNetworkReply *reply = m_network->post(req, mp); mp->setParent(reply);
            connect(reply, &QNetworkReply::finished, this, [=]() {
                QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
                reply->deleteLater();
                QString typeNorm = normaliserType(obj["text"].toString().trimmed());
                if (!typeNorm.isEmpty()) fType->setCurrentText(typeNorm);
                else QMessageBox::warning(this, "Type non reconnu", "Dites : municipalité, entreprise ou individu");
            });
        });
    });

    form->addRow("Nom :",            fNom);
    form->addRow("Type :",           typeRow);
    form->addRow("Téléphone :",      fTel);
    form->addRow("Adresse :",        fAdr);
    form->addRow("Email :",          fEmail);
    form->addRow("Nb transactions :", fTrans);

    QPushButton *btnOk  = new QPushButton("Confirmer et ajouter");
    QPushButton *btnAnn = new QPushButton("Annuler");
    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch();
    btnRow->addWidget(btnAnn);
    btnRow->addWidget(btnOk);

    QVBoxLayout *mainL = new QVBoxLayout(&dialog);
    mainL->addLayout(form);
    mainL->addLayout(btnRow);

    connect(btnOk, &QPushButton::clicked, &dialog, [&]() {
        if (fNom->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dialog, "Nom", "Le nom est obligatoire.");
            return;
        }
        dialog.accept();
    });
    connect(btnAnn, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) return;

    static const QStringList types = {"municipalite", "entreprise", "individu"};
    client nouveau(0, fNom->text(), types[fType->currentIndex()],
                   fAdr->text(), 0, fEmail->text(), fTel->text(), fTrans->text().toInt());
    if (nouveau.ajouterClient()) {
        QMessageBox::information(this, "Succès", "Client ajouté depuis note vocale.");
        chargerClients();
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de l'ajout : " +
                                                  QSqlDatabase::database().lastError().text());
    }
}


// Boutons retour vers la page CRUD client
void WasteRecycling::on_clretour1_clicked() { ui->stackedWidgetclient->setCurrentIndex(PAGE_CRUDCLIENT); }
void WasteRecycling::on_clretour2_clicked() { ui->stackedWidgetclient->setCurrentIndex(PAGE_CRUDCLIENT); }
void WasteRecycling::on_clretour3_clicked() { ui->stackedWidgetclient->setCurrentIndex(PAGE_CRUDCLIENT); }

// Navigation vers les pages métier client
void WasteRecycling::on_clbtnsysfide_clicked() { chargerFidelite(); ui->stackedWidgetclient->setCurrentIndex(PAGE_SYSFIDECLIENT); }
void WasteRecycling::on_clbtnsysprio_clicked() { ui->stackedWidgetclient->setCurrentIndex(PAGE_SYSPRIOCLIENT); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Fonction couleur pour la progress bar GPC
QColor WasteRecycling::getFillColor(int niveau) {
    if (niveau < 50) return QColor(76, 175, 80);
    if (niveau < 80) return QColor(255, 152, 0);
    return QColor(244, 67, 54);
}