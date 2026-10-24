#include "Scene.h"
#include "model/Constraint.h"
#include "model/Polygon.h"

#include <algorithm>
#include <cmath>

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

  if (!e)
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

  if (!lengthsSatisfied(*polygon)) {
    *this = backup;
    return 0;
  }

  return id;
}

Scene Scene::createDemo() {
  Scene scene;
  const PolygonId quad =
      scene.addPolygon({{100, 100}, {330, 130}, {290, 330}, {130, 280}});
  scene.addPolygon({{470, 120}, {730, 330}, {470, 330}, {730, 120}});

  const std::vector<Edge> &edges = scene.findPolygon(quad)->edges();
  scene.setLengthConstraint({quad, edges[0].id}, 230);
  scene.setLengthConstraint({quad, edges[1].id}, 200);
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
  if (allEdgesHaveLength(*polygon)) {
    auto delta = position - vertex->position;
    polygon->translate(delta.x(), delta.y());
    return;
  }

  const std::size_t index =
      static_cast<std::size_t>(vertex - polygon->vertices().data());
  const auto before = polygon->vertices();
  polygon->moveVertex(vertexId, position);
  propagate(*polygon, before, {index},
            std::vector<bool>(polygon->vertices().size(), false));
  // enforceLengths(*polygon, index,
  //                std::vector<bool>(polygon->vertices().size(), false));
}

void Scene::enforceLengths(Polygon &polygon, std::size_t origin,
                           std::vector<bool> fixed) {
  const std::size_t n = polygon.vertices().size();
  fixed[origin] = true;

  for (const bool forward : {true, false}) {
    std::size_t current = origin;
    while (true) {
      const std::size_t next =
          forward ? (current + 1) % n : (current + n - 1) % n;
      const Edge &edge = polygon.edges()[forward ? current : next];
      const std::optional<double> length =
          lengthConstraint({polygon.id(), edge.id});
      if (!length || fixed[next])
        break;

      const QPointF a = polygon.vertices()[current].position;
      const QPointF b = polygon.vertices()[next].position;
      if (std::abs(distance(a, b) - *length) < lengthEpsilon)
        break;

      polygon.moveVertex(polygon.vertices()[next].id,
                         pointAtDistance(a, b, *length));
      fixed[next] = true;
      current = next;
    }
  }
}

bool Scene::allEdgesHaveLength(const Polygon &polygon) const {
  return std::all_of(
      polygon.edges().begin(), polygon.edges().end(), [&](const Edge &edge) {
        return lengthConstraint({polygon.id(), edge.id}).has_value();
      });
}

bool Scene::lengthsSatisfied(const Polygon &polygon) const {
  constexpr double tolerance = 1e-2;
  for (const Edge &edge : polygon.edges()) {
    const std::optional<double> length =
        lengthConstraint({polygon.id(), edge.id});
    if (!length)
      continue;
    const QPointF a = polygon.findVertex(edge.start)->position;
    const QPointF b = polygon.findVertex(edge.end)->position;
    if (std::abs(distance(a, b) - *length) > tolerance)
      return false;
  }
  return true;
}

void Scene::removeConstraintsOf(EdgeRef edge) {
  constraints_.erase(std::remove_if(constraints_.begin(), constraints_.end(),
                                    [&](const Constraint &c) {
                                      return c.first == edge ||
                                             (c.second && *c.second == edge);
                                    }),
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
      const std::optional<double> length = lengthConstraint(edge);
      if (!length)
        continue;
      const QPointF a = polygon.vertices()[current].position;
      const QPointF b = polygon.vertices()[neighbour].position;
      if (std::abs(distance(a, b) - *length) < lengthEpsilon)
        continue;

      polygon.moveVertex(polygon.vertices()[neighbour].id,
                         pointAtDistance(a, b, *length));
      fixed[neighbour] = true;
      queue.push_back(neighbour);
    }
  }
  (void)before;
}
} // namespace model
