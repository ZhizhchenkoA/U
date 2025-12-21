#include "presenter.h"

Presenter::Presenter(QObject* parent)
    : QObject(parent),
      mapWidget(nullptr),
      playerWindow(nullptr)
{
    // Загружаем карту из файлов
    map.get_from_JSON("data/new_russia (1).geojson", "data/russia_neighbours.json");

    // Создаём виджеты
    mapWidget = new MapWidget(&map);
    playerWindow = new PlayerWindow(&map);

    // Инициализация игры, если требуется
    playerWindow->initGame();

    // Обновляем кэш карты
    mapWidget->rebuildCache();

    // Подключаем сигнал посещения региона к слоту обновления карты
   connect(playerWindow, &PlayerWindow::regionVisited,
       this, &Presenter::onRegionVisited);
}

Presenter::~Presenter()
{
    delete mapWidget;
    delete playerWindow;
}

MapWidget* Presenter::getMapWidget() const
{
    return mapWidget;
}

PlayerWindow* Presenter::getPlayerWindow() const
{
    return playerWindow;
}

void Presenter::onRegionVisited(const QString& regionName)
{
    Q_UNUSED(regionName);
    mapWidget->rebuildCache();
    mapWidget->update();
}
