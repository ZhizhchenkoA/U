#include <QApplication>
#include <QMainWindow>
#include "start_menu.h"
#include "presenter.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    StartMenu menu;
    menu.show();
    
    // Callback вызывается при нажатии "Играть с компьютером"
    menu.setOnStartGameCallback([&]() {
        // Создаем Presenter. Он сам создаст Map, MapWidget и PlayerWindow.
        auto* presenter = new Presenter();
        
        // Создаем окно для карты
        auto* mapWindow = new QMainWindow();
        mapWindow->setAttribute(Qt::WA_DeleteOnClose);
        mapWindow->setCentralWidget(presenter->getMapWidget());
        mapWindow->setWindowTitle("Карта");
        mapWindow->resize(800, 600);
        
        // Создаем окно для управления игрой
        auto* playerWindow = presenter->getPlayerWindow();
        playerWindow->setAttribute(Qt::WA_DeleteOnClose);
        playerWindow->setWindowTitle("Турнир Угольникова");
        playerWindow->resize(1000, 800);
        
        // Завершаем приложение, когда закрывается любое из окон
        QObject::connect(mapWindow, &QMainWindow::destroyed, &app, &QApplication::quit);
        QObject::connect(playerWindow, &QMainWindow::destroyed, &app, &QApplication::quit);
        
        // Очищаем presenter, когда окна закрываются (исправление утечки памяти)
        QObject::connect(mapWindow, &QMainWindow::destroyed, presenter, &QObject::deleteLater);
        
        mapWindow->show();
        playerWindow->show();
        
        // Запускаем игру
        presenter->startGame();
    });
    
    return app.exec();
}