#pragma once

#include "Vertex.h"

namespace model {
using EdgeId = std::uint64_t;

struct Edge {
    EdgeId id;
    VertexId start;
    VertexId end;
};
} 
