#pragma once
#include <QObject>
#include <QThread>
#include <QElapsedTimer> // <-- ОБЯЗАТЕЛЬНО ДОБАВИТЬ ЭТОТ ИНКЛУД
#include <memory>
#include <mutex>
#include <vector>
#include <string>
#include "game.h"
#include "subject.h"

class GameWorker : public QObject {
    Q_OBJECT
public:
    explicit GameWorker(QObject* parent = nullptr);
    ~GameWorker() override;

    // Инициализация игры (вызывать из главного потока перед стартом)
    void init(int numberOfSubjects, std::list<AbstractSubject*>* subjects,
              AbstractSubject* start = nullptr, AbstractSubject* final = nullptr);
    int makeNetworkMove(const std::string& destination, int expectedTurn);
    Game* getGame() const { return game_.get(); }

public slots:
    // Слоты для управления игрой (вызываются из GUI-потока)
    void onPlayerMove(const std::string& destination);
    void onComputerMove();
    void onReset();
    void onQuit();

signals:
    // Сигналы для обновления UI (отправляются в GUI-поток)
    void playerMoveResult(int code);      // 0=ok, 1=win, -1=invalid, -2=lose
    void computerMoveResult(int code);    // 0=ok, 1=win, -2=no moves
    void gameFinished(int winner);        // 0=player, 1=computer
    void gameStateChanged();              // общее обновление состояния
    
    // Новый сигнал для передачи общего времени обдумывания в UI
    void thinkTimesUpdated(int playerTotalSec, int computerTotalSec);

    // Детальные обновления для UI
    void currentRegionChanged(const std::string& name);
    void startRegionChanged(const std::string& name);
    void finalRegionChanged(const std::string& name);
    void neighborListUpdated(const std::vector<std::string>& names);
    void visitedListUpdated(const std::vector<std::string>& names);
    void mistakesUpdated(int count);
    void turnChanged(int turn);  // 0=player, 1=computer
    void gameReady();

private:
    // Вспомогательные методы
    void emitGameState();
    bool isValidMove(const std::string& destination, AbstractSubject*& outSubject);

    std::unique_ptr<Game> game_;
    std::mutex gameMutex_;
    bool running_ = true;

    // Кэш для быстрого доступа (копия указателей из Map)
    std::list<AbstractSubject*>* subjects_ = nullptr;
    
    // Таймер для замера длительности хода
    QElapsedTimer turnTimer_;
};