#include "CanvasWidget.h"
#include "model/Constraint.h"
#include "model/Edge.h"
#include "model/Polygon.h"
#include "render/Bezier.h"
#include "render/LineDrawing.h"
#include <QContextMenuEvent>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>
namespace {
void drawEdgeLabel(QPainter &painter, QPointF a, QPointF b, const QString &text,
                   double offset) {
  const QPointF ab = b - a;
  const double length = std::hypot(ab.x(), ab.y());
  if (length == 0)
    return;
  const QPointF normal(-ab.y() / length, ab.x() / length);
  const QPointF center = (a + b) / 2 + offset * normal;
  painter.drawText(QRectF(center - QPointF(60, 9), QSizeF(120, 18)),
                   Qt::AlignCenter, text);
}
} // namespace

CanvasWidget::CanvasWidget(model::Scene &scene, QWidget *parent)
    : QWidget(parent), scene_(scene) {
  setFocusPolicy(Qt::StrongFocus);
}

void CanvasWidget::paintEvent(QPaintEvent *) {
  QPainter painter(this);
  painter.fillRect(rect(), Qt::white);

  for (const model::Polygon &polygon : scene_.polygons()) {
    painter.setPen(QPen(Qt::darkBlue, 2));
    for (const model::Edge &edge : polygon.edges()) {
      const QPointF a = polygon.findVertex(edge.start)->position;
      const QPointF b = polygon.findVertex(edge.end)->position;
      if (edge.kind == model::EdgeKind::Bezier)
        render::drawBezier(painter, a, edge.controls[0], edge.controls[1], b);
      else
        render::drawSegment(painter, a, b, lineAlgorithm_);
    }
    painter.save();
    painter.setPen(QPen(Qt::gray, 1, Qt::DashLine));
    painter.setBrush(Qt::gray);
    for (const model::Edge &edge : polygon.edges()) {
      if (edge.kind != model::EdgeKind::Bezier)
        continue;
      const QPointF a = polygon.findVertex(edge.start)->position;
      const QPointF b = polygon.findVertex(edge.end)->position;
      const QPointF c0 = edge.controls[0];
      const QPointF c1 = edge.controls[1];
      painter.drawLine(a, c0);
      painter.drawLine(c0, c1);
      painter.drawLine(c1, b);
      for (const QPointF &control : edge.controls)
        painter.drawEllipse(control, 4, 4);
    }
    painter.restore();

    painter.setPen(Qt::black);
    painter.setBrush(Qt::white);
    for (const model::Vertex &vertex : polygon.vertices())
      painter.drawEllipse(vertex.position, 5, 5);
  }

  if (pendingOrthogonal_) {
    const model::Polygon *polygon =
        scene_.findPolygon(pendingOrthogonal_->polygon);
    const model::Edge *edge =
        polygon ? polygon->findEdge(pendingOrthogonal_->edge) : nullptr;
    if (polygon && edge) {
      painter.setPen(QPen(QColor(255, 140, 0), 4));
      painter.drawLine(polygon->findVertex(edge->start)->position,
                       polygon->findVertex(edge->end)->position);
    }
    painter.setPen(Qt::black);
    painter.drawText(QPointF(10, 20),
                     "Kliknij na druga krawedz (lub Esc aby anulowac)");
  }
  drawConstraintLabels(painter);
}

void CanvasWidget::drawConstraintLabels(QPainter &painter) const {

  constexpr double labelOffset = 14.0;
  for (const model::Polygon &polygon : scene_.polygons()) {
    for (const model::Edge &edge : polygon.edges()) {
      const model::EdgeRef ref{polygon.id(), edge.id};
      const QPointF a = polygon.findVertex(edge.start)->position;
      const QPointF b = polygon.findVertex(edge.end)->position;

      if (const std::optional<double> length = scene_.lengthConstraint(ref)) {
        painter.setPen(Qt::darkRed);
        drawEdgeLabel(painter, a, b, QString::number(*length, 'f', 1),
                      labelOffset);
      }

      QStringList orthogonal;
      for (const model::Constraint &c : scene_.constraints()) {
        if (c.type == model::ConstraintType::Orthogonal && c.involves(ref))
          orthogonal << QString("⊥%1").arg(c.id);
      }
      if (!orthogonal.isEmpty()) {
        painter.setPen(Qt::darkGreen);
        drawEdgeLabel(painter, a, b, orthogonal.join(' '), -labelOffset);
      }
    }
  }
}

