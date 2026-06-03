#include "network_server.h"
#include "network_protocol.h"
#include <QDebug>
#include <QMetaObject>
#include <QTimer>

NetworkServer::NetworkServer(Map* map, QObject* parent) 
    : QObject(parent), server_(new QTcpServer(this)), map_(map), gameStarted_(false)
{
    connect(server_, &QTcpServer::newConnection, this, &NetworkServer::onNewConnection);
    
    workerThread_ = new QThread(this);
    gameWorker_ = new GameWorker();
    gameWorker_->moveToThread(workerThread_);
    workerThread_->start();
    
    mapWidget_ = new MapWidget(map_);
    
    connect(gameWorker_, &GameWorker::gameStateChanged, this, &NetworkServer::onGameStateChanged);
    connect(gameWorker_, &GameWorker::gameFinished, this, &NetworkServer::gameFinished);
    connect(gameWorker_, &GameWorker::gameReady, this, &NetworkServer::gameReady);
}

NetworkServer::~NetworkServer() {
    for (QTcpSocket* client : clients_) {
        client->disconnectFromHost();
        client->deleteLater();
    }
    server_->close();
    if (workerThread_->isRunning()) {
        QMetaObject::invokeMethod(gameWorker_, &GameWorker::onQuit, Qt::QueuedConnection);
        workerThread_->quit();
        workerThread_->wait(1000);
    }
}

bool NetworkServer::startListening(quint16 port) {
    if (!server_->listen(QHostAddress::Any, port)) {
        emit logMessage("Ошибка: Не удалось запустить сервер на порту " + QString::number(port));
        return false;
    }
    emit logMessage("Сервер запущен на порту " + QString::number(port) + ". Ожидание игроков...");
    // Автоматический старт игры убран отсюда
    return true;
}

void NetworkServer::startGame() {
    if (gameStarted_) {
        emit logMessage("Игра уже запущена.");
        return;
    }
    if (clients_.isEmpty()) {
        emit logMessage("Ошибка: Нет подключенных игроков для старта игры.");
        return;
    }
    
    gameStarted_ = true;
    emit logMessage("=== ИГРА НАЧАЛАСЬ! ===");
    emit gameStarted();
    
    QMetaObject::invokeMethod(gameWorker_, [=]() {
        auto& subjects = map_->get_subjects();
        gameWorker_->init(static_cast<int>(subjects.size()), &subjects);
    }, Qt::QueuedConnection);
}

// В методе onNewConnection() замените блок подключения на этот:
void NetworkServer::onNewConnection() {
    if (clients_.size() >= 2) {
        QTcpSocket* extra = server_->nextPendingConnection();
        extra->write("Server full\n");
        extra->disconnectFromHost();
        emit logMessage("Отклонено подключение: сервер заполнен (макс. 2 игрока)");
        return;
    }
    
    QTcpSocket* client = server_->nextPendingConnection();
    clients_.append(client);
    
    int playerNumber = clients_.size(); // 1 для первого, 2 для второго
    emit logMessage(QString("Игрок %1 подключился.").arg(playerNumber));
    
    client->write(NetworkProtocol::makeWelcomeMsg(playerNumber).toUtf8());
    
    connect(client, &QTcpSocket::readyRead, this, [this, client]() { onReadyRead(client); });
    connect(client, &QTcpSocket::disconnected, this, [this, client]() { onDisconnected(client); });
    
    if (clients_.size() == 2) {
        emit logMessage("Оба игрока подключились. Хост может нажать 'Начать игру'.");
    }
}

void NetworkServer::onReadyRead(QTcpSocket* socket) {
    QByteArray data = socket->readAll();
    QString msg = QString::fromUtf8(data).trimmed();
    if (msg.isEmpty()) return;

    try {
        json j = json::parse(msg.toStdString());
        if (j["type"] == "move") {
            std::string region = j["region"].get<std::string>();
            processMove(socket, region);
        }
    } catch (const std::exception& e) {
        emit logMessage("Ошибка парсинга JSON: " + QString(e.what()));
    }
}

void NetworkServer::onDisconnected(QTcpSocket* socket) {
    emit logMessage("Игрок отключился. Игра остановлена.");
    clients_.removeOne(socket);
    socket->deleteLater();
}

void NetworkServer::processMove(QTcpSocket* sender, const std::string& regionName) {
    int senderIndex = clients_.indexOf(sender); // 0 или 1
    emit logMessage(QString("Игрок %1 пытается сделать ход: %2").arg(senderIndex + 1).arg(QString::fromStdString(regionName)));
    
    QMetaObject::invokeMethod(gameWorker_, [=]() {
        int result = gameWorker_->makeNetworkMove(regionName, senderIndex);
        
        QString error;
        if (result == -1) error = "Неверный ход или не ваша очередь!";
        else if (result == -2) error = "Игрок совершил 3 ошибки и проиграл!";
        
        QTimer::singleShot(50, this, [this, error]() {
            broadcastState(error);
        });
    }, Qt::QueuedConnection);
}

void NetworkServer::onGameStateChanged() {
    broadcastState();
    emit gameStateChanged();
}

void NetworkServer::broadcastState(const QString& errorMsg) {
    if (clients_.isEmpty() || !gameWorker_ || !gameWorker_->getGame()) return;
    
    auto* game = gameWorker_->getGame();
    QString jsonMsg = NetworkProtocol::makeStateMsg(
        game->getTurn(),
        game->getCurrentRegionName(),
        game->getFinalRegionName(),
        game->getMistakesCount(),
        game->getVisitedRegionNames(),
        game->isGameFinished(),
        game->getWinner(),
        errorMsg.toStdString()
    );
    
    for (QTcpSocket* client : clients_) {
        if (client->state() == QAbstractSocket::ConnectedState) {
            client->write(jsonMsg.toUtf8());
        }
    }
}