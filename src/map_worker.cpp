#include "map_worker.h"
#include <algorithm>
#include <cmath>
#include <QDebug>

MapWorker::MapWorker(QObject* parent) : QObject(parent) {}
MapWorker::~MapWorker() = default;

void MapWorker::processCache(Map* map, int widgetWidth, int widgetHeight) {
    qDebug() << "[MapWorker] processCache started. Size:" << widgetWidth << "x" << widgetHeight;
    
    if (widgetWidth <= 0 || widgetHeight <= 40) {
        qDebug() << "[MapWorker] Aborted: Invalid widget size";
        return;
    }

    MapCachePayload payload;
    payload.width = widgetWidth;
    payload.height = widgetHeight;

    double minX = 1e100, maxX = -1e100;
    double minY = 1e100, maxY = -1e100;
    bool hasData = false;

    const std::list<AbstractSubject*>& subjects = map->get_subjects();
    if (subjects.empty()) {
        qDebug() << "[MapWorker] Warning: Subject list is empty!";
        emit cacheReady(payload);
        return;
    }

    for (std::list<AbstractSubject*>::const_iterator it = subjects.begin(); it != subjects.end(); ++it) {
        AbstractSubject* subj = *it;
        CachedSubject* cached = new CachedSubject();
        cached->visited = subj->is_visited();
        const std::list<Polygon*>& borders = subj->get_border();
        
        for (std::list<Polygon*>::const_iterator pit = borders.begin(); pit != borders.end(); ++pit) {
            Polygon* poly = *pit;
            QPolygonF qpoly;
            for (std::list<Coordinates>::const_iterator cit = poly->begin(); cit != poly->end(); ++cit) {
                double lon = cit->x;
                double lat = cit->y;
                
                while (lon < 0) lon += 360.0;
                while (lon >= 360) lon -= 360.0;
                
                qpoly << QPointF(lon, lat);
                if (!hasData) {
                    minX = maxX = lon;
                    minY = maxY = lat;
                    hasData = true;
                } else {
                    if (lon < minX) minX = lon;
                    if (lon > maxX) maxX = lon;
                    if (lat < minY) minY = lat;
                    if (lat > maxY) maxY = lat;
                }
            }
            if (qpoly.size() >= 3) {
                qpoly << qpoly.front();
                cached->polygons.append(qpoly);
            }
        }
        payload.cache.insert(subj, cached);
    }

    if (!hasData) {
        qDebug() << "[MapWorker] No valid geometry data found.";
        emit cacheReady(payload);
        return;
    }

    double marginLon = (maxX - minX) * 0.05;
    double marginLat = (maxY - minY) * 0.05;
    minX -= marginLon; maxX += marginLon;
    minY -= marginLat; maxY += marginLat;

    double mapWidth = maxX - minX;
    double mapHeight = maxY - minY;
    if (mapWidth <= 0 || mapHeight <= 0) {
        emit cacheReady(payload);
        return;
    }

    const int topMargin = 40;
    double scaleX = static_cast<double>(widgetWidth) / mapWidth;
    double scaleY = static_cast<double>(widgetHeight - topMargin) / mapHeight;
    double scale = std::min(scaleX, scaleY);
    double offsetX = (widgetWidth - mapWidth * scale) / 2.0;
    double offsetY = (widgetHeight - topMargin - mapHeight * scale) / 2.0;

    for (QMap<AbstractSubject*, CachedSubject*>::iterator it = payload.cache.begin(); it != payload.cache.end(); ++it) {
        CachedSubject* cached = it.value();
        for (int i = 0; i < cached->polygons.size(); ++i) {
            QPolygonF& poly = cached->polygons[i];
            for (int j = 0; j < poly.size(); ++j) {
                double x = poly[j].x();
                double y = poly[j].y();
                poly[j].setX((x - minX) * scale + offsetX);
                poly[j].setY((maxY - y) * scale + offsetY);
            }
        }
    }

    qDebug() << "[MapWorker] Cache processed successfully. Emitting signal...";
    emit cacheReady(payload);
}