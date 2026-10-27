#pragma once

#include <QPainter>
#include <optional>
#include <vector>

namespace render {
std::vector<QPointF> arcPoints(QPointF a, QPointF b,
                               std::optional<QPointF> startTangent,
                               std::optional<QPointF> endTangent);

void drawArc(QPainter &painter, QPointF a, QPointF b,
             std::optional<QPointF> startTangent,
             std::optional<QPointF> endTangent);
} // namespace render