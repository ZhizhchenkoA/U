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
    
    // Multithread
    QThread* workerThread_;
    MapWorker* mapWorker_;
    QTimer* rebuildTimer_;

    // changing the scale or moving map
    double zoomFactor_;   
    double panOffsetX_;    
    double panOffsetY_;      
    bool isPanning_;         
    QPoint lastPanPoint_;   

public:
    explicit MapWidget(Map* m, QWidget* parent = nullptr);
    ~MapWidget() override;
    void requestRebuildCache();
    void clearCache();
    
    void resetView(); 

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    
    // Zoom and changing scale
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private slots:
    void onCacheReady(const MapCachePayload& payload);
};