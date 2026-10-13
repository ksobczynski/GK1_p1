#include "Polygon.h"

#include <algorithm>
#include <stdexcept>

namespace model {
Polygon::Polygon(PolygonId id, const std::vector<Point>& points) : id_(id)
{
    if (points.size() < 3)
        throw std::invalid_argument("A polygon needs at least three vertices");

    for (const Point& point : points)
        vertices_.push_back({nextVertexId_++, point});

    for (std::size_t i = 0; i < vertices_.size(); ++i)
        edges_.push_back({nextEdgeId_++, vertices_[i].id,
                          vertices_[(i + 1) % vertices_.size()].id});
}

const Vertex* Polygon::findVertex(VertexId id) const
{
    auto it = std::find_if(vertices_.begin(), vertices_.end(),
                           [id](const Vertex& v) { return v.id == id; });
    return it == vertices_.end() ? nullptr : &*it;
}

bool Polygon::moveVertex(VertexId id, Point position)
{
    for (Vertex& vertex : vertices_) {
        if (vertex.id == id) {
            vertex.position = position;
            return true;
        }
    }
    return false;
}

void Polygon::translate(double dx, double dy)
{
    for (Vertex& vertex : vertices_) {
        vertex.position.x += dx;
        vertex.position.y += dy;
    }
}

bool Polygon::insertVertexAtEdgeMidpoint(EdgeId id)
{
    auto it = std::find_if(edges_.begin(), edges_.end(),
                           [id](const Edge& e) { return e.id == id; });
    if (it == edges_.end())
        return false;

    const std::size_t index = static_cast<std::size_t>(it - edges_.begin());
    const Edge old = *it;
    const Point a = vertices_[index].position;
    const Point b = vertices_[(index + 1) % vertices_.size()].position;
    const Vertex vertex{nextVertexId_++, {(a.x + b.x) / 2, (a.y + b.y) / 2}};
    vertices_.insert(vertices_.begin() + index + 1, vertex);

    // Replace only the split edge; unrelated edge IDs remain valid.
    edges_[index] = {nextEdgeId_++, old.start, vertex.id};
    edges_.insert(edges_.begin() + index + 1,
                  Edge{nextEdgeId_++, vertex.id, old.end});
    return true;
}

bool Polygon::removeVertex(VertexId id)
{
    if (vertices_.size() <= 3)
        return false;
    auto it = std::find_if(vertices_.begin(), vertices_.end(),
                           [id](const Vertex& v) { return v.id == id; });
    if (it == vertices_.end())
        return false;

    const std::size_t index = static_cast<std::size_t>(it - vertices_.begin());
    const std::size_t previous = (index + vertices_.size() - 1) % vertices_.size();
    const Edge merged{nextEdgeId_++, edges_[previous].start, edges_[index].end};
    edges_[previous] = merged;
    edges_.erase(edges_.begin() + index);
    vertices_.erase(it);
    return true;
}
}
