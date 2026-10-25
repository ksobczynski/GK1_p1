#pragma once

#include <QPainter>
#include <vector>

namespace render {
std::vector<QPointF> bezierPoints(QPointF p0, QPointF p1, QPointF p2,
                                  QPointF p3);
void drawBezier(QPainter &painter, QPointF p0, QPointF p1, QPointF p2,
                QPointF p3);
} // namespace render