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
    
    turnTimer_.start(); 
    
    emitGameState();
    emit gameReady();
}

void GameWorker::onPlayerMove(const std::string& destination) {
    std::lock_guard<std::mutex> lock(gameMutex_);
    if (!running_ || !game_) return;

    qint64 elapsedMs = turnTimer_.elapsed();
    turnTimer_.restart();
    game_->addPlayerTime(static_cast<int>(elapsedMs / 1000));

    int result = game_->makePlayerMove(destination);
    emit playerMoveResult(result);
    
    if (game_->isGameFinished()) {
        emit gameFinished(game_->getWinner());
    }
    
    emitGameState();
    
    emit thinkTimesUpdated(game_->getPlayerTotalTime(), game_->getComputerTotalTime());
}

void GameWorker::onComputerMove() {
    std::lock_guard<std::mutex> lock(gameMutex_);
    if (!running_ || !game_) return;
    turnTimer_.start();
    
    QThread::msleep(300);
    
    int result = game_->makeComputerMove();
    
    qint64 elapsedMs = turnTimer_.elapsed();
    turnTimer_.restart();
    game_->addComputerTime(static_cast<int>(elapsedMs / 1000));

    emit computerMoveResult(result);
    
    if (game_->isGameFinished()) {
        emit gameFinished(game_->getWinner());
    }
    
    emitGameState();
    emit thinkTimesUpdated(game_->getPlayerTotalTime(), game_->getComputerTotalTime());
}

void GameWorker::onReset() {
    std::lock_guard<std::mutex> lock(gameMutex_);
    if (!game_) return;
    
    game_->reset();
    turnTimer_.start();
    emitGameState();
    emit thinkTimesUpdated(0, 0);
}

void GameWorker::onQuit() {
    running_ = false;
    qDebug() << "[GameWorker] Quit signal received";
}

void GameWorker::emitGameState() {
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
    if (!game_ || !subjects_) 
        return false;

    for (AbstractSubject* subject : *subjects_) {
        const std::list<std::string>& names = subject->get_names();
        if (std::find(names.begin(), names.end(), destination) != names.end()) {
            outSubject = subject;
            return true;
        }
    }
    
    return false;
}

int GameWorker::makeNetworkMove(const std::string& destination, int expectedTurn) {
    std::lock_guard<std::mutex> lock(gameMutex_);
    if (!running_ || !game_) {
        qWarning() << "[GameWorker] Game not initialized or stopped!";
        return -1;
    }
    
    int result = game_->makeNetworkMove(destination, expectedTurn);
    if (game_->isGameFinished()) {
        emit gameFinished(game_->getWinner());
    }
    emitGameState();
    
    return result;
}