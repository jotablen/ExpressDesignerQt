#pragma once
#include <QPointF>
#include <QtMath>

namespace ExpressDesigner {
namespace Geometry {

/// Dot product
inline double dot(const QPointF& a, const QPointF& b) {
    return a.x() * b.x() + a.y() * b.y();
}

/// 2D cross product (scalar)
inline double cross(const QPointF& a, const QPointF& b) {
    return a.x() * b.y() - a.y() * b.x();
}

/// Squared length
inline double lengthSq(const QPointF& v) {
    return dot(v, v);
}

/// Length
inline double length(const QPointF& v) {
    return qSqrt(lengthSq(v));
}

/// Normalized vector (returns (0,1) if zero-length)
inline QPointF normalized(const QPointF& v) {
    double l = length(v);
    return l > 1e-12 ? v / l : QPointF(0, 1);
}

/// Normal of a 2D curve tangent: n = (-tangent.y, tangent.x)
inline QPointF curveNormal2D(const QPointF& tangent, bool flipped = false) {
    QPointF n(-tangent.y(), tangent.x());
    double l = length(n);
    n = l > 1e-12 ? n / l : QPointF(0, 1);
    return flipped ? -n : n;
}

/// Euclidean distance
inline double dist(const QPointF& a, const QPointF& b) {
    return length(b - a);
}

/// Squared Euclidean distance
inline double distSq(const QPointF& a, const QPointF& b) {
    return lengthSq(b - a);
}

} // namespace Geometry
} // namespace ExpressDesigner