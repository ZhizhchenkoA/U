#pragma once
#include <QObject>
#include <QMap>
#include <QList>
#include <QPolygonF>
#include <QMetaType>
#include "subject.h"

// Структура кэшированного субъекта для отрисовки
struct CachedSubject {
    QList<QPolygonF> polygons;
    bool visited = false;
};

// Структура для передачи данных между потоками
struct MapCachePayload {
    QMap<AbstractSubject*, CachedSubject*> cache;
    int width;
    int height;
};
Q_DECLARE_METATYPE(MapCachePayload)

class MapWorker : public QObject {
    Q_OBJECT
public:
    explicit MapWorker(QObject* parent = nullptr);
    ~MapWorker() override;

public slots:
    void processCache(Map* map, int widgetWidth, int widgetHeight);

signals:
    void cacheReady(const MapCachePayload& payload);
};