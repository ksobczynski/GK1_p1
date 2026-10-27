#pragma once

#include "Vertex.h"
#include <array>

namespace model {
using EdgeId = std::uint64_t;

enum class EdgeKind { Segment, Bezier, Arc };

struct Edge {
  EdgeId id;
  VertexId start;
  VertexId end;
  EdgeKind kind = EdgeKind::Segment;
  std::array<QPointF, 2> controls{};
};
} // namespace model
