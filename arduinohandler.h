#ifndef ARDUINOHANDLER_H
#define ARDUINOHANDLER_H
#include <QFile>
#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDir>
class ArduinoHandler : public QObject
{
    Q_OBJECT
public:
    explicit ArduinoHandler(QObject *parent = nullptr);
    ~ArduinoHandler();
    bool connecter();
    void deconnecter();
    bool estConnecte() const;
signals:
    void arduinoPret();
    void enregistrementDebute();
    void enregistrementTermine(int duree);
    void erreurConnexion(const QString &msg);
    void fichierWavRecu(const QString &cheminWav);
private slots:
    void onDonneesRecues();   // ← était manquant
private:
    QSerialPort *m_serial;
    QString      m_buffer;
    QString      m_wavPath;
    QByteArray   m_wavBuffer;
    bool         m_recvWav    = false;
    int          m_wavSize    = 0;
    int          m_dureeEnCours = 0;
    void traiterMessage(const QString &message);
    QString detecterPort();
};
#endif