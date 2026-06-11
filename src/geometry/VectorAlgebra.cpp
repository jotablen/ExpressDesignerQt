#include "SISLWrapper.h"
#include <QtMath>

namespace ExpressDesigner {
namespace Geometry {

double dotProduct(const QPointF& a, const QPointF& b) {
    return a.x() * b.x() + a.y() * b.y();
}

double crossProduct(const QPointF& a, const QPointF& b) {
    return a.x() * b.y() - a.y() * b.x();
}

double magnitude(const QPointF& v) {
    return qSqrt(v.x() * v.x() + v.y() * v.y());
}

QPointF normalize(const QPointF& v) {
    double mag = magnitude(v);
    return mag > 1e-12 ? v / mag : QPointF();
}

double angleBetween(const QPointF& a, const QPointF& b) {
    double dot = dotProduct(a, b);
    double mags = magnitude(a) * magnitude(b);
    return mags > 1e-12 ? qRadiansToDegrees(qAcos(qBound(-1.0, dot / mags, 1.0))) : 0.0;
}

QPointF perpendicular(const QPointF& v, bool clockwise) {
    return clockwise ? QPointF(v.y(), -v.x()) : QPointF(-v.y(), v.x());
}

} // namespace Geometry
} // namespace ExpressDesigner