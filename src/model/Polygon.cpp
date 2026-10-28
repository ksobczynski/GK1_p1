#include "Polygon.h"
#include "model/Edge.h"
#include "model/Vertex.h"

#include <algorithm>
#include <stdexcept>

namespace model {
Polygon::Polygon(PolygonId id, const std::vector<QPointF> &points) : id_(id) {
  if (points.size() < 3)
    throw std::invalid_argument("A polygon needs at least three vertices");

  for (const QPointF &point : points)
    vertices_.push_back({nextVertexId_++, point});

  for (std::size_t i = 0; i < vertices_.size(); ++i)
    edges_.push_back({nextEdgeId_++, vertices_[i].id,
                      vertices_[(i + 1) % vertices_.size()].id});
}

const Vertex *Polygon::findVertex(VertexId id) const {
  auto it = std::find_if(vertices_.begin(), vertices_.end(),
                         [id](const Vertex &v) { return v.id == id; });
  return it == vertices_.end() ? nullptr : &*it;
}

const Edge *Polygon::findEdge(EdgeId id) const {
  auto it = std::find_if(edges_.begin(), edges_.end(),
                         [id](const Edge &e) { return e.id == id; });
  return it == edges_.end() ? nullptr : &*it;
}

bool Polygon::moveVertex(VertexId id, QPointF position) {
  for (Vertex &vertex : vertices_) {
    if (vertex.id == id) {
      vertex.position = position;
      return true;
    }
  }
  return false;
}

void Polygon::translate(double dx, double dy) {
  for (Vertex &vertex : vertices_)
    vertex.position += QPointF(dx, dy);

  for (Edge &edge : edges_) {
    for (QPointF &control : edge.controls) {
      control += QPointF(dx, dy);
    }
  }
}

bool Polygon::insertVertexAtEdgeMidpoint(EdgeId id) {
  auto it = std::find_if(edges_.begin(), edges_.end(),
                         [id](const Edge &e) { return e.id == id; });
  if (it == edges_.end() || it->hidden)
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

bool Polygon::removeVertex(VertexId id) {
  if (vertices_.size() <= 3)
    return false;
  auto it = std::find_if(vertices_.begin(), vertices_.end(),
                         [id](const Vertex &v) { return v.id == id; });
  if (it == vertices_.end())
    return false;

  const std::size_t index = static_cast<std::size_t>(it - vertices_.begin());
  const std::size_t previous =
      (index + vertices_.size() - 1) % vertices_.size();
  const Edge merged{nextEdgeId_++, edges_[previous].start, edges_[index].end};
  edges_[previous] = merged;
  edges_.erase(edges_.begin() + index);
  vertices_.erase(it);
  return true;
}

namespace {
double cross(QPointF o, QPointF a, QPointF b) {
  return (a.x() - o.x()) * (b.y() - o.y()) - (a.y() - o.y()) * (b.x() - o.x());
}
} // namespace

bool Polygon::convexHullContains(QPointF point) const {
  std::vector<QPointF> points;
  points.reserve(vertices_.size());
  for (const Vertex &vertex : vertices_)
    points.push_back(vertex.position);

  // Znalezienie poczatku otoczki
  auto pivotIt =
      std::min_element(points.begin(), points.end(), [](QPointF a, QPointF b) {
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
    while (hull.size() >= 2 &&
           cross(hull[hull.size() - 2], hull.back(), p) <= 0)
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

Edge *Polygon::findEdgeMutable(EdgeId id) {
  auto it = std::find_if(edges_.begin(), edges_.end(),
                         [id](const Edge &e) { return e.id == id; });
  return it == edges_.end() ? nullptr : &*it;
}

bool Polygon::setEdgeKind(EdgeId id, EdgeKind kind) {

  Edge *edge = findEdgeMutable(id);
  if (!edge)
    return false;
  if (edge->kind == kind)
    return true;

  edge->kind = kind;
  const std::size_t n = vertices_.size();
  const std::size_t e = static_cast<std::size_t>(edge - edges_.data());
  if (kind == EdgeKind::Arc) {
    Vertex &a = vertices_[e];
    Vertex &b = vertices_[(e + 1) % n];
    for (Vertex *v : {&a, &b})
      if (v->continuity == Continuity::C1)
        v->continuity = Continuity::G1;
    if (edges_[(e + n - 1) % n].kind == EdgeKind::Arc)
      a.continuity = Continuity::G0;
    if (edges_[(e + 1) % n].kind == EdgeKind::Arc)
      b.continuity = Continuity::G0;
    if (a.continuity != Continuity::G0 && b.continuity != Continuity::G0)
      b.continuity = Continuity::G0;
  }
  if (kind == EdgeKind::Bezier) {

    for (Vertex &v : vertices_) {
      const std::size_t index = static_cast<std::size_t>(&v - vertices_.data());
      if ((v.id == edge->start || v.id == edge->end) && !v.continuityExplicit &&
          !nextToArc(v.id) && !isOpenEnd(index)) {
        v.continuity = Continuity::C1;
      }
    }
    const QPointF a = findVertex(edge->start)->position;
    const QPointF b = findVertex(edge->end)->position;
    edge->controls = {a + (b - a) / 3, a + 2 * (b - a) / 3};
  }
  return true;
}

bool Polygon::moveControlPoint(EdgeId id, std::size_t index, QPointF position) {
  Edge *edge = findEdgeMutable(id);
  if (!edge || edge->kind != EdgeKind::Bezier || index >= edge->controls.size())
    return false;
  edge->controls[index] = position;
  return true;
}

bool Polygon::setContinuity(VertexId id, Continuity continuity) {
  for (Vertex &v : vertices_) {
    if (v.id == id) {
      v.continuity = continuity;
      v.continuityExplicit = true;
      return true;
    }
  }
  return false;
}

bool Polygon::hideEdge(EdgeId id) {
  if (hiddenEdgeIndex())
    return false;
  Edge *edge = findEdgeMutable(id);
  if (!edge)
    return false;
  edge->kind = EdgeKind::Segment;
  edge->hidden = true;
  resetOpenEnds();
  return true;
}

bool Polygon::showHiddenEdge() {
  const std::optional<std::size_t> hidden = hiddenEdgeIndex();
  if (!hidden)
    return false;
  edges_[*hidden].hidden = false;
  return true;
}

std::optional<std::size_t> Polygon::hiddenEdgeIndex() const {
  for (std::size_t e = 0; e < edges_.size(); ++e)
    if (edges_[e].hidden)
      return e;
  return std::nullopt;
}

bool Polygon::isOpenEnd(std::size_t v) const {
  const std::size_t n = vertices_.size();
  return edges_[v].hidden || edges_[(v + n - 1) % n].hidden;
}

std::vector<QPointF> Polygon::chainPoints() const {
  const std::size_t n = vertices_.size();
  const std::optional<std::size_t> hidden = hiddenEdgeIndex();
  const std::size_t first = hidden ? (*hidden + 1) % n : 0;
  std::vector<QPointF> points;
  points.reserve(n);
  for (std::size_t i = 0; i < n; ++i)
    points.push_back(vertices_[(first + i) % n].position);
  return points;
}

void Polygon::resetOpenEnds() {
  for (std::size_t v = 0; v < vertices_.size(); ++v) {
    if (!isOpenEnd(v))
      continue;
    vertices_[v].continuity = Continuity::G0;
    vertices_[v].continuityExplicit = false;
  }
}

bool Polygon::nextToArc(VertexId id) const {
  const std::size_t n = vertices_.size();
  for (std::size_t v = 0; v < n; ++v) {
    if (vertices_[v].id == id)
      return edges_[v].kind == EdgeKind::Arc ||
             edges_[(v + n - 1) % n].kind == EdgeKind::Arc;
  }
  return false;
}

QPointF Polygon::incomingTangent(std::size_t v) const {
  const std::size_t n = vertices_.size();
  const Edge &edge = edges_[(v + n - 1) % n];
  const QPointF V = vertices_[v].position;
  if (edge.kind == EdgeKind::Bezier)
    return 3 * (V - edge.controls[1]);
  return V - vertices_[(v + n - 1) % n].position;
}

QPointF Polygon::outgoingTangent(std::size_t v) const {
  const std::size_t n = vertices_.size();
  const Edge &edge = edges_[v];
  const QPointF V = vertices_[v].position;
  if (edge.kind == EdgeKind::Bezier)
    return 3 * (edge.controls[0] - V);
  return vertices_[(v + 1) % n].position - V;
}
Polygon::ArcTangents Polygon::arcTangents(std::size_t e) const {
  const std::size_t n = vertices_.size();
  const std::size_t start = e;
  const std::size_t end = (e + 1) % n;
  ArcTangents result;
  if (vertices_[start].continuity != Continuity::G0 &&
      edges_[(start + n - 1) % n].kind != EdgeKind::Arc)
    result.start = incomingTangent(start);
  else if (vertices_[end].continuity != Continuity::G0 &&
           edges_[end].kind != EdgeKind::Arc)
    result.end = outgoingTangent(end);
  return result;
}
} // namespace model
