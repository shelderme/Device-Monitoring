#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDateTime>
#include <QHeaderView>
#include <QJsonDocument>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , server(new QTcpServer(this))
{
    ui->setupUi(this);
    setupTables();

    connect(server, &QTcpServer::newConnection, this, &MainWindow::newConnection);

    // Timer для периодического обновления списка клиентов
    QTimer *updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &MainWindow::updateClientList);
    updateTimer->start(1000);

    logMessage("Server application started");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupTables()
{
    // таблица клиентов
    ui->clientsTable->setColumnCount(3);
    ui->clientsTable->setHorizontalHeaderLabels({"Client ID", "IP Address", "Status"});
    ui->clientsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // таблица данных
    ui->dataTable->setColumnCount(4);
    ui->dataTable->setHorizontalHeaderLabels({"Client ID", "Data Type", "Content", "Time"});
    ui->dataTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

// кнопка start
void MainWindow::on_startServerButton_clicked()
{
    ui->logTextEdit->append("Сервер запущен");
    ui->startServerButton->setEnabled(false);
    ui->stopServerButton->setEnabled(true);
    if (!server->isListening()) {
        if (server->listen(QHostAddress::Any, 12345)) {
            logMessage("Server started on port 12345");
        } else {
            logMessage("Failed to start server: " + server->errorString());
        }
    }
}

// кнопка stop
void MainWindow::on_stopServerButton_clicked()
{

    if (server->isListening()) {
        server->close();

        // отключение клиентов
        for (auto it = clients.begin(); it != clients.end(); ++it) {
            QTcpSocket *socket = it.key();
            socket->abort();
            socket->deleteLater();
        }
        clients.clear();

        ui->startServerButton->setEnabled(true);
        ui->stopServerButton->setEnabled(false);
        logMessage("Server stopped");
    }
}

void MainWindow::newConnection()
{
    QTcpSocket *clientSocket = server->nextPendingConnection();
    QString clientId = "Client_" + QString::number(++clientCounter);
    QString clientIp = clientSocket->peerAddress().toString();

    clients.insert(clientSocket, {clientId, true});

    connect(clientSocket, &QTcpSocket::readyRead, this, &MainWindow::readClientData);
    connect(clientSocket, &QTcpSocket::disconnected, this, &MainWindow::clientDisconnected);

    // отправляем подтверждение подключения
    QJsonObject ack;
    ack["type"] = "connection_ack";
    ack["client_id"] = clientId;
    QJsonDocument doc(ack);
    clientSocket->write(doc.toJson());

    logMessage("New client connected: " + clientId + " (" + clientIp + ")");
    updateClientList();
}

void MainWindow::clientDisconnected()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (clientSocket && clients.contains(clientSocket)) {
        QString clientId = clients[clientSocket].first;
        logMessage("Client disconnected: " + clientId);
        clients.remove(clientSocket);
        clientSocket->deleteLater();
        updateClientList();
    }
}

// чтение данных от клиента
void MainWindow::readClientData()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket || !clients.contains(clientSocket)) return;

    QByteArray data = clientSocket->readAll();
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        logMessage("Invalid JSON from client: " + error.errorString());
        return;
    }

    if (!doc.isObject()) {
        logMessage("Received data is not a JSON object");
        return;
    }

    QJsonObject json = doc.object();
    processClientData(clientSocket, json);
}

// обработка данных
void MainWindow::processClientData(QTcpSocket *clientSocket, const QJsonObject &data)
{
    QString clientId = clients[clientSocket].first;
    QString type = data["type"].toString();
    QString content = QJsonDocument(data).toJson(QJsonDocument::Compact);
    QString time = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");

    addDataToTable(clientId, type, content);
    sendAck(clientSocket, type);

    // Обработка разных типов данных
    if (type == "NetworkMetrics") {
        double bandwidth = data["bandwidth"].toDouble();
        double latency = data["latency"].toDouble();

    }
    else if (type == "DeviceStatus") {
        int cpuUsage = data["cpu_usage"].toInt();
        int memoryUsage = data["memory_usage"].toInt();

    }
    else if (type == "Log") {
        QString message = data["message"].toString();
        QString severity = data["severity"].toString();

    }
}

void MainWindow::sendAck(QTcpSocket *socket, const QString &type)
{
    QJsonObject ack;
    ack["type"] = "data_ack";
    ack["data_type"] = type;
    ack["status"] = "received";
    QJsonDocument doc(ack);
    socket->write(doc.toJson());
}

// добавление данных в таблицу
void MainWindow::addDataToTable(const QString &clientId, const QString &dataType, const QString &content)
{
    int row = ui->dataTable->rowCount();
    ui->dataTable->insertRow(row);

    ui->dataTable->setItem(row, 0, new QTableWidgetItem(clientId));
    ui->dataTable->setItem(row, 1, new QTableWidgetItem(dataType));
    ui->dataTable->setItem(row, 2, new QTableWidgetItem(content));
    ui->dataTable->setItem(row, 3, new QTableWidgetItem(QDateTime::currentDateTime().toString("hh:mm:ss")));
}

// обновление списка клиентов
void MainWindow::updateClientList()
{
    ui->clientsTable->setRowCount(0);

    for (auto it = clients.begin(); it != clients.end(); ++it) {
        QTcpSocket *socket = it.key();
        QString clientId = it.value().first;
        bool isConnected = socket->state() == QAbstractSocket::ConnectedState;

        int row = ui->clientsTable->rowCount();
        ui->clientsTable->insertRow(row);

        ui->clientsTable->setItem(row, 0, new QTableWidgetItem(clientId));
        ui->clientsTable->setItem(row, 1, new QTableWidgetItem(socket->peerAddress().toString()));
        ui->clientsTable->setItem(row, 2, new QTableWidgetItem(isConnected ? "Connected" : "Disconnected"));
    }
}

void MainWindow::logMessage(const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    ui->logTextEdit->append("[" + timestamp + "] " + message);
}
