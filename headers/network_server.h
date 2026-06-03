#pragma once
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include <QList>
#include "gameworker.h"
#include "subject.h"
#include "demo.h"

class NetworkServer : public QObject {
    Q_OBJECT
public:
    explicit NetworkServer(Map* map, QObject* parent = nullptr);
    ~NetworkServer() override;

    bool startListening(quint16 port = 8888);
    
    void startGame();
    

    MapWidget* getMapWidget() const { return mapWidget_; }

signals:
    void logMessage(const QString& msg);
    void gameStateChanged();
    void gameFinished(int winner);
    void gameReady();
    void gameStarted();

private slots:
    void onNewConnection();
    void onReadyRead(QTcpSocket* socket);
    void onDisconnected(QTcpSocket* socket);
    void onGameStateChanged();

private:
    void broadcastState(const QString& errorMsg = "");
    void processMove(QTcpSocket* sender, const std::string& regionName);

    QTcpServer* server_;
    QList<QTcpSocket*> clients_;
    
    QThread* workerThread_;
    GameWorker* gameWorker_;
    Map* map_;
    MapWidget* mapWidget_;
    bool gameStarted_ = false;
};