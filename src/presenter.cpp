#include "presenter.h"
#include "gameworker.h"
#include "player_window.h"
#include "demo.h"

#include <QThread>
#include <QMetaObject>
#include <QMessageBox>
#include <QTimer>

Presenter::Presenter(QObject *parent)
    : QObject(parent),
      mapWidget_(nullptr),
      playerWindow_(nullptr),
      workerThread_(new QThread(this)),
      gameWorker_(new GameWorker())
{
    // 1. Загрузка данных карты (выполняется в главном потоке)
    map_.get_from_JSON("data/new_russia (1).geojson", "data/russia_neighbours.json");

    // 2. Создание виджетов
    mapWidget_ = new MapWidget(&map_);
    playerWindow_ = new PlayerWindow();
    playerWindow_->setMap(&map_);
    playerWindow_->setPresenter(this);

    // 3. Настройка многопоточности
    setupWorkerThread();
    setupConnections();

    // 4. Асинхронная инициализация игры в рабочем потоке
    QMetaObject::invokeMethod(gameWorker_, [=]() {
        auto& subjects = map_.get_subjects();
        gameWorker_->init(static_cast<int>(subjects.size()), &subjects);
        // Сигнал gameReady() отправляется внутри GameWorker::init()
    }, Qt::QueuedConnection);
}

Presenter::~Presenter() {
    // Безопасная остановка рабочего потока
    if (workerThread_->isRunning()) {
        QMetaObject::invokeMethod(gameWorker_, &GameWorker::onQuit, Qt::QueuedConnection);
        workerThread_->quit();
        if (!workerThread_->wait(3000)) {
            workerThread_->terminate();
        }
    }
    
    
    delete playerWindow_;
}

void Presenter::setupWorkerThread() {
    gameWorker_->moveToThread(workerThread_);
    workerThread_->start();
}

void Presenter::setupConnections() {
    // UI - Presenter - Worker (Команды игрока)
    connect(playerWindow_, &PlayerWindow::requestPlayerMove,
            this, &Presenter::forwardPlayerMove, Qt::QueuedConnection);
    connect(playerWindow_, &PlayerWindow::requestComputerMove,
            this, &Presenter::forwardComputerMove, Qt::QueuedConnection);
    connect(playerWindow_, &PlayerWindow::requestResetGame,
            this, &Presenter::forwardResetGame, Qt::QueuedConnection);

    // Worker - Presenter/UI (Обновления состояния)
    connect(gameWorker_, &GameWorker::gameReady,
            this, &Presenter::onGameReady, Qt::QueuedConnection);
    connect(gameWorker_, &GameWorker::gameStateChanged,
            this, &Presenter::onGameStateChanged, Qt::QueuedConnection);
    connect(gameWorker_, &GameWorker::gameFinished,
            this, &Presenter::onGameFinished, Qt::QueuedConnection);
    connect(gameWorker_, &GameWorker::playerMoveResult,
            this, &Presenter::onPlayerMoveResult, Qt::QueuedConnection);
    connect(gameWorker_, &GameWorker::computerMoveResult,
            this, &Presenter::onComputerMoveResult, Qt::QueuedConnection);

    connect(gameWorker_, &GameWorker::currentRegionChanged,
            playerWindow_, &PlayerWindow::updateCurrentRegion, Qt::QueuedConnection);
    connect(gameWorker_, &GameWorker::finalRegionChanged,
            playerWindow_, &PlayerWindow::updateFinalRegion, Qt::QueuedConnection);

    connect(gameWorker_, &GameWorker::mistakesUpdated,
        playerWindow_, &PlayerWindow::updateMistakes, Qt::QueuedConnection);
    
    connect(gameWorker_, &GameWorker::visitedListUpdated,
            playerWindow_, &PlayerWindow::updateVisitedList, Qt::QueuedConnection);
    connect(gameWorker_, &GameWorker::neighborListUpdated,
            playerWindow_, &PlayerWindow::updateNeighborList, Qt::QueuedConnection);
    connect(gameWorker_, &GameWorker::turnChanged,
        playerWindow_, &PlayerWindow::updateTurn, Qt::QueuedConnection);
}

// === Реализация слотов-посредников ===

void Presenter::onGameReady() {
    playerWindow_->updateMistakes(0);
    playerWindow_->initGame();
    qDebug() << "UI unlocked";
}

void Presenter::forwardPlayerMove(const std::string& destination) {
    QMetaObject::invokeMethod(gameWorker_, [=]() {
        gameWorker_->onPlayerMove(destination);
    }, Qt::QueuedConnection);
}

void Presenter::forwardComputerMove() {
    QMetaObject::invokeMethod(gameWorker_, &GameWorker::onComputerMove, Qt::QueuedConnection);
}

void Presenter::forwardResetGame() {
    QMetaObject::invokeMethod(gameWorker_, &GameWorker::onReset, Qt::QueuedConnection);
}

// === Обработка ответов от Worker ===

void Presenter::onGameStateChanged() {
    mapWidget_->rebuildCache();
    mapWidget_->update();
}

void Presenter::onGameFinished(int winner) {
    // Сброс визуальных флагов "посещено" на карте
    auto& subjects = map_.get_subjects();
    for (auto* subj : subjects) {
        subj->unvisit();
    }

    mapWidget_->clearCache();
    mapWidget_->rebuildCache();
    mapWidget_->update();

    // Показываем диалог результата в окне игрока
    playerWindow_->onGameFinished(winner);
}

void Presenter::onPlayerMoveResult(int code) {
    if (code == 0) {
        // Ход игрока успешен
        QTimer::singleShot(600, this, [this]() {
            forwardComputerMove();
        });
    } else if (code == -1) {
        QMessageBox::warning(playerWindow_, "Ход", "Неверный регион или ход невозможен!");
    } else if (code == -2) {
        QMessageBox::information(playerWindow_, "Игра окончена", "Вы проиграли (3 ошибки)!");
    }
}

void Presenter::onComputerMoveResult(int code) {
    if (code == -2) {
        QMessageBox::information(playerWindow_, "Игра окончена", "Компьютер не может сделать ход. Вы победили!");
    } else if (code == -1) {
        playerWindow_->updateTurn(0);
    }
}

MapWidget* Presenter::getMapWidget() const { return mapWidget_; }
PlayerWindow* Presenter::getPlayerWindow() const { return playerWindow_; }