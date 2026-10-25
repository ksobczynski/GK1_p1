#include "Scene.h"
#include "model/Constraint.h"
#include "model/Edge.h"
#include "model/Polygon.h"

#include <algorithm>
#include <cmath>
#include <optional>

namespace model {
namespace {
constexpr double lengthEpsilon = 1e-6;

double distance(QPointF a, QPointF b) {
  const QPointF d = b - a;
  return std::hypot(d.x(), d.y());
}

QPointF pointAtDistance(QPointF a, QPointF b, double length) {
  const double current = distance(a, b);

  if (current < lengthEpsilon)
    return a + QPointF(length, 0);

  return a + (b - a) * (length / current);
}
} // namespace

PolygonId Scene::addPolygon(const std::vector<QPointF> &points) {
  const PolygonId id = nextPolygonId_;
  polygons_.emplace_back(id, points);
  ++nextPolygonId_;
  return id;
}

Polygon *Scene::findPolygon(PolygonId id) {
  for (Polygon &polygon : polygons_)
    if (polygon.id() == id)
      return &polygon;

  return nullptr;
}

const Polygon *Scene::findPolygon(PolygonId id) const {
  for (const Polygon &polygon : polygons_)
    if (polygon.id() == id)
      return &polygon;

  return nullptr;
}

ConstraintId Scene::setLengthConstraint(EdgeRef edgeRef, double length) {

  Polygon *polygon = findPolygon(edgeRef.polygon);
  const Edge *e = polygon ? polygon->findEdge(edgeRef.edge) : nullptr;

  if (!e || e->kind != EdgeKind::Segment)
    return 0;

  // const Polygon polygonBackup = *polygon;
  // const std::vector<Constraint> constraintsBackup = constraints_;
  // const ConstraintId nextIdBackup = nextConstraintId_;
  const Scene backup = *this;
  ConstraintId id = 0;
  for (Constraint &constraint : constraints_) {
    if (constraint.type == ConstraintType::Length &&
        constraint.first == edgeRef) {
      constraint.length = length;
      id = constraint.id;
      break;
    }
  }

  if (id == 0) {
    id = nextConstraintId_++;
    constraints_.push_back(
        {id, ConstraintType::Length, edgeRef, std::nullopt, length});
  }

  const std::size_t n = polygon->vertices().size();
  const std::size_t startIndex =
      static_cast<std::size_t>(e - polygon->edges().data());
  const std::size_t endIndex = (startIndex + 1) % n;
  const QPointF a = polygon->vertices()[startIndex].position;
  const QPointF b = polygon->vertices()[endIndex].position;
  const auto before = polygon->vertices();

  polygon->moveVertex(polygon->vertices()[endIndex].id,
                      pointAtDistance(a, b, length));

  std::vector<bool> fixed(n, false);
  fixed[startIndex] = true;
  propagate(*polygon, before, {endIndex}, fixed);

  if (!constraintsSatisfied()) {
    *this = backup;
    return 0;
  }

  return id;
}

Scene Scene::createDemo() {
  Scene scene;
  const PolygonId quad =
      scene.addPolygon({{100, 100}, {330, 130}, {290, 330}, {100, 330}});
  const PolygonId bowtie =
      scene.addPolygon({{470, 120}, {730, 330}, {470, 330}, {730, 120}});

  const std::vector<Edge> &edges = scene.findPolygon(quad)->edges();

  const model::EdgeRef curve{quad, edges[2].id};
  scene.setEdgeKind(curve, EdgeKind::Bezier);
  scene.moveControlPoint(curve, 0, {260, 420});
  scene.moveControlPoint(curve, 1, {140, 420});

  const auto &bowtieEdges = scene.findPolygon(bowtie)->edges();
  scene.setLengthConstraint({quad, edges[0].id}, 230);
  scene.setLengthConstraint({quad, edges[1].id}, 200);
  // pionowa krawedz czworokata _|_ pozioma krawedz muszki
  scene.setOrthogonalConstraint({quad, edges[3].id},
                                {bowtie, bowtieEdges[1].id});
  return scene;
}

std::optional<double> Scene::lengthConstraint(EdgeRef edgeRef) const {
  for (const Constraint &constraint : constraints_) {
    if (constraint.type == ConstraintType::Length &&
        constraint.first == edgeRef) {
      return constraint.length;
    }
  }
  return std::nullopt;
}

void Scene::moveVertex(PolygonId polygonId, VertexId vertexId,
                       QPointF position) {
  Polygon *polygon = findPolygon(polygonId);

  const Vertex *vertex = polygon ? polygon->findVertex(vertexId) : nullptr;
  if (!vertex)
    return;
  // if (allEdgesHaveLength(*polygon)) {
  //   auto delta = position - vertex->position;
  //   polygon->translate(delta.x(), delta.y());
  //   return;
  // }
  const std::size_t index =
      static_cast<std::size_t>(vertex - polygon->vertices().data());

  const QPointF delta = position - vertex->position;
  const Polygon backup = *polygon;
  // const auto before = polygon->vertices();
  polygon->moveVertex(vertexId, position);
  propagate(*polygon, backup.vertices(), {index},
            std::vector<bool>(polygon->vertices().size(), false));
  // enforceLengths(*polygon, index,
  //                std::vector<bool>(polygon->vertices().size(), false));

  if (!constraintsSatisfied()) {
    *polygon = backup;
    polygon->translate(delta.x(), delta.y());
  }
}

// void Scene::enforceLengths(Polygon &polygon, std::size_t origin,
//                            std::vector<bool> fixed) {
//   const std::size_t n = polygon.vertices().size();
//   fixed[origin] = true;

//   for (const bool forward : {true, false}) {
//     std::size_t current = origin;
//     while (true) {
//       const std::size_t next =
//           forward ? (current + 1) % n : (current + n - 1) % n;
//       const Edge &edge = polygon.edges()[forward ? current : next];
//       const std::optional<double> length =
//           lengthConstraint({polygon.id(), edge.id});
//       if (!length || fixed[next])
//         break;

//       const QPointF a = polygon.vertices()[current].position;
//       const QPointF b = polygon.vertices()[next].position;
//       if (std::abs(distance(a, b) - *length) < lengthEpsilon)
//         break;

//       polygon.moveVertex(polygon.vertices()[next].id,
//                          pointAtDistance(a, b, *length));
//       fixed[next] = true;
//       current = next;
//     }
//   }
// }

// bool Scene::allEdgesHaveLength(const Polygon &polygon) const {
//   return std::all_of(
//       polygon.edges().begin(), polygon.edges().end(), [&](const Edge &edge) {
//         return lengthConstraint({polygon.id(), edge.id}).has_value();
//       });
// }

// bool Scene::lengthsSatisfied(const Polygon &polygon) const {
//   constexpr double tolerance = 1e-2;
//   for (const Edge &edge : polygon.edges()) {
//     const std::optional<double> length =
//         lengthConstraint({polygon.id(), edge.id});
//     if (!length)
//       continue;
//     const QPointF a = polygon.findVertex(edge.start)->position;
//     const QPointF b = polygon.findVertex(edge.end)->position;
//     if (std::abs(distance(a, b) - *length) > tolerance)
//       return false;
//   }
//   return true;
// }

void Scene::removeConstraintsOf(EdgeRef edge) {
  constraints_.erase(
      std::remove_if(constraints_.begin(), constraints_.end(),
                     [&](const Constraint &c) { return c.involves(edge); }),
      constraints_.end());
}

bool Scene::insertVertexAtMidpoint(PolygonId polygonId, EdgeId edgeId) {
  Polygon *polygon = findPolygon(polygonId);
  if (!polygon || !polygon->insertVertexAtEdgeMidpoint(edgeId))
    return false;
  removeConstraintsOf({polygonId, edgeId});
  return true;
}

bool Scene::removeVertex(PolygonId polygonId, VertexId vertexId) {
  Polygon *polygon = findPolygon(polygonId);
  const Vertex *vertex = polygon ? polygon->findVertex(vertexId) : nullptr;
  if (!vertex)
    return false;

  const std::size_t n = polygon->vertices().size();
  const std::size_t index =
      static_cast<std::size_t>(vertex - polygon->vertices().data());
  const EdgeId incoming = polygon->edges()[(index + n - 1) % n].id;
  const EdgeId outgoing = polygon->edges()[index].id;
  if (!polygon->removeVertex(vertexId))
    return false;

  removeConstraintsOf({polygonId, incoming});
  removeConstraintsOf({polygonId, outgoing});
  return true;
}

void Scene::removeLengthConstraint(EdgeRef edge) {
  constraints_.erase(std::remove_if(constraints_.begin(), constraints_.end(),
                                    [&](const Constraint &c) {
                                      return c.type == ConstraintType::Length &&
                                             c.first == edge;
                                    }),
                     constraints_.end());
}

void Scene::propagate(Polygon &polygon, const std::vector<Vertex> &before,
                      std::vector<std::size_t> queue, std::vector<bool> fixed) {
  const std::size_t n = polygon.vertices().size();
  for (const std::size_t i : queue)
    fixed[i] = true;

  for (std::size_t q = 0; q < queue.size(); ++q) {
    const std::size_t current = queue[q];
    const std::size_t prev = (current + n - 1) % n;
    const std::size_t next = (current + 1) % n;

    const std::pair<std::size_t, std::size_t> neighbours[] = {{next, current},
                                                              {prev, prev}};
    for (const auto &[neighbour, edgeIndex] : neighbours) {
      if (fixed[neighbour])
        continue;
      const EdgeRef edge{polygon.id(), polygon.edges()[edgeIndex].id};
      const bool orthogonal = hasOrthogonalConstraint(edge);
      const std::optional<double> length = lengthConstraint(edge);
      if (!orthogonal && !length)
        continue;
      const QPointF a = polygon.vertices()[current].position;
      const QPointF b = polygon.vertices()[neighbour].position;

      QPointF target;
      if (orthogonal) {
        const QPointF delta = a - before[current].position;
        if (std::hypot(delta.x(), delta.y()) < lengthEpsilon)
          continue;
        target = b + delta;
      } else {
        if (std::abs(distance(a, b) - *length) < lengthEpsilon)
          continue;
        target = pointAtDistance(a, b, *length);
      }

      polygon.moveVertex(polygon.vertices()[neighbour].id, target);
      fixed[neighbour] = true;
      queue.push_back(neighbour);
    }
  }
}

bool Scene::hasOrthogonalConstraint(EdgeRef edge) const {
  return std::any_of(
      constraints_.begin(), constraints_.end(), [&](const Constraint &c) {
        return c.type == ConstraintType::Orthogonal && c.involves(edge);
      });
}

ConstraintId Scene::setOrthogonalConstraint(EdgeRef first, EdgeRef second) {

  if (!isSegment(first) || !isSegment(second))
    return 0;
  const std::optional<QPointF> u = edgeVector(first);
  const std::optional<QPointF> v = edgeVector(second);
  if (!u || !v || first == second)
    return 0;
  if (std::hypot(u->x(), u->y()) < lengthEpsilon ||
      std::hypot(v->x(), v->y()) < lengthEpsilon)
    return 0;

  const bool exists = std::any_of(
      constraints_.begin(), constraints_.end(), [&](const Constraint &c) {
        return c.type == ConstraintType::Orthogonal && c.second &&
               ((c.first == first && *c.second == second) ||
                (c.first == second && *c.second == first));
      });
  if (exists)
    return 0;

  const Scene backup = *this;
  const ConstraintId id = nextConstraintId_++;
  constraints_.push_back({id, ConstraintType::Orthogonal, first, second});
  if (constraintsSatisfied())
    return id;

  struct Attempt {
    EdgeRef rotated;
    EdgeRef reference;
    bool pivotAtStart;
  };

  const Attempt attempts[] = {
      {second, first, true},
      {second, first, false},
      {first, second, true},
      {first, second, false},
  };

  const Scene withConstraint = *this;
  for (const Attempt &attempt : attempts) {
    *this = withConstraint;
    rotatePerpendicular(attempt.rotated, *edgeVector(attempt.reference),
                        attempt.pivotAtStart);
    if (constraintsSatisfied())
      return id;
  }

  *this = backup;
  return 0;
}

std::optional<QPointF> Scene::edgeVector(EdgeRef ref) const {
  const Polygon *polygon = findPolygon(ref.polygon);
  const Edge *edge = polygon ? polygon->findEdge(ref.edge) : nullptr;
  if (!edge)
    return std::nullopt;
  return polygon->findVertex(edge->end)->position -
         polygon->findVertex(edge->start)->position;
}

bool Scene::constraintsSatisfied() const {
  constexpr double lengthTolerance = 1e-2;
  constexpr double cosTolerance = 1e-3;
  for (const Constraint &c : constraints_) {
    const std::optional<QPointF> u = edgeVector(c.first);
    if (!u)
      return false;
    const double lu = std::hypot(u->x(), u->y());

    switch (c.type) {
    case ConstraintType::Length: {
      if (std::abs(lu - c.length) > lengthTolerance)
        return false;
      break;
    }
    case ConstraintType::Orthogonal: {
      const std::optional<QPointF> v =
          c.second ? edgeVector(*c.second) : std::nullopt;
      if (!v)
        continue;
      const double lv = std::hypot(v->x(), v->y());
      if (lu < lengthEpsilon || lv < lengthEpsilon)
        continue;

      if (std::abs(QPointF::dotProduct(*u, *v)) / (lu * lv) > cosTolerance)
        return false;
      break;
    }
    }
  }
  return true;
}

void Scene::rotatePerpendicular(EdgeRef ref, QPointF reference,
                                bool pivotAtStart) {
  Polygon *polygon = findPolygon(ref.polygon);
  const Edge *edge = polygon ? polygon->findEdge(ref.edge) : nullptr;
  if (!polygon || !edge)
    return;

  const std::size_t n = polygon->vertices().size();
  const std::size_t start =
      static_cast<std::size_t>(edge - polygon->edges().data());
  const std::size_t end = (start + 1) % n;

  const QPointF a = polygon->vertices()[start].position;
  const QPointF b = polygon->vertices()[end].position;
  const QPointF d = b - a;

  const double referenceLength = std::hypot(reference.x(), reference.y());
  QPointF normal(-reference.y() / referenceLength,
                 reference.x() / referenceLength);
  if (QPointF::dotProduct(normal, d) < 0)
    normal = -normal;

  const QPointF rotated = normal * std::hypot(d.x(), d.y());

  const std::vector<Vertex> before = polygon->vertices();
  std::vector<bool> fixed(n, false);
  std::size_t moved;

  if (pivotAtStart) {
    polygon->moveVertex(polygon->vertices()[end].id, a + rotated);
    fixed[start] = true;
    moved = end;
  } else {
    polygon->moveVertex(polygon->vertices()[start].id, b - rotated);
    fixed[end] = true;
    moved = start;
  }

  propagate(*polygon, before, {moved}, fixed);
}

void Scene::removeConstraint(ConstraintId id) {
  constraints_.erase(
      std::remove_if(constraints_.begin(), constraints_.end(),
                     [&](const Constraint &c) { return c.id == id; }),
      constraints_.end());
}

bool Scene::isSegment(EdgeRef ref) const {
  const Polygon *polygon = findPolygon(ref.polygon);
  if (!polygon)
    return false;
  const Edge *edge = polygon->findEdge(ref.edge);
  return edge && edge->kind == EdgeKind::Segment;
}

bool Scene::setEdgeKind(EdgeRef ref, EdgeKind kind) {
  Polygon *polygon = findPolygon(ref.polygon);
  if (!polygon || !polygon->setEdgeKind(ref.edge, kind))
    return false;
  if (kind != EdgeKind::Segment)
    removeConstraintsOf(ref);
  return true;
}

void Scene::moveControlPoint(EdgeRef ref, std::size_t index, QPointF position) {
  if (Polygon *polygon = findPolygon(ref.polygon))
    polygon->moveControlPoint(ref.edge, index, position);
}
} // namespace model
