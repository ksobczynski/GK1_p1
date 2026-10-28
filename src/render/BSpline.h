#pragma once

#include <QPointF>
#include <array>
#include <vector>

namespace render {
using BezierSegment = std::array<QPointF, 4>;

std::vector<BezierSegment> bsplineToBezier(const std::vector<QPointF> &points,
                                           bool closed);
} // namespace render
