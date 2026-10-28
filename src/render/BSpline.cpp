#include "BSpline.h"

namespace render {
namespace {
std::vector<BezierSegment> closedToBezier(const std::vector<QPointF> &d) {
  const std::size_t n = d.size();
  const auto junction = [&](std::size_t i) {
    return (d[(i + n - 1) % n] + 4 * d[i] + d[(i + 1) % n]) / 6;
  };

  std::vector<BezierSegment> segments;
  segments.reserve(n);
  for (std::size_t i = 0; i < n; ++i) {
    const QPointF next = d[(i + 1) % n];
    segments.push_back({junction(i), (2 * d[i] + next) / 3,
                        (d[i] + 2 * next) / 3, junction((i + 1) % n)});
  }
  return segments;
}

std::vector<BezierSegment> openToBezier(const std::vector<QPointF> &p) {
  const std::size_t m = p.size();
  if (m == 3)
    return {{p[0], p[0] + 2 * (p[1] - p[0]) / 3, p[2] + 2 * (p[1] - p[2]) / 3,
             p[2]}};
  if (m == 4)
    return {{p[0], p[1], p[2], p[3]}};

  std::vector<QPointF> cuts;
  cuts.push_back(p[1]);
  for (std::size_t k = 1; k + 2 < m; ++k) {
    const QPointF a = p[k];
    const QPointF b = p[k + 1];
    if (k == 1 || k == m - 3) {
      cuts.push_back((a + b) / 2);
    } else {
      cuts.push_back(a + (b - a) / 3);
      cuts.push_back(a + 2 * (b - a) / 3);
    }
  }
  cuts.push_back(p[m - 2]);

  std::vector<BezierSegment> segments;
  QPointF start = p[0];
  for (std::size_t i = 0; i + 1 < cuts.size(); i += 2) {
    const bool last = i + 2 == cuts.size();
    const QPointF end = last ? p[m - 1] : (cuts[i + 1] + cuts[i + 2]) / 2;
    segments.push_back({start, cuts[i], cuts[i + 1], end});
    start = end;
  }
  return segments;
}
} // namespace

std::vector<BezierSegment> bsplineToBezier(const std::vector<QPointF> &points,
                                           bool closed) {
  if (points.size() < 3)
    return {};
  return closed ? closedToBezier(points) : openToBezier(points);
}
} // namespace render
