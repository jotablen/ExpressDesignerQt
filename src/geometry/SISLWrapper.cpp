#include "SISLWrapper.h"
#include <QtMath>
#include <algorithm>

namespace ExpressDesigner {
namespace Geometry {

// SISLCurve
SISLCurve::SISLCurve() = default;
SISLCurve::SISLCurve(const QVector<QPointF>& controlPoints, int order, bool open)
    : m_points(controlPoints), m_order(order), m_open(open), m_valid(!controlPoints.isEmpty() && order >= 2)
{
}
SISLCurve::~SISLCurve() = default;
SISLCurve::SISLCurve(SISLCurve&&) noexcept = default;
SISLCurve& SISLCurve::operator=(SISLCurve&&) noexcept = default;

bool SISLCurve::isValid() const { return m_valid; }
int SISLCurve::order() const { return m_order; }
bool SISLCurve::isOpen() const { return m_open; }
QVector<QPointF> SISLCurve::controlPoints() const { return m_points; }
void SISLCurve::setControlPoints(const QVector<QPointF>& points) { m_points = points; m_valid = !points.isEmpty(); }

QPointF SISLCurve::evaluate(double t) const
{
    if (m_points.isEmpty()) return {};
    int n = m_points.size();
    double param = qBound(0.0, t, 1.0);
    double seg = param * (n - 1);
    int idx = qMin(static_cast<int>(seg), n - 2);
    double lt = seg - idx;
    return m_points[idx] + (m_points[idx + 1] - m_points[idx]) * lt;
}

QVector<QPointF> SISLCurve::evaluateAll(int numPoints) const
{
    QVector<QPointF> result;
    result.reserve(numPoints);
    for (int i = 0; i < numPoints; ++i) {
        double t = numPoints > 1 ? static_cast<double>(i) / (numPoints - 1) : 0.0;
        result.append(evaluate(t));
    }
    return result;
}

QPointF SISLCurve::derivative(double t) const
{
    if (m_points.size() < 2) return {};
    int n = m_points.size();
    double seg = qBound(0.0, t, 1.0) * (n - 1);
    int idx = qMin(static_cast<int>(seg), n - 2);
    return m_points[idx + 1] - m_points[idx];
}

SISLCurve SISLCurve::interpolate(const QVector<QPointF>& points, int order)
{
    return SISLCurve(points, order, true);
}

SISLCurve SISLCurve::approximate(const QVector<QPointF>& points, double)
{
    return SISLCurve(points, 3, true);
}

// SISLSurface
SISLSurface::SISLSurface() = default;
SISLSurface::~SISLSurface() = default;
bool SISLSurface::isValid() const { return m_valid; }
QPointF SISLSurface::evaluate(double, double) const { return {}; }

// Operations
namespace Operations {

QVector<QPointF> offsetCurve(const QVector<QPointF>& points, double distance)
{
    if (points.size() < 2) return points;
    QVector<QPointF> result;
    result.reserve(points.size());
    for (int i = 0; i < points.size() - 1; ++i) {
        QPointF dir = points[i + 1] - points[i];
        double len = qSqrt(dir.x() * dir.x() + dir.y() * dir.y());
        if (len > 1e-12) {
            QPointF normal(-dir.y() / len, dir.x() / len);
            result.append(points[i] + normal * distance);
        }
    }
    if (!result.isEmpty()) result.append(result.last());
    return result;
}

QVector<QPointF> intersectCurves(const QVector<QPointF>& a, const QVector<QPointF>& b)
{
    // Simplified intersection - returns empty for now
    Q_UNUSED(a); Q_UNUSED(b);
    return {};
}

QVector<Normal> computeNormals2D(const QVector<QPointF>& points, int numRays, double rayLength, bool flipped)
{
    QVector<Normal> result;
    if (points.size() < 2 || numRays < 1) return result;
    result.reserve(numRays);
    int totalSegs = points.size() - 1;
    for (int i = 0; i < numRays; ++i) {
        double t = numRays > 1 ? static_cast<double>(i) / (numRays - 1) : 0.0;
        double seg = t * totalSegs;
        int idx = qMin(static_cast<int>(seg), totalSegs - 1);
        QPointF p0 = points[idx], p1 = points[idx + 1];
        double lt = seg - idx;
        QPointF pos = p0 + (p1 - p0) * lt;
        QPointF dir = p1 - p0;
        double len = qSqrt(dir.x() * dir.x() + dir.y() * dir.y());
        if (len > 1e-12) {
            QPointF normal(-dir.y() / len, dir.x() / len);
            if (flipped) normal = -normal;
            result.append({pos, pos + normal * rayLength});
        }
    }
    return result;
}

QVector<QPointF> discretizeArc(const QPointF& center, double radius, double startAngle, double endAngle, int numPoints)
{
    QVector<QPointF> result;
    result.reserve(numPoints);
    double range = endAngle - startAngle;
    double radStart = qDegreesToRadians(startAngle);
    for (int i = 0; i < numPoints; ++i) {
        double t = numPoints > 1 ? static_cast<double>(i) / (numPoints - 1) : 0.0;
        double angle = radStart + qDegreesToRadians(range * t);
        result.append(QPointF(center.x() + radius * qCos(angle), center.y() + radius * qSin(angle)));
    }
    return result;
}

} // namespace Operations
} // namespace Geometry
} // namespace ExpressDesigner