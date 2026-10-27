#pragma once

#include <QPointF>
#include <cstdint>

namespace model {
using VertexId = std::uint64_t;

enum class Continuity { G0, G1, C1 };

struct Vertex {
  VertexId id;
  QPointF position;
  Continuity continuity = Continuity::G0;

  bool continuityExplicit = false;
};
} // namespace model
