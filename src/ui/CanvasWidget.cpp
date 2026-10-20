#include "CanvasWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <algorithm>

CanvasWidget::CanvasWidget(model::Scene &scene, QWidget *parent)
    : QWidget(parent), scene_(scene)
{
    setWindowTitle("Edytor wielokatow");
    resize(900, 600);
}

void CanvasWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(rect(), Qt::white);

    for (const model::Polygon &polygon : scene_.polygons())
    {
        painter.setPen(QPen(Qt::darkBlue, 2));
        for (const model::Edge &edge : polygon.edges())
        {
            const QPointF a = polygon.findVertex(edge.start)->position;
            const QPointF b = polygon.findVertex(edge.end)->position;
            painter.drawLine(a, b);
        }

        painter.setPen(Qt::black);
        painter.setBrush(Qt::white);
        for (const model::Vertex &vertex : polygon.vertices())
            painter.drawEllipse(vertex.position, 5, 5);
    }
}

void CanvasWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        dragState_ = hitTest(event->position());
    }
}

std::optional<CanvasWidget::DragState> CanvasWidget::hitTest(QPointF position) const
{
    constexpr double hitRadius = 8.0;
    const std::vector<model::Polygon> &polygons = scene_.polygons();

    for (const model::Polygon &polygon : polygons)
    {
        for (const model::Vertex &vertex : polygon.vertices())
        {
            const QPointF d = vertex.position - position;
            if (QPointF::dotProduct(d, d) <= hitRadius * hitRadius)
                return DragState{polygon.id(), vertex.id, position};
        }
    }

    for (auto it = polygons.rbegin(); it != polygons.rend(); ++it)
        if (it->convexHullContains(position))
            return DragState{it->id(), std::nullopt, position};

    return std::nullopt;
}

void CanvasWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (dragState_)
    {
        model::Polygon *polygon = scene_.findPolygon(dragState_->polygon);
        if (polygon)
        {
            const QPointF position = event->position();
            if (dragState_->vertex)
            {
                polygon->moveVertex(*dragState_->vertex, position);
            }
            else
            {
                const QPointF delta = position - dragState_->lastPosition;
                polygon->translate(delta.x(), delta.y());
            }
            dragState_->lastPosition = position;
            update();
        }
    }
}
void CanvasWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        dragState_.reset();
    }
}

void CanvasWidget::contextMenuEvent(QContextMenuEvent *event)
{
    const std::optional<DragState> hit = hitTest(event->pos());
    if (hit)
    {
        if (hit->vertex)
        {
            model::Polygon *polygon = scene_.findPolygon(hit->polygon);
            if (!polygon)
                return;
            QMenu menu(this);

            QAction *removeVertexAction = menu.addAction("Usun wierzcholek");
            removeVertexAction->setEnabled(polygon->vertices().size() > 3);

            if (menu.exec(event->globalPos()) == removeVertexAction)
            {
                polygon->removeVertex(*hit->vertex);
                dragState_.reset();
                update();
            }
        }
    }
}

namespace
{
    double squaredDistanceToSegment(QPointF p, QPointF a, QPointF b)
    {
        const QPointF ab = b - a;
        const double lengthSquared = QPointF::dotProduct(ab, ab);
        double t = 0.0;

        if (lengthSquared > 0)
        {
            t = std::clamp(QPointF::dotProduct(p - a, ab) / lengthSquared, 0.0, 1.0);
        }

        const QPointF d = p - (a + t * ab);
        return QPointF::dotProduct(d, d);
    }

}
std::optional<CanvasWidget::EdgeHit> CanvasWidget::hitTestEdge(QPointF position) const
{
    constexpr double hitDistance = 5.0;
    for (const model::Polygon &polygon : scene_.polygons())
    {
        for (const model::Edge &edge : polygon.edges())
        {
            const QPointF a = polygon.findVertex(edge.start)->position;
            const QPointF b = polygon.findVertex(edge.end)->position;
            if (squaredDistanceToSegment(position, a, b) <= hitDistance * hitDistance)
                return EdgeHit{polygon.id(), edge.id};
        }
    }
    return std::nullopt;
}

void CanvasWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        const QPointF position = event->position();

        const std::optional<EdgeHit> edgeHit = hitTestEdge(position);
        if (!edgeHit)
            return;

        model::Polygon *polygon = scene_.findPolygon(edgeHit->polygon);
        if (polygon && polygon->insertVertexAtEdgeMidpoint(edgeHit->edge))
        {
            dragState_.reset();
            update();
        }
    }
}
