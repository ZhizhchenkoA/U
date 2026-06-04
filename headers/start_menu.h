#pragma once
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <functional>

class StartMenu : public QMainWindow {
    Q_OBJECT
public:
    explicit StartMenu(QWidget *parent = nullptr);
    ~StartMenu();

    void setOnStartGameCallback(std::function<void()> callback);

private slots:
    void onStartComputerClicked();
    void onSelectMapClicked();

private:
    QVBoxLayout *mainLayout;
    QLabel *titleLabel;
    QPushButton *btnComputer;
    QPushButton *btnMap;
    std::function<void()> onStartGame;
};