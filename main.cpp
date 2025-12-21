#include <QApplication>
#include <QMainWindow>

#include "presenter.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Presenter presenter;

    // Окно с картой
    QMainWindow mapWindow;
    mapWindow.setCentralWidget(presenter.getMapWidget());
    mapWindow.setWindowTitle("Карта России");
    mapWindow.resize(800, 600);

    // Игровое окно
    presenter.getPlayerWindow()->setWindowTitle("Игровое окно");
    presenter.getPlayerWindow()->resize(600, 400);

    // Показываем оба окна
    mapWindow.show();
    presenter.getPlayerWindow()->show();

    return app.exec();
}
