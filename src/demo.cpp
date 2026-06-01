#include "demo.h"
#include <algorithm>
#include <cmath>

MapWidget::MapWidget(Map* m, QWidget* parent)
    : QWidget(parent), map(m), widgetWidth(0), widgetHeight(0)
{
    setMinimumSize(400, 300);
}

MapWidget::~MapWidget() {
    qDeleteAll(cache);
    cache.clear();
}

void MapWidget::rebuildCache() {
    qDeleteAll(cache);
    cache.clear();

    double minX = 1e100, maxX = -1e100;
    double minY = 1e100, maxY = -1e100;
    bool hasData = false;

    const auto& subjects = map->get_subjects();
    for (auto* subj : subjects) {
        auto* cached = new CachedSubject();
        cached->visited = subj->is_visited();

        // get_border() возвращает список полигонов (мультполигон)
        const auto& borders = subj->get_border();
        for (auto* poly : borders) {
            QPolygonF qpoly;
            
            // Собираем точки одного контура (острова/части региона)
            for (const auto& c : *poly) {
                double lon = c.x;
                double lat = c.y;

                // Нормализация в [0, 360] для корректной отрисовки 180-го меридиана
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

            // Явно замыкаем контур. Это критично для Antialiasing в Qt.
            if (qpoly.size() >= 3) {
                qpoly << qpoly.front(); 
                cached->polygons.append(qpoly);
            }
        }
        cache.insert(subj, cached);
    }

    if (!hasData) return;

    // Отступы 5%
    double marginLon = (maxX - minX) * 0.05;
    double marginLat = (maxY - minY) * 0.05;
    minX -= marginLon; maxX += marginLon;
    minY -= marginLat; maxY += marginLat;

    double mapWidth = maxX - minX;
    double mapHeight = maxY - minY;
    if (mapWidth <= 0 || mapHeight <= 0) return;

    const int topMargin = 40;
    double scaleX = widgetWidth / mapWidth;
    double scaleY = (widgetHeight - topMargin) / mapHeight;
    double scale = std::min(scaleX, scaleY);
    
    double offsetX = (widgetWidth - mapWidth * scale) / 2.0;
    double offsetY = (widgetHeight - topMargin - mapHeight * scale) / 2.0;

    // Применяем масштаб к каждому полигону
    for (auto it = cache.begin(); it != cache.end(); ++it) {
        auto* cached = it.value();
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
}

void MapWidget::clearCache() {
    qDeleteAll(cache);
    cache.clear();
    update();
}

void MapWidget::resizeEvent(QResizeEvent* event) {
    widgetWidth = width();
    widgetHeight = height();
    rebuildCache();
    QWidget::resizeEvent(event);
}

void MapWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), Qt::white);

    const int topMargin = 40;
    QFont font = painter.font();
    font.setPointSize(16);
    painter.setFont(font);
    painter.setPen(Qt::black);
    painter.drawText(0, 0, width(), topMargin, Qt::AlignCenter, "Карта России");
    painter.translate(0, topMargin);

    // 🔹 ПРОХОД 1: ЗАЛИВКА (без обводки)
    // Qt не будет рисовать границы, поэтому острова не соединятся линиями
    for (auto it = cache.begin(); it != cache.end(); ++it) {
        auto* cached = it.value();
        if (cached->visited) {
            painter.setBrush(QColor(0, 180, 0, 160));
            painter.setPen(Qt::NoPen); 
            for (const auto& poly : cached->polygons) {
                painter.drawPolygon(poly, Qt::WindingFill);
            }
        }
    }

    //  ПРОХОД 2: ОБВОДКА (без заливки)
    // Рисуем чёткие контуры поверх заливки
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(Qt::black, 1));
    for (auto it = cache.begin(); it != cache.end(); ++it) {
        auto* cached = it.value();
        for (const auto& poly : cached->polygons) {
            painter.drawPolygon(poly, Qt::WindingFill);
        }
    }
}