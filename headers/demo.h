#pragma once
#include <QWidget>
#include <QPainter>
#include <QPolygonF>
#include <QMap>
#include <QList>
#include <QThread>
#include <QTimer>
#include <QWheelEvent>
#include <QMouseEvent>
#include "subject.h"
#include "map_worker.h"

class MapWidget : public QWidget {
    Q_OBJECT
    Map* map;
    QMap<AbstractSubject*, CachedSubject*> cache;
    int widgetWidth;
    int widgetHeight;
    
    // Многопоточность
    QThread* workerThread_;
    MapWorker* mapWorker_;
    QTimer* rebuildTimer_;

    // === Зум и панорамирование ===
    double zoomFactor_;      // Коэффициент масштабирования (1.0 = по размеру окна)
    double panOffsetX_;      // Дополнительный сдвиг по X (в экранных пикселях)
    double panOffsetY_;      // Дополнительный сдвиг по Y
    bool isPanning_;         // Флаг: идёт ли сейчас перетаскивание
    QPoint lastPanPoint_;    // Последняя позиция мыши при перетаскивании

public:
    explicit MapWidget(Map* m, QWidget* parent = nullptr);
    ~MapWidget() override;
    void requestRebuildCache();
    void clearCache();
    void resetView(); // Сброс зума и панорамы к исходному состоянию

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    
    // Обработчики зума и панорамирования
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private slots:
    void onCacheReady(const MapCachePayload& payload);
};