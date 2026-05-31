#include "demo.h"
#include <algorithm>
#include <vector>

MapWidget::MapWidget(Map* m, QWidget* parent)
    : QWidget(parent), map(m), widgetWidth(0), widgetHeight(0)
{
    setMinimumSize(400, 300);
}

MapWidget::~MapWidget()
{
    qDeleteAll(cache);
    cache.clear();
}

double MapWidget::normalizeLongitude360(double lon) {
    while (lon < 0) lon += 360;
    while (lon >= 360) lon -= 360;
    return lon;
}

void MapWidget::rebuildCache()
{
    qDeleteAll(cache);
    cache.clear();

    auto& subjects = map->get_subjects();  // std::list<AbstractSubject*>&
    std::vector<double> longitudes;
    double minY =  1e100;
    double maxY = -1e100;

    // Собираем bounding box
    for (AbstractSubject* subj : subjects) {
        auto& borders = subj->get_border();  // std::list<Polygon*>&
        for (Polygon* poly : borders) {
            for (const auto& c : *poly) {    // Polygon = std::list<Coordinates>
                double normLon = normalizeLongitude360(c.x);
                longitudes.push_back(normLon);
                if (c.y < minY) minY = c.y;
                if (c.y > maxY) maxY = c.y;
            }
        }
    }

    if (longitudes.empty() || minY >= maxY)
        return;

    double minX = *std::min_element(longitudes.begin(), longitudes.end());
    double maxX = *std::max_element(longitudes.begin(), longitudes.end());

    // Коррекция долготы для перехода через 180 меридиан
    if (maxX - minX > 180) {
        for (auto& lon : longitudes) {
            if (lon > 180) lon -= 360;
        }
        minX = *std::min_element(longitudes.begin(), longitudes.end());
        maxX = *std::max_element(longitudes.begin(), longitudes.end());
    }

    double mapWidth  = maxX - minX;
    double mapHeight = maxY - minY;
    if (mapWidth <= 0 || mapHeight <= 0) return;

    const int topMargin = 40;
    double scaleX = widgetWidth / mapWidth;
    double scaleY = (widgetHeight - topMargin) / mapHeight;
    double scale = (scaleX < scaleY) ? scaleX : scaleY;
    double offsetX = (widgetWidth  - mapWidth  * scale) / 2.0;
    double offsetY = (widgetHeight - topMargin - mapHeight * scale) / 2.0;

    // Заполняем кэш полигонами
    for (AbstractSubject* subj : subjects) {
        auto* cached = new CachedSubject();
        cached->visited = subj->is_visited();

        auto& borders = subj->get_border();
        for (Polygon* poly : borders) {
            QPolygonF qpoly;
            for (const auto& c : *poly) {
                double normLon = normalizeLongitude360(c.x);
                if (maxX - minX > 180 && normLon > 180) normLon -= 360;

                double nx = (normLon - minX) * scale + offsetX;
                double ny = (maxY - c.y) * scale + offsetY;
                qpoly << QPointF(nx, ny);
            }
            cached->polygons.push_back(qpoly);
        }
        cache.insert(subj, cached);
    }
}

void MapWidget::clearCache()
{
    qDeleteAll(cache);
    cache.clear();
    update();
}

void MapWidget::resizeEvent(QResizeEvent* event)
{
    widgetWidth = width();
    widgetHeight = height();
    rebuildCache();
    QWidget::resizeEvent(event);
}

void MapWidget::paintEvent(QPaintEvent*)
{
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

    QPen pen(Qt::black);
    pen.setWidth(1);
    painter.setPen(pen);

    // Отрисовка кэшированных регионов
    for (auto it = cache.begin(); it != cache.end(); ++it) {
        auto* cached = it.value();
        painter.setBrush(cached->visited ? QColor(0, 180, 0, 160) : Qt::NoBrush);
        for (const auto& poly : cached->polygons) {
            painter.drawPolygon(poly);
        }
    }
}