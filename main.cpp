#include <QApplication>
#include <QMainWindow>

#include "presenter.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Presenter presenter;


    QMainWindow mapWindow;
    mapWindow.setCentralWidget(presenter.getMapWidget());
    mapWindow.setWindowTitle("Карта России");
    mapWindow.resize(800, 600);

    presenter.getPlayerWindow()->setWindowTitle("Игровое окно");
    presenter.getPlayerWindow()->resize(1000, 800);


    mapWindow.show();
    presenter.getPlayerWindow()->show();

    return app.exec();
}
