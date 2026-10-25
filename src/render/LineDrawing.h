#pragma once

#include <QPainter>

namespace render {
enum class LineAlgorithm { Library, Bresenham };

void drawSegment(QPainter &painter, QPointF a, QPointF b,
                 LineAlgorithm algorithm);
} // namespace render