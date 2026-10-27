#pragma once

#include "Edge.h"
#include "model/Vertex.h"
#include <optional>
#include <vector>

namespace model {
using PolygonId = std::uint64_t;

class Polygon {
public:
  Polygon(PolygonId id, const std::vector<QPointF> &points);

  PolygonId id() const { return id_; }
  const std::vector<Vertex> &vertices() const { return vertices_; }
  const std::vector<Edge> &edges() const { return edges_; }
  const Vertex *findVertex(VertexId id) const;
  const Edge *findEdge(EdgeId id) const;

  bool moveVertex(VertexId id, QPointF position);
  void translate(double dx, double dy);
  bool insertVertexAtEdgeMidpoint(EdgeId id);
  bool removeVertex(VertexId id);
  bool convexHullContains(QPointF point) const;

  bool setEdgeKind(EdgeId id, EdgeKind kind);
  bool moveControlPoint(EdgeId id, std::size_t index, QPointF position);

  bool setContinuity(VertexId id, Continuity continuity);

  QPointF incomingTangent(std::size_t v) const;
  QPointF outgoingTangent(std::size_t v) const;

  struct ArcTangents {
    std::optional<QPointF> start;
    std::optional<QPointF> end;
  };

  ArcTangents arcTangents(std::size_t e) const;

private:
  bool nextToArc(VertexId id) const;
  PolygonId id_;
  VertexId nextVertexId_ = 1;
  EdgeId nextEdgeId_ = 1;
  std::vector<Vertex> vertices_;
  std::vector<Edge> edges_;
  Edge *findEdgeMutable(EdgeId id);
};
} // namespace model
