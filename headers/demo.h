#pragma once

#include <QPainter>
#include <QWidget>
#include <QPolygonF>
#include <QMap>
#include "list.h"
#include "subject.h"

struct CachedSubject {
    QPolygonF polygon;
    bool visited = false;
};

class MapWidget : public QWidget {
    Q_OBJECT

    Map* map;
    QMap<AbstractSubject*, CachedSubject> cache;

    int widgetWidth = 0;
    int widgetHeight = 0;

public:
    explicit MapWidget(Map* m, QWidget* parent = nullptr);

    void rebuildCache();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
};
