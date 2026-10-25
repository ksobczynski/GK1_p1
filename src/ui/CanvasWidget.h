#pragma once

#include "model/Scene.h"
#include "render/LineDrawing.h"
#include <QWidget>
#include <optional>

class CanvasWidget : public QWidget {
public:
  explicit CanvasWidget(model::Scene &scene, QWidget *parent = nullptr);
  void setLineAlgorithm(render::LineAlgorithm algorithm) {
    lineAlgorithm_ = algorithm;
    update();
  }

protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void contextMenuEvent(QContextMenuEvent *event) override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;

private:
  struct Hit {
    enum class Kind { Vertex, ControlPoint, Edge, Interior };
    Kind kind;
    model::PolygonId polygon;
    model::VertexId vertex = 0;
    model::EdgeId edge = 0;
    std::size_t control = 0;
  };

  struct DragState {
    Hit target;
    QPointF lastPosition;
  };

  std::optional<Hit> hitTest(QPointF position) const;
  void drawConstraintLabels(QPainter &painter) const;
  model::Scene &scene_;
  std::optional<DragState> dragState_;
  std::optional<model::EdgeRef> pendingOrthogonal_;
  render::LineAlgorithm lineAlgorithm_ = render::LineAlgorithm::Library;
};
