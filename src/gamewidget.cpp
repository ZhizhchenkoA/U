#include "gamewidget.h"
#include <iostream>

GameWidget::GameWidget(QWidget* parent) : QWidget(parent), game(nullptr) {
    createGame();
    
    // interface elements
    statusLabel = new QLabel("Игра началась! Ваш ход.");
    currentRegionLabel = new QLabel();
    finalRegionLabel = new QLabel();
    mistakesLabel = new QLabel();
    visitedRegionsText = new QTextEdit();
    visitedRegionsText->setReadOnly(true);
    
    regionInput = new QLineEdit();
    regionInput->setPlaceholderText("Введите название региона");
    
    moveButton = new QPushButton("Сделать ход");
    connect(moveButton, &QPushButton::clicked, this, &GameWidget::onPlayerMove);
    connect(regionInput, &QLineEdit::returnPressed, this, &GameWidget::onPlayerMove);
    
    restartButton = new QPushButton("Новая игра");
    connect(restartButton, &QPushButton::clicked, this, &GameWidget::onRestartGame);
    

    QVBoxLayout* mainLayout = new QVBoxLayout();
    
    QHBoxLayout* infoLayout = new QHBoxLayout();
    infoLayout->addWidget(currentRegionLabel);
    infoLayout->addWidget(finalRegionLabel);
    infoLayout->addWidget(mistakesLabel);
    
    mainLayout->addWidget(statusLabel);
    mainLayout->addLayout(infoLayout);
    
    // input
    QHBoxLayout* inputLayout = new QHBoxLayout();
    inputLayout->addWidget(new QLabel("Введите регион:"));
    inputLayout->addWidget(regionInput);
    inputLayout->addWidget(moveButton);
    
    mainLayout->addLayout(inputLayout);
  
    
    mainLayout->addWidget(new QLabel("Посещённые регионы:"));
    mainLayout->addWidget(visitedRegionsText);
    mainLayout->addWidget(restartButton);
    
    setLayout(mainLayout);
    setWindowTitle("Турнир Угольникова");
    
    updateUI();
}

GameWidget::~GameWidget() {
    delete game;
}

void GameWidget::createGame() {
    // Создаем тестовые данные
    List<AbstractSubject*> subjects;
    
    // Создаем регионы
    for (int i = 0; i < 5; ++i) {
        char name[20];
        sprintf(name, "Region%d", i+1);
        subjects.Add(new AbstractSubject(String(name)));
    }
    
    // Устанавливаем связи
    subjects.Get(0)->addNeighbor(subjects.Get(1)); // Region1 - Region2
    subjects.Get(0)->addNeighbor(subjects.Get(2)); // Region1 - Region3
    subjects.Get(1)->addNeighbor(subjects.Get(0)); // Region2 - Region1
    subjects.Get(2)->addNeighbor(subjects.Get(0)); // Region3 - Region1
    
    subjects.Get(1)->addNeighbor(subjects.Get(3)); // Region2 - Region4
    subjects.Get(3)->addNeighbor(subjects.Get(1)); // Region4 - Region2
    
    subjects.Get(2)->addNeighbor(subjects.Get(4)); // Region3 - Region5
    subjects.Get(4)->addNeighbor(subjects.Get(2)); // Region5 - Region3
    
    game = new Game(subjects.GetSize(), subjects);
}


void GameWidget::onPlayerMove() {
    if (!game || game->isGameFinished()) return;
    
    QString input = regionInput->text().trimmed();
    if (input.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите название региона!");
        return;
    }
    
    String destination(input.toStdString().c_str());
    int result = game->makePlayerMove(destination);
    
    if (result == 0) {
        // Успешный ход, теперь ход компьютера
        regionInput->clear();
        updateUI();
        
        // Через небольшую задержку делаем ход компьютера
        QTimer::singleShot(1000, this, &GameWidget::onComputerMove);
    } else if (result == 1) {
        // Игрок победил
        regionInput->clear();
        updateUI();
        QMessageBox::information(this, "Победа!", "Вы достигли конечного региона!");
    } else if (result == -1) {
        // Ошибка хода
        updateUI();
        QMessageBox::warning(this, "Ошибка", "Неверный ход! Проверьте:\n"
                                           "1. Регион должен быть соседним\n"
                                           "2. Регион не должен быть посещён ранее");
        regionInput->clear();
        regionInput->setFocus();
    } else if (result == -2) {
        // 3 ошибки
        regionInput->clear();
        updateUI();
        QMessageBox::information(this, "Проигрыш", "Вы совершили 3 ошибки. Компьютер победил.");
    }
}

void GameWidget::onComputerMove() {
    if (!game || game->isGameFinished()) return;
    
    int result = game->makeComputerMove();
    
    if (result == 0) {
        // Успешный ход компьютера
        updateUI();
        statusLabel->setText("Ход компьютера завершён. Ваш ход.");
        regionInput->setFocus();
    } else if (result == 1) {
        // Компьютер победил
        updateUI();
        QMessageBox::information(this, "Проигрыш", "Компьютер достиг конечного региона!");
    } else if (result == -2) {
        // У компьютера нет ходов
        updateUI();
        QMessageBox::information(this, "Победа!", "У компьютера нет ходов. Вы победили!");
    }
}

void GameWidget::onRestartGame() {
    delete game;
    createGame();
    updateUI();
    statusLabel->setText("Игра началась! Ваш ход.");
    regionInput->setFocus();
}

void GameWidget::updateUI() {
    if (!game) return;
    
    // Обновляем информацию о текущем состоянии
    currentRegionLabel->setText("Текущий регион: " + 
        QString::fromStdString(game->getCurrentRegionName().c_str()));
    
    finalRegionLabel->setText("Конечный регион: " + 
        QString::fromStdString(game->getFinalRegionName().c_str()));
    
    mistakesLabel->setText("Ошибки: " + 
        QString::number(game->getMistakesCount()) + "/3");
    
    // Обновляем список посещённых регионов
    List<String> visited = game->getVisitedRegionNames();
    QString visitedText;
    for (int i = 0; i < visited.GetSize(); i++) {
        visitedText += QString::fromStdString(visited.Get(i).c_str());
        if (i < visited.GetSize() - 1) {
            visitedText += " → ";
        }
    }
    visitedRegionsText->setText(visitedText);
    
    
    bool isPlayerTurn = (game->getTurn() == 0) && !game->isGameFinished();
    regionInput->setEnabled(isPlayerTurn);
    moveButton->setEnabled(isPlayerTurn);
    
    // Если ход игрока, устанавливаем фокус на поле ввода
    if (isPlayerTurn) {
        regionInput->setFocus();
    }
    
    if (game->isGameFinished()) {
        int winner = game->getWinner();
        if (winner == 0) {
            statusLabel->setText("Игра окончена. Вы победили!");
        } else if (winner == 1) {
            statusLabel->setText("Игра окончена. Компьютер победил!");
        }
    } else {
        if (game->getTurn() == 0) {
            statusLabel->setText("Ваш ход. Введите название соседнего региона.");
        } else {
            statusLabel->setText("Ход компьютера...");
        }
    }
}