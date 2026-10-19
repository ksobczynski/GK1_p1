#include "Polygon.h"

#include <algorithm>
#include <stdexcept>

namespace model {
Polygon::Polygon(PolygonId id, const std::vector<QPointF>& points) : id_(id)
{
    if (points.size() < 3)
        throw std::invalid_argument("A polygon needs at least three vertices");

    for (const QPointF& point : points)
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

bool Polygon::moveVertex(VertexId id, QPointF position)
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
    for (Vertex& vertex : vertices_)
        vertex.position += QPointF(dx, dy);
}

bool Polygon::insertVertexAtEdgeMidpoint(EdgeId id)
{
    auto it = std::find_if(edges_.begin(), edges_.end(),
                           [id](const Edge& e) { return e.id == id; });
    if (it == edges_.end())
        return false;

    const std::size_t index = static_cast<std::size_t>(it - edges_.begin());
    const Edge old = *it;
    const QPointF a = vertices_[index].position;
    const QPointF b = vertices_[(index + 1) % vertices_.size()].position;
    const Vertex vertex{nextVertexId_++, (a + b) / 2};
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

namespace {
double cross(QPointF o, QPointF a, QPointF b)
{
    return (a.x() - o.x()) * (b.y() - o.y()) - (a.y() - o.y()) * (b.x() - o.x());
}
} // namespace

bool Polygon::convexHullContains(QPointF point) const
{
    std::vector<QPointF> points;
    points.reserve(vertices_.size());
    for (const Vertex& vertex : vertices_)
        points.push_back(vertex.position);

    // Znalezienie poczatku otoczki
    auto pivotIt = std::min_element(points.begin(), points.end(), [](QPointF a, QPointF b) {
        return a.y() < b.y() || (a.y() == b.y() && a.x() < b.x());
    });
    std::iter_swap(points.begin(), pivotIt);
    const QPointF pivot = points.front();

    // Sortowanie przed Grahamem
    std::sort(points.begin() + 1, points.end(), [pivot](QPointF a, QPointF b) {
        const double turn = cross(pivot, a, b);
        if (turn != 0)
            return turn > 0;
        const QPointF da = a - pivot;
        const QPointF db = b - pivot;
        return QPointF::dotProduct(da, da) < QPointF::dotProduct(db, db);
    });

    // Alg Grahama
    std::vector<QPointF> hull;
    for (QPointF p : points) {
        while (hull.size() >= 2 && cross(hull[hull.size() - 2], hull.back(), p) <= 0)
            hull.pop_back();
        hull.push_back(p);
    }

    if (hull.size() < 3)
        return false; // koliniowe wierzcholki, rzadki przypadek ale jest

    // Sprawdzamy czy punkt wewnatrz wielokata - czy kazdy skret jest w prawo
    for (std::size_t i = 0; i < hull.size(); ++i)
        if (cross(hull[i], hull[(i + 1) % hull.size()], point) < 0)
            return false;
    return true;
}
} // namespace model
