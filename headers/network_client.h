#pragma once
#include <QObject>
#include <QTcpSocket>
#include <vector>
#include <string>

class NetworkClient : public QObject {
    Q_OBJECT
public:
    explicit NetworkClient(QObject* parent = nullptr);
    ~NetworkClient() override;

    void connectToServer(const QString& host, quint16 port = 8888);
    void sendMove(const std::string& regionName);
    bool isConnected() const;

signals:
    // Новый сигнал: сервер присвоил номер игрока
    void playerNumberReceived(int playerNumber);
    
    // Сигналы, идентичные GameWorker, для прозрачной работы с PlayerWindow
    void currentRegionChanged(const std::string& name);
    void finalRegionChanged(const std::string& name);
    void mistakesUpdated(int count);
    void turnChanged(int turn);
    void visitedListUpdated(const std::vector<std::string>& names);
    void gameFinished(int winner);
    void gameReady();
    void showError(const QString& msg);

private slots:
    void onConnected();
    void onReadyRead();
    void onDisconnected();

private:
    QTcpSocket* socket_;
};