#include "start_menu.h"
#include <QMessageBox>
#include <QFileDialog>

StartMenu::StartMenu(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Турнир Угольникова. Меню");
    resize(400, 300);

    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    mainLayout = new QVBoxLayout(central);

    titleLabel = new QLabel("Турнир Угольникова", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; margin: 20px;");
    mainLayout->addWidget(titleLabel);

    btnComputer = new QPushButton("Игра против компьютера", this);
    btnNetwork = new QPushButton("Сетевая игра", this);
    btnMap = new QPushButton("Выбор карты", this);

    QString btnStyle = "padding: 12px; font-size: 16px; margin: 8px; border-radius: 5px;";
    btnComputer->setStyleSheet(btnStyle + "background-color: #4CAF50; color: white;");
    btnNetwork->setStyleSheet(btnStyle + "background-color: #2196F3; color: white;");
    btnMap->setStyleSheet(btnStyle);

    mainLayout->addWidget(btnComputer);
    mainLayout->addWidget(btnNetwork);
    mainLayout->addWidget(btnMap);
    mainLayout->addStretch();

    connect(btnComputer, &QPushButton::clicked, this, &StartMenu::onStartComputerClicked);
    connect(btnNetwork, &QPushButton::clicked, this, &StartMenu::onStartNetworkClicked);
    connect(btnMap, &QPushButton::clicked, this, &StartMenu::onSelectMapClicked);
}

StartMenu::~StartMenu() = default;

void StartMenu::setOnStartGameCallback(std::function<void()> callback) {
    onStartGame = std::move(callback);
}

void StartMenu::onStartComputerClicked() {
    if (onStartGame) {
        hide();
        onStartGame();
    }
}

void StartMenu::onStartNetworkClicked() {
    QMessageBox::information(this, "В разработке", 
        "Игру по сети украли пришельцы.\n"
        "Пока доступна только игра с компьютером.");
}

void StartMenu::onSelectMapClicked() {
    QString fileName = QFileDialog::getOpenFileName(this, "Select Map File", "", 
        "GeoJSON (*.geojson *.json);;All Files (*)");
    
    if (!fileName.isEmpty()) {
        QMessageBox::information(this, "Map Selected", 
            QString("File: %1\n\n"
                    "Dynamic map loading is under development.\n"
                    "The default map is currently in use.").arg(fileName));
    }
}