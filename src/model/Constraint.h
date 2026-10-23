#pragma once

#include "Polygon.h"
#include <optional>

namespace model
{
    using ConstraintId = std::uint64_t;

    struct EdgeRef
    {
        PolygonId polygon;
        EdgeId edge;

        bool operator==(const EdgeRef &other) const
        {
            return polygon == other.polygon && edge == other.edge;
        }
        bool operator!=(const EdgeRef &other) const { return !(*this == other); }
    };

    enum class ConstraintType
    {
        Length,
        Orthogonal,
    };

    struct Constraint
    {
        ConstraintId id;
        ConstraintType type;
        EdgeRef first;
        std::optional<EdgeRef> second;
        double length = 0.0;
    };
} // namespace model
