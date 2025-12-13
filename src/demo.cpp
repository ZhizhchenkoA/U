#include "demo.h"

void MapWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int w = width();
    int h = height();
    const int TOP_MARGIN = 50;

    // Заголовок
    QFont titleFont;
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    painter.setFont(titleFont);

    QString title = "Карта России";
    QFontMetrics fm(titleFont);
    int titleWidth = fm.horizontalAdvance(title);

    painter.setPen(Qt::black);
    painter.drawText((w - titleWidth) / 2, fm.ascent() + 10, title);

    // Перо 
    QPen pen(Qt::black);
    pen.setWidth(2);
    painter.setPen(pen);

    int mapTop = TOP_MARGIN;
    int mapHeight = h - mapTop;

    // Регионы
    List<AbstractSubject*>& subjects = map->get_subjects();
    List<AbstractSubject*>::Iterator<AbstractSubject*> it = subjects.iter();

    while (!it.isEnd()) {
        AbstractSubject* subj = it.next();
        List<Coordinates>& border = subj->get_border();
        List<Coordinates>::Iterator<Coordinates> itc = border.iter();

        QPolygonF polygon;
        while (!itc.isEnd()) {
            Coordinates c = itc.next();
            double x = c.x * w;
            double y = c.y * mapHeight + mapTop;
            polygon << QPointF(x, y);
        }

        // Закрашивание по флагу is_visited 
        if (subj->is_visited()) {
            painter.setBrush(QColor(180, 220, 255));  // посещён
        } else {
            painter.setBrush(Qt::NoBrush);            // не посещён
        }

        painter.drawPolygon(polygon);
    }
}
