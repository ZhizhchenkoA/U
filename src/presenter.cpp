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
    map_.get_from_JSON("data/new_russia (1).geojson", "data/russia_neighbours.json");
    mapWidget_ = new MapWidget(&map_);
    playerWindow_ = new PlayerWindow();
    playerWindow_->setMap(&map_);
    playerWindow_->setPresenter(this);
    setupWorkerThread();
    setupConnections();
    QMetaObject::invokeMethod(gameWorker_, [=]() {
        auto& subjects = map_.get_subjects();
        gameWorker_->init(static_cast<int>(subjects.size()), &subjects);
    }, Qt::QueuedConnection);
}

Presenter::~Presenter() {
    if (workerThread_->isRunning()) {
        QMetaObject::invokeMethod(gameWorker_, &GameWorker::onQuit, Qt::QueuedConnection);
        workerThread_->quit();
        if (!workerThread_->wait(3000)) 
            workerThread_->terminate();
    }
    delete playerWindow_;
}

void Presenter::setupWorkerThread() {
    gameWorker_->moveToThread(workerThread_);
    workerThread_->start();
}

void Presenter::setupConnections() {
    // UI - Presenter
    connect(playerWindow_, &PlayerWindow::requestPlayerMove,
            this, &Presenter::forwardPlayerMove, Qt::QueuedConnection);
    connect(playerWindow_, &PlayerWindow::requestComputerMove,
            this, &Presenter::forwardComputerMove, Qt::QueuedConnection);
    connect(playerWindow_, &PlayerWindow::requestResetGame,
            this, &Presenter::forwardResetGame, Qt::QueuedConnection);
    connect(gameWorker_, &GameWorker::thinkTimesUpdated,
            playerWindow_, &PlayerWindow::updateThinkTimes, Qt::QueuedConnection);
    
    // Worker - Presenter/UI 
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

void Presenter::onGameStateChanged() {
    mapWidget_->requestRebuildCache();
    mapWidget_->update();
}

void Presenter::onGameFinished(int winner) {
    std::list<AbstractSubject*>& subjects = map_.get_subjects();
    for (AbstractSubject* subj : subjects) {
        subj->unvisit();
    }

    mapWidget_->clearCache();
    mapWidget_->requestRebuildCache();
    mapWidget_->update();

    playerWindow_->onGameFinished(winner);
}

void Presenter::onPlayerMoveResult(int code) {
    if (code == 0) {
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

void Presenter::startGame() {
    qDebug() << "[Presenter] startGame() вызван. Ожидание сигнала gameReady...";
}
MapWidget* Presenter::getMapWidget() const { return mapWidget_; }
PlayerWindow* Presenter::getPlayerWindow() const { return playerWindow_; }