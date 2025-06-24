#include "client.h"
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QRandomGenerator>
#include <QDateTime>
#include <iostream>

DeviceEmulator::DeviceEmulator(QObject *parent) : QObject(parent),
    socket(new QTcpSocket(this)),
    metricsTimer(nullptr),
    statusTimer(nullptr),
    logTimer(nullptr)
{
    connect(socket, &QTcpSocket::connected, this, &DeviceEmulator::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &DeviceEmulator::onDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &DeviceEmulator::onReadyRead);
    connectToServer();
}

DeviceEmulator::~DeviceEmulator()
{
    delete metricsTimer;
    delete statusTimer;
    delete logTimer;
}

void DeviceEmulator::connectToServer()
{
    std::cout << "Connecting to server..." << std::endl;
    socket->connectToHost("localhost", 12345);

    if (!socket->waitForConnected(5000)) {
        std::cout << "Connection failed. Retrying in 5 seconds..." << std::endl;
        QTimer::singleShot(5000, this, &DeviceEmulator::connectToServer);
    }
}

void DeviceEmulator::onConnected()
{
    std::cout << "Connected to server. Waiting for acknowledgment..." << std::endl;
}

void DeviceEmulator::onDisconnected()
{
    std::cout << "Disconnected from server. Reconnecting..." << std::endl;
    QTimer::singleShot(5000, this, &DeviceEmulator::connectToServer);
}

// Слот: получение данных от сервера
void DeviceEmulator::onReadyRead()
{
    QByteArray data = socket->readAll();
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    // Проверка на ошибки парсинга
    if (error.error != QJsonParseError::NoError) {
        std::cerr << "Invalid JSON received: " << error.errorString().toStdString() << std::endl;
        return;
    }

    QJsonObject json = doc.object();
    QString type = json["type"].toString();

    if (type == "connection_ack") {
        clientId = json["client_id"].toString();
        std::cout << "Connection acknowledged. Client ID: " << clientId.toStdString() << std::endl;
        startSendingData();
    }
    else if (type == "data_ack") {
        std::cout << "Data acknowledged: " << json["data_type"].toString().toStdString() << std::endl;
    }
}

// отправка данных на сервер
void DeviceEmulator::startSendingData()
{
    metricsTimer = new QTimer(this);
    connect(metricsTimer, &QTimer::timeout, this, &DeviceEmulator::sendNetworkMetrics);
    metricsTimer->start(100 + QRandomGenerator::global()->bounded(900));

    statusTimer = new QTimer(this);
    connect(statusTimer, &QTimer::timeout, this, &DeviceEmulator::sendDeviceStatus);
    statusTimer->start(1000 + QRandomGenerator::global()->bounded(4000));

    logTimer = new QTimer(this);
    connect(logTimer, &QTimer::timeout, this, &DeviceEmulator::sendLog);
    logTimer->start(2000 + QRandomGenerator::global()->bounded(8000));
}

// отправка метрик сети
void DeviceEmulator::sendNetworkMetrics()
{
    QJsonObject metrics;
    metrics["type"] = "NetworkMetrics";
    metrics["bandwidth"] = QRandomGenerator::global()->bounded(1000) / 10.0;
    metrics["latency"] = QRandomGenerator::global()->bounded(100) / 10.0;
    metrics["packet_loss"] = QRandomGenerator::global()->bounded(50) / 1000.0;
    sendJson(metrics);
}

// статус устройства
void DeviceEmulator::sendDeviceStatus()
{
    QJsonObject status;
    status["type"] = "DeviceStatus";
    status["uptime"] = QRandomGenerator::global()->bounded(100000);
    status["cpu_usage"] = QRandomGenerator::global()->bounded(100);
    status["memory_usage"] = QRandomGenerator::global()->bounded(100);
    sendJson(status);
}

// логи
void DeviceEmulator::sendLog()
{
    static const QStringList severities = {"INFO", "WARNING", "ERROR"};
    static const QStringList messages = {
        "System boot completed",
        "Network interface eth0 connected",
        "High CPU usage detected",
        "Memory threshold exceeded",
        "Packet loss increased",
        "Device temperature normal",
        "Disk space low",
        "Scheduled maintenance required"
    };

    QJsonObject log;
    log["type"] = "Log";
    log["severity"] = severities[QRandomGenerator::global()->bounded(severities.size())];
    log["message"] = messages[QRandomGenerator::global()->bounded(messages.size())];
    sendJson(log);
}

void DeviceEmulator::sendJson(const QJsonObject &json)
{
    if (socket->state() != QAbstractSocket::ConnectedState) {
        std::cerr << "Cannot send data - not connected to server" << std::endl;
        return;
    }

    // преобразуем JSON в QByteArray и отправляем
    QJsonDocument doc(json);
    socket->write(doc.toJson());
    std::cout << "Sent: " << json["type"].toString().toStdString() << std::endl;
}
