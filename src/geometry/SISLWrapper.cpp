#include "SISLWrapper.h"
#include <QtMath>
#include <algorithm>
#include <limits>

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

QPointF SISLCurve::normal(double t, bool flipped) const
{
    QPointF d = derivative(t);
    double len = qSqrt(d.x() * d.x() + d.y() * d.y());
    if (len < 1e-12) return flipped ? QPointF(0, -1) : QPointF(0, 1);
    QPointF n(-d.y() / len, d.x() / len);
    return flipped ? QPointF(-n.x(), -n.y()) : n;
}

// --- Continuous operations on polyline ---

QPointF SISLCurve::closestPoint(const QPointF& p, double* outT) const
{
    if (m_points.size() < 2) return m_points.isEmpty() ? QPointF() : m_points[0];
    double bestDistSq = std::numeric_limits<double>::max();
    QPointF bestPt;
    double bestT = 0.0;
    int n = m_points.size();

    for (int i = 0; i < n - 1; ++i) {
        QPointF a = m_points[i];
        QPointF b = m_points[i + 1];
        QPointF ab = b - a;
        double abLenSq = ab.x() * ab.x() + ab.y() * ab.y();
        if (abLenSq < 1e-18) {
            // Degenerate segment – just check the point
            double d2 = (p.x() - a.x()) * (p.x() - a.x()) + (p.y() - a.y()) * (p.y() - a.y());
            if (d2 < bestDistSq) { bestDistSq = d2; bestPt = a; bestT = static_cast<double>(i) / (n - 1); }
            continue;
        }
        // Project p onto segment a->b
        double t = ((p.x() - a.x()) * ab.x() + (p.y() - a.y()) * ab.y()) / abLenSq;
        t = qBound(0.0, t, 1.0);
        QPointF proj = a + ab * t;
        double d2 = (p.x() - proj.x()) * (p.x() - proj.x()) + (p.y() - proj.y()) * (p.y() - proj.y());
        if (d2 < bestDistSq) {
            bestDistSq = d2;
            bestPt = proj;
            bestT = (i + t) / (n - 1);
        }
    }
    if (outT) *outT = qBound(0.0, bestT, 1.0);
    return bestPt;
}

double SISLCurve::rayIntersection(const QPointF& origin, const QPointF& dir) const
{
    if (m_points.size() < 2) return -1.0;
    double bestT = std::numeric_limits<double>::max();
    bool found = false;
    int n = m_points.size();

    for (int i = 0; i < n - 1; ++i) {
        QPointF seg = m_points[i + 1] - m_points[i];
        double denom = dir.x() * seg.y() - dir.y() * seg.x();
        if (qAbs(denom) < 1e-12) continue;
        QPointF diff = m_points[i] - origin;
        double tRay   = (diff.x() * seg.y() - diff.y() * seg.x()) / denom;
        double tSeg   = (diff.x() * dir.y() - diff.y() * dir.x()) / -denom;
        if (tRay >= 0.0 && tSeg >= 0.0 && tSeg <= 1.0) {
            if (tRay < bestT) { bestT = tRay; found = true; }
        }
    }
    return found ? bestT : -1.0;
}

QPointF SISLCurve::normalAt(const QPointF& p) const
{
    double t;
    closestPoint(p, &t);
    return normal(t);
}

// --- SISLSurface ---

SISLSurface::SISLSurface() = default;
SISLSurface::~SISLSurface() = default;
bool SISLSurface::isValid() const { return m_valid; }
QPointF SISLSurface::evaluate(double, double) const { return {}; }

// --- Operations ---

namespace Operations {

QVector<QPointF> offsetCurve(const QVector<QPointF>& points, double distance)
{
    if (points.size() < 2) return points;
    QVector<QPointF> result;
    result.reserve(points.size());
    SISLCurve curve(points, 3, true);
    int n = points.size();
    for (int i = 0; i < n; ++i) {
        double t = n > 1 ? static_cast<double>(i) / (n - 1) : 0.0;
        QPointF nrm = curve.normal(t);
        result.append(points[i] + nrm * distance);
    }
    return result;
}

QVector<QPointF> intersectCurves(const QVector<QPointF>& a, const QVector<QPointF>& b)
{
    Q_UNUSED(a); Q_UNUSED(b);
    return {};
}

QVector<Normal> computeNormals2D(const QVector<QPointF>& points, int numRays, double rayLength, bool flipped)
{
    QVector<Normal> result;
    result.reserve(numRays);
    SISLCurve curve(points, 3, true);
    for (int i = 0; i < numRays; ++i) {
        double t = numRays > 1 ? static_cast<double>(i) / (numRays - 1) : 0.0;
        QPointF origin = curve.evaluate(t);
        QPointF nrm = curve.normal(t, flipped);
        result.append({origin, origin + nrm * rayLength});
    }
    return result;
}

QVector<QPointF> discretizeArc(const QPointF& center, double radius, double startAngle, double endAngle, int numPoints)
{
    QVector<QPointF> result;
    result.reserve(numPoints);
    for (int i = 0; i < numPoints; ++i) {
        double t = numPoints > 1 ? static_cast<double>(i) / (numPoints - 1) : 0.0;
        double angle = startAngle + (endAngle - startAngle) * t;
        result.append(QPointF(center.x() + radius * qCos(angle), center.y() + radius * qSin(angle)));
    }
    return result;
}

} // namespace Operations
} // namespace Geometry
} // namespace ExpressDesigner