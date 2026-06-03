#include "demo.h"
#include <QThread>
#include <QTimer>
#include <algorithm>
#include <cmath>
#include <QDebug>

MapWidget::MapWidget(Map* m, QWidget* parent)
    : QWidget(parent), map(m), widgetWidth(0), widgetHeight(0),
      zoomFactor_(1.0), panOffsetX_(0.0), panOffsetY_(0.0),
      isPanning_(false)
{
    setMinimumSize(400, 300);
    setMouseTracking(true); // Нужно для панорамирования
    
    workerThread_ = new QThread(this);
    mapWorker_ = new MapWorker();
    mapWorker_->moveToThread(workerThread_);
    workerThread_->start();

    rebuildTimer_ = new QTimer(this);
    rebuildTimer_->setSingleShot(true);
    rebuildTimer_->setInterval(150);

    connect(rebuildTimer_, &QTimer::timeout, this, [this]() {
        QMetaObject::invokeMethod(mapWorker_, "processCache", 
            Qt::QueuedConnection, 
            Q_ARG(Map*, map), 
            Q_ARG(int, widgetWidth), 
            Q_ARG(int, widgetHeight));
    });

    connect(mapWorker_, &MapWorker::cacheReady, this, &MapWidget::onCacheReady, Qt::QueuedConnection);
}

MapWidget::~MapWidget() {
    if (workerThread_->isRunning()) {
        workerThread_->quit();
        workerThread_->wait(1000);
    }
    qDeleteAll(cache);
    cache.clear();
}

void MapWidget::resetView() {
    zoomFactor_ = 1.0;
    panOffsetX_ = 0.0;
    panOffsetY_ = 0.0;
    update();
}

void MapWidget::requestRebuildCache() {
    widgetWidth = width();
    widgetHeight = height();
    if (widgetWidth <= 0 || widgetHeight <= 0) return;
    rebuildTimer_->start();
}

void MapWidget::onCacheReady(const MapCachePayload& payload) {
    qDeleteAll(cache);
    cache.clear();
    for (QMap<AbstractSubject*, CachedSubject*>::const_iterator it = payload.cache.begin(); 
         it != payload.cache.end(); ++it) {
        cache.insert(it.key(), it.value());
    }
    update();
}

void MapWidget::clearCache() {
    qDeleteAll(cache);
    cache.clear();
    update();
}

void MapWidget::resizeEvent(QResizeEvent* event) {
    requestRebuildCache();
    QWidget::resizeEvent(event);
}

void MapWidget::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    requestRebuildCache();
}

// === ЗУМ КОЛЕСОМ МЫШИ ===
void MapWidget::wheelEvent(QWheelEvent* event) {
    if (cache.isEmpty()) return;
    
    QPointF mousePos = event->position();
    double mx = mousePos.x();
    double my = mousePos.y() - 40; // Учитываем верхний отступ под заголовок

    // Запоминаем, какая точка ЭКРАНА находится под курсором
    // После зума мы хотим, чтобы эта же точка КАРТЫ осталась под курсором
    
    // Меняем зум
    double delta = event->angleDelta().y();
    double factor = (delta > 0) ? 1.15 : (1.0 / 1.15);
    double newZoom = zoomFactor_ * factor;
    
    // Ограничиваем диапазон зума
    if (newZoom < 0.2) newZoom = 0.2;
    if (newZoom > 20.0) newZoom = 20.0;
    
    // Корректируем panOffset так, чтобы точка под курсором осталась на месте
    // Формула: новая_точка = (старая_точка - panOffset) * (newZoom / oldZoom) + newPanOffset
    // Мы хотим: новая_точка == mx (или my)
    // => newPanOffset = mx - (mx - panOffsetX_) * (newZoom / zoomFactor_)
    double zoomRatio = newZoom / zoomFactor_;
    panOffsetX_ = mx - (mx - panOffsetX_) * zoomRatio;
    panOffsetY_ = my - (my - panOffsetY_) * zoomRatio;
    
    zoomFactor_ = newZoom;
    update(); // Просто перерисовываем, кэш не пересчитываем!
    event->accept();
}

// === ПАНОРАМИРОВАНИЕ ===
void MapWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton ||
        (event->button() == Qt::LeftButton && (event->modifiers() & Qt::ControlModifier))) {
        isPanning_ = true;
        lastPanPoint_ = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    } else {
        QWidget::mousePressEvent(event);
    }
}

void MapWidget::mouseMoveEvent(QMouseEvent* event) {
    if (isPanning_) {
        QPoint currentPos = event->pos();
        int dx = currentPos.x() - lastPanPoint_.x();
        int dy = currentPos.y() - lastPanPoint_.y();
        panOffsetX_ += dx;
        panOffsetY_ += dy;
        lastPanPoint_ = currentPos;
        update(); // Просто перерисовываем
        event->accept();
    } else {
        QWidget::mouseMoveEvent(event);
    }
}

void MapWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (isPanning_ && (event->button() == Qt::MiddleButton ||
                       event->button() == Qt::LeftButton)) {
        isPanning_ = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
    } else {
        QWidget::mouseReleaseEvent(event);
    }
}

void MapWidget::mouseDoubleClickEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        resetView();
        event->accept();
    } else {
        QWidget::mouseDoubleClickEvent(event);
    }
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
    
    painter.translate(panOffsetX_, panOffsetY_ + topMargin);
    painter.scale(zoomFactor_, zoomFactor_);
    
    painter.resetTransform();
    painter.setPen(QColor(100, 100, 100));
    QFont smallFont = painter.font();
    smallFont.setPointSize(10);
    painter.setFont(smallFont);
    QString zoomText = QString("Масштаб: x%1 | Ctrl+ЛКМ или СКМ — перетащить | Двойной клик — сброс")
                           .arg(zoomFactor_, 0, 'f', 2);
    painter.drawText(10, height() - 10, zoomText);
    
    painter.translate(panOffsetX_, panOffsetY_ + topMargin);
    painter.scale(zoomFactor_, zoomFactor_);

    for (QMap<AbstractSubject*, CachedSubject*>::const_iterator it = cache.begin(); 
         it != cache.end(); ++it) {
        CachedSubject* cached = it.value();
        if (cached->visited) {
            painter.setBrush(QColor(0, 180, 0, 160));
            painter.setPen(Qt::NoPen);
            for (int i = 0; i < cached->polygons.size(); ++i) {
                painter.drawPolygon(cached->polygons[i], Qt::WindingFill);
            }
        }
    }
    

    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(Qt::black, 1));
    for (QMap<AbstractSubject*, CachedSubject*>::const_iterator it = cache.begin(); 
         it != cache.end(); ++it) {
        CachedSubject* cached = it.value();
        for (int i = 0; i < cached->polygons.size(); ++i) {
            painter.drawPolygon(cached->polygons[i], Qt::WindingFill);
        }
    }
}