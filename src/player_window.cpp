#include "player_window.h"
#include "ui_player_window.h"
#include "presenter.h"

#include <QMessageBox>
#include <QCompleter>
#include <QStringListModel>

PlayerWindow::PlayerWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PlayerWindow)
    , presenter_(nullptr)
    , map_(nullptr)
    , computerTimer_(new QTimer(this))
    , playerTurn_(true)
    , gameInitialized_(false)
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
}

PlayerWindow::~PlayerWindow() { delete ui; }

void PlayerWindow::setPresenter(Presenter* presenter) { presenter_ = presenter; }
void PlayerWindow::setMap(Map* map) { map_ = map; updateRegionNameCache(); }

void PlayerWindow::initGame() {
    gameInitialized_ = true;
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
void PlayerWindow::updateTurn(int turn) {
    playerTurn_ = (turn == 0);
    enablePlayerInput(playerTurn_ && gameInitialized_);
    refreshTextLog();
}
void PlayerWindow::updateVisitedList(const std::vector<std::string>& names) {
    currentPath_ = names; refreshTextLog();
}
void PlayerWindow::updateNeighborList(const std::vector<std::string>& names) {
    currentNeighbors_ = names; refreshTextLog();
}

void PlayerWindow::onGameFinished(int winner) {
    enablePlayerInput(false);
    QString title = (winner == 0) ? "Вы победили!" : "Вы проиграли!";
    QString msg = (winner == 0) ? "Вы дошли до конечного региона!" : "Компьютер победил!";
    QMessageBox::information(this, title, msg);
    
    if (QMessageBox::question(this, "Новая игра", "Начать заново?", 
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        emit requestResetGame();
    }
}

// === Логика UI ===
void PlayerWindow::enablePlayerInput(bool enabled) {
    ui->regionInput->setEnabled(enabled);
    ui->makeMoveButton->setEnabled(enabled);
    
    if (enabled) {
        ui->regionInput->setPlaceholderText("Введите название региона");
        // ✅ Безопасный фокус через таймер (не блокирует macOS Qt)
        QTimer::singleShot(50, this, [this]() { if (ui->regionInput) ui->regionInput->setFocus();});
    } else {
        ui->regionInput->setPlaceholderText("Ожидание...");
    }
}

void PlayerWindow::refreshTextLog() {
    if (!gameInitialized_) return;
    ui->gameInfoText->clear();
    ui->gameInfoText->append("Текущий: <b>" + QString::fromStdString(currentRegionName_) + "</b>");
    ui->gameInfoText->append("Цель: <b>" + QString::fromStdString(finalRegionName_) + "</b>");
    ui->gameInfoText->append("Ошибки: <b>" + QString::number(mistakesCount_) + "/3</b>");
    ui->gameInfoText->append("");
    
    if (!currentPath_.empty()) {
        QString path;
        for (size_t i = 0; i < currentPath_.size(); ++i) {
            if (i > 0) path += " → ";
            path += QString::fromStdString(currentPath_[i]);
        }
        ui->gameInfoText->append("Путь: " + path);
    }
    
    ui->gameInfoText->append("------------------------------");
    ui->gameInfoText->append(playerTurn_ ? "<font color='green'><b>Ваш ход!</b></font>" 
                                         : "<font color='blue'><b>Ход компьютера...</b></font>");
    
    auto cursor = ui->gameInfoText->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->gameInfoText->setTextCursor(cursor);
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