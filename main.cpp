#include <QApplication>
#include <QMainWindow>
#include "start_menu.h"
#include "presenter.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    StartMenu menu;
    menu.show();

    menu.setOnStartGameCallback([&]() {
        auto* presenter = new Presenter();

        auto* mapWindow = new QMainWindow();
        mapWindow->setAttribute(Qt::WA_DeleteOnClose);
        mapWindow->setCentralWidget(presenter->getMapWidget());
        mapWindow->setWindowTitle("Map of Russia");
        mapWindow->resize(800, 600);

        auto* playerWindow = presenter->getPlayerWindow();
        playerWindow->setAttribute(Qt::WA_DeleteOnClose);
        playerWindow->setWindowTitle("Game Window");
        playerWindow->resize(1000, 800);

        QObject::connect(mapWindow, &QMainWindow::destroyed, &app, &QApplication::quit);
        QObject::connect(playerWindow, &QMainWindow::destroyed, &app, &QApplication::quit);

        mapWindow->show();
        playerWindow->show();
    });

    return app.exec();
}