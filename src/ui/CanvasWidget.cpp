#include "CanvasWidget.h"

#include <QPainter>

CanvasWidget::CanvasWidget(model::Scene& scene, QWidget* parent)
    : QWidget(parent), scene_(scene)
{
    setWindowTitle("Edytor wielokatow");
    resize(900, 600);
}

void CanvasWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.fillRect(rect(), Qt::white);

    for (const model::Polygon& polygon : scene_.polygons()) {
        painter.setPen(QPen(Qt::darkBlue, 2));
        for (const model::Edge& edge : polygon.edges()) {
            const model::Point a = polygon.findVertex(edge.start)->position;
            const model::Point b = polygon.findVertex(edge.end)->position;
            painter.drawLine(QPointF(a.x, a.y), QPointF(b.x, b.y));
        }

        painter.setPen(Qt::black);
        painter.setBrush(Qt::white);
        for (const model::Vertex& vertex : polygon.vertices())
            painter.drawEllipse(QPointF(vertex.position.x, vertex.position.y), 5, 5);
    }
}
