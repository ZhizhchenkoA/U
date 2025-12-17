#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QTimer>
#include "game.h"

class GameWidget : public QWidget {
    Q_OBJECT
    
public:
    GameWidget(QWidget* parent = nullptr);
    ~GameWidget();
    
private slots:
    void onPlayerMove();
    void onComputerMove();
    void onRestartGame();
    
private:
    void updateUI();
    void createGame();
    
    Game* game;
    QLabel* statusLabel;
    QLabel* currentRegionLabel;
    QLabel* finalRegionLabel;
    QLabel* mistakesLabel;
    QTextEdit* visitedRegionsText;
    QLineEdit* regionInput;
    QPushButton* moveButton;
    QPushButton* restartButton;
};