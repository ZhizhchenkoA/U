#include "network_menu.h"
#include <QMessageBox>
#include <QTableWidgetItem>

NetworkMenu::NetworkMenu(QWidget *parent) : QMainWindow(parent) {
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("LAN Game Browser");
    resize(600, 400);

    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    auto *mainLayout = new QVBoxLayout(centralWidget);

    // Таблица с найденными играми
    gameTable = new QTableWidget(0, 3, this);
    gameTable->setHorizontalHeaderLabels({"IP", "Players", "Status"});
    gameTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    gameTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    gameTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainLayout->addWidget(gameTable);

    // Кнопки управления
    auto *buttonLayout = new QHBoxLayout();
    btnBack = new QPushButton("Back to Menu", this);
    btnRefresh = new QPushButton("Refresh List", this);
    btnJoin = new QPushButton("Join Selected", this);

    buttonLayout->addWidget(btnBack);
    buttonLayout->addStretch();
    buttonLayout->addWidget(btnRefresh);
    buttonLayout->addWidget(btnJoin);
    mainLayout->addLayout(buttonLayout);

    // Связи
    connect(btnBack, &QPushButton::clicked, this, &NetworkMenu::onBackClicked);
    connect(btnRefresh, &QPushButton::clicked, this, &NetworkMenu::onRefreshClicked);
    connect(btnJoin, &QPushButton::clicked, this, &NetworkMenu::onJoinClicked);

    // Начальная загрузка (заглушка)
    onRefreshClicked();
}

NetworkMenu::~NetworkMenu() = default;

void NetworkMenu::onRefreshClicked() {
    gameTable->setRowCount(0);
    populateMockGames();
    
    // TODO: В будущем здесь будет отправка UDP broadcast пакета
    // и ожидание ответов от серверов в локальной сети.
}

void NetworkMenu::populateMockGames() {
    struct MockGame { QString host; QString players; QString status; };
    QList<MockGame> mocks = {
        {"ip1", "1/2", "Waiting"},
        {"ip2", "1/2", "Waiting"},
        {"ip3", "1/2", "Waiting"}
    };

    for (const auto& g : mocks) {
        int row = gameTable->rowCount();
        gameTable->insertRow(row);
        gameTable->setItem(row, 0, new QTableWidgetItem(g.host));
        gameTable->setItem(row, 1, new QTableWidgetItem(g.players));
        gameTable->setItem(row, 2, new QTableWidgetItem(g.status));
    }
}

void NetworkMenu::onJoinClicked() {
    int row = gameTable->currentRow();
    if (row == -1) {
        QMessageBox::warning(this, "No Selection", "Please select a game from the list.");
        return;
    }
    
    QString host = gameTable->item(row, 0)->text();
    emit joinGame(host);
    
    // TODO: Здесь будет логика подключения к серверу через QTcpSocket
    QMessageBox::information(this, "Joining Game", 
        QString("Connecting to: %1\n(Network logic pending)").arg(host));
}

void NetworkMenu::onBackClicked() {
    emit backToMainMenu();
    close();
}