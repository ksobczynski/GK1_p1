#pragma once

#include <QPointF>
#include <cstdint>

namespace model {
using VertexId = std::uint64_t;

struct Vertex {
    VertexId id;
    QPointF position;
};
} // namespace model
