#include <QApplication>
#include <QMainWindow>
#include <QMessageBox>
#include <QInputDialog>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QNetworkInterface>
#include "start_menu.h"
#include "presenter.h"
#include "network_server.h"
#include "network_client.h"
#include "player_window.h"

// === Функция получения IP-адресов машины ===
QStringList getLocalIPAddresses() {
    QStringList addresses;
    QList<QHostAddress> addressesList = QNetworkInterface::allAddresses();
    for (const QHostAddress& address : addressesList) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && 
            address != QHostAddress::LocalHost) {
            addresses << address.toString();
        }
    }
    return addresses;
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // === Окно выбора режима запуска ===
    QMessageBox msgBox;
    msgBox.setWindowTitle("Выбор режима");
    msgBox.setText("Запустить приложение как:");
    QAbstractButton* btnServer = msgBox.addButton("Сервер (Хост)", QMessageBox::ActionRole);
    QAbstractButton* btnClient1 = msgBox.addButton("Клиент (Игрок)", QMessageBox::ActionRole);
    QAbstractButton* btnSingle = msgBox.addButton("Одиночная игра (против ПК)", QMessageBox::ActionRole);
    msgBox.exec();

    // ============================================================
    // === ОДИНОЧНАЯ ИГРА (против компьютера) ===
    // ============================================================
    if (msgBox.clickedButton() == btnSingle) {
        StartMenu menu;
        menu.show();
        menu.setOnStartGameCallback([&]() {
            Presenter* presenter = new Presenter();
            
            QMainWindow* mapWindow = new QMainWindow();
            mapWindow->setAttribute(Qt::WA_DeleteOnClose);
            mapWindow->setCentralWidget(presenter->getMapWidget());
            mapWindow->setWindowTitle("Карта России");
            mapWindow->resize(800, 600);
            
            PlayerWindow* playerWindow = presenter->getPlayerWindow();
            playerWindow->setAttribute(Qt::WA_DeleteOnClose);
            playerWindow->setWindowTitle("Игра против компьютера");
            playerWindow->resize(1000, 800);
            
            QObject::connect(mapWindow, &QMainWindow::destroyed, &app, &QApplication::quit);
            QObject::connect(playerWindow, &QMainWindow::destroyed, &app, &QApplication::quit);
            
            mapWindow->show();
            playerWindow->show();
            presenter->startGame();
        });
        return app.exec();
    }

    // ============================================================
    // === СЕТЕВОЙ РЕЖИМ ===
    // ============================================================
    Map* sharedMap = new Map();
    sharedMap->get_from_JSON("data/new_russia (1).geojson", "data/russia_neighbours.json");

    // ------------------------------------------------------------
    // === СЕРВЕР (ХОСТ) ===
    // ------------------------------------------------------------
    if (msgBox.clickedButton() == btnServer) {
        NetworkServer* server = new NetworkServer(sharedMap);
        quint16 port = 8888;
        
        if (!server->startListening(port)) {
            QMessageBox::critical(nullptr, "Ошибка", "Не удалось запустить сервер");
            return 1;
        }
        
        // Получаем IP-адреса машины
        QStringList ipAddresses = getLocalIPAddresses();
        
        // Формируем инструкцию для подключения
        QString connectionInfo = "=== ИНФОРМАЦИЯ ДЛЯ ПОДКЛЮЧЕНИЯ ===\n";
        connectionInfo += "Порт: " + QString::number(port) + "\n\n";
        connectionInfo += "Для подключения с ЭТОГО КОМПЬЮТЕРА:\n";
        connectionInfo += "  IP: 127.0.0.1\n\n";
        connectionInfo += "Для подключения из ЛОКАЛЬНОЙ СЕТИ:\n";
        if (ipAddresses.isEmpty()) {
            connectionInfo += "  (Не удалось определить IP-адреса)\n";
        } else {
            for (const QString& ip : ipAddresses) {
                connectionInfo += "  IP: " + ip + "\n";
            }
        }
        connectionInfo += "======================================";
        
        // === Окно с картой (наблюдение за игрой) ===
        QMainWindow* serverWindow = new QMainWindow();
        serverWindow->setAttribute(Qt::WA_DeleteOnClose);
        serverWindow->setCentralWidget(server->getMapWidget());
        
        QString windowTitle = "СЕРВЕР (Порт: " + QString::number(port);
        if (!ipAddresses.isEmpty()) {
            windowTitle += " | IP: " + ipAddresses.first();
        }
        windowTitle += ")";
        serverWindow->setWindowTitle(windowTitle);
        serverWindow->resize(800, 600);
        
        // === Окно лога с кнопкой управления ===
        QTextEdit* logWidget = new QTextEdit();
        logWidget->setReadOnly(true);
        logWidget->setFont(QFont("Consolas", 10));
        
        QPushButton* btnStartGame = new QPushButton("▶ Начать игру");
        btnStartGame->setStyleSheet(
            "QPushButton { padding: 12px; font-size: 14px; font-weight: bold; "
            "background-color: #4CAF50; color: white; border-radius: 4px; }"
            "QPushButton:hover { background-color: #45a049; }"
            "QPushButton:pressed { background-color: #3d8b40; }"
        );
        
        QWidget* controlWidget = new QWidget();
        QVBoxLayout* controlLayout = new QVBoxLayout(controlWidget);
        controlLayout->addWidget(btnStartGame);
        controlLayout->addWidget(logWidget);
        
        QMainWindow* logWindow = new QMainWindow();
        logWindow->setCentralWidget(controlWidget);
        logWindow->setWindowTitle("Управление сервером и лог");
        logWindow->resize(500, 400);
        
        // Выводим инструкцию в лог
        logWidget->append(connectionInfo);
        logWidget->append("");
        
        // Подключаем сигналы сервера к логу
        QObject::connect(server, &NetworkServer::logMessage, logWidget, 
                         [logWidget](const QString& msg) {
            logWidget->append(msg);
        });
        
        // Обновление карты сервера при изменении состояния игры
        QObject::connect(server, &NetworkServer::gameStateChanged, 
                         server->getMapWidget(), &MapWidget::requestRebuildCache);
        
        // Кнопка "Начать игру" запускает игру на сервере
        QObject::connect(btnStartGame, &QPushButton::clicked, 
                         server, &NetworkServer::startGame);
        
        // Выводим инструкцию в консоль
        qDebug() << "\n" << connectionInfo.toUtf8().constData() << "\n";
        
        serverWindow->show();
        logWindow->show();
        
        // Показываем всплывающее окно с инструкцией
        QMessageBox::information(nullptr, "Сервер запущен", 
            connectionInfo + "\n\nКогда игроки подключатся, нажмите 'Начать игру'.");
        
        return app.exec();
    } 
    // ------------------------------------------------------------
    // === КЛИЕНТ (ИГРОК) ===
    // ------------------------------------------------------------
    else {
        bool ok;
        QString ip = QInputDialog::getText(nullptr, "Подключение к серверу", 
                                           "Введите IP-адрес сервера:\n\n"
                                           "Для подключения на этом же компьютере: 127.0.0.1\n"
                                           "Для подключения из сети: IP из окна сервера",
                                           QLineEdit::Normal, "127.0.0.1", &ok);
        if (!ok || ip.isEmpty()) return 0;

        NetworkClient* client = new NetworkClient();
        client->connectToServer(ip, 8888);

        // === Создаём только окно игры (карта только у сервера) ===
        PlayerWindow* playerWindow = new PlayerWindow();
        playerWindow->setAttribute(Qt::WA_DeleteOnClose);
        playerWindow->setMap(sharedMap); // Для кэша имён регионов (автодополнение)
        playerWindow->setWindowTitle("Ожидание подключения...");
        playerWindow->resize(1000, 800);
        
        // Устанавливаем сетевой режим (номер игрока придёт от сервера)
        playerWindow->setNetworkMode(true, 0);

        // === Связываем сигналы клиента с UI ===
        QObject::connect(client, &NetworkClient::currentRegionChanged, 
                         playerWindow, &PlayerWindow::updateCurrentRegion);
        QObject::connect(client, &NetworkClient::finalRegionChanged, 
                         playerWindow, &PlayerWindow::updateFinalRegion);
        QObject::connect(client, &NetworkClient::mistakesUpdated, 
                         playerWindow, &PlayerWindow::updateMistakes);
        QObject::connect(client, &NetworkClient::turnChanged, 
                         playerWindow, &PlayerWindow::updateTurn);
        QObject::connect(client, &NetworkClient::visitedListUpdated, 
                         playerWindow, &PlayerWindow::updateVisitedList);
        QObject::connect(client, &NetworkClient::gameFinished, 
                         playerWindow, &PlayerWindow::onGameFinished);
        QObject::connect(client, &NetworkClient::gameReady, 
                         playerWindow, &PlayerWindow::initGame);
        
        // === Обработка получения номера игрока от сервера ===
        QObject::connect(client, &NetworkClient::playerNumberReceived, 
                         playerWindow, [playerWindow, ip](int playerNumber) {
            playerWindow->setNetworkMode(true, playerNumber);
            qDebug() << "[Client] Assigned player number:" << playerNumber 
                     << "connected to" << ip;
        });

        // === Связываем ввод игрока с отправкой на сервер ===
        QObject::connect(playerWindow, &PlayerWindow::requestPlayerMove, 
                         client, [client](const std::string& dest) {
            client->sendMove(dest);
        });
        
        // === Обработка сетевых ошибок ===
        QObject::connect(client, &NetworkClient::showError, 
                         [](const QString& msg) {
            QMessageBox::warning(nullptr, "Сеть", msg);
        });

        QObject::connect(playerWindow, &QMainWindow::destroyed, 
                         &app, &QApplication::quit);

        playerWindow->show();
        return app.exec();
    }
}