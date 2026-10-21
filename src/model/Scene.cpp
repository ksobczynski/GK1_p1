#include "Scene.h"

namespace model {
PolygonId Scene::addPolygon(const std::vector<QPointF>& points)
{
    const PolygonId id = nextPolygonId_;
    polygons_.emplace_back(id, points);
    ++nextPolygonId_;
    return id;
}

Polygon* Scene::findPolygon(PolygonId id)
{
    for (Polygon& polygon : polygons_)
        if (polygon.id() == id)
            return &polygon;
            
    return nullptr;
}

const Polygon* Scene::findPolygon(PolygonId id) const
{
    for (const Polygon& polygon : polygons_)
        if (polygon.id() == id)
            return &polygon;

    return nullptr;
}

ConstraintId Scene::setLengthConstraint(EdgeRef edge, double length)
{
    for (Constraint& constraint : constraints_) {
        if (constraint.type == ConstraintType::Length && constraint.first == edge) {
            constraint.length = length;
            return constraint.id;
        }
    }

    const ConstraintId id = nextConstraintId_++;
    constraints_.push_back({id, ConstraintType::Length, edge, std::nullopt, length});
    return id;
}

Scene Scene::createDemo()
{
    Scene scene;
    scene.addPolygon({{100, 100}, {330, 130}, {290, 330}, {130, 280}});
    // A self-intersecting polygon is represented just like any other polygon.
    scene.addPolygon({{470, 120}, {730, 330}, {470, 330}, {730, 120}});
    return scene;
}
} // namespace model
