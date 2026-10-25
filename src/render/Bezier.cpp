#include "Bezier.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace render {
namespace {
double distance(QPointF a, QPointF b) {
  return std::hypot(b.x() - a.x(), b.y() - a.y());
}
} // namespace

std::vector<QPointF> bezierPoints(QPointF p0, QPointF p1, QPointF p2,
                                  QPointF p3) {
  const QPointF a = -p0 + 3 * p1 - 3 * p2 + p3;
  const QPointF b = 3 * p0 - 6 * p1 + 3 * p2;
  const QPointF c = -3 * p0 + 3 * p1;
  const QPointF d = p0;

  const double controlLength =
      distance(p0, p1) + distance(p1, p2) + distance(p2, p3);
  const int n = std::clamp(static_cast<int>(controlLength / 3), 8, 1000);
  const double h = 1.0 / n;

  QPointF point = d;
  QPointF delta1 = a * (h * h * h) + b * (h * h) + c * h;
  QPointF delta2 = a * (6 * h * h * h) + b * (2 * h * h);
  const QPointF delta3 = a * (6 * h * h * h);

  std::vector<QPointF> points;
  points.reserve(n + 1);
  points.push_back(point);
  for (int i = 0; i < n; i++) {
    point += delta1;
    delta1 += delta2;
    delta2 += delta3;
    points.push_back(point);
  }
  points.back() = p3;
  return points;
}

void drawBezier(QPainter &painter, QPointF p0, QPointF p1, QPointF p2,
                QPointF p3) {
  const std::vector<QPointF> points = bezierPoints(p0, p1, p2, p3);
  painter.drawPolyline(points.data(), static_cast<int>(points.size()));
}
} // namespace render