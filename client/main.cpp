// #include "client.h"
// #include <QCoreApplication>

// int main(int argc, char *argv[])
// {
//     QCoreApplication a(argc, argv);
//     DeviceEmulator emulator;
//     return a.exec();
// }
#include "client.h"
#include <QCoreApplication>
#include <QThread>
#include <QVector>
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    int clientCount = 10; // Количество клиентов для тестирования
    if (argc > 1) {
        clientCount = QString(argv[1]).toInt();
    }

    std::cout << "Starting " << clientCount << " client emulators..." << std::endl;

    QVector<QThread*> threads;
    QVector<DeviceEmulator*> emulators;

    for (int i = 0; i < clientCount; ++i) {
        QThread* thread = new QThread();
        DeviceEmulator* emulator = new DeviceEmulator();

        emulator->moveToThread(thread);

        QObject::connect(thread, &QThread::started, emulator, &DeviceEmulator::connectToServer);
        QObject::connect(thread, &QThread::finished, emulator, &DeviceEmulator::deleteLater);
        QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);

        threads.append(thread);
        emulators.append(emulator);

        thread->start();

        // Небольшая задержка между запуском клиентов
        QThread::msleep(100);
    }

    std::cout << "All clients started. Press Ctrl+C to stop." << std::endl;

    return a.exec();
}
