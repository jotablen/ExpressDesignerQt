#include "SISLWrapper.h"
#include <QtMath>

namespace ExpressDesigner {
namespace Geometry {

// PointList is defined in SISLWrapper.h as QVector<QPointF>
// This file provides additional point list operations.

QPointF centroid(const PointList& points)
{
    if (points.isEmpty()) return {};
    QPointF sum;
    for (const auto& p : points) sum += p;
    return sum / points.size();
}

QVector<QPointF> scale(const PointList& points, double sx, double sy)
{
    QVector<QPointF> result;
    result.reserve(points.size());
    for (const auto& p : points)
        result.append(QPointF(p.x() * sx, p.y() * sy));
    return result;
}

QVector<QPointF> translate(const PointList& points, const QPointF& offset)
{
    QVector<QPointF> result;
    result.reserve(points.size());
    for (const auto& p : points)
        result.append(p + offset);
    return result;
}

QVector<QPointF> rotate(const PointList& points, double angleDeg)
{
    double rad = qDegreesToRadians(angleDeg);
    double cosA = qCos(rad), sinA = qSin(rad);
    QVector<QPointF> result;
    result.reserve(points.size());
    for (const auto& p : points) {
        result.append(QPointF(p.x() * cosA - p.y() * sinA,
                               p.x() * sinA + p.y() * cosA));
    }
    return result;
}

} // namespace Geometry
} // namespace ExpressDesigner