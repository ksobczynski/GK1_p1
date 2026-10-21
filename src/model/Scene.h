#pragma once

#include "Constraint.h"
#include "Polygon.h"

namespace model {
class Scene {
public:
    PolygonId addPolygon(const std::vector<QPointF>& points);
    const std::vector<Polygon>& polygons() const { return polygons_; }
    Polygon* findPolygon(PolygonId id);
    const Polygon* findPolygon(PolygonId id) const;

    const std::vector<Constraint>& constraints() const { return constraints_; }
    // Krawedz ma co najwyzej jedno ograniczenie dlugosci: istniejace jest nadpisywane.
    ConstraintId setLengthConstraint(EdgeRef edge, double length);
    static Scene createDemo();

private:
    PolygonId nextPolygonId_ = 1;
    std::vector<Polygon> polygons_;
    ConstraintId nextConstraintId_ = 1;
    std::vector<Constraint> constraints_;
};
} // namespace model
