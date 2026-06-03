#pragma once
#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHeaderView>

class NetworkMenu : public QMainWindow {
    Q_OBJECT
public:
    explicit NetworkMenu(QWidget *parent = nullptr);
    ~NetworkMenu();

signals:
    void backToMainMenu();
    void joinGame(const QString& hostInfo);

private slots:
    void onRefreshClicked();
    void onJoinClicked();
    void onBackClicked();

private:
    QTableWidget *gameTable;
    QPushButton *btnRefresh;
    QPushButton *btnJoin;
    QPushButton *btnBack;
    QWidget *centralWidget;
    
    void populateMockGames();
};