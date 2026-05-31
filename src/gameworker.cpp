#include "gameworker.h"
#include <QThread>
#include <QDebug>
#include <algorithm>

GameWorker::GameWorker(QObject* parent)
    : QObject(parent), game_(nullptr), subjects_(nullptr)
{
    qDebug() << "[GameWorker] Created in thread:" << QThread::currentThreadId();
}

GameWorker::~GameWorker() {
    onQuit();
    qDebug() << "[GameWorker] Destroyed";
}

void GameWorker::init(int numberOfSubjects, std::list<AbstractSubject*>* subjects,
                      AbstractSubject* start, AbstractSubject* final) {
    std::lock_guard<std::mutex> lock(gameMutex_);
    
    if (!subjects || subjects->empty()) {
        qWarning() << "[GameWorker] Invalid subjects list!";
        return;
    }
    
    subjects_ = subjects;
    game_ = std::make_unique<Game>(numberOfSubjects, *subjects);
    
    qDebug() << "[GameWorker] Game initialized with" << numberOfSubjects << "subjects";
    qDebug() << "[GameWorker] Thread:" << QThread::currentThreadId();
    
    // Если start/final не указаны, Game выберет их случайно
    // Но можно принудительно установить, если нужно
    if (start && final) {
        // В текущей реализации Game выбирает start/final в конструкторе
        // Если нужно изменить — добавьте методы в Game
    }
    
    emitGameState();
    qDebug() << "[GameWorker] Emitting gameReady signal...";
    emit gameReady(); 
}

void GameWorker::onPlayerMove(const std::string& destination) {
    std::lock_guard<std::mutex> lock(gameMutex_);
    
    if (!running_ || !game_) {
        qWarning() << "[GameWorker] Game not initialized or stopped!";
        return;
    }
    
    qDebug() << "[GameWorker] Player move:" << QString::fromStdString(destination)
             << "in thread:" << QThread::currentThreadId();
    
    int result = game_->makePlayerMove(destination);
    emit playerMoveResult(result);
    
    if (game_->isGameFinished()) {
        emit gameFinished(game_->getWinner());
    }
    
    emitGameState();
}

void GameWorker::onComputerMove() {
    std::lock_guard<std::mutex> lock(gameMutex_);
    
    if (!running_ || !game_) {
        qWarning() << "[GameWorker] Game not initialized or stopped!";
        return;
    }
    
    qDebug() << "[GameWorker] Computer move in thread:" << QThread::currentThreadId();
    
    // Имитация "думания" компьютера (можно настроить)
    QThread::msleep(300);
    
    int result = game_->makeComputerMove();
    emit computerMoveResult(result);
    
    if (game_->isGameFinished()) {
        emit gameFinished(game_->getWinner());
    }
    
    emitGameState();
}

void GameWorker::onReset() {
    std::lock_guard<std::mutex> lock(gameMutex_);
    
    if (!game_) {
        qWarning() << "[GameWorker] Cannot reset - game not initialized!";
        return;
    }
    
    qDebug() << "[GameWorker] Resetting game in thread:" << QThread::currentThreadId();
    
    game_->reset();
    emitGameState();
}

void GameWorker::onQuit() {
    running_ = false;
    qDebug() << "[GameWorker] Quit signal received";
}

// === Вспомогательные методы ===

void GameWorker::emitGameState() {
    // Этот метод вызывается с захваченным gameMutex_
    if (!game_) return;
    
    emit currentRegionChanged(game_->getCurrentRegionName());
    emit startRegionChanged(game_->getStartRegionName());
    emit finalRegionChanged(game_->getFinalRegionName());
    emit neighborListUpdated(game_->getNeighborRegionNames());
    emit visitedListUpdated(game_->getVisitedRegionNames());
    emit mistakesUpdated(game_->getMistakesCount());
    emit turnChanged(game_->getTurn());
    
    emit gameStateChanged();
}

bool GameWorker::isValidMove(const std::string& destination, AbstractSubject*& outSubject) {
    // Этот метод вызывается с захваченным gameMutex_
    if (!game_ || !subjects_) return false;
    
    // Поиск региона по имени
    for (auto* subject : *subjects_) {
        const auto& names = subject->get_names();
        if (std::find(names.begin(), names.end(), destination) != names.end()) {
            outSubject = subject;
            return true;
        }
    }
    
    return false;
}