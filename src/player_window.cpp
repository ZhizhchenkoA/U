#include "player_window.h"
#include "ui_player_window.h"
#include "presenter.h"

#include <QMessageBox>
#include <QCompleter>
#include <QStringListModel>

PlayerWindow::PlayerWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::PlayerWindow), presenter_(nullptr), map_(nullptr),
      computerTimer_(new QTimer(this)), playerTurn_(true), gameInitialized_(false),
      uiTickTimer_(new QTimer(this)) 
{
    ui->setupUi(this);
    setWindowTitle("Турнир Угольникова");
    
    
    connect(ui->makeMoveButton, &QPushButton::clicked,
            this, &PlayerWindow::on_makeMoveButton_clicked);
    connect(ui->regionInput, &QLineEdit::returnPressed,
            this, &PlayerWindow::on_regionInput_returnPressed);
    
    auto* completer = new QCompleter(QStringList(), this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
    ui->regionInput->setCompleter(completer);
    
    enablePlayerInput(false);
    connect(uiTickTimer_, &QTimer::timeout, this, &PlayerWindow::onUiTick);
    uiTickTimer_->start(1000);
    
    enablePlayerInput(false);
}

PlayerWindow::~PlayerWindow() { delete ui; }

void PlayerWindow::setPresenter(Presenter* presenter) { presenter_ = presenter; }
void PlayerWindow::setMap(Map* map) { map_ = map; updateRegionNameCache(); }

void PlayerWindow::initGame() {
    gameInitialized_ = true;
    currentTurnTimer_.start(); 
    enablePlayerInput(true);
    refreshTextLog();
}

void PlayerWindow::updateRegionNameCache() {
    if (!map_) return;
    regionNamesCache_.clear();
    for (auto* subj : map_->get_subjects()) {
        for (const auto& name : subj->get_names()) {
            regionNamesCache_.push_back(name);
        }
    }
    if (auto* c = ui->regionInput->completer()) {
        QStringList model;
        for (const auto& n : regionNamesCache_) model << QString::fromStdString(n);
        if (auto* lm = qobject_cast<QStringListModel*>(c->model())) lm->setStringList(model);
        else c->setModel(new QStringListModel(model, c));
    }
}

// === Слоты обновления от Worker ===
void PlayerWindow::updateCurrentRegion(const std::string& name) {
    currentRegionName_ = name; refreshTextLog();
}
void PlayerWindow::updateFinalRegion(const std::string& name) {
    finalRegionName_ = name; refreshTextLog();
}
void PlayerWindow::updateMistakes(int count) {
    mistakesCount_ = count; refreshTextLog();
}

void PlayerWindow::updateVisitedList(const std::vector<std::string>& names) {
    currentPath_ = names; refreshTextLog();
}
void PlayerWindow::updateNeighborList(const std::vector<std::string>& names) {
    currentNeighbors_ = names; refreshTextLog();
}

void PlayerWindow::updateThinkTimes(int playerTotalSec, int computerTotalSec) {
    currentPlayerTotalTime_ = playerTotalSec;
    currentComputerTotalTime_ = computerTotalSec;
    refreshTextLog();
}

void PlayerWindow::onUiTick() {
    if (!gameInitialized_) return;
    // Просто обновляем текст, чтобы показать прошедшее время текущего хода
    refreshTextLog();
}

// === Логика UI ===
void PlayerWindow::enablePlayerInput(bool enabled) {
    ui->regionInput->setEnabled(enabled);
    ui->makeMoveButton->setEnabled(enabled);
    
    if (enabled) {
        ui->regionInput->setPlaceholderText("Введите название региона");
        // Безопасный фокус через таймер (не блокирует macOS Qt)
        QTimer::singleShot(50, this, [this]() { if (ui->regionInput) ui->regionInput->setFocus();});
    } else {
        ui->regionInput->setPlaceholderText("Ожидание...");
    }
}


void PlayerWindow::on_makeMoveButton_clicked() { on_regionInput_returnPressed(); }

void PlayerWindow::on_regionInput_returnPressed() {
    if (!playerTurn_ || !gameInitialized_) return;
    QString input = ui->regionInput->text().trimmed();
    if (input.isEmpty()) {
        ui->gameInfoText->append("<font color='red'>Введите название региона!</font>");
        return;
    }
    emit requestPlayerMove(input.toStdString());
    enablePlayerInput(false);
    ui->regionInput->clear();
}

// === НОВЫЙ МЕТОД ===
void PlayerWindow::setNetworkMode(bool isNetwork, int playerNumber) {
    isNetworkMode_ = isNetwork;
    myPlayerNumber_ = playerNumber;
    
    if (isNetworkMode_ && playerNumber > 0) {
        setWindowTitle(QString("Игра — Игрок %1").arg(playerNumber));
    }
    
    refreshTextLog();
}

// === ИЗМЕНЁННЫЙ МЕТОД updateTurn ===
void PlayerWindow::updateTurn(int turn) {
    if (isNetworkMode_ && myPlayerNumber_ > 0) {
        // В сетевом режиме: turn 0 = ход Игрока 1, turn 1 = ход Игрока 2
        playerTurn_ = (turn == myPlayerNumber_ - 1);
    } else {
        // В одиночном режиме: turn 0 = ход игрока, turn 1 = ход компьютера
        playerTurn_ = (turn == 0);
    }
    
    currentTurnTimer_.restart();
    enablePlayerInput(playerTurn_ && gameInitialized_);
    refreshTextLog();
}

// === ИЗМЕНЁННЫЙ МЕТОД refreshTextLog ===
void PlayerWindow::refreshTextLog() {
    if (!gameInitialized_) return;
    ui->gameInfoText->clear();
    
    // === ОТОБРАЖЕНИЕ НОМЕРА ИГРОКА В СЕТЕВОМ РЕЖИМЕ ===
    if (isNetworkMode_ && myPlayerNumber_ > 0) {
        QString playerColor = (myPlayerNumber_ == 1) ? "blue" : "red";
        ui->gameInfoText->append(QString("<font color='%1' size='4'><b>Вы — Игрок %2</b></font>")
                                     .arg(playerColor).arg(myPlayerNumber_));
        ui->gameInfoText->append("");
    }
    
    ui->gameInfoText->append("Текущий: <b>" + QString::fromStdString(currentRegionName_) + "</b>");
    ui->gameInfoText->append("Цель: <b>" + QString::fromStdString(finalRegionName_) + "</b>");
    ui->gameInfoText->append("Ошибки: <b>" + QString::number(mistakesCount_) + "/3</b>");
    ui->gameInfoText->append("");
    
    int currentTurnSec = static_cast<int>(currentTurnTimer_.elapsed() / 1000);
    ui->gameInfoText->append("Время на текущий ход: <b>" + QString::number(currentTurnSec) + " сек</b>");
    
    if (!currentPath_.empty()) {
        QString path;
        for (size_t i = 0; i < currentPath_.size(); ++i) {
            if (i > 0) path += " → ";
            path += QString::fromStdString(currentPath_[i]);
        }
        ui->gameInfoText->append("Путь: " + path);
    }
    
    ui->gameInfoText->append("------------------------------");
    
    // === РАЗНЫЕ СООБЩЕНИЯ ДЛЯ РАЗНЫХ РЕЖИМОВ ===
    if (isNetworkMode_) {
        if (playerTurn_) {
            ui->gameInfoText->append("<font color='green'><b>Ваш ход!</b></font>");
        } else {
            ui->gameInfoText->append("<font color='orange'><b>Ход соперника...</b></font>");
        }
    } else {
        ui->gameInfoText->append(playerTurn_ ? "<font color='green'><b>Ваш ход!</b></font>"
                                             : "<font color='blue'><b>Ход компьютера...</b></font>");
    }
                                         
    QTextCursor cursor = ui->gameInfoText->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->gameInfoText->setTextCursor(cursor);
}

// === ИЗМЕНЁННЫЙ МЕТОД onGameFinished ===
void PlayerWindow::onGameFinished(int winner) {
    enablePlayerInput(false);
    
    QString title, msg;
    if (isNetworkMode_) {
        // В сетевом режиме winner = 0 означает победу Игрока 1, winner = 1 — Игрока 2
        if (winner == myPlayerNumber_ - 1) {
            title = "Победа!";
            msg = "Поздравляем, вы победили!";
        } else {
            title = "Поражение";
            msg = "Победил соперник.";
        }
    } else {
        title = (winner == 0) ? "Вы победили!" : "Вы проиграли!";
        msg = (winner == 0) ? "Вы победили!" : "Компьютер победил!";
    }
    
    QMessageBox::information(this, title, msg);
    
    // В сетевом режиме не предлагаем начать заново (это делает хост)
    if (!isNetworkMode_) {
        if (QMessageBox::question(this, "Новая игра", "Начать заново?",
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            emit requestResetGame();
        }
    }
}