namespace {
// Kwadrat odleglosci punktu p od odcinka ab.
double squaredDistanceToSegment(QPointF p, QPointF a, QPointF b) {
  const QPointF ab = b - a;
  const double lengthSquared = QPointF::dotProduct(ab, ab);
  double t = 0.0;
  if (lengthSquared > 0)
    t = std::clamp(QPointF::dotProduct(p - a, ab) / lengthSquared, 0.0, 1.0);
  const QPointF d = p - (a + t * ab);
  return QPointF::dotProduct(d, d);
}
} // namespace

std::optional<CanvasWidget::Hit> CanvasWidget::hitTest(QPointF position) const {
  constexpr double vertexRadius = 8.0;
  constexpr double edgeDistance = 5.0;
  const std::vector<model::Polygon> &polygons = scene_.polygons();

  for (const model::Polygon &polygon : polygons) {
    for (const model::Vertex &vertex : polygon.vertices()) {
      const QPointF d = vertex.position - position;
      if (QPointF::dotProduct(d, d) <= vertexRadius * vertexRadius)
        return Hit{Hit::Kind::Vertex, polygon.id(), vertex.id, 0};
    }
  }
  for (const model::Polygon &polygon : polygons) {
    for (const model::Edge &edge : polygon.edges()) {
      if (edge.kind != model::EdgeKind::Bezier)
        continue;
      for (std::size_t i = 0; i < edge.controls.size(); i++) {
        const QPointF d = edge.controls[i] - position;
        if (QPointF::dotProduct(d, d) <= vertexRadius * vertexRadius)
          return Hit{Hit::Kind::ControlPoint, polygon.id(), 0, edge.id, i};
      }
    }
  }

  for (const model::Polygon &polygon : polygons) {
    for (const model::Edge &edge : polygon.edges()) {
      const QPointF a = polygon.findVertex(edge.start)->position;
      const QPointF b = polygon.findVertex(edge.end)->position;
      double distanceSquared;
      if (edge.kind == model::EdgeKind::Bezier) {
        const std::vector<QPointF> points =
            render::bezierPoints(a, edge.controls[0], edge.controls[1], b);
        distanceSquared = std::numeric_limits<double>::max();
        for (std::size_t i = 1; i < points.size(); i++) {
          distanceSquared = std::min(
              distanceSquared,
              squaredDistanceToSegment(position, points[i - 1], points[i]));
        }
      } else {
        distanceSquared = squaredDistanceToSegment(position, a, b);
      }
      if (distanceSquared <= edgeDistance * edgeDistance)
        return Hit{Hit::Kind::Edge, polygon.id(), 0, edge.id};
    }
  }

  for (auto it = polygons.rbegin(); it != polygons.rend(); ++it)
    if (it->convexHullContains(position))
      return Hit{Hit::Kind::Interior, it->id(), 0, 0};

  return std::nullopt;
}

void CanvasWidget::mousePressEvent(QMouseEvent *event) {
  if (event->button() != Qt::LeftButton)
    return;
  if (pendingOrthogonal_) {
    const std::optional<Hit> hit = hitTest(event->position());
    const model::EdgeRef first = *pendingOrthogonal_;
    pendingOrthogonal_.reset();
    update();

    if (hit && hit->kind == Hit::Kind::Edge &&
        scene_.setOrthogonalConstraint(first, {hit->polygon, hit->edge}) == 0)
      QMessageBox::warning(this, "Ograniczenie prostopadlosci",
                           "Nie da sie spelnic ograniczenia prostopadlosci");
    return;
  }
  const QPointF position = event->position();
  if (const std::optional<Hit> hit = hitTest(position))
    dragState_ = DragState{*hit, position};
  else
    dragState_.reset();
}

void CanvasWidget::mouseMoveEvent(QMouseEvent *event) {
  if (!dragState_)
    return;
  model::Polygon *polygon = scene_.findPolygon(dragState_->target.polygon);
  if (!polygon)
    return;

  const QPointF position = event->position();
  switch (dragState_->target.kind) {
  case Hit::Kind::Vertex:
    scene_.moveVertex(dragState_->target.polygon, dragState_->target.vertex,
                      position);
    break;
  case Hit::Kind::Edge: // chwycenie krawedzi przesuwa caly wielokat
  case Hit::Kind::Interior: {
    const QPointF delta = position - dragState_->lastPosition;
    polygon->translate(delta.x(), delta.y());
    break;
  }
  case Hit::Kind::ControlPoint: {
    scene_.moveControlPoint(
        {dragState_->target.polygon, dragState_->target.edge},
        dragState_->target.control, position);
    break;
  }
  }
  dragState_->lastPosition = position;
  update();
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton)
    dragState_.reset();
}

