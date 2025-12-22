#pragma once

#include <QObject>

#include "demo.h"
#include "player_window.h"
#include "subject.h"

class Presenter : public QObject {
    Q_OBJECT
public:
    explicit Presenter(QObject* parent = nullptr);
    ~Presenter();

    MapWidget* getMapWidget() const;
    PlayerWindow* getPlayerWindow() const;

private Q_SLOTS:
    void onRegionVisited(const QString& regionName);
    void onGameFinished(); 
    
private:
    Map map;
    MapWidget* mapWidget;
    PlayerWindow* playerWindow;
};
