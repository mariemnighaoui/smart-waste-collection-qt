#include "arduinohandler.h"
#include <QDebug>
ArduinoHandler::ArduinoHandler(QObject *parent)
    : QObject(parent)
    , m_serial(new QSerialPort(this))
{
    connect(m_serial, &QSerialPort::readyRead,
            this,     &ArduinoHandler::onDonneesRecues);
}
ArduinoHandler::~ArduinoHandler()
{
    deconnecter();
}
QString ArduinoHandler::detecterPort()
{
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : ports) {
        if (info.description().contains("Arduino", Qt::CaseInsensitive) ||
            info.manufacturer().contains("Arduino", Qt::CaseInsensitive) ||
            info.description().contains("USB Serial", Qt::CaseInsensitive)) {
            return info.portName();
        }
    }
#ifdef Q_OS_WIN
    return "COM3";
#else
    return "/dev/ttyUSB0";
#endif
}
bool ArduinoHandler::connecter()
{
    if (m_serial->isOpen())
        m_serial->close();
    QString port = detecterPort();
    m_serial->setPortName(port);
    m_serial->setBaudRate(QSerialPort::Baud9600);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);
    if (!m_serial->open(QIODevice::ReadOnly)) {
        emit erreurConnexion(
            QString("Impossible de se connecter sur %1 : %2")
                .arg(port).arg(m_serial->errorString()));
        return false;
    }
    qDebug() << "[Arduino] Connecté sur" << port;
    return true;
}
void ArduinoHandler::deconnecter()
{
    if (m_serial->isOpen())
        m_serial->close();
}
bool ArduinoHandler::estConnecte() const
{
    return m_serial->isOpen();
}
void ArduinoHandler::onDonneesRecues()
{
    m_buffer += m_serial->readAll();
    while (m_buffer.contains('\n')) {
        int idx       = m_buffer.indexOf('\n');
        QString ligne = m_buffer.left(idx).trimmed();
        m_buffer      = m_buffer.mid(idx + 1);
        if (!ligne.isEmpty()) {
            qDebug() << "[Arduino] Reçu :" << ligne;
            traiterMessage(ligne);
        }
    }
}
void ArduinoHandler::traiterMessage(const QString &message)
{
    if (message == "STATUS:READY") {
        emit arduinoPret();
    } else if (message == "STATUS:REC_START") {
        emit enregistrementDebute();
    } else if (message.startsWith("REC:DONE:")) {
        QStringList parts = message.split(':');
        bool ok;
        int duree = parts.size() >= 3 ? parts[2].toInt(&ok) : 0;
        if (ok) emit enregistrementTermine(duree);
        else qWarning() << "[Arduino] Format invalide :" << message;
    }
}