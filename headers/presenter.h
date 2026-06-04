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
    void startGame();

private slots:
    // Ui commands to worker
    void forwardPlayerMove(const std::string& destination);
    void forwardComputerMove();
    void forwardResetGame();

    // handling events from worker
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
