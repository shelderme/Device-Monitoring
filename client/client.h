#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QJsonObject>

class DeviceEmulator : public QObject
{
    Q_OBJECT
public:
    explicit DeviceEmulator(QObject *parent = nullptr);
    ~DeviceEmulator();
    void connectToServer();

private slots:

    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void sendNetworkMetrics();
    void sendDeviceStatus();
    void sendLog();

private:
    void startSendingData();
    void sendJson(const QJsonObject &json);

    QTcpSocket *socket;
    QString clientId;
    QTimer *metricsTimer;
    QTimer *statusTimer;
    QTimer *logTimer;
signals:
    void finished();
};

#endif // CLIENT_H
