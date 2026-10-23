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

private:
  void enforceLengths(Polygon &polygon, std::size_t origin,
                      std::vector<bool> fixed);
  bool allEdgesHaveLength(const Polygon &polygon) const;
  bool lengthsSatisfied(const Polygon &polygon) const;
  void removeConstraintsOf(EdgeRef edge);
  PolygonId nextPolygonId_ = 1;
  std::vector<Polygon> polygons_;
  ConstraintId nextConstraintId_ = 1;
  std::vector<Constraint> constraints_;
};
} // namespace model
