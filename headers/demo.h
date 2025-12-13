#pragma once

#include <QWidget>
#include <QPainter>
#include <QPolygonF>
#include <QPen>
#include "list.h"
#include "subject.h"



class MapWidget : public QWidget {
    Q_OBJECT
    Map* map;
    static const int TOP_MARGIN;
public:
    explicit MapWidget(Map* m, QWidget* parent = nullptr) : QWidget(parent), map(m) {}

protected:
    void paintEvent(QPaintEvent*) override;
};
