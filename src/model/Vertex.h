#pragma once

#include <cstdint>

namespace model {
using VertexId = std::uint64_t;

struct Point {
    double x = 0.0;
    double y = 0.0;
};

struct Vertex {
    VertexId id;
    Point position;
};
} // namespace model
