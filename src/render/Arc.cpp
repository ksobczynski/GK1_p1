#include "Arc.h"
#include <algorithm>

#include <cmath>
#include <optional>
#include <vector>

namespace render {
namespace {
constexpr double epsilon = 1e-9;
constexpr double pi = 3.14159265358979323846;

double norm(QPointF v) { return std::hypot(v.x(), v.y()); }

QPointF rotate90(QPointF v) { return QPointF(-v.y(), v.x()); }

std::vector<QPointF> sampleArc(QPointF center, QPointF a, QPointF b,
                               bool increasing) {
  const double radius = norm(a - center);
  const double from = std::atan2(a.y() - center.y(), a.x() - center.x());
  const double to = std::atan2(b.y() - center.y(), b.x() - center.x());
  double sweep = to - from;
  if (increasing)
    while (sweep <= 0)
      sweep += 2 * pi;
  else {
    while (sweep >= 0)
      sweep -= 2 * pi;
  }

  const int n =
      std::clamp(static_cast<int>(radius * std::abs(sweep) / 3), 8, 1000);
  std::vector<QPointF> points;
  points.reserve(n + 1);
  for (int i = 0; i <= n; i++) {
    const double angle = from + sweep * i / n;
    points.push_back(center +
                     radius * QPointF(std::cos(angle), std::sin(angle)));
  }
  points.front() = a;
  points.back() = b;
  return points;
}

std::vector<QPointF> tangentArc(QPointF a, QPointF b, QPointF tangent) {
  const QPointF ab = b - a;
  const QPointF normal = rotate90(tangent / norm(tangent));
  const double denominator = 2 * QPointF::dotProduct(normal, ab);
  if (std::abs(denominator) < epsilon)
    return {a, b};
  const QPointF center =
      a + normal * (QPointF::dotProduct(ab, ab) / denominator);
  const bool increasing =
      QPointF::dotProduct(tangent, rotate90(a - center)) > 0;
  return sampleArc(center, a, b, increasing);
}
} // namespace

std::vector<QPointF> arcPoints(QPointF a, QPointF b,
                               std::optional<QPointF> startTangent,
                               std::optional<QPointF> endTangent) {
  if (startTangent && norm(*startTangent) > epsilon) {
    return tangentArc(a, b, *startTangent);
  }
  if (endTangent && norm(*endTangent) > epsilon) {
    std::vector<QPointF> points = tangentArc(b, a, -*endTangent);
    std::reverse(points.begin(), points.end());
    return points;
  }

  return sampleArc((a + b) / 2, a, b, true);
}

void drawArc(QPainter &painter, QPointF a, QPointF b,
             std::optional<QPointF> startTangent,
             std::optional<QPointF> endTangent) {
  const std::vector<QPointF> points = arcPoints(a, b, startTangent, endTangent);
  painter.drawPolyline(points.data(), static_cast<int>(points.size()));
}
} // namespace render