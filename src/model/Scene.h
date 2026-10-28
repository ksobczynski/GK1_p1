#pragma once

#include "Constraint.h"
#include "Polygon.h"
#include "model/Vertex.h"
#include <optional>
#include <vector>

namespace model {
class Scene {
public:
  PolygonId addPolygon(const std::vector<QPointF> &points);
  const std::vector<Polygon> &polygons() const { return polygons_; }
  Polygon *findPolygon(PolygonId id);
  const Polygon *findPolygon(PolygonId id) const;

  const std::vector<Constraint> &constraints() const { return constraints_; }
  // Krawedz ma co najwyzej jedno ograniczenie dlugosci: istniejace jest
  // nadpisywane.
  ConstraintId setLengthConstraint(EdgeRef edgeRef, double length);
  static Scene createDemo();
  std::optional<double> lengthConstraint(EdgeRef edgeRef) const;

  void moveVertex(PolygonId polygon, VertexId vertex, QPointF position);

  bool insertVertexAtMidpoint(PolygonId polygon, EdgeId edge);
  bool removeVertex(PolygonId polygon, VertexId vertex);
  void removeLengthConstraint(EdgeRef edge);

  ConstraintId setOrthogonalConstraint(EdgeRef first, EdgeRef second);
  void removeConstraint(ConstraintId id);

  bool setEdgeKind(EdgeRef edge, EdgeKind kind);
  void moveControlPoint(EdgeRef edge, std::size_t index, QPointF position);

  bool setContinuity(PolygonId polygon, VertexId vertex, Continuity continuity);

  bool hideEdge(EdgeRef edge);
  bool showHiddenEdge(PolygonId polygon);

private:
  // void enforceLengths(Polygon &polygon, std::size_t origin,
  // std::vector<bool> fixed);
  void propagate(Polygon &polygon, const std::vector<Vertex> &before,
                 std::vector<std::size_t> queue, std::vector<bool> fixed);
  // bool allEdgesHaveLength(const Polygon &polygon) const;
  // bool lengthsSatisfied(const Polygon &polygon) const;
  void removeConstraintsOf(EdgeRef edge);

  bool hasOrthogonalConstraint(EdgeRef edge) const;

  std::optional<QPointF> edgeVector(EdgeRef edge) const;
  bool constraintsSatisfied() const;

  void rotatePerpendicular(EdgeRef ref, QPointF reference, bool pivotAtStart);

  bool isSegment(EdgeRef edge) const;
  PolygonId nextPolygonId_ = 1;
  std::vector<Polygon> polygons_;
  ConstraintId nextConstraintId_ = 1;
  std::vector<Constraint> constraints_;

  enum class Side { Incoming, Outgoing };
  void followControlPoints(Polygon &polygon, const std::vector<Vertex> &before);
  void enforceContinuityEverywhere(Polygon &polygon);
  void enforceContinuity(Polygon &polygon, std::size_t vertex, Side master);
};
} // namespace model