void CanvasWidget::contextMenuEvent(QContextMenuEvent *event) {
  pendingOrthogonal_.reset();
  const std::optional<Hit> hit = hitTest(event->pos());
  if (!hit)
    return;
  model::Polygon *polygon = scene_.findPolygon(hit->polygon);
  if (!polygon)
    return;

  QMenu menu(this);

  if (hit->kind == Hit::Kind::Vertex) {
    QAction *removeVertexAction = menu.addAction("Usun wierzcholek");
    removeVertexAction->setEnabled(polygon->vertices().size() > 3);

    if (menu.exec(event->globalPos()) == removeVertexAction) {
      scene_.removeVertex(hit->polygon, hit->vertex);
      dragState_.reset();
      update();
    }
  } else if (hit->kind == Hit::Kind::Edge) {
    const model::EdgeRef edgeRef{hit->polygon, hit->edge};

    const model::Edge *edge = polygon->findEdge(hit->edge);
    const bool bezier = edge->kind == model::EdgeKind::Bezier;
    QAction *kindAction = menu.addAction(bezier ? "zamień na odcinek"
                                                : "zamień na krzywą beziera");

    QAction *lengthAction = menu.addAction("Ustaw dlugosc...");
    QAction *removeLengthAction = menu.addAction("Usun ograniczenie długości");

    removeLengthAction->setEnabled(
        scene_.lengthConstraint(edgeRef).has_value());

    QAction *orthogonalAction = menu.addAction("Prostopadła do...");

    lengthAction->setEnabled(!bezier);
    orthogonalAction->setEnabled(!bezier);

    for (const model::Constraint &c : scene_.constraints()) {
      if (c.type != model::ConstraintType::Orthogonal || !c.involves(edgeRef))
        continue;
      QAction *action =
          menu.addAction(QString("Usun prostopadlosc ⊥%1").arg(c.id));
      action->setData(static_cast<qulonglong>(c.id));
    }

    QAction *chosen = menu.exec(event->globalPos());

    if (chosen == lengthAction) {
      const QPointF a = polygon->findVertex(edge->start)->position;
      const QPointF b = polygon->findVertex(edge->end)->position;
      const double currentLength = std::hypot(b.x() - a.x(), b.y() - a.y());

      bool ok = false;
      const double length = QInputDialog::getDouble(this, "Dlugosc krawedzi",
                                                    "Dlugosc:", currentLength,
                                                    1.0, 10000.0, 1, &ok);

      if (ok) {
        if (scene_.setLengthConstraint({hit->polygon, hit->edge}, length) == 0)
          QMessageBox::warning(this, "Długość krawędzi",
                               "Nie da się spełnić długości krawedzi");
        update();
      }
    } else if (chosen == removeLengthAction) {
      scene_.removeLengthConstraint(edgeRef);
      update();
    } else if (chosen == orthogonalAction) {
      pendingOrthogonal_ = edgeRef;
      update();
    } else if (chosen == kindAction) {
      scene_.setEdgeKind(edgeRef, bezier ? model::EdgeKind::Segment
                                         : model::EdgeKind::Bezier);
      update();
    } else if (chosen && chosen->data().isValid()) {
      scene_.removeConstraint(chosen->data().toULongLong());
      update();
    }
  }
}

void CanvasWidget::mouseDoubleClickEvent(QMouseEvent *event) {
  if (event->button() != Qt::LeftButton)
    return;

  const std::optional<Hit> hit = hitTest(event->position());
  if (!hit || hit->kind != Hit::Kind::Edge)
    return;

  if (scene_.insertVertexAtMidpoint(hit->polygon, hit->edge)) {
    dragState_.reset();
    update();
  }
}

void CanvasWidget::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Escape && pendingOrthogonal_) {
    pendingOrthogonal_.reset();
    update();
    return;
  }
  QWidget::keyPressEvent(event);
}
