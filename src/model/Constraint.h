#pragma once

#include "Polygon.h"
#include <optional>

namespace model {
using ConstraintId = std::uint64_t;

// Edge reference valid across polygons (see the note in Scene.h).
struct EdgeRef {
    PolygonId polygon;
    EdgeId edge;

    bool operator==(const EdgeRef& other) const
    {
        return polygon == other.polygon && edge == other.edge;
    }
    bool operator!=(const EdgeRef& other) const { return !(*this == other); }
};

enum class ConstraintType {
    Length,     // first has a fixed length
    Orthogonal, // first is perpendicular to second
};

struct Constraint {
    ConstraintId id;
    ConstraintType type;
    EdgeRef first;
    std::optional<EdgeRef> second; // only for Orthogonal
    double length = 0.0;           // only for Length
};
} // namespace model
