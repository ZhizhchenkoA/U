#pragma once

#include <QObject>
#include <QThread>
#include <memory>

#include "demo.h"
#include "player_window.h"
#include "subject.h"
#include "gameworker.h"

class Presenter : public QObject {
    Q_OBJECT
public:
    explicit Presenter(QObject* parent = nullptr);
    ~Presenter() override;

    MapWidget* getMapWidget() const;
    PlayerWindow* getPlayerWindow() const;

    // Запуск игры (вызывается из main или после полной инициализации)
    void startGame();

private slots:
    // Перенаправление команд из UI в Worker
    void forwardPlayerMove(const std::string& destination);
    void forwardComputerMove();
    void forwardResetGame();

    // Обработка ответов от Worker
    void onPlayerMoveResult(int code);
    void onComputerMoveResult(int code);
    void onGameFinished(int winner);
    void onGameStateChanged();
    void onGameReady();

private:
    void setupWorkerThread();
    void setupConnections();

    Map map_;
    MapWidget* mapWidget_;
    PlayerWindow* playerWindow_;

    QThread* workerThread_;
    GameWorker* gameWorker_;
};
