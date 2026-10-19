#pragma once

#include "Edge.h"
#include <vector>

namespace model {
using PolygonId = std::uint64_t;

class Polygon {
public:
    Polygon(PolygonId id, const std::vector<QPointF>& points);

    PolygonId id() const { return id_; }
    const std::vector<Vertex>& vertices() const { return vertices_; }
    const std::vector<Edge>& edges() const { return edges_; }
    const Vertex* findVertex(VertexId id) const;

    bool moveVertex(VertexId id, QPointF position);
    void translate(double dx, double dy);
    bool insertVertexAtEdgeMidpoint(EdgeId id);
    bool removeVertex(VertexId id);
    bool convexHullContains(QPointF point) const;

private:
    PolygonId id_;
    VertexId nextVertexId_ = 1;
    EdgeId nextEdgeId_ = 1;
    std::vector<Vertex> vertices_;
    // Edge i connects vertex i to vertex (i + 1) modulo vertex count.
    std::vector<Edge> edges_;
};
} // namespace model
