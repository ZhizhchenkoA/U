#include "network_client.h"
#include "network_protocol.h"
#include <QDebug>

NetworkClient::NetworkClient(QObject* parent) : QObject(parent), socket_(new QTcpSocket(this)) {
    connect(socket_, &QTcpSocket::connected, this, &NetworkClient::onConnected);
    connect(socket_, &QTcpSocket::readyRead, this, &NetworkClient::onReadyRead);
    connect(socket_, &QTcpSocket::disconnected, this, &NetworkClient::onDisconnected);
    connect(socket_, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred), 
            [this](QAbstractSocket::SocketError) {
                emit showError("Ошибка сети: " + socket_->errorString());
            });
}

NetworkClient::~NetworkClient() {
    if (socket_->state() == QAbstractSocket::ConnectedState) {
        socket_->disconnectFromHost();
    }
}

void NetworkClient::connectToServer(const QString& host, quint16 port) {
    socket_->connectToHost(host, port);
}

void NetworkClient::sendMove(const std::string& regionName) {
    if (isConnected()) {
        socket_->write(NetworkProtocol::makeMoveMsg(regionName).toUtf8());
    }
}

bool NetworkClient::isConnected() const {
    return socket_->state() == QAbstractSocket::ConnectedState;
}

void NetworkClient::onConnected() {
    qDebug() << "Connected to server";
}

void NetworkClient::onReadyRead() {
    QByteArray data = socket_->readAll();
    QString msg = QString::fromUtf8(data).trimmed();
    if (msg.isEmpty()) return;

    try {
        json j = json::parse(msg.toStdString());
        std::string type = j["type"].get<std::string>();
        
        // === ОБРАБОТКА ПРИВЕТСТВИЯ ОТ СЕРВЕРА ===
        if (type == "welcome") {
            int playerNumber = j["playerNumber"].get<int>();
            qDebug() << "[Client] Received player number:" << playerNumber;
            emit playerNumberReceived(playerNumber);
            return;
        }
        
        if (type == "state") {
            std::vector<std::string> visited;
            for (const auto& v : j["visited"]) {
                visited.push_back(v.get<std::string>());
            }
            
            emit currentRegionChanged(j["current"].get<std::string>());
            emit finalRegionChanged(j["final"].get<std::string>());
            emit mistakesUpdated(j["mistakes"].get<int>());
            emit turnChanged(j["turn"].get<int>());
            emit visitedListUpdated(visited);
            
            if (!j["error"].is_null() && !j["error"].get<std::string>().empty()) {
                emit showError(QString::fromStdString(j["error"].get<std::string>()));
            }

            if (j["finished"].get<bool>()) {
                emit gameFinished(j["winner"].get<int>());
            } else {
                emit gameReady();
            }
        }
    } catch (const std::exception& e) {
        qWarning() << "[Client] JSON parse error:" << e.what();
    }
}

void NetworkClient::onDisconnected() {
    emit showError("Соединение с сервером разорвано.");
}