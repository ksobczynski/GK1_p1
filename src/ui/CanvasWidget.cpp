#include "CanvasWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <algorithm>
#include <cmath>
#include <QInputDialog>

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

    drawConstraintLabels(painter);
}

void CanvasWidget::drawConstraintLabels(QPainter &painter) const
{
    painter.setPen(Qt::darkRed);
    for (const model::Constraint &constraint : scene_.constraints())
    {
        if (constraint.type != model::ConstraintType::Length)
            continue;

        const model::Polygon *polygon = scene_.findPolygon(constraint.first.polygon);
        if (!polygon)
            continue;
        const model::Edge *edge = polygon->findEdge(constraint.first.edge);
        if (!edge)
            continue;

        const QPointF a = polygon->findVertex(edge->start)->position;
        const QPointF b = polygon->findVertex(edge->end)->position;
        const QPointF ab = b - a;
        const double length = std::hypot(ab.x(), ab.y());
        if (length == 0)
            continue;

        constexpr double labelOffset = 14.0;
        const QPointF normal(-ab.y() / length, ab.x() / length);
        const QPointF center = (a + b) / 2 + labelOffset * normal;

        const QRectF box(center - QPointF(30, 9), QSizeF(60, 18));
        painter.drawText(box, Qt::AlignCenter, QString::number(constraint.length, 'f', 1));
    }
}

namespace
{
    // Kwadrat odleglosci punktu p od odcinka ab.
    double squaredDistanceToSegment(QPointF p, QPointF a, QPointF b)
    {
        const QPointF ab = b - a;
        const double lengthSquared = QPointF::dotProduct(ab, ab);
        double t = 0.0;
        if (lengthSquared > 0)
            t = std::clamp(QPointF::dotProduct(p - a, ab) / lengthSquared, 0.0, 1.0);
        const QPointF d = p - (a + t * ab);
        return QPointF::dotProduct(d, d);
    }
} // namespace

std::optional<CanvasWidget::Hit> CanvasWidget::hitTest(QPointF position) const
{
    constexpr double vertexRadius = 8.0;
    constexpr double edgeDistance = 5.0;
    const std::vector<model::Polygon> &polygons = scene_.polygons();

    for (const model::Polygon &polygon : polygons)
    {
        for (const model::Vertex &vertex : polygon.vertices())
        {
            const QPointF d = vertex.position - position;
            if (QPointF::dotProduct(d, d) <= vertexRadius * vertexRadius)
                return Hit{Hit::Kind::Vertex, polygon.id(), vertex.id, 0};
        }
    }

    for (const model::Polygon &polygon : polygons)
    {
        for (const model::Edge &edge : polygon.edges())
        {
            const QPointF a = polygon.findVertex(edge.start)->position;
            const QPointF b = polygon.findVertex(edge.end)->position;
            if (squaredDistanceToSegment(position, a, b) <= edgeDistance * edgeDistance)
                return Hit{Hit::Kind::Edge, polygon.id(), 0, edge.id};
        }
    }

    for (auto it = polygons.rbegin(); it != polygons.rend(); ++it)
        if (it->convexHullContains(position))
            return Hit{Hit::Kind::Interior, it->id(), 0, 0};

    return std::nullopt;
}

void CanvasWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    const QPointF position = event->position();
    if (const std::optional<Hit> hit = hitTest(position))
        dragState_ = DragState{*hit, position};
    else
        dragState_.reset();
}

void CanvasWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!dragState_)
        return;
    model::Polygon *polygon = scene_.findPolygon(dragState_->target.polygon);
    if (!polygon)
        return;

    const QPointF position = event->position();
    switch (dragState_->target.kind)
    {
    case Hit::Kind::Vertex:
        polygon->moveVertex(dragState_->target.vertex, position);
        break;
    case Hit::Kind::Edge: // chwycenie krawedzi przesuwa caly wielokat
    case Hit::Kind::Interior:
    {
        const QPointF delta = position - dragState_->lastPosition;
        polygon->translate(delta.x(), delta.y());
        break;
    }
    }
    dragState_->lastPosition = position;
    update();
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        dragState_.reset();
}

void CanvasWidget::contextMenuEvent(QContextMenuEvent *event)
{
    const std::optional<Hit> hit = hitTest(event->pos());
    if (!hit)
        return;
    model::Polygon *polygon = scene_.findPolygon(hit->polygon);
    if (!polygon)
        return;

    QMenu menu(this);

    if (hit->kind == Hit::Kind::Vertex)
    {
        QAction *removeVertexAction = menu.addAction("Usun wierzcholek");
        removeVertexAction->setEnabled(polygon->vertices().size() > 3);

        if (menu.exec(event->globalPos()) == removeVertexAction)
        {
            polygon->removeVertex(hit->vertex);
            dragState_.reset();
            update();
        }
    }
    else if (hit->kind == Hit::Kind::Edge)
    {
        QAction *lengthAction = menu.addAction("Ustaw dlugosc...");

        if (menu.exec(event->globalPos()) == lengthAction)
        {
            const model::Edge *edge = polygon->findEdge(hit->edge);
            const QPointF a = polygon->findVertex(edge->start)->position;
            const QPointF b = polygon->findVertex(edge->end)->position;
            const double currentLength = std::hypot(b.x() - a.x(), b.y() - a.y());

            bool ok = false;
            const double length = QInputDialog::getDouble(
                this, "Dlugosc krawedzi", "Dlugosc:", currentLength, 1.0, 10000.0, 1, &ok);

            if (ok)
            {
                scene_.setLengthConstraint({hit->polygon, hit->edge}, length);
                (void)length;
                update();
            }
        }
    }
}

void CanvasWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    const std::optional<Hit> hit = hitTest(event->position());
    if (!hit || hit->kind != Hit::Kind::Edge)
        return;

    model::Polygon *polygon = scene_.findPolygon(hit->polygon);
    if (polygon && polygon->insertVertexAtEdgeMidpoint(hit->edge))
    {
        dragState_.reset();
        update();
    }
}
