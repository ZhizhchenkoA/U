#include "demo.h"


MapWidget::MapWidget(Map* m, QWidget* parent)
    : QWidget(parent), map(m)
{
    setMinimumSize(400, 300);
}

void MapWidget::rebuildCache()
{
    cache.clear();

    List<AbstractSubject*>& subjects = map->get_subjects();
    auto it = subjects.iter();

    while (!it.isEnd()) {
        AbstractSubject* subj = it.next();
        CachedSubject cached;

        cached.visited = subj->is_visited();

        QPolygonF polygon;
        List<Coordinates>& border = subj->get_border();
        auto coordIter = border.iter();

        while (!coordIter.isEnd()) {
            Coordinates c = coordIter.next();
            qreal x = c.x * widgetWidth;
            qreal y = c.y * widgetHeight;
            polygon << QPointF(x, y);
        }

        cached.polygon = polygon;
        cache.insert(subj, cached);
    }
}

void MapWidget::resizeEvent(QResizeEvent* event)
{
    widgetWidth = width();
    widgetHeight = height();
    rebuildCache();
    QWidget::resizeEvent(event);
}

void MapWidget::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPen pen(Qt::black);
    pen.setWidth(2);
    painter.setPen(pen);

    const int topMargin = 40;

    painter.fillRect(rect(), Qt::white);

    QFont font = painter.font();
    font.setPointSize(16);
    painter.setFont(font);
    painter.setPen(Qt::black);
    painter.drawText(rect().adjusted(0, 0, 0, -height() + topMargin), Qt::AlignCenter, "Карта России");

    painter.translate(0, topMargin);

    for (auto it = cache.begin(); it != cache.end(); ++it) {
        if (it.value().visited)
            painter.setBrush(Qt::green);
        else
            painter.setBrush(Qt::NoBrush);

        painter.drawPolygon(it.value().polygon);
    }
}
