#include "presenter.h"

Presenter::Presenter(QObject* parent)
    : QObject(parent),
      mapWidget(nullptr),
      playerWindow(nullptr)
{
    map.get_from_JSON("data/new_russia (1).geojson", "data/russia_neighbours.json");

    mapWidget = new MapWidget(&map);
    playerWindow = new PlayerWindow(&map);
    playerWindow->initGame();

    mapWidget->rebuildCache();

   connect(playerWindow, &PlayerWindow::regionVisited,
       this, &Presenter::onRegionVisited);

    connect(playerWindow, &PlayerWindow::gameFinished,
        this, &Presenter::onGameFinished);
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

void Presenter::onGameFinished()
{

    List<AbstractSubject*>& subjects = map.get_subjects();
    for (int i = 0; i < subjects.size(); ++i) {
        subjects.Get(i)->unvisit();
    }

    mapWidget->clearCache();
    mapWidget->rebuildCache();
}