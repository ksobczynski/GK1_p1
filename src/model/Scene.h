#pragma once

#include "Polygon.h"

namespace model {
// Element references across polygons should pair PolygonId with VertexId/EdgeId.
class Scene {
public:
    PolygonId addPolygon(const std::vector<Point>& points);
    const std::vector<Polygon>& polygons() const { return polygons_; }
    Polygon* findPolygon(PolygonId id);
    static Scene createDemo();

private:
    PolygonId nextPolygonId_ = 1;
    std::vector<Polygon> polygons_;
};
} // namespace model
