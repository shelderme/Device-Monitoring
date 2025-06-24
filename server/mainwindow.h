#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_startServerButton_clicked();
    void on_stopServerButton_clicked();
    void newConnection();
    void clientDisconnected();
    void readClientData();
    void updateClientList();

private:
    Ui::MainWindow *ui;
    QTcpServer *server;
    QMap<QTcpSocket*, QPair<QString, bool>> clients; // socket -> <id, status>
    int clientCounter = 0;

    void setupTables();
    void logMessage(const QString &message);
    void processClientData(QTcpSocket *clientSocket, const QJsonObject &data);
    void sendAck(QTcpSocket *socket, const QString &type);
    void addDataToTable(const QString &clientId, const QString &dataType, const QString &content);
};
#endif // MAINWINDOW_H
