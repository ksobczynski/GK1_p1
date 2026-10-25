#include "LineDrawing.h"

#include <cstdlib>
#include <utility>
#include <vector>

namespace render {
namespace {
std::vector<QPoint> bresenham(QPoint a, QPoint b) {
  int dx = std::abs(b.x() - a.x());
  int dy = std::abs(b.y() - a.y());
  const int sx = a.x() < b.x() ? 1 : -1;
  const int sy = a.y() < b.y() ? 1 : -1;

  const bool steep = dy > dx;

  if (steep)
    std::swap(dx, dy);

  const int incrE = 2 * dy;
  const int incrNE = 2 * (dy - dx);
  int d = 2 * dy - dx;
  std::vector<QPoint> pixels;
  pixels.reserve(dx + 1);
  int x = a.x();
  int y = a.y();
  pixels.emplace_back(x, y);
  for (int i = 0; i < dx; i++) {
    if (d < 0) {
      d += incrE;
    } else {
      d += incrNE;
      if (steep) {
        x += sx;
      } else {
        y += sy;
      }
    }
    if (steep)
      y += sy;
    else
      x += sx;
    pixels.emplace_back(x, y);
  }
  return pixels;
}
} // namespace

void drawSegment(QPainter &painter, QPointF a, QPointF b,
                 LineAlgorithm algorithm) {
  if (algorithm == LineAlgorithm::Library) {
    painter.drawLine(a, b);
    return;
  }

  const std::vector<QPoint> pixels = bresenham(a.toPoint(), b.toPoint());
  painter.save();
  QPen pen = painter.pen();
  pen.setWidth(1);
  painter.setPen(pen);
  painter.drawPoints(pixels.data(), static_cast<int>(pixels.size()));
  painter.restore();
}

} // namespace render