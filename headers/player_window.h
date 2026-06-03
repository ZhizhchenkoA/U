#pragma once
#include <QMainWindow>
#include <QTimer>
#include <QElapsedTimer>
#include <QString>
#include <vector>
#include <string>

namespace Ui { class PlayerWindow; }
class Presenter;
class Map;

class PlayerWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit PlayerWindow(QWidget *parent = nullptr);
    ~PlayerWindow() override;
    void setPresenter(Presenter* presenter);
    void setMap(Map* map);
    void initGame();
    
    // === НОВЫЙ МЕТОД ДЛЯ СЕТЕВОГО РЕЖИМА ===
    void setNetworkMode(bool isNetwork, int playerNumber = 0);

public slots:
    void updateCurrentRegion(const std::string& name);
    void updateFinalRegion(const std::string& name);
    void updateMistakes(int count);
    void updateTurn(int turn);
    void updateVisitedList(const std::vector<std::string>& names);
    void updateNeighborList(const std::vector<std::string>& names);
    void onGameFinished(int winner);
    void updateThinkTimes(int playerTotalSec, int computerTotalSec);

private slots:
    void on_makeMoveButton_clicked();
    void on_regionInput_returnPressed();
    void onUiTick();

signals:
    void requestPlayerMove(const std::string& destination);
    void requestComputerMove();
    void requestResetGame();

private:
    void setupConnections();
    void updateRegionNameCache();
    void enablePlayerInput(bool enabled);
    void refreshTextLog();

    Ui::PlayerWindow *ui;
    Presenter* presenter_ = nullptr;
    Map* map_ = nullptr;
    QTimer* computerTimer_;
    QTimer* uiTickTimer_;
    QElapsedTimer currentTurnTimer_;

    bool playerTurn_ = true;
    bool gameInitialized_ = false;
    
    // === НОВЫЕ ПОЛЯ ДЛЯ СЕТЕВОГО РЕЖИМА ===
    bool isNetworkMode_ = false;
    int myPlayerNumber_ = 0; // 1 или 2 (0 — не задан)

    std::string currentRegionName_;
    std::string finalRegionName_;
    int mistakesCount_ = 0;
    std::vector<std::string> currentPath_;
    std::vector<std::string> currentNeighbors_;
    
    int currentPlayerTotalTime_ = 0;
    int currentComputerTotalTime_ = 0;

    mutable std::vector<std::string> regionNamesCache_;
};