#pragma once

#include "model/Scene.h"
#include <QWidget>
#include <optional>

class CanvasWidget : public QWidget {
public:
    explicit CanvasWidget(model::Scene& scene, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;


private:
    struct DragState {
        model::PolygonId polygon;
        std::optional<model::VertexId> vertex; // pusty oznacza przeciagaie wielokata
        QPointF lastPosition;                  // ostatnia pozycja myszy w czasie przeciagania
    };

    struct EdgeHit {
        model::PolygonId polygon;
        model::EdgeId edge;
    };

    std::optional<DragState> hitTest(QPointF position) const;
    std::optional<EdgeHit> hitTestEdge(QPointF position) const;
    model::Scene& scene_;
    std::optional<DragState> dragState_;
};